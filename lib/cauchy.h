/* cauchy.h — A SUCESSÃO É UM OBJETO, E ℝ = Cauchy(ℚ)/∼.
 *
 * O `eval.txt` dá a segunda construção do andar, e ela é o dual da primeira: o CORTE diz
 * onde o ponto está (uma decisão sobre ℚ); a SUCESSÃO vai lá (um caminho por ℚ). O corte
 * é estático e a sucessão é dinâmica, e o mesmo real sai dos dois.
 *
 *     ∀ε>0 ∃N: m,n > N ⟹ |aₘ − aₙ| < ε
 *
 * E o ε NÃO é um número pequeno: é um RACIONAL qualquer, e o N é a TESTEMUNHA que se
 * exibe. Sem N exibido não há afirmação — é a regra da casa aplicada à análise.
 *
 *     Duas sucessões representam o MESMO real quando aₙ − bₙ → 0.
 *
 * É por isso que os três caminhos do andar (a órbita do Möbius, as pontas da bisseção e
 * os convergentes da FC) não são três aproximações de √2: são TRÊS REPRESENTANTES DA
 * MESMA CLASSE, e a equivalência mede-se.
 *
 * SEM MEMÓRIA: nenhum termo se guarda. Cada aₙ recalcula-se do princípio — a sucessão é
 * a REGRA, não a tabela. É a mesma disciplina do resto da casa.
 *
 * Precisa de `racionais.h`, `reais.h` e `cifra.h`. */
#ifndef CAUCHY_H
#define CAUCHY_H

typedef enum {
    S_MOBIUS,   /* a órbita x ↦ (a+bx)/(x+b) — o ponto fixo é x² = a */
    S_LO,       /* a ponta ESQUERDA do encaixotamento de √a */
    S_HI,       /* a ponta DIREITA — e as duas são a mesma classe */
    S_CONV,     /* os convergentes da FC de √a */
    S_CONST,    /* a constante p/q — o racional visto como real */
    S_INV,      /* 1/(n+1) — o zero visto de cima */
    S_HARM,     /* a HARMÓNICA: os saltos vão a zero e ELA NÃO É DE CAUCHY */
    S_ALT       /* (−1)ⁿ — nem os saltos vão a zero */
} Tipo;

typedef struct { Tipo t; long a, p, q; } Suc;

/* O TETO da harmónica: H₂₀ = 55835135/15519504 e os produtos de `qz_soma` ainda cabem
 * em `long`. Acima disso o denominador (o mmc) estoura — e um número que não cabe no
 * tipo é um número que a máquina não mediu. */
#define CY_HARM_TETO 20

static Qz cy_termo(Suc s, long n){
    switch(s.t){
    case S_CONST: return qz(s.p, s.q);
    case S_INV:   return qz(1, n + 1);
    case S_ALT:   return qz_de_inteiro((n % 2) ? -1 : 1);
    case S_HARM: {
        if(n > CY_HARM_TETO) n = CY_HARM_TETO;
        Qz h = qz(0,1);
        for(long k = 1; k <= n + 1; k++) h = qz_soma(h, qz(1, k));
        return h; }
    case S_MOBIUS: {
        long b = rz_b(s.a);
        Qz x = qz_de_inteiro(raizi(s.a));
        for(long k = 0; k < n; k++) x = rz_passo(s.a, b, x);
        return x; }
    case S_LO: case S_HI: {
        Corte c = { s.a, 2 };
        Qz lo, hi;
        if(!rz_caixa_inicial(c, &lo, &hi)) return qz(0,1);
        rz_encaixota(c, &lo, &hi, (int)n);
        return s.t == S_LO ? lo : hi; }
    case S_CONV: {
        long t[48];
        size_t nt = lado(0, -s.a, t, 48);
        if(!nt) return qz(0,1);
        long pn = 1, qn = 0, pa = 0, qa = 1;
        for(long i = 0; i <= n; i++){
            long ai = t[(size_t)i < nt ? (size_t)i : (1 + (i - 1) % (long)(nt > 1 ? nt - 1 : 1))];
            long pp = ai*pn + pa, qq = ai*qn + qa;
            if(qq > (1L<<28)) break;              /* o teto, e diz-se onde */
            pa = pn; qa = qn; pn = pp; qn = qq;
        }
        return qn ? qz(pn, qn) : qz(0,1); }
    }
    return qz(0,1);
}
static Qz cy_dist(Qz a, Qz b){                    /* |a − b| */
    Qz d = qz_soma(a, qz_oposto(b));
    return d.p < 0 ? qz_oposto(d) : d;
}
/* O MÓDULO DE CONVERGÊNCIA: o menor N tal que |aₘ − aₙ| < ε para todo m,n na janela
 * acima de N. A JANELA DIZ-SE — não se varre o infinito, e fingir que sim era pior que
 * não medir. Devolve 1 e escreve o N; 0 se não achou até ao teto. */
static int cy_modulo(Suc s, Qz eps, long teto, long janela, long *N){
    for(long k = 0; k <= teto; k++){
        int bom = 1;
        for(long m = k; m <= k + janela && bom; m++)
            for(long n2 = k; n2 <= k + janela; n2++){
                Qz d = cy_dist(cy_termo(s, m), cy_termo(s, n2));
                if(!qz_menor(d, eps)){ bom = 0; break; }
            }
        if(bom){ if(N) *N = k; return 1; }
    }
    return 0;
}
/* A EQUIVALÊNCIA: aₙ − bₙ → 0, e é ela que faz de duas sucessões o MESMO real. O N
 * também se exibe — é o mesmo contrato.
 *
 * E É SOBRE A CAUDA, não sobre o primeiro acerto. Escrevi-a primeiro a devolver o
 * primeiro k com |aₖ − bₖ| < ε, e as três sucessões do andar arrancam todas em ⌊√a⌋:
 * dava N = 0 por COINCIDÊNCIA no arranque, e a asserção passava sem medir convergência
 * nenhuma. «→ 0» quer dizer que a partir do N NUNCA MAIS se afastam. */
static int cy_equiv(Suc x, Suc y, Qz eps, long teto, long *N){
    for(long k = 0; k <= teto; k++){
        int fica = 1;
        for(long m = k; m <= teto; m++){
            Qz d = cy_dist(cy_termo(x, m), cy_termo(y, m));
            if(!qz_menor(d, eps)){ fica = 0; break; }
        }
        if(fica){ if(N) *N = k; return 1; }
    }
    return 0;
}
/* LIMITADA: exibe-se a cota M, e é ela a testemunha */
static int cy_limitada(Suc s, long ate, Qz *M){
    Qz m = qz(0,1);
    for(long k = 0; k <= ate; k++){
        Qz a = cy_termo(s, k), aa = a.p < 0 ? qz_oposto(a) : a;
        if(qz_menor(m, aa)) m = aa;
    }
    if(M) *M = m;
    return 1;
}
static int cy_crescente(Suc s, long ate){
    for(long k = 0; k < ate; k++)
        if(!qz_menor(cy_termo(s, k), cy_termo(s, k+1))) return 0;
    return 1;
}
static int cy_decrescente(Suc s, long ate){
    for(long k = 0; k < ate; k++)
        if(!qz_menor(cy_termo(s, k+1), cy_termo(s, k))) return 0;
    return 1;
}
/* CONVERGE PARA UM CORTE: todos os termos a partir de N caem dentro da caixa que a
 * bisseção fechou. É assim que uma sucessão de ℚ aponta para um ponto de ℝ sem que o
 * ponto seja fração. */
static int cy_aponta(Suc s, Corte c, int dobras, long ate, long *N){
    Qz lo, hi;
    if(!rz_caixa_inicial(c, &lo, &hi)) return 0;
    rz_encaixota(c, &lo, &hi, dobras);
    for(long k = 0; k <= ate; k++){
        int dentro = 1;
        for(long m = k; m <= ate; m++){
            Qz a = cy_termo(s, m);
            if(qz_menor(a, lo) || qz_menor(hi, a)){ dentro = 0; break; }
        }
        if(dentro){ if(N) *N = k; return 1; }
    }
    return 0;
}
#endif
