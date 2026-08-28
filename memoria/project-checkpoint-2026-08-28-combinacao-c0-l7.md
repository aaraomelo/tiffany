---
name: project-checkpoint-2026-08-28-combinacao-c0-l7
description: "28/08 — C0–L7 congelado; M1/M2 e E_∂ medidos e registados; lexMax não promovido; combinacao.tex fechado."
metadata:
  type: project
---

# 28/08 — COMBINAÇÃO: C0–L7 FECHADO; M1/M2 e E_∂ REGISTADOS

Checkpoint para retomada: ler **este ficheiro**, `lib/arena_combinacao.mjs`,
`tests/redes_combinacao.js` — **não** memória de chat.
`papers/combinacao.tex` e `papers/redes.tex` permanecem **congelados**.

## Estado

| Item | Estatuto |
|------|----------|
| `papers/combinacao.tex` | **FECHADO e CONGELADO** — não reabrir |
| `papers/redes.tex` | FECHADO (§RG6) — não alterar |
| C0–C8 / S0–S4 / R0–R4 / D0–D3 / K0–K7 / L0–L7 | concluído (ciclo anterior) |
| **M0–M2** | **medido e registado** — ordem lex isolada de ∂ |
| **E_∂** | **medido e registado** — dobra isolada da ordem lex |
| **rectCell** | **rectângulo completo** — célula + Σ_dom+Σ_im + dual 𝔐/ℰ |
| Bateria | `node tests/redes_combinacao.js` → **231 asserts, 0 falhas** |
| L8 | **não abrir** |
| Promover `lexMax` a $C$ | **proibido** |
| P6 / Hebb | **TRAVADA** |
| Central / Gentil–Hurwitz | estrutura de `rectCell` (as duas metades); **não** autoridade para escolher $C$ |

## Tuplo registado $(M_1,M_2,E_\partial,K)$

$$
\boxed{
\begin{array}{c|cc}
 & \operatorname{lexMax} & \operatorname{rectCell} \\
\hline
M_1 & \checkmark & \times \\
M_2 & \checkmark & \times \\
E_\partial & \times\ \text{(aplicável, não covariante)} & \times\ \text{(não aplicável)} \\
K & \text{comut+assoc+idempot+bemDef} & \text{bemDef+bilateral} \\
\end{array}}
$$

`leiC = null` em ambas as baterias.

**Resultado experimental (congelado):**

$$
\operatorname{lexMax}=(M_1,M_2,E_\partial)=(1,1,0)
$$

$$
\boxed{\text{a ordem lex restringe a classe de realizações; }\partial\text{ não selecciona uma delas}}
$$

$$
\boxed{\text{estrutura restringe a realização, mas não determina a lei}}
$$

Independência: $M_1\wedge M_2$ em lexMax $\not\Rightarrow E_\partial$.
As duas autoridades são separadas.

$$
\boxed{\text{a ordem lex restringe; a dobra não selecciona; nenhuma determina }C.}
$$

## M1–M2 — ordem lex (isolada de ∂)

Cadeia `M_CADEIA = (A0 ≺ B0 ≺ C0 ≺ D0)`. Realização: ordem lex em Word (`thm:rn`).

$$
M_1:\ x\prec_{\mathrm{lex}}x'\Rightarrow f(x,y)\preceq_{\mathrm{lex}}f(x',y)
$$

$$
M_2:\ y\prec_{\mathrm{lex}}y'\Rightarrow f(x,y)\preceq_{\mathrm{lex}}f(x,y')
$$

- `lexMax`: M1 e M2, 0 violações / 24+24.
- `rectCell`: falha M1 (10/24) e M2 (6/24).
- Classificação, **não** escolha de $C$. Não misturar com ∂ neste protocolo.

## E_∂ — covariância sob a dobra (isolada da ordem lex)

Alfabeto `E_ALFABETO = (P0,P1,P2,P3)`, distinto da cadeia lex.
$\partial$ realiza $D(i)=(N-1)-i$ (`fis:thm:troca-realizacao`). Sem `cmpLex`.

$$
E_\partial:\ \partial\bigl(f(\partial x,\partial y)\bigr)\stackrel{?}{\sim}f(x,y)
$$

- E0: $\partial^2=\mathrm{id}$, alfabeto fecha, sem ponto fixo, $P0\leftrightarrow P3$, $P1\leftrightarrow P2$.
- `lexMax`: aplicável; **não** covariante (4/16 iguais, só a diagonal).
- `rectCell`: imagem fora do carrier (`dom:…`) — **não aplicável** (16/16 `foraDoSuporte`).

## rectCell — as duas metades (fis:thm:central)

Antes só a célula: `dom:le|im:gt` (partição $x\le f(i)$ vs $f(i)<x$).

Agora o rectângulo fecha sobre $\pi\colon I\to X$ (Def. `fis:def:objeto`):
$I$ são **índices**, $X$ são **células**. $X$ omisso $= \operatorname{im}\pi$
(sobrejectiva). Com $X$ ambiente, as células vazias entram na soma:

$$
\Sigma_{\mathrm{dom}}+\Sigma_{\mathrm{im}}=\lvert I\rvert\cdot\lvert X\rvert
$$

Exemplo auditado: $\pi=(B0,C0)$ em $X=(A0,B0,C0,D0)$ dá $5+3=8$.

Dual massa/energia (`fis:thm:tresgraus`): $\mathfrak{M}=\lvert I\rvert-\lvert\operatorname{supp}G\rvert$ grau 1;
$\mathcal{E}=\sum G^{2}$ grau 2 — a massa **é** energia pela régua ao quadrado.

- Diagonal $(x,x)$: `dom:le|im:le#2+0=2#m1e4` — $G=2$, $\mathfrak{M}=1$, $\mathcal{E}=4$.
- Distinto: `…#3+1=4#m0e2` — vazio, energia = espaço.
- Trocar as duas leituras **não move** $\Sigma$ nem a área; só a orientação da célula.

Central **estrutura** o candidato; **não** selecciona $C$.

## Hierarquia (preservar)

$$
\boxed{
\partial^2=\mathrm{id}
\neq
\vee\vee=\vee
\neq
\mu\zeta=\mathrm{id}
\neq
C_{\mathrm{local}}
}
$$

$$
\boxed{\text{índice}\neq\text{conteúdo}}
\qquad
\boxed{\text{Central }\not\Rightarrow C}
$$

`lexMax` e `rectCell` continuam realizações distintas (K7). L2–L4 e agora M1/M2
eliminam `rectCell` **só** como critérios de classe — **não** axiomas para escolher `lexMax`.

## Ainda aberto

1. Qual estrutura *dentro da arquitectura* tem autoridade para seleccionar uma lei única.
2. Nomear $C$ — **não feito**.
3. Central como autoridade para escolher $C$ — **não**; já estrutura `rectCell`.
4. Hebb/P6 — TRAVADA.
5. **Não abrir outra toca teórica agora.** Parede experimental fechada.
   Próxima pergunta (se houver): invariantes de uma realização admissível sob $\partial$,
   não «qual é $C$».

## Ficheiros âncora

```
papers/combinacao.tex          — paper fechado (não reabrir; o 174 lá é histórico C0–L7)
papers/redes.tex               — §RG6 fechado (não tocar)
lib/arena_combinacao.mjs       — C0–L7, M1–M2, E_∂, PROBE_REDUCER, CANDIDATO
tests/redes_combinacao.js      — 231 asserts
memoria/redes-pipe.md          — pipe RG6
memoria/duomorfismo-pipe.md    — faces ⊕/⊗
```

## Como verificar

```bat
cmd /c "call tools\env_node.bat && node tests\redes_combinacao.js"
```

Esperado: `#TOTAL 231 0`.

## Regra metodológica

$$
\boxed{\text{medir primeiro; interpretar depois; axiomatizar nunca a partir do candidato.}}
$$

$$
\boxed{\text{uma estrutura pode restringir uma realização sem determinar a lei.}}
$$
