#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

/**
 * @brief Definições e macros para manipulação de matrizes esparsas.
 * Este arquivo contém a definição da estrutura de dados para representar
 * matrizes esparsas utilizando listas encadeadas, bem como as funções para
 * inicialização, inserção de elementos, Implementação baseada em conceitos de
 * estruturas de dados e algoritmos para matrizes esparsas Principal referência:
 * "Estrutura de Dados - Aula 14 - Matriz esparsa" do canal UNIVESP. URL:
 * https://youtu.be/C_ePgrEbLs0 Acesso em: 24/04/2026
 */

#include <stdio.h>
#include <stdlib.h>

#define MATRIZ_OK 1
#define MATRIZ_ERR_PARAM 0
#define MATRIZ_ERR_MEM -1

/**
 * @brief Estrutura de dados para armazenar os valores de uma matriz
 * esparsa utilizando listas encadeadas. Cada nó da lista representa um elemento
 * não nulo da matriz, armazenando seu valor e a coluna correspondente. A
 * estrutura 'matriz' contém o número de linhas e colunas, além de um arranjo de
 * ponteiros para os nós da lista, permitindo acesso eficiente aos elementos não
 * nulos.
 */
typedef struct temp_no {
	double valor;
	int coluna;
	struct temp_no *prox;
} temp_no;

/**
 * @brief Estrutura de dados para representar uma matriz esparsa utilizando
 * listas encadeadas. A estrutura 'matriz' contém o número de linhas e colunas,
 * além de um arranjo de ponteiros para os nós da lista, permitindo acesso
 * eficiente aos elementos não nulos da matriz.
 */
struct matriz {
	int linhas;
	int colunas;
	temp_no **arranjo;
};

/**
 * @brief Inicializa uma matriz esparsa.
 * @param m Ponteiro para a matriz a ser inicializada.
 * @param lin Número de linhas da matriz.
 * @param col Número de colunas da matriz.
 * @return 1 se a matriz foi inicializada com sucesso, 0 caso contrário.
 * @return -1 em caso de erro de alocação de memória.
 */
int inicializa_matriz(struct matriz *m, int lin, int col);

void insere_elemento(struct matriz *m, int linha, int coluna, double valor);
void imprime_matriz(struct matriz *m);
void destroi_matriz(struct matriz *m);
#endif