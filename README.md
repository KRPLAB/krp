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
│   │   └── ...                 # Teste 1: Análise 
│   └── especificacao-t1.pdf    # Enunciado oficial do trabalho
├── Makefile                    # Makefile para compilação do projeto
├── README.md                   # Este arquivo de descrição do projeto
└── .gitignore                  # Arquivo para ignorar arquivos desnecessários no Git
```


---