# Infraestrutura de Software - Implementação 2

Gerador do conjunto de Mandelbrot em C, com quatro implementações: serial, OpenMP,
Pthreads com particao estatica em blocos e Pthreads com fila dinamica protegida por mutex.

Login: lagi

## Compilar

make

## Limpar

make clean

## Executar

./mandelbrot <largura> <altura> <max_iteracoes> <num_threads>

Exemplo:

./mandelbrot 800 600 1000 4

## Saida

- mandelbrot_lagi_serial.pgm
- mandelbrot_lagi_openmp.pgm
- mandelbrot_lagi_pthreads1.pgm
- mandelbrot_lagi_pthreads2.pgm
- times.txt

Cada .pgm contem os valores de intensidade dos pixels (0-255), um por pixel,
separados por espaco, uma linha por linha da imagem. Os quatro arquivos sao
identicos entre si. O times.txt registra o tempo de calculo de cada implementacao.
