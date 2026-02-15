/*
Copyright (c) 2025 Dipanjan Dhar
SPDX-License-Identifier: GPL-3.0-only
*/

#include "axeon/parser.hpp"
#include <stdexcept>

namespace kio {

static std::runtime_error error(const Token &token, const char *message) {
    return std::runtime_error(std::string("[line ") + std::to_string(token.line) + ":" + std::to_string(token.column) + "] " + message);
}

Parser::Parser(const std::vector<Token> &tokens) : tokens_(tokens) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

bool Parser::isAtEnd() const { return peek().type == TokenType::END_OF_FILE; }
const Token &Parser::peek() const { return tokens_[current_]; }
const Token &Parser::previous() const { return tokens_[current_ - 1]; }
const Token &Parser::advance() { if (!isAtEnd()) current_++; return previous(); }
bool Parser::check(TokenType type) const { return !isAtEnd() && peek().type == type; }

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType t : types) {
        if (check(t)) { advance(); return true; }
    }
    return false;
}

StmtPtr Parser::declaration() {
    if (match({TokenType::LET})) return varDeclaration();
    if (match({TokenType::CONST})) return constDeclaration();
    if (match({TokenType::FUNCTION})) return functionDeclaration();
    if (match({TokenType::CLASS})) return classDeclaration();
    if (match({TokenType::NAMESPACE})) return namespaceDeclaration();
    if (match({TokenType::MODULE})) return moduleDeclaration();
    if (match({TokenType::EXPORT})) return exportStatement();
    return statement();
}

// ... varDeclaration / constDeclaration ...

StmtPtr Parser::functionDeclaration() {
    Token name = peek();
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect function name.");
    advance();

    if (!match({TokenType::LEFT_PAREN})) throw error(peek(), "Expect '(' after function name.");
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

    std::string returnType;
    if (match({TokenType::COLON})) {
        if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect return type.");
        returnType = advance().lexeme;
    }

    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' before function body.");
    auto bodyStmt = block();
    
    // Convert StmtPtr (block) to Block struct
    if (auto blockNode = std::get_if<Stmt::Block>(&bodyStmt->node)) {
        auto func = std::make_unique<Stmt>();
        func->node = Stmt::Function{name.lexeme, std::move(parameters), returnType, std::move(blockNode->statements)};
        return func;
    }
    return nullptr;
}

StmtPtr Parser::varDeclaration() {
    Token name = peek();
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect variable name after 'let'.");
    advance();
    
    std::string typeAnnotation;
    if (match({TokenType::COLON})) {
        if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect type name after ':'.");
        typeAnnotation = advance().lexeme;
    }

    ExprPtr init = nullptr;
    if (match({TokenType::EQUAL})) {
        init = expression();
    } else {
        // Init is optional for let, defaults to nil (not implemented yet) or error
        // For now require init
        throw error(peek(), "Expect '=' after variable declaration.");
    }
    
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after variable declaration.");
    auto var = std::make_unique<Stmt>();
    var->node = Stmt::Var{name.lexeme, std::move(init), typeAnnotation, false};
    return var;
}

StmtPtr Parser::constDeclaration() {
    Token name = peek();
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect variable name after 'const'.");
    advance();

    std::string typeAnnotation;
    if (match({TokenType::COLON})) {
        if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect type name after ':'.");
        typeAnnotation = advance().lexeme;
    }

    if (!match({TokenType::EQUAL})) throw error(peek(), "Expect '=' after const declaration.");
    ExprPtr init = expression();
    
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after const declaration.");
    auto var = std::make_unique<Stmt>();
    var->node = Stmt::Var{name.lexeme, std::move(init), typeAnnotation, true};
    return var;
}

StmtPtr Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement();
    if (match({TokenType::SWITCH})) return switchStatement();
    if (match({TokenType::TRY})) return tryCatchStatement();
    if (match({TokenType::THROW})) return throwStatement();
    if (match({TokenType::BREAK})) return breakStatement();
    if (match({TokenType::CONTINUE})) return continueStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::LEFT_BRACE})) return block();
    if (match({TokenType::SAVE})) return saveStatement();
    if (match({TokenType::LOAD})) return loadStatement();
    if (match({TokenType::IMPORT})) return importStatement();
    if (match({TokenType::PARALLEL})) return parallelStatement();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr value = expression();
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after value.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Print{std::move(value)};
    return stmt;
}

StmtPtr Parser::ifStatement() {
    if (!match({TokenType::LEFT_PAREN})) throw error(peek(), "Expect '(' after 'if'.");
    ExprPtr condition = expression();
    if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after if condition.");

    StmtPtr thenBranch = statement();
    StmtPtr elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }

    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::If{std::move(condition), std::move(thenBranch), std::move(elseBranch)};
    return stmt;
}

StmtPtr Parser::whileStatement() {
    if (!match({TokenType::LEFT_PAREN})) throw error(peek(), "Expect '(' after 'while'.");
    ExprPtr condition = expression();
    if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after while condition.");
    StmtPtr body = statement();

    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::While{std::move(condition), std::move(body)};
    return stmt;
}

StmtPtr Parser::parallelStatement() {
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' after 'parallel'.");
    auto body = block();
    auto stmt = std::make_unique<Stmt>();
    if (auto blockNode = std::get_if<Stmt::Block>(&body->node)) {
        stmt->node = Stmt::Parallel{std::move(blockNode->statements)};
    }
    return stmt;
}

StmtPtr Parser::block() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    if (!match({TokenType::RIGHT_BRACE})) throw error(peek(), "Expect '}' after block.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Block{std::move(statements)};
    return stmt;
}

StmtPtr Parser::forStatement() {
    // Check if it's a for-in loop: for i in range(n)
    bool hasParen = match({TokenType::LEFT_PAREN});
    
    if (!hasParen && check(TokenType::IDENTIFIER)) {
        Token nameToken = advance();
        if (match({TokenType::IN})) {
            ExprPtr iterable = expression(); // Usually range(n)
            StmtPtr body = statement();
            
            auto stmt = std::make_unique<Stmt>();
            stmt->node = Stmt::ForIn{nameToken.lexeme, std::move(iterable), std::move(body)};
            return stmt;
        }
        // If it was an identifier but no 'in', we might need to backtrack or it's an error.
        // Since we already advanced nameToken, let's just assume it's a for-in or error.
        throw error(peek(), "Expect 'in' after variable name in for loop.");
    }

    if (!hasParen) {
        throw error(peek(), "Expect '(' after 'for'.");
    }
    
    StmtPtr initializer;
    if (match({TokenType::SEMICOLON})) {
        initializer = nullptr;
    } else if (match({TokenType::LET})) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }

    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after loop condition.");

    ExprPtr increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after for clauses.");

    StmtPtr body = statement();

    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::For{std::move(initializer), std::move(condition), std::move(increment), std::move(body)};
    return stmt;
}

StmtPtr Parser::saveStatement() {
    if (!match({TokenType::STRING})) throw error(peek(), "Expect path string after save.");
    std::string path = previous().lexeme;
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after save path.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Save{std::move(path)};
    return stmt;
}

StmtPtr Parser::loadStatement() {
    if (!match({TokenType::STRING})) throw error(peek(), "Expect path string after load.");
    std::string path = previous().lexeme;
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after load path.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Load{std::move(path)};
    return stmt;
}

StmtPtr Parser::importStatement() {
    if (match({TokenType::IDENTIFIER, TokenType::ASYNC})) {
        std::string name = previous().lexeme;
        if (match({TokenType::FROM})) {
            if (!match({TokenType::STRING})) throw error(peek(), "Expect path string after 'from'.");
            std::string path = previous().lexeme;
            if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after import.");
            auto stmt = std::make_unique<Stmt>();
            stmt->node = Stmt::ImportFrom{name, path};
            return stmt;
        }
        throw error(peek(), "Expect 'from' after import name.");
    }

    if (!match({TokenType::STRING})) throw error(peek(), "Expect path string after import.");
    std::string path = previous().lexeme;
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after import path.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Import{std::move(path)};
    return stmt;
}

StmtPtr Parser::moduleDeclaration() {
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect module name.");
    std::string name = advance().lexeme;
    if (!match({TokenType::LEFT_BRACE})) throw error(peek(), "Expect '{' after module name.");
    
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    if (!match({TokenType::RIGHT_BRACE})) throw error(peek(), "Expect '}' after module body.");
    
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Module{name, std::move(statements)};
    return stmt;
}

StmtPtr Parser::exportStatement() {
    StmtPtr stmt = declaration();
    auto exportStmt = std::make_unique<Stmt>();
    exportStmt->node = Stmt::Export{std::move(stmt)};
    return exportStmt;
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after expression.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Expression{std::move(expr)};
    return stmt;
}

ExprPtr Parser::expression() { return assignment(); }

ExprPtr Parser::assignment() {
    ExprPtr expr = or_expr();
    if (match({TokenType::EQUAL, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL,
               TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL})) {
        Token op = previous();
        ExprPtr value = assignment();
        if (auto var = std::get_if<Expr::Variable>(&expr->node)) {
            auto node = std::make_unique<Expr>();
            if (op.type == TokenType::EQUAL) {
                node->node = Expr::Assign{var->name, std::move(value)};
            } else {
                 // Compound assignment handled...
            }
            return node;
        } else if (auto get = std::get_if<Expr::Get>(&expr->node)) {
            auto node = std::make_unique<Expr>();
            if (op.type == TokenType::EQUAL) {
                node->node = Expr::Set{std::move(get->object), get->name, std::move(value)};
            } else {
                 // Compound assignment for Set
            }
            return node;
        } else if (auto idx = std::get_if<Expr::Index>(&expr->node)) {
            auto node = std::make_unique<Expr>();
            if (op.type == TokenType::EQUAL) {
                node->node = Expr::IndexSet{std::move(idx->object), std::move(idx->index), std::move(value)};
            } else {
                 // Compound assignment for Index
            }
            return node;
        }
        throw error(op, "Invalid assignment target.");
    }
    return expr;
}

ExprPtr Parser::or_expr() {
    ExprPtr expr = and_expr();
    while (match({TokenType::OR})) {
        Token op = previous();
        ExprPtr right = and_expr();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Logical{std::move(expr), op, std::move(right)};
        expr = std::move(node);
    }
    return expr;
}

ExprPtr Parser::and_expr() {
    ExprPtr expr = equality();
    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = equality();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Logical{std::move(expr), op, std::move(right)};
        expr = std::move(node);
    }
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    while (match({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Binary{std::move(expr), op, std::move(right)};
        expr = std::move(node);
    }
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                  TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = term();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Binary{std::move(expr), op, std::move(right)};
        expr = std::move(node);
    }
    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = factor();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Binary{std::move(expr), op, std::move(right)};
        expr = std::move(node);
    }
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        ExprPtr right = unary();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Binary{std::move(expr), op, std::move(right)};
        expr = std::move(node);
    }
    return expr;
}

ExprPtr Parser::unary() {
    // sys "key" as an expression
    if (match({TokenType::SYS})) {
        return sysQuery();
    }
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = unary();
        auto node = std::make_unique<Expr>();
        node->node = Expr::Unary{op, std::move(right)};
        return node;
    }
    ExprPtr expr = call();
    if (match({TokenType::PLUS_PLUS, TokenType::MINUS_MINUS})) {
        Token op = previous();
        if (auto var = std::get_if<Expr::Variable>(&expr->node)) {
            auto node = std::make_unique<Expr>();
            node->node = Expr::PostOp{var->name, op};
            return node;
        }
        throw error(op, "Invalid increment/decrement target.");
    }
    return expr;
}

ExprPtr Parser::primary() {
    if (match({TokenType::NUMBER})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{std::stod(previous().lexeme)};
        return node;
    }
    if (match({TokenType::STRING}) || match({TokenType::RAW_STRING})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{previous().lexeme};
        return node;
    }
    if (match({TokenType::CHAR})) {
        auto node = std::make_unique<Expr>();
        std::string charVal = previous().lexeme;
        node->node = Expr::Literal{charVal};
        return node;
    }
    if (match({TokenType::TRUE})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{std::string("true")};
        return node;
    }
    if (match({TokenType::FALSE})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{std::string("false")};
        return node;
    }
    if (match({TokenType::NIL})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{std::string("")};
        return node;
    }
    if (match({TokenType::THIS})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::This{previous().lexeme};
        return node;
    }
    if (match({TokenType::IDENTIFIER})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Variable{previous().lexeme};
        return node;
    }
    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after expression.");
        auto node = std::make_unique<Expr>();
        node->node = Expr::Grouping{std::move(expr)};
        return node;
    }
    if (match({TokenType::LEFT_BRACKET})) {
        return array();
    }
    throw error(peek(), "Expect expression.");
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();
    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            expr = finishCall(std::move(expr));
        } else if (match({TokenType::DOT})) {
            if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect property name after '.'.");
            Token name = advance();
            auto get = std::make_unique<Expr>();
            get->node = Expr::Get{std::move(expr), name.lexeme};
            expr = std::move(get);
        } else if (match({TokenType::LEFT_BRACKET})) {
            ExprPtr index = expression();
            if (!match({TokenType::RIGHT_BRACKET})) throw error(peek(), "Expect ']' after index.");
            auto idx = std::make_unique<Expr>();
            idx->node = Expr::Index{std::move(expr), std::move(index)};
            expr = std::move(idx);
        } else {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    std::vector<ExprPtr> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                throw error(peek(), "Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after arguments.");
    
    auto callExpr = std::make_unique<Expr>();
    callExpr->node = Expr::Call{std::move(callee), std::move(arguments)};
    return callExpr;
}

ExprPtr Parser::array() {
    std::vector<ExprPtr> elements;
    if (!check(TokenType::RIGHT_BRACKET)) {
        do {
            elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    if (!match({TokenType::RIGHT_BRACKET})) throw error(peek(), "Expect ']' after array elements.");
    
    auto arr = std::make_unique<Expr>();
    arr->node = Expr::Array{std::move(elements)};
    return arr;
}

ExprPtr Parser::ternary() {
    ExprPtr expr = or_expr();
    if (match({TokenType::QUESTION})) {
        ExprPtr thenExpr = expression();
        if (!match({TokenType::COLON})) throw error(peek(), "Expect ':' after ternary condition.");
        ExprPtr elseExpr = ternary();
        auto tern = std::make_unique<Expr>();
        tern->node = Expr::Ternary{std::move(expr), std::move(thenExpr), std::move(elseExpr)};
        return tern;
    }
    return expr;
}

ExprPtr Parser::sysQuery() {
    bool hasParen = match({TokenType::LEFT_PAREN});
    if (!match({TokenType::STRING})) throw error(peek(), "Expect string key after sys.");
    std::string key = previous().lexeme;
    if (hasParen) {
        if (!match({TokenType::RIGHT_PAREN})) throw error(peek(), "Expect ')' after sys query.");
    }
    auto node = std::make_unique<Expr>();
    node->node = Expr::SysQuery{key};
    return node;
}

} // namespace kio
