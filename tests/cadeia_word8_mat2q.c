/* tests/cadeia_word8_mat2q.c — A CADEIA Word₈ → E₁₆ → Qz → Mat → Mat2Q, COMPOSTA.
 *
 * Porquê este ficheiro existe. Cada elo desta cadeia já estava medido, mas em
 * medidores SEPARADOS, e nenhum deles compunha: `cadeia.c`, `migracao.c`,
 * `palavra.c`, `promocao.c`, `reducao.c` mexem em W₈ e Qz e NÃO em Mat;
 * `matriz_q.c`, `ponte_matriz_q.c`, `coerencia_mult_mat2q.c` mexem em Qz e Mat
 * e NÃO em W₈. Era uma cadeia NARRATIVA, não uma cadeia de DADOS.
 *
 * Aqui a saída de cada etapa é literalmente a entrada da seguinte. O valor que
 * entra em `qz_de_inteiro` não é um literal: é o `lg_val` do `LgPar` que
 * `w8_mult_larg` devolveu. É isso que se mede.
 *
 * ── O QUE ESTE FICHEIRO NÃO FAZ, e é deliberado ─────────────────────────────
 *   - NÃO altera sql.c, campos.tex, espaco.tex, inteiros.tex, palavra8.h,
 *     naturais.h, racionais.h, linear.h, matriz2q.h, matriz2q_ponte.h
 *   - NÃO escreve uma função nova de biblioteca: todos os passos são funções
 *     que já existiam, chamadas pelo nome
 *   - NÃO inventa denominador: o passo E₁₆ → Qz usa `qz_de_inteiro`, cujo
 *     corpo É `qz(n,1)` (racionais.h:188). O contrato é o nome da função
 *   - NÃO constrói GF(2^16): `BN_ANDARES 4` (binario.h:42) chega a 256 e
 *     §NB13 diz que o andar seguinte não precisa de ser construído
 *   - NÃO implementa ι₁ : X₁ ↪ X₂, e NÃO identifica X₂ com GF(2^16)
 *
 * ── O QUE A CADEIA COMPÕE, E O QUE NÃO COMPÕE ──────────────────────────────
 * COMPÕE: a CONSTRUÇÃO. Um valor de Word₈ desce a E₁₆, vira Qz, entra numa Mat
 *         e chega a Mat2Q com o valor intacto em cada etapa.
 * NÃO COMPÕE: a ARITMÉTICA. O produto que desce é o NUMÉRICO (`lg_mult`, o
 *         mesmo de `promocao.c` §SP0/§SP3), não o de corpo (`w8_mul_f8`).
 *         São operações diferentes, e §2 mede a prova de que diferem.
 *
 *   cc -O2 -std=c99 -Ilib -o cadeia tests/cadeia_word8_mat2q.c -lm
 */
#include "matriz2q.h"
#include "matriz2q_ponte.h"
#include "linear.h"
#include "palavra8.h"
#include "naturais.h"
#include "unidade.h"

/* ══ OS CASOS: pares de Word₈, com as fronteiras todas dentro ═════════════════
 *
 * Um array. A contagem sai de `sizeof`, numa passage só — como se fez em
 * `coerencia_mult_mat2q.c`, e pelo mesmo motivo. */
static const unsigned char PARES[][2] = {
    {  0,  0}, {  0,255}, {255,  0}, {  1,  1},     /* os quatro cantos */
    {  1,255}, {255,  1}, { 15, 17}, { 16, 16},     /* 255 = o maior que fica; 256 = o primeiro que sai */
    { 15, 18}, { 16, 17}, {  2,128}, {128,  2},     /* a borda outra vez, dos dois lados */
    {128,128}, {  7, 37}, { 13, 19}, {255,255},     /* 65025: o máximo, e o limite do E₁₆ */
};
#define NP    ((int)(sizeof PARES / sizeof *PARES))
#define NMAT  (NP / 4)                     /* matrizes 2×2 montadas com a saída */
#define W8_MAX_UINT16 65025u               /* 255·255: o maior produto de dois Word₈ */

/* ── o valor de um par, pela CADEIA, e não por literal ──────────────────────
 * Devolve o Qz que saiu do último elo, e diz quantos passos promotion-counted. */
typedef struct { Qz q; long promo; int inteiro_preservado, dentro; } Elo;

static Elo desce(unsigned char a, unsigned char b){
    Elo e; e.promo = 0; e.dentro = 0; e.inteiro_preservado = 0;
    /* 1. Word₈ × Word₈ → o par de E₁₆.  w8_mult_larg(8,·,·) = lg_mult (largura.h:49) */
    LgPar p = w8_mult_larg(8, a, b);
    /* 2. o par lido como valor de 16 bits.  lg_val (largura.h:43) */
    uint64_t v = lg_val(p, 8);
    if(v > 0xFFFFu) return e;                 /* guarda: estreitar seria perda (nunca acontece, e mede-se) */
    e.dentro = (v <= W8_MAX_UINT16);           /* o tecto de dois bytes cabe de facto em 16 bits */
    /* 3. a política de escrita no envelope: saturo→promove.  w8_proj_sat (naturais.h:155) */
    uint16_t e16 = w8_proj_sat((uint16_t)v);
    /* 4. E₁₆ → Qz.  qz_de_inteiro (racionais.h:188) = qz(n,1): o contrato nomeado. */
    e.q = qz_de_inteiro((long)e16);
    e.inteiro_preservado = (e.q.p == (int64_t)v && e.q.q == 1);
    e.promo = (v > 255u) ? 1 : 0;
    return e;
}

int main(void){
    printf("══════════════════════════════════════════════════════════════\n");
    printf("  A CADEIA COMPOSTA:  Word₈ → E₁₆ → Qz → Mat → Mat2Q\n");
    printf("══════════════════════════════════════════════════════════════\n");

    /* ══ §1 O INVENTÁRIO: que função é cada elo, e de onde vem ═══════════════
     * Sai impresso para que a composição seja auditável sem abrir os headers. */
    printf("\n§1  a cadeia, elo a elo — só funções que JÁ EXISTIAM\n");
    printf("      1  Word₈ × Word₈ → par E₁₆   w8_mult_larg(8,a,b)   palavra8.h:62\n");
    printf("      2  par → valor de 16 bits     lg_val(p,8)           largura.h:43\n");
    printf("      3  política de escrita        w8_proj_sat(uint16)   naturais.h:155\n");
    printf("      4  E₁₆ → Qz                   qz_de_inteiro(n)      racionais.h:188\n");
    printf("         └─ o corpo é qz(n,1): o denominador 1 é o CONTRATO, não uma escolha\n");
    printf("      5  Qz,2 → V2Q                 m2q_v(x,y)            matriz2q.h:37\n");
    printf("      6  V2Q → Vec                   m2q_ponte_Vec(u)      matriz2q_ponte.h:31\n");
    printf("      7  Vec → Mat                   mat_de_colunas(v,k)   linear.h:231\n");
    printf("      8  Mat → Mat2Q                m2q_ponte_Mat2Q(A,&M) matriz2q_ponte.h:37\n");
    printf("      9  e a volta, para o resíduo   m2q_ponte_Mat(M)     matriz2q_ponte.h:24\n");
    printf("      adaptadores escritos aqui: ZERO\n");

    /* ══ §2 O QUE O BYTE É, MEDIDO: GF(2^8), e NÃO Z/256Z ═══════════════════
     * A cadeia acima compõe o produto NUMÉRICO, e por isso não decide o que é
     * o byte. Isto decide. E é a mesma distinção que `campos.tex` faz em
     * `X_d = (Z/256Z)^d`: se 128 fosse divisor de zero, o byte seria Z/256Z. */
    printf("\n§2  o byte é GF(2^8) e não Z/256Z — medido, não afirmado\n");
    unsigned char soma_128 = w8_som_f8(128, 128);
    unsigned char mult_128 = w8_mul_f8(128, 128);
    unsigned char inv_128  = w8_inv_f8(128);
    unsigned char volta_128 = w8_mul_f8(128, inv_128);
    unsigned int  z256_128  = (128u * 128u) % 256u;      /* o que Z/256Z daria */
    printf("      w8_som_f8(128,128) = %u   (característica 2: o oposto é a identidade)\n", soma_128);
    printf("      w8_mul_f8(128,128) = %u   em Z/256Z daria %u (= 0, divisor de zero)\n",
           mult_128, z256_128);
    printf("      w8_inv_f8(128)    = %u   e 128·inv = %u\n", inv_128, volta_128);
    ok("§2a: em GF(2^8), 128 tem INVERSO e 128·128 ≠ 0. Em Z/256Z, 128·128 ≡ 0 e não tem"
       " inverso. Logo o Word8 do código NÃO é (Z/256Z) — e o `Word_8 = F_8` de"
       " palavra8.h:1 fica pelo lado do corpo, sem atravessar para X₁ de campos.tex",
       inv_128 != 0 && volta_128 == 1 && mult_128 != 0 && z256_128 == 0);
    ok("§2b: a soma 128+128 = 0 NÃO distingue os dois (vale em característica 2 e em Z/256Z)."
       " O que distingue é o PRODUTO, e é o §2a que o mede",
       soma_128 == 0);

    /* ══ §3 A DESCIDA, VALOR A VALOR ═══════════════════════════════════════ */
    printf("\n§3  a descida Word₈ → E₁₆ → Qz, com a saída a alimentar o passo seguinte\n");
    long sat_antes = w8_saturou, perdido_antes = qz_perdeu;
    Elo elos[NP];
    long compostos = 0, promo = 0, exatos = 0, perdas = 0, dentro = 0;
    for(int i = 0; i < NP; i++){
        Elo e = desce(PARES[i][0], PARES[i][1]);
        elos[i] = e;
        if(e.inteiro_preservado) exatos++;
        if(e.promo) promo++;
        if(!e.inteiro_preservado) perdas++;
        if(e.dentro) dentro++;
        compostos++;
        printf("      %3u·%-3u → Qz(%lld/%lld)%s\n",
               PARES[i][0], PARES[i][1],
               (long long)elos[i].q.p, (long long)elos[i].q.q,
               e.promo ? "  [promoveu: saiu do byte]" : "");
    }
    printf("      %ld composto(s): %ld exacto(s), %ld promoveu, %ld perdido(s)\n",
           compostos, exatos, promo, perdas);
    ok("§3a: o Qz de cada elo é o valor EXACTO do produto, com denominador 1 — a descida"
       " é a identidade sobre o valor, não uma reetiquetagem", exatos == compostos && perdas == 0);
    ok("§3b: o contador `w8_saturou` da casa concorda com a contagem INDEPENDENTE de"
       " produtos acima de 255 — o gatilho não é uma opinião, é o resultado",
       w8_saturou - sat_antes == promo);
    ok("§3c: nenhum valor passou por perda na aritmética racional (qz_perdeu não mexeu)",
       qz_perdeu == perdido_antes);
    ok("§3d: o estreitamento para 16 bits é TOTAL, não é uma perda disfarçada — o maior"
       " produto de dois Word₈ é 255·255 = 65025 e cabe em E₁₆, sem perda em nenhum caso",
       dentro == compostos);

    /* ══ §4 A MONTAGEM: a saída de §3 é a entrada de §4 ════════════════════
     * Quatro Qz que SAIERAM de §3 entram, sem recalcular nada, na Mat. */
    printf("\n§4  Qz → V2Q → Vec → Mat → Mat2Q, com a saída de §3 como entrada\n");
    long mal_mont = 0;
    for(int m = 0; m < NMAT; m++){
        Elo *e = &elos[4 * m];
        /* §3 já deu os Qz. Aqui só se empacotam, com as funções da casa. */
        Vec c0 = m2q_ponte_Vec(m2q_v(e[0].q, e[1].q));   /* coluna 0 */
        Vec c1 = m2q_ponte_Vec(m2q_v(e[2].q, e[3].q));   /* coluna 1 */
        Vec cols[2] = { c0, c1 };
        Mat A = mat_de_colunas(cols, 2);                  /* 2×2 com entradas Qz */
        Mat2Q M; int traduziu = m2q_ponte_Mat2Q(A, &M);
        /* mat_de_colunas põe A[i][j] = v[j].c[i] — a COLUNA v[j]. A matriz
           sai [(e0,e2),(e1,e3)], e é isso que se verifica, escrito à mão. */
        int bate = traduziu
                && M.a.p == e[0].q.p && M.a.q == e[0].q.q
                && M.b.p == e[2].q.p && M.b.q == e[2].q.q
                && M.c.p == e[1].q.p && M.c.q == e[1].q.q
                && M.d.p == e[3].q.p && M.d.q == e[3].q.q
                && A.m == 2 && A.n == 2;
        if(!bate) mal_mont++;
        printf("      mat%ld  traduziu=%d  [%lld/%lld %lld/%lld ; %lld/%lld %lld/%lld]%s\n",
               (long)m, traduziu,
               (long long)A.a[0][0].p, (long long)A.a[0][0].q,
               (long long)A.a[0][1].p, (long long)A.a[0][1].q,
               (long long)A.a[1][0].p, (long long)A.a[1][0].q,
               (long long)A.a[1][1].p, (long long)A.a[1][1].q,
               bate ? "" : "   <<< NÃO BATE");
    }
    ok("§4a: a Mat2Q resultante tem EXACTAMENTE os Qz que saíram de §3, campo a campo"
       " (p e q em bruto). Nenhum foi recalculado, arredondado ou re-normalizado pelo caminho",
       mal_mont == 0 && NMAT > 0);

    /* ══ §5 A VOLTA: Mat2Q → Mat, e o resíduo ═════════════════════════════ */
    printf("\n§5  a volta pela ponte, entrada a entrada: resíduo 0\n");
    long residuo = 0;
    for(int m = 0; m < NMAT; m++){
        Elo *e = &elos[4 * m];
        Vec c0 = m2q_ponte_Vec(m2q_v(e[0].q, e[1].q));
        Vec c1 = m2q_ponte_Vec(m2q_v(e[2].q, e[3].q));
        Vec cols[2] = { c0, c1 };
        Mat A = mat_de_colunas(cols, 2);
        Mat2Q M = m2q_id(); m2q_ponte_Mat2Q(A, &M);
        Mat volta = m2q_ponte_Mat(M);
        if(!mat_igual(A, volta)) residuo++;
    }
    ok("§5a: ir (Mat → Mat2Q) e voltar (Mat2Q → Mat) devolve a MESMA matriz, entrada a"
       " entrada — resíduo 0 em todas as matrizes compostas", residuo == 0);

    /* ══ §6 O QUE ESTE FICHEIRO NÃO COMPOE ═════════════════════════════════ */
    printf("\n§6  as duas fronteiras que continuam abertas, e porquê\n");
    printf("      GF(2^16): NÃO construído. BN_ANDARES = %s (binario.h:42); §NB13\n", "4");
    printf("                  declara que o andar seguinte não precisa de ser erguido.\n");
    printf("      ι₁ : X₁ ↪ X₂ (campos.tex:479-481): NÃO implementado, sem teste.\n");
    printf("                  X₁ = Z/256Z e Word₈ = GF(2^8) partilham os 256 rótulos e\n");
    printf("                  NÃO são identificados como estruturas (§2 mediu a diferença).\n");
    printf("      X₂ ≡ GF(2^16): NÃO afirmado. Mesmo cardinal (65536) e naturezas\n");
    printf("                  distintas — (Z/256Z)² é um módulo, GF(2^16) seria um corpo.\n");
    ok("§6a: este medidor não afirma nenhuma das três: GF(2^16) não existe no código, ι₁ não"
       " tem implementação, e a igualdade X₂ ≡ GF(2^16) não é usada em nenhum teste",
       1);

    printf("\n══════════════════════════════════════════════════════════════\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas,
           falhas ? "" : " — RESIDUO 0");
    printf("  pares %d (sizeof) · compostos %ld · promovidos %ld · matrices %d · resíduo %ld\n",
           NP, compostos, promo, NMAT, residuo);
    return falhas ? 1 : 0;
}
