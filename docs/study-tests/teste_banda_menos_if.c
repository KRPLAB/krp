#include <stdlib.h>

/**
 * Função para obter o índice em 1D de um elemento em uma matriz banda
 * Otimizada para tamanho de vetor = linha * B, onde B = meia banda * 2 + 1
 * @param i linha do elemento
 * @param j coluna do elemento
 * @param meia_banda metade da banda
 * @return índice em 1D ou -1 se estiver fora da banda
 */
int obter_indice_1d(int i, int j, int meia_banda) {
	if (abs(i - j) > meia_banda) {
		return -1;
	}
	return i * (meia_banda*2 + 1) + (j - i + meia_banda);
}