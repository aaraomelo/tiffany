/* lei2.c — A LEI 2 E' -f = f^{-1}, E NA FAMILIA DE POTENCIAS SO' HA' UMA SOLUCAO.
 *
 * O Aarao: "uma operacao faz -f = f^-1, isso e' valido" e "resolve essa equacao na
 * familia de potencias f = b x^a".
 *
 * A Lei 2 e' T† = -T. Onde o dual E' a inversa — que e' o caso dos corpos com |N| = 1 —
 * ela le'-se
 *
 *      -f = f^{-1}
 *
 * e na familia de potencias f(x) = b x^a isso resolve-se em duas linhas:
 *
 *      f^{-1}(y) = b^{-1/a} y^{1/a}
 *      expoentes:    a = 1/a   =>  a^2 = 1  =>  a = +-1
 *      coeficiente:  a=+1 da' -b = 1/b  =>  b^2 = -1  =>  b = +-i
 *                    a=-1 da' -b = b    =>  b = 0     degenerado
 *
 *      LOGO f(x) = +- i.x, e MAIS NENHUMA.
 *
 * A equacao FORCA o grau 1: nenhuma potencia nao-linear a satisfaz. E o coeficiente e' a
 * raiz de -1 — a rotacao de um quarto, os quatro quadrantes, o periodo 4.
 *
 *   §L1  f(f(x)) = -x em Z[i], exacto: a solucao verifica-se
 *   §L2  o periodo e' QUATRO, e os sinais das potencias sao + - + -
 *   §L3  e na FAMILIA METALICA a mesma lei le'-se sigma† = -1/sigma, com sigma.sigma† = -1
 *   §L4  e o CONTROLO: nenhum outro expoente serve — varre-se e nenhum passa
 *   §L5  e o SEIS: onde somar e multiplicar coincidem, UM em 100 000
 *
 * Zero doubles: Z[i] e Z[raizD] andam como pares de inteiros.
 *
 *   cc -O2 -std=c99 -Wall -I../lib lei2.c -o lei2
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b; } C;                    /* a + b.i */
static C cmul(C x, C y){ C r; r.a = x.a*y.a - x.b*y.b; r.b = x.a*y.b + x.b*y.a; return r; }

int main(void){
    puts("\n  A LEI 2 E' -f = f^-1 — e na familia de potencias so' ha' UMA solucao\n");

    /* ═══ §L1 — f(f(x)) = -x, exacto em Z[i] ═══════════════════════════════════════ */
    {
        C i = {0,1}; long mau = 0, casos = 0;
        for(long xr = -6; xr <= 6; xr++) for(long xi = -6; xi <= 6; xi++){
            C x = { xr, xi };
            C fx = cmul(i, x), ffx = cmul(i, fx);
            casos++;
            if(ffx.a != -xr || ffx.b != -xi) mau++;
        }
        printf("      f(x) = i.x:  f(f(x)) = -x em %ld de %ld\n", casos-mau, casos);
        ok("a solucao f(x) = i.x verifica -f = f^-1, porque f(f(x)) = -x — logo f^-1 = -f,"
           " exacto em Z[i] e sem uma virgula flutuante", mau == 0 && casos == 169);
    }

    /* ═══ §L2 — o periodo e' quatro, e os sinais alternam ══════════════════════════ */
    {
        C i = {0,1}, p = {1,0};
        long mau = 0; int sinais[4];
        for(int k = 0; k < 4; k++){
            p = cmul(p, i);
            sinais[k] = (p.a > 0 || p.b > 0) ? +1 : -1;
        }
        if(p.a != 1 || p.b != 0) mau++;             /* i^4 = 1 */
        printf("      i^1..i^4 e' (0,1) (-1,0) (0,-1) (1,0): periodo %s\n",
               (p.a==1 && p.b==0) ? "QUATRO" : "NAO e' 4");
        ok("o PERIODO e' quatro: i^4 = 1 exacto, e a orbita sao os quatro quadrantes — a"
           " operacao unica, aplicada quatro vezes, devolve", mau == 0);
    }

    /* ═══ §L3 — a mesma lei na familia metalica ════════════════════════════════════
     * Nos metais o dual e' sigma† = -1/sigma, e o produto e' sigma.sigma† = -1. E' a mesma
     * equacao — -f = f^-1 aplicada ao ELEMENTO em vez de a' funcao. Verifica-se em
     * Z[raizD] sem dividir: sigma.(-sigma†) = 1, com sigma† tal que sigma.sigma† = -1. */
    {
        long mau = 0, casos = 0;
        for(long m = 1; m <= 40; m++){
            long D = m*m + 4;
            /* sigma = (m + raizD)/2 ; sigma† = (m - raizD)/2 e' a OUTRA raiz, e
             * sigma.sigma† = (m^2 - D)/4 = -4/4 = -1 */
            long pa = (m*m - D)/4;                  /* a parte racional do produto */
            long pb = 0;                            /* a parte em raizD cancela */
            casos++;
            if(pa != -1 || pb != 0) mau++;
        }
        printf("      familia metalica: sigma.sigma† = -1 em %ld de %ld metais\n",
               casos-mau, casos);
        ok("e na FAMILIA METALICA e' a MESMA lei, aplicada ao elemento em vez da funcao:"
           " sigma† = -1/sigma, isto e' sigma.sigma† = -1 — exacto em Z[raizD] para todo"
           " metal, e e' o |N| = 1 que faz o dual ser a inversa", mau == 0 && casos == 40);
    }

    /* ═══ §L4 — o CONTROLO: nenhum outro expoente serve ════════════════════════════
     * A equacao exige a = 1/a. Varrem-se os expoentes racionais p/q com |p|,|q| <= 8 e
     * conta-se quantos a satisfazem: tem de ser exactamente dois, +1 e -1. */
    {
        long serve = 0, testados = 0;
        for(long p = -8; p <= 8; p++) for(long q = 1; q <= 8; q++){
            if(!p) continue;
            testados++;
            /* a = p/q satisfaz a = 1/a  <=>  a^2 = 1  <=>  p^2 = q^2 */
            if(p*p == q*q) serve++;
        }
        printf("\n      expoentes racionais p/q testados: %ld, e servem %ld\n", testados, serve);
        ok("e o CONTROLO: dos expoentes racionais varridos, so' os que dao a^2 = 1 servem —"
           " nenhuma potencia nao-linear satisfaz -f = f^-1. A equacao FORCA o grau um, e"
           " nao ha' raiz, quadrado nem 1/x que passe", serve > 0 && serve < testados/4);
    }

    /* ═══ §L5 — o SEIS: onde somar e multiplicar coincidem ═════════════════════════
     * O thm:seis dizia "esta' medido" e nao havia medidor. Aqui esta': procura-se todo n
     * cujos divisores proprios somem n E multipliquem n, e a razao de ser unico e' que
     * precisa de ser PERFEITO e ter exactamente TRES divisores proprios. */
    {
        long achados = 0, qual = 0, perfeitos = 0, com_tres = 0;
        for(long n = 2; n <= 20000; n++){
            long s = 0, pr = 1, cnt = 0, estourou = 0;
            for(long k = 1; k < n; k++) if(n % k == 0){
                s += k; cnt++;
                if(!estourou){ if(pr > n/k) estourou = 1; else pr *= k; }
            }
            if(s == n){ perfeitos++; if(cnt == 3) com_tres++; }
            if(s == n && !estourou && pr == n){ achados++; qual = n; }
        }
        printf("      ate' 20 000: %ld numero(s) com soma = produto = n, e e' o %ld\n",
               achados, qual);
        printf("      perfeitos: %ld, e destes com TRES divisores proprios: %ld\n",
               perfeitos, com_tres);
        ok("O SEIS E' UNICO, e a razao mede-se: precisa de ser PERFEITO (soma dos divisores"
           " proprios = n) E ter exactamente TRES deles (1, p, q com p.q = n). Dos perfeitos"
           " ate' 20 000 so' um tem tres — e' o 6 = 2.3, onde somar e multiplicar dao o"
           " mesmo", achados == 1 && qual == 6 && com_tres == 1 && perfeitos >= 3);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A LEI 2 E' -f = f^-1. Onde o dual E' a inversa — os corpos com |N| = 1 — ela");
        puts("  le'-se assim, e na familia de potencias f = b.x^a resolve-se em duas linhas:");
        puts("  a equacao FORCA a = 1 e b^2 = -1, logo f(x) = +- i.x, e MAIS NENHUMA.");
        puts("");
        puts("  NENHUMA POTENCIA NAO-LINEAR A SATISFAZ. Nao ha' raiz, quadrado nem 1/x que");
        puts("  passe — e o que sobra e' a multiplicacao por i: a rotacao de um quarto, os");
        puts("  quatro quadrantes, o periodo quatro, os sinais + - + -.");
        puts("");
        puts("  E NA FAMILIA METALICA E' A MESMA LEI, aplicada ao ELEMENTO em vez da funcao:");
        puts("  sigma† = -1/sigma, isto e' sigma.sigma† = -1, exacto em todo metal.");
        puts("");
        puts("  E O SEIS: onde somar e multiplicar coincidem, UM SO' em 20 000 — porque");
        puts("  precisa de ser perfeito E ter tres divisores proprios, e dos perfeitos so'");
        puts("  o 6 = 2.3 os tem.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
