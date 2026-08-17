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

printf("\n=== FECHO ==================================================================\n");
printf("    A quadratura estava escrita e nunca tinha corrido: zero ficheiros incluíam\n");
printf("    o lib/calculo2.h. Vinte e uma peças em ℚ — séries com integral, Fubini nas\n");
printf("    duas ordens, Green pelos dois lados — e nenhuma vírgula em sítio nenhum.\n");
printf("    Um integral de um polinómio É um racional, e um racional escreve-se.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
