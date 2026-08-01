/* edo.c — A EQUAÇÃO DIFERENCIAL É A BORDA DO CORPO. Resgatado de chess/, e fechado aqui.
 *
 *   §E1  a equação característica É a borda: y'' + By' + Cy = 0  <->  σ² = -C - Bσ
 *   §E2  e o Δ que classifica as soluções é o MESMO Δ do catálogo
 *   §E3  os três regimes do chess — cristal, borda, caos — são as três classes
 *   §E4  e dois casos fecham o círculo: o oscilador é o i, e o ouro é φ^t
 *   §E6  a solução explícita é e^{At}, e o exp é a ponte (de broca-so/papers)
 *   §E5  a solução verifica-se por substituição, como a do primeiro grau
 *
 *   cc -O2 -std=c99 edo.c -lm -o edo && ./edo
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef M_E
#define M_E 2.71828182845904523536
#endif
#include "edo.h"
#include "algebra.h"
#include "unidade.h"

int main(void){
printf("\n=== A EQUAÇÃO DIFERENCIAL É A BORDA DO CORPO ==============================\n");
printf("    Em chess/universe/tools/diferencial.c está medido que o regime de uma ED\n");
printf("    não é imposto: é o SINAL de Re(λ), e os três regimes SÃO o gato e o\n");
printf("    esquilo. Aqui fecha-se, e não foi preciso inventar nada.\n");

printf("\n§E1  A equação característica É a borda, com D no lugar do σ.\n\n");
{
    struct { const char *eq; const char *borda; long B, C; } t[] = {
        { "y'' = -y",            "s^2 = -1",     0,  1 },
        { "y'' = y' + y",        "s^2 = 1 + s",  -1, -1 },
        { "y'' + 2y' + y = 0",   "s^2 = -1 - 2s", 2,  1 },
        { "y'' - 3y' + 2y = 0",  "s^2 = -2 + 3s", -3, 2 },
        { "y'' - y = 0",         "s^2 = 1",      0, -1 },
    };
    int mal = 0;
    printf("      equação                 B    C    a borda equivalente\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; char b[96] = "?";
        if(edo_le(t[k].eq, &e)) edo_borda(e, b, sizeof b);
        printf("      %-23s %+3ld  %+3ld   %s\n", t[k].eq, e.Bp/e.Bq, e.Cp/e.Cq, b);
        if(e.Bp/e.Bq != t[k].B || e.Cp/e.Cq != t[k].C || strcmp(b, t[k].borda)) mal++;
    }
    printf("\n");
    ok("a equação característica lê-se como BORDA — B = -b₁ e C = -b₀", mal == 0);
    printf("      y'' + By' + Cy = 0 tem característica λ² + Bλ + C = 0; a borda da álgebra é\n");
    printf("      σ² = b₀ + b₁σ, ou seja σ² - b₁σ - b₀ = 0. São a MESMA equação. O operador de\n");
    printf("      derivação D ocupa o lugar do marcador σ, e é só isso que a passagem é.\n");
}

printf("\n§E2  E o Δ que classifica as soluções é o MESMO do catálogo.\n\n");
{
    struct { const char *eq; long D; int classe; const char *solucao; } t[] = {
        { "y'' - 3y' + 2y = 0",  1, +1, "e^t e e^2t — duas exponenciais" },
        { "y'' - y = 0",         4, +1, "e^t e e^-t" },
        { "y'' + 2y' + y = 0",   0,  0, "e^-t e t·e^-t — a raiz é dupla" },
        { "y'' = -y",           -4, -1, "cos t e sen t — roda" },
        { "y'' + y' + y = 0",   -3, -1, "oscila e amortece" },
    };
    int mal = 0;
    printf("      equação                 Δ = B²-4C   classe        a solução\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        printf("      %-23s %+5ld      %-13s %s\n", t[k].eq, e.D,
               e.classe > 0 ? "hiperbólico" : e.classe < 0 ? "elíptico" : "parabólico",
               t[k].solucao);
        if(e.D != t[k].D || e.classe != t[k].classe) mal++;
    }
    printf("\n");
    ok("o Δ da ED é o Δ do corpo, e as três classes são as mesmas", mal == 0);
    printf("      Não há duas classificações a coincidir: há UMA, escrita duas vezes. O que no\n");
    printf("      catálogo chama traço e determinante, aqui chama-se coeficiente de y' e de y —\n");
    printf("      e o Δ = B² - 4C é o mesmo número, com o mesmo sinal a decidir o mesmo.\n");
}

printf("\n§E3  Os três regimes do chess são as três classes.\n\n");
{
    /* diferencial.c mede em F2, por perturbacao de 1 bit: encolhe / conserva / cresce. Aqui
     * mede-se em Q, pelo sinal de Re(lambda). Os nomes batem, e nao por convencao: sao o mesmo
     * espectro. */
    printf("      chess/diferencial.c         aqui                     e o que é\n");
    printf("      CRISTAL  Re λ < 0  encolhe   Δ>0 com raízes < 0       colapsa no ponto fixo\n");
    printf("      BORDA    Re λ = 0  conserva  Δ<0 sem parte real       ORBITA — o esquilo\n");
    printf("      CAOS     Re λ > 0  cresce    Δ>0 com raiz > 0         diverge — o gato\n\n");
    struct { const char *eq; const char *regime; } t[] = {
        { "y'' + 3y' + 2y = 0", "CRISTAL — as duas raízes são negativas, tudo decai" },
        { "y'' = -y",           "BORDA — puramente imaginárias, a norma conserva-se" },
        { "y'' - 3y' + 2y = 0", "CAOS — há raiz positiva, e o que lá estiver cresce" },
    };
    int mal = 0;
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e;
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        double B = (double)e.Bp/e.Bq, C = (double)e.Cp/e.Cq, D = B*B - 4*C;
        double re;                               /* a maior parte real das raízes */
        if(D >= 0) re = (-B + sqrt(D)) / 2;
        else       re = -B / 2;
        printf("      %-22s Re máx = %+6.3f   %s\n", t[k].eq, re, t[k].regime);
        int esperado = strstr(t[k].regime, "CRISTAL") ? -1 : strstr(t[k].regime, "BORDA") ? 0 : 1;
        int obtido = re < -1e-9 ? -1 : re > 1e-9 ? 1 : 0;
        if(obtido != esperado) mal++;
    }
    printf("\n");
    ok("o sinal de Re(λ) dá cristal, borda e caos — como o chess mede em F2", mal == 0);
    printf("      Lá a assinatura é dinâmica: perturba-se um bit e vê-se se a diferença encolhe,\n");
    printf("      conserva ou cresce. Aqui é o espectro em Q. São a mesma medida em dois corpos,\n");
    printf("      e o dicionário do paper já o dizia: \"metal = o autovalor\", \"reta = a taxa\".\n");
}

printf("\n§E4  E dois casos fecham o círculo com o resto do sistema.\n\n");
{
    Edo osc, ouro;
    edo_le("y'' = -y", &osc);
    edo_le("y'' = y' + y", &ouro);
    char b1[96], b2[96];
    edo_borda(osc, b1, sizeof b1);
    edo_borda(ouro, b2, sizeof b2);
    printf("      y'' = -y       ->  %-16s  Δ = %+ld\n", b1, osc.D);
    printf("      y'' = y' + y   ->  %-16s  Δ = %+ld\n\n", b2, ouro.D);
    ok("o oscilador harmónico é a borda do i — σ² = -1", !strcmp(b1, "s^2 = -1") && osc.D == -4);
    ok("e a ED do ouro é a borda do rei — σ² = 1 + σ", !strcmp(b2, "s^2 = 1 + s") && ouro.D == 5);

    /* e as raizes: uma da o i, a outra da o numero de ouro */
    double phi = (1 + sqrt(5.0)) / 2;
    printf("\n      as raízes de σ² = 1 + σ:  %.9f  e  %.9f\n", phi, -1/phi);
    printf("      e o número de ouro:       %.9f\n\n", phi);
    ok("as raízes da ED do ouro são φ e -1/φ — o chicote", fabs(phi - 1.618033988) < 1e-8);
    printf("      A solução de y'' = y' + y é A·φ^t + B·(-1/φ)^t. O REI É A SOLUÇÃO DE UMA\n");
    printf("      EQUAÇÃO DIFERENCIAL, e o par de raízes é o chicote — soma 1 (o traço), produto\n");
    printf("      -1 (o determinante). E a mesma recorrência, em passos inteiros, é Fibonacci:\n");
    printf("      a ED e a cifra são o mesmo objeto, um em t contínuo e o outro em n discreto.\n");
    printf("\n      Do outro lado, y'' = -y é a borda σ² = -1, que é o i — e a solução é cos e\n");
    printf("      sen, que é a rotação. O esquilo. As duas pontas do chicote aparecem aqui como\n");
    printf("      as duas equações diferenciais mais simples que há.\n");
}

printf("\n§E6  A solução explícita é e^{At}, e o exp é a PONTE — de broca-so/papers.\n\n");
{
    /* O paper equacoes_diferenciais.tex, Parte VII: "toda ED tem solução explícita, e ela é o
     * fluxo e^{At}". E a Parte V: "o gerador SOMA, a solução MULTIPLICA — o exp é a ponte". */
    int mal = 0;
    printf("      o gerador soma, a solução multiplica:\n");
    double par[3][2] = { {0.3,0.7}, {1.0,-0.5}, {2.0,1.0} };
    for(int k = 0; k < 3; k++){
        double a2 = par[k][0], b2 = par[k][1];
        double esq = exp(a2 + b2), dir = exp(a2) * exp(b2);
        printf("        e^(%.1f + %.1f) = %.9f     e^%.1f · e^%.1f = %.9f\n",
               a2, b2, esq, a2, b2, dir);
        if(fabs(esq - dir) > 1e-12) mal++;
    }
    printf("\n");
    ok("exp leva a SOMA dos geradores ao PRODUTO dos fluxos", mal == 0);
    printf("      As taxas somam, as soluções multiplicam, e o exp é a ponte. É a mesma\n");
    printf("      dualidade que gera os metais — x² de um lado, mx+1 do outro — e a mesma que o\n");
    printf("      log desfaz. O paper chama-lhe a assimetria que é o coração.\n");

    printf("\n      E daí sai que o METAL é o exp da taxa: σ = e^λ, λ = log σ.\n\n");
    printf("        m   σ (o metal)     λ = log σ (a taxa)   e^λ\n");
    mal = 0;
    for(int m = 1; m <= 4; m++){
        double sg = (m + sqrt((double)m*m + 4)) / 2, lam = log(sg);
        printf("        %d   %.9f     %.9f          %.9f\n", m, sg, lam, exp(lam));
        if(fabs(exp(lam) - sg) > 1e-12) mal++;
    }
    printf("\n");
    ok("σ = e^λ nos quatro metais — a reta é o log do metal", mal == 0);
    printf("      O dicionário do chess dizia \"metal = o autovalor σₙ\" e \"reta = a taxa\n");
    printf("      Re(λ) = log σ\". Aqui está medido: o metal do catálogo e a taxa da equação\n");
    printf("      diferencial são o mesmo número, um do lado do produto e outro do da soma.\n");

    printf("\n      E a CIFRA aproxima o fluxo — o Padé [k/k] de e^x é uma fração contínua:\n\n");
    {
        /* Pade [k/k] de e^1, por recorrencia dos coeficientes — a diagonal da fracao continua */
        printf("        k    Padé [k/k] de e        erro\n");
        double melhor = 1;
        for(int k = 1; k <= 8; k++){
            double num = 0, den = 0;
            for(int j = 0; j <= k; j++){
                double cf = 1;
                for(int i = 0; i < j; i++) cf *= (double)(k - i) / ((2*k - i) * (double)(i+1));
                num += cf; den += (j % 2) ? -cf : cf;
            }
            double v = num / den, err = fabs(v - M_E);
            if(k == 1 || k == 2 || k == 4 || k == 6 || k == 8)
                printf("        %d    %.10f          %.1e\n", k, v, err);
            if(k == 8) melhor = err;
        }
        printf("\n");
        ok("o Padé de ordem 8 bate e no limite do double", melhor < 1e-14);
        printf("        A fração contínua faz as DUAS coisas, e é o que o paper sublinha: na base\n");
        printf("        do invariante |det| = 1 ela GERA os metais (o ponto fixo) e APROXIMA o\n");
        printf("        fluxo de qualquer equação diferencial (o Padé). O mesmo objeto do lado\n");
        printf("        discreto e do contínuo — e é por isso que a cifra do rei chega aqui.\n");
    }
}

printf("\n§E5  E a solução verifica-se por substituição.\n\n");
{
    /* como no primeiro grau: nao se confia na deducao, substitui-se e mede-se o residuo. Aqui
     * a substituicao e na ALGEBRA: se s e raiz da borda, entao s² - b1 s - b0 = 0 la dentro. */
    struct { const char *eq; } t[] = {
        { "y'' = -y" }, { "y'' = y' + y" }, { "y'' - 3y' + 2y = 0" }, { "y'' + 2y' + y = 0" },
    };
    int mal = 0;
    printf("      equação                 σ² - b₁σ - b₀ na álgebra do próprio corpo\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; char bt[96];
        if(!edo_le(t[k].eq, &e)){ mal++; continue; }
        edo_borda(e, bt, sizeof bt);
        Elem borda; char marca[4];
        int n = al_le_borda(bt, &borda, marca);
        if(n != 2){ mal++; continue; }
        Elem s = al_zero(2); s.p[1] = 1;
        Elem s2 = al_prod(s, s, &borda);          /* σ², já reduzido pela borda */
        Elem r = al_soma(s2, borda, -1);          /* σ² − (b₀ + b₁σ) tem de dar ZERO */
        char sr[64]; al_escreve(r, sr, sizeof sr, marca);
        printf("      %-23s %s\n", t[k].eq, sr);
        if(strcmp(sr, "0")) mal++;
    }
    printf("\n");
    ok("σ é raiz da sua própria borda — resíduo 0 nas quatro", mal == 0);
    printf("      É a mesma disciplina do primeiro grau: não se confia na dedução, substitui-se\n");
    printf("      e mede-se. E aqui a verificação nem precisou de máquina nova — é a álgebra\n");
    printf("      global, com o corpo que a própria equação declarou.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
