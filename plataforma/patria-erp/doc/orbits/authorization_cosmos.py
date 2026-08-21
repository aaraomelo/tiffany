#!/usr/bin/env python3
"""
COSMOS DE AUTORIZAÇÃO — a leitura operacional (Reading B) do modelo cosmológico.
Protótipo de observabilidade: o estado de autorização da empresa como um
universo observável. Não inspeciona o CONTEÚDO das regras; lê a GEOMETRIA.

Ideias (todas do modelo casl-propagation.tex):
  1. Massa autorizativa  M(p) = Σ_{κ∈Hold(p)} δ_κ   -> estrelas vs nós escuros
  2. Órbitas (ciclos)    -> sinal organizacional (governança), não falha de seg.
  3. Curvatura local     C(p) = autoridade emitida / recebida  -> hubs/gargalos
  4. Horizonte           δ=0  -> "essa autorização termina aqui" (verde/amar/verm)
  5. Sinais vitais geométricos: baseline vs anomalia -> detecção comportamental
"""
import numpy as np
import networkx as nx
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from collections import deque

OUT = __import__("os").path.dirname(__import__("os").path.abspath(__file__))
RNG = np.random.default_rng(11)


# ----------------------------------------------------------------------
def build_org(anomaly=False):
    """Organização realista: operador Ω -> donos -> perfis -> usuários, com δ
    caindo nível a nível. Cada aresta u->w carrega uma capacidade de profundidade
    delta[w]. Se anomaly=True, injeta um surto: ciclos administrativos + bursts."""
    G = nx.DiGraph()
    delta = {}
    G.add_node("Ω");  delta["Ω"] = 6  # operador de plataforma (estrela central)
    roles_by_owner = {}
    rng = np.random.default_rng(3)
    for o in range(2):  # 2 tenants/donos
        own = f"Dono{o}"
        G.add_node(own); G.add_edge("Ω", own); delta[own] = 5
        roles = ["Financeiro", "Comercial", "RH", "TI"][:3 + o]
        roles_by_owner[own] = [f"{own}/{r}" for r in roles]
        for r in roles_by_owner[own]:
            G.add_node(r); G.add_edge(own, r); delta[r] = 3
            nu = int(rng.integers(4, 9))
            for u in range(nu):
                usr = f"{r}/u{u}"
                G.add_node(usr); G.add_edge(r, usr)
                delta[usr] = int(rng.choice([0, 1, 1, 2]))  # muitos no horizonte
    # um super-delegador (hub de alta curvatura)
    hub = "Dono0/TI"
    if hub not in G:
        G.add_node(hub); G.add_edge("Dono0", hub); delta[hub] = 3
        roles_by_owner["Dono0"].append(hub)
    for tgt in list(G.nodes())[:14]:
        if tgt != hub:
            G.add_edge(hub, tgt)

    if anomaly:
        # surto: ciclo administrativo (Financeiro -> João -> GrupoX -> Financeiro)
        fin = "Dono0/Financeiro"
        for n, d in [("João", 2), ("GrupoX", 2)]:
            G.add_node(n); delta[n] = d
        G.add_edge(fin, "João"); G.add_edge("João", "GrupoX"); G.add_edge("GrupoX", fin)
        # bursts: 60 novas delegações cruzadas + 6 ciclos curtos
        nodes = list(G.nodes())
        for _ in range(60):
            a, b = rng.choice(nodes, 2, replace=False)
            G.add_edge(str(a), str(b))
        for _ in range(6):
            c = list(rng.choice(nodes, 3, replace=False))
            for i in range(3):
                G.add_edge(str(c[i]), str(c[(i + 1) % 3]))
    return G, delta


# ----------------------------------------------------------------------
def mass(G, delta):
    """M(p) = Σ δ das capacidades detidas (proxy: δ próprio + δ que chega por
    arestas de entrada)."""
    return {p: delta.get(p, 0) + sum(delta.get(u, 0) for u in G.predecessors(p))
            for p in G.nodes()}


def curvature(G, delta):
    """C(p) = autoridade emitida / recebida (out vs in, ponderada por δ)."""
    C = {}
    for p in G.nodes():
        out_a = sum(delta.get(w, 0) for w in G.successors(p)) + 0.5
        in_a = sum(delta.get(u, 0) for u in G.predecessors(p)) + 0.5
        C[p] = out_a / in_a
    return C


def orbits(G):
    """SCCs não-triviais = órbitas (ciclos administrativos)."""
    return [c for c in nx.strongly_connected_components(G) if len(c) > 1]


def vital_signs(G, delta):
    orb = orbits(G)
    M = mass(G, delta)
    return {
        "nós": G.number_of_nodes(),
        "delegações": G.number_of_edges(),
        "órbitas": len(orb),
        "nós em órbita": sum(len(c) for c in orb),
        "energia média": round(float(np.mean(list(M.values()))), 1),
        "horizonte (δ=0)": sum(1 for d in delta.values() if d == 0),
    }


# ======================================================================
def main():
    G, delta = build_org(anomaly=False)
    Ga, da = build_org(anomaly=True)

    M = mass(G, delta)
    C = curvature(G, delta)
    orb = orbits(Ga)  # órbita aparece na versão com anomalia
    orbit_nodes = set().union(*orb) if orb else set()

    base = vital_signs(G, delta)
    anom = vital_signs(Ga, da)

    print("=" * 60)
    print("SINAIS VITAIS GEOMÉTRICOS  (baseline -> anomalia)")
    for k in base:
        flag = "  <== SALTO" if anom[k] >= 2 * max(base[k], 1) else ""
        print(f"  {k:18s}: {base[k]:>6} -> {anom[k]:>6}{flag}")
    print("=" * 60)

    # ----------------------------------------------------------------
    fig = plt.figure(figsize=(15, 10))
    pos = nx.spring_layout(Ga, seed=7, k=0.55, iterations=120)  # layout comum

    def draw_cosmos(ax, Gx, dx, title, show_orbits):
        Mx, Cx = mass(Gx, dx), curvature(Gx, dx)
        sizes = [50 + 200 * Cx[n] for n in Gx.nodes()]
        nx.draw_networkx_edges(Gx, pos, ax=ax, alpha=0.10, arrows=False, width=0.5)
        nc = nx.draw_networkx_nodes(Gx, pos, ax=ax, node_size=sizes,
                                    node_color=[Mx[n] for n in Gx.nodes()],
                                    cmap="inferno", vmin=0, vmax=max(Mx.values()))
        if show_orbits and orbit_nodes:
            oe = [(u, v) for u, v in Gx.edges() if u in orbit_nodes and v in orbit_nodes]
            nx.draw_networkx_edges(Gx, pos, edgelist=oe, ax=ax, edge_color="red",
                                   width=2.0, arrows=True, arrowsize=8)
        hz = [n for n in Gx.nodes() if dx.get(n, 0) == 0]
        nx.draw_networkx_nodes(Gx, pos, nodelist=hz, ax=ax,
                               node_size=[50 + 200 * Cx[n] for n in hz],
                               node_color="none", edgecolors="red", linewidths=1.0)
        nx.draw_networkx_labels(Gx, pos, ax=ax, font_size=6, font_color="cyan",
                                labels={n: n for n in ["Ω", "João", "GrupoX"] if n in Gx})
        ax.set_title(title, fontsize=10); ax.axis("off")
        return nc

    ax = fig.add_subplot(2, 2, 1)
    draw_cosmos(ax, G, delta, "(a) BASELINE — universo calmo\n"
                "estrelas (massa) + horizontes (aro vermelho)", show_orbits=False)
    ax = fig.add_subplot(2, 2, 2)
    nc = draw_cosmos(ax, Ga, da, "(b) ANOMALIA — surto: 60 delegações + ciclos\n"
                     "órbita gigante (vermelho), geometria perturbada", show_orbits=True)
    plt.colorbar(nc, ax=fig.axes[:2], fraction=0.02, pad=0.01, label="massa M(p)")

    # (c) concentração de poder: curva de Lorenz da massa
    ax = fig.add_subplot(2, 2, 3)
    vals = np.sort(list(M.values()))
    cum = np.cumsum(vals) / vals.sum()
    x = np.linspace(0, 1, len(cum))
    ax.plot(x, cum, color="#1f4e79", lw=2, label="massa autorizativa")
    ax.plot([0, 1], [0, 1], "--", color="gray", lw=1, label="igualdade perfeita")
    gini = 1 - 2 * np.trapezoid(cum, x)
    ax.fill_between(x, cum, x, alpha=0.15, color="#1f4e79")
    ax.set_title(f"(b) Concentração de autoridade (Lorenz)\nGini={gini:.2f} — "
                 f"quanto maior, mais poder concentrado", fontsize=10)
    ax.set_xlabel("fração de principals"); ax.set_ylabel("fração da autoridade")
    ax.legend(fontsize=8); ax.grid(alpha=0.3)

    # (c) curvatura: top hubs (super-delegadores) e poços (terminais)
    ax = fig.add_subplot(2, 2, 4)
    top = sorted(C.items(), key=lambda kv: kv[1], reverse=True)[:10]
    names = [k.replace("Dono0/", "").replace("Dono1/", "")[:14] for k, _ in top]
    ax.barh(range(len(top)), [v for _, v in top], color="#c1440e")
    ax.set_yticks(range(len(top))); ax.set_yticklabels(names, fontsize=7)
    ax.invert_yaxis()
    ax.set_title("(c) Curvatura C(p)=emitida/recebida — hubs de autorização\n"
                 "(super-delegadores: candidatos a gargalo/risco)", fontsize=10)
    ax.set_xlabel("curvatura"); ax.grid(alpha=0.3, axis="x")

    fig.suptitle("Cosmos de autorização: observabilidade por geometria "
                 "(detecta anomalia sem ler regra alguma)",
                 fontsize=13, fontweight="bold")
    fig.tight_layout(rect=[0, 0, 1, 0.96])
    path = OUT + "/authorization_cosmos.png"
    fig.savefig(path, dpi=125)
    print("figura salva em", path)


if __name__ == "__main__":
    main()
