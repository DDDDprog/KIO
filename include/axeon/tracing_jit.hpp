/*
 Copyright (c) 2025 Dipanjan Dhar
 SPDX-License-Identifier: GPL-3.0-only

 V8-style Tracing JIT Compiler for KIO
 Features:
 - Execution path tracing
 - Type check elimination
 - SIMD vectorization
 - Fast native code generation
*/

#pragma once

#include "axeon/bytecode.hpp"
#include "axeon/vm.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace kio {

// ============================================================================
// Type Feedback System
// ============================================================================

struct TypeFeedback {
    ValueType type = ValueType::VAL_NIL;
    bool is_monomorphic = true;  // Always same type
    bool is_number = false;
    bool is_integer = false;
    int sample_count = 0;
    
    void record(ValueType t) {
        sample_count++;
        if (type == ValueType::VAL_NIL) {
            type = t;
        } else if (type != t) {
            is_monomorphic = false;
        }
        if (t == ValueType::VAL_NUMBER) {
            is_number = true;
        }
    }
    
    bool canEliminateTypeCheck() const {
        return is_monomorphic && sample_count > 10;
    }
};

// ============================================================================
// Trace Instruction
// ============================================================================

struct TraceInstruction {
    OpCode opcode;
    uint8_t* instruction_ptr;
    std::vector<ValueType> operand_types;
    bool is_loop_back = false;
    
    TraceInstruction(OpCode op, uint8_t* ptr) 
        : opcode(op), instruction_ptr(ptr) {}
};

// ============================================================================
// Execution Trace
// ============================================================================

class Trace {
public:
    Trace(uint8_t* start_ip);
    
    void addInstruction(OpCode opcode, uint8_t* ptr);
    void addOperandType(ValueType type);
    void markLoopBack();
    
    const std::vector<TraceInstruction>& getInstructions() const { 
        return instructions_; 
    }
    
    uint8_t* getStartIP() const { return start_ip_; }
    
    // Analyze trace for optimization opportunities
    bool canVectorize() const;
    bool canEliminateTypeChecks() const;
    
private:
    uint8_t* start_ip_;
    std::vector<TraceInstruction> instructions_;
    std::unordered_map<int, TypeFeedback> type_feedback_;
};

// ============================================================================
// Code Buffer for Native Code Generation
// ============================================================================

class CodeBuffer {
public:
    CodeBuffer(size_t size = 64 * 1024);
    ~CodeBuffer();
    
    void* getBuffer() const { return buffer_; }
    size_t getSize() const { return size_; }
    size_t getOffset() const { return offset_; }
    
    void emitByte(uint8_t byte);
    void emitBytes(const uint8_t* bytes, size_t count);
    void emitInt32(int32_t value);
    void emitInt64(int64_t value);
    void emitDouble(double value);
    
    void* finalize();
    
private:
    void* buffer_;
    size_t size_;
    size_t offset_;
    
    bool allocateExecutableMemory();
    void makeExecutable();
};

// ============================================================================
// Native Code Generator (x86_64)
// ============================================================================

class NativeCodeGenerator {
public:
    enum class Register {
        RAX, RBX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15
    };
    
    NativeCodeGenerator(CodeBuffer& buffer);
    
    // Prologue/Epilogue
    void emitPrologue();
    void emitEpilogue();
    
    // Register operations
    void emitMov(Register dest, Register src);
    void emitMov(Register reg, int64_t imm);
    void emitMov(Register reg, void* addr);
    void emitMov(void* addr, Register reg);
    
    // Stack operations
    void emitPush(Register reg);
    void emitPop(Register reg);
    void emitAdd(Register reg, int32_t imm);
    void emitSub(Register reg, int32_t imm);
    
    // Arithmetic operations (SSE2 for doubles)
    void emitAddSD(Register xmm_dest, Register xmm_src);
    void emitSubSD(Register xmm_dest, Register xmm_src);
    void emitMulSD(Register xmm_dest, Register xmm_src);
    void emitDivSD(Register xmm_dest, Register xmm_src);
    
    // SIMD Vectorization (AVX2)
    void emitVAddPD(Register ymm_dest, Register ymm_src1, Register ymm_src2);
    void emitVMulPD(Register ymm_dest, Register ymm_src1, Register ymm_src2);
    void emitVLoadPD(Register ymm_dest, void* addr);
    void emitVStorePD(void* addr, Register ymm_src);
    
    // Comparison and branching
    void emitCmp(Register reg1, Register reg2);
    void emitJump(void* target);
    void emitJumpIfEqual(void* target);
    void emitJumpIfNotEqual(void* target);
    void emitJumpIfGreater(void* target);
    void emitJumpIfLess(void* target);
    
    // Type operations (with elimination)
    void emitTypeCheck(Register reg, ValueType expected_type, bool can_eliminate);
    void emitLoadValue(Register xmm_reg, Register stack_ptr, int offset);
    void emitStoreValue(Register stack_ptr, int offset, Register xmm_reg);
    
    // Loop operations
    void emitLoopStart(Register counter_reg, int limit);
    void emitLoopEnd(void* loop_start);
    
private:
    CodeBuffer& buffer_;
    
    // Helper methods
    void emitREX(bool w, bool r, bool x, bool b);
    void emitModRM(uint8_t mod, uint8_t reg, uint8_t rm);
    void emitSIB(uint8_t scale, uint8_t index, uint8_t base);
    uint8_t regToBits(Register reg);
};

// ============================================================================
// Tracing JIT Compiler
// ============================================================================

class TracingJIT {
public:
    typedef void (*CompiledTrace)(Value* stack, int& sp, int slots, 
                                   std::unordered_map<std::string, Value>& globals);
    
    TracingJIT();
    ~TracingJIT();
    
    // Start recording a trace at the given IP
    void startRecording(VM* vm, uint8_t* ip);
    
    // Record an instruction execution
    void recordInstruction(OpCode opcode, uint8_t* ip);
    
    // Record type information
    void recordType(int stack_slot, ValueType type);
    
    // Stop recording and compile the trace
    CompiledTrace stopRecordingAndCompile();
    
    // Check if currently recording
    bool isRecording() const { return state_ == TraceState::RECORDING; }
    
    // Get current trace
    Trace* getCurrentTrace() const { return current_trace_.get(); }
    
private:
    enum class TraceState {
        IDLE,
        RECORDING,
        COMPILING
    };
    
    TraceState state_;
    std::unique_ptr<Trace> current_trace_;
    VM* current_vm_;
    
    std::unique_ptr<CodeBuffer> code_buffer_;
    std::unique_ptr<NativeCodeGenerator> code_gen_;
    
    // Type feedback
    std::unordered_map<int, TypeFeedback> type_feedback_;
    
    // Cache of compiled traces
    std::unordered_map<uint8_t*, CompiledTrace> compiled_traces_;
    
    // Compilation methods
    CompiledTrace compileTrace(const Trace& trace);
    void generateCodeForInstruction(const TraceInstruction& inst);
    void optimizeAndVectorize();
};

// ============================================================================
// Inline Cache for Property Access
// ============================================================================

class InlineCache {
public:
    static constexpr int CACHE_SIZE = 4;
    
    struct CacheEntry {
        void* handler;
        ValueType type;
        bool valid;
    };
    
    InlineCache();
    
    // Lookup in cache
    CacheEntry* lookup(ValueType type);
    
    // Update cache
    void update(ValueType type, void* handler);
    
    // Clear cache
    void clear();
    
private:
    CacheEntry entries_[CACHE_SIZE];
    size_t next_entry_;
};

// ============================================================================
// Hot Path Optimizer
// ============================================================================

class HotPathOptimizer {
public:
    HotPathOptimizer();
    
    // Record execution at a specific IP
    void recordExecution(uint8_t* ip);
    
    // Check if a path is hot
    bool isHot(uint8_t* ip) const;
    
    // Get execution count
    int getExecutionCount(uint8_t* ip) const;
    
    // Reset counters
    void reset();
    
private:
    static constexpr int HOT_THRESHOLD = 1000;
    
    std::unordered_map<uint8_t*, int> execution_counts_;
};

// ============================================================================
// Optimizing Compiler that uses Tracing JIT
// ============================================================================

class OptimizingCompiler {
public:
    OptimizingCompiler();
    ~OptimizingCompiler();
    
    // Optimize a chunk using tracing JIT
    void optimize(Chunk* chunk, VM* vm);
    
    // Check if optimization is needed
    bool shouldOptimize(Chunk* chunk) const;
    
    // Get compiled function for a loop
    TracingJIT::CompiledTrace getCompiledLoop(uint8_t* ip);
    
    // Record execution for hot path detection
    void recordExecution(uint8_t* ip);
    
    // Check if a path is hot and should be optimized
    bool isHotPath(uint8_t* ip) const;
    
private:
    std::unique_ptr<TracingJIT> tracing_jit_;
    std::unique_ptr<HotPathOptimizer> hot_path_optimizer_;
    std::unique_ptr<InlineCache> inline_cache_;
    
    std::unordered_map<Chunk*, bool> optimized_chunks_;
};

} // namespace kio
