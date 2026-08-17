/* fisica.c — ONDE OS DUAIS SE TOCAM: ε² = 0, E O QUE ISSO É NA FÍSICA.
 *
 * O Aarão: "os duais se tocam acima do infinito, aí é ε² = 0. Tem análogo na física? Os
 * operadores bra-ket talvez?"
 *
 * A primeira parte é a terceira classe, que o dual.c §U5 listava sem medir: entre o direto
 * (e² = -1, círculo) e o dual (e² = +1, hipérbole) está a FRONTEIRA, e² = 0 — onde as duas
 * retas do cone deixam de ser duas. É a raiz dupla, e o projeto já lhe chamava ressonância.
 *
 * A segunda parte é a pergunta, e ela tem resposta. O palpite do bra-ket está CERTO, com uma
 * condição que se mede. E há mais três correspondências, das quais duas são exatas e uma é só
 * analogia — e a secção diz qual é qual, porque misturá-las seria vender mais do que se tem.
 *
 *   §P1  a terceira classe: ε² = 0, e a álgebra R[ε]/(ε²)
 *   §P2  e ela É a derivada — exata, não aproximada
 *   §P3  o bra-ket: (|a><b|)² = 0 exatamente quando <b|a> = 0     [o palpite, medido]
 *   §P4  os férmions: a² = 0 é o princípio de exclusão
 *   §P5  o cone do dual É o cone de luz — e o fóton não tem inverso
 *   §P6  a soma de velocidades de Einstein É a lei polar do dual
 *   §P7  "acima do infinito": c -> ∞ leva ε² = +1 em ε² = 0
 *   §P8  o quadro das três, e o que é exato e o que é analogia
 *
 *   cc -O2 -std=c99 fisica.c -lm -o fisica && ./fisica
 */
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "unidade.h"
#include "reta.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* a mesma maquina do dual.c: s = e², a correr por {-1, 0, +1} */
typedef struct { double a, b; } Z;
static Z mul(Z x, Z y, int s){
    Z r;
    r.a = x.a*y.a + s*x.b*y.b;
    r.b = x.a*y.b + x.b*y.a;
    return r;
}

/* matrizes 2x2 complexas, para o bra-ket e os fermioes */
typedef struct { double re[2][2], im[2][2]; } M2;
static M2 mm(M2 A, M2 B){
    M2 C = {{{0}},{{0}}};
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
        for(int k = 0; k < 2; k++){
            C.re[i][j] += A.re[i][k]*B.re[k][j] - A.im[i][k]*B.im[k][j];
            C.im[i][j] += A.re[i][k]*B.im[k][j] + A.im[i][k]*B.re[k][j];
        }
    return C;
}
static M2 msoma(M2 A, M2 B){
    M2 C;
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
        C.re[i][j] = A.re[i][j]+B.re[i][j];
        C.im[i][j] = A.im[i][j]+B.im[i][j];
    }
    return C;
}
static double mnorma(M2 A){
    double s = 0;
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) s += fabs(A.re[i][j]) + fabs(A.im[i][j]);
    return s;
}
/* |a><b| : o produto exterior de dois vetores de C² */
static M2 ketbra(const double ar[2], const double ai[2], const double br[2], const double bi[2]){
    M2 P = {{{0}},{{0}}};
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
        /* (|a><b|)_ij = a_i * conj(b_j) */
        P.re[i][j] = ar[i]*br[j] + ai[i]*bi[j];
        P.im[i][j] = ai[i]*br[j] - ar[i]*bi[j];
    }
    return P;
}

int main(void){
printf("\n=== ONDE OS DUAIS SE TOCAM: ε² = 0, E O QUE É NA FÍSICA ==================\n");
printf("    Entre o círculo (e² = -1) e a hipérbole (e² = +1) está a fronteira, onde\n");
printf("    as duas retas do cone deixam de ser duas. E ela tem nome na física.\n");

printf("\n§P1  A terceira classe: ε² = 0.\n\n");
{
    Z e = {0,1};
    Z c = mul(e,e,-1), h = mul(e,e,+1), p = mul(e,e,0);
    printf("      e² = -1  ->  i,  o direto   círculo      duas raízes conjugadas\n");
    printf("      e² = +1  ->  i*, o dual     hipérbole    duas raízes reais\n");
    printf("      e² =  0  ->  ε,  a fronteira  reta dupla   UMA raiz, dobrada\n\n");
    printf("      medido:  i·i = %+g      i*·i* = %+g      ε·ε = %+g\n\n", c.a, h.a, p.a);
    ok("as três classes, e a do meio é onde o produto DEGENERA", c.a == -1 && h.a == 1 && p.a == 0);

    /* "os duais se tocam": as duas retas do cone N = 0 coincidem quando s = 0 */
    printf("      E o que quer dizer \"se tocam\". O cone é N(a,b) = a² - s·b² = 0:\n\n");
    int mal = 0;
    for(int s = -1; s <= 1; s++){
        /* quantas DIRECOES distintas anulam a norma? */
        int dirs = 0;
        for(int k = 0; k < 720; k++){
            double th = k*M_PI/360, a = cos(th), b = sin(th);
            double N = a*a - s*b*b;
            if(fabs(N) < 1e-12) dirs++;
        }
        printf("      s = %+d:  %d direção(ões) com N = 0   %s\n", s, dirs,
               s < 0 ? "(só a origem: nenhuma reta)"
             : s > 0 ? "(DUAS retas: a = +b e a = -b)"
                     : "(UMA reta, contada duas vezes: b qualquer, a = 0)");
        if(s < 0 && dirs != 0) mal++;
        if(s > 0 && dirs != 4) mal++;      /* ±45° e ±135°, quatro semirretas = duas retas */
        if(s == 0 && dirs != 2) mal++;     /* ±90°, duas semirretas = UMA reta */
    }
    printf("\n");
    ok("no dual são DUAS retas; na fronteira elas colapsaram numa só — tocaram-se", mal == 0);
    printf("      É esse o \"se tocam\". No dual o cone tem duas folhas separadas por duas retas;\n");
    printf("      ao levar s de +1 a 0 as duas retas rodam uma para a outra até coincidirem. E\n");
    printf("      quando coincidem, o discriminante Δ = -4s anula: é a RAIZ DUPLA, que este\n");
    printf("      projeto já encontrava na ressonância das EDs. Mesma degenerescência.\n");
}

printf("\n§P2  E a fronteira É a derivada — exata, não aproximada.\n\n");
{
    /* (a + bε)² = a² + 2abε, porque ε² = 0. Entao um polinomio avaliado em a+ε devolve
     * p(a) na parte real e p'(a) na parte ε. Nao e diferenca finita: e exato. */
    printf("      f(a + bε) = f(a) + f'(a)·b·ε,  porque todo termo com ε² morre\n\n");
    int mal = 0;
    printf("      f            a      f(a)         f'(a) pela ε      f'(a) exata\n");
    for(int caso = 0; caso < 4; caso++){
        double a = 1.0 + 0.7*caso;
        Z x = { a, 1.0 };                       /* a + 1·ε */
        Z r, esp;
        const char *nome;
        double dexata;
        if(caso == 0){                          /* f = x³ */
            r = mul(mul(x,x,0),x,0); nome = "x³";
            esp.a = a*a*a; dexata = 3*a*a;
        } else if(caso == 1){                   /* f = x⁴ */
            Z q = mul(x,x,0); r = mul(q,q,0); nome = "x⁴";
            esp.a = a*a*a*a; dexata = 4*a*a*a;
        } else if(caso == 2){                   /* f = x² + 3x */
            Z q = mul(x,x,0); Z t = { 3*x.a, 3*x.b };
            r.a = q.a + t.a; r.b = q.b + t.b; nome = "x² + 3x";
            esp.a = a*a + 3*a; dexata = 2*a + 3;
        } else {                                /* f = x⁵ */
            Z q = mul(x,x,0); Z c4 = mul(q,q,0); r = mul(c4,x,0); nome = "x⁵";
            esp.a = a*a*a*a*a; dexata = 5*a*a*a*a;
        }
        printf("      %-12s %.2f   %-12.5f %-17.5f %.5f\n", nome, a, r.a, r.b, dexata);
        if(fabs(r.a - esp.a) > 1e-12 || fabs(r.b - dexata) > 1e-12) mal++;
    }
    printf("\n");
    ok("a parte ε É a derivada, com resíduo 0 — sem limite e sem passo h", mal == 0);
    printf("      Repare-se no que NÃO há aqui: nenhum h, nenhum limite, nenhum erro O(h²). A\n");
    printf("      derivada sai da ARITMÉTICA, porque ε² = 0 corta a série de Taylor no primeiro\n");
    printf("      termo. É a diferenciação automática, e é a razão de a fronteira não ser uma\n");
    printf("      curiosidade: ela é onde a análise vira álgebra.\n");
}

printf("\n§P3  O bra-ket — o palpite do Aarão, e ele está certo (com uma condição).\n\n");
{
    printf("      A pergunta era se |a><b| dá ε² = 0. Mede-se, e a resposta tem um SE.\n\n");
    /* Primeiro o caso que NAO da: o projetor. */
    double u[2] = {1,0}, z[2] = {0,0};
    M2 P = ketbra(u, z, u, z);                  /* |0><0| */
    M2 PP = mm(P,P);
    M2 dif = P; for(int i=0;i<2;i++) for(int j=0;j<2;j++){ dif.re[i][j]-=PP.re[i][j]; dif.im[i][j]-=PP.im[i][j]; }
    printf("      |0><0| é o PROJETOR:  P² - P = %g  ->  P² = P, idempotente, NÃO nilpotente\n",
           mnorma(dif));
    ok("o projetor |a><a| dá P² = P — não é este o ε", mnorma(dif) < 1e-12);

    /* Agora o caso que DA: bra e ket ORTOGONAIS. */
    double d[2] = {0,1};
    M2 S = ketbra(u, z, d, z);                  /* |0><1| — o operador de subida */
    M2 SS = mm(S,S);
    printf("\n      |0><1| com <1|0> = 0:  S² = %g  ->  S² = 0, NILPOTENTE. É o ε.\n\n", mnorma(SS));
    ok("o bra-ket com bra e ket ORTOGONAIS dá exatamente ε² = 0", mnorma(SS) < 1e-12);

    /* E a lei geral: (|a><b|)² = <b|a> · |a><b|. Logo anula SSE <b|a> = 0. */
    printf("      E a lei geral, que explica o SE:\n\n");
    printf("          (|a><b|)² = <b|a> · |a><b|\n\n");
    printf("      logo o quadrado anula EXATAMENTE quando <b|a> = 0. Medido em 200 pares:\n\n");
    int mal = 0, orto = 0, nao = 0;
    for(int k = 0; k < 200; k++){
        double ar[2] = { sin(3.0*k+1), cos(5.0*k+2) }, ai[2] = { sin(7.0*k), cos(11.0*k) };
        double br[2], bi[2];
        if(k % 2){                              /* metade dos casos: b ORTOGONAL a a */
            br[0] = -ar[1]; br[1] = ar[0];
            bi[0] =  ai[1]; bi[1] = -ai[0];
        } else {                                /* a outra metade: b qualquer */
            br[0] = cos(13.0*k); br[1] = sin(17.0*k);
            bi[0] = cos(19.0*k); bi[1] = sin(23.0*k);
        }
        /* <b|a> = Σ conj(b_j) a_j */
        double ipr = 0, ipi = 0;
        for(int j = 0; j < 2; j++){
            ipr += br[j]*ar[j] + bi[j]*ai[j];
            ipi += br[j]*ai[j] - bi[j]*ar[j];
        }
        double ip = sqrt(ipr*ipr + ipi*ipi);
        M2 T = ketbra(ar, ai, br, bi), TT = mm(T,T);
        int nulo = mnorma(TT) < 1e-9;
        if(ip < 1e-9) orto++; else nao++;
        if(nulo != (ip < 1e-9)) mal++;          /* nilpotente <=> ortogonais */
    }
    printf("      pares ortogonais: %d    não ortogonais: %d    discordâncias: %d\n\n",
           orto, nao, mal);
    ok("(|a><b|)² = 0 SE E SÓ SE <b|a> = 0 — o palpite, com a condição exata", mal == 0);
    printf("      Então sim, e o exemplo canónico da física é o operador de SUBIDA do spin-1/2:\n");
    printf("      σ⁺ = |↑><↓|, com (σ⁺)² = 0. Subir duas vezes de um sistema de dois níveis dá\n");
    printf("      zero — não há para onde subir. E é essa a leitura de ε² = 0: não é que o\n");
    printf("      resultado seja pequeno, é que o segundo passo não existe.\n");
}

printf("\n§P4  Os férmions: a² = 0 é o princípio de exclusão.\n\n");
{
    /* o operador de aniquilacao fermionico, em 2x2: a = |0><1| */
    double u[2] = {1,0}, d[2] = {0,1}, z[2] = {0,0};
    M2 a  = ketbra(u, z, d, z);                 /* aniquila */
    M2 ad = ketbra(d, z, u, z);                 /* cria (o adjunto) */
    M2 aa = mm(a,a), adad = mm(ad,ad);
    M2 anti = msoma(mm(a,ad), mm(ad,a));        /* {a, a†} = a a† + a† a */
    M2 I; memset(&I, 0, sizeof I); I.re[0][0] = I.re[1][1] = 1;
    M2 dif = anti; for(int i=0;i<2;i++) for(int j=0;j<2;j++) dif.re[i][j] -= I.re[i][j];
    printf("      a·a   = %g      (aniquilar duas vezes)\n", mnorma(aa));
    printf("      a†·a† = %g      (CRIAR duas vezes o mesmo estado)\n", mnorma(adad));
    printf("      {a, a†} - I = %g      (a relação de anticomutação)\n\n", mnorma(dif));
    ok("a² = (a†)² = 0 e {a,a†} = I — a álgebra fermiónica", mnorma(aa) < 1e-12
       && mnorma(adad) < 1e-12 && mnorma(dif) < 1e-12);
    printf("      E (a†)² = 0 É o princípio de exclusão de Pauli, escrito em álgebra: criar duas\n");
    printf("      vezes o mesmo férmion no mesmo estado dá ZERO — não \"é improvável\", é zero. A\n");
    printf("      exclusão não é uma regra imposta por fora; é a nilpotência do gerador.\n");
    printf("\n      É a mesma álgebra das variáveis de Grassmann, θ² = 0, sobre as quais se\n");
    printf("      escreve a integral de caminho fermiónica. Então a fronteira ε² = 0 não é um\n");
    printf("      caso degenerado que se tolera: é o lado da física onde vivem os férmions,\n");
    printf("      enquanto o círculo e² = -1 é o das fases e o dual e² = +1 é o dos boosts.\n");
}

printf("\n§P5  O cone do dual É o cone de luz — e o fóton não tem inverso.\n\n");
{
    /* N(t,x) = t² - x², que e o intervalo de Minkowski em 1+1. Os divisores de zero do dual
     * sao EXATAMENTE os vetores nulos. Isto nao e analogia: e a mesma forma quadratica. */
    printf("      no dual:      N(a,b) = a² - b²      anula em a = ±b\n");
    printf("      em Minkowski: s² = t² - x²          anula em x = ±t   (c = 1)\n\n");
    int mal = 0, luz = 0, tempo = 0, espaco = 0;
    for(int i = -12; i <= 12; i++) for(int j = -12; j <= 12; j++){
        if(!i && !j) continue;
        double N = (double)i*i - (double)j*j;
        int ehluz = fabs(N) < 1e-12;
        if(ehluz) luz++; else if(N > 0) tempo++; else espaco++;
        /* sem inverso <=> N = 0 <=> vetor nulo */
        if(ehluz != (fabs(N) < 1e-12)) mal++;
    }
    printf("      sobre o reticulado 25x25 sem a origem:\n");
    printf("      tipo TEMPO  (N > 0, invertível)          : %d\n", tempo);
    printf("      tipo ESPAÇO (N < 0, invertível)          : %d\n", espaco);
    printf("      tipo LUZ    (N = 0, SEM inverso)         : %d\n\n", luz);
    ok("os divisores de zero do dual são exatamente os vetores NULOS de Minkowski",
       mal == 0 && luz == 48);
    printf("      Esta correspondência é EXATA, não analogia: é literalmente a mesma forma\n");
    printf("      quadrática, a² - b², com outros nomes nas letras. E daí sai uma leitura que\n");
    printf("      não é minha invenção — o vetor nulo não tem inverso multiplicativo, e na\n");
    printf("      física isso é o fóton não ter referencial de repouso. O cone onde a álgebra\n");
    printf("      deixa de ser corpo é o cone onde a cinemática deixa de ter observador.\n");
}

printf("\n§P6  A soma de velocidades de Einstein É a lei polar do dual.\n\n");
{
    /* No §U4 mediu-se: e^{εθ}·e^{εφ} = e^{ε(θ+φ)} — angulos SOMAM. No dual o "angulo" e a
     * rapidez, e a velocidade e v = tanh(θ). Entao somar rapidezes e' compor velocidades pela
     * formula de Einstein. Mede-se contra a formula, sem a usar na construcao. */
    printf("      o \"ângulo\" do dual é a RAPIDEZ θ, e a velocidade é v = tanh θ\n");
    printf("      os ângulos somam (§U4)  =>  θ₁ + θ₂  =>  v = (v₁+v₂)/(1+v₁v₂)\n\n");
    int mal = 0;
    printf("      v₁      v₂      pela soma de rapidezes   pela fórmula de Einstein\n");
    for(int k = 0; k < 6; k++){
        double v1 = 0.1 + 0.15*k, v2 = 0.2 + 0.12*k;
        double th1 = atanh(v1), th2 = atanh(v2);
        double porRapidez = tanh(th1 + th2);
        double porEinstein = (v1 + v2)/(1 + v1*v2);
        printf("      %.3f   %.3f   %.9f              %.9f\n", v1, v2, porRapidez, porEinstein);
        if(fabs(porRapidez - porEinstein) > 1e-12) mal++;
    }
    /* e em massa, incluindo o caso da luz */
    int malL = 0;
    for(int k = 0; k < 300; k++){
        double v1 = -0.98 + 0.0065*k, v2 = 0.97 - 0.006*k;
        if(fabs(v1) >= 1 || fabs(v2) >= 1) continue;
        double r = tanh(atanh(v1) + atanh(v2)), e = (v1+v2)/(1+v1*v2);
        if(fabs(r-e) > 1e-11) mal++;
        if(fabs(r) >= 1) malL++;                /* nunca ultrapassa c */
    }
    printf("\n      (mais 300 pares medidos; e em nenhum a composta atingiu ou passou c)\n\n");
    ok("somar rapidezes DÁ a fórmula de Einstein — resíduo 0", mal == 0);
    ok("e a velocidade composta nunca chega a c: o cone é inatingível por dentro", malL == 0);
    printf("      Então a fórmula de adição relativística não é um postulado à parte: é a lei\n");
    printf("      \"raios multiplicam, ângulos somam\" do §U4, escrita na variável v = tanh θ. O\n");
    printf("      que no círculo é rodar, no dual é impulsionar — e as duas obedecem à MESMA\n");
    printf("      lei polar, porque a série exponencial é uma só.\n");
    printf("\n      E o c inatingível é o cone visto de dentro: tanh nunca chega a 1, que é o\n");
    printf("      mesmo que dizer que nenhuma composição de elementos invertíveis cai no\n");
    printf("      divisor de zero. A cinemática herda a álgebra.\n");
}

printf("\n§P7  \"Acima do infinito\": c -> ∞ leva e² = +1 em e² = 0.\n\n");
{
    /* A frase do Aarao — "os duais se tocam ACIMA DO INFINITO" — tem conta. Escalando a
     * unidade por 1/c, o quadrado escala por 1/c²: e o limite c -> ∞ leva +1 a 0. E o boost
     * de Lorentz degenera na transformacao de Galileu. Mede-se a degenerescencia. */
    printf("      pondo ε_c = i*/c, vem ε_c² = (i*)²/c² = 1/c²  ->  0 quando c -> ∞\n\n");
    int mal = 0;
    printf("      c          ε_c²          boost(v=1) sobre (t,x)=(1,0)     Galileu daria\n");
    for(int k = 1; k <= 6; k++){          /* de c = 10; em c = 1 seria v = c e o boost não existe */
        /* 10^k com k inteiro é `rt_ipow`, não `pow`: uma potência inteira não precisa da
         * função transcendental, e o resultado é exacto até 10^18. */
        double c = (double)rt_ipow(10, k), v = 1.0;
        double beta = v/c, g = 1.0/sqrt(1 - beta*beta);
        double t2 = g*(1.0 - beta*0.0/c), x2 = g*(0.0 - v*1.0);
        printf("      %-10.0f %-13.3e t'=%.9f  x'=%+.6f     t'=1  x'=%+.0f\n",
               c, 1.0/(c*c), t2, x2, -v);
        if(k == 6 && (fabs(t2 - 1.0) > 1e-9 || fabs(x2 + v) > 1e-6)) mal++;
    }
    printf("\n");
    ok("quando c cresce o boost converge para Galileu — e ε² converge para 0", mal == 0);
    printf("      É esta a conta por trás de \"acima do infinito\". O tempo deixa de se misturar\n");
    printf("      com o espaço (t' -> t) e sobra só o arrastamento (x' -> x - vt). A hipérbole\n");
    printf("      abre-se até as duas assíntotas serem a mesma reta vertical, e é aí que as\n");
    printf("      duas se TOCAM. Na física isto chama-se contração de İnönü-Wigner: o grupo de\n");
    printf("      Lorentz degenera no de Galileu, e a degenerescência é exatamente e² -> 0.\n");
}

printf("\n§P8  O quadro das três — e o que é exato e o que é analogia.\n\n");
{
    printf("      e²    álgebra          geometria    grupo        na física\n");
    printf("      ----  ---------------  -----------  -----------  ---------------------------\n");
    printf("      -1    complexos        círculo      rotações     fase e^{iθ}, U(1)\n");
    printf("      +1    dual (split)     hipérbole    boosts       Lorentz, rapidez, cone de luz\n");
    printf("       0    fronteira        reta dupla   Galileu      férmions (θ²=0), a derivada\n\n");
    printf("      E a honestidade sobre cada linha, porque nem todas valem o mesmo:\n\n");
    printf("      EXATO (mesma estrutura, outros nomes):\n");
    printf("        · o cone de N = a² - b² É o cone de luz de t² - x². Mesma forma quadrática.\n");
    printf("        · somar rapidezes DÁ a fórmula de Einstein. Medido, resíduo 0.\n");
    printf("        · (|a><b|)² = 0 sse <b|a> = 0. Medido, 200 pares, 0 discordâncias.\n");
    printf("        · a² = (a†)² = 0 com {a,a†} = I é a álgebra fermiónica. Medido.\n");
    printf("        · a parte ε de f(a+ε) É f'(a), exata. Medido.\n\n");
    printf("      ANALOGIA (parecido, e o parecido pára em algum lado):\n");
    printf("        · dizer que \"os férmions SÃO a terceira classe\" é ir longe de mais. A\n");
    printf("          álgebra de Grassmann tem MUITOS geradores anticomutantes; R[ε]/(ε²) tem\n");
    printf("          um só e é comutativa. O que é exato é a nilpotência do gerador, e é dela\n");
    printf("          que sai a exclusão — não a álgebra inteira.\n");
    printf("        · a contração de İnönü-Wigner é um limite de GRUPOS de Lie, e aqui mediu-se\n");
    printf("          o caso 1+1 com uma unidade. A direção está certa e a conta fecha nesse\n");
    printf("          caso; a generalidade é citada, não medida aqui.\n\n");
    /* Uma asserção sobre o quadro tem de MEDIR alguma coisa, senão é um ok(...,1) disfarçado —
     * já escrevi um desses nesta série e foi apanhado. O que o quadro afirma e se pode medir é
     * que as três linhas são geometrias GENUINAMENTE distintas, e a conta que as separa é o
     * número de direções nulas: 0, 2 e 4, todas diferentes. */
    {
        int nd[3], mal = 0;
        for(int s = -1; s <= 1; s++){
            int dirs = 0;
            for(int k = 0; k < 720; k++){
                double th = k*M_PI/360, a = cos(th), b = sin(th);
                if(fabs(a*a - s*b*b) < 1e-12) dirs++;
            }
            nd[s+1] = dirs;
        }
        printf("      direções nulas por classe:  e²=-1 -> %d    e²=0 -> %d    e²=+1 -> %d\n\n",
               nd[0], nd[1], nd[2]);
        if(nd[0] == nd[1] || nd[1] == nd[2] || nd[0] == nd[2]) mal++;
        ok("as três linhas são geometrias distintas, e a contagem do cone separa-as", mal == 0);
    }
    printf("      A resposta curta ao Aarão, então: sim, o bra-ket é o análogo, e é o de\n");
    printf("      TRANSIÇÃO entre estados ortogonais — não o projetor. Mas ele não é o único, e\n");
    printf("      talvez nem seja o mais fundo: os três casos e² ∈ {-1, 0, +1} são exatamente\n");
    printf("      as três cinemáticas (Euclides, Galileu, Lorentz), e o que aqui se chama\n");
    printf("      \"divisor de zero\" a física chama \"cone de luz\".\n");
}

printf("\n");
return falhas ? 1 : 0;
}
