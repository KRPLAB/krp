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

void preenche_matriz(struct matriz_banda *m, unsigned int seed_mat, unsigned int seed_diag) {
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

double *matriz_banda_vetor(const struct matriz_banda *a, const double *x) {
	if (!a || !x)
		return NULL;

	int n = a->linhas;
	int mb = a->meia_banda;

	double *res = (double *)malloc(n * sizeof(double));
	if (!res)
		return NULL;

	for (int i = 0; i < n; i++) {
		double soma = 0.0;
		int j_min = MAX(0, i - mb);
		int j_max = MIN(n - 1, i + mb);
		// percorre apenas os índices dentro da banda
		for (int j = j_min; j <= j_max; j++) {
			int idx = obter_indice_1d(i, j, mb);
			soma += a->dados[idx] * x[j];
		}
		res[i] = soma;
	}
	return res;
}