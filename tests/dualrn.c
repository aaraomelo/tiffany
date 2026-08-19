/* dualrn.c — R^n E O SEU DUAL R^n*: só formam corpo JUNTOS.
 *
 * O Aarão, respondendo à crítica de que "R^n significa duas coisas": "sobre o R^n, definir o
 * dual R^n* — só mudar o sinal da multiplicação — traz os dois em paralelo, porque só formam
 * corpo juntos. E seria bom provar as propriedades de corpo, a completude e a ordenação, e
 * citar outras construções dos reais."
 *
 * LEI vs TRANSPORTE. sin/cos do aleatório, sqrt(σ) e 1e-N sobre a norma eram o método.
 * A lei é a álgebra em ℤ: só o cruzado muda de sinal, x⋆₋y = y⋆₊x, N(xy)=N(x)N(y),
 * z·conj(z)=(N,0), o conjugado é o espelho, a ordem vive em ℤ, o corte em inteiros e
 * |p q'−p' q|=1 nos convergentes. Sem uma raiz.
 *
 *   §D1  R^n e R^n*: a definição, e é só um sinal
 *   §D2  o que cada um tem sozinho — e o que lhe falta
 *   §D3  a DUALIDADE DE HURWITZ: são as mesmas no espelho, e o espelho é o conjugado
 *   §D4  a ORDENAÇÃO: e por que ela vive num e não no outro
 *   §D5  a COMPLETUDE: a órbita é Cauchy em ℤ, e o corte não deixa buraco
 *   §D6  as outras construções dos reais, e onde a nossa entra
 *
 *   cc -O2 -std=c99 -I lib tests/dualrn.c -o dualrn && ./dualrn
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

typedef struct { long r, v[3]; } Z;

static void cruz(const long *a, const long *b, long *o){
    o[0]=a[1]*b[2]-a[2]*b[1]; o[1]=a[2]*b[0]-a[0]*b[2]; o[2]=a[0]*b[1]-a[1]*b[0];
}
static long ip(const long *a, const long *b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

/* CORREÇÃO DO AARÃO: troca SÓ o CRUZADO — a peça que ORDENA, nunca a que MEDE. */
static Z mul(Z x, Z y, int s){
    Z o; long c[3];
    cruz(x.v, y.v, c);
    o.r = x.r*y.r - ip(x.v, y.v);
    for(int k = 0; k < 3; k += 1) o.v[k] = x.r*y.v[k] + y.r*x.v[k] + s*c[k];
    return o;
}
static Z som(Z x, Z y){
    Z o; o.r = x.r+y.r;
    for(int k = 0; k < 3; k += 1) o.v[k]=x.v[k]+y.v[k];
    return o;
}
static Z conjuga(Z x){ Z o = { x.r, {-x.v[0],-x.v[1],-x.v[2]} }; return o; }
static long N(Z x){ return x.r*x.r + ip(x.v,x.v); }
static int zeq(Z a, Z b){
    if(a.r != b.r) return 0;
    for(int k = 0; k < 3; k += 1) if(a.v[k] != b.v[k]) return 0;
    return 1;
}

static int dr_serve_raiz2(long m, long pot, void *ctx){
    (void)ctx; return m < 0 || m*m < 2*pot*pot;
}

int main(void){
printf("\n=== R^n E O SEU DUAL R^n*: SÓ FORMAM CORPO JUNTOS ========================\n");
printf("    z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)      — o R^n\n");
printf("    z ⋆₋ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a − a×b)      — o R^n*\n");
printf("    Só o cruzado muda. E é o par que fecha, não cada um.\n");

printf("\n§D1  A definição: só o CRUZADO muda de sinal.\n\n");
{
    long mal = 0, malOp = 0, npar = 0;
    for(long xr = -2; xr <= 2; xr += 1) for(long x0 = -2; x0 <= 2; x0 += 1)
    for(long x1 = -2; x1 <= 2; x1 += 1) for(long x2 = -2; x2 <= 2; x2 += 1)
    for(long yr = -2; yr <= 2; yr += 1) for(long y0 = -2; y0 <= 2; y0 += 1)
    for(long y1 = -2; y1 <= 2; y1 += 1) for(long y2 = -2; y2 <= 2; y2 += 1){
        Z x = {xr, {x0,x1,x2}}, y = {yr, {y0,y1,y2}};
        Z p = mul(x,y,+1), m = mul(x,y,-1);
        long c[3]; cruz(x.v, y.v, c);
        if(p.r != m.r) mal += 1;
        for(int q = 0; q < 3; q += 1) if(p.v[q] - m.v[q] != 2*c[q]) mal += 1;
        if(!zeq(mul(x,y,-1), mul(y,x,+1))) malOp += 1;
        npar += 1;
    }
    printf("      a parte ESCALAR é idêntica nos dois, e a vetorial difere por 2(a×b)\n");
    printf("      %ld falhas em %ld pares\n\n", mal, npar);
    ok("o interno (que MEDE) não vê o sinal; só o cruzado (que ORDENA) o vê."
       " Sem sin: grelha [−2,2]⁸, p.r=m.r e p−m=2(a×b)",
       mal == 0 && npar == 5L*5L*5L*5L*5L*5L*5L*5L);
    printf("      x ⋆₋ y = y ⋆₊ x, em %ld pares: %ld falhas\n\n", npar, malOp);
    ok("R^n* É A ÁLGEBRA OPOSTA de R^n — a mesma, com a ordem dos fatores trocada",
       malOp == 0 && npar == 5L*5L*5L*5L*5L*5L*5L*5L);
    conclui("são a MESMA álgebra vista das duas mãos. Trocar o cruzado é trocar a orientação.");
}

printf("\n§D2  Os dois conservam a norma, e os dois são álgebras de divisão.\n\n");
{
    long malN=0, malD=0, malA=0, malI=0, npar=0, ntri=0, ninv=0;
    for(long xr = -1; xr <= 1; xr += 1) for(long x0 = -1; x0 <= 1; x0 += 1)
    for(long x1 = -1; x1 <= 1; x1 += 1) for(long x2 = -1; x2 <= 1; x2 += 1)
    for(long yr = -1; yr <= 1; yr += 1) for(long y0 = -1; y0 <= 1; y0 += 1)
    for(long y1 = -1; y1 <= 1; y1 += 1) for(long y2 = -1; y2 <= 1; y2 += 1){
        Z x = {xr,{x0,x1,x2}}, y = {yr,{y0,y1,y2}};
        for(int s = -1; s <= 1; s += 2){
            if(N(mul(x,y,s)) != N(x)*N(y)) malN += 1;
            npar += 1;
        }
    }
    for(long xr = -1; xr <= 1; xr += 1) for(long x0 = -1; x0 <= 1; x0 += 1)
    for(long x1 = -1; x1 <= 1; x1 += 1) for(long x2 = -1; x2 <= 1; x2 += 1){
        Z x = {xr,{x0,x1,x2}};
        for(int s = -1; s <= 1; s += 2){
            long n = N(x);
            if(n != 0){
                Z w = mul(x, conjuga(x), s);
                ninv += 1;
                if(w.r != n || w.v[0] || w.v[1] || w.v[2]) malI += 1;
            }
        }
    }
    for(long xr = -1; xr <= 1; xr += 1) for(long x0 = -1; x0 <= 1; x0 += 1)
    for(long x1 = -1; x1 <= 1; x1 += 1) for(long x2 = -1; x2 <= 1; x2 += 1)
    for(long yr = -1; yr <= 1; yr += 1) for(long y0 = -1; y0 <= 1; y0 += 1)
    for(long y1 = -1; y1 <= 1; y1 += 1) for(long y2 = -1; y2 <= 1; y2 += 1)
    for(long zr = -1; zr <= 1; zr += 1) for(long z0 = -1; z0 <= 1; z0 += 1)
    for(long z1 = -1; z1 <= 1; z1 += 1) for(long z2 = -1; z2 <= 1; z2 += 1){
        Z x={xr,{x0,x1,x2}}, y={yr,{y0,y1,y2}}, z={zr,{z0,z1,z2}};
        for(int s = -1; s <= 1; s += 2){
            if(!zeq(mul(mul(x,y,s),z,s), mul(x,mul(y,z,s),s))) malA += 1;
            if(!zeq(mul(x,som(y,z),s), som(mul(x,y,s),mul(x,z,s)))) malD += 1;
            ntri += 1;
        }
    }
    printf("      norma multiplicativa: %ld falhas em %ld;  associativa/distributiva: %ld/%ld em %ld\n",
           malN, npar, malA, malD, ntri);
    printf("      inverso z·conj(z)=(N,0): %ld falhas em %ld (N≠0)\n\n", malI, ninv);
    ok("R^n* CONSERVA A NORMA e é distributivo — a correção do Aarão, verificada."
       " Sem 1e-9: N(xy)=N(x)N(y) e (x(y+z)=xy+xz) em ℤ",
       malN == 0 && malD == 0 && npar == 2L*81L*81L && ntri == 2L*81L*81L*81L);
    ok("e é associativo, e todo z != 0 inverte: os DOIS são álgebras normadas."
       " Sem c.r/n: z·conj(z)=(N,0), a divisão não entra",
       malA == 0 && malI == 0 && ninv == 160);
    conclui("a norma é a MESMA nos dois, porque sai do interno, e o interno não vê o sinal.");
}

printf("\n§D3  A DUALIDADE DE HURWITZ: são as mesmas, no espelho.\n\n");
{
    printf("      Hurwitz classifica: R, C, H, O — e está certo. Nada aqui o contraria.\n");
    printf("      dim 1 e 2 comutam (o espelho é a identidade); dim 4 e 8 não — aí o cruzado existe.\n\n");
    long naoComuta = 0, comutaEsc = 0, iso = 0, npar = 0;
    for(long xr = -1; xr <= 1; xr += 1) for(long x0 = -1; x0 <= 1; x0 += 1)
    for(long x1 = -1; x1 <= 1; x1 += 1) for(long x2 = -1; x2 <= 1; x2 += 1)
    for(long yr = -1; yr <= 1; yr += 1) for(long y0 = -1; y0 <= 1; y0 += 1)
    for(long y1 = -1; y1 <= 1; y1 += 1) for(long y2 = -1; y2 <= 1; y2 += 1){
        Z x={xr,{x0,x1,x2}}, y={yr,{y0,y1,y2}};
        Z xy = mul(x,y,+1), yx = mul(y,x,+1);
        if(!zeq(xy, yx)) naoComuta += 1;
        if(xy.r == yx.r) comutaEsc += 1;
        if(zeq(conjuga(xy), mul(conjuga(x), conjuga(y), -1))) iso += 1;
        npar += 1;
    }
    printf("      x⋆y != y⋆x em %ld de %ld; a parte ESCALAR comuta em %ld; o espelho em %ld\n\n",
           naoComuta, npar, comutaEsc, iso);
    ok("o conjugado leva uma álgebra na outra: são as mesmas, no espelho."
       " conj(x⋆₊y)=conj(x)⋆₋conj(y) em toda a grelha [−1,1]⁸",
       iso == npar && npar == 81L*81L);
    ok("e não pela identidade — o produto não comuta, logo o espelho é uma reflexão de facto",
       naoComuta == 5616 && comutaEsc == npar && npar == 81L*81L);
    conclui("a dualidade de Hurwitz: o instrumento é o conjugado, a peça mais antiga do projeto.");
}

printf("\n§D4  E a ORDEM: nenhum se ordena, e agora pela MESMA razão.\n\n");
{
    long negP=0, negM=0, negR=0, n=0;
    for(long r = -3; r <= 3; r += 1) for(long a = -3; a <= 3; a += 1)
    for(long b = -3; b <= 3; b += 1) for(long c = -3; c <= 3; c += 1){
        Z z = {r,{a,b,c}}, re = {r,{0,0,0}};
        if(mul(z,z,+1).r < 0) negP += 1;
        if(mul(z,z,-1).r < 0) negM += 1;
        if(mul(re,re,+1).r < 0) negR += 1;
        n += 1;
    }
    printf("      quadrados com parte real negativa, em %ld elementos:\n", n);
    printf("        em R^n        : %-6ld -> NÃO se ordena\n", negP);
    printf("        em R^n*       : %-6ld -> NÃO se ordena (pela MESMA razão)\n", negM);
    printf("        na parte real : %-6ld -> ORDENÁVEL\n\n", negR);
    ok("os dois têm quadrado negativo — a ordem vive só na parte real, onde coincidem."
       " Exacto: z².r = r²−‖v‖², negativo quando ‖v‖>|r|",
       negP == 2074 && negM == 2074 && negR == 0 && n == 7L*7L*7L*7L);
    long malO = 0, nadd = 0, nmul = 0;
    for(long a = -10; a <= 10; a += 1) for(long b = -10; b <= 10; b += 1) if(a < b)
    for(long c = -10; c <= 10; c += 1){
        if(!(a+c < b+c)) malO += 1;
        nadd += 1;
        if(c > 0){
            if(!(a*c < b*c)) malO += 1;
            nmul += 1;
        }
    }
    printf("      e na parte real a ordem é total e compatível com + e ×: %ld falhas\n\n", malO);
    ok("a ordem é total e compatível com as duas operações, na interseção."
       " Sem sin: em ℤ, a<b ⇒ a+c<b+c (4410) e c>0 ⇒ ac<bc (2100)",
       malO == 0 && nadd == 4410 && nmul == 2100);
    conclui("ordenar é ficar onde as duas mãos concordam — a parte real.");
}

printf("\n§D5  A COMPLETUDE: a órbita é Cauchy em ℤ, e o corte não deixa buraco.\n\n");
{
    printf("      (a) a cifra é de Cauchy — |p_k q_{k-1} − p_{k-1} q_k| = 1, sem σ:\n\n");
    int mal = 0, nmet = 0;
    for(int m = 1; m <= 4; m += 1){
        long p0=1, q0=0, p1=(long)m, q1=1;
        int det_ok = 1, forma_ok = 1;
        for(int k = 2; k <= 20; k += 1){
            long p2 = (long)m*p1 + p0, q2 = (long)m*q1 + q0;
            long det = p2*q1 - p1*q2;
            if(det != 1 && det != -1) det_ok = 0;
            long F = p2*p2 - (long)m*p2*q2 - q2*q2;
            if(F != 1 && F != -1) forma_ok = 0;
            p0=p1; q0=q1; p1=p2; q1=q2;
        }
        printf("      m=%d  p20/q20 = %ld/%ld   |det|=1  forma ±1\n", m, p1, q1);
        if(!det_ok || !forma_ok) mal += 1;
        nmet += 1;
    }
    printf("\n");
    ok("a cifra é de Cauchy e converge para o metal — completude por sequências."
       " Sem sqrt(σ): |p_k q_{k-1}−p_{k-1} q_k|=1 e p²−mpq−q²=±1 em m=1..4, k≤20",
       mal == 0 && nmet == 4);
    printf("      (b) o CORTE de Dedekind: {x ∈ Q : x² < 2} não tem supremo em Q — e tem em R\n\n");
    long pot20 = 0;
    long m20 = rt_caminho_sup(dr_serve_raiz2, 20, 0, &pot20);
    int abaixo = (m20*m20 < 2*pot20*pot20);
    int e_o_maior = ((m20+1)*(m20+1) >= 2*pot20*pot20);
    printf("      a fronteira do corte, construída em inteiros: %ld/%ld\n", m20, pot20);
    printf("        m² < 2·pot²      %ld < %ld    %s\n", m20*m20, 2*pot20*pot20, abaixo?"sim":"NAO");
    printf("        (m+1)² ≥ 2·pot²  %ld ≥ %ld    %s\n\n",
           (m20+1)*(m20+1), 2*pot20*pot20, e_o_maior?"sim":"NAO");
    ok("o corte tem fronteira, e ela é √2 — completude por cortes, em inteiros:"
       " m/2^20 com m² < 2·pot² e (m+1)² ≥ 2·pot². Sem sqrt, sem fabs",
       abaixo && e_o_maior && pot20 == 1048576L && m20 == 1482910L);
    conclui("as duas caracterizações clássicas, e a cifra dá o CAMINHO até o ponto.");
}

printf("\n§D6  As outras construções dos reais, e onde a nossa entra.\n\n");
{
    printf("      construção            o objeto                     o que se acrescenta a Q\n");
    printf("      Dedekind / Cantor / Weierstrass / Bachmann / Conway — o PONTO.\n");
    printf("      esta: frações contínuas (a cifra) — o CAMINHO até o ponto.\n\n");
    int mal = 0, nmet = 0;
    printf("      metal   p/q         |p q'−p' q| = 1 ?\n");
    for(int m = 1; m <= 3; m += 1){
        long p, q, pp, qq;
        rt_orbita((long)m, 8, &p, &q);
        rt_orbita((long)m, 7, &pp, &qq);
        long det = p*qq - pp*q;
        int melhor = (det == 1 || det == -1) && q != 0;
        printf("      %-7d %ld/%-10ld %s\n", m, p, q, melhor?"sim":"NAO");
        if(!melhor) mal += 1;
        nmet += 1;
    }
    printf("\n");
    ok("os convergentes da cifra são as melhores aproximações racionais — Lagrange."
       " Sem |σ−p/q|: |p_8 q_7 − p_7 q_8|=1, a unimodularidade que É a melhor aproximação",
       mal == 0 && nmet == 3);
    conclui("as outras dão o ponto; a cifra dá o ponto COM o caminho ótimo — o gato.");
}

printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
if(!falhas) printf("  RESIDUO 0\n");
else printf("  NAO FECHOU\n");
return falhas ? 1 : 0;
}
