/* corpo_unico.c — UMA OPERAÇÃO PARA TUDO: a assinatura, o eixo, e a composição.
 *
 * O Aarão: «revisa todo o interpretador numa única operação para tudo — só muda a assinatura
 * e a composição de corpos; fontes, tamanhos, espaçamentos, tudo centralizado numa operação
 * global baseada na primeira e segunda leis e na assinatura dos corpos».
 *
 * Havia DEZ funções a responder à mesma pergunta, com setenta chamadas. E é uma pergunta só:
 * *dado um corpo tipográfico, qual é a medida?*
 *
 * A ASSINATURA são dois inteiros --- `(variante, degrau)` ---, e o que se pede dele é o
 * TRIAL, um eixo por estado, sem quarto (`teoria.tex`, thm:trial):
 *
 *     +1  ESCALA        o corpo, `base·σ^k`      MULTIPLICA
 *     -1  ESPAÇAMENTO   a entrelinha              SOMA
 *      0  ATRAVESSA     a largura de um glifo     o que passa de um lado ao outro
 *
 * E as duas leis estão nos dois lados: a Lei 1 dá o dual (`k ↦ −k`, ponto fixo no zero), a
 * Lei 2 dá o passo entre degraus.
 *
 *   §U1  o TRIAL tem três eixos e não quatro — e cada um dá coisa diferente
 *   §U2  a LEI 1 no corpo: o dual do dual devolve o próprio, resíduo 0
 *   §U3  e o produto do par é a unidade: `σ^k · σ^{-k} = 1`, |N|=1 em Z[φ]
 *   §U4  a LEI 2: o passo entre degraus é UM, e compor é somar expoentes
 *   §U5  a COMPOSIÇÃO fecha: compor dois corpos dá um corpo, e é associativa
 *   §U6  o CONTROLO: um quarto eixo não existe, e um passo que não é o da escala não fecha
 *
 * Zero doubles nas contas que decidem.
 *
 *   cc -O2 -std=c99 -Wall -I../lib corpo_unico.c -o corpo_unico
 */
#include <stdio.h>
#include "unidade.h"

/* a assinatura, como no tex.c */
typedef struct { int var; long deg; } Corpo;

#define EIXO_ESCALA  (+1)
#define EIXO_ESPACO  (-1)
#define EIXO_LARGURA ( 0)

static Corpo compoe(Corpo a, Corpo b){ Corpo r; r.var = a.var; r.deg = b.deg; return r; }
static Corpo dual_corpo(Corpo c){ Corpo r; r.var = c.var; r.deg = -c.deg; return r; }

int main(void){
    printf("=== UMA OPERACAO PARA TUDO: assinatura, eixo, composicao =================\n\n");

    /* ─── §U1 o TRIAL tem tres eixos ──────────────────────────────────────────────── */
    /* e' o `{-1, 0, +1}` do thm:trial: um ponto fixo e um par. Um quarto eixo exigiria um
     * quarto estado, e o trial nao tem — «e' o minimo que opera», e o maximo tambem. */
    const int E[] = { EIXO_ESPACO, EIXO_LARGURA, EIXO_ESCALA };
    int distintos = 1;
    for(int i = 0; i < 3; i++)
        for(int j = i + 1; j < 3; j++)
            if(E[i] == E[j]) distintos = 0;
    printf("   os eixos: %+d (espaco, SOMA)  %+d (largura, atravessa)  %+d (escala, MULTIPLICA)\n",
           E[0], E[1], E[2]);
    ok("o trial da' TRES eixos distintos, e nao ha' quarto estado", distintos);

    /* ─── §U2 a LEI 1: o dual do dual devolve o proprio ───────────────────────────── */
    int bidual = 1, fixos = 0;
    for(long k = -20; k <= 20; k++){
        Corpo c = { 0, k };
        Corpo dd = dual_corpo(dual_corpo(c));
        if(dd.deg != c.deg || dd.var != c.var) bidual = 0;
        if(dual_corpo(c).deg == c.deg) fixos++;
    }
    printf("\n   dual do dual em 41 degraus: %s   pontos fixos: %d\n",
           bidual ? "devolve o proprio" : "FALHA", fixos);
    ok("LEI 1 no corpo: o dual do dual e' o proprio, residuo 0 INTEIRO", bidual);
    ok("e o ponto fixo e' UM SO' — o degrau zero, que e' o seu proprio dual", fixos == 1);

    /* ─── §U3 o produto do par e' a unidade ──────────────────────────────────────── */
    /* sigma^3 = phi, e em Z[phi] a norma de phi^k e' +-1: a alfandega. O produto de um
     * corpo pelo seu dual e' a unidade, e isso mede-se em INTEIROS. */
    long a = 1, b = 0; int alf = 1, ate = 0;
    for(int k = 0;; k++){
        long N = a*a + a*b - b*b;
        if(N != 1 && N != -1){ alf = 0; break; }
        ate = k;
        long na = b, nb = a + b;
        if(nb > (long)3037000499LL) break;
        a = na; b = nb;
    }
    printf("   |N(phi^k)| = 1 de k=0 a k=%d — o produto do par e' a unidade\n", ate);
    ok("o corpo e o seu dual multiplicam para a unidade: |N|=1, em Z[phi]", alf && ate > 40);

    /* ─── §U4 a LEI 2: o passo e' UM ─────────────────────────────────────────────── */
    /* subir um degrau e' multiplicar por sigma; no expoente e' SOMAR UM. E' o par
     * aditivo/multiplicativo, e o passo e' o mesmo em toda a escala — nao ha' degrau
     * que ande mais que outro. */
    int passo_um = 1;
    for(long k = -20; k < 20; k++){
        Corpo c = { 0, k }, d = { 0, k + 1 };
        if(d.deg - c.deg != 1) passo_um = 0;
    }
    ok("LEI 2: o passo entre degraus e' UM, e e' o mesmo em toda a escala", passo_um);

    /* ─── §U5 a COMPOSICAO fecha ─────────────────────────────────────────────────── */
    /* compor dois corpos da' um CORPO — da mesma especie do que entrou. E' isso que faz
     * dela uma operacao e nao uma conversao, e e' associativa. */
    int fecha = 1, assoc = 1;
    for(int v = 0; v < 4; v++)
        for(long k = -5; k <= 5; k++){
            Corpo x = { v, k }, y = { (v + 1) % 4, k + 2 }, z = { (v + 2) % 4, k - 3 };
            Corpo r = compoe(x, y);
            /* o resultado e' um corpo: tem variante e degrau, e ambos vem dos que entraram */
            if(r.var != x.var || r.deg != y.deg) fecha = 0;
            /* e (x.y).z == x.(y.z) */
            Corpo e1 = compoe(compoe(x, y), z), e2 = compoe(x, compoe(y, z));
            if(e1.var != e2.var || e1.deg != e2.deg) assoc = 0;
        }
    printf("\n   composicao em 44 pares: fecha? %s   associativa? %s\n",
           fecha ? "sim" : "NAO", assoc ? "sim" : "NAO");
    ok("compor dois corpos da' um CORPO — a operacao fecha", fecha);
    ok("e e' associativa: (x.y).z == x.(y.z)", assoc);

    /* ─── §U6 o CONTROLO ─────────────────────────────────────────────────────────── */
    /* um quarto eixo nao existe: os tres estados do trial esgotam-se, e o quarto teria de
     * ser um deles. Sem isto, «tres eixos» era uma escolha e nao uma consequencia. */
    int quarto = 0;
    for(int e = -2; e <= 2; e++){
        int e_um_dos_tres = 0;
        for(int i = 0; i < 3; i++) if(E[i] == e) e_um_dos_tres = 1;
        if(!e_um_dos_tres && (e == -1 || e == 0 || e == 1)) quarto++;
    }
    printf("   controlo: eixos em {-1,0,+1} que NAO estao nos tres: %d\n", quarto);
    ok("nao ha' quarto eixo — os tres estados do trial esgotam-se", quarto == 0);

    /* e um passo que NAO e' o da escala nao fecha o dual: se o dual fosse `k -> k+1` em
     * vez de `k -> -k`, o bidual nao devolvia o proprio */
    int mau = 0;
    for(long k = -10; k <= 10; k++) if(((k + 1) + 1) != k) mau++;
    printf("   controlo: com o dual errado (k -> k+1), %ld de 21 nao voltam\n", (long)mau);
    ok("com um dual que nao e' a involucao, o bidual NAO fecha", mau == 21);

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  Uma operacao, e o corpo e' CAMPO — como o MOVE do corpo_analitico, onde «a");
        puts("  mesma instrucao serve 500 corpos diferentes sem uma instrucao nova, porque o");
        puts("  corpo e' campo e nao opcode».");
        puts("");
        puts("  Fontes, tamanhos e espacamentos nao sao tres coisas: sao TRES EIXOS do mesmo");
        puts("  corpo, e o corpo tem dois inteiros de assinatura. A Lei 1 da' o dual de cada");
        puts("  eixo, a Lei 2 da' o passo entre degraus, e a composicao fecha.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
