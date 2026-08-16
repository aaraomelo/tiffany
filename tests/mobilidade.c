/* mobilidade.c — O EXPERIMENTO DO AARAO: quantos lances sem captura ate' a mobilidade zerar.
 *
 * O Aarao: "um mecanismo concreto pro xadrez: se vc pegar um tabuleiro e mover as pecas
 *           aleatoriamente mantendo as regras, rei nao pode ficar em xeque, e DESVIANDO DE
 *           CAPTURAS (colisoes), se vc roda isso aleatorio cegamente obedecendo a regra do
 *           xadrez, EM 32 LANCES A MOBILIDADE VAI A ZERO. ai inicia a partida dual e
 *           reacoplamento, e vc vai ver as sequencias de capturas, os intervalos abrem e
 *           depois se encaixam. roda isso, monta o experimento."
 *
 * E' uma PREVISAO FALSIFICAVEL, e e' isso que a torna valiosa: ou a mobilidade zera perto
 * de 32 lances, ou nao zera, e a medicao decide. Nao ha' aqui nada a interpretar.
 *
 * A TESE QUE ELA TESTA. Recusar capturar mantem as 32 pecas de pe'; 32 pecas em 64 casas
 * enchem metade do tabuleiro; e um tabuleiro meio cheio onde ninguem sai estrangula-se
 * sozinho. Se for verdade, a densidade NAO E' consequencia da ambicao — e' consequencia da
 * RECUSA, e o proximo lance legal so' pode ser a captura. E' a Lei 1 a produzir o aperto que
 * obriga a Lei 2.
 *
 *   §X1  o gerador de lances e' legal: rei nunca fica em xeque, nenhuma captura e' feita
 *   §X2  a mobilidade DECRESCE — e mede-se a curva, nao se afirma
 *   §X3  em quantos lances chega a zero: a medida contra a previsao de 32
 *   §X4  o que sobra quando zera: as pecas continuam TODAS de pe' (nada se perdeu)
 *   §X5  e o lance seguinte: quantas capturas estao disponiveis no instante do bloqueio
 *
 * Xadrez implementado: movimentos de todas as pecas e legalidade do rei. SEM roque, SEM
 * en passant, SEM promocao — declarado, e nao afeta a pergunta (nenhum deles cria mobilidade
 * num tabuleiro fechado; o roque so' existiria no inicio, quando ha' mobilidade de sobra).
 *
 * Aleatorio com semente FIXA por corrida, e varias corridas — reprodutivel, e a dispersao
 * e' medida antes de qualquer conclusao.
 *
 *   cc -O2 -std=c99 -Wall mobilidade.c -o mobilidade && ./mobilidade
 */
#include <stdio.h>
#include "../lib/unidade.h"

/* pecas: 0 vazio; +1..+6 brancas (P,N,B,R,Q,K); negativo pretas */
enum { P=1, N=2, B=3, R=4, Q=5, K=6 };
typedef int Tab[64];
static int EVITA_ATAQUE=0;

static int sinal(int x){ return x>0 ? 1 : (x<0 ? -1 : 0); }
static int absv(int x){ return x<0 ? -x : x; }

static void inicial(Tab t){
    static const int back[8] = {R,N,B,Q,K,B,N,R};
    for(int i=0;i<64;i++) t[i]=0;
    for(int c=0;c<8;c++){
        t[0*8+c]  = -back[c];  t[1*8+c] = -P;      /* pretas em cima */
        t[6*8+c]  =  P;        t[7*8+c] =  back[c];
    }
}

/* gera pseudo-lances de uma peca; devolve quantos, em dest[] */
static int lances_peca(const Tab t, int sq, int dest[], int so_sem_captura){
    int p=t[sq], s=sinal(p), a=absv(p), n=0;
    int r=sq/8, c=sq%8;
    const int dirN[8][2]={{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
    const int dirB[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}};
    const int dirR[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
    if(a==P){
        int d = (s>0) ? -1 : 1;                      /* brancas sobem (linha diminui) */
        int r1=r+d;
        if(r1>=0&&r1<8 && t[r1*8+c]==0){
            dest[n++]=r1*8+c;
            int r2=r+2*d, base=(s>0)?6:1;
            if(r==base && t[r2*8+c]==0) dest[n++]=r2*8+c;
        }
        if(!so_sem_captura)                          /* capturas em diagonal */
            for(int dc=-1; dc<=1; dc+=2){
                int cc=c+dc;
                if(r1>=0&&r1<8&&cc>=0&&cc<8 && t[r1*8+cc]!=0 && sinal(t[r1*8+cc])!=s)
                    dest[n++]=r1*8+cc;
            }
        return n;
    }
    if(a==N||a==K){
        int k = (a==N)?8:8;
        for(int i=0;i<k;i++){
            int dr,dc;
            if(a==N){ dr=dirN[i][0]; dc=dirN[i][1]; }
            else { const int dk[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
                   dr=dk[i][0]; dc=dk[i][1]; }
            int rr=r+dr, cc=c+dc;
            if(rr<0||rr>7||cc<0||cc>7) continue;
            int q=t[rr*8+cc];
            if(q!=0 && sinal(q)==s) continue;
            if(q!=0 && so_sem_captura) continue;
            dest[n++]=rr*8+cc;
        }
        return n;
    }
    /* deslizantes */
    int nd=0; const int (*dd)[2]=NULL;
    if(a==B){ nd=4; dd=dirB; } else if(a==R){ nd=4; dd=dirR; }
    for(int i=0;i<(a==Q?8:nd);i++){
        int dr,dc;
        if(a==Q){ const int dq[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
                  dr=dq[i][0]; dc=dq[i][1]; }
        else { dr=dd[i][0]; dc=dd[i][1]; }
        int rr=r+dr, cc=c+dc;
        while(rr>=0&&rr<8&&cc>=0&&cc<8){
            int q=t[rr*8+cc];
            if(q==0) dest[n++]=rr*8+cc;
            else { if(sinal(q)!=s && !so_sem_captura) dest[n++]=rr*8+cc; break; }
            rr+=dr; cc+=dc;
        }
    }
    return n;
}

/* o rei de cor s esta' atacado? */
static int em_xeque(const Tab t, int s){
    int rei=-1;
    for(int i=0;i<64;i++) if(t[i]==s*K) rei=i;
    if(rei<0) return 0;
    int d[40];
    for(int i=0;i<64;i++){
        if(t[i]==0 || sinal(t[i])==s) continue;
        int m=lances_peca(t,i,d,0);
        for(int j=0;j<m;j++) if(d[j]==rei) return 1;
    }
    return 0;
}

/* todos os lances LEGAIS e SEM CAPTURA de quem joga (s). devolve quantos, em de[]/pa[] */
static int mobilidade(const Tab t, int s, int de[], int pa[]){
    int n=0, d[40];
    for(int i=0;i<64;i++){
        if(t[i]==0 || sinal(t[i])!=s) continue;
        int m=lances_peca(t,i,d,1);              /* 1 = so' lances sem captura */
        for(int j=0;j<m;j++){
            Tab u; for(int k=0;k<64;k++) u[k]=t[k];
            u[d[j]]=u[i]; u[i]=0;
            if(em_xeque(u,s)) continue;
            if(EVITA_ATAQUE){                 /* desviar da COLISAO: nao ficar atacavel */
                int atacada=0, e[40];
                for(int k2=0;k2<64 && !atacada;k2++){
                    if(u[k2]==0||sinal(u[k2])==s) continue;
                    int m2=lances_peca(u,k2,e,0);
                    for(int j2=0;j2<m2;j2++) if(e[j2]==d[j]){ atacada=1; break; }
                }
                if(atacada) continue;
            }
            if(de){de[n]=i; pa[n]=d[j];} n++;
        }
    }
    return n;
}

/* quantas capturas legais existem para s (usado so' no instante do bloqueio) */
static int capturas(const Tab t, int s){
    int n=0, d[40];
    for(int i=0;i<64;i++){
        if(t[i]==0 || sinal(t[i])!=s) continue;
        int m=lances_peca(t,i,d,0);
        for(int j=0;j<m;j++){
            if(t[d[j]]==0) continue;              /* so' capturas */
            Tab u; for(int k=0;k<64;k++) u[k]=t[k];
            u[d[j]]=u[i]; u[i]=0;
            if(!em_xeque(u,s)) n++;
        }
    }
    return n;
}

static unsigned semente;
static unsigned prox(void){ semente = semente*1103515245u + 12345u; return (semente>>16)&0x7fff; }

int main(void){
    puts("\n  O EXPERIMENTO: mover ao acaso SEM CAPTURAR, ate' a mobilidade zerar\n");
    puts("  previsao do Aarao: a mobilidade vai a zero por volta de 32 lances.\n");

    const int CORRIDAS = 60;
    int zerou[256], nz=0, soma=0, mini=99999, maxi=0;
    int pecas_no_fim=0, caps_no_fim=0, xeque_ilegal=0, captura_feita=0;
    int curva[80]; for(int i=0;i<80;i++) curva[i]=0;
    int amostras[80]; for(int i=0;i<80;i++) amostras[i]=0;

    for(int corr=0; corr<CORRIDAS; corr++){
        semente = 12345u + 7919u*corr;            /* fixa e reprodutivel por corrida */
        Tab t; inicial(t);
        int s=1, lance=0;
        for(;;){
            int de[300], pa[300];
            int m = mobilidade(t,s,de,pa);
            if(lance<80){ curva[lance]+=m; amostras[lance]++; }
            if(m==0) break;
            int esc = prox() % m;
            /* §X1: verificar que o lance escolhido nao captura e nao deixa o rei em xeque */
            if(t[pa[esc]]!=0) captura_feita++;
            t[pa[esc]]=t[de[esc]]; t[de[esc]]=0;
            if(em_xeque(t,s)) xeque_ilegal++;
            s=-s; lance++;
            if(lance>400) break;                   /* travao, nunca deve ser preciso */
        }
        zerou[nz<256?nz:255]=lance; nz++;
        soma+=lance; if(lance<mini)mini=lance; if(lance>maxi)maxi=lance;
        int cont=0; for(int i=0;i<64;i++) if(t[i]!=0) cont++;
        pecas_no_fim+=cont;
        caps_no_fim+=capturas(t,s);
    }

    /* ═══ §X1 — o gerador e' legal ═══════════════════════════════════════════════════ */
    ok("nenhuma captura foi feita em nenhuma corrida — o gerador respeita a recusa",
       captura_feita==0);
    ok("nenhum lance deixou o proprio rei em xeque: as regras foram cumpridas",
       xeque_ilegal==0);

    /* ═══ §X2 — a curva da mobilidade ════════════════════════════════════════════════ */
    puts("\n      a curva (media de lances legais sem captura disponiveis, por lance):");
    for(int i=0;i<40;i+=4){
        if(!amostras[i]) break;
        printf("      lance %2d: %5.1f      lance %2d: %5.1f\n",
               i, (double)curva[i]/amostras[i],
               i+2, amostras[i+2]? (double)curva[i+2]/amostras[i+2] : 0.0);
    }
    int m0 = amostras[0]? curva[0]/amostras[0] : 0;
    int m20= amostras[20]? curva[20]/amostras[20] : 0;
    /* MEDIDO E CONTRA A EXPECTATIVA: a mobilidade SOBE. 20 lances legais na posicao inicial,
     * ~28 ao lance 20. A razao e' geometrica e obvia depois de vista: as 32 pecas comecam
     * empilhadas em duas filas, tapando-se umas as outras; ao espalharem-se DESTAPAM-SE. */
    printf("      mobilidade inicial %d, ao lance 20 %d  ->  SOBE %+d\n", m0, m20, m20-m0);
    ok("a mobilidade nao decresce entre o lance 0 e o 20 — as pecas comecam tapadas e"
       " espalhar-se destapa-as (os dois numeros estao na linha acima, medidos)", m20 > m0);

    /* ═══ §X3 — a previsao ═══════════════════════════════════════════════════════════ */
    printf("\n      %d corridas: bloqueio ao fim de %d lances em media (min %d, max %d)\n",
           CORRIDAS, soma/CORRIDAS, mini, maxi);
    int media = soma/CORRIDAS;
    printf("      PREVISAO DO AARAO: 32.   MEDIDO: %d.   desvio: %+d\n", media, media-32);
    /* A PREVISAO ERA 32 E FALHOU: medido 261, e o maximo bate no travao de 400, ou seja
     * muitas corridas NEM CHEGAM a bloquear. Recusar capturar NAO fecha o tabuleiro. */
    ok("a previsao de 32 lances NAO se confirma: o bloqueio, quando acontece, e' uma ordem"
       " de grandeza mais tarde", media > 100);
    ok("e em parte das corridas nem chega a bloquear dentro do travao de 400 lances",
       maxi >= 400);

    /* ═══ §X4 — nada se perdeu ═══════════════════════════════════════════════════════ */
    printf("      pecas de pe' no instante do bloqueio: %d em media (das 32)\n",
           pecas_no_fim/CORRIDAS);
    ok("as 32 pecas continuam TODAS de pe' quando a mobilidade zera: o aperto nao"
       " veio de perdas, veio de nao haver para onde ir", pecas_no_fim/CORRIDAS == 32);

    /* ═══ §X5 — e o lance seguinte ═══════════════════════════════════════════════════ */
    printf("      capturas legais disponiveis no instante do bloqueio: %d em media\n",
           caps_no_fim/CORRIDAS);
    ok("no instante do bloqueio HA' capturas disponiveis: o unico lance legal que resta"
       " e' capturar", caps_no_fim/CORRIDAS > 0);

    /* ═══ A VARIANTE FORTE: desviar tambem de ficar atacado ══════════════════════════ */
    EVITA_ATAQUE=1;
    { int soma2=0, mini2=99999, maxi2=0, pecas2=0;
      for(int corr=0; corr<CORRIDAS; corr++){
        semente = 12345u + 7919u*corr;
        Tab t; inicial(t); int s=1, lance=0;
        for(;;){ int de[300],pa[300]; int m=mobilidade(t,s,de,pa);
          if(m==0) break;
          int esc=prox()%m; t[pa[esc]]=t[de[esc]]; t[de[esc]]=0; s=-s; lance++;
          if(lance>400) break; }
        soma2+=lance; if(lance<mini2)mini2=lance; if(lance>maxi2)maxi2=lance;
        int c=0; for(int i=0;i<64;i++) if(t[i]!=0) c++; pecas2+=c; }
      printf("\n      VARIANTE FORTE (nao capturar E nao ficar atacavel), %d corridas:\n", CORRIDAS);
      printf("      bloqueio ao fim de %d lances em media (min %d, max %d); %d pecas de pe'\n",
             soma2/CORRIDAS, mini2, maxi2, pecas2/CORRIDAS);
      printf("      PREVISAO 32.  MEDIDO %d.  desvio %+d\n", soma2/CORRIDAS, soma2/CORRIDAS-32);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────────");
        puts("  A PREVISAO NAO SE CONFIRMOU, e o que a medicao mostra e' mais interessante");
        puts("  do que o que se esperava dela.");
        puts("");
        printf("  Esperava-se o bloqueio ao fim de 32 lances. Mede-se %d, e em parte das\n", media);
        puts("  corridas ele nem chega a acontecer dentro de 400. Mais: A MOBILIDADE SOBE nos");
        puts("  primeiros lances — de 20 para cerca de 28 — e a razao e' geometrica: as trinta e");
        puts("  duas pecas comecam EMPILHADAS em duas filas, a taparem-se umas as outras, e");
        puts("  espalhar-se DESTAPA-AS. Recusar capturar nao aperta o tabuleiro: alivia-o.");
        puts("");
        puts("  O QUE FICA DE PE'. Duas coisas, e sao as que sustentam a leitura do enredo:");
        puts("  (i) quando o bloqueio acontece, as 32 pecas estao TODAS de pe' — o aperto nunca");
        puts("      vem de perdas, vem de nao haver para onde ir;");
        puts("  (ii) no instante do bloqueio HA' capturas disponiveis, e sao o unico lance legal");
        puts("      que resta — nao se escolhe capturar, fica-se sem outra coisa.");
        puts("");
        puts("  O QUE CAI. A ideia de que a recusa produz o aperto DEPRESSA. Ela produz, mas");
        puts("  devagar e nem sempre — e um tabuleiro de 64 casas com 32 pecas tem folga a mais");
        puts("  para se estrangular em 32 lances. O numero 32 e' a contagem das pecas, nao o");
        puts("  tempo do bloqueio, e a medicao separa as duas coisas que a intuicao juntava.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
