#!/usr/bin/env python3
"""
Gera gráficos a partir do CSV de resultados aggregados do benchmark CG.
Uso: python3 plot_results.py <arquivo_csv> [--output-dir DIR]
"""

import csv
import sys
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from collections import defaultdict

plt.rcParams.update({
    'font.size': 12,
    'axes.titlesize': 14,
    'axes.labelsize': 12,
    'legend.fontsize': 10,
    'figure.figsize': (8, 5),
    'savefig.dpi': 300,
    'savefig.bbox': 'tight'
})

def load_csv(csv_file):
    """Carrega dados do CSV gerado por analyze_results.py ou analyze_results_basic.py"""
    data = {'cgSolver-naive': [], 'cgSolver': []}
    with open(csv_file, 'r') as f:
        reader = csv.DictReader(f)
        fieldnames = reader.fieldnames
        has_hardware = 'energy_core_j' in fieldnames
        
        for row in reader:
            binary = row['binario']
            row_data = {
                'tamanho': int(row['tamanho']),
                'bandas': int(row['bandas']),
                'seed': int(row['seed']),
                'iter_mean': float(row['iteracoes_media']),
            }
            
            # Carrega tempo e erro se disponíveis (CSV básico)
            if 'tempo_media_s' in fieldnames:
                row_data['tempo_mean'] = float(row['tempo_media_s'])
            else:
                row_data['tempo_mean'] = 0
                
            if 'erro_media' in fieldnames:
                row_data['erro_mean'] = float(row['erro_media'])
            else:
                row_data['erro_mean'] = 0
            
            # Carrega métricas de hardware se disponíveis
            if has_hardware:
                row_data.update({
                    'l3_request_rate': float(row.get('l3_request_rate', 0)),
                    'l3_miss_rate': float(row.get('l3_miss_rate', 0)),
                    'memory_bandwidth_mbps': float(row.get('memory_bandwidth_mbps', 0)),
                    'energy_core_j': float(row.get('energy_core_j', 0)),
                })
            else:
                row_data.update({
                    'l3_request_rate': 0,
                    'l3_miss_rate': 0,
                    'memory_bandwidth_mbps': 0,
                    'energy_core_j': 0,
                })
            
            data[binary].append(row_data)
    
    return data

def plot_tempo(data, binary_name, output_dir):
    """Plota tempo vs tamanho, agrupado por banda"""
    combined = defaultdict(list)
    for row in data[binary_name]:
        key = (row['tamanho'], row['bandas'])
        combined[key].append(row['tempo_mean'])
    
    sizes = sorted(set(k[0] for k in combined.keys()))
    for b in [7, 27]:
        means = []
        for n in sizes:
            vals = [v for (n2, b2), v in combined.items() if n2 == n and b2 == b]
            if vals:
                means.append(np.mean(vals))
            else:
                means.append(np.nan)
        plt.plot(sizes, means, marker='o', label=f'{b} bandas')
    
    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Tamanho da matriz (N)')
    plt.ylabel('Tempo (s)')
    plt.title(f'Tempo de execução - {binary_name}')
    plt.legend()
    plt.grid(True, which='both', ls='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(Path(output_dir) / f'tempo_{binary_name}.pdf')
    plt.close()

def plot_iteracoes(data, binary_name, output_dir):
    """Plota número de iterações vs tamanho"""
    combined = defaultdict(list)
    for row in data[binary_name]:
        key = (row['tamanho'], row['bandas'])
        combined[key].append(row['iter_mean'])
    
    sizes = sorted(set(k[0] for k in combined.keys()))
    for b in [7, 27]:
        means = []
        for n in sizes:
            vals = [v for (n2, b2), v in combined.items() if n2 == n and b2 == b]
            if vals:
                means.append(np.mean(vals))
            else:
                means.append(np.nan)
        plt.plot(sizes, means, marker='s', label=f'{b} bandas')
    
    plt.xscale('log')
    plt.xlabel('Tamanho da matriz (N)')
    plt.ylabel('Número de iterações')
    plt.title(f'Iterações até convergência - {binary_name}')
    plt.legend()
    plt.grid(True, which='both', ls='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(Path(output_dir) / f'iteracoes_{binary_name}.pdf')
    plt.close()

def plot_erro(data, binary_name, output_dir):
    """Plota erro relativo vs tamanho"""
    combined = defaultdict(list)
    for row in data[binary_name]:
        key = (row['tamanho'], row['bandas'])
        combined[key].append(row['erro_mean'])
    
    sizes = sorted(set(k[0] for k in combined.keys()))
    for b in [7, 27]:
        means = []
        for n in sizes:
            vals = [v for (n2, b2), v in combined.items() if n2 == n and b2 == b]
            if vals:
                means.append(np.mean(vals))
            else:
                means.append(np.nan)
        plt.plot(sizes, means, marker='^', label=f'{b} bandas')
    
    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Tamanho da matriz (N)')
    plt.ylabel('Erro relativo')
    plt.title(f'Erro relativo - {binary_name}')
    plt.legend()
    plt.grid(True, which='both', ls='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig(Path(output_dir) / f'erro_{binary_name}.pdf')
    plt.close()

def plot_hardware(data, binary_name, output_dir):
    """Plota métricas de hardware: largura de banda, L3 miss rate e energia"""
    # Verifica se há dados de hardware
    has_data = any(row['energy_core_j'] > 0 for row in data[binary_name])
    
    if not has_data:
        print(f"⚠️  Sem dados de hardware para {binary_name}, pulando plot_hardware()")
        return
    
    sizes = sorted(set(row['tamanho'] for row in data[binary_name]))
    
    for b in [7, 27]:
        bw_means = []
        l3_miss_means = []
        energy_means = []
        
        for n in sizes:
            rows = [row for row in data[binary_name] if row['tamanho'] == n and row['bandas'] == b]
            if rows:
                bw_means.append(np.mean([row['memory_bandwidth_mbps'] for row in rows]))
                l3_miss_means.append(np.mean([row['l3_miss_rate'] for row in rows]))
                energy_means.append(np.mean([row['energy_core_j'] for row in rows]))
            else:
                bw_means.append(np.nan)
                l3_miss_means.append(np.nan)
                energy_means.append(np.nan)
        
        # Largura de banda
        plt.figure()
        plt.plot(sizes, bw_means, marker='o', label=f'{b} bandas')
        plt.xscale('log')
        plt.xlabel('Tamanho da matriz (N)')
        plt.ylabel('Largura de banda (MB/s)')
        plt.title(f'Largura de banda - {binary_name}')
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(Path(output_dir) / f'bw_{binary_name}_bandas{b}.pdf')
        plt.close()
        
        # L3 miss rate
        plt.figure()
        plt.plot(sizes, l3_miss_means, marker='s', label=f'{b} bandas')
        plt.xscale('log')
        plt.xlabel('Tamanho da matriz (N)')
        plt.ylabel('L3 miss rate')
        plt.title(f'L3 Miss Rate - {binary_name}')
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(Path(output_dir) / f'l3miss_{binary_name}_bandas{b}.pdf')
        plt.close()
        
        # Energia
        plt.figure()
        plt.plot(sizes, energy_means, marker='D', label=f'{b} bandas')
        plt.xscale('log')
        plt.xlabel('Tamanho da matriz (N)')
        plt.ylabel('Energia (J)')
        plt.title(f'Energia do núcleo - {binary_name}')
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(Path(output_dir) / f'energy_{binary_name}_bandas{b}.pdf')
        plt.close()

def plot_speedup(data_naive, data_opt, output_dir):
    """Compara tempos: speedup = tempo_naive / tempo_opt"""
    speedups = []
    
    for row_naive in data_naive['cgSolver-naive']:
        key = (row_naive['tamanho'], row_naive['bandas'], row_naive['seed'])
        row_opt = next((r for r in data_opt['cgSolver'] 
                       if r['tamanho'] == key[0] and r['bandas'] == key[1] and r['seed'] == key[2]), None)
        if row_opt and row_opt['tempo_mean'] > 0:
            sp = row_naive['tempo_mean'] / row_opt['tempo_mean']
            speedups.append((row_naive['tamanho'], row_naive['bandas'], sp))
    
    if not speedups:
        print("Sem dados de speedup (versão otimizada ausente).")
        return
    
    sizes = sorted(set(s[0] for s in speedups))
    for b in [7, 27]:
        sp_vals = [sp for n, b2, sp in speedups if b2 == b]
        ns = [n for n, b2, sp in speedups if b2 == b]
        if sp_vals:
            plt.plot(ns, sp_vals, marker='D', label=f'{b} bandas')
    
    plt.xscale('log')
    plt.xlabel('Tamanho da matriz (N)')
    plt.ylabel('Speedup (naive / otimizado)')
    plt.title('Speedup da versão otimizada')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(Path(output_dir) / 'speedup.pdf')
    plt.close()

def main():
    if len(sys.argv) < 2:
        print("Uso: python3 plot_results.py <arquivo_csv> [arquivo_csv_opt] [--output-dir DIR]")
        sys.exit(1)
    
    csv_naive = sys.argv[1]
    csv_opt = None
    output_dir = "graficos"
    
    # Parse argumentos
    for i in range(2, len(sys.argv)):
        if sys.argv[i] == '--output-dir' and i+1 < len(sys.argv):
            output_dir = sys.argv[i+1]
        elif sys.argv[i].endswith('.csv') and csv_opt is None:
            csv_opt = sys.argv[i]
    
    Path(output_dir).mkdir(exist_ok=True)
    
    # Carrega dados do CSV
    print(f"Carregando dados de {csv_naive}...")
    data = load_csv(csv_naive)
    
    # Se tiver dados de naive
    if data['cgSolver-naive']:
        print("Gerando gráficos para versão naive...")
        plot_tempo(data, 'cgSolver-naive', output_dir)
        plot_iteracoes(data, 'cgSolver-naive', output_dir)
        plot_erro(data, 'cgSolver-naive', output_dir)
        plot_hardware(data, 'cgSolver-naive', output_dir)
    
    # Se tiver dados de otimizado
    if data['cgSolver']:
        print("Gerando gráficos para versão otimizada...")
        plot_tempo(data, 'cgSolver', output_dir)
        plot_iteracoes(data, 'cgSolver', output_dir)
        plot_erro(data, 'cgSolver', output_dir)
        plot_hardware(data, 'cgSolver', output_dir)
    
    # Se tiver segundo CSV para comparação
    if csv_opt:
        print(f"Carregando dados de {csv_opt} para speedup...")
        data_opt = load_csv(csv_opt)
        plot_speedup(data, data_opt, output_dir)
    
    print(f"✓ Gráficos salvos em '{output_dir}/'")


if __name__ == '__main__':
    main()

if __name__ == '__main__':
    main()