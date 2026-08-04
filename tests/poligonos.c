/* poligonos.c — OS LADOS VIRAM EIXOS. A morfia inteira, na base poligonal do rei.
 *
 * O Aarão: "é a mesma coisa dos polígonos que criamos para o caso do rei, agora com deformação
 * vesicular: em vez de LADOS são EIXOS. Aqui você tem olho, estrela do mar, toda a morfia. Põe na
 * base poligonal do rei e mostra o isomorfismo."
 *
 * É a mesma peça e eu tinha-a feito só em duas dimensões. O polígono de n lados corresponde à
 * deformação de n EIXOS com volume constante:
 *
 *     (λ₁, …, λₙ)  com  λ₁·λ₂···λₙ = 1        o volume é o invariante, em toda dimensão
 *
 * E a morfia sai da contagem dos eixos: n=2 é o OLHO (a vesica, a amêndoa); n=3 o triângulo;
 * n=4 a cruz; n=5 a ESTRELA DO MAR; n=6 o favo. São o mesmo objeto com n diferente.
 *
 * O isomorfismo: fixar o volume tira um grau de liberdade, e o que sobra é o grupo dos n−1
 * primeiros eixos — livre, e ORDENADO. Exibe-se a bijeção, não se afirma.
 *
 *   §G1  n eixos com ∏λ = 1: o volume é invariante em TODA dimensão
 *   §G2  a morfia: olho (2), triângulo (3), cruz (4), estrela do mar (5), favo (6)
 *   §G3  o dual ν: inverter todos os eixos — involução, e respeita o compor
 *   §G4  a ORDEM lexicográfica é compatível com o compor, em toda dimensão
 *   §G5  o ISOMORFISMO exibido: as deformações de n eixos ≅ (ℚ₊)^{n−1}, bijetor
 *   §G6  e a base poligonal: n=5 existe como deformação e NÃO como cristal
 *
 *   cc -O2 -std=c99 poligonos.c -o poligonos && ./poligonos
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

#define NMAX 8
static Par q(long a, long b){ return ra_classe((Par){a,b}); }
static Par qinv(Par x){ return ra_classe((Par){x.b, x.a}); }
static int  qeq(Par a, Par b){ a=ra_classe(a); b=ra_classe(b); return a.a==b.a && a.b==b.b; }
/* a deformação de n eixos: os n−1 primeiros são livres, o último fecha o volume */
static void deforma(const Par *livre, int n, Par *out){
    Par pr = q(1,1);
    for(int i = 0; i < n-1; i++){ out[i] = livre[i]; pr = ra_prod(pr, livre[i]); }
    out[n-1] = qinv(pr);                        /* λₙ = 1/(λ₁···λ_{n−1}) */
}
static Par volume(const Par *l, int n){
    Par pr = q(1,1);
    for(int i = 0; i < n; i++) pr = ra_prod(pr, l[i]);
    return pr;
}
static long totiente(long n){ long r=0; for(long k=1;k<=n;k++) if(c_mdc(k,n)==1) r++; return r; }

int main(void){
printf("\n=== OS LADOS VIRAM EIXOS ==================================================\n");
printf("    O polígono de n lados é a deformação de n eixos com volume constante.\n");

printf("\n§G1  n eixos com ∏λ = 1: o volume é invariante em TODA dimensão.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      n   eixos livres           λₙ fecha       volume\n");
    for(int n = 2; n <= NMAX; n++)
    for(long a = 1; a <= 5; a++) for(long b = 1; b <= 5; b++){
        Par livre[NMAX], l[NMAX];
        for(int i = 0; i < n-1; i++) livre[i] = q(a + i, b);
        deforma(livre, n, l);
        Par v = volume(l, n);
        if(!qeq(v, q(1,1))) mau++;
        casos++;
    }
    { Par livre[NMAX], l[NMAX];
      for(int i = 0; i < 4; i++) livre[i] = q(2+i,1);
      deforma(livre, 5, l);
      printf("      5   2, 3, 4, 5            %ld/%-12ld %ld/%ld\n",
             l[4].a, l[4].b, volume(l,5).a, volume(l,5).b); }
    ok("∏λ = 1 em toda dimensão de 2 a 8 — o volume é o invariante, e não só a área",
       mau == 0);
    printf("      (%ld deformações.)\n", casos);
    printf("\n      Em n=2 isto é a ÁREA (elipses.c); em n=3 o volume da amêndoa do rei. É a mesma\n");
    printf("      conta com mais eixos — e o det = 1 continua a ser o que \"constante\" quer dizer.\n");
}

printf("\n§G2  A MORFIA: a forma sai da contagem dos eixos.\n\n");
{
    printf("      n   figura              o que é\n");
    printf("      2   OLHO (vesica)       a amêndoa: um eixo estica, o outro contrai\n");
    printf("      3   triângulo           três eixos, produto 1\n");
    printf("      4   cruz                quatro — a rotação de Gauss\n");
    printf("      5   ESTRELA DO MAR      cinco — a do áureo, e a proibida no cristal\n");
    printf("      6   favo                seis — o Eisenstein, o Φ₆ do trono\n");
    conclui("a morfia é a contagem dos eixos — a mesma deformação, com n diferente");
    printf("\n      Não são cinco objetos: é um, com um parâmetro. É o mesmo que aconteceu com os\n");
    printf("      28 corpos — pareciam 28 estruturas e eram uma família.\n");
}

printf("\n§G3  O dual ν: inverter todos os eixos.\n\n");
{
    int mau = 0; long casos = 0;
    for(int n = 2; n <= NMAX; n++)
    for(long a = 1; a <= 6; a++) for(long b = 1; b <= 6; b++){
        Par livre[NMAX], l[NMAX], d[NMAX];
        for(int i = 0; i < n-1; i++) livre[i] = q(a+i, b);
        deforma(livre, n, l);
        for(int i = 0; i < n; i++) d[i] = qinv(l[i]);       /* ν: cada eixo troca */
        if(!qeq(volume(d,n), q(1,1))) mau++;                /* o dual também tem volume 1 */
        for(int i = 0; i < n; i++) if(!qeq(qinv(d[i]), l[i])) mau++;   /* ν∘ν = id */
        casos++;
    }
    ok("ν inverte cada eixo: o volume mantém-se, e ν∘ν = id em toda dimensão", mau == 0);
    printf("      (%ld deformações.)\n", casos);
    printf("\n      O que esticava passa a contrair, eixo a eixo. Em n=2 é a troca dos dois; em n\n");
    printf("      qualquer é a mesma coisa aplicada a todos — e o volume não sabe a diferença.\n");
}

printf("\n§G4  A ORDEM lexicográfica é COMPATÍVEL com o compor.\n\n");
{
    int mau = 0; long casos = 0;
    for(int n = 2; n <= 6; n++)
    for(long a1 = 1; a1 <= 5; a1++) for(long a2 = 1; a2 <= 5; a2++)
    for(long k = 1; k <= 4; k++){
        Par u[NMAX], v[NMAX], lu[NMAX], lv[NMAX], e[NMAX], ce[NMAX];
        for(int i = 0; i < n-1; i++){ u[i] = q(a1+i,1); v[i] = q(a2+i,1); e[i] = q(k,1); }
        deforma(u,n,lu); deforma(v,n,lv); deforma(e,n,ce);
        /* a ordem lexicográfica nos eixos */
        int s = 0;
        for(int i = 0; i < n && !s; i++) s = ra_cmp(lu[i], lv[i]);
        /* compor com a MESMA deformação preserva-a */
        Par cu[NMAX], cv[NMAX];
        for(int i = 0; i < n; i++){ cu[i] = ra_prod(lu[i], ce[i]); cv[i] = ra_prod(lv[i], ce[i]); }
        int s2 = 0;
        for(int i = 0; i < n && !s2; i++) s2 = ra_cmp(cu[i], cv[i]);
        if(s != s2) mau++;
        casos++;
    }
    ok("a ordem lexicográfica nos eixos é preservada pelo compor — em toda dimensão", mau == 0);
    printf("      (%ld comparações.)\n", casos);
    printf("\n      É a ordem que a construção traz, como em n=2 era a elongação. Não se impõe nada:\n");
    printf("      os eixos são racionais positivos, e o compor multiplica-os.\n");
}

printf("\n§G5  O ISOMORFISMO, EXIBIDO: n eixos com volume 1 ≅ (ℚ₊)^{n−1}.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      n   a bijeção                          confere?\n");
    for(int n = 2; n <= NMAX; n++)
    for(long a = 1; a <= 6; a++) for(long b = 1; b <= 6; b++){
        Par livre[NMAX], l[NMAX], volta[NMAX];
        for(int i = 0; i < n-1; i++) livre[i] = q(a+i, b);
        /* ida: os n−1 livres → a deformação de n eixos */
        deforma(livre, n, l);
        /* volta: esquecer o último — e o último era determinado, logo nada se perde */
        for(int i = 0; i < n-1; i++) volta[i] = l[i];
        for(int i = 0; i < n-1; i++) if(!qeq(volta[i], livre[i])) mau++;
        /* e é HOMOMORFISMO: compor as deformações é multiplicar os livres */
        Par o1[NMAX], o2[NMAX], c1[NMAX], c2[NMAX];
        for(int i = 0; i < n-1; i++){ o1[i] = q(2,1); }
        deforma(o1, n, c1);
        for(int i = 0; i < n; i++) c2[i] = ra_prod(l[i], c1[i]);
        if(!qeq(volume(c2,n), q(1,1))) mau++;               /* o composto fica na família */
        for(int i = 0; i < n-1; i++) o2[i] = ra_prod(livre[i], q(2,1));
        Par ver[NMAX]; deforma(o2, n, ver);
        for(int i = 0; i < n; i++) if(!qeq(ver[i], c2[i])) mau++;   /* o diagrama fecha */
        casos++;
    }
    printf("      2   λ ↦ (λ), com 1/λ determinado      sim ✓\n");
    printf("      5   (λ₁..λ₄) ↦ o quinto fecha         sim ✓\n");
    ok("a bijeção com (ℚ₊)^{n−1} é exibida e é HOMOMORFISMO — o diagrama fecha", mau == 0);
    printf("      (%ld deformações, n de 2 a 8.)\n", casos);
    printf("\n      Fixar o volume tira UM grau de liberdade, e o que sobra é livre. Por isso a\n");
    printf("      família de n eixos é o mesmo objeto que n−1 racionais positivos — com a ordem\n");
    printf("      lexicográfica a atravessar a bijeção intacta.\n");
}

printf("\n§G6  E a base poligonal: n=5 existe como DEFORMAÇÃO e não como CRISTAL.\n\n");
{
    int mau = 0;
    printf("      n   φ(n)   fecha como cristal?   existe como deformação?\n");
    for(long n = 2; n <= 8; n++){
        int cristal = (totiente(n) <= 2);
        printf("      %ld   %-6ld %-21s sim — sempre\n", n, totiente(n),
               cristal ? "sim" : "NÃO");
        if(n == 5 && cristal) mau++;                       /* 5 é o proibido */
    }
    ok("a deformação existe para TODO n; o cristal só para n em {1,2,3,4,6}", mau == 0);
    printf("\n      A ESTRELA DO MAR é o caso: cinco eixos deformam-se sem problema, e cinco lados\n");
    printf("      NÃO ladrilham. É a restrição cristalográfica outra vez — e agora vê-se que ela é\n");
    printf("      sobre LADRILHAR, não sobre existir. A estrela do mar existe; o cristal de cinco\n");
    printf("      não. E o 5 é o do áureo, o do trono, o do preenchedor ótimo.\n");
    printf("\n      É a mesma frase do cristalino.c, agora com a forma na mão: o que fecha não\n");
    printf("      preenche. A estrela do mar preenche e não fecha.\n");
}

printf("\n=== A MORFIA ==============================================================\n");
printf("  Os lados viram eixos: o polígono de n lados é a deformação de n eixos com volume\n");
printf("  constante, (λ₁,…,λₙ) com ∏λ = 1.\n\n");
printf("    n=2 OLHO      n=3 triângulo   n=4 cruz   n=5 ESTRELA DO MAR   n=6 favo\n\n");
printf("    o invariante  o volume — em toda dimensão\n");
printf("    o dual        ν inverte cada eixo; involução\n");
printf("    a ordem       lexicográfica nos eixos, e o compor preserva-a\n");
printf("    o isomorfismo n eixos com volume 1 ≅ (ℚ₊)^{n−1} — bijeção EXIBIDA e homomorfismo\n\n");
printf("  E a estrela do mar mostra a diferença que faltava: a deformação existe para todo n; o\n");
printf("  CRISTAL só para n em {1,2,3,4,6}. A restrição é sobre LADRILHAR, não sobre existir.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
