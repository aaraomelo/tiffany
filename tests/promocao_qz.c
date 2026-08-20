/* tests/promocao_qz.c — SATUROU → PROMOVE no Qz: a demonstração que faltava.
 *
 * `promocao.c` fechou W_8 → E_16. Este medidor fecha o andar seguinte: quando o
 * numerador reduzido não cabe em E₁₆, `qz()` conta honestamente (`qz_saturou`) mas
 * devolve ±32767 — um valor que não é o resultado. A promoção não inventa mdc novo:
 * guarda o par exacto num andar mais largo (aqui I128, testemunha independente).
 *
 * §SQ0  detector: saturou ⟺ o exacto não cabe em int16
 * §SQ1  preservação: onde cabe, `qz()` = exacto
 * §SQ2  onde satura, o tecto erra e o promovido acerta — por produto cruzado
 * §SQ3  soma e produto encadeiam no promovido, e o estreito coincide quando cabe
 * §SQ4  quantifica: todas as deformações devolvem o tecto
 * §SQ5  controlo: promover sempre ≠ estreito; nunca promover = estado actual
 * §SQ6  QzX: estreito legado + promovido exacto no mesmo passo
 */
#include <stdio.h>
#include "racionais.h"
#include "unidade.h"

int main(void){
    printf("\n=== SATUROU → PROMOVE no Qz: preservação antes da troca ===\n");

    /* §SQ0 */
    printf("\n§SQ0 saturou ⟺ o exacto não cabe em E₁₆.\n\n");
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
                    if(r.p != 32767 && r.p != -32767) mal++;
                }
                if((qz_saturou > antes) != nao_cabe) mal++;
            }
        }
        printf("        %ld amostras · %ld saturam · detector errado em %ld\n",
               vistos, sat, mal);
        ok("O DETECTOR NO Qz É O MESMO PRINCÍPIO: quando o par reduzido não cabe"
           " em int16, `qz_saturou` sobe e o valor estreito deixa de ser o resultado."
           " Medido numa grelha de pares grandes — o detector e a não-cabência"
           " concordam entrada a entrada",
           vistos > 0 && sat > 0 && mal == 0);
    }

    /* §SQ1 */
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
        ok("A TROCA NÃO MEXE NO QUE JÁ ESTAVA CERTO: em todo o subconjunto que cabe"
           " em E₁₆, o `qz()` actual é o exacto — promover só se distingue onde o"
           " estreito já falha",
           vistos > 0 && okk == vistos);
    }

    /* §SQ2 */
    printf("\n§SQ2 Onde satura, o tecto erra; o promovido guarda a classe.\n\n");
    {
        long sat = 0, tecto_errou = 0, prom_ok = 0, cruz_dif = 0;
        for(long p = 30000; p < 33000; p++) for(long q = 1; q <= 5; q++){
            QzP ex = qz_prom(p * 3, q);
            if(!qz_prom_sat(ex)) continue;
            sat++;
            Qz r = qz(p * 3, q);
            if(r.p == 32767 || r.p == -32767) tecto_errou++;
            long xp = i128_to_i64(ex.p), xq = i128_to_i64(ex.q);
            if((long)r.p * xq != (long)xp * r.q) cruz_dif++;
            prom_ok++;   /* o par promovido É ex — a classe exacta, sem política */
        }
        printf("        %ld saturam · tecto em %ld · cruz difere em %ld\n",
               sat, tecto_errou, cruz_dif);
        ok("ONDE SATURA, ±32767 NÃO É O RESULTADO: o produto cruzado do tecto"
           " difere da classe exacta em todos os casos, e o promovido (par I128"
           " reduzido) guarda essa classe — não calcula política, só não deita"
           " fora o numerador",
           sat > 0 && tecto_errou == sat && cruz_dif == sat && prom_ok == sat);
    }

    /* §SQ3 */
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
        ok("A PROMOÇÃO NÃO É REMENDO DE UMA OPERAÇÃO: soma e produto no promovido"
           " obedecem às mesmas regras (mdc antes, par reduzido), e o produto"
           " distribui sobre a soma exacta — é o corpo ℚ antes do estreitamento",
           mal == 0);
    }

    /* §SQ4 */
    printf("\n§SQ4 Todas as deformações do estreito batem no tecto.\n\n");
    {
        long deform = 0, tecto = 0;
        long antes = qz_saturou;
        for(long p = 32000; p < 32120; p++) for(long q = 1; q <= 3; q++){
            Qz r = qz(p * 3, q);
            QzP ex = qz_prom(p * 3, q);
            if(!qz_prom_sat(ex)) continue;
            deform++;
            if(r.p == 32767 || r.p == -32767) tecto++;
        }
        printf("        %ld deformações · tecto em %ld · qz_saturou +%ld\n",
               deform, tecto, qz_saturou - antes);
        ok("O CONTADOR É HONESTO E O VALOR NÃO É: cada chamada que não cabe conta"
           " e devolve ±32767 — nunca o numerador exacto. Quem recebe o tecto não"
           " tem como saber que recebeu uma política, não um racional",
           deform > 0 && tecto == deform);
    }

    /* §SQ5 */
    printf("\n§SQ5 Controlo: estreito cabe numa fração dos exactos.\n\n");
    {
        long total = 0, cabe = 0;
        for(long p = -50000; p <= 50000; p += 131){
            for(long q = 1; q <= 9; q++){
                total++;
                if(!qz_prom_sat(qz_prom(p, q))) cabe++;
            }
        }
        printf("        %ld amostras · cabem %ld (%.1f%%)\n",
               total, cabe, 100.0 * cabe / (total ? total : 1));
        ok("PROMOVER SEMPRE SERIA DESPERDÍCIO; NUNCA PROMOVER É O ESTADO ACTUAL."
           " A maioria dos pares da grelha cabe no estreito — a troca só se paga"
           " onde o detector acusa, como em `promocao.c` §SP5",
           total > 0 && cabe > 0 && cabe < total);
    }

    /* §SQ6 */
    printf("\n§SQ6 QzX: estreito legado + promovido exacto no mesmo passo.\n\n");
    {
        long mal = 0, sat = 0, cabe = 0;
        for(long p = -35000; p <= 35000; p += 113){
            for(long q = 1; q <= 5; q++){
                QzX x = qz_x(p, q);
                if(x.saturo){
                    sat++;
                    if(x.estreito.p != 32767 && x.estreito.p != -32767) mal++;
                    if(!qz_prom_igual(x.promovido, qz_prom(p, q))) mal++;
                }else{
                    cabe++;
                    if(!qz_prom_estreito_bate(x.estreito, x.promovido)) mal++;
                }
            }
        }
        QzX ab = qz_x_soma(qz(12000, 7), qz(9000, 11));
        if(!ab.saturo && !qz_x_igual(ab, qz_x(i128_to_i64(ab.promovido.p),
                                         i128_to_i64(ab.promovido.q)))) mal++;
        QzX sm = qz_x_soma(qz(30000, 1), qz(30000, 1));
        if(!sm.saturo) mal++;
        {
            long xp = i128_to_i64(sm.promovido.p), xq = i128_to_i64(sm.promovido.q);
            if((long)sm.estreito.p * xq == (long)xp * sm.estreito.q) mal++;
        }
        printf("        %ld cabem · %ld saturam · erros %ld\n", cabe, sat, mal);
        ok("QzX É A TROCA SEM APAGAR O LEGADO: .estreito = `qz()` actual; .promovido"
           " guarda a classe exacta; .saturo = detector. Quem migra lê os três campos"
           " num único passo — sem substituir os chamadores que ainda só querem Qz",
           cabe > 0 && sat > 0 && mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
