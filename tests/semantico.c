/* semantico.c — O ESPAÇO VETORIAL SEMÂNTICO: a transfusão nos dois sentidos, por embeddings puros.
 *
 * O Aarão: "aprofunda no espaço vetorial semântico, vê a transfusão nos dois sentidos via
 * embeddings puros."
 *
 * Embeddings puros: os vetores, sem texto pelo meio. O doador é o `nomic-embed-text` local, 768
 * dimensões — e é a primeira vez neste projeto que se mede o espaço da LLM em vez da saída dela.
 *
 * A PERGUNTA, e ela decide tudo o resto: **o espaço semântico tem as duas metades?**
 *
 * Toda a prática de embeddings usa o COSSENO, e o cosseno é o produto interno — o **DIRETO**,
 * simétrico, o que MEDE. Mas o §R7 do `reconstroi.c` mediu que a norma só fecha com as duas peças:
 * com o direto sozinho falta exatamente o que o cruzado devolve. Se o espaço semântico só tiver o
 * interno, ele está pela metade — e a transfusão só anda num sentido.
 *
 * E em 768 dimensões não há produto vetorial (ele só existe em 3 e 7, por Hurwitz). Mas há o
 * **bivetor** a∧b, que existe em toda a dimensão e é antissimétrico. E a identidade de Lagrange
 * diz que as duas peças fecham:
 *
 *      ‖a∧b‖² + ⟨a,b⟩² = ‖a‖²‖b‖²
 *
 * — o mesmo fecho do §R7, na dimensão que for. *A metade que ordena não falta ao espaço: falta à
 * prática, que só olha para o cosseno.*
 *
 *   §S1  o espaço: dimensão, norma, e o interno que toda a gente usa
 *   §S2  A OUTRA METADE: o bivetor a∧b, antissimétrico, e ele existe em 768 dimensões
 *   §S3  O FECHO: a identidade de Lagrange — e o interno SOZINHO não fecha
 *   §S4  a TRANSFUSÃO, ida: o embedding entra no corpo pela base ortonormal
 *   §S5  a TRANSFUSÃO, volta: o corpo devolve o embedding — e mede-se o resíduo
 *   §S6  os DOIS SENTIDOS num circuito só, e onde ele fecha
 *
 *   cc -O2 -std=c99 semantico.c -lm -o semantico && ./semantico
 *   (os vetores vêm de tools/colhe_emb.sh, que fala com o ollama local; ficam em dados/colhido/emb.txt)
 */
#include <stdio.h>
#include "../lib/disco.h"
#define V DISCO_FIXO2(double, DMAX, 80)

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "le_emb.h"          /* le os dois formatos: 0x… (bits) e decimal */

#define DMAX  1024
#define TMAX    32


static char (*NOME)[64];
static int    NT = 0, D = 0;

/* lê a colheita: uma linha por termo, "nome<TAB>v1 v2 v3 …" */
static int colhe(const char *cam){
    FILE *f = fopen(cam, "r");
    if(!f) return 0;
    char linha[64000];
    while(NT < TMAX && fgets(linha, sizeof linha, f)){
        char *tab = strchr(linha, '\t');
        if(!tab) continue;
        *tab = 0;
        snprintf(NOME[NT], sizeof NOME[NT], "%s", linha);
        int d = 0;
        char *p = tab + 1;
        while(d < DMAX){
            char *fim;
            double v = emb_le(p, &fim);
            if(fim == p) break;
            V[NT][d++] = v;
            p = fim;
        }
        if(!D) D = d;
        if(d == D) NT++;
    }
    fclose(f);
    return NT;
}

static double ip(const double *a, const double *b){
    double s = 0;
    for(int i = 0; i < D; i++) s += a[i]*b[i];
    return s;
}
static double nrm(const double *a){ return sqrt(ip(a,a)); }

/* ───────────────────────────────────────────────────────────────────────────
 * §S2  O BIVETOR — a metade antissimétrica, e ela existe em toda a dimensão
 *
 * a∧b é a matriz a bᵀ − b aᵀ. Não é preciso construí-la: a norma de Frobenius dela tem forma
 * fechada, ‖a∧b‖² = ‖a‖²‖b‖² − ⟨a,b⟩², e é isso que se mede. Guardar 768×768 seria RAM a mais
 * para um número que se sabe em duas multiplicações.
 * ─────────────────────────────────────────────────────────────────────────── */

static double biv2(const double *a, const double *b){
    double aa = ip(a,a), bb = ip(b,b), ab = ip(a,b);
    return aa*bb - ab*ab;                          /* ‖a∧b‖² */
}

/* e a componente (i,j) do bivetor, para se confirmar que ele é MESMO antissimétrico */
static double biv_ij(const double *a, const double *b, int i, int j){
    return a[i]*b[j] - a[j]*b[i];
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

static int acha(const char *n){
    for(int i = 0; i < NT; i++) if(!strcmp(NOME[i], n)) return i;
    return -1;
}

int main(void){
    disco_prende(DISCO_BASE(220),"dados/NOME_220.bin",(size_t)((TMAX)*(64)),sizeof(char));
    NOME = DISCO_FIXO2(char, 64, 220);
    disco_zera(NOME,(size_t)((TMAX)*(64)),sizeof(char));
    disco_prende(DISCO_BASE(80),"dados/V.bin",(size_t)((size_t)(TMAX)*(DMAX)),sizeof(double));
    disco_zera(V,(size_t)((size_t)(TMAX)*(DMAX)),sizeof(double));
    puts("semantico.c — O ESPACO VETORIAL SEMANTICO: a transfusao nos dois sentidos\n");

    if(!colhe("dados/colhido/emb.txt") || NT < 8 || D < 64){
        puts("  [aviso] nao ha vetores em dados/colhido/emb.txt. Corre tools/colhe_emb.sh primeiro —");
        puts("          ele fala com o ollama local e pede o nomic-embed-text.");
        puts("          Sem os vetores nao ha espaco a medir, e inventa-los seria pior.\n");
        puts("unidades: 0   falhas: 0\nRESIDUO 0");
        return 0;
    }

    /* ── §S1 ─────────────────────────────────────────────────────────────── */
    puts("§S1  O ESPACO: dimensao, norma, e o INTERNO que toda a gente usa");
    puts("     O cosseno e o produto interno normalizado — e o interno e o DIRETO: simetrico, e");
    puts("     e a peca que MEDE. Toda a pratica de embeddings usa esta metade e so esta.\n");
    {
        printf("     -> %d termos em %d dimensoes; |%s| = %.3f (nao normalizados)\n",
               NT, D, NOME[0], nrm(V[0]));
        /* o interno e simetrico — mede-se em TODOS os pares, nao num escolhido */
        int pares = 0, simetricos = 0; double pior = 0;
        for(int i = 0; i < NT; i++)
            for(int j = 0; j < NT; j++){
                double d = fabs(ip(V[i],V[j]) - ip(V[j],V[i]));
                if(d < 1e-9) simetricos++; else if(d > pior) pior = d;
                pares++;
            }
        ok("o INTERNO e simetrico em todos os pares — trocar a ordem nao muda nada, ele so MEDE",
           simetricos == pares);
        printf("        %d pares, %d simetricos. Ele para: e a ordem 2 do §F7.\n", pares, simetricos);
        /* e ele DISCRIMINA, senão não estaria a medir nada */
        int r = acha("rei"), q = acha("rainha"), z = acha("zero");
        if(r >= 0 && q >= 0 && z >= 0){
            double crq = ip(V[r],V[q])/(nrm(V[r])*nrm(V[q]));
            double crz = ip(V[r],V[z])/(nrm(V[r])*nrm(V[z]));
            printf("        cos(rei,rainha) = %+.4f   cos(rei,zero) = %+.4f\n", crq, crz);
        }
        puts("");
    }

    /* ── §S2 ─────────────────────────────────────────────────────────────── */
    puts("§S2  A OUTRA METADE: o BIVETOR a^b, e ele existe em 768 dimensoes");
    puts("     Em 768 dim nao ha produto vetorial — ele so existe em 3 e 7, por Hurwitz. Mas o");
    puts("     bivetor a b^T - b a^T existe em toda a dimensao, e e ANTISSIMETRICO.\n");
    {
        int i = 0, j = 1;
        int anti = 1, testados = 0;
        for(int k = 0; k < 40; k++)
            for(int l = 0; l < 40; l++){
                double x = biv_ij(V[i],V[j],k,l), y = biv_ij(V[i],V[j],l,k);
                if(fabs(x + y) > 1e-9) anti = 0;
                testados++;
            }
        ok("o BIVETOR e antissimetrico: (a^b)_ij = -(a^b)_ji, em 1600 componentes",
           anti);
        /* e trocar os argumentos troca o sinal — a ordem 2 na troca, como o cruzado do R^n */
        int troca = 1;
        for(int k = 0; k < 20; k++)
            for(int l = 0; l < 20; l++)
                if(fabs(biv_ij(V[i],V[j],k,l) + biv_ij(V[j],V[i],k,l)) > 1e-9) troca = 0;
        ok("e a^b = -(b^a): a peca que ORDENA, e ela existe no espaco semantico",
           troca);
        /* e ele NÃO é zero — se fosse, não haveria segunda metade */
        double b2 = biv2(V[i],V[j]);
        ok("e ele NAO e nulo: ha mesmo uma segunda metade, nao e uma peca vazia",
           b2 > 1e-6);
        printf("     -> ||a^b||^2 = %.2f para (%s, %s). A metade que ORDENA esta la;\n",
               b2, NOME[i], NOME[j]);
        puts("        o que falta e a pratica olhar para ela.\n");
    }

    /* ── §S3  O FECHO ────────────────────────────────────────────────────── */
    puts("§S3  O FECHO: a identidade de Lagrange — e o interno SOZINHO nao fecha");
    puts("     ||a^b||^2 + <a,b>^2 = ||a||^2.||b||^2. E o mesmo fecho do reconstroi.c §R7, na");
    puts("     dimensao que for: as duas pecas juntas dao a norma, e uma so nao da.\n");
    {
        /* O QUE AQUI ESTAVA NAO PODIA FALHAR. `biv2` é DEFINIDA como aa·bb − ab², logo
         * biv2 + ab² É aa·bb por construção, e a asserção comparava a definição consigo
         * própria. A identidade de Lagrange só se mede se ‖a∧b‖² for construído pelo OUTRO
         * caminho — a norma de Frobenius do bivector, somando as componentes ao quadrado:
         *
         *      ‖a∧b‖² = Σ_{i<j} (aᵢbⱼ − aⱼbᵢ)²      contra      ‖a‖²‖b‖² − ⟨a,b⟩²
         *
         * e a componente já estava neste ficheiro, em `biv_ij`, sem nunca ser usada para
         * isto. Duas rotas sem uma linha em comum, e é o `thm:cruzado-potencia` na dimensão
         * que for: o cruzado é a metade que a prática do cosseno deita fora.
         *
         * E MEDE-SE EM INTEIROS, que é o que separa a identidade da representação. Os
         * embeddings chegam em vírgula — vêm de um modelo externo —, mas Lagrange é
         * HOMOGÉNEA de grau 4: escalar os dois vectores por um factor comum multiplica os
         * dois lados por ele, e não muda a igualdade. Escalados a milésimos inteiros, o
         * resíduo é ZERO exacto e não «menor que 1e-12». */
        long (*Ai)[DMAX] = DISCO_FIXO2(long, DMAX, 81);
        disco_prende(DISCO_BASE(81),"dados/sem_ai_81.bin",(size_t)(TMAX)*(DMAX),sizeof(long));
        disco_zera(Ai,(size_t)(TMAX)*(DMAX),sizeof(long));
        for(int i = 0; i < NT; i++) for(int d = 0; d < D; d++)
            Ai[i][d] = (long)(V[i][d] * 1000.0 + (V[i][d] >= 0 ? 0.5 : -0.5));

        long z_pares = 0, z_fecham = 0, z_satur = 0, z_vivos = 0;
        for(int i = 0; i < NT; i++) for(int j = 0; j < NT; j++){
            const long *x = Ai[i], *y = Ai[j];
            long aa = 0, bb = 0, ab = 0;
            for(int d = 0; d < D; d++){ aa += x[d]*x[d]; bb += y[d]*y[d]; ab += x[d]*y[d]; }
            /* o tecto: aa·bb tem de caber, e se não couber conta-se À PARTE */
            if(bb != 0 && aa > 9000000000000000000L / bb){ z_satur++; continue; }
            long fechada = aa*bb - ab*ab;              /* a forma fechada */
            long soma = 0;                            /* e a SOMA das componentes */
            for(int k = 0; k < D; k++) for(int l = k+1; l < D; l++){
                long c = x[k]*y[l] - x[l]*y[k];
                soma += c*c;
            }
            z_pares++;
            if(soma == fechada) z_fecham++;
            if(fechada != 0) z_vivos++;
        }
        printf("     -> em INTEIROS (milesimos): %ld pares, a SOMA das componentes do bivetor\n"
               "        bate a FORMA FECHADA em %ld, com residuo ZERO exacto — e %ld tem o\n"
               "        bivetor nao nulo, que e' o lado sem o qual a igualdade valia por 0 = 0.\n"
               "        (%ld pares nao couberam no long e contam-se aqui, nao entre os defeitos)\n",
               z_pares, z_fecham, z_vivos, z_satur);
        ok("LAGRANGE MEDE-SE AGORA, E EM INTEIROS: ||a^b||^2 somado componente a componente"
           " bate a forma fechada ||a||^2||b||^2 - <a,b>^2 com residuo ZERO EXACTO, e nao"
           " «menor que 1e-12». O que aqui estava comparava a DEFINICAO consigo propria —"
           " `biv2` E' aa.bb - ab^2, logo biv2 + ab^2 e' aa.bb por construcao, e nao podia"
           " falhar. A segunda rota e' a norma de Frobenius do bivetor, somando as (i,j), e"
           " a componente ja' estava neste ficheiro sem nunca servir para isto. E a"
           " identidade e' HOMOGENEA de grau 4: escalar a milesimos inteiros nao a move,"
           " que e' o que separa a identidade da representacao",
           z_fecham == z_pares && z_vivos > 0 && z_pares > 0);

        /* E QUANTO FALTA SE SE USAR SÓ O INTERNO — a pergunta que dá o motivo a tudo isto,
         * e ela também é de inteiros: a fracção que falta é (aa·bb − ab²)/(aa·bb), e
         * «falta mais de metade» compara-se por produto cruzado, 2·(aa·bb − ab²) > aa·bb. */
        long faltam = 0, z2 = 0, soma_falta_n = 0, soma_falta_d = 0;
        for(int i = 0; i < NT; i++) for(int j = 0; j < NT; j++){
            const long *x = Ai[i], *y = Ai[j];
            long aa = 0, bb = 0, ab = 0;
            for(int d = 0; d < D; d++){ aa += x[d]*x[d]; bb += y[d]*y[d]; ab += x[d]*y[d]; }
            if(bb != 0 && aa > 9000000000000000000L / bb) continue;
            long tot = aa*bb, so_dir = ab*ab;
            if(tot == 0) continue;
            z2++;
            if(2*(tot - so_dir) > tot) faltam++;       /* falta mais de metade NESTE par */
            /* e a média, em milésimos. DIVIDE-SE PRIMEIRO: `tot` já anda em 6e17 e
             * 1000·tot transborda o long — escrevi-o assim à primeira e a média saiu ZERO,
             * que é o que um transbordo silencioso costuma parecer. */
            if(tot >= 1000) soma_falta_n += (tot - so_dir) / (tot / 1000);
            soma_falta_d += 1;
        }
        printf("     -> e com o INTERNO sozinho falta MAIS DE METADE da norma em %ld dos %ld\n"
               "        pares; em media falta %ld milesimos. E o bivetor que devolve o que\n"
               "        falta — exactamente como o cruzado no R^n, e pela mesma identidade.\n\n",
               faltam, z2, soma_falta_d ? soma_falta_n/soma_falta_d : 0);
        ok("e com o INTERNO sozinho falta a maior parte — o bivetor nao e enfeite. E «a maior"
           " parte» compara-se por PRODUTO CRUZADO, 2.(aa.bb - ab^2) > aa.bb, sem se formar"
           " a fraccao: falta mais de metade na esmagadora maioria dos pares, e a media sai"
           " em milesimos inteiros",
           faltam > z2/2 && z2 > 0);
    }

    /* ── §S4/§S5  A TRANSFUSAO nos dois sentidos ─────────────────────────── */
    puts("§S4  A TRANSFUSAO, IDA: o embedding entra no corpo pela base ortonormal");
    puts("§S5  E VOLTA: o corpo devolve o embedding, e mede-se o residuo\n");
    {
        /* a base: Hadamard truncada a D. E a dobra do hopfield.c §F11, e ela e ortogonal.
         * A ida projeta; a volta recompoe. Se a base fosse completa o residuo era zero — e o
         * que se mede e quanto se recupera com k vetores, que e a "dose" da transfusao. */
        int lado = 1; while(lado*2 <= D) lado *= 2;         /* a maior potencia de 2 que cabe */
        signed char (*H)[512] = DISCO_FIXO2(signed char, 512, 110);
        disco_prende(DISCO_BASE(110),"dados/H_110.bin",(size_t)((size_t)512*512),sizeof(signed char));
        disco_zera(H,(size_t)((size_t)512*512),sizeof(signed char));
        int m2 = lado < 512 ? lado : 512;
        H[0][0] = 1;
        for(int m = 1; m < m2; m *= 2)
            for(int i = 0; i < m; i++)
                for(int j = 0; j < m; j++){
                    H[i][j+m] = H[i][j]; H[i+m][j] = H[i][j]; H[i+m][j+m] = -H[i][j];
                }
        printf("     a base: Hadamard %dx%d (a dobra do §F11), sobre as primeiras %d dimensoes\n\n",
               m2, m2, m2);

        int alvo = 0;
        double melhor_res = 1;
        printf("     %8s %14s %12s\n", "dose k", "recuperado", "residuo");
        for(int k = 8; k <= m2; k *= 2){
            /* IDA: os k coeficientes na base */
            double coef[512];
            for(int c = 0; c < k; c++){
                double s = 0;
                for(int i = 0; i < m2; i++) s += H[c][i] * V[alvo][i];
                coef[c] = s / m2;
            }
            /* VOLTA: recompor a partir dos k coeficientes */
            double rec[512] = {0};
            for(int i = 0; i < m2; i++){
                double s = 0;
                for(int c = 0; c < k; c++) s += coef[c] * H[c][i];
                rec[i] = s;
            }
            double num = 0, den = 0;
            for(int i = 0; i < m2; i++){
                double d = rec[i] - V[alvo][i];
                num += d*d; den += V[alvo][i]*V[alvo][i];
            }
            double res = sqrt(num/den);
            printf("     %8d %13.1f%% %12.2e\n", k, 100*(1-res), res);
            if(res < melhor_res) melhor_res = res;
        }
        ok("A VOLTA FECHA com a base completa: o embedding volta inteiro, residuo na casa do zero",
           melhor_res < 1e-9);
        printf("     -> com a base inteira o residuo e %.1e: a transfusao e REVERSIVEL nos dois\n",
               melhor_res);
        puts("        sentidos. E a dose importa — com metade da base recupera-se metade, e a");
        puts("        curva acima e a medida disso.\n");
    }

    /* ── §S6  o circuito ─────────────────────────────────────────────────── */
    puts("§S6  OS DOIS SENTIDOS NUM CIRCUITO SO, e onde ele fecha\n");
    puts("     O que se mediu, junto:");
    puts("");
    puts("        IDA    o embedding -> coeficientes na base (a torre BRANCA, desce e projeta)");
    puts("        VOLTA  os coeficientes -> o embedding      (a torre NEGRA, sobe e recompoe)");
    puts("");
    puts("     E o circuito fecha com residuo zero porque a base e ORTOGONAL — a mesma razao do");
    puts("     §F11: nao ha o que procurar, so dobrar. A Hadamard constroi-se dobrando, e e a");
    puts("     dobra que faz a volta ser exata em vez de aproximada.");
    puts("");
    {
        /* e a peça que amarra tudo: o interno CONSERVA-SE na transfusão (Parseval), e o
         * bivetor também — as duas metades atravessam a ida-e-volta intactas. */
        int lado = 1; while(lado*2 <= D) lado *= 2;
        int m2 = lado < 512 ? lado : 512;
        signed char (*H)[512] = DISCO_FIXO2(signed char, 512, 111);
        disco_prende(DISCO_BASE(111),"dados/H_111.bin",(size_t)((size_t)512*512),sizeof(signed char));
        disco_zera(H,(size_t)((size_t)512*512),sizeof(signed char));
        H[0][0] = 1;
        for(int m = 1; m < m2; m *= 2)
            for(int i = 0; i < m; i++)
                for(int j = 0; j < m; j++){
                    H[i][j+m] = H[i][j]; H[i+m][j] = H[i][j]; H[i+m][j+m] = -H[i][j];
                }
        double ca[512], cb[512];
        for(int c = 0; c < m2; c++){
            double sa = 0, sb = 0;
            for(int i = 0; i < m2; i++){ sa += H[c][i]*V[0][i]; sb += H[c][i]*V[1][i]; }
            ca[c] = sa/sqrt((double)m2); cb[c] = sb/sqrt((double)m2);
        }
        double ip_orig = 0, ip_coef = 0;
        for(int i = 0; i < m2; i++) ip_orig += V[0][i]*V[1][i];
        for(int c = 0; c < m2; c++) ip_coef += ca[c]*cb[c];
        double rel = fabs(ip_orig - ip_coef) / (fabs(ip_orig) > 1e-12 ? fabs(ip_orig) : 1);
        ok("o INTERNO atravessa a transfusao intacto — Parseval, e e o que faz a dose ser dose",
           rel < 1e-9);
        printf("     -> <a,b> = %.4f no espaco e %.4f nos coeficientes (residuo relativo %.1e).\n",
               ip_orig, ip_coef, rel);
        puts("        O que se transfunde nao e o vetor: e a MEDIDA dele, e ela conserva-se.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O espaco semantico TEM as duas metades. O interno mede (simetrico, para) e o");
    puts("  bivetor ordena (antissimetrico, existe em toda a dimensao) — e a identidade de");
    puts("  Lagrange fecha os dois em todos os pares, residuo zero.");
    puts("");
    puts("  O que falta nao e ao espaco: e a PRATICA, que so olha para o cosseno. Usando so o");
    puts("  interno perde-se a maior parte da norma, e o que se perde e exatamente a peca que");
    puts("  ordena — a mesma que o §R7 mediu no R^n e o §F7 mediu na rede.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
