#include <stdlib.h>

void swap(unsigned int *a, unsigned int *b) {
  unsigned int tmp = *a;
  *a = *b;
  *b = tmp;
}

void reverse_array(unsigned int* values, size_t length) {
  if (length <= 1) {
    return;
  }
  
  for (unsigned int i = 0; i < length / 2; i++) {
    swap(values + i, values + length - 1 - i);
  }
}