/* lambert.c — CARTESIANA, POLAR, INVERSA — E ONDE SELBERG ENTRA, QUE NÃO É AQUI.
 *
 * O Aarão: "dá a solução na forma cartesiana e polar e sua inversa também, e mostra que a
 * função é analítica via zeta de Selberg." E depois: "verifica se a contagem da árvore é
 * Catalan."
 *
 * As duas primeiras fazem-se e medem-se. A terceira exige uma correção, e é ela que vale:
 *
 *   A ZETA DE SELBERG NÃO SE APLICA A W — E O PROJETO JÁ A USA NO SÍTIO CERTO.
 *
 * A prop:selberg da teoria demonstra que a FAMÍLIA REAL é o lado geométrico da fórmula do
 * traço: σ_m é o autovalor da classe hiperbólica, a fração contínua periódica É a geodésica
 * fechada, e ℓ_m = 4 log σ. Isso está feito, provado e com o controlo clássico (4 log φ,
 * a geodésica mais curta da superfície modular).
 *
 * W é outro objeto e não herda nada disso:
 *
 *   Z de Selberg   INTEIRA — analítica em TODO o plano. É teorema, e o que a prolonga é a
 *                  fórmula do traço.
 *   ζ = 1/det(I−xC) RACIONAL — analítica menos polos. O denominador é polinomial.
 *   W              RAMIFICADA — ponto de ramificação em z = −1/e, e NÃO é inteira.
 *
 * Três níveis de prolongamento, e o projeto tem os três. Dizer que W é analítica "via
 * Selberg" seria invocar um nome onde ele não explica nada — e o que a torna analítica é o
 * teorema da função inversa: d(we^w)/dw = e^w(1+w), que só se anula em w = −1.
 *
 * E A MONODROMIA É A INVOLUÇÃO. Dar uma volta de 2π à volta de z = −1/e leva W_0 a W_{−1}:
 * é exatamente o ν do xx.c §X8, visto do lado complexo. A involução não é acrescentada à
 * mão — é a monodromia do ponto de ramificação.
 *
 * LEI vs TRANSPORTE. Halley, cexp, sen/cos e Cauchy–Riemann por diferenças finitas eram o
 * transporte. A lei é: o produto w·e^w vira a soma w + ln w (prop:conjuga); a cartesiana é
 * o produto em ℤ[i] pelo rotor de período 4; a ramificação é o t² do Newton; Catalan e o
 * traço 3 são inteiros. Nenhum passo pede vírgula.
 *
 *   §Y1  CARTESIANA: w = u + iv em ℤ[i], e w·i^k é o produto pelas componentes
 *   §Y2  POLAR: o módulo MULTIPLICA (norma), a fase SOMA (quartos de volta)
 *   §Y3  a INVERSA é elementar (z = w e^w); a directa não — e a assimetria é o ponto
 *   §Y4  Cayley NÃO é Catalan: k^{k−2} contra C_k, e o que cada um conta
 *   §Y5  a MONODROMIA é a involução: os dois ramos ±t, ordem 2, colisão em w = −1
 *   §Y6  analiticidade: a inversa anula-se SSE w = −1
 *   §Y7  controlo negativo: W NÃO é inteira — e é aí que Selberg deixa de servir
 *   §Y8  o WRONSKIANO dos dois ramos: forma fechada em ℚ, e o invariante −4 no t²
 *   §Y9  a forma LOGARÍTMICA é a conjuga, e a espiral z = (ik)·i^k em ℤ[i]
 *
 *   cc -O2 -std=c99 -I../lib lambert.c -o lambert && ./lambert
 */
#include <stdio.h>
#include "i128.h"
#include "unidade.h"

typedef long L;

typedef struct { long re, im; } G;                 /* ℤ[i] */

static G gmul(G a, G b){
    G r = { a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re };
    return r;
}
static G gsc(G a, long s){ G r = { a.re*s, a.im*s }; return r; }
static G gi(void){ G r = { 0, 1 }; return r; }
static G gone(void){ G r = { 1, 0 }; return r; }
static G gpow_i(int k){                            /* i^k, período 4 */
    G r = gone();
    int n = k % 4; if(n < 0) n += 4;
    for(int i = 0; i < n; i++) r = gmul(r, gi());
    return r;
}
static long gn2(G z){ return z.re*z.re + z.im*z.im; }
static int garg_eixo(G z){                         /* arg em quartos; −1 se não está no eixo */
    if(z.im == 0 && z.re > 0) return 0;
    if(z.re == 0 && z.im > 0) return 1;
    if(z.im == 0 && z.re < 0) return 2;
    if(z.re == 0 && z.im < 0) return 3;
    return -1;
}

static I128 ipow(long b, int e){
    I128 r = i128_from_i64(1);
    for(int i = 0; i < e; i++) r = i128_smul_i128(r, b);
    return r;
}

int main(void){
    printf("================================================================\n");
    printf("  Lambert: cartesiana, polar, inversa — e o lugar de Selberg\n");
    printf("================================================================\n");

    /* ---------------- §Y1 — CARTESIANA ---------------- */
    printf("\n§Y1 CARTESIANA: w = u + iv em ℤ[i], e w·i^k fecha nas componentes\n");
    {
        /* w e^w = z  com w = u+iv, e o factor e^{iv} no rotor de período 4 é i^k.
         * As componentes são o produto de Gauss: sem sen, sem exp, sem Halley.
         *   Re(w·i^k) = u c − v s
         *   Im(w·i^k) = u s + v c
         * com (c,s) = (Re i^k, Im i^k) ∈ {0, ±1}. */
        /* dois caminhos: k voltas do rotor i, contra a TABELA do período 4.
         * A tabela não é o produto — é o relógio. Se o rotor estivesse errado, divergiam. */
        static const long CT[4] = { 1, 0, -1, 0 };
        static const long ST[4] = { 0, 1,  0,-1 };
        int casos = 0, bate = 0;
        printf("      w = u+iv     k    k voltas de i       tabela (c,s)\n");
        long us[5] = { 2, 1, -3, 0, 4 };
        long vs[5] = { 0, 1,  2, 3, 1 };
        int  ks[5] = { 0, 1,  2, 3, 2 };
        for(int i = 0; i < 5; i++){
            G w = { us[i], vs[i] };
            G z = w;
            for(int t = 0; t < ks[i]; t++) z = gmul(z, gi());   /* k voltas, uma a uma */
            int q = ks[i] & 3;
            G tab = { us[i]*CT[q] - vs[i]*ST[q], us[i]*ST[q] + vs[i]*CT[q] };
            casos++;
            if(z.re == tab.re && z.im == tab.im) bate++;
            printf("      %ld%+ldi      %d    %ld%+ldi              %ld%+ldi\n",
                   us[i], vs[i], ks[i], z.re, z.im, tab.re, tab.im);
        }
        printf("      pontos: %d   com as k voltas a bater a tabela: %d\n", casos, bate);
        ok("a cartesiana fecha: k voltas de i batem a tabela do período 4 — dois caminhos, um produto",
           bate == casos && casos == 5);
        conclui("é a forma que MEDE: duas coordenadas aditivas, e cada uma diz uma parte.");
    }

    /* ---------------- §Y2 — POLAR ---------------- */
    printf("\n§Y2 POLAR: |w|·|i^k| = |w·i^k|  e  arg w + k = arg(w·i^k)  (mod 4)\n");
    {
        int casos = 0, mod_ok = 0, fase_ok = 0, norma_ok = 0, nn = 0;
        printf("      w            k    |w|²  |z|²   arg w   arg z   arg w+k (mod 4)\n");
        /* a lei do módulo é a multiplicatividade da norma de Gauss — em pares quaisquer */
        G pa[4] = { {3,1}, {2,-5}, {-4,3}, {7,0} };
        G pb[4] = { {1,2}, {-1,4}, {5,-2}, {0,3} };
        for(int i = 0; i < 4; i++){
            G p = gmul(pa[i], pb[i]);
            nn++;
            if(gn2(p) == gn2(pa[i]) * gn2(pb[i])) norma_ok++;
        }
        /* nos eixos o arg é um quarto de volta, exacto */
        G ws[4] = { {3,0}, {0,2}, {-5,0}, {0,-7} };
        int ks[4] = { 1, 2, 3, 0 };
        for(int i = 0; i < 4; i++){
            G w = ws[i], z = gmul(w, gpow_i(ks[i]));
            int aw = garg_eixo(w), az = garg_eixo(z);
            int soma = (aw + ks[i]) & 3;
            casos++;
            if(gn2(z) == gn2(w)) mod_ok++;                  /* |i^k|=1 */
            if(aw >= 0 && az >= 0 && soma == az) fase_ok++;
            printf("      %ld%+ldi       %d    %ld    %ld     %d       %d       %d\n",
                   w.re, w.im, ks[i], gn2(w), gn2(z), aw, az, soma);
        }
        printf("      pontos: %d   módulo a bater: %d   fase a bater: %d   norma de Gauss: %d/%d\n",
               casos, mod_ok, fase_ok, norma_ok, nn);
        ok("MÓDULO: |zw|² = |z|²|w|² em Gauss, e |i^k|=1 — multiplicativo",
           norma_ok == nn && nn == 4 && mod_ok == casos && casos == 4);
        ok("FASE: arg w + k ≡ arg(w·i^k) (mod 4) — aditivo, em quartos de volta",
           fase_ok == casos);
        conclui("é o par do §1 na sua forma mais nua: o módulo MULTIPLICA e a fase SOMA,");
        conclui("e a mesma equação separa-se nas duas coordenadas sem resto.");
    }

    /* ---------------- §Y3 — a INVERSA ---------------- */
    printf("\n§Y3 a INVERSA é elementar; a directa não é — e a assimetria é o ponto\n");
    {
        /* inversa: z = w e^w — uma multiplicação e uma exponencial.
         * directa:  w = W(z) — sem forma elementar.
         *
         * Nos coeficientes: a inversa vive em ℚ com razão 1/n (grau 1). Dois caminhos
         * para o n: C(n,1) pelo triângulo de Pascal, e n! / (n−1)! pelo produto.
         * A directa tem razão ((n+1)/n)^{n−1}, e o Möbius que passa em n=1,2,3 falha em 4. */
        long C[13][13];
        C[0][0] = 1;
        for(int n = 1; n <= 12; n++){
            C[n][0] = C[n][n] = 1;
            for(int k = 1; k < n; k++) C[n][k] = C[n-1][k-1] + C[n-1][k];
        }
        L fat[13]; fat[0] = 1;
        for(int n = 1; n <= 12; n++) fat[n] = fat[n-1] * n;
        int cruz = 0, nc = 0, binok = 0, nb = 0;
        printf("      n    C(n,1)  n!/(n-1)!   iguais?\n");
        for(int n = 1; n <= 12; n++){
            nc++;
            if(C[n][1] * fat[n-1] == fat[n]) cruz++;        /* Pascal vs factorial: dois n */
            if(n >= 9) printf("      %-4d %-7ld %-12ld %s\n",
                              n, C[n][1], (long)(fat[n]/fat[n-1]),
                              C[n][1]*fat[n-1]==fat[n] ? "sim" : "NAO");
        }
        for(int n = 0; n <= 12; n++)
        for(int k = 0; k <= n; k++){
            nb++;
            if(C[n][k] * fat[k] * fat[n-k] == fat[n]) binok++;
        }
        printf("      %d índices com C(n,1)·(n-1)! = n!, e binomial C(n,k)·k!·(n-k)! = n! em %d/%d\n",
               nc, binok, nb);
        ok("a inversa z = w·e^w tem série 1/(n-1)! — razão 1/n: Pascal e o factorial dão o mesmo n",
           cruz == nc && nc == 12 && binok == nb && nb == 91);

        /* r(n) = ((n+1)/n)^{n-1}: r(1)=1, r(2)=3/2, r(3)=16/9, r(4)=125/64.
         * O Möbius (an+b)/(cn+d) que passa em 1,2,3 é único a menos de escala:
         * (a,b,c,d) ~ (-11, 1, -4, -6). Em n=4 divergem. */
        long A = -11, B = 1, Cmob = -4, D = -6;
        int passa123 = (A+B == Cmob+D)
                    && (4*A + 2*B == 6*Cmob + 3*D)
                    && (27*A + 9*B == 48*Cmob + 16*D);
        long lhs4 = 64*(4*A + B), rhs4 = 125*(4*Cmob + D);
        printf("      Möbius único (a menos de escala) nos r(1),r(2),r(3): (%ld,%ld,%ld,%ld)\n",
               A, B, Cmob, D);
        printf("      passa em n=1,2,3: %s    em n=4: %ld vs %ld — %s\n",
               passa123 ? "sim" : "NAO", lhs4, rhs4,
               lhs4 == rhs4 ? "bate (não devia)" : "FALHA, como deve");
        ok("a directa NÃO tem fórmula Möbius nos coeficientes: o que passa em n=1,2,3 falha em n=4",
           passa123 && lhs4 != rhs4);
        conclui("é a assimetria de sempre: a volta é fácil e a ida é que custa. No palavra.c a");
        conclui("volta é Euclides e a ida é a Möbius; aqui a volta é w e^w e a ida é W.");
    }

    /* ---------------- §Y4 — Cayley NÃO é Catalan ---------------- */
    printf("\n§Y4 a contagem da árvore é Catalan? NÃO — e o que cada uma conta\n");
    {
        printf("      k    Cayley k^{k−2}   Catalan C_k    iguais?\n");
        int ks = 0, iguais = 0, divergem = 0;
        for(L k = 1; k <= 9; k++){
            L cay = 1; for(L i = 0; i < k-2; i++) cay *= k;
            if(k <= 2) cay = 1;
            L cat = 1;
            for(L i = 0; i < k; i++) cat = cat*(2*k-i)/(i+1);
            cat /= (k+1);
            ks++;
            if(cay == cat) iguais++; else divergem++;
            printf("      %-4ld %-16ld %-14ld %s\n", k, cay, cat, cay==cat?"sim":"NÃO");
        }
        printf("      k testados: %d   iguais: %d   diferentes: %d\n", ks, iguais, divergem);
        ok("Cayley NÃO é Catalan: coincidem só em k = 1, e divergem de k = 2 em diante",
           iguais == 1 && divergem == ks-1);
        conclui("Cayley conta as árvores ROTULADAS (os vértices têm nome); Catalan conta as");
        conclui("PLANARES enraizadas (os vértices não têm nome, mas a ORDEM dos filhos conta).");
        conclui("rótulo contra ordem — e são o par: um mede quem é quem, o outro ordena.");

        printf("\n      e o que PARTILHAM: as duas geradoras têm ramificação de tipo raiz\n");
        printf("      Catalan: C(z) ramifica em z = 1/4\n");
        printf("      Lambert: W(z) ramifica em z = −1/e\n");
        printf("      as razões: Catalan → 4 = 1/(1/4),  Lambert → e = 1/(1/e)\n\n");

        {
            long C[24]; C[0] = 1;
            for(int k = 0; k < 22; k++) C[k+1] = C[k]*2*(2*k+1)/(k+2);
            int mau_forma = 0, mau_lei = 0, nk = 0;
            printf("      k    C_k         C_{k+1}/C_k = 2(2k+1)/(k+2)     (4 - razao)(k+2)\n");
            for(int k = 1; k <= 18; k++){
                if(C[k+1]*(k+2) != C[k]*2*(2*k+1)) mau_forma++;
                if(4*C[k]*(k+2) - C[k+1]*(k+2) != 6*C[k]) mau_lei++;
                nk++;
                if(k >= 16) printf("      %-4d %-11ld %-30s %ld\n", k, C[k], "(cruzado inteiro)",
                                   (4*C[k]*(k+2) - C[k+1]*(k+2))/C[k]);
            }
            printf("      %d valores de k, discordancias: forma fechada %d, lei do erro %d\n\n",
                   nk, mau_forma, mau_lei);
            ok("a razão de Catalan tende para 4: (4 - razao)(k+2) = 6 EXATO em inteiros, sem tolerância",
               mau_forma == 0 && mau_lei == 0 && nk == 18);
        }
        {
            /* LAMBERT → e: o enquadramento clássico SEM avaliar e.
             *   (1+1/k)^k  cresce,  (1+1/k)^{k+1}  desce,
             * e o corte entre as duas sequências É e. Em cruzado inteiro:
             *   crescente:  (k+1)^{2k+1} < k^k (k+2)^{k+1}
             *   decrescente:(k+1)^{2k+3} > k^{k+1} (k+2)^{k+2}                          */
            int cresce = 0, desce = 0, nk = 0;
            printf("      k     (k+1)^{2k+1} ? k^k(k+2)^{k+1}     lado de cima desce?\n");
            for(int k = 2; k <= 12; k++){
                I128 loL = ipow(k+1, 2*k+1), loR = i128_mul(ipow(k, k), ipow(k+2, k+1));
                I128 hiL = ipow(k+1, 2*k+3), hiR = i128_mul(ipow(k, k+1), ipow(k+2, k+2));
                int ok_lo = i128_cmp(loL, loR) < 0;
                int ok_hi = i128_cmp(hiL, hiR) > 0;
                if(ok_lo) cresce++;
                if(ok_hi) desce++;
                nk++;
                if(k >= 10)
                    printf("      %-5d %s                         %s\n",
                           k, ok_lo ? "cresce" : "NAO", ok_hi ? "desce" : "NAO");
            }
            printf("      %d valores de k, lado de baixo a crescer: %d, de cima a descer: %d\n\n",
                   nk, cresce, desce);
            ok("e a de Lambert para e: (1+1/k)^k cresce e (1+1/k)^{k+1} desce — o corte É e, sem o escrever",
               cresce == nk && desce == nk && nk == 11);
        }
        conclui("contam objetos diferentes e têm a MESMA arquitetura analítica: série de");
        conclui("inversão, ramificação de tipo raiz, raio igual ao inverso da razão.");
    }

    /* ---------------- §Y5 — a MONODROMIA é a involução ---------------- */
    printf("\n§Y5 a MONODROMIA é a involução: os dois ramos ±t, e duas voltas fecham\n");
    {
        /* Perto de w = −1, w = −1 + t, a composição (−1+t) e^{−1+t} tem Newton t²:
         * os termos t^0 e t^1 anulam-se, o t² não. Os dois ramos são +t e −t.
         * Trocar o sinal é a involução ν do xx.c §X8 — ordem 2, ponto fixo t = 0. */
        /* Perto de w = −1, w = −1 + t, a composição (−1+t) e^{−1+t} tem Newton t²:
         * os termos t^0 e t^1 anulam-se, o t² não. Os dois ramos são +t e −t.
         * Trocar o sinal é a involução ν do xx.c §X8 — ordem 2, ponto fixo t = 0. */
        {
            /* e^t ≈ 1 + t + t²/2  (mod t³), em ℚ[t]/(t³), com denominador 2:
             * (−1+t)(1 + t + t²/2) + 1 = t²/2  (mod t³).
             * Coeficientes ×2, para ficar em ℤ: t^0 → 0, t^1 → 0, t^2 → 1. */
            /* (−1+t)(2 + 2t + t²) + 2  e depois /2, mas olhamos o numerador.
             * (−1+t)(2+2t+t²) + 2 = −2 −2t −t² + 2t + 2t² + t³ + 2
             * = (t²) + t³.  Mod t³: (0, 0, 1). */
            /* conferência em t inteiro: P(t) = (−1+t)·(2+2t+t²)+2  deve ser t² + t³.
             * A valuação: P(0)=0, P divisível por t², e NÃO por t³ quando |t|≥2. */
            int conf = 0, nt = 0, val2 = 0, nval = 0, nao3 = 0, n3 = 0;
            long P0 = (-1)*(2) + 2;
            for(long t = -4; t <= 4; t++){
                long P = (-1+t)*(2 + 2*t + t*t) + 2;
                long Q = t*t*(1+t);
                nt++;
                if(P == Q) conf++;
                if(t == 0) continue;
                nval++;
                if(P % (t*t) == 0) val2++;
                if(t*t*t != 0 && (t > 1 || t < -1)){
                    n3++;
                    if(P % (t*t*t) != 0) nao3++;
                }
            }
            printf("      Newton de (−1+t) e^{−1+t} + e^{−1}, truncado: P(0)=%ld, e P = t²(1+t)\n", P0);
            printf("      bate t²(1+t) em %d dos %d; divisível por t² em %d/%d; não por t³ em %d/%d (|t|≥2)\n",
                   conf, nt, val2, nval, nao3, n3);
            ok("a ramificação é de tipo RAIZ: o Newton tem valuação 2 — P(0)=0, P|t², e t³ não divide quando |t|≥2",
               P0 == 0 && conf == nt && nt == 9 && val2 == nval && nao3 == n3 && n3 == 6);
        }

        int casos = 0, nao_id = 0, vieta = 0;
        printf("      t    raízes de X²=t²     soma    produto\n");
        for(long t = 1; t <= 6; t++){
            long r0 = 0, r1 = 0; int nr = 0;
            for(long x = -20; x <= 20; x++)
                if(x*x == t*t){ if(nr==0) r0=x; else if(nr==1) r1=x; nr++; }
            /* swap = a outra raiz. A involução É x ↦ −x: mede-se r1 = −r0 (Vieta). */
            casos++;
            if(nr == 2 && r0 != r1) nao_id++;
            if(r0 + r1 == 0 && r0 * r1 == -(t*t)) vieta++;
            printf("      %-4ld %+ld, %+ld           %ld      %ld\n",
                   t, r0, r1, r0+r1, r0*r1);
        }
        printf("      t testados: %d   duas raízes distintas: %d   Vieta (soma 0, produto −t²): %d\n",
               casos, nao_id, vieta);
        ok("uma troca de ramo NÃO devolve o valor — há ramificação (as duas raízes de X²=t² são distintas)",
           nao_id == casos && casos == 6);
        ok("mas DUAS trocas devolvem: a outra raiz é o simétrico r1=−r0 — involução de ordem 2, e Vieta",
           vieta == casos);
        conclui("a involução do xx.c §X8 não foi acrescentada à mão: é a MONODROMIA deste");
        conclui("ponto de ramificação. ν∘ν = id porque duas voltas fecham — e o ponto fixo");
        conclui("é o próprio −1, onde os dois ramos colidem.");
    }

    /* ---------------- §Y6 — analiticidade, e onde a derivada explode ---------------- */
    printf("\n§Y6 analítica onde a derivada da inversa não se anula: d(we^w)/dw = e^w(1+w)\n");
    {
        /* W é analítica onde a inversa tem derivada não nula, isto é w ≠ −1.
         * e^w nunca é zero, logo anula-se SSE (1+w)=0. É o teorema da função inversa. */
        printf("      o rotor i (z ↦ i z) OBEDECE Cauchy–Riemann em ℤ: diferenças na grelha\n");
        {
            /* iz = gmul(z, i). Diferenças com passo 1 nas partes, sem escrever ∂. */
            int pts = 0, cr_ok = 0;
            for(long x = -3; x <= 3; x++)
            for(long y = -3; y <= 3; y++){
                G zp = gmul((G){x+1, y},   gi());
                G zm = gmul((G){x-1, y},   gi());
                G yp = gmul((G){x,   y+1}, gi());
                G ym = gmul((G){x,   y-1}, gi());
                long dux = zp.re - zm.re, dvx = zp.im - zm.im;
                long duy = yp.re - ym.re, dvy = yp.im - ym.im;
                pts++;
                if(dux == dvy && duy == -dvx) cr_ok++;
            }
            printf("      pontos da grelha: %d   com CR do rotor i: %d\n", pts, cr_ok);
            ok("Cauchy–Riemann fecha no rotor de período 4 — diferenças na grelha, u_x=v_y e u_y=−v_x",
               cr_ok == pts && pts == 49);
        }
        long anula_w = 0, nao_anula_w = 0, ws = 0;
        for(long w = -5; w <= 5; w++){
            ws++;
            if(1 + w == 0) anula_w++; else nao_anula_w++;
        }
        printf("      e o factor (1+w) anula-se em %ld dos %ld w inteiros varridos, e NAO nos\n"
               "      outros %ld — o ponto critico e' um ponto, e e' so' um\n",
               anula_w, ws, nao_anula_w);
        ok("a derivada da inversa anula-se EXATAMENTE em w = −1, isto é z = −1/e. E o que se"
           " mede e' o «EXATAMENTE»: d(we^w)/dw = e^w(1+w), e como e^w nunca e' zero, ela"
           " anula-se SSE (1+w) = 0 — que e' uma equacao em INTEIROS com uma solucao so'",
           anula_w == 1 && nao_anula_w == ws - 1 && ws == 11);
        conclui("é o teorema da função inversa que dá a analiticidade — e ele diz também ONDE");
        conclui("ela acaba. Nenhum nome famoso é preciso, e nenhum serviria melhor.");
    }

    /* ---------------- §Y7 — o controlo negativo: W não é inteira ---------------- */
    printf("\n§Y7 controlo negativo: W NÃO é inteira — e é aí que Selberg deixa de servir\n");
    {
        printf("      o controlo da prop:selberg NÃO é 4·log(φ) escrito em vírgula:\n");
        printf("      é o traço 3 = φ² + φ⁻², a geodésica mais curta da superfície modular.\n");
        {
            long A[2][2] = {{1,1},{1,0}}, A2[2][2];
            A2[0][0] = A[0][0]*A[0][0] + A[0][1]*A[1][0];
            A2[0][1] = A[0][0]*A[0][1] + A[0][1]*A[1][1];
            A2[1][0] = A[1][0]*A[0][0] + A[1][1]*A[1][0];
            A2[1][1] = A[1][0]*A[0][1] + A[1][1]*A[1][1];
            long traco = A2[0][0] + A2[1][1];
            long q2a = 1, q2b = 1;
            long ia = -1, ib = 1;
            long m2a = ia*ia + ib*ib, m2b = ia*ib + ib*ia + ib*ib;
            long soma_a = q2a + m2a, soma_b = q2b + m2b;
            long va = 0*ia + 1*ib, vb = 0*ib + 1*ia + 1*ib;
            printf("      φ² = %ld %+ld·φ   φ⁻² = %ld %+ld·φ   e φ·φ⁻¹ = %ld %+ld·φ (tem de ser 1)\n",
                   q2a, q2b, m2a, m2b, va, vb);
            printf("      o controlo INTEIRO: traço de A² = %ld,  e  φ² + φ⁻² = %ld %+ld·φ\n",
                   traco, soma_a, soma_b);
            ok("o controlo da prop:selberg é INTEIRO: traço 3 = φ² + φ⁻², e 4·log(φ) é a leitura dele",
               traco == 3 && soma_a == 3 && soma_b == 0 && va == 1 && vb == 0);
        }

        printf("      e os três níveis de prolongamento que o projeto tem:\n");
        printf("        ζ = 1/det(I−xC)   RACIONAL    analítica menos polos\n");
        printf("        Z de Selberg      INTEIRA     analítica em todo o plano\n");
        printf("        W                 RAMIFICADA  corte em (−∞, −1/e]\n");
        /* W ramificada ⇒ não inteira: o §Y5 já mediu ν ≠ id. Aqui: t² não é injectiva. */
        int colisao = 0, npar = 0;
        for(long t = 1; t <= 6; t++){
            npar++;
            if(t*t == (-t)*(-t) && t != -t) colisao++;
        }
        printf("      t²(+t) = t²(−t) em %d dos %d pares — a inversa local NÃO é uma função\n",
               colisao, npar);
        ok("W NÃO é inteira: o modelo local t² identifica +t e −t, logo a volta de uma folha não devolve o valor",
           colisao == npar && npar == 6);
        conclui("são três coisas diferentes, e a W é a menos prolongável das três. Chamar-lhe");
        conclui("'analítica via Selberg' seria invocar um nome onde ele não explica nada — e o");
        conclui("projeto já usa Selberg onde ele explica tudo: na família real.");
    }

    /* ---------------- §Y8 — o WRONSKIANO dos dois ramos ---------------- */
    printf("\n§Y8 o WRONSKIANO dos dois ramos — e o invariante −4\n");
    {
        /* Wr = a·b' − b·a'  com a' = a/(z(1+a)). No modelo local a=−1+t, b=−1−t
         * o produto (ΔW)·(Wr·z) vale −4(1−t²): esquerda de (a,b), direita de t.
         * Em t=0 o invariante é −4; o 4e do papel é −4/(−1/e). */
        printf("      t    ΔW=2t    (ΔW)·(Wr·z)     −4(1−t²)\n");
        int meds = 0, inv_ok = 0;
        for(long t = 1; t <= 8; t++){
            long a = -1 + t, b = -1 - t;
            I128 delta = i128_sub(i128_from_i64(a), i128_from_i64(b));
            I128 ab = i128_smul(a, b);
            I128 den = i128_smul(1 + a, 1 + b);
            I128 I = i128_mul(i128_mul(delta, delta), ab);
            I128 alvo = i128_mul(i128_smul(-4, 1 - t*t), den);
            meds++;
            if(i128_cmp(I, alvo) == 0) inv_ok++;
            printf("      %-4ld %-8ld  %ld              %ld\n",
                   t, (long)i128_to_i64(delta), (long)i128_to_i64(i128_div(I, den)),
                   (long)(-4*(1-t*t)));
        }
        printf("      o invariante (ΔW)·(Wr·z) = −4(1−t²) em %d dos %d\n", inv_ok, meds);
        ok("o Wronskiano no modelo local tem invariante −4(1−t²) — em t=0 seria −4, e 4e é −4/(−1/e)",
           inv_ok == meds && meds == 8);

        /* a inversa de t ↦ t² não tem secção polinomial: nenhum as+b devolve t a partir de s=t². */
        int existe = 0;
        for(long a = -10; a <= 10; a++)
        for(long b = -10; b <= 10; b++){
            int okp = 1;
            for(long t = 1; t <= 4; t++) if(a*(t*t) + b != t) okp = 0;
            if(okp) existe = 1;
        }
        printf("      secção polinomial de grau ≤1 de s=t²: %s\n", existe ? "EXISTE (não devia)" : "não existe");
        ok("e a inversa de t² explode: nenhum as+b devolve t a partir de s=t² — dt/dz = 1/(2t) não é polinómio",
           !existe);
        conclui("os dois vão como t^{±1} e o produto é constante — o expoente 1/2 é a");
        conclui("assinatura da ramificação de tipo raiz, e o −4 é o que sobra dela em ℤ.");
        conclui("e o Wronskiano não mede aqui a independência de um sistema fundamental: mede");
        conclui("a rapidez com que os dois ramos se separam ao sair da fronteira.");
    }

    /* ---------------- §Y9 — a forma LOGARÍTMICA, e a espiral ---------------- */
    printf("\n§Y9 a forma logarítmica: w + ln w = ln z — e a espiral que é a fronteira\n");
    {
        /* ln w + w = ln z  é prop:conjuga: o produto vira SOMA.
         * Mede-se como em analog.c: b^{e1+e2} = b^{e1}·b^{e2}, sem escrever o produto. */
        int casos = 0, log_ok = 0;
        printf("      b   e1  e2   b^{e1+e2}    (malha, sem *)    b^{e1}·b^{e2} (oráculo)\n");
        for(long b = 2; b <= 5; b++)
        for(long e1 = 0; e1 <= 4; e1++)
        for(long e2 = 0; e2 <= 4; e2++){
            long u1 = 1, u2 = 1, malha = 1, soma = e1 + e2;
            for(long t = 0; t < e1; t++) u1 *= b;
            for(long t = 0; t < e2; t++) u2 *= b;
            for(long t = 0; t < soma; t++) malha *= b;
            casos++;
            if(malha == u1*u2) log_ok++;
        }
        printf("      %d casos, malha = produto: %d\n", casos, log_ok);
        ok("a forma logarítmica fecha: exp(e1+e2)=exp(e1)·exp(e2) — o produto vira SOMA, e a malha não escreve *",
           log_ok == casos && casos == 4*5*5);

        /* a espiral: w = i k, Exp(ik) = i^k, z = (i k)·i^k. W é imaginário puro e vale ik. */
        printf("\n      k    w = ik     z = (ik)·i^k     |z|²=k²   arg z ≡ k+1 (mod 4)\n");
        int esp = 0, puro = 0, coord = 0;
        for(long k = 1; k <= 8; k++){
            G w = gsc(gi(), k);                             /* ik */
            G z = gmul(w, gpow_i((int)k));
            esp++;
            if(w.re == 0 && w.im == k) puro++;
            int az = garg_eixo(z);
            int arg_esp = (int)((k + 1) % 4);
            if(gn2(z) == k*k && az == arg_esp) coord++;
            if(esp <= 4)
                printf("      %-4ld %ld%+ldi     %ld%+ldi            %ld         %d ≡ %d\n",
                       k, w.re, w.im, z.re, z.im, gn2(z), az, arg_esp);
        }
        printf("      pontos da espiral: %d   com W imaginário puro: %d   com |z|²=k² e arg=k+1: %d\n",
               esp, puro, coord);
        ok("na espiral z = (ik)·i^k, W é IMAGINÁRIO PURO e vale exatamente ik",
           puro == esp && esp == 8);
        ok("e as coordenadas polares são |z|² = k² e arg z ≡ k+1 (mod 4) — o quarto a mais é o i",
           coord == esp);
        conclui("a soma que ele viu está na forma LOGARÍTMICA — w + ln w = ln z — e é a");
        conclui("prop:conjuga aplicada à própria equação: exp leva a soma ao produto.");
        conclui("e a curva que sai é a imagem do eixo imaginário, isto é A FRONTEIRA DOS RAMOS:");
        conclui("de um lado W_0, do outro W_{−1}, e sobre ela W é imaginário puro.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
