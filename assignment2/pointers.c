#include "lib.h"

void array_apply(int (*f)(int), int *array, size_t n) {
  for (unsigned int i = 0; i < n; i++) {
    array[i] = f(array[i]);
  }
}