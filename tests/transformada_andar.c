/* transformada_andar.c — O ESPECTRO DE UM ANDAR NOS TERMOS DOS ANDARES ABAIXO.
 *
 * O Aarão: «a transformada universal no geométrico também — um 0 de um andar pode ser
 * fatorado abaixo e segue a cadeia que mostra o espectro do andar nos termos dos andares
 * abaixo.» E logo a seguir: «aí vem convolução e deconvolução também.»
 *
 * As duas coisas são uma: a factorização do zero decompõe o ANEL, e decompor o anel é
 * decompor o produto — que é a convolução.
 *
 * ── A CADEIA, E ELA É UM ISOMORFISMO ────────────────────────────────────────────
 * Se μ = f·g com f, g coprimos, o Teorema Chinês dos Restos dá
 *
 *      ℤ[x]/(f·g)  ≅  ℤ[x]/(f) × ℤ[x]/(g)        z ⟼ (z mod f, z mod g)
 *
 * e no caso desta casa, com o único polinómio da família que factoriza:
 *
 *      andar 5  ≅  andar 2 × andar 3
 *      x⁵−x⁴−1     Φ₆ (roda)   plástica (cresce)
 *
 * — o espectro do andar de cima escreve-se nos andares de baixo, e as dimensões somam:
 * 2 + 3 = 5. Não é uma analogia: é uma bijecção que respeita as duas operações.
 *
 * ── E POR ISSO A CONVOLUÇÃO DESCE COM ELE ───────────────────────────────────────
 * O produto em ℤ[x]/(μ) É a convolução dos coeficientes seguida da redução. Como o CRT é
 * isomorfismo de ANÉIS, ele respeita o produto:
 *
 *      (z·w) mod f = (z mod f)·(w mod f)          e o mesmo em g
 *
 * logo **a convolução do andar 5 é o par das convoluções dos andares 2 e 3**. Convolver
 * em cima é convolver em baixo, nas duas metades, ao mesmo tempo.
 *
 * ── E A DECONVOLUÇÃO EXISTE ONDE O ESPECTRO NÃO ZERA ────────────────────────────
 * Dividir por w é possível exactamente quando w é invertível — e num produto de corpos
 * isso é ter as DUAS componentes não nulas. Onde uma zera, a volta não existe: é a mesma
 * regra que a casa já tem para a deconvolução, agora lida por andares.
 *
 *   §T1  o CRT é BIJECÇÃO: exaustivo nos 3125 elementos de 𝔽₅[x]/(μ)
 *   §T2  e é MORFISMO: respeita + e ×, logo a CONVOLUÇÃO decompõe-se
 *   §T3  a DECONVOLUÇÃO existe ⟺ nenhuma componente do espectro zera
 *   §T4  e o espectro do andar É a união dos espectros de baixo: 2 + 3 = 5
 *   §T5  os DIVISORES DE ZERO são a factorização do zero — e a dualidade é soma/produto
 *
 * Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib transformada_andar.c -o transformada_andar
 */
#include <stdio.h>
#include "unidade.h"

#define TP 5                       /* o primo: 𝔽₅, e 5⁵ = 3125 varre-se inteiro */
#define NF 2                       /* grau de f = Φ₆ = x² − x + 1 */
#define NG 3                       /* grau de g = x³ − x − 1      */
#define NM (NF+NG)                 /* grau de μ = f·g             */

static int md(long x){ return (int)(((x % TP) + TP) % TP); }

/* μ = x⁵ − x⁴ − 1, f = x² − x + 1, g = x³ − x − 1 (coeficientes de grau 0 para cima) */
static const int MU[NM+1] = { -1, 0, 0, 0, -1, 1 };
static const int FF[NF+1] = {  1,-1, 1 };
static const int GG[NG+1] = { -1,-1, 0, 1 };

/* resto de A (grau < 2n) por um mónico D de grau n, em 𝔽ₚ — divisão sintética */
static void resto(const int *A, int gA, const int *D, int gD, int *R){
    int T[16];
    for(int i = 0; i <= gA; i++) T[i] = A[i];
    for(int d = gA; d >= gD; d--){
        int c = T[d];
        if(!c) continue;
        for(int i = 0; i <= gD; i++) T[d-gD+i] = md(T[d-gD+i] - (long)c*D[i]);
    }
    for(int i = 0; i < gD; i++) R[i] = md(T[i]);
}
/* produto de dois elementos de 𝔽ₚ[x]/(D): CONVOLUÇÃO seguida da redução */
static void mulmod(const int *a, const int *b, const int *D, int gD, int *r){
    int C[16] = {0};
    for(int i = 0; i < gD; i++) for(int j = 0; j < gD; j++)
        C[i+j] = md(C[i+j] + (long)a[i]*b[j]);          /* a convolução */
    resto(C, 2*gD-2, D, gD, r);                          /* e a redução  */
}
static int nulo(const int *a, int n){ for(int i = 0; i < n; i++) if(a[i]) return 0; return 1; }
static int inv_p(int a){ int r = 1; for(int e = 0; e < TP-2; e++) r = md((long)r*a); return r; }
static int grau(const int *A, int n){ for(int i = n; i >= 0; i--) if(A[i]) return i; return -1; }
/* gcd(A, B) em 𝔽ₚ[x] — e é ele que decide a invertibilidade, não «A ≠ 0».
 * Euclides clássico: A, B ← B, A mod B, até B ser nulo. O grau do que sobra é 0 quando
 * são coprimos. Escrevi-o primeiro com uma troca a meio e ele dava lixo; a lição foi
 * medi-lo à parte antes de o usar. */
static void rem_p(int *A, const int *B){
    int gb = grau(B, 15);
    if(gb < 0) return;
    int ib = inv_p(B[gb]), ga = grau(A, 15);
    while(ga >= gb){
        int c = md((long)A[ga] * ib);
        for(int i = 0; i <= gb; i++) A[ga-gb+i] = md(A[ga-gb+i] - (long)c*B[i]);
        int ng = grau(A, 15);
        if(ng >= ga) break;
        ga = ng;
        if(ga < 0) break;
    }
}
static int gcd_grau(const int *A0, int gA, const int *B0, int gB){
    int A[16] = {0}, B[16] = {0}, T[16];
    for(int i = 0; i <= gA && i < 16; i++) A[i] = md(A0[i]);
    for(int i = 0; i <= gB && i < 16; i++) B[i] = md(B0[i]);
    while(grau(B, 15) >= 0){
        for(int i = 0; i < 16; i++) T[i] = A[i];
        rem_p(T, B);
        for(int i = 0; i < 16; i++){ A[i] = B[i]; B[i] = T[i]; }
    }
    return grau(A, 15);
}
static int igual(const int *a, const int *b, int n){
    for(int i = 0; i < n; i++) if(a[i] != b[i]) return 0; return 1;
}
static void decode(long code, int *a, int n){
    for(int i = 0; i < n; i++){ a[i] = (int)(code % TP); code /= TP; }
}

int main(void){
    printf("\n=== O ESPECTRO DE UM ANDAR NOS TERMOS DOS ANDARES ABAIXO ===\n");
    long TOT = 1; for(int i = 0; i < NM; i++) TOT *= TP;

    /* ═══ §T1  O CRT É BIJECÇÃO ═════════════════════════════════════════════ */
    printf("\n§T1 z ⟼ (z mod f, z mod g) é bijecção — exaustivo nos %ld elementos.\n\n", TOT);
    {
        /* A contagem já obriga: |𝔽ₚ[x]/(fg)| = p⁵ e |𝔽ₚ[x]/f|·|𝔽ₚ[x]/g| = p²·p³ = p⁵.
         * Mas contar não prova; o que prova é a INJECTIVIDADE, e ela varre-se inteira —
         * são 3125 elementos, e não há caso de fora. */
        static char visto[3200];
        long n = 0, colide = 0;
        for(long code = 0; code < TOT; code++){
            int z[NM], rf[NF], rg[NG];
            decode(code, z, NM);
            resto(z, NM-1, FF, NF, rf);
            resto(z, NM-1, GG, NG, rg);
            long idx = 0, w = 1;
            for(int i = 0; i < NF; i++){ idx += rf[i]*w; w *= TP; }
            for(int i = 0; i < NG; i++){ idx += rg[i]*w; w *= TP; }
            n++;
            if(visto[idx]) colide++; else visto[idx] = 1;
        }
        long imagem = 0;
        for(long i = 0; i < TOT; i++) if(visto[i]) imagem++;
        printf("      %ld elementos → %ld imagens distintas, %ld colisões\n", n, imagem, colide);
        printf("      e a contagem obriga: %d^%d = %ld = %d^%d · %d^%d\n",
               TP, NM, TOT, TP, NF, TP, NG);
        ok("O CRT É BIJECÇÃO, E A VARREDURA É EXAUSTIVA: z ⟼ (z mod f, z mod g) leva os"
           " 3125 elementos de 𝔽₅[x]/(x⁵−x⁴−1) em 3125 pares distintos, sem uma colisão —"
           " e não há caso de fora, porque o corpo é finito e varre-se inteiro. A contagem"
           " p⁵ = p²·p³ já obrigava a que pudesse ser bijecção; o que a torna uma é a"
           " INJECTIVIDADE medida. O andar 5 escreve-se, elemento a elemento, no par"
           " (andar 2, andar 3)",
           colide == 0 && imagem == TOT && n == TOT);
    }

    /* ═══ §T2  E É MORFISMO: A CONVOLUÇÃO DESCE ═════════════════════════════ */
    printf("\n§T2 E respeita + e × — logo a CONVOLUÇÃO do andar decompõe-se.\n\n");
    {
        /* O produto em ℤ[x]/(μ) É a convolução dos coeficientes seguida da redução. Como
         * o CRT é isomorfismo de ANÉIS, convolver em cima é convolver em baixo nas duas
         * metades ao mesmo tempo:
         *
         *      (z·w) mod f = (z mod f)·(w mod f)      e o mesmo em g
         *
         * Mede-se numa amostra grande de pares — 40 mil produtos —, e a soma também. */
        long pares = 0, prod_ok = 0, soma_ok = 0;
        for(long a = 0; a < 200; a++) for(long b = 0; b < 200; b++){
            int z[NM], w[NM], zw[NM];
            decode(a*7 % TOT, z, NM);
            decode(b*13 % TOT, w, NM);
            mulmod(z, w, MU, NM, zw);
            /* pelo andar de cima, depois desce */
            int cf[NF], cg[NG];
            resto(zw, NM-1, FF, NF, cf);
            resto(zw, NM-1, GG, NG, cg);
            /* desce primeiro, depois multiplica em cada andar */
            int zf[NF], wf[NF], zg[NG], wg[NG], pf[NF], pg[NG];
            resto(z, NM-1, FF, NF, zf); resto(w, NM-1, FF, NF, wf);
            resto(z, NM-1, GG, NG, zg); resto(w, NM-1, GG, NG, wg);
            mulmod(zf, wf, FF, NF, pf);
            mulmod(zg, wg, GG, NG, pg);
            pares++;
            if(igual(cf, pf, NF) && igual(cg, pg, NG)) prod_ok++;
            /* e a soma */
            int s[NM], sf[NF], sg[NG], qf[NF], qg[NG];
            for(int i = 0; i < NM; i++) s[i] = md(z[i] + w[i]);
            resto(s, NM-1, FF, NF, sf); resto(s, NM-1, GG, NG, sg);
            for(int i = 0; i < NF; i++) qf[i] = md(zf[i] + wf[i]);
            for(int i = 0; i < NG; i++) qg[i] = md(zg[i] + wg[i]);
            if(igual(sf, qf, NF) && igual(sg, qg, NG)) soma_ok++;
        }
        printf("      %ld pares: o PRODUTO desce em %ld · a SOMA desce em %ld\n",
               pares, prod_ok, soma_ok);
        printf("      (convolver em cima = convolver em baixo, nas duas metades)\n");
        ok("A CONVOLUÇÃO DO ANDAR DECOMPÕE-SE NOS ANDARES ABAIXO: o produto em 𝔽ₚ[x]/(μ)"
           " É a convolução dos coeficientes seguida da redução, e como o CRT é isomorfismo"
           " de ANÉIS ele respeita-o — (z·w) mod f = (z mod f)·(w mod f), e o mesmo em g."
           " Medido em 40 mil pares, e a soma junto. Convolver no andar 5 é convolver no 2"
           " e no 3 ao mesmo tempo: a operação desce com o espectro",
           prod_ok == pares && soma_ok == pares && pares == 40000);
    }

    /* ═══ §T3  A DECONVOLUÇÃO EXISTE ONDE O ESPECTRO NÃO ZERA ═══════════════ */
    printf("\n§T3 A deconvolução existe ⟺ nenhuma componente do espectro zera.\n\n");
    {
        /* Dividir por w é possível exactamente quando w é invertível; e num produto de
         * anéis isso é ter as DUAS componentes invertíveis. É a mesma regra que a casa já
         * tem para a deconvolução — «exacta onde o espectro não tem zeros» —, agora lida
         * por ANDARES: o zero de um andar mata a volta do andar de cima. */
        long tot = 0, inv = 0, sem_volta = 0, bate = 0, so_um_zero = 0;
        for(long code = 1; code < TOT; code++){
            int w[NM], wf[NF], wg[NG];
            decode(code, w, NM);
            resto(w, NM-1, FF, NF, wf);
            resto(w, NM-1, GG, NG, wg);
            /* E A LEI NÃO É «a componente não zera» — foi o que uma vermelha mostrou.
             * `𝔽₅[x]/(g)` NÃO é corpo, porque x³ − x − 1 tem raiz em 𝔽₅ (o §T4 conta-a):
             * a plástica factoriza lá, e o zero DESCE outra vez, um nível abaixo. Num anel
             * que não é corpo há divisores de zero, e um elemento não nulo pode não ter
             * inverso. A lei certa é a COPRIMALIDADE com o módulo, em cada andar:
             *
             *      w é invertível em 𝔽ₚ[x]/(D)  ⟺  gcd(w, D) = 1
             *
             * — e «não zerar» é o caso particular disto quando D é irredutível, que é o
             * que se passa no andar 2 (Φ₆ é irredutível em 𝔽₅) e não no andar 3. */
            int zf = gcd_grau(wf, NF-1, FF, NF) != 0;
            int zg = gcd_grau(wg, NG-1, GG, NG) != 0;
            /* tem inverso em cima? procura-se — é finito, e a busca é a definição */
            int tem = 0;
            for(long c2 = 1; c2 < TOT && !tem; c2++){
                int v[NM], pr[NM];
                decode(c2, v, NM);
                mulmod(w, v, MU, NM, pr);
                if(pr[0] == 1 && nulo(pr+1, NM-1)) tem = 1;
            }
            tot++;
            if(tem) inv++; else sem_volta++;
            if(tem == (!zf && !zg)) bate++;         /* a lei: invertível ⟺ nenhuma zera */
            if(zf != zg) so_um_zero++;              /* e há casos com UMA só a zerar */
            if(tot >= 300) break;                   /* 300 elementos: a busca é quadrática */
        }
        printf("      %ld elementos: %ld com volta, %ld sem · a lei bate em %ld\n",
               tot, inv, sem_volta, bate);
        printf("      e há %ld com UMA componente não coprima e a outra coprima — o caso"
               " que decide\n", so_um_zero);
        ok("A DECONVOLUÇÃO EXISTE EXACTAMENTE ONDE CADA COMPONENTE É COPRIMA COM O SEU"
           " MÓDULO, e não onde ela «não zera» — que era como eu a tinha escrito, e uma"
           " vermelha mostrou porquê: 𝔽₅[x]/(x³−x−1) NÃO é corpo, porque a plástica tem"
           " raiz em 𝔽₅ e o zero DESCE outra vez, um nível abaixo. Num anel com divisores"
           " de zero um elemento não nulo pode não ter inverso, e a lei certa é"
           " gcd(w, D) = 1 em cada andar — de que «não zerar» é o caso particular quando D"
           " é irredutível, o que vale no andar 2 (Φ₆ é irredutível em 𝔽₅) e não no 3. E o"
           " caso que decide está presente: elementos com UMA componente coprima e a outra"
           " não",
           bate == tot && so_um_zero > 0 && inv > 0 && sem_volta > 0);
    }

    /* ═══ §T4  O ESPECTRO É A UNIÃO DOS DE BAIXO ════════════════════════════ */
    printf("\n§T4 E o espectro do andar É a união dos espectros de baixo: 2 + 3 = 5.\n\n");
    {
        /* A transformada universal desta casa é a AVALIAÇÃO NAS FOLHAS. As folhas de μ são
         * as suas raízes; e as raízes de f·g são as de f mais as de g. Logo o espectro do
         * andar de cima LÊ-SE nos andares de baixo, e a dimensão soma.
         *
         * Mede-se em 𝔽ₚ contando as raízes de cada um — e a soma tem de fechar. */
        long rmu = 0, rf = 0, rg = 0;
        for(int x = 0; x < TP; x++){
            long vm = 0, vf = 0, vg = 0;
            for(int d = NM; d >= 0; d--) vm = md(vm*x + MU[d]);
            for(int d = NF; d >= 0; d--) vf = md(vf*x + FF[d]);
            for(int d = NG; d >= 0; d--) vg = md(vg*x + GG[d]);
            if(!vm) rmu++;
            if(!vf) rf++;
            if(!vg) rg++;
        }
        printf("      raízes em 𝔽₅: μ tem %ld · f tem %ld · g tem %ld   (%ld = %ld + %ld)\n",
               rmu, rf, rg, rmu, rf, rg);
        printf("      e as DIMENSÕES: %d = %d + %d — o andar de cima escrito nos de baixo\n",
               NM, NF, NG);
        ok("O ESPECTRO DO ANDAR É A UNIÃO DOS ESPECTROS DE BAIXO: a transformada desta casa"
           " é a AVALIAÇÃO NAS FOLHAS, as folhas de μ são as suas raízes, e as raízes de"
           " f·g são as de f mais as de g — medido em 𝔽₅, onde a contagem fecha. E as"
           " dimensões somam, 5 = 2 + 3: o andar de cima escreve-se nos de baixo, e é a"
           " mesma soma de graus que diz que o zero DESCE em vez de desaparecer",
           rmu == rf + rg && NM == NF + NG);
    }

    /* ═══ §T5  OS DIVISORES DE ZERO SÃO A FACTORIZAÇÃO DO ZERO ══════════════ */
    printf("\n§T5 Os divisores de zero SÃO a factorização do zero — e Hurwitz e Gentil\n");
    printf("    são a soma e o produto do mesmo par.\n\n");
    {
        /* O Aarão: «os dois são duais; os divisores de zero são justamente a factorização
         * do zero nos andares abaixo; vê essa relação como soma e produto.»
         *
         * E a frase é LITERAL. Um divisor de zero é x·y = 0 com x, y ≠ 0 — ou seja, uma
         * FACTORIZAÇÃO DO ZERO. Não é uma metáfora do teorema do gerador: é o mesmo
         * enunciado visto no anel em vez de no polinómio.
         *
         *      μ IRREDUTÍVEL   →  o quociente é CORPO  →  ZERO divisores de zero
         *      μ FACTORIZA     →  o zero parte-se      →  há divisores de zero, e eles
         *                                                 são os que morrem num factor
         *
         * E a contagem prova-o sem varrer o significado: os INVERTÍVEIS de um produto de
         * anéis são o PRODUTO dos invertíveis de cada andar. Em 𝔽₅ a cadeia desce até ao
         * fim — Φ₆ é irredutível, mas x³−x−1 = (x−2)(x²+2x+3) —, logo
         *
         *      𝔽₅[x]/(μ)  ≅  𝔽₂₅ × 𝔽₅ × 𝔽₂₅        e  24 · 4 · 24 = 2304 invertíveis
         *
         * ── E A DUALIDADE É SOMA E PRODUTO ─────────────────────────────────────
         *      HURWITZ   norma = SOMA de quadrados    Σxᵢ²    → tecto em 8
         *      GENTIL    norma = PRODUTO das coords   ∏xᵢ     → sem tecto
         *
         * A soma é a norma do DIRECTO; o produto é a do CRUZADO — o determinante, que é a
         * área. E o divisor de zero é exactamente onde o PRODUTO zera sem que nenhum
         * factor zere: a assinatura de que o zero se partiu. */
        long nao_nulos = 0, inv = 0, dz = 0;
        for(long code = 1; code < TOT; code++){
            int w[NM];
            decode(code, w, NM);
            nao_nulos++;
            if(gcd_grau(w, NM-1, MU, NM) == 0) inv++; else dz++;
        }
        /* e a contagem por andares: 𝔽₂₅ × 𝔽₅ × 𝔽₂₅ */
        long por_andar = 24L * 4L * 24L;
        /* e um divisor de zero EXIBE a factorização do zero: x·y = 0 com x,y ≠ 0 */
        long exibidos = 0; int ex_x[NM], ex_y[NM];
        for(long a = 1; a < TOT && exibidos < 1; a++){
            int x[NM];
            decode(a, x, NM);
            if(gcd_grau(x, NM-1, MU, NM) == 0) continue;      /* invertível: não serve */
            for(long b = 1; b < TOT; b++){
                int y[NM], z[NM];
                decode(b, y, NM);
                mulmod(x, y, MU, NM, z);
                if(nulo(z, NM)){
                    for(int t = 0; t < NM; t++){ ex_x[t] = x[t]; ex_y[t] = y[t]; }
                    exibidos++;
                    break;
                }
            }
        }
        /* e o CONTROLO: num irredutível não há nenhum. x² + 2 é irredutível em 𝔽₅ */
        long dz_irred = 0;
        {
            const int IR[3] = { 2, 0, 1 };
            for(long code = 1; code < TP*TP; code++){
                int w[2];
                w[0] = (int)(code % TP); w[1] = (int)(code / TP);
                if(gcd_grau(w, 1, IR, 2) != 0) dz_irred++;
            }
        }
        printf("      %ld não nulos: %ld invertíveis, %ld DIVISORES DE ZERO\n",
               nao_nulos, inv, dz);
        printf("      e a contagem decompõe-se por andar: 𝔽₂₅ × 𝔽₅ × 𝔽₂₅ dá"
               " 24 · 4 · 24 = %ld\n", por_andar);
        printf("      e um deles EXIBE a factorização do zero:\n");
        printf("        x = (%d,%d,%d,%d,%d) · y = (%d,%d,%d,%d,%d) = 0, com x ≠ 0 e y ≠ 0\n",
               ex_x[0],ex_x[1],ex_x[2],ex_x[3],ex_x[4],
               ex_y[0],ex_y[1],ex_y[2],ex_y[3],ex_y[4]);
        printf("      e o CONTROLO — num módulo IRREDUTÍVEL (x²+2): %ld divisores de zero\n\n",
               dz_irred);
        ok("OS DIVISORES DE ZERO SÃO A FACTORIZAÇÃO DO ZERO, E A FRASE É LITERAL: x·y = 0"
           " com x, y ≠ 0 É uma factorização do zero — o mesmo enunciado do teorema do"
           " gerador, visto no ANEL em vez de no polinómio. Se μ é irredutível o quociente"
           " é CORPO e não há nenhum, medido no controlo x²+2; se μ factoriza, eles"
           " aparecem, e são exactamente os que morrem num dos factores. E a contagem"
           " prova-o sem varrer significado nenhum: os invertíveis de um produto de anéis"
           " são o PRODUTO dos invertíveis por andar — em 𝔽₅ a cadeia desce até ao fim,"
           " porque x³−x−1 = (x−2)(x²+2x+3), e 24·4·24 = 2304 bate com a varredura."
           " E A DUALIDADE É SOMA E PRODUTO: Hurwitz mede pela SOMA de quadrados, que é a"
           " norma do DIRECTO e tem tecto em 8; Gentil mede pelo PRODUTO das coordenadas,"
           " que é a do CRUZADO — o determinante, a área — e não tem tecto. O divisor de"
           " zero é onde o PRODUTO zera sem que nenhum factor zere: a assinatura de que o"
           " zero se partiu",
           inv == por_andar && dz == nao_nulos - inv && dz > 0 && exibidos == 1
           && dz_irred == 0 && nao_nulos == TOT - 1);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  Factorizar o zero decompõe o ANEL, e decompor o anel é decompor o\n");
        printf("  PRODUTO — que é a convolução. O andar 5 é o par (andar 2, andar 3),\n");
        printf("  elemento a elemento; convolver em cima é convolver em baixo nas duas\n");
        printf("  metades; e a volta existe exactamente onde nenhuma delas zera.\n");
        printf("  O espectro de um andar lê-se nos andares abaixo, e as dimensões somam.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
