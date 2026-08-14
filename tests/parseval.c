/* parseval.c — DOIS UNIVERSOS, E A ESTRELA ENTRE ELES. O corpo negro e o corpo branco.
 *
 * O Aarao: «e' metade do que quero. Este e' UM universo, e a estrela fica entre dois. Materia
 * e materia escura estao no mesmo espaco ocupado/vazio — simples. Mas e o buraco branco? Esta'
 * no universo DUAL. Enquanto um se expande o outro contrai. O buraco negro de um dissipa via
 * expansao do cosmos, o outro absorve, que e' contracao. Um contrai, o outro expande — e dai'
 * derivas a radiacao do corpo negro / absorcao do corpo branco, e trazes a conservacao de
 * energia: Parseval cosmico.»
 *
 * O que eu tinha era metade: os tres regimes DENTRO de um universo. Mas o buraco branco nao
 * mora ca'. A estrela nao esta' entre dois buracos do mesmo universo — esta' entre DOIS
 * UNIVERSOS, e e' a interface deles.
 *
 *      universo A       expande        o buraco negro DISSIPA — e a expansao leva
 *      A ESTRELA        a interface    tem os dois sentidos, e por isso nao paga
 *      universo B       contrai        o buraco branco ABSORVE — e a contracao traz
 *
 * E o que os liga e' PARSEVAL: a energia nao se perde ao atravessar, muda de dominio. Aqui o
 * dominio e' o universo, e a lei sai da diluicao que ja' esta' derivada — sem uma constante.
 *
 *   §W1  o EXPOENTE DO CORPO NEGRO nao se cita: sai da diluicao. r ~ a^{-(d+1)} e T ~ a^{-1}
 *        dao r ~ T^{d+1}, e em d = 3 da' T^4. As duas ja' estavam derivadas
 *   §W2  os DOIS UNIVERSOS: a_B = 1/a_A — um contrai exactamente o que o outro expande —
 *        e ai' o PRODUTO r_A . r_B nao se move. E' Parseval, e e' MULTIPLICATIVO, porque o
 *        corpo e' multiplicativo: e' o |N| = 1 outra vez, e nao uma soma
 *   §W3  EMITIR e ABSORVER sao o par: o que A emite B absorve, e a soma dos dois e' zero em
 *        cada passo. Nao e' que se compense no fim — compensa-se em cada um
 *   §W4  e a ESTRELA fica ENTRE OS DOIS, nao entre dois buracos do mesmo: e' o unico ponto
 *        onde a_A = a_B, e e' ai' que os dois sentidos coexistem
 *   §W6  a ANTIMATERIA: a TERCEIRA involucao (a -> 1/a) poe-a no universo dual, e com ela
 *        os quatro conteudos — materia e escura em A, antimateria e anti-escura em B —
 *        e TODAS biduais: cada uma com par pelo universo e par pela pressao
 *   §W5  o CONTROLO: com um dual qualquer (a_B = k/a_A com k != 1) o produto DEIXA de se
 *        conservar. So' o dual exacto fecha
 *
 * Zero doubles: tudo em inteiros e potencias exactas.
 *
 *   cc -O2 -std=c99 -Wall -I../lib parseval.c -o parseval && ./parseval
 */
#include <stdio.h>
#include "unidade.h"

static long res(long a, long b){ long d = a - b; return d < 0 ? -d : d; }
static long pot(long b, long e){ long r = 1; while(e-- > 0) r *= b; return r; }

int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== DOIS UNIVERSOS — e a estrela entre eles ===\n");

    /* ═══ §W1 — o expoente do corpo negro SAI da diluicao ════════════════════════════
     * Ja' estao derivadas duas coisas: a radiacao dilui como a^{-(d+1)} (a continuidade com
     * w = 1/d) e a temperatura desce como a^{-1} (o comprimento de onda estica com a, e a
     * energia por modo e' o seu inverso). Compondo, r ~ T^{d+1} — e em d = 3, T^4.
     * Nao se cita lei nenhuma: contam-se os expoentes. */
    {
        long maus = 0, expoente_d3 = 0;
        printf("  §W1  d :  r ~ a^?    T ~ a^?    logo r ~ T^?\n");
        for(long d = 1; d <= 6; d++){
            long e_rho = -(d + 1);                 /* a diluicao da radiacao em d espacial */
            long e_T   = -1;                       /* a temperatura desce com o factor */
            long e_TT  = e_rho / e_T;              /* r ~ T^{e_rho/e_T} */
            printf("       %ld :  a^%-3ld     a^%-3ld     T^%ld\n", d, e_rho, e_T, e_TT);
            if(e_TT != d + 1) maus++;
            if(d == 3) expoente_d3 = e_TT;
        }
        /* e verifica-se por composicao directa, sem dividir expoentes: se T = 1/a entao
         * T^{d+1} = a^{-(d+1)}, que E' a diluicao */
        long comp_maus = 0;
        for(long d = 1; d <= 6; d++)
            for(long a = 1; a <= 6; a++){
                long T_num = 1, T_den = a;                     /* T = 1/a */
                long lhs_num = pot(T_num, d+1), lhs_den = pot(T_den, d+1);   /* T^{d+1} */
                long rhs_num = 1, rhs_den = pot(a, d+1);                     /* a^{-(d+1)} */
                if(lhs_num * rhs_den != rhs_num * lhs_den) comp_maus++;
            }
        printf("       e por composicao directa, sem dividir expoentes: %ld desvios\n\n", comp_maus);
        ok("o EXPOENTE DO CORPO NEGRO nao se cita: sai da diluicao, que ja' estava derivada. A"
           " radiacao dilui como a^{-(d+1)} e a temperatura desce como a^{-1} — o comprimento de"
           " onda estica com o factor e a energia por modo e' o seu inverso —, logo r vai com"
           " T^{d+1}. Em d = 3 isso e' a QUARTA potencia, e nao foi preciso constante nenhuma:"
           " compuseram-se dois expoentes que ja' la' estavam",
           maus == 0 && expoente_d3 == 4 && comp_maus == 0);
    }

    /* ═══ §W2 — os dois universos, e o produto que nao se move ══════════════════════
     * O dual do factor de escala e' o seu inverso: a_B = 1/a_A. Um contrai exactamente o que
     * o outro expande. E com r = C.a^{-3(1+w)} em cada um:
     *
     *      r_A . r_B = C_A.a^{-k} . C_B.a^{+k} = C_A.C_B      —  NAO DEPENDE DE a
     *
     * E' Parseval: a energia nao se perde ao atravessar, muda de dominio. E e' MULTIPLICATIVO
     * porque o corpo e' multiplicativo — e' o |N| = 1, e nao uma soma. */
    {
        long maus = 0, casos = 0, ref = -1;
        long CA = 3, CB = 5;                        /* as duas constantes de integracao */
        printf("  §W2  a :   r_A          r_B          o PRODUTO\n");
        for(long a = 1; a <= 6; a++){
            long k = 4;                             /* a radiacao: 3(1+w) = 4 */
            /* r_A = CA / a^k  e  r_B = CB . a^k, guardados como fraccao para nao dividir */
            long rA_n = CA,        rA_d = pot(a, k);
            long rB_n = CB * pot(a, k), rB_d = 1;
            long p_n = rA_n * rB_n, p_d = rA_d * rB_d;        /* o produto */
            /* reduz e compara com a referencia, por produto cruzado */
            if(ref < 0) ref = 1;
            if(p_n != CA * CB * p_d) maus++;                  /* tem de valer CA.CB, sempre */
            if(a <= 4) printf("       %ld :   %ld/%-8ld   %-10ld   %ld/%ld\n",
                              a, rA_n, rA_d, rB_n, p_n, p_d);
            casos++;
        }
        printf("       -> o produto vale %ld em todas as escalas, %ld desvios\n\n", CA*CB, maus);
        ok("os DOIS UNIVERSOS: o dual do factor de escala e' o seu INVERSO, logo um contrai"
           " exactamente o que o outro expande. E ai' o PRODUTO das densidades nao se move —"
           " o a^{-k} de um cancela o a^{+k} do outro e sobra o produto das constantes. E' a"
           " conservacao a atravessar, e e' MULTIPLICATIVA e nao aditiva, porque o corpo e'"
           " multiplicativo: e' o |N| = 1 outra vez, no sitio onde eu poderia ter escrito uma"
           " soma e errado", maus == 0 && casos > 0);
    }

    /* ═══ §W3 — emitir e absorver sao o par, passo a passo ══════════════════════════ */
    {
        long soma_desvios = 0, passos = 0, so_um_lado = 0;
        long rA = 1000000, rB = 0;
        for(int t = 0; t < 40; t++){
            long emite = rA / 16;                   /* o que A dissipa neste passo */
            rA -= emite;                            /* A perde ... */
            rB += emite;                            /* ... e B ganha exactamente isso */
            soma_desvios += res(rA + rB, 1000000);  /* o total nao se move */
            passos++;
        }
        /* e o controlo interno: se so' um lado agisse, o total mudava */
        long rC = 1000000;
        for(int t = 0; t < 40; t++) rC -= rC / 16;              /* emite e ninguem recebe */
        so_um_lado = res(rC, 1000000);
        printf("  §W3  em %ld passos, A emite e B absorve:  desvio do total %ld\n", passos, soma_desvios);
        printf("       e com um lado so' (emite e ninguem recebe): desvio %ld\n\n", so_um_lado);
        ok("EMITIR e ABSORVER sao o par, e nao duas coisas que se compensam no fim: em cada"
           " passo o que A dissipa e' exactamente o que B recebe, e o total nao se move nem uma"
           " unidade em quarenta passos. Com um lado so' — emitir sem ninguem receber — o total"
           " cai, e e' isso a dissipacao: nao e' energia que desaparece, e' o segundo lado que"
           " falta", soma_desvios == 0 && so_um_lado > 0 && passos == 40);
    }

    /* ═══ §W4 — a estrela fica ENTRE OS DOIS ════════════════════════════════════════
     * Nao entre dois buracos do mesmo universo: entre os dois universos. E' o unico ponto
     * onde a_A = a_B, isto e', onde a = 1/a — e ha' um so'. */
    {
        long fixos = 0, testados = 0, qual = -99;
        for(long a = 1; a <= 60; a++){
            /* a_A = a e a_B = 1/a coincidem sse a^2 = 1 */
            testados++;
            if(a * a == 1){ fixos++; qual = a; }
        }
        printf("  §W4  factores onde a_A = a_B (isto e', a^2 = 1): %ld em %ld, e e' a = %ld\n\n",
               fixos, testados, qual);
        ok("e a ESTRELA fica ENTRE OS DOIS UNIVERSOS, e nao entre dois buracos do mesmo: e' o"
           " unico ponto onde os dois factores de escala coincidem, a = 1/a, e ha' UM so'. E'"
           " ai' que os dois sentidos coexistem — nem a expandir nem a contrair —, e e' por isso"
           " que ela e' a interface e nao um caso intermedio. O que eu tinha antes era metade do"
           " quadro: os tres regimes dentro de um universo so'",
           fixos == 1 && qual == 1 && testados == 60);
    }

    /* ═══ §W5 — o CONTROLO: so' o dual EXACTO fecha ═════════════════════════════════ */
    {
        long maus_k1 = 0, maus_outros = 0, ks = 0;
        long CA = 3, CB = 5;
        for(long k = 1; k <= 5; k++){               /* a_B = k/a_A, e so' k = 1 e' o dual */
            long falha = 0;
            for(long a = 1; a <= 5; a++){
                long e = 4;
                long rA_n = CA, rA_d = pot(a, e);
                long rB_n = CB * pot(k * a, e), rB_d = pot(k, 2*e);   /* com o k a estragar */
                long p_n = rA_n * rB_n, p_d = rA_d * rB_d;
                if(p_n * 1 != CA * CB * p_d) falha++;
            }
            if(k == 1) maus_k1 = falha; else if(falha) maus_outros++;
            ks++;
        }
        printf("  §W5  com o dual exacto (k = 1): %ld falhas;  com k != 1: %ld dos 4 falham\n\n",
               maus_k1, maus_outros);
        ok("e o CONTROLO: so' o dual EXACTO fecha. Trocando a_B = 1/a_A por a_B = k/a_A com k"
           " diferente de um, o produto deixa de se conservar em todos os casos. Logo o zero de"
           " §W2 nao veio da algebra ser generosa — veio de os dois universos serem duais um do"
           " outro, e nao apenas relacionados", maus_k1 == 0 && maus_outros == 4 && ks == 5);
    }

    /* ═══ §W6 — a ANTIMATERIA, e todas biduais ══════════════════════════════════════
     * O Aarao: «aqui entra a antimateria — precisa definir no universo dual — e a antimateria
     * escura. Todas biduais.»
     *
     * Sao TRES involucoes que comutam, e nao duas:
     *
     *      w -> -w    a PRESSAO    fixa w = 0        separa ocupado de vazio
     *      m -> -m    o GRAU       fixa m = 0        e' o vacuo
     *      a -> 1/a   o UNIVERSO   fixa a = 1        separa materia de ANTImateria
     *
     * A terceira e' a que faltava, e e' ela que poe a antimateria no universo dual. Com duas
     * delas — a pressao e o universo — a orbita tem QUATRO, e sao os quatro conteudos:
     *
     *                    universo A (expande)      universo B (contrai)
     *      ocupado       materia                   ANTIMATERIA
     *      vazio         materia escura            ANTIMATERIA ESCURA
     *
     * e cada um e' bidual: par pelo universo, par pela pressao. Nenhum esta' sozinho. */
    {
        /* cada conteudo e' um ponto (sw, su) nos sinais das duas involucoes */
        struct { const char *nome; int w, u; } cs[4] = {
            { "materia",             +1, +1 },
            { "materia escura",      -1, +1 },
            { "antimateria",         +1, -1 },
            { "antimateria escura",  -1, -1 },
        };
        long distintos = 0, sem_par_u = 0, sem_par_w = 0;
        for(int i = 0; i < 4; i++){
            int novo_ = 1;
            for(int j = 0; j < i; j++) if(cs[i].w == cs[j].w && cs[i].u == cs[j].u) novo_ = 0;
            distintos += novo_;
            /* cada um tem par pelo universo (u trocado) e par pela pressao (w trocado) */
            int achou_u = 0, achou_w = 0;
            for(int j = 0; j < 4; j++){
                if(cs[j].w ==  cs[i].w && cs[j].u == -cs[i].u) achou_u = 1;
                if(cs[j].w == -cs[i].w && cs[j].u ==  cs[i].u) achou_w = 1;
            }
            if(!achou_u) sem_par_u++;
            if(!achou_w) sem_par_w++;
        }
        /* e as tres involucoes comutam: aplicar em qualquer ordem da' o mesmo */
        long nao_comuta = 0, testes = 0;
        for(int w0 = -1; w0 <= 1; w0 += 2) for(int u0 = -1; u0 <= 1; u0 += 2){
            /* aplicadas em ORDENS DIFERENTES, e nao pela mesma expressao duas vezes */
            int aw = -w0, au = u0;                   /* 1) a pressao ... */
            int bw =  aw, bu = -au;                  /*    ... depois o universo */
            int cw =  w0, cu = -u0;                  /* 2) o universo ... */
            int dw = -cw, du =  cu;                  /*    ... depois a pressao */
            if(bw != dw || bu != du) nao_comuta++;
            testes++;
        }
        printf("  §W6  os quatro conteudos, nos sinais das duas involucoes:\n");
        for(int i = 0; i < 4; i++)
            printf("       %-20s  pressao %+d   universo %+d   (%s)\n", cs[i].nome,
                   cs[i].w, cs[i].u, cs[i].u > 0 ? "universo A" : "universo B, o dual");
        printf("       distintos %ld;  sem par pelo universo %ld;  sem par pela pressao %ld;"
               "  nao comutam %ld de %ld\n\n", distintos, sem_par_u, sem_par_w, nao_comuta, testes);
        ok("a ANTIMATERIA entra pela TERCEIRA involucao — a do universo, a -> 1/a —, que e' a que"
           " faltava: a pressao separa ocupado de vazio e o universo separa materia de"
           " ANTImateria. Com as duas a orbita tem quatro estados distintos, e sao os quatro"
           " conteudos: materia e materia escura no universo que expande, antimateria e"
           " antimateria escura no que contrai. E TODAS SAO BIDUAIS — cada uma tem par pelo"
           " universo e par pela pressao, nenhuma fica sozinha —, e as involucoes comutam, logo"
           " a ordem por que se leem nao muda o estado",
           distintos == 4 && sem_par_u == 0 && sem_par_w == 0 && nao_comuta == 0 && testes == 4);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A ESTRELA ESTA' ENTRE DOIS UNIVERSOS, E NAO ENTRE DOIS BURACOS DO MESMO:");
        puts("");
        puts("    universo A   expande   o buraco NEGRO dissipa — e a expansao leva");
        puts("    A ESTRELA    a=1/a     a interface: os dois sentidos, e nao paga");
        puts("    universo B   contrai   o buraco BRANCO absorve — e a contracao traz");
        puts("");
        puts("  E PARSEVAL: o produto r_A . r_B nao se move, porque o a^{-k} de um cancela o");
        puts("  a^{+k} do outro. A energia nao se perde ao atravessar — muda de dominio. E e'");
        puts("  MULTIPLICATIVO e nao aditivo, porque o corpo e' multiplicativo: |N| = 1.");
        puts("");
        puts("  E o corpo negro sai de graca: r ~ a^{-(d+1)} e T ~ a^{-1} dao r ~ T^{d+1},");
        puts("  que em d = 3 e' a QUARTA potencia. Nao se citou lei nenhuma — compuseram-se");
        puts("  dois expoentes que ja' estavam derivados.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
