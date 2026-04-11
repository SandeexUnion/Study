#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>
#include <omp.h>

// ==================== СТРУКТУРЫ ДАННЫХ ====================

typedef struct {
    uint8_t r, g, b;
} rgb_pixel;

typedef struct {
    int width, height;
    rgb_pixel* pixels;
} image_t;

// ==================== УТИЛИТЫ ====================

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

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

// ==================== СКАЛЯРНЫЕ ФУНКЦИИ ====================

uint8_t rgb_to_gray_scalar(uint8_t r, uint8_t g, uint8_t b) {
    return (uint8_t)(0.2126f * r + 0.7152f * g + 0.0722f * b);
}

void grayscale_scalar(rgb_pixel* rgb, uint8_t* gray, int n_pixels) {
    for (int i = 0; i < n_pixels; i++) {
        gray[i] = rgb_to_gray_scalar(rgb[i].r, rgb[i].g, rgb[i].b);
    }
}

const int GX[3][3] = {{-1,0,1}, {-2,0,2}, {-1,0,1}};
const int GY[3][3] = {{-1,-2,-1}, {0,0,0}, {1,2,1}};

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

void sobel_scalar(uint8_t* gray, uint8_t* result, int width, int height) {
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

// ==================== OPENMP SOBEL ФУНКЦИИ ====================

void sobel_openmp_static(uint8_t* gray, uint8_t* result, int width, int height, int num_threads) {
    omp_set_num_threads(num_threads);
    
    memcpy(result, gray, width);
    memcpy(result + (height-1)*width, gray + (height-1)*width, width);
    
    for (int y = 0; y < height; y++) {
        result[y * width] = gray[y * width];
        result[y * width + width - 1] = gray[y * width + width - 1];
    }
    
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
        }
    }
}

void sobel_openmp_dynamic(uint8_t* gray, uint8_t* result, int width, int height, int num_threads, int chunk_size) {
    omp_set_num_threads(num_threads);
    
    memcpy(result, gray, width);
    memcpy(result + (height-1)*width, gray + (height-1)*width, width);
    
    for (int y = 0; y < height; y++) {
        result[y * width] = gray[y * width];
        result[y * width + width - 1] = gray[y * width + width - 1];
    }
    
    if (chunk_size > 0) {
        #pragma omp parallel for collapse(2) schedule(dynamic, chunk_size)
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
            }
        }
    } else {
        #pragma omp parallel for collapse(2) schedule(dynamic)
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
            }
        }
    }
}

void sobel_openmp_guided(uint8_t* gray, uint8_t* result, int width, int height, int num_threads, int chunk_size) {
    omp_set_num_threads(num_threads);
    
    memcpy(result, gray, width);
    memcpy(result + (height-1)*width, gray + (height-1)*width, width);
    
    for (int y = 0; y < height; y++) {
        result[y * width] = gray[y * width];
        result[y * width + width - 1] = gray[y * width + width - 1];
    }
    
    if (chunk_size > 0) {
        #pragma omp parallel for collapse(2) schedule(guided, chunk_size)
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
            }
        }
    } else {
        #pragma omp parallel for collapse(2) schedule(guided)
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
            }
        }
    }
}

void sobel_openmp_auto(uint8_t* gray, uint8_t* result, int width, int height, int num_threads) {
    omp_set_num_threads(num_threads);
    
    memcpy(result, gray, width);
    memcpy(result + (height-1)*width, gray + (height-1)*width, width);
    
    for (int y = 0; y < height; y++) {
        result[y * width] = gray[y * width];
        result[y * width + width - 1] = gray[y * width + width - 1];
    }
    
    #pragma omp parallel for collapse(2) schedule(auto)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            result[y * width + x] = sobel_pixel_optimized(gray, x, y, width);
        }
    }
}

// ==================== OPENMP GRAYSCALE ФУНКЦИИ ====================

void grayscale_openmp_static(rgb_pixel* rgb, uint8_t* gray, int n_pixels, int num_threads) {
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n_pixels; i++) {
        gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
    }
}

void grayscale_openmp_dynamic(rgb_pixel* rgb, uint8_t* gray, int n_pixels, int num_threads, int chunk_size) {
    omp_set_num_threads(num_threads);
    
    if (chunk_size > 0) {
        #pragma omp parallel for schedule(dynamic, chunk_size)
        for (int i = 0; i < n_pixels; i++) {
            gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
        }
    } else {
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < n_pixels; i++) {
            gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
        }
    }
}

void grayscale_openmp_guided(rgb_pixel* rgb, uint8_t* gray, int n_pixels, int num_threads, int chunk_size) {
    omp_set_num_threads(num_threads);
    
    if (chunk_size > 0) {
        #pragma omp parallel for schedule(guided, chunk_size)
        for (int i = 0; i < n_pixels; i++) {
            gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
        }
    } else {
        #pragma omp parallel for schedule(guided)
        for (int i = 0; i < n_pixels; i++) {
            gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
        }
    }
}

void grayscale_openmp_auto(rgb_pixel* rgb, uint8_t* gray, int n_pixels, int num_threads) {
    omp_set_num_threads(num_threads);
    
    #pragma omp parallel for schedule(auto)
    for (int i = 0; i < n_pixels; i++) {
        gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
    }
}

// ==================== ФУНКЦИИ ДЛЯ АВТОВЕКТОРИЗАЦИИ ====================

void grayscale_auto(rgb_pixel* restrict rgb, uint8_t* restrict gray, int n_pixels) {
    for (int i = 0; i < n_pixels; i++) {
        gray[i] = (uint8_t)(0.2126f * rgb[i].r + 0.7152f * rgb[i].g + 0.0722f * rgb[i].b);
    }
}

void sobel_auto(uint8_t* restrict gray, uint8_t* restrict result, int width, int height) {
    memcpy(result, gray, width);
    memcpy(result + (height-1)*width, gray + (height-1)*width, width);
    
    for (int y = 0; y < height; y++) {
        result[y * width] = gray[y * width];
        result[y * width + width - 1] = gray[y * width + width - 1];
    }
    
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int gx = 0, gy = 0;
            
            int idx = (y-1) * width + (x-1);
            gx += gray[idx] * -1;
            gy += gray[idx] * -1;
            
            idx = (y-1) * width + x;
            gy += gray[idx] * -2;
            
            idx = (y-1) * width + (x+1);
            gx += gray[idx] * 1;
            gy += gray[idx] * -1;
            
            idx = y * width + (x-1);
            gx += gray[idx] * -2;
            
            idx = y * width + (x+1);
            gx += gray[idx] * 2;
            
            idx = (y+1) * width + (x-1);
            gx += gray[idx] * -1;
            gy += gray[idx] * 1;
            
            idx = (y+1) * width + x;
            gy += gray[idx] * 2;
            
            idx = (y+1) * width + (x+1);
            gx += gray[idx] * 1;
            gy += gray[idx] * 1;
            
            float g = sqrtf((float)(gx * gx + gy * gy));
            int val = (int)g;
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            result[y * width + x] = (uint8_t)val;
        }
    }
}

// ==================== SSE ФУНКЦИИ ====================

void grayscale_sse(rgb_pixel* rgb, uint8_t* gray, int n_pixels) {
    __m128 coeff_r = _mm_set1_ps(0.2126f);
    __m128 coeff_g = _mm_set1_ps(0.7152f);
    __m128 coeff_b = _mm_set1_ps(0.0722f);
    
    int i = 0;
    
    for (; i + 4 <= n_pixels; i += 4) {
        __m128i pixels = _mm_loadu_si128((__m128i*)(rgb + i));
        
        __m128i r16 = _mm_unpacklo_epi8(pixels, _mm_setzero_si128());
        __m128i g16 = _mm_unpackhi_epi8(pixels, _mm_setzero_si128());
        __m128i b16 = _mm_unpacklo_epi8(_mm_srli_si128(pixels, 8), _mm_setzero_si128());
        
        __m128i r32_lo = _mm_unpacklo_epi16(r16, _mm_setzero_si128());
        __m128i r32_hi = _mm_unpackhi_epi16(r16, _mm_setzero_si128());
        __m128i g32_lo = _mm_unpacklo_epi16(g16, _mm_setzero_si128());
        __m128i g32_hi = _mm_unpackhi_epi16(g16, _mm_setzero_si128());
        __m128i b32_lo = _mm_unpacklo_epi16(b16, _mm_setzero_si128());
        __m128i b32_hi = _mm_unpackhi_epi16(b16, _mm_setzero_si128());
        
        __m128 r_lo = _mm_cvtepi32_ps(r32_lo);
        __m128 r_hi = _mm_cvtepi32_ps(r32_hi);
        __m128 g_lo = _mm_cvtepi32_ps(g32_lo);
        __m128 g_hi = _mm_cvtepi32_ps(g32_hi);
        __m128 b_lo = _mm_cvtepi32_ps(b32_lo);
        __m128 b_hi = _mm_cvtepi32_ps(b32_hi);
        
        __m128 y_lo = _mm_add_ps(_mm_add_ps(_mm_mul_ps(r_lo, coeff_r), 
                                             _mm_mul_ps(g_lo, coeff_g)), 
                                  _mm_mul_ps(b_lo, coeff_b));
        __m128 y_hi = _mm_add_ps(_mm_add_ps(_mm_mul_ps(r_hi, coeff_r), 
                                             _mm_mul_ps(g_hi, coeff_g)), 
                                  _mm_mul_ps(b_hi, coeff_b));
        
        __m128i y32_lo = _mm_cvtps_epi32(y_lo);
        __m128i y32_hi = _mm_cvtps_epi32(y_hi);
        
        __m128i y16 = _mm_packs_epi32(y32_lo, y32_hi);
        __m128i y8 = _mm_packus_epi16(y16, _mm_setzero_si128());
        
        *(int*)(gray + i) = _mm_cvtsi128_si32(y8);
    }
    
    for (; i < n_pixels; i++) {
        gray[i] = rgb_to_gray_scalar(rgb[i].r, rgb[i].g, rgb[i].b);
    }
}

// ==================== ТЕСТИРОВАНИЕ OPENMP ====================

void test_openmp_strategies(int width, int height) {
    printf("\n========== ТЕСТИРОВАНИЕ OPENMP СТРАТЕГИЙ ==========\n");
    printf("Размер: %dx%d (%d пикселей)\n", width, height, width * height);
    
    image_t* img = create_test_image(width, height);
    int n_pixels = width * height;
    
    uint8_t* gray_seq = (uint8_t*)malloc(n_pixels);
    uint8_t* gray_omp = (uint8_t*)malloc(n_pixels);
    uint8_t* result = (uint8_t*)malloc(n_pixels);
    
    // Последовательный grayscale для сравнения
    double start_seq = omp_get_wtime();
    grayscale_scalar(img->pixels, gray_seq, n_pixels);
    double end_seq = omp_get_wtime();
    double time_seq_gray = end_seq - start_seq;
    
    // Последовательный Sobel для сравнения
    start_seq = omp_get_wtime();
    sobel_scalar(gray_seq, result, width, height);
    end_seq = omp_get_wtime();
    double time_seq_sobel = end_seq - start_seq;
    double time_seq_total = time_seq_gray + time_seq_sobel;
    
    printf("\nПоследовательная версия:\n");
    printf("  Grayscale: %.6f сек\n", time_seq_gray);
    printf("  Sobel:     %.6f сек\n", time_seq_sobel);
    printf("  TOTAL:     %.6f сек\n", time_seq_total);
    
    int thread_counts[] = {1, 2, 4, 8};
    int chunk_sizes[] = {0, 64, 256, 1024};
    
    printf("\n====================================================================================================\n");
    printf("| Стратегия | Потоки | Chunk | Grayscale (сек) | Sobel (сек) | Total (сек) | Ускорение | Эффективность |\n");
    printf("====================================================================================================\n");
    
    for (int t = 0; t < 4; t++) {
        int num_threads = thread_counts[t];
        
        // ===== STATIC =====
        memset(gray_omp, 0, n_pixels);
        memset(result, 0, n_pixels);
        
        double start = omp_get_wtime();
        grayscale_openmp_static(img->pixels, gray_omp, n_pixels, num_threads);
        double mid = omp_get_wtime();
        sobel_openmp_static(gray_omp, result, width, height, num_threads);
        double end = omp_get_wtime();
        
        double time_gray = mid - start;
        double time_sobel = end - mid;
        double time_total = end - start;
        double speedup = time_seq_total / time_total;
        double efficiency = speedup / num_threads;
        
        printf("| static      |   %2d   |   -   |    %.6f   |   %.6f  |  %.6f |   %.2fx   |     %.2f      |\n",
               num_threads, time_gray, time_sobel, time_total, speedup, efficiency);
        
        // ===== DYNAMIC =====
        for (int c = 0; c < 4; c++) {
            memset(gray_omp, 0, n_pixels);
            memset(result, 0, n_pixels);
            
            start = omp_get_wtime();
            grayscale_openmp_dynamic(img->pixels, gray_omp, n_pixels, num_threads, chunk_sizes[c]);
            mid = omp_get_wtime();
            sobel_openmp_dynamic(gray_omp, result, width, height, num_threads, chunk_sizes[c]);
            end = omp_get_wtime();
            
            time_gray = mid - start;
            time_sobel = end - mid;
            time_total = end - start;
            speedup = time_seq_total / time_total;
            efficiency = speedup / num_threads;
            
            if (chunk_sizes[c] > 0) {
                printf("| dynamic     |   %2d   | %4d |    %.6f   |   %.6f  |  %.6f |   %.2fx   |     %.2f      |\n",
                       num_threads, chunk_sizes[c], time_gray, time_sobel, time_total, speedup, efficiency);
            } else {
                printf("| dynamic     |   %2d   | auto |    %.6f   |   %.6f  |  %.6f |   %.2fx   |     %.2f      |\n",
                       num_threads, time_gray, time_sobel, time_total, speedup, efficiency);
            }
        }
        
        // ===== GUIDED =====
        for (int c = 0; c < 4; c++) {
            memset(gray_omp, 0, n_pixels);
            memset(result, 0, n_pixels);
            
            start = omp_get_wtime();
            grayscale_openmp_guided(img->pixels, gray_omp, n_pixels, num_threads, chunk_sizes[c]);
            mid = omp_get_wtime();
            sobel_openmp_guided(gray_omp, result, width, height, num_threads, chunk_sizes[c]);
            end = omp_get_wtime();
            
            time_gray = mid - start;
            time_sobel = end - mid;
            time_total = end - start;
            speedup = time_seq_total / time_total;
            efficiency = speedup / num_threads;
            
            if (chunk_sizes[c] > 0) {
                printf("| guided      |   %2d   | %4d |    %.6f   |   %.6f  |  %.6f |   %.2fx   |     %.2f      |\n",
                       num_threads, chunk_sizes[c], time_gray, time_sobel, time_total, speedup, efficiency);
            } else {
                printf("| guided      |   %2d   | auto |    %.6f   |   %.6f  |  %.6f |   %.2fx   |     %.2f      |\n",
                       num_threads, time_gray, time_sobel, time_total, speedup, efficiency);
            }
        }
        
        // ===== AUTO =====
        memset(gray_omp, 0, n_pixels);
        memset(result, 0, n_pixels);
        
        start = omp_get_wtime();
        grayscale_openmp_auto(img->pixels, gray_omp, n_pixels, num_threads);
        mid = omp_get_wtime();
        sobel_openmp_auto(gray_omp, result, width, height, num_threads);
        end = omp_get_wtime();
        
        time_gray = mid - start;
        time_sobel = end - mid;
        time_total = end - start;
        speedup = time_seq_total / time_total;
        efficiency = speedup / num_threads;
        
        printf("| auto        |   %2d   |   -   |    %.6f   |   %.6f  |  %.6f |   %.2fx   |     %.2f      |\n",
               num_threads, time_gray, time_sobel, time_total, speedup, efficiency);
        
        printf("====================================================================================================\n");
    }
    
    free(gray_seq);
    free(gray_omp);
    free(result);
    free_image(img);
}

// ==================== ИЗМЕРЕНИЯ ====================

void run_experiment(int width, int height) {
    printf("\n=== РАЗМЕР: %dx%d (%d пикселей) ===\n", width, height, width * height);
    
    image_t* img = create_test_image(width, height);
    int n_pixels = width * height;
    
    uint8_t* gray = (uint8_t*)malloc(n_pixels);
    uint8_t* result = (uint8_t*)malloc(n_pixels);
    uint8_t* gray2 = (uint8_t*)malloc(n_pixels);
    uint8_t* result2 = (uint8_t*)malloc(n_pixels);
    uint8_t* gray3 = (uint8_t*)malloc(n_pixels);
    uint8_t* result3 = (uint8_t*)malloc(n_pixels);
    
    const int RUNS = 5;
    double t_scalar = 0, t_auto = 0, t_sse = 0;
    
    // ===== СКАЛЯР =====
    printf("Измерение скалярной версии...\n");
    for (int run = 0; run < RUNS; run++) {
        double start = get_time();
        grayscale_scalar(img->pixels, gray, n_pixels);
        sobel_scalar(gray, result, width, height);
        double end = get_time();
        t_scalar += (end - start);
    }
    
    // ===== АВТОВЕКТОРИЗАЦИЯ =====
    printf("Измерение автовекторизованной версии...\n");
    for (int run = 0; run < RUNS; run++) {
        double start = get_time();
        grayscale_auto(img->pixels, gray2, n_pixels);
        sobel_auto(gray2, result2, width, height);
        double end = get_time();
        t_auto += (end - start);
    }
    
    // ===== SSE =====
    printf("Измерение SSE версии...\n");
    for (int run = 0; run < RUNS; run++) {
        double start = get_time();
        grayscale_sse(img->pixels, gray3, n_pixels);
        sobel_scalar(gray3, result3, width, height);
        double end = get_time();
        t_sse += (end - start);
    }
    
    t_scalar /= RUNS;
    t_auto /= RUNS;
    t_sse /= RUNS;
    
    printf("\n========== РЕЗУЛЬТАТЫ ==========\n");
    printf("Скаляр:  %.6f сек\n", t_scalar);
    printf("Авто:    %.6f сек (ускорение: %.2fx)\n", t_auto, t_scalar/t_auto);
    printf("SSE:     %.6f сек (ускорение: %.2fx)\n", t_sse, t_scalar/t_sse);
    printf("SSE vs Авто: %.2fx\n", t_auto/t_sse);
    
    
    free(gray);
    free(result);
    free(gray2);
    free(result2);
    free(gray3);
    free(result3);
    free_image(img);
}

// ==================== MAIN ====================

int main() {
    printf("========================================\n");
    printf("ЛАБОРАТОРНАЯ РАБОТА: Детектирование границ\n");
    printf("Сравнение: Скаляр vs Автовекторизация vs SSE vs OpenMP\n");
    printf("========================================\n\n");
    
    #ifdef __SSE__
        printf("✓ SSE поддерживается\n");
    #else
        printf("✗ SSE не поддерживается! Компилируйте с -msse\n");
    #endif
    
    #ifdef _OPENMP
        printf("✓ OpenMP поддерживается\n");
    #else
        printf("✗ OpenMP не поддерживается! Компилируйте с -fopenmp\n");
    #endif
    
    run_experiment(64, 64);
    run_experiment(256, 256);
    run_experiment(512, 512);
    run_experiment(1024, 768);
    
    test_openmp_strategies(1024, 768);
    
    return 0;
}