#include <stdlib.h>
#include <string.h>
#include "noftypes.h"

bool mem_debug = false;

void * _my_malloc(int size) {
  return malloc(size);
}

void _my_free(void **data) {
  if (data && *data) {
    free(*data);
    *data = NULL;
  }
}

char * _my_strdup(const char *string) {
  if (!string) return NULL;
  size_t len = strlen(string) + 1;
  char *copy = (char *)malloc(len);
  if (copy) memcpy(copy, string, len);
  return copy;
}

void mem_cleanup(void) {}
void mem_checkblocks(void) {}
void mem_checkleaks(void) {}
