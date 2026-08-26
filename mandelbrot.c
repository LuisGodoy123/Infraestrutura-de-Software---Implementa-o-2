#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
        fprintf(stderr, "Erro: largura invalida '%s'.\n", argv[1]);
        return 0;
    }
    if (!ConverteInteiroPositivo(argv[2], &altura)) {
        fprintf(stderr, "Erro: altura invalida '%s'.\n", argv[2]);
        return 0;
    }
    if (!ConverteInteiroPositivo(argv[3], &maxIter)) {
        fprintf(stderr, "Erro: max_iteracoes invalido '%s'.\n", argv[3]);
        return 0;
    }
    if (!ConverteInteiroPositivo(argv[4], &numThreads)) {
        fprintf(stderr, "Erro: num_threads invalido '%s'.\n", argv[4]);
        return 0;
    }

    cfg->largura = (int)largura;
    cfg->altura = (int)altura;
    cfg->maxIter = (int)maxIter;
    cfg->numThreads = (int)numThreads;
    return 1;
}
