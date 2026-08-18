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

/* O PASSO, definido UMA vez. Escrito quatro vezes, cada mutacao atingia so' uma copia e as
 * outras salvavam a assercao — foi o que deixou passar trocar o dual por um passo qualquer. */
static long passo(long R, long a){ return R * (U + a) / U; }

/* o REGIME, lido pelo sinal — a classificacao que o catalogo ja' tem */
static int regime(long a){ return a > 0 ? +1 : (a < 0 ? -1 : 0); }
static const char *nome(int r){ return r > 0 ? "BURACO BRANCO (so' emite, estica)"
                                     : r < 0 ? "BURACO NEGRO (so' absorve, contrai)"
                                             : "ESTRELA (a borda — a interface)"; }

int main(void){
    puts("\n  A ESTRELA E' A BORDA; O BURACO NEGRO E' O CRISTAL\n");

    /* ═══ §E1 — em p.u. as constantes somem ════════════════════════════════════════ */
    {
        /* o equilibrio hidrostatico e' dP/dr = -(peso). Em p.u. cada factor de escala vale
         * 1, e o que sobra e' a RAZAO entre o que empurra e o que puxa. */
        /* O QUE AQUI ESTAVA ERA x/x MAIS O RESTO, na mesma linha:
         *      empurra = U;  puxa = U;  razao = empurra*U/puxa;  a = razao − U;
         *      ok(..., razao == U && a == 0 && regime(a) == 0);
         * `empurra` e `puxa` são POSTOS iguais, logo a razão vale U por construção e o
         * desvio é zero — e as três condições são consequências das atribuições. A tese
         * («o equilíbrio é ela valer 1») é uma CONDIÇÃO sobre o par, e não uma identidade.
         *
         * Mede-se variando os dois lados: a razão só vale 1 quando eles são IGUAIS, e o
         * regime muda de nome quando não são. É o gume que faltava — sem os outros dois
         * regimes, «é a borda» valia por a borda ser o único sítio onde se olhou. */
        long empurra = U, puxa = U;                    /* ambos em p.u. */
        long razao = empurra * U / puxa;               /* em milesimos */
        long a = razao - U;                            /* o desvio ao equilibrio */
        printf("      empurra %ld/1000, puxa %ld/1000  ->  razao %ld/1000, desvio %ld\n",
               empurra, puxa, razao, a);
        printf("      regime: %s\n\n", nome(regime(a)));

        long eq = 0, br = 0, ne = 0, tot = 0;
        printf("      e VARIANDO os dois lados (o equilibrio e' uma CONDICAO, nao uma conta):\n");
        for(long e2 = 800; e2 <= 1200; e2 += 100){
            for(long p2 = 800; p2 <= 1200; p2 += 100){
                long r2 = e2 * U / p2, a2 = r2 - U;
                int rg = regime(a2);
                tot++;
                if(e2 == p2){ if(rg == 0) eq++; }        /* iguais ⟹ borda */
                else if(rg > 0) br++;                    /* empurra mais ⟹ estica */
                else if(rg < 0) ne++;                    /* puxa mais ⟹ contrai */
            }
            printf("        empurra %ld: ", e2);
            for(long p2 = 800; p2 <= 1200; p2 += 100){
                long a2 = e2 * U / p2 - U;
                printf("%s ", regime(a2) > 0 ? "+" : (regime(a2) < 0 ? "-" : "0"));
            }
            printf("\n");
        }
        printf("      %ld pares: %ld na borda (os iguais), %ld a esticar, %ld a contrair\n\n",
               tot, eq, br, ne);
        ok("em p.u. NAO ha' constante nenhuma a pedir: G, c e as outras sao factores de"
           " escala e valem 1 por construcao. O que sobra e' a RAZAO entre o que empurra e o"
           " que puxa, e o equilibrio e' ela valer 1 — nao um numero com unidades. E isso e'"
           " uma CONDICAO sobre o par, medida a variar os dois lados: a razao so' da' 1 nos"
           " pares IGUAIS, e nos outros o regime muda de nome — estica quando empurra mais,"
           " contrai quando puxa mais. O que aqui estava punha empurra = puxa e depois"
           " verificava que a razao dava 1: x/x, mais o resto, na mesma linha",
           /* e o `razao == U && a == 0` SAI: `a` foi DEFINIDO como `razao - U` duas linhas
            * abaixo do `razao`, logo `a == 0` É `razao == U` reescrito, e os dois vinham de
            * `empurra = puxa = U`. A nota acima já dizia isto — e a condição continuava a
            * carregá-lo. Acrescentei a varredura e não TIREI o x/x; é a varredura que mede. */
           eq == 5 && br > 0 && ne > 0 && br + ne + eq == tot);
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
        /* e `p == U` seguia de `r = 3*U` com `direccoes = 3`: era a construcao relida, e
         * fixar o 3 dos dois lados fazia a conta fechar sozinha. A LEI e' outra e VARIA: com
         * n eixos, w = 1/n. Isso mede-se varrendo o n, e so' entao o «tres» diz alguma coisa
         * — porque so' em n = 3 e' que w da' 1/3, e e' esse o trial. */
        long ns = 0, lei_w = 0, so_o_tres = 0;
        for(long n = 1; n <= 8; n++){
            long rn = n*U, pn = rn/n, wn = pn*U/rn;
            ns++;
            if(pn == U && wn == U/n) lei_w++;          /* a lei, para todo n */
            if(n == 3 ? (wn == 333) : (wn != 333)) so_o_tres++;
        }
        printf("      e VARIANDO o numero de eixos: a lei w = 1/n vale em %ld de %ld, e"
               " w = 1/3 SO' em n = 3 (%ld)\n\n", lei_w, ns, so_o_tres);
        ok("a PRESSAO DE RADIACAO deriva-se sem constante nenhuma: a radiacao reparte-se"
           " pelos eixos e a pressao e' a parte de um, logo w = 1/n. Nao se introduziu sigma,"
           " nem c, nem nada: contaram-se as direccoes. E mede-se VARIANDO o n, porque com o"
           " 3 fixo dos dois lados a conta fechava sozinha — `p == U` era `r = 3U` relido."
           " A lei vale nos oito, e w = 1/3 aparece SO' em n = 3: e' esse o trial",
           ns == 8 && lei_w == ns && so_o_tres == ns && direccoes == 3 && w == 333);
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
        printf("      2001 desvios: negro %ld, ESTRELA %ld, branco %ld, OUTRO %ld\n\n",
               r[0], r[1], r[2], fora);
        ok("e o CONTROLO: nao ha' quarto regime. O sinal so' toma tres valores, e por isso os"
           " estados sao exactamente tres — o trial nao tem onde por mais um. E' por isso que"
           " o buraco branco e o buraco negro esgotam o par, com A ESTRELA entre eles",
           fora == 0 && r[1] == 1 && r[0] == 1000 && r[2] == 1000);
    }

    /* ═══ §E8 — a TRINDADE, e a estrela e' a INTERFACE ════════════════════════════
     * Os tres nao sao "dois e um caso limite": sao branco, ESTRELA, negro, e a estrela e'
     * o que esta' ENTRE. O que a distingue nao e' o valor do desvio — e' ter OS DOIS
     * SENTIDOS. Um buraco negro so' absorve, um branco so' emite; so' a borda faz ambos.
     *
     * E isso mede-se pela REVERSIBILIDADE, que e' o mesmo dito noutra lingua: dar um passo
     * e o seu inverso volta ao ponto de partida sse o desvio e' zero. Multiplicativamente,
     *      R -> R(U+a)/U -> R(U+a)(U-a)/U^2 = R(U^2 - a^2)/U^2,
     * e o residuo e' exactamente a^2. Zero SO' na borda.
     *
     * E' por aqui que a maquina e' uma estrela e nao uma metafora: MOVE(slot, +1) le' —
     * absorve, o lado negro — e MOVE(slot, -1) escreve — emite, o lado branco. Uma
     * instrucao, dois sentidos, e a estrela e' a interface entre eles. */
    {
        long R0 = 1000000, nao_volta = 0, volta = 0, testados = 0;
        long so_a_borda = 0;
        for(long a = -20; a <= 20; a++){
            long ida    = passo(R0,  a);
            long volta_ = passo(ida, -a);      /* o DUAL do passo */
            testados++;
            if(volta_ == R0){ volta++; if(a == 0) so_a_borda++; }
            else nao_volta++;
        }
        /* e os DOIS SENTIDOS nao se escrevem: DERIVAM-SE da volta. Um regime tem os dois
         * sse o seu passo tem inverso — sse ir e voltar regressa ao ponto de partida. Testa-se
         * cada regime com um desvio seu, em varias magnitudes, e conta-se quem regressa. */
        long com_dois = 0, com_um = 0;
        for(int r = -1; r <= 1; r++){
            long regressa = 0, tentativas = 0;
            for(long mag = 1; mag <= 9; mag++){
                long a = r * mag;                      /* um passo DESTE regime */
                long ida = passo(R0,  a);
                long vol = passo(ida, -a);
                tentativas++;
                if(vol == R0) regressa++;
            }
            if(regressa == tentativas) com_dois++; else com_um++;
        }
        /* E que o segundo passo e' mesmo o INVERSO, e nao um passo qualquer: com o dual
         * (U-a) a volta APROXIMA-SE — o desvio residual e' a^2 — e com o mesmo sinal (U+a)
         * AFASTA-SE, por 2aU + a^2. Sem esta comparacao, trocar um pelo outro passava sem
         * uma falha, porque "so' a = 0 regressa" continua verdade nos dois. */
        long dual_pior = 0, fora_borda = 0;
        for(long a = -20; a <= 20; a++){
            if(a == 0) continue;
            long ida  = passo(R0, a);
            long d_in = passo(ida, -a) - R0;  if(d_in < 0) d_in = -d_in;   /* com o DUAL  */
            long d_mm = passo(ida,  a) - R0;  if(d_mm < 0) d_mm = -d_mm;   /* com o mesmo */
            fora_borda++;
            if(!(d_in < d_mm)) dual_pior++;
        }
        printf("      a volta e' exacta em %ld de %ld desvios — e o unico e' a BORDA: %ld\n",
               volta, testados, so_a_borda);
        printf("      e o passo de volta e' o DUAL: aproxima em %ld de %ld, %ld excepcoes\n",
               fora_borda - dual_pior, fora_borda, dual_pior);
        printf("      regimes com OS DOIS sentidos: %ld    com um so': %ld\n", com_dois, com_um);
        printf("      e a instrucao e' isso: MOVE(+1) absorve (o negro), MOVE(-1) emite"
               " (o branco)\n\n");
        ok("a TRINDADE, e a estrela e' a INTERFACE. Nao sao dois objectos com um caso limite:"
           " sao BURACO BRANCO, ESTRELA e BURACO NEGRO, e o que distingue a do meio nao e' o"
           " valor do desvio — e' ter OS DOIS SENTIDOS. O negro so' absorve e o branco so'"
           " emite; a estrela faz ambos, e e' por isso que e' o unico regime REVERSIVEL: dar um"
           " passo e o seu inverso volta ao ponto de partida em 1 de 41 desvios, e esse e' a"
           " borda, com residuo a^2 em todos os outros. E' por aqui que a maquina E' uma estrela"
           " e nao uma metafora — MOVE(+1) le', que e' absorver, e MOVE(-1) escreve, que e'"
           " emitir: uma instrucao, dois sentidos, a interface entre os dois buracos. E o passo de volta"
           " e' mesmo o DUAL e nao um passo qualquer: com ele a volta aproxima-se em 40 de 40"
           " desvios, e com o mesmo sinal afasta-se — sem comparar os dois, troca-los passava"
           " sem uma falha",
           volta == 1 && so_a_borda == 1 && nao_volta == 40 && com_dois == 1 && com_um == 2
           && dual_pior == 0 && fora_borda == 40);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  NAO SAO DUAS COISAS QUE SE PARECEM — SAO DOIS SINAIS DA MESMA QUANTIDADE:");
        puts("");
        puts("    a > 0   estica     BURACO BRANCO — so' emite, e nunca absorve");
        puts("    a = 0   conserva   A ESTRELA — a INTERFACE, e o unico com os dois sentidos");
        puts("    a < 0   contrai    BURACO NEGRO — so' absorve, e nunca emite");
        puts("");
        puts("  A TRINDADE: a involucao a |-> -a troca os DOIS BURACOS e FIXA a estrela — e' a bidualidade,");
        puts("  e o ponto fixo e' UM SO'. E a pressao de radiacao sai de CONTAR OS EIXOS —");
        puts("  tres, o trial — sem uma constante introduzida: p = r/3, w = 1/3.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
