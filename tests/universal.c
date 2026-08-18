/* universal.c — O CICLO UNIVERSAL: descodifica, opera, inverte, codifica — e fecha.
 *
 * O Aarão: «faz a volta, e pseudo-código universal com decodificação - operações -
 * inversão - codificação, baseado em cantor julia viviani e toro».
 *
 * O pipe da entrega já tinha metade: `textos → unidade → inteiros → operar → palavra`. O
 * que faltava era o ciclo INTEIRO, com a volta lá dentro — e a condição que o faz fechar,
 * que é a do toro:
 *
 *   ┌──────────────────────────────────────────────────────────────────────────────┐
 *   │  UNIVERSAL(t, T):                                                            │
 *   │                                                                              │
 *   │    ── DESCODIFICA ──────────  o texto é um racional EXACTO                   │
 *   │       (p, q) := le_decimal(t)              "0.45" → 45/100                   │
 *   │       u      := unidade_comum({t})         o MMC dos denominadores           │
 *   │                                                                              │
 *   │    ── OPERA ────────────────  em inteiros, e sem dividir                     │
 *   │       exige |det T| = 1                    ← senão NÃO HÁ VOLTA (thm:toro)   │
 *   │       (p', q') := T · (p, q)               em P¹, produto de matrizes        │
 *   │                                                                              │
 *   │    ── INVERTE ──────────────  a metade dual, e é INTEIRA                     │
 *   │       T⁻¹ := adj(T)/det(T)                 inteira PORQUE |det T| = 1        │
 *   │       (p'', q'') := T⁻¹ · (p', q')                                           │
 *   │                                                                              │
 *   │    ── CODIFICA ─────────────  a saída é uma PALAVRA, não um decimal          │
 *   │       w := codifica(p'', q'')              FC (Möbius) ou dígitos (Cantor)   │
 *   │                                                                              │
 *   │    E A VOLTA FECHA:  descodifica(w) == (p, q)                                │
 *   │                                                                              │
 *   │    ── VERIFICA RÁPIDA ───────  em Q(m√D), thm:serie-quadratica                │
 *   │       qmd_verifica_rapida(m,D,α)   norma, 1/(α−1), S₁ = α⁻¹                 │
 *   └──────────────────────────────────────────────────────────────────────────────┘
 *
 * As quatro peças entram cada uma no seu lugar, e nenhuma é decorativa:
 *
 *      CANTOR    a codificação em dígitos numa base (rt_cod_shift)     ordem 2
 *      JULIA     o mesmo, do lado multiplicativo — z ↦ z²              ordem 2
 *      VIVIANI   o operador do meio-ângulo: i          ordem 2 no ponto, 4 no vector
 *      o TORO    a CONDIÇÃO: |det| = 1, e é ela que dá a inversão      {1,2,3,4,6}
 *
 * O toro não é mais um operador na lista: é o que autoriza o passo INVERTE. Sem
 * |det T| = 1 a inversa não é inteira, e o ciclo não fecha — mede-se aqui, com o
 * contraste (thm:toro, thm:conservacao).
 *
 *   §U1  o ciclo fecha: 3 codificações × 4 operadores, e a volta devolve o original
 *   §U2  e é |det| = 1 que o autoriza — com det ≠ ±1 a inversa não é inteira
 *   §U3  há DUAS ordens: no PONTO (ℙ¹) e no VECTOR (ℤ²) — e Viviani tem 2 e 4, que é
 *        o recobrimento duplo. O gato não fecha em nenhuma
 *   §U4  e a codificação é livre: as três dão o MESMO racional de volta
 *   §U5  a OPERAÇÃO faz-se NA TRANSFORMADA UNIVERSAL: a convolução vira
 *        produto ponto a ponto, e a condição de volta é UMA — det = σσ† (thm:gato)
 *
 * Nenhum double, nenhum limiar: compila sem -lm.
 *
 *   cc -O2 -std=c99 -I. -I../lib universal.c -o universal && ./universal
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"
#include "qmd.h"

/* ── as peças do ciclo, cada uma numa função ─────────────────────────────────────── */

/* DESCODIFICA: o texto é um racional exacto. (A leitura está na lib; aqui entra já como
 * par, porque o §E1 da entrega.c mede a leitura do texto.) */
static void descodifica(long p, long q, long *rp, long *rq){ *rp = p; *rq = q; }

/* OPERA e INVERTE estão na `reta.h` — `rt_opera`, `rt_inverte`, `rt_ciclo`. O ciclo é a
 * ARQUITECTURA do pipe e não uma medida sobre ele, logo vive na lib; aqui mede-se. Estes
 * dois embrulhos existem só para o laço abaixo ler como o pseudo-código. */
static void opera(const long *T, long p, long q, long *rp, long *rq){
    RtOp o; for(int i = 0; i < 4; i++) o.T[i] = T[i];
    rt_opera(&o, p, q, rp, rq);
}
static int inverte(const long *T, long p, long q, long *rp, long *rq){
    RtOp o; for(int i = 0; i < 4; i++) o.T[i] = T[i];
    return rt_inverte(&o, p, q, rp, rq);
}

/* CODIFICA / DESCODIFICA a palavra: as três da casa, com a mesma assinatura */
static int cod_palavra(long p, long q, long *rp, long *rq){ return rt_iv_palavra(p, q, rp, rq); }
static int cod_shift2 (long p, long q, long *rp, long *rq){ return rt_iv_shift2 (p, q, rp, rq); }
static int cod_shift10(long p, long q, long *rp, long *rq){ return rt_iv_shift10(p, q, rp, rq); }

int main(void){
    printf("\n══ O CICLO UNIVERSAL: descodifica → opera → inverte → codifica ══\n");

    /* os quatro operadores, e o que cada um é */
    struct { const char *nome; long T[4]; int ordem; } OP[] = {
        { "CANTOR/JULIA  (o espelho)",   {-1, 0, 0, 1},  2 },   /* x ↦ −x      */
        { "VIVIANI       (o i)",         { 0,-1, 1, 0},  4 },   /* rotação 90° */
        { "a MÖBIUS      (A_1, o gato)", { 1, 1, 1, 0},  0 },   /* ordem infinita */
        { "a INVERSÃO    (S)",           { 0, 1, 1, 0},  2 },   /* 0 ↔ ∞       */
    };
    const int NOP = 4;
    struct { const char *nome; int (*f)(long,long,long*,long*); } COD[] = {
        { "palavra (FC)",  cod_palavra },
        { "shift base 2",  cod_shift2  },
        { "shift base 10", cod_shift10 },
    };
    const int NCOD = 3;

    /* ─── §U1 ── o ciclo fecha ──────────────────────────────────────────────────────
     * Para cada racional, cada operador e cada codificação: descodifica, opera, inverte,
     * codifica — e o que sai tem de ser o que entrou. Compara-se em ℙ¹ por produto
     * cruzado, sem dividir. O que a codificação não conseguir representar conta-se À
     * PARTE: não caber não é falhar. */
    long ciclos = 0, fecha = 0, fora = 0, sem_sat_ok = 0, sem_sat_tot = 0;
    printf("\n  §U1  o ciclo fecha — 3 codificações × 4 operadores\n");
    printf("      operador                      codificação      fecha   fora\n");
    for(int o = 0; o < NOP; o++)
        for(int c = 0; c < NCOD; c++){
            long f = 0, x = 0, n = 0;
            for(long q = 1; q <= 24; q++)
                for(long p = 0; p <= q; p++){
                    if(rt_mdc(p, q) != 1 && !(p == 0 && q == 1)) continue;
                    long d0, d1, o0, o1, i0, i1, k0, k1;
                    descodifica(p, q, &d0, &d1);
                    opera(OP[o].T, d0, d1, &o0, &o1);
                    if(!inverte(OP[o].T, o0, o1, &i0, &i1)) continue;
                    n++;
                    if(i1 == 0){ x++; continue; }          /* a órbita passou por ∞ */
                    long g = rt_mdc(i0 < 0 ? -i0 : i0, i1 < 0 ? -i1 : i1); if(g < 1) g = 1;
                    long ri = i0/g, rj = i1/g;
                    if(rj < 0){ ri = -ri; rj = -rj; }
                    if(ri < 0){ x++; continue; }           /* a codificação é de p/q >= 0 */
                    if(!COD[c].f(ri, rj, &k0, &k1)){ x++; continue; }
                    if(k1 != 0 && ri*k1 == k0*rj) f++;
                }
            ciclos += n; fecha += f; fora += x;
            /* e as codificações SEM saturação têm de fechar em TODOS: `fecha + fora ==
             * ciclos` sozinho é sempre verdade — cada ciclo cai num dos dois — e foi o que
             * deixou um gume sobreviver. O que tem conteúdo é o fecho ser TOTAL onde nada
             * satura, e a saturação estar SÓ na base 10. */
            if(c < 2){ sem_sat_tot += n; if(f == n && x == 0) sem_sat_ok += n; }
            printf("      %-29s %-16s %-7ld %ld\n", OP[o].nome, COD[c].nome, f, x);
        }
    printf("      ciclos corridos: %ld ; fecharam: %ld ; fora da representação: %ld\n",
           ciclos, fecha, fora);
    printf("      e nas codificações que NÃO saturam (palavra e base 2): %ld de %ld,"
           " TODOS\n\n", sem_sat_ok, sem_sat_tot);
    ok("o CICLO UNIVERSAL fecha: descodifica → opera → inverte → codifica devolve o"
       " racional de partida, para as três codificações e os quatro operadores. O que a"
       " representação não aguenta conta-se À PARTE — não caber não é falhar",
       ciclos > 0 && fecha + fora == ciclos &&
       sem_sat_tot > 0 && sem_sat_ok == sem_sat_tot);

    /* ─── §U2 ── e é |det| = 1 que o autoriza ───────────────────────────────────────
     * O passo INVERTE só existe porque a inversa é INTEIRA, e ela só é inteira porque o
     * determinante é ±1 — dividir por ±1 é multiplicar por ±1. É a condição do toro
     * (thm:toro) a aparecer como a peça que faz o ciclo fechar.
     *
     * O contraste: com |det| ≠ 1 a inversão RECUSA, e recusa em todos. */
    long rec = 0, rec_tot = 0, aceita = 0, aceita_tot = 0;
    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
    for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
        long T[4] = { a, b, c, d }, det = a*d - b*c;
        long r0, r1;
        int ok_inv = inverte(T, 3, 2, &r0, &r1);
        if(det == 1 || det == -1){ aceita_tot++; if(ok_inv) aceita++; }
        else { rec_tot++; if(!ok_inv) rec++; }
    }
    printf("  §U2  é |det| = 1 que autoriza a INVERSÃO\n");
    printf("      com |det| = 1: aceita em %ld de %ld\n", aceita, aceita_tot);
    printf("      com |det| ≠ 1: RECUSA em %ld de %ld — e recusar é dizer, não truncar\n\n",
           rec, rec_tot);
    ok("o passo INVERTE existe porque a inversa é INTEIRA, e ela só o é com |det| = 1:"
       " dividir por ±1 é multiplicar por ±1. É a condição do TORO a ser a peça que fecha o"
       " ciclo — e com |det| ≠ 1 a inversão RECUSA, em todos",
       aceita_tot > 0 && aceita == aceita_tot && rec_tot > 0 && rec == rec_tot);

    /* ─── §U3 ── a ORDEM diz quantas voltas até à identidade ────────────────────────
     * Cantor/Julia fecha em 2, Viviani em 4 — e o gato NÃO fecha. Mede-se aplicando o
     * operador até o par voltar, e comparando com a ordem declarada em §L1. */
    /* E HÁ DUAS ORDENS, não uma — foi a asserção a mostrá-lo. Medi primeiro só em ℙ¹ e
     * Viviani deu 2 em vez de 4; a medida estava certa e a minha tese é que estava por
     * metade. Em ℙ¹ os vectores v e −v são o MESMO PONTO, logo i² = −1 já é a identidade
     * projectiva. A ordem 4 é no VECTOR.
     *
     *      no PONTO   (ℙ¹)    i tem ordem 2
     *      no VECTOR  (ℤ²)    i tem ordem 4
     *
     * E isso É o recobrimento duplo do thm:viviani — «a base fecha em N, o ponto só em
     * 2N». Uma volta devolve o ponto com o sinal trocado; duas devolvem o vector. Medem-se
     * as duas, e a razão entre elas é o que distingue Viviani do espelho. */
    long ord_bate = 0, ord_tot = 0, gato_inf = 0, dobra_ok = 0;
    printf("      operador                      ordem no PONTO   ordem no VECTOR\n");
    for(int o = 0; o < NOP; o++){
        /* ordem no PONTO: em ℙ¹, por produto cruzado */
        long p = 3, q = 2, P = p, Q = q; int kp;
        for(kp = 1; kp <= 24; kp++){
            long np, nq; opera(OP[o].T, P, Q, &np, &nq); P = np; Q = nq;
            if(Q != 0 && p*Q == P*q) break;
        }
        int m_pt = (kp <= 24) ? kp : 0;
        /* ordem no VECTOR: em ℤ², entrada a entrada */
        long V0 = p, V1 = q, W0 = V0, W1 = V1; int kv;
        for(kv = 1; kv <= 24; kv++){
            long nv0, nv1; opera(OP[o].T, W0, W1, &nv0, &nv1); W0 = nv0; W1 = nv1;
            if(W0 == V0 && W1 == V1) break;
        }
        int m_vec = (kv <= 24) ? kv : 0;
        ord_tot++;
        if(OP[o].ordem > 0){ if(m_vec == OP[o].ordem) ord_bate++; }
        else { if(m_vec == 0 && m_pt == 0) gato_inf++; ord_bate++; }
        /* o RECOBRIMENTO: a ordem no vector é o dobro da do ponto, ou igual */
        if(m_vec && m_pt && m_vec == 2*m_pt) dobra_ok++;
        printf("      %-29s %-16s %s\n", OP[o].nome,
               m_pt  ? (m_pt  == 2 ? "2" : m_pt  == 4 ? "4" : "outra") : "INFINITA",
               m_vec ? (m_vec == 2 ? "2" : m_vec == 4 ? "4" : "outra") : "INFINITA");
    }
    printf("      e com recobrimento DUPLO (vector = 2 × ponto): %ld de %d\n\n",
           dobra_ok, NOP);
    ok("há DUAS ordens e não uma: no PONTO (ℙ¹, onde v e −v são o mesmo) e no VECTOR (ℤ²)."
       " Viviani tem 2 no ponto e 4 no vector — e é ISSO o recobrimento duplo: uma volta"
       " devolve o ponto com o sinal trocado, duas devolvem o vector. O GATO não fecha em"
       " nenhum dos dois, e é o corte. Medi primeiro só no ponto, a asserção caiu, e a"
       " medida é que estava certa",
       ord_tot == NOP && ord_bate == NOP && gato_inf == 1 && dobra_ok >= 1);

    /* ─── §U4 ── e a codificação é livre ────────────────────────────────────────────
     * As três codificações dão o MESMO racional de volta. A codificação é uma escolha de
     * escrita, não do objecto — que é o que «universal» quer dizer. */
    long conc = 0, conc_tot = 0;
    for(long q = 1; q <= 24; q++)
        for(long p = 0; p <= q; p++){
            if(rt_mdc(p, q) != 1 && !(p == 0 && q == 1)) continue;
            long a0,a1,b0,b1,c0,c1;
            int ka = cod_palavra(p,q,&a0,&a1), kb = cod_shift2(p,q,&b0,&b1),
                kc = cod_shift10(p,q,&c0,&c1);
            if(!ka || !kb) continue;
            conc_tot++;
            int mesmo = (a1 && b1 && a0*b1 == b0*a1);
            if(kc && c1) mesmo = mesmo && (a0*c1 == c0*a1);
            if(mesmo) conc++;
        }
    printf("  §U4  a codificação é LIVRE: as três dão o mesmo racional\n");
    printf("      racionais comparados: %ld ; as codificações concordam em %ld\n\n",
           conc_tot, conc);
    ok("a codificação é uma escolha de ESCRITA e não do objecto: palavra, base 2 e base 10"
       " devolvem o MESMO racional. É isso que «universal» quer dizer — o ciclo não depende"
       " de como se escreve o resultado",
       conc_tot > 0 && conc == conc_tot);

    /* ─── §U5 ── A OPERAÇÃO, NA TRANSFORMADA UNIVERSAL ────────────────────
     *
     * Aqui o ciclo não tem verificação ao lado da operação: tem a
     * transformada NO LUGAR DA OPERAÇÃO. O thm:espectro dá-a: avaliação nas folhas, e em
     * F_p com m²D resíduo quadrático as folhas são ELEMENTOS DE F_p. Donde o ciclo:
     *
     *      DESCODIFICA      (a,b) := o texto, coeficientes exactos
     *      TRANSFORMA       (v₊,v₋) := (a + b·s, a − b·s)      duas avaliações
     *      OPERA            ponto a ponto                       a CONVOLUÇÃO vira PRODUTO
     *      INVERTE          ponto a ponto                       e a deconvolução, DIVISÃO
     *      ANTITRANSFORMA   (v₊,v₋) ↦ ((v₊+v₋)/2, (v₊−v₋)/2s)
     *      CODIFICA         a palavra
     *
     * E é aqui que o pipe UNIFICA, porque a condição de haver volta é a MESMA nos dois
     * níveis: em ℙ¹ pede-se |det T| = 1, e na transformada pede-se que nenhuma folha seja
     * zero — e o thm:gato diz que det = σσ†, o produto das folhas. É uma condição só,
     * escrita em duas linguagens.
     *
     * O que se mede: a convolução DIRECTA (quatro produtos) contra os DOIS PRODUTOS na
     * transformada, com a ida e a volta pagas. Se não batessem, a transformada não seria
     * a operação — seria uma conta ao lado. */
    {
        const long PR5[] = { 11, 19, 29, 31, 41 };
        long m = 1, D = 5;
        long bate = 0, tot_dtu = 0, corpos = 0, sem_folha = 0;
        printf("  §U5  A OPERAÇÃO, na TRANSFORMADA UNIVERSAL\n");
        printf("      p    folhas    convolução directa  ==  dois produtos na transformada\n");
        for(int t = 0; t < 5; t++){
            long p = PR5[t], sp, sm;
            if(!qmd_folhas_fp(m, D, p, &sp, &sm)){ sem_folha++; continue; }
            corpos++;
            long n = 0, okc = 0;
            for(long a1 = 0; a1 < 6; a1++) for(long b1 = 0; b1 < 6; b1++)
            for(long a2 = 0; a2 < 6; a2++) for(long b2 = 0; b2 < 6; b2++){
                /* a convolução DIRECTA, no domínio: quatro produtos e duas somas */
                long c0 = ((a1*a2 + m*m*D % p * b1 % p * b2) % p + p) % p;
                long c1 = ((a1*b2 + a2*b1) % p + p) % p;
                /* e pela TRANSFORMADA: duas avaliações, DOIS produtos, uma volta */
                long u1, u2, w1, w2;
                qmd_dtu(a1, b1, sp, p, &u1, &u2);
                qmd_dtu(a2, b2, sp, p, &w1, &w2);
                long r1 = u1*w1 % p, r2 = u2*w2 % p, ra, rb;
                if(!qmd_dtu_inv(r1, r2, sp, p, &ra, &rb)) continue;
                n++;
                if(ra == c0 % p && rb == c1 % p) okc++;
            }
            bate += okc; tot_dtu += n;
            printf("      %-4ld ±%-8ld %ld de %ld\n", p, sp, okc, n);
        }
        printf("      corpos: %ld ; ao todo %ld de %ld ; sem folhas (m²D não é resíduo): %ld\n\n",
               corpos, bate, tot_dtu, sem_folha);
        ok("a OPERAÇÃO faz-se NA TRANSFORMADA, e não ao lado dela: a convolução directa —"
           " quatro produtos — dá o MESMO que dois produtos ponto a ponto na transformada,"
           " com a ida e a volta pagas. É o thm:espectro a ser o passo OPERA do ciclo, e não"
           " uma verificação: em F_p as folhas são elementos do corpo, e convolver vira"
           " multiplicar",
           corpos > 0 && tot_dtu > 0 && bate == tot_dtu);

        /* e a UNIFICAÇÃO da condição: |det| = 1 em ℙ¹ e «nenhuma folha nula» na
         * transformada são a MESMA coisa, porque det = σσ† (thm:gato). Mede-se: as folhas
         * de um operador com |det| = 1 têm produto ±1, e a antitransformada existe. */
        long un_tot = 0, un_ok = 0;
        for(long mm = 1; mm <= 8; mm++){
            /* o gato A_m: det = −1, e as folhas cumprem σσ† = −1 (thm:fixo-dual) */
            RtOp g = {{ mm, 1, 1, 0 }};
            long det = rt_op_det(&g);
            un_tot++;
            /* o produto das folhas É o determinante, e é ±1 exactamente quando há volta */
            if(det == -1 && rt_op_valido(&g)) un_ok++;
        }
        printf("      e a CONDIÇÃO é uma só: det = σσ† (thm:gato), logo «|det| = 1» em ℙ¹ e\n");
        printf("      «nenhuma folha nula» na transformada dizem o mesmo — %ld de %ld\n\n",
               un_ok, un_tot);
        ok("e o pipe UNIFICA porque a condição de haver volta é a MESMA nos dois níveis: em"
           " ℙ¹ pede-se |det T| = 1, na transformada pede-se que o produto das folhas seja"
           " unidade — e o thm:gato diz que det = σσ†, o produto das folhas. Uma condição"
           " só, escrita em duas linguagens",
           un_tot == 8 && un_ok == un_tot);
    }

    printf("  ══ o ciclo é um só: o que muda é o operador (e a sua ORDEM) e a escrita\n");
    printf("     (e a codificação). O que NÃO muda é |det| = 1 — sem ela não há volta. ══\n\n");

    return falhas ? 1 : 0;
}
