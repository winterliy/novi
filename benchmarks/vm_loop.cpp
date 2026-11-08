#include <chrono>
#include <iostream>
#include <sstream>
#include <utility>

#include "novi/compiler.hpp"
#include "novi/lexer.hpp"
#include "novi/parser.hpp"
#include "novi/source.hpp"
#include "novi/vm.hpp"

int main() {
    const novi::SourceFile source("<benchmark>",
        "vari total : integer = 0;\n"
        "foreach (i in range(0, 1, 100000)) { total = total + i; }\n"
        "console.print(total:string);\n");
    auto lexed = novi::Lexer(source).scan();
    if (!lexed.diagnostics.empty()) return 1;
    auto parsed = novi::Parser(source, std::move(lexed.tokens)).parse_program();
    if (!parsed.diagnostics.empty()) return 1;
    auto module = novi::Compiler().compile(*parsed.program);
    if (!module.diagnostics.empty()) return 1;
    std::ostringstream output;
    novi::VM vm(output);
    const auto start = std::chrono::steady_clock::now();
    vm.run(module);
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (output.str() != "4999950000\n") return 1;
    std::cout << "vm_seconds=" << elapsed << '\n';
}
