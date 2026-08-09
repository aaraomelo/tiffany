/* config_corpo.c — A CONFIG É UM CORPO, TEM ASSINATURA. E recupera as assinaturas dos formatos.
 *
 * O Aarão: «para os parsers a config é um corpo, tem assinatura. Recupera as assinaturas dos
 * formatos de arquivos que já tem no corpo técnico.»
 *
 * O corpo técnico já tem as assinaturas dos formatos, em dois níveis, e não se inventa outra:
 *   TEXTO   (lib/caminho.h)  um formato é um corpo (marca, razão, sinal) --- a CIFRA do gerador
 *   BINÁRIO (lib/banco.h)    um registo assina-se {MAGIC, tamanho, crc32}, e o leitor CONFERE
 *
 * A config que os parsers produzem (margem, cores, escalas...) é BINÁRIA: assina-se como o banco,
 * e é um corpo que entra no mesmo catálogo dos formatos. A assinatura é a régua (a Lei 1: o crc
 * CONFERIDO tem de bater com o GUARDADO --- a metade refletida do teorema operacional, não um chute).
 *
 *   §G1  recupera as assinaturas dos formatos de TEXTO (caminho.h): cada formato é um corpo, cifras distintas
 *   §G2  a config é um corpo BINÁRIO assinado {MAGIC, len, crc32} (banco.h); o leitor CONFERE, round-trip 0
 *   §G3  a assinatura é a metade refletida: um bit trocado muda o crc, e o leitor RECUSA (não chuta)
 *
 *   cc -O2 -std=c99 -Wall -I../lib config_corpo.c -o config_corpo && ./config_corpo
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "caminho.h"     /* RECUPERA as assinaturas dos formatos de texto: FORMATOS[] */

typedef long long L;

/* o crc32 IEEE do banco.h (lib/banco.h): a assinatura de um formato binário. Reescrito à mesma
 * régua --- é o mesmo polinómio 0xEDB88320, e §G2 confere-o contra um valor conhecido. */
static unsigned crc32c(const unsigned char *b, long n){
    unsigned c = 0xFFFFFFFFu;
    for(L i = 0; i < n; i++){ c ^= b[i];
        for(int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (unsigned)(-(int)(c & 1))); }
    return c ^ 0xFFFFFFFFu;
}
#define MAGIC 0x0A7A0BA5u    /* a marca do registo (banco.h) */
#define CAB   12             /* o cabeçalho: magic(4) + len(4) + crc(4) */

/* escreve o inteiro little-endian de 4 bytes (o cabeçalho do banco) */
static void p32(unsigned char *b, unsigned v){ b[0]=v; b[1]=v>>8; b[2]=v>>16; b[3]=v>>24; }
static unsigned g32(const unsigned char *b){ return b[0] | (b[1]<<8) | (b[2]<<16) | ((unsigned)b[3]<<24); }

int main(void){
    printf("=== A CONFIG É UM CORPO, TEM ASSINATURA — e recupera as dos formatos ========\n\n");

    /* ── §G1 recupera as assinaturas dos formatos de texto (caminho.h) ───────────────────── */
    /* UM FORMATO É UM CORPO, e como qualquer corpo é (razão, sinal) mais a marca do gerador. Não
     * se inventa: lê-se de FORMATOS[] do caminho.h. Cada formato tem cifra PRÓPRIA --- e é isso que
     * o distingue, como um corpo tipográfico é (variante, degrau). */
    int nf = (int)(sizeof FORMATOS / sizeof FORMATOS[0]);
    int distintas = 1;
    printf("      formato | marca | razão | sinal\n");
    for(int i = 0; i < nf; i++){
        printf("      %-7s |   %c   |   %ld   |  %+ld\n",
               FORMATOS[i].nome, FORMATOS[i].marca, FORMATOS[i].razao, FORMATOS[i].sinal);
        for(int j = i + 1; j < nf; j++)
            if(FORMATOS[i].marca == FORMATOS[j].marca &&
               FORMATOS[i].razao == FORMATOS[j].razao &&
               FORMATOS[i].sinal == FORMATOS[j].sinal) distintas = 0;   /* cifras iguais = colisão */
    }
    printf("\n");
    ok("§G1 RECUPERADAS as assinaturas dos formatos (caminho.h): cada formato é um corpo (marca,"
       " razão, sinal), e as cifras são DISTINTAS --- é a assinatura que distingue um formato do"
       " outro, como (variante, degrau) distingue um corpo tipográfico", distintas && nf >= 3);

    /* ── §G2 a config é um corpo binário assinado {MAGIC, len, crc32} (banco.h) ───────────── */
    /* a config dos parsers --- margem, cores, escalas --- é bytes. Assina-se como o banco: o slot é
     * [MAGIC][len][crc32][payload], e o leitor CONFERE os três. Round-trip: escrever a config,
     * ler de volta, e o que sai é o que entrou (resíduo 0). A régua é a mesma dos formatos. */
    unsigned char cfg[32];
    /* uma config de exemplo: margem=64, 2 cores (RGB), 1 escala (base, degrau) */
    long clen = 0;
    cfg[clen++] = 64;                                  /* margem */
    cfg[clen++] = 204; cfg[clen++] = 170; cfg[clen++] = 0;   /* cor 1 (ouro) */
    cfg[clen++] = 0;   cfg[clen++] = 0;   cfg[clen++] = 0;   /* cor 2 (tinta) */
    cfg[clen++] = 10;  cfg[clen++] = 1;                /* escala: base 10, degrau 1 */

    /* o crc é o IEEE real (o de banco.h), não uma cópia à mão: confere-se contra o valor-padrão
     * crc32("123456789")=0xCBF43926, e só depois se usa para assinar. */
    int crc_e_ieee = (crc32c((const unsigned char*)"123456789", 9) == 0xCBF43926u);

    unsigned char slot[64];
    p32(slot, MAGIC); p32(slot + 4, (unsigned)clen); p32(slot + 8, crc32c(cfg, clen));  /* assina */
    memcpy(slot + CAB, cfg, (size_t)clen);            /* o payload */

    /* o leitor CONFERE (como o banco: cabeçalho torto = recusa) */
    unsigned mg = g32(slot), ln = g32(slot + 4), cr = g32(slot + 8);
    int confere = (mg == MAGIC) && ((long)ln == clen) && (cr == crc32c(slot + CAB, ln));
    /* round-trip: a config lida do slot é byte-a-byte a escrita */
    int volta_zero = (memcmp(slot + CAB, cfg, (size_t)clen) == 0);
    printf("§G2  slot assinado: MAGIC=%08X len=%u crc=%08X ; o leitor confere: %s\n\n",
           mg, ln, cr, confere ? "sim" : "não");
    ok("§G2 a CONFIG é um corpo binário ASSINADO {MAGIC, len, crc32} (banco.h): o leitor CONFERE os"
       " três (magic, comprimento, crc) e o round-trip devolve a config byte-a-byte (resíduo 0). É a"
       " porta pela qual o wrapper entrega a config ao núcleo, com assinatura", confere && volta_zero && crc_e_ieee);

    /* ── §G3 a assinatura é a metade refletida: um bit trocado é apanhado ─────────────────── */
    /* o teorema operacional: mede-se pela metade refletida. O crc GUARDADO é a assinatura; o crc
     * CONFERIDO é a volta. Se um bit vazar no payload, os dois deixam de bater --- e o leitor recusa.
     * Não se chuta que a config está certa: confere-se contra a sua própria assinatura. */
    unsigned char corrompido[64]; memcpy(corrompido, slot, (size_t)(CAB + clen));
    corrompido[CAB + 3] ^= 1;                          /* um bit vaza no payload */
    unsigned cr_guardado = g32(corrompido + 8);        /* a assinatura escrita */
    unsigned cr_conferido = crc32c(corrompido + CAB, g32(corrompido + 4));  /* a volta */
    int apanha = (cr_conferido != cr_guardado);        /* a metade refletida não fecha: recusa */
    /* e o controlo: sem corromper, os dois batem (o crc conferido = o guardado) */
    int fecha_limpo = (crc32c(slot + CAB, ln) == g32(slot + 8));
    printf("§G3  bit trocado: crc guardado=%08X, crc conferido=%08X → %s\n\n",
           cr_guardado, cr_conferido, apanha ? "RECUSA" : "passou (mau)");
    ok("§G3 a ASSINATURA é a metade refletida (teorema operacional): o crc GUARDADO e o crc CONFERIDO"
       " têm de bater; um bit vazado no payload muda o conferido e o leitor RECUSA, e sem corromper"
       " os dois batem. Não se chuta que a config está certa --- confere-se contra a assinatura",
       apanha && fecha_limpo);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  A config dos parsers é um CORPO, e como todo corpo tem ASSINATURA. Não se inventa uma:");
        puts("  recupera-se a do corpo técnico --- a cifra (marca, razão, sinal) dos formatos de texto");
        puts("  (caminho.h) e o {MAGIC, len, crc32} do binário (banco.h). A config é binária, assina-se");
        puts("  como o banco, e entra no mesmo catálogo dos formatos. O wrapper entrega-a ao núcleo por");
        puts("  um slot assinado; o núcleo CONFERE a assinatura antes de ler --- a régua da Lei 1, e a");
        puts("  metade refletida do teorema operacional: o crc conferido bate com o guardado, ou recusa.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
