#include <stdio.h>
#include <stdlib.h>
#define SIZE 800 

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

void matrix_multiply() {
    int i, j, k;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            // c[i][j] = 0; 
            for (k = 0; k < SIZE; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}


void init_matrices() {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            a[i][j] = (double)i * (double)j;
            b[i][j] = (double)i / ((double)j + 1.0);
            c[i][j] = 0.0;
        }
    }
}

int main() {
    printf("Starting initialization...\n");
    init_matrices();
    
    printf("Starting multiplication...\n");
    matrix_multiply(); 
    
    printf("Finished. C[100][100] = %f\n", c[100][100]);
    return 0;
}
