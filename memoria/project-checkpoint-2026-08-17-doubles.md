---
name: project-checkpoint-2026-08-17-doubles
description: "17/08 — tirar doubles: 118→85 limiares, 505:505; F_p, cruzado nulo, tese já exacta"
metadata:
  node_type: memory
  type: project
  modified: 2026-08-17T22:00:00.000Z
---

17/08/2026. Sessão Cursor a seguir `cursor.txt` §3 — eliminar limiares que DECIDEM
asserções, não converter à cega.

## ESTADO

- **Bateria 505:505** (era 505:505 no cursor; mantém-se verde)
- **Limiares em `ok(...)`**: **118 → 85** (−33 nesta sessão: 20 + 13)
- **Ficheiros tocados**: `fecha.c`, `plugs.c`, `spline.c`, `solar.c`, `cosmico.c`,
  `forca.c`, `nne.c`, `sombra_cone.c`, `eletrico.c`, `moe.c`, `travessia.c`,
  `dif.c`, `quantico.c`, `headjack.c`, `radiacao.c`, `corpo_fisico.c`

## O QUE SAIU (por lei, não por limiar)

| ficheiro | antes | depois |
|---|---|---|
| `fecha.c` §F4 | `pior < 1e-12` sobre σ−1/σ | `rt_traco_metalico(m,1) == m` |
| `plugs.c` §P2 | `fabs(σ²−mσ−1) < 1e-12` | `(2σ)² = 2m(2σ)+4` em ℤ[√D] |
| `spline.c` §P5 | `fabs(w12/w10−1.2)` redundante | só `escala_ok == glifos` (def. relida) |
| `solar.c` §S5 | `fabs(FP−φ^{−1/2}) < 1e-14` | `phi_fecha` (ℤ[√5], já medido acima) |
| `cosmico.c` §X5 | `fabs(menor) < 1e-12` tautologia | `ident == ident_tot` (Qf·Tq = Q·Tf em ℤ) |
| `forca.c` §G3 | ok duplicado com `pior < 1e-15` | removido — §G3 já mede Lagrange em ℤ |
| `nne.c` §N1 | 7 limiares double | exacto em Q(√5); 2.ª ok redundante fundida |
| `eletrico.c` §E2 | `fabs(soma)`, `fabs(geo²−L/C)` | `+1−1=0` e `Ln·Cd = 1000·Cn·Ld` |
| `moe.c` §M4 | `pior < 1e-14` sin/cos | `GD·y_MoE = GD·y_densa` em inteiros |
| `travessia.c` §T5 | `res/escala < 1e-13`, `|nE/N−nc|` | só F_p (`volta_ok`, `pars_ok`) |
| `dif.c` §F2 | `e1,e2 < 1e-12` | F⁴ e F² em ℤ₁₇ (`z1==0`, `z2==0`) |
| `dif.c` §F10 | Parseval `pior < 1e-12` | `N·Σx² = Σ X_k X_{−k}` em ℤ₁₇ |
| `dif.c` §F9 | `e2 < 1e-12` linearidade DFT | F(x+y)=F(x)+F(y) em ℤ₁₇ |
| `quantico.c` §Q5 | `>= lado² − 1e-12` | `dx²·dy² >= lado²` (satura exacto em \|0⟩) |
| `headjack.c` | `nt2 > 1e-18`, `d < esc²·1e-24` | `nt2 > 0`, `d == 0` (cruzado de paralelos) |
| `radiacao.c` | `nBr2 < nBt2·1e-24`, `|Pr−Pt|/Pt` | `nBr2 == 0`; só as duas metades em ℤ |
| `corpo_fisico.c` | `1e-9` no comentário dentro do `ok()` | comentário saiu; `p31 != p30` |

## LIÇÕES DESTA RONDA

1. **Definição relida (forma 12)**: `w12/w10 == 1.2` em `spline.c` — av e upem cancelam;
   a asserção real é `escala_ok == glifos`.
2. **Mesma expressão duas vezes (forma 9)**: `cosmico.c` `dS=0` no limite Carnot;
   `solar.c` FP com `pow(PHI,-0.5)` quando `phi_fecha` já prova o passo.
3. **Normalizar antes de medir**: `forca.c` — o ok com θ/cos/sin era redundante depois do
   bloco ℤ de Lagrange.
4. **Float ≠ exacto**: `sombra_cone.c` — `soma_i == 0.0` falhou; a soma exacta mede-se
   em ℤ (`(N−1) + (N−1)·(−1) = 0`) antes de confiar em `⟨s,n⟩`.

## PRÓXIMO

- **85 limiares** restantes — prioridade: `quantico.c` (série mexp, Robertson saturado
  com θ), `travessia.c` (logs de Hodge/ν∘rev, circuito), `dif.c` (símbolo da diferença,
  volta DFT, eixos polares), `encaixa.c`, `corpo_fisico.c` (integrador)
- `tools/alvos_doubles.txt`: 177 candidatos já verificados — continuar um a um com gume
- Citações: 37 a bater (cursor.txt)

Ver [[feedback-assercoes-vazias]], [[feedback-o-double-que-so-transportava]],
[[project-checkpoint-2026-08-16-a-raiz-sai]].
