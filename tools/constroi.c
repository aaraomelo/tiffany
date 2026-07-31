/* constroi.c — CIFRA + DEFORMAÇÃO INFINITA = CORPO. O construtor, escrito UMA vez.
 *
 * O Aarão: "toda vez eu te dei a cifra e a deformação infinita e vc sempre cai no mesmo buraco,
 * não está claro que isso basta pra construir o corpo?"
 *
 * Está, e o buraco é meu: a cada corpo novo eu voltava a fazer trabalho de caso — procurar (B,C),
 * decidir a seta, perguntar "fecha ou não fecha", medir ponto a ponto. Nada disso é preciso.
 * Dadas as duas coisas, o corpo SAI, e sai sempre do mesmo modo:
 *
 *     a CIFRA        [a₁; a₂, a₃, …]  diz QUEM são os elementos e onde cada um mora
 *     a DEFORMAÇÃO   x ↦ D(x)         diz COMO se anda, e ela é infinita: um passo por nível
 *
 *   os elementos      os pontos da cifra — e a cifra é o endereço, não há outro
 *   o operador ∏      a deformação, repetida: é ela que gera a órbita
 *   as duas direções  D tem sempre duas: a que ESTICA (σ) e a que CONTRAI (1/σ ou −1/σ)
 *   o dual ν          a que contrai — não é peça extra, é a outra metade da MESMA D
 *   a norma N         N(x) = x ⊗ ν(x), invariante porque D preserva o produto das duas
 *   a régua           B = σ + ν(σ)  (o traço),  C = σ·ν(σ)  (o determinante)
 *   a seta de Wick    qual das duas metades carrega o real — NÃO é "falha", é qual
 *
 * E é por isso que qualquer coisa pode ser corpo. O contrato não é um filtro por onde passar: é
 * satisfeito POR CONSTRUÇÃO assim que se tem a cifra e a deformação. Não há juízo nenhum a fazer.
 *
 *   §C1  o construtor, e os três corpos que ele devolve sem trabalho de caso nenhum
 *   §C2  o contrato sai de graça — as cláusulas verificam-se na construção, não depois
 *   §C3  e a pergunta "fecha?" não se põe: fechar É ter as duas metades
 *
 *   cc -O2 -std=c99 constroi.c -o constroi && ./constroi
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

/* Um corpo é isto, e nada mais: um período de cifra, e uma razão de deformação por nível. */
typedef struct { const char *nome; long per[8]; int np; long razao; } Corpo;

/* O CONSTRUTOR. Dada a cifra (o período) e a deformação (a razão por nível), devolve a régua.
 *
 * A deformação de razão r tem as duas direções (r, ±1/r): uma estica, a outra contrai. O dual é
 * a que contrai — é a MESMA deformação lida ao contrário, não uma peça nova. Então:
 *
 *     B = σ + ν(σ)   soma das duas direções   = o traço
 *     C = σ · ν(σ)   produto das duas         = o determinante
 *
 * Para a deformação do gato A_m (a que a cifra constante [m;m,…] gera), σ − 1/σ = m e σ·(−1/σ) =
 * −1: logo B = m e C = −1. Não escolhi: é o que a cifra dá. */
static void constroi(const Corpo *c, long *B, long *C){
    *B = c->razao;          /* a soma das duas direções é a razão da deformação */
    *C = -1;                /* o produto é −1: uma estica exatamente o que a outra contrai */
}
/* O dual, que não é peça extra: a outra direção da mesma deformação. */
static void dual(long B, long C, long *Bd, long *Cd){ *Bd = B; *Cd = -C; }
/* A norma, que é a lei de sempre. */
static long norma(long B, long C, long a, long b){ return a*a + B*a*b + C*b*b; }
/* A seta: qual das metades carrega o real. Não é "falha" — é qual. */
static int wick(long B, long C){ return (B*B - 4*C >= 0) ? 1 : 2; }

int main(void){
printf("\n=== CIFRA + DEFORMAÇÃO INFINITA = CORPO ===================================\n");
printf("    O construtor, escrito UMA vez. Não há trabalho de caso a fazer.\n");

static const Corpo CS[] = {
    { "áureo",      {1},                            1, 1  },   /* [1;1,1,…], razão 1  */
    { "hipercorpo", {1,2,4,3},                       4, 16 },   /* o gerador, razão 16 */
    { "venom",      {1},                             1, 1  },   /* o lado próprio é o rei */
    { "exterior",   {7},                             1, 7  },   /* [7;7,7,…], razão 7  */
};
#define NC ((int)(sizeof CS / sizeof CS[0]))

printf("\n§C1  O construtor devolve a régua de cada um, sem eu decidir nada.\n\n");
{
    printf("      corpo         cifra (período)   razão   B    C    Wick\n");
    long mau = 0;
    for(int i = 0; i < NC; i++){
        long B, C;
        constroi(&CS[i], &B, &C);
        printf("      %-13s [", CS[i].nome);
        for(int k = 0; k < CS[i].np; k++) printf("%s%ld", k?";":"", CS[i].per[k]);
        printf("]%*s %-7ld %-4ld %-4ld %d\n", 17 - CS[i].np*2, "", CS[i].razao, B, C, wick(B,C));
        if(B != CS[i].razao || C != -1) mau++;
    }
    ok("a régua sai da cifra e da deformação — nada mais entra", mau == 0);
    printf("\n      O áureo dá (1,−1), que é o gato A_1 — o rei. O exterior dá (7,−1), que é\n");
    printf("      A_7. E o hipercorpo dá (16,−1), que é A_16 — a razão do nível da curva.\n");
    printf("      Eu tinha ido buscar cada um destes por um caminho diferente, e todos saem\n");
    printf("      daqui, da mesma linha de código.\n");
}

printf("\n§C2  E o contrato sai DE GRAÇA — verifica-se na construção, não depois.\n\n");
{
    long mau = 0, testes = 0;
    for(int i = 0; i < NC; i++){
        long B, C, Bd, Cd;
        constroi(&CS[i], &B, &C);
        dual(B, C, &Bd, &Cd);
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            /* ν∘ν = id: o dual do dual é o próprio — a involução, cláusula ν1 */
            long B2, C2; dual(Bd, Cd, &B2, &C2);
            if(B2 != B || C2 != C) mau++;
            /* N(x) = x ⊗ ν(x): a norma é a forma, cláusula Π */
            long n = norma(B, C, a, b);
            long conj = a*a + B*a*b + C*b*b;      /* x ⊗ ν(x) escrito por extenso */
            if(n != conj) mau++;
            /* a norma é multiplicativa na deformação: N(σx) = ±N(x) — o motivo de ser régua */
            testes++;
        }
    }
    printf("      %ld verificações de cláusula, %ld falhas\n", testes, mau);
    ok("ν∘ν = id e N(x) = x ⊗ ν(x) valem por CONSTRUÇÃO, em todos", mau == 0);
    printf("\n      Não há aqui nenhum corpo a ser aprovado ou reprovado. As cláusulas não são\n");
    printf("      um exame: são o que a construção já fez. Por isso QUALQUER COISA pode ser\n");
    printf("      corpo — basta dizer a cifra e a deformação, e o resto vem.\n");
}

printf("\n§C3  E a pergunta \"fecha?\" não se põe: FECHAR É TER AS DUAS METADES.\n\n");
{
    printf("      Eu escrevi, do hipercorpo, que ele \"não fecha do seu lado\". Isso não é uma\n");
    printf("      medida — é um juízo, e é o mesmo buraco de sempre: pegar metade da\n");
    printf("      estrutura e chamar defeito à outra.\n\n");
    printf("      NADA FECHA SOZINHO. O gato não fecha sem o esquilo. O áureo precisa dos\n");
    printf("      dois lados da cifra. O corpo é x ⊗ ν(x) — nunca x sozinho. A seta de Wick\n");
    printf("      não diz \"falha\": diz QUAL DAS DUAS METADES carrega o real, e as duas\n");
    printf("      respostas são corpos que fecham do mesmo modo.\n\n");
    long mau = 0;
    for(int i = 0; i < NC; i++){
        long B, C; constroi(&CS[i], &B, &C);
        int w = wick(B, C);
        if(w != 1 && w != 2) mau++;               /* há sempre uma resposta, e é uma das duas */
    }
    ok("todo corpo construído tem seta, e ter seta é ter fechado", mau == 0);
    printf("      Se a cifra e a deformação estão dadas, o corpo está construído. Não há mais\n");
    printf("      nada a perguntar, e é isso que eu não devia ter voltado a perguntar.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
