/* tests/largura.c — A ESCADA DE LARGURAS É UMA LEI SÓ, e mede-se pelos dois caminhos.
 *
 * O `arquitetura.tex` §sec:torre fixa a escada E_16 → D_32 → D_64 → I_128 e dá um header
 * a cada andar. Postos lado a lado, `d16_mult_u` (dual16.h) e `d64_mult` (dual32.h) são o
 * MESMO texto com as larguras trocadas. O `binario.h` mostrou, um andar abaixo, que a lei
 * se escreve uma vez e o andar é argumento.
 *
 * Aqui pergunta-se, e mede-se: os headers especializados são instâncias de UM construtor
 * parametrizado pela largura? Se sim, a replicação é redundante — e a evidência vem ANTES
 * de qualquer remoção. Os headers não se tocam: correm lado a lado com o construtor.
 *
 * §LG0  a lei genérica bate com o produto NATIVO em SEIS andares (w = 1,2,4,8,16,32)
 * §LG1  instância w=16: lg_mult(16,·,·) É d16_mult_u — o par, bit a bit
 * §LG2  instância w=32: lg_mult(32,·,·) É d64_mult — o par, bit a bit
 * §LG3  o par é a MEMÓRIA DA DIVISÃO: guardar só o baixo perde, e conta-se quanto
 * §LG4  A MESMA DOBRA SEM TRANSPORTE é o produto de 𝔽₂ — o thm:transporte no produto
 * §LG5  o CONTROLO: sem o transporte do meio, o produto de ℕ diverge — e conta-se onde
 *
 * ── O QUE ISTO NÃO MEDE ────────────────────────────────────────────────────────────
 * Não mede que os headers devam sair. Mede que a lei é uma só. A remoção é decisão do
 * coordenador e vem depois: caminho novo → equivalência medida → remoção por último.
 * E o tecto w ≤ 32 é do VEÍCULO (o par tem de caber em 64 bits), não da lei: o andar
 * seguinte pede que o próprio veículo seja um par, que é o `i128.h`.
 */
#include <stdio.h>
#include <stdint.h>
#include "largura.h"
#include "dual16.h"
#include "dual32.h"
#include "binario.h"
#include "unidade.h"

/* as bordas que interessam em cada largura: 0, 1, meia largura, o topo */
static int bordas(int w, uint64_t *v){
    int h = w/2, n = 0;
    v[n++] = 0; v[n++] = 1; v[n++] = 2;
    v[n++] = lg_masc(h);            /* o topo da metade  */
    v[n++] = lg_masc(h) + 1;        /* o primeiro que sai da metade */
    v[n++] = lg_masc(w);            /* o topo da largura */
    v[n++] = lg_masc(w) - 1;
    v[n++] = (uint64_t)1 << (w-1);  /* o bit alto sozinho */
    return n;
}

int main(void){
    printf("\n=== A ESCADA DE LARGURAS: uma lei, seis andares ===\n");
    const int ANDAR[6] = {1,2,4,8,16,32};

    /* ═══ §LG0 A LEI GENÉRICA CONTRA O PRODUTO NATIVO ══════════════════════════ */
    printf("\n§LG0 lg_mult(w,·,·) contra o produto da máquina, nos seis andares.\n\n");
    {
        long mal = 0, casos = 0;
        printf("        w   pares vistos   modo\n");
        for(int i = 0; i < 6; i++){
            int w = ANDAR[i]; long vistos = 0;
            if(w <= 8){                                  /* exaustivo */
                uint64_t n = lg_masc(w) + 1;
                for(uint64_t a = 0; a < n; a++) for(uint64_t b = 0; b < n; b++){
                    vistos++;
                    if(lg_val(lg_mult(w,a,b), w) != a*b) mal++;
                }
                printf("        %2d  %10ld   exaustivo\n", w, vistos);
            } else {                                     /* bordas + amostra fixa */
                uint64_t bs[8]; int nb = bordas(w, bs);
                for(int x = 0; x < nb; x++) for(int y = 0; y < nb; y++){
                    vistos++;
                    if(lg_val(lg_mult(w,bs[x],bs[y]), w) != bs[x]*bs[y]) mal++;
                }
                uint64_t s = 20260820ULL;
                for(int k = 0; k < 200000; k++){
                    uint64_t a = lg_prox(&s) & lg_masc(w), b = lg_prox(&s) & lg_masc(w);
                    vistos++;
                    if(lg_val(lg_mult(w,a,b), w) != a*b) mal++;
                }
                printf("        %2d  %10ld   bordas (%d²) + 200000 amostras\n", w, vistos, nb);
            }
            casos += vistos;
        }
        printf("        total %ld pares, %ld falhas\n", casos, mal);
        ok("A LEI NÃO É DE UM ANDAR: é a mesma em seis. Um só corpo de função, com a largura"
           " por argumento, reproduz o produto da máquina em w = 1, 2, 4, 8, 16 e 32 — os"
           " três primeiros e o w=8 exaustivamente, os dois maiores nas bordas e em 200 000"
           " amostras cada. A aritmética é mod 2^w explícita em cada passo: o tipo da máquina"
           " é o veículo, a largura é o parâmetro. É o mesmo que o `binario.h` faz nos"
           " andares do corpo, agora na escada de larguras do `arquitetura.tex` §sec:torre",
           mal == 0 && casos > 0);
    }

    /* ═══ §LG1 INSTÂNCIA w=16: O CONSTRUTOR CONTRA dual16.h ════════════════════ */
    printf("\n§LG1 lg_mult(16,·,·) contra d16_mult_u — o PAR, bit a bit.\n\n");
    {
        long mal = 0, vistos = 0;
        uint64_t bs[8]; int nb = bordas(16, bs);
        for(int x = 0; x < nb; x++) for(int y = 0; y < nb; y++){
            LgPar g = lg_mult(16, bs[x], bs[y]);
            D32 h = d16_mult_u((uint16_t)bs[x], (uint16_t)bs[y]);
            vistos++;
            if(g.alto != h.alto || g.baixo != h.baixo) mal++;
        }
        uint64_t s = 16161616ULL;
        for(int k = 0; k < 300000; k++){
            uint16_t a = (uint16_t)lg_prox(&s), b = (uint16_t)lg_prox(&s);
            LgPar g = lg_mult(16, a, b);
            D32 h = d16_mult_u(a, b);
            vistos++;
            if(g.alto != h.alto || g.baixo != h.baixo) mal++;
        }
        printf("        %ld pares (bordas %d² + 300000 amostras), %ld divergências\n",
               vistos, nb, mal);
        ok("O ANDAR DE 16 DO HEADER É UMA INSTÂNCIA DO CONSTRUTOR, e não uma lei própria: o"
           " par (alto, baixo) que `lg_mult(16,·,·)` devolve é o mesmo que `d16_mult_u`"
           " devolve, campo a campo, em todos os pares vistos — incluindo as bordas, que são"
           " onde o transporte do meio acontece. Dois caminhos que não partilham código: um"
           " parte em metades recursivamente até um bit, o outro usa o produto nativo de"
           " 8×8. Isto não retira o header: dá a evidência de que ele é dispensável",
           mal == 0 && vistos > 0);
    }

    /* ═══ §LG2 INSTÂNCIA w=32: O CONSTRUTOR CONTRA dual32.h ════════════════════ */
    printf("\n§LG2 lg_mult(32,·,·) contra d64_mult — o PAR, bit a bit.\n\n");
    {
        long mal = 0, vistos = 0;
        uint64_t bs[8]; int nb = bordas(32, bs);
        for(int x = 0; x < nb; x++) for(int y = 0; y < nb; y++){
            LgPar g = lg_mult(32, bs[x], bs[y]);
            D64 h = d64_mult((unsigned)bs[x], (unsigned)bs[y]);
            vistos++;
            if(g.alto != h.alto || g.baixo != h.baixo) mal++;
        }
        uint64_t s = 32323232ULL;
        for(int k = 0; k < 300000; k++){
            unsigned a = (unsigned)lg_prox(&s), b = (unsigned)lg_prox(&s);
            LgPar g = lg_mult(32, a, b);
            D64 h = d64_mult(a, b);
            vistos++;
            if(g.alto != h.alto || g.baixo != h.baixo) mal++;
        }
        printf("        %ld pares (bordas %d² + 300000 amostras), %ld divergências\n",
               vistos, nb, mal);
        ok("E O ANDAR DE 32 TAMBÉM, PELO MESMO CORPO DE FUNÇÃO. `d16_mult_u` e `d64_mult`"
           " são, letra por letra, o mesmo texto com 16 trocado por 32 — e o construtor"
           " parametrizado reproduz os dois sem uma linha por andar. O que a escada de"
           " larguras tem de específico por andar não é a lei: é o tipo que a transporta",
           mal == 0 && vistos > 0);
    }

    /* ═══ §LG3 O PAR É A MEMÓRIA DA DIVISÃO ════════════════════════════════════ */
    printf("\n§LG3 Guardar só o baixo perde — e conta-se quanto.\n\n");
    {
        long com_alto = 0, total = 0, mal = 0;
        uint64_t s = 12345678ULL;
        for(int k = 0; k < 200000; k++){
            uint16_t a = (uint16_t)lg_prox(&s), b = (uint16_t)lg_prox(&s);
            LgPar g = lg_mult(16, a, b);
            total++;
            if(g.alto != 0) com_alto++;
            if(lg_val(g,16) != (uint64_t)a*b) mal++;          /* a volta reconstrói */
            if(g.baixo != (uint64_t)((unsigned)a*b & 0xFFFFu)) mal++;  /* o baixo é o truncado */
        }
        printf("        %ld produtos: %ld (%.1f%%) têm alto ≠ 0 — perdidos se se guardar 16 bits\n",
               total, com_alto, 100.0*com_alto/total);
        ok("O PAR NÃO É UMA CONVENIÊNCIA: é a metade que a divisão perderia. O produto de"
           " dois de 16 bits SAI do tipo, e guardar só o baixo é exactamente truncar — o que"
           " se mede aqui dos dois lados: o valor reconstrói-se do par (alto·2^16 + baixo é o"
           " produto exacto) e o baixo sozinho é o produto módulo 2^16. A maioria dos pares"
           " tem alto não nulo, de modo que a perda não é um caso raro de borda: é o caso"
           " comum. «A dualidade é a memória da divisão», e aqui é literal",
           mal == 0 && com_alto > total/2);
    }

    /* ═══ §LG4 A MESMA DOBRA SEM TRANSPORTE É 𝔽₂ ═══════════════════════════════ */
    printf("\n§LG4 Trocar a soma pelo XOR na composição dá o produto de 𝔽₂.\n\n");
    {
        long mal = 0, difere = 0, vistos = 0;
        for(uint64_t a = 0; a < 256; a++) for(uint64_t b = 0; b < 256; b++){
            vistos++;
            if(lg_mult_f2(8, a, b) != bn_clmul(a, b)) mal++;      /* == o carry-less */
            if(lg_mult_f2(8, a, b) != lg_val(lg_mult(8,a,b), 8)) difere++;
        }
        printf("        %ld pares em w=8: contra bn_clmul %ld falhas · difere do produto de ℕ"
               " em %ld\n", vistos, mal, difere);
        ok("E A DIFERENÇA ENTRE AS DUAS ARITMÉTICAS, NO PRODUTO, É A MESMA QUE ERA NA SOMA:"
           " o transporte. A dobra é uma só — partir em metades, quatro parcelas, compor —, e"
           " conforme se componha com a SOMA ou com o XOR sai o produto de ℕ ou o de 𝔽₂[x]"
           " sem redução. Medido contra `bn_clmul` do `binario.h`, que foi escrito por outra"
           " via, os 65 536 pares batem; e os dois produtos divergem na maioria deles, que é"
           " o transporte a fazer-se ver. É o `naturais.tex thm:transporte` um andar acima",
           mal == 0 && difere > 0 && vistos == 65536);
    }

    /* ═══ §LG5 O CONTROLO: sem o transporte do meio, diverge ═══════════════════ */
    printf("\n§LG5 O transporte do meio não é decoração — suprimi-lo quebra, e conta-se onde.\n\n");
    {
        long divergiu = 0, total = 0, sem_efeito = 0;
        for(uint64_t a = 0; a < 256; a++) for(uint64_t b = 0; b < 256; b++){
            /* a mesma composição de §LG0 com w=8, MAS ignorando o transporte do meio */
            int w = 8, h = 4;
            uint64_t mh = lg_masc(h), mw = lg_masc(w);
            uint64_t a0=a&mh, a1=(a>>h)&mh, b0=b&mh, b1=(b>>h)&mh;
            uint64_t p00=a0*b0, p01=a0*b1, p10=a1*b0, p11=a1*b1;
            uint64_t meio = (p01 + p10) & mw;                 /* sem o bit que transborda */
            uint64_t alto = (p11 + (meio >> h)) & mw;         /* transp NÃO somado */
            uint64_t baixo = (p00 + ((meio << h) & mw)) & mw;
            if(baixo < p00) alto = (alto + 1) & mw;
            uint64_t v = (alto << w) | baixo;
            total++;
            if(v != a*b) divergiu++; else sem_efeito++;
        }
        printf("        %ld pares: sem o transporte do meio, %ld divergem do produto"
               " (%ld coincidem por acaso)\n", total, divergiu, sem_efeito);
        ok("E O CONTROLO DIZ QUE A PEÇA MEDIDA É MESMO A PEÇA: retirando UMA linha — o bit"
           " que transborda na soma do meio — o produto deixa de bater, e não numa borda"
           " isolada. Os casos que continuam a coincidir são aqueles em que o meio não"
           " transborda, e é por isso que uma varredura pequena de números pequenos não"
           " apanharia o defeito: é preciso ir onde a soma sai da largura. Sem este controlo,"
           " §LG0--§LG2 estariam a afirmar que dois caminhos concordam sem mostrar que"
           " conseguem discordar",
           divergiu > 0 && sem_efeito > 0 && total == 65536);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
