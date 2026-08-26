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

void PixelParaComplexo(int col, int lin, int largura, int altura, double *re, double *im);
int CalculaIteracoes(double cRe, double cIm, int maxIter);

int NormalizaIntensidade(int iter, int maxIter);
unsigned char *AlocaBuffer(int largura, int altura);

int SalvaImagem(const char *nomeArquivo, unsigned char *buffer, int largura, int altura);
int SalvaTempo(const char *nomeArquivo, const char *rotulo, double segundos);

double TempoAtual(void);
void ExecutaSerial(Config *cfg, unsigned char *buffer);
void ExecutaOpenMP(Config *cfg, unsigned char *buffer);

#endif
