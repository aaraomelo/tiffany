/* homotopia.h — O BURACO QUE NENHUM PONTO VÊ.
 *
 *  ordem do coordenador: «o círculo força a distinguir LOCALMENTE IGUAL ≠ GLOBALMENTE IGUAL. A
 * reta pode ser contraída a um ponto; o círculo não. Aí vamos descobrir se ele consegue
 * capturar um BURACO QUE NÃO É VISÍVEL POR NENHUM PONTO ISOLADAMENTE.»
 *
 * ── COMO SE MEDE HOMOTOPIA SEM UM DECIMAL ──────────────────────────────────────
 * Combinatoriamente. Um grafo conexo tem π₁ LIVRE de posto E − V + 1, e isso é aritmética
 * de inteiros. O círculo é o ciclo Cₙ (posto 1), a reta é uma árvore (posto 0), e o oito
 * é dois ciclos colados (posto 2, e NÃO abeliano).
 *
 * E o número de VOLTAS de um laço é um inteiro: percorre-se o laço somando +1 nas arestas
 * no sentido positivo e −1 nas outras, e divide-se pelo comprimento do ciclo. Exacto.
 *
 * ── E O BURACO QUE NENHUM PONTO VÊ, que é o pedido ─────────────────────────────
 * Em Cₙ toda VIZINHANÇA de um vértice é um caminho — uma árvore, posto 0, contráctil. Logo
 * NENHUM teste local distingue o círculo da reta: ponto a ponto eles são iguais. E o todo
 * tem posto 1. A diferença não vive em ponto nenhum; vive na maneira como os pontos estão
 * COLADOS.
 *
 * Isso mede-se: percorrem-se TODOS os vértices, verifica-se que a vizinhança de cada um é
 * contráctil, e depois verifica-se que o todo não é. As duas coisas ao mesmo tempo são o
 * teorema.
 *
 * ── E O NÃO ABELIANO ──────────────────────────────────────────────────────────
 * No oito, π₁ é livre de posto 2, e aí ab ≠ ba — como PALAVRAS REDUZIDAS. É o primeiro
 * sítio desta casa onde a ordem importa e não há matriz por trás: a não comutatividade é
 * do grupo, não de uma representação.
 *
 * Precisa de `racionais.h`. Tudo em inteiros. */
#ifndef HOMOTOPIA_H
#define HOMOTOPIA_H

#define HT_V 12                        /* vértices */
#define HT_E 24                        /* arestas */
#define HT_W 32                        /* comprimento máximo de uma palavra */

typedef struct { int nv, ne; int a[HT_E], b[HT_E]; } Grafo;   /* arestas a—b */

static long ht_estouros = 0;

static Grafo gr0(int nv){ Grafo g; g.nv = nv; g.ne = 0; return g; }
static void gr_liga(Grafo *g, int a, int b){
    if(g->ne >= HT_E){ ht_estouros++; return; }
    g->a[g->ne] = a; g->b[g->ne] = b; g->ne++;
}
/* o CICLO Cₙ — o círculo */
static Grafo gr_ciclo(int n){
    Grafo g = gr0(n);
    for(int i = 0; i < n; i++) gr_liga(&g, i, (i+1) % n);
    return g;
}
/* o CAMINHO Pₙ — a reta */
static Grafo gr_caminho(int n){
    Grafo g = gr0(n);
    for(int i = 0; i + 1 < n; i++) gr_liga(&g, i, i+1);
    return g;
}
/* o OITO: dois ciclos colados num vértice */
static Grafo gr_oito(int n){
    Grafo g = gr0(2*n - 1);
    for(int i = 0; i < n; i++) gr_liga(&g, i, (i+1) % n);
    for(int i = 0; i < n; i++){
        int u = (i == 0) ? 0 : n + i - 1;
        int v = (i == n-1) ? 0 : n + i;
        gr_liga(&g, u, v);
    }
    return g;
}
/* ── A CONEXIDADE, e as COMPONENTES — por união-busca, exacta ───────────────────*/
static int ht_raiz(int *p, int x){ while(p[x] != x) x = p[x] = p[p[x]]; return x; }
static int gr_componentes(Grafo g){
    int p[HT_V];
    for(int i = 0; i < g.nv; i++) p[i] = i;
    for(int e = 0; e < g.ne; e++){
        int ra = ht_raiz(p, g.a[e]), rb = ht_raiz(p, g.b[e]);
        if(ra != rb) p[ra] = rb;
    }
    int c = 0;
    for(int i = 0; i < g.nv; i++) if(ht_raiz(p,i) == i) c++;
    return c;
}
static int gr_conexo(Grafo g){ return gr_componentes(g) == 1; }

/* ── O POSTO DE π₁: E − V + C, e é aritmética de inteiros ───────────────────────
 * Para um grafo conexo, π₁ é LIVRE de posto E − V + 1 — o número de arestas que sobram
 * depois de escolher uma árvore geradora. Cada uma dá um laço independente, e não há
 * relações entre eles. Um posto ZERO quer dizer contráctil. */
static int gr_posto(Grafo g){ return g.ne - g.nv + gr_componentes(g); }
static int gr_contractil(Grafo g){ return gr_conexo(g) && gr_posto(g) == 0; }

/* a CARACTERÍSTICA de Euler, χ = V − E — e a relação com o posto */
static int gr_euler(Grafo g){ return g.nv - g.ne; }

/* ── A VIZINHANÇA de um vértice, e é AQUI que o buraco se esconde ───────────────
 * A vizinhança de v é o subgrafo dos vizinhos de v com as arestas que os ligam a v (a
 * estrela). Num ciclo ela é sempre um caminho de três vértices — uma árvore, contráctil.
 * Percorrer TODOS os vértices e ver que todas são contrácteis é metade do teorema; a
 * outra metade é o todo não ser. */
static Grafo gr_estrela(Grafo g, int v){
    int mapa[HT_V];
    for(int i = 0; i < HT_V; i++) mapa[i] = -1;
    int nv = 0;
    mapa[v] = nv++;
    for(int e = 0; e < g.ne; e++){
        if(g.a[e] == v && mapa[g.b[e]] < 0) mapa[g.b[e]] = nv++;
        if(g.b[e] == v && mapa[g.a[e]] < 0) mapa[g.a[e]] = nv++;
    }
    Grafo s = gr0(nv);
    for(int e = 0; e < g.ne; e++)
        if(g.a[e] == v || g.b[e] == v) gr_liga(&s, mapa[g.a[e]], mapa[g.b[e]]);
    return s;
}
/* ── O NÚMERO DE VOLTAS num ciclo Cₙ, exacto ───────────────────────────────────
 * Um laço dá-se pela lista de vértices visitados. Cada passo de i para (i+1) mod n conta
 * +1; de i para (i−1) mod n conta −1; e o total, dividido por n, é o número de voltas.
 * Devolve 0 se o percurso não fechar ou não for por arestas. */
static int ht_volta(int n, const int *v, int len, int *voltas){
    if(len < 2 || v[0] != v[len-1]) return 0;
    long s = 0;
    for(int k = 0; k + 1 < len; k++){
        int d = (v[k+1] - v[k] + n) % n;
        if(d == 1) s += 1;
        else if(d == n - 1) s -= 1;
        else return 0;                        /* não é um passo por aresta */
    }
    if(s % n) return 0;                       /* não fechou um número inteiro de voltas */
    *voltas = (int)(s / n);
    return 1;
}
/* ── PALAVRAS NO GRUPO LIVRE, para o OITO ──────────────────────────────────────
 * Geradores 1 = a, −1 = a⁻¹, 2 = b, −2 = b⁻¹. Reduzir é apagar pares xx⁻¹ até nada mudar.
 * Duas palavras são iguais no grupo livre EXACTAMENTE quando as reduzidas coincidem — e
 * é por isso que ab ≠ ba se decide sem representação nenhuma. */
typedef struct { int g[HT_W]; int n; } Pal;

static Pal pal_de(const int *s, int n){
    Pal p; p.n = 0;
    for(int i = 0; i < n && p.n < HT_W; i++) p.g[p.n++] = s[i];
    return p;
}
static Pal pal_reduz(Pal p){
    Pal r; r.n = 0;
    for(int i = 0; i < p.n; i++){
        if(r.n > 0 && r.g[r.n-1] == -p.g[i]) r.n--;
        else if(r.n < HT_W) r.g[r.n++] = p.g[i];
        else ht_estouros++;
    }
    return r;
}
static Pal pal_junta(Pal x, Pal y){
    Pal r; r.n = 0;
    for(int i = 0; i < x.n && r.n < HT_W; i++) r.g[r.n++] = x.g[i];
    for(int i = 0; i < y.n && r.n < HT_W; i++) r.g[r.n++] = y.g[i];
    return pal_reduz(r);
}
static Pal pal_inversa(Pal x){
    Pal r; r.n = 0;
    for(int i = x.n - 1; i >= 0; i--) r.g[r.n++] = -x.g[i];
    return r;
}
static int pal_igual(Pal x, Pal y){
    Pal a = pal_reduz(x), b = pal_reduz(y);
    if(a.n != b.n) return 0;
    for(int i = 0; i < a.n; i++) if(a.g[i] != b.g[i]) return 0;
    return 1;
}
static int pal_trivial(Pal x){ return pal_reduz(x).n == 0; }

/* ── A MATRIZ DE INCIDÊNCIA, e a PONTE para a cohomologia ──────────────────────
 * A linha da aresta a→b tem −1 na coluna a e +1 na coluna b. Aplicá-la a f ∈ Λ⁰ dá
 * exactamente (df)(e) = f(b) − f(a): é a derivada exterior de grau 0 escrita num grafo.
 * O posto dela é dim im d, e daí saem as três dimensões do complexo. E a TRANSPOSTA é o
 * ∂ — a adjunção ∂ ⊣ d em coordenadas. */
static Mat gr_incidencia(Grafo g){
    if(g.ne > LN_MAX || g.nv > LN_MAX){ ht_estouros++; return mat0(1,1); }
    Mat M = mat0(g.ne, g.nv);
    for(int e = 0; e < g.ne; e++){
        M.a[e][g.a[e]] = qz(-1,1);
        M.a[e][g.b[e]] = qz(1,1);
    }
    return M;
}
/* a soma de uma cocadeia ao longo do grafo — detecta a não-exactidão nos ciclos */
static Qz gr_soma_cocadeia(Grafo g, const Qz *w){
    Qz s = qz(0,1);
    for(int e = 0; e < g.ne; e++) s = qz_soma(s, w[e]);
    return s;
}
/* ω = df? propaga f pelas arestas e CONFERE em todas — a fibra, e ela pode ser vazia */
static int gr_potencial(Grafo g, const Qz *w, Qz *f){
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
    for(int e = 0; e < g.ne; e++)
        if(!qz_igual(qz_soma(f[g.b[e]], qz_oposto(f[g.a[e]])), w[e])) return 0;
    return 1;
}
/* ── A ABELIANIZAÇÃO: contar os expoentes ──────────────────────────────────────
 * Em ℤ² (o toro) ab = ba; no grupo LIVRE não. A abelianização apaga a ordem e fica só a
 * contagem — e é ela que mostra que a diferença entre os dois é a ORDEM, não os
 * geradores. */
static void pal_abeliana(Pal x, int *na, int *nb){
    *na = 0; *nb = 0;
    for(int i = 0; i < x.n; i++){
        if(x.g[i] == 1) (*na)++;
        else if(x.g[i] == -1) (*na)--;
        else if(x.g[i] == 2) (*nb)++;
        else if(x.g[i] == -2) (*nb)--;
    }
}
#endif
