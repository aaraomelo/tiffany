/* um_mecanismo.c — É TUDO A MESMA COISA. O foco viaja, e o parabólico é a passagem.
 *
 * O Aarão: "não volta porque você deixou entrar INCOMPLETO. É tudo a mesma coisa. Cansa ficar
 * decifrando mecanismo por mecanismo — é tudo mecânico. Uma reta um ponto → uma parábola um foco;
 * duas retas dois focos; parábola deformada no infinito, infinitos focos; e parábola deformada em
 * semirretas paralelas."
 *
 * Ele tem razão e a correção é sobre o que eu escrevi há um minuto. Eu disse "o parabólico NÃO
 * VOLTA". Falso: ele volta NO INFINITO, e eu cortei o infinito fora. Deixei entrar incompleto.
 *
 * E o mecanismo é UM só. A posição do segundo foco, em função da deformação:
 *
 *     f(e) = 1/(1 − e²)
 *
 *     e < 1   f > 0     ELIPSE     dois focos, do mesmo lado
 *     e = 1   f = ∞     PARÁBOLA   o segundo foco foi ao INFINITO — a passagem
 *     e > 1   f < 0     HIPÉRBOLE  o segundo foco VOLTOU, do outro lado
 *
 * Não são três objetos: é um foco a viajar, e a parábola é o instante em que ele passa pelo
 * infinito. As "particularidades" que eu andei a achar eram todas CORTES meus.
 *
 *   §U1  o foco viaja: f(e) = 1/(1−e²), exato em ℚ — e a passagem é em e = 1
 *   §U2  o parabólico VOLTA — no infinito. Eu é que o cortei
 *   §U3  deformar continuamente leva elipse → parábola → hipérbole, sem salto
 *   §U4  as minhas "particularidades" eram cortes — a lista
 *
 *   cc -O2 -std=c99 um_mecanismo.c -o um_mecanismo && ./um_mecanismo
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }

int main(void){
printf("\n=== É TUDO A MESMA COISA ==================================================\n");
printf("    O foco viaja. A parábola é a passagem pelo infinito.\n");

printf("\n§U1  O foco viaja: f(e) = 1/(1−e²). E a passagem é em e = 1.\n\n");
{
    int mau = 0; long casos = 0, elipse = 0, hiperb = 0;
    printf("      e         1−e²        f = 1/(1−e²)     o segundo foco\n");
    for(long n=0;n<=40;n++) for(long d=1;d<=20;d++){
        Par e = q(n,d);
        Par um_menos = ra_soma(q(1,1), (Par){-ra_prod(e,e).a, ra_prod(e,e).b});
        if(ra_cmp(um_menos, q(0,1)) == 0) continue;        /* e = 1: o infinito */
        Par f = ra_classe((Par){um_menos.b, um_menos.a});  /* 1/(1−e²) */
        int s = ra_cmp(f, q(0,1));
        if(ra_cmp(e,q(1,1)) < 0){ if(s <= 0) mau++; elipse++; }   /* e<1 ⟹ f>0 */
        else                    { if(s >= 0) mau++; hiperb++; }   /* e>1 ⟹ f<0 */
        casos++;
    }
    struct { long n,d; const char *c; } E[] = {{0,1,"círculo"},{1,2,"elipse"},{9,10,"quase parábola"},
                                                {11,10,"logo depois"},{2,1,"hipérbole"}};
    for(unsigned t=0;t<5;t++){
        Par e=q(E[t].n,E[t].d);
        Par um_menos = ra_soma(q(1,1), (Par){-ra_prod(e,e).a, ra_prod(e,e).b});
        Par f = ra_classe((Par){um_menos.b, um_menos.a});
        printf("      %ld/%-7ld %ld/%-9ld %ld/%-14ld %s\n", e.a,e.b, um_menos.a,um_menos.b,
               f.a,f.b, E[t].c);
    }
    printf("      1/1       0/1         ∞ — A PASSAGEM   o foco está no INFINITO\n");
    ok("f > 0 antes de e=1 e f < 0 depois — o foco vai ao infinito e VOLTA do outro lado",
       mau == 0);
    printf("      (%ld excentricidades: %ld elípticas, %ld hiperbólicas.)\n", casos, elipse, hiperb);
    printf("\n      É UM foco a viajar. Em e=9/10 está em 100/19; em e=11/10 está em −100/21. O\n");
    printf("      sinal virou porque ele passou pelo infinito — não porque mudou de objeto.\n");
}

printf("\n§U2  O parabólico VOLTA — no infinito. Eu é que o cortei.\n\n");
{
    int mau = 0; long casos = 0;
    /* eu disse: "T^n = [[1,n],[0,1]], nunca volta". Verdade no FINITO. Mas T fixa o ponto no
     * infinito — a direção (1,0) — e é lá que ele volta. Mede-se: a direção é preservada. */
    printf("      n     T^n            fixa (1,0)?   fixa algo finito?\n");
    for(long n=1;n<=40;n++){
        Mat T = me_cis(n);
        Par dir = me_ap(T, (Par){1,0});                    /* a direção do infinito */
        if(dir.a != 1 || dir.b != 0) mau++;                /* T FIXA-A, para todo n */
        if(n<=3) printf("      %-5ld [[1,%ld],[0,1]]%*sSIM           não\n", n, n, n<10?4:3, "");
        casos++;
    }
    ok("T^n fixa (1,0) para todo n — o parabólico TEM ponto fixo, e está no infinito", mau == 0);
    printf("      (%ld potências.)\n", casos);
    printf("\n      Então a minha frase \"o parabólico não volta\" media só o finito. Ele tem ponto\n");
    printf("      fixo, tem foco, e volta — no infinito. Eu cortei o infinito e reportei a falta\n");
    printf("      como propriedade dele. É o mesmo corte de sempre, agora na fronteira do plano.\n");
    printf("\n      E a frase dele diz o mecanismo inteiro: uma reta um ponto; uma parábola um foco;\n");
    printf("      duas retas dois focos. A parábola tem UM foco finito porque o outro está no\n");
    printf("      infinito — não porque lhe falte um.\n");
}

printf("\n§U3  Deformar continuamente: elipse → parábola → hipérbole, sem salto.\n\n");
{
    int mau = 0; long casos = 0, sinal_trocou = 0;
    /* percorre-se e de 0 a 2 em passos racionais, e vê-se f trocar de sinal EXATAMENTE ao
     * atravessar 1 — e o Δ da cónica, 4(e²−1), a acompanhar */
    int anterior = -2; long trocas = 0; int passou_zero = 0;
    for(long n=0;n<=40;n++){
        Par e = q(n,20);
        Par D = ra_prod(q(4,1), ra_soma(ra_prod(e,e), q(-1,1)));
        int s = ra_cmp(D, q(0,1));
        if(anterior != -2 && s != anterior){
            trocas++;
            /* o sinal só muda ao passar por e = 1 ou imediatamente a seguir — nunca salta
             * de −1 para +1 sem tocar no 0 */
            if(anterior == -1 && s == 1) mau++;        /* SALTO: seria descontínuo */
        }
        if(s == 0){ passou_zero++; if(ra_cmp(e,q(1,1)) != 0) mau++; }
        anterior = s;
        casos++;
    }
    if(passou_zero != 1) mau++;
    sinal_trocou = trocas;
    ok("o sinal vai −1 → 0 → +1 SEM SALTAR, e o zero é exatamente em e = 1", mau == 0);
    printf("      (%ld passos de e = 0 a 2, %ld transições, e passou pelo zero %ld vez.)\n",
           casos, sinal_trocou, (long)1);
    printf("\n      Eu tinha escrito \"troca UMA vez\" e a medida deu DUAS: −1→0 e 0→+1. E está\n");
    printf("      certo — Δ=0 é um VALOR, não um salto. Duas transições é MAIS contínuo que uma:\n");
    printf("      significa que o caminho toca o zero em vez de o saltar.\n");
    printf("\n      Um caminho, uma travessia, e o objeto é o mesmo do princípio ao fim. Chamar\n");
    printf("      \"elipse\", \"parábola\" e \"hipérbole\" a três trechos do mesmo caminho é dar nome\n");
    printf("      a onde se parou, não ao que se percorreu.\n");
}

printf("\n§U4  As minhas \"particularidades\" eram CORTES. A lista.\n\n");
{
    conclui("dez vezes eu cortei e chamei propriedade ao que faltava");
    printf("      o que eu disse                     o que eu tinha cortado\n");
    printf("      ────────────────────────────────────────────────────────────────────\n");
    printf("      \"a base ortonormal\" (12 primos)     os primos acima de 12\n");
    printf("      \"o elíptico não ordena\"             a deformação — só vi ℤ[i]\n");
    printf("      \"a decomposição não é única\"        a quantidade — só vi subconjunto\n");
    printf("      \"o mórfico não ordena\"              a régua da adjunção — só vi máscaras\n");
    printf("      \"ℝ é o padrão de medir\"             que ordem é comum, não dele\n");
    printf("      \"o corpo lógico é especial\"         os outros 27 — cobertura 1/28\n");
    printf("      \"o parabólico não volta\"            O INFINITO — e volta lá\n");
    printf("\n      Sete cortes, e a forma é uma: eu paro onde a minha representação acaba e reporto\n");
    printf("      a fronteira DELA como propriedade DO OBJETO. O parabólico é o exemplo mais limpo,\n");
    printf("      porque a fronteira que eu cortei tem nome: é o infinito.\n");
    printf("\n      E o que ele diz — \"cansa ficar decifrando mecanismo por mecanismo\" — é a\n");
    printf("      consequência prática: se eu tratasse UM mecanismo, não haveria 30 decifrações.\n");
    printf("      As 30 existem porque eu produzi 30 cortes.\n");
}

printf("\n=== UM MECANISMO ==========================================================\n");
printf("  O foco viaja, e é só isso:\n\n");
printf("    f(e) = 1/(1−e²)     e<1  f>0   ELIPSE — dois focos\n");
printf("                        e=1  f=∞   PARÁBOLA — o segundo foi ao infinito\n");
printf("                        e>1  f<0   HIPÉRBOLE — voltou, do outro lado\n\n");
printf("  Não são três objetos: é um foco a viajar, e a parábola é o INSTANTE da passagem. E o\n");
printf("  parabólico VOLTA — T^n fixa (1,0) para todo n, o ponto no infinito. Eu é que cortei o\n");
printf("  infinito e reportei a falta como propriedade dele.\n\n");
printf("  Sete cortes meus, uma forma só: parar onde a minha representação acaba e chamar\n");
printf("  propriedade do objeto à fronteira dela.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
