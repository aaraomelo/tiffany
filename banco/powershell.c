/* banco/powershell.c — pleno PowerShell (Windows local; pwsh noutros).
 *   cc -O2 -std=c99 -DPLENO_PWSH banco/powershell.c conecthus/backends/powershell/interpretar.c -o powershell */
#define PLENO_PWSH 1
#include <stdio.h>
#include <string.h>

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

int powershell_escreve(int in_off, int n);
int powershell_le(int out_off, int max);
int powershell_pronto(void);
int powershell_pendente(void);

static FILE *powershell_popen(const char *script){
#ifdef _WIN32
    char run[CAP + 96];
    snprintf(run, sizeof run, "powershell -NoProfile -ExecutionPolicy Bypass -Command \"%s\"", script);
    return popen(run, "r");
#else
    char run[CAP + 64];
    snprintf(run, sizeof run, "pwsh -NoProfile -Command \"%s\"", script);
    return popen(run, "r");
#endif
}

int powershell_corre(void){
    int nin = le16(OFF_NIN);
    if(nin <= 0) return 0;
    if(nin > CAP) nin = CAP;
    char cmd[CAP + 1];
    int i = 0;
    while(i < nin){
        cmd[i] = (char)arena[OFF_IN + i];
        i = i + 1;
    }
    cmd[i] = 0;
    grava16(OFF_NIN, 0);

    FILE *f = powershell_popen(cmd);
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

int powershell_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0){
        powershell_escreve(in_off, n);
        powershell_corre();
        return le16(OFF_NOUT);
    }
    if(n <= 0) n = CAP;
    return powershell_le(out_off, n);
}
