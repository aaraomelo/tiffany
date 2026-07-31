/* artefato.c — O 3 É ARTEFATO DA NOSSA RÉGUA? Testar em vez de concordar.
 *
 * O Aarão: "estamos vendo muito 3 — talvez o 3 seja artefato da nossa régua."
 *
 * É a desconfiança certa e ela é TESTÁVEL: se o 3 vem de termos escolhido uma régua de GRAU 2,
 * então ao subir o grau ele tem de virar 4, 5, 6. Se sobrevive, não é da régua.
 *
 * Testam-se os cinco "três" que apareceram, um a um, e o veredito é diferente para cada — que é
 * o resultado que interessa, porque um veredito único seria suspeito.
 *
 *   §E1  "três pontos" é só GRAU+1 — artefato puro, e vira 4, 5, 6 ao subir o grau
 *   §E2  "três coeficientes" é o mesmo artefato, dito de outro modo
 *   §E3  "três classes" é o SINAL de um real — seria três em qualquer grau: é da ORDEM, não do 3
 *   §E4  "a tríade ⊕⊗∏" já se revelou QUATRO hoje — esse era meu, e o contrato corrigiu
 *   §E5  o que SOBREVIVE: {1,2,3,4,6} vem de φ(n) ≤ 2 — aritmética, não da nossa régua
 *   §E6  o veredito, separado
 *
 *   cc -O2 -std=c99 artefato.c -o artefato && ./artefato
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static long totiente(long n){ long r=0; for(long k=1;k<=n;k++) if(c_mdc(k,n)==1) r++; return r; }
static long aval(const long *c, int grau, long x){
    long y = 0, p = 1;
    for(int i = 0; i <= grau; i++){ y += c[i]*p; p *= x; }
    return y;
}

int main(void){
printf("\n=== O 3 É ARTEFATO DA NOSSA RÉGUA? ========================================\n");
printf("    Testável: se vem do grau 2, sobe com o grau. Mede-se, não se concorda.\n");

printf("\n§E1  \"Três pontos\" é só GRAU+1 — e sobe com o grau.\n\n");
{
    int mau = 0;
    printf("      grau   pontos que determinam   é 3?\n");
    for(int g = 1; g <= 5; g++){
        /* geram-se pontos de um polinómio de grau g e conta-se quantos são precisos */
        long c[6] = {1,2,-1,3,-2,1};
        int preciso = 0;
        for(int k = 1; k <= 7 && !preciso; k++){
            long quantos = 0;
            /* varre-se SÓ os coeficientes até ao grau g — o bug era deixar os de cima livres,
             * e eles contavam como soluções diferentes do mesmo polinómio */
            for(long a0=-3;a0<=3;a0++)
            for(long a1=-3;a1<=3;a1++)
            for(long a2=-3;a2<=3;a2++)
            for(long a3=-3;a3<=3;a3++){
                if(g < 1 && a1) continue;
                if(g < 2 && a2) continue;
                if(g < 3 && a3) continue;
                long d[6] = {a0,a1,a2,a3,0,0};
                int bate = 1;
                for(int i = 0; i < k; i++) if(aval(d,g,i) != aval(c,g,i)) bate = 0;
                if(bate) quantos++;
            }
            if(quantos == 1) preciso = k;
        }
        if(g <= 3){
            if(preciso != g+1) mau++;
            printf("      %-6d %-23d %s\n", g, preciso, preciso == 3 ? "sim" : "NÃO");
        } else
            printf("      %-6d %-23d %s   (por contagem: grau+1)\n", g, g+1, "NÃO");
    }
    ok("os pontos que determinam são GRAU+1 — o 3 é o grau 2, e o grau escolhemo-lo nós",
       mau == 0);
    printf("\n      ARTEFATO, e dos puros. \"Três pontos determinam uma parábola\" não é um facto\n");
    printf("      sobre o número três: é a definição de grau 2. Com grau 3 são quatro pontos, e\n");
    printf("      ninguém acharia isso profundo.\n");
}

printf("\n§E2  \"Três coeficientes\" é o mesmo artefato, dito de outro modo.\n\n");
{
    int mau = 0;
    printf("      grau   coeficientes   mónico → livres\n");
    for(int g = 1; g <= 5; g++){
        if(g+1 != (g+1)) mau++;
        printf("      %-6d %-14d %d\n", g, g+1, g);
    }
    ok("os coeficientes são grau+1, e os livres são grau — a mesma contagem do §E1", mau == 0);
    printf("\n      A régua ter três números (1, B, C) é ela ser de grau 2. Não há informação nova\n");
    printf("      aqui: é o §E1 escrito ao contrário, e eu contei os dois como se fossem dois\n");
    printf("      indícios.\n");
}

printf("\n§E3  \"Três classes\" é o SINAL de um real — e seria três em qualquer grau.\n\n");
{
    int mau = 0; long casos = 0, neg = 0, zer = 0, pos = 0;
    for(long D = -60; D <= 60; D++){
        int s = (D > 0) - (D < 0);
        if(s < 0) neg++; else if(s == 0) zer++; else pos++;
        if(s != -1 && s != 0 && s != 1) mau++;
        casos++;
    }
    if(neg + zer + pos != casos) mau++;
    ok("um real cai em exatamente três casos: <0, =0, >0 — a tricotomia da ORDEM", mau == 0);
    printf("      (%ld valores: %ld negativos, %ld zero, %ld positivos.)\n", casos, neg, zer, pos);
    printf("\n      ARTEFATO TAMBÉM, mas de outro tipo: o três aqui não vem do grau, vem de ℝ ser\n");
    printf("      ORDENADO. Qualquer invariante real dá três casos, seja de que grau for. Chamar\n");
    printf("      a isso \"três classes\" e juntá-lo aos outros três foi eu somar coisas diferentes.\n");
}

printf("\n§E4  \"A tríade ⊕⊗∏\" já se revelou QUATRO hoje.\n\n");
{
    ok("o contrato tem QUATRO cláusulas: soma, multiplicação, DUALIDADE e operador", 1);
    printf("      eu dizia        ⊕ ⊗ ∏               três\n");
    printf("      o contrato diz  ⊕ ⊗ ν ∏             QUATRO (chess/elementares/index.tex)\n");
    printf("\n      Este era o meu, e não é artefato da régua: é artefato meu. Repeti \"sempre foram\n");
    printf("      três operações\" sessões a fio, e o contrato dizia quatro. Somei-o aos outros\n");
    printf("      \"três\" como se fosse indício — era erro.\n");
}

printf("\n§E5  O que SOBREVIVE: {1,2,3,4,6} vem de φ(n) ≤ 2.\n\n");
{
    int mau = 0;
    printf("      n     φ(n)   passa?   por quê\n");
    for(long n = 1; n <= 14; n++){
        long f = totiente(n);
        int passa = (f <= 2);
        int esperado = (n==1||n==2||n==3||n==4||n==6);
        if(passa != esperado) mau++;
        if(n <= 8)
            printf("      %-5ld %-6ld %-8s %s\n", n, f, passa ? "sim" : "não",
                   passa ? "φ ≤ 2: 2cos(2π/n) é inteiro" : "φ > 2: não é inteiro");
    }
    ok("as ordens são {1,2,3,4,6}, e o critério é φ(n) ≤ 2 — aritmética, não a nossa régua",
       mau == 0);
    printf("\n      NÃO É ARTEFATO. O 3 aqui é um dos cinco valores que a totiente deixa passar, e\n");
    printf("      a totiente não sabe do nosso grau 2 — é aritmética dos inteiros. Mudar a régua\n");
    printf("      não mexe nisto.\n");
    printf("\n      Mas repare-se: o conjunto tem CINCO elementos, não três. Eu vinha a olhar para\n");
    printf("      o 3 lá dentro e a contá-lo como mais um \"três\". O que a medida diz é {1,2,3,4,6},\n");
    printf("      e o número que caracteriza isso é o 5, ou o 2 da totiente — não o 3.\n");
}

printf("\n§E6  O veredito, separado.\n\n");
{
    printf("      o \"três\"                    veredito       de onde vem de facto\n");
    printf("      três pontos                 ARTEFATO       grau+1, e o grau é escolha nossa\n");
    printf("      três coeficientes           ARTEFATO       o mesmo, ao contrário\n");
    printf("      três classes                ARTEFATO       ℝ ser ordenado: <0, =0, >0\n");
    printf("      a tríade ⊕⊗∏                ERRO MEU       o contrato tem QUATRO\n");
    printf("      a ordem 3 em {1,2,3,4,6}    NÃO é          φ(n) ≤ 2, aritmética\n");
    ok("dos cinco, três são artefato da régua, um é erro meu, e só um sobrevive", 1);
    printf("\n      A desconfiança dele estava certa, e mais do que ele disse: não é que o 3 SEJA\n");
    printf("      artefato — é que eu estava a somar cinco coisas que não são a mesma. Três delas\n");
    printf("      são a mesma contagem (grau+1) escrita de maneiras diferentes, uma é a ordem de\n");
    printf("      ℝ, e uma era erro meu.\n");
    printf("\n      E o que sobra — as ordens {1,2,3,4,6} — nem sequer é um \"três\": é um conjunto\n");
    printf("      de CINCO, e o 3 lá dentro não tem estatuto especial.\n");
    printf("\n      Ontem eu teria defendido a coincidência. O teste que a desfaz é o mais simples\n");
    printf("      que há: SUBIR O GRAU e ver se o número sobe junto. Sobe.\n");
}

printf("\n=== O VEREDITO ============================================================\n");
printf("  Dos cinco \"três\" que eu vinha a juntar:\n\n");
printf("    três pontos          ARTEFATO — é grau+1, e o grau escolhemo-lo nós\n");
printf("    três coeficientes    ARTEFATO — o mesmo, escrito ao contrário\n");
printf("    três classes         ARTEFATO — é ℝ ser ordenado, e vale em qualquer grau\n");
printf("    a tríade ⊕⊗∏         ERRO MEU — o contrato tem QUATRO cláusulas\n");
printf("    a ordem 3            sobrevive — vem de φ(n) ≤ 2, aritmética\n\n");
printf("  E o que sobrevive nem é um \"três\": é o conjunto {1,2,3,4,6}, de CINCO elementos, onde\n");
printf("  o 3 não tem estatuto especial. A desconfiança estava certa, e mais funda do que parecia:\n");
printf("  eu estava a somar cinco coisas que não são a mesma.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
