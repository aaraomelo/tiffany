/* sombra_cone.c — AS RELAÇÕES SÃO SOMBRAS: o simplex é a projeção da base de cima.
 *
 * O Aarão: "o corpo de entrada vira uma projeção áurea desse corpo na dimensão acima. Todas as
 * relações são sombras do cone acima. Ele completa, torna dual e reversível."
 *
 * E ISTO EXPLICA O `maisum.c` EM VEZ DE O REPETIR. Lá mediu-se que n+1 vetores em posição de
 * simplex formam um tight frame e que a reconstrução fecha sem se ortogonalizar nada. Ficou por
 * dizer PORQUÊ — e a razão é que eles não são uma construção esperta: são a SOMBRA de uma base
 * ortonormal que vive um andar acima.
 *
 *     em R^(n+1)   e_1 … e_(n+1)      ortonormais, ângulo 90°, sem relação nenhuma
 *     projetados   no hiperplano Σx=0  ->  o simplex, com ângulo arccos(−1/n)
 *
 * O −1/n que o §M2 mediu não é uma propriedade do simplex: é o que sobra da ortogonalidade de
 * cima depois de se perder uma dimensão. As relações entre os vetores de baixo são a sombra da
 * AUSÊNCIA de relações em cima.
 *
 * E é por isso que o +1 «completa, torna dual e reversível»: acrescentar a dimensão não é somar
 * um vetor qualquer — é subir ao andar onde a base já era ortonormal, e onde a reconstrução é
 * trivial porque não há nada a corrigir.
 *
 *   §Z1  o SIMPLEX é a sombra: projeta-se e_i e sai exatamente o simplex
 *   §Z2  o ÂNGULO de baixo é o que a projeção produz — fórmula fechada, não ajuste
 *   §Z3  o CONE: a sombra vive no hiperplano Σx=0, e a normal é a dimensão perdida
 *   §Z4  SUBIR restitui a ortogonalidade — e é isso que torna reversível
 *   §Z5  e a PROJEÇÃO ÁUREA: onde φ aparece, e onde eu não a encontro
 *
 *   cc -O2 -std=c99 -I. sombra_cone.c -lm -o sombra_cone && ./sombra_cone
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define NM 16

static double dot(const double*a,const double*b,int n){
    double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s;
}

int main(void){
printf("\n=== AS RELAÇÕES SÃO SOMBRAS DO CONE ACIMA ================================\n");
printf("    O maisum.c mediu QUE o n+1 conserta. Aqui mede-se PORQUÊ: os n+1\n");
printf("    vetores são a projeção de uma base ortonormal que vive um andar acima.\n");

printf("\n§Z1  O SIMPLEX É A SOMBRA: projeta-se e_i e sai exatamente o simplex.\n\n");
{
    /* Tomam-se os n+1 eixos ortonormais de R^(n+1) e projeta-se cada um no hiperplano
     * ortogonal a (1,1,...,1) — o hiperplano da soma zero. O que sai tem de ser o simplex
     * regular, e mede-se contra as duas propriedades que o §M2 do maisum.c usou: soma nula, e
     * todos os angulos iguais. Se saisse outra coisa, a sombra nao era o simplex. */
    printf("      n    e_i em R^(n+1)   sombra em Σx=0   ‖Σ sombras‖   ⟨s_i,s_j⟩   −1/n\n");
    int mau = 0;
    for(int n = 2; n <= 8; n++){
        int N = n+1;
        double s[NM][NM];
        for(int i = 0; i < N; i++){
            for(int d = 0; d < N; d++) s[i][d] = (i==d) ? 1.0 : 0.0;   /* e_i em R^(n+1) */
            double m = 0;
            for(int d = 0; d < N; d++) m += s[i][d];
            m /= N;
            for(int d = 0; d < N; d++) s[i][d] -= m;                   /* projeta em Σx=0 */
            double nn = sqrt(dot(s[i],s[i],N));
            for(int d = 0; d < N; d++) s[i][d] /= nn;                  /* normaliza a sombra */
        }
        double soma[NM] = {0};
        for(int i = 0; i < N; i++) for(int d = 0; d < N; d++) soma[d] += s[i][d];
        double ns = sqrt(dot(soma,soma,N));
        double pior = 0, ip = dot(s[0],s[1],N);
        for(int i = 0; i < N; i++) for(int j = i+1; j < N; j++){
            double g = dot(s[i],s[j],N);
            if(fabs(g + 1.0/n) > pior) pior = fabs(g + 1.0/n);
        }
        if(ns > 1e-12 || pior > 1e-12) mau++;
        printf("      %-4d %-16d %-16s %-13.2e %-11.6f %.6f\n",
               n, n+1, "Σx=0", ns, ip, -1.0/n);
    }
    printf("\n");
    ok("a projeção dos n+1 eixos dá EXATAMENTE o simplex — soma nula e ângulo −1/n",
       mau == 0);
    printf("      Não se construiu simplex nenhum: projetou-se a base canónica de cima e ele\n");
    printf("      apareceu. O simplex do maisum.c §M2 e esta sombra são o mesmo objeto.\n");
}

printf("\n§Z2  O ÂNGULO de baixo é o que a projeção PRODUZ — fórmula fechada.\n\n");
{
    /* De onde vem o -1/n. Em cima, <e_i,e_j> = 0. A projecao tira a componente ao longo de
     * (1,...,1)/sqrt(N), e cada e_i tem 1/sqrt(N) dessa direcao. Logo o produto interno das
     * sombras e' 0 - 1/N, e a norma de cada uma e' sqrt(1 - 1/N). Dividindo:
     *
     *     cos = (-1/N) / (1 - 1/N) = -1/(N-1) = -1/n
     *
     * — e o -1/n nao e' escolhido, e' o que resta de uma ortogonalidade a que se tirou uma
     * dimensao. Mede-se a formula contra o valor medido. */
    printf("      n    ⟨e_i,e_j⟩ em cima   componente em 1⃗   cos da sombra   −1/n      bate\n");
    int mau = 0;
    for(int n = 2; n <= 8; n++){
        double N = n+1;
        double ip_cima = 0.0;
        double comp = 1.0/N;                       /* o que cada e_i tem da direção (1,…,1) */
        double cos_sombra = (ip_cima - comp)/(1.0 - comp);
        if(fabs(cos_sombra + 1.0/n) > 1e-15) mau++;
        printf("      %-4d %-19.1f %-18.6f %-15.6f %-9.6f %s\n",
               n, ip_cima, comp, cos_sombra, -1.0/n,
               fabs(cos_sombra + 1.0/n) < 1e-15 ? "sim" : "NÃO");
    }
    printf("\n");
    ok("o ângulo da sombra sai da fórmula, não de ajuste: −1/N sobre 1−1/N = −1/n",
       mau == 0);
    printf("      É a frase do Aarão com número: as relações de baixo são o que sobra da\n");
    printf("      AUSÊNCIA de relações em cima. Lá os eixos não se falam; cá, a dimensão que\n");
    printf("      lhes falta obriga-os a um ângulo comum, e esse ângulo é −1/n.\n");
}

printf("\n§Z3  O CONE: a sombra vive em Σx=0, e a normal é a dimensão perdida.\n\n");
{
    /* O hiperplano Sx=0 e' o corpo de baixo; a normal (1,...,1)/sqrt(N) e' exatamente a
     * direcao que se perdeu. Mede-se que TODA sombra e' ortogonal a essa normal — se alguma
     * nao fosse, nao estava no corpo de baixo. */
    int n = 6, N = n+1;
    double s[NM][NM], normal[NM];
    for(int d = 0; d < N; d++) normal[d] = 1.0/sqrt((double)N);
    double pior = 0;
    for(int i = 0; i < N; i++){
        for(int d = 0; d < N; d++) s[i][d] = (i==d) ? 1.0 : 0.0;
        double m = 0;
        for(int d = 0; d < N; d++) m += s[i][d];
        m /= N;
        for(int d = 0; d < N; d++) s[i][d] -= m;
        double c = fabs(dot(s[i], normal, N));
        if(c > pior) pior = c;
    }
    printf("      n = %d, o corpo de baixo é o hiperplano Σx = 0 em R^%d\n", n, N);
    printf("      a normal perdida é (1,…,1)/√%d\n", N);
    printf("      maior componente de uma sombra ao longo da normal: %.3e\n\n", pior);
    ok("toda sombra é ortogonal à normal — vive inteiramente no corpo de baixo", pior < 1e-15);
    printf("      A dimensão que falta não está espalhada pelas sombras: está TODA na normal,\n");
    printf("      e é por isso que se pode devolvê-la de uma vez só. O +1 não repara n coisas —\n");
    printf("      repõe uma.\n");
}

printf("\n§Z4  SUBIR restitui a ortogonalidade — e é isso que torna reversível.\n\n");
{
    /* O fecho. Em baixo a reconstrucao precisa da constante (n+1)/n (maisum.c §M3); em cima
     * ela e' a identidade pura, porque a base e' ortonormal. Mede-se os dois, lado a lado, e a
     * diferenca entre eles E' o preco de ter perdido a dimensao. */
    printf("      n    reconstrução em BAIXO   constante   reconstrução em CIMA   constante\n");
    int mau = 0;
    for(int n = 2; n <= 6; n++){
        int N = n+1;
        double s[NM][NM];
        for(int i = 0; i < N; i++){
            for(int d = 0; d < N; d++) s[i][d] = (i==d) ? 1.0 : 0.0;
            double m = 0;
            for(int d = 0; d < N; d++) m += s[i][d];
            m /= N;
            for(int d = 0; d < N; d++) s[i][d] -= m;
            double nn = sqrt(dot(s[i],s[i],N));
            for(int d = 0; d < N; d++) s[i][d] /= nn;
        }
        /* em baixo: x no hiperplano, reconstruído pelas sombras */
        double x[NM], rec[NM] = {0};
        for(int d = 0; d < N; d++) x[d] = sin(1.7*d);
        double m = 0;
        for(int d = 0; d < N; d++) m += x[d];
        m /= N;
        for(int d = 0; d < N; d++) x[d] -= m;                  /* x no corpo de baixo */
        for(int i = 0; i < N; i++){
            double c = dot(x, s[i], N);
            for(int d = 0; d < N; d++) rec[d] += c*s[i][d];
        }
        double c_baixo = dot(rec,x,N)/dot(x,x,N);
        /* em cima: os e_i, ortonormais — a reconstrução é a identidade */
        double rec2[NM] = {0};
        for(int i = 0; i < N; i++) rec2[i] = x[i];             /* <x,e_i> e_i = x, trivial */
        double c_cima = dot(rec2,x,N)/dot(x,x,N);
        double e = 0;
        for(int d = 0; d < N; d++){ double t = rec[d]/c_baixo - x[d]; e += t*t; }
        if(sqrt(e) > 1e-12 || fabs(c_cima - 1.0) > 1e-12) mau++;
        printf("      %-4d %-23s %-11.6f %-22s %.6f\n",
               n, "exata (÷ constante)", c_baixo, "exata (identidade)", c_cima);
    }
    printf("\n");
    ok("em cima a reconstrução é a identidade; em baixo, exata a menos da constante",
       mau == 0);
    printf("      É isto o «completa, torna dual e reversível»: subir devolve a ortogonalidade,\n");
    printf("      e com ela a volta deixa de precisar de constante nenhuma. O corpo de baixo é\n");
    printf("      reversível PORQUE é sombra de um corpo onde a reversão é trivial.\n");
}

printf("\n§Z5  E A PROJEÇÃO ÁUREA: onde φ aparece, e onde eu NÃO a encontro.\n\n");
{
    /* O Aarao disse "projecao AUREA". Procura-se phi nas quantidades desta projecao — e diz-se
     * o que se acha e o que nao se acha, em vez de forcar. */
    double phi = (1.0 + sqrt(5.0))/2.0;
    printf("      φ = %.10f\n\n", phi);
    printf("      n    ‖sombra‖/‖e_i‖ = √(n/(n+1))   é φ ou 1/φ?\n");
    int achou = 0;
    for(int n = 2; n <= 8; n++){
        double r = sqrt((double)n/(n+1.0));
        int e_phi = fabs(r - phi) < 1e-6 || fabs(r - 1/phi) < 1e-6;
        if(e_phi) achou++;
        printf("      %-4d %-30.10f %s\n", n, r, e_phi ? "SIM" : "não");
    }
    printf("\n");
    ok("a razão de projeção NÃO é a áurea — é √(n/(n+1)), e depende de n", achou == 0);
    printf("      Não encontro φ aqui, e digo-o em vez de o forçar: a razão entre a sombra e o\n");
    printf("      eixo é √(n/(n+1)), que varia com a dimensão e não estabiliza em φ.\n\n");
    /* onde ela ESTÁ, e isso mede-se: no ângulo entre um eixo e a sua própria sombra, no caso
     * n = 1 — e no facto de σ ser o ponto fixo da projeção iterada, que é outra coisa */
    printf("      Onde φ ESTÁ, e está medido noutro sítio: σ = 1 + 1/σ é o ponto fixo da\n");
    printf("      realimentação (checkup.c), e σ·σ' = −1 é a dualidade (furos.c §F4). A\n");
    printf("      projeção que aqui se mede é ORTOGONAL, e a ortogonal não tem φ dentro.\n");
    printf("      Se a projeção áurea existir, é outra projeção — e teria de se dizer qual.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O simplex não é construção: é a sombra da base de cima. O −1/n é o que\n");
printf("    resta da ortogonalidade depois de se perder uma dimensão, e a dimensão\n");
printf("    perdida está toda na normal — por isso o +1 repõe uma coisa e não n.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
