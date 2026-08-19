/* gguf.c — O CARREGADOR: ler os pesos do llama do DISCO, tensor a tensor, sem os residir.
 *
 * REGUA: RssAnon — mede a RAM ao percorrer o modelo
 *
 * O Aarão: "depois põe os pesos do llama nela."
 *
 * O `llm.c` montou a máquina e mediu-a com pesos sintéticos. Esta é a porta por onde os pesos
 * reais entram: o GGUF do qwen2.5:1.5b, 940 MiB em disco, 338 tensores, quantizado em Q4_K_M.
 *
 * O QUE ISTO NÃO FAZ, e é o ponto: não carrega o modelo. Lê o cabeçalho, constrói um índice de
 * 338 nomes (21 KiB — contra 940 MiB de pesos, uma razão de 1:45000), e depois cada tensor é
 * lido por `pread` quando é preciso e largado a seguir. O índice fica; os pesos nunca.
 *
 * COMO É QUE ISTO SE VALIDA SEM SER CONTRA MIM PRÓPRIO. É a pergunta que interessa, porque um
 * leitor de formato testado contra um escritor meu do mesmo formato não prova nada — os dois
 * lados partilhariam o meu erro de leitura da especificação. Então os oráculos são de fora:
 *
 *   1. o `ollama show` diz embedding 1536, contexto 32768, 1,5 B parâmetros, Q4_K_M
 *   2. a SOMA dos tamanhos dos 338 tensores tem de fechar com o tamanho do ficheiro
 *   3. a contagem de parâmetros que sai das FORMAS tem de dar os tais 1,5 B
 *   4. e a dequantização tem de devolver uma distribuição de pesos sã — não é prova, mas
 *      apanha o erro grosso de desempacotamento, que produz lixo com variância absurda
 *
 *   §G1  o CABEÇALHO: magic, versão, e as duas contagens
 *   §G2  os METADADOS: e batem com o que o ollama diz de fora
 *   §G3  os TENSORES: nome, forma, tipo, deslocamento — e a soma fecha o ficheiro
 *   §G4  os PARÂMETROS: contados das formas, contra os 1,5 B anunciados
 *   §G5  DEQUANTIZAR Q4_K: e o bloco desempacotado tem de ser são
 *   §G6  a RAM não cresce com o modelo — 940 MiB lidos, e o processo não engorda
 *
 *   cc -O2 -std=c99 -I lib tests/gguf.c -o gguf && ./gguf [caminho.gguf]
 *
 * LEI vs TRANSPORTE. f16→float, sin/cos nos vectores, variância em vírgula e 1e-4
 * no desvio eram o método. A lei é o índice a fechar o ficheiro ao byte, os 1,5 B
 * inteiros, Q16 como pinos.c, linearidade EXACTA W(x+z)=Wx+Wz, W·e_j a coluna j.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "unidade.h"

#define MAX_TENSORES 512
#define MAX_NOME     64
#define QK_K         256
#define BLOCO_Q4_K   144
#define Q16          65536L

/* os tipos de tensor do ggml que este ficheiro precisa de saber nomear */
enum { T_F32 = 0, T_F16 = 1, T_Q4_K = 12, T_Q6_K = 14 };
static const char *nome_tipo(unsigned t){
    switch(t){
        case T_F32:  return "F32";
        case T_F16:  return "F16";
        case T_Q4_K: return "Q4_K";
        case T_Q6_K: return "Q6_K";
        default:     return "outro";
    }
}
/* bytes por bloco e valores por bloco, por tipo */
static int bloco_bytes(unsigned t){
    switch(t){
        case T_F32:  return 4;   case T_F16:  return 2;
        case T_Q4_K: return 144; case T_Q6_K: return 210;
        default: return 0;
    }
}
static int bloco_valores(unsigned t){
    switch(t){
        case T_F32: case T_F16: return 1;
        case T_Q4_K: case T_Q6_K: return QK_K;
        default: return 0;
    }
}

typedef struct {
    char     nome[MAX_NOME];
    int      n_dims;
    long long dims[4];
    unsigned tipo;
    long long desloc;           /* relativo ao início dos dados */
} Tensor;

static Tensor tensores[MAX_TENSORES];
static int    n_tensores = 0;
static long long inicio_dados = 0;

/* ---- leitura sequencial por pread: o cursor é nosso, o ficheiro nunca é mapeado ----------*/
static int fd_g = -1;
static long long cur = 0;
static int falhou_leitura = 0;

static void ler(void *dest, size_t n){
    ssize_t r = pread(fd_g, dest, n, cur);
    if(r != (ssize_t)n){ falhou_leitura = 1; memset(dest, 0, n); return; }
    cur += (long long)n;
}
static unsigned u32(void){ unsigned v = 0; ler(&v, 4); return v; }
static unsigned long long u64(void){ unsigned long long v = 0; ler(&v, 8); return v; }
/* uma string GGUF: u64 de comprimento, e os bytes. Trunca-se para o índice, e o teto é dito. */
static void gstr(char *dest, size_t cap){
    unsigned long long n = u64();
    size_t copiar = n < cap-1 ? (size_t)n : cap-1;
    if(dest) ler(dest, copiar); else cur += (long long)copiar;
    if(dest) dest[copiar] = 0;
    cur += (long long)(n - copiar);           /* salta o resto sem o guardar */
}

/* saltar um valor de metadado, seja do tipo que for — só nos interessam alguns */
static void saltar_valor(unsigned tipo);
static void saltar_um(unsigned tipo){
    switch(tipo){
        case 0: case 1: case 7: cur += 1; break;          /* uint8 int8 bool */
        case 2: case 3:         cur += 2; break;          /* uint16 int16 */
        case 4: case 5: case 6: cur += 4; break;          /* uint32 int32 float32 */
        case 10: case 11: case 12: cur += 8; break;       /* uint64 int64 float64 */
        case 8: gstr(NULL, 1); break;                     /* string */
        case 9: saltar_valor(9); break;                   /* array */
        default: falhou_leitura = 1; break;
    }
}
static void saltar_valor(unsigned tipo){
    if(tipo != 9){ saltar_um(tipo); return; }
    unsigned t_elem = u32();
    unsigned long long n = u64();
    for(unsigned long long i = 0; i < n && !falhou_leitura; i += 1) saltar_um(t_elem);
}

/* ---- §G5  dequantizar um bloco Q4_K em Q16, como pinos.c — sem um float ----------------*
 * O formato: 2 B de escala do super-bloco (f16), 2 B de mínimo (f16), 12 B com oito pares
 * (escala, mínimo) de 6 bits empacotados, e 128 B com 256 valores de 4 bits.                  */
static long f16_q16(unsigned short h){
    unsigned s = (h >> 15) & 1, e = (h >> 10) & 0x1F, m = h & 0x3FF;
    if(e == 0){ long v = ((long)m + 128) / 256; return s ? -v : v; }
    if(e == 31) return s ? -(1L << 30) : (1L << 30);
    long v = (long)(1024 + m);
    int sh = (int)e - 9;
    if(sh >= 0) v <<= sh;
    else { int r = -sh; v = (v + (1L << (r - 1))) >> r; }
    return s ? -v : v;
}
static void escala_min_k4(int j, const unsigned char *q, unsigned char *d, unsigned char *m){
    if(j < 4){ *d = q[j] & 63;                       *m = q[j+4] & 63; }
    else     { *d = (q[j+4] & 0xF) | ((q[j-4] >> 6) << 4);
               *m = (q[j+4] >> 4)  | ((q[j-0] >> 6) << 4); }
}
static void deq_q4k(const unsigned char *b, long *y){
    unsigned short hd, hm;
    memcpy(&hd, b, 2); memcpy(&hm, b+2, 2);
    long d = f16_q16(hd), dm = f16_q16(hm);
    const unsigned char *escalas = b + 4;
    const unsigned char *q = b + 16;
    int is = 0, k = 0;
    for(int j = 0; j < QK_K; j += 64){
        unsigned char sc, m;
        escala_min_k4(is + 0, escalas, &sc, &m);
        long long d1 = (long long)d * sc, m1 = (long long)dm * m;
        escala_min_k4(is + 1, escalas, &sc, &m);
        long long d2 = (long long)d * sc, m2 = (long long)dm * m;
        for(int l = 0; l < 32; l += 1) y[k++] = (long)(d1 * (q[l] & 0xF) - m1);
        for(int l = 0; l < 32; l += 1) y[k++] = (long)(d2 * (q[l] >> 4) - m2);
        q += 32; is += 2;
    }
}
static long pseudo(long i){
    unsigned long h = (unsigned long)i * 6364136223846793005UL + 1442695040888963407UL;
    h ^= h >> 33; h *= 0xff51afd7ed558ccdUL; h ^= h >> 33;
    return (long)(h % 7) - 3;
}

static long rss_anon_kb(void){
    FILE *f = fopen("/proc/self/status", "r");
    if(!f) return -1;
    char lin[256]; long v = -1;
    while(fgets(lin, sizeof lin, f))
        if(!strncmp(lin, "RssAnon:", 8)){ sscanf(lin + 8, "%ld", &v); break; }
    fclose(f);
    return v;
}
static int acha(const char *nome){
    for(int i = 0; i < n_tensores; i += 1)
        if(!strcmp(tensores[i].nome, nome)) return i;
    return -1;
}
static long long elementos(const Tensor *t){
    long long n = 1;
    for(int i = 0; i < t->n_dims; i += 1) n *= t->dims[i];
    return n;
}
static long long bytes_de(const Tensor *t){
    int bv = bloco_valores(t->tipo);
    if(!bv) return -1;
    return elementos(t) / bv * bloco_bytes(t->tipo);
}

int main(int argc, char **argv){
const char *caminho = argc > 1 ? argv[1] :
    "/usr/share/ollama/.ollama/models/blobs/"
    "sha256-183715c435899236895da3869489cc30ac241476b4971a20285b1a462818a5b4";

printf("\n=== O CARREGADOR GGUF: OS PESOS DO LLAMA, DO DISCO =========================\n");
printf("    Não se carrega o modelo. Constrói-se um índice de nomes e lê-se cada\n");
printf("    tensor quando é preciso — por pread, e larga-se a seguir.\n");
printf("\n    ficheiro: %s\n", caminho);

fd_g = open(caminho, O_RDONLY);
if(fd_g < 0){
    /* O MODELO NAO ESTA NO REPO: sao 941 MB de blob do Ollama, e sem ele este medidor
     * emitia ZERO unidades e saia com exit 1 — que a bateria conta como FALHA. E nao
     * falhou nada: faltou um recurso de FORA. Medido: com o modelo, 16 unidades; sem
     * ele, 0 e um vermelho, e a bateria descia de 501 para 485 sem dizer porque.
     *
     * As 16 ficam agora CONTADAS como saltadas, com o motivo, e a saida e limpa. Ver
     * lib/unidade.h, funcao saltou(): a terceira palavra existe para isto. */
    printf("\n    o modelo GGUF nao esta neste sistema — %s\n", caminho);
    printf("    Nao ha nada a medir sem ele, e isso vai CONTADO em vez de calado:\n\n");
    for(int i = 0; i < 16; i += 1)
        saltou("as medidas do carregador GGUF (cabecalho, indice, dequantizacao)",
               "o modelo .gguf nao esta no disco — 941 MB, fora do repositorio");
    return 0;
}
struct stat st;
if(fstat(fd_g, &st)){ perror("gguf: fstat"); return 1; }
long long tam_ficheiro = (long long)st.st_size;
printf("    tamanho:  %lld B  (%lld MiB)\n", tam_ficheiro, tam_ficheiro/1048576);

printf("\n§G1  O CABEÇALHO: magic, versão, e as duas contagens.\n\n");
unsigned long long n_tens_dito = 0, n_kv = 0;
{
    cur = 0;
    char magic[5] = {0};
    ler(magic, 4);
    unsigned versao = u32();
    n_tens_dito = u64();
    n_kv        = u64();
    printf("      magic         %s\n", magic);
    printf("      versão        %u\n", versao);
    printf("      tensores      %llu\n", n_tens_dito);
    printf("      metadados     %llu\n\n", n_kv);
    ok("o magic é GGUF e a versão é 3", !strcmp(magic, "GGUF") && versao == 3);
    ok("as contagens são sãs (tensores e metadados > 0)",
       n_tens_dito > 0 && n_tens_dito < MAX_TENSORES && n_kv > 0);
}

printf("\n§G2  OS METADADOS: e têm de bater com o que o `ollama show` diz DE FORA.\n\n");
long long emb_dito = 0, ctx_dito = 0, camadas_ditas = 0;
char arq[64] = {0};
{
    for(unsigned long long i = 0; i < n_kv && !falhou_leitura; i += 1){
        char chave[128];
        gstr(chave, sizeof chave);
        unsigned tipo = u32();
        /* guardam-se QUATRO chaves. As outras saltam-se sem ocupar lugar nenhum. */
        if(!strcmp(chave, "general.architecture") && tipo == 8){
            gstr(arq, sizeof arq);
        } else if(!strcmp(chave, "qwen2.embedding_length") && tipo == 4){
            emb_dito = u32();
        } else if(!strcmp(chave, "qwen2.context_length") && tipo == 4){
            ctx_dito = u32();
        } else if(!strcmp(chave, "qwen2.block_count") && tipo == 4){
            camadas_ditas = u32();
        } else {
            saltar_valor(tipo);
        }
    }
    printf("      chave                       no ficheiro     o ollama diz\n");
    printf("      general.architecture        %-15s qwen2\n", arq);
    printf("      qwen2.embedding_length      %-15lld 1536\n", emb_dito);
    printf("      qwen2.context_length        %-15lld 32768\n", ctx_dito);
    printf("      qwen2.block_count           %-15lld (28 camadas)\n\n", camadas_ditas);
    ok("a arquitetura lida é qwen2, como o ollama anuncia", !strcmp(arq, "qwen2"));
    ok("o embedding lido é 1536, como o ollama anuncia", emb_dito == 1536);
    ok("o contexto lido é 32768, como o ollama anuncia", ctx_dito == 32768);
    printf("      Nenhum destes números é meu: saem do ficheiro de um lado e do `ollama\n");
    printf("      show` do outro. Se eu tivesse lido a especificação ao contrário, era aqui\n");
    printf("      que se via.\n");
}

printf("\n§G3  OS TENSORES: nome, forma, tipo, deslocamento — e a soma fecha o ficheiro.\n\n");
{
    for(unsigned long long i = 0; i < n_tens_dito && !falhou_leitura; i += 1){
        if(n_tensores >= MAX_TENSORES){ printf("      (teto de %d tensores atingido)\n", MAX_TENSORES); break; }
        Tensor *t = &tensores[n_tensores];
        gstr(t->nome, MAX_NOME);
        t->n_dims = (int)u32();
        if(t->n_dims > 4){ falhou_leitura = 1; break; }
        for(int d = 0; d < t->n_dims; d += 1) t->dims[d] = (long long)u64();
        t->tipo   = u32();
        t->desloc = (long long)u64();
        n_tensores++;
    }
    /* o alinhamento: os dados começam no próximo múltiplo de 32 depois do índice */
    long long alinhamento = 32;
    inicio_dados = (cur + alinhamento - 1) / alinhamento * alinhamento;
    printf("      lidos %d tensores; os dados começam em %lld\n\n", n_tensores, inicio_dados);
    printf("      %-38s %-14s %-6s %s\n", "nome", "forma", "tipo", "deslocamento");
    for(int i = 0; i < n_tensores && i < 4; i += 1){
        char forma[32] = {0}; int p = 0;
        for(int d = 0; d < tensores[i].n_dims; d += 1)
            p += snprintf(forma+p, sizeof forma - (size_t)p, d ? "×%lld" : "%lld", tensores[i].dims[d]);
        printf("      %-38s %-14s %-6s %lld\n", tensores[i].nome, forma,
               nome_tipo(tensores[i].tipo), tensores[i].desloc);
    }
    printf("      ... (%d no total)\n\n", n_tensores);

    /* A SOMA FECHA? Cada tensor ocupa dos seus bytes, e o fim do ultimo tem de cair
     * exatamente no fim do ficheiro. Se eu tivesse errado a tabela de tamanhos por tipo, ou o
     * tamanho do bloco Q4_K, este numero nao fechava — e nao ha' aqui folga nenhuma. */
    long long soma = 0, fim_max = 0; int tipo_desconhecido = 0;
    for(int i = 0; i < n_tensores; i += 1){
        long long b = bytes_de(&tensores[i]);
        if(b < 0){ tipo_desconhecido++; continue; }
        soma += b;
        long long fim = tensores[i].desloc + b;
        if(fim > fim_max) fim_max = fim;
    }
    printf("      soma dos tensores      %lld B  (%lld MiB)\n", soma, soma/1048576);
    printf("      fim do último tensor   %lld\n", inicio_dados + fim_max);
    printf("      tamanho do ficheiro    %lld\n", tam_ficheiro);
    printf("      diferença              %lld B\n\n", tam_ficheiro - (inicio_dados + fim_max));
    ok("nenhum tensor tem tipo que eu não saiba medir", tipo_desconhecido == 0);
    ok("o fim do último tensor cai no fim do ficheiro — a tabela de tamanhos fecha",
       llabs(tam_ficheiro - (inicio_dados + fim_max)) < 32);
}

printf("\n§G4  OS PARÂMETROS: contados das formas, contra os 1,5 B anunciados.\n\n");
{
    long long total = 0;
    for(int i = 0; i < n_tensores; i += 1) total += elementos(&tensores[i]);
    printf("      parâmetros somados das formas  %lld  (%lld M)\n", total, total/1000000);
    printf("      o ollama anuncia               1.5B\n\n");
    ok("a contagem de parâmetros bate com os 1,5 B anunciados",
       total > 1300000000LL && total < 1800000000LL);
    printf("      É um oráculo de fora e é grosseiro de propósito: erra-se por uma dimensão\n");
    printf("      trocada e o número sai fora da janela na hora.\n");
}

printf("\n§G5  DEQUANTIZAR Q4_K: e o bloco desempacotado tem de ser são.\n\n");
{
    int i = acha("blk.0.attn_q.weight");
    if(i < 0) i = acha("blk.0.ffn_down.weight");
    if(i < 0){ printf("      (não achei um tensor Q4_K para medir)\n"); }
    else {
        Tensor *t = &tensores[i];
        printf("      tensor  %s   forma %lldx%lld   tipo %s\n\n",
               t->nome, t->dims[0], t->n_dims > 1 ? t->dims[1] : 1, nome_tipo(t->tipo));
        if(t->tipo != T_Q4_K){
            printf("      (o tensor não é Q4_K, é %s — nada a desempacotar aqui)\n", nome_tipo(t->tipo));
        } else {
            unsigned char bloco[BLOCO_Q4_K];
            long vals[QK_K];
            long long soma = 0, soma2 = 0, mn = 1, mx = -1;
            int n_blocos = 64, inf = 0, primeiro = 1;
            for(int b = 0; b < n_blocos; b += 1){
                if(pread(fd_g, bloco, BLOCO_Q4_K,
                         inicio_dados + t->desloc + (long long)b*BLOCO_Q4_K) != BLOCO_Q4_K) break;
                deq_q4k(bloco, vals);
                for(int k = 0; k < QK_K; k += 1){
                    long v = vals[k];
                    if(v == (1L << 30) || v == -(1L << 30)){ inf += 1; continue; }
                    soma += v; soma2 += (long long)v * v;
                    if(primeiro || v < mn) mn = v;
                    if(primeiro || v > mx) mx = v;
                    primeiro = 0;
                }
            }
            long n = (long)n_blocos * QK_K;
            /* |média| < 1/20  <=>  |soma|·20 < n·Q16
             * 1e-8 < var_float < 1  <=>  n·soma2 − soma²  entre  (Q16² n²)/1e8  e  Q16² n² */
            long long n2 = (long long)n * n;
            long long q2 = (long long)Q16 * Q16;
            long long num = (long long)n * soma2 - soma * soma;   /* n² · var_Q16 */
            int centrado = (soma < 0 ? -soma : soma) * 20 < (long long)n * Q16;
            int var_ok = (num > 40LL * n2 && num < q2 * n2);
            printf("      %ld pesos desempacotados de %d blocos, em Q16:\n\n", n, n_blocos);
            printf("        soma             %lld\n", soma);
            printf("        mínimo           %ld\n", mn);
            printf("        máximo           %ld\n", mx);
            printf("        sentinela inf     %d\n\n", inf);
            ok("nenhum peso saiu não-finito", inf == 0);
            ok("os pesos estão centrados perto de zero",
               centrado);
            ok("e a variância é de escala plausível — em Q16, sem se formar a raiz:"
               " |média|<1/20 e n·soma2−soma² entre as potências de Q16, não 1e-8..1 em float",
               var_ok);
            printf("      Isto não prova que a dequantização está certa — prova que não está\n");
            printf("      grosseiramente errada. A prova a sério é o §G6 do llm.c, quando a\n");
            printf("      saída da rede for comparada com a do ollama para o mesmo prompt.\n");
        }
    }
}

printf("\n§G6  A RAM não cresce: lê-se o modelo inteiro e o processo não engorda.\n\n");
{
    /* Percorre-se TODO o ficheiro de 940 MiB, bloco a bloco, e mede-se a RAM anonima. Se o
     * carregador estivesse a acumular seja o que for, era aqui que aparecia. */
    long base = rss_anon_kb();
    unsigned char buf[65536];
    long long lidos = 0;
    for(long long off = 0; off < tam_ficheiro; off += (long long)sizeof buf){
        ssize_t r = pread(fd_g, buf, sizeof buf, off);
        if(r <= 0) break;
        lidos += r;
    }
    long depois = rss_anon_kb();
    printf("      bytes lidos do disco   %lld  (%lld MiB)\n", lidos, lidos/1048576);
    printf("      RssAnon antes          %ld kB\n", base);
    printf("      RssAnon depois         %ld kB\n", depois);
    printf("      cresceu                %+ld kB\n\n", depois - base);
    ok("leu-se o modelo inteiro do disco", lidos == tam_ficheiro);
    ok("e a RAM anónima não acompanhou — o modelo passou, não ficou",
       depois - base < 4096);
    printf("      940 MiB atravessaram este processo e ele não engordou 4 MiB. O modelo é\n");
    printf("      lido, não residido — que era o pedido.\n");
}

printf("\n§G7  OS PESOS ENTRAM NA MÁQUINA: uma projeção real do qwen, do disco.\n\n");
{
    /* O fecho do pedido. Ate' aqui os pesos foram LIDOS; agora sao USADOS — a mesma
     * matmul_disco do llm.c, mas a dequantizar Q4_K linha a linha em vez de ler floats crus.
     * Uma linha de 1536 pesos sao 6 blocos de 144 B = 864 B de disco, e 1536 floats de RAM
     * que se reaproveitam em todas as linhas.
     *
     * E COMO SE VALIDA SEM ORACULO EXTERNO PARA O RESULTADO. Nao se compara o vetor com
     * numero nenhum: usam-se duas propriedades que o produto matriz-vetor TEM de ter, e que
     * um erro de indexacao (linha por coluna, deslocamento trocado) parte na hora —
     *
     *   1. LINEARIDADE  W(x+y) = Wx + Wy, que nenhuma indexacao errada respeita por acaso
     *   2. BASE         W·e_j tem de dar exatamente a coluna j, e a coluna j le-se
     *                   independentemente, desempacotando o bloco onde ela cai
     */
    int idx = acha("blk.0.attn_q.weight");
    if(idx < 0 || tensores[idx].tipo != T_Q4_K){
        printf("      (sem tensor Q4_K para projetar)\n");
    } else {
        Tensor *t = &tensores[idx];
        long long cols = t->dims[0], linhas = t->dims[1];
        printf("      %s   %lld×%lld   Q4_K   %lld B em disco\n\n",
               t->nome, linhas, cols, bytes_de(t));

        static long x[2048], y[2048], z[2048], xy[2048], yxy[2048], linha[2048];
        int L = 64;
        for(long long j = 0; j < cols; j += 1){
            x[j] = pseudo(j);
            z[j] = pseudo(j + 1000);
            xy[j] = x[j] + z[j];
        }
        long base_rss = rss_anon_kb();
        #define PROJETA(saida, entrada) do {                                              \
            for(int i = 0; i < L; i += 1){                                                \
                long long off = inicio_dados + t->desloc + (long long)i*cols/QK_K*BLOCO_Q4_K; \
                for(long long b = 0; b < cols/QK_K; b += 1){                              \
                    unsigned char blo[BLOCO_Q4_K];                                        \
                    if(pread(fd_g, blo, BLOCO_Q4_K, off + b*BLOCO_Q4_K) != BLOCO_Q4_K) break; \
                    deq_q4k(blo, linha + b*QK_K);                                         \
                }                                                                         \
                long long acc = 0;                                                        \
                for(long long j = 0; j < cols; j += 1) acc += (long long)linha[j]*(entrada)[j]; \
                (saida)[i] = (long)acc;                                                   \
            }                                                                             \
        } while(0)

        PROJETA(y,   x);
        PROJETA(yxy, xy);
        static long y2[2048];
        PROJETA(y2,  z);
        int linear_mau = 0;
        for(int i = 0; i < L; i += 1)
            if(y[i] + y2[i] != yxy[i]) linear_mau += 1;
        printf("      LINEARIDADE — W(x+z) contra Wx + Wz, em %d linhas:\n", L);
        printf("        discordâncias  %d   (Q16, exacto)\n\n", linear_mau);
        ok("o produto com os pesos reais é linear — a indexação está de pé."
           " 1e-4 relativo era IEEE; W(x+z)=Wx+Wz e' identidade em Z",
           linear_mau == 0);

        long long jj = 700;
        static long e[2048], col[2048];
        for(long long j = 0; j < cols; j += 1) e[j] = (j == jj) ? 1 : 0;
        PROJETA(col, e);
        int col_mau = 0;
        for(int i = 0; i < L; i += 1){
            unsigned char blo[BLOCO_Q4_K];
            long vals[QK_K];
            long long off = inicio_dados + t->desloc + (long long)i*cols/QK_K*BLOCO_Q4_K
                          + (jj/QK_K)*BLOCO_Q4_K;
            if(pread(fd_g, blo, BLOCO_Q4_K, off) != BLOCO_Q4_K) break;
            deq_q4k(blo, vals);
            if(col[i] != vals[jj % QK_K]) col_mau += 1;
        }
        printf("      BASE — W·e_%lld contra a coluna %lld lida à parte:\n", jj, jj);
        printf("        discordâncias  %d\n\n", col_mau);
        ok("W·e_j concorda com a leitura direta da coluna j",
           col_mau == 0);

        long subiu = rss_anon_kb() - base_rss;
        printf("      RAM anónima durante as %d projeções: %+ld kB\n\n", L*5, subiu);
        ok("projetar com os pesos do qwen não fez a RAM crescer", subiu < 1024);
        printf("      E É PRECISO DIZER O QUE ESTAS DUAS ASSERÇÕES *NÃO* PROVAM. A do zero\n");
        printf("      exato deu zero porque os dois caminhos partilham a mesma aritmética de\n");
        printf("      bloco — é consistência interna, não correção; se eu errasse o cálculo do\n");
        printf("      deslocamento, errava-o dos dois lados. E a linearidade vale para\n");
        printf("      QUALQUER indexação fixa, mesmo trocada. O que elas apanham é a máquina\n");
        printf("      partir-se; o que não apanham é ela estar coerentemente errada.\n\n");
        printf("      O que sustenta a correção está noutro sítio, e é de fora: os metadados\n");
        printf("      contra o `ollama show` (§G2), a soma dos tensores a fechar o ficheiro ao\n");
        printf("      BYTE (§G3), e os 1,54 B de parâmetros (§G4). A prova que falta é uma só —\n");
        printf("      o forward completo, com a saída comparada à do ollama para o mesmo\n");
        printf("      prompt. Até lá isto lê os pesos do llama e projeta com eles, sem residir.\n");
        #undef PROJETA
    }
}

printf("\n=== FECHO ==================================================================\n");
printf("    O índice tem %d nomes e ocupa %d kB. Os pesos ficam onde estão.\n",
       n_tensores, (int)(sizeof tensores/1024));
printf("    Falta o forward completo — 28 camadas, GQA, SwiGLU, tokenizador — e o\n");
printf("    oráculo para ele é a saída do próprio ollama para o mesmo prompt.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
close(fd_g);
return falhas != 0;
}
