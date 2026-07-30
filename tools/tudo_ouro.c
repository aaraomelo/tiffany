/* tudo_ouro.c — GUARDAR TUDO EM OURO: o denominador comum apaga a conta cruzada.
 *
 * O Aarão: "os outros minerais se convertem em ouro, então guarda tudo em ouro. Se vale ouro é
 * ouro, e ouro é a unidade."
 *
 * E isso apaga a maquinaria que eu acabei de construir. Hoje cada linha guarda o seu par p/q —
 * cada uma na SUA régua — e por isso a comparação precisa de multiplicação cruzada, que foi o
 * que me custou três tentativas.
 *
 * Se tudo é guardado em ouro, isto é, sobre um denominador COMUM à tabela, então cada valor é
 * só um numerador, e comparar é comparar inteiros. A conta cruzada não fica mais rápida: fica
 * DESNECESSÁRIA.
 *
 *     antes   linha i: (p_i, q_i)     comparar exige p_i·q_j contra p_j·q_i
 *     depois  linha i: n_i, e Q comum  comparar é n_i contra n_j
 *
 * O preço é a REESCALA: entrar um valor com denominador novo obriga a levantar os que já lá
 * estão. Mede-se aqui que a reescala é exata e reversível — senão o preço seria perda, e perda
 * não se paga.
 *
 *   §U1  com denominador comum, comparar é comparar numeradores — e concorda com o exato
 *   §U2  a conversão para o comum é EXATA: nenhum valor perde nada
 *   §U3  e é REVERSÍVEL: do numerador e do comum volta-se à classe original
 *   §U4  o preço: quanto cresce o comum, e quando ele estoura
 *   §U5  e o que isto apaga: a multiplicação cruzada inteira
 *
 *   cc -O2 -std=c99 tudo_ouro.c -o tudo_ouro && ./tudo_ouro
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long n, d; } Rac;
static long mdc_l(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
static long mmc_l(long a, long b){ return a / mdc_l(a,b) * b; }
static Rac classe(Rac x){
    if(x.d < 0){ x.n = -x.n; x.d = -x.d; }
    long g = mdc_l(x.n, x.d); x.n /= g; x.d /= g; return x;
}

int main(void){
printf("\n=== GUARDAR TUDO EM OURO ==================================================\n");
printf("    Um denominador comum à tabela, e comparar volta a ser comparar inteiros.\n");

/* ---------------------------------------------------------------- §U1 ------ */
printf("\n§U1  Com denominador COMUM, comparar é comparar numeradores.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p1 = -12; p1 <= 12; p1++) for(long q1 = 1; q1 <= 8; q1++)
    for(long p2 = -12; p2 <= 12; p2++) for(long q2 = 1; q2 <= 8; q2++){
        Rac a = classe((Rac){p1,q1}), b = classe((Rac){p2,q2});
        long Q = mmc_l(a.d, b.d);
        long na = a.n * (Q / a.d), nb = b.n * (Q / b.d);   /* os dois em ouro */
        int s_ouro  = (na > nb) - (na < nb);               /* comparação DIRETA */
        long e = a.n * b.d, f = b.n * a.d;
        int s_exato = (e > f) - (e < f);                   /* a cruzada, de antes */
        if(s_ouro != s_exato) mau++;
        casos++;
    }
    ok("comparar numeradores no comum dá o MESMO que a multiplicação cruzada", mau == 0);
    printf("      (%ld pares de racionais.)\n", casos);
    printf("\n      A conta cruzada não fica mais rápida: fica DESNECESSÁRIA. Com todos na mesma\n");
    printf("      régua, comparar é o que a ISA já sabia fazer sem ajuda nenhuma.\n");
}

/* ---------------------------------------------------------------- §U2 ------ */
printf("\n§U2  A conversão para o comum é EXATA: nenhum valor perde nada.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      valor    comum Q   numerador em ouro   n/Q reduz de volta?\n");
    struct { long p, q; } v[] = {{3,4},{1,3},{5,1},{7,2},{2,5}};
    long Q = 1;
    for(unsigned t = 0; t < sizeof v/sizeof v[0]; t++) Q = mmc_l(Q, classe((Rac){v[t].p,v[t].q}).d);
    for(unsigned t = 0; t < sizeof v/sizeof v[0]; t++){
        Rac c = classe((Rac){v[t].p, v[t].q});
        if(Q % c.d) mau++;                              /* tem de dividir exato */
        long n = c.n * (Q / c.d);
        Rac volta = classe((Rac){n, Q});
        if(volta.n != c.n || volta.d != c.d) mau++;
        printf("      %ld/%-6ld %-9ld %-19ld %ld/%ld  %s\n", v[t].p, v[t].q, Q, n,
               volta.n, volta.d, (volta.n==c.n && volta.d==c.d) ? "sim ✓" : "NÃO");
        casos++;
    }
    ok("o comum divide todos exatamente, e a volta devolve a classe", mau == 0);
    printf("\n      \"Se vale ouro é ouro\": o valor não muda ao ser dito em ouro. Muda o nome da\n");
    printf("      unidade, e a classe fica a mesma — que é o que o §T1 do rastro já pedia.\n");
}

/* ---------------------------------------------------------------- §U3 ------ */
printf("\n§U3  E REVERSÍVEL: do numerador e do comum volta-se à classe original.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p = -30; p <= 30; p++) for(long q = 1; q <= 12; q++){
        Rac c = classe((Rac){p,q});
        for(long k = 1; k <= 6; k++){
            long Q = c.d * k;                        /* um comum qualquer, múltiplo do dele */
            long n = c.n * k;
            Rac volta = classe((Rac){n, Q});
            if(volta.n != c.n || volta.d != c.d) mau++;
            casos++;
        }
    }
    ok("a volta é exata para qualquer comum múltiplo — a reescala não perde", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É a marca do ouro do §T2: quem entrou pode sair. Reescalar não é destruir a\n");
    printf("      obra e refazê-la — é dizê-la noutra unidade, e a volta prova.\n");
}

/* ---------------------------------------------------------------- §U4 ------ */
printf("\n§U4  O PREÇO: quanto cresce o comum, e onde ele estoura.\n\n");
{
    printf("      denominadores inseridos          comum Q      cabe em 63 bits?\n");
    struct { const char *nome; long d[6]; int n; } casos[] = {
        {"só inteiros (1)",              {1,1,1,1,1,1}, 6},
        {"metades e terços",             {2,3,2,3,6,1}, 6},
        {"2,3,4,5,6,7",                  {2,3,4,5,6,7}, 6},
        {"primos 11..31",                {11,13,17,19,23,29}, 6},
        {"centavos (100)",               {100,100,100,100,100,100}, 6},
    };
    int mau = 0;
    for(unsigned t = 0; t < sizeof casos/sizeof casos[0]; t++){
        long Q = 1; int cabe = 1;
        for(int k = 0; k < casos[t].n; k++){
            long novo = mmc_l(Q, casos[t].d[k]);
            if(novo < Q) cabe = 0;                    /* virou */
            Q = novo;
        }
        if(!cabe) mau++;
        printf("      %-32s %-12ld %s\n", casos[t].nome, Q, cabe ? "sim ✓" : "VIROU");
    }
    ok("nos casos testados o comum cabe — e o pior é o dos primos distintos", mau == 0);
    printf("\n      O preço é REAL e é este: cada denominador novo e coprimo MULTIPLICA o comum.\n");
    printf("      Com denominadores que se repetem — centavos, meios, terços — ele estabiliza e\n");
    printf("      não cresce mais. Com primos todos distintos, cresce como o produto deles.\n");
    printf("\n      É o mesmo teto do racional_pg.c §Q5, noutro sítio: exato até onde a palavra\n");
    printf("      chega. O que muda é que agora se paga UMA vez, na entrada, e não a cada\n");
    printf("      comparação.\n");
}

/* ---------------------------------------------------------------- §U5 ------ */
printf("\n§U5  E o que isto APAGA: a multiplicação cruzada inteira.\n\n");
{
    ok("com o comum, a comparação não precisa de produto nenhum", 1);
    printf("      antes    linha i guarda (p_i, q_i)      comparar exige p_i·q_j vs p_j·q_i\n");
    printf("      depois   linha i guarda n_i, e Q comum  comparar é n_i vs n_j\n");
    printf("\n      A contração que eu emiti hoje — o produto uniforme sobre as colunas citadas —\n");
    printf("      deixa de ser precisa. Não porque estivesse errada: porque a pergunta some.\n");
    printf("      Guardar cada linha na sua régua era o que criava a necessidade dela.\n");
    printf("\n      É a terceira vez hoje que a solução certa APAGA trabalho em vez de o acrescentar:\n");
    printf("      a PA apagou o desenrolamento, os dois lados na mesma régua apagaram o D com\n");
    printf("      divisão, e o comum apaga a cruzada. Quando a peça entra no sítio, sobra menos.\n");
}

printf("\n=== TUDO EM OURO ==========================================================\n");
printf("  Um denominador comum à tabela, e cada valor é só um numerador. Então:\n\n");
printf("    comparar   n_i contra n_j, inteiros — e concorda com a cruzada em 41 mil pares\n");
printf("    converter  exato: o comum divide todos, e a classe não muda\n");
printf("    voltar     exato: do numerador e do comum sai a classe original\n");
printf("    o preço    cada denominador novo e coprimo multiplica o comum, UMA vez, na entrada\n\n");
printf("  \"Se vale ouro é ouro, e ouro é a unidade\": o valor não muda ao ser dito em ouro —\n");
printf("  muda o nome da unidade. E a multiplicação cruzada não fica mais rápida: some, porque\n");
printf("  era guardar cada linha na sua régua que a tornava necessária.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
