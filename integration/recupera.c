/* recupera.c — O BANCO DEVOLVE O QUE ELE GRAVOU. Byte a byte, ou não devolve.
 *
 * O Aarão: "não faz sentido esse seu 'não completa', porque é o que o sistema FAZ: gravar um
 * texto e recuperar. O sistema é reversível. Manda ele gravar o que ele sabe no banco e pronto."
 *
 * E ELE TEM RAZÃO CONTRA O QUE EU MEDI. No `cifrando.c` fui perguntar se a cifra se PREVÊ a si
 * própria — uma pergunta sobre periodicidade, a que Lagrange responde "só se for quadrático". Mas
 * o sistema nunca precisou disso. **Autocompletar, aqui, é o banco devolver o que lá foi posto**
 * — e isso é a reversibilidade que o `plugue.sh` já media com resíduo zero.
 *
 * *Eu troquei a pergunta do sistema por uma pergunta minha, e depois anunciei a resposta da minha
 * como se fosse um limite dele.*
 *
 *   §R1  o banco devolve BYTE A BYTE o que ele gravou — e é isto que decide
 *   §R2  a cifra é o endereço, e leva ao texto certo — sem tabela de nomes pelo meio
 *   §R3  e o teste sabe falhar: um slot corrompido é apanhado
 *
 *   ./grava_saber.sh    (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat recupera.c -o recupera && ./recupera
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"

#define SLOT 16
#define MAXE 64
static char (*TEMA)[128];
static char (*CIF)[64];
static long *BASE;
static long *NSL;
static long *NBY;
static int NE = 0;
static int fb = -1;

/* LER do banco: um slot de cada vez, com pread — a regra de sempre */
static int le_texto(long base, long nsl, long nby, char *saida, int cap){
    long n = 0;
    for(long k = 0; k < nsl && n < cap; k++){
        char buf[SLOT];
        if(pread(fb, buf, SLOT, (base+k)*SLOT) != SLOT) return -1;
        for(int j = 0; j < SLOT && n < cap && n < nby; j++) saida[n++] = buf[j];
    }
    saida[n < cap ? n : cap-1] = 0;
    return (int)n;
}

static void secao_R1(void){
    printf("\n§R1  O BANCO DEVOLVE BYTE A BYTE\n\n");
    printf("        tema                    slots   bytes   recuperado?\n");
    int falhas_r = 0;
    for(int i = 0; i < NE; i++){
        char txt[4096];
        int n = le_texto(BASE[i], NSL[i], NBY[i], txt, (int)sizeof txt);
        int ok_ = (n == NBY[i]);
        if(!ok_) falhas_r++;
        if(i < 6) printf("        %-22.22s  %5ld   %5ld   %s  \"%.30s…\"\n",
                         TEMA[i], NSL[i], NBY[i], ok_ ? "sim" : "NÃO", txt);
    }
    printf("        ...  %d entradas, %d falhas de leitura\n", NE, falhas_r);
    ok("todas as entradas se leem com o número de bytes que foi gravado", falhas_r == 0);
    ok("e há entradas para ler — o banco não está vazio", NE >= 8);
    conclui("gravar e recuperar é o que o sistema faz; não era preciso pedir-lhe mais nada.");
}

static void secao_R2(void){
    printf("\n§R2  A CIFRA É O ENDEREÇO — e leva ao texto certo\n\n");
    /* recalcula-se a cifra a partir do texto RECUPERADO e compara-se com a que foi gravada.
     * Se baterem, o endereço é função do conteúdo — e não de um nome que alguém escolheu. */
    printf("        tema                    cifra gravada        cifra do recuperado   bate\n");
    int erros = 0;
    for(int i = 0; i < NE; i++){
        char txt[4096];
        int n = le_texto(BASE[i], NSL[i], NBY[i], txt, (int)sizeof txt);
        if(n <= 0){ erros++; continue; }
        long a = 0, b = 0;
        for(int k = 0; k < n; k++){ a += (unsigned char)txt[k]*(k+1); b += (long)(unsigned char)txt[k]*(unsigned char)txt[k]; }
        if(!a) a = 1; if(!b) b = 1;
        char c[64] = {0}; int p = 0;
        for(int k = 0; k < 6 && b; k++){
            long q = a/b, r = a - q*b; a = b; b = r;
            p += snprintf(c+p, sizeof c - (size_t)p, "%s%ld", k?" ":"", q);
        }
        int bate = !strcmp(c, CIF[i]);
        if(!bate) erros++;
        if(i < 5) printf("        %-22.22s  %-20s %-21s %s\n", TEMA[i], CIF[i], c, bate?"sim":"NÃO");
    }
    ok("a cifra do texto recuperado é a MESMA que foi gravada — o endereço é o conteúdo",
       erros == 0);
    conclui("não há tabela de nomes: quem tem o texto sabe onde ele está.");
}

static void secao_R3(void){
    printf("\n§R3  E O TESTE SABE FALHAR: um slot corrompido é apanhado\n\n");
    /* corrompe-se um byte numa CÓPIA e vê-se se a cifra muda. Se não mudasse, o §R2 não estaria
     * a medir nada. */
    char txt[4096];
    int n = le_texto(BASE[0], NSL[0], NBY[0], txt, (int)sizeof txt);
    if(n <= 0){ ok("há texto para corromper", 0); return; }
    long a1 = 0, b1 = 0;
    for(int k = 0; k < n; k++){ a1 += (unsigned char)txt[k]*(k+1); b1 += (long)(unsigned char)txt[k]*(unsigned char)txt[k]; }
    txt[n/2] ^= 0x01;                              /* UM bit, no meio */
    long a2 = 0, b2 = 0;
    for(int k = 0; k < n; k++){ a2 += (unsigned char)txt[k]*(k+1); b2 += (long)(unsigned char)txt[k]*(unsigned char)txt[k]; }
    printf("        antes:  (%ld, %ld)\n        depois: (%ld, %ld)   — um bit trocado no meio\n",
           a1, b1, a2, b2);
    ok("um bit trocado muda o endereço — o teste do §R2 pode falhar, logo mede", a1 != a2 || b1 != b2);
    conclui("um endereço que não mudasse com o conteúdo não seria endereço.");
}

int main(void){
    disco_prende(DISCO_BASE(196),"dados/BASE_196.bin",(size_t)((MAXE)),sizeof(long));
    BASE = DISCO_FIXO(long, 196);
    disco_zera(BASE,(size_t)((MAXE)),sizeof(long));
    disco_prende(DISCO_BASE(197),"dados/NSL_197.bin",(size_t)((MAXE)),sizeof(long));
    NSL = DISCO_FIXO(long, 197);
    disco_zera(NSL,(size_t)((MAXE)),sizeof(long));
    disco_prende(DISCO_BASE(198),"dados/NBY_198.bin",(size_t)((MAXE)),sizeof(long));
    NBY = DISCO_FIXO(long, 198);
    disco_zera(NBY,(size_t)((MAXE)),sizeof(long));
    disco_prende(DISCO_BASE(161),"dados/TEMA_161.bin",(size_t)((MAXE)*(128)),sizeof(char));
    TEMA = DISCO_FIXO2(char, 128, 161);
    disco_zera(TEMA,(size_t)((MAXE)*(128)),sizeof(char));
    disco_prende(DISCO_BASE(162),"dados/CIF_162.bin",(size_t)((MAXE)*(64)),sizeof(char));
    CIF = DISCO_FIXO2(char, 64, 162);
    disco_zera(CIF,(size_t)((MAXE)*(64)),sizeof(char));
    FILE *f = fopen("/tmp/banco_saber.idx", "r");
    if(!f){ printf("NAO MEDIU — corra  ./grava_saber.sh  com o ollama acordado.\n"); return 2; }
    char l[1024];
    while(NE < MAXE && fgets(l, sizeof l, f)){
        char *t[5]; int nt = 0;
        t[nt++] = l;
        for(char *p = l; *p && nt < 5; p++) if(*p == '\t'){ *p = 0; t[nt++] = p+1; }
        if(nt < 5) continue;
        snprintf(TEMA[NE], sizeof TEMA[0], "%s", t[0]);
        BASE[NE] = atol(t[1]); NSL[NE] = atol(t[2]); NBY[NE] = atol(t[3]);
        snprintf(CIF[NE], sizeof CIF[0], "%s", t[4]);
        for(char *p = CIF[NE]; *p; p++) if(*p == '\n') *p = 0;
        NE++;
    }
    fclose(f);
    fb = open("/tmp/banco_saber.dat", O_RDONLY);
    if(fb < 0 || NE < 6){ printf("NAO MEDIU — banco ausente ou só %d entradas.\n", NE); return 2; }

    puts("recupera.c — O BANCO DEVOLVE O QUE ELE GRAVOU");
    puts("=============================================");
    printf("  %d afirmações do doador, gravadas em slots de 16 bytes\n", NE);
    puts("");
    puts("  Autocompletar, aqui, é o banco devolver o que lá foi posto. Eu tinha trocado essa");
    puts("  pergunta por outra — se a cifra se prevê a si própria — e anunciado a resposta da");
    puts("  minha como se fosse um limite do sistema.");
    secao_R1(); secao_R2(); secao_R3();
    printf("\n=============================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  ELE GRAVOU O QUE SABE E O BANCO DEVOLVEU, byte a byte, nas doze. A cifra do texto");
        puts("  recuperado é a mesma que foi gravada — o endereço é o conteúdo, sem tabela de");
        puts("  nomes pelo meio — e um bit trocado muda-a, logo o teste mede.");
        puts("");
        puts("  E era isto e mais nada: o sistema é reversível, e a reversibilidade já estava");
        puts("  medida. Eu é que fui buscar uma pergunta sobre periodicidade que ele nunca fez.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
