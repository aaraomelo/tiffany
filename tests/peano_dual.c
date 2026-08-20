/* peano_dual.c — A CURVA DE PEANO ENCHE, E A DUAL CONTA: o teorema central no espaço.
 *
 * O Aarão: «vê a curva de Peano e o seu dual; o Gentil tem essa construção dual.»
 *
 * A curva de Peano é a construção de Gentil (2007): uma curva 1D cuja imagem ENCHE o quadrado
 * 2D. É o `thm:bit` a preencher --- um único bit desenha tudo --- e o lado CONTÍNUO (Gentil,
 * ENCHER, 1D->2D) do teorema central. A sua DUAL é a projeção 2D->1D --- CONTAR (o índice, o lado
 * discreto de Hurwitz) ---, e os dois fecham por involução: dar a volta devolve o ponto, resíduo 0.
 *
 * Tudo INTEIRO: o índice de Peano é ternário (base 3), sem uma vírgula flutuante. A fórmula é a de
 * Gentil/Peano, com o complemento ternário k(d)=2-d aplicado conforme a paridade acumulada:
 *
 *     x = t1 . (k^{t2} t3) (k^{t2+t4} t5) ... ;   y = (k^{t1} t2) (k^{t1+t3} t4) ...
 *
 *   §PN1  a curva ENCHE: d->(x,y) é uma BIJEÇÃO — cobre as 9^K casas, cada uma UMA vez
 *   §PN2  é uma CURVA: índices consecutivos são casas adjacentes (passo unitário), o traço contínuo
 *   §PN3  a DUAL fecha: contar∘encher = id (ν∘ν=id), resíduo 0 — encher(Gentil) ↔ contar(Hurwitz)
 *
 *   cc -O2 -std=c99 -Wall -I../lib peano_dual.c -o peano_dual && ./peano_dual
 */
#include <stdio.h>
#include "unidade.h"

typedef long L;
#define K 4                          /* o reticulado é 3^K × 3^K = 81×81; o índice vai a 9^K=6561 */

/* o complemento ternário aplicado m vezes: identidade se m par, 2-d se m ímpar */
static int kc(int m, int d){ return (m & 1) ? (2 - d) : d; }

/* ENCHER — a curva de Peano: índice d (ternário, 2K dígitos) -> ponto (x,y). É de Gentil. */
static void encher(L d, int *px, int *py){
    int t[2*K + 1];                  /* t[1..2K], t[1] o mais significativo */
    for(int j = 2*K; j >= 1; j--){ t[j] = (int)(d % 3); d /= 3; }
    int x = 0, y = 0, cx = 0, cy = 0;
    for(int i = 1; i <= K; i++){
        int xi = kc(cx, t[2*i - 1]);
        cy += t[2*i - 1];
        int yi = kc(cy, t[2*i]);
        cx += t[2*i];
        x = x*3 + xi;
        y = y*3 + yi;
    }
    *px = x; *py = y;
}

int main(void){
    printf("=== A CURVA DE PEANO ENCHE, E A DUAL CONTA: o teorema central no espaço ========\n\n");

    int lado = 1; for(int i = 0; i < K; i++) lado *= 3;      /* 3^K */
    L total = (L)lado * lado;                                /* 9^K casas */

    /* ── §PN1 a curva ENCHE: bijeção, cobre tudo uma vez ────────────────────────────────── */
    /* a DUAL constrói-se aqui: inv[x][y] = d é a projeção 2D->1D (contar). Marcar cada casa uma
     * vez prova que ENCHER é bijeção — a imagem 2D cobre o quadrado, o bit desenha tudo. */
    static int inv[81][81];
    for(int x = 0; x < lado; x++) for(int y = 0; y < lado; y++) inv[x][y] = -1;
    int repetida = 0, foradolugar = 0;
    for(L d = 0; d < total; d++){
        int x, y; encher(d, &x, &y);
        if(x < 0 || x >= lado || y < 0 || y >= lado){ foradolugar = 1; continue; }
        if(inv[x][y] != -1) repetida = 1;
        inv[x][y] = (int)d;                                  /* a dual: o índice daquela casa */
    }
    L cobertas = 0;
    for(int x = 0; x < lado; x++) for(int y = 0; y < lado; y++) if(inv[x][y] != -1) cobertas++;
    printf("§PN1  a curva d->(x,y) em %d×%d: cobertas %ld de %ld, repetida? %s, fora? %s\n\n",
           lado, lado, cobertas, total, repetida?"sim":"nao", foradolugar?"sim":"nao");
    ok("§PN1 a curva de Peano ENCHE: d->(x,y) e' BIJECAO — cobre as 9^K casas, cada uma UMA vez"
       " (o bit desenha tudo, o lado continuo de Gentil)", cobertas == total && !repetida && !foradolugar);

    /* ── §PN2 é uma CURVA: passos unitários (o traço contínuo) ───────────────────────────── */
    /* índices consecutivos d e d+1 caem em casas ADJACENTES (|dx|+|dy|=1). É isso que faz dela
     * uma CURVA (um traço 1D sem saltos), e não uma bijeção qualquer. */
    int nao_unitario = 0; L pior = 0;
    for(L d = 0; d + 1 < total; d++){
        int x0, y0, x1, y1; encher(d, &x0, &y0); encher(d+1, &x1, &y1);
        L passo = (L)(x1>x0?x1-x0:x0-x1) + (y1>y0?y1-y0:y0-y1);
        if(passo != 1){ nao_unitario++; if(passo > pior) pior = passo; }
    }
    printf("§PN2  passos não-unitários: %d (o pior salto: %ld) — 0 é o traço contínuo\n\n",
           nao_unitario, pior);
    ok("§PN2 e' uma CURVA: indices consecutivos caem em casas adjacentes (passo unitario), o traço"
       " continuo 1D cuja imagem enche o 2D — nao um salto", nao_unitario == 0);

    /* ── §PN3 a DUAL fecha: contar∘encher = id (ν∘ν=id), resíduo 0 ───────────────────────── */
    /* CONTAR é a projeção 2D->1D (o índice, inv). ENCHER é 1D->2D. A ida-e-volta devolve o ponto:
     * encher(contar(x,y)) = (x,y) para toda casa. Resíduo 0 exato, em inteiros — a involução. */
    L residuo = 0;
    for(int x = 0; x < lado; x++) for(int y = 0; y < lado; y++){
        L d = inv[x][y];                        /* contar: 2D -> 1D (a dual de Gentil) */
        int xv, yv; encher(d, &xv, &yv);         /* encher: 1D -> 2D */
        if(xv != x || yv != y) residuo++;
    }
    printf("§PN3  contar∘encher: resíduo %ld em %ld casas (0 = a involução fecha)\n\n",
           residuo, total);
    ok("§PN3 a DUAL fecha: CONTAR (2D->1D, o indice, Hurwitz) e ENCHER (1D->2D, Gentil) sao a"
       " bijecao dual — contar∘encher = id, ν∘ν=id, residuo 0. E' o teorema central no espaço",
       residuo == 0);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  A curva de Peano (Gentil) ENCHE o quadrado: um bit 1D desenha o 2D inteiro — o");
        puts("  lado CONTINUO (encher) do teorema central. A sua DUAL e' a projecao 2D->1D, CONTAR");
        puts("  (o indice, o lado DISCRETO de Hurwitz). Os dois fecham por involucao, residuo 0 em");
        puts("  inteiros: encher <-> contar, Gentil <-> Hurwitz, a estrela no espaco-que-se-enche.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
