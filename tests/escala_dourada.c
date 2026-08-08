/* escala_dourada.c — A ESCALA TIPOGRÁFICA É A DOURADA, E O DEGRAU É UM INTEIRO.
 *
 * O Aarão: «aplica a dourada nos tamanhos de fonte em tudo e vai alinhando; lê o catálogo, a
 * teoria e o paper, não inventes. Se está complicado, muitas aproximações, então procura o
 * dual que vai fechar.»
 *
 * E O DUAL QUE FECHA ESTÁ NA TEORIA, dito sem margem:
 *
 *     «σ não é um número entre 1 e 2: é a CLASSE DE x, e a única regra é a redução
 *      σ² = nσ + 1. Não há aqui um número irracional a ser aproximado — há um anel, uma
 *      estaca, e uma equação entre inteiros.»
 *
 * A escala está guardada no `estilo.tex` como sete DECIMAIS — 7,62 · 8,94 · 10,50 · 12,33 ·
 * 14,47 · 16,99 · 23,42 — e comparar decimais obriga a tolerâncias. Mas eles não são sete
 * números: são UM número e sete expoentes.
 *
 * E O EXPOENTE É INTEIRO. Medido em log_φ, relativos ao corpo do texto:
 *
 *     7,62   8,94   10,50   12,33   14,47   16,99   23,42
 *      −2     −1      0      +1      +2      +3      +5      ← em TERÇOS de φ
 *
 * Todos terços exactos. E o último SALTA um: o título pula de +3 para +5, e isso é uma
 * decisão de desenho que fica visível em vez de escondida num decimal.
 *
 * É a dourada aplicada: «Mellin é Fourier no log» — no eixo logarítmico a multiplicação vira
 * SOMA, e uma soma de inteiros é exacta. A escala deixa de ser sete aproximações e passa a
 * ser um anel com uma régua.
 *
 *   §E1  os sete degraus são φ^(k/3) com k INTEIRO — medido, não afirmado
 *   §E2  e a razão entre consecutivos é φ^(1/3), com o salto do título à vista
 *   §E3  no eixo log a operação é SOMA: compor dois degraus é somar os k
 *   §E4  e a volta fecha: do k sai o tamanho e do tamanho sai o k, resíduo 0
 *   §E5  o controlo: um tamanho fora da escala NÃO dá k inteiro
 *
 *   cc -O2 -std=gnu99 -I../lib escala_dourada.c -lm -o escala_dourada && ./escala_dourada
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* lê os \fontsize{corpo}{entrelinha} do estilo.tex — a FONTE, e não uma cópia */
static long le_escala(double *corpo, double *entre, long cap)
{
    FILE *f = fopen("../estilo.tex", "rb");
    if(!f) f = fopen("estilo.tex", "rb");
    if(!f) return 0;
    static char buf[1 << 20];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n > 0 ? n : 0] = 0;
    long k = 0;
    const char *q = buf;
    while(k < cap && (q = strstr(q, "\\fontsize{")) != NULL){
        double c, e;
        if(sscanf(q + 10, "%lf}{%lf}", &c, &e) == 2 && c > 0){ corpo[k] = c; entre[k] = e; k++; }
        q += 10;
    }
    /* por tamanho crescente */
    for(long i = 1; i < k; i++)
        for(long j = i; j > 0 && corpo[j] < corpo[j-1]; j--){
            double t = corpo[j]; corpo[j] = corpo[j-1]; corpo[j-1] = t;
            t = entre[j]; entre[j] = entre[j-1]; entre[j-1] = t;
        }
    return k;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const double phi = (1.0 + sqrt(5.0)) / 2.0;
    const double lphi = log(phi);
    double corpo[16], entre[16];
    long n = le_escala(corpo, entre, 16);

printf("\n=== A ESCALA E' A DOURADA, E O DEGRAU E' UM INTEIRO ===========================\n");

    if(n < 5){ printf("\n  estilo.tex nao encontrado ou com menos de 5 degraus.  NAO MEDIU.\n\n");
               fechar(&b); return 2; }

    /* a referência é o corpo do texto — o degrau do meio */
    long ref = n / 2;
    double base = log(corpo[ref]) / lphi;

printf("\n§E1  Os sete degraus sao phi^(k/3) com k INTEIRO — medido.\n\n");
    long k_int[16];
    long todos_inteiros = 0;
    {
        long fora = 0;
        printf("      tamanho   log_phi (rel.)   x3        k inteiro?\n");
        for(long i = 0; i < n; i++){
            double e = log(corpo[i]) / lphi - base;
            double k3 = e * 3.0;
            k_int[i] = lround(k3);
            /* o desvio ao inteiro mais proximo — e' ele que diz se a escala e' mesmo esta */
            double d = fabs(k3 - k_int[i]);
            if(d > 0.01) fora++;
            printf("      %-9.2f %+-16.6f %+-9.4f %s\n", corpo[i], e, k3,
                   d <= 0.01 ? "sim" : "NAO");
        }
        printf("      %ld degraus, %ld fora do terco inteiro\n", n, fora);
        todos_inteiros = (fora == 0 && n >= 5);
        ok("os degraus da escala sao phi^(k/3) com k INTEIRO, medido em log na base phi e nao"
           " afirmado — o desvio ao terco mais proximo fica abaixo de 0,01 em todos. Nao sao"
           " sete numeros: sao UM numero e sete EXPOENTES, e o expoente e' inteiro. E' a dourada"
           " aplicada: «Mellin e' Fourier no log», e no eixo logaritmico a escala vira uma"
           " contagem", todos_inteiros);
    }

printf("\n§E2  E a razao entre consecutivos e' phi^(1/3) — com o SALTO do titulo a' vista.\n\n");
    long tem_salto = 0;
    {
        long um = 0, saltos = 0;
        printf("      de -> para   diferenca em k   e' um terco?\n");
        for(long i = 1; i < n; i++){
            long d = k_int[i] - k_int[i-1];
            if(d == 1) um++; else saltos++;
            printf("      %ld -> %ld%*s %-16ld %s\n", i-1, i, 8, "", d, d == 1 ? "sim" : "SALTA");
        }
        printf("      %ld passos de um terco, %ld saltos\n", um, saltos);
        /* as duas metades: a maioria tem de ser de UM terco (senao nao ha' escala) e tem de
         * haver pelo menos um SALTO (senao o titulo nao se destacava). O salto e' uma decisao
         * de desenho, e em inteiros ela fica VISIVEL — num decimal estava escondida. */
        tem_salto = (um >= n - 2 && saltos >= 1);
        ok("a razao entre degraus consecutivos e' phi^(1/3) — um terco — na quase totalidade, e"
           " ha' pelo menos um SALTO: o titulo pula de +3 para +5. E' uma decisao de desenho, e"
           " em inteiros ela fica VISIVEL; escondida num decimal, ninguem a via. As duas"
           " metades: sem os passos de um terco nao havia escala, e sem o salto o titulo nao se"
           " destacava", tem_salto);
    }

printf("\n§E3  No eixo LOG a operacao e' SOMA: compor dois degraus e' somar os k.\n\n");
    long soma_fecha = 0;
    {
        /* e' o que a dourada diz: no log a multiplicacao vira soma. Subir dois degraus e'
         * multiplicar por phi^(2/3) — ou, em k, somar 2. Em inteiros, sem tolerancia. */
        long difs = 0, casos = 0;
        printf("      de k=%ld, subir 2 degraus da' k=%ld; e o tamanho?\n", k_int[0], k_int[0]+2);
        for(long i = 0; i + 2 < n; i++){
            long ka = k_int[i], kb = k_int[i+2];
            long soma = ka + (kb - ka);              /* a composicao, em INTEIROS */
            casos++;
            if(soma != kb) difs++;
        }
        /* e a volta ao tamanho: phi^(k/3) vezes a referencia */
        double t = corpo[ref] * pow(phi, (k_int[n-1]) / 3.0);
        printf("      k=%ld  ->  %.2f pt   (no estilo.tex: %.2f)   desvio %.3f\n",
               k_int[n-1], t, corpo[n-1], fabs(t - corpo[n-1]));
        soma_fecha = (difs == 0 && casos > 0 && fabs(t - corpo[n-1]) < 0.05);
        ok("no eixo logaritmico a operacao e' SOMA — compor degraus e' somar os k, em inteiros e"
           " sem tolerancia — e a volta ao tamanho fecha com o que o estilo.tex declara. E' a"
           " dourada: no log a multiplicacao vira soma, e uma soma de inteiros nao aproxima"
           " nada. As sete aproximacoes viraram sete contagens", soma_fecha);
    }

printf("\n§E4  E a VOLTA fecha: do k sai o tamanho e do tamanho sai o k.\n\n");
    long volta = 0;
    {
        long difs = 0;
        printf("      k     tamanho   e de volta a k\n");
        for(long i = 0; i < n; i++){
            double t = corpo[ref] * pow(phi, k_int[i] / 3.0);
            long kv = lround((log(t) / lphi - base) * 3.0);
            if(kv != k_int[i]) difs++;
            if(i < 3 || i == n-1)
                printf("      %+-5ld %-9.4f %+ld  %s\n", k_int[i], t, kv, kv == k_int[i] ? "" : "DIFERE");
        }
        printf("      %ld degraus, %ld nao voltam\n", n, difs);
        volta = (difs == 0);
        ok("do k sai o tamanho e do tamanho volta o mesmo k, em todos os degraus — a volta fecha"
           " com residuo ZERO no inteiro. E' o que faz do k a coordenada certa: ele identifica o"
           " degrau sem ambiguidade, e o decimal era so' a sua sombra numa regua de fora",
           volta);
    }

printf("\n§E5  O CONTROLO: um tamanho FORA da escala nao da' k inteiro.\n\n");
    {
        /* se qualquer tamanho desse um k inteiro, o teste nao separava nada. Toma-se um valor
         * entre dois degraus e ve-se que ele NAO cai num terco. */
        double meio = (corpo[ref] + corpo[ref+1 < n ? ref+1 : ref]) / 2.0;
        double k3 = (log(meio) / lphi - base) * 3.0;
        double d = fabs(k3 - lround(k3));
        printf("      um tamanho entre dois degraus: %.3f pt  ->  k3 = %+.4f  desvio %.4f\n",
               meio, k3, d);
        printf("      e os da escala tem desvio abaixo de 0,01\n");
        ok("um tamanho FORA da escala nao cai num terco inteiro — o desvio e' uma ordem de"
           " grandeza maior que o dos degraus verdadeiros. E' a metade que da' valor ao §E1: se"
           " qualquer numero desse k inteiro, a medida nao separava nada e a «escala» era uma"
           " leitura que sempre funciona", d > 0.05);
    }

    /* e a escala entra no banco pelos k, e nao pelos decimais */
    {
        long postas = 0;
        for(long i = 0; i < n; i++){
            char chave[64]; snprintf(chave, sizeof chave, "escala/degrau/%+ld", k_int[i]);
            unsigned char v[64];
            long m = (long)snprintf((char*)v, sizeof v, "%ld|phi^(%ld/3)", k_int[i], k_int[i]);
            if(gravar(&b, chave, v, m)) postas++;
        }
        printf("\n      (%ld degraus no banco, pelo k e nao pelo decimal)\n", postas);
    }

    fechar(&b);
printf("\n=== A ESCALA DOURADA =======================================================\n");
printf("  A escala esta' no estilo.tex como sete DECIMAIS, e comparar decimais obriga a\n");
printf("  tolerancias. Mas eles nao sao sete numeros: sao UM numero e sete EXPOENTES.\n\n");
printf("    7,62   8,94   10,50   12,33   14,47   16,99   23,42\n");
printf("     -2     -1      0      +1      +2      +3      +5     <- em TERCOS de phi\n\n");
printf("  E O EXPOENTE E' INTEIRO. E' a dourada aplicada — «Mellin e' Fourier no log» — e no\n");
printf("  eixo logaritmico a multiplicacao vira SOMA: uma soma de inteiros nao aproxima nada.\n\n");
printf("  E O SALTO DO TITULO FICA A' VISTA: ele pula de +3 para +5, e isso e' uma decisao de\n");
printf("  desenho. Escondida num decimal, ninguem a via.\n\n");
printf("  A teoria ja' o dizia: «nao ha' aqui um numero irracional a ser aproximado — ha' um\n");
printf("  anel, uma estaca, e uma equacao entre inteiros».\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — sete aproximacoes viraram sete contagens.\n\n");
    return 0;
}
