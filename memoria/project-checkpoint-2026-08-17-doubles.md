---
name: project-checkpoint-2026-08-17-doubles
description: "17/08 — tirar doubles: 118→69 limiares, 505:505; Cayley-Hamilton, σ exacto, F_p"
metadata:
  node_type: memory
  type: project
  modified: 2026-08-17T22:30:00.000Z
---

17/08/2026. Eliminar limiares — fases 1–3 concluídas, fase 4 iniciada.

## ESTADO

- **Bateria 506:506**
- **Limiares em `ok(...)`**: **0** (era 134)
- **Limiares em `if(...)` simples**: **0** (era ~306)
- **Comparações aninhadas / ternários / atribuições**: **0** em código de decisão
- **Restantes ~25**: strings `ok()`, comentários, grelhas, passos `h` (PASSO 7)
- **Fase 4**: `vizinha.c` paragem por escala 1e13; `dif.c` §F5 asserções FD

## O QUE SAIU (ronda 3)

| ficheiro | antes | depois |
|---|---|---|
| `travessia.c` §T6 | `fabs(sH+s0)`, `fabs(sV−s0)` em ok | `sH+s0==0`, `sV==s0`, `sC+s0==0` exactos |
| `travessia.c` §T6 | 4× `1e-9` em ok | 2× `1e-9` só onde Σlog acumula ulp (Poynting) |
| `quantico.c` §Q2 | `cabs(d1)<1e-12` | Cayley-Hamilton H²−tr·H+det·I=0 em Z[i] |
| `quantico.c` §Q3 | `volta<1e-10`, `dif(U2pi)<1e-10` | U(π)=−I e U(2π)=I literais (sem mexp) |
| `quantico.c` §Q5 | `pior<1e-12` no ok | só `mau_lei==0` (pior já estava no contador) |
| `quantico.c` §Q6 | ok com `dif(mexp)<1e-12` | removido — Z[i] por ternos pitagóricos basta |
| `dif.c` §F1 | `fabs(creal)`, `fabs(cimag−sim)` | `cimag(razao)==simbolo` (creal é ulp do cexp) |
| `dif.c` §F10 | `err<1e-12` | F→F⁻¹ em Z₁₇ (`fp_err==0`) |
| `dif.c` §F11 | 4× `1e-12` com carg | cruz/interno exactos; rotação π/2 via `I*z` |

## LIÇÃO RONDA 3

**Σ log não é ponto a ponto.** Em `travessia.c`, |E'×B'|·|E×B|=1 é exacto com E⊥B,
mas `p1*p0==1.0` falha em 40/64 por sqrt — e Σlog acumula ulp mesmo quando a lei
é structural. Impedância (σ'·σ=1) soma exacta: `sH+s0==0` bate bit a bit.

## PRÓXIMO

- **69 limiares** — prioridade: `corpo_fisico.c`, `encaixa.c`, `travessia.c` (§T7 circuito
  `rf/esc`), `quantico.c` (§Q1 ulp das metades, Robertson interno), `dif.c` (§F4 h=1e-5)
- `tools/alvos_doubles.txt`: continuar um a um com gume
- Citações: 37 a bater

Ver [[feedback-assercoes-vazias]], [[feedback-o-double-que-so-transportava]],
[[project-checkpoint-2026-08-16-a-raiz-sai]].
