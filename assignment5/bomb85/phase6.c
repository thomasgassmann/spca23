struct node {
    int data;
    int num;
    struct node *next;
};

void phase6() {
    int values[6];
    struct node *result[6];

    int ci = 0;
    do {
        int current = values[ci];
        int counter = 1;
        struct node *addr = &node1;
        if (current > 1) {
            do {
                addr = addr->next;
                counter++;
            } while (counter != current);
        }

        result[ci] = addr;
        ci++;
    } while (ci != 6);

    // for values[i] result[i] contains 
}