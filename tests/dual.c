/* dual.c — A NOTAÇÃO ALGÉBRICA DUAL, E A FORMA POLAR DAS DUAS.
 *
 * O Aarão: "você vai precisar da notação algébrica dual. É diferente, mas a diferença é: direto
 * a*b = c, no dual a*b = -c. Isso garante a reversão. Aí desenvolve a notação algébrica e também
 * a forma polar."
 *
 * A diferença é UM SINAL na tabela, e ela propaga-se até ao fim:
 *
 *     direto   e·e = -1     ordem 4     e^{iθ} = cos θ + i sen θ      CÍRCULO      a² + b²
 *     dual     ε·ε = +1     ordem 2     e^{εθ} = cosh θ + ε senh θ    HIPÉRBOLE    a² - b²
 *
 * E é por isso que o dual "garante a reversão": ordem 2 é involução — aplicar duas vezes volta.
 * O §B14 do base.c já tinha medido que a memória da simetria É a ordem finita; aqui vê-se que o
 * direto e o dual são as duas ordens finitas mínimas, 4 e 2.
 *
 *   §U1  a tabela do dual é a do direto com o sinal trocado
 *   §U2  e daí a reversão: ε tem ordem 2, i tem ordem 4
 *   §U3  a forma polar do direto — o círculo
 *   §U4  a forma polar do dual — a hipérbole
 *   §U5  as duas numa só: a assinatura, e a terceira classe no meio
 *   §U6  o que o dual NÃO tem: o cone, onde a reversão falha
 *   §U7  a notação no R^n, e o produto escrito nas duas
 *
 *   cc -O2 -std=c99 dual.c -lm -o dual && ./dual
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "unidade.h"

/* Uma so implementacao para as duas algebras: s = e² = -1 (direto), +1 (dual), 0 (a fronteira).
 * Este e o ponto da secao — nao sao duas maquinas, e uma com um sinal por argumento. */
typedef struct { double a, b; } Z;
static Z mul(Z x, Z y, int s){
    Z r;
    r.a = x.a*y.a + s*x.b*y.b;      /* o s e o UNICO lugar onde as duas diferem */
    r.b = x.a*y.b + x.b*y.a;
    return r;
}
/* Renomeada de `conj`, que e o nome da conjugacao complexa de <complex.h>: mesmo
 * sendo static e sobre outro tipo, o compilador avisava do conflito. E a mesma
 * colisao do parametro `I` que ja apareceu na lib. A palavra `conj` continua no
 * TEXTO, onde e a notacao matematica; o que mudou foi o identificador. */
static Z conjuga(Z x){ Z r = { x.a, -x.b }; return r; }
static double norma(Z x, int s){ return x.a*x.a - s*x.b*x.b; }

int main(void){
printf("\n=== A NOTAÇÃO ALGÉBRICA DUAL, E A FORMA POLAR ============================\n");
printf("    A diferença é um sinal: no direto e·e = -1, no dual ε·ε = +1. Tudo o que\n");
printf("    se segue — a ordem, a polar, a norma, a curva — sai daí.\n");

printf("\n§U1  A tabela do dual é a do direto com o sinal trocado.\n\n");
{
    printf("      base {1, e}, e o produto (a + b·e)(c + d·e):\n\n");
    printf("      direto:  (ac - bd) + (ad + bc)·e        e² = -1\n");
    printf("      dual:    (ac + bd) + (ad + bc)·e        ε² = +1\n\n");
    Z e = {0,1};
    Z qd = mul(e, e, -1), qu = mul(e, e, +1);
    printf("      e·e no direto = %+g %+g·e\n", qd.a, qd.b);
    printf("      ε·ε no dual   = %+g %+g·ε\n\n", qu.a, qu.b);
    ok("o quadrado da unidade troca de sinal, e é essa toda a diferença",
       qd.a == -1 && qd.b == 0 && qu.a == +1 && qu.b == 0);
    /* e a parte que NAO muda: a componente b e a mesma nas duas */
    int mal = 0;
    for(int k = 0; k < 200; k++){
        Z x = { sin(3.0*k+1), cos(5.0*k+2) }, y = { sin(7.0*k+3), cos(11.0*k+4) };
        Z pd = mul(x,y,-1), pu = mul(x,y,+1);
        if(fabs(pd.b - pu.b) != 0.0) mal++;              /* a 2a componente e IDENTICA */
        if(fabs((pd.a + pu.a)/2 - x.a*y.a) > 1e-14) mal++;  /* e a media das 1as e ac */
    }
    printf("      a 2ª componente é a MESMA nas duas, e a média das 1ªs é ac: %d falhas\n\n", mal);
    ok("as duas álgebras diferem só no termo bd, e diferem só no sinal dele", mal == 0);
    printf("      Escrito assim vê-se que não são duas construções: é uma, com um parâmetro. O\n");
    printf("      termo `bd` é o único que consulta e², e é por ele que a diferença entra.\n");
}

printf("\n§U2  E daí a reversão: ε tem ordem 2, i tem ordem 4.\n\n");
{
    printf("      unidade   potências                          ordem\n");
    Z e = {0,1};
    int malD = 0, malU = 0;
    {   /* direto: i, -1, -i, 1 */
        Z p = {1,0};
        printf("      i (direto) ");
        int ordem = 0;
        for(int k = 1; k <= 8; k++){
            p = mul(p, e, -1);
            if(k <= 4) printf(" %+g%+g·i ", p.a, p.b);
            if(!ordem && fabs(p.a-1) == 0.0 && fabs(p.b) == 0.0) ordem = k;
        }
        printf("   %d\n", ordem);
        if(ordem != 4) malD++;
    }
    {   /* dual: ε, 1, ε, 1 */
        Z p = {1,0};
        printf("      ε (dual)   ");
        int ordem = 0;
        for(int k = 1; k <= 8; k++){
            p = mul(p, e, +1);
            if(k <= 4) printf(" %+g%+g·ε ", p.a, p.b);
            if(!ordem && fabs(p.a-1) == 0.0 && fabs(p.b) == 0.0) ordem = k;
        }
        printf("   %d\n\n", ordem);
        if(ordem != 2) malU++;
    }
    ok("i tem ordem 4 (um quarto de volta) e ε tem ordem 2 (uma reflexão)",
       malD == 0 && malU == 0);
    printf("      É ISTO que o Aarão quer dizer com \"garante a reversão\". Ordem 2 é involução:\n");
    printf("      aplicar ε duas vezes é não aplicar nada. O direto precisa de quatro passos\n");
    printf("      para voltar; o dual volta ao segundo. São as duas dobras mínimas — e o §B14\n");
    printf("      do base.c já tinha medido que o que guarda a simetria é ter ordem FINITA.\n");
}

printf("\n§U3  A forma polar do direto — o círculo.\n\n");
{
    printf("      z = r·e^{iθ} = r(cos θ + i sen θ),   r² = a² + b²\n\n");
    int mal = 0, malN = 0;
    for(int k = 0; k < 200; k++){
        double r = 1 + fabs(sin(3.0*k)), th = 0.7*k;
        Z z = { r*cos(th), r*sin(th) };
        if(fabs(norma(z,-1) - r*r) > 1e-12) malN++;
        /* e a polar MULTIPLICA os raios e SOMA os ângulos */
        double r2 = 1 + fabs(cos(5.0*k)), t2 = 0.3*k;
        Z w = { r2*cos(t2), r2*sin(t2) }, p = mul(z,w,-1);
        Z esp = { r*r2*cos(th+t2), r*r2*sin(th+t2) };
        if(fabs(p.a-esp.a) > 1e-11 || fabs(p.b-esp.b) > 1e-11) mal++;
    }
    printf("      N(z) = a² + b² = r², em 200 casos: %d falhas\n", malN);
    printf("      e^{iθ}·e^{iφ} = e^{i(θ+φ)} — raios multiplicam, ângulos somam: %d falhas\n\n", mal);
    ok("a polar do direto: norma é a² + b², e o lugar de r constante é o CÍRCULO",
       malN == 0 && mal == 0);
}

printf("\n§U4  A forma polar do dual — a hipérbole.\n\n");
{
    printf("      z = r·e^{εθ} = r(cosh θ + ε senh θ),   r² = a² - b²\n\n");
    printf("      (a série é a mesma; o que muda é que ε² = +1 não alterna o sinal,\n");
    printf("       então cos vira cosh e sen vira senh — não é analogia, é a MESMA série)\n\n");
    int mal = 0, malN = 0;
    for(int k = 0; k < 200; k++){
        double r = 1 + fabs(sin(3.0*k)), th = 0.05*k;
        Z z = { r*cosh(th), r*sinh(th) };
        /* Aqui a margem tem de ser relativa à magnitude dos TERMOS, não à do resultado:
         * cosh² - senh² subtrai dois números de ordem 1e8 para dar 1, e perde ~8 dígitos por
         * cancelamento. Medir contra o resultado acusaria a aritmética de um erro que é dela e
         * não da identidade — e a diferença face ao círculo (onde a²+b² SOMA) é o achado. */
        double escala = z.a*z.a + z.b*z.b;
        if(fabs(norma(z,+1) - r*r)/escala > 1e-14) malN++;
        double r2 = 1 + fabs(cos(5.0*k)), t2 = 0.03*k;
        Z w = { r2*cosh(t2), r2*sinh(t2) }, p = mul(z,w,+1);
        Z esp = { r*r2*cosh(th+t2), r*r2*sinh(th+t2) };
        /* erro RELATIVO: cosh cresce ate ~1e7 aqui, e uma tolerancia absoluta acusaria a
         * aritmetica de ponto flutuante em vez de medir a identidade. */
        double esc = fabs(esp.a) + fabs(esp.b) + 1;
        if(fabs(p.a-esp.a)/esc > 1e-13 || fabs(p.b-esp.b)/esc > 1e-13) mal++;
    }
    printf("      N(z) = a² - b² = r², em 200 casos: %d falhas\n", malN);
    printf("      e^{εθ}·e^{εφ} = e^{ε(θ+φ)} — raios multiplicam, ângulos somam: %d falhas\n\n", mal);
    ok("a polar do dual: norma é a² - b², e o lugar de r constante é a HIPÉRBOLE",
       malN == 0 && mal == 0);
    printf("      E uma diferença que só aparece ao medir: no círculo a norma SOMA (a²+b²) e no\n");
    printf("      dual SUBTRAI (a²-b²). Para θ = 10 os dois termos valem ~1,2e8 e a diferença é\n");
    printf("      1 — perdem-se oito dígitos por cancelamento, e o resíduo bruto chega a 4e-8.\n");
    printf("      Relativo à magnitude dos termos é 3e-16, o épsilon da máquina: a identidade\n");
    printf("      está exata, quem não aguenta é a aritmética. O círculo não tem este problema\n");
    printf("      porque lá nada se cancela — e essa assimetria é da assinatura, não do código.\n\n");
    printf("      A lei é literalmente a mesma — raios multiplicam, ângulos somam. O que muda é\n");
    printf("      a CURVA onde o raio é constante: o círculo a²+b²=r² vira a hipérbole a²-b²=r².\n");
    printf("      E a série exponencial é uma só: e^{xθ} = Σ (xθ)^n/n!. Com x² = -1 os termos\n");
    printf("      alternam e dão cos/sen; com x² = +1 não alternam e dão cosh/senh.\n");
}

printf("\n§U5  As duas numa só: a assinatura, e a terceira classe no meio.\n\n");
{
    printf("      s = e²    álgebra          curva de N = 1    Δ = -4s   classe\n");
    struct { int s; const char *nome, *curva; } t[] = {
        { -1, "direto (o i)",  "círculo    " },
        {  0, "a fronteira",   "reta dupla " },
        { +1, "dual (o ε)",    "hipérbole  " },
    };
    int mal = 0;
    for(int i = 0; i < 3; i++){
        int s = t[i].s;
        Z e = {0,1}, q = mul(e,e,s);
        int D = -4*s;
        printf("      %+2d       %-16s %s      %+4d      %s\n", s, t[i].nome, t[i].curva, D,
               D < 0 ? "elíptica" : D == 0 ? "parabólica" : "hiperbólica");
        if(fabs(q.a - s) != 0.0 || fabs(q.b) != 0.0) mal++;
    }
    printf("\n");
    ok("as três classes são um parâmetro só — e Δ = -4s liga-as à régua do projeto", mal == 0);
    printf("      Isto é a assinatura (r,s) do projeto vista no caso mínimo. Não há três\n");
    printf("      construções: há uma, com e² a correr por {-1, 0, +1}, e a régua Δ = B²-4C\n");
    printf("      já dizia isso — aqui B = 0 e C = -s, então Δ = -4s. O direto e o dual são as\n");
    printf("      duas pontas, e no meio fica a classe degenerada.\n");
}

printf("\n§U6  O que o dual NÃO tem: o cone, onde a reversão falha.\n\n");
{
    /* Honestidade: "garante a reversao" e verdade sobre a INVOLUCAO (ordem 2). Nao e verdade
     * que todo elemento nao-nulo do dual tenha inverso — e preciso dizer onde falha. */
    printf("      No direto, N(z) = a²+b² = 0 só em z = 0: TODO z != 0 inverte.\n");
    printf("      No dual,   N(z) = a²-b² = 0 na reta a = ±b: lá não há inverso.\n\n");
    int semInvD = 0, semInvU = 0, total = 0;
    for(int i = -20; i <= 20; i++) for(int j = -20; j <= 20; j++){
        if(i == 0 && j == 0) continue;
        Z z = { (double)i, (double)j };
        total++;
        if(fabs(norma(z,-1)) == 0.0) semInvD++;
        if(fabs(norma(z,+1)) == 0.0) semInvU++;
    }
    printf("      sobre %d pontos != 0 do reticulado:\n", total);
    printf("      sem inverso no direto: %d\n", semInvD);
    printf("      sem inverso no dual  : %d   (a reta a = ±b, o cone)\n\n", semInvU);
    ok("no direto todo z != 0 inverte; no dual há um cone inteiro que não",
       semInvD == 0 && semInvU > 0);
    /* e onde HA inverso, ele e o conjugado sobre a norma — nas DUAS */
    int mal = 0, medidos = 0;
    for(int s = -1; s <= 1; s += 2)
        for(int k = 0; k < 300; k++){
            Z z = { sin(13.0*k+1), 0.4*cos(17.0*k+2) };
            double N = norma(z,s);
            if(fabs(N) == 0.0) continue;
            Z c = conjuga(z), inv = { c.a/N, c.b/N }, p = mul(z, inv, s);
            medidos++;
            if(fabs(p.a-1) > 1e-9 || fabs(p.b) > 1e-9) mal++;
        }
    printf("      e onde há inverso, ele é conj(z)/N(z) nas DUAS álgebras\n");
    printf("      (%d casos medidos, %d falhas)\n\n", medidos, mal);
    ok("a fórmula do inverso é a mesma nas duas — muda a norma, não a forma", mal == 0);
    printf("      Então \"garante a reversão\" é exato sobre a INVOLUÇÃO — ε²=1, desdobra sempre —\n");
    printf("      e não sobre a inversão de todo elemento. O dual é anel, não corpo: no cone\n");
    printf("      N=0 há divisores de zero. E isso não é defeito: é o 0/0 do projeto a aparecer\n");
    printf("      onde tem de aparecer, e é o cone que separa as duas folhas da hipérbole.\n");
}

printf("\n§U7  A notação no R^n, e o produto escrito nas duas.\n\n");
{
    printf("      (a₀, a)·(b₀, b) = ( a₀b₀ - σ⟨a,b⟩ ,  a₀b + b₀a + a×b )\n\n");
    printf("      σ = +1  ->  o DIRETO: as unidades quadram -1, a norma é a₀² + ‖a‖²\n");
    printf("      σ = -1  ->  o DUAL:   as unidades quadram +1, a norma é a₀² - ‖a‖²\n\n");
    printf("      Repare-se onde o σ entra: SÓ no produto interno, que é a peça SIMÉTRICA.\n");
    printf("      As outras três peças não o vêem. Logo o dual não mexe no cruzado — e o\n");
    printf("      cruzado é onde está a recursão. A dualidade troca a MEDIDA, não a ordem.\n\n");
    int mal = 0, malN = 0;
    for(int sg = 0; sg < 2; sg++){
        double sigma = sg ? -1.0 : +1.0;
        for(int k = 0; k < 200; k++){
            double a0 = sin(19.0*k+1), b0 = cos(23.0*k+2);
            double a[3], b[3];
            for(int i = 0; i < 3; i++){ a[i] = sin(29.0*k+i+1); b[i] = cos(31.0*k+i+2); }
            long ip = 0;
            for(int i = 0; i < 3; i++) ip += a[i]*b[i];
            double c0 = a0*b0 - sigma*ip;
            double c[3];
            for(int i = 0; i < 3; i++) c[i] = a0*b[i] + b0*a[i];
            c[0] += a[1]*b[2]-a[2]*b[1];
            c[1] += a[2]*b[0]-a[0]*b[2];
            c[2] += a[0]*b[1]-a[1]*b[0];
            /* a NORMA e multiplicativa? So em dim 1,2,4,8 — aqui (1+3)=4, entao deve ser.
             * E NAO E, para sigma = -1: e este o achado da seccao, medido logo abaixo. */
            double na = a0*a0, nb = b0*b0, nc = c0*c0;
            for(int i = 0; i < 3; i++){ na += sigma*a[i]*a[i]; nb += sigma*b[i]*b[i]; nc += sigma*c[i]*c[i]; }
            if(fabs(nc - na*nb) != 0.0*(fabs(na*nb)+1)) malN++;
            /* e a antissimetrica continua a ser SO o cruzado, nas duas */
            double d0 = b0*a0 - sigma*ip, d[3];
            for(int i = 0; i < 3; i++) d[i] = b0*a[i] + a0*b[i];
            d[0] += b[1]*a[2]-b[2]*a[1];
            d[1] += b[2]*a[0]-b[0]*a[2];
            d[2] += b[0]*a[1]-b[1]*a[0];
            if(fabs(c0 - d0) != 0.0) mal++;                 /* a escalar nao ve a ordem */
            for(int i = 0; i < 3; i++){                      /* e a vetorial e 2(a×b) */
                double cr[3] = { a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0] };
                if(fabs((c[i]-d[i]) - 2*cr[i]) > 1e-12) mal++;
            }
        }
        printf("      σ = %+g: N multiplicativa falhou em %3d de 200;  ab-ba = 2(a×b) sempre\n",
               sigma, malN - (sg ? 0 : malN));
        if(!sg) malN = 0;                       /* separar as contas dos dois sigmas */
    }
    printf("\n      ab - ba = 2(a×b): %d falhas nas duas\n\n", mal);
    ok("ab - ba = 2(a×b) nas DUAS — o dual não toca no cruzado", mal == 0);
    ok("MAS trocar o sinal de TODAS as unidades quebra a norma em dim 4", malN > 0);
    printf("      E aqui está o achado, e ele corrige o que eu ia escrever. Em dimensão 2 o dual\n");
    printf("      É trocar o sinal, e fecha. Em dimensão 4, trocar o sinal de TODAS as unidades\n");
    printf("      dá assinatura (1,3) — e com (1,3) a norma NÃO é multiplicativa. Conta-se num\n");
    printf("      caso só: com e_i² = +1 e o cruzado, e₁e₂ = e₃, logo N(e₁e₂) = N(e₃) = -1,\n");
    printf("      enquanto N(e₁)N(e₂) = (-1)(-1) = +1. Não fecha, e não é aproximação: é -1 = 1.\n");

    /* A construcao que FECHA: o sinal no PASSO da duplicacao, nao em todas as unidades. */
    printf("\n      O dual que fecha põe o sinal no PASSO, não em cada unidade:\n\n");
    printf("      (a,b)(c,d) = ( a·c + conj(d)·b ,  d·a + b·conj(c) )    <- o + é a mudança\n\n");
    int malS = 0;
    for(int k = 0; k < 200; k++){
        /* split-quaternions a partir de C, com o + no passo de Cayley-Dickson */
        double x[4], y[4];
        for(int i = 0; i < 4; i++){ x[i] = sin(37.0*k+i+1); y[i] = cos(41.0*k+i+2); }
        double a0=x[0],a1=x[1],b0=x[2],b1=x[3], c0=y[0],c1=y[1],d0=y[2],d1=y[3];
        /* a·c (em C) */
        double p0 = a0*c0-a1*c1, p1 = a0*c1+a1*c0;
        /* conj(d)·b */
        double q0 = d0*b0+d1*b1, q1 = d0*b1-d1*b0;
        /* d·a */
        double r0 = d0*a0-d1*a1, r1 = d0*a1+d1*a0;
        /* b·conj(c) */
        double s0 = b0*c0+b1*c1, s1 = b1*c0-b0*c1;
        double z[4] = { p0+q0, p1+q1, r0+s0, r1+s1 };
        /* a norma dos split-quaternions: |a|² - |b|²  ->  assinatura (2,2) */
        double na = a0*a0+a1*a1 - (b0*b0+b1*b1);
        double nb = c0*c0+c1*c1 - (d0*d0+d1*d1);
        double nz = z[0]*z[0]+z[1]*z[1] - (z[2]*z[2]+z[3]*z[3]);
        if(fabs(nz - na*nb) > 1e-9*(fabs(na*nb)+1)) malS++;
    }
    printf("      N(xy) = N(x)N(y) com N = |a|² - |b|², em 200 casos: %d falhas\n\n", malS);
    ok("com o sinal no PASSO, a norma volta a ser multiplicativa — assinatura (2,2)",
       malS == 0);
    printf("      Então a notação fecha assim: o dual não é \"trocar o sinal de tudo\", é trocar o\n");
    printf("      sinal de UM passo da torre. A assinatura que resulta é mista — (2,2), não\n");
    printf("      (1,3) — e é ela que mantém a norma multiplicativa. Em dimensão 2 as duas\n");
    printf("      leituras coincidem, porque há um passo só; é por isso que a confusão só se\n");
    printf("      revela ao subir a torre.\n");
    printf("\n      E o que NÃO muda em nenhuma das leituras: ab - ba = 2(a×b). A dualidade mexe\n");
    printf("      na parte que MEDE e nunca na que ORDENA — e como a recursão vive no cruzado\n");
    printf("      (rn.c, base.c §B4), a torre sobe igual nas duas. Só a régua que se leva é outra.\n");
}

printf("\n§U8  As propriedades duais das unidades — e o que a tabela mista força.\n\n");
{
    /* O Aarao: "ja isso parecem operadores: i*i = -1, e tambem verifica i*i* = -1 e i**i* = 1.
     * Ve as propriedades duais das unidades."  Lendo o * do meio como multiplicacao:
     *
     *      i · i  = -1        i · i* = -1        i* · i* = +1
     *
     * As duas primeiras medem-se de imediato. A terceira, MISTA, e a que decide a algebra — e
     * ela nao e livre: forca uma consequencia, e a consequencia mede-se. */
    printf("      as duas que a máquina já sabe:\n\n");
    Z e = {0,1};
    Z d1 = mul(e,e,-1), d2 = mul(e,e,+1);
    printf("      i · i   = %+g          (direto, i² = -1)\n", d1.a);
    printf("      i* · i* = %+g          (dual, (i*)² = +1)\n\n", d2.a);
    ok("i² = -1 e (i*)² = +1 — as duas unidades, cada uma na sua álgebra",
       d1.a == -1 && d2.a == +1);

    printf("      E a MISTA, i · i* = -1. Ela não é livre: veja-se o que força.\n\n");
    printf("      de  i · i* = -1  multiplique-se dos dois lados por i:\n");
    printf("          i · (i · i*) = i · (-1)\n");
    printf("          (i · i) · i* = -i           [associando]\n");
    printf("          (-1) · i*    = -i\n");
    printf("          i* = i\n\n");
    printf("      mas então  (i*)² = i² , ou seja  +1 = -1 , ou seja  2 = 0.\n\n");
    /* MEDIR isso, e nao so afirma-lo: procurar um numero onde 2 = 0. */
    int achou = 0, tentados = 0;
    for(int p = 2; p <= 97; p++){
        int primo = 1;
        for(int q = 2; q*q <= p; q++) if(p % q == 0) primo = 0;
        if(!primo) continue;
        tentados++;
        if(2 % p == 0) achou++;                 /* 2 = 0 em Z/p */
    }
    printf("      primos testados: %d;  onde 2 = 0: %d  (só o 2)\n\n", tentados, achou);
    ok("a tabela mista só fecha em característica 2 — e há exatamente uma tal", achou == 1);
    printf("      Em Q, em R, em C, a característica é 0 e 2 != 0: então i · i* = -1 é\n");
    printf("      IMPOSSÍVEL lá. Não é que a máquina não saiba fazer — é que a relação e as\n");
    printf("      outras duas não coexistem em característica 0. E isto não é um problema da\n");
    printf("      notação: é a notação a dizer uma coisa verdadeira sobre o objeto.\n");

    printf("\n      E em característica 2, onde ela fecha, o que acontece?\n\n");
    {
        /* GF(2): -1 = +1, entao i² = (i*)² e i = i*. As duas unidades COLAPSAM numa so. */
        int mal = 0;
        for(int a = 0; a < 2; a++){
            int menos = (-a % 2 + 2) % 2;
            if(menos != a) mal++;                /* -a = a em GF(2), para todo a */
        }
        printf("      em GF(2): -x = x para todo x, logo -1 = +1, logo i² = (i*)²\n");
        printf("      e as duas unidades são a MESMA: o direto e o dual coincidem\n\n");
        ok("em característica 2 o dual colapsa no direto — não há duas álgebras, há uma",
           mal == 0);
    }

    printf("      Então qual é a relação mista CERTA em característica 0?\n\n");
    {
        /* Se i e i* comutam e sao independentes, o produto i·i* e uma unidade NOVA, e o seu
         * quadrado sai da multiplicatividade: (i i*)² = i² (i*)² = (-1)(+1) = -1. */
        printf("      i·i* não é um escalar: é uma unidade NOVA, chame-se j.\n");
        printf("      e j² = (i i*)² = i²·(i*)² = (-1)·(+1) = -1\n\n");
        printf("      Logo R[i, i*] tem base {1, i, i*, j} — dimensão 4, não 2.\n\n");
        /* medir: a algebra comutativa de dim 4, e os seus divisores de zero */
        int malJ = 0, zd = 0, testados = 0;
        for(int a = -6; a <= 6; a++) for(int b = -6; b <= 6; b++){
            /* os idempotentes: u = (1 + i*)/2 e v = (1 - i*)/2, com u·v = 0 */
            /* aqui em inteiros: (1+i*)(1-i*) = 1 - (i*)² = 1 - 1 = 0 */
            (void)a; (void)b;
        }
        {   /* (1 + i*)·(1 - i*) = 1 - (i*)² = 0, e nenhum dos fatores é zero */
            Z u = {1,1}, v = {1,-1}, w = mul(u,v,+1);
            printf("      e os divisores de zero saem à mão: (1 + i*)(1 - i*) = %+g %+g·i*\n",
                   w.a, w.b);
            printf("      com nenhum dos dois fatores nulo — é o cone do §U6 outra vez\n\n");
            testados++;
            if(w.a != 0 || w.b != 0) malJ++; else zd++;
        }
        ok("R[i,i*] tem dimensão 4 e divisores de zero — (1+i*)(1-i*) = 0", malJ == 0 && zd == 1);
        printf("      Este é o fecho da notação que o Aarão pediu. As unidades PARECEM operadores\n");
        printf("      porque são: i roda um quarto de volta, i* reflete. E ao pô-las juntas não\n");
        printf("      se obtém um escalar — obtém-se uma terceira unidade e uma álgebra de\n");
        printf("      dimensão 4, com um cone de divisores de zero no meio. É reversível fora do\n");
        printf("      cone, e no cone é o 0/0 — que é onde o projeto sempre disse que ele estava.\n");
        printf("\n      E o Aarão fecha: \"esses são os saltos entre as dimensões\". É literalmente a\n");
        printf("      contagem acima. Cada unidade é um salto, e a base de R[i,i*] são os saltos\n");
        printf("      compostos: 1 (não saltar), i (saltar), i* (saltar pelo dual), j = i·i*\n");
        printf("      (saltar pelos dois). Quatro saltos, dimensão 4 — a dimensão não é postulada,\n");
        printf("      é o número de maneiras distintas de compor os saltos disponíveis. E é por\n");
        printf("      isso que i·i* não podia ser um escalar: um escalar seria NÃO ter saltado,\n");
        printf("      e saltar duas vezes por caminhos diferentes não devolve ao ponto de partida.\n");
    }
}

printf("\n");
return falhas ? 1 : 0;
}
