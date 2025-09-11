#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace kio {

class ASTNode; // Forward declaration
class Expr; // Forward declaration
class Stmt; // Forward declaration

class Optimizer {
public:
    Optimizer();
    ~Optimizer();
    
    // Optimize AST
    std::vector<std::unique_ptr<ASTNode>> optimize(
        const std::vector<std::unique_ptr<ASTNode>>& ast
    );
    
    // Individual optimization passes
    std::unique_ptr<Expr> optimizeExpression(std::unique_ptr<Expr> expr);
    std::unique_ptr<Stmt> optimizeStatement(std::unique_ptr<Stmt> stmt);
    
    // Configuration
    void enableOptimization(const std::string& name, bool enabled = true);
    void disableOptimization(const std::string& name);
    bool isOptimizationEnabled(const std::string& name) const;
    
    // Statistics
    size_t getOptimizationCount() const { return optimization_count_; }
    void resetStatistics();

private:
    std::unordered_map<std::string, bool> enabled_optimizations_;
    size_t optimization_count_;
    
    // Optimization passes
    std::unique_ptr<Expr> constantFolding(std::unique_ptr<Expr> expr);
    std::unique_ptr<Expr> deadCodeElimination(std::unique_ptr<Expr> expr);
    std::unique_ptr<Expr> commonSubexpressionElimination(std::unique_ptr<Expr> expr);
    std::unique_ptr<Stmt> loopOptimization(std::unique_ptr<Stmt> stmt);
    
    void initializeOptimizations();
};

} // namespace kio
