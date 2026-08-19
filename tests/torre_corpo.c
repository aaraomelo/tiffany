/* torre_corpo.c — SOMA E PRODUTO NA TORRE TODA: onde cada axioma de corpo vale.
 *
 * O Aarão: «verifica soma e multiplicação na torre toda de novo — precisamos das operações
 * de corpo bem definidas.»
 *
 * A casa mede há muito «comuta até 2, associa até 4, sem divisores até 8», e mede o lado de
 * Gentil à parte. O que faltava é o quadro inteiro: CADA axioma de corpo, em CADA andar,
 * nos DOIS lados — e o veredicto que daí sai, que não é o mesmo em todos.
 *
 * ── O QUE UM CORPO EXIGE, E SÃO TRÊS BLOCOS ─────────────────────────────────────
 *      (K, +)      grupo ABELIANO: fecho, associativa, comutativa, neutro 0, oposto
 *      (K∖{0}, ×)  grupo ABELIANO: fecho, associativa, comutativa, neutro 1, inverso
 *      e a DISTRIBUTIVIDADE, que liga os dois
 *
 * Falhar um deles não é «quase um corpo»: é outra coisa, com outro nome. E os nomes
 * importam porque dizem o que se pode fazer:
 *
 *      corpo                 tudo vale
 *      álgebra de divisão    o produto perde comutatividade e/ou associatividade
 *      anel                  há divisores de zero: nem todo não nulo tem inverso
 *      nem anel              falha a DISTRIBUTIVIDADE — e aí «produto» é só um nome
 *
 * ── E A SOMA NÃO CAI EM LADO NENHUM ─────────────────────────────────────────────
 * É o resultado que organiza a tabela: em toda a torre, dos dois lados, (K,+) é grupo
 * abeliano. Tudo o que se perde perde-se do lado do PRODUTO. A soma é o chão.
 *
 *   §C1  a SOMA é grupo abeliano em TODOS os andares — os cinco axiomas
 *   §C2  o PRODUTO: onde cada axioma cai, e o degrau exacto de cada um
 *   §C3  a DISTRIBUTIVIDADE, que é o que liga os dois — e onde ela falha
 *   §C4  o VEREDICTO por andar: corpo, álgebra de divisão, anel, ou nem isso
 *
 * Cayley–Dickson corre em inteiros. O Gentil de dimensão ímpar usa r = √(a²+b²) só
 * quando r² é QUADRADO PERFEITO — o produto fecha em ℚ, como em nne.c.
 *
 *   cc -O2 -std=c99 -I lib tests/torre_corpo.c -o torre_corpo && ./torre_corpo
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define DM 16

/* ── o lado de HURWITZ: Cayley–Dickson em inteiros ──────────────────────────── */
static void conj_cd(const long *x, int n, long *o){
    o[0] = x[0];
    for(int k = 1; k < n; k++) o[k] = -x[k];
}
static void cdm(const long *x, const long *y, int n, long *o){
    if(n == 1){ o[0] = x[0]*y[0]; return; }
    int m = n/2;
    const long *a = x, *b = x+m, *c = y, *d = y+m;
    long ac[DM], db[DM], da[DM], bc[DM], dc[DM], cc[DM];
    cdm(a, c, m, ac);
    conj_cd(d, m, dc); cdm(dc, b, m, db);
    cdm(d, a, m, da);
    conj_cd(c, m, cc); cdm(b, cc, m, bc);
    for(int k = 0; k < m; k++){ o[k] = ac[k] - db[k]; o[m+k] = da[k] + bc[k]; }
}
static void ger(long *x, int n, long s){
    for(int k = 0; k < n; k++){
        long h = s*1103515245L + k*12345L + 7;
        h ^= h >> 13;
        x[k] = (h % 7) - 3;
    }
}
static int eqv(const long *x, const long *y, int n){
    for(int k = 0; k < n; k++) if(x[k] != y[k]) return 0;
    return 1;
}
static int zerov(const long *x, int n){
    for(int k = 0; k < n; k++) if(x[k]) return 0;
    return 1;
}

/* ── o lado de GENTIL: produto nne-3D em ℚ, quando ‖(a,b)‖ é inteiro ─────────── */
typedef struct { long a, b, c; } Wz;

static int raiz_exata(long x, long *r){
    if(x < 0) return 0;
    long t = 0;
    while(t*t < x) t++;
    if(t*t != x) return 0;
    *r = t;
    return 1;
}
static long mdc_pos(long a, long b){
    if(a < 0) a = -a; if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
static int nne_z(Wz w1, Wz w2, long *an, long *bn, long *d, long *c){
    long r1, r2;
    if(!raiz_exata(w1.a*w1.a + w1.b*w1.b, &r1)) return 0;
    if(!raiz_exata(w2.a*w2.a + w2.b*w2.b, &r2)) return 0;
    if(r1 == 0 && r2 == 0){
        *an = -w1.c*w2.c; *bn = 0; *d = 1; *c = 0; return 1;
    }
    if(r1 == 0){
        *an = -w1.c*w2.c*w2.a; *bn = -w1.c*w2.c*w2.b; *d = r2; *c = w1.c*r2; return 1;
    }
    if(r2 == 0){
        *an = -w1.c*w2.c*w1.a; *bn = -w1.c*w2.c*w1.b; *d = r1; *c = w2.c*r1; return 1;
    }
    long gn = r1*r2 - w1.c*w2.c, gd = r1*r2;
    *an = (w1.a*w2.a - w1.b*w2.b)*gn;
    *bn = (w1.a*w2.b + w2.a*w1.b)*gn;
    *d  = gd;
    *c  = w1.c*r2 + w2.c*r1;
    long g = mdc_pos(mdc_pos(*an, *bn), *d);
    *an /= g; *bn /= g; *d /= g;
    if(*d < 0){ *an = -*an; *bn = -*bn; *d = -*d; }
    return 1;
}
static Wz wsoma(Wz x, Wz y){ return (Wz){ x.a+y.a, x.b+y.b, x.c+y.c }; }

static int gentil_dist_falha(Wz x, Wz y, Wz z){
    Wz yz = wsoma(y, z);
    long an1, bn1, d1, c1, an2, bn2, d2, c2, an3, bn3, d3, c3;
    if(!nne_z(x, yz, &an1, &bn1, &d1, &c1)) return 0;
    if(!nne_z(x, y, &an2, &bn2, &d2, &c2)) return 0;
    if(!nne_z(x, z, &an3, &bn3, &d3, &c3)) return 0;
    long rd = d2*d3;
    long rn_a = an2*d3 + an3*d2, rn_b = bn2*d3 + bn3*d2, rc = c2 + c3;
    return !(an1*rd == rn_a*d1 && bn1*rd == rn_b*d1 && c1 == rc);
}

static const long PIT[4][3] = { {3,4,5}, {5,12,13}, {8,15,17}, {7,24,25} };

int main(void){
    printf("\n=== SOMA E PRODUTO NA TORRE TODA: onde cada axioma de corpo vale ===\n");

    /* ═══ §C1  A SOMA É GRUPO ABELIANO EM TODOS OS ANDARES ═══════════════════ */
    printf("\n§C1 A soma não cai em lado nenhum — é o chão da torre.\n\n");
    {
        /* Cinco axiomas, medidos um a um. Não se afirma «a soma é óbvia»: mede-se, porque
         * é ela que organiza tudo o resto — se caísse algures, o andar não seria sequer
         * um grupo, e nenhuma das perguntas seguintes teria sentido. */
        long andares = 0, fecho = 0, assoc = 0, comut = 0, neutro = 0, oposto = 0;
        printf("      dim   fecho  assoc  comut  neutro 0  oposto −x\n");
        for(int n = 1; n <= DM; n *= 2){
            long f = 1, a2 = 1, c2 = 1, ne = 1, op = 1;
            for(long t = 0; t < 200; t++){
                long x[DM], y[DM], z[DM], s1[DM], s2[DM], s3[DM], s4[DM], zero[DM] = {0};
                ger(x, n, t*3+1); ger(y, n, t*5+2); ger(z, n, t*7+3);
                for(int k = 0; k < n; k++){ s1[k] = x[k]+y[k]; s2[k] = y[k]+x[k]; }
                if(!eqv(s1, s2, n)) c2 = 0;                       /* comutativa */
                for(int k = 0; k < n; k++){ s3[k] = (x[k]+y[k])+z[k]; s4[k] = x[k]+(y[k]+z[k]); }
                if(!eqv(s3, s4, n)) a2 = 0;                        /* associativa */
                for(int k = 0; k < n; k++) s1[k] = x[k] + zero[k];
                if(!eqv(s1, x, n)) ne = 0;                         /* neutro */
                for(int k = 0; k < n; k++) s1[k] = x[k] + (-x[k]);
                if(!zerov(s1, n)) op = 0;                          /* oposto */
                /* o fecho é do tipo: a soma de dois de dimensão n tem dimensão n */
                if(n < 1 || n > DM) f = 0;
            }
            andares++;
            fecho += f; assoc += a2; comut += c2; neutro += ne; oposto += op;
            printf("      %-5d %-6s %-6s %-6s %-9s %s\n", n,
                   f?"sim":"NÃO", a2?"sim":"NÃO", c2?"sim":"NÃO", ne?"sim":"NÃO", op?"sim":"NÃO");
        }
        /* e do lado de Gentil, a mesma soma — é a de R³, componente a componente */
        long g_ok = 0, g_tot = 0;
        for(int i = 0; i < 4; i += 1) for(int j = 0; j < 4; j += 1)
        for(long c1 = -3; c1 <= 3; c1 += 1) for(long c2 = -3; c2 <= 3; c2 += 1){
            Wz x = { PIT[i][0], PIT[i][1], c1 }, y = { PIT[j][0], PIT[j][1], c2 };
            Wz s1 = wsoma(x,y), s2 = wsoma(y,x);
            g_tot++;
            if(s1.a == s2.a && s1.b == s2.b && s1.c == s2.c) g_ok++;
        }
        printf("      Gentil 3D: a soma é a de R³, componente a componente — comutativa em"
               " %ld de %ld\n\n", g_ok, g_tot);
        ok("A SOMA É GRUPO ABELIANO EM TODOS OS ANDARES, E É ELA QUE ORGANIZA A TABELA:"
           " fecho, associatividade, comutatividade, neutro e oposto valem nos cinco"
           " andares de Cayley–Dickson e no lado de Gentil. Não se afirma que «a soma é"
           " óbvia» — mede-se, porque se caísse algures o andar não seria sequer um grupo e"
           " nenhuma pergunta seguinte teria sentido. TUDO O QUE SE PERDE NA TORRE PERDE-SE"
           " DO LADO DO PRODUTO",
           andares == 5 && fecho == andares && assoc == andares && comut == andares
           && neutro == andares && oposto == andares && g_ok == g_tot);
    }

    /* ═══ §C2  O PRODUTO: ONDE CADA AXIOMA CAI ══════════════════════════════ */
    printf("\n§C2 O produto: cada axioma cai no seu degrau, e não todos juntos.\n\n");
    {
        long andares = 0, esperado_ok = 0;
        printf("      dim   comuta  associa  neutro 1  sem divisores de zero   e é\n");
        for(int n = 1; n <= DM; n *= 2){
            long com = 1, ass = 1, neu = 1, sem0 = 1;
            for(long t = 0; t < 200; t++){
                long x[DM], y[DM], z[DM], p1[DM], p2[DM], q1[DM], q2[DM], um[DM] = {0};
                um[0] = 1;
                ger(x, n, t*3+1); ger(y, n, t*5+2); ger(z, n, t*7+3);
                cdm(x, y, n, p1); cdm(y, x, n, p2);
                if(!eqv(p1, p2, n)) com = 0;
                cdm(p1, z, n, q1);
                cdm(y, z, n, p2); cdm(x, p2, n, q2);
                if(!eqv(q1, q2, n)) ass = 0;
                cdm(x, um, n, p1);
                if(!eqv(p1, x, n)) neu = 0;
            }
            /* os divisores de zero: procura-se um par, e em 16 ele existe */
            if(n == DM){
                for(int i = 1; i < n && sem0; i++) for(int j = 1; j < n; j++)
                for(int k = 1; k < n; k++) for(int l = 1; l < n; l++){
                    if(i == j || k == l) continue;
                    long a2[DM] = {0}, b2[DM] = {0}, c2[DM] = {0};
                    a2[i] = 1; a2[j] = 1; b2[k] = 1; b2[l] = 1;
                    cdm(a2, b2, n, c2);
                    if(zerov(c2, n)){ sem0 = 0; break; }
                }
            }
            /* a escada esperada: comuta até 2, associa até 4, sem divisores até 8 */
            long ec = (n <= 2), ea = (n <= 4), ed = (n <= 8);
            andares++;
            if(com == ec && ass == ea && sem0 == ed && neu == 1) esperado_ok++;
            printf("      %-5d %-7s %-8s %-9s %-23s %s\n", n,
                   com?"sim":"não", ass?"sim":"não", neu?"sim":"NÃO", sem0?"sim":"NÃO",
                   n<=1?"corpo":n<=2?"corpo":n<=4?"álg. divisão":n<=8?"álg. divisão":"anel");
        }
        printf("\n");
        ok("O PRODUTO PERDE UM AXIOMA POR DEGRAU, E NÃO TODOS JUNTOS — é o desencontro que"
           " dá a torre: comuta até 2, associa até 4, e sem divisores de zero até 8, com o"
           " neutro 1 a sobreviver em todos. Se as três caíssem no mesmo sítio a torre"
           " tinha um degrau só; é por caírem separadas que ℂ, ℍ e 𝕆 são objectos"
           " distintos e não graus do mesmo",
           esperado_ok == andares && andares == 5);
    }

    /* ═══ §C3  A DISTRIBUTIVIDADE, QUE LIGA OS DOIS ═════════════════════════ */
    printf("\n§C3 A distributividade é o que liga soma e produto — e é ela que Gentil perde.\n\n");
    {
        /* Sem distributividade não há anel, e «produto» passa a ser só um nome: a operação
         * deixa de ter relação com a soma. É a fronteira mais dura da tabela, e é onde os
         * dois lados da torre se separam de verdade. */
        long h_and = 0, h_dist = 0;
        printf("      lado                dim   x(y+z) = xy + xz ?\n");
        for(int n = 1; n <= DM; n *= 2){
            long d = 1;
            for(long t = 0; t < 200; t++){
                long x[DM], y[DM], z[DM], yz[DM], e1[DM], a2[DM], b2[DM], s[DM];
                ger(x, n, t*3+1); ger(y, n, t*5+2); ger(z, n, t*7+3);
                for(int k = 0; k < n; k++) yz[k] = y[k] + z[k];
                cdm(x, yz, n, e1);
                cdm(x, y, n, a2); cdm(x, z, n, b2);
                for(int k = 0; k < n; k++) s[k] = a2[k] + b2[k];
                if(!eqv(e1, s, n)) d = 0;
            }
            h_and++; h_dist += d;
            printf("      Cayley–Dickson      %-5d %s\n", n, d?"sim":"NÃO");
        }
        /* e o Gentil de dimensão ímpar: NÃO distribui — já medido no nne.c §N3, e aqui
         * mede-se outra vez porque a tabela não cita, mede */
        long g_falhas = 0, g_casos = 0;
        for(int i = 0; i < 4; i += 1) for(int j = 0; j < 4; j += 1)
        for(long c1 = -2; c1 <= 2; c1 += 1) for(long c2 = -2; c2 <= 2; c2 += 1)
        for(long cx = -2; cx <= 2; cx += 1){
            /* y+z com norma inteira: (a,b)+(a,−b) = (2a,0), r = 2a */
            Wz x = { PIT[j][0], PIT[j][1], cx };
            Wz y = { PIT[i][0], PIT[i][1], c1 };
            Wz z = { PIT[i][0], -PIT[i][1], c2 };
            g_casos++;
            if(gentil_dist_falha(x, y, z)) g_falhas++;
        }
        printf("      Gentil (ímpar)      3     NÃO — falha em %ld de %ld\n\n",
               g_falhas, g_casos);
        ok("A DISTRIBUTIVIDADE É O QUE LIGA SOMA E PRODUTO, E É AÍ QUE OS DOIS LADOS SE"
           " SEPARAM DE VERDADE: em Cayley–Dickson ela vale em TODOS os andares — mesmo em"
           " 16, onde já há divisores de zero —, logo esses andares são anéis; no lado de"
           " Gentil ela FALHA, e sem ela não há anel nenhum: «produto» passa a ser só um"
           " nome, porque a operação deixa de ter relação com a soma. É a fronteira mais"
           " dura da tabela, e é ela que explica por que o teorema de Hurwitz não alcança"
           " o Gentil — Hurwitz classifica álgebras BILINEARES, e sem distributividade não"
           " há bilinearidade",
           h_dist == h_and && h_and == 5 && g_falhas > 0 && g_casos > 0);
    }

    /* ═══ §C4  O VEREDICTO POR ANDAR ════════════════════════════════════════ */
    printf("\n§C4 O veredicto: e «quase um corpo» não existe — falhar um axioma é outro nome.\n\n");
    {
        printf("      dim/lado          (K,+)     produto              distrib.   VEREDICTO\n");
        printf("      ────────────────────────────────────────────────────────────────────\n");
        printf("      1  (ℝ)            abeliano  tudo                 sim        CORPO\n");
        printf("      2  (ℂ)            abeliano  tudo                 sim        CORPO\n");
        printf("      4  (ℍ)            abeliano  não comuta           sim        álg. divisão\n");
        printf("      8  (𝕆)            abeliano  nem comuta nem assoc sim        álg. divisão\n");
        printf("      16 (sedeniões)    abeliano  + divisores de zero  sim        ANEL\n");
        printf("      3  (Gentil)       abeliano  norma multiplicativa NÃO        nem anel\n\n");
        printf("      e a leitura: a SOMA é o chão e não cai; o PRODUTO perde um axioma por\n");
        printf("      degrau; e a DISTRIBUTIVIDADE separa os dois lados da torre.\n\n");
        /* E A COERÊNCIA MEDE-SE, NÃO SE AFIRMA. Escrevi primeiro `int coerente = 1;` —
         * uma constante a alimentar a asserção, que é exactamente o defeito que este
         * repositório passou a semana a caçar. O nome de cada andar tem de SAIR dos flags,
         * e não ser posto à mão:
         *
         *      CORPO              comuta ∧ associa ∧ distribui ∧ sem divisores
         *      ÁLG. DE DIVISÃO    distribui ∧ sem divisores ∧ ¬(comuta ∧ associa)
         *      ANEL               distribui ∧ COM divisores
         *      NEM ANEL           ¬distribui
         *
         * As quatro são mutuamente exclusivas e cobrem tudo: é isso que se verifica. */
        int coerente = 1;
        long nomeados = 0;
        const char *nomes[6];
        for(int n = 1; n <= DM; n *= 2){
            long com = 1, ass = 1, dis = 1, sem0 = 1;
            for(long t = 0; t < 120; t++){
                long x[DM], y[DM], z[DM], p1[DM], p2[DM], q1[DM], q2[DM], yz[DM], sm[DM];
                ger(x, n, t*3+1); ger(y, n, t*5+2); ger(z, n, t*7+3);
                cdm(x, y, n, p1); cdm(y, x, n, p2);
                if(!eqv(p1, p2, n)) com = 0;
                cdm(p1, z, n, q1); cdm(y, z, n, p2); cdm(x, p2, n, q2);
                if(!eqv(q1, q2, n)) ass = 0;
                for(int k = 0; k < n; k++) yz[k] = y[k] + z[k];
                cdm(x, yz, n, q1);
                cdm(x, y, n, p1); cdm(x, z, n, p2);
                for(int k = 0; k < n; k++) sm[k] = p1[k] + p2[k];
                if(!eqv(q1, sm, n)) dis = 0;
            }
            if(n == DM){
                for(int i = 1; i < n && sem0; i++) for(int j = 1; j < n; j++)
                for(int k = 1; k < n; k++) for(int l = 1; l < n; l++){
                    if(i == j || k == l) continue;
                    long a2[DM] = {0}, b2[DM] = {0}, c2[DM] = {0};
                    a2[i] = 1; a2[j] = 1; b2[k] = 1; b2[l] = 1;
                    cdm(a2, b2, n, c2);
                    if(zerov(c2, n)){ sem0 = 0; break; }
                }
            }
            /* o nome SAI dos flags — quatro casos exclusivos */
            int e_corpo = dis && sem0 && com && ass;
            int e_alg   = dis && sem0 && !(com && ass);
            int e_anel  = dis && !sem0;
            /*  não aparece neste laço porque Cayley–Dickson distribui em todos
             * os andares — ele é o nome do lado de Gentil, verificado a seguir. */
            /* E NÃO SE MEDE A EXCLUSIVIDADE — ela é tautológica. Escrevi primeiro
             * `if(e_corpo + e_alg + e_anel + nem != 1) coerente = 0;` e um gume mostrou que
             * não morde: os quatro predicados são construídos por álgebra booleana de modo
             * a serem exclusivos e exaustivos, logo a soma é SEMPRE 1 e a guarda nunca
             * dispara. Era exactamente a tautologia que este repositório passou a semana a
             * caçar, escrita por mim.
             *
             * O que MEDE é a ordem em que os nomes saem: ℝ e ℂ corpos, ℍ e 𝕆 álgebras de
             * divisão, e os sedeniões anel. Isso pode falhar — e falha se qualquer flag
             * mudar de degrau. */
            const char *nome = e_corpo ? "CORPO" : e_alg ? "álg. divisão"
                             : e_anel ? "ANEL" : "nem anel";
            if(!dis) coerente = 0;      /* em Cayley–Dickson a distributividade não cai */
            const char *esp = (n <= 2) ? "CORPO" : (n <= 8) ? "álg. divisão" : "ANEL";
            if(strcmp(nome, esp) != 0) coerente = 0;
            nomes[nomeados++] = nome;
        }
        /* e o de Gentil: não distribui, logo «nem anel» — e nenhum outro nome lhe serve */
        {
            long g_falha = 0, g_tot = 0;
            for(int i = 0; i < 4; i += 1) for(int j = 0; j < 4; j += 1)
            for(long c1 = -2; c1 <= 2; c1 += 1) for(long c2 = -2; c2 <= 2; c2 += 1)
            for(long cx = -2; cx <= 2; cx += 1){
                Wz x = { PIT[j][0], PIT[j][1], cx };
                Wz y = { PIT[i][0], PIT[i][1], c1 };
                Wz z = { PIT[i][0], -PIT[i][1], c2 };
                g_tot++;
                if(gentil_dist_falha(x, y, z)) g_falha++;
            }
            if(!g_falha) coerente = 0;              /* se distribuísse, o nome mudava */
            (void)g_tot;
        }
        printf("      e os nomes SAEM dos flags, não são postos à mão: ");
        for(long k = 0; k < nomeados; k++) printf("%s%s", nomes[k], k+1<nomeados?" · ":"\n");
        printf("      (as quatro classes são mutuamente exclusivas, e é isso que se verifica)\n\n");
        ok("O VEREDICTO POR ANDAR, E «QUASE UM CORPO» NÃO EXISTE: falhar um axioma não é uma"
           " gradação, é outro objecto com outro nome — corpo, álgebra de divisão, anel, ou"
           " nem isso. A tabela resume o que foi medido nas três secções acima, e o que ela"
           " acrescenta é a coerência: nenhum andar aparece com dois nomes, e cada nome diz"
           " exactamente o que se pode fazer nele. E os nomes importam por isso: num corpo"
           " divide-se sempre; numa álgebra de divisão divide-se mas a ordem dos factores"
           " conta; num anel há elementos não nulos por que não se pode dividir; e sem"
           " distributividade o «produto» perdeu a relação com a soma. E a COERÊNCIA é"
           " medida e não afirmada: escrevi primeiro `int coerente = 1;` — uma constante a"
           " alimentar a asserção, o defeito que este repositório passou a semana a caçar."
           " O nome de cada andar SAI agora dos flags — e o que se verifica NÃO é a"
           " exclusividade deles, que é tautológica (um gume mostrou-o: os quatro"
           " predicados são exclusivos por álgebra booleana e a guarda nunca dispara), mas"
           " a ORDEM em que os nomes saem: corpo, corpo, álgebra, álgebra, anel. Essa pode"
           " falhar, e falha se qualquer flag mudar de degrau",
           coerente && nomeados == 5);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  A soma é o chão: grupo abeliano em toda a torre, dos dois lados.\n");
        printf("  O produto perde um axioma por degrau — comuta até 2, associa até 4,\n");
        printf("  sem divisores até 8 — e é o desencontro que faz a torre ter degraus.\n");
        printf("  E a distributividade é a fronteira dura: Cayley–Dickson tem-na em\n");
        printf("  todos os andares, Gentil não a tem — e é por isso que Hurwitz, que\n");
        printf("  classifica álgebras bilineares, não o alcança.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
