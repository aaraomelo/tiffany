/* ═══════════════════════════════════════════════════════════════════════════
 * lib/global.h — O COROLÁRIO GLOBAL, realizado: travessia(X, R_o, R_d)
 *
 * A `aranha cor:global` diz que todo objecto finitamente coordenado herda um
 * endereço reversível, que quaisquer duas representações reversíveis do mesmo
 * objecto são ligadas por uma travessia efectiva, e que sob a ultramétrica dos
 * endereços o custo de uma composição é dominado pelo maior custo dos passos:
 *
 *     D(R_0, R_n)  ≤  max_i D(R_{i-1}, R_i)
 *
 * E dá a OPERAÇÃO em cinco passos, que é o que aqui se executa:
 *   (1) endereça --- não se pergunta qual representação é «melhor»
 *   (2) toma o destino da tarefa
 *   (3) aplica  a' = R_d ∘ R_o⁻¹ (a)
 *   (4) mede D, o pior salto
 *   (5) absorve os passos pequenos: pago um δ, os ≤ δ ficam dentro da mesma bola
 *
 * O QUE NÃO SE AFIRMA, e o paper é explícito: o passo (5) não autoriza «executar
 * primeiro o de custo máximo» --- isso exigiria os intermediários serem
 * operacionalmente comutáveis, e sem essa hipótese a ordem vem da dependência
 * entre as travessias. E: endereçar é geral, OPERAR é particular --- a operação
 * transporta-se por conjugação e continua específica do objecto.
 *
 * Medido em `tests/global.c` e `tests/pgwire.c` §W174.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef GLOBAL_H
#define GLOBAL_H

#include "triade.h"      /* tv_prof: a ultramétrica dos endereços */

#define GL_MAX 512
#define GL_REPR 16

/* ── (1) A HIPÓTESE, verificada e não suposta: a representação é REVERSÍVEL.
 * Sem isto o corolário não se aplica --- e é a única hipótese que ele tem
 * sobre cada representação individual. */
static int gl_reversivel(const long *R, long n){
    for(long i = 0; i < n; i++) for(long j = 0; j < i; j++)
        if(R[i] == R[j]) return 0;
    return 1;
}

/* ── (3) A TRAVESSIA: a' = R_d ∘ R_o⁻¹ (a). Não se procura: o objecto que tem
 * o endereço `a` em R_o é o mesmo que tem `a'` em R_d. Devolve −1 se `a` não é
 * endereço de ninguém em R_o. */
static long gl_travessia(long a, const long *Ro, const long *Rd, long n){
    for(long i = 0; i < n; i++) if(Ro[i] == a) return Rd[i];
    return -1;
}

/* ── (4) O CUSTO: D(R_o,R_d) = 2^{−q}, com q a MENOR profundidade a que os dois
 * endereços do mesmo objecto divergem --- o pior salto. Devolve q; o custo é
 * 2^{−q}, e q = bits significa que as duas leituras coincidem (custo zero).
 *
 * Conta do dígito mais significativo, como a def:arvore e o `prof`. */
static int gl_custo(const long *Ro, const long *Rd, long n, int bits){
    int pior = bits;
    for(long i = 0; i < n; i++){
        int p = tv_prof(Ro[i], Rd[i], bits);
        if(p < pior) pior = p;
    }
    return pior;
}

/* ── A DESIGUALDADE DO COROLÁRIO, sobre uma cadeia de representações.
 * `R[k]` é a k-ésima representação, cada uma com n endereços. */
typedef struct {
    int  passos;        /* quantas travessias na cadeia */
    int  q_ponta;       /* a profundidade entre R_0 e R_n --- o custo global */
    int  q_pior_passo;  /* a MENOR profundidade entre passos --- o maior custo */
    int  domina;        /* 1 se D(R_0,R_n) ≤ max_i D --- isto é, q_ponta ≥ q_pior */
    int  reversiveis;   /* quantas das representações são reversíveis */
} GlCadeia;

static GlCadeia gl_cadeia(const long R[][GL_MAX], int k, long n, int bits){
    GlCadeia c = {0, bits, bits, 0, 0};
    for(int i = 0; i < k; i++) if(gl_reversivel(R[i], n)) c.reversiveis++;
    c.passos = k - 1;
    for(int i = 1; i < k; i++){
        int q = gl_custo(R[i-1], R[i], n, bits);
        if(q < c.q_pior_passo) c.q_pior_passo = q;
    }
    c.q_ponta = gl_custo(R[0], R[k-1], n, bits);
    /* custo = 2^{−q}, logo «custo menor» é «q maior»: a dominação é q_ponta ≥ q_pior */
    c.domina = (c.q_ponta >= c.q_pior_passo);
    return c;
}

/* ── (5) A ABSORÇÃO: pago um salto de escala δ, toda travessia posterior de
 * custo ≤ δ fica DENTRO DA MESMA BOLA e não aumenta o custo global. Aqui δ é
 * dado por q_delta (custo 2^{−q_delta}); devolve quantos passos da cadeia são
 * absorvidos, isto é, têm custo ≤ δ.
 *
 * E o que isto NÃO autoriza: reordenar a cadeia para pagar primeiro o máximo.
 * O paper di-lo em letra própria --- faria falta os passos serem comutáveis. */
static int gl_absorve(const long R[][GL_MAX], int k, long n, int bits, int q_delta){
    int absorvidos = 0;
    for(int i = 1; i < k; i++)
        if(gl_custo(R[i-1], R[i], n, bits) >= q_delta) absorvidos++;
    return absorvidos;
}

/* ═══ A PEÇA DO COROLÁRIO: A ULTRAMÉTRICA HERDA-SE PELA BIJEÇÃO ════════════
 *
 * O global não é só «há travessia». É que a ultramétrica vive no ÍNDICE, e a
 * bijeção transporta-a para o corpo:
 *
 *     d_X(x,y) := d_I( R(x), R(y) )
 *
 * Como R é bijecção, d_X é métrica; e como d_I é ultramétrica, d_X é
 * ultramétrica --- a desigualdade forte atravessa sem se degradar, porque só
 * usa a igualdade dos valores. É por isto que completar um corpo do catálogo
 * não pede régua nova: PEDE UMA REPRESENTAÇÃO REVERSÍVEL, e a régua vem atrás.
 *
 * E duas representações dão a MESMA topologia mesmo com valores diferentes: as
 * bolas coincidem, porque a travessia entre elas é bijecção. O que difere é o
 * custo D, não a estrutura. */

/* a distância herdada por R: d_X(i,j) = 2^{−prof(R_i,R_j)}, devolvida como prof */
static int gl_dist(const long *R, long i, long j, int bits){
    return tv_prof(R[i], R[j], bits);
}

/* a desigualdade ULTRAMÉTRICA no corpo, herdada: conta os triplos que a
 * respeitam, os que a violam, e os ESTRITOS --- sem estritos a condição não
 * está a decidir nada. */
typedef struct { long triplos, viola, estrito; } GlUltra;

static GlUltra gl_ultra_herdada(const long *R, long n, int bits){
    GlUltra u = {0, 0, 0};
    for(long i = 0; i < n; i++) for(long j = 0; j < n; j++) for(long k = 0; k < n; k++){
        int a = gl_dist(R, i, j, bits), b = gl_dist(R, j, k, bits),
            c = gl_dist(R, i, k, bits);
        int m = a < b ? a : b;          /* prof menor = distância maior */
        u.triplos++;
        if(c < m) u.viola++;            /* d(i,k) > max: viola a forte */
        else if(c > m) u.estrito++;
    }
    return u;
}

/* E AS BOLAS COINCIDEM entre duas representações: o que muda é o valor de D,
 * não quem está dentro. Conta os pares em que as duas leituras discordam sobre
 * «estar na mesma bola de raio q» --- tem de ser zero quando as duas cifram o
 * mesmo prefixo, e é a medida de que a topologia é a MESMA. */
static long gl_bolas_discordam(const long *R0, const long *R1, long n, int bits, int q){
    long d = 0;
    for(long i = 0; i < n; i++) for(long j = 0; j < i; j++){
        int a = tv_prof(R0[i], R0[j], bits) >= q;
        int b = tv_prof(R1[i], R1[j], bits) >= q;
        if(a != b) d++;
    }
    return d;
}

#endif /* GLOBAL_H */
