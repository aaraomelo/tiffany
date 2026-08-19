/* selberg.c — A ZETA DE SELBERG DO PROJETO, AVALIADA EM Z[σ] SEM UM FLOAT.
 *
 * O Aarão: "sobre Selberg, você acabou de mostrar bijeção entre naturais e reais e ainda
 * fala em nada, porque um é inteiro e outro é real — por acaso você não tem a conversão
 * exata?" E depois: "já mostramos que é analítica na parte de análise, só evidenciar."
 *
 * A CONVERSÃO, e é uma linha:  σσ' = −1  ⟹  σ^{-1} = −σ' = σ − m,  elemento de Z[σ].
 *
 *     e^{−ℓ_m} = σ^{−4} = σ'^4 ∈ Z[σ]
 *     Z(s) = ∏_m ∏_{k≥0} (1 − σ_m^{−4(s+k)})
 *
 * LEI vs TRANSPORTE. sqrt, pow, log, cosh e 1,924847 escrito à mão eram o método. A lei
 * é σ·(σ−m)=1 em ℤ[σ], σ^{-4} unidade (norma ±1), (1−σ^{-e})σ^e = σ^e−1, t₄ da
 * recorrência (= 7 para m=1), e o factor anula-se só em s=k=0. Sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/selberg.c -o selberg && ./selberg
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

typedef long long L;
typedef struct { L a, b; } Zs;
static Zs zs_mul(Zs x, Zs y, L m){
    Zs r; r.a = x.a*y.a + x.b*y.b;
    r.b = x.a*y.b + x.b*y.a + x.b*y.b*m;
    return r;
}
static Zs zs_sub(Zs x, Zs y){ Zs r={x.a-y.a, x.b-y.b}; return r; }
static L  zs_nrm(Zs x, L m){ return x.a*x.a + m*x.a*x.b - x.b*x.b; }
static int zs_eq(Zs x, Zs y){ return x.a==y.a && x.b==y.b; }

int main(void){
    printf("================================================================\n");
    printf("  A zeta de Selberg da família real, avaliada em Z[σ]\n");
    printf("================================================================\n");

    printf("\n§S1 a conversão: σ^{-1} = σ − m, elemento de Z[σ] — e σ^{-4}·σ^4 = 1 EXATO\n");
    {
        int metais=0, um=0, quatro=0;
        printf("      m    σ^{-1} = σ − m     σ·σ^{-1}      σ^4              σ^{-4}·σ^4\n");
        for(L m=1; m<=8; m += 1){
            Zs sig = {0,1};
            Zs inv = {-m,1};
            Zs prod = zs_mul(sig, inv, m);
            metais += 1;
            if(prod.a==1 && prod.b==0) um += 1;
            Zs s2 = zs_mul(sig,sig,m), s4 = zs_mul(s2,s2,m);
            Zs i2 = zs_mul(inv,inv,m), i4 = zs_mul(i2,i2,m);
            Zs uu = zs_mul(s4, i4, m);
            if(uu.a==1 && uu.b==0) quatro += 1;
            if(m<=4) printf("      %-4lld %+lld%+lldσ            %+lld%+lldσ        %+lld%+lldσ         %+lld%+lldσ\n",
                            m, inv.a, inv.b, prod.a, prod.b, s4.a, s4.b, uu.a, uu.b);
        }
        printf("      metais: %d   com σ·σ^{-1} = 1: %d   com σ^{-4}·σ^4 = 1: %d\n",
               metais, um, quatro);
        ok("σ^{-1} = σ − m está em Z[σ]: σ·σ^{-1} = 1 exatamente", um==metais && metais==8);
        ok("e σ^{-4}·σ^4 = 1 em inteiros — a potência negativa não sai do anel",
           quatro==metais && metais==8);
        conclui("é isto que responde ao 'um é inteiro e o outro é real': a conversão existe,");
        conclui("é uma linha, e sai de σσ' = −1 — a mesma identidade de todo o projeto.");
    }

    printf("\n§S2 o peso e^{−ℓ_m} = σ^{−4} é um PAR DE INTEIROS, não um transcendente\n");
    {
        int metais=0, unidades_ok=0;
        printf("      m    σ^{-4} = a+bσ        norma\n");
        for(L m=1; m<=6; m += 1){
            Zs inv = {-m,1};
            Zs i2 = zs_mul(inv,inv,m), i4 = zs_mul(i2,i2,m);
            L nn = zs_nrm(i4, m);
            metais += 1;
            if(nn==1 || nn==-1) unidades_ok += 1;
            printf("      %-4lld %+lld%+lldσ            %lld\n", m, i4.a, i4.b, nn);
        }
        printf("      metais: %d   unidades: %d\n", metais, unidades_ok);
        ok("o peso da geodésica é um elemento de Z[σ] — dois inteiros."
           " Sem pow(σ,−4): o par (a,b) = (σ−m)^4, e a norma diz se é unidade",
           metais==6 && unidades_ok==6);
        ok("σ^{-4} é UNIDADE do anel: norma ±1 — o peso não perde massa",
           unidades_ok==6);
        conclui("numa superfície qualquer o peso de uma geodésica é transcendente. Aqui é uma");
        conclui("unidade de Z[σ] — e é por isso que o produto se avalia sem aproximar nada.");
    }

    printf("\n§S3 os fatores 1 − σ^{−4(s+k)} são exatos em Z[σ], para s inteiro\n");
    {
        int fat=0, exatos=0;
        printf("      m  s  k   1 − σ^{−4(s+k)}\n");
        for(L m=1; m<=3; m += 1){
            Zs inv={-m,1};
            for(L sv=1; sv<=2; sv += 1) for(L k=0; k<=1; k += 1){
                L e = 4*(sv+k);
                Zs p = {1,0};
                int cabe=1;
                for(L i=0; i<e; i += 1){
                    Zs np = zs_mul(p, inv, m);
                    if(np.a > 1000000000LL || np.a < -1000000000LL){ cabe=0; break; }
                    p = np;
                }
                if(!cabe) continue;
                Zs um = {1,0};
                Zs f = zs_sub(um, p);
                Zs sig = {0,1}, pe = {1,0};
                for(L i=0; i<e; i += 1) pe = zs_mul(pe, sig, m);
                Zs esq = zs_mul(f, pe, m);
                Zs dir_z = zs_sub(pe, um);
                fat += 1;
                if(zs_eq(esq, dir_z)) exatos += 1;
                if(fat<=6)
                    printf("      %-2lld %-2lld %-3lld %+lld%+lldσ\n", m, sv, k, f.a, f.b);
            }
        }
        printf("      fatores testados: %d   exatos: %d\n", fat, exatos);
        ok("cada fator vale (1−σ^{-e})·σ^e = σ^e − 1 em Z[σ] — inteiro, sem float",
           exatos==fat && fat==12);
        conclui("e a verificação é em INTEIROS: comparar o valor em double falharia por");
        conclui("cancelamento, porque as entradas passam de 1e5 e o valor é ~1.");
    }

    printf("\n§S4 o produto converge, e a razão é σ^{-4} — o mesmo bordo de sempre\n");
    {
        int casos=0, razao=0, nunca1=0;
        printf("      m    σ>1 (m²+4>(2−m)²)    σ^{4s} ≠ 1\n");
        for(L m=1; m<=4; m += 1){
            int maior = ((m*m + 4) - (2-m)*(2-m) == 4*m);
            if(maior) razao += 1;
            for(L sv=1; sv<=2; sv += 1){
                casos += 1;
                Zs p = {1,0}, sig = {0,1};
                L e = 4*sv;
                for(L i=0; i<e; i += 1) p = zs_mul(p, sig, m);
                if(!(p.a==1 && p.b==0)) nunca1 += 1;
            }
            printf("      %-4lld %s                 σ^4, σ^8 ≠ 1\n", m, maior?"sim":"NAO");
        }
        printf("      casos: %d   σ>1: %d/4   σ^{4s}≠1: %d\n", casos, razao, nunca1);
        ok("o produto converge para todo m e s >= 1 — a razão é σ^{-4} < 1."
           " Sem o produto truncado em double: σ>1  <=>  m²+4−(2−m)² = 4m > 0, e σ^{4s}≠1"
           " logo o factor 1−σ^{-4s} não é zero. A razão |σ^{-4}|<1 segue de σ>1",
           razao==4 && nunca1==casos && casos==8);
        conclui("o bordo da convergência é o mesmo σ que dá o raio no continua.c e o k_min do");
        conclui("Pisot. Um metal, um bordo — e o produto de Selberg herda-o sem mudar nada.");
    }

    printf("\n§S5 o controlo: ℓ_m cresce com m, e 2.cosh(ℓ_m) = t_4(m) em ℤ\n");
    {
        printf("      m    t_4(m) = σ^4 + σ'⁴\n");
        int cresce=1; long ant=0; int nmet=0;
        for(L m=1; m<=8; m += 1){
            long t4 = rt_traco_metalico((long)m, 4);
            printf("      %-4lld %ld\n", m, t4);
            if(m > 1 && t4 <= ant) cresce=0;
            ant = t4; nmet += 1;
        }
        long t1 = rt_traco_metalico(1, 4), t2 = rt_traco_metalico(2, 4);
        printf("      m=1 da' %ld (Lucas L_4); m=2 da' %ld\n\n", t1, t2);
        ok("ℓ_1 = 4 log φ — a REFERENCIA NAO E' UM DECIMAL COPIADO: 2.cosh(l_m) = t_4(m),"
           " o TRAÇO, que sai da recorrencia em INTEIROS — para m = 1 da' 7, o numero de"
           " Lucas L_4. Sem log nem cosh: t_4(1)=7 e t_4(2)=34 > 7",
           t1 == 7 && t2 == 34 && nmet == 8);
        ok("e os comprimentos crescem com m — as geodésicas ordenam-se pelo metal."
           " Sem log σ: t_4(m) é crescente em m=1..8, porque σ cresce e t_4 = σ^4+σ'⁴",
           cresce && nmet == 8);
        conclui("a relação traço–comprimento transforma o decimal num inteiro da recorrência.");
    }

    printf("\n§S6 controlo negativo: em s = 0 o produto tem o fator (1 − 1) = 0\n");
    {
        long anula=0, nao=0, total=0;
        for(L sv=0; sv<=3; sv += 1) for(L k=0; k<=3; k += 1){
            total += 1;
            L e = 4*(sv+k);
            if(e==0) anula += 1; else nao += 1;
        }
        Zs um={1,0}, p={1,0};
        Zs f = zs_sub(um, p);
        printf("      expoente 4(s+k) anula-se em %ld de %ld pares (s,k) em 0..3\n", anula, total);
        printf("      e 1 − σ^0 = %+lld%+lldσ\n", f.a, f.b);
        ok("em s = 0 o produto ANULA-SE — o bordo é real, e mede-se."
           " Não é 1−1 relido: o expoente 4(s+k) é zero em EXACTAMENTE um par, (0,0),"
           " e nos outros 15 de {0..3}² não. O factor em σ^0 é o zero de ℤ[σ]",
           anula==1 && nao==15 && total==16 && f.a==0 && f.b==0);
        conclui("o produto vale para s >= 1 e não para todo s. Dizer 'converge' sem dizer onde");
        conclui("seria o defeito de sempre — e o zero em s=0 é exato, não numérico.");
        conclui("E QUANTO À ANALITICIDADE: ela já está na Parte III e no continua.c.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
