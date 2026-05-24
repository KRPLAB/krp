#include "../include/band_matrix.h"
#include "../include/common.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void inicializa_matriz(struct matriz_banda *m, int linhas, int meia_banda) {
	m->linhas = linhas;
	m->meia_banda = meia_banda;
	int total_bandas = (meia_banda * 2) + 1;
	m->dados = (double *)malloc(linhas * total_bandas * sizeof(double));
}

void libera_matriz(struct matriz_banda *m) {
	free(m->dados);
	m->dados = NULL;
}

void imprime_matriz(const struct matriz_banda *m) {
	if (!m || !m->dados) {
		printf("Matriz não inicializada.\n");
		return;
	}

	for (int i = 0; i < m->linhas; i++) {
		for (int j = 0; j < m->linhas; j++) {
			int idx = obter_indice_1d(i, j, m->meia_banda);
			if (idx != -1) {
				printf("%6.5f ", m->dados[idx]);
			} else {
				printf("%6.5f ", 0.0);
			}
		}
		printf("\n");
	}
}

int obter_indice_1d(int i, int j, int meia_banda) {
	if (abs(i - j) > meia_banda) {
		return -1;
	}
	return i * (meia_banda * 2 + 1) + (j - i + meia_banda);
}

void preenche_matriz(struct matriz_banda *m, unsigned int seed_mat,
                     unsigned int seed_diag) {
	if (!m || !m->dados)
		return;

	// Preenche elementos fora da diagonal (simétrico)
	srand(seed_mat);
	for (int i = 0; i < m->linhas; i++) {
		for (int j = MAX(0, i - m->meia_banda); j < i; j++) {
			int idx1 = obter_indice_1d(i, j, m->meia_banda);
			int idx2 = obter_indice_1d(j, i, m->meia_banda);
			double valor = rand() / (double)RAND_MAX;
			m->dados[idx1] = valor;
			m->dados[idx2] = valor;
		}
	}
	preencher_diagonal_spd(m, seed_diag);
}

void preencher_diagonal_spd(struct matriz_banda *m, unsigned int semente) {
	if (!m || !m->dados)
		return;
	srand(semente);
	for (int i = 0; i < m->linhas; i++) {
		double soma = 0.0;
		int j_min = MAX(0, i - m->meia_banda);
		int j_max = MIN(m->linhas - 1, i + m->meia_banda);
		for (int j = j_min; j <= j_max; j++) {
			if (j == i)
				continue;
			int idx = obter_indice_1d(i, j, m->meia_banda);
			if (idx != -1)
				soma += fabs(m->dados[idx]);
		}
		/* extra em [1, 2): garante diagonal > soma mesmo quando soma == 0 */
		double extra = rand() / (double)RAND_MAX + 1.0;
		int idx_diag = obter_indice_1d(i, i, m->meia_banda);
		m->dados[idx_diag] = soma + extra;
	}
}

double *matriz_banda_vetor(const struct matriz_banda *restrict a, const double *restrict x) {
	if (!a || !x)
		return NULL;

	int n = a->linhas;
	int mb = a->meia_banda;
	int bw = 2 * mb + 1; // largura total da banda (número de diagonais)

	double *res = (double *)malloc(n * sizeof(double));
	if (!res)
		return NULL;

	for (int i = 0; i < n; i++) {
		double soma = 0.0;
		int base = i * bw; // início da linha i no vetor 1D

		// Desenrolamento em blocos de 4 diagonais
		int k;
		for (k = 0; k + 3 < bw; k += 4) {
			// Calcula os índices das colunas para cada diagonal
			int j0 = i + ((k + 0) - mb);
			int j1 = i + ((k + 1) - mb);
			int j2 = i + ((k + 2) - mb);
			int j3 = i + ((k + 3) - mb);

			double v0 =
			    (j0 >= 0 && j0 < n) ? a->dados[base + k + 0] * x[j0] : 0.0;
			double v1 =
			    (j1 >= 0 && j1 < n) ? a->dados[base + k + 1] * x[j1] : 0.0;
			double v2 =
			    (j2 >= 0 && j2 < n) ? a->dados[base + k + 2] * x[j2] : 0.0;
			double v3 =
			    (j3 >= 0 && j3 < n) ? a->dados[base + k + 3] * x[j3] : 0.0;

			soma += v0 + v1 + v2 + v3;
		}

		// Laço residual para as diagonais restantes (menos de 4)
		for (; k < bw; k++) {
			int j = i + (k - mb);
			if (j >= 0 && j < n) {
				soma += a->dados[base + k] * x[j];
			}
		}

		res[i] = soma;
	}
	return res;
}