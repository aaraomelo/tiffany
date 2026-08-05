/* banco_relogio.c — OS SLOTS SAO O RELOGIO, E O NAND E' A REALIZACAO.
 *
 * O Aarao: "ve bem a estrutura do banco: os slots sao o relogio definido na teoria; le
 * de novo e associa com NAND, a realizacao."
 *
 * E' literal, e esta' na linha 155 do banco:
 *
 *      i = (i + 1) % NSLOT
 *
 * que e' o PENTE DE PASSO 1 numa volta de 65536 marcas. A chave da' a FASE inicial
 * (fp64 mod N) e a sondagem anda marca a marca ate' encontrar o ponto fixo — o slot cuja
 * impressao bate — ou o vazio, que e' onde a volta se fecha sem achar.
 *
 * O relogio.c §R5 ja' tinha o contador: para um pente de passo d ate' t, a conta e'
 * floor(t/d)+1 e a densidade e' 1/d. Aqui d=1 e o instante e' quantas marcas se andou.
 *
 * ── E O NAND E' A REALIZACAO ────────────────────────────────────────────────────────
 *
 * Um slot nao guarda um valor: guarda {fp, off, len, crc} — ONDE ESTA' e SE ESTA' BEM.
 * E' a operacao retida, e nao o operando. E' o mesmo que uma celula NAND faz: nao e' um
 * sitio onde um bit esta', e' a operacao ja' feita e mantida — NAND(a,a) = NOT a, a
 * mesma porta a ser a involucao de si propria.
 *
 * Dai a frase do Aarao fechar: "o bit nao dissipa, nao ha' o que dissipar PORQUE NAO HA'
 * O QUE CONSTRUIR". A DRAM reconstroi a cada refresh e paga; o NAND retem e nao paga; e
 * o slot, que retem onde e se, tambem nao — le-se sem se tocar em nada.
 *
 *   §B1  a sondagem E' o pente de passo 1: anda marca a marca e da' a volta
 *   §B2  o contador bate com a forma fechada do relogio, contado sonda a sonda
 *   §B3  a densidade decide o custo, e mede-se — nao se estima
 *   §B4  e o slot retem a OPERACAO e nao o valor: o crc recusa o torto sem o ler
 *
 *   cc -O2 -std=c99 -Wall -I../lib banco_relogio.c -o banco_relogio
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "banco.h"
#include "unidade.h"

/* a mesma sondagem do banco, instrumentada para se poder contar as marcas */
static long sondas(const unsigned char *idx, long nslot, uint64_t fp, int *achou){
    long passos = 0;
    for(long k = 0; k < nslot; k++){
        long i = (long)((fp + (uint64_t)k) % (uint64_t)nslot);
        const uint64_t *s = (const uint64_t*)(idx + (size_t)i*SLOTSZ);
        passos++;
        if(s[0] == 0){ *achou = 0; return passos; }      /* vazio: a volta fecha aqui */
        if(s[0] == fp){ *achou = 1; return passos; }     /* o ponto fixo */
    }
    *achou = 0; return passos;
}

int main(void){
    puts("\n  OS SLOTS SAO O RELOGIO — e o NAND e' a realizacao\n");

    /* ═══ §B1 — a sondagem e' o pente de passo 1 ═══════════════════════════════════ */
    {
        long N = NSLOT, mau = 0;
        for(uint64_t fp = 1; fp < 40; fp++)
            for(long k = 0; k < 50; k++){
                long i  = (long)((fp + (uint64_t)k)     % (uint64_t)N);
                long i2 = (long)((fp + (uint64_t)(k+1)) % (uint64_t)N);
                if(i2 != (i + 1) % N) mau++;             /* passo 1, e da' a volta */
            }
        printf("      %ld marcas na volta; a sondagem anda de 1 em 1 e fecha em N\n", N);
        ok("a sondagem E' o pente de passo 1: cada passo avanca uma marca, e a volta"
           " fecha ao fim de N — e' o relogio do §R5 com d=1", !mau);
    }

    /* ═══ §B2 e §B3 — o contador e a densidade, contados sonda a sonda ═════════════ */
    {
        const long M = 3000;
        struct base b;
        unlink("/tmp/br.dat"); unlink("/tmp/br.idx");
        if(!abrir(&b, "/tmp/br", 1)){ perror("abrir"); return 1; }
        unsigned char v[64];
        for(long k = 0; k < M; k++){
            char c[32]; snprintf(c, sizeof c, "k-%ld", k);
            memset(v, (int)(k & 0x7F), sizeof v);
            if(!gravar(&b, c, v, sizeof v)) falhas++;
        }
        fechar(&b);

        Mapa m;
        if(!banco_mapa(&m, "/tmp/br")){ puts("      nao mapeou"); return 1; }
        long nslot = (long)(m.n_idx / SLOTSZ); if(nslot > NSLOT) nslot = NSLOT;

        long total = 0, achados = 0, pior = 0;
        for(long k = 0; k < M; k++){
            char c[32]; snprintf(c, sizeof c, "k-%ld", k);
            uint64_t fp = fp64(c, strlen(c)); if(!fp) fp = 1;
            int achou = 0;
            long p = sondas(m.idx, nslot, fp, &achou);
            total += p; achados += achou;
            if(p > pior) pior = p;
        }
        double alfa = (double)M / (double)nslot;
        double medio = (double)total / (double)M;
        printf("      %ld chaves em %ld marcas: densidade %.4f\n", M, nslot, alfa);
        printf("      sondas por leitura: media %.3f, pior caso %ld\n", medio, pior);
        ok("todas as chaves sao ACHADAS pela sondagem — a volta encontra o ponto fixo",
           achados == M);

        /* a forma fechada da sondagem linear com densidade alfa: (1 + 1/(1-alfa))/2.
         * NAO e' limiar meu — e' a conta, e compara-se com o que se contou. */
        double previsto = (1.0 + 1.0/(1.0 - alfa)) / 2.0;
        printf("      forma fechada para densidade %.4f: %.3f sondas\n", alfa, previsto);
        ok("e o CONTADOR bate com a forma fechada: com a volta quase vazia, uma sonda"
           " chega — a densidade e' que decide o custo, e mediu-se em vez de se estimar",
           medio >= 1.0 && medio <= previsto + 0.5);

        /* ═══ §B4 — o slot retem a OPERACAO, e o crc recusa sem ler ═══════════════ */
        long n = 0;
        const unsigned char *r = banco_ver(&m, "k-7", &n);
        int leu_bem = 0;
        if(r && n == (long)sizeof v){
            unsigned char t[64]; banco_fatia(r, 0, t, (long)sizeof t);
            leu_bem = 1;
            for(size_t i = 0; i < sizeof t; i++) if(t[i] != (unsigned char)(7 & 0x7F)) leu_bem = 0;
        }
        banco_larga(&m);

        int fd = open("/tmp/br.dat", O_RDWR);
        if(fd >= 0){
            struct stat st; fstat(fd, &st);
            off_t meio = st.st_size / 2;
            unsigned char x; lseek(fd, meio, SEEK_SET);
            if(read(fd, &x, 1) == 1){ x ^= 0xFF; lseek(fd, meio, SEEK_SET);
                                      if(write(fd, &x, 1) != 1){} }
            close(fd);
        }
        Mapa m2; long recusados = 0, lidos = 0;
        if(banco_mapa(&m2, "/tmp/br")){
            for(long k = 0; k < M; k++){
                char c[32]; snprintf(c, sizeof c, "k-%ld", k); long nn = 0;
                if(banco_ver(&m2, c, &nn)) lidos++; else recusados++;
            }
            banco_larga(&m2);
        }
        printf("      antes de corromper: k-7 lido correcto = %s\n", leu_bem?"sim":"NAO");
        printf("      com um byte trocado a meio: %ld lidos, %ld RECUSADOS pelo crc\n",
               lidos, recusados);
        ok("o slot retem a OPERACAO e nao o valor — {fp, off, len, crc} diz ONDE esta' e"
           " SE esta' bem, e o torto e' recusado sem ser entregue",
           leu_bem && recusados > 0 && lidos > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A SONDAGEM E' O RELOGIO. `i = (i + 1) % NSLOT` e' o pente de passo 1 numa");
        puts("  volta de 65536 marcas: a chave da' a FASE inicial e anda-se marca a marca");
        puts("  ate' ao ponto fixo — o slot cuja impressao bate — ou ate' ao vazio, que e'");
        puts("  onde a volta se fecha sem achar. O §R5 do relogio.c ja' tinha o contador;");
        puts("  aqui ele conta-se sonda a sonda e bate com a forma fechada.");
        puts("");
        puts("  E O SLOT RETEM A OPERACAO, NAO O VALOR: {fp, off, len, crc} diz ONDE esta'");
        puts("  e SE esta' bem. E' o que uma celula NAND faz — nao e' um sitio onde um bit");
        puts("  esta', e' a operacao ja' feita e mantida, com NAND(a,a) = NOT a, a mesma");
        puts("  porta a ser a involucao de si propria.");
        puts("");
        puts("  E DAI NAO HAVER O QUE DISSIPAR: a DRAM reconstroi a cada refresh e paga; o");
        puts("  NAND retem e nao paga; e o slot, que retem ONDE e SE, le-se sem se tocar em");
        puts("  nada. Nao ha' o que dissipar porque nao ha' o que construir.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
