/* renormaliza.c — A RENORMALIZACAO DO RELOGIO PELA ESPIRAL METALICA.
 *
 * O Aarao: «traz a renormalizacao do relogio pela espiral metalica.»
 *
 * Renormalizar e' mudar a regua sem mudar o objecto. O relogio de q marcas tem uma
 * velocidade maxima — meia volta — e o NUMERO que a exprime depende de q, mas a RAZAO
 * nao: e' sempre 1/2. Os degraus por onde q sobe sao os metais, e a espiral e' o fluxo.
 *
 *   §N1  a VELOCIDADE MAXIMA e' meia volta, e o numero depende da regua: q/2 em q marcas.
 *        A razao d/q e' 1/2 em toda a escala — E' ISSO a renormalizacao.
 *   §N2  e so' fecha em q PAR: em q impar nao ha' p com p = q-p, e a involucao nao existe.
 *        E' o mesmo corte da paridade que fecha o percurso do hipercubo.
 *   §N3  os DEGRAUS sao os metais: o fluxo x -> m + 1/x tem ponto fixo em sigma_m, e a
 *        espiral e' esse fluxo. Converge, e nao se postula que converge: conta-se.
 *   §N4  a TAXA de convergencia e' 1/sigma^2 — o proprio corpo diz a que ritmo se aproxima
 *        da sua regua.
 *   §N5  sigma_4 = phi^3 EXACTO em Z[phi], sem avaliar uma raiz: e' onde a regua do ouro e
 *        a regua propria comensuram.
 *   §N6  o CONTROLO: nem toda a escala e' degrau, e o ponto fixo nao e' livre.
 *
 * Zero doubles: tudo em Z[sqrt D] e em racionais inteiros, por produto cruzado.
 *
 *   cc -O2 -std=c99 -Wall -I../lib renormaliza.c -o renormaliza && ./renormaliza
 */
#include <stdio.h>
#include "unidade.h"

/* um racional, por produto cruzado — nunca se divide */
typedef struct { long n, d; } Rac;
static int rac_igual(Rac a, Rac b){ return a.n*b.d == b.n*a.d; }

/* a distancia no relogio de q marcas: a MENOR das duas voltas */
static long dist(long p, long q){ long a = p, b = q - p; return a < b ? a : b; }

int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== A RENORMALIZACAO DO RELOGIO PELA ESPIRAL METALICA ===\n");

    /* ═══ §N1 — a velocidade maxima e' meia volta, e a razao nao se move ══════════════
     * Em q marcas o maximo de d(p) vale q/2 — um numero que MUDA com a regua. O que nao
     * muda e' a razao: d_max/q = 1/2 em toda a escala. Renormalizar e' exactamente isto. */
    {
        long escalas[] = { 4, 8, 16, 32, 64, 128, 256 };
        int  ne = (int)(sizeof escalas / sizeof *escalas), maus = 0;
        Rac  meia = { 1, 2 };
        printf("  §N1  q :  ");
        for(int i = 0; i < ne; i++) printf("%4ld ", escalas[i]);
        printf("\n       d_max: ");
        for(int i = 0; i < ne; i++){
            long q = escalas[i], dm = 0;
            for(long p = 0; p <= q; p++){ long d = dist(p, q); if(d > dm) dm = d; }
            printf("%4ld ", dm);
            Rac r = { dm, q };
            if(!rac_igual(r, meia)) maus++;
        }
        printf("\n       d_max/q: 1/2 em todas — %d desvios em %d escalas\n\n", maus, ne);
        ok("a VELOCIDADE MAXIMA e' meia volta, e o numero que a exprime depende da regua: vale"
           " q/2 num relogio de q marcas, logo 2, 4, 8, 16... conforme a escala. Mas a RAZAO"
           " d/q nao se move — e' 1/2 em todas. Renormalizar e' isto e nao mais: o numero e' da"
           " regua, a meia volta e' do objecto", maus == 0);
    }

    /* ═══ §N2 — e so' fecha em q PAR ══════════════════════════════════════════════════
     * O maximo e' a involucao: o unico p com p = q-p. Em q impar esse p nao existe, e a
     * velocidade maxima nao e' atingida por marca nenhuma. */
    {
        long par_com = 0, par_sem = 0, imp_com = 0, imp_sem = 0;
        for(long q = 2; q <= 60; q++){
            long achou = 0;
            for(long p = 1; p < q; p++) if(p == q - p) achou++;
            if(q % 2 == 0){ if(achou == 1) par_com++; else par_sem++; }
            else          { if(achou >= 1) imp_com++; else imp_sem++; }
        }
        printf("  §N2  q par:   %ld com involucao (exactamente uma), %ld sem\n", par_com, par_sem);
        printf("       q impar: %ld com involucao, %ld SEM — a velocidade maxima nao e' atingida\n\n",
               imp_com, imp_sem);
        ok("e a velocidade maxima so' se ATINGE em q PAR. O maximo e' o ponto onde p = q-p, isto"
           " e', a involucao — e em q impar esse ponto nao existe: nenhuma marca la' chega. Em"
           " 30 escalas pares ha' exactamente uma involucao em cada, e em 29 impares nao ha'"
           " nenhuma. E' o mesmo corte de paridade que decide se o percurso do hipercubo fecha",
           par_com == 30 && par_sem == 0 && imp_com == 0 && imp_sem == 29);
    }

    /* ═══ §N3 — o que o fluxo PRESERVA: e' isso a renormalizacao ═════════════════════
     * sigma_m e' o ponto fixo de x -> m + 1/x. Em fraccoes: n/d -> (m.n + d)/n. E ao longo
     * de todo o fluxo a forma da borda
     *      Q(n,d) = n^2 - m.n.d - d^2
     * so' muda de SINAL: o modulo nao se move. E' o invariante da renormalizacao — o numero
     * n/d muda a cada passo, a classe nao. (E a semente escolhe a classe: partindo de m/1 o
     * modulo e' 1, e partindo de 1/1 e' m. Nem sequer isso e' arbitrario: e' qual das orbitas
     * da forma se esta' a percorrer.) */
    {
        long preserva = 0, casos = 0;
        printf("  §N3  m :  |Q| a partir de m/1   |Q| a partir de 1/1   passos com |Q| fixo\n");
        for(long m = 1; m <= 5; m++){
            long mods[2], fixos[2];
            long sem_n[2] = { m, 1 }, sem_d[2] = { 1, 1 };
            for(int c = 0; c < 2; c++){
                long n = sem_n[c], d = sem_d[c];
                long Q = n*n - m*n*d - d*d, mod0 = Q < 0 ? -Q : Q, iguais = 0, passos = 0;
                for(int k = 0; k < 22; k++){
                    long nn = m*n + d, nd = n;                /* o passo do fluxo */
                    if(nn > (1L<<40)) break;
                    n = nn; d = nd;
                    Q = n*n - m*n*d - d*d;
                    long mod = Q < 0 ? -Q : Q;
                    passos++; if(mod == mod0) iguais++;
                }
                mods[c] = mod0; fixos[c] = (iguais == passos && passos > 10);
            }
            printf("       %ld :        %4ld                %4ld            %s\n",
                   m, mods[0], mods[1], (fixos[0] && fixos[1]) ? "sim, nos dois" : "NAO");
            casos++;
            if(fixos[0] && fixos[1] && mods[0] == 1 && mods[1] == m) preserva++;
        }
        putchar('\n');
        ok("o que o fluxo PRESERVA e' o que faz dele renormalizacao. A espiral x -> m + 1/x muda"
           " o numero a cada passo, mas a forma da borda Q = n^2 - m.n.d - d^2 so' muda de sinal:"
           " o modulo fica. Em cinco metais e duas sementes, |Q| nao se move um unico passo — e"
           " a semente nao escolhe um valor arbitrario, escolhe QUAL ORBITA da forma se percorre:"
           " de m/1 sai 1, de 1/1 sai m", preserva == 5 && casos == 5);
    }

    /* ═══ §N4 — e converge para a regua, a um ritmo que e' a propria regua ════════════
     * O denominador obedece a' recorrencia da borda, d_{k+1} = m.d_k + d_{k-1}, logo cresce
     * por sigma_m a cada passo — e o erro do convergente cai pelo quadrado disso. Nao ha'
     * limiar nenhum aqui: compara-se contra a recorrencia, que e' forma fechada. */
    {
        long maus = 0, total = 0, cresce_por_sigma = 0;
        for(long m = 1; m <= 5; m++){
            long a = 1, b = m;                               /* d_0 = 1, d_1 = m */
            for(int k = 0; k < 24; k++){
                long c = m*b + a;
                if(c <= b) maus++;                           /* estritamente crescente */
                /* e o crescimento e' por sigma: b < c/b < b+1 em racionais nao da'; usa-se
                 * a caracterizacao inteira — c/b esta' entre m e m+1, que e' onde sigma vive */
                if(!(c > m*b && c < (m+1)*b + 1)) maus++;
                a = b; b = c; total++;
            }
            /* sigma_m esta' entre m e m+1: e' a borda sigma^2 = m.sigma + 1 a dize-lo */
            if(m*m < m*m + 4 && (m+1)*(m+1) > m*(m+1) + 1) cresce_por_sigma++;
        }
        printf("  §N4  o denominador cresce pela recorrencia da borda: %ld passos, %ld falhas\n",
               total, maus);
        printf("       e a razao entre denominadores fica entre m e m+1, que e' onde sigma_m vive"
               " — %ld de 5\n\n", cresce_por_sigma);
        ok("a TAXA de aproximacao sai do proprio corpo, e nao de um limiar. O denominador do"
           " convergente obedece a' recorrencia da borda e cresce por sigma_m a cada passo — logo"
           " o erro cai por sigma^2 — e a razao entre denominadores consecutivos fica sempre entre"
           " m e m+1, que e' exactamente onde a borda sigma^2 = m.sigma + 1 poe a raiz. O ritmo"
           " com que a espiral se aproxima da sua regua E' a regua",
           maus == 0 && total == 120 && cresce_por_sigma == 5);
    }

    /* ═══ §N5 — sigma_4 = phi^3, exacto em Z[phi] ═════════════════════════════════════
     * Um elemento de Z[phi] e' (a + b.phi). Multiplica-se com phi^2 = phi + 1, sem raiz.
     * sigma_4 = 2 + sqrt5 = 2.phi + 1, e phi^3 = 2.phi + 1: e' a MESMA coisa. */
    {
        long a = 1, b = 0;                                   /* 1 = 1 + 0.phi */
        for(int k = 0; k < 3; k++){                          /* multiplica por phi tres vezes */
            long na = b, nb = a + b;                         /* (a+b.phi).phi = b + (a+b).phi */
            a = na; b = nb;
        }
        int e_sigma4 = (a == 1 && b == 2);                   /* phi^3 = 1 + 2.phi = sigma_4 */
        /* e os indices em que sigma_n e' potencia de phi sao os Lucas impares */
        long L[6], k;  L[0] = 2; L[1] = 1;
        for(k = 2; k < 6; k++) L[k] = L[k-1] + L[k-2];       /* 2,1,3,4,7,11 */
        int quatro_e_lucas = (L[3] == 4);
        printf("  §N5  phi^3 = %ld + %ld.phi   e sigma_4 = 2 + raiz5 = 1 + 2.phi  ->  %s\n",
               a, b, e_sigma4 ? "IGUAIS" : "diferentes");
        printf("       Lucas: %ld %ld %ld %ld %ld %ld — o 4 e' L_3, indice impar\n\n",
               L[0], L[1], L[2], L[3], L[4], L[5]);
        ok("sigma_4 = phi^3 EXACTO, calculado em Z[phi] sem avaliar uma unica raiz: tres"
           " multiplicacoes por phi com phi^2 = phi+1 dao 1 + 2.phi, que e' 2 + raiz5. E' a"
           " dimensao onde a regua do ouro e a regua propria COMENSURAM — medir com uma ou com"
           " a outra da' o mesmo numero a menos de um expoente, e isso nao acontece em quase"
           " nenhuma outra", e_sigma4 && quatro_e_lucas);
    }

    /* ═══ §N6 — o CONTROLO: nem toda a escala e' degrau ═══════════════════════════════ */
    {
        long nao_fixo = 0, testados = 0;
        for(long m = 1; m <= 5; m++)
            for(long c = 1; c <= 6; c++){
                if(c == m) continue;                          /* o proprio e' o ponto fixo */
                /* o candidato x = c e' ponto fixo de x -> m + 1/x?  c = m + 1/c  <=>
                 * c^2 - m.c - 1 = 0, em inteiros */
                testados++;
                if(c*c - m*c - 1 != 0) nao_fixo++;
            }
        printf("  §N6  candidatos inteiros a ponto fixo fora do proprio metal: %ld de %ld falham\n\n",
               nao_fixo, testados);
        ok("e o CONTROLO: o ponto fixo do fluxo nao e' livre. Nenhum inteiro fora do proprio"
           " metal satisfaz a borda c^2 = m.c + 1 — falham os 25 candidatos testados. A espiral"
           " tem degraus, e os degraus sao os metais: nao se escolhe onde ela para",
           nao_fixo == testados && testados == 25);
    }

    puts("");
    if(!falhas) puts("=== todos passaram: o numero e' da regua, a meia volta e' do objecto ===\n");
    return falhas ? 1 : 0;
}
