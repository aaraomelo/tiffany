#!/usr/bin/env python3
"""
Delegação a nível de CAMPO: a autorização passa de 2^R (linhas) para 2^{R×F}
(pares linha×campo). Entram dimensões novas. Pergunta: de Sitter 4D suporta?

Verificação simbólica (mesmo método do paper hiper/.../paper_metrica_quantica_algebras.tex,
que confere o dS_2 via tensor de Ricci). Testamos a forma ESTÁTICA de de Sitter:

  dS_n :  ds^2 = -cos^2(rho) dt^2 + drho^2 + sin^2(rho) dOmega^2_{n-2}

  n=2 (nível de LINHA, do paper):  ds^2 = -cos^2(psi) dtheta^2 + dpsi^2
       -> esperado Einstein  R_uv = (n-1) g_uv = 1*g ,  R = n(n-1) = 2
  n=4 (nível de CAMPO):  + sin^2(rho) (dalpha^2 + sin^2(alpha) dbeta^2)
       (a 2-esfera (alpha,beta) = as direções de CAMPO)
       -> esperado Einstein  R_uv = (n-1) g_uv = 3*g ,  R = n(n-1) = 12
"""

import sympy as sp


def ricci(g, x):
    """Tensor de Ricci, escalar e checagem de Einstein para a métrica g(x)."""
    n = len(x)
    gi = g.inv()
    # Christoffel  Gamma^a_{bc}
    Ga = [[[sp.simplify(
        sum(gi[a, d] * (sp.diff(g[d, b], x[c]) + sp.diff(g[d, c], x[b])
                        - sp.diff(g[b, c], x[d])) for d in range(n)) / 2)
        for c in range(n)] for b in range(n)] for a in range(n)]
    # Ricci  R_{bd} = R^a_{bad}
    Ric = sp.zeros(n, n)
    for b in range(n):
        for d in range(n):
            s = 0
            for a in range(n):
                s += sp.diff(Ga[a][b][d], x[a]) - sp.diff(Ga[a][b][a], x[d])
                for e in range(n):
                    s += Ga[a][a][e] * Ga[e][b][d] - Ga[a][d][e] * Ga[e][b][a]
            Ric[b, d] = sp.simplify(s)
    R = sp.simplify(sum(gi[b, d] * Ric[b, d] for b in range(n) for d in range(n)))
    # Einstein? R_uv - (R/n) g_uv == 0 ?
    einstein = sp.simplify(Ric - (R / n) * g)
    return Ric, R, einstein


def report(name, g, x, n_expected, n_points=12):
    """Verifica Einstein de curvatura constante numericamente em vários pontos
    (robusto à fraqueza do trigsimp). Mesmo método do paper."""
    import random
    n = len(x)
    Ric, R, _ = ricci(g, x)
    rng = random.Random(7)
    lam = n_expected - 1  # R_uv = (n-1) g_uv  (l=1)
    max_ein, max_R = 0.0, 0.0
    for _ in range(n_points):
        # evita singularidades (cos rho != 0, sin alpha != 0)
        sub = {xi: rng.uniform(0.2, 1.2) for xi in x}
        for i in range(n):
            for j in range(n):
                v = complex((Ric[i, j] - lam * g[i, j]).subs(sub))
                max_ein = max(max_ein, abs(v))
        max_R = max(max_R, abs(complex(R.subs(sub)) - n_expected * (n_expected - 1)))
    einstein_ok = max_ein < 1e-9
    R_ok = max_R < 1e-9
    print(f"=== {name}  (dim {n}) ===")
    print(f"  |R_uv - (n-1) g_uv|  máx em {n_points} pontos = {max_ein:.2e}"
          f"  -> Einstein, R_uv = {lam} g_uv ?  {einstein_ok}")
    print(f"  |R - n(n-1)| = |R - {n_expected*(n_expected-1)}| máx = {max_R:.2e}"
          f"  -> curvatura constante ?  {R_ok}")
    print(f"  -> de Sitter dS_{n_expected}: {'CONFIRMADO' if einstein_ok and R_ok else 'NÃO'}")
    print()


def main():
    # --- n=2: nível de linha (o de Sitter do paper) ---
    th, ps = sp.symbols("theta psi", real=True)
    g2 = sp.diag(-sp.cos(ps) ** 2, 1)
    report("dS_2  nível de LINHA (paper)", g2, [th, ps], 2)

    # --- n=4: nível de campo (2 direções de campo = 2-esfera) ---
    t, rho, al, be = sp.symbols("t rho alpha beta", real=True)
    g4 = sp.diag(
        -sp.cos(rho) ** 2,           # t  (fluxo de delegação / fase)
        1,                            # rho (energia delta)
        sp.sin(rho) ** 2,             # alpha (campo)
        sp.sin(rho) ** 2 * sp.sin(al) ** 2,  # beta (campo)
    )
    report("dS_4  nível de CAMPO", g4, [t, rho, al, be], 4)


if __name__ == "__main__":
    main()
