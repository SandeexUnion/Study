#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

typedef struct {
    uint8_t r, g, b;
} rgb_pixel;

typedef struct {
    int width, height;
    rgb_pixel* pixels;
} image_t;

image_t* create_test_image(int width, int height) {
    image_t* img = (image_t*)malloc(sizeof(image_t));
    img->width = width;
    img->height = height;
    img->pixels = (rgb_pixel*)malloc(width * height * sizeof(rgb_pixel));
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img->pixels[y * width + x].r = (x * 255) / width;
            img->pixels[y * width + x].g = (y * 255) / height;
            img->pixels[y * width + x].b = ((x + y) * 255) / (width + height);
        }
    }
    return img;
}

void free_image(image_t* img) {
    free(img->pixels);
    free(img);
}

uint8_t rgb_to_gray_scalar(uint8_t r, uint8_t g, uint8_t b) {
    return (uint8_t)(0.2126f * r + 0.7152f * g + 0.0722f * b);
}

void grayscale_sequential(rgb_pixel* rgb, uint8_t* gray, int n_pixels) {
    for (int i = 0; i < n_pixels; i++) {
        gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
    }
}

void grayscale_parallel(rgb_pixel* rgb, uint8_t* gray, int n_pixels, int num_threads) {
    omp_set_num_threads(num_threads);
    #pragma omp parallel for
    for (int i = 0; i < n_pixels; i++) {
        gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
    }
}

uint8_t sobel_pixel_optimized(uint8_t* gray, int x, int y, int width) {
    int gx = 0, gy = 0;
    
    int row_above = (y - 1) * width;
    int row_mid   = y * width;
    int row_below = (y + 1) * width;
    
    gx += gray[row_above + (x-1)] * -1;
    gy += gray[row_above + (x-1)] * -1;
    gy += gray[row_above + x] * -2;
    gx += gray[row_above + (x+1)] * 1;
    gy += gray[row_above + (x+1)] * -1;
    
    gx += gray[row_mid + (x-1)] * -2;
    gx += gray[row_mid + (x+1)] * 2;
    
    gx += gray[row_below + (x-1)] * -1;
    gy += gray[row_below + (x-1)] * 1;
    gy += gray[row_below + x] * 2;
    gx += gray[row_below + (x+1)] * 1;
    gy += gray[row_below + (x+1)] * 1;
    
    float g = sqrtf((float)(gx * gx + gy * gy));
    int val = (int)g;
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    return (uint8_t)val;
}

void sobel_sequential(uint8_t* gray, uint8_t* result, int width, int height) {
    memcpy(result, gray, width);
    memcpy(result + (height-1)*width, gray + (height-1)*width, width);
    
    for (int y = 0; y < height; y++) {
        result[y * width] = gray[y * width];
        result[y * width + width - 1] = gray[y * width + width - 1];
    }
    
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
        }
    }
}

void sobel_hybrid(uint8_t* gray, uint8_t* result, int width, int height, 
                  int num_threads, double* comp_time, double* comm_time) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double t_start, t_end;
    
    int rows_per_proc = (height - 2) / size;
    int remainder = (height - 2) % size;
    
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    int local_height = local_rows + 2;
    
    uint8_t* local_gray = (uint8_t*)calloc(local_height * width, sizeof(uint8_t));
    uint8_t* local_result = (uint8_t*)malloc(local_rows * width * sizeof(uint8_t));
    
    int *sendcounts = NULL;
    int *displs = NULL;
    int *recvcounts = NULL;
    int *recv_displs = NULL;
    
    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        recvcounts = (int*)malloc(size * sizeof(int));
        recv_displs = (int*)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int rows = rows_per_proc + (i < remainder ? 1 : 0);
            sendcounts[i] = rows * width;
            displs[i] = offset;
            recvcounts[i] = rows * width;
            recv_displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    
    t_start = MPI_Wtime();
    MPI_Scatterv(gray, sendcounts, displs, MPI_UINT8_T,
                 local_gray + width, local_rows * width, MPI_UINT8_T,
                 0, MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    *comm_time += (t_end - t_start);
    
    t_start = MPI_Wtime();
    int prev = rank - 1;
    int next = rank + 1;
    
    if (prev >= 0) {
        MPI_Sendrecv(local_gray + width, width, MPI_UINT8_T, prev, 0,
                     local_gray, width, MPI_UINT8_T, prev, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    if (next < size) {
        MPI_Sendrecv(local_gray + local_rows * width, width, MPI_UINT8_T, next, 0,
                     local_gray + (local_rows + 1) * width, width, MPI_UINT8_T, next, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    t_end = MPI_Wtime();
    *comm_time += (t_end - t_start);
    
    omp_set_num_threads(num_threads);
    
    t_start = MPI_Wtime();
    #pragma omp parallel for
    for (int y = 1; y <= local_rows; y++) {
        for (int x = 1; x < width - 1; x++) {
            local_result[(y-1) * width + x] = sobel_pixel_optimized(local_gray, x, y, width);
        }
    }
    t_end = MPI_Wtime();
    *comp_time += (t_end - t_start);
    
    t_start = MPI_Wtime();
    MPI_Gatherv(local_result, local_rows * width, MPI_UINT8_T,
                result, recvcounts, recv_displs, MPI_UINT8_T,
                0, MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    *comm_time += (t_end - t_start);
    
    if (rank == 0) {
        memcpy(result, gray, width);
        memcpy(result + (height-1)*width, gray + (height-1)*width, width);
        
        for (int y = 0; y < height; y++) {
            result[y * width] = gray[y * width];
            result[y * width + width - 1] = gray[y * width + width - 1];
        }
    }
    
    free(local_gray);
    free(local_result);
    
    if (rank == 0) {
        free(sendcounts);
        free(displs);
        free(recvcounts);
        free(recv_displs);
    }
}

void run_experiment(int width, int height, int num_procs, int num_threads) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    image_t* img = NULL;
    uint8_t* gray = NULL;
    uint8_t* gray_seq = NULL;
    uint8_t* result_seq = NULL;
    uint8_t* result_hyb = NULL;
    
    double time_seq = 0.0;
    double time_hyb_comp = 0.0;
    double time_hyb_comm = 0.0;
    double time_hyb_total = 0.0;
    double time_gray_seq = 0.0;
    double time_gray_par = 0.0;
    
    int n_pixels = width * height;
    
    if (rank == 0) {
        img = create_test_image(width, height);
        gray = (uint8_t*)malloc(n_pixels);
        gray_seq = (uint8_t*)malloc(n_pixels);
        result_seq = (uint8_t*)malloc(n_pixels);
        result_hyb = (uint8_t*)malloc(n_pixels);
        
        double start_gray_seq = MPI_Wtime();
        grayscale_sequential(img->pixels, gray_seq, n_pixels);
        double end_gray_seq = MPI_Wtime();
        time_gray_seq = end_gray_seq - start_gray_seq;
        
        double start_seq = MPI_Wtime();
        sobel_sequential(gray_seq, result_seq, width, height);
        double end_seq = MPI_Wtime();
        time_seq = end_seq - start_seq;
        
        double start_gray_par = MPI_Wtime();
        grayscale_parallel(img->pixels, gray, n_pixels, num_threads);
        double end_gray_par = MPI_Wtime();
        time_gray_par = end_gray_par - start_gray_par;
    } else {
        gray = (uint8_t*)malloc(n_pixels);
        result_hyb = (uint8_t*)malloc(n_pixels);
        memset(gray, 0, n_pixels);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    double start_hyb = MPI_Wtime();
    sobel_hybrid(gray, result_hyb, width, height, num_threads, &time_hyb_comp, &time_hyb_comm);
    double end_hyb = MPI_Wtime();
    time_hyb_total = end_hyb - start_hyb;
    
    if (rank == 0) {
        double total_seq = time_gray_seq + time_seq;
        double speedup = total_seq / time_hyb_total;
        double efficiency = speedup / (num_procs * num_threads);
        
        printf("========================================\n");
        printf("Размер: %dx%d\n", width, height);
        printf("MPI: %d, OMP: %d, Всего ядер: %d\n", num_procs, num_threads, num_procs * num_threads);
        printf("----------------------------------------\n");
        printf("Grayscale sequential: %.6f сек\n", time_gray_seq);
        printf("Sobel sequential:     %.6f сек\n", time_seq);
        printf("Total sequential:     %.6f сек\n", total_seq);
        printf("----------------------------------------\n");
        printf("Grayscale parallel:   %.6f сек\n", time_gray_par);
        printf("----------------------------------------\n");
        printf("Гибридный Sobel:\n");
        printf("  Вычисления:   %.6f сек (%.1f%%)\n", time_hyb_comp, 
               (time_hyb_comp/time_hyb_total)*100);
        printf("  Коммуникации: %.6f сек (%.1f%%)\n", time_hyb_comm, 
               (time_hyb_comm/time_hyb_total)*100);
        printf("  Общее время:  %.6f сек\n", time_hyb_total);
        printf("----------------------------------------\n");
        printf("Ускорение:          %.2fx\n", speedup);
        printf("Эффективность:      %.2f\n", efficiency);
        printf("========================================\n\n");
        
        free(gray);
        free(gray_seq);
        free(result_seq);
        free(result_hyb);
        free_image(img);
    } else {
        free(gray);
        free(result_hyb);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (rank == 0) {
        printf("========================================\n");
        printf("ЛАБОРАТОРНАЯ РАБОТА No3\n");
        printf("Гибридное программирование: MPI + OpenMP\n");
        printf("Детектирование границ методом Собеля\n");
        printf("========================================\n\n");
    }
    
    int sizes[3][2] = {
        {512, 512},
        {1024, 768},
        {1920, 1080}
    };
    
    int proc_counts[] = {1, 2, 4, 8};
    int thread_counts[] = {1, 2, 4, 8};
    
    for (int s = 0; s < 3; s++) {
        int width = sizes[s][0];
        int height = sizes[s][1];
        
        for (int p = 0; p < 4; p++) {
            for (int t = 0; t < 4; t++) {
                if (proc_counts[p] * thread_counts[t] <= 8) {
                    run_experiment(width, height, proc_counts[p], thread_counts[t]);
                }
            }
        }
    }
    
    MPI_Finalize();
    return 0;
}