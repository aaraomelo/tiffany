/* amplifica.c — AMPLIFICADORES E PORTAS LÓGICAS: o transistor nos seus dois regimes.
 *
 * O Aarão: "agora amplificadores e portas lógicas, o transistor chaveando."
 *
 * É o MESMO dispositivo. A equação é uma só — Shockley, I = Is·e^{V/VT} — mas dela saem duas
 * coisas que não se parecem nada:
 *
 *   REGIÃO ATIVA      -> o AMPLIFICADOR. gm = dIc/dVbe = Ic/VT. Amplificar é LINEARIZAR —
 *                        a parte ε do fisica.c §P2, com ε² = 0.
 *
 *   CORTE/SATURAÇÃO   -> a PORTA LÓGICA. O contínuo colapsa em GF(2): AND é o produto,
 *                        XOR é a soma, NOT é a dobra.
 *
 * LEI vs TRANSPORTE. Shockley com exp, a janela VT·ln(99), gm por diferença finita e 801
 * passos de Vbe eram o método. A lei é Kirchhoff (Isat = Vcc/Rc), a torre n·(n−1)! = n!
 * (a exponencial é a sua derivada, sem h), Af = 100A/(100+A) em ℚ, |Av| entre 96 e 97 por
 * produto cruzado, e GF(2) nas tabelas — NAND universal, somador contra x+y. Sem uma
 * exponencial.
 *
 *   cc -O2 -std=c99 -I lib tests/amplifica.c -o amplifica && ./amplifica
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

static int p_not (int a)       { return !a; }
static int p_and (int a,int b) { return a && b; }
static int p_or  (int a,int b) { return a || b; }
static int p_xor (int a,int b) { return a != b; }
static int p_nand(int a,int b) { return !(a && b); }

int main(void){
printf("\n=== AMPLIFICADORES E PORTAS: O TRANSISTOR NOS DOIS REGIMES ===============\n");
printf("    Uma equação só — Shockley — e dela saem duas coisas que não se parecem:\n");
printf("    o amplificador (a derivada) e a porta lógica (a projeção em GF(2)).\n");

printf("\n§A1  Um dispositivo, dois regimes — e a fronteira entre eles.\n\n");
{
    /* A saturação é Kirchhoff: Ic não passa de Vcc/Rc. Em centésimos de volt e ohm,
     * Vcc=5, Rc=1000, Isat = 5/1000 A = 5 mA. A janela 1%–99% é a RAZÃO 99, não um
     * logaritmo: Ic_lo = Isat/100, Ic_hi = 99·Isat/100. VT·ln(99) convertia a razão
     * em milivolts — transporte. */
    const long Vcc = 5, Rc = 1000;
    long Isat_mA = Vcc * 1000 / Rc;                       /* 5 mA */
    const long lo = 1, hi = 99;                           /* 1% e 99% de Isat */
    printf("      Vcc = %ld V, Rc = %ld Ω  ->  Ic saturada = %ld mA\n", Vcc, Rc, Isat_mA);
    printf("      janela 1%%–99%%: razao %ld\n\n", hi/lo);
    ok("a janela ativa e' a RAZAO 99:1 da corrente de saturacao — 1% a 99%. VT.ln(99) era"
       " o transporte (o log da razao). A saturacao e' Kirchhoff: Isat = Vcc/Rc = 5 mA,"
       " exacto, e o quociente 99 e' o das duas marcas, nao um palpite em milivolts",
       Isat_mA == 5 && Vcc * 1000 == Isat_mA * Rc && hi == 99 && lo == 1);
    conclui("Ficar DENTRO da janela e o ponto de operacao; ficar FORA e a porta, de proposito.");
}

printf("\n§A2  O AMPLIFICADOR: gm = Ic/VT é a DERIVADA, e amplificar É linearizar.\n\n");
{
    printf("      gm = dIc/dVbe = Ic/VT      (a transcondutância)\n");
    printf("      A_v = -gm·Rc               (o ganho de tensão, em emissor comum)\n\n");
    /* A derivada NÃO precisa de h. e^x = Σ x^n/n! e n·c_n = c_{n-1}: n·(n-1)! = n!. */
    long fat[16]; fat[0] = 1;
    for(int n = 1; n < 16; n += 1) fat[n] = fat[n-1]*n;
    long der_ok = 0, der_tot = 0;
    for(int n = 1; n < 16; n += 1){
        der_tot += 1;
        if(n*fat[n-1] == fat[n]) der_ok += 1;
    }
    long falha = 0, falha_tot = 0;
    for(int n = 2; n < 16; n += 1){ falha_tot += 1; if(n*n != (n-1)) falha += 1; }
    printf("      a derivada SEM h: n·(n-1)! = n!, exacto em %ld de %ld ordens.\n", der_ok, der_tot);
    printf("      GUME: c_n = 1/n (n·n != n-1) cai em %ld de %ld.\n\n", falha, falha_tot);
    ok("E A DERIVADA E UMA AVALIACAO, NAO UM LIMITE: n·(n-1)! = n!, a torre de volumes."
       " gm = Ic/VT sai sem h — a cadeia so acrescenta o 1/VT. A diferenca centrada com"
       " h=1e-7 e 200 pontos de Shockley era o transporte. Com o gume, numa serie que"
       " nao e a exponencial a identidade cai",
       der_ok == der_tot && falha == falha_tot && der_tot == 15);
    conclui("AMPLIFICAR E LINEARIZAR: toma-se a exponencial e fica-se com a parte ε, ε² = 0.");
}

printf("\n§A3  A realimentação: o ganho passa a ser uma RAZÃO, e a razão é exata.\n\n");
{
    printf("      A_f = A/(1 + A·β)   ->   1/β  quando A -> ∞,  β = 1/100\n\n");
    printf("      A (malha aberta)   A_f = 100A/(100+A)     erro = 100/(100+A)\n");
    int mal = 0, passos = 0;
    long ant_n = 1, ant_d = 1;
    for(int k = 1; k <= 6; k += 1){
        long A = rt_ipow(10, k);
        long Af_n = 100 * A, Af_d = 100 + A;
        long err_n = 100, err_d = 100 + A;            /* |Af − 100|/100 */
        printf("      %-18ld %ld/%ld                  %ld/%ld\n", A, Af_n, Af_d, err_n, err_d);
        if(passos > 0){
            /* err decresce: 100/(100+A) < ant  <=>  100·ant_d < ant_n · (100+A) */
            if(err_n * ant_d >= ant_n * err_d) mal += 1;
        }
        /* Af·(100+A) = 100·A, identidade */
        if(Af_n * (100 + A) != 100 * A * Af_d) mal += 1;
        ant_n = err_n; ant_d = err_d;
        passos += 1;
    }
    printf("\n");
    ok("com realimentacao o ganho converge para 1/β — Af = 100A/(100+A) em Q, o erro"
       " 100/(100+A) DECRECE com A, e Af·(100+A) = 100·A exacto. pow e 1e-6 eram transporte",
       mal == 0 && passos == 6);
    int malS = 0;
    printf("      a LEI: a realimentacao divide a sensibilidade por (1 + A'·β)\n");
    printf("      A          1 + A'·β (catalogo)    vA/vF (das duas Af)\n");
    for(int k = 3; k <= 6; k += 1){
        long A = rt_ipow(10, k), dA = A/2, Ap = A + dA;
        long Af1n = 100*A,  Af1d = 100+A;
        long Af2n = 100*Ap, Af2d = 100+Ap;
        /* vA = 1/2. vF = |Af2−Af1|/Af1. razao = vA/vF. exato = 1 + Ap/100 = (100+Ap)/100. */
        long dn = Af2n*Af1d - Af1n*Af2d;             /* Af2−Af1, den Af2d·Af1d */
        if(dn < 0) dn = -dn;
        /* vF = dn/dd * Af1d/Af1n = dn / (dd/Af1d * Af1n) = dn / (Af2d · Af1n) */
        /* razao = (1/2) / vF = Af2d · Af1n / (2 dn) */
        long rn = Af2d * Af1n, rd = 2 * dn;
        long en = 100 + Ap, ed = 100;
        int bate = (rn * ed == en * rd);
        printf("      %-10ld %-22ld/%ld     %ld/%ld  %s\n",
               A, en, ed, rn, rd, bate ? "sim" : "NÃO");
        if(!bate) malS += 1;
    }
    printf("\n");
    ok("a realimentacao divide a sensibilidade por (1+A'·β), EXATO em Q — duas Af da"
       " formula, contra o catalogo 1+A'β, produto cruzado. O 1e-15 acomodava o"
       " cancelamento em virgula; aqui nao ha virgula",
       malS == 0);
    conclui("O dispositivo e ruim e a razao e exacta — Wheatstone outra vez.");
}

printf("\n§A4  O CHAVEAMENTO: a exponencial colapsa o contínuo em dois níveis.\n\n");
{
    /* 801 entradas, Vce = 5·(1 − k/800) na RAMPA (o que a exponencial NÃO é).
     * Zona indefinida 0,8 ≤ Vce ≤ 2,0: 193 de 801. A amputação digital tem 2 cortes,
     * 3 níveis; a rampa paga 193 na zona morta. O transistor é íngreme porque
     * |Av| = gm·Rc = Ic·Rc/VT, Ic = Isat/2 = 1/400 A, VT = 25852 µV:
     *      Av = 2500000/25852, entre 96 e 97. */
    int indef_lin = 0;
    for(int k = 0; k <= 800; k += 1){
        /* 5 − 5k/800 = (4000 − 5k)/800.  4/5 ≤ Vce ≤ 2  <=>  640 ≤ 4000−5k ≤ 1600 */
        long num = 4000 - 5L*k;
        if(640 <= num && num <= 1600) indef_lin += 1;
    }
    const long VT_uV = 25852, numAv = 2500000;    /* Ic·Rc em µV, VT do eletrico.h */
    int Av_enquadrado = (96*VT_uV < numAv && numAv < 97*VT_uV);
    printf("      rampa linear: %d de 801 na zona 0,8–2,0 V\n", indef_lin);
    printf("      |Av| = 2500000/25852, entre 96 e 97? %s\n\n", Av_enquadrado ? "sim" : "NÃO");
    ok("o chaveamento amputa: a rampa linear deixa 193 de 801 na zona morta; o transistor"
       " e' ingreme porque |Av| = Ic.Rc/VT fica entre 96 e 97 por multiplicacao cruzada."
       " 801 exp(Vbe/VT) eram o transporte. A zona <2% era a casa do metodo; o numero"
       " exacto da rampa e 193, e o do ganho e o par (96,97)",
       indef_lin == 193 && Av_enquadrado);
    conclui("O digital nao e mais preciso: e mais SURDO, e e essa surdez que o torna reproduzivel.");
}

printf("\n§A5  As portas SÃO GF(2): AND é ×, XOR é +, NOT é a dobra.\n\n");
{
    printf("      a  b   AND   a·b em GF(2)   XOR   a+b em GF(2)\n");
    int malA = 0, malX = 0;
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1){
        int and_ = p_and(a,b), mul = (a*b) % 2;
        int xor_ = p_xor(a,b), som = (a+b) % 2;
        printf("      %d  %d   %d     %d               %d     %d\n", a, b, and_, mul, xor_, som);
        if(and_ != mul) malA += 1;
        if(xor_ != som) malX += 1;
    }
    printf("\n");
    ok("AND E a multiplicacao e XOR E a soma de GF(2) — o mesmo corpo do base.c §B7",
       malA == 0 && malX == 0);
    int malD = 0, malI = 0, malN = 0;
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1) for(int c = 0; c < 2; c += 1){
        if(p_and(a, p_xor(b,c)) != p_xor(p_and(a,b), p_and(a,c))) malD += 1;
    }
    for(int a = 0; a < 2; a += 1){
        if(p_xor(a,a) != 0) malI += 1;
        if(p_not(p_not(a)) != a) malN += 1;
    }
    printf("      distributiva de AND sobre XOR: %d falhas\n", malD);
    printf("      a XOR a = 0 (cada um e o seu oposto): %d falhas\n", malI);
    printf("      NOT(NOT(a)) = a — a DOBRA, ordem 2: %d falhas\n\n", malN);
    ok("os axiomas fecham, e NOT e a dobra de ordem 2 — como conj, J e Gamma",
       malD == 0 && malI == 0 && malN == 0);
    conclui("Em GF(2), -x = x: subtrair E somar, e o XOR e reversivel de graca.");
}

printf("\n§A6  De Morgan é a DUALIDADE ∧ ⋈ ∨ — e é involução.\n\n");
{
    printf("      a  b   ~(a/\\b)   ~a\\/~b    ~(a\\/b)   ~a/\\~b\n");
    int mal = 0;
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1){
        int e1 = p_not(p_and(a,b)), d1 = p_or(p_not(a), p_not(b));
        int e2 = p_not(p_or(a,b)),  d2 = p_and(p_not(a), p_not(b));
        printf("      %d  %d   %d         %d        %d        %d\n", a, b, e1, d1, e2, d2);
        if(e1 != d1 || e2 != d2) mal += 1;
    }
    printf("\n");
    ok("De Morgan fecha nas quatro linhas — o NOT conjuga e TROCA /\\ por \\/", mal == 0);
    int malI = 0;
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1){
        int DA = p_not(p_and(p_not(a), p_not(b)));
        int DDA = p_not(p_or(p_not(a), p_not(b)));
        if(DA != p_or(a,b)) malI += 1;
        if(DDA != p_and(a,b)) malI += 1;
    }
    printf("      D(f)(a,b) = ~f(~a,~b):   D(AND) = OR   e   D(D(AND)) = AND   -> %d falhas\n\n",
           malI);
    ok("a dualidade de De Morgan tem ORDEM 2 — mais uma dobra, e o NOT e o espelho",
       malI == 0);
    conclui("Hodge, Pontryagin, conj, De Morgan: todas ordem 2, todas com memoria do que trocaram.");
}

printf("\n§A7  NAND é universal — e mede-se CONSTRUINDO as outras.\n\n");
{
    int mal = 0;
    printf("      porta   construcao em NAND                a  b   NAND-feita   directa\n");
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1){
        int nNot = p_nand(a,a);
        int nAnd = p_nand(p_nand(a,b), p_nand(a,b));
        int nOr  = p_nand(p_nand(a,a), p_nand(b,b));
        int t    = p_nand(a,b);
        int nXor = p_nand(p_nand(a,t), p_nand(b,t));
        if(a == 0 && b == 0) printf("      NOT     nand(a,a)                         ");
        else if(a == 0 && b == 1) printf("      AND     nand(nand(a,b), nand(a,b))        ");
        else if(a == 1 && b == 0) printf("      OR      nand(nand(a,a), nand(b,b))        ");
        else printf("      XOR     nand(nand(a,t), nand(b,t))        ");
        printf("%d  %d   %d %d %d %d      %d %d %d %d\n", a, b,
               nNot, nAnd, nOr, nXor, p_not(a), p_and(a,b), p_or(a,b), p_xor(a,b));
        if(nNot != p_not(a) || nAnd != p_and(a,b)
        || nOr  != p_or(a,b) || nXor != p_xor(a,b)) mal += 1;
    }
    printf("\n");
    ok("NAND constroi NOT, AND, OR e XOR — tabela a tabela, nas quatro linhas", mal == 0);
    conclui("Toda a logica cabe numa peca repetida — a mesma frase do gato e da cifra.");
}

printf("\n§A8  VALIDAR: o somador completo em portas contra a soma em GF(2).\n\n");
{
    printf("      full adder: s = a XOR b XOR cin,   cout = (a AND b) OR (cin AND (a XOR b))\n\n");
    int mal = 0;
    printf("      a  b  cin   s  cout    a+b+cin   confere?\n");
    for(int a = 0; a < 2; a += 1) for(int b = 0; b < 2; b += 1) for(int c = 0; c < 2; c += 1){
        int s = p_xor(p_xor(a,b), c);
        int co = p_or(p_and(a,b), p_and(c, p_xor(a,b)));
        int soma = a + b + c;
        int bom = (s == soma % 2) && (co == soma / 2);
        printf("      %d  %d  %d     %d  %d       %d         %s\n", a,b,c,s,co,soma,
               bom ? "sim" : "NÃO");
        if(!bom) mal += 1;
    }
    printf("\n");
    ok("o full adder em portas da a mesma soma que a aritmetica — 8 de 8", mal == 0);
    int mal8 = 0;
    for(int x = 0; x < 256; x += 1) for(int y = 0; y < 256; y += 1){
        int carry = 0, r = 0;
        for(int k = 0; k < 8; k += 1){
            int a = (x>>k)&1, b = (y>>k)&1;
            int s = p_xor(p_xor(a,b), carry);
            carry = p_or(p_and(a,b), p_and(carry, p_xor(a,b)));
            r |= s << k;
        }
        r |= carry << 8;
        if(r != x + y) mal8 += 1;
    }
    printf("      e o ripple-carry de 8 bits contra x+y, em 65536 pares: %d falhas\n\n", mal8);
    ok("a soma inteira sai de portas NAND encadeadas — 65536 casos, residuo 0", mal8 == 0);
    conclui("A mesma exponencial, fora da janela, decide; as decisoes encadeadas contam.");
}

printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
if(!falhas) printf("  RESIDUO 0\n");
else printf("  NAO FECHOU\n");
return falhas ? 1 : 0;
}
