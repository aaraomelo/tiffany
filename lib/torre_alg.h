/* torre_alg.h — CONTRATO DA TORRE: dobra, involução, slots. NÃO implementa aritmética.
 *
 *  ordem do coordenador (19/08/2026) fixa a leitura pós-Fase-A:
 *
 *   A largura NÃO define a álgebra. A ordem é:
 *
 *     álgebra → operação → dual → largura necessária → slot
 *
 *   e não:  tipo C → operação.
 *
 * ── DOBRA (corpo_algebrico.tex thm:torre · corpo_topologico.tex thm:rn) ───────────
 *
 *     T_{k+1} = T_k + T_k*        d_{k+1} = 2 d_k
 *
 *   Cada andar realiza as MESMAS operações estruturais sobre um objecto dobrado.
 *   A passagem de andar não inventa aritmética nova — duplica pela dualidade.
 *
 * ── TRÊS PIPELINES (ordem do coordenador, §3 — NÃO confundir) ─────────────────────────────────
 *
 *   1. INVOLUÇÃO   ν∘ν = id          estrutural
 *        swap ι(a,b)=(b,a)           racionais.tex def:espaco, thm:swap-inverso
 *        hip_conj                    torre.h (Cayley–Dickson)
 *
 *   2. RETRAÇÃO    Σ∘Π = Id          representação
 *        palavra FC → slots .mem     rt_cf_slot.h, slot_mem.h, slot_map.h S_CF
 *        cone desce / espiral sobe   corpo_algebrico.tex def:cone, thm:cone
 *
 *   3. SUBIDA      T_k → T_{k+1}     construção
 *        tr_mult / dobra             torre.h
 *
 *   Retração ≠ involução: espaços e representações diferentes (coordenador).
 *
 * ── ANDARES DA REALIZAÇÃO (largura = memória da dobra) ───────────────────────────
 *
 *   Andar     Tipo / header      Operações (onde vivem)
 *   ─────     ─────────────      ─────────────────────────────────────────────
 *   E₁₆       Qz  racionais.h    ⊕ ⊗ swap(inverso)  — envelope §Q16
 *   32        D32 dual16.h       d16_mult d16_soma  d16_par_*  d16_par_dual
 *   64        D64 dual32.h       d64_mult d64_soma  d32_par_*  d64_dual
 *   128       I128 i128.h        i128_mul i128_add  (produto 64×64 quando cabe)
 *   2ⁿ·dim    Hip torre.h        hip_soma hip_mult hip_conj hip_inverso
 *
 *   16 → 32 → 64 → 128 NÃO é escada de tipos C: é
 *
 *     representação ──(dual/dobra)──→ representação seguinte
 *
 * ── SLOTS: memória física comum (ordem do coordenador, §4) ───────────────────────────────────
 *
 *     CORPO ALGÉBRICO ── operação/dual ── andar k ── representação ── slots .mem
 *                                                      │
 *                                              leitura │ escrita
 *                                              λ⁻ / Σ  │ λ⁺ / Π
 *
 *   Peça: SlotWord {total, e} — 16 bytes/slot (slot_mem.h)
 *   FC:   S_CF = 2048, stride RT_CF_MAX+2 (slot_map.h)
 *   API:  rt_cf_slot_* (rt_cf_slot.h) ≡ sql.c / conversa.c / isa_disk.h
 *
 *   Soma de coeficientes + carry; produto = convolução + carry (posicional).
 *
 * ── QUATRO OPERAÇÕES ESTRUTURAIS POR ANDAR (corpo_algebrico.tex thm:operacao) ───
 *
 *   soma · produto · dual · inversão/fibra
 *
 *   Implementação concreta: dual16.h, dual32.h, i128.h, torre.h — NESTE header
 *   só o contrato. Incluir os headers de andar quando for implementar.
 *
 * Papers cruzados:
 *   racionais.tex      sec:espaco def:ops thm:swap-inverso
 *   corpo_algebrico.tex thm:torre thm:operacao def:cone thm:cone
 *   corpo_topologico.tex thm:rn (A_{n+1}=A_n+A_n*)
 *   teoria.tex / catalogo.tex — citações cruzadas aos medidores e à torre operacional
 */
#ifndef TORRE_ALG_H
#define TORRE_ALG_H

/* índice k da torre operacional (corpo_topologico.tex: d_k = 2^{k+1}, d_0 = 2) */
#define TORRE_D0   2
#define TORRE_DK(k)  (TORRE_D0 << (k))   /* d_k = 2^{k+1} */

/* andares da REALIZAÇÃO por largura de par (bits do produto exacto) */
#define TORRE_LARG_E16   16
#define TORRE_LARG_D32   32
#define TORRE_LARG_D64   64
#define TORRE_LARG_I128 128

/* região FC no .mem — ver slot_map.h */
#ifndef S_CF
#include "slot_map.h"
#endif

#endif
