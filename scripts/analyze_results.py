#!/usr/bin/env python3

"""
Script para agregar e analisar resultados dos testes em JSON.
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
                
                # Extrai métricas de todas as execuções
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
                
                # Calcula estatísticas
                stats = {
                    'config': config,
                    'num_runs': len(tempos),
                    'tempo': {
                        'media': statistics.mean(tempos),
                        'stdev': statistics.stdev(tempos) if len(tempos) > 1 else 0,
                        'min': min(tempos),
                        'max': max(tempos),
                    },
                    'iteracoes': {
                        'media': statistics.mean(iteracoes),
                        'stdev': statistics.stdev(iteracoes) if len(iteracoes) > 1 else 0,
                    },
                    'erro': {
                        'media': statistics.mean(erros),
                        'min': min(erros),
                        'max': max(erros),
                    }
                }
                
                aggregated[binary_name].append(stats)
        
        return aggregated
    
    def compute_speedup(self, aggregated: Dict) -> Dict:
        """Calcula speedup entre versões naive e otimizada"""
        speedup = defaultdict(dict)
        
        # Organiza por config
        naive_data = {(r['config']['tamanho'], r['config']['bandas'], r['config']['seed']): r 
                      for r in aggregated.get('cgSolver-naive', [])}
        opt_data = {(r['config']['tamanho'], r['config']['bandas'], r['config']['seed']): r 
                    for r in aggregated.get('cgSolver', [])}
        
        for config_key in naive_data:
            if config_key in opt_data:
                n_time = naive_data[config_key]['tempo']['media']
                o_time = opt_data[config_key]['tempo']['media']
                if o_time > 0:
                    speedup[config_key] = n_time / o_time
        
        return speedup
    
    def generate_csv(self, aggregated: Dict, output_file: str = None):
        """Gera CSV consolidado"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_aggregated.csv')
        
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
    
    def generate_markdown(self, aggregated: Dict, speedup: Dict, output_file: str = None):
        """Gera tabelas em Markdown"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_report.md')
        
        with open(output_file, 'w') as f:
            f.write("# Relatório de Análise de Resultados\n\n")
            f.write(f"**Arquivo de entrada:** `{self.json_file}`\n")
            f.write(f"**Número de rodadas:** {self.data['metadata']['num_runs']}\n\n")
            
            # Tabela Naive
            f.write("## Resultados - Versão NAIVE\n\n")
            f.write("|Tamanho|Bandas|Seed|Rodadas|Tempo (s)|Desvio|Iterações|Erro Relativo|\n")
            f.write("|-------|------|----|----|---------|------|---------|-------------|\n")
            
            for result in aggregated.get('cgSolver-naive', []):
                config = result['config']
                f.write(f"|{config['tamanho']:,}|"
                        f"{config['bandas']}|"
                        f"{config['seed']}|"
                        f"{result['num_runs']}|"
                        f"{result['tempo']['media']:.6e}|"
                        f"{result['tempo']['stdev']:.6e}|"
                        f"{result['iteracoes']['media']:.0f}|"
                        f"{result['erro']['media']:.6e}|\n")
            
            # Tabela Otimizada
            f.write("\n## Resultados - Versão OTIMIZADA\n\n")
            f.write("|Tamanho|Bandas|Seed|Rodadas|Tempo (s)|Desvio|Iterações|Erro Relativo|\n")
            f.write("|-------|------|----|----|---------|------|---------|-------------|\n")
            
            for result in aggregated.get('cgSolver', []):
                config = result['config']
                f.write(f"|{config['tamanho']:,}|"
                        f"{config['bandas']}|"
                        f"{config['seed']}|"
                        f"{result['num_runs']}|"
                        f"{result['tempo']['media']:.6e}|"
                        f"{result['tempo']['stdev']:.6e}|"
                        f"{result['iteracoes']['media']:.0f}|"
                        f"{result['erro']['media']:.6e}|\n")
            
            # Tabela de Speedup
            if speedup:
                f.write("\n## Speedup (Naive / Otimizado)\n\n")
                f.write("|Tamanho|Bandas|Seed|Speedup|\n")
                f.write("|-------|------|----|----|\n")
                
                for (tamanho, bandas, seed), sp in sorted(speedup.items()):
                    f.write(f"|{tamanho:,}|{bandas}|{seed}|{sp:.2f}x|\n")
        
        return output_file
    
    def generate_latex(self, aggregated: Dict, speedup: Dict, output_file: str = None):
        """Gera tabelas em LaTeX"""
        if output_file is None:
            output_file = self.json_file.replace('.json', '_tables.tex')
        
        with open(output_file, 'w') as f:
            # Tabela Naive
            f.write("% Resultados - Versão NAIVE\n")
            f.write("\\begin{table}[h]\n")
            f.write("\\centering\n")
            f.write("\\caption{Resultados - Versão NAIVE}\n")
            f.write("\\label{tab:naive}\n")
            f.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n")
            f.write("\\hline\n")
            f.write("Tamanho & Bandas & Seed & Rodadas & Tempo (s) & Desvio & Iterações & Erro \\\\\n")
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
            f.write("% Resultados - Versão OTIMIZADA\n")
            f.write("\\begin{table}[h]\n")
            f.write("\\centering\n")
            f.write("\\caption{Resultados - Versão OTIMIZADA}\n")
            f.write("\\label{tab:otimizado}\n")
            f.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|}\n")
            f.write("\\hline\n")
            f.write("Tamanho & Bandas & Seed & Rodadas & Tempo (s) & Desvio & Iterações & Erro \\\\\n")
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
            f.write("\\end{table}\n\n")
            
            # Tabela de Speedup
            if speedup:
                f.write("% Speedup\n")
                f.write("\\begin{table}[h]\n")
                f.write("\\centering\n")
                f.write("\\caption{Speedup: Naive / Otimizado}\n")
                f.write("\\label{tab:speedup}\n")
                f.write("\\begin{tabular}{|c|c|c|c|}\n")
                f.write("\\hline\n")
                f.write("Tamanho & Bandas & Seed & Speedup \\\\\n")
                f.write("\\hline\n")
                
                for (tamanho, bandas, seed), sp in sorted(speedup.items()):
                    f.write(f"{tamanho:,} & {bandas} & {seed} & {sp:.2f}\\times \\\\\n")
                
                f.write("\\hline\n")
                f.write("\\end{tabular}\n")
                f.write("\\end{table}\n")
        
        return output_file
    
    def print_summary(self, aggregated: Dict, speedup: Dict):
        """Imprime resumo no console"""
        print("\n" + "="*80)
        print("RESUMO DA ANÁLISE")
        print("="*80 + "\n")
        
        print(f"Arquivo: {self.json_file}")
        print(f"Número de rodadas: {self.data['metadata']['num_runs']}")
        print(f"Likwid habilitado: {self.data['metadata']['likwid_enabled']}\n")
        
        for binary_name in ['cgSolver-naive', 'cgSolver']:
            results = aggregated.get(binary_name, [])
            if not results:
                continue
            
            print(f"\n{binary_name}:")
            print("-" * 80)
            for result in results[:3]:  # Mostra primeiros 3
                config = result['config']
                print(f"  N={config['tamanho']:>7} | Bandas={config['bandas']} | "
                      f"Tempo={result['tempo']['media']:.6e}s "
                      f"(±{result['tempo']['stdev']:.6e}s)")
        
        if speedup:
            print(f"\n\nSpeedup (Naive / Otimizado):")
            print("-" * 80)
            speedup_values = list(speedup.values())
            print(f"  Média: {statistics.mean(speedup_values):.2f}x")
            print(f"  Min:   {min(speedup_values):.2f}x")
            print(f"  Max:   {max(speedup_values):.2f}x")


def main():
    if len(sys.argv) < 2:
        print("Uso: python3 analyze_results.py <arquivo_json>")
        sys.exit(1)
    
    json_file = sys.argv[1]
    
    if not Path(json_file).exists():
        print(f"❌ Erro: Arquivo '{json_file}' não encontrado")
        sys.exit(1)
    
    print(f"📊 Analisando: {json_file}")
    
    analyzer = ResultsAnalyzer(json_file)
    aggregated = analyzer.aggregate_results()
    speedup = analyzer.compute_speedup(aggregated)
    
    # Gera saídas
    csv_file = analyzer.generate_csv(aggregated)
    md_file = analyzer.generate_markdown(aggregated, speedup)
    tex_file = analyzer.generate_latex(aggregated, speedup)
    
    # Resumo
    analyzer.print_summary(aggregated, speedup)
    
    print(f"\n✓ CSV gerado: {csv_file}")
    print(f"✓ Markdown gerado: {md_file}")
    print(f"✓ LaTeX gerado: {tex_file}")
    print("\n")


if __name__ == '__main__':
    main()
