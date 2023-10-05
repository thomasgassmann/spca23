#include <stdio.h>

int main() {
    int value = 0x000000FF;

    char *p = (char *)&value;
    if (*p == 0xFF) {
        printf("Little endian\n");
    } else {
        printf("Big endian\n");
    }
}
