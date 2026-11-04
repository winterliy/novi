#include "lexer.hpp"

#include <cctype>
#include <cstdlib>

namespace novi {
namespace {

bool identifier_start(unsigned char c) { return std::isalpha(c) || c == '_'; }
bool identifier_part(unsigned char c) { return std::isalnum(c) || c == '_'; }

}  // namespace

LexResult Lexer::scan() const {
    LexResult result;
    const std::string& text = source_.text();
    std::size_t position = 0;
    auto emit = [&](TokenKind kind, std::size_t begin, std::size_t end, double number = 0.0) {
        result.tokens.push_back({kind, text.substr(begin, end - begin), number, source_.span(begin, end)});
    };

    while (position < text.size()) {
        const unsigned char c = static_cast<unsigned char>(text[position]);
        if (std::isspace(c)) {
            ++position;
            continue;
        }

        const std::size_t begin = position;
        if (identifier_start(c)) {
            ++position;
            while (position < text.size() && identifier_part(static_cast<unsigned char>(text[position]))) ++position;
            emit(TokenKind::Identifier, begin, position);
            continue;
        }

        if (std::isdigit(c) || (text[position] == '.' && position + 1 < text.size() &&
                               std::isdigit(static_cast<unsigned char>(text[position + 1])))) {
            char* end = nullptr;
            const double value = std::strtod(text.c_str() + position, &end);
            if (end == text.c_str() + position) {
                result.diagnostics.push_back({DiagnosticCategory::Lexical, "Invalid number", source_.span(begin, begin + 1)});
                ++position;
                continue;
            }
            position = static_cast<std::size_t>(end - text.c_str());
            emit(TokenKind::Number, begin, position, value);
            continue;
        }

        if (text[position] == '\'' || text[position] == '"') {
            const char quote = text[position++];
            std::string decoded;
            bool closed = false;
            while (position < text.size()) {
                const char current = text[position++];
                if (current == quote) {
                    closed = true;
                    break;
                }
                if (current == '\\' && position < text.size()) {
                    const char escaped = text[position++];
                    switch (escaped) {
                        case 'n': decoded.push_back('\n'); break;
                        case 'r': decoded.push_back('\r'); break;
                        case 't': decoded.push_back('\t'); break;
                        case '\\': decoded.push_back('\\'); break;
                        case '\'': decoded.push_back('\''); break;
                        case '"': decoded.push_back('"'); break;
                        default:
                            result.diagnostics.push_back({DiagnosticCategory::Lexical,
                                std::string("Unknown string escape \\") + escaped, source_.span(position - 2, position)});
                            decoded.push_back(escaped);
                            break;
                    }
                } else {
                    decoded.push_back(current);
                }
            }
            if (!closed) {
                result.diagnostics.push_back({DiagnosticCategory::Lexical, "Unterminated string", source_.span(begin, position)});
                continue;
            }
            result.tokens.push_back({TokenKind::String, std::move(decoded), 0.0, source_.span(begin, position)});
            continue;
        }

        ++position;
        auto take = [&](char expected) {
            if (position < text.size() && text[position] == expected) {
                ++position;
                return true;
            }
            return false;
        };
        switch (text[begin]) {
            case '+': emit(TokenKind::Plus, begin, position); break;
            case '-': emit(TokenKind::Minus, begin, position); break;
            case '*': emit(TokenKind::Star, begin, position); break;
            case '/': emit(take('/') ? TokenKind::FloorSlash : TokenKind::Slash, begin, position); break;
            case '%': emit(TokenKind::Percent, begin, position); break;
            case '&': emit(take('&') ? TokenKind::LogicalAnd : TokenKind::Ampersand, begin, position); break;
            case '|': emit(take('|') ? TokenKind::LogicalOr : TokenKind::Pipe, begin, position); break;
            case '^': emit(TokenKind::Caret, begin, position); break;
            case '~': emit(TokenKind::Tilde, begin, position); break;
            case '!': emit(take('=') ? TokenKind::BangEqual : TokenKind::Bang, begin, position); break;
            case '=': emit(take('=') ? TokenKind::EqualEqual : TokenKind::Equal, begin, position); break;
            case '<':
                if (take('=')) emit(TokenKind::LessEqual, begin, position);
                else if (take('<')) emit(TokenKind::ShiftLeft, begin, position);
                else emit(TokenKind::Less, begin, position);
                break;
            case '>':
                if (take('=')) emit(TokenKind::GreaterEqual, begin, position);
                else if (take('>')) emit(TokenKind::ShiftRight, begin, position);
                else emit(TokenKind::Greater, begin, position);
                break;
            case '?': emit(TokenKind::Question, begin, position); break;
            case ':': emit(TokenKind::Colon, begin, position); break;
            case '.': emit(TokenKind::Dot, begin, position); break;
            case ',': emit(TokenKind::Comma, begin, position); break;
            case ';': emit(TokenKind::Semicolon, begin, position); break;
            case '(': emit(TokenKind::LeftParen, begin, position); break;
            case ')': emit(TokenKind::RightParen, begin, position); break;
            case '{': emit(TokenKind::LeftBrace, begin, position); break;
            case '}': emit(TokenKind::RightBrace, begin, position); break;
            case '@': emit(TokenKind::At, begin, position); break;
            case '#': emit(TokenKind::Hash, begin, position); break;
            default:
                result.diagnostics.push_back({DiagnosticCategory::Lexical,
                    std::string("Unexpected character '") + text[begin] + "'", source_.span(begin, position)});
                break;
        }
    }

    const SourceSpan end_span = source_.span(text.size(), text.size());
    result.tokens.push_back({TokenKind::End, "", 0.0, end_span});
    return result;
}

}  // namespace novi
