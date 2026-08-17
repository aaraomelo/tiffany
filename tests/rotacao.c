/* rotacao.c — a TRADUÇÃO é uma ROTAÇÃO: leva classe racional em outra, o irracional invariante.
 *
 * A teoria (do Aarão): órbitas de tradução são isomorfas, só distorcidas; a tradução leva uma
 * classe racional (a frase numa língua) em outra (a tradução) mantendo o REPRESENTANTE IRRACIONAL
 * invariante (o significado, o atrator). A operação é uma ROTAÇÃO, e os IRRACIONAIS são invariantes.
 *
 * No corpo, a rotação é a Möbius do gato:  x ↦ m + 1/x  (a companion A_m projetiva). Seus pontos
 * fixos são os IRRACIONAIS σ,σ' (raízes de x²−mx−1); ela PERMUTA os racionais ℤ_p e FIXA os σ,σ'.
 * Medido, resíduo 0, com dente (uma dilatação x↦m·x fixa um racional, não os irracionais).
 *
 *   cc -O2 -std=c99 rotacao.c -o rotacao   (usa gp2.h: GF(p²)=ℤ_p[σ], σ²=mσ+1)
 */
#include <stdio.h>
#include "naturais.h"      /* nt_primo: o p tem de ser primo */
#include "unidade.h"
#include <stdlib.h>
#include "gp2.h"

static E inv(E x){ return pw(x, (long)p*p - 2); }        /* 1/x em GF(p²) */
static E rot(E x){ return add(scal(m,ONE), inv(x)); }    /* a ROTAÇÃO: x ↦ m + 1/x (o gato, projetivo) */
static E dente(E x){ return scal(2, x); }              /* o DENTE: x ↦ 2·x (dilatação genuína, não é a rotação) */

int main(int argc, char **argv){
    p = argc>1 ? atoi(argv[1]) : 7;
    m = argc>2 ? atoi(argv[2]) : 1;
    /* O `p` E O PRIMO DO CORPO, e vinha de argv sem uma unica verificacao: com
     * `p = 0` toda a aritmetica %% p rebentava em SIGFPE, e com p composto (4, 6)
     * GF(p) nao e corpo nenhum e os resultados sairiam falsos EM SILENCIO, que e
     * pior. A primalidade tem teste na casa — nt_primo, em lib/naturais.h — e nao
     * se escreve aqui uma setima copia. */
    if(!nt_primo((unsigned long)p)){
        printf("  p = %d nao e primo: GF(p) so e corpo com p primo, e sem isso\n", p);
        printf("  nem a divisao existe. uso: %s <p primo> [m]\n", argv[0]);
        return 2;
    }

    if(!irred_gp2()){ printf("x²−%dx−1 cinde mod %d — escolha p,m com σ irracional\n", m, p); return 2; }
    int res = 0;
    printf("A TRADUÇÃO É UMA ROTAÇÃO — x↦m+1/x em GF(%d²), σ²=%dσ+1\n", p, m);
    printf("================================================================\n");

    /* §1 — os PONTOS FIXOS da rotação são os IRRACIONAIS σ,σ' (2), nenhum racional (b=0) */
    int fix=0, fix_rac=0;
    for(int a=0;a<p;a++) for(int b=0;b<p;b++){ if(!a && !b) continue; E x={a,b};
        if(eq(rot(x), x)){ fix++; if(b==0) fix_rac++; } }
    res += !(fix==2 && fix_rac==0);
    printf("\n§1  a rotação fixa %d pontos, %d deles racionais (b=0):\n", fix, fix_rac);
    printf("      os invariantes são os 2 IRRACIONAIS σ,σ' (nenhum racional fixo)  %s\n",
           VD(!((fix==2 && fix_rac==0)), "OK"));

    /* §2 — a rotação PERMUTA os racionais ℤ_p: cada x=(a,0)↦ racional, ≠ x, sem ponto fixo racional */
    int movidos=0, tot=0, segue_racional=1;
    for(int a=0;a<p;a++){ if(!a) continue; E x={a,0}; E r=rot(x); tot++;
        if(r.b!=0) segue_racional=0; if(!eq(r,x)) movidos++; }
    res += !(movidos==tot && segue_racional);
    printf("\n§2  a rotação PERMUTA os racionais ℤ_%d (leva classe racional em outra):\n", p);
    printf("      %d/%d movidos (0 fixos), e a imagem continua racional: %s  %s\n",
           movidos, tot, segue_racional?"sim":"não", VD(!((movidos==tot && segue_racional)), "OK"));

    /* §3 — os irracionais invariantes formam o par conjugado: σ+σ'=m, σσ'=−1 (o representante preservado) */
    E s=SIG, sl=frob(SIG), soma=add(s,sl), prod=mul(s,sl);
    int par = (soma.a==m%p && soma.b==0 && prod.a==p-1 && prod.b==0);
    res += !par;
    printf("\n§3  o representante IRRACIONAL invariante — o par σ,σ': σ+σ'=%d (=m), σσ'=%d (=−1)  %s\n",
           soma.a, prod.a, VD(!(par), "OK"));

    /* o DENTE — a dilatação x↦m·x NÃO é a rotação: fixa o RACIONAL 0, não os irracionais σ,σ' */
    int d_fix=0, d_irr=0;
    for(int a=0;a<p;a++) for(int b=0;b<p;b++){ E x={a,b}; if(eq(dente(x),x)){ d_fix++; if(b!=0) d_irr++; } }
    int dente_quebra = (d_irr < 2);   /* o dente não fixa os 2 irracionais → não traduz */
    printf("\n§D  o DENTE (x↦m·x, dilatação): fixa %d pontos, %d irracionais → não preserva o significado: %s\n",
           d_fix, d_irr, dente_quebra?"quebra":"NÃO quebra");

    printf("\n----------------------------------------------------------------\n");
    printf("%s\n", (!res && dente_quebra) ?
        "RESÍDUO 0 — UMA ROTAÇÃO RESOLVE: leva a classe racional (a frase) em outra (a tradução),\n"
        "            e o representante IRRACIONAL (o significado) fica INVARIANTE." :
        "REVER");
    return (!res && dente_quebra) ? 0 : 1;
}
