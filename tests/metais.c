/* metais.c — os metais, as réguas e a derivação, medidos como o cliente usa.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/metais tests/metais.c && /tmp/metais
 */
#include "unidade.h"
#include "metais.h"
#include "reguas.h"
#include "derivacao.h"
#include <stdio.h>

int main(void){
    printf("METAIS, RÉGUAS E DERIVAÇÃO: a lib chamada\n\n");

    /* ── §M1 σ⁻¹ = σ − m, sem fração ─────────────────────────────────────── */
    {
        printf("§M1  o inverso da unidade fundamental escreve-se sem uma fração.\n\n");
        long mal = 0;
        printf("      m    σ·(σ−m)      N(σ)\n");
        for(long m = 1; m <= 8; m++){
            Met s = met_sigma(m), si = met_sigma_inv(m);
            Met p = met_prod(s, si);
            long N = met_norma(s);
            if(m <= 4) printf("      %ld    %ld + %ld·σ %8ld\n", m, p.a, p.b, N);
            if(!(p.a == 1 && p.b == 0)) mal++;
            if(N != -1) mal++;                    /* N(σ) = 0 + 0 − 1 = −1 */
        }
        printf("        σ⁻¹ = σ − m nos oito metais: a face multiplicativa NÃO sai do degrau\n");
        ok("σ⁻¹ = σ − m, e o produto dá 1 exacto", mal == 0);
    }

    /* ── §M2 A NORMA COMPÕE-SE, E O INVERSO RECUSA ──────────────────────── */
    {
        printf("\n§M2  N(a+bσ) = a² + mab − b² é multiplicativa; o inverso recusa fora de ±1.\n\n");
        long okn = 0, tot = 0, deu = 0, rec = 0, mal = 0;
        for(long m = 1; m <= 4; m++)
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
            Met x = met(a,b,m);
            for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
                Met y = met(c,d,m);
                tot++;
                if(met_norma(met_prod(x,y)) == met_norma(x)*met_norma(y)) okn++;
            }
            Met inv;
            if(met_inverso(x, &inv)){
                deu++;
                Met p = met_prod(x, inv);
                if(!(p.a == 1 && p.b == 0)) mal++;
            } else { rec++; long N = met_norma(x); if(N == 1 || N == -1) mal++; }
        }
        printf("      N(xy) = N(x)N(y) em %ld/%ld; inverso dado em %ld e RECUSADO em %ld\n",
               okn, tot, deu, rec);
        ok("a norma compõe-se e o inverso recusa em vez de inventar",
           okn == tot && mal == 0 && deu > 0 && rec > 0);
    }

    /* ── §M3 A DESCIDA: [m;m,m,…] e os convergentes de norma ±1 ──────────── */
    {
        printf("\n§M3  a fração contínua de σ é [m;m,m,…], e daí as unidades.\n\n");
        long mal = 0;
        printf("      m    p/q (k=4)     N(p−qσ)\n");
        for(long m = 1; m <= 8; m++){
            int todos = 1;
            for(int k = 0; k < 6; k++){
                MetConv c = met_conv(m, k);
                long N = met_conv_norma(m, c);
                if(N != 1 && N != -1) todos = 0;
            }
            MetConv c4 = met_conv(m, 4);
            if(m <= 4) printf("      %ld    %ld/%ld %11ld\n", m, c4.p, c4.q, met_conv_norma(m, c4));
            if(!todos) mal++;
        }
        printf("        período UM, e é por isso que todo convergente é unidade\n");
        ok("os convergentes de [m;m,m,…] têm norma ±1 nos oito metais", mal == 0);
    }

    /* ── §R1 AS RÉGUAS: uma por primo, e a forte vale ───────────────────── */
    {
        printf("\n§R1  a régua p-ádica é ultramétrica --- uma por primo.\n\n");
        long mal = 0;
        printf("      p    forte vale em     v(xy)=v(x)+v(y)\n");
        for(long p = 2; p <= 5; p++){
            if(p == 4) continue;
            long f = 0, tf = 0, m = 0, tm = 0;
            for(long x = -6; x <= 6; x++) for(long y = -6; y <= 6; y++) for(long z = -6; z <= 6; z++){
                tf++; if(rg_forte_p(x,y,z,p)) f++;
            }
            for(long x = 1; x <= 12; x++) for(long y = 1; y <= 12; y++){
                tm++; if(rg_vp_multiplicativa(x,y,p)) m++;
            }
            printf("      %ld    %6ld/%-6ld    %6ld/%ld\n", p, f, tf, m, tm);
            if(f != tf || m != tm) mal++;
        }
        /* e a norma da rotação NÃO é ultramétrica: a forte falha */
        long falha = 0, tn = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            tn++; if(rg_norma_forte_falha(a,b,c,d)) falha++;
        }
        printf("      e a NORMA da rotação: a forte falha em %ld/%ld --- arquimediana\n",
               falha, tn);
        printf("        onde há valoração o produto vira SOMA; onde há norma fica PRODUTO\n");
        ok("as réguas p-ádicas são ultramétricas e a norma é arquimediana",
           mal == 0 && falha > 0);
    }

    /* ── §D1 A DERIVAÇÃO, E A NILPOTÊNCIA ───────────────────────────────── */
    {
        printf("\n§D1  d é derivação, e d^{n+1} = 0 --- o exterior subido de ordem.\n\n");
        long regra = 0, tot = 0;
        for(long s = 0; s < 300; s++){
            long f[6] = {0,0,0,0,0,0}, g[6] = {0,0,0,0,0,0};
            long x = s;
            for(int i = 0; i < 3; i++){ f[i] = (x % 5) - 2; x /= 5; }
            for(int i = 0; i < 3; i++){ g[i] = (x % 5) - 2; x /= 5; }
            tot++;
            if(dv_regra_produto(f, g, 6)) regra++;
        }
        printf("      d(fg) = f·dg + g·df em %ld/%ld pares\n", regra, tot);
        long nil = 0, tn = 0;
        printf("      grau  derivadas até zerar\n");
        for(int g = 0; g <= 5; g++){
            long p[8] = {0,0,0,0,0,0,0,0};
            p[g] = 1;
            int k = dv_grau_nilpotencia(p, 8);
            if(g <= 3) printf("      %4d  %ld\n", g, (long)k);
            tn++;
            if(k == g + 1) nil++;
        }
        /* e o núcleo são as constantes */
        long nuc = 0, tnu = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
            long p[4] = {a, b, 0, 0};
            tnu++;
            if(dv_no_nucleo(p, 4) == (b == 0)) nuc++;
        }
        printf("      d^{grau+1} = 0 em %ld/%ld graus, e o núcleo são as constantes em %ld/%ld\n",
               nil, tn, nuc, tnu);
        ok("d é derivação, é nilpotente em grau, e o núcleo são as constantes",
           regra == tot && nil == tn && nuc == tnu);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
