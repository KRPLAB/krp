#include "../include/band_matrix_dia.h"
#include <stdio.h>
#include <stdlib.h>

// Protótipo da função AVX de produto matriz-vetor
double *matriz_banda_vetor_dia_avx(const struct matriz_banda_dia *a, const double *x);

// Versão AVX2 da matriz banda diagonal
static double dot_product_avx(const double *a, const double *b, int n) {
	__m256d sum_vec = _mm256_setzero_pd();
	int i = 0;
	for (; i + 3 < n; i += 4) {
		__m256d av = _mm256_load_pd(&a[i]);
		__m256d bv = _mm256_load_pd(&b[i]);
		sum_vec = _mm256_fmadd_pd(av, bv, sum_vec); // sum_vec += av * bv
	}
	// Reduzir o vetor de 4 doubles para um único double
	double sum[4];
	_mm256_store_pd(sum, sum_vec);
	double total = sum[0] + sum[1] + sum[2] + sum[3];
	for (; i < n; i++)
		total += a[i] * b[i];
	return total;
}

// Passo do CG (recebe matriz_banda_dia)
double *gradiente_conjugado_dia(const struct matriz_banda_dia *a,
                                const double *x, double *r, const double *d) {
	double *z = matriz_banda_vetor_dia_avx(a, d);
	if (!z)
		return NULL;

	double r_dot = dot_product_avx(r, r, a->n);
	double d_dot_z = dot_product_avx(d, z, a->n);
	if (d_dot_z == 0.0) {
		free(z);
		return NULL;
	}
	double alpha = r_dot / d_dot_z;

	double *x_new = malloc(a->n * sizeof(double));
	if (!x_new) {
		free(z);
		return NULL;
	}
	for (int i = 0; i < a->n; i++)
		x_new[i] = x[i] + alpha * d[i];

	for (int i = 0; i < a->n; i++)
		r[i] -= alpha * z[i];

	free(z);
	return x_new;
}

void update_d_avx(double *d, const double *r, double beta, int n) {
	__m256d beta_vec = _mm256_set1_pd(beta); // espalha beta nos 4 slots
	int i = 0;
	for (; i + 3 < n; i += 4) {
		__m256d dv = _mm256_load_pd(&d[i]);
		__m256d rv = _mm256_load_pd(&r[i]);
		__m256d prod = _mm256_mul_pd(beta_vec, dv);
		__m256d new_d = _mm256_add_pd(rv, prod);
		_mm256_store_pd(&d[i], new_d);
	}
	for (; i < n; i++) {
		d[i] = r[i] + beta * d[i];
	}
}

// Resolvedor CG principal
int cg_solve_dia(const struct matriz_banda_dia *a, const double *b, double *x,
                 double tol, int max_iter) {
	int n = a->n;
	int iter = 0;

	double *Ax = matriz_banda_vetor_dia_avx(a, x);
	if (!Ax)
		return -1;

	double *r = malloc(n * sizeof(double));
	double *d = malloc(n * sizeof(double));
	if (!r || !d) {
		free(Ax);
		free(r);
		free(d);
		return -1;
	}
	for (int i = 0; i < n; i++) {
		r[i] = b[i] - Ax[i];
		d[i] = r[i];
	}
	free(Ax);

	double aux = dot_product_avx(r, r, n);
	double tol2 = tol * tol;

	while (aux > tol2 && iter < max_iter) {
		double r_dot_old = aux;

		double *x_new = gradiente_conjugado_dia(a, x, r, d);
		if (!x_new) {
			free(r);
			free(d);
			return -1;
		}
		for (int i = 0; i < n; i++)
			x[i] = x_new[i];
		free(x_new);

		aux = dot_product_avx(r, r, n);
		iter++;

		if (aux <= tol2)
			break;

		double beta = aux / r_dot_old;
		update_d_avx(d, r, beta, n);
	}

	free(r);
	free(d);
	return iter;
}