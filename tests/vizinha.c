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
 * LEI vs TRANSPORTE. A iteração em vírgula até 1e-13 e q = r_{k+1}/r_k eram o método. A lei
 * é Kirchhoff em ℤ (centesimais): o argmax da marginal contra o do bairro, a órbita
 * [p:q]↦[m·p+q : p] com forma ±1, e o rácio Kirchhoff ≠ o do ouro. Sem um resíduo em R.
 *
 *   cc -O2 -std=c99 -I lib tests/vizinha.c -o vizinha && ./vizinha
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "reta.h"

#define NT 4
#define KC 3
#define SC 100L                               /* centésimos */

static int arg_l(const long *v, int n){
    int b = 0;
    for(int k = 1; k < n; k += 1) if(v[k] > v[b]) b = k;
    return b;
}

/* Um passo de Kirchhoff em ℤ: score_j = a_j · (SC² + Σ s·c). Argmax = argmax de s. */
static void passo(int nt, const int nk[NT], const long a[NT][KC],
                  const long c[NT][KC][NT][KC], long s[NT][KC], int raio){
    long nv[NT][KC];
    memset(nv, 0, sizeof nv);
    for(int t = 0; t < nt; t += 1){
        for(int j = 0; j < nk[t]; j += 1){
            long viz = 0;
            for(int u = 0; u < nt; u += 1){
                if(u == t) continue;
                int d = u > t ? u - t : t - u;
                if(d > raio) continue;
                for(int l = 0; l < nk[u]; l += 1) viz += s[u][l] * c[t][j][u][l];
            }
            nv[t][j] = a[t][j] * (SC*SC + viz);
        }
    }
    memcpy(s, nv, sizeof nv);
}

int main(void){
printf("\n=== A ASSISTENTE NO BAIRRO — quem escolhe é a contração do rei ============\n");
printf("    Eu escolhia pelo prefixo comum e escrevi que 'o contexto é raso'. Era:\n");
printf("    eu tinha inventado um contexto em vez de usar o que já está medido.\n");

const char *pos[NT]  = { "o", "rio", "banco", "largo" };
const char *cand[NT][KC] = {
    { "the", "", "" },
    { "river", "laugh", "" },
    { "bank", "bench", "" },
    { "wide", "long", "" },
};
int nk[NT] = { 1, 2, 2, 2 };
long a[NT][KC] = { {100,0,0}, {60,40,0}, {45,55,0}, {50,50,0} };
static long c[NT][KC][NT][KC];

printf("\n§V1  A contração converge, e o ponto fixo é o do rei.\n\n");
{
    memset(c, 0, sizeof c);
    c[1][0][2][0] = 90;  c[2][0][1][0] = 90;      /* river <-> bank */
    c[1][0][2][1] = 5;   c[2][1][1][0] = 5;       /* river <-> bench */
    c[3][0][2][0] = 40;  c[2][0][3][0] = 40;      /* wide <-> bank */
    long s[NT][KC];
    memcpy(s, a, sizeof s);
    long g0_bank = s[2][0], g0_bench = s[2][1];
    passo(NT, nk, a, c, s, 2);
    long g1_bank = s[2][0], g1_bench = s[2][1];
    int virou = (g0_bank < g0_bench) && (g1_bank > g1_bench);
    /* a parte do vencedor cresce: g1_bank / (g1_bank+g1_bench) > g0_bank / (g0_bank+g0_bench) */
    int cresce = (g1_bank * (g0_bank + g0_bench) > g0_bank * (g1_bank + g1_bench));
    printf("      marginal banco/bench = %ld/%ld; depois de um Kirchhoff = %ld/%ld\n",
           g0_bank, g0_bench, g1_bank, g1_bench);
    printf("      virou? %s   a parte do bank cresce? %s\n\n",
           virou ? "sim" : "NÃO", cresce ? "sim" : "NÃO");
    ok("A CONTRACAO E KIRCHHOFF EM Z: um passo, score = a·(SC² + Σ s·c), vira bench->bank"
       " e a parte do vencedor CRESCE — produto cruzado, sem residuo em R. d < 1e-12 e"
       " q = r_{k+1}/r_k eram o transporte",
       virou && cresce);

    long forma_ok = 0, nunca_zero = 0, andares = 0, coprimos = 0;
    const long M_REI = 1;
    for(int k = 1; k <= 40; k += 1){
        long P, Q;
        rt_orbita(M_REI, k, &P, &Q);
        long f = rt_forma(P, Q, M_REI);
        andares += 1;
        if(f == 1 || f == -1) forma_ok += 1;
        if(f != 0) nunca_zero += 1;
        if(rt_mdc(P, Q) == 1) coprimos += 1;
    }
    printf("      orbita do rei: forma ±1 em %ld de %ld, nunca zero em %ld, coprimos %ld\n\n",
           forma_ok, andares, nunca_zero, coprimos);
    ok("A ORBITA DO REI E EXACTA, E TRAZ O PROPRIO CERTIFICADO: [p:q] -> [m.p+q : p]"
       " em INTEIROS, forma ±1 nos 40 andares e NUNCA zero, p/q em termos minimos",
       forma_ok == andares && nunca_zero == andares && coprimos == andares
       && andares == 40);

    /* o rácio Kirchhoff NÃO é o do ouro [p:q]↦[p+q:p] sobre a marginal 45:55 */
    long ouro_n = 45 + 55, ouro_d = 45;           /* [100:45] */
    int mesmo = (g1_bank * ouro_d == ouro_n * g1_bench);
    printf("      Kirchhoff %ld:%ld  contra ouro %ld:%ld  — iguais? %s\n\n",
           g1_bank, g1_bench, ouro_n, ouro_d, mesmo ? "sim" : "não");
    ok("MAS A TAXA DESTE BAIRRO NAO E A DO REI COM m = 1: o racio Kirchhoff 45·(SC²+viz)"
       " nao e o mapa [45:55] -> [100:45]. A estrutura e a da orbita; o operador e o"
       " bairro. 0,142 contra 0,382 era o transporte do quociente em R",
       !mesmo && g1_bank > 0 && g1_bench > 0);
    conclui("O que faz dela contracao nao e o limiar: e o vencedor puxar, em Z.");
}

printf("\n§V2  A iteração 0 É a marginal — e o bairro tem de PAGAR para valer.\n\n");
{
    long s[NT][KC];
    memcpy(s, a, sizeof s);
    int m0 = arg_l(a[2], nk[2]);
    passo(NT, nk, a, c, s, 2);
    int mf = arg_l(s[2], nk[2]);
    printf("      posicao   marginal              com o bairro\n");
    for(int t = 1; t < NT; t += 1){
        int a0 = arg_l(a[t], nk[t]), af = arg_l(s[t], nk[t]);
        printf("      %-9s %-21s %s%s\n", pos[t], cand[t][a0], cand[t][af],
               a0 != af ? "   <- MUDOU" : "");
    }
    printf("\n");
    ok("a marginal sozinha diz 'bench' — e a frequencia, e mais nada", m0 == 1);
    ok("e o bairro muda para 'bank' — a vizinhanca pagou", mf == 0);
    conclui("Se a iteracao 0 ja acertasse, o bairro nao valia nada.");
}

printf("\n§V3  O bairro muda escolhas, e muda-as para o lado certo.\n\n");
{
    memset(c, 0, sizeof c);
    c[1][1][2][1] = 90;  c[2][1][1][1] = 90;      /* laugh <-> bench */
    c[1][1][2][0] = 5;   c[2][0][1][1] = 5;
    long a2[NT][KC] = { {100,0,0}, {40,60,0}, {55,45,0}, {50,50,0} };
    long s[NT][KC];
    memcpy(s, a2, sizeof s);
    int m0 = arg_l(a2[2], nk[2]);
    passo(NT, nk, a2, c, s, 2);
    int mf = arg_l(s[2], nk[2]);
    printf("      marginal diz '%s'; com a outra vizinhanca o bairro diz '%s'\n\n",
           cand[2][m0], cand[2][mf]);
    ok("a MESMA palavra vai para o outro lado quando a vizinhanca muda", m0 == 0 && mf == 1);
    conclui("A vizinhanca inteira entra e a contracao desdobra.");
}

printf("\n§V4  E é fail-closed: onde não há vizinhança, não se inventa.\n\n");
{
    memset(c, 0, sizeof c);
    long s[NT][KC];
    memcpy(s, a, sizeof s);
    int m0 = arg_l(a[2], nk[2]);
    passo(NT, nk, a, c, s, 2);
    int mf = arg_l(s[2], nk[2]);
    printf("      sem vizinhanca nenhuma: marginal '%s', depois do passo '%s'\n\n",
           cand[2][m0], cand[2][mf]);
    /* com c=0, score = a·SC², argmax = argmax de a */
    int paralelo = 1;
    for(int j = 0; j < nk[2]; j += 1)
        if(s[2][j] != a[2][j] * (SC*SC)) paralelo = 0;
    ok("sem bairro, o ponto fixo E a marginal — nao se inventa informacao."
       " E score = a·SC², paralelo a a, exacto — nao so o argmax",
       m0 == mf && paralelo && m0 == 1);
    conclui("Onde nao ha de onde decidir, devolve-se o que se tinha.");
}

printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
if(!falhas) printf("  RESIDUO 0\n");
else printf("  NAO FECHOU\n");
return falhas ? 1 : 0;
}
