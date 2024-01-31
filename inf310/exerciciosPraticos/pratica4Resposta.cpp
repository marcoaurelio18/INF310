#include <stdio.h>
#include <omp.h>
#include <random>

void createMatrix(int **a, int m, int n) {
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j)
            a[i][j]=i+j;
    }
}

void deleteMatrix(int**a, int m){
    for (int i=0; i<m; ++i)
        delete [] a[i];
    delete [] a;
}

/** Multiplicação de matrizes de forma paralela.
 * Alterar as linhas comentadas para testar cada uma das 3 abordagens de paralelização.
 */
void multiMatrix(int **A, int **B, int **res, int am, int an, int bn) {
    int i, j, k, r;
    #pragma omp parallel for default(none) shared(A,B,res,am,an,bn) private(i,j,k,r)
    for (i = 0; i < am; i++)
        // #pragma omp parallel for default(none) shared(A,B,res,am,an,bn,i) private(j,k,r)
        for (j = 0; j < bn; j++) {
            r=0;
            // #pragma omp parallel for default(none) shared(A,B,res,am,an,bn,i,j) private(k) reduction(+:r)
            for (k = 0; k < an; k++)
                r += A[i][k] * B[k][j];
            res[i][j]=r;
        }
}

void multiMatrixSeq(int **A, int **B, int **res, int am, int an, int bn) {
    int i, j, k, r;
    for (i = 0; i < am; i++)
        for (j = 0; j < bn; j++) {
            r=0;
            for (k = 0; k < an; k++)
                r += A[i][k] * B[k][j];
            res[i][j]=r;
        }
}

void transposeMatrix(int **A, int **T, int am, int an) {
    #pragma omp parallel for default(none) shared(A,T,am,an)
    for (int i=0; i<am; ++i)
        for (int j=0; j<an; ++j)
            T[j][i]=A[i][j];
}

/* Multiplicação da matriz transposta ganha eficiência ao diminuir cache miss. */
void multiMatrixTrans(int **A, int **B, int **res, int am, int an, int bn) {
    int **T=new int*[bn];
    for (int i=0; i<bn; ++i) 
        T[i]=new int[an];
    transposeMatrix(B,T,an,bn);

    int i, j, k, r;
    #pragma omp parallel for default(none) shared(A,T,res,am,an,bn) private(i,j,k,r)
    for (i = 0; i < am; i++)
        // #pragma omp parallel for default(none) shared(A,T,res,am,an,bn,i) private(j,k,r)
        for (j = 0; j < bn; j++) {
            r=0;
            // #pragma omp parallel for default(none) shared(A,T,res,am,an,bn,i,j) private(k) reduction(+:r)
            for (k = 0; k < an; k++)
                r += A[i][k] * T[j][k];
            res[i][j]=r;
        }
}

bool compareMatrix(int **a, int **b, int m, int n){
    for (int i=0; i<m; ++i)
        for (int j=0; j<n; ++j)
            if(a[i][j]!=b[i][j])
                return false;
    return true;
}

void printMatrix(int **a, int m, int n){
    for (int i=0; i<m; ++i) {
        for (int j=0; j<n; ++j) {
            printf("%4d ",a[i][j]);
            if(j>=9) {
                printf("...");
                break;
            }
        }
        printf("\n");
        if (i>=9) {
            printf("...\n");
            break;
        }
    }
    printf("\n");
}

int main() {
    int am=1000;
    int an=1000;
    int bm=an;
    int bn=1000;

    // Alocação dinâmica de arrays (mais eficiente que trabalhar com vector)
    int **a=new int*[am];
    for (int i=0; i<am; ++i) 
        a[i]=new int[an];
    int **b=new int*[bm];
    for (int i=0; i<bm; ++i) 
        b[i]=new int[bn];
    int **c=new int*[am];       //Matriz produto (calculada de forma paralela)
    for (int i=0; i<am; ++i) 
        c[i]=new int[bn];
    int **d=new int*[am];       //Matriz produto (calculada de forma sequencial)
    for (int i=0; i<am; ++i) 
        d[i]=new int[bn];

    createMatrix(a,am,an);
    createMatrix(b,bm,bn);

    double t1=omp_get_wtime();
    multiMatrix(a,b,c,am,an,bn);
    // multiMatrixTrans(a,b,c,am,an,bn);
    double t2=omp_get_wtime();

    multiMatrixSeq(a,b,d,am,an,bn);
    double t3=omp_get_wtime();

    /*
    printMatrix(a,am,an);
    printMatrix(b,bm,bn);
    printMatrix(c,am,bn);
    printMatrix(d,am,bn);
    if (compareMatrix(c,d,am,bn))
        printf("Resultado correto\n");
    else
        printf("%sRESULTADO INCORRETO!!!%s\n","\033[31m","\033[0m");

    */
    deleteMatrix(a,am);
    deleteMatrix(b,bm);
    deleteMatrix(c,am);
    deleteMatrix(d,am);

    printf("tempo paralelo:   %.3f ms\n",(t2-t1)*1000);
    printf("tempo sequencial: %.3f ms\n",(t3-t2)*1000);
    return 0;
}