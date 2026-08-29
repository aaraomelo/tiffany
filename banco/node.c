/* banco/node.c — pleno Node (node in.js no metal).
 *   cc -O2 -std=c99 -DPLENO_NODE banco/node.c conecthus/backends/node/interpretar.c -o node_shell */
#define PLENO_NODE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiffany_node.h"

#ifdef _WIN32
#define _POSIX_C_SOURCE 200809L
#include <direct.h>
#define mkdir_p(p) _mkdir(p)
#else
#include <sys/stat.h>
#define mkdir_p(p) mkdir(p, 0755)
#endif

extern unsigned char arena[65536];

#define OFF_NIN  24576
#define OFF_NOUT 24578
#define OFF_SEQ  24580
#define OFF_IN   256
#define OFF_OUT  16384
#define CAP      8192

static int le16(int off){
    return arena[off] + arena[off + 1] * 256;
}
static void grava16(int off, int v){
    if(v < 0) v = 0;
    if(v > 65535) v = 65535;
    arena[off] = (unsigned char)(v % 256);
    arena[off + 1] = (unsigned char)(v / 256);
}

int node_escreve(int in_off, int n);
int node_le(int out_off, int max);
int node_pronto(void);
int node_pendente(void);

static const char *node_bin(void){
    return tiffany_node_bin();
}

int node_corre(void){
    int nin = le16(OFF_NIN);
    if(nin <= 0) return 0;
    if(nin > CAP) nin = CAP;
    char script[CAP + 1];
    int i = 0;
    while(i < nin){
        script[i] = (char)arena[OFF_IN + i];
        i = i + 1;
    }
    script[i] = 0;
    grava16(OFF_NIN, 0);

    char dir[256], path[320], run[384];
#ifdef _WIN32
    const char *tmp = getenv("TEMP");
    if(!tmp || !*tmp) tmp = "C:\\temp";
    snprintf(dir, sizeof dir, "%s\\tiffany_node", tmp);
#else
    snprintf(dir, sizeof dir, "/tmp/tiffany_node");
#endif
    mkdir_p(dir);
    snprintf(path, sizeof path, "%s/in.js", dir);
    FILE *js = fopen(path, "wb");
    if(!js) return -1;
    fwrite(script, 1, (size_t)i, js);
    fclose(js);

#ifdef _WIN32
    snprintf(run, sizeof run, "%s \"%s\"", node_bin(), path);
#else
    snprintf(run, sizeof run, "\"%s\" \"%s\"", node_bin(), path);
#endif
    FILE *f = popen(run, "r");
    if(!f) return -1;

    int nout = le16(OFF_NOUT);
    char buf[256];
    while(nout < CAP){
        size_t r = fread(buf, 1, 256, f);
        if(r == 0) break;
        int j = 0;
        while(j < (int)r && nout < CAP){
            arena[OFF_OUT + nout] = (unsigned char)buf[j];
            nout = nout + 1;
            j = j + 1;
        }
    }
    pclose(f);
    grava16(OFF_NOUT, nout);
    grava16(OFF_SEQ, le16(OFF_SEQ) + 1);
    return nout;
}

int node_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0){
        node_escreve(in_off, n);
        node_corre();
        return le16(OFF_NOUT);
    }
    if(n <= 0) n = CAP;
    return node_le(out_off, n);
}
