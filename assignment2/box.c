#include <stdio.h>
int foo(int *bar, int **baz)
{
    *bar = 5;
    *(bar + 1) = 6;
    *baz = bar + 2;
    return *((*baz) + 1);
}

int main(int argc, char **argv)
{
    int arr[4] = {1, 2, 3, 4};
    int *ptr;
    arr[0] = foo(&(arr[0]), &ptr);
    printf("%d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], *ptr);
    return 0;
}
