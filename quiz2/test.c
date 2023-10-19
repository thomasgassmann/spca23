#include <stdio.h>

extern int funcf(int x);
extern int funcg(int x, int y);
extern int funch(int x, int y);

int main() {
    printf("%d = 5\n", funcf(-1));
    printf("%d = 15\n", funcf(1));
    printf("%d = 5\n", funcf(0));

    printf("%d = 12\n", funcg(3, 1));
    printf("%d = 9\n", funcg(2, 2));
    printf("%d = 12\n", funcg(3, 5));
    printf("%d = 183\n", funcg(60, 1));
}
