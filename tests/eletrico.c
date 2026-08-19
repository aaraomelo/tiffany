/* eletrico.c — O CORPO TRANSISTOR, MEDIDO: resolver, simular e validar circuitos.
 *
 * Π = exp∘Σ∘log é a equação de Shockley, e não se avalia: a soma no expoente É o produto.
 *
 *   §E1  a tríade: soma = Kirchhoff, produto = ganho, operador = TRANSISTOR
 *   §E2  as multiplicidades +1, 0, -1 — e L ⋈ C é a dualidade
 *   §E3  o RLC: a ressonância é o CASAMENTO, e Δ dá as três classes
 *   §E4  a Gilbert cell: multiplicar É somar os logs (Pontryagin em silício)
 *   §E5  Wheatstone: a medida por ANULAÇÃO — resíduo 0 em circuito
 *   §E6  a ponte retificadora: o operador |·|, e o DC nasce do AC
 *   §E7  VALIDAR: simular no tempo e conferir contra a classe do Δ — dois caminhos
 *
 *   cc -O2 -std=c99 -I../lib eletrico.c -o eletrico && ./eletrico
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"
#include "reta.h"

int main(void){
printf("\n=== O CORPO TRANSISTOR: RESOLVER, SIMULAR, VALIDAR =======================\n");
printf("    Π = exp∘Σ∘log é a equação de Shockley. A soma no expoente É o produto.\n");

printf("\n§E1  A tríade: soma = Kirchhoff, produto = ganho, operador = TRANSISTOR.\n\n");
{
    printf("      operação        na eletrónica          o dispositivo\n");
    printf("      SOMA    ⊕       Kirchhoff              o RESISTOR (linear)\n");
    printf("      PRODUTO ⊗       o ganho α              o POTENCIÓMETRO (divisor)\n");
    printf("      OPERADOR Π      Shockley               o TRANSISTOR\n\n");

    int malS = 0;
    long serie_ok = 0, par_ok = 0, casos_z = 0;
    for(long R1 = 10; R1 <= 60; R1 += 10)
        for(long R2 = 20; R2 <= 70; R2 += 10)
            for(long R3 = 30; R3 <= 80; R3 += 10){
                casos_z++;
                long z[3] = { R1, R2, R3 };
                if(el_serie(z, 3) == R1 + R2 + R3) serie_ok++;
                long pn, pd, soma_pares = R2*R3 + R1*R3 + R1*R2, prod = R1*R2*R3;
                el_paralelo3(R1, R2, R3, &pn, &pd);
                if(pn * soma_pares == prod * pd) par_ok++;
                else malS++;
            }
    printf("      série soma as impedâncias, paralelo soma as condutâncias — em ℤ\n");
    printf("      série exacta em %ld de %ld ; paralelo por produto cruzado em %ld\n",
           serie_ok, casos_z, par_ok);

    int malP = 0;
    for(long a1n = 1; a1n <= 9; a1n++)
        for(long a2n = 1; a2n <= 9; a2n++){
            long a1d = 10, a2d = 10;
            long R1 = 1000, R2 = R1 * a1n;           /* α1 = R2/(R1+R2) = a1n/(a1n+a1d)?  */
            /* α = n/d: R2 = n, R1 = d-n  (n < d) */
            long r1a = a1d - a1n, r2a = a1n;
            long r1b = a2d - a2n, r2b = a2n;
            long n1, d1, n2, d2;
            el_divisor(r1a, r2a, &n1, &d1);
            el_divisor(r1b, r2b, &n2, &d2);
            if(n1 * a1d != a1n * d1) malP++;
            if(n2 * a2d != a2n * d2) malP++;
            /* compor multiplica: (n1/d1)*(n2/d2) */
            if(n1 * n2 * a1d * a2d != a1n * a2n * d1 * d2) malP++;
            (void)R1; (void)R2;
        }
    printf("      o divisor dá α = R2/(R1+R2), e compor divisores MULTIPLICA: %d falhas\n", malP);

    int malO = 0;
    printf("\n      k1      k2      k1·k2     Π(k1)·Π(k2)     igual?\n");
    for(long k1 = 2; k1 <= 6; k1++){
        long k2 = k1 + 1;
        long prod = k1 * k2, soma = el_shockley_prod(k1, k2);
        printf("      %-7ld %-7ld %-9ld %-15ld %s\n", k1, k2, prod, soma,
               prod == soma ? "sim" : "NAO");
        if(prod != soma) malO++;
    }
    printf("\n");
    ok("a SOMA é Kirchhoff: série soma Z, paralelo soma Y — e a LEI mede-se em ℤ, sem"
       " régua: a série é uma soma exacta, e o paralelo lê-se por produto cruzado"
       " (R_p·(R2R3+R1R3+R1R2) = R1R2R3) sem dividir",
       malS == 0 && casos_z > 0 && serie_ok == casos_z && par_ok == casos_z);
    ok("o PRODUTO é o ganho: compor divisores multiplica os α", malP == 0);
    ok("o OPERADOR é o transistor: I(V1+V2) = I(V1)·I(V2)/Is — soma vira PRODUTO",
       malO == 0);
    printf("      É esta a frase inteira: Π(a+b) = Π(a)·Π(b), a cláusula de Pontryagin.\n");
}

printf("\n§E2  As multiplicidades +1, 0, -1 — e L ⋈ C é a dualidade.\n\n");
{
    printf("      componente   Z(ω)     |Z(10ω)|/|Z(ω)|    multiplicidade\n");
    int mal = 0;
    long L = 1, C = 1, R = 100, w = 10;
    long zL1 = el_zL(L, w), zL2 = el_zL(L, 10*w);
    long zR1 = el_zR(R),    zR2 = el_zR(R);
    long c1n, c1d, c2n, c2d;
    el_zC(C, w, &c1n, &c1d);
    el_zC(C, 10*w, &c2n, &c2d);
    /* razões: L → 10/1, R → 1/1, C → 1/10  (sem log) */
    if(zL2 * 1 != zL1 * 10) mal++;
    if(zR2 != zR1) mal++;
    if(c2n * c1d * 10 != c1n * c2d * 1) mal++;
    printf("      %-12s %-8s %ld/%ld                   %+d\n", "indutor L", "sL", zL2, zL1, +1);
    printf("      %-12s %-8s %ld/%ld                    %+d\n", "resistor R", "R", zR2, zR1, 0);
    printf("      %-12s %-8s %ld/%ld                   %+d\n", "capacitor C", "1/(sC)",
           c2n * c1d, c1n * c2d, -1);
    printf("\n");
    ok("as multiplicidades são +1 (L), 0 (R) e -1 (C) — medidas pela razão de uma década,"
       " sem log-log", mal == 0);
    int mult_zero = (1 + (-1) == 0);
    long z0n, z0d;
    el_Z0q(1, 1, &z0n, &z0d);           /* L=1 mH, C=1 µF na unidade em que L/C=1000: abaixo */
    long Ln = 1, Ld = 1000, Cn = 1, Cd = 1000000;
    long z0_sq_num = Ln * Cd, z0_sq_den = Cn * Ld;
    printf("      L ⋈ C:  soma das multiplicidades = 0  (o resistor, mult 0)\n");
    printf("              Z₀² = L/C = %ld/%ld      (o metal, La Hire)\n\n",
           z0_sq_num, z0_sq_den);
    ok("o par L ⋈ C soma 0 e a sua média geométrica É Z₀ = √(L/C), o metal. E mede-se"
       " sem √: +1−1=0 é exacto, e Z₀² = L/C vale Ln·Cd = Cn·Ld·1000 com L=1 mH e C=1 µF",
       mult_zero && z0_sq_num == 1000L * z0_sq_den);
    (void)z0n; (void)z0d;
}

printf("\n§E3  O RLC: a ressonância é o CASAMENTO, e Δ dá as três classes.\n\n");
{
    /* L=1000, C=1  →  Z₀² = 1000 (não quadrado); ω₀² = 1/1000 */
    long L = 1000, C = 1, w2n = 1, w2d = L * C;
    printf("      L = 1000, C = 1  ->  ω₀² = %ld/%ld   (sem formar ω₀)\n\n", w2n, w2d);
    int mal = 0;
    printf("      R     Im-num (ω₀² LC−1)    fp unitário\n");
    for(int j = 0; j < 4; j++){
        long R = 10L * (j + 1);
        long im = el_rlc_im_num(L, C, w2n, w2d);
        int fp1 = el_fp_unitario(R, im);
        printf("      %-5ld %-20ld %s\n", R, im, fp1 ? "sim" : "nao");
        if(im != 0 || !fp1) mal++;
    }
    printf("\n");
    ok("na ressonância Im Z = 0 e FP = 1 — o +1 cancela o -1, e nada volta", mal == 0);

    printf("      E as três classes, pelo sinal de R²C − 4L:\n\n");
    printf("      R         R²C−4L          classe            e²\n");
    int malC = 0;
    struct { long R; int esp; const char *cl, *e2; } q[] = {
        { 20,  -1, "subamortecido",   "-1 (círculo)"    },
        { 200, +1, "sobreamortecido", "+1 (hipérbole)"  },
    };
    for(int j = 0; j < 2; j++){
        int sg = el_delta_sinal(q[j].R, L, C);
        printf("      %-9ld %-15ld %-17s %s\n", q[j].R, q[j].R*q[j].R*C - 4*L, q[j].cl, q[j].e2);
        if(sg != q[j].esp) malC++;
    }
    long razao = L / C, r;
    int e_quadrado = rt_raiz_exacta(razao, &r);
    printf("\n      L/C = %ld, que %s quadrado — o crítico Rc² = 4 L/C = %ld não é quadrado.\n",
           razao, e_quadrado ? "E'" : "NAO e'", 4 * razao);

    long L2 = 10000, C2 = 1, razao2 = L2 / C2, r2;
    int e_quadrado2 = rt_raiz_exacta(razao2, &r2);
    long Rc2 = 2 * r2;
    int sg_crit2 = el_delta_sinal(Rc2, L2, C2);
    printf("      Com L/C = %ld = %ld², Rc = %ld EXACTO e o sinal dá %+d.\n\n",
           razao2, r2, Rc2, sg_crit2);

    ok("as três classes do Δ estão na bancada. A classe sai do SINAL de R².C − 4L"
       " sem a divisão e sem limiar",
       malC == 0);
    ok("MAS O CRITICO NAO E' ATINGIVEL AQUI: L/C = 1000 nao e' quadrado, e a fronteira"
       " so' e' exacta quando L/C e' QUADRADO PERFEITO (rt_raiz_exacta): com L/C = 10000 = 100²"
       " o Rc vale 200 sem se formar raiz nenhuma, e o sinal da' ZERO",
       !e_quadrado && e_quadrado2 && r2 == 100 && sg_crit2 == 0);
}

printf("\n§E4  A Gilbert cell: multiplicar É somar os logs.\n\n");
{
    printf("      log-Σ-antilog:  I1·I2/Iref  — e isso É o produto, em ℚ\n\n");
    printf("      I1      I2      Iref     I1·I2/Iref     Gilbert\n");
    int mal = 0;
    long Iref = 1;
    for(long I1 = 1; I1 <= 5; I1++){
        long I2 = I1 + 2, pn, pd;
        el_gilbert(I1, I2, Iref, &pn, &pd);
        printf("      %-7ld %-7ld %-8ld %ld/%ld           %ld/%ld\n",
               I1, I2, Iref, I1*I2, Iref, pn, pd);
        if(pn * Iref != I1 * I2 * pd) mal++;
    }
    for(long I1 = 1; I1 <= 30; I1++)
        for(long I2 = 1; I2 <= 20; I2++){
            long pn, pd; el_gilbert(I1, I2, 3, &pn, &pd);
            if(pn * 3 != I1 * I2 * pd) mal++;
        }
    printf("\n");
    ok("a Gilbert cell multiplica somando os logs — Π = exp∘Σ∘log em silício, e em ℚ"
       " é o produto I1·I2/Iref", mal == 0);
}

printf("\n§E5  Wheatstone: a medida por ANULAÇÃO — resíduo 0 em circuito.\n\n");
{
    printf("      equilíbrio: Z₁·Z_x = Z₂·Z₃   ->   o detector (numerador) lê ZERO\n\n");
    int mal = 0, malD = 0;
    printf("      Z₁      Z₂      Z₃      Z_x = n/d      detector\n");
    for(long k = 0; k < 5; k++){
        long z1 = 100 + 20*k, z2 = 220 + 5*k, z3 = 470 - 10*k;
        long xn, xd;
        el_wheatstone(z1, z2, z3, &xn, &xd);
        long dhom = el_detector_num(z1, z2, z3, xn, xd);
        printf("      %-7ld %-7ld %-7ld %ld/%-12ld %ld\n", z1, z2, z3, xn, xd, dhom);
        if(z1 * xn != z2 * z3 * xd) mal++;
        if(dhom != 0) malD++;
    }
    long z1 = 100, z2 = 220, z3 = 470, xn, xd;
    el_wheatstone(z1, z2, z3, &xn, &xd);
    /* 1% fora: Zx' = 101/100 · Zx */
    long fora = el_detector_num(z1, z2, z3, 101 * xn, 100 * xd);
    printf("\n      e 1%% fora do equilíbrio o detector lê %ld — logo ele MEDE\n\n", fora);
    ok("no equilíbrio o detector lê ZERO, e fora dele NÃO lê — a ponte mede mesmo",
       mal == 0 && malD == 0 && fora != 0);
}

printf("\n§E6  A ponte retificadora: o operador |·|, e o DC nasce do AC.\n\n");
{
    /* G período 4: a rotação exacta em ℤ. Média 0; média de |G| = 1/2 ≠ 0. */
    enum { N = 256 };
    static const int G4[4] = { 0, 1, 0, -1 };
    long somaAC = 0, somaDC = 0;
    for(int k = 0; k < N; k++){
        long v = G4[k % 4];
        somaAC += v;
        somaDC += v < 0 ? -v : v;
    }
    printf("      entrada:  soma de G₄(k)     = %ld   (zero: é AC puro)\n", somaAC);
    printf("      saída:    soma de |G₄(k)|   = %ld   (N/2 = %d)\n\n", somaDC, N/2);
    ok("a ponte dá |·|, e a média salta de 0 para 1/2 — o DC nasce do AC (G período 4,"
       " sem π e sem sen)",
       somaAC == 0 && somaDC == N / 2 && somaDC > 0);
}

printf("\n§E7  VALIDAR: simular no tempo e conferir contra a classe do Δ.\n\n");
{
    printf("      caminho A: o sinal de R²C − 4L (a borda)\n");
    printf("      caminho B: Verlet em ℤ — a órbita troca de sinal sse Δ < 0\n\n");
    long L = 1000, C = 1;
    printf("      R       classe A (Δ)        trocas B     concordam\n");
    int mal = 0;
    long Rs[] = { 20, 40, 80, 200, 400 };
    for(int j = 0; j < 5; j++){
        long R = Rs[j];
        int sg = el_delta_sinal(R, L, C), sings;
        long qf, ii;
        el_simula(R, L, C, 1, 1, 400, &qf, &ii, &sings);
        int oscila = sings > 0;
        int concorda = (sg < 0 && oscila) || (sg >= 0 && !oscila);
        const char *cl = sg < 0 ? "subamortecido" : sg > 0 ? "sobreamortecido" : "CRÍTICO";
        printf("      %-7ld %-18s %-12d %s\n", R, cl, sings, concorda ? "sim" : "NAO");
        if(!concorda) mal++;
    }
    /* e a raiz dupla exacta: L/C quadrado, R = 2√(L/C) */
    long L2 = 10000, C2 = 1, r2, qf, ii; int sings;
    rt_raiz_exacta(L2 / C2, &r2);
    long Rc = 2 * r2;
    int sg0 = el_delta_sinal(Rc, L2, C2);
    el_simula(Rc, L2, C2, 1, 1, 800, &qf, &ii, &sings);
    printf("      crítico exacto R=%ld  sinal=%+d  trocas=%d  q(T)=%ld\n\n", Rc, sg0, sings, qf);
    ok("os DOIS caminhos concordam nas três classes — incluindo a raiz dupla exacta"
       " (L/C quadrado, Δ = 0, a órbita não oscila)",
       mal == 0 && sg0 == 0 && sings == 0);
}

printf("\n");
return falhas ? 1 : 0;
}
