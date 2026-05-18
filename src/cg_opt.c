#include "../include/band_matrix.h"
#include <stdlib.h>

/* ---------- Funções auxiliares (produto escalar, norma) ---------- */
static double dot_product(const double *a, const double *b, int n) {
	double s = 0.0;
	for (int i = 0; i < n; i++)
		s += a[i] * b[i];
	return s;
}

/* ---------- Passo do Gradiente Conjugado ---------- */
/**
 * Realiza um passo do CG:
 *   z = A * d
 *   alpha = (r·r) / (d·z)
 *   x_new = x + alpha * d
 *   r (in-place) = r - alpha * z
 *
 * Retorna novo x (deve ser liberado pelo chamador).
 */
double *gradiente_conjugado(const struct matriz_banda *A, const double *x,
                            double *r, const double *d) {
	// z = A * d
	double *z = matriz_banda_vetor(A, d);
	if (!z)
		return NULL;

	double r_dot = dot_product(r, r, A->linhas);
	double d_dot_z = dot_product(d, z, A->linhas);
	if (d_dot_z == 0.0) {
		free(z);
		return NULL;
	}
	double alpha = r_dot / d_dot_z;

	// x_new = x + alpha * d
	double *x_new = malloc(A->linhas * sizeof(double));
	if (!x_new) {
		free(z);
		return NULL;
	}
	for (int i = 0; i < A->linhas; i++)
		x_new[i] = x[i] + alpha * d[i];

	// r = r - alpha * z  (atualização in-place)
	for (int i = 0; i < A->linhas; i++)
		r[i] -= alpha * z[i];

	free(z);
	return x_new;
}

/* ---------- Atualização da direção conjugada ---------- */
/**
 * d = r + beta * d  (in-place)
 */
void update_d(double *d, const double *r, double beta, int n) {
	for (int i = 0; i < n; i++)
		d[i] = r[i] + beta * d[i];
}

/* ---------- Algoritmo completo para resolver Ax = b ---------- */
/**
 * Resolve Ax = b com o Método dos Gradientes Conjugados.
 * O vetor x deve vir alocado com o chute inicial e será sobrescrito com a
 * solução. Retorna o número de iterações realizadas.
 */
int cg_solve(const struct matriz_banda *A, const double *b, double *x,
             double tol, int max_iter) {
	int n = A->linhas;
	int iter = 0;

	// r0 = b - A*x0
	double *Ax = matriz_banda_vetor(A, x);
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

	double aux = dot_product(r, r, n);
	double tol2 = tol * tol;

	while (aux > tol2 && iter < max_iter) {
		double r_dot_old = aux;

		double *x_new = gradiente_conjugado(A, x, r, d);
		if (!x_new) {
			free(r);
			free(d);
			return -1;
		}

		// x = x_new (podemos só copiar os valores de volta)
		for (int i = 0; i < n; i++)
			x[i] = x_new[i];
		free(x_new);

		aux = dot_product(r, r, n);
		iter++;

		if (aux <= tol2)
			break;

		double beta = aux / r_dot_old;
		update_d(d, r, beta, n);
	}

	free(r);
	free(d);
	return iter;
}
