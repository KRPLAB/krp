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

O projeto conta com **três versões** do solucionador CG, cada uma com um nível crescente de otimização:

1. **`cgSolver-naive`**: Implementação direta do algoritmo matemático, sem otimizações de compilador (-O0).
2. **`cgSolver`**: Implementação otimizada com flags de compilador (-O3 -march=native) e loop unrolling manual + restrict.
3. **`cgSolver-dia`**: Implementação com formato DIA (Diagonal Storage) puro, permitindo vetorização automática (SIMD/AVX2) para acessos com stride regular ao vetor de solução.

## Estrutura do Repositório

```
krp/
├── cgSolver                        # Binário: versão OPT (-O3 -march=native)
├── cgSolver-dia                    # Binário: versão DIA (-O3 -march=native)
├── cgSolver-naive                  # Binário: versão ingênua (-O0)
│
├── docs/                           # Documentação e estudos
│   ├── especificacao-t1.pdf        # Enunciado oficial do trabalho
│   ├── results/                    # Resultados de execuções
│   │   ├── graficos/               # Gráficos de comparação
│   │   │   ├── bw_comparativo.pdf  # Comparativo de largura de banda
│   │   │   ├── speedup_comparativo.pdf  # Comparativo de speedup
│   │   │   └── tempo_comparativo.pdf    # Comparativo de tempo
│   │   ├── naive/                  # Resultados da versão NAIVE (30 rodadas + Likwid)
│   │   │   ├── resultados_2026-05-23T21-03-45Z_naive.json
│   │   │   ├── resultados_2026-05-23T21-03-45Z_naive_aggregated_basic.csv
│   │   │   ├── resultados_2026-05-23T22-20-56Z_naive_likwid.json
│   │   │   └── resultados_2026-05-23T22-20-56Z_naive_likwid_aggregated.csv
│   │   ├── opt1/                   # Resultados da versão OPT1 (30 rodadas + Likwid)
│   │   │   ├── resultados_2026-05-24T00-14-15Z.json
│   │   │   ├── resultados_2026-05-24T00-14-15Z_aggregated_basic.csv
│   │   │   ├── resultados_2026-05-24T00-16-48Z.json
│   │   │   └── resultados_2026-05-24T00-16-48Z_aggregated.csv
│   │   ├── opt2/                   # Resultados da versão OPT2 (30 rodadas + Likwid)
│   │   │   ├── resultados_2026-05-24T18-06-55Z.json
│   │   │   ├── resultados_2026-05-24T18-06-55Z_aggregated_basic.csv
│   │   │   ├── resultados_2026-05-24T18-09-29Z.json
│   │   │   └── resultados_2026-05-24T18-09-29Z_aggregated.csv
│   │   └── opt3/                   # Resultados da versão OPT3 (DIA, 30 rodadas + Likwid)
│   │       ├── resultados_2026-05-24T19-55-36Z.json
│   │       ├── resultados_2026-05-24T19-55-36Z_aggregated_basic.csv
│   │       ├── resultados_2026-05-24T19-58-08Z.json
│   │       └── resultados_2026-05-24T19-58-08Z_aggregated.csv
│   └── study-tests/                # Testes e estudos preliminares
│       ├── band-matrix/            # Estudos sobre armazenamento em banda
│       │   ├── band-matrix_video.c
│       │   ├── matrix-band-4x4.txt
│       │   ├── readme.md
│       │   ├── sparse_matrix.c
│       │   ├── sparse_matrix.h
│       │   └── teste_banda_menos_if.c
│       └── gradients/              # Estudos sobre o método CG
│           ├── gradiente.c
│           ├── gradiente_conjugado.c
│           └── readme.md
│
├── include/                        # Arquivos de cabeçalho
│   ├── band_matrix.h               # Protótipos para matrizes de banda
│   ├── band_matrix_dia.h           # Protótipos para formato DIA
│   ├── common.h                    # Definições comuns e macros
│   └── timer.h                     # Protótipos para funções de temporização
│
├── obj/                            # Arquivos objetos (gerados na compilação)
│   ├── dia/                        # Objetos da versão DIA
│   │   ├── band_matrix_dia.o
│   │   ├── cgSolver-dia.o
│   │   ├── main_dia.o
│   │   └── timer.o
│   ├── naive/                      # Objetos da versão NAIVE
│   │   ├── band_matrix.o
│   │   ├── cg_naive.o
│   │   ├── main.o
│   │   └── timer.o
│   └── opt/                        # Objetos da versão OPT
│       ├── band_matrix.o
│       ├── cg_opt.o
│       ├── main.o
│       └── timer.o
│
├── scripts/                        # Scripts para execução e análise
│   ├── analyze_results.py          # Análise e geração de tabelas (com métricas de hardware)
│   ├── analyze_results_basic.py    # Análise básica (sem hardware)
│   ├── generate_plots.py           # Geração de gráficos comparativos
│   ├── plot_results.py             # Script auxiliar para plotagem
│   └── run_tests.sh                # Script unificado para testar todas as versões
│
├── src/                            # Código-fonte do projeto
│   ├── band_matrix.c               # Funções para geração e manipulação (formato banda)
│   ├── band_matrix_dia.c           # Funções para formato DIA puro
│   ├── cg_naive.c                  # Implementação ingênua do CG (-O0)
│   ├── cg_opt.c                    # Implementação otimizada do CG (banda compacta)
│   ├── cgSolver-dia.c              # Implementação com formato DIA
│   ├── main.c                      # Programa principal (versão OPT)
│   ├── main_dia.c                  # Programa principal (versão DIA)
│   └── timer.c                     # Funções para medição de tempo
│
├── Makefile                        # Makefile para compilação (compila 3 versões)
└── README.md                       # Este arquivo
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

O projeto inclui um sistema automatizado para coleta de métricas em JSON com suporte a múltiplas rodadas e agregação estatística.

**Script principal:** `scripts/run_tests.sh`

```bash
# Execução simples (1 rodada, sem Likwid, testa todas as 3 versões)
./scripts/run_tests.sh

# 3 rodadas para cálculo de média e desvio padrão
./scripts/run_tests.sh --runs 3

# 5 rodadas com coleta de métricas de hardware (Likwid)
./scripts/run_tests.sh --runs 5 --likwid

# Modo silencioso
./scripts/run_tests.sh --runs 3 --quiet
```

**Saída:** Um arquivo JSON com timestamp, contendo resultados de **todas as 3 versões** (cgSolver-naive, cgSolver, cgSolver-dia) estruturados hierarquicamente:
```json
{
  "metadata": {"timestamp": "...", "num_runs": 3, "likwid_enabled": true},
  "results": {
    "cgSolver-naive": [{"config": {...}, "executions": [...]}],
    "cgSolver": [{"config": {...}, "executions": [...]}],
    "cgSolver-dia": [{"config": {...}, "executions": [...]}]
  }
}
```

#### Agregação e Geração de Relatórios

Para agregar os resultados e gerar tabelas em múltiplos formatos:

```bash
python3 scripts/analyze_results.py resultados_2026-05-24T*.json
```

Gera automaticamente:
- **CSV consolidado** (`_aggregated.csv`): para análise com Excel/Pandas
- **LaTeX** (`_tables.tex`): tabelas prontas para incluir no relatório PDF

**Resumo no console:**
```
RESUMO DA ANÁLISE
================
cgSolver-naive:
  N=   1024 | Bandas=7 | Tempo=1.234e-03s (±5.67e-05s)
  ...

Speedup (Naive / Otimizado):
  Média: 2.45x
  Min:   1.98x
  Max:   3.12x
```

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

## Estratégia de Otimização

O projeto segue uma **metodologia iterativa de otimização**, com três fases bem definidas, cada uma com aumento progressivo de complexidade e ganho de desempenho esperado:

### Otimização 1: Flags de Compilador (-O3 -march=native)

**Status:** ✅ Concluído

- **Descrição:** Aplicação de otimizações padrão de compilador (nível 3) e extensões específicas da arquitetura (-march=native).
- **Benefício esperado:** 2–4x de speedup sem mudanças de código.
- **Implementação:** `cgSolver` usa `FLAGS_OPT = -O3 -march=native`; `cgSolver-naive` usa `-O0` para baseline.
- **Documentação:** [docs/estudo_01.md](docs/estudo_01.md)

### Otimização 2: Loop Unrolling + restrict (Banda Compacta)

**Status:** ✅ Concluído

- **Descrição:** Manualmente desenrolar laços internos no produto matriz-vetor e usar `restrict` para promover vetorização automática.
- **Ganho incremental esperado:** 1.2–1.8x sobre Otimização 1 (total: ~3–7x sobre naive).
- **Implementação:** `cg_opt.c` + `band_matrix.c` com otimizações de laço.
- **Documentação:** [docs/estudo_02.md](docs/estudo_02.md)

### Otimização 3: Formato DIA Puro (Diagonal Storage)

**Status:** ✅ Concluído

- **Descrição:** Restruturação de dados: cada diagonal armazenada como vetor separado, permitindo acessos com stride regular ao vetor de solução `x`.
- **Ganho incremental esperado:** 1.3–2.5x sobre Otimização 2 (total: ~5–17x sobre naive).
- **Benefício principal:** Vetorização automática trivial (SIMD/AVX2) due to predictable memory access patterns.
- **Implementação:** `band_matrix_dia.c` + `cgSolver-dia.c` + `main_dia.c`
- **Documentação:** [docs/estudo_03.md](docs/estudo_03.md)
- **Estrutura:** Format diagonal puro (DIA) - cada diagonal é um vetor contíguo de comprimento n. Mapeamento: diagonal d (deslocamento) está em `diagonais[d + mb]` onde mb é a meia-banda.

### Otimização 4: Intrinsecar AVX2 (Futuro - Opcional)

**Status:** 🔄 Planejado

- **Descrição:** Uso explícito de intrinsecar AVX2 nos kernels críticos (produto matriz-vetor, produtos internos).
- **Ganho esperado:** +20–30% adicional.
- **Não será implementado neste trabalho**, mas a base está pronta para expansão futura.

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