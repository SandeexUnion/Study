#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main() {
    const int N = 10000000;
    int* data = (int*)malloc(N * sizeof(int));
    long long sum = 0;
    
    // Инициализация
    for (int i = 0; i < N; i++) {
        data[i] = 1;
    }
    
    // Массив со стратегиями для тестирования
    const char* schedule_names[] = {"static", "dynamic", "guided", "auto"};
    
    omp_set_num_threads(4);
    printf("Тестирование на %d потоках:\n\n", omp_get_max_threads());
    
    for (int s = 0; s < 4; s++) {
        sum = 0;
        double start = omp_get_wtime();
        
        // Выбираем стратегию в зависимости от s
        switch(s) {
            case 0:
                #pragma omp parallel for schedule(static) reduction(+:sum)
                for (int i = 0; i < N; i++) sum += data[i];
                break;
            case 1:
                #pragma omp parallel for schedule(dynamic) reduction(+:sum)
                for (int i = 0; i < N; i++) sum += data[i];
                break;
            case 2:
                #pragma omp parallel for schedule(guided) reduction(+:sum)
                for (int i = 0; i < N; i++) sum += data[i];
                break;
            case 3:
                #pragma omp parallel for schedule(auto) reduction(+:sum)
                for (int i = 0; i < N; i++) sum += data[i];
                break;
        }
        
        double end = omp_get_wtime();
        printf("%s: %f сек, сумма = %lld\n", 
               schedule_names[s], end - start, sum);
    }
    
    free(data);
    return 0;
}