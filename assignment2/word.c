#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct word {
  char* word_string;
  size_t length;
} word;

word find_first_word(char* input_string) {
  word result = {.word_string = NULL, .length = 0};
  if (input_string == NULL || *input_string == '\0') {
    return result;
  }
  
  while (*input_string == ' ') {
    input_string++;
  }
  
  char *start = input_string;
  while (*(start + result.length) != '\0' && *(start + result.length) != ' ') {
    result.length++;
  }
  
  result.word_string = calloc(sizeof(char), result.length + 1);
  strncpy(result.word_string, start, result.length);
  return result;
}

int main() {
    word w = find_first_word("   lorem ipsum");
    printf("%s\n", w.word_string);
}
