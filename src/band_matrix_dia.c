#include "../include/band_matrix_dia.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void inicializa_matriz_dia(struct matriz_banda_dia *m, int n, int mb) {
	m->n = n;
	m->mb = mb;
	m->bw = 2 * mb + 1;
	m->diagonais = (double **)malloc(m->bw * sizeof(double *));
	for (int k = 0; k < m->bw; k++) {
		m->diagonais[k] = (double *)malloc(n * sizeof(double));
		// Inicializa com zero (útil para bordas não preenchidas)
		for (int i = 0; i < n; i++)
			m->diagonais[k][i] = 0.0;
	}
}

void libera_matriz_dia(struct matriz_banda_dia *m) {
	if (!m->diagonais)
		return;
	for (int k = 0; k < m->bw; k++)
		free(m->diagonais[k]);
	free(m->diagonais);
	m->diagonais = NULL;
}

void preencher_diagonal_spd_dia(struct matriz_banda_dia *m, unsigned int seed_diag) {
	int n = m->n;
	int mb = m->mb;
	srand(seed_diag);
	for (int i = 0; i < n; i++) {
		double soma = 0.0;
		// Soma dos módulos dos elementos fora da diagonal na linha i
		for (int d = -mb; d <= mb; d++) {
			if (d == 0)
				continue;
			int j = i + d;
			if (j < 0 || j >= n)
				continue;
			int k = d + mb;
			soma += fabs(m->diagonais[k][i]);
		}
		double extra = rand() / (double)RAND_MAX + 1.0; // [1,2)
		int k_diag = 0 + mb; // índice da diagonal principal
		m->diagonais[k_diag][i] = soma + extra;
	}
}

// Preenche a matriz com valores aleatórios e garante SPD (dominância diagonal)
void preenche_matriz_dia(struct matriz_banda_dia *m, unsigned int seed_mat,
                         unsigned int seed_diag) {
	int n = m->n;
	int mb = m->mb;

	// 1. Preencher elementos fora da diagonal (simétricos)
	srand(seed_mat);
	for (int i = 0; i < n; i++) {
		for (int d = 1; d <= mb; d++) {  // apenas deslocamentos positivos
			int j = i + d;
			if (j >= n)
				continue;
			double valor = rand() / (double)RAND_MAX;

			// Diagonal superior (d)
			int k_sup = d + mb;
			m->diagonais[k_sup][i] = valor;

			// Diagonal inferior (-d) - espelho para simetria
			int k_inf = -d + mb;
			m->diagonais[k_inf][j] = valor;
		}
	}

	// 2. Diagonal principal com dominância
	preencher_diagonal_spd_dia(m, seed_diag);
}

// Produto matriz-vetor: y = A * x
double *matriz_banda_vetor_dia(const struct matriz_banda_dia *a,
                               const double *x) {
	if (!a || !x)
		return NULL;
	int n = a->n;
	int mb = a->mb;
	int bw = a->bw;
	double *y = (double *)calloc(n, sizeof(double)); // inicializa com 0
	if (!y)
		return NULL;

	// Para cada diagonal (deslocamento d)
	for (int k = 0; k < bw; k++) {
		int d = k - mb; // deslocamento da diagonal
		double *diag = a->diagonais[k];
		if (d == 0) {
			// Diagonal principal: y[i] += diag[i] * x[i]
			for (int i = 0; i < n; i++) {
				y[i] += diag[i] * x[i];
			}
		} else if (d > 0) {
			// Diagonal superior: y[i] += diag[i] * x[i+d], para i de 0 até
			// n-d-1
			for (int i = 0; i < n - d; i++) {
				y[i] += diag[i] * x[i + d];
			}
		} else { // d < 0
			// Diagonal inferior: y[i] += diag[i] * x[i+d], para i de -d até n-1
			// Note: para d negativo, i+d é menor que i.
			// Podemos percorrer i de -d até n-1:
			int start = -d;
			for (int i = start; i < n; i++) {
				y[i] += diag[i] * x[i + d];
			}
		}
	}
	return y;
}