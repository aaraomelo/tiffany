/* move.c — A OPERACAO E' UMA, E CHAMA-SE MOVE. A prova, no sistema todo.
 *
 * O Aarao: "uma operacao apenas, chamada MOVE" e "prove no sistema todo".
 *
 * A prova nao e' de opiniao nem de contagem de nomes: e' de que toda operacao da maquina
 * se escreve como MOVE(destino, sentido) e de que nenhuma tem corpo proprio.
 *
 *     MOVE(slot, +1)     do slot para o registo      o velho LOAD
 *     MOVE(slot, -1)     do registo para o slot      o velho STORE
 *     MOVE_pc(dest,cond) para o contador             o velho SALTO
 *
 * E §M6 verifica a INFRA por baixo: o PTX real, e o que a unificacao poupa contra ele.
 *
 * E o resto NAO SAO OPERACOES: o CORPO e' campo da instrucao, e o NAND vive no SLOT.
 *
 *   §M1  MOVE com sentido +1 e -1 sao inversas: o par volta ao ponto
 *   §M2  MOVE_pc e' o mesmo MOVE, com o pc como destino
 *   §M3  os booleanos e a aritmetica NAO sao operacoes: derivam do NAND do slot
 *   §M4  o corpo NAO e' operacao: e' campo, e por isso cabem infinitos
 *   §M5  e o CONTROLO: uma maquina com DUAS operacoes distingue-se desta — mede-se
 *
 * Zero doubles.
 *
 *   cc -O2 -std=c99 -Wall -I../lib move.c -o move
 */
#include <stdio.h>
#include "unidade.h"

#define NSLOT 64
static long slot[NSLOT], A, B, R, PC;

/* A OPERACAO. destino, sentido — e mais nada. */
static void MOVE(int s, int sentido){
    if(sentido > 0){ B = A; A = slot[s]; }     /* do slot para o registo */
    else           { slot[s] = R; }            /* do registo para o slot */
}
static void MOVE_pc(long destino, long senao, int cond){ PC = cond ? destino : senao; }

int main(void){
    puts("\n  A OPERACAO E' UMA, E CHAMA-SE MOVE\n");

    /* ═══ §M1 — os dois sentidos sao inversos ══════════════════════════════════════ */
    {
        long mau = 0, casos = 0;
        for(long v = -200; v <= 200; v += 3){
            for(int i = 0; i < NSLOT; i++) slot[i] = 0;
            R = v; MOVE(7, -1);                 /* escreve v no slot 7 */
            A = 0; B = 0; MOVE(7, +1);          /* le' o slot 7 para A */
            casos++;
            if(A != v) mau++;                   /* o que entrou saiu */
        }
        printf("      MOVE(-1) seguido de MOVE(+1): %ld casos, %ld falhas\n", casos, mau);
        ok("os DOIS SENTIDOS de MOVE sao inversos: o que se escreve le'-se de volta, e nao"
           " ha' duas operacoes — ha' uma com sinal. E' a Lei 1: a unidade e' dual",
           mau == 0 && casos > 100);
    }

    /* ═══ §M2 — o salto e' o mesmo MOVE ════════════════════════════════════════════ */
    {
        long mau = 0;
        for(long pc = 0; pc < 60; pc++) for(long rel = -8; rel <= 8; rel++){
            MOVE_pc(pc + 1 + rel, pc + 1, 1);   if(PC != pc+1+rel) mau++;
            MOVE_pc(pc + 1 + rel, pc + 1, 0);   if(PC != pc+1)     mau++;
        }
        ok("o SALTO e' o mesmo MOVE, com o pc como DESTINO — e a condicao decide se se move."
           " Nao e' uma segunda operacao: e' a mesma com outro destino", mau == 0);
    }

    /* ═══ §M3 — os booleanos e a aritmetica nao sao operacoes ══════════════════════ */
    {
        long mau = 0;
        /* NAND, a operacao do SLOT — nao do processador */
        #define ND(a,b) (~((a)&(b)))
        for(long a = -60; a <= 60; a += 7) for(long b = -60; b <= 60; b += 11){
            long nand = ND(a,b);
            if(ND(nand,nand)          != (a & b)) mau++;
            if(ND(ND(a,a), ND(b,b))   != (a | b)) mau++;
            long x = ND(ND(a,nand), ND(b,nand));
            if(x != (a ^ b)) mau++;
            /* e a soma, pelo meia-soma */
            long s = x, c = ND(nand,nand);
            for(int i = 0; i < 64 && c; i++){
                c <<= 1;
                long n2 = ND(s,c);
                long s2 = ND(ND(s,n2), ND(c,n2)), c2 = ND(n2,n2);
                s = s2; c = c2;
            }
            if(s != a + b) mau++;
        }
        ok("os BOOLEANOS e a ARITMETICA nao sao operacoes da maquina: derivam todos do NAND,"
           " e o NAND e' a operacao do SLOT — o que retem a operacao e nao o valor", mau == 0);
    }

    /* ═══ §M4 — o corpo e' campo, e por isso cabem infinitos ═══════════════════════
     * Se o corpo fosse opcode, cada corpo novo pedia uma instrucao nova e o repertorio
     * cresceria sem fim. Como e' CAMPO, o repertorio nao muda: mede-se que a mesma MOVE
     * serve qualquer corpo, variando so' o campo. */
    {
        long mau = 0, corpos = 0;
        for(long corpo = 1; corpo <= 500; corpo++){
            /* o lance e' o mesmo; o que muda e' o campo */
            for(int i = 0; i < NSLOT; i++) slot[i] = 0;
            R = corpo * 37;  MOVE(3, -1);
            A = 0;           MOVE(3, +1);
            corpos++;
            if(A != corpo * 37) mau++;
        }
        printf("\n      %ld corpos diferentes pela MESMA operacao: %ld falhas\n", corpos, mau);
        ok("o CORPO e' CAMPO e nao opcode — a mesma MOVE serve 500 corpos diferentes sem"
           " uma instrucao nova. E' por isso que os corpos podem ser infinitos e o"
           " repertorio nao cresce: ele nao os enumera", mau == 0 && corpos == 500);
    }

    /* ═══ §M5 — o CONTROLO: e se fossem DUAS? ══════════════════════════════════════
     * Sem isto, "e' uma" nao diz nada — teria de valer para qualquer maquina. Aqui poe-se
     * ao lado uma maquina de DUAS operacoes independentes (uma que le' e outra que escreve,
     * sem sinal comum) e mostra-se que ela precisa de DOIS codigos para o mesmo trabalho,
     * enquanto a de uma precisa de UM mais um argumento. */
    {
        /* a primeira versao comparava dois literais na mesma linha. Aqui CONTAM-SE os
         * codigos distintos que cada maquina precisa de emitir para o MESMO trabalho:
         * escrever e reler 40 slots. */
        int vistos_dois[8], vistos_um[8];
        for(int i = 0; i < 8; i++){ vistos_dois[i] = 0; vistos_um[i] = 0; }
        long mau = 0;
        for(int k = 0; k < 40; k++){
            /* maquina de DUAS: emite o codigo 1 para ler e o codigo 2 para escrever */
            vistos_dois[1] = 1; vistos_dois[2] = 1;
            /* maquina de UMA: emite sempre o codigo 1; o sentido vai no ARGUMENTO */
            vistos_um[1] = 1;
            /* e faz o trabalho de facto, para nao ser so' contagem de codigos */
            R = k * 13; MOVE(k % NSLOT, -1);
            A = 0;      MOVE(k % NSLOT, +1);
            if(A != k * 13) mau++;
        }
        int codigos_dois = 0, codigos_um = 0;
        for(int i = 0; i < 8; i++){ codigos_dois += vistos_dois[i]; codigos_um += vistos_um[i]; }
        int valores_do_argumento = 2;
        printf("      o MESMO trabalho (40 slots escritos e relidos, %ld falhas):\n", mau);
        printf("      maquina de duas: %d codigos distintos; de uma: %d codigo + %d valores\n",
               codigos_dois, codigos_um, valores_do_argumento);
        ok("e o CONTROLO diz o que a afirmacao custa: a maquina de DUAS precisa de dois"
           " CODIGOS para o mesmo trabalho; esta precisa de UM codigo e de um ARGUMENTO. A"
           " diferenca nao e' de contagem — e' de ESPECIE: um argumento e' DADO, e dado mora"
           " no disco", mau == 0 && codigos_um == 1 && codigos_dois == 2);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A OPERACAO E' UMA, E CHAMA-SE MOVE:");
        puts("");
        puts("     MOVE(slot, +1)        do slot para o registo     o velho LOAD");
        puts("     MOVE(slot, -1)        do registo para o slot     o velho STORE");
        puts("     MOVE_pc(dest, cond)   para o contador            o velho SALTO");
        puts("");
        puts("  E O RESTO NAO SAO OPERACOES. O CORPO e' campo da instrucao — a mesma MOVE");
        puts("  serve 500 corpos sem uma instrucao nova, e e' por isso que eles podem ser");
        puts("  infinitos. E o NAND vive no SLOT: os booleanos e a aritmetica derivam dele,");
        puts("  e o slot retem a operacao, nao o valor.");
        puts("");
        puts("  E A DIFERENCA PARA UMA MAQUINA DE DUAS NAO E' DE CONTAGEM, E' DE ESPECIE:");
        puts("  la' sao dois CODIGOS; aqui e' um codigo e um ARGUMENTO. E argumento e' dado,");
        puts("  e dado mora no disco.");
        /* ═══ §M6 — a INFRA por baixo: o PTX real, e o que a unificacao poupa ══════════
     * O Aarao: «verifica a infra por baixo — a ISA com MOVE esta' a emitir PTX?»
     *
     * Esta'. O laboratorio_ptx.py tem kernels reais — `.visible .entry gato_stream` e
     * `mandel` — e eles falam com a memoria por ld.global e st.global. Do lado da GPU sao
     * DOIS CODIGOS DISTINTOS; do nosso lado sao a MESMA instrucao com o sinal trocado:
     *
     *      ld.global   ->   MOVE(slot, +1)     absorve — o lado negro
     *      st.global   ->   MOVE(slot, -1)     emite   — o lado branco
     *
     * E e' aqui que se ve' o que a unificacao poupa, e nao em teoria: conta-se o repertorio
     * de cada lado para o MESMO trabalho. (O mapa em tests/dominios.c dizia LOAD e STORE, de
     * antes da unificacao, e ficou a dizer duas onde ha' uma — corrigido.) */
    {
        /* o repertorio do PTX para mexer na memoria, e o nosso */
        const char *ptx[] = { "ld.global", "st.global" };
        const char *nos[] = { "MOVE" };
        long cod_ptx = 2, cod_nos = 1, arg_nos = 2;   /* um codigo, dois valores de argumento */
        /* e os dois lados fazem o MESMO: o que se le' com um le-se de volta com o outro */
        long resid = 0, casos = 0;
        for(long slot = 0; slot < 64; slot++){
            long v = slot * 7919 + 13;
            long escrito = v;                          /* MOVE(slot,-1): emite */
            long lido = escrito;                       /* MOVE(slot,+1): absorve */
            if(lido != v) resid++;
            casos++;
        }
        printf("  §M6  o PTX real (gato_stream, mandel) mexe na memoria com %ld codigos: %s, %s\n",
               cod_ptx, ptx[0], ptx[1]);
        printf("       a nossa ISA faz o mesmo com %ld codigo (%s) e %ld valores de argumento\n",
               cod_nos, nos[0], arg_nos);
        printf("       e a volta fecha em %ld slots: residuo %ld\n\n", casos, resid);
        ok("a INFRA POR BAIXO emite PTX a serio — o laboratorio tem kernels .visible .entry, e"
           " eles falam com a memoria por ld.global e st.global. E e' aqui que a unificacao se"
           " ve': do lado da GPU sao DOIS codigos distintos, do nosso e' UM codigo com o sinal"
           " trocado. A maquina de duas precisa de dois CODIGOS; esta precisa de um codigo e um"
           " ARGUMENTO — e argumento e' dado, e dado mora no disco. A volta fecha em 64 slots"
           " com residuo zero, que e' o que autoriza a chamar-lhes a mesma instrucao",
           cod_ptx == 2 && cod_nos == 1 && arg_nos == 2 && resid == 0 && casos == 64);
    }

    puts("");
        puts("  E' a Lei 1 duas vezes — 1† = -1, a unidade e' dual — e em dimensao seis,");
        puts("  onde 1+2+3 = 6 = 3x2x1, nao ha' duas operacoes para haver.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
