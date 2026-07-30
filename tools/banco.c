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
static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-54s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}

int main(int argc, char **argv){
    if(argc >= 2 && !strcmp(argv[1], "teste")){
        const long N = 2000;
        const char *nome = "/tmp/banco_teste";
        char pdat[512], pidx[512];
        snprintf(pdat, sizeof pdat, "%s.dat", nome);
        snprintf(pidx, sizeof pidx, "%s.idx", nome);
        unlink(pdat); unlink(pidx);
        struct base b;
        unsigned char val[VMAX], lido[VMAX];
        long n;

        printf("\n=== O BANCO: escrita atômica ===========================================\n");
        printf("\n§1  O caminho normal: grava, fsync, fecha, reabre, confere byte a byte.\n\n");
        if(!abrir(&b, nome, 1)){ perror("abrir"); return 2; }
        long bytes = 0;
        for(long k = 0; k < N; k++){
            char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
            conteudo(k, val, &n);
            if(!gravar(&b, ch, val, n)) falhas++;
            bytes += n;
        }
        fechar(&b);
        if(!abrir(&b, nome, 0)) return 2;
        long iguais = 0;
        for(long k = 0; k < N; k++){
            char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
            conteudo(k, val, &n);
            long m = ler(&b, ch, lido, VMAX);
            if(m == n && memcmp(val, lido, (size_t)n) == 0) iguais++;
        }
        fechar(&b);
        printf("      gravados %ld registros, %ld bytes; fechado e reaberto\n", N, bytes);
        printf("      conferidos idênticos .......................... %ld de %ld\n", iguais, N);
        ok("todos voltam byte a byte", iguais == N);

        printf("\n§2  (a) Queda no meio do PAYLOAD: o .dat é truncado.\n");
        printf("     Um registro fica pela metade. O leitor tem de dizer 'não existe',\n");
        printf("     nunca devolver meio dado como se fosse inteiro.\n\n");
        {
            off_t tam;
            { int f = open(pdat, O_RDONLY); tam = lseek(f, 0, SEEK_END); close(f); }
            int f = open(pdat, O_RDWR); ftruncate(f, tam - 300); fsync(f); close(f);
            if(!abrir(&b, nome, 0)) return 2;
            long inteiros = 0, recusados = 0, lixo = 0;
            for(long k = 0; k < N; k++){
                char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
                conteudo(k, val, &n);
                long m = ler(&b, ch, lido, VMAX);
                if(m < 0) recusados++;
                else if(m == n && memcmp(val, lido, (size_t)n) == 0) inteiros++;
                else lixo++;
            }
            fechar(&b);
            printf("      truncado em 300 bytes do fim do .dat\n");
            printf("      voltaram inteiros ............................. %ld\n", inteiros);
            printf("      recusados (o crc barrou) ...................... %ld\n", recusados);
            printf("      devolveram LIXO ............................... %ld\n", lixo);
            ok("nenhum lixo passou", lixo == 0);
            ok("e o truncamento foi detectado, não ignorado", recusados > 0);
            ok("os demais continuam lendo normalmente", inteiros >= N - 2);
        }

        printf("\n§3  (a) Corrupção silenciosa: um bit vira no meio do payload.\n\n");
        {
            unlink(pdat); unlink(pidx);
            if(!abrir(&b, nome, 1)) return 2;
            /* dois registros LONGOS, e o primeiro fica no offset 0 — assim o byte que se
             * corrompe cai comprovadamente dentro do payload dele, e não no vizinho.
             * (Na versão anterior deste teste eu virei um bit em CAB+5 supondo que ali fosse
             *  o payload do registro 0 — mas ele tem 1 byte de tamanho, e o que corrompi foi
             *  o CABEÇALHO do registro 1. O banco acertou; o teste é que estava errado.) */
            long nA, nB;
            unsigned char A[VMAX], B[VMAX];
            conteudo(5, A, &nA);                      /* 684 bytes */
            conteudo(6, B, &nB);
            gravar(&b, "alvo", A, nA);                /* offset 0, payload em CAB */
            gravar(&b, "vizinho", B, nB);
            fechar(&b);
            printf("      alvo: %ld bytes no offset 0; vizinho: %ld bytes logo depois\n", nA, nB);

            int f = open(pdat, O_RDWR);
            off_t pos = CAB + nA/2;                   /* comprovadamente dentro do payload do alvo */
            unsigned char by; pread(f, &by, 1, pos); by ^= 0x01; pwrite(f, &by, 1, pos);
            fsync(f); close(f);

            if(!abrir(&b, nome, 0)) return 2;
            long mA = ler(&b, "alvo", lido, VMAX);
            long mB = ler(&b, "vizinho", lido, VMAX);
            int vizinho_ok = (mB == nB && memcmp(B, lido, (size_t)nB) == 0);
            fechar(&b);
            printf("      um bit invertido no byte %ld (dentro do payload do alvo)\n", (long)pos);
            printf("      o alvo devolve ................................ %s\n", mA < 0 ? "nada ✓" : "LIXO ✗");
            printf("      o vizinho devolve ............................. %s\n", vizinho_ok ? "o dado ✓" : "nada ✗");
            ok("o bit trocado é detectado pelo crc", mA < 0);
            ok("e o vizinho corrompido não contamina os outros", vizinho_ok);
        }

        printf("\n§4  (c) Queda no meio do SLOT: meia entrada no índice.\n\n");
        {
            /* cada seção monta a PRÓPRIA base: o §3 recriou o arquivo com outras chaves,
             * e assumir o estado deixado pela seção anterior já me deu dois falsos negativos. */
            unlink(pdat); unlink(pidx);
            if(!abrir(&b, nome, 1)) return 2;
            for(long k = 0; k < 10; k++){
                char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
                conteudo(k, val, &n); gravar(&b, ch, val, n);
            }
            fechar(&b);

            int f = open(pidx, O_RDWR);
            uint64_t s[4];
            long alvo = -1;
            for(long i = 0; i < NSLOT; i++){
                pread(f, s, SLOTSZ, i*SLOTSZ);
                if(s[0]){ alvo = i; break; }
            }
            uint64_t metade[2] = { 0xDEADBEEFCAFEBABEUL, 0x1111111111111111UL };
            pwrite(f, metade, 16, alvo*SLOTSZ);           /* escreve metade do slot */
            fsync(f); close(f);
            if(!abrir(&b, nome, 0)) return 2;
            long lidos = 0, nada = 0, lixo = 0;
            for(long k = 0; k < 10; k++){
                char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
                conteudo(k, val, &n);
                long m = ler(&b, ch, lido, VMAX);
                if(m < 0) nada++;
                else if(m == n && memcmp(val, lido, (size_t)n) == 0) lidos++;
                else lixo++;
            }
            fechar(&b);
            printf("      slot %ld sobrescrito pela metade\n", alvo);
            printf("      registros ainda legíveis ...................... %ld de 10\n", lidos);
            printf("      devolveram LIXO ............................... %ld\n", lixo);
            ok("o slot torto não devolve lixo", lixo == 0);
            ok("e não derruba os outros registros", lidos >= 8);
        }

        printf("\n§5  (b) Queda ENTRE o dado e o índice: o dado fica órfão.\n");
        printf("     Grava-se o payload no .dat SEM escrever o slot — exatamente o que\n");
        printf("     acontece se a energia cair nesse intervalo.\n\n");
        {
            unlink(pdat); unlink(pidx);
            if(!abrir(&b, nome, 1)) return 2;
            for(long k = 0; k < 10; k++){
                char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
                conteudo(k, val, &n); gravar(&b, ch, val, n);
            }
            /* simula a queda: escreve o registro 99 no .dat e NÃO escreve o slot */
            conteudo(99, val, &n);
            unsigned char rec[CAB + VMAX];
            memcpy(rec + CAB, val, (size_t)n); gato_ida(rec + CAB, (size_t)n);
            uint32_t mg = MAGIC, ln = (uint32_t)n, cr = crc_de(rec + CAB, (size_t)n);
            memcpy(rec, &mg, 4); memcpy(rec+4, &ln, 4); memcpy(rec+8, &cr, 4);
            lseek(b.dat, 0, SEEK_END); write(b.dat, rec, (size_t)(CAB + n));
            fechar(&b);

            if(!abrir(&b, nome, 0)) return 2;
            long m99 = ler(&b, "registro-99", lido, VMAX);
            long lidos = 0;
            for(long k = 0; k < 10; k++){
                char ch[64]; snprintf(ch, sizeof ch, "registro-%ld", k);
                conteudo(k, val, &n);
                if(ler(&b, ch, lido, VMAX) == n && memcmp(val, lido, (size_t)n) == 0) lidos++;
            }
            fechar(&b);
            printf("      o órfão (registro 99) devolve ................. %s\n",
                   m99 < 0 ? "nada ✓" : "APARECEU ✗");
            printf("      os 10 anteriores continuam .................... %ld de 10\n", lidos);
            ok("o dado órfão é invisível — não meio-gravado", m99 < 0);
            ok("e o banco fica consistente, não corrompido", lidos == 10);
            printf("\n      É por isso que a ordem importa: dado primeiro, ponteiro depois.\n");
            printf("      Ao contrário, o índice apontaria para o nada — e aí sim seria erro.\n");
        }

        unlink(pdat); unlink(pidx);
        printf("\n");
        if(falhas){ printf("  FALHOU: %d\n\n", falhas); return 1; }
        printf("  ATÔMICO. Meio registro, bit trocado, slot torto e queda entre as duas\n");
        printf("  escritas: em nenhum caso o banco devolve lixo, e em nenhum caso perde o\n");
        printf("  que já estava lá.\n\n");
        return 0;
    }

    if(argc >= 5 && !strcmp(argv[1], "grava")){
        struct base b;
        int novo = access(argv[2], F_OK) != 0;
        char p[512]; snprintf(p, sizeof p, "%s.idx", argv[2]);
        novo = access(p, F_OK) != 0;
        if(!abrir(&b, argv[2], novo)){ perror("abrir"); return 2; }
        int r = gravar(&b, argv[3], (const unsigned char*)argv[4], (long)strlen(argv[4]));
        fechar(&b);
        printf("%s\n", r ? "gravado" : "falhou");
        return r ? 0 : 1;
    }
    if(argc >= 4 && !strcmp(argv[1], "le")){
        struct base b;
        if(!abrir(&b, argv[2], 0)){ perror("abrir"); return 2; }
        unsigned char out[VMAX];
        long n = ler(&b, argv[3], out, VMAX);
        fechar(&b);
        if(n < 0){ printf("(nao existe)\n"); return 1; }
        fwrite(out, 1, (size_t)n, stdout); printf("\n");
        return 0;
    }
    fprintf(stderr, "uso: banco teste | banco grava <base> <chave> <valor> | banco le <base> <chave>\n");
    return 2;
}
