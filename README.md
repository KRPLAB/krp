# Trabalho 1: Análise Numérica - Gradiente Conjugado (CG)

**Instituição:** Instituto Federal do Paraná (IFPR) - Campus Pinhais  
**Curso:** Bacharelado em Ciência da Computação  
**Disciplina:** Análise Numérica  
**Aluno:** Kauan da Rosa Paulino  

---

## Sobre o Projeto

Este projeto implementa o **Método do Gradiente Conjugado (CG)** para a resolução de sistemas de equações lineares ($Ax = b$). A matriz de coeficientes $A$ é gerada de forma pseudoaleatória (garantindo ser Simétrica e Positiva Definida - SPD) e possui uma estrutura de **banda** (tipicamente 7 ou 27 diagonais não-nulas). 

O foco do trabalho não é apenas na matemática do método iterativo, mas principalmente em **Computação de Alta Performance (HPC)**. Por conta das restrições de memória em matrizes imensas (ex: 1.048.576 x 1.048.576), foi implementada uma estrutura de dados de matriz esparsa otimizada baseada nos padrões do LAPACK/BLAS (achatar as bandas em um vetor 1D contíguo).

As principais especificações do projeto incluem:
- Implementação do CG para matrizes de banda.
- Geração de matrizes SPD
- Medição de desempenho (tempo de CPU, iterações, erro relativo) para matrizes de tamanhos variados (1024 a 1.048.576).
- Análise de desempenho e comparação entre uma versão ingênua (sem otimizações) e uma versão otimizada (com AVX e Loop Unrolling).

Mais detalhes sobre especificações, critérios de avaliação e requisitos estão disponíveis no enunciado oficial do trabalho no arquivo [`docs/especificacao-t1.pdf`](docs/especificacao-t1.pdf).

O projeto conta com duas versões do solucionador:
- **`cgSolver_naive`**: Implementação direta do algoritmo matemático.
- **`cgSolver_opt`**: Implementação com foco em otimização de CPU (Loop Unrolling & Jam, instruções vetorizadas AVX) e melhor uso da memória Cache.

## Estrutura do Repositório

```
krp/
├── src/                        # Código-fonte do projeto
│   ├── cg_naive.c              # Implementação ingênua do CG
│   ├── cg_opt.c                # Implementação otimizada do CG
│   ├── band_matrix.c           # Funções para geração e manipulação de matrizes de banda
│   ├── timer.c                 # Funções para medição de tempo
│   └── main.c                  # Programa principal para execução dos testes
├── include/                    # Arquivos de cabeçalho
│   ├── common.h                # Definições comuns e macros
│   ├── band_matrix.h           # Protótipos para matrizes de banda
│   └── timer.h                 # Protótipos para funções de temporização
├── scripts/                    # Scripts para execução de testes e geração de resultados
│   ├── run_tests.sh            # Script para rodar os testes e coletar resultados
│   └── likwid_metrics.sh       # Script para coletar métricas de desempenho com LIKWID
├── docs/                       # Documentação e enunciados
│   ├── study-tests/            # Documentação dos testes de estudo
│   │   ├── band-matrix/        # Estudo e ADR sobre armazenamento em banda
│   │   │   └── readme.md
│   │   └── gradients/          # Estudo sobre Método do Gradiente Conjugado
│   │       └── readme.md
│   └── especificacao-t1.pdf    # Enunciado oficial do trabalho
├── Makefile                    # Makefile para compilação do projeto
├── README.md                   # Este arquivo de descrição do projeto
└── .gitignore                  # Arquivo para ignorar arquivos desnecessários no Git
```

---

## Como Compilar e Executar

O projeto utiliza um `Makefile` para gerenciar o processo de build.

### Compilação

Para compilar ambas as versões (naive e otimizada), basta rodar na raiz do projeto:

```bash
make
```

*(Isso gerará os binários `cgSolver-naive` (versão sem otimizações) e `cgSolver` na raiz)*

Para limpar os arquivos objetos e binários compilados:

```bash
make clean
```

### Execução

Os binários devem ser executados passando os parâmetros de geração da matriz por linha de comando:

```bash
./cgSolver-naive <tamanho_N> <num_bandas> <semente_matriz> [semente_diagonal]
```

* `<tamanho_N>`: Dimensão NxN da matriz (ex: 1024, 4096, 1048576).
* `<num_bandas>`: Quantidade de bandas não-nulas (ex: 7 ou 27).
* `<semente_matriz>`: Semente (seed) fixa para geração dos valores das posições da matriz.
* `[semente_diagonal]`: (Opcional) Semente para garantir o excesso numérico da diagonal principal para matrizes SPD.

**Exemplo de execução:**

```bash
./cgSolver-naive 1024 7 2026 54321
```

A saída do programa é fornecida em formato CSV padronizado:
```
tamanho,bandas,seed,iteracoes,tempo_execucao,erro_relativo
```

### Execução Automatizada (Bateria de Testes)

Em construção.

---

## Histórico de Desenvolvimento e Progresso

Abaixo está o registro das principais tomadas de decisão e evoluções arquiteturais do sistema ao longo das semanas de desenvolvimento:

* **24/04/2026 e 25/04/2026 - Estruturação de Dados e Layout de Memória; Geração Procedural e Matriz SPD**
  * Estudo sobre matrizes esparsas. Decisão técnica de **não utilizar** Arranjo de Listas Ligadas (LIL) devido ao alto custo de *Cache Misses* e incompatibilidade com instruções vetoriais (AVX).
  * Adoção do modelo de vetor unidimensional contíguo (inspirado no armazenamento em banda do LAPACK/BLAS). Mapeamento das coordenadas `(i, j)` para `1D` implementado com sucesso sem o uso de condicionais extras no laço interno.
  * Refatoração do gerador de matrizes.
  * Implementada a lógica baseada no Teorema dos Círculos de Gershgorin (dominância diagonal estrita) para garantir matematicamente que a matriz gerada aleatoriamente seja Simétrica Positiva Definida (SPD), condição necessária para a convergência do CG. Uso separado de sementes para estrutura e para diagonal.

* **12/05/2026 - Estudo do Algoritmo do Gradiente Conjugado**
  * Início da revisão matemática de derivadas parciais e vetores gradientes.
  * Implementação em ambiente isolado (`gradiente.c` e `gradiente_conjugado.c` da estrutura padrão do método) baseado nas videoaulas do Prof. Thadeu Penna para validar a teoria de "escorregar pela encosta" do paraboloide até o mínimo.

* **18/05/2026 - Integração CG + Matriz de Banda (Versão Naive)**
  * Finalização da adaptação do CG para trabalhar exclusivamente com o nosso formato em vetor 1D.
  * A operação crítica $A \cdot d = z$ foi implementada na função `matriz_banda_vetor`, percorrendo de forma eficiente apenas os limites reais das colunas da banda através das macros `MAX` e `MIN`.
  * Validação com sistema-teste: geração de matriz SPD em banda, solução conhecida, cálculo de $b = A \cdot x_{true}$ e resolução com CG. Erro relativo tipicamente da ordem de $10^{-14}$ a $10^{-10}$, confirmando a corretude da implementação.

---

## Análise de Eficiência

### Memória

Para matrizes de dimensão $n = 1.048.576$ com 27 bandas:
- **Representação densa:** ~8 TB (impraticável)
- **Representação em banda:** ~226 MB (viável)

Essa redução permite que os testes sejam executados em hardware comum, viabilizando o projeto.

### Tempo de Execução

Cada iteração do CG realiza:
- Uma multiplicação matriz-vetor com custo $O(n \cdot bw)$, onde $bw = 2 \cdot mb + 1$ é a largura da banda (7 ou 27).
- Produtos internos e somas vetoriais com custo $O(n)$.

Para matrizes bem condicionadas (como as geradas com dominância diagonal estrita), o número de iterações é tipicamente muito menor que $n$, resultando em tempo total aceitável mesmo para os maiores casos.

### Modularidade e Preparação para Otimização

As funções de matriz banda ficam isoladas em `band_matrix.c`, enquanto o CG está em `cg_naive.c`. Isso facilita a substituição futura por versões otimizadas sem mexer no restante do código. Melhorias previstas incluem:
- Desenrolamento de laços (loop unrolling)
- Instruções vetoriais AVX2/AVX-512
- Reorganização dos dados para melhorar localidade de cache
- Técnicas de blocking para reduzir faltas de cache

---

## Ferramentas Utilizadas

* **GCC**: Compilador C (com flags padrão e de otimização).
* **Make**: Automação de compilação.
* **LIKWID**: Monitoramento de hardware (cache e banda de memória) utilizado para coletar os dados do relatório final.

---