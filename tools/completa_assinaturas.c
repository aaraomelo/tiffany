/* completa_assinaturas.c — OS SETE GRUPOS DE ASSINATURA, completos no banco.
 *
 * O catálogo agrupa os corpos pela contagem (p,q,r): «dois corpos com a mesma
 * contagem são congruentes --- se um é o outro visto noutra base». Cada grupo é
 * UM corpo, e é ele que vai para o banco: a leitura é a NORMA N(x)=xᵀAx, que
 * funde; o levantamento pelo ι/π completa-a; e a régua vem atrás.
 *
 *   cc -O2 -std=c99 -Ilib -Ibanco -Itests -DSQL_NO_MAIN -DPGWIRE_NO_MAIN \
 *      tools/completa_assinaturas.c banco/sql.c banco/pgwire.c -lm -o /tmp/ca
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include "levanta.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    SqlOut o, o2;
    unlink("/tmp/ca.mem"); unlink("/tmp/ca.prog");
    if(!sql_abrir("/tmp/ca")) return 1;

    struct { const char *tab; const char *ass; const char *quais;
             int n; int d[6]; int lado; } G[] = {
      {"a100", "(1,0,0)", "algébrico (a raiz), deflexivo",              1, {+1},                 12},
      {"a200", "(2,0,0)", "reflexivo, celeste, óptico, cristalino, individual", 2, {+1,+1},      6},
      {"a110", "(1,1,0)", "expansivo, relógio, sensitivo, evolutivo",   2, {+1,-1},              6},
      {"a220", "(2,2,0)", "telescópico",                                4, {+1,+1,-1,-1},        3},
      {"a130", "(1,3,0)", "geométrico",                                 4, {+1,-1,-1,-1},        3},
      {"a410", "(4,1,0)", "conforme (grau cinco)",                      5, {+1,+1,+1,+1,-1},     2},
      {"a330", "(3,3,0)", "eletromagnético",                            6, {+1,+1,+1,-1,-1,-1},  2},
    };
    int NG = (int)(sizeof G / sizeof G[0]);

    printf("\n  assinatura  os corpos que ela junta                 obj  fibras  G  →  |I|"
           "  no banco  G  régua\n");
    printf("  ─────────────────────────────────────────────────────────────────────────"
           "──────────────────────\n");
    int completos = 0, desce = 0, todas = 0;
    for(int g = 0; g < NG; g++){
        /* os objectos: a grade de lado `lado` em `n` coordenadas */
        long e[LV_MAX]; long nobj = 0;
        long idx[6] = {0};
        long total = 1;
        for(int i = 0; i < G[g].n; i++) total *= G[g].lado;
        for(long t = 0; t < total && nobj < LV_MAX; t++){
            long u = t;
            for(int i = 0; i < G[g].n; i++){ idx[i] = u % G[g].lado; u /= G[g].lado; }
            /* A LEITURA É A NORMA: N(x) = Σ d_i · x_i²  --- e ela FUNDE */
            long N = 0;
            for(int i = 0; i < G[g].n; i++){
                long xi = idx[i] - G[g].lado/2;
                N += (long)G[g].d[i] * xi * xi;
            }
            e[nobj++] = N;
        }
        { long mn = e[0];
          for(long i = 1; i < nobj; i++) if(e[i] < mn) mn = e[i];
          if(mn < 0) for(long i = 0; i < nobj; i++) e[i] -= mn; }
        EsFibra f = es_fibra(e, nobj);
        LvLevanta L;
        if(!lv_levanta(e, nobj, &L)){
            printf("  %-11s %-40s %4ld  NÃO COUBE\n", G[g].ass, G[g].quais, nobj);
            continue;
        }
        /* compacta pela enumeração: bijecção, a fibra não vê o nome */
        long end[LV_MAX];
        { long ord[LV_MAX], m = 0;
          for(long k = 0; k < L.n; k++){
              long v = lv_endereco(&L, k); int novo = 1;
              for(long j = 0; j < m; j++) if(ord[j] == v){ novo = 0; break; }
              if(novo) ord[m++] = v; }
          for(long k = 0; k < L.n; k++){
              long v = lv_endereco(&L, k);
              for(long j = 0; j < m; j++) if(ord[j] == v){ end[k] = j; break; } } }
        char q[240];
        snprintf(q, sizeof q, "DROP TABLE IF EXISTS %s", G[g].tab); sql_executa(q, &o2);
        snprintf(q, sizeof q, "CREATE TABLE %s (e INTEIRO)", G[g].tab); sql_executa(q, &o2);
        for(long k = 0; k < L.n; k++){
            snprintf(q, sizeof q, "INSERT INTO %s VALUES (%ld)", G[g].tab, end[k]);
            sql_executa(q, &o2);
        }
        snprintf(q, sizeof q, "SELECT count(*) FROM %s", G[g].tab); sql_executa(q, &o2);
        long entraram = o2.ok ? atol(o2.cell[0][0]) : -1;
        snprintf(q, sizeof q, "SELECT fibra(*) FROM %s", G[g].tab); sql_executa(q, &o);
        if(!o.ok){ printf("  %-11s RECUSADA\n", G[g].ass); continue; }
        int comp = !strcmp(o.cell[0][3], "sim");
        snprintf(q, sizeof q, "SELECT ultra(*) FROM %s", G[g].tab); sql_executa(q, &o2);
        int reg = o2.ok && !strcmp(o2.cell[0][4], "sim");
        printf("  %-11s %-40s %4ld %5ld %3ld  → %4ld %7ld %4s  %s\n",
               G[g].ass, G[g].quais, nobj, f.fibras, f.maior, L.n, entraram,
               o.cell[0][2], reg ? "desce" : "NAO");
        if(entraram != L.n){
            printf("      ^^^ SÓ %ld DAS %ld ENTRARAM --- não conta\n", entraram, L.n);
            continue;
        }
        todas++;
        if(comp) completos++;
        if(reg) desce++;
    }
    printf("  ─────────────────────────────────────────────────────────────────────────"
           "──────────────────────\n");
    printf("  %d de %d grupos com TODAS as linhas no banco · %d completos · %d com a régua"
           " a descer\n\n", todas, NG, completos, desce);
    sql_fechar();
    return 0;
}
