/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/parser.hpp"
#include "axeon/error_recovery.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace kio {

// External globals from parser.cpp
extern std::string g_source_code;
extern std::string g_file_path;

static std::string getSourceLine(int line) {
    if (g_source_code.empty()) return "";
    std::istringstream iss(g_source_code);
    std::string result;
    int current_line = 1;
    while (std::getline(iss, result)) {
        if (current_line == line) return result;
        current_line++;
    }
    return "";
}

// Get multiple source lines for context
static std::vector<std::pair<int, std::string>> getSourceLines(int start_line, int end_line) {
    std::vector<std::pair<int, std::string>> result;
    if (g_source_code.empty()) return result;
    
    std::istringstream iss(g_source_code);
    std::string line;
    int current_line = 1;
    
    while (std::getline(iss, line)) {
        if (current_line >= start_line && current_line <= end_line) {
            result.push_back({current_line, line});
        }
        if (current_line > end_line) break;
        current_line++;
    }
    return result;
}

// Enhanced error function with error code and hint support
static std::runtime_error error(const Token &token, const char *message, 
                                 ErrorCode code = ErrorCode::E999_UNKNOWN, 
                                 const char *hint = nullptr) {
    std::ostringstream oss;
    oss << "\n";
    
    // Error header with code
    std::string code_str = "E" + std::string(3 - std::to_string(static_cast<int>(code)).length(), '0') + 
                           std::to_string(static_cast<int>(code));
    oss << colors::BOLD << colors::BRIGHT_RED << "error" << colors::RESET 
        << colors::BOLD << colors::DIM << "[" << code_str << "]" << colors::RESET
        << colors::BOLD << ": " << message << colors::RESET << "\n";
    
    oss << colors::DIM << "  --> " << colors::RESET 
        << colors::CYAN << (g_file_path.empty() ? "<stdin>" : g_file_path) << colors::RESET
        << ":" << token.line << ":" << token.column << "\n";
    
    // Show source code with context lines (2 before and 2 after)
    constexpr int CONTEXT_LINES = 2;
    int start_line = std::max(1, token.line - CONTEXT_LINES);
    int end_line = token.line + CONTEXT_LINES;
    
    auto lines = getSourceLines(start_line, end_line);
    
    if (!lines.empty()) {
        oss << colors::DIM << "   |" << colors::RESET << "\n";
        
        for (const auto& [line_num, line_content] : lines) {
            if (line_num == token.line) {
                // Error line - highlighted
                oss << colors::DIM << std::setw(3) << line_num << " |" << colors::RESET 
                    << " " << line_content << "\n";
                
                // Error pointer
                oss << colors::DIM << "   |" << colors::RESET << " ";
                for (int i = 0; i < token.column - 1; ++i) {
                    oss << " ";
                }
                oss << colors::BRIGHT_RED << "^~~" << colors::RESET << "\n";
            } else {
                // Context line - dimmed
                oss << colors::DIM << std::setw(3) << line_num << " |" << colors::RESET 
                    << " " << colors::DIM << line_content << colors::RESET << "\n";
            }
        }
    } else {
        // Fallback to single line
        std::string source_line = getSourceLine(token.line);
        if (!source_line.empty()) {
            oss << colors::DIM << "   |" << colors::RESET << "\n";
            oss << colors::DIM << std::setw(3) << token.line << " |" << colors::RESET 
                << " " << source_line << "\n";
            oss << colors::DIM << "   |" << colors::RESET << " ";
            for (int i = 0; i < token.column - 1; ++i) {
                oss << " ";
            }
            oss << colors::BRIGHT_RED << "^~~" << colors::RESET << "\n";
        } else {
            oss << colors::DIM << "   |" << colors::RESET << "\n";
            oss << colors::DIM << std::setw(3) << token.line << " |" << colors::RESET << "\n";
            oss << colors::DIM << "   |" << colors::RESET << " ";
            for (int i = 0; i < token.column - 1; ++i) {
                oss << " ";
            }
            oss << colors::BRIGHT_RED << "^" << colors::RESET << "\n";
        }
    }
    
    // Show hint if available
    if (hint) {
        oss << colors::DIM << "   |" << colors::RESET << "\n";
        oss << colors::DIM << "   = " << colors::RESET 
            << colors::BOLD << colors::BRIGHT_GREEN << "help: " << colors::RESET
            << colors::GREEN << hint << colors::RESET << "\n";
    }
    
    oss << colors::DIM << "   |" << colors::RESET << "\n";
    
    return std::runtime_error(oss.str());
}

StmtPtr Parser::switchStatement() {
    if (!match({TokenType::LEFT_PAREN})) throw error(peek(), "Expect '(' after 'switch'.");
    ExprPtr expression = this->expression();
    if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after switch expression.");
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' after switch expression.");
    
    std::vector<std::pair<ExprPtr, std::vector<StmtPtr>>> cases;
    std::vector<StmtPtr> defaultCase;
    bool hasDefault = false;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({TokenType::CASE})) {
            ExprPtr caseExpr = this->expression();
            if (!match({TokenType::COLON})) throw error(peek(), "Expect ':' after case expression.");
            
            std::vector<StmtPtr> caseStatements;
            while (!check(TokenType::CASE) && !check(TokenType::DEFAULT) && !check(TokenType::RIGHT_BRACE)) {
                caseStatements.push_back(statement());
            }
            cases.push_back({std::move(caseExpr), std::move(caseStatements)});
        } else if (match({TokenType::DEFAULT})) {
            if (hasDefault) throw error(peek(), "Multiple default cases in switch.");
            hasDefault = true;
            if (!match({TokenType::COLON})) throw error(peek(), "Expect ':' after 'default'.");
            
            while (!check(TokenType::RIGHT_BRACE)) {
                defaultCase.push_back(statement());
            }
        } else {
            throw error(peek(), "Expect 'case' or 'default' in switch statement.");
        }
    }
    
    if (!match({TokenType::RIGHT_BRACE})) throw error(peek(), "Expect '}' after switch statement.");
    
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Switch{std::move(expression), std::move(cases), std::move(defaultCase)};
    return stmt;
}

StmtPtr Parser::tryCatchStatement() {
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' after 'try'.");
    auto tryBlockStmt = block();
    std::vector<StmtPtr> tryBlock;
    if (auto blockNode = std::get_if<Stmt::Block>(&tryBlockStmt->node)) {
        tryBlock = std::move(blockNode->statements);
    }
    
    std::string catchVar;
    std::vector<StmtPtr> catchBlock;
    std::vector<StmtPtr> finallyBlock;
    
    if (match({TokenType::CATCH})) {
        if (!match({TokenType::LEFT_PAREN})) throw error(peek(), "Expect '(' after 'catch'.");
        if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect exception variable name.");
        catchVar = advance().lexeme;
        if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after catch variable.");
        if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' after catch.");
        auto catchStmt = block();
        if (auto blockNode = std::get_if<Stmt::Block>(&catchStmt->node)) {
            catchBlock = std::move(blockNode->statements);
        }
    }
    
    if (match({TokenType::FINALLY})) {
        if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' after 'finally'.");
        auto finallyStmt = block();
        if (auto blockNode = std::get_if<Stmt::Block>(&finallyStmt->node)) {
            finallyBlock = std::move(blockNode->statements);
        }
    }
    
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::TryCatch{std::move(tryBlock), catchVar, std::move(catchBlock), std::move(finallyBlock)};
    return stmt;
}

StmtPtr Parser::throwStatement() {
    ExprPtr expr = expression();
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after throw expression.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Throw{std::move(expr)};
    return stmt;
}

StmtPtr Parser::breakStatement() {
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after 'break'.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Break{};
    return stmt;
}

StmtPtr Parser::continueStatement() {
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after 'continue'.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Continue{};
    return stmt;
}

StmtPtr Parser::returnStatement() {
    ExprPtr value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after return value.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Return{std::move(value)};
    return stmt;
}

StmtPtr Parser::classDeclaration() {
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect class name.");
    Token name = advance();
    
    std::string superclass;
    if (match({TokenType::LESS})) {
        if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect superclass name.");
        superclass = advance().lexeme;
    }
    
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' before class body.");
    
    std::vector<StmtPtr> methods;
    std::vector<StmtPtr> fields;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({TokenType::FUNCTION}) || check(TokenType::IDENTIFIER)) {
            methods.push_back(functionDeclaration());
        } else if (match({TokenType::LET}) || match({TokenType::CONST})) {
            bool isConst = previous().type == TokenType::CONST;
            if (isConst) {
                fields.push_back(constDeclaration());
            } else {
                fields.push_back(varDeclaration());
            }
        } else {
            throw error(peek(), "Expect method or field declaration in class body.");
        }
    }
    
    if (!match({TokenType::RIGHT_BRACE})) throw error(peek(), "Expect '}' after class body.");
    
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Class{name.lexeme, superclass, std::move(methods), std::move(fields)};
    return stmt;
}

StmtPtr Parser::namespaceDeclaration() {
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect namespace name.");
    Token name = advance();
    
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' before namespace body.");
    
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    
    if (!match({TokenType::RIGHT_BRACE})) throw error(peek(), "Expect '}' after namespace body.");
    
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Namespace{name.lexeme, std::move(statements)};
    return stmt;
}

ExprPtr Parser::lambda() {
    if (!match({TokenType::LEFT_PAREN})) throw error(peek(), "Expect '(' after '->'.");
    
    std::vector<std::pair<std::string, std::string>> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect parameter name.");
            std::string paramName = advance().lexeme;
            std::string paramType;
            if (match({TokenType::COLON})) {
                if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect type name.");
                paramType = advance().lexeme;
            }
            parameters.push_back({paramName, paramType});
        } while (match({TokenType::COMMA}));
    }
    if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after parameters.");
    
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' before lambda body.");
    auto bodyStmt = block();
    
    std::vector<StmtPtr> body;
    if (auto blockNode = std::get_if<Stmt::Block>(&bodyStmt->node)) {
        body = std::move(blockNode->statements);
    }
    
    auto lambdaExpr = std::make_unique<Expr>();
    lambdaExpr->node = Expr::Lambda{std::move(parameters), std::move(body)};
    return lambdaExpr;
}

} // namespace kio
