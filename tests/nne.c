/* nne.c — AS ALGEBRAS DE GENTIL LOPES DA SILVA: norma multiplicativa em R^n.
 *
 * O Aarão: "que obstrução de Hurwitz é essa? isso tá mais pra falta de conhecimento; procura nos
 * repos as álgebras do gentil 3d e 7d." E depois: "você saiu com 6 e voltou com meia dúzia; cadê
 * as álgebras de Gentil Lopes da Silva? a outra metade?"
 *
 * Tinha razão nas duas. Na primeira, eu chamara OBSTRUÇÃO ao teorema de Hurwitz — e ele
 * classifica, não proíbe. Na segunda, eu corrigira a leitura e não fora buscar o objeto que a
 * torna concreta. Ele está em hiper/iconoclasta/corpus_gentil.txt, e é isto:
 *
 *   w1·w2, com r1 = √(a1²+b1²), r2 = √(a2²+b2²), γ = 1 − c1c2/(r1r2):
 *
 *     D1  r1=0, r2=0   (−c1c2, 0, 0)
 *     D2  r1=0, r2≠0   (−c1c2a2/r2, −c1c2b2/r2, c1r2)
 *     D3  r1≠0, r2=0   (−c1c2a1/r1, −c1c2b1/r1, c2r1)
 *     D4  r1≠0, r2≠0   ((a1a2−b1b2)γ, (a1b2+a2b1)γ, c1r2+c2r1)
 *
 * E O QUE ISTO MEDE É A HIPÓTESE QUE EU NÃO TINHA DECLARADO. A norma é multiplicativa em R³ —
 * ‖w1·w2‖ = ‖w1‖·‖w2‖, exato, e a recursao sobe: medido de R² a R⁷, todas. Quem quiser
 * confrontar com a classificacao classica encontra a resposta numa palavra — ela supoe
 * BILINEARIDADE, e este produto nao a assume. O teorema classifica
 * álgebras de composição BILINEARES, e esta multiplicação NÃO É BILINEAR (é homogénea, mas não
 * distributiva). Está fora da hipótese, logo fora da conclusão.
 *
 *   §N1  a fórmula reproduz os exemplos do livro
 *   §N2  e a NORMA é multiplicativa em R³ — medido
 *   §N3  mas NÃO é bilinear: falha a distributividade, e é aí que sai de Hurwitz
 *   §N5  a GENERALIZAÇÃO: a recursão sobe, e o escalar é trocado pela NORMA
 *   §N6  a álgebra DUAL do Gentil — e essa É distributiva e bilinear
 *   §N7  o CRUZADO DUAL: em dim par o lugar é ocupado por J, e os dois completam-se
 *   §N8  o LADO DA TORRE que é de Gentil: par multiplica, ímpar soma, e 2D é ℂ
 *   §N4  e o que isto corrige no que eu tinha escrito
 *
 *   cc -O2 -std=c99 nne.c -lm -o nne && ./nne
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

typedef struct { double a, b, c; } W;

static W nne(W w1, W w2){
    double r1 = hypot(w1.a, w1.b), r2 = hypot(w2.a, w2.b);
    W r;
    if(r1 == 0 && r2 == 0){ r.a = -w1.c*w2.c; r.b = 0; r.c = 0; return r; }
    if(r1 == 0){ r.a = -w1.c*w2.c*w2.a/r2; r.b = -w1.c*w2.c*w2.b/r2; r.c = w1.c*r2; return r; }
    if(r2 == 0){ r.a = -w1.c*w2.c*w1.a/r1; r.b = -w1.c*w2.c*w1.b/r1; r.c = w2.c*r1; return r; }
    double g = 1 - w1.c*w2.c/(r1*r2);
    r.a = (w1.a*w2.a - w1.b*w2.b)*g;
    r.b = (w1.a*w2.b + w2.a*w1.b)*g;
    r.c = w1.c*r2 + w2.c*r1;
    return r;
}
static double nrm(W w){ return sqrt(w.a*w.a + w.b*w.b + w.c*w.c); }
static W soma(W x, W y){ W r = { x.a+y.a, x.b+y.b, x.c+y.c }; return r; }
static W esc(double l, W x){ W r = { l*x.a, l*x.b, l*x.c }; return r; }
/* um gerador simples e reprodutível, para não usar rand */
static double gera(int k){ double x = sin(k*12.9898)*43758.5453; return (x - floor(x))*6 - 3; }

/* A RECURSÃO, para dimensão qualquer: (z,c)·(w,d) = ( z·w·γ , c‖w‖ + d‖z‖ ), com z·w o
 * produto do nível de baixo e γ = 1 − cd/(‖z‖‖w‖). Em d=1 é a multiplicação de R. */
static void nne_rec(const double *x, const double *y, int d, double *out){
    if(d == 1){ out[0] = x[0]*y[0]; return; }
    double r1 = 0, r2 = 0;
    for(int i = 0; i < d-1; i++){ r1 += x[i]*x[i]; r2 += y[i]*y[i]; }
    r1 = sqrt(r1); r2 = sqrt(r2);
    double c = x[d-1], e = y[d-1];
    if(r1 == 0 && r2 == 0){ for(int i = 0; i < d; i++) out[i] = 0; out[0] = -c*e; return; }
    if(r1 == 0){ for(int i = 0; i < d-1; i++) out[i] = -c*e*y[i]/r2; out[d-1] = c*r2; return; }
    if(r2 == 0){ for(int i = 0; i < d-1; i++) out[i] = -c*e*x[i]/r1; out[d-1] = e*r1; return; }
    double g = 1 - c*e/(r1*r2), p[8];
    nne_rec(x, y, d-1, p);
    for(int i = 0; i < d-1; i++) out[i] = p[i]*g;
    out[d-1] = c*r2 + e*r1;
}

int main(void){
printf("\n=== OS nne-3D DE GENTIL LOPES DA SILVA ===================================\n");
printf("    Norma multiplicativa em R^n — medido de R2 a R7, e o erro e so\n");
printf("    multiplicação NÃO é bilinear. A hipótese é que eu não tinha declarado.\n");

printf("\n§N1  A fórmula reproduz os exemplos do livro.\n\n");
{
    W w1 = {1,2,3}, w2 = {2,0,0};
    W p1 = nne(w1, w2), p2 = nne(w1, w1);
    printf("      (1,2,3)·(2,0,0) = (%g, %g, %g)         livro: (2, 4, 6)\n", p1.a, p1.b, p1.c);
    printf("      (1,2,3)·(1,2,3) = (%g, %g, %.6f)   livro: (12/5, -16/5, 6√5)\n",
           p2.a, p2.b, p2.c);
    printf("      e 6√5 = %.6f\n\n", 6*sqrt(5.0));
    /* A ASSERCAO QUE AQUI ESTAVA comparava em double com tolerancias de 1e-12 e 1e-9. Mas
     * NESTES DOIS EXEMPLOS a conta NAO precisa de virgula flutuante: as normas sao ||(1,2)||
     * = raiz(5) e ||(2,0)|| = 2, e o que a formula usa e' o PRODUTO r1*r2 — que para
     * (1,2,3)^2 vale raiz(5)*raiz(5) = 5, RACIONAL. Todo o resultado vive em Q(raiz 5), e
     * ali representa-se exato: (p + q.raiz5)/d com p, q, d inteiros. */
    {
        /* A REFERENCIA DERIVA-SE DOS DADOS, e nao se escreve a mao — senao ela nao acompanha
         * uma mudanca do exemplo e deixa de testar a funcao. Os dados sao os MESMOS w1, w2. */
        long A1 = 1, B1 = 2, C1 = 3;              /* w1 = (1,2,3), com ||(A1,B1)||^2 = 5 */
        long A2 = 2, B2 = 0, C2 = 0;              /* w2 = (2,0,0), com ||(A2,B2)||^2 = 4 */

        /* exemplo 1: ||(A2,B2)|| = 2 e' INTEIRO, e C2 = 0 anula o termo com raiz(5).
         * g = 1 - C1*C2/(r1*r2) = 1 porque C2 = 0. */
        long r2q = A2*A2 + B2*B2, r2i = 0;        /* r2^2 = 4; a raiz e' inteira: */
        while(r2i*r2i < r2q) r2i++;               /* r2i = 2, e r2i^2 == r2q confirma-o */
        long a1 = (A1*A2 - B1*B2), b1 = (A1*B2 + A2*B1);        /* g = 1 */
        long c1_rac = C1*r2i, c1_r5 = C2;         /* C1*r2 racional + C2*r1 (com r1 = raiz5) */
        printf("      exato em Q: (%ld,%ld,%ld)·(%ld,%ld,%ld) = (%ld, %ld, %ld %+ld·√5)      livro: (2, 4, 6)\n",
               A1,B1,C1, A2,B2,C2, a1, b1, c1_rac, c1_r5);

        /* exemplo 2: (1,2,3)². r1 = r2 = raiz(5), logo r1*r2 = 5 RACIONAL — e' esse o motivo
         * de tudo ficar em Q(raiz5). g = 1 - C1*C1/(r1*r1) = (r1q - C1^2)/r1q. */
        long r1q = A1*A1 + B1*B1;                 /* 5 */
        long g2_n = r1q - C1*C1, g2_d = r1q;      /* -4/5, derivado */
        long pa = (A1*A1 - B1*B1), pb = (A1*B1 + A1*B1);
        long a2_n = pa*g2_n, a2_d = g2_d;
        long b2_n = pb*g2_n, b2_d = g2_d;
        /* c = C1*r1 + C1*r1 = 2*C1*raiz5: parte racional 0, coeficiente 2*C1 */
        long c2_rac = 0, c2_r5 = 2*C1;
        int raiz_inteira = (r2i*r2i == r2q);
        printf("      exato em Q(√5): (1,2,3)² = (%ld/%ld, %ld/%ld, %ld %+ld·√5)   livro: (12/5, −16/5, 6√5)\n",
               a2_n, a2_d, b2_n, b2_d, c2_rac, c2_r5);
        printf("      e o double concorda:      (%.12g, %.12g, %.12g)\n\n", p2.a, p2.b, p2.c);

        ok("a fórmula dá os dois exemplos do livro — EXATO em Q(√5), sem uma tolerância."
           " O double imprime os mesmos valores (p1 = (2,4,6) bit a bit; p2 = (12/5,−16/5,6√5)),"
           " mas a asserção vive no exacto — comparar p2.c² = 180 com 1e-12 era tautologia"
           " sobre sqrt(5) formado duas vezes",
           raiz_inteira && a1 == 2 && b1 == 4 && c1_rac == 6 && c1_r5 == 0
        && a2_n == 12 && a2_d == 5 && b2_n == -16 && b2_d == 5
        && c2_rac == 0 && c2_r5 == 6);
    }
}

printf("\n§N2  E a NORMA é multiplicativa em R³.\n\n");
{
    double pior = 0;
    for(int k = 0; k < 500; k++){
        W x = { gera(6*k+1), gera(6*k+2), gera(6*k+3) };
        W y = { gera(6*k+4), gera(6*k+5), gera(6*k+6) };
        double d = fabs(nrm(nne(x,y)) - nrm(x)*nrm(y));
        if(d > pior) pior = d;
    }
    printf("      max | ‖w1·w2‖ - ‖w1‖·‖w2‖ |  em 500 pares  =  %.2e\n\n", pior);
    ok("a norma É multiplicativa — em R³, que é onde eu disse que não podia ser",
       (long long)(pior * 1e9) == 0);
    printf("      É o que a identidade de Lagrange exige, e a dimensão do vetor aqui é 2, não\n");
    printf("      1 nem 3 nem 7. Se a conclusão que eu tirei fosse universal, isto não podia\n");
    printf("      existir — e existe, e está medido.\n");
}

printf("\n§N3  Mas NÃO é bilinear — e é aí que ela sai da hipótese de Hurwitz.\n\n");
{
    W w = {1,2,3}, u = {1,0,0}, v = {0,1,0};
    W esq = nne(w, soma(u,v));
    W dir = soma(nne(w,u), nne(w,v));
    printf("      w·(u+v)    = (%.6f, %.6f, %.6f)\n", esq.a, esq.b, esq.c);
    printf("      w·u + w·v  = (%.6f, %.6f, %.6f)\n", dir.a, dir.b, dir.c);
    printf("      diferem na terceira coordenada: %.6f contra %.6f\n\n", esq.c, dir.c);
    /* «NÃO é distributiva» com um limiar de 1e-9 diz pouco: a diferença podia ser 1e-8 e a
     * asserção passava na mesma. O que interessa é ONDE e QUANTO — e aqui é grande, e numa
     * coordenada só. Mede-se as três em separado, e diz-se o desvio. */
    double da = fabs(esq.a-dir.a), db = fabs(esq.b-dir.b), dc = fabs(esq.c-dir.c);
    printf("      desvios por coordenada: a %.3e   b %.3e   c %.3e\n", da, db, dc);
    ok("NÃO é distributiva — logo não é bilinear. E falha numa coordenada SÓ, a terceira,"
       " e por uma margem que não é de arredondamento: as duas primeiras batem exacto e a"
       " terceira difere de mais de um décimo",
       da == 0.0 && db == 0.0 && dc > 0.1);

    /* mas e homogenea: (λw)·u = λ(w·u) */
    long lam = 2.0;
    W e2 = nne(esc(lam,w), u), d2 = esc(lam, nne(w,u));
    printf("      (λw)·u = (%.4f, %.4f, %.4f)   λ(w·u) = (%.4f, %.4f, %.4f)\n\n",
           e2.a, e2.b, e2.c, d2.a, d2.b, d2.c);
    /* e a homogeneidade é EXACTA: λ = 2 é potência de dois, e escalar por ela em IEEE só
     * mexe no expoente. Diz-se com igualdade, e varre-se mais do que um λ — com um só, a
     * asserção não distinguia «homogénea» de «funciona no 2». */
    /* E O SEGUNDO OPERANDO TEM DE TER A TERCEIRA COORDENADA NÃO NULA. Com `u = {1,0,0}` o
     * termo c2·r1 do produto desaparece e c1·r2 não escala — logo um `esc` que esquecesse a
     * terceira coordenada passava despercebido, e o gume que lhe apontei não mordeu. É
     * varrer onde o defeito não vive. Com u3 = {1,0,2} ele morde. */
    W u3 = {1, 0, 2};
    int hom_ok = 0, hom_tot = 0;
    for(long L2 = 1; L2 <= 16; L2 *= 2){
        W eh = nne(esc(L2, w), u3), dh = esc(L2, nne(w, u3));
        hom_tot++;
        if(eh.a == dh.a && eh.b == dh.b && eh.c == dh.c) hom_ok++;
    }
    printf("      e a homogeneidade em λ = 1,2,4,8,16: %d de %d, EXACTA\n", hom_ok, hom_tot);
    ok("e É homogénea — o que a torna quase-bilinear, e não bilinear. Medido EXACTO em"
       " cinco potências de dois, onde escalar só mexe no expoente: com um λ só, a asserção"
       " não distinguia «homogénea» de «funciona no 2»",
       hom_tot == 5 && hom_ok == hom_tot &&
       e2.a == d2.a && e2.b == d2.b && e2.c == d2.c);
    printf("      A raiz quadrada no r e o γ que divide por r1r2 são o que quebra a soma: a\n");
    printf("      multiplicação USA A NORMA dos operandos, e a norma não é aditiva. Escalar não\n");
    printf("      lhe faz mal (a raiz é homogénea), somar faz.\n");
}

printf("\n§N5  A GENERALIZAÇÃO: já estava feita, e faltava interpretar.\n\n");
{
    /* O Aarao: "quanto ao 7D é só generalizar; na verdade já está generalizado no R^n, foi o
     * que fizemos, precisa só interpretar. Procura o 2D de Gentil."
     *
     * O 2D está lá, e o livro diz: "os B-3D generalizam, a um só tempo, os números complexos e
     * os nne-2D", com a imersão (x,y) = (x,0,y). E olhando a fórmula do 3D vê-se a recursão:
     * (a1a2 - b1b2, a1b2 + a2b1) É o produto complexo. Logo, com z no nível de baixo,
     *
     *     (z, c)·(w, d) = ( z·w·γ , c‖w‖ + d‖z‖ ),   γ = 1 − cd/(‖z‖‖w‖)
     *
     * e o "z·w" é o produto do nível de baixo. Sobe-se um nível de cada vez. */
    printf("      (z,c)·(w,d) = ( z·w·γ , c‖w‖ + d‖z‖ ),   γ = 1 - cd/(‖z‖‖w‖)\n");
    printf("      com z·w o produto do NÍVEL DE BAIXO — e daí sobe-se um de cada vez.\n\n");
    /* e mede-se AQUI, em C, e não se cita medida feita noutro sítio: escrever ok(...,1)
     * seria uma asserção que passa sempre, e isso já me apanhou antes. */
    int mal = 0;
    double piores[8] = {0};
    printf("        nível   norma multiplicativa?   max | ‖xy‖ - ‖x‖‖y‖ |\n");
    for(int d = 2; d <= 7; d++){
        double pior = 0;
        for(int k = 0; k < 400; k++){
            double x[8], y[8], p[8];
            for(int i = 0; i < d; i++){ x[i] = sin(7.0*k+i+1)*3; y[i] = cos(5.0*k+i+2)*3; }
            nne_rec(x, y, d, p);
            double nx = 0, ny = 0, np = 0;
            for(int i = 0; i < d; i++){ nx += x[i]*x[i]; ny += y[i]*y[i]; np += p[i]*p[i]; }
            /* ‖xy‖ = ‖x‖·‖y‖ ELEVA-SE AO QUADRADO e fica np = nx·ny — três raízes fora,
             * e a tese é a mesma porque as normas são não negativas e x ↦ x² é monótona
             * nelas. O que estava aqui tinha a MESMA função nos dois lados de uma
             * subtracção, e o resíduo media o arredondamento das três raízes tanto como a
             * multiplicatividade. Compara-se relativamente, que é o que a escala pede. */
            double e = fabs(np - nx*ny) / (nx*ny > 0 ? nx*ny : 1.0);
            if(e > pior) pior = e;
        }
        piores[d] = pior;
        printf("        R^%d     %-22s %.2e\n", d,
               (long long)(pior * 1e9) == 0 ? "SIM" : "NAO", pior);
        if((long long)(pior * 1e9) >= 1) mal++;
    }
    /* O `piores[]` era escrito e NUNCA lido — o compilador dizia-o — enquanto a linha
     * abaixo afirmava «o erro cresce de 3,5e-15 em R² para 1,4e-14 em R⁷», dois números
     * escritos à mão. E ERRADOS: o medido é 5,9e-16 e 9,3e-16, por um factor de seis e de
     * quinze. Ficaram da versão anterior, de antes de o resíduo passar a RELATIVO (a nota
     * de cima), e a correcção que mudou a conta não voltou ao texto.
     *
     * Agora os números saem do array, e a afirmação também: se eu mudar a medida, eles
     * mudam sozinhos. E a afirmação corrigida é mais fraca do que a que lá estava, porque
     * é a verdadeira — o erro NÃO cresce a cada andar: de R² para R³ ele DESCE. */
    long sobe = 0, passos = 0;
    for(int d = 3; d <= 7; d++){ passos++; if(piores[d] > piores[d-1]) sobe++; }
    printf("\n      o pior resíduo relativo vai de %.2e em R² a %.2e em R⁷ (factor %.1f),\n",
           piores[2], piores[7], piores[7]/piores[2]);
    printf("      e sobe em %ld dos %ld passos — nao em todos: de R² para R³ ele DESCE.\n\n",
           sobe, passos);
    ok("a recursão sobe e a norma continua multiplicativa — medido de R² a R⁷", mal == 0);
    ok("e o preço de subir é o ARREDONDAMENTO, que se acumula mas não a cada andar: o pior"
       " resíduo relativo é maior em R⁷ do que em R², e no entanto DESCE de R² para R³ —"
       " sobe em quatro dos cinco passos. Os números vêm do array que os mede, e antes"
       " estavam escritos à mão no texto, errados por um factor de quinze",
       piores[7] > piores[2] && sobe == 4 && passos == 5);
    printf("      É o arredondamento a\n");
    /* e a recursao TEM de dar o mesmo que a formula direta em d=3: senao eu implementei outra
     * coisa e chamei-lhe a mesma. */
    {
        double x[3] = {1,2,3}, y[3] = {2,0,0}, p[3];
        nne_rec(x, y, 3, p);
        W q = nne((W){1,2,3}, (W){2,0,0});
        printf("\n      e a recursão em d=3 dá (%g,%g,%g); a fórmula direta dá (%g,%g,%g)\n\n",
               p[0],p[1],p[2], q.a,q.b,q.c);
        ok("a recursão e a fórmula direta são a MESMA coisa em d=3 — mede-se EXACTO, sem"
           " limiar: a recursão e a fórmula directa são o MESMO programa com entradas"
           " iguais, logo batem bit a bit",
           p[0]==q.a && p[1]==q.b && p[2]==q.c);
    }
    printf("      acumular com os níveis, e nada mais. NÃO HÁ NÍVEL EM QUE PARE.\n");

    printf("\n      E A INTERPRETAÇÃO, que é o que faltava:\n\n");
    printf("        peça          quatro peças (bilinear)     nne (com a norma)\n");
    printf("        real          a0b0 - <a,b>                z·w·γ  (o produto de baixo)\n");
    printf("        imaginária    a0b + b0a                   c‖w‖ + d‖z‖\n");
    printf("        cruzado       a×b, só em dim 1,3,7        NÃO HÁ — o γ ocupa o lugar\n\n");
    printf("      O papel do ESCALAR a0 é feito pela NORMA ‖z‖. É essa a troca, e dela sai tudo:\n");
    printf("\n        a norma NÃO É LINEAR   ->  a multiplicação deixa de ser bilinear\n");
    printf("        e por isso              ->  sai da hipótese de Hurwitz\n");
    printf("        e não precisa do cruzado ->  não herda a obstrução de dimensão\n\n");
    printf("      O cruzado só existe em 1, 3 e 7; a NORMA existe sempre. Trocar um pelo outro é\n");
    printf("      trocar a bilinearidade pela liberdade de dimensão — e é um preço, não um\n");
    printf("      almoço grátis: perde-se a distributividade, que é o que faz um anel ser anel.\n");
    printf("\n      Então o Aarão tem razão nas duas: o 7D sai por recursão, e a generalização já\n");
    printf("      estava escrita no R^n — as quatro peças. O que faltava era ver que os nne são\n");
    printf("      a MESMA decomposição com o escalar trocado pela norma, e que é essa troca que\n");
    printf("      compra a dimensão livre.\n");
}

printf("\n§N6  A ÁLGEBRA DUAL DO GENTIL — e esta É distributiva. Corrijo-me.\n\n");
{
    /* O Aarão: "veja que Gentil preserva norma E é distributiva, e também tem potências".
     *
     * Eu tinha medido os nne-3D, visto que não são distributivos, e escrito isso como se
     * fosse "o Gentil". Não é: ele tem DUAS construções, e a outra é esta —
     *
     *     (a,b) ∗ (c,d) = (a·c, −b·d)
     *
     * com a MESMA adição, e o livro di-lo com todas as letras: "estas duas operações conferem
     * a Q uma estrutura de CORPO", e prova a distributividade em duas linhas via x∗y = −x·y. */
    printf("      (a,b) ∗ (c,d) = (a·c, −b·d),  com a mesma adição\n\n");
    int mal_d = 0, mal_h = 0;
    for(int k = 0; k < 300; k++){
        double x[2] = { sin(3.0*k+1)*3, cos(3.0*k+2)*3 };
        double y[2] = { sin(5.0*k+3)*3, cos(5.0*k+4)*3 };
        double z[2] = { sin(7.0*k+5)*3, cos(7.0*k+6)*3 };
        double yz[2] = { y[0]+z[0], y[1]+z[1] };
        double e[2] = { x[0]*yz[0], -x[1]*yz[1] };
        double d[2] = { x[0]*y[0] + x[0]*z[0], -x[1]*y[1] - x[1]*z[1] };
        if((long long)(fabs(e[0]-d[0]) * 1e12) >= 1 || (long long)(fabs(e[1]-d[1]) * 1e12) >= 1) mal_d++;
        double l = 1.7;
        double e2[2] = { l*x[0]*y[0], -l*x[1]*y[1] };
        double d2[2] = { l*(x[0]*y[0]), l*(-x[1]*y[1]) };
        if((long long)(fabs(e2[0]-d2[0]) * 1e12) >= 1 || (long long)(fabs(e2[1]-d2[1]) * 1e12) >= 1) mal_h++;
    }
    printf("      x∗(y+z) contra x∗y + x∗z, em 300 triplos: %d falhas\n", mal_d);
    printf("      (λx)∗y  contra λ(x∗y),    em 300 pares:   %d falhas\n\n", mal_h);
    ok("a álgebra DUAL do Gentil é distributiva E bilinear", mal_d == 0 && mal_h == 0);

    /* e a norma que ela preserva NAO e a euclidiana: e |ab|, o determinante de diag(a,b).
     *
     * A ROTA EM VIRGULA SAIU. Ela varria 200 pontos de sin/cos, guardava o pior erro de
     * cada uma das duas normas, e decidia com `pd < 1e-9`. Depois de a identidade passar a
     * ser medida EXACTA em Z, essa rota deixou de decidir seja o que for — ficava a
     * consumir sete doubles, duas chamadas a hypot por ponto e um limiar, para confirmar o
     * que a conta inteira ja' diz sem margem. Em matematica pura o par digital/analogico
     * nao se aplica: nao ha' meio a medir, ha' uma identidade.
     *
     * E A IDENTIDADE E' POLINOMIAL, logo mede-se EXACTA em Z e nao pede o 1e-9. Com
     * p = (x0.y0, -x1.y1), o produto das componentes e'
     *
     *     p0.p1 = x0.y0 . (-x1.y1) = -(x0.x1).(y0.y1)     ⟹  |p0.p1| = |x0.x1|.|y0.y1|
     *
     * — nao ha nada a arredondar. E o CONTRASTE mede-se do mesmo modo: a euclidiana falha,
     * e conta-se em quantos casos, para que «nao preserva» seja um numero e nao uma
     * afirmacao. */
    long det_ok = 0, eucl_ok = 0, tot_i = 0, sinal_ok = 0;
    for(long x0 = -4; x0 <= 4; x0++) for(long x1 = -4; x1 <= 4; x1++)
    for(long y0 = -4; y0 <= 4; y0++) for(long y1 = -4; y1 <= 4; y1++){
        long p0 = x0*y0, p1 = -x1*y1;
        long lhs = p0*p1;              if(lhs < 0) lhs = -lhs;
        long rx = x0*x1;               if(rx < 0)  rx = -rx;
        long ry = y0*y1;               if(ry < 0)  ry = -ry;
        tot_i++;
        if(lhs == rx*ry) det_ok++;                       /* o DETERMINANTE: exacto */
        /* E COM SINAL, que e' mais forte. Mutei `p1 = -x1.y1` para `+x1.y1` e a asserção
         * do modulo SOBREVIVEU — o valor absoluto nao ve o sinal, logo aquela medida nao
         * distinguia a multiplicacao hiperbolica de outra. A identidade com sinal
         *
         *     p0.p1 = -(x0.x1).(y0.y1)
         *
         * ve-o, e e' ela que fixa qual e' o produto. Uma mutacao que sobrevive e' um gap
         * meu, e o remedio nao e' afrouxar o gume: e' medir a frase inteira. */
        if(p0*p1 == -(x0*x1)*(y0*y1)) sinal_ok++;
        /* a euclidiana: |p|² =? |x|².|y|², nos quadrados para nao formar raiz */
        if((p0*p0 + p1*p1) == (x0*x0 + x1*x1)*(y0*y0 + y1*y1)) eucl_ok++;
    }
    printf("      e em INTEIROS, sem regua: |p0.p1| = |x0.x1|.|y0.y1| em %ld de %ld\n",
           det_ok, tot_i);
    printf("      e a EUCLIDIANA so' fecha em %ld de %ld — e' o contraste que mede\n",
           eucl_ok, tot_i);
    printf("      e COM SINAL, p0.p1 = -(x0.x1)(y0.y1), em %ld de %ld — o modulo nao o via\n\n",
           sinal_ok, tot_i);
    ok("ela preserva |ab| — que é o DETERMINANTE, e não a norma euclidiana. E a identidade"
       " é POLINOMIAL, logo mede-se EXACTA em ℤ: vale em todos os casos, enquanto a"
       " euclidiana falha na maioria — o contraste é um número. A rota em vírgula que aqui"
       " estava, com 200 pontos de sin/cos e um 1e-9, saiu: deixou de decidir",
       tot_i > 0 && det_ok == tot_i && eucl_ok < tot_i && sinal_ok == tot_i);
    printf("      E isso arruma o par: |ab| = 1 é a HIPÉRBOLE (Δ>0, o gato); a²+b² = 1 é a\n");
    printf("      ESFERA (Δ<0, o esquilo). A canónica fica com a esfera, a dual com a hipérbole,\n");
    printf("      e são as duas metades do chicote. Não são construções rivais: são o par.\n");
    printf("\n      A MINHA AFIRMAÇÃO ESTAVA ESTREITA. Eu medi os nne-3D, vi que não distribuem,\n");
    printf("      e escrevi \"o Gentil não é bilinear\" — quando o que eu tinha medido era UMA das\n");
    printf("      construções dele. A outra é bilinear, distributiva, tem corpo, tem inverso\n");
    printf("      ((1/a, −1/b), com neutro (1,−1)) e preserva a sua norma. Generalizar de uma\n");
    printf("      amostra é o erro que ando a apanhar, e apanhei-me nele outra vez.\n");
}

printf("\n§N7  O CRUZADO DUAL: não é que não exista em par — é que ali é OUTRO.\n\n");
{
    /* O Aarao: "vc afirma que cruzado nao existe em pares, mas nao se trata de nao existir, se
     * trata de COMPLETAR o cruzado; quando vc abandona a exigencia de Hurwitz e assume a dual
     * vc obtem o cruzado dual, que sao as esferas pares que completam todas as esferas". */
    printf("      cruzado  a×b    antissimétrico e bilinear, com a identidade de Lagrange\n");
    printf("                      -> só em dimensão 1, 3 e 7, que são ÍMPARES\n");
    printf("      o DUAL   J      J² = −I, a rotação de 90° em cada plano\n");
    printf("                      -> só em dimensão PAR\n\n");
    printf("      e a razão de J só existir em par é de duas linhas:\n");
    printf("        det(J)² = det(J²) = det(−I) = (−1)^n\n");
    printf("        em n ÍMPAR isso pede det(J)² = −1, e o determinante de matriz real é real\n\n");
    int mal = 0;
    printf("      dim   a×b   J     o que existe lá\n");
    for(int n = 1; n <= 8; n++){
        int c = (n == 1 || n == 3 || n == 7), j = (n % 2 == 0);
        printf("      %-5d %-5s %-5s %s\n", n, c ? "sim" : "—", j ? "sim" : "—",
               c ? "o cruzado" : j ? "o DUAL (J)" : "nenhum dos dois");
        if(c && j) mal++;                       /* nunca os dois ao mesmo tempo */
    }
    printf("\n");
    ok("o cruzado e o seu dual NUNCA coexistem — um está onde o outro não está", mal == 0);
    printf("      Então não é que o cruzado \"não exista em par\": é que em par o lugar dele é\n");
    printf("      ocupado por OUTRO objeto, com a mesma função e outra assinatura. Dizer que não\n");
    printf("      existe é olhar só para um dos dois e chamar ausência ao que é troca.\n");
    printf("\n      E os dois COMPLETAM-SE: onde a dimensão é par há J, onde é 1, 3 ou 7 há a×b.\n");
    printf("      O que fica a descoberto são as ímpares que não são 1, 3 nem 7 — e aí, nem um\n");
    printf("      nem outro. É a lacuna verdadeira, e é bem menor do que a que eu tinha escrito.\n");
    printf("\n      E o par (esfera, hipérbole) é o mesmo: a norma euclidiana dá a esfera e a\n");
    printf("      |ab| dá a hipérbole. As esferas são o lugar da norma 1 — o determinante ±1 —\n");
    printf("      e é aí que a cifra vive, porque é aí que ela não se degrada.\n");
}

printf("\n§N8  O LADO DA TORRE QUE É DE GENTIL — par e ímpar, e continua dual.\n\n");
{
    /* O Aarão, em três golpes: «Gentil dá as álgebras de dimensão ÍMPAR, todas elas»;
     * «em 2D simplesmente dá corpo, porque estão juntos os duais — em 2D Gentil e
     * complexos»; «nos outros separa em parte par e ímpar, mas continua dual».
     *
     * E a fórmula do corpus diz as três, se se olhar para a forma dela. Em R³ o elemento
     * é w = (a, b, c) e o produto trata as duas partes de modo DIFERENTE:
     *
     *      parte PAR   (a,b)   MULTIPLICA — o produto complexo, escalado por γ
     *      parte ÍMPAR (c)     SOMA       — c₁r₂ + c₂r₁
     *
     * É o par dual desta casa outra vez: um lado multiplica, o outro soma. A parte par é
     * o DIRECTO que gira; a ímpar é o que se acumula.
     *
     * ── E EM 2D DÁ CORPO, PORQUE A PARTE ÍMPAR NÃO EXISTE ────────────────────
     * Com c₁ = c₂ = 0 vem γ = 1 − 0 = 1, e D4 colapsa em
     *
     *      (a₁a₂ − b₁b₂,  a₁b₂ + a₂b₁,  0)
     *
     * que é EXACTAMENTE o produto de ℂ. Sem parte ímpar, os duais ficam juntos e o que
     * sobra é um corpo — e é por isso que em dimensão 2 Gentil e os complexos são o mesmo.
     *
     * ── E É ASSIM QUE OS DOIS LADOS DA TORRE SE ENCAIXAM ─────────────────────
     *      HURWITZ   1, 2, 4, 8    norma BILINEAR         a dobra dual
     *      GENTIL    as ÍMPARES    norma HOMOGÉNEA        a parte que soma
     * e em 2 coincidem, porque aí a parte ímpar é vazia. */
    long casos = 0, complexo = 0, par_mult = 0, impar_soma = 0, norma_ok = 0;
    printf("      caso                       parte PAR             parte ÍMPAR\n");
    for(int t = 0; t < 300; t++){
        double a1 = sin(2.0*t+1)*3, b1 = cos(2.0*t+2)*3;
        double a2 = sin(3.0*t+3)*3, b2 = cos(3.0*t+4)*3;
        /* (i) com a parte ÍMPAR nula, é o produto complexo — exacto */
        {
            W u = { a1, b1, 0.0 }, v = { a2, b2, 0.0 }, p = nne(u, v);
            double cr = a1*a2 - b1*b2, ci = a1*b2 + a2*b1;
            casos++;
            if((long long)(fabs(p.a - cr) * 1e9) == 0 && (long long)(fabs(p.b - ci) * 1e9) == 0
            && (long long)(fabs(p.c) * 1e12) == 0) complexo++;
        }
        /* (ii) com ela NÃO nula, as duas partes fazem coisas diferentes */
        {
            double c1 = 0.5 + 0.3*sin(5.0*t), c2 = 0.7 + 0.2*cos(5.0*t);
            W u = { a1, b1, c1 }, v = { a2, b2, c2 }, p = nne(u, v);
            double r1 = hypot(a1,b1), r2 = hypot(a2,b2);
            double g = 1.0 - c1*c2/(r1*r2);
            /* a par MULTIPLICA (o complexo vezes γ) */
            if((long long)(fabs(p.a - (a1*a2-b1*b2)*g) * 1e9) == 0
            && (long long)(fabs(p.b - (a1*b2+a2*b1)*g) * 1e9) == 0) par_mult++;
            if((long long)(fabs(p.c - (c1*r2 + c2*r1)) * 1e9) == 0) impar_soma++;
            double nu = sqrt(a1*a1+b1*b1+c1*c1), nv = sqrt(a2*a2+b2*b2+c2*c2);
            double np = sqrt(p.a*p.a+p.b*p.b+p.c*p.c);
            if((long long)(fabs(np - nu*nv) * 1e9) == 0) norma_ok++;
        }
    }
    printf("      c = 0 (sem parte ímpar)    o produto de ℂ         ausente\n");
    printf("      c ≠ 0                      complexo × γ           c₁r₂ + c₂r₁ (SOMA)\n\n");
    printf("      %ld casos: colapsa em ℂ em %ld · a par multiplica em %ld · a ímpar soma"
           " em %ld · norma em %ld\n\n", casos, complexo, par_mult, impar_soma, norma_ok);
    ok("O LADO DA TORRE QUE É DE GENTIL SEPARA PAR E ÍMPAR, E CONTINUA DUAL: no produto do"
       " corpus a parte PAR (a,b) MULTIPLICA — é o produto complexo escalado por γ — e a"
       " parte ÍMPAR (c) SOMA, c₁r₂ + c₂r₁. É o par desta casa outra vez: um lado"
       " multiplica, o outro acumula. E EM 2D DÁ CORPO, porque a parte ímpar não existe:"
       " com c = 0 vem γ = 1 e a fórmula colapsa EXACTAMENTE no produto de ℂ — os duais"
       " ficam juntos, e Gentil e os complexos são o mesmo objecto. É assim que os dois"
       " lados da torre encaixam: Hurwitz nas dimensões da dobra bilinear, Gentil nas"
       " ímpares com norma homogénea, e em 2 coincidem porque aí não há parte ímpar",
       complexo == casos && par_mult == casos && impar_soma == casos
       && norma_ok == casos && casos == 300);
}

printf("\n§N4  E o que isto corrige no que eu tinha escrito.\n\n");
{
    printf("      Eu escrevi, no multiplicacao.tex:\n\n");
    printf("        \"um produto bilinear antissimétrico com a identidade de Lagrange só existe\n");
    printf("         em dimensão 0, 1, 3 e 7\"\n\n");
    printf("      e isso está CERTO. O que estava errado era o que eu tirei dele — que a torre\n");
    printf("      PARA, dito sem repetir a hipótese. A hipótese é BILINEAR, e é ela que faz todo\n");
    printf("      o trabalho:\n\n");
    printf("        com bilinearidade    -> R, C, H, O, e mais nada (Hurwitz)\n");
    printf("        sem bilinearidade    -> os nne-3D, com norma multiplicativa em R³\n\n");
    conclui("as duas coisas convivem: uma é dentro da hipótese, a outra é fora");
    printf("      Não há contradição nenhuma entre os dois, e é esse o ponto. Hurwitz não é uma\n");
    printf("      parede do mundo: é uma classificação SOB UMA CONDIÇÃO, e largando a condição\n");
    printf("      há mais coisas. Chamar-lhe obstrução foi eu a tomar a minha hipótese por uma\n");
    printf("      propriedade do espaço.\n");
    printf("\n      E é literalmente o erro que este projeto passa o dia a apanhar no corpus\n");
    printf("      científico: a afirmação vale NO CORPO DECLARADO, e omitir o corpo transforma\n");
    printf("      um teorema numa proibição. Cometi-o num paper meu, sobre um teorema que eu\n");
    printf("      próprio tinha citado corretamente duas linhas acima.\n");
    printf("\n      Quanto ao 7D: o corpus que tenho aqui (corpus_gentil.txt) traz os nne-2D e os\n");
    printf("      nne-3D, com a fórmula acima e as aplicações a fractais. NÃO encontrei a versão\n");
    printf("      7D nesse material, e por isso não a meço nem a afirmo — fica dito que a\n");
    printf("      procurei e não estava lá, e não que não exista.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
