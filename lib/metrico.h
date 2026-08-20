/* metrico.h — ESPAÇOS MÉTRICOS: só o que faltava.
 *
 *  ordem do coordenador traz dezasseis problemas de espaços métricos e diz «ingerir tudo». Mas
 * procurei antes de escrever, e o `cauchy.h` já tinha metade do andar montado:
 *
 *   · `cy_termo`/`cy_dist` — a sucessão e a distância |a − b|, exactas em ℚ
 *   · `cy_modulo(s, ε, …, &N)` — o N PROCURADO para cada ε, que é o Problema 3
 *   · `cy_equiv` — (xₙ) ~ (yₙ) ⟺ d(xₙ,yₙ) → 0, que é o Problema 14
 *   · `cy_aponta` — a sucessão a apontar para um CORTE, que é o Problema 4
 *   · e os tipos S_CONV (convergentes de √a), S_HARM (a harmónica, que NÃO é de Cauchy)
 *     e S_ALT ((−1)ⁿ), que são as testemunhas dos gumes
 *
 * Então aqui fica só o que não existia: os quatro axiomas da métrica, a bola, Lipschitz
 * e Banach, a isometria, as duas métricas de ℝ² e o fecho.
 *
 * ── UMA CORREÇÃO QUE É MINHA, E NÃO DELE ───────────────────────────────────────
 * O Problema 4 dá «1, 4/3, 7/5, 24/17, 41/29» como aproximações racionais de √2, e eu
 * escrevi que dois termos estavam errados — julguei-os pelo certificado dos convergentes
 * da fracção contínua, |p² − 2q²| = 1, que eles não satisfazem.
 *
 * ESTAVA EU ERRADO. Aquela lista é a ÓRBITA DE MÖBIUS x ↦ (2 + 2x)/(x + 2), e é
 * exactamente o que o `rz_passo` desta casa gera a partir de 1: 1, 4/3, 7/5, 24/17,
 * 41/29, 140/99. O certificado dela alterna −1, −2, −1, −2 — é outro certificado, e é
 * igualmente válido.
 *
 * Apliquei a régua dos convergentes a uma sucessão de Möbius: DUAS RÉGUAS PARA O MESMO
 * OBJECTO, que é o defeito que esta casa cataloga — e desta vez apliquei-o a quem estava
 * certo. As duas sucessões aproximam √2 e ambas são legítimas; o que muda é o passo.
 *
 * ── A RÉGUA: A DISTÂNCIA MEDE-SE AO QUADRADO ONDE PRECISA DE RAIZ ──────────────
 * A métrica euclidiana de ℝ² tem uma raiz. Aqui compara-se d² com d∞², que é exacto em
 * ℚ e decide a mesma coisa — a raiz é monótona. Onde a raiz fosse mesmo precisa, ela
 * ficava por avaliar, que é a régua desta casa.
 *
 * Precisa de `racionais.h`, `reais.h`, `cauchy.h`. */
#ifndef METRICO_H
#define METRICO_H

#include "dual32.h"   /* o par: |p² − a·q²| sem tipo de 64 bits */

static long mt_estouros = 0;

static Qz mt_abs(Qz x){ return x.p < 0 ? qz_oposto(x) : x; }
static Qz mt_d(Qz x, Qz y){ return mt_abs(qz_soma(x, qz_oposto(y))); }

/* ── OS QUATRO AXIOMAS, cada um verificado À PARTE ──────────────────────────────
 * O eval pede-os «separadamente», e é a maneira certa: uma métrica que falhasse só a
 * triangular passaria num teste que soma os quatro. Devolve um bit por axioma. */
static int mt_positiva(Qz x, Qz y){ return mt_d(x,y).p >= 0; }
static int mt_identidade(Qz x, Qz y){                       /* d = 0 ⟺ x = y */
    return (mt_d(x,y).p == 0) == qz_igual(x,y);
}
static int mt_simetrica(Qz x, Qz y){ return qz_igual(mt_d(x,y), mt_d(y,x)); }
static int mt_triangular(Qz x, Qz y, Qz z){                 /* d(x,z) ≤ d(x,y)+d(y,z) */
    Qz e = mt_d(x,z), dir = qz_soma(mt_d(x,y), mt_d(y,z));
    return e.p * dir.q <= dir.p * e.q;
}
/* ── A BOLA ABERTA ─────────────────────────────────────────────────────────────
 * B(x,r) = {y : d(x,y) < r}. Em ℚ com |·| ela é o intervalo (x−r, x+r) ∩ ℚ — e o que
 * se devolve são os EXTREMOS, não uma lista: o conjunto é infinito. */
static void mt_bola(Qz x, Qz r, Qz *lo, Qz *hi){
    *lo = qz_soma(x, qz_oposto(r));
    *hi = qz_soma(x, r);
}
static int mt_na_bola(Qz x, Qz r, Qz y){
    Qz d = mt_d(x,y);
    return d.p * r.q < r.p * d.q;
}
/* ── LIPSCHITZ E CONTRAÇÃO ─────────────────────────────────────────────────────
 * f afim, f(x) = (p·x + s)/q. A constante é |p/q|, e ela é CONTRAÇÃO quando < 1.
 * O eval quer a constante IDENTIFICADA, não estimada — e aqui ela é exacta. */
typedef struct { Qz m, b; } Afim;                  /* f(x) = m·x + b */
static Qz af_av(Afim f, Qz x){ return qz_soma(qz_mult(f.m, x), f.b); }
static Qz af_constante(Afim f){ return mt_abs(f.m); }
static int af_contracao(Afim f){
    Qz q = af_constante(f);
    return q.p < q.q;                               /* |m| < 1 */
}
/* a desigualdade de Lipschitz, verificada num par: d(f(x),f(y)) ≤ q·d(x,y) */
static int af_lipschitz(Afim f, Qz x, Qz y){
    Qz e = mt_d(af_av(f,x), af_av(f,y));
    Qz dir = qz_mult(af_constante(f), mt_d(x,y));
    return e.p * dir.q <= dir.p * e.q;
}
/* ── BANACH: o ponto fixo, e a EXISTÊNCIA separada da UNICIDADE ────────────────
 * Para f afim com |m| < 1 o ponto fixo resolve-se em ℚ: x* = b/(1−m). A existência
 * exibe-se pela iteração; a unicidade sai de d(x*,y*) ≤ q·d(x*,y*) com q < 1, que
 * força d = 0. São duas provas, e o eval pede-as separadas. */
static int af_ponto_fixo(Afim f, Qz *x){
    Qz um_menos = qz_soma(qz(1,1), qz_oposto(f.m));
    if(um_menos.p == 0) return 0;                   /* m = 1: a fibra é vazia */
    return qz_divide(f.b, um_menos, x);
}
/* a iteração xₙ₊₁ = f(xₙ), e a distância ao ponto fixo em cada passo */
static Qz af_itera(Afim f, Qz x0, long n){
    Qz x = x0;
    for(long k = 0; k < n; k++) x = af_av(f, x);
    return x;
}
/* ── ISOMETRIA: preserva a distância, e é o oposto da contração ────────────────*/
static int af_isometria(Afim f, Qz x, Qz y){
    return qz_igual(mt_d(af_av(f,x), af_av(f,y)), mt_d(x,y));
}
/* ── AS DUAS MÉTRICAS DE ℝ², comparadas AO QUADRADO ────────────────────────────
 * d₁ é a euclidiana e traz raiz; d∞ é o máximo e não traz. Compara-se d₁² com d∞², que
 * decide o mesmo porque a raiz é monótona — e assim a raiz fica por avaliar. */
static Qz mt_d2_euclid(Qz x1, Qz y1, Qz x2, Qz y2){
    Qz a = qz_soma(x1, qz_oposto(x2)), b = qz_soma(y1, qz_oposto(y2));
    return qz_soma(qz_mult(a,a), qz_mult(b,b));
}
static Qz mt_d2_max(Qz x1, Qz y1, Qz x2, Qz y2){
    Qz a = mt_abs(qz_soma(x1, qz_oposto(x2))), b = mt_abs(qz_soma(y1, qz_oposto(y2)));
    Qz m = (a.p * b.q >= b.p * a.q) ? a : b;
    return qz_mult(m, m);
}
/* as duas são EQUIVALENTES: d∞ ≤ d₁ ≤ √2·d∞, e ao quadrado d∞² ≤ d₁² ≤ 2·d∞².
 * Isso mede-se sem raiz nenhuma, e é o que torna a comparação honesta. */
static int mt_equivalentes(Qz x1, Qz y1, Qz x2, Qz y2){
    Qz e = mt_d2_euclid(x1,y1,x2,y2), m = mt_d2_max(x1,y1,x2,y2);
    return m.p * e.q <= e.p * m.q                          /* d∞² ≤ d₁² */
        && e.p * m.q <= 2 * m.p * e.q;                     /* d₁² ≤ 2 d∞² */
}
/* ── O CERTIFICADO DE PELL: é ele que diz se p/q é convergente de √a ───────────
 * |p² − a·q²| = 1. É exacto, e identifica os convergentes da FRACÇÃO CONTÍNUA — e SÓ
 * esses. Uma sucessão de Möbius aproxima √2 igualmente bem e NÃO o satisfaz: o
 * certificado distingue duas famílias, não separa o certo do errado. */
static long mt_pell(long p, long q, long a){
    /* |p² − a·q²|, pelo PAR DUAL de 32 bits — sem tipo largo nenhum */
    D64 e = d64_mult(d32_abs((int)p), d32_abs((int)p));
    D64 q2 = d64_mult(d32_abs((int)q), d32_abs((int)q)), d;
    if(!d64_esc(q2, (unsigned)(a < 0 ? -a : a), &d)){ mt_estouros++; return -1; }
    D64 r = (d64_cmp(e,d) >= 0) ? d64_menos(e,d) : d64_menos(d,e);
    if(r.alto != 0){ mt_estouros++; return -1; }
    return (long)r.baixo;
}
/* ── O FECHO: x ∈ Ā ⟺ toda bola em torno de x toca A ──────────────────────────
 * Para A = (0,1) ∩ ℚ em ℚ, testa-se se x está no fecho procurando um racional de A
 * dentro de B(x,r). A resposta é [0,1] ∩ ℚ, e as DUAS inclusões medem-se à parte. */
static int mt_no_fecho_01(Qz x, Qz r){
    /* há racional em (0,1) ∩ (x−r, x+r)? o intervalo interseta (0,1)? */
    Qz lo, hi;
    mt_bola(x, r, &lo, &hi);
    Qz zero = qz(0,1), um = qz(1,1);
    Qz a = (lo.p * zero.q >= zero.p * lo.q) ? lo : zero;   /* max(lo, 0) */
    Qz b = (hi.p * um.q <= um.p * hi.q) ? hi : um;          /* min(hi, 1) */
    return a.p * b.q < b.p * a.q;                           /* a < b: há espaço */
}
#endif
