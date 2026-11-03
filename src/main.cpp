#include <iostream>
#include <string>
#include <utility>

#include "novi/compiler.hpp"
#include "novi/lexer.hpp"
#include "novi/parser.hpp"
#include "novi/source.hpp"
#include "novi/vm.hpp"

namespace {

void print_diagnostics(const novi::Diagnostics& diagnostics, std::ostream& output) {
    for (const auto& diagnostic : diagnostics) {
        output << diagnostic.span.start.line << ':' << diagnostic.span.start.column << ": "
               << diagnostic.message << '\n';
    }
}

}  // namespace

int main() {
    novi::VM vm(std::cout);
    novi::Compiler compiler;
    std::string line;
    std::size_t line_number = 0;

    while (std::cout << "> " && std::getline(std::cin, line)) {
        ++line_number;
        if (line == "exit") break;
        if (line.empty()) continue;

        try {
            novi::SourceFile source("<repl>", line, line_number);
            auto lexed = novi::Lexer(source).scan();
            if (!lexed.diagnostics.empty()) {
                print_diagnostics(lexed.diagnostics, std::cerr);
                continue;
            }
            auto parsed = novi::Parser(source, std::move(lexed.tokens)).parse_line();
            if (!parsed.diagnostics.empty()) {
                print_diagnostics(parsed.diagnostics, std::cerr);
                continue;
            }
            auto module = compiler.compile(*parsed.program);
            if (!module.diagnostics.empty()) {
                print_diagnostics(module.diagnostics, std::cerr);
                continue;
            }
            const novi::Value result = vm.run(module);
            if (!std::holds_alternative<std::monostate>(result.data)) {
                std::cout << novi::value_to_string(result) << '\n';
            }
        } catch (const novi::RuntimeError& error) {
            std::cerr << error.span.start.line << ':' << error.span.start.column << ": "
                      << error.what() << '\n';
            for (const auto& frame : error.trace) {
                std::cerr << "  at " << frame.start.line << ':' << frame.start.column << '\n';
            }
        } catch (const std::exception& error) {
            std::cerr << "Novi error: " << error.what() << '\n';
        }
    }
    return 0;
}
