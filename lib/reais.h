/* reais.h — ℝ: O REAL É O CORTE, E NUNCA UM DECIMAL.
 *
 * A frase do andar é dele: «**ℝ acrescenta a completude: todo buraco racional recebe um
 * ponto**». E a cadeia fecha: ℕ conta, ℤ reverte a soma, ℚ reverte a multiplicação não
 * nula, ℝ **fecha os limites que ℚ não consegue conter**.
 *
 * Aqui a regra da casa manda mais que nunca: **não entra decimal**. Um `double` seria a
 * negação exata deste andar — ele afirma que o real *é* uma tira de casas, e o corte diz
 * que o real é a **decisão** sobre cada racional: abaixo ou acima. A decisão é inteira
 * (`pⁿ < a·dⁿ`), e por isso √2 mede-se sem nunca se aproximar de nada.
 *
 *   «O racional fornece as marcações; o real preenche os cortes entre elas.»
 *
 * TRÊS CAMINHOS PARA O MESMO PONTO, e eles têm de concordar — é essa a medida:
 *
 *   1. o CORTE          A = {q : q ≤ 0 ou qⁿ < a} — a decisão, exata e inteira
 *   2. o PONTO FIXO     x ↦ (a + bx)/(x + b), Möbius INTEIRO, cujo ponto fixo é x² = a
 *                       (o inversor da casa outra vez: o real é o que a órbita persegue)
 *   3. a FRAÇÃO CONTÍNUA  `lado` (cifra.h) — periódica por Lagrange, e é a ESCRITA
 *
 * O corte diz onde está, o Möbius vai lá, a FC escreve-o. Se os três discordassem num
 * único racional, um deles estava errado — e é isso que se quer apanhar.
 *
 * Precisa de `racionais.h` (o Qz) e de `cifra.h` (o `raizi` e o `lado`). */
#ifndef REAIS_H
#define REAIS_H

/* ── O CORTE ─────────────────────────────────────────────────────────────────────
 * ⁿ√a como decomposição de ℚ. Não guarda casas: guarda o critério que as decidiria
 * todas. */
typedef struct { long a; int n; } Corte;

/* O TETO, e diz-se em vez de dar a volta ao contador: com |p|,d ≤ 2³⁰ e n ≤ 3, o
 * `__int128` chega para pⁿ e para a·dⁿ com folga. Acima disso a comparação RECUSA-SE —
 * um número que não cabe no tipo é um número que a máquina não mediu. */
#define RZ_TETO (1L << 30)
static int rz_cabe(long p, long d, int n){
    long ap = p < 0 ? -p : p;
    return n >= 1 && n <= 3 && ap <= RZ_TETO && d > 0 && d <= RZ_TETO;
}
/* compara qⁿ com a: devolve −1 (abaixo), 0 (EM CIMA — o buraco fechou), +1 (acima).
 * Põe *bom = 0 quando não cabe, e aí não se afirma nada. */
static int rz_cmp(Qz q, int n, long a, int *bom){
    if(bom) *bom = 1;
    if(!rz_cabe(q.p, q.q, n)){ if(bom) *bom = 0; return 0; }
    __int128 P = 1, D = 1;
    for(int i = 0; i < n; i++){ P *= q.p; D *= q.q; }
    __int128 R = (__int128)a * D;
    if(P < R) return -1;
    if(P > R) return +1;
    return 0;
}
/* q ∈ A? — «q < 0 ou qⁿ < a», que é o corte que ele escreve para o √2 */
static int rz_abaixo(Corte c, Qz q){
    if(q.p <= 0) return 1;                       /* a > 0 ⟹ ⁿ√a > 0: todo q ≤ 0 é de A */
    int bom, s = rz_cmp(q, c.n, c.a, &bom);
    if(!bom) return -1;                          /* não mediu: e diz-se */
    return s < 0;
}
/* O BURACO: existe racional EM CIMA do corte? (qⁿ = a exatamente) */
static int rz_no_corte(Corte c, Qz q){
    int bom, s = rz_cmp(q, c.n, c.a, &bom);
    return bom && s == 0;
}
/* e o corte fecha em ℚ exatamente quando a é potência n-ésima perfeita — medido pela
 * régua da casa (`raizi`), não por tentativa */
static int rz_fecha_em_q(Corte c, long *r){
    long x;
    if(c.n == 2) x = raizi(c.a);                 /* a régua da casa, e não uma segunda */
    else {                                       /* o índice geral sobe um degrau de cada vez */
        x = 0;
        while(x < RZ_TETO){
            __int128 P = 1;
            for(int i = 0; i < c.n; i++) P *= (x + 1);
            if(P > c.a) break;
            x++;
        }
    }
    __int128 P = 1;
    for(int i = 0; i < c.n; i++) P *= x;
    if(P == c.a){ if(r) *r = x; return 1; }
    return 0;
}

/* ── O ENCAIXOTAMENTO: os intervalos encaixados, e as larguras são EXATAS ─────────
 * I₁ ⊇ I₂ ⊇ …, com b_n − a_n = (b₀ − a₀)/2ⁿ — não «tende a zero»: É essa fração.
 * Ele chama-lhe «praticamente um relógio de aproximação geométrica», e é literal: cada
 * passo é um tick e a velocidade só refina por DOBRA. */
static int rz_caixa_inicial(Corte c, Qz *lo, Qz *hi){
    long k = 0;
    while(k < RZ_TETO){
        Qz z = qz_de_inteiro(k + 1);
        int bom, s = rz_cmp(z, c.n, c.a, &bom);
        if(!bom) return 0;
        if(s > 0){ *lo = qz_de_inteiro(k); *hi = z; return 1; }
        if(s == 0){ *lo = z; *hi = z; return 1; }      /* fechou em ℚ */
        k++;
    }
    return 0;
}
static int rz_encaixota(Corte c, Qz *lo, Qz *hi, int passos){
    for(int k = 0; k < passos; k++){
        Qz m = qz_medio(*lo, *hi);
        int bom, s = rz_cmp(m, c.n, c.a, &bom);
        if(!bom) return k;                        /* parou por não caber, e diz quantos */
        if(s == 0){ *lo = m; *hi = m; return k + 1; }
        if(s < 0) *lo = m; else *hi = m;
    }
    return passos;
}

/* ── O PONTO FIXO: x ↦ (a + bx)/(x + b), Möbius INTEIRO ──────────────────────────
 * O erro multiplica-se por (b − √a)/(x + b) a cada passo: com b² > a o sinal não muda e
 * a sucessão é MONÓTONA e limitada — o §9 dele, exato. E o ponto fixo é x² = a: por isso
 * a sucessão é de Cauchy em ℚ e **não tem limite racional**. É o buraco à vista.
 * Só para n = 2: é o andar quadrático, o mesmo onde a FC é periódica (Lagrange). */
static long rz_b(long a){ long r = raizi(a); return r + 1; }   /* b² > a, garantido */
static Qz rz_passo(long a, long b, Qz x){
    return qz(a * x.q + b * x.p, x.p + b * x.q);   /* (a + bx)/(x + b), tudo inteiro */
}
#endif
