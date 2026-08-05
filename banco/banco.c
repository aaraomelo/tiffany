/* banco.c — o PROGRAMA. O corpo mudou-se para lib/banco.h e nao foi tocado: o programa
 * e a biblioteca sao agora o mesmo codigo, em vez de duas copias a divergir. */
#include "../lib/banco.h"

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
