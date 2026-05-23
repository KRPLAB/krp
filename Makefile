CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -I./include
LDFLAGS = -lm

SRC_DIR   = src
OBJ_NAIVE = obj/naive
OBJ_OPT   = obj/opt

FLAGS_NAIVE = $(CFLAGS) -O0
FLAGS_OPT   = $(CFLAGS) -O3 -march=native

COMMON_SRCS = $(SRC_DIR)/band_matrix.c $(SRC_DIR)/timer.c
SRCS_NAIVE  = $(COMMON_SRCS) $(SRC_DIR)/cg_naive.c $(SRC_DIR)/main.c
SRCS_OPT    = $(COMMON_SRCS) $(SRC_DIR)/cg_opt.c   $(SRC_DIR)/main.c

OBJS_NAIVE = $(patsubst $(SRC_DIR)/%.c, $(OBJ_NAIVE)/%.o, $(SRCS_NAIVE))
OBJS_OPT   = $(patsubst $(SRC_DIR)/%.c, $(OBJ_OPT)/%.o,   $(SRCS_OPT))

all: cgSolver-naive cgSolver

$(OBJ_NAIVE) $(OBJ_OPT):
	mkdir -p $@

$(OBJ_NAIVE)/%.o: $(SRC_DIR)/%.c | $(OBJ_NAIVE)
	$(CC) $(FLAGS_NAIVE) -c -o $@ $<

$(OBJ_OPT)/%.o: $(SRC_DIR)/%.c | $(OBJ_OPT)
	$(CC) $(FLAGS_OPT) -c -o $@ $<

cgSolver-naive: $(OBJS_NAIVE)
	$(CC) -o $@ $^ $(LDFLAGS)

cgSolver-opt: $(OBJS_OPT)
	$(CC) -o $@ $^ $(LDFLAGS)

cgSolver: $(OBJS_OPT)
	$(CC) -o $@ $^ $(LDFLAGS)

test: all
	@echo "Executando testes (1 rodada, sem Likwid)..."
	bash ./scripts/run_tests.sh

# Executa testes com múltiplas rodadas (ex: make test-runs NUM_RUNS=3)
test-runs: all
	@echo "Executando testes (${NUM_RUNS} rodadas)..."
	bash ./scripts/run_tests.sh --runs ${NUM_RUNS}

test-likwid: all
	@echo "Executando testes com Likwid (3 rodadas)..."
	bash ./scripts/run_tests.sh --runs 3 --likwid

analyze:
	@if [ -z "$(RESULTS)" ]; then \
		echo "Uso: make analyze RESULTS=resultados_*.json"; \
		exit 1; \
	fi
	@python3 ./scripts/analyze_results.py $(RESULTS)

clean:
	rm -rf obj cgSolver cgSolver-naive resultados_*.csv

.PHONY: all clean test
