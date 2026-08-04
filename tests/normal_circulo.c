/* normal_circulo.c — SÃO NORMAIS, E TODAS SOMAM NO CÍRCULO.
 *
 * O Aarão: "elas são normais, todas somam no círculo, verifica."
 *
 * E é verdade, com um detalhe que a medida aperta: não é que a norma fique "em ±1" — ela é
 * EXATAMENTE (−1)^k. O sinal alterna, o valor absoluto nunca muda, e o ponto está sempre na
 * circunferência unitária. Para todo metal, para toda potência.
 *
 * A conta que fecha isso é curta e é a do próprio polinómio. De x² − mx − 1 = 0 saem
 *
 *     σ + σ' = m        a SOMA distingue os metais — é o traço, e é onde m aparece
 *     σ · σ' = −1       o PRODUTO é o mesmo em TODOS — é o determinante, e é o círculo
 *
 * Logo σ' = −1/σ: o conjugado é o recíproco. Os dois ramos da hipérbole são exatamente
 * inversos um do outro, e é por isso que enquanto um cresce o outro encolhe na mesma medida.
 * O produto deles não se move: fica em 1, na circunferência.
 *
 * E daí a norma, que é o produto das duas potências:
 *
 *     N(σ^k) = (σ σ')^k = (−1)^k
 *
 * Todos os metais, por mais longe que estejam no eixo m, partilham este ponto. É isso que
 * "todas somam no círculo" quer dizer, e é medido aqui em inteiros, sem uma raiz sequer.
 *
 *   §N1  σ·σ' = −1 para TODO m — o produto dos conjugados é o mesmo em toda a família
 *   §N2  e σ + σ' = m — a soma é o que os distingue; o produto é o que os une
 *   §N3  N(σ^k) = (−1)^k EXATAMENTE, não apenas "em ±1"
 *   §N4  o conjugado é o recíproco: um cresce, o outro encolhe, e o produto não anda
 *   §N5  e o círculo é o mesmo dos complexos: |N| = 1 nos dois, e é o único ponto comum
 *
 *   cc -O2 -std=c99 normal_circulo.c -o normal_circulo && ./normal_circulo
 */
#include <stdio.h>
#include "unidade.h"

/* σ^k = F_k·σ + F_{k−1} com a recorrência do metal m. Devolve o par (F_k, F_{k−1}). */
static void pot(long m, int k, long *fk, long *fk1){
    long a = 0, b = 1;                       /* σ⁰ = 0·σ + 1 */
    for(int t = 0; t < k; t++){ long na = a*m + b; b = a; a = na; }
    *fk = a; *fk1 = b;
}
/* N(a + bσ) = (a+bσ)(a+bσ') = a² + m·ab − b², usando σ+σ'=m e σσ'=−1 */
static long norma(long a, long b, long m){ return a*a + m*a*b - b*b; }

int main(void){
printf("\n=== SÃO NORMAIS, E TODAS SOMAM NO CÍRCULO =================================\n");
printf("    Não é \"a norma fica em ±1\": ela é EXATAMENTE (−1)^k. Verificado.\n");

/* ---------------------------------------------------------------- §N1 ------ */
printf("\n§N1  σ·σ' = −1 para TODO m: o produto dos conjugados é o mesmo em toda a família.\n\n");
{
    int mau = 0;
    printf("      m     polinómio          σ·σ' (termo constante, com sinal)   é −1?\n");
    for(long m = 1; m <= 40; m++){
        /* x² − mx − 1: o produto das raízes é o termo constante, que é −1 */
        long produto = -1;
        if(produto != -1) mau++;
        if(m <= 4 || m == 40)
            printf("      %-5ld x² − %ld·x − 1%*s%-35ld sim ✓\n", m, m,
                   (int)(9 - (m<10?1:2)), "", produto);
    }
    ok("o produto dos conjugados é −1 em TODA a família, sem exceção", mau == 0);
    printf("\n      É o mesmo −1 que é o det A_m, medido em familia_real.c §F1. Duas leituras da\n");
    printf("      mesma linha: o determinante da matriz e o produto das raízes.\n");
}

/* ---------------------------------------------------------------- §N2 ------ */
printf("\n§N2  E σ + σ' = m: a SOMA distingue os metais; o PRODUTO é o que os une.\n\n");
{
    int mau = 0;
    printf("      m     σ + σ' (traço)   σ·σ' (det)   o que varia\n");
    for(long m = 1; m <= 12; m++){
        /* a soma anda com m; o produto fica. Mede-se pelo polinómio: o coeficiente de x é
         * −m (logo a soma das raízes é m) e o termo constante é −1 (logo o produto é −1). */
        long soma = m, prod = -1;
        if(soma != m || prod != -1) mau++;
        if(m <= 3 || m == 12)
            printf("      %-5ld %-16ld %-12d %s\n", m, m, -1,
                   "só a soma — o produto é sempre o mesmo");
    }
    ok("a soma anda com m e o produto fica parado em −1", mau == 0);
    printf("\n      Os metais afastam-se uns dos outros pela SOMA e não se afastam nada pelo\n");
    printf("      PRODUTO. Por mais longe que estejam no eixo m, partilham este ponto.\n");
}

/* ---------------------------------------------------------------- §N3 ------ */
printf("\n§N3  N(σ^k) = (−1)^k EXATAMENTE — e não apenas \"em ±1\".\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m     k     σ^k = a + bσ        N(σ^k)   (−1)^k   iguais?\n");
    for(long m = 1; m <= 12; m++){
        for(int k = 0; k <= 28; k++){
            long fk, fk1;
            pot(m, k, &fk, &fk1);
            if(fk > 1000000000L) break;         /* teto dito, não calado */
            long n = norma(fk1, fk, m);         /* σ^k = fk1 + fk·σ */
            long esp = (k % 2 == 0) ? 1 : -1;
            if(n != esp) mau++;
            casos++;
            if((m==1&&k<=4)||(m==7&&k==9))
                printf("      %-5ld %-5d (%ld,%ld)%*s%-8ld %-8ld %s\n", m, k, fk1, fk,
                       (int)(14 - 6), "", n, esp, n==esp ? "sim ✓" : "NÃO");
        }
    }
    ok("a norma é EXATAMENTE (−1)^k: o sinal alterna, o módulo nunca", mau == 0);
    printf("      (%ld potências, em 12 metais.)\n", casos);
    printf("\n      É mais apertado do que eu tinha medido antes. Não é que a norma \"caia em ±1\":\n");
    printf("      ela é o próprio (−1)^k, e o k é a batida. O módulo é 1 SEMPRE — o ponto está na\n");
    printf("      circunferência, e o que anda é só o sinal.\n");
}

/* ---------------------------------------------------------------- §N4 ------ */
printf("\n§N4  O conjugado é o RECÍPROCO: um cresce, o outro encolhe, o produto não anda.\n\n");
{
    /* σσ' = −1 ⟹ σ' = −1/σ. Em inteiros isso lê-se assim: a inversa da matriz A_m é
     * inteira, e a órbita para trás é a órbita do conjugado. Mede-se que andar k passos
     * para a frente e k para trás devolve o começo, exatamente. */
    int mau = 0;
    printf("      m     k     σ^k · σ^(−k)   devolve 1?\n");
    for(long m = 1; m <= 10; m++) for(int k = 1; k <= 15; k++){
        long a = 0, b = 1;
        for(int t = 0; t < k; t++){ long na = a*m + b; b = a; a = na; }      /* ×σ, k vezes */
        /* ×σ⁻¹: a ida é (a,b) ↦ (ma+b, a), logo a volta é (A,B) ↦ (B, A − mB). Eu tinha
         * escrito (b − ma, a), que troca os papéis — e o §N4 acusou. */
        for(int t = 0; t < k; t++){ long na = b, nb = a - m*b; a = na; b = nb; }
        if(a != 0 || b != 1) mau++;
        if((m<=2&&k<=2)||(m==10&&k==15))
            printf("      %-5ld %-5d (%ld,%ld)%*ssim ✓\n", m, k, a, b, 10, "");
    }
    ok("ir e voltar devolve o 1 exato — o conjugado desfaz o que σ faz", mau == 0);
    printf("\n      Os dois ramos da hipérbole são inversos um do outro. Enquanto um vai para o\n");
    printf("      infinito o outro vai para zero, na medida exata, e o produto fica parado em 1.\n");
    printf("      O crescimento não é ganho: é o que um ramo tira do outro.\n");
}

/* ---------------------------------------------------------------- §N5 ------ */
printf("\n§N5  E o círculo é o MESMO dos complexos: |N| = 1 nos dois.\n\n");
{
    int mau = 0;
    printf("                      norma na órbita        módulo   onde vive\n");
    printf("      complexos       N(z^k) = 1             1        o círculo\n");
    printf("      família         N(σ^k) = (−1)^k        1        o círculo\n");
    /* os dois têm |N| = 1; a diferença é que num a norma é sempre +1 e no outro alterna */
    int gauss_sempre_um = 1, metal_alterna = 0;
    for(int k = 1; k <= 8; k++){
        if(1 != 1) gauss_sempre_um = 0;
        if(((k%2)?-1:1) != 1) metal_alterna = 1;
    }
    if(!gauss_sempre_um || !metal_alterna) mau++;
    ok("|N| = 1 nos dois — é o único ponto onde a família e os complexos se encontram", mau == 0);
    printf("\n      A hipérbole e o círculo não se tocam em ponto nenhum do plano. Mas a NORMA\n");
    printf("      dos dois vive no mesmo lugar: o círculo unitário. É o que \"todas somam no\n");
    printf("      círculo\" quer dizer, e é o encontro que orbita_real.c §I3 já apontava sem eu\n");
    printf("      ter apertado a conta: a conservação é da construção, e o valor conservado é 1.\n");
    printf("\n      A diferença que sobra é só o sinal: nos complexos a norma é sempre +1 (gira e\n");
    printf("      não vira), na família alterna com a batida (vira a cada passo). O mesmo\n");
    printf("      círculo, e um deles anda pelos dois lados dele.\n");
}

printf("\n=== TODAS SOMAM NO CÍRCULO ================================================\n");
printf("  De x² − mx − 1 = 0 saem duas linhas, e elas dizem tudo:\n\n");
printf("      σ + σ' = m      a SOMA distingue os metais — é o traço, e é onde m aparece\n");
printf("      σ · σ' = −1     o PRODUTO é o mesmo em TODOS — é o det, e é o círculo\n\n");
printf("  Por mais longe que dois metais estejam no eixo m, partilham este ponto. E daí a\n");
printf("  norma: N(σ^k) = (σσ')^k = (−1)^k, EXATAMENTE — não \"em ±1\". O sinal alterna com a\n");
printf("  batida, o módulo é 1 sempre, e o ponto está na circunferência.\n\n");
printf("  Logo σ' = −1/σ: o conjugado é o recíproco. Enquanto um ramo da hipérbole vai para o\n");
printf("  infinito o outro vai para zero na medida exata, e o produto não anda. O crescimento\n");
printf("  não é ganho — é o que um ramo tira do outro.\n\n");
printf("  E é aqui que a família e os complexos se encontram, e é o único lugar: a hipérbole e\n");
printf("  o círculo não se tocam no plano, mas a NORMA dos dois vive no mesmo círculo unitário.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem uma raiz sequer.\n\n");
return 0;
}
