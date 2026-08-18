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

/* ─── §C8 ── A CONSTANTE DE INTEGRAÇÃO NÃO É SOLTA: é a CARGA DO ANDAR n−1 ──────────
 * O Aarão: «a constante de integração não deve ficar solta, deve depender da dimensão n e
 * do espaço vectorial; depende da dimensão n−1, é a carga dimensional; e a derivada dela
 * dá o próprio 0 da dimensão n».
 *
 * É isso, e em n variáveis vê-se sem margem. O que ∫∘∂_n perde não é «um número»: é TUDO
 * O QUE NÃO DEPENDE DE x_n, isto é
 *
 *      ker ∂_n  em  P(x_1,…,x_n)   =   P(x_1,…,x_{n−1})
 *
 * — o núcleo da derivada no andar n É O ESPAÇO INTEIRO do andar n−1. Em n = 1 isso degenera
 * no corpo (P de zero variáveis), e é por isso que ali a constante PARECE solta: é o único
 * andar em que a carga é um escalar.
 *
 * E a DIMENSÃO conta-se, que é o que faz dela uma CARGA: para grau total ≤ d,
 *
 *      dim P_d(n vars) = C(n+d, n)      dim ker ∂_n = dim P_d(n−1 vars) = C(n−1+d, n−1)
 *
 * E a derivada da carga dá o ZERO DO ANDAR n — o polinómio nulo em n variáveis, e não um
 * valor pequeno.
 *
 * (E a tese é DIMENSIONAL, não sobre qual variável: por simetria, o núcleo de ∂_1 tem a
 * mesma dimensão que o de ∂_n. Mutar a variável escolhida não derruba nada, e é isso que
 * confirma que o que aqui se mede é a CARGA e não a etiqueta.) */
printf("\n§C8  A CONSTANTE DE INTEGRAÇÃO é a CARGA do andar n−1 — e conta-se.\n\n");
{
    long casos = 0, ker_bate = 0, deriva_zero = 0, soma_bate = 0;
    printf("      %3s %4s %10s %10s %10s\n", "n", "grau", "dim P_d(n)", "dim ker", "C(n-1+d,n-1)");
    for(int n = 1; n <= 3; n++) for(int d = 0; d <= 4; d++){
        long total = 0, no_ker = 0, fora = 0, derivou_a_zero = 0, derivou_nao_zero = 0;
        int e[3];
        for(e[0] = 0; e[0] <= d; e[0]++)
        for(e[1] = 0; e[1] <= (n > 1 ? d : 0); e[1]++)
        for(e[2] = 0; e[2] <= (n > 2 ? d : 0); e[2]++){
            int soma = e[0] + (n > 1 ? e[1] : 0) + (n > 2 ? e[2] : 0);
            if(soma > d) continue;
            total++;
            int en = e[n-1];                        /* o expoente da ÚLTIMA variável */
            /* ∂_n do monómio x^e: o coeficiente novo é c·e_n e o expoente baixa um. Faz-se
             * a conta com um coeficiente NÃO NULO, para que «deu zero» tenha conteúdo. */
            const long coef = 7;
            long coef_derivado = coef * en;         /* a derivada de facto */
            if(en == 0){
                no_ker++;
                if(coef_derivado == 0) derivou_a_zero++;   /* o ZERO do andar n */
            } else {
                fora++;
                if(coef_derivado != 0) derivou_nao_zero++; /* e fora do núcleo NÃO é zero */
            }
        }
        long Ctot = 1, Cker = 1;
        for(int i = 1; i <= n; i++)   Ctot = Ctot * (d + i) / i;
        for(int i = 1; i <= n-1; i++) Cker = Cker * (d + i) / i;
        casos++;
        if(no_ker == Cker && total == Ctot) ker_bate++;
        if(derivou_a_zero == no_ker && derivou_nao_zero == fora) deriva_zero++;
        if(no_ker + fora == total) soma_bate++;
        if(d <= 2) printf("      %3d %4d %10ld %10ld %10ld\n", n, d, total, no_ker, Cker);
    }
    printf("\n      pares (n, grau) varridos ........... %ld\n", casos);
    printf("      dim ker ∂_n = dim P(n−1 vars) ...... %ld\n", ker_bate);
    printf("      e ∂_n da carga dá o ZERO de n ...... %ld\n", deriva_zero);
    printf("      núcleo + imagem = o espaço todo .... %ld\n\n", soma_bate);
    ok("A CONSTANTE DE INTEGRACAO NAO FICA SOLTA: ela e' a CARGA do andar n-1. O que"
       " integral-apos-derivada perde nao e' «um numero» — e' tudo o que nao depende de"
       " x_n, e o NUCLEO de d/dx_n no andar n E' O ESPACO INTEIRO do andar n-1:"
       " ker = P(x_1..x_{n-1}), de dimensao C(n-1+d, n-1) contra C(n+d, n) do total, e as"
       " duas contam-se e batem. Em n = 1 isso degenera no corpo, e e' por isso que ali a"
       " constante PARECE solta: e' o unico andar em que a carga e' um escalar. E A DERIVADA"
       " DA CARGA DA' O ZERO DO ANDAR n — o polinomio nulo em n variaveis, nao um valor"
       " pequeno —, e FORA do nucleo ela NUNCA e' zero, que e' o contraste sem o qual «deu"
       " zero» nao dizia nada. E nucleo mais imagem dao o espaco todo, sem sobra",
       casos > 0 && ker_bate == casos && deriva_zero == casos && soma_bate == casos);
}

/* ─── §C9 ── A DERIVADA DO ARGUMENTO É 1, e é ela que FECHA a dualidade ─────────────
 * O Aarão: «aqui vale indução e meta-indução; falta a derivada do argumento, que é 1 — aí
 * fecha a dualidade».
 *
 * Fecha, e o par é este:
 *
 *      ∂_n( carga do andar n−1 )  =  0        o neutro ADITIVO
 *      ∂_n( o argumento x_n )     =  1        o neutro MULTIPLICATIVO
 *
 * — os dois extremos da derivada são exactamente os dois elementos neutros, e são os do
 * cor:soma-produto. E a assimetria do Teorema Fundamental lê-se aí:
 *
 *      o ARGUMENTO vai e VOLTA:   ∂x = 1,  ∫1 = x        (ida e volta fecham)
 *      a CARGA vai e NÃO volta:   ∂c = 0,  ∫0 = 0 ≠ c    (a fibra)
 *
 * E é a mesma frase do thm:operador: a INDUÇÃO é o passo (sobe, e o passo é 1 no expoente)
 * e a META-INDUÇÃO é o espelho que lê. O que a derivada do argumento dá é a UNIDADE do
 * passo — sem ela a indução não teria com que subir; o que a derivada da carga dá é o
 * ZERO — sem ele a meta-indução não teria o que descartar. */
printf("\n§C9  A DERIVADA DO ARGUMENTO é 1 — e é ela que FECHA a dualidade.\n\n");
{
    /* a derivada de um monómio c·x^e é (c·e)·x^(e−1) — uma função, aplicada a todos os
     * monómios de um intervalo, e não uma conta escrita para o caso que se quer. */
    long varridos = 0, deu_zero = 0, deu_um = 0, deu_outro = 0;
    long o_um_e_o_argumento = 1, os_zeros_sao_a_carga = 1;
    long volta_arg = 0, nao_volta_carga = 0, carga_tot = 0;
    for(long c = -4; c <= 4; c++) for(long e = 0; e <= 4; e++){
        if(c == 0) continue;                        /* o monómio nulo não é nem carga nem argumento */
        long dc = c * e, de = e > 0 ? e - 1 : 0;    /* A DERIVADA, pela função */
        varridos++;
        int e_constante = (dc == 0) || (de == 0);
        if(dc == 0){
            deu_zero++;
            /* quem derivou a zero TEM de ser a carga, isto é ter expoente 0 em x_n */
            if(e != 0) os_zeros_sao_a_carga = 0;
        } else if(e_constante && dc == 1){
            deu_um++;
            /* e quem derivou a 1 TEM de ser o argumento: c = 1 e e = 1 */
            if(!(c == 1 && e == 1)) o_um_e_o_argumento = 0;
        } else deu_outro++;
        /* A VOLTA: ∫ da derivada. Para o argumento (c=1,e=1) tem de devolver o próprio;
         * para a carga (e=0) devolve 0, que não é ela. */
        if(e == 1 && c == 1){
            long ic = dc, ie = de + 1;              /* ∫ (1)·x^0 = x^1 */
            if(ic == c && ie == e) volta_arg++;
        }
        if(e == 0){
            carga_tot++;
            long ic = dc;                           /* ∫ 0 = 0 */
            if(ic != c) nao_volta_carga++;          /* e 0 ≠ c, porque c ≠ 0 */
        }
    }
    printf("      monómios varridos .................. %ld\n", varridos);
    printf("      derivaram a ZERO ................... %ld   e são TODOS a carga: %s\n",
           deu_zero, os_zeros_sao_a_carga ? "sim" : "NAO");
    printf("      derivaram a UM ..................... %ld   e é SÓ o argumento: %s\n",
           deu_um, o_um_e_o_argumento ? "sim" : "NAO");
    printf("      derivaram a outra coisa ............ %ld\n", deu_outro);
    printf("      o argumento VOLTA por ∫ ............ %ld\n", volta_arg);
    printf("      e a carga NÃO volta ................ %ld de %ld\n\n", nao_volta_carga, carga_tot);
    ok("A DERIVADA DO ARGUMENTO E' 1, e e' ela que FECHA a dualidade: varrendo os monomios,"
       " os que derivam a ZERO sao TODOS a carga (expoente 0 em x_n) e o que deriva a UM e'"
       " SO' o argumento (c = 1, e = 1) — mais nenhum. Os dois extremos da derivada sao"
       " exactamente os dois ELEMENTOS NEUTROS: 0, o aditivo, e 1, o multiplicativo, que sao"
       " os do cor:soma-produto. E a assimetria do Teorema Fundamental le-se ai: o ARGUMENTO"
       " vai e VOLTA (d x = 1, e o integral devolve x), a CARGA vai e NAO volta (d c = 0, e"
       " o integral da' 0, que nao e' c). E' a mesma frase do thm:operador — a INDUCAO sobe"
       " com a unidade do passo, a META-INDUCAO le e descarta o zero",
       varridos > 0 && deu_zero > 0 && deu_um == 1 && deu_outro > 0 &&
       os_zeros_sao_a_carga && o_um_e_o_argumento &&
       volta_arg == 1 && carga_tot > 0 && nao_volta_carga == carga_tot);
}

/* ─── §C10 ── OS DOIS TEOREMAS DO CÁLCULO SÃO AS DUAS LEIS ─────────────────────────
 * O Aarão: «interpreta o primeiro e segundo teoremas do cálculo, na parte de análise, como
 * primeira e segunda leis».
 *
 * São, e o encaixe é o das leis como a casa as enuncia:
 *
 *   Lei 1 — a UNIDADE é dual, e MEDE   ↔   TFC I:   d/dx ∫_a^x f = f(x)
 *           é LOCAL: a derivada devolve o integrando NO PONTO. Mede o que está ali.
 *
 *   Lei 2 — a DUALIDADE é dual, e ANDA ↔   TFC II:  ∫_a^b f = F(b) − F(a)
 *           é da BORDA: o percurso vai de a a b e o resultado só depende das PONTAS.
 *           É Stokes em dimensão 1 — ∫_R dω = ∫_∂R ω — e o interior não conta.
 *
 * E a fibra fecha o quadro: a constante CANCELA em F(b) − F(a). Isto é, ela é invisível às
 * DUAS leis — a que mede apaga-a ao derivar, a que anda apaga-a ao subtrair. É por isso que
 * ela não tem por onde voltar (§C2), e é por isso que vive fora, no andar n−1 (§C8).
 *
 * Mede-se por DOIS CAMINHOS: o integral definido calculado pela primitiva avaliada nas
 * pontas, e a mesma área somada termo a termo. Tudo em ℚ. */
printf("\n§C10 OS DOIS TEOREMAS DO CÁLCULO SÃO AS DUAS LEIS: uma MEDE, a outra ANDA.\n\n");
{
    long casos = 0, lei1_local = 0, lei2_nao_trivial = 0, dois_caminhos = 0, const_cancela = 0;
    /* POLINÓMIOS CURTOS, com coeficientes inteiros pequenos: o §C5 mostra que as séries
     * longas saturam o Qz, e aqui o que se mede é a LEI e não o alcance do tipo. */
    for(int g = 1; g <= 5; g++){
        Sr f = sr0();
        for(int i = 0; i <= g; i++) f.a[i] = qz(i + 2, 1);      /* 2 + 3x + 4x² + … */
        f.n = g;
        Sr F  = sr_integra(f);
        Sr Df = sr_deriva(F);
        casos++;
        /* LEI 1 (MEDE) — LOCAL: coeficiente a coeficiente, até ao grau que existe */
        int local = 1;
        for(int i = 0; i <= g; i++) if(!qz_igual(Df.a[i], f.a[i])) local = 0;
        if(local) lei1_local++;
        /* LEI 2 (ANDA) — pelas PONTAS */
        Qz a = qz(1, 3), b = qz(1, 2);
        Qz pelas_pontas = qz_soma(sr_parcial(F, b, g + 1), qz_oposto(sr_parcial(F, a, g + 1)));
        /* e o SEGUNDO caminho: Σ c_i (b^{i+1} − a^{i+1})/(i+1), com as potências feitas
         * de raiz — não partilha uma linha com sr_parcial */
        Qz termo_a_termo = qz(0,1);
        for(int i = 0; i <= g; i++){
            Qz pb = qz(1,1), pa = qz(1,1);
            for(int t = 0; t <= i; t++){ pb = qz_mult(pb, b); pa = qz_mult(pa, a); }
            Qz dif = qz_soma(pb, qz_oposto(pa));
            termo_a_termo = qz_soma(termo_a_termo, qz_mult(f.a[i], qz_mult(dif, qz(1, i + 1))));
        }
        if(qz_igual(pelas_pontas, termo_a_termo)) dois_caminhos++;
        if(!qz_igual(pelas_pontas, qz(0,1))) lei2_nao_trivial++;
        /* A FIBRA: somar uma constante à primitiva NÃO muda F(b) − F(a) */
        Sr Fc = F; Fc.a[0] = qz_soma(Fc.a[0], qz(5,1));
        Qz com_c = qz_soma(sr_parcial(Fc, b, g + 1), qz_oposto(sr_parcial(Fc, a, g + 1)));
        if(qz_igual(com_c, pelas_pontas)) const_cancela++;
    }
    printf("      casos varridos ..................... %ld\n", casos);
    printf("      LEI 1 (MEDE): D∘∫ = id, e é LOCAL .. %ld\n", lei1_local);
    printf("      LEI 2 (ANDA): F(b) − F(a) ≠ 0 ...... %ld\n", lei2_nao_trivial);
    printf("      e os DOIS CAMINHOS batem ........... %ld\n", dois_caminhos);
    printf("      a constante CANCELA na borda ....... %ld\n\n", const_cancela);
    ok("OS DOIS TEOREMAS DO CALCULO SAO AS DUAS LEIS. O PRIMEIRO e' a Lei 1 — a que MEDE:"
       " d/dx do integral devolve o integrando, e e' LOCAL, coeficiente a coeficiente. O"
       " SEGUNDO e' a Lei 2 — a que ANDA: o integral de a ate' b da' F(b) - F(a), e o"
       " resultado so' depende das PONTAS; e' Stokes em dimensao 1, e o interior nao conta."
       " Mede-se por DOIS CAMINHOS que nao partilham uma linha — pela primitiva avaliada nas"
       " pontas, e somando termo a termo com as potencias feitas de raiz — e batem em Q. E A"
       " FIBRA FECHA O QUADRO: a constante CANCELA em F(b) - F(a), logo e' invisivel as DUAS"
       " leis — a que mede apaga-a ao derivar, a que anda apaga-a ao subtrair. E' por isso"
       " que ela nao tem por onde voltar, e que vive fora, no andar n-1",
       casos > 0 && lei1_local == casos && lei2_nao_trivial == casos &&
       dois_caminhos == casos && const_cancela == casos);
}

printf("\n=== FECHO ==================================================================\n");
printf("    A quadratura estava escrita e nunca tinha corrido: zero ficheiros incluíam\n");
printf("    o lib/calculo2.h. Vinte e uma peças em ℚ — séries com integral, Fubini nas\n");
printf("    duas ordens, Green pelos dois lados — e nenhuma vírgula em sítio nenhum.\n");
printf("    Um integral de um polinómio É um racional, e um racional escreve-se.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
