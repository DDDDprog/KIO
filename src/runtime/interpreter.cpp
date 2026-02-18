/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/interpreter.hpp"
#include "axeon/platform.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>
#include <cmath>
#include <ctime>

namespace kio {

static std::string valToString(Value v) {
    if (isNil(v)) return "nil";
    if (v == TRUE_VAL) return "true";
    if (v == FALSE_VAL) return "false";
    if (isNumber(v)) {
        char buf[64]; snprintf(buf, sizeof(buf), "%.15g", valueToDouble(v));
        return std::string(buf);
    }
    if (isObj(v)) {
        Obj* o = valueToObj(v);
        if (o->type == ObjType::OBJ_STRING) return ((ObjString*)o)->chars;
        return "[Object]";
    }
    return "";
}

static double asNumber(Value v) {
    if (isNumber(v)) return valueToDouble(v);
    throw std::runtime_error("Operand must be a number.");
}

static bool isTruthy(Value v) {
    if (isNil(v)) return false;
    if (v == FALSE_VAL) return false;
    if (isNumber(v)) return valueToDouble(v) != 0.0;
    if (isObj(v)) {
         Obj* o = valueToObj(v);
         if (o->type == ObjType::OBJ_STRING) return !((ObjString*)o)->chars.empty();
    }
    return true;
}

void Interpreter::interpret(const std::vector<StmtPtr> &statements) {
    for (const auto &stmt : statements) { execute(stmt); }
}

void Interpreter::execute(const StmtPtr &stmt) {
    std::visit([&](auto &&node) {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Stmt::Print>) executePrint(node);
        else if constexpr (std::is_same_v<T, Stmt::Var>) executeVar(node);
        else if constexpr (std::is_same_v<T, Stmt::Expression>) (void)evaluate(node.expression);
        else if constexpr (std::is_same_v<T, Stmt::Block>) for (const auto &s : node.statements) execute(s);
        else if constexpr (std::is_same_v<T, Stmt::If>) {
            if (isTruthy(evaluate(node.condition))) execute(node.thenBranch);
            else if (node.elseBranch) execute(node.elseBranch);
        } else if constexpr (std::is_same_v<T, Stmt::While>) {
            while (isTruthy(evaluate(node.condition))) execute(node.body);
        } else if constexpr (std::is_same_v<T, Stmt::ForIn>) {
            Value iterable = evaluate(node.iterable);
            if (isNumber(iterable)) {
                double max = valueToDouble(iterable);
                for (double i = 0; i < max; ++i) {
                    environment_[node.name] = doubleToValue(i);
                    execute(node.body);
                }
            }
        }
    }, stmt->node);
}

Value Interpreter::evaluate(const ExprPtr &expr) {
    return std::visit([&](auto &&node) -> Value {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, Expr::Literal>) {
            if (std::holds_alternative<double>(node.value)) return doubleToValue(std::get<double>(node.value));
            std::string s = std::get<std::string>(node.value);
            if (s == "true") return TRUE_VAL;
            if (s == "false") return FALSE_VAL;
            return objToValue(new ObjString(s));
        }
        if constexpr (std::is_same_v<T, Expr::Variable>) return environment_[node.name];
        if constexpr (std::is_same_v<T, Expr::Assign>) return environment_[node.name] = evaluate(node.value);
        if constexpr (std::is_same_v<T, Expr::Binary>) {
            Value left = evaluate(node.left), right = evaluate(node.right);
            switch (node.op.type) {
                case TokenType::PLUS:
                    if (isNumber(left) && isNumber(right)) return doubleToValue(valueToDouble(left) + valueToDouble(right));
                    if (isObj(left) && isObj(right)) return objToValue(new ObjString(((ObjString*)valueToObj(left))->chars + ((ObjString*)valueToObj(right))->chars));
                    return NIL_VAL;
                case TokenType::MINUS:
                    return doubleToValue(asNumber(left) - asNumber(right));
                case TokenType::STAR:
                    return doubleToValue(asNumber(left) * asNumber(right));
                case TokenType::SLASH:
                    return doubleToValue(asNumber(left) / asNumber(right));
                case TokenType::PERCENT:
                    return doubleToValue(std::fmod(asNumber(left), asNumber(right)));
                case TokenType::LESS:
                    return BOOL_VAL(asNumber(left) < asNumber(right));
                case TokenType::GREATER:
                    return BOOL_VAL(asNumber(left) > asNumber(right));
                case TokenType::EQUAL_EQUAL:
                    return BOOL_VAL(left == right);
                default:
                    return NIL_VAL;
            }
        }
        if constexpr (std::is_same_v<T, Expr::SysQuery>) {
            if (node.key == "time") return doubleToValue(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now().time_since_epoch()).count());
            if (node.key == "os_name") {
#ifdef _WIN32
                return objToValue(new ObjString("Windows"));
#elif __APPLE__
                return objToValue(new ObjString("macOS"));
#else
                return objToValue(new ObjString("Linux"));
#endif
            }
            if (node.key == "arch") return objToValue(new ObjString("x64"));
            if (node.key == "kio_version") return objToValue(new ObjString("2.1.0"));
            if (node.key == "cpu_model") return objToValue(new ObjString(PlatformInfo::get_cpu_model()));
            if (node.key == "mem_total_kb") return doubleToValue((double)PlatformInfo::get_total_memory());
            if (node.key == "disk_root_kb") return doubleToValue((double)PlatformInfo::get_root_disk_space());
            return NIL_VAL;
        }
        return NIL_VAL;
    }, expr->node);
}

void Interpreter::executePrint(const Stmt::Print &s) { std::cout << valToString(evaluate(s.expression)) << std::endl; }
void Interpreter::executeVar(const Stmt::Var &s) { environment_[s.name] = evaluate(s.initializer); }

} // namespace kio
