/*
Abordagens:
Primeira: 
tempo paralelo:   1243.745 ms
tempo sequencial: 3748.281 ms

Segunda: 
tempo paralelo:   11461.990 ms
tempo sequencial: 5024.859 ms

Terceira: 
tempo paralelo:   13971.963 ms
tempo sequencial: 3714.175 ms
*/

#include <iostream>
#include <omp.h>

using namespace std;
void multiplicarMatrizesSequencial(int **a, int **b, int **c, int n) {
    int r;
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++) {
            r = 0;
            for(int k = 0; k < n; k++) {
                r += a[i][k] * b[k][j];
            }
            c[i][j] = r;
        }
}

void multiplicarMatrizesParalelo(int **a, int **b, int **c, int n) {
    int i, j, k, r;
    #pragma omp parallel for default(none) shared(a,b,c,n) private(i,j,k,r) // abordagem 1
    for (i = 0; i < n; ++i) {
        // #pragma omp parallel default(none) shared(i,a,b,c,n) private(j,k,r) // abordagem 2
        for (j = 0; j < n; ++j) {
            r = 0;
            // #pragma omp parallel default(none) shared(i,j,a,b,c,n) private(k) reduction(+:r) // abordagem 3
            for (k = 0; k < n; ++k) {
                r += a[i][k] * b[k][j];
            }
            c[i][j] = r;
        }
    }

}

int main() {
    int n = 1000;
    
    int **a = new int *[n];
    int **b = new int *[n];
    int **c = new int *[n];
    int **d = new int *[n];

    for(int i = 0; i < n; i++) {
        a[i] = new int[n];
        b[i] = new int[n];
        c[i] = new int[n];
        d[i] = new int[n];
    }


    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++) {
            a[i][j] = i+j;
            b[i][j] = i+j;
        }

    for(int i = 0; i < n; i++) 
        for(int j = 0; j < n; j++) {
            c[i][j] = 0;
            d[i][j] = 0;
        }

    double t1=omp_get_wtime();
    multiplicarMatrizesParalelo(a, b, c, n);
    double t2=omp_get_wtime();    

    double t3=omp_get_wtime();
    multiplicarMatrizesSequencial(a,b,d,n);
    double t4=omp_get_wtime();
    
    printf("tempo paralelo:   %.3f ms\n",(t2-t1)*1000);
    printf("tempo sequencial: %.3f ms\n",(t4-t3)*1000);

    for (int i=0; i < n; ++i){
        delete [] a[i];
        delete [] b[i];
        delete [] c[i];
        delete [] d[i];
    }
    delete [] a;
    delete [] b;
    delete [] c;
    delete [] d;
    
    return 0;
}