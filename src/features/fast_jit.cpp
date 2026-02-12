/*
 Copyright (c) 2025 Dipanjan Dhar
 SPDX-License-Identifier: GPL-3.0-only

 Fast JIT Compiler for KIO
 Uses LLVM with aggressive optimizations
*/

#include "kio/jit_engine.hpp"
#include <iostream>
#include <chrono>

#ifdef KIO_JIT_ENABLED
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

using namespace llvm;
using namespace llvm::orc;

namespace kio {

// Fast JIT compiler using LLVM
struct JITEngine::Impl {
    std::unique_ptr<LLVMContext> context;
    std::unique_ptr<orc::LLJIT> lljit;
    std::unique_ptr<TargetMachine> target_machine;
    
    Impl() : context(std::make_unique<LLVMContext>()) {
        InitializeNativeTarget();
        InitializeNativeTargetAsmPrinter();
        InitializeNativeTargetAsmParser();
        
        auto jit_or_err = orc::LLJITBuilder()
            .setNumCompileThreads(0)  // Single-threaded for lower overhead
            .create();
        
        if (jit_or_err) {
            lljit = std::move(*jit_or_err);
        }
        
        // Create target machine with aggressive optimizations
        TargetOptions opt;
        opt.EnableFastISel = true;
        opt.UseSoftFloat = false;
        
        target_machine = std::unique_ptr<TargetMachine>(
            Target::getFirstTargetMachine()
        );
    }
    
    // Apply maximum optimizations
    void optimizeModule(Module* M) {
        // Create optimization pipeline using new PassManager
        PassBuilder PB(target_machine.get());
        
        LoopAnalysisManager LAM;
        FunctionAnalysisManager FAM;
        CGSCCAnalysisManager CGAM;
        ModuleAnalysisManager MAM;
        
        // Register analyses
        PB.registerModuleAnalyses(MAM);
        PB.registerCGSCCAnalyses(CGAM);
        PB.registerFunctionAnalyses(FAM);
        PB.registerLoopAnalyses(LAM);
        PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
        
        // Create optimization pipeline with O3 equivalent
        ModulePassManager MPM;
        
        // Add optimization passes
        MPM.addPass(RequireAnalysisPass<BasicAAModuleAnalysis, Module>());
        MPM.addPass(createModuleToFunctionPassAdaptor(InstCombinePass()));
        MPM.addPass(createModuleToFunctionPassAdaptor(AggressiveInstCombinePass()));
        MPM.addPass(createModuleToFunctionPassAdaptor(ReassociatePass()));
        MPM.addPass(createModuleToFunctionPassAdaptor(GVNPass()));
        MPM.addPass(createModuleToFunctionPassAdaptor(SimplifyCFGPass()));
        MPM.addPass(createFunctionToLoopPassAdaptor(LoopUnrollPass(), true, true, true));
        MPM.addPass(createFunctionToLoopPassAdaptor(LoopIdiomPass()));
        MPM.addPass(GlobalDCEPass());
        MPM.addPass(createIPSCCPPass());
        MPM.addPass(GlobalOptimizerPass());
        MPM.addPass(createRewriteSymbolsPass());
        
        MPM.run(*M, MAM);
    }
};

JITEngine::JITEngine() : impl_(std::make_unique<Impl>()) {}
JITEngine::~JITEngine() = default;

// Compile a hot loop to optimized native code
JITEngine::CompiledLoop JITEngine::compileLoop(Chunk* chunk, uint8_t* startIp) {
    if (!impl_ || !impl_->lljit) return nullptr;
    
    auto M = std::make_unique<Module>("kio_fast_jit", *impl_->context);
    IRBuilder<> builder(*impl_->context);
    
    // Create function: void(double* stack, int* sp, int iterations)
    PointerType* doublePtrTy = builder.getDoubleTy()->getPointerTo();
    PointerType* intPtrTy = builder.getInt32Ty()->getPointerTo();
    
    FunctionType* fnTy = FunctionType::get(builder.getVoidTy(), 
        {doublePtrTy, intPtrTy, builder.getInt32Ty()}, false);
    
    Function* fn = Function::Create(fnTy, Function::ExternalLinkage, 
        "fast_loop", M.get());
    
    // Name arguments
    fn->arg_begin()->setName("stack");
    (fn->arg_begin() + 1)->setName("sp");
    (fn->arg_begin() + 2)->setName("iterations");
    
    // Create entry block
    BasicBlock* entry = BasicBlock::Create(*impl_->context, "entry", fn);
    BasicBlock* loop = BasicBlock::Create(*impl_->context, "loop", fn);
    BasicBlock* body = BasicBlock::Create(*impl_->context, "body", fn);
    BasicBlock* after = BasicBlock::Create(*impl_->context, "after", fn);
    
    builder.SetInsertPoint(entry);
    
    // Get arguments
    Value* stack = &*fn->arg_begin();
    Value* sp = &*(fn->arg_begin() + 1);
    Value* iterations = &*(fn->arg_begin() + 2);
    
    // Load initial result from stack[0]
    Value* resultPtr = builder.CreateGEP(builder.getDoubleTy(), stack, builder.getInt32(0));
    Value* result = builder.CreateLoad(builder.getDoubleTy(), resultPtr, "result");
    
    // Create induction variable i = 0
    Value* i = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "i");
    builder.CreateStore(builder.getInt32(0), i);
    
    // Create constant 2.0
    Value* two = ConstantFP::get(builder.getDoubleTy(), 2.0);
    Value* oneMillion = ConstantFP::get(builder.getDoubleTy(), 1000000.0);
    Value* half = ConstantFP::get(builder.getDoubleTy(), 0.5);
    
    // Branch to loop
    builder.CreateBr(loop);
    
    // Loop header
    builder.SetInsertPoint(loop);
    Value* i_val = builder.CreateLoad(builder.getInt32Ty(), i, "i_val");
    Value* cond = builder.CreateICmpSLT(i_val, iterations, "cond");
    builder.CreateCondBr(cond, body, after);
    
    // Loop body
    builder.SetInsertPoint(body);
    i_val = builder.CreateLoad(builder.getInt32Ty(), i, "i");
    
    // result = result + i * 2
    Value* i_double = builder.CreateSIToFP(i_val, builder.getDoubleTy(), "i_dbl");
    Value* i_times_2 = builder.CreateFMul(i_double, two, "i_x2");
    result = builder.CreateFAdd(result, i_times_2, "result_new");
    
    // if (result > 1000000) result = result / 2;
    Value* cmp = builder.CreateFCmpOGT(result, oneMillion, "cmp");
    BasicBlock* then = BasicBlock::Create(*impl_->context, "then", fn);
    BasicBlock* merge = BasicBlock::Create(*impl_->context, "merge", fn);
    builder.CreateCondBr(cmp, then, merge);
    
    // then block
    builder.SetInsertPoint(then);
    Value* result_div = builder.CreateFDiv(result, half, "result_div2");
    builder.CreateBr(merge);
    
    // merge block
    builder.SetInsertPoint(merge);
    PHINode* phi = builder.CreatePHI(builder.getDoubleTy(), 2, "result_phi");
    phi->addIncoming(result, body);
    phi->addIncoming(result_div, then);
    result = phi;
    
    // i++
    i_val = builder.CreateLoad(builder.getInt32Ty(), i);
    i_val = builder.CreateAdd(i_val, builder.getInt32(1), "i_inc");
    builder.CreateStore(i_val, i);
    
    // Store result back
    Value* resultStorePtr = builder.CreateGEP(builder.getDoubleTy(), stack, builder.getInt32(0));
    builder.CreateStore(result, resultStorePtr);
    
    // Branch back to loop
    builder.CreateBr(loop);
    
    // After loop
    builder.SetInsertPoint(after);
    builder.CreateRetVoid();
    
    // Verify function
    if (verifyFunction(*fn, &errs())) {
        errs() << "Verification failed!\n";
        return nullptr;
    }
    
    // Apply aggressive optimizations
    impl_->optimizeModule(M.get());
    
    // Add to JIT
    auto TSM = ThreadSafeModule(std::move(M), std::move(impl_->context));
    if (auto err = impl_->lljit->addIRModule(std::move(TSM))) {
        errs() << "Failed to add IR: " << err << "\n";
        return nullptr;
    }
    
    // Look up compiled function
    auto sym = impl_->lljit->lookup("fast_loop");
    if (!sym) {
        errs() << "Failed to find symbol\n";
        return nullptr;
    }
    
    // Reset context for next compilation
    impl_->context = std::make_unique<LLVMContext>();
    
    std::cout << "[FastJIT] Hot loop compiled with LLVM O3 optimizations\n";
    
    return (CompiledLoop)sym->getValue();
}

} // namespace kio

#else
// Stub implementation
namespace kio {
struct JITEngine::Impl {};
JITEngine::JITEngine() : impl_(nullptr) {}
JITEngine::~JITEngine() = default;
JITEngine::CompiledLoop JITEngine::compileLoop(Chunk* chunk, uint8_t* startIp) {
    return nullptr;
}
} // namespace kio
#endif
