#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void Hello(void) {
    #ifdef _OPENMP
        int myself = omp_get_thread_num();
        int nThreads = omp_get_num_threads();
    #else
        int myself = 0;
        int nThreads = 1;
    #endif
    printf("Hello da thread %d de %d\n",myself,nThreads);
}
int main(){
    int a=1, b=2, c=3;
    #pragma omp parallel default(none) firstprivate(a) shared(b,c) \
    num_threads(2)
    {
    a = 2;
    b = c;
    c = a;
    }
    printf("%d, %d, %d\n", a,b,c);
}