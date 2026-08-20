/* disco_wasm.c — O DISCO DE WASM: os slots são fatias de UM array global, o dual do mmap.
 *
 * O Aarão escolheu o tex_core.c auto-contido (sem cpp). A peça que falta é o disco de WASM: no
 * nativo o slot é um ficheiro mmap num endereço fixo (disco_mmap, sem RAM); em WASM não há ficheiro
 * nem mmap --- o slot É a memória linear. Então os slots são FATIAS de um array global, em offsets
 * COMPACTOS (a memória linear do wasm é pequena; os 32 TiB do DISCO_BASE não cabem).
 *
 * É o dual do disco_mmap, pela mesma interface (g_disco): mesma assinatura (i, nome, n → ponteiro),
 * outro mecanismo. E a régua é a de sempre: escrever num slot não toca no vizinho (resíduo 0), e a
 * volta lê o que se escreveu.
 *
 *   §W1  o layout é COMPACTO e não-sobreposto: OFF[i+1] = OFF[i] + TAM[i], tudo dentro do array
 *   §W2  os slots são independentes: escrever no slot i não corrompe o slot j (o dual da soma directa)
 *   §W3  a volta lê o que se escreveu, byte a byte (resíduo 0) --- o slot É a memória
 *
 *   cc -O2 -std=c99 -Wall -I../lib disco_wasm.c -o disco_wasm && ./disco_wasm
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

typedef long L;
enum { NSLOT = 16 };

/* os tamanhos máximos por slot, como em tex.c / libc.c (fonte e source 4 MB,
 * PDF 128 MB no slot 14, fundo 1 MB). Em wasm cada slot tem tecto fixo, o array é a soma. */
static const L TAM[NSLOT] = {
    1L<<20, 1L<<16, 1L<<16, 1L<<22, 1L<<22, 1L<<20, 1L<<16, 1L<<18,
    1L<<16, 1L<<14, 1L<<18, 1L<<16, 1L<<16, 1L<<16, 1L<<27, 1L<<20
};
static L OFF[NSLOT];                 /* o offset compacto de cada slot no array */
static L DISCO_TAM;                  /* o tamanho total (a soma) */

/* o array global É a memória linear do wasm. Aqui, para medir, um único bloco; em wasm é a memória
 * do módulo. (mmap/calloc para não estourar a .bss do medidor com 140 MB; a doutrina é «o slot É a memória».) */
static unsigned char *DISCO;

/* o dual do disco_mmap: mesma interface, mas devolve uma fatia do array em vez de um ficheiro. */
static unsigned char *disco_slot(int i, const char *nome, L n){
    (void)nome; (void)n;                          /* o host já sabe o tamanho: é TAM[i] */
    return DISCO + OFF[i];
}

int main(void){
    printf("=== O DISCO DE WASM: fatias de um array global, o dual do mmap ==============\n\n");

    /* monta o layout compacto */
    OFF[0] = 0;
    for(int i = 1; i < NSLOT; i++) OFF[i] = OFF[i-1] + TAM[i-1];
    DISCO_TAM = OFF[NSLOT-1] + TAM[NSLOT-1];
    DISCO = (unsigned char*)calloc(1, (size_t)DISCO_TAM);   /* uma vez, o «disco» inteiro */
    if(!DISCO){ printf("sem memória para o disco de teste\n"); return 1; }

    /* ── §W1 o layout é compacto e não-sobreposto ────────────────────────────────────────── */
    /* OFF[i+1] = OFF[i] + TAM[i]: cada slot começa exactamente onde o anterior acaba (compacto,
     * sem buraco), e o último cabe no array (OFF+TAM ≤ DISCO_TAM). É o oposto dos 256 GiB entre
     * bases do mmap: em wasm a memória é pequena e o layout tem de ser justo. */
    int compacto = 1;
    for(int i = 1; i < NSLOT; i++) if(OFF[i] != OFF[i-1] + TAM[i-1]) compacto = 0;
    if(OFF[NSLOT-1] + TAM[NSLOT-1] != DISCO_TAM) compacto = 0;
    printf("      %d slots, total %ld bytes (%ld MB); OFF[3]=%ld OFF[14]=%ld\n\n",
           NSLOT, DISCO_TAM, DISCO_TAM >> 20, OFF[3], OFF[14]);
    ok("§W1 o layout é COMPACTO e não-sobreposto: cada slot começa onde o anterior acaba"
       " (OFF[i+1]=OFF[i]+TAM[i]) e o todo cabe no array --- em wasm a memória é pequena, ao contrário"
       " dos 256 GiB entre bases do mmap", compacto);

    /* ── §W2 os slots são independentes: escrever num não toca no vizinho ─────────────────── */
    /* escreve-se em CADA slot um padrão próprio (o índice, repetido no início/meio/fim da sua fatia).
     * Depois relê-se TUDO: cada slot tem só o seu padrão --- nenhuma escrita transbordou para o
     * vizinho. É a soma directa: R^{a} ⊕ R^{b}, casas que não falam. */
    for(int i = 0; i < NSLOT; i++){
        unsigned char *b = disco_slot(i, "", TAM[i]);
        b[0] = (unsigned char)(i + 1);
        b[TAM[i]/2] = (unsigned char)(i + 1);
        b[TAM[i]-1] = (unsigned char)(i + 1);
    }
    int independentes = 1;
    for(int i = 0; i < NSLOT; i++){
        unsigned char *b = disco_slot(i, "", TAM[i]);
        if(b[0] != (unsigned char)(i+1) || b[TAM[i]/2] != (unsigned char)(i+1) || b[TAM[i]-1] != (unsigned char)(i+1))
            independentes = 0;                       /* o padrão do slot i sobreviveu intacto */
    }
    printf("§W2  escrito o índice em cada slot (início/meio/fim); reler dá o de cada um, sem transbordo\n\n");
    ok("§W2 os slots são INDEPENDENTES: escrever no slot i (início, meio, fim) não corrompe o slot j"
       " --- cada fatia guarda só o seu padrão. É a soma directa, casas que não falam", independentes);

    /* ── §W3 a volta lê o que se escreveu (resíduo 0) ────────────────────────────────────── */
    /* num slot, escreve-se uma sequência e relê-se: byte a byte igual. O slot É a memória --- não há
     * ida ao ficheiro, e por isso não há onde perder. */
    unsigned char *s = disco_slot(4, "", TAM[4]);    /* o slot do source */
    L residuo = 0;
    for(L k = 0; k < 100000; k++) s[k] = (unsigned char)((k*7 + 3) & 0xFF);
    for(L k = 0; k < 100000; k++) if(s[k] != (unsigned char)((k*7 + 3) & 0xFF)) residuo++;
    printf("§W3  escrito+relido 100000 bytes no slot 4: resíduo %ld\n\n", residuo);
    ok("§W3 a VOLTA lê o que se escreveu, byte a byte (resíduo 0): o slot É a memória, não há ida ao"
       " ficheiro e por isso não há onde perder --- é a mesma régua do disco_mmap, outro mecanismo",
       residuo == 0);

    free(DISCO);
    printf("==========================================================================\n");
    if(!falhas){
        puts("  O disco de WASM é o dual do disco_mmap: mesma interface (g_disco: i, nome, n → ponteiro),");
        puts("  outro mecanismo. No nativo o slot é um ficheiro mmap num endereço fixo (sem RAM); em WASM");
        puts("  o slot É a memória linear --- uma fatia de um array global, em offset compacto. O layout é");
        puts("  justo (a memória do wasm é pequena), os slots não falam entre si (soma directa), e a volta");
        puts("  lê o que se escreveu. É a peça que o tex_core.c auto-contido aponta em g_disco no wasm.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
