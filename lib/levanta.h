/* ═══════════════════════════════════════════════════════════════════════════
 * lib/levanta.h — COMPLETAR um corpo: o levantamento ι e a projecção π
 *
 * O `lib/escada.h` diz QUANDO um corpo está completo (G constante) e QUANTO
 * lhe falta. Este diz como se COMPLETA --- e a construção não é nova: é o par
 * ι/π da `aranha cor:pik`, com ι a inclusão canónica que preenche com zeros as
 * posições novas e π a projecção que as esquece.
 *
 *     resumo          x ∈ X          fibras de tamanhos DIFERENTES: G varia
 *     levantado    (x,j) ∈ X × G_max fibras todas do MESMO tamanho: G ≡ G_max
 *
 *     π(x,j) = x        e        π ∘ ι = id
 *
 * A projecção é sobrejectiva e as suas fibras têm todas G_max elementos, logo
 * o levantado está completo POR CONSTRUÇÃO --- e o resumo é a sua sombra. O
 * que se acrescenta são exactamente as `es_falta` posições em falta, nem mais
 * uma: o levantamento não inventa objectos, dá lugar aos que já lá estavam a
 * disputar o mesmo endereço.
 *
 * E a lei que faz isto valer a pena é a compatibilidade entre andares da
 * `aranha prop:travessia`,   π_K^L ∘ P_L ∘ ι_K^L = P_K:  a travessia não
 * depende da casa onde se a calcula, logo medir no levantado e projectar dá o
 * mesmo que medir no resumo --- quando o resumo distingue o suficiente.
 *
 * Medido em `tests/levanta.c` e `tests/pgwire.c` §W170.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef LEVANTA_H
#define LEVANTA_H

#include "escada.h"

#define LV_MAX 4096

/* ── O LEVANTADO: quem é cada objecto lá em cima ────────────────────────
 * `base[k]` é o endereço do resumo e `folha[k]` a posição dentro da fibra,
 * de 0 a G_max−1. Os que existiam ficam com folha < G(x); os que faltavam
 * entram como as posições novas --- os «zeros» da inclusão canónica. */
typedef struct {
    long n;             /* quantos objectos tem o levantado: fibras × G_max */
    long fibras;        /* quantas fibras tinha o resumo */
    long gmax;          /* o tamanho da maior --- e o G constante lá em cima */
    long acrescentados; /* quantos lugares vazios se abriram: é es_falta */
    long base[LV_MAX];  /* o endereço no resumo de cada objecto levantado */
    long folha[LV_MAX]; /* a posição dentro da fibra */
    int  existia[LV_MAX];/* 1 se já havia objecto nesse lugar, 0 se é novo */
} LvLevanta;

/* ── ι: levantar. Devolve 0 se não coube, e aí nada se lê da estrutura. ── */
static int lv_levanta(const long *end, long n, LvLevanta *L){
    EsFibra f = es_fibra(end, n);
    if(f.fibras == 0 || f.fibras * f.maior > LV_MAX) return 0;
    L->n = f.fibras * f.maior;
    L->fibras = f.fibras;
    L->gmax = f.maior;
    L->acrescentados = f.fibras * f.maior - f.soma;
    /* os endereços distintos, na ordem em que aparecem --- a ordem não é
     * estrutura, é só uma enumeração; o que é estrutura é a fibra */
    long vis[LV_MAX]; long nf = 0;
    for(long i = 0; i < n; i++){
        int novo = 1;
        for(long j = 0; j < nf; j++) if(vis[j] == end[i]){ novo = 0; break; }
        if(novo) vis[nf++] = end[i];
    }
    long k = 0;
    for(long j = 0; j < nf; j++){
        long tam = 0;
        for(long i = 0; i < n; i++) if(end[i] == vis[j]) tam++;
        for(long p = 0; p < f.maior; p++){
            L->base[k] = vis[j];
            L->folha[k] = p;
            L->existia[k] = (p < tam);
            k++;
        }
    }
    return 1;
}

/* ── π: projectar de volta. É esquecer a folha, e mais nada. ───────────── */
static long lv_projecta(const LvLevanta *L, long k){ return L->base[k]; }

/* ── π ∘ ι = id: o que subiu e desceu é o mesmo. ────────────────────────
 * Percorre os que existiam e confirma que a projecção devolve o endereço
 * de partida, com as multiplicidades certas. */
static int lv_pi_iota_id(const long *end, long n, const LvLevanta *L){
    for(long i = 0; i < n; i++){
        long conta = 0;
        for(long k = 0; k < L->n; k++)
            if(L->existia[k] && lv_projecta(L, k) == end[i]) conta++;
        long tam = 0;
        for(long j = 0; j < n; j++) if(end[j] == end[i]) tam++;
        if(conta != tam) return 0;
    }
    return 1;
}

/* ── E O QUE INTERESSA: lá em cima G é constante e a falta é zero. ─────── */
static int lv_completo(const LvLevanta *L){
    EsFibra f = es_fibra(L->base, L->n);
    return f.constante && f.maior == L->gmax && f.soma == L->n;
}

/* ── O endereço do levantado numa palavra só, para quem quer medir a régua
 * lá em cima: a folha entra nos bits BAIXOS, porque é a distinção mais
 * fina --- a que se decide por último ao descer. */
static long lv_endereco(const LvLevanta *L, long k){
    long d = 1, g = L->gmax - 1;
    while(g){ d <<= 1; g >>= 1; }
    return L->base[k] * d + L->folha[k];
}

/* ── ISOMORFO À ARANHA: dois corpos completos com o mesmo tamanho e o mesmo
 * G têm a mesma assinatura de fibras, logo há bijecção que a respeita.
 * Não é uma conta sobre os elementos: é a única coisa que a fibra vê. */
static int lv_isomorfo(const LvLevanta *A, const LvLevanta *B){
    return A->n == B->n && A->gmax == B->gmax && A->fibras == B->fibras;
}

#endif /* LEVANTA_H */
