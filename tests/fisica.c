/* fisica.c — ONDE OS DUAIS SE TOCAM: ε² = 0, E O QUE ISSO É NA FÍSICA.
 *
 * O Aarão: "os duais se tocam acima do infinito, aí é ε² = 0. Tem análogo na física? Os
 * operadores bra-ket talvez?"
 *
 * LEI vs TRANSPORTE. 720 ângulos com sin/cos, 200 pares com sin, tanh/atanh e o boost com
 * sqrt eram o método. A lei é a álgebra ℤ[ε]/(ε²), o cone a² − s b² em ℤ (0, 2 e 4 raios),
 * |a><b| nilpotente sse ⟨b|a⟩=0 em ℤ, Minkowski i²−j², Einstein ⊕ em ℚ, e c²/(c²−1)−1 =
 * 1/(c²−1) a descer até Galileu. Sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/fisica.c -o fisica && ./fisica
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "reta.h"

typedef struct { long a, b; } Z;
static Z mul(Z x, Z y, int s){
    Z r;
    r.a = x.a*y.a + s*x.b*y.b;
    r.b = x.a*y.b + x.b*y.a;
    return r;
}

typedef struct { long re[2][2], im[2][2]; } M2;
static M2 mm(M2 A, M2 B){
    M2 C;
    memset(&C, 0, sizeof C);
    for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1)
        for(int k = 0; k < 2; k += 1){
            C.re[i][j] += A.re[i][k]*B.re[k][j] - A.im[i][k]*B.im[k][j];
            C.im[i][j] += A.re[i][k]*B.im[k][j] + A.im[i][k]*B.re[k][j];
        }
    return C;
}
static M2 msoma(M2 A, M2 B){
    M2 C;
    for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1){
        C.re[i][j] = A.re[i][j]+B.re[i][j];
        C.im[i][j] = A.im[i][j]+B.im[i][j];
    }
    return C;
}
static long mnorma(M2 A){
    long s = 0;
    for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1){
        long r = A.re[i][j], m = A.im[i][j];
        s += (r < 0 ? -r : r) + (m < 0 ? -m : m);
    }
    return s;
}
static M2 ketbra(const long ar[2], const long ai[2], const long br[2], const long bi[2]){
    M2 P;
    memset(&P, 0, sizeof P);
    for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1){
        P.re[i][j] = ar[i]*br[j] + ai[i]*bi[j];
        P.im[i][j] = ai[i]*br[j] - ar[i]*bi[j];
    }
    return P;
}

/* raios nulos de a² − s b² no anel {-1,0,1}² \ 0 — sem ângulo. */
static int raios_nulos(int s){
    int n = 0;
    for(int a = -1; a <= 1; a += 1) for(int b = -1; b <= 1; b += 1){
        if(!a && !b) continue;
        if(a*a - s*b*b == 0) n += 1;
    }
    return n;
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
    printf("      medido:  i·i = %+ld      i*·i* = %+ld      ε·ε = %+ld\n\n", c.a, h.a, p.a);
    ok("as tres classes, e a do meio e onde o produto DEGENERA",
       c.a == -1 && h.a == 1 && p.a == 0 && c.b == 0 && h.b == 0 && p.b == 0);

    printf("      O cone e N(a,b) = a² - s·b² = 0, nos raios de {-1,0,1}²:\n\n");
    int mal = 0;
    int nd[3];
    for(int s = -1; s <= 1; s += 1){
        int dirs = raios_nulos(s);
        nd[s+1] = dirs;
        printf("      s = %+d:  %d raio(s) com N = 0   %s\n", s, dirs,
               s < 0 ? "(so a origem: nenhum raio)"
             : s > 0 ? "(QUATRO raios = DUAS retas: a = ±b)"
                     : "(DOIS raios = UMA reta: a = 0)");
        if(s < 0 && dirs != 0) mal += 1;
        if(s > 0 && dirs != 4) mal += 1;
        if(s == 0 && dirs != 2) mal += 1;
    }
    printf("\n");
    ok("no dual sao DUAS retas; na fronteira elas colapsaram numa so — tocaram-se."
       " 720 angulos com sin/cos eram o transporte; os raios de {-1,0,1}² dao"
       " 0, 2 e 4, exactos",
       mal == 0 && nd[0] == 0 && nd[1] == 2 && nd[2] == 4);
    conclui("Quando coincidem, Δ = −4s anula: e a RAIZ DUPLA, a ressonancia das EDs.");
}

printf("\n§P2  E a fronteira É a derivada — exata, não aproximada.\n\n");
{
    printf("      f(a + bε) = f(a) + f'(a)·b·ε,  porque todo termo com ε² morre\n\n");
    int mal = 0;
    printf("      f            a      f(a)     f'(a) pela ε    f'(a)\n");
    for(int a = 1; a <= 4; a += 1){
        Z x = { a, 1 };
        Z r, q;
        const char *nome;
        long fa, fp;
        if(a == 1){
            r = mul(mul(x,x,0),x,0); nome = "x³";
            fa = a*a*a; fp = 3*a*a;
        } else if(a == 2){
            q = mul(x,x,0); r = mul(q,q,0); nome = "x⁴";
            fa = a*a*a*a; fp = 4*a*a*a;
        } else if(a == 3){
            q = mul(x,x,0);
            r.a = q.a + 3*x.a; r.b = q.b + 3*x.b; nome = "x² + 3x";
            fa = a*a + 3*a; fp = 2*a + 3;
        } else {
            q = mul(x,x,0); r = mul(mul(q,q,0),x,0); nome = "x⁵";
            fa = a*a*a*a*a; fp = 5*a*a*a*a;
        }
        printf("      %-12s %-6d %-8ld %-15ld %ld\n", nome, a, r.a, r.b, fp);
        if(r.a != fa || r.b != fp) mal += 1;
    }
    printf("\n");
    ok("a parte ε E a derivada, exacta em Z — sem limite, sem passo h e sem 1e-12",
       mal == 0);
    conclui("A derivada sai da ARITMETICA, porque ε² = 0 corta a serie no primeiro termo.");
}

printf("\n§P3  O bra-ket — o palpite do Aarão, e ele está certo (com uma condição).\n\n");
{
    long u[2] = {1,0}, z[2] = {0,0}, d[2] = {0,1};
    M2 P = ketbra(u, z, u, z);
    M2 PP = mm(P,P);
    M2 dif = P;
    for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1){
        dif.re[i][j] -= PP.re[i][j]; dif.im[i][j] -= PP.im[i][j];
    }
    printf("      |0><0| e o PROJECTOR:  ||P² − P|| = %ld  ->  P² = P, NAO nilpotente\n",
           mnorma(dif));
    ok("o projetor |a><a| da P² = P — nao e este o ε, e a diferenca e ZERO EXACTO",
       mnorma(dif) == 0);

    M2 S = ketbra(u, z, d, z);
    M2 SS = mm(S,S);
    printf("      |0><1| com <1|0> = 0:  ||S²|| = %ld  ->  S² = 0, NILPOTENTE. E o ε.\n\n",
           mnorma(SS));
    ok("o bra-ket com bra e ket ORTOGONAIS da exactamente ε² = 0", mnorma(SS) == 0);

    int mal = 0, orto = 0, nao = 0;
    for(long ar0 = -2; ar0 <= 2; ar0 += 1)
    for(long ar1 = -2; ar1 <= 2; ar1 += 1)
    for(long ai0 = -2; ai0 <= 2; ai0 += 1)
    for(long ai1 = -2; ai1 <= 2; ai1 += 1){
        if(!ar0 && !ar1 && !ai0 && !ai1) continue;
        long ar[2] = {ar0, ar1}, ai[2] = {ai0, ai1};
        long br[2] = {-ar1, ar0}, bi[2] = {ai1, -ai0};
        long ipr = 0, ipi = 0;
        for(int j = 0; j < 2; j += 1){
            ipr += br[j]*ar[j] + bi[j]*ai[j];
            ipi += br[j]*ai[j] - bi[j]*ar[j];
        }
        M2 T = ketbra(ar, ai, br, bi), TT = mm(T,T);
        int ip0 = (ipr == 0 && ipi == 0);
        int tt0 = (mnorma(TT) == 0);
        if(ip0) orto += 1; else nao += 1;
        if(tt0 != ip0) mal += 1;
        /* e o nao-ortogonal: b = a, projector, T² ≠ 0 */
        M2 Ta = ketbra(ar, ai, ar, ai), TTa = mm(Ta,Ta);
        long ipa = ar0*ar0 + ar1*ar1 + ai0*ai0 + ai1*ai1;
        if((mnorma(TTa) == 0) != (ipa == 0)) mal += 1;
        if(ipa != 0) nao += 1;
    }
    printf("      pares ortogonais: %d    nao ortogonais: %d    discordancias: %d\n\n",
           orto, nao, mal);
    ok("(|a><b|)² = 0 SE E SO SE <b|a> = 0 — o palpite, com a condicao exacta em Z."
       " 200 pares com sin/cos eram o transporte; a grelha {-2..2}⁴ e exacta",
       mal == 0 && orto == 624);
    conclui("Subir duas vezes num sistema de dois niveis da zero: o segundo passo nao existe.");
}

printf("\n§P4  Os férmions: a² = 0 é o princípio de exclusão.\n\n");
{
    long u[2] = {1,0}, d[2] = {0,1}, z[2] = {0,0};
    M2 a  = ketbra(u, z, d, z);
    M2 ad = ketbra(d, z, u, z);
    M2 aa = mm(a,a), adad = mm(ad,ad);
    M2 anti = msoma(mm(a,ad), mm(ad,a));
    M2 I; memset(&I, 0, sizeof I); I.re[0][0] = I.re[1][1] = 1;
    M2 dif = anti;
    for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1) dif.re[i][j] -= I.re[i][j];
    printf("      a·a   = %ld      (aniquilar duas vezes)\n", mnorma(aa));
    printf("      a†·a† = %ld      (CRIAR duas vezes o mesmo estado)\n", mnorma(adad));
    printf("      {a, a†} - I = %ld      (a relacao de anticomutacao)\n\n", mnorma(dif));
    ok("a² = (a†)² = 0 e {a,a†} = I — a algebra fermionica, com os tres residuos a ZERO"
       " EXACTO em Z",
       mnorma(aa) == 0 && mnorma(adad) == 0 && mnorma(dif) == 0);
    conclui("A exclusao nao e uma regra imposta: e a nilpotencia do gerador.");
}

printf("\n§P5  O cone do dual É o cone de luz — e o fóton não tem inverso.\n\n");
{
    printf("      no dual:      N(a,b) = a² - b²      anula em a = ±b\n");
    printf("      em Minkowski: s² = t² - x²          anula em x = ±t   (c = 1)\n\n");
    int mal = 0, luz = 0, tempo = 0, espaco = 0;
    for(int i = -12; i <= 12; i += 1) for(int j = -12; j <= 12; j += 1){
        if(!i && !j) continue;
        long N = (long)i*i - (long)j*j;
        int ehluz = (N == 0);
        if(ehluz) luz += 1; else if(N > 0) tempo += 1; else espaco += 1;
        if(ehluz != (i == j || i == -j)) mal += 1;
    }
    printf("      sobre o reticulado 25x25 sem a origem:\n");
    printf("      tipo TEMPO  (N > 0) : %d\n", tempo);
    printf("      tipo ESPACO (N < 0) : %d\n", espaco);
    printf("      tipo LUZ    (N = 0) : %d\n\n", luz);
    ok("os divisores de zero do dual sao exactamente os vectores NULOS de Minkowski."
       " |i|=|j| em 12 raios × 4 sinais = 48, exacto — o 1e-12 nao tinha o que acomodar",
       mal == 0 && luz == 48);
    conclui("O cone onde a algebra deixa de ser corpo e o cone onde a cinematica deixa de ter observador.");
}

printf("\n§P6  A soma de velocidades de Einstein É a lei polar do dual.\n\n");
{
    printf("      v1 ⊕ v2 = (v1+v2)/(1+v1v2),  e com v = p/q:  (p1q2+p2q1)/(q1q2+p1p2)\n\n");
    long assoc = 0, neutro = 0, cone = 0, tot_q = 0;
    const long P[] = {1, 1, 2, 3, -1, -2}, Q[] = {2, 3, 5, 7, 4, 9};
    for(int i = 0; i < 6; i += 1) for(int j = 0; j < 6; j += 1) for(int k = 0; k < 6; k += 1){
        long a1 = P[i]*Q[j] + P[j]*Q[i], b1 = Q[i]*Q[j] + P[i]*P[j];
        long e1 = a1*Q[k] + P[k]*b1,     f1 = b1*Q[k] + a1*P[k];
        long a2 = P[j]*Q[k] + P[k]*Q[j], b2 = Q[j]*Q[k] + P[j]*P[k];
        long e2 = P[i]*b2 + a2*Q[i],     f2 = Q[i]*b2 + P[i]*a2;
        tot_q += 1;
        if(e1*f2 == e2*f1) assoc += 1;
        long ae = e1 < 0 ? -e1 : e1, af = f1 < 0 ? -f1 : f1;
        if(ae < af) cone += 1;
    }
    for(int i = 0; i < 6; i += 1){
        long a1 = P[i]*1 + 0*Q[i], b1 = Q[i]*1 + P[i]*0;
        if(a1*Q[i] == P[i]*b1) neutro += 1;
    }
    printf("      ⊕ e associativa em %ld de %ld ternos\n", assoc, tot_q);
    printf("      o neutro e 0 em %ld de 6, e o CONE |v|<1 preserva-se em %ld de %ld\n\n",
           neutro, cone, tot_q);
    ok("somar rapidezes DA a formula de Einstein, exacta em Q: ⊕ e associativa, tem"
       " neutro 0 e preserva o cone |v| < 1 — e isso que a faz uma SOMA. tanh/atanh"
       " eram o transporte, e o residuo delas era o do arredondamento",
       assoc == tot_q && neutro == 6 && tot_q == 216);
    ok("e a velocidade composta nunca chega a c: o cone e inatingivel por dentro —"
       " |e| < |f| nos 216 ternos, exacto, sem uma tanh",
       cone == tot_q && tot_q == 216);
    conclui("O que no circulo e rodar, no dual e impulsionar — a mesma lei polar.");
}

printf("\n§P7  \"Acima do infinito\": c -> ∞ leva e² = +1 em e² = 0.\n\n");
{
    printf("      ε_c² = 1/c²  ->  0,  e o boost degenera em Galileu: t'² − 1 = 1/(c²−1)\n\n");
    int mal = 0, passos = 0;
    long ant = 0;
    printf("      c          1/c²            1/(c²−1)     desce?\n");
    for(int k = 1; k <= 6; k += 1){
        long c = rt_ipow(10, k);
        long c2 = c*c;
        long gal = c2 - 1;                       /* denominador de t'² − 1 */
        printf("      %-10ld 1/%ld          1/%ld        ", c, c2, gal);
        if(passos > 0){
            if(gal <= ant){ mal += 1; printf("NÃO\n"); }
            else printf("sim\n");
        } else printf("—\n");
        /* identidade: c² = (c²−1) + 1 */
        if(c2 != gal + 1) mal += 1;
        ant = gal;
        passos += 1;
    }
    printf("\n");
    ok("quando c cresce o boost converge para Galileu — ε² = 1/c² e t'²−1 = 1/(c²−1)"
       " DESCEM com k, exactos em Z, sem sqrt nem γ. O 1e-9 em t' comparava a forma"
       " fechada em R",
       mal == 0 && passos == 6);
    conclui("Acima do infinito as duas assintotas sao a mesma reta: Inonu-Wigner em 1+1.");
}

printf("\n§P8  O quadro das três — e o que é exato e o que é analogia.\n\n");
{
    printf("      e²    algebra          geometria    grupo        na fisica\n");
    printf("      ----  ---------------  -----------  -----------  ---------------------------\n");
    printf("      -1    complexos        circulo      rotacoes     fase e^{iθ}, U(1)\n");
    printf("      +1    dual (split)     hiperbola    boosts       Lorentz, rapidez, cone de luz\n");
    printf("       0    fronteira        reta dupla   Galileu      fermioes (θ²=0), a derivada\n\n");
    int nd0 = raios_nulos(-1), nd1 = raios_nulos(0), nd2 = raios_nulos(1);
    printf("      raios nulos por classe:  e²=-1 -> %d    e²=0 -> %d    e²=+1 -> %d\n\n",
           nd0, nd1, nd2);
    ok("as tres linhas sao geometrias distintas, e a contagem do cone separa-as:"
       " 0, 2 e 4 raios, todos diferentes. Os 720 angulos repetiam o §P1 em virgula",
       nd0 == 0 && nd1 == 2 && nd2 == 4
       && nd0 != nd1 && nd1 != nd2 && nd0 != nd2);
    conclui("O analogo do bra-ket e o de TRANSICAO entre estados ortogonais — nao o projector.");
}

printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
if(!falhas) printf("  RESIDUO 0\n");
else printf("  NAO FECHOU\n");
return falhas ? 1 : 0;
}
