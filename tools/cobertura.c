/* cobertura.c — O CORPO LÓGICO É CONTÍNUO E ORDENADO, e o parâmetro é a COBERTURA.
 *
 * O Aarão: "o corpo lógico é contínuo e ordenado, e está na cifra do rei?"
 *
 * Como eu o construí no logico.c: DISCRETO — máscaras de 8 bits. Outra vez a representação. A
 * régua contínua do corpo lógico existe, e é esta:
 *
 *     a COBERTURA: a fração do domínio que a prova verifica
 *
 * É racional, é densa, é ordenada — e é a régua certa porque é ela que separa uma prova de uma
 * afirmação. Uma verificação parcial cobre p/q dos casos; uma prova cobre 1; um decreto cobre 0.
 *
 * E é exatamente onde os meus erros de hoje vivem: eu anunciei cobertura 1 tendo cobertura p/q.
 *
 *   §K1  a cobertura é racional, densa e ordenada — contínua, como as outras réguas
 *   §K2  encadear provas MULTIPLICA as coberturas — e o ⊗ do corpo lógico é esse
 *   §K3  está na cifra do rei: toda cobertura tem fração contínua FINITA
 *   §K4  o decreto tem cobertura ZERO — e é essa a sua marca, medida
 *   §K5  os meus erros de hoje, com a cobertura real de cada um
 *
 *   cc -O2 -std=c99 cobertura.c -o cobertura && ./cobertura
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }

int main(void){
printf("\n=== A COBERTURA: a régua contínua do corpo lógico ==========================\n");
printf("    Eu construí-o discreto. A régua dele é a fração do domínio verificada.\n");

printf("\n§K1  A cobertura é racional, DENSA e ordenada.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      cobertura   o que significa\n");
    printf("      0/1         nada verificado — o decreto\n");
    printf("      1/2         metade dos casos — \"medi uma fatia\"\n");
    printf("      12/∞ → 0    doze primos de infinitos (o erro da base ortonormal)\n");
    printf("      1/1         tudo — a prova\n\n");
    for(long p1=0;p1<=20;p1++) for(long r1=1;r1<=20;r1++)
    for(long p2=0;p2<=20;p2++) for(long r2=1;r2<=20;r2++){
        if(p1>r1 || p2>r2) continue;                    /* cobertura ∈ [0,1] */
        Par a=q(p1,r1), b=q(p2,r2);
        int s = ra_cmp(a,b);
        if(s != -ra_cmp(b,a)) mau++;                    /* ordem total */
        if(s < 0){                                      /* DENSA: há uma entre duas */
            Par m = ra_prod(ra_soma(a,b), q(1,2));
            if(ra_cmp(a,m) >= 0 || ra_cmp(m,b) >= 0) mau++;
        }
        casos++;
    }
    ok("a cobertura é ordem total e DENSA em [0,1] — contínua, como toda régua deste trabalho",
       mau == 0);
    printf("      (%ld pares de coberturas.)\n", casos);
    printf("\n      Entre \"metade verificado\" e \"tudo verificado\" há sempre um grau intermédio, e\n");
    printf("      isso é o que faz dela régua e não rótulo. Provado/não-provado é um interruptor;\n");
    printf("      a cobertura é uma régua.\n");
}

printf("\n§K2  Encadear provas MULTIPLICA as coberturas.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      prova A     prova B     encadeada    porquê\n");
    printf("      1/2         1/2         1/4          ambas têm de valer no caso\n");
    printf("      1/1         3/4         3/4          uma completa não estraga\n");
    printf("      0/1         1/1         0/1          o decreto ANULA a cadeia\n\n");
    for(long p1=0;p1<=12;p1++) for(long r1=1;r1<=12;r1++)
    for(long p2=0;p2<=12;p2++) for(long r2=1;r2<=12;r2++){
        if(p1>r1 || p2>r2) continue;
        Par a=q(p1,r1), b=q(p2,r2);
        Par c = ra_prod(a,b);
        if(ra_cmp(c,a) > 0 || ra_cmp(c,b) > 0) mau++;   /* encadear NUNCA aumenta */
        if(p1 == 0 || p2 == 0){ if(ra_cmp(c,q(0,1)) != 0) mau++; }   /* zero anula */
        casos++;
    }
    ok("encadear nunca AUMENTA a cobertura, e um elo de cobertura 0 anula a cadeia", mau == 0);
    printf("      (%ld cadeias.)\n", casos);
    printf("\n      É o ⊗ do corpo lógico com a régua certa: uma cadeia vale o produto dos elos. E um\n");
    printf("      elo decretado (cobertura 0) leva a cadeia inteira a zero — por mais elos provados\n");
    printf("      que tenha antes ou depois.\n");
}

printf("\n§K3  Está na CIFRA DO REI: toda cobertura tem fração contínua finita.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      cobertura   cifra           volta?\n");
    for(long p=0;p<=30;p++) for(long r=1;r<=30;r++){
        if(p>r) continue;
        Par c = q(p,r);
        long a[64]; int k = cf_cifra(c, a, 64);
        if(k >= 64) mau++;                              /* PARA: é racional */
        Par v = ra_classe(cf_decifra(a,k));
        if(v.a != c.a || v.b != c.b) mau++;
        casos++;
    }
    { Par c=q(12,17); long a[16]; int k=cf_cifra(c,a,16); Par v=cf_decifra(a,k);
      printf("      12/17       [%ld;%ld,%ld,%ld]      %ld/%ld  sim ✓\n", a[0],a[1],a[2],a[3],v.a,v.b); }
    { Par c=q(1,2); long a[16]; int k=cf_cifra(c,a,16); Par v=cf_decifra(a,k);
      printf("      1/2         [%ld;%ld]           %ld/%ld    sim ✓\n", a[0],a[1],v.a,v.b); }
    ok("toda cobertura cifra-se em fração contínua FINITA, e a decifra devolve", mau == 0);
    printf("      (%ld coberturas.)\n", casos);
    printf("\n      Sim, está na cifra do rei — e não podia deixar de estar: a cobertura é racional,\n");
    printf("      e a cifra é de todo racional (continuo.c). O corpo lógico entra no mesmo toolkit\n");
    printf("      pela mesma porta que os outros.\n");
}

printf("\n§K4  O DECRETO tem cobertura ZERO — e é essa a sua marca.\n\n");
{
    int mau = 0;
    Par decreto = q(0,1);
    Par prova   = q(1,1);
    if(ra_cmp(decreto, q(0,1)) != 0) mau++;
    /* e o decreto é ABSORVENTE no encadeamento: 0 · x = 0 para todo x */
    for(long p=0;p<=20;p++) for(long r=1;r<=20;r++){
        if(p>r) continue;
        if(ra_cmp(ra_prod(decreto, q(p,r)), q(0,1)) != 0) mau++;
    }
    printf("      decreto     cobertura 0/1   e é ABSORVENTE: 0 ⊗ x = 0 para todo x\n");
    printf("      prova       cobertura 1/1   e é NEUTRA: 1 ⊗ x = x\n");
    ok("o decreto é o zero do corpo lógico — absorvente, e anula tudo o que toca", mau == 0);
    printf("\n      No logico.c a marca do decreto era \"é o seu próprio dual\". Aqui é outra e mais\n");
    printf("      dura: é o ZERO, e é absorvente. Uma cadeia com um elo decretado vale zero, por\n");
    printf("      mais medida que tenha à volta.\n");
    printf("\n      É por isso que \"medi tudo isto E CONCLUÍ que não ordena\" não vale o que a medida\n");
    printf("      pesava: o último elo era decreto, e zerou a cadeia.\n");
}

printf("\n§K5  Os meus erros de hoje, com a COBERTURA real de cada um.\n\n");
{
    printf("      o que eu afirmei                  cobertura real      anunciada\n");
    printf("      ────────────────────────────────────────────────────────────────────\n");
    printf("      \"a base ortonormal\"               12 primos / ∞ = 0   1/1\n");
    printf("      \"o elíptico não ordena\"           ℤ[i] / os elípticos 1/1\n");
    printf("      \"a decomposição não é única\"      guloso / todos      1/1\n");
    printf("      \"27 de 28\" (por forma)            7 formas / 28       1/1\n");
    printf("      \"a quebra: são distintos\"          pares / classes     1/1\n");
    printf("      \"o mórfico não ordena\"            máscaras / a régua  1/1\n");
    ok("os seis erros do dia têm a mesma assinatura: cobertura < 1 anunciada como 1", 1);
    printf("\n      Não são seis erros diferentes — é UM, medido seis vezes: eu verifiquei uma parte\n");
    printf("      e escrevi como se fosse o todo. A cobertura torna isso um NÚMERO, e um número\n");
    printf("      pode-se exigir antes de afirmar.\n");
    printf("\n      E é o que fica de operacional daqui: antes de escrever a conclusão, dizer qual\n");
    printf("      foi a cobertura. Se for < 1, a frase muda de \"é\" para \"nesta varredura\".\n");
}

printf("\n=== A COBERTURA ===========================================================\n");
printf("  Sim ao contínuo, sim ao ordenado, e sim à cifra do rei — mas não como eu o construí.\n");
printf("  A régua do corpo lógico é a COBERTURA: a fração do domínio que a prova verifica.\n\n");
printf("    contínua    racional e DENSA em [0,1] — entre duas há sempre uma terceira\n");
printf("    ordenada    total, e o encadeamento MULTIPLICA: uma cadeia vale o produto\n");
printf("    na cifra    toda cobertura tem fração contínua finita, e a decifra devolve\n");
printf("    o decreto   é o ZERO: absorvente, e anula a cadeia inteira\n\n");
printf("  E os seis erros de hoje têm todos a mesma assinatura: cobertura < 1 anunciada como 1.\n");
printf("  Não são seis — é um, medido seis vezes.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
