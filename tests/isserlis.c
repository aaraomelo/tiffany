/* isserlis.c — O DEFEITO E_k ENTRA EM FORMA FECHADA, OU NÃO ENTRA.
 *
 *   cc -O2 -std=c99 -I lib tests/isserlis.c -o isserlis && ./isserlis
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reta.h"
#include "unidade.h"

typedef struct { long long n, d; } Q;

static long long q_gcd(long long a, long long b){
    if(a < 0) a = -a; if(b < 0) b = -b;
    while(b){ long long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
static Q q_norm(Q x){
    long long g = q_gcd(x.n, x.d);
    if(g <= 0) g = 1;
    if(x.d < 0){ g = -g; }
    return (Q){ x.n/g, x.d/g };
}
static Q q_mul(Q a, Q b){ return q_norm((Q){ a.n*b.n, a.d*b.d }); }
static Q q_add(Q a, Q b){
    long long den = a.d * b.d;
    return q_norm((Q){ a.n*b.d + b.n*a.d, den });
}

/* Σ = [[1, ρ], [ρ, 1]] com ρ = 1/2 — tudo em ℚ */
static Q SIG[2][2] = {{{1,1},{1,2}}, {{1,2},{1,1}}};

static Q isserlis_q(const int *idx, int n, int *usado){
    int primeiro = -1;
    for(int i = 0; i < n; i++) if(!usado[i]){ primeiro = i; break; }
    if(primeiro < 0) return (Q){1,1};
    usado[primeiro] = 1;
    Q soma = {0,1};
    for(int j = primeiro + 1; j < n; j++){
        if(usado[j]) continue;
        usado[j] = 1;
        soma = q_add(soma, q_mul(SIG[idx[primeiro]][idx[j]], isserlis_q(idx, n, usado)));
        usado[j] = 0;
    }
    usado[primeiro] = 0;
    return soma;
}

static int q_eq(Q a, Q b){ return a.n * b.d == b.n * a.d; }

static long long conta_emparelhamentos(int n, int *usado){
    int primeiro = -1;
    for(int i = 0; i < n; i++) if(!usado[i]){ primeiro = i; break; }
    if(primeiro < 0) return 1;
    usado[primeiro] = 1;
    long long c = 0;
    for(int j = primeiro + 1; j < n; j++){
        if(usado[j]) continue;
        usado[j] = 1;
        c += conta_emparelhamentos(n, usado);
        usado[j] = 0;
    }
    usado[primeiro] = 0;
    return c;
}
static long long dupfat(long long n){
    long long r = 1;
    for(long long i = n; i > 1; i -= 2) r *= i;
    return r;
}

int main(void){
printf("\n=== O DEFEITO EM FORMA FECHADA: ISSERLIS ===================================\n");
printf("    E_k é polinomial de grau 4, logo a esperança é soma de momentos — e momento\n");
printf("    gaussiano é contagem de emparelhamentos. Combinatória é inteira.\n");

printf("\n§I1  Isserlis contra valores exactos em ℚ — Σ = [[1,1/2],[1/2,1]].\n\n");
{
    struct { int idx[8]; int p; const char *nome; Q esp; } casos[] = {
        {{0,0},        2, "E[x0²]",        {1,1}},
        {{0,1},        2, "E[x0 x1]",      {1,2}},
        {{0,0,0,0},    4, "E[x0⁴]",        {3,1}},
        {{0,0,1,1},    4, "E[x0² x1²]",    {3,2}},
        {{0,1,1,1},    4, "E[x0 x1³]",     {3,2}},
        {{0,0,0,1},    4, "E[x0³ x1]",     {3,2}},
        {{0,0,0,0,0,0},6, "E[x0⁶]",        {15,1}},
        {{0,0,1,1,1,1},6, "E[x0² x1⁴]",    {6,1}},
        {{0,1,0,1,0,1},6, "E[x0³ x1³]",    {21,4}},
    };
    int mau = 0;
    printf("      momento          Isserlis (ℚ)     esperado         confere?\n");
    for(unsigned c = 0; c < sizeof casos/sizeof casos[0]; c++){
        int usado[8]; memset(usado, 0, sizeof usado);
        Q vi = isserlis_q(casos[c].idx, casos[c].p, usado);
        int okq = q_eq(vi, casos[c].esp);
        printf("      %-14s %6lld/%-6lld   %6lld/%-6lld   %s\n",
               casos[c].nome, vi.n, vi.d, casos[c].esp.n, casos[c].esp.d, okq?"✓":"✗");
        if(!okq) mau++;
    }
    ok("Isserlis = valores exactos em ℚ, em todos os momentos", mau == 0);
    printf("\n      Note E[x0² x1²] = 1 + 2ρ² = 3/2 com ρ=1/2 — os três emparelhamentos, um\n");
    printf("      dando Σ00·Σ11 e dois dando Σ01². É a fórmula, não um ajuste.\n");
}

printf("\n§I2  A contagem: os emparelhamentos de 2n pontos são (2n−1)!!.\n\n");
{
    int mau = 0;
    printf("      2n    força bruta    (2n−1)!!\n");
    for(int n = 1; n <= 6; n++){
        int usado[16]; memset(usado, 0, sizeof usado);
        long long c = conta_emparelhamentos(2*n, usado);
        long long f = dupfat(2*n - 1);
        printf("      %2d    %11lld    %8lld\n", 2*n, c, f);
        if(c != f) mau++;
    }
    ok("a contagem bate o duplo fatorial, sem exceção", mau == 0);
}

printf("\n§I3  N_k É um número de Isserlis — e isso a tabela do capítulo não diz.\n");
printf("     Ela dá N_k = 3, 15, 105, 945 para k = 3, 5, 7, 9. Conferindo contra a\n");
printf("     contagem de emparelhamentos de k+1 pontos, por força bruta:\n\n");
{
    long long Nk_tabela[4] = {3, 15, 105, 945};
    int ks[4] = {3, 5, 7, 9};
    int mau = 0;
    printf("      k    N_k (tabela)   emparelhamentos de k+1 pontos   k!!\n");
    for(int i = 0; i < 4; i++){
        int usado[16]; memset(usado, 0, sizeof usado);
        long long c = conta_emparelhamentos(ks[i] + 1, usado);
        long long d = dupfat(ks[i]);
        printf("      %d    %12lld   %29lld   %6lld\n", ks[i], Nk_tabela[i], c, d);
        if(c != Nk_tabela[i] || d != Nk_tabela[i]) mau++;
    }
    ok("N_k = emparelhamentos de k+1 pontos = k!!", mau == 0);
    printf("\n      Logo a constante não é um número avulso do cálculo: é a própria contagem de\n");
    printf("      Wick aparecendo no resultado. É o que torna esta verificação genuína em vez\n");
    printf("      de circular — a tabela dá o valor, e a combinatória diz DE ONDE ele vem.\n");
}

printf("\n§I4  As constantes fechadas contra a tabela — e as duas formas de C_k².\n\n");
{
    long long Nk[4] = {3, 15, 105, 945}, ak[4] = {192, 1152, 6912, 41472};
    long long Ck2[4] = {36, 1080, 45360, 2449440};
    int ks[4] = {3, 5, 7, 9};
    const long long bk = 16;
    int mau_a = 0, mau_c1 = 0, mau_c2 = 0;
    printf("      k    a_k = 32·6^((k−1)/2)   N_k·a_k/b_k    2k(k−2)!!·6^((k−1)/2)   tabela\n");
    for(int i = 0; i < 4; i++){
        int k = ks[i];
        long long a = 32 * rt_ipow(6, (k-1)/2);
        long long c1 = Nk[i] * a / bk;
        long long c2 = 2LL * k * dupfat(k-2) * rt_ipow(6, (k-1)/2);
        printf("      %d    %20lld   %11lld    %21lld   %8lld\n", k, a, c1, c2, Ck2[i]);
        if(a != ak[i]) mau_a++;
        if(c1 != Ck2[i]) mau_c1++;
        if(c2 != Ck2[i]) mau_c2++;
    }
    ok("a_k = 32·6^((k−1)/2) bate a tabela", mau_a == 0);
    ok("C_k² = N_k·a_k/b_k bate a tabela", mau_c1 == 0);
    ok("C_k² = 2k(k−2)!!·6^((k−1)/2) bate a tabela", mau_c2 == 0);

    int mau_id = 0;
    for(int i = 0; i < 4; i++){
        int k = ks[i];
        if(dupfat(k) != (long long)k * dupfat(k-2)) mau_id++;
        if(32*rt_ipow(6,(k-1)/2) != bk * 2 * rt_ipow(6,(k-1)/2)) mau_id++;
    }
    ok("as duas formas são UMA identidade (k!! = k·(k−2)!!, a_k/b_k = 2·6^…)", mau_id == 0);
    printf("\n      Ou seja: C_k² = 2·N_k·6^((k−1)/2), com N_k a contagem de Wick. O ‘2’ vem de\n");
    printf("      a_k/b_k = 32/16, e o 6^((k−1)/2) é o que sobra de a_k. Nada avulso.\n");
}

printf("\n=== O QUE ISSO LIBERA ======================================================\n");
printf("  O defeito E_k pode entrar nesta bateria sem virar estimativa: a esperança\n");
printf("  gaussiana é soma finita de emparelhamentos, validada de fora contra valores\n");
printf("  exactos em ℚ (§I1), e as constantes do capítulo são elas próprias contagens de Wick\n");
printf("  (§I3). Logo a contração de k chaves passa a ter DEFEITO CALCULÁVEL — uma medida\n");
printf("  de quanto a composição se afasta da recuperação exata, e não só uma notação.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato em todas as seções.\n\n");
return 0;
}
