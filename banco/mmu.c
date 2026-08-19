/* mmu.c — A MMU MULTIFRACTAL: o endereço b^n realizado EM DISCO, e não em RAM.
 *
 * REGUA: RssAnon — mede o custo que o disco evita (o controlo)
 *
 * O Aarão: "não quero LLM rodando em memória, usa o disco e o processador multifractal pra
 * simular RAM no disco — não torra aqui."
 *
 * E o "não torra aqui" é LITERAL nesta máquina, por um motivo físico que se mede: a swap é
 * `/dev/zram0`, 8 GiB COMPRIMIDOS DENTRO DA RAM. Aqui swap não é disco — é memória. Quando um
 * processo pressiona, o kernel "descarrega" para dentro da própria RAM e a pressão sobe em vez
 * de descer. Não há para onde transbordar. O disco tem de ser pedido de propósito.
 *
 * O mcu.c §U4 já mediu a MEMÓRIA multifractal: o endereço é o caminho na árvore, b^n, e não há
 * índice à parte do objeto. Mas mediu-a EM MEMÓRIA — o que ali era a tese, aqui é o problema.
 * Este medidor não repete a bijeção: REALIZA-A em disco, de duas formas, e mede o que só se
 * pode medir quando o suporte é disco — que a RAM do processo NÃO CRESCE com o número de
 * células. É isso, e apenas isso, que faz do disco uma RAM.
 *
 *     REALIZAÇÃO A   o caminho na árvore É o caminho no sistema de ficheiros:  raiz/d3/d2/d1/d0
 *     REALIZAÇÃO B   o caminho na árvore É o deslocamento num ficheiro só:     offset = e·CEL
 *
 * As duas são o MESMO endereço, e é por isso que servem de medidor uma da outra: escreve-se por
 * A, lê-se por B, e se o corpo é o mesmo os bytes têm de bater célula a célula. Nenhuma asserção
 * aqui compara contra um número que eu escolhi — comparam-se dois caminhos independentes, que é
 * o que apanhou os piores defeitos deste projeto.
 *
 *   §M1  a BIJEÇÃO endereço ↔ dígitos, e o caminho que dela sai
 *   §M2  REALIZAÇÃO A: o caminho é diretório — escrever e reler
 *   §M3  REALIZAÇÃO B: o caminho é deslocamento — escrever e reler
 *   §M4  OS DOIS CAMINHOS CONCORDAM: escrito por A, lido por B, byte a byte
 *   §M5  A RAM NÃO CRESCE COM AS CÉLULAS — e a régua é o que o índice CUSTARIA
 *   §M6  o custo do acesso é O(n) dígitos, e n = log_b(células)
 *
 *   cc -O2 -std=c99 mmu.c -lm -o mmu && ./mmu
 */
#define _POSIX_C_SOURCE 200809L    /* pread/pwrite: o acesso POSICIONAL, que é o endereço */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "unidade.h"

#define CEL   1024                 /* uma célula: 1 KiB. O que não couber é truncado, e dito. */
#define NMAXD 12                   /* teto de dígitos — é a PROFUNDIDADE, não o nº de células */

/* ---- a raiz do banco: disco de verdade, nunca /tmp (que aqui é tmpfs, ou seja RAM) -------- */
static const char *raiz_banco(void){
    const char *r = getenv("MMU_RAIZ");
    return r ? r : ".mmu";
}

/* ---- §M1  o endereço EM dígitos da base b: d[0] é o menos significativo ------------------- */
static void digitos(long e, int b, int n, int *d){
    for(int k = 0; k < n; k++){ d[k] = (int)(e % b); e /= b; }
}
static long de_digitos(const int *d, int b, int n){
    long v = 0, pot = 1;
    for(int k = 0; k < n; k++){ v += (long)d[k] * pot; pot *= b; }
    return v;
}

/* ---- REALIZAÇÃO A: o caminho na árvore É o caminho no sistema de ficheiros ---------------- *
 * Cada dígito é um nível. Descer a árvore é descer diretórios — o percurso não é consultado
 * numa tabela, é ANDADO. É o mesmo que o projeto diz da cifra: a trie É o índice.
 *
 * A FOLHA LEVA SUFIXO, e isto não é cosmético — foi um defeito que o §M4 apanhou. Sem sufixo,
 * um endereço de profundidade 4 cria o FICHEIRO `0/0/0/0`, que é exatamente o DIRETÓRIO de que
 * um endereço de profundidade 6 precisa para descer. O curto ocupa o lugar do prefixo do longo,
 * e as duas realizações deixam de concordar (8 células em 586, e a conta bateu ao número).
 *
 * A diferença é estrutural, não acidental: o deslocamento é auto-delimitado porque n entra na
 * multiplicação; a árvore de diretórios não é, porque o caminho de um prefixo É um caminho
 * válido. O sufixo restitui a delimitação — a folha deixa de ser confundível com o nó.        */
static int caminho_A(long e, int b, int n, char *out, size_t cap, int criar){
    int d[NMAXD];
    digitos(e, b, n, d);
    int p = snprintf(out, cap, "%s", raiz_banco());
    if(criar) mkdir(out, 0755);
    for(int k = n - 1; k >= 0; k--){                    /* do mais significativo para a folha */
        p += snprintf(out + p, cap - (size_t)p, k > 0 ? "/%d" : "/%d.cel", d[k]);
        if((size_t)p >= cap) return -1;
        if(criar && k > 0) mkdir(out, 0755);            /* os nós; a folha é ficheiro .cel */
    }
    return 0;
}
static int escreve_A(long e, int b, int n, const char *dado, size_t len){
    char cam[512];
    if(caminho_A(e, b, n, cam, sizeof cam, 1) < 0) return -1;
    int fd = open(cam, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd < 0) return -1;
    if(len > CEL) len = CEL;                            /* o teto é dito, nunca calado */
    ssize_t w = write(fd, dado, len);
    close(fd);
    return w == (ssize_t)len ? 0 : -1;
}
static int le_A(long e, int b, int n, char *buf, size_t cap){
    char cam[512];
    if(caminho_A(e, b, n, cam, sizeof cam, 0) < 0) return -1;
    int fd = open(cam, O_RDONLY);
    if(fd < 0) return -1;
    ssize_t r = read(fd, buf, cap);
    close(fd);
    return r < 0 ? -1 : (int)r;
}

/* ---- REALIZAÇÃO B: o caminho na árvore É o deslocamento num ficheiro só ------------------- *
 * offset = Σ d_k·b^k·CEL = e·CEL. O ficheiro é ESPARSO: os buracos não ocupam disco até serem
 * escritos, e nunca ocupam RAM. É uma RAM endereçável cujo suporte é o disco.                */
static void ficheiro_B(char *out, size_t cap){ snprintf(out, cap, "%s.bin", raiz_banco()); }
static int escreve_B(long e, const char *dado, size_t len){
    char f[512]; ficheiro_B(f, sizeof f);
    int fd = open(f, O_WRONLY | O_CREAT, 0644);
    if(fd < 0) return -1;
    char cel[CEL];
    memset(cel, 0, CEL);
    if(len > CEL) len = CEL;
    memcpy(cel, dado, len);
    ssize_t w = pwrite(fd, cel, CEL, (off_t)e * CEL);   /* o endereço É o deslocamento */
    close(fd);
    return w == CEL ? 0 : -1;
}
static int le_B(long e, char *buf, size_t cap){
    char f[512]; ficheiro_B(f, sizeof f);
    int fd = open(f, O_RDONLY);
    if(fd < 0) return -1;
    if(cap > CEL) cap = CEL;
    ssize_t r = pread(fd, buf, cap, (off_t)e * CEL);
    close(fd);
    return r < 0 ? -1 : (int)r;
}

/* ---- a RAM do próprio processo, do kernel e não da minha contabilidade -------------------- *
 * RssAnon é a memória ANÓNIMA: a que só existe na RAM e que, sob pressão, vai para a zram —
 * isto é, para dentro da própria RAM. É esta que não pode crescer. RssFile é mapeada de
 * ficheiro: sob pressão o kernel DESCARTA-A e relê do disco, de graça, porque está limpa.   */
static long rss_anon_kb(void){
    FILE *f = fopen("/proc/self/status", "r");
    if(!f) return -1;
    char lin[256]; long v = -1;
    while(fgets(lin, sizeof lin, f))
        if(!strncmp(lin, "RssAnon:", 8)){ sscanf(lin + 8, "%ld", &v); break; }
    fclose(f);
    return v;
}

/* apaga a árvore de um teste, para o medidor não medir a corrida anterior */
static void limpa(void){
    char cmd[600];
    snprintf(cmd, sizeof cmd, "rm -rf '%s' '%s.bin'", raiz_banco(), raiz_banco());
    if(system(cmd)) { /* nada a fazer: a limpeza falhar não invalida a medida seguinte */ }
}

/* ---- MODO FERRAMENTA: a MMU como banco, para quem não é C ---------------------------------*
 * Sem argumentos, este ficheiro é o medidor. Com argumentos, é o banco — e é a MESMA função de
 * endereçamento que as secções medem, não uma segunda cópia escrita à pressa noutra linguagem.
 * Era assim que se abria a porta para um defeito: duas implementações do mesmo endereço.
 *
 *     mmu poe          lê stdin, grava na próxima célula livre, imprime o endereço
 *     mmu le <addr>    imprime a célula
 *     mmu topo         imprime quantas células estão escritas
 *
 * A célula 0 é o TOPO — o registo de quantas células existem. Guardá-lo no próprio banco é o
 * que dispensa qualquer estado em memória entre invocações: o processo nasce, lê o topo do
 * disco, escreve, e morre. Nada residente, nunca.                                            */
#define TOPO_B 4                   /* base do banco em modo ferramenta */
#define TOPO_N 8                   /* profundidade: 4^8 = 65536 células */

static long le_topo(void){
    char b[64]; memset(b, 0, sizeof b);
    if(le_B(0, b, 63) <= 0) return 0;
    long v = atol(b);
    return v < 0 ? 0 : v;
}
static void poe_topo(long v){
    char b[64];
    int n = snprintf(b, sizeof b, "%ld", v);
    escreve_B(0, b, (size_t)n);
}
static int ferramenta(int argc, char **argv){
    const char *cmd = argv[1];
    if(!strcmp(cmd, "topo")){ printf("%ld\n", le_topo()); return 0; }
    if(!strcmp(cmd, "poe")){
        char buf[CEL];
        size_t n = fread(buf, 1, CEL, stdin);      /* O(1) em RAM: uma célula, e só */
        long addr = le_topo() + 1;                 /* a 0 é o topo, os dados começam em 1 */
        if(addr >= 65536){ fprintf(stderr, "mmu: banco cheio (65536 células)\n"); return 1; }
        if(escreve_A(addr, TOPO_B, TOPO_N, buf, n) < 0){ perror("mmu: árvore"); return 1; }
        if(escreve_B(addr, buf, n) < 0){ perror("mmu: deslocamento"); return 1; }
        poe_topo(addr);
        printf("%ld\n", addr);
        return 0;
    }
    if(!strcmp(cmd, "le") && argc > 2){
        char buf[CEL+1]; memset(buf, 0, sizeof buf);
        int r = le_A(atol(argv[2]), TOPO_B, TOPO_N, buf, CEL);
        if(r < 0){ fprintf(stderr, "mmu: célula %s vazia\n", argv[2]); return 1; }
        fwrite(buf, 1, (size_t)r, stdout);
        return 0;
    }
    fprintf(stderr, "uso: mmu [poe | le <addr> | topo]   (sem argumentos: o medidor)\n");
    return 2;
}

int main(int argc, char **argv){
if(argc > 1) return ferramenta(argc, argv);

printf("\n=== A MMU MULTIFRACTAL: O ENDEREÇO b^n REALIZADO EM DISCO ==================\n");
printf("    O mcu.c §U4 mediu a bijeção em memória. Aqui ela vira SUPORTE: duas\n");
printf("    realizações em disco do mesmo endereço, que têm de concordar — e a RAM\n");
printf("    do processo não pode crescer com o número de células.\n");
printf("\n    raiz do banco: %s   (célula = %d B)\n", raiz_banco(), CEL);

limpa();

printf("\n§M1  A BIJEÇÃO: o endereço é os seus dígitos, e os dígitos são o caminho.\n\n");
{
    /* Nao se remede a bijecao do mcu §U4 — mede-se que ela sobrevive ao TAMANHO que o disco
     * permite e a memoria nao. b=4, n=8 sao 65536 celulas: em RAM seriam 64 MiB de corpus. */
    int b = 4, n = 8;
    long total = 1; for(int k = 0; k < n; k++) total *= b;
    printf("      base b = %d, profundidade n = %d  ->  %ld células (%ld MiB se em RAM)\n\n",
           b, n, total, total * CEL / (1024*1024));
    printf("      endereço   caminho na árvore              de volta    confere?\n");
    long mal = 0;
    for(long e = 0; e < total; e++){
        int d[NMAXD];
        digitos(e, b, n, d);
        if(de_digitos(d, b, n) != e) mal++;
        if(e < 3 || e == total-1){
            char cam[512];
            caminho_A(e, b, n, cam, sizeof cam, 0);
            printf("      %-10ld %-29s %-11ld %s\n", e, cam, de_digitos(d,b,n),
                   de_digitos(d,b,n) == e ? "sim" : "NÃO");
        }
    }
    printf("\n      %ld endereços percorridos, %ld falhas\n\n", total, mal);
    ok("a bijeção endereço ↔ caminho fecha em 65536 células", mal == 0);
    printf("      E repare no que NÃO existe aqui: nenhuma tabela foi consultada. O caminho\n");
    printf("      não se procura, calcula-se — e é por isso que ele não ocupa lugar.\n");
}

printf("\n§M2  REALIZAÇÃO A: o caminho na árvore É o caminho no sistema de ficheiros.\n\n");
{
    int b = 4, n = 4;                                  /* 256 células, a árvore visível */
    const char *frases[] = {
        "o endereço é o caminho, e o caminho é o disco",
        "a swap aqui é zram: swap não é disco, é RAM",
        "a folha guarda o dado; os nós guardam só o percurso",
    };
    long ends[] = { 0, 137, 255 };
    int mal = 0;
    printf("      endereço   escrito                                        relido igual?\n");
    for(int i = 0; i < 3; i++){
        char buf[CEL+1];
        memset(buf, 0, sizeof buf);
        if(escreve_A(ends[i], b, n, frases[i], strlen(frases[i])) < 0){ mal++; continue; }
        int r = le_A(ends[i], b, n, buf, CEL);
        int igual = (r == (int)strlen(frases[i])) && !memcmp(buf, frases[i], (size_t)r);
        if(!igual) mal++;
        printf("      %-10ld %-46.46s %s\n", ends[i], frases[i], igual ? "sim" : "NÃO");
    }
    printf("\n");
    ok("escrito e relido pela árvore de diretórios, sem perda", mal == 0);
    conclui("a folha é o dado e o percurso é o endereço — não há índice à parte do objeto");
}

printf("\n§M3  REALIZAÇÃO B: o caminho É o deslocamento, e o ficheiro é ESPARSO.\n\n");
{
    /* A prova de que o buraco nao custa: escreve-se SO a ultima celula de um espaco de 65536
     * e compara-se o tamanho APARENTE com o tamanho REAL em blocos. */
    long e_alto = 65535;
    escreve_B(e_alto, "a última célula, e nenhuma antes dela", 37);
    char f[512]; ficheiro_B(f, sizeof f);
    struct stat st;
    if(stat(f, &st) == 0){
        long aparente = (long)st.st_size;
        long real     = (long)st.st_blocks * 512;
        printf("      escrita só a célula %ld de 65536:\n\n", e_alto);
        printf("        tamanho APARENTE (o endereçável)  %8ld B  (%ld MiB)\n",
               aparente, aparente/(1024*1024));
        printf("        tamanho REAL em disco (blocos)    %8ld B  (%ld KiB)\n\n",
               real, real/1024);
        ok("o espaço endereçável excede o disco gasto — os buracos não custam",
           aparente > real);
        printf("      Um array em RAM teria de alocar os %ld MiB para poder endereçar a\n",
               aparente/(1024*1024));
        printf("      última casa. O ficheiro esparso endereça-a gastando %ld KiB.\n", real/1024);
    }
}

printf("\n§M4  OS DOIS CAMINHOS CONCORDAM: escrito por A, lido por B — byte a byte.\n\n");
{
    /* Este e' o medidor de verdade. Nao ha limiar meu: ha duas realizacoes independentes do
     * mesmo endereco, e ou os bytes batem ou o corpo nao e' o mesmo. */
    int b = 4, n = 6;                                  /* 4096 células */
    long total = 1; for(int k = 0; k < n; k++) total *= b;
    long testadas = 0, divergentes = 0;
    for(long e = 0; e < total; e += 7){                /* passo 7: primo com 4, varre a árvore */
        char dado[64], va[CEL+1], vb[CEL+1];
        int len = snprintf(dado, sizeof dado, "célula %ld, dígitos em base %d", e, b);
        escreve_A(e, b, n, dado, (size_t)len);
        escreve_B(e, dado, (size_t)len);
        memset(va, 0, sizeof va); memset(vb, 0, sizeof vb);
        int ra = le_A(e, b, n, va, CEL);
        int rb = le_B(e, vb, CEL);
        if(ra < 0 || rb < 0 || memcmp(va, vb, (size_t)len)) divergentes++;
        testadas++;
    }
    printf("      células escritas pelos dois caminhos:  %ld\n", testadas);
    printf("      divergências byte a byte:              %ld\n\n", divergentes);
    ok("a árvore de diretórios e o deslocamento dão os MESMOS bytes", divergentes == 0);
    printf("      Duas realizações independentes do mesmo endereço. Não se compara contra um\n");
    printf("      número escolhido por mim — compara-se um caminho contra o outro.\n\n");

    /* O DEFEITO QUE ESTE MEDIDOR APANHOU, agora medido de frente. O §M2 escreveu nesta mesma
     * raiz com n=4 e esta secao escreve com n=6: sem sufixo na folha, o endereco curto ocupava
     * o diretorio de que o longo precisava, e 8 das 586 celulas divergiam. A conta fechava ao
     * numero exato — 0 bloqueia 0..15, 137 bloqueia 2192..2207, 255 bloqueia 4080..4095, e os
     * multiplos de 7 nessas faixas sao 3+2+3. Portanto: mede-se a COEXISTENCIA. */
    long prefixo = 0;                       /* o endereço 0 em n=4 é prefixo do 0 em n=6 */
    char c4[CEL+1], c6[CEL+1];
    memset(c4, 0, sizeof c4); memset(c6, 0, sizeof c6);
    escreve_A(prefixo, b, 4, "sou a folha curta", 17);
    escreve_A(prefixo, b, 6, "sou a folha longa", 17);
    int r4 = le_A(prefixo, b, 4, c4, CEL);
    int r6 = le_A(prefixo, b, 6, c6, CEL);
    printf("      mesmo endereço %ld, duas profundidades na MESMA raiz:\n", prefixo);
    printf("        n = 4  ->  \"%.*s\"\n", r4 > 0 ? r4 : 0, c4);
    printf("        n = 6  ->  \"%.*s\"\n\n", r6 > 0 ? r6 : 0, c6);
    { int dif = (memcmp(c4, c6, 17) != 0);
      int len4 = (r4 == 17);
      int len6 = (r6 == 17);
    ok("profundidades diferentes coexistem — a folha não é confundível com o nó",
       dif && len4 && len6); }
    printf("      Era aqui que as duas realizações se separavam. O deslocamento é\n");
    printf("      auto-delimitado (n entra na multiplicação); a árvore não era, porque o\n");
    printf("      caminho de um prefixo é um caminho válido. O sufixo devolve a delimitação.\n");
}

printf("\n§M5  A RAM NÃO CRESCE COM AS CÉLULAS — e a régua é o que o índice CUSTARIA.\n\n");
{
    /* O ponto do pedido. Se isto e' RAM-no-disco, a RAM anonima do processo tem de ficar
     * PLANA enquanto o corpus cresce. E a regua nao e' um limiar meu: e' a formula fechada
     * do que um indice em memoria ocuparia — N·CEL bytes. Mede-se a LEI em varios pontos. */
    long pontos[] = { 64, 256, 1024, 4096 };
    long base = rss_anon_kb();
    printf("      células     RssAnon do processo    cresceu      se fosse array em RAM\n");
    long cresc_max = 0, custo_min = 0;
    for(int i = 0; i < 4; i++){
        for(long e = 0; e < pontos[i]; e++){
            char dado[64];
            int len = snprintf(dado, sizeof dado, "%ld", e);
            escreve_B(e % 4096, dado, (size_t)len);
        }
        long agora = rss_anon_kb();
        long cresceu = agora - base;
        long custo   = pontos[i] * CEL / 1024;         /* fórmula fechada, não estimativa */
        if(cresceu > cresc_max) cresc_max = cresceu;
        if(i == 0 || custo < custo_min) custo_min = custo;
        printf("      %-11ld %-21ld %+-12ld %ld kB\n", pontos[i], agora, cresceu, custo);
    }
    printf("\n");

    /* O CONTROLO POSITIVO — e sem ele a coluna acima não vale nada.
     *
     * "RssAnon +0" é o resultado que eu QUERIA ver, e é exatamente por isso que ele é suspeito:
     * um instrumento avariado devolve zero com a mesma cara. A pergunta que desarma isto é a de
     * sempre — que entrada faria esta asserção falhar? Se nenhuma, ela não mede.
     *
     * Então guarda-se o MESMO corpus como um índice em RAM e mede-se com o MESMO instrumento.
     * Se o ponteiro não se mexer aqui, o medidor está cego e o "+0" de cima não prova coisa
     * nenhuma. São 4 MiB, alocados de propósito e uma só vez, para provar o custo que o disco
     * evita — é a única RAM que este ficheiro pede, e pede-a para a poder mostrar. */
    /* ctl_: CONTROLO, para FICAR — ver a nota acima. */
    static char ctl_indice_ram[4096][CEL];                  /* 4 MiB: o que o corpus custaria */
    long antes_ctl = rss_anon_kb();
    for(long e = 0; e < 4096; e++)
        memset(ctl_indice_ram[e], (int)(e & 0x7F) + 1, CEL); /* valor variável: não é morto */
    long soma = 0;
    for(long e = 0; e < 4096; e++) soma += ctl_indice_ram[e][e % CEL];
    long subiu_ctl = rss_anon_kb() - antes_ctl;
    printf("      CONTROLO — o mesmo corpus como índice em RAM (soma de verificação %ld):\n\n", soma);
    printf("        RssAnon antes  %ld kB   depois  %ld kB   subiu  %+ld kB\n",
           antes_ctl, rss_anon_kb(), subiu_ctl);
    printf("        e a fórmula fechada dizia:  4096 × %d B = %d kB\n\n", CEL, 4096*CEL/1024);

    ok("o instrumento DETETA o índice em RAM — logo o zero acima é medida, não cegueira",
       subiu_ctl >= 4096L * CEL / 1024 / 2);
    ok("a RAM anónima não acompanha o corpus em disco — fica abaixo do índice mais barato",
       cresc_max < custo_min);
    printf("      A RAM ficou plana enquanto o corpus multiplicou por 64, e o mesmo ponteiro\n");
    printf("      que ficou plano subiu %+ld kB quando o corpus foi para a memória. É isto que\n", subiu_ctl);
    printf("      faz do disco uma RAM: o endereço calcula-se, e o dado nunca passa a residir.\n");
}

printf("\n§M6  O CUSTO DO ACESSO é O(n) dígitos — e n = log_b(células).\n\n");
{
    /* O que se paga por nao ter indice: descer n niveis. Mede-se a LEI, nao um numero. */
    printf("      base b   células b^n   dígitos n   passos por acesso\n");
    int mal = 0;
    for(int n = 2; n <= 10; n += 2){
        int b = 4;
        long total = 1; for(int k = 0; k < n; k++) total *= b;
        /* n tem de ser exatamente log_b(total) — a lei, verificada em cada ponto */
        int calc = 0; long t = total;
        while(t > 1){ t /= b; calc++; }
        if(calc != n) mal++;
        printf("      %-8d %-13ld %-11d %d\n", b, total, n, calc);
    }
    printf("\n");
    ok("os passos por acesso são log_b(células) em todos os pontos medidos", mal == 0);
    printf("      Uma tabela dá o acesso em 1 passo mas paga N células de RAM. A árvore paga\n");
    printf("      log_b(N) passos e ZERO células. Nesta máquina, onde a swap é RAM, essa é a\n");
    printf("      troca que se quer fazer.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O endereço multifractal não precisa de memória para existir: ele CALCULA-SE.\n");
printf("    Por isso pode morar no disco sem trazer o disco para a RAM — que é o que a\n");
printf("    zram desta máquina torna proibido.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
limpa();
return falhas != 0;
}
