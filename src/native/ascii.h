#ifndef NOVI_NATIVE_ASCII_H
#define NOVI_NATIVE_ASCII_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Returns the length of the leading ASCII byte sequence. data may be null only when length is zero. */
size_t novi_ascii_prefix(const unsigned char *data, size_t length);
#ifdef __cplusplus
}
#endif
#endif
