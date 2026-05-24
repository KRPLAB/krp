#!/usr/bin/env python3

"""
Script para agregar resultados COM MÉTRICAS DE HARDWARE (Likwid).
Gera tabelas em múltiplos formatos (CSV, Markdown, LaTeX).

Uso:
    python3 analyze_results.py <arquivo_json>
    python3 analyze_results.py resultados_2026-05-23T19-33-42Z.json
"""

import json
import sys
import statistics
from pathlib import Path
from collections import defaultdict
from typing import Dict, List, Any

class ResultsAnalyzer:
    def __init__(self, json_file: str):
        self.json_file = json_file
        self.data = self._load_json()
        
    def _load_json(self) -> Dict:
        """Carrega arquivo JSON"""
        with open(self.json_file, 'r') as f:
            return json.load(f)
    
    def aggregate_results(self) -> Dict:
        """Agrega estatísticas de múltiplas execuções"""
        aggregated = {}
        
        for binary_name, configs in self.data['results'].items():
            aggregated[binary_name] = []
            
            for config_group in configs:
                config = config_group['config']
                executions = config_group['executions']
                
                tempos = []
                iteracoes = []
                erros = []
                l3_reqs = []
                l3_misses = []
                mem_bws = []
                energies = []
                
                for exec_data in executions:
                    if exec_data['performance'] is not None:
                        perf = exec_data['performance']
                        tempos.append(perf['tempo_s'])
                        iteracoes.append(perf['iteracoes'])
                        erros.append(perf['erro_relativo'])
                    
                    if exec_data.get('hardware_metrics') is not None:
                        hw = exec_data['hardware_metrics']
                        if hw.get('l3_request_rate') and hw['l3_request_rate'] != 0:
                            l3_reqs.append(float(hw['l3_request_rate']))
                        if hw.get('l3_miss_rate') and hw['l3_miss_rate'] != 0:
                            l3_misses.append(float(hw['l3_miss_rate']))
                        if hw.get('memory_bandwidth_mbps') and hw['memory_bandwidth_mbps'] != 0:
                            mem_bws.append(float(hw['memory_bandwidth_mbps']))
                        if hw.get('energy_core_j') and hw['energy_core_j'] != 0:
                            energies.append(float(hw['energy_core_j']))
                
                if not tempos:
                    continue
                
                stats = {
                    'config': config,
                    'num_runs': len(tempos),
                    'tempo': {
                        'media': statistics.mean(tempos),
                        'stdev': statistics.stdev(tempos) if len(tempos) > 1 else 0,
                    },
                    'iteracoes': {
                        'media': statistics.mean(iteracoes),
                    },
                    'erro': {
                        'media': statistics.mean(erros),
                    },
                    'hardware': {
                        'l3_request_rate': statistics.mean(l3_reqs) if l3_reqs else 0,
                        'l3_miss_rate': statistics.mean(l3_misses) if l3_misses else 0,
                        'memory_bandwidth_mbps': statistics.mean(mem_bws) if mem_bws else 0,
                        'energy_core_j': statistics.mean(energies) if energies else 0,
                    }
                }
                
                aggregated[binary_name].append(stats)
        
        return aggregated
    
    def generate_csv(self, aggregated: Dict, output_file: str = None):
        """Gera CSV com métricas de hardware (sem num_runs)"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_aggregated.csv')
        
        with open(output_file, 'w') as f:
            f.write("binario,tamanho,bandas,seed,tempo_media_s,iteracoes_media,erro_media,l3_request_rate,l3_miss_rate,memory_bandwidth_mbps,energy_core_j\n")
            
            for binary_name, results in aggregated.items():
                for result in results:
                    config = result['config']
                    hw = result['hardware']
                    f.write(f"{binary_name},"
                            f"{config['tamanho']},"
                            f"{config['bandas']},"
                            f"{config['seed']},"
                            f"{result['tempo']['media']:.6e},"
                            f"{result['iteracoes']['media']:.1f},"
                            f"{result['erro']['media']:.6e},"
                            f"{hw['l3_request_rate']:.6f},"
                            f"{hw['l3_miss_rate']:.6f},"
                            f"{hw['memory_bandwidth_mbps']:.2f},"
                            f"{hw['energy_core_j']:.6f}\n")
        
        return output_file
    
    def generate_latex(self, aggregated: Dict, output_file: str = None):
        """Gera tabelas em LaTeX com métricas de hardware (sem Tempo, Erro, Rodadas, Desvio)"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_tables.tex')
        
        with open(output_file, 'w') as f:
            # Tabela Naive com Hardware
            f.write("% Resultados - Versão NAIVE com Hardware\n")
            f.write("\\begin{table}[H]\n")
            f.write("\\centering\n")
            f.write("\\caption{Resultados - Versão NAIVE (com métricas de hardware)}\n")
            f.write("\\label{tab:naive_hw}\n")
            f.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n")
            f.write("\\hline\n")
            f.write("Tamanho & Bandas & Seed & Iter. & L3R & L3M & BW (MB/s) & Energia (J) \\\\\n")
            f.write("\\hline\n")
            
            for result in aggregated.get('cgSolver-naive', []):
                config = result['config']
                hw = result['hardware']
                f.write(f"{config['tamanho']:,} & "
                        f"{config['bandas']} & "
                        f"{config['seed']} & "
                        f"{result['iteracoes']['media']:.0f} & "
                        f"{hw['l3_request_rate']:.4f} & "
                        f"{hw['l3_miss_rate']:.4f} & "
                        f"{hw['memory_bandwidth_mbps']:.2f} & "
                        f"{hw['energy_core_j']:.2f} \\\\\n")
            
            f.write("\\hline\n")
            f.write("\\end{tabular}\n")
            f.write("\\end{table}\n\n")
            
            # Tabela Otimizada com Hardware
            f.write("% Resultados - Versão OTIMIZADA com Hardware\n")
            f.write("\\begin{table}[H]\n")
            f.write("\\centering\n")
            f.write("\\caption{Resultados - Versão OTIMIZADA (com métricas de hardware)}\n")
            f.write("\\label{tab:otimizado_hw}\n")
            f.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n")
            f.write("\\hline\n")
            f.write("Tamanho & Bandas & Seed & Iter. & L3R & L3M & BW (MB/s) & Energia (J) \\\\\n")
            f.write("\\hline\n")
            
            for result in aggregated.get('cgSolver', []):
                config = result['config']
                hw = result['hardware']
                f.write(f"{config['tamanho']:,} & "
                        f"{config['bandas']} & "
                        f"{config['seed']} & "
                        f"{result['iteracoes']['media']:.0f} & "
                        f"{hw['l3_request_rate']:.4f} & "
                        f"{hw['l3_miss_rate']:.4f} & "
                        f"{hw['memory_bandwidth_mbps']:.2f} & "
                        f"{hw['energy_core_j']:.2f} \\\\\n")
            
            f.write("\\hline\n")
            f.write("\\end{tabular}\n")
            f.write("\\end{table}\n")
        
        return output_file
    
    def print_summary(self, aggregated: Dict):
        """Imprime resumo no console"""
        print("\n" + "="*120)
        print("RESUMO DA ANÁLISE - COM MÉTRICAS DE HARDWARE")
        print("="*120 + "\n")
        
        print(f"Arquivo: {self.json_file}")
        print(f"Número de rodadas: {self.data['metadata']['num_runs']}")
        print(f"Likwid habilitado: {self.data['metadata']['likwid_enabled']}\n")
        
        for binary_name in ['cgSolver-naive', 'cgSolver']:
            results = aggregated.get(binary_name, [])
            if not results:
                continue
            
            print(f"\n{binary_name}:")
            print("-" * 120)
            for result in results[:3]:
                config = result['config']
                hw = result['hardware']
                print(f"  N={config['tamanho']:>7} | B={config['bandas']} | Seed={config['seed']} | "
                      f"T={result['tempo']['media']:.3e}s ± {result['tempo']['stdev']:.3e}s | "
                      f"BW={hw['memory_bandwidth_mbps']:.0f} MB/s | "
                      f"Energy={hw['energy_core_j']:.2f}J")


def main():
    if len(sys.argv) < 2:
        print("Uso: python3 analyze_results.py <arquivo_json>")
        sys.exit(1)
    
    json_file = sys.argv[1]
    
    if not Path(json_file).exists():
        print(f"❌ Erro: Arquivo '{json_file}' não encontrado")
        sys.exit(1)
    
    print(f"📊 Analisando (COM hardware): {json_file}")
    
    analyzer = ResultsAnalyzer(json_file)
    aggregated = analyzer.aggregate_results()
    
    csv_file = analyzer.generate_csv(aggregated)
    tex_file = analyzer.generate_latex(aggregated)
    
    analyzer.print_summary(aggregated)
    
    print(f"\n✓ CSV gerado: {csv_file}")
    print(f"✓ LaTeX gerado: {tex_file}\n")


if __name__ == '__main__':
    main()
