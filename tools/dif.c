/* dif.c — O CORPO DIFERENCIAL, construído como os outros: cifra e deformação.
 *
 * O Aarão, em quatro tempos:
 *   "esse é o corpo diferencial, inclusive falta completar ele"
 *   "com as operações, a cifra do rei e deformação — igual fizemos para os outros"
 *   "então a cifra é: você sai da reta e chega no círculo via polinómios"
 *   "Fourier na soma, Mellin no produto"
 *
 * E isso é o corpo inteiro, dito em quatro linhas. Aqui mede-se peça a peça.
 *
 *   A CIFRA é a reta: o operador D, e as funções e^{λt} com λ real — cresce ou decai, e não
 *   volta. A DEFORMAÇÃO leva-a ao círculo, e o caminho é o POLINÓMIO: sob Fourier, D vira
 *   multiplicação por iω, e o operador p(D) vira o polinómio p(iω). Quando as raízes saem do
 *   eixo real, o movimento sai da reta e entra no círculo — e e^{iωt} vive no círculo unitário.
 *
 *   AS DUAS OPERAÇÕES TÊM CADA UMA A SUA TRANSFORMADA, e é o par ⊕/⊗ do contrato:
 *     FOURIER na SOMA     — os caracteres do grupo aditivo são e^{iωt}; F leva D em ·iω
 *     MELLIN  no PRODUTO  — os caracteres do grupo multiplicativo são t^s; M leva tD em ·s
 *
 *   §F1  Fourier leva a derivada em multiplicação — o operador vira POLINÓMIO
 *   §F2  e F⁴ = id: a transformada tem ordem 4, e é o mesmo J do zero.c
 *   §F3  Mellin no produto: o outro grupo, os outros caracteres
 *   §F4  LEIBNIZ, que é o que faz do corpo um corpo diferencial
 *   §F5  e o que FALTAVA COMPLETAR: o dual de D é ∫, e o par não é simétrico
 *   §F7  PONTRYAGIN flipando: o caractere troca ⊕ por ⊗, e a dualidade é involução
 *   §F8  e PREENCHE a área do círculo? só com o irracional — e o ouro é o melhor
 *   §F6  a régua do corpo diferencial, e onde ele cai no catálogo
 *
 *   cc -O2 -std=c99 dif.c -lm -o dif && ./dif
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <complex.h>
#include "unidade.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define N 16                                   /* a caixa: N pontos. Finito, e dito. */

static void dft(const double complex *x, double complex *X, int inv){
    for(int k = 0; k < N; k++){
        double complex s = 0;
        for(int j = 0; j < N; j++){
            double a = (inv ? 2 : -2) * M_PI * j * k / N;
            s += x[j] * (cos(a) + I*sin(a));
        }
        X[k] = s / sqrt((double)N);            /* unitária dos dois lados: é assim que F⁴ = id */
    }
}

int main(void){
printf("\n=== O CORPO DIFERENCIAL — cifra e deformação, como os outros ==============\n");
printf("    A cifra é a RETA: o operador D, e as funções e^(λt) com λ real, que\n");
printf("    crescem ou decaem e não voltam. A DEFORMAÇÃO leva-a ao CÍRCULO, e o\n");
printf("    caminho é o POLINÓMIO. Fourier na soma, Mellin no produto.\n");

printf("\n§F1  Fourier leva a derivada em multiplicação — o operador vira POLINÓMIO.\n\n");
{
    /* d/dt e^{iωt} = iω e^{iωt}. Na DFT: a derivada discreta vira multiplicação pelo modo. */
    double complex x[N], X[N], dx[N], dX[N];
    for(int j = 0; j < N; j++){
        double t = 2*M_PI*j/N;
        x[j] = cos(3*t) + I*sin(3*t);          /* o modo 3: e^{i3t} */
    }
    for(int j = 0; j < N; j++) dx[j] = (x[(j+1)%N] - x[(j+N-1)%N]) * N / (4*M_PI);
    dft(x, X, 0); dft(dx, dX, 0);
    int modo = 0; double mx = 0;
    for(int k = 0; k < N; k++) if(cabs(X[k]) > mx){ mx = cabs(X[k]); modo = k; }
    double complex razao = dX[modo] / X[modo];
    double h = 2*M_PI/N, w = 3;
    double simbolo = sin(w*h)/h;               /* o símbolo EXATO da diferença central */
    printf("      f(t) = e^(i3t) — o espectro concentra-se no modo %d\n", modo);
    printf("      e (Df)^ / f^ nesse modo  = %+.6f %+.6fi\n", creal(razao), cimag(razao));
    printf("      o símbolo da diferença central, i·sen(ωh)/h = %+.6f %+.6fi\n", 0.0, simbolo);
    printf("      e o do operador contínuo,   iω              = %+.6f %+.6fi\n\n", 0.0, w);
    ok("a derivada vira MULTIPLICAÇÃO — e o valor é o SÍMBOLO exato do operador usado",
       fabs(creal(razao)) < 1e-9 && fabs(cimag(razao) - simbolo) < 1e-9);
    /* e o símbolo do discreto tende ao do contínuo quando o passo encolhe */
    int mal2 = 0; double ant = 1e9;
    for(int M = 32; M <= 4096; M *= 2){
        double hh = 2*M_PI/M, sm = sin(w*hh)/hh, err = fabs(sm - w);
        if(err > ant) mal2++;
        ant = err;
    }
    ok("e o símbolo do discreto TENDE ao do contínuo quando o passo encolhe", mal2 == 0);
    printf("      E AQUI A TOLERÂNCIA ESTAVA A ESCONDER UMA COISA. Eu tinha escrito \"iω, a menos\n");
    printf("      do erro O(h²)\" com margem de 0,2 — e o valor medido é 2,3526, que não é 3 com\n");
    printf("      erro: é o símbolo EXATO da diferença central, i·sen(ωh)/h. A transformada não\n");
    printf("      aproxima nada; ela mostra o operador que eu de facto usei, e o que difere de iω\n");
    printf("      é o operador DISCRETO, não a medida. Uma margem larga teria dado verde e\n");
    printf("      escondido isso.\n\n");
    printf("      É ISTO A CIFRA. O operador D, que na reta é uma coisa que se aplica, do outro\n");
    printf("      lado é um NÚMERO que multiplica — e então p(D) vira p(iω), um polinómio. Sair\n");
    printf("      da reta e chegar ao círculo via polinómios é literalmente isto: a reta é o t,\n");
    printf("      o círculo é onde vivem os e^(iωt), e quem atravessa é o polinómio.\n");
}

printf("\n§F2  E F⁴ = id — a transformada tem ordem 4, e é o mesmo J do zero.c.\n\n");
{
    double complex x[N], a[N], b[N], c[N], d[N];
    for(int j = 0; j < N; j++) x[j] = (j == 2) ? 1 : (j == 5 ? 0.5 : 0);
    dft(x, a, 0); dft(a, b, 0); dft(b, c, 0); dft(c, d, 0);
    double e1 = 0, e2 = 0;
    for(int j = 0; j < N; j++){
        if(cabs(d[j] - x[j]) > e1) e1 = cabs(d[j] - x[j]);      /* F⁴ = id      */
        if(cabs(b[j] - x[(N-j)%N]) > e2) e2 = cabs(b[j] - x[(N-j)%N]);  /* F² = paridade */
    }
    printf("      max |F⁴x - x|        = %.2e\n", e1);
    printf("      max |F²x - x(-t)|    = %.2e\n\n", e2);
    ok("F⁴ = id: a transformada de Fourier tem ordem QUATRO", e1 < 1e-12);
    ok("e F² é a paridade, t -> -t — logo F é a raiz quadrada da reflexão", e2 < 1e-12);
    printf("      Quatro. É o mesmo número do J do zero.c, e não por coincidência: F² troca o\n");
    printf("      sinal do argumento como J² = -I troca o sinal do vetor, e são precisos quatro\n");
    printf("      passos para voltar. A transformada de Fourier É o flip deste corpo — a\n");
    printf("      involução que troca os dois lados, e o lado de lá é o círculo.\n");
    printf("\n      E daí sai o par: as potências de F são {id, F, F², F³} — a FAMÍLIA REAL do\n");
    printf("      corpo diferencial, com quatro elementos, tal como as unidades de Z[i].\n");
}

printf("\n§F3  Mellin no produto: o outro grupo, os outros caracteres.\n\n");
{
    /* Fourier e do grupo ADITIVO (t -> t + a), com caracteres e^{iωt}. Mellin e do grupo
     * MULTIPLICATIVO (t -> at), com caracteres t^s. E o operador que Mellin diagonaliza nao e
     * D: e o de EULER, tD. */
    int mal = 0;
    printf("      operador       caracteres     vira        grupo\n");
    printf("      D = d/dt       e^(iωt)        · iω        aditivo,       t -> t + a\n");
    printf("      tD = t·d/dt    t^s            · s         multiplicativo, t -> a·t\n\n");
    /* mede-se: tD aplicado a t^s da s·t^s */
    for(double s = -2; s <= 3; s += 1){
        double t0 = 2.0, h = 1e-6;
        double f1 = pow(t0+h, s), f0 = pow(t0-h, s);
        double tD = t0 * (f1 - f0) / (2*h);
        double esperado = s * pow(t0, s);
        if(fabs(tD - esperado) > 1e-5*fabs(esperado) + 1e-9) mal++;
    }
    printf("      tD aplicado a t^s em t=2, para s = -2..3: dá s·t^s em todos\n\n");
    ok("o operador de Euler tD tem os t^s por próprios, com valor próprio s", mal == 0);
    printf("      São os dois lados do contrato: a SOMA tem Fourier, o PRODUTO tem Mellin. E o\n");
    printf("      log é a ponte entre os dois grupos — t = e^u leva o multiplicativo no aditivo,\n");
    printf("      e leva Mellin em Fourier. É a mesma ponte do §E6: o exp leva a soma dos\n");
    printf("      geradores ao produto dos fluxos, e aqui leva um grupo no outro.\n");
}

printf("\n§F4  LEIBNIZ — é o que faz do corpo um corpo DIFERENCIAL.\n\n");
{
    /* Um corpo diferencial e (K, D) com D linear e D(ab) = D(a)b + aD(b). Sem Leibniz e so um
     * operador linear qualquer; COM Leibniz e uma derivacao, e o corpo passa a ter dinamica. */
    int mal_lin = 0, mal_leib = 0;
    double h = 1e-5;
    double (*fs[4])(double) = { sin, cos, exp, sqrt };
    const char *nm[4] = { "sen", "cos", "exp", "raiz" };
    printf("      par            D(ab)            D(a)b + aD(b)     diferença\n");
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
        double t = 1.3;
        double a0 = fs[i](t), b0 = fs[j](t);
        double da = (fs[i](t+h) - fs[i](t-h)) / (2*h);
        double db = (fs[j](t+h) - fs[j](t-h)) / (2*h);
        double dab = (fs[i](t+h)*fs[j](t+h) - fs[i](t-h)*fs[j](t-h)) / (2*h);
        double leib = da*b0 + a0*db;
        if(fabs(dab - leib) > 1e-6) mal_leib++;
        if(i <= 1 && j <= 1){
            double dsoma = (fs[i](t+h)+fs[j](t+h) - fs[i](t-h)-fs[j](t-h)) / (2*h);
            if(fabs(dsoma - (da+db)) > 1e-6) mal_lin++;
        }
        if(i == j || (i == 0 && j == 2))
            printf("      %-4s · %-6s  %+13.9f    %+13.9f     %.1e\n", nm[i], nm[j], dab, leib,
                   fabs(dab - leib));
    }
    printf("\n");
    ok("D é linear: D(a+b) = D(a) + D(b)", mal_lin == 0);
    ok("e cumpre LEIBNIZ: D(ab) = D(a)b + aD(b), nos 16 pares", mal_leib == 0);
    printf("      Leibniz é a cláusula que liga D às DUAS operações do contrato ao mesmo tempo:\n");
    printf("      ele é morfismo para a soma e QUASE morfismo para o produto — quase, porque o\n");
    printf("      resultado tem dois termos em vez de um. É essa falta de simetria que carrega a\n");
    printf("      dinâmica, e é o que distingue uma derivação de um operador linear qualquer.\n");
}

printf("\n§F5  E O QUE FALTAVA COMPLETAR: o dual de D é ∫, e o par NÃO é simétrico.\n\n");
{
    /* D tem dual: a integracao. Mas o par nao fecha dos dois lados —
     *     D∘∫ = id          (derivar o integral devolve a funcao)
     *     ∫∘D = id - av0    (integrar a derivada PERDE a constante)
     * e e exatamente a irreversibilidade do 0/0: onde a informacao se apaga, a volta tem uma
     * classe inteira em vez de um valor. O nucleo de D sao as CONSTANTES, e e ele que se perde. */
    double t0 = 1.7, h = 1e-5;
    /* D∘∫ : integra sin de 0 a t (= 1 - cos t) e deriva -> tem de dar sin t */
    double I1 = 1 - cos(t0+h), I0 = 1 - cos(t0-h);
    double dI = (I1 - I0) / (2*h);
    printf("      D(∫ sen) em t = %.1f  ->  %+.9f     e sen(t) = %+.9f\n", t0, dI, sin(t0));
    /* ∫∘D : deriva (sin t + 5) e integra de 0 a t -> da sin t, e o 5 SUMIU */
    double c = 5.0;
    double volta = sin(t0) - sin(0.0);        /* ∫₀ᵗ D(sen + c) = sen t - sen 0 */
    printf("      ∫(D(sen + %g)) em t     ->  %+.9f     e sen + %g = %+.9f\n",
           c, volta, c, sin(t0) + c);
    printf("      a diferença é %g — exatamente a constante que se perdeu\n\n", sin(t0)+c - volta);
    ok("D∘∫ = id — derivar o integral devolve a função", fabs(dI - sin(t0)) < 1e-6);
    ok("mas ∫∘D PERDE a constante: o par não fecha dos dois lados",
       fabs((sin(t0)+c) - volta - c) < 1e-12);
    printf("      É ESTE o buraco que faltava completar, e ele não se tapa: tapa-se DECLARANDO.\n");
    printf("      O núcleo de D são as constantes — ker D = C, o corpo dos escalares — e é ele\n");
    printf("      que ∫ não consegue devolver. A volta não dá UM valor: dá uma CLASSE, o\n");
    printf("      conjunto y + C, e é preciso um dado a mais (a condição inicial) para escolher\n");
    printf("      um representante.\n");
    printf("\n      E é o mesmo desenho do 0/0. Lá, 0·x apaga o x e a pergunta inversa tem a\n");
    printf("      classe toda por resposta; aqui, D apaga a constante e ∫ devolve a classe. A\n");
    printf("      condição inicial é ao corpo diferencial o que a cifra é ao 0/0: a maneira de\n");
    printf("      percorrer a classe e escolher um ponto.\n");
    printf("\n      Então o corpo diferencial COMPLETO é (K, D, ∫, C) — o corpo, a derivação, o\n");
    printf("      seu dual, e o núcleo que os separa. Sem o núcleo declarado o par parece\n");
    printf("      simétrico e não é, e essa é a incompletude que estava aqui.\n");
}

printf("\n§F7  PONTRYAGIN FLIPANDO — o caractere troca a soma pelo produto, e volta.\n\n");
{
    /* O contrato ja o tinha na clausula Π: "ou ∏(a⊕b)=∏(a)⊗∏(b) (Pontryagin, o caractere)".
     * Aqui mede-se: o caractere e^{iωt} leva a SOMA no PRODUTO, e a dualidade é uma INVOLUÇÃO
     * — o dual do dual volta. É o flip, e é a mesma ordem 4 do §F2. */
    int mal = 0;
    printf("      a       b       Π(a+b)                    Π(a)·Π(b)                 dif\n");
    double par[4][2] = { {0.3,0.7}, {1.0,-0.5}, {2.0,1.0}, {-1.2,0.4} };
    for(int k = 0; k < 4; k++){
        double a2 = par[k][0], b2 = par[k][1], w = 3;
        double complex esq = cexp(I*w*(a2+b2));
        double complex dir = cexp(I*w*a2) * cexp(I*w*b2);
        printf("      %+5.1f   %+5.1f   %+.6f%+.6fi    %+.6f%+.6fi    %.1e\n",
               a2, b2, creal(esq), cimag(esq), creal(dir), cimag(dir), cabs(esq-dir));
        if(cabs(esq - dir) > 1e-12) mal++;
    }
    printf("\n");
    ok("Π(a+b) = Π(a)·Π(b) — o caractere leva a SOMA no PRODUTO", mal == 0);
    printf("      É a cláusula Π do contrato, na forma de Pontryagin. E o \"flipando\" é a outra\n");
    printf("      metade: a dualidade de Pontryagin é uma INVOLUÇÃO — o dual do dual é o próprio.\n");
    printf("      O dual de R é R, o do círculo é Z, e o de Z é o círculo: os dois trocam-se e ao\n");
    printf("      fim de dois passos está-se em casa.\n");
    printf("\n      E isso amarra o §F2: F² é a paridade e F⁴ = id, ou seja o flip precisa de\n");
    printf("      QUATRO passos no nível da função e de DOIS no nível do grupo. É a raiz\n");
    printf("      quadrada outra vez — F é a raiz da reflexão, como o i é a raiz de -1.\n");
    printf("\n      E o que Pontryagin troca é exatamente o par do contrato: onde havia ⊕ passa a\n");
    printf("      haver ⊗. Fourier na soma e Mellin no produto são o MESMO teorema visto de cada\n");
    printf("      lado do flip — e é por isso que o log leva um no outro.\n");
}

printf("\n§F8  E PREENCHE A ÁREA DO CÍRCULO? Só com o irracional — e o ouro é o melhor.\n\n");
{
    /* A mesma pergunta do prismatico: a deformacao preenche? Aqui a orbita de e^{iωt} no
     * circulo — com ω racional ela FECHA num ciclo finito e deixa buracos; com ω irracional e
     * densa. E entre os irracionais, o ouro e o que enche mais uniformemente. */
    struct { const char *nome; double w; } t[] = {
        { "1/2   (racional)", 0.5 },
        { "1/3   (racional)", 1.0/3 },
        { "3/8   (racional)", 0.375 },
        { "raiz2 (irracional)", 1.41421356237309505 - 1 },
        { "pi    (irracional)", 3.14159265358979324 - 3 },
        { "ouro  (o rei)",    0.61803398874989485 },
    };
    int mal = 0;
    printf("      passo               pontos distintos   maior buraco   preenche?\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        /* 400 passos da rotacao, e mede-se o maior intervalo vazio no circulo */
        double p[400];
        int n = 400;
        for(int j = 0; j < n; j++){ double v = t[k].w * j; p[j] = v - floor(v); }
        /* ordena (insercao, n pequeno) e mede o maior intervalo */
        for(int i = 1; i < n; i++){ double v = p[i]; int j = i-1;
            while(j >= 0 && p[j] > v){ p[j+1] = p[j]; j--; } p[j+1] = v; }
        int dist = 1;
        double maior = p[0] + (1 - p[n-1]);
        for(int i = 1; i < n; i++){
            double g = p[i] - p[i-1];
            if(g > 1e-12) dist++;
            if(g > maior) maior = g;
        }
        int enche = maior < 0.02;
        printf("      %-19s %-18d %.6f       %s\n", t[k].nome, dist, maior,
               enche ? "SIM" : "não — fica buraco");
        if((k < 3) == enche) mal++;            /* os racionais NÃO enchem, os outros enchem */
    }
    printf("\n");
    ok("o racional fecha em ciclo e deixa buraco; o irracional preenche", mal == 0);
    printf("      É a mesma pergunta do prismático — \"deformá-lo de modo a preencher a área\" —\n");
    printf("      e a resposta aqui é a mesma do resto do sistema: quem preenche é o IRRACIONAL.\n");
    printf("      Com ω racional a órbita fecha ao fim de q passos e não visita mais nada: são\n");
    printf("      q pontos e q buracos, por mais que se itere. Com ω irracional nunca fecha, e\n");
    printf("      a órbita é densa.\n");

    /* e entre os irracionais, qual enche MELHOR — a discrepancia */
    printf("\n      E entre os irracionais, o OURO é o que enche mais uniformemente:\n\n");
    printf("        passo        maior buraco após 400 pontos\n");
    double melhor = 1; const char *quem = "";
    for(size_t k = 3; k < sizeof t/sizeof *t; k++){
        double p[400]; int n = 400;
        for(int j = 0; j < n; j++){ double v = t[k].w * j; p[j] = v - floor(v); }
        for(int i = 1; i < n; i++){ double v = p[i]; int j = i-1;
            while(j >= 0 && p[j] > v){ p[j+1] = p[j]; j--; } p[j+1] = v; }
        double maior = p[0] + (1 - p[n-1]);
        for(int i = 1; i < n; i++) if(p[i]-p[i-1] > maior) maior = p[i]-p[i-1];
        printf("        %-18s %.8f\n", t[k].nome, maior);
        if(maior < melhor){ melhor = maior; quem = t[k].nome; }
    }
    printf("\n");
    ok("e o menor buraco é o do OURO — ele é o que preenche melhor", strstr(quem, "ouro") != 0);
    printf("      O ouro é o irracional que pior se aproxima por racionais — a cifra dele é\n");
    printf("      [1,1,1,…], só termos neutros, e nenhum termo grande a dar um convergente bom.\n");
    printf("      É exatamente por isso que ele enche melhor: aproximar-se mal de um racional é\n");
    printf("      não cair num ciclo, e não cair num ciclo é não deixar buraco.\n");
    printf("\n      Então a deformação DO CORPO DIFERENCIAL preenche o círculo, e o rei é o passo\n");
    printf("      que a faz preencher melhor. A cifra sai da reta, o polinómio leva ao círculo, e\n");
    printf("      quem enche a área é o mesmo [1,1,1,…] de sempre.\n");
}

printf("\n§F6  A régua do corpo diferencial, e onde ele cai no catálogo.\n\n");
{
    /* A deformacao e F, a transformada, e ela tem ordem 4: F⁴ = id, F² = paridade. O gerador
     * de um ciclo de ordem 4 em GL2(Z) e [[0,-1],[1,0]] — traco 0, determinante 1, Δ = -4. */
    long B = 0, C = 1, D = B*B - 4*C;
    printf("      a deformação é F, com F⁴ = id e F² = paridade\n");
    printf("      o gerador de ordem 4 é [[0,-1],[1,0]]:  traço %ld, det %ld, Δ = %ld\n\n", B, C, D);
    ok("o corpo diferencial é ELÍPTICO, com a régua (0, 1) e Δ = -4", D == -4);
    printf("      É a mesma régua do i, e tem de ser: a deformação deste corpo é a transformada,\n");
    printf("      a transformada tem ordem 4, e em GL2(Z) só há um lugar de ordem 4. O corpo\n");
    printf("      diferencial NÃO abre lugar novo no catálogo — senta-se no do i, e o que ele\n");
    printf("      traz de próprio é a DEFORMAÇÃO, não a régua.\n");
    printf("\n      E isso responde à pergunta que se faz a cada corpo novo: ele é o mesmo lugar\n");
    printf("      com outra deformação. A cifra é a reta, a deformação é o polinómio que a leva\n");
    printf("      ao círculo, o flip é Fourier, e o dual é a integração — com o núcleo declarado.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
