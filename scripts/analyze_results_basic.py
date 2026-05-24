#!/usr/bin/env python3

"""
Script para agregar resultados SEM MÉTRICAS DE HARDWARE.
Gera apenas tabelas com tempo, iterações, erro relativo, rodadas e desvio.

Uso:
    python3 analyze_results_basic.py <arquivo_json>
"""

import json
import sys
import statistics
from pathlib import Path
from typing import Dict

class ResultsAnalyzerBasic:
    def __init__(self, json_file: str):
        self.json_file = json_file
        self.data = self._load_json()
        
    def _load_json(self) -> Dict:
        with open(self.json_file, 'r') as f:
            return json.load(f)
    
    def aggregate_results(self) -> Dict:
        """Agrega estatísticas de múltiplas execuções (sem hardware)"""
        aggregated = {}
        
        for binary_name, configs in self.data['results'].items():
            aggregated[binary_name] = []
            
            for config_group in configs:
                config = config_group['config']
                executions = config_group['executions']
                
                tempos = []
                iteracoes = []
                erros = []
                
                for exec_data in executions:
                    if exec_data['performance'] is not None:
                        perf = exec_data['performance']
                        tempos.append(perf['tempo_s'])
                        iteracoes.append(perf['iteracoes'])
                        erros.append(perf['erro_relativo'])
                
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
                    }
                }
                
                aggregated[binary_name].append(stats)
        
        return aggregated
    
    def generate_csv(self, aggregated: Dict, output_file: str = None):
        """Gera CSV básico com colunas: binario,tamanho,bandas,seed,num_runs,tempo_media_s,tempo_stdev_s,iteracoes_media,erro_media"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_aggregated_basic.csv')
        
        with open(output_file, 'w') as f:
            f.write("binario,tamanho,bandas,seed,num_runs,tempo_media_s,tempo_stdev_s,iteracoes_media,erro_media\n")
            
            for binary_name, results in aggregated.items():
                for result in results:
                    config = result['config']
                    f.write(f"{binary_name},"
                            f"{config['tamanho']},"
                            f"{config['bandas']},"
                            f"{config['seed']},"
                            f"{result['num_runs']},"
                            f"{result['tempo']['media']:.6e},"
                            f"{result['tempo']['stdev']:.6e},"
                            f"{result['iteracoes']['media']:.1f},"
                            f"{result['erro']['media']:.6e}\n")
        
        return output_file
    
    def generate_latex(self, aggregated: Dict, output_file: str = None):
        """Gera tabelas em LaTeX sem hardware, com colunas:
        Tamanho | Bandas | Seed | Rodadas | Tempo (s) | Desvio | Iter. | Erro Relativo"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_tables_basic.tex')
        
        with open(output_file, 'w') as f:
            # Tabela Naive
            f.write("% Resultados - Versão NAIVE (sem hardware)\n")
            f.write("\\begin{table}[H]\n")
            f.write("\\centering\n")
            f.write("\\caption{Resultados - Versão NAIVE}\n")
            f.write("\\label{tab:naive}\n")
            f.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n")
            f.write("\\hline\n")
            f.write("Tamanho & Bandas & Seed & Rodadas & Tempo (s) & Desvio & Iter. & Erro Relativo \\\\\n")
            f.write("\\hline\n")
            
            for result in aggregated.get('cgSolver-naive', []):
                config = result['config']
                f.write(f"{config['tamanho']:,} & "
                        f"{config['bandas']} & "
                        f"{config['seed']} & "
                        f"{result['num_runs']} & "
                        f"\\num{{{result['tempo']['media']:.3e}}} & "
                        f"\\num{{{result['tempo']['stdev']:.3e}}} & "
                        f"{result['iteracoes']['media']:.0f} & "
                        f"\\num{{{result['erro']['media']:.3e}}} \\\\\n")
            
            f.write("\\hline\n")
            f.write("\\end{tabular}\n")
            f.write("\\end{table}\n\n")
            
            # Tabela Otimizada
            f.write("% Resultados - Versão OTIMIZADA (sem hardware)\n")
            f.write("\\begin{table}[H]\n")
            f.write("\\centering\n")
            f.write("\\caption{Resultados - Versão OTIMIZADA}\n")
            f.write("\\label{tab:otimizado}\n")
            f.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n")
            f.write("\\hline\n")
            f.write("Tamanho & Bandas & Seed & Rodadas & Tempo (s) & Desvio & Iter. & Erro Relativo \\\\\n")
            f.write("\\hline\n")
            
            for result in aggregated.get('cgSolver', []):
                config = result['config']
                f.write(f"{config['tamanho']:,} & "
                        f"{config['bandas']} & "
                        f"{config['seed']} & "
                        f"{result['num_runs']} & "
                        f"\\num{{{result['tempo']['media']:.3e}}} & "
                        f"\\num{{{result['tempo']['stdev']:.3e}}} & "
                        f"{result['iteracoes']['media']:.0f} & "
                        f"\\num{{{result['erro']['media']:.3e}}} \\\\\n")
            
            f.write("\\hline\n")
            f.write("\\end{tabular}\n")
            f.write("\\end{table}\n")
        
        return output_file
    
    def print_summary(self, aggregated: Dict):
        print("\n" + "="*100)
        print("RESUMO DA ANÁLISE - SEM MÉTRICAS DE HARDWARE")
        print("="*100 + "\n")
        print(f"Arquivo: {self.json_file}")
        print(f"Número de rodadas: {self.data['metadata']['num_runs']}\n")
        
        for binary_name in ['cgSolver-naive', 'cgSolver']:
            results = aggregated.get(binary_name, [])
            if not results:
                continue
            print(f"\n{binary_name}:")
            print("-" * 100)
            for result in results[:3]:
                config = result['config']
                print(f"  N={config['tamanho']:>7} | B={config['bandas']} | Seed={config['seed']} | "
                      f"T={result['tempo']['media']:.3e}s ± {result['tempo']['stdev']:.3e}s | "
                      f"Iter.={result['iteracoes']['media']:.0f}")


def main():
    if len(sys.argv) < 2:
        print("Uso: python3 analyze_results_basic.py <arquivo_json>")
        sys.exit(1)
    
    json_file = sys.argv[1]
    
    if not Path(json_file).exists():
        print(f"❌ Erro: Arquivo '{json_file}' não encontrado")
        sys.exit(1)
    
    print(f"📊 Analisando (SEM hardware): {json_file}")
    
    analyzer = ResultsAnalyzerBasic(json_file)
    aggregated = analyzer.aggregate_results()
    
    csv_file = analyzer.generate_csv(aggregated)
    tex_file = analyzer.generate_latex(aggregated)
    
    analyzer.print_summary(aggregated)
    
    print(f"\n✓ CSV gerado: {csv_file}")
    print(f"✓ LaTeX gerado: {tex_file}\n")


if __name__ == '__main__':
    main()