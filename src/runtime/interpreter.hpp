/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/
#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "axeon/ast.hpp"
#include "axeon/bytecode.hpp"

namespace kio {

class Interpreter {
public:
    void interpret(const std::vector<StmtPtr> &statements);

private:
    std::unordered_map<std::string, Value> environment_;

    void execute(const StmtPtr &stmt);
    Value evaluate(const ExprPtr &expr);

    void executePrint(const Stmt::Print &printStmt);
    void executeVar(const Stmt::Var &varStmt);
    void executeExpression(const Stmt::Expression &exprStmt);
    void executeBlock(const Stmt::Block &blockStmt);
    void executeIf(const Stmt::If &ifStmt);
    void executeWhile(const Stmt::While &whileStmt);
    void executeFor(const Stmt::For &forStmt);   
    void executeForIn(const Stmt::ForIn &forInStmt);
    void executeSave(const Stmt::Save &saveStmt);
    void executeLoad(const Stmt::Load &loadStmt);
    void executeImport(const Stmt::Import &importStmt);
    void executeImportFrom(const Stmt::ImportFrom &importFromStmt);
    void executeModule(const Stmt::Module &moduleStmt);
    void executeExport(const Stmt::Export &exportStmt);
    void executeParallel(const Stmt::Parallel &parallelStmt);
    Value evaluateLiteral(const Expr::Literal &lit);
    Value evaluateVariable(const Expr::Variable &var);
    Value evaluateBinary(const Expr::Binary &bin);
    Value evaluateGrouping(const Expr::Grouping &grp);
    Value evaluateAssign(const Expr::Assign &as);
    Value evaluateSysQuery(const Expr::SysQuery &sq);
    Value evaluateLogical(const Expr::Logical &logical);
    Value evaluatePostOp(const Expr::PostOp &post);
};

} // namespace kio
