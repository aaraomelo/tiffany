/* faces.c — o par de faces nos seus extremos: o lógico e o simétrico.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/faces tests/faces.c && /tmp/faces
 */
#include "unidade.h"
#include "faces.h"
#include <stdio.h>

int main(void){
    printf("AS FACES NOS EXTREMOS: o lógico e o simétrico\n\n");

    /* ── §F1 O LÓGICO: as faces colapsam ────────────────────────────────── */
    {
        printf("§F1  ∂x = x, e a face multiplicativa no seu extremo mais pobre.\n\n");
        long colapsam = 0, tot = 0;
        for(unsigned x = 0; x < 64; x++){ tot++; if(lg_faces_colapsam(x)) colapsam++; }
        unsigned topo = 63, inv;
        long tem = 0;
        for(unsigned x = 0; x < 64; x++) if(lg_inverso(x, topo, &inv)) tem++;
        printf("      cada elemento é o seu próprio oposto em %ld/%ld\n", colapsam, tot);
        printf("      no AND, %ld dos %ld têm inverso --- e a lib di-lo pela contagem: %ld\n",
               tem, tot, lg_quantos_invertem(topo));
        printf("        o par está o mais desequilibrado possível, e é por isso que este\n"
               "        andar é XOR: a dobra da face aditiva é a identidade\n");
        ok("no lógico as faces colapsam e a multiplicativa é a mais pobre",
           colapsam == tot && tem == 1 && lg_quantos_invertem(topo) == 1);
    }

    /* ── §F2 O SIMÉTRICO: fecha na soma, não no produto ─────────────────── */
    {
        printf("\n§F2  AB é simétrica se e só se AB = BA --- a lei deste corpo.\n\n");
        long soma_ok = 0, lei = 0, sim_prod_n = 0, comuta_n = 0, tot = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++) for(long c = -2; c <= 2; c++)
        for(long d = -2; d <= 2; d++) for(long e = -2; e <= 2; e++) for(long f = -2; f <= 2; f++){
            Sim x = sim(a,b,c), y = sim(d,e,f);
            Sim s = sim_soma(x, y);
            tot++;
            /* a soma fecha: o resultado é ainda um terno, logo simétrico */
            if(s.a == a+d && s.b == b+e && s.c == c+f) soma_ok++;
            if(sim_lei(x, y)) lei++;
            if(sim_produto_simetrico(x, y)) sim_prod_n++;
            if(sim_comutam(x, y)) comuta_n++;
        }
        printf("      fecha na SOMA em %ld/%ld\n", soma_ok, tot);
        printf("      o produto é simétrico em %ld e comutam em %ld --- e as duas condições\n"
               "      coincidem em %ld/%ld\n", sim_prod_n, comuta_n, lei, tot);
        printf("        NÃO fecha no produto: %ld dos %ld pares saem do corpo, e o que o\n"
               "        fecharia é a comutação\n", tot - sim_prod_n, tot);
        ok("o simétrico fecha na soma e não no produto, e a lei é a comutação",
           soma_ok == tot && lei == tot && sim_prod_n < tot && sim_prod_n == comuta_n);
    }

    /* ── §F3 E O DETERMINANTE DO SIMÉTRICO ligado à tríade ──────────────── */
    {
        printf("\n§F3  o det de uma simétrica, e a face que ela ocupa.\n\n");
        long neg = 0, zero = 0, pos = 0, tot = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++) for(long c = -3; c <= 3; c++){
            Sim x = sim(a,b,c);
            long D = sim_traco(x)*sim_traco(x) - 4*sim_det(x);
            /* para uma simétrica, Δ = (a−c)² + 4b² ≥ 0: NUNCA é elíptica */
            tot++;
            if(D < 0) neg++; else if(D == 0) zero++; else pos++;
            if(D != (a-c)*(a-c) + 4*b*b) return 1;   /* a identidade tem de valer */
        }
        printf("      Δ = (a−c)² + 4b² ≥ 0 sempre: %ld elípticas, %ld parabólicas, %ld"
               " hiperbólicas (de %ld)\n", neg, zero, pos, tot);
        printf("        RELAÇÃO — uma simétrica real NUNCA é elíptica: o seu Δ é uma soma\n"
               "        de quadrados, e é por isso que ela não roda\n");
        ok("uma simétrica nunca ocupa a face elíptica: Δ é soma de quadrados",
           neg == 0 && zero > 0 && pos > 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
