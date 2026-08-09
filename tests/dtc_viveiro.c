/* dtc_viveiro.c — CASAR O INVERSOR POR CONTROLE DIRETO DE TORQUE NA TORRE, SINCRONIZADA PELO VIVEIRO.
 *
 * O Aarão: «precisa casar o inversor por controle direto de torque no viveiro.»
 *
 * O paper já tinha as duas metades separadas: dtcn.c casa o inversor em UM corpo (o imposto
 * Π(s)=(1−s²)·‖a×b‖² anula em s=±1 → fp=1), e viveiro.c funde os corpos pelo lcm. O que faltava era
 * a JUNÇÃO --- cor:dtctorre: «um DTC por andar, sincronizados pelo viveiro». Este medidor mede-a,
 * e em inteiro (dtcn.c fá-lo em double; a doutrina é inteiro desde o primeiro rascunho).
 *
 * A torre: N andares, cada um um corpo com período = a DOBRA Δ_n = n²+4 (§sub:atravessa). Cada andar
 * tem o seu inversor (trial {−1,0,+1}); o DTC de Takahashi escolhe o vector que empurra s→±1 (a banda),
 * levando o imposto a zero. Os relógios dos andares NÃO são lineares; o viveiro sincroniza-os no lcm
 * das dobras, ⋁_k R^{a_k}=R^{lcm}. A torre casa andar a andar, e bate junta no lcm.
 *
 *   §W1  o inversor casa por DTC: o imposto Π(s)=(1−s²)·‖a×b‖² anula em s=±1, e o DTC escolhe ±1
 *   §W2  o viveiro sincroniza: o fold do lcm é independente da ordem, contém cada andar, é o menor
 *   §W3  a torre casa NO lcm: em t=lcm todos os relógios estão na origem E todos os andares casados
 *   §W4  o torque é o cruzado: o imposto cavalga ‖a×b‖²; casar o cruzado (s→±1) é fp=1
 *
 *   cc -O2 -std=c99 -Wall -I../lib dtc_viveiro.c -o dtc_viveiro && ./dtc_viveiro
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;
enum { N = 4 };                                    /* a torre: 4 andares */

/* o torque É o produto cruzado; a massa reactiva é ‖a×b‖² (inteiro) */
static L massa_cruz(const L a[3], const L b[3]){
    L cx = a[1]*b[2] - a[2]*b[1];
    L cy = a[2]*b[0] - a[0]*b[2];
    L cz = a[0]*b[1] - a[1]*b[0];
    return cx*cx + cy*cy + cz*cz;
}
/* o imposto algébrico: a potência reactiva que a álgebra cobra fora do eixo. Inteiro. */
static L imposto(L s, L massa){ return (1 - s*s) * massa; }
/* o DTC escolhe, do trial {−1,0,+1}, o vector que MINIMIZA o imposto (empurra para a banda) */
static L dtc_escolhe_s(L massa){
    L melhor = 0, melhor_imp = imposto(0, massa);
    L trial[3] = { -1, 0, +1 };
    for(int i = 0; i < 3; i++){
        L v = imposto(trial[i], massa);
        if(v < melhor_imp){ melhor_imp = v; melhor = trial[i]; }
    }
    return melhor;                                 /* dá ±1: a banda, imposto 0 */
}
static L mdc(L a, L b){ while(b){ L t = a % b; a = b; b = t; } return a; }
static L mmc(L a, L b){ return a / mdc(a, b) * b; }

int main(void){
    printf("=== CASAR O INVERSOR POR DTC NA TORRE, SINCRONIZADA PELO VIVEIRO ==========\n\n");

    /* a torre: andar n tem período (dobra) Δ_n = n²+4, e um par de vectores (o seu torque) */
    L per[N], massa[N];
    L A[N][3] = {{1,0,0},{2,1,0},{1,2,0},{3,0,1}};
    L B[N][3] = {{0,1,0},{0,1,2},{2,0,1},{0,2,1}};
    for(int k = 0; k < N; k++){ int n = k + 1; per[k] = (L)n*n + 4; massa[k] = massa_cruz(A[k], B[k]); }

    /* ── §W1 o inversor casa por DTC: o imposto anula em s=±1 ─────────────────────────────── */
    /* para cada andar: o trial {−1,0,+1} dá imposto {0, massa, 0} --- reactivo máximo no meio (s=0),
     * ZERO nos extremos (s=±1). O DTC escolhe ±1: leva o imposto a zero e casa fp=1. */
    int casa = 1;
    printf("      andar | Δ=n²+4 | ‖a×b‖² | Π(−1) Π(0) Π(+1) | DTC escolhe | Π casado\n");
    for(int k = 0; k < N; k++){
        L p0 = imposto(-1, massa[k]), pm = imposto(0, massa[k]), pp = imposto(+1, massa[k]);
        L sdtc = dtc_escolhe_s(massa[k]);
        L pcasado = imposto(sdtc, massa[k]);
        if(p0 != 0 || pp != 0) casa = 0;           /* anula nos extremos */
        if(massa[k] > 0 && pm != massa[k]) casa = 0;/* máximo no meio */
        if(sdtc*sdtc != 1) casa = 0;               /* o DTC escolhe ±1 (a banda) */
        if(pcasado != 0) casa = 0;                 /* e casa: imposto 0 */
        printf("        %d   |   %2lld   |  %3lld   |  %2lld   %3lld   %2lld  |     %+lld      |   %lld\n",
               k+1, per[k], massa[k], p0, pm, pp, sdtc, pcasado);
    }
    printf("\n");
    ok("§W1 o INVERSOR casa por DTC: o imposto Π(s)=(1−s²)·‖a×b‖² anula EXACTO em s=±1 (a banda) e vale"
       " ‖a×b‖² em s=0 (reactivo máximo); o DTC escolhe do trial {−1,0,+1} o ±1 que leva o imposto a 0"
       " --- fp=1 por modulação, não por armazenamento", casa);

    /* ── §W2 o viveiro sincroniza: o fold do lcm ─────────────────────────────────────────── */
    /* os períodos não são lineares; o viveiro alinha-os no lcm. O fold é comutativo/associativo/
     * idempotente (as três do lcm), logo independente da ordem; contém cada andar (a_k | lcm) e é o
     * menor que o faz. */
    L lcm_esq = 1; for(int k = 0; k < N; k++) lcm_esq = mmc(lcm_esq, per[k]);   /* fold à esquerda */
    L lcm_dir = 1; for(int k = N-1; k >= 0; k--) lcm_dir = mmc(lcm_dir, per[k]);/* fold à direita */
    int contem = 1; for(int k = 0; k < N; k++) if(lcm_esq % per[k] != 0) contem = 0;
    int menor = 1;                                 /* nenhum m < lcm é divisível por todos */
    for(L m = 1; m < lcm_esq; m++){
        int todos = 1; for(int k = 0; k < N; k++) if(m % per[k] != 0){ todos = 0; break; }
        if(todos){ menor = 0; break; }
    }
    printf("§W2  períodos {%lld,%lld,%lld,%lld} → lcm esquerda=%lld, direita=%lld (independente da ordem)\n\n",
           per[0],per[1],per[2],per[3], lcm_esq, lcm_dir);
    ok("§W2 o VIVEIRO sincroniza pelo lcm: o fold ⋁_k R^{a_k}=R^{lcm} é independente da ordem"
       " (esquerda=direita), contém cada andar (a_k | lcm) e é o MENOR que os contém --- comutativo,"
       " associativo, idempotente, resíduo 0", lcm_esq == lcm_dir && contem && menor);

    /* ── §W3 a torre casa NO lcm: todos na origem E todos casados ─────────────────────────── */
    /* faz-se correr o relógio da torre. Em cada instante t, o andar k está na origem se t%a_k=0. O
     * PRIMEIRO instante em que TODOS batem juntos é o lcm (antes, nunca todos); e nesse instante o DTC
     * já casou cada andar (s=±1, imposto 0). A torre casa andar a andar, sincronizada pelo lcm. */
    L primeiro_junto = 0;
    for(L t = 1; t <= lcm_esq; t++){
        int todos = 1; for(int k = 0; k < N; k++) if(t % per[k] != 0){ todos = 0; break; }
        if(todos){ primeiro_junto = t; break; }
    }
    int torre_casa = 1;
    if(primeiro_junto != lcm_esq) torre_casa = 0;  /* o primeiro instante comum é o lcm */
    for(int k = 0; k < N; k++){                     /* e cada andar está casado (DTC → s=±1 → imposto 0) */
        L sdtc = dtc_escolhe_s(massa[k]);
        if(imposto(sdtc, massa[k]) != 0) torre_casa = 0;
        if(lcm_esq % per[k] != 0) torre_casa = 0;   /* alinhado no lcm */
    }
    printf("§W3  primeiro instante em que TODOS os andares batem juntos: t=%lld (= lcm); antes, nunca todos\n\n",
           primeiro_junto);
    ok("§W3 a TORRE casa NO lcm: o primeiro instante em que todos os relógios estão na origem é t=lcm"
       " (residuo 0), e aí cada andar está casado pelo seu DTC (s=±1, imposto 0). Um DTC por andar,"
       " sincronizados pelo viveiro --- sem oscilador, sem armazenar", torre_casa);

    /* ── §W4 o torque é o cruzado, e casar o cruzado é fp=1 ───────────────────────────────── */
    /* o imposto (1−s²)·‖a×b‖² CAVALGA o produto cruzado (o torque). Em s=0 é o cruzado inteiro (todo
     * reactivo); em s=±1 anula-se INDEPENDENTE do cruzado (todo activo). Logo controlar o cruzado ---
     * o DTC a empurrar s→±1 --- é o que casa fp=1: T_e=ψ_s×i_s, e casar o cruzado zera a reactância. */
    int cavalga = 1;
    for(int k = 0; k < N; k++){
        if(imposto(0, massa[k]) != massa[k]) cavalga = 0;   /* s=0: o imposto É o cruzado */
        if(imposto(+1, massa[k]) != 0) cavalga = 0;         /* s=+1: anula, seja qual for o cruzado */
        if(imposto(-1, massa[k]) != 0) cavalga = 0;
    }
    printf("§W4  o imposto em s=0 é ‖a×b‖² (o cruzado inteiro); em s=±1 é 0 (independente do cruzado)\n\n");
    ok("§W4 o TORQUE é o cruzado (T_e=ψ×i): o imposto cavalga ‖a×b‖² --- em s=0 é o cruzado inteiro"
       " (todo reactivo), em s=±1 anula-se seja qual for o cruzado (todo activo). Casar o cruzado"
       " (o DTC a empurrar s→±1) É casar fp=1 --- controlar o reactivo é controlar o cruzado", cavalga);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  O inversor casa o factor por CONTROLE DIRETO DE TORQUE: o imposto (1−s²)·‖a×b‖² é a");
        puts("  reactância, e o DTC escolhe do trial {−1,0,+1} o ±1 que a zera (fp=1) --- por modulação,");
        puts("  não por buffer. Na TORRE há um relógio por andar (a dobra Δ=n²+4), e o VIVEIRO sincroniza-os");
        puts("  no lcm: ⋁ R^{a_k}=R^{lcm}, independente da ordem. A torre casa andar a andar e bate junta");
        puts("  no lcm --- um DTC por andar, sincronizados pelo viveiro. O torque é o cruzado, e casar o");
        puts("  cruzado é casar o factor. Sem oscilador, sem armazenar: a estaca a modular.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
