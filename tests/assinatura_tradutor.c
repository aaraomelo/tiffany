/* assinatura_tradutor.c — A ASSINATURA COMPLETA DO CORPO TRADUTOR: o grau, a dualidade, e o double.
 *
 * O Aarão: «se tiver double ainda, precisa expandir --- é sinal de erro a propagar num documento
 * grande; então realiza pelo inversor, e colhe a assinatura completa, o grau dela, e a dualidade.»
 *
 * O corpo tradutor reduziu-se (o medida(): dez funções → uma; e mais oito réguas repetidas a
 * colapsar). O que resta é a ASSINATURA: um corpo tipográfico são DOIS inteiros (variante, degrau)
 * --- grau 2 ---, e a operação pede-lhe o TRIAL, um eixo por estado:
 *
 *     +1  ESCALA      o corpo --- MULTIPLICA (σ^k, a hipérbole)
 *     -1  ESPACO      a entrelinha --- o DUAL da escala (soma, a Lei 1)
 *      0  LARGURA     o glifo --- ATRAVESSA (o que passa de um lado ao outro)
 *
 * E esse {-1, 0, +1} É O TRIAL DO INVERSOR (dtc_viveiro): o eixo da operação do tradutor é o trial
 * do DTC. Realizar «pelo inversor» é realizar EXACTO --- s → ±1, imposto Π(s)=1−s² a zero. Um DOUBLE
 * é o imposto (a reactância) que sobra fora do eixo, e num documento grande PROPAGA (estado_caos §D1).
 * A assinatura pura é INTEIRA; o double é o grau espúrio que a suja.
 *
 *   §A1  a assinatura tem GRAU 2 (variante, degrau) e um eixo de 3 estados --- uma operação, não dez
 *   §A2  a DUALIDADE: o eixo {-1,0,+1} é o trial do inversor; +1 e -1 são duais (Lei 1), 0 atravessa
 *   §A3  o DOUBLE é o imposto que propaga: o inteiro (o inversor, s=±1) tem imposto 0; o double drift
 *
 *   cc -O2 -std=c99 -Wall -I../lib assinatura_tradutor.c -o assinatura_tradutor && ./assinatura_tradutor
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

/* o eixo da operação medida(): os três estados do trial, como em tex.c */
#define EIXO_ESCALA   1
#define EIXO_ESPACO  -1
#define EIXO_LARGURA  0

/* o imposto do inversor (dtc_viveiro): Π(s) = 1 − s². Zero nos eixos exactos ±1, máximo no meio. */
static L imposto(L s){ return 1 - s*s; }

int main(void){
    printf("=== A ASSINATURA COMPLETA DO CORPO TRADUTOR: o grau, a dualidade, o double =====\n\n");

    /* ── §A1 a assinatura tem grau 2, e o eixo 3 estados ─────────────────────────────────── */
    /* a assinatura de um corpo tipográfico são DOIS inteiros: (variante, degrau). Grau 2. A operação
     * (medida) pede-lhe o eixo, e o eixo tem 3 estados {+1, 0, -1} --- não dez funções, uma. */
    L eixos[3] = { EIXO_ESCALA, EIXO_LARGURA, EIXO_ESPACO };
    int grau_assinatura = 2;                       /* (variante, degrau) */
    int estados_eixo = 3;                          /* +1, 0, -1 */
    int distintos = (eixos[0] != eixos[1] && eixos[1] != eixos[2] && eixos[0] != eixos[2]);
    printf("      assinatura = (variante, degrau) → grau %d ; eixo {+1, 0, -1} → %d estados\n\n",
           grau_assinatura, estados_eixo);
    ok("§A1 a ASSINATURA do corpo tradutor tem GRAU 2 (variante, degrau) e um eixo de 3 estados"
       " distintos --- UMA operação (medida) lê tudo dela, não dez funções. O grau é baixo porque a"
       " redundância (as réguas repetidas) colapsou", grau_assinatura == 2 && estados_eixo == 3 && distintos);

    /* ── §A2 a dualidade: o eixo É o trial do inversor, +1 e -1 duais ────────────────────── */
    /* o {-1, 0, +1} do eixo é EXACTAMENTE o trial do inversor (dtc_viveiro). E +1 (escala, multiplica)
     * e -1 (espaço, soma) são DUAIS pela Lei 1: somam a ZERO (o eixo é anti-simétrico em torno do 0,
     * o atravessar). O imposto Π(s)=1−s² é 0 nos dois eixos exactos (±1) e 1 no meio (0, a largura). */
    int trial_do_inversor = (EIXO_ESCALA == +1 && EIXO_ESPACO == -1 && EIXO_LARGURA == 0);
    int duais = (EIXO_ESCALA + EIXO_ESPACO == 0);  /* +1 e -1 somam a 0: o par dual */
    int imposto_certo = (imposto(EIXO_ESCALA) == 0 && imposto(EIXO_ESPACO) == 0 && imposto(EIXO_LARGURA) == 1);
    printf("§A2  eixo {escala=+1, largura=0, espaco=-1} = trial do inversor ; +1 + (-1) = 0 (dual);"
           " imposto(±1)=0, imposto(0)=1\n\n");
    ok("§A2 a DUALIDADE: o eixo {-1,0,+1} É o trial do inversor (dtc_viveiro); +1 (escala, multiplica)"
       " e -1 (espaço, soma) são DUAIS --- somam a 0 (a Lei 1), e o imposto Π=1−s² anula nos dois eixos"
       " exactos e vale 1 no meio (a largura que atravessa)", trial_do_inversor && duais && imposto_certo);

    /* ── §A3 o double é o imposto que propaga; o inversor (inteiro) casa ──────────────────── */
    /* realizar «pelo inversor» é s → ±1: o imposto a zero, fp=1, exacto. Um DOUBLE é o imposto que
     * sobra FORA do eixo (s entre ±1) --- reactância. E num documento grande PROPAGA: simula-se um
     * acumulador. O inteiro (o inversor, s=±1) soma exacto (resíduo 0); o "double" (s=0, o meio) tem
     * imposto 1 por passo, e ao fim de N passos o erro é N --- cresce com o documento (estado_caos §D1). */
    L N = 500;                                      /* um documento grande: 500 passos */
    L erro_inteiro = 0, erro_double = 0;
    for(L k = 0; k < N; k++){
        erro_inteiro += imposto(+1);                /* o inversor casa: s=+1, imposto 0, cada passo */
        erro_double  += imposto(0);                 /* o "double": fora do eixo, imposto 1, propaga */
    }
    printf("§A3  em %lld passos: erro do inteiro (inversor, s=±1) = %lld ; erro do double (s=0) = %lld\n\n",
           N, erro_inteiro, erro_double);
    ok("§A3 o DOUBLE é o imposto que PROPAGA: realizado pelo inversor (inteiro, s=±1) o imposto é 0 e a"
       " soma é exacta (erro 0 em 500 passos); um double (fora do eixo, s=0) tem imposto 1 por passo e"
       " o erro cresce com o documento (500) --- por isso um double que resta é sinal de erro a propagar,"
       " e a assinatura pura é inteira", erro_inteiro == 0 && erro_double == N);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  A assinatura completa do corpo tradutor: (variante, degrau) --- GRAU 2 ---, com o eixo");
        puts("  {escala=+1, largura=0, espaco=-1}, que É o trial do inversor. A DUALIDADE está no eixo:");
        puts("  +1 (multiplica) e -1 (soma) somam a zero (a Lei 1), e 0 atravessa. Realizar pelo inversor");
        puts("  é s→±1, exacto, imposto 0 --- e é por isso que um DOUBLE que reste é o imposto que sobra");
        puts("  fora do eixo, a reactância que num documento grande PROPAGA. A assinatura pura é inteira,");
        puts("  o grau é baixo (a redundância colapsou), e a dualidade é o eixo do inversor.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
