#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double **le_matriz(int m, int n) {
	if (m <= 0 || n <= 0)
		return NULL;

	double **mat = malloc(m * sizeof(double *));
	if (!mat)
		return NULL;

	for (int i = 0; i < m; i++) {
		mat[i] = malloc(n * sizeof(double));
		if (!mat[i]) {
			for (int k = 0; k < i; k++)
				free(mat[k]);
			free(mat);
			return NULL;
		}
		for (int j = 0; j < n; j++) {
			if (scanf("%lf", &mat[i][j]) != 1) {
				mat[i][j] = 0.0;
			}
		}
	}
	return mat;
}

void libera_matriz(double **mat, int m) {
	if (!mat)
		return;
	for (int i = 0; i < m; i++)
		free(mat[i]);
	free(mat);
}

void imprime_matriz(double **matriz, int m, int n) {
	if (!matriz || m <= 0 || n <= 0)
		return;

	for (int i = 0; i < m; i++) {
		if (!matriz[i])
			continue;
		for (int j = 0; j < n; j++) {
			printf("%f ", matriz[i][j]);
		}
		printf("\n");
	}
}

double dot_product(double *a, int m, double *b) {
	double dot = 0;
	for (int i = 0; i < m; i++) {
		dot += a[i] * b[i];
	}
	return dot;
}

double *matriz_vetor(double **matriz, int m, double *r) {
	if (!matriz || !r || m <= 0)
		return NULL;

	double *resultado = malloc(m * sizeof(double));
	if (!resultado)
		return NULL;

	for (int i = 0; i < m; i++) {
		if (!matriz[i]) {
			free(resultado);
			return NULL;
		}
		resultado[i] = dot_product(matriz[i], m, r);
	}
	return resultado;
}

double norma(double *r, int m) {
	double ri, norm = 0;
	for (int i = 0; i < m; i++) {
		ri = fabs(r[i]);
		norm = (ri > norm) ? ri : norm;
	}
	return norm;
}

/**
 * Realiza um passo do Gradiente Conjugado:
 * Atualiza x e o resíduo r (in-place), usando a direção d.
 *
 * Parâmetros:
 *   a  : matriz do sistema (m x m, apenas as primeiras m colunas são usadas)
 *   m  : dimensão
 *   x  : vetor solução atual (não modificado)
 *   r  : resíduo atual (b - A*x). Será atualizado in-place para o novo resíduo.
 *   d  : direção conjugada atual (não modificada)
 *
 * Retorna:
 *   Novo vetor x (x_{k+1} = x_k + alpha * d). O chamador deve liberá-lo.
 */
double *gradiente_conjugado(double **a, int m, double *x, double *r,
                            double *d) {
	// z = A * d
	double *z = matriz_vetor(a, m, d);
	if (!z)
		return NULL;

	// alpha = (r^T r) / (d^T z)
	double r_dot = dot_product(r, m, r);
	double d_dot_z = dot_product(d, m, z);
	if (d_dot_z == 0.0) {
		free(z);
		return NULL;
	}
	double alpha = r_dot / d_dot_z;

	// x_new = x + alpha * d
	double *x_new = malloc(m * sizeof(double));
	if (!x_new) {
		free(z);
		return NULL;
	}
	for (int i = 0; i < m; i++)
		x_new[i] = x[i] + alpha * d[i];

	// r_new = r - alpha * z  (atualiza in-place)
	for (int i = 0; i < m; i++)
		r[i] -= alpha * z[i];

	free(z);
	return x_new;
}

/**
 * Atualiza a direção conjugada d no CG:
 * d_{k+1} = r_{k+1} + beta * d_k
 *
 * Parâmetros:
 *   d    : vetor de direção (atualizado in-place)
 *   m    : dimensão
 *   r    : novo resíduo (r_{k+1})
 *   beta : coeficiente de conjugação (já calculado)
 */
void update_d(double *d, int m, double *r, double beta) {
	for (int i = 0; i < m; i++)
		d[i] = r[i] + beta * d[i];
}

int main(int argc, char **argv) {
	int m, n, i, iterations;
	double **a;
	double *x0, *Ax, *r0, *d0;
	double *x_new;
	double aux, aux_new, beta;

	const double tolerance = 1e-8;
	const int max_iter = 20;

	if (argc < 3) {
		fprintf(stderr, "Uso: %s <linhas> <colunas>\n", argv[0]);
		return 1;
	}

	m = atoi(argv[1]);
	n = atoi(argv[2]);

	if (n != m + 1) {
		fprintf(stderr, "Erro: a matriz deve ser aumentada (número de colunas "
		                "= linhas + 1)\n");
		return 1;
	}

	a = le_matriz(m, n);
	if (!a) {
		fprintf(stderr, "Erro ao ler a matriz\n");
		return 1;
	}

	// Leitura do vetor inicial x0
	x0 = malloc(m * sizeof(double));
	if (!x0) {
		fprintf(stderr, "Erro de alocação\n");
		libera_matriz(a, m);
		return 1;
	}
	for (i = 0; i < m; i++) {
		if (scanf("%lf", &x0[i]) != 1) {
			fprintf(stderr, "Erro na leitura de x0\n");
			free(x0);
			libera_matriz(a, m);
			return 1;
		}
	}

	// Resíduo inicial r0 = b - A*x0
	Ax = matriz_vetor(a, m, x0);
	if (!Ax) {
		free(x0);
		libera_matriz(a, m);
		return 1;
	}
	r0 = malloc(m * sizeof(double));
	if (!r0) {
		free(x0);
		free(Ax);
		libera_matriz(a, m);
		return 1;
	}
	for (i = 0; i < m; i++)
		r0[i] = a[i][m] - Ax[i];
	free(Ax);

	// Inicialização da direção conjugada: d0 = r0
	d0 = malloc(m * sizeof(double));
	if (!d0) {
		free(x0);
		free(r0);
		libera_matriz(a, m);
		return 1;
	}
	for (i = 0; i < m; i++)
		d0[i] = r0[i];

	// aux = (r0)^T r0  (será usado no cálculo de alpha e beta)
	aux = dot_product(r0, m, r0);

	iterations = 0;

	while (aux > tolerance * tolerance && iterations < max_iter) {
		// Guarda o valor de r^T r antes da atualização
		double r_dot_old = aux;

		x_new = gradiente_conjugado(a, m, x0, r0, d0);
		if (!x_new) {
			fprintf(stderr,
			        "Falha no gradiente conjugado (divisão por zero?)\n");
			break;
		}

		free(x0);
		x0 = x_new;

		// Calcula o novo aux = (r_{k+1})^T r_{k+1}
		aux_new = dot_product(r0, m, r0);

		iterations++;

		printf("Iteração %d:\n", iterations);
		for (i = 0; i < m; i++)
			printf("x[%d] = %.6f\n", i, x0[i]);
		printf("Norma do resíduo: %g\n\n", sqrt(aux_new));

		if (aux_new < tolerance * tolerance)
			break;

		// beta = (r_{k+1}^T r_{k+1}) / (r_k^T r_k)
		beta = aux_new / r_dot_old;

		// Atualiza a direção conjugada: d = r + beta * d
		update_d(d0, m, r0, beta);

		// Atualiza aux para a próxima iteração
		aux = aux_new;
	}

	printf("Finalizado após %d iterações.\n", iterations);
	printf("Norma do resíduo: %g\n", sqrt(aux));
	printf("Solução encontrada:\n");
	for (i = 0; i < m; i++)
		printf("x[%d] = %.6f\n", i, x0[i]);

	free(x0);
	free(r0);
	free(d0);
	libera_matriz(a, m);
	return 0;
}