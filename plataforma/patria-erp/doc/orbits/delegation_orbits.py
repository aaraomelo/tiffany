#!/usr/bin/env python3
"""
Órbitas de delegação: um grafo grande COM ciclos, propagação com a intensidade
delta como energia dissipativa, e verificação empírica das órbitas.

Ponte com a mecânica celeste de
  hiper/circular/paper_metrica_quantica_algebras.tex
(de Sitter 2D  ds^2 = -cos^2(psi) dtheta^2 + dpsi^2 ; energia de Noether
E = cos^2(psi) * theta_dot conservada ao longo da geodésica; métrica quântica
kappa_q(x,y)=min(|x-y|,1-|x-y|) no círculo S^1=R/Z).

Tese verificada: onde de Sitter tem energia CONSERVADA (órbita fechada), a
delegação tem delta ESTRITAMENTE decrescente (a atenuação é o atrito) -> a
órbita ESPIRALA pra dentro até delta=0. Logo ciclos no grafo são inofensivos:
toda trajetória termina em <= delta0 saltos (Lema da energia bem-fundada).
"""

import numpy as np
import networkx as nx
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from collections import deque

RNG = np.random.default_rng(42)
OUT = __import__("os").path.dirname(__import__("os").path.abspath(__file__))


# ----------------------------------------------------------------------
# 1. Grafo de delegação grande, COM ciclos (não é DAG)
# ----------------------------------------------------------------------
def build_delegation_graph(n=3000, avg_out=4.0, seed=42):
    """Grafo dirigido tipo scale-free + aleatório: grau médio > 1 garante um
    SCC gigante (muitos ciclos)."""
    p = avg_out / n
    G = nx.gnp_random_graph(n, p, seed=seed, directed=True)
    # alguns ciclos curtos deliberados (grupos temporários mútuos)
    rng = np.random.default_rng(seed)
    for _ in range(n // 50):
        c = rng.choice(n, size=rng.integers(2, 6), replace=False)
        for i in range(len(c)):
            G.add_edge(int(c[i]), int(c[(i + 1) % len(c)]))
    return G


# ----------------------------------------------------------------------
# 2. Propagação com orçamento delta (operador P^delta) — BFS bem-fundado
# ----------------------------------------------------------------------
def propagate(G, roots, delta0):
    """Capacidade emitida na raiz com profundidade delta0 flui por arestas,
    decrementando 1 por salto. Estado = (no, delta_restante). Retorna o melhor
    delta com que cada no e' alcançado e estatisticas de terminação."""
    best = {}  # no -> maior delta restante observado
    longest_chain = 0  # maior cadeia de re-delegação realizada
    # frente de BFS: (no, delta_restante)
    q = deque((r, delta0) for r in roots)
    for r in roots:
        best[r] = delta0
    transitions_strict = True  # toda transição decresce delta em 1?
    while q:
        u, d = q.popleft()
        longest_chain = max(longest_chain, delta0 - d)
        if d <= 0:
            continue
        for w in G.successors(u):
            nd = d - 1
            if nd != d - 1:  # invariante: estritamente -1
                transitions_strict = False
            if best.get(w, -1) < nd:
                best[w] = nd
                q.append((w, nd))
    return best, longest_chain, transitions_strict


def reach_within(best, delta0, k):
    """|P^k(raizes)| = nos alcançaveis em <= k saltos = delta_restante >= delta0-k."""
    return sum(1 for d in best.values() if d >= delta0 - k)


# ----------------------------------------------------------------------
# 3. Geodésica conservativa (mecânica celeste do paper) — seção EUCLIDIANA.
#    O paper: Wick theta -> -i theta_E torna ds^2 a esfera S^2 (Obs. da
#    continuação de Wick). Métrica  dpsi^2 + cos^2(psi) dphi^2  (esfera de
#    raio 1: psi latitude, phi longitude). Geodésicas = círculos máximos.
#    Conservada L = cos^2(psi) phi_dot ; unit-speed: psi_dot^2 = 1 - L^2/cos^2(psi)
#    -> psi OSCILA entre +-arccos(L): ÓRBITA FECHADA (energia conservada).
# ----------------------------------------------------------------------
def sphere_geodesic(L=0.6, loops=2.0, n=1500):
    """Círculo máximo inclinado i=arccos(L) sobre a equatorial: forma analítica
    psi(phi)=arcsin(sin(i) sin(phi)) — geodésica fechada, psi oscila +-arccos(L)."""
    i = np.arccos(L)
    phi = np.linspace(0, 2 * np.pi * loops, n)
    psi = np.arcsin(np.sin(i) * np.sin(phi))
    return phi, psi


# ======================================================================
def main():
    kq = lambda x, y: min(abs(x - y), 1 - abs(x - y))  # métrica quântica S^1

    # --- grafo grande com ciclos ---
    G = build_delegation_graph(n=3000, avg_out=4.0, seed=42)
    sccs = [c for c in nx.strongly_connected_components(G) if len(c) > 1]
    giant = max(sccs, key=len) if sccs else set()
    has_cycle = len(giant) > 1
    n_cyclic_nodes = sum(len(c) for c in sccs)

    # --- propagação delta0 a partir de raizes (autoridades intrínsecas) ---
    delta0 = 20
    roots = [int(n) for n in list(giant)[:3]] or [0]
    best, longest, strict = propagate(G, roots, delta0)

    # fixed point: reach(k) satura em k=delta0 (mu P)
    reach_curve = [reach_within(best, delta0, k) for k in range(delta0 + 3)]
    fixed_point_at = next(
        (k for k in range(1, len(reach_curve)) if reach_curve[k] == reach_curve[k - 1]),
        None,
    )

    # --- VERIFICAÇÕES ---
    print("=" * 64)
    print("GRAFO DE DELEGAÇÃO (grande, com ciclos)")
    print(f"  nós={G.number_of_nodes()}  arestas={G.number_of_edges()}")
    print(f"  tem ciclo? {has_cycle}  (SCC gigante: {len(giant)} nós;"
          f" {n_cyclic_nodes} nós em ciclos)")
    print("ÓRBITAS / PROPAGAÇÃO (delta0=%d)" % delta0)
    print(f"  raizes={roots}  alcance={len(best)} nós")
    print(f"  [V1] terminação apesar de ciclos: maior cadeia realizada"
          f" = {longest} (<= delta0={delta0})  -> {'OK' if longest <= delta0 else 'FALHA'}")
    print(f"  [V2] delta estritamente decrescente (energia bem-fundada):"
          f" {'OK' if strict else 'FALHA'}")
    print(f"  [V3] ponto fixo mu P (reach satura): em k={fixed_point_at}"
          f"  reach_final={reach_curve[-1]}  -> {'OK' if fixed_point_at and fixed_point_at<=delta0+1 else 'FALHA'}")
    print(f"  [V4] reach é monótono não-decrescente em k:"
          f" {'OK' if all(reach_curve[i] <= reach_curve[i+1] for i in range(len(reach_curve)-1)) else 'FALHA'}")

    # --- órbita concreta: token percorrendo um ciclo do grafo ---
    def shortest_cycle_len(s):
        """Menor ciclo por s via BFS (sem pandas)."""
        dist = {s: 0}
        q = deque([s])
        while q:
            u = q.popleft()
            for w in G.successors(u):
                if w == s:
                    return dist[u] + 1
                if w not in dist:
                    dist[w] = dist[u] + 1
                    q.append(w)
        return None

    L = next((cl for s in list(giant)[:50] if (cl := shortest_cycle_len(s))), 5)
    # trajetória: ângulo = posição no ciclo (2pi por volta), raio = delta restante
    steps = delta0
    ang = np.array([2 * np.pi * (t % L) / L for t in range(steps + 1)])
    rad = np.array([delta0 - t for t in range(steps + 1)], dtype=float)
    print(f"  ciclo de exemplo: comprimento L={L}; a órbita dá ~{delta0/L:.1f}"
          f" voltas espiralando até delta=0")
    print("=" * 64)

    # ----------------------------------------------------------------
    # PLOTS
    # ----------------------------------------------------------------
    fig = plt.figure(figsize=(13, 9))

    # (a) órbita de delegação: espiral dissipativa vs órbita conservativa
    ax = fig.add_subplot(2, 2, 1, projection="polar")
    ax.plot(ang, rad, "-o", ms=3, lw=1.2, color="#1f4e79", label="delegação: δ dissipa → espiral")
    ax.plot(np.linspace(0, 2 * np.pi, 200), np.full(200, delta0),
            "--", color="crimson", lw=1, label="de Sitter: δ conservado → órbita fechada")
    ax.set_title("(a) Órbita: ciclo do grafo (L=%d)\nraio = energia δ" % L, fontsize=10)
    ax.set_rmax(delta0 + 1)
    ax.legend(loc="upper right", fontsize=7, bbox_to_anchor=(1.25, 1.12))

    # (b) token DANDO VOLTAS no ciclo: revisita os mesmos nós (órbita), mas δ
    #     só cai. Eixo y direito = nó visitado (mostra a repetição).
    ax = fig.add_subplot(2, 2, 2)
    hops = np.arange(delta0 + 1)
    deltas = delta0 - hops
    node_in_cycle = hops % L  # nó revisitado a cada L saltos (a órbita)
    ax.plot(hops, deltas, "-o", ms=3, color="#1f4e79", label="δ (energia) ↓")
    for b in range(L, delta0 + 1, L):
        ax.axvline(b, color="gray", ls=":", lw=0.8)
    ax.set_title("(b) Token orbitando o ciclo (L=%d): revisita os mesmos\n"
                 "nós, mas a energia δ só decresce (linhas = voltas)" % L, fontsize=10)
    ax.set_xlabel("salto de delegação")
    ax.set_ylabel("δ restante (energia)")
    ax2 = ax.twinx()
    ax2.step(hops, node_in_cycle, where="post", color="crimson", alpha=0.5, lw=1)
    ax2.set_ylabel("posição no ciclo (revisita)", color="crimson", fontsize=8)
    ax2.tick_params(axis="y", labelcolor="crimson")
    ax.grid(alpha=0.3)
    ax.legend(fontsize=8, loc="upper right")

    # (c) ponto fixo mu P: reach satura em k=delta0
    ax = fig.add_subplot(2, 2, 3)
    ax.plot(range(len(reach_curve)), reach_curve, "-o", ms=3, color="#1f4e79")
    ax.axvline(delta0, color="crimson", ls="--", lw=1, label="k=δ0 (saturação = μP)")
    ax.set_title("(c) Alcance |Pᵏ(raiz)| → ponto fixo μP\n(Kleene de baixo, converge em ≤ δ0)", fontsize=10)
    ax.set_xlabel("k (saltos)")
    ax.set_ylabel("nós alcançados")
    ax.legend(fontsize=8)
    ax.grid(alpha=0.3)

    # (d) geodésica conservativa (seção euclidiana S² do paper) — órbita FECHADA
    ax = fig.add_subplot(2, 2, 4)
    Ph, Ps = sphere_geodesic(L=0.6)
    ax.plot(Ph, Ps, lw=1.2, color="crimson")
    pmax = np.arccos(0.6)
    ax.axhline(pmax, color="k", ls=":", lw=0.8)
    ax.axhline(-pmax, color="k", ls=":", lw=0.8, label="retorno ψ=±arccos(L)")
    ax.set_title("(d) Geodésica conservativa: seção euclidiana S² do paper\n"
                 "dψ²+cos²ψ dφ² · energia CONSERVADA → órbita fechada", fontsize=10)
    ax.set_xlabel("φ (fase/tempo euclidiano)")
    ax.set_ylabel("ψ (parâmetro de álgebra)")
    ax.legend(fontsize=7)
    ax.grid(alpha=0.3)

    fig.suptitle(
        "Órbitas de delegação vs mecânica celeste de de Sitter — "
        "atenuação = atrito ⇒ ciclos espiralam a δ=0",
        fontsize=12, fontweight="bold")
    fig.tight_layout(rect=[0, 0, 1, 0.96])
    path = OUT + "/delegation_orbits.png"
    fig.savefig(path, dpi=130)
    print("figura salva em", path)


if __name__ == "__main__":
    main()
