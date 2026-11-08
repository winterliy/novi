#include "native/ascii.h"
#include <chrono>
#include <iostream>
#include <string>

int main() {
    const std::string text(4096, 'a');
    std::size_t result = 0;
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < 20000; ++i)
        result += novi_ascii_prefix(reinterpret_cast<const unsigned char*>(text.data()), text.size());
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (result != 4096u * 20000u) return 1;
    std::cout << "ascii_seconds=" << elapsed << '\n';
}
