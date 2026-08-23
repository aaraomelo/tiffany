/* quantico.c — a fase global, e porque a leitura é ρ e não ψ.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/quantico tests/quantico.c && /tmp/quantico
 */
#include "unidade.h"
#include "quantico.h"
#include <stdio.h>

int main(void){
    printf("O QUÂNTICO: a igualdade é a fase, e ela decide a leitura\n\n");

    /* ── §Q1 A IGUALDADE DO CORPO É A FASE ──────────────────────────────── */
    {
        printf("§Q1  ψ ~ uψ: o mesmo estado, e a órbita tem quatro nos pontos livres.\n\n");
        long livres = 0, fixos = 0, tot = 0;
        long tam[5] = {0,0,0,0,0};
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long d = -2; d <= 2; d++){
            Psi p = psi(a,b,c,d);
            if(psi_nulo(p)) continue;
            int n = psi_orbita(p);
            tot++;
            if(n >= 1 && n <= 4) tam[n]++;
            if(n == 4) livres++; else fixos++;
        }
        printf("      órbitas de tamanho 1: %ld · 2: %ld · 4: %ld  (de %ld estados)\n",
               tam[1], tam[2], tam[4], tot);
        printf("        a acção é livre onde a órbita tem quatro; os fixos são os estados\n"
               "        que alguma unidade não move\n");
        ok("a fase é a igualdade do corpo, e a órbita tem no máximo quatro",
           livres > 0 && tam[3] == 0 && livres + fixos == tot);
    }

    /* ── §Q2 AS AMPLITUDES QUEBRAM; ρ NÃO QUEBRA, E O QUE ELA AINDA FUNDE
     * TEM NOME: o CONJUGADO, onde ρ12 é real. ────────────────────────────── */
    {
        printf("\n§Q2  o critério decide sozinho --- e diz mais do que se esperava.\n\n");
        Psi P[700]; long n = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long d = -2; d <= 2; d++){
            Psi p = psi(a,b,c,d);
            if(psi_nulo(p)) continue;
            if(n < 700) P[n++] = p;
        }
        long q_amp = 0, f_amp = 0, q_rho = 0, f_rho = 0, pares = 0;
        long f_conj = 0, f_r12real = 0;
        for(long i = 0; i < n; i++) for(long j = 0; j < i; j++){
            int mesmo = psi_mesmo_estado(P[i], P[j]);
            int e_amp = (psi_end_amplitudes(P[i],4) == psi_end_amplitudes(P[j],4));
            Rho ri = psi_rho(P[i]), rj = psi_rho(P[j]);
            int e_rho = rho_igual(ri, rj);
            pares++;
            if(mesmo && !e_amp) q_amp++;
            if(e_amp && !mesmo) f_amp++;
            if(mesmo && !e_rho) q_rho++;
            if(e_rho && !mesmo){
                f_rho++;
                /* o que ρ ainda funde: o CONJUGADO. E ele só passa quando a
                 * parte imaginária de ρ12 é zero --- porque ρ(ψ̄) = conj(ρ(ψ)). */
                /* o que ρ ainda funde: pares que diferem por uma fase de
                 * norma 1 em ℚ(i) --- maior que o grupo das quatro unidades */
                if(psi_fase_racional(P[i], P[j])) f_conj++;
                if(ri.r12im == 0) f_r12real++;
            }
        }
        printf("      leitura        pares  quebras  fusões  bem def.  separa\n");
        printf("      amplitudes  %8ld %8ld %7ld %9s %7s\n", pares, q_amp, f_amp,
               q_amp ? "NAO" : "sim", f_amp ? "nao" : "sim");
        printf("      ρ = |ψ⟩⟨ψ|  %8ld %8ld %7ld %9s %7s\n", pares, q_rho, f_rho,
               q_rho ? "nao" : "sim", f_rho ? "nao" : "sim");
        printf("      das %ld fusões de ρ, %ld diferem por FASE RACIONAL de norma 1\n",
               f_rho, f_conj);
        printf("        RELAÇÃO NOVA — ρ mata a fase CONTÍNUA, e Z[i] só tem QUATRO\n"
               "        unidades: logo ρ funde estados que a igualdade do corpo não\n"
               "        relaciona --- eles diferem por uma fase como (3−4i)/5, de norma 1\n"
               "        e NÃO inteira. A leitura por ρ é bem definida sempre e separa a\n"
               "        menos dessa fase maior; dizer que «serve» seria medir com o grupo\n"
               "        errado\n");
        ok("as amplitudes quebram; ρ não quebra, e o que ela funde diferem por fase racional",
           q_amp > 0 && f_amp == 0 && q_rho == 0 && f_rho > 0 && f_conj == f_rho);
    }

    /* ── §Q3 E A PUREZA É EXACTA ────────────────────────────────────────── */
    {
        printf("\n§Q3  det ρ = 0 em todo estado --- a pureza sai em inteiros.\n\n");
        long puros = 0, tot = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            Psi p = psi(a,b,c,d);
            if(psi_nulo(p)) continue;
            tot++;
            if(rho_puro(psi_rho(p))) puros++;
        }
        printf("      det ρ = ρ11·ρ22 − |ρ12|² = 0 em %ld/%ld estados\n", puros, tot);
        printf("        e sai exacto, sem uma raiz: é a identidade de Lagrange nos\n"
               "        quatro inteiros da amplitude\n");
        ok("todo estado é puro, e a conta é exacta em inteiros", puros == tot);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
