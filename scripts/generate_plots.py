#!/usr/bin/env python3
"""Gera gráficos de comparação de desempenho"""

import json
import csv
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import os

def load_csv(filepath):
    """Carrega dados de um arquivo CSV"""
    data = {}
    with open(filepath, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            tamanho = int(row['tamanho'])
            bandas = int(row['bandas'])
            tempo = float(row['tempo_media_s'])
            
            key = (tamanho, bandas)
            if key not in data:
                data[key] = {'tempo': tempo, 'bandas': bandas, 'tamanho': tamanho}
    
    return data

def plot_tempo_comparativo():
    """Gera gráfico de tempo de execução comparativo"""
    opt1 = load_csv("docs/results/opt1/resultados_2026-05-24T00-14-15Z_aggregated_basic.csv")
    opt2 = load_csv("docs/results/opt2/resultados_2026-05-24T18-06-55Z_aggregated.csv")
    opt3 = load_csv("docs/results/opt3/resultados_2026-05-24T19-55-36Z_aggregated_basic.csv")
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Bandas = 7
    sizes_7 = sorted([k[0] for k in opt1.keys() if k[1] == 7])
    opt1_times_7 = [opt1[(s, 7)]['tempo'] for s in sizes_7]
    opt2_times_7 = [opt2[(s, 7)]['tempo'] for s in sizes_7]
    opt3_times_7 = [opt3[(s, 7)]['tempo'] for s in sizes_7]
    
    ax1.loglog(sizes_7, opt1_times_7, 'o-', label='OPT1 (-O3)', linewidth=2)
    ax1.loglog(sizes_7, opt2_times_7, 's-', label='OPT2 (unroll)', linewidth=2)
    ax1.loglog(sizes_7, opt3_times_7, '^-', label='OPT3 (DIA)', linewidth=2)
    ax1.set_xlabel('Tamanho da matriz (n)', fontsize=12)
    ax1.set_ylabel('Tempo (s)', fontsize=12)
    ax1.set_title('Tempo de Execução - Bandas=7', fontsize=13, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend(fontsize=10)
    
    # Bandas = 27
    sizes_27 = sorted([k[0] for k in opt1.keys() if k[1] == 27])
    opt1_times_27 = [opt1[(s, 27)]['tempo'] for s in sizes_27]
    opt2_times_27 = [opt2[(s, 27)]['tempo'] for s in sizes_27]
    opt3_times_27 = [opt3[(s, 27)]['tempo'] for s in sizes_27]
    
    ax2.loglog(sizes_27, opt1_times_27, 'o-', label='OPT1 (-O3)', linewidth=2)
    ax2.loglog(sizes_27, opt2_times_27, 's-', label='OPT2 (unroll)', linewidth=2)
    ax2.loglog(sizes_27, opt3_times_27, '^-', label='OPT3 (DIA)', linewidth=2)
    ax2.set_xlabel('Tamanho da matriz (n)', fontsize=12)
    ax2.set_ylabel('Tempo (s)', fontsize=12)
    ax2.set_title('Tempo de Execução - Bandas=27', fontsize=13, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=10)
    
    plt.tight_layout()
    os.makedirs('docs/results/graficos', exist_ok=True)
    plt.savefig('docs/results/graficos/tempo_comparativo.pdf', dpi=300, bbox_inches='tight')
    print("✓ Gerado: tempo_comparativo.pdf")
    plt.close()

def plot_speedup():
    """Gera gráfico de speedup"""
    opt1 = load_csv("docs/results/opt1/resultados_2026-05-24T00-14-15Z_aggregated_basic.csv")
    opt2 = load_csv("docs/results/opt2/resultados_2026-05-24T18-06-55Z_aggregated.csv")
    opt3 = load_csv("docs/results/opt3/resultados_2026-05-24T19-58-08Z_aggregated.csv")
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Bandas = 7
    sizes_7 = sorted([k[0] for k in opt1.keys() if k[1] == 7])
    opt1_times_7 = np.array([opt1[(s, 7)]['tempo'] for s in sizes_7])
    opt2_times_7 = np.array([opt2[(s, 7)]['tempo'] for s in sizes_7])
    opt3_times_7 = np.array([opt3[(s, 7)]['tempo'] for s in sizes_7])
    
    speedup_opt1_7 = opt1_times_7 / opt1_times_7
    speedup_opt2_7 = opt1_times_7 / opt2_times_7
    speedup_opt3_7 = opt1_times_7 / opt3_times_7
    
    ax1.semilogx(sizes_7, speedup_opt1_7, 'o-', label='OPT1 (baseline)', linewidth=2)
    ax1.semilogx(sizes_7, speedup_opt2_7, 's-', label='OPT2', linewidth=2)
    ax1.semilogx(sizes_7, speedup_opt3_7, '^-', label='OPT3', linewidth=2)
    ax1.axhline(y=1, color='k', linestyle='--', alpha=0.3)
    ax1.set_xlabel('Tamanho da matriz (n)', fontsize=12)
    ax1.set_ylabel('Speedup', fontsize=12)
    ax1.set_title('Speedup (vs OPT1) - Bandas=7', fontsize=13, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend(fontsize=10)
    
    # Bandas = 27
    sizes_27 = sorted([k[0] for k in opt1.keys() if k[1] == 27])
    opt1_times_27 = np.array([opt1[(s, 27)]['tempo'] for s in sizes_27])
    opt2_times_27 = np.array([opt2[(s, 27)]['tempo'] for s in sizes_27])
    opt3_times_27 = np.array([opt3[(s, 27)]['tempo'] for s in sizes_27])
    
    speedup_opt1_27 = opt1_times_27 / opt1_times_27
    speedup_opt2_27 = opt1_times_27 / opt2_times_27
    speedup_opt3_27 = opt1_times_27 / opt3_times_27
    
    ax2.semilogx(sizes_27, speedup_opt1_27, 'o-', label='OPT1 (baseline)', linewidth=2)
    ax2.semilogx(sizes_27, speedup_opt2_27, 's-', label='OPT2', linewidth=2)
    ax2.semilogx(sizes_27, speedup_opt3_27, '^-', label='OPT3', linewidth=2)
    ax2.axhline(y=1, color='k', linestyle='--', alpha=0.3)
    ax2.set_xlabel('Tamanho da matriz (n)', fontsize=12)
    ax2.set_ylabel('Speedup', fontsize=12)
    ax2.set_title('Speedup (vs OPT1) - Bandas=27', fontsize=13, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=10)
    
    plt.tight_layout()
    plt.savefig('docs/results/graficos/speedup_comparativo.pdf', dpi=300, bbox_inches='tight')
    print("✓ Gerado: speedup_comparativo.pdf")
    plt.close()

def plot_bandwidth():
    """Gera gráfico de largura de banda"""
    # Carregar dados com hardware (Likwid)
    def load_hardware_csv(filepath):
        data = {}
        with open(filepath, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                tamanho = int(row['tamanho'])
                bandas = int(row['bandas'])
                bw = float(row['memory_bandwidth_mbps'])
                
                key = (tamanho, bandas)
                if key not in data:
                    data[key] = {'bw': bw, 'tamanho': tamanho, 'bandas': bandas}
        return data
    
    opt1_hw = load_hardware_csv("docs/results/opt1/resultados_2026-05-24T00-16-48Z_aggregated.csv")
    opt2_hw = load_hardware_csv("docs/results/opt2/resultados_2026-05-24T18-09-29Z_aggregated.csv")
    opt3_hw = load_hardware_csv("docs/results/opt3/resultados_2026-05-24T19-58-08Z_aggregated.csv")
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))
    
    # Bandas = 7 (apenas maiores tamanhos com dados confiáveis)
    sizes_7 = sorted([k[0] for k in opt1_hw.keys() if k[1] == 7 and k[0] >= 65536])
    if sizes_7:
        opt1_bw_7 = [opt1_hw[(s, 7)]['bw'] for s in sizes_7]
        opt2_bw_7 = [opt2_hw[(s, 7)]['bw'] for s in sizes_7]
        opt3_bw_7 = [opt3_hw[(s, 7)]['bw'] for s in sizes_7]
        
        ax1.loglog(sizes_7, opt1_bw_7, 'o-', label='OPT1 (-O3)', linewidth=2, markersize=8)
        ax1.loglog(sizes_7, opt2_bw_7, 's-', label='OPT2 (unroll)', linewidth=2, markersize=8)
        ax1.loglog(sizes_7, opt3_bw_7, '^-', label='OPT3 (DIA)', linewidth=2, markersize=8)
        ax1.set_xlabel('Tamanho da matriz (n)', fontsize=12)
        ax1.set_ylabel('Largura de banda (MB/s)', fontsize=12)
        ax1.set_title('Largura de Banda - Bandas=7', fontsize=13, fontweight='bold')
        ax1.grid(True, alpha=0.3, which='both')
        ax1.legend(fontsize=10)
    
    # Bandas = 27
    sizes_27 = sorted([k[0] for k in opt1_hw.keys() if k[1] == 27 and k[0] >= 65536])
    if sizes_27:
        opt1_bw_27 = [opt1_hw[(s, 27)]['bw'] for s in sizes_27]
        opt2_bw_27 = [opt2_hw[(s, 27)]['bw'] for s in sizes_27]
        opt3_bw_27 = [opt3_hw[(s, 27)]['bw'] for s in sizes_27]
        
        ax2.loglog(sizes_27, opt1_bw_27, 'o-', label='OPT1 (-O3)', linewidth=2, markersize=8)
        ax2.loglog(sizes_27, opt2_bw_27, 's-', label='OPT2 (unroll)', linewidth=2, markersize=8)
        ax2.loglog(sizes_27, opt3_bw_27, '^-', label='OPT3 (DIA)', linewidth=2, markersize=8)
        ax2.set_xlabel('Tamanho da matriz (n)', fontsize=12)
        ax2.set_ylabel('Largura de banda (MB/s)', fontsize=12)
        ax2.set_title('Largura de Banda - Bandas=27', fontsize=13, fontweight='bold')
        ax2.grid(True, alpha=0.3, which='both')
        ax2.legend(fontsize=10)
    
    plt.tight_layout()
    plt.savefig('docs/results/graficos/bw_comparativo.pdf', dpi=300, bbox_inches='tight')
    print("✓ Gerado: bw_comparativo.pdf")
    plt.close()

if __name__ == '__main__':
    try:
        plot_tempo_comparativo()
        plot_speedup()
        plot_bandwidth()
        print("\n✓ Todos os gráficos foram gerados com sucesso!")
    except Exception as e:
        print(f"Erro ao gerar gráficos: {e}")
        import traceback
        traceback.print_exc()
