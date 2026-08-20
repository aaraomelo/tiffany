/* eliptica.h — A CURVA É UMA MÁQUINA DE RASTROS, E A FIBRA DECIDE A OPERAÇÃO.
 *
 *     E: y² = x³ + ax + b,   com  4a³ + 27b² ≠ 0   (não singular)
 *
 *  ordem do coordenador diz porque é que este andar é o teste certo: «reta → interseção →
 * reflexão → novo ponto → repetição», e sobretudo esta frase, que já é a língua da casa:
 *
 *     «a FIBRA determina qual operação existe»
 *
 * É literal. Com x₁ ≠ x₂ há secante; com x₁ = x₂ há duas fibras diferentes — ou Q = −P
 * (e a soma é 𝒪) ou Q = P (e é a tangente). Não se «aproxima» o denominador zero: a
 * divisão por zero continua a ser uma operação SEM FIBRA, e o que muda é qual das três
 * operações se aplica. O andar de ℚ volta aqui inteiro.
 *
 * DUAS REALIZAÇÕES, e é de propósito:
 *
 *   sobre ℚ    — coordenadas RACIONAIS exatas (Qz). Nenhum decimal.
 *   sobre 𝔽ₚ   — tudo módulo p, e a divisão é a INVERSÃO MODULAR: a/b = a·b⁻¹ (mod p).
 *                «Olha aí o andar dos racionais voltando como inversão modular.»
 *
 * E as duas não vivem separadas: a REDUÇÃO mod p leva a conta de ℚ na conta de 𝔽ₚ, e é
 * essa concordância que se mede — dois caminhos sobre o mesmo objeto.
 *
 * Precisa de `racionais.h`, `inteiros.h` e `numeros.h`. */
#ifndef ELIPTICA_H
#define ELIPTICA_H

/* ── SOBRE ℚ: as coordenadas são frações exatas ────────────────────────────────── */
typedef struct { int inf; Qz x, y; } PtQ;      /* inf = 1 é o ponto no infinito 𝒪 */

static PtQ eq_O(void){ PtQ P; P.inf = 1; P.x = qz(0,1); P.y = qz(0,1); return P; }
static PtQ eq_pt(Qz x, Qz y){ PtQ P; P.inf = 0; P.x = x; P.y = y; return P; }

/* a NÃO SINGULARIDADE: 4a³ + 27b² ≠ 0 — e é ela que garante que a tangente existe
 * em todo o ponto, isto é, que a máquina não encrava */
static int eq_regular(Qz a, Qz b){
    Qz t = qz_soma(qz_mult(qz_de_inteiro(4), qz_mult(a, qz_mult(a,a))),
                   qz_mult(qz_de_inteiro(27), qz_mult(b,b)));
    return t.p != 0;
}
static int eq_na_curva(Qz a, Qz b, PtQ P){
    if(P.inf) return 1;                        /* 𝒪 está sempre na curva */
    Qz e = qz_mult(P.y, P.y);
    Qz d = qz_soma(qz_soma(qz_mult(P.x, qz_mult(P.x, P.x)), qz_mult(a, P.x)), b);
    return qz_igual(e, d);
}
static PtQ eq_neg(PtQ P){                      /* −P = (x, −y): a reflexão no eixo x */
    if(P.inf) return P;
    return eq_pt(P.x, qz_oposto(P.y));
}
static int eq_igual(PtQ P, PtQ Q){
    if(P.inf || Q.inf) return P.inf && Q.inf;
    return qz_igual(P.x, Q.x) && qz_igual(P.y, Q.y);
}
/* A SOMA, E A FIBRA A DECIDIR QUAL DAS TRÊS OPERAÇÕES CORRE.
 *   P = 𝒪 ou Q = 𝒪        → o neutro
 *   x₁ = x₂ e y₁ = −y₂     → opostos: 𝒪   (inclui y = 0, onde 2P = 𝒪)
 *   P = Q                  → a TANGENTE, m = (3x² + a)/(2y)
 *   x₁ ≠ x₂                → a SECANTE,  m = (y₂ − y₁)/(x₂ − x₁) */
static PtQ eq_soma(Qz a, Qz b, PtQ P, PtQ Q){
    (void)b;
    if(P.inf) return Q;
    if(Q.inf) return P;
    if(qz_igual(P.x, Q.x) && qz_igual(P.y, qz_oposto(Q.y))) return eq_O();
    Qz m;
    if(eq_igual(P, Q)){
        if(P.y.p == 0) return eq_O();          /* tangente vertical: o gume do §5 */
        Qz num = qz_soma(qz_mult(qz_de_inteiro(3), qz_mult(P.x, P.x)), a);
        Qz den = qz_mult(qz_de_inteiro(2), P.y);
        if(!qz_divide(num, den, &m)) return eq_O();
    } else {
        Qz num = qz_soma(Q.y, qz_oposto(P.y));
        Qz den = qz_soma(Q.x, qz_oposto(P.x));
        if(!qz_divide(num, den, &m)) return eq_O();
    }
    Qz x3 = qz_soma(qz_mult(m,m), qz_oposto(qz_soma(P.x, Q.x)));
    Qz y3 = qz_soma(qz_mult(m, qz_soma(P.x, qz_oposto(x3))), qz_oposto(P.y));
    return eq_pt(x3, y3);
}
/* DOUBLE-AND-ADD: «multiplicação inteira → dobras + somas», que é o quantizador da
 * casa aplicado ao grupo. n em binário, e cada bit é um tick. */
static PtQ eq_mult(Qz a, Qz b, PtQ P, long n){
    PtQ R = eq_O(), D = P;
    if(n < 0){ n = -n; D = eq_neg(P); }
    while(n > 0){
        if(n & 1) R = eq_soma(a, b, R, D);
        D = eq_soma(a, b, D, D);
        n >>= 1;
    }
    return R;
}
/* a ORDEM de P: o menor k > 0 com kP = 𝒪 (0 se não fechar dentro do teto) */
static long eq_ordem(Qz a, Qz b, PtQ P, long teto){
    PtQ R = P;
    for(long k = 1; k <= teto; k++){
        if(R.inf) return k;
        R = eq_soma(a, b, R, P);
    }
    return 0;
}

/* ── SOBRE 𝔽ₚ: a mesma geometria, agora aritmética discreta ────────────────────── */
typedef struct { int inf; long x, y; } PtF;

static long ef_m(long v, long p){ long r = v % p; return r < 0 ? r + p : r; }
static PtF ef_O(void){ PtF P; P.inf = 1; P.x = 0; P.y = 0; return P; }
static PtF ef_pt(long x, long y, long p){ PtF P; P.inf = 0; P.x = ef_m(x,p); P.y = ef_m(y,p); return P; }
static int ef_na_curva(long a, long b, long p, PtF P){
    if(P.inf) return 1;
    return ef_m(P.y*P.y, p) == ef_m(P.x*P.x%p*P.x + a*P.x + b, p);
}
static PtF ef_neg(long p, PtF P){
    if(P.inf) return P;
    return ef_pt(P.x, -P.y, p);
}
static int ef_igual(PtF P, PtF Q){
    if(P.inf || Q.inf) return P.inf && Q.inf;
    return P.x == Q.x && P.y == Q.y;
}
/* A DIVISÃO AQUI É A INVERSÃO MODULAR — «a/b não significa decimal: significa a·b⁻¹» */
static PtF ef_soma(long a, long b, long p, PtF P, PtF Q){
    (void)b;
    if(P.inf) return Q;
    if(Q.inf) return P;
    /* OS OPOSTOS, ditos à cabeça — e é o SEGUNDO fecho, não o único (medido por
     * mutação: apagar esta linha não muda um único resultado). O `nm_inv_mod` abaixo já
     * recusa quando o denominador é 0, e den = 0 acontece EXATAMENTE nos dois casos em
     * que a resposta é 𝒪: x₁ = x₂ com y₂ = −y₁ (opostos) e a tangente com y = 0. A
     * linha está cá para NOMEAR o ramo, que é o que uma linha de guarda deve fazer —
     * é a mesma situação do guarda da coprimalidade no Teorema Chinês. */
    if(P.x == Q.x && ef_m(P.y + Q.y, p) == 0) return ef_O();
    long m, num, den, inv;
    if(ef_igual(P, Q)){
        num = ef_m(3*P.x%p*P.x + a, p);
        den = ef_m(2*P.y, p);
    } else {
        num = ef_m(Q.y - P.y, p);
        den = ef_m(Q.x - P.x, p);
    }
    if(!nm_inv_mod(den, p, &inv)) return ef_O();   /* sem fibra: não há operação */
    m = ef_m(num * inv, p);
    long x3 = ef_m(m*m - P.x - Q.x, p);
    long y3 = ef_m(m*(P.x - x3) - P.y, p);
    return ef_pt(x3, y3, p);
}
static PtF ef_mult(long a, long b, long p, PtF P, long n){
    PtF R = ef_O(), D = P;
    if(n < 0){ n = -n; D = ef_neg(p, P); }
    while(n > 0){
        if(n & 1) R = ef_soma(a, b, p, R, D);
        D = ef_soma(a, b, p, D, D);
        n >>= 1;
    }
    return R;
}
static long ef_ordem(long a, long b, long p, PtF P){
    PtF R = P;
    for(long k = 1; k <= 4*p + 8; k++){
        if(R.inf) return k;
        R = ef_soma(a, b, p, R, P);
    }
    return 0;
}
/* CONTAR OS PONTOS, PELA VARREDURA DOS x — «faça por duas vias» é o exercício, e a
 * segunda via (quadrados e cubos tabelados) fica no medidor, para serem mesmo DUAS. */
static long ef_conta(long a, long b, long p){
    long n = 1;                                 /* o 𝒪 conta */
    for(long x = 0; x < p; x++){
        long d = ef_m(x*x%p*x + a*x + b, p);
        for(long y = 0; y < p; y++) if(ef_m(y*y, p) == d) n++;
    }
    return n;
}
#endif
