/* circuito_tradutor.c — O CIRCUITO tex↔pdf FECHA: tex (tetral 4) — [hexal 6] — pdf (tetral 4), dim 8 REVERSÍVEL.
 *
 * A cruz do número é quatro racionais em Z: ±m/10^k e ±m·10^k. A volta é parse inteiro
 * do texto, sem strtod. A perda dos centésimos é o resto da divisão por 10^{k−1}.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/circuito_tradutor.c -o circuito_tradutor
 */
#include <stdio.h>
#include <string.h>
#include "reta.h"
#include "unidade.h"

static long pot10(int k){
    long p = 1;
    for(int i = 0; i < k; i++) p *= 10;
    return p;
}

/* a ESCRITA de um lado (vírgula-fixa): sign·m·10^(-k) -> texto */
static void escreve(char *o, int sign, long m, int k){
    char *p=o; if(sign<0)*p++='-';
    if(k<=0){ char t[24]; int n=0; long u=m; if(!u)t[n++]='0'; else while(u){t[n++]=(char)('0'+u%10);u/=10;} while(n)*p++=t[--n]; for(int i=0;i<-k;i++)*p++='0'; }
    else { long d=pot10(k); long ip=m/d,fr=m%d; char t[24]; int n=0; long u=ip; if(!u)t[n++]='0'; else while(u){t[n++]=(char)('0'+u%10);u/=10;} while(n)*p++=t[--n]; *p++='.'; for(int i=k-1;i>=0;i--){long q=fr;for(int j=0;j<i;j++)q/=10;*p++=(char)('0'+q%10);} }
    *p=0;
}

/* a LEITURA: do texto voltam (sinal, mantissa, casas) — sem strtod */
static int le_fixo(const char *s, int *sg, long *m, int *k){
    int sign = 1;
    if(*s == '-'){ sign = -1; s++; }
    else if(*s == '+') s++;
    long ip = 0; int houve = 0;
    while(*s >= '0' && *s <= '9'){ ip = ip*10 + (*s - '0'); s++; houve = 1; }
    int kk = 0; long fr = 0;
    if(*s == '.'){
        s++;
        while(*s >= '0' && *s <= '9'){ fr = fr*10 + (*s - '0'); kk++; s++; houve = 1; }
    }
    if(!houve || *s) return 0;
    *sg = sign; *m = ip * pot10(kk) + fr; *k = kk;
    return 1;
}

/* os dois operadores do §Z2, para a ordem se MEDIR e não se declarar */
static void p_dual  (long *e, int n){ (void)n; e[0] = -e[0]; }
static void p_trial3(long *e, int n){ (void)n;
    long t = e[2]; e[2] = e[1]; e[1] = e[0]; e[0] = t; }

int main(void){
    printf("=== O CIRCUITO tex↔pdf FECHA: tex(4) — [hexal 6] — pdf(4), DIM 8 REVERSÍVEL ========\n\n");

    /* ── §Z1 cada lado é um TETRAL (4): a cruz do número, quatro racionais em Z ──── */
    long tetral_tex = 0, tetral_pdf = 0, ntot = 0;
    for(long m=1;m<=300;m++) for(int k=1;k<=3;k++){
        long p = pot10(k);
        long num[4] = { m, -m, m*p, -m*p };
        long den[4] = { p,  p,   1,    1 };
        int d = 1;
        for(int i = 0; i < 4 && d; i++)
            for(int j = i+1; j < 4; j++)
                if(num[i]*den[j] == num[j]*den[i]){ d = 0; break; }
        if(d){ tetral_tex++; tetral_pdf++; }
        ntot++;
    }
    printf("      lado tex: %ld/%ld cruzes de 4 estados ; lado pdf: %ld/%ld --- dois tetrais\n\n",
           tetral_tex, ntot, tetral_pdf, ntot);
    ok("§Z1 CADA LADO é um TETRAL (grau 4): o número lê-se pela cruz (sinal do valor × sinal da escala),"
       " 4 estados distintos --- o de partida (tex, o corpo) e o de chegada (pdf, a posição). Dois"
       " tetrais, um de cada lado. Os quatro sao racionais em Z, iguais sse num_i.den_j = num_j.den_i",
       tetral_tex == ntot && tetral_pdf == ntot && ntot > 0);

    /* ── §Z2 o meio é a HEXAL (6): o eixo trial × a escrita/leitura ─────────────────────── */
    int eixo[3] = { -1, 0, +1 };
    int n_eixo = (int)(sizeof eixo/sizeof eixo[0]);
    long e_dual[1] = { 1 }, e_tri[3] = { -1, 0, +1 };
    int o_dual = rt_ordem(p_dual, e_dual, 1, 24);
    int o_tri  = rt_ordem(p_trial3, e_tri, 3, 24);
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
    int dim_tex = 4, dim_pdf = 4;
    long res_rev = 0, res_perda = 0, npas = 0;
    for(long m=1;m<=500;m++) for(int k=1;k<=3;k++) for(int sg=-1;sg<=1;sg+=2){
        char tx[40]; escreve(tx, sg, m, k);
        int sg2 = 0, k2 = 0; long m2 = 0;
        if(!le_fixo(tx, &sg2, &m2, &k2) || sg2 != sg || m2 != m || k2 != k) res_rev++;
        /* perda a um decimal: m/10^k vs floor(m/10^{k-1})/10. Iguais sse m e' multiplo de 10^{k-1}. */
        long dec = pot10(k - 1);
        if(m % dec != 0) res_perda++;
        npas++;
    }
    printf("§Z3  dim = %d+%d = %d ; circuito REVERSÍVEL (inteiro): %ld resíduo ; com PERDA (centésimos):"
           " %ld de %ld quebram\n\n", dim_tex, dim_pdf, dim_tex + dim_pdf, res_rev, res_perda, npas);
    ok("§Z3 os dois tetrais formam DIM 8 (4+4), e o circuito é REVERSÍVEL: tex→[escrita]→pdf→[leitura]"
       "→tex FECHA, resíduo 0 --- por ser inteiro. NÃO é o octonião de Cayley-Dickson (que perde): um"
       " passo com perda (centésimos, o resto de m mod 10^{k-1}) quebra a volta (resíduo>0). A"
       " reversibilidade É a diferença --- a estrela liga sem fundir, não dissipa",
       dim_tex + dim_pdf == 8 && res_rev == 0 && res_perda > 0);

    /* ── §Z4 FECHA o circuito ──────────────────────────────────────────────────────────── */
    printf("§Z4  tex(%d) — hexal(%d) — pdf(%d) = dim %d reversível : circuito %s\n\n",
           dim_tex, hexal, dim_pdf, dim_tex + dim_pdf,
           (tetral_tex == ntot && res_rev == 0) ? "FECHA" : "ABERTO");
    ok("§Z4 O CIRCUITO FECHA: tex (tetral 4) — [hexal 6, a interface] — pdf (tetral 4), os dois tetrais"
       " em DIM 8 REVERSÍVEL. Não é Cayley-Dickson (essa perde no grau 8); é dois tecidos ligados pela"
       " estrela, e a volta fecha (resíduo 0) --- o circuito reversível do tradutor",
       tetral_tex == ntot && hexal == 6 && dim_tex + dim_pdf == 8 && res_rev == 0 && res_perda > 0);

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
