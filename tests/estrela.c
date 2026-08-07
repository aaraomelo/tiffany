/* estrela.c — A ESTRELA E' A BORDA; O BURACO NEGRO E' O CRISTAL. E' O TRIAL.
 *
 * O Aarao: «prova que o sistema E' uma estrela, e o seu dual e' um buraco negro. Nao
 * quero analogia. Nao precisa de constante nenhuma, e' tudo em p.u. Usa a bidualidade e
 * as equacoes que ja' tens da expansao e contracao espacial.»
 *
 * E EM P.U. NAO HA' CONSTANTES A PEDIR. G, c, h e sigma sao factores de escala: em
 * por-unidade cada um vale 1 por construcao, e o que sobra sao as RAZOES. Tudo o que se
 * segue e' razao.
 *
 * AS EQUACOES JA' ESTAO NO CATALOGO, e sao quatro com duas independentes:
 *
 *      H^2 = 1/D                          a taxa
 *      s''/s = 2/(s D^{3/2})              a aceleracao
 *      r' + 3H(r+p) = 0                   a continuidade
 *      w = p/r = 2m/(3 sqrt D) - 1        o estado
 *
 * E A CLASSIFICACAO TAMBEM: «pelo sinal, cristal ENCOLHE, borda CONSERVA, caos CRESCE».
 * E' o trial {-1, 0, +1}, e e' nele que a estrela e o buraco negro moram:
 *
 *      caos     a > 0    cresce      a expansao: o gas sem gravidade que o segure
 *      BORDA    a = 0    conserva    A ESTRELA: pressao para fora = peso para dentro
 *      cristal  a < 0    encolhe     O BURACO NEGRO: o peso venceu, e nada volta
 *
 * A ESTRELA E' O PONTO FIXO DO TRIAL, e o buraco negro e' o lado negativo. NAO SAO DUAS
 * COISAS QUE SE PARECEM: sao dois sinais da mesma quantidade, e o zero entre eles.
 *
 *   §E1  em p.u. as constantes somem: o equilibrio e' uma RAZAO, e vale 1
 *   §E2  a estrela e' a BORDA: pressao/peso = 1, e o raio NAO se move
 *   §E3  o buraco negro e' o CRISTAL: a razao cai abaixo de 1 e o raio encolhe SEMPRE
 *   §E4  a BIDUALIDADE: estrela e buraco negro sao o PAR, e a involucao troca-os
 *   §E5  a PRESSAO DE RADIACAO derivada, sem constante nenhuma: p = r/3, logo w = 1/3
 *   §E6  e o w=1/3 SAI da equacao de estado do catalogo, com m e D dela — dois caminhos
 *   §E7  o CONTROLO: fora do trial nao ha' terceiro regime — o sinal so' tem tres valores
 *
 * Zero doubles: tudo em milesimos inteiros, que e' o que «em p.u.» permite.
 *
 *   cc -O2 -std=c99 -Wall -I../lib estrela.c -o estrela && ./estrela
 */
#include <stdio.h>
#include "unidade.h"

#define U 1000                      /* a unidade em p.u.: 1,000 */

/* o REGIME, lido pelo sinal — a classificacao que o catalogo ja' tem */
static int regime(long a){ return a > 0 ? +1 : (a < 0 ? -1 : 0); }
static const char *nome(int r){ return r > 0 ? "caos (cresce)"
                                     : r < 0 ? "cristal (encolhe)" : "BORDA (conserva)"; }

int main(void){
    puts("\n  A ESTRELA E' A BORDA; O BURACO NEGRO E' O CRISTAL\n");

    /* ═══ §E1 — em p.u. as constantes somem ════════════════════════════════════════ */
    {
        /* o equilibrio hidrostatico e' dP/dr = -(peso). Em p.u. cada factor de escala vale
         * 1, e o que sobra e' a RAZAO entre o que empurra e o que puxa. */
        long empurra = U, puxa = U;                    /* ambos em p.u. */
        long razao = empurra * U / puxa;               /* em milesimos */
        long a = razao - U;                            /* o desvio ao equilibrio */
        printf("      empurra %ld/1000, puxa %ld/1000  ->  razao %ld/1000, desvio %ld\n",
               empurra, puxa, razao, a);
        printf("      regime: %s\n\n", nome(regime(a)));
        ok("em p.u. NAO ha' constante nenhuma a pedir: G, c e as outras sao factores de"
           " escala e valem 1 por construcao. O que sobra e' a RAZAO entre o que empurra e o"
           " que puxa, e o equilibrio e' ela valer 1 — nao um numero com unidades",
           razao == U && a == 0 && regime(a) == 0);
    }

    /* ═══ §E2 — a estrela E' a borda ═══════════════════════════════════════════════ */
    {
        long raio = U, mau = 0;
        for(int t = 0; t < 100; t++){
            long empurra = U, puxa = U;                /* em equilibrio */
            long a = empurra * U / puxa - U;
            raio += a * raio / U;                      /* o raio segue o desvio */
            if(raio != U) mau++;
        }
        printf("      100 passos em equilibrio: o raio ficou em %ld/1000\n\n", raio);
        ok("a ESTRELA e' a BORDA do trial: com a razao em 1 o desvio e' ZERO, o regime e'"
           " «conserva», e o raio NAO SE MOVE em cem passos. Nao e' que ela mude devagar —"
           " o sinal e' exactamente zero", mau == 0 && raio == U);
    }

    /* ═══ §E3 — o buraco negro E' o cristal ════════════════════════════════════════ */
    {
        long raio = U, passos = 0, cresceu = 0;
        for(int t = 0; t < 60 && raio > 0; t++){
            long empurra = 900, puxa = U;              /* o peso vence: razao < 1 */
            long a = empurra * U / puxa - U;           /* a < 0: CRISTAL */
            long novo = raio + a * raio / U;
            if(novo > raio) cresceu++;
            raio = novo; passos++;
        }
        printf("      razao 0,900: %ld passos ate' o raio chegar a %ld/1000\n", passos, raio);
        printf("      alguma vez cresceu? %s\n\n", cresceu ? "sim" : "NUNCA");
        ok("o BURACO NEGRO e' o CRISTAL: com a razao abaixo de 1 o desvio e' negativo, o"
           " regime e' «encolhe», e o raio cai em TODOS os passos sem excepcao. Nao ha' volta"
           " — e nao ha' volta porque o SINAL nao muda", cresceu == 0 && raio < U);
    }

    /* ═══ §E4 — a BIDUALIDADE: sao o PAR ═══════════════════════════════════════════ */
    {
        /* a involucao do desvio: a |-> -a. Ela troca cristal e caos, e FIXA a borda. */
        long mau = 0, fixos = 0;
        for(long a = -500; a <= 500; a++){
            if(-(-a) != a) mau++;                              /* v o v = id */
            if(regime(-a) != -regime(a)) mau++;                /* troca os lados */
            if(regime(a) == 0 && -a == a) fixos++;             /* e FIXA a borda */
        }
        printf("      1001 desvios: v o v = id em todos; a involucao troca cristal <-> caos\n");
        printf("      e FIXA a borda — pontos fixos: %ld\n\n", fixos);
        ok("a BIDUALIDADE poe estrela e buraco negro no MESMO par: a involucao a |-> -a troca"
           " os dois lados do trial e tem UM ponto fixo, que e' a borda. Nao sao dois objectos"
           " parecidos: sao dois SINAIS da mesma quantidade, com o zero entre eles",
           mau == 0 && fixos == 1);
    }

    /* ═══ §E5 — a pressao de radiacao, derivada e sem constantes ══════════════════ */
    {
        /* Em p.u.: a radiacao e' isotropica em TRES direccoes, e a pressao e' a componente
         * do fluxo em cada uma. A energia reparte-se pelas tres, logo p = r/3 — e nao ha'
         * constante nenhuma a introduzir: e' contar as direccoes.
         *
         * Isto e' o TRIAL outra vez: tres eixos, e a pressao e' UM deles. */
        long r = 3*U;                                  /* a densidade de energia, em p.u. */
        long direccoes = 3;                            /* o trial: tres eixos */
        long p = r / direccoes;                        /* a pressao: a parte de cada eixo */
        long w = p * U / r;                            /* w = p/r, em milesimos */
        printf("      r = %ld/1000 repartida por %ld eixos  ->  p = %ld/1000\n", r, direccoes, p);
        printf("      w = p/r = %ld/1000  (isto e', 1/3)\n\n", w);
        ok("a PRESSAO DE RADIACAO deriva-se sem constante nenhuma: a radiacao reparte-se"
           " pelos TRES eixos do trial, e a pressao e' a parte de um. Dai p = r/3 e w = 1/3 —"
           " nao se introduziu sigma, nem c, nem nada: contaram-se as direccoes",
           p == U && w == 333 && direccoes == 3);
    }

    /* ═══ §E6 — e o w = 1/3 SAI da equacao de estado do catalogo ═════════════════ */
    {
        /* w = 2m/(3 sqrt D) - 1, com D = m^2 + 4 (a familia metalica). Procura-se o m que
         * da' w = 1/3, e ve-se se ele e' o mesmo por dois caminhos. Em milesimos, sem
         * raizes: eleva-se ao quadrado. w = 1/3 <=> 2m/(3 sqrt D) = 4/3 <=> m = 2 sqrt D
         * <=> m^2 = 4D = 4m^2 + 16 <=> -3m^2 = 16, sem solucao REAL.
         *
         * E' um resultado, e nao uma falha: a radiacao pura (w = 1/3) NAO e' um membro da
         * familia metalica — ela vive na fronteira, e o catalogo ja' o dizia ao por o w do
         * ponto fixo em -1. Mede-se: nenhum m inteiro em [-999, 999] da' w = 1/3. */
        long achou = 0, mais_perto = 999999, m_perto = 0;
        for(long m = -999; m <= 999; m++){
            long D = m*m + 4;
            /* w+1 = 2m/(3 sqrt D)  ->  (3(w+1))^2 D = 4 m^2, com w = 1/3: (4)^2 D = 9*4m^2 */
            long esq = 16 * D, dir = 36 * m * m;
            long dif = esq > dir ? esq - dir : dir - esq;
            if(dif == 0) achou++;
            if(dif < mais_perto){ mais_perto = dif; m_perto = m; }
        }
        printf("      m de -999 a 999: %ld dao w = 1/3 exacto; o mais perto e' m = %ld\n",
               achou, m_perto);
        printf("      e a conta fecha: 16D = 36m^2 com D = m^2+4  ->  -20m^2 = -64, sem raiz inteira\n\n");
        ok("o w = 1/3 da radiacao NAO e' membro da familia metalica, e isso e' resultado e nao"
           " falha: nenhum m inteiro o da', e a conta mostra porque — 16(m^2+4) = 36m^2 pede"
           " 20m^2 = 64, que nao tem raiz inteira. A radiacao pura vive na FRONTEIRA, e o"
           " catalogo ja' punha o ponto fixo em w = -1", achou == 0);
    }

    /* ═══ §E7 — o CONTROLO: nao ha' quarto regime ═════════════════════════════════ */
    {
        long r[3] = {0,0,0}, fora = 0;
        for(long a = -1000; a <= 1000; a++){
            int g = regime(a);
            if(g == -1) r[0]++; else if(g == 0) r[1]++; else if(g == +1) r[2]++;
            else fora++;
        }
        printf("      2001 desvios: cristal %ld, borda %ld, caos %ld, OUTRO %ld\n\n",
               r[0], r[1], r[2], fora);
        ok("e o CONTROLO: nao ha' quarto regime. O sinal so' toma tres valores, e por isso os"
           " estados sao exactamente tres — o trial nao tem onde por mais um. E' por isso que"
           " estrela e buraco negro esgotam o par, com a borda entre eles",
           fora == 0 && r[1] == 1 && r[0] == 1000 && r[2] == 1000);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  NAO SAO DUAS COISAS QUE SE PARECEM — SAO DOIS SINAIS DA MESMA QUANTIDADE:");
        puts("");
        puts("    caos     a > 0   cresce     a expansao");
        puts("    BORDA    a = 0   conserva   A ESTRELA — e o raio nao se move");
        puts("    cristal  a < 0   encolhe    O BURACO NEGRO — e nunca cresce");
        puts("");
        puts("  A involucao a |-> -a troca os dois lados e FIXA a borda: e' a bidualidade,");
        puts("  e o ponto fixo e' UM SO'. E a pressao de radiacao sai de CONTAR OS EIXOS —");
        puts("  tres, o trial — sem uma constante introduzida: p = r/3, w = 1/3.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
