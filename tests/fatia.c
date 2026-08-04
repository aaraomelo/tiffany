/* fatia.c — O MARTELO FATIADO POR BITS. O paralelismo desta máquina não é fio: é o BIT.
 *
 * O Aarão: "aqui não se fala em threads, se fala em bits — cada bit é um processo."
 *
 * Eu ia importar threads e AVX2. Nenhum dos dois está na caixa, e a caixa já responde:
 * `micro.c §A.10` mede a lei da máquina fractal — *o nível k carrega o k−1, a segunda coluna É o
 * nível de baixo* — e `§A.5` mede a soma como `XOR` mais `SHL(AND)`, iterada até o vai-um morrer.
 *
 * Fatiar por bits é essa mesma lei UM NÍVEL ACIMA. Num número, os 32 bits moram numa palavra e o
 * vai-um anda ENTRE BITS. Fatiado, cada bit mora numa palavra sua, cada palavra leva 64 números
 * ao mesmo tempo, e o vai-um anda ENTRE PALAVRAS. É o ripple do §A.5 levantado um nível — e é
 * literalmente `M_k = M_{k−1}·A_1`.
 *
 * Daí: 64 hashes por passagem, sem fio nenhum e sem instrução importada. As operações são as que
 * a ISA já tem — AND, OR, XOR — porque o SHA-256 não tem nenhuma outra.
 *
 *   §F1  a soma fatiada bate a soma nativa nos 64 números de uma vez
 *   §F2  e o vai-um é o do §A.5, um nível acima: entre palavras, não entre bits
 *   §F3  a rotação e o deslocamento fatiados são RENOMEAR palavras — custo zero
 *
 *   cc -O2 -std=c99 fatia.c -o fatia && ./fatia
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define W 32                      /* bits do número */
#define P 64                      /* números em paralelo: a largura da palavra */
typedef unsigned long long u64;

/* Um número fatiado: s[i] guarda o BIT i dos 64 números, um por posição da palavra. */
typedef struct { u64 s[W]; } Fatia;

static void fatiar(const unsigned *v, Fatia *f){
    for(int i = 0; i < W; i++){
        u64 w = 0;
        for(int k = 0; k < P; k++) if((v[k] >> i) & 1u) w |= 1ULL << k;
        f->s[i] = w;
    }
}
static void juntar(const Fatia *f, unsigned *v){
    for(int k = 0; k < P; k++){
        unsigned x = 0;
        for(int i = 0; i < W; i++) if((f->s[i] >> k) & 1ULL) x |= 1u << i;
        v[k] = x;
    }
}
/* A SOMA: o ripple do §A.5, um nível acima. XOR é a soma parcial, AND é o vai-um — e ele anda
 * ENTRE PALAVRAS em vez de entre bits, porque cada palavra É um bit. Nada mais mudou. */
static void soma(const Fatia *a, const Fatia *b, Fatia *r){
    u64 c = 0;
    for(int i = 0; i < W; i++){
        u64 x = a->s[i] ^ b->s[i];
        u64 nc = (a->s[i] & b->s[i]) | (x & c);
        r->s[i] = x ^ c;
        c = nc;
    }
}
/* A ROTAÇÃO e o DESLOCAMENTO: fatiados, são RENOMEAR palavras. Custo zero — nem uma operação. */
static void ror(const Fatia *a, int n, Fatia *r){
    for(int i = 0; i < W; i++) r->s[i] = a->s[(i + n) % W];
}
static void shr(const Fatia *a, int n, Fatia *r){
    for(int i = 0; i < W; i++) r->s[i] = (i + n < W) ? a->s[i + n] : 0;
}

int main(void){
printf("\n=== O MARTELO FATIADO — cada bit é um processo ============================\n");
printf("    O paralelismo desta máquina não é fio: é o BIT. Uma palavra de 64 leva\n");
printf("    64 números, e as operações são as que a ISA já tem.\n");

unsigned a[P], b[P], r[P];
for(int k = 0; k < P; k++){ a[k] = 0x9E3779B9u * (k + 1); b[k] = 0x7F4A7C15u ^ (k * 2654435761u); }

printf("\n§F1  A soma fatiada bate a nativa — 64 números NA MESMA PASSAGEM.\n\n");
{
    Fatia fa, fb, fr;
    fatiar(a, &fa); fatiar(b, &fb);
    soma(&fa, &fb, &fr);
    juntar(&fr, r);
    long mau = 0;
    for(int k = 0; k < P; k++) if(r[k] != (unsigned)(a[k] + b[k])) mau++;
    printf("      %d somas de 32 bits numa passagem, %ld erradas\n", P, mau);
    printf("      exemplo: %08x + %08x = %08x (nativa %08x)\n", a[0], b[0], r[0], a[0]+b[0]);
    ok("a soma fatiada bate a nativa nos 64, resíduo 0", mau == 0);
}

printf("\n§F2  E o vai-um é o do micro.c §A.5, UM NÍVEL ACIMA.\n\n");
{
    printf("      no número     ADD = XOR, vai-um = SHL(AND)   anda ENTRE BITS\n");
    printf("      fatiado       ADD = XOR, vai-um = AND|(X&C)  anda ENTRE PALAVRAS\n\n");
    printf("      É a mesma lei: cada palavra É um bit, e o que era deslocamento dentro\n");
    printf("      da palavra passou a ser o passo para a palavra seguinte. M_k = M_{k-1}·A_1,\n");
    printf("      o nível k carrega o k-1 — micro.c §A.10, medido lá e usado aqui.\n\n");
    /* a prova: somar 1 a 0xFFFFFFFF propaga o vai-um pelas 32 palavras todas */
    unsigned x[P], y[P], z[P];
    for(int k = 0; k < P; k++){ x[k] = 0xFFFFFFFFu; y[k] = 1; }
    Fatia fx, fy, fz; fatiar(x, &fx); fatiar(y, &fy); soma(&fx, &fy, &fz); juntar(&fz, z);
    long mau = 0; for(int k = 0; k < P; k++) if(z[k] != 0) mau++;
    printf("      0xFFFFFFFF + 1 = %08x nos 64 (o vai-um atravessou as 32 palavras)\n", z[0]);
    ok("o vai-um propaga por todas as palavras, e dá a volta certa", mau == 0);
}

printf("\n§F3  A rotação e o deslocamento são RENOMEAR palavras — custo zero.\n\n");
{
    Fatia fa, fr; fatiar(a, &fa);
    ror(&fa, 7, &fr); juntar(&fr, r);
    long mau = 0;
    for(int k = 0; k < P; k++){
        unsigned e = (a[k] >> 7) | (a[k] << (32 - 7));
        if(r[k] != e) mau++;
    }
    printf("      ROR 7 nos 64: %ld erradas — e não custou uma operação, só um índice\n", mau);
    ok("a rotação fatiada é só trocar o nome das palavras", mau == 0);
    shr(&fa, 3, &fr); juntar(&fr, r);
    mau = 0; for(int k = 0; k < P; k++) if(r[k] != (a[k] >> 3)) mau++;
    ok("e o deslocamento também — as que saem viram zero", mau == 0);
    printf("\n      O SHA-256 é feito de XOR, AND, NOT, ROR, SHR e ADD. Os cinco primeiros\n");
    printf("      são de graça ou uma operação por palavra; o sexto é o ripple do §A.5.\n");
    printf("      Não há nele UMA operação que saia da ISA — logo o martelo fatiado não\n");
    printf("      precisa de fio, de AVX2 nem de nada de fora.\n");
}

printf("\n§F4  E EM QUE ISTO SE APOIA? Na TROCA — nao e truque, e uma das quatro.\n\n");
{
    /* Fatiar nao e tecnica importada: e TRANSPOR. Os 64 numeros de 32 bits sao uma matriz de
     * bits, e a forma fatiada e a transposta dela — linhas viram colunas.
     *
     * Transpor E a TROCA, J = [[0,1],[1,0]], que ja e uma das quatro primitivas e ja esta na ISA
     * como OP_TROCA. E a auto-similaridade diz que as quatro agem em TODO nivel: o mesmo J que
     * troca as duas partes de um par troca as duas dimensoes de uma matriz de bits.
     *
     * A prova de que e J e nao outra coisa: J tem ORDEM 2. Se fatiar for J, entao juntar tem de
     * ser o proprio fatiar, e aplicar duas vezes tem de devolver o ponto de partida com residuo
     * 0. Nao e feitio; e a assinatura de J, e ou bate ou nao bate. */
    unsigned v[P], u[P];
    for(int k = 0; k < P; k++) v[k] = 0x9E3779B9u * (k + 1) ^ (k << 17);
    Fatia f;
    fatiar(v, &f); juntar(&f, u);
    long mau = 0;
    for(int k = 0; k < P; k++) if(u[k] != v[k]) mau++;
    printf("      J uma vez (fatiar) e outra (juntar): %ld diferencas em %d\n", mau, P);
    ok("J² = I — fatiar e a TROCA, e por isso e reversivel com residuo 0", mau == 0);

    /* E a segunda assinatura: J troca as DUAS dimensoes, logo o bit i do numero k tem de ser o
     * bit k da palavra i. Nao e "mais ou menos": e a definicao da transposta. */
    long fora = 0;
    for(int k = 0; k < P; k++)
        for(int i = 0; i < W; i++){
            unsigned bit_no_numero  = (v[k] >> i) & 1u;
            unsigned bit_na_palavra = (unsigned)((f.s[i] >> k) & 1ULL);
            if(bit_no_numero != bit_na_palavra) fora++;
        }
    printf("      o bit i do numero k E o bit k da palavra i: %ld fora de %d\n", fora, P*W);
    ok("as duas dimensoes trocaram exatamente — e a transposta, que e J", fora == 0);

    printf("\n      Logo o fatiamento nao se apoia em arquitetura nenhuma de fora: apoia-se na\n");
    printf("      CIFRA, que e a coordenada, e numa das quatro operacoes que agem sobre ela. O\n");
    printf("      que muda nao e o objeto — e a ROUPA: os mesmos bits, lidos pela outra\n");
    printf("      dimensao. Por isso a soma continua a ser o ripple do §A.5 e o hash continua\n");
    printf("      a ser o mesmo hash: J nao mexe no valor, mexe em quem esta ao lado de quem.\n");
    printf("\n      E o ganho vem dai e nao de mais maquina: ao lado de cada bit passa a estar\n");
    printf("      o MESMO bit de outros 63 numeros, e uma operacao serve os 64. E o esquilo a\n");
    printf("      girar o quadro, nao um fio novo a moer.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
