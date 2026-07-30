/* transformada.c — A TRANSFORMADA UNIVERSAL, e por que ela serve de teste.
 *
 * A definição está no enredo (Reino Dourado, "A primeira operação"), e é esta:
 *
 *     F(x)_k = (1/√n) Σ_j x_j χ_k(j),      F⁻¹(X)_j = (1/√n) Σ_k X_k χ_{−k}(j)
 *
 * com a normalização 1/√n em CADA lado, e não por gosto: somar a órbita inteira dá
 *
 *     Σ_k χ_k(j) χ_{−k}(j') = n · δ_{j,j'}          ← O PONTO
 *
 * Essa soma É o Dirac. Os caracteres são a órbita da torção; somados sobre a órbita toda,
 * colapsam num ponto. É isto que o Aarão chama de furar no infinito e trazer a amostra: a
 * órbita não termina, e mesmo assim a soma dela cabe num ponto exato.
 *
 * E a transformada não se define — CAI DO DUAL. O dual do dual devolve o grupo (medido em
 * dualidade.c §P5), então emparelhar um objeto com um caractere não é construção a mais: é a
 * avaliação dos caracteres. Trocar o grupo troca a transformada sem trocar a conta.
 *
 * POR QUE SERVE DE TESTE. A bateria é um vetor de resíduos — uma entrada por medidor, 0 quando
 * verde. "erro 0" é esse vetor ser nulo. E como a transformada é UNITÁRIA, nenhuma medida vaza:
 *
 *     ‖x‖² = ‖F x‖²      ⟹      x = 0  ⟺  F x = 0
 *
 * Logo o SELO pode ser a norma: um número só, que é zero exatamente quando tudo está verde. E
 * quando não é zero, o DIRAC localiza — a volta concentra o espalhado de novo num ponto, e o
 * ponto diz qual casa abrir. Testa-se com um número; só se abre a casa que o ponto apontou.
 *
 * Aqui o grupo é (Z/2)^m — que é o que a bateria é: componentes binárias independentes, uma por
 * medidor, a soma direta do trio.c §S4. Nesse grupo os caracteres valem ±1, então TUDO isto é
 * conta inteira, sem um único float. E há uma consequência que eu não esperava, no §U2.
 *
 *   §U1  O PONTO: Σ_k χ_k(j)χ_k(j') = n·δ — a órbita inteira cabe num ponto
 *   §U2  F² = n·id AQUI: em característica 2 a reflexão É a identidade, e o período cai de 4 a 2
 *   §U3  unitária: ‖Fx‖² = n‖x‖², exato em inteiros — nenhuma medida vaza
 *   §U4  O TESTE: x = 0 ⟺ Fx = 0, e o selo é a norma
 *   §U5  fura e traz a amostra: o delta espalha-se em tudo, e a volta concentra no ponto
 *
 *   cc -O2 -std=c99 transformada.c -o transformada && ./transformada
 */
#include <stdio.h>

#include "unidade.h"
/* o caractere do grupo (Z/2)^m: χ_k(j) = (−1)^(bits comuns). ±1, inteiro. */
static int chi(long k, long j){
    long b = k & j, p = 0;
    while(b){ p ^= (b & 1); b >>= 1; }
    return p ? -1 : 1;
}

int main(void){
printf("\n=== A TRANSFORMADA UNIVERSAL, E POR QUE ELA SERVE DE TESTE =================\n");
printf("    F(x)_k = (1/√n) Σ_j x_j χ_k(j).  Os caracteres são a órbita da torção.\n");

/* ---------------------------------------------------------------- §U1 ------ */
printf("\n§U1  O PONTO: somar a órbita inteira colapsa num Dirac.\n\n");
{
    int mau = 0;
    printf("      n     Σ_k χ_k(j)χ_k(j') = n quando j=j'   e 0 quando j≠j'   pares\n");
    for(int m = 1; m <= 7; m++){
        long n = 1L << m, pares = 0;
        int bom = 1;
        for(long j = 0; j < n; j++) for(long jl = 0; jl < n; jl++){
            long s = 0;
            for(long k = 0; k < n; k++) s += chi(k,j) * chi(k,jl);
            long esperado = (j == jl) ? n : 0;
            if(s != esperado) bom = 0;
            pares++;
        }
        if(!bom) mau++;
        printf("      %-5ld %36s %19s   %ld\n", n, bom?"sim ✓":"NÃO", bom?"sim ✓":"NÃO", pares);
    }
    ok("a órbita inteira soma exatamente n·δ — o Dirac é medido", mau == 0);
    printf("\n      É aqui que a transformada fura o infinito e traz a amostra: a órbita dos\n");
    printf("      caracteres não termina em nada de finito, e mesmo assim a soma dela cabe num\n");
    printf("      ponto exato. Não é aproximação de ponto — É o ponto, sem resto.\n");
}

/* ---------------------------------------------------------------- §U2 ------ */
printf("\n§U2  F² = n·id aqui: em característica 2 a reflexão É a identidade.\n\n");
{
    int mau_2 = 0, mau_4 = 0;
    printf("      n     F²(x) = n·x   F⁴(x) = n²·x   período com normalização\n");
    for(int m = 1; m <= 6; m++){
        long n = 1L << m;
        long x[64], f1[64], f2[64], f3[64], f4[64];
        for(long j = 0; j < n; j++) x[j] = ((j*7+3) % 11) - 5;
        for(long k = 0; k < n; k++){ f1[k]=0; for(long j=0;j<n;j++) f1[k] += x[j]*chi(k,j); }
        for(long k = 0; k < n; k++){ f2[k]=0; for(long j=0;j<n;j++) f2[k] += f1[j]*chi(k,j); }
        for(long k = 0; k < n; k++){ f3[k]=0; for(long j=0;j<n;j++) f3[k] += f2[j]*chi(k,j); }
        for(long k = 0; k < n; k++){ f4[k]=0; for(long j=0;j<n;j++) f4[k] += f3[j]*chi(k,j); }
        int e2 = 1, e4 = 1;
        for(long j = 0; j < n; j++){ if(f2[j] != n*x[j]) e2 = 0; if(f4[j] != n*n*x[j]) e4 = 0; }
        if(!e2) mau_2++;
        if(!e4) mau_4++;
        printf("      %-5ld %13s %14s   %s\n", n, e2?"sim ✓":"NÃO", e4?"sim ✓":"NÃO",
               e2 ? "2 (não 4)" : "?");
    }
    ok("F² devolve o que entrou, a menos de n — o período é 2 aqui", mau_2 == 0);
    ok("e F⁴ também, a menos de n² — 4 continua a valer, mas não é o menor", mau_4 == 0);
    printf("\n      O enredo diz F² = reflexão e F⁴ = id, e está certo no geral. Neste grupo a\n");
    printf("      reflexão j ↦ −j É a identidade, porque −j = j em característica 2 — e então o\n");
    printf("      período CAI DE 4 PARA 2: a transformada é a sua própria inversa.\n");
    printf("\n      É La Hire outra vez, e no mesmo lugar de sempre: r=2, a involução, a ida que\n");
    printf("      É a volta. O grupo binário é exatamente onde o hipociclo degenera em reta.\n");
}

/* ---------------------------------------------------------------- §U3 ------ */
printf("\n§U3  UNITÁRIA: ‖Fx‖² = n‖x‖², exato em inteiros. Nenhuma medida vaza.\n\n");
{
    int mau = 0;
    printf("      n     ‖x‖²   ‖Fx‖²   n·‖x‖²   confere\n");
    for(int m = 1; m <= 6; m++){
        long n = 1L << m;
        long x[64], f[64], nx = 0, nf = 0;
        for(long j = 0; j < n; j++){ x[j] = ((j*13+5) % 9) - 4; nx += x[j]*x[j]; }
        for(long k = 0; k < n; k++){ f[k]=0; for(long j=0;j<n;j++) f[k] += x[j]*chi(k,j); nf += f[k]*f[k]; }
        if(nf != n*nx) mau++;
        printf("      %-5ld %6ld %7ld %8ld   %s\n", n, nx, nf, n*nx, nf==n*nx?"✓":"✗");
    }
    ok("a norma atravessa: o que entra é o que sai, sem sobra nem excesso", mau == 0);
    printf("\n      É o Princípio da Medida Consistente lido nos coeficientes: decompor um objeto\n");
    printf("      nos pedaços da unidade e somá-los reconstitui o objeto. Nada vaza na decomposição\n");
    printf("      — e por isso a ida-e-volta é exata, não aproximada.\n");
}

/* ---------------------------------------------------------------- §U4 ------ */
printf("\n§U4  O TESTE: x = 0 ⟺ Fx = 0. O selo é a norma, e ela é zero só se tudo é verde.\n\n");
{
    int mau_z = 0, mau_n = 0;
    printf("      n     x todo 0 → ‖Fx‖²   um só medidor vermelho → ‖Fx‖²   detecta?\n");
    for(int m = 2; m <= 6; m++){
        long n = 1L << m;
        long x[64], f[64], nf0 = 0, nf1 = 0;
        /* a bateria toda verde: resíduo 0 em cada casa */
        for(long j = 0; j < n; j++) x[j] = 0;
        for(long k = 0; k < n; k++){ f[k]=0; for(long j=0;j<n;j++) f[k] += x[j]*chi(k,j); nf0 += f[k]*f[k]; }
        /* uma casa vermelha, e uma só — o pior caso para um selo global */
        x[n/3] = 1;
        for(long k = 0; k < n; k++){ f[k]=0; for(long j=0;j<n;j++) f[k] += x[j]*chi(k,j); nf1 += f[k]*f[k]; }
        if(nf0 != 0) mau_z++;
        if(nf1 == 0) mau_n++;      /* se não distinguisse, o selo seria cego */
        printf("      %-5ld %17ld %33ld   %s\n", n, nf0, nf1, nf1?"sim ✓":"CEGO ✗");
    }
    ok("verde total dá selo 0 — e só o verde total dá", mau_z == 0);
    ok("uma única casa vermelha já move o selo: ele não é cego", mau_n == 0);
    printf("\n      É o teste inteiro num número. Não é preciso abrir casa nenhuma para saber que\n");
    printf("      há erro: a norma do transformado é zero se e só se todo resíduo é zero. E ela\n");
    printf("      não pode ser enganada por cancelamento, porque é soma de quadrados.\n");
}

/* ---------------------------------------------------------------- §U5 ------ */
printf("\n§U5  FURA E TRAZ A AMOSTRA: o delta espalha-se em tudo, e a volta concentra.\n\n");
{
    int mau_e = 0, mau_v = 0;
    printf("      n     F(δ_j) é ±1 em TODO k   e a volta devolve n·δ_j   localiza?\n");
    for(int m = 2; m <= 6; m++){
        long n = 1L << m;
        int espalha = 1, volta = 1;
        for(long j = 0; j < n; j++){
            long d[64], f[64], g[64];
            for(long t = 0; t < n; t++) d[t] = (t == j);
            /* ida: o ponto vira a órbita inteira — não sobra nenhum k em zero */
            for(long k = 0; k < n; k++){ f[k]=0; for(long t=0;t<n;t++) f[k] += d[t]*chi(k,t); }
            for(long k = 0; k < n; k++) if(f[k] != 1 && f[k] != -1) espalha = 0;
            /* volta: a órbita concentra-se de novo, e no MESMO ponto */
            for(long t = 0; t < n; t++){ g[t]=0; for(long k=0;k<n;k++) g[t] += f[k]*chi(k,t); }
            for(long t = 0; t < n; t++) if(g[t] != (t==j ? n : 0)) volta = 0;
        }
        if(!espalha) mau_e++;
        if(!volta) mau_v++;
        printf("      %-5ld %22s %26s   %s\n", n, espalha?"sim ✓":"NÃO",
               volta?"sim ✓":"NÃO", volta?"sim ✓":"NÃO");
    }
    ok("o ponto vira a órbita inteira — nenhum k fica de fora", mau_e == 0);
    ok("e a órbita volta a ser o MESMO ponto, exatamente", mau_v == 0);
    printf("\n      É a mecânica do teste, nos dois sentidos. Um erro numa casa só espalha-se por\n");
    printf("      todo o selo — por isso o selo o vê. E a volta concentra o espalhado de novo\n");
    printf("      naquela casa — por isso o selo diz QUAL abrir, e só ela.\n");
}

printf("\n=== O QUE ISTO DÁ À BATERIA ===============================================\n");
printf("  Um selo e um ponteiro, e nenhum dos dois abre casa à toa:\n\n");
printf("    O SELO      ‖Fx‖², um número. Zero se e só se todo resíduo é zero, e imune a\n");
printf("                cancelamento porque é soma de quadrados. Enquanto ele for 0, não há\n");
printf("                nada a abrir — e é isso que dispensa rodar os 67.\n\n");
printf("    O PONTEIRO  quando o selo se move, a volta concentra o espalhado na casa que\n");
printf("                mudou. Abre-se essa, e só essa.\n\n");
printf("  E o que autoriza os dois é a mesma coisa: a transformada é unitária, então nenhuma\n");
printf("  medida vaza entre os dois lados. O selo não é resumo do teste — é o teste, visto do\n");
printf("  outro lado do espelho.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
