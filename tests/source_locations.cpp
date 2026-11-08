#include "novi/source.hpp"

int main() {
    const novi::SourceFile source("example.nvi", "ab\nc\n", 10);
    const auto start = source.location(0);
    const auto newline = source.location(2);
    const auto second = source.location(3);
    const auto end = source.location(500);
    return !(start.line == 10 && start.column == 1 && newline.line == 10 && newline.column == 3 &&
             second.line == 11 && second.column == 1 && end.line == 12 && end.column == 1 &&
             end.offset == 5 && end.file == "example.nvi");
}
