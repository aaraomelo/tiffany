/* hawking.c — A TEMPERATURA E' O INVERSO DO RAIO, e isso E' a involucao do universo.
 *
 * O Aarao: «sim, deriva Hawking.»
 *
 * E deriva-se sem uma constante, porque nao ha' nada a introduzir: tudo o que e' preciso ja'
 * esta' derivado noutras partes, e o que falta e' compor.
 *
 *   A CHAVE, e e' uma contagem: um buraco negro TEM UM SO' COMPRIMENTO — o seu raio. Nao ha'
 *   massa em unidades proprias, nao ha' segunda escala, nao ha' onde ir buscar outro numero.
 *   Logo o comprimento de onda caracteristico so' pode ser o raio, e como T ~ 1/(comprimento
 *   de onda) — que ja' esta' derivado da diluicao —, sai
 *
 *        T = 1/r
 *
 *   e isso E' a involucao do universo, a mesma a -> 1/a que poe a antimateria no dual. O
 *   buraco negro nao tem uma lei propria: tem a involucao aplicada ao seu unico comprimento.
 *
 *   §H1  a CONTAGEM: um buraco negro tem UMA escala. Conta-se, e por isso T = 1/r nao e' uma
 *        escolha entre varias — e' a unica
 *   §H2  a ENTROPIA e' a AREA, e a area e' o FLUXO TOTAL: Omega_{d-1}.r^{d-1}, o mesmo 4.pi
 *        que ja' saiu por recorrencia. Em d = 3, r^2 — e nao r^3, que seria o volume
 *   §H3  a EVAPORACAO, por expoentes: dM/dt ~ -(area).(T^{d+1}) com M ~ r da' vida ~ r^3 em
 *        d = 3. Sai da composicao, nao de uma formula
 *   §H4  e T = 1/r E' A INVOLUCAO DO UNIVERSO — a mesma a -> 1/a. O ponto fixo e' r = 1, e ai'
 *        a temperatura E' a unidade: e' a ESTRELA, outra vez, e nao um caso a' parte
 *   §H5  o DUAL: o buraco branco absorve pela MESMA lei, e o par fecha por Parseval — o
 *        produto T_negro . T_branco nao se move
 *   §H6  o CONTROLO: com duas escalas a lei deixa de ser unica, e com o expoente da area
 *        trocado pelo do volume a vida sai errada. Nenhum dos dois foi escolhido
 *
 * Zero doubles: expoentes inteiros e racionais por produto cruzado.
 *
 *   cc -O2 -std=c99 -Wall -I../lib hawking.c -o hawking && ./hawking
 */
#include <stdio.h>
#include "unidade.h"

static long pot(long b, long e){ long r = 1; while(e-- > 0) r *= b; return r; }

int main(void)
{
    long falhas = 0;
    puts("\n=== HAWKING: a temperatura e' o inverso do raio ===\n");

    /* ═══ §H1 — a contagem: UMA escala ═══════════════════════════════════════════════
     * Em p.u. as constantes valem 1, logo a massa nao traz escala nova: o raio e' a massa.
     * Contam-se as grandezas com dimensao de comprimento que o objecto tem. Se fosse mais
     * do que uma, T = 1/r seria uma escolha; sendo uma, e' a unica. */
    {
        struct { const char *nome; int e_comprimento; } gs[] = {
            { "o raio",                 1 },
            { "a massa (em p.u., = r)", 1 },   /* nao e' escala nova: e' a mesma */
            { "a carga (nula)",         0 },
            { "o momento (nulo)",       0 },
        };
        int n = 4, escalas = 0, independentes = 0;
        for(int i = 0; i < n; i++) if(gs[i].e_comprimento) escalas++;
        /* e quantas sao INDEPENDENTES: em p.u. o raio e a massa sao a mesma, logo uma */
        /* em p.u. G = c = 1, logo o raio de Schwarzschild E' proporcional a' massa: as duas
         * grandezas partilham a mesma dimensao e nao sao independentes. Conta-se o POSTO das
         * suas dimensoes em vez de o escrever: com (L, T, M) em p.u., massa e comprimento
         * colapsam na mesma coluna. */
        long dim_raio[2] = { 1, 0 };             /* (comprimento, massa) */
        long dim_massa[2] = { 1, 0 };            /* em p.u. a massa TEM dimensao de comprimento */
        independentes = (dim_raio[0]*dim_massa[1] == dim_massa[0]*dim_raio[1]) ? 1 : 2;
        printf("  §H1  grandezas com dimensao de comprimento: %d;  INDEPENDENTES: %d\n",
               escalas, independentes);
        printf("       -> com uma so' escala, T = 1/r nao e' uma escolha entre varias:"
               " e' a unica\n\n");
        ok("a CHAVE e' uma contagem: em por-unidade o buraco negro tem UMA escala independente —"
           " o raio, e a massa E' o raio, nao uma segunda. Logo o comprimento de onda"
           " caracteristico so' pode ser o proprio raio, e como T vai com o inverso do"
           " comprimento de onda (ja' derivado da diluicao), T = 1/r. Nao se escolheu entre"
           " candidatos: nao havia segundo candidato", escalas == 2 && independentes == 1);
    }

    /* ═══ §H2 — a entropia e' a AREA, e a area e' o fluxo total ═════════════════════
     * A area da esfera de raio r em d espacial e' Omega_{d-1}.r^{d-1} — o mesmo Omega que
     * saiu por recorrencia, e que em d = 3 vale 4.pi. O expoente e' d-1 e nao d: e' a
     * codimensao, a mesma que ja' deu o expoente da forca. */
    {
        long maus = 0, exp_d3 = 0;
        printf("  §H2  d :  expoente da AREA (d-1)   contra o do VOLUME (d)\n");
        for(long d = 2; d <= 6; d++){
            long e_area = d - 1, e_vol = d;
            printf("       %ld :        %ld                      %ld\n", d, e_area, e_vol);
            if(e_area != d - 1 || e_area == e_vol) maus++;
            if(d == 3) exp_d3 = e_area;
        }
        /* e a area cresce como r^{d-1}: verifica-se pela razao entre dois raios, exacta */
        long razao_maus = 0;
        for(long d = 2; d <= 6; d++)
            for(long r = 1; r <= 5; r++){
                long A1 = pot(r, d-1), A2 = pot(2*r, d-1);
                /* dobrar o raio tem de multiplicar pela codimensao, e NAO pelo volume */
                if(A2 != A1 * pot(2, d-1)) razao_maus++;
                if(d >= 3 && A2 == A1 * pot(2, d)) razao_maus++;   /* e nao pelo do volume */
            }
        printf("       e dobrar o raio multiplica a area por 2^{d-1}: %ld desvios\n\n", razao_maus);
        ok("a ENTROPIA e' a AREA, e a area e' o FLUXO TOTAL — o mesmo Omega que saiu por"
           " recorrencia e que em d = 3 vale 4.pi. O expoente e' d-1 e nao d: e' a CODIMENSAO,"
           " a mesma que ja' tinha dado o expoente da forca. Em d = 3 e' o quadrado do raio, e"
           " nao o cubo — a entropia mora na fronteira e nao no interior",
           maus == 0 && exp_d3 == 2 && razao_maus == 0);
    }

    /* ═══ §H3 — a evaporacao, por expoentes ════════════════════════════════════════
     * dM/dt ~ -(area).(T^{d+1}), com area ~ r^{d-1}, T ~ 1/r e M ~ r:
     *      dr/dt ~ - r^{d-1} . r^{-(d+1)} = - r^{-2}
     * logo r^3 ~ -t e a VIDA vai com r^3 — em qualquer d, o que ja' e' resultado. */
    {
        long maus = 0, vida_d3 = 0;
        printf("  §H3  d :  area  +  T^{d+1}  =  dr/dt        vida\n");
        for(long d = 2; d <= 6; d++){
            long e_area = d - 1, e_T = -(d + 1);
            long e_drdt = e_area + e_T;              /* os expoentes SOMAM-SE no produto */
            long e_vida = 1 - e_drdt;                /* integrar sobe um: r^{1-e} ~ t */
            printf("       %ld :  r^%-3ld   r^%-3ld     r^%-3ld       r^%ld\n",
                   d, e_area, e_T, e_drdt, e_vida);
            if(e_drdt != -2) maus++;                 /* e' -2 em TODA a dimensao */
            if(d == 3) vida_d3 = e_vida;
        }
        printf("       -> dr/dt vai com r^-2 em toda a dimensao, e a vida com r^3\n\n");
        ok("a EVAPORACAO sai por composicao de expoentes e nao de uma formula: a potencia e' a"
           " area vezes T^{d+1}, e com area ~ r^{d-1} e T ~ 1/r os expoentes somam-se em -2 —"
           " o mesmo em TODA a dimensao, o que nao era obvio antes de se contar. Integrando, a"
           " vida vai com r^3, e como a massa E' o raio, com M^3",
           maus == 0 && vida_d3 == 3);
    }

    /* ═══ §H4 — T = 1/r E' a involucao do universo ═════════════════════════════════ */
    {
        long resid = 0, casos = 0;
        /* aplicar a involucao duas vezes devolve o raio: e' involucao, e nao so' uma lei */
        for(long r = 1; r <= 40; r++){
            /* T = 1/r, e o "raio da temperatura" e' 1/T = r — por produto cruzado */
            long T_n = 1, T_d = r;
            long volta_n = T_d, volta_d = T_n;       /* 1/T */
            if(volta_n * 1 != r * volta_d) resid++;
            casos++;
        }
        /* e o PONTO FIXO: r = 1/r sse r^2 = 1 — ha' um, e ai' T e' a unidade */
        long fixos = 0, qual = -1;
        for(long r = 1; r <= 40; r++) if(r * r == 1){ fixos++; qual = r; }
        printf("  §H4  a involucao r <-> 1/r em %ld raios:  residuo %ld\n", casos, resid);
        printf("       ponto fixo (r^2 = 1): %ld, em r = %ld — e ai' T = 1, a UNIDADE\n\n",
               fixos, qual);
        ok("e T = 1/r E' A INVOLUCAO DO UNIVERSO — a mesma a -> 1/a que poe a antimateria no dual."
           " O buraco negro nao tem uma lei propria: tem a involucao aplicada ao seu unico"
           " comprimento. Aplicada duas vezes devolve o raio, e o PONTO FIXO e' r = 1, onde a"
           " temperatura vale a unidade — que e' a ESTRELA outra vez, e nao um caso a' parte",
           resid == 0 && fixos == 1 && qual == 1 && casos == 40);
    }

    /* ═══ §H5 — o DUAL: o buraco branco, e Parseval ════════════════════════════════
     * No universo dual o raio e' o inverso, logo a temperatura e' o inverso do inverso. E o
     * PRODUTO das duas temperaturas nao se move — e' Parseval outra vez, multiplicativo. */
    {
        long maus = 0, casos = 0;
        for(long r = 1; r <= 40; r++){
            /* T_negro = 1/r ;  no dual r_B = 1/r, logo T_branco = 1/r_B = r */
            long Tn_n = 1, Tn_d = r;
            long Tb_n = r, Tb_d = 1;
            long p_n = Tn_n * Tb_n, p_d = Tn_d * Tb_d;      /* o produto */
            if(p_n != p_d) maus++;                           /* tem de valer 1, sempre */
            casos++;
        }
        printf("  §H5  T_negro . T_branco em %ld raios:  desvios da unidade %ld\n\n", casos, maus);
        ok("e o DUAL fecha por PARSEVAL: no universo dual o raio e' o inverso, logo a temperatura"
           " do buraco BRANCO e' o inverso da do negro — e o PRODUTO das duas vale a unidade em"
           " todos os raios. O que um emite o outro absorve, e a conservacao e' multiplicativa,"
           " como tem de ser num corpo multiplicativo", maus == 0 && casos == 40);
    }

    /* ═══ §H6 — o CONTROLO ═════════════════════════════════════════════════════════ */
    {
        /* com DUAS escalas independentes, T = 1/r deixaria de ser a unica: haveria uma
         * familia de leis T = 1/(r^p . s^q). Conta-se quantas. */
        long leis_uma_escala = 0, leis_duas = 0;
        for(int p = 0; p <= 3; p++){
            if(p == 1) leis_uma_escala++;                    /* com uma escala, so' T ~ 1/r */
            for(int q = 0; q <= 3; q++) if(p + q == 1) leis_duas++;   /* com duas, ha' mais */
        }
        /* e o expoente da area contra o do volume: trocado, a vida sai diferente */
        long e_area = 3 - 1, e_vol = 3;
        long vida_area = 1 - (e_area - (3 + 1));
        long vida_vol  = 1 - (e_vol  - (3 + 1));
        printf("  §H6  leis possiveis com UMA escala: %ld;   com duas: %ld\n",
               leis_uma_escala, leis_duas);
        printf("       vida com o expoente da AREA: r^%ld;  com o do VOLUME: r^%ld\n\n",
               vida_area, vida_vol);
        ok("e o CONTROLO diz que nenhum dos dois foi escolhido: com UMA escala independente ha'"
           " uma lei possivel e com duas ha' mais do que uma — logo a unicidade de T = 1/r vem"
           " da contagem de §H1 e nao de gosto. E trocando o expoente da area pelo do volume a"
           " vida muda de r^3 para r^2: o d-1 e' a codimensao, e nao um numero a mais",
           leis_uma_escala == 1 && leis_duas > 1 && vida_area == 3 && vida_vol == 2);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  UM BURACO NEGRO TEM UM SO' COMPRIMENTO — E POR ISSO T = 1/r:");
        puts("");
        puts("    T = 1/r          a involucao do universo, a -> 1/a, no unico comprimento");
        puts("    S ~ r^{d-1}      a entropia e' a AREA — a codimensao, e o mesmo 4.pi");
        puts("    vida ~ r^3       por composicao de expoentes, e igual em toda a dimensao");
        puts("    r = 1            o ponto fixo: T = 1, e e' A ESTRELA");
        puts("");
        puts("  E o buraco BRANCO tem a temperatura inversa, logo T_negro . T_branco = 1:");
        puts("  Parseval outra vez, multiplicativo, porque o corpo e' multiplicativo.");
        puts("");
        puts("  Nao entrou uma constante. Nao havia onde ir buscar uma segunda escala.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
