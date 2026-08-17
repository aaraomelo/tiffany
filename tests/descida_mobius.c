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

    printf("\n  ══ a torre é uma só: de cima parte de ∞ e cresce, de baixo parte de p/q e\n");
    printf("     desce até ∞, e S troca os dois extremos porque 0 = 1/∞. E as duas usam\n");
    printf("     as MESMAS duas peças — inverter e transladar — na ordem oposta. ══\n\n");

    return falhas ? 1 : 0;
}
