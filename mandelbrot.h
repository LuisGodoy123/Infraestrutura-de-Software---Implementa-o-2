#ifndef MANDELBROT_H
#define MANDELBROT_H

#define LOGIN "lagi"

/* Regiao do plano complexo representada pela imagem. */
#define REAL_MIN -2.0
#define REAL_MAX  1.0
#define IMAG_MIN -1.5
#define IMAG_MAX  1.5

typedef struct {
    int largura;
    int altura;
    int max_iteracoes;
    int num_threads;
} Config;

int parse_positive_int(const char *str, long *out);
int parse_args(int argc, char *argv[], Config *cfg);

#endif
