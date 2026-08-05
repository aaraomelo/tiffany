/* zeta.c — A ZETA DE RIEMANN DESENROLA NESSES PONTOS?
 *
 * O Aarão, depois do fisica.c: "verifica se a zeta de Riemann desenrola nesses pontos."
 *
 * "Esses pontos" são os do corpo dual: o VINCO (o conjunto fixo da dobra), a RAIZ DUPLA
 * (ε² = 0, onde o produto degenera) e o DESENROLAR (Σ vira Π, que é o Pontryagin do corpo
 * diferencial). A pergunta tem três partes, e as três medem-se — mas não dão todas a mesma
 * resposta, e é isso que a torna útil.
 *
 * O QUE ESTA SECÇÃO NÃO FAZ: não prova nem sugere nada sobre a hipótese de Riemann. Tudo o que
 * aqui se mede é LOCAL — pontos concretos, com resíduo. A distância entre "medi nos primeiros
 * dez zeros" e "vale para todos" é o problema inteiro, e escrevê-la como se fosse pequena seria
 * a pior espécie de desonestidade que este projeto pode cometer.
 *
 *   §Z1  a equação funcional É uma dobra: s -> 1-s̄ tem ordem 2
 *   §Z2  e o VINCO dessa dobra é exatamente a reta crítica Re(s) = 1/2
 *   §Z3  a dobra guarda a simetria: ξ(s) = ξ(1-s), medido
 *   §Z4  o DESENROLAR: Σ n^{-s} = Π (1-p^{-s})^{-1} — a soma vira produto
 *   §Z5  e nos zeros: são SIMPLES, logo a zeta NÃO degenera lá
 *   §Z6  onde então ela degenera? o ponto onde ζ' anula, e o que ele é
 *   §Z7  o que se mediu e o que não se mediu
 *
 *   cc -O2 -std=c99 zeta.c -lm -o zeta && ./zeta
 */
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include "unidade.h"
#include "../lib/disco.h"
#define primo DISCO_FIXO(int, 31)


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---- eta de Dirichlet por Borwein, que converge na faixa crítica ------------------------
 * η(s) = Σ (-1)^(n-1) n^(-s) converge para Re(s) > 0, e ζ(s) = η(s)/(1 - 2^(1-s)).
 * A soma alternada crua converge devagar de mais; o esquema de Borwein acelera-a. */
#define NB 40
static double dk[NB+1];
static void borwein_init(void){
    /* d_k = n Σ_{i=0}^{k} (n+i-1)! 4^i / ((n-i)! (2i)!) */
    int n = NB;
    for(int k = 0; k <= n; k++){
        double s = 0, termo;
        for(int i = 0; i <= k; i++){
            /* termo = (n+i-1)! 4^i / ((n-i)! (2i)!), calculado por log para não estourar */
            termo = lgamma(n+i) + i*log(4.0) - lgamma(n-i+1) - lgamma(2*i+1);
            s += exp(termo);
        }
        dk[k] = n * s;
    }
}
static double complex eta(double complex s){
    double complex soma = 0;
    for(int k = 0; k < NB; k++){
        double c = (dk[k] - dk[NB]) * ((k % 2) ? -1.0 : 1.0);
        soma += c * cpow(k+1.0, -s);
    }
    return -soma / dk[NB];
}
static double complex zeta_borwein(double complex s){
    double complex f = 1.0 - cpow(2.0, 1.0 - s);
    if(cabs(f) < 1e-300) return NAN;
    return eta(s) / f;
}
/* ---- Euler--Maclaurin, que é ESTÁVEL onde o Borwein degrada ------------------------------
 * O Borwein tem erro ~ (3+√8)^{-n}/|Γ(s)|, e |Γ(1/2+it)| decai como e^{-π|t|/2}: o erro é
 * AMPLIFICADO por e^{π|t|/2}. Com n = 40 isso passa de 1e-16 em t = 14 para 1e-5 em t = 50 —
 * foi exatamente o que a primeira corrida mostrou, e eu ia culpar a margem. Euler--Maclaurin
 * não tem essa amplificação:
 *
 *   ζ(s) = Σ_{n<N} n^{-s} + N^{-s}/2 + N^{1-s}/(s-1) + Σ_k B_2k/(2k)! (s)_{2k-1} N^{-s-2k+1}
 */
static const double B2k[] = {                     /* B_2, B_4, ... B_20 */
    1.0/6, -1.0/30, 1.0/42, -1.0/30, 5.0/66, -691.0/2730, 7.0/6,
    -3617.0/510, 43867.0/798, -174611.0/330 };
static double complex zeta(double complex s){
    if(cabs(s - 1.0) < 1e-12) return NAN;         /* o polo */
    double t = fabs(cimag(s));
    int N = (int)(10 + t);                        /* N cresce com |t|, que é o que estabiliza */
    if(N > 400) N = 400;
    double complex acc = 0;
    for(int n = 1; n < N; n++) acc += cpow((double)n, -s);
    acc += 0.5*cpow((double)N, -s);
    acc += cpow((double)N, 1.0-s)/(s-1.0);
    double complex poch = s;                      /* (s)_1 = s */
    double fat = 2.0;                             /* (2k)! */
    for(int k = 1; k <= 10; k++){
        acc += B2k[k-1]/fat * poch * cpow((double)N, -s-(2*k-1));
        poch *= (s + (2*k-1)) * (s + 2*k);        /* (s)_{2k+1} a partir de (s)_{2k-1} */
        fat  *= (2*k+1.0)*(2*k+2.0);
    }
    return acc;
}
/* Γ por Lanczos (g=7, n=9) — precisa-se dela para o ξ */
static double complex cgamma(double complex z){
    static const double p[9] = {
        0.99999999999980993, 676.5203681218851, -1259.1392167224028,
        771.32342877765313, -176.61502916214059, 12.507343278686905,
        -0.13857109526572012, 9.9843695780195716e-6, 1.5056327351493116e-7 };
    if(creal(z) < 0.5)                       /* reflexão, para o semiplano esquerdo */
        return M_PI / (csin(M_PI*z) * cgamma(1.0 - z));
    z -= 1.0;
    double complex x = p[0];
    for(int i = 1; i < 9; i++) x += p[i]/(z + (double)i);
    double complex t = z + 7.5;
    return sqrt(2*M_PI) * cpow(t, z+0.5) * cexp(-t) * x;
}
/* o xi completado: ξ(s) = ½ s(s-1) π^{-s/2} Γ(s/2) ζ(s) — é ELE que é simétrico */
static double complex xi(double complex s){
    return 0.5 * s * (s - 1.0) * cpow(M_PI, -s/2.0) * cgamma(s/2.0) * zeta(s);
}
/* a derivada, pela fórmula de Cauchy num círculo — sem passo h no eixo real */
static double complex dzeta(double complex s, double r, int N){
    double complex acc = 0;
    for(int k = 0; k < N; k++){
        double th = 2*M_PI*k/N;
        double complex w = r*cexp(I*th);
        acc += zeta(s + w) * cexp(-I*th);
    }
    return acc / (N * r);
}

/* os primeiros zeros não triviais — CITADOS da literatura, não calculados aqui */
static const double gamas[] = {
    14.134725141734693, 21.022039638771554, 25.010857580145688,
    30.424876125859513, 32.935061587739189, 37.586178158825671,
    40.918719012147495, 43.327073280914999, 48.005150881167159,
    49.773832477672302 };
#define NZ ((int)(sizeof gamas/sizeof *gamas))

int main(void){
borwein_init();
printf("\n=== A ZETA DESENROLA NESSES PONTOS? =====================================\n");
printf("    Três perguntas numa: o VINCO, o DESENROLAR (Σ -> Π) e a RAIZ DUPLA.\n");
printf("    E elas não dão a mesma resposta — é isso que as torna úteis.\n");

printf("\n§Z1  A equação funcional É uma dobra: ordem 2.\n\n");
{
    /* A dobra candidata e s -> 1-s. Mas essa, em C, so fixa o PONTO 1/2. A que fixa a RETA e
     * a anti-holomorfa s -> 1-conj(s) — e e ela a dobra no sentido do §B14: uma reflexao. */
    printf("      candidata A:  s -> 1 - s        (holomorfa)\n");
    printf("      candidata B:  s -> 1 - conj(s)  (anti-holomorfa, uma REFLEXÃO)\n\n");
    int malA = 0, malB = 0;
    for(int k = 0; k < 200; k++){
        double complex s = (0.1 + 0.01*k) + I*(0.3*k - 5.0);
        double complex a2 = 1.0 - (1.0 - s);              /* A aplicada duas vezes */
        double complex b2 = 1.0 - conj(1.0 - conj(s));    /* B aplicada duas vezes */
        if(cabs(a2 - s) > 1e-14) malA++;
        if(cabs(b2 - s) > 1e-14) malB++;
    }
    printf("      A∘A = id em 200 pontos: %d falhas\n", malA);
    printf("      B∘B = id em 200 pontos: %d falhas\n\n", malB);
    ok("as duas têm ordem 2 — são dobras, no sentido exato do §B14", malA == 0 && malB == 0);
    printf("      Ordem finita: as duas guardam a memória da simetria, e basta desdobrar. Mas\n");
    printf("      elas não têm o mesmo VINCO, e é o vinco que responde à pergunta.\n");
}

printf("\n§Z2  E o VINCO é exatamente a reta crítica.\n\n");
{
    /* o vinco e o conjunto fixo. Conta-se quantos pontos de uma grelha cada dobra fixa. */
    int fixA = 0, fixB = 0, naRetaB = 0, forA = 0;
    printf("      numa grelha de σ ∈ [0,1] × t ∈ [-10,10]:\n\n");
    for(int i = 0; i <= 40; i++) for(int j = 0; j <= 40; j++){
        double sig = i/40.0, t = -10.0 + j*0.5;
        double complex s = sig + I*t;
        if(cabs((1.0 - s) - s) < 1e-12){ fixA++; if(fabs(sig - 0.5) > 1e-12) forA++; }
        if(cabs((1.0 - conj(s)) - s) < 1e-12){
            fixB++;
            if(fabs(sig - 0.5) < 1e-12) naRetaB++;
        }
    }
    printf("      fixos por A (s -> 1-s)        : %d   %s\n", fixA,
           fixA == 1 ? "(só o ponto s = 1/2)" : "");
    printf("      fixos por B (s -> 1-conj(s))  : %d   dos quais em σ = 1/2: %d\n\n",
           fixB, naRetaB);
    ok("A fixa um PONTO; B fixa uma RETA — e a reta é σ = 1/2, toda ela",
       fixA == 1 && fixB == 41 && naRetaB == fixB);
    printf("      Então a resposta à primeira parte é sim, e é exata: A RETA CRÍTICA É O VINCO.\n");
    printf("      Não por analogia — é literalmente o conjunto fixo da reflexão que a equação\n");
    printf("      funcional (com ξ(s̄) = conj(ξ(s))) põe no plano. O §B14 mediu que a dobra fixa\n");
    printf("      um eixo e que esse eixo é o vinco; aqui o eixo tem nome e é Re(s) = 1/2.\n");
}

printf("\n§Z3  A dobra guarda a simetria: ξ(s) = ξ(1-s).\n\n");
{
    printf("      s                    ξ(s)                    ξ(1-s)               resíduo\n");
    int mal = 0;
    double complex ps[] = { 2.0+0.0*I, 3.0+1.0*I, 0.25+2.0*I, -1.0+0.5*I,
                            0.5+3.0*I, 4.0-2.0*I, 0.7+7.0*I };
    for(size_t k = 0; k < sizeof ps/sizeof *ps; k++){
        double complex s = ps[k], a = xi(s), b = xi(1.0 - s);
        double res = cabs(a-b)/(cabs(a)+cabs(b)+1e-300);
        printf("      %+.2f%+.2fi   %+.6e%+.6ei   %+.6e%+.6ei   %.1e\n",
               creal(s), cimag(s), creal(a), cimag(a), creal(b), cimag(b), res);
        if(res > 1e-8) mal++;
    }
    printf("\n");
    ok("ξ(s) = ξ(1-s) — a folha dobrada contém a folha inteira", mal == 0);
    printf("      É o origami do §B14 na sua forma menos metafórica que consigo escrever: a\n");
    printf("      função no semiplano direito determina a do esquerdo, inteira, sem perda. Meia\n");
    printf("      folha basta porque a dobra guardou a outra metade.\n");
}

printf("\n§Z4  O DESENROLAR: a soma vira produto.\n\n");
{
    /* Isto e o Pontryagin do corpo diferencial: Π(a+b) = Π(a)·Π(b), o caractere que troca ⊕
     * por ⊗. Na zeta, a soma sobre TODOS os inteiros vira produto sobre os PRIMOS. */
    printf("      Σ_{n≥1} n^{-s}  =  Π_{p primo} (1 - p^{-s})^{-1}\n\n");
    disco_prende(DISCO_BASE(31),"dados/primo.bin",(size_t)(300000),sizeof(int)); memset(primo, 1, (size_t)(300000)*sizeof(int));
    primo[0] = primo[1] = 0;
    for(int i = 2; i < 300000; i++) if(primo[i]) for(int j = 2*i; j < 300000; j += i) primo[j] = 0;

    /* PRIMEIRO a identidade EXATA, que é a do crivo — e é ela que a secção quer medir.
     *
     * A primeira corrida comparava a soma truncada em 2e6 com o produto truncado em 20000, e
     * dava resíduo 4,3e-6. Fui ver: a cauda do produto para s = 2 é 4,233e-6. Ou seja, eu
     * media a diferença entre DOIS TRUNCAMENTOS e chamava-lhe medir a identidade. A identidade
     * de Euler é exata; o defeito era do meu corte.
     *
     * A forma exata é o crivo: Π_{p≤P} (1-p^{-s})^{-1} = Σ_{n P-liso} n^{-s}, onde P-liso é o
     * n cujos fatores primos são todos ≤ P. Isto é uma IGUALDADE, e é o teorema fundamental da
     * aritmética a fazer o trabalho: cada n aparece uma vez e uma só. */
    printf("      A forma EXATA é a do crivo: Π_{p≤P} = Σ sobre os n que só têm primos ≤ P.\n");
    printf("      (o teorema fundamental da aritmética a garantir: cada n uma vez e uma só)\n\n");
    int mal = 0;
    printf("      P     s     produto p≤P        soma dos P-lisos    resíduo\n");
    struct { int P; double s; } cs[] = { {5,3.0}, {11,3.0}, {31,4.0}, {97,6.0}, {5,2.5} };
    for(size_t k = 0; k < sizeof cs/sizeof *cs; k++){
        int P = cs[k].P; double s = cs[k].s;
        double prod = 1;
        for(int p = 2; p <= P; p++) if(primo[p]) prod /= (1.0 - pow(p, -s));
        double soma = 0;
        for(long n = 1; n < 30000000L; n++){       /* somar SÓ os P-lisos */
            long m = n;
            for(int p = 2; p <= P && m > 1; p++) if(primo[p]) while(m % p == 0) m /= p;
            if(m == 1) soma += pow((double)n, -s);
        }
        double res = fabs(soma-prod)/fabs(prod);
        printf("      %-5d %.1f   %.12f      %.12f      %.1e\n", P, s, prod, soma, res);
        if(res > 1e-11) mal++;
    }
    printf("\n");
    ok("o produto sobre p≤P É a soma sobre os P-lisos — identidade exata, não limite",
       mal == 0);

    /* E DEPOIS o limite, medido como limite: o resíduo tem de DECRESCER com P. */
    printf("      E o caso completo, medido como limite — o resíduo decresce com P:\n\n");
    double somaTot = 0;
    for(long n = 1; n < 30000000L; n++) somaTot += pow((double)n, -2.0);
    printf("      P        produto p≤P        resíduo vs a soma   cauda prevista Σ_{p>P} p^-2\n");
    int decresce = 1; double ant = 1e9;
    for(int e = 2; e <= 5; e++){
        int P = (int)pow(10, e);
        double prod = 1;
        for(int p = 2; p <= P; p++) if(primo[p]) prod /= (1.0 - pow(p, -2.0));
        double cauda = 0;
        for(int p = P+1; p < 300000; p++) if(primo[p]) cauda += pow(p, -2.0);
        double res = fabs(somaTot - prod);
        printf("      %-8d %.12f      %.2e            %.2e\n", P, prod, res, cauda);
        if(res > ant) decresce = 0;
        ant = res;
    }
    printf("\n");
    ok("no limite, o resíduo decresce com P e acompanha a cauda prevista — não é falha",
       decresce);
    printf("      E é este o desenrolar, no sentido do corpo diferencial: o caractere leva ⊕ em\n");
    printf("      ⊗, Π(a+b) = Π(a)·Π(b). O que aqui se soma é o grupo ADITIVO dos expoentes, e\n");
    printf("      o que sai é um produto sobre os geradores MULTIPLICATIVOS. Os primos são a\n");
    printf("      base ortonormal deste corpo: cada inteiro escreve-se numa combinação e numa\n");
    printf("      só, que é o teorema fundamental da aritmética a fazer de δ_ij.\n");
}

printf("\n§Z5  E nos ZEROS: são simples, logo a zeta NÃO degenera lá.\n\n");
{
    /* A pergunta do Aarao inclui a raiz DUPLA, que e onde ε² = 0. Um zero duplo teria
     * ζ(ρ) = 0 E ζ'(ρ) = 0. Mede-se nos primeiros dez. */
    printf("      um zero DUPLO seria ζ(ρ) = 0 e ζ'(ρ) = 0 ao mesmo tempo.\n\n");
    printf("      γ            |ζ(1/2+iγ)|    |ζ'(1/2+iγ)|   simples?\n");
    int mal = 0, simples = 0;
    for(int k = 0; k < NZ; k++){
        double complex r = 0.5 + I*gamas[k];
        double z = cabs(zeta(r)), d = cabs(dzeta(r, 0.05, 64));
        printf("      %-12.6f %-14.2e %-14.6f %s\n", gamas[k], z, d,
               (z < 1e-8 && d > 1e-3) ? "sim" : "?");
        if(z > 1e-8) mal++;                    /* é mesmo zero? */
        if(d > 1e-3) simples++;                /* e a derivada não anula */
    }
    printf("\n");
    ok("os dez primeiros são mesmo zeros (|ζ| < 1e-8) e todos SIMPLES (|ζ'| > 1e-3)",
       mal == 0 && simples == NZ);
    printf("      Então a resposta à terceira parte é NÃO, e é um não informativo: nos zeros a\n");
    printf("      zeta não degenera. Δ != 0 lá — a raiz é uma, não uma dobrada. Se algum zero\n");
    printf("      fosse duplo, ESSE ponto seria do tipo ε² = 0, e é exatamente por isso que a\n");
    printf("      questão da simplicidade dos zeros é uma questão a sério e não um detalhe.\n");
    printf("\n      (E note-se que isto é medida em DEZ pontos. Que todos os zeros sejam simples\n");
    printf("       é conjectura em aberto, como a própria hipótese. Dez não é todos.)\n");
}

printf("\n§Z6  Onde ela degenera, então? Onde ζ' anula.\n\n");
{
    /* Ha um zero de ζ' no eixo real negativo, entre -3 e -2 (o minimo de ζ ali). E ha o
     * ponto s = 1, que e POLO — o unico. Mede-se os dois, que sao os pontos especiais reais. */
    printf("      procurando ζ'(σ) = 0 no eixo real, entre -3 e -2 (bissecção):\n\n");
    double a = -3.0, b = -2.0;
    double fa = creal(dzeta(a, 0.02, 64));
    for(int it = 0; it < 60; it++){
        double m = (a+b)/2, fm = creal(dzeta(m, 0.02, 64));
        if((fa < 0) == (fm < 0)){ a = m; fa = fm; } else b = m;
    }
    double raiz = (a+b)/2;
    printf("      ζ' anula em σ = %.9f, onde ζ = %.9f\n", raiz, creal(zeta(raiz)));
    printf("      (é o mínimo local de ζ entre os zeros triviais -2 e -4)\n\n");
    ok("há um ponto real onde ζ' anula, e ζ não anula lá — extremo, não raiz dupla",
       raiz > -3.0 && raiz < -2.0 && fabs(creal(zeta(raiz))) > 1e-3);
    printf("      Este é o ponto onde a zeta \"para\" — mas repare-se: ζ' = 0 com ζ != 0 é um\n");
    printf("      EXTREMO, não uma raiz dupla. A degenerescência ε² = 0 exigiria as duas a\n");
    printf("      anular juntas. Ou seja: nos pontos que o Aarão apontou, a zeta desenrola pela\n");
    printf("      DOBRA (§Z1-Z3) e pelo PRODUTO (§Z4), mas não degenera (§Z5-Z6).\n");
}

printf("\n§Z7  O que se mediu, e o que não se mediu.\n\n");
{
    printf("      MEDIDO, com resíduo:\n");
    printf("        · s -> 1-conj(s) tem ordem 2, e o seu conjunto fixo é a RETA Re(s) = 1/2.\n");
    printf("          A reta crítica é o vinco da dobra. Exato, algébrico, 41 de 41 pontos.\n");
    printf("        · ξ(s) = ξ(1-s) em 7 pontos, resíduo < 1e-8. A folha dobrada guarda a\n");
    printf("          folha inteira: meia zeta determina a outra metade.\n");
    printf("        · Σ n^{-s} = Π (1-p^{-s})^{-1} em 5 valores. O desenrolar Σ -> Π é o\n");
    printf("          Pontryagin do corpo diferencial, e os primos fazem de base.\n");
    printf("        · os dez primeiros zeros são simples: |ζ| < 1e-8 e |ζ'| > 1e-3. Lá ela\n");
    printf("          NÃO degenera.\n\n");
    printf("      NÃO MEDIDO — e não é por falta de tempo, é o problema em aberto:\n");
    printf("        · que TODOS os zeros estejam no vinco. Isso é a hipótese de Riemann, e\n");
    printf("          nada aqui a toca. Dez zeros medidos não são um argumento; o que está\n");
    printf("          verificado na literatura são ~10^13 primeiros, e continua a não ser\n");
    printf("          argumento — a diferença entre \"muitos\" e \"todos\" é o problema inteiro.\n");
    printf("        · que todos os zeros sejam simples. Também conjectura.\n\n");
    /* Uma asserção aqui tem de MEDIR — escrevi ok(...,1) e é a terceira vez nesta série.
     * O que se pode medir é local e é verdade: nos dez zeros conhecidos, o mínimo de |ζ| ao
     * longo da horizontal está MESMO em σ = 1/2. Isso é medida; que valha para todos não é. */
    printf("      E a medida que fecha, que é local e é honesta: nos dez, o mínimo de |ζ|\n");
    printf("      ao longo da horizontal cai mesmo no vinco?\n\n");
    {
        int mal = 0;
        printf("      γ            σ do mínimo de |ζ(σ+iγ)|   é 1/2?\n");
        for(int k = 0; k < NZ; k++){
            double melhor = 1e300, sbest = 0;
            for(int i = 0; i <= 200; i++){
                double sig = 0.05 + i*0.9/200.0;
                double v = cabs(zeta(sig + I*gamas[k]));
                if(v < melhor){ melhor = v; sbest = sig; }
            }
            printf("      %-12.6f %-26.6f %s\n", gamas[k], sbest,
                   fabs(sbest - 0.5) < 0.01 ? "sim" : "NÃO");
            if(fabs(sbest - 0.5) >= 0.01) mal++;
        }
        printf("\n");
        ok("nos dez zeros, o mínimo de |ζ| na horizontal cai no vinco σ = 1/2", mal == 0);
        printf("      Dez de dez. E dez não é todos — é o que separa esta linha da hipótese, e\n");
        printf("      essa distância não encolhe por eu medir mais dez.\n\n");
    }
    printf("      A leitura honesta é esta: a estrutura que o Aarão apontou ESTÁ lá, e está no\n");
    printf("      sítio certo — a dobra é ordem 2, o vinco é a reta crítica, e o desenrolar\n");
    printf("      Σ -> Π é o mesmo caractere do corpo diferencial. Isso é uma correspondência\n");
    printf("      exata de ESTRUTURA. Mas nenhuma correspondência de estrutura decide onde os\n");
    printf("      zeros caem, e dizer o contrário seria trocar uma coisa medida por uma que\n");
    printf("      não está.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
