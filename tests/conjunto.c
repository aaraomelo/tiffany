/* conjunto.c — O ELEMENTO É UM CONJUNTO, NÃO UMA SEQUÊNCIA. E a comparação é INTERSEÇÃO.
 *
 * O Aarão: "vc sempre olha pro símbolo, não pro recipiente onde ele está contido; a topologia
 * invariante está na álgebra — usa conjuntos, usa o corpo lógico."
 *
 * O diagnóstico explica os meus erros do dia de uma vez. Eu codificava SEQUÊNCIA e comparava
 * POSIÇÃO: por isso o comprimento à frente da cifra destruiu o prefixo, e por isso o analisador
 * do mining.notify partiu com a merkle branch e com a aspa escapada. A posição não é invariante —
 * mete-se um símbolo à frente e tudo desloca. O que não desloca é QUEM CONTÉM QUEM.
 *
 * Então um elemento é O CONJUNTO DOS SEUS PREFIXOS:
 *
 *     'ouro'  =  { o, ou, our, ouro }
 *
 * e aí não há primeiro termo, não há comprimento à frente, não há nada a deslocar. A comparação
 * é a INTERSEÇÃO, que é o mo_prod do corpos.h (La Hire, a erosão), e a ordem é a INCLUSÃO.
 *
 * E não é preciso codificar nada de novo: A ÁRVORE JÁ É ESSE CONJUNTO. O caminho de um texto é o
 * conjunto dos seus nós, e o caminho partilhado é a interseção. Eu tinha-a construída desde
 * manhã e estava a lê-la como passeio em vez de como conjunto.
 *
 *   §J1  o elemento é o conjunto dos seus prefixos — e a inclusão ordena
 *   §J2  a comparação é INTERSEÇÃO, e é o mo_prod que já existe
 *   §J3  e é por isso que o comprimento deixa de estorvar
 *   §J4  o complemento é ν, e fecha o corpo lógico
 *
 *   cc -O2 -std=c99 conjunto.c -o conjunto && ./conjunto
 */
#include <stdio.h>
#include <string.h>
#include "corpos.h"
#include "unidade.h"

/* O conjunto dos prefixos, em máscara: o bit k diz "tem prefixo de comprimento k+1".
 * Dois textos partilham o bit k se e só se os primeiros k+1 símbolos coincidem. */
static unsigned pref_conj(const char *s, const char *ref){
    unsigned A = 0;
    for(int k = 0; k < 32 && s[k] && ref[k]; k++){
        if(memcmp(s, ref, (size_t)k + 1)) break;
        A |= 1u << k;
    }
    return A;
}
static int card(unsigned A){ int n = 0; while(A){ n += A & 1; A >>= 1; } return n; }

int main(void){
printf("\n=== O ELEMENTO É UM CONJUNTO — e a comparação é INTERSEÇÃO ================\n");
printf("    A posição não é invariante: mete-se um símbolo à frente e tudo desloca.\n");
printf("    O que não desloca é QUEM CONTÉM QUEM.\n");

printf("\n§J1  O elemento é o conjunto dos seus prefixos, e a INCLUSÃO ordena.\n\n");
{
    const char *a = "ourives", *b = "ourivesaria";
    unsigned A = pref_conj(a, a), B = pref_conj(b, b);
    printf("      'ourives'      %d prefixo(s)\n", card(A));
    printf("      'ourivesaria'  %d prefixo(s)\n", card(B));
    unsigned inter = mo_prod(pref_conj(a, b), pref_conj(b, a));
    printf("      a intersecao tem %d — e 'ourives' esta CONTIDO em 'ourivesaria'\n", card(inter));
    ok("prefixo de outro => o conjunto e SUBCONJUNTO: a inclusao ordena",
       card(inter) == card(A));
    printf("\n      Nao ha primeiro termo, nao ha comprimento a frente, nao ha nada a deslocar.\n");
    printf("      Um conjunto nao tem posicao — e por isso a topologia dele e invariante.\n");
}

printf("\n§J2  A comparação é INTERSEÇÃO — e é o mo_prod que já existe.\n\n");
{
    struct { const char *a, *b; } P[] = {
        {"ourives","ourivesaria"}, {"ouro","ourico"}, {"ouro","prata"}, {"ouro","ouro"},
    };
    printf("      par                        |A∩B|   distancia 1/2^|A∩B|\n");
    long mau = 0;
    for(int i = 0; i < 4; i++){
        unsigned in = mo_prod(pref_conj(P[i].a, P[i].b), pref_conj(P[i].b, P[i].a));
        int c = card(in);
        printf("      %-10s %-14s %-7d 1/%ld\n", P[i].a, P[i].b, c, 1L << c);
        /* a intersecao e simetrica, e e o que uma regua tem de ser */
        unsigned in2 = mo_prod(pref_conj(P[i].b, P[i].a), pref_conj(P[i].a, P[i].b));
        if(in != in2) mau++;
    }
    ok("a intersecao e simetrica — mo_prod(A,B) = mo_prod(B,A)", mau == 0);
    printf("\n      mo_prod E o & — La Hire, a erosao. Nao escrevi comparacao nenhuma: usei a\n");
    printf("      que estava no corpos.h desde sempre.\n");
}

printf("\n§J3  E é por isso que o COMPRIMENTO deixa de estorvar.\n\n");
{
    /* o teste que derrubou a unificacao por sequencia: comprimentos diferentes */
    unsigned in = mo_prod(pref_conj("ourives","ourivesaria"), pref_conj("ourivesaria","ourives"));
    printf("      'ourives' vs 'ourivesaria': |A∩B| = %d, distancia 1/%ld\n", card(in), 1L<<card(in));
    printf("      pela sequencia com o comprimento a frente: partilhavam 1 termo, 1/2\n\n");
    ok("por conjunto, os dois ficam PERTO apesar de tamanhos diferentes", card(in) == 7);
    printf("      A unificacao por sequencia pos o comprimento no termo 1 e mandou duas\n");
    printf("      palavras da mesma familia para a distancia maxima. Por conjunto isso nao\n");
    printf("      pode acontecer: nao ha termo 1.\n");
}

printf("\n§J4  O complemento é ν, e fecha o corpo lógico.\n\n");
{
    unsigned topo = 0xFFu;
    long mau = 0;
    for(unsigned A = 0; A < 256; A++)
        if(mo_nao(mo_nao(A, topo), topo) != A) mau++;
    printf("      ν∘ν = id em %d conjuntos, %ld falhas\n", 256, mau);
    ok("o complemento e involucao — e o mesmo ν da contraposicao, regua (0,-1)", mau == 0);
    /* e a dualidade de De Morgan: o complemento troca uniao por intersecao */
    long dm = 0;
    for(unsigned A = 0; A < 64; A++) for(unsigned B = 0; B < 64; B++){
        unsigned e = mo_nao(A & B, 63), d = mo_nao(A,63) | mo_nao(B,63);
        if(e != d) dm++;
    }
    ok("De Morgan: ν(A∩B) = ν(A)∪ν(B) — o dual troca as duas operacoes", dm == 0);
    printf("\n      Interseccao e uniao sao as duas metades, e ν troca-as. E o chicote outra\n");
    printf("      vez, no corpo logico: a regua (0,-1), a mesma do criativo, do tecnico, do\n");
    printf("      sensitivo, do canal e do fatiamento.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
