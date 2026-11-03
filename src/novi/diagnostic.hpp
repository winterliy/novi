#pragma once

#include <string>
#include <vector>

#include "source.hpp"

namespace novi {

enum class DiagnosticCategory { Lexical, Syntax, Semantic, Compile, Runtime };

struct Diagnostic {
    DiagnosticCategory category = DiagnosticCategory::Syntax;
    std::string message;
    SourceSpan span;
};

using Diagnostics = std::vector<Diagnostic>;

inline const char* category_name(DiagnosticCategory category) {
    switch (category) {
        case DiagnosticCategory::Lexical: return "lexical";
        case DiagnosticCategory::Syntax: return "syntax";
        case DiagnosticCategory::Semantic: return "semantic";
        case DiagnosticCategory::Compile: return "compile";
        case DiagnosticCategory::Runtime: return "runtime";
    }
    return "unknown";
}

}  // namespace novi
