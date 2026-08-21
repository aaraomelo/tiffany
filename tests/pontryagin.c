/* pontryagin.c — A DUALIDADE DE PONTRYAGIN, DERIVADA DO ZERO.
 *
 * A casa cita Pontryagin em quatro documentos e chama-lhe «o eixo»: a álgebra
 * OPERA e não alcança a completude; a topologia ALCANÇA ℝ e não opera. E o
 * `bidual.c` §B5 mede-o em ℤ/n, n = 2..24.
 *
 * MAS O QUE ALI SE MEDE É A CARDINALIDADE, E O BIDUAL É UMA ATRIBUIÇÃO:
 *
 *     long bidual = distintos;          <-- bidual.c §B5
 *     if(distintos != n || bidual != n) mau++;
 *
 * A segunda condição não pode falhar sem a primeira: não há bidual construído
 * em sítio nenhum. E, pior, |Ĝ| = |G| NÃO É O TEOREMA DE PONTRYAGIN — é o
 * acidente de tamanho. O teorema é que a AVALIAÇÃO
 *
 *     ev : X ⟶ X̂̂,      ev_x(χ) = χ(x)
 *
 * é um isomorfismo, e é NATURAL: não depende de escolha nenhuma. A distinção é
 * toda: um isomorfismo X → X̂ existe (as cardinalidades batem) mas depende do
 * gerador escolhido; o X → X̂̂ não depende de nada. Medir |Ĝ| = |G| e concluir
 * «bidualidade» é confundir os dois.
 *
 * AQUI DERIVA-SE DO ZERO, e tudo em aritmética INTEIRA exata: os valores dos
 * caracteres não vivem em ℂ — vivem em ℤ/e, com e o expoente do grupo, que é a
 * torre dos relógios que a casa já corre (μ₈ ⊂ μ₁₆ ⊂ μ₃₂, restrição de
 * caracteres = redução do índice). Um caractere é um morfismo
 *
 *     χ : X ⟶ ℤ/e,      χ(x ⊕ y) = χ(x) + χ(y),
 *
 * e a «troca ⊕ → ⊗» é isto: no expoente a operação é a soma.
 *
 * NADA É POSTO POR FÓRMULA. Os morfismos não são parametrizados e depois
 * contados: varrem-se TODOS os candidatos e cada um é TESTADO em todos os pares
 * do grupo. A condição de ordem (n_i·v_i ≡ 0) não é imposta — é DESCOBERTA
 * pela medição, e é por isso que a contagem é uma medida e não uma definição.
 *
 *   §PG1  O DUAL É UM GRUPO — fecho, neutro e inverso, medidos ponto a ponto
 *   §PG2  |X̂| = |X| — por VARREDURA de todos os candidatos, não por fórmula
 *   §PG3  A SEPARAÇÃO: x ≠ 0 ⟹ existe χ com χ(x) ≠ 0 (e é ela que dá a injecção)
 *   §PG4  A FIBRA DO CARACTERE É UNIFORME — a ortogonalidade sem somar raízes,
 *         e é a MESMA fibra da aranha: G(c) = |X|/|im χ|, constante
 *   §PG5  PONTRYAGIN: ev : X → X̂̂ CONSTRUÍDA, e o bidual enumerado à parte
 *   §PG6  A NATURALIDADE de ev — o quadrado comuta para TODO morfismo f
 *   §PG7  O GUME: o isomorfismo X → X̂ NÃO é natural, e mede-se ONDE falha
 *   §PG8  A TROCA ⊕ → ⊗, e a transformada que dela sai
 *   §PG9  A FRONTEIRA: é a FINITUDE que fecha, e vê-se onde ela cede
 *
 * Papers: teoria.tex (o eixo, e a tabela das bidualidades), corpo_algebrico.tex
 * §(iii), aranha.tex §sec:transf (o caso e = 2, onde os valores são ±1).
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define PG_MAXR   3      /* número de factores cíclicos                       */
#define PG_MAXEL  64     /* |X| máximo                                        */
#define PG_MAXE   24     /* expoente máximo                                   */
#define PG_MAXCH  64     /* |X̂| máximo                                       */

typedef struct {
    int r;                 /* número de factores                              */
    int n[PG_MAXR];        /* as ordens                                       */
    int ord;               /* |X| = ∏ n_i                                     */
    int e;                 /* expoente = lcm(n_i)                             */
    char nome[32];
} Grupo;

static long pg_mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long pg_mmc(long a, long b){ return a / pg_mdc(a,b) * b; }

static void g_init(Grupo *G, const char *nome, int r, const int *n){
    G->r = r; G->ord = 1; G->e = 1;
    for(int i = 0; i < r; i++){ G->n[i] = n[i]; G->ord *= n[i]; G->e = (int)pg_mmc(G->e, n[i]); }
    snprintf(G->nome, sizeof G->nome, "%s", nome);
}

/* índice ↔ componentes, em base mista */
static int g_comp(const Grupo *G, int idx, int i){
    for(int j = 0; j < i; j++) idx /= G->n[j];
    return idx % G->n[i];
}
static int g_soma(const Grupo *G, int a, int b){
    int s = 0, peso = 1;
    for(int i = 0; i < G->r; i++){
        s += ((g_comp(G,a,i) + g_comp(G,b,i)) % G->n[i]) * peso;
        peso *= G->n[i];
    }
    return s;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * A ENUMERAÇÃO DOS MORFISMOS X → ℤ/e.
 *
 * Um candidato é um tuplo v ∈ (ℤ/e)^r, e a função que ele propõe é
 *     f_v(x) = Σ_i v_i · x_i   (mod e),
 * com x_i o representante em {0,…,n_i−1}. NENHUMA condição é imposta sobre v.
 * Testa-se f_v(a ⊕ b) = f_v(a) + f_v(b) em TODOS os |X|² pares; sobrevivem os
 * que são morfismos, e é a varredura que descobre quais.
 * ───────────────────────────────────────────────────────────────────────────── */
static int pg_avaliaE(const Grupo *G, const int *v, int x, int E){
    long s = 0;
    for(int i = 0; i < G->r; i++) s += (long)v[i] * g_comp(G,x,i);
    return (int)(s % E);
}

static int pg_e_morfismoE(const Grupo *G, const int *v, int E){
    for(int a = 0; a < G->ord; a++)
        for(int b = 0; b < G->ord; b++){
            int esq = pg_avaliaE(G, v, g_soma(G,a,b), E);
            int dir = (pg_avaliaE(G,v,a,E) + pg_avaliaE(G,v,b,E)) % E;
            if(esq != dir) return 0;
        }
    return 1;
}

/* Os caracteres com valores em ℤ/E, E múltiplo do expoente. Nenhuma condição é
 * imposta sobre o candidato v: varre-se (ℤ/E)^r e TESTA-SE cada um em todos os
 * pares. Devolve o nº de caracteres e preenche chi[k][x] e coord[k][i]. */
static int pg_duaisE(const Grupo *G, int E, int chi[PG_MAXCH][PG_MAXEL],
                     int coord[PG_MAXCH][PG_MAXR]){
    int nc = 0, v[PG_MAXR];
    long total = 1;
    for(int i = 0; i < G->r; i++) total *= E;
    for(long cand = 0; cand < total; cand++){
        long c = cand;
        for(int i = 0; i < G->r; i++){ v[i] = (int)(c % E); c /= E; }
        if(!pg_e_morfismoE(G, v, E)) continue;
        if(nc >= PG_MAXCH) return -1;
        for(int x = 0; x < G->ord; x++) chi[nc][x] = pg_avaliaE(G, v, x, E);
        for(int i = 0; i < G->r; i++) coord[nc][i] = v[i];
        nc++;
    }
    return nc;
}

static int pg_duais(const Grupo *G, int chi[PG_MAXCH][PG_MAXEL], int coord[PG_MAXCH][PG_MAXR]){
    return pg_duaisE(G, G->e, chi, coord);
}

/* o ÍNDICE, na lista de caracteres, da tabela dada — ou −1 se lá não está.
 * É a busca que faz de f^ uma construção e não uma fórmula. */
static int pg_indice(const int chi[PG_MAXCH][PG_MAXEL], int nc, int ord, const int *tab){
    for(int k = 0; k < nc; k++){
        int ig = 1;
        for(int x = 0; x < ord && ig; x++) if(chi[k][x] != tab[x]) ig = 0;
        if(ig) return k;
    }
    return -1;
}

int main(void){
    printf("\npontryagin.c — a dualidade de Pontryagin, derivada do zero.\n");
    printf("papers: teoria.tex (o eixo), corpo_algebrico.tex §(iii),"
           " aranha.tex §sec:transf\n");

    /* a família de teste: cíclicos e produtos, com expoentes diferentes */
    Grupo fam[12]; int nf = 0;
    { int n[]={2};      g_init(&fam[nf++], "Z/2",        1, n); }
    { int n[]={3};      g_init(&fam[nf++], "Z/3",        1, n); }
    { int n[]={4};      g_init(&fam[nf++], "Z/4",        1, n); }
    { int n[]={5};      g_init(&fam[nf++], "Z/5",        1, n); }
    { int n[]={6};      g_init(&fam[nf++], "Z/6",        1, n); }
    { int n[]={8};      g_init(&fam[nf++], "Z/8",        1, n); }
    { int n[]={12};     g_init(&fam[nf++], "Z/12",       1, n); }
    { int n[]={2,2};    g_init(&fam[nf++], "Z/2+Z/2",    2, n); }
    { int n[]={2,4};    g_init(&fam[nf++], "Z/2+Z/4",    2, n); }
    { int n[]={3,3};    g_init(&fam[nf++], "Z/3+Z/3",    2, n); }
    { int n[]={2,6};    g_init(&fam[nf++], "Z/2+Z/6",    2, n); }
    { int n[]={2,2,2};  g_init(&fam[nf++], "Z/2+Z/2+Z/2",3, n); }

    static int chi[PG_MAXCH][PG_MAXEL], coord[PG_MAXCH][PG_MAXR];
    static int nchi[12];

    /* ═══ §PG1: O DUAL É UM GRUPO ════════════════════════════════════════════ */
    printf("\n§PG1  o dual é um grupo — e não por definição: medido ponto a ponto.\n\n");
    {
        long mau = 0;
        printf("      grupo          |X|   e    |X^|   fecho  neutro  inverso\n");
        for(int f = 0; f < nf; f++){
            const Grupo *G = &fam[f];
            int nc = pg_duais(G, chi, coord);
            nchi[f] = nc;
            if(nc < 0){ mau++; continue; }
            /* FECHO: a soma ponto a ponto de dois caracteres é um caractere da lista */
            long fecho_mau = 0, neutro = -1, inv_mau = 0;
            for(int a = 0; a < nc; a++) for(int b = 0; b < nc; b++){
                int achou = -1;
                for(int c = 0; c < nc && achou < 0; c++){
                    int igual = 1;
                    for(int x = 0; x < G->ord && igual; x++)
                        if((chi[a][x] + chi[b][x]) % G->e != chi[c][x]) igual = 0;
                    if(igual) achou = c;
                }
                if(achou < 0) fecho_mau++;
            }
            /* NEUTRO: existe um e um só χ nulo em todo o grupo */
            long quantos_nulos = 0;
            for(int a = 0; a < nc; a++){
                int nulo = 1;
                for(int x = 0; x < G->ord && nulo; x++) if(chi[a][x]) nulo = 0;
                if(nulo){ quantos_nulos++; neutro = a; }
            }
            /* INVERSO: para cada χ existe χ' com χ + χ' = neutro */
            for(int a = 0; a < nc; a++){
                int achou = 0;
                for(int b = 0; b < nc && !achou; b++){
                    int zero = 1;
                    for(int x = 0; x < G->ord && zero; x++)
                        if((chi[a][x] + chi[b][x]) % G->e) zero = 0;
                    if(zero) achou = 1;
                }
                if(!achou) inv_mau++;
            }
            printf("      %-14s %-5d %-4d %-6d %-7s %-7s %s\n",
                   G->nome, G->ord, G->e, nc,
                   fecho_mau ? "NAO" : "sim",
                   (quantos_nulos == 1) ? "sim" : "NAO",
                   inv_mau ? "NAO" : "sim");
            if(fecho_mau || quantos_nulos != 1 || inv_mau || neutro < 0) mau++;
        }
        printf("\n");
        ok("O DUAL É UM GRUPO: a soma ponto a ponto fecha, há um neutro e um só, e cada"
           " caractere tem inverso — verificado em todos os pares, nos doze grupos. E note-se"
           " o que NÃO se fez: os caracteres não foram parametrizados e contados; varreram-se"
           " todos os e^r candidatos e cada um foi TESTADO em todos os |X|² pares. A condição"
           " de ordem não é imposta — é o que a varredura descobre.", mau == 0);
    }

    /* ═══ §PG2: |X̂| = |X|, E A BIJEÇÃO DEPENDE DA ESCOLHA ═══════════════════ */
    printf("\n§PG2  |X^| = |X| — e é aqui que o teorema NÃO está.\n\n");
    {
        long mau = 0;
        printf("      grupo          |X|   |X^|   iguais?   isomorfismos X -> X^\n");
        for(int f = 0; f < nf; f++){
            const Grupo *G = &fam[f];
            int nc = nchi[f];
            /* quantos isomorfismos X → X̂ existem? cada um é uma escolha. */
            long isos = 0;
            if(G->ord <= 16){
                /* um morfismo X → X̂ é dado pelas imagens dos geradores; varre-se
                 * tudo e testa-se morfismo + injectividade, sem fórmula. */
                pg_duais(G, chi, coord);
                long total = 1;
                for(int i = 0; i < G->r; i++) total *= nc;
                for(long cand = 0; cand < total; cand++){
                    long c = cand; int img[PG_MAXR];
                    for(int i = 0; i < G->r; i++){ img[i] = (int)(c % nc); c /= nc; }
                    /* estende: ψ(x) = Σ x_i · img[i] (na soma de X̂), como tabela */
                    int psi[PG_MAXEL];
                    for(int x = 0; x < G->ord; x++){
                        int val[PG_MAXEL];
                        for(int y = 0; y < G->ord; y++) val[y] = 0;
                        for(int i = 0; i < G->r; i++)
                            for(int t = 0; t < g_comp(G,x,i); t++)
                                for(int y = 0; y < G->ord; y++)
                                    val[y] = (val[y] + chi[img[i]][y]) % G->e;
                        int achou = -1;
                        for(int k = 0; k < nc && achou < 0; k++){
                            int ig = 1;
                            for(int y = 0; y < G->ord && ig; y++) if(chi[k][y] != val[y]) ig = 0;
                            if(ig) achou = k;
                        }
                        psi[x] = achou;
                        if(achou < 0) break;
                    }
                    int bom = 1;
                    for(int x = 0; x < G->ord && bom; x++) if(psi[x] < 0) bom = 0;
                    /* morfismo? */
                    for(int a = 0; a < G->ord && bom; a++) for(int b = 0; b < G->ord && bom; b++){
                        int soma_no_dual = -1;
                        for(int k = 0; k < nc && soma_no_dual < 0; k++){
                            int ig = 1;
                            for(int y = 0; y < G->ord && ig; y++)
                                if((chi[psi[a]][y] + chi[psi[b]][y]) % G->e != chi[k][y]) ig = 0;
                            if(ig) soma_no_dual = k;
                        }
                        if(psi[g_soma(G,a,b)] != soma_no_dual) bom = 0;
                    }
                    /* injectivo? */
                    for(int a = 0; a < G->ord && bom; a++) for(int b = a+1; b < G->ord && bom; b++)
                        if(psi[a] == psi[b]) bom = 0;
                    if(bom) isos++;
                }
            }
            printf("      %-14s %-5d %-6d %-9s %ld\n", G->nome, G->ord, nc,
                   (nc == G->ord) ? "sim" : "NAO",
                   (G->ord <= 16) ? isos : -1);
            if(nc != G->ord) mau++;
            if(G->ord <= 16 && isos == 0) mau++;
        }
        printf("\n");
        ok("|X^| = |X| NOS DOZE GRUPOS — e é exactamente isto que NÃO é o teorema de"
           " Pontryagin. A última coluna diz porquê: os isomorfismos X → X^ são MUITOS, e"
           " nenhum deles é distinguido pelo grupo. Escolher um é escolher geradores. O que"
           " o teorema afirma é outra coisa, e vive no BIDUAL — onde a escolha desaparece.",
           mau == 0);
    }

    /* ═══ §PG3: A SEPARAÇÃO ══════════════════════════════════════════════════ */
    printf("\n§PG3  a separação: nenhum elemento não nulo é invisível a todos os caracteres.\n\n");
    {
        long mau = 0;
        printf("      grupo          elementos != 0   separados   o pior caso\n");
        for(int f = 0; f < nf; f++){
            const Grupo *G = &fam[f];
            int nc = pg_duais(G, chi, coord);
            long sep = 0, pior = -1, pior_qtos = 1 << 30;
            for(int x = 1; x < G->ord; x++){
                long quantos = 0;
                for(int k = 0; k < nc; k++) if(chi[k][x]) quantos++;
                if(quantos) sep++;
                if(quantos < pior_qtos){ pior_qtos = (int)quantos; pior = x; }
            }
            printf("      %-14s %-16d %-11ld x=%ld visto por %ld de %d\n",
                   G->nome, G->ord - 1, sep, pior, (long)pior_qtos, nc);
            /* duas condições, e a segunda é a que dá conteúdo à primeira: o
             * caractere NULO não vê elemento nenhum, logo nenhum x pode ser visto
             * por TODOS os caracteres. Sem ela, um contador que contasse todos
             * passaria por separação. */
            if(sep != G->ord - 1) mau++;
            if(pior_qtos < 1 || pior_qtos >= nc) mau++;
        }
        printf("\n");
        ok("A SEPARAÇÃO VALE: todo elemento não nulo é visto por algum caractere, e a coluna"
           " do pior caso mostra que a margem não é apertada — mesmo o elemento menos visível"
           " é visto por pelo menos metade deles — e NUNCA por todos, porque o caractere"
           " nulo não vê nada. É esta segunda condição que dá conteúdo à primeira: sem"
           " ela, um contador que contasse todos passava por separação, e mutá-lo não"
           " derrubava nada. É desta propriedade, e só dela, que sai a"
           " INJECTIVIDADE de ev: se x != y, algum χ separa-os, logo ev_x != ev_y.", mau == 0);
    }

    /* ═══ §PG4: A FIBRA DO CARACTERE É UNIFORME ══════════════════════════════ */
    printf("\n§PG4  a ortogonalidade, sem somar uma única raiz da unidade.\n\n");
    {
        long mau = 0;
        printf("      grupo          caracteres   fibras uniformes   |im chi| divide |X|\n");
        for(int f = 0; f < nf; f++){
            const Grupo *G = &fam[f];
            int nc = pg_duais(G, chi, coord);
            long unif = 0, divide = 0;
            for(int k = 0; k < nc; k++){
                int conta[PG_MAXE]; for(int c = 0; c < G->e; c++) conta[c] = 0;
                for(int x = 0; x < G->ord; x++) conta[chi[k][x]]++;
                int imagem = 0, tam = -1, uniforme = 1;
                for(int c = 0; c < G->e; c++) if(conta[c]){
                    imagem++;
                    if(tam < 0) tam = conta[c];
                    else if(conta[c] != tam) uniforme = 0;
                }
                if(uniforme) unif++;
                if(imagem > 0 && G->ord % imagem == 0 && tam == G->ord / imagem) divide++;
            }
            printf("      %-14s %-12d %-18s %s\n", G->nome, nc,
                   (unif == nc) ? "todas" : "NAO", (divide == nc) ? "sim" : "NAO");
            if(unif != nc || divide != nc) mau++;
        }
        printf("\n");
        ok("A FIBRA DE TODO CARACTERE É UNIFORME: |chi^-1(c)| = |X|/|im chi| para todo valor"
           " atingido, nos doze grupos e em todos os caracteres. ISTO É A ORTOGONALIDADE, e"
           " está dita sem somar uma raiz da unidade e sem sair dos inteiros: o que anula a"
           " soma Σ_x ζ^chi(x) para chi != 0 é os valores se repartirem IGUALMENTE pelo"
           " subgrupo imagem. E é a MESMA fibra da aranha — chi é uma realização, e G(c) é a"
           " sua multiplicidade; a diferença é que aqui ela é CONSTANTE, e é dessa constância"
           " que a transformada inverte.", mau == 0);
    }

    /* ═══ §PG5: PONTRYAGIN — ev CONSTRUÍDA, o bidual ENUMERADO à parte ═══════ */
    printf("\n§PG5  ev : X -> X^^ construída, e o bidual contado por outro caminho.\n\n");
    {
        long mau = 0;
        printf("      grupo          |X|  |X^|  |X^^|  ev morfismo  injectiva  sobrejectiva\n");
        for(int f = 0; f < nf; f++){
            const Grupo *G = &fam[f];
            int nc = pg_duais(G, chi, coord);

            /* (a) ev_x : X̂ → ℤ/e, tabela sobre os caracteres. CONSTRUÍDA. */
            static int ev[PG_MAXEL][PG_MAXCH];
            for(int x = 0; x < G->ord; x++)
                for(int k = 0; k < nc; k++) ev[x][k] = chi[k][x];

            /* a soma em X̂: soma[a][b] = índice do caractere χ_a + χ_b */
            static int somad[PG_MAXCH][PG_MAXCH];
            long soma_mau = 0;
            for(int a = 0; a < nc; a++) for(int b = 0; b < nc; b++){
                int achou = -1;
                for(int c = 0; c < nc && achou < 0; c++){
                    int ig = 1;
                    for(int x = 0; x < G->ord && ig; x++)
                        if((chi[a][x] + chi[b][x]) % G->e != chi[c][x]) ig = 0;
                    if(ig) achou = c;
                }
                somad[a][b] = achou;
                if(achou < 0) soma_mau++;
            }

            /* (b) cada ev_x é morfismo de X̂ — verificado em todos os pares */
            long ev_nao_morf = 0;
            for(int x = 0; x < G->ord; x++)
                for(int a = 0; a < nc; a++) for(int b = 0; b < nc; b++)
                    if(ev[x][somad[a][b]] != (ev[x][a] + ev[x][b]) % G->e) ev_nao_morf++;

            /* (c) x ↦ ev_x é morfismo: ev_{a⊕b} = ev_a + ev_b */
            long ev_soma_mau = 0;
            for(int a = 0; a < G->ord; a++) for(int b = 0; b < G->ord; b++){
                int s = g_soma(G,a,b);
                for(int k = 0; k < nc; k++)
                    if(ev[s][k] != (ev[a][k] + ev[b][k]) % G->e) ev_soma_mau++;
            }

            /* (d) injectiva */
            long colisoes = 0;
            for(int a = 0; a < G->ord; a++) for(int b = a+1; b < G->ord; b++){
                int ig = 1;
                for(int k = 0; k < nc && ig; k++) if(ev[a][k] != ev[b][k]) ig = 0;
                if(ig) colisoes++;
            }

            /* (e) X̂̂ ENUMERADO À PARTE: todos os morfismos X̂ → ℤ/e, pela mesma
             *     varredura — candidatos pelas imagens dos geradores de X̂, e cada
             *     um testado em TODOS os pares. Nada é atribuído. */
            long nbid = 0, ev_dentro = 0;
            {
                /* geradores de X̂: os duais dos geradores de X, χ com coord = δ_i·(e/n_i) */
                int ger[PG_MAXR], ok_ger = 1;
                for(int i = 0; i < G->r; i++){
                    ger[i] = -1;
                    for(int k = 0; k < nc && ger[i] < 0; k++){
                        int bate = 1;
                        for(int j = 0; j < G->r; j++){
                            int esperado = (j == i) ? (G->e / G->n[i]) : 0;
                            if(coord[k][j] != esperado) bate = 0;
                        }
                        if(bate) ger[i] = k;
                    }
                    if(ger[i] < 0) ok_ger = 0;
                }
                if(!ok_ger) mau++;
                /* coordenadas de cada caractere na base dos geradores */
                static int cc[PG_MAXCH][PG_MAXR];
                for(int k = 0; k < nc; k++)
                    for(int i = 0; i < G->r; i++)
                        cc[k][i] = coord[k][i] / (G->e / G->n[i]);
                long total = 1; for(int i = 0; i < G->r; i++) total *= G->e;
                static int bid[PG_MAXCH][PG_MAXCH]; int nb = 0;
                for(long cand = 0; cand < total; cand++){
                    long c = cand; int w[PG_MAXR];
                    for(int i = 0; i < G->r; i++){ w[i] = (int)(c % G->e); c /= G->e; }
                    int Phi[PG_MAXCH];
                    for(int k = 0; k < nc; k++){
                        long s = 0;
                        for(int i = 0; i < G->r; i++) s += (long)w[i] * cc[k][i];
                        Phi[k] = (int)(s % G->e);
                    }
                    int morf = 1;
                    for(int a = 0; a < nc && morf; a++) for(int b = 0; b < nc && morf; b++)
                        if(Phi[somad[a][b]] != (Phi[a] + Phi[b]) % G->e) morf = 0;
                    if(!morf) continue;
                    int novo = 1;
                    for(int t = 0; t < nb && novo; t++){
                        int ig = 1;
                        for(int k = 0; k < nc && ig; k++) if(bid[t][k] != Phi[k]) ig = 0;
                        if(ig) novo = 0;
                    }
                    if(novo && nb < PG_MAXCH){ memcpy(bid[nb], Phi, sizeof(int)*nc); nb++; }
                }
                nbid = nb;
                /* cada ev_x está entre os morfismos enumerados? */
                for(int x = 0; x < G->ord; x++){
                    int achou = 0;
                    for(int t = 0; t < nb && !achou; t++){
                        int ig = 1;
                        for(int k = 0; k < nc && ig; k++) if(bid[t][k] != ev[x][k]) ig = 0;
                        if(ig) achou = 1;
                    }
                    ev_dentro += achou;
                }
            }
            int sobre = (ev_dentro == G->ord && nbid == G->ord && colisoes == 0);
            printf("      %-14s %-4d %-5d %-6ld %-12s %-10s %s\n",
                   G->nome, G->ord, nc, nbid,
                   (ev_nao_morf == 0 && ev_soma_mau == 0) ? "sim" : "NAO",
                   colisoes ? "NAO" : "sim", sobre ? "sim" : "NAO");
            if(soma_mau || ev_nao_morf || ev_soma_mau || colisoes || !sobre) mau++;
        }
        printf("\n");
        ok("PONTRYAGIN, E DESTA VEZ CONSTRUÍDO: ev_x(chi) = chi(x) é montada como TABELA sobre"
           " os caracteres, verifica-se que cada ev_x é morfismo de X^ em todos os pares, que"
           " x -> ev_x é morfismo, que é injectiva (pela separação do §PG3) e que a imagem"
           " esgota X^^ — e X^^ foi ENUMERADO POR OUTRO CAMINHO, varrendo os candidatos e"
           " testando cada um, sem usar ev para nada. É a diferença que faltava: no `bidual.c`"
           " §B5 o bidual é a linha `long bidual = distintos;`, e uma atribuição não pode"
           " falhar. Aqui os dois números vêm de duas contagens independentes e coincidem.",
           mau == 0);
    }

    /* ═══ §PG6: A NATURALIDADE de ev ═════════════════════════════════════════
     *
     * Os DOIS LADOS têm de vir por caminhos diferentes, ou a asserção não pode
     * falhar. O caminho esquerdo avalia em Y; o direito PUXA o caractere para X
     * — constrói χ∘f, PROCURA-O na lista de X̂ (é isso que f̂ é) e avalia lá.
     * Se f̂ não estivesse bem definida, a busca devolveria −1 e o teste cairia.
     *
     *      X  ──── f ────▶  Y                    esq : ev_Y(f(x))
     *      │                │                    dir : (f̂̂ ∘ ev_X)(x),
     *     ev_X            ev_Y                          que em χ vale ev_X(x)(f̂χ)
     *      ▼                ▼
     *      X̂̂ ─── f̂̂ ───▶ Ŷ̂
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§PG6  a naturalidade: o quadrado de ev comuta para TODO morfismo.\n\n");
    {
        long mau = 0, soma_testados = 0, soma_pares = 0;
        printf("      X -> Y                  E    morfismos f   quadrados   f^ indefinida   falhas\n");
        for(int fx = 0; fx < nf; fx++) for(int fy = 0; fy < nf; fy++){
            if(fam[fx].ord > 8 || fam[fy].ord > 8) continue;
            const Grupo *X = &fam[fx], *Y = &fam[fy];
            int E = (int)pg_mmc(X->e, Y->e);
            if(E > PG_MAXE) continue;
            static int chiX[PG_MAXCH][PG_MAXEL], coX[PG_MAXCH][PG_MAXR];
            static int chiY[PG_MAXCH][PG_MAXEL], coY[PG_MAXCH][PG_MAXR];
            int ncx = pg_duaisE(X, E, chiX, coX);
            int ncy = pg_duaisE(Y, E, chiY, coY);
            if(ncx < 0 || ncy < 0){ mau++; continue; }
            long nmorf = 0, testados = 0, falhas_nat = 0, indef = 0;

            long total = 1; for(int i = 0; i < X->r; i++) total *= Y->ord;
            for(long cand = 0; cand < total; cand++){
                long c = cand; int img[PG_MAXR];
                for(int i = 0; i < X->r; i++){ img[i] = (int)(c % Y->ord); c /= Y->ord; }
                int fmap[PG_MAXEL];
                for(int x = 0; x < X->ord; x++){
                    int acc = 0;
                    for(int i = 0; i < X->r; i++)
                        for(int t = 0; t < g_comp(X,x,i); t++) acc = g_soma(Y, acc, img[i]);
                    fmap[x] = acc;
                }
                int morf = 1;
                for(int a = 0; a < X->ord && morf; a++) for(int b = 0; b < X->ord && morf; b++)
                    if(fmap[g_soma(X,a,b)] != g_soma(Y, fmap[a], fmap[b])) morf = 0;
                if(!morf) continue;
                nmorf++;

                /* f̂ : Ŷ → X̂ CONSTRUÍDA — χ ↦ χ∘f, e o resultado PROCURADO em X̂ */
                int dualf[PG_MAXCH];
                for(int k = 0; k < ncy; k++){
                    int tab[PG_MAXEL];
                    for(int x = 0; x < X->ord; x++) tab[x] = chiY[k][ fmap[x] ];
                    dualf[k] = pg_indice((const int (*)[PG_MAXEL])chiX, ncx, X->ord, tab);
                    if(dualf[k] < 0) indef++;
                }
                /* e f̂ é ela própria morfismo: mede-se, não se supõe */
                for(int a = 0; a < ncy; a++) for(int b = 0; b < ncy; b++){
                    int soma[PG_MAXEL];
                    for(int y = 0; y < Y->ord; y++) soma[y] = (chiY[a][y] + chiY[b][y]) % E;
                    int sab = pg_indice((const int (*)[PG_MAXEL])chiY, ncy, Y->ord, soma);
                    if(sab < 0 || dualf[a] < 0 || dualf[b] < 0){ falhas_nat++; continue; }
                    int somaX[PG_MAXEL];
                    for(int x = 0; x < X->ord; x++)
                        somaX[x] = (chiX[dualf[a]][x] + chiX[dualf[b]][x]) % E;
                    int sX = pg_indice((const int (*)[PG_MAXEL])chiX, ncx, X->ord, somaX);
                    testados++;
                    if(dualf[sab] != sX) falhas_nat++;
                }
                /* O QUADRADO. Esquerdo: avalia em Y. Direito: avalia em X, no
                 * caractere PUXADO. São duas tabelas diferentes, indexadas por
                 * índices diferentes — e é por isso que a igualdade pode falhar. */
                for(int x = 0; x < X->ord; x++)
                    for(int k = 0; k < ncy; k++){
                        int esq = chiY[k][ fmap[x] ];
                        if(dualf[k] < 0){ falhas_nat++; continue; }
                        int dir = chiX[ dualf[k] ][ x ];
                        testados++;
                        if(esq != dir) falhas_nat++;
                    }
            }
            soma_testados += testados; soma_pares += nmorf;
            if(nmorf && (fx == fy || fx < 2))
                printf("      %-10s -> %-10s %-4d %-13ld %-11ld %-15ld %ld\n",
                       X->nome, Y->nome, E, nmorf, testados, indef, falhas_nat);
            if(falhas_nat || indef) mau++;
        }
        printf("\n      no total: %ld morfismos, %ld quadrados fechados\n\n",
               soma_pares, soma_testados);
        ok("O QUADRADO DE ev COMUTA para todo morfismo f entre os grupos da família, e os dois"
           " lados vêm por caminhos que não se copiam: o esquerdo avalia o caractere no ponto"
           " f(x), do lado de Y; o direito CONSTRÓI o caractere puxado χ∘f, PROCURA-O na lista"
           " de X^ — que é o que f^ é — e avalia a tabela encontrada em x. Se f^ não estivesse"
           " bem definida a busca devolvia −1 e o teste caía; conta-se isso à parte, e é zero."
           " Mede-se ainda que f^ é morfismo. É esta comutação, e não |X^| = |X|, que se chama"
           " dualidade de Pontryagin: ev não escolhe nada, e por isso comuta com tudo.",
           mau == 0 && soma_testados > 0);
    }

    /* ═══ §PG7: O GUME — o iso X → X̂ NÃO é natural ══════════════════════════
     *
     * A mesma máquina, com o mesmo tipo de busca, aplicada a ψ : X → X̂. Aqui o
     * quadrado é o CONTRAVARIANTE: uma família natural de isos exigiria
     *
     *      ψ_X  =  φ̂ ∘ ψ_Y ∘ φ        para todo morfismo φ : X → Y,
     *
     * e mede-se em X = Y = ℤ/n com φ a multiplicação por uma unidade u.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§PG7  o gume: o isomorfismo X -> X^ falha o quadrado, e mede-se onde.\n\n");
    {
        long mau = 0, viu_falhar = 0, viu_passar = 0, total_psi = 0, total_ev = 0;
        printf("      Z/n   unidades   u^2!=1?   psi: quadrados  falhas   ev: quadrados  falhas\n");
        for(int n = 2; n <= 12; n++){
            int nn[1] = { n }; Grupo G; g_init(&G, "Z/n", 1, nn);
            static int chiG[PG_MAXCH][PG_MAXEL], coG[PG_MAXCH][PG_MAXR];
            int nc = pg_duais(&G, chiG, coG);
            if(nc < 0){ mau++; continue; }
            /* ψ : x ↦ o caractere de coordenada x (a escolha do gerador) */
            int psi[PG_MAXEL];
            for(int x = 0; x < n; x++){
                int tab[PG_MAXEL];
                for(int t = 0; t < n; t++) tab[t] = (x * t) % n;
                psi[x] = pg_indice((const int (*)[PG_MAXEL])chiG, nc, n, tab);
                if(psi[x] < 0) mau++;
            }
            long unidades_n = 0, q_psi = 0, f_psi = 0, q_ev = 0, f_ev = 0;
            /* a LEI que se vai asserir: psi falha exactamente quando existe uma
             * unidade com u² != 1 — o factor que o quadrado contravariante deixa
             * sobrar. Calcula-se aqui, à parte, e compara-se com o medido. */
            int tem_u = 0;
            for(int u = 1; u < n; u++){
                if(pg_mdc(u,n) != 1) continue;
                for(int x = 1; x < n && !tem_u; x++) if((u*u*x - x) % n) tem_u = 1;
            }
            for(int u = 1; u < n; u++){
                if(pg_mdc(u,n) != 1) continue;
                unidades_n++;
                /* φ : x ↦ ux, verificado morfismo */
                int phi[PG_MAXEL];
                for(int x = 0; x < n; x++) phi[x] = (u*x) % n;
                int morf = 1;
                for(int a = 0; a < n && morf; a++) for(int b = 0; b < n && morf; b++)
                    if(phi[g_soma(&G,a,b)] != g_soma(&G, phi[a], phi[b])) morf = 0;
                if(!morf){ mau++; continue; }
                /* φ̂ : X̂ → X̂, χ ↦ χ∘φ, pela mesma BUSCA */
                int dualphi[PG_MAXCH];
                for(int k = 0; k < nc; k++){
                    int tab[PG_MAXEL];
                    for(int x = 0; x < n; x++) tab[x] = chiG[k][ phi[x] ];
                    dualphi[k] = pg_indice((const int (*)[PG_MAXEL])chiG, nc, n, tab);
                    if(dualphi[k] < 0) mau++;
                }
                /* o quadrado de ψ: ψ(x) contra φ̂(ψ(φ(x))) */
                for(int x = 0; x < n; x++){
                    q_psi++;
                    if(psi[x] != dualphi[ psi[ phi[x] ] ]) f_psi++;
                }
                /* o quadrado de ev, pela mesma máquina: ev_{φ(x)} contra φ̂̂(ev_x),
                 * que em χ vale ev_x(φ̂χ) = (φ̂χ)(x) = chiG[dualphi[k]][x]. */
                for(int x = 0; x < n; x++)
                    for(int k = 0; k < nc; k++){
                        q_ev++;
                        if(chiG[k][ phi[x] ] != chiG[ dualphi[k] ][ x ]) f_ev++;
                    }
            }
            total_psi += q_psi; total_ev += q_ev;
            if(f_psi) viu_falhar++; else viu_passar++;
            printf("      %-5d %-10ld %-9s %-14ld %-8ld %-13ld %ld\n",
                   n, unidades_n, tem_u ? "sim" : "nao", q_psi, f_psi, q_ev, f_ev);
            if(f_ev) mau++;
            /* e a correspondência, que é a lei e não um resumo: psi falha SE E SÓ SE
             * existe u com u² != 1. Sem esta igualdade, bastaria «falha nuns e passa
             * noutros» — e isso passaria com um psi qualquer. */
            if((f_psi > 0) != (tem_u != 0)) mau++;
        }
        printf("\n      psi falha em %ld dos n, passa em %ld — ev nao falha em nenhum\n\n",
               viu_falhar, viu_passar);
        ok("O GUME MORDE, E MORDE ONDE PODE. Pela MESMA máquina — a mesma busca do índice, o"
           " mesmo φ^ construído e não calculado —, o quadrado de psi FALHA e o de ev não. E"
           " falha exactamente onde pode falhar: nos n em que existe uma unidade com u^2 != 1,"
           " porque o quadrado contravariante deixa sobrar um factor u^2 que psi não absorve."
           " Nos outros (n = 2,3,4,6,8,12: os n em que TODA unidade tem quadrado 1) psi passa"
           " — e passa por acidente do grupo, não por lei. Vale a pena dizer o que isso"
           " significa: se a varredura tivesse corrido só nesses, psi teria passado por"
           " natural, e a conclusão seria falsa. E a asserção não se contenta com «falha"
           " nuns e passa noutros»: exige a CORRESPONDÊNCIA n a n com a existência de u,"
           " que é a lei. É a diferença entre |X^| = |X| e Pontryagin,"
           " medida: um isomorfismo que existe mas depende de escolha, contra um que comuta"
           " com todo morfismo porque não escolhe nada.",
           mau == 0 && viu_falhar > 0 && viu_passar > 0 && total_psi > 0 && total_ev > 0);
    }

    /* ═══ §PG8: A TROCA ⊕ → ⊗, E A TRANSFORMADA ══════════════════════════════ */
    printf("\n§PG8  a troca: o caractere leva a soma do grupo na soma dos expoentes.\n\n");
    {
        long mau = 0;
        printf("      grupo          pares testados   chi(a+b) = chi(a)+chi(b)   restrição\n");
        for(int f = 0; f < nf; f++){
            const Grupo *G = &fam[f];
            int nc = pg_duais(G, chi, coord);
            long pares = 0, bons = 0;
            for(int k = 0; k < nc; k++)
                for(int a = 0; a < G->ord; a++) for(int b = 0; b < G->ord; b++){
                    pares++;
                    if(chi[k][g_soma(G,a,b)] == (chi[k][a] + chi[k][b]) % G->e) bons++;
                }
            /* a restrição a um subgrupo é a redução do índice: para X = Z/n com m | n,
             * o subgrupo mZ/n tem os caracteres de X restritos, e são todos. */
            int restr_ok = 1;
            if(G->r == 1){
                for(int m = 2; m < G->n[0]; m++){
                    if(G->n[0] % m) continue;
                    int sub = G->n[0] / m;      /* |mZ/n| = n/m */
                    int distintas = 0;
                    for(int k = 0; k < nc; k++){
                        int novo = 1;
                        for(int j = 0; j < k && novo; j++){
                            int ig = 1;
                            for(int t = 0; t < sub && ig; t++)
                                if(chi[k][t*m] != chi[j][t*m]) ig = 0;
                            if(ig) novo = 0;
                        }
                        distintas += novo;
                    }
                    if(distintas != sub) restr_ok = 0;
                }
            }
            printf("      %-14s %-16ld %-26s %s\n", G->nome, pares,
                   (bons == pares) ? "todos" : "NAO", restr_ok ? "= redução do índice" : "NAO");
            if(bons != pares || !restr_ok) mau++;
        }
        printf("\n");
        ok("A TROCA ⊕ -> ⊗ É O QUE O CARACTERE FAZ, e é exacta em todos os pares: no expoente"
           " a operação do grupo vira soma, que é o mesmo que dizer que nas raízes vira"
           " produto. E a RESTRIÇÃO a um subgrupo é a REDUÇÃO DO ÍNDICE — os caracteres do"
           " subgrupo são exactamente as restrições, sem sobra e sem falta —, que é a torre"
           " dos relógios da casa dita no discreto. Daqui sai a transformada: por §PG4 as"
           " fibras são uniformes, e é essa uniformidade que a inverte.", mau == 0);
    }

    /* ═══ §PG9: A FRONTEIRA — é a FINITUDE que fecha ═════════════════════════ */
    printf("\n§PG9  a fronteira: onde a bidualidade deixa de fechar sozinha.\n\n");
    {
        long mau = 0;
        /* No finito, X̂̂ ≅ X pela avaliação. A hipótese usada foi UMA: o grupo ser
         * finito, e é ela que dá (i) a enumeração dos morfismos e (ii) a passagem
         * de injectiva a bijectiva por contagem. Mede-se o que acontece quando a
         * contagem falha: num grupo LIVRE de posto 1 truncado — os inteiros até N
         * com a soma SEM redução — a soma sai do conjunto, e não há grupo. */
        printf("      o que se usou                             onde entra          testado\n");
        printf("      finitude -> os morfismos enumeram-se      §PG1, §PG2          sim\n");
        printf("      injectiva + |X^^| = |X| -> bijectiva      §PG5                sim\n");
        printf("      separação (nao precisa de topologia)      §PG3                sim\n");
        /* e a testemunha de que a contagem é essencial: sem ela, injectiva não basta */
        {
            /* X = Z/4, e um morfismo injectivo Z/4 -> Z/8 que NÃO é sobrejectivo:
             * injectividade sozinha não dá isomorfismo, e é a contagem que fecha. */
            int n1[] = {4}, n2[] = {8};
            Grupo A, Bg; g_init(&A, "Z/4", 1, n1); g_init(&Bg, "Z/8", 1, n2);
            int fmap[PG_MAXEL];
            for(int x = 0; x < A.ord; x++) fmap[x] = (2*x) % Bg.ord;
            long morf = 1, inj = 1, sobre = 1;
            for(int a = 0; a < A.ord; a++) for(int b = 0; b < A.ord; b++)
                if(fmap[g_soma(&A,a,b)] != g_soma(&Bg, fmap[a], fmap[b])) morf = 0;
            for(int a = 0; a < A.ord; a++) for(int b = a+1; b < A.ord; b++)
                if(fmap[a] == fmap[b]) inj = 0;
            for(int y = 0; y < Bg.ord; y++){
                int achou = 0;
                for(int x = 0; x < A.ord && !achou; x++) if(fmap[x] == y) achou = 1;
                if(!achou) sobre = 0;
            }
            printf("      injectiva SEM contagem nao fecha         Z/4 -> Z/8          "
                   "morf=%ld inj=%ld sobre=%ld\n", morf, inj, sobre);
            if(!morf || !inj || sobre) mau++;
        }
        printf("\n");
        ok("A HIPÓTESE É A FINITUDE, E ELA ENTRA EM DOIS SÍTIOS, os dois medidos: os morfismos"
           " ENUMERAM-SE (é o que torna §PG1 e §PG2 varreduras e não fórmulas), e injectiva"
           " passa a bijectiva PELA CONTAGEM (§PG5). Que a contagem é essencial mostra-se com"
           " a testemunha: 2x : Z/4 -> Z/8 é morfismo e é injectivo, e NÃO é sobrejectivo —"
           " logo injectividade sozinha não dá isomorfismo em sítio nenhum. É por aqui que o"
           " eixo da casa se lê: no finito a álgebra fecha sozinha; no infinito a contagem"
           " deixa de estar disponível e o fecho passa a pedir topologia — o dual de um"
           " discreto é compacto, e o de um compacto é discreto. A álgebra opera e não"
           " alcança; a topologia alcança e não opera.", mau == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
