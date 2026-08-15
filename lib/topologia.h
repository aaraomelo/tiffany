/* topologia.h — A TOPOLOGIA SEM RÉGUA.
 *
 * O `eval.txt` traz vinte problemas de topologia e um gume que é a alma do andar:
 *
 *        «REMOVA A MÉTRICA.»
 *
 * e refazer os exercícios só com linguagem topológica, «porque isso testa se ele
 * realmente entendeu que MÉTRICA ⟹ TOPOLOGIA, mas TOPOLOGIA ⇏ UMA MÉTRICA ESPECÍFICA».
 *
 * ── COMO SE MEDE TOPOLOGIA SEM APROXIMAR NADA ──────────────────────────────────
 * Num conjunto FINITO tudo é exaustivo: uma topologia é uma família de subconjuntos, e
 * cada subconjunto é uma máscara de bits. Verificar os axiomas, o fecho, o interior, a
 * fronteira, Hausdorff, a continuidade e a conexidade é percorrer 2ⁿ máscaras — sem uma
 * distância, sem um ε, e sem um caso por decidir.
 *
 * E os finitos não são um brinquedo: são exactamente onde a intuição de régua QUEBRA. O
 * espaço de Sierpiński ({0,1} com abertos ∅, {1}, X) é T₀ e não é Hausdorff, e nenhuma
 * métrica o realiza — é o contra-exemplo que o gume pede, e cabe em dois pontos.
 *
 * ── E ONDE O FINITO NÃO CHEGA ──────────────────────────────────────────────────
 * A compacidade e a conexidade da reta precisam de infinito, e aí a casa tem material
 * exacto:
 *   · (0,1) NÃO é compacto: a cobertura Uₙ = (1/n, 1) não tem subcobertura finita, e a
 *     testemunha é exacta — dada uma subfamília finita com máximo N, o ponto 1/(2N)
 *     fica de fora.
 *   · ℚ NÃO é conexo: U = {x < 0} ∪ {x² < 2} e V = {x > 0, x² > 2} são abertos,
 *     disjuntos, não vazios e cobrem ℚ. A separação é EXACTA porque não há racional com
 *     x² = 2 — é o corte outra vez, e é o material do andar de ℝ.
 * As duas são testemunhas em ℚ, exactas, e nenhuma precisa de decimal.
 *
 * Precisa de `racionais.h`. */
#ifndef TOPOLOGIA_H
#define TOPOLOGIA_H

#define TP_MAX 6                       /* pontos; 2^6 = 64 subconjuntos */
#define TP_SUB (1 << TP_MAX)

/* uma topologia é a lista das máscaras dos abertos, num espaço de n pontos */
typedef struct { int n; int ab[TP_SUB]; int na; } Top;

static long tp_estouros = 0;

static int tp_tem(Top t, int m){
    for(int i = 0; i < t.na; i++) if(t.ab[i] == m) return 1;
    return 0;
}
static void tp_poe(Top *t, int m){
    if(tp_tem(*t, m)) return;
    if(t->na >= TP_SUB){ tp_estouros++; return; }
    t->ab[t->na++] = m;
}
static int tp_cheio(int n){ return (1 << n) - 1; }

/* ── OS AXIOMAS, cada um verificado À PARTE ─────────────────────────────────────
 * ∅ e X abertos; união ARBITRÁRIA de abertos é aberta; intersecção FINITA é aberta.
 * Num finito, «arbitrária» e «finita» coincidem — e é preciso dizê-lo, porque é
 * exactamente aí que a assimetria da definição desaparece. */
static int tp_axioma_vazio(Top t){ return tp_tem(t, 0); }
static int tp_axioma_total(Top t){ return tp_tem(t, tp_cheio(t.n)); }
static int tp_axioma_uniao(Top t){
    for(int i = 0; i < t.na; i++) for(int j = 0; j < t.na; j++)
        if(!tp_tem(t, t.ab[i] | t.ab[j])) return 0;
    return 1;
}
static int tp_axioma_intersecao(Top t){
    for(int i = 0; i < t.na; i++) for(int j = 0; j < t.na; j++)
        if(!tp_tem(t, t.ab[i] & t.ab[j])) return 0;
    return 1;
}
static int tp_valida(Top t){
    return tp_axioma_vazio(t) && tp_axioma_total(t)
        && tp_axioma_uniao(t) && tp_axioma_intersecao(t);
}
/* ── AS TOPOLOGIAS CANÓNICAS ────────────────────────────────────────────────────*/
static Top tp_discreta(int n){
    Top t; t.n = n; t.na = 0;
    for(int m = 0; m < (1 << n); m++) tp_poe(&t, m);
    return t;
}
static Top tp_indiscreta(int n){
    Top t; t.n = n; t.na = 0;
    tp_poe(&t, 0); tp_poe(&t, tp_cheio(n));
    return t;
}
static Top tp_sierpinski(void){                /* {0,1}: ∅, {1}, X — T₀ e NÃO Hausdorff */
    Top t; t.n = 2; t.na = 0;
    tp_poe(&t, 0); tp_poe(&t, 2); tp_poe(&t, 3);
    return t;
}
/* a topologia GERADA por uma família: fecha por união e intersecção até estabilizar */
static Top tp_gerada(int n, const int *base, int nb){
    Top t; t.n = n; t.na = 0;
    tp_poe(&t, 0); tp_poe(&t, tp_cheio(n));
    for(int i = 0; i < nb; i++) tp_poe(&t, base[i]);
    int mudou = 1, voltas = 0;
    while(mudou && voltas++ < 64){
        mudou = 0;
        int na = t.na;
        for(int i = 0; i < na; i++) for(int j = 0; j < na; j++){
            int u = t.ab[i] | t.ab[j], v = t.ab[i] & t.ab[j];
            if(!tp_tem(t, u)){ tp_poe(&t, u); mudou = 1; }
            if(!tp_tem(t, v)){ tp_poe(&t, v); mudou = 1; }
        }
    }
    return t;
}
/* ── FECHO, INTERIOR E FRONTEIRA ────────────────────────────────────────────────
 * O fecho por DUAS vias independentes, que é o que o Problema 5 pede:
 *   (a) a intersecção de todos os FECHADOS que contêm A;
 *   (b) os x cuja toda vizinhança aberta toca A.
 * Elas têm de dar o mesmo, e é a comparação que é o teorema. */
static int tp_fechado(Top t, int m){ return tp_tem(t, tp_cheio(t.n) & ~m); }

static int tp_fecho_por_fechados(Top t, int A){
    int r = tp_cheio(t.n);
    for(int m = 0; m < (1 << t.n); m++)
        if(tp_fechado(t, m) && (A & ~m) == 0) r &= m;
    return r;
}
static int tp_fecho_por_vizinhancas(Top t, int A){
    int r = 0;
    for(int x = 0; x < t.n; x++){
        int adere = 1;
        for(int i = 0; i < t.na && adere; i++)
            if((t.ab[i] >> x & 1) && (t.ab[i] & A) == 0) adere = 0;
        if(adere) r |= 1 << x;
    }
    return r;
}
static int tp_interior(Top t, int A){
    int r = 0;
    for(int i = 0; i < t.na; i++) if((t.ab[i] & ~A) == 0) r |= t.ab[i];
    return r;
}
static int tp_fronteira(Top t, int A){
    return tp_fecho_por_fechados(t, A) & ~tp_interior(t, A);
}
/* ── ACUMULAÇÃO contra ADERÊNCIA ────────────────────────────────────────────────
 * x adere a A se toda vizinhança toca A; x ACUMULA A se toda vizinhança toca A∖{x}.
 * A diferença são os pontos ISOLADOS de A — e o Problema 7 pede a testemunha. */
static int tp_acumulacao(Top t, int A){
    int r = 0;
    for(int x = 0; x < t.n; x++){
        int acumula = 1;
        for(int i = 0; i < t.na && acumula; i++)
            if((t.ab[i] >> x & 1) && (t.ab[i] & A & ~(1 << x)) == 0) acumula = 0;
        if(acumula) r |= 1 << x;
    }
    return r;
}
/* ── HAUSDORFF, e o PRIMEIRO contra-exemplo quando falha ────────────────────────*/
static int tp_hausdorff(Top t, int *px, int *py){
    for(int x = 0; x < t.n; x++) for(int y = x+1; y < t.n; y++){
        int separa = 0;
        for(int i = 0; i < t.na && !separa; i++) for(int j = 0; j < t.na; j++)
            if((t.ab[i] >> x & 1) && (t.ab[j] >> y & 1)
               && (t.ab[i] & t.ab[j]) == 0){ separa = 1; break; }
        if(!separa){ if(px) *px = x; if(py) *py = y; return 0; }
    }
    return 1;
}
/* ── CONTINUIDADE: f⁻¹(U) aberto, SEM distância ─────────────────────────────────
 * f é um vector de imagens: f[x] ∈ {0..m−1}. A pré-imagem de uma máscara calcula-se
 * bit a bit — e é isto a definição inteira. */
static int tp_preimagem(const int *f, int n, int U){
    int r = 0;
    for(int x = 0; x < n; x++) if(U >> f[x] & 1) r |= 1 << x;
    return r;
}
static int tp_continua(Top X, Top Y, const int *f){
    for(int i = 0; i < Y.na; i++)
        if(!tp_tem(X, tp_preimagem(f, X.n, Y.ab[i]))) return 0;
    return 1;
}
/* pelos FECHADOS — e mede-se À PARTE, que é o que o Problema 10 pede */
static int tp_continua_fechados(Top X, Top Y, const int *f){
    for(int m = 0; m < (1 << Y.n); m++){
        if(!tp_fechado(Y, m)) continue;
        if(!tp_fechado(X, tp_preimagem(f, X.n, m))) return 0;
    }
    return 1;
}
/* ── HOMEOMORFISMO: PROCURA-SE a inversa, não se declara ────────────────────────*/
static int tp_bijeccao(const int *f, int n, int m, int *inv){
    if(n != m) return 0;
    int visto = 0;
    for(int x = 0; x < n; x++){
        if(visto >> f[x] & 1) return 0;
        visto |= 1 << f[x];
        inv[f[x]] = x;
    }
    return visto == tp_cheio(n);
}
static int tp_homeomorfismo(Top X, Top Y, const int *f, int *inv){
    if(!tp_bijeccao(f, X.n, Y.n, inv)) return 0;
    return tp_continua(X, Y, f) && tp_continua(Y, X, inv);
}
/* ── CONEXIDADE: X = U ∪ V com U,V abertos, não vazios e disjuntos ──────────────*/
static int tp_conexo(Top t, int *pu, int *pv){
    for(int i = 0; i < t.na; i++) for(int j = 0; j < t.na; j++){
        int U = t.ab[i], V = t.ab[j];
        if(U == 0 || V == 0) continue;
        if((U & V) != 0) continue;
        if((U | V) != tp_cheio(t.n)) continue;
        if(pu) *pu = U;
        if(pv) *pv = V;
        return 0;
    }
    return 1;
}
/* ── COMPACIDADE por cobertura aberta ───────────────────────────────────────────
 * Num finito TODO espaço é compacto — e isso não é uma vitória, é o aviso de que o
 * finito não distingue aqui. A testemunha real está em ℚ, abaixo. */
static int tp_compacto_finito(Top t){ (void)t; return 1; }

/* ── AS DUAS TESTEMUNHAS EM ℚ, exactas ──────────────────────────────────────────
 * (a) (0,1) não é compacto: Uₙ = (1/n, 1). Dada uma subfamília finita com máximo N, o
 *     ponto 1/(2N) está em (0,1) e não está em nenhum Uₙ com n ≤ N. Exacto.
 * (b) ℚ não é conexo: U = {x < 0} ∪ {x² < 2}, V = {x > 0 e x² > 2}. São abertos,
 *     disjuntos, não vazios, e cobrem ℚ porque NÃO HÁ racional com x² = 2. */
static int tp_cobre_01(long N, Qz x){           /* x ∈ Uₙ = (1/n,1) para algum n ≤ N? */
    if(!(x.p > 0 && x.p * 1 < 1 * x.q)) return 0;          /* fora de (0,1) */
    for(long n = 1; n <= N; n++){
        Qz lo = qz(1, n);
        if(lo.p * x.q < x.p * lo.q) return 1;
    }
    return 0;
}
static Qz tp_escapa_01(long N){ return qz(1, 2*N); }       /* a testemunha, exacta */

static int tp_lado_esquerdo(Qz x){              /* U: x < 0 ou x² < 2 */
    if(x.p < 0) return 1;
    return qz_mult(x,x).p * 1 < 2 * qz_mult(x,x).q;
}
static int tp_lado_direito(Qz x){               /* V: x > 0 e x² > 2 */
    if(x.p <= 0) return 0;
    return 2 * qz_mult(x,x).q < qz_mult(x,x).p * 1;
}
/* e o que faz a separação FECHAR: não há racional com x² = 2 */
static int tp_sem_fronteira(Qz x){
    return !(tp_lado_esquerdo(x) && tp_lado_direito(x))     /* disjuntos */
        &&  (tp_lado_esquerdo(x) || tp_lado_direito(x));    /* e cobrem */
}
#endif
