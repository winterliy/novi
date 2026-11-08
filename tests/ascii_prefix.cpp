#include "native/ascii.h"
#include <cstddef>

int main() {
    if (novi_ascii_prefix(nullptr, 0) != 0) return 1;
    unsigned char data[256];
    for (std::size_t i = 0; i < 256; ++i) data[i] = 'a';
    for (std::size_t i = 0; i < 256; ++i) {
        data[i] = 0x80;
        if (novi_ascii_prefix(data, sizeof(data)) != i) return 1;
        data[i] = 'a';
    }
    for (unsigned value = 0; value < 256; ++value) {
        data[0] = static_cast<unsigned char>(value);
        if (novi_ascii_prefix(data, 1) != (value < 128 ? 1u : 0u)) return 1;
    }
    return 0;
}
