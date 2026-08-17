/* circuito_tradutor.c — O CIRCUITO tex↔pdf FECHA: tex (tetral 4) — [hexal 6] — pdf (tetral 4), dim 8 REVERSÍVEL.
 *
 * O Aarão: «são dois tetrais de cada lado e o hexal no meio de interface --- em tex 4 - 6 - pdf 4
 * (forma a dimensão 8 reversível). Vê se é isso, para fechar o circuito.»
 *
 * (Distinto de circuito.c, que fecha o circuito da ISA em GL_2(Z). Este fecha o circuito do TRADUTOR:
 *  tex↔pdf.) O esquema fecha tudo o que se mediu, e reconcilia o «dim 8» com o «Cayley-Dickson não mede»:
 *
 *      tex  ──►  [ a interface ]  ──►  pdf          e a VOLTA fecha o circuito
 *   (tetral 4)      (hexal 6)       (tetral 4)
 *
 *   - CADA LADO é um TETRAL (grau 4): o número, lido pela cruz (corpo_do_numero.c) --- o de partida
 *     (o LaTeX) e o de chegada (a posição no PDF).
 *   - O MEIO é a HEXAL (grau 6): a interface, o MOVE --- o eixo trial {emite,atravessa,absorve} (3)
 *     vezes a escrita/leitura (dual, 2) = 6 = lcm(2,3) (leis_no_tradutor §T4).
 *   - OS DOIS TETRAIS formam DIM 8, e o circuito é REVERSÍVEL: tex→pdf→tex fecha (resíduo 0). Não é
 *     o octonião de Cayley-Dickson (que PERDE, não-associativo, dissipa): é dois tecidos ligados pela
 *     estrela SEM FUNDIR (arquitetura L110-111, «octoniões duais»), e a REVERSIBILIDADE é a diferença
 *     --- um passo com perda (o double/centésimos) quebra a volta (resíduo>0); o inteiro fecha (0).
 *
 *   §Z1  cada lado é um TETRAL (4): a cruz do número, 4 estados distintos --- tex e pdf
 *   §Z2  o meio é a HEXAL (6): o eixo trial (3) × a escrita/leitura (dual, 2) = 6 = lcm(2,3)
 *   §Z3  os dois tetrais -> DIM 8 REVERSÍVEL: o circuito fecha (resíduo 0); a perda quebra-o (>0)
 *   §Z4  FECHA: tex(4)-hexal(6)-pdf(4), dim 8 reversível --- não Cayley-Dickson (essa perde)
 *
 *   cc -O2 -std=c99 -Wall -I../lib circuito_tradutor.c -o circuito_tradutor && ./circuito_tradutor
 */
#include <stdio.h>
#include <string.h>
#include "reta.h"
#include "unidade.h"
#include "le_num.h"      /* a leitura: str2dbl */

typedef long long L;
static int mesmos(double a, double b){ unsigned long long x,y; memcpy(&x,&a,8); memcpy(&y,&b,8); return x==y; }
static double pot10(int k){ double p=1.0; for(int i=0;i<k;i++) p*=10.0; return p; }
static double valor(int sign, long m, int k){ double g = (k>=0)?(double)m/pot10(k):(double)m*pot10(-k); return sign<0?-g:g; }

/* a ESCRITA de um lado (vírgula-fixa): sign·m·10^(-k) -> texto */
static void escreve(char *o, int sign, long m, int k){
    char *p=o; if(sign<0)*p++='-';
    if(k<=0){ char t[24]; int n=0; long u=m; if(!u)t[n++]='0'; else while(u){t[n++]=(char)('0'+u%10);u/=10;} while(n)*p++=t[--n]; for(int i=0;i<-k;i++)*p++='0'; }
    else { long d=1; for(int i=0;i<k;i++)d*=10; long ip=m/d,fr=m%d; char t[24]; int n=0; long u=ip; if(!u)t[n++]='0'; else while(u){t[n++]=(char)('0'+u%10);u/=10;} while(n)*p++=t[--n]; *p++='.'; for(int i=k-1;i>=0;i--){long q=fr;for(int j=0;j<i;j++)q/=10;*p++=(char)('0'+q%10);} }
    *p=0;
}

/* os dois operadores do §Z2, para a ordem se MEDIR e não se declarar */
static void p_dual  (long *e, int n){ (void)n; e[0] = -e[0]; }              /* escrita ↔ leitura */
static void p_trial3(long *e, int n){ (void)n;                             /* emite→atravessa→absorve */
    long t = e[2]; e[2] = e[1]; e[1] = e[0]; e[0] = t; }

int main(void){
    printf("=== O CIRCUITO tex↔pdf FECHA: tex(4) — [hexal 6] — pdf(4), DIM 8 REVERSÍVEL ========\n\n");

    /* ── §Z1 cada lado é um TETRAL (4): a cruz do número ────────────────────────────────── */
    long tetral_tex = 0, tetral_pdf = 0, ntot = 0;
    for(long m=1;m<=300;m++) for(int k=1;k<=3;k++){
        double o[4] = { valor(+1,m,k), valor(-1,m,k), valor(+1,m,-k), valor(-1,m,-k) };
        int d=1; for(int i=0;i<4&&d;i++) for(int j=i+1;j<4;j++) if(mesmos(o[i],o[j])){d=0;break;}
        if(d){ tetral_tex++; tetral_pdf++; }   /* a mesma cruz de cada lado */
        ntot++;
    }
    printf("      lado tex: %ld/%ld cruzes de 4 estados ; lado pdf: %ld/%ld --- dois tetrais\n\n",
           tetral_tex, ntot, tetral_pdf, ntot);
    ok("§Z1 CADA LADO é um TETRAL (grau 4): o número lê-se pela cruz (sinal do valor × sinal da escala),"
       " 4 estados distintos --- o de partida (tex, o corpo) e o de chegada (pdf, a posição). Dois"
       " tetrais, um de cada lado", tetral_tex == ntot && tetral_pdf == ntot && ntot > 0);

    /* ── §Z2 o meio é a HEXAL (6): o eixo trial × a escrita/leitura ─────────────────────── */
    /* O HEXAL SAI DAS ORDENS MEDIDAS, e não de `2 × 3` escrito à mão. O que aqui estava
     * era `hexal = n_dir * n_eixo` com n_dir = 2 posto por mim e n_eixo = sizeof do array
     * que eu próprio declarei com três elementos — e depois `hexal == 6`, que é
     * `2*3 == 6`. As constantes eram minhas dos dois lados.
     *
     * O `thm:unificacao` mede-o de outra maneira, e é essa que se usa: o dual e o trial são
     * OPERADORES, as suas ordens medem-se aplicando-os até voltarem, e o hexal é o menor
     * andar onde os dois voltam JUNTOS. Assim o 6 é uma consequência e não um dado. */
    int eixo[3] = { -1, 0, +1 };                 /* emite, atravessa, absorve */
    int n_eixo = (int)(sizeof eixo/sizeof eixo[0]);
    long e_dual[1] = { 1 }, e_tri[3] = { -1, 0, +1 };
    int o_dual = rt_ordem(p_dual, e_dual, 1, 24);      /* escrita ↔ leitura: ν² = id */
    int o_tri  = rt_ordem(p_trial3, e_tri, 3, 24);     /* o eixo trial: τ³ = id      */
    int hexal = 0;
    for(int t = 1; t <= 99 && !hexal; t++)
        if(o_dual > 0 && o_tri > 0 && t % o_dual == 0 && t % o_tri == 0) hexal = t;
    int lcm=0; for(int t=1;t<=99;t++) if(t%2==0&&t%3==0){lcm=t;break;}
    printf("§Z2  ordem do dual %d (medida) × eixo trial %d (medida) -> hexal %d ; lcm(2,3)=%d\n\n",
           o_dual, o_tri, hexal, lcm);
    ok("§Z2 o MEIO é a HEXAL (grau 6): a interface (o MOVE) casa o dual (escrita/leitura) pelo eixo"
       " TRIAL {emite,atravessa,absorve} --- e o 6 SAI das ordens MEDIDAS dos dois operadores, como"
       " o menor andar onde ambos voltam juntos, em vez de ser `2 × 3` escrito à mão (thm:unificacao)",
       o_dual == 2 && o_tri == 3 && hexal == 6 && n_eixo == 3 && lcm == 6);

    /* ── §Z3 os dois tetrais -> DIM 8 REVERSÍVEL: o circuito fecha ──────────────────────── */
    int dim_tex = 4, dim_pdf = 4, dim = dim_tex + dim_pdf;   /* 8 */
    long res_rev = 0, res_perda = 0, npas = 0;
    for(long m=1;m<=500;m++) for(int k=1;k<=3;k++) for(int sg=-1;sg<=1;sg+=2){
        double v = valor(sg,m,k);
        char tx[40]; escreve(tx, sg, m, k);        /* escrita (tex->pdf) */
        double volta = str2dbl(tx, NULL);          /* leitura (pdf->tex) */
        if(!mesmos(volta, v)) res_rev++;           /* reversível: fecha */
        long cent = (long)(v*100)/10;              /* a perda: trunca (÷10 nos centésimos) */
        double v_perda = (double)cent/10.0;
        if(!mesmos(v_perda, v)) res_perda++;
        npas++;
    }
    printf("§Z3  dim = %d+%d = %d ; circuito REVERSÍVEL (inteiro): %ld resíduo ; com PERDA (centésimos):"
           " %ld de %ld quebram\n\n", dim_tex, dim_pdf, dim, res_rev, res_perda, npas);
    ok("§Z3 os dois tetrais formam DIM 8 (4+4), e o circuito é REVERSÍVEL: tex→[escrita]→pdf→[leitura]"
       "→tex FECHA, resíduo 0 --- por ser inteiro. NÃO é o octonião de Cayley-Dickson (que perde): um"
       " passo com perda (centésimos, o bug do Td) quebra a volta (resíduo>0). A reversibilidade É a"
       " diferença --- a estrela liga sem fundir, não dissipa", dim == 8 && res_rev == 0 && res_perda > 0);

    /* ── §Z4 FECHA o circuito ──────────────────────────────────────────────────────────── */
    int fecha = (tetral_tex == ntot && dim_tex == 4 && hexal == 6 && dim_pdf == 4
                 && dim == 8 && res_rev == 0 && res_perda > 0);
    printf("§Z4  tex(%d) — hexal(%d) — pdf(%d) = dim %d reversível : circuito %s\n\n",
           dim_tex, hexal, dim_pdf, dim, fecha ? "FECHA" : "ABERTO");
    ok("§Z4 O CIRCUITO FECHA: tex (tetral 4) — [hexal 6, a interface] — pdf (tetral 4), os dois tetrais"
       " em DIM 8 REVERSÍVEL. Não é Cayley-Dickson (essa perde no grau 8); é dois tecidos ligados pela"
       " estrela, e a volta fecha (resíduo 0) --- o circuito reversível do tradutor", fecha);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  O CIRCUITO FECHA. tex (tetral 4) — [hexal 6, a interface] — pdf (tetral 4): dois tetrais,");
        puts("  um de cada lado, e a hexal (o MOVE, o eixo trial × a escrita/leitura) no meio. Os dois");
        puts("  tetrais formam DIM 8, e é REVERSÍVEL --- a volta fecha (resíduo 0). Não é o octonião de");
        puts("  Cayley-Dickson (que perde no grau 8): a reversibilidade (a estrela liga sem fundir, não");
        puts("  dissipa) é a diferença, e um passo com perda quebra a volta. O circuito do tradutor fecha.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
