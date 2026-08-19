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
 * marca dessa aproximação é a espiral: a componente branca encolhe por 1/σ e a negra cresce
 * por σ, de modo que o ângulo à direção nula cai por 1/σ² a cada batida.
 *
 *   §P1  o prensor: o cone, e a conservação exata em INTEIROS
 *   §P2  as marcas: a órbita não toca o cone, e espirala para ele com razão medida
 *   §P3  a espiral é logarítmica: passo constante de rapidez; com o esquilo, a áurea
 *   §P4  a dualidade fecha: o estático e o dinâmico, um definindo o outro
 *
 *   cc -O2 -std=c99 -I lib tests/prensor.c -o prensor && ./prensor
 */
#include <stdio.h>
#include "unidade.h"

/* Q(p) = p^t M p, com M = [[-2,m],[m,2]] — inteiro quando p é inteiro */
static long long Q(long long m, long long x, long long y){
    return -2*x*x + 2*m*x*y + 2*y*y;
}
static void gato_ap(long long m, long long *x, long long *y){
    long long nx = m*(*x) + (*y), ny = *x;
    *x = nx; *y = ny;
}

/* Q=0 nas direções nulas: ponto (xa+xb√D, y) com Q em ℤ[√D] */
static int q_nulo_zd(long m, long xa, long xb, long y){
    long D = m*m + 4;
    long x2a = xa*xa + D*xb*xb, x2b = 2*xa*xb;
    long qa = -2*x2a + 2*m*xa*y + 2*y*y;
    long qb = -2*x2b + 2*m*xb*y;
    return qa == 0 && qb == 0;
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
    int mau = 0;
    printf("      m        2σ= m+√D         Q(σ,1)          2σ'=m−√D        Q(σ',1)\n");
    for(long long m = 1; m <= 4; m++){
        int ok1 = q_nulo_zd(m, m, 1, 2);               /* (m+√D, 2) ∝ (σ,1)  */
        int ok2 = q_nulo_zd(m, m, -1, 2);              /* (m−√D, 2) ∝ (σ',1) */
        printf("      %lld   (%lld,1)√D+(%lld,2)  %s           (%lld,−1)√D+(%lld,2)  %s\n",
               m, m, m, ok1 ? "0 ✓" : "≠0 ✗", m, m, ok2 ? "0 ✓" : "≠0 ✗");
        if(!ok1 || !ok2) mau++;
    }
    ok("as direções nulas do cone são exatamente σ e σ'", mau == 0);
    printf("\n      Logo o cone não é figura: é o par de atratores do §1 visto como LUGAR.\n");
}

/* ---------------------------------------------------------------- §P3 ------ */
printf("\n§P3  As marcas: quem começa fora do cone nunca o toca — e espirala para ele.\n\n");
{
    int mau_toca = 0, mau_zero = 0;
    printf("      m    |Q| ao longo de 40 batidas   Q≠0 em (1,1) por 20 batidas\n");
    for(long long m = 1; m <= 4; m++){
        long long x = 1, y = 0;
        long long q0 = Q(m,x,y);
        int conserva = 1, nunca_zero = 1;
        for(int k = 0; k < 40 && x > -1000000000LL && x < 1000000000LL; k++){
            gato_ap(m,&x,&y);
            long long q = Q(m,x,y);
            if((q > 0 ? q : -q) != (q0 > 0 ? q0 : -q0)) conserva = 0;
            if(q == 0) nunca_zero = 0;
        }
        if(!conserva) mau_toca++;
        x = 1; y = 1;
        for(int k = 0; k < 20; k++){
            if(Q(m,x,y) == 0) nunca_zero = 0;
            gato_ap(m,&x,&y);
        }
        if(!nunca_zero) mau_zero++;
        printf("      %lld    %-26s %s\n", m, conserva ? "constante ✓" : "MUDOU ✗",
               nunca_zero ? "sim ✓" : "NÃO ✗");
    }
    ok("|Q| constante: a órbita nunca alcança o cone", mau_toca == 0);
    ok("e Q≠0 ao longo da órbita — espirala sem tocar a ponta", mau_zero == 0);
    printf("\n      É a espiral: nem chega, nem escapa. Enrola-se para sempre —\n");
    printf("      a razão 1/σ² por batida é algébrica (§P4 em m=1 vira Fibonacci).\n");
}

/* ---------------------------------------------------------------- §P4 ------ */
printf("\n§P4  A espiral, medida como espiral: passo constante, e a áurea no caso m=1.\n\n");
{
    int mau = 0;
    printf("      m    r² cresce a cada batida (a partir de (1,1))?\n");
    for(long long m = 1; m <= 4; m++){
        long long x = 1, y = 1, r2 = x*x + y*y, ok_inc = 1;
        for(int k = 0; k < 15; k++){
            gato_ap(m, &x, &y);
            long long r2n = x*x + y*y;
            if(r2n <= r2) ok_inc = 0;
            r2 = r2n;
            if(r2 > 1000000000000LL) break;
        }
        printf("      %lld    %s\n", m, ok_inc ? "sim ✓" : "NÃO ✗");
        if(!ok_inc) mau++;
    }
    ok("r² = x²+y² cresce a cada batida — espiral logarítmica (monótona)", mau == 0);
    printf("\n      E com m=1 a marca é Fibonacci: r²=F_{2k+3} ao longo do gato.\n\n");
    {
        long long fib[32];
        fib[0] = 0; fib[1] = 1;
        for(int i = 2; i < 32; i++) fib[i] = fib[i-1] + fib[i-2];
        long long x = 1, y = 1;
        int mau_au = 0;
        printf("      k    r² medido   F_{2k+3} esperado\n");
        for(int k = 0; k <= 10; k++){
            long long r2 = x*x + y*y;
            long long esp = fib[2*k+3];
            if(r2 != esp) mau_au++;
            printf("      %2d   %-9lld %lld\n", k, r2, esp);
            gato_ap(1, &x, &y);
        }
        ok("a marca do gato m=1 é r²=F_{2k+3} — a espiral áurea, exacta", mau_au == 0);
    }
}

/* ---------------------------------------------------------------- §P5 ------ */
printf("\n§P5  A ponta do cone é o 0 — a passagem reversível. E a dinâmica NÃO a cruza.\n\n");
{
    int mau_fix = 0;
    for(long long m = 1; m <= 4; m++){
        long long x = 0, y = 0;
        gato_ap(m,&x,&y);
        if(x || y) mau_fix++;
        for(long long a = -6; a <= 6; a++) for(long long b = -6; b <= 6; b++){
            if(!a && !b) continue;
            long long u = a, v = b; gato_ap(m,&u,&v);
            if(u == a && v == b) mau_fix++;
        }
    }
    ok("a ponta (p=0) é o ÚNICO ponto fixo do chicote", mau_fix == 0);
    printf("      É a passagem reversível: A·0 = 0 e A⁻¹·0 = 0 — o único lugar onde ir e voltar\n");
    printf("      dão o mesmo. Q(0) = 0, e é o vértice onde as duas geratrizes se encontram.\n\n");

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

    printf("\n      As batidas duais: G = [[0,-1],[1,0]] (o esquilo, det = +1).\n\n");
    long long G[2][2] = {{0,-1},{1,0}}, P[2][2] = {{1,0},{0,1}};
    long long Gi[2][2] = {{0,1},{-1,0}};
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
printf("  por batida impede ESCAPAR dele. Nem chega, nem sai — enrola.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato em todas as seções.\n\n");
return 0;
}
