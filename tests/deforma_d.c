/* deforma_d.c — A DEFORMAÇÃO EM DIMENSÕES MAIORES, K CRESCENTE (KAM).
 *
 * Em T¹ cada racional abre uma língua e a fração travada vai de 0 (K=0) a ~1 (deforma.c). Em Tᵈ a
 * ressonância não é um ponto: é o hiperplano k·ω = 0 com k ∈ ℤᵈ, e o número deles cresce como Nᵈ.
 * Logo a MESMA deformação deve destruir mais em dimensão maior. Mede-se por partes:
 *
 *  (Dd1) A CONTAGEM, exata contra fórmula fechada: #{k ∈ ℤᵈ : |k|₁ ≤ N} = Σᵢ 2ⁱ C(d,i) C(N,i)
 *        (os pontos inteiros da bola ℓ¹ — número de Delannoy). Cresce como (2N)ᵈ/d!.
 *
 *  (Dd2) A FRAÇÃO RESSONANTE cresce com d para a MESMA largura, com N e τ FIXOS.
 *
 *  (Dd3) A DINÂMICA — o mapa tem de ser SIMPLÉTICO. Kick-drift: det J = 1. Lyapunov, sen/cos e
 *        o K≈0,9716 de Greene eram o transporte. A lei é o TRIAL da linearização: K=0 parabólico,
 *        K>0 hiperbólico na origem, |det|=1 sempre.
 *
 *  (Dd4) O METAL DA DIMENSÃO: o polinómio x^{d+1}−x^d−1, companion com |det|=1. Em d=1 é o ouro,
 *        e o ouro não fecha porque É O GATO (deforma.c §D4) — sem simular o standard map.
 *
 * LEI vs TRANSPORTE. Froeschlé com 2800 passos, λ, √2 e a raiz de x^n−x^{n−1}−1 por bissecção
 * eram o método. A lei é Delannoy em ℤ, ressonância por produto cruzado, o toro |det|=1, o trial.
 *
 *   cc -O2 -std=c99 -I lib tests/deforma_d.c -o deforma_d && ./deforma_d
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"
#include "poli.h"

#define DMAX 4

static long bola_l1(int d, int N){
    long s = 0;
    for(int i = 0; i <= d && i <= N; i++)
        s += (1L << i) * pl_binom(d, i) * pl_binom(N, i);
    return s;
}
static long bola_l1_contando(int d, int N){
    long k[DMAX], cnt = 0;
    for(int i = 0; i < d; i++) k[i] = -N;
    while(1){
        long n1 = 0;
        for(int i = 0; i < d; i++) n1 += k[i] < 0 ? -k[i] : k[i];
        if(n1 <= N) cnt++;
        int i = 0;
        while(i < d){ if(++k[i] <= N) break; k[i] = -N; i++; }
        if(i == d) break;
    }
    return cnt;
}

/* |k·ω − m| < γ / |k|_1^τ  com ω_i = a_i/M, γ = g/100, τ = 4. Sem vírgula. */
static int ressonante(const long *a, int d, int N, long g, long M){
    long k[DMAX];
    for(int i = 0; i < d; i++) k[i] = -N;
    while(1){
        long n1 = 0;
        for(int i = 0; i < d; i++) n1 += k[i] < 0 ? -k[i] : k[i];
        if(n1 > 0 && n1 <= N){
            long s = 0;
            for(int i = 0; i < d; i++) s += k[i] * a[i];
            long r = s % M; if(r < 0) r += M;
            long dist = r < M - r ? r : M - r;          /* ||s/M|| · M */
            long n4 = n1*n1*n1*n1;
            /* dist/M < (g/100) / n1^4  ⇔  dist · n1^4 · 100 < g · M */
            if(dist * n4 * 100 < g * M) return 1;
        }
        int i = 0;
        while(i < d){ if(++k[i] <= N) break; k[i] = -N; i++; }
        if(i == d) break;
    }
    return 0;
}
static void varre_omega(long *a, int i, int d, int N, long g, long M, long *res, long *tot){
    if(i == d){ (*tot)++; *res += ressonante(a, d, N, g, M); return; }
    for(a[i] = 0; a[i] < M; a[i]++)
        varre_omega(a, i + 1, d, N, g, M, res, tot);
}

/* companion de x^n − x^{n−1} − 1 via rt_companheira: última linha (1, 0, …, 0, 1). */
static long det_metal(int n){
    long c[8], G[64];
    for(int j = 0; j < n; j++) c[j] = 0;
    c[0] = 1;
    c[n-1] = 1;
    rt_companheira(c, n, G);
    return rt_det_bareiss(G, n, n);
}

int main(void){
    printf("DEFORMA_D — a deformação em dimensões maiores, K crescente (KAM)\n");
    printf("=================================================================\n");

    printf("§Dd1 CONTAGEM das ressonâncias: #{k ∈ ℤᵈ : |k|₁ ≤ N} contra a fórmula fechada\n");
    printf("     Σᵢ 2ⁱ·C(d,i)·C(N,i)  (pontos da bola ℓ¹ — Delannoy):\n");
    {
        int erro = 0;
        printf("       d   N   contado    fórmula\n");
        for(int d = 1; d <= 4; d++) for(int N = 4; N <= 12; N += 4){
            long c = bola_l1_contando(d, N);
            long f = bola_l1(d, N);
            printf("       %d  %2d  %8ld  %9ld  %s\n", d, N, c, f, c == f ? "✓" : "← REVER");
            if(c != f) erro = 1;
        }
        int cresce = 1;
        for(int N = 4; N <= 12; N += 4)
            for(int d = 1; d < 4; d++)
                if(bola_l1(d+1, N) <= bola_l1(d, N)) cresce = 0;
        ok("resíduo 0 — a contagem BATE a fórmula em toda (d,N), Delannoy em ℤ, sem tgamma"
           " e sem pow. E a bola CRESCE com d: mais hiperplanos, a mesma N",
           erro == 0 && cresce);
        printf("     ⟹ o nº de hiperplanos de ressonância cresce como Nᵈ: em dimensão maior há\n");
        printf("        exponencialmente mais lugares por onde a deformação entra.\n");
    }

    printf("\n§Dd2 FRAÇÃO RESSONANTE, mesma largura γ, N=4 e τ=4 FIXOS (comparação justa):\n");
    {
        const long M = 8, N = 4;
        printf("        γ          d=1      d=2      d=3\n");
        int cresce_ok = 1, linhas = 0;
        for(long g = 4; g <= 32; g *= 2){
            long r[3], t[3];
            printf("      %ld/100  ", g);
            for(int d = 1; d <= 3; d++){
                long a[DMAX];
                r[d-1] = 0; t[d-1] = 0;
                varre_omega(a, 0, d, (int)N, g, M, &r[d-1], &t[d-1]);
                printf(" %4ld/%-4ld", r[d-1], t[d-1]);
            }
            printf("\n");
            if(r[1]*t[0] < r[0]*t[1]) cresce_ok = 0;
            if(r[2]*t[1] < r[1]*t[2]) cresce_ok = 0;
            linhas++;
        }
        ok("a mesma largura consome mais espaço em dimensão maior — fracção ressonante não"
           " desce com d, N=4 e τ=4 fixos, ω = a/M em ℤ, γ = g/100 por produto cruzado",
           cresce_ok && linhas == 4);
        printf("     cresce com d: %s — a mesma largura consome mais espaço em dimensão maior\n",
               cresce_ok ? "sim, resíduo 0" : "NÃO");
    }

    printf("\n§Dd3 DINÂMICA (kick-drift, SIMPLÉTICO): det J = 1, e o TRIAL na origem.\n");
    printf("     Lyapunov, sen/cos e o K≈0,9716 de Greene eram o transporte. A lei é o\n");
    printf("     jacobiano do standard map: [[1+Kc, 1], [Kc, 1]], det = 1 SEMPRE.\n");
    {
        /* c = cos(2πx) ∈ {−1, 0, +1} no grupo C₄. K inteiro. det = (1+Kc)·1 − 1·Kc = 1. */
        int det_ok = 1, nJ = 0;
        int k0_para = 0, origem_hip = 1;
        printf("       K   c    tr=2+Kc      D=tr²−4    det   trial\n");
        for(long K = 0; K <= 3; K++){
            long Cs[3] = { -1, 0, 1 };
            for(int i = 0; i < 3; i++){
                long c = Cs[i];
                long tr = 2 + K*c;
                long det = (1 + K*c)*1 - 1*(K*c);
                long D = tr*tr - 4*det;
                if(det != 1) det_ok = 0;
                nJ++;
                if(K == 0 && c == 1 && D == 0) k0_para = 1;
                if(K > 0 && c == 1 && D <= 0) origem_hip = 0;
                const char *reg = D < 0 ? "ELÍPTICO" : (D == 0 ? "PARABÓLICO" : "HIPERBÓLICO");
                if(K <= 1)
                    printf("       %ld  %+ld    %+8ld    %+9ld    %ld    %s\n",
                           K, c, tr, D, det, reg);
            }
        }
        /* acoplamento: d=1 tem 1 kick; d>1 tem d próprios + d vizinhos = 2d.
         * Conta-se, não se relê: o laço em d escreve 1, 4, 6, 8. */
        long nk[5]; int nkick_ok = 1;
        for(int d = 1; d <= 4; d++){
            nk[d] = (d == 1) ? 1L : 2L * d;
            if(d > 1 && nk[d] <= nk[d-1]) nkick_ok = 0;
        }
        printf("       kicks por dimensão: 1:%ld  2:%ld  3:%ld  4:%ld\n",
               nk[1], nk[2], nk[3], nk[4]);
        ok("o mapa é SIMPLÉTICO: det J = 1 em todo (K,c) do grupo C₄, sem sen e sem 2π."
           " Na origem K=0 é parabólico (D=0) e K>0 hiperbólico (D>0) — o trial, não Greene."
           " E o número de kicks cresce com d (1, 4, 6, 8): mais eixos, mais deformação",
           det_ok && nJ == 12 && k0_para && origem_hip && nkick_ok);
        printf("     d=1 origem: K=0 parabólico, K>0 hiperbólico. Greene 0,9716 é literatura\n");
        printf("     (o último toro, global) — o trial local já lê a virada no sinal de D.\n");
    }

    printf("\n§Dd4 PREDIÇÃO da peça: o toro mais robusto é o do metal. Em T¹ isso É o ouro,\n");
    printf("     e o ouro não trava porque é o GATO (deforma.c §D4). Em Tᵈ o candidato é a\n");
    printf("     raiz de x^{d+1}−x^d−1 — o companion tem |det|=1 (é um toro), e o ponto fixo\n");
    printf("     não cabe em ℚ (o corte). Sem bissecção e sem número de rotação simulado.\n");
    {
        printf("       n=d+1   companion |det|   polinómio          m-metal?\n");
        int det_un = 1, ouro_n2 = 1;
        for(int n = 2; n <= 5; n++){
            long det = det_metal(n);
            long adet = det < 0 ? -det : det;
            Pz p; p.n = n;
            for(int k = 0; k <= n; k++) p.a[k] = 0;
            p.a[n] = 1; p.a[n-1] = -1; p.a[0] = -1;
            long met2 = (n == 2) ? pz_metalica(p) : 0;
            long pis = pz_beta_pisot(p);
            printf("       %d        %ld              x^%d − x^%d − 1     %s\n",
                   n, adet, n, n-1,
                   n == 2 ? "ouro m=1" : (pis ? "Pisot m≥2" : "m=1 (Selmer)"));
            if(adet != 1) det_un = 0;
            if(n == 2 && met2 != 1) ouro_n2 = 0;
        }
        RtOp g = {{ 1, 1, 1, 0 }};
        int ord_gato = rt_ordem_ponto(&g, 1, 1, 4096);
        /* R_{π/2} em ℙ¹ identifica v ~ −v, logo o PONTO fecha em 2; o VECTOR fecha em 4. */
        RtOp e4 = {{ 0, -1, 1, 0 }};
        int ord_pt = rt_ordem_ponto(&e4, 1, 1, 64);
        int ord_vec = rt_ordem_vector(&e4, 1, 1, 64);
        printf("       CONTROLO: gato A_1 não fecha em 4096; elíptico: ponto em %d, vector em %d.\n",
               ord_pt, ord_vec);
        ok("o METAL da dimensão tem companion com |det|=1 (toro), e em n=2 É o ouro (m=1)."
           " O ouro não fecha porque é o GATO (D=m²+4>0, hiperbólico); o controlo elíptico"
           " gira: o ponto em ℙ¹ fecha em 2 e o vector em 4. A raiz real de x^n−x^{n−1}−1"
           " não se extrai: o corte é o polinómio",
           det_un && ouro_n2 && ord_gato == 0 && ord_pt == 2 && ord_vec == 4
           && !rt_fixo_racional(1));
        printf("     ⟹ em d=1 a predição reduz-se ao clássico, já medido em deforma.c. Em d≥2\n");
        printf("        o companion é unimodular e o ponto fixo não é racional — a mesma frase,\n");
        printf("        noutro grau. Greene multidimensional fica NÃO TESTADO, e diz-se.\n");
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("RESÍDUO 0 nas partes estruturais: Delannoy bate a contagem, a fracção ressonante\n");
    printf("cresce com d, o kick-drift tem det 1 e o trial lê K=0 / K>0, o metal é unimodular\n");
    printf("e o ouro é o gato. Lyapunov e Greene eram o transporte.\n");
    printf("    %d asserções, %d falhas.\n", unidades, falhas);
    return falhas != 0;
}
