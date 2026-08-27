/* banda_viva.c — DOIS BANCOS, DOIS PROCESSOS, UMA BANDA, POR UDP.
 *
 * O `tests/canal.c` prova a antena entre dois sockets no MESMO processo. Isto
 * prova o degrau seguinte, que é o que o Aarão pede: dois PROCESSOS distintos —
 * dois bancos — a falar pela banda por UDP a sério, com a mesma trama do canal
 * do `banco/sql.c` (slot·4 + Word·2, bump-ada com a banda do tecido).
 *
 * Não reinventa protocolo: usa `lib/banda.h` (banda = sha256(tecido), keystream,
 * bump = XOR = J), e o mesmo grupo/porta/interface que o `sql.c` já lê do
 * ambiente. É «o banco via UDP» posto a andar, e medido onde o canal existe.
 *
 *   TIFFANY_TECIDO       a banda exclusiva (por omissão "tecido por omissao")
 *   TIFFANY_CANAL_GRUPO  o multicast (239.7.31.27)   TIFFANY_CANAL_PORTA (47313)
 *   TIFFANY_CANAL_IF     loopback | any | <ip da placa>
 *
 *   cc -O2 -std=c99 -Ilib -o /tmp/bv tests/banda_viva.c
 *   /tmp/bv le 8 &          # o banco que escuta, na banda
 *   /tmp/bv escreve 8       # o banco que fala
 *   # ou, tudo num: /tmp/bv prova 8
 */
#define _POSIX_C_SOURCE 200809L
/* _DEFAULT_SOURCE senão o -std=c99 estrito esconde `struct ip_mreq` e o
 * ficheiro não compila — a mesma lição que está no cabeçalho do banco/sql.c. */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "banda.h"

/* a mesma leitura de ambiente que o canal do sql.c, para não haver dois donos da verdade */
static unsigned long if_addr(void){
    const char *v = getenv("TIFFANY_CANAL_IF");
    if(!v || !*v || !strcmp(v,"loopback")) return INADDR_LOOPBACK;
    if(!strcmp(v,"any")) return INADDR_ANY;
    unsigned long a = ntohl(inet_addr(v));
    return a ? a : INADDR_LOOPBACK;
}
static const char *grupo(void){ const char*g=getenv("TIFFANY_CANAL_GRUPO"); return (g&&*g)?g:"239.7.31.27"; }
static unsigned short porta(void){ const char*p=getenv("TIFFANY_CANAL_PORTA"); return (p&&*p)?(unsigned short)atoi(p):47313; }
static const char *tecido(void){ const char*t=getenv("TIFFANY_TECIDO"); return (t&&*t)?t:"tecido por omissao"; }

static int abre(struct sockaddr_in *dst){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    int um = 1; setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof um);
    struct sockaddr_in e; memset(&e,0,sizeof e);
    e.sin_family=AF_INET; e.sin_addr.s_addr=htonl(INADDR_ANY); e.sin_port=htons(porta());
    bind(fd,(struct sockaddr*)&e,sizeof e);
    struct ip_mreq mr; mr.imr_multiaddr.s_addr=inet_addr(grupo()); mr.imr_interface.s_addr=htonl(if_addr());
    setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof mr);
    struct in_addr i; i.s_addr=htonl(if_addr());
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &i, sizeof i);
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &um, sizeof um);
    struct timeval to={2,0}; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
    memset(dst,0,sizeof *dst); dst->sin_family=AF_INET;
    dst->sin_addr.s_addr=inet_addr(grupo()); dst->sin_port=htons(porta());
    return fd;
}
/* a trama do canal: 4 bytes de slot, depois total e e — bump-ada */
static void trama(unsigned slot, unsigned char total, unsigned char e,
                  const unsigned char *banda, unsigned char *out6){
    unsigned char m[6], ks[6]; memcpy(m,&slot,4); m[4]=total; m[5]=e;
    keystream(banda, ks, 6); bump(m, ks, out6, 6);
}

static int escreve(int n){
    unsigned char banda[32]; banda_de(tecido(), banda);
    struct sockaddr_in dst; int fd = abre(&dst);
    usleep(150000);                       /* dá ao leitor tempo de entrar no grupo */
    for(int i=0;i<n;i++){
        unsigned char b[6];
        trama(1000+(unsigned)i, (unsigned char)i, (unsigned char)(i*2), banda, b);
        sendto(fd, b, 6, 0, (struct sockaddr*)&dst, sizeof dst);
        usleep(20000);
    }
    return 0;
}
static int le(int n){
    unsigned char banda[32]; banda_de(tecido(), banda);
    unsigned char ks[6]; keystream(banda, ks, 6);
    struct sockaddr_in dst; int fd = abre(&dst);
    int certos=0, vistos=0;
    for(int guarda=0; guarda < n*4 && certos < n; guarda++){
        unsigned char b[6], m[6]; struct sockaddr_in de; socklen_t sl=sizeof de;
        long r = recvfrom(fd, b, 6, 0, (struct sockaddr*)&de, &sl);
        if(r != 6) break;                 /* timeout: o meio calou-se */
        vistos++;
        bump(b, ks, m, 6);                /* desbumpar com a MINHA banda */
        unsigned slot; memcpy(&slot,m,4);
        unsigned i = slot - 1000u;
        if(i < (unsigned)n && m[4]==(unsigned char)i && m[5]==(unsigned char)(i*2)) certos++;
    }
    printf("#UNIT %s  dois processos na banda: %d/%d Words chegaram inteiros por UDP (vistos %d)\n",
           certos==n ? "ok   " : "falha", certos, n, vistos);
    return certos==n ? 0 : 1;
}
/* a criptofiltração, entre processos: um leitor com a banda ERRADA não lê a mensagem */
static int le_errado(int n){
    unsigned char banda[32]; banda_de("banda errada, nao e o tecido", banda);
    unsigned char ks[6]; keystream(banda, ks, 6);
    struct sockaddr_in dst; int fd = abre(&dst);
    int enganados=0, vistos=0;
    for(int guarda=0; guarda < n*4 && vistos < n; guarda++){
        unsigned char b[6], m[6]; struct sockaddr_in de; socklen_t sl=sizeof de;
        if(recvfrom(fd, b, 6, 0, (struct sockaddr*)&de, &sl) != 6) break;
        vistos++; bump(b, ks, m, 6);
        unsigned slot; memcpy(&slot,m,4); unsigned i=slot-1000u;
        if(i < (unsigned)n && m[4]==(unsigned char)i && m[5]==(unsigned char)(i*2)) enganados++;
    }
    printf("#UNIT %s  a banda errada NAO le a mensagem: %d de %d datagramas decifraram (tem de ser 0)\n",
           enganados==0 ? "ok   " : "falha", enganados, vistos);
    return enganados==0 ? 0 : 1;
}

int main(int argc, char **argv){
    const char *papel = argc>1 ? argv[1] : "prova";
    int n = argc>2 ? atoi(argv[2]) : 8; if(n<1) n=8; if(n>200) n=200;
    if(!strcmp(papel,"escreve")) return escreve(n);
    if(!strcmp(papel,"le"))      return le(n);
    if(!strcmp(papel,"prova")){
        /* um processo escuta certo, um escuta errado, e um terceiro fala */
        printf("== A BANDA VIVA: dois bancos, por UDP, tecido=\"%s\" ==\n", tecido());
        fflush(stdout);                 /* esvazia o buffer ANTES do fork, senão o banner sai 3x */
        int p_le = fork();
        if(p_le==0){ int r=le(n); fflush(stdout); _exit(r); }        /* _exit não faz flush do pipe */
        int p_err = fork();
        if(p_err==0){ int r=le_errado(n); fflush(stdout); _exit(r); }
        usleep(80000);
        escreve(n);
        fflush(stdout);
        int s1=0,s2=0; waitpid(p_le,&s1,0); waitpid(p_err,&s2,0);
        int r = (WIFEXITED(s1)?WEXITSTATUS(s1):1) | (WIFEXITED(s2)?WEXITSTATUS(s2):1);
        printf(r? "\nfalhou\n" : "\na banda esta viva: quem tem o tecido le, quem nao tem ouve ruido.\n");
        return r;
    }
    fprintf(stderr, "uso: banda_viva le|escreve|prova [n]\n");
    return 2;
}
