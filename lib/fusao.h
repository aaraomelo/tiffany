/* ═══════════════════════════════════════════════════════════════════════════
 * lib/fusao.h — A FUSÃO DE DOIS CORPOS: o entrelaçamento C_ent do paper
 *
 * A `aranha prop:travessia` escreve-a: «a fusão é precisa, para réguas de
 * escalas diferentes. Escreva-se C_ent(a,b) para o ENTRELAÇAMENTO --- os dígitos
 * de a nas posições pares e os de b nas ímpares»:
 *
 *     C_ent(a,b) = Σ a_j 2^{2j} + Σ b_j 2^{2j+1}
 *
 * «Ela JUNTA duas réguas de k dígitos numa de 2k entrelaçando-as posição a
 * posição, e o desentrelaçar SEPARA devolvendo as duas exactas --- e é assim que
 * C_ent REALIZA o papel estrutural da dobra C do thm:enumfin, que ali é dada por
 * existência. Não é o mesmo objeto pelo nome: é uma escolha concreta que serve
 * para o mesmo, e com ela a dobra deixa de ser uma bijeção que existe e passa a
 * ser uma que se ESCREVE.»
 *
 * E a fusão de dois corpos DINÂMICOS é outra coisa, que aqui também se escreve:
 * dois sistemas de primeira ordem acoplados por uma constante dão UM de segunda
 * ordem, e a companheira do fundido lê-se do traço e do determinante da matriz
 * do acoplamento. É a mesma palavra em dois níveis --- o endereço funde-se por
 * entrelaçamento, a dinâmica funde-se por acoplamento --- e convém não as colar.
 *
 * Medido em `tests/fusao.c` e `tests/pgwire.c` §W177.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef FUSAO_H
#define FUSAO_H

/* ── O ENDEREÇO: entrelaçar duas réguas de `k` bits numa de 2k ──────────── */
static long fu_entrelaca(long a, long b, int k){
    long e = 0;
    for(int j = 0; j < k; j++){
        e |= ((a >> j) & 1L) << (2*j);
        e |= ((b >> j) & 1L) << (2*j + 1);
    }
    return e;
}
/* e o desentrelaçar, que SEPARA devolvendo as duas exactas */
static void fu_separa(long e, int k, long *a, long *b){
    long x = 0, y = 0;
    for(int j = 0; j < k; j++){
        x |= ((e >> (2*j))     & 1L) << j;
        y |= ((e >> (2*j + 1)) & 1L) << j;
    }
    *a = x; *b = y;
}
/* a volta é exacta: separar o entrelaçado devolve o par de partida */
static int fu_volta(long a, long b, int k){
    long x, y; fu_separa(fu_entrelaca(a, b, k), k, &x, &y);
    return x == a && y == b;
}

/* ── A DINÂMICA: dois corpos de 1.ª ordem acoplados dão UM de 2.ª ───────────
 * Cada corpo tem a sua taxa própria --- p1 = a1/m1 e p2 = a2/m2 --- e o
 * acoplamento entra com a constante K nos dois sentidos, com sinais opostos
 * (é o que faz a energia atravessar em vez de se somar):
 *
 *     x1' = −p1 x1 − (K/m1) x2
 *     x2' = +(K/m2) x1 − p2 x2
 *
 * A matriz tem traço −(p1+p2) e determinante p1p2 + K²/(m1m2), donde a
 * característica do FUNDIDO é λ² + Bλ + C = 0 com
 *
 *     B = p1 + p2,        C = p1 p2 + K²/(m1 m2).
 *
 * Com K = 0 o determinante é p1p2 e as raízes são −p1 e −p2: a fusão DEGENERA
 * nos dois corpos separados, que é o controlo sem o qual ela não diria nada. */
typedef struct { long B, C; long m1, m2; } FuFundido;

static FuFundido fu_acopla(long a1, long m1, long a2, long m2, long K){
    FuFundido f;
    /* aqui p1 = a1/m1 e p2 = a2/m2, e trabalha-se com m1 = m2 = 1 para a conta
     * ficar inteira --- o cliente que precisar de outras escalas sobe a torre */
    f.B = (m1 ? a1/m1 : 0) + (m2 ? a2/m2 : 0);
    f.C = (m1 && m2) ? (a1*a2 + K*K)/(m1*m2) : 0;
    f.m1 = m1; f.m2 = m2;
    return f;
}
static long fu_disc(FuFundido f){ return f.B*f.B - 4*f.C; }

#endif /* FUSAO_H */
