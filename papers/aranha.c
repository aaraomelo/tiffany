/*
 * neuronio8 — as 8 leis / base ortonormal de 8
 *
 * Cada bit do byte é uma coordenada da base:
 *
 *     b = b0 e0 + b1 e1 + ... + b7 e7
 *
 * com
 *
 *     <ei,ej> = delta_ij
 *
 * As oito coordenadas são identificadas com as oito leis:
 *
 *     e0 -> Lei 0
 *     e1 -> Lei 1
 *     e2 -> Lei 2
 *     e3 -> Lei 3
 *     e4 -> Lei 4
 *     e5 -> Lei 5
 *     e6 -> Lei 6
 *     e7 -> Lei 7
 *
 * Espectro:
 *
 *     [total,f0,f1,f2,f3,f4,f5,f6,f7]
 *
 * onde fi conta quantos bytes possuem o bit i ligado.
 *
 * As leis:
 *
 *   0  (+1)⊕(-1)                  0† = ∞
 *   1  ν∘ν = id                   Poincaré
 *   2  K** = K                    rotor
 *   3  {-1,0,+1}                  ternário
 *   4  T + T*                     |det| = 1
 *   5  x² = -1                    bit i
 *   6  ⊕ = ⊗                      interface
 *   7  H × H*                     topo / norma
 *
 * IMPORTANTE:
 * A tabela estrutural fornecida não especifica um mapa
 *
 *     Lei i -> Lei j†
 *
 * para i=1,...,7. Portanto o programa NÃO inventa essa
 * correspondência. O dual explícito conhecido é:
 *
 *     Lei 0: 0† = ∞
 */

#include <stdio.h>

static const char *lei_nome[8] = {
    "Lei 0",
    "Lei 1",
    "Lei 2",
    "Lei 3",
    "Lei 4",
    "Lei 5",
    "Lei 6",
    "Lei 7"
};

static const char *lei_operador[8] = {
    "(+1) xor (-1)",
    "nu o nu = id",
    "K** = K; rotor",
    "{-1,0,+1}",
    "T + T*; |det| = 1",
    "x^2 = -1",
    "xor = tensor",
    "H x H*"
};

static const char *lei_leitura[8] = {
    "dois nulos",
    "Poincare",
    "Yang-Mills; P vs NP",
    "Riemann / BSD / Hodge",
    "Navier-Stokes",
    "sem milenio proprio",
    "costura",
    "topo / norma"
};

static const char *lei_chao[8] = {
    "divisao do zero",
    "dualidade H^k <-> H_{n-k}",
    "parada / estatuto",
    "Kronecker; Dirichlet; Lefschetz",
    "Liouville",
    "Z[i]",
    "lcm(2,3) = 6",
    "Hurwitz"
};

static void mostra_leis(void)
{
    puts("");
    puts("=== BASE ORTONORMAL E AS 8 LEIS ===");
    puts("");

    for (int i = 0; i < 8; i++) {
        printf("e%d  <->  %s\n", i, lei_nome[i]);
        printf("       operador : %s\n", lei_operador[i]);
        printf("       leitura  : %s\n", lei_leitura[i]);
        printf("       chao     : %s\n", lei_chao[i]);
        puts("");
    }

    puts("=== DUAL ===");
    puts("");
    puts("Lei 0: 0^dagger = infinito");
    puts("");
    puts("Os demais pares duais nao sao");
    puts("especificados na tabela estrutural.");
    puts("");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;

    FILE *f = fopen(argv[1], "rb");
    if (!f)
        return 1;

    long phase[8] = {0};
    int c;

    while ((c = fgetc(f)) != EOF) {
        unsigned char b = (unsigned char)c;

        /*
         * Projecao do byte na base ortonormal:
         *
         *     e0 e1 e2 e3 e4 e5 e6 e7
         *
         * Cada bit alimenta exatamente uma lei.
         */
        phase[0] += (b >> 0) & 1u;
        phase[1] += (b >> 1) & 1u;
        phase[2] += (b >> 2) & 1u;
        phase[3] += (b >> 3) & 1u;
        phase[4] += (b >> 4) & 1u;
        phase[5] += (b >> 5) & 1u;
        phase[6] += (b >> 6) & 1u;
        phase[7] += (b >> 7) & 1u;
    }

    fclose(f);

    long total = 0;

    for (int i = 0; i < 8; i++)
        total += phase[i];

    /*
     * Espectro bruto.
     */
    printf("[%ld", total);

    for (int i = 0; i < 8; i++)
        printf(",%ld", phase[i]);

    printf("]\n");

    /*
     * Mostra a estrutura somente depois do resultado,
     * para manter a saida principal mecanicamente simples.
     */
    mostra_leis();

    return 0;
}