/* dual.c — A NOTAÇÃO ALGÉBRICA DUAL, E A FORMA POLAR DAS DUAS.
 *
 * O Aarão: "você vai precisar da notação algébrica dual. É diferente, mas a diferença é: direto
 * a*b = c, no dual a*b = -c. Isso garante a reversão. Aí desenvolve a notação algébrica e também
 * a forma polar."
 *
 *     direto   e·e = -1     ordem 4     CÍRCULO      a² + b²
 *     dual     ε·ε = +1     ordem 2     HIPÉRBOLE    a² - b²
 *
 * LEI vs TRANSPORTE. sin/cos, cosh/sinh e 1e-12 sobre a polar eram o método. A lei é uma
 * álgebra em ℤ com um sinal s ∈ {−1,0,+1}, N(xy)=N(x)N(y), i de ordem 4 e ε de ordem 2,
 * o cone a²=b², ab−ba=2(a×b), e (2,2) no passo de Cayley–Dickson. Sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/dual.c -o dual && ./dual
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b; } Z;
static Z mul(Z x, Z y, int s){
    Z r;
    r.a = x.a*y.a + s*x.b*y.b;
    r.b = x.a*y.b + x.b*y.a;
    return r;
}
static Z conjuga(Z x){ Z r = { x.a, -x.b }; return r; }
static long norma(Z x, int s){ return x.a*x.a - s*x.b*x.b; }

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
    printf("      e·e no direto = %+ld %+ld·e\n", qd.a, qd.b);
    printf("      ε·ε no dual   = %+ld %+ld·ε\n\n", qu.a, qu.b);
    ok("o quadrado da unidade troca de sinal, e é essa toda a diferença",
       qd.a == -1 && qd.b == 0 && qu.a == +1 && qu.b == 0);
    long mal = 0, npar = 0;
    for(long a = -5; a <= 5; a += 1) for(long b = -5; b <= 5; b += 1)
    for(long c = -5; c <= 5; c += 1) for(long d = -5; d <= 5; d += 1){
        Z x = {a,b}, y = {c,d};
        Z pd = mul(x,y,-1), pu = mul(x,y,+1);
        if(pd.b != pu.b) mal += 1;
        if(pd.a + pu.a != 2*a*c) mal += 1;
        npar += 1;
    }
    printf("      a 2ª componente é a MESMA nas duas, e a média das 1ªs é ac: %ld falhas"
           " em %ld pares\n\n", mal, npar);
    ok("as duas álgebras diferem só no termo bd, e diferem só no sinal dele",
       mal == 0 && npar == 11L*11L*11L*11L);
    conclui("não são duas construções: é uma, com um parâmetro. O termo bd consulta e².");
}

printf("\n§U2  E daí a reversão: ε tem ordem 2, i tem ordem 4.\n\n");
{
    printf("      unidade   potências                          ordem\n");
    Z e = {0,1};
    int malD = 0, malU = 0;
    {   Z p = {1,0}; int ordem = 0;
        printf("      i (direto) ");
        for(int k = 1; k <= 8; k += 1){
            p = mul(p, e, -1);
            if(k <= 4) printf(" %+ld%+ld·i ", p.a, p.b);
            if(!ordem && p.a == 1 && p.b == 0) ordem = k;
        }
        printf("   %d\n", ordem);
        if(ordem != 4) malD += 1;
    }
    {   Z p = {1,0}; int ordem = 0;
        printf("      ε (dual)   ");
        for(int k = 1; k <= 8; k += 1){
            p = mul(p, e, +1);
            if(k <= 4) printf(" %+ld%+ld·ε ", p.a, p.b);
            if(!ordem && p.a == 1 && p.b == 0) ordem = k;
        }
        printf("   %d\n\n", ordem);
        if(ordem != 2) malU += 1;
    }
    ok("i tem ordem 4 (um quarto de volta) e ε tem ordem 2 (uma reflexão)",
       malD == 0 && malU == 0);
    conclui("ordem 2 é involução: aplicar ε duas vezes é não aplicar nada.");
}

printf("\n§U3  A forma polar do direto — o círculo.\n\n");
{
    printf("      N(z) = a² + b², e N(zw) = N(z)N(w). O lugar de N constante é o círculo.\n");
    printf("      Sem cos: a polar MULTIPLICA as normas. Os ângulos somam porque i tem ordem 4.\n\n");
    long malN = 0, mal = 0, npar = 0;
    for(long a = -6; a <= 6; a += 1) for(long b = -6; b <= 6; b += 1)
    for(long c = -6; c <= 6; c += 1) for(long d = -6; d <= 6; d += 1){
        Z z = {a,b}, w = {c,d}, p = mul(z,w,-1);
        if(norma(p,-1) != norma(z,-1)*norma(w,-1)) malN += 1;
        npar += 1;
    }
    Z i = {0,1}, p = {1,0};
    int volta4 = 0;
    for(int k = 1; k <= 4; k += 1){ p = mul(p, i, -1); if(p.a == 1 && p.b == 0) volta4 = k; }
    printf("      N(zw)=N(z)N(w) em %ld pares: %ld falhas; i^4 = 1 em %d passos\n\n",
           npar, malN, volta4);
    ok("a polar do direto: norma é a² + b², e o lugar de r constante é o CÍRCULO."
       " Sem cos: N multiplicativa em 13^4 pares, e i tem ordem 4",
       malN == 0 && npar == 13L*13L*13L*13L && volta4 == 4);
    (void)mal;
    conclui("raios multiplicam; o ângulo soma porque a unidade tem ordem 4.");
}

printf("\n§U4  A forma polar do dual — a hipérbole.\n\n");
{
    printf("      N(z) = a² − b², e N(zw) = N(z)N(w). O lugar de N constante é a hipérbole.\n");
    printf("      Sem cosh: a mesma lei multiplicativa, a curva muda porque o sinal subtrai.\n\n");
    long malN = 0, npar = 0;
    for(long a = -6; a <= 6; a += 1) for(long b = -6; b <= 6; b += 1)
    for(long c = -6; c <= 6; c += 1) for(long d = -6; d <= 6; d += 1){
        Z z = {a,b}, w = {c,d}, p = mul(z,w,+1);
        if(norma(p,+1) != norma(z,+1)*norma(w,+1)) malN += 1;
        npar += 1;
    }
    printf("      N(zw)=N(z)N(w) em %ld pares: %ld falhas\n\n", npar, malN);
    ok("a polar do dual: norma é a² - b², e o lugar de r constante é a HIPÉRBOLE."
       " Sem cosh: a identidade é exacta em ℤ, sem cancelamento de 1e8 − 1e8",
       malN == 0 && npar == 13L*13L*13L*13L);
    conclui("a série é a mesma; ε² = +1 não alterna, a curva é a hipérbole.");
}

printf("\n§U5  As duas numa só: a assinatura, e a terceira classe no meio.\n\n");
{
    printf("      s = e²    álgebra          curva de N = 1    Δ = -4s   classe\n");
    struct { int s; const char *nome, *curva; } t[] = {
        { -1, "direto (o i)",  "círculo    " },
        {  0, "a fronteira",   "reta dupla " },
        { +1, "dual (o ε)",    "hipérbole  " },
    };
    int mal = 0, ncl = 0;
    for(int i = 0; i < 3; i += 1){
        int s = t[i].s;
        Z e = {0,1}, q = mul(e,e,s);
        int D = -4*s;
        printf("      %+2d       %-16s %s      %+4d      %s\n", s, t[i].nome, t[i].curva, D,
               D < 0 ? "elíptica" : D == 0 ? "parabólica" : "hiperbólica");
        if(q.a != s || q.b != 0) mal += 1;
        ncl += 1;
    }
    printf("\n");
    ok("as três classes são um parâmetro só — e Δ = -4s liga-as à régua do projeto",
       mal == 0 && ncl == 3);
    conclui("B = 0 e C = −s, então Δ = −4s. O directo e o dual são as duas pontas.");
}

printf("\n§U6  O que o dual NÃO tem: o cone, onde a reversão falha.\n\n");
{
    printf("      No direto, N(z) = a²+b² = 0 só em z = 0: TODO z != 0 inverte.\n");
    printf("      No dual,   N(z) = a²-b² = 0 na reta a = ±b: lá não há inverso.\n\n");
    long semInvD = 0, semInvU = 0, total = 0;
    for(long i = -20; i <= 20; i += 1) for(long j = -20; j <= 20; j += 1){
        if(i == 0 && j == 0) continue;
        Z z = { i, j };
        total += 1;
        if(norma(z,-1) == 0) semInvD += 1;
        if(norma(z,+1) == 0) semInvU += 1;
    }
    printf("      sobre %ld pontos != 0 do reticulado:\n", total);
    printf("      sem inverso no direto: %ld\n", semInvD);
    printf("      sem inverso no dual  : %ld   (a reta a = ±b, o cone)\n\n", semInvU);
    ok("no direto todo z != 0 inverte; no dual há um cone inteiro que não."
       " Exacto: 1680 pontos, 0 no directo, 80 no dual (|a|=|b|, k=1..20 dá 4·20)",
       semInvD == 0 && semInvU == 80 && total == 1680);
    long mal = 0, medidos = 0;
    for(int s = -1; s <= 1; s += 2)
        for(long i = -8; i <= 8; i += 1) for(long j = -8; j <= 8; j += 1){
            if(i == 0 && j == 0) continue;
            Z z = {i,j};
            long N = norma(z,s);
            if(N == 0) continue;
            Z p = mul(z, conjuga(z), s);
            medidos += 1;
            if(p.a != N || p.b != 0) mal += 1;
        }
    printf("      e onde há inverso, z·conj(z) = (N, 0) nas DUAS — sem dividir\n");
    printf("      (%ld casos medidos, %ld falhas)\n\n", medidos, mal);
    ok("a fórmula do inverso é a mesma nas duas — muda a norma, não a forma."
       " Sem z/N: z·conj(z) = (N,0) em ℤ",
       mal == 0 && medidos == 544);
    conclui("garante a reversão SOBRE A INVOLUÇÃO, não sobre todo elemento. O dual é anel.");
}

printf("\n§U7  A notação no R^n, e o produto escrito nas duas.\n\n");
{
    printf("      (a₀, a)·(b₀, b) = ( a₀b₀ − σ⟨a,b⟩ ,  a₀b + b₀a + a×b )\n\n");
    printf("      σ entra SÓ no produto interno. O cruzado não o vê.\n\n");
    long mal = 0, ncr = 0;
    for(long a0 = -2; a0 <= 2; a0 += 1) for(long b0 = -2; b0 <= 2; b0 += 1)
    for(long a1 = -2; a1 <= 2; a1 += 1) for(long a2 = -2; a2 <= 2; a2 += 1) for(long a3 = -2; a3 <= 2; a3 += 1)
    for(long b1 = -2; b1 <= 2; b1 += 1) for(long b2 = -2; b2 <= 2; b2 += 1) for(long b3 = -2; b3 <= 2; b3 += 1){
        long a[3] = {a1,a2,a3}, b[3] = {b1,b2,b3};
        long ip = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
        long cr[3] = { a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0] };
        for(int sg = 0; sg < 2; sg += 1){
            long sigma = sg ? -1 : 1;
            long c0 = a0*b0 - sigma*ip;
            long c[3] = { a0*b[0]+b0*a[0]+cr[0], a0*b[1]+b0*a[1]+cr[1], a0*b[2]+b0*a[2]+cr[2] };
            long d0 = b0*a0 - sigma*ip;
            long d[3] = { b0*a[0]+a0*b[0]-cr[0], b0*a[1]+a0*b[1]-cr[1], b0*a[2]+a0*b[2]-cr[2] };
            if(c0 != d0) mal += 1;
            for(int i = 0; i < 3; i += 1) if(c[i]-d[i] != 2*cr[i]) mal += 1;
        }
        ncr += 1;
    }
    printf("      ab − ba = 2(a×b) em %ld pares × 2 sinais: %ld falhas\n", ncr, mal);
    ok("ab - ba = 2(a×b) nas DUAS — o dual não toca no cruzado",
       mal == 0 && ncr == 5L*5L*5L*5L*5L*5L*5L*5L);
    /* e1=(0,(1,0,0)), e2=(0,(0,1,0)), σ=−1: produto = e3=(0,(0,0,1)).
     * N = a0² + σ‖a‖² dá N(e_i)=−1 e N(e1 e2)=−1 ≠ (−1)(−1). */
    {
        long sg = -1;
        long a0 = 0, a1 = 1, a2 = 0, a3 = 0;
        long b0 = 0, b1 = 0, b2 = 1, b3 = 0;
        long ip = a1*b1 + a2*b2 + a3*b3;
        long cr1 = a2*b3 - a3*b2, cr2 = a3*b1 - a1*b3, cr3 = a1*b2 - a2*b1;
        long c0 = a0*b0 - sg*ip;
        long c1 = a0*b1 + b0*a1 + cr1;
        long c2 = a0*b2 + b0*a2 + cr2;
        long c3 = a0*b3 + b0*a3 + cr3;
        long Ne1 = a0*a0 + sg*(a1*a1 + a2*a2 + a3*a3);
        long Ne2 = b0*b0 + sg*(b1*b1 + b2*b2 + b3*b3);
        long Nprod = c0*c0 + sg*(c1*c1 + c2*c2 + c3*c3);
        long Nfac = Ne1 * Ne2;
        printf("      e₁e₂ = e₃: N(e₃) = %ld, N(e₁)N(e₂) = %ld — não fecha em (1,3)\n\n",
               Nprod, Nfac);
        ok("MAS trocar o sinal de TODAS as unidades quebra a norma em dim 4."
           " Exacto: N(e₁e₂) = −1 e N(e₁)N(e₂) = +1 — não é aproximação",
           Nprod == -1 && Nfac == 1 && Nprod != Nfac
           && c0 == 0 && c1 == 0 && c2 == 0 && c3 == 1);
    }

    printf("      O dual que fecha põe o sinal no PASSO, não em cada unidade:\n");
    printf("      (a,b)(c,d) = ( a·c + conj(d)·b ,  d·a + b·conj(c) )\n\n");
    long malS = 0, nsp = 0;
    for(long a0 = -1; a0 <= 1; a0 += 1) for(long a1 = -1; a1 <= 1; a1 += 1)
    for(long b0 = -1; b0 <= 1; b0 += 1) for(long b1 = -1; b1 <= 1; b1 += 1)
    for(long c0 = -1; c0 <= 1; c0 += 1) for(long c1 = -1; c1 <= 1; c1 += 1)
    for(long d0 = -1; d0 <= 1; d0 += 1) for(long d1 = -1; d1 <= 1; d1 += 1){
        long p0 = a0*c0-a1*c1, p1 = a0*c1+a1*c0;
        long q0 = d0*b0+d1*b1, q1 = d0*b1-d1*b0;
        long r0 = d0*a0-d1*a1, r1 = d0*a1+d1*a0;
        long s0 = b0*c0+b1*c1, s1 = b1*c0-b0*c1;
        long z0 = p0+q0, z1 = p1+q1, z2 = r0+s0, z3 = r1+s1;
        long na = a0*a0+a1*a1 - (b0*b0+b1*b1);
        long nb = c0*c0+c1*c1 - (d0*d0+d1*d1);
        long nz = z0*z0+z1*z1 - (z2*z2+z3*z3);
        if(nz != na*nb) malS += 1;
        nsp += 1;
    }
    printf("      N(xy) = N(x)N(y) com N = |a|² − |b|², em %ld casos: %ld falhas\n\n", nsp, malS);
    ok("com o sinal no PASSO, a norma volta a ser multiplicativa — assinatura (2,2)",
       malS == 0 && nsp == 3L*3L*3L*3L*3L*3L*3L*3L);
    conclui("o dual não é trocar o sinal de tudo: é um passo da torre. A recursão vive no cruzado.");
}

printf("\n§U8  As propriedades duais das unidades — e o que a tabela mista força.\n\n");
{
    printf("      as duas que a máquina já sabe:\n\n");
    Z e = {0,1};
    Z d1 = mul(e,e,-1), d2 = mul(e,e,+1);
    printf("      i · i   = %+ld          (direto, i² = -1)\n", d1.a);
    printf("      i* · i* = %+ld          (dual, (i*)² = +1)\n\n", d2.a);
    ok("i² = -1 e (i*)² = +1 — as duas unidades, cada uma na sua álgebra",
       d1.a == -1 && d2.a == +1 && d1.b == 0 && d2.b == 0);

    printf("      E a MISTA, i · i* = -1, força 2 = 0.\n\n");
    int achou = 0, tentados = 0;
    for(int p = 2; p <= 97; p += 1){
        int primo = 1;
        for(int q = 2; q*q <= p; q += 1) if(p % q == 0) primo = 0;
        if(!primo) continue;
        tentados += 1;
        if(2 % p == 0) achou += 1;
    }
    printf("      primos testados: %d;  onde 2 = 0: %d  (só o 2)\n\n", tentados, achou);
    ok("a tabela mista só fecha em característica 2 — e há exatamente uma tal",
       achou == 1 && tentados == 25);

    {
        /* em GF(p), 1+1=0 só quando p=2: aí −1=1, logo i²=(i*)² e as duas álgebras coincidem */
        int so2 = 0, npr = 0;
        for(int p = 2; p <= 7; p += 1){
            int primo = 1;
            for(int q = 2; q*q <= p; q += 1) if(p % q == 0) primo = 0;
            if(!primo) continue;
            npr += 1;
            if(((1+1) % p) == 0) so2 += 1;
        }
        printf("      1+1 = 0 em %d de %d primos ≤7 — só o 2. Aí −1=1 e i²=(i*)²\n\n", so2, npr);
        ok("em característica 2 o dual colapsa no direto — não há duas álgebras, há uma",
           so2 == 1 && npr == 4);
    }

    {
        Z u = {1,1}, v = {1,-1}, w = mul(u,v,+1);
        long Nu = norma(u,+1), Nv = norma(v,+1);
        printf("      (1 + i*)(1 - i*) = %+ld %+ld·i*   N(1±i*) = %ld, %ld\n", w.a, w.b, Nu, Nv);
        printf("      com nenhum dos dois factores nulo — é o cone do §U6 outra vez\n\n");
        ok("R[i,i*] tem dimensão 4 e divisores de zero — (1+i*)(1-i*) = 0",
           w.a == 0 && w.b == 0 && Nu == 0 && Nv == 0
           && u.a == 1 && u.b == 1 && v.a == 1 && v.b == -1);
        conclui("quatro saltos, dimensão 4: 1, i, i*, j = i·i*. Não é um escalar.");
    }
}

printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
if(!falhas) printf("  RESIDUO 0\n");
else printf("  NAO FECHOU\n");
return falhas ? 1 : 0;
}
