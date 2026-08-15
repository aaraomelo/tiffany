/* relacao.h — RELAÇÃO, FUNÇÃO, BIJEÇÃO: e a bijeção é a que TEM VOLTA.
 *
 * O `eval.txt` fecha assim, e é o desenho desta casa em miniatura:
 *
 *     A×B → pares → relação → função → bijetividade → inversa → volta
 *
 * «começa nos pontos, forma pares, corta uma relação, seleciona uma função, verifica a
 * propriedade e EXIGE A INVERSA para fechar o circuito». E a frase que liga tudo ao
 * resto: «f bijetiva ⟺ f⁻¹ existe — a bijeção é justamente a função que possui volta».
 *
 * O ficheiro também diz como isto entra no motor: «uma relação vira novamente uma
 * TABELA BOOLEANA sobre os pares». Então é isso: R ⊆ A×A é uma matriz de bits, e cada
 * propriedade é uma varredura EXAUSTIVA sobre ela — n² para as unárias, n³ para a
 * transitividade. Nada de amostra: o conjunto é finito por construção.
 *
 * As cinco operações continuam as mesmas: a composição é o PRODUTO (⊗) de relações, a
 * inversa é o DUAL (†, e (R†)† = R), a identidade é o neutro, e a volta é a INVERSÃO. */
#ifndef RELACAO_H
#define RELACAO_H
#include <string.h>

#define RL_MAX 12                       /* até 12 elementos: 144 pares, e é dito */

typedef struct {
    int n;                              /* o conjunto é {1..n} */
    unsigned char m[RL_MAX][RL_MAX];    /* m[a][b] = 1 sse aRb */
} Rel;

static void rl_zera(Rel *r, int n){
    r->n = n;
    for(int a = 0; a < RL_MAX; a++) for(int b = 0; b < RL_MAX; b++) r->m[a][b] = 0;
}
/* ── as propriedades, cada uma a sua definição, varrida INTEIRA ────────────────── */
static int rl_reflexiva(const Rel *r){                 /* ∀a: aRa */
    for(int a = 0; a < r->n; a++) if(!r->m[a][a]) return 0;
    return 1;
}
static int rl_simetrica(const Rel *r){                 /* aRb ⇒ bRa */
    for(int a = 0; a < r->n; a++) for(int b = 0; b < r->n; b++)
        if(r->m[a][b] && !r->m[b][a]) return 0;
    return 1;
}
static int rl_antissimetrica(const Rel *r){            /* aRb ∧ bRa ⇒ a=b */
    for(int a = 0; a < r->n; a++) for(int b = 0; b < r->n; b++)
        if(a != b && r->m[a][b] && r->m[b][a]) return 0;
    return 1;
}
static int rl_transitiva(const Rel *r){                /* aRb ∧ bRc ⇒ aRc */
    for(int a = 0; a < r->n; a++) for(int b = 0; b < r->n; b++){
        if(!r->m[a][b]) continue;
        for(int c = 0; c < r->n; c++)
            if(r->m[b][c] && !r->m[a][c]) return 0;
    }
    return 1;
}
static int rl_equivalencia(const Rel *r){
    return rl_reflexiva(r) && rl_simetrica(r) && rl_transitiva(r);
}
static int rl_ordem(const Rel *r){
    return rl_reflexiva(r) && rl_antissimetrica(r) && rl_transitiva(r);
}
/* as CLASSES: o representante de cada uma é o menor elemento que a ela pertence.
 * Devolve quantas há, e escreve em `cl` a classe de cada elemento. E a PARTIÇÃO
 * verifica-se: cada elemento numa classe e numa só, e a união dá o conjunto. */
static int rl_classes(const Rel *r, int *cl){
    int nc = 0;
    for(int a = 0; a < r->n; a++) cl[a] = -1;
    for(int a = 0; a < r->n; a++){
        if(cl[a] >= 0) continue;
        for(int b = 0; b < r->n; b++) if(r->m[a][b]) cl[b] = nc;
        cl[a] = nc;
        nc++;
    }
    return nc;
}
static int rl_particao(const Rel *r, const int *cl, int nc){
    for(int a = 0; a < r->n; a++) if(cl[a] < 0 || cl[a] >= nc) return 0;
    /* na mesma classe ⟺ relacionados — é isto que faz das classes uma partição */
    for(int a = 0; a < r->n; a++) for(int b = 0; b < r->n; b++)
        if((cl[a] == cl[b]) != (r->m[a][b] != 0)) return 0;
    return 1;
}
/* ── a FUNÇÃO: a relação com existência e unicidade ─────────────────────────────── */
static int rl_total(const Rel *r){                     /* ∀a ∃b: existência */
    for(int a = 0; a < r->n; a++){
        int achou = 0;
        for(int b = 0; b < r->n; b++) if(r->m[a][b]) achou = 1;
        if(!achou) return 0;
    }
    return 1;
}
static int rl_univalente(const Rel *r){                /* ∃! : unicidade */
    for(int a = 0; a < r->n; a++){
        int q = 0;
        for(int b = 0; b < r->n; b++) if(r->m[a][b]) q++;
        if(q > 1) return 0;
    }
    return 1;
}
static int rl_funcao(const Rel *r){ return rl_total(r) && rl_univalente(r); }
static int rl_injetiva(const Rel *r){                  /* f(a)=f(b) ⇒ a=b */
    for(int b = 0; b < r->n; b++){
        int q = 0;
        for(int a = 0; a < r->n; a++) if(r->m[a][b]) q++;
        if(q > 1) return 0;
    }
    return 1;
}
static int rl_sobrejetiva(const Rel *r){               /* ∀b ∃a: f(a)=b */
    for(int b = 0; b < r->n; b++){
        int q = 0;
        for(int a = 0; a < r->n; a++) if(r->m[a][b]) q++;
        if(!q) return 0;
    }
    return 1;
}
static int rl_bijetiva(const Rel *r){
    return rl_funcao(r) && rl_injetiva(r) && rl_sobrejetiva(r);
}
/* ── o DUAL e o PRODUTO: a inversa e a composição ───────────────────────────────── */
static void rl_dual(const Rel *r, Rel *d){             /* R† = a inversa da relação */
    rl_zera(d, r->n);
    for(int a = 0; a < r->n; a++) for(int b = 0; b < r->n; b++) d->m[b][a] = r->m[a][b];
}
static void rl_comp(const Rel *f, const Rel *g, Rel *h){   /* h = g ∘ f */
    rl_zera(h, f->n);
    for(int a = 0; a < f->n; a++) for(int b = 0; b < f->n; b++){
        if(!f->m[a][b]) continue;
        for(int c = 0; c < f->n; c++) if(g->m[b][c]) h->m[a][c] = 1;
    }
}
static void rl_id(Rel *r, int n){
    rl_zera(r, n);
    for(int a = 0; a < n; a++) r->m[a][a] = 1;
}
static int rl_igual(const Rel *a, const Rel *b){
    if(a->n != b->n) return 0;
    for(int i = 0; i < a->n; i++) for(int j = 0; j < a->n; j++)
        if(a->m[i][j] != b->m[i][j]) return 0;
    return 1;
}
/* A VOLTA, e é ela que define a bijeção: f⁻¹∘f = id E f∘f⁻¹ = id, nos DOIS sentidos —
 * um lado só seria metade com o nome do par. */
static int rl_volta(const Rel *f){
    Rel d, e1, e2, id;
    rl_dual(f, &d);
    rl_comp(f, &d, &e1);                /* f então f†  */
    rl_comp(&d, f, &e2);                /* f† então f  */
    rl_id(&id, f->n);
    return rl_igual(&e1, &id) && rl_igual(&e2, &id);
}
#endif
