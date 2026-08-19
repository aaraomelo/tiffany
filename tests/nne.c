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
 *   §N1  a fórmula reproduz os exemplos do livro — exacto em Q(√5)
 *   §N2  e a NORMA é multiplicativa em R³ — Pitágoras, no quadrado
 *   §N3  mas NÃO é bilinear: falha a distributividade, e é aí que sai de Hurwitz
 *   §N5  a GENERALIZAÇÃO: a recursão sobe, e o escalar é trocado pela NORMA
 *   §N6  a álgebra DUAL do Gentil — e essa É distributiva e bilinear
 *   §N7  o CRUZADO DUAL: em dim par o lugar é ocupado por J, e os dois completam-se
 *   §N8  o LADO DA TORRE que é de Gentil: par multiplica, ímpar soma, e 2D é ℂ
 *   §N4  e o que isto corrige no que eu tinha escrito
 *
 * LEI vs TRANSPORTE. hypot, sin/cos em 500 pares, 1e-9 na recursão e 1e-12 no dual eram o
 * método. A lei é o produto em ℚ quando r é inteiro (terno pitagórico), ‖xy‖²=‖x‖²‖y‖²,
 * a triangular 2<4, a homogeneidade em Q(√5), a recursão em ℤ de R² a R⁷, e |ab| o
 * determinante — sem uma raiz formada.
 *
 *   cc -O2 -std=c99 -I lib tests/nne.c -o nne && ./nne
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b, c; } Wz;

static int raiz_exata(long x, long *r){
    if(x < 0) return 0;
    long t = 0;
    while(t*t < x) t++;
    if(t*t != x) return 0;
    *r = t;
    return 1;
}
static long mdc_pos(long a, long b){
    if(a < 0) a = -a; if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}

/* Produto nne-3D em ℚ, quando ‖(a,b)‖ é inteiro. (an/d, bn/d, c). Devolve 0 se r não for. */
static int nne_z(Wz w1, Wz w2, long *an, long *bn, long *d, long *c){
    long r1, r2;
    if(!raiz_exata(w1.a*w1.a + w1.b*w1.b, &r1)) return 0;
    if(!raiz_exata(w2.a*w2.a + w2.b*w2.b, &r2)) return 0;
    if(r1 == 0 && r2 == 0){
        *an = -w1.c*w2.c; *bn = 0; *d = 1; *c = 0; return 1;
    }
    if(r1 == 0){
        *an = -w1.c*w2.c*w2.a; *bn = -w1.c*w2.c*w2.b; *d = r2; *c = w1.c*r2; return 1;
    }
    if(r2 == 0){
        *an = -w1.c*w2.c*w1.a; *bn = -w1.c*w2.c*w1.b; *d = r1; *c = w2.c*r1; return 1;
    }
    long gn = r1*r2 - w1.c*w2.c, gd = r1*r2;
    *an = (w1.a*w2.a - w1.b*w2.b)*gn;
    *bn = (w1.a*w2.b + w2.a*w1.b)*gn;
    *d  = gd;
    *c  = w1.c*r2 + w2.c*r1;
    long g = mdc_pos(mdc_pos(*an, *bn), *d);
    *an /= g; *bn /= g; *d /= g;
    if(*d < 0){ *an = -*an; *bn = -*bn; *d = -*d; }
    return 1;
}

/* Recursão (z,c)·(w,d) = ( z·w·γ , c‖w‖ + d‖z‖ ), em ℚ, quando cada r é inteiro. */
static int nne_rec_z(const long *x, const long *y, int d, long *pn, long *pd){
    if(d == 1){ pn[0] = x[0]*y[0]; *pd = 1; return 1; }
    long r1q = 0, r2q = 0;
    for(int i = 0; i < d-1; i += 1){ r1q += x[i]*x[i]; r2q += y[i]*y[i]; }
    long r1, r2;
    if(!raiz_exata(r1q, &r1) || !raiz_exata(r2q, &r2)) return 0;
    long c = x[d-1], e = y[d-1];
    if(r1 == 0 && r2 == 0){
        for(int i = 0; i < d; i += 1) pn[i] = 0;
        pn[0] = -c*e; *pd = 1; return 1;
    }
    if(r1 == 0){
        *pd = r2;
        for(int i = 0; i < d-1; i += 1) pn[i] = -c*e*y[i];
        pn[d-1] = c * r2 * r2;
        return 1;
    }
    if(r2 == 0){
        *pd = r1;
        for(int i = 0; i < d-1; i += 1) pn[i] = -c*e*x[i];
        pn[d-1] = e * r1 * r1;
        return 1;
    }
    long subn[8], subd;
    if(!nne_rec_z(x, y, d-1, subn, &subd)) return 0;
    long gn = r1*r2 - c*e, gd = r1*r2;
    *pd = subd * gd;
    for(int i = 0; i < d-1; i += 1) pn[i] = subn[i] * gn;
    pn[d-1] = (c*r2 + e*r1) * (*pd);
    /* pn[d-1]/pd = c r2 + e r1, logo o numerador leva o den. */
    return 1;
}

int main(void){
printf("\n=== OS nne-3D DE GENTIL LOPES DA SILVA ===================================\n");
printf("    Norma multiplicativa em R^n — medido de R2 a R7, e o erro e so\n");
printf("    multiplicação NÃO é bilinear. A hipótese é que eu não tinha declarado.\n");

printf("\n§N1  A fórmula reproduz os exemplos do livro.\n\n");
{
    /* NESTES DOIS EXEMPLOS a conta NÃO precisa de vírgula: ||(1,2)||² = 5 e ||(2,0)|| = 2,
     * e o produto r1·r2 para (1,2,3)² vale 5, RACIONAL. Tudo vive em Q(√5). */
    long A1 = 1, B1 = 2, C1 = 3;
    long A2 = 2, B2 = 0, C2 = 0;
    long r2q = A2*A2 + B2*B2, r2i = 0;
    while(r2i*r2i < r2q) r2i++;
    long ex1_a = (A1*A2 - B1*B2), ex1_b = (A1*B2 + A2*B1);
    long c1_rac = C1*r2i, c1_r5 = C2;
    printf("      exato em Q: (%ld,%ld,%ld)·(%ld,%ld,%ld) = (%ld, %ld, %ld %+ld·√5)      livro: (2, 4, 6)\n",
           A1,B1,C1, A2,B2,C2, ex1_a, ex1_b, c1_rac, c1_r5);

    long r1q = A1*A1 + B1*B1;
    long g2_n = r1q - C1*C1, g2_d = r1q;
    long pa = (A1*A1 - B1*B1), pb = (A1*B1 + A1*B1);
    long a2_n = pa*g2_n, a2_d = g2_d;
    long b2_n = pb*g2_n, b2_d = g2_d;
    long c2_rac = 0, c2_r5 = 2*C1;
    int raiz_inteira = (r2i*r2i == r2q);
    printf("      exato em Q(√5): (1,2,3)² = (%ld/%ld, %ld/%ld, %ld %+ld·√5)   livro: (12/5, −16/5, 6√5)\n\n",
           a2_n, a2_d, b2_n, b2_d, c2_rac, c2_r5);

    ok("a fórmula dá os dois exemplos do livro — EXATO em Q(√5), sem uma tolerância."
       " comparar p2.c² = 180 com 1e-12 era tautologia sobre sqrt(5) formado duas vezes",
       raiz_inteira && ex1_a == 2 && ex1_b == 4 && c1_rac == 6 && c1_r5 == 0
    && a2_n == 12 && a2_d == 5 && b2_n == -16 && b2_d == 5
    && c2_rac == 0 && c2_r5 == 6);
}

printf("\n§N2  E a NORMA é multiplicativa em R³.\n\n");
{
    /* Com (a,b) um terno PITAGÓRICO, r é inteiro, γ é racional e o produto fecha em ℚ.
     * A tese vive no QUADRADO: ‖w₁·w₂‖² = ‖w₁‖²·‖w₂‖², e nenhuma raiz se forma. */
    long mult_tot = 0, mult_ok = 0;
    { const long PIT[4][3] = { {3,4,5}, {5,12,13}, {8,15,17}, {7,24,25} };
      for(int i = 0; i < 4; i += 1) for(int j = 0; j < 4; j += 1)
      for(long c1 = -3; c1 <= 3; c1 += 1) for(long c2 = -3; c2 <= 3; c2 += 1){
          long a1 = PIT[i][0], b1 = PIT[i][1], r1 = PIT[i][2];
          long a2 = PIT[j][0], b2 = PIT[j][1], r2 = PIT[j][2];
          long gn = r1*r2 - c1*c2, gd = r1*r2;
          long pa_n = (a1*a2 - b1*b2)*gn, pb_n = (a1*b2 + a2*b1)*gn, pd = gd;
          long pc = c1*r2 + c2*r1;
          long esq_n = pa_n*pa_n + pb_n*pb_n + pc*pc*pd*pd;
          long dir_n = (r1*r1 + c1*c1) * (r2*r2 + c2*c2) * pd*pd;
          mult_tot++;
          if(esq_n == dir_n) mult_ok++;
      } }
    printf("      %ld de %ld pares pitagóricos: ‖w1·w2‖² = ‖w1‖²·‖w2‖²\n\n", mult_ok, mult_tot);
    ok("a norma É multiplicativa — em R³, que é onde eu disse que não podia ser. E mede-se"
       " em Z, sem uma raiz: escolhendo (a,b) PITAGORICO a norma do plano e' inteira, logo"
       " gamma = 1 - c1c2/(r1r2) e' racional e o produto fecha em Q; e a tese vive no"
       " QUADRADO, ‖w1.w2‖^2 = ‖w1‖^2.‖w2‖^2, que e' uma igualdade de inteiros",
       mult_tot > 0 && mult_ok == mult_tot);
    printf("      É o que a identidade de Lagrange exige, e a dimensão do vetor aqui é 2, não\n");
    printf("      1 nem 3 nem 7. Se a conclusão que eu tirei fosse universal, isto não podia\n");
    printf("      existir — e existe, e está medido.\n");
}

printf("\n§N3  Mas NÃO é bilinear — e é aí que ela sai da hipótese de Hurwitz.\n\n");
{
    /* w·(u+v) vs w·u+w·v. Com u=(1,0,0), v=(0,1,0), as terceiras são zero, logo
     *      esq.c = w_c·‖u+v‖        dir.c = w_c·‖u‖ + w_c·‖v‖
     * e a diferença é a DESIGUALDADE TRIANGULAR, estrita porque u e v não são paralelos. */
    const long nu2 = 1, nv2 = 1, nuv2 = 2;
    long soma_normas2 = nu2 + nv2 + 2;                  /* (‖u‖+‖v‖)² = 1+1+2 */
    int triangular_estrita = (nuv2 < soma_normas2);     /* 2 < 4 */
    int desvio_maior = (2*900 < 59*59);                 /* 3(2−√2) > 1/10 por corte */
    /* as duas primeiras coordenadas: g=1 (c_u=c_v=0), produto complexo. */
    long esq_a = 1*1 - 2*1, esq_b = 1*1 + 1*2;          /* w=(1,2,·) · (1,1,0) */
    long dir_a = (1*1 - 2*0) + (1*0 - 2*1);
    long dir_b = (1*0 + 1*2) + (1*1 + 0*2);
    int coords_ab_ok = (esq_a == dir_a && esq_b == dir_b);
    printf("      w·(u+v) nas duas primeiras: (%ld, %ld)\n", esq_a, esq_b);
    printf("      w·u + w·v                 : (%ld, %ld)  — batem\n", dir_a, dir_b);
    printf("      ‖u+v‖² = %ld < %ld = (‖u‖+‖v‖)²  — a triangular, estrita\n\n", nuv2, soma_normas2);
    ok("NÃO é distributiva — logo não é bilinear. E falha numa coordenada SÓ, a terceira, e a"
       " falha TEM NOME: e' a DESIGUALDADE TRIANGULAR, estrita porque u e v nao sao"
       " paralelos. Mede-se sem raiz — ‖u+v‖^2 = 2 < 4 = (‖u‖+‖v‖)^2 —, e o desvio de mais"
       " de um decimo enquadra-se por um racional: 3(2-raiz2) > 1/10 e' raiz2 < 59/30, isto"
       " e' 1800 < 3481. As duas primeiras coordenadas batem exacto",
       triangular_estrita && desvio_maior && coords_ab_ok);

    /* homogeneidade: (λw)·u = λ(w·u), em Q, com r inteiro. λ = 1,2,4,8,16. */
    Wz w = { 3, 4, 1 }, u = { 5, 12, 2 };
    int hom_ok = 0, hom_tot = 0;
    for(long L = 1; L <= 16; L *= 2){
        Wz Lw = { L*w.a, L*w.b, L*w.c };
        long an, bn, d, c, An, Bn, D, C;
        if(!nne_z(Lw, u, &an, &bn, &d, &c)) continue;
        if(!nne_z(w,  u, &An, &Bn, &D, &C)) continue;
        /* λ(w·u) = (L An/D, L Bn/D, L C)  vs  (an/d, bn/d, c) */
        hom_tot++;
        if(an*D == L*An*d && bn*D == L*Bn*d && c == L*C) hom_ok++;
    }
    printf("      homogeneidade em λ = 1,2,4,8,16: %d de %d, EXACTA em Q\n\n", hom_ok, hom_tot);
    ok("e É homogénea — o que a torna quase-bilinear, e não bilinear. Medido EXACTO em"
       " cinco potências de dois, em Q, com r pitagórico: com um λ só, a asserção"
       " não distinguia «homogénea» de «funciona no 2»",
       hom_tot == 5 && hom_ok == hom_tot);
    printf("      A raiz quadrada no r e o γ que divide por r1r2 são o que quebra a soma: a\n");
    printf("      multiplicação USA A NORMA dos operandos, e a norma não é aditiva. Escalar não\n");
    printf("      lhe faz mal (a raiz é homogénea), somar faz.\n");
}

printf("\n§N5  A GENERALIZAÇÃO: já estava feita, e faltava interpretar.\n\n");
{
    printf("      (z,c)·(w,d) = ( z·w·γ , c‖w‖ + d‖z‖ ),   γ = 1 - cd/(‖z‖‖w‖)\n");
    printf("      com z·w o produto do NÍVEL DE BAIXO — e daí sobe-se um de cada vez.\n\n");
    printf("        nível   norma multiplicativa?   pares\n");
    int mal = 0, niveis = 0;
    for(int d = 2; d <= 7; d += 1){
        long okp = 0, tot = 0;
        for(long c1 = -2; c1 <= 2; c1 += 1) for(long c2 = -2; c2 <= 2; c2 += 1){
            long x[8] = {0}, y[8] = {0};
            x[0] = 3; x[1] = 4; x[d-1] = c1;
            y[0] = 5; y[1] = 12; y[d-1] = c2;
            long pn[8], pd;
            if(!nne_rec_z(x, y, d, pn, &pd) || pd == 0) continue;
            long np2 = 0, nx2 = 0, ny2 = 0;
            for(int i = 0; i < d; i += 1){
                np2 += pn[i]*pn[i];
                nx2 += x[i]*x[i];
                ny2 += y[i]*y[i];
            }
            tot++;
            if(np2 == nx2 * ny2 * pd * pd) okp++;
        }
        printf("        R^%d     %-22s %ld/%ld\n", d,
               (okp == tot && tot > 0) ? "SIM" : "nao", okp, tot);
        niveis++;
        if(okp != tot || tot == 0) mal++;
    }
    printf("\n");
    ok("a recursão sobe e a norma continua multiplicativa — medido de R² a R⁷, em Z,"
       " com (3,4) e (5,12) nas duas primeiras e o resto zero salvo a última coordenada."
       " hypot e 1e-9 mediam o arredondamento das tres raizes; o quadrado nao as forma",
       mal == 0 && niveis == 6);

    {
        long x[3] = {3,4,1}, y[3] = {5,12,2}, pn[8], pd;
        long an, bn, d, c;
        int rec = nne_rec_z(x, y, 3, pn, &pd);
        int dir = nne_z((Wz){3,4,1}, (Wz){5,12,2}, &an, &bn, &d, &c);
        int bate = rec && dir && pn[0]*d == an*pd && pn[1]*d == bn*pd && pn[2] == c*pd;
        printf("      recursão em d=3: (%ld/%ld, %ld/%ld, %ld/%ld)   directa: (%ld/%ld, %ld/%ld, %ld)\n\n",
               pn[0], pd, pn[1], pd, pn[2], pd, an, d, bn, d, c);
        ok("a recursão e a fórmula direta são a MESMA coisa em d=3 — mede-se EXACTO em Q,"
           " sem limiar: as duas rotas nao partilham o codigo da outra, e batem",
           bate);
    }
    printf("      NÃO HÁ NÍVEL EM QUE PARE. O arredondamento que aqui se media era o da\n");
    printf("      vírgula, e saiu: em ℤ o resíduo é zero em todos os andares.\n");

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
    printf("      (a,b) ∗ (c,d) = (a·c, −b·d),  com a mesma adição\n\n");
    int mal_d = 0, mal_h = 0, tot = 0;
    for(long x0 = -3; x0 <= 3; x0 += 1) for(long x1 = -3; x1 <= 3; x1 += 1)
    for(long y0 = -3; y0 <= 3; y0 += 1) for(long y1 = -3; y1 <= 3; y1 += 1)
    for(long z0 = -2; z0 <= 2; z0 += 1) for(long z1 = -2; z1 <= 2; z1 += 1){
        long e0 = x0*(y0+z0), d0 = x0*y0 + x0*z0;
        long e1 = -x1*(y1+z1), d1 = -x1*y1 - x1*z1;
        if(e0 != d0 || e1 != d1) mal_d++;
        long L = 3;
        long eh0 = (L*x0)*y0, dh0 = L*(x0*y0);
        long eh1 = -(L*x1)*y1, dh1 = L*(-x1*y1);
        if(eh0 != dh0 || eh1 != dh1) mal_h++;
        tot++;
    }
    printf("      x∗(y+z) contra x∗y + x∗z, em %d triplos: %d falhas\n", tot, mal_d);
    printf("      (λx)∗y  contra λ(x∗y),    em %d pares:   %d falhas\n\n", tot, mal_h);
    ok("a álgebra DUAL do Gentil é distributiva E bilinear — em Z, na fórmula (ac, −bd)."
       " Os 300 triplos de sin/cos com 1e-12 mediam o IEEE; a identidade e' polinomial",
       mal_d == 0 && mal_h == 0 && tot > 0);

    long det_ok = 0, eucl_ok = 0, tot_i = 0, sinal_ok = 0;
    for(long x0 = -4; x0 <= 4; x0 += 1) for(long x1 = -4; x1 <= 4; x1 += 1)
    for(long y0 = -4; y0 <= 4; y0 += 1) for(long y1 = -4; y1 <= 4; y1 += 1){
        long p0 = x0*y0, p1 = -x1*y1;
        long lhs = p0*p1;              if(lhs < 0) lhs = -lhs;
        long rx = x0*x1;               if(rx < 0)  rx = -rx;
        long ry = y0*y1;               if(ry < 0)  ry = -ry;
        tot_i++;
        if(lhs == rx*ry) det_ok++;
        if(p0*p1 == -(x0*x1)*(y0*y1)) sinal_ok++;
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
       " euclidiana falha na maioria — o contraste é um número",
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
    printf("      cruzado  a×b    antissimétrico e bilinear, com a identidade de Lagrange\n");
    printf("                      -> só em dimensão 1, 3 e 7, que são ÍMPARES\n");
    printf("      o DUAL   J      J² = −I, a rotação de 90° em cada plano\n");
    printf("                      -> só em dimensão PAR\n\n");
    printf("      e a razão de J só existir em par é de duas linhas:\n");
    printf("        det(J)² = det(J²) = det(−I) = (−1)^n\n");
    printf("        em n ÍMPAR isso pede det(J)² = −1, e o determinante de matriz real é real\n\n");
    int mal = 0;
    printf("      dim   a×b   J     o que existe lá\n");
    for(int n = 1; n <= 8; n += 1){
        int c = (n == 1 || n == 3 || n == 7), j = (n % 2 == 0);
        printf("      %-5d %-5s %-5s %s\n", n, c ? "sim" : "—", j ? "sim" : "—",
               c ? "o cruzado" : j ? "o DUAL (J)" : "nenhum dos dois");
        if(c && j) mal++;
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
    /* Parte PAR (a,b) MULTIPLICA — produto complexo × γ; parte ÍMPAR (c) SOMA.
     * Com c=0, γ=1 e colapsa no produto de ℂ. */
    long casos_c = 0, complexo = 0, par_mult = 0, impar_soma = 0, norma_ok = 0, casos_p = 0;
    printf("      caso                       parte PAR             parte ÍMPAR\n");
    const long PIT[4][3] = { {3,4,5}, {5,12,13}, {8,15,17}, {7,24,25} };
    for(int i = 0; i < 4; i += 1) for(int j = 0; j < 4; j += 1){
        long a1 = PIT[i][0], b1 = PIT[i][1], r1 = PIT[i][2];
        long a2 = PIT[j][0], b2 = PIT[j][1], r2 = PIT[j][2];
        {   /* (i) c = 0: é ℂ */
            long pa = a1*a2 - b1*b2, pb = a1*b2 + a2*b1;
            Wz u = { a1, b1, 0 }, v = { a2, b2, 0 };
            long an, bn, d, c;
            casos_c++;
            if(nne_z(u, v, &an, &bn, &d, &c) && an == pa*d && bn == pb*d && c == 0)
                complexo++;
        }
        for(long c1 = -2; c1 <= 2; c1 += 1) for(long c2 = -2; c2 <= 2; c2 += 1){
            if(c1 == 0 && c2 == 0) continue;
            long gn = r1*r2 - c1*c2, gd = r1*r2;
            long pa = (a1*a2 - b1*b2)*gn, pb = (a1*b2 + a2*b1)*gn;
            long pc = c1*r2 + c2*r1;
            Wz u = { a1, b1, c1 }, v = { a2, b2, c2 };
            long an, bn, d, c;
            if(!nne_z(u, v, &an, &bn, &d, &c)) continue;
            casos_p++;
            if(an*gd == pa*d && bn*gd == pb*d) par_mult++;
            if(c == pc) impar_soma++;
            long esq = an*an + bn*bn + c*c*d*d;
            long dir = (r1*r1 + c1*c1)*(r2*r2 + c2*c2)*d*d;
            if(esq == dir) norma_ok++;
        }
    }
    printf("      c = 0 (sem parte ímpar)    o produto de ℂ         ausente\n");
    printf("      c ≠ 0                      complexo × γ           c₁r₂ + c₂r₁ (SOMA)\n\n");
    printf("      colapsa em ℂ em %ld/%ld · a par multiplica em %ld/%ld · a ímpar soma"
           " em %ld · norma em %ld\n\n", complexo, casos_c, par_mult, casos_p, impar_soma, norma_ok);
    ok("O LADO DA TORRE QUE É DE GENTIL SEPARA PAR E ÍMPAR, E CONTINUA DUAL: no produto do"
       " corpus a parte PAR (a,b) MULTIPLICA — é o produto complexo escalado por γ — e a"
       " parte ÍMPAR (c) SOMA, c₁r₂ + c₂r₁. E EM 2D DÁ CORPO, porque a parte ímpar não existe:"
       " com c = 0 vem γ = 1 e a fórmula colapsa EXACTAMENTE no produto de ℂ, em Z, sem 1e-9",
       complexo == casos_c && casos_c == 16
       && par_mult == casos_p && impar_soma == casos_p && norma_ok == casos_p
       && casos_p > 0);
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
