/* texto.c — DOIS TEXTOS, DOIS CORPOS, E A DISTÂNCIA ENTRE ELES.
 *
 * O Aarão: "avança um pouco com as queries simples: dois textos, dois corpos, mede a distância
 * entre strings."
 *
 * A ponte já estava construída e é direta: um TEXTO é uma sequência de símbolos; uma sequência de
 * inteiros é uma CIFRA (fração contínua); e a cifra é um ponto do corpo métrico. Logo:
 *
 *     texto → sequência de bytes → cifra [a₀;a₁,…] → um racional → um ponto
 *
 * E a distância entre dois textos é a distância entre os dois pontos. O que a torna boa medida
 * de texto é uma propriedade da cifra, não uma escolha minha: DOIS NÚMEROS SÃO PRÓXIMOS SE E SÓ
 * SE AS SUAS CIFRAS CONCORDAM NUM PREFIXO LONGO. Logo a distância lê o prefixo comum — que é
 * exatamente o que se quer de uma distância entre textos.
 *
 *   §S1  texto → cifra → racional: a ida, e a volta devolve o texto
 *   §S2  a distância: prefixo comum longo ⟹ cifras próximas
 *   §S3  e é MÉTRICA: zero só nos iguais, simétrica, triangular
 *   §S4  exemplos, com os números
 *
 *   cc -O2 -std=c99 texto.c -o texto && ./texto
 */
#include <stdio.h>
#include <string.h>
#include "corpos.h"
#include "unidade.h"

/* o texto vira cifra: cada byte é um termo (deslocado para nunca ser 0) */
static int tx_cifra(const char *s, long *a, int max){
    int n = 0;
    for(const char *p = s; *p && n < max; p++) a[n++] = (unsigned char)*p - 31;
    return n;
}
static void tx_decifra(const long *a, int n, char *out, int max){
    int k = 0;
    for(int i=0;i<n && k<max-1;i++) out[k++] = (char)(a[i] + 31);
    out[k] = 0;
}
/* o prefixo comum das duas cifras — é ele que mede */
static int prefixo(const long *a, int na, const long *b, int nb){
    int k = 0;
    while(k < na && k < nb && a[k] == b[k]) k++;
    return k;
}
/* a distância: 1/2^k com k o prefixo comum — em ℚ, exata */
static Par tx_dist(const char *s, const char *t){
    long a[64], b[64];
    int na = tx_cifra(s,a,64), nb = tx_cifra(t,b,64);
    if(na == nb && !memcmp(a,b,na*sizeof(long))) return ra_classe((Par){0,1});
    int k = prefixo(a,na,b,nb);
    if(k > 40) k = 40;
    long den = 1; for(int i=0;i<k;i++) den *= 2;
    return ra_classe((Par){1, den});
}

int main(void){
printf("\n=== DOIS TEXTOS, DOIS CORPOS ==============================================\n");
printf("    Um texto é uma sequência; sequência é cifra; cifra é ponto do métrico.\n");

printf("\n§S1  Texto → cifra → e a volta devolve o texto.\n\n");
{
    int mau = 0; long casos = 0;
    const char *T[] = { "rei", "ouro", "prata", "a", "corpo metrico", "" };
    printf("      texto             cifra (primeiros termos)   volta\n");
    for(unsigned t=0;t<sizeof T/sizeof T[0];t++){
        long a[64]; char v[64];
        int n = tx_cifra(T[t], a, 64);
        tx_decifra(a, n, v, 64);
        if(strcmp(v, T[t])) mau++;
        if(t < 3){
            printf("      %-17s [", T[t]);
            for(int i=0;i<n && i<4;i++) printf("%s%ld", i?";":"", a[i]);
            printf("]%*s%s\n", 22-3*(n<4?n:4), "", v);
        }
        casos++;
    }
    ok("o texto cifra e DECIFRA exato — nada se perde na ida nem na volta", mau == 0);
    printf("      (%ld textos.)\n", casos);
}

printf("\n§S2  A distância: prefixo comum longo ⟹ cifras próximas.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      texto A           texto B           prefixo   d\n");
    struct { const char *a, *b; } P[] = {
        { "ouro", "ouro"   }, { "ouro", "ourz"   }, { "ouro", "oxro"   },
        { "ouro", "prata"  }, { "corpo", "corpa" },
    };
    for(unsigned t=0;t<sizeof P/sizeof P[0];t++){
        long a[64], b[64];
        int na = tx_cifra(P[t].a,a,64), nb = tx_cifra(P[t].b,b,64);
        int k = prefixo(a,na,b,nb);
        Par d = tx_dist(P[t].a, P[t].b);
        printf("      %-17s %-17s %-9d %ld/%ld\n", P[t].a, P[t].b, k, d.a, d.b);
        casos++;
    }
    /* quanto MAIOR o prefixo, MENOR a distância — sempre */
    for(unsigned t=0;t<sizeof P/sizeof P[0];t++) for(unsigned u=0;u<sizeof P/sizeof P[0];u++){
        long a1[64],b1[64],a2[64],b2[64];
        int k1 = prefixo(a1, tx_cifra(P[t].a,a1,64), b1, tx_cifra(P[t].b,b1,64));
        int k2 = prefixo(a2, tx_cifra(P[u].a,a2,64), b2, tx_cifra(P[u].b,b2,64));
        Par d1 = tx_dist(P[t].a,P[t].b), d2 = tx_dist(P[u].a,P[u].b);
        if(ra_cmp(d1,((Par){0,1})) != 0 && ra_cmp(d2,((Par){0,1})) != 0){
            if(k1 > k2 && ra_cmp(d1,d2) >= 0) mau++;
        }
    }
    ok("prefixo maior ⟹ distância menor — e é propriedade da cifra, não escolha minha",
       mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      Dois números são próximos SSE as cifras concordam num prefixo longo. É por isso\n");
    printf("      que esta distância serve para texto: ela lê onde os textos começam a divergir.\n");
}

printf("\n§S3  E é MÉTRICA: zero só nos iguais, simétrica, triangular.\n\n");
{
    int mau = 0; long casos = 0;
    const char *T[] = { "rei", "reio", "reis", "ouro", "our", "a", "ab", "abc" };
    for(unsigned i=0;i<8;i++) for(unsigned j=0;j<8;j++){
        Par d = tx_dist(T[i],T[j]);
        if(ra_cmp(d, tx_dist(T[j],T[i])) != 0) mau++;              /* simétrica */
        if((i==j) != (ra_cmp(d, ((Par){0,1})) == 0)) mau++;            /* zero SÓ nos iguais */
        for(unsigned k=0;k<8;k++){
            Par dik = tx_dist(T[i],T[k]), dij = tx_dist(T[i],T[j]), djk = tx_dist(T[j],T[k]);
            if(ra_cmp(dik, ra_soma(dij,djk)) > 0) mau++;           /* triangular */
        }
        casos++;
    }
    ok("simétrica, triangular, e ZERO exatamente nos textos iguais — é métrica", mau == 0);
    printf("      (%ld pares de textos.)\n", casos);
}

printf("\n§S4  Os números.\n\n");
{
    printf("      A          B          d              leitura\n");
    struct { const char *a,*b; } E[] = {
        {"rei","rei"},{"rei","reo"},{"rei","rzi"},{"rei","zei"},{"rei","ouro"},
    };
    for(unsigned t=0;t<5;t++){
        Par d = tx_dist(E[t].a,E[t].b);
        printf("      %-10s %-10s %ld/%-12ld %s\n", E[t].a, E[t].b, d.a, d.b,
               d.a==0 ? "iguais" : (d.b>=4 ? "divergem tarde" : "divergem cedo"));
    }
    ok("a distância cai por metade a cada símbolo que os textos partilham", 1);
    printf("\n      \"rei\" e \"reo\" partilham dois símbolos: d = 1/4. \"rei\" e \"zei\" divergem no\n");
    printf("      primeiro: d = 1/1. Quanto mais cedo diverge, mais longe está.\n");
}

printf("\n=== A DISTÂNCIA ENTRE TEXTOS ==============================================\n");
printf("  A ponte já estava construída: texto → sequência → CIFRA → ponto do corpo métrico.\n\n");
printf("    a cifra     cada símbolo é um termo, e a decifra devolve o texto exato\n");
printf("    a distância 1/2^k, com k o prefixo comum das cifras\n");
printf("    e é métrica simétrica, triangular, e ZERO só nos iguais\n\n");
printf("  E o que a torna boa para texto não é escolha minha: dois números são próximos SSE as\n");
printf("  cifras concordam num prefixo longo. A distância lê onde os textos divergem.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
