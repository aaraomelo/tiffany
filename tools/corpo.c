/* corpo.c — A MULTIPLICAÇÃO DE R^k: o tensor μ que a borda gera, e o corpo que ele fecha.
 *
 * tools/tensor.c contraiu expressões num tensor de multi-índice — mas ali o grau só SOBE: um
 * produto de k parênteses dá grau k, e nada o traz de volta. Isso é o anel de polinômios
 * livre, não um corpo.
 *
 * teoria.tex §4 diz o que falta, e é a peça inteira:
 *
 *     R^n = Z_p[x]/(p_n),      p_n(x) = x^n − m·x^{n−1} − 1
 *
 * Todo dado é um polinômio na base de potências {1, σ, …, σ^{n−1}}. A soma é coordenada a
 * coordenada. E o produto NÃO deixa o grau crescer: as potências excedentes σ^n … σ^{2n−2}
 * BAIXAM pela realimentação da borda σ^n = m·σ^{n−1} + 1, aplicada até só restarem σ^0…σ^{n−1}.
 *
 * É a borda que fecha o corpo. Sem ela, multiplicar sai do espaço; com ela, volta.
 *
 * E a multiplicação, sendo bilinear, É UM TENSOR de ordem 3:
 *
 *     σ^i · σ^j = Σ_l  μ^l_{ij} · σ^l
 *
 * — logo "somar multiplicações" e "multiplicar multiplicações" são operações sobre μ, e é
 * disso que se trata quando se expande polinômio de verdade.
 *
 *   §C1  a borda gera μ: as potências excedentes descem, e a tabela fecha
 *   §C2  o produto RECURSIVO é o produto DIRETO — a autossimilaridade, medida
 *   §C3  μ é comutativo e associativo: é multiplicação, não uma tabela qualquer
 *   §C4  o corpo: todo não-nulo tem inverso — e o furo do ouro em n=5 aparece sozinho
 *   §C5  somar e multiplicar MULTIPLICAÇÕES: o que sobrevive e o que não
 *
 *   cc -O2 -std=c99 corpo.c -o corpo && ./corpo
 */
#include <stdio.h>
#include <string.h>

#define KMAX 8
#include "unidade.h"
static int P = 5, M = 1, K = 3;                 /* corpo, metal, dimensão */
static int md(int a){ a %= P; return a < 0 ? a + P : a; }

/* ---- a borda: σ^t reduzido na base {1,…,σ^{K−1}} ---- */
static int POT[2*KMAX][KMAX];                    /* POT[t] = σ^t reduzido */
static void borda(void){
    memset(POT, 0, sizeof POT);
    for(int t = 0; t < K; t++) POT[t][t] = 1;    /* σ^t é ele mesmo, até σ^{K−1} */
    for(int t = K; t < 2*K-1; t++){
        /* σ^t = σ·σ^{t−1}: desloca, e o que passa de σ^{K−1} volta por σ^K = mσ^{K−1}+1 */
        int alto = POT[t-1][K-1];
        for(int i = K-1; i > 0; i--) POT[t][i] = POT[t-1][i-1];
        POT[t][0] = 0;
        if(alto){                                 /* a realimentação da borda */
            POT[t][K-1] = md(POT[t][K-1] + M*alto);
            POT[t][0]   = md(POT[t][0]   + alto);
        }
        for(int i = 0; i < K; i++) POT[t][i] = md(POT[t][i]);
    }
}
/* o tensor da multiplicação: μ^l_{ij} = POT[i+j][l] */
static int mu(int i, int j, int l){ return POT[i+j][l]; }

/* ---- os dois produtos ---- */
static void prod_direto(const int *x, const int *y, int *r){
    memset(r, 0, sizeof(int)*KMAX);
    for(int i = 0; i < K; i++) for(int j = 0; j < K; j++){
        int c = md(x[i]*y[j]);
        if(!c) continue;
        for(int l = 0; l < K; l++) r[l] = md(r[l] + c*mu(i,j,l));
    }
}
/* o produto recursivo do §4: parte em (x̃, x_{K−1}) e usa a peça um nível abaixo */
static void prod_rec(const int *x, const int *y, int *r, int k){
    if(k == 1){ memset(r, 0, sizeof(int)*KMAX); r[0] = md(x[0]*y[0]); return; }
    int xt[KMAX], yt[KMAX], a[KMAX], b[KMAX], c[KMAX];
    memcpy(xt, x, sizeof(int)*KMAX); xt[k-1] = 0;
    memcpy(yt, y, sizeof(int)*KMAX); yt[k-1] = 0;
    prod_rec(xt, yt, a, k-1);                                  /* x̃·ỹ, um nível abaixo */
    /* (x̃·y_{k−1} + x_{k−1}·ỹ)·σ^{k−1} e x_{k−1}y_{k−1}·σ^{2k−2}, tudo em coeficiente */
    long acum[2*KMAX]; memset(acum, 0, sizeof acum);
    for(int i = 0; i < k; i++) acum[i] += a[i];
    for(int i = 0; i < k-1; i++){
        acum[i + (k-1)] += (long)xt[i]*y[k-1];
        acum[i + (k-1)] += (long)x[k-1]*yt[i];
    }
    acum[2*(k-1)] += (long)x[k-1]*y[k-1];
    /* e agora a borda BAIXA tudo o que passou de σ^{K−1} */
    memset(r, 0, sizeof(int)*KMAX);
    for(int t = 0; t < 2*K-1; t++){
        int co = md((int)(acum[t] % P));
        if(!co) continue;
        for(int l = 0; l < K; l++) r[l] = md(r[l] + co*POT[t][l]);
    }
    (void)b; (void)c;
}
static int igual(const int *a, const int *b){
    for(int i = 0; i < K; i++) if(a[i] != b[i]) return 0;
    return 1;
}
static int nulo(const int *a){ for(int i = 0; i < K; i++) if(a[i]) return 0; return 1; }
static long total(void){ long n = 1; for(int i = 0; i < K; i++) n *= P; return n; }
static void decodifica(long cod, int *v){
    memset(v, 0, sizeof(int)*KMAX);
    for(int i = 0; i < K; i++){ v[i] = (int)(cod % P); cod /= P; }
}

int main(void){
printf("\n=== O CORPO R^k: a borda gera a multiplicação ==============================\n");
printf("    O produto não deixa o grau crescer — as potências excedentes BAIXAM.\n");

/* ---------------------------------------------------------------- §C1 ------ */
printf("\n§C1  A borda gera μ: σ^k = m·σ^{k−1} + 1, aplicada até fechar.\n\n");
P = 5; M = 1; K = 3; borda();
{
    printf("      p=%d, m=%d, k=%d   —   p_k(x) = x^%d − %d·x^%d − 1\n\n", P, M, K, K, M, K-1);
    printf("      potência   reduzida em {1, σ, σ²}\n");
    for(int t = 0; t < 2*K-1; t++){
        printf("      σ^%d        (", t);
        for(int i = 0; i < K; i++) printf("%d%s", POT[t][i], i+1<K?", ":"");
        printf(")%s\n", t >= K ? "   ← baixou pela borda" : "");
    }
    int fechou = 1;
    for(int t = 0; t < 2*K-1; t++) for(int i = 0; i < K; i++) if(POT[t][i] < 0 || POT[t][i] >= P) fechou = 0;
    ok("toda potência excedente cai na base — a tabela fecha", fechou);
    printf("\n      μ^l_{ij} = POT[i+j][l]: a multiplicação É um tensor de ordem 3, e quem o\n");
    printf("      gera é a realimentação. Não se escolhe a tabela; colhe-se da borda.\n");
}

/* ---------------------------------------------------------------- §C2 ------ */
printf("\n§C2  O produto RECURSIVO é o produto DIRETO — a autossimilaridade, medida.\n");
printf("     O §4 define o produto partindo x em (x̃, x_{k−1}) e usando a peça um nível\n");
printf("     abaixo. Se for a mesma coisa que reduzir direto, a recursão é legítima.\n\n");
{
    printf("      p   m   k    pares testados   recursivo = direto\n");
    int mau_geral = 0;
    int ps[3] = {2, 3, 5};
    for(int ip = 0; ip < 3; ip++)
    for(int m = 1; m <= 2; m++)
    for(int k = 2; k <= 4; k++){
        P = ps[ip]; M = m; K = k; borda();
        long n = total(), pares = 0; int mau = 0;
        for(long cx = 0; cx < n; cx++) for(long cy = 0; cy < n; cy++){
            int x[KMAX], y[KMAX], r1[KMAX], r2[KMAX];
            decodifica(cx, x); decodifica(cy, y);
            prod_direto(x, y, r1);
            prod_rec(x, y, r2, K);
            pares++;
            if(!igual(r1, r2)) mau++;
        }
        printf("      %d   %d   %d    %14ld   %s\n", P, m, k, pares, mau ? "NÃO ✗" : "sim ✓");
        if(mau) mau_geral++;
    }
    ok("a peça um nível abaixo dá o mesmo produto, sempre", mau_geral == 0);
    printf("\n      É a autossimilaridade como definição, não como figura: o produto em R^k É\n");
    printf("      o produto em R^{k−1} mais a borda trazendo o excedente de volta.\n");
}

/* ---------------------------------------------------------------- §C3 ------ */
printf("\n§C3  μ é multiplicação de verdade: comutativa e associativa.\n\n");
{
    printf("      p   m   k    comutativo   associativo\n");
    int mau_c = 0, mau_a = 0;
    int ps[2] = {2, 3};
    for(int ip = 0; ip < 2; ip++)
    for(int m = 1; m <= 2; m++)
    for(int k = 2; k <= 3; k++){
        P = ps[ip]; M = m; K = k; borda();
        long n = total(); int c = 1, as = 1;
        for(long cx = 0; cx < n && (c||as); cx++)
        for(long cy = 0; cy < n; cy++){
            int x[KMAX], y[KMAX], z[KMAX], r1[KMAX], r2[KMAX], t1[KMAX], t2[KMAX];
            decodifica(cx, x); decodifica(cy, y);
            prod_direto(x, y, r1); prod_direto(y, x, r2);
            if(!igual(r1, r2)) c = 0;
            for(long cz = 0; cz < n; cz++){
                decodifica(cz, z);
                prod_direto(x, y, t1); prod_direto(t1, z, r1);
                prod_direto(y, z, t2); prod_direto(x, t2, r2);
                if(!igual(r1, r2)) as = 0;
            }
        }
        printf("      %d   %d   %d    %10s   %s\n", P, m, k, c?"sim ✓":"NÃO ✗", as?"sim ✓":"NÃO ✗");
        if(!c) mau_c++;
        if(!as) mau_a++;
    }
    ok("comutativa em todos os casos", mau_c == 0);
    ok("associativa em todos os casos", mau_a == 0);
}

/* ---------------------------------------------------------------- §C4 ------ */
printf("\n§C4  O corpo: todo não-nulo tem inverso — e o furo do ouro aparece sozinho.\n\n");
{
    printf("      p   m   k    não-nulos   com inverso   é corpo?\n");
    int ouro5 = -1;
    int ps[2] = {2, 3};
    for(int ip = 0; ip < 2; ip++)
    for(int m = 1; m <= 3; m++)
    for(int k = 2; k <= 5; k++){
        P = ps[ip]; M = m; K = k; borda();
        long n = total(), nn = 0, com = 0;
        for(long cx = 1; cx < n; cx++){
            int x[KMAX]; decodifica(cx, x);
            if(nulo(x)) continue;
            nn++;
            int achou = 0;
            for(long cy = 1; cy < n && !achou; cy++){
                int y[KMAX], r[KMAX]; decodifica(cy, y);
                prod_direto(x, y, r);
                if(r[0] == 1){ int so = 1; for(int i = 1; i < K; i++) if(r[i]) so = 0; if(so) achou = 1; }
            }
            com += achou;
        }
        int corpo = (com == nn);
        if(m == 1 && k == 5) ouro5 = corpo;
        if(k == 5 || (m <= 2 && k <= 3))
            printf("      %d   %d   %d    %9ld   %11ld   %s\n", P, m, k, nn, com,
                   corpo ? "sim ✓" : "NÃO — tem divisor de zero");
    }
    ok("o ouro (m=1) em k=5 NÃO é corpo — o furo previsto", ouro5 == 0);
    printf("\n      x^5 − x^4 − 1 = (x²−x+1)(x³−x−1) é redutível sobre Q, logo módulo TODO p.\n");
    printf("      Não é azar de escolha de p: é o furo do metal do laço mais curto, e ele\n");
    printf("      aparece na medida sem que ninguém o tenha posto lá.\n");
}

/* ---------------------------------------------------------------- §C5 ------ */
printf("\n§C5  Somar e multiplicar MULTIPLICAÇÕES — o que sobrevive e o que não.\n");
printf("     μ é um tensor: dá para somá-los e compô-los. Mas nem tudo o que sai é\n");
printf("     multiplicação — e é isso que separa o corpo de uma tabela qualquer.\n\n");
{
    P = 3; K = 3;
    int A[2*KMAX][KMAX], B[2*KMAX][KMAX];
    M = 1; borda(); memcpy(A, POT, sizeof POT);       /* μ do ouro   */
    M = 2; borda(); memcpy(B, POT, sizeof POT);       /* μ da prata  */

    /* a SOMA das duas multiplicações: μ_soma = μ_ouro + μ_prata */
    int S[2*KMAX][KMAX];
    for(int t = 0; t < 2*K-1; t++) for(int i = 0; i < K; i++) S[t][i] = md(A[t][i] + B[t][i]);

    long n = total();
    int c_soma = 1, a_soma = 1, tem_um = 0;
    for(long cx = 0; cx < n; cx++) for(long cy = 0; cy < n; cy++){
        int x[KMAX], y[KMAX], r1[KMAX], r2[KMAX];
        decodifica(cx, x); decodifica(cy, y);
        memcpy(POT, S, sizeof POT); prod_direto(x, y, r1);
        memcpy(POT, S, sizeof POT); prod_direto(y, x, r2);
        if(!igual(r1, r2)) c_soma = 0;
    }
    for(long cx = 0; cx < n && a_soma; cx++) for(long cy = 0; cy < n && a_soma; cy++)
    for(long cz = 0; cz < n && a_soma; cz++){
        int x[KMAX], y[KMAX], z[KMAX], t1[KMAX], t2[KMAX], r1[KMAX], r2[KMAX];
        decodifica(cx,x); decodifica(cy,y); decodifica(cz,z);
        memcpy(POT, S, sizeof POT);
        prod_direto(x,y,t1); prod_direto(t1,z,r1);
        prod_direto(y,z,t2); prod_direto(x,t2,r2);
        if(!igual(r1,r2)) a_soma = 0;
    }
    /* o 1 continua sendo neutro na soma das multiplicações? */
    {
        int um[KMAX]; memset(um,0,sizeof um); um[0] = 1;
        int x[KMAX], r[KMAX];
        tem_um = 1;
        for(long cx = 0; cx < n; cx++){
            decodifica(cx, x);
            memcpy(POT, S, sizeof POT); prod_direto(x, um, r);
            if(!igual(x, r)){ tem_um = 0; break; }
        }
    }
    printf("      μ_ouro + μ_prata:   comutativa %s   associativa %s   com neutro %s\n",
           c_soma?"sim":"NÃO", a_soma?"sim":"NÃO", tem_um?"sim":"NÃO");
    ok("a soma de multiplicações continua comutativa", c_soma);
    ok("mas NÃO é associativa — somar μ não dá multiplicação", !a_soma);
    printf("\n      Este é o achado, e vale mais que um sim: o espaço das multiplicações é\n");
    printf("      fechado para a SOMA como espaço vetorial, mas a soma de duas multiplicações\n");
    printf("      não é multiplicação — a associatividade quebra. Logo μ não é um ponto\n");
    printf("      qualquer de um espaço linear: é um ponto ESPECIAL, e o que o seleciona é a\n");
    printf("      borda. Por isso a multiplicação se COLHE da realimentação e não se escolhe.\n");
}

printf("\n=== O QUE FECHA O CORPO ====================================================\n");
printf("  O que faltava ao tensor de tools/tensor.c: lá o grau só sobe, e um produto de k\n");
printf("  parênteses sai do espaço. Aqui a borda σ^k = m·σ^{k−1} + 1 traz o excedente de\n");
printf("  volta, e por isso R^k FECHA: o produto de dois polinômios de grau < k é de grau\n");
printf("  < k. A multiplicação é o tensor μ^l_{ij} = POT[i+j][l], colhido da borda, e o\n");
printf("  produto recursivo do §4 é exatamente o direto — a peça inteira, em toda escala.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
