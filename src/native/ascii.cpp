#include "ascii.h"

extern "C" size_t novi_ascii_prefix(const unsigned char* data, size_t length) {
    size_t i = 0;
    while (i < length && data[i] < 0x80) ++i;
    return i;
}
