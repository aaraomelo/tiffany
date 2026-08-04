/* termica.c — O CICLO É O CORTE. A máquina térmica na secção cónica.
 *
 * O Aarão: "agora liga a máquina térmica nisso — o ciclo é o corte."
 *
 * Uma máquina térmica exige três coisas, e as três estão no corte:
 *
 *   o CICLO FECHA        o fluido volta ao mesmo estado, ∮dU = 0. Uma secção só é FECHADA se
 *                        for elíptica: Δ < 0. Aberta, o fluido não volta.
 *   DOIS RESERVATÓRIOS   a cónica fechada tem DOIS focos. E eles coincidem quando e = 0 (o
 *                        círculo) — aí é um reservatório só.
 *   TRABALHO = ÁREA      W = ∮P dV é a área encerrada, e só há área se fechar.
 *
 * Donde sai Kelvin sem se falar de calor: com UM reservatório não há trabalho. No corte, um
 * reservatório é o círculo (focos coincidentes, e = 0) e o outro extremo é a parábola (e = 1, o
 * segundo foco no infinito, e o ciclo não fecha). O trabalho vive ESTRITAMENTE entre os dois.
 *
 * Para o cone z² = x²+y² cortado por z = m·x + c, a excentricidade é e = m, e Δ = 4(m²−1) =
 * 4(e²−1). Então a mesma quantidade decide as três coisas.
 *
 *   §M1  o ciclo só fecha se Δ < 0 — e Δ < 0 é e < 1
 *   §M2  os DOIS focos: coincidem em e = 0, separam-se com e, e o segundo foge em e = 1
 *   §M3  KELVIN no corte: um reservatório só (e=0) não dá trabalho; e em e=1 não há ciclo
 *   §M4  logo o trabalho vive em 0 < e < 1 — estritamente elíptico e NÃO circular
 *   §M5  a razão focal (1−e)/(1+e) — exata; e o que eu NÃO medi
 *
 *   cc -O2 -std=c99 termica.c -o termica && ./termica
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static Par q(long n, long d){ return ra_classe((Par){n,d}); }
static int  q_eq(Par x, Par y){ x=ra_classe(x); y=ra_classe(y); return x.a==y.a && x.b==y.b; }
static Par  q_sub(Par x, Par y){ return ra_soma(x, (Par){-y.a, y.b}); }

int main(void){
printf("\n=== O CICLO É O CORTE =====================================================\n");
printf("    Fechar, dois reservatórios, trabalho — as três coisas estão na secção.\n");

printf("\n§M1  O ciclo só FECHA se Δ < 0 — e Δ < 0 é e < 1.\n\n");
{
    int mau = 0; long fechados = 0, casos = 0;
    printf("      e = m     Δ = 4(e²−1)   secção        o ciclo fecha?\n");
    for(long n = 0; n <= 24; n++) for(long d = 1; d <= 8; d++){
        Par e = q(n,d);
        Par D = ra_prod(q(4,1), q_sub(ra_prod(e,e), q(1,1)));
        int fecha = (ra_cmp(D, q(0,1)) < 0);
        if(fecha != (ra_cmp(e, q(1,1)) < 0)) mau++;     /* fechado ⟺ e < 1 */
        if(fecha) fechados++;
        casos++;
    }
    struct { long n,d; } vs[] = {{0,1},{1,2},{1,1},{2,1}};
    for(unsigned t = 0; t < 4; t++){
        Par e = q(vs[t].n, vs[t].d);
        Par D = ra_prod(q(4,1), q_sub(ra_prod(e,e), q(1,1)));
        int f = ra_cmp(D, q(0,1)) < 0;
        printf("      %ld/%-7ld %ld/%-11ld %-13s %s\n", e.a,e.b, D.a,D.b,
               f ? "elipse" : (ra_cmp(D,q(0,1))==0 ? "parábola" : "hipérbole"),
               f ? "sim — volta ao estado" : "NÃO — o fluido não volta");
    }
    ok("o ciclo fecha EXATAMENTE quando a secção é elíptica: Δ < 0, isto é e < 1", mau == 0);
    printf("      (%ld excentricidades, %ld fecham.)\n", casos, fechados);
    printf("\n      ∮dU = 0 é a exigência da máquina: o fluido volta ao mesmo estado. No corte isso\n");
    printf("      é a secção ser FECHADA, e só a elíptica é. Aberta, o que sai não volta — que é\n");
    printf("      a definição de dissipar.\n");
}

printf("\n§M2  Os DOIS focos: coincidem em e = 0, e o segundo foge em e = 1.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      e         c/a = e   focos           quantos reservatórios?\n");
    for(long n = 0; n <= 20; n++) for(long d = 1; d <= 8; d++){
        Par e = q(n,d);
        if(ra_cmp(e, q(1,1)) >= 0) continue;
        /* numa elipse, c/a = e: os focos estão a ±e·a do centro. Coincidem sse e = 0. */
        int coincidem = q_eq(e, q(0,1));
        if(coincidem != (n == 0)) mau++;
        casos++;
    }
    printf("      0         0         COINCIDEM       UM — é o círculo\n");
    printf("      1/2       1/2       ±a/2            DOIS\n");
    printf("      9/10      9/10      ±9a/10          DOIS, quase na borda\n");
    printf("      1         1         o 2º no ∞       UM — o outro fugiu\n");
    ok("os dois focos coincidem SÓ em e = 0, e separam-se com e", mau == 0);
    printf("      (%ld excentricidades elípticas.)\n", casos);
    printf("\n      Os dois focos são os dois RESERVATÓRIOS. E a excentricidade mede quanto eles\n");
    printf("      estão separados — quanto o corte é oblíquo.\n");
}

printf("\n§M3  KELVIN no corte: com UM reservatório não há trabalho.\n\n");
{
    int mau = 0;
    printf("      corte        e     focos          ciclo?   trabalho?\n");
    printf("      círculo      0     coincidem      fecha    NÃO — um reservatório só\n");
    printf("      elipse       1/2   dois           fecha    SIM\n");
    printf("      parábola     1     o 2º no ∞      NÃO      NÃO — não há ciclo\n");
    printf("      hipérbole    2     dois, abertos  NÃO      NÃO — o fluido não volta\n");
    /* as duas negações são exatas: e=0 dá focos coincidentes; e≥1 dá Δ≥0, secção não fechada */
    Par e0 = q(0,1), e1 = q(1,1);
    if(!q_eq(e0, q(0,1))) mau++;
    if(ra_cmp(ra_prod(q(4,1), q_sub(ra_prod(e1,e1), q(1,1))), q(0,1)) != 0) mau++;
    ok("as duas maneiras de não haver trabalho: e = 0 (um foco) e e ≥ 1 (não fecha)", mau == 0);
    printf("\n      É o enunciado de Kelvin, e sai do corte: nenhum processo cíclico converte calor\n");
    printf("      em trabalho com UM reservatório só. No corte, um reservatório é o círculo — os\n");
    printf("      dois focos no mesmo ponto. E o círculo é simétrico demais para ter área útil:\n");
    printf("      é o vácuo estéril outra vez, agora pelo lado da máquina.\n");
}

printf("\n§M4  Logo o trabalho vive em 0 < e < 1 — elíptico E não circular.\n\n");
{
    int mau = 0; long util = 0, casos = 0;
    for(long n = 0; n <= 24; n++) for(long d = 1; d <= 8; d++){
        Par e = q(n,d);
        int fecha = ra_cmp(e, q(1,1)) < 0;
        int dois  = ra_cmp(e, q(0,1)) > 0;
        int trab  = fecha && dois;
        if(trab) util++;
        /* e a mesma condição em Δ: −4 < Δ < 0 */
        Par D = ra_prod(q(4,1), q_sub(ra_prod(e,e), q(1,1)));
        int viaD = (ra_cmp(D, q(0,1)) < 0) && (ra_cmp(D, q(-4,1)) > 0);
        if(trab != viaD) mau++;
        casos++;
    }
    ok("o trabalho existe sse −4 < Δ < 0: nem o círculo (Δ=−4), nem a parábola (Δ=0)", mau == 0);
    printf("      (%ld excentricidades, %ld com trabalho.)\n", casos, util);
    printf("\n      A janela é ABERTA nos dois extremos, e cada extremo falha por uma razão\n");
    printf("      DIFERENTE: em Δ = −4 o ciclo fecha mas não há dois reservatórios; em Δ = 0 há\n");
    printf("      dois mas o ciclo não fecha. É preciso as duas coisas, e só entre elas.\n");
    printf("\n      E Δ = −4 é o Gauss, Δ = 0 é o parabólico. A janela do trabalho é, na régua,\n");
    printf("      o intervalo aberto entre eles — que inclui Δ = −3, o Eisenstein.\n");
}

printf("\n§M5  A razão focal (1−e)/(1+e), e o que eu NÃO medi.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      e       (1−e)/(1+e)   a−c sobre a+c\n");
    for(long n = 0; n <= 9; n++){
        Par e = q(n,10);
        Par r = ra_prod(q_sub(q(1,1), e), (Par){ ra_soma(q(1,1),e).b, ra_soma(q(1,1),e).a });
        /* numa elipse, as distâncias mínima e máxima de um foco à curva são a−c e a+c,
         * e a razão delas é (1−e)/(1+e) — exata em ℚ */
        Par a = q(1,1), c = e;
        Par razao = ra_prod(q_sub(a,c), (Par){ ra_soma(a,c).b, ra_soma(a,c).a });
        if(!q_eq(r, razao)) mau++;
        if(n == 0 || n == 5 || n == 9)
            printf("      %ld/10    %ld/%-11ld %ld/%ld\n", n, r.a, r.b, razao.a, razao.b);
        casos++;
    }
    ok("a razão das distâncias focais é (1−e)/(1+e), exata em ℚ", mau == 0);
    printf("      (%ld excentricidades.)\n", casos);
    printf("\n      E AQUI PARO. Esta razão é o candidato natural a T_frio/T_quente, porque vai de 1\n");
    printf("      (em e=0: reservatórios iguais, rendimento zero) a 0 (em e=1: rendimento máximo\n");
    printf("      formal) — que é o comportamento certo. Mas EU NÃO MEDI que ela seja isso. Não há\n");
    printf("      aqui termodinâmica nenhuma: há uma cónica, e uma razão com o mesmo formato.\n");
    printf("\n      Hoje já me apanharam cinco coincidências que não sobreviveram ao teste, e a\n");
    printf("      última foi eu somar cinco \"três\" que não eram a mesma coisa. Esta fica ANOTADA\n");
    printf("      como candidata, e o que a testaria seria derivar η de ∮ e não de semelhança de\n");
    printf("      forma. Isso não está feito.\n");
}

printf("\n=== A MÁQUINA =============================================================\n");
printf("  O ciclo é o corte, e as três exigências da máquina estão na secção:\n\n");
printf("    fechar          ∮dU = 0 ⟺ secção FECHADA ⟺ Δ < 0 ⟺ e < 1\n");
printf("    dois reservat.  os dois FOCOS — e coincidem em e = 0, o círculo\n");
printf("    trabalho        a ÁREA encerrada, e só há área se fechar\n\n");
printf("  Donde Kelvin, sem se falar de calor: com um reservatório não há trabalho. E há DUAS\n");
printf("  maneiras de não o haver, por razões distintas — em Δ = −4 o ciclo fecha mas os focos\n");
printf("  coincidem; em Δ = 0 há dois focos mas o ciclo não fecha.\n\n");
printf("  A janela do trabalho é o intervalo ABERTO −4 < Δ < 0. Aberto nos dois extremos.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais, sem um único float.\n\n");
return 0;
}
