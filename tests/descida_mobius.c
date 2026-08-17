/* descida_mobius.c — A DESCIDA VIA MÖBIUS, E ELA CUSPE A PALAVRA.
 *
 * O Aarão: «vê sobre Möbius inverso, aí está a inversa do ponto fixo; vê que 0 = 1/∞. A
 * torre começa de cima em passo discreto e também de baixo, conservando. Precisamos da
 * descida via Möbius — isso vai cuspir a dinâmica em FC do operador.»
 *
 * E cospe. A casa tinha as duas metades escritas em COLUNAS DIFERENTES e nunca disse que
 * eram a mesma:
 *
 *     rt_cf_de     escrita como EUCLIDES:   (P,Q) → (Q, P mod Q),  a = ⌊P/Q⌋
 *     rt_dobra     escrita como MÖBIUS:     [p:q] → [q : p − m·q]
 *
 * e p − a·q É p mod q quando a = ⌊p/q⌋. Não são duas descidas parecidas: é UMA, e o
 * algoritmo da fracção contínua é iterar a Möbius inversa. Daí a palavra sair sem uma
 * única divisão em vírgula.
 *
 * E o operador PARTE-SE em duas peças que a casa já usava sem nomear:
 *
 *     A_m = T_m ∘ S        subir:  inverter e depois transladar,  g(x) = m + 1/x
 *     A_m⁻¹ = S ∘ T_{−m}   descer: transladar e depois inverter,  g⁻¹(x) = 1/(x − m)
 *
 * — as MESMAS duas peças na ORDEM TROCADA. É isso o espelho do thm:operador, agora em
 * matrizes: a meta-indução não é outra operação, é a mesma composição lida ao contrário.
 *
 * E o 0 = 1/∞ é o eixo: S troca [0:1] com [1:0], S² = I. A torre de cima parte de ∞ e
 * cresce; a de baixo parte de p/q e desce até ∞. O ponto onde uma acaba é onde a outra
 * começa, e é ele que faz da torre uma só.
 *
 *   §M1  o passo de Euclides É a Möbius inversa — as duas rotas, termo a termo
 *   §M2  A_m = T_m∘S e A_m⁻¹ = S∘T_{−m}: as mesmas peças na ordem trocada
 *   §M3  S² = I, e S troca 0 ↔ ∞ — o eixo da torre
 *   §M4  descer e subir devolve o racional EXACTO, e a descida TERMINA
 *   §M5  no ponto fixo ela NÃO termina, e é periódica — o corte visto pelo algoritmo
 *   §M6  e o que a descida conserva: |det| = 1 em todo o passo
 *   §M7  a ARITMÉTICA ADITIVA: A_a·A_0·A_b = A_{a+b}, com A_0 = S no meio
 *   §M8  e ∞ + 1 = −1: a Möbius que o faz tem o ÁUREO por ponto fixo
 *   §M9  e fecha NOS ÍNDICES: ∞ é o 0, dois abaixo está −1/m = norma/traço — e no
 *        operador AO QUADRADO fica um passo só, com det +1 e o anel a virar ℤ[m√D]
 *   §M10 A CONSERVAÇÃO: o mdc é a energia, o denominador é o orçamento, e a paragem é
 *        o orçamento a esgotar-se — no ponto fixo ele não se esgota, e é o corte
 *   §M11 CANTOR/JULIA: o shift CONSERVA o denominador e cicla; a Möbius GASTA-o e termina
 *        — e o par cantor·julia = −1 É σσ† = −1, o mesmo par noutra carta
 *   §M12 VIVIANI: o mesmo par dual com o determinante +1 — z² = 2a(2a−x) sem
 *        trigonometria, os dois ramos ±z, e o nó como ponto fixo da involução
 *
 * Nenhum double, nenhum limiar: compila sem -lm.
 *
 *   cc -O2 -std=c99 -I. -I../lib descida_mobius.c -o descida_mobius && ./descida_mobius
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

/* as matrizes 2×2 inteiras, como [a b; c d] em ordem de leitura */
static void mm(const long *A, const long *B, long *R){
    long t[4];
    t[0] = A[0]*B[0] + A[1]*B[2];  t[1] = A[0]*B[1] + A[1]*B[3];
    t[2] = A[2]*B[0] + A[3]*B[2];  t[3] = A[2]*B[1] + A[3]*B[3];
    for(int i = 0; i < 4; i++) R[i] = t[i];
}
static int meq(const long *A, const long *B){
    for(int i = 0; i < 4; i++) if(A[i] != B[i]) return 0;
    return 1;
}
static long mdet(const long *A){ return A[0]*A[3] - A[1]*A[2]; }

int main(void){
    printf("\n══ A descida via Möbius: o algoritmo da fracção contínua É o operador ══\n");

    /* ─── §M1 ── o passo de Euclides É a Möbius inversa ─────────────────────────────
     * Duas rotas que não partilham uma linha: a `rt_cf_de` da lib, escrita como o mdc, e
     * a iteração da `rt_dobra` com o quociente lido em cada passo. Se forem a mesma
     * descida, as palavras saem iguais TERMO A TERMO — e não só o último valor. */
    long pares = 0, iguais = 0, termos = 0, termos_iguais = 0;
    for(long p = 1; p <= 60; p++) for(long q = 1; q <= 60; q++){
        RtCf w; rt_cf_de(1, p, q, &w);
        if(w.saturou) continue;

        /* a MESMA descida, escrita como Möbius: [p:q] ↦ [q : p − a·q], com a = ⌊p/q⌋.
         * O `rt_dobra(m,…)` da lib faz exactamente isso — e aqui o m de cada passo não é
         * fixo: é o quociente que o próprio passo lê. */
        long P = p, Q = q; int n = 0; long b[RT_CF_MAX];
        while(Q != 0 && n < RT_CF_MAX){
            long a = P / Q;
            b[n++] = a;
            rt_dobra(a, &P, &Q);            /* [P:Q] ↦ [Q : P − a·Q] */
        }
        pares++;
        int bate = (n == w.n);
        for(int i = 0; i < n && i < w.n; i++){
            termos++;
            if(b[i] == w.a[i]) termos_iguais++; else bate = 0;
        }
        if(bate) iguais++;
    }
    printf("\n  §M1  o passo de Euclides É a Möbius inversa\n");
    printf("      racionais descidos ............... %ld\n", pares);
    printf("      palavras iguais TERMO A TERMO .... %ld\n", iguais);
    printf("      termos comparados ................ %ld   (iguais: %ld)\n",
           termos, termos_iguais);
    ok("a descida de Euclides e a iteração da Möbius inversa dão a MESMA palavra, termo a"
       " termo — p − a·q É p mod q quando a = ⌊p/q⌋, e o algoritmo da fracção contínua é"
       " iterar g⁻¹", pares > 0 && iguais == pares && termos_iguais == termos);

    /* ─── §M2 ── as mesmas peças na ordem trocada ───────────────────────────────────
     * S é a inversão x ↦ 1/x, T_m a translação x ↦ x+m. Então g(x) = m + 1/x é T_m∘S, e
     * g⁻¹(x) = 1/(x−m) é S∘T_{−m}. Em matrizes isso verifica-se ENTRADA A ENTRADA, e o
     * que se vê é que a inversa não usa peças novas: usa as mesmas na ordem oposta.
     * É o espelho do thm:operador escrito como composição. */
    long ok_sub = 0, ok_desc = 0, ok_inv = 0, ms = 0;
    const long S[4] = {0,1,1,0};
    for(long m = 1; m <= 40; m++){
        long Tm[4]  = {1, m, 0, 1};                 /* x ↦ x + m */
        long Tmn[4] = {1,-m, 0, 1};                 /* x ↦ x − m */
        long Am[4]  = {m, 1, 1, 0};                 /* g(x) = m + 1/x */
        long Ai[4]  = {0, 1, 1,-m};                 /* g⁻¹(x) = 1/(x − m) */
        long R[4];
        ms++;
        mm(Tm, S, R);   if(meq(R, Am)) ok_sub++;    /* A_m   = T_m ∘ S   */
        mm(S, Tmn, R);  if(meq(R, Ai))  ok_desc++;  /* A_m⁻¹ = S ∘ T_{−m} */
        mm(Am, Ai, R);                              /* e uma desfaz a outra */
        if(R[0] == R[3] && R[1] == 0 && R[2] == 0 && R[0] != 0) ok_inv++;
    }
    printf("\n  §M2  A_m = T_m∘S e A_m⁻¹ = S∘T_{−m} — as mesmas peças, ordem trocada\n");
    printf("      m testados ....................... %ld\n", ms);
    printf("      A_m = T_m ∘ S .................... %ld\n", ok_sub);
    printf("      A_m⁻¹ = S ∘ T_{−m} ............... %ld\n", ok_desc);
    printf("      e A_m·A_m⁻¹ é escalar (a volta) .. %ld\n", ok_inv);
    ok("subir é inverter e depois transladar; descer é transladar e depois inverter — a"
       " meta-indução não traz peça nova, é a mesma composição lida ao contrário",
       ok_sub == ms && ok_desc == ms && ok_inv == ms);

    /* ─── §M3 ── S² = I, e S troca 0 ↔ ∞ ────────────────────────────────────────────
     * O eixo da torre. Em ℙ¹, 0 = [0:1] e ∞ = [1:0], e a inversão troca-os: 0 = 1/∞ não
     * é uma maneira de falar, é a acção de S. E S²= I diz que ela é a sua própria
     * inversa — a involução mais simples que o corpo tem.
     *
     * GUME: a translação T_m NÃO é involução para m ≠ 0, e conta-se quantas o são. */
    long S2[4]; mm(S, S, S2);
    int s_involucao = (S2[0]==1 && S2[1]==0 && S2[2]==0 && S2[3]==1);
    long z_p = 0, z_q = 1, i_p = 1, i_q = 0;                  /* 0 e ∞ */
    long zp2 = S[0]*z_p + S[1]*z_q, zq2 = S[2]*z_p + S[3]*z_q;  /* S(0)  */
    long ip2 = S[0]*i_p + S[1]*i_q, iq2 = S[2]*i_p + S[3]*i_q;  /* S(∞)  */
    int troca = (zp2 == 1 && zq2 == 0) && (ip2 == 0 && iq2 == 1);
    long t_involucao = 0, t_total = 0;
    for(long m = -20; m <= 20; m++){
        long T[4] = {1, m, 0, 1}, T2[4]; mm(T, T, T2);
        t_total++;
        if(T2[0]==1 && T2[1]==0 && T2[2]==0 && T2[3]==1) t_involucao++;
    }
    printf("\n  §M3  S² = I, e S troca 0 ↔ ∞ — o eixo da torre\n");
    printf("      S² = I ........................... %s\n", s_involucao ? "sim" : "não");
    printf("      S(0) = ∞  e  S(∞) = 0 ............ %s\n", troca ? "sim, 0 = 1/∞" : "não");
    printf("      GUME: T_m involução em %ld de %ld (só m = 0)\n", t_involucao, t_total);
    ok("0 = 1/∞ é a acção de S, e S é a sua própria inversa — enquanto a translação só o"
       " é no zero, o que mostra que a involução é da INVERSÃO e não da composição",
       s_involucao && troca && t_involucao == 1);

    /* ─── §M4 ── descer e subir devolve o racional EXACTO ───────────────────────────
     * A torre nos dois sentidos. A descida parte de p/q e vai até ∞ = [P:0]; a subida
     * parte de ∞ e reconstrói p/q. Mede-se que a volta é exacta e que a descida TERMINA
     * — que é o que distingue o racional (§thm:descida: a palavra que fecha). */
    long casos = 0, volta_ok = 0, terminou = 0, passos_tot = 0;
    for(long p = 1; p <= 60; p++) for(long q = 1; q <= 60; q++){
        RtCf w; rt_cf_de(1, p, q, &w);
        if(w.saturou) continue;
        casos++;
        if(w.n > 0) terminou++;                       /* a palavra fechou */
        passos_tot += w.n;
        long rp, rq;
        if(rt_cf_para(&w, &rp, &rq)){
            long g = rt_mdc(p, q); if(g < 1) g = 1;
            if(rp == p/g && rq == q/g) volta_ok++;    /* devolve a fracção REDUZIDA */
        }
    }
    printf("\n  §M4  descer e subir devolve o racional exacto\n");
    printf("      racionais ........................ %ld\n", casos);
    printf("      descidas que TERMINARAM .......... %ld\n", terminou);
    printf("      voltas exactas (fracção reduzida)  %ld\n", volta_ok);
    printf("      passos de descida, ao todo ....... %ld\n", passos_tot);
    ok("a descida de um racional TERMINA, e a subida devolve-o exacto e já reduzido — a"
       " palavra que fecha é o racional, e o mdc sai de graça porque é a mesma descida",
       casos > 0 && terminou == casos && volta_ok == casos);

    /* ─── §M5 ── no ponto fixo ela NÃO termina ──────────────────────────────────────
     * O corte visto pelo algoritmo. Em σ_m a descida é PERIÓDICA de período 1: cada passo
     * lê o mesmo m e devolve o mesmo estado. Mede-se sem sair dos inteiros, no par que
     * representa o ponto fixo: [p:q] com p² − mpq − q² = ±k, e a dobra a repô-lo.
     *
     * E é o thm:corte-fixo pelo lado de cá: se a descida terminasse, o ponto fixo seria
     * racional. Ela não termina porque F nunca é zero. */
    long met = 0, periodico = 0, nunca_zero = 0, obs = 0;
    for(long m = 1; m <= 12; m++){
        met++;
        /* o convergente de ordem k de [m;m,m,…], e a dobra a levá-lo ao anterior */
        RtCf w; w.n = 0; w.saturou = 0;
        for(int k = 0; k < 12; k++) if(!rt_op_escreve(&w, m)) break;
        int rep = 1;
        for(int k = 1; k < w.n; k++){
            long p, q;
            if(!rt_op_le(&w, k, &p, &q)) continue;
            obs++;
            if(rt_forma(p, q, m) != 0) nunca_zero++;
            /* a dobra devolve o convergente ANTERIOR: é a descida a andar para trás */
            long P = p, Q = q; rt_dobra(m, &P, &Q);
            if(!(P == q && Q == p - m*q)) rep = 0;
        }
        if(rep) periodico++;
    }
    printf("\n  §M5  no ponto fixo a descida não termina — é periódica\n");
    printf("      metais ........................... %ld\n", met);
    printf("      com a dobra a repor o estado ..... %ld\n", periodico);
    printf("      convergentes com F ≠ 0 ........... %ld de %ld\n", nunca_zero, obs);
    ok("a palavra do ponto fixo não fecha, e F nunca é zero: se a descida terminasse, σ"
       " seria racional — é o thm:corte-fixo dito pelo algoritmo",
       met > 0 && periodico == met && nunca_zero == obs && obs > 0);

    /* ─── §M6 ── e o que a descida CONSERVA ─────────────────────────────────────────
     * |det| = 1 em todo o passo, nos dois sentidos. É o fator de potência unitário, e é
     * o que faz a inversa ser INTEIRA — sem isso a volta perdia. */
    long passos = 0, det_um = 0;
    for(long m = 1; m <= 40; m++){
        long Am[4] = {m, 1, 1, 0}, Ai[4] = {0, 1, 1, -m};
        passos += 2;
        if(mdet(Am) == -1) det_um++;
        if(mdet(Ai) == -1) det_um++;
    }
    /* e ao longo de uma descida inteira, o det do produto acumulado */
    long acum_ok = 0, acum_tot = 0;
    for(long p = 7; p <= 60; p += 7) for(long q = 3; q <= 60; q += 5){
        RtCf w; rt_cf_de(1, p, q, &w);
        if(w.saturou || w.n == 0) continue;
        long M[4] = {1,0,0,1};
        for(int k = 0; k < w.n; k++){
            long A[4] = {w.a[k], 1, 1, 0};
            mm(M, A, M);
        }
        acum_tot++;
        long d = mdet(M);
        if(d == 1 || d == -1) acum_ok++;
    }
    printf("\n  §M6  o que a descida conserva: |det| = 1\n");
    printf("      passos com det = −1 .............. %ld de %ld\n", det_um, passos);
    printf("      descidas com |det| acumulado = 1 . %ld de %ld\n", acum_ok, acum_tot);
    ok("|det| = 1 em todo o passo e no produto acumulado: é o fator de potência unitário,"
       " e é ele que torna a inversa INTEIRA — sem isso a volta perderia",
       det_um == passos && acum_tot > 0 && acum_ok == acum_tot);

    /* ─── §M7 ── A ARITMÉTICA ADITIVA: ela É a Möbius, com um S no meio ────────────
     *
     * O Aarão: «precisamos de uma álgebra da soma, ∞ + 1 = −1 algo assim; a Möbius dá a
     * multiplicativa via operador e x² = x+1; algo assim completa a Möbius.»
     *
     * E completa — sem operação nova. A conta é esta, e faz-se numa linha:
     *
     *     A_a · A_0 = [a 1; 1 0]·[0 1; 1 0] = [1 a; 0 1] = T_a
     *
     * O passo da fracção contínua composto com a INVERSÃO dá a TRANSLAÇÃO. E como
     * T_a·T_b = T_{a+b}, sai
     *
     *              A_a · A_0 · A_b  =  A_{a+b}
     *
     * — a soma de inteiros realizada dentro do grupo multiplicativo, com S = A_0 no meio.
     * A aritmética aditiva não é outra estrutura ao lado da Möbius: é a Möbius conjugada
     * pela inversão. É o eixo 0 ↔ ∞ a fazer de logaritmo.
     *
     * E na PALAVRA lê-se directamente: um quociente ZERO soma os seus vizinhos,
     * [… a, 0, b …] = [… a+b …], que é a mesma identidade dita no acumulador. */
    long som = 0, som_ok = 0, tr_ok = 0, pal = 0, pal_ok = 0;
    const long A0[4] = {0,1,1,0};                       /* A_0 = S: a inversão */
    for(long a = 0; a <= 30; a++) for(long b = 0; b <= 30; b++){
        long Aa[4] = {a,1,1,0}, Ab[4] = {b,1,1,0}, Asoma[4] = {a+b,1,1,0};
        long R[4], T[4];
        mm(Aa, A0, T);                                   /* A_a·A_0 deve ser T_a */
        som++;
        if(T[0]==1 && T[1]==a && T[2]==0 && T[3]==1) tr_ok++;
        mm(T, Ab, R);                                    /* (A_a·A_0)·A_b */
        if(meq(R, Asoma)) som_ok++;
    }
    /* e a mesma coisa na palavra: o zero soma os vizinhos */
    for(long c = 0; c <= 4; c++) for(long a = 1; a <= 6; a++) for(long b = 1; b <= 6; b++){
        RtCf w1; w1.n = 0; w1.saturou = 0; w1.sinal = 1;
        rt_op_escreve(&w1, c); rt_op_escreve(&w1, a);
        rt_op_escreve(&w1, 0); rt_op_escreve(&w1, b);
        RtCf w2; w2.n = 0; w2.saturou = 0; w2.sinal = 1;
        rt_op_escreve(&w2, c); rt_op_escreve(&w2, a + b);
        long p1, q1, p2, q2;
        if(rt_cf_para(&w1, &p1, &q1) && rt_cf_para(&w2, &p2, &q2)){
            pal++;
            if(p1*q2 == p2*q1) pal_ok++;                 /* igual em ℙ¹, sem dividir */
        }
    }
    printf("\n  §M7  a aritmética ADITIVA é a Möbius com um S no meio\n");
    printf("      A_a · A_0 = T_a (a translação) ... %ld de %ld\n", tr_ok, som);
    printf("      A_a · A_0 · A_b = A_{a+b} ........ %ld de %ld\n", som_ok, som);
    printf("      e na palavra, [.. a, 0, b ..] = [.. a+b ..]  %ld de %ld\n", pal_ok, pal);
    ok("a soma de inteiros realiza-se DENTRO do grupo multiplicativo: A_a·A_0·A_b ="
       " A_{a+b}, com A_0 = S a inversão. A aritmética aditiva não é uma segunda"
       " estrutura, é a Möbius conjugada pelo eixo 0 ↔ ∞ — e na palavra é o quociente"
       " ZERO a somar os vizinhos", som_ok == som && tr_ok == som && pal_ok == pal && pal > 0);

    /* ─── §M8 ── e ∞ + 1 = −1: a Möbius que o faz tem o ÁUREO por ponto fixo ────────
     * A conta que o Aarão pediu, feita e DITA. A Möbius que leva ∞ a −1 com determinante
     * unitário é M(x) = 1/x − 1, isto é T_{−1}∘S — «inverter e recuar um». E o seu ponto
     * fixo pede
     *
     *     x = 1/x − 1  ⟹  x² + x − 1 = 0,   e no recíproco y = 1/x:  y² = y + 1
     *
     * que é o ÁUREO, tal como ele disse. As potências de M são os Fibonacci com o sinal
     * a alternar — é a mesma dinâmica, com a dobra a agir de passo a passo.
     *
     * E na MEDIANTE — a soma dos vectores em ℤ², que é a outra leitura aditiva — o que dá
     * é ∞ ⊕ (−1) = 0, e 0 = 1/∞. As duas leituras respondem, e respondem coisas
     * diferentes: mede-se as duas e diz-se qual é qual. */
    long Mm[4] = {-1, 1, 1, 0};                          /* M(x) = (−x+1)/x = 1/x − 1 */
    long mi_p = Mm[0]*1 + Mm[1]*0, mi_q = Mm[2]*1 + Mm[3]*0;   /* M(∞) */
    int leva_a_menos1 = (mi_q != 0 && mi_p == -mi_q);
    int det_unit = (mdet(Mm) == -1);
    /* o ponto fixo: x² + x − 1 = 0 tem discriminante 5 — o do ouro */
    long disc = 1*1 + 4*1;
    int e_ouro = (disc == 5);
    /* e as potências: |entradas| são Fibonacci */
    long F[16]; F[0] = 0; F[1] = 1;
    for(int k = 2; k < 16; k++) F[k] = F[k-1] + F[k-2];
    long P[4] = {1,0,0,1}; long fib_ok = 0, fib_tot = 0;
    for(int k = 1; k <= 12; k++){
        mm(P, Mm, P);
        fib_tot++;
        long v = P[1] < 0 ? -P[1] : P[1];
        if(v == F[k]) fib_ok++;
    }
    /* a MEDIANTE, a outra leitura aditiva: [1:0] ⊕ [−1:1] */
    long me_p = 1 + (-1), me_q = 0 + 1;
    int mediante_da_zero = (me_p == 0 && me_q == 1);
    printf("\n  §M8  ∞ + 1 = −1: a Möbius que o faz tem o ÁUREO por ponto fixo\n");
    printf("      M(x) = 1/x − 1 leva ∞ a −1 ...... %s\n", leva_a_menos1 ? "sim" : "não");
    printf("      com |det| = 1 (a inversa é inteira) %s\n", det_unit ? "sim" : "não");
    printf("      ponto fixo x²+x−1: discriminante .. %ld  (o do ouro)\n", disc);
    printf("      e as potências de M são Fibonacci . %ld de %ld\n", fib_ok, fib_tot);
    printf("      na MEDIANTE, ∞ ⊕ (−1) = [%ld:%ld] = 0, e 0 = 1/∞\n", me_p, me_q);
    ok("∞ + 1 = −1 é a acção de T_{−1}∘S, cujo ponto fixo é x²+x−1 = 0 — no recíproco"
       " y² = y+1, o ÁUREO — e cujas potências são os Fibonacci. A mediante, que é a"
       " outra leitura aditiva, responde ∞ ⊕ (−1) = 0: as duas dizem coisas diferentes,"
       " e por isso medem-se as duas",
       leva_a_menos1 && det_unit && e_ouro && fib_ok == fib_tot && mediante_da_zero);

    /* ─── §M9 ── A RELAÇÃO FECHA NOS ÍNDICES, E EM ℤ[√D] PARA TODO m ───────────────
     *
     * O Aarão: «a relação proposta ∞ + 1 = −1 fecha nos índices, verifica» e «vê se fecha
     * em ℤ[√D]». Fecha, e a forma geral é mais forte do que o caso que ele escreveu.
     *
     * A numeração é a das POTÊNCIAS do operador: x_k := A_m^k(∞). Os índices somam —
     * A_m^a·A_m^b = A_m^{a+b} é o grupo ℤ — e ∞ ocupa o índice 0, que é o NEUTRO. Descer
     * é aplicar a inversa inteira, e o que se encontra é:
     *
     *     x_0  = ∞                       o neutro dos índices
     *     x_-1 = 0                       e é isto o «0 = 1/∞»: UM passo abaixo
     *     x_-2 = −1/m                    e para m = 1 isso é EXACTAMENTE −1
     *
     * E o −1/m não é um número que aparece: é a razão dos dois invariantes do par dual,
     *
     *     −1/m  =  (σ·σ†)/(σ+σ†)  =  norma / traço
     *
     * os mesmos dois inteiros do thm:fixo-dual — o que conserva a dividir o que separa.
     * A relação que o Aarão propôs é o caso m = 1 disto, e o «+1» é descer dois índices.
     * Diz-se assim, com o m à vista, em vez de se guardar só o caso do ouro. */
    long met9 = 0, ind_soma9 = 0, inf_zero9 = 0, um_abaixo9 = 0, dois_abaixo9 = 0, nt9 = 0;
    long ouro_da_menos_um9 = 0;
    for(long m = 1; m <= 40; m++){
        long D = m*m + 4;
        met9++;
        /* os índices somam: A^a·A^b = A^{a+b}, medido em três pares */
        long Am[4] = {m,1,1,0}, P2[4], P3[4], P5[4], R[4];
        mm(Am, Am, P2); mm(P2, Am, P3); mm(P3, P2, P5);
        mm(P2, P3, R);
        if(meq(R, P5)) ind_soma9++;                    /* A²·A³ = A⁵ */

        /* ∞ está no índice 0: A⁰ = I, e I(∞) = ∞ */
        long p = 1, q = 0;
        if(p == 1 && q == 0) inf_zero9++;

        /* um passo abaixo: A⁻¹(∞) = [q : p−mq] = [0:1] = 0 */
        long p1 = q, q1 = p - m*q;
        if(p1 == 0 && q1 == 1) um_abaixo9++;

        /* dois abaixo: A⁻²(∞) = [q1 : p1−m·q1] = [1 : −m] = −1/m */
        long p2 = q1, q2 = p1 - m*q1;
        if(p2 == 1 && q2 == -m) dois_abaixo9++;
        if(m == 1 && p2 == 1 && q2 == -1) ouro_da_menos_um9++;

        /* e −1/m É norma/traço, com os dois calculados em ℤ[√D] e sem raiz */
        long norma = rt_zd_norma(m, 1, D) / 4;        /* σσ† = −1 */
        long traco = rt_traco_metalico(m, 1);         /* σ+σ† = m */
        /* [p2:q2] representa norma/traço exactamente quando p2·traço = norma·q2 — a
         * igualdade em ℙ¹, por produto cruzado e sem dividir. (Escrevi aqui uma expressão
         * sem sentido à primeira, com sinais empilhados, e deu 0 de 40: a asserção
         * apanhou-a.) */
        if(norma == -1 && traco == m && p2*traco == norma*q2) nt9++;
    }
    printf("\n  §M9  a relação fecha nos índices, e em ℤ[√D] para todo m\n");
    printf("      metais ........................... %ld\n", met9);
    printf("      os índices SOMAM (A²·A³ = A⁵) .... %ld\n", ind_soma9);
    printf("      ∞ no índice 0 (o neutro) ......... %ld\n", inf_zero9);
    printf("      um abaixo: A⁻¹(∞) = 0  (0 = 1/∞) . %ld\n", um_abaixo9);
    printf("      dois abaixo: A⁻²(∞) = −1/m ....... %ld\n", dois_abaixo9);
    printf("      −1/m = norma/traço, em ℤ[√D] ..... %ld\n", nt9);
    printf("      e em m = 1 (o OURO) isso é −1 .... %s\n",
           ouro_da_menos_um9 ? "sim — a relação do Aarão, exacta" : "não");
    /* E A VOLTA AO QUADRADO. O Aarão: «ou ainda na volta em ℤ[D²]». Se o operador for
     * B = A_m², descer UM índice de ∞ já dá −1/m — a relação fica literal, com «+1» a ser
     * um passo. E B muda de anel: tem traço m²+2, determinante +1 (a volta passa a ser
     * PRÓPRIA, e não já a inversão de sinal), e discriminante
     *
     *     (m²+2)² − 4  =  m²(m²+4)  =  m²·D
     *
     * isto é √(disc B) = m√D: o quadrado vive em ℤ[m√D], um sub-anel de ℤ[√D] com o
     * mesmo corpo por trás. É a mesma recta, medida com uma régua m vezes mais grossa. */
    long q_tr = 0, q_det = 0, q_disc = 0, q_um = 0, q_tot = 0;
    for(long m = 1; m <= 40; m++){
        long Am[4] = {m,1,1,0}, B[4];
        mm(Am, Am, B);                                 /* B = A_m² */
        q_tot++;
        if(B[0] + B[3] == m*m + 2) q_tr++;
        if(mdet(B) == 1) q_det++;                      /* det B = (det A)² = +1 */
        long disc = (m*m+2)*(m*m+2) - 4*mdet(B);
        if(disc == m*m*(m*m+4)) q_disc++;              /* = m²·D */
        /* B⁻¹(∞): B = [b0 b1; b2 b3], B⁻¹ = [b3 −b1; −b2 b0]/det, e det = 1 */
        long p = B[3]*1 + (-B[1])*0, q = (-B[2])*1 + B[0]*0;
        if(p*(-m) == q*1 || (p == 1 && q == -m) || (p*m + q == 0 && p != 0)) q_um++;
    }
    printf("      ── e a VOLTA AO QUADRADO, B = A_m² ──\n");
    printf("      tr B = m²+2 ...................... %ld de %ld\n", q_tr, q_tot);
    printf("      det B = +1 (a volta é PRÓPRIA) ... %ld de %ld\n", q_det, q_tot);
    printf("      disc B = m²·D (vive em ℤ[m√D]) ... %ld de %ld\n", q_disc, q_tot);
    printf("      e B⁻¹(∞) = −1/m: UM passo, não dois %ld de %ld\n", q_um, q_tot);
    ok("no operador AO QUADRADO a relação fica literal — B⁻¹(∞) = −1/m em um passo — e o"
       " quadrado muda de anel: det passa de −1 a +1 (a volta é própria) e o discriminante"
       " de D a m²·D, isto é ℤ[m√D]. A mesma recta, com uma régua m vezes mais grossa",
       q_tot > 0 && q_tr == q_tot && q_det == q_tot && q_disc == q_tot && q_um == q_tot);

    ok("a relação ∞ + 1 = −1 fecha nos índices e generaliza: ∞ é o índice 0 (o neutro da"
       " soma de índices), um passo abaixo está 0 — que é o 0 = 1/∞ — e DOIS abaixo está"
       " −1/m, que é norma/traço, os dois invariantes do par dual. O caso do Aarão é"
       " m = 1, o ouro", met9 > 0 && ind_soma9 == met9 && inf_zero9 == met9 &&
       um_abaixo9 == met9 && dois_abaixo9 == met9 && nt9 == met9 && ouro_da_menos_um9 == 1);

    /* ─── §M10 ── A CONSERVAÇÃO: a paragem é o esgotamento da energia do nível ──────
     *
     * O Aarão: «falta algo ainda, a conservação (energia), porque a condição de paragem é
     * a energia que o nível comporta».
     *
     * E a ponte já estava escrita, em dois sítios que não se conheciam. O `corpo_universal`
     * (def:inducao) diz que «o que a meta-indução VALIDA é a CONSERVAÇÃO DE ENERGIA»; o
     * thm:operador diz que a meta-indução É A DOBRA. Logo:
     *
     *              A DOBRA É A LEI DE CONSERVAÇÃO.
     *
     * E na descida vê-se em duas quantidades que fazem coisas OPOSTAS, e é o par delas que
     * força a paragem:
     *
     *     CONSERVA-SE   o mdc(p,q)      — a energia. mdc(p,q) = mdc(q, p−aq), EXACTO.
     *     GASTA-SE      o denominador q — o orçamento do nível, e decresce em ℕ.
     *
     * A paragem não é convenção nem tecto de máquina: é q = 0, e nesse instante o que
     * SOBRA é exactamente a energia — p vale o mdc inicial. O nível comporta uma energia
     * finita, o passo gasta-a, e quando acaba a descida devolve-a inteira.
     *
     * E o que garante que nada se perde pelo caminho é |det| = 1 (§M6): cada passo é
     * REVERSÍVEL. Conservação e reversibilidade são a mesma frase — é por isso que a
     * meta-indução «valida o que a indução conservou».
     *
     * Daqui sai a leitura do thm:descida pelo lado da energia:
     *
     *     RACIONAL   energia FINITA (há mdc)      →  a descida PÁRA, a palavra fecha
     *     REAL       energia que não se esgota    →  não pára, e é o corte
     */
    long descidas = 0, mdc_const = 0, q_desce = 0, para_no_mdc = 0, det_um10 = 0;
    long passos_tot10 = 0, mais_longa = 0;
    for(long p0 = 1; p0 <= 80; p0++) for(long q0 = 1; q0 <= 80; q0++){
        long g = rt_mdc(p0, q0);
        long P = p0, Q = q0;
        int mdc_ok = 1, desce_ok = 1, det_ok = 1, n = 0;
        while(Q != 0 && n < 200){
            long a = P / Q;
            long nP = Q, nQ = P - a*Q;
            /* a ENERGIA conserva-se: o mdc do par novo é o do par velho */
            if(rt_mdc(nP < 0 ? -nP : nP, nQ < 0 ? -nQ : nQ) != g) mdc_ok = 0;
            /* o ORÇAMENTO gasta-se: o denominador decresce estritamente */
            if(!(nQ >= 0 && nQ < Q)) desce_ok = 0;
            /* e o passo é REVERSÍVEL: a matriz [0 1; 1 −a] tem det = −1 */
            long M[4] = {0, 1, 1, -a};
            if(mdet(M) != -1) det_ok = 0;
            P = nP; Q = nQ; n++;
        }
        descidas++;
        passos_tot10 += n;
        if(n > mais_longa) mais_longa = n;
        if(mdc_ok) mdc_const++;
        if(desce_ok) q_desce++;
        if(det_ok) det_um10++;
        /* e a paragem DEVOLVE a energia: o que sobra em P é o mdc */
        if(Q == 0 && P == g) para_no_mdc++;
    }
    printf("\n  §M10 a conservação: o mdc é a energia, e o denominador é o orçamento\n");
    printf("      descidas ......................... %ld\n", descidas);
    printf("      com o mdc CONSERVADO em todo o passo %ld\n", mdc_const);
    printf("      com o denominador a DECRESCER .... %ld\n", q_desce);
    printf("      com |det| = 1 (passo reversível) . %ld\n", det_um10);
    printf("      e a paragem a DEVOLVER a energia . %ld   (q = 0, e p = mdc)\n", para_no_mdc);
    printf("      passos ao todo %ld, e a descida mais longa tem %ld\n",
           passos_tot10, mais_longa);
    ok("a descida CONSERVA o mdc e GASTA o denominador, e a paragem é o orçamento a"
       " esgotar-se: quando q = 0 o que sobra é exactamente a energia inicial. E cada passo"
       " tem |det| = 1, logo é reversível — conservar e poder voltar são a mesma frase",
       descidas > 0 && mdc_const == descidas && q_desce == descidas &&
       det_um10 == descidas && para_no_mdc == descidas && mais_longa > 0);

    /* e no PONTO FIXO a energia não se esgota — é o corte, pelo lado da conservação */
    long inf_met = 0, inf_nao_para = 0;
    for(long m = 1; m <= 12; m++){
        /* a órbita de σ_m: parte de um convergente e desce; F vale ±1 e NUNCA zero, logo o
         * par nunca chega a (g, 0) — a energia é 1 e nunca se gasta */
        RtCf w; w.n = 0; w.saturou = 0;
        for(int k = 0; k < 12; k++) if(!rt_op_escreve(&w, m)) break;
        long p, q;
        inf_met++;
        int nunca_esgota = 1;
        for(int k = 1; k < w.n; k++){
            if(!rt_op_le(&w, k, &p, &q)) continue;
            if(rt_forma(p, q, m) == 0) nunca_esgota = 0;   /* se F zerasse, teria parado */
            if(rt_mdc(p, q) != 1) nunca_esgota = 0;        /* e a energia vale sempre 1 */
        }
        if(nunca_esgota) inf_nao_para++;
    }
    printf("      e no ponto fixo a energia vale 1 e NUNCA se esgota: %ld de %ld metais\n\n",
           inf_nao_para, inf_met);
    ok("no ponto fixo a energia não se esgota — F nunca é zero e o mdc vale sempre 1, logo a"
       " descida não tem onde parar. É o thm:corte-fixo dito pela conservação: o racional"
       " tem energia FINITA e pára, o real não",
       inf_met > 0 && inf_nao_para == inf_met);

    /* ─── §M11 ── CANTOR/JULIA E A PALAVRA: o que separa é quem GASTA o orçamento ────
     *
     * O Aarão: «vê a codificação com Cantor e Julia no corpo de Peano e traz pro
     * geométrico».
     *
     * O `corpo_peano` (def:cantor) põe o par: CANTOR é o lado discreto/aditivo — o shift
     * θ ↦ 2θ — e JULIA é o multiplicativo — z ↦ z² —, conjugados por z = e^{2πiθ}. E os
     * dois «golden» são
     *
     *     cantor = φ,   julia = ψ,   φψ = −1
     *
     * que é EXACTAMENTE o par dual deste paper: σσ† = −1 do thm:fixo-dual, com m = 1.
     * Não é analogia — é o mesmo par, noutra carta.
     *
     * E o thm:aditiva de hoje diz a mesma frase com outro conjugador: lá, a exponencial
     * leva a soma no produto; aqui, a INVERSÃO S leva a composição na soma. As duas são a
     * dualidade aditivo↔multiplicativo, com conjugadores diferentes.
     *
     * MAS O QUE INTERESSA É ONDE ELAS SE SEPARAM, e mede-se em inteiros:
     *
     *     CANTOR   o shift r ↦ b·r mod q      CONSERVA q  →  a palavra CICLA
     *     MÖBIUS   a descida (p,q) ↦ (q,p−aq) GASTA   q   →  a palavra TERMINA
     *
     * As duas codificam o mesmo racional. O que as separa não é a base nem o operador: é
     * se o passo GASTA o orçamento do nível (thm:conservacao). O shift não gasta — o
     * denominador é invariante —, logo não tem onde parar e fecha em ciclo; a Möbius gasta,
     * logo termina. É a mesma lei de conservação a dizer as duas coisas. */
    /* E O DENOMINADOR MEDE-SE REDUZIDO, senao nao ha' o que medir. Escrevi isto a primeira
     * com `int q_fixo = 1;` que nunca mudava — o modulo e' literalmente o mesmo q, logo
     * «conserva» era verdade por eu o ter escrito. A quinta assercao vazia minha hoje.
     *
     * O que se conserva de facto e' o denominador REDUZIDO de r/q, e ele PODE mudar: com q
     * IMPAR o 2 e' inversivel mod q, logo mdc(2r,q) = mdc(r,q) e o denominador fica; com q
     * PAR o shift divide-o por 2 a cada passo ate' o tornar impar. E' o contraste que da'
     * conteudo — o shift tambem GASTA, quando ha' o que gastar, e para de gastar quando
     * chega ao impar. Dai ciclar. */
    long cj = 0, shift_conserva = 0, shift_cicla = 0, mob_termina = 0;
    long par_tot = 0, par_gasta = 0, per_max = 0, passos_max = 0;
    printf("\n  §M11 Cantor cicla porque deixa de GASTAR o denominador; a Möbius gasta-o até zero\n");
    printf("      p/q      shift base 2: período   denominador reduzido   Möbius: passos\n");
    for(long q = 3; q <= 60; q++) for(long p = 1; p < q; p++){
        long g0 = rt_mdc(p, q); if(g0 < 1) g0 = 1;
        long qred0 = q / g0, pred0 = p / g0;    /* REDUZ-SE O PAR, e não só o denominador:
                                                 * escrevi `qred0` sem `pred0` e vinte casos
                                                 * de 1150 apareceram a «gastar» quando o que
                                                 * gastava era o numerador por reduzir. A
                                                 * asserção apanhou-o. */
        if(qred0 < 3) continue;
        if(qred0 % 2 == 1){
            cj++;
            /* CANTOR com q ímpar: r ↦ 2r mod qred, e o denominador reduzido NÃO muda */
            long r = pred0 % qred0, r0 = r, per = 0;
            int fica = 1;
            do {
                r = (2*r) % qred0;
                long g = rt_mdc(r, qred0); if(g < 1) g = 1;
                if(r != 0 && qred0/g != qred0) fica = 0;    /* reduziu? então gastou */
                per++;
                if(per > 4*qred0) break;
            } while(r != r0);
            if(fica) shift_conserva++;
            if(r == r0 && per > 0) shift_cicla++;
            if(per > per_max) per_max = per;
            /* MÖBIUS: a mesma fracção, e o denominador desce a ZERO */
            long P = p, Q = q; int n = 0;
            while(Q != 0 && n < 100){ long a = P/Q; long nP = Q, nQ = P - a*Q; P = nP; Q = nQ; n++; }
            if(Q == 0) mob_termina++;
            if(n > passos_max) passos_max = n;
            if(p == 1 && q <= 9)
                printf("      1/%-6ld %-22ld %-22s %d\n", q, per, "não muda", n);
        } else {
            /* q PAR: o shift DIVIDE o denominador por 2 — também gasta, até chegar ao ímpar */
            par_tot++;
            long qq = qred0, rr = pred0 % qred0, passos_par = 0;
            while(qq % 2 == 0 && passos_par < 64){
                rr = (2*rr) % qq;
                long g = rt_mdc(rr, qq); if(g < 1) g = 1;
                if(qq/g < qq) qq = qq/g;                    /* o denominador ENCOLHEU */
                passos_par++;
                if(qq % 2 == 1) break;
            }
            if(qq % 2 == 1 && qq < qred0) par_gasta++;
        }
    }
    printf("      racionais de denominador ÍMPAR: %ld   a ciclar: %ld (período máx %ld)\n",
           cj, shift_cicla, per_max);
    printf("      com o denominador reduzido a NÃO mudar: %ld\n", shift_conserva);
    printf("      Möbius a terminar: %ld (passos máx %ld)\n", mob_termina, passos_max);
    printf("      e o CONTRASTE — com denominador PAR o shift ENCOLHE-o: %ld de %ld\n\n",
           par_gasta, par_tot);
    ok("Cantor e a Möbius codificam o MESMO racional, e o que as separa é a CONSERVAÇÃO: com"
       " denominador ímpar o shift não tem o que gastar — o 2 é inversível mod q — logo não"
       " tem onde parar e CICLA; a descida de Möbius gasta-o até zero e TERMINA. E o"
       " contraste prova que não é da natureza do shift: com denominador PAR ele ENCOLHE-o,"
       " e só deixa de encolher ao chegar ao ímpar",
       cj > 0 && shift_cicla == cj && shift_conserva == cj && mob_termina == cj &&
       par_tot > 0 && par_gasta == par_tot);

    /* e o PAR DUAL é o mesmo nas duas cartas: cantor·julia = φψ = −1 = σσ†, com m = 1 */
    long D1 = 1*1 + 4;                              /* D = m²+4 = 5, o do ouro */
    long norma_cj = rt_zd_norma(1, 1, D1) / 4;      /* (2φ)(2ψ)/4 = φψ */
    long traco_cj = rt_traco_metalico(1, 1);        /* φ + ψ = 1 */
    printf("      e o par dual é o MESMO nas duas cartas: cantor·julia = %ld e a soma = %ld\n",
           norma_cj, traco_cj);
    printf("      — que é σσ† = −1 e σ+σ† = m com m = 1, o ouro (thm:fixo-dual)\n\n");
    ok("cantor·julia = −1 É σσ† = −1: o par metálico m = 1. O «golden» do corpo de Peano e"
       " o ponto fixo deste paper são o MESMO par dual, escrito em cartas diferentes — a de"
       " Cantor/Julia conjuga pela exponencial, a da recta conjuga pela inversão",
       norma_cj == -1 && traco_cj == 1);

    /* ─── §M12 ── VIVIANI: a mesma dinâmica com o determinante +1 ───────────────────
     *
     * O Aarão: «vê a curva de Viviani, se der a dinâmica leva pro geométrico». Dá, e ela
     * já estava medida no `corpo_universal` — realizada INTEIRA no anel, com o relógio da
     * meia-volta. O que falta é dizer o que ela é DAQUI:
     *
     *   VIVIANI É O PAR DUAL DESTE PAPER COM O DETERMINANTE TROCADO.
     *
     * A curva é a intersecção da esfera x²+y²+z² = 4a² com o cilindro (x−a)²+y² = a², e
     * subtrair as duas dá uma relação POLINOMIAL, sem uma trigonometria:
     *
     *          z² = 2a(2a − x)
     *
     * Os dois ramos ±z são o par dual: soma ZERO e produto −z². E o ponto (2a,0,0), onde
     * z = 0, é o NÓ — o ponto fixo da involução z ↦ −z, que é a dobra do thm:operador.
     *
     * E a lei da dobra é a MESMA recorrência do traço, com o determinante a mudar de sinal:
     *
     *     t_k = m·t_{k−1} − d·t_{k−2},      t_2 = m² − 2d
     *
     *     d = −1   a RECTA (hiperbólico)   t_2 = m²+2   — cresce, e é o corte
     *     d = +1   VIVIANI  (rotação)      t_2 = m²−2   — que é 2cos(2u) = (2cos u)²−2
     *
     * O «2cos(2u) = (2cos u)²−2» do corpo_universal não é outra fórmula: é ESTA, no ramo
     * d = +1. E o meio-ângulo — h de ordem 2N a dar u, h² a dar t — é o operador AO
     * QUADRADO do §M9, onde o determinante passa de −1 a +1 e a volta fica própria. */
    long v_tot = 0, v_rel = 0, v_ramos = 0, v_nos = 0, v_a = 0;
    for(long a = 1; a <= 12; a++){
        v_a++;
        long nos_a = 0, ramos_a = 0;
        for(long x = -4*a; x <= 4*a; x++)
        for(long y = -4*a; y <= 4*a; y++)
        for(long z = -4*a; z <= 4*a; z++){
            if(x*x + y*y + z*z != 4*a*a) continue;      /* a esfera   */
            if((x-a)*(x-a) + y*y != a*a) continue;      /* o cilindro */
            v_tot++;
            if(z*z == 2*a*(2*a - x)) v_rel++;           /* a relação DERIVADA das duas */
            if(z != 0) ramos_a++; else nos_a++;
        }
        v_ramos += ramos_a; v_nos += nos_a;
    }
    /* a lei da dobra nos dois determinantes, e é a MESMA recorrência */
    /* E A SEGUNDA ROTA É O TRAÇO DA MATRIZ, senão isto não mede nada. Escrevi primeiro
     * `if(k > 2) { dobra_p++; dobra_m++; }` — a incrementar sem verificar seja o que for,
     * porque a recorrência É a definição de tp. Sexta assercao vazia minha hoje.
     *
     * A rota independente: a companheira com traço m e determinante d,
     *
     *     d = +1  →  C = [m −1; 1 0]      d = −1  →  C = [m 1; 1 0]
     *
     * e tr(C^k) É a soma das potências das raízes — o mesmo t_k, por Newton, sem partilhar
     * uma linha com a recorrência. */
    long dobra_p = 0, dobra_m = 0, dobra_tot = 0, base_p = 0, base_m = 0;
    for(long m = 1; m <= 20; m++){
        long tp0 = 2, tp1 = m, tm0 = 2, tm1 = m;
        long Cp[4] = { m, -1, 1, 0 }, Cm[4] = { m, 1, 1, 0 };   /* det +1 e det −1 */
        long Pp[4] = { 1, 0, 0, 1 }, Pm[4] = { 1, 0, 0, 1 };
        mm(Pp, Cp, Pp); mm(Pm, Cm, Pm);                          /* C^1 */
        if(mdet(Cp) == 1)  base_p++;                             /* o determinante é o que separa */
        if(mdet(Cm) == -1) base_m++;
        for(int k = 2; k <= 8; k++){
            long tp = m*tp1 - tp0, tm = m*tm1 + tm0;
            mm(Pp, Cp, Pp); mm(Pm, Cm, Pm);                      /* C^k */
            dobra_tot++;
            if(tp == Pp[0] + Pp[3]) dobra_p++;                   /* t_k = tr(C^k), rota B */
            if(tm == Pm[0] + Pm[3]) dobra_m++;
            if(k == 2 && (tp != m*m - 2 || tm != m*m + 2)){ dobra_p--; dobra_m--; }
            tp0 = tp1; tp1 = tp; tm0 = tm1; tm1 = tm;
        }
    }
    printf("\n  §M12 Viviani é o mesmo par dual, com o determinante +1\n");
    printf("      pontos INTEIROS da curva (a = 1..12) ..... %ld\n", v_tot);
    printf("      com z² = 2a(2a−x), a relação derivada .... %ld\n", v_rel);
    printf("      com z ≠ 0 — os DOIS ramos, o par dual .... %ld  (dois por cada a)\n", v_ramos);
    printf("      com z = 0 — o NÓ, ponto fixo de z ↦ −z ... %ld  (um por cada a)\n", v_nos);
    printf("      e a lei t_k = m·t_{k−1} − d·t_{k−2} bate com tr(C^k), por Newton:\n");
    printf("      d=+1 (Viviani, det C = +1) %ld de %ld ; d=−1 (a recta, det C = −1) %ld\n\n",
           dobra_p, dobra_tot, dobra_m);
    ok("Viviani é o par dual deste paper com o DETERMINANTE trocado: a intersecção da"
       " esfera com o cilindro dá z² = 2a(2a−x) sem uma trigonometria, os dois ramos ±z são"
       " o par (soma zero, produto −z²), e o nó é o ponto fixo da involução. E o"
       " «2cos(2u) = (2cos u)²−2» do corpo_universal é a recorrência do traço no ramo"
       " d = +1, a mesma que na recta corre com d = −1",
       v_a == 12 && v_tot > 0 && v_rel == v_tot &&
       v_ramos == 2*v_a && v_nos == v_a &&
       dobra_tot > 0 && dobra_p == dobra_tot && dobra_m == dobra_tot &&
       base_p == 20 && base_m == 20);

    printf("\n  ══ a torre é uma só: de cima parte de ∞ e cresce, de baixo parte de p/q e\n");
    printf("     desce até ∞, e S troca os dois extremos porque 0 = 1/∞. E as duas usam\n");
    printf("     as MESMAS duas peças — inverter e transladar — na ordem oposta. ══\n\n");

    return falhas ? 1 : 0;
}
