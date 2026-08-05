/* hipercubo32.c — 32 E' O HIPERCUBO DE DIMENSAO 4, E E' ONDE A VOLTA FECHA.
 *
 * O Aarao: "32 e' o numero de arestas do hipercubo, a distancia maxima percorrida na
 * velocidade maxima."
 *
 * O hipercubo de dimensao n tem 2^n vertices e n.2^(n-1) arestas, e em n=4 sao 32. Mas o
 * numero sozinho nao diz nada — o que o faz ser ESTE numero e' a volta fechar la':
 *
 *   cada vertice tem grau n, e um circuito que passe por TODAS as arestas e volte ao
 *   principio existe exactamente quando todos os graus sao PARES (Euler). Logo o
 *   hipercubo fecha para n PAR e nao fecha para n IMPAR.
 *
 * n=4 e' o primeiro par onde o grau tambem e' o periodo da involucao (G^4 = +I, F^4 = id),
 * e ai a distancia maxima — percorrer as 32 arestas sem repetir uma — e' um CIRCUITO: sai
 * e volta. Percorrer tudo a velocidade maxima e voltar E' a involucao, feita de uma vez.
 *
 *   §H1  as contagens: 2^n vertices, n.2^(n-1) arestas, e 32 em n=4
 *   §H2  o grau e' n em TODO o vertice — nao ha' vertice privilegiado
 *   §H3  a volta fecha sse n e' PAR, e mede-se construindo o circuito e conferindo-o
 *   §H4  e o diametro e' n: a distancia maxima entre dois vertices e' trocar n bits
 *
 * Zero doubles. Os vertices sao inteiros e as arestas sao um XOR de um bit.
 *
 *   cc -O2 -std=c99 -Wall -I../lib hipercubo32.c -o hipercubo32
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define NMAXD 8
#define VMAX  (1<<NMAXD)

static int popc(int x){ int c=0; while(x){ c += x&1; x >>= 1; } return c; }

/* Hierholzer: constroi um circuito euleriano se ele existir. Devolve o comprimento, ou -1.
 * O grafo e' o hipercubo: vertice v liga a v^(1<<b) para cada bit b. */
static long circuito(int n, unsigned char *usada, int *pilha, int *saida){
    long V = 1L << n, E = (long)n * (V/2);
    memset(usada, 0, (size_t)(V*n));
    long topo = 0, ns = 0;
    pilha[topo++] = 0;
    while(topo){
        int v = pilha[topo-1], b;
        for(b = 0; b < n; b++) if(!usada[(long)v*n + b]) break;
        if(b == n){ saida[ns++] = v; topo--; }
        else{
            int w = v ^ (1 << b);
            usada[(long)v*n + b] = 1;
            usada[(long)w*n + b] = 1;              /* a mesma aresta, dos dois lados */
            pilha[topo++] = w;
        }
    }
    /* um circuito euleriano tem E+1 vertices na lista e comeca onde acaba */
    if(ns != E + 1) return -1;
    if(saida[0] != saida[ns-1]) return -1;
    return E;
}

int main(void){
    /* sem `static`: 18 KB na pilha e ZERO em .bss — o portao mede .bss, e um rascunho
     * de funcao nao tem por que la' estar */
    unsigned char usada[VMAX*NMAXD];
    int pilha[VMAX*NMAXD + 8], saida[VMAX*NMAXD + 8];

    puts("\n  32 E' O HIPERCUBO DE DIMENSAO 4 — e e' onde a volta fecha\n");

    /* ═══ §H1 — as contagens ═══════════════════════════════════════════════════════ */
    {
        long mau = 0, a4 = 0;
        printf("      n :  vertices  arestas   grau\n");
        for(int n = 1; n <= 6; n++){
            long V = 1L << n, E = 0;
            for(long v = 0; v < V; v++) for(int b = 0; b < n; b++) if(!((v>>b)&1)) E++;
            long prev = (long)n * (V/2);
            if(E != prev) mau++;
            if(n == 4) a4 = E;
            printf("      %d :  %7ld  %7ld  %5d%s\n", n, V, E, n, n==4 ? "   <- 32" : "");
        }
        ok("o hipercubo de dimensao n tem 2^n vertices e n.2^(n-1) arestas, contadas uma a"
           " uma e nao pela formula — e em n=4 sao 32", mau == 0 && a4 == 32);
    }

    /* ═══ §H2 — o grau e' n em todo o vertice ══════════════════════════════════════ */
    {
        long mau = 0;
        for(int n = 1; n <= 6; n++){
            long V = 1L << n;
            for(long v = 0; v < V; v++){
                /* contam-se os vizinhos DISTINTOS e dentro do cubo — contar o numero de
                 * bits era contar n outra vez, e uma assercao dessas passa sem poder falhar */
                int g = 0;
                for(long w = 0; w < V; w++) if(w != v && popc((int)(v ^ w)) == 1) g++;
                if(g != n) mau++;
            }
        }
        ok("e o grau e' n em TODO o vertice — nao ha' vertice privilegiado, que e' o que"
           " permite falar de velocidade sem escolher de onde se parte", mau == 0);
    }

    /* ═══ §H3 — a volta fecha sse n e' PAR ═════════════════════════════════════════
     * Aqui esta' o ponto. Nao se cita Euler e passa-se a' frente: constroi-se o circuito e
     * confere-se que ele usa CADA aresta uma vez e acaba onde comecou. Nos impares a
     * construcao falha, e falha por razao — o grau e' impar. */
    {
        long mau = 0;
        printf("\n      n : arestas   circuito que passa por todas e VOLTA?\n");
        for(int n = 1; n <= 6; n++){
            long E = (long)n * ((1L<<n)/2);
            long c = circuito(n, usada, pilha, saida);
            int par = (n % 2 == 0);
            printf("      %d : %7ld   %s\n", n, E,
                   c >= 0 ? "SIM, e usa cada aresta uma vez" : "nao fecha (grau impar)");
            if(par && c != E) mau++;               /* par: tem de fechar, com E arestas */
            if(!par && c >= 0) mau++;              /* impar: nao pode fechar */
        }
        ok("a volta FECHA exactamente nas dimensoes PARES: o circuito construido usa cada"
           " aresta uma vez e acaba onde comecou, e nas impares nao existe — e' o grau que"
           " decide, e o grau e' a dimensao", mau == 0);
    }

    /* ═══ §H4 — o diametro e' n ════════════════════════════════════════════════════
     * A distancia entre dois vertices e' quantos bits diferem, e o maximo e' n — o vertice
     * OPOSTO. E o oposto de v e' v com todos os bits trocados, que e' a involucao do cubo:
     * a distancia maxima e' atingida no ponto onde ir e voltar sao o mesmo caminho. */
    {
        long mau = 0; int diam4 = 0;
        for(int n = 1; n <= 6; n++){
            long V = 1L << n; int maxd = 0, quantos_no_max = 0;
            for(long v = 0; v < V; v++){
                int d = popc((int)(v ^ 0));                  /* de 0 a v */
                if(d > maxd){ maxd = d; quantos_no_max = 1; }
                else if(d == maxd) quantos_no_max++;
            }
            if(maxd != n) mau++;
            if(quantos_no_max != 1) mau++;                   /* um so' oposto */
            if(n == 4) diam4 = maxd;
        }
        printf("\n      diametro em n=4: %d bits, e o vertice a essa distancia e' UNICO\n", diam4);
        ok("o diametro e' n e o vertice a distancia maxima e' UNICO — o oposto, que se"
           " obtem trocando todos os bits: a distancia maxima e' o ponto onde ir e voltar"
           " sao o mesmo caminho, que e' a involucao do cubo", mau == 0 && diam4 == 4);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  32 SAO AS ARESTAS DO HIPERCUBO DE DIMENSAO 4, contadas uma a uma. Mas o que");
        puts("  faz o numero ser este nao e' a contagem: e' a VOLTA FECHAR la'. Um percurso");
        puts("  que use todas as arestas e regresse ao principio existe exactamente quando");
        puts("  todos os graus sao pares, e o grau do hipercubo E' a dimensao.");
        puts("");
        puts("  DAI AS PARES FECHAREM E AS IMPARES NAO — o mesmo corte do isomorfo.c, onde");
        puts("  a folha dupla so' aparece nas pares. Em n=4 a duplicacao chega, o circuito");
        puts("  existe, e percorrer as 32 arestas a velocidade maxima E' a involucao feita");
        puts("  de uma vez: sai e volta, sem repetir uma aresta.");
        puts("");
        puts("  E O DIAMETRO E' n, atingido num vertice UNICO: o oposto, trocando todos os");
        puts("  bits. A distancia maxima e' onde ir e voltar sao o mesmo caminho — a mesma");
        puts("  coisa que o fase_regua.c mediu no circulo com min(p, q-p) maximo em q/2.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
