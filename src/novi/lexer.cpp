#include "lexer.hpp"

#include <cctype>
#include <cstdlib>

namespace novi {

LexResult Lexer::scan() const {
    LexResult result;
    const std::string& text = source_.text();
    std::size_t position = 0;
    while (position < text.size()) {
        const unsigned char character = static_cast<unsigned char>(text[position]);
        if (std::isspace(character)) {
            ++position;
            continue;
        }
        const std::size_t begin = position;
        TokenKind kind;
        switch (text[position]) {
            case '+': kind = TokenKind::Plus; ++position; break;
            case '-': kind = TokenKind::Minus; ++position; break;
            case '*': kind = TokenKind::Star; ++position; break;
            case '/': kind = TokenKind::Slash; ++position; break;
            case '(': kind = TokenKind::LeftParen; ++position; break;
            case ')': kind = TokenKind::RightParen; ++position; break;
            default:
                if (!std::isdigit(character) && text[position] != '.') {
                    result.diagnostics.push_back({DiagnosticCategory::Lexical,
                        std::string("Unexpected character '") + text[position] + "'", source_.span(position, position + 1)});
                    ++position;
                    continue;
                }
                char* end = nullptr;
                std::strtod(text.c_str() + position, &end);
                if (end == text.c_str() + position) {
                    result.diagnostics.push_back({DiagnosticCategory::Lexical, "Invalid number", source_.span(position, position + 1)});
                    ++position;
                    continue;
                }
                position = static_cast<std::size_t>(end - text.c_str());
                result.tokens.push_back({TokenKind::Number, text.substr(begin, position - begin),
                                         std::strtod(text.c_str() + begin, nullptr), source_.span(begin, position)});
                continue;
        }
        result.tokens.push_back({kind, text.substr(begin, position - begin), 0.0, source_.span(begin, position)});
    }
    const SourceSpan end_span = source_.span(text.size(), text.size());
    result.tokens.push_back({TokenKind::End, "", 0.0, end_span});
    return result;
}

}  // namespace novi
