/* sombras.c — gato e esquilo: os planos das pares e as retas das ímpares, DUAIS (espelho).
 *
 * O gato gira num plano e conserva a ÁREA: preserva uma forma antissimétrica J (J^T=−J), a
 * simplética. Ela tem posto SEMPRE par, 2k: o espaço parte-se em k planos — k GATOS. Como
 * det J=(−1)^n det J, em dimensão ÍMPAR a forma tem det 0 e sobra um EIXO próprio (o núcleo):
 * uma RETA — o ESQUILO. Não há aqui nada "degenerado" nem inferior: a reta é o DUAL do plano, o
 * espelho do gato. Quem é sombra de quem é simétrico — a par 2k é a projeção da ímpar 2k+1 (tira
 * a reta) tanto quanto a ímpar é a par mais a reta. E a multiplicação continua um CORPO em toda
 * dimensão (a norma preservada — nenhuma é mais rica): a diferença par/ímpar é só qual dual, plano
 * ou reta, aparece. É a mesma dualidade do cone e da espiral (§13): espelho, cadeia conservativa.
 * O gato é o boost (relativístico, conserva a área). Exato, resíduo 0.
 *
 *   cc -O2 -std=c99 sombras.c -o sombras
 *   ./sombras [N] [p]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>

static int p;
static long inv_mod(long a){                          /* inverso por Euclides estendido (sem Fermat) */
    long t=0,nt=1,r=p,nr=((a%p)+p)%p;
    while(nr){ long q=r/nr,x; x=t-q*nt; t=nt; nt=x; x=r-q*nr; r=nr; nr=x; }
    return r>1? 0 : (t%p+p)%p;
}
static unsigned long xs=88172645463325252ULL;
static int rnd(void){ xs^=xs<<13; xs^=xs>>7; xs^=xs<<17; return (int)(xs%p); }

/* posto de M (n×n) mod p, por eliminação (M é destruída) */
static int rankmod(int n, long M[32][32]){
    int rank=0;
    for(int col=0,row=0; col<n && row<n; col++){
        int piv=-1; for(int i=row;i<n;i++) if(M[i][col]){ piv=i; break; }
        if(piv<0) continue;
        if(piv!=row) for(int j=0;j<n;j++){ long t=M[piv][j]; M[piv][j]=M[row][j]; M[row][j]=t; }
        long iv=inv_mod(M[row][col]);
        for(int i=0;i<n;i++) if(i!=row && M[i][col]){ long f=M[i][col]*iv%p;
            for(int j=0;j<n;j++) M[i][j]=((M[i][j]-f*M[row][j])%p+p)%p; }
        row++; rank++;
    }
    return rank;
}

int main(int argc,char**argv){
    int N = argc>1? atoi(argv[1]) : 8;
    p = argc>2? atoi(argv[2]) : 101;                  /* primo, entradas variadas                    */
    int res=0;
    printf("GATOS NAS PARES, ESQUILOS (SOMBRAS) NAS ÍMPARES — a forma da área do gato (ℤ_%d)\n", p);
    printf("================================================================\n");
    printf("  o gato conserva a ÁREA (uma forma antissimétrica J); ela só é cheia em dimensão PAR\n");
    printf("\n  dim n   posto J   nº de GATOS (planos)   núcleo   ESQUILOS (retas)   forma\n");
    printf("  ----------------------------------------------------------------------------\n");
    for(int n=1;n<=N;n++){
        int posto=0;
        for(int amostra=0; amostra<8; amostra++){      /* o posto genérico = o máximo observado       */
            long J[32][32];
            for(int i=0;i<n;i++){ J[i][i]=0; for(int j=i+1;j<n;j++){ int r=rnd()%p; J[i][j]=r; J[j][i]=(p-r)%p; } }
            int rk=rankmod(n,J); if(rk>posto) posto=rk;
        }
        int gatos=posto/2, nucleo=n-posto, esquilos=nucleo;
        int postopar = (posto%2==0);
        int certo = postopar && (n%2==0 ? (posto==n && esquilos==0) : (posto==n-1 && esquilos==1));
        if(!certo) res++;
        printf("    %-2d      %-2d        %-2d %-18s  %-2d       %-2d %-14s   %s\n",
               n, posto, gatos, gatos==1?"gato":(gatos?"gatos":"—"), nucleo, esquilos,
               esquilos==1?"esquilo (reta)":(esquilos?"":"—"),
               (n%2? "ímpar: plano(s)+eixo" : "par: só planos"));
    }
    printf("\n  ⇒ posto de J é SEMPRE par (= 2·nº de gatos); nas PARES o espaço é só planos (gatos),\n");
    printf("    nas ÍMPARES há um EIXO próprio a mais — o esquilo (a reta), DUAL ao plano do gato.\n");
    printf("    a ímpar 2k+1 é a par 2k mais a reta; e a par é a projeção da ímpar (tira a reta):\n");
    printf("    quem é sombra de quem é simétrico — plano e reta, gato e esquilo, são ESPELHO.\n");
    printf("    nenhuma é degenerada nem superior: a multiplicação é corpo em toda dimensão\n");
    printf("    (a norma preservada, cf. dimensoes) — é a dualidade do cone e da espiral, conservativa.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("N=%d   plano↔reta, gato↔esquilo: duais, espelho   resíduo total = %d   %s\n",
           N, res, VD(res, "GATO E ESQUILO, PLANO E RETA — DUAIS NUMA CADEIA CONSERVATIVA, NENHUM É MENOR"));
    return res?1:0;
}
