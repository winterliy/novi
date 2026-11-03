#pragma once

#include <memory>
#include <vector>

#include "ast.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"

namespace novi {

struct ParseResult {
    std::unique_ptr<Program> program;
    Diagnostics diagnostics;
};

class Parser {
public:
    Parser(const SourceFile& source, std::vector<Token> tokens)
        : source_(source), tokens_(std::move(tokens)) {}
    ParseResult parse_line();

private:
    struct ParseFailure {};
    const Token& peek() const;
    const Token& previous() const;
    bool match(TokenKind kind);
    const Token& consume(TokenKind kind, const char* message);
    bool at_end() const;
    ExprPtr expression();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();
    void report(const Token& token, const std::string& message);

    const SourceFile& source_;
    std::vector<Token> tokens_;
    std::size_t current_ = 0;
    Diagnostics diagnostics_;
};

}  // namespace novi
