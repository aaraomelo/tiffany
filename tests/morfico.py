#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
O CORPO MÓRFICO — o antiroque é a deflexão pela metade, e NÃO a reflexão ν=−1.

Certifica os [N] de elementares/morfico.tex pelo Formalizador: cada afirmação
exibe um REFUTADOR — o MESMO predicado num OBJETO MUTADO — que devolve False.

Este paper já esteve FALSO NO TÍTULO: afirmava que a complementação ¬ era a
reflexão ν=−1 "porque −1 = +1 em característica 2". É falso. Em Z/2:

    End(Z/2) = F₂,  e a única involução de End é ν=1, a IDENTIDADE;
    D₁(x)=x⊕1 tem D₁(0)=1 ≠ 0, logo é uma TRANSLAÇÃO, não um endomorfismo.

Por isso as mutações que mordem, aqui, são: mutar o módulo (para desfazer o
colapso −1≡+1), mutar o primo p (para tirar a involutividade de D₁), e mutar a
máscara de simétrica para não-simétrica (para pôr a antípoda B̌=−B a trabalhar).

    python3 elementares/morfico.py
"""
from __future__ import annotations

import random
import sys

from formalizador import Formalizador

random.seed(1)

# ------------------------------------------------------------------ o corpo booleano
_corpo = lambda n: all(any((a & b) == (2 ** n - 1) for b in range(2 ** n))
                       for a in range(1, 2 ** n))            # todo não-nulo inverte
_zerodiv = lambda n: any((a & b) == 0 for a in range(1, 2 ** n) for b in range(1, 2 ** n))
_inverte = lambda a, n: any((a & b) == (2 ** n - 1) for b in range(2 ** n))

# ------------------------------------------------------------------ Z/p e o antiroque
_D1 = lambda x, p: (x + 1) % p                               # a deflexão (translação +1)
_is_endo = lambda f, m: all((f((x + y) % m)) % m == (f(x) + f(y)) % m
                            for x in range(m) for y in range(m))
_invol_end = lambda m: [nu for nu in range(m)               # involuções de End(Z/m): ν²=id
                        if all((nu * nu * x) % m == x % m for x in range(m))]
_refl = lambda m: [(-x) % m for x in range(m)]              # ν=−1 como permutação
_ident = lambda m: list(range(m))
_D1perm = lambda m: [(x + 1) % m for x in range(m)]         # D₁ como permutação
_defl_inv = lambda l: [k for k in range(1, l)               # deflexões x↦x+k involutivas: 2k≡0
                       if all((x + 2 * k) % l == x for x in range(l))]

# ------------------------------------------------------------------ mórfico ⊣ métrico (o círculo R/ℓZ)
# ℓ=None é o métrico (ℝ, char 0, livre de torção); ℓ finito é o mórfico.
_reflex = lambda l: ((-1) != 1) if l is None else ((-1) % l != 1 % l)          # ν=−1 não trivial?
_metade = lambda l: (any(2 * t == 0 for t in range(1, 999))) if l is None \
    else any((2 * t) % l == 0 for t in range(1, l))                            # 2-torção não trivial?

# ------------------------------------------------------------------ a antípoda no toro Z/12
_M = 12
_neg = lambda A: set(range(_M)) - A                         # complementação ¬ (translação)
_ant = lambda A: {(-a) % _M for a in A}                     # antípoda ν=−1 (reflexão)


def main() -> int:
    F = Formalizador("O CORPO MÓRFICO — o antiroque é a metade, não a reflexão")

    # =====================================================================
    F.seccao("1. (F₂ⁿ, ⊕, ∧) é anel booleano: corpo se e somente se n = 1")
    F.afirma("todo a é idempotente: a ∧ a = a",
             lambda: all((a & a) == a for a in range(8)),
             lambda: all((a ^ a) == a for a in range(8)),
             "muta ∧→⊕: a ⊕ a = 0 ≠ a")
    F.afirma("n=1 (GF(2)): 1 inverte — é corpo",
             lambda: _corpo(1),
             lambda: _corpo(3),
             "muta n→3: 0b011 já não inverte, deixa de ser corpo")
    F.afirma("n=3: 0b001 ∧ 0b010 = 0, divisores de zero",
             lambda: _zerodiv(3),
             lambda: _zerodiv(1),
             "muta n→1: GF(2) não tem divisores de zero")
    F.afirma("n=3: 0b011 não tem inverso",
             lambda: not _inverte(3, 3),
             lambda: not _inverte(7, 3),
             "muta o elemento 3→7: 0b111 ∧ 0b111 = 0b111, inverte-se a si")

    # =====================================================================
    F.seccao("2. o ANTIROQUE é a deflexão pela metade, em Z/2")
    F.afirma("𝒢 = Z/2 tem ℓ = 2, e ℓ/2 = 1 é uma torção genuína",
             lambda: _metade(2) and (2 // 2) % 2 != 0,
             lambda: _metade(3) and (3 // 2) % 3 != 0,
             "muta ℓ→3 (ímpar): não há metade (2-torção não trivial)")
    F.afirma("D₁(x) = x ⊕ 1 = ¬x  (em Z/2)",
             lambda: all(_D1(x, 2) == (x ^ 1) for x in (0, 1)),
             lambda: all(_D1(x, 3) == (x ^ 1) for x in range(3)),
             "muta p→3: (x+1)%3 já não é o XOR")
    F.afirma("D₁ é involução  (⟺ p = 2)",
             lambda: all(_D1(_D1(x, 2), 2) == x for x in range(2)),
             lambda: all(_D1(_D1(x, 3), 3) == x for x in range(3)),
             "muta p→3: D₁∘D₁(x)=x+2, involutiva só se 2≡0")
    F.afirma("1 é a 2-torção de Z/2: 2·1 ≡ 0",
             lambda: (2 * 1) % 2 == 0,
             lambda: (2 * 1) % 3 == 0,
             "muta o módulo→3: 2·1 = 2 ≢ 0")
    F.afirma("logo o antiroque é a ÚNICA deflexão involutiva de Z/2 (teo:deflexaometade)",
             lambda: _defl_inv(2) == [1],
             lambda: _defl_inv(3) == [1],
             "muta ℓ→3: não há deflexão involutiva não nula ([] ≠ [1])")

    # =====================================================================
    F.seccao("2b. MAS NÃO É A REFLEXÃO: em char 2, ν=−1 colapsa na identidade")
    F.afirma("as involuções de End(Z/2) são só ν=1, a IDENTIDADE",
             lambda: _invol_end(2) == [1],
             lambda: _invol_end(3) == [1],
             "muta o módulo→3: em Z/3 as involuções são {1,2}=±1, não só {1}")
    F.afirma("ν=−1 ≡ +1 em char 2: a reflexão do contrato colapsa",
             lambda: (-1) % 2 == 1 % 2,
             lambda: (-1) % 3 == 1 % 3,
             "muta o módulo→3: −1 ≡ 2 ≠ 1")
    F.afirma("D₁ NÃO é endomorfismo: D₁(0)=1 ≠ 0",
             lambda: not _is_endo(lambda x: _D1(x, 2), 2),
             lambda: not _is_endo(lambda x: 0, 2),
             "muta D₁→(x↦0): a aplicação nula É endomorfismo (fixa 0)")
    F.afirma("a reflexão ν=−1 colapsa na identidade [0,1]; D₁ dá o swap [1,0]",
             lambda: _refl(2) == _ident(2) and _D1perm(2) != _ident(2),
             lambda: _refl(3) == _ident(3) and _D1perm(3) != _ident(3),
             "muta o módulo→3: aí ν=−1=[0,2,1] NÃO é a identidade")
    F.afirma("logo o antiroque NÃO é a reflexão: como permutações, [1,0] ≠ [0,1]",
             lambda: _D1perm(2) != _refl(2),
             lambda: _refl(2) != _refl(2),
             "muta D₁→ν=−1: identificar os dois mapas colapsa a desigualdade")

    # =====================================================================
    F.seccao("2c. e as duas involuções da dualidade morfológica são distintas")
    _A = {0, 1, 2}
    F.afirma("¬A ≠ (−1)·A",
             lambda: _neg(_A) != _ant(_A),
             lambda: _neg(_A) != _neg(_A),
             "muta ν=−1 → ¬: se a antípoda FOSSE a complementação, coincidiam")
    F.afirma("¬ não fixa o vazio; ν=−1 fixa",
             lambda: _neg(set()) != set() and _ant(set()) == set(),
             lambda: _neg(set()) != set() and _neg(set()) == set(),
             "muta a antípoda → ¬: a complementação NÃO fixa ∅, quebra a 2ª metade")

    # =====================================================================
    F.seccao("2d. mórfico e métrico são DUAIS: trocam reflexão por deflexão-pela-metade")
    F.afirma("métrico (ℓ→∞): a reflexão ν=−1 é NÃO trivial",
             lambda: _reflex(None),
             lambda: _reflex(2),
             "muta o círculo ℓ→2: em Z/2 a reflexão colapsa (−1≡1)")
    F.afirma("métrico (ℓ→∞): a metade é trivial — só θ=0 (livre de torção)",
             lambda: not _metade(None),
             lambda: not _metade(2),
             "muta o círculo ℓ→2: aí a metade θ=1 EXISTE")
    F.afirma("mórfico (Z/2): reflexões -> só ν=1, a identidade",
             lambda: _invol_end(2) == [1],
             lambda: _invol_end(3) == [1],
             "muta o módulo→3: involuções {1,2}≠{1}  (restata a afirmação de 2b)")
    F.afirma("mórfico (Z/2): a deflexão involutiva θ=1=ℓ/2 EXISTE",
             lambda: _metade(2) and (1 ^ 1) == 0,
             lambda: _metade(3) and (1 ^ 1) == 0,
             "muta o círculo ℓ→3: não há metade")
    F.afirma("exatamente um dos dois é não trivial em cada: (metade, reflexão)",
             lambda: (_metade(2) != _reflex(2)) and (_metade(None) != _reflex(None)),
             lambda: _metade(4) != _reflex(4),
             "muta o círculo ℓ→4: R/4Z tem AMBOS, quebra o ou-exclusivo")
    F.afirma("R/ℓZ tem AMBOS: é o meio-termo onde o contrato vive",
             lambda: (_metade(10) and _reflex(10)) and (_metade(1000) and _reflex(1000)),
             lambda: _metade(2) and _reflex(2),
             "muta o círculo ℓ→2: só a metade, a reflexão colapsou")
    F.afirma("ℓ→∞ dá o métrico (reflexão, sem metade); ℓ=2 dá o mórfico (metade, sem reflexão)",
             lambda: (_reflex(None) and not _metade(None)) and (_metade(2) and not _reflex(2)),
             lambda: (_reflex(None) and not _metade(None)) and (_metade(3) and not _reflex(3)),
             "muta o círculo ℓ=2→3: ℓ=3 não é o degenerado mórfico")

    # =====================================================================
    F.seccao("3. a adjunção δ ⊣ ε, e as leis que dela seguem")
    N = 10
    VIZ = {s: {t for t in (s - 1, s, s + 1) if 0 <= t < N} for s in range(N)}
    dil = lambda A: {t for s in A for t in VIZ[s]}
    ero = lambda A: {s for s in range(N) if VIZ[s] <= A}
    ero_estrita = lambda A: {s for s in range(N) if VIZ[s] < A}   # a mutação: ⊂ em vez de ⊆
    alea = lambda: set(random.sample(range(N), random.randint(0, N)))
    pares = [(alea(), alea()) for _ in range(3000)]

    F.afirma("δ(A) ⊆ B  ⟺  A ⊆ ε(B)   [3000 pares]",
             lambda: all((dil(A) <= B) == (A <= ero(B)) for A, B in pares),
             lambda: all((dil(A) <= B) == (A <= ero_estrita(B)) for A, B in pares),
             "muta a erosão ⊆→⊂: a adjunção quebra")
    F.afirma("δεδ = δ",
             lambda: all(dil(ero(dil(A))) == dil(A) for A, _ in pares),
             lambda: all(dil(ero_estrita(dil(A))) == dil(A) for A, _ in pares),
             "muta a erosão ⊆→⊂")
    F.afirma("εδε = ε",
             lambda: all(ero(dil(ero(A))) == ero(A) for A, _ in pares),
             lambda: all(ero_estrita(dil(ero_estrita(A))) == ero_estrita(A) for A, _ in pares),
             "muta a erosão ⊆→⊂")
    F.afirma("φ = εδ é idempotente e extensiva",
             lambda: all(ero(dil(ero(dil(A)))) == ero(dil(A)) and A <= ero(dil(A)) for A, _ in pares),
             lambda: all(ero_estrita(dil(ero_estrita(dil(A)))) == ero_estrita(dil(A))
                         and A <= ero_estrita(dil(A)) for A, _ in pares),
             "muta a erosão ⊆→⊂: a extensividade A⊆εδ(A) falha")
    F.afirma("γ = δε é idempotente e anti-extensiva",
             lambda: all(dil(ero(dil(ero(A)))) == dil(ero(A)) and dil(ero(A)) <= A for A, _ in pares),
             lambda: all(dil(ero_estrita(dil(ero_estrita(A)))) == dil(ero_estrita(A))
                         and dil(ero_estrita(A)) <= A for A, _ in pares),
             "muta a erosão ⊆→⊂")

    # =====================================================================
    F.seccao("4. a erosão preserva a MULTIPLICAÇÃO; a dilatação, a soma reticular")
    F.afirma("ε(A ∧ B) = ε(A) ∧ ε(B)",
             lambda: all(ero(A & B) == (ero(A) & ero(B)) for A, B in pares),
             lambda: all(ero(A | B) == (ero(A) | ero(B)) for A, B in pares),
             "troca as operações ∧→∨: ε NÃO preserva o ∨")
    F.afirma("δ(A ∨ B) = δ(A) ∨ δ(B)",
             lambda: all(dil(A | B) == (dil(A) | dil(B)) for A, B in pares),
             lambda: all(dil(A & B) == (dil(A) & dil(B)) for A, B in pares),
             "troca as operações ∨→∧: δ NÃO preserva o ∧")
    F.afirma("mas ε NÃO preserva ∨",
             lambda: any(ero(A | B) != (ero(A) | ero(B)) for A, B in pares),
             lambda: any(ero(A & B) != (ero(A) & ero(B)) for A, B in pares),
             "troca ∨→∧: ε preserva o ∧, logo não há contraexemplo")

    # =====================================================================
    F.seccao("5. a dualidade: ν = −1 conjuga a dilatação na erosão")
    M = 12
    Vsim = {s: {(s - 1) % M, s % M, (s + 1) % M} for s in range(M)}   # simétrico: B̌=B
    Vass = {s: {s % M, (s + 1) % M, (s + 2) % M} for s in range(M)}   # a mutação: assimétrico
    ds = lambda A: {t for s in A for t in Vsim[s]}
    es = lambda A: {s for s in range(M) if Vsim[s] <= A}
    da = lambda A: {t for s in A for t in Vass[s]}
    ea = lambda A: {s for s in range(M) if Vass[s] <= A}
    n2 = lambda A: set(range(M)) - A
    am = [set(random.sample(range(M), random.randint(0, M))) for _ in range(2000)]

    F.afirma("ε(A) = ¬ δ(¬A)   (elemento simétrico: a antípoda é trivial)",
             lambda: all(es(A) == n2(ds(n2(A))) for A in am),
             lambda: all(ea(A) == n2(da(n2(A))) for A in am),
             "muta a máscara simétrica→assimétrica: sem antípoda a lei quebra")

    # A antípoda posta a trabalhar: máscara NÃO simétrica.
    B = {0, 1, 2}
    Bc = {(-b) % M for b in B}          # B̌ = −B
    Bsim = {0, 1, 11}                   # {0,1,11} é simétrico mod 12: −B = B
    dB = lambda A, S: {(a + b) % M for a in A for b in S}
    eB = lambda A, S: {x for x in range(M) if all((x + b) % M in A for b in S)}

    F.afirma("a máscara B = {0,1,2} NÃO é simétrica: B̌ = −B ≠ B",
             lambda: Bc != B,
             lambda: {(-b) % M for b in Bsim} != Bsim,
             "muta B→{0,1,11} (simétrico): aí B̌ = B, e a antípoda não trabalha")
    F.afirma("a lei GERAL vale com a antípoda: ε_B(A) = ¬ δ_{B̌}(¬A)",
             lambda: all(eB(A, B) == n2(dB(n2(A), Bc)) for A in am),
             lambda: all(eB(A, B) == n2(dB(n2(A), B)) for A in am),
             "muta B̌→B: sem a antípoda a igualdade falha")
    F.afirma("e FALHA sem a antípoda: ε_B(A) ≠ ¬ δ_B(¬A)",
             lambda: any(eB(A, B) != n2(dB(n2(A), B)) for A in am),
             lambda: any(eB(A, B) != n2(dB(n2(A), Bc)) for A in am),
             "muta B→B̌ (repõe a antípoda): a lei passa, não há contraexemplo")
    F.afirma("o que conjuga δ em ε é a ANTÍPODA B̌ = −B, e não a complementação ¬",
             lambda: (all(eB(A, B) == n2(dB(n2(A), Bc)) for A in am)
                      and any(eB(A, B) != n2(dB(n2(A), B)) for A in am)),
             lambda: (all(eB(A, B) == n2(dB(n2(A), B)) for A in am)
                      and any(eB(A, B) != n2(dB(n2(A), B)) for A in am)),
             "muta B̌→B na conjunção: o lado 'vale com antípoda' cai")

    return F.veredito()


if __name__ == "__main__":
    sys.exit(main())
