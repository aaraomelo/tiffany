/* conecthus/backends/node/interpretar.c — Node realiza «interpretar» (script JS).
 * Pleno: banco/node.c (-DPLENO_NODE). Slots S_CANAL+9120/9121.
 * Sem helpers internos — só exports, para traduz desce fechar (sobe∘desce=iso).
 *
 * Trial fis:thm:conteudos {−1,0,+1} — sem quarto estado:
 *   −1  return -1     vácuo: recusa (prefixo, aspas, fecho)
 *    0  nin==0       matéria: vazio; (node: sem LF/CR — delimitador q)
 *   +1  return nout  radiação: opera (corpo → OFF_OUT)
 * Três ~ (fis:def:duomorf): célula=arena[slot]; diferença=i==nin,c==q,prefixo;
 *   razão=não usar magnitude no laço. 𝒟 troca diferença↔razão; célula fica. */
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
#define PREFIX_LEN 12

int node_escreve(int in_off, int n){
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

int node_le(int out_off, int max){
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

int node_pronto(void){
    return arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256;
}

int node_pendente(void){
    return arena[OFF_NIN] + arena[OFF_NIN + 1] * 256;
}

#ifndef PLENO_NODE
static int node_corre_semantica(void){
    int nin = arena[OFF_NIN] + arena[OFF_NIN + 1] * 256;
    if(nin == 0) return 0;  /* trial 0: matéria, vazio */
    if(nin > CAP) nin = CAP;
    if(nin < PREFIX_LEN) return -1;  /* trial −1: prefixo curto */
    int k = 0;
    while(k != PREFIX_LEN){
        if(arena[OFF_IN + k] != arena[RODATA_TAG + k]) return -1;  /* ~ diferença; −1 */
        k = k + 1;
    }
    int i = PREFIX_LEN;
    unsigned char q = arena[OFF_IN + i];
    /* trial opera: delimitador q ∈ {' ', "} — diferença, não magnitude */
    if(q != 39){
        if(q != 34) return -1;  /* trial −1: vácuo, aspas inválidas */
    }
    i = i + 1;
    int nout = 0;
    while(i != nin){
        unsigned char c = arena[OFF_IN + i];
        if(c == q) break;  /* ~ diferença: fim do literal */
        if(nout == CAP - 2) break;
        arena[OFF_OUT + nout] = c;
        nout = nout + 1;
        i = i + 1;
    }
    if(i == nin) return -1;  /* trial −1: aspas não fechadas */
    if(arena[OFF_IN + i] != q) return -1;
    arena[OFF_OUT + nout] = 10;
    nout = nout + 1;
    arena[OFF_NIN] = 0;
    arena[OFF_NIN + 1] = 0;
    arena[OFF_NOUT] = (unsigned char)(nout % 256);
    arena[OFF_NOUT + 1] = (unsigned char)(nout / 256);
    arena[OFF_SEQ] = (unsigned char)(arena[OFF_SEQ] + 1);
    return nout;
}
int node_corre(void){
    return node_corre_semantica();
}
int node_move(int in_off, int n, int out_off, int sentido){
    if(sentido < 0) return node_escreve(in_off, n);
    if(n == 0) n = CAP;
    return node_le(out_off, n);
}
#endif
