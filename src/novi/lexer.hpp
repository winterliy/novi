#pragma once

#include <string>
#include <vector>

#include "diagnostic.hpp"

namespace novi {

enum class TokenKind {
    Identifier, Number, String,
    Plus, Minus, Star, Slash, FloorSlash, Percent,
    Ampersand, Pipe, LogicalAnd, LogicalOr, Caret, Tilde, ShiftLeft, ShiftRight,
    Bang, Equal, EqualEqual, BangEqual, Less, LessEqual, Greater, GreaterEqual,
    Question, Colon, Dot, Comma, Semicolon,
    LeftParen, RightParen, LeftBrace, RightBrace, LeftBracket, RightBracket,
    At, Hash, End
};

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
