/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "kio/interpreter.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <regex>
#include <sys/statvfs.h>
#include <chrono>
#include <cmath>
#include "kio/lexer.hpp"
#include "kio/parser.hpp"

namespace kio {

static double asNumber(const Value &v) {
    if (auto p = std::get_if<double>(&v)) return *p;
    throw std::runtime_error("Operand must be a number.");
}

void Interpreter::interpret(const std::vector<StmtPtr> &statements) {
    for (const auto &stmt : statements) {
        execute(stmt);
    }
}

void Interpreter::execute(const StmtPtr &stmt) {
    std::visit([&](auto &&node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Stmt::Print>) {
            executePrint(node);
        } else if constexpr (std::is_same_v<T, Stmt::Var>) {
            executeVar(node);
        } else if constexpr (std::is_same_v<T, Stmt::Expression>) {
            executeExpression(node);
        } else if constexpr (std::is_same_v<T, Stmt::Save>) {
            executeSave(node);
        } else if constexpr (std::is_same_v<T, Stmt::Load>) {
            executeLoad(node);
        } else if constexpr (std::is_same_v<T, Stmt::Import>) {
            executeImport(node);
        }
    }, stmt->node);
}

Value Interpreter::evaluate(const ExprPtr &expr) {
    return std::visit([&](auto &&node) -> Value {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Expr::Literal>) return evaluateLiteral(node);
        if constexpr (std::is_same_v<T, Expr::Variable>) return evaluateVariable(node);
        if constexpr (std::is_same_v<T, Expr::Binary>) return evaluateBinary(node);
        if constexpr (std::is_same_v<T, Expr::Grouping>) return evaluateGrouping(node);
        if constexpr (std::is_same_v<T, Expr::Assign>) return evaluateAssign(node);
        if constexpr (std::is_same_v<T, Expr::SysQuery>) return evaluateSysQuery(node);
        return {};
    }, expr->node);
}

Value Interpreter::evaluateLiteral(const Expr::Literal &lit) {
    if (std::holds_alternative<double>(lit.value)) {
        return std::get<double>(lit.value);
    }
    return std::get<std::string>(lit.value);
}

Value Interpreter::evaluateVariable(const Expr::Variable &var) {
    auto it = environment_.find(var.name);
    if (it == environment_.end()) throw std::runtime_error("Undefined variable: " + var.name);
    return it->second;
}

Value Interpreter::evaluateBinary(const Expr::Binary &bin) {
    Value left = evaluate(bin.left);
    Value right = evaluate(bin.right);
    switch (bin.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right))
                return std::get<double>(left) + std::get<double>(right);
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right))
                return std::get<std::string>(left) + std::get<std::string>(right);
            // Support number+string and string+number by coercion
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<double>(right)) {
                return std::get<std::string>(left) + std::to_string(std::get<double>(right));
            }
            if (std::holds_alternative<double>(left) && std::holds_alternative<std::string>(right)) {
                return std::to_string(std::get<double>(left)) + std::get<std::string>(right);
            }
            throw std::runtime_error("Operands must be numbers or strings for '+'.");
        case TokenType::MINUS: return asNumber(left) - asNumber(right);
        case TokenType::STAR: return asNumber(left) * asNumber(right);
        case TokenType::SLASH: return asNumber(left) / asNumber(right);
        case TokenType::PERCENT: return std::fmod(asNumber(left), asNumber(right));
        case TokenType::EQUAL_EQUAL: {
            if (left.index() != right.index()) return 0.0;
            if (std::holds_alternative<double>(left))
                return std::get<double>(left) == std::get<double>(right) ? 1.0 : 0.0;
            if (std::holds_alternative<std::string>(left))
                return std::get<std::string>(left) == std::get<std::string>(right) ? 1.0 : 0.0;
            return 1.0; // both monostate
        }
        case TokenType::BANG_EQUAL: {
            if (left.index() != right.index()) return 1.0;
            if (std::holds_alternative<double>(left))
                return std::get<double>(left) != std::get<double>(right) ? 1.0 : 0.0;
            if (std::holds_alternative<std::string>(left))
                return std::get<std::string>(left) != std::get<std::string>(right) ? 1.0 : 0.0;
            return 0.0;
        }
        case TokenType::GREATER:
            return asNumber(left) > asNumber(right) ? 1.0 : 0.0;
        case TokenType::GREATER_EQUAL:
            return asNumber(left) >= asNumber(right) ? 1.0 : 0.0;
        case TokenType::LESS:
            return asNumber(left) < asNumber(right) ? 1.0 : 0.0;
        case TokenType::LESS_EQUAL:
            return asNumber(left) <= asNumber(right) ? 1.0 : 0.0;
        default: break;
    }
    return {};
}

Value Interpreter::evaluateGrouping(const Expr::Grouping &grp) {
    return evaluate(grp.expression);
}

void Interpreter::executePrint(const Stmt::Print &printStmt) {
    Value v = evaluate(printStmt.expression);
    if (std::holds_alternative<double>(v)) std::cout << std::get<double>(v) << "\n";
    else if (std::holds_alternative<std::string>(v)) std::cout << std::get<std::string>(v) << "\n";
    else std::cout << "nil\n";
}

void Interpreter::executeVar(const Stmt::Var &varStmt) {
    Value v = evaluate(varStmt.initializer);
    environment_[varStmt.name] = v;
}

void Interpreter::executeExpression(const Stmt::Expression &exprStmt) {
    (void)evaluate(exprStmt.expression);
}
static std::string readFileQuick(const std::string &path) {
    std::ifstream ifs(path);
    std::stringstream ss; ss << ifs.rdbuf();
    return ss.str();
}

Value Interpreter::evaluateSysQuery(const Expr::SysQuery &sq) {
    const std::string &k = sq.key;
    // Linux-only basic implementation
    if (k == "time") {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
        return static_cast<double>(ms);
    }
    if (k == "cpu_model") {
        std::ifstream ifs("/proc/cpuinfo");
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.rfind("model name", 0) == 0) {
                auto pos = line.find(':');
                if (pos != std::string::npos) return line.substr(pos + 2);
            }
        }
        return std::string("");
    }
    if (k == "mem_total_kb") {
        std::ifstream ifs("/proc/meminfo");
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.rfind("MemTotal:", 0) == 0) {
                std::stringstream ss(line.substr(9));
                double kb; ss >> kb; return kb;
            }
        }
        return 0.0;
    }
    if (k == "disk_root_kb") {
        struct statvfs vfs{};
        if (statvfs("/", &vfs) == 0) {
            double kb = (double)vfs.f_blocks * (double)vfs.f_frsize / 1024.0;
            return kb;
        }
        return 0.0;
    }
    if (k == "os_name") {
        return std::string("Linux");
    }
    return std::monostate{};
}
static std::string escapeJson(const std::string &s) {
    std::string o; o.reserve(s.size());
    for (char c : s) {
        if (c == '"' || c == '\\') { o.push_back('\\'); o.push_back(c); }
        else if (c == '\n') { o += "\\n"; }
        else { o.push_back(c); }
    }
    return o;
}

void Interpreter::executeSave(const Stmt::Save &saveStmt) {
    std::ofstream ofs(saveStmt.path);
    ofs << "{\n  \"vars\": {";
    bool first = true;
    for (const auto &kv : environment_) {
        if (!first) ofs << ",";
        first = false;
        ofs << "\n    \"" << escapeJson(kv.first) << "\": ";
        if (std::holds_alternative<double>(kv.second)) {
            ofs << std::get<double>(kv.second);
        } else if (std::holds_alternative<std::string>(kv.second)) {
            ofs << "\"" << escapeJson(std::get<std::string>(kv.second)) << "\"";
        } else {
            ofs << "null";
        }
    }
    ofs << "\n  }\n}\n";
}

void Interpreter::executeLoad(const Stmt::Load &loadStmt) {
    std::ifstream ifs(loadStmt.path);
    if (!ifs) throw std::runtime_error("Cannot open file: " + loadStmt.path);
    std::string line;
    bool inVars = false;
    while (std::getline(ifs, line)) {
        // detect start of vars object
        if (!inVars) {
            if (line.find("\"vars\"") != std::string::npos && line.find('{') != std::string::npos) {
                inVars = true;
            }
            continue;
        }
        if (line.find('}') != std::string::npos) break;
        // parse:    "key": value
        auto q1 = line.find('"');
        if (q1 == std::string::npos) continue;
        auto q2 = line.find('"', q1 + 1);
        if (q2 == std::string::npos) continue;
        std::string key = line.substr(q1 + 1, q2 - (q1 + 1));
        auto colon = line.find(':', q2);
        if (colon == std::string::npos) continue;
        std::string val = line.substr(colon + 1);
        // trim spaces and trailing comma
        auto lpos = val.find_first_not_of(" \t");
        auto rpos = val.find_last_not_of(" \t,\r\n");
        if (lpos == std::string::npos) continue;
        val = val.substr(lpos, rpos - lpos + 1);
        if (!val.empty() && val.front() == '"' && val.back() == '"') {
            environment_[key] = val.substr(1, val.size() - 2);
        } else {
            try { environment_[key] = std::stod(val); }
            catch (...) { environment_[key] = std::monostate{}; }
        }
    }
}

void Interpreter::executeImport(const Stmt::Import &importStmt) {
    // Load a module file and execute it in current environment
    std::ifstream ifs(importStmt.path);
    if (!ifs) throw std::runtime_error("Cannot open module: " + importStmt.path);
    std::stringstream buf; buf << ifs.rdbuf();
    Lexer lx(buf.str());
    auto toks = lx.scanTokens();
    Parser ps(toks);
    auto stmts = ps.parse();
    for (const auto &s : stmts) execute(s);
}

Value Interpreter::evaluateAssign(const Expr::Assign &as) {
    Value v = evaluate(as.value);
    auto it = environment_.find(as.name);
    if (it == environment_.end()) throw std::runtime_error("Undefined variable: " + as.name);
    it->second = v;
    return v;
}

} // namespace kio
