/* banco/bash.c — pleno bash (bash in.sh no metal).
 *
 * Compilar com interpretar.c:
 *   cc -O2 -std=c99 banco/bash.c conecthus/backends/bash/interpretar.c -o bash
 *
 * bash_corre lê o stdin da arena, corre o bash ingerido, escreve no stdout da arena.
 * No browser só corre interpretar.wasm; bash_corre existe só no metal. */
#define PLENO_BASH 1
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiffany_shell.h"

#ifdef _WIN32
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

int bash_escreve(int in_off, int n);
int bash_le(int out_off, int max);
int bash_pronto(void);
int bash_pendente(void);

int bash_corre(void){
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

    char dir[256], path[320], run[640];
#ifdef _WIN32
    const char *tmp = getenv("TEMP");
    if(!tmp || !*tmp) tmp = "C:\\temp";
    snprintf(dir, sizeof dir, "%s\\tiffany_bash", tmp);
#else
    snprintf(dir, sizeof dir, "/tmp/tiffany_bash");
#endif
    mkdir_p(dir);
    snprintf(path, sizeof path, "%s/in.sh", dir);
    FILE *sh = fopen(path, "wb");
    if(!sh) return -1;
    fwrite(script, 1, (size_t)i, sh);
    fclose(sh);

#ifdef _WIN32
    snprintf(run, sizeof run, "\"%s\" \"%s\"", tiffany_bash_bin(), path);
#else
    snprintf(run, sizeof run, "\"%s\" \"%s\"", tiffany_bash_bin(), path);
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

int bash_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0){
        bash_escreve(in_off, n);
        bash_corre();
        return le16(OFF_NOUT);
    }
    if(n <= 0) n = CAP;
    return bash_le(out_off, n);
}
