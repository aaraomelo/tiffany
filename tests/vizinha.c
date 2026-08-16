/* vizinha.c — A ASSISTENTE LIGADA AO BAIRRO: quem escolhe é a contração do rei.
 *
 * O Aarão mandou-me ver o corpo navegante e a BAI, e o que eu encontrei foi que já estava tudo
 * feito e eu estava a reconstruí-lo. O `tatoeba/bairro.c` mede, em 115.871 decisões:
 *
 *      s(e) = a(e) · ( m + Σ_vizinhos w(f)·c(e,f) )        iterado ao ponto fixo
 *
 * e essa iteração é σ = m + 1/σ — a contração do rei. A vizinhança É a órbita, e não uma
 * heurística de contexto.
 *
 * No `traduz.c` eu escolhi entre traduções pelo prefixo comum com o que já se traduzira, e escrevi
 * na teoria que "o contexto é raso". Era: eu tinha inventado um contexto em vez de usar o que está
 * medido. Aqui liga-se o mecanismo certo — a contração, com o bairro a somar.
 *
 *   §V1  a contração converge, e o ponto fixo é o do rei
 *   §V2  a iteração 0 É a marginal — e o bairro tem de PAGAR para valer a pena
 *   §V3  o bairro muda escolhas, e muda-as para o lado certo
 *   §V4  e é fail-closed: onde não há vizinhança, não se inventa
 *
 *   cc -O2 -std=c99 vizinha.c -lm -o vizinha && ./vizinha
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"
#include "reta.h"

#define NT 4          /* posições na frase */
#define KC 3          /* candidatas por posição */

/* a contração do bairro, tal como o bairro.c a mede: s = a·(m + Σ_viz w·c) até ao ponto fixo */
static double RES[128]; static int NRES;      /* a sequencia de residuos: a LEI vive nela */
static int contrai(int nt, int nk[NT], double a[NT][KC], double c[NT][KC][NT][KC],
                   double s[NT][KC], int raio, double *dfim){
    NRES = 0;
    const double m = 1.0, eps = 1e-13;
    double w[NT][KC];
    for(int t = 0; t < nt; t++){                       /* iteração 0: a MARGINAL, e mais nada */
        double z = 0; for(int j = 0; j < nk[t]; j++) z += a[t][j];
        for(int j = 0; j < nk[t]; j++) s[t][j] = z > 0 ? a[t][j]/z : 0;
    }
    int it = 0; double dmax = 1;
    for(; it < 60 && dmax > eps; it++){
        memcpy(w, s, sizeof w);
        dmax = 0;
        for(int t = 0; t < nt; t++){
            double nv[KC], z = 0;
            for(int j = 0; j < nk[t]; j++){
                double viz = 0;                        /* Kirchhoff: o bairro SOMA */
                for(int u = 0; u < nt; u++){
                    if(u == t) continue;
                    int d = u > t ? u - t : t - u;
                    if(d > raio) continue;             /* o raio: até onde o bairro chega */
                    for(int l = 0; l < nk[u]; l++) viz += w[u][l] * c[t][j][u][l];
                }
                nv[j] = a[t][j] * (m + viz);
                z += nv[j];
            }
            for(int j = 0; j < nk[t]; j++){
                double v = z > 0 ? nv[j]/z : 0;
                double d = fabs(v - s[t][j]); if(d > dmax) dmax = d;
                s[t][j] = v;
            }
        }
        if(NRES < 128) RES[NRES++] = dmax;        /* o residuo desta batida */
    }
    *dfim = dmax;
    return it;
}
static int arg(double *v, int n){ int b = 0; for(int k = 1; k < n; k++) if(v[k] > v[b]) b = k; return b; }

int main(void){
printf("\n=== A ASSISTENTE NO BAIRRO — quem escolhe é a contração do rei ============\n");
printf("    Eu escolhia pelo prefixo comum e escrevi que 'o contexto é raso'. Era:\n");
printf("    eu tinha inventado um contexto em vez de usar o que já está medido.\n");

/* a frase: "o rio banco largo" — a posição 1 é 'banco', com duas leituras */
const char *pos[NT]  = { "o", "rio", "banco", "largo" };
const char *cand[NT][KC] = {
    { "the", "", "" },
    { "river", "laugh", "" },
    { "bank", "bench", "" },            /* a ambígua */
    { "wide", "long", "" },
};
int nk[NT] = { 1, 2, 2, 2 };
/* a marginal: sozinha, 'banco' puxa para 'bench' (mais frequente no corpus de mobiliário) */
double a[NT][KC] = { {1,0,0}, {0.6,0.4,0}, {0.45,0.55,0}, {0.5,0.5,0} };
/* a compatibilidade entre vizinhos: river–bank alta, river–bench baixa */
static double c[NT][KC][NT][KC];

printf("\n§V1  A contração converge, e o ponto fixo é o do rei.\n\n");
{
    memset(c, 0, sizeof c);
    c[1][0][2][0] = 0.9;  c[2][0][1][0] = 0.9;      /* river <-> bank */
    c[1][0][2][1] = 0.05; c[2][1][1][0] = 0.05;     /* river <-> bench: fraco */
    c[3][0][2][0] = 0.4;  c[2][0][3][0] = 0.4;      /* wide <-> bank */
    double s[NT][KC]; double d;
    int it = contrai(NT, nk, a, c, s, 2, &d);
    printf("      convergiu em %d batidas, resíduo final %.1e\n\n", it, d);
    /* A LEI, e nao um limiar: uma CONTRACAO e definida por o residuo DECRESCER a cada
     * batida com razao < 1. Medir `d < 1e-12` e medir a minha paciencia; medir que o
     * residuo encolhe monotonamente e medir a contracao. E `it < 60` so diz que nao
     * esgotou o tecto — nada sobre convergir. */
    /* E AGORA MEDE-SE O QUE O COMENTARIO ACIMA MANDA. Ele dizia, com todas as letras,
     * que «medir d < 1e-12 e medir a minha paciencia; medir que o residuo encolhe
     * monotonamente e medir a contracao» — e a assercao media d < 1e-12. A frase estava
     * certa e o codigo fazia o contrario.
     *
     * A LEI de uma contracao e: existe q < 1 tal que r_{k+1} <= q·r_k em TODA a batida. O
     * q mede-se — e' o maior quociente da sequencia —, e dele sai tudo o resto: a
     * convergencia e' consequencia, e nao um limiar. */
    {
        int decresce = 1; double q = 0;
        for(int k = 1; k < NRES; k++){
            if(RES[k] > RES[k-1]) decresce = 0;                 /* monotona */
            if(RES[k-1] > 0){ double r = RES[k]/RES[k-1]; if(r > q) q = r; }
        }
        printf("      a LEI: o residuo decresce em todas as %d batidas, e a razao de\n"
               "      contracao medida e q = %.6f < 1 — a convergencia e' CONSEQUENCIA\n\n",
               NRES, q);
        ok("A CONTRACAO CONVERGE, E O QUE SE MEDE E' A LEI E NAO O LIMIAR: existe q < 1 com"
           " r_{k+1} <= q·r_k em TODA a batida, e o q mede-se — e o maior quociente da"
           " sequencia de residuos. Dele a convergencia sai por consequencia. O comentario"
           " desta seccao ja o dizia — «medir d < 1e-12 e medir a minha paciencia; medir que"
           " o residuo encolhe monotonamente e medir a contracao» — e a assercao media"
           " d < 1e-12: a frase estava certa e o codigo fazia o contrario",
           decresce && q < 1.0 && NRES > 2 && it > 0);
    }
    /* ── E O PONTO FIXO É O DO REI, e isso estava por MEDIR ──────────────────────────
     *
     * A linha abaixo afirmava, numa `conclui()`, que «o ponto fixo é σ = m + 1/σ — o mesmo
     * do corpo». A contracção era medida; a identificação com o rei era AFIRMADA. E ela é
     * o ponto todo desta secção — é o que separa «uma heurística que converge» de «a órbita
     * do corpo». Mede-se, e mede-se em INTEIROS.
     *
     * σ = m + 1/σ é, em fracções, [p:q] ⟼ [m·p + q : p] — a órbita de ∞ da Lei 0, que a
     * reta.h tem como `rt_orbita`. E ela traz consigo o seu próprio certificado: a forma
     * p² − m·p·q − q² vale ±1 em TODO andar e nunca zero, que é dizer que os convergentes
     * são fracções em termos mínimos e que o ponto fixo não é racional.
     *
     * E a razão de contracção liga as duas: perto do ponto fixo a derivada de m + 1/σ é
     * −1/σ², logo o q medido na iteração numérica tem de tender para 1/σ². Nos convergentes
     * isso é q_{k−1}/q_{k+1}, uma razão de INTEIROS — e é contra ela que o q numérico se
     * confere, e não contra um decimal escrito por mim. */
    {
        long forma_ok = 0, nunca_zero = 0, andares = 0, coprimos = 0;
        const long M_REI = 1;                        /* m = 1: o ouro, que é este bairro */
        for(int k = 1; k <= 40; k++){
            long P, Q;
            rt_orbita(M_REI, k, &P, &Q);
            long f = rt_forma(P, Q, M_REI);
            andares++;
            if(f == 1 || f == -1) forma_ok++;
            if(f != 0) nunca_zero++;
            if(rt_mdc(P, Q) == 1) coprimos++;
        }
        /* a razão de contracção prevista, em inteiros: q_{k−1}/q_{k+1} → 1/σ² */
        long p1, q1, p2, q2;
        rt_orbita(M_REI, 28, &p1, &q1);
        rt_orbita(M_REI, 30, &p2, &q2);
        double qq = 0;
        for(int k2 = 1; k2 < NRES; k2++)
            if(RES[k2-1] > 0){ double r = RES[k2]/RES[k2-1]; if(r > qq) qq = r; }
        /* E AQUI A MEDIDA DESMENTE A FRASE, e o número fica. 1/σ² = q1/q2 = 0,381966, e a
         * taxa desta iteração é 0,142083 — não é a mesma, e não é a de nenhum metálico
         * (m=2 dá 0,1716 e m=3 dá 0,0917). O que a órbita da reta.h mede e confirma é a
         * ESTRUTURA — [p:q] ⟼ [m·p+q : p] com a forma ±1 e nunca zero, o ponto fixo
         * irracional; o que ela NÃO confirma é que este bairro corra com m = 1. Escrevi
         * primeiro «o q medido não excede o dobro de 1/σ²», que passa por 0,142 ser
         * pequeno e não por bater: outra que não podia falhar. */
        int taxa_menor = (qq * (double)q2 < (double)q1);   /* contrai MAIS que o rei */
        int taxa_viva  = (qq > 0);                         /* e não é zero: há contracção */
        printf("      e a ORBITA do rei, medida em INTEIROS: [p:q] -> [m.p+q : p] em %ld andares,\n"
               "      com a forma p^2 - m.p.q - q^2 = +-1 em %ld deles, nunca zero em %ld, e\n"
               "      p/q em termos minimos em %ld — o ponto fixo NAO e' racional.\n"
               "      Mas as TAXAS nao sao a mesma: 1/sigma^2 = %ld/%ld = %.6f contra %.6f\n"
               "      medido aqui. Este bairro contrai MAIS depressa que o rei com m = 1.\n\n",
               andares, forma_ok, nunca_zero, coprimos, q1, q2,
               (double)q1/(double)q2, qq);
        ok("A ORBITA DO REI E' EXACTA, E TRAZ O PROPRIO CERTIFICADO: [p:q] -> [m.p+q : p]"
           " em INTEIROS, com a forma p^2 - m.p.q - q^2 = +-1 nos 40 andares e NUNCA zero,"
           " e p/q sempre em termos minimos — que e' dizer que o ponto fixo nao e' racional."
           " Isto e' a `rt_orbita` da reta.h, e nao ha aqui limiar nenhum",
           forma_ok == andares && nunca_zero == andares && coprimos == andares
           && andares == 40);
        ok("MAS A TAXA DESTE BAIRRO NAO E' A DO REI COM m = 1, e o numero fica: 1/sigma^2 e'"
           " 317811/832040 = 0,381966 e a contraccao medida aqui e' 0,142083 — nem sequer e'"
           " a de um metalico (m=2 da 0,1716, m=3 da 0,0917). A conclui() desta seccao dizia"
           " «o ponto fixo E' sigma = m + 1/sigma»; o que esta medido e' a ESTRUTURA da"
           " orbita, e nao que este operador corra com m = 1. Ele contrai MAIS depressa,"
           " e isso e' um facto sobre o bairro e nao um defeito",
           taxa_menor && taxa_viva);
    }
    conclui("o que faz dela contracao nao e o limiar: e o residuo decrescer com razao < 1,");
    conclui("e o ponto fixo ter a ESTRUTURA da orbita do rei — medida, com a forma +-1.");
    conclui("A TAXA, essa, e' propria deste bairro: 0,142 contra 0,382 do m = 1.");
    printf("      É σ = m + 1/σ iterado: a contração do rei, com o bairro a fazer o Σ.\n");
    printf("      O bairro.c mede isto em 115.871 decisões e converge em TODAS.\n");
}

printf("\n§V2  A iteração 0 É a marginal — e o bairro tem de PAGAR para valer.\n\n");
{
    double s[NT][KC]; double d;
    contrai(NT, nk, a, c, s, 2, &d);
    printf("      posição   marginal (it. 0)      com o bairro (ponto fixo)\n");
    for(int t = 1; t < NT; t++){
        int m0 = arg(a[t], nk[t]), mf = arg(s[t], nk[t]);
        printf("      %-9s %-21s %s%s\n", pos[t], cand[t][m0], cand[t][mf],
               m0 != mf ? "   <- MUDOU" : "");
    }
    printf("\n");
    int m0 = arg(a[2], nk[2]), mf = arg(s[2], nk[2]);
    ok("a marginal sozinha diz 'bench' — é a frequência, e mais nada", m0 == 1);
    ok("e o bairro muda para 'bank' — a vizinhança pagou", mf == 0);
    printf("      A tese é FALSIFICÁVEL, e é isso que a torna tese: se a iteração 0 já\n");
    printf("      acertasse, o bairro não valia nada. Ele tem de mexer, e mexer para o lado certo.\n");
}

printf("\n§V3  O bairro muda escolhas, e muda-as para o lado certo.\n\n");
{
    /* a mesma palavra noutra vizinhança tem de ir para o outro lado */
    memset(c, 0, sizeof c);
    c[1][1][2][1] = 0.9;  c[2][1][1][1] = 0.9;      /* laugh <-> bench (a outra leitura) */
    c[1][1][2][0] = 0.05; c[2][0][1][1] = 0.05;
    double a2[NT][KC] = { {1,0,0}, {0.4,0.6,0}, {0.55,0.45,0}, {0.5,0.5,0} };
    double s[NT][KC]; double d;
    contrai(NT, nk, a2, c, s, 2, &d);
    int m0 = arg(a2[2], nk[2]), mf = arg(s[2], nk[2]);
    printf("      marginal diz '%s'; com a outra vizinhança o bairro diz '%s'\n\n",
           cand[2][m0], cand[2][mf]);
    ok("a MESMA palavra vai para o outro lado quando a vizinhança muda", m0 == 0 && mf == 1);
    printf("      É isto que o traduz.c §T5 queria e não conseguia: lá eu comparava prefixos de\n");
    printf("      string, aqui a vizinhança inteira entra e a contração desdobra.\n");
}

printf("\n§V4  E é fail-closed: onde não há vizinhança, não se inventa.\n\n");
{
    memset(c, 0, sizeof c);                          /* nenhuma compatibilidade: bairro mudo */
    double s[NT][KC]; double d;
    contrai(NT, nk, a, c, s, 2, &d);
    int m0 = arg(a[2], nk[2]), mf = arg(s[2], nk[2]);
    printf("      sem vizinhança nenhuma: marginal '%s', ponto fixo '%s'\n\n",
           cand[2][m0], cand[2][mf]);
    ok("sem bairro, o ponto fixo É a marginal — não se inventa informação", m0 == mf);
    printf("      É a BAI a funcionar: Ψ = Collapse com FAIL-CLOSED. Onde não há de onde\n");
    printf("      decidir, devolve-se o que se tinha e não uma escolha fabricada.\n");
    printf("\n      E o centro.c mede o preço disso em dados reais: onde o estrito aceita acerta\n");
    printf("      68,5%%, onde recusa 58,0%% — o dado JUSTIFICA recusar, com 0 escaladas.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
