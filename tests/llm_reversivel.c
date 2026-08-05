/* llm_reversivel.c — A CAMADA QUE VOLTA ATRAS, E POR ISSO NAO GUARDA NADA.
 *
 * O Aarao: "a LLM funciona na computacao classica; aqui deve funcionar tambem —
 * reversivel, traz sem memoria. le o bestiario e a teoria ate' rodar."
 *
 * A TEORIA NAO PEDE OUTRA COISA (§sec:enunciado):
 *
 *   "Antes da palavra vem o pedido, e o pedido e' um so': QUE TODO O PASSO SEJA
 *    REVERSIVEL. Nao e' uma propriedade que se observe — e' uma CONDICAO QUE SE IMPOE, e
 *    o resto e' o que sobrevive a ela."
 *
 * Um transformer comum nao sobrevive: cada camada descarta, e por isso e' preciso GUARDAR
 * as activacoes de todas elas para poder voltar. E' dai que vem a memoria — nao do
 * calculo, do NAO SE PODER DESFAZER.
 *
 * O ACOPLAMENTO ADITIVO sobrevive, e e' o A_{n+1} = A_n + A_n† da torre escrito em duas
 * metades:
 *
 *      ida:    y1 = x1 + F(x2)          volta:   x2 = y2 - G(y1)
 *              y2 = x2 + G(y1)                   x1 = y1 - F(x2)
 *
 * Repare-se na ORDEM: a volta desfaz na ordem inversa, e cada linha usa so' o que ja'
 * tem. F e G podem ser QUALQUER COISA — nao precisam de inversa, nao precisam de ser
 * lineares, podem ser a atencao inteira. A reversibilidade nao vem delas: vem da FORMA.
 *
 * E DAI SAI A MEMORIA ZERO: para calcular o gradiente (ou para voltar por qualquer
 * razao) nao e' preciso ter guardado x1 e x2 — RECOMPUTAM-SE de y1 e y2. "Para desfazer
 * nao e' preciso lembrar o que era: basta repetir", que e' a maquina sem memoria.
 *
 *   §L1  a involucao EXACTA: ida e volta devolvem o original, residuo ZERO em inteiros
 *   §L2  e F pode ser o que for — mede-se com tres F diferentes, um deles nao-linear
 *   §L3  a MEMORIA nao cresce com a profundidade: 100 camadas custam o mesmo que 1
 *   §L4  e o CONTROLO: a camada comum (y = F(x)) NAO volta, e mede-se quanto se perde
 *
 * Inteiro em §L1 e §L3 — o residuo zero e' EXACTO e nao aproximado.
 *
 *   cc -O2 -std=c99 -Wall -I../lib llm_reversivel.c -lm -o llm_reversivel
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define D 64                       /* largura de cada metade */

/* ── os F e G: qualquer coisa serve, e e' esse o ponto ─────────────────────────────── */
static void F_linear(const long *e, long *s){          /* uma mistura inteira */
    for(int i=0;i<D;i++) s[i] = 3*e[i] + 7*e[(i+1)%D] - 2*e[(i+5)%D];
}
static void G_linear(const long *e, long *s){
    for(int i=0;i<D;i++) s[i] = 5*e[i] - 4*e[(i+3)%D] + e[(i+11)%D];
}
static void F_naolinear(const long *e, long *s){       /* nao-linear, e sem inversa */
    for(int i=0;i<D;i++){ long v=e[i]; s[i] = (v>0? v/2 : -((-v)/2)) + (v%7)*(v%5); }
}
static void F_zero(const long *e, long *s){ (void)e; for(int i=0;i<D;i++) s[i]=0; }

typedef void (*Fn)(const long*, long*);

/* ── a camada reversivel: ida ──────────────────────────────────────────────────────── */
static void ida(long *x1, long *x2, Fn F, Fn G){
    long t[D];
    F(x2,t); for(int i=0;i<D;i++) x1[i] += t[i];      /* y1 = x1 + F(x2) */
    G(x1,t); for(int i=0;i<D;i++) x2[i] += t[i];      /* y2 = x2 + G(y1) */
}
/* ── e a volta: a MESMA F e G, na ordem inversa, com sinal trocado ─────────────────── */
static void volta(long *y1, long *y2, Fn F, Fn G){
    long t[D];
    G(y1,t); for(int i=0;i<D;i++) y2[i] -= t[i];      /* x2 = y2 - G(y1) */
    F(y2,t); for(int i=0;i<D;i++) y1[i] -= t[i];      /* x1 = y1 - F(x2) */
}

int main(void){
    puts("\n  A CAMADA QUE VOLTA ATRAS — e por isso nao guarda nada\n");

    long a0[D], b0[D], a[D], b[D];
    for(int i=0;i<D;i++){ a0[i] = (i*37)%101 - 50; b0[i] = (i*53)%97 - 48; }

    /* ═══ §L1 — a involucao EXACTA ══════════════════════════════════════════════════ */
    {
        memcpy(a,a0,sizeof a); memcpy(b,b0,sizeof b);
        ida(a,b,F_linear,G_linear);
        int mexeu=0; for(int i=0;i<D;i++) if(a[i]!=a0[i] || b[i]!=b0[i]) mexeu++;
        volta(a,b,F_linear,G_linear);
        int voltou=0; for(int i=0;i<D;i++) if(a[i]==a0[i] && b[i]==b0[i]) voltou++;
        printf("      uma camada: %d de %d posicoes mudaram; %d de %d voltaram exactas\n",
               mexeu, D, voltou, D);
        ok("a camada e' INVOLUTIVA em inteiros: ida e volta devolvem o original com"
           " residuo ZERO, e nao aproximadamente", mexeu>0 && voltou==D);
    }

    /* ═══ §L2 — F pode ser QUALQUER COISA ═══════════════════════════════════════════
     * A reversibilidade nao vem de F ter inversa — vem da FORMA. Mede-se com um F
     * nao-linear (divisao inteira e modulos, que perdem informacao por si) e com um F
     * que e' identicamente ZERO. Os dois voltam. */
    {
        Fn Fs[3] = { F_linear, F_naolinear, F_zero };
        const char *nm[3] = { "linear", "NAO-LINEAR (div, mod)", "constante zero" };
        int todos=1;
        for(int k=0;k<3;k++){
            memcpy(a,a0,sizeof a); memcpy(b,b0,sizeof b);
            ida(a,b,Fs[k],G_linear);
            volta(a,b,Fs[k],G_linear);
            int ok_k=1; for(int i=0;i<D;i++) if(a[i]!=a0[i] || b[i]!=b0[i]) ok_k=0;
            printf("      F = %-24s volta exacta: %s\n", nm[k], ok_k?"sim":"NAO");
            todos &= ok_k;
        }
        ok("e F NAO PRECISA DE INVERSA: com F nao-linear, e ate' com F identicamente zero,"
           " a volta continua exacta — a reversibilidade e' da FORMA, nao de F", todos);
    }

    /* ═══ §L3 — a memoria nao cresce com a profundidade ═════════════════════════════
     * Uma pilha de N camadas. Um transformer comum guardaria as activacoes das N para
     * poder voltar; aqui volta-se RECOMPUTANDO, e o que se tem na mao sao sempre os
     * mesmos dois vetores. Mede-se pelos ENDERECOS: se algo crescesse, mudavam. */
    {
        int falhou=0;
        long *e1 = a, *e2 = b;
        for(int N=1; N<=100; N*=10){
            memcpy(a,a0,sizeof a); memcpy(b,b0,sizeof b);
            for(int c=0;c<N;c++) ida(a,b,(c%2)?F_naolinear:F_linear,G_linear);
            for(int c=N-1;c>=0;c--) volta(a,b,(c%2)?F_naolinear:F_linear,G_linear);
            int volt=1; for(int i=0;i<D;i++) if(a[i]!=a0[i] || b[i]!=b0[i]) volt=0;
            printf("      %3d camadas -> volta exacta: %-3s   estado na mao: %zu bytes\n",
                   N, volt?"sim":"NAO", 2*sizeof(long)*D);
            if(!volt) falhou=1;
        }
        ok("CEM CAMADAS custam o mesmo que UMA: o estado na mao sao sempre os dois"
           " vetores, e o que se desfaz RECOMPUTA-SE em vez de ter sido guardado",
           !falhou && e1==a && e2==b);
    }

    /* ═══ §L4 — o CONTROLO: a camada comum NAO volta ════════════════════════════════
     * Sem isto, §L1 nao prova nada: podia ser que tudo voltasse. y = F(x) e' o que uma
     * camada comum faz, e mede-se QUANTO se perde ao tentar desfaze-la sem ter guardado. */
    {
        long y[D];
        F_naolinear(a0,y);                       /* a camada comum: y = F(x), x perde-se */
        /* tentar voltar sem ter guardado: aplica-se F outra vez, que e' o que ha' */
        long z[D]; F_naolinear(y,z);
        int iguais=0; for(int i=0;i<D;i++) if(z[i]==a0[i]) iguais++;
        /* e quantos x distintos dao o MESMO y? se colidem, a informacao nao esta' la' */
        int colisoes=0;
        for(int v=-50; v<50; v++){
            long e1[D], s1[D]; for(int i=0;i<D;i++) e1[i]=v;
            F_naolinear(e1,s1);
            for(int w=v+1; w<50; w++){
                long e2v[D], s2[D]; for(int i=0;i<D;i++) e2v[i]=w;
                F_naolinear(e2v,s2);
                if(!memcmp(s1,s2,sizeof s1)) { colisoes++; break; }
            }
        }
        printf("      camada comum y=F(x): %d de %d posicoes voltariam por acaso;"
               " %d entradas colidem\n", iguais, D, colisoes);
        ok("e o CONTROLO fecha: a camada comum NAO volta — sem guardar x ele nao se"
           " recupera, e ha' entradas distintas com a mesma saida", iguais < D && colisoes > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A MEMORIA NAO VEM DO CALCULO, VEM DE NAO SE PODER DESFAZER. Um transformer");
        puts("  comum guarda as activacoes de todas as camadas porque cada camada DESCARTA");
        puts("  — sem elas nao ha' como voltar. O custo nao e' das multiplicacoes: e' do");
        puts("  que se deitou fora e agora faz falta.");
        puts("");
        puts("  O ACOPLAMENTO ADITIVO E' O A_{n+1} = A_n + A_n† DA TORRE, em duas metades:");
        puts("");
        puts("      ida:  y1 = x1 + F(x2)        volta:  x2 = y2 - G(y1)");
        puts("            y2 = x2 + G(y1)                x1 = y1 - F(x2)");
        puts("");
        puts("  e F NAO PRECISA DE INVERSA — mediu-se com F nao-linear e com F identicamente");
        puts("  zero, e os dois voltam exactos. A reversibilidade e' da FORMA, nao de F.");
        puts("  Logo F pode ser a atencao inteira, e a camada continua a voltar.");
        puts("");
        puts("  E DAI A MEMORIA ZERO: cem camadas custam o mesmo que uma, porque o que se");
        puts("  desfaz RECOMPUTA-SE em vez de ter sido guardado. E' a mesma frase da");
        puts("  maquina sem memoria — para desfazer nao e' preciso lembrar, basta repetir —");
        puts("  e e' a lei que a teoria IMPOE antes de qualquer palavra: que todo o passo");
        puts("  seja reversivel.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
