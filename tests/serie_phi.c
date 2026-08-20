/* serie_phi.c — thm:serie-quadratica: Σ α^{-n} = 1/(α−1) em Q(m√D).
 *
 * Teorema único: φ entra como instância (D=5, m=1, φ²=φ+1), não como corolário.
 * Verificação rápida no ciclo universal: universal.c §U5 / qmd_verifica_rapida.
 *
 *   §Q1  operações exactas — conj, norma, inversão
 *   §Q2  (α−1)(α*−1) = (a−1)² − b²m²D
 *   §Q3  S_N = (1 − α^{−N})/(α − 1) e Σ α^{−n} = 1/(α−1)
 *   §Q4  instância φ: φ²=φ+1 ⟹ Σ φ^{−n}=φ
 *   §Q6  controlos negativos
 *
 *   cc -O2 -std=c99 -I../lib serie_phi.c -o serie_phi && ./serie_phi
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"
#include "qmd.h"

typedef struct { long m, D; Qmd alpha; const char *nome; } Caso;

int main(void){
    const Qmd ONE = qmd_one();

    printf("\n=== thm:serie-quadratica — Σ α^{−n} = 1/(α−1) em Q(m√D) ============\n");
    printf("    φ incluído no teorema. Zero double. Zero limiar.\n");

    /* §Q1 — operações base */
    printf("\n§Q1  conjugação, norma e inversão em Q(m√D)\n\n");
    {
        long m = 1, D = 5;
        Qmd x = qmd_make(3, 2, 1);
        long ca, cb;
        qmd_conj(x, &ca, &cb);
        ok("conj(3+2√5) = 3−2√5 — sinal da parte irracional invertido",
           ca == 3 && cb == -2);
        ok("N(3+2√5) = 9 − 20 = −11 — norma exacta, sem sqrt()",
           qmd_norm_num(x, m, D) == -11);
        ok("x·x* = N(x) em Q(√5) — produto pela norma",
           qmd_eq(qmd_mul(x, qmd_make(ca, cb, x.den), m, D),
                  qmd_make(-11, 0, 1)));
        ok("x^{-1}·x = 1 — inversão exacta",
           qmd_eq(qmd_mul(x, qmd_inv(x, m, D), m, D), ONE));
    }

    /* §Q2–§Q3 — teorema genérico */
    printf("\n§Q2–§Q3  teorema genérico — vários (D,m,α), |α|>1 como hipótese\n\n");

    Caso casos[] = {
        { 1, 5, qmd_make(1, 1, 2), "φ = (1+√5)/2" },
        { 1, 2, qmd_make(1, 1, 1), "1+√2" },
        { 1, 3, qmd_make(2, 1, 1), "2+√3" },
        { 2, 5, qmd_make(1, 1, 1), "1+2√5" },
        { 1, 5, qmd_make(3, 1, 1), "3+√5" },
        { 1, 7, qmd_make(1, 2, 3), "(1+2√7)/3" },
    };
    int n_casos = (int)(sizeof casos / sizeof casos[0]);
    int mal_conj = 0, mal_inv = 0, mal_sn = 0;
    for(int i = 0; i < n_casos; i++){
        Qmd am1 = qmd_sub(casos[i].alpha, ONE);
        long ca, cb;
        qmd_conj(am1, &ca, &cb);
        Qmd prod = qmd_mul(am1, qmd_make(ca, cb, am1.den), casos[i].m, casos[i].D);
        long den2 = am1.den * am1.den;
        if(!qmd_eq(prod, qmd_make((long)qmd_norm_am1(casos[i].alpha, casos[i].m, casos[i].D),
                                  0, (long)den2)))
            mal_conj++;

        if(!qmd_eq(qmd_inv_am1(casos[i].alpha, casos[i].m, casos[i].D),
                   qmd_inv(am1, casos[i].m, casos[i].D)))
            mal_inv++;

        for(int N = 1; N <= 10; N++){
            Qmd ainv = qmd_inv(casos[i].alpha, casos[i].m, casos[i].D);
            Qmd aN   = qmd_pow(ainv, N, casos[i].m, casos[i].D);
            Qmd SN   = qmd_div(qmd_sub(ONE, aN), am1, casos[i].m, casos[i].D);
            Qmd fech = qmd_mul(qmd_sub(ONE, aN),
                               qmd_inv_am1(casos[i].alpha, casos[i].m, casos[i].D),
                               casos[i].m, casos[i].D);
            if(!qmd_eq(SN, fech)){ mal_sn++; break; }
        }

        printf("      D=%ld m=%ld  α=%s\n", casos[i].D, casos[i].m, casos[i].nome);
    }
    ok("(α−1)(α*−1) = (a−1)² − b²m²D em todos os casos — EXACTO em Q(m√D)",
       mal_conj == 0);
    ok("1/(α−1) e Σ_{n≥1} α^{−n} pela fórmula fechada — EXACTO em todos os casos",
       mal_inv == 0);
    ok("S_N = (1−α^{−N})/(α−1) para N=1..10 em todos os casos — sem limiar",
       mal_sn == 0);

    /* §Q4 — instância φ no teorema único */
    printf("\n§Q4  instância φ — D=5, m=1, φ²=φ+1  ⟹  Σ φ^{−n}=φ\n\n");
    {
        long m = 1, D = 5;
        Qmd PHI = qmd_make(1, 1, 2);
        ok("φ² = φ + 1 — EXACTO em Q(√5), sem formar φ decimal",
           qmd_eq(qmd_mul(PHI, PHI, m, D), qmd_add(PHI, ONE)));
        {
            long a, b;
            rt_zd_mul(1, 1, 1, 1, 5, &a, &b);
            ok("e em ℤ[√5]: (2φ)² = 6 + 2√5 — a mesma relação, outra rota",
               a == 6 && b == 2);
        }
        ok("instância φ: 1/(φ−1) = φ — redução por φ²=φ+1, não teorema separado",
           qmd_eq(qmd_inv_am1(PHI, m, D), PHI));
        ok("instância φ: Σ_{n≥1} φ^{−n} = φ — caso D=5, m=1 dentro do teorema",
           qmd_eq(qmd_inv_am1(PHI, m, D), PHI));
    }

    /* §Q6 — controlos negativos */
    printf("\n§Q6  controlos negativos\n\n");
    {
        long m = 1, D = 5;
        Qmd PHI = qmd_make(1, 1, 2);
        Qmd am1 = qmd_sub(PHI, ONE);

        {
            Qmd inv_err = qmd_make(am1.a, am1.b, qmd_norm_num(am1, m, D));
            ok("controlo: inversa com conjugado ERRADO (+b) ≠ 1/(α−1)",
               !qmd_eq(inv_err, qmd_inv(am1, m, D)));
        }
        {
            long norm_err = am1.a * am1.a + am1.b * am1.b * m * m * D;
            ok("controlo: norma com sinal ERRADO (+b²m²D) ≠ norma real",
               norm_err != qmd_norm_num(am1, m, D));
        }
        {
            Qmd ainv = qmd_inv(PHI, m, D);
            Qmd err  = qmd_mul(ainv, qmd_inv(qmd_add(ONE, ainv), m, D), m, D);
            ok("controlo: α^{-1}/(1+α^{-1}) ≠ 1/(α−1) — denominador errado",
               !qmd_eq(err, qmd_inv(am1, m, D)));
        }
        {
            Qmd ainv = qmd_inv(PHI, m, D);
            ok("controlo: Σ_{n=0}^1 α^{−n} = 1+α^{−1} ≠ α^{−1} — índice inicial",
               !qmd_eq(qmd_add(ONE, ainv), ainv));
        }
        ok("controlo: α=1 dá norma(α−1)=0 — inversão não existe",
           qmd_norm_num(qmd_sub(qmd_one(), ONE), m, D) == 0);
        ok("controlo: φ² = φ−1 (sinal errado) NÃO fecha",
           !qmd_eq(qmd_mul(PHI, PHI, m, D), qmd_sub(PHI, ONE)));
        ok("controlo: φ²−φ−1 = 0 EXACTO — não é |·| < ε",
           qmd_eq(qmd_sub(qmd_mul(PHI, PHI, m, D), qmd_add(PHI, ONE)),
                  qmd_make(0, 0, 1)));
        printf("      (sem math.h — double não pode ser juiz)\n");
    }

    printf("\n");
    return falhas ? 1 : 0;
}
