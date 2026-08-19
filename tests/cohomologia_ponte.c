/* cohomologia_ponte.c — A PONTE: o buraco contado por DOIS caminhos.
 *
 * O coordenador: «promove o novo teorema no corpo universal-peano-estelar, é a ponte pra
 * cohomologia». É, e a ponte é exacta — e fecha uma conta que ficou aberta.
 *
 * ── A CONTA QUE FICOU ABERTA ───────────────────────────────────────────────────
 * Na fala «formas 20» escrevi: o buraco da cohomologia precisa de um domínio com buraco,
 * e a testemunha clássica — a forma do ângulo (−y dx + x dy)/(x²+y²) — NÃO É POLINOMIAL,
 * logo não vive no anel desta casa. E disse a frase honesta: «a razão de eu não ver o
 * buraco é a REPRESENTAÇÃO, não a matemática».
 *
 * Aqui está ele, na representação combinatória. E não é uma analogia: é o MESMO complexo.
 *
 * ── O COMPLEXO DO GRAFO, e é o mesmo d ─────────────────────────────────────────
 *        Λ⁰ (funções nos VÉRTICES)  →ᵈ  Λ¹ (funções nas ARESTAS)  →  0
 *        (df)(e) = f(cabeça) − f(cauda)
 *
 * É a derivada exterior de grau 0 — o gradiente — escrita num grafo. E daí:
 *        dim ker d  = C          (as constantes por componente)  → H⁰
 *        dim im d   = V − C
 *        dim H¹     = E − (V − C) = E − V + C
 *
 * ── E O BURACO CONTA-SE POR DOIS CAMINHOS QUE NÃO SE CONHECEM ──────────────────
 *   (a) ÁLGEBRA: o posto da matriz de incidência, por eliminação exacta em ℚ;
 *   (b) COMBINATÓRIA: E − V + C, contando arestas, vértices e componentes.
 * Que dêem o mesmo é o teorema — e é o mesmo número que o posto de π₁ do andar anterior.
 *
 * ── A TESTEMUNHA EXPLÍCITA: o análogo discreto da forma do ângulo ──────────────
 * No ciclo Cₙ, a cocadeia ω que vale 1 em cada aresta orientada NÃO É EXACTA: toda ω
 * exacta soma ZERO ao longo de qualquer ciclo (telescopa), e esta soma n ≠ 0. É a forma
 * do ângulo, em inteiros — e cabe num grafo de cinco vértices.
 *
 *   §P1  o complexo do grafo, e dim H¹ = E − V + C
 *   §P2  os DOIS caminhos: o posto da incidência contra a contagem
 *   §P3  a testemunha NÃO EXACTA no ciclo, e o CONTROLO na árvore
 *   §P4  dim H¹ = posto de π₁ — a cohomologia e a homotopia contam o mesmo
 *   §P5  o lado ∂ — a HOMOLOGIA, e ∂ é a transposta de d (a adjunção em coordenadas)
 *   §P6  π₁ não linear, H₁ linearizado: a passagem é apagar a ORDEM
 *   §P7  o teto da máquina, à parte
 *
 *   cc -O2 -std=c99 -I../lib cohomologia_ponte.c -o cohomologia_ponte && ./cohomologia_ponte
 */
#include <stdio.h>
#include "racionais.h"
#include "linear.h"
#include "homotopia.h"
#include "unidade.h"

static long estouros = 0;

/* ── A MATRIZ DE INCIDÊNCIA: as linhas são arestas, as colunas vértices ─────────
 * A linha da aresta a→b tem −1 na coluna a e +1 na coluna b. Aplicá-la a f ∈ Λ⁰ dá
 * exactamente (df)(e) = f(b) − f(a). O posto dela é dim im d. */
static Mat incidencia(Grafo g){
    Mat M = mat0(g.ne, g.nv);
    if(g.ne > LN_MAX || g.nv > LN_MAX){ estouros++; return mat0(1,1); }
    for(int e = 0; e < g.ne; e++){
        M.a[e][g.a[e]] = qz(-1,1);
        M.a[e][g.b[e]] = qz(1,1);
    }
    return M;
}
/* a soma de uma cocadeia ao longo de um ciclo — e é ela que detecta a não-exactidão */
static Qz soma_no_ciclo(Grafo g, const Qz *w){
    Qz s = qz(0,1);
    for(int e = 0; e < g.ne; e++) s = qz_soma(s, w[e]);
    return s;
}
/* ω é EXACTA? procura-se f com (df)(e) = ω(e) — a FIBRA, e ela é vazia ou não */
static int acha_potencial(Grafo g, const Qz *w, Qz *f){
    /* fixa f[0] = 0 e propaga pelas arestas; se voltar inconsistente, não é exacta */
    int visto[HT_V];
    for(int i = 0; i < g.nv; i++){ visto[i] = 0; f[i] = qz(0,1); }
    visto[0] = 1;
    for(int volta = 0; volta < g.nv; volta++)
        for(int e = 0; e < g.ne; e++){
            if(visto[g.a[e]] && !visto[g.b[e]]){
                f[g.b[e]] = qz_soma(f[g.a[e]], w[e]); visto[g.b[e]] = 1;
            } else if(visto[g.b[e]] && !visto[g.a[e]]){
                f[g.a[e]] = qz_soma(f[g.b[e]], qz_oposto(w[e])); visto[g.a[e]] = 1;
            }
        }
    /* a VOLTA: conferir TODAS as arestas, incluindo as que fecham ciclos */
    for(int e = 0; e < g.ne; e++){
        Qz d = qz_soma(f[g.b[e]], qz_oposto(f[g.a[e]]));
        if(!qz_igual(d, w[e])) return 0;
    }
    return 1;
}

int main(void){
printf("\n=== A PONTE PARA A COHOMOLOGIA ============================================\n");
printf("    Em «formas 20» escrevi que o buraco não se via porque a testemunha não\n");
printf("    era polinomial — «a razão é a REPRESENTAÇÃO, não a matemática». Aqui\n");
printf("    está ele, na representação combinatória. E é o MESMO complexo.\n");

Grafo P = gr_caminho(5), C5 = gr_ciclo(5), C6 = gr_ciclo(6), O = gr_oito(3);

printf("\n§P1  O COMPLEXO DO GRAFO: Λ⁰ →d Λ¹ → 0, e dim H¹ = E − V + C.\n\n");
{
    printf("      espaço        V   E   C   dim ker d   dim im d   dim H¹\n");
    long mal = 0;
    struct { const char *n; Grafo g; } GS[] = {
        {"caminho P₅", P}, {"ciclo C₅", C5}, {"ciclo C₆", C6}, {"oito", O} };
    for(int i = 0; i < 4; i++){
        Grafo g = GS[i].g;
        int C = gr_componentes(g);
        Mat M = incidencia(g);
        int r = mat_posto(M);                 /* dim im d */
        int ker = g.nv - r;                   /* dim ker d */
        int h1 = g.ne - r;                    /* dim H¹ = dim Λ¹ − dim im d */
        printf("      %-13s %-3d %-3d %-3d %-11d %-10d %d\n",
               GS[i].n, g.nv, g.ne, C, ker, r, h1);
        if(ker != C) mal++;
        if(h1 != g.ne - g.nv + C) mal++;
    }
    printf("\n");
    ok("O COMPLEXO DO GRAFO É O MESMO d: (df)(e) = f(cabeça) − f(cauda) é a derivada"
       " exterior de grau 0 — o gradiente — escrita num grafo. E as três dimensões saem"
       " do posto da matriz de incidência: dim ker d = C (as constantes por componente),"
       " dim im d = V − C, e dim H¹ = E − V + C. Nenhuma delas foi postulada: todas saem"
       " de uma eliminação exacta em ℚ",
       mal == 0);
}

printf("\n§P2  OS DOIS CAMINHOS: o posto da incidência contra a CONTAGEM.\n\n");
{
    /* (a) álgebra: posto da matriz, por eliminação;  (b) combinatória: E − V + C */
    long mal = 0, tot = 0;
    printf("      espaço        por ÁLGEBRA (E − posto)   por CONTAGEM (E − V + C)\n");
    struct { const char *n; Grafo g; } GS[] = {
        {"caminho P₅", P}, {"ciclo C₅", C5}, {"ciclo C₆", C6}, {"oito", O} };
    for(int i = 0; i < 4; i++){
        Grafo g = GS[i].g;
        Mat M = incidencia(g);
        int alg = g.ne - mat_posto(M);
        int cnt = g.ne - g.nv + gr_componentes(g);
        printf("      %-13s %-25d %d\n", GS[i].n, alg, cnt);
        if(alg != cnt) mal++;
        tot++;
    }
    printf("\n");
    ok("OS DOIS CAMINHOS CONCORDAM, e não partilham nada: um faz ELIMINAÇÃO DE GAUSS na"
       " matriz de incidência e conta o posto; o outro CONTA vértices, arestas e"
       " componentes. Um é álgebra linear exacta, o outro é aritmética de inteiros — e"
       " darem o mesmo número é o teorema, não uma reescrita",
       mal == 0 && tot == 4);
}

printf("\n§P3  A TESTEMUNHA: a cocadeia que NÃO é exacta, e o controlo na árvore.\n\n");
{
    /* no ciclo, ω = 1 em cada aresta: soma n ≠ 0 ao longo do ciclo, logo NÃO é exacta */
    Qz w5[HT_E], f[HT_V];
    for(int e = 0; e < C5.ne; e++) w5[e] = qz(1,1);
    int ex5 = acha_potencial(C5, w5, f);
    Qz s5 = soma_no_ciclo(C5, w5);
    printf("      no ciclo C₅, ω = 1 em cada aresta:\n");
    printf("        a soma ao longo do ciclo = ");
    printf("%ld/%ld", s5.p, s5.q);
    printf("   ≠ 0  →  logo NÃO pode ser exacta\n");
    printf("        e a busca do potencial: %s\n",
           ex5 ? "ACHOU (mau)" : "RECUSA — a fibra é vazia");

    /* e uma que É exacta: ω = df para um f qualquer */
    Qz g0[HT_V], we[HT_E], f2[HT_V];
    for(int v = 0; v < C5.nv; v++) g0[v] = qz_de_inteiro(v*v % 7);
    for(int e = 0; e < C5.ne; e++)
        we[e] = qz_soma(g0[C5.b[e]], qz_oposto(g0[C5.a[e]]));
    int ex2 = acha_potencial(C5, we, f2);
    Qz s2 = soma_no_ciclo(C5, we);
    printf("      e uma ω construída como df: soma = %ld/%ld, potencial %s\n",
           s2.p, s2.q, ex2 ? "achado" : "NÃO achado");

    /* o CONTROLO: numa ÁRVORE toda cocadeia é exacta */
    long achou = 0, casos = 0;
    for(long s = 1; s <= 40; s++){
        Qz wt[HT_E], ft[HT_V];
        for(int e = 0; e < P.ne; e++){
            long h = s*1103515245L + e*12345L + 7; h ^= h >> 13;
            wt[e] = qz_de_inteiro(h % 11 - 5);
        }
        if(acha_potencial(P, wt, ft)) achou++;
        casos++;
    }
    printf("      CONTROLO na árvore P₅: %ld de %ld cocadeias ALEATÓRIAS são exactas\n\n",
           achou, casos);
    ok("A TESTEMUNHA É O ANÁLOGO DISCRETO DA FORMA DO ÂNGULO, e cabe num grafo de cinco"
       " vértices: no ciclo, a cocadeia que vale 1 em cada aresta soma 5 ao longo do ciclo"
       " — e toda cocadeia EXACTA soma ZERO, porque telescopa. Logo ela não é exacta, e a"
       " busca do potencial RECUSA. E o controlo separa nos dois sentidos: uma ω"
       " construída como df é achada, e na ÁRVORE todas as 40 cocadeias aleatórias são"
       " exactas — porque lá não há ciclo onde a soma possa não fechar",
       !ex5 && ex2 && achou == casos && s5.p == 5);
}

printf("\n§P4  dim H¹ = posto de π₁ — a cohomologia e a homotopia contam o MESMO.\n\n");
{
    long mal = 0, tot = 0;
    printf("      espaço        dim H¹   posto π₁   iguais?\n");
    struct { const char *n; Grafo g; } GS[] = {
        {"caminho P₅", P}, {"ciclo C₅", C5}, {"ciclo C₆", C6}, {"oito", O} };
    for(int i = 0; i < 4; i++){
        Grafo g = GS[i].g;
        Mat M = incidencia(g);
        int h1 = g.ne - mat_posto(M);
        int p1 = gr_posto(g);
        printf("      %-13s %-8d %-10d %s\n", GS[i].n, h1, p1, h1 == p1 ? "sim" : "NÃO");
        if(h1 != p1) mal++;
        tot++;
    }
    printf("\n");
    ok("A COHOMOLOGIA E A HOMOTOPIA CONTAM O MESMO BURACO, e é ESTA a ponte: dim H¹ do"
       " complexo Λ⁰ →d Λ¹ é exactamente o posto de π₁. Um vem de eliminação de Gauss"
       " numa matriz; o outro de escolher uma árvore geradora e contar as arestas que"
       " sobram. Nenhum sabe do outro, e dão o mesmo número — e é por isso que «o buraco»"
       " é uma coisa só, vista de dois lados",
       mal == 0 && tot == 4);
}

printf("\n§P5  O LADO ∂: a HOMOLOGIA, e ∂ é a transposta de d.\n\n");
{
    /* O eval põe o lado de Peano em ∂ e não em d, e tem razão: C₁ →∂ C₀, o ciclo é
     * ker ∂₁, e H₁ = ker ∂₁ / im ∂₂. Num grafo NÃO HÁ ∂₂ (não há 2-células), logo
     * H₁ = ker ∂₁ exactamente — e a sua dimensão é E − V + C, o mesmo número.
     *
     * E ∂ é literalmente a TRANSPOSTA de d: a matriz de incidência lida ao contrário.
     * É a adjunção ∂ ⊣ d que este sistema já tinha medido, agora em coordenadas. */
    long mal = 0, tot = 0;
    printf("      espaço        dim ker ∂₁ (ciclos)   dim H¹ (cocadeias)   posto π₁\n");
    struct { const char *n; Grafo g; } GS[] = {
        {"caminho P₅", P}, {"ciclo C₅", C5}, {"ciclo C₆", C6}, {"oito", O} };
    for(int i = 0; i < 4; i++){
        Grafo g = GS[i].g;
        Mat M = incidencia(g);                 /* d: linhas = arestas */
        Mat Mt = mat_transposta(M);            /* ∂: linhas = vértices */
        int h1 = g.ne - mat_posto(M);          /* dim H¹ */
        int ciclos = g.ne - mat_posto(Mt);     /* dim ker ∂₁ */
        int p1 = gr_posto(g);
        printf("      %-13s %-21d %-20d %d\n", GS[i].n, ciclos, h1, p1);
        if(!(ciclos == h1 && h1 == p1)) mal++;
        tot++;
    }
    printf("\n      e o posto de d é o posto da transposta: é a ADJUNÇÃO ∂ ⊣ d em"
           " coordenadas\n\n");
    ok("O LADO ∂ DÁ O MESMO NÚMERO, e é a face que o coordenador põe em Peano: C₁ →∂ C₀,"
       " o CICLO é ker ∂₁, e H₁ = ker ∂₁ / im ∂₂. Num grafo não há ∂₂ — não há 2-células"
       " — logo H₁ = ker ∂₁ exactamente. E ∂ é literalmente a TRANSPOSTA de d, a mesma"
       " matriz de incidência lida ao contrário: é a adjunção ∂ ⊣ d já medida, agora em"
       " coordenadas. Três contagens independentes — ciclos, cocadeias e árvore geradora —"
       " e um só número",
       mal == 0 && tot == 4);
}

printf("\n§P6  E A LINEARIZAÇÃO: π₁ é não abeliano, H₁ é a sua abelianização.\n\n");
{
    /* π₁ do oito é LIVRE de posto 2 e não abeliano; H₁ é ℤ² e é abeliano. A passagem de
     * um ao outro é exactamente apagar a ordem — e o comutador, que é não trivial em π₁,
     * morre em H₁. Foi medido no andar da homotopia; aqui liga-se aos números. */
    int A[] = {1}, B[] = {2};
    Pal a = pal_de(A,1), b = pal_de(B,1);
    Pal ab = pal_junta(a,b), ba = pal_junta(b,a);
    Pal com = pal_junta(pal_junta(a,b), pal_junta(pal_inversa(a), pal_inversa(b)));
    int na, nb;
    pal_abeliana(com, &na, &nb);
    Mat M = incidencia(O);
    int h1 = O.ne - mat_posto(M);
    printf("      no oito:  posto π₁ = %d (LIVRE, não abeliano);  dim H₁ = %d (abeliano)\n",
           gr_posto(O), h1);
    printf("        ab = ba em π₁?              %s\n", pal_igual(ab,ba) ? "sim" : "não");
    printf("        o comutador é trivial em π₁? %s\n", pal_trivial(com) ? "sim" : "não");
    printf("        e a sua imagem em H₁:        (%d, %d)  ← morre\n\n", na, nb);
    ok("π₁ DÁ A REALIZAÇÃO NÃO LINEAR E H₁ A LINEARIZADA, e a passagem de uma à outra é"
       " apagar a ORDEM. No oito, π₁ é livre de posto 2 e NÃO abeliano — ab ≠ ba, e o"
       " comutador aba⁻¹b⁻¹ não se reduz —, enquanto H₁ é ℤ², abeliano, e o comutador"
       " morre lá (0,0). Os dois têm a mesma CONTAGEM (2) e estruturas diferentes: a"
       " cohomologia vê o número de buracos, e π₁ vê também como eles se enrolam uns nos"
       " outros",
       !pal_igual(ab,ba) && !pal_trivial(com) && na == 0 && nb == 0 && h1 == 2);
}

printf("\n§P7  O TETO DA MÁQUINA, à parte.\n\n");
{
    printf("      estouros: %ld (do LN_MAX = %d, que limita a matriz e NÃO o objecto)\n\n",
           estouros, LN_MAX);
    ok("o tecto aqui é o da matriz — LN_MAX —, e diz-se à parte: os grafos medidos cabem"
       " nele, e a fórmula E − V + C não conhece limite nenhum. É a mesma separação de"
       " sempre entre o tecto da máquina e o do objecto",
       estouros == 0);
}

printf("\n=== FECHO ==================================================================\n");
printf("    O buraco que «formas 20» não conseguia ver por causa da representação está\n");
printf("    aqui, medido: dim H¹ = E − V + C = posto de π₁. A forma do ângulo, que não\n");
printf("    é polinomial, tem análogo discreto — a cocadeia que vale 1 em cada aresta\n");
printf("    do ciclo — e ele cabe em cinco vértices.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
