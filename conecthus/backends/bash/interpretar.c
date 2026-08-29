/* conecthus/backends/bash/interpretar.c — bash realiza «interpretar» (porta única).
 * Pleno: banco/bash.c (-DPLENO_BASH). Slots S_CANAL+9100/9101.
 *
 * Trial fis:thm:conteudos {−1,0,+1} — sem quarto estado:
 *   −1  return -1     vácuo: prefixo rodata falha
 *    0  nin==0; c==10/13 break   matéria: vazio; fronteira LF/CR
 *   +1  return nout   radiação: corpo → OFF_OUT (+ LF)
 * Três ~: célula=arena[slot]; diferença=prefixo VINCO, c==10/13; razão=não no laço. */

#ifdef SHELL_ARENA_EXTERN
extern unsigned char arena[65536];
#else
unsigned char arena[65536];
#endif

#define OFF_NIN  24576
#define OFF_NOUT 24578
#define OFF_SEQ  24580
#define OFF_IN   256
#define OFF_OUT  16384
#define CAP      8192
#define RODATA_TAG 65408
#define PREFIX_LEN 5

int bash_escreve(int in_off, int n){
    int nin = arena[OFF_NIN] + arena[OFF_NIN + 1] * 256;
    int i = 0;
    while(i != n){
        if(nin == CAP) break;
        arena[OFF_IN + nin] = arena[in_off + i];
        nin = nin + 1;
        i = i + 1;
    }
    if(nin > 65535) nin = 65535;
    arena[OFF_NIN] = (unsigned char)(nin % 256);
    arena[OFF_NIN + 1] = (unsigned char)(nin / 256);
    return i;
}

int bash_le(int out_off, int max){
    int nout = arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256;
    if(max > nout) max = nout;
    if(max > CAP) max = CAP;
    int i = 0;
    while(i != max){
        arena[out_off + i] = arena[OFF_OUT + i];
        i = i + 1;
    }
    int resto = nout - max;
    int j = 0;
    while(j != resto){
        arena[OFF_OUT + j] = arena[OFF_OUT + j + max];
        j = j + 1;
    }
    if(resto > 65535) resto = 65535;
    arena[OFF_NOUT] = (unsigned char)(resto % 256);
    arena[OFF_NOUT + 1] = (unsigned char)(resto / 256);
    return max;
}

int bash_pronto(void){
    return arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256;
}

int bash_pendente(void){
    return arena[OFF_NIN] + arena[OFF_NIN + 1] * 256;
}

#ifndef PLENO_BASH
static int bash_corre_semantica(void){
    int nin = arena[OFF_NIN] + arena[OFF_NIN + 1] * 256;
    if(nin == 0) return 0;  /* trial 0: matéria, vazio */
    if(nin > CAP) nin = CAP;
    if(nin < PREFIX_LEN) return -1;  /* trial −1 */
    int k = 0;
    while(k != PREFIX_LEN){
        if(arena[OFF_IN + k] != arena[RODATA_TAG + k]) return -1;  /* ~ diferença; −1 */
        k = k + 1;
    }
    int i = PREFIX_LEN;
    int nout = 0;
    while(i != nin){
        unsigned char c = arena[OFF_IN + i];
        if(c == 10) break;  /* trial 0: fronteira LF — ~ diferença */
        if(c == 13) break;  /* trial 0: fronteira CR — ~ diferença */
        if(nout == CAP - 2) break;
        arena[OFF_OUT + nout] = c;
        nout = nout + 1;
        i = i + 1;
    }
    arena[OFF_OUT + nout] = 10;
    nout = nout + 1;
    arena[OFF_NIN] = 0;
    arena[OFF_NIN + 1] = 0;
    arena[OFF_NOUT] = (unsigned char)(nout % 256);
    arena[OFF_NOUT + 1] = (unsigned char)(nout / 256);
    arena[OFF_SEQ] = (unsigned char)(arena[OFF_SEQ] + 1);
    return nout;
}
int bash_corre(void){
    return bash_corre_semantica();
}
int bash_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0) return bash_escreve(in_off, n);
    if(n == 0) n = CAP;
    return bash_le(out_off, n);
}
#endif
