/* eletromecanico.c — O ELECTROMECÂNICO É A FUSÃO DOS DOIS CORPOS DO BANCO.
 *
 * Não se acoplam equações genéricas: fundem-se os CORPOS que já lá estão --- o
 * mecânico (torque r×F) e o eletromagnético (E²−B², assinatura (3,3,0)) --- pelo
 * C_ent do paper, que entrelaça os dígitos de um nas posições pares e os do
 * outro nas ímpares. O corpo fundido é o electromecânico, e a dinâmica dele é o
 * circuito.
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include "levanta.h"
#include "fusao.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    SqlOut o, o2;
    char q[240];
    unlink("/tmp/em.mem"); unlink("/tmp/em.prog");
    if(!sql_abrir("/tmp/em")) return 1;

    /* os DOIS corpos, com as leituras que o catálogo lhes dá */
    long mec[64], ele[64]; long nm = 0, ne = 0;
    for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++)
        mec[nm++] = a*(b+1) - b*(a+1);          /* mecânico: o torque r×F */
    for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++)
        ele[ne++] = a*a - b*b;                  /* eletromagnético: E²−B² */
    { long mn = mec[0]; for(long i=1;i<nm;i++) if(mec[i]<mn) mn=mec[i];
      if(mn<0) for(long i=0;i<nm;i++) mec[i]-=mn; }
    { long mn = ele[0]; for(long i=1;i<ne;i++) if(ele[i]<mn) mn=ele[i];
      if(mn<0) for(long i=0;i<ne;i++) ele[i]-=mn; }

    printf("\n  OS DOIS CORPOS, cada um como está no banco:\n");
    struct { const char *nome; long *e; long n; } P[2] = {
        {"mecânico (torque r×F)", mec, nm}, {"eletromagnético (E²−B²)", ele, ne} };
    for(int i = 0; i < 2; i++){
        EsFibra f = es_fibra(P[i].e, P[i].n);
        printf("    %-26s %ld objectos · %ld fibras · G até %ld · falta %ld\n",
               P[i].nome, P[i].n, f.fibras, f.maior, es_falta(P[i].e, P[i].n));
    }

    /* ── A FUSÃO: C_ent entrelaça os dois endereços num só ─────────────────── */
    long fus[64]; long nf = 0;
    for(long i = 0; i < nm && i < ne; i++) fus[nf++] = fu_entrelaca(mec[i], ele[i], 6);
    EsFibra ff = es_fibra(fus, nf);
    printf("\n  A FUSÃO C_ent(mecânico, eletromagnético):\n");
    printf("    %ld objectos · %ld fibras · G até %ld · falta %ld\n",
           nf, ff.fibras, ff.maior, es_falta(fus, nf));

    /* a volta SEPARA: o fundido devolve os dois corpos exactos */
    long volta = 0;
    for(long i = 0; i < nf; i++){
        long a, b; fu_separa(fus[i], 6, &a, &b);
        if(a == mec[i] && b == ele[i]) volta++;
    }
    printf("    a volta devolve os DOIS exactos em %ld de %ld --- a fusão não perde\n",
           volta, nf);

    /* ── O FUNDIDO NO BANCO, completo, com régua e tríade ──────────────────── */
    LvLevanta L;
    if(!lv_levanta(fus, nf, &L)){ printf("    NÃO COUBE\n"); sql_fechar(); return 1; }
    long end[LV_MAX];
    { long ord[LV_MAX], m = 0;
      for(long k = 0; k < L.n; k++){ long v = lv_endereco(&L,k); int nv = 1;
        for(long j = 0; j < m; j++) if(ord[j]==v){ nv=0; break; } if(nv) ord[m++]=v; }
      for(long k = 0; k < L.n; k++){ long v = lv_endereco(&L,k);
        for(long j = 0; j < m; j++) if(ord[j]==v){ end[k]=j; break; } } }
    sql_executa("DROP TABLE IF EXISTS eletromec", &o2);
    sql_executa("CREATE TABLE eletromec (e INTEIRO)", &o2);
    for(long k = 0; k < L.n; k++){
        snprintf(q,sizeof q,"INSERT INTO eletromec VALUES (%ld)", end[k]);
        sql_executa(q,&o2);
    }
    sql_executa("SELECT count(*) FROM eletromec", &o2);
    long entraram = o2.ok ? atol(o2.cell[0][0]) : -1;
    sql_executa("SELECT fibra(*) FROM eletromec", &o);
    sql_executa("SELECT ultra(*) FROM eletromec", &o2);
    printf("\n  O CORPO ELECTROMECÂNICO no banco: %ld objectos (%ld entraram) · G = %s ·"
           " completo: %s · régua: %s\n", L.n, entraram,
           o.ok ? o.cell[0][2] : "?", o.ok ? o.cell[0][3] : "?",
           o2.ok ? o2.cell[0][4] : "?");

    /* ── E O CIRCUITO É A DINÂMICA DELE ────────────────────────────────────── */
    printf("\n  E O CIRCUITO SAI DA FUSÃO: cada corpo traz a sua taxa, e o acoplamento K\n");
    printf("    o eléctrico  L i' + R i + K ω = 0     (R/L é a taxa dele)\n");
    printf("    o mecânico   J ω' + b ω − K i = 0     (b/J é a taxa dele)\n\n");
    printf("    K     a equação do fundido      Δ   classe       o que faz\n");
    long R = 3, Lm = 1, b = 2, J = 1;
    for(long K = 0; K <= 3; K++){
        FuFundido f = fu_acopla(R, Lm, b, J, K);
        sql_executa("DROP TABLE IF EXISTS A", &o2);
        sql_executa("CREATE TABLE A (x RACIONAL, y RACIONAL)", &o2);
        sql_executa("INSERT INTO A VALUES (0,1)", &o2);
        snprintf(q,sizeof q,"INSERT INTO A VALUES (%ld,%ld)", -f.C, -f.B);
        sql_executa(q,&o2);
        sql_executa("SELECT edo(*) FROM A", &o);
        if(!o.ok){ printf("    K=%ld RECUSADA\n", K); continue; }
        printf("    K=%-3ld %-24s %5s  %-11s  %s\n", K, o.cell[0][0], o.cell[0][1],
               o.cell[0][2], !strcmp(o.cell[0][2],"eliptico") ? "OSCILA a decair"
                                                             : "decai sem oscilar");
        if(K == 0) printf("          ^ K=0: a fusão DEGENERA nos dois corpos separados,"
                          " com λ = −R/L e −b/J\n");
    }
    printf("\n");
    sql_fechar();
    return 0;
}
