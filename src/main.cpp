#include <fstream>
#include <iostream>
#include <iterator>
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
        output << diagnostic.span.start.file << ':' << diagnostic.span.start.line << ':'
               << diagnostic.span.start.column << ": " << diagnostic.message << '\n';
    }
}

bool execute_source(const novi::SourceFile& source, novi::VM& vm, novi::Compiler& compiler,
                    bool repl, bool& had_error) {
    auto lexed = novi::Lexer(source).scan();
    if (!lexed.diagnostics.empty()) {
        print_diagnostics(lexed.diagnostics, std::cerr);
        had_error = true;
        return false;
    }
    auto parser = novi::Parser(source, std::move(lexed.tokens));
    auto parsed = repl ? parser.parse_line() : parser.parse_program();
    if (!parsed.diagnostics.empty()) {
        print_diagnostics(parsed.diagnostics, std::cerr);
        had_error = true;
        return false;
    }
    auto module = compiler.compile(*parsed.program);
    if (!module.diagnostics.empty()) {
        print_diagnostics(module.diagnostics, std::cerr);
        had_error = true;
        return false;
    }
    try {
        const novi::Value result = vm.run(module);
        if (repl && !std::holds_alternative<std::monostate>(result.data))
            std::cout << novi::value_to_string(result) << '\n';
    } catch (const novi::RuntimeError& error) {
        std::cerr << error.span.start.file << ':' << error.span.start.line << ':'
                  << error.span.start.column << ": " << error.what() << '\n';
        for (const auto& frame : error.trace)
            std::cerr << "  at " << frame.start.file << ':' << frame.start.line << ':'
                      << frame.start.column << '\n';
        had_error = true;
        return false;
    } catch (const std::exception& error) {
        std::cerr << "Novi error: " << error.what() << '\n';
        had_error = true;
        return false;
    }
    return true;
}

int run_repl() {
    novi::VM vm(std::cout);
    novi::Compiler compiler;
    std::string line;
    std::size_t line_number = 0;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        ++line_number;
        if (line == "exit") break;
        if (line.empty()) continue;
        bool had_error = false;
        execute_source(novi::SourceFile("<repl>", line, line_number), vm, compiler, true, had_error);
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 1) return run_repl();
    if (argc != 2) {
        std::cerr << "Usage: novi [source.nvi]\n";
        return 2;
    }

    const std::string path = argv[1];
    if (path.size() < 4 || path.substr(path.size() - 4) != ".nvi") {
        std::cerr << "Novi source files must use the .nvi extension: " << path << '\n';
        return 2;
    }
    std::ifstream input(path);
    if (!input) {
        std::cerr << "Could not open Novi source file: " << path << '\n';
        return 2;
    }
    const std::string text{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    novi::VM vm(std::cout);
    novi::Compiler compiler;
    bool had_error = false;
    execute_source(novi::SourceFile(path, text), vm, compiler, false, had_error);
    return had_error ? 1 : 0;
}
