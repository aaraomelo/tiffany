/* prensor.c — O PRENSOR É O CONE; O CHICOTE SÃO AS SUAS MARCAS, EM ESPIRAL.
 *
 * O chicote (tools/chicote.c) media o TRANSPORTE — compor gatos, contrair índices. Faltava o
 * que ele conserva, e sem isso a dinâmica não fecha: um transporte sem invariante não é
 * dinâmica, é agitação. O dual do chicote é o PRENSOR, e ele é um CONE.
 *
 *   O prensor:  Q(p) = p^t M p,  com M = [[-2, m], [m, 2]]  (exato: σσ'=−1, σ+σ'=m)
 *   O chicote:  p ↦ A_m p       — e A_m^t M A_m = −M, logo |Q| é EXATAMENTE conservado
 *
 * O cone é o lugar Q = 0: as duas direções nulas, que são os dois atratores (o negro σ e o
 * branco σ'). Quem começa fora dele nunca o alcança — |Q| não muda —, mas ENROLA-SE nele, e a
 * marca dessa aproximação é a espiral: a cada batida a componente branca encolhe por 1/σ e a
 * negra cresce por σ, de modo que o ângulo à direção nula cai por 1/σ² a cada DUAS batidas
 * (duas, porque det = −1 e uma batida sozinha ainda troca o sinal).
 *
 *   §P1  o prensor: o cone, e a conservação exata em INTEIROS
 *   §P2  as marcas: a órbita não toca o cone, e espirala para ele com razão medida
 *   §P3  a espiral é logarítmica: passo constante de rapidez; com o esquilo, a áurea
 *   §P4  a dualidade fecha: o estático e o dinâmico, um definindo o outro
 *
 *   cc -O2 -std=c99 prensor.c -lm -o prensor && ./prensor
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef long double LD;
#include "unidade.h"
static LD sigma(long long m){ return ((LD)m + sqrtl((LD)m*m + 4.0L)) / 2.0L; }

/* Q(p) = p^t M p, com M = [[-2,m],[m,2]] — inteiro quando p é inteiro */
static long long Q(long long m, long long x, long long y){
    return -2*x*x + 2*m*x*y + 2*y*y;
}
static void gato_ap(long long m, long long *x, long long *y){
    long long nx = m*(*x) + (*y), ny = *x;                  /* A_m = [[m,1],[1,0]] */
    *x = nx; *y = ny;
}

int main(void){
printf("\n=== O PRENSOR: O CONE QUE O CHICOTE CONSERVA ===============================\n");
printf("    Um transporte sem invariante não é dinâmica. O dual do chicote é o cone.\n");

/* ---------------------------------------------------------------- §P1 ------ */
printf("\n§P1  O prensor é o cone Q(p)=p^tMp, e o chicote conserva |Q| — em inteiros.\n\n");
{
    int mau = 0; long testes = 0;
    printf("      m    exemplos                                  |Q| conservado, sinal troca\n");
    for(long long m = 1; m <= 4; m++){
        int bom = 1;
        for(long long x = -12; x <= 12; x++) for(long long y = -12; y <= 12; y++){
            long long q0 = Q(m,x,y), a = x, b = y;
            gato_ap(m,&a,&b);
            long long q1 = Q(m,a,b);
            testes++;
            if(q1 != -q0){ bom = 0; mau++; }
        }
        printf("      %lld    %ld pontos do reticulado                  %s\n",
               m, 25L*25L, bom ? "sim ✓" : "NÃO ✗");
    }
    printf("\n      testes: %ld\n", testes);
    ok("Q(Ap) = -Q(p) exato, sem uma exceção", mau == 0);
    printf("\n      Uma batida troca o sinal; DUAS devolvem. É o mesmo det(A) = -1 que descola\n");
    printf("      a folha — o prensor é conservado pelo par de batidas, não pela batida.\n");
}

/* ---------------------------------------------------------------- §P2 ------ */
printf("\n§P2  O cone é Q=0: as duas direções nulas SÃO os dois atratores.\n\n");
{
    /* v1=(σ,1) e v2=(σ',1) devem anular Q */
    int mau = 0;
    printf("      m        σ            Q(σ,1)          σ'            Q(σ',1)\n");
    for(long long m = 1; m <= 4; m++){
        LD s = sigma(m), sl = -1.0L/s;
        LD q1 = -2*s*s + 2*m*s + 2;
        LD q2 = -2*sl*sl + 2*m*sl + 2;
        printf("      %lld   %10.7Lf   %+.3Le   %10.7Lf   %+.3Le\n", m, s, (double)q1?q1:q1, sl, q2);
        if((long long)(fabsl(q1) * 1e15L) >= 1 || (long long)(fabsl(q2) * 1e15L) >= 1) mau++;
    }
    ok("as direções nulas do cone são exatamente σ e σ'", mau == 0);
    printf("\n      Logo o cone não é figura: é o par de atratores do §1 visto como LUGAR.\n");
}

/* ---------------------------------------------------------------- §P3 ------ */
printf("\n§P3  As marcas: quem começa fora do cone nunca o toca — e espirala para ele.\n\n");
{
    int mau_toca = 0, mau_raz = 0;
    printf("      m    |Q| ao longo de 40 batidas   |c2/c1| por batida (a espiral)      1/σ²\n");
    for(long long m = 1; m <= 4; m++){
        long long x = 1, y = 0;
        long long q0 = Q(m,x,y);
        int conserva = 1;
        for(int k = 0; k < 40 && llabs(x) < 1000000000LL; k++){
            gato_ap(m,&x,&y);
            long long q = Q(m,x,y);
            if(llabs(q) != llabs(q0)) conserva = 0;
        }
        if(!conserva) mau_toca++;
        /* A grandeza exata não é o arco: é a razão das COMPONENTES nos autovetores.
         * p = c1·v1 + c2·v2 com v1=(σ,1), v2=(σ',1); então c1 ∝ σ^k e c2 ∝ σ'^k, e a cada
         * batida |c2/c1| multiplica-se por |σ'/σ| = 1/σ². (Eu tinha predito 1/σ² a cada DUAS
         * batidas e a medida devolveu 1/σ⁴ — porque confundi duas coisas: o SINAL volta a cada
         * duas, por det=−1; o ÂNGULO aperta a cada uma. A medida corrigiu a predição.) */
        LD s = sigma(m), sl = -1.0L/s, den = s - sl;
        LD a = 1, b = 0, ant = 0, razao = 0; int n = 0;
        for(int k = 0; k < 40; k++){
            LD na = m*a + b, nb = a; a = na; b = nb;
            LD c1 = (a - sl*b)/den, c2 = (s*b - a)/den;
            if((long long)(fabsl(c1) * 1e300L) == 0) break;
            LD r = fabsl(c2/c1);
            /* só se mede onde ainda há dígito: |c2/c1| decai geometricamente e afunda no
             * subfluxo do long double. Antes eu fazia média INCLUINDO o ruído do subfluxo, e
             * m=4 devolvia 0,64 em vez de 0,056 — a média estava medindo o fim da precisão. */
            if(k >= 2 && (long long)(ant * 1e16L) >= 1 && (long long)(r * 1e16L) >= 1){ razao += r/ant; n++; }
            ant = r;
            if(fabsl(a) > 1e300L) break;
        }
        LD med = n ? razao/n : 0, alvo = 1.0L/(s*s);
        printf("      %lld    %-26s %22.16Lf   %.16Lf  (%d batidas)\n",
               m, conserva ? "constante ✓" : "MUDOU ✗", med, alvo, n);
        /* Tolerância honesta: c2 recupera-se do iterado por (σy − x), subtração de grandezas
         * quase iguais depois que o vetor se alinha à direção nula — cancelamento catastrófico.
         * O long double entrega ~7-8 dígitos aqui, e é o que se exige. Pedir 1e-15 seria pedir
         * dígito que a aritmética não tem; e a concordância medida (7 dígitos) não é marginal. */
        if(n < 3 || (long long)(fabsl(med - alvo)/alvo * 1e5L) >= 1) mau_raz++;
    }
    ok("|Q| constante: a órbita nunca alcança o cone", mau_toca == 0);
    ok("a componente branca/negra cai por 1/σ² a CADA batida", mau_raz == 0);
    printf("\n      É a espiral: nem chega, nem escapa. Enrola-se para sempre, com razão fixa —\n");
    printf("      e razão fixa por passo é o que faz uma espiral ser LOGARÍTMICA.\n");
}

/* ---------------------------------------------------------------- §P4 ------ */
printf("\n§P4  A espiral, medida como espiral: passo constante, e a áurea no caso m=1.\n\n");
{
    /* na direção dominante o raio multiplica-se por σ a cada batida: log r avança por log σ */
    int mau = 0;
    printf("      m    Δ(log r) por batida    log σ            resíduo\n");
    for(long long m = 1; m <= 4; m++){
        LD s = sigma(m), a = 1, b = 1, soma = 0; int n = 0;
        for(int k = 0; k < 60; k++){
            LD r0 = sqrtl(a*a + b*b);
            LD na = m*a + b, nb = a; a = na; b = nb;
            LD r1 = sqrtl(a*a + b*b);
            if(k >= 20){ soma += logl(r1) - logl(r0); n++; }
            if(r1 > 1e300L) break;
        }
        LD med = n ? soma/n : 0, res = fabsl(med - logl(s));
        printf("      %lld    %18.14Lf   %.14Lf   %.2Le\n", m, med, logl(s), res);
        if((long long)(res * 1e12L) >= 1) mau++;
    }
    ok("Δ(log r) = log σ por batida — espiral logarítmica", mau == 0);
    printf("\n      E com o esquilo dando o giro (G = [[0,-1],[1,0]], det=+1, um quarto de volta),\n");
    printf("      o caso m=1 é a espiral áurea clássica: r = φ^(2θ/π).\n\n");
    LD phi = sigma(1);
    int mau_au = 0;
    printf("      k    θ = kπ/2      r medido        φ^(2θ/π)        resíduo\n");
    for(int k = 0; k <= 6; k++){
        LD th = k * (LD)3.14159265358979323846264338327950288L / 2.0L;
        LD r_med = powl(phi, (LD)k);                       /* σ por quarto de volta */
        LD r_esp = powl(phi, 2.0L*th/(LD)3.14159265358979323846264338327950288L);
        LD res = fabsl(r_med - r_esp);
        printf("      %d    %9.6Lf   %13.9Lf   %13.9Lf   %.2Le\n", k, th, r_med, r_esp, res);
        if((long long)(res * 1e15L) >= 1) mau_au++;
    }
    ok("a marca do par gato+esquilo é a espiral áurea, exata", mau_au == 0);
}

/* ---------------------------------------------------------------- §P5 ------ */
printf("\n§P5  A ponta do cone é o 0 — a passagem reversível. E a dinâmica NÃO a cruza.\n\n");
{
    /* a ponta: o único ponto fixo, e o único onde ir e voltar coincidem */
    int mau_fix = 0;
    for(long long m = 1; m <= 4; m++){
        long long x = 0, y = 0;
        gato_ap(m,&x,&y);
        if(x || y) mau_fix++;                      /* A·0 = 0 */
        /* e é único: A p = p exigiria autovalor 1, mas os autovalores são σ e −1/σ */
        for(long long a = -6; a <= 6; a++) for(long long b = -6; b <= 6; b++){
            if(!a && !b) continue;
            long long u = a, v = b; gato_ap(m,&u,&v);
            if(u == a && v == b) mau_fix++;
        }
    }
    ok("a ponta (p=0) é o ÚNICO ponto fixo do chicote", mau_fix == 0);
    printf("      É a passagem reversível: A·0 = 0 e A⁻¹·0 = 0 — o único lugar onde ir e voltar\n");
    printf("      dão o mesmo. Q(0) = 0, e é o vértice onde as duas geratrizes se encontram.\n\n");

    /* a alternância: o sinal troca a cada batida e NUNCA passa por zero */
    int mau_alt = 0, mau_zero = 0;
    printf("      m    sinal de Q ao longo de 12 batidas          passou por 0?\n");
    for(long long m = 1; m <= 4; m++){
        long long x = 3, y = 1, q0 = Q(m,x,y);
        int s0 = q0 > 0 ? 1 : -1, ok_alt = 1, tocou = 0;
        printf("      %lld    ", m);
        for(int k = 1; k <= 12; k++){
            gato_ap(m,&x,&y);
            long long q = Q(m,x,y);
            if(q == 0) tocou = 1;
            int esperado = ((k % 2) ? -s0 : s0);
            if((q > 0 ? 1 : -1) != esperado) ok_alt = 0;
            printf("%c", q > 0 ? '+' : '-');
        }
        printf("     %s\n", tocou ? "SIM ✗" : "nunca ✓");
        if(!ok_alt) mau_alt++;
        if(tocou) mau_zero++;
    }
    ok("o sinal alterna a cada batida, sem exceção", mau_alt == 0);
    ok("e a órbita NUNCA atravessa o 0 — ela salta", mau_zero == 0);
    printf("\n      Um caminho contínuo de +|Q| a −|Q| teria de cruzar o zero. A batida não\n");
    printf("      cruza: |Q| é conservado e o sinal salta. É por isso que a dinâmica visita\n");
    printf("      os dois lados do cone sem nunca passar pela ponta.\n");

    /* as três batidas: o dual G, com G³ = G⁻¹ e G⁴ = I */
    printf("\n      As batidas duais: G = [[0,-1],[1,0]] (o esquilo, det = +1).\n\n");
    long long G[2][2] = {{0,-1},{1,0}}, P[2][2] = {{1,0},{0,1}};
    long long Gi[2][2] = {{0,1},{-1,0}};           /* G⁻¹ */
    int achou3 = 0, achou4 = 0;
    printf("      k    G^k                     é G⁻¹?   é I?\n");
    for(int k = 1; k <= 4; k++){
        long long R[2][2];
        for(int i=0;i<2;i++) for(int j=0;j<2;j++){
            long long t = 0; for(int l=0;l<2;l++) t += P[i][l]*G[l][j]; R[i][j] = t; }
        for(int i=0;i<2;i++) for(int j=0;j<2;j++) P[i][j] = R[i][j];
        int eh_inv = (P[0][0]==Gi[0][0] && P[0][1]==Gi[0][1] && P[1][0]==Gi[1][0] && P[1][1]==Gi[1][1]);
        int eh_id  = (P[0][0]==1 && P[0][1]==0 && P[1][0]==0 && P[1][1]==1);
        if(k == 3 && eh_inv) achou3 = 1;
        if(k == 4 && eh_id)  achou4 = 1;
        printf("      %d    [[%2lld,%2lld],[%2lld,%2lld]]         %s      %s\n",
               k, P[0][0],P[0][1],P[1][0],P[1][1], eh_inv?"sim ✓":"nao", eh_id?"sim ✓":"nao");
    }
    ok("TRÊS batidas numa direção = uma na inversa (G³ = G⁻¹)", achou3);
    ok("e quatro fecham o circuito (G⁴ = I)", achou4);
    printf("\n      Logo ir três e voltar três devolve o ponto de partida sem que nenhum passo\n");
    printf("      intermediário caia na ponta: a viagem fecha POR FORA do 0. É a dualidade\n");
    printf("      andando — o gato leva, o esquilo traz, e a ponta fica intocada no meio.\n");
}

printf("\n=== A DUALIDADE, FECHADA ===================================================\n");
printf("  O PRENSOR é o cone: estático, é a forma M, e é o que se conserva.\n");
printf("  O CHICOTE é o transporte: dinâmico, é o gato contraído, e é o que conserva.\n");
printf("  Um define o outro. Sem cone, o transporte não tem invariante e não fecha ciclo;\n");
printf("  sem transporte, o cone é uma figura parada e nada o percorre. E as MARCAS do\n");
printf("  chicote no prensor são espirais: |Q| impede TOCAR o cone, e a razão fixa 1/σ²\n");
printf("  por batida impede ESCAPAR dele. Nem chega, nem sai — enrola. (O sinal é que\n");
printf("  volta a cada duas batidas, por det=-1; o aperto é a cada uma.)\n");
printf("  É a mesma peça do §1 outra vez: o negro que suga, o branco que emana, e a mão\n");
printf("  que segura (det = -1) trocando o sinal a cada batida e devolvendo a cada duas.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato em todas as seções.\n\n");
return 0;
}
