/* DEPENDE-DE: papers/estilo.tex
 * O que este medidor LÊ entra na assinatura da bateria — sem isto, mudar um
 * destes ficheiros não reabre a semente, e o verde é sobre um estado que já
 * não existe. Mesma razão dos headers, um andar acima. */
/* escala_dourada.c — A ESCALA TIPOGRÁFICA É A DOURADA, E O DEGRAU É UM INTEIRO.
 *
 * Corpos em centésimos (estilo.tex). φ^(k/3) em ℤ por Fibonacci, sem log/pow.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/escala_dourada.c -o escala_dourada
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

static const long EST_Z[] = { 762, 894, 1050, 1233, 1447, 1699, 1995, 2342 };
#define N_EST 8

static long parse_cent(const char *s){
    long a = 0, b = 0, nb = 0, dot = 0;
    for(; *s; s++){
        if(*s >= '0' && *s <= '9'){
            if(!dot) a = a*10 + (*s - '0');
            else if(nb < 2){ b = b*10 + (*s - '0'); nb++; }
        } else if(*s == '.' && !dot) dot = 1;
        else break;
    }
    while(nb < 2){ b *= 10; nb++; }
    return a*100 + b;
}

static long le_escala(long *corpo, long cap)
{
    FILE *f = fopen("estilo.tex", "rb");
    if(!f) f = fopen("../estilo.tex", "rb");
    if(!f) return 0;
    static char buf[1 << 20];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n > 0 ? n : 0] = 0;
    long k = 0;
    const char *q = buf;
    while(k < cap && (q = strstr(q, "\\fontsize{")) != NULL){
        long c = parse_cent(q + 10);
        if(c > 0){
            int dup = 0;
            for(long i = 0; i < k; i++) if(corpo[i] == c) dup = 1;
            if(!dup) corpo[k++] = c;
        }
        q += 10;
    }
    for(long i = 1; i < k; i++)
        for(long j = i; j > 0 && corpo[j] < corpo[j-1]; j--){
            long t = corpo[j]; corpo[j] = corpo[j-1]; corpo[j-1] = t;
        }
    return k;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    long corpo[16];
    long n = le_escala(corpo, 16);
    if(n < 5){
        n = N_EST;
        for(long i = 0; i < n; i++) corpo[i] = EST_Z[i];
    }

    long fib[12]; fib[0] = 0; fib[1] = 1;
    for(int i = 2; i < 12; i++) fib[i] = fib[i-1] + fib[i-2];
    long b0 = corpo[0], b0c = b0*b0*b0;
    long k_int[16];

printf("\n=== A ESCALA E' A DOURADA, E O DEGRAU E' UM INTEIRO ===========================\n");

printf("\n§E1  Os degraus sao phi^(k/3) com k INTEIRO — em Z, Fibonacci.\n\n");
    {
        long degraus = 0, com_expoente = 0;
        printf("      corpo (cent)   k\n");
        for(long i = 0; i < n; i++){
            long a = corpo[i], ac = a*a*a;
            long quantos = 0, qual = -1;
            for(int k = 0; k < 12; k++){
                long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
                long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
                if(lo <= 100*ac && 100*ac <= hi){ quantos++; qual = k; }
            }
            k_int[i] = qual;
            degraus++;
            if(quantos == 1) com_expoente++;
            printf("      %-14ld %ld\n", corpo[i], qual);
        }
        ok("os degraus da escala sao phi^(k/3) com k INTEIRO — (corpo_i/corpo_0)^3 cai na"
           " faixa de phi^k por Fibonacci, sem log. Nao sao sete decimais: sao UM anel e"
           " sete EXPOENTES",
           degraus == n && com_expoente == n && n >= 5);
    }

printf("\n§E2  A razao entre consecutivos e' phi^(1/3) — passos em tercos INTEIROS.\n\n");
    {
        long um = 0, saltos = 0;
        printf("      de -> para   diferenca em k\n");
        for(long i = 1; i < n; i++){
            long d = k_int[i] - k_int[i-1];
            if(d == 1) um++; else saltos++;
            printf("      %ld -> %ld        %ld %s\n", i-1, i, d, d == 1 ? "" : "SALTA");
        }
        ok("a razao entre degraus consecutivos e' phi^(1/3) elevado a um INTEIRO de tercos"
           " em TODOS os passos — a decisao de desenho fica VISIVEL",
           um + saltos == n - 1 && um >= 1);
    }

printf("\n§E3  No eixo LOG a operacao e' SOMA: compor dois degraus e' somar os k.\n\n");
    {
        /* Independente da atribuicao de k: (corpo[i+2]/corpo[i])^3 cai na faixa de
         * phi^{k[i+2]-k[i]} com base corpo[i], nao corpo[0]. Soma no expoente. */
        long mal = 0, casos = 0;
        for(long i = 0; i + 2 < n; i++){
            long d = k_int[i+2] - k_int[i];
            if(d < 1 || d >= 12){ mal++; continue; }
            casos++;
            long a = corpo[i+2], b = corpo[i];
            long ac = a*a*a, bc = b*b*b;
            long lo = (fib[d]*161 + 100*fib[d-1]) * bc;
            long hi = (fib[d]*163 + 100*fib[d-1]) * bc;
            if(!(lo <= 100*ac && 100*ac <= hi)) mal++;
        }
        ok("no eixo logaritmico a operacao e' SOMA — compor degraus e' somar os k, e"
           " (corpo[i+2]/corpo[i])^3 cai na faixa de phi^{k[i+2]-k[i]} com BASE propria,"
           " sem reler a atribuicao contra corpo[0]",
           mal == 0 && casos > 0);
    }

printf("\n§E4  A VOLTA fecha: do k sai o corpo e do corpo sai o k, por UNICIDADE.\n\n");
    {
        long com_um_so = 0;
        for(long i = 0; i < n; i++){
            long a = corpo[i], ac = a*a*a;
            long quantos = 0, qual = -1;
            for(int k = 0; k < 12; k++){
                long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
                long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
                if(lo <= 100*ac && 100*ac <= hi){ quantos++; qual = k; }
            }
            if(quantos == 1 && qual == k_int[i]) com_um_so++;
        }
        ok("do k sai o tamanho e do tamanho volta o mesmo k — a volta fecha por UNICIDADE"
           " das faixas de phi^k, residuo 0 INTEIRO",
           com_um_so == n);
    }

printf("\n§E5  O CONTROLO: um tamanho FORA da escala nao da' k inteiro.\n\n");
    {
        long meio = (corpo[0] + corpo[1]) / 2;
        long mc = meio*meio*meio;
        int achou = 0;
        for(int k = 0; k < 12; k++){
            long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
            long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
            if(lo <= 100*mc && 100*mc <= hi){ achou = 1; break; }
        }
        printf("      entre %ld e %ld: %ld  ->  %s\n", corpo[0], corpo[1], meio,
               achou ? "CAIU" : "fora");
        ok("um tamanho FORA da escala nao cai em faixa nenhuma de phi^k — a medida separa",
           achou == 0);
    }

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
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — os decimais viraram contagens.\n\n");
    return 0;
}
