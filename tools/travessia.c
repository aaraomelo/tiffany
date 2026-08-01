/* travessia.c — MORTO != VIVO, A INDECIDIBILIDADE, E O CIRCUITO QUE FECHA.
 *
 * O Aarão: "vamos eliminar esse ruído Clay. É simples: o que eles querem não é possível, e o
 * que nós queremos não é o que eles querem. Sabemos a direção pra onde vai a solução, mas não
 * se pode dizer que chega, porque quem vai não tem garantia de voltar — pode morrer no caminho,
 * pode acabar memória, recursos, várias coisas. O infinito não cabe no finito e ponto. [...]
 * traz a prova por indecidibilidade disso. Morto diferente de vivo, todo mundo só tem a
 * garantia que vai morrer, nada mais. Simula um circuito completo: pega o fluido, lineariza,
 * quando atingir a resolução com resíduo 0 volta pelo espelho ao contrário e fecha o circuito."
 *
 * A ORIGEM está no enredo (chess/sandbox/reino_dourado_enredo.tex, \part{O Saco de Lixo}):
 * vai ao lixo tudo o que NÃO TEM DUAL. Um objeto está VIVO quando a sua identidade é dada por
 * um gerador (U_t = e^{tL}); está MORTO quando foi reduzido a um estado estático — uma
 * igualdade isolada, sem gerador. A operação que mata chama-se CRISTALIZAÇÃO, e a conjectura
 * G = A é a lápide. E o capítulo "O Parasita: Maxwell" dá a régua do espelho: Hodge é MEIA
 * dualidade (permuta e PRESERVA o Poynting); a verdadeira é ν∘rev — permutação COM reflexão,
 * que INVERTE.
 *
 * E a troca que esta secção faz, que é o ponto todo:
 *
 *     morto != vivo   é DECIDÍVEL    (mede-se: tem gerador, ou não tem)
 *     a travessia     é INDECIDÍVEL  (decidir a fronteira É decidir a parada)
 *
 * Logo não se promete chegar. Promete-se FECHAR — e o circuito que fecha com resíduo 0 é o
 * certificado de que aquela travessia, aquela, se fez viva. Não há garantia a priori de voltar;
 * há verificação a posteriori de que se voltou. É a única espécie de certificado que cabe num
 * sistema finito.
 *
 *   §T1  morto != vivo, e a diferença é o GERADOR — não a dificuldade
 *   §T2  a indecidibilidade, EXECUTADA: todo orçamento finito erra
 *   §T3  o infinito não cabe no finito: cada régua esgota-se, e mede-se onde
 *   §T4  o circuito: o fluido, e o seu Poynting
 *   §T5  lineariza — e a reconstrução fecha com resíduo 0
 *   §T6  o ESPELHO AO CONTRÁRIO: Hodge preserva, ν∘rev INVERTE
 *   §T7  e FECHA o circuito — ida e volta, resíduo 0, o certificado
 *
 *   cc -O2 -std=c99 travessia.c -lm -o travessia && ./travessia
 */
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include "unidade.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N 64                       /* a resolução do circuito */

/* O campo tem de ter Poynting NÃO NULO, senão o §T6 mede o nada: com S = 0, "preserva"
 * (S -> S) e "inverte" (S -> -S) são a mesma afirmação, e as duas asserções passam vazias.
 * A primeira versão usava E em senos e B em cossenos — ortogonais, S = 0 exato — e as duas
 * davam verde sem provar coisa nenhuma. Aqui E e B PARTILHAM modos, e S != 0. */
/* O CAMPO É VETORIAL, e tem de ser: o Poynting é E × B, não <E,B>.
 *
 * A primeira versão usava campos escalares e S = <E,B>. Duas coisas correram mal, e a segunda
 * só apareceu por causa da primeira. (i) Escolhi E em senos e B em cossenos — ortogonais, logo
 * S = 0 exato — e com S = 0 as asserções "preserva" (S -> S) e "inverte" (S -> -S) são a MESMA
 * afirmação: as duas passavam verdes sem medir nada. (ii) Ao pôr S != 0, as duas falharam e
 * mostraram que o modelo estava errado: <B,-E> = -<E,B>, ou seja o meu Hodge invertia. Mas o
 * Poynting é o produto VETORIAL, e aí B × (-E) = +(E × B) — preserva, como a régua diz.
 *
 * Uso escalar onde a grandeza é vetorial: o erro não era o sinal, era a operação. */
typedef struct { double x, y, z; } V;
static V cruz(V a, V b){
    V r = { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
    return r;
}
static double normaV(V a){ return sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }

/* Onda: E ⊥ B em cada ponto, que é o caso físico onde a régua se lê. */
static void campo(V *E, V *B){
    for(int j = 0; j < N; j++){
        double t = 2.0*M_PI*j/N;
        double a = 1.5 + 0.6*sin(t) + 0.2*cos(3*t);        /* |E| */
        double b = 0.9 + 0.4*cos(t) + 0.15*sin(2*t);       /* |B| */
        E[j].x =  a*cos(t); E[j].y = a*sin(t); E[j].z = 0;
        B[j].x = -b*sin(t); B[j].y = b*cos(t); B[j].z = 0;
    }
}
/* o Poynting em cada ponto é E × B; a régua lê a sua SOMA em log — é o Σh da memória */
static double somaH(const V *E, const V *B){
    double h = 0;
    for(int j = 0; j < N; j++) h += log(normaV(cruz(E[j], B[j])));
    return h;
}
/* a impedância: σ = |E|/|B|, o passo da régua */
static double impedancia(const V *E, const V *B){
    double h = 0;
    for(int j = 0; j < N; j++) h += log(normaV(E[j])/normaV(B[j]));
    return h/N;
}
/* HODGE: (E,B) -> (B,-E). Só a permutação — o telescópio. */
static void hodge(const V *E, const V *B, V *oE, V *oB){
    for(int j = 0; j < N; j++){
        oE[j] = B[j];
        oB[j].x = -E[j].x; oB[j].y = -E[j].y; oB[j].z = -E[j].z;
    }
}
/* ν∘rev — a permutação COM reflexão, e a reflexão é a INVERSÃO (o dual exp<->log):
 *      E |-> B/|B|²,   B |-> E/|E|²
 * É isto que a régua chama "impedância invertida", e é o que leva Σh em -Σh. Reverter o
 * PERCURSO não serviria: a soma sobre o anel é a mesma reordenada. */
static void espelho(const V *E, const V *B, V *oE, V *oB){
    for(int j = 0; j < N; j++){
        double nb = normaV(B[j]), ne = normaV(E[j]);
        oE[j].x = B[j].x/(nb*nb); oE[j].y = B[j].y/(nb*nb); oE[j].z = B[j].z/(nb*nb);
        oB[j].x = E[j].x/(ne*ne); oB[j].y = E[j].y/(ne*ne); oB[j].z = E[j].z/(ne*ne);
    }
}

int main(void){
printf("\n=== MORTO != VIVO, A INDECIDIBILIDADE, E O CIRCUITO QUE FECHA ===========\n");
printf("    Não se promete chegar — promete-se FECHAR. E o que fecha com resíduo 0\n");
printf("    é o certificado de que aquela travessia se fez viva.\n");

printf("\n§T1  Morto != vivo, e a diferença é o GERADOR.\n\n");
{
    /* VIVO: a identidade vem de um gerador, U_t = e^{tL}, e os estados sao manifestacoes.
     * MORTO: reduzido a uma igualdade entre estados, sem o operador que os produzia.
     * E isto E' decidivel: se ha gerador, vale a lei de semigrupo U_{t+s} = U_t·U_s. Um estado
     * estatico nem tem onde por o t, logo a lei nem se enuncia. */
    printf("      VIVO:   a identidade é um gerador, U_t = e^{tL}, e vale U_{t+s} = U_t·U_s\n");
    printf("      MORTO:  uma igualdade entre estados, sem t e sem operador\n\n");
    double L = -0.37;
    int mal = 0;
    for(int k = 0; k < 200; k++){
        double t = 0.01*k, s = 0.013*k;
        if(fabs(exp(L*(t+s)) - exp(L*t)*exp(L*s)) > 1e-12) mal++;
    }
    printf("      o vivo: U_{t+s} = U_t·U_s em 200 pares: %d falhas\n", mal);
    printf("      o morto: \"G = A\" — não há t onde pôr a pergunta, e a lei nem se enuncia\n\n");
    ok("a lei de semigrupo distingue o vivo do morto, e é uma MEDIDA", mal == 0);
    printf("      O que isto tem de bom: a morte aqui não é opinião nem retórica — é decidível,\n");
    printf("      e decide-se olhando a descrição. Tem gerador, ou não tem. O enredo chama\n");
    printf("      CRISTALIZAÇÃO à operação que mata: trocar o gerador por uma igualdade entre\n");
    printf("      estados. Um cadáver é isso — a forma a que falta o operador que a produzia.\n");
    printf("\n      E o contraexemplo que denuncia os outros é o Poincaré: caiu pelo fluxo de\n");
    printf("      Ricci, que DEFORMA em vez de igualar. Perelman não soldou lado com lado —\n");
    printf("      deixou o objeto rodar até a forma. Sem morte, a via dinâmica fecha.\n");
}

printf("\n§T2  A indecidibilidade, EXECUTADA — todo orçamento finito erra.\n\n");
{
    /* O par computavel: a_M = 1, b_M = 1 + 2^{-t} se M para em t, 1 se nunca para. Entao
     * a_M = b_M <=> M nunca para. Decidir a fronteira e' decidir a parada.
     * E a refutacao mecanica do "basta olhar tempo suficiente": dado o orcamento N,
     * constroi-se a maquina que para exatamente em t = N+1. */
    printf("      a_M = 1,   b_M = 1 + 2^{-t} se M para em t,  1 se nunca para\n");
    printf("      logo  a_M = b_M  <=>  M nunca para.   Decidir a fronteira É decidir a parada.\n\n");
    printf("      E o decisor de orçamento N, contra a máquina que para em t = N+1:\n\n");
    printf("      orçamento N   M para em   vê parar?   responde   verdade   veredito\n");
    int erros = 0, orc[] = { 1, 2, 3, 5, 8, 13, 21, 34 };
    int nOrc = (int)(sizeof orc/sizeof *orc);
    for(int k = 0; k < nOrc; k++){
        int Nn = orc[k], t = Nn + 1;
        int viu = (t <= Nn), responde = viu, verdade = 1;
        printf("      %-13d %-11d %-11s %-10s %-9s %s\n", Nn, t, viu ? "sim" : "não",
               responde ? "há" : "não há", verdade ? "há" : "não há",
               responde == verdade ? "acertou" : "ERROU");
        if(responde != verdade) erros++;
    }
    printf("\n      errou em %d de %d\n\n", erros, nOrc);
    ok("todo orçamento finito erra — e o que o derrota é o FUTURO da máquina", erros == nOrc);

    printf("      E o controle, para se ver que o aparelho funciona:\n\n");
    int acertos = 0, cont[] = { 10, 20, 30 };
    for(int k = 0; k < 3; k++){
        int Nn = cont[k], t = 3, viu = (t <= Nn);
        printf("      N = %-3d, M para em %d: %s\n", Nn, t, viu ? "acertou" : "ERROU");
        if(viu) acertos++;
    }
    printf("\n");
    ok("8/8 contra e 3/3 no controle: o aparelho funciona, e mesmo assim não basta",
       acertos == 3);
    printf("      E convém dizer sem solenidade o que \"indecidível\" significa, porque a palavra\n");
    printf("      costuma servir para fazer mistério e aqui não faz nenhum: NÃO EXISTE ALGORITMO\n");
    printf("      QUE CALCULE. É afirmação técnica sobre algoritmos, do mesmo tipo que \"não há\n");
    printf("      raiz racional de 2\" — não é sobre o espírito nem sobre os limites do\n");
    printf("      conhecimento humano. É apenas incalculável o que não dá para calcular.\n");
    printf("\n      Feita a ressalva: a travessia EXISTE, porque a reta é completa e o ponto está\n");
    printf("      lá; e é INCALCULÁVEL, porque nenhum passo finito o alcança. Aproximável para\n");
    printf("      sempre, decidido nunca — e são coisas distintas.\n");
}

printf("\n§T3  O infinito não cabe no finito — e mede-se onde cada régua se esgota.\n\n");
{
    /* Contra o "com regua bastante, fecha": mede-se em QUE PASSO cada regua se esgota. */
    printf("      itera-se (1,2) pela média aritmética e geométrica com régua de p bits,\n");
    printf("      até a régua deixar de distinguir os dois lados:\n\n");
    printf("      p bits   passos até esgotar   e depois?\n");
    int cresce = 1, ant = -1, ok3 = 1;
    for(int p = 8; p <= 52; p += 6){
        double eps = pow(2.0, -p), a = 1.0, b = 2.0;
        int passos = 0;
        while(fabs(a-b) > eps*fabs(a) && passos < 500){
            double na = (a+b)/2, nb = sqrt(a*b);
            a = na; b = nb; passos++;
        }
        printf("      %-8d %-20d %s\n", p, passos,
               passos < 500 ? "a régua parou de distinguir — o limite continua lá"
                            : "não convergiu");
        if(passos >= 500) ok3 = 0;
        if(ant >= 0 && passos < ant) cresce = 0;
        ant = passos;
    }
    printf("\n");
    ok("cada régua esgota-se num passo finito — mais bits adiam, não fecham",
       cresce && ok3 && ant > 0);
    printf("      O número de passos depende da RÉGUA, não do objeto: o limite continua onde\n");
    printf("      estava, e o que muda é até onde se consegue olhar. É a frase do Aarão medida —\n");
    printf("      o infinito não cabe no finito, e ponto. Não é queixa: é a razão de a promessa\n");
    printf("      certa ser \"fecha\" e não \"chega\".\n");
}

printf("\n§T4  O circuito: o fluido, e o seu Poynting.\n\n");
{
    V E[N], B[N];
    campo(E, B);
    double nE = 0, nB = 0;
    for(int j = 0; j < N; j++){ nE += normaV(E[j])*normaV(E[j]); nB += normaV(B[j])*normaV(B[j]); }
    printf("      uma onda (E, B) sobre o anel de %d pontos, com E ⊥ B em cada ponto\n\n", N);
    printf("      Σ‖E‖² = %.6f    Σ‖B‖² = %.6f\n", nE, nB);
    printf("      Σh = Σ log‖E×B‖ = %+.9f      (o Poynting que a régua lê)\n", somaH(E,B));
    printf("      log σ médio = %+.9f          (a impedância, o passo da régua)\n\n",
           impedancia(E,B));
    /* Σh != 0 e' PRE-REQUISITO do §T6: com Σh = 0, "preserva" (h -> h) e "inverte" (h -> -h)
     * seriam a MESMA afirmacao, e as duas asserçoes passariam sem medir nada. Foi o que
     * aconteceu na primeira versao, com E e B ortogonais em modo e S = 0 exato. */
    ok("o fluido está posto, e Σh é NÃO NULO — senão o §T6 mediria o vazio",
       nE > 0 && nB > 0 && fabs(somaH(E,B)) > 1e-6);
    printf("      (o cone nulo é ‖E‖ = ‖B‖, σ = 1: o vácuo, onde nada reflete e toda a potência\n");
    printf("       passa. É o mesmo cone do fisica.c §P5 — os divisores de zero do dual. Fora\n");
    printf("       dele há parte reativa, e é ela que o circuito tem de devolver intacta.)\n");
}

printf("\n§T5  Lineariza — e a reconstrução fecha com resíduo 0.\n\n");
{
    V E[N], B[N], Er[N];
    double complex cx[N], cy[N];
    campo(E, B);
    for(int k = 0; k < N; k++){                    /* IDA: decompor na base (Teorema 2.1) */
        double complex sx = 0, sy = 0;
        for(int j = 0; j < N; j++){
            double complex w = cexp(-2.0*M_PI*I*k*j/N);
            sx += E[j].x*w; sy += E[j].y*w;
        }
        cx[k] = sx/N; cy[k] = sy/N;
    }
    for(int j = 0; j < N; j++){                    /* VOLTA: recompor */
        double complex sx = 0, sy = 0;
        for(int k = 0; k < N; k++){
            double complex w = cexp(2.0*M_PI*I*k*j/N);
            sx += cx[k]*w; sy += cy[k]*w;
        }
        Er[j].x = creal(sx); Er[j].y = creal(sy); Er[j].z = 0;
    }
    double res = 0, escala = 0, nE = 0, nc = 0;
    for(int j = 0; j < N; j++){
        res += fabs(E[j].x-Er[j].x) + fabs(E[j].y-Er[j].y);
        escala += fabs(E[j].x) + fabs(E[j].y);
        nE += E[j].x*E[j].x + E[j].y*E[j].y;
    }
    for(int k = 0; k < N; k++) nc += creal(cx[k]*conj(cx[k]) + cy[k]*conj(cy[k]));
    printf("      resíduo da reconstrução: %.3e   (sobre escala %.3f)\n", res, escala);
    printf("      Parseval: Σ‖E‖²/N = %.9f   vs   Σ|c_k|² = %.9f\n\n", nE/N, nc);
    ok("a linearização é reversível: decompor e recompor devolve o campo, resíduo ~0",
       res/escala < 1e-13);
    ok("e nada vaza: Parseval fecha — é o Teorema 2.1 (milenio.c §M1)",
       fabs(nE/N - nc) < 1e-12);
    printf("      Este é o primeiro meio-arco, e ele fecha SOZINHO — ida e volta pela mesma\n");
    printf("      porta. O que falta é a outra metade: voltar pela porta DUAL.\n");
}

printf("\n§T6  O ESPELHO AO CONTRÁRIO: Hodge preserva, ν∘rev INVERTE.\n\n");
{
    V E[N], B[N], hE[N], hB[N], vE[N], vB[N];
    campo(E, B);
    double h0 = somaH(E,B), s0 = impedancia(E,B);
    hodge(E, B, hE, hB);
    espelho(E, B, vE, vB);
    double hH = somaH(hE,hB), hV = somaH(vE,vB);
    double sH = impedancia(hE,hB), sV = impedancia(vE,vB);
    printf("      operação      Σh = Σ log‖E×B‖      log σ         veredito\n");
    printf("      original      %+-20.9f %+-13.9f —\n", h0, s0);
    printf("      Hodge         %+-20.9f %+-13.9f %s\n", hH, sH,
           fabs(hH - h0) < 1e-9 ? "PRESERVA Σh — meia dualidade" : "muda Σh");
    printf("      ν∘rev         %+-20.9f %+-13.9f %s\n\n", hV, sV,
           fabs(hV + h0) < 1e-9 ? "INVERTE Σh — a dualidade inteira" : "não inverte");
    ok("Hodge PRESERVA o Poynting — é só a permutação, e falta-lhe o sinal",
       fabs(hH - h0) < 1e-9);
    ok("ν∘rev INVERTE o Poynting: Σh -> -Σh", fabs(hV + h0) < 1e-9);
    /* E a conta que a medida corrigiu. Eu tinha escrito que ν∘rev inverte AS DUAS — o Poynting
     * e a impedância. Os números dizem outra coisa, e é melhor:
     *     Hodge:  |E'| = |B|,   |B'| = |E|      =>  σ' = 1/σ   INVERTE σ, preserva Σh
     *     ν∘rev:  |E'| = 1/|B|, |B'| = 1/|E|    =>  σ' = σ     preserva σ, INVERTE Σh
     * Cada uma inverte exatamente UMA das duas. Nenhuma é a dualidade inteira sozinha. */
    ok("Hodge inverte a IMPEDÂNCIA e ν∘rev preserva-a — o oposto do Poynting",
       fabs(sH + s0) < 1e-9 && fabs(sV - s0) < 1e-9);
    {   /* logo a dualidade INTEIRA é a composição das duas: inverte as duas grandezas */
        V cE[N], cB[N];
        hodge(vE, vB, cE, cB);
        double hC = somaH(cE,cB), sC = impedancia(cE,cB);
        printf("      Hodge∘ν∘rev   %+-20.9f %+-13.9f %s\n\n", hC, sC,
               (fabs(hC + h0) < 1e-9 && fabs(sC + s0) < 1e-9)
               ? "INVERTE AS DUAS — a dualidade inteira" : "não inverte as duas");
        ok("a dualidade INTEIRA é a composição: só ela inverte Poynting E impedância",
           fabs(hC + h0) < 1e-9 && fabs(sC + s0) < 1e-9);
    }
    V wE[N], wB[N];
    espelho(vE, vB, wE, wB);                        /* aplicá-lo duas vezes */
    int mal = 0;
    for(int j = 0; j < N; j++)
        if(normaV((V){wE[j].x-E[j].x, wE[j].y-E[j].y, wE[j].z-E[j].z}) > 1e-12
        || normaV((V){wB[j].x-B[j].x, wB[j].y-B[j].y, wB[j].z-B[j].z}) > 1e-12) mal++;
    printf("      e (ν∘rev)² = id, em %d pontos: %d falhas\n\n", N, mal);
    ok("o espelho ao contrário tem ORDEM 2 — é uma dobra, e desdobra-se aplicando-o",
       mal == 0);
    printf("      E aqui a medida corrigiu-me. Eu ia escrever que ν∘rev inverte AS DUAS coisas.\n");
    printf("      Não inverte: cada uma inverte exatamente UMA.\n\n");
    printf("          Hodge:  |E'| = |B|,   |B'| = |E|     -> σ vira 1/σ,  Σh fica\n");
    printf("          ν∘rev:  |E'| = 1/|B|, |B'| = 1/|E|   -> σ fica,      Σh vira -Σh\n\n");
    printf("      Por isso Hodge é MEIA dualidade — e ν∘rev é a OUTRA metade, não o todo. A\n");
    printf("      dualidade inteira é a composição, e só ela vira as duas. É o mesmo padrão do\n");
    printf("      §B12: uma torre sozinha não fecha, é o par que fecha.\n");
    printf("\n      E tem ORDEM 2, como o conj (§B14), o i* (§U2) e o Γ̂̂ = Γ (§M5). O espelho\n");
    printf("      guarda a memória do que refletiu — e é por isso que se pode voltar por ele.\n");
}

printf("\n§T7  E FECHA o circuito — ida e volta, resíduo 0.\n\n");
{
    V E0[N], B0[N], E[N], B[N], tE[N], tB[N];
    campo(E0, B0);
    memcpy(E, E0, sizeof E); memcpy(B, B0, sizeof B);
    printf("      etapa                                  resíduo        Σh\n");
    printf("      0. o fluido, como veio                 %-14.1e %+.9f\n", 0.0, somaH(E,B));

    double complex cx[N], cy[N];
    for(int k = 0; k < N; k++){                    /* 1. LINEARIZA */
        double complex sx = 0, sy = 0;
        for(int j = 0; j < N; j++){
            double complex w = cexp(-2.0*M_PI*I*k*j/N);
            sx += E[j].x*w; sy += E[j].y*w;
        }
        cx[k] = sx/N; cy[k] = sy/N;
    }
    printf("      1. linearizado (está na base)          %-14s %s\n", "—", "(no índice)");

    for(int j = 0; j < N; j++){                    /* 2. RESOLUÇÃO, e confere o resíduo */
        double complex sx = 0, sy = 0;
        for(int k = 0; k < N; k++){
            double complex w = cexp(2.0*M_PI*I*k*j/N);
            sx += cx[k]*w; sy += cy[k]*w;
        }
        E[j].x = creal(sx); E[j].y = creal(sy); E[j].z = 0;
    }
    double r2 = 0;
    for(int j = 0; j < N; j++) r2 += fabs(E[j].x-E0[j].x) + fabs(E[j].y-E0[j].y);
    printf("      2. resolvido, resíduo conferido        %-14.1e %+.9f\n", r2, somaH(E,B));

    espelho(E, B, tE, tB);                         /* 3. O ESPELHO AO CONTRÁRIO */
    memcpy(E, tE, sizeof E); memcpy(B, tB, sizeof B);
    printf("      3. voltou pelo espelho (ν∘rev)         %-14s %+.9f  <- INVERTEU\n", "—",
           somaH(E,B));

    espelho(E, B, tE, tB);                         /* 4. FECHA */
    memcpy(E, tE, sizeof E); memcpy(B, tB, sizeof B);
    double rf = 0, esc = 0;
    for(int j = 0; j < N; j++){
        rf  += normaV((V){E[j].x-E0[j].x, E[j].y-E0[j].y, E[j].z-E0[j].z});
        rf  += normaV((V){B[j].x-B0[j].x, B[j].y-B0[j].y, B[j].z-B0[j].z});
        esc += normaV(E0[j]) + normaV(B0[j]);
    }
    printf("      4. FECHOU o circuito                   %-14.1e %+.9f\n\n", rf, somaH(E,B));
    ok("O CIRCUITO FECHA: fluido -> base -> resíduo 0 -> espelho -> volta, exato",
       rf/esc < 1e-13);
    printf("      E é isto que se promete, e só isto. Não se promete chegar: quem vai não tem\n");
    printf("      garantia de voltar, e o §T2 mostra que essa garantia não existe nem em\n");
    printf("      princípio — decidir a fronteira É decidir a parada, e nenhum orçamento finito\n");
    printf("      basta. Pode acabar a memória, podem acabar os recursos, pode morrer no\n");
    printf("      caminho. Todo mundo só tem a garantia de que vai morrer, nada mais.\n");
    printf("\n      Mas quando fecha, FECHOU — e o resíduo diz que fechou. É verificação a\n");
    printf("      posteriori, não promessa a priori, e é a única espécie de certificado que\n");
    printf("      cabe num sistema finito. \"Chega sempre?\" é indecidível; \"fechou desta vez?\"\n");
    printf("      tem resposta, e a resposta é um número.\n");
    printf("\n      E é por isso que a formulação estática vai ao lixo, sem drama nenhum: ela\n");
    printf("      pede uma garantia que ninguém pode dar, sobre objetos que só fecham no\n");
    printf("      limite, depois de ter jogado fora o gerador que os fazia mexer. Nós queremos\n");
    printf("      outra coisa — o gerador, o circuito, o resíduo — e essa nós medimos.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
