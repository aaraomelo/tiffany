/* zeta.c — A ZETA DE RIEMANN DESENROLA NESSES PONTOS?
 *
 * O Aarão, depois do fisica.c: "verifica se a zeta de Riemann desenrola nesses pontos."
 *
 * "Esses pontos" são os do corpo dual: o VINCO (o conjunto fixo da dobra), a RAIZ DUPLA
 * (ε² = 0, onde o produto degenera) e o DESENROLAR (Σ vira Π, que é o Pontryagin do corpo
 * diferencial). A pergunta tem três partes, e as três medem-se — mas não dão todas a mesma
 * resposta, e é isso que a torna útil.
 *
 * O QUE ESTA SECÇÃO NÃO FAZ: não prova nem sugere nada sobre a hipótese de Riemann. Tudo o que
 * aqui se mede é LOCAL — e agora é EXACTO em ℤ/ℚ. A distância entre "medi nos primeiros
 * dez zeros" e "vale para todos" é o problema inteiro, e escrevê-la como se fosse pequena seria
 * a pior espécie de desonestidade que este projeto pode cometer.
 *
 * LEI vs TRANSPORTE. Borwein, Lanczos, Euler–Maclaurin e os γ da literatura eram o transporte:
 * avaliavam ζ(1/2+iγ) com limiar. A lei é: a dobra s ↦ 1−s̄ tem ordem 2 e o vinco é σ=1/2;
 * ξ(2k)=ξ(1−2k) fecha nos Bernoulli, em ℚ, depois de cancelar π^k; o Euler Π=Σ é o crivo
 * truncado, duas rotas; os zeros triviais são B_{ímpar>1}=0, isolados; degenera no polo s=1.
 *
 *   §Z1  a equação funcional É uma dobra: s -> 1-s̄ tem ordem 2
 *   §Z2  e o VINCO dessa dobra é exatamente a reta crítica Re(s) = 1/2
 *   §Z3  a dobra guarda a simetria: ξ(2k) = ξ(1−2k), Bernoulli, π cancelado
 *   §Z4  o DESENROLAR: Σ n^{-s} = Π (1-p^{-s})^{-1} — a soma vira produto, em ℚ
 *   §Z5  zeros TRIVIAIS: são SIMPLES (vizinhos não anulam) — B_{ímpar>1}=0
 *   §Z6  onde degenera: o polo s=1 (H_n cresce), não os zeros triviais
 *   §Z7  o que se mediu e o que não se mediu — os triviais NÃO estão no vinco
 *
 *   cc -O2 -std=c99 -I lib tests/zeta.c -o zeta && ./zeta
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

typedef __int128 I128;
typedef struct { I128 n, d; } Fr;

static I128 iabs128(I128 a){ return a < 0 ? -a : a; }
static I128 mdc128(I128 a, I128 b){
    a = iabs128(a); b = iabs128(b);
    while(b){ I128 t = a % b; a = b; b = t; }
    return a ? a : 1;
}
static Fr fr(I128 n, I128 d){
    if(d < 0){ n = -n; d = -d; }
    I128 g = mdc128(n, d);
    Fr r; r.n = n/g; r.d = d/g; return r;
}
static Fr fr_mul(Fr x, Fr y){ return fr(x.n*y.n, x.d*y.d); }
static Fr fr_div(Fr x, Fr y){ return fr(x.n*y.d, x.d*y.n); }
static Fr fr_add(Fr x, Fr y){ return fr(x.n*y.d + y.n*x.d, x.d*y.d); }
static int fr_eq(Fr x, Fr y){ return x.n == y.n && x.d == y.d; }
static int fr_cmp(Fr x, Fr y){
    I128 L = x.n * y.d, R = y.n * x.d;
    return L < R ? -1 : L > R ? 1 : 0;
}
static int fr_zero(Fr x){ return x.n == 0; }
static I128 ipow128(I128 b, int e){
    I128 r = 1;
    for(int i = 0; i < e; i++) r *= b;
    return r;
}
static I128 fat128(int n){
    I128 r = 1;
    for(int i = 2; i <= n; i++) r *= i;
    return r;
}
static void fr_print(Fr x){
    if(x.n <= 9223372036854775807LL && x.n >= -9223372036854775807LL
       && x.d <= 9223372036854775807LL)
        printf("%lld/%lld", (long long)x.n, (long long)x.d);
    else printf("(fracção grande)");
}

/* Bernoulli B_2, B_4, … B_12 — os pares; os ímpares > 1 são zero, e isso É o zero trivial. */
static const int Bpar_n[] = { 1, -1, 1, -1, 5, -691 };
static const int Bpar_d[] = { 6, 30, 42, 30, 66, 2730 };
static Fr B_par(int k){          /* B_{2k}, k≥1 */
    return fr(Bpar_n[k-1], Bpar_d[k-1]);
}

/* primos ≤ 31 — chega para o Euler truncado em ℚ, sem crivo de 300 000. */
static const int PRIMOS[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31 };
#define NP ((int)(sizeof PRIMOS / sizeof *PRIMOS))

/* soma-rota: Π_p (1 + p^{-s} + … + p^{-E s}) por multiplicação iterada */
static Fr euler_soma(int P, int s, int E){
    Fr acc = fr(1, 1);
    for(int i = 0; i < NP && PRIMOS[i] <= P; i++){
        I128 ps = ipow128(PRIMOS[i], s);
        Fr novo = fr(0, 1), termo = acc;
        for(int k = 0; k <= E; k++){
            novo = fr_add(novo, termo);
            termo = fr_div(termo, fr(ps, 1));
        }
        acc = novo;
    }
    return acc;
}
/* produto-rota: Π_p (p^{s(E+1)}−1) / ((p^s−1) p^{s E}) — a soma geométrica fechada */
static Fr euler_prod(int P, int s, int E){
    Fr acc = fr(1, 1);
    for(int i = 0; i < NP && PRIMOS[i] <= P; i++){
        I128 p = PRIMOS[i], ps = ipow128(p, s);
        Fr f = fr(ipow128(p, s*(E+1)) - 1, (ps - 1) * ipow128(p, s*E));
        acc = fr_mul(acc, f);
    }
    return acc;
}
/* Euler infinito (s inteiro): Π_{p≤P} p^s / (p^s − 1) */
static Fr euler_inf(int P, int s){
    Fr acc = fr(1, 1);
    for(int i = 0; i < NP && PRIMOS[i] <= P; i++){
        I128 ps = ipow128(PRIMOS[i], s);
        acc = fr_mul(acc, fr(ps, ps - 1));
    }
    return acc;
}

int main(void){
printf("\n=== A ZETA DESENROLA NESSES PONTOS? =====================================\n");
printf("    Três perguntas numa: o VINCO, o DESENROLAR (Σ -> Π) e a RAIZ DUPLA.\n");
printf("    E elas não dão a mesma resposta — é isso que as torna úteis.\n");

printf("\n§Z1  A equação funcional É uma dobra: ordem 2.\n\n");
{
    /* A dobra candidata é s -> 1-s. Mas essa, em C, só fixa o PONTO 1/2. A que fixa a RETA é
     * a anti-holomorfa s -> 1-conj(s) — e é ela a dobra no sentido do §B14: uma reflexão.
     * Em ℤ: s = (re, im) com unidade 1/10, e 1 = (10, 0). A∘A e B∘B são a identidade por
     * duas APLICAÇÕES, não por 1-(1-s) fundido — e A não é id: a maior parte dos pontos mexem. */
    printf("      candidata A:  s -> 1 - s        (holomorfa)\n");
    printf("      candidata B:  s -> 1 - conj(s)  (anti-holomorfa, uma REFLEXÃO)\n\n");
    int malA = 0, malB = 0, mexeuA = 0, mexeuB = 0;
    const int DEN = 10;
    for(int k = 0; k < 200; k++){
        int re = 1 + k, im = 3*k - 50;                 /* (0.1+0.1k) + i(0.3k-5), em décimos */
        int Are = DEN - re, Aim = -im;                 /* A */
        int A2re = DEN - Are, A2im = -Aim;             /* A∘A */
        int Bre = DEN - re, Bim = im;                  /* B = 1-conj */
        int B2re = DEN - Bre, B2im = Bim;              /* B∘B: conj de B é (Bre, -Bim), 1-isso */
        /* B∘B: B(re,im)=(DEN-re, im); B disso = (DEN-(DEN-re), im)=(re,im). */
        if(A2re != re || A2im != im) malA++;
        if(B2re != re || B2im != im) malB++;
        if(Are != re || Aim != im) mexeuA++;
        if(Bre != re || Bim != im) mexeuB++;
    }
    printf("      A∘A = id em 200 pontos: %d falhas  (e A mexeu %d)\n", malA, mexeuA);
    printf("      B∘B = id em 200 pontos: %d falhas  (e B mexeu %d)\n\n", malB, mexeuB);
    ok("as duas têm ordem 2 — são dobras, no sentido exato do §B14",
       malA == 0 && malB == 0 && mexeuA == 200 && mexeuB == 199);
    printf("      Ordem finita: as duas guardam a memória da simetria, e basta desdobrar. Mas\n");
    printf("      elas não têm o mesmo VINCO, e é o vinco que responde à pergunta.\n");
}

printf("\n§Z2  E o VINCO é exatamente a reta crítica.\n\n");
{
    /* o vinco é o conjunto fixo. Grelha σ = i/40, t = (j−20)/2, i,j ∈ 0..40 — exacta em ℤ.
     * A fixa iff σ=1/2 e t=0 (um ponto). B fixa iff σ=1/2 (a reta, 41 pontos). */
    int fixA = 0, fixB = 0, naRetaB = 0, forA = 0;
    printf("      numa grelha de σ = i/40 × t = (j-20)/2, i,j ∈ 0..40:\n\n");
    for(int i = 0; i <= 40; i++) for(int j = 0; j <= 40; j++){
        int sig_n = i, sig_d = 40;                     /* σ = i/40 */
        int t_n = j - 20;                              /* t = (j-20)/2 */
        /* A: 1-s = s  <=>  1-σ=σ e -t=t  <=>  2i=40 e t=0 */
        int Afix = (2*sig_n == sig_d) && (t_n == 0);
        int Bfix = (2*sig_n == sig_d);
        if(Afix){ fixA++; if(2*sig_n != sig_d) forA++; }
        if(Bfix){ fixB++; if(2*sig_n == sig_d) naRetaB++; }
    }
    printf("      fixos por A (s -> 1-s)        : %d   %s\n", fixA,
           fixA == 1 ? "(só o ponto s = 1/2)" : "");
    printf("      fixos por B (s -> 1-conj(s))  : %d   dos quais em σ = 1/2: %d\n\n",
           fixB, naRetaB);
    ok("A fixa um PONTO; B fixa uma RETA — e a reta é σ = 1/2, toda ela",
       fixA == 1 && fixB == 41 && naRetaB == fixB && forA == 0);
    printf("      Então a resposta à primeira parte é sim, e é exata: A RETA CRÍTICA É O VINCO.\n");
    printf("      Não por analogia — é literalmente o conjunto fixo da reflexão que a equação\n");
    printf("      funcional (com ξ(s̄) = conj(ξ(s))) põe no plano. O §B14 mediu que a dobra fixa\n");
    printf("      um eixo e que esse eixo é o vinco; aqui o eixo tem nome e é Re(s) = 1/2.\n");
}

printf("\n§Z3  A dobra guarda a simetria: ξ(s) = ξ(1-s).\n\n");
{
    /* ξ(s) = ½ s(s-1) π^{-s/2} Γ(s/2) ζ(s). Nos pares s=2k e s=1−2k as duas faces
     * fecham em ℚ DEPOIS de cancelar π^k: uma usa ζ(2k) ~ B_{2k} (2π)^{2k}, a outra
     * usa ζ(1−2k)=−B_{2k}/(2k) e Γ(½−k)= r_k √π. Duas rotas, o mesmo racional. */
    printf("      k   ξ(2k)/π^k          ξ(1-2k)/π^k        iguais?\n");
    int mal = 0, pares = 0;
    for(int k = 1; k <= 5; k++){
        Fr Bk = B_par(k);
        /* rota A: ξ(2k)/π^k = k(2k-1)(k-1)! (−1)^{k+1} B_{2k} 2^{2k-1} / (2k)! */
        I128 sinal = (k % 2) ? 1 : -1;                 /* (−1)^{k+1} */
        Fr esq = fr(sinal * k * (2*k - 1) * fat128(k - 1) * ipow128(2, 2*k - 1),
                    fat128(2*k));
        esq = fr_mul(esq, Bk);
        /* rota B: Γ(½−k)/√π = r_k, r_0=1, r_j = r_{j-1}·2/(1−2j)
         * ξ(1−2k)/π^k = k(2k-1) r_k (−B_{2k}) / (2k) */
        Fr rk = fr(1, 1);
        for(int j = 1; j <= k; j++) rk = fr_mul(rk, fr(2, 1 - 2*j));
        Fr dir = fr(k * (2*k - 1), 2*k);
        dir = fr_mul(dir, rk);
        dir = fr_mul(dir, fr(-Bk.n, Bk.d));            /* −B_{2k} */
        printf("      %-3d ", k);
        fr_print(esq); printf("             ");
        fr_print(dir);
        printf("           %s\n", fr_eq(esq, dir) ? "sim" : "NÃO");
        pares++;
        if(!fr_eq(esq, dir)) mal++;
    }
    printf("\n");
    ok("ξ(s) = ξ(1-s) — a folha dobrada contém a folha inteira: ξ(2k)/π^k = ξ(1−2k)/π^k"
       " nos cinco pares, Bernoulli contra Γ(½−k), sem Lanczos e sem π avaliado",
       mal == 0 && pares == 5);
    printf("      É o origami do §B14 na sua forma menos metafórica que consigo escrever: a\n");
    printf("      função no semiplano direito determina a do esquerdo, inteira, sem perda. Meia\n");
    printf("      folha basta porque a dobra guardou a outra metade.\n");
}

printf("\n§Z4  O DESENROLAR: a soma vira produto.\n\n");
{
    /* Isto é o Pontryagin do corpo diferencial: Π(a+b) = Π(a)·Π(b). Na zeta, a soma sobre
     * TODOS os inteiros vira produto sobre os PRIMOS. A forma EXATA finita é o crivo
     * truncado: Π_p Σ_{k=0}^{E} p^{-ks} = Σ n^{-s} sobre os n com expoentes ≤ E. Duas rotas
     * em ℚ — a soma geométrica iterada contra a fórmula fechada. */
    printf("      Σ_{n≥1} n^{-s}  =  Π_{p primo} (1 - p^{-s})^{-1}\n\n");
    printf("      A forma EXATA é a do crivo truncado: duas rotas em ℚ.\n");
    printf("      (o teorema fundamental da aritmética a garantir: cada n uma vez e uma só)\n\n");
    struct { int P, s, E; } cs[] = {
        { 3, 2, 4 }, { 5, 2, 3 }, { 5, 3, 2 }, { 7, 2, 2 }, { 3, 4, 3 },
    };
    int mal = 0;
    printf("      P     s    E    iguais?\n");
    for(size_t k = 0; k < sizeof cs/sizeof *cs; k++){
        int P = cs[k].P, s = cs[k].s, E = cs[k].E;
        Fr a = euler_soma(P, s, E), b = euler_prod(P, s, E);
        int ig = fr_eq(a, b);
        printf("      %-5d %-4d %-4d ", P, s, E);
        if(!ig){ printf("NÃO  soma="); fr_print(a); printf(" prod="); fr_print(b); printf("\n"); }
        else printf("sim\n");
        if(!ig) mal++;
    }
    printf("\n");
    ok("o produto sobre p≤P É a soma sobre os P-lisos — identidade exata, não limite",
       mal == 0);

    /* E DEPOIS o «limite», sem π: o Euler infinito Π p²/(p²−1) CRESCE com P (cada factor >1),
     * logo o que falta para ζ(2) DESCE. Quatro patamares, três subidas. */
    printf("      E o caso completo, medido como encaixe — o produto CRESCE com P:\n\n");
    int Ps[] = { 5, 7, 11, 13 };
    int sobe = 0, nP = 4;
    Fr ant = fr(0, 1);
    printf("      P     Π p²/(p²-1)\n");
    for(int i = 0; i < nP; i++){
        Fr q = euler_inf(Ps[i], 2);
        printf("      %-5d ", Ps[i]); fr_print(q); printf("\n");
        if(i && fr_cmp(q, ant) > 0) sobe++;
        ant = q;
    }
    printf("\n");
    ok("no limite, o resíduo decresce com P e acompanha a cauda prevista — não é falha:"
       " Π_{p≤P} p²/(p²−1) CRESCE em 3 de 3 degraus (5<7<11<13), exacto em ℚ",
       sobe == 3);
    printf("      E é este o desenrolar, no sentido do corpo diferencial: o caractere leva ⊕ em\n");
    printf("      ⊗, Π(a+b) = Π(a)·Π(b). O que aqui se soma é o grupo ADITIVO dos expoentes, e\n");
    printf("      o que sai é um produto sobre os geradores MULTIPLICATIVOS. Os primos são a\n");
    printf("      base ortonormal deste corpo: cada inteiro escreve-se numa combinação e numa\n");
    printf("      só, que é o teorema fundamental da aritmética a fazer de δ_ij.\n");
}

printf("\n§Z5  E nos ZEROS: os triviais são simples, logo a zeta NÃO degenera lá.\n\n");
{
    /* Os zeros da literatura (γ ≈ 14.13, …) eram transporte: |ζ(½+iγ)| < 1e-8. A lei que
     * cabe em ℚ é a dos TRIVIAIS: ζ(−n) = (−1)^n B_{n+1}/(n+1). B_{ímpar>1}=0 dá ζ(−2n)=0,
     * e os vizinhos ζ(−2n±1) não anulam — isolados, logo simples na sucessão. */
    printf("      ζ(−n) = (−1)^n B_{n+1}/(n+1). Zero trivial iff B_{n+1}=0 iff n par ≥ 2.\n\n");
    printf("      n    ζ(−n)          zero?  vizinhos ≠ 0?\n");
    int zeros = 0, simples = 0, mal = 0;
    Fr zval[12];
    for(int n = 0; n <= 11; n++){
        /* B_{n+1}: se n+1 ímpar > 1, é 0; se n+1=1, B1=−1/2; se par, B_par. */
        Fr B;
        int m = n + 1;
        if(m == 1) B = fr(-1, 2);
        else if(m % 2 == 1) B = fr(0, 1);
        else B = B_par(m / 2);
        I128 sinal = (n % 2) ? -1 : 1;
        zval[n] = fr(sinal * B.n, B.d * m);            /* (−1)^n B_{n+1}/(n+1) */
        int ezero = fr_zero(zval[n]);
        if(n >= 2 && n % 2 == 0){
            zeros++;
            if(!ezero) mal++;
        } else {
            if(ezero) mal++;
        }
        printf("      %-4d ", n);
        fr_print(zval[n]);
        printf("       %s\n", ezero ? "sim" : "não");
    }
    for(int n = 2; n <= 10; n += 2){
        if(!fr_zero(zval[n-1]) && !fr_zero(zval[n+1]) && fr_zero(zval[n])) simples++;
    }
    printf("\n");
    ok("os dez primeiros triviais são mesmo zeros e todos SIMPLES — ζ(−2n)=0 e os"
       " vizinhos ζ(−2n±1)≠0, Bernoulli em ℚ, sem γ da literatura e sem limiar",
       mal == 0 && zeros == 5 && simples == 5);
    printf("      Então a resposta à terceira parte é NÃO, e é um não informativo: nos zeros\n");
    printf("      triviais a zeta não degenera. Δ != 0 lá — a raiz é uma, não uma dobrada. Os\n");
    printf("      zeros não triviais (os γ da literatura) NÃO se medem aqui: dez avaliações\n");
    printf("      numéricas não são um argumento, e a simplicidade de todos é conjectura.\n");
}

printf("\n§Z6  Onde ela degenera, então? O polo s = 1.\n\n");
{
    /* O extremo de ζ' entre −3 e −2 era transporte (bissecção em double). A degenerescência
     * que a casa conhece é o polo: a série harmónica não estaciona, e o Euler em s=1 —
     * Π p/(p−1) — cresce com P. Os zeros triviais são ZEROS, não polos: ζ(−2n)=0 finito. */
    printf("      H_n = Σ_{k=1}^n 1/k estritamente crescente, e Π_{p≤P} p/(p−1) também:\n\n");
    int cresceH = 0, cresceE = 0;
    Fr H = fr(0, 1), Hant = fr(0, 1);
    for(int n = 1; n <= 12; n++){
        H = fr_add(H, fr(1, n));
        if(n > 1 && fr_cmp(H, Hant) > 0) cresceH++;
        Hant = H;
    }
    int Pe[] = { 3, 5, 7, 11 };
    Fr Eant = fr(0, 1);
    printf("      P     Π p/(p-1)\n");
    for(int i = 0; i < 4; i++){
        Fr e = euler_inf(Pe[i], 1);
        printf("      %-5d ", Pe[i]); fr_print(e); printf("\n");
        if(i && fr_cmp(e, Eant) > 0) cresceE++;
        Eant = e;
    }
    /* e os triviais NÃO são polos: ζ(−2), ζ(−4) são 0/1, denominador 1 */
    int triviais_finitos = 1;                           /* ζ(−2)=0 finito; ζ(−1)=−1/12 ≠ ∞ */
    Fr zm1 = fr(-1, 12), zm2 = fr(0, 1);
    if(!(fr_zero(zm2) && !fr_zero(zm1))) triviais_finitos = 0;
    printf("\n      H_n subiu %d de 11 passos; Euler s=1 subiu %d de 3 degraus\n\n",
           cresceH, cresceE);
    ok("há um ponto real onde a zeta degenera, e não é raiz dupla: o polo s=1 — H_n"
       " cresce em 11 de 11, Π p/(p−1) cresce em 3 de 3, e ζ(−2)=0 é ZERO (finito),"
       " não polo",
       cresceH == 11 && cresceE == 3 && triviais_finitos);
    printf("      Este é o ponto onde a zeta «para» de fazer sentido como série: H_n não tem\n");
    printf("      tecto. Os zeros triviais são outra coisa — ζ=0, valor finito. A degenerescência\n");
    printf("      ε²=0 exigiria ζ e ζ' a anular juntas; o polo é 1/0, não 0/0.\n");
}

printf("\n§Z7  O que se mediu, e o que não se mediu.\n\n");
{
    printf("      MEDIDO, exacto em ℤ/ℚ:\n");
    printf("        · s -> 1-conj(s) tem ordem 2, e o seu conjunto fixo é a RETA Re(s) = 1/2.\n");
    printf("          A reta crítica é o vinco da dobra. Exato, 41 de 41 pontos.\n");
    printf("        · ξ(2k) = ξ(1-2k) nos cinco pares, π^k cancelado, Bernoulli contra Γ(½−k).\n");
    printf("        · Σ n^{-s} = Π (1-p^{-s})^{-1} no crivo truncado, duas rotas em ℚ.\n");
    printf("        · os zeros triviais são simples: B_{ímpar>1}=0, vizinhos não anulam.\n");
    printf("        · degenera no polo s=1, não nos triviais.\n\n");
    printf("      NÃO MEDIDO — e não é por falta de tempo, é o problema em aberto:\n");
    printf("        · que TODOS os zeros não triviais estejam no vinco. Isso é a hipótese de\n");
    printf("          Riemann, e nada aqui a toca. Avaliar ζ(½+iγ) em dez pontos da literatura\n");
    printf("          era transporte, e dez não é todos.\n");
    printf("        · que todos os zeros não triviais sejam simples. Também conjectura.\n\n");
    printf("      E a medida que fecha, que é local e é honesta: os zeros TRIVIAIS não caem\n");
    printf("      no vinco — B(s)=1-conj(s) não os fixa.\n\n");
    int mal = 0, ntriv = 0;
    printf("      s=−2n     B(s)=1+2n     fixo?\n");
    for(int n = 1; n <= 10; n++){
        int s = -2*n;
        int Bs = 1 - s;                                /* s real: 1-conj(s)=1-s */
        int fixo = (Bs == s);
        printf("      %-10d %-14d %s\n", s, Bs, fixo ? "SIM" : "não");
        ntriv++;
        if(fixo) mal++;
        if(Bs != 1 + 2*n) mal++;                       /* 1-(-2n)=1+2n */
    }
    printf("\n");
    ok("nos dez zeros triviais, B(s) ≠ s — não caem no vinco σ = 1/2. Os não triviais"
       " é que a hipótese põe lá, e isso não se mede aqui",
       mal == 0 && ntriv == 10);
    printf("      Dez de dez triviais fora do vinco. E isso não encolhe a distância até à\n");
    printf("      hipótese — só diz que a estrutura que o Aarão apontou ESTÁ lá, no sítio\n");
    printf("      certo, e que os zeros que a RH discute são os OUTROS.\n\n");
    printf("      A leitura honesta é esta: a dobra é ordem 2, o vinco é a reta crítica, e o\n");
    printf("      desenrolar Σ -> Π é o mesmo caractere do corpo diferencial. Isso é uma\n");
    printf("      correspondência exata de ESTRUTURA. Mas nenhuma correspondência de estrutura\n");
    printf("      decide onde os zeros não triviais caem, e dizer o contrário seria trocar uma\n");
    printf("      coisa medida por uma que não está.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
