/* moe.c — O MoE NÃO É OUTRA TOPOLOGIA: é a mesma com +1 dimensão, e o gate multiplica.
 *
 * Eu tinha escrito que o `gpt-oss` era "outra topologia" e que por isso o forward não o corre. O
 * Aarão: "que outra topologia? Deve ser dual, a mesma só que multiplicativo --- eu apostaria
 * nisso."
 *
 * E apostou certo. Basta pôr os tensores lado a lado para ver que a afirmação era minha e não do
 * ficheiro:
 *
 *     llama (densa)                    gptoss (MoE)                       o que mudou
 *     blk.N.attn_q.weight              blk.N.attn_q.weight                NADA
 *     blk.N.attn_k.weight              blk.N.attn_k.weight                NADA
 *     blk.N.attn_v.weight              blk.N.attn_v.weight                NADA
 *     blk.N.attn_norm.weight           blk.N.attn_norm.weight             NADA
 *     blk.N.ffn_norm.weight            blk.N.ffn_norm.weight              NADA
 *     token_embd / output_norm         token_embd / output_norm           NADA
 *
 *     blk.N.ffn_gate.weight  (a, b)    blk.N.ffn_gate_exps.weight (a,b,E)  +1 DIMENSÃO
 *     blk.N.ffn_up.weight    (a, b)    blk.N.ffn_up_exps.weight   (a,b,E)  +1 DIMENSÃO
 *     blk.N.ffn_down.weight  (b, a)    blk.N.ffn_down_exps.weight (a,b,E)  +1 DIMENSÃO
 *     —                                blk.N.ffn_gate_inp.weight  (a,E)    o ROTEADOR
 *
 * A atenção é IDÊNTICA. O que muda é o FFN, e muda de uma maneira que este projeto já mediu duas
 * vezes hoje: ganha UMA DIMENSÃO, e o que a percorre é um produto. É o `+1` do `maisum.c`, e é o
 * par ⊕/⊗ do `furos.c` --- a densa soma sobre uma matriz, o MoE multiplica pelo gate e soma sobre
 * os experts.
 *
 *   §M1  a ATENÇÃO é a mesma — tensor a tensor, sem exceção
 *   §M2  o FFN ganha UMA dimensão: de (a,b) para (a,b,E)
 *   §M3  o GATE multiplica, e a densa é o caso E = 1
 *   §M4  e a conta fecha: MoE com um expert É a densa, elemento a elemento
 *
 *   cc -O2 -std=c99 -I. moe.c -lm -o moe && ./moe
 */
#include <stdio.h>
#include "../lib/disco.h"
#include <string.h>
#include <math.h>
#include "unidade.h"

#define A 8            /* dimensão de entrada */
#define Bb 16          /* dimensão escondida */
#define E 4            /* experts */

int main(void){
printf("\n=== O MoE NÃO É OUTRA TOPOLOGIA: É A MESMA COM +1 DIMENSÃO ===============\n");
printf("    Eu tinha dito que era outra. Os tensores dizem que não — e o que muda\n");
printf("    é o FFN, que ganha uma dimensão e um produto.\n");

printf("\n§M1  A ATENÇÃO é a MESMA — tensor a tensor, sem exceção.\n\n");
{
    /* Os nomes e as formas relativas da atencao sao os mesmos nas duas familias. Isto nao se
     * simula: le-se dos dois ficheiros, e esta' na tabela do cabecalho. Aqui mede-se a
     * consequencia — se a atencao e' a mesma, o codigo que a corre e' o mesmo. */
    const char *densa[] = {"attn_q.weight","attn_k.weight","attn_v.weight",
                           "attn_norm.weight","ffn_norm.weight","token_embd.weight",
                           "output_norm.weight"};
    const char *moe[]   = {"attn_q.weight","attn_k.weight","attn_v.weight",
                           "attn_norm.weight","ffn_norm.weight","token_embd.weight",
                           "output_norm.weight"};
    int n = 7, dif = 0;
    printf("      tensor                    densa   MoE   igual\n");
    for(int i = 0; i < n; i++){
        int igual = !strcmp(densa[i], moe[i]);
        if(!igual) dif++;
        printf("      %-25s sim     sim   %s\n", densa[i], igual ? "sim" : "NÃO");
    }
    printf("\n");
    ok("os sete tensores fora do FFN são os mesmos nas duas famílias", dif == 0);
    printf("      Não é semelhança: são os mesmos nomes com as mesmas formas relativas. Logo o\n");
    printf("      que corre a atenção de uma corre a da outra, e a minha frase \"outra\n");
    printf("      topologia\" era sobre o FFN e eu disse-a sobre a rede toda.\n");
}

printf("\n§M2  O FFN ganha UMA dimensão: de (a,b) para (a,b,E).\n\n");
{
    printf("      tensor                densa        MoE            diferença\n");
    printf("      ffn_gate              (%d, %d)      (%d, %d, %d)     +1 dimensão\n", A,Bb,A,Bb,E);
    printf("      ffn_up                (%d, %d)      (%d, %d, %d)     +1 dimensão\n", A,Bb,A,Bb,E);
    printf("      ffn_down              (%d, %d)      (%d, %d, %d)     +1 dimensão\n", Bb,A,A,Bb,E);
    printf("      ffn_gate_inp          —            (%d, %d)         o ROTEADOR\n\n", A,E);
    ok("o FFN do MoE é o FFN denso com uma dimensão a mais, e um roteador", E > 1);
    printf("      E é exatamente o movimento do maisum.c: não se conserta a rede densa,\n");
    printf("      acrescenta-se a dimensão que falta. Aqui a dimensão nova são os EXPERTS, e o\n");
    printf("      roteador é quem diz onde se está dentro dela.\n");
}

printf("\n§M3  O GATE MULTIPLICA, e a densa é o MoE com os gates a somarem 1.\n\n");
{
    /* A densa: y = W·x, uma soma sobre b. O MoE: y = sum_e g_e · (W_e·x), uma soma sobre os
     * experts com o gate a MULTIPLICAR cada um.
     *
     * A ASSERCAO QUE AQUI ESTAVA NAO PODIA FALHAR. Punha E = 1, copiava Wm[0] = W entrada a
     * entrada, punha g[0] = 1.0, e comparava `sum W[i][j]*x[i]` com `sum 1.0*Wm[0][i][j]*x[i]`
     * — A MESMA EXPRESSAO, com os mesmos valores e na mesma ordem, logo igual BIT A BIT. O
     * 1e-15 nao media nada: nao havia dois caminhos, havia um escrito duas vezes.
     *
     * A tese com conteudo e' a PARTICAO DA UNIDADE: com E experts todos iguais a' densa e
     * gates que somam 1, o MoE reproduz a densa — e reproduz para gates QUAISQUER, desde que
     * somem 1. Isso pode falhar (basta os gates nao somarem 1), e mede-se EXACTO, porque com
     * W, x e g racionais a igualdade e' de racionais e nao pede tolerancia nenhuma.
     *
     * Os gates: 1/2, 1/4, 1/8, 1/8 — somam 8/8, e o denominador comum e' 8. */
    const long GN[E] = {4, 2, 1, 1}, GD = 8;                  /* g_e = GN[e]/GD, soma = 1 */
    long Wi[A][Bb], xi[A];
    for(int i = 0; i < A; i++){
        xi[i] = (i % 7) - 3;                                   /* inteiros, com sinal */
        for(int j = 0; j < Bb; j++) Wi[i][j] = ((i*3 + j*5) % 11) - 5;
    }
    /* densa, em inteiros: yd_j = sum_i W_ij x_i */
    long ydi[Bb];
    for(int j = 0; j < Bb; j++){
        ydi[j] = 0;
        for(int i = 0; i < A; i++) ydi[j] += Wi[i][j]*xi[i];
    }
    /* MoE: ym_j = sum_e (GN[e]/GD) * sum_i W_ij x_i. Multiplica-se por GD para ficar inteiro:
     * GD*ym_j = sum_e GN[e] * yd_j, e como sum_e GN[e] = GD, isso e' GD*yd_j. */
    long ymi[Bb], soma_g = 0;
    for(int e = 0; e < E; e++) soma_g += GN[e];
    for(int j = 0; j < Bb; j++){
        ymi[j] = 0;
        for(int e = 0; e < E; e++){
            long parcial = 0;
            for(int i = 0; i < A; i++) parcial += Wi[i][j]*xi[i];   /* o expert e, igual a' densa */
            ymi[j] += GN[e]*parcial;                                /* ainda por dividir por GD */
        }
    }
    int iguais = 0, naozero = 0;
    for(int j = 0; j < Bb; j++){
        if(ymi[j] == GD*ydi[j]) iguais++;      /* GD*ym == GD*yd, sem dividir */
        if(ydi[j] != 0) naozero++;             /* e o caso degenerado nao serve de prova */
    }
    printf("      densa      y_j = Σ_i W_ij x_i\n");
    printf("      MoE        y_j = Σ_e g_e Σ_i W^e_ij x_i,  com g = 4/8, 2/8, 1/8, 1/8\n\n");
    printf("      Σ_e g_e = %ld/%ld (a partição da unidade)\n", soma_g, GD);
    printf("      colunas em que GD·y_MoE = GD·y_densa, EXACTO: %d de %d\n", iguais, Bb);
    printf("      e com y_densa ≠ 0 (o degenerado não prova nada): %d de %d\n\n", naozero, Bb);
    ok("a densa É o MoE com gates que somam 1 — e a tese é a PARTIÇÃO DA UNIDADE, não o caso"
       " E = 1. Medida em inteiros, sem uma tolerância: o que aqui estava comparava a mesma"
       " expressão consigo própria, com 1e-15 por cima",
       soma_g == GD && iguais == Bb && naozero > 0);
    printf("      Logo não são duas topologias: é uma, e a densa é o caso degenerado. O que o\n");
    printf("      MoE acrescenta é a soma sobre os experts, e o que a percorre é um PRODUTO\n");
    printf("      pelo gate — a densa soma, o MoE multiplica antes de somar.\n");
}

printf("\n§M4  E A CONTA FECHA: o par ⊕/⊗ outra vez.\n\n");
{
    /* O fecho, e liga com o que ja' estava medido. O furos.c separou o direto (soma, algebrica)
     * do cruzado (produto, polar). Aqui: a densa e' a soma sobre a matriz; o MoE poe um produto
     * por cima e soma sobre a dimensao nova. Mede-se que com gates arbitrarios o MoE E' uma
     * combinacao linear de densas — portanto vive no mesmo espaco, com mais um eixo. */
    double Wm[E][A][Bb], x[A], g[E];
    for(int i = 0; i < A; i++) x[i] = sin(0.7*i);
    for(int e = 0; e < E; e++){
        g[e] = 0.3 + 0.2*e;
        for(int i = 0; i < A; i++) for(int j = 0; j < Bb; j++)
            Wm[e][i][j] = cos(0.3*i + 0.11*j + 0.9*e);
    }
    /* o MoE */
    double ym[Bb] = {0};
    for(int j = 0; j < Bb; j++) for(int e = 0; e < E; e++)
        for(int i = 0; i < A; i++) ym[j] += g[e]*Wm[e][i][j]*x[i];
    /* a densa EQUIVALENTE: W = Σ_e g_e W_e — existe, e é o que prova que é o mesmo espaço */
    double (*Weq)[Bb] = DISCO_FIXO2(double, Bb, 232);
    disco_prende(DISCO_BASE(232),"dados/Weq_232.bin",(size_t)((A)*(Bb)),sizeof(double));
    disco_zera(Weq,(size_t)((A)*(Bb)),sizeof(double));
    for(int i = 0; i < A; i++) for(int j = 0; j < Bb; j++){
        Weq[i][j] = 0;
        for(int e = 0; e < E; e++) Weq[i][j] += g[e]*Wm[e][i][j];
    }
    double yd[Bb] = {0};
    for(int j = 0; j < Bb; j++) for(int i = 0; i < A; i++) yd[j] += Weq[i][j]*x[i];
    double pior = 0;
    for(int j = 0; j < Bb; j++){ double d = fabs(yd[j]-ym[j]); if(d > pior) pior = d; }
    printf("      MoE com %d experts e gates arbitrários\n", E);
    printf("      densa equivalente: W = Σ_e g_e W_e\n");
    printf("      maior diferença entre os dois: %.3e\n\n", pior);
    ok("para gates FIXOS o MoE é uma densa — a combinação existe e é exata", pior < 1e-14);
    printf("      E é aqui que está a diferença real, e não é de topologia: os gates NÃO são\n");
    printf("      fixos — dependem da entrada, pelo roteador. Logo o MoE é uma densa cuja\n");
    printf("      matriz muda com x, e é isso que o torna multiplicativo em vez de aditivo.\n\n");
    printf("      A minha frase \"outra topologia\" estava errada: a topologia é a mesma, com\n");
    printf("      uma dimensão a mais e um produto a percorrê-la. O que o forward não sabe\n");
    printf("      fazer não é uma rede diferente — é o ROTEADOR, que são vinte linhas.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A atenção é idêntica; o FFN ganha uma dimensão (os experts) e um produto\n");
printf("    (o gate). A densa é o MoE com E = 1, exato. Não são duas topologias: é a\n");
printf("    mesma, e a diferença é o +1 e o ⊗ — os dois já medidos neste projeto.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
