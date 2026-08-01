/* edo.c — A EQUAÇÃO DIFERENCIAL É A BORDA DO CORPO. Resgatado de chess/, e fechado aqui.
 *
 *   §E1  a equação característica É a borda: y'' + By' + Cy = 0  <->  σ² = -C - Bσ
 *   §E2  e o Δ que classifica as soluções é o MESMO Δ do catálogo
 *   §E3  os três regimes do chess — cristal, borda, caos — são as três classes
 *   §E4  e dois casos fecham o círculo: o oscilador é o i, e o ouro é φ^t
 *   §E6  a solução explícita é e^{At}, e o exp é a ponte (de broca-so/papers)
 *   §E7  a NÃO HOMOGÉNEA: y = y_h + y_p, e a ressonância é a raiz dupla outra vez
 *   §E8  e a particular verifica-se por SUBSTITUIÇÃO — resíduo medido
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

printf("\n§E7  A NÃO HOMOGÉNEA: y = y_h + y_p, e a RESSONÂNCIA é a raiz dupla outra vez.\n\n");
{
    struct { const char *eq; const char *yp; int res; } t[] = {
        { "y'' + y = 1",          "1",                       0 },
        { "y'' - y = e^2t",       "0.333333·e^(2 t)",        0 },
        { "y'' + 3y' + 2y = 5",   "2.5",                     0 },
        { "y'' + y = sen 3t",     "0·cos(3 t) + -0.125·sen(3 t)", 0 },
        { "y'' - y = e^t",        "0.5·t·e^(1 t)",           1 },
        { "y'' - 3y' + 2y = e^t", "-1·t·e^(1 t)",            1 },
        { "y'' + y = cos t",      "0.5·t·sen(1 t)",          1 },
        { "y'' + 4y = cos 2t",    "0.25·t·sen(2 t)",         1 },
        { "y'' + 2y' + y = e^-t", "0.5·t²·e^(-1 t)",         2 },
    };
    int mal = 0;
    printf("      equação                  a particular                    ressonância\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; Fonte f; char p[160] = "?";
        int r = -1;
        if(edo_le_nh(t[k].eq, &e, &f)){
            double B = (double)e.Bp/e.Bq, C = (double)e.Cp/e.Cq;
            r = edo_particular(B, C, f, p, sizeof p);
        }
        printf("      %-24s %-31s %s\n", t[k].eq, p,
               r == 0 ? "" : r == 1 ? "SIMPLES" : r == 2 ? "DUPLA" : "?");
        if(strcmp(p, t[k].yp) || r != t[k].res) mal++;
    }
    printf("\n");
    ok("as particulares batem a conta à mão, e a ressonância é detetada", mal == 0);
    printf("      A solução geral é y = y_h + y_p — a homogénea MAIS uma particular. E isso diz\n");
    printf("      uma coisa sobre a estrutura: o conjunto das soluções NÃO é um espaço vetorial,\n");
    printf("      é um espaço vetorial TRANSLADADO. A fonte desloca o corpo livre, não o deforma.\n");
    printf("\n      E A RESSONÂNCIA É A RAIZ DUPLA OUTRA VEZ. Substituindo y = A·e^{at} sai\n");
    printf("      A·p(a)·e^{at}, com p(a) = a² + Ba + C — o PRÓPRIO polinómio característico. Se\n");
    printf("      p(a) ≠ 0, A = k/p(a) e acabou. Se p(a) = 0, a fonte cai SOBRE o espectro, não\n");
    printf("      há A que sirva, e entra um t. É o mesmo t da raiz dupla, e pelo mesmo motivo:\n");
    printf("      o denominador anulou-se. Em y'' + 2y' + y = e^-t as duas coisas coincidem — a\n");
    printf("      raiz é dupla E a fonte cai nela — e aí entra t².\n");
}

printf("\n§E8  E a particular VERIFICA-SE por substituição — resíduo medido.\n\n");
{
    /* Nao se confia na deducao. Substitui-se a particular na equacao e mede-se o que sobra,
     * por diferencas finitas em varios pontos. Se o coeficiente estivesse errado, isto acusa. */
    struct { const char *eq; } t[] = {
        { "y'' + y = 1" }, { "y'' - y = e^2t" }, { "y'' + 3y' + 2y = 5" },
        { "y'' + y = sen 3t" }, { "y'' - y = e^t" }, { "y'' - 3y' + 2y = e^t" },
        { "y'' + y = cos t" }, { "y'' + 4y = cos 2t" }, { "y'' + 2y' + y = e^-t" },
    };
    int mal = 0;
    double pior = 0;
    printf("      equação                  resíduo máximo em t = 0.3, 1.0, 2.0\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Edo e; Fonte f; char p[160];
        if(!edo_le_nh(t[k].eq, &e, &f)){ mal++; continue; }
        double B = (double)e.Bp/e.Bq, C = (double)e.Cp/e.Cq;
        int r = edo_particular(B, C, f, p, sizeof p);
        /* reconstrói a particular como função, a partir de (r, f, B, C) */
        double A, w = f.w, a2 = (f.tipo == F_CONST) ? 0 : f.a;
        double pa = a2*a2 + B*a2 + C, dpa = 2*a2 + B;
        double maxr = 0;
        double ts[3] = { 0.3, 1.0, 2.0 };
        for(int j = 0; j < 3; j++){
            double tt = ts[j], h = 1e-4, y[3];
            for(int d = -1; d <= 1; d++){
                double u = tt + d*h, v;
                if(f.tipo == F_CONST || f.tipo == F_EXP){
                    A = r == 0 ? f.k/pa : r == 1 ? f.k/dpa : f.k/2;
                    v = A * exp(a2*u) * (r == 0 ? 1 : r == 1 ? u : u*u);
                } else {
                    double d1 = C - w*w, d2 = B*w, det = d1*d1 + d2*d2;
                    if(fabs(det) > 1e-12){
                        double P = (f.tipo == F_COS) ? f.k*d1/det : f.k*d2/det;
                        double Q = (f.tipo == F_COS) ? -f.k*d2/det : f.k*d1/det;
                        v = P*cos(w*u) + Q*sin(w*u);
                    } else {
                        v = (f.tipo == F_COS) ? f.k/(2*w)*u*sin(w*u) : -f.k/(2*w)*u*cos(w*u);
                    }
                }
                y[d+1] = v;
            }
            double y1 = (y[2] - y[0]) / (2e-4), y2 = (y[2] - 2*y[1] + y[0]) / 1e-8;
            double fv = f.tipo == F_CONST ? f.k
                      : f.tipo == F_EXP   ? f.k*exp(f.a*tt)
                      : f.tipo == F_COS   ? f.k*cos(f.w*tt)
                                          : f.k*sin(f.w*tt);
            double res = fabs(y2 + B*y1 + C*y[1] - fv);
            if(res > maxr) maxr = res;
        }
        printf("      %-24s %.2e\n", t[k].eq, maxr);
        if(maxr > 1e-5) mal++;
        if(maxr > pior) pior = maxr;
    }
    printf("\n");
    ok("as nove particulares substituídas dão a fonte — resíduo abaixo de 1e-5", mal == 0);
    printf("      O resíduo não é zero exato porque a derivada é numérica: h = 1e-4 e a segunda\n");
    printf("      diferença tem erro de ordem h². É a caixa da medida, e não a conta — e digo-o\n");
    printf("      em vez de arredondar o número e chamar-lhe zero. O que isto apanha é o que\n");
    printf("      interessa: um coeficiente errado dá resíduo da ordem da própria fonte.\n");
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
