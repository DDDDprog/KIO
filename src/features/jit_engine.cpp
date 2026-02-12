/*
 Copyright (c) 2025 Dipanjan Dhar
 SPDX-License-Identifier: GPL-3.0-only
 */

#include "kio/jit_engine.hpp"
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
        jit_builder.setNumCompileThreads(0);
        
        auto jit_or_err = jit_builder.create();
        if (jit_or_err) {
            lljit = std::move(*jit_or_err);
        } else {
            lljit = nullptr;
        }
    }

    void optimizeModule(llvm::Module* M) {
        // Create the analysis managers.
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager MAM;

        // Create the new pass manager builder.
        PassBuilder PB;

        // Register all the basic analyses with the managers.
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

        // Create the pass manager with O3 optimizations.
        ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O3);

        // Run the passes.
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
    int limit = 1000;
    
    while (!scan_done && limit-- > 0 && scan < chunk->code.data() + chunk->code.size()) {
        OpCode op = (OpCode)(*scan);
        switch (op) {
            case OpCode::GET_LOCAL:
            case OpCode::SET_LOCAL: {
                uint8_t slot = scan[1];
                if (slot > max_slot) max_slot = slot;
                scan += 2;
                break;
            }
            case OpCode::CONSTANT: scan += 2; break;
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
                return nullptr;
            default: 
                scan += 1; 
                break;
        }
    }
    
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

    std::vector<llvm::Value*> localAllocas(max_slot + 1, nullptr);
    for (int i = 0; i <= max_slot; i++) {
        localAllocas[i] = builder.CreateAlloca(doubleTy, nullptr, "local_" + std::to_string(i));
    }
    
    for (int i = 0; i <= max_slot; i++) {
            llvm::Value* idx = builder.CreateAdd(slotsOffset, builder.getInt32(i));
            llvm::Value* offset = builder.CreateMul(idx, builder.getInt32(16));
            llvm::Value* valAddr = builder.CreateInBoundsGEP(builder.getInt8Ty(), stackBase, offset);
            llvm::Value* dataAddr = builder.CreateConstInBoundsGEP1_32(builder.getInt8Ty(), valAddr, 8);
            llvm::Value* rawVal = builder.CreateLoad(doubleTy, dataAddr);
            builder.CreateStore(rawVal, localAllocas[i]);
    }

    builder.CreateBr(loopDetailsBB);
    builder.SetInsertPoint(loopDetailsBB);
    builder.CreateBr(loopBodyBB);
    builder.SetInsertPoint(loopBodyBB);

    llvm::SmallVector<llvm::Metadata*, 4> loopMetadata;
    llvm::LLVMContext& ctx = *impl_->context;
    llvm::TempMDTuple tempNode = llvm::MDNode::getTemporary(ctx, {});
    loopMetadata.push_back(tempNode.get());
    loopMetadata.push_back(llvm::MDNode::get(ctx, { 
        llvm::MDString::get(ctx, "llvm.loop.unroll.enable") 
    }));
    llvm::MDNode* loopID = llvm::MDNode::get(ctx, loopMetadata);
    tempNode->replaceAllUsesWith(loopID);
    
    std::vector<llvm::Value*> simStack;

    uint8_t* ip = startIp;
    bool compiling = true;
    
    while (compiling) {
        OpCode op = (OpCode)(*ip++);
        switch (op) {
            case OpCode::CONSTANT: {
                uint8_t idx = *ip++;
                Value v = chunk->constants[idx];
                if (v.type == ValueType::VAL_NUMBER) {
                    simStack.push_back(llvm::ConstantFP::get(doubleTy, v.as.number));
                } else if (v.type == ValueType::VAL_BOOL) {
                        simStack.push_back(llvm::ConstantFP::get(doubleTy, v.as.boolean ? 1.0 : 0.0));
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
            case OpCode::MULTIPLY:
            case OpCode::DIVIDE: {
                if (simStack.size() < 2) return nullptr;
                llvm::Value* b = simStack.back(); simStack.pop_back();
                llvm::Value* a = simStack.back(); simStack.pop_back();
                llvm::Value* res = nullptr;
                if (op == OpCode::ADD) res = builder.CreateFAdd(a, b);
                else if (op == OpCode::SUBTRACT) res = builder.CreateFSub(a, b);
                else if (op == OpCode::MULTIPLY) res = builder.CreateFMul(a, b);
                else if (op == OpCode::DIVIDE) res = builder.CreateFDiv(a, b);
                simStack.push_back(res);
                break;
            }
            case OpCode::LESS:
            case OpCode::GREATER: 
            case OpCode::EQUAL: {
                    if (simStack.size() < 2) return nullptr;
                    llvm::Value* b = simStack.back(); simStack.pop_back();
                    llvm::Value* a = simStack.back(); simStack.pop_back();
                    llvm::Value* cmp = nullptr;
                    if (op == OpCode::LESS) cmp = builder.CreateFCmpOLT(a, b);
                    else if (op == OpCode::GREATER) cmp = builder.CreateFCmpOGT(a, b);
                    else cmp = builder.CreateFCmpOEQ(a, b);
                    simStack.push_back(builder.CreateUIToFP(cmp, doubleTy));
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
                auto br = builder.CreateBr(loopBodyBB);
                br->setMetadata(llvm::LLVMContext::MD_loop, loopID);
                compiling = false;
                break;
            }
            case OpCode::POP: { 
                if (!simStack.empty()) simStack.pop_back(); 
                break; 
            }
            default: return nullptr;
        }
    }
    
    builder.SetInsertPoint(exitBB);
    
    for (int i = 0; i <= max_slot; i++) {
            llvm::Value* idx = builder.CreateAdd(slotsOffset, builder.getInt32(i));
            llvm::Value* offset = builder.CreateMul(idx, builder.getInt32(16));
            llvm::Value* valAddr = builder.CreateInBoundsGEP(builder.getInt8Ty(), stackBase, offset);
            
            llvm::Value* typeAddr = valAddr;
            llvm::Value* typePtr = builder.CreateBitCast(typeAddr, builder.getInt32Ty()->getPointerTo());
            builder.CreateStore(builder.getInt32((int)ValueType::VAL_NUMBER), typePtr);

            llvm::Value* dataAddr = builder.CreateConstInBoundsGEP1_32(builder.getInt8Ty(), valAddr, 8);
            llvm::Value* val = builder.CreateLoad(doubleTy, localAllocas[i]);
            builder.CreateStore(val, dataAddr);
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
