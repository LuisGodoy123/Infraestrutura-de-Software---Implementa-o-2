#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>

#include "mandelbrot.h"

int ConverteInteiroPositivo(const char *txt, long *saida) {
    if (txt == NULL || txt[0] == '\0') {
        return 0;
    }

    char *ptrFim;
    errno = 0;
    long valor = strtol(txt, &ptrFim, 10);

    if (errno != 0 || ptrFim == txt || *ptrFim != '\0') {
        return 0;
    }
    if (valor <= 0) {
        return 0;
    }

    *saida = valor;
    return 1;
}

int LeArgumentos(int argc, char *argv[], Config *cfg) {
    if (argc != 5) {
        fprintf(stderr,
            "Uso: %s <largura> <altura> <max_iteracoes> <num_threads>\n",
            argc > 0 ? argv[0] : "mandelbrot");
        return 0;
    }

    long largura, altura, maxIter, numThreads;

    if (!ConverteInteiroPositivo(argv[1], &largura)) {
        fprintf(stderr, "largura invalida '%s'.\n", argv[1]);
        return 0;
    }
    if (!ConverteInteiroPositivo(argv[2], &altura)) {
        fprintf(stderr, "altura invalida '%s'.\n", argv[2]);
        return 0;
    }
    if (!ConverteInteiroPositivo(argv[3], &maxIter)) {
        fprintf(stderr, "max_iteracoes invalido '%s'.\n", argv[3]);
        return 0;
    }
    if (!ConverteInteiroPositivo(argv[4], &numThreads)) {
        fprintf(stderr, "num_threads invalido '%s'.\n", argv[4]);
        return 0;
    }

    cfg->largura = (int)largura;
    cfg->altura = (int)altura;
    cfg->maxIter = (int)maxIter;
    cfg->numThreads = (int)numThreads;
    return 1;
}

void PixelParaComplexo(int col, int lin, int largura, int altura, double *re, double *im) {
    *re = REAL_MIN + (col / (double)largura) * (REAL_MAX - REAL_MIN);
    *im = IMAG_MIN + (lin / (double)altura) * (IMAG_MAX - IMAG_MIN);
}

int CalculaIteracoes(double cRe, double cIm, int maxIter) {
    double zRe = 0.0, zIm = 0.0;
    int iter = 0;

    while (zRe * zRe + zIm * zIm <= 4.0 && iter < maxIter) {
        double zReNovo = zRe * zRe - zIm * zIm + cRe;
        double zImNovo = 2.0 * zRe * zIm + cIm;
        zRe = zReNovo;
        zIm = zImNovo;
        iter++;
    }

    return iter;
}

int NormalizaIntensidade(int iter, int maxIter) {
    return (int)(255.0 * iter / maxIter);
}

unsigned char *AlocaBuffer(int largura, int altura) {
    size_t total = (size_t)largura * (size_t)altura;

    if (largura != 0 && total / (size_t)largura != (size_t)altura) {
        return NULL;
    }

    return (unsigned char *)malloc(total);
}

int SalvaImagem(const char *nomeArquivo, unsigned char *buffer, int largura, int altura) {
    FILE *arq = fopen(nomeArquivo, "w");
    if (arq == NULL) {
        return 0;
    }

    for (int lin = 0; lin < altura; lin++) {
        for (int col = 0; col < largura; col++) {
            fprintf(arq, col == largura - 1 ? "%d" : "%d ", buffer[lin * largura + col]);
        }
        fprintf(arq, "\n");
    }

    int erro = ferror(arq);
    fclose(arq);
    return erro == 0;
}

double TempoAtual(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void ExecutaSerial(Config *cfg, unsigned char *buffer) {
    for (int lin = 0; lin < cfg->altura; lin++) {
        for (int col = 0; col < cfg->largura; col++) {
            double re, im;
            PixelParaComplexo(col, lin, cfg->largura, cfg->altura, &re, &im);
            int iter = CalculaIteracoes(re, im, cfg->maxIter);
            buffer[lin * cfg->largura + col] = (unsigned char)NormalizaIntensidade(iter, cfg->maxIter);
        }
    }
}

void *TrabalhoEstatico(void *arg) {
    ArgEstatico *a = (ArgEstatico *)arg;

    for (int lin = a->linhaInicio; lin < a->linhaFim; lin++) {
        for (int col = 0; col < a->cfg->largura; col++) {
            double re, im;
            PixelParaComplexo(col, lin, a->cfg->largura, a->cfg->altura, &re, &im);
            int iter = CalculaIteracoes(re, im, a->cfg->maxIter);
            a->buffer[lin * a->cfg->largura + col] = (unsigned char)NormalizaIntensidade(iter, a->cfg->maxIter);
        }
    }

    return NULL;
}

int ExecutaPthreadsEstatico(Config *cfg, unsigned char *buffer) {
    int n = cfg->numThreads;
    pthread_t *threads = malloc((size_t)n * sizeof(pthread_t));
    ArgEstatico *args = malloc((size_t)n * sizeof(ArgEstatico));

    if (threads == NULL || args == NULL) {
        free(threads);
        free(args);
        return 0;
    }

    int linhasPorThread = cfg->altura / n;
    int resto = cfg->altura % n;
    int linhaAtual = 0;

    for (int i = 0; i < n; i++) {
        int linhas = linhasPorThread + (i < resto ? 1 : 0);
        args[i].cfg = cfg;
        args[i].buffer = buffer;
        args[i].linhaInicio = linhaAtual;
        args[i].linhaFim = linhaAtual + linhas;
        linhaAtual += linhas;

        if (pthread_create(&threads[i], NULL, TrabalhoEstatico, &args[i]) != 0) {
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(args);
            return 0;
        }
    }

    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);
    return 1;
}

void *TrabalhoDinamico(void *arg) {
    ArgDinamico *a = (ArgDinamico *)arg;

    while (1) {
        pthread_mutex_lock(&a->mutex);
        int lin = a->proximaLinha;
        if (lin < a->cfg->altura) {
            a->proximaLinha++;
        }
        pthread_mutex_unlock(&a->mutex);

        if (lin >= a->cfg->altura) {
            break;
        }

        for (int col = 0; col < a->cfg->largura; col++) {
            double re, im;
            PixelParaComplexo(col, lin, a->cfg->largura, a->cfg->altura, &re, &im);
            int iter = CalculaIteracoes(re, im, a->cfg->maxIter);
            a->buffer[lin * a->cfg->largura + col] = (unsigned char)NormalizaIntensidade(iter, a->cfg->maxIter);
        }
    }

    return NULL;
}

int ExecutaPthreadsDinamico(Config *cfg, unsigned char *buffer) {
    int n = cfg->numThreads;
    pthread_t *threads = malloc((size_t)n * sizeof(pthread_t));

    if (threads == NULL) {
        return 0;
    }

    ArgDinamico a;
    a.cfg = cfg;
    a.buffer = buffer;
    a.proximaLinha = 0;

    if (pthread_mutex_init(&a.mutex, NULL) != 0) {
        free(threads);
        return 0;
    }

    int criadas = 0;
    int falhou = 0;

    for (int i = 0; i < n; i++) {
        if (pthread_create(&threads[i], NULL, TrabalhoDinamico, &a) != 0) {
            falhou = 1;
            break;
        }
        criadas++;
    }

    for (int i = 0; i < criadas; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&a.mutex);
    free(threads);

    return !falhou;
}

void ExecutaOpenMP(Config *cfg, unsigned char *buffer) {
    #pragma omp parallel for num_threads(cfg->numThreads) schedule(dynamic)
    for (int lin = 0; lin < cfg->altura; lin++) {
        for (int col = 0; col < cfg->largura; col++) {
            double re, im;
            PixelParaComplexo(col, lin, cfg->largura, cfg->altura, &re, &im);
            int iter = CalculaIteracoes(re, im, cfg->maxIter);
            buffer[lin * cfg->largura + col] = (unsigned char)NormalizaIntensidade(iter, cfg->maxIter);
        }
    }
}

int SalvaTempo(const char *nomeArquivo, const char *rotulo, double segundos) {
    FILE *arq = fopen(nomeArquivo, "a");
    if (arq == NULL) {
        return 0;
    }

    fprintf(arq, "%s: %.6f segundos\n", rotulo, segundos);

    int erro = ferror(arq);
    fclose(arq);
    return erro == 0;
}
