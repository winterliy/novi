#include "parser.hpp"

namespace novi {

const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }

bool Parser::match(TokenKind kind) {
    if (peek().kind != kind) return false;
    ++current_;
    return true;
}

const Token& Parser::consume(TokenKind kind, const char* message) {
    if (peek().kind == kind) return tokens_[current_++];
    report(peek(), message);
    throw ParseFailure{};
}

bool Parser::at_end() const { return peek().kind == TokenKind::End; }

void Parser::report(const Token& token, const std::string& message) {
    diagnostics_.push_back({DiagnosticCategory::Syntax, message, token.span});
}

ExprPtr Parser::expression() { return term(); }

ExprPtr Parser::term() {
    ExprPtr result = factor();
    while (match(TokenKind::Plus) || match(TokenKind::Minus)) {
        const Token operation = previous();
        ExprPtr right = factor();
        const auto kind = operation.kind == TokenKind::Plus ? BinaryOperator::Add : BinaryOperator::Subtract;
        SourceSpan span{result->span.start, right->span.end};
        result = std::make_unique<Expr>(std::move(span), BinaryExpr{kind, std::move(result), std::move(right)});
    }
    return result;
}

ExprPtr Parser::factor() {
    ExprPtr result = unary();
    while (match(TokenKind::Star) || match(TokenKind::Slash)) {
        const Token operation = previous();
        ExprPtr right = unary();
        const auto kind = operation.kind == TokenKind::Star ? BinaryOperator::Multiply : BinaryOperator::Divide;
        SourceSpan span{result->span.start, right->span.end};
        result = std::make_unique<Expr>(std::move(span), BinaryExpr{kind, std::move(result), std::move(right)});
    }
    return result;
}

ExprPtr Parser::unary() {
    if (match(TokenKind::Minus)) {
        const Token operation = previous();
        ExprPtr operand = unary();
        SourceSpan span{operation.span.start, operand->span.end};
        return std::make_unique<Expr>(std::move(span), UnaryExpr{UnaryOperator::Negate, std::move(operand)});
    }
    return primary();
}

ExprPtr Parser::primary() {
    if (match(TokenKind::Number)) {
        const Token token = previous();
        return std::make_unique<Expr>(token.span, LiteralExpr{token.number});
    }
    if (match(TokenKind::LeftParen)) {
        const Token opening = previous();
        ExprPtr expression_value = expression();
        const Token& closing = consume(TokenKind::RightParen, "Expected ')' after expression");
        expression_value->span = {opening.span.start, closing.span.end};
        return expression_value;
    }
    report(peek(), "Expected a number or parenthesized expression");
    throw ParseFailure{};
}

ParseResult Parser::parse_line() {
    ParseResult result;
    try {
        auto program = std::make_unique<Program>();
        program->echo_last_expression = true;
        ExprPtr expression_value = expression();
        const SourceSpan span = expression_value->span;
        program->statements.push_back(std::make_unique<Stmt>(span, ExpressionStmt{std::move(expression_value)}));
        if (!at_end()) consume(TokenKind::End, "Unexpected input after expression");
        result.program = std::move(program);
    } catch (const ParseFailure&) {
        // A line is independent in the REPL, so discard its incomplete parse.
    }
    result.diagnostics = std::move(diagnostics_);
    return result;
}

}  // namespace novi
