#ifndef MANDELBROT_H
#define MANDELBROT_H

#define LOGIN "lagi"

#define REAL_MIN -2.0
#define REAL_MAX  1.0
#define IMAG_MIN -1.5
#define IMAG_MAX  1.5

typedef struct {
    int largura;
    int altura;
    int maxIter;
    int numThreads;
} Config;

int ConverteInteiroPositivo(const char *txt, long *saida);
int LeArgumentos(int argc, char *argv[], Config *cfg);

#endif
