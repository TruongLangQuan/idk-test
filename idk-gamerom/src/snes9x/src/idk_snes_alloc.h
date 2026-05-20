#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* idk_snes_malloc(size_t size);
void* idk_snes_calloc(size_t count, size_t size);
void idk_snes_free(void* ptr);

#ifdef __cplusplus
}
#endif
