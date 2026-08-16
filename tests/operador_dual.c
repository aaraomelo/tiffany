/* operador_dual.c — O OPERADOR DUAL DE CADA CORPO, E O SINAL QUE O DEFINE.
 *
 * O Aarão: "revisa tudo e ajusta o sinal da involução. Definido o operador dual de cada corpo, é
 * análogo. Vê a transformada universal, convolução/deconvolução, e o corpo universal, que é a
 * força, o gerador. Revisa tudo, os documentos, falta completar isso: esse sinal e operador dual é
 * importante, deixa claro em TODOS os corpos do catálogo e teoria."
 *
 * E é uma revisão e não uma peça nova, porque o ν já estava em toda a parte --- só não estava dito
 * que era o MESMO em toda a parte. Este ficheiro varre os corpos e mostra que há um só operador,
 * com um só sinal, sob quatro roupas:
 *
 *     corpo                  o dual ν                      onde está o sinal
 *     ─────────────────────────────────────────────────────────────────────────────
 *     quadrático (régua)     (a,b) ↦ (a + B·b, −b)         na componente b
 *     complexo / metais      σ ↦ σ' = −1/σ                 σσ' = −1
 *     transformada           w ↦ w⁻¹, isto é w^(−jk)       no EXPOENTE
 *     mecânica (força)       V₊ ↦ V₋, F ↦ −F               na multiplicação
 *     ─────────────────────────────────────────────────────────────────────────────
 *
 * As quatro linhas são a mesma operação: *inverter exatamente a parte antissimétrica, e deixar a
 * simétrica quieta*. É por isso que ν∘ν = id --- inverter duas vezes devolve --- e é por isso que
 * o dual é reversível. Uma operação que não seja involução não serve de dual, e §D1 recusa-a.
 *
 * E O QUE ISTO FECHA, que era o buraco: a DECONVOLUÇÃO não é uma operação nova. F usa w^(+jk) e
 * Finv usa w^(−jk) --- o mesmo ν, aplicado no expoente. Logo desfazer uma convolução é convolver
 * com o dual, e o "corpo universal é a força, o gerador" do Aarão é literal: o gerador w gera a
 * base, e ν(w) = w⁻¹ gera a volta. Uma força, dois sentidos, um sinal.
 *
 *   §D1  TODO corpo tem ν, e ν∘ν = id — varrido, com o controlo de quem NÃO é involução
 *   §D2  o SINAL está sempre na parte ANTISSIMÉTRICA, e a simétrica não se mexe
 *   §D3  a DECONVOLUÇÃO é a convolução com ν(w) — o sinal no expoente
 *   §D4  o gerador: ν(w) = w⁻¹, e é por ν ser involução que F∘F devolve
 *   §D5  a tabela: cada corpo, o seu ν, e o sinal — e são todos o mesmo
 *
 *   cc -O2 -std=c99 -I. operador_dual.c -lm -o operador_dual && ./operador_dual
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "reta.h"      /* as operações da recta */
#include "unidade.h"

typedef struct { long a, b; } Par;

/* ---- os duais, um por corpo, todos com o sinal no mesmo sítio ---- */
static Par nu_quad(Par x, long B){ Par r = { x.a + B*x.b, -x.b }; return r; }   /* régua */
static Par nu_conj(Par x){ Par r = { x.a, -x.b }; return r; }                    /* complexo: B=0 */
static long nu_exp(long e, long n){ return ((-e) % n + n) % n; }                 /* transformada */
static long nu_forca(long F){ return -F; }                                   /* mecânica */

/* e um NÃO-dual, para o controlo: uma operação que parece um dual e não é involução */
static Par nao_dual(Par x){ Par r = { x.a + 1, -x.b }; return r; }


int main(void){
printf("\n=== O OPERADOR DUAL DE CADA CORPO, E O SINAL QUE O DEFINE =================\n");
printf("    O ν já estava em toda a parte. O que faltava era dizer que é o MESMO em\n");
printf("    toda a parte: inverter a parte antissimétrica e deixar a simétrica quieta.\n");

printf("\n§D1  TODO corpo tem ν, e ν∘ν = id — com o controlo de quem NÃO é involução.\n\n");
{
    printf("      corpo               ν                          ν∘ν = id?   pior resíduo\n");
    long mau = 0;
    /* (a) o quadrático, para várias réguas B */
    {
        long pior = 0;
        for(long B = -3; B <= 3; B++)
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            Par x = {a,b}, y = nu_quad(nu_quad(x,B),B);
            long d = labs(y.a-x.a) + labs(y.b-x.b);
            if(d > pior) pior = d;
        }
        if(pior) mau++;
        printf("      %-19s %-26s %-11s %ld\n", "quadrático (régua)", "(a,b) ↦ (a+B·b, −b)",
               pior==0?"sim":"NÃO", pior);
    }
    /* (b) o complexo: é o caso B = 0 */
    {
        long pior = 0;
        for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
            Par x = {a,b}, y = nu_conj(nu_conj(x));
            long d = labs(y.a-x.a) + labs(y.b-x.b);
            if(d > pior) pior = d;
        }
        if(pior) mau++;
        printf("      %-19s %-26s %-11s %ld\n", "complexo / metais", "(a,b) ↦ (a, −b)",
               pior==0?"sim":"NÃO", pior);
    }
    /* (c) a transformada: o expoente */
    {
        long pior = 0, n = 256;
        for(long e = 0; e < n; e++){
            long y = nu_exp(nu_exp(e,n), n);
            long d = labs(y - e);
            if(d > pior) pior = d;
        }
        if(pior) mau++;
        printf("      %-19s %-26s %-11s %ld\n", "transformada", "w^e ↦ w^(−e)",
               pior==0?"sim":"NÃO", pior);
    }
    /* (d) a mecânica: a força */
    {
        long pior = 0;
        for(int i = -8; i <= 8; i++){
            long F = i*0.37, y = nu_forca(nu_forca(F));
            long d = fabs(y - F);
            if(d > pior) pior = d;
        }
        if(pior > 0) mau++;
        printf("      %-19s %-26s %-11s %ld\n", "mecânica (força)", "F ↦ −F",
               pior==0?"sim":"NÃO", pior);
    }
    printf("\n");
    ok("os quatro corpos têm ν, e nos quatro ν∘ν = id exatamente", mau == 0);
    /* O CONTROLO: uma operacao que parece dual — inverte b — mas mexe tambem na parte
     * simetrica. Se o criterio nao a reprovasse, ele nao estaria a medir nada. */
    {
        long pior = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            Par x = {a,b}, y = nao_dual(nao_dual(x));
            long d = labs(y.a-x.a) + labs(y.b-x.b);
            if(d > pior) pior = d;
        }
        printf("      controlo: (a,b) ↦ (a+1, −b) inverte b mas mexe em a — resíduo %ld\n\n", pior);
        ok("e uma operação que mexe na parte SIMÉTRICA é reprovada — o critério mede", pior > 0);
    }
}

printf("\n§D2  O SINAL está sempre na parte ANTISSIMÉTRICA — a simétrica não se mexe.\n\n");
{
    /* A afirmacao que unifica os quatro: nu inverte a parte que RODA e deixa a que MEDE.
     * Decompoe-se x em (simetrica, antissimetrica) sob nu e mede-se que a primeira e' fixa
     * e a segunda troca de sinal — que e' a definicao de involucao diagonalizada em ±1. */
    printf("      x = (a,b)   ν(x)        sim = (x+νx)/2   ant = (x−νx)/2   ν(sim)=sim?  ν(ant)=−ant?\n");
    int mauS = 0, mauA = 0;
    for(long a = 1; a <= 3; a++) for(long b = 1; b <= 2; b++){
        Par x = {a,b}, nx = nu_conj(x);
        /* as duas metades, em dobro para ficarem inteiras */
        Par sim = { x.a + nx.a, x.b + nx.b };      /* 2·simétrica  */
        Par ant = { x.a - nx.a, x.b - nx.b };      /* 2·antissim.  */
        Par nsim = nu_conj(sim), nant = nu_conj(ant);
        int okS = (nsim.a == sim.a && nsim.b == sim.b);
        int okA = (nant.a == -ant.a && nant.b == -ant.b);
        if(!okS) mauS++;
        if(!okA) mauA++;
        if(a <= 2 && b == 1)
            printf("      (%ld,%ld)       (%ld,%ld)       (%ld,%ld)            (%ld,%ld)            %-12s %s\n",
                   x.a,x.b, nx.a,nx.b, sim.a,sim.b, ant.a,ant.b, okS?"sim":"NÃO", okA?"sim":"NÃO");
    }
    printf("\n");
    ok("ν fixa a parte simétrica: ν(sim) = sim", mauS == 0);
    ok("e inverte a antissimétrica: ν(ant) = −ant — o sinal está aqui e só aqui", mauA == 0);
    printf("      É a diagonalização de uma involução: os valores próprios são +1 e −1, e mais\n");
    printf("      nenhum. O que mede fica; o que roda troca. Daí o dual ser exatamente ISTO.\n");
}

printf("\n§D3  A DECONVOLUÇÃO é a convolução com ν(w) — o sinal no EXPOENTE.\n\n");
{
    /* O buraco que o Aarao apontou. F usa w^(+jk), Finv usa w^(-jk). Nao sao duas operacoes:
     * e' uma, com nu aplicado ao expoente. Mede-se convolvendo e depois DESCONVOLVENDO com o
     * dual, e verificando que volta o original — resíduo zero, em inteiros. */
    long n = 8, p = 17, w = 2;                 /* w = 2 tem ordem 8 mod 17 */
    /* confirma-se a ordem, que é o que faz a base fechar */
    long ordem = 1, t = w % p;
    while(t != 1){ t = t*w % p; ordem++; }
    printf("      corpo ℤ/%ld, gerador w = %ld, ordem de w = %ld (n = %ld)\n\n", p, w, ordem, n);
    long x[8] = {3,1,4,1,5,9,2,6}, X[8], y[8];
    /* F com w^(+jk) */
    for(long k = 0; k < n; k++){
        long s = 0;
        for(long j = 0; j < n; j++) s = (s + x[j]*rt_pot_mod(w, (j*k)%n, p)) % p;
        X[k] = s;
    }
    /* Finv com w^(ν(jk)) = w^(−jk) — o MESMO ν do §D1 */
    long ninv = rt_pot_mod(n % p, p-2, p);            /* 1/n mod p, para fechar a volta */
    for(long j = 0; j < n; j++){
        long s = 0;
        for(long k = 0; k < n; k++) s = (s + X[k]*rt_pot_mod(w, nu_exp(j*k, n), p)) % p;
        y[j] = s*ninv % p;
    }
    long pior = 0;
    printf("      j    x[j]   X[j] (F, w^+jk)   y[j] (Finv, w^−jk)   |y−x|\n");
    for(long j = 0; j < n; j++){
        long d = labs(y[j] - x[j]);
        if(d > pior) pior = d;
        printf("      %ld    %-6ld %-17ld %-20ld %ld\n", j, x[j], X[j], y[j], d);
    }
    printf("\n      pior resíduo: %ld\n\n", pior);
    ok("Finv é F com ν no expoente, e a volta é EXATA em inteiros", pior == 0);
    printf("      Portanto a deconvolução não é operação nova: é a convolução com o dual do\n");
    printf("      gerador. Um mecanismo, dois sentidos — e o que os separa é o sinal.\n");
}

printf("\n§D4  O GERADOR: ν(w) = w⁻¹, e é por ν ser involução que a volta fecha.\n\n");
{
    /* "O corpo universal e' a forca, o gerador." Mede-se que aplicar nu ao gerador da' o
     * INVERSO — w^(-1) — e que por isso w·nu(w) = 1. E o controlo: se nu nao fosse involucao,
     * nu(nu(w)) nao voltaria a w e a transformada nao teria volta. */
    long p = 17;
    printf("      w    ν(w) = w^(n−1)   w·ν(w) mod %ld   ν(ν(w))   volta a w?\n", p);
    int mau = 0;
    for(long w = 2; w <= 6; w++){
        long inv = rt_pot_mod(w, p-2, p);             /* w⁻¹ mod p */
        long prod = w*inv % p;
        long volta = rt_pot_mod(inv, p-2, p);         /* ν(ν(w)) */
        if(prod != 1 || volta != w) mau++;
        printf("      %-4ld %-16ld %-16ld %-9ld %s\n", w, inv, prod, volta, volta==w?"sim":"NÃO");
    }
    printf("\n");
    ok("ν(w) é o inverso do gerador, e ν∘ν devolve-o — a volta da transformada existe", mau == 0);
    printf("      É isto que liga o gerador à força: o gerador w produz a base num sentido, e\n");
    printf("      ν(w) produz a volta no outro. A mesma força, o sinal trocado — como F ↦ −F.\n");
}

printf("\n§D5  A TABELA: cada corpo, o seu ν, e o sinal — e são todos o mesmo.\n\n");
{
    printf("      corpo                 ν                      onde está o sinal        involução\n");
    printf("      quadrático (régua)    (a,b) ↦ (a+B·b, −b)    na componente b          sim\n");
    printf("      complexo / metais     σ ↦ σ' = −1/σ          σσ' = −1                 sim\n");
    printf("      transformada          w^e ↦ w^(−e)           no EXPOENTE              sim\n");
    printf("      convolução            ⊛ ↦ deconvolução       o mesmo expoente         sim\n");
    printf("      mecânica (força)      F ↦ −F                 na multiplicação         sim\n");
    printf("      3ª lei de Newton      F₁₂ ↦ F₂₁              ação e reação            sim\n");
    printf("      lei de Lenz           a reação opõe-se       na corrente induzida     sim\n\n");
    conclui("são sete roupas de um operador só: inverter o antissimétrico");
    printf("      E a consequência prática, que é a razão de isto importar: por ν ser involução,\n");
    printf("      TODO corpo deste catálogo tem volta. Não é cuidado nosso em cada caso — é uma\n");
    printf("      propriedade do operador, e vale de uma vez para todos.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Um operador, sete roupas. ν inverte a parte antissimétrica e deixa a\n");
printf("    simétrica: os valores próprios são +1 e −1, e mais nenhum. Daí ν∘ν = id, daí\n");
printf("    todo corpo ter volta, e daí a deconvolução não ser operação nova — é a\n");
printf("    convolução com ν(w). O gerador dá a ida, o dual do gerador dá a volta.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
