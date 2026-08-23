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
    unlink("/tmp/cb.mem"); unlink("/tmp/cb.prog");
    if(!sql_abrir("/tmp/cb")) return 1;
    struct { const char *nome; const char *tab; int caso; } C[] = {
        {"tríade  a+bω",         "triade",   0}, {"escada  X2 (oposto)", "escada",  1},
        {"racional  p/q",        "racional", 2}, {"áureo  cifra FC",     "aureo",   3},
        {"quadrático  Δ",        "quadrat",  4}, {"mórfico  erosão",     "morfico", 5},
        {"booleano  ⊕",          "boole",    6}, {"quântico  fase",      "quantic", 7},
        {"métrico  prefixo",     "metrico",  8}, {"entrópico",           "entrop",  9},
        {"matricial  det=±1",    "matric",  10}, {"cantor  π(a,b)",      "cantor", 11},
        {"eletromagnético E²−B²","eletro",  12}, {"mecânico  torque",    "mecanic",13},
        {"cósmico  (z,d)",       "cosmico", 14}, {"pneumático ≡ hidráulico","pneum",15},
    };
    int N = (int)(sizeof C / sizeof C[0]);
    printf("\n  corpo                       |I|  no banco  G  completo  falta  régua\n");
    printf("  ──────────────────────────────────────────────────────────────────\n");
    int completos = 0, desce = 0;
    for(int c = 0; c < N; c++){
        char q[240];
        long e[64]; long n = 0;
        for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
            long v;
            switch(C[c].caso){
              case 0:  v = (a*a - b*b) % 12; break;
              case 1:  v = (a - b + 6) % 6; break;
              case 2:  v = a*6 + b; break;
              case 3:  v = ((a+1)*8 + (b+1)) % 36; break;
              case 4:  v = (b*b - 4*a) % 12; break;
              case 5:  v = (a < b ? a : b); break;
              case 6:  v = (a ^ b); break;
              case 7:  v = a*6 + b; break;
              case 8:  v = a*6 + b; break;
              case 9:  v = a + b; break;
              case 10: v = ((a+b) % 2) ? 1 : 35; break;
              case 11: v = (a+b)*(a+b+1)/2 + b; break;
              case 12: v = a*a - b*b; break;
              case 13: v = a*(b+1) - b*(a+1); break;
              case 14: { long z=a+1,d=b+1,ex=0,t=z*d; while(t>1){ex++;t>>=1;} v=ex*6+(z*d)%6; } break;
              default: v = (a+1)*(b+1);
            }
            e[n++] = v;
        }
        { long mn = e[0];
          for(long i=1;i<n;i++) if(e[i]<mn) mn=e[i];
          if(mn<0) for(long i=0;i<n;i++) e[i]-=mn; }
        /* COMPLETA: o levantado é o que fica no banco */
        LvLevanta L;
        if(!lv_levanta(e, n, &L)){ printf("  %-26s  NAO COUBE\n", C[c].nome); continue; }
        /* OS ENDEREÇOS COMPACTAM-SE para 0..n-1 pela enumeração: é bijeção, e a
         * fibra não vê o NOME do endereço. Sem isto o banco RECUSA as linhas que
         * não cabem no envelope Word_8 (-128..127) --- e recusa bem, sem truncar
         * em silêncio, mas o corpo ficava por inserir e eu lia «completo» sobre
         * a fatia que entrou. */
        long end[LV_MAX];
        { long ord[LV_MAX], m = 0;
          for(long k = 0; k < L.n; k++){
              long v = lv_endereco(&L, k); int novo = 1;
              for(long j = 0; j < m; j++) if(ord[j] == v){ novo = 0; break; }
              if(novo) ord[m++] = v; }
          for(long k = 0; k < L.n; k++){
              long v = lv_endereco(&L, k);
              for(long j = 0; j < m; j++) if(ord[j] == v){ end[k] = j; break; } } }
        snprintf(q, sizeof q, "DROP TABLE IF EXISTS %s", C[c].tab); sql_executa(q, &o2);
        snprintf(q, sizeof q, "CREATE TABLE %s (e INTEIRO)", C[c].tab); sql_executa(q, &o2);
        for(long k = 0; k < L.n; k++){
            snprintf(q, sizeof q, "INSERT INTO %s VALUES (%ld)", C[c].tab, end[k]);
            sql_executa(q, &o2);
        }
        snprintf(q, sizeof q, "SELECT count(*) FROM %s", C[c].tab);
        sql_executa(q, &o2);
        long entraram = o2.ok ? atol(o2.cell[0][0]) : -1;
        snprintf(q, sizeof q, "SELECT fibra(*) FROM %s", C[c].tab);
        sql_executa(q, &o);
        if(!o.ok){ printf("  %-26s  RECUSADA\n", C[c].nome); continue; }
        int comp = !strcmp(o.cell[0][3], "sim");
        snprintf(q, sizeof q, "SELECT ultra(*) FROM %s", C[c].tab);
        sql_executa(q, &o2);
        int reg = o2.ok && !strcmp(o2.cell[0][4], "sim");
        printf("  %-26s %4ld %4ld %3s   %-8s %5s   %s\n", C[c].nome, L.n, entraram,
               o.cell[0][2], comp ? "SIM" : "nao", o.cell[0][4], reg ? "desce" : "NAO");
        if(entraram != L.n){ printf("      ^^^ SÓ %ld DAS %ld LINHAS ENTRARAM\n", entraram, L.n); continue; }
        if(comp) completos++;
        if(reg) desce++;
    }
    printf("  ──────────────────────────────────────────────────────────────────\n");
    printf("  %d de %d COMPLETOS · %d de %d com a régua a descer\n\n", completos, N, desce, N);
    sql_fechar();
    return 0;
}
