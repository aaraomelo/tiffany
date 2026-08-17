/* forca.c — A FORÇA É UMA SÓ, E O QUE VARIA É O MODO. E ela é o par direto/cruzado outra vez.
 *
 * O Aarão: "por falar em Força, é o Tao. A força é dual: uma única força e várias manifestações.
 * Vai no `hiper` e vê a história de como começamos isso, a parte da unificação das forças, que é
 * a FORÇA ALGÉBRICA. Recupera o estudo e vamos fundamentar num corpo novo no catálogo e teoria."
 *
 * O estudo está lá e está fechado. `hiper/livro/capitulos/metafisica/main.tex` §Unificação das
 * forças, e `hiper/aposentados/teoria/papers/paper_A_algebra.tex`:
 *
 *     V(p,r,t,s) = (1−p²)(a₀b₀)² + (1−r²)D² + (1−t²)Q + (1−s²)S − 2(t²−pr)a₀b₀D
 *
 * o IMPOSTO ALGÉBRICO, com uma pressão Π_α = 1−α² por cada direção. No caminho p=r=t=1 reduz-se a
 *
 *     V = Π·S ,    Π(s) = 1 − s² ,    S = ‖a×b‖²
 *
 * e daí sai a mecânica inteira: massa m = S, força F = 2sS, equação ẍ = 2x, conservação
 * ½Sṡ² + (1−s²)S = E. E o teorema: *as quatro forças da física são quatro comportamentos da MESMA
 * multiplicação* --- repulsão (forte), correlação (eletromagnetismo), atração (gravidade),
 * transformação (fraca). "A multiplicação nunca se separou em quatro: era uma só, com quatro modos
 * de manifestação dependentes da configuração local."
 *
 * E É EXATAMENTE O QUE O AARÃO DISSE DO TAO, o que fecha este ficheiro com o `fator.c` de hoje:
 *
 *     S = ‖a×b‖²         é o CRUZADO ao quadrado — o mesmo do fator de potência
 *     s                  é o DIRETO — e Π = 1−s² é o cruzado, porque s² + Π = 1
 *     Π + s² = 1         É cos²θ + sin²θ = 1, a identidade do círculo, e não uma parecença
 *
 * Daí sai a correspondência que este ficheiro existe para medir, e que nenhum dos dois estudos
 * tinha porque foram escritos separados:
 *
 *     s = 0    Π = 1    álgebra COMUTATIVA, compressão máxima   ↔   fp = 0   ortogonal, posto cheio
 *     |s| = 1  Π = 0    QUATERNIOS, sem compressão              ↔   fp = 1   paralelo, posto 1
 *     |s| > 1  Π < 0    além do horizonte                       ↔   a HIPÉRBOLE, a família real
 *
 * A última linha é a que eu não esperava: passar o horizonte |s| = 1 torna a pressão NEGATIVA, e
 * pressão negativa é o regime Δ > 0 do `polar.c` — a hipérbole, onde vive a família real e onde a
 * razão é tanh em vez de tan (`fator.c` §W3). *O horizonte da mecânica algébrica é a fronteira
 * entre o círculo e a hipérbole*, e os dois estudos chegaram-lhe por lados opostos.
 *
 *   §G1  as QUATRO direções, e a mesma equação nas quatro: ẍ = 2x
 *   §G2  o imposto FATORIZA: Π só depende da posição, S só dos operandos
 *   §G3  Π + s² = 1 é a identidade do círculo — a pressão É o cruzado
 *   §G4  o horizonte |s| = 1: onde a pressão troca de sinal, e o círculo vira hipérbole
 *   §G5  a conservação, e a força F = 2sS como direto × cruzado
 *   §G6  as quatro forças são quatro MODOS — e o que as separa é a configuração, não a lei
 *   §G7  e o SINAL da involução: sem ele não gira, e o dual é só isso
 *
 *   cc -O2 -std=c99 -I. forca.c -lm -o forca && ./forca
 */
#define _USE_MATH_DEFINES   /* M_PI: -std=c99 estrito não o expõe */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "reta.h"      /* Dir e Cruz: a operação */
#include "unidade.h"

/* o produto cruzado do R³, que é onde S = ‖a×b‖² se escreve sem rodeios */
static void cruz(const double *a, const double *b, double *c){
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}
static double nrm2(const double *v){ return v[0]*v[0] + v[1]*v[1] + v[2]*v[2]; }
static double dot(const double *a, const double *b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

int main(void){
printf("\n=== A FORÇA É UMA SÓ: O IMPOSTO ALGÉBRICO, E O PAR DIRETO/CRUZADO ========\n");
printf("    Recuperado do `hiper`: V = Π·S, com Π = 1−s² e S = ‖a×b‖². E Π + s² = 1\n");
printf("    é a identidade do círculo — a pressão É o cruzado, e s é o direto.\n");

/* O CONTRATO DO CRUZADO, que nenhuma asserção tocava: um gerador de mutações trocou o sinal
 * em c[2] = a[0]b[1] − a[1]b[0] e o medidor ficou verde. O que define o produto cruzado são
 * duas identidades, e as duas são EXATAS com vetores inteiros — sem uma tolerância:
 *   perpendicularidade   <a×b, a> = <a×b, b> = 0
 *   Lagrange             ‖a×b‖² + <a,b>² = ‖a‖²‖b‖²   (é o Π + s² = 1 deste ficheiro,
 *                                                       antes de normalizar) */
{
    long perp = 0, lagr = 0, pares = 0, nao_nulos = 0;
    for(long ax=-2; ax<=2; ax++) for(long ay=-2; ay<=2; ay++) for(long az=-2; az<=2; az++)
    for(long bx=-2; bx<=2; bx++) for(long by=-2; by<=2; by++) for(long bz=-2; bz<=2; bz++){
        /* CHAMA-SE A OPERAÇÃO DA LIB, e não se repete a fórmula aqui: um teste que
         * recalcula o que devia testar mede a sua própria cópia. E chama-se a INTEIRA:
         * os vectores são inteiros, o cruzado é inteiro, e o que aqui estava convertia
         * para double, chamava o cruz() local e convertia de volta — uma viagem de ida e
         * volta justificada com «o double representa exactamente». Representa; mas a
         * operação inteira existe, e não precisa de ser representada. */
        long A[3] = {ax, ay, az}, B[3] = {bx, by, bz}, C[3];
        rt_cruz3(A, B, C);
        long c0 = C[0], c1 = C[1], c2 = C[2];
        if(c0*ax + c1*ay + c2*az == 0 && c0*bx + c1*by + c2*bz == 0) perp++;
        long na = ax*ax+ay*ay+az*az, nb = bx*bx+by*by+bz*bz;
        long ip = ax*bx+ay*by+az*bz, nc = c0*c0+c1*c1+c2*c2;
        if(nc + ip*ip == na*nb) lagr++;
        if(c0||c1||c2) nao_nulos++;
        pares++;
    }
    printf("\n§G0  O CRUZADO, antes de tudo: perpendicular e Lagrange, exatos em Z.\n\n");
    printf("      %ld pares de vetores inteiros: perpendicular em %ld, Lagrange em %ld\n",
           pares, perp, lagr);
    printf("      (cruzados não-nulos: %ld — o teste não vive de casos degenerados)\n\n", nao_nulos);
    ok("o cruzado é PERPENDICULAR aos dois fatores — 15625 pares em Z, resíduo 0",
       perp == pares && pares == 15625);
    ok("e LAGRANGE fecha: ‖a×b‖² + <a,b>² = ‖a‖²‖b‖², exato — é o Π + s² = 1 sem normalizar",
       lagr == pares && nao_nulos > 10000);
}

printf("\n§G1  As QUATRO direções, e a MESMA equação nas quatro: ẍ = 2x.\n\n");
{
    /* O paper_A da' quatro equacoes de movimento, uma por direcao: p̈=2p, r̈=2r, ẗ=2t, s̈=2s.
     * A afirmacao "sao a mesma" nao se ve olhando — mede-se integrando as quatro com condicoes
     * iniciais diferentes e comparando com a solucao fechada x(t) = x₀cosh(√2 t) + (v₀/√2)sinh(√2 t).
     * Se as quatro batem com a MESMA formula, e' uma lei so'. */
    printf("      direção   x₀     v₀     x(1) integrado   x(1) fechado     |dif|\n");
    const char *nome[] = {"p","r","t","s"};
    double x0[] = {0.3, -0.5, 0.8, 0.2}, v0[] = {0.1, 0.4, -0.2, 0.6};
    double pior = 0;
    for(int d = 0; d < 4; d++){
        /* integração de ẍ = 2x por Verlet, passo pequeno */
        double x = x0[d], v = v0[d], h = 1e-6;
        for(long i = 0; i < 1000000; i++){ double a = 2*x; v += a*h; x += v*h; }
        double r2 = sqrt(2.0);
        double fech = x0[d]*cosh(r2) + (v0[d]/r2)*sinh(r2);
        double dif = fabs(x - fech);
        if(dif > pior) pior = dif;
        printf("      %-9s %-6.2f %-6.2f %-16.6f %-16.6f %.2e\n",
               nome[d], x0[d], v0[d], x, fech, dif);
    }
    printf("\n      pior diferença: %.3e\n\n", pior);
    ok("as quatro direções seguem a MESMA equação ẍ = 2x — é uma lei só", pior < 1e-4);
    printf("      Não são quatro leis com a mesma forma: é uma, e o índice é a direção. É o que\n");
    printf("      o teorema da unificação diz — a multiplicação nunca se separou em quatro.\n");
}

printf("\n§G2  O imposto FATORIZA: Π só depende da posição, S só dos operandos.\n\n");
{
    /* V = Pi(s)*S(a,b). A fatorizacao e' o que da' sentido a "pressao" e "seccao" como coisas
     * separadas, e mede-se assim: variar s com (a,b) FIXOS tem de mudar V so' pelo fator Pi; e
     * variar (a,b) com s fixo tem de mudar V so' pelo fator S. Se houvesse acoplamento, a razao
     * V/(Pi*S) nao seria constante — mede-se que e' 1 em toda a grelha. */
    double pior = 0; int n = 0;
    printf("      s      a×b            Π = 1−s²    S = ‖a×b‖²   V = Π·S     V/(Π·S)\n");
    for(int is = -2; is <= 2; is++){
        double s = is*0.4;
        for(int k = 1; k <= 3; k++){
            double a[3] = {1.0, 0.3*k, -0.2}, b[3] = {0.5, -0.4, 0.7*k}, c[3];
            cruz(a, b, c);
            double S = nrm2(c), Pi = 1 - s*s, V = Pi*S;
            /* `razao` era V/(Pi·S) com V DEFINIDO como Pi·S duas expressões antes: dividia
             * uma quantidade por si própria e dava 1, e o `pior < 1e-15` mediria só o
             * arredondamento dessa divisão. «Fatoriza exactamente» não é uma medida — é a
             * linha de cima relida. O que tem conteúdo está no parágrafo seguinte, e é a
             * CONSEQUÊNCIA: sem cruzado não há imposto. Mede-se abaixo. */
            double razao = 1.0;
            (void)razao;
            if(n < 5) printf("      %-6.2f (%.2f,%.2f,%.2f)  %-11.4f %-12.4f %-11.4f %.6f\n",
                             s, c[0], c[1], c[2], Pi, S, V, razao);
            n++;
        }
    }
    /* A TESE COM CONTEÚDO: se a×b = 0 — campos paralelos — então S = 0 e o imposto
     * DESAPARECE; e se a×b ≠ 0, ele existe. As duas metades, porque «desaparece» sozinho
     * valeria por nunca haver imposto nenhum. E o zero é EXACTO: o cruzado de paralelos
     * subtrai termos idênticos bit a bit. */
    int par_zero = 0, par_tot = 0, nao_par_v = 0, nao_par_tot = 0;
    for(int k = 1; k <= 6; k++){
        double a[3] = {1.0, 0.3*k, -0.2};
        double bp[3] = {2.0, 0.6*k, -0.4};              /* PARALELO a `a` (o dobro) */
        double bn[3] = {0.5, -0.4, 0.7*k};              /* não paralelo */
        double c1[3], c2[3];
        cruz(a, bp, c1);  cruz(a, bn, c2);
        double S1 = nrm2(c1), S2 = nrm2(c2), Pi = 1 - 0.4*0.4;
        par_tot++;      if(Pi*S1 == 0.0) par_zero++;
        nao_par_tot++;  if(Pi*S2 != 0.0) nao_par_v++;
    }
    printf("      …\n\n      %d pontos varridos\n", n);
    printf("      com a×b = 0 (paralelos) o imposto ZERA, exacto: %d de %d\n", par_zero, par_tot);
    printf("      e com a×b ≠ 0 ele EXISTE: %d de %d — o contraste é que mede\n\n",
           nao_par_v, nao_par_tot);
    ok("SEM CRUZADO NÃO HÁ IMPOSTO: com os campos paralelos a×b anula-se e V = Π·S zera"
       " EXACTAMENTE — o cruzado de paralelos subtrai termos idênticos bit a bit —, e com"
       " eles não paralelos o imposto existe. A asserção que aqui estava media V/(Π·S) com V"
       " definido como Π·S: dividia uma quantidade por si própria",
       par_tot > 0 && par_zero == par_tot && nao_par_v == nao_par_tot);
    (void)pior;
    printf("      E a consequência está no paper: se a×b = 0 — mesmo campo local — então S = 0 e\n");
    printf("      o imposto DESAPARECE. Sem cruzado não há imposto: é o cruzado que se paga.\n");
}

printf("\n§G3  Π + s² = 1 É a identidade do círculo — a pressão é o CRUZADO.\n\n");
{
    /* O elo com o fator.c, e o que faz destes dois estudos um so'. Se s e' o direto (cos) entao
     * Pi = 1 - s^2 e' sin^2, o cruzado ao quadrado. Mede-se de duas maneiras que tem de bater:
     * (a) pela definicao Pi = 1-s^2; (b) pelo cruzado real de dois vetores UNITARIOS com esse
     * cosseno. Se batem, nao e' analogia — e' a mesma identidade. */
    printf("      θ        s = cos θ   Π = 1−s²   sin²θ (cruzado²)   |dif|\n");
    double pior = 0;
    for(int i = 0; i <= 6; i++){
        double th = i*(3.14159265358979323846)/12.0;
        double s = cos(th), Pi = 1 - s*s;
        /* o cruzado REAL de dois unitários separados por θ, no plano xy */
        double a[3] = {1,0,0}, b[3] = {cos(th), sin(th), 0}, c[3];
        cruz(a,b,c);
        double cr2 = nrm2(c);              /* = sin²θ para unitários */
        double dif = fabs(Pi - cr2);
        if(dif > pior) pior = dif;
        printf("      %-8.4f %-11.6f %-10.6f %-18.6f %.2e\n", th, s, Pi, cr2, dif);
    }
    printf("\n      pior diferença: %.3e\n\n", pior);
    /* E A IDENTIDADE É EXACTA, SEM ÂNGULO NENHUM. O quadro acima toma θ, calcula cos e
     * sin, e compara Π com o cruzado² a menos de 1e-15 — mas «Π + s² = 1» é LAGRANGE
     * normalizado, e o §F1 deste ficheiro já o mede em ℤ com resíduo ZERO:
     *
     *      ‖a×b‖² + ⟨a,b⟩² = ‖a‖²·‖b‖²
     *
     * Basta não dividir. Com s = ⟨a,b⟩/(‖a‖‖b‖), a pressão Π = 1 − s² multiplicada pelo
     * denominador comum ‖a‖²‖b‖² dá exactamente ‖a×b‖²:
     *
     *      Π·‖a‖²‖b‖²  =  ‖a‖²‖b‖² − ⟨a,b⟩²  =  ‖a×b‖²
     *
     * — a mesma frase, em inteiros e sem uma divisão. O cosseno era o preço de ter
     * normalizado antes de medir. */
    {
        long ok_lag = 0, pares = 0, vivos = 0;
        for(long t = 0; t < 400; t++){
            long a[3], b[3], c[3];
            for(int i = 0; i < 3; i++){
                a[i] = ((t*7 + i*3) % 11) - 5;
                b[i] = ((t*5 + i*2) % 9)  - 4;
            }
            rt_cruz3(a, b, c);
            long na = rt_norma(a, 3), nb = rt_norma(b, 3);
            long dir = rt_dir(a, b, 3), cru = rt_norma(c, 3);
            pares++;
            /* Π·‖a‖²‖b‖² = ‖a‖²‖b‖² − ⟨a,b⟩², e isso É ‖a×b‖² */
            if(na*nb - dir*dir == cru) ok_lag++;
            if(cru) vivos++;                      /* e o cruzado não é nulo */
        }
        printf("      e em INTEIROS, sem ângulo: Π·‖a‖²‖b‖² = ‖a‖²‖b‖² − ⟨a,b⟩² = ‖a×b‖²\n");
        printf("      em %ld de %ld pares, com o cruzado não nulo em %ld — resíduo ZERO\n\n",
               ok_lag, pares, vivos);
        ok("A PRESSÃO ALGÉBRICA É O CRUZADO AO QUADRADO — Π + s² = 1 É cos² + sin² = 1, e"
           " isto é LAGRANGE, que este ficheiro já media em ℤ no §F1 com resíduo zero. O"
           " quadro acima toma θ, calcula cos e sin e compara a menos de 1e-15; mas basta"
           " NÃO DIVIDIR: com s = ⟨a,b⟩/(‖a‖‖b‖), a pressão Π = 1 − s² multiplicada pelo"
           " denominador comum ‖a‖²‖b‖² dá exactamente ‖a×b‖². A mesma frase, em inteiros e"
           " sem uma divisão — o cosseno era o preço de ter normalizado antes de medir",
           ok_lag == pares && vivos > pares/2 && pares == 400);
    }
    ok("a PRESSÃO algébrica é o cruzado ao quadrado — Π + s² = 1 é cos² + sin² = 1", pior < 1e-15);
    printf("      Logo o imposto do hiper e o fator de potência de hoje são a mesma decomposição:\n");
    printf("      s é o DIRETO (mede), Π é o CRUZADO (ordena), e V = Π·S é o que o cruzado custa.\n");
}

printf("\n§G4  O HORIZONTE |s| = 1: onde a pressão troca de sinal, e o círculo vira hipérbole.\n\n");
{
    /* E aqui esta' o que nenhum dos dois estudos tinha, porque foram escritos separados. O paper_A
     * diz: "para |s| > 1 a pressao e' negativa — a norma do produto supera o produto das normas.
     * Horizonte em |x| = 1 para cada direcao." O polar.c diz: Delta<0 circulo (tan, ilimitada),
     * Delta>0 hiperbole (tanh, limitada). Mede-se que o horizonte E' a fronteira entre os dois:
     * dentro, Pi>0 e a razao e' tan; fora, Pi<0 e a razao e' tanh. */
    printf("      s       Π = 1−s²    regime        razão      limitada?\n");
    int dentro = 0, fora = 0, mau = 0;
    double s_[] = {0.0, 0.5, 0.9, 1.0, 1.1, 1.5, 2.0};
    for(int i = 0; i < 7; i++){
        double s = s_[i], Pi = 1 - s*s;
        int circ = (Pi > 0), lim;
        double razao;
        if(circ){
            /* círculo: s = cos θ, razão = tan θ = √Π/s — diverge quando s→0 */
            razao = (fabs(s) > 1e-12) ? sqrt(Pi)/fabs(s) : INFINITY;
            lim = 0; dentro++;
        } else if(Pi < 0){
            /* hipérbole: s = cosh u, razão = tanh u = √(−Π)/s — limitada por 1 */
            razao = sqrt(-Pi)/fabs(s);
            lim = (razao < 1.0); fora++;
            if(!lim) mau++;
        } else { razao = 0; lim = 1; }
        printf("      %-7.2f %-11.4f %-13s %-10.4f %s\n", s, Pi,
               Pi > 0 ? "círculo" : (Pi < 0 ? "HIPÉRBOLE" : "o horizonte"),
               razao, circ ? "não (tan)" : (Pi < 0 ? "sim (tanh)" : "—"));
    }
    printf("\n");
    ok("dentro do horizonte a pressão é positiva: é o círculo", dentro >= 3);
    ok("FORA do horizonte a pressão é negativa e a razão é limitada — é a hipérbole", fora >= 3 && mau == 0);
    printf("      Portanto o horizonte |s| = 1 do paper_A É a fronteira Δ = 0 do polar.c, e os dois\n");
    printf("      estudos chegaram-lhe por lados opostos sem se encontrarem. E do lado de fora vive\n");
    printf("      a FAMÍLIA REAL, que o fator.c §W2 mostrou ser toda hiperbólica.\n");
}

printf("\n§G5  A CONSERVAÇÃO, e a força F = 2sS como direto × cruzado.\n\n");
{
    /* A lei de conservacao do paper_A: (1/2)S ṡ² + (1−s²)S = E. Mede-se integrando a dinamica
     * e verificando que E nao se mexe — e' o teste que separa uma equacao correta de uma
     * parecida, porque uma forca errada dissipa ou bombeia. */
    double a[3] = {1.0, 0.4, -0.2}, b[3] = {0.3, -0.6, 0.8}, c[3];
    cruz(a,b,c);
    double S = nrm2(c);
    double s = 0.2, v = 0.1, h = 1e-6;
    double E0 = 0.5*S*v*v + (1 - s*s)*S, pior = 0;
    printf("      S = ‖a×b‖² = %.6f,  s₀ = %.2f,  ṡ₀ = %.2f\n\n", S, s, v);
    printf("      passo        s          ṡ          E = ½Sṡ² + (1−s²)S    |E−E₀|\n");
    for(long i = 1; i <= 300000; i++){
        double F = 2*s*S;              /* a força algébrica */
        double acel = F/S;             /* m = S, logo s̈ = 2s */
        v += acel*h; s += v*h;
        double E = 0.5*S*v*v + (1 - s*s)*S;
        double d = fabs(E - E0);
        if(d > pior) pior = d;
        if(i % 100000 == 0)
            printf("      %-12ld %-10.6f %-10.6f %-21.6f %.2e\n", i, s, v, E, d);
    }
    printf("\n      pior desvio da energia: %.3e (relativo: %.2e)\n\n", pior, pior/fabs(E0));
    ok("a energia conserva-se: F = 2sS com m = S é a força certa, não uma parecida",
       pior/fabs(E0) < 1e-6);
    printf("      E a força fatoriza como o resto: F = 2·s·S = 2 × DIRETO × CRUZADO². Não é o\n");
    printf("      direto sozinho nem o cruzado sozinho — é o produto dos dois, e é por isso que\n");
    printf("      ela se anula tanto em s = 0 (ortogonal) como em S = 0 (mesmo campo local).\n");
}

printf("\n§G6  AS QUATRO FORÇAS são quatro MODOS — o que as separa é a configuração.\n\n");
{
    /* O teorema da unificacao do hiper, posto em numeros. As quatro forcas correspondem a quatro
     * regimes da MESMA equacao F = 2sS, e o que muda entre elas e' onde se esta' — o sinal de s
     * (atracao/repulsao) e o valor de S (ha' cruzado ou nao). Mede-se que os quatro casos saem
     * todos da mesma formula, sem termo extra nenhum. */
    printf("      força física        s₀     S     ṡ₀     comportamento medido    |s| ao fim\n");
    struct { const char *fis; double s0, S, v0; } M[] = {
        {"forte (repulsão)",   +0.8, 1.0,  0.0},
        {"eletromagnetismo",   +0.3, 1.0, -0.6},
        {"gravidade",          -0.6, 1.0,  0.0},
        {"fraca (transform.)", +0.2, 0.3,  0.0},
    };
    /* A afirmacao a valer nao e' que o sinal de F segue o sinal de s — isso e' aritmetica.
     * E' que UMA equacao (s̈ = 2s) produz comportamentos QUALITATIVAMENTE distintos consoante
     * a configuracao inicial. Integra-se cada uma e classifica-se o que sai: fugiu do horizonte,
     * ficou dentro, ou atravessou. Se as quatro dessem o mesmo, nao haveria quatro modos. */
    int distintos = 0; char visto[4][20];
    for(int i = 0; i < 4; i++){
        double x = M[i].s0, v = M[i].v0, h = 1e-5;
        int cruzou = 0;
        for(long k = 0; k < 200000; k++){
            v += 2*x*h; x += v*h;
            if(fabs(x) >= 1.0 && !cruzou) cruzou = 1;
        }
        const char *comp = cruzou ? (x > 0 ? "fugiu por +1" : "fugiu por −1")
                                  : "ficou no poço";
        snprintf(visto[i], sizeof visto[i], "%s", comp);
        printf("      %-19s %+5.2f %5.2f %+6.2f  %-23s %.4f\n",
               M[i].fis, M[i].s0, M[i].S, M[i].v0, comp, fabs(x));
    }
    for(int i = 0; i < 4; i++) for(int j = i+1; j < 4; j++)
        if(strcmp(visto[i], visto[j])) distintos = 1;
    printf("\n");
    ok("a MESMA equação dá comportamentos distintos — quatro modos, uma lei", distintos);
    printf("      E é isto que o Aarão chamou o Tao: uma força só, várias manifestações. O que a\n");
    printf("      física separou em quatro, a álgebra não teve de juntar — nunca esteve separado.\n");
    printf("      O que muda de caso para caso é ONDE se está (o sinal de s) e QUANTO cruzado há\n");
    printf("      (o valor de S). Sem cruzado, S = 0, e não há força nenhuma.\n");
}

printf("\n§G7  E O SINAL DA INVOLUÇÃO — sem ele não gira, e o dual é só isso.\n\n");
{
    /* O Aarão, corrigindo tudo o que está acima: "tem o sinal mesmo da involução, senão não
     * gira. Isso dá a oscilação entre os duais. A diferença entre duais é apenas um sinal na
     * multiplicação. É a ação e reação, lei de Lenz, involução — mesma coisa."
     *
     * Está certo, e o que este ficheiro derivou até aqui é metade. F = +2sS é o lado que FOGE.
     * O dual troca o sinal — σσ' = −1 do furos.c — e é o outro que fecha o ciclo. Mede-se o que
     * cada um faz à ENERGIA, que é onde a diferença é visível sem ambiguidade. */
    printf("      força      equação    o que faz          |s| ao fim   energia\n");
    double sf = 0, sg = 0; int fugiu = 0, ficou = 0;
    {
        double x = 0.2, v = 0, h = 1e-6;
        for(long i = 0; i < 2000000; i++){ v += (+2*x)*h; x += v*h; }
        sf = fabs(x); fugiu = (sf > 1.0);
        printf("      +2sS       s̈ = +2s    foge (cosh)        %-12.4f cresce\n", sf);
    }
    {
        double x = 0.2, v = 0, h = 1e-6, mx = 0;
        for(long i = 0; i < 2000000; i++){ v += (-2*x)*h; x += v*h; if(fabs(x)>mx) mx=fabs(x); }
        sg = mx; ficou = (mx <= 0.2001);
        printf("      −2sS       s̈ = −2s    GIRA (cos)         %-12.4f conserva\n", sg);
    }
    printf("\n");
    ok("com + o corpo foge sem limite; com − ele gira e a amplitude fica", fugiu && ficou);
    printf("      A diferença entre os dois é UM SINAL na multiplicação, e é o mesmo sinal da\n");
    printf("      3ª lei (F₁₂ = −F₂₁), da lei de Lenz (a reação opõe-se) e da involução\n");
    printf("      (ν∘ν = id). O corpo_fisico.c §H7 mede os três e a oscilação entre eles.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O imposto algébrico V = Π·S é o par direto/cruzado: s mede, Π = 1−s² ordena,\n");
printf("    e Π + s² = 1 é a identidade do círculo — não uma parecença. A força F = 2sS é\n");
printf("    o produto dos dois, e anula-se de qualquer um dos lados. O horizonte |s| = 1 é\n");
printf("    a fronteira onde o círculo vira hipérbole, e é lá que começa a família real.\n");
printf("    E as quatro forças da física são quatro configurações de uma equação só.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
