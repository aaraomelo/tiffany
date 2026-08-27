/* zetamu.c — ζ ACUMULA, μ DESACUMULA, E O PAR FECHA COM RESÍDUO 0.
 *
 *   cc -O2 -std=c99 -Ilib -Itests -o /tmp/zetamu tests/zetamu.c && /tmp/zetamu
 *
 * O `fisica.tex thm:mu` e `thm:zetamu`: o motor acumula (o G do `GROUP BY` é a
 * convolução com ζ) mas não desacumulava. A inversa de ζ é μ, e na ordem total
 * μ é a diferença finita. Este medidor mostra que o par é exacto e que a
 * trajetória volta da contagem — e confronta a mesma álgebra na árvore dos
 * divisores (o `dirichlet.h`), onde μ é a função de Möbius clássica.
 *
 *   §Z1  ζ∘μ = μ∘ζ = id — o par fecha, resíduo 0, em toda a largura
 *   §Z2  a diferença finita É o μ do thm:mu (o núcleo 1,−1,0)
 *   §Z3  a trajetória volta: a_x(t) = G_t(x) − G_{t−1}(x), exacto nas visitas
 *   §Z4  a mesma inversão na ordem por divisibilidade (fis:cor:mobius, dirichlet.h)
 *   §Z5  o levantamento: k(i)=G_i(π(i)) sobe 1 por visita — o G̃=1
 */
#include <string.h>
#include "unidade.h"
#include "incidencia.h"
/* §Z4 confronta com a ordem por divisibilidade. O `lib/dirichlet.h` realiza-a
 * (dl_acumula, dl_inverte), mas arrasta o `numeros.h` (fatoração); aqui o μ dos
 * divisores computa-se pela sua definição — μ = 1⁻¹ pela recorrência — para o
 * medidor ficar auto-contido. É o MESMO objecto, e o `dirichlet.h` é onde a casa
 * o guarda como biblioteca. */
#define DN 120

/* uma sucessão fabricada pelo índice — sem RNG, para o medidor não ter semente */
static long fab(long t, long sal){ return ((t * 2654435761L + sal * 40503L) % 97) - 48; }

int main(void){
    printf("\n=== ζ ACUMULA, μ DESACUMULA: o par que faltava ao motor ===\n\n");

    /* ── §Z1 ── o par fecha em toda a largura ──────────────────────────────── */
    printf("  §Z1  ζ∘μ = μ∘ζ = id\n");
    {   int fecha = 1; long N = 0;
        for(long n = 1; n <= 200; n++){
            long a[200], b[200], a2[200], b2[200];
            for(long t = 0; t < n; t++) a[t] = fab(t, n);
            inc_zeta(a, b, n); inc_mu(b, a2, n);           /* μ desfaz ζ */
            for(long t = 0; t < n; t++) if(a2[t] != a[t]) fecha = 0;
            inc_mu(a, a2, n); inc_zeta(a2, b2, n);         /* ζ desfaz μ */
            for(long t = 0; t < n; t++) if(b2[t] != a[t]) fecha = 0;
            N += n;
        }
        ok("μ desfaz ζ e ζ desfaz μ, resíduo 0, em toda a largura até 200", fecha);
        printf("      %ld posições varridas nas duas voltas\n", N * 2);
    }

    /* ── §Z2 ── a diferença finita É o μ do thm:mu ─────────────────────────── */
    printf("\n  §Z2  a diferença finita é o núcleo μ(u,t)\n");
    {   int bate = 1, nucleo = 1;
        long n = 64, b[64], a[64];
        for(long t = 0; t < n; t++) b[t] = fab(t, 7);
        inc_mu(b, a, n);
        for(long t = 0; t < n; t++){
            /* (b*μ)(t) = Σ_{u≤t} b(u)·μ(u,t) — e μ só é ≠0 em u=t e u=t−1 */
            long s = 0;
            for(long u = 0; u <= t; u++) s += b[u] * inc_mu_nucleo(u, t);
            if(s != a[t]) bate = 0;
        }
        if(inc_mu_nucleo(5,5)!=1 || inc_mu_nucleo(4,5)!=-1 || inc_mu_nucleo(3,5)!=0) nucleo = 0;
        ok("inc_mu iguala a convolução com o núcleo μ (1 na diagonal, −1 abaixo)", bate);
        ok("e o núcleo é o do thm:mu: μ(t,t)=1, μ(t−1,t)=−1, resto 0", nucleo);
    }

    /* ── §Z3 ── a trajetória volta da contagem ─────────────────────────────── */
    printf("\n  §Z3  a recuperação é a deconvolução com μ (thm:zetamu (2))\n");
    {   /* uma realização π: 300 passos sobre 8 valores, fabricada */
        long n = 300, pi[300];
        for(long t = 0; t < n; t++) pi[t] = (fab(t, 3) + 96) % 8;
        int volta = 1, salto = 1;
        for(long x = 0; x < 8; x++){
            long G[300], a[300], axr[300];
            inc_indicador(pi, n, x, a);       /* o indicador verdadeiro */
            inc_G(pi, n, x, G);               /* G_t(x) = (a_x * ζ)(t) */
            inc_mu(G, axr, n);                /* a diferença finita devolve a_x? */
            for(long t = 0; t < n; t++){
                if(axr[t] != a[t]) volta = 0;
                /* G_t − G_{t−1} vale 1 exactamente quando π(t)=x, senão 0 */
                if(axr[t] != (pi[t] == x)) salto = 0;
            }
        }
        ok("μ sobre {G_t(x)} devolve o indicador a_x — a trajetória inteira volta", volta);
        ok("e a diferença vale 1 exactamente nas visitas de x, 0 no resto", salto);
        printf("      300 passos × 8 valores, sem nada guardado da trajetória\n");
    }

    /* ── §Z4 ── a mesma inversão, na ordem por divisibilidade ───────────────── */
    printf("\n  §Z4  a álgebra não muda, só a ordem (fis:cor:mobius)\n");
    {   /* o μ dos divisores pela sua definição: μ = 1⁻¹, isto é Σ_{d|n} μ(d)=ε(n) */
        long mu[DN + 1];
        mu[1] = 1;
        for(long n = 2; n <= DN; n++){
            long s = 0;
            for(long d = 1; d < n; d++) if(n % d == 0) s += mu[d];
            mu[n] = -s;                         /* garante Σ_{d|n} μ(d) = 0 para n>1 */
        }
        /* F = f*1 (acumula sobre os divisores); depois F*μ tem de devolver f */
        long f[DN + 1], F[DN + 1], fr[DN + 1];
        for(long n = 1; n <= DN; n++) f[n] = (n * 7 + 3) % 11;      /* f fabricada */
        for(long n = 1; n <= DN; n++){ long s = 0; for(long d = 1; d <= n; d++) if(n % d == 0) s += f[d]; F[n] = s; }
        for(long n = 1; n <= DN; n++){ long s = 0; for(long d = 1; d <= n; d++) if(n % d == 0) s += mu[d] * F[n/d]; fr[n] = s; }
        int volta = 1; for(long n = 1; n <= DN; n++) if(fr[n] != f[n]) volta = 0;
        ok("F=f*1 e depois F*μ devolve f: ζ e μ invertem-se também nos divisores", volta);
        /* e a assinatura, a mesma frase da ordem total: μ * 1 = ε */
        int eps = 1;
        for(long n = 1; n <= DN; n++){ long s = 0; for(long d = 1; d <= n; d++) if(n % d == 0) s += mu[d]; if(s != (n == 1)) eps = 0; }
        ok("μ * 1 = ε — μ é o inverso de 1, como na ordem total é a diferença", eps);
    }

    /* ── §Z5 ── o levantamento na fibra ─────────────────────────────────────── */
    printf("\n  §Z5  o levantamento: k(i) sobe 1 por visita (thm:zetamu (3))\n");
    {   long n = 300, pi[300], k[300];
        for(long t = 0; t < n; t++) pi[t] = (fab(t, 5) + 96) % 8;
        inc_levanta(pi, n, k);
        /* a diferença finita de k, RESTRITA a cada fibra, é 1 em cada visita:
         * k no r-ésimo encontro de x vale r, logo k sobe exactamente 1 de visita
         * a visita da MESMA célula. Verifica-se por valor. */
        int um_por_visita = 1;
        for(long x = 0; x < 8; x++){
            long esperado = 0;
            for(long t = 0; t < n; t++) if(pi[t] == x){ esperado++; if(k[t] != esperado) um_por_visita = 0; }
        }
        ok("k(i)=G_i(π(i)) vale r no r-ésimo encontro — a fibra é {1,…,G(x)}, G̃=1",
           um_por_visita);
    }

    printf("\n  %d unidades, %d falha(s)\n", unidades, falhas);
    if(!falhas) printf("  o par fecha: ζ acumula, μ desacumula, e a volta é exacta.\n\n");
    return falhas ? 1 : 0;
}
