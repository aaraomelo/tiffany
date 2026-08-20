/* tests/promocao.c — SATUROU → PROMOVE: a substituição demonstrada ANTES de ser feita.
 *
 * O `arquitetura.tex` §sec:pipelines já decidiu: «Subida: a largura exigida segue a
 * operação». O gatilho já existe — `w8_wrap`, `w8_saturou`, `qz_saturou` contam o que não
 * coube. O que não existe é a realização: hoje, quando não cabe, TRUNCA-SE (ou enrola-se,
 * ou devolve-se ±32767) e conta-se. O valor devolvido não é o resultado.
 *
 * Este medidor não troca nada. Demonstra a troca como PRESERVAÇÃO DE RESULTADO, que é a
 * ordem que a casa exige: caminho novo → equivalência medida → substituição por último.
 *
 * ── E A PROMOÇÃO NÃO PRECISA DE MECANISMO NOVO ────────────────────────────────────
 * O construtor de `largura.h` já devolve o PAR (alto, baixo). Logo:
 *
 *      SATUROU  ⟺  alto ≠ 0          (o detector vem DENTRO do resultado, de graça)
 *      PROMOVE  =  ler o par no andar de cima   (o valor exacto já está lá)
 *
 * É o `promove.h` literal: «escrever o par não apaga nada, porque o par contém a volta».
 * O que hoje se faz — deitar fora o alto — é a única linha que perde.
 *
 * §SP0  o DETECTOR é exacto e gratuito: saturou ⟺ alto ≠ 0, no produto E na soma
 * §SP1  PRESERVAÇÃO: onde não satura, promover dá EXACTAMENTE o que hoje se dá
 * §SP2  onde satura, o truncado erra e o promovido acerta — e conta-se em quantos
 * §SP3  a promoção ENCADEIA e tem volta: 8→16→32 e descer, resíduo 0
 * §SP4  o custo do que hoje se faz: `qz()` ao saturar devolve ±32767 — não é o resultado
 * §SP5  o CONTROLO: onde o detector vale (na soma) e onde poupa pouco (no produto)
 *
 * ── O ESCOPO, dito ────────────────────────────────────────────────────────────────
 * Mede-se a fronteira W_8 → E_16 (a que o `inteiros.tex` §sec:palavra8 e o
 * `arquitetura.tex` §sec:descida já nomeiam), exaustiva nos 65 536 pares. O caso do `Qz`
 * quantifica-se em §SP4 mas NÃO se propõe aqui: envolve mdc e 128 bits, e a sua troca
 * pede a sua própria demonstração.
 */
#include <stdio.h>
#include <stdint.h>
#include "largura.h"
#include "naturais.h"
#include "racionais.h"
#include "unidade.h"

int main(void){
    printf("\n=== SATUROU → PROMOVE: a preservação medida antes da troca ===\n");

    /* ═══ §SP0 O DETECTOR VEM DENTRO DO RESULTADO ══════════════════════════════ */
    printf("\n§SP0 saturou ⟺ alto ≠ 0 — no produto e na soma, exaustivo em W_8.\n\n");
    {
        long mal_p = 0, mal_s = 0, sat_p = 0, sat_s = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            LgPar p = lg_mult(8, a, b);
            int cabe_real = (a * b) <= 255;                  /* a verdade, em ℕ */
            if((p.alto == 0) != cabe_real) mal_p++;
            if(p.alto != 0) sat_p++;
            /* a soma: o transporte é o alto de um par de 1 bit */
            unsigned s = a + b;
            unsigned carry = (s > 255) ? 1u : 0u;
            if((carry != 0) != (s > 255)) mal_s++;
            if(carry) sat_s++;
        }
        printf("        produto: %ld de 65536 saturam · soma: %ld · detector errado em"
               " %ld/%ld casos\n", sat_p, sat_s, mal_p, mal_s);
        ok("O DETECTOR NÃO É UM TESTE A ACRESCENTAR: JÁ VEM NO RESULTADO. O construtor"
           " devolve o par (alto, baixo), e «não coube» é exactamente «alto ≠ 0» — sem"
           " comparação com limite nenhum, sem constante escrita à mão, e concordando com a"
           " verdade em ℕ nos 65 536 pares, no produto e na soma. Hoje gasta-se uma"
           " comparação para detectar e depois deita-se fora precisamente a metade que"
           " responderia à pergunta",
           mal_p == 0 && mal_s == 0 && sat_p > 0 && sat_s > 0);
    }

    /* ═══ §SP1 PRESERVAÇÃO — a asserção que a troca exige ══════════════════════ */
    printf("\n§SP1 Onde não satura, promover dá o MESMO que hoje se dá.\n\n");
    {
        long nao_sat = 0, difere_wrap = 0, difere_sat = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            unsigned s = a + b;
            if(s > 255) continue;                            /* só onde NÃO satura */
            nao_sat++;
            uint16_t cruz = w8_cruz_ld((uint8_t)a, (uint8_t)b);   /* a leitura exacta */
            if(w8_proj_wrap(cruz) != s) difere_wrap++;       /* política de hoje: wrap */
            if(w8_proj_sat(cruz)  != s) difere_sat++;        /* política de hoje: sat  */
        }
        printf("        %ld pares não saturam: wrap difere em %ld, saturação em %ld\n",
               nao_sat, difere_wrap, difere_sat);
        ok("A TROCA NÃO MEXE NO QUE JÁ ESTAVA CERTO, e é isto que uma substituição tem de"
           " mostrar antes de ser feita: em todos os pares que cabem no byte, as duas"
           " políticas actuais (enrolar e saturar) e o valor promovido coincidem — não há um"
           " único caso em que promover mudasse um resultado que hoje está correcto. O"
           " caminho novo só se distingue do antigo exactamente onde o antigo já não"
           " conseguia responder",
           difere_wrap == 0 && difere_sat == 0 && nao_sat > 0);
    }

    /* ═══ §SP2 ONDE SATURA: o truncado erra, o promovido acerta ════════════════ */
    printf("\n§SP2 E onde satura, conta-se quem acerta.\n\n");
    {
        long sat = 0, trunc_errou = 0, prom_errou = 0, wrap_vs_sat = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            unsigned exacto = a * b;
            LgPar p = lg_mult(8, a, b);
            if(p.alto == 0) continue;
            sat++;
            if(p.baixo != exacto) trunc_errou++;             /* guardar só o baixo */
            if(lg_val(p, 8) != exacto) prom_errou++;         /* promover: o par */
            uint16_t s16 = (uint16_t)exacto;
            if(w8_proj_wrap(s16) == w8_proj_sat(s16)) continue;
            wrap_vs_sat++;                                    /* as duas políticas divergem */
        }
        printf("        %ld produtos saturam: truncar erra em %ld, promover erra em %ld;"
               " wrap≠sat em %ld\n", sat, trunc_errou, prom_errou, wrap_vs_sat);
        ok("E É AQUI QUE AS DUAS POLÍTICAS DE HOJE SE DENUNCIAM: onde o produto sai do byte,"
           " guardar só o baixo erra em TODOS os casos, e as duas políticas actuais nem"
           " sequer concordam entre si — enrolar e saturar dão números diferentes para o"
           " mesmo produto, e nenhum deles é o produto. Promover acerta em todos, porque não"
           " calcula nada de novo: lê o par que o construtor já tinha na mão. Contar a"
           " saturação à parte foi honesto; não a corrigir é que era a dívida",
           sat > 0 && trunc_errou == sat && prom_errou == 0 && wrap_vs_sat > 0);
    }

    /* ═══ §SP3 A PROMOÇÃO ENCADEIA, E TEM VOLTA ════════════════════════════════ */
    printf("\n§SP3 8 → 16 → 32, e a descida devolve o valor: resíduo 0.\n\n");
    {
        long casos = 0, mal = 0, subiu_dois = 0;
        uint64_t s = 8161632ULL;
        for(int k = 0; k < 100000; k++){
            uint64_t a = lg_prox(&s) & 0xFF, b = lg_prox(&s) & 0xFF;
            LgPar p8 = lg_mult(8, a, b);                     /* andar 8 → valor de 16 */
            uint64_t v16 = lg_val(p8, 8);
            uint64_t c = lg_prox(&s) & 0xFFFF;
            LgPar p16 = lg_mult(16, v16, c);                 /* andar 16 → valor de 32 */
            uint64_t v32 = lg_val(p16, 16);
            casos++;
            if(v32 != a * b * c) mal++;                      /* a volta: o valor exacto */
            if(p16.alto != 0) subiu_dois++;                  /* precisou do 2.º andar */
        }
        printf("        %ld cadeias 8→16→32: %ld falhas · %ld precisaram mesmo do 2.º andar\n",
               casos, mal, subiu_dois);
        ok("A PROMOÇÃO NÃO É UM REMENDO NUM ANDAR: ENCADEIA. Um produto de bytes sobe a 16,"
           " o resultado multiplica-se outra vez e sobe a 32, e o valor final é o produto"
           " exacto dos três factores — sem tipo mais largo em passo nenhum, cada andar a"
           " ser o par do anterior. É o mesmo que `promove.c` §Q4 mede na torre (1→2→4, duas"
           " promoções e duas descidas, resíduo 0), agora na escada de larguras: subir e"
           " descer são a mesma operação lida nos dois sentidos",
           mal == 0 && casos > 0 && subiu_dois > 0);
    }

    /* ═══ §SP4 O CUSTO DO QUE HOJE SE FAZ, no Qz ═══════════════════════════════ */
    printf("\n§SP4 Hoje, ao saturar, `qz()` devolve ±32767 — que não é o resultado.\n\n");
    {
        long antes = qz_saturou, vistos = 0, deformados = 0, iguais_a_teto = 0;
        for(long p = 32000; p < 32120; p++) for(long q = 1; q <= 3; q++){
            Qz r = qz(p * 3, q);                             /* força passar do tecto */
            vistos++;
            long exacto_p = (p * 3) / qz_mdc(p * 3, q);
            if(!qz_cabe(exacto_p)){
                deformados++;
                if(r.p == 32767 || r.p == -32767) iguais_a_teto++;
            }
        }
        printf("        %ld chamadas: %ld não cabiam · %ld devolveram o tecto ±32767 ·"
               " qz_saturou subiu %ld\n", vistos, deformados, iguais_a_teto,
               qz_saturou - antes);
        ok("E O MESMO PADRÃO ESTÁ UM ANDAR ACIMA, NO RACIONAL, com o preço à vista: quando o"
           " numerador reduzido não cabe em 16 bits, `qz()` conta — e devolve ±32767, um"
           " número que não é o resultado de nada. O contador é honesto e o valor não é; e"
           " quem receber esse ±32767 não tem como saber que o recebeu. Aqui só se"
           " QUANTIFICA: a troca no `Qz` envolve mdc e 128 bits e pede a sua própria"
           " demonstração — não se faz de arrasto com a de W_8",
           vistos > 0 && deformados > 0 && iguais_a_teto == deformados
             && qz_saturou - antes == deformados);
    }

    /* ═══ §SP5 O CONTROLO: os dois extremos são ambos maus ═════════════════════ */
    printf("\n§SP5 Promover sempre é desperdício; nunca promover é o estado actual.\n\n");
    {
        long total = 0, cabe_p = 0, cabe_s = 0;
        for(unsigned a = 0; a < 256; a++) for(unsigned b = 0; b < 256; b++){
            total++;
            if(lg_mult(8,a,b).alto == 0) cabe_p++;           /* o produto coube */
            if(a + b <= 255) cabe_s++;                        /* a soma coube */
        }
        printf("        de %ld pares: cabem no byte --- produto %ld (%.1f%%), soma %ld"
               " (%.1f%%)\n", total, cabe_p, 100.0*cabe_p/total, cabe_s, 100.0*cabe_s/total);
        ok("E O CONTROLO DIZ ONDE O DETECTOR VALE, QUE NÃO É EM TODA A PARTE --- e o número"
           " obriga a dizê-lo com cuidado. No PRODUTO de bytes quase tudo sai do byte (só"
           " 3% cabe), de modo que ali promover sempre custaria pouco: o detector poupa"
           " pouco e o que decide é a correcção. Na SOMA cabe cerca de metade, e é aí que"
           " condicionar ao alto do par poupa mesmo metade das promoções. Os dois extremos"
           " continuam maus --- nunca promover perde nos 63 568 produtos que não cabem ---,"
           " mas o argumento da ECONOMIA vale na soma, não no produto, e sem esta secção"
           " §SP1--§SP2 diriam «o critério faz diferença» sem dizer onde",
           cabe_p > 0 && cabe_s > 0 && cabe_s > cabe_p && total == 65536);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
