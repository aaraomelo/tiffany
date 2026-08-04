/* expressao.c — EXPRESSÕES EQUIVALENTES SÃO O MESMO TENSOR. E o que sobra é o invariante.
 *
 * Simplificação simbólica costuma ser um saco de REGRAS de reescrita — x+0→x, x·1→x, x−x→0 —
 * e cada regra é uma decisão de quem escreve. Com tensor não há regra a escolher: a expressão
 * CONTRAI num objeto, e duas expressões equivalentes contraem no MESMO objeto. A simplificação
 * deixa de ser reescrita e passa a ser contração.
 *
 *   grau 1:  a expressão é um VETOR de coeficientes (c₀ ; c₁ … c_n)
 *   grau 2:  é um TENSOR simétrico T[i][j], porque x_i·x_j = x_j·x_i
 *
 * E o cancelamento deixa de ser regra: em (a+b) − (a+c), o `a` some porque 1 − 1 = 0. Ninguém
 * escreveu "cancele termos iguais" — a aritmética do coeficiente faz sozinha.
 *
 * O que se mede, tudo em inteiros e por varredura EXAUSTIVA (sem amostra):
 *
 *   §X1  contrair é somar coeficiente: o cancelamento sai da aritmética, não de regra
 *   §X2  mesmo tensor ⟺ mesmo valor em TODO ponto — a equivalência é o tensor, os dois lados
 *   §X3  grau 2: a simetria é CONTADA (n(n+1)/2, não n²), não imposta
 *   §X4  as classes de equivalência SÃO os tensores: #classes = #tensores distintos
 *   §X5  o invariante: o tensor não muda sob reescrita nenhuma — é o que sobra no fim
 *
 *   cc -O2 -std=c99 expressao.c -o expressao && ./expressao
 */
#include <stdio.h>
#include <string.h>

#define NV     3          /* as variáveis: a, b, c                     */
#define NTERM  8          /* termos que uma expressão pode ter escrita */
#define LO    -4
#define HI     4

#include "unidade.h"
static const char *NOME = "abc";

/* ---------- a expressão COMO ESCRITA: uma lista de termos ---------- */
struct expr {
    int n;
    long coef[NTERM];
    int  var[NTERM];       /* 0..NV-1, ou -1 para o termo constante */
};
static void zera(struct expr *e){ e->n = 0; }
static void termo(struct expr *e, long c, int v){
    if(e->n < NTERM){ e->coef[e->n] = c; e->var[e->n] = v; e->n++; }
}
static void escreve(const struct expr *e, char *out, size_t cap){
    size_t k = 0; out[0] = 0;
    for(int i = 0; i < e->n; i++){
        char t[32];
        long c = e->coef[i];
        if(e->var[i] < 0) snprintf(t, sizeof t, "%s%ld", (i && c >= 0) ? "+" : "", c);
        else if(c == 1)   snprintf(t, sizeof t, "%s%c", i ? "+" : "", NOME[e->var[i]]);
        else if(c == -1)  snprintf(t, sizeof t, "-%c", NOME[e->var[i]]);
        else              snprintf(t, sizeof t, "%s%ld%c", (i && c >= 0) ? "+" : "", c, NOME[e->var[i]]);
        size_t l = strlen(t);
        if(k + l + 1 < cap){ memcpy(out + k, t, l); k += l; out[k] = 0; }
    }
    if(!k) snprintf(out, cap, "0");
}
/* ---------- a CONTRAÇÃO: a expressão vira vetor ---------- */
struct vetor { long c0; long c[NV]; };
static struct vetor contrai(const struct expr *e){
    struct vetor v; memset(&v, 0, sizeof v);
    for(int i = 0; i < e->n; i++){
        if(e->var[i] < 0) v.c0 += e->coef[i];
        else              v.c[e->var[i]] += e->coef[i];   /* o cancelamento acontece AQUI */
    }
    return v;
}
static int vetor_igual(struct vetor a, struct vetor b){
    if(a.c0 != b.c0) return 0;
    for(int i = 0; i < NV; i++) if(a.c[i] != b.c[i]) return 0;
    return 1;
}
static long avalia(const struct expr *e, const long *x){
    long s = 0;
    for(int i = 0; i < e->n; i++)
        s += e->coef[i] * (e->var[i] < 0 ? 1 : x[e->var[i]]);
    return s;
}
static long avalia_vetor(struct vetor v, const long *x){
    long s = v.c0;
    for(int i = 0; i < NV; i++) s += v.c[i] * x[i];
    return s;
}

int main(void){
printf("\n=== EXPRESSÕES EQUIVALENTES SÃO O MESMO TENSOR =============================\n");
printf("    Sem regra de reescrita. A expressão contrai, e a contração já é a classe.\n");

/* ---------------------------------------------------------------- §X1 ------ */
printf("\n§X1  Contrair é somar coeficiente — e o cancelamento sai da aritmética.\n\n");
{
    struct expr e1, e2, e3;
    zera(&e1); termo(&e1,1,0); termo(&e1,1,1); termo(&e1,-1,0); termo(&e1,-1,2); /* (a+b)−(a+c) */
    zera(&e2); termo(&e2,1,1); termo(&e2,-1,2);                                  /* b−c        */
    zera(&e3); termo(&e3,1,0); termo(&e3,1,0);                                   /* a+a        */
    char s1[64], s2[64], s3[64];
    escreve(&e1,s1,sizeof s1); escreve(&e2,s2,sizeof s2); escreve(&e3,s3,sizeof s3);
    struct vetor v1 = contrai(&e1), v2 = contrai(&e2), v3 = contrai(&e3);
    printf("      escrita          vetor (c0 ; a b c)\n");
    printf("      %-14s   (%ld ; %ld %ld %ld)\n", s1, v1.c0, v1.c[0], v1.c[1], v1.c[2]);
    printf("      %-14s   (%ld ; %ld %ld %ld)\n", s2, v2.c0, v2.c[0], v2.c[1], v2.c[2]);
    printf("      %-14s   (%ld ; %ld %ld %ld)\n", s3, v3.c0, v3.c[0], v3.c[1], v3.c[2]);
    ok("(a+b)−(a+c) e b−c contraem no MESMO vetor", vetor_igual(v1, v2));
    printf("\n      O `a` sumiu porque 1 + (−1) = 0. Não há regra de cancelamento neste código:\n");
    printf("      há uma soma. E `a+a` não vira `2a` por reescrita — ele JÁ É (0;2 0 0).\n");
}

/* ---------------------------------------------------------------- §X2 ------ */
printf("\n§X2  Mesmo tensor ⟺ mesmo valor em TODO ponto. Os dois sentidos, exaustivo.\n");
printf("     %d³ = %d pontos por par, varridos inteiros — nenhuma amostragem.\n\n",
       HI-LO+1, (HI-LO+1)*(HI-LO+1)*(HI-LO+1));
{
    /* gera expressões escritas de formas diferentes, sistematicamente */
    struct expr E[600]; int ne = 0;
    for(int i = 0; i < NV && ne < 600; i++)
      for(int j = 0; j < NV && ne < 600; j++)
        for(long s = -2; s <= 2 && ne < 600; s++)
          for(long t = -1; t <= 1 && ne < 600; t++){
              struct expr e; zera(&e);
              termo(&e, s, i); termo(&e, 1, j); termo(&e, -1, j); termo(&e, 1, i); termo(&e, t, -1);
              E[ne++] = e;                              /* escrita com par que se cancela */
              struct expr f; zera(&f);
              termo(&f, s+1, i); termo(&f, t, -1);
              E[ne++] = f;                              /* a mesma coisa, escrita curta   */
          }
    long mesmo_tensor_mesmo_valor = 0, mesmo_tensor = 0;
    long tensor_diferente = 0, diferente_e_difere = 0;
    for(int p = 0; p < ne; p++) for(int q = p+1; q < ne; q++){
        struct vetor vp = contrai(&E[p]), vq = contrai(&E[q]);
        int igual_em_todo_ponto = 1, difere_em_algum = 0;
        long x[NV];
        for(x[0] = LO; x[0] <= HI; x[0]++)
        for(x[1] = LO; x[1] <= HI; x[1]++)
        for(x[2] = LO; x[2] <= HI; x[2]++){
            long a = avalia(&E[p], x), b = avalia(&E[q], x);
            if(a != b){ igual_em_todo_ponto = 0; difere_em_algum = 1; }
        }
        if(vetor_igual(vp, vq)){ mesmo_tensor++; if(igual_em_todo_ponto) mesmo_tensor_mesmo_valor++; }
        else { tensor_diferente++; if(difere_em_algum) diferente_e_difere++; }
    }
    printf("      expressões geradas ............................ %d\n", ne);
    printf("      pares com o MESMO tensor ..................... %ld\n", mesmo_tensor);
    printf("        e que valem igual em todo ponto ............ %ld\n", mesmo_tensor_mesmo_valor);
    printf("      pares com tensor DIFERENTE ................... %ld\n", tensor_diferente);
    printf("        e que diferem em algum ponto ............... %ld\n", diferente_e_difere);
    ok("mesmo tensor ⟹ mesmo valor em todo ponto", mesmo_tensor == mesmo_tensor_mesmo_valor);
    ok("tensor diferente ⟹ difere em algum ponto", tensor_diferente == diferente_e_difere);
    printf("\n      Os dois sentidos fecham: o tensor não é um resumo com perda da expressão —\n");
    printf("      é exatamente a classe de equivalência dela.\n");
}

/* ---------------------------------------------------------------- §X3 ------ */
printf("\n§X3  Grau 2: a simetria é CONTADA, não imposta.\n\n");
{
    /* monômio x_i·x_j: a entrada (i,j) de um tensor. x_i x_j = x_j x_i ⟹ T é simétrico */
    long T[NV][NV]; memset(T, 0, sizeof T);
    int distintos = 0;
    for(int i = 0; i < NV; i++) for(int j = 0; j < NV; j++){
        int a = i < j ? i : j, b = i < j ? j : i;       /* a mesma casa para (i,j) e (j,i) */
        if(T[a][b] == 0) distintos++;
        T[a][b]++;
    }
    printf("      pares (i,j) possíveis ......................... %d  (= n²)\n", NV*NV);
    printf("      casas distintas do tensor ..................... %d\n", distintos);
    printf("      n(n+1)/2 ...................................... %d\n", NV*(NV+1)/2);
    ok("as casas são n(n+1)/2 — a parte simétrica", distintos == NV*(NV+1)/2);
    printf("\n      Ninguém escreveu a regra `xy = yx`: o monômio caiu na mesma casa porque a\n");
    printf("      casa é o PAR não-ordenado. A comutatividade virou endereço.\n");
}

/* ---------------------------------------------------------------- §X4 ------ */
printf("\n§X4  As classes de equivalência SÃO os tensores.\n\n");
{
    struct expr E[400]; int ne = 0;
    for(long s = -3; s <= 3 && ne < 400; s++)
      for(long t = -3; t <= 3 && ne < 400; t++)
        for(int i = 0; i < NV && ne < 400; i++){
            struct expr e; zera(&e); termo(&e, s, i); termo(&e, t, -1); E[ne++] = e;
            struct expr f; zera(&f); termo(&f, s, i); termo(&f, 1, 1); termo(&f, -1, 1);
            termo(&f, t, -1); E[ne++] = f;               /* a mesma, com ruído que cancela */
        }
    /* classes por comportamento (avaliação em todo ponto) contra classes por tensor */
    int classe_val[400], classe_ten[400];
    for(int i = 0; i < ne; i++){ classe_val[i] = -1; classe_ten[i] = -1; }
    int nv_ = 0, nt_ = 0;
    for(int i = 0; i < ne; i++){
        if(classe_val[i] < 0){
            classe_val[i] = nv_;
            for(int j = i+1; j < ne; j++){
                if(classe_val[j] >= 0) continue;
                int igual = 1; long x[NV];
                for(x[0] = LO; x[0] <= HI && igual; x[0]++)
                for(x[1] = LO; x[1] <= HI && igual; x[1]++)
                for(x[2] = LO; x[2] <= HI && igual; x[2]++)
                    if(avalia(&E[i], x) != avalia(&E[j], x)) igual = 0;
                if(igual) classe_val[j] = nv_;
            }
            nv_++;
        }
        if(classe_ten[i] < 0){
            classe_ten[i] = nt_;
            struct vetor vi = contrai(&E[i]);
            for(int j = i+1; j < ne; j++)
                if(classe_ten[j] < 0 && vetor_igual(vi, contrai(&E[j]))) classe_ten[j] = nt_;
            nt_++;
        }
    }
    int mesma_particao = 1;
    for(int i = 0; i < ne; i++) for(int j = i+1; j < ne; j++)
        if((classe_val[i] == classe_val[j]) != (classe_ten[i] == classe_ten[j])) mesma_particao = 0;
    printf("      expressões .................................... %d\n", ne);
    printf("      classes por COMPORTAMENTO (varredura completa)  %d\n", nv_);
    printf("      classes por TENSOR ............................ %d\n", nt_);
    ok("o número de classes coincide", nv_ == nt_);
    ok("e é a MESMA partição, par a par", mesma_particao);
    printf("\n      Logo a classe não precisa ser calculada por comparação: ela É o tensor.\n");
    printf("      Calcular equivalência é O(1) numa contração, e não O(pontos) numa varredura.\n");
}

/* ---------------------------------------------------------------- §X5 ------ */
printf("\n§X5  O invariante: reescrever não mexe no tensor. É o que sobra no fim.\n\n");
{
    struct expr base; zera(&base);
    termo(&base, 2, 0); termo(&base, -1, 1); termo(&base, 3, -1);   /* 2a − b + 3 */
    struct vetor v0 = contrai(&base);
    char s0[64]; escreve(&base, s0, sizeof s0);

    long mexeu = 0, tentadas = 0;
    printf("      partindo de  %s   →  (%ld ; %ld %ld %ld)\n\n", s0, v0.c0, v0.c[0], v0.c[1], v0.c[2]);
    printf("      reescrita                          vetor            mudou?\n");
    /* três famílias de reescrita que preservam o significado */
    for(int i = 0; i < NV; i++){
        struct expr e = base;                              /* (1) somar e subtrair a mesma coisa */
        termo(&e, 1, i); termo(&e, -1, i);
        struct vetor v = contrai(&e); tentadas++;
        if(!vetor_igual(v, v0)) mexeu++;
        char s[64]; escreve(&e, s, sizeof s);
        printf("      %-32s (%ld ; %ld %ld %ld)   %s\n", s, v.c0, v.c[0], v.c[1], v.c[2],
               vetor_igual(v,v0) ? "não ✓" : "MUDOU ✗");
    }
    {
        struct expr e; zera(&e);                           /* (2) partir um termo em dois */
        termo(&e, 1, 0); termo(&e, 1, 0); termo(&e, -1, 1); termo(&e, 1, -1); termo(&e, 2, -1);
        struct vetor v = contrai(&e); tentadas++;
        if(!vetor_igual(v, v0)) mexeu++;
        char s[64]; escreve(&e, s, sizeof s);
        printf("      %-32s (%ld ; %ld %ld %ld)   %s\n", s, v.c0, v.c[0], v.c[1], v.c[2],
               vetor_igual(v,v0) ? "não ✓" : "MUDOU ✗");
    }
    {
        struct expr e; zera(&e);                           /* (3) trocar a ordem dos termos */
        termo(&e, 3, -1); termo(&e, -1, 1); termo(&e, 2, 0);
        struct vetor v = contrai(&e); tentadas++;
        if(!vetor_igual(v, v0)) mexeu++;
        char s[64]; escreve(&e, s, sizeof s);
        printf("      %-32s (%ld ; %ld %ld %ld)   %s\n", s, v.c0, v.c[0], v.c[1], v.c[2],
               vetor_igual(v,v0) ? "não ✓" : "MUDOU ✗");
    }
    printf("\n      reescritas testadas ........................... %ld\n", tentadas);
    printf("      que mexeram no tensor ......................... %ld\n", mexeu);
    ok("o tensor é INVARIANTE sob toda reescrita testada", mexeu == 0);
    printf("\n      É o que sobra depois de tirar tudo o que é neutro: o 0 da soma e o 1 do\n");
    printf("      produto somem porque não têm coeficiente próprio, e o par que se cancela\n");
    printf("      soma zero. O que fica não é o menor jeito de ESCREVER a expressão —\n");
    printf("      é a expressão sem a escrita. O invariante da operação.\n");
}

printf("\n=== O QUE ISSO RESOLVE =====================================================\n");
printf("  Simplificar deixa de ser um saco de regras e passa a ser CONTRAIR. Não há\n");
printf("  `x+0→x` neste código, nem `x−x→0`: há soma de coeficiente, e o resto acontece.\n");
printf("  A classe de equivalência não se calcula comparando — ela É o tensor, e por isso\n");
printf("  decidir se duas expressões são a mesma custa uma contração, não uma varredura.\n");
printf("  E a comutatividade não é regra: é o par não-ordenado ser o endereço da casa.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
