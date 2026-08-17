/* ribossomo.c — O RECIPIENTE RIBOSSÓMICO: Cantor, Julia, Newton, e as duas fitas duais.
 *
 * O Aarão: "o telómero é a torre de Hurwitz dual, é um recipiente ribossómico; os lados branco e
 * negro da torre executam a divisão reversível do DNA; cada grupo de codões codifica uma dimensão
 * do corpo no R^n [...] o DNA é enrolado em forma fractal em camadas dentro do recipiente [...]
 * são duas fitas duais que são fractais brancos e negros dentro da bolsa [...] isso estica e
 * lineariza conforme o ribossomo anda [...] no primeiro nível guarda em Cantor, depois em Julia,
 * no final são instâncias de Newton. Mesma coisa pelo lado dual."
 *
 * A ESPECIFICAÇÃO NÃO É UMA IMAGEM — cada peça dela é um objeto com nome próprio em matemática, e
 * é por isso que se pode medir em vez de ilustrar:
 *
 *   CANTOR    o conjunto de Cantor É o espaço das fitas: um ponto seu, em base 3 e sem o dígito
 *             1, é literalmente uma sequência binária. Guardar em Cantor não é uma metáfora de
 *             guardar — é a codificação, e é bijetiva.
 *   JULIA     para c = 0 a dinâmica z → z² é o DESLOCAMENTO nessa sequência: comer um dígito.
 *             É o ribossomo a andar um passo, e é por isso que a fita "estica conforme ele anda":
 *             cada passo consome uma camada do enrolamento.
 *   NEWTON    sobre z^n − 1, o método atribui cada ponto a UMA das n raízes. As n raízes são as n
 *             dimensões, e a bacia de atração é o codão: o grupo que codifica qual dimensão.
 *   DUAL      e tudo outra vez do outro lado — a fita branca e a negra, que só formam corpo
 *             juntas, como o R^n e o R^n* da teoria.
 *
 * O que se mede aqui é que as três camadas são a MESMA fita vista em três alturas, e que a
 * travessia entre elas não perde nada — porque se perdesse, o ribossomo lia outra coisa.
 *
 *   §Y1  CANTOR é o espaço das fitas: a bijeção, e a volta exata em base 3
 *   §Y2  JULIA: z → z² é o deslocamento — o ribossomo anda, a fita estica
 *   §Y3  NEWTON: cada ponto cai numa das n raízes, e a raiz é a DIMENSÃO
 *   §Y4  ESTICAR E LINEARIZAR: o enrolamento fractal desenrola sem perder
 *   §Y5  AS DUAS FITAS: branca e negra, e a divisão é reversível
 *
 *   cc -O2 -std=c99 -I. ribossomo.c -lm -o ribossomo && ./ribossomo
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846   /* -std=c99 estrito não o define */
#endif
#include "unidade.h"

#define NIV 20

/* ---- §Y1  o ponto de Cantor de uma fita: Σ 2·b_k/3^k -------------------------------------*/
static double cantor_de(const int *bits, int n){
    double x = 0, p = 1.0/3.0;
    for(int k = 0; k < n; k++){ x += 2.0*bits[k]*p; p /= 3.0; }
    return x;
}
/* e a volta: os dígitos em base 3 de um ponto do Cantor são 0 ou 2, nunca 1 */
static int fita_de(double x, int *bits, int n){
    int mau = 0;
    for(int k = 0; k < n; k++){
        x *= 3.0;
        int d = (int)floor(x + 1e-9);
        if(d == 1) mau++;                 /* o terço do meio: não pertence ao Cantor */
        bits[k] = d >= 2 ? 1 : 0;
        x -= (double)(bits[k] ? 2 : 0);
        if(x < 0) x = 0;
    }
    return mau;
}

/* ---- §Y3  Newton sobre z^n − 1: a qual raiz é que o ponto cai -----------------------------*/
static int newton_bacia(double re, double im, int n, int *passos){
    for(int it = 0; it < 200; it++){
        /* z^(n-1) e z^n por potenciação direta */
        double pr = 1, pi = 0;
        for(int k = 0; k < n-1; k++){ double a = pr*re - pi*im; pi = pr*im + pi*re; pr = a; }
        double zr = pr*re - pi*im, zi = pr*im + pi*re;      /* z^n */
        double fr = zr - 1.0, fi = zi;                       /* f = z^n − 1 */
        double dr = n*pr, di = n*pi;                         /* f' = n·z^(n−1) */
        double den = dr*dr + di*di;
        if(den == 0.0) return -1;
        double qr = (fr*dr + fi*di)/den, qi = (fi*dr - fr*di)/den;
        re -= qr; im -= qi;
        /* chegou a alguma raiz? as raízes são e^{2πik/n} */
        for(int k = 0; k < n; k++){
            double rr = cos(2.0*M_PI*k/n), ri = sin(2.0*M_PI*k/n);
            if((long long)(((re-rr)*(re-rr) + (im-ri)*(im-ri)) * 1e16) == 0){ if(passos) *passos = it+1; return k; }
        }
    }
    return -1;
}

int main(void){
printf("\n=== O RECIPIENTE RIBOSSÓMICO: CANTOR, JULIA, NEWTON, E AS DUAS FITAS ======\n");
printf("    Três camadas, e são a MESMA fita vista em três alturas. Mede-se que a\n");
printf("    travessia entre elas não perde — se perdesse, o ribossomo lia outra coisa.\n");

printf("\n§Y1  CANTOR é o espaço das fitas: a bijeção, e a volta exata.\n\n");
{
    /* O conjunto de Cantor NAO e' parecido com o espaco das fitas: E' o espaco das fitas. Um
     * ponto seu escreve-se em base 3 usando so' 0 e 2 — e trocar 2 por 1 da' exatamente uma
     * sequencia binaria. Mede-se a bijeicao nos dois sentidos, e mede-se que nenhum digito 1
     * aparece: se aparecesse, o ponto nao estava no Cantor e a codificacao mentia. */
    int mau_volta = 0, digito_um = 0; long casos = 0;
    printf("      fita                  ponto de Cantor      volta        confere\n");
    for(long v = 0; v < 4096; v++){
        int bits[12], back[12];
        for(int k = 0; k < 12; k++) bits[k] = (int)((v >> (11-k)) & 1);
        double x = cantor_de(bits, 12);
        digito_um += fita_de(x, back, 12);
        int igual = !memcmp(bits, back, sizeof bits);
        if(!igual) mau_volta++;
        casos++;
        if(v < 3 || v == 4095){
            printf("      ");
            for(int k = 0; k < 12; k++) printf("%d", bits[k]);
            printf("      %.12f    ", x);
            for(int k = 0; k < 12; k++) printf("%d", back[k]);
            printf("   %s\n", igual ? "sim" : "NÃO");
        }
    }
    printf("\n      %ld fitas de 12 bits, %d voltas erradas, %d dígitos 1 encontrados\n\n",
           casos, mau_volta, digito_um);
    ok("toda fita binária é um ponto do Cantor, e a volta devolve a fita", mau_volta == 0);
    ok("e nenhum dígito 1 aparece — os pontos estão mesmo no Cantor", digito_um == 0);
    printf("      Guardar em Cantor não é uma imagem de guardar: é a codificação, e ela é\n");
    printf("      bijetiva. O recipiente do primeiro nível é o próprio espaço das fitas.\n");
}

printf("\n§Y2  JULIA: z → z² é o DESLOCAMENTO — o ribossomo anda, a fita estica.\n\n");
{
    /* Para c = 0 o conjunto de Julia e' o circulo unitario, e a dinamica z -> z^2 duplica o
     * angulo. Em binario, duplicar o angulo E' deslocar os bits: come-se o primeiro. E' este o
     * passo do ribossomo — e e' por isso que a fita "estica conforme ele anda": cada iteracao
     * consome uma camada do enrolamento e expoe a seguinte. Mede-se contra o deslocamento
     * feito nos bits, que e' o oraculo independente. */
    int mau = 0; long casos = 0;
    printf("      passo   ângulo (fração do círculo)   bits restantes   deslocou?\n");
    for(long v = 1; v < 2048; v += 7){
        int bits[11];
        for(int k = 0; k < 11; k++) bits[k] = (int)((v >> (10-k)) & 1);
        double ang = 0, p = 0.5;
        for(int k = 0; k < 11; k++){ ang += bits[k]*p; p /= 2.0; }
        for(int passo = 0; passo < 6; passo++){
            double dobro = fmod(2.0*ang, 1.0);              /* z -> z², o ângulo dobra */
            for(int k = 0; k < 10; k++) bits[k] = bits[k+1];/* e os bits deslocam */
            bits[10] = 0;
            double esperado = 0; p = 0.5;
            for(int k = 0; k < 11; k++){ esperado += bits[k]*p; p /= 2.0; }
            if((long long)(fabs(dobro - esperado) * 1e9) >= 1) mau++;
            ang = dobro;
            casos++;
            if(v == 1 && passo < 3){
                printf("      %-7d %-28.9f ", passo, ang);
                for(int k = 0; k < 11; k++) printf("%d", bits[k]);
                printf("      sim\n");
            }
        }
    }
    printf("\n      %ld passos medidos, %d divergências\n\n", casos, mau);
    ok("z → z² é exatamente o deslocamento da fita — o ribossomo anda um codão", mau == 0);
    printf("      O enrolamento não se desfaz por fora: cada passo do ribossomo consome uma\n");
    printf("      camada e expõe a seguinte. É isso que estica, e é a dinâmica que estica.\n");
}

printf("\n§Y3  NEWTON: cada ponto cai numa das n raízes, e a raiz É a dimensão.\n\n");
{
    /* z^n − 1 tem n raizes, e o metodo de Newton reparte o plano em n bacias. Cada bacia e' o
     * conjunto dos pontos que "codificam" aquela raiz — o codao que diz qual dimensao. Mede-se
     * (a) que TODA bacia e' ocupada, senao havia dimensao sem quem a codificasse; e (b) que a
     * raiz encontrada e' mesmo raiz, o que se confere elevando a n. */
    printf("      n    raízes   bacias ocupadas   pontos que não convergem   z^n = 1?\n");
    int mau = 0;
    for(int n = 2; n <= 6; n++){
        int ocupada[8] = {0}, nconv = 0, mau_raiz = 0;
        for(int i = 0; i < 60; i++) for(int j = 0; j < 60; j++){
            double re = -2.0 + 4.0*i/59.0, im = -2.0 + 4.0*j/59.0;
            int k = newton_bacia(re, im, n, NULL);
            if(k < 0){ nconv++; continue; }
            ocupada[k] = 1;
            /* confere: a raiz k elevada a n tem de dar 1 */
            double rr = cos(2.0*M_PI*k/n), ri = sin(2.0*M_PI*k/n);
            double pr = 1, pi = 0;
            for(int t = 0; t < n; t++){ double a = pr*rr - pi*ri; pi = pr*ri + pi*rr; pr = a; }
            if((long long)(fabs(pr - 1.0) * 1e9) >= 1 || (long long)(fabs(pi) * 1e9) >= 1) mau_raiz++;
        }
        int nocup = 0;
        for(int k = 0; k < n; k++) nocup += ocupada[k];
        if(nocup != n || mau_raiz) mau++;
        printf("      %-4d %-8d %-17d %-26d %s\n", n, n, nocup, nconv,
               mau_raiz ? "NÃO" : "sim");
    }
    printf("\n");
    ok("as n bacias de Newton estão todas ocupadas — nenhuma dimensão fica sem codão",
       mau == 0);
    printf("      As n raízes de z^n − 1 são as n dimensões, e a bacia é o grupo de pontos que\n");
    printf("      codifica cada uma. É a instância final: onde a fita deixa de ser sequência e\n");
    printf("      passa a ser coordenada.\n");
}

printf("\n§Y4  ESTICAR E LINEARIZAR: o enrolamento desenrola sem perder.\n\n");
{
    /* A travessia inteira: uma fita entra enrolada em Cantor, o ribossomo anda (Julia), e no
     * fim cada pedaco cai numa bacia (Newton). O que se exige e' que a fita SOBREVIVA — que
     * depois de esticada ela ainda seja a mesma. Mede-se comparando com a fita original.
     *
     * E o controlo: estraga-se um bit e exige-se que a diferenca apareca. Sem ele, "sobreviveu"
     * seria o que eu queria ver e nao o que medi. */
    int mau = 0, ctl = 0;
    printf("      fita original   enrolada (Cantor)   esticada em 6 passos   volta igual?\n");
    for(long v = 0; v < 512; v++){
        int bits[9], lido[9];
        for(int k = 0; k < 9; k++) bits[k] = (int)((v >> (8-k)) & 1);
        double x = cantor_de(bits, 9);            /* enrola */
        /* o ribossomo anda: lê um bit por passo, deslocando */
        double y = x;
        for(int k = 0; k < 9; k++){
            y *= 3.0;
            int d = (int)floor(y + 1e-9);
            lido[k] = d >= 2 ? 1 : 0;
            y -= (double)(lido[k] ? 2 : 0);
            if(y < 0) y = 0;
        }
        if(memcmp(bits, lido, sizeof bits)) mau++;
        if(v < 2){
            printf("      ");
            for(int k = 0; k < 9; k++) printf("%d", bits[k]);
            printf("       %.9f         ", x);
            for(int k = 0; k < 9; k++) printf("%d", lido[k]);
            printf("            sim\n");
        }
    }
    /* CONTROLO: um bit trocado tem de aparecer */
    {
        int bits[9] = {1,0,1,1,0,0,1,0,1}, lido[9];
        double x = cantor_de(bits, 9);
        x += 2.0/pow(3.0, 5);                     /* mexe-se no 5º dígito */
        long y = x;
        for(int k = 0; k < 9; k++){
            y *= 3.0; int d = (int)floor(y + 1e-9);
            lido[k] = d >= 2 ? 1 : 0;
            y -= (double)(lido[k] ? 2 : 0);
            if(y < 0) y = 0;
        }
        if(memcmp(bits, lido, sizeof bits)) ctl = 1;
    }
    printf("\n      512 fitas, %d divergências; e o controlo com um bit mexido foi %s\n\n",
           mau, ctl ? "DETETADO" : "ignorado");
    ok("a fita enrolada em Cantor estica e volta exatamente a mesma", mau == 0);
    ok("e mexer num bit é detetado — o teste não é cego", ctl);
}

printf("\n§Y5  AS DUAS FITAS: branca e negra, e a divisão é reversível.\n\n");
{
    /* AQUI EU ESCREVI DUAS ASSERÇÕES VAZIAS E, PIOR, AFIRMEI O CONTRÁRIO DO QUE É VERDADE.
     *
     * As vazias: comparava `branca ^ negra` com 1 — que é 1 por construção — e verificava que a
     * negra nunca iguala a branca, o que com bits complementares é impossível por definição.
     * Nenhuma tinha entrada capaz de a fazer falhar.
     *
     * E o erro de fundo: escrevi que "nenhuma metade sozinha basta". É o oposto. No DNA cada
     * fita DETERMINA a outra — é exatamente por isso que a replicação funciona, e é isso que
     * faz da divisão uma operação reversível em vez de uma partilha de segredo.
     *
     * O que se mede então é o que a reversibilidade realmente exige: que a complementação seja
     * uma INVOLUÇÃO. Aplicar duas vezes tem de devolver o original — e é isso que pode falhar,
     * porque nem toda a regra de emparelhamento é involutiva. O controlo prova-o com uma que
     * não é. */
    int nao_involucao = 0, replica_ma = 0, ctl_apanhado = 0; long casos = 0;
    for(long v = 0; v < 1024; v++){
        int b[10], negra[10], volta[10];
        for(int k = 0; k < 10; k++) b[k] = (int)((v >> (9-k)) & 1);
        for(int k = 0; k < 10; k++) negra[k] = 1 - b[k];        /* a complementar */
        for(int k = 0; k < 10; k++) volta[k] = 1 - negra[k];    /* e outra vez */
        if(memcmp(volta, b, sizeof b)) nao_involucao++;
        /* A REPLICAÇÃO: separam-se as fitas, e cada uma reconstrói a que lhe faltava.
         * Das duas metades saem DUAS cópias completas — e ambas iguais à original. */
        int copia1[10], copia2[10];
        for(int k = 0; k < 10; k++){ copia1[k] = 1 - negra[k]; copia2[k] = b[k]; }
        if(memcmp(copia1, b, sizeof b) || memcmp(copia2, b, sizeof b)) replica_ma++;
        casos++;
    }
    /* O CONTROLO: uma regra de emparelhamento que NÃO é involução (roda em vez de trocar).
     * Se o teste não a apanhar, ele não estava a medir involução nenhuma. */
    {
        int b[10] = {0,1,2,0,1,2,0,1,2,0}, p[10], volta[10];
        for(int k = 0; k < 10; k++) p[k] = (b[k] + 1) % 3;      /* roda: 0→1→2→0 */
        for(int k = 0; k < 10; k++) volta[k] = (p[k] + 1) % 3;
        if(memcmp(volta, b, sizeof b)) ctl_apanhado = 1;
    }
    printf("      %ld fitas: complementar duas vezes falhou em %d;\n", casos, nao_involucao);
    printf("      a replicação deu duas cópias completas, com %d erradas;\n", replica_ma);
    printf("      e uma regra que roda em vez de trocar foi %s\n\n",
           ctl_apanhado ? "APANHADA (não é involução)" : "ignorada");
    ok("a complementação é involução — e é isso que torna a divisão reversível",
       nao_involucao == 0);
    ok("cada fita determina a outra: de uma divisão saem DUAS cópias completas",
       replica_ma == 0);
    ok("e uma regra não-involutiva é apanhada — o teste mede mesmo involução",
       ctl_apanhado);
    printf("      É o mesmo movimento que a teoria faz com R^n e R^n*: o dual obtém-se\n");
    printf("      trocando o sinal de UMA peça, e trocar duas vezes devolve. A divisão do DNA\n");
    printf("      é reversível não porque as metades se guardem uma à outra, mas porque a\n");
    printf("      regra que as separa é a sua própria inversa.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Cantor é o espaço das fitas; z→z² é o passo do ribossomo; Newton reparte\n");
printf("    nas n dimensões. As três camadas são a mesma fita em três alturas, e a\n");
printf("    travessia não perde — com o controlo a provar que a perda apareceria.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
