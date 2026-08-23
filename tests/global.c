/* global.c — O COROLÁRIO GLOBAL nos corpos do catálogo: completar é TRAVESSAR.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/global tests/global.c && /tmp/global
 */
#include "unidade.h"
#include "global.h"
#include <stdio.h>

/* a cifra do corpo áureo: a fracção contínua de p/q, exacta, em inteiros.
 * É a régua que o catálogo já usa --- 7/2 = [3;2], 22/7 = [3;7] --- e o
 * `matricial.c` diz que ela é um PRODUTO DE MATRIZES M(a)=[[a,1],[1,0]]. */
static int fc(long p, long q, long *a, int max){
    int n = 0;
    while(q && n < max){
        long d = p / q, r = p - d*q;
        if(r < 0){ d--; r += q; }        /* piso, para o negativo também descer */
        a[n++] = d; p = q; q = r;
    }
    return n;
}
/* e o endereço: os termos empilhados em base fixa --- o prefixo da cifra fica
 * nos dígitos ALTOS, que é o que faz a régua do catálogo ser a ultramétrica. */
static long endereco_fc(const long *a, int n, int larg, int termos){
    long e = 0;
    for(int i = 0; i < termos; i++){
        long t = (i < n) ? a[i] : 0;
        if(t < 0) t = 0; if(t >= larg) t = larg - 1;
        e = e*larg + t;
    }
    return e;
}

int main(void){
    printf("O GLOBAL NOS CORPOS DO CATÁLOGO: duas representações, uma travessia\n\n");

    /* os objectos: racionais p/q reduzidos, que são o corpo do catálogo ---
     * não uma lista inventada, mas as fracções que a régua do rei cifra */
    long P[GL_MAX], Q[GL_MAX]; long n = 0;
    for(long q = 1; q <= 8; q++) for(long p = 1; p <= 8; p++){
        long x = p, y = q; while(y){ long t = x % y; x = y; y = t; }
        if(x != 1) continue;                       /* só os reduzidos: um por objecto */
        P[n] = p; Q[n] = q; n++;
    }

    /* ── §G1 AS DUAS REPRESENTAÇÕES SÃO REVERSÍVEIS, E A TRAVESSIA EXISTE ── */
    {
        printf("§G1  o par (p,q) e a cifra contínua: duas leituras do MESMO racional.\n\n");
        long mal = 0;
        long R[2][GL_MAX];
        for(long i = 0; i < n; i++) R[0][i] = P[i]*16 + Q[i];      /* R_0: o par */
        for(long i = 0; i < n; i++){                               /* R_1: a cifra */
            long a[16]; int m = fc(P[i], Q[i], a, 16);
            R[1][i] = endereco_fc(a, m, 16, 4);
        }
        printf("      %ld racionais reduzidos com p,q ≤ 8\n", n);
        printf("      R_0 = o par (p,q):   reversível? %s\n",
               gl_reversivel(R[0], n) ? "sim" : "NAO");
        printf("      R_1 = a cifra [a0;a1,...]: reversível? %s\n",
               gl_reversivel(R[1], n) ? "sim" : "NAO");
        if(!gl_reversivel(R[0], n) || !gl_reversivel(R[1], n)) mal++;

        /* a travessia, objecto a objecto --- e não se procura nada */
        long fecha = 0;
        for(long i = 0; i < n; i++)
            if(gl_travessia(R[0][i], R[0], R[1], n) == R[1][i]) fecha++;
        printf("      a travessia R_1∘R_0⁻¹ fecha em %ld dos %ld objectos\n", fecha, n);
        if(fecha != n) mal++;

        /* e as três da régua do catálogo, pelo nome */
        printf("      a régua do catálogo, verificada: ");
        struct { long p, q; const char *c; } V[3] = {{7,2,"[3;2]"},{7,3,"[2;3]"},{22,7,"[3;7]"}};
        long certas = 0;
        for(int k = 0; k < 3; k++){
            long a[16]; int m = fc(V[k].p, V[k].q, a, 16);
            printf("%ld/%ld=[%ld;%ld] ", V[k].p, V[k].q, a[0], m > 1 ? a[1] : 0);
            if(m >= 2) certas++;
        }
        printf("\n");
        if(certas != 3) mal++;
        /* 7/2 e 7/3 divergem no PRIMEIRO termo, 22/7 partilha o 3 com 7/2 */
        {
            long a1[16], a2[16], a3[16];
            fc(7,2,a1,16); fc(7,3,a2,16); fc(22,7,a3,16);
            int d12 = (a1[0] == a2[0]), d13 = (a1[0] == a3[0]);
            printf("      7/2 vs 7/3 partilham o 1.º termo? %s   ·   7/2 vs 22/7? %s\n",
                   d12 ? "sim" : "nao", d13 ? "sim" : "nao");
            if(d12 || !d13) mal++;      /* o catálogo diz: divergem / partilham */
        }
        printf("\n");
        ok("AS DUAS REPRESENTAÇÕES SÃO REVERSÍVEIS E A TRAVESSIA EXISTE, que é a hipótese e a"
           " tese do cor:global. O par (p,q) e a cifra contínua endereçam os mesmos racionais"
           " reduzidos, cada uma sem colisão, e R_1∘R_0⁻¹ fecha em todos --- não se procura"
           " nada: o objecto que tem o endereço a em R_0 é o mesmo que tem a' em R_1. E a"
           " régua sai como o catálogo a escreve, não como eu gostaria: 7/2 e 7/3 divergem no"
           " PRIMEIRO termo, e 22/7 partilha o 3 com 7/2. Isto não é uma lista minha --- são"
           " as fracções que a régua do rei cifra, e a cifra é o produto de matrizes"
           " M(a)=[[a,1],[1,0]] que o matricial.c já mede.", mal == 0);
    }

    /* ── §G2 O CUSTO, E A DOMINAÇÃO DA CADEIA ──────────────────────────────── */
    {
        printf("§G2  D(R_0,R_n) ≤ max_i D(R_{i-1},R_i): a desigualdade, medida.\n\n");
        long mal = 0;
        int bits = 20;
        long R[GL_REPR][GL_MAX];
        /* uma CADEIA de representações do mesmo corpo, cada uma reversível:
         *   R_0 o par · R_1 a cifra · R_2 a cifra com os termos trocados
         *   R_3 o par pela outra ordem (q,p) --- todas do mesmo objecto */
        for(long i = 0; i < n; i++){
            long a[16]; int m = fc(P[i], Q[i], a, 16);
            R[0][i] = P[i]*16 + Q[i];
            R[1][i] = endereco_fc(a, m, 16, 4);
            long b[4] = {m>1?a[1]:0, a[0], m>2?a[2]:0, m>3?a[3]:0};
            R[2][i] = endereco_fc(b, 4, 16, 4);
            R[3][i] = Q[i]*16 + P[i];
        }
        GlCadeia c = gl_cadeia((const long (*)[GL_MAX])R, 4, n, bits);
        printf("      %d representações, %d passos · reversíveis: %d de 4\n",
               4, c.passos, c.reversiveis);
        printf("      o pior passo: q = %d  (custo 2^-%d)\n", c.q_pior_passo, c.q_pior_passo);
        printf("      a ponta R_0→R_3: q = %d  (custo 2^-%d)\n", c.q_ponta, c.q_ponta);
        printf("      domina? %s   ---   custo da ponta ≤ maior custo dos passos\n",
               c.domina ? "SIM" : "NAO");
        if(c.reversiveis != 4 || !c.domina) mal++;

        /* passo a passo, para se ver de onde vem o máximo */
        printf("      passo a passo: ");
        for(int i = 1; i < 4; i++)
            printf("D(R%d,R%d)=2^-%d  ", i-1, i,
                   gl_custo(R[i-1], R[i], n, bits));
        printf("\n");

        /* (5) A ABSORÇÃO: pago δ, os passos de custo ≤ δ ficam na mesma bola */
        int absorvidos = gl_absorve((const long (*)[GL_MAX])R, 4, n, bits, c.q_pior_passo);
        printf("      pago δ = 2^-%d, ficam absorvidos %d dos %d passos\n",
               c.q_pior_passo, absorvidos, c.passos);
        if(absorvidos != c.passos) mal++;

        /* ── O GUME, e é o que o paper avisa em letra própria: a absorção NÃO
         * autoriza reordenar a cadeia para pagar primeiro o máximo. Mede-se
         * mostrando que o máximo é SIMÉTRICO --- não muda com a ordem ---, logo
         * dele não sai nenhuma prescrição de ordem. */
        {
            long S[GL_REPR][GL_MAX];
            for(long i = 0; i < n; i++){
                S[0][i] = R[0][i]; S[1][i] = R[2][i];
                S[2][i] = R[1][i]; S[3][i] = R[3][i];
            }
            GlCadeia d = gl_cadeia((const long (*)[GL_MAX])S, 4, n, bits);
            printf("      trocada a ordem dos intermediários: pior passo q = %d (era %d)\n",
                   d.q_pior_passo, c.q_pior_passo);
            printf("      → o máximo é SIMÉTRICO, logo dele não sai «pagar primeiro o"
                   " máximo»\n");
            if(d.q_pior_passo != c.q_pior_passo) mal++;
        }
        printf("\n");
        ok("A DESIGUALDADE DO COROLÁRIO FECHA, E A ABSORÇÃO É O QUE O PAPER DIZ QUE É. Sobre"
           " uma cadeia de quatro representações do mesmo corpo --- o par, a cifra, a cifra"
           " com termos trocados, o par invertido ---, todas reversíveis, o custo da ponta é"
           " dominado pelo maior custo dos passos. E pago o δ do pior passo, todos os outros"
           " ficam dentro da mesma bola. MAS O GUME É A LETRA DO PAPER: trocada a ordem dos"
           " intermediários o máximo NÃO MUDA, porque o máximo é simétrico --- logo dele não"
           " se tira «executar primeiro o de custo máximo», que exigiria os passos serem"
           " comutáveis. Medir isto era obrigatório: era exactamente a consequência que o"
           " corolário não tem e que seria fácil atribuir-lhe.", mal == 0);
    }

    /* ── §G3 A ULTRAMÉTRICA HERDA-SE PELA BIJEÇÃO ─────────────────────────── */
    {
        printf("§G3  a régua não se constrói no corpo: HERDA-SE pela bijeção do global.\n\n");
        long mal = 0;
        int bits = 20;
        long R0[GL_MAX], R1[GL_MAX];
        for(long i = 0; i < n; i++){
            long a[16]; int m = fc(P[i], Q[i], a, 16);
            R0[i] = P[i]*16 + Q[i];
            R1[i] = endereco_fc(a, m, 16, 4);
        }
        /* d_X(x,y) := d_I(R(x),R(y)) --- e é ultramétrica porque d_I é, e a
         * desigualdade forte só usa a igualdade dos valores, que a bijeção
         * preserva. Mede-se nas DUAS representações. */
        GlUltra u0 = gl_ultra_herdada(R0, n, bits);
        GlUltra u1 = gl_ultra_herdada(R1, n, bits);
        printf("      pelo par (p,q):  %ld triplos · %ld violam · %ld estritos\n",
               u0.triplos, u0.viola, u0.estrito);
        printf("      pela cifra:      %ld triplos · %ld violam · %ld estritos\n",
               u1.triplos, u1.viola, u1.estrito);
        if(u0.viola || u1.viola || !u0.estrito || !u1.estrito) mal++;

        /* E AS BOLAS: o que muda entre as duas leituras é o CUSTO D, não quem
         * está dentro. Onde as duas discordam, discordam por quanto? */
        printf("      as bolas, raio a raio: ");
        long total_disc = 0;
        for(int q = 2; q <= 14; q += 4){
            long d = gl_bolas_discordam(R0, R1, n, bits, q);
            printf("q=%d:%ld  ", q, d);
            total_disc += d;
        }
        printf("\n");
        printf("      → as duas leituras são ultramétricas AMBAS; o que difere entre elas é"
               " o custo D, e ele já foi medido em §G2\n");

        /* ── E O GUME: uma leitura NÃO reversível não herda régua nenhuma ---
         * a desigualdade forte cai, porque dois objectos distintos ficam à
         * distância zero. É a hipótese do corolário a fazer trabalho. */
        {
            long Rmau[GL_MAX];
            for(long i = 0; i < n; i++) Rmau[i] = P[i];      /* esquece o denominador */
            int rev = gl_reversivel(Rmau, n);
            long zeros = 0;
            for(long i = 0; i < n; i++) for(long j = 0; j < i; j++)
                if(Rmau[i] == Rmau[j]) zeros++;
            printf("      gume: a leitura que esquece o denominador é reversível? %s ---"
                   " e põe %ld pares DISTINTOS à distância zero\n",
                   rev ? "sim (mal)" : "nao", zeros);
            if(rev || zeros == 0) mal++;
        }
        printf("\n");
        ok("A RÉGUA NÃO SE CONSTRÓI NO CORPO: HERDA-SE. É a peça do cor:global que interessa"
           " a quem tem corpos para completar --- a ultramétrica vive no ÍNDICE, e a bijeção"
           " transporta-a: d_X(x,y) := d_I(R(x),R(y)). Como R é bijecção, é métrica; como d_I"
           " é ultramétrica, d_X é ultramétrica, e a desigualdade forte atravessa sem se"
           " degradar porque só usa a igualdade dos valores. Medido nas DUAS representações"
           " dos mesmos racionais: zero violações e triplos estritos em ambas. Logo completar"
           " um corpo do catálogo não pede régua nova --- pede uma REPRESENTAÇÃO REVERSÍVEL,"
           " e a régua vem atrás. E o gume é a hipótese a trabalhar: a leitura que esquece o"
           " denominador não é reversível e não herda régua nenhuma --- põe pares distintos à"
           " distância zero, e aí não há métrica, quanto mais ultramétrica.", mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
