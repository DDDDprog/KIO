#include "kio/parser.hpp"
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
    return statement();
}

StmtPtr Parser::varDeclaration() {
    if (!check(TokenType::IDENTIFIER)) throw error(peek(), "Expect variable name after 'let'.");
    std::string name = advance().lexeme;
    if (!match({TokenType::EQUAL})) throw error(peek(), "Expect '=' after variable name.");
    ExprPtr init = expression();
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after variable declaration.");
    auto var = std::make_unique<Stmt>();
    var->node = Stmt::Var{std::move(name), std::move(init)};
    return var;
}

StmtPtr Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::SAVE})) return saveStatement();
    if (match({TokenType::LOAD})) return loadStatement();
    if (match({TokenType::IMPORT})) return importStatement();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr value = expression();
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after value.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Print{std::move(value)};
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
    if (!match({TokenType::STRING})) throw error(peek(), "Expect path string after import.");
    std::string path = previous().lexeme;
    if (!match({TokenType::SEMICOLON})) throw error(peek(), "Expect ';' after import path.");
    auto stmt = std::make_unique<Stmt>();
    stmt->node = Stmt::Import{std::move(path)};
    return stmt;
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
    ExprPtr expr = equality();
    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        ExprPtr value = assignment();
        if (auto var = std::get_if<Expr::Variable>(&expr->node)) {
            auto node = std::make_unique<Expr>();
            node->node = Expr::Assign{var->name, std::move(value)};
            return node;
        }
        throw error(equals, "Invalid assignment target.");
    }
    return expr;
}

ExprPtr Parser::equality() { return comparison(); }

ExprPtr Parser::comparison() { return term(); }

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
    while (match({TokenType::STAR, TokenType::SLASH})) {
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
    return primary();
}

ExprPtr Parser::primary() {
    if (match({TokenType::NUMBER})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{std::stod(previous().lexeme)};
        return node;
    }
    if (match({TokenType::STRING})) {
        auto node = std::make_unique<Expr>();
        node->node = Expr::Literal{previous().lexeme};
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
    throw error(peek(), "Expect expression.");
}

ExprPtr Parser::sysQuery() {
    if (!match({TokenType::STRING})) throw error(peek(), "Expect string key after sys.");
    auto node = std::make_unique<Expr>();
    node->node = Expr::SysQuery{previous().lexeme};
    return node;
}

} // namespace kio
