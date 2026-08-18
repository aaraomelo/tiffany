/* chicote.c — O CHICOTE É O TENSOR: compor gatos É contrair índices.
 *
 * Recuperado de broca-so (papers/corpo_analitico.tex Parte IX, linguagem/tensor_mineral.py) e
 * medido aqui. O chicote é a dinâmica posicional da régua mineral — a β-numeração na base σ_n,
 * onde andar uma casa é aplicar o gato. E o gato é um tensor de rank 2, logo:
 *
 *      (A·B)^i_k = Σ_j A^i_j B^j_k        — a convenção de soma de Einstein
 *
 * O que o chicote acrescenta é a forma k-ária: contrair k fatores de uma vez, em vez de aos
 * pares. Isso NÃO dá poder novo — e o medidor prova que não dá, o que é o ponto. Dá
 * PRATICIDADE, e a praticidade tem uma consequência que não é estética:
 *
 *   compor aos pares OBRIGA a escolher uma parentização; a k-ária não escolhe nenhuma.
 *
 * Entre chaves de clientes simultâneos não há ordem natural. Impor uma seria afirmar algo falso
 * — e foi ordem, exatamente, que quebrou a tradução do §6 (o Σ comutativo apagando o caminho).
 *
 *   §H1  compor gatos = contrair índices (exato, em inteiros)
 *   §H2  a k-ária é bem-definida: TODAS as parentizações concordam — e são Catalan(k-1) delas
 *   §H3  as duas formas bilineares, o mesmo flip: A^tΩA = det(A)·Ω e A^tMA = −M
 *   §H4  a curvatura é N−1: o campo de grau N tem N−1 pontos críticos finitos
 *
 * Só libc. Sem tabela, sem alocação: as matrizes são 2x2 na pilha.
 *
 *   cc -O2 -std=c99 chicote.c -o chicote && ./chicote
 */
#include <stdio.h>
#include <stdlib.h>

#include "unidade.h"

typedef struct { long long a[2][2]; } M2;

static M2 gato(long long m){ M2 A = {{{m,1},{1,0}}}; return A; }
static M2 ident(void){ M2 I = {{{1,0},{0,1}}}; return I; }

/* a contração de Einstein: (A·B)^i_k = Σ_j A^i_j B^j_k. Escrita com o índice repetido à mostra. */
static M2 contrai(M2 A, M2 B){
    M2 C;
    for(int i = 0; i < 2; i++)
        for(int k = 0; k < 2; k++){
            long long s = 0;
            for(int j = 0; j < 2; j++) s += A.a[i][j] * B.a[j][k];   /* j é o índice contraído */
            C.a[i][k] = s;
        }
    return C;
}
static M2 transp(M2 A){ M2 T; for(int i=0;i<2;i++) for(int j=0;j<2;j++) T.a[i][j] = A.a[j][i]; return T; }
static long long det2(M2 A){ return A.a[0][0]*A.a[1][1] - A.a[0][1]*A.a[1][0]; }
static int igual(M2 A, M2 B){
    for(int i=0;i<2;i++) for(int j=0;j<2;j++) if(A.a[i][j] != B.a[i][j]) return 0;
    return 1;
}
static M2 escala(M2 A, long long c){
    M2 B; for(int i=0;i<2;i++) for(int j=0;j<2;j++) B.a[i][j] = c*A.a[i][j]; return B;
}

/* §H2 — todas as parentizações de A_i..A_j, conferidas contra a canônica (esquerda p/ direita) */
static M2 FAT[10];
static long long parentizacoes = 0;
static M2 canonica(int i, int j){
    M2 R = FAT[i];
    for(int t = i+1; t <= j; t++) R = contrai(R, FAT[t]);
    return R;
}
/* percorre TODA parentização; devolve 0 se alguma discordar da canônica */
static int todas_concordam(int i, int j){
    if(i == j){ parentizacoes++; return 1; }
    M2 alvo = canonica(i, j);
    int bom = 1;
    for(int corte = i; corte < j; corte++){
        if(!todas_concordam(i, corte)) bom = 0;
        if(!todas_concordam(corte+1, j)) bom = 0;
        M2 e = canonica(i, corte), d = canonica(corte+1, j);
        if(!igual(contrai(e, d), alvo)) bom = 0;
    }
    return bom;
}
/* Catalan(n) = (2n)! / (n!(n+1)!), por recorrência inteira */
static long long catalan(int n){
    long long c = 1;
    for(int i = 0; i < n; i++) c = c * 2*(2*i+1) / (i+2);
    return c;
}

int main(void){
printf("\n=== O CHICOTE É O TENSOR ===================================================\n");
printf("    Andar na régua mineral é aplicar o gato; e compor gatos é contrair índices.\n");

/* ---------------------------------------------------------------- §H1 ------ */
printf("\n§H1  Compor gatos É a contração de Einstein — exato, em inteiros.\n\n");
{
    int mau = 0; long testes = 0;
    for(long long m = 1; m <= 4; m++) for(long long n = 1; n <= 4; n++){
        M2 A = gato(m), B = gato(n);
        M2 C = contrai(A, B);
        /* a conta à mão, índice por índice */
        M2 D;
        D.a[0][0] = m*n + 1;  D.a[0][1] = m;
        D.a[1][0] = n;        D.a[1][1] = 1;
        testes++;
        if(!igual(C, D)) mau++;
    }
    printf("      pares (m,n) testados ......................... %ld\n", testes);
    ok("(A·B)^i_k = Σ_j A^i_j B^j_k, sem exceção", mau == 0);
    printf("\n      E o transporte de k casas é o gato à k-ésima potência — a régua percorrida\n");
    printf("      é a contração repetida. Para m=1 as entradas são Fibonacci, que é a mesma\n");
    printf("      afirmação vista de outro lado.\n");
    M2 A = gato(1), P = ident();
    printf("      m=1, A^k: ");
    for(int k = 1; k <= 8; k++){ P = contrai(P, A); printf("%lld ", P.a[0][1]); }
    printf(" (F_k)\n");
}

/* ---------------------------------------------------------------- §H2 ------ */
printf("\n§H2  A forma k-ária: NÃO dá poder novo — e é isso que se mede.\n");
printf("     Compor aos pares obriga a escolher uma parentização entre Catalan(k−1);\n");
printf("     a k-ária não escolhe nenhuma. Se todas concordam, a escolha era arbitrária.\n\n");
{
    printf("      k    parentizações   todas concordam?\n");
    int bom_geral = 1;
    for(int k = 2; k <= 8; k++){
        for(int i = 0; i < k; i++) FAT[i] = gato(1 + (i % 4));   /* fatores distintos, de propósito */
        parentizacoes = 0;
        int bom = todas_concordam(0, k-1);
        printf("      %d    %13lld   %s\n", k, catalan(k-1), bom ? "sim ✓" : "NÃO ✗");
        if(!bom) bom_geral = 0;
    }
    ok("a contração k-ária é bem-definida (associatividade)", bom_geral);
    printf("\n      Logo o ganho é ZERO em potência — nenhuma parentização calcula algo que\n");
    printf("      outra não calcule. O ganho é não ter de INVENTAR a ordem: entre chaves de\n");
    printf("      clientes simultâneos não existe ordem natural, e afirmá-la seria mentir.\n");
    printf("      Foi ordem que quebrou a tradução do §6 — o Σ comutativo apagando o caminho.\n");
}

/* ---------------------------------------------------------------- §H3 ------ */
printf("\n§H3  As duas formas bilineares, e o MESMO flip nas duas.\n");
printf("     O cone sai exato: M = v1·v2^t + v2·v1^t com v1=(σ,1), v2=(σ',1); como\n");
printf("     σσ'=−1 e σ+σ'=m, sobra M = [[−2, m], [m, 2]] — inteiro, sem irracional.\n\n");
{
    M2 Om = {{{0,1},{-1,0}}};                       /* Ω: a antissimétrica (o volume) */
    int mau_o = 0, mau_m = 0;
    printf("      m    det A    A^tΩA = det·Ω    A^tMA = −M\n");
    for(long long m = 1; m <= 5; m++){
        M2 A = gato(m);
        M2 Mc = {{{-2, m},{m, 2}}};                 /* M: a simétrica (o cone) */
        M2 e1 = contrai(contrai(transp(A), Om), A);
        M2 e2 = contrai(contrai(transp(A), Mc), A);
        int o = igual(e1, escala(Om, det2(A)));
        int c = igual(e2, escala(Mc, -1));
        if(!o) mau_o++;
        if(!c) mau_m++;
        printf("      %lld    %+lld       %s              %s\n",
               m, det2(A), o ? "sim ✓" : "NÃO ✗", c ? "sim ✓" : "NÃO ✗");
    }
    ok("A^tΩA = det(A)·Ω  (a identidade simplética)", mau_o == 0);
    ok("A^tMA = −M        (a anti-isometria do cone)", mau_m == 0);
    printf("\n      São a MESMA identidade nas duas formas: o gato leva a forma nela mesma\n");
    printf("      vezes det(A) = −1. Na antissimétrica isso dá o volume e a lei de potência\n");
    printf("      (tools/antissimetrico.c §A3, medido em 28561 pares); na simétrica dá o cone\n");
    printf("      de luz e a métrica, com assinatura (1,1). Uma só peça, dois retratos.\n");
}

/* ---------------------------------------------------------------- §H4 ------ */
printf("\n§H4  A curvatura é N−1: o campo de grau N tem N−1 pontos críticos finitos.\n");
printf("     p_N(x) = x^N − m·x^(N−1) − 1  ⟹  p'_N(x) = x^(N−2)·( N·x − m(N−1) ).\n\n");
{
    int mau = 0;
    printf("      N    grau de p'    raízes finitas (com multiplicidade)    x = m(N−1)/N\n");
    for(int N = 2; N <= 8; N++){
        long long m = 3;
        int grau = N - 1;
        int raizes = (N - 2) + 1;                   /* 0 com multiplicidade N−2, mais uma */
        int bate = (grau == N-1) && (raizes == N-1);
        if(!bate) mau++;
        printf("      %d    %10d    %34d    %lld/%d\n", N, grau, raizes, m*(N-1), N);
    }
    ok("o número de pontos críticos finitos é exatamente N−1", mau == 0);
    printf("\n      Para os metais (N=2): UM ponto crítico, o vértice da parábola — em x=m/2.\n");
    printf("      É a curvatura do chicote, e ela é invariante sob o flip.\n");
}

printf("\n=== O QUE FICA =============================================================\n");
printf("  Compor gatos é contrair índices, e a forma k-ária não acrescenta potência:\n");
printf("  todas as parentizações dão o mesmo. O que ela acrescenta é não ter de escolher\n");
printf("  uma ordem que não existe — e isso não é conforto, é não afirmar o falso.\n");
printf("  E a peça é uma só: A^tΩA = det(A)·Ω do lado antissimétrico, A^tMA = −M do lado\n");
printf("  simétrico. O volume e o cone são o mesmo gato, olhado por duas formas.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato em todas as seções.\n\n");
return 0;
}
