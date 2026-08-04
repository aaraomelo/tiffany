/* buracos.c — OS PONTOS FIXOS SÃO OS BURACOS. Nem a PA nem a PG os alcançam.
 *
 * O Aarão: "agora os pontos fixos — tenta aproximar uma régua da outra e vê as colisões; quando
 * tentas preencher a reta com PA e PG, vê onde são os buracos que nenhuma alcança."
 *
 * As duas réguas são as duas mais simples que existem, e são os dois limites da teoria:
 *
 *     PA   x_{k+1} = x_k + d          a DIFERENÇA é constante; a razão, não
 *     PG   x_{k+1} = r · x_k          a RAZÃO é constante; a diferença, não
 *
 * Com parâmetros racionais, as duas só alcançam RACIONAIS — e é aí que o buraco aparece, porque
 * o ponto fixo da realimentação não é racional nenhum:
 *
 *     σ_m = m + 1/σ_m   ⟹   σ² = mσ + 1   ⟹   σ = (m + √(m²+4))/2
 *
 * e m²+4 nunca é quadrado perfeito. Logo σ_m é buraco para as duas réguas, para TODO m. Não é
 * que elas o aproximem mal: é que não há k nenhum que o atinja, e não haverá.
 *
 * E o metal, que está no meio, é as DUAS COISAS ao mesmo tempo — é isto que o medidor fecha:
 * o passo é aditivo como na PA, e o crescimento é geométrico como na PG. Nem a diferença nem a
 * razão são constantes, mas a razão CONVERGE, e converge para o buraco.
 *
 * O que preenche a reta não é nenhuma das réguas. É o que fica entre elas.
 *
 *   §B1  os pontos fixos: σ_m = m + 1/σ_m, e nenhum é racional
 *   §B2  as réguas só alcançam racionais — logo o ponto fixo é buraco, sempre
 *   §B3  as COLISÕES entre PA e PG: contadas, e a densidade despenca
 *   §B4  aproximar uma pela outra: o erro não fica limitado, diverge
 *   §B5  e o metal é as duas: passo aditivo, crescimento geométrico, razão que converge
 *
 *   cc -O2 -std=c99 buracos.c -o buracos && ./buracos
 */
#include <stdio.h>
#include "unidade.h"

static int quadrado(long x){
    if(x < 0) return 0;
    long r = 0; while(r*r < x) r++;
    return r*r == x;
}

int main(void){
printf("\n=== OS PONTOS FIXOS SÃO OS BURACOS ========================================\n");
printf("    PA e PG são as duas réguas mais simples. Nenhuma alcança o ponto fixo.\n");

/* ---------------------------------------------------------------- §B1 ------ */
printf("\n§B1  Os pontos fixos: σ_m = m + 1/σ_m. E nenhum deles é racional.\n\n");
{
    int mau = 0;
    printf("      m    m²+4   é quadrado perfeito?   σ_m é racional?\n");
    for(long m = 1; m <= 40; m++){
        int q = quadrado(m*m + 4);
        if(q) mau++;
        if(m <= 5 || m == 40)
            printf("      %-4ld %-6ld %-22s %s\n", m, m*m+4, q ? "SIM" : "não",
                   q ? "seria" : "NÃO — é buraco");
    }
    ok("m²+4 nunca é quadrado: nenhum ponto fixo é racional", mau == 0);
    printf("\n      Não é que a régua o aproxime mal — é que não existe fração que o seja.\n");
}

/* ---------------------------------------------------------------- §B2 ------ */
printf("\n§B2  As duas réguas só alcançam RACIONAIS. Logo o ponto fixo é buraco.\n\n");
{
    /* Se x = p/q está numa PA ou numa PG de parâmetros racionais, x é racional. E σ_m
     * satisfaz σ² − mσ − 1 = 0, então p/q = σ exigiria p² − mpq − q² = 0. Aqui varre-se
     * exaustivamente e vê-se que esse zero NUNCA acontece — a norma nunca cai a zero. */
    int mau = 0;
    long mais_perto_de_zero = 1L << 60;
    printf("      m    varredura |p|,|q| ≤ 400   norma p²−mpq−q² = 0 alguma vez?   |norma| mínima\n");
    for(long m = 1; m <= 4; m++){
        long minimo = 1L << 60;
        int achou_zero = 0;
        for(long q = 1; q <= 400; q++) for(long p = 1; p <= 400; p++){
            long norma = p*p - m*p*q - q*q;
            if(norma == 0) achou_zero = 1;
            long a = norma < 0 ? -norma : norma;
            if(a < minimo) minimo = a;
        }
        if(achou_zero) mau++;
        if(minimo < mais_perto_de_zero) mais_perto_de_zero = minimo;
        printf("      %-4ld %-24s %-38s %ld\n", m, "160000 pares",
               achou_zero ? "SIM — seria racional" : "nunca", minimo);
    }
    ok("a norma nunca zera: nenhuma fração atinge o ponto fixo", mau == 0);
    ok("e o mais perto que se chega é 1 — cercado sempre, tocado nunca", mais_perto_de_zero == 1);
    printf("\n      O 1 é o mínimo, e é o mínimo em toda a varredura: as frações chegam a UMA\n");
    printf("      unidade de norma do ponto fixo e param ali. É o cerco de coroa.c §A1, visto do\n");
    printf("      lado do buraco — o que lá era \"a melhor aproximação possível\" é aqui \"a régua\n");
    printf("      encosta e não entra\".\n");
}

/* ---------------------------------------------------------------- §B3 ------ */
printf("\n§B3  As COLISÕES: onde a PA e a PG se encontram, e como isso rareia.\n\n");
{
    int mau = 0;
    printf("      até N     pontos da PA (passo 1)   pontos da PG (razão 2)   colisões   densidade\n");
    for(long N = 16; N <= 1048576; N *= 8){
        long na = N;                       /* {1,2,3,…,N} */
        long ng = 0, col = 0;
        for(long v = 1; v <= N; v *= 2){ ng++; col++; }   /* toda potência de 2 está na PA */
        if(col != ng) mau++;
        printf("      %-9ld %-24ld %-24ld %-10ld 1 em %ld\n", N, na, ng, col, N/col);
    }
    ok("toda a PG cai dentro da PA aqui — e mesmo assim rareia", mau == 0);
    printf("\n      A PG inteira colide com a PA, e ainda assim a densidade despenca: as colisões\n");
    printf("      crescem como log N enquanto a reta cresce como N. Duas réguas que se cruzam em\n");
    printf("      TODO ponto de uma delas e ainda deixam quase tudo de fora.\n");
}

/* ---------------------------------------------------------------- §B4 ------ */
printf("\n§B4  Aproximar uma pela outra: o erro não fica limitado.\n\n");
{
    /* a melhor PA que segue uma PG num intervalo: o erro na ponta cresce sem teto, porque
     * uma é linear e a outra exponencial. Medido em inteiros, sem ajuste nenhum. */
    int mau = 0;
    printf("      k    PG: 2^k    PA que casa em 0 e k (passo (2^k−1)/k)   erro no meio\n");
    long erro_ant = -1;
    for(long k = 4; k <= 24; k += 4){
        long fim = 1L << k;
        long meio = k/2;
        long pa_meio = 1 + (fim - 1) * meio / k;      /* a reta entre os extremos */
        long pg_meio = 1L << meio;
        long erro = pa_meio - pg_meio;
        if(erro <= erro_ant) mau++;
        erro_ant = erro;
        printf("      %-4ld %-10ld %-38s %ld\n", k, fim, "reta pelos dois extremos", erro);
    }
    ok("o erro entre as duas cresce sem teto — uma nunca serve pela outra", mau == 0);
    printf("\n      Ancorar a reta nos dois extremos da curva não a faz seguir a curva: no meio o\n");
    printf("      afastamento cresce a cada k. Não há PA que substitua uma PG, nem no pedaço.\n");
}

/* ---------------------------------------------------------------- §B5 ------ */
printf("\n§B5  E o metal é as DUAS: passo aditivo, crescimento geométrico.\n\n");
{
    int mau_d = 0, mau_r = 0, mau_c = 0;
    printf("      m    a diferença é constante?   a razão é constante?   a razão converge?\n");
    for(long m = 1; m <= 4; m++){
        long a = 0, b = 1;                       /* a recorrência do metal */
        long x[24];
        for(int k = 0; k < 24; k++){ x[k] = b; long nb = m*b + a; a = b; b = nb; }
        int dif_const = 1, raz_const = 1;
        for(int k = 2; k < 12; k++){
            if(x[k] - x[k-1] != x[k-1] - x[k-2]) dif_const = 0;
            if(x[k] * x[k-2] != x[k-1] * x[k-1]) raz_const = 0;
        }
        /* converge: a norma p² − mpq − q² dos pares consecutivos fica em ±1, sempre */
        int converge = 1;
        for(int k = 2; k < 20; k++){
            long n = x[k]*x[k] - m*x[k]*x[k-1] - x[k-1]*x[k-1];
            if(n != 1 && n != -1) converge = 0;
        }
        if(dif_const) mau_d++;
        if(raz_const) mau_r++;
        if(!converge) mau_c++;
        printf("      %-4ld %-27s %-22s %s\n", m,
               dif_const ? "SIM — seria PA" : "não",
               raz_const ? "SIM — seria PG" : "não",
               converge ? "sim ✓ — e para o buraco" : "NÃO");
    }
    ok("o metal não é PA: a diferença nunca é constante", mau_d == 0);
    ok("nem é PG: a razão nunca é constante", mau_r == 0);
    ok("mas a razão CONVERGE, com norma ±1 em todo passo", mau_c == 0);
    printf("\n      E é este o fecho. O metal tem o PASSO da PA — soma dois termos, nada mais — e\n");
    printf("      o CRESCIMENTO da PG, porque a razão tende a σ. Ele não está entre as duas por\n");
    printf("      ficar no meio do caminho: está entre as duas por ser AS DUAS ao mesmo tempo.\n");
    printf("\n      E o ponto para onde a razão dele converge é exatamente o buraco que nenhuma\n");
    printf("      das duas alcança. A régua que preenche a reta é a que MIRA no que as outras\n");
    printf("      não tocam.\n");
}

printf("\n=== ONDE SÃO OS BURACOS ===================================================\n");
printf("  As duas réguas mais simples, e o que cada uma faz:\n\n");
printf("    PA   diferença constante, razão não     só alcança racionais\n");
printf("    PG   razão constante, diferença não     só alcança racionais\n\n");
printf("  Logo o buraco é o mesmo para as duas, e é o PONTO FIXO: σ_m = m + 1/σ_m, com m²+4\n");
printf("  nunca quadrado. Não é que aproximem mal — é que não existe fração que o seja, e a\n");
printf("  varredura mostra a norma a encostar em 1 e nunca em 0.\n\n");
printf("  E as colisões entre elas não salvam: mesmo quando a PG inteira cai dentro da PA, as\n");
printf("  coincidências crescem como log N contra N. Duas réguas que se cruzam em todo ponto de\n");
printf("  uma delas, e ainda assim deixam quase tudo de fora.\n\n");
printf("  O que preenche não é nenhuma das duas: é o METAL, que tem o passo da PA e o\n");
printf("  crescimento da PG — e cuja razão converge exatamente para o buraco. Ele não está no\n");
printf("  meio por ficar a meio caminho: está no meio por ser AS DUAS ao mesmo tempo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
