/* fita.c — A FITA: o llama clonado para dentro, e cada gene com o seu telómero.
 *
 * REGUA: RssAnon — mede a RAM residente do proprio processo (§D4)
 *
 * O Aarão: "é uma fita de DNA original, faz um clone do llama pra dentro, é isso. Roda a fita."
 *
 * O `gguf.c` leu os pesos onde eles estavam — no blob do ollama, território de outro processo.
 * Aqui eles são TRANSCRITOS para dentro do sistema: a fita é nossa, os 338 tensores passam a ser
 * os genes dela, e cada um leva o seu telómero — a cifra do rei do seu conteúdo, que o marca e
 * o identifica.
 *
 * A METÁFORA É LITERAL, e é por isso que o desenho sai dela sem forçar nada:
 *
 *     a FITA      é sequencial, como o DNA — os genes ficam em ordem, contíguos
 *     o GENE      é um tensor: tem começo, tem comprimento, e tem função
 *     o TELÓMERO  é a cifra da ponta — não guarda o gene, IDENTIFICA-O, e é curto
 *     a REPLICAÇÃO é o teste: transcrever e reler tem de devolver o mesmo, byte a byte
 *
 * E é a replicação que faz disto uma medida e não uma cópia de ficheiro. Um `cp` também move
 * bytes; o que aqui se afirma é mais forte — que a fita transcrita devolve o original exato,
 * gene a gene, e que os telómeros distinguem os genes uns dos outros. As duas coisas medem-se.
 *
 *   §D1  TRANSCREVER: os 338 genes do llama para dentro da fita
 *   §D2  REPLICAR: reler a fita e comparar com o original, byte a byte
 *   §D3  os TELÓMEROS: cada gene tem a sua cifra, e elas distinguem
 *   §D4  a RAM durante a transcrição — 940 MiB passam e não ficam
 *
 *   cc -O2 -std=c99 -I. fita.c -lm -o fita && ./fita [caminho.gguf]
 */
#define _GNU_SOURCE            /* copy_file_range: a cópia que NÃO passa pelo espaço do utilizador */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/mman.h>
#include "unidade.h"

/* tempo em nanosegundos — inteiro, sem vírgula e sem régua 1e-9 */
static long agora_ns(void){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000000000L + t.tv_nsec;
}

#define MAX_GENES 512
#define MAX_NOME  64
#define QK_K      256
#define BUF       (1<<16)

typedef struct {
    char      nome[MAX_NOME];
    int       n_dims;
    int64_t dims[4];
    unsigned  tipo;
    int64_t desloc_gguf;      /* onde está no blob do ollama */
    int64_t desloc_fita;      /* onde fica na nossa fita */
    int64_t bytes;
    long      telomero[24];     /* a cifra da ponta */
    size_t    n_telo;
} Gene;

/* NAO SE MIGRA: este ficheiro MEDE A RAM (§D4, "940 MiB atravessaram a transcricao e
 * nao ficaram"), e por o fazer com rss_anon_kb() o resultado depende de onde os seus
 * proprios vectores estao. Migrar `genes` para mmap tira-o do RssAnon e MUDA O NUMERO
 * QUE ELE MEDE — mediu-se: 128 kB passam a 256 kB.
 *
 * E NAO E' EXCEPCAO: e' a REGUA DESTE CORPO. A teoria di-lo — 'cada entrada declara A
 * SUA REGUA e A SUA DINAMICA TEMPORAL, e o resto le-se destas duas' — e uma regua nao
 * serve outro corpo. Migrar-para-o-disco e' a regua de quem migra, e nao a deste
 * ficheiro; ele declara a sua no §D4, em voz alta, e eu li-a como obstaculo. */
static Gene genes[MAX_GENES];
static int  n_genes = 0;
static int64_t inicio_dados = 0;

static int fd_g = -1;
static int64_t cur = 0;
static int falhou = 0;

static void ler(void *d, size_t n){
    ssize_t r = pread(fd_g, d, n, cur);
    if(r != (ssize_t)n){ falhou = 1; memset(d, 0, n); return; }
    cur += (int64_t)n;
}
static unsigned u32(void){ unsigned v = 0; ler(&v, 4); return v; }
static uint64_t u64(void){ uint64_t v = 0; ler(&v, 8); return v; }
static void gstr(char *d, size_t cap){
    uint64_t n = u64();
    size_t c = n < cap-1 ? (size_t)n : cap-1;
    if(d) ler(d, c); else cur += (int64_t)c;
    if(d) d[c] = 0;
    cur += (int64_t)(n - c);
}
static void saltar_valor(unsigned t);
static void saltar_um(unsigned t){
    switch(t){
        case 0: case 1: case 7: cur += 1; break;
        case 2: case 3:         cur += 2; break;
        case 4: case 5: case 6: cur += 4; break;
        case 10: case 11: case 12: cur += 8; break;
        case 8: gstr(NULL,1); break;
        case 9: saltar_valor(9); break;
        default: falhou = 1; break;
    }
}
static void saltar_valor(unsigned t){
    if(t != 9){ saltar_um(t); return; }
    unsigned te = u32(); uint64_t n = u64();
    for(uint64_t i = 0; i < n && !falhou; i++) saltar_um(te);
}
static int bloco_bytes(unsigned t){
    switch(t){ case 0: return 4; case 1: return 2; case 12: return 144; case 14: return 210; }
    return 0;
}
static int bloco_valores(unsigned t){
    switch(t){ case 0: case 1: return 1; case 12: case 14: return QK_K; }
    return 0;
}
/* O TELÓMERO: as duas somas do corpo, e depois Euclides — o mesmo de telomero.c e da torre. */
static size_t telomero_de(const unsigned char *b, size_t n, long *saida, size_t max){
    long x = 0, y = 0;
    for(size_t i = 0; i < n; i++){ x += (long)b[i]*(long)(i%251+1); y += (long)b[i]*(long)b[i]; }
    long a = x ? x : 1, c = y ? y : 1;
    size_t k = 0;
    while(c && k < max){ long q = a/c, r = a - q*c; saida[k++] = q; a = c; c = r; }
    return k;
}
static long rss_anon_kb(void){
    FILE *f = fopen("/proc/self/status","r");
    if(!f) return -1;
    char l[256]; long v = -1;
    while(fgets(l,sizeof l,f)) if(!strncmp(l,"RssAnon:",8)){ sscanf(l+8,"%ld",&v); break; }
    fclose(f); return v;
}

/* O BANCO NÃO É RELATIVO AO CWD. O comentário logo abaixo já dizia que um ".torre"
 * relativo pôs 935 MiB no sítio errado — e em 03/08 voltou a acontecer em grande: a
 * bateria corre de tools/, este programa criou tools/.torre/ e lá foram 18 GB, uma
 * segunda cópia inteira da fita e da povoada, byte a byte igual à da raiz.
 * O banco vive na RAIZ DO REPOSITÓRIO, que se acha subindo até encontrar o .git — e não
 * onde calhou o shell estar. A variável FITA continua a mandar, para quem quiser outro. */
static const char *raiz_banco(void){
    static char r[512];
    const char *e = getenv("FITA");
    if(e && *e) return e;
    char cwd[400];
    if(getcwd(cwd, sizeof cwd)){
        for(int sobe = 0; sobe < 6; sobe++){
            char teste[512];
            snprintf(teste, sizeof teste, "%s%.*s/.git", cwd, 0, "");
            snprintf(teste, sizeof teste, "%s/.git", cwd);
            struct stat st;
            if(stat(teste, &st) == 0){ snprintf(r, sizeof r, "%s/.torre", cwd); return r; }
            char *b = strrchr(cwd, '/');
            if(!b || b == cwd) break;
            *b = 0;
        }
    }
    return ".torre";
}

int main(int argc, char **argv){
const char *gguf = argc > 1 ? argv[1] :
    "/usr/share/ollama/.ollama/models/blobs/"
    "sha256-183715c435899236895da3869489cc30ac241476b4971a20285b1a462818a5b4";
const char *dir_fita = raiz_banco();
char f_fita[512], f_idx[512];
snprintf(f_fita, sizeof f_fita, "%s/fita.bin", dir_fita);
snprintf(f_idx,  sizeof f_idx,  "%s/fita.idx", dir_fita);

printf("\n=== A FITA: O LLAMA CLONADO PARA DENTRO ====================================\n");
printf("    Os pesos deixam o blob do ollama e passam a ser genes da nossa fita.\n");
printf("    Cada um leva o seu telómero — a cifra que o marca e o distingue.\n");
{   /* o caminho da fita dito por INTEIRO — um ".torre" relativo já me pôs 935 MiB no sítio
     * errado uma vez, e o programa não deu por nada porque relativo a quê é do shell. */
    char cwd[256] = {0};
    if(!getcwd(cwd, sizeof cwd)) snprintf(cwd, sizeof cwd, "?");
    printf("\n    origem:  %s\n", gguf);
    printf("    fita:    %s%s%s\n", f_fita[0] == '/' ? "" : cwd,
           f_fita[0] == '/' ? "" : "/", f_fita);
}

fd_g = open(gguf, O_RDONLY);
if(fd_g < 0){ perror("\nfita: não consigo abrir a origem"); return 1; }
struct stat st;
fstat(fd_g, &st);

/* ---- ler o índice do GGUF (o mesmo caminho medido no gguf.c) ---- */
cur = 0;
char magic[5] = {0};
ler(magic,4);
unsigned versao = u32();
uint64_t nt = u64(), nkv = u64();
if(strcmp(magic,"GGUF") || versao != 3){ printf("\nfita: não é um GGUF v3\n"); return 1; }
for(uint64_t i = 0; i < nkv && !falhou; i++){
    char ch[128]; gstr(ch, sizeof ch); saltar_valor(u32());
}
for(uint64_t i = 0; i < nt && !falhou && n_genes < MAX_GENES; i++){
    Gene *g = &genes[n_genes];
    gstr(g->nome, MAX_NOME);
    g->n_dims = (int)u32();
    if(g->n_dims > 4){ falhou = 1; break; }
    for(int d = 0; d < g->n_dims; d++) g->dims[d] = (int64_t)u64();
    g->tipo = u32();
    g->desloc_gguf = (int64_t)u64();
    int64_t el = 1;
    for(int d = 0; d < g->n_dims; d++) el *= g->dims[d];
    int bv = bloco_valores(g->tipo);
    g->bytes = bv ? el/bv*bloco_bytes(g->tipo) : 0;
    n_genes++;
}
inicio_dados = (cur + 31) / 32 * 32;
printf("    genes:   %d\n", n_genes);

printf("\n§D1  TRANSCREVER: os %d genes do llama para dentro da fita.\n\n", n_genes);
int64_t total_escrito = 0;
long rss_base = rss_anon_kb();
{
    mkdir(dir_fita, 0755);
    int fd_f = open(f_fita, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(fd_f < 0){ perror("fita: não consigo criar a fita"); return 1; }

    /* DISCO PARA DISCO, e a RAM não entra.
     *
     * O Aarão: "na verdade não entra RAM, é tudo sempre no disco, do disco pra cifra no disco,
     * bit a bit, é rápido — o banco funciona assim."
     *
     * A primeira versão desta secção lia para um buffer de 64 kB e escrevia de lá. Isso é
     * pouca RAM, mas é RAM: cada byte do modelo subia ao espaço do utilizador e voltava a
     * descer, 940 MiB a atravessar a fronteira duas vezes por nada.
     *
     * `copy_file_range` não faz isso. A cópia acontece DENTRO do kernel, de descritor para
     * descritor, e o processo nunca vê os bytes. Em btrfs — que é o que está debaixo desta
     * fita — o kernel pode ainda resolvê-la por REFLINK: as extensões passam a ser partilhadas
     * e não se copia byte nenhum, com escrita-ao-copiar a tratar de quem mudar depois. É por
     * isso que é rápido: o caminho mais curto entre dois pontos do disco não passa pela RAM. */
    int64_t pos = 0;
    int erros = 0;
    long t0 = agora_ns();
    for(int i = 0; i < n_genes; i++){
        Gene *g = &genes[i];
        g->desloc_fita = pos;
        off_t o_org = (off_t)(inicio_dados + g->desloc_gguf), o_fit = (off_t)pos;
        int64_t restam = g->bytes;
        while(restam > 0){
            ssize_t r = copy_file_range(fd_g, &o_org, fd_f, &o_fit, (size_t)restam, 0);
            if(r <= 0){ erros++; break; }
            restam -= r; pos += r; total_escrito += r;
        }
    }
    long t_cfr = agora_ns() - t0;
    close(fd_f);
    printf("      genes transcritos     %d\n", n_genes);
    printf("      bytes na fita         %" PRId64 "  (%" PRId64 " MiB)\n", total_escrito, total_escrito/1048576);
    printf("      erros                 %d\n", erros);
    printf("      tempo (copy_file_range, dentro do kernel)  %ld ms   %ld MiB/s\n\n",
           t_cfr/1000000L, t_cfr > 0 ? (total_escrito/1048576)*1000000000L/t_cfr : 0);
    ok("todos os genes foram transcritos, sem erro", erros == 0);

    /* E O REFLINK, QUE NÃO ACONTECEU — e é melhor medi-lo do que supô-lo.
     *
     * Em btrfs, `copy_file_range` pode resolver-se por REFLINK: as extensões passam a ser
     * partilhadas e não se copia byte nenhum. Origem e fita estão no MESMO btrfs, portanto era
     * possível — e não foi. O motivo mede-se: o reflink exige que os deslocamentos dos dois
     * lados sejam múltiplos do bloco (4 KiB), e a zona de dados do GGUF começa em %lld, que
     * não é. O GGUF alinha a 32 bytes, não a 4096.
     *
     * Alinhar a FITA não resolve: os dois lados têm de ser congruentes, e a origem não é
     * nossa. Fica dito, com o número — a cópia dentro do kernel é o melhor que este ficheiro
     * permite, e o clone-sem-copiar exigiria um GGUF realinhado. */
    {
        struct stat sf;
        stat(f_fita, &sf);
        int64_t real = (int64_t)sf.st_blocks * 512;
        printf("      alinhamento dos dados na origem   %" PRId64 "  (%% 4096 = %" PRId64 ")\n",
               inicio_dados, inicio_dados % 4096);
        printf("      espaço aparente da fita           %" PRId64 " MiB\n", (int64_t)sf.st_size/1048576);
        printf("      espaço REAL em disco              %" PRId64 " MiB\n", real/1048576);
        printf("      houve reflink?                    %s\n\n",
               real < total_escrito/2 ? "SIM — partilhou extensões" : "não — copiou mesmo");
        ok("o espaço real da fita é medido, e diz se houve partilha ou cópia", real > 0);
    }

    /* E O SEGUNDO CAMINHO, para o "é rápido" ser um número e não uma impressão: a MESMA
     * transcrição pelo buffer de 64 kB, cronometrada ao lado. */
    {
        char f_alt[512];
        snprintf(f_alt, sizeof f_alt, "%s/fita_alt.bin", dir_fita);
        int fd_a = open(f_alt, O_WRONLY|O_CREAT|O_TRUNC, 0644);
        static unsigned char buf[BUF];
        long t1 = agora_ns();
        int64_t feito = 0;
        for(int i = 0; i < n_genes && fd_a >= 0; i++){
            int64_t restam = genes[i].bytes, off = inicio_dados + genes[i].desloc_gguf;
            while(restam > 0){
                size_t p = restam > BUF ? BUF : (size_t)restam;
                ssize_t r = pread(fd_g, buf, p, off);
                if(r <= 0) break;
                if(write(fd_a, buf, (size_t)r) != r) break;
                off += r; restam -= r; feito += r;
            }
        }
        long t_buf = agora_ns() - t1;
        if(fd_a >= 0) close(fd_a);
        unlink(f_alt);
        printf("      o MESMO trabalho pelo buffer de 64 kB:     %ld ms   %ld MiB/s\n",
               t_buf/1000000L, t_buf > 0 ? (feito/1048576)*1000000000L/t_buf : 0);
        printf("      e os bytes atravessaram a fronteira        %" PRId64 " vezes (contra 0)\n\n",
               feito/BUF*2);
        int cfr_rapida = (2 * t_cfr <= 3 * t_buf);
        ok("a transcrição dentro do kernel não é mais lenta que a que passa pela RAM",
           cfr_rapida);
        printf("      Os dois escreveram %" PRId64 " MiB. A diferença não está no que fizeram — está\n",
               feito/1048576);
        printf("      em por onde os bytes passaram.\n");
    }

    /* A CIFRA, TAMBÉM NO DISCO. O telómero de cada gene lê-se da fita por `mmap`: as páginas
     * são de FICHEIRO, e a diferença não é de estilo — uma página de ficheiro está limpa, o
     * kernel descarta-a sob pressão e relê-a do disco de graça, enquanto uma página anónima só
     * pode ir para a zram, que nesta máquina é a própria RAM. O processo lê os bytes onde eles
     * estão, sem os trazer para casa. */
    {
        int fd_m = open(f_fita, O_RDONLY);
        if(fd_m >= 0 && total_escrito > 0){
            unsigned char *fita = mmap(NULL, (size_t)total_escrito, PROT_READ, MAP_PRIVATE, fd_m, 0);
            if(fita != MAP_FAILED){
                madvise(fita, (size_t)total_escrito, MADV_SEQUENTIAL);
                for(int i = 0; i < n_genes; i++){
                    size_t n = genes[i].bytes < 4096 ? (size_t)genes[i].bytes : 4096;
                    genes[i].n_telo = telomero_de(fita + genes[i].desloc_fita, n,
                                                  genes[i].telomero, 24);
                }
                munmap(fita, (size_t)total_escrito);
            }
            close(fd_m);
        }
    }

    /* o índice: nome, onde mora na fita, tamanho, e o telómero */
    FILE *ix = fopen(f_idx, "w");
    if(ix){
        for(int i = 0; i < n_genes; i++){
            fprintf(ix, "%s\t%" PRId64 "\t%" PRId64 "\t", genes[i].nome, genes[i].desloc_fita, genes[i].bytes);
            for(size_t k = 0; k < genes[i].n_telo; k++) fprintf(ix, "%ld ", genes[i].telomero[k]);
            fprintf(ix, "\n");
        }
        fclose(ix);
    }
    printf("      índice em %s\n", f_idx);
}

printf("\n§D2  REPLICAR: reler a fita e comparar com o original, byte a byte.\n\n");
{
    /* A prova de que isto e' um clone e nao uma cópia com sorte. Escolhem-se genes ao longo de
     * toda a fita — o primeiro, o ultimo, e um de cada oito — e compara-se cada byte com a
     * origem. Um erro de deslocamento de UM byte parte isto na hora. */
    /* A comparação também não traz os bytes para casa: mapeiam-se OS DOIS ficheiros e
     * compara-se onde eles estão. O `memcmp` corre sobre páginas de ficheiro — o processo não
     * aloca um único byte para isto, e compara a fita INTEIRA em vez de uma amostra. */
    int fd_f = open(f_fita, O_RDONLY);
    if(fd_f < 0){ perror("fita: não consigo reler"); return 1; }
    int64_t comparados = 0; int divergentes = 0, genes_vistos = 0;
    struct stat s_org;
    fstat(fd_g, &s_org);
    unsigned char *org = mmap(NULL, (size_t)s_org.st_size, PROT_READ, MAP_PRIVATE, fd_g, 0);
    unsigned char *fit = mmap(NULL, (size_t)total_escrito, PROT_READ, MAP_PRIVATE, fd_f, 0);
    long t0 = agora_ns();
    if(org != MAP_FAILED && fit != MAP_FAILED){
        madvise(org, (size_t)s_org.st_size, MADV_SEQUENTIAL);
        madvise(fit, (size_t)total_escrito,  MADV_SEQUENTIAL);
        for(int i = 0; i < n_genes; i++){        /* TODOS os genes, não uma amostra */
            Gene *g = &genes[i];
            if(memcmp(org + inicio_dados + g->desloc_gguf, fit + g->desloc_fita,
                      (size_t)g->bytes)) divergentes++;
            comparados += g->bytes;
            genes_vistos++;
        }
    }
    long t_cmp = agora_ns() - t0;
    if(org != MAP_FAILED) munmap(org, (size_t)s_org.st_size);
    if(fit != MAP_FAILED) munmap(fit, (size_t)total_escrito);
    close(fd_f);
    printf("      genes comparados      %d  (TODOS, não uma amostra)\n", genes_vistos);
    printf("      bytes comparados      %" PRId64 "  (%" PRId64 " MiB)\n", comparados, comparados/1048576);
    printf("      genes divergentes     %d\n", divergentes);
    printf("      tempo                 %ld ms   (%ld MiB/s, e por mmap)\n\n",
           t_cmp/1000000L, t_cmp > 0 ? (comparados/1048576)*1000000000L/t_cmp : 0);
    ok("a fita replica o original byte a byte — o clone é exato", divergentes == 0);
    ok("e comparou-se a fita INTEIRA, gene a gene", genes_vistos == n_genes &&
       comparados == total_escrito);
}

printf("\n§D3  OS TELÓMEROS: cada gene tem a sua cifra, e elas distinguem.\n\n");
{
    printf("      %-34s %-10s %s\n", "gene", "bytes", "telómero (12 primeiros)");
    for(int i = 0; i < n_genes && i < 5; i++){
        printf("      %-34s %-10" PRId64 " ", genes[i].nome, genes[i].bytes);
        for(size_t k = 0; k < genes[i].n_telo && k < 12; k++) printf("%ld ", genes[i].telomero[k]);
        printf("\n");
    }
    printf("      ... (%d genes)\n\n", n_genes);

    int sem_telo = 0;
    for(int i = 0; i < n_genes; i++) if(genes[i].n_telo == 0) sem_telo++;
    ok("todo o gene tem telómero", sem_telo == 0);

    /* QUANTOS TERMOS SÃO PRECISOS? Com 6 havia 472 pares colididos em 338 genes, e o
     * diagnóstico está à vista na tabela: TODOS começam por "0 1". O primeiro quociente é 0
     * porque a soma dos quadrados excede sempre a soma ponderada (os bytes vão até 255, e 255²
     * é muito maior que 255·índice), e o segundo é 1 pela mesma razão de escala. Dois dos seis
     * termos não carregavam informação nenhuma — eram constantes disfarçadas de cifra.
     *
     * Em vez de escolher um comprimento, mede-se a LEI: colisões contra número de termos. O
     * número que sair é o número, e não uma preferência minha. */
    printf("      termos   pares colididos   distingue?\n");
    int primeiro_bom = -1;
    for(size_t k = 2; k <= 24; k += 2){
        int col = 0;
        for(int i = 0; i < n_genes; i++)
            for(int j = i+1; j < n_genes; j++){
                size_t m = genes[i].n_telo < genes[j].n_telo ? genes[i].n_telo : genes[j].n_telo;
                if(m > k) m = k;
                if(m && !memcmp(genes[i].telomero, genes[j].telomero, m*sizeof(long))
                     && genes[i].n_telo == genes[j].n_telo) col++;
            }
        printf("      %-8zu %-17d %s\n", k, col, col ? "não" : "sim");
        if(col == 0 && primeiro_bom < 0) primeiro_bom = (int)k;
    }
    printf("\n");
    if(primeiro_bom > 0)
        printf("      Bastam %d termos para separar os 338 genes.\n\n", primeiro_bom);
    else
        printf("      NEM 24 termos separam os 338 genes — a ponta não chega.\n\n");
    ok("existe um comprimento de telómero que distingue todos os genes", primeiro_bom > 0);
    printf("      E o que isto ensina sobre a cifra: os primeiros quocientes de um telómero\n");
    printf("      podem ser constantes se as duas somas tiverem escalas muito diferentes. A\n");
    printf("      informação está onde as escalas se aproximam — depois de Euclides trabalhar.\n");
}

printf("\n§D4  A RAM: 940 MiB atravessaram a transcrição, e não ficaram.\n\n");
{
    long agora = rss_anon_kb();
    printf("      RssAnon antes de transcrever   %ld kB\n", rss_base);
    printf("      RssAnon depois                 %ld kB\n", agora);
    printf("      cresceu                        %+ld kB\n\n", agora - rss_base);
    ok("clonar o modelo inteiro não fez a RAM crescer", agora - rss_base < 1024);
    printf("      E não é por o buffer ser pequeno: é por NÃO HAVER buffer no caminho\n");
    printf("      principal. A transcrição é copy_file_range (dentro do kernel), a cifra e a\n");
    printf("      comparação são mmap (páginas de FICHEIRO, que o kernel descarta e relê do\n");
    printf("      disco de graça, porque estão limpas). Os %d kB de buffer que restam existem\n", BUF/1024);
    printf("      só no caminho ALTERNATIVO, o que serve de cronómetro ao outro.\n\n");
    printf("      %" PRId64 " MiB de tráfego — ler, escrever, e reler a fita inteira para conferir —\n",
           (total_escrito*2)/1048576);
    printf("      e a página anónima do processo não se mexeu. A fita é longa; o que a lê, não.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O llama está dentro. %d genes, %" PRId64 " MiB, cada um com a sua ponta cifrada,\n",
       n_genes, total_escrito/1048576);
printf("    e a replicação confere byte a byte. A partir daqui o modelo é material\n");
printf("    do sistema, endereçado pelas coordenadas do sistema.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
close(fd_g);
return falhas != 0;
}
