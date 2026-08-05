/* disco.h — COMO SE ESCREVE A MAQUINA SEM MEMORIA: o ficheiro E' o vector.
 *
 * A maquina esta' em code/maquina.c e no catalogo, §"A maquina sem memoria". Isto e' a
 * ferramenta que a torna possivel no resto do sistema.
 *
 * O Aarao: "promove em tudo, testa e garante 0 de memoria em todo o sistema, tudo no
 * disco." E antes: "disco e' barato, RAM e' cara."
 *
 * A razao nao e' so' o preco. Uma operacao reversivel nao apaga, logo nao paga Landauer
 * (kT ln2 por bit apagado, ~2,9e-21 J, medido em laboratorio). A DRAM paga-o EM CADA
 * REFRESH, milhares de vezes por segundo, por existir — mesmo parada. O disco escreve
 * uma vez e fica quieto. Nao e' "mais barato por byte": e' que um dissipa por existir e
 * o outro nao.
 *
 * ── O PADRAO ─────────────────────────────────────────────────────────────────────────
 *
 * Substitui-se
 *
 *     static unsigned buf[N];                  <- N*4 bytes em .bss, para sempre
 *
 * por
 *
 *     unsigned *buf = disco_u32("dados/buf.bin", N);     <- 0 bytes em .bss
 *
 * e o ponteiro vive na PILHA do main e passa-se por parametro. Nao fica global: um
 * ponteiro global sao 8 bytes em .bss, e 8 nao e' 0.
 *
 * O que isto faz e' mmap: o ficheiro no disco E' o vector, sem copia. Le-se e escreve-se
 * com o mesmo `buf[i]` de sempre — o kernel pagina o que for preciso e larga o resto. E
 * MAP_SHARED significa que o que se escreve JA' ESTA' no ficheiro: nao ha' "gravar no
 * fim", nao ha' o que perder se o processo morrer, e nao ha' arranque a carregar nada.
 *
 * ── O CUSTO, MEDIDO E NAO AFIRMADO ───────────────────────────────────────────────────
 *
 * Nesta maquina, com o ficheiro em page cache:
 *
 *     abre/fecha por acesso   5690 ns/passo      175 735 passos/s
 *     descritor aberto         902 ns/passo    1 109 078 passos/s
 *     o calculo em si          0% do tempo
 *
 * mmap nao tem nem uma coisa nem outra: o acesso e' um LOAD normal depois da primeira
 * falta de pagina. Quem disser que "o disco e' lento" que traga o numero — eu disse-o
 * sem numero nenhum e o que estava a medir eram os meus proprios fopen/fclose.
 *
 * ── AVISO ────────────────────────────────────────────────────────────────────────────
 *
 * O ficheiro nao encolhe sozinho: disco_u32 estende-o ao tamanho pedido e nunca o corta.
 * E o conteudo PERSISTE entre corridas — o que e' a vantagem, e e' tambem a armadilha se
 * o programa contava com .bss a zeros. Use disco_zera() quando quiser comecar limpo.
 */
/* ── POR FAZER: ISTO DEVIA ASSENTAR NO BANCO ────────────────────────────────────────
 *
 * O Aarao: "sao so' simbolos, poe no banco — a leitura e' simples, so' colocar no slot."
 *
 * As lajes daqui sao um SEGUNDO mecanismo de disco, e o sistema ja' tem o primeiro:
 * banco/banco.c, com 65536 slots de 32 bytes {fp:8, off:8, len:8, crc:8} e escrita
 * atomica — grava o dado, fsync, e SO' ENTAO escreve o slot; um orfao e' inofensivo.
 *
 * O banco tem duas coisas que estas lajes nao tem:
 *   - CRC por registo: um slot torto e' tratado como VAZIO, em vez de lido a' toa
 *   - a ordem de escrita pensada contra queda a meio
 *
 * O que falta para trocar: banco.c e' um PROGRAMA, e abrir/gravar/ler/fechar sao static
 * dentro dele. A ponte e' extrai-las para lib/banco.h e por DISCO_FIXO a assentar em
 * slots em vez de mmap. Mexe nos ~20 ficheiros ja' migrados e tem de passar a bateria
 * inteira — nao se faz de passagem, e partir o banco e' pior que ter 8 MB em .bss.
 *
 * Ate' la' as lajes ficam: a conta que interessa (69 153 -> 8 099 KB, -88%) nao muda com
 * o mecanismo, muda com o que saiu da RAM.
 */
#ifndef BROCA_DISCO_H
#define BROCA_DISCO_H

/* mmap, ftruncate e o resto sao POSIX, e com -std=c99 estrito a libc esconde-os.
 * A bateria compila com -std=c99, e sem esta linha os tres primeiros ficheiros
 * migrados NAO COMPILARAM la' — eu tinha-os testado com -std=gnu99 e nao dei por
 * nada. Testar com flags diferentes das do sistema nao e' testar. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

/* ftruncate e POSIX e com -std=c99 ESTRITO a libc esconde-o. Definir
 * _POSIX_C_SOURCE aqui nao chega: quando este cabecalho e lido, a libc ja foi
 * incluida pelo .c e a macro nao tem efeito retroactivo. Declara-se o prototipo,
 * que nao depende de ordem nenhuma.
 *   (a bateria compila com -std=c99; eu testei com -std=gnu99 e os tres primeiros
 *    ficheiros migrados NAO COMPILARAM la. testar com flags diferentes das do
 *    sistema nao e testar.) */
#if !defined(__USE_XOPEN2K) && !defined(_GNU_SOURCE)
extern int ftruncate(int, off_t);
#endif

/* mapeia n elementos de `tam` bytes do ficheiro `path`. O ficheiro E' o vector. */
static void *disco_mapa(const char *path, size_t n, size_t tam)
{
    size_t bytes = n * tam;
    const char *b = strrchr(path, '/');
    if (b) {                                   /* garante a pasta, sem guardar nada */
        char dir[512];
        size_t k = (size_t)(b - path);
        if (k >= sizeof dir) k = sizeof dir - 1;
        memcpy(dir, path, k); dir[k] = 0;
        mkdir(dir, 0755);
    }
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd < 0) { perror(path); exit(1); }
    struct stat st;
    if (fstat(fd, &st) != 0) { perror(path); close(fd); exit(1); }
    if ((size_t)st.st_size < bytes && ftruncate(fd, (off_t)bytes) != 0) {
        perror(path); close(fd); exit(1);
    }
    void *p = mmap(NULL, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);                                 /* o mapa sobrevive ao descritor */
    if (p == MAP_FAILED) { perror(path); exit(1); }
    return p;
}

static unsigned      *disco_u32(const char *p, size_t n){ return (unsigned*)      disco_mapa(p,n,sizeof(unsigned)); }
static unsigned char *disco_u8 (const char *p, size_t n){ return (unsigned char*) disco_mapa(p,n,1); }
static double        *disco_f64(const char *p, size_t n){ return (double*)        disco_mapa(p,n,sizeof(double)); }
static long long     *disco_i64(const char *p, size_t n){ return (long long*)     disco_mapa(p,n,sizeof(long long)); }

/* comeca limpo — o disco persiste, e nem sempre e' isso que se quer */
static void disco_zera(void *p, size_t n, size_t tam){ memset(p, 0, n*tam); }

/* devolve ao sistema; opcional, o fim do processo tambem o faz */
static void disco_larga(void *p, size_t n, size_t tam){ if(p) munmap(p, n*tam); }

/* ── ZERO ABSOLUTO: o endereco e' CONSTANTE, nao e' variavel ─────────────────────────
 *
 * `unsigned *buf = disco_u32(...)` guarda um ponteiro. Se for global sao 8 bytes em
 * .bss, e 8 nao e' 0. Passar por parametro resolve, mas obriga a mexer em todas as
 * funcoes que tocam o vector.
 *
 * Ha' uma terceira via, e e' a que respeita a doutrina a' letra: MAPEAR NUM ENDERECO
 * FIXO. O endereco passa a ser uma CONSTANTE DO PROGRAMA — nao ocupa .bss porque nao e'
 * uma variavel, e' um literal que o compilador poe na instrucao. Nada e' guardado, e o
 * sitio e' sabido. E' o mesmo movimento do A_0: nao guarda nada e mesmo assim leva la'.
 *
 *     #define CANVAS  DISCO_FIXO(unsigned, 0)          <- 0 bytes em .bss
 *     ...
 *     disco_prende(DISCO_BASE(0), "dados/canvas.bin", N, sizeof(unsigned));
 *
 * As lajes ficam a 2 TiB de distancia umas das outras, bem acima do que o heap e as
 * bibliotecas usam, e MAP_FIXED_NOREPLACE faz o kernel RECUSAR em vez de calcar o que
 * la' estiver — se um dia colidir, falha alto em vez de corromper em silencio. */
#define DISCO_LAJE       0x0000200000000000UL      /* 32 TiB: fora do heap e das libs */
#define DISCO_PASSO      0x0000004000000000UL      /* 256 GiB entre lajes             */
/* 256 GiB e nao 2 TiB: com 2 TiB a laje 63 caia em 158 TiB, ACIMA do limite de
 * utilizador do x86-64 (128 TiB), e o mmap recusava. com 256 GiB a laje 255 ainda
 * cabe. o kernel recusar foi a proteccao a funcionar — falhou alto, como devia. */
/* os parametros levam sufixo _ porque `i`, `T` e `D` sao nomes comuns e ja'
 * colidiram: um #define D 768 no ficheiro que inclui isto rebenta a macro. */
#define DISCO_BASE(i_)   ((void*)(DISCO_LAJE + (unsigned long)(i_)*DISCO_PASSO))
#define DISCO_FIXO(T_,i_)  ((T_*)DISCO_BASE(i_))
/* 2D sem tocar num unico acesso: um PONTEIRO PARA ARRAY mantem a sintaxe V[i][j].
 *     static double V[MAXV][MAXD];   ->   #define V DISCO_FIXO2(double, MAXD, k) */
#define DISCO_FIXO2(T_,cols_,i_)  ((T_(*)[cols_])DISCO_BASE(i_))

static void disco_prende(void *onde, const char *path, size_t n, size_t tam)
{
    size_t bytes = n * tam;
    const char *b = strrchr(path, '/');
    if (b) {
        char dir[512];
        size_t k = (size_t)(b - path);
        if (k >= sizeof dir) k = sizeof dir - 1;
        memcpy(dir, path, k); dir[k] = 0;
        mkdir(dir, 0755);
    }
    int fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd < 0) { perror(path); exit(1); }
    struct stat st;
    if (fstat(fd, &st) != 0) { perror(path); close(fd); exit(1); }
    if ((size_t)st.st_size < bytes && ftruncate(fd, (off_t)bytes) != 0) {
        perror(path); close(fd); exit(1);
    }
#ifdef MAP_FIXED_NOREPLACE
    void *p = mmap(onde, bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED_NOREPLACE, fd, 0);
#else
    void *p = mmap(onde, bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0);
#endif
    close(fd);
    if (p == MAP_FAILED || p != onde) {
        fprintf(stderr, "disco_prende: nao consegui %s em %p\n", path, onde);
        exit(1);
    }
}

#endif
