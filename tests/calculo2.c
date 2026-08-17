/* calculo2.c — A QUADRATURA EM FRACÇÕES DE INTEIROS, e a lib que a tinha e nunca correu.
 *
 * O Aarão: «tirar os doubles todos, todos os pontos flutuantes, tudo vira fração de
 * inteiros, normalizar e aplicar a quadratura».
 *
 * E a quadratura já estava escrita. O `lib/calculo2.h` tem vinte e uma peças — séries
 * formais com coeficientes em ℚ, o produto de Cauchy, a derivada, a INTEGRAL, os integrais
 * duplos nas duas ordens e o teorema de Green pelos dois lados — todas em `Qz`, o racional
 * em `int` de 32 bits, sem uma vírgula em sítio nenhum.
 *
 * NENHUM MEDIDOR A USAVA. Zero ficheiros incluíam `calculo2.h`: a lib estava escrita, o
 * paper citava-a, e ela nunca tinha corrido. É o mesmo defeito que a casa persegue nas
 * asserções — o que não é exercitado não está medido —, um andar acima: uma BIBLIOTECA
 * por estrear. Este ficheiro é o que faltava.
 *
 * E A REGRA QUE GOVERNA TUDO O QUE SE SEGUE: nenhuma vírgula. As entradas são racionais
 * dados como par (p,q), as contas fazem-se em `Qz` com produto cruzado, e as igualdades
 * são IGUALDADES — `qz_igual`, não `fabs(...) < eps`. Um integral de um polinómio é um
 * racional exacto, e a quadratura numérica só existe onde o objecto não é polinomial.
 *
 *   §C1  a INTEGRAL de uma série é exacta, e D∘∫ = id nos coeficientes
 *   §C2  ∫ e a derivada são o par dual — e ∫∘D perde a constante, que é a fibra
 *   §C3  FUBINI: as duas ordens do integral duplo dão o MESMO racional
 *   §C4  GREEN: a borda e a área dão o mesmo — dois caminhos, um teorema
 *   §C5  e o TECTO da máquina, contado: quantas divisões saturaram
 *
 *   cc -O2 -std=c99 -I. -I../lib calculo2.c -o calculo2 && ./calculo2
 */
#include <stdio.h>
#include "dual32.h"
#include "racionais.h"
#include "linear.h"
#include "calculo2.h"
#include "reta.h"      /* RtCf: a palavra, uma sequencia de longs */
#include "unidade.h"

static long c2_sat_tecto = 0;

int main(void){
printf("\n=== A QUADRATURA EM FRACÇÕES DE INTEIROS ==================================\n");
printf("    Nenhuma vírgula: as séries têm coeficientes em ℚ, os integrais são\n");
printf("    racionais exactos, e as igualdades são IGUALDADES.\n");

/* ═══ §C1 — a integral de uma série, coeficiente a coeficiente ═══════════════ */
printf("\n§C1  ∫ de uma série é EXACTA, e D∘∫ = id nos coeficientes.\n\n");
{
    /* A integral formal divide o coeficiente i por (i+1) e desloca — e isso é ℚ, não
     * uma aproximação. Derivar a seguir desfaz exactamente, coeficiente a coeficiente.
     * Mede-se nas três séries que a lib traz, e não numa só: se fosse uma, «funciona»
     * podia ser uma coincidência do caso escolhido. */
    /* E O PRIMEIRO QUE ESTE MEDIDOR ENCONTROU FOI O TECTO. O `Qz` é racional em `int` de
     * 32 bits, e a integral DESLOCA: ∫(xᵏ/k!) dá x^{k+1}/(k+1)!. Com exp de 12 termos o
     * denominador que sai é 13! = 6 227 020 800, e isso NÃO CABE em 32 bits — 12! =
     * 479 001 600 cabe, 13! não. A lib SATUROU E CONTOU, que é o que ela promete: o
     * qz_saturou subiu a 2 em vez de truncar calada.
     *
     * Não é defeito da lib nem da tese — é o tecto da máquina, e mede-se em vez de se
     * evitar. Abaixo dele as três séries fecham; acima, a saturação aparece e é contada. */
    Sr fs[3]; const char *nm[3] = { "geométrica", "exp", "sin" };
    fs[0] = sr_geometrica(); fs[1] = sr_exp(11); fs[2] = sr_sin(11);
    long bate = 0, tot = 0, coefs = 0;
    printf("      série         D(∫f) = f ?   coeficientes comparados\n");
    for(int t = 0; t < 3; t++){
        Sr I = sr_integra(fs[t]);
        Sr DI = sr_deriva(I);
        int igual = 1, n = 0;
        for(int i = 0; i < C2_MAX - 1; i++){
            n++;
            if(!qz_igual(DI.a[i], fs[t].a[i])) igual = 0;
        }
        tot++; coefs += n;
        if(igual) bate++;
        printf("      %-13s %-13s %d\n", nm[t], igual ? "sim" : "NÃO", n);
    }
    printf("\n      %ld de %ld séries com D(∫f) = f em todos os %ld coeficientes\n",
           bate, tot, coefs);
    /* e ONDE ele bate, medido: o primeiro k cuja integral não cabe */
    long sat_antes = qz_saturou, k_estoura = 0;
    for(int k = 8; k <= 14 && !k_estoura; k++){
        long marca = qz_saturou;
        Sr e = sr_exp(k); Sr ie = sr_integra(e); (void)ie;
        if(qz_saturou > marca) k_estoura = k;
    }
    printf("      e o TECTO: a integral de exp com %ld termos é a primeira que não cabe em\n"
           "      `int` de 32 bits — 12! = 479001600 cabe, 13! = 6227020800 não\n\n", k_estoura);
    long sat_do_tecto = qz_saturou - sat_antes;
    c2_sat_tecto = sat_do_tecto;
    ok("A INTEGRAL FORMAL É EXACTA: ∫ divide o coeficiente i por (i+1) e desloca, o que é"
       " uma conta em ℚ e não uma aproximação — e derivar a seguir desfaz-a COEFICIENTE A"
       " COEFICIENTE, nas três séries que a lib traz. A igualdade é qz_igual, por produto"
       " cruzado, e não uma diferença menor que uma régua",
       bate == tot && tot == 3 && coefs > 0 && k_estoura == 12 && sat_do_tecto > 0);
}

/* ═══ §C2 — o par dual: ∫∘D perde a constante ═══════════════════════════════ */
printf("\n§C2  O PAR: D∘∫ = id, mas ∫∘D = id − a constante. A fibra é o termo zero.\n\n");
{
    /* É a assimetria que o dif.c §F5 já nomeia: derivar apaga a constante, e integrar não
     * a pode devolver. Aqui vê-se no COEFICIENTE: ∫(D f) tem o termo zero a ZERO, seja
     * qual for o f — e todos os outros coeficientes voltam. Sem a segunda metade, «o par
     * não é simétrico» era uma frase. */
    Sr f = sr_exp(12);
    Sr ID = sr_integra(sr_deriva(f));
    int zero_perdido = qz_igual(ID.a[0], qz(0,1));
    int resto_volta = 1, n = 0;
    for(int i = 1; i < C2_MAX - 1; i++){ n++; if(!qz_igual(ID.a[i], f.a[i])) resto_volta = 0; }
    printf("      f = eˣ:  f[0] = %d/%d   e   (∫Df)[0] = %d/%d   — a constante NÃO volta\n",
           f.a[0].p, f.a[0].q, ID.a[0].p, ID.a[0].q);
    printf("      e os outros %d coeficientes voltam todos: %s\n\n", n, resto_volta ? "sim" : "NÃO");
    ok("E O PAR NÃO É SIMÉTRICO, medido no coeficiente: D∘∫ devolve tudo, mas ∫∘D põe o"
       " termo ZERO a zero e devolve o resto — a constante que a derivada apaga não tem por"
       " onde voltar. É a fibra sem volta do 0·x = 0, aqui no andar do cálculo, e vê-se"
       " porque o objecto é a SÉRIE e não um valor",
       zero_perdido && resto_volta && !qz_igual(f.a[0], qz(0,1)) && n > 0);
}

/* ═══ §C3 — FUBINI, e os dois caminhos são objectos diferentes ══════════════ */
printf("\n§C3  FUBINI: ∫∫ dy dx = ∫∫ dx dy, e os intermédios são DIFERENTES.\n\n");
{
    /* O que faz disto um teorema e não uma tautologia é o comentário da própria lib: o
     * caminho ∫dy-depois-∫dx colapsa y primeiro e deixa um polinómio em x; o outro colapsa
     * x e deixa um polinómio em y. Os objectos intermédios são DIFERENTES, e a igualdade
     * dos totais é que é Fubini. Varre-se uma família de polinómios, não um caso. */
    long iguais = 0, casos = 0;
    Qz x0 = qz(0,1), x1 = qz(2,1), y0 = qz(-1,1), y1 = qz(3,2);   /* o rectângulo, em ℚ */
    printf("      p(x,y)                    ∫∫dy dx        ∫∫dx dy       igual?\n");
    for(int a = -2; a <= 2; a++) for(int b = -2; b <= 2; b++) for(int c = -2; c <= 2; c++){
        P2 p = p2_0();
        p.c[0][0] = qz(a,1); p.c[1][0] = qz(b,1); p.c[0][1] = qz(c,1);
        p.c[1][1] = qz(a+b,2); p.c[2][0] = qz(c,3);        /* denominadores 2 e 3: ℚ a sério */
        Qz A = p2_int_dy_dx(p, x0, x1, y0, y1);
        Qz B = p2_int_dx_dy(p, x0, x1, y0, y1);
        casos++;
        if(qz_igual(A, B)) iguais++;
        if(a == 1 && b == 1 && c == 1)
            printf("      1 + x + y + x·y + x²/3    %5d/%-8d %5d/%-8d %s\n",
                   A.p, A.q, B.p, B.q, qz_igual(A,B) ? "sim" : "NÃO");
    }
    printf("\n      as duas ordens dão o MESMO racional em %ld de %ld polinómios\n\n", iguais, casos);
    ok("FUBINI, e não é uma tautologia: o caminho ∫dy-então-∫dx colapsa y primeiro e deixa"
       " um polinómio em X; o outro colapsa x e deixa um polinómio em Y — os objectos"
       " intermédios são DIFERENTES, e é a igualdade dos totais que é o teorema. Medido em"
       " 125 polinómios com coeficientes racionais de denominador 2 e 3, num rectângulo de"
       " cantos racionais, e o resultado é o MESMO racional — não dois próximos",
       iguais == casos && casos == 125);
}

/* ═══ §C4 — GREEN pelos dois lados ══════════════════════════════════════════ */
printf("\n§C4  GREEN: ∮(P dx + Q dy) = ∫∫(Q_x − P_y), e são dois caminhos.\n\n");
{
    /* A borda percorre QUATRO segmentos e soma quatro integrais de uma variável; a área
     * deriva, subtrai e integra DUAS vezes. Não partilham uma linha de código, e o teorema
     * é terem de bater. E o gume: um campo em que Q_x − P_y é ZERO dá borda zero — e sem
     * um caso onde ela NÃO é zero, «bate» valia por ser 0 = 0. */
    long bate = 0, casos = 0, vivos = 0;
    Qz x0 = qz(0,1), x1 = qz(3,2), y0 = qz(-1,1), y1 = qz(2,1);
    printf("      (P,Q)                        ∮ borda       ∫∫ área      igual?  rot≠0?\n");
    for(int a = -2; a <= 2; a++) for(int b = -2; b <= 2; b++){
        P2 P = p2_0(), Q = p2_0();
        P.c[0][1] = qz(a,1);              /* P = a·y      →  P_y = a      */
        P.c[1][0] = qz(1,2);              /* + x/2        →  não contribui */
        Q.c[1][0] = qz(b,1);              /* Q = b·x      →  Q_x = b      */
        Q.c[0][2] = qz(1,3);              /* + y²/3       →  não contribui */
        Qz B = p2_green_borda(P, Q, x0, x1, y0, y1);
        Qz A = p2_green_area (P, Q, x0, x1, y0, y1);
        casos++;
        if(qz_igual(A, B)) bate++;
        if(b - a != 0) vivos++;                       /* o rotacional NÃO é zero */
        if(a == 1 && b == 2)
            printf("      P=y+x/2, Q=2x+y²/3           %5d/%-8d %5d/%-8d %-6s %s\n",
                   B.p, B.q, A.p, A.q, qz_igual(A,B)?"sim":"NÃO", (b-a)?"sim":"nao");
    }
    printf("\n      a borda e a área dão o MESMO racional em %ld de %ld campos,\n", bate, casos);
    printf("      e em %ld deles o rotacional NÃO é zero — sem esses, «bate» era 0 = 0\n\n", vivos);
    ok("GREEN FECHA PELOS DOIS LADOS, em ℚ e sem uma vírgula: a BORDA soma quatro integrais"
       " de uma variável ao longo dos quatro segmentos, e a ÁREA deriva, subtrai e integra"
       " duas vezes — não partilham uma linha de código. Os dois dão o MESMO racional nos"
       " 25 campos varridos, e em 20 deles o rotacional Q_x − P_y NÃO é zero: sem essa"
       " metade, «bate» valia por 0 = 0",
       bate == casos && casos == 25 && vivos == 20);
}

/* ═══ §C5 — o tecto da máquina, contado ═════════════════════════════════════ */
printf("\n§C5  O TECTO: as divisões que não couberam, contadas à parte.\n\n");
{
    /* O `Qz` é racional em int de 32 bits, e a lib conta o que satura em vez de truncar
     * calado — qz_saturou e c2_estouros. Uma quadratura exacta que estoire em silêncio é
     * pior que uma aproximada que o diga, e é por isso que o número vai aqui. */
    printf("      divisões que saturaram no calculo2:  %ld\n", c2_estouros);
    printf("      saturações do Qz (racional em int):  %ld  — TODAS do §C1, ao procurar\n", qz_saturou);
    printf("      o tecto de propósito; as séries, os 125 duplos e os 25 campos de Green\n");
    printf("      correram sem uma\n\n");
    ok("E O TECTO DIZ-SE: as contas todas acima correram sem uma única saturação — nem nas"
       " séries de doze termos, nem nos 125 integrais duplos, nem nos 25 campos de Green."
       " O racional é em `int` de 32 bits, e a lib conta o que não cabe em vez de truncar"
       " calada. Uma quadratura exacta que estoire em silêncio é pior que uma aproximada"
       " que o diga",
       c2_estouros == 0 && qz_saturou == c2_sat_tecto && c2_sat_tecto > 0);
}

/* ═══ §C7 — exp e log são o PAR, e a composição prova-o em ℚ ═══════════════ */
printf("\n§C7  exp∘log = id, coeficiente a coeficiente — o par sem uma vírgula.\n\n");
{
    /* O Aarão: «a exponencial está na lib e a inversa também, daí tem logaritmo».
     *
     * E o par fecha-se em ℚ, sem avaliar nada: com log(1+x) = Σ (−1)^{n+1}xⁿ/n e
     * eˣ = Σ xⁿ/n!, a composição exp(log(1+x)) tem de dar 1 + x — isto é, o coeficiente
     * 0 vale 1, o coeficiente 1 vale 1, e TODOS os outros valem ZERO. Não é «próximo de
     * zero»: é zero, e mede-se com qz_igual.
     *
     * É a mesma inversão que a reta.h tem do lado inteiro — rt_ipow, rt_raiz_k e
     * rt_log_int são as três perguntas de b^k = n. Ali a inversa é pelo expoente; aqui é
     * pela série. As duas dizem o mesmo par. */
    int N = 8;
    Sr L = sr_log1p(N), E = sr_exp(N);
    Sr comp = sr_compoe(E, L, N);
    long zeros = 0, olhados = 0;
    int c0 = qz_igual(comp.a[0], qz(1,1));
    int c1 = qz_igual(comp.a[1], qz(1,1));
    printf("      exp(log(1+x)) coeficiente a coeficiente:\n");
    printf("        grau 0: %d/%d   grau 1: %d/%d   (têm de ser 1 e 1)\n",
           comp.a[0].p, comp.a[0].q, comp.a[1].p, comp.a[1].q);
    for(int i = 2; i <= N; i++){ olhados++; if(qz_igual(comp.a[i], qz(0,1))) zeros++; }
    printf("        graus 2..%d: %ld de %ld são ZERO exacto\n\n", N, zeros, olhados);
    /* e o GUME: uma série que NÃO é a inversa não compõe na identidade */
    Sr S1 = sr_sin(N);
    Sr errada = sr_compoe(E, S1, N);
    long zeros_err = 0;
    for(int i = 2; i <= N; i++) if(qz_igual(errada.a[i], qz(0,1))) zeros_err++;
    printf("      e o GUME: com sin no lugar do log, só %ld dos %ld graus dão zero —\n",
           zeros_err, olhados);
    printf("      a composição só colapsa na identidade com a INVERSA certa\n\n");
    ok("exp E log SÃO O PAR, e prova-se em ℚ sem avaliar nada: exp(log(1+x)) dá 1 + x"
       " coeficiente a coeficiente — o grau 0 vale 1, o grau 1 vale 1, e todos os outros"
       " são ZERO EXACTO, medido com qz_igual e não com uma régua. É a mesma inversão que"
       " a reta.h tem do lado inteiro (rt_ipow, rt_raiz_k, rt_log_int são as três perguntas"
       " de b^k = n); ali é pelo expoente, aqui é pela série. E o gume: pondo sin no lugar"
       " do log a composição NÃO colapsa",
       c0 && c1 && zeros == olhados && olhados > 0 && zeros_err < olhados);
}

/* ═══ §C6 — E O TECTO NÃO É DO OBJECTO: É DA REPRESENTAÇÃO ═════════════════ */
printf("\n§C6  O TECTO ERA DO PAR (p,q). Na PALAVRA não há tecto: é uma sequência.\n\n");
{
    /* O Aarão: «1e-12 = 10^-12, e você coloca tudo em frações contínuas — não tem porquê
     * não caber, é uma sequência num acumulador long int».
     *
     * E é isso. O §C1 mediu que a integral de e^x com 12 termos não cabe, porque o
     * denominador que sai é 13! = 6 227 020 800 e o `Qz` é um PAR de `int` de 32 bits. Mas
     * o tecto é DA REPRESENTAÇÃO, não do número: a mesma fracção como PALAVRA é
     *
     *      1/13!  =  [0; 6227020800]
     *
     * dois termos, cada um a caber num `long`, e a volta é exacta. A palavra não satura
     * porque não guarda o produto: guarda a DESCIDA, e cada degrau cabe sozinho.
     *
     * E O MESMO VALE PARA OS LIMIARES. `1e-12` não é uma vírgula — é 10^-12, o racional
     * 1/1000000000000, e a sua palavra é [0; 1000000000000]. Um limiar escrito assim
     * deixa de ser uma régua e passa a ser um número do corpo, comparável por produto
     * cruzado como qualquer outro. */
    long f13 = 1; for(int k = 2; k <= 13; k++) f13 *= k;
    RtCf w13; rt_cf_de(1, 1, f13, &w13);
    long p13, q13; int volta13 = rt_cf_para(&w13, &p13, &q13);
    long e12 = 1; for(int k = 0; k < 12; k++) e12 *= 10;
    RtCf w12; rt_cf_de(1, 1, e12, &w12);
    long pe, qe; int volta12 = rt_cf_para(&w12, &pe, &qe);
    printf("      13! = %ld — cabe em int32? %s ; em long? sim\n",
           f13, f13 <= 2147483647L ? "sim" : "NAO");
    printf("      1/13! como PALAVRA: [%ld;%ld]  (%d termos, saturou %d) e a volta dá %ld/%ld\n",
           w13.a[0], w13.a[1], w13.n, w13.saturou, p13, q13);
    printf("      1e-12 = 10^-12 = 1/%ld ; palavra [%ld;%ld] e a volta dá %ld/%ld\n\n",
           e12, w12.a[0], w12.a[1], pe, qe);
    ok("O TECTO ERA DA REPRESENTAÇÃO, NÃO DO OBJECTO. O §C1 mede que a integral de e^x com"
       " doze termos não cabe — 13! = 6227020800 não entra num `int` de 32 bits —, e isso é"
       " verdade do PAR (p,q). A mesma fracção como PALAVRA é [0;6227020800]: dois termos,"
       " cada um a caber num `long`, e a volta é exacta. A palavra não satura porque não"
       " guarda o produto, guarda a DESCIDA, e cada degrau cabe sozinho. E o mesmo vale"
       " para os limiares: 1e-12 é 10^-12, o racional 1/10^12, palavra [0;1000000000000] —"
       " um número do corpo, e não uma régua",
       volta13 && p13 == 1 && q13 == f13 && w13.saturou == 0 && w13.n == 2
       && volta12 && pe == 1 && qe == e12 && w12.saturou == 0
       && f13 > 2147483647L);
}

printf("\n=== FECHO ==================================================================\n");
printf("    A quadratura estava escrita e nunca tinha corrido: zero ficheiros incluíam\n");
printf("    o lib/calculo2.h. Vinte e uma peças em ℚ — séries com integral, Fubini nas\n");
printf("    duas ordens, Green pelos dois lados — e nenhuma vírgula em sítio nenhum.\n");
printf("    Um integral de um polinómio É um racional, e um racional escreve-se.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
