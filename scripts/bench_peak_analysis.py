#!/usr/bin/env python3
"""
Analisa bench-evolution e identifica o step do peak histórico de cada daemon
via média móvel (mais robusta que pico isolado, evita outliers de instância).

Uso:
  curl -s https://nco.patriatechnology.com/api/nco/bench-evolution > /tmp/bench.json
  python3 bench_peak_analysis.py /tmp/bench.json

Output: pra cada daemon mostra peak step + cut/n^1.5 do peak vs recente,
e quanto perdeu desde então.

Critério de decisão pra restaurar ckpt:
  Δ > 1% no recente vs peak → considerar restauração do snapshot do peak step.
  Δ < 0.5% → marginal, manter.
"""

import argparse
import json


def moving_avg_peak(snaps, window=7):
    snaps = sorted(snaps, key=lambda x: x.get("step", 0))
    if len(snaps) < window:
        best = max(snaps, key=lambda s: s["cut_mean"]) if snaps else None
        return best, best["cut_mean"] if best else 0
    best_idx, best_avg = 0, 0
    for i in range(len(snaps) - window + 1):
        win = snaps[i:i + window]
        avg = sum(s["cut_mean"] for s in win) / window
        if avg > best_avg:
            best_avg = avg
            best_idx = i + window // 2
    return snaps[best_idx], best_avg


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path", help="JSON do bench-evolution")
    ap.add_argument("--window", type=int, default=7, help="janela da média móvel")
    args = ap.parse_args()

    d = json.load(open(args.path))
    print(f"Peak histórico (média móvel janela={args.window}):\n")
    for name, snaps in d.get("daemons", {}).items():
        if not snaps:
            continue
        peak, peak_avg = moving_avg_peak(snaps, window=args.window)
        last = sorted(snaps, key=lambda x: x.get("ts", ""))[-1]
        n = peak.get("n", 0) if peak else 0
        n15 = n ** 1.5 if n else 1
        recent_w = sorted(snaps, key=lambda x: x.get("ts", ""))[-args.window:]
        recent_avg = sum(s["cut_mean"] for s in recent_w) / len(recent_w)
        delta = (recent_avg - peak_avg) / peak_avg * 100 if peak_avg else 0
        marker = "🚨 RESTAURAR" if delta < -1.0 else ("⚠️  marginal" if delta < -0.5 else "✓ ok")
        print(f"{name} (n={n}):")
        print(f"  PEAK    step≈{peak['step']:>7}  avg={peak_avg:>10.0f}  cut/n^1.5={peak_avg/n15:.4f}")
        print(f"  RECENTE step={last['step']:>7}    avg={recent_avg:>10.0f}  cut/n^1.5={recent_avg/n15:.4f}")
        print(f"  Δ recente vs peak: {delta:+.2f}%   {marker}\n")


if __name__ == "__main__":
    main()
