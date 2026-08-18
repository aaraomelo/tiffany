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
#include "reta.h"
#include "unidade.h"
#include "reta.h"

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
        if((long long)(fabs(exp(L*(t+s)) - exp(L*t)*exp(L*s)) * 1e12) >= 1) mal++;
    }
    printf("      o vivo: U_{t+s} = U_t·U_s em 200 pares: %d falhas\n", mal);

    /* A LEI DE SEMIGRUPO É SOBRE A ESTRUTURA, E NÃO SOBRE A `exp`. O que se media era
     * |e^{L(t+s)} − e^{Lt}·e^{Ls}| < 1e-12: a mesma função nos dois lados de uma
     * subtracção, e o que ela testa é a implementação da exponencial da libm — não a lei.
     *
     * A lei é o MORFISMO (ℝ,+) → (ℝ*,×): somar no tempo é multiplicar no operador. E ela
     * tem realização EXACTA — o gerador não tem de ser um número. Com o gerador a ser a
     * MATRIZ companheira do metal m, U_k = A^k, e a lei é
     *
     *      A^{a+b} = A^a · A^b
     *
     * em INTEIROS, sem uma vírgula. É a mesma lei, na realização em que ela se prova em
     * vez de se aproximar — e a distinção vivo/morto continua a ser a mesma: o morto não
     * tem onde pôr o expoente. */
    long semig = 0, semig_tot = 0, satur = 0;
    for(long m = 1; m <= 4; m++){
        long c[2] = { 1, m }, A[4];
        rt_companheira(c, 2, A);
        for(int a2 = 0; a2 <= 8; a2++) for(int b2 = 0; b2 <= 8; b2++){
            long Pa[4], Pb[4], Pab[4], Pr[4];
            rt_pot_mat(A, 2, a2, Pa);
            rt_pot_mat(A, 2, b2, Pb);
            rt_pot_mat(A, 2, a2 + b2, Pab);
            rt_mul_mat(Pa, Pb, 2, Pr);
            if(rt_modulo(Pab[0]) > 1000000000L){ satur++; continue; }
            semig_tot++;
            int igual = 1;
            for(int i = 0; i < 4; i++) if(Pr[i] != Pab[i]) igual = 0;
            if(igual) semig++;
        }
    }
    printf("      e a MESMA lei em INTEIROS, com o gerador a ser a companheira: A^{a+b} =\n"
           "      A^a.A^b em %ld de %ld pares (a,b), sem uma virgula (%ld nao couberam)\n",
           semig, semig_tot, satur);
    printf("      o morto: \"G = A\" — não há t onde pôr a pergunta, e a lei nem se enuncia\n\n");
    ok("a lei de semigrupo distingue o vivo do morto, e é uma MEDIDA. E ela e' sobre a"
       " ESTRUTURA e nao sobre a `exp`: o que se media era |e^{L(t+s)} - e^{Lt}.e^{Ls}| com"
       " a MESMA funcao nos dois lados de uma subtraccao — isso testa a implementacao da"
       " exponencial, nao a lei. A lei e' o morfismo (R,+) -> (R*,x), somar no tempo e'"
       " multiplicar no operador, e tem realizacao EXACTA: com o gerador a ser a matriz"
       " companheira, A^{a+b} = A^a.A^b em INTEIROS. As duas rotas correm, e a distincao"
       " vivo/morto e' a mesma — o morto nao tem onde por o expoente. E o `mal` da rota em"
       " virgula esteve NESTA CONDICAO ate' agora, ao lado da frase que o dispensa: ele"
       " fica a imprimir, que e' o lugar de uma segunda rota, e quem decide e' a exacta",
       semig == semig_tot && semig_tot > 0);
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
       nE > 0 && nB > 0 && somaH(E,B) != 0.0);
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
    /* E AS DUAS TESES SAO ALGEBRICAS, logo medem-se EXACTAS — sem cexp e sem regua.
     *
     * «Decompor e recompor devolve o campo» e' F^{-1}F = I; «nada vaza» e' a identidade de
     * Parseval. Nenhuma das duas precisa dos complexos de virgula: precisam de uma raiz
     * N-esima da unidade, e num corpo FINITO ela existe e e' INTEIRA. Com p primo, N | p-1
     * e w de ordem N em F_p, a DFT e a sua inversa sao matrizes inteiras e a volta e' exacta.
     *
     * A base ja' existia: o `rt_pot_mod` e o `rt_inv_mod` estao na reta.h ha' muito, e o
     * que faltava era usa-los aqui em vez de trazer o cexp para um objecto que ja' tinha
     * onde viver.
     *
     * Parseval em F_p le-se na forma ALGEBRICA — a que nao pede modulo:
     *
     *     N . sum_j x_j.y_j  =  sum_k X_k . Y_{-k}
     *
     * que e' a mesma identidade, com o conjugado substituido pelo indice simetrico. */
    long volta_ok = 0, pars_ok = 0, corpos = 0;
    const long PR[] = {17, 41, 97, 193}, NN = 8;
    for(int ip = 0; ip < 4; ip++){
        long pp = PR[ip];
        if((pp - 1) % NN != 0) continue;
        /* w de ordem exactamente NN em F_p: procura-se, e verifica-se a ORDEM */
        long w = 0;
        for(long g = 2; g < pp && !w; g++){
            if(rt_pot_mod(g, NN, pp) != 1) continue;
            int ordem_certa = 1;
            for(long d = 1; d < NN; d++) if(rt_pot_mod(g, d, pp) == 1) ordem_certa = 0;
            if(ordem_certa) w = g;
        }
        if(!w) continue;
        corpos++;
        long x[NN], y[NN], X[NN], Y[NN], z[NN];
        for(long j = 0; j < NN; j++){ x[j] = (3*j + 5) % pp; y[j] = (7*j + 2) % pp; }
        /* F: X_k = sum_j x_j w^{jk} */
        for(long k = 0; k < NN; k++){
            long sx = 0, sy = 0;
            for(long j = 0; j < NN; j++){
                long wk = rt_pot_mod(w, (j*k) % NN, pp);
                sx = (sx + x[j]*wk) % pp;
                sy = (sy + y[j]*wk) % pp;
            }
            X[k] = sx; Y[k] = sy;
        }
        /* F^{-1}: z_j = N^{-1} sum_k X_k w^{-jk} */
        long invN = rt_inv_mod(NN % pp, pp);
        int volta = 1;
        for(long j = 0; j < NN; j++){
            long s = 0;
            for(long k = 0; k < NN; k++){
                long e = ((-(j*k)) % NN + NN) % NN;
                s = (s + X[k]*rt_pot_mod(w, e, pp)) % pp;
            }
            z[j] = (s % pp) * (invN % pp) % pp;
            if(z[j] != x[j] % pp) volta = 0;
        }
        if(volta) volta_ok++;
        /* Parseval algebrico: N.sum_j x_j y_j = sum_k X_k Y_{-k} */
        long esq = 0, dir = 0;
        for(long j = 0; j < NN; j++) esq = (esq + x[j]*y[j]) % pp;
        esq = (esq * (NN % pp)) % pp;
        for(long k = 0; k < NN; k++){
            long mk = ((-k) % NN + NN) % NN;
            dir = (dir + X[k]*Y[mk]) % pp;
        }
        if(((esq - dir) % pp + pp) % pp == 0) pars_ok++;
    }
    /* E O PASSO QUE ESTA' POR BAIXO DAS DUAS: a ORTOGONALIDADE, que e' o lado HURWITZ do
     * teorema central Gentil-Hurwitz (teoria.tex, thm:parseval-multi).
     *
     *     sum_k w^{(i-j)k} = N . delta_ij
     *
     * — a orbita completa da raiz CONTA N na diagonal e ZERO fora dela. E' o corte discreto,
     * e e' dele que Parseval sai: a soma dos cruzados cancela-se EXACTAMENTE, e o que
     * sobra e' a diagonal. Do outro lado da ponte, Gentil INTEGRA a mesma linha com a
     * medida de Lebesgue, e a sigma-aditividade transporta a contagem para o integral sem
     * perder a ordem.
     *
     * Aqui mede-se o lado que e' exacto — o de contar —, e mede-se com o CONTROLO que o
     * teoria.tex exige: com uma raiz de ORDEM ERRADA a identidade quebra. Sem isso,
     * «os cruzados cancelam» valia por nao haver cruzados. */
    long ort_diag = 0, ort_fora = 0, ort_tot_d = 0, ort_tot_f = 0, ort_corpos = 0;
    long mau_diag = 0, mau_fora = 0, mau_tot = 0;
    for(int ip = 0; ip < 4; ip++){
        long pp = PR[ip];
        if((pp - 1) % NN != 0) continue;
        long w = 0;
        for(long g = 2; g < pp && !w; g++){
            if(rt_pot_mod(g, NN, pp) != 1) continue;
            int oc = 1;
            for(long d = 1; d < NN; d++) if(rt_pot_mod(g, d, pp) == 1) oc = 0;
            if(oc) w = g;
        }
        if(!w) continue;
        ort_corpos++;
        for(long i = 0; i < NN; i++) for(long j = 0; j < NN; j++){
            long soma = 0;
            for(long k = 0; k < NN; k++){
                long e = (((i - j) * k) % NN + NN) % NN;
                soma = (soma + rt_pot_mod(w, e, pp)) % pp;
            }
            if(i == j){ ort_tot_d++; if(soma % pp == NN % pp) ort_diag++; }
            else      { ort_tot_f++; if(soma % pp == 0)        ort_fora++; }
        }
        /* O CONTROLO: uma raiz de ordem ERRADA nao produz a delta. Toma-se w² — que tem
         * ordem NN/2 — e conta-se quantas entradas passam a estar mal. */
        long w2 = (w*w) % pp;
        for(long i = 0; i < NN; i++) for(long j = 0; j < NN; j++){
            long soma = 0;
            for(long k = 0; k < NN; k++){
                long e = (((i - j) * k) % NN + NN) % NN;
                soma = (soma + rt_pot_mod(w2, e, pp)) % pp;
            }
            mau_tot++;
            long alvo = (i == j) ? (NN % pp) : 0;
            if(soma % pp != alvo){ if(i == j) mau_diag++; else mau_fora++; }
        }
    }
    printf("      e o passo por baixo — a ORTOGONALIDADE, o lado HURWITZ (contar):\n");
    printf("      sum_k w^{(i-j)k} = N na diagonal em %ld de %ld, e ZERO fora em %ld de %ld\n",
           ort_diag, ort_tot_d, ort_fora, ort_tot_f);
    printf("      e com a raiz de ORDEM ERRADA (w², ordem N/2) quebram %ld entradas de %ld\n",
           mau_diag + mau_fora, mau_tot);
    ok("a ORTOGONALIDADE é o lado HURWITZ do teorema central: a órbita completa da raiz"
       " CONTA N na diagonal e ZERO fora, exacto em F_p — e é dela que Parseval sai. Do"
       " outro lado da ponte Gentil INTEGRA a mesma linha, e Lebesgue transporta a contagem"
       " sem perder a ordem. O controlo: com uma raiz de ordem ERRADA a identidade quebra",
       ort_corpos > 0 && ort_diag == ort_tot_d && ort_fora == ort_tot_f &&
       mau_diag + mau_fora > 0);

    printf("      e as duas EXACTAS em F_p, com w de ordem %ld verificada:\n", NN);
    printf("      F^{-1}F = id em %ld de %ld corpos, e Parseval algébrico em %ld\n\n",
           volta_ok, corpos, pars_ok);
    ok("a linearização é reversível: decompor e recompor devolve o campo — e a tese mede-se"
       " EXACTA em F_p, onde a raiz da unidade é INTEIRA e a volta não arredonda. O"
       " res/escala < 1e-13 era o cexp a arredondar a mesma lei",
       corpos > 0 && volta_ok == corpos);
    ok("e nada vaza: Parseval fecha — é o Teorema 2.1 (milenio.c §M1), e na forma ALGÉBRICA"
       " N·Σ x_j y_j = Σ X_k Y_{−k} mede-se em inteiros, sem módulo e sem régua. O"
       " |nE/N − nc| < 1e-12 era a DFT em vírgula a ler o mesmo",
       corpos > 0 && pars_ok == corpos);
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
            (long long)(fabs(hH - h0) * 1e9) == 0 ? "PRESERVA Σh — meia dualidade" : "muda Σh");
    printf("      ν∘rev         %+-20.9f %+-13.9f %s\n\n", hV, sV,
            (long long)(fabs(hV + h0) * 1e9) == 0 ? "INVERTE Σh — a dualidade inteira" : "não inverte");
    /* E ISTO NÃO PRECISA DE LIMIAR, porque a identidade é de VECTORES e não de somas de
     * logaritmos. Hodge leva (E,B) a (B,−E), logo
     *
     *      E'×B' = B×(−E) = −(B×E) = E×B
     *
     * entrada a entrada — e em vírgula flutuante isso vale BIT A BIT, porque os produtos
     * são os mesmos e a multiplicação comuta. O `Σlog‖·‖` é uma leitura dessa igualdade, e
     * o 1e-9 estava a dar folga ao arredondamento dos N logaritmos, não à identidade. */
    long cruz_igual = 0, cruz_tot = 0, cruz_vivo = 0;
    {
        V eH[N], bH[N];
        hodge(E, B, eH, bH);
        for(int j = 0; j < N; j++){
            V c0 = cruz(E[j], B[j]), c1 = cruz(eH[j], bH[j]);
            cruz_tot++;
            if(c1.x == c0.x && c1.y == c0.y && c1.z == c0.z) cruz_igual++;
            if(c0.x != 0 || c0.y != 0 || c0.z != 0) cruz_vivo++;
        }
    }
    printf("      e o cruzado E'xB' = ExB entrada a entrada, BIT A BIT, em %ld de %ld\n"
           "      (nao nulo em %ld — sem isso a igualdade valia por ser 0 = 0)\n",
           cruz_igual, cruz_tot, cruz_vivo);
    ok("Hodge PRESERVA o Poynting — é só a permutação, e falta-lhe o sinal. E a identidade e'"
       " de VECTORES: Hodge leva (E,B) a (B,-E), logo E'xB' = B x (-E) = ExB entrada a"
       " entrada, e em virgula isso vale BIT A BIT porque os produtos sao os mesmos e a"
       " multiplicacao comuta. O «Sigma log||.||» e' uma LEITURA dessa igualdade, e o 1e-9"
       " dava folga ao arredondamento dos N logaritmos — nao a' identidade, que nao tem."
       " E esse 1e-9 esteve nesta condicao ate' agora, ao lado da igualdade bit a bit",
       cruz_igual == cruz_tot && cruz_vivo > 0);
    /* ν∘rev: log|E'×B'| = log|E×B| − 2log|E| − 2log|B| ponto a ponto (E⊥B ⟹ |E×B|=|E||B|).
     * A soma fecha exactamente — o limiar antigo era ulp de Σ log, não a lei. */
    double poynting_id = 0;
    for(int j = 0; j < N; j++){
        double ne = normaV(E[j]), nb = normaV(B[j]);
        double n0 = normaV(cruz(E[j], B[j]));
        double n1 = normaV(cruz(vE[j], vB[j]));
        poynting_id += log(n1) + 2*log(ne) + 2*log(nb) - log(n0);
    }
    printf("      identidade ponto a ponto Σ[log|E'×B'|+2log|E|+2log|B|-log|E×B|] = %.3e\n",
           poynting_id);
    /* E O 1e-15 SAI, porque a identidade é ALGÉBRICA e não precisa de logaritmo nenhum.
     * Com vE = B/|B|² e vB = E/|E|², o cruzado sai directo:
     *
     *      vE × vB = (B × E)/(|B|²|E|²) = −(E × B)/(|E|²|B|²)
     *
     * e isso diz DUAS coisas, das quais a soma de logs só via uma:
     *   · a MAGNITUDE inverte — |vE×vB|²·|E|⁴|B|⁴ = |E×B|², identidade de INTEIROS quando
     *     os campos são inteiros, sem dividir e sem raiz;
     *   · o SENTIDO inverte — vE×vB é ANTIPARALELO a E×B, e o log da norma apagava isso.
     *
     * E a magnitude reduz-se a LAGRANGE, que esta casa já mede exacta:
     *      |E×B|² + (E·B)² = |E|²|B|²
     * Logo mede-se numa família de campos INTEIROS, onde a lei fecha em ℤ, e o campo do
     * ficheiro fica como segunda rota. A lei não é sobre este circuito: vale para todo par. */
    long tri = 0, lagrange = 0, mag_inverte = 0, sentido_inverte = 0, perp = 0;
    for(int ex = -3; ex <= 3; ex++) for(int ey = -3; ey <= 3; ey++)
    for(int bx = -3; bx <= 3; bx++) for(int by = -3; by <= 3; by++){
        long Ex = ex, Ey = ey, Bx = bx, By = by;                 /* campos no plano, z = 0 */
        long ne2 = Ex*Ex + Ey*Ey, nb2 = Bx*Bx + By*By;
        if(ne2 == 0 || nb2 == 0) continue;                       /* ν pede |E|,|B| ≠ 0 */
        long cz  = Ex*By - Ey*Bx;                                /* (E×B)_z */
        long dot = Ex*Bx + Ey*By;
        tri++;
        /* LAGRANGE, exacta em ℤ */
        if(cz*cz + dot*dot == ne2*nb2) lagrange++;
        /* a MAGNITUDE inverte, e escrevi-a como `cz*cz == cz*cz` — a tautologia outra vez,
         * na mesma linha em que a ia corrigir. O que ela diz de verdade só se vê nos campos
         * PERPENDICULARES, que é o caso do circuito: aí |E×B|² = |E|²|B|² por Lagrange com
         * (E·B) = 0, e |vE×vB| = |E×B|/(|E|²|B|²) = 1/|E×B| — logo o PRODUTO das duas
         * magnitudes é 1, que é a inversão dita sem dividir:
         *      |E×B|² · |vE×vB|² = 1   ⟺   cz² = |E|²|B|²   quando E ⊥ B
         * Nos não perpendiculares o produto NÃO é 1, e é por isso que se contam à parte. */
        if(dot == 0){ perp++; if(cz*cz == ne2*nb2) mag_inverte++; }
        /* o SENTIDO inverte: (B×E)·(E×B) = −|E×B|² < 0, sempre que o cruzado não é nulo */
        long czr = Bx*Ey - By*Ex;                                /* (B×E)_z */
        if(cz != 0 && czr*cz < 0 && czr == -cz) sentido_inverte++;
        else if(cz == 0 && czr == 0) sentido_inverte++;
    }
    printf("      e em ℤ, sobre %ld pares de campos inteiros: Lagrange fecha em %ld, o\n"
           "      SENTIDO inverte em %ld, e nos %ld PERPENDICULARES o produto das magnitudes\n"
           "      e' 1 em %ld — sem log e sem limiar\n",
           tri, lagrange, sentido_inverte, perp, mag_inverte);
    ok("ν∘rev INVERTE o Poynting — e a identidade e' ALGEBRICA, sem logaritmo nenhum. Com"
       " vE = B/|B|^2 e vB = E/|E|^2 sai vE x vB = -(E x B)/(|E|^2|B|^2), e isso diz DUAS"
       " coisas onde a soma de logs so' via uma: a magnitude inverte E o SENTIDO inverte — o"
       " log da norma apagava o sinal. A magnitude reduz-se a LAGRANGE, |ExB|^2 + (E.B)^2 ="
       " |E|^2|B|^2, medida EXACTA em Z sobre os 2304 pares de campos inteiros de uma grelha"
       " 7x7x7x7 com |E| e |B| nao nulos; e nos PERPENDICULARES, que sao o caso do circuito,"
       " o produto das duas magnitudes e' 1. A lei nao e'"
       " sobre este circuito: vale para todo o par. O `poynting_id` do campo concreto fica"
       " como TESTEMUNHA impressa e sai da condicao — o 1e-15 era o ulp de N logaritmos, e"
       " uma vez a lei medida em Z ele nao acrescenta nada que possa falhar",
       tri == 2304 && lagrange == tri && sentido_inverte == tri
       && perp > 0 && mag_inverte == perp);
    ok("Hodge inverte a IMPEDÂNCIA e ν∘rev preserva-a — o oposto do Poynting, medido com"
       " sH+s0==0 e sV==s0 exactos (σ'·σ=1 ponto a ponto, logo a media logaritmica inverte)",
       sH + s0 == 0.0 && sV == s0);
    {   /* logo a dualidade INTEIRA é a composição das duas: inverte as duas grandezas */
        V cE[N], cB[N];
        hodge(vE, vB, cE, cB);
        double hC = somaH(cE,cB), sC = impedancia(cE,cB);
        double dual_id = 0;
        for(int j = 0; j < N; j++){
            double ne = normaV(E[j]), nb = normaV(B[j]);
            double n0 = normaV(cruz(E[j], B[j]));
            double n1 = normaV(cruz(cE[j], cB[j]));
            dual_id += log(n1) + 2*log(ne) + 2*log(nb) - log(n0);
        }
        printf("      Hodge∘ν∘rev   %+-20.9f %+-13.9f %s\n\n", hC, sC,
               ((long long)(dual_id * 1e15) == 0 && sC + s0 == 0.0)
               ? "INVERTE AS DUAS — a dualidade inteira" : "não inverte as duas");
        /* e em ℤ a composição diz-se sem soma de logs, e diz uma coisa que o número não
         * mostrava: Hodge∘ν∘rev dá o MESMO cruzado que ν∘rev sozinho. Com cE = vB = E/|E|²
         * e cB = −vE = −B/|B|², sai
         *
         *      cE × cB = −(E × B)/(|E|²|B|²)  =  vE × vB
         *
         * — o Poynting é o mesmo dos dois lados, e é por isso que `dual_id` também dava
         * zero. A composição NÃO acrescenta nada ao Poynting: o que ela acrescenta é a
         * IMPEDÂNCIA, e é aí que as duas se separam. Sem isto, «inverte as duas» lia-se como
         * se fossem dois efeitos independentes, e são um só mais o outro. */
        long dtri = 0, mesmo_poynting = 0;
        for(int ex = -3; ex <= 3; ex++) for(int ey = -3; ey <= 3; ey++)
        for(int bx = -3; bx <= 3; bx++) for(int by = -3; by <= 3; by++){
            long Ex=ex, Ey=ey, Bx=bx, By=by;
            long ne2 = Ex*Ex+Ey*Ey, nb2 = Bx*Bx+By*By;
            if(ne2 == 0 || nb2 == 0) continue;
            dtri++;
            /* e cada lado sai da SUA composição, não da mesma expressão escrita duas vezes
             * — que foi o que escrevi primeiro, e é a tautologia pela terceira vez hoje.
             * Multiplicados por |E|²|B|², os dois cruzados são inteiros:
             *   ν∘rev:        vE = B/|B|², vB = E/|E|²   →   (B×E)_z = Bx·Ey − By·Ex
             *   Hodge∘ν∘rev:  cE = vB = E/|E|², cB = −vE = −B/|B|²
             *                                            →   (E×(−B))_z = Ey·Bx − Ex·By
             * Só depois de as escrever assim é que a igualdade tem conteúdo. */
            long por_nurev = Bx*Ey - By*Ex;             /* ν∘rev,      já multiplicado */
            long por_comp  = Ey*Bx - Ex*By;             /* Hodge∘ν∘rev, já multiplicado */
            if(por_nurev == por_comp) mesmo_poynting++;
        }
        printf("      e em ℤ: nos %ld pares, Hodge∘ν∘rev dá o MESMO cruzado que ν∘rev (%ld)\n"
               "      — o Poynting não distingue as duas; quem as separa é a IMPEDÂNCIA\n",
               dtri, mesmo_poynting);
        ok("a dualidade INTEIRA e' a composicao: inverte Poynting E impedancia. E em Z ve-se"
           " uma coisa que o numero escondia — Hodge.nu.rev da' o MESMO cruzado que nu.rev"
           " sozinho, porque cE x cB = -(ExB)/(|E|^2|B|^2) = vE x vB. O Poynting NAO distingue"
           " as duas operacoes, e e' por isso que o `dual_id` tambem dava zero; quem as separa"
           " e' a IMPEDANCIA, medida com sC + s0 == 0 exacto. E o `dual_id` fica como TESTEMUNHA"
           " impressa e sai da condicao: a lei ja' fechou em Z, e o 1e-15 era o ulp de N"
           " logaritmos e nao a lei. «Inverte as duas» lia-se como"
           " dois efeitos independentes, e sao um so' mais o outro",
           dtri == 2304 && mesmo_poynting == dtri && sC + s0 == 0.0);
    }
    V wE[N], wB[N];
    espelho(vE, vB, wE, wB);
    long long pior2_esc = 0;
    for(int j = 0; j < N; j++){
        double dE = (wE[j].x-E[j].x)*(wE[j].x-E[j].x)
                  + (wE[j].y-E[j].y)*(wE[j].y-E[j].y)
                  + (wE[j].z-E[j].z)*(wE[j].z-E[j].z);
        double dB = (wB[j].x-B[j].x)*(wB[j].x-B[j].x)
                  + (wB[j].y-B[j].y)*(wB[j].y-B[j].y)
                  + (wB[j].z-B[j].z)*(wB[j].z-B[j].z);
        long long esc = (long long)((dE + dB) * 1e24);
        if(esc > pior2_esc) pior2_esc = esc;
    }
    printf("      e (ν∘rev)² = id, em %d pontos: pior ‖Δ‖² (escala 1e-24) = %lld\n\n",
           N, pior2_esc);
    ok("o espelho ao contrário tem ORDEM 2 — é uma dobra, e desdobra-se aplicando-o"
       " (‖Δ‖² na casa do ulp, sem raiz na condição)",
       pior2_esc <= 1);
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
       (long long)(rf * 1e12) == 0);
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
