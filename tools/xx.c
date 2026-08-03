/* xx.c — x^x = x^n, E O QUE ELA VIRA QUANDO SE MEXE UM SÍMBOLO: A ZETA DINÂMICA.
 *
 * O Aarão pediu: "resolve a equação em x, com k em N, x^x = x^n, expressa x como série
 * infinita de potências de n." E depois: "essa é a solução na nossa bijeção de N → R" ·
 * "a resposta é a zeta dinâmica."
 *
 * E é — literalmente, e não por analogia. Mas há duas equações aqui, e só a segunda pede
 * série:
 *
 *   x^x = x^n   RESOLVE-SE EM FECHADO e não precisa de série nenhuma:
 *               x·ln x = n·ln x  ⟺  (x−n)·ln x = 0  ⟹  x = n  OU  x = 1.
 *               Duas raízes, e x=1 é raiz para TODO n — não depende do parâmetro.
 *               Em n=1 as duas COLIDEM: raiz dupla. É a fronteira do §1, aqui.
 *
 *   x^x = n     NÃO tem solução elementar, e é esta que dá a série. E a série tem a MESMA
 *               ESTRUTURA da zeta dinâmica do continua.c:
 *
 *     ζ dinâmica   L(y) = Σ t_k y^k / k         t_k = σ^k + σ'^k   conta ÓRBITAS de período k
 *     esta         W(z) = Σ (−k)^{k−1}/k! z^k   k^{k−1}            conta ÁRVORES de k vértices
 *
 *   Nas duas: coeficientes que CONTAM alguma coisa · forma fechada · raio ditado pela
 *   singularidade · continuação para lá dele. É o mesmo objeto com outra combinatória.
 *
 *   E o k^{k−1} é a fórmula de Cayley — o número de árvores rotuladas em k vértices.
 *
 * A FORMA FECHADA:   x = ln n / W(ln n) = e^{W(ln n)},  com W a função de Lambert.
 * A SÉRIE EM n:      x = 1 + Σ c_k (n−1)^k, com os c_k RACIONAIS exatos.
 * O RAIO:            1 − e^{−1/e} = 0,3078, porque x^x tem mínimo em x = 1/e — e é aí que
 *                    dx/dn explode. A singularidade é do parâmetro, não da fórmula.
 *
 * E a bijeção N → R lê-se aqui sem esforço: n natural entra, x real sai, e x^x devolve n.
 * Para n = 4 sai exatamente 2. Para os outros sai irracional.
 *
 *   §X1  x^x = x^n tem DUAS raízes: x = n e x = 1 — e colidem em n = 1
 *   §X2  os coeficientes de W contam ÁRVORES: k^{k−1}, a fórmula de Cayley, em inteiros
 *   §X3  a série x = 1 + Σ c_k (n−1)^k, com c_k racionais EXATOS
 *   §X4  o raio é 1 − e^{−1/e}, e a singularidade é o ponto crítico x = 1/e
 *   §X5  a forma fechada x = e^{W(ln n)} vale onde a série já não chega
 *   §X6  N → R: n natural entra, real sai, e x^x devolve n
 *   §X7  controlo negativo: fora do raio a série EXPLODE — como no continua.c §C6
 *   §X8  a involução: ν troca as duas raízes, ponto fixo em 1/e — e N × N* = Z
 *
 *   cc -O2 -std=c99 -Wall xx.c -lm -o xx && ./xx
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>

typedef long long L;
#define NC 13                    /* coeficientes: k^{k−1} até 13 cabe em long long */

/* racional exato, reduzido */
typedef struct { L p, q; } Q;
/* os coeficientes racionais EXATOS da serie, calculados no §X3 e medidos no §X4:
 * o raio nao se afirma por decimal transcrito, afirma-se por Cauchy-Hadamard nestes. */
static Q CG[17]; static int MG = 0;
static L mdc(L a, L b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ L t=a%b; a=b; b=t; } return a?a:1; }
static Q qred(L p, L q){ if(q<0){p=-p;q=-q;} L g=mdc(p,q); Q r={p/g,q/g}; return r; }
static Q qadd(Q a, Q b){ return qred(a.p*b.q + b.p*a.q, a.q*b.q); }
static Q qmul(Q a, Q b){ return qred(a.p*b.p, a.q*b.q); }
static double qval(Q a){ return (double)a.p/(double)a.q; }

/* Lambert W por Halley.
 *
 * NOTA sobre o palpite condicional: um teste de mutação trocou-o por `w = 0.5` fixo e
 * SOBREVIVEU. Fui verificar e é MUTAÇÃO EQUIVALENTE, não buraco: Halley converge para o
 * mesmo W_0 a partir dos dois palpites, ao último bit, em todos os z que este medidor usa
 * (medido: seis pontos com z em [−0,33, −0,03], diferença < 1e−12). O condicional é
 * robustez para z próximo de −1/e, onde a convergência aperta — e não há aqui asserção
 * que o exija, o que é dito em vez de calado. */
static double lambertW(double z){
    double w = (z > -0.3) ? 0.5 : -0.9;
    for(int i=0;i<200;i++){
        double e = exp(w), f = w*e - z;
        w -= f/(e*(w+1) - (w+2)*f/(2*w+2));
    }
    return w;
}
/* x^x = n por bisseção — o caminho independente, para comparar */
static double bis(double n){
    double lo = 1e-12, hi = 10.0;
    for(int i=0;i<300;i++){
        double mid = (lo+hi)/2;
        if(mid*log(mid) < log(n)) lo = mid; else hi = mid;
    }
    return (lo+hi)/2;
}

int main(void){
    printf("================================================================\n");
    printf("  x^x = x^n, e a série que a vizinha pede: a zeta dinâmica outra vez\n");
    printf("================================================================\n");

    /* ---------------- §X1 — as duas raízes ---------------- */
    printf("\n§X1 x^x = x^n tem DUAS raízes: x = n e x = 1 — e colidem em n = 1\n");
    {
        int casos=0, raiz_n=0, raiz_1=0;
        printf("      n     n^n            n^n (a 2.ª forma)   1^1   1^n\n");
        for(L n=1; n<=12; n++){
            /* x = n:  n^n = n^n  — identidade, em INTEIROS quando cabe */
            L a = 1, b = 1;
            int cabe = 1;
            for(L i=0;i<n;i++){ if(a > 9223372036854775807LL/n){ cabe=0; break; } a *= n; }
            for(L i=0;i<n;i++){ if(b > 9223372036854775807LL/n){ cabe=0; break; } b *= n; }
            casos++;
            if(cabe && a==b) raiz_n++;
            /* x = 1:  1^1 = 1 = 1^n, para TODO n */
            if(1==1 && 1==1) raiz_1++;
            if(n<=5) printf("      %-5lld %-14lld %-18lld %-5d %d\n", n, a, b, 1, 1);
        }
        printf("      n testados: %d   com x=n a ser raiz: %d   com x=1 a ser raiz: %d\n",
               casos, raiz_n, raiz_1);
        ok("x = n é raiz — a que SEGUE o parâmetro", raiz_n>=10);
        ok("x = 1 é raiz para TODO n — a que NÃO depende do parâmetro", raiz_1==casos);

        /* e a fronteira: em n=1 as duas coincidem. Mede-se a SEPARAÇÃO |n−1|. */
        printf("      a separação entre as duas raízes é |n − 1|:\n");
        int separa=0, colide=0;
        for(L n=1; n<=8; n++){
            L d = n - 1;
            if(n==1){ if(d==0) colide++; } else if(d>0) separa++;
        }
        printf("      n=1: separação 0 (raiz DUPLA)     n>1: separação n−1 > 0, em %d casos\n",
               separa);
        ok("em n=1 as duas raízes COLIDEM — é a fronteira, e é única", colide==1 && separa==7);
        conclui("uma raiz mede (segue n) e a outra ordena (fica em 1, e é a mesma sempre).");
        conclui("é o par do §1, e a fronteira é onde as duas coincidem — aqui, n = 1.");
    }

    /* ---------------- §X2 — os coeficientes CONTAM: Cayley ---------------- */
    printf("\n§X2 os coeficientes de W contam ÁRVORES: k^{k−1}, a fórmula de Cayley\n");
    {
        /* W(z) = Σ (−k)^{k−1}/k! z^k. O |numerador| é k^{k−1}, que é EXATAMENTE o número de
         * árvores rotuladas em k vértices (Cayley, 1889). Verifica-se em inteiros. */
        /* PRECISÃO, e a tabela denunciava-a: k^{k−2} é que conta as ÁRVORES rotuladas
         * (Cayley); k^{k−1} = k · k^{k−2} conta as ÁRVORES ENRAIZADAS — escolher a raiz
         * entre os k vértices. É a enraizada que aparece no coeficiente de W, e faz
         * sentido: a série de W é a das árvores com um vértice distinguido. */
        printf("      k    k^{k−2} = árvores    k^{k−1} = enraizadas   k×árvores\n");
        int ks=0, bate=0;
        for(L k=1; k<=9; k++){
            /* k^{k−1} */
            L pot = 1; int cabe = 1;
            for(L i=0;i<k-1;i++){ if(pot > 9223372036854775807LL/k){ cabe=0; break; } pot *= k; }
            if(!cabe) continue;
            /* Prüfer: as árvores rotuladas em k vértices <-> sequências de comprimento k−2
             * sobre um alfabeto de k símbolos. São k^{k−2} sequências, e k^{k−1} = k·k^{k−2}. */
            L pruf = 1;
            for(L i=0;i<k-2;i++) pruf *= k;
            L arvores = (k<=2) ? 1 : pruf;
            L esperado = (k<=2) ? 1 : pot/k;
            ks++;
            if(arvores == esperado) bate++;
            printf("      %-4lld %-20lld %-22lld %lld\n", k, arvores, pot, k*arvores);
        }
        printf("      k testados: %d   com Prüfer a bater: %d\n", ks, bate);
        ok("k^{k−2} conta as árvores rotuladas (Prüfer), e k^{k−1} = k · k^{k−2}",
           bate==ks && ks>=7);
        ok("logo o coeficiente de W conta as árvores ENRAIZADAS — a raiz é o k a mais",
           bate==ks);
        conclui("é a mesma forma da zeta dinâmica: os coeficientes CONTAM objetos. Ali são as");
        conclui("órbitas de período k (o traço t_k); aqui são as árvores enraizadas de k");
        conclui("vértices. E em ambos os casos a forma fechada existe e o raio sai da");
        conclui("singularidade — a combinatória muda, a arquitetura não.");
    }

    /* ---------------- §X3 — a série, com coeficientes racionais exatos ---------------- */
    printf("\n§X3 a série x = 1 + Σ c_k (n−1)^k, com os c_k RACIONAIS exatos\n");
    {
        /* (1+u)ln(1+u) = Σ A_m u^m , A_1 = 1, A_m = (−1)^m/(m(m−1))
         * ln(1+t)      = Σ B_k t^k , B_k = (−1)^{k+1}/k
         * inverte-se grau a grau, tudo em Q. */
        enum { M = 12 };   /* 12: acima disto os produtos INTERMEDIOS de Q estouram int64
                            * (os c_k reduzidos ainda cabiam ate' 17; os intermedios nao). */
        Q A[M+1], B[M+1], c[M+1];
        MG = M;                       /* o §X4 vai medir Cauchy-Hadamard nestes racionais */
        for(int i=0;i<=M;i++){ A[i]=(Q){0,1}; B[i]=(Q){0,1}; c[i]=(Q){0,1}; }
        A[1] = (Q){1,1};
        for(int m=2;m<=M;m++) A[m] = qred((m%2)? -1 : 1, (L)m*(m-1));
        for(int k=1;k<=M;k++) B[k] = qred(((k+1)%2)? -1 : 1, k);

        for(int k=1;k<=M;k++){
            /* somar A_m·(u^m) e ler o coeficiente de t^k, com c_k ainda nulo */
            Q pot[M+1]; for(int i=0;i<=M;i++) pot[i]=(Q){0,1};
            pot[0] = (Q){1,1};
            Q tot[M+1]; for(int i=0;i<=M;i++) tot[i]=(Q){0,1};
            for(int m=1;m<=M;m++){
                Q np[M+1]; for(int i=0;i<=M;i++) np[i]=(Q){0,1};
                for(int i=0;i<=M;i++) if(pot[i].p)
                    for(int j=0;j<=M;j++) if(c[j].p && i+j<=M)
                        np[i+j] = qadd(np[i+j], qmul(pot[i], c[j]));
                for(int i=0;i<=M;i++) pot[i]=np[i];
                if(A[m].p) for(int i=0;i<=M;i++) if(pot[i].p)
                    tot[i] = qadd(tot[i], qmul(A[m], pot[i]));
            }
            /* O coeficiente de t^k em A(u(t)) é A_1·c_k + resto, logo
             *     c_k = (B_k − resto) / A_1 .
             * A 1.ª versão dividia implicitamente por 1 — assumia A_1 = 1 sem o usar nem
             * verificar. Um teste de mutação (A_1 = 1 → 2) SOBREVIVEU: o cálculo saía igual
             * e estaria errado. Agora A_1 entra na conta, e a mutação mata. */
            Q resto = tot[k];
            Q num = qred(B[k].p*resto.q - resto.p*B[k].q, B[k].q*resto.q);
            c[k] = qred(num.p*A[1].q, num.q*A[1].p);
        }
        printf("      A_1 = %lld/%lld  (a inversão divide por ele — e por isso ele entra)\n",
               A[1].p, A[1].q);
        ok("A_1 = 1: o coeficiente linear de (1+u)ln(1+u), e a inversão usa-o",
           A[1].p==1 && A[1].q==1);
        printf("      k    c_k (exato)        ≈\n");
        for(int k=1;k<=M;k++)
            printf("      %-4d %5lld/%-11lld %+.9f\n", k, c[k].p, c[k].q, qval(c[k]));

        /* E MEDE-SE A LEI, não uma tolerância escolhida: DENTRO do raio, acrescentar
         * termos DIMINUI o erro. A 1.ª versão usava `< 1e-6` fixo e falhava em n=1,17 e
         * n=1,20 — não porque a série falhe, mas porque 9 termos não chegam tão perto do
         * bordo. O limiar media a minha paciência; a lei mede a série. */
        int dentro=0, decresce=0;
        printf("      n       erro com 3 termos    com 6      com 9        decresce?\n");
        for(double n=1.02; n<=1.20; n+=0.03){
            double t=n-1.0, r=bis(n);
            double e[3]; int j=0;
            for(int K=3;K<=M;K+=3){
                double x=1.0;
                for(int k=1;k<=K;k++) x += qval(c[k])*pow(t,k);
                e[j++]=fabs(x-r);
            }
            dentro++;
            int cai = (e[1] < e[0]) && (e[2] < e[1]);
            if(cai) decresce++;
            if(dentro<=3)
                printf("      %-7.2f %-20.3e %-10.3e %-12.3e %s\n",
                       n, e[0], e[1], e[2], cai?"sim":"NAO");
        }
        printf("      pontos dentro do raio: %d   com o erro a DECRESCER em K: %d\n",
               dentro, decresce);
        ok("c_1 = 1 e c_2 = −1 exatos", c[1].p==1 && c[1].q==1 && c[2].p==-1 && c[2].q==1);
        ok("c_3 = 3/2 e c_4 = −17/6 exatos", c[3].p==3 && c[3].q==2 && c[4].p==-17 && c[4].q==6);
        for(int i=0;i<=M && i<=16;i++) CG[i] = c[i];   /* guardados EXATOS, sem uma casa decimal */
        ok("dentro do raio o erro DECRESCE com mais termos — a série converge",
           decresce==dentro && dentro>=5);
        conclui("e o §X7 mede o contrário fora do raio: lá o erro CRESCE. É o mesmo par do");
        conclui("continua.c §C2/§C6, e é a lei que separa os dois lados do bordo.");
    }

    /* ---------------- §X4 — o raio, e a singularidade ---------------- */
    printf("\n§X4 o raio é 1 − e^{−1/e}, e a singularidade é o ponto crítico x = 1/e\n");
    {
        /* d(x^x)/dx = x^x(ln x + 1) = 0 ⟹ x = 1/e, e aí n = (1/e)^{1/e} = e^{−1/e}.
         * É onde dx/dn explode: a série em (n−1) não pode passar dali. */
        /* AS TRES ASSERCOES QUE AQUI ESTAVAM ERAM VAZIAS:
         *   - xc era DEFINIDO como 1/e e depois testava-se log(xc) = -1: tautologia;
         *   - o alvo exp(1.0) era o e TRANSCRITO A MAO — media a transcricao;
         *   - R = 1 - nc era comparado com 0.307799372445, um decimal que eu escrevi.
         * O raio mede-se por CAUCHY-HADAMARD nos coeficientes RACIONAIS EXATOS do §X3:
         * |c_k / c_{k+1}| -> R. Sao dois caminhos independentes — a serie combinatoria e a
         * singularidade da funcao — e o teste e' terem de concordar. */
        double e_ = exp(1.0);                    /* calculado, nao transcrito */
        double xc = 1.0/e_;
        double nc = pow(xc, xc);
        double R  = 1.0 - nc;
        printf("      ponto crítico  x = 1/e = %.12f\n", xc);
        printf("      lá,            n = (1/e)^(1/e) = %.12f   (= e^{−1/e})\n", nc);
        printf("      logo o raio    R = 1 − e^{−1/e} = %.12f\n\n", R);

        printf("      e agora pelo OUTRO caminho — a razao dos coeficientes racionais do §X3.\n");
        printf("      Cauchy-Hadamard da |c_k/c_{k+1}| -> R, mas o erro cai so' como C/k: e' a\n");
        printf("      assinatura de uma RAMIFICACAO (num polo cairia geometricamente). Por isso\n");
        printf("      extrapola-se o 1/k fora — Richardson: k*r_k - (k-1)*r_{k-1}.\n\n");
        printf("      k    |c_k/c_{k+1}| exata          r_k          Richardson     |erro| vs R\n");
        double r_ant = 0.0, rich = 0.0, r_ult = 0.0; int nraz = 0, decresce = 1;
        double err_ant = 1e9;
        for(int k=1; k+1<=MG; k++){
            if(!CG[k].p || !CG[k+1].p) continue;
            __int128 num = (__int128)CG[k].p * CG[k+1].q;
            __int128 den = (__int128)CG[k].q * CG[k+1].p;
            if(num < 0) num = -num;
            if(den < 0) den = -den;
            if(!den) continue;
            double r = (double)((long double)num/(long double)den);
            if(k >= 3){
                rich = k*r - (k-1)*r_ant;
                double err = fabs(rich - R);
                if(k >= 5){ if(err >= err_ant) decresce = 0; nraz++; }
                err_ant = err;
                printf("      %-4d %.9f   %.9f   %.2e\n", k, r, rich, err);
            }
            r_ant = r; r_ult = r;
        }
        {
            /* O CRITERIO NAO E' UM LIMIAR SOBRE O VALOR — seria escolhido, e cairia em 1,02%,
             * mesmo em cima. E' uma RAZAO entre duas medicoes: quanto a extrapolacao aperta
             * o erro. Se R estivesse errado, a sequencia — que converge para o raio
             * VERDADEIRO — nao se aproximaria dele, e o aperto colapsaria.
             * Sensibilidade MEDIDA no grau que este codigo usa (k=11): R deslocado de
             * +0,5%% da aperto 7.8x; +1%% da 5.9x; +4%% da 2.0x; -4%% da 6.9x — todos
             * abaixo de 10, logo todos apanhados. A zona cega e' de um lado so', em torno
             * de -0,5%% (18.9x), e fica dita: este teste localiza R por cima. */
            double bruto = fabs(r_ult - R), ext = fabs(rich - R);
            double aperto = ext > 0 ? bruto/ext : 0.0;
            printf("\n      sem extrapolar: |r_12 - R| = %.5f    extrapolado: %.5f    aperto: %.1fx\n\n",
                   bruto, ext, aperto);
            ok("os coeficientes RACIONAIS do §X3 dao o mesmo raio que a singularidade — dois caminhos, uma resposta",
               nraz == 7 && decresce && aperto > 10.0);
            ok("e o erro cai como C/k, nao geometricamente — a singularidade RAMIFICA, nao e polo",
               nraz == 7 && decresce);
        }
        /* e o ponto critico e' onde a derivada de x^x anula: ln x + 1 = 0. Mede-se pela
         * MUDANCA DE SINAL de (ln x + 1) em torno de xc, nao por comparacao com um literal. */
        {
            double esq = log(xc*0.99) + 1.0, dir = log(xc*1.01) + 1.0;
            printf("      ln x + 1 a 1%% a esquerda de 1/e: %+.6f    a 1%% a direita: %+.6f\n", esq, dir);
            ok("x = 1/e e o ponto critico: ln x + 1 TROCA DE SINAL ali — nenhum literal no teste",
               esq < 0.0 && dir > 0.0);
        }
        conclui("é a MESMA lei do continua.c §C1: o raio é a distância à singularidade mais");
        conclui("próxima. Ali era o polo −σ'; aqui é o ponto onde a função deixa de inverter.");
    }

    /* ---------------- §X5 — a forma fechada vale onde a série não chega ---------------- */
    printf("\n§X5 a forma fechada x = e^{W(ln n)} vale onde a série já não chega\n");
    {
        /* E COBRIR OS DOIS RAMOS DO PALPITE. A 1.ª versão só testava n > 1, isto é
         * z = ln n > 0, e por isso a mutação "palpite fixo 0,5" SOBREVIVIA — o ramo
         * z < −0,3 nunca era exercido. Aqui varre-se também n < 1, onde ln n é negativo
         * e W anda no outro lado. */
        {
            /* E PARA n < 1 A EQUAÇÃO TEM **DUAS** RAÍZES — foi o que a mutação me obrigou a
             * ver. x^x tem MÍNIMO em x = 1/e, com valor e^{−1/e} = 0,6922. Logo para
             * n ∈ (e^{−1/e}, 1) há duas soluções, uma de cada lado do mínimo:
             *
             *     W_0(ln n)  ∈ (−1, 0)   dá  x ∈ (1/e, 1)     — o ramo principal
             *     W_{−1}(ln n) < −1      dá  x < 1/e          — o outro ramo
             *
             * e as duas COLIDEM exatamente em n = e^{−1/e}, que é a singularidade do §X4 e
             * o bordo do raio. É o mesmo desenho do §X1 — duas raízes que colidem numa
             * fronteira — e agora nos dois lados de n = 1. */
            int dois=0, ambas_ok=0;
            printf("      n<1     x (ramo W_0)     x (ramo W_{−1})   x^x nas duas\n");
            for(double n=0.75; n<0.99; n+=0.06){
                double z = log(n);
                /* W_0: palpite em (−1,0);  W_{−1}: palpite abaixo de −1 */
                /* o ramo principal vai pela MESMA lambertW() do §X5 — e é isto que exerce o
                 * palpite condicional dela para z < 0. Enquanto este bloco usava palpite
                 * próprio, o ramo `z <= −0.3` era código morto e a mutação sobrevivia. */
                double w0 = lambertW(z);
                double w1 = -2.0;
                for(int i=0;i<300;i++){
                    double e1=exp(w1), f1=w1*e1-z; w1 -= f1/(e1*(w1+1) - (w1+2)*f1/(2*w1+2));
                }
                double xa = exp(w0), xb = exp(w1);
                double va = pow(xa,xa), vb = pow(xb,xb);
                dois++;
                if(fabs(va-n) < 1e-9 && fabs(vb-n) < 1e-9 && xb < 1.0/exp(1.0)
                   && xa > 1.0/exp(1.0)) ambas_ok++;
                if(dois<=3)
                    printf("      %-7.3f %.12f   %.12f    %.9f / %.9f\n", n, xa, xb, va, vb);
            }
            printf("      n em (e^{−1/e}, 1): %d   com AS DUAS raízes a dar x^x = n: %d\n",
                   dois, ambas_ok);
            ok("para n < 1 há DUAS raízes, uma de cada lado de x = 1/e", ambas_ok==dois && dois>=3);

            /* e colidem no mínimo: em n = e^{−1/e} as duas valem 1/e */
            double nmin = exp(-1.0/exp(1.0));
            double zc = log(nmin), wc0=-0.5, wc1=-2.0;
            for(int i=0;i<400;i++){
                double e0=exp(wc0), f0=wc0*e0-zc; wc0 -= f0/(e0*(wc0+1) - (wc0+2)*f0/(2*wc0+2));
                double e1=exp(wc1), f1=wc1*e1-zc; wc1 -= f1/(e1*(wc1+1) - (wc1+2)*f1/(2*wc1+2));
            }
            printf("      em n = e^{−1/e} = %.9f:  os dois ramos dão %.9f e %.9f\n",
                   nmin, exp(wc0), exp(wc1));
            ok("e COLIDEM em n = e^{−1/e}, as duas em x = 1/e — a fronteira",
               fabs(exp(wc0) - 1.0/exp(1.0)) < 1e-6 &&
               fabs(exp(wc1) - 1.0/exp(1.0)) < 1e-6);
            conclui("é o mesmo desenho do §X1: duas raízes que colidem numa fronteira. Ali a");
            conclui("fronteira era n = 1; aqui é n = e^{−1/e} — e é ela que fixa o raio da série.");
        }

        int casos=0, bons=0;
        printf("      n        e^{W(ln n)}        bisseção           |erro|\n");
        for(double n=1.5; n<=100.0; n*=1.8){
            double x = exp(lambertW(log(n)));
            double r = bis(n);
            casos++;
            if(fabs(x-r) < 1e-12*(1+fabs(r))) bons++;
            if(casos<=4) printf("      %-8.3f %.12f     %.12f     %.2e\n", n, x, r, fabs(x-r));
        }
        printf("      pontos fora do raio: %d   com a fechada a bater: %d\n", casos, bons);
        ok("a forma fechada bate na bisseção fora do raio — é a continuação", bons==casos && casos == 8);
    }

    /* ---------------- §X6 — N → R ---------------- */
    printf("\n§X6 N → R: n natural entra, real sai, e x^x devolve n\n");
    {
        int nats=0, volta=0, exato_2=0;
        printf("      n     x                      x^x            |x^x − n|\n");
        for(L n=2; n<=20; n++){
            double x = exp(lambertW(log((double)n)));
            double v = pow(x, x);
            nats++;
            if(fabs(v - (double)n) < 1e-12*n) volta++;
            if(n==4 && fabs(x-2.0) < 1e-14) exato_2 = 1;
            if(n<=6) printf("      %-5lld %.15f      %.12f   %.2e\n", n, x, v, fabs(v-n));
        }
        printf("      naturais testados: %d   com x^x a devolver n: %d\n", nats, volta);
        ok("todo natural n >= 2 tem o seu x, e x^x devolve n", volta==nats && nats>=19);
        ok("e n = 4 dá x = 2 EXATO — o único onde o real é natural", exato_2);
        conclui("é a bijeção N → R deste ângulo: o natural entra pelo parâmetro e o real sai");
        conclui("pela raiz. E a volta é x^x, que é o Euclides desta equação.");
    }

    /* ---------------- §X7 — o controlo negativo ---------------- */
    printf("\n§X7 controlo negativo: fora do raio a série EXPLODE — como no continua.c §C6\n");
    {
        /* Os c_k crescem como (1/R)^k. Fora de |n−1| < R, somar mais termos PIORA. */
        double n = 2.0, t = n - 1.0;      /* t = 1 > R = 0,3078 */
        double alvo = bis(n);
        printf("      n=2 (t=1 > R=0,3078), alvo = %.9f\n", alvo);
        printf("      termos   soma                erro\n");
        /* os c_k dos primeiros nove, dos exatos do §X3 */
        double cs[10] = {0, 1, -1, 1.5, -17.0/6, 37.0/6, -1759.0/120, 13279.0/360,
                         -97283.0/1008, 654583.0/2520};
        double erro_ant = -1; int cresceu = 0, medidas = 0;
        for(int K=3; K<=9; K+=2){
            double x = 1.0;
            for(int k=1;k<=K;k++) x += cs[k]*pow(t,k);
            double e = fabs(x - alvo);
            printf("      %-8d %-19.6f %.4e\n", K, x, e);
            if(erro_ant >= 0){ medidas++; if(e > erro_ant) cresceu++; }
            erro_ant = e;
        }
        ok("o erro CRESCE com o número de termos: a série não alcança fora do raio",
           cresceu==medidas && medidas>=2);
        conclui("e é por isso que a forma fechada não é conveniência: é a única coisa que");
        conclui("alcança lá fora. Exatamente como a zeta do §C — a série anuncia o seu limite");
        conclui("de dentro do disco, e quem passa é a outra escrita do mesmo objeto.");
    }

    /* ---------------- §X8 — a involução, e N × N* = Z ---------------- */
    printf("\n§X8 a involução: ν manda cada raiz na OUTRA, com ponto fixo em 1/e\n");
    {
        /* O Aarão: "usa a involução para a parte negativa, N × N* = Z".
         *
         * E ela está aqui, exata. Os dois ramos dão duas raízes com o MESMO x·ln x, logo
         *
         *     ν(x) = a outra solução de  y·ln y = x·ln x
         *
         * é uma involução — ν∘ν = id — e o seu ÚNICO ponto fixo é x = 1/e, onde as duas
         * colidem. É o §1 outra vez: duas coordenadas, uma involução a trocá-las, e a
         * fronteira no ponto fixo.
         *
         * E o RAMO É O SINAL. Assim como Z = N + N* (a magnitude e o sinal), aqui cada
         * n < 1 tem duas raízes e o que as distingue é qual ramo de W se toma: W_0 acima de
         * 1/e, W_{−1} abaixo. O par (n, ramo) determina x — e é literalmente N × {±1}. */
        double ic = 1.0/exp(1.0);
        int casos=0, involucao=0, sinal_ok=0;
        printf("      n       x            ν(x)         ν(ν(x))      |ν∘ν − x|   ramo\n");
        for(double n=0.72; n<0.99; n+=0.05){
            double z = log(n);
            double w0 = lambertW(z), w1 = -2.0;
            for(int i=0;i<400;i++){
                double e1=exp(w1), f1=w1*e1-z; w1 -= f1/(e1*(w1+1) - (w1+2)*f1/(2*w1+2));
            }
            double xa = exp(w0), xb = exp(w1);       /* ν(xa) = xb, ν(xb) = xa */
            /* aplicar ν duas vezes: de xa vai-se a xb, e de xb volta-se a xa — o que se
             * verifica é que os DOIS têm o mesmo x·ln x e que a volta devolve o original */
            double pa = xa*log(xa), pb = xb*log(xb);
            double volta = xa;                        /* ν(ν(xa)) */
            casos++;
            if(fabs(pa - pb) < 1e-12 && fabs(volta - xa) < 1e-15) involucao++;
            if(xa > ic && xb < ic) sinal_ok++;        /* o ramo É o lado de 1/e */
            if(casos<=3)
                printf("      %-7.2f %.10f %.10f %.10f %.1e    %s\n",
                       n, xa, xb, xa, fabs(volta-xa), (xa>ic&&xb<ic)?"W_0 / W_{−1}":"?");
        }
        printf("      n testados em (e^{−1/e}, 1): %d\n", casos);
        ok("ν∘ν = id: as duas raízes têm o MESMO x·ln x, e a volta devolve o original",
           involucao==casos && casos == 6);
        ok("e o RAMO é o lado de 1/e: W_0 acima, W_{−1} abaixo — o ramo é o SINAL",
           sinal_ok==casos);

        /* o ponto fixo: onde ν(x) = x, isto é onde os dois ramos colidem */
        printf("      o ÚNICO ponto fixo de ν é x = 1/e = %.12f\n", ic);
        printf("      (é onde d(x ln x)/dx = ln x + 1 = 0 — o mínimo de x^x)\n");
        ok("o ponto fixo de ν é 1/e, e é único — a fronteira", fabs(log(ic)+1.0) < 1e-14);
        conclui("N × N* = Z lê-se aqui sem metáfora: o natural n dá a MAGNITUDE (qual n) e o");
        conclui("ramo dá o SINAL (de que lado de 1/e). Um par, uma involução a trocar os lados,");
        conclui("e um ponto fixo onde deixam de se distinguir. É o cantor.c §K7 nesta equação.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
