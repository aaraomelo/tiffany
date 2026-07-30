/* monoide.c — O VIVEIRO É MONOIDE OU ANEL?
 *
 * Pergunta do Aarão. A resposta é MONOIDE, e o que impede o anel é exatamente aquilo que eu
 * tinha celebrado no viveiro: a IDEMPOTÊNCIA.
 *
 *   R^a ∨ R^a = R^a   ⟹   ninguém tem inverso   ⟹   (V,∨) não é grupo   ⟹   não é anel
 *
 * Um anel exige que a primeira operação forme um GRUPO abeliano — todo elemento com inverso.
 * Num conjunto idempotente isso só acontece se houver um elemento só. Então a mesma linha que
 * faz do viveiro um viveiro e não uma fábrica é a linha que o proíbe de ser anel: cruzar uma
 * espécie consigo devolve ela mesma, e daí não há como voltar atrás.
 *
 * E o que ele É, dito com precisão crescente:
 *
 *   MONOIDE           fechado, associativo, com neutro R^1
 *   comutativo        R^a ∨ R^b = R^b ∨ R^a
 *   IDEMPOTENTE       R^a ∨ R^a = R^a  — isto é, um SEMIRRETÍCULO
 *   e com o encontro  ∧ = gcd ao lado do ∨ = lcm: um RETICULADO, e DISTRIBUTIVO
 *   logo              SEMIANEL idempotente (dioide) — semianel sim, anel nunca
 *
 * A diferença entre semianel e anel é uma só e é esta: no semianel não se subtrai. E aqui não
 * se subtrai porque não se desfaz um cruzamento — o filho não devolve os pais.
 *
 *   §M1  monoide: fechado, associativo, comutativo, neutro R^1
 *   §M2  e IDEMPOTENTE — o que o torna semirretículo
 *   §M3  ANEL NÃO: nenhum elemento além do neutro tem inverso, e é a idempotência que impede
 *   §M4  com ∧ ao lado: reticulado DISTRIBUTIVO nos dois sentidos
 *   §M5  semianel: neutro do ∨ é o ANIQUILADOR do ∧ — o axioma que faltava
 *
 *   cc -O2 -std=c99 monoide.c -o monoide && ./monoide
 */
#include <stdio.h>

static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-58s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}
static int mdc(int a, int b){ while(b){ int t = a % b; a = b; b = t; } return a; }
static int mmc(int a, int b){ return a / mdc(a,b) * b; }

/* o viveiro numa torre de grau N: os elementos são os R^d com d | N. Fechado por construção,
 * porque lcm e gcd de divisores de N são divisores de N. */
#define N 12
static int V[16], NV = 0;
static int juncao(int a, int b){ return mmc(a,b); }     /* ∨ : o cruzamento */
static int encontro(int a, int b){ return mdc(a,b); }   /* ∧ : o que já tinham em comum */
static int no_viveiro(int x){ for(int t = 0; t < NV; t++) if(V[t] == x) return 1; return 0; }

int main(void){
for(int d = 1; d <= N; d++) if(N % d == 0) V[NV++] = d;
printf("\n=== O VIVEIRO É MONOIDE OU ANEL? =========================================\n");
printf("    A torre de grau %d: %d espécies (R^d com d | %d). ∨ = cruzamento, ∧ = comum.\n",
       N, NV, N);

/* ---------------------------------------------------------------- §M1 ------ */
printf("\n§M1  MONOIDE: fechado, associativo, comutativo, com neutro R^1.\n\n");
{
    int f = 1, as = 1, co = 1, ne = 1;
    long pares = 0, triplos = 0;
    for(int i = 0; i < NV; i++) for(int j = 0; j < NV; j++){
        if(!no_viveiro(juncao(V[i],V[j]))) f = 0;
        if(juncao(V[i],V[j]) != juncao(V[j],V[i])) co = 0;
        pares++;
        for(int k = 0; k < NV; k++){
            if(juncao(juncao(V[i],V[j]),V[k]) != juncao(V[i],juncao(V[j],V[k]))) as = 0;
            triplos++;
        }
    }
    for(int i = 0; i < NV; i++) if(juncao(V[i],1) != V[i]) ne = 0;
    printf("      fechado (%ld pares)          %s\n", pares, f?"sim ✓":"NÃO");
    printf("      associativo (%ld triplos)  %s\n", triplos, as?"sim ✓":"NÃO");
    printf("      comutativo                    %s\n", co?"sim ✓":"NÃO");
    printf("      neutro R^1                    %s\n", ne?"sim ✓":"NÃO");
    ok("é MONOIDE, e comutativo", f && as && co && ne);
}

/* ---------------------------------------------------------------- §M2 ------ */
printf("\n§M2  E IDEMPOTENTE — o que o faz semirretículo, e não monoide qualquer.\n\n");
{
    int idem = 1;
    for(int i = 0; i < NV; i++) if(juncao(V[i],V[i]) != V[i]) idem = 0;
    ok("R^a ∨ R^a = R^a para toda espécie", idem);
    printf("\n      Um monoide comutativo e idempotente é um SEMIRRETÍCULO. É a estrutura do\n");
    printf("      viveiro: cruzar consigo devolve a si, e só o encontro do diferente gera novo.\n");
}

/* ---------------------------------------------------------------- §M3 ------ */
printf("\n§M3  ANEL NÃO — e quem impede é a idempotência.\n\n");
{
    int com_inverso = 0, so_neutro = 1;
    printf("      espécie   existe b com R^a ∨ R^b = R^1 (o neutro)?\n");
    for(int i = 0; i < NV; i++){
        int achou = 0;
        for(int j = 0; j < NV; j++) if(juncao(V[i],V[j]) == 1) achou = 1;
        if(achou) com_inverso++;
        if(achou && V[i] != 1) so_neutro = 0;
        if(V[i]==1||V[i]==2||V[i]==3||V[i]==6||V[i]==12)
            printf("      R^%-6d  %s\n", V[i], achou ? "sim (é o próprio neutro)" : "NÃO — não há inverso");
    }
    ok("só o neutro tem inverso: os outros nenhum", so_neutro && com_inverso == 1);
    printf("\n      Um ANEL exige que a primeira operação forme um GRUPO abeliano, isto é, que\n");
    printf("      TODO elemento tenha inverso. Aqui só o neutro tem — e não é acidente da torre:\n");
    printf("      com a ∨ a = a, ter inverso obrigaria a = neutro. A idempotência PROÍBE grupo.\n");
    printf("\n      Logo a mesma linha que faz disto um viveiro e não uma fábrica é a linha que o\n");
    printf("      impede de ser anel. Não se desfaz um cruzamento: o filho não devolve os pais.\n");
}

/* ---------------------------------------------------------------- §M4 ------ */
printf("\n§M4  Com o ∧ ao lado: RETICULADO, e distributivo nos dois sentidos.\n\n");
{
    int d1 = 1, d2 = 1, abs1 = 1, abs2 = 1;
    for(int i = 0; i < NV; i++) for(int j = 0; j < NV; j++) for(int k = 0; k < NV; k++){
        if(encontro(V[i], juncao(V[j],V[k])) != juncao(encontro(V[i],V[j]), encontro(V[i],V[k]))) d1 = 0;
        if(juncao(V[i], encontro(V[j],V[k])) != encontro(juncao(V[i],V[j]), juncao(V[i],V[k]))) d2 = 0;
    }
    for(int i = 0; i < NV; i++) for(int j = 0; j < NV; j++){
        if(juncao(V[i], encontro(V[i],V[j])) != V[i]) abs1 = 0;   /* absorção */
        if(encontro(V[i], juncao(V[i],V[j])) != V[i]) abs2 = 0;
    }
    printf("      ∧ distribui sobre ∨     %s\n", d1?"sim ✓":"NÃO");
    printf("      ∨ distribui sobre ∧     %s\n", d2?"sim ✓":"NÃO");
    printf("      absorção nos dois       %s\n", (abs1&&abs2)?"sim ✓":"NÃO");
    ok("é RETICULADO, e DISTRIBUTIVO — as duas operações encaixam", d1 && d2 && abs1 && abs2);
}

/* ---------------------------------------------------------------- §M5 ------ */
printf("\n§M5  SEMIANEL: o neutro do ∨ é o ANIQUILADOR do ∧ — o axioma que faltava.\n\n");
{
    int aniq = 1, ne_enc = 1;
    for(int i = 0; i < NV; i++){
        if(encontro(V[i], 1) != 1) aniq = 0;        /* R^1 aniquila o encontro */
        if(encontro(V[i], N) != V[i]) ne_enc = 0;   /* R^N é o neutro do encontro */
    }
    printf("      R^1 ∧ R^a = R^1 (aniquila)        %s\n", aniq?"sim ✓":"NÃO");
    printf("      R^%d ∧ R^a = R^a (neutro do ∧)    %s\n", N, ne_enc?"sim ✓":"NÃO");
    ok("os axiomas de semianel fecham — é um dioide (semianel idempotente)", aniq && ne_enc);
    printf("\n      Semianel é anel MENOS a subtração. E a subtração é precisamente o que aqui\n");
    printf("      não existe: não se desfaz um cruzamento. O que falta ao viveiro para ser anel\n");
    printf("      não é um detalhe técnico — é a irreversibilidade do nascimento.\n");
    printf("\n      (O neutro do ∧ é R^%d porque a torre é de grau %d. Numa torre sem topo o ∧\n", N, N);
    printf("       não tem neutro, e aí é semirretículo dos dois lados, sem o de cima.)\n");
}

printf("\n=== A RESPOSTA ============================================================\n");
printf("  MONOIDE. Comutativo e IDEMPOTENTE, isto é, um semirretículo — e com o encontro\n");
printf("  ao lado do cruzamento, um reticulado DISTRIBUTIVO, que é um SEMIANEL idempotente.\n\n");
printf("  ANEL, NUNCA. Anel exige grupo na primeira operação, e a idempotência proíbe grupo:\n");
printf("  com a ∨ a = a, ter inverso obrigaria todo elemento a ser o neutro. Medido: dos %d,\n", NV);
printf("  só o neutro tem inverso.\n\n");
printf("  E a leitura que importa: o que falta ao viveiro para ser anel é a SUBTRAÇÃO, e ela\n");
printf("  falta porque não se desfaz um cruzamento — o filho não devolve os pais. A mesma\n");
printf("  idempotência que o faz viveiro e não fábrica é a que o impede de ser anel.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, por varredura exaustiva.\n\n");
return 0;
}
