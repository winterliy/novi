#pragma once

#include <string>
#include <vector>

#include "diagnostic.hpp"

namespace novi {

enum class TokenKind { Number, Plus, Minus, Star, Slash, LeftParen, RightParen, End };

struct Token {
    TokenKind kind;
    std::string lexeme;
    double number = 0.0;
    SourceSpan span;
};

struct LexResult {
    std::vector<Token> tokens;
    Diagnostics diagnostics;
};

class Lexer {
public:
    explicit Lexer(const SourceFile& source) : source_(source) {}
    LexResult scan() const;

private:
    const SourceFile& source_;
};

}  // namespace novi
