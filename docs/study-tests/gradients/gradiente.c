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

double *gradiente(double **a, int m, double *x, double *r) {
	double *Ar = matriz_vetor(a, m, r);
	if (!Ar)
		return NULL;

	double r_dot = dot_product(r, m, r);
	double denom = dot_product(r, m, Ar);
	if (denom == 0.0) {
		free(Ar);
		return NULL;
	}

	double alpha = r_dot / denom;
	free(Ar);

	double *x_new = malloc(m * sizeof(double));
	if (!x_new)
		return NULL;

	for (int i = 0; i < m; i++)
		x_new[i] = x[i] + alpha * r[i];

	return x_new;
}

void libera_matriz(double **mat, int m) {
	if (!mat)
		return;
	for (int i = 0; i < m; i++)
		free(mat[i]);
	free(mat);
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Uso: %s <linhas> <colunas>\n", argv[0]);
		return 1;
	}

	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	// Para a matriz aumentada, precisamos que n == m+1
	if (n != m + 1) {
		fprintf(stderr, "Erro: a matriz deve ser aumentada (número de colunas "
		                "= linhas + 1)\n");
		return 1;
	}

	double **a = le_matriz(m, n);
	if (!a) {
		fprintf(stderr, "Erro ao ler a matriz\n");
		return 1;
	}

	// Leitura do vetor inicial x0
	double *x0 = malloc(m * sizeof(double));
	if (!x0) {
		fprintf(stderr, "Erro de alocação\n");
		libera_matriz(a, m); // supõe uma função para liberar a matriz
		return 1;
	}
	for (int i = 0; i < m; i++) {
		if (scanf("%lf", &x0[i]) != 1) {
			fprintf(stderr, "Erro na leitura de x0\n");
			free(x0);
			libera_matriz(a, m);
			return 1;
		}
	}

	// Resíduo inicial r0 = b - A*x0
	double *Ax = matriz_vetor(a, m, x0);
	if (!Ax) {
		free(x0);
		libera_matriz(a, m);
		return 1;
	}
	double *r0 = malloc(m * sizeof(double));
	if (!r0) {
		free(x0);
		free(Ax);
		libera_matriz(a, m);
		return 1;
	}
	for (int i = 0; i < m; i++)
		r0[i] = a[i][m] - Ax[i];
	free(Ax);

	const double tolerance = 1e-8;
	const int max_iter = 1000;
	int iterations = 0;

	while (norma(r0, m) > tolerance && iterations < max_iter) {
		double *x_new = gradiente(a, m, x0, r0);
		if (!x_new) {
			fprintf(stderr, "Falha no gradiente (divisão por zero?)\n");
			break;
		}

		free(x0);
		x0 = x_new;

		// Atualiza resíduo: r = b - A*x
		double *Ax_novo = matriz_vetor(a, m, x0);
		if (!Ax_novo) {
			free(x0);
			free(r0);
			libera_matriz(a, m);
			return 1;
		}
		double *r_novo = malloc(m * sizeof(double));
		if (!r_novo) {
			free(x0);
			free(r0);
			free(Ax_novo);
			libera_matriz(a, m);
			return 1;
		}
		for (int i = 0; i < m; i++)
			r_novo[i] = a[i][m] - Ax_novo[i];
		free(Ax_novo);

		free(r0);
		r0 = r_novo;

		iterations++;

		printf("Iteração %d:\n", iterations);
		for (int i = 0; i < m; i++)
			printf("x[%d] = %.6f\n", i, x0[i]);
		printf("\n");
	}

	printf("Finalizado após %d iterações.\n", iterations);
	printf("Norma do resíduo: %g\n", norma(r0, m));
	printf("Solução encontrada:\n");
	for (int i = 0; i < m; i++)
		printf("x[%d] = %.6f\n", i, x0[i]);

	free(x0);
	free(r0);
	libera_matriz(a, m);
	return 0;
}