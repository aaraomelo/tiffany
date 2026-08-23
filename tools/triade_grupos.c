/* triade_grupos.c — A TRÍADE FECHADA NOS SETE GRUPOS, sobre os corpos JÁ COMPLETOS.
 *
 * A régua é UMA SÓ --- a ultramétrica dos endereços --- e as três faces
 * (elipse, parábola, hipérbole) são leituras do MESMO par. O catálogo já o diz:
 * «cada corpo completo recebe as três faces com t∈{−1,0,+1}, e em todas a régua
 * desce, PORQUE O ENDEREÇO É O PAR E ELE NÃO SABE DE t».
 *
 * Logo a régua mede-se sobre o endereço do corpo COMPLETO (a coluna levantada),
 * uma vez, e não uma vez por face.
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
    unlink("/tmp/tg.mem"); unlink("/tmp/tg.prog");
    if(!sql_abrir("/tmp/tg")) return 1;
    struct { const char *tab; const char *ass; const char *quais;
             int n; int d[6]; int lado; } G[] = {
      {"g100","(1,0,0)","algébrico, deflexivo",                        1,{+1},                12},
      {"g200","(2,0,0)","reflexivo, celeste, óptico, cristalino, ind.",2,{+1,+1},              6},
      {"g110","(1,1,0)","expansivo, relógio, sensitivo, evolutivo",    2,{+1,-1},              6},
      {"g220","(2,2,0)","telescópico",                                 4,{+1,+1,-1,-1},        3},
      {"g130","(1,3,0)","geométrico",                                  4,{+1,-1,-1,-1},        3},
      {"g410","(4,1,0)","conforme",                                    5,{+1,+1,+1,+1,-1},     2},
      {"g330","(3,3,0)","eletromagnético",                             6,{+1,+1,+1,-1,-1,-1},  2},
    };
    int NG = (int)(sizeof G/sizeof G[0]);
    printf("\n  assinatura  os corpos                        |I|  no banco   ΣN(−1)  ΣN(0)"
           "  ΣN(+1)  progressão  A RÉGUA\n");
    printf("  ──────────────────────────────────────────────────────────────────────────"
           "───────────────────────────\n");
    int fecham = 0, desce = 0, todas = 0;
    for(int g = 0; g < NG; g++){
        char q[240];
        /* o corpo COMPLETO: o par (a,b) e o endereço levantado, na mesma tabela.
         * A leitura que funde é a norma; o levantamento completa-a. */
        long total = 1;
        for(int i = 0; i < G[g].n; i++) total *= G[g].lado;
        long e[LV_MAX], A[LV_MAX], B[LV_MAX]; long nobj = 0;
        for(long t = 0; t < total && nobj < LV_MAX; t++){
            long u = t, idx[6];
            for(int i = 0; i < G[g].n; i++){ idx[i] = u % G[g].lado; u /= G[g].lado; }
            long N = 0;
            for(int i = 0; i < G[g].n; i++){
                long xi = idx[i];
                N += (long)G[g].d[i] * xi * xi;
            }
            A[nobj] = idx[0];
            B[nobj] = (G[g].n > 1 ? idx[1] : 0);
            e[nobj++] = N;
        }
        { long mn = e[0];
          for(long i = 1; i < nobj; i++) if(e[i] < mn) mn = e[i];
          if(mn < 0) for(long i = 0; i < nobj; i++) e[i] -= mn; }
        LvLevanta L;
        if(!lv_levanta(e, nobj, &L)){ printf("  %-11s NÃO COUBE\n", G[g].ass); continue; }
        /* o endereço levantado, compactado --- e o par que lhe corresponde */
        long end[LV_MAX], pa[LV_MAX], pb[LV_MAX];
        { long ord[LV_MAX], m = 0;
          for(long k = 0; k < L.n; k++){
              long v = lv_endereco(&L, k); int novo = 1;
              for(long j = 0; j < m; j++) if(ord[j] == v){ novo = 0; break; }
              if(novo) ord[m++] = v; }
          long viv = 0;
          for(long k = 0; k < L.n; k++){
              long v = lv_endereco(&L, k);
              for(long j = 0; j < m; j++) if(ord[j] == v){ end[k] = j; break; }
              /* o par: os objectos vivos levam o seu, os lugares abertos herdam
               * o par da sua fibra --- a base é a mesma */
              if(L.existia[k] && viv < nobj){ pa[k] = A[viv]; pb[k] = B[viv]; viv++; }
              else { pa[k] = L.base[k] % G[g].lado; pb[k] = (L.base[k]/G[g].lado) % G[g].lado; }
          } }
        snprintf(q,sizeof q,"DROP TABLE IF EXISTS %s",G[g].tab); sql_executa(q,&o2);
        snprintf(q,sizeof q,"CREATE TABLE %s (a RACIONAL, b RACIONAL)",G[g].tab);
        sql_executa(q,&o2);
        for(long k = 0; k < L.n; k++){
            snprintf(q,sizeof q,"INSERT INTO %s VALUES (%ld,%ld)",G[g].tab,pa[k],pb[k]);
            sql_executa(q,&o2);
        }
        snprintf(q,sizeof q,"SELECT count(*) FROM %s",G[g].tab); sql_executa(q,&o2);
        long entraram = o2.ok ? atol(o2.cell[0][0]) : -1;
        /* AS TRÊS FACES, de uma vez --- e a régua vem na mesma resposta, porque é
         * a mesma nas três: o endereço é o par e ele não sabe de t */
        snprintf(q,sizeof q,"SELECT triade(*) FROM %s",G[g].tab); sql_executa(q,&o);
        if(!o.ok){ printf("  %-11s RECUSADA: %s\n",G[g].ass,o.err); continue; }
        long nc = atol(o.cell[0][0]), np = atol(o.cell[0][1]), nh = atol(o.cell[0][2]);
        /* a coluna da régua do `triade` diz «desce», não «sim» --- a do `ultra`
         * é que diz «sim». Comparar com a palavra errada dava «NAO» em TODOS os
         * grupos com o motor a responder «desce»: o defeito era do leitor. */
        int reg = !strcmp(o.cell[0][4],"desce");
        int prog = (nc + nh == 2*np);
        printf("  %-11s %-32s %4ld %8ld %8ld %6ld %7ld  %-9s  %s\n",
               G[g].ass, G[g].quais, L.n, entraram, nc, np, nh,
               prog ? "FECHA" : "NAO", reg ? "desce" : "NAO");
        if(entraram != L.n){ printf("      ^^^ SÓ %ld DAS %ld ENTRARAM\n", entraram, L.n); continue; }
        if(nc == np && np == nh)
            printf("      ^^^ grau um: sem b as três faces COINCIDEM --- degenerescência,"
                   " não tríade\n");
        todas++;
        if(prog) fecham++;
        if(reg) desce++;
    }
    printf("  ──────────────────────────────────────────────────────────────────────────"
           "───────────────────────────\n");
    printf("  %d de %d no banco · %d com N(−1)+N(+1)=2N(0) · %d com a régua a descer"
           " --- e a régua é UMA SÓ\n\n", todas, NG, fecham, desce);
    sql_fechar();
    return 0;
}
