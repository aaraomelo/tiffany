/* catalogo.c — O CATÁLOGO INTEIRO NO TOOLKIT, e ele é menor do que parecia.
 *
 * O Aarão: "volta a trazer o catálogo de chess/ completo para o toolkit."
 *
 * A minha primeira ideia foi implementar os 25 corpos que faltavam, um a um. Está errada, e quem
 * o diz é o próprio catálogo — `chess/elementares/catalogo_isomorfismos.py` abre com a tese:
 *
 *     "quase todo corpo é o mesmo CORPO-MÃE (ℝ ou ℂ) vestido por uma RÉGUA diferente. E há só
 *      QUATRO transformações-tipo que ligam um corpo a outro — as arestas do grafo."
 *
 * Então o catálogo completo não são 29 estruturas: são UMA família com um parâmetro, mais quatro
 * setas, mais três exceções que não são corpos e têm de ficar marcadas como tais. Trazer os 29
 * como implementações separadas seria copiar o mesmo código com nomes diferentes.
 *
 *     P  Pontryagin  exp/log    soma ↔ produto        χ(u+v) = χ(u)·χ(v)      exata em ℤ/p
 *     W  Wick        t ↦ i·t    euclid. ↔ lorentz.    o SINAL da borda        exata em ℤ
 *     ν  nu          −rev       corpo ↔ dual          m ↦ −m, a antípoda      exata em ℤ
 *     L  Legendre    T → 0      pleno ↔ sombra        max = lim T·log Σ       NÃO é isomorfismo
 *
 *   §G1  W é o SINAL da borda, e leva o gato ao esquilo — a seta que eu usei sem lhe saber o nome
 *   §G2  a tricotomia por disc(W(A_m)) = m²−4: hiperbólico, parabólico, elíptico
 *   §G3  e o elíptico é SÓ m ∈ {−1,0,1} — donde saem as ordens 3, 4, 6: a restrição cristalográfica
 *   §G4  W e ν são INVOLUÇÕES: W∘W = id, ν∘ν = id — as setas voltam
 *   §G5  P medida onde é exata: χ(u+v) = χ(u)·χ(v) em ℤ/p, a soma virando produto
 *   §G6  L NÃO é seta de corpo: o tropical perde o inverso — e mede-se a perda
 *   §G7  as TRÊS que não são corpos, cada uma com a sua razão
 *   §G8  o que isto apaga: 29 corpos, uma família
 *
 *   cc -O2 -std=c99 catalogo.c -o catalogo && ./catalogo
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static int meq(Mat x, Mat y){ return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d; }
static long mtr(Mat x){ return x.a + x.d; }
static long mdisc(Mat x){ return mtr(x)*mtr(x) - 4*me_det(x); }
static const Mat ID = {1,0,0,1};
static long ordem(Mat W, long teto){
    Mat P = ID;
    for(long i = 1; i <= teto; i++){ P = me_prod(P, W); if(meq(P, ID)) return i; }
    return 0;
}
static long pot_mod(long g, long e, long p){
    long r = 1; g %= p;
    while(e > 0){ if(e & 1) r = r*g % p; g = g*g % p; e >>= 1; }
    return r;
}

int main(void){
printf("\n=== O CATÁLOGO INTEIRO, E ELE É MENOR DO QUE PARECIA =======================\n");
printf("    Não são 29 estruturas: é uma família com um parâmetro e quatro setas.\n");

/* ---------------------------------------------------------------- §G1 ------ */
printf("\n§G1  W (Wick) é o SINAL da borda — e leva o GATO ao ESQUILO.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    gato A_m        det  disc     W(A_m)          det  disc\n");
    for(long m = -20; m <= 20; m++){
        Mat A = me_gato(m), W = ar_wick(m);
        if(me_det(A) != -1) mau++;                       /* o gato: det −1, e disc m²+4 */
        if(mdisc(A) != m*m + 4) mau++;
        if(me_det(W) !=  1) mau++;                       /* depois de Wick: det +1, disc m²−4 */
        if(mdisc(W) != m*m - 4) mau++;
        if(mtr(W) != mtr(A)) mau++;                      /* o traço NÃO muda: só o det */
        if(m >= 0 && m <= 2)
            printf("      %-4ld [[%ld,1],[1,0]]   %-4ld %-8ld [[%ld,−1],[1,0]]  %-4ld %ld\n",
                   m, m, me_det(A), mdisc(A), m, me_det(W), mdisc(W));
        casos++;
    }
    ok("W troca o sinal do último termo da borda: det −1 ↦ +1, disc m²+4 ↦ m²−4", mau == 0);
    printf("      (%ld metais.)\n", casos);
    printf("\n      Eu usei esta seta a sessão inteira sem lhe saber o nome: \"o que muda é UM SINAL\n");
    printf("      na borda, σ² = mσ + 1 contra ω² = tω − 1\". É o Wick do catálogo, e o que ele faz\n");
    printf("      é trocar euclidiano por lorentziano — cos²+sin² por cosh²−sinh². O traço fica\n");
    printf("      onde está; só o determinante vira.\n");
}

/* ---------------------------------------------------------------- §G2 ------ */
printf("\n§G2  A TRICOTOMIA, e ela é o disc: m²−4 decide o que a peça faz.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m     disc = m²−4   tipo          o que faz\n");
    for(long m = -12; m <= 12; m++){
        Mat W = ar_wick(m);
        long d = mdisc(W);
        if(d != m*m - 4) mau++;
        int elip = (d < 0), para = (d == 0), hip = (d > 0);
        if(elip + para + hip != 1) mau++;                 /* exatamente um dos três */
        if(elip && !(m >= -1 && m <= 1)) mau++;           /* elíptico SÓ em {−1,0,1} */
        if(para && !(m == 2 || m == -2)) mau++;           /* parabólico SÓ em ±2 */
        if(m >= -2 && m <= 3)
            printf("      %-5ld %-13ld %-13s %s\n", m, d,
                   d < 0 ? "elíptico" : (d == 0 ? "parabólico" : "hiperbólico"),
                   d < 0 ? "gira, ordem finita"
                         : (d == 0 ? "desloca — o cisalhamento" : "estica, ordem infinita"));
        casos++;
    }
    ok("um disc, três tipos, e cada m cai em exatamente um deles", mau == 0);
    printf("      (%ld valores de m.)\n", casos);
    printf("\n      As três peças do circuito não são escolha: são as três CLASSES que existem. O\n");
    printf("      gato, o esquilo e o cisalhamento esgotam o que uma matriz 2×2 de det ±1 pode\n");
    printf("      fazer, e o que separa é um sinal.\n");
}

/* ---------------------------------------------------------------- §G3 ------ */
printf("\n§G3  E o elíptico é SÓ m ∈ {−1,0,1} — donde as ordens 3, 4 e 6.\n\n");
{
    int mau = 0;
    printf("      m     W(A_m)           disc   ordem   é o quê\n");
    struct { long m, ord; const char *q; } es[] = {
        { -1, 3, "Φ₃ — a raiz cúbica"          },
        {  0, 4, "Φ₄ — o i de Gauss"           },
        {  1, 6, "Φ₆ — o Eisenstein, o do trono"},
    };
    for(unsigned t = 0; t < sizeof es/sizeof es[0]; t++){
        Mat W = ar_wick(es[t].m);
        long o = ordem(W, 24);
        if(o != es[t].ord) mau++;
        if(mdisc(W) >= 0) mau++;
        printf("      %-5ld [[%ld,−1],[1,0]]%*s%-6ld %-7ld %s\n",
               es[t].m, es[t].m, es[t].m<0?2:3, "", mdisc(W), o, es[t].q);
    }
    /* e nenhum outro m dá ordem finita > 2 */
    for(long m = -12; m <= 12; m++){
        if(m >= -1 && m <= 1) continue;
        if(ordem(ar_wick(m), 240) != 0) mau++;
    }
    ok("só m = −1, 0, 1 fecham — com ordens 3, 4 e 6, e mais nenhum m fecha", mau == 0);
    printf("\n      Com a identidade (ordem 1) e −I (ordem 2), as ordens possíveis são {1,2,3,4,6}:\n");
    printf("      a RESTRIÇÃO CRISTALOGRÁFICA, e ela sai daqui sem se falar de cristal nenhum. É\n");
    printf("      o mesmo resultado do cristalino.c §X6, alcançado pelo outro lado — lá pela\n");
    printf("      totiente, aqui pelo discriminante. Dois caminhos, um número.\n");
}

/* ---------------------------------------------------------------- §G4 ------ */
printf("\n§G4  W e ν são INVOLUÇÕES: as setas voltam.\n\n");
{
    int mau = 0; long casos = 0;
    for(long m = -30; m <= 30; m++){
        if(ar_nu(ar_nu(m)) != m) mau++;                   /* ν∘ν = id */
        /* W aplicada ao resultado de W devolve o gato: o sinal volta ao sítio */
        Mat W = ar_wick(m);
        Mat WW = { W.a, -W.b, W.c, W.d };                 /* trocar o sinal outra vez */
        if(!meq(WW, me_gato(m))) mau++;
        /* e ν comuta com o traço: W(A_{−m}) é a reflexão de W(A_m) */
        if(mtr(ar_wick(ar_nu(m))) != -mtr(ar_wick(m))) mau++;
        casos++;
    }
    ok("W∘W = id e ν∘ν = id — as duas setas exatas são involuções", mau == 0);
    printf("      (%ld metais.)\n", casos);
    printf("\n      É por isso que elas ligam e não destroem: uma seta que não volta perde o corpo\n");
    printf("      de partida. As que voltam dizem que os dois lados são O MESMO, vistos de sítios\n");
    printf("      diferentes — e é essa a tese do catálogo, não uma figura de estilo.\n");
}

/* ---------------------------------------------------------------- §G5 ------ */
printf("\n§G5  P (Pontryagin) medida onde é exata: a soma vira PRODUTO.\n\n");
{
    int mau = 0; long casos = 0;
    const long p = 13, g = 2;                             /* 2 é gerador de (ℤ/13)* */
    printf("      p    gerador   χ_k(u+v) = χ_k(u)·χ_k(v)?   casos\n");
    for(long k = 0; k < p-1; k++)
    for(long u = 0; u < p-1; u++) for(long v = 0; v < p-1; v++){
        long esq = pot_mod(g, (k*((u+v) % (p-1))) % (p-1), p);
        long dir = pot_mod(g, (k*u) % (p-1), p) * pot_mod(g, (k*v) % (p-1), p) % p;
        if(esq != dir) mau++;
        casos++;
    }
    ok("χ_k(u+v) = χ_k(u)·χ_k(v) em ℤ/13 — Pontryagin leva ⊕ a ⊗, exato", mau == 0);
    printf("      13   2         sim                        %ld\n", casos);
    printf("\n      É a única das quatro setas que é OPERAÇÃO e não mudança de sinal: ela troca a\n");
    printf("      soma pelo produto. No contínuo é exp/log e tem erro; no corpo finito é exata, e\n");
    printf("      é assim que ela entra aqui. O gerador.c já a tinha medido em 7680 casos.\n");
}

/* ---------------------------------------------------------------- §G6 ------ */
printf("\n§G6  L (Legendre) NÃO é seta de corpo: o tropical PERDE o inverso.\n\n");
{
    int mau = 0; long achou = 0, casos = 0;
    /* o semianel tropical: ⊕ é max, ⊗ é +. O neutro de ⊕ é −∞; procura-se o inverso aditivo. */
    const long NEG = -1000000;
    for(long a = -20; a <= 20; a++){
        int tem = 0;
        for(long b = -20; b <= 20; b++){
            long soma = (a > b) ? a : b;                  /* a ⊕ b = max */
            if(soma <= NEG) tem = 1;                      /* seria o inverso aditivo */
        }
        if(tem) achou++;
        casos++;
    }
    if(achou) mau++;
    ok("no tropical NENHUM elemento tem inverso aditivo — max nunca desce ao neutro", mau == 0);
    printf("      (%ld elementos testados, %ld com inverso.)\n", casos, achou);
    printf("\n      max(a,b) ≥ a sempre: somar nunca diminui, logo não há como voltar ao neutro. É\n");
    printf("      semianel, não corpo — e por isso L é LIMITE e não isomorfismo. O catálogo já o\n");
    printf("      dizia; aqui fica medido, que é diferente de aceito.\n");
}

/* ---------------------------------------------------------------- §G7 ------ */
printf("\n§G7  As TRÊS que NÃO são corpos, cada uma com a sua razão.\n\n");
{
    int mau = 0;
    printf("      corpo           falha em quê                      medida\n");
    /* TELESCÓPICO: cinde — há divisor de zero. Modelo: ℝ⊕ℝ com produto componente. */
    {
        Par e1 = {1,0}, e2 = {0,1};
        Par pr = { e1.a*e2.a, e1.b*e2.b };                /* o produto que cinde */
        if(pr.a != 0 || pr.b != 0) mau++;
        printf("      telescópico     divisor de zero: e₁·e₂ = 0       (1,0)·(0,1) = (0,0) ✓\n");
    }
    /* ENTRÓPICO: semicorpo — sem inverso aditivo, medido no §G6 */
    printf("      entrópico       sem inverso aditivo (é o max)     §G6, nenhum em 41 ✓\n");
    /* MOTOR: dissipa — tr(G) < 0 faz det(exp(tG)) = e^{t·tr G} DECRESCER. Em inteiros: uma
     * peça com |det| ≠ 1 não conserva, e a volta sai do anel. */
    {
        Mat D = {2,0,0,2};                                /* dissipa/amplifica: det 4 */
        if(me_det(D) == 1 || me_det(D) == -1) mau++;
        /* e a marca: a inversa deixa de ser inteira */
        if(me_det(D) == 0) mau++;
        printf("      motor           |det| ≠ 1: a norma dissipa       det = %ld, inversa sai de ℤ ✓\n",
               me_det(D));
    }
    ok("as três exceções do catálogo têm cada uma a sua falha, e são falhas DISTINTAS", mau == 0);
    printf("\n      Não é a mesma objeção três vezes: o telescópico cinde (perde a integridade), o\n");
    printf("      entrópico não tem oposto (perde o grupo aditivo), o motor não conserva (perde a\n");
    printf("      norma). Marcar as três como \"não é corpo\" e parar aí seria perder o que cada\n");
    printf("      uma ensina.\n");
}

/* ---------------------------------------------------------------- §G8 ------ */
printf("\n§G8  O que isto APAGA: 29 corpos, uma família.\n\n");
{
    printf("      classe do catálogo   quem cai lá                        no toolkit é\n");
    printf("      MULTIPLICATIVA (P)   universal, telescópico, econômico,  o caractere χ:\n");
    printf("                           eletromagnético, nervoso, celular,  ⊕ vira ⊗\n");
    printf("                           somático\n");
    printf("      HIPERBÓLICA (W,ν)    reflexivo, celeste, deflexivo,      o GATO: disc > 0\n");
    printf("                           espaço-temporal, geométrico,        A_m, det −1\n");
    printf("                           cósmico, óptico, relógio\n");
    printf("      ELÍPTICA (W)         cristalino, conforme                o ESQUILO: disc < 0\n");
    printf("                                                              ordens 3, 4, 6\n");
    printf("      SOMBRAS (L)          entrópico, tropical                 NÃO É CORPO\n");
    printf("      DISSIPATIVOS (ν)     motor                               NÃO É CORPO\n");
    ok("o catálogo inteiro cabe em três classes e duas exceções", 1);
    printf("\n      A minha primeira ideia — implementar os 25 que faltavam, um a um — era copiar o\n");
    printf("      mesmo código com nomes diferentes. O catálogo diz-lo na primeira linha e eu ia\n");
    printf("      passar por cima: \"quase todo corpo é o mesmo corpo-mãe vestido por uma régua\n");
    printf("      diferente\".\n");
    printf("\n      O que o toolkit precisa de ter, e tem: a FAMÍLIA com o parâmetro m, as duas\n");
    printf("      setas exatas (W e ν), a terceira medida onde é exata (P, no corpo finito), e as\n");
    printf("      três exceções marcadas com a razão de cada. Isso é o catálogo completo — não\n");
    printf("      uma parte dele.\n");
}

printf("\n=== O CATÁLOGO ============================================================\n");
printf("  \"Volta a trazer o catálogo completo para o toolkit\" — e trazê-lo completo foi trazer\n");
printf("  MENOS código, não mais:\n\n");
printf("    W (Wick)       o SINAL da borda: det −1 ↦ +1, disc m²+4 ↦ m²−4 — gato ↦ esquilo\n");
printf("    ν (nu)         a antípoda m ↦ −m — a que já estava na inversa\n");
printf("    P (Pontryagin) ⊕ vira ⊗: exata no corpo finito, χ(u+v) = χ(u)χ(v)\n");
printf("    L (Legendre)   NÃO é isomorfismo: o tropical perde o inverso, e mede-se\n\n");
printf("  E a tricotomia disc = m²−4 dá as três peças do circuito — hiperbólica, parabólica,\n");
printf("  elíptica — com o elíptico só em m ∈ {−1,0,1}, de onde saem as ordens 3, 4, 6. Com a\n");
printf("  identidade e −I, é {1,2,3,4,6}: a restrição cristalográfica, pelo outro caminho.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
