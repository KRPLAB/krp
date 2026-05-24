/**
 * @file band_matrix.h
 * @brief Definição da estrutura e funções para matriz esparsa com bandas
 * @author Kauan da Rosa Paulino
 * @date 2026-04-25
 * Esta biblioteca define a estrutura de dados para uma matriz esparsa com bandas, bem como funções para inicialização, acesso e liberação de memória. A matriz é armazenada em um formato compacto, onde apenas os elementos dentro da banda são alocados, economizando espaço e melhorando a eficiência para matrizes grandes.
 */

#ifndef BAND_MATRIX_H
#define BAND_MATRIX_H

struct matriz_banda {
	int linhas;		// Número de linhas da matriz
	int meia_banda;	// Número de elementos na meia banda (incluindo a diagonal)
	double *dados;	// Ponteiro para os dados da matriz, armazenados em formato compacto
};

/********** Funções para manipulação da matriz de banda **********/

/**
 * @brief Inicializa a matriz de banda com o número de linhas e meia banda especificados.
 * @param m Ponteiro para a estrutura da matriz de banda a ser inicializada
 * @param linhas Número de linhas da matriz
 * @param meia_banda Número de elementos na meia banda (incluindo a diagonal)
 * Esta função aloca memória para os dados da matriz, considerando apenas os elementos dentro da banda.
 * A memória é alocada como um array unidimensional, onde os elementos são armazenados em ordem de linha.
 */
void inicializa_matriz(struct matriz_banda *m, int linhas, int meia_banda);

/**
 * @brief Libera a memória alocada para os dados da matriz de banda.
 * @param m Ponteiro para a estrutura da matriz de banda a ser liberada
 * Esta função deve ser chamada para liberar a memória alocada para os dados da matriz quando
 * a matriz não for mais necessária, evitando vazamentos de memória.
 */
void libera_matriz(struct matriz_banda *m);

/**
 * @brief Imprime a matriz de banda no formato tradicional, preenchendo os elementos fora da banda com zeros.
 * @param m Ponteiro para a estrutura da matriz de banda a ser impressa
 * Esta função é útil para visualização e depuração, permitindo que o usuário veja a matriz
 * completa, mesmo que os elementos fora da banda sejam armazenados como zeros.
 */
void imprime_matriz(const struct matriz_banda *m);

/********** Outras funções para acesso e manipulação da matriz de banda **********/

/**
 * @brief Retorna o índice no vetor compacto de um elemento (i, j) da matriz banda.
 * @param i         Linha do elemento.
 * @param j         Coluna do elemento.
 * @param meia_banda Metade da largura de banda (sem contar a diagonal).
 * @return Índice no vetor 1D, ou -1 se (i, j) estiver fora da banda.
 *
 * O vetor compacto tem stride @c B = 2*meia_banda+1 por linha.
 * O índice é calculado como @c i*B + (j - i + meia_banda).
 */
int obter_indice_1d(int i, int j, int meia_banda);

/**
 * @brief Segundo passo de geração: preenche a diagonal para garantir SPD.
 * @param m       Ponteiro para a matriz banda já preenchida com os elementos fora da diagonal.
 * @param semente Semente do gerador de números aleatórios usada para o valor extra positivo.
 *
 * Para cada linha @p i, calcula a soma dos módulos de todos os elementos fora da diagonal
 * presentes na banda e define:
 * @code
 *   diagonal[i] = soma_dos_modulos + extra
 * @endcode
 * onde @c extra é um valor aleatório em [1, 2) gerado com a @p semente fornecida.
 *
 * Isso garante **dominância diagonal estrita**, condição suficiente para que uma matriz
 * simétrica seja Positiva Definida (SPD) — requisito obrigatório para o Método do
 * Gradiente Conjugado.
 *
 * @pre A matriz deve ter sido inicializada com @c inicializa_matriz() e todos os elementos
 *      fora da diagonal devem ter sido preenchidos antes desta chamada.
 */
void preencher_diagonal_spd(struct matriz_banda *m, unsigned int semente);

/**
 * @brief Preenche a matriz de banda com valores aleatórios simétricos e garante SPD.
 * @param m          Ponteiro para a matriz banda inicializada.
 * @param seed_mat   Semente para preencher elementos fora da diagonal (simétrico).
 * @param seed_diag  Semente para preencher a diagonal com garantia de SPD.
 *
 * Esta função combina o preenchimento simétrico da parte fora da diagonal com a garantia
 * de dominância diagonal estrita para tornar a matriz Simétrica Positiva Definida (SPD).
 * Internamente, chama @c preencher_diagonal_spd() com @p seed_diag.
 */
void preenche_matriz(struct matriz_banda *m, unsigned int seed_mat, unsigned int seed_diag);

/**
 * @brief Multiplica a matriz banda A por um vetor x e retorna o resultado.
 * @param a Matriz banda.
 * @param x Vetor de entrada (tamanho a->linhas).
 * @return Ponteiro para vetor resultado (tamanho a->linhas). Deve ser liberado com free.
 */
double *matriz_banda_vetor(const struct matriz_banda * restrict a, const double * restrict x);

#endif // BAND_MATRIX_H
