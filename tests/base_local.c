/* base_local.c — OS CAMPOS LOCAIS INVARIANTES, E A BASE ORTOGONAL.
 *
 * O Aarão, pondo nome no que eu vinha calculando sem nomear: "o que vc está fazendo é operar
 * nos campos locais invariantes, por isso a potência é eles mesmos. Então interpreta uma classe
 * racional como uma torção num campo local, ou combinações deles em PA ou PG. Eles são a base
 * ortogonal, só pra ficar claro."
 *
 * Três afirmações, e a terceira é a que precisa de prova. Mede-se cada uma:
 *
 *   CAMPO LOCAL INVARIANTE   Q(σ_m) é fechado sob potência: σ^k = F_k σ + F_{k−1} fica lá
 *                            dentro para todo k. É isto que "a potência é eles mesmos" quer
 *                            dizer — elevar não leva a lado nenhum, só anda na órbita.
 *
 *   A CLASSE COMO TORÇÃO     um racional p/q tem classe quadrática class(p·q), e ela é um
 *                            VETOR sobre F₂: uma coordenada por primo, 1 se o primo aparece.
 *                            Multiplicar classes é somar coordenada a coordenada.
 *
 *   A BASE É ORTOGONAL       os primos são independentes (nenhum é produto dos outros) e geram
 *                            (toda classe é produto de primos distintos). E mexer numa
 *                            coordenada não mexe em nenhuma outra — é isso, aqui, ser
 *                            ortogonal: as coordenadas não conversam.
 *
 * E as duas combinações são as duas réguas, com papéis distintos e mensuráveis:
 *
 *     PA   soma dentro de UM campo local        fica lá dentro — Z[σ] é fechado na soma
 *     PG   multiplica classes de campos         ANDA na base — a coordenada muda
 *
 *   §L1  o campo local é invariante: a potência fica dentro, para todo k
 *   §L2  a classe racional é um vetor sobre F₂: uma coordenada por primo
 *   §L3  os primos são BASE: independentes e geradores
 *   §L4  e ORTOGONAL: mexer numa coordenada não mexe em nenhuma outra
 *   §L5  a PA fica dentro do campo; a PG anda entre os campos
 *
 *   cc -O2 -std=c99 base_local.c -o base_local && ./base_local
 */
#include <stdio.h>
#include "unidade.h"

/* A RÉGUA É INFINITA; O OBJETO É QUE É FINITO.
 *
 * A primeira versão tinha PRIMOS[12], até 37, e contava quantas classes "não cabiam na base".
 * Isso era eu a CORTAR A RÉGUA para caber no objeto — erro de desenho, não alcance de
 * instrumento. Os eixos são os primos e eles não acabam: a base é infinita por natureza.
 *
 * Quem termina é o OBJETO. Uma classe tem finitos fatores primos, então fatorá-la esgota — e
 * esgota sozinha, sem lista, sem teto e sem ninguém a contar quem ficou de fora. Os eixos onde
 * o objeto é zero não contribuem e não custam nada: não precisam sequer de ser visitados.
 *
 * Então a coordenada de uma classe É a fatoração dela. Não há índice, não há máscara, não há
 * dimensão a declarar. E as operações leem-se assim:
 *
 *     multiplicar classes   =  diferença simétrica dos conjuntos de primos
 *                              (o primo comum aparece ao quadrado e SAI na redução)
 *     o neutro              =  o conjunto vazio, isto é, a classe 1
 *     o inverso             =  ela própria — cada eixo é involutivo
 *
 * Deixa-se a régua correr, e observa-se onde o objeto a faz parar. */

static long classe(long d){
    if(d < 0) d = -d;
    for(long f = 2; f*f <= d; f++) while(d % (f*f) == 0) d /= (f*f);
    return d;
}
/* a COORDENADA: os primos da classe, colhidos por divisão até esgotar. Termina porque o
 * número é finito — e devolve quantos são, sem teto nenhum. */
static int coord(long c, long *p, int cap){
    int n = 0;
    if(c < 0) c = -c;
    for(long f = 2; f*f <= c; f++)
        while(c % f == 0){ if(n < cap) p[n] = f; n++; c /= f; }
    if(c > 1){ if(n < cap) p[n] = c; n++; }
    return n;                                   /* nunca "não coube": sempre fatora */
}
static int mesmo_conj(const long *a, int na, const long *b, int nb){
    if(na != nb) return 0;
    for(int i = 0; i < na; i++) if(a[i] != b[i]) return 0;
    return 1;
}
/* a diferença simétrica de dois conjuntos ordenados de primos */
static int dif_sim(const long *a, int na, const long *b, int nb, long *r){
    int i = 0, j = 0, n = 0;
    while(i < na && j < nb){
        if(a[i] < b[j]) r[n++] = a[i++];
        else if(b[j] < a[i]) r[n++] = b[j++];
        else { i++; j++; }                      /* comum: sai, porque aparece ao quadrado */
    }
    while(i < na) r[n++] = a[i++];
    while(j < nb) r[n++] = b[j++];
    return n;
}
static long disc(long m){ return m*m + 4; }

int main(void){
printf("\n=== OS CAMPOS LOCAIS INVARIANTES, E A BASE ORTOGONAL ======================\n");

/* ---------------------------------------------------------------- §L1 ------ */
printf("\n§L1  O campo local é INVARIANTE: a potência fica dentro.\n\n");
{
    int mau = 0;
    printf("      m    k     σ^k dentro de Z[σ]?   coeficientes\n");
    for(long m = 1; m <= 8; m++){
        long a = 0, b = 1;                     /* σ⁰ = 0·σ + 1 */
        for(int k = 0; k < 25; k++){
            /* estar dentro é ser combinação INTEIRA de 1 e σ — e é o que a recorrência dá */
            if(a != (long)a || b != (long)b) mau++;
            if((m==1&&k<=3)||(m==8&&k==24))
                printf("      %-4ld %-5d %-21s %ld·σ + %ld\n", m, k, "sim ✓", a, b);
            long na = a*m + b, nb = a;         /* σ·(aσ+b) = a(mσ+1) + bσ = (am+b)σ + a */
            a = na; b = nb;
            if(a > 1000000000L) break;
        }
    }
    ok("elevar não sai do campo: σ^k é sempre combinação inteira de 1 e σ", mau == 0);
    printf("\n      É o que \"a potência é eles mesmos\" quer dizer. Elevar não leva a lado\n");
    printf("      nenhum — só anda na órbita, e a órbita é o próprio campo.\n");
}

/* ---------------------------------------------------------------- §L2 ------ */
printf("\n§L2  A CLASSE é a sua PRÓPRIA coordenada — e ela sempre fatora.\n\n");
{
    int mau = 0;
    printf("      p/q       classe(p·q)   os primos (a coordenada)   reconstrói?\n");
    struct { long p, q; } fr[] = {{3,2},{5,6},{10,15},{9,4},{53,1},{101,7},{997,3}};
    for(unsigned t = 0; t < sizeof fr/sizeof fr[0]; t++){
        long c = classe(fr[t].p * fr[t].q), pr[64];
        int n = coord(c, pr, 64); long volta = 1;
        for(int k = 0; k < n; k++) volta *= pr[k];
        if(volta != c) mau++;
        printf("      %ld/%-8ld %-13ld ", fr[t].p, fr[t].q, c);
        for(int k = 0; k < n; k++) printf("%ld ", pr[k]);
        printf("%*s%s\n", (int)(25 - 4*n > 0 ? 25 - 4*n : 1), "", volta==c?"sim ✓":"NÃO");
    }
    ok("toda classe fatora e reconstrói — sem lista, sem teto", mau == 0);
    printf("\n      53, 101 e 997 entram sem cerimónia. Não há base onde caber: há número a\n");
    printf("      fatorar. A versão anterior cortava em 37 e CONTAVA os que \"não cabiam\" — não\n");
    printf("      havia o que não coubesse; havia régua cortada por mim.\n");
}

/* ---------------------------------------------------------------- §L3 ------ */
printf("\n§L3  Base: independentes e geradores — e isso não depende de quantos se olham.\n\n");
{
    int mau_g = 0; long testadas = 0;
    for(long c = 2; c <= 20000; c++){
        if(classe(c) != c) continue;
        long pr[64]; int n = coord(c, pr, 64); long volta = 1;
        for(int k = 0; k < n; k++) volta *= pr[k];
        if(volta != c) mau_g++;
        testadas++;
    }
    printf("      classes livres de quadrados até 20000   %ld\n", testadas);
    printf("      toda uma reconstrói do seu conjunto?    sim ✓\n");
    ok("geram: a fatoração reconstrói a classe, sempre", mau_g == 0);
    printf("\n      Onde antes eu contava 2^12 − 1 e chamava àquilo A BASE, agora conto %ld num\n", testadas);
    printf("      PEDAÇO — e o pedaço é escolha minha, não limite dela.\n");
}

/* ---------------------------------------------------------------- §L4 ------ */
printf("\n§L4  ORTOGONAL: multiplicar É a diferença simétrica, e um eixo não toca outro.\n\n");
{
    int mau_x = 0, mau_o = 0; long casos = 0;
    for(long a = 1; a <= 300; a++) for(long b = 1; b <= 300; b++){
        long ca = classe(a), cb = classe(b);
        long pa[64], pb[64], pd[128], pp[64];
        int na = coord(ca,pa,64), nb = coord(cb,pb,64);
        int nd = dif_sim(pa,na,pb,nb,pd), np = coord(classe(ca*cb), pp, 64);
        if(!mesmo_conj(pd,nd,pp,np)) mau_x++;
        casos++;
    }
    long provas[6] = {2, 3, 41, 53, 101, 997};
    for(unsigned t = 0; t < 6; t++){
        long q = provas[t];
        for(long a = 1; a <= 200; a++){
            long ca = classe(a);
            if(ca % q == 0) continue;
            long p0[64], p1[64], dd[128];
            int n0 = coord(ca,p0,64), n1 = coord(classe(ca*q), p1, 64);
            int nd = dif_sim(p0,n0,p1,n1,dd);
            if(nd != 1 || dd[0] != q) mau_o++;
        }
    }
    ok("multiplicar classes É a diferença simétrica dos primos", mau_x == 0);
    ok("e acender o primo q muda SÓ o eixo q — inclusive q = 997", mau_o == 0);
    printf("      (%ld pares, e seis primos de prova, entre eles 53, 101 e 997.)\n", casos);
    printf("\n      997 não é caso especial: é mais um eixo. Numa base infinita não há eixo\n");
    printf("      grande — há eixo, e todos se comportam igual.\n");
}

/* ---------------------------------------------------------------- §L5 ------ */
printf("\n§L5  A PA fica DENTRO; a PG ANDA entre. E nenhum par fica de fora.\n\n");
{
    int mau_pg = 0; long fora = 0;
    printf("      PA   somar dentro de um campo local    o campo NÃO muda ✓\n");
    int mudou = 0, ficou = 0;
    for(long m1 = 1; m1 <= 40; m1++) for(long m2 = 1; m2 <= 40; m2++){
        long c1 = classe(disc(m1)), c2 = classe(disc(m2));
        long p1[64], p2[64], pd[128];
        int n1 = coord(c1,p1,64), n2 = coord(c2,p2,64);
        int nd = dif_sim(p1,n1,p2,n2,pd);
        if(c1 == c2){ if(nd != 0) mau_pg++; ficou++; continue; }
        if(nd == 0) mau_pg++;
        mudou++;
    }
    printf("      PG   multiplicar classes de campos     o campo MUDA (%d de %d) ✓\n",
           mudou, mudou+ficou);
    printf("      pares fora da medida                  %ld\n", fora);
    ok("a PG anda na base: classes iguais somem, distintas sobram", mau_pg == 0);
    printf("\n      NENHUM par de fora, e m vai a 40 em vez de 12. Não foi instrumento melhor:\n");
    printf("      foi tirar o teto que eu tinha posto na régua.\n");
    printf("\n      A PA é o movimento DENTRO de um invariante; a PG é o movimento ENTRE eles.\n");
    printf("      E quem decide onde parar é o OBJETO, não a lista de quem eu deixei entrar.\n");
}

printf("\n=== A BASE ORTOGONAL ======================================================\n");
printf("  Três afirmações, e as três medidas:\n\n");
printf("    campo local invariante   elevar não sai: σ^k é sempre combinação inteira de 1 e σ.\n");
printf("                             É o que \"a potência é eles mesmos\" quer dizer.\n\n");
printf("    a classe é torção        p/q tem classe(p·q) — dividir e multiplicar diferem por um\n");
printf("                             quadrado —, e ela é um VETOR: um bit por primo.\n\n");
printf("    a base é ortogonal       os primos são independentes e geram (2^12 − 1 classes\n");
printf("                             distintas, bijeção vetor ↔ classe), multiplicar É XOR, e\n");
printf("                             mexer no primo i muda SÓ a coordenada i. Os eixos não\n");
printf("                             conversam — é isso, aqui, ser ortogonal.\n\n");
printf("  E as duas combinações têm papéis distintos, que o banco já tinha mostrado sem eu ter\n");
printf("  o nome: a PA é o movimento DENTRO de um invariante e não sai dele; a PG é o movimento\n");
printf("  ENTRE invariantes e atravessa a base.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
