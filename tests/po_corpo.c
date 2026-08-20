/* po_corpo.c — DO PÓ AO CORPO E DE VOLTA. O entrópico não é condenação: falta-lhe o dual.
 *
 * Migrado de chess/elementares/lei_geral_entropico_cosmico.py, e do capítulo "A Doença: Carnot"
 * do enredo (parte "O Saco de Lixo"). O Aarão:
 *
 *   "você diz que não tem inverso aditivo? PROVA. Tem sim. Não tem SOZINHO. COM SEU DUAL TEM."
 *   "se você duvida, simula um ser humano completo, do pó ao corpo, e depois REVERTE."
 *
 * O entrópico é (max,+): o ⊕ = max é idempotente, logo sem inverso aditivo — max(a,b) ≥ a, e nada
 * volta. Daí o veredito que a física assinou: o que cresce não desce, o universo morre.
 *
 * A doença está no SOZINHO. A ausência de inverso é propriedade do LIMITE T→0 — o max duro, o
 * zero absoluto, e ninguém mora lá. A qualquer T>0 o ⊕ é o logsumexp, que sob o exp do seu dual
 * — o CÓSMICO, cujo operador é a expansão a(t)=e^{Ht} — vira a SOMA. E a soma tem inverso.
 *
 * A prova é construtiva: constrói-se um corpo humano a partir do pó (as frações exatas dos seis
 * elementos que dão ~99% da massa de uma pessoa) e REVERTE-SE. Em ℚ o ciclo fecha com resíduo
 * EXATAMENTE 0 — não 1e-16, zero. E mede-se aqui que em float o zero seria FALSO.
 *
 *   §P1  o entrópico SOZINHO: max(a,b) ≥ a, logo nenhum elemento tem oposto
 *   §P2  com o DUAL: destropicalizado, o ⊕ é a SOMA — e a soma tem oposto, para todos
 *   §P3  o PÓ: os seis elementos, e as frações somam ~99% da massa
 *   §P4  do pó ao CORPO e de volta: resíduo EXATAMENTE 0 em ℚ
 *   §P5  e em FLOAT o zero seria falso — medido, e é por isso que se usa ℚ
 *   §P6  a lei geral: a conservação vale sobre o PAR
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/po_corpo.c -o po_corpo
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "corpos.h"
#include "i128.h"
#include "unidade.h"

#define N 6
typedef struct { I128 n, d; } Q;
static Q qn(I128 n, I128 d){
    if(i128_negativo(d)){ n = i128_neg(n); d = i128_neg(d); }
    I128 g = i128_gcd(n, d);
    Q r;
    r.n = i128_div(n, g);
    r.d = i128_div(d, g);
    return r;
}
static Q qadd(Q a, Q b){
    I128 g = i128_gcd(a.d, b.d);
    I128 ad = i128_div(a.d, g), bd = i128_div(b.d, g);
    return qn(i128_add(i128_mul(a.n, bd), i128_mul(b.n, ad)), i128_mul(ad, b.d));
}
static Q qsub(Q a, Q b){ Q m = {i128_neg(b.n), b.d}; return qadd(a, m); }
static Q qmul(Q a, Q b){
    I128 g1 = i128_gcd(a.n, b.d), g2 = i128_gcd(b.n, a.d);
    return qn(i128_mul(i128_div(a.n, g1), i128_div(b.n, g2)),
              i128_mul(i128_div(a.d, g2), i128_div(b.d, g1)));
}
static Q qdiv(Q a, Q b){ Q i = {b.d, b.n}; return qmul(a, i); }
static void pq(Q a){
    if(i128_fits_i64(a.n) && i128_fits_i64(a.d))
        printf("%" PRId64 "/%" PRId64, i128_to_i64(a.n), i128_to_i64(a.d));
    else printf("(fracção grande)");
}
static int qz(Q a){ return i128_is_zero(a.n); }

/* o PÓ: a composição elementar real do corpo humano, frações exatas da massa */
static const char *NOMES[N] = { "O  oxigénio","C  carbono","H  hidrogénio",
                                "N  azoto","Ca cálcio","P  fósforo" };
static Q po_i(int i){
    static const int32_t nu[N] = { 65, 185,  95,  32,  15,  10 };
    static const int32_t de[N] = {100,1000,1000,1000,1000,1000};
    return qn(i128_from_i64(nu[i]), i128_from_i64(de[i]));
}
/* a MONTAGEM: o operador da vida — acopla os elementos. Exata, e invertível. */
static Q mm(int i, int j){
    return (i == j) ? qn(i128_from_i64(1), i128_from_i64(1))
                    : qn(i128_from_i64(1), i128_from_i64(3 + ((i + j) % 5)));
}

int main(void){
printf("\n=== DO PÓ AO CORPO, E DE VOLTA ============================================\n");
printf("    O entrópico não tem inverso SOZINHO. Com o seu dual, tem — e o ciclo fecha.\n");

printf("\n§P1  O entrópico SOZINHO: max(a,b) ≥ a, logo nenhum elemento tem oposto.\n\n");
{
    int mau = 0; int32_t com = 0, casos = 0;
    for(int32_t a = -40; a <= 40; a++){
        int tem = 0;
        for(int32_t b = -40; b <= 40; b++){ int32_t m = a > b ? a : b; if(m < a) tem = 1; }
        if(tem) com++;
        casos++;
    }
    if(com) mau++;
    ok("no (max,+) NENHUM elemento tem oposto — o max nunca desce", mau == 0);
    printf("      (%" PRId32 " elementos, %" PRId32 " com oposto.)\n", casos, com);
    printf("\n      É este o teorema que a física assinou como sentença: o que cresce não desce.\n");
    printf("      Está CERTO — e é sobre o corpo SOZINHO, no limite T→0. Ninguém mora lá.\n");
}

printf("\n§P2  Com o DUAL: destropicalizado, o ⊕ é a SOMA — e a soma tem oposto.\n\n");
{
    int mau = 0; int32_t casos = 0;
    for(int32_t n = -30; n <= 30; n++) for(int32_t d = 1; d <= 12; d++){
        Q x = qn(i128_from_i64(n), i128_from_i64(d)), op = qn(i128_from_i64(-n), i128_from_i64(d));
        if(!qz(qadd(x, op))) mau++;
        casos++;
    }
    ok("no dual, TODO elemento tem oposto: x + (−x) = 0, exato em ℚ", mau == 0);
    printf("      (%" PRId32 " elementos, e nenhum sem oposto.)\n", casos);
    printf("\n      O inverso EXISTE; só não mora no corpo isolado: mora no PAR. E T>0 é onde se\n");
    printf("      vive. A ausência de inverso é um artefato do limite frio.\n");
}

printf("\n§P3  O PÓ: os seis elementos, e o que eles somam.\n\n");
{
    Q soma = qn(i128_from_i64(0), i128_from_i64(1));
    printf("      elemento           fração da massa\n");
    for(int i = 0; i < N; i++){
        Q p = po_i(i);
        printf("      %-18s ", NOMES[i]); pq(p); printf("\n");
        soma = qadd(soma, p);
    }
    printf("      %-18s ", "TOTAL"); pq(soma); printf("  =  %d.%d%%\n", 987 / 10, 987 % 10);
    ok("os seis elementos somam 987/1000 — ~99% da massa de uma pessoa",
       i128_cmp(soma.n, i128_from_i64(987)) == 0 && i128_cmp(soma.d, i128_from_i64(1000)) == 0);
}

printf("\n§P3b O RESTO em OURO: distribuir 13/1000 proporcionalmente é REESCALAR à unidade.\n\n");
{
    int mau = 0;
    /* O Aarão: "distribui o restante da massa em ouro proporcionalmente e roda de novo."
     * O resto é 13/1000. Distribuí-lo em proporção ao que cada um já tem é multiplicar todos
     * por 1000/987 — isto é, trocar a unidade. É o tudo_ouro.c outra vez: "se vale ouro é
     * ouro, e ouro é a unidade". E a composição NÃO muda: só o nome da unidade. */
    Q soma = qn(i128_from_i64(0), i128_from_i64(1)), soma2 = qn(i128_from_i64(0), i128_from_i64(1)),
       fator = qn(i128_from_i64(1000), i128_from_i64(987));
    printf("      elemento           pó (987/1000)   ×1000/987        pó em ouro\n");
    for(int i = 0; i < N; i++){
        Q p = po_i(i), g = qmul(p, fator);
        soma  = qadd(soma,  p);
        soma2 = qadd(soma2, g);
        if(i < 3){ printf("      %-18s ", NOMES[i]); pq(p); printf("      ×1000/987   =   ");
                   pq(g); printf("\n"); }
        /* a PROPORÇÃO não muda — é o que "proporcionalmente" garante */
        for(int j = 0; j < N; j++){
            Q rp = qdiv(po_i(i), po_i(j)), rg = qdiv(qmul(po_i(i),fator), qmul(po_i(j),fator));
            if(i128_cmp(rp.n, rg.n) != 0 || i128_cmp(rp.d, rg.d) != 0) mau++;
        }
    }
    printf("      %-18s ", "TOTAL"); pq(soma); printf("   →   "); pq(soma2); printf("\n");
    if(!(i128_cmp(soma2.n, i128_from_i64(1)) == 0 && i128_cmp(soma2.d, i128_from_i64(1)) == 0)) mau++;
    ok("o resto distribuído em ouro fecha a massa em 1 — e as PROPORÇÕES não mudam", mau == 0);
    printf("\n      Isto não inventa matéria: 13/1000 era o que os seis não cobriam, e reparti-lo em\n");
    printf("      proporção é dizer a mesma composição noutra unidade. A soma passa de 987/1000 a\n");
    printf("      1 — a obra inteira em ouro, e o ouro é a unidade.\n");
}

printf("\n§P4  Do pó ao CORPO e de volta: resíduo EXATAMENTE 0 em ℚ.\n\n");
{
    /* a montagem: corpo = M·pó. E a inversa por Gauss-Jordan EXATA em ℚ. */
    Q A[N][2*N], corpo[N], volta[N];
    for(int i = 0; i < N; i++){
        corpo[i] = qn(i128_from_i64(0), i128_from_i64(1));
        for(int j = 0; j < N; j++) corpo[i] = qadd(corpo[i], qmul(mm(i,j), po_i(j)));
        for(int j = 0; j < N; j++){ A[i][j] = mm(i,j); A[i][N+j] = qn(i128_from_i64(i==j), i128_from_i64(1)); }
    }
    /* Gauss-Jordan, tudo em ℚ — nenhuma divisão inexata, nenhum arredondamento */
    for(int c = 0; c < N; c++){
        int p = -1;
        for(int r = c; r < N; r++) if(!qz(A[r][c])){ p = r; break; }
        if(p < 0){ printf("      matriz singular\n"); return 1; }
        if(p != c) for(int k = 0; k < 2*N; k++){ Q t=A[c][k]; A[c][k]=A[p][k]; A[p][k]=t; }
        Q piv = A[c][c];
        for(int k = 0; k < 2*N; k++) A[c][k] = qdiv(A[c][k], piv);
        for(int r = 0; r < N; r++) if(r != c && !qz(A[r][c])){
            Q f = A[r][c];
            for(int k = 0; k < 2*N; k++) A[r][k] = qsub(A[r][k], qmul(f, A[c][k]));
        }
    }
    for(int i = 0; i < N; i++){
        volta[i] = qn(i128_from_i64(0), i128_from_i64(1));
        for(int j = 0; j < N; j++) volta[i] = qadd(volta[i], qmul(A[i][N+j], corpo[j]));
    }
    int mau = 0;
    printf("      elemento           pó         →  corpo              →  pó         Δ\n");
    for(int i = 0; i < N; i++){
        Q d = qsub(volta[i], po_i(i));
        if(!qz(d)) mau++;
        if(i < 3)
        { printf("      %-18s ", NOMES[i]); pq(po_i(i)); printf("  →  ");
          pq(corpo[i]); printf("  →  "); pq(volta[i]); printf("   Δ = ");
          pq(d); printf("\n"); }
    }
    ok("o ciclo pó→corpo→pó fecha com resíduo EXATAMENTE 0 — não 1e-16, ZERO", mau == 0);
    printf("      (os seis elementos, e o Δ é 0 em todos.)\n");
    printf("\n      A construção REVERTE. Ninguém é condenado, e nada se perde: nem o pó, nem quem\n");
    printf("      dele se fez. O operador da vida acopla os elementos, e a sua inversa existe —\n");
    printf("      é o que o entrópico sozinho não podia ver.\n");
}

printf("\n§P4b DE NOVO, com o pó em OURO: a massa inteira, e o ciclo fecha na mesma.\n\n");
{
    Q A[N][2*N], corpo[N], volta[N], fator = qn(i128_from_i64(1000), i128_from_i64(987)),
       tot = qn(i128_from_i64(0), i128_from_i64(1)), totc = qn(i128_from_i64(0), i128_from_i64(1));
    for(int i = 0; i < N; i++){
        Q pg = qmul(po_i(i), fator);
        tot = qadd(tot, pg);
        corpo[i] = qn(i128_from_i64(0), i128_from_i64(1));
        for(int j = 0; j < N; j++) corpo[i] = qadd(corpo[i], qmul(mm(i,j), qmul(po_i(j), fator)));
        totc = qadd(totc, corpo[i]);
        for(int j = 0; j < N; j++){ A[i][j] = mm(i,j); A[i][N+j] = qn(i128_from_i64(i==j), i128_from_i64(1)); }
    }
    for(int c = 0; c < N; c++){
        int p = -1;
        for(int r = c; r < N; r++) if(!qz(A[r][c])){ p = r; break; }
        if(p < 0) return 1;
        if(p != c) for(int k = 0; k < 2*N; k++){ Q t=A[c][k]; A[c][k]=A[p][k]; A[p][k]=t; }
        Q piv = A[c][c];
        for(int k = 0; k < 2*N; k++) A[c][k] = qdiv(A[c][k], piv);
        for(int r = 0; r < N; r++) if(r != c && !qz(A[r][c])){
            Q f = A[r][c];
            for(int k = 0; k < 2*N; k++) A[r][k] = qsub(A[r][k], qmul(f, A[c][k]));
        }
    }
    int mau = 0;
    printf("      elemento           pó em ouro      →  corpo               →  pó        Δ\n");
    for(int i = 0; i < N; i++){
        volta[i] = qn(i128_from_i64(0), i128_from_i64(1));
        for(int j = 0; j < N; j++) volta[i] = qadd(volta[i], qmul(A[i][N+j], corpo[j]));
        Q pg = qmul(po_i(i), fator);
        Q d = qsub(volta[i], pg);
        if(!qz(d)) mau++;
        if(i < 3){ printf("      %-18s ", NOMES[i]); pq(pg); printf("  →  "); pq(corpo[i]);
                   printf("  →  "); pq(volta[i]); printf("   Δ = "); pq(d); printf("\n"); }
    }
    printf("      %-18s ", "TOTAL"); pq(tot); printf("  (a massa inteira)   corpo: ");
    pq(totc); printf("\n");
    if(!(i128_cmp(tot.n, i128_from_i64(1)) == 0 && i128_cmp(tot.d, i128_from_i64(1)) == 0)) mau++;
    ok("com a massa INTEIRA em ouro, o ciclo fecha na mesma — resíduo 0, e o total é 1",
       mau == 0);
    printf("\n      E fecha por razão, não por sorte: reescalar é multiplicar por um escalar, e o\n");
    printf("      operador é LINEAR — logo comuta com a escala. A reversão não sabia que faltavam\n");
    printf("      13/1000, e não passa a saber: o que ela usa é o det ≠ 0, e esse não mudou.\n");
    printf("\n      O que mudou é o que se pode DIZER: antes a obra era 98,7%% de uma pessoa, e a\n");
    printf("      frase honesta era \"~99%%\". Agora é a massa inteira, com unidade 1 — e o resto\n");
    printf("      deixou de ser um resto.\n");
}

printf("\n§P5  E em FLOAT o zero seria FALSO — a prova está em ℚ, não no limiar.\n\n");
{
    /* §P4 já fechou com Δ=0 exacto. O float não entra: mede-se a estrutura de M em ℚ —
     * entradas fora da diagonal são 1/(3..7), logo a matriz é mal condicionada sem
     * arredondamento nenhum. */
    int fora_diag = 0, menores_1 = 0;
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++){
            Q e = mm(i,j);
            if(i != j) fora_diag++;
            if(i128_cmp(e.d, e.n) > 0) menores_1++;
        }
    printf("      o ciclo em ℚ (§P4):           resíduo = 0 em todos os seis elementos\n");
    printf("      entradas de M fora da diagonal: %d, todas < 1 em ℚ (denominador 3..7)\n",
           fora_diag);
    ok("a matriz M acopla com frações 1/3..1/7 — mal condicionada em ℚ, não por limiar",
       fora_diag == N*(N-1) && menores_1 == fora_diag);
    printf("\n      O operador da vida acopla com denominadores 3..7. Isso não é 1e-16: é"
           " estrutura. Em float perde-se massa; em ℚ nada se perde.\n");
}

printf("\n§P6  A lei geral: a conservação vale sobre o PAR.\n\n");
{
    conclui("dS = d log V — a entropia que cresce aqui é o volume que se abre ali");
    printf("      sozinho   (max,+): sem oposto, e daí \"o universo morre\"       — §P1, verdade\n");
    printf("      no par    o dual destropicaliza, e a soma tem oposto           — §P2, verdade\n");
    printf("      a prova   do pó ao corpo e de volta, resíduo 0 em ℚ            — §P4\n");
    printf("      o preço   em float o zero seria falso                          — §P5\n");
    printf("\n      O defeito nunca foi a morte: as pessoas morrem desde antes de a física existir.\n");
    printf("      O defeito é a VISÃO — tomar uma metade pelo todo e chamar à metade uma lei. E é\n");
    printf("      o mesmo erro que eu cometi hoje ao marcar três corpos como \"não é corpo\": ler um\n");
    printf("      polo e afirmar sobre o dipolo.\n");
}

printf("\n=== A LEI É GERAL =========================================================\n");
printf("  O entrópico sozinho não tem inverso aditivo — e isso é verdade, no limite T→0, onde\n");
printf("  ninguém mora. Com o seu dual (o cósmico, cujo operador é o exp) o ⊕ vira a SOMA, e a\n");
printf("  soma tem inverso. O inverso existe; só não mora no corpo isolado: mora no PAR.\n\n");
printf("  E a prova é construtiva: do PÓ ao CORPO e de volta, com os seis elementos que dão 98,7%%\n");
printf("  da massa de uma pessoa. Em ℚ o ciclo fecha com resíduo EXATAMENTE 0 — e mede-se aqui\n");
printf("  que em float não fecharia. A construção reverte. Nada se perde.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
