#include <inttypes.h>  // for PRIu64
#include <stdio.h>
#include <stdlib.h>

#include "cycleclock.h"

static int *randmatrix(int size) {
	int i;
	int j;
        int* matrix = (int*)malloc(size*size*sizeof(int));

        for (i=0; i<size; i++) {
                for (j=0; j<size; j++) {
              		matrix[i+j*size] = random() / (RAND_MAX / 100);          
                }
        }

        return matrix;
}

#if 0

static void printrow(int size, int row, int* matrix) {
        int i;
        for (i=0; i<size; i++) {
                printf("%10d", matrix[row*size+i]);
        }        
        printf("\n");
}

static void printmatrix(int size, int *matrix, char name) {
	int i;
	printf("Matrix %c:\n", name);
	for (i=0; i<size; i++) {
		printrow(size, i, matrix);
	}
}

#endif

static int *orig_mmul(int size, int *A, int *B) {
	int i,j,k;
	int sum;
	int* result = (int*)malloc(size*size*sizeof(int));

#ifndef NO_CYC
  uint64_t start = cycleclock_now();
#endif

	for (i=0; i<size; i++) {
		for (j=0; j<size; j++) {
			sum = 0;
			for (k=0; k<size; k++) {
				sum += A[i*size+k] * B[k*size+j];
			}
			result[j+i*size] = sum;
		}
	}

#ifndef NO_CYC
  uint64_t end = cycleclock_now();
  printf("%" PRIu64 " ", end - start);
#endif

 	return result; 
}

static int *mmul(int size, int *A, int *B) {
	int i,j,k;
	int sum;
	int tmp;
	int row_a, row_b;
	int off_a, off_b, j_limit, k_limit;
	int result_offset;
	int* result = (int*)malloc(size*size*sizeof(int));

#ifndef NO_CYC
  uint64_t start = cycleclock_now();
#endif


	/* Transpose B */
	for(i=0; i<size; ++i) {
		row_a = i*size;
		row_b = (i+1)*size;
		for(j=i+1;j<size;++j) {
			tmp = B[row_a+j];
			B[row_a+j] = B[row_b+i];
			B[row_b+i] = tmp;
			row_b += size;
		}
	}
	
	j_limit = size - 3;
	k_limit = size - 7;

	row_a = 0;
	for (i=0; i<size; i++) {
		row_b = 0;
		result_offset = row_a;
		for (j=0; j<j_limit; j+=4) {
			// 1st
			sum = 0;
			for (k=0; k<k_limit; k+=8) {
				off_a = row_a + k; off_b = row_b+k;
				sum += A[off_a] * B[off_b]
					+ A[off_a+1] * B[off_b+1]
					+ A[off_a+2] * B[off_b+2]
					+ A[off_a+3] * B[off_b+3]
					+ A[off_a+4] * B[off_b+4]
					+ A[off_a+5] * B[off_b+5]
					+ A[off_a+6] * B[off_b+6]
					+ A[off_a+7] * B[off_b+7];
			}
			for(; k<size; k++)
				sum += A[row_a + k] * B[row_b + k];
			result[result_offset] = sum;
			row_b += size;
			result_offset += 1;
			// 2nd
			sum = 0;
			for (k=0; k<k_limit; k+=8) {
				off_a = row_a + k; off_b = row_b+k;
				sum += A[off_a] * B[off_b]
					+ A[off_a+1] * B[off_b+1]
					+ A[off_a+2] * B[off_b+2]
					+ A[off_a+3] * B[off_b+3]
					+ A[off_a+4] * B[off_b+4]
					+ A[off_a+5] * B[off_b+5]
					+ A[off_a+6] * B[off_b+6]
					+ A[off_a+7] * B[off_b+7];
			}
			for(; k<size; k++)
				sum += A[row_a + k] * B[row_b + k];
			result[result_offset] = sum;
			row_b += size;
			result_offset += 1;
			// 3st
			sum = 0;
			for (k=0; k<k_limit; k+=8) {
				off_a = row_a + k; off_b = row_b+k;
				sum += A[off_a] * B[off_b]
					+ A[off_a+1] * B[off_b+1]
					+ A[off_a+2] * B[off_b+2]
					+ A[off_a+3] * B[off_b+3]
					+ A[off_a+4] * B[off_b+4]
					+ A[off_a+5] * B[off_b+5]
					+ A[off_a+6] * B[off_b+6]
					+ A[off_a+7] * B[off_b+7];
			}
			for(; k<size; k++)
				sum += A[row_a + k] * B[row_b + k];
			result[result_offset] = sum;
			row_b += size;
			result_offset += 1;
			// 4nd
			sum = 0;
			for (k=0; k<k_limit; k+=8) {
				off_a = row_a + k; off_b = row_b+k;
				sum += A[off_a] * B[off_b]
					+ A[off_a+1] * B[off_b+1]
					+ A[off_a+2] * B[off_b+2]
					+ A[off_a+3] * B[off_b+3]
					+ A[off_a+4] * B[off_b+4]
					+ A[off_a+5] * B[off_b+5]
					+ A[off_a+6] * B[off_b+6]
					+ A[off_a+7] * B[off_b+7];
			}
			for(; k<size; k++)
				sum += A[row_a + k] * B[row_b + k];
			result[result_offset] = sum;
			row_b += size;
			result_offset += 1;

		} 
		for(; j<size;j++) {
			sum = 0;
			for (k=0; k<k_limit; k+=8) {
				off_a = row_a + k; off_b = row_b+k;
				sum += A[off_a] * B[off_b]
					+ A[off_a+1] * B[off_b+1]
					+ A[off_a+2] * B[off_b+2]
					+ A[off_a+3] * B[off_b+3]
					+ A[off_a+4] * B[off_b+4]
					+ A[off_a+5] * B[off_b+5]
					+ A[off_a+6] * B[off_b+6]
					+ A[off_a+7] * B[off_b+7];
			}
			for(; k<size; k++)
				sum += A[row_a + k] * B[row_b + k];
			result[row_a + j] = sum;
			row_b += size;
		} 
		row_a += size;
	}


#ifndef NO_CYC
  uint64_t end = cycleclock_now();
  printf("%" PRIu64 " ", end - start);
#endif

 	return result; 
}


#ifdef TEST
static void correctness(int *A, int *B, int *C, int size) {
	int i,j,k,sum;
	int err = 0;
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++) {
			sum = 0;
			for (k=0; k<size; k++) {
				sum += A[i*size+k] * B[k*size+j];
			}
			if((C[j+i*size] - sum) != 0) {
				++err;
				printf("Wrong value for C[%d,%d] (is: %d, should: %d)\n",i,j, C[i+j*size],sum);
			}
		}
	}
	printf("%d errors\n", err);
}
#endif

int *copy(int *M, int size) {
	int elems = size*size;
	int i;
	int *result = (int *)malloc(sizeof(int)*elems);
	for(i=0;i<elems;++i)
		result[i] = M[i];
	return result;
}

void run(int size) {
	int *A, *B, *C;
	A = randmatrix(size);
	B = randmatrix(size);
	C = mmul(size, A, B);
	return;
}

void orig_run(int size) {
	int *A, *B, *C;
	A = randmatrix(size);
	B = randmatrix(size);
	C = orig_mmul(size, A, B);
	return;
}

#ifdef TEST
void run_check(int size) {
	int *A, *B, *Bs, *C;
	A = randmatrix(size);
	B = randmatrix(size);
	Bs = copy(B, size);
	C = mmul(size, A, B);
	correctness(A, Bs, C, size);
	return;
}
#endif

#ifndef NO_MAIN

int main(int argc, char **argv) {
	int size;
	int* A;
	int* B;
	int *Bs;
	int* C;

	if (argc != 2) {
		printf("USAGE: mmul <matrix_size>\n");
		return 1;
	}	
	size = atoi(argv[1]); 
	printf("CASP Simple Matrix Multiplicator. Matrix size: %d\n", size);

	A = randmatrix(size);
//	printmatrix(size, A, 'A');

	B = randmatrix(size);
	Bs = copy(B, size);
//	printmatrix(size, B, 'B');

	C = mmul(size, A, B);
//	printmatrix(size, C, 'C');

#ifdef TEST
	correctness(A, Bs, C, size);
#endif

	free(A);
	free(B);
	free(Bs);
	free(C);
	return 0;
}

#endif
