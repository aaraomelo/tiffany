/* termica.h — A RADIAÇÃO NEGRA: as constantes e as leis, para o headjack e o que vier depois.
 *
 * O Aarão: "falta o dual pra fechar o circuito, é a radiação térmica — projetar o array de
 * sensores pra detectar. É a radiação negra, o dual do eletromagnético."
 *
 * Aqui só as leis e as constantes, medidas no `radiacao.c`. Elas são de tabela — CODATA e as
 * definições do SI — e não há uma escolhida por mim.
 *
 * Uma constante física NÃO É EXCEPÇÃO: é um decimal escrito, logo um racional. σT⁴, λT = b
 * e hc/λkT vivem em ℤ — Wien por CORTE (como cosmico.c / radiacao.c), Stefan–Boltzmann é
 * T⁴ por definição, e o expoente de Planck é produto (Π), sem exp().
 */
#ifndef TERMICA_H
#define TERMICA_H

/* SI exactos como inteiros (o decimal escrito, sem vírgula). */
#define C_LUZ        299792458L       /* m/s — exacta por definição */
#define WIEN_B_p12   2897771955L      /* 2.897771955×10^{-3} m·K, em 10^{-12} */
#define K_B_n        1380649L         /* 1.380649×10^{-23} J/K */
#define H_PLANCK_n   662607015L       /* 6.62607015×10^{-34} J·s */

/* Stefan–Boltzmann: P ∝ T⁴ por DEFINIÇÃO. (2T)⁴ / T⁴ = 16, exacto em ℤ. */
static long tm_sb_pot(long T){ return T*T*T*T; }

/* Wien por CORTE: o pico NÃO SE FORMA. b, T e a janela são racionais; «lmin ≤ b/T ≤ lmax»
 * vira comparação de inteiros. Escalas: as do radiacao.c (b em 10^{-12} m·K, T em
 * centésimos de K, λ em µm × 10000). */
static int tm_wien_corte(long b, long T, long lmin, long lmax){
    return b > lmin * T && b < lmax * T;
}

/* Planck: o expoente é hc/λkT. Π = produto — dois (λ,T) têm o mesmo x sse λT iguais.
 * Sem exp(), sem grelha de nm. É a lei de deslocamento, outra vez. */
static int tm_planck_mesmo_x(long l1, long T1, long l2, long T2){
    return l1 * T1 == l2 * T2;
}

/* dP/dT ∝ 4 T³ — o que um sensor térmico mede, em unidades de σ. */
static long tm_sb_dP(long T){ return 4L * T*T*T; }

#endif
