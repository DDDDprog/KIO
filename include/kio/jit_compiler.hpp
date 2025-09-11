#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include "kio/ast.hpp"

namespace kio {

class Value;

class JITCompiler {
public:
    JITCompiler();
    ~JITCompiler();
    
    // Compile frequently executed code paths to native machine code
    bool compileFunction(const std::string &name, const std::vector<StmtPtr> &body);
    bool compileExpression(const ExprPtr &expr);
    
    // Execute compiled code
    Value executeCompiledFunction(const std::string &name, const std::vector<Value> &args);
    
    // Performance monitoring
    void markHotPath(const std::string &path);
    bool isHotPath(const std::string &path) const;
    
    // Optimization levels
    void setOptimizationLevel(int level);
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    std::unordered_map<std::string, int> hot_paths_;
    int optimization_level_ = 2;
    static constexpr int HOT_PATH_THRESHOLD = 100;
};

} // namespace kio
