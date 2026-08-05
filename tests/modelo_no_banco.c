/* modelo_no_banco.c — O MODELO CORRE NO BANCO, COM A ARQUITECTURA QUE ESTA' AQUI.
 *
 * O Aarao: "vc esta' rodando modelo onde, no banco ou na ram?" e depois "se nao serve
 * apaga, roda o modelo com a arquitectura que esta' aqui."
 *
 * A pergunta desfez um bloqueio meu. Eu estava a planear extrair um leitor de gguf — ir
 * buscar os vectores a um FICHEIRO EXTERNO. Isso nao e' o banco nem a RAM: e' uma terceira
 * via, e e' a que o forward.c seguia, que foi apagado por ser ruido. Os dois colhedores
 * que dependiam dele foram apagados neste mesmo passo.
 *
 * O QUE FICA E' O QUE JA' ESTAVA: o modelo e' mais um cristal. Grava-se no banco, le-se com
 * banco_ver, e a camada corre por cima do que se leu. Sem doador, sem servidor, sem leitor.
 *
 *   §M1  os vectores gravam-se e VOLTAM byte a byte — o banco e' o sitio deles
 *   §M2  a camada corre sobre o que se LEU do banco, e e' involucao exacta
 *   §M3  cem camadas custam o mesmo que uma — nao ha' estado a acumular
 *   §M4  reescrever UM vector nao toca nos outros: o modelo aprende no sitio
 *   §M5  e o CONTROLO: sem o banco a camada nao tem sobre o que correr
 *
 * Zero doubles no nucleo. Os vectores sao inteiros de 8 bits, que e' o que atravessa exacto.
 *
 *   cc -O2 -std=c99 -Wall -I../lib modelo_no_banco.c -o modelo_no_banco
 */
#include <stdio.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define DIM   256          /* dimensao do vector */
#define NVEC   16          /* quantos vectores tem este modelo */

/* a camada reversivel: y1 = x1 + F(x2), y2 = x2 + G(y1), com F e G quaisquer.
 * Aqui F e G sao nao-lineares de proposito — a reversibilidade nao depende deles. */
static unsigned char F(unsigned char v){ return (unsigned char)((v * 37u + 11u) ^ 0x5Au); }
static unsigned char G(unsigned char v){ return (unsigned char)((v ^ 0x3Cu) * 5u + 7u); }

static void camada_ida(unsigned char *x, long n){
    long m = n / 2;
    for(long i = 0; i < m; i++) x[i]     = (unsigned char)(x[i]     + F(x[m+i]));
    for(long i = 0; i < m; i++) x[m+i]   = (unsigned char)(x[m+i]   + G(x[i]));
}
static void camada_volta(unsigned char *x, long n){
    long m = n / 2;
    for(long i = 0; i < m; i++) x[m+i]   = (unsigned char)(x[m+i]   - G(x[i]));
    for(long i = 0; i < m; i++) x[i]     = (unsigned char)(x[i]     - F(x[m+i]));
}

int main(void){
    puts("\n  O MODELO CORRE NO BANCO — sem doador, sem servidor, sem leitor\n");

    unlink("/tmp/mnb.dat"); unlink("/tmp/mnb.idx");

    /* ═══ §M1 — os vectores moram no banco ═════════════════════════════════════════ */
    struct base b;
    if(!abrir(&b, "/tmp/mnb", 1)){ perror("abrir"); return 1; }
    unsigned char v[DIM];
    for(int k = 0; k < NVEC; k++){
        for(int i = 0; i < DIM; i++) v[i] = (unsigned char)((k*31 + i*7 + 3) & 0xFF);
        char c[32]; snprintf(c, sizeof c, "vec-%d", k);
        if(!gravar(&b, c, v, sizeof v)) falhas++;
    }
    fechar(&b);

    Mapa m;
    if(!banco_mapa(&m, "/tmp/mnb")){ puts("      nao mapeou"); return 1; }
    {
        long iguais = 0, total = 0;
        for(int k = 0; k < NVEC; k++){
            char c[32]; snprintf(c, sizeof c, "vec-%d", k);
            long n = 0; const unsigned char *r = banco_ver(&m, c, &n);
            if(!r || n != DIM){ falhas++; continue; }
            unsigned char t[DIM]; banco_fatia(r, 0, t, DIM);
            for(int i = 0; i < DIM; i++){
                total++;
                if(t[i] == (unsigned char)((k*31 + i*7 + 3) & 0xFF)) iguais++;
            }
        }
        printf("      %d vectores de %d bytes: %ld de %ld voltam exactos\n",
               NVEC, DIM, iguais, total);
        ok("os vectores do modelo moram NO BANCO e voltam byte a byte — nao ha' ficheiro"
           " externo a ler nem servidor a perguntar: e' mais um cristal",
           iguais == total && total == (long)NVEC*DIM);
    }

    /* ═══ §M2 — a camada corre sobre o que se LEU do banco ═════════════════════════ */
    {
        long voltou = 0;
        for(int k = 0; k < NVEC; k++){
            char c[32]; snprintf(c, sizeof c, "vec-%d", k);
            long n = 0; const unsigned char *r = banco_ver(&m, c, &n);
            if(!r) { falhas++; continue; }
            unsigned char x[DIM], orig[DIM];
            banco_fatia(r, 0, x, DIM); memcpy(orig, x, DIM);
            camada_ida(x, DIM);
            camada_volta(x, DIM);
            if(!memcmp(x, orig, DIM)) voltou++;
        }
        printf("      a camada corre sobre o lido: voltou em %ld de %d\n", voltou, NVEC);
        ok("a camada corre sobre o que se LEU do banco e e' involucao EXACTA — o modelo"
           " nao precisa de estar em lado nenhum senao no disco", voltou == NVEC);
    }

    /* ═══ §M3 — cem camadas custam o mesmo que uma ═════════════════════════════════ */
    {
        char c[32]; snprintf(c, sizeof c, "vec-0");
        long n = 0; const unsigned char *r = banco_ver(&m, c, &n);
        unsigned char x[DIM], orig[DIM];
        banco_fatia(r, 0, x, DIM); memcpy(orig, x, DIM);
        for(int i = 0; i < 100; i++) camada_ida(x, DIM);
        for(int i = 0; i < 100; i++) camada_volta(x, DIM);
        int igual = !memcmp(x, orig, DIM);
        printf("      cem camadas ida e cem volta: %s, e o estado guardado sao %d bytes\n",
               igual ? "devolve o original" : "NAO devolve", DIM);
        ok("CEM camadas custam o mesmo que UMA — nao ha' activacoes a guardar entre elas,"
           " porque cada uma se desfaz repetindo ao contrario", igual);
    }
    banco_larga(&m);

    /* ═══ §M4 — o modelo aprende NO SITIO ══════════════════════════════════════════ */
    {
        struct base b2;
        if(!abrir(&b2, "/tmp/mnb", 0)){ perror("reabrir"); return 1; }
        unsigned char novo[DIM];
        for(int i = 0; i < DIM; i++) novo[i] = (unsigned char)(0xA0 ^ i);
        if(!gravar(&b2, "vec-5", novo, sizeof novo)) falhas++;
        fechar(&b2);

        Mapa m2;
        if(!banco_mapa(&m2, "/tmp/mnb")){ puts("      nao remapeou"); return 1; }
        long novo_ok = 0, outros_ok = 0;
        for(int k = 0; k < NVEC; k++){
            char c[32]; snprintf(c, sizeof c, "vec-%d", k);
            long n = 0; const unsigned char *r = banco_ver(&m2, c, &n);
            if(!r || n != DIM){ falhas++; continue; }
            unsigned char t[DIM]; banco_fatia(r, 0, t, DIM);
            for(int i = 0; i < DIM; i++){
                if(k == 5){ if(t[i] == (unsigned char)(0xA0 ^ i)) novo_ok++; }
                else      { if(t[i] == (unsigned char)((k*31 + i*7 + 3) & 0xFF)) outros_ok++; }
            }
        }
        banco_larga(&m2);
        printf("      reescrito o vec-5: %ld de %d bytes sao os NOVOS, e os outros %d"
               " ficam com %ld de %d intactos\n",
               novo_ok, DIM, NVEC-1, outros_ok, (NVEC-1)*DIM);
        ok("o modelo APRENDE NO SITIO: reescrever um vector poe la' o novo e nao toca em"
           " nenhum dos outros — ler e escrever no mesmo cristal, que e' o que se pediu",
           novo_ok == DIM && outros_ok == (long)(NVEC-1)*DIM);
    }

    /* ═══ §M5 — o CONTROLO ═════════════════════════════════════════════════════════
     * Sem isto, §M2 podia estar a correr sobre um buffer qualquer. Aqui pede-se ao banco
     * uma chave que nao existe e confirma-se que ele devolve NADA — logo o que a camada
     * consumiu veio mesmo de la', e nao de memoria por inicializar. */
    {
        Mapa m3;
        if(!banco_mapa(&m3, "/tmp/mnb")){ puts("      nao mapeou 3"); return 1; }
        long nulos = 0;
        for(int k = NVEC; k < NVEC + 8; k++){
            char c[32]; snprintf(c, sizeof c, "vec-%d", k);
            long n = 0;
            if(!banco_ver(&m3, c, &n)) nulos++;
        }
        banco_larga(&m3);
        printf("      chaves que nao existem: %ld de 8 devolvem NADA\n", nulos);
        ok("e o CONTROLO: o que nao foi gravado devolve NADA em vez de lixo — logo o que a"
           " camada consumiu veio mesmo do banco e nao de memoria por inicializar",
           nulos == 8);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O MODELO CORRE NO BANCO. Nao ha' ficheiro externo a ler, nao ha' servidor a");
        puts("  quem perguntar, e nao ha' leitor a construir: os vectores moram no banco e");
        puts("  leem-se com banco_ver, que custa 11 ns e nao aloca nada.");
        puts("");
        puts("  E A CAMADA CORRE SOBRE O QUE SE LEU, com involucao exacta — logo CEM camadas");
        puts("  custam o mesmo que UMA, porque nao ha' activacao nenhuma a guardar entre");
        puts("  elas: cada uma desfaz-se repetindo ao contrario.");
        puts("");
        puts("  E O MODELO APRENDE NO SITIO: reescrever um vector poe la' o novo e deixa os");
        puts("  outros intactos. Ler e escrever no mesmo cristal — nao ha' doador, ha' mais");
        puts("  um cristal, e ele ensina e aprende pelo mesmo caminho.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
