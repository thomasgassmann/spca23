#include <stdio.h>

extern void quicksort(int *x, int *y);

// void swap(int *x, int *y) {
//     int tmp = *x;
//     *x = *y;
//     *y = tmp;
// }

// int *partition(int *x, int *y) {
//     int pivot = *y;

//     int *i = x - 1;
//     for (int *j = x; j <= y - 1; j++) {
//         if (*j <= pivot) {
//             i++;
//             swap(i, j);
//         }
//     }

//     i++;
//     swap(i, y);
//     return i;
// }

// void quicksort(int *x, int *y) {
//     if (x >= y) {
//         return;
//     }

//     int *p = partition(x, y);
//     quicksort(x, p - 1);
//     quicksort(p + 1, y);
// }

int main() {
    int arr[7] = {2, 6, 1, 0, 10, 50, 1};
    for (int i = 0; i < 7; i++) {
        printf("%d\n", arr[i]);
    }

    printf("Sorting...\n");
    quicksort(&arr[0], &arr[6]);
    for (int i = 0; i < 7; i++) {
        printf("%d\n", arr[i]);
    }
}
