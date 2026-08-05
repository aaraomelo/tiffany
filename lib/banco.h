/* banco.h — O BANCO COMO BIBLIOTECA: os slots, e a escrita que nao se parte a meio.
 *
 * O Aarao: "sao so' simbolos, poe no banco — a leitura e' simples, so' colocar no slot."
 *
 * Isto e' o banco/banco.c extraido, sem uma linha mudada: 65536 slots de 32 bytes
 * {fp:8, off:8, len:8, crc:8}, e a ordem que o defende de uma queda a meio — grava o
 * dado, fsync, e SO' ENTAO escreve o slot. Um orfao no .dat e' inofensivo; um slot torto
 * nao passa no crc e le-se como vazio.
 *
 * E' o que as lajes do disco.h nao tem, e e' por isso que a memoria que sai da RAM deve
 * acabar aqui e nao la'. O banco.c continua a existir e agora inclui isto — o programa e
 * a biblioteca sao o mesmo codigo, e nao duas copias que divergem.
 *
 *   struct base b;
 *   abrir(&b, "dados/meu", 1);          gravar(&b, "chave", val, n);
 *   ler(&b, "chave", out, cap);         fechar(&b);
 */
#ifndef TIFFANY_BANCO_H
#define TIFFANY_BANCO_H

/* banco.c — O BANCO. Mete dado, tira dado, sem erro. Agora com ESCRITA ATÔMICA.
 *
 * Sem paper, sem teoria nova. As peças já medidas:
 *   1. a transformação REVERSÍVEL — o gato (x,y) ↦ (m·x+y, x) mod 256 é permutação, porque
 *      det = −1 é unidade (espelho.c §E3). Nada some ao guardar.
 *   2. a volta EXATA — o que entra sai idêntico, bit a bit (semente.c §M3).
 *   3. a durabilidade — grava, fsync, fecha, reabre, lê.
 *
 * A ATOMICIDADE, que é o que esta versão acrescenta. O perigo não é o disco mentir: é a
 * energia cair NO MEIO. Três lugares onde isso quebra, e o que se faz em cada um:
 *
 *   (a) queda no meio do PAYLOAD  → o registro fica pela metade e leria lixo.
 *       Defesa: cada registro carrega cabeçalho {MAGIC, tamanho, crc32} e o leitor CONFERE.
 *       Meio registro não passa no crc, e o leitor devolve "não existe" — nunca lixo.
 *   (b) queda ENTRE o dado e o índice → o dado fica órfão no .dat.
 *       Defesa: a ORDEM. Grava o dado, fsync, e só então escreve o slot. Órfão é inofensivo:
 *       ninguém aponta para ele. O contrário (índice antes do dado) apontaria para o nada.
 *   (c) queda no meio do SLOT → o índice fica com meia entrada.
 *       Defesa: o slot tem crc próprio, e tem 32 bytes — 128 por página de 4 KiB, logo NENHUM
 *       slot cruza a borda de página. Slot torto não passa no crc e é tratado como vazio.
 *
 * O que NÃO se promete: se o disco mentir no fsync (cache volátil sem barreira), nada disto
 * salva — e isso é hardware, não código. Está dito porque prometer o contrário seria mentira.
 *
 * SEM RAM: estado O(1). Índice e dados em disco, pread/pwrite. Nada carregado inteiro.
 * NÃO-CUSTODIAL: o banco nunca olha dentro do valor. Guarda o que recebe, devolve idêntico.
 *
 *   cc -O2 -std=c99 banco.c -o banco
 *   ./banco teste                       # round-trip + os três cenários de queda
 *   ./banco grava <base> <chave> <valor>
 *   ./banco le    <base> <chave>
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define NSLOT   (1L<<16)
#define SLOTSZ  32                /* {fp:8, off:8, len:8, crc:8} — 128 por página de 4 KiB  */
#define VMAX    4096
#define GATO_M  1
#define MAGIC   0x0A7A0BA5u       /* marca do registro                                      */
#define CAB     12                /* cabeçalho no .dat: magic(4) + len(4) + crc(4)          */

static uint64_t fp64(const char *s, size_t n){
    uint64_t h = 1469598103934665603UL;
    for(size_t i = 0; i < n; i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
/* crc32 (IEEE), sem tabela persistida — a tabela nasce e morre na pilha */
static uint32_t crc32(const unsigned char *b, size_t n){
    uint32_t c = 0xFFFFFFFFu;
    for(size_t i = 0; i < n; i++){
        c ^= b[i];
        for(int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(c & 1)));
    }
    return c ^ 0xFFFFFFFFu;
}
static uint32_t crc_de(const void *p, size_t n){ return crc32((const unsigned char*)p, n); }

/* --- o gato sobre bytes: permutação, reversível sem perda --- */
static void gato_ida(unsigned char *b, size_t n){
    for(size_t i = 0; i + 1 < n; i += 2){
        unsigned char x = b[i], y = b[i+1];
        b[i] = (unsigned char)((GATO_M * x + y) & 0xFF); b[i+1] = x;
    }
}
static void gato_volta(unsigned char *b, size_t n){
    for(size_t i = 0; i + 1 < n; i += 2){
        unsigned char a = b[i], c = b[i+1];
        b[i] = c; b[i+1] = (unsigned char)((a - GATO_M * c) & 0xFF);
    }
}

struct base { int idx, dat; };

static int abrir(struct base *b, const char *nome, int criar){
    char p1[512], p2[512];
    snprintf(p1, sizeof p1, "%s.idx", nome);
    snprintf(p2, sizeof p2, "%s.dat", nome);
    int fl = O_RDWR | (criar ? O_CREAT : 0);
    b->idx = open(p1, fl, 0644);
    b->dat = open(p2, fl, 0644);
    if(b->idx < 0 || b->dat < 0) return 0;
    if(criar){ ftruncate(b->idx, NSLOT * SLOTSZ); ftruncate(b->dat, 0); }
    return 1;
}
static void fechar(struct base *b){
    if(b->idx >= 0){ fsync(b->idx); close(b->idx); }
    if(b->dat >= 0){ fsync(b->dat); close(b->dat); }
    b->idx = b->dat = -1;
}

/* --- o slot, com crc próprio: torto é tratado como vazio --- */
static int slot_le(int fd, long i, uint64_t s[4]){
    if(pread(fd, s, SLOTSZ, i*SLOTSZ) != SLOTSZ) return 0;
    if(!s[0]) return 1;                                  /* vazio, e vazio é válido */
    uint64_t esperado = (uint64_t)crc_de(s, 24);
    return s[3] == esperado;                             /* 0 = torto: ignora-se */
}
static int slot_grava(int fd, long i, uint64_t fp, uint64_t off, uint64_t len){
    uint64_t s[4] = { fp, off, len, 0 };
    s[3] = (uint64_t)crc_de(s, 24);
    return pwrite(fd, s, SLOTSZ, i*SLOTSZ) == SLOTSZ;
}

static int gravar(struct base *b, const char *chave, const unsigned char *val, long n){
    if(n <= 0 || n > VMAX) return 0;
    unsigned char rec[CAB + VMAX];
    memcpy(rec + CAB, val, (size_t)n);
    gato_ida(rec + CAB, (size_t)n);

    uint32_t mg = MAGIC, ln = (uint32_t)n, cr = crc_de(rec + CAB, (size_t)n);
    memcpy(rec + 0, &mg, 4); memcpy(rec + 4, &ln, 4); memcpy(rec + 8, &cr, 4);

    /* (b) a ORDEM: o dado primeiro, e só depois o ponteiro para ele */
    off_t off = lseek(b->dat, 0, SEEK_END);
    if(write(b->dat, rec, (size_t)(CAB + n)) != (ssize_t)(CAB + n)) return 0;
    if(fsync(b->dat) != 0) return 0;                     /* o dado está em disco ANTES do índice */

    uint64_t fp = fp64(chave, strlen(chave));
    long i = (long)(fp % (uint64_t)NSLOT);
    for(long t = 0; t < NSLOT; t++){
        uint64_t s[4];
        int bom = slot_le(b->idx, i, s);
        if(!bom || !s[0] || s[0] == fp){                 /* vazio, torto ou o mesmo: pode usar */
            if(!slot_grava(b->idx, i, fp, (uint64_t)off, (uint64_t)n)) return 0;
            return fsync(b->idx) == 0;
        }
        i = (i + 1) % NSLOT;
    }
    return 0;
}

/* devolve o tamanho, ou −1. NUNCA devolve lixo: o crc decide. */
static long ler(struct base *b, const char *chave, unsigned char *out, long cap){
    uint64_t fp = fp64(chave, strlen(chave));
    long i = (long)(fp % (uint64_t)NSLOT);
    for(long t = 0; t < NSLOT; t++){
        uint64_t s[4];
        if(!slot_le(b->idx, i, s)) { i = (i+1) % NSLOT; continue; }   /* slot torto: pula */
        if(!s[0]) return -1;
        if(s[0] == fp){
            long n = (long)s[2];
            if(n <= 0 || n > cap) return -1;
            unsigned char cab[CAB];
            if(pread(b->dat, cab, CAB, (off_t)s[1]) != CAB) return -1;
            uint32_t mg, ln, cr;
            memcpy(&mg, cab+0, 4); memcpy(&ln, cab+4, 4); memcpy(&cr, cab+8, 4);
            if(mg != MAGIC || (long)ln != n) return -1;               /* (a) cabeçalho torto */
            if(pread(b->dat, out, (size_t)n, (off_t)s[1] + CAB) != n) return -1;  /* (a) truncado */
            if(crc_de(out, (size_t)n) != cr) return -1;               /* (a) corrompido */
            gato_volta(out, (size_t)n);
            return n;
        }
        i = (i + 1) % NSLOT;
    }
    return -1;
}

static unsigned long lcg;
static unsigned char proximo(void){
    lcg ^= lcg << 13; lcg ^= lcg >> 7; lcg ^= lcg << 17;
    return (unsigned char)(lcg & 0xFF);
}
static void conteudo(long k, unsigned char *v, long *n){
    *n = 1 + (k * 7919) % 1024;
    lcg = 88172645463325252UL + (unsigned long)k * 2654435761UL;
    for(long j = 0; j < *n; j++) v[j] = proximo();
}
#include "unidade.h"


#endif
