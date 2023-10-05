#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stddef.h>

struct student {
    char fname[100];
    char lname[100];
    int year;
    int age;
};

extern struct student class[];

int compare_first_name(const void *a, const void *b);
int compare_last_name(const void *a, const void *b);
void apply(struct student *sarr, int nrec, void(*fp)(void *prec, void *arg, char *out), void *arg, char *out);
void printrec(void *prec, void *arg, char *out);
void isolder(void *prec, void *arg, char *out);

//bonus
void mysort(void *base, size_t num, size_t width, int (*comparator)(const void *, const void *));

struct student class[] = {
    {"Sean", "Penn", 2, 21},
    {"Sean", "Connery", 4, 25},
    {"Angelina", "Jolie", 3, 22},
    {"Meryl", "Streep", 4, 29},
    {"Robin", "Williams", 3, 32},
    {"Bill", "Gates", 3, 17},
    {"Jodie", "Foster", 4, 25},
    {"John", "Travolta", 1, 17},
    {"Isaac", "Newton", 2, 19},
    {"Sarah", "Palin", 2, 19}};

/*
 @function compare_first_name
 @desc     compares first name of two records.
 */
int compare_first_name(const void *a, const void *b)
{
  struct student *first = (struct student *)a;
  struct student *second = (struct student *)b;
  return strcmp(first->fname, second->fname);
}

/*
 @function compare_last_name
 @desc     compares last name of two records.
 */
int compare_last_name(const void *a, const void *b)
{
  struct student *first = (struct student *)a;
  struct student *second = (struct student *)b;
  return strcmp(first->lname, second->lname);
}

/*!
 @function apply
 @desc     applies given function to each array element
 */
void apply(struct student *sarr, int nrec, void (*fp)(void *prec, void *arg, char *out),
           void *arg, char *out)
{
  for (int i = 0; i < nrec; i++) {
    fp(&sarr[i], arg, out);
  }
}

/*
 @function printrec
 @desc     prints student record
 */
void printrec(void *prec, void *arg, char *out)
{
  struct student *pstud = (struct student *)prec;
  printf("%-20s %-20s %2d %2d\n", pstud->fname, pstud->lname, pstud->year,
         pstud->age);

  // also write into the given buffer for testing/"grading" purposes
  char temp[256];
  snprintf(temp, 256, "%-20s %-20s %2d %2d\n", pstud->fname, pstud->lname, pstud->year,
           pstud->age);
  strcat(out, temp);
}

/*
 @function isolder
 @desc     prints student record only if the student is older than *((int*)arg)
 NOTE: use printrec to print the record
 */
void isolder(void *prec, void *arg, char *out)
{
  struct student *student = (struct student *)prec;
  if (student->age > *((int*)arg)) {
    printrec(prec, arg, out);
  }
}

// PART OF BONUS SOLUTION: as an example we implement heapsort
// https://en.wikipedia.org/wiki/Heapsort
// built on https://www.geeksforgeeks.org/iterative-heap-sort/

// Helper function to swap two elements in the array
void swap(void *a, void *b, size_t width)
{
  // void* temp = alloca(width);
  // not recommmended to use alloca in modern C (99+)
  char temp[width];

  memcpy(temp, a, width);
  memcpy(a, b, width);
  memcpy(b, temp, width);
}

//

// Helper function to build a max heap
// function build Max Heap where value
// of each child is always smaller
// than value of their parent
void buildMaxHeap(void *base, size_t n, size_t size,
                  int (*cmp)(const void *, const void *))
{
  for (int i = 1; i < n; i++)
  {
    // if child is bigger than parent
    if (cmp(base + i * size, base + (i - 1) / 2 * size) > 0)
    {
      int j = i;

      // swap child and parent until
      // parent is smaller
      while (cmp(base + j * size, base + (j - 1) / 2 * size) > 0)
      {
        swap(base + j * size, base + (j - 1) / 2 * size, size);
        j = (j - 1) / 2;
      }
    }
  }
}
//---------------------------------
/*
  @function mysort
  @desc     see qsort, sorts in-place given a pointer, length, width and comparator
  NOTE: this is a bit complex and time-consuming, whatever sorting algorithm you use
*/
void mysort(void *base, size_t num, size_t width,
            int (*comparator)(const void *, const void *))
{
  buildMaxHeap(base, num, width, comparator);
  size_t c = num - 1;
  while (c > 0) {
    swap(base + c * width, base, width);
    buildMaxHeap(base, c, width, comparator);
    c--;
  }
}

int compare(int *a, int *b) {
    return *a - *b;
}

int main() {
    int values[] = { 12, 5, 8, 9, 100, 1 };

    int length = sizeof(values) / sizeof(values[0]);

    mysort(values, length, sizeof(values[0]), &compare);

    for (int i = 0; i < length; i++) {
        printf("%d\n", values[i]);
    }
}
