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

#define NT 4          /* posições na frase */
#define KC 3          /* candidatas por posição */

/* a contração do bairro, tal como o bairro.c a mede: s = a·(m + Σ_viz w·c) até ao ponto fixo */
static int contrai(int nt, int nk[NT], double a[NT][KC], double c[NT][KC][NT][KC],
                   double s[NT][KC], int raio, double *dfim){
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
    ok("a contração converge — e converge sozinha, sem Metrópolis", d < 1e-12 && it < 60);
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
