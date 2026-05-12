## Documento de Decisão Arquitetural (ADR) - 25/04/2026
**Contexto:** Armazenamento otimizado de Matrizes em Banda para o Método do Gradiente Conjugado.

### 1. Estrutura de Dados: Vetor 1D vs. Arranjo de Listas Ligadas (LIL)

A decisão de abandonar estruturas baseadas em ponteiros (como listas ligadas para representar as colunas não-nulas) em favor de um **Vetor Unidimensional Contíguo (Flat Array)** baseia-se em dois pilares da arquitetura de computadores:

* **Localidade Espacial e Cache da CPU:** A memória RAM é lenta. Para contornar isso, a CPU carrega blocos inteiros de memória contígua (linhas de cache, geralmente 64 bytes) para as caches L1/L2/L3. Numa lista ligada, cada nó é alocado via `malloc` em pontos aleatórios do *heap*. Iterar por essa matriz geraria um alto índice de **Cache Misses** (o que o LIKWID fatalmente apontaria no seu relatório), forçando a CPU a esperar a RAM. Um vetor 1D garante que, ao ler `vetor[0]`, os próximos elementos já estejam no cache.
* **Vetorização (AVX / SIMD):** O trabalho exige otimização via *intrinsics* (AVX). Essas instruções processam múltiplos dados simultaneamente (ex: somam 4 números `double` em um único ciclo de clock), mas **exigem** que esses dados estejam perfeitamente alinhados e contíguos na memória. Instruções vetoriais são incapazes de seguir ponteiros de uma lista ligada.

> **Resumo:** A lista ligada otimiza o uso da memória RAM em 100%, mas destrói a performance da CPU. O vetor 1D otimiza a CPU, viabilizando o processamento em tempo hábil.

---

### 2. Layout de Memória: Padding vs. Encaixe Perfeito

A largura total da banda é $B$. A decisão de alocar blocos idênticos de tamanho $B$ para todas as linhas (`Tamanho = linhas * B`), ignorando que as primeiras e últimas linhas possuem menos elementos válidos, é um *trade-off* clássico de HPC.

Ao economizar esse espaço (ex: subtraindo os cantos com `linhas * B - constante`), seria necessário a utilização de lógica condicional para acessar a memória:

```c
// Cenário Ruim (Encaixe Perfeito)
if (linha < d) { 
    // Fórmula para o topo 
} else if (linha > n - d) { 
    // Fórmula para a base 
} else { 
    // Fórmula padrão 
}
```

* **O Custo do Branch Prediction:** CPUs modernas usam *pipelines* longos (buscam, decodificam e executam dezenas de instruções ao mesmo tempo). Para não parar nos `ifs`, a CPU tenta "adivinhar" o caminho (*Branch Prediction*). Se ela errar, todo o trabalho adiantado é jogado fora (*Pipeline Flush*), custando de 15 a 20 ciclos de clock. 
* **A Solução Determinística:** Ao garantir que toda linha tem exatamente o tamanho $B$, preenchendo os "cantos" matemáticos com lixo ou zeros virtuais (Padding), reduzimos o acesso a uma pura equação de 1º grau: `indice = i * B + (j - i + d)`. 

> **Resumo:** Troca-se o desperdício de alguns ínfimos bytes de memória (fantasmas nas bordas) pela eliminação completa de instruções de desvio (`ifs`) no laço interno de cálculo. A CPU substitui ramificações imprevisíveis por aritmética determinística e executável em 1 ciclo.


### Referências Bibliográficas

ANDERSON, E. et al. **LAPACK Users' Guide**. 3. ed. Philadelphia: SIAM, 1999. 
- O capítulo *Band Storage* detalha exatamente o porquê de adotarem arrays com colunas fantasmas (`LDAB` - *Leading Dimension of Array B*) para otimizar chamadas BLAS.

GOLUB, G. H.; VAN LOAN, C. F. **Matrix Computations**. 4. ed. Baltimore: Johns Hopkins University Press, 2013.
- A "Bíblia" da álgebra linear computacional. O Capítulo 1 (Matrix Multiplication) tem uma seção inteira dedicada a *Banded Matrices* e como o layout de memória afeta o número de *flops* (operações em ponto flutuante).

SAAD, Y. **Iterative Methods for Sparse Linear Systems**. 2. ed. Philadelphia: SIAM, 2003.
- O Capítulo 3 cobre formatos de armazenamento (CSR, COO, DIA). O formato que estamos usando é formalmente conhecido como **DIA (Diagonal Storage Format)**.
