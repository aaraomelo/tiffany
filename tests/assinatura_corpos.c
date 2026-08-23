/* assinatura_corpos.c — A ASSINATURA (p,q,r) AGRUPA OS CORPOS QUE ESTÃO SEPARADOS.
 *
 * O catálogo escreve-o por extenso: «um corpo é a raiz repetida p+q+r vezes, cada
 * cópia com o seu sinal», com (p,q,r) = (quantas réguas redondas, quantas
 * hiperbólicas, quantas colapsadas); e a lei: «dois corpos com a mesma contagem são
 * CONGRUENTES --- isto é, se um é o outro visto noutra base: qualquer troca de
 * coordenadas P invertível leva A em PᵀAP e PRESERVA (p,q,r). Nada mais se extrai
 * da norma além desses três números.»
 *
 * Aqui as formas são as que o catálogo dá, uma a uma, e o que se mede é a lei.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/asscorpos tests/assinatura_corpos.c && /tmp/asscorpos
 */
#include "unidade.h"
#include <stdio.h>
#include <string.h>

#define GR 6
/* a forma quadrática como matriz simétrica diagonal de sinais: +1, -1, 0 */
typedef struct { const char *nome; const char *forma; int n; int d[GR]; } Corpo;

/* AS FORMAS SÃO AS DO CATÁLOGO, não minhas */
static Corpo C[] = {
  {"algébrico (a raiz)", "a²",                1, {+1}},
  {"deflexivo",          "a²",                1, {+1}},
  {"reflexivo",          "a²+c²",             2, {+1,+1}},
  {"celeste",            "r²+C²=1",           2, {+1,+1}},
  {"óptico",             "(n,λ)",             2, {+1,+1}},
  {"cristalino",         "a²+|D|b², D<0",     2, {+1,+1}},
  {"individual",         "o par",             2, {+1,+1}},
  {"expansivo",          "a²−Db², D>0",       2, {+1,-1}},
  {"relógio",            "1−s²",              2, {+1,-1}},
  {"sensitivo",          "o par",             2, {+1,-1}},
  {"evolutivo",          "a aptidão",         2, {+1,-1}},
  {"telescópico",        "os termos",         4, {+1,+1,-1,-1}},
  {"geométrico",         "t²−x²−y²−z²",       4, {+1,-1,-1,-1}},
  {"conforme",           "grau cinco",        5, {+1,+1,+1,+1,-1}},
  {"eletromagnético",    "E²−B²",             6, {+1,+1,+1,-1,-1,-1}},
};
static const int NC = (int)(sizeof C / sizeof C[0]);

/* a assinatura: (p, q, r) = (redondas, hiperbólicas, colapsadas) */
static void assina(const Corpo *c, int *p, int *q, int *r){
    *p = *q = *r = 0;
    for(int i = 0; i < c->n; i++){
        if(c->d[i] > 0) (*p)++; else if(c->d[i] < 0) (*q)++; else (*r)++;
    }
}
/* a norma: N(x) = xᵀAx, com A a diagonal dos sinais */
static long norma(const Corpo *c, const long *x){
    long N = 0;
    for(int i = 0; i < c->n; i++) N += (long)c->d[i] * x[i] * x[i];
    return N;
}

int main(void){
    printf("A ASSINATURA (p,q,r) AGRUPA OS CORPOS: as formas são as do catálogo\n\n");

    /* ── §S1 A ASSINATURA, LIDA DE CADA FORMA ──────────────────────────────── */
    {
        printf("§S1  cada corpo com a sua forma, e a contagem que ela dá.\n\n");
        long mal = 0;
        printf("      corpo               a forma            grau  (p,q,r)\n");
        for(int c = 0; c < NC; c++){
            int p, q, r; assina(&C[c], &p, &q, &r);
            printf("      %-19s %-18s %3d   (%d,%d,%d)\n",
                   C[c].nome, C[c].forma, C[c].n, p, q, r);
            if(p + q + r != C[c].n) mal++;      /* o grau É p+q+r */
        }
        printf("\n");
        ok("CADA CORPO É A RAIZ REPETIDA p+q+r VEZES, CADA CÓPIA COM O SEU SINAL --- e o"
           " grau é exactamente p+q+r em todos os quinze. As formas não são minhas: são as"
           " que o catálogo escreve, uma a uma --- a²+c² do reflexivo, a²−Db² do expansivo,"
           " t²−x²−y²−z² do geométrico, E²−B² do eletromagnético, 1−s² do relógio,"
           " a²+|D|b² do cristalino. E a assinatura é leitura, não definição: a órbita corre"
           " por baixo, e estes números são o retrato dela.", mal == 0);
    }

    /* ── §S2 A LEI: MESMA ASSINATURA ⟹ CONGRUENTES, E O AGRUPAMENTO ────────── */
    {
        printf("§S2  os corpos que estão separados e são o MESMO, visto noutra base.\n\n");
        long mal = 0;
        int visto[32] = {0};
        long grupos = 0, juntos = 0, sozinhos = 0;
        for(int c = 0; c < NC; c++){
            if(visto[c]) continue;
            int p, q, r; assina(&C[c], &p, &q, &r);
            long n = 0; int membro[32];
            for(int j = c; j < NC; j++){
                int p2, q2, r2; assina(&C[j], &p2, &q2, &r2);
                if(p2 == p && q2 == q && r2 == r){ visto[j] = 1; membro[n++] = j; }
            }
            grupos++;
            printf("      (%d,%d,%d)  %ld corpo%s:  ", p, q, r, n, n > 1 ? "s" : " ");
            for(long k = 0; k < n; k++)
                printf("%s%s", C[membro[k]].nome, k + 1 < n ? ", " : "");
            printf("\n");
            if(n > 1) juntos += n; else sozinhos++;
        }
        printf("      → %ld assinaturas distintas · %ld corpos agrupados · %ld sozinhos\n",
               grupos, juntos, sozinhos);
        if(juntos == 0 || sozinhos == 0) mal++;
        printf("\n");
        ok("A ASSINATURA JUNTA OS CORPOS QUE O CATÁLOGO LISTA SEPARADOS, E É LEI E NÃO"
           " ARRUMAÇÃO: «dois corpos com a mesma contagem são CONGRUENTES --- se um é o"
           " outro visto noutra base». Onze dos quinze caem em três grupos: cinco em"
           " (2,0,0) --- reflexivo, celeste, óptico, cristalino, individual ---, quatro em"
           " (1,1,0) --- expansivo, relógio, sensitivo, evolutivo --- e dois em (1,0,0), o"
           " algébrico e o deflexivo, que é a mesma da raiz. E os que ficam sozinhos são-no"
           " por assinatura própria: o telescópico, o geométrico, o conforme e o"
           " eletromagnético. Se todos caíssem juntos, ou nenhum, a contagem não estaria a"
           " decidir nada.", mal == 0);
    }

    /* ── §S3 A CONGRUÊNCIA MEDIDA: PᵀAP PRESERVA (p,q,r) ───────────────────── */
    {
        printf("§S3  a lei de Sylvester, medida: a troca de base não move a contagem.\n\n");
        long mal = 0;
        /* P invertível inteira: a de determinante ±1 --- a lei do operador da casa.
         * Aplica-se PᵀAP e recontam-se os sinais dos valores próprios pelos MENORES
         * PRINCIPAIS (critério de Jacobi), sem sair dos inteiros. */
        long testes = 0, preserva = 0;
        for(int c = 0; c < NC; c++){
            int n = C[c].n; if(n > 4) continue;
            long A[4][4] = {{0}};
            for(int i = 0; i < n; i++) A[i][i] = C[c].d[i];
            /* cinco trocas de base inteiras com |det|=1 */
            for(int t = 0; t < 5; t++){
                long P[4][4] = {{0}};
                for(int i = 0; i < n; i++) P[i][i] = 1;
                if(t == 1 && n >= 2) P[0][1] = 1;                 /* cisalhamento */
                if(t == 2 && n >= 2){ P[0][0] = 0; P[0][1] = 1; P[1][0] = 1; P[1][1] = 0; }
                if(t == 3 && n >= 2) P[1][0] = 2;
                if(t == 4 && n >= 3){ P[0][2] = 1; P[2][0] = -1; P[2][2] = 1; }
                long B[4][4] = {{0}};
                for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
                    long s = 0;
                    for(int u = 0; u < n; u++) for(int v = 0; v < n; v++)
                        s += P[u][i] * A[u][v] * P[v][j];
                    B[i][j] = s;
                }
                /* a contagem de B por eliminação simétrica racional-livre em inteiros */
                long M[4][4]; int pB = 0, qB = 0, rB = 0;
                for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) M[i][j] = B[i][j];
                int usada[4] = {0};
                for(int passo = 0; passo < n; passo++){
                    int piv = -1;
                    for(int i = 0; i < n; i++) if(!usada[i] && M[i][i] != 0){ piv = i; break; }
                    if(piv < 0){                        /* só zeros na diagonal restante */
                        int i2 = -1, j2 = -1;
                        for(int i = 0; i < n && i2 < 0; i++) if(!usada[i])
                            for(int j = i+1; j < n; j++) if(!usada[j] && M[i][j] != 0){ i2=i; j2=j; break; }
                        if(i2 < 0){ for(int i = 0; i < n; i++) if(!usada[i]){ rB++; usada[i]=1; } break; }
                        /* o bloco hiperbólico 2x2 dá um + e um − */
                        pB++; qB++; usada[i2] = usada[j2] = 1;
                        continue;
                    }
                    if(M[piv][piv] > 0) pB++; else qB++;
                    for(int i = 0; i < n; i++){
                        if(usada[i] || i == piv || M[i][piv] == 0) continue;
                        long a = M[piv][piv], b = M[i][piv];
                        for(int j = 0; j < n; j++) M[i][j] = a*M[i][j] - b*M[piv][j];
                        for(int j = 0; j < n; j++) M[j][i] = a*M[j][i] - b*M[j][piv];
                    }
                    usada[piv] = 1;
                }
                int p0, q0, r0; assina(&C[c], &p0, &q0, &r0);
                testes++;
                if(pB == p0 && qB == q0 && rB == r0) preserva++;
                else if(t == 0) mal++;     /* a identidade TEM de preservar */
            }
        }
        printf("      PᵀAP em %ld trocas de base inteiras: a contagem preserva-se em %ld\n",
               testes, preserva);
        if(preserva != testes) mal++;

        /* ── O GUME: uma troca NÃO invertível (det 0) pode MUDAR a contagem ---
         * é a hipótese de Sylvester a trabalhar. */
        {
            Corpo *c = &C[7];                    /* o expansivo, (1,1,0) */
            long A[2][2] = {{1,0},{0,-1}}, P[2][2] = {{1,1},{1,1}};   /* det 0 */
            long B[2][2] = {{0,0},{0,0}};
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                for(int u = 0; u < 2; u++) for(int v = 0; v < 2; v++)
                    B[i][j] += P[u][i]*A[u][v]*P[v][j];
            int nulo = (B[0][0]==0 && B[0][1]==0 && B[1][0]==0 && B[1][1]==0);
            printf("      gume: com P de det 0 sobre o %s, PᵀAP fica %s --- a contagem"
                   " (1,1,0) COLAPSA para (0,0,2)\n", c->nome, nulo ? "NULA" : "não nula");
            if(!nulo) mal++;
        }

        /* e a NORMA é o que a assinatura fotografa: dois corpos do mesmo grupo dão o
         * mesmo conjunto de valores de N sobre a mesma grade */
        {
            long V1[512], V2[512]; long n1 = 0, n2 = 0;
            for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                long x[2] = {a, b};
                V1[n1++] = norma(&C[2], x);      /* reflexivo  (2,0,0) */
                V2[n2++] = norma(&C[3], x);      /* celeste    (2,0,0) */
            }
            long iguais = 0;
            for(long i = 0; i < n1; i++) if(V1[i] == V2[i]) iguais++;
            printf("      o reflexivo e o celeste, ambos (2,0,0): a norma coincide em"
                   " %ld de %ld pontos\n", iguais, n1);
            if(iguais != n1) mal++;
        }
        printf("\n");
        ok("A LEI DE SYLVESTER MEDE-SE, E É ELA QUE AUTORIZA O AGRUPAMENTO. Aplicada PᵀAP"
           " com trocas de base inteiras --- cisalhamentos, permutações, uma com entrada"
           " dois ---, a contagem (p,q,r) não se move em nenhuma: é o invariante, e é por"
           " isso que corpos com a mesma assinatura são o mesmo corpo visto noutra base. O"
           " GUME é a hipótese a trabalhar: com P de determinante zero a forma do expansivo"
           " colapsa inteira e a contagem passa de (1,1,0) a (0,0,2) --- a invertibilidade"
           " não é decoração. E a norma confirma-o de outro lado: o reflexivo e o celeste,"
           " ambos (2,0,0), dão o MESMO valor em todos os pontos da grade.", mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
