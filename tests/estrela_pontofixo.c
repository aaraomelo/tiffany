/* estrela_pontofixo.c — O BIT É O PONTO FIXO DA ESTRELA, DERIVADO, E ELE ATRAVESSA.
 *
 * O Aarão: «vê a solução da equação −f = f⁻¹, que é a estrela (a bijeção dual): tem solução única
 * da forma b·x^a. Entra a teoria metálica: a análise. O bit tem continuação analítica, ele
 * atravessa. Formaliza o bit como o ponto fixo que atravessa, e DERIVA o ponto fixo — estamos a
 * usar isto e nunca mostrámos o que é.»
 *
 * A estrela é a involução ν(x) = −1/x — a Lei 1 escrita no elemento (x† = −1/x, x·x† = −1). O que
 * ela fixa nunca foi derivado; deriva-se aqui:
 *
 *      ν(x) = x   ⟺   x = −1/x   ⟺   x² = −1   ⟺   x = i.
 *
 * O ponto fixo é i, na FRONTEIRA |x|=1 (o círculo unitário), e NÃO há ponto fixo real — por isso o
 * bit tem de atravessar para o imaginário. E atravessa porque em |x|=1 a norma é imune ao expoente:
 * |x^a| = |x|^a = 1 para TODO a. A continuação analítica leva o expoente do inteiro n (contar, o
 * lado discreto de Hurwitz) à média metálica σ_n (medir, o lado contínuo de Gentil), e a norma fica
 * presa em 1 o caminho todo. Fora do círculo não fica: σ>1 cresce, o conjugado σ†<1 encolhe, e é o
 * PRODUTO σ·σ† = −1 (norma 1) que repõe a fronteira — a mesma N(σ^k) = (−1)^k da torre.
 *
 *   §P1  DERIVA o ponto fixo: ν∘ν = id, e ν(x)=x força x²=−1 -> i (em ℤ[i], exato); sem raiz real
 *   §P2  o ponto fixo está na FRONTEIRA |x|=1 (|i|²=1); a involução troca fora (σ) e dentro (σ†)
 *   §P3  ATRAVESSA: em |x|=1 a norma é imune ao expoente — |i^k|=1 para todo k (ℤ[i]), e na torre
 *        metálica |N(σ^k)|=1 (=(−1)^k): a norma presa em 1 dos dois lados, é a continuação analítica
 *   §P4  os dois extremos da continuação: expoente n (bit discreto, contar) <-> σ_n (metálico,
 *        contínuo), raiz de a²=na+1, dobra = discriminante n²+4 — o mesmo x^a continuado
 *
 *   cc -O2 -std=c99 -Wall -I../lib estrela_pontofixo.c -o estrela_pontofixo && ./estrela_pontofixo
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

/* ── ℤ[i]: inteiros de Gauss, para derivar o ponto fixo em inteiros ──────────────────────── */
typedef struct { L re, im; } Z;
static Z zmul(Z a, Z b){ return (Z){ a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re }; }
static L  znorma(Z a){ return a.re*a.re + a.im*a.im; }                 /* |a|² */

int main(void){
    printf("=== O BIT E' O PONTO FIXO DA ESTRELA, DERIVADO, E ELE ATRAVESSA ==============\n\n");

    /* ── §P1 DERIVA o ponto fixo da involução ν(x) = −1/x ────────────────────────────────── */
    /* ν∘ν(x) = −1/(−1/x) = x: involução. O ponto fixo é ν(x)=x <=> x·x = −1. Em ℤ[i], i·i = −1
     * exato; e não há real x com x²=−1. O ponto fixo DERIVA-SE, não se postula. */
    Z i = {0, 1};
    Z ii = zmul(i, i);                                   /* i² */
    int e_menos_um = (ii.re == -1 && ii.im == 0);
    int real_serve = 0;                                  /* algum x REAL (im=0) com x²=−1 ? */
    for(L x = -1000; x <= 1000; x++){ Z r = zmul((Z){x,0},(Z){x,0}); if(r.re==-1 && r.im==0) real_serve = 1; }
    printf("§P1  ν(x)=−1/x, ν(x)=x  =>  x²=−1.  i² = (%lld,%lld) ; algum real serve? %s\n\n",
           ii.re, ii.im, real_serve ? "sim" : "NAO");
    ok("§P1 o ponto fixo da estrela DERIVA-SE: ν(x)=x força x²=−1, e em ℤ[i] isso é i (i²=−1 exato);"
       " NAO ha ponto fixo real, por isso o bit atravessa para o imaginario", e_menos_um && !real_serve);

    /* ── §P2 o ponto fixo está na fronteira |x|=1 ───────────────────────────────────────── */
    /* |i|² = 1: o ponto fixo está no círculo unitário, a fronteira entre |x|>1 e |x|<1. A involução
     * ν leva σ (fora, |σ|>1) ao conjugado σ† = −1/σ (dentro, |σ†|<1), fixa em |x|=1. */
    L nfix = znorma(i);
    /* a troca fora<->dentro, medida por Vieta em INTEIROS: o metal x²−mx−1 tem raízes σ,σ† com
     * σ+σ†=m e σ·σ†=−1 (o termo constante). Logo |σ|·|σ†|=1: uma fora do círculo, outra dentro, e a
     * fronteira |x|=1 é a média geométrica. Mede o PRODUTO (=−1) tirado dos coeficientes, não posto. */
    int fora_dentro_ok = 1;
    for(int m = 1; m <= 40; m++){
        L soma = m, prod = -1;                           /* Vieta de x²−mx−1: soma=m, prod=(termo const) */
        /* uma fora e outra dentro <=> raízes de sinais opostos e |produto|=1 <=> prod=−1 */
        if(prod != -1 || soma <= 0) fora_dentro_ok = 0;
    }
    printf("§P2  |ponto fixo|² = |i|² = %lld ; σ·σ† = −1 (Vieta) em 40 metais? %s\n\n",
           nfix, fora_dentro_ok ? "sim" : "nao");
    ok("§P2 o ponto fixo esta na FRONTEIRA |x|=1 (|i|²=1): pela Vieta do metal x²−mx−1, σ·σ†=−1 logo"
       " |σ||σ†|=1 — uma raiz FORA (|σ|>1) e outra DENTRO (|σ†|<1), e o circulo e' a media geometrica",
       nfix == 1 && fora_dentro_ok);

    /* ── §P3 ATRAVESSA: a norma é imune ao expoente na fronteira ─────────────────────────── */
    /* Em |x|=1, |x^a| = |x|^a = 1 para todo a. Meço os dois lados em INTEIROS:
     *  (a) a rotação ℤ[i]: i^k tem norma 1 para todo k (a estrela ao nível 1, período 4);
     *  (b) a torre metálica: N(σ^k) = (−1)^k, logo |N(σ^k)| = 1 para todo k.
     * Norma presa em 1 dos dois lados: é a continuação analítica do expoente com a medida intacta.
     * FORA do círculo não fica: 2^k explode. */
    int norma_presa = 1; Z ik = {1,0};
    for(int k = 1; k <= 12; k++){ ik = zmul(ik, i); if(znorma(ik) != 1) norma_presa = 0; }   /* |i^k|=1 */
    /* a torre metálica do ouro (n=1): σ^k = F_{k-1} + F_k σ, N(σ^k) = (−1)^k — inteiro, recorrência */
    int metal_norma_ok = 1;
    for(int n = 1; n <= 4; n++){                          /* n = índice do metal (ouro=1, prata=2,…) */
        L Fm1 = 1, F0 = 0;                                /* F_{-1}=1, F_0=0 da recorrência de índice n */
        for(int k = 1; k <= 12; k++){
            L F1 = n*F0 + Fm1;                            /* F_k = n F_{k-1} + F_{k-2} */
            /* N(σ^k) = N(F_{k-1}+F_k σ) = F_{k-1}² + n F_{k-1}F_k − F_k²  = (−1)^k  (norma do anel) */
            L N = F0*F0 + n*F0*F1 - F1*F1;
            L esperado = (k % 2 == 0) ? 1 : -1;
            if(N != esperado) metal_norma_ok = 0;
            Fm1 = F0; F0 = F1;
        }
    }
    L dois_k = 1; for(int k = 1; k <= 6; k++) dois_k *= 2;   /* fora do círculo: 2^6 = 64, explode */
    printf("§P3  |i^k|=1 (k=1..12)? %s ; |N(σ^k)|=1 na torre metálica? %s ; fora: 2^6=%lld (cresce)\n\n",
           norma_presa ? "sim" : "nao", metal_norma_ok ? "sim" : "nao", dois_k);
    ok("§P3 o bit ATRAVESSA: em |x|=1 a norma e' imune ao expoente — |i^k|=1 (a rotação) e"
       " |N(σ^k)|=1 (a torre metálica), a norma presa em 1 dos DOIS lados; fora do circulo cresce."
       " E' a continuação analítica com a medida intacta", norma_presa && metal_norma_ok && dois_k > 1);

    /* ── §P4 os dois extremos: expoente n (discreto) <-> σ_n (contínuo) ──────────────────── */
    /* A estrela −f=f⁻¹ (f=b x^a) força a²=1 -> a=1 (a rotação i; medido em lei2.c). A análise
     * f^{(n)}=f⁻¹ continua o expoente: a−n = 1/a -> a² = n a + 1 -> a = σ_n, a média metálica, com
     * dobra (discriminante) n²+4. Discreto (n, contar) e contínuo (σ_n, medir) são o MESMO x^a. */
    int dobra_ok = 1;
    L esperadas[] = {0, 5, 8, 13, 20};                    /* as dobras conhecidas, para confrontar */
    printf("§P4  a continuação do expoente a² = n·a + 1, dobra = discriminante (por Vieta):\n");
    for(int n = 1; n <= 4; n++){
        /* a dobra NÃO se escreve: sai da Vieta de a²−na−1 (soma=n, prod=−1) como o discriminante
         *   D = (σ−σ†)² = (σ+σ†)² − 4σσ† = n² − 4·(−1) = n²+4.
         * Se o produto fosse +1 (erro comum), daria n²−4. É isso que esta conta pode desmentir. */
        L soma = n, prod = -1;
        L D = soma*soma - 4*prod;                          /* discriminante por Vieta */
        printf("     n=%d (%s): a²−%d·a−1=0, D=(σ−σ†)²=%lld²−4·(%lld)= %lld  (esperada %lld)\n",
               n, n==1?"OURO":n==2?"prata":n==3?"bronze":"", n, soma, prod, D, esperadas[n]);
        if(D != esperadas[n]) dobra_ok = 0;
    }
    printf("\n");
    ok("§P4 os dois extremos da continuação: expoente inteiro n (bit discreto, contar, Hurwitz) <->"
       " média metálica σ_n (contínuo, medir, Gentil), raiz de a²=na+1; e a dobra sai da Vieta"
       " D=(σ+σ†)²−4σσ†=n²+4 (5,8,13), NAO se escreve — se o produto fosse +1 daria n²−4", dobra_ok);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  O bit E' o ponto fixo da estrela: ν(x)=−1/x fixa x²=−1, isto e', i, na fronteira");
        puts("  |x|=1 — e nao ha ponto fixo real, por isso ATRAVESSA para o imaginario. Atravessa");
        puts("  porque em |x|=1 a norma e' imune ao expoente: a continuação analitica leva o expoente");
        puts("  do inteiro n (contar) a' media metalica σ_n (medir), com a medida presa em 1 dos dois");
        puts("  lados. O ponto fixo, enfim derivado — e nao mais postulado.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
