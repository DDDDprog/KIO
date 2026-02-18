/*
 Copyright (c) 2025 Dipanjan Dhar
 SPDX-License-Identifier: GPL-3.0-only
 */

#include "axeon/jit_engine.hpp"
#ifdef KIO_JIT_ENABLED
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Vectorize/LoopVectorize.h>
#include <llvm/Transforms/Vectorize/SLPVectorizer.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/LICM.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar/SimpleLoopUnswitch.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <iostream>
#include <cmath>
#include <chrono>

using namespace llvm;
using namespace llvm::orc;

namespace kio {

struct JITEngine::Impl {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::orc::LLJIT> lljit;

    Impl() : context(std::make_unique<llvm::LLVMContext>()) {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
        
        auto jit_builder = llvm::orc::LLJITBuilder();
        
        auto JTMB = llvm::orc::JITTargetMachineBuilder::detectHost();
        if (JTMB) {
            JTMB->setCodeGenOptLevel(llvm::CodeGenOptLevel::Aggressive);
            // Use the host CPU detected by LLVM instead of the literal "native"
            jit_builder.setJITTargetMachineBuilder(std::move(*JTMB));
        }
        
        jit_builder.setNumCompileThreads(0);
        
        auto jit_or_err = jit_builder.create();
        if (jit_or_err) {
            lljit = std::move(*jit_or_err);
        } else {
            lljit = nullptr;
        }
    }

    void optimizeModule(llvm::Module* M) {
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager MAM;
        PassBuilder PB;

        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O3);
        MPM.run(*M, MAM);
    }
};

JITEngine::JITEngine() : impl_(std::make_unique<Impl>()) {}
JITEngine::~JITEngine() = default;

JITEngine::CompiledLoop JITEngine::compileLoop(Chunk* chunk, uint8_t* startIp) {
    if (!impl_->lljit) return nullptr;

    // 1. Scan bytecode to find max local slot usage
    int max_slot = -1;
    uint8_t* scan = startIp;
    bool scan_done = false;
    bool uses_globals = false;
    int limit = 1000;
    
    while (!scan_done && limit-- > 0 && scan < chunk->code.data() + chunk->code.size()) {
        OpCode op = (OpCode)(*scan);
        switch (op) {
            case OpCode::CONSTANT: scan += 2; break;
            case OpCode::GET_LOCAL:
            case OpCode::SET_LOCAL: {
                uint8_t slot = scan[1];
                if (slot > max_slot) max_slot = slot;
                scan += 2;
                break;
            }
            case OpCode::GET_GLOBAL:
            case OpCode::SET_GLOBAL:
            case OpCode::DEFINE_GLOBAL:
                return nullptr; // Globals use string keys, not supported in JIT
            case OpCode::JUMP: 
            case OpCode::JUMP_IF_FALSE: scan += 3; break;
            case OpCode::LOOP: {
                scan_done = true;
                scan += 3;
                break;
            }
            case OpCode::CALL: 
            case OpCode::PRINT: 
            case OpCode::HALT:
            case OpCode::SYS_QUERY:
            case OpCode::ARRAY_NEW:
            case OpCode::ARRAY_GET:
            case OpCode::ARRAY_SET:
            case OpCode::FLOOR:
            case OpCode::SQRT:
                scan += 1;
                break;
            default: 
                scan += 1; // Single-byte ops (ADD, SUB, etc.)
                break;
        }
    }
    
    if (limit <= 0) return nullptr;
    
    max_slot += 2; 

    auto M = std::make_unique<llvm::Module>("kio_jit_module", *impl_->context);
    M->setDataLayout(impl_->lljit->getDataLayout());
    
    llvm::IRBuilder<> builder(*impl_->context);

    llvm::Type* i32 = builder.getInt32Ty();
    llvm::Type* doubleTy = builder.getDoubleTy();
    llvm::Type* voidTy = builder.getVoidTy();
    llvm::PointerType* ptrTy = builder.getPtrTy();

    std::vector<llvm::Type*> argTypes = { ptrTy, ptrTy, i32, ptrTy };
    llvm::FunctionType* FT = llvm::FunctionType::get(voidTy, argTypes, false);
    llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "hot_loop", M.get());

    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*impl_->context, "entry", F);
    llvm::BasicBlock* loopDetailsBB = llvm::BasicBlock::Create(*impl_->context, "loop_setup", F);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(*impl_->context, "loop_body", F);
    llvm::BasicBlock* exitBB = llvm::BasicBlock::Create(*impl_->context, "exit", F);

    builder.SetInsertPoint(entryBB);
    
    llvm::FastMathFlags fmf;
    fmf.setFast();
    builder.setFastMathFlags(fmf);

    llvm::Value* stackBase = F->getArg(0);
    llvm::Value* slotsOffset = F->getArg(2);

    llvm::Value* stackStructPtr = builder.CreateBitCast(stackBase, llvm::PointerType::get(*impl_->context, 0));

    std::vector<llvm::Value*> localAllocas(max_slot + 1, nullptr);
    for (int i = 0; i <= max_slot; i++) {
        localAllocas[i] = builder.CreateAlloca(doubleTy, nullptr, "local_" + std::to_string(i));
    }
    
    // Load initial values from stack
    for (int i = 0; i <= max_slot; i++) {
        llvm::Value* idx = builder.CreateAdd(slotsOffset, builder.getInt32(i));
        llvm::Value* valPtr = builder.CreateGEP(doubleTy, stackStructPtr, idx);
        llvm::Value* rawVal = builder.CreateLoad(doubleTy, valPtr);
        builder.CreateStore(rawVal, localAllocas[i]);
    }

    builder.CreateBr(loopDetailsBB);
    builder.SetInsertPoint(loopDetailsBB);
    builder.CreateBr(loopBodyBB);
    builder.SetInsertPoint(loopBodyBB);

    std::vector<llvm::Value*> simStack;

    uint8_t* ip = startIp;
    bool compiling = true;
    
    while (compiling) {
        OpCode op = (OpCode)(*ip++);
        std::cerr << "[JIT] Compiling op: " << (int)op << " at offset " << (int)(ip - startIp - 1) << std::endl;
        switch (op) {
            case OpCode::CONSTANT: {
                uint8_t idx = *ip++;
                Value v = chunk->constants[idx];
                if (isNumber(v)) {
                    simStack.push_back(llvm::ConstantFP::get(doubleTy, valueToDouble(v)));
                } else if (isBool(v)) {
                     simStack.push_back(llvm::ConstantFP::get(doubleTy, (v.v == (0x7ff8000000000000 | 3)) ? 1.0 : 0.0));
                } else return nullptr;
                break;
            }
            case OpCode::GET_LOCAL: {
                uint8_t idx = *ip++;
                if (idx > max_slot) return nullptr;
                llvm::Value* val = builder.CreateLoad(doubleTy, localAllocas[idx]);
                simStack.push_back(val);
                break;
            }
            case OpCode::SET_LOCAL: {
                uint8_t idx = *ip++;
                if (simStack.empty()) return nullptr;
                llvm::Value* val = simStack.back();
                simStack.pop_back();
                builder.CreateStore(val, localAllocas[idx]);
                break;
            }
            case OpCode::ADD:
            case OpCode::SUBTRACT:
            case OpCode::MULTIPLY: {
                if (simStack.size() < 2) return nullptr;
                llvm::Value* b = simStack.back(); simStack.pop_back();
                llvm::Value* a = simStack.back(); simStack.pop_back();
                if (op == OpCode::ADD) simStack.push_back(builder.CreateFAdd(a, b));
                else if (op == OpCode::SUBTRACT) simStack.push_back(builder.CreateFSub(a, b));
                else simStack.push_back(builder.CreateFMul(a, b));
                break;
            }
            case OpCode::DIVIDE: {
                if (simStack.size() < 2) return nullptr;
                llvm::Value* b = simStack.back(); simStack.pop_back();
                llvm::Value* a = simStack.back(); simStack.pop_back();
                // Optimize divide by constant
                if (llvm::ConstantFP* CFP = llvm::dyn_cast<llvm::ConstantFP>(b)) {
                    double val = CFP->getValueAPF().convertToDouble();
                    if (val != 0.0) {
                        simStack.push_back(builder.CreateFMul(a, llvm::ConstantFP::get(doubleTy, 1.0 / val)));
                        break;
                    }
                }
                simStack.push_back(builder.CreateFDiv(a, b));
                break;
            }
            case OpCode::MODULO: {
                if (simStack.size() < 2) return nullptr;
                llvm::Value* b = simStack.back(); simStack.pop_back();
                llvm::Value* a = simStack.back(); simStack.pop_back();
                // a % b = a - b * floor(a / b)
                // This is often faster than fmod call
                llvm::Function* floorFunc = llvm::Intrinsic::getOrInsertDeclaration(M.get(), llvm::Intrinsic::floor, {doubleTy});
                
                llvm::Value* div;
                // Optimization: if b is constant, use fmul by reciprocal
                if (llvm::ConstantFP* CFP = llvm::dyn_cast<llvm::ConstantFP>(b)) {
                    double val = CFP->getValueAPF().convertToDouble();
                    div = builder.CreateFMul(a, llvm::ConstantFP::get(doubleTy, 1.0 / val));
                } else {
                    div = builder.CreateFDiv(a, b);
                }
                
                llvm::Value* fl = builder.CreateCall(floorFunc, {div});
                llvm::Value* mul = builder.CreateFMul(b, fl);
                simStack.push_back(builder.CreateFSub(a, mul));
                break;
            }
            case OpCode::LESS:
            case OpCode::GREATER: 
            case OpCode::LESS_EQUAL:
            case OpCode::GREATER_EQUAL:
            case OpCode::EQUAL: {
                    if (simStack.size() < 2) return nullptr;
                    llvm::Value* b = simStack.back(); simStack.pop_back();
                    llvm::Value* a = simStack.back(); simStack.pop_back();
                    llvm::Value* cmp = nullptr;
                    if (op == OpCode::LESS) cmp = builder.CreateFCmpOLT(a, b);
                    else if (op == OpCode::GREATER) cmp = builder.CreateFCmpOGT(a, b);
                    else if (op == OpCode::LESS_EQUAL) cmp = builder.CreateFCmpOLE(a, b);
                    else if (op == OpCode::GREATER_EQUAL) cmp = builder.CreateFCmpOGE(a, b);
                    else cmp = builder.CreateFCmpOEQ(a, b);
                    simStack.push_back(builder.CreateUIToFP(cmp, doubleTy));
                    break;
            }
            case OpCode::NEGATE: {
                if (simStack.empty()) return nullptr;
                llvm::Value* val = simStack.back(); simStack.pop_back();
                simStack.push_back(builder.CreateFNeg(val));
                break;
            }
            case OpCode::NOT: {
                if (simStack.empty()) return nullptr;
                llvm::Value* val = simStack.back(); simStack.pop_back();
                // Not in KIO is: v == 0 ? 1 : 0
                llvm::Value* isZero = builder.CreateFCmpOEQ(val, llvm::ConstantFP::get(doubleTy, 0.0));
                simStack.push_back(builder.CreateUIToFP(isZero, doubleTy));
                break;
            }
            case OpCode::FLOOR: {
                if (simStack.empty()) return nullptr;
                llvm::Value* val = simStack.back(); simStack.pop_back();
                llvm::Function* floorFunc = llvm::Intrinsic::getOrInsertDeclaration(M.get(), llvm::Intrinsic::floor, {doubleTy});
                simStack.push_back(builder.CreateCall(floorFunc, {val}));
                break;
            }
            case OpCode::SQRT: {
                if (simStack.empty()) return nullptr;
                llvm::Value* val = simStack.back(); simStack.pop_back();
                llvm::Function* sqrtFunc = llvm::Intrinsic::getOrInsertDeclaration(M.get(), llvm::Intrinsic::sqrt, {doubleTy});
                simStack.push_back(builder.CreateCall(sqrtFunc, {val}));
                break;
            }
            case OpCode::JUMP_IF_FALSE: {
                ip += 2;
                if (simStack.empty()) return nullptr;
                llvm::Value* condVal = simStack.back(); simStack.pop_back();
                llvm::Value* isZero = builder.CreateFCmpOEQ(condVal, llvm::ConstantFP::get(doubleTy, 0.0));
                llvm::BasicBlock* nextBB = llvm::BasicBlock::Create(*impl_->context, "cont", F);
                builder.CreateCondBr(isZero, exitBB, nextBB);
                builder.SetInsertPoint(nextBB);
                break;
            }
            case OpCode::LOOP: {
                ip += 2;
                builder.CreateBr(loopBodyBB);
                compiling = false;
                break;
            }
            case OpCode::POP: { 
                if (!simStack.empty()) simStack.pop_back(); 
                break; 
            }
            case OpCode::JUMP: {
                // Jump forward by 2-byte offset (used for else branches, etc.)
                uint8_t hi = *ip++;
                uint8_t lo = *ip++;
                uint16_t offset = (hi << 8) | lo;
                ip += offset;  // Jump forward from current position
                break;
            }
            default: 
                std::cerr << "[JIT] Unknown opcode: " << (int)op << std::endl;
                return nullptr;
        }
    }
    
    builder.SetInsertPoint(exitBB);
    
    for (int i = 0; i <= max_slot; i++) {
        llvm::Value* idx = builder.CreateAdd(slotsOffset, builder.getInt32(i));
        llvm::Value* valPtr = builder.CreateGEP(doubleTy, stackStructPtr, idx);
        llvm::Value* val = builder.CreateLoad(doubleTy, localAllocas[i]);
        builder.CreateStore(val, valPtr);
    }
    
    builder.CreateRetVoid();

    if (llvm::verifyFunction(*F, &llvm::errs())) return nullptr;
    impl_->optimizeModule(M.get());
    
    auto TSM = llvm::orc::ThreadSafeModule(std::move(M), std::move(impl_->context));
    if (auto err = impl_->lljit->addIRModule(std::move(TSM))) return nullptr;
    
    auto sym = impl_->lljit->lookup("hot_loop");
    if (!sym) return nullptr;

    impl_->context = std::make_unique<llvm::LLVMContext>();
    return (CompiledLoop)sym->getValue();
}

} // namespace kio

#else
namespace kio {
struct JITEngine::Impl {};
JITEngine::JITEngine() : impl_(nullptr) {}
JITEngine::~JITEngine() = default;
JITEngine::CompiledLoop JITEngine::compileLoop(Chunk* chunk, uint8_t* startIp) {
    return nullptr;
}
} // namespace kio
#endif
