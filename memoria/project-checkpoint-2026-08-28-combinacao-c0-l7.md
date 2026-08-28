---
name: project-checkpoint-2026-08-28-combinacao-c0-l7
description: "28/08 — fecha C0–L7; combinacao.tex congelado; próxima toca (M1/M2, E∂) só registada, não implementada."
metadata:
  type: project
---

# 28/08 — COMBINAÇÃO FECHADA (C0–L7); PRÓXIMA TOCA CONGELADA

Checkpoint para retomada por outro agente: ler **este ficheiro**, `papers/combinacao.tex`,
`lib/arena_combinacao.mjs`, `tests/redes_combinacao.js` e o git — **não** memória de chat.

## Estado congelado

| Item | Estatuto |
|------|----------|
| `papers/combinacao.tex` | **FECHADO e CONGELADO** — não reabrir |
| `papers/redes.tex` | FECHADO (§RG6) — não alterar |
| C0–C8 | concluído |
| S0–S4 | concluído |
| R0–R4 | concluído |
| D0–D3 | concluído |
| K0–K7 | concluído |
| L0–L7 | concluído |
| Bateria | `node tests/redes_combinacao.js` → **174 asserts, 0 falhas** |
| L8 | **não abrir** |
| Promover `lexMax` a $C$ | **proibido** |
| P6 / Hebb | **TRAVADA** |

## Conclusão experimental

$$
\boxed{\exists f:X\times X\to X\text{ bem definido e dependente dos dois argumentos}}
$$

mas

$$
\boxed{\text{a arquitetura medida até aqui não determina uma lei única }C.}
$$

- `lexMax` e `rectCell` são realizações **distintas** (K7).
- L2–L4 eliminam `rectCell` **só** se tomadas como critérios de uma classe — **não** como axiomas retroativos para escolher `lexMax`.
- Classe mínima: $\mathcal{A}_{\min}=\{f$ bilateral, bemDef, $\tau$, $G$, $\zeta/\mu$-compat$\}$.

## Separação conceptual (preservar)

$$
\partial^2=\mathrm{id}
\quad\text{(operador)}
$$

Faces / cruzados:

$$
x\oplus\partial x=1,\qquad x\otimes\partial x=0.
$$

Incidência (não determina $C$):

$$
\mu\zeta=\mathrm{id}.
$$

Hierarquia de níveis:

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
$$

Viveiro (localização, não lei ontológica $C$):

$$
\vee=\operatorname{lcm},\qquad
\wedge=\operatorname{gcd},\qquad
\operatorname{lcm}(a,b)\operatorname{gcd}(a,b)=ab.
$$

Teorema Central Gentil–Hurwitz–Lebesgue: **fundo / comparação**, não quarta autoridade na sonda M1/M2.
`Central $\not\Rightarrow C$`.

## Próxima toca — REGISTADA, NÃO IMPLEMENTADA

**Não começar neste checkpoint.** Ordem obrigatória:

1. **Ordem dos andares** (`thm:rn`, lex) — isolada:

$$
M_1:\ x\prec_{\mathrm{lex}}x'\Rightarrow C(x,y)\preceq_{\mathrm{lex}}C(x',y)
$$

$$
M_2:\ y\prec_{\mathrm{lex}}y'\Rightarrow C(x,y)\preceq_{\mathrm{lex}}C(x,y')
$$

2. **Só depois** — dobra:

$$
E_\partial:\ \partial\bigl(C(\partial x,\partial y)\bigr)\stackrel{?}{\sim}C(x,y)
$$

3. Registar $(M_1,M_2,E_\partial,K)$. Independência é resultado válido.
4. Central/medida — só **depois**, se ainda houver pergunta legítima.

**Não misturar** ordem lex e $\partial$ no mesmo experimento.
**Não** axiomatizar a partir do candidato.

## Ficheiros âncora

```
papers/combinacao.tex          — paper fechado
papers/redes.tex               — §RG6 fechado (não tocar)
lib/arena_combinacao.mjs       — C0–L7, PROBE_REDUCER, CANDIDATO
tests/redes_combinacao.js      — 174 asserts
tools/auditoria_rg6.bat        — wrapper da bateria
memoria/redes-pipe.md          — pipe RG6
memoria/duomorfismo-pipe.md    — faces ⊕/⊗
```

## Como verificar

```bat
cmd /c "call tools\env_node.bat && node tests\redes_combinacao.js"
```

Esperado: `#TOTAL 174 0`.

## Regra metodológica

$$
\boxed{\text{medir primeiro; interpretar depois; axiomatizar nunca a partir do candidato.}}
$$

$$
\boxed{\text{uma estrutura pode restringir uma realização sem determinar a lei.}}
$$
