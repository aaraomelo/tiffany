/* canal_patria.c — lado branco do barramento: atende S_FRONT_* e shells na Patria.
 *
 *   cc -O2 -std=c99 -DSHELL_ARENA_EXTERN -DPLENO_NODE -DPLENO_BASH -DPLENO_PWSH \
 *       banco/canal_patria.c conecthus/backends/shell/arena.c \
 *       conecthus/backends/node/interpretar.c banco/node.c \
 *       conecthus/backends/bash/interpretar.c banco/bash.c \
 *       conecthus/backends/powershell/interpretar.c banco/powershell.c -o canal_patria
 *   TIFFANY_CANAL_IF=any ./canal_patria ./sql /caminho/base
 *
 * Node: arena de interpretar.c + node_corre() de banco/node.c — sem NODE MOVE SQL. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "banda.h"

#define ISA_TECTO (1u << 16)
#define ZBITS 14
#define ZONA(k) ((unsigned)(k) << ZBITS)
#define S_CANAL   (ISA_TECTO + ZONA(600))
#define S_BASH_IN  (S_CANAL + 9100u)
#define S_BASH_OUT (S_CANAL + 9101u)
#define S_CHUNK    (S_CANAL + 9102u)
#define S_NODE_IN  (S_CANAL + 9120u)
#define S_NODE_OUT (S_CANAL + 9121u)
#define S_PWSH_IN  (S_CANAL + 9110u)
#define S_PWSH_OUT (S_CANAL + 9111u)
#define S_FRONT_REQ (S_CANAL + 9200u)
#define S_FRONT_RSP (S_CANAL + 9201u)

static int canal_fd = -1;
static unsigned char canal_banda[32];
static struct sockaddr_in canal_dst;
static char sql_bin[512];
static char sql_base[512];
static char repo_raiz[512];

static unsigned long canal_if_addr(void){
    const char *v = getenv("TIFFANY_CANAL_IF");
    if(!v || !*v || !strcmp(v, "loopback")) return htonl(INADDR_LOOPBACK);
    if(!strcmp(v, "any")) return INADDR_ANY;
    return inet_addr(v);
}

static void canal_abre(void){
    if(canal_fd >= 0) return;
    banda_de(getenv("TIFFANY_TECIDO") ? getenv("TIFFANY_TECIDO") : "tecido por omissao",
             canal_banda);
    const char *grupo = getenv("TIFFANY_CANAL_GRUPO");
    if(!grupo || !*grupo) grupo = "239.7.31.27";
    const char *porta_s = getenv("TIFFANY_CANAL_PORTA");
    unsigned porta = porta_s && *porta_s ? (unsigned)atoi(porta_s) : 47313u;
    canal_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int um = 1;
    setsockopt(canal_fd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof um);
    struct sockaddr_in e;
    memset(&e, 0, sizeof e);
    e.sin_family = AF_INET;
    e.sin_addr.s_addr = htonl(INADDR_ANY);
    e.sin_port = htons((uint16_t)porta);
    bind(canal_fd, (struct sockaddr*)&e, sizeof e);
    struct ip_mreq mr;
    mr.imr_multiaddr.s_addr = inet_addr(grupo);
    mr.imr_interface.s_addr = canal_if_addr();
    setsockopt(canal_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof mr);
    unsigned i = canal_if_addr();
    setsockopt(canal_fd, IPPROTO_IP, IP_MULTICAST_IF, &i, sizeof i);
    setsockopt(canal_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &um, sizeof um);
    memset(&canal_dst, 0, sizeof canal_dst);
    canal_dst.sin_family = AF_INET;
    canal_dst.sin_addr.s_addr = inet_addr(grupo);
    canal_dst.sin_port = htons((uint16_t)porta);
}

static void canal_grava(unsigned slot, unsigned char total, unsigned char e){
    canal_abre();
    unsigned char m[6], ks[6], b[6];
    memcpy(m, &slot, 4);
    m[4] = total;
    m[5] = e;
    keystream(canal_banda, ks, 6);
    bump(m, ks, b, 6);
    sendto(canal_fd, b, 6, 0, (struct sockaddr*)&canal_dst, sizeof canal_dst);
}

static int canal_recebe(unsigned *slot, unsigned char *total, unsigned char *e){
    canal_abre();
    unsigned char ks[6], b[6], m[6];
    keystream(canal_banda, ks, 6);
    for(;;){
        socklen_t n = sizeof canal_dst;
        ssize_t g = recvfrom(canal_fd, b, 6, 0, (struct sockaddr*)&canal_dst, &n);
        if(g != 6) return 0;
        bump(b, ks, m, 6);
        memcpy(slot, m, 4);
        *total = m[4];
        *e = m[5];
        return 1;
    }
}

static void envia_corpo(unsigned slot_fim, const unsigned char *buf, int len){
    int i = 0;
    while(i < len){
        unsigned char a = buf[i++];
        unsigned char c = (i < len) ? buf[i++] : 0;
        canal_grava(S_CHUNK, a, c);
    }
    canal_grava(slot_fim, (unsigned char)(len & 255), (unsigned char)((len >> 8) & 255));
}

static int run_sql(const char *q, unsigned char *out, int cap){
    char cmd[4096];
    snprintf(cmd, sizeof cmd, "\"%s\" \"%s\" \"%s\"", sql_bin, sql_base, q);
    FILE *f = popen(cmd, "r");
    if(!f) return -1;
    int n = 0;
    int c;
    while((c = fgetc(f)) != EOF && n + 1 < cap)
        out[n++] = (unsigned char)c;
    out[n] = 0;
    pclose(f);
    return n;
}

static const char *front_paths[] = {
    "app/banco/pagina.html",
    "app/banco/pagina.css",
    "app/banco/pagina.js",
};

static int le_ficheiro(const char *rel, unsigned char *out, int cap){
    char path[768];
    if(repo_raiz[0])
        snprintf(path, sizeof path, "%s/%s", repo_raiz, rel);
    else
        snprintf(path, sizeof path, "%s", rel);
    FILE *f = fopen(path, "rb");
    if(!f) return -1;
    int n = (int)fread(out, 1, (size_t)(cap - 1), f);
    fclose(f);
    out[n] = 0;
    return n;
}

static void serve_front(unsigned char kind){
    unsigned char buf[65536];
    int n = -1;
    if(kind < 3){
        char q[256];
        snprintf(q, sizeof q, "GET CORPO '%s'", front_paths[kind]);
        n = run_sql(q, buf, (int)sizeof buf);
        if(n <= 0) n = le_ficheiro(front_paths[kind], buf, (int)sizeof buf);
    }
    if(n <= 0){
        fprintf(stderr, "canal_patria: FRONT kind %u falhou\n", (unsigned)kind);
        envia_corpo(S_FRONT_RSP, (const unsigned char*)"", 0);
        return;
    }
    fprintf(stderr, "canal_patria: FRONT %s (%d bytes)\n", front_paths[kind], n);
    envia_corpo(S_FRONT_RSP, buf, n);
}

static unsigned char chunk_buf[8192];
static int chunk_len = 0;

#include <sys/stat.h>

extern unsigned char arena[65536];

#define OFF_NIN  24576
#define OFF_IN   256
#define OFF_OUT  16384

int node_corre(void);

int bash_corre(void);
int powershell_corre(void);

static void shell_slot(int nin, int (*corre)(void), unsigned out_slot, const char *tag){
    if(nin <= 0 || nin > chunk_len) nin = chunk_len;
    if(nin <= 0) return;
    arena[OFF_NIN] = (unsigned char)(nin & 255);
    arena[OFF_NIN + 1] = (unsigned char)((nin >> 8) & 255);
    arena[OFF_NIN + 2] = 0;
    arena[OFF_NIN + 3] = 0;
    int i = 0;
    while(i < nin){
        arena[OFF_IN + i] = chunk_buf[i];
        i = i + 1;
    }
    chunk_len = 0;
    int nout = corre();
    if(nout <= 0){
        envia_corpo(out_slot, (const unsigned char*)"", 0);
        return;
    }
    fprintf(stderr, "canal_patria: %s %d bytes\n", tag, nout);
    envia_corpo(out_slot, &arena[OFF_OUT], nout);
}

static void bash_slot(int nin){
    shell_slot(nin, bash_corre, S_BASH_OUT, "bash_corre");
}

static void node_slot(int nin){
    shell_slot(nin, node_corre, S_NODE_OUT, "node_corre");
}

static void powershell_slot(int nin){
    shell_slot(nin, powershell_corre, S_PWSH_OUT, "powershell_corre");
}

int main(int argc, char **argv){
    if(argc < 3){
        fprintf(stderr, "uso: canal_patria <sql.bin> <base> [repo_raiz]\n");
        return 1;
    }
    snprintf(sql_bin, sizeof sql_bin, "%s", argv[1]);
    snprintf(sql_base, sizeof sql_base, "%s", argv[2]);
    if(argc > 3) snprintf(repo_raiz, sizeof repo_raiz, "%s", argv[3]);
    const char *r = getenv("TIFFANY_RAIZ");
    if(r && *r) snprintf(repo_raiz, sizeof repo_raiz, "%s", r);
    fprintf(stderr, "canal_patria: sql=%s base=%s raiz=%s\n",
            sql_bin, sql_base, repo_raiz[0] ? repo_raiz : "(corpo)");
    fprintf(stderr, "canal_patria: TIFFANY_CANAL_IF=%s — a escutar bumps\n",
            getenv("TIFFANY_CANAL_IF") ? getenv("TIFFANY_CANAL_IF") : "loopback");
    for(;;){
        unsigned slot = 0;
        unsigned char t = 0, e = 0;
        if(!canal_recebe(&slot, &t, &e)) continue;
        if(slot == S_CHUNK){
            if(chunk_len + 2 <= (int)sizeof chunk_buf){
                chunk_buf[chunk_len++] = t;
                chunk_buf[chunk_len++] = e;
            }
            continue;
        }
        if(slot == S_FRONT_REQ){
            chunk_len = 0;
            serve_front(t);
            continue;
        }
        if(slot == S_BASH_IN){
            int nin = (int)t + ((int)e << 8);
            bash_slot(nin);
            continue;
        }
        if(slot == S_NODE_IN){
            int nin = (int)t + ((int)e << 8);
            node_slot(nin);
            continue;
        }
        if(slot == S_PWSH_IN){
            int nin = (int)t + ((int)e << 8);
            powershell_slot(nin);
            continue;
        }
    }
    return 0;
}
