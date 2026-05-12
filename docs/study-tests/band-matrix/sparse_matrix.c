#include "sparse_matrix.h"

int inicializa_matriz(struct matriz *m, int lin, int col) {
	if (m == NULL || lin <= 0 || col <= 0)
		return MATRIZ_ERR_PARAM;

	temp_no **novo = malloc((size_t)lin * sizeof(void *));
	if (novo == NULL)
		return MATRIZ_ERR_MEM;

	for (int i = 0; i < lin; i++)
		novo[i] = NULL;

	m->linhas = lin;
	m->colunas = col;
	m->arranjo = novo;

	return MATRIZ_OK;
}