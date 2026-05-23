#include "../include/band_matrix.h"
#include "../include/timer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Protótipos das funções do CG (definidas em cg_naive.c ou cg_opt.c)
int cg_solve(const struct matriz_banda *a, const double *b, double *x,
             double tol, int max_iter);

int main(int argc, char *argv[]) {
	if (argc < 4) {
		fprintf(
		    stderr,
		    "Uso: %s <tamanho> <bandas> <semente_matriz> [semente_diagonal]\n",
		    argv[0]);
		return 1;
	}

	int tam = atoi(argv[1]);
	int bandas = atoi(argv[2]);
	unsigned int seed_mat = (unsigned int)atoi(argv[3]);
	unsigned int seed_diag = (argc >= 5) ? (unsigned int)atoi(argv[4]) : 54321u;

	if (tam <= 0 || bandas <= 0) {
		fprintf(stderr, "Erro: tamanho e bandas devem ser positivos.\n");
		return 1;
	}

	int meia_banda = (bandas - 1) / 2;
	struct matriz_banda a;
	inicializa_matriz(&a, tam, meia_banda);
	preenche_matriz(&a, seed_mat, seed_diag);

	// Gera um vetor b = A * x_true, com x_true = (1,1,...,1)
	double *x_true = malloc(tam * sizeof(double));
	if (!x_true) {
		fprintf(stderr, "Erro de alocação para x_true.\n");
		libera_matriz(&a);
		return 1;
	}
	for (int i = 0; i < tam; i++)
		x_true[i] = 1.0;
	double *b = matriz_banda_vetor(&a, x_true);

	if (!b) {
		fprintf(stderr, "Erro de alocação para b.\n");
		free(x_true);
		libera_matriz(&a);
		return 1;
	}

	// Chute inicial x0 = (0,0,...,0)
	double *x = calloc(tam, sizeof(double));
	if (!x) {
		fprintf(stderr, "Erro de alocação para x.\n");
		free(x_true);
		free(b);
		libera_matriz(&a);
		return 1;
	}

	// Parâmetros do CG
	double tol = 1e-8;
	int max_iter = 2 * tam;

	// Inicia medição de tempo
	double t_inicio = get_time_sec();
	int iters = cg_solve(&a, b, x, tol, max_iter);
	double t_fim = get_time_sec();
	double tempo = t_fim - t_inicio;

	if (iters < 0) {
		fprintf(stderr, "Erro durante o CG.\n");
		free(x_true);
		free(b);
		free(x);
		libera_matriz(&a);
		return 1;
	}

	// Calcula erro relativo
	double *Ax_final = matriz_banda_vetor(&a, x);
	double erro = 0.0, norma_b = 0.0;
	for (int i = 0; i < tam; i++) {
		double diff = Ax_final[i] - b[i];
		erro += diff * diff;
		norma_b += b[i] * b[i];
	}
	free(Ax_final);
	double erro_relativo = sqrt(erro) / sqrt(norma_b);

	// Saída em formato CSV: tam,bandas,seed,iteracoes,tempo,erro
	printf("%d,%d,%u,%d,%.6e,%.6e\n", tam, bandas, seed_mat, iters, tempo,
	       erro_relativo);

	free(x_true);
	free(b);
	free(x);
	libera_matriz(&a);
	return 0;
}