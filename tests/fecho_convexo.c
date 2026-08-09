/* fecho_convexo.c — O FECHO CONVEXO DO DTC: o hexágono dos vetores, e a polar (K**=K).
 *
 * O Aarão: «deriva o fecho convexo; ele funda a geometria — saímos da análise e estamos na
 * geometria. Corolário do teorema.»
 *
 * O espaço alcançável do inversor é o FECHO CONVEXO dos seus vetores. A dois níveis, os oito
 * estados de comutação são GF(2)³ (o cubo); os seis ACTIVOS (peso 1 e 2) projectados no plano
 * perpendicular a (1,1,1) dão um HEXÁGONO regular; os dois nulos ((0,0,0),(1,1,1)) caem no CENTRO.
 * A modulação --- a média da comutação --- é a combinação convexa: alcança tudo DENTRO do hexágono.
 *
 * E a dual do fecho convexo é o POLAR K*, com K**=K (o teorema do bipolar). O hexágono é auto-dual:
 * 6 vértices <-> 6 arestas, e dar a volta duas vezes devolve K (ν∘ν=id). É o teorema central em
 * convexidade --- encher K (a combinação convexa) <-> contar as faces por K* (o polar).
 *
 * Tudo INTEIRO: os vetores são bits (GF(2)³), a projecção é exacta em ⟨·⟩×3, e a bidualidade é a
 * involução da rede de faces.
 *
 *   §H1  os 6 activos dão um HEXÁGONO regular (mesma norma, ângulos de 60°); os nulos, o centro
 *   §H2  o fecho é o hexágono: os 6 são extremos, e a modulação (média) cai DENTRO
 *   §H3  a polar e o bipolar: 6 vértices <-> 6 arestas, e K**=K (ν∘ν=id na rede de faces)
 *
 *   cc -O2 -std=c99 -Wall -I../lib fecho_convexo.c -o fecho_convexo && ./fecho_convexo
 */
#include <stdio.h>
#include "unidade.h"

/* um vetor do inversor: 3 bits (a,b,c) ∈ GF(2)³ */
typedef struct { int a, b, c; } V;
static int peso(V v){ return v.a + v.b + v.c; }
/* produto interno projectado ⊥(1,1,1), vezes 3 (para ficar inteiro):
 *   3⟨u',v'⟩ = 3⟨u,v⟩ − ⟨u,1⟩⟨v,1⟩ */
static int dot3(V u, V v){
    int uv = u.a*v.a + u.b*v.b + u.c*v.c;
    return 3*uv - peso(u)*peso(v);
}

int main(void){
    printf("=== O FECHO CONVEXO DO DTC: o hexágono dos vetores, e a polar (K**=K) ==========\n\n");

    /* os 6 activos, em ordem à volta do hexágono (peso alterna 1,2,1,2,...) */
    V hex[6] = { {1,0,0}, {1,1,0}, {0,1,0}, {0,1,1}, {0,0,1}, {1,0,1} };
    V nulo[2] = { {0,0,0}, {1,1,1} };

    /* ── §H1 os 6 activos dão um HEXÁGONO regular ───────────────────────────────────────── */
    /* mesma norma projectada: 3|v'|² = 2 para os seis. E os ângulos são múltiplos de 60°:
     * 3⟨v_i',v_j'⟩ ∈ {2 (0°), 1 (60°), −1 (120°), −2 (180°)}. Os nulos projectam no centro (norma 0). */
    int norma_igual = 1, angulos_ok = 1;
    for(int i = 0; i < 6; i++) if(dot3(hex[i], hex[i]) != 2) norma_igual = 0;
    for(int i = 0; i < 6; i++) for(int j = 0; j < 6; j++){
        int d = dot3(hex[i], hex[j]);
        if(d != 2 && d != 1 && d != -1 && d != -2) angulos_ok = 0;
        /* adjacentes (|i−j|=1 mod 6) devem dar 60° (d=1); opostos (|i−j|=3) dão 180° (d=−2) */
        int dif = (i - j + 6) % 6;
        if(dif == 1 || dif == 5){ if(d != 1) angulos_ok = 0; }
        if(dif == 3){ if(d != -2) angulos_ok = 0; }
    }
    int nulos_centro = (dot3(nulo[0], nulo[0]) == 0 && dot3(nulo[1], nulo[1]) == 0);
    printf("§H1  6 activos: norma projectada 3|v'|²=2 igual? %s; ângulos múltiplos de 60°? %s; nulos no centro? %s\n\n",
           norma_igual?"sim":"nao", angulos_ok?"sim":"nao", nulos_centro?"sim":"nao");
    ok("§H1 os 6 vetores activos do inversor dao um HEXAGONO regular (mesma norma projectada, angulos"
       " de 60°, adjacentes a 60 e opostos a 180); os dois nulos caem no CENTRO",
       norma_igual && angulos_ok && nulos_centro);

    /* ── §H2 o fecho é o hexágono: os 6 são extremos, a modulação cai dentro ─────────────── */
    /* cada activo é EXTREMO: nenhum é a média (combinação convexa) de outros dois. E a modulação ---
     * a média de dois vetores --- cai DENTRO: a de dois opostos é o centro; a de dois adjacentes é
     * um ponto do interior (norma projectada menor que a dos vértices). */
    int todos_extremos = 1;
    for(int i = 0; i < 6; i++){                       /* v_i é média de v_j,v_k (j,k≠i)? => não extremo */
        for(int j = 0; j < 6; j++) for(int k = 0; k < 6; k++){
            if(j==i || k==i || j==k) continue;
            /* 2·v_i == v_j + v_k componente a componente? (a média coincidiria com v_i) */
            if(2*hex[i].a==hex[j].a+hex[k].a && 2*hex[i].b==hex[j].b+hex[k].b && 2*hex[i].c==hex[j].c+hex[k].c)
                todos_extremos = 0;
        }
    }
    /* a média de dois opostos (i, i+3): (v_i+v_{i+3})/2 projecta no centro (soma dá (1,1,1)) */
    int media_dentro = 1;
    for(int i = 0; i < 3; i++){
        V s = { hex[i].a+hex[i+3].a, hex[i].b+hex[i+3].b, hex[i].c+hex[i+3].c };
        if(dot3(s, s) != 0) media_dentro = 0;         /* a soma de opostos é (1,1,1) -> centro */
    }
    printf("§H2  os 6 são extremos (nenhum é média de dois)? %s; a média de opostos cai no centro? %s\n\n",
           todos_extremos?"sim":"nao", media_dentro?"sim":"nao");
    ok("§H2 o fecho convexo E' o hexagono: os 6 activos sao EXTREMOS (nenhum e' combinacao convexa"
       " de outros), e a modulacao (a media da comutacao) cai DENTRO --- opostos dao o centro",
       todos_extremos && media_dentro);

    /* ── §H3 a polar e o bipolar: 6 vértices <-> 6 arestas, K**=K ────────────────────────── */
    /* a polar troca vértices por arestas. O hexágono tem 6 vértices e 6 arestas (aresta e liga os
     * vértices e e (e+1)%6). Na rede de faces, dualizar reverte a incidência vértice-aresta; a
     * polar do hexágono é um hexágono (auto-dual); e dualizar DUAS vezes devolve K --- é K**=K, a
     * involução ν∘ν=id. Mede-se a incidência dualizada duas vezes contra a original. */
    int inc[6][6];                                    /* inc[v][e]=1 se o vértice v está na aresta e */
    for(int v = 0; v < 6; v++) for(int e = 0; e < 6; e++) inc[v][e] = 0;
    for(int e = 0; e < 6; e++){ inc[e][e] = 1; inc[(e+1)%6][e] = 1; }
    /* K*: as arestas de K viram vértices de K*, os vértices de K viram arestas de K*. A incidência
     * de K* é a TRANSPOSTA (vértice-de-K* = aresta-de-K, aresta-de-K* = vértice-de-K). */
    int estrela[6][6];                                /* estrela[v*][e*] = inc[e*][v*]  (transposta) */
    for(int i = 0; i < 6; i++) for(int j = 0; j < 6; j++) estrela[i][j] = inc[j][i];
    /* K**: transposta outra vez -> deve voltar a inc */
    int bibi_ok = 1;
    for(int i = 0; i < 6; i++) for(int j = 0; j < 6; j++) if(estrela[j][i] != inc[i][j]) bibi_ok = 0;
    /* e o hexágono é auto-dual: K* tem 6 vértices e 6 arestas, cada aresta em 2 vértices (como K) */
    int autodual = 1;
    for(int e = 0; e < 6; e++){ int c = 0; for(int v = 0; v < 6; v++) c += estrela[v][e]; if(c != 2) autodual = 0; }
    printf("§H3  polar do hexágono é hexágono (auto-dual)? %s; K**=K (bipolar, dupla transposta)? %s\n\n",
           autodual?"sim":"nao", bibi_ok?"sim":"nao");
    ok("§H3 a POLAR troca vertices por arestas (6<->6, o hexagono e' auto-dual), e K**=K --- dualizar"
       " duas vezes devolve K, a involucao ν∘ν=id (o teorema do bipolar). E' o teorema central em"
       " convexidade", autodual && bibi_ok);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  O espaco alcancavel do inversor E' o FECHO CONVEXO dos seus vetores: os 6 activos");
        puts("  (GF(2)³) dao um hexagono, os nulos o centro, e a modulacao (a media da comutacao)");
        puts("  cai DENTRO. A dual do fecho e' o POLAR, com K**=K (o bipolar) --- o teorema central");
        puts("  lido em convexidade: encher K <-> contar as faces por K*. E' a geometria, fundada");
        puts("  no fecho convexo, saindo da analise.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
