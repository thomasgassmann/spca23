#include <stdio.h>

extern void quicksort(int *x, int *y);

int main() {
    int arr[7] = {2, 6, 1, 0, 10, 50, 1};
    quicksort(&arr[0], &arr[6]);
}
