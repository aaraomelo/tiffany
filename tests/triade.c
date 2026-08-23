/* triade.c — a lib da tríade e da travessia, medida como o cliente a usa.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/triade tests/triade.c && /tmp/triade
 *
 * Não repete as provas do pgwire: mede que a LIB faz o que elas dizem, e que
 * recusa onde deve recusar.
 */
#include "unidade.h"
#include "triade.h"
#include <stdio.h>

int main(void){
    printf("A TRÍADE E A TRAVESSIA: a lib que o cliente usa\n\n");

    /* ── §T1 UM TIPO, TRÊS FACES: Δ = 4tb², e a dinâmica sai dele ────────── */
    {
        printf("§T1  um tipo, três faces --- e a face escolhe-se pelo t.\n\n");
        long mal = 0;
        struct { Tri x; const char *esp; } C[3] = {
            {tri_rot(3,2), "volta (elíptica)"},
            {tri_ext(3,2), "desliza (parabólica)"},
            {tri_hip(3,2), "foge (hiperbólica)"},
        };
        printf("      elemento        N        Δ         dinâmica\n");
        for(int i = 0; i < 3; i++){
            Tri x = C[i].x;
            long N = tri_norma(x), D = tri_disc(x);
            const char *d = tri_dinamica_nome(x);
            printf("      %ld + %ld·ω (t=%+ld) %5ld %8ld   %s\n", x.a, x.b, x.t, N, D, d);
            /* Δ = 4tb², e o det bate com a norma */
            if(D != 4*x.t*x.b*x.b) mal++;
            if(tri_det(x) != N) mal++;
            /* a dinâmica é a que se espera */
            if(__builtin_strcmp(d, C[i].esp)) mal++;
        }
        ok("um tipo dá as três faces: Δ = 4tb² e a dinâmica lê-se dele, sem iterar", mal == 0);
    }

    /* ── §T2 A NORMA COMPÕE-SE NAS TRÊS ─────────────────────────────────── */
    {
        printf("\n§T2  a norma é multiplicativa nas três faces.\n\n");
        long ok_n = 0, tot = 0;
        for(long t = -1; t <= 1; t++)
        for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
        for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
            Tri x = tri(a,b,t), y = tri(c,d,t);
            tot++;
            if(tri_norma(tri_prod(x,y)) == tri_norma(x)*tri_norma(y)) ok_n++;
        }
        printf("      N(xy) = N(x)N(y) em %ld/%ld produtos, nas três faces\n", ok_n, tot);
        ok("a norma compõe-se: a métrica do produto sai das dos factores", ok_n == tot);
    }

    /* ── §T3 O INVERSO RECUSA, E NÃO INVENTA ────────────────────────────── */
    {
        printf("\n§T3  o inverso existe sse N = ±1 --- e RECUSA quando não existe.\n\n");
        long mal = 0, deu = 0, recusou = 0;
        for(long t = -1; t <= 1; t++)
        for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++){
            Tri x = tri(a,b,t), inv;
            long N = tri_norma(x);
            int r = tri_inverso(x, &inv);
            if(r){
                deu++;
                /* se deu, x·inv tem de ser 1 exacto */
                Tri p = tri_prod(x, inv);
                if(!(p.a == 1 && p.b == 0)) mal++;
                if(N != 1 && N != -1) mal++;
            } else {
                recusou++;
                if(N == 1 || N == -1) mal++;
            }
        }
        printf("      inverso devolvido em %ld casos (todos com x·x⁻¹ = 1 exacto) e"
               " recusado em %ld\n", deu, recusou);
        printf("        a recusa não devolve valor: o cliente sabe que não há, e não"
               " recebe um número inventado\n");
        ok("o inverso é exacto onde existe e RECUSA onde não existe", mal == 0 && deu > 0 && recusou > 0);
    }

    /* ── §T4 A VALORAÇÃO DO EXTERIOR: a régua da def:arvore ─────────────── */
    {
        printf("\n§T4  a valoração do exterior, e ela é multiplicativa.\n\n");
        long ult = 0, tot = 0, est = 0, mult = 0, tm = 0;
        for(long a1 = -3; a1 <= 3; a1++) for(long b1 = -3; b1 <= 3; b1++)
        for(long a2 = -3; a2 <= 3; a2++) for(long b2 = -3; b2 <= 3; b2++)
        for(long a3 = -3; a3 <= 3; a3++) for(long b3 = -3; b3 <= 3; b3++){
            long v12 = tri_val(tri_ext(a1-a2, b1-b2));
            long v23 = tri_val(tri_ext(a2-a3, b2-b3));
            long v13 = tri_val(tri_ext(a1-a3, b1-b3));
            long mn = v12 < v23 ? v12 : v23;
            tot++;
            if(v13 >= mn) ult++;
            if(v13 > mn) est++;
        }
        long satura = 0;
        for(long a1 = -3; a1 <= 3; a1++) for(long b1 = -3; b1 <= 3; b1++)
        for(long a2 = -3; a2 <= 3; a2++) for(long b2 = -3; b2 <= 3; b2++){
            Tri x = tri_ext(a1,b1), y = tri_ext(a2,b2);
            long v1 = tri_val(x), v2 = tri_val(y);
            if(v1 >= TRI_INF || v2 >= TRI_INF) continue;   /* o zero fica de fora */
            long vp = tri_val(tri_prod(x,y));
            tm++;
            /* a lei: v(xy) = v(x)+v(y) enquanto a soma cabe no grau de
             * nilpotência; acima dele SATURA em ∞, porque ε² = 0. */
            if(v1 + v2 <= 1){ if(vp == v1 + v2) mult++; }
            else            { if(vp == TRI_INF){ mult++; satura++; } }
        }
        printf("      d = 2^{−v} é ultramétrica em %ld/%ld triplos (%ld estritos)\n",
               ult, tot, est);
        printf("      v(xy) = v(x)+v(y) em %ld/%ld, com %ld a SATURAR em ∞ --- a face\n"
               "        multiplicativa lida como SOMA, e ela satura no grau de nilpotência\n",
               mult, tm, satura);
        printf("        RELAÇÃO — em ℤ[ε] a valoração é truncada: v ∈ {0,1,∞}, e ε² = 0 é\n"
               "        exactamente o que faz a soma 1+1 cair em ∞ em vez de dar 2\n");
        ok("a valoração do exterior é a régua da def:arvore, e satura no grau de nilpotência",
           ult == tot && est > 0 && mult == tm && satura > 0);
    }

    /* ── §T5 AS TRÊS MÉDIAS: o trio fecha em g² = h·m ───────────────────── */
    {
        printf("\n§T5  as três médias, e o trio fecha sem uma raiz.\n\n");
        long fecha = 0, ordem = 0, tot = 0;
        for(long a = 1; a <= 30; a++) for(long b = 1; b <= 30; b++){
            tot++;
            if(tri_medias_fecham(a,b)) fecha++;
            if(tri_medias_ordem(a,b)) ordem++;
        }
        printf("      g² = h·m em %ld/%ld pares, e h ≤ g ≤ m em %ld --- uma desigualdade,"
               " três médias\n", fecha, tot, ordem);
        ok("o trio das médias fecha sobre si, exacto e sem raiz", fecha == tot && ordem == tot);
    }

    /* ── §T6 A TRAVESSIA: escreve-se, e o preço lê-se da permutação ──────── */
    {
        printf("\n§T6  a travessia entre duas leituras, e o preço em O(n).\n\n");
        long mal = 0;
        int n = 4; long M = 5;
        int r[4] = {0,1,2,3}, s[4] = {2,0,3,1};
        long N = 1; for(int i = 0; i < n; i++) N *= M;
        /* é bijeção */
        long visto[625]; for(long i = 0; i < N; i++) visto[i] = 0;
        long atinge = 0;
        for(long a = 0; a < N; a++){
            long b = tv_travessia(a, M, n, r, s);
            if(b < 0 || b >= N){ mal++; continue; }
            if(!visto[b]){ visto[b] = 1; atinge++; }
        }
        /* e a volta desfaz */
        long volta = 0;
        for(long a = 0; a < N; a++)
            if(tv_travessia(tv_travessia(a, M, n, r, s), M, n, s, r) == a) volta++;
        int q = tv_preco(n, r, s);
        /* e o q tem de bater com a MENOR profundidade sobre os objectos, medida
         * em dígitos --- é o supremo atingido, e é ele que valida a convenção */
        int q_medido = n;
        for(long a = 0; a < N; a++){
            long b = tv_travessia(a, M, n, r, s);
            if(a == b) continue;
            long x = a, y = b; long dx[8], dy[8];
            for(int i = 0; i < n; i++){ dx[i] = x % M; x /= M; dy[i] = y % M; y /= M; }
            for(int j = 0; j < n; j++){
                int i = n - 1 - j;
                if(dx[i] != dy[i]){ if(j < q_medido) q_medido = j; break; }
            }
        }
        printf("      T atinge %ld/%ld endereços, e S∘R⁻¹ seguida de R∘S⁻¹ devolve em %ld/%ld\n",
               atinge, N, volta, N);
        printf("      o preço: q = %d pela fórmula e %d pela varredura --- %s; lido de π em"
               " %d comparações, não nos %ld objectos\n",
               q, q_medido, q == q_medido ? "batem" : "NÃO BATEM", n, N);
        if(q != q_medido) mal++;
        /* e π = id dá preço nulo */
        int id[4] = {0,1,2,3};
        int q0 = tv_preco(n, id, id);
        printf("      com π = id o preço é q = %d (= n): não há travessia nem preço\n", q0);
        ok("a travessia escreve-se, é bijeção com volta, e o preço lê-se da permutação",
           mal == 0 && atinge == N && volta == N && q0 == n);
    }

    /* ── §T7 O CRITÉRIO DA LEITURA, como o cliente o chama ──────────────── */
    {
        printf("\n§T7  o critério: bem definida e separadora, com a igualdade DO CORPO.\n\n");
        /* o racional: a igualdade é ad = bc, não (a,b) = (c,d) */
        static long pa[64], pb[64];
        long end_cru[64], end_red[64]; int n = 0;
        for(long a = -3; a <= 3; a++) for(long b = 1; b <= 3; b++){
            pa[n] = a; pb[n] = b;
            end_cru[n] = (a+8)*8 + b;
            long x = a<0?-a:a, y = b;
            while(y){ long t = x%y; x = y; y = t; }
            if(x == 0) x = 1;
            end_red[n] = ((a/x)+8)*8 + (b/x);
            n++;
        }
        struct Ctx { long *a, *b; } ctx = { pa, pb };
        int igual_racional(int i, int j, void *c){
            struct Ctx *k = (struct Ctx *)c;
            return k->a[i]*k->b[j] == k->a[j]*k->b[i];
        }
        TvCriterio cru = tv_criterio(end_cru, n, igual_racional, &ctx);
        TvCriterio red = tv_criterio(end_red, n, igual_racional, &ctx);
        printf("      leitura CRUA     (a,b): bem definida %s · separa %s\n",
               cru.bem_definida ? "sim" : "NÃO", cru.separa ? "sim" : "NÃO");
        printf("      leitura REDUZIDA      : bem definida %s · separa %s\n",
               red.bem_definida ? "sim" : "NÃO", red.separa ? "sim" : "NÃO");
        printf("        a mesma família, duas leituras --- e o critério decide sem abrir"
               " o corpo\n");
        ok("o critério apanha a leitura que quebra e aprova a que endereça",
           !cru.bem_definida && cru.separa && red.bem_definida && red.separa);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
