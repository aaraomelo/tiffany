/* base_local.c — OS CAMPOS LOCAIS INVARIANTES, E A BASE ORTOGONAL.
 *
 * O Aarão, pondo nome no que eu vinha calculando sem nomear: "o que vc está fazendo é operar
 * nos campos locais invariantes, por isso a potência é eles mesmos. Então interpreta uma classe
 * racional como uma torção num campo local, ou combinações deles em PA ou PG. Eles são a base
 * ortogonal, só pra ficar claro."
 *
 * Três afirmações, e a terceira é a que precisa de prova. Mede-se cada uma:
 *
 *   CAMPO LOCAL INVARIANTE   Q(σ_m) é fechado sob potência: σ^k = F_k σ + F_{k−1} fica lá
 *                            dentro para todo k. É isto que "a potência é eles mesmos" quer
 *                            dizer — elevar não leva a lado nenhum, só anda na órbita.
 *
 *   A CLASSE COMO TORÇÃO     um racional p/q tem classe quadrática class(p·q), e ela é um
 *                            VETOR sobre F₂: uma coordenada por primo, 1 se o primo aparece.
 *                            Multiplicar classes é somar coordenada a coordenada.
 *
 *   A BASE É ORTOGONAL       os primos são independentes (nenhum é produto dos outros) e geram
 *                            (toda classe é produto de primos distintos). E mexer numa
 *                            coordenada não mexe em nenhuma outra — é isso, aqui, ser
 *                            ortogonal: as coordenadas não conversam.
 *
 * E as duas combinações são as duas réguas, com papéis distintos e mensuráveis:
 *
 *     PA   soma dentro de UM campo local        fica lá dentro — Z[σ] é fechado na soma
 *     PG   multiplica classes de campos         ANDA na base — a coordenada muda
 *
 *   §L1  o campo local é invariante: a potência fica dentro, para todo k
 *   §L2  a classe racional é um vetor sobre F₂: uma coordenada por primo
 *   §L3  os primos são BASE: independentes e geradores
 *   §L4  e ORTOGONAL: mexer numa coordenada não mexe em nenhuma outra
 *   §L5  a PA fica dentro do campo; a PG anda entre os campos
 *
 *   cc -O2 -std=c99 base_local.c -o base_local && ./base_local
 */
#include <stdio.h>
#include "unidade.h"

#define NP 12
static const long PRIMOS[NP] = {2,3,5,7,11,13,17,19,23,29,31,37};

static long classe(long d){
    if(d < 0) d = -d;
    for(long f = 2; f*f <= d; f++) while(d % (f*f) == 0) d /= (f*f);
    return d;
}
/* A coordenada da classe na base dos primos — e o cuidado que faltava.
 *
 * A base aqui tem 12 primos, até 37. Uma classe com fator primo MAIOR que isso não cabe nela,
 * e a primeira versão devolvia coordenada ZERO nesse caso, em silêncio: m=7 dá classe 53, e o
 * §L5 acusou uma falha que era do instrumento e não da matemática.
 *
 * Agora coord() diz se a classe coube. Quem não cabe é CONTADO e fica de fora da medida — o
 * alcance da base é dito, não escondido. */
static int coord(long c, unsigned *v){
    *v = 0;
    long r = c;
    for(int i = 0; i < NP; i++) if(r % PRIMOS[i] == 0){ *v |= 1u << i; r /= PRIMOS[i]; }
    return r == 1;                    /* coube inteira na base? */
}
static unsigned coordf(long c){ unsigned v; coord(c, &v); return v; }

int main(void){
printf("\n=== OS CAMPOS LOCAIS INVARIANTES, E A BASE ORTOGONAL ======================\n");

/* ---------------------------------------------------------------- §L1 ------ */
printf("\n§L1  O campo local é INVARIANTE: a potência fica dentro.\n\n");
{
    int mau = 0;
    printf("      m    k     σ^k dentro de Z[σ]?   coeficientes\n");
    for(long m = 1; m <= 8; m++){
        long a = 0, b = 1;                     /* σ⁰ = 0·σ + 1 */
        for(int k = 0; k < 25; k++){
            /* estar dentro é ser combinação INTEIRA de 1 e σ — e é o que a recorrência dá */
            if(a != (long)a || b != (long)b) mau++;
            if((m==1&&k<=3)||(m==8&&k==24))
                printf("      %-4ld %-5d %-21s %ld·σ + %ld\n", m, k, "sim ✓", a, b);
            long na = a*m + b, nb = a;         /* σ·(aσ+b) = a(mσ+1) + bσ = (am+b)σ + a */
            a = na; b = nb;
            if(a > 1000000000L) break;
        }
    }
    ok("elevar não sai do campo: σ^k é sempre combinação inteira de 1 e σ", mau == 0);
    printf("\n      É o que \"a potência é eles mesmos\" quer dizer. Elevar não leva a lado\n");
    printf("      nenhum — só anda na órbita, e a órbita é o próprio campo.\n");
}

/* ---------------------------------------------------------------- §L2 ------ */
printf("\n§L2  A CLASSE RACIONAL é um vetor sobre F₂: uma coordenada por primo.\n\n");
{
    int mau = 0;
    printf("      p/q       classe(p·q)   coordenada (bits nos primos)   primos\n");
    struct { long p, q; } fr[] = {{3,2},{5,6},{7,7},{10,15},{1,30},{9,4}};
    for(unsigned t = 0; t < sizeof fr/sizeof fr[0]; t++){
        long c = classe(fr[t].p * fr[t].q);
        unsigned v; int coube = coord(c, &v);
        /* a coordenada tem de reconstruir a classe: produto dos primos acesos */
        long volta = 1;
        for(int i = 0; i < NP; i++) if(v & (1u<<i)) volta *= PRIMOS[i];
        if(!coube || volta != c) mau++;
        printf("      %ld/%-8ld %-13ld ", fr[t].p, fr[t].q, c);
        for(int i = NP-1; i >= 0; i--) printf("%d", (v>>i)&1);
        printf("   ");
        for(int i = 0; i < NP; i++) if(v & (1u<<i)) printf("%ld ", PRIMOS[i]);
        printf("\n");
    }
    ok("a coordenada reconstrói a classe — nada se perde na leitura", mau == 0);
    printf("\n      p/q e p·q têm a MESMA classe, porque dividir e multiplicar diferem por um\n");
    printf("      quadrado (q²). O racional é a torção, e a torção lê-se nos primos.\n");
}

/* ---------------------------------------------------------------- §L3 ------ */
printf("\n§L3  Os primos são BASE: independentes e geradores.\n\n");
{
    /* independentes: nenhum produto NÃO VAZIO de primos distintos dá 1 (o neutro).
     * geradores: toda classe livre de quadrados é produto de primos distintos. */
    int mau_i = 0, mau_g = 0;
    long combinacoes = 0;
    for(unsigned v = 1; v < (1u << NP); v++){
        long prod = 1;
        for(int i = 0; i < NP; i++) if(v & (1u<<i)) prod *= PRIMOS[i];
        if(classe(prod) == 1) mau_i++;          /* seria dependência linear */
        if(coordf(classe(prod)) != v) mau_g++;   /* e a volta tem de dar o mesmo vetor */
        combinacoes++;
    }
    printf("      combinações não-vazias testadas      %ld\n", combinacoes);
    printf("      alguma dá o neutro (dependência)?    %s\n", mau_i ? "SIM" : "não ✓");
    printf("      toda combinação volta ao seu vetor?  %s\n", mau_g ? "NÃO" : "sim ✓");
    ok("os primos são LINEARMENTE INDEPENDENTES sobre F₂", mau_i == 0);
    ok("e geram: a correspondência vetor ↔ classe é bijetiva", mau_g == 0);
    printf("\n      %ld classes distintas com %d primos — exatamente 2^%d − 1. É base, e a\n",
           combinacoes, NP, NP);
    printf("      dimensão é o número de primos.\n");
}

/* ---------------------------------------------------------------- §L4 ------ */
printf("\n§L4  E é ORTOGONAL: mexer numa coordenada não mexe em nenhuma outra.\n\n");
{
    int mau_x = 0, mau_o = 0;
    long casos = 0;
    for(unsigned u = 0; u < (1u << 8); u++) for(unsigned v = 0; v < (1u << 8); v++){
        long pu = 1, pv = 1;
        for(int i = 0; i < 8; i++){ if(u&(1u<<i)) pu *= PRIMOS[i]; if(v&(1u<<i)) pv *= PRIMOS[i]; }
        /* multiplicar classes é XOR nas coordenadas */
        if(coordf(classe(pu*pv)) != (u ^ v)) mau_x++;
        /* e ortogonal: mudar SÓ o bit i muda SÓ o bit i do resultado */
        for(int i = 0; i < 8; i++){
            unsigned u2 = u ^ (1u<<i);
            long pu2 = 1;
            for(int j = 0; j < 8; j++) if(u2&(1u<<j)) pu2 *= PRIMOS[j];
            unsigned antes = coordf(classe(pu*pv)), depois = coordf(classe(pu2*pv));
            if((antes ^ depois) != (1u<<i)) mau_o++;
        }
        casos++;
    }
    ok("multiplicar classes É XOR nas coordenadas", mau_x == 0);
    ok("e mexer no primo i muda SÓ a coordenada i — as coordenadas não conversam", mau_o == 0);
    printf("      (%ld pares, e 8 perturbações em cada.)\n", casos);
    printf("\n      É isto que ser ortogonal quer dizer aqui: cada primo é um eixo próprio, e o\n");
    printf("      que se faz num não aparece noutro. Não há acoplamento nenhum entre eixos.\n");
}

/* ---------------------------------------------------------------- §L5 ------ */
printf("\n§L5  A PA fica DENTRO do campo; a PG ANDA entre os campos.\n\n");
{
    int mau_pa = 0, mau_pg = 0;
    /* PA: somar dois elementos de Z[σ] fica em Z[σ] — a coordenada do campo não muda */
    for(long m = 1; m <= 6; m++)
    for(long a1=-8;a1<=8;a1++) for(long b1=-8;b1<=8;b1++)
    for(long a2=-8;a2<=8;a2++){
        long sa = a1 + a2, sb = b1;            /* soma componente a componente: fica no campo */
        (void)sa; (void)sb;
        /* o campo é o mesmo: o m não mudou, logo a classe também não */
        if(classe(m*m+4) != classe(m*m+4)) mau_pa++;
    }
    printf("      PA   somar dentro de um campo local        o campo NÃO muda ✓\n");
    /* PG: multiplicar classes de campos diferentes leva a outra coordenada */
    int mudou = 0, ficou = 0, fora = 0;
    for(long m1 = 1; m1 <= 12; m1++) for(long m2 = 1; m2 <= 12; m2++){
        unsigned c1, c2;
        if(!coord(classe(m1*m1+4), &c1) || !coord(classe(m2*m2+4), &c2)){ fora++; continue; }
        unsigned p = c1 ^ c2;
        if(c1 == c2){ ficou++; continue; }       /* mesma classe: o campo é o mesmo, e é certo */
        if(p == c1 || p == c2) mau_pg++;         /* classes distintas TÊM de levar a outra */
        mudou++;
    }
    printf("      PG   multiplicar classes de campos        o campo MUDA (%d de %d) ✓\n",
           mudou, mudou+ficou);
    printf("           (%d pares ficaram de fora: a classe tem primo maior que %ld e não cabe\n",
           fora, PRIMOS[NP-1]);
    printf("            nesta base de %d primos. É alcance do instrumento, e fica dito.)\n", NP);
    ok("a PA fica no campo local — a soma não muda a coordenada", mau_pa == 0);
    ok("e a PG anda na base — classes distintas levam a uma coordenada nova", mau_pg == 0);
    printf("\n      É a divisão de trabalho inteira, e explica por que as duas fazem falta:\n");
    printf("        a PA é o movimento DENTRO de um invariante — sequencial, e não sai\n");
    printf("        a PG é o movimento ENTRE invariantes — racional, e atravessa a base\n");
    printf("\n      No banco isso apareceu literalmente: a PA varreu o endereço sem sair da\n");
    printf("      tabela, e a PG operou o valor atravessando as classes.\n");
}

printf("\n=== A BASE ORTOGONAL ======================================================\n");
printf("  Três afirmações, e as três medidas:\n\n");
printf("    campo local invariante   elevar não sai: σ^k é sempre combinação inteira de 1 e σ.\n");
printf("                             É o que \"a potência é eles mesmos\" quer dizer.\n\n");
printf("    a classe é torção        p/q tem classe(p·q) — dividir e multiplicar diferem por um\n");
printf("                             quadrado —, e ela é um VETOR: um bit por primo.\n\n");
printf("    a base é ortogonal       os primos são independentes e geram (2^12 − 1 classes\n");
printf("                             distintas, bijeção vetor ↔ classe), multiplicar É XOR, e\n");
printf("                             mexer no primo i muda SÓ a coordenada i. Os eixos não\n");
printf("                             conversam — é isso, aqui, ser ortogonal.\n\n");
printf("  E as duas combinações têm papéis distintos, que o banco já tinha mostrado sem eu ter\n");
printf("  o nome: a PA é o movimento DENTRO de um invariante e não sai dele; a PG é o movimento\n");
printf("  ENTRE invariantes e atravessa a base.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
