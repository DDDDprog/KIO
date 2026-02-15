/*
Copyright (c) 2026 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/parser.hpp"
#include <stdexcept>

namespace kio {

static std::runtime_error error(const Token &token, const char *message) {
    return std::runtime_error(std::string("[line ") + std::to_string(token.line) + ":" + std::to_string(token.column) + "] " + message);
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
