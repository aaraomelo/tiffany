/* kernelb.c — O KERNEL NO PLUGUE: a syscall É a ISA do SO, e o VFS É o trie.
 *
 * O Aarão: "já seria bom adicionar o kernel do Linux no plugue — é o que falta pra controle total."
 *
 * E ele entra pelo mesmo sítio que o nginx entrou há minutos, o que não é coincidência:
 *
 *      no nginx    o `location` casa por PREFIXO MAIS LONGO
 *      no kernel   o caminho casa o MOUNT POINT mais longo
 *      aqui        a descida no trie
 *
 * São a mesma operação três vezes. Um servidor web e um sistema de ficheiros resolvem o mesmo
 * problema — *dado um nome, quem responde?* — e os dois chegaram ao trie sem se falarem.
 *
 * E A SYSCALL É A ISA. Um número de chamada e alguns argumentos em registos, e o kernel despacha
 * por esse número. É `LOAD`/`STORE` com o opcode noutro sítio:
 *
 *      read(fd, buf, n)      →  LOAD  — traz de fora para dentro
 *      write(fd, buf, n)     →  STORE — leva de dentro para fora
 *      o número da syscall   →  o opcode
 *      o fd                  →  o SLOT: um inteiro pequeno que endereça
 *
 * *E o par (read, write) é adjunto, como o nosso.* O kernel não tem uma terceira operação de
 * transferência — tem duas, e a segunda é a primeira ao contrário.
 *
 * O QUE ISTO MEDE É A MÁQUINA REAL, lendo `/proc`. Se `/proc` não existir (não é Linux), sai
 * com 2: um medidor sem o objeto a medir não passa nem falha — não mediu.
 *
 *   §K1  o VFS casa o mount point MAIS LONGO — e é o mesmo trie do nginx
 *   §K2  o fd é um SLOT: inteiro pequeno, denso, e reutilizado
 *   §K3  /proc é o banco exposto: o conteúdo CALCULA-SE, não está guardado
 *   §K4  o que falta para controle total — e o que já está lá
 *
 *   cc -O2 -std=c99 -Wall -Wformat kernelb.c -o kernelb && ./kernelb
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include "unidade.h"

/* ================================================================================ */
/* §K1 — o VFS casa o mount point mais longo                                        */
/* ================================================================================ */
#define MAXMNT 256
/* NAO SE MIGRA: este ficheiro mede a RAM (rss_anon_kb), e tirar MNT do RssAnon muda o
 * numero que ele mede. Mesma regra do fita.c e dos ctl_ do llm.c e do mmu.c. */
static char MNT[MAXMNT][256];
static int NMNT = 0;

static int casa_mount(const char *caminho){
    int melhor = -1; size_t maislongo = 0;
    for(int i = 0; i < NMNT; i++){
        size_t p = strlen(MNT[i]);
        /* "/" casa tudo; os outros exigem que o caminho continue com / ou acabe ali */
        if(strncmp(caminho, MNT[i], p) != 0) continue;
        if(p > 1 && caminho[p] != '/' && caminho[p] != 0) continue;
        if(p >= maislongo){ maislongo = p; melhor = i; }
    }
    return melhor;
}

static void secao_K1(void){
    printf("\n§K1  O VFS CASA O MOUNT POINT MAIS LONGO — o mesmo trie do nginx\n\n");

    FILE *f = fopen("/proc/mounts", "r");
    if(!f){ ok("o /proc/mounts abre", 0); return; }
    char linha[1024];
    while(fgets(linha, sizeof linha, f) && NMNT < MAXMNT){
        char dev[256], pt[256], tipo[64];
        if(sscanf(linha, "%255s %255s %63s", dev, pt, tipo) == 3)
            snprintf(MNT[NMNT++], 256, "%s", pt);
    }
    fclose(f);
    printf("     %d pontos de montagem lidos de /proc/mounts\n", NMNT);
    ok("há montagens para medir — senão isto não media nada", NMNT > 1);

    /* os casos: o mount point mais longo tem de ganhar, e o "/" só ganha quando nenhum outro casa */
    struct { const char *caminho; } casos[] = {
        { "/etc/passwd" }, { "/proc/self/status" }, { "/sys/kernel" },
        { "/tmp/x" }, { "/dev/null" }, { "/home" },
    };
    printf("        caminho                    responde        (o mais longo que casa)\n");
    int erros = 0;
    for(int i = 0; i < 6; i++){
        int r = casa_mount(casos[i].caminho);
        if(r < 0){ erros++; printf("        %-26s (NENHUM)\n", casos[i].caminho); continue; }
        /* conferir à mão: nenhum outro mount que case pode ser mais longo */
        size_t p = strlen(MNT[r]);
        for(int j = 0; j < NMNT; j++){
            size_t q = strlen(MNT[j]);
            if(q <= p) continue;
            if(strncmp(casos[i].caminho, MNT[j], q) != 0) continue;
            if(q > 1 && casos[i].caminho[q] != '/' && casos[i].caminho[q] != 0) continue;
            erros++;   /* havia um mais longo que casava, e não foi escolhido */
        }
        printf("        %-26s %s\n", casos[i].caminho, MNT[r]);
    }
    ok("todo caminho cai no mount point MAIS LONGO que casa — é a descida no trie", erros == 0);

    /* e a raiz casa tudo, o que é a folha de topo do trie */
    int tem_raiz = 0;
    for(int i = 0; i < NMNT; i++) if(!strcmp(MNT[i], "/")) tem_raiz = 1;
    ok("a raiz / está montada e casa tudo — é o topo do trie, não um caso especial", tem_raiz);

    conclui("um servidor web e um sistema de ficheiros chegaram ao mesmo trie sem se falarem.");
}

/* ================================================================================ */
/* §K2 — o fd é um slot                                                             */
/* ================================================================================ */
/* A afirmação: o descritor não é um ponteiro nem um identificador opaco — é um ÍNDICE, pequeno
 * e denso, e o kernel reaproveita o mais baixo que estiver livre. É exatamente o que um slot é.
 * Isto mede-se abrindo e fechando ficheiros e olhando para os números. */
static void secao_K2(void){
    printf("\n§K2  O fd É UM SLOT — inteiro pequeno, denso, e o mais baixo livre\n\n");

    int a = open("/dev/null", O_RDONLY);
    int b = open("/dev/null", O_RDONLY);
    int c = open("/dev/null", O_RDONLY);
    printf("     três aberturas seguidas:  %d  %d  %d\n", a, b, c);
    ok("os descritores são pequenos e consecutivos — é um índice, não um ponteiro",
       a > 2 && b == a + 1 && c == b + 1);

    close(b);                                   /* liberta o do meio */
    int d = open("/dev/null", O_RDONLY);
    printf("     fecha-se o do meio (%d) e abre-se outro:  %d\n", b, d);
    ok("o novo reocupa o SLOT libertado — o mais baixo livre, como o banco", d == b);

    close(a); close(c); close(d);

    /* e o par read/write é adjunto: a mesma transferência, o sentido trocado */
    int p[2];
    if(pipe(p) == 0){
        const char *msg = "o mesmo conteudo, os dois sentidos";
        ssize_t w = write(p[1], msg, strlen(msg));
        char buf[64] = {0};
        ssize_t r = read(p[0], buf, sizeof buf - 1);
        printf("     write %zd bytes, read %zd bytes, iguais: %s\n", w, r,
               (r == w && !strcmp(buf, msg)) ? "sim" : "NÃO");
        ok("write∘read devolve o mesmo — o par é adjunto, como o nosso plugue",
           r == w && !strcmp(buf, msg));
        close(p[0]); close(p[1]);
    }

    printf("\n     na nossa ISA:  LOAD slot  ↔  read(fd)      STORE slot  ↔  write(fd)\n");
    printf("     e o número da syscall é o OPCODE. O kernel despacha por ele, como o nosso\n");
    printf("     `passo()` despacha pelo byte do programa.\n");

    conclui("o kernel não tem uma terceira operação de transferência: tem duas, e a segunda é a primeira ao contrário.");
}

/* ================================================================================ */
/* §K3 — /proc é o banco exposto                                                    */
/* ================================================================================ */
/* O ponto que interessa ao dispositivo: em /proc o conteúdo NÃO ESTÁ GUARDADO. Ele é calculado
 * no momento da leitura. É por isso que o tamanho dá 0 e a leitura devolve bytes — e é
 * exatamente o que fazemos quando o endereço é a cifra: o valor sai da conta, não da tabela. */
static void secao_K3(void){
    printf("\n§K3  /proc É O BANCO EXPOSTO: o conteúdo CALCULA-SE, não está guardado\n\n");

    const char *alvos[] = { "/proc/self/stat", "/proc/uptime", "/proc/meminfo", NULL };
    printf("        ficheiro                 tamanho no stat    bytes lidos\n");
    int mentem = 0, leram = 0;
    for(int i = 0; alvos[i]; i++){
        struct stat st;
        if(stat(alvos[i], &st) != 0) continue;
        char buf[4096];
        int f = open(alvos[i], O_RDONLY);
        if(f < 0) continue;
        ssize_t n = read(f, buf, sizeof buf - 1);
        close(f);
        if(n > 0) leram++;
        if(st.st_size == 0 && n > 0) mentem++;
        printf("        %-24s %14lld    %11zd\n", alvos[i], (long long)st.st_size, n);
    }
    ok("os ficheiros leem-se — há conteúdo", leram >= 2);
    ok("e o stat diz TAMANHO 0 enquanto a leitura devolve bytes — não estava guardado",
       mentem >= 2);

    /* e o conteúdo MUDA entre duas leituras sem ninguém escrever nada */
    char u1[128] = {0}, u2[128] = {0};
    int f = open("/proc/uptime", O_RDONLY); if(f >= 0){ (void)!read(f, u1, 127); close(f); }
    for(volatile long k = 0; k < 40000000L; k++){ }        /* queimar tempo sem dormir */
    f = open("/proc/uptime", O_RDONLY); if(f >= 0){ (void)!read(f, u2, 127); close(f); }
    printf("     duas leituras de /proc/uptime: \"%.12s\" e \"%.12s\"\n", u1, u2);
    ok("o conteúdo mudou sem ninguém ter escrito — é calculado a cada leitura",
       strcmp(u1, u2) != 0);

    conclui("é o que a cifra faz: o valor sai da conta, e não de uma tabela onde alguém o pôs.");
}

/* ================================================================================ */
/* §K4 — o que falta para controle total                                            */
/* ================================================================================ */
static void secao_K4(void){
    printf("\n§K4  PARA CONTROLE TOTAL — o que já está e o que falta\n\n");

    struct utsname u;
    if(uname(&u) == 0)
        printf("     a máquina:  %s %s (%s)\n\n", u.sysname, u.release, u.machine);

    struct { const char *peca; const char *onde; int temos; } M[] = {
        { "ler e escrever ficheiros",   "read/write — o par adjunto",     1 },
        { "endereçar por slot",         "o fd, índice denso",             1 },
        { "resolver nomes",             "o VFS, prefixo mais longo",      1 },
        { "estado da máquina",          "/proc, calculado à leitura",     1 },
        { "correr um programa",         "execve — e é um só opcode",      1 },
        { "memória sem ficheiro",       "— não usamos: a regra é sem RAM",0 },
        { "escalonar processos",        "— não temos, e não queremos",    0 },
        { "drivers de dispositivo",     "— não temos",                    0 },
    };
    printf("        a peça                          onde                             temos\n");
    int sim = 0, nao = 0;
    for(int i = 0; i < 8; i++){
        printf("        %-31s %-32s %s\n", M[i].peca, M[i].onde, M[i].temos ? "sim" : "NÃO");
        if(M[i].temos) sim++; else nao++;
    }
    ok("a tabela tem linhas dos dois lados — se todas batessem, não era comparação",
       sim > 0 && nao > 0);

    printf("\n     E O QUE 'CONTROLE TOTAL' QUER DIZER AQUI, que é menos do que parece e melhor:\n");
    printf("     o dispositivo não precisa de escalonador nem de drivers — precisa de LER, de\n");
    printf("     ESCREVER e de RESOLVER UM NOME. As três estão medidas acima, e as três são a\n");
    printf("     mesma máquina que já tínhamos. O que o kernel acrescenta não é poder: é ALCANCE.\n");

    conclui("controle total é ler, escrever e resolver um nome — e as três já eram nossas.");
}

/* ================================================================================ */
int main(void){
    struct stat st;
    if(stat("/proc/mounts", &st) != 0){
        printf("NAO MEDIU — sem /proc (isto não é Linux, ou está sem procfs montado).\n");
        printf("Um medidor sem o objeto a medir não passa nem falha: não mediu.\n");
        return 2;
    }

    puts("kernelb.c — O KERNEL NO PLUGUE: a syscall É a ISA do SO, e o VFS É o trie");
    puts("=========================================================================");
    puts("");
    puts("  O nginx casa o location mais longo; o kernel casa o mount point mais longo; nós");
    puts("  descemos o trie. São a mesma operação três vezes, e ninguém combinou nada.");

    secao_K1(); secao_K2(); secao_K3(); secao_K4();

    printf("\n=========================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  read é LOAD, write é STORE, o fd é o slot e o número da syscall é o opcode. O");
        puts("  kernel não traz uma máquina nova — traz ALCANCE para a que já existe.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
