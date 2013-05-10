#include <stdio.h>
#include <math.h>

#define MATRIX_SIZE 256

void print_linear_sys(const double matrix[][MATRIX_SIZE], const double b[]);
void print_answer(const double x[]);
void gaussian_elimination(double matrix[][MATRIX_SIZE], double b[], double x[]);

double matrix1[MATRIX_SIZE][MATRIX_SIZE], matrix2[MATRIX_SIZE][MATRIX_SIZE];
    double b1[MATRIX_SIZE], b2[MATRIX_SIZE];
    double answer1[MATRIX_SIZE], answer2[MATRIX_SIZE];

int main(void)
{
 //   double matrix1[MATRIX_SIZE][MATRIX_SIZE], matrix2[MATRIX_SIZE][MATRIX_SIZE];
 //   double b1[MATRIX_SIZE], b2[MATRIX_SIZE];
 //   double answer1[MATRIX_SIZE], answer2[MATRIX_SIZE];
    int i, j;
    
    printf("*** Gaussian Elimination ***\n");
    
    //Fill the matrix and right-hand side
    for(i = 0; i < MATRIX_SIZE; ++i)
    {
        for(j = 0; j < MATRIX_SIZE; ++j)
        {
            matrix1[i][j] = 1.0 / (i + j + 1);
            matrix2[i][j] = 2.0 / (i + j + 2);
        }
        b1[i] = 1;
        b2[i] = 2;
    }
    
    //print the matrix
    printf("Linear system 1: \n");
    print_linear_sys(matrix1, b1);
    printf("\n");
    
    printf("Linear system 2: \n");
    print_linear_sys(matrix2, b2);
    printf("\n");
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            gaussian_elimination(matrix1, b1, answer1);
        }

        #pragma omp section
        {
            gaussian_elimination(matrix2, b2, answer2);
        }
    }
    
    //#pragma omp master
    printf("Answer to linear system 2: \n");
    print_answer(answer1);
    printf("\n");
    
    printf("Answer to linear system 1: \n");
    print_answer(answer2);
    printf("\n");
    
    return 0;
}

void print_linear_sys(const double matrix[][MATRIX_SIZE], const double b[])
{
    int i, j;
    /*
    for(i = 0; i < MATRIX_SIZE; ++i)
    {
        printf("|");
        for(j = 0; j < MATRIX_SIZE; ++j)
        {
                printf("%.3f ",matrix[i][j]);
        }
        printf("|%s|%.3f|\n", (i == MATRIX_SIZE / 2) ? " X = " : "     ", b[i]);
    }*/
}

void print_answer(const double x[])
{
    int i;
    
    // for(i = 0; i < MATRIX_SIZE; ++i)
    //     printf("        x%d = %.3f\n", i, x[i]);
}

void gaussian_elimination(double matrix[][MATRIX_SIZE], double b[], double x[])
{
    int i, j, count;
    double ratio, temp;
    
    //Gaussian elimination
    for(i = 0; i < (MATRIX_SIZE - 1); ++i)
    {
        for( j = (i + 1); j < MATRIX_SIZE; ++j)
        {
            ratio = matrix[j][i] / matrix[i][i];
            for(count = i; count < MATRIX_SIZE; ++count) 
                matrix[j][count] -= (ratio * matrix[i][count]);
            b[j] -= (ratio * b[i]);
        }
    }
    //Back substitution
    x[MATRIX_SIZE - 1] = b[MATRIX_SIZE - 1] / matrix[MATRIX_SIZE - 1][MATRIX_SIZE - 1];
    for(i = (MATRIX_SIZE - 2); i >= 0; --i)
    {
        temp = b[i];
        for(j = (i + 1); j < MATRIX_SIZE; ++j)
                temp -= (matrix[i][j] * x[j]);
        x[i] = temp / matrix[i][i];
    }
}
