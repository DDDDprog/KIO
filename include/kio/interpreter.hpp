/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#pragma once

#include <unordered_map>
#include <variant>
#include <string>
#include <memory>
#include "kio/ast.hpp"

namespace kio {

// Minimal runtime value type used by the current interpreter implementation
using Value = std::variant<std::monostate, double, std::string>;

class Interpreter {
public:
    // Use compiler-generated constructor/destructor
    void interpret(const std::vector<StmtPtr> &statements);

private:
    std::unordered_map<std::string, Value> environment_;

    void execute(const StmtPtr &stmt);
    Value evaluate(const ExprPtr &expr);

    Value evaluateLiteral(const Expr::Literal &lit);
    Value evaluateVariable(const Expr::Variable &var);
    Value evaluateBinary(const Expr::Binary &bin);
    Value evaluateGrouping(const Expr::Grouping &grp);
    Value evaluateAssign(const Expr::Assign &as);
    Value evaluateSysQuery(const Expr::SysQuery &sq);

    void executePrint(const Stmt::Print &printStmt);
    void executeVar(const Stmt::Var &varStmt);
    void executeExpression(const Stmt::Expression &exprStmt);
    void executeSave(const Stmt::Save &saveStmt);
    void executeLoad(const Stmt::Load &loadStmt);
    void executeImport(const Stmt::Import &importStmt);
};

} // namespace kio
