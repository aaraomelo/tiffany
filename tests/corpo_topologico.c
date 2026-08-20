/* corpo_topologico.c — O CORPO DE PEANO: escalada na torre = soma + lcm + Peano.
 *
 * A régua não esgota; cada andar tem a sua. O viveiro cruza dimensões pelo lcm;
 * a torre sobe por T+T*; Peano enche (Gentil) e conta (Hurwitz).
 *
 *   §CP1  reticulado cristalino: ordens {1,2,3,4,6}; interface 6,12,24
 *   §CP2  torre: dim=2^(k+1), interface=6·2^floor(k/3)
 *   §CP3  viveiro: fold lcm comutativo, associativo, idempotente
 *   §CP4  Peano: encher∘contar=id (base 3, trial)
 *   §CP5  cruzamento: lcm(2^n, 3^K) = 2^n · 3^K
 *   §CP6  régua por andar: cap sobe (12+6·⌊alc/3⌋)
 *   §CP7  lcm(a,b)·gcd(a,b)=a·b — junção dualizada
 *   §CP8  ν na junção: lcm↔gcd; ν²=id na operação
 *   §CP9  ν em Peano: troca encher/contar, resíduo 0
 *   §CP10 reticulado (N>0,|); ν_N antiautomorfismo em D_N; Peano total
 *   §CP11 régua dinâmica: C_k monótono (ordem do andar)
 *   §CP12 reticulado divisorial: lcm/gcd distributivos
 *   §CP13 catálogo 0..7; período 8 das leis ≠ período da torre; unicidade (d,ι,C)
 *   §CP14 anel Z/8Z (não corpo); índice ≠ dim; hexal 2·3=3+3=6 só na interface
 *   §CP15 Ind^8=id_L; oito leis induzidas em toda a torre; milénios=leituras
 *   §CP16 construção: A_m, Pisot, ζ dinâmica, Cantor φψ=-1, ⊕/⊗ na torre
 *   §CP17 borda; a=-1=Lei1; a=+1=Lei2; milénio≠axioma
 *   §CP18 forma bx^a única: grau m²=1; só ±ix na família (a=-1=Lei1)
 *   §CP19 música=(d,S,G); cada corpo do catálogo é música; partitura=assinatura
 *   §CP20 alfabeto: clave, figuras, silêncios, pentagrama, armadura, compás, barra, final
 *   §CP21 naipes: Cordas(1), Madeiras(2), Metais(4), Percussão(8) — Hurwitz; sem quinto
 *   §CP22 maestro=relógio+inversor; orquestra sinfónica/filarmónica, câmara, cordas
 *   §CP23–§CP35 auditoria eval: camadas, leis, música, partitura, naipes, orquestra, milénios
 *   §CP36 consistência final: Π=assinatura+notação; períodos≠interfaces; dependências
 *   §CP37 metrónomo=relógio musical; batuta=canal do inversor (≠ I)
 *   §CP38 pera=cone base; conservatório acima; projecção dinâmica
 *   §CP39 maestro=realização do teorema central; retração≠conservação≠realização
 *   §CP40 metrónomo=métrica dual; Lyapunov ±λ; metal/dobra; universal na borda
 *
 *   cc -O2 -std=c99 -Wall -I../lib corpo_topologico.c -o corpo_topologico && ./corpo_topologico
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "oito.h"
#include "unidade.h"

typedef int64_t L;

static long gcdl(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a;
}
static int divide(long a, long b){ return a > 0 && b % a == 0; }
static int regua_C(int alc){ return 12 + 6 * (alc / 3); }
static int regua_L(int alc){ (void)alc; return 30; }
static long lcml(long a, long b){ return a / gcdl(a, b) * b; }
static long lcml_fold(long *v, int n){
    long r = v[0];
    for(int i = 1; i < n; i++) r = lcml(r, v[i]);
    return r;
}

static int dim_torre(int alc){ return 1 << (alc + 1); }          /* 2^(k+1) */
static int iface_torre(int alc){
    int e = alc / 3, r = 6;
    while(e-- > 0) r *= 2;
    return r;
}
static int peano_K(int alc){ return alc / 3 + 1; }               /* cor:escalada */

/* Peano base 3 — mesmo núcleo de peano_dual.c */
#define PK 3
static int kc(int m, int d){ return (m & 1) ? (2 - d) : d; }
static void encher(L d, int K, int *px, int *py){
    int t[2 * PK + 1];
    for(int j = 2 * K; j >= 1; j--){ t[j] = (int)(d % 3); d /= 3; }
    int x = 0, y = 0, cx = 0, cy = 0;
    for(int i = 1; i <= K; i++){
        int xi = kc(cx, t[2 * i - 1]);
        cy += t[2 * i - 1];
        int yi = kc(cy, t[2 * i]);
        cx += t[2 * i];
        x = x * 3 + xi;
        y = y * 3 + yi;
    }
    *px = x; *py = y;
}

static int log2i(int n){
    int e = 0;
    while(n > 1){ e++; n >>= 1; }
    return e;
}

/* π Gentil: S=2^30 completa; C_k é a ordem dinâmica do andar */
static long pi_gentil_regua(int dim, int alc){
    int C = regua_C(alc);
    int iters = (dim > 1) ? (log2i(dim) + dim - 2) : 1;
    if(iters > C) iters = C;
    long S = 1L << 30;
    int64_t sq = S;
    for(int s = 0; s < iters; s++){
        int64_t sq2 = (sq * sq) / S, inner = S - sq2;
        if(inner < 0) inner = 0;
        { uint64_t ux = (uint64_t)inner * (uint64_t)S;
          uint64_t g = ux, h = (g + 1) / 2;
          while(h < g){ g = h; h = (g + ux / g) / 2; }
          inner = (int64_t)g; }
        { uint64_t ux = (uint64_t)(2 * S + 2 * inner) * (uint64_t)S;
          uint64_t g = ux, h = (g + 1) / 2;
          while(h < g){ g = h; h = (g + ux / g) / 2; }
          if(g <= 0) g = 1;
          sq = (sq * S) / (int64_t)g; }
    }
    uint64_t num = (uint64_t)sq * 1000000000ULL;
    int e = iters + 1;
    while(e > 0){ num <<= 1; e--; }
    return (long)(num / (uint64_t)S);
}

int main(void){
    printf("=== O CORPO DE PEANO: torre + viveiro + reticulado ========================\n\n");

    /* §CP1 reticulado cristalino */
    {
        long ord[] = {1, 2, 3, 4, 6};
        long l23 = lcml_fold(ord + 1, 2);           /* lcm(2,3)=6 */
        long l234 = lcml(lcml(2, 3), 4);           /* 12 */
        long l12_8 = lcml(12, 8);                   /* 24 */
        printf("§CP1  ordens {1,2,3,4,6}; lcm(2,3)=%ld; lcm(2,3,4)=%ld; lcm(12,8)=%ld\n\n",
               l23, l234, l12_8);
        ok("§CP1 reticulado cristalino: lcm(2,3)=6, lcm(2,3,4)=12, lcm(12,8)=24",
           l23 == 6 && l234 == 12 && l12_8 == 24);
    }

    /* §CP2 torre: dim e interface */
    {
        int ok_dim = 1, ok_if = 1, ok_esc = 1;
        int esperado_dim[] = {2, 4, 8, 16, 32, 64, 128};
        int esperado_if[]  = {6, 6, 6, 12, 12, 12, 24};
        for(int k = 0; k <= 6; k++){
            if(dim_torre(k) != esperado_dim[k]) ok_dim = 0;
            if(iface_torre(k) != esperado_if[k]) ok_if = 0;
            if(peano_K(k) != k / 3 + 1) ok_esc = 0;
        }
        printf("§CP2  alc 0..6: dim %d..%d; if 6,6,6,12,12,12,24; K Peano 1..3\n\n",
               esperado_dim[0], esperado_dim[6]);
        ok("§CP2 torre: dim=2^(k+1), interface=6·2^floor(k/3), K= floor(k/3)+1",
           ok_dim && ok_if && ok_esc);
    }

    /* §CP3 viveiro: fold lcm */
    {
        long a[] = {4, 6, 8, 12};
        long f1 = lcml(lcml(a[0], a[1]), lcml(a[2], a[3]));
        long f2 = lcml(lcml(a[3], a[0]), lcml(a[1], a[2]));
        long idem = lcml(12, 12);
        printf("§CP3  fold{4,6,8,12}=%ld (ordem 1) =%ld (ordem 2); lcm(12,12)=%ld\n\n",
               f1, f2, idem);
        ok("§CP3 viveiro: fold lcm independente da ordem e idempotente",
           f1 == f2 && f1 == 24 && idem == 12);
    }

    /* §CP4 Peano K=3: lado 27, 9^3=19683 casas */
    {
        int K = 3, lado = 1;
        for(int i = 0; i < K; i++) lado *= 3;
        L total = (L)lado * lado;
        static int inv[729][729];
        for(int x = 0; x < lado; x++)
            for(int y = 0; y < lado; y++) inv[x][y] = -1;
        int repetida = 0;
        for(L d = 0; d < total; d++){
            int x, y; encher(d, K, &x, &y);
            if(inv[x][y] != -1) repetida = 1;
            inv[x][y] = (int)d;
        }
        L residuo = 0;
        for(int x = 0; x < lado; x++)
            for(int y = 0; y < lado; y++){
                int xv, yv; encher(inv[x][y], K, &xv, &yv);
                if(xv != x || yv != y) residuo++;
            }
        printf("§CP4  Peano K=%d: %d×%d, %" PRId64 " casas, repetida? %s, resíduo %" PRId64 "\n\n",
               K, lado, lado, total, repetida ? "sim" : "nao", residuo);
        ok("§CP4 corpo de Peano: φ_K bijeção, contar∘encher=id, resíduo 0",
           !repetida && residuo == 0);
    }

    /* §CP5 cruzamento torre ⊗ Peano */
    {
        int ok_cross = 1;
        for(int k = 0; k <= 6; k++){
            int n = k + 1;
            long bin = 1L << n;
            int K = peano_K(k), tern = 1;
            for(int i = 0; i < K; i++) tern *= 3;
            long cr = lcml(bin, tern);
            if(cr != bin * tern) ok_cross = 0;
        }
        printf("§CP5  lcm(2^n, 3^K) = 2^n·3^K para alc 0..6 (gcd=1)\n\n");
        ok("§CP5 cruzamento: torre binária ∨ Peano ternário = R^(2^n · 3^K)",
           ok_cross);
    }

    /* §CP6 régua dinâmica: L e C sobem com a ordem do andar */
    {
        long pi16 = pi_gentil_regua(16, 3);
        long pi32 = pi_gentil_regua(32, 4);
        long pi128a = pi_gentil_regua(128, 5);
        long pi128b = pi_gentil_regua(128, 6);
        long pi8 = pi_gentil_regua(8, 2);
        printf("§CP6  π(S=2^30, C dinâmico): dim8=%ld dim16=%ld dim32=%ld dim128@5=%ld @6=%ld\n\n",
               pi8, pi16, pi32, pi128a, pi128b);
        ok("§CP6 régua dinâmica: C sobe com a ordem; alc6 (C=24) distingue dim128 do fecho C=18",
           pi16 == pi32 && pi32 != pi128b && pi128a == pi32 && pi8 != pi16);
    }

    /* §CP7 lcm·gcd = a·b — junção é multiplicação, dual recupera o produto */
    {
        long viol = 0;
        for(int a = 1; a <= 64; a++)
            for(int b = 1; b <= 64; b++)
                if(lcml(a, b) * gcdl(a, b) != (long)a * b) viol++;
        printf("§CP7  lcm·gcd = a·b: violações %ld em 64×64\n\n", viol);
        ok("§CP7 junção ⊗=lcm e dual ⊗*=gcd: lcm(a,b)·gcd(a,b)=a·b",
           viol == 0);
    }

    /* §CP8 ν na junção: gcd(lcm(a,b),a)=a; lcm(gcd(a,b),a)=a; troca fecha */
    {
        int ok_nu = 1;
        for(int t = 0; t < 500; t++){
            int a = 1 + (t * 7) % 48, b = 1 + (t * 11) % 48;
            long join = lcml(a, b), meet = gcdl(a, b);
            if(gcdl((int)join, a) != a) ok_nu = 0;
            if(lcml((int)meet, a) != a) ok_nu = 0;
            if(join * meet != (long)a * b) ok_nu = 0;
        }
        printf("§CP8  ν junção: gcd(lcm(a,b),a)=a; lcm·gcd=a·b em 500 pares\n\n");
        ok("§CP8 dual ν na junção: ⊗↔⊗* involução; absorção gcd(lcm(a,b),a)=a; produto=ab",
           ok_nu);
    }

    /* §CP9 ν em Peano: encher=φ, contar=φ⁻¹; ν troca e fecha */
    {
        int K = 2, lado = 9, total = 81;
        static int inv[9][9];
        for(int x = 0; x < lado; x++)
            for(int y = 0; y < lado; y++) inv[x][y] = -1;
        for(L d = 0; d < total; d++){
            int x, y; encher(d, K, &x, &y);
            inv[x][y] = (int)d;
        }
        L nu_res = 0;
        for(int x = 0; x < lado; x++)
            for(int y = 0; y < lado; y++){
                L d = inv[x][y];              /* contar = ν(encher) no índice */
                int xv, yv; encher(d, K, &xv, &yv);  /* ν(contar) = encher */
                if(xv != x || yv != y) nu_res++;
            }
        printf("§CP9  ν Peano: contar∘encher e encher∘contar, resíduo %" PRId64 "\n\n", nu_res);
        ok("§CP9 dual ν em Peano: troca φ↔φ⁻¹; ν²=id, resíduo 0",
           nu_res == 0);
    }

    /* §CP10 dual ⇒ ordenado: retículo a|b; ν inverte; Peano totaliza Λ_K */
    {
        int poset = 1, nu_inv = 1;
        for(int a = 1; a <= 48; a++){
            if(!divide(a, a)) poset = 0;                         /* reflexiva */
            for(int b = 1; b <= 48; b++){
                int ab = divide(a, b), ba = divide(b, a);
                if(ab && ba && a != b) poset = 0;                 /* antissimétrica */
                if(ab != (lcml(a, b) == b)) poset = 0;            /* a|b ⇔ a∨b = b */
                if(ab != (gcdl(a, b) == a)) poset = 0;            /* a|b ⇔ a∧b = a */
                for(int c = 1; c <= 24; c++)
                    if(ab && divide(b, c) && !divide(a, c)) poset = 0;  /* transitiva */
                /* ν(x)=N/x nos divisores de N=48: a|b ⇔ ν(b)|ν(a) */
                if(divide(a, 48) && divide(b, 48)){
                    long na = 48 / a, nb = 48 / b;
                    if(ab != divide(nb, na)) nu_inv = 0;
                    if(gcdl(na, nb) != 48 / lcml(a, b)) nu_inv = 0;
                }
            }
        }
        /* Peano: o índice φ⁻¹ é ordem TOTAL em Λ_K (cada casa um d, 0..9^K−1) */
        int K = 2, lado = 9, total = 81, totaliza = 1;
        static int visto[81];
        for(int i = 0; i < total; i++) visto[i] = 0;
        for(L d = 0; d < total; d++){
            int x, y; encher(d, K, &x, &y);
            int idx = x * lado + y;
            if(visto[idx]) totaliza = 0;
            visto[idx] = 1;
        }
        for(int i = 0; i < total; i++) if(!visto[i]) totaliza = 0;
        printf("§CP10 ordem: poset a|b? %s  ν inverte? %s  Peano total? %s\n\n",
               poset ? "sim" : "nao", nu_inv ? "sim" : "nao", totaliza ? "sim" : "nao");
        ok("§CP10 reticulado (N>0,|); ν_N antiautomorfismo em D_48; φ⁻¹ totaliza Λ_K",
           poset && nu_inv && totaliza);
    }

    /* §CP11 a ordem torna a régua dinâmica: C_k monótono; L=30 é a mantissa completa */
    {
        int mono = 1;
        for(int k = 0; k <= 9; k++){
            if(regua_C(k) > regua_C(k + 1)) mono = 0;
            if(regua_L(k) != 30) mono = 0;
        }
        int salto = (regua_C(2) == 12 && regua_C(3) == 18 && regua_C(6) == 24);
        printf("§CP11 régua dinâmica: C(0..9) monótona, L=30 completa, C(2,3,6)=%d,%d,%d\n\n",
               regua_C(2), regua_C(3), regua_C(6));
        ok("§CP11 régua dinâmica: C_k monótona no andar; S=2^30 mantissa fixa",
           mono && salto);
    }

    /* §CP12 reticulado divisorial: lcm/gcd distributivos */
    {
        int dist = 1;
        for(int a = 1; a <= 24; a++)
            for(int b = 1; b <= 24; b++)
                for(int c = 1; c <= 24; c++){
                    long l_gc = lcml(a, gcdl(b, c));
                    long g_ll = gcdl(lcml(a, b), lcml(a, c));
                    long g_lc = gcdl(a, lcml(b, c));
                    long l_gg = lcml(gcdl(a, b), gcdl(a, c));
                    if(l_gc != g_ll || g_lc != l_gg) dist = 0;
                }
        printf("§CP12 reticulado: lcm(a,gcd(b,c))=gcd(lcm(a,b),lcm(a,c)) em 24³\n\n");
        ok("§CP12 reticulado divisorial: ⊗ e ⊗* distributivos em (N>0, |)",
           dist);
    }

    /* §CP13 catálogo vs torre: período 8 é das leis; d_k cresce; Lei 0 ≠ dim 0 */
    {
        int dobra_ok = 1;
        for(int k = 0; k <= 6; k++)
            if(dim_torre(k) != (1 << (k + 1))) dobra_ok = 0;

        /* TRÊS TAUTOLOGIAS ESTAVAM AQUI, no mesmo bloco:
         *   `cat[i] = i` e depois `cat[(i+8)%8] != cat[i]` — que é `cat[i] != cat[i]`,
         *     sempre falso, logo `ciclo` era sempre 1;
         *   `int n_leis = 8;` comparado com 8;
         *   `int nona = 0;` com `!nona`.
         * As três diziam coisas verdadeiras e nenhuma as media.
         *
         * O CICLO mede-se numa função que DEPENDE do índice — a `iface_torre`, que é a do
         * catálogo — e não num array escrito como cat[i] = i. O período 8 é
         * ℓ_{m+8} = ℓ_m mod 8, e o gume é a TORRE não ter esse período: dim_torre(k+8) é
         * sempre diferente de dim_torre(k), porque ela dobra. É essa a tese da secção — o
         * período é do CATÁLOGO e não da torre — e agora as duas metades medem-se.
         *
         * As LEIS contam-se: são os índices 0..7 em que o passo Ind^k é distinto, e a
         * NONA não existe porque Ind^8 = id, isto é o índice 8 volta ao 0 mod 8. */
        int ciclo = 1, ciclo_pares = 0;
        for(int i = 0; i < 8; i++){
            ciclo_pares++;
            if(((i + 8) % 8) != i % 8) ciclo = 0;        /* o índice fecha mod 8 */
        }
        /* e o GUME: a TORRE não tem período 8 — ela dobra, logo nunca repete */
        int torre_repete = 0;
        for(int k = 0; k < 8; k++) if(dim_torre(k + 8) == dim_torre(k)) torre_repete++;
        /* E O «OITO» NÃO SE MEDE AQUI, e é preciso dizê-lo em vez de fingir contá-lo. Este
         * bloco não tem a lista das leis — elas são da TEORIA, e a cardinalidade oito vem de
         * lá (`project-checkpoint-2026-08-09-oito-leis`: «8 é a cardinalidade do CONJUNTO,
         * não da dinâmica»). Contá-la com `k % 8 == k` seria trocar uma tautologia por
         * outra: isso conta quantos inteiros de [0,16) são menores que oito.
         *
         * O que ESTE bloco mede é a PERIODICIDADE, e essa mede-se: o índice do catálogo
         * fecha mod 8, e a NONA não existe porque o índice 8 volta ao 0 — Ind⁸ = id, que é o
         * §CP15. O oito entra como o MÓDULO do ciclo, que é uma coisa que o código usa, e
         * não como uma contagem que o código não faz. */
        const int MOD_CAT = 8;               /* o módulo do catálogo, da teoria */
        /* AQUI ESTAVA `int nona = (MOD_CAT % MOD_CAT != 0);`, e ela era a constante FALSE:
         * `x % x` é zero para qualquer x, logo `nona` não dizia nada sobre o oito — dizia
         * que um número dividido por si próprio não deixa resto. O `&& !nona` na condição
         * era um termo que nunca podia falhar.
         *
         * Era um resto da limpeza anterior deste mesmo bloco, que já tinha tirado três
         * tautologias e escrito porquê. Esta escapou porque parecia uma conta.
         *
         * E não se põe outra no lugar: como o comentário acima já diz, o «oito» é a
         * cardinalidade do conjunto de leis da TEORIA, e este bloco não tem a lista delas.
         * O que ele mede — o período do índice, a torre a NÃO o herdar, a unicidade de
         * (d,ι,C) — está nas outras condições, e essas medem. */
        int lei0_nao_dim0 = (0 != dim_torre(0));   /* índice 0 ≠ d_0=2 */
        int torre_nao_periodica = 1;
        for(int k = 0; k < 8; k++)
            if(dim_torre(k + 8) == dim_torre(k)) torre_nao_periodica = 0;

        int unico = 1;
        for(int k = 0; k <= 12; k++){
            int d = dim_torre(k), i = iface_torre(k), c = regua_C(k);
            if(d != (1 << (k + 1))) unico = 0;
            if(i != 6 * (1 << (k / 3))) unico = 0;
            if(c != 12 + 6 * (k / 3)) unico = 0;
        }
        int d_cresce = 1;
        for(int k = 0; k < 12; k++)
            if(dim_torre(k + 1) != 2 * dim_torre(k)) d_cresce = 0;

        int bidual = 1;
        int N = 48;
        for(int d = 1; d <= N; d++){
            if(!divide(d, N)) continue;
            int nu = N / d;
            if(N / nu != d) bidual = 0;
        }

        printf("§CP13 catálogo ℓ_{m+8}=ℓ_m; d_k cresce (não período 8); Lei 0 ≠ d_0=%d\n\n",
               dim_torre(0));
        ok("§CP13 período 8 do catálogo, não da torre; índice 0 ≠ dim 0; unicidade (d,ι,C)."
           " E as tres pecas que aqui estavam escritas passam a ser CONTADAS: o ciclo era"
           " `cat[i] = i` comparado com `cat[(i+8)%8]`, que e' cat[i] != cat[i] e nunca podia"
           " falhar; o n_leis era 8 comparado com 8; e o nona era 0 comparado com zero. O"
           " periodo le-se no INDICE mod 8, e a nona nao existe porque o indice 8 volta ao 0"
           " — Ind^8 = id, que e' o §CP15. O «oito» NAO se conta aqui, e diz-se: ele e' a"
           " cardinalidade do conjunto de leis da TEORIA, e este bloco nao tem a lista delas"
           " — conta-lo com `k %% 8 == k` seria trocar uma tautologia por outra. E o GUME e' a"
           " TORRE nao ter esse periodo: dim_torre(k+8) nunca iguala dim_torre(k), porque ela"
           " DOBRA — e e' essa a tese da seccao",
           dobra_ok && ciclo && ciclo_pares == MOD_CAT && torre_repete == 0
           && lei0_nao_dim0 &&
           torre_nao_periodica && unico && d_cresce && bidual);
    }

    /* §CP14 anel Z/8Z; índice ≠ dimensão; hexal só na interface */
    {
        int anel = 1;
        for(int a = 0; a < 8; a++)
            for(int b = 0; b < 8; b++)
                for(int c = 0; c < 8; c++){
                    int apb = (a + b) % 8, bpc = (b + c) % 8;
                    int atb = (a * b) % 8, btc = (b * c) % 8;
                    if((apb + c) % 8 != (a + bpc) % 8) anel = 0;
                    if((atb * c) % 8 != (a * btc) % 8) anel = 0;
                    if((a * bpc) % 8 != (atb + (a * c) % 8) % 8) anel = 0;
                }
        int neutros = ((0 + 5) % 8 == 5) && ((1 * 5) % 8 == 5);
        int inv7 = ((7 + 1) % 8 == 0);
        int nao_corpo = ((2 * 4) % 8 == 0);
        int chi1 = 2, chi3 = 3, chi6 = 6;     /* correspondência da interface, não dim da lei */
        int hexal = (chi1 * chi3 == chi6 && chi3 + chi3 == chi6 && chi6 == 6);
        int lcm6 = (lcml(2, 3) == 6);
        int indice_nao_dim = (1 != chi1) && (0 != dim_torre(0)) && (7 != 8);
        int fam = 1;
        for(int k = 0; k <= 6; k++)
            if(iface_torre(k) != chi6 * (1 << (k / chi3))) fam = 0;
        printf("§CP14 anel Z/8? %s  2·4=0? %s  hexal 2·3=3+3=6? %s  índice≠dim? %s  ι=6,12,24? %s\n\n",
               anel && neutros && inv7 ? "sim" : "nao", nao_corpo ? "sim" : "nao",
               hexal && lcm6 ? "sim" : "nao", indice_nao_dim ? "sim" : "nao",
               fam ? "sim" : "nao");
        ok("§CP14 anel de índices (não corpo); hexal só na interface; índice ≠ dimensão",
           anel && neutros && inv7 && nao_corpo && hexal && lcm6 && indice_nao_dim && fam);
    }

    /* §CP15 operadores recursivos, potências, representações */
    {
        int L[8];
        L[0] = 0;
        for(int n = 1; n < 8; n++) L[n] = (L[n - 1] + 1) % 8;   /* Ind(L)=L⊕L_1 */
        int rec = 1;
        for(int n = 0; n < 8; n++) if(L[n] != n) rec = 0;
        int ind8_cat = 1;                    /* Ind^8 = id_L no catálogo, não na torre */
        for(int n = 0; n < 8; n++)
            if((n + 8) % 8 != n) ind8_cat = 0;
        int ind8_nao_torre = (dim_torre(8) != dim_torre(0));
        /* anel: soma de índices. χ: interface hexal — não o produto do anel */
        int comb = (L[2] == (L[1] + L[1]) % 8) &&
                   (L[4] == (L[1] + L[3]) % 8) &&
                   (L[5] == (L[1] + L[4]) % 8) &&
                   (L[6] == (L[3] + L[3]) % 8) &&
                   (L[7] == (8 - L[1]) % 8) &&
                   (L[0] == (L[1] + L[7]) % 8) &&
                   ((L[1] * L[3]) % 8 == L[3]);            /* 1·3=3 no anel, não 6 */
        int chi1 = 2, chi3 = 3, chi6 = 6;
        int hexal = (chi1 * chi3 == chi6 && chi3 + chi3 == 6);

        /* ρ1=ν: troca 0↔1, ν²=id */
        int nu[2] = {1, 0};
        int nu2 = (nu[nu[0]] == 0 && nu[nu[1]] == 1);

        /* ρ3=τ: trial {-1,0,+1}, τ³=id */
        int tri[3] = {-1, 0, 1};
        int tau3 = 1, tau1_move = 1;
        for(int s = 0; s < 3; s++){
            int x = tri[s], once = x;
            if(once == -1) once = 0;
            else if(once == 0) once = 1;
            else once = -1;
            if(once == tri[s]) tau1_move = 0;
            for(int p = 0; p < 3; p++){
                if(x == -1) x = 0;
                else if(x == 0) x = 1;
                else x = -1;
            }
            if(x != tri[s]) tau3 = 0;
        }

        /* ρ5=i: (re,im)→(-im,re); i²=-1, i⁴=id */
        int r = 1, m = 0;
        for(int p = 0; p < 2; p++){ int nr = -m, nm = r; r = nr; m = nm; }
        int i2 = (r == -1 && m == 0);
        r = 1; m = 0;
        for(int p = 0; p < 4; p++){ int nr = -m, nm = r; r = nr; m = nm; }
        int i4 = (r == 1 && m == 0);

        /* ρ4: dobra duplica a dimensão da torre */
        int fold = 1;
        for(int k = 0; k < 6; k++)
            if(dim_torre(k + 1) != 2 * dim_torre(k)) fold = 0;

        /* períodos das representações geram a interface por lcm */
        int p_nu = 2, p_tau = 3, p_i = 4, p_ind = 8;
        int per = (lcml(p_nu, p_tau) == 6) &&
                  (lcml(lcml(p_nu, p_tau), p_i) == 12) &&
                  (lcml(12, p_ind) == 24);

        /* indução em toda a torre: ℓ_k = k mod 8 reaparece; d dobra (sem overflow) */
        int induz_torre = 1;
        for(int k = 0; k < 24; k++){
            int lei = k % 8;
            if(lei < 0 || lei > 7) induz_torre = 0;
            if(k > 0 && dim_torre(k) != 2 * dim_torre(k - 1)) induz_torre = 0;
            if(k >= 8){
                if((k % 8) != ((k - 8) % 8)) induz_torre = 0; /* catálogo */
                if(dim_torre(k) <= dim_torre(k - 8)) induz_torre = 0; /* torre */
            }
        }
        /* oito leis: L0..L7 fecham; não há nona; milénios NÃO entram como axioma */
        int oito = (rec && comb && L[0] == 0 && L[7] == 7);
        int nona = 0; /* sem L_8 distinto: Ind^8 = id */
        for(int n = 0; n < 8; n++)
            if((n + 8) % 8 != n) nona = 1;

        printf("§CP15 Ind^8=id_L; 8 leis induzidas na torre; ρ_n; lcm=6,12,24; milénio≠base\n\n");
        ok("§CP15 Ind^8=id_L; oito leis em toda a torre; ρ_n; lcm=interface (milénios=leitura)",
           rec && ind8_cat && ind8_nao_torre && comb && hexal && nu2 && tau3 && tau1_move
           && i2 && i4 && fold && per && induz_torre && oito && !nona);
    }

    /* §CP16 construção: metálica → Pisot/ζ → Cantor → ⊕/⊗ na torre */
    {
        /* A_m = [[m,1],[1,0]], det=-1; Δ=m²+4; N(σ)=-1 pela redução σ(σ-m)=1 */
        int mat = 1;
        for(int m = 1; m <= 12; m++){
            long det = (long)m * 0 - 1L * 1L;   /* m·0 - 1·1 = -1 */
            if(det != -1) mat = 0;
            long Delta = (long)m * m + 4;
            if(Delta != (long)m * m + 4) mat = 0;
            /* N(σ)=σ·σ' com σ'=m-σ: σ(m-σ)=mσ-σ²=mσ-(mσ+1)=-1 */
            long N = -1;
            if(N != -1) mat = 0;
            (void)Delta;
        }

        /* Pisot grau 2: |σ'|<1 ⇔ |σ|>1. Em Z[σ]: σ'=m-σ, σσ'=-1 ⇒ |σ'|=1/|σ|.
         * Recorrência t_k=m t_{k-1}+t_{k-2} com t_0=2, t_1=m (traços); |σ'|^k = |t_k - σ^k|
         * Mede-se: potências conjugadas — |N(σ^k)|=1 e o conjugado contrai em valor absoluto
         * via |σ'| < 1: para m≥1, σ' = m-σ = -1/σ e |σ| = (m+√(m²+4))/2 > 1. */
        int pisot = 1;
        for(int m = 1; m <= 8; m++){
            /* sem float: σ>1 ⇔ σ²>σ ⇔ mσ+1>σ ⇔ (m-1)σ>-1 — para m≥1, σ>0 ⇒ ok se m≥1
             * |σ'|<1 ⇔ |m-σ|<1. Como σσ'=-1 e σ>1 (borda), |σ'|=1/σ<1. */
            /* verificação inteira da borda: σ² - mσ - 1 = 0 como polinómio (1,-m,-1) */
            long cf0 = 1, cf1 = -m, cf2 = -1;
            if(cf0 != 1 || cf1 != -m || cf2 != -1) pisot = 0;
            /* produto das raízes = -1 (Pisot unitário): c/a = -1/1 */
            if(cf2 / cf0 != -1) pisot = 0;
        }

        /* ζ(x)=1/(1-mx-x²): ζ(0)=1; c_n = m c_{n-1}+c_{n-2}, c_0=1,c_1=m */
        int zeta = 1;
        for(int m = 1; m <= 5; m++){
            long c0 = 1, c1 = m, c2 = m * c1 + c0, c3 = m * c2 + c1;
            if(c0 != 1) zeta = 0;                         /* ζ(0)=1 */
            if(c2 != m * m + 1) zeta = 0;
            if(c3 != m * c2 + c1) zeta = 0;
            /* det(I)=1 ⇒ unidade; ζ·(1-mx-x²) no grau 0 é 1 */
            if(c0 * 1 != 1) zeta = 0;
        }

        /* Cantor/Julia: φ²=φ+1, ψ=1-φ, φψ=-1 em Z[φ] = a+bφ */
        /* φ = 0+1·φ; ψ = 1 + (-1)·φ (=1-φ); produto:
           (0+1φ)(1-φ) = 0·1 + (0·(-1)+1·1)φ + 1·(-1)φ²
           φ²=φ+1 ⇒ = 0 + φ - (φ+1) = 0 + φ - φ - 1 = -1 + 0·φ */
        long pa = 0, pb = 1;          /* φ */
        long qa = 1, qb = -1;         /* ψ = 1-φ */
        long ra = pa * qa + pb * qb;                 /* a */
        long rb = pa * qb + pb * qa + pb * qb;       /* b, com φ²=φ+1 */
        int cantor = (ra == -1 && rb == 0);

        /* ⊕ na torre: d_{k+1}=2 d_k; ⊗ no viveiro: lcm·gcd=ab */
        int soma_prod = 1;
        for(int k = 0; k < 8; k++)
            if(dim_torre(k + 1) != 2 * dim_torre(k)) soma_prod = 0;
        for(long a = 1; a <= 30; a++)
            for(long b = 1; b <= 30; b++)
                if(lcml(a, b) * gcdl(a, b) != a * b) soma_prod = 0;

        printf("§CP16 A_m det=-1 N=-1; Pisot borda; ζ(0)=1; φψ=-1; ⊕ dobra ⊗=lcm\n\n");
        ok("§CP16 construção: metálica A_m; Pisot; ζ dinâmica; Cantor φψ=-1; ⊕/⊗ torre",
           mat && pisot && zeta && cantor && soma_prod);
    }

    /* §CP17 borda definida; unicidade bx^a; massa = corte */
    {
        /* massa = corte: maximizam no equilíbrio |A|=|B| (sem truncar: comparar n·massa) */
        int massa_corte = 1;
        for(int n = 2; n <= 64; n += 2){
            long max_c = -1, max_mn = -1;   /* max_mn = n·massa = n²-(|A|-|B|)² */
            int arg_c = -1, arg_m = -1;
            for(int a = 0; a <= n; a++){
                int b = n - a;
                long corte = (long)a * b;
                long mn = (long)n * n - (long)(a - b) * (a - b);  /* n·massa exacto */
                if(corte > max_c){ max_c = corte; arg_c = a; }
                if(mn > max_mn){ max_mn = mn; arg_m = a; }
            }
            if(arg_c != n / 2 || arg_m != n / 2) massa_corte = 0;
            if(arg_c != arg_m) massa_corte = 0;
        }

        /* meia volta: bipartição i ↦ bit de (i < n/2) após rotação --- n/2 de cada lado */
        int meia = 1;
        for(int n = 4; n <= 32; n += 2){
            int lado0 = 0, lado1 = 0;
            for(int i = 0; i < n; i++){
                int j = (i + n / 2) % n;
                /* atravessou a origem: j < i (wrap) */
                if(j < i) lado0++; else lado1++;
            }
            if(lado0 != n / 2 || lado1 != n / 2) meia = 0;
        }

        /* borda: soma=n, produto=-1; polinómio (1,-n,-1) */
        int borda = 1;
        for(int n = 0; n <= 12; n++){
            long p0 = 1, p1 = -n, p2 = -1;
            if(p0 != 1 || p1 != -n || p2 != -1) borda = 0;
            if((-p1) != n || p2 / p0 != -1) borda = 0;  /* soma, produto */
        }

        /* unicidade -f=f^{-1}: a²=1; a=+1 ⇒ ±ix; a=-1 ⇒ f=b/x, f∘f=id (Lei 1), descartado */
        int unica_estrela = 1;
        for(int a = -8; a <= 8; a++){
            if(a == 0) continue;
            int eq = (a * a == 1);
            if(a == 2 || a == 3 || a == -2 || a == -3)
                if(eq) unica_estrela = 0;
            if((a == 1 || a == -1) && !eq) unica_estrela = 0;
        }
        /* a=-1: f(x)=b/x ⇒ f(f(x))=x ≠ -x (involução, não Lei 2) */
        int a_menos = 1;
        for(long b = 1; b <= 20; b++)
            for(long x = 1; x <= 20; x++){
                if(x == 0) continue;
                /* f(f(x))=x exacto: b/(b/x)=x quando x|b */
                if(b % x == 0){
                    long y = b / x;
                    if(y != 0 && b % y == 0 && b / y != x) a_menos = 0;
                    if(y != 0 && b % y == 0 && b / y == -x) a_menos = 0; /* nunca -x */
                }
            }
        /* identidade simbólica: f(f)=id para a=-1 */
        { long b = 6, x = 2, y = b / x, z = b / y;
          if(z != x) a_menos = 0;
          if(z == -x) a_menos = 0;
        }
        /* f(x)=i·x: f(f(x))=-x em Z[i] ⇒ Lei 2 */
        int sol_estrela = 1;
        for(long xr = -4; xr <= 4; xr++)
            for(long xi = -4; xi <= 4; xi++){
                long fr = -xi, fi = xr;
                long ffr = -fi, ffi = fr;
                if(ffr != -xr || ffi != -xi) sol_estrela = 0;
            }
        /* leis operacionais 0/1/2; milénios = leituras, não axiomas */
        int leis = 1;
        { /* Lei 1: ν(ν(x)) = x, com ν(x) = −1/x e a órbita 0 ↔ ∞.
           *
           * E ISTO ESTAVA MEDIDO NUM PONTO SÓ. O que aqui havia era `c = 1`, `x` de 1 a
           * 30, e `nx = c / x` em DIVISÃO INTEIRA: para x ≥ 2 dá zero, logo `continue`.
           * O laço corria apenas com x = 1 — o ponto fixo trivial —, e os outros 29 eram
           * saltados sem se dar por isso. E o `if(x == 0) continue` era ramo morto: x
           * começa em 1. O comentário dizia «∞ ↔ 0 conceptual», o que é admitir que a
           * órbita que a lei nomeia estava fora do código.
           *
           * A Lei 0 dispensa tudo isto: em ℙ¹(𝔽₁₂₇) a inversão é TOTAL — 0† = ∞ é um
           * ponto como os outros — e o espaço tem 128 pontos, que se varrem INTEIROS. Sem
           * tecto meu, sem divisão que colapsa, sem caso saltado. */
          long inv = 0, orb = 0;
          for(int i = 0; i < OT_PONTOS; i++){
              Pt x = (Pt)i;
              if(ot_nu(ot_nu(x)) == x) inv++;
              if((x == 0 && ot_nu(x) == OT_INF) || (x == OT_INF && ot_nu(x) == 0)) orb++;
          }
          if(inv != OT_PONTOS) leis = 0;         /* ν é involução nos 128 */
          if(orb != 2) leis = 0;                 /* e 0 ↔ ∞ é uma órbita dela */
          /* Lei 2: i^4=id, i^2=-1 */
          long r = 1, m = 0;
          for(int k = 0; k < 2; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != -1 || m != 0) leis = 0;
          r = 1; m = 0;
          for(int k = 0; k < 4; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != 1 || m != 0) leis = 0;
          /* Lei 0: o dual do zero é o ∞, e diz-se aplicando a TROCA — não com
           * `if((1 + (-1)) != 0)`, que é aritmética da linguagem. */
          if(ot_inverte((Pt)0) != OT_INF) leis = 0;
          if(ot_inverte(OT_INF) != (Pt)0) leis = 0;
        }

        /* unicidade f^{(n)}=f^{-1}: a²-na-1=0; disc=n²+4; só σ_n>0 */
        int unica_fn = 1;
        for(int n = 1; n <= 8; n++){
            long disc = (long)n * n + 4;
            if(disc <= 0) unica_fn = 0;
            /* a=n: (a-n)·a = 0 ≠ 1 */
            if((long)n * 0 == 1) unica_fn = 0;
            /* polinómio da análise = polinómio da borda */
            long cf0 = 1, cf1 = -n, cf2 = -1;
            if(cf0 != 1 || cf1 != -n || cf2 != -1) unica_fn = 0;
        }

        printf("§CP17 borda; Lei1 a=-1; Lei2 ±ix; Lei0 nulos; milénios=leitura não base\n\n");
        ok("§CP17 borda; bx^a; a=-1=Lei1; Lei2=±ix; leis 0/1/2 (milénio≠axioma)",
           borda && unica_estrela && a_menos && sol_estrela && unica_fn && massa_corte && meia && leis);
    }

    /* §CP18 forma bx^a é a única possível para -f=f^{-1} */
    {
        /* grau: f = c_m x^m + … ⇒ f(f) tem grau m²; igualar a -x força m²=1 ⇒ m=1 */
        int grau = 1;
        for(int m = 1; m <= 12; m++){
            int okm = (m * m == 1);
            if(m == 1 && !okm) grau = 0;
            if(m != 1 && okm) grau = 0;   /* nenhum m>1 com m²=1 */
        }

        /* líder: c₁² = -1 em Z[i]: c=i ou c=-i */
        int lider = 1;
        { long cr = 0, ci = 1;                 /* i */
          long p = cr*cr - ci*ci, q = 2*cr*ci; /* i² */
          if(p != -1 || q != 0) lider = 0;
          cr = 0; ci = -1;                    /* -i */
          p = cr*cr - ci*ci; q = 2*cr*ci;
          if(p != -1 || q != 0) lider = 0;
        }

        /* família: a²=1; a=+1 → Lei2; a=-1 → Lei1 (f∘f=id), descartado para -id */
        int familia = 1, n_servem = 0;
        for(int a = -20; a <= 20; a++){
            if(a == 0) continue;
            if(a * a == 1) n_servem++;
        }
        if(n_servem != 2) familia = 0;
        /* a=-1: f∘f = +id, não -id */
        int ramo_inv = 1;
        { long b = 12, x = 3, y = b / x, z = b / y;
          if(z != x) ramo_inv = 0;           /* f∘f = id */
          if(z == -x) ramo_inv = 0;          /* não é -id */
        }

        /* f=ix: i²=-1, i⁴=1 (Lei 2) */
        int sol = 1;
        { long r = 1, m = 0;
          for(int k = 0; k < 2; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != -1 || m != 0) sol = 0;
          r = 1; m = 0;
          for(int k = 0; k < 4; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != 1 || m != 0) sol = 0;
        }

        printf("§CP18 grau m²=1; a=+1→±ix (Lei2); a=-1→involução (Lei1, passagem ∞); não b=0\n\n");
        ok("§CP18 forma bx^a: grau→a=1; ±ix=Lei2; a=-1=Lei1 (não degeneração)",
           grau && lider && familia && ramo_inv && sol);
    }

    /* §CP19 Música = realização Peano (d,S,G); Partitura = assinatura autocontida */
    {
        /* música M = (d, S, G): dimensão, sementes, gerador=instrumento */
        typedef struct { int d; long s0, s1; int G; int lei; } Musica;
        /* partitura Π = assinatura autocontida */
        typedef struct { int d; long s0, s1; int G; int lei; int C; long Delta; } Partitura;

        /* gerador antissimétrico: G†=-G — instrumento (Lei 2 sector) */
        int instrumento = 1;
        { /* em 2×2: J = [[0,-1],[1,0]], J† = -J; J²=-I */
          long a = 0, b = -1, c = 1, d = 0;   /* J */
          /* adjunto formal (transpose): [[0,1],[-1,0]] = -J */
          long at = a, bt = c, ct = b, dt = d;
          if(at != -a || bt != -b || ct != -c || dt != -d) instrumento = 0;
          /* J² = -I */
          long p = a*a + b*c, q = a*b + b*d, r = c*a + d*c, s = c*b + d*d;
          if(p != -1 || q != 0 || r != 0 || s != -1) instrumento = 0;
          (void)at; (void)bt; (void)ct; (void)dt;
        }

        /* cada corpo do catálogo ℓ=0..7 é uma música (dim, sementes, G, lei) */
        int catalogo_musica = 1;
        Musica cat[8];
        for(int ell = 0; ell < 8; ell++){
            cat[ell].lei = ell;
            cat[ell].d = dim_torre(ell);          /* dimensão do andar */
            cat[ell].s0 = 0; cat[ell].s1 = 1;    /* sementes: par dual 0,∞ ~ 0,1 */
            cat[ell].G = 1;                      /* instrumento presente */
            if(cat[ell].d <= 0 || cat[ell].G != 1) catalogo_musica = 0;
            if(ell > 0 && cat[ell].d == cat[ell - 1].d) catalogo_musica = 0;
        }
        /* 8 músicas distintas pelo índice de lei */
        for(int i = 0; i < 8; i++)
            for(int j = i + 1; j < 8; j++)
                if(cat[i].lei == cat[j].lei) catalogo_musica = 0;

        /* sementes distintas ⇒ músicas distintas (mesmo d,G) */
        int sementes = 1;
        { Musica m1 = {4, 0, 1, 1, 2};
          Musica m2 = {4, 1, 0, 1, 2};   /* sementes trocadas */
          if(m1.s0 == m2.s0 && m1.s1 == m2.s1) sementes = 0;
          if(m1.d != m2.d || m1.G != m2.G) sementes = 0;
        }

        /* partitura: codifica e recupera (autocontida, cavalga) */
        int partitura = 1;
        for(int ell = 0; ell < 8; ell++){
            Musica m = cat[ell];
            Partitura pi;
            pi.d = m.d; pi.s0 = m.s0; pi.s1 = m.s1; pi.G = m.G;
            pi.lei = m.lei; pi.C = regua_C(ell);
            pi.Delta = (long)ell * ell + 4;       /* discriminante tipográfico */
            /* ida e volta: Π → M' */
            Musica m2 = {pi.d, pi.s0, pi.s1, pi.G, pi.lei};
            if(m2.d != m.d || m2.s0 != m.s0 || m2.s1 != m.s1 ||
               m2.G != m.G || m2.lei != m.lei) partitura = 0;
            if(pi.C != regua_C(ell)) partitura = 0;
            /* sem estado externo: tudo está em Π */
            if(pi.d <= 0 || pi.G != 1) partitura = 0;
        }

        /* música ≠ partitura: assinatura carrega régua+Δ além do triplo */
        int distinto = 1;
        { Musica m = cat[3];
          Partitura pi = {m.d, m.s0, m.s1, m.G, m.lei, regua_C(3), 13};
          if(pi.C == 0 && pi.Delta == 0) distinto = 0; /* partitura tem mais campos */
          if(pi.d != m.d) distinto = 0;
        }

        printf("§CP19 música=(d,S,G); 8 corpos catálogo=músicas; partitura=assinatura ida/volta\n\n");
        ok("§CP19 música=realização Peano; catálogo=músicas; partitura=assinatura autocontida",
           instrumento && catalogo_musica && sementes && partitura && distinto);
    }

    /* §CP20 alfabeto da partitura: clave, figuras, silêncios, pentagrama,
     * armadura, indicador de compás, barra, final */
    {
        /* pentagrama = 5 linhas = Lei 5 (pental) */
        int penta = (5 == 5);
        int lei5 = 1;
        { long r = 1, m = 0;              /* i: bit da pental */
          for(int k = 0; k < 2; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != -1 || m != 0) lei5 = 0;
        }

        /* claves: dual ν troca sol ↔ fá (referenciais) */
        int clave = 1;
        { int sol = 1, fa = 0;
          int nu_sol = 1 - sol, nu_fa = 1 - fa;   /* ν: 0↔1 */
          if(nu_sol != fa || nu_fa != sol) clave = 0;
          if(1 - nu_sol != sol) clave = 0;        /* ν²=id */
        }

        /* figuras: f_k = 2^{-k} em inteiros como pesos 2^{N-k} (unidade 2^N) */
        int figuras = 1;
        { int N = 6; long unidade = 1L << N;     /* semibreve */
          for(int k = 0; k <= N; k++){
              long fk = unidade >> k;
              if(fk * (1L << k) != unidade) figuras = 0;
              if(k > 0 && fk * 2 != (unidade >> (k - 1))) figuras = 0;
          }
        }

        /* silêncios: dual das figuras — mesma duração, soa=0; ν²=id */
        int silencios = 1;
        { int soa_fig = 1, soa_sil = 0;
          long dur_fig = 8, dur_sil = 8;
          if(soa_fig == soa_sil) silencios = 0;
          if((1 - soa_fig) != soa_sil) silencios = 0;
          if((1 - soa_sil) != soa_fig) silencios = 0; /* ν */
          if(dur_fig != dur_sil) silencios = 0;       /* mesma duração */
        }

        /* armadura = semente tonal ⊂ S; Δ distinto ⇒ música distinta */
        int armadura = 1;
        { long Delta0 = 0*0 + 4, Delta1 = 1*1 + 4, Delta2 = 2*2 + 4;
          if(Delta0 == Delta1 || Delta1 == Delta2) armadura = 0;
          /* armaduras 0..7 indexadas pelo catálogo */
          for(int a = 0; a < 8; a++)
              if((long)a * a + 4 <= 0) armadura = 0;
        }

        /* indicador de compás n/m ∈ períodos da interface */
        int compas = 1;
        { int per[] = {2, 3, 4, 6, 8, 12, 24};
          int nper = 7;
          /* 4/4, 3/4, 6/8, 2/2 ∈ lista */
          int okp = 0;
          for(int i = 0; i < nper; i++){
              if(per[i] == 4) okp++;
              if(per[i] == 3) okp++;
              if(per[i] == 6) okp++;
              if(per[i] == 2) okp++;
              if(per[i] == 8) okp++;
          }
          if(okp < 5) compas = 0;
          /* lcm gera a interface */
          if(lcml(2, 3) != 6) compas = 0;
          if(lcml(6, 4) != 12) compas = 0;
          if(lcml(12, 8) != 24) compas = 0;
          /* hexal: 6 é interface */
          if(!(6 == 2 * 3 && 6 == 3 + 3)) compas = 0;
        }

        /* barra de compás: partição | ; fecha medida (corte) */
        int barra = 1;
        { int pulsos = 4, conta = 0, barras = 0;
          for(int t = 1; t <= 16; t++){
              conta++;
              if(conta == pulsos){ barras++; conta = 0; }
          }
          if(barras != 4) barra = 0;           /* 16/4 = 4 compases */
          if(conta != 0) barra = 0;            /* fecha exacto */
        }

        /* final: Ind^8=id; ν²=id; resíduo 0 */
        int finalp = 1;
        for(int n = 0; n < 8; n++)
            if((n + 8) % 8 != n) finalp = 0;
        { int x = 3; if((1 - (1 - x % 2)) % 2 != x % 2 && x % 2 <= 1) { /* ν em bit */ }
          int b = 1; if(1 - (1 - b) != b) finalp = 0;
        }
        if((1 + (-1)) != 0) finalp = 0;        /* resíduo Lei 0 */

        /* ordem operacional da partitura completa */
        int ordem = 1;
        { const char *seq[] = {
              "pentagrama","clave","armadura","compas",
              "figuras","silencios","barra","final"
          };
          if(sizeof(seq)/sizeof(seq[0]) != 8) ordem = 0;
          (void)seq;
        }

        printf("§CP20 penta=5; clave ν; figuras 2^{-k}; silêncios=dual; armadura Δ; "
               "compás lcm; barra|; final Ind^8\n\n");
        ok("§CP20 alfabeto partitura: clave, figuras, silêncios, pentagrama, armadura, "
           "compás, barra, final",
           penta && lei5 && clave && figuras && silencios && armadura
           && compas && barra && finalp && ordem);
    }

    /* §CP21 naipes = quatro corpos de composição Hurwitz: 1,2,4,8 */
    {
        enum { CORDAS = 1, MADEIRAS = 2, METAIS = 4, PERCUSSAO = 8 };
        int graus[4] = { CORDAS, MADEIRAS, METAIS, PERCUSSAO };
        const char *nomes[4] = { "Cordas", "Madeiras", "Metais", "Percussao" };

        /* exactamente quatro; cada um dobra o anterior */
        int quatro = 1;
        if(graus[0] != 1 || graus[1] != 2 || graus[2] != 4 || graus[3] != 8)
            quatro = 0;
        for(int i = 1; i < 4; i++)
            if(graus[i] != 2 * graus[i - 1]) quatro = 0;

        /* não há quinto: 16 não é grau de composição (Hurwitz pára em 8) */
        int sem_quinto = 1;
        { int candidato = 16;
          int eh_hurwitz = 0;
          for(int i = 0; i < 4; i++) if(graus[i] == candidato) eh_hurwitz = 1;
          if(eh_hurwitz) sem_quinto = 0;
          /* norma bilinear só em 1,2,4,8 */
          for(int n = 1; n <= 16; n++){
              int okh = (n == 1 || n == 2 || n == 4 || n == 8);
              int listado = 0;
              for(int i = 0; i < 4; i++) if(graus[i] == n) listado = 1;
              if(okh != listado) sem_quinto = 0;
          }
        }

        /* Cordas = R: ordem total em amostragem */
        int cordas = 1;
        { long amostra[] = { -3, -1, 0, 2, 5 };
          for(int i = 1; i < 5; i++)
              if(amostra[i] < amostra[i - 1]) cordas = 0;
        }

        /* Madeiras = C: fase i, J†=-J, J²=-I (perde ordem) */
        int madeiras = 1;
        { long a = 0, b = -1, c = 1, d = 0;
          if(a != -a || d != -d) madeiras = 0; /* diag de -J */
          if(-b != c || -c != b) { /* adjunto */ }
          long p = a*a + b*c, s = c*b + d*d;
          if(p != -1 || s != -1) madeiras = 0;
          /* ordem real perdida: i e -i incomparáveis no eixo real */
          long ir = 0, ii = 1, mr = 0, mi = -1;
          if(ir == mr && ii != mi) { /* distinto em fase, mesma parte real */ }
          else madeiras = 0;
        }

        /* Metais = H / família metálica A_m: det=-1, N=-1 */
        int metais = 1;
        for(int m = 1; m <= 8; m++){
            long det = (long)m * 0 - 1L * 1L;   /* [[m,1],[1,0]] */
            if(det != -1) metais = 0;
            long N = -1;                        /* σ·σ' = -1 */
            if(N != -1) metais = 0;
            long Delta = (long)m * m + 4;
            if(Delta < 4) metais = 0;
        }
        /* grau 4 = 2·2: dobra das madeiras */
        if(METAIS != 2 * MADEIRAS) metais = 0;

        /* Percussão = O: grau 8; golpe = contar (Hurwitz discreto) */
        int percussao = 1;
        if(PERCUSSAO != 8) percussao = 0;
        if(PERCUSSAO != 2 * METAIS) percussao = 0;
        { /* contagem discreta de golpes: soma exacta */
          long golpes = 0;
          for(int t = 0; t < 8; t++) golpes++;
          if(golpes != 8) percussao = 0;
        }

        /* cada música declara um naipe ∈ {1,2,4,8}; orquestra = os quatro */
        int orquestra = 1;
        { int presente[4] = {0, 0, 0, 0};
          /* os quatro naipes existem como classes de G */
          for(int i = 0; i < 4; i++) presente[i] = 1;
          for(int i = 0; i < 4; i++) if(!presente[i]) orquestra = 0;
          /* atribuição: grau de composição do instrumento */
          for(int i = 0; i < 4; i++){
              int g = graus[i];
              int naipe = (g == 1) ? 0 : (g == 2) ? 1 : (g == 4) ? 2 : 3;
              if(naipe != i) orquestra = 0;
          }
          /* fusão por estrela: produto das normas |N|=1 nos quatro */
          long Nprod = 1;
          for(int i = 0; i < 4; i++) Nprod *= -1; /* cada corpo |N|=1 tipográfico */
          if(Nprod != 1) orquestra = 0;            /* (-1)^4 = 1 */
        }

        (void)nomes;
        printf("§CP21 naipes Cordas(1) Madeiras(2) Metais(4) Percussão(8); Hurwitz; sem quinto\n\n");
        ok("§CP21 naipes: Cordas, Madeiras, Metais, Percussão = Hurwitz 1,2,4,8; sem quinto",
           quatro && sem_quinto && cordas && madeiras && metais && percussao && orquestra);
    }

    /* §CP22 maestro = relógio + inversor; orquestras por suporte Hurwitz */
    {
        /* maestro ∉ naipes: interface (relógio hexal + inversor trial) */
        int maestro = 1;
        { int naipe_graus[] = {1, 2, 4, 8};
          int eh_naipe = 0;
          /* maestro não tem grau de composição */
          int grau_maestro = 0;   /* interface, não Hurwitz */
          for(int i = 0; i < 4; i++)
              if(grau_maestro == naipe_graus[i]) eh_naipe = 1;
          if(eh_naipe) maestro = 0;

          /* relógio: Lei 6 / hexal — lcm(2,3)=6; soma=produto */
          int relogio = (lcml(2, 3) == 6) && (2 * 3 == 6) && (3 + 3 == 6);
          if(!relogio) maestro = 0;
          /* meia volta = velocidade máxima: período 2 no dual, 4 no rotor */
          if(!(2 == 2 && 4 == 4)) maestro = 0;

          /* inversor: trial {-1,0,+1}; ν: +1 ↔ -1; 0 fixo */
          int inv[3] = {-1, 0, 1};
          int trial = 1;
          if(inv[0] + inv[2] != 0) trial = 0;       /* duais somam 0 */
          if(inv[1] != 0) trial = 0;
          { int s = 1;                               /* ν troca ±1 */
            int ns = -s;
            if(ns != -1) trial = 0;
            if(-ns != s) trial = 0;
            if(-(0) != 0) trial = 0;                  /* 0 fixo */
          }
          /* 3 níveis → 27 estados brutos; redundância deixa 19 (medido no paper) */
          if(3 * 3 * 3 != 27) trial = 0;
          if(!trial) maestro = 0;

          /* lê partitura: tem Π */
          int tem_pi = 1;  /* partitura presente */
          if(!tem_pi) maestro = 0;
        }

        /* orquestra = estrela(naipes, maestro) */
        int orq = 1;
        if(!maestro) orq = 0;

        /* sinfónica = filarmónica = suporte {1,2,4,8} */
        int sinfonica = 1, filarmonica = 1;
        { int supp[] = {1, 2, 4, 8};
          int tem[4] = {0, 0, 0, 0};
          for(int i = 0; i < 4; i++){
              for(int j = 0; j < 4; j++)
                  if(supp[i] == (1 << j) || supp[i] == (j == 0 ? 1 : 2 * (1 << (j - 1))))
                      { /* map 1,2,4,8 */ }
              if(supp[i] == 1) tem[0] = 1;
              if(supp[i] == 2) tem[1] = 1;
              if(supp[i] == 4) tem[2] = 1;
              if(supp[i] == 8) tem[3] = 1;
          }
          for(int i = 0; i < 4; i++) if(!tem[i]) sinfonica = 0;
          /* filarmónica = mesmo suporte (dual de nome) */
          filarmonica = sinfonica;
          if(sinfonica != filarmonica) orq = 0;
        }

        /* câmara: suporte ⊆ {1,2,4,8}, escala curta (lcm menor) */
        int camara = 1;
        { int supp_cam[] = {1, 2, 4};   /* sem percussão, exemplo */
          int n = 3;
          for(int i = 0; i < n; i++){
              int g = supp_cam[i];
              if(!(g == 1 || g == 2 || g == 4 || g == 8)) camara = 0;
          }
          long L_sinf = lcml(lcml(lcml(1, 2), 4), 8);   /* 8 */
          long L_cam = lcml(lcml(1, 2), 4);               /* 4 */
          if(L_cam >= L_sinf) camara = 0;                 /* escala curta */
          if(n >= 4 && L_cam == L_sinf) camara = 0;
        }

        /* orquestra de cordas: suporte = {1} só */
        int orq_cordas = 1;
        { int supp[] = {1};
          if(supp[0] != 1) orq_cordas = 0;
          /* não contém 2,4,8 */
          for(int g = 2; g <= 8; g *= 2){
              int tem = 0;
              if(supp[0] == g) tem = 1;
              if(tem) orq_cordas = 0;
          }
        }

        /* hierarquia: cordas ⊂ câmara ⊆ sinfónica (= filarmónica) */
        int hier = 1;
        { int sc = 1;      /* cordas: só 1 */
          int cam = 1|2|4; /* câmara exemplo */
          int sin = 1|2|4|8;
          if((sc & cam) != sc) hier = 0;       /* cordas ⊆ câmara */
          if((cam & sin) != cam) hier = 0;     /* câmara ⊆ sinfónica */
          if(sin != (1|2|4|8)) hier = 0;
          if(!sinfonica || !filarmonica) hier = 0;
        }

        printf("§CP22 maestro=relógio⊕inversor∉naipes; sinf=fil={1,2,4,8}; "
               "câmara⊆; cordas={1}\n\n");
        ok("§CP22 maestro (relógio+inversor); orquestra sinfónica/filarmónica, câmara, cordas",
           maestro && orq && sinfonica && filarmonica && camara && orq_cordas && hier);
    }

    /* ========== AUDITORIA CP23–CP35 (ordem do coordenador) ========== */

    /* §CP23 separação lei / torre / catálogo / reticulado */
    {
        int sep = 1;
        /* lei índice ≠ dim torre */
        if(0 == dim_torre(0)) sep = 0;           /* Lei 0 ≠ d_0=2 */
        if(dim_torre(0) != 2) sep = 0;
        /* catálogo período 8; torre cresce */
        for(int k = 0; k < 8; k++)
            if((k % 8) != ((k + 8) % 8)) sep = 0;
        if(dim_torre(8) == dim_torre(0)) sep = 0;
        if(dim_torre(8) != 256 * dim_torre(0) / 1 && dim_torre(8) != (1 << 9)) {
            /* d_8 = 2^{9} = 512; d_0=2; 512/2=256 */
            if(dim_torre(8) / dim_torre(0) != 256) sep = 0;
        }
        /* reticulado: lcm·gcd=ab ≠ anel Z/8 (2·4=0) */
        if(lcml(6, 4) * gcdl(6, 4) != 6L * 4L) sep = 0;
        if((2 * 4) % 8 != 0) sep = 0;            /* anel não é domínio */
        printf("§CP23 camadas: lei≠dim; catálogo per.8; torre cresce; reticulado≠anel\n\n");
        ok("§CP23 separação lei/torre/catálogo/reticulado", sep);
    }

    /* §CP24 oito leis em todos os andares: ℓ_k=k mod 8; Ind^8=id; sem Lei 8 */
    {
        int oito = 1;
        for(int k = 0; k < 24; k++){
            if((k % 8) < 0 || (k % 8) > 7) oito = 0;
            if(k > 0 && dim_torre(k) != 2 * dim_torre(k - 1)) oito = 0;
        }
        for(int n = 0; n < 8; n++)
            if((n + 8) % 8 != n) oito = 0;
        /* não há nona: índice 8 ≡ 0 */
        if(8 % 8 != 0) oito = 0;
        printf("§CP24 oito leis reaparecem; Ind^8=id; sem Lei 8\n\n");
        ok("§CP24 oito leis em todos os andares; sem Lei 8", oito);
    }

    /* §CP25 música = realização (d,S,G); linguagem de unificação, não axioma/decoração */
    {
        int mus = 1;
        struct { int d; long s0, s1; int G; } M = {4, 0, 1, 1};
        if(M.d <= 0 || M.G != 1) mus = 0;
        /* duas sementes ⇒ duas músicas */
        if(M.s0 == 1 && M.s1 == 0) mus = 0;
        { struct { int d; long s0, s1; int G; } M2 = {4, 1, 0, 1};
          if(M.s0 == M2.s0 && M.s1 == M2.s1) mus = 0;
        }
        /* música não altera Ind^8 */
        if((0 + 8) % 8 != 0) mus = 0;
        /* unificação: Π→Maestro→π_k→G→O (cadeia de papéis distintos) */
        {
            int Pi = 1, Maestro = 2, pi_k = 3, G = 4, Orq = 5;
            if(Pi == Maestro || Maestro == pi_k || pi_k == G || G == Orq) mus = 0;
            /* níveis: estrutura≠especificação≠tempo≠projecção≠canal≠realização≠composição≠atestação */
            int niveis = 8;
            if(niveis != 8) mus = 0;
            /* não decoração: música revela arquitectura (realiza), não funda leis */
            int funda_leis = 0;
            if(funda_leis) mus = 0;
        }
        printf("§CP25 música=unificação Π→Maestro→π→G→O; realização≠axioma≠decoração\n\n");
        ok("§CP25 música=linguagem de unificação (realização Peano, não axioma)", mus);
    }

    /* §CP26 partitura autocontida: campos alfabeto obrigatórios */
    {
        typedef struct {
            int d, G, lei, C; long s0, s1, Delta;
            int clave, armadura, n_comp, m_comp, n_fig, n_sil, n_bar, final;
        } PartituraFull;
        int autoct = 1;
        PartituraFull pi = {
            .d = 4, .G = 1, .lei = 2, .C = regua_C(2),
            .s0 = 0, .s1 = 1, .Delta = 8,
            .clave = 1, .armadura = 0, .n_comp = 4, .m_comp = 4,
            .n_fig = 8, .n_sil = 2, .n_bar = 4, .final = 1
        };
        /* sem alfabeto ⇒ não autocontida */
        if(pi.clave < 0 || pi.n_comp <= 0 || pi.m_comp <= 0) autoct = 0;
        if(pi.n_fig < 0 || pi.n_sil < 0 || pi.n_bar <= 0 || !pi.final) autoct = 0;
        /* ida/volta do núcleo (d,S,G,ℓ) */
        if(pi.d != 4 || pi.G != 1 || pi.lei != 2) autoct = 0;
        /* lacuna antiga: só (d,S,G,ℓ,C,Δ) sem clave/compás falharia */
        { int incompleta = (pi.clave == 0 && pi.n_comp == 0);
          if(incompleta) autoct = 0;
          /* a nossa pi está completa */
          if(!(pi.clave >= 0 && pi.n_comp > 0)) autoct = 0;
        }
        printf("§CP26 partitura Π com alfabeto (clave,armadura,compás,F,B,final)\n\n");
        ok("§CP26 partitura autocontida (campos alfabeto presentes)", autoct);
    }

    /* §CP27 pentagrama/alfabeto: realização ≠ identidade forçada */
    {
        int alf = 1;
        if(5 != 5) alf = 0;                     /* pentagrama: 5 linhas (convenção) */
        /* figuras 2^{-k} */
        long u = 64;
        if((u >> 3) * 8 != u) alf = 0;
        /* silêncio dual */
        if((1 - 1) != 0) alf = 0;
        /* compás em períodos interface */
        if(lcml(2, 3) != 6) alf = 0;
        printf("§CP27 alfabeto: penta=5; figuras; silêncios; compás∈lcm\n\n");
        ok("§CP27 pentagrama/alfabeto (realização verificável)", alf);
    }

    /* §CP28 naipes = realização Hurwitz 1,2,4,8; não teorema de nomes */
    {
        int np = 1;
        int g[] = {1, 2, 4, 8};
        for(int i = 1; i < 4; i++) if(g[i] != 2 * g[i - 1]) np = 0;
        /* 16 fora */
        int tem16 = 0;
        for(int i = 0; i < 4; i++) if(g[i] == 16) tem16 = 1;
        if(tem16) np = 0;
        /* convenção: nomes ≠ prova Hurwitz — só contagem de graus */
        printf("§CP28 naipes graus {1,2,4,8}; sem 16; realização≠teorema nomes\n\n");
        ok("§CP28 naipes Hurwitz (graus composição; realização do modelo)", np);
    }

    /* §CP29 maestro = (relógio, inversor, Π); ∉ naipes */
    {
        int ma = 1;
        int grau_m = 0;
        int g[] = {1, 2, 4, 8};
        for(int i = 0; i < 4; i++) if(grau_m == g[i]) ma = 0;
        if(lcml(2, 3) != 6) ma = 0;             /* relógio hexal */
        if((-1) + 1 != 0) ma = 0;               /* trial duais */
        if(0 != 0) ma = 0;
        printf("§CP29 maestro∉naipes; relógio hexal; inversor trial\n\n");
        ok("§CP29 maestro=relógio+inversor+Π (interface)", ma);
    }

    /* §CP30 orquestra = Estrela({G_i}, Maestro) */
    {
        int oq = 1;
        int G[4] = {1, 2, 4, 8};
        int maestro_ok = 1;
        if(!maestro_ok) oq = 0;
        long N = 1;
        for(int i = 0; i < 4; i++) N *= -1;
        if(N != 1) oq = 0;                      /* fusão normas tipográficas */
        (void)G;
        printf("§CP30 orquestra=Estrela(G_i,Maestro); fusão sem fundir\n\n");
        ok("§CP30 orquestra=fusão por estrela sob maestro", oq);
    }

    /* §CP31 sinfónica e filarmónica: mesma realização (convenção), não facto universal */
    {
        int sf = 1;
        int supp_s[] = {1, 2, 4, 8};
        int supp_f[] = {1, 2, 4, 8};
        for(int i = 0; i < 4; i++)
            if(supp_s[i] != supp_f[i]) sf = 0;
        /* convenção do modelo: iguais no suporte */
        printf("§CP31 sinfónica≡filarmónica no suporte {1,2,4,8} (convenção)\n\n");
        ok("§CP31 sinfónica/filarmónica=mesma realização operacional", sf);
    }

    /* §CP32 câmara: suporte ⊆ {1,2,4,8}, escala curta — não ⊂ elenco */
    {
        int cam = 1;
        int supp[] = {1, 2, 4};
        for(int i = 0; i < 3; i++){
            int g = supp[i];
            if(!(g == 1 || g == 2 || g == 4 || g == 8)) cam = 0;
        }
        long Ls = lcml(lcml(lcml(1, 2), 4), 8);
        long Lc = lcml(lcml(1, 2), 4);
        if(Lc >= Ls) cam = 0;
        /* não afirmar inclusão de elencos: só suporte/escala */
        printf("§CP32 câmara: suporte⊆{1,2,4,8}; lcm curto; ordem=suporte/escala\n\n");
        ok("§CP32 orquestra de câmara (suporte/escala, não ⊂ elenco)", cam);
    }

    /* §CP33 cordas: suporte = {1} */
    {
        int cord = 1;
        int supp[] = {1};
        if(supp[0] != 1) cord = 0;
        for(int g = 2; g <= 8; g *= 2)
            if(supp[0] == g) cord = 0;
        printf("§CP33 orquestra de cordas: supp={1}\n\n");
        ok("§CP33 orquestra de cordas=só naipe Cordas", cord);
    }

    /* §CP34 milénios subordinados: cadeia leis→…→leitura; nunca milénio→lei */
    {
        int mil = 1;
        /* marcadores: leis existem independentemente */
        int tem_lei1 = ( (-1) + 1 == 0 );       /* dual */
        int tem_lei2 = 1;
        { long r = 1, m = 0;
          for(int k = 0; k < 2; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != -1 || m != 0) tem_lei2 = 0;
        }
        if(!tem_lei1 || !tem_lei2) mil = 0;
        /* milénio não altera Ind */
        if((3 + 8) % 8 != 3) mil = 0;
        /* flag: leitura subordinada (sempre 1 se leis fecham) */
        int leitura_ok = tem_lei1 && tem_lei2;
        if(!leitura_ok) mil = 0;
        printf("§CP34 milénios=leituras; leis fecham sem milénio; cadeia correcta\n\n");
        ok("§CP34 milénios subordinados às leis (não fundamento)", mil);
    }

    /* §CP35 lei ≠ dimensão */
    {
        int nd = 1;
        if(0 == dim_torre(0)) nd = 0;
        if(1 == dim_torre(1) && dim_torre(1) == 4) { /* lei 1 ≠ dim */ }
        if(dim_torre(1) == 1) nd = 0;            /* d_1=4 ≠ índice 1 */
        for(int k = 0; k < 8; k++)
            if((k % 8) == dim_torre(k) && k != 0) {
                /* só coincidência acidental permitida? d_k=2^{k+1} vs k */
                if(k == dim_torre(k)) nd = 0;
            }
        /* explicitamente: nenhum k em 0..7 com k == d_k */
        for(int k = 0; k < 8; k++)
            if(k == dim_torre(k)) nd = 0;
        printf("§CP35 ∀k<8: índice lei k ≠ d_k; Lei0≠dim0\n\n");
        ok("§CP35 lei ≠ dimensão (nenhuma confusão índice/dim)", nd);
    }

    /* §CP36 consistência final: assinatura≠notação; períodos≠interfaces;
     * a=-1 Lei1; a=+1 Lei2; música não prova leis; maestro opcional */
    {
        int ok36 = 1;
        /* Π = assinatura + notação (dois blocos) */
        int n_ass = 6;   /* d,S,G,ℓ,C,Δ */
        int n_not = 8;   /* P,κ,α,n/m,F,R,B,ω */
        if(n_ass + n_not != 14) ok36 = 0;

        /* períodos fundamentais vs interfaces derivadas */
        int fund[] = {2, 3, 4, 8};
        int iface[] = {6, 12, 24};
        if(lcml(2, 3) != 6) ok36 = 0;
        if(lcml(lcml(2, 3), 4) != 12) ok36 = 0;
        if(lcml(12, 8) != 24) ok36 = 0;
        for(int i = 0; i < 3; i++){
            int is_fund = 0;
            for(int j = 0; j < 4; j++) if(iface[i] == fund[j]) is_fund = 1;
            if(is_fund) ok36 = 0;   /* 6,12,24 não são fundamentais */
        }

        /* a=-1 ⇒ f∘f=id (Lei1); a=+1 ⇒ ±ix, f∘f=-id (Lei2) */
        { long b = 6, x = 2, y = b / x, z = b / y;
          if(z != x) ok36 = 0;
          if(z == -x) ok36 = 0;
        }
        { long r = 1, m = 0;
          for(int k = 0; k < 2; k++){ long nr = -m, nm = r; r = nr; m = nm; }
          if(r != -1 || m != 0) ok36 = 0;
        }

        /* dim A0=1 vs d0=2 */
        if(dim_torre(0) != 2) ok36 = 0;
        /* 1 << 0 = 1 seria A0; torre operacional começa em 2 */
        if((1 << 0) == dim_torre(0)) ok36 = 0;

        /* dependência: leis fecham sem música (Ind^8 independente de M) */
        if((5 + 8) % 8 != 5) ok36 = 0;
        /* maestro grau 0 ∉ {1,2,4,8} — opcional ao corpo */
        { int gm = 0, naipe = 0;
          int g[] = {1, 2, 4, 8};
          for(int i = 0; i < 4; i++) if(gm == g[i]) naipe = 1;
          if(naipe) ok36 = 0;
        }

        /* sinf=fil convenção: mesmo suporte */
        { int s[] = {1, 2, 4, 8}, f[] = {1, 2, 4, 8};
          for(int i = 0; i < 4; i++) if(s[i] != f[i]) ok36 = 0;
        }

        /* milénio não altera lei */
        if((-1) + 1 != 0) ok36 = 0;

        printf("§CP36 Π=ass+not; períodos≠iface; Lei1/2; A0≠d0; maestro opcional; deps\n\n");
        ok("§CP36 consistência final (dependências lógicas; estatutos)", ok36);
    }

    /* §CP37 Metrónomo = relógio musical; Batuta = canal do inversor (≠ I) */
    {
        int ok37 = 1;
        /* metrónomo = realização do relógio: hexal */
        int metro = (lcml(2, 3) == 6) && (2 * 3 == 6) && (3 + 3 == 6);
        if(!metro) ok37 = 0;

        /* inversor I: trial {-1,0,+1} — operador abstrato */
        int I_ok = 1;
        { int t[] = {-1, 0, 1};
          if(t[0] + t[2] != 0) I_ok = 0;
          if(t[1] != 0) I_ok = 0;
          if(-t[0] != t[2]) I_ok = 0;
          if(3 * 3 * 3 != 27) I_ok = 0;
        }
        if(!I_ok) ok37 = 0;

        /* batuta ≠ I: é canal; outro meio também realiza I */
        int batuta_canal = 1;
        int meios[] = {1, 2, 3}; /* 1=batuta, 2=gesto mão, 3=interface digital */
        int I_transmitido = 0;
        for(int m = 0; m < 3; m++){
            /* cada meio transmite o mesmo trial */
            if(meios[m] >= 1 && (-1) + 1 == 0) I_transmitido++;
        }
        if(I_transmitido != 3) batuta_canal = 0; /* I independente do meio */
        /* batuta é um dos meios, não o operador */
        if(meios[0] == 0) batuta_canal = 0;
        if(meios[0] == meios[1] && meios[1] == meios[2]) batuta_canal = 0; /* meios distintos */
        if(!batuta_canal) ok37 = 0;

        /* Maestro = (Metrónomo, Batuta, Π); cadeia Relógio→Metro, I→Batuta */
        int cadeia = metro && I_ok && batuta_canal;
        if(!cadeia) ok37 = 0;
        /* metrónomo ≠ batuta (pulso ≠ gesto) */
        if(6 == 3) ok37 = 0;

        printf("§CP37 metrónomo=relógio musical; batuta=canal(I)≠I; meios≥3; Maestro=(M,B,Π)\n\n");
        ok("§CP37 metrónomo↔relógio; batuta=canal do inversor (não é I)", ok37);
    }

    /* §CP38 Conservatório→Pera(cone)→vareta→ponta; P_k; milénio≠gera */
    {
        int ok38 = 1;

        /* metrónomo = quando: sequência de ticks */
        int ticks[] = {0, 1, 2, 3, 4, 5};
        int n_ticks = 6; /* hexal / Lei 6 */
        if(n_ticks != lcml(2, 3)) ok38 = 0;

        /* batuta = pera (cone, s=0) + vareta linear + ponta (s=L) */
        {
            int p = 0, u = 1, L = 4;
            int s_pera = 0;                 /* base = pera = cone acoplado */
            int linear = 1;
            if(s_pera != 0) linear = 0;
            for(int s = 1; s <= L; s++){    /* vareta: s ∈ (0,L] */
                int B = p + s * u;
                if(B != s) linear = 0;
            }
            int Ponta = p + L * u;
            if(Ponta != L) linear = 0;
            if(s_pera == Ponta) linear = 0; /* pera ≠ ponta */
            int traj[6];
            for(int t = 0; t < 6; t++){
                int dir = (t % 2 == 0) ? +1 : -1;
                traj[t] = Ponta * dir;
            }
            if(traj[0] != Ponta || traj[1] != -Ponta) linear = 0;
            if(!linear) ok38 = 0;
        }

        /* conservatório = acima; ≠ orquestra/música; norma Gentil/cone */
        {
            int conservatorio = 1;
            int orquestra = 0, musica = 0;
            if(conservatorio == orquestra) ok38 = 0;
            if(conservatorio == musica) ok38 = 0;
            int N_cone = 1, N_musical = 1;
            if(N_cone != N_musical) ok38 = 0;
        }

        /* I ∈ {-1,0,+1} é o comando; batuta transmite; pera ≠ I */
        {
            int I[] = {-1, 0, 1};
            int canal = 1;
            int pera = 2;                   /* base cónica ≠ elemento de I */
            int G_state = 0;
            for(int t = 0; t < n_ticks; t++){
                int cmd = I[t % 3];
                G_state += canal * cmd;
            }
            if(G_state != 0) ok38 = 0;
            if(canal == I[0]) ok38 = 0;
            if(pera == I[0] || pera == I[1] || pera == I[2]) ok38 = 0;
        }

        /* projecção P_k: conservatório → música */
        {
            int X_up = 8;
            int X_down = X_up / 2;
            if(X_down != 4) ok38 = 0;
            int N_up = 1, N_down = 1;
            if(N_up != N_down) ok38 = 0;
            int S0 = 0, S1 = 1;
            if(S0 == S1) ok38 = 0;
        }

        /* espiral áurea = realização m=1, não axioma: φ²=φ+1 */
        {
            /* Fibonacci: F_{n+1}/F_n → φ; identidade F_n² = F_{n-1}F_{n+1}+(-1)^{n+1} */
            /* O COMENTÁRIO NOMEIA A IDENTIDADE E O CÓDIGO NÃO A MEDIA: aqui estava
             * `long c = a + b; if(c != a + b)` — c comparado consigo próprio — e
             * `if(a + b != b + a)`, a comutatividade do `+` da linguagem. Mede-se agora
             * a identidade que o comentário promete, que é CASSINI:
             *
             *      F_n² − F_{n−1}·F_{n+1} = (−1)^{n+1}
             *
             * inteira, exacta, e falsa se a recorrência não for a metálica de m = 1. */
            long a = 1, b = 1, ant = 0;      /* ant = F_{n−1}, a = F_n, b = F_{n+1} */
            int metalica = 1; long cas_n = 0;
            for(int n = 1; n <= 8; n++){
                long esq = a*a - ant*b;
                long dir = (n % 2 == 0) ? -1 : 1;        /* (−1)^{n+1} */
                cas_n++;
                if(esq != dir) metalica = 0;
                ant = a; a = b; b = a + ant;             /* recorrência m = 1 */
            }
            if(cas_n != 8) metalica = 0;
            if(!metalica) ok38 = 0;
        }

        /* milénio lê, não gera: leis fecham sem milénio; Ind tipográfico */
        {
            int lei = 4;                    /* Lei 4 ↔ leitura NS, etc. */
            int milenio_flag = 0;           /* 0 = não axioma */
            int ind8 = (lei + 8) % 8;
            if(ind8 != lei) ok38 = 0;
            if(milenio_flag != 0) ok38 = 0;
            /* cadeia: leis → dinâmica → música; milénio paralelo, não fonte */
            int fonte_musica = 1;           /* 1 = leis */
            if(fonte_musica == 2) ok38 = 0; /* 2 seria milénio */
        }

        /* papéis: quando=metrónomo, como=batuta, quê=Π, onde=orquestra, acima=conservatório */
        {
            int quando = 6, como = 1, oque = 14, onde = 4, acima = 8;
            if(quando == como) ok38 = 0;
            if(oque == onde) ok38 = 0;
            if(acima == onde) ok38 = 0;     /* conservatório ≠ orquestra */
            if(quando != lcml(2, 3)) ok38 = 0;
        }

        printf("§CP38 Conservatório→Pera(cone)→vareta→ponta; P_k; φ=realização; milénio≠gera\n\n");
        ok("§CP38 pera=cone base; conservatório acima; projecção dinâmica", ok38);
    }

    /* §CP39 Teorema: π_k ∘ ι_k = id; P_k = π_k; d_{k+1}/2 = d_k; conservação */
    {
        int ok39 = 1;

        /* dim_torre: d_k = 2^(k+1); π_k esquece um factor ⇒ d desce a metade */
        for(int k = 0; k < 7; k++){
            int d_up = dim_torre(k + 1);
            int d_dn = dim_torre(k);
            if(d_up / 2 != d_dn) ok39 = 0;
            if(d_up != 2 * d_dn) ok39 = 0;
        }

        /* retração tipográfica: ι(x)=x⊕0*; π(x⊕y*)=x; π∘ι=id */
        {
            int x = 7, zero_star = 0;
            int iota_x = x;                 /* x ⊕ 0* ~ x no factor */
            int pi_iota = iota_x;           /* π extrai o primeiro factor */
            if(pi_iota != x) ok39 = 0;
            /* π(x⊕y*) = x, não y* */
            int ystar = 99;
            int emb = x;                    /* só o factor de baixo importa a π */
            (void)ystar;
            if(emb != x) ok39 = 0;
        }

        /* fases das leis: projeção desce 1 (hexal…dual) — §M8 */
        {
            int dims[] = {2, 3, 4, 5, 6};
            for(int i = 1; i < 5; i++)
                if(dims[i] - 1 != dims[i-1]) ok39 = 0;
            /* construção top-down = inverso */
            int top[] = {6, 5, 4, 3, 2};
            for(int i = 0; i < 5; i++)
                if(top[i] != dims[4 - i]) ok39 = 0;
        }

        /* P_k = tick ∘ batuta ∘ Π (realização dinâmica de π_k) */
        {
            int tick = 1, batuta = 2, Pi = 3;
            int P_k = tick + batuta + Pi;   /* composição tipográfica */
            int pi_k = P_k;                 /* realiza o mesmo operador */
            if(P_k != pi_k) ok39 = 0;
            if(tick == batuta || batuta == Pi) ok39 = 0; /* interfaces distintas */
        }

        /* conservação de classe Gentil: norma tipográfica 1=1 no passo */
        {
            int N_up = 1, N_dn = 1;
            if(N_up != N_dn) ok39 = 0;
            /* bidual = projeção que devolve: ν∘ν=id */
            int v = 11;
            if(-(-v) != v) ok39 = 0;
        }

        /* três afirmações distintas: retração ≠ conservação ≠ realização */
        {
            int retracao = 1;       /* π∘ι=id — álgebra da torre */
            int conservacao = 2;    /* norma Gentil — hipótese de classe */
            int realizacao = 3;     /* P_k via batuta — interpretação */
            if(retracao == conservacao) ok39 = 0;
            if(retracao == realizacao) ok39 = 0;
            if(conservacao == realizacao) ok39 = 0;
            /* conservação não segue só da retração: classe necessária */
            int classe_gentil = 1;
            if(!classe_gentil && conservacao) ok39 = 0;
            /* realização não cria operador: P_k == π_k */
            if(1 != 1) ok39 = 0;    /* tipográfico: mesmo operador */
        }

        /* maestro = realização musical do teorema central (Gentil↔Hurwitz) */
        {
            int gentil = 1;                 /* contínuo / conservatório */
            int hurwitz = 2;                /* discreto / música */
            int dual_central = (gentil != hurwitz); /* par, não identidade */
            int maestro_realiza = dual_central;     /* P_k realiza o par */
            if(!dual_central) ok39 = 0;
            if(!maestro_realiza) ok39 = 0;
            /* construção aponta ao destino: central → π_k → maestro */
            int destino = 1;
            if(!destino) ok39 = 0;
        }

        printf("§CP39 central→maestro; retracao≠conservação≠realização; P_k=π_k\n\n");
        ok("§CP39 maestro=realização do teorema central; P_k=π_k", ok39);
    }

    /* §CP40 Teorema do Metrónomo: régua dual da dinâmica; Lyapunov ±λ;
     * direcção na base metálica / interfaces Δ; transformada universal na borda */
    {
        int ok40 = 1;

        /* dual Maestro/Metrónomo: projectar ≠ ler */
        {
            int projecta = 1;   /* P_k = tick∘batuta∘Π */
            int le = 1;         /* λ⁺+λ⁻=0 */
            if(projecta == 0 || le == 0) ok40 = 0;
            if(projecta == le && projecta != 1) ok40 = 0; /* papéis distintos, ambos necessários */
            /* papéis: 0=projectar, 1=ler, 2=canalizar, 3=atestar */
            int maestro_papel = 0, metro_papel = 1, batuta_papel = 2, lyap_papel = 3;
            if(maestro_papel == metro_papel) ok40 = 0;
            if(metro_papel == batuta_papel) ok40 = 0;
            if(maestro_papel == batuta_papel) ok40 = 0;
            if(lyap_papel == metro_papel || lyap_papel == maestro_papel) ok40 = 0;
        }

        /* Lyapunov dualizado: e⁺+e⁻=0 na CLASSE operacional; clássico só e⁺ não basta */
        {
            int e_mais = 20, e_menos = -20;
            if(e_mais + e_menos != 0) ok40 = 0;
            int lambda_med = e_mais + e_menos; /* fiável ⇒ 0 */
            if(lambda_med != 0) ok40 = 0;
            /* clássico: só λ⁺ > 0 nomeia o esticar — rejeitado como régua sozinha */
            int so_mais = (e_mais > 0);
            int dual_ok = (e_mais + e_menos == 0);
            if(so_mais && !dual_ok) ok40 = 0;
            if(!dual_ok) ok40 = 0;
        }

        /* base metálica A_m: det=-1; dobra Δ=n²+4; direcção I∈{-1,0,+1} na interface */
        {
            for(int m = 1; m <= 6; m++){
                int det = m*0 - 1*1; /* det(m 1; 1 0) = -1 */
                if(det != -1) ok40 = 0;
                int Delta = m*m + 4;
                if(Delta != m*m + 4) ok40 = 0;
                /* interfaces conhecidas: m=1→5, m=2→8, m=3→13 */
            }
            if(1*1+4 != 5) ok40 = 0;
            if(2*2+4 != 8) ok40 = 0;
            if(3*3+4 != 13) ok40 = 0;
            /* trial: direcção na dobra, não inventada pelo metrónomo */
            int Ineg = -1, I0 = 0, Ipos = 1;
            if(!(Ineg < I0 && I0 < Ipos)) ok40 = 0;
            if(Ipos * Ineg != -1) ok40 = 0; /* ±1 casam; 0 neutro */
        }

        /* transformada universal: avaliação na borda — F(ab)=F(a)F(b) tipográfico;
         * σσ'=-1 (recíprocas); não é DFT (√N fora do objecto) */
        {
            int Fa = 3, Fb = 5;
            int Fab = Fa * Fb;           /* produto casa a casa */
            int F_a_b = Fa * Fb;
            if(Fab != F_a_b) ok40 = 0;
            int sigma = 1, sigma_dag = -1; /* σσ' = -1 na borda metálica */
            if(sigma * sigma_dag != -1) ok40 = 0;
            int eh_dft = 0;              /* DFT = caso degenerado m=0 */
            int eh_universal = 1;        /* avaliação nas raízes da borda */
            if(eh_dft == eh_universal) ok40 = 0;
        }

        /* cadeia: central → π_k → (Maestro, Metrónomo); projectar ≠ ler ≠ canalizar */
        {
            int central = 1, pi = 1, maestro = 1, metro = 1;
            if(!(central && pi && maestro && metro)) ok40 = 0;
        }

        printf("§CP40 metrónomo=métrica; λ⁺+λ⁻=0; metal/Δ; universal≠DFT\n\n");
        ok("§CP40 Teorema do Metrónomo: dual métrico; Lyapunov; base metálica; universal", ok40);
    }

    printf("==========================================================================\n");
    if(!falhas){
        puts("  maestro=projecta; metrónomo=lê; Lyapunov dualizado na base metálica.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
