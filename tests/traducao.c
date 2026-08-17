/* traducao.c — a FUNÇÃO DE ROTAÇÃO DETERMINÍSTICA entre classes (a tradução).
 *
 * A rotação (a Möbius do gato) tem dois pontos fixos: os IRRACIONAIS σ,σ' (o significado, o
 * atrator — invariante). Na coordenada canônica u=(z−σ)/(z−σ'), toda rotação com esses fixos é
 *
 *        R_k(z):   (R−σ)/(R−σ') = k·(z−σ)/(z−σ')          (k = o MULTIPLICADOR)
 *
 * Uma Möbius é DETERMINADA pelos 2 pontos fixos + 1 par. Logo, dado σ,σ' (o significado) e UM par
 * de classes correspondentes (PT₀,EN₀), o multiplicador k fica fixado — e de QUALQUER classe PT
 * obtém-se a sua EN por R_k. A tradução é uma função determinística; o irracional é invariante.
 *
 *   R_k(z) = (σ − k·u·σ')/(1 − k·u),   u=(z−σ)/(z−σ')   ;   k = [(w−σ)/(w−σ')]/[(z−σ)/(z−σ')]
 *
 *   cc -O2 -std=c99 traducao.c -o traducao   (usa gp2.h: GF(p²)=ℤ_p[σ], σ²=mσ+1)
 */
#include <stdio.h>
#include "naturais.h"      /* nt_primo: o p tem de ser primo */
#include "unidade.h"
#include <stdlib.h>
#include "gp2.h"

static E inv(E x){ return pw(x, (long)p*p - 2); }
static E dvd(E a, E b){ return mul(a, inv(b)); }
/* a rotação R_k com pontos fixos σ,σ' e multiplicador k */
static E Rk(E z, E s, E sl, E k){
    E u = dvd(sub(z,s), sub(z,sl)), ku = mul(k,u);
    return dvd(sub(s, mul(ku,sl)), sub(ONE, ku));
}
/* o multiplicador de um par (z→w) com fixos σ,σ':  k = [(w−σ)/(w−σ')]/[(z−σ)/(z−σ')] */
static E mult(E z, E w, E s, E sl){
    E uz = dvd(sub(z,s), sub(z,sl)), uw = dvd(sub(w,s), sub(w,sl));
    return dvd(uw, uz);
}

int main(int argc, char **argv){
    p = argc>1 ? atoi(argv[1]) : 97;
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
    E s = SIG, sl = frob(SIG);                          /* σ,σ' — os irracionais (o significado) */
    int res = 0;
    printf("A TRADUÇÃO É UMA FUNÇÃO DE ROTAÇÃO DETERMINÍSTICA — GF(%d²), σ²=%dσ+1\n", p, m);
    printf("================================================================\n");

    /* uma língua-alvo é uma rotação R_K (a distorção entre PT e EN); escolho um K qualquer */
    E K = {3, 5};

    /* §1 — a rotação FIXA os irracionais σ,σ' (o significado invariante) */
    int fixa = eq(Rk(s,s,sl,K), s);                     /* R_K(σ)=σ (u=0) */
    res += !fixa;
    printf("\n§1  a rotação fixa o irracional: R(σ)=σ (o significado invariante)  %s\n", VD(!(fixa), "OK"));

    /* §2 — DETERMINÍSTICA: de UM par (z₀→w₀) recupera-se K, e R traduz TODA classe (== o original) */
    E z0={7,2}, w0=Rk(z0,s,sl,K);                        /* um par de classes correspondentes PT₀↔EN₀ */
    E Krec = mult(z0, w0, s, sl);                        /* o multiplicador determinado pelo par */
    int k_ok = eq(Krec, K);
    long ok=0, tot=0; int dente_quebra=1;
    for(int a=0;a<p;a++) for(int b=0;b<p;b++){ E z={a,b};
        if(eq(z,s)||eq(z,sl)) continue;                 /* evita os fixos (u indefinido) */
        E en = Rk(z,s,sl,K);                            /* a EN verdadeira */
        E en_rec = Rk(z,s,sl,Krec);                     /* a EN reconstruída do par (determinística) */
        if(eq(en_rec,en)) ok++; tot++;
        E en_dente = Rk(z,s,sl,add(K,ONE));             /* o DENTE: multiplicador errado (K+1) */
        if(eq(en_dente,en)) dente_quebra=0;
    }
    res += !(k_ok && ok==tot);
    printf("\n§2  DETERMINÍSTICA — um par (PT₀→EN₀) fixa o multiplicador k, e R traduz toda classe:\n");
    printf("      k recuperado do par == k verdadeiro: %s ; R traduz %ld/%ld classes exatas  %s\n",
           k_ok?"sim":"não", ok, tot, VD(!((k_ok && ok==tot)), "OK"));

    /* §3 — de uma CLASSE obtém-se a outra, e a volta fecha (R⁻¹ = R_{1/k}) — reversível */
    E z={11,4}, en=Rk(z,s,sl,K), volta=Rk(en,s,sl,inv(K));
    int rev = eq(volta,z);
    res += !rev;
    printf("\n§3  de uma classe obtém-se a outra, e a VOLTA fecha (R⁻¹=R_{1/k}): PT→EN→PT  %s\n", VD(!(rev), "OK"));

    printf("\n§D  o DENTE (multiplicador errado k+1) traduz alguma classe certo? %s\n",
           dente_quebra?"não — quebra":"SIM (falha)");

    printf("\n----------------------------------------------------------------\n");
    printf("%s\n", (!res && dente_quebra) ?
        "RESÍDUO 0 — A TRADUÇÃO É UMA ROTAÇÃO DETERMINÍSTICA: σ,σ' (o significado) fixam a rotação,\n"
        "            um par PT↔EN fixa o multiplicador, e de qualquer classe se obtém a outra." :
        "REVER");
    return (!res && dente_quebra) ? 0 : 1;
}
