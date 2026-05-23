#!/bin/bash

# Script unificado para executar testes e coletar métricas em JSON
# Uso: ./scripts/run_tests.sh [--runs N] [--likwid] [--quiet]
#
# Exemplos:
#   ./scripts/run_tests.sh                    # 1 rodada, sem Likwid
#   ./scripts/run_tests.sh --runs 3 --likwid # 3 rodadas com Likwid
#   ./scripts/run_tests.sh --runs 5           # 5 rodadas para média

set -e

# Configuração padrão
NUM_RUNS=1
ENABLE_LIKWID=0
VERBOSE=1

# Parse argumentos
while [[ $# -gt 0 ]]; do
	case $1 in
		--runs)
			NUM_RUNS="$2"
			shift 2
			;;
		--likwid)
			ENABLE_LIKWID=1
			shift
			;;
		--quiet)
			VERBOSE=0
			shift
			;;
		*)
			echo "Opção desconhecida: $1"
			echo "Uso: $0 [--runs N] [--likwid] [--quiet]"
			exit 1
			;;
	esac
done

print_header() {
	if [ $VERBOSE -eq 1 ]; then
		echo "=================================================="
		echo " $1"
		echo "=================================================="
	fi
}

print_step() {
	if [ $VERBOSE -eq 1 ]; then
		echo "$1"
	fi
}

# Configuração dos testes
sizes=(1024 4096 16384 65536 262144 1048576)
bands=(7 27)
seeds=(42 123)

# Timestamp ISO 8601
TIMESTAMP=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
OUTPUT_FILE="resultados_${TIMESTAMP//:/-}.json"

# Função para extrair métrica do Likwid
extract_likwid_metric() {
  local file=$1
  local metric_name=$2
  
  if [ ! -f "$file" ]; then
    echo "0"
    return
  fi
  
  # Procura por padrões comuns no output do Likwid
  grep -oP "(?<=$metric_name\s+:\s)\S+" "$file" 2>/dev/null | head -1 || echo "0"
}

# Verifica compilação
print_header "COMPILAÇÃO"
print_step "[1/5] A compilar os códigos-fonte..."

if make clean > /dev/null 2>&1; then
	:
fi

if ! make > /dev/null 2>&1; then
	echo "Erro na compilação. Abortando testes. Verificar mensagens de erro acima."
	echo "Executar 'make' manualmente para detalhes."
	exit 1
fi

print_step "!! Compilação concluída com sucesso"

# ============================================================================
# INÍCIO DO JSON
# ============================================================================

{
	echo "{"
	echo "  \"metadata\": {"
	echo "    \"timestamp\": \"$TIMESTAMP\","
	echo "    \"num_runs\": $NUM_RUNS,"
	echo "    \"likwid_enabled\": $([ $ENABLE_LIKWID -eq 1 ] && echo 'true' || echo 'false')"
	echo "  },"
	echo "  \"results\": {"

  # Loop sobre cada binário
  FIRST_BINARY=1
  for BINARY in "cgSolver-naive"; do
	  if [ ! -f "./$BINARY" ]; then
		  continue
	  fi

	  if [ $FIRST_BINARY -eq 0 ]; then
		  echo "    },"
	  fi
	  FIRST_BINARY=0

	  echo "    \"$BINARY\": ["

    # Loop sobre cada combinação de parâmetros
    FIRST_PARAM=1
    for n in "${sizes[@]}"; do
	    for b in "${bands[@]}"; do
		    for s in "${seeds[@]}"; do
			    if [ $FIRST_PARAM -eq 0 ]; then
				    echo "      },"
			    fi
			    FIRST_PARAM=0

			    echo "      {"
			    echo "        \"config\": {"
			    echo "          \"tamanho\": $n,"
			    echo "          \"bandas\": $b,"
			    echo "          \"seed\": $s"
			    echo "        },"
			    echo "        \"executions\": ["

	  # Loop sobre múltiplas execuções
	  for run in $(seq 1 $NUM_RUNS); do
		  if [ $run -gt 1 ]; then
			  echo "        },"
		  fi

		  echo "          {"
		  echo "            \"run\": $run,"

	    # Executa e captura saída
	    PERF_OUTPUT=$(./$BINARY $n $b $s 2>/dev/null || echo "")

	    if [ -z "$PERF_OUTPUT" ]; then
		    echo "            \"error\": \"Execução falhou\","
		    echo "            \"performance\": null,"
		    echo "            \"hardware_metrics\": null"
	    else
		    # Parse da saída CSV do programa
		    IFS=',' read -r tam bandas seed iteracoes tempo erro <<< "$PERF_OUTPUT"

		    echo "            \"performance\": {"
		    echo "              \"tamanho\": ${tam:-0},"
		    echo "              \"bandas\": ${bandas:-0},"
		    echo "              \"seed\": ${seed:-0},"
		    echo "              \"iteracoes\": ${iteracoes:-0},"
		    echo "              \"tempo_s\": ${tempo:-0.0},"
		    echo "              \"erro_relativo\": ${erro:-0.0}"
		    echo "            },"

	      # Coleta métricas do LIKWID apenas na primeira execução
	      if [ $ENABLE_LIKWID -eq 1 ] && [ $run -eq 1 ]; then
		      L3_FILE="/tmp/likwid_l3_${BINARY}.txt"
		      MEM_FILE="/tmp/likwid_mem_${BINARY}.txt"
		      FLOPS_FILE="/tmp/likwid_flops_${BINARY}.txt"
		      ENERGY_FILE="/tmp/likwid_energy_${BINARY}.txt"

		      # L3 Cache
		      sudo likwid-perfctr -C 0 -g L3CACHE ./$BINARY $n $b $s \
			      > "$L3_FILE" 2>&1 || true

		      # Memória
		      sudo likwid-perfctr -C 0 -g MEM ./$BINARY $n $b $s \
			      > "$MEM_FILE" 2>&1 || true

		      # FLOPS double precision
		      sudo likwid-perfctr -C 0 -g FLOPS_DP ./$BINARY $n $b $s \
			      > "$FLOPS_FILE" 2>&1 || true

		      # Energia
		      sudo likwid-perfctr -C 0 -g ENERGY ./$BINARY $n $b $s \
			      > "$ENERGY_FILE" 2>&1 || true

		      # Extração das métricas
		      L3_REQ=$(grep "L3 request rate" "$L3_FILE" | awk '{print $NF}' | head -1)
		      L3_MISS=$(grep "L3 miss rate" "$L3_FILE" | awk '{print $NF}' | head -1)

		      MEM_BW=$(grep "Memory bandwidth" "$MEM_FILE" | awk '{print $NF}' | head -1)

		      FLOPS_DP=$(grep "DP MFLOP/s" "$FLOPS_FILE" | awk '{print $NF}' | head -1)

		      ENERGY=$(grep "Energy \[J\]" "$ENERGY_FILE" | awk '{print $NF}' | head -1)

		      # Valores padrão
		      L3_REQ=${L3_REQ:-0}
		      L3_MISS=${L3_MISS:-0}
		      MEM_BW=${MEM_BW:-0}
		      FLOPS_DP=${FLOPS_DP:-0}
		      ENERGY=${ENERGY:-0}

		      echo "            \"hardware_metrics\": {"
		      echo "              \"l3_request_rate\": $L3_REQ,"
		      echo "              \"l3_miss_rate\": $L3_MISS,"
		      echo "              \"memory_bandwidth_gb_s\": $MEM_BW,"
		      echo "              \"flops_dp_mflops\": $FLOPS_DP,"
		      echo "              \"energy_j\": $ENERGY"
		      echo "            }"

		      rm -f \
			      "$L3_FILE" \
			      "$MEM_FILE" \
			      "$FLOPS_FILE" \
			      "$ENERGY_FILE"

	      else
		      echo "            \"hardware_metrics\": null"
	      fi
	    fi
    done

    echo "        }"
    echo "        ]"
done
done
done

if [ $FIRST_PARAM -eq 0 ]; then
	echo "      }"
fi
echo "    ]"
done

if [ $FIRST_BINARY -eq 0 ]; then
	echo "    }"
fi
echo "  }"
echo "}"
} > "$OUTPUT_FILE"

# ============================================================================
# RESUMO FINAL
# ============================================================================

print_header "TESTES CONCLUÍDOS COM SUCESSO"

if [ $VERBOSE -eq 1 ]; then
	echo ""
	echo "Arquivo JSON gerado: $OUTPUT_FILE"
	echo ""
	echo "Estrutura JSON:"
	echo "  {metadata, results: {cgSolver-naive: [{config, executions}], cgSolver: [...]}}"
	echo ""
	if [ $NUM_RUNS -gt 1 ]; then
		echo "Usar Python para calcular médias das $NUM_RUNS execuções"
		echo "   python3 -c \"import json; data=json.load(open('$OUTPUT_FILE')); ...\""
	fi
	echo ""
fi
