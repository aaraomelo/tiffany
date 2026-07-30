/* tensor.c — O PARÊNTESE É A POSIÇÃO. A SIMETRIA ELIMINA A REDUNDÂNCIA. O QUE SOBRA É O INVARIANTE.
 *
 * expressao.c contraiu o grau 1: a expressão vira VETOR, e escritas equivalentes viram o mesmo
 * vetor. Aqui sobe-se de grau, e a regra é a do Aarão: cada PARÊNTESE corresponde a uma posição
 * do tensor. Um produto de k parênteses é um k-TENSOR, e a entrada (i₁…i_k) é o coeficiente do
 * monômio x_{i₁}···x_{i_k}.
 *
 *     (a+b)·(a+c)   →   T[i][j] = f₁[i] · f₂[j]        (produto externo: a posição é o fator)
 *
 * E a redundância não se remove por regra: ela É a simetria do tensor. Como a multiplicação
 * comuta, as k! permutações de uma posição são o MESMO monômio, e o representante é a posição
 * ORDENADA. Sobram C(n+k−1, k) casas de n^k — e a conta é exata, não estimada.
 *
 * O índice 0 é a constante 1, então um fator é afim (c₀ + Σ cᵢxᵢ) e os monômios de grau ≤ k
 * entram todos no mesmo objeto. Nada de tratar constante à parte.
 *
 *   §T1  o parêntese é a posição: o produto externo, entrada por entrada
 *   §T2  a simetria elimina: n^k posições, C(n+k−1,k) monômios — contado, não imposto
 *   §T3  o que sobra AVALIA igual, em todo ponto (varredura exaustiva)
 *   §T4  ordem dos parênteses não importa: mesmo tensor reduzido
 *   §T5  o que sobra é o invariante: escritas diferentes, tensor idêntico
 *
 *   cc -O2 -std=c99 tensor.c -o tensor && ./tensor
 */
#include <stdio.h>
#include <string.h>

#define NV   3                 /* as variáveis a, b, c                              */
#define NI   (NV+1)            /* os índices: 0 = a constante 1, 1..NV = as variáveis */
#define KMAX 3                 /* ordem máxima do tensor (parênteses no produto)     */
#define LO  -3
#define HI   3

static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-56s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}
static const char *SIMB[NI] = { "1", "a", "b", "c" };

/* um fator é um vetor afim: f[0]·1 + f[1]·a + f[2]·b + f[3]·c */
typedef long Fator[NI];

static void escreve_fator(const Fator f, char *o, size_t cap){
    size_t k = 0; o[0] = 0;
    for(int i = 0; i < NI; i++){
        if(!f[i]) continue;
        char t[24];
        if(i == 0)            snprintf(t, sizeof t, "%s%ld", (k && f[i] > 0) ? "+" : "", f[i]);
        else if(f[i] ==  1)   snprintf(t, sizeof t, "%s%s", k ? "+" : "", SIMB[i]);
        else if(f[i] == -1)   snprintf(t, sizeof t, "-%s", SIMB[i]);
        else                  snprintf(t, sizeof t, "%s%ld%s", (k && f[i] > 0) ? "+" : "", f[i], SIMB[i]);
        size_t l = strlen(t);
        if(k + l + 1 < cap){ memcpy(o + k, t, l); k += l; o[k] = 0; }
    }
    if(!k) snprintf(o, cap, "0");
}
static long vale_fator(const Fator f, const long *x){
    long s = f[0];
    for(int i = 1; i < NI; i++) s += f[i] * x[i-1];
    return s;
}

/* ---- o k-tensor cheio: n^k casas, indexadas pela POSIÇÃO (um índice por parêntese) ---- */
static long T[NI*NI*NI];
static int  potencia(int k){ int p = 1; while(k--) p *= NI; return p; }

static void constroi(const Fator *f, int k){
    memset(T, 0, sizeof T);
    int n = potencia(k);
    for(int cod = 0; cod < n; cod++){
        int idx[KMAX], c = cod;
        long v = 1;
        for(int t = 0; t < k; t++){ idx[t] = c % NI; c /= NI; v *= f[t][idx[t]]; }
        T[cod] = v;                                  /* produto externo: a posição é o fator */
    }
}
/* a posição ORDENADA é o representante do monômio — a simetria em ato */
static int canoniza(int cod, int k){
    int idx[KMAX], c = cod;
    for(int t = 0; t < k; t++){ idx[t] = c % NI; c /= NI; }
    for(int i = 1; i < k; i++)                       /* ordenação simples */
        for(int j = i; j > 0 && idx[j] < idx[j-1]; j--){ int t = idx[j]; idx[j] = idx[j-1]; idx[j-1] = t; }
    int r = 0;
    for(int t = k-1; t >= 0; t--) r = r*NI + idx[t];
    return r;
}
static void simetriza(long *S, int k){
    memset(S, 0, sizeof(long) * (size_t)potencia(k));
    int n = potencia(k);
    for(int cod = 0; cod < n; cod++) S[canoniza(cod, k)] += T[cod];
}
static long vale_reduzido(const long *S, int k, const long *x){
    long soma = 0, n = potencia(k);
    for(int cod = 0; cod < n; cod++){
        if(!S[cod]) continue;
        if(canoniza(cod, k) != cod) continue;        /* só os representantes */
        int idx[KMAX], c = cod;
        long m = S[cod];
        for(int t = 0; t < k; t++){ idx[t] = c % NI; c /= NI; m *= (idx[t] == 0 ? 1 : x[idx[t]-1]); }
        soma += m;
    }
    return soma;
}
static long binom(int n, int k){
    long r = 1;
    for(int i = 1; i <= k; i++) r = r * (n - k + i) / i;
    return r;
}

int main(void){
printf("\n=== O PARÊNTESE É A POSIÇÃO DO TENSOR =====================================\n");
printf("    Um produto de k parênteses é um k-tensor. A simetria tira a redundância.\n");

/* ---------------------------------------------------------------- §T1 ------ */
printf("\n§T1  O parêntese dá a posição: o produto externo, entrada por entrada.\n\n");
{
    Fator f[2];
    memset(f, 0, sizeof f);
    f[0][1] = 1; f[0][2] = 1;                      /* (a + b) */
    f[1][1] = 1; f[1][3] = 1;                      /* (a + c) */
    char s0[32], s1[32];
    escreve_fator(f[0], s0, sizeof s0); escreve_fator(f[1], s1, sizeof s1);
    printf("      (%s)·(%s)\n\n", s0, s1);
    constroi(f, 2);
    printf("      posição (i,j)   monômio   T[i][j]\n");
    for(int i = 0; i < NI; i++) for(int j = 0; j < NI; j++){
        int cod = i + j*NI;
        if(!T[cod]) continue;
        printf("      (%d,%d)           %s·%s       %ld\n", i, j, SIMB[i], SIMB[j], T[cod]);
    }
    printf("\n      O índice do tensor É o número do parêntese: o primeiro fator indexa i,\n");
    printf("      o segundo indexa j. Nada de expandir e depois agrupar — a posição já é o\n");
    printf("      endereço, e o coeficiente cai nela por produto.\n");
}

/* ---------------------------------------------------------------- §T2 ------ */
printf("\n§T2  A simetria elimina: as k! permutações são a MESMA casa. Contado.\n\n");
{
    printf("      k    posições (n^k)   monômios distintos   C(n+k−1,k)\n");
    int mau = 0;
    for(int k = 1; k <= KMAX; k++){
        int n = potencia(k), dist = 0;
        for(int cod = 0; cod < n; cod++) if(canoniza(cod, k) == cod) dist++;
        long esperado = binom(NI + k - 1, k);
        printf("      %d    %14d   %18d   %10ld\n", k, n, dist, esperado);
        if(dist != esperado) mau++;
    }
    ok("os monômios distintos são C(n+k−1,k)", mau == 0);
    printf("\n      Ninguém escreveu `xy = yx`. A casa é a posição ORDENADA, e as k! escritas\n");
    printf("      caem nela por endereço. A redundância some porque não tinha onde existir.\n");
}

/* ---------------------------------------------------------------- §T3 ------ */
printf("\n§T3  O que sobra AVALIA igual — em todo ponto, varredura exaustiva.\n\n");
{
    long S[NI*NI*NI];
    int mau = 0, casos = 0;
    long pontos = 0;
    /* varre famílias de fatores e confere o reduzido contra o produto original */
    for(long p = -2; p <= 2; p++)
    for(long q = -2; q <= 2; q++)
    for(int  i = 1; i < NI; i++)
    for(int  j = 1; j < NI; j++){
        Fator f[2]; memset(f, 0, sizeof f);
        f[0][0] = p; f[0][i] = 1;
        f[1][0] = q; f[1][j] = 1;
        constroi(f, 2);
        simetriza(S, 2);
        casos++;
        long x[NV];
        for(x[0] = LO; x[0] <= HI; x[0]++)
        for(x[1] = LO; x[1] <= HI; x[1]++)
        for(x[2] = LO; x[2] <= HI; x[2]++){
            long orig = vale_fator(f[0], x) * vale_fator(f[1], x);
            long red  = vale_reduzido(S, 2, x);
            pontos++;
            if(orig != red) mau++;
        }
    }
    printf("      produtos testados ............................. %d\n", casos);
    printf("      pontos avaliados .............................. %ld\n", pontos);
    printf("      divergências .................................. %d\n", mau);
    ok("o tensor reduzido vale o mesmo que o produto original", mau == 0);
    printf("\n      Reduzir não perdeu nada: as casas que sumiram estavam repetindo.\n");
}

/* ---------------------------------------------------------------- §T4 ------ */
printf("\n§T4  A ordem dos parênteses não importa — o reduzido é o mesmo.\n\n");
{
    long S1[NI*NI*NI], S2[NI*NI*NI];
    Fator f[2], g[2];
    memset(f, 0, sizeof f); memset(g, 0, sizeof g);
    f[0][1] = 1; f[0][2] = 1;  f[1][0] = 2; f[1][3] = 1;     /* (a+b)·(2+c) */
    g[0][0] = 2; g[0][3] = 1;  g[1][1] = 1; g[1][2] = 1;     /* (2+c)·(a+b) */
    constroi(f, 2); simetriza(S1, 2);
    constroi(g, 2); simetriza(S2, 2);
    int igual = memcmp(S1, S2, sizeof(long)*(size_t)potencia(2)) == 0;
    char a0[32], a1[32], b0[32], b1[32];
    escreve_fator(f[0],a0,sizeof a0); escreve_fator(f[1],a1,sizeof a1);
    escreve_fator(g[0],b0,sizeof b0); escreve_fator(g[1],b1,sizeof b1);
    printf("      (%s)·(%s)   e   (%s)·(%s)\n", a0, a1, b0, b1);
    ok("dão o MESMO tensor reduzido, casa a casa", igual);
    printf("\n      No tensor CHEIO eles são diferentes (T[i][j] contra T[j][i]); no reduzido\n");
    printf("      são um só. A comutatividade do produto virou a simetria do objeto.\n");
}

/* ---------------------------------------------------------------- §T5 ------ */
printf("\n§T5  O que sobra é o invariante: escritas diferentes, tensor idêntico.\n\n");
{
    long S1[NI*NI*NI], S2[NI*NI*NI], S3[NI*NI*NI];
    Fator f[2], g[2], h[2];
    memset(f,0,sizeof f); memset(g,0,sizeof g); memset(h,0,sizeof h);
    /* (a+b)·(a-b)  contra  (a-b)·(a+b)  contra  (b+a)·(a-b) */
    f[0][1]=1; f[0][2]=1;   f[1][1]=1; f[1][2]=-1;
    g[0][1]=1; g[0][2]=-1;  g[1][1]=1; g[1][2]=1;
    h[0][2]=1; h[0][1]=1;   h[1][1]=1; h[1][2]=-1;
    constroi(f,2); simetriza(S1,2);
    constroi(g,2); simetriza(S2,2);
    constroi(h,2); simetriza(S3,2);
    size_t nb = sizeof(long)*(size_t)potencia(2);
    ok("(a+b)(a−b) e (a−b)(a+b) dão o mesmo reduzido", memcmp(S1,S2,nb) == 0);
    ok("e (b+a)(a−b) também", memcmp(S1,S3,nb) == 0);
    printf("\n      o que sobra de (a+b)(a−b):\n");
    printf("      monômio   coeficiente\n");
    int n = potencia(2);
    for(int cod = 0; cod < n; cod++){
        if(canoniza(cod, 2) != cod || !S1[cod]) continue;
        int i = cod % NI, j = (cod/NI) % NI;
        printf("      %s·%-6s  %ld\n", SIMB[i], SIMB[j], S1[cod]);
    }
    printf("\n      Sobrou a·a − b·b. O termo cruzado a·b apareceu duas vezes, com sinais\n");
    printf("      opostos, e somou zero NA CASA — não porque alguém mandou cancelar, mas\n");
    printf("      porque as duas escritas endereçam a mesma casa e a soma resolve.\n");
}

printf("\n=== O QUE FICA ============================================================\n");
printf("  O parêntese é a posição; o produto de k parênteses é um k-tensor; e a simetria\n");
printf("  do tensor elimina a redundância sozinha, porque as k! permutações de uma posição\n");
printf("  são o mesmo endereço. De n^k casas sobram C(n+k−1,k), e o que sobra avalia igual\n");
printf("  ao original em todo ponto — logo não é resumo, é a mesma coisa sem a repetição.\n");
printf("  E é invariante: a escrita muda, o tensor não. É ele o que a expressão É.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
