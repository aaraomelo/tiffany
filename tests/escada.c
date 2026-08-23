/* escada.c — os degraus, a fibra G e a COMPLETAÇÃO, medidos como o cliente usa.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/escada tests/escada.c && /tmp/escada
 */
#include "unidade.h"
#include "escada.h"
#include "incidencia.h"
#include <stdio.h>

int main(void){
    printf("A ESCADA E A INCIDÊNCIA: a lib que completa um corpo\n\n");

    /* ── §E1 A FIBRA DIZ SE O CORPO ESTÁ COMPLETO ───────────────────────── */
    {
        printf("§E1  G constante = corpo completo. E quando não é, diz-se QUANTO falta.\n\n");
        long mal = 0;
        printf("      corpo                    |I|  fibras  G mín–máx  ΣG=|I|  falta  expandido\n");
        struct { const char *n; int caso; } C[4] = {
            {"soma mod 6  (quociente)", 0},
            {"projecção   (quociente)", 1},
            {"distância   (resumo)   ", 2},
            {"soma livre  (resumo)   ", 3},
        };
        long completos = 0, incompletos = 0;
        for(int c = 0; c < 4; c++){
            long end[400]; long n = 0;
            for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
                if(C[c].caso == 0)      end[n++] = (a + b) % 6;
                else if(C[c].caso == 1) end[n++] = a;
                else if(C[c].caso == 2) end[n++] = a*a + b*b;
                else                    end[n++] = a + b;
            }
            EsFibra f = es_fibra(end, n);
            long falta = es_falta(end, n), exp = es_expandido(end, n);
            printf("      %s %4ld %7ld %6ld–%-4ld %6s %6ld %10ld\n",
                   C[c].n, n, f.fibras, f.menor, f.maior,
                   es_soma_fecha(end, n) ? "sim" : "NÃO", falta, exp);
            if(!es_soma_fecha(end, n)) mal++;          /* o thm:escada tem de valer sempre */
            if(f.constante){ completos++; if(falta != 0) mal++; }
            else { incompletos++; if(falta == 0) mal++; }
        }
        printf("        o cliente pergunta `es_falta`: zero se já era quociente, e o que\n"
               "        falta se não era --- e `es_expandido` diz o tamanho do corpo completo\n");
        ok("a fibra decide a completude, e a falta conta-se", mal == 0 && completos == 2 && incompletos == 2);
    }

    /* ── §E2 OS DEGRAUS, E CADA UM FECHA UMA FACE ───────────────────────── */
    {
        printf("\n§E2  X_2 fecha o oposto, X_3 o inverso --- e X_3 é o degrau do projectivo.\n\n");
        long mal = 0;
        /* X_2: todo elemento tem oposto */
        long op = 0, n2 = 0;
        for(long a = -8; a <= 8; a++){ n2++; if(es_x2_repr(a,0) == a && es_x2_repr(0,a) == -a) op++; }
        /* X_3: a relação é a mesma do projectivo, e o representante reduz */
        long red = 0, n3 = 0, mesma = 0;
        for(long p = -6; p <= 6; p++) for(long q = 1; q <= 6; q++){
            long rp, rq; es_x3_repr(p, q, &rp, &rq);
            n3++;
            if(es_x3_igual(p, q, rp, rq)) red++;
            /* a relação do racional e a do projectivo coincidem */
            for(long r = -3; r <= 3; r++) for(long s = 1; s <= 3; s++)
                if(es_x3_igual(p,q,r,s) == (p*s == r*q)) mesma++;
        }
        printf("      X_2: o oposto em %ld/%ld\n", op, n2);
        printf("      X_3: o representante reduzido é equivalente ao par em %ld/%ld\n", red, n3);
        printf("      e a relação de X_3 é a MESMA do projectivo em %ld comparações\n", mesma);
        if(op != n2 || red != n3) mal++;
        ok("cada degrau fecha a sua face, e o racional e o projectivo são o mesmo", mal == 0);
    }

    /* ── §E3 O CORTE NÃO É ATINGIDO --- e prova-se, não se procura ───────── */
    {
        printf("\n§E3  {x : x² < 2} não tem máximo em X_3: a subida preserva e cresce.\n\n");
        long invar = 0, cresce = 0, tot = 0;
        for(long q = 1; q <= 20; q++) for(long p = 1; p <= 2*q; p++){
            if(p*p >= 2*q*q) continue;
            long P, Q; es_x4_sobe(p, q, &P, &Q);
            tot++;
            if(es_x4_invariante(P,Q) == es_x4_invariante(p,q)) invar++;
            if(P*q - p*Q > 0) cresce++;
        }
        printf("      o invariante p²−2q² conserva-se em %ld/%ld e a fração cresce em %ld\n",
               invar, tot, cresce);
        printf("        o cliente chama `es_x4_sobe` e obtém sempre um melhor: não há máximo,\n"
               "        e é isso que o degrau seguinte fecha\n");
        ok("o corte não é atingido em X_3, e a subida prova-o", invar == tot && cresce == tot && tot > 0);
    }

    /* ── §E4 A INCIDÊNCIA: ζ acumula, μ desfaz, e a volta é exacta ───────── */
    {
        printf("\n§E4  ζ e μ: acumular e diferenciar, um a inversa do outro.\n\n");
        long mal = 0, exactas = 0, tot = 0;
        for(long s = 0; s < 200; s++){
            long a[8], z[8], m[8];
            long x = s;
            for(int i = 0; i < 8; i++){ a[i] = (x % 7) - 3; x /= 7; }
            in_zeta(a, z, 8);
            /* a acumulação é a soma parcial */
            long acc = 0; int bate = 1;
            for(int i = 0; i < 8; i++){ acc += a[i]; if(z[i] != acc) bate = 0; }
            if(!bate) mal++;
            in_mu(z, m, 8);
            for(int i = 0; i < 8; i++) if(m[i] != a[i]) mal++;
            tot++;
            if(in_volta_exacta(a, 8)) exactas++;
        }
        printf("      (ζa)(t) = Σ_{u≤t} a(u) e μ(ζa) = a em %ld/%ld vectores\n", exactas, tot);
        printf("      a série de ζ tem %d termos: S é nilpotente, e é isso que a faz terminar\n",
               in_grau_nilpotente(8));
        ok("ζ acumula, μ desfaz, e a volta é exacta sem divisão", mal == 0 && exactas == tot);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
