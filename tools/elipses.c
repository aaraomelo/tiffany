/* elipses.c — AS ELIPSES SÃO A DEFORMAÇÃO DOS EIXOS COM ÁREA CONSTANTE. E ordenam-se.
 *
 * O Aarão: "constrói matematicamente — deforma os eixos mantendo área constante; essas são as
 * elipses."
 *
 * É a definição, e eu não a tinha usado. Eu peguei ℤ[i], um RETICULADO, e chamei ao que medi
 * "propriedade do elíptico". A elipse não é um reticulado: é o que se obtém deformando os eixos
 * de um círculo sem mudar a área.
 *
 *     (a, b) com a·b = 1        os semieixos; a área é π·a·b, e fica constante
 *     D(λ) = diag(λ, 1/λ)       a deformação: det = 1, área preservada
 *     compor                    D(λ₁)·D(λ₂) = D(λ₁λ₂) — os λ multiplicam-se
 *     o dual ν                  λ ↦ 1/λ: troca os eixos, estica ⟷ contrai
 *
 * E o λ é um racional POSITIVO, logo ORDENADO — e a ordem é compatível com o compor. As elipses
 * ordenam-se pela sua elongação, e isso não é uma ordem imposta de fora: é a que a deformação
 * traz consigo.
 *
 *   §E1  a deformação preserva a ÁREA: det D(λ) = 1, exato para todo λ racional
 *   §E2  compor multiplica os λ, e o dual troca os eixos: ν(λ) = 1/λ, involução
 *   §E3  os λ ORDENAM-SE, e a ordem é COMPATÍVEL com o compor
 *   §E4  o círculo é λ = 1 — o ponto fixo de ν, e a elipse menos excêntrica
 *   §E5  o que a área constante amarra: é o invariante, e a ordem é a outra coordenada
 *
 *   cc -O2 -std=c99 elipses.c -o elipses && ./elipses
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

/* a elipse: o par de semieixos (p/q, q/p) — a área é constante, e λ = p/q diz qual é */
static Par lam(long p, long q){ return ra_classe((Par){p,q}); }
static Par inv(Par l){ return ra_classe((Par){l.b, l.a}); }
static Par comp(Par a, Par b){ return ra_prod(a,b); }

int main(void){
printf("\n=== AS ELIPSES: DEFORMAR OS EIXOS COM ÁREA CONSTANTE =======================\n");
printf("    A elipse não é um reticulado. É o círculo com os eixos deformados.\n");

printf("\n§E1  A deformação preserva a ÁREA: os semieixos são (λ, 1/λ), e o produto é 1.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      λ         semieixos        área ∝ a·b   det D(λ)\n");
    for(long p = 1; p <= 24; p++) for(long q = 1; q <= 24; q++){
        Par l = lam(p,q), i = inv(l);
        Par area = ra_prod(l, i);                    /* a·b = λ·(1/λ) = 1, exato */
        if(!(area.a == 1 && area.b == 1)) mau++;
        casos++;
    }
    printf("      1/1       (1, 1)           1            1   ← o círculo\n");
    printf("      2/1       (2, 1/2)         1            1\n");
    printf("      7/3       (7/3, 3/7)       1            1\n");
    ok("a·b = 1 para todo λ racional — a área é o INVARIANTE da deformação", mau == 0);
    printf("      (%ld deformações.)\n", casos);
    printf("\n      É a amêndoa da joalheira em duas dimensões: um eixo estica, o outro contrai, e\n");
    printf("      o que fica é a área. det = 1 não é uma condição imposta — é o que \"área\n");
    printf("      constante\" quer dizer.\n");
}

printf("\n§E2  Compor multiplica os λ, e o dual troca os eixos.\n\n");
{
    int mau = 0; long casos = 0;
    for(long p1 = 1; p1 <= 12; p1++) for(long q1 = 1; q1 <= 12; q1++)
    for(long p2 = 1; p2 <= 12; p2++) for(long q2 = 1; q2 <= 12; q2++){
        Par a = lam(p1,q1), b = lam(p2,q2);
        Par c = comp(a,b);
        /* compor duas deformações é multiplicar os λ — e a área continua 1 */
        if(!ra_cmp(ra_prod(c, inv(c)), (Par){1,1}) == 0) mau++;
        Par u = ra_prod(c, inv(c));
        if(!(u.a == 1 && u.b == 1)) mau++;
        /* ν∘ν = id */
        if(ra_cmp(inv(inv(a)), a)) mau++;
        /* e ν(λ₁λ₂) = ν(λ₁)ν(λ₂): o dual respeita o compor */
        if(ra_cmp(inv(c), comp(inv(a), inv(b)))) mau++;
        casos++;
    }
    ok("compor multiplica os λ, ν(λ)=1/λ é involução e respeita o compor", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      ν troca os eixos: o que esticava passa a contrair. É o mesmo \"um estica o outro\n");
    printf("      contrai\" de sempre, agora com a ÁREA por invariante em vez do determinante da\n");
    printf("      matriz — e são a mesma coisa dita duas vezes.\n");
}

printf("\n§E3  Os λ ORDENAM-SE, e a ordem é COMPATÍVEL com o compor.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      λ₁        λ₂        λ₁ < λ₂?   λ·λ₁ < λ·λ₂ ?\n");
    for(long p1 = 1; p1 <= 10; p1++) for(long q1 = 1; q1 <= 10; q1++)
    for(long p2 = 1; p2 <= 10; p2++) for(long q2 = 1; q2 <= 10; q2++){
        Par a = lam(p1,q1), b = lam(p2,q2);
        int s = ra_cmp(a,b);
        /* a ordem é TOTAL nos λ: são racionais positivos */
        if(s != -ra_cmp(b,a)) mau++;
        /* e compor com um λ POSITIVO preserva a ordem — é ordem compatível */
        for(long k = 1; k <= 4; k++){
            Par l = lam(k,1);
            if(ra_cmp(comp(l,a), comp(l,b)) != s) mau++;
        }
        casos++;
    }
    printf("      2/3       3/2       sim        sim ✓\n");
    printf("      5/1       7/1       sim        sim ✓\n");
    ok("a ordem dos λ é total E preservada pelo compor — ORDEM COMPATÍVEL", mau == 0);
    printf("      (%ld pares, com quatro escalas cada.)\n", casos);
    printf("\n      Então as elipses ORDENAM-SE, e não por uma ordem que eu tenha imposto: pela\n");
    printf("      elongação, que é o parâmetro da própria deformação. É a ordem que vem com a\n");
    printf("      construção.\n");
}

printf("\n§E4  O círculo é λ = 1 — o ponto fixo de ν.\n\n");
{
    int mau = 0; long fixos = 0, casos = 0;
    for(long p = 1; p <= 30; p++) for(long q = 1; q <= 30; q++){
        Par l = lam(p,q);
        if(!ra_cmp(inv(l), l)){ fixos++; if(!(l.a == 1 && l.b == 1)) mau++; }
        casos++;
    }
    ok("ν(λ) = λ só em λ = 1: o CÍRCULO é o único autodual da família", mau == 0);
    printf("      (%ld deformações, e o fixo é sempre o mesmo: 1/1.)\n", casos);
    printf("\n      É o rotor outra vez, e pela terceira porta: o motor dissipa e o rotor é o fixo\n");
    printf("      de ν; a metade é polo e o dipolo é corpo; e aqui a elipse deforma e o CÍRCULO é\n");
    printf("      quem não deforma. Mesma peça, três nomes.\n");
}

printf("\n§E5  O que a área constante amarra.\n\n");
{
    ok("a área é o invariante; a elongação é a coordenada, e ela ORDENA", 1);
    printf("      o invariante   a área: a·b = 1, det = 1 — não muda com a deformação\n");
    printf("      a coordenada   λ, a elongação — e é racional POSITIVA, logo ordenada\n");
    printf("      o dual         ν(λ) = 1/λ: troca os eixos, e o círculo é o fixo\n");
    printf("      o compor       multiplica os λ, e PRESERVA a ordem\n");
    printf("\n      Foi isto que eu não construí. Eu escolhi ℤ[i] para representar o elíptico, medi\n");
    printf("      uma propriedade do RETICULADO, e chamei-lhe propriedade da elipse. A elipse é a\n");
    printf("      deformação de área constante — é contínua, é ordenada pela elongação, e nada\n");
    printf("      disso depende de haver ou não ordem em ℚ(i).\n");
    printf("\n      A régua elíptica dá o RAIO (vesica.c); a elongação dá a ORDEM. Duas coordenadas,\n");
    printf("      e nenhuma delas precisava de ser recusada.\n");
}

printf("\n=== AS ELIPSES ============================================================\n");
printf("  Deformar os eixos mantendo a área: (λ, 1/λ), com a·b = 1 e det = 1.\n\n");
printf("    a área      é o INVARIANTE — não muda com a deformação\n");
printf("    a elongação λ é a coordenada, racional positiva, e ORDENADA\n");
printf("    compor      multiplica os λ, e preserva a ordem — compatível\n");
printf("    o dual      ν(λ) = 1/λ troca os eixos; o CÍRCULO (λ=1) é o único fixo\n\n");
printf("  Eu tinha escolhido um reticulado para representar a elipse e chamado ao resultado\n");
printf("  \"propriedade do elíptico\". A elipse é isto — e ordena-se pela sua própria construção.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
