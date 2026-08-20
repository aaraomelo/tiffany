/* tensor.h — GRAM, SYLVESTER, JORDAN, TENSOR E EXTERIOR.
 *
 *  ordem do coordenador sobe a álgebra linear até à geometria, e traz três coisas que mudam o
 * modo de medir — não só o conteúdo:
 *
 * 1. «NÃO BASTA OLHAR PARA A MATRIZ; é preciso PROCURAR TESTEMUNHAS v com Q(v) > 0,
 *    Q(v) < 0 ou Q(v) = 0.» A assinatura não se lê: caça-se.
 *
 * 2. «Encontrar um contra-exemplo quando a característica é 2.» A polarização
 *    B(u,v) = (Q(u+v) − Q(u) − Q(v))/2 precisa do 1/2 — e em 𝔽₂ ele NÃO EXISTE. É a
 *    fibra vazia outra vez, agora a derrubar uma identidade.
 *
 * 3. E a correção que ele faz ao que já cá estava: ker T* = (im T)° «já apareceu no teu
 *    corpus, mas agora dá para provar ESTRUTURALMENTE, SEM VARREDURA» — cada seta com a
 *    sua lei. Eu tinha-o medido varrendo 625 matrizes; a cadeia mede-se ELO A ELO.
 *
 * E o determinante deixa de ser receita: Λⁿ T é a multiplicação por det T — «a ação do
 * operador no VOLUME ORIENTADO». Aqui isso não se afirma: constrói-se a ação e vê-se
 * que o fator É o determinante.
 *
 * Precisa de `racionais.h`, `linear.h`, `forma.h`. */
#ifndef TENSOR_H
#define TENSOR_H

/* ── A MATRIZ DE GRAM E A CONGRUÊNCIA ──────────────────────────────────────────
 * Mudar de base leva G em PᵀGP — e é isso, e não a semelhança P⁻¹AP, que preserva a
 * FORMA. As duas relações são diferentes, e confundi-las é o erro clássico. */
static Mat gr_congruente(Mat G, Mat P){
    return mat_mult(mat_transposta(P), mat_mult(G, P));
}
static Mat gr_semelhante(Mat A, Mat P){
    Mat Pi;
    if(!mat_inversa(P, &Pi)) return mat0(A.m, A.n);
    return mat_mult(Pi, mat_mult(A, P));
}
/* ── A ASSINATURA, CAÇADA ──────────────────────────────────────────────────────
 * Devolve quantas testemunhas de cada sinal se acharam no cubo, e o cubo DIZ-SE. Não é
 * a matriz que se lê: são os v que se procuram. */
typedef struct { long pos, neg, zero; Vec vp, vn, vz; } Assin;
static Assin gr_assinatura(Mat G, long lim){
    Assin a; a.pos = a.neg = a.zero = 0;
    a.vp = a.vn = a.vz = vec0(G.n);
    long lado = 2*lim + 1, total = 1;
    for(int i = 0; i < G.n; i++) total *= lado;
    for(long k = 0; k < total; k++){
        Vec v = vec0(G.n);
        long t = k;
        for(int i = 0; i < G.n; i++){ v.c[i] = qz_de_inteiro(t % lado - lim); t /= lado; }
        if(vec_zero(v)) continue;
        Qz q = fb_quadratica(G, v);
        if(q.p > 0){ if(!a.pos) a.vp = v; a.pos++; }
        else if(q.p < 0){ if(!a.neg) a.vn = v; a.neg++; }
        else { if(!a.zero) a.vz = v; a.zero++; }
    }
    return a;
}
/* ── A POLARIZAÇÃO, E ONDE ELA MORRE ───────────────────────────────────────────
 * B(u,v) = (Q(u+v) − Q(u) − Q(v))/2, válida quando 2 é invertível. Sobre ℚ é exata. */
static int gr_polariza(Mat G, Vec u, Vec v, Qz *saida){
    Qz s = qz_soma(fb_quadratica(G, vec_soma(u,v)),
                   qz_oposto(qz_soma(fb_quadratica(G,u), fb_quadratica(G,v))));
    Qz meio;
    if(!qz_divide(s, qz_de_inteiro(2), &meio)) return 0;   /* char 2: sem fibra */
    *saida = meio;
    return 1;
}
/* ── O POLINÓMIO MÍNIMO (2×2) ──────────────────────────────────────────────────
 * Grau 1 quando A é escalar (A = λI, e então A − λI = 0); grau 2 caso contrário, e aí
 * coincide com o característico. É a distinção que  ordem do coordenador pede a seguir ao
 * Cayley–Hamilton — o mínimo pode ser MENOR que o característico. */
static int esp_minimo_grau(Mat A){
    Qz d = A.a[0][0];
    if(qz_igual(A.a[1][1], d) && A.a[0][1].p == 0 && A.a[1][0].p == 0) return 1;
    return 2;
}
/* ── SIMILARIDADE: o que ela preserva, e o que NÃO determina ───────────────────
 * Preserva traço, determinante, característico e espectro. Mas o espectro NÃO a
 * determina — e é esse o gume que ele pede ao buscador. */
static int sim_mesmo_espectro(Mat A, Mat B){
    return qz_igual(esp_tr(A), esp_tr(B)) && qz_igual(mat_det(A), mat_det(B));
}
/* similares ⟹ mesmo número de autovetores independentes; usa-se o contrapositivo */
static int sim_podem_ser_similares(Mat A, Mat B){
    Vec va[LN_MAX], vb[LN_MAX];
    if(!sim_mesmo_espectro(A,B)) return 0;
    return esp_autovetores(A,va) == esp_autovetores(B,vb);
}
/* ── O PRODUTO TENSORIAL, realizado pelo produto de KRONECKER ──────────────────
 * dim(V⊗W) = dim V · dim W, e a propriedade universal: uma B bilinear V×W→K é
 * exatamente uma linear em V⊗W. Em coordenadas, a B tem mn coeficientes — que são as
 * coordenadas do funcional no tensorial. É a mesma tabela, lida de dois modos. */
static Mat tn_kron(Mat A, Mat B){
    Mat R = mat0(A.m * B.m, A.n * B.n);
    if(R.m > LN_MAX || R.n > LN_MAX){ R.m = 0; R.n = 0; return R; }
    for(int i = 0; i < A.m; i++) for(int j = 0; j < A.n; j++)
        for(int k = 0; k < B.m; k++) for(int l = 0; l < B.n; l++)
            R.a[i*B.m + k][j*B.n + l] = qz_mult(A.a[i][j], B.a[k][l]);
    return R;
}
/* o tensor SIMPLES u⊗v, achatado no índice (i,j) ↦ i·n + j */
static Vec tn_simples(Vec u, Vec v){
    Vec r = vec0(u.n * v.n);
    for(int i = 0; i < u.n; i++) for(int j = 0; j < v.n; j++)
        r.c[i*v.n + j] = qz_mult(u.c[i], v.c[j]);
    return r;
}
/* a linear induzida: B̃(u⊗v) = B(u,v) — os coeficientes de B lidos como funcional */
static Qz tn_induzida(Mat B, Vec t){
    Qz s = qz(0,1);
    for(int i = 0; i < B.m; i++) for(int j = 0; j < B.n; j++)
        s = qz_soma(s, qz_mult(B.a[i][j], t.c[i*B.n + j]));
    return s;
}
/* ── A ÁLGEBRA EXTERIOR: Λ²(ℚ²) tem dimensão 1, e o fator É o determinante ─────
 * u∧v em dimensão 2 é o escalar u₁v₂ − u₂v₁. A ação de T em Λ² é Tu ∧ Tv, e o teorema
 * diz que ela é a multiplicação por det T — «o determinante é a ação do operador no
 * VOLUME ORIENTADO», e não uma receita de cofatores. Aqui constrói-se e vê-se. */
static Qz ex_cunha2(Vec u, Vec v){
    return qz_soma(qz_mult(u.c[0], v.c[1]), qz_oposto(qz_mult(u.c[1], v.c[0])));
}
/* a ALTERNÂNCIA: u∧u = 0, e u∧v = −(v∧u) — as duas medem-se */
static int ex_alterna(Vec u, Vec v){
    return ex_cunha2(u,u).p == 0
        && qz_igual(ex_cunha2(u,v), qz_oposto(ex_cunha2(v,u)));
}
#endif
