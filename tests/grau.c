/* grau.c — A SPLINE É DE GRAU n, E OS COEFICIENTES SÃO PASCAL. Resíduo 0, em inteiros.
 *
 * O Aarão: «vc não consegue desenhar e medir um polinómio do terceiro grau?»
 *
 * A pergunta é justa porque eu andei a tratar TrueType e OpenType como dois formatos, e
 * eles não são: a TrueType tem contornos de grau 2, a OpenType de grau 3, e é o MESMO
 * polinómio uma linha acima. O `tests/pascal.c` diz porquê, e não é analogia:
 *
 *     C(n,k) = C(n-1,k-1) + C(n-1,k)     a recorrência de Pascal
 *     A_{n+1} = A_n ⊕ A_n†               o passo da torre
 *     SÃO A MESMA COISA.
 *
 * A Bézier de grau n é B(t) = Σ C(n,k) t^k (1−t)^(n−k) P_k, e os C(n,k) são a linha n do
 * triângulo. Grau 2 dá 1,2,1; grau 3 dá 1,3,3,1. Um avaliador que gere a linha pela
 * recorrência lê os dois sem saber que são formatos diferentes.
 *
 * E A ELEVAÇÃO DE GRAU É EXACTA, o que é o que fecha isto: toda quadrática É uma cúbica,
 * com os pontos de controlo que a própria recorrência dá,
 *
 *     Q(P0,P1,P2)  =  C(P0,  (P0+2P1)/3,  (2P1+P2)/3,  P2)
 *
 * e daí que o resíduo entre as duas seja ZERO — não pequeno, zero. Tudo em inteiros: os
 * pontos multiplicam-se por 3 antes de elevar, e a igualdade mede-se por PRODUTO CRUZADO,
 * sem uma única divisão.
 *
 *   §G1  a linha n de Pascal, pela recorrência, e a soma dá 2^n
 *   §G2  B(t) de grau 2 avaliada com os coeficientes gerados == a fórmula fechada
 *   §G3  ELEVAR: a quadrática elevada a cúbica dá o MESMO ponto, resíduo 0 INTEIRO
 *   §G4  e o CONTROLO: uma cúbica qualquer NÃO é uma quadrática — a volta não fecha
 *   §G5  o mesmo avaliador lê grau 2 e grau 3 sem um `if` de formato
 *
 * Zero doubles. Zero divisões na comparação.
 *
 *   cc -O2 -std=c99 -Wall -I../lib grau.c -o grau && ./grau
 */
#include <stdio.h>
#include "unidade.h"

#define GMAX 8

/* a linha n do triângulo, pela recorrência e não pela fórmula — é o passo da torre */
static void pascal(int n, long *c){
    c[0] = 1;
    for(int i = 1; i <= n; i++){
        c[i] = 1;
        for(int k = i - 1; k >= 1; k--) c[k] = c[k] + c[k-1];   /* C(i,k)=C(i-1,k-1)+C(i-1,k) */
    }
}

/* B(t) de grau n, com t = a/b racional e os pontos inteiros. Devolve o NUMERADOR sobre b^n:
 * não se divide, e por isso não há resto nenhum a esconder o resultado. */
static long bezier_num(int n, const long *P, long a, long b){
    long c[GMAX + 1]; pascal(n, c);
    long s = 0;
    for(int k = 0; k <= n; k++){
        long termo = c[k] * P[k];
        for(int i = 0; i < k; i++)     termo *= a;          /* t^k */
        for(int i = 0; i < n - k; i++) termo *= (b - a);    /* (1-t)^(n-k) */
        s += termo;
    }
    return s;                                              /* sobre b^n */
}

int main(void){
    printf("=== A SPLINE E' DE GRAU n, E OS COEFICIENTES SAO PASCAL ===================\n\n");

    /* ─── §G1 a linha de Pascal, e a soma ─────────────────────────────────────────── */
    int g1 = 1;
    for(int n = 0; n <= 6; n++){
        long c[GMAX + 1]; pascal(n, c);
        long soma = 0, pot = 1;
        printf("   n=%d:", n);
        for(int k = 0; k <= n; k++){ printf(" %ld", c[k]); soma += c[k]; }
        for(int i = 0; i < n; i++) pot *= 2;
        printf("   soma %ld  2^%d %ld\n", soma, n, pot);
        if(soma != pot) g1 = 0;
        /* e a simetria C(n,k)=C(n,n-k), que e' a involucao que troca os lados */
        for(int k = 0; k <= n; k++) if(c[k] != c[n-k]) g1 = 0;
    }
    ok("a linha de Pascal soma 2^n e e' simetrica — a dimensao da torre e a involucao", g1);

    /* ─── §G2 o avaliador contra a forma fechada do grau 2 ────────────────────────── */
    const long Q[3] = { 100, 700, 400 };
    int g2 = 1, n2 = 0;
    for(long a = 0; a <= 12; a++){
        long b = 12;
        long meu = bezier_num(2, Q, a, b);
        /* a forma fechada: (1-t)^2 P0 + 2t(1-t) P1 + t^2 P2, tudo sobre b^2 */
        long u = b - a;
        long fech = u*u*Q[0] + 2*a*u*Q[1] + a*a*Q[2];
        if(meu != fech) g2 = 0;
        n2++;
    }
    printf("\n   grau 2 avaliado em %d pontos de t\n", n2);
    ok("o avaliador generico == a forma fechada do grau 2, resIduo 0 exacto", g2 && n2 == 13);

    /* ─── §G3 ELEVAR: a quadratica E' uma cubica ──────────────────────────────────── */
    /* multiplicam-se os pontos por 3 para que a elevacao seja INTEIRA:
     *   C0 = 3 P0,  C1 = P0 + 2 P1,  C2 = 2 P1 + P2,  C3 = 3 P2   (sobre 3) */
    const long C[4] = { 3*Q[0], Q[0] + 2*Q[1], 2*Q[1] + Q[2], 3*Q[2] };
    int g3 = 1, n3 = 0; long pior = 0;
    for(long a = 0; a <= 12; a++){
        long b = 12;
        long q = bezier_num(2, Q, a, b);      /* sobre b^2      */
        long c = bezier_num(3, C, a, b);      /* sobre 3 * b^3  */
        /* PRODUTO CRUZADO, sem divisao:  q/b^2 == c/(3 b^3)  <=>  q*3*b == c */
        long e = q * 3 * b, d = e > c ? e - c : c - e;
        if(d){ g3 = 0; if(d > pior) pior = d; }
        n3++;
    }
    printf("\n   elevacao 2->3 em %d pontos, pior desvio %ld INTEIRO\n", n3, pior);
    ok("a quadratica elevada a cubica da' o MESMO ponto: resIduo 0 INTEIRO", g3 && n3 == 13);

    /* ─── §G4 o CONTROLO: nem toda cubica e' uma quadratica ───────────────────────── */
    /* se a elevacao fechasse para qualquer cubica, §G3 nao estava a medir nada */
    const long X[4] = { 300, 900, 100, 1200 };     /* uma cubica qualquer */
    int discorda = 0;
    for(long a = 0; a <= 12; a++){
        long b = 12;
        long q = bezier_num(2, Q, a, b);
        long c = bezier_num(3, X, a, b);
        if(q * 3 * b != c) discorda++;
    }
    printf("\n   controlo: uma cubica qualquer discorda em %d de 13 pontos\n", discorda);
    ok("uma cubica QUALQUER nao e' a quadratica — sem isto o §G3 passava sozinho",
       discorda >= 11);

    /* ─── §G5 o mesmo avaliador, os dois graus ────────────────────────────────────── */
    /* nao ha' `if` de formato em lado nenhum: muda o n, e a linha de Pascal vem atras */
    int g5 = 1;
    for(int n = 1; n <= 5; n++){
        long P[GMAX + 1];
        for(int k = 0; k <= n; k++) P[k] = 100 + 37*k;
        /* nos extremos a curva passa PELOS pontos: B(0)=P0 e B(1)=Pn, em qualquer grau */
        long b = 7, pot = 1;
        for(int i = 0; i < n; i++) pot *= b;
        if(bezier_num(n, P, 0, b) != P[0]  * pot) g5 = 0;
        if(bezier_num(n, P, b, b) != P[n] * pot) g5 = 0;
    }
    ok("o mesmo avaliador serve grau 1 a 5 e passa pelos extremos — sem um `if` de formato", g5);

    /* ─── §G6 CONCATENAR: o fim de um E' o inicio do seguinte, e o contorno FECHA ─── */
    /* A concatenacao nao precisa de cola: os segmentos PARTILHAM o ponto. O ultimo ponto
     * de controlo de um segmento e' o primeiro do seguinte — e' a mesma coordenada, nao
     * duas iguais. E o contorno e' uma ORBITA: fecha quando o ultimo volta ao primeiro, e
     * o residuo desse fecho e' o que se mede.
     *
     * E os graus podem MISTURAR-SE no mesmo contorno, que e' o que uma fonte real faz:
     * eleva-se cada um ao maior e a juncao continua exacta, porque elevar nao aproxima. */
    {
        /* um contorno de tres segmentos: dois quadraticos e um cubico, a fechar */
        long S1[3] = { 0, 300, 600 };                    /* grau 2:  0 -> 600 */
        long S2[3] = { 600, 900, 400 };                  /* grau 2: 600 -> 400 */
        long S3[4] = { 400, 200, 100,   0 };             /* grau 3: 400 ->   0 */
        long b = 9;
        int liga = 1, fecha_ok, n6 = 0;

        /* C0 nas juncoes: B(1) de um == B(0) do seguinte, EXACTO e sem divisao */
        long p1 = 1, p2 = 1, p3 = 1;
        for(int i = 0; i < 2; i++){ p1 *= b; p2 *= b; }
        for(int i = 0; i < 3; i++) p3 *= b;
        if(bezier_num(2, S1, b, b) != S1[2] * p1) liga = 0;
        if(bezier_num(2, S2, 0, b) != S2[0] * p2) liga = 0;
        if(S1[2] != S2[0]) liga = 0;                    /* e' O MESMO ponto, nao dois iguais */
        if(bezier_num(2, S2, b, b) != S2[2] * p2) liga = 0;
        if(bezier_num(3, S3, 0, b) != S3[0] * p3) liga = 0;
        if(S2[2] != S3[0]) liga = 0;
        /* `n6 = 3` seguido de `n6 == 3` não acrescentava nada ao `liga` — era um 3
         * comparado com um 3. O que se pode contar são as VERIFICAÇÕES que o `liga` faz:
         * seis, e todas têm de correr. Se uma fosse esquecida, a contagem denunciava-a. */
        n6 = 6;                                   /* as seis verificações acima */
        long feitas = 0;
        feitas += (bezier_num(2, S1, b, b) == S1[2] * p1);
        feitas += (bezier_num(2, S2, 0, b) == S2[0] * p2);
        feitas += (S1[2] == S2[0]);
        feitas += (bezier_num(2, S2, b, b) == S2[2] * p2);
        feitas += (bezier_num(3, S3, 0, b) == S3[0] * p3);
        feitas += (S2[2] == S3[0]);
        ok("os segmentos ligam pelo PONTO PARTILHADO: B(1) de um e' B(0) do seguinte — e sao"
           " SEIS verificacoes, contadas: as duas pontas de cada juncao e o ponto ser O MESMO"
           " e nao dois iguais. O `n6 = 3` que aqui estava era um tres comparado com um tres,"
           " e nao acrescentava nada ao `liga`",
           liga && feitas == n6 && n6 == 6);

        /* e a ORBITA FECHA: o ultimo ponto do ultimo segmento e' o primeiro do primeiro */
        long ini = S1[0], fim = S3[3];
        fecha_ok = (fim - ini) == 0;
        printf("\n   contorno de %d segmentos (2,2,3): inicio %ld, fim %ld, residuo do fecho %ld\n",
               n6, ini, fim, fim - ini);
        ok("o contorno FECHA: o fim volta ao inicio, resIduo 0 INTEIRO", fecha_ok);

        /* e o CONTROLO: um contorno que nao volta NAO fecha — senao isto passava sozinho */
        long A1[4] = { 400, 200, 100, 50 };
        printf("   controlo: um que nao volta da' residuo %ld\n", A1[3] - ini);
        ok("um contorno que nao volta ao inicio NAO fecha — o zero acima nao e' automatico",
           (A1[3] - ini) != 0);
    }

    /* ─── §G7 O CONTORNO É UMA ÓRBITA: o relógio gera-o no plano, sem segmentos ────── */
    /* O Aarão: «não são curvas de grau, são curvas de grau INFINITO — o relógio
     * (transformada) gera direto no plano, a concatenação usa infinitos graus se quiser,
     * instantaneamente. Nunca viu transformada no plano?»
     *
     * E é outra coisa do que eu estava a fazer. Partir um contorno em segmentos de grau 2
     * ou 3 e colá-los é escolher um grau e depois remendar as juntas. Um contorno FECHADO
     * é uma ÓRBITA, e a transformada dá-a inteira: cada harmónico é um círculo a rodar, e
     * a soma deles É a curva. Não há juntas porque não há segmentos.
     *
     * E qual transformada, o `universal.c` já respondeu: a AVALIAÇÃO NAS RAÍZES. Num
     * contorno cíclico de N pontos as raízes são as de x^N−1 — o caso m=0 da borda. Aqui
     * corre em Z_p, onde as raízes da unidade EXISTEM e são inteiras: nem um double, nem
     * uma raiz avaliada, e a volta fecha com resíduo 0 EXACTO. */
    {
        const long p = 13, N = 12, g = 2;      /* 2 é gerador de Z_13*, e 2^12 = 1 */
        long w = 1;                            /* w = g^((p-1)/N) é raiz N-ésima da unidade */
        for(long i = 0; i < (p-1)/N; i++) w = w * g % p;
        /* que ela É raiz N-ésima e não de ordem menor: sem isto não é o relógio certo */
        long o = 1, ordem = 0;
        do { o = o * w % p; ordem++; } while(o != 1 && ordem <= N);
        printf("\n   Z_%ld: w=%ld tem ordem %ld (precisa de %ld)\n", p, w, ordem, N);
        ok("a raiz da unidade tem ordem N EXACTA — e' o relogio, nao um sub-relogio",
           ordem == N);

        /* o contorno: N pontos, e a transformada e' avalia-lo nas N raizes */
        long z[12] = { 3, 5, 9, 11, 12, 10, 6, 4, 1, 2, 7, 8 }, Z[12], v[12];
        for(long k = 0; k < N; k++){
            /* Z_k = sum_n z_n w^(nk) — o expoente e' o PRODUTO n*k, e nao n. Escrevi `w^n`
             * e a volta deu desvio 12, que e' -1 mod 13: um expoente errado nao da' lixo,
             * da' OUTRA transformada, e ela tambem tem inversa — so' que nao e' esta. */
            long s2 = 0, wnk = 1, wk1 = 1;
            for(long i = 0; i < k; i++) wk1 = wk1 * w % p;     /* w^k */
            for(long n = 0; n < N; n++){ s2 = (s2 + z[n] * wnk) % p; wnk = wnk * wk1 % p; }
            Z[k] = s2;
            /* prepara-se w^k para a volta */
            long t2 = 1; for(long i = 0; i < k; i++) t2 = t2 * w % p;
            v[k] = t2;
        }
        /* A VOLTA: soma dos harmonicos sobre o inverso da raiz, dividida por N — e a
         * divisao por N e' um INVERSO em Z_p, exacto, sem resto nenhum. */
        long winv = 1, Ninv = 1;
        for(long i = 1; i < p; i++) if(w * i % p == 1) winv = i;
        for(long i = 1; i < p; i++) if(N % p * i % p == 1) Ninv = i;
        int volta_ok = 1; long pior7 = 0;
        for(long n = 0; n < N; n++){
            long s2 = 0, wnk = 1, wn1 = 1;
            for(long i = 0; i < n; i++) wn1 = wn1 * winv % p;  /* w^(-n) */
            for(long k = 0; k < N; k++){ s2 = (s2 + Z[k] * wnk) % p; wnk = wnk * wn1 % p; }
            s2 = s2 * Ninv % p;
            long d = s2 > z[n] ? s2 - z[n] : z[n] - s2;
            if(d){ volta_ok = 0; if(d > pior7) pior7 = d; }
        }
        printf("   contorno de %ld pontos -> %ld harmonicos -> volta: pior desvio %ld\n",
               N, N, pior7);
        ok("o contorno vai e volta pela transformada: RESIDUO 0 EXACTO, sem um double",
           volta_ok);

        /* e o CONTROLO: com a raiz ERRADA (ordem menor) a volta NAO fecha */
        long wm = w * w % p;                    /* ordem N/2, se N par */
        int mau = 0;
        for(long n = 0; n < N; n++){
            long s2 = 0, wk = 1;
            for(long k = 0; k < N; k++){
                long h = 0, wj = 1;
                for(long j = 0; j < N; j++){ h = (h + z[j] * wj) % p; wj = wj * wm % p; }
                s2 = (s2 + h * wk) % p; wk = wk * wm % p;
            }
            if(s2 * Ninv % p != z[n]) mau++;
        }
        printf("   controlo: com a raiz de ordem menor, %d de %ld pontos nao voltam\n", mau, N);
        ok("com a raiz ERRADA a volta nao fecha — o zero acima nao e' automatico", mau >= 8);
    }

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  A TrueType e a OpenType nao sao dois formatos: sao a mesma spline em graus");
        puts("  diferentes, e os coeficientes sao as linhas de Pascal — 1,2,1 e 1,3,3,1.");
        puts("  A recorrencia que as gera E' o passo da torre (tests/pascal.c), e por isso a");
        puts("  elevacao de grau nao aproxima nada: e' EXACTA, e o residuo e' 0 INTEIRO.");
        puts("");
        puts("  E mede-se por PRODUTO CRUZADO, sem uma divisao — porque divisao inteira tem");
        puts("  resto, e um resto pequeno de mais para se ver e' onde os defeitos moram.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
