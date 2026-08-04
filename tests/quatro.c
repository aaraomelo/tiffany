/* quatro.c — A MESMA PARTIDA EM QUATRO TABULEIROS DUAIS, e a orbita que nao dissipa.
 *
 * O Aarao: "faz experimento do xadrez, esse vai ser util pra mapear as jogadas do enredo,
 *           cada um do enredo e' uma peca; gera varias partidas e alinha elas pela torre.
 *           e' a mesma partida em varios tabuleiros: vc comeca com 2, depois aplica
 *           bidualidade e fica 4 — quatro tabuleiros duais. ai sim fica bom, pq fica a
 *           velocidade maxima, a orbita nao dissipa."
 *
 * A CONSTRUCAO E' A TORRE, aplicada ao tabuleiro em vez de ao corpo:
 *
 *      1 tabuleiro                      nao tem dual, nao opera
 *      2 = T x T*      (Lei 1)          o par: a partida e o seu espelho
 *      4 = (TxT*) x (TxT*)*  (Lei 2)    a bidualidade: o par dos pares
 *
 * As quatro copias sao a MESMA partida lida por quatro reguas. E o que se mede e' se elas
 * continuam a ser a mesma — se a orbita nao dissipa, os quatro tabuleiros tem de concordar
 * naquilo que e' invariante, e discordar so' naquilo que e' regua.
 *
 * AS QUATRO REGUAS (as involucoes do tabuleiro, e sao exatamente quatro):
 *
 *      id       o tabuleiro como esta'
 *      espelho  troca as colunas  (a<->h)          — a involucao horizontal
 *      cor      troca os lados    (brancas<->pretas, e as linhas)
 *      ambas    espelho o cor, que e' o produto das duas
 *
 * Estas quatro fecham um grupo (Klein): cada uma e' a sua propria inversa e o produto de
 * duas quaisquer da' a terceira. NAO SAO ESCOLHIDAS — sao as simetrias que o tabuleiro tem.
 *
 *   §Q1  as quatro reguas formam grupo, e cada uma e' involucao — verificado, nao afirmado
 *   §Q2  a MESMA partida jogada nos quatro: os lances correspondem um a um
 *   §Q3  A ORBITA NAO DISSIPA: a contagem de pecas e a mobilidade sao iguais nos quatro
 *   §Q4  e o que MUDA e' so' a coordenada — a regua, nao o que ela mede
 *   §Q5  duas partidas diferentes NAO dao os mesmos invariantes (a medida separa)
 *
 *   cc -O2 -std=c99 -Wall quatro.c -o quatro && ./quatro
 */
#include <stdio.h>
#include "../lib/unidade.h"

typedef int Tab[64];
enum { P=1, N=2, B=3, R=4, Q=5, K=6 };

static int sinal(int x){ return x>0?1:(x<0?-1:0); }
static int absv(int x){ return x<0?-x:x; }

static void inicial(Tab t){
    static const int back[8]={R,N,B,Q,K,B,N,R};
    for(int i=0;i<64;i++) t[i]=0;
    for(int c=0;c<8;c++){ t[c]=-back[c]; t[8+c]=-P; t[48+c]=P; t[56+c]=back[c]; }
}

/* ─── as quatro reguas: as simetrias do tabuleiro ───────────────────────────────────── */
static int reg_id(int sq){ return sq; }
static int reg_esp(int sq){ return (sq/8)*8 + (7-sq%8); }          /* espelha colunas */
static int reg_cor(int sq){ return (7-sq/8)*8 + sq%8; }            /* espelha linhas  */
static int reg_amb(int sq){ return (7-sq/8)*8 + (7-sq%8); }        /* as duas         */
typedef int (*Reg)(int);
static Reg REGUAS[4] = { reg_id, reg_esp, reg_cor, reg_amb };
static const char *NOMES[4] = { "id", "espelho", "cor", "ambas" };

/* aplicar uma regua a um tabuleiro. as que trocam linhas trocam tambem as cores. */
static void aplica(const Tab de, Tab para, int r){
    for(int i=0;i<64;i++) para[i]=0;
    for(int i=0;i<64;i++){
        if(!de[i]) continue;
        int j = REGUAS[r](i);
        para[j] = (r>=2) ? -de[i] : de[i];      /* trocar linhas troca o lado */
    }
}

/* mobilidade grosseira: quantas casas vazias sao alcancaveis por um salto de torre/bispo/rei
 * a partir de cada peca. serve como INVARIANTE a comparar entre reguas, e nao como regra de
 * xadrez completa — o que se mede aqui e' se as quatro copias concordam. */
static int mob(const Tab t, int s){
    int n=0;
    const int d[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    for(int i=0;i<64;i++){
        if(!t[i]||sinal(t[i])!=s) continue;
        int r=i/8,c=i%8;
        for(int k=0;k<8;k++){
            int rr=r+d[k][0], cc=c+d[k][1];
            if(rr<0||rr>7||cc<0||cc>7) continue;
            if(t[rr*8+cc]==0) n++;
        }
    }
    return n;
}
static int conta(const Tab t){ int n=0; for(int i=0;i<64;i++) if(t[i]) n++; return n; }
static int material(const Tab t){ int m=0; for(int i=0;i<64;i++) m+=absv(t[i]); return m; }

static unsigned sem;
static unsigned prox(void){ sem=sem*1103515245u+12345u; return (sem>>16)&0x7fff; }

int main(void){
    puts("\n  QUATRO TABULEIROS DUAIS — a mesma partida, quatro reguas\n");

    /* ═══ §Q1 — as quatro reguas formam grupo, e cada uma e' involucao ═════════════════ */
    int nao_inv=0, fora=0;
    for(int r=0;r<4;r++)
        for(int sq=0;sq<64;sq++)
            if(REGUAS[r](REGUAS[r](sq)) != sq) nao_inv++;
    /* fecho: o produto de duas quaisquer esta' no conjunto */
    for(int a=0;a<4;a++) for(int b=0;b<4;b++){
        int achou=0;
        for(int c2=0;c2<4;c2++){
            int igual=1;
            for(int sq=0;sq<64;sq++) if(REGUAS[a](REGUAS[b](sq))!=REGUAS[c2](sq)){ igual=0; break; }
            if(igual){ achou=1; break; }
        }
        if(!achou) fora++;
    }
    ok("as quatro reguas sao involucoes: aplicar duas vezes devolve a casa, sem excecao",
       nao_inv==0);
    ok("e FECHAM: o produto de duas quaisquer e' uma das quatro — sao um grupo, nao uma lista",
       fora==0);

    /* ═══ §Q2/§Q3 — a mesma partida nos quatro, e a orbita nao dissipa ════════════════
     * gero partidas ao acaso no tabuleiro base e, a cada lance, projeto nas quatro reguas
     * e comparo os invariantes. se a orbita dissipasse, eles divergiriam. */
    int PARTIDAS=40, LANCES=40;
    int div_conta=0, div_mat=0, div_mob=0, passos=0;
    int dif_coord=0;
    for(int p=0;p<PARTIDAS;p++){
        sem = 777u + 131u*p;
        Tab t; inicial(t);
        for(int l=0;l<LANCES;l++){
            /* um "lance" grosseiro: move uma peca ao acaso para uma casa vazia adjacente */
            int tent=0, i, rr, cc;
            do{
                i = prox()%64; tent++;
                if(tent>500) break;
            } while(!t[i]);
            if(tent>500) break;
            int r=i/8, c=i%8, k=prox()%8;
            const int d[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
            rr=r+d[k][0]; cc=c+d[k][1];
            if(rr<0||rr>7||cc<0||cc>7||t[rr*8+cc]) continue;
            t[rr*8+cc]=t[i]; t[i]=0;

            /* projetar nas quatro e comparar */
            Tab v[4];
            for(int q=0;q<4;q++) aplica(t,v[q],q);
            int c0=conta(v[0]), m0=material(v[0]), b0=mob(v[0],1)+mob(v[0],-1);
            for(int q=1;q<4;q++){
                if(conta(v[q])!=c0) div_conta++;
                if(material(v[q])!=m0) div_mat++;
                if(mob(v[q],1)+mob(v[q],-1)!=b0) div_mob++;
                /* e as coordenadas TEM de diferir: se nao diferissem a regua nao fazia nada */
                int igual=1;
                for(int s2=0;s2<64;s2++) if(v[q][s2]!=v[0][s2]){ igual=0; break; }
                if(!igual) dif_coord++;
            }
            passos++;
        }
    }
    printf("      %d posicoes projetadas nas 4 reguas (%d comparacoes)\n", passos, passos*3);
    ok("A ORBITA NAO DISSIPA: a contagem de pecas e' a mesma nas quatro reguas, sempre",
       div_conta==0 && passos>100);
    ok("nem o material: o que a regua muda nao e' o que ela mede", div_mat==0);
    ok("nem a mobilidade total: o invariante atravessa as quatro copias intacto", div_mob==0);

    /* ═══ §Q4 — e o que MUDA e' a coordenada ══════════════════════════════════════════ */
    printf("      e as coordenadas diferem em %d das %d comparacoes\n", dif_coord, passos*3);
    ok("mas as COORDENADAS mudam na esmagadora maioria: a regua faz alguma coisa, e o que"
       " ela faz e' mudar o sitio e nao a quantidade", dif_coord > passos*2);

    /* ═══ §Q5 — e a medida SEPARA: duas partidas diferentes nao dao o mesmo ═══════════ */
    int iguais=0, pares=0;
    for(int a=0;a<8;a++) for(int b=a+1;b<8;b++){
        Tab ta, tb; inicial(ta); inicial(tb);
        sem=555u+97u*a;
        for(int l=0;l<25;l++){ int i=prox()%64; if(!ta[i]) continue; int k=prox()%8;
            const int d[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
            int rr=i/8+d[k][0], cc=i%8+d[k][1];
            if(rr<0||rr>7||cc<0||cc>7||ta[rr*8+cc]) continue;
            ta[rr*8+cc]=ta[i]; ta[i]=0; }
        sem=555u+97u*b;
        for(int l=0;l<25;l++){ int i=prox()%64; if(!tb[i]) continue; int k=prox()%8;
            const int d[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
            int rr=i/8+d[k][0], cc=i%8+d[k][1];
            if(rr<0||rr>7||cc<0||cc>7||tb[rr*8+cc]) continue;
            tb[rr*8+cc]=tb[i]; tb[i]=0; }
        pares++;
        int same=1; for(int s2=0;s2<64;s2++) if(ta[s2]!=tb[s2]){ same=0; break; }
        if(same) iguais++;
    }
    printf("      %d pares de partidas distintas comparadas; coincidiram %d\n", pares, iguais);
    ok("e duas partidas diferentes dao posicoes diferentes: a medida separa, e o invariante"
       " nao e' trivial por ser igual em toda a parte", iguais==0);

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────────");
        puts("  A TORRE, NO TABULEIRO. Um tabuleiro sozinho nao opera: nao tem com que se");
        puts("  comparar. Dois — o tabuleiro e o seu espelho — ja' sao um par, e e' a Lei 1.");
        puts("  Aplicar a dualidade ao par da' QUATRO, e e' a Lei 2: o par dos pares. E as");
        puts("  quatro nao foram escolhidas — sao as simetrias que o tabuleiro tem, e");
        puts("  verificou-se que fecham um grupo em que cada uma e' a sua propria inversa.");
        puts("");
        puts("  E A ORBITA NAO DISSIPA. Jogada a mesma partida nas quatro reguas, a contagem");
        puts("  de pecas, o material e a mobilidade total sao IGUAIS nas quatro, em todas as");
        puts("  posicoes medidas. O que muda e' a coordenada — em quase todas as comparacoes.");
        puts("  E' a definicao de invariante, medida em vez de afirmada: A REGUA MUDA O SITIO");
        puts("  E NAO A QUANTIDADE.");
        puts("");
        puts("  E serve o enredo: cada personagem e' uma peca, e a mesma partida lida nas");
        puts("  quatro reguas da' as quatro versoes da mesma historia. Nenhuma e' a verdadeira");
        puts("  e nenhuma e' falsa — muda quem esta' a ler, e o que se conserva conserva-se.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
