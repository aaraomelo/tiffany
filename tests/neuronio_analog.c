/* neuronio_analog.c — a DUALIDADE do gato, em CORRENTES (par de neuronio.c).
 *
 * O gato e o esquilo são os dois lados do corpo, a MESMA peça com o sinal trocado (o
 * espelho 𝒥, microprocessador.tex §1: ANTILOG(log a + s·log b − s·log ref)):
 *   ⊗ gato ×σ    (o negro, s=+1, a CONVOLUÇÃO)  d_0 = m·c_0 + c_{n-1}   — SOBE
 *   ⊘ esquilo ×σ' (o branco, s=−1, a DECONVOLUÇÃO) c_{n-1} = d_0 − m·d_1  — DESCE
 * σσ'=−1 (a mão que segura). O ganho m·(·) é o TRANSLINEAR; a soma é o Kirchhoff.
 * O oráculo é o gato/esquilo DIGITAL: as correntes TÊM de bater — e batem porque a
 * malha translinear É o produto (analog.c §B.4), sem log nem exp.
 *
 *   §N1  Kirchhoff: os bits do documento são as correntes
 *   §N2  o gato A_m em SL: det = −1, D = m²+4 > 0, não fecha (hiperbólico)
 *   §N3  σσ' = −1 em ℤ[√D] — rt_zd_mul, sem formar a raiz
 *   §N4  gato sobe e esquilo desce: a volta é id, exacta em ℤ
 *   §N5  o gume: o sinal + em vez do − não desfaz
 *
 * LEI vs TRANSPORTE. log/exp da junção, σ impresso com sqrt, e o 1e-6 entre o
 * translinear e o digital eram o método — comparavam MEIOS. A lei é o produto, o
 * Kirchhoff, o companion A_m, e esquilo∘gato = id bit a bit.
 *
 *   cc -O2 -std=c99 -I lib tests/neuronio_analog.c -o neuronio_analog
 *   ./neuronio_analog CAMINHO [m] [K] [N]
 */
#include <stdio.h>
#include <stdlib.h>
#include "reta.h"
#include "unidade.h"

static const char *metal(long m){
    switch(m){ case 1: return "ouro (φ)"; case 2: return "prata"; case 3: return "bronze";
               default: return "metal m"; }
}

/* ⊗ gato ×σ: d_0 = m·c_0 + c_{n-1} (translinear + Kirchhoff), d_i = c_{i-1}  — SOBE */
static void gato_n(long *c, int n, long m){
    long v0 = c[0], vn1 = c[n-1];
    for(int i = n-1; i >= 1; i--) c[i] = c[i-1];
    c[0] = m*v0 + vn1;
}
/* ⊘ esquilo ×σ': c_j = d_{j+1}, c_{n-1} = d_0 − m·d_1  — DESCE */
static void esquilo_n(long *d, int n, long m){
    long d0 = d[0], d1 = d[1];
    for(int i = 0; i < n-1; i++) d[i] = d[i+1];
    d[n-1] = d0 - m*d1;
}
/* o DENTE: o sinal da involução trocado — não é o dual, e a volta NÃO fecha */
static void esquilo_mais(long *d, int n, long m){
    long d0 = d[0], d1 = d[1];
    for(int i = 0; i < n-1; i++) d[i] = d[i+1];
    d[n-1] = d0 + m*d1;
}

int main(int argc, char **argv){
    if(argc < 2) return 1;
    FILE *f = fopen(argv[1], "rb");
    if(!f) return 1;
    long mi = (argc > 2) ? atol(argv[2]) : 1;
    int N = (argc > 4) ? atoi(argv[4]) : 8;
    if(mi < 1) mi = 1;
    if(N < 2) N = 2;
    if(N > 8) N = 8;

    long b[8] = {0};
    int ch; long nch = 0;
    while((ch = fgetc(f)) != EOF){
        nch++;
        for(int p = 0; p < 8; p++) b[p] += (ch >> p) & 1;
    }
    fclose(f);

    long I_tot = 0;
    for(int p = 0; p < 8; p++) I_tot += b[p];

printf("\n=== NEURÓNIO ANALÓGICO: A DUALIDADE DO GATO, EM CORRENTES ================\n");
printf("    metal m=%ld (%s). O translinear É o produto; o Kirchhoff É a soma.\n",
       mi, metal(mi));
printf("    log/exp da junção e o 1e-6 contra o digital eram o método.\n");

printf("\n§N1  Kirchhoff: os bits do documento são as correntes.\n\n");
{
    printf("      %ld octetos → correntes por posição de bit:\n      ", nch);
    for(int p = 0; p < 8; p++) printf("I_%d=%ld%s", p, b[p], p<7?"  ":"");
    printf("\n      Σ I = %ld  (um bit = uma unidade de corrente)\n\n", I_tot);
    ok("o documento acende o nó: há corrente, e cada bit conta uma unidade — o Kirchhoff"
       " do analog.c §B.5, sem vírgula",
       nch > 0 && I_tot > 0);
}

printf("\n§N2  O gato A_m: det = −1, D = m²+4 > 0 — hiperbólico, não fecha.\n\n");
{
    /* A razão → σ com sqrt era transporte. A lei é o TRIAL do companion:
     * A_m = [[m,1],[1,0]], det = −1 = σσ', D = m²+4 > 0, ordem 0 no tecto. */
    int metais = 0, hip = 0, nao_fecha = 0;
    printf("      m     det    D=m²+4    ordem em tecto 20\n");
    for(long m = 1; m <= 3; m++){
        RtOp A = {{ m, 1, 1, 0 }};
        long det = rt_op_det(&A), D = m*m + 4;
        int per = rt_ordem_vector(&A, 1, 1, 20);
        metais++;
        if(det == -1 && D > 0 && rt_op_valido(&A)) hip++;
        if(per == 0) nao_fecha++;
        printf("      %-5ld %-6ld %-10ld %d%s\n",
               m, det, D, per, per ? "" : "  (não fecha — é o gato)");
    }
    printf("\n");
    ok("A_m tem det = −1 e D > 0 nos três metais — hiperbólico, σσ' é o determinante",
       hip == 3 && metais == 3);
    ok("e NÃO FECHA no tecto 20: o gato é o ouro, está do outro lado do trial",
       nao_fecha == 3);
}

printf("\n§N3  σσ' = −1 em ℤ[√D], sem formar a raiz.\n\n");
{
    /* 2σ = m + √D, 2σ' = m − √D, D = m²+4. O produto é m² − D = −4, logo σσ' = −1.
     * rt_zd_mul, exacto. sqrt((m+√D)/2) era o método. */
    int dual = 0;
    printf("      m     (2σ)(2σ') = a + b√D\n");
    for(long m = 1; m <= 3; m++){
        long za, zb;
        rt_zd_mul(m, 1, m, -1, m*m + 4, &za, &zb);
        int bate = (za == -4 && zb == 0);
        if(bate) dual++;
        printf("      %-5ld %ld %+ld√D   %s\n", m, za, zb, bate ? "σσ' = −1" : "NÃO");
    }
    printf("\n");
    ok("σσ' = −1 exacto em ℤ[√D] nos três metais — (2σ)(2σ') = −4, sem uma raiz",
       dual == 3);
}

printf("\n§N4  Gato sobe, esquilo desce: a volta é id, nas correntes do documento.\n\n");
{
    /* Em n=2 o gato É rt_opera(A_m) e o esquilo É rt_inverte — a mesma peça da lib.
     * Em n>2 é o companion: translinear no ganho, Kirchhoff no nó. */
    int volta_n = 0, dim = 0;
    printf("      n     correntes                  volta?\n");
    for(int n = 2; n <= N; n++){
        long cc[8] = {0}, orig[8];
        for(int p = 0; p < 8; p++) cc[p % n] += b[p];
        for(int i = 0; i < n; i++) orig[i] = cc[i];
        gato_n(cc, n, mi);
        esquilo_n(cc, n, mi);
        int volta = 1;
        for(int i = 0; i < n; i++) if(cc[i] != orig[i]) volta = 0;
        dim++;
        if(volta) volta_n++;
        printf("      R%-4d [", n);
        for(int i = 0; i < n; i++) printf("%ld%s", orig[i], i<n-1?",":"");
        printf("]  %s\n", volta ? "id" : "NÃO");
    }

    /* e em n=2, contra a lib: gato = A_m, esquilo = A_m⁻¹ */
    RtOp A = {{ mi, 1, 1, 0 }};
    int lib_ok = 0, lib_tot = 0;
    for(long p = -5; p <= 5; p++) for(long q = -5; q <= 5; q++){
        long c[2] = { p, q };
        gato_n(c, 2, mi);
        long gp, gq; rt_opera(&A, p, q, &gp, &gq);
        long ip, iq; int inv = rt_inverte(&A, gp, gq, &ip, &iq);
        lib_tot++;
        if(c[0]==gp && c[1]==gq && inv && ip==p && iq==q) lib_ok++;
    }

    long inv_falha = 0, inv_casos = 0, outro = 0;
    for(int n = 2; n <= N; n++)
        for(long v0 = -3; v0 <= 3; v0++) for(long v1 = -3; v1 <= 3; v1++){
            long d[8], o[8], e[8];
            for(int i = 0; i < n; i++){ d[i] = (i ? v1 + i : v0); o[i] = d[i]; e[i] = d[i]; }
            gato_n(d, n, mi); esquilo_n(d, n, mi);
            esquilo_n(e, n, mi); gato_n(e, n, mi);
            inv_casos++;
            int id1 = 1, id2 = 1;
            for(int i = 0; i < n; i++){
                if(d[i] != o[i]) id1 = 0;
                if(e[i] != o[i]) id2 = 0;
            }
            if(!id1) inv_falha++;
            if(id2) outro++;
        }
    printf("\n      gato = A_m da lib em %d de %d pares (n=2)\n", lib_ok, lib_tot);
    printf("      esquilo∘gato = id: %ld falhas em %ld  ·  gato∘esquilo = id: %ld/%ld\n\n",
           inv_falha, inv_casos, outro, inv_casos);
    ok("nas correntes do documento, esquilo desfaz o gato em toda a dimensão — volta exacta,"
       " sem 1e-6 entre meios",
       volta_n == dim && dim == N-1);
    ok("gato = A_m da lib (n=2) e as duas composições são id na grelha — o digital está"
       " certo, não só o analógico a imitá-lo",
       lib_ok == lib_tot && lib_tot == 121 && inv_falha == 0 && outro == inv_casos
       && inv_casos > 0);
}

printf("\n§N5  O gume: o sinal da involução, senão não desfaz.\n\n");
{
    /* É o mesmo sinal do forca.c §G7 / corpo_fisico §H7: + foge, − gira. Aqui o +
     * no esquilo NÃO é o dual — a volta falha FORA do eixo c_0=0. No eixo, gato
     * manda [0,c₁] → [c₁,0] e o sinal some: o caso degenerado não decide. */
    long dente = 0, eixo = 0, off = 0;
    for(long v0 = -3; v0 <= 3; v0++) for(long v1 = -3; v1 <= 3; v1++){
        long d[2] = { v0, v1 }, o[2] = { v0, v1 };
        gato_n(d, 2, mi);
        esquilo_mais(d, 2, mi);
        int fecha = (d[0] == o[0] && d[1] == o[1]);
        if(v0 == 0){ if(fecha) eixo++; }
        else { off++; if(!fecha) dente++; }
    }
    printf("      esquilo com sinal +: falha em %ld de %ld fora do eixo c₀=0\n",
           dente, off);
    printf("      no eixo c₀=0 o sinal some e a volta fecha em %ld (degenerado — não decide)\n\n",
           eixo);
    ok("com o sinal + o esquilo NÃO desfaz o gato fora do eixo — o dual é o sinal −,"
       " e o gume não vive do caso degenerado c₀=0",
       dente == off && off == 42 && eixo == 7);
}

printf("\n=== FECHO ==================================================================\n");
printf("    A dualidade fecha em correntes porque o translinear é o produto e o\n");
printf("    Kirchhoff é a soma: gato ×σ e esquilo ×σ' são a mesma peça, σσ'=−1.\n");
printf("    O 1e-6 entre meios saiu; o resíduo é zero em ℤ.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
