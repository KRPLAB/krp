#include "../include/band_matrix_dia.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void inicializa_matriz_dia_avx(struct matriz_banda_dia *m, int n, int mb) {
	m->n = n;
	m->mb = mb;
	m->bw = 2 * mb + 1;
	m->diagonais = (double **)malloc(m->bw * sizeof(double *));
	for (int k = 0; k < m->bw; k++) {
		// Aloca memória alinhada a 32 bytes (múltiplo de 32)
		int ret = posix_memalign((void **)&m->diagonais[k], 32, n * sizeof(double));
		if (ret != 0) {
			fprintf(stderr, "Erro de alocação para diagonal %d\n", k);
			exit(EXIT_FAILURE);
		}
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
		for (int d = 1; d <= mb; d++) {
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
double *matriz_banda_vetor_dia_avx(const struct matriz_banda_dia *a, const double *x) {
	if (!a || !x)
		return NULL;
	int n = a->n;
	int mb = a->mb;
	int bw = a->bw;

	// alinhado a 32
	double *y = (double *)aligned_alloc(32, n * sizeof(double));

	if (!y)
		return NULL;
	// Para cada diagonal
	for (int k = 0; k < bw; k++) {
		int d = k - mb; // deslocamento
		double *diag = a->diagonais[k];

		if (d == 0) {
			// Diagonal principal: y[i] += diag[i] * x[i]
			// Processamos 4 i por vez
			int i = 0;
			for (; i + 3 < n; i += 4) {
				__m256d dx = _mm256_load_pd(&x[i]);			// x[i..i+3]
				__m256d dd = _mm256_load_pd(&diag[i]);		// diag[i..i+3]
				__m256d prod = _mm256_mul_pd(dd, dx);		// produto
				__m256d yv = _mm256_load_pd(&y[i]);			// y[i..i+3] atual
				yv = _mm256_add_pd(yv, prod);				// soma
				_mm256_store_pd(&y[i], yv);
			}
			// Resíduo (i < 4)
			for (; i < n; i++) {
				y[i] += diag[i] * x[i];
			}
		} else if (d > 0) {
			// Diagonal superior: y[i] += diag[i] * x[i+d], para i de 0 até
			// n-d-1
			int limit = n - d;
			int i = 0;
			for (; i + 3 < limit; i += 4) {
				// x[i+d .. i+d+3]
				__m256d xv = _mm256_load_pd(&x[i + d]);
				__m256d dv = _mm256_load_pd(&diag[i]);
				__m256d prod = _mm256_mul_pd(dv, xv);
				__m256d yv = _mm256_load_pd(&y[i]);
				yv = _mm256_add_pd(yv, prod);
				_mm256_store_pd(&y[i], yv);
			}
			for (; i < limit; i++) {
				y[i] += diag[i] * x[i + d];
			}
		} else { // d < 0
			// Diagonal inferior: y[i] += diag[i] * x[i+d], para i de -d até n-1
			int start = -d;
			int i = start;
			// Como i+d pode ser menor que i, mas os índices são sequenciais a
			// partir de start. Processamos blocos de 4.
			for (; i + 3 < n; i += 4) {
				// Carregar x[i+d] .. x[i+d+3]. Note que i+d é contínuo.
				__m256d xv = _mm256_load_pd(&x[i + d]);
				__m256d dv = _mm256_load_pd(&diag[i]);
				__m256d prod = _mm256_mul_pd(dv, xv);
				__m256d yv = _mm256_load_pd(&y[i]);
				yv = _mm256_add_pd(yv, prod);
				_mm256_store_pd(&y[i], yv);
			}
			for (; i < n; i++) {
				y[i] += diag[i] * x[i + d];
			}
		}
	}
	return y;
}