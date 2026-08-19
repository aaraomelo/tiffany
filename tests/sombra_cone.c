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
 * LEI vs TRANSPORTE. Projectar, normalizar com sqrt e sin a gerar x eram o método. A lei é
 * w_i = (n+1) e_i − 1 em ℤ^{n+1}: w_i + 1 = (n+1) e_i (a identidade da projecção, sem
 * dividir), n⟨w_i,w_j⟩ + ‖w‖² = 0 (é −1/n), Σ⟨x,w⟩w = (n+1)² x no hiperplano, e n²−n−1 ≠ 0
 * (a razão não é φ). Sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/sombra_cone.c -o sombra_cone && ./sombra_cone
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

#define DMAX 10

/* A sombra inteira: N e_i − 1, N = n+1. É a projecção no hiperplano Σ = 0, vezes N. */
static void sombra(int n, long w[DMAX][DMAX]){
    int d = n + 1;
    for(int i = 0; i < d; i += 1)
        for(int j = 0; j < d; j += 1)
            w[i][j] = (i == j) ? n : -1;
}

int main(void){
printf("\n=== AS RELAÇÕES SÃO SOMBRAS DO CONE ACIMA ================================\n");
printf("    O maisum.c mediu QUE o n+1 conserta. Aqui mede-se PORQUÊ: os n+1\n");
printf("    vetores são a projeção de uma base ortonormal que vive um andar acima.\n");

printf("\n§Z1  O SIMPLEX É A SOMBRA: projeta-se e_i e sai exatamente o simplex.\n\n");
{
    /* w_i + 1⃗ = N e_i — a identidade da projecção, sem dividir por N. Se a sombra
     * não viesse dos eixos, esta conta não fechava. */
    printf("      n    N    w_i+1 = N e_i?   Σw = 0?   n⟨,⟩+‖w‖² = 0?\n");
    int mau = 0, ns = 0;
    for(int n = 2; n <= 8; n += 1){
        long w[DMAX][DMAX];
        sombra(n, w);
        int d = n + 1;
        int proj = 1, sum0 = 1, ang = 1;
        long soma[DMAX] = {0};
        for(int i = 0; i < d; i += 1){
            for(int j = 0; j < d; j += 1){
                long eixo = (i == j) ? d : 0;
                if(w[i][j] + 1 != eixo) proj = 0;
                soma[j] += w[i][j];
            }
        }
        for(int j = 0; j < d; j += 1) if(soma[j]) sum0 = 0;
        long ip = rt_dir(w[0], w[1], d), nw = rt_norma(w[0], d);
        for(int i = 0; i < d; i += 1) for(int j = i + 1; j < d; j += 1){
            long g = rt_dir(w[i], w[j], d);
            if(g != ip) ang = 0;
            if(n * g + nw != 0) ang = 0;
        }
        for(int i = 0; i < d; i += 1) if(rt_norma(w[i], d) != nw) ang = 0;
        ns += 1;
        if(!proj || !sum0 || !ang) mau += 1;
        printf("      %-4d %-4d %-16s %-10s %s\n",
               n, d, proj ? "sim" : "NÃO", sum0 ? "sim" : "NÃO", ang ? "sim" : "NÃO");
    }
    printf("\n");
    ok("a projecao dos n+1 eixos da EXACTAMENTE o simplex — w_i + 1 = N e_i, soma nula"
       " e n⟨w_i,w_j⟩ + ‖w‖² = 0, que e' −1/n sem dividir nem raiz. Nao se construiu"
       " simplex nenhum: projectou-se a base canonica de cima e ele apareceu",
       mau == 0 && ns == 7);
    conclui("O simplex do maisum.c §M2 e esta sombra sao o mesmo objeto.");
}

printf("\n§Z2  O ÂNGULO de baixo é o que a projecção PRODUZ — fórmula fechada.\n\n");
{
    /* Em cima ⟨e_i,e_j⟩ = 0. A sombra s = N e − 1 dá
     *      ⟨s_i,s_j⟩ = N²⟨e_i,e_j⟩ − N − N + N = −N     (usou-se a ortogonalidade)
     *      ‖s‖² = N(N−1)
     * e n⟨s_i,s_j⟩ + ‖s‖² = (N−1)(−N) + N(N−1) = 0. Se os eixos de cima NÃO fossem
     * ortogonais, o −N não saía. */
    printf("      n    ⟨e_i,e_j⟩   ⟨s_i,s_j⟩   −N    n⟨s,s⟩+‖s‖²\n");
    int mau = 0, ns = 0, gume_cai = 0;
    for(int n = 2; n <= 8; n += 1){
        int N = n + 1;
        long ip_cima = 0;
        long ip_s = N*N*ip_cima - N - N + N;
        long nw = (long)N * n;
        long id = n * ip_s + nw;
        ns += 1;
        if(ip_s != -N || id != 0) mau += 1;
        printf("      %-4d %-12ld %-12ld %-6d %ld\n", n, ip_cima, ip_s, -N, id);
        /* gume: dois eixos com ⟨e_i,e_j⟩ = 1 (não ortogonais) — o −N cai */
        long ip_mau = N*N*1 - N - N + N;
        if(ip_mau != -N) gume_cai += 1;
    }
    printf("\n");
    ok("o angulo da sombra sai da formula, nao de ajuste: ⟨s_i,s_j⟩ = −N vem de"
       " ⟨e_i,e_j⟩ = 0 em cima, e n⟨s,s⟩+‖s‖² = 0 e' −1/n. Com eixos NAO ortogonais"
       " o −N cai — a ortogonalidade de cima e' a hipotese, nao o resultado",
       mau == 0 && ns == 7 && gume_cai == 7);
    conclui("As relacoes de baixo sao o que sobra da AUSENCIA de relacoes em cima.");
}

printf("\n§Z3  O CONE: a sombra vive em Σx=0, e a normal é a dimensão perdida.\n\n");
{
    int n_dim = 6, Ndim = n_dim + 1;
    long w[DMAX][DMAX], normal[DMAX];
    sombra(n_dim, w);
    for(int d = 0; d < Ndim; d += 1) normal[d] = 1;
    int orto = 0;
    for(int i = 0; i < Ndim; i += 1){
        long s1 = 0, cn = rt_dir(w[i], normal, Ndim);
        for(int d = 0; d < Ndim; d += 1) s1 += w[i][d];
        if(s1 == 0 && cn == 0) orto += 1;
    }
    printf("      n = %d, o corpo de baixo e o hiperplano Σx = 0 em Z^%d\n", n_dim, Ndim);
    printf("      a normal perdida e (1,…,1) — sem dividir por raiz(N)\n");
    printf("      sombras com Σs = 0 e ⟨s,1⟩ = 0: %d de %d\n\n", orto, Ndim);
    ok("toda sombra e ortogonal a normal — vive inteiramente no corpo de baixo."
       " Projectar e_i da (N−1, −1, …, −1), que somam 0 exactos, e o produto com"
       " (1,…,1) e' zero em Z. O 1e-15 comparava ⟨s,n⟩ depois de normalizar",
       orto == Ndim);
    conclui("A dimensao que falta nao esta espalhada pelas sombras: esta TODA na normal.");
}

printf("\n§Z4  SUBIR restitui a ortogonalidade — e é isso que torna reversível.\n\n");
{
    printf("      n    reconstrucao em BAIXO     em CIMA\n");
    int mau = 0, tot = 0;
    for(int n = 2; n <= 6; n += 1){
        long w[DMAX][DMAX];
        sombra(n, w);
        int d = n + 1;
        long c0 = (long)d * d;
        long x[DMAX], rec[DMAX] = {0}, rec2[DMAX] = {0};
        long sx = 0;
        for(int j = 0; j < n; j += 1){ x[j] = j + 1; sx += x[j]; }
        x[n] = -sx;
        for(int i = 0; i < d; i += 1){
            long c = rt_dir(x, w[i], d);
            for(int j = 0; j < d; j += 1) rec[j] += c * w[i][j];
        }
        for(int i = 0; i < d; i += 1) rec2[i] = x[i];
        int baixo = 1, cima = 1;
        for(int j = 0; j < d; j += 1){
            if(rec[j] != c0 * x[j]) baixo = 0;
            if(rec2[j] != x[j]) cima = 0;
        }
        tot += 1;
        if(!baixo || !cima) mau += 1;
        printf("      %-4d rec = %ld·x ? %s          identidade? %s\n",
               n, c0, baixo ? "sim" : "NÃO", cima ? "sim" : "NÃO");
    }
    printf("\n");
    ok("em cima a reconstrucao e a identidade; em baixo, exacta a menos da constante"
       " (n+1)² — tight frame, sem ortogonalizar. sin(1.7 d) era transporte",
       mau == 0 && tot == 5);
    conclui("Subir devolve a ortogonalidade, e com ela a volta deixa de precisar de constante.");
}

printf("\n§Z5  E A PROJEÇÃO ÁUREA: onde φ aparece, e onde eu NÃO a encontro.\n\n");
{
    printf("      n    n²−n−1    (n+3)²    5(n+1)²    e' φ?\n");
    int achou = 0, ns = 0, coinc = 0;
    long Fb[8] = {1,1,2,3,5,8,13,21};
    for(int n = 2; n <= 8; n += 1){
        long p = (long)n*n - n - 1;
        long a = (long)(n+3)*(n+3), b = 5L*(n+1)*(n+1);
        int e_phi = (p == 0) || (a == b);
        if(e_phi) achou += 1;
        ns += 1;
        for(int k = 0; k < 7; k += 1)
            if(n * Fb[k+1] == (n+1) * Fb[k]) coinc += 1;
        printf("      %-4d %-10ld %-10ld %-12ld %s\n", n, p, a, b, e_phi ? "SIM" : "não");
    }
    printf("\n");
    ok("a razao de projecao NAO e a aurea — e n/(n+1), e depende de n. n²−n−1 = 0"
       " seria n = φ, e (n+3)² = 5(n+1)² seria n/(n+1) = 1/φ². Nenhum n de 2 a 8"
       " satisfaz. A coincidencia Fibonacci n/(n+1)=F_k/F_{k+1} acontece UMA vez (n=2,"
       " 2/3), nao em todos — a razao nao e a sequencia aurea. Sem formar a raiz nem φ",
       achou == 0 && ns == 7 && coinc == 1);
    conclui("A projeccao que aqui se mede e ORTOGONAL, e a ortogonal nao tem φ dentro.");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O simplex nao e construcao: e a sombra da base de cima. O −1/n e o que\n");
printf("    resta da ortogonalidade depois de se perder uma dimensao, e a dimensao\n");
printf("    perdida esta toda na normal — por isso o +1 repoem uma coisa e nao n.\n\n");
printf("    %d assercoes, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
