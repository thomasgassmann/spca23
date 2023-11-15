unsigned long fun7(void *n1, int number) {
    if (n1 == NULL) {
        return 4294967295; // unsigned long max
    }

    if (*((int *)n1) > number) {
        return 2 * fun7(*(void **)(n1 + 8), number);
    } else if (*((int *)n1) != number) {
        return 1 + 2 * fun7(*(void **)(n1 + 16), number);
    } else {
        return 0;
    }
}
