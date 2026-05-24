#include "../include/band_matrix_dia.h"
#include "../include/timer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Protótipo do solver DIA
int cg_solve_dia(const struct matriz_banda_dia *a, const double *b, double *x,
                 double tol, int max_iter);

int main(int argc, char *argv[]) {
    // mesma leitura de argumentos...
    int tam = atoi(argv[1]);
    int bandas = atoi(argv[2]);
    unsigned int seed_mat = (unsigned int)atoi(argv[3]);
    unsigned int seed_diag = (argc >= 5) ? (unsigned int)atoi(argv[4]) : 54321u;

    int meia_banda = (bandas - 1) / 2;
    struct matriz_banda_dia a;
    inicializa_matriz_dia(&a, tam, meia_banda);
    preenche_matriz_dia(&a, seed_mat, seed_diag);

    // Gera b = A * x_true (x_true = 1)
    double *x_true = malloc(tam * sizeof(double));
    for (int i = 0; i < tam; i++) x_true[i] = 1.0;
    double *b = matriz_banda_vetor_dia(&a, x_true);
    double *x = calloc(tam, sizeof(double));

    double t_inicio = get_time_sec();
    int iters = cg_solve_dia(&a, b, x, 1e-8, 2*tam);
    double t_fim = get_time_sec();

    // Erro relativo...
    double *Ax_final = matriz_banda_vetor_dia(&a, x);
    double erro = 0.0, norma_b = 0.0;
    for (int i = 0; i < tam; i++) {
        double diff = Ax_final[i] - b[i];
        erro += diff*diff;
        norma_b += b[i]*b[i];
    }
    double erro_relativo = sqrt(erro)/sqrt(norma_b);

    printf("%d,%d,%u,%d,%.6e,%.6e\n", tam, bandas, seed_mat, iters, t_fim - t_inicio, erro_relativo);

    free(x_true); free(b); free(x); free(Ax_final);
    libera_matriz_dia(&a);
    return 0;
}