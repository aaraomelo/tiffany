/* tests/promocao_qz.c — SATUROU → PROMOVE no Qz (após a troca no hot path).
 *
 * `qz()` fora de E₁₆ conta (`qz_saturou`) e GUARDA o exacto em int64.
 * QzP/I128 permanece o testemunho acima do int64. QzX: saturo = saiu de E₁₆.
 */
#include <stdio.h>
#include "racionais.h"
#include "unidade.h"

int main(void){
    printf("\n=== SATUROU → PROMOVE no Qz (hot path migrado) ===\n");

    printf("\n§SQ0 saturou ⟺ o exacto não cabe em E₁₆; qz() promove.\n\n");
    {
        long mal = 0, sat = 0, vistos = 0;
        for(long p = -40000; p <= 40000; p += 97){
            for(long q = 1; q <= 7; q++){
                QzP ex = qz_prom(p, q);
                int nao_cabe = qz_prom_sat(ex);
                long antes = qz_saturou;
                Qz r = qz(p, q);
                vistos++;
                if(nao_cabe){
                    sat++;
                    if(!qz_prom_estreito_bate(r, ex)) mal++;
                    if(r.p == 32767 || r.p == -32767) mal++;
                }
                if((qz_saturou > antes) != nao_cabe) mal++;
            }
        }
        printf("        %ld amostras · %ld saturam · erros %ld\n", vistos, sat, mal);
        ok("Detector e promoção: fora de E₁₆, qz_saturou sobe e o valor É a classe exacta",
           vistos > 0 && sat > 0 && mal == 0);
    }

    printf("\n§SQ1 Onde cabe, `qz()` bate com o exacto.\n\n");
    {
        long okk = 0, vistos = 0;
        for(int p = -300; p <= 300; p++) for(int q = 1; q <= 30; q++){
            QzP ex = qz_prom(p, q);
            if(qz_prom_sat(ex)) continue;
            vistos++;
            Qz r = qz(p, q);
            if(qz_prom_estreito_bate(r, ex)) okk++;
        }
        printf("        %ld pares cabem · %ld batem\n", vistos, okk);
        ok("Preservação: em E₁₆, qz() continua o exacto",
           vistos > 0 && okk == vistos);
    }

    printf("\n§SQ2 Onde satura E₁₆, qz() acerta; cruz com tecto antigo diverge.\n\n");
    {
        long sat = 0, exact_ok = 0, tecto_dif = 0;
        for(long p = 30000; p < 33000; p++) for(long q = 1; q <= 5; q++){
            QzP ex = qz_prom(p * 3, q);
            if(!qz_prom_sat(ex)) continue;
            sat++;
            Qz r = qz(p * 3, q);
            if(qz_prom_estreito_bate(r, ex)) exact_ok++;
            long xp = i128_to_i64(ex.p), xq = i128_to_i64(ex.q);
            /* o antigo tecto ±32767/1 divergiria da classe */
            if(32767L * xq != xp * 1L) tecto_dif++;
        }
        printf("        %ld saturam · exactos %ld · tecto divergiria em %ld\n",
               sat, exact_ok, tecto_dif);
        ok("Fora de E₁₆ o promovido (qz) guarda a classe; ±32767 teria errado",
           sat > 0 && exact_ok == sat && tecto_dif == sat);
    }

    printf("\n§SQ3 Soma e produto encadeiam no promovido.\n\n");
    {
        long mal = 0;
        Qz a = qz(12345, 7), b = qz(9876, 11), c = qz(555, 13);
        QzP ab = qz_prom_soma(a, b);
        QzP abc1 = qz_prom_mult_pp(ab, qz_prom_de(c));
        QzP ac = qz_prom_mult(a, c), bc = qz_prom_mult(b, c);
        QzP abc2 = qz_prom_soma_pp(ac, bc);
        if(!qz_prom_igual(abc1, abc2)) mal++;
        printf("        cadeia soma→produto: %s\n", mal ? "falhou" : "fecha");
        ok("Soma e produto no promovido fecham (distributividade)", mal == 0);
    }

    printf("\n§SQ4 Deformações: zero tectos; todos exactos.\n\n");
    {
        long deform = 0, exactos = 0, tectos = 0;
        long antes = qz_saturou;
        for(long p = 32000; p < 32120; p++) for(long q = 1; q <= 3; q++){
            Qz r = qz(p * 3, q);
            QzP ex = qz_prom(p * 3, q);
            if(!qz_prom_sat(ex)) continue;
            deform++;
            if(qz_prom_estreito_bate(r, ex)) exactos++;
            if(r.p == 32767 || r.p == -32767) tectos++;
        }
        printf("        %ld deformações · exactos %ld · tectos %ld · qz_saturou +%ld\n",
               deform, exactos, tectos, qz_saturou - antes);
        ok("Hot path: conta e promove — nenhum ±32767",
           deform > 0 && exactos == deform && tectos == 0);
    }

    printf("\n§SQ5 Controlo: fração que cabe em E₁₆.\n\n");
    {
        long total = 0, cabe = 0;
        for(long p = -50000; p <= 50000; p += 131){
            for(long q = 1; q <= 9; q++){
                total++;
                if(!qz_prom_sat(qz_prom(p, q))) cabe++;
            }
        }
        { long d = total ? total : 1;
          printf("        %ld amostras · cabem %ld (%ld,%ld%%)\n",
                 total, cabe, 1000*cabe/d/10, 1000*cabe/d%10); }
        ok("Detector ainda distingue E₁₆ do promovido",
           total > 0 && cabe > 0 && cabe < total);
    }

    printf("\n§SQ6 QzX: estreito=qz() exacto; saturo=saiu de E₁₆.\n\n");
    {
        long mal = 0, sat = 0, cabe = 0;
        for(long p = -35000; p <= 35000; p += 113){
            for(long q = 1; q <= 5; q++){
                QzX x = qz_x(p, q);
                if(!qz_prom_estreito_bate(x.estreito, x.promovido)) mal++;
                if(x.saturo){
                    sat++;
                    if(x.estreito.p == 32767 || x.estreito.p == -32767) mal++;
                }else cabe++;
            }
        }
        QzX sm = qz_x_soma(qz(30000, 1), qz(30000, 1));
        if(!sm.saturo) mal++;
        if(!qz_prom_estreito_bate(sm.estreito, sm.promovido)) mal++;
        printf("        %ld cabem · %ld saturam · erros %ld\n", cabe, sat, mal);
        ok("QzX: estreito concorda com promovido; saturo só marca saída de E₁₆",
           cabe > 0 && sat > 0 && mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
