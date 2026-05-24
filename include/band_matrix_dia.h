#ifndef BAND_MATRIX_DIA_H
#define BAND_MATRIX_DIA_H

struct matriz_banda_dia {
	int n;  // ordem da matriz
	int mb; // meia-banda (número de diagonais acima/abaixo da principal)
	int bw; // largura total da banda = 2*mb + 1
	double **diagonais; // array de ponteiros, cada um aponta para um vetor de n doubles
};

void inicializa_matriz_dia(struct matriz_banda_dia *m, int n, int mb);
void libera_matriz_dia(struct matriz_banda_dia *m);
void preenche_matriz_dia(struct matriz_banda_dia *m, unsigned int seed_mat, unsigned int seed_diag);

// Produto matriz-vetor otimizado
double *matriz_banda_vetor_dia(const struct matriz_banda_dia *a, const double *x);

#endif