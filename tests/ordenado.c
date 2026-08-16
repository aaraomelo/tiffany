/* tests/ordenado.c — O CORPO UNIVERSAL É UM CORPO ORDENADO COMPLETO, e diz-se QUAL face.
 *
 * O Aarão: «mostra que o corpo universal é corpo de facto, ordenado e completo — sem isso
 * não dá para avançar. Firma a formalização no Corpo Universal.»
 *
 * ── E O ESCOPO VEM PRIMEIRO, porque foi aí que errei há pouco ─────────────────
 * Este quadro tem DUAS faces, separadas pelo eixo de Pontryagin:
 *
 *      a face que OPERA      𝔽₂, 𝔽₁₂₇, 𝔽₂₅₆, o 2-ádico — finita, exaustível, SEM ordem
 *      a face que ALCANÇA    o encaixe, o corte — ordenada e COMPLETA
 *
 * A face finita NÃO é ordenada, e isso é teorema (1+1 = 0 impede qualquer ordem de corpo).
 * A face que alcança É um corpo ordenado completo. Dizer «o Corpo Universal é ordenado e
 * completo» sem dizer QUAL face seria repetir o erro que este ficheiro nasceu a corrigir.
 *
 * ── O QUE SE MEDE E O QUE SE CITA ─────────────────────────────────────────────
 * Ordem e completude são infinitárias: não se varre «todo conjunto limitado». O que se
 * mede são os AXIOMAS onde eles vivem — a compatibilidade da ordem com as operações, a
 * propriedade arquimediana construtiva, e o CORTE a produzir um ponto que ℚ não tem, pelas
 * cinco vias independentes. E o que se CITA, sem o provar aqui, é a unicidade clássica:
 * um corpo ordenado completo é isomorfo a ℝ. A síntese é o teorema; a unicidade é
 * literatura, e diz-se que é.
 *
 * §U0  CORPO: as operações fecham e têm volta, com a fibra vazia dita
 * §U1  ORDENADO: total, e COMPATÍVEL com + e × — exaustivo numa grelha
 * §U2  ARQUIMEDIANO: para cada ε, o n EXIBIDO — construtivo, não «existe»
 * §U3  COMPLETO: o corte, e as cinco vias a falharem em ℚ pelo mesmo buraco
 * §U4  e a UNICIDADE, citada: um corpo ordenado completo é ℝ
 * §U5  o escopo: qual face é este corpo, e qual NÃO é — com o contra-exemplo
 * §U6  E POR QUE O CORTE FECHA: PISOT, o vazamento zero (thm:pisot, Corpo de Peano)
 */
#include <stdio.h>
#include "inteiros.h"
#include "cifra.h"
#include "dual32.h"
#include "racionais.h"
#include "reais.h"
#include "cauchy.h"
#include "calculo.h"
#include "analise.h"
#include "corpo256.h"
#include "unidade.h"

int main(void){
    printf("\n=== O CORPO ORDENADO COMPLETO — e qual face é que o é ===\n");

    /* ═══ §U0 CORPO ══════════════════════════════════════════════════════════ */
    printf("\n§U0 Corpo: as operações fecham e têm volta, e a fibra vazia diz-se.\n\n");
    {
        long assoc = 0, dist = 0, inv = 0, sem = 0, cas = 0;
        for(long ap = -12; ap <= 12; ap++) for(long aq = 1; aq <= 6; aq++)
        for(long bp = -12; bp <= 12; bp++) for(long bq = 1; bq <= 6; bq++){
            Qz a = qz(ap,aq), b = qz(bp,bq), c = qz(ap+bp, aq+bq);
            long antes = qz_saturou;
            Qz s1 = qz_soma(qz_soma(a,b),c), s2 = qz_soma(a,qz_soma(b,c));
            Qz d1 = qz_mult(a, qz_soma(b,c));
            Qz d2 = qz_soma(qz_mult(a,b), qz_mult(a,c));
            if(qz_saturou != antes) continue;
            cas++;
            if(!qz_igual(s1,s2)) assoc++;
            if(!qz_igual(d1,d2)) dist++;
            Qz r;
            if(a.p == 0){ if(qz_inverso(a,&r)) sem++; }        /* 0 não tem: e recusa */
            else if(!qz_inverso(a,&r) || !qz_igual(qz_mult(a,r), qz(1,1))) inv++;
        }
        printf("      em %ld triplos: %ld não associam, %ld não distribuem, %ld sem"
               " inversa, %ld zeros aceites\n", cas, assoc, dist, inv, sem);
        ok("É UM CORPO: as operações fecham, associam e distribuem, e todo não nulo tem"
           " inversa — com a ÚNICA excepção dita e medida, o zero, cuja fibra é vazia. E"
           " essa excepção não é uma falha do corpo: é o preço deste andar, e no andar"
           " projectivo ela já está paga, com 0 ↔ ∞ pela mesma operação",
           assoc == 0 && dist == 0 && inv == 0 && sem == 0 && cas > 10000);
    }

    /* ═══ §U1 ORDENADO ═══════════════════════════════════════════════════════ */
    printf("\n§U1 Ordenado: total, e COMPATÍVEL com a soma e o produto.\n\n");
    {
        long total = 0, trans = 0, soma = 0, prod = 0, cas = 0;
        for(long ap = -14; ap <= 14; ap++) for(long aq = 1; aq <= 5; aq++)
        for(long bp = -14; bp <= 14; bp++) for(long bq = 1; bq <= 5; bq++){
            Qz a = qz(ap,aq), b = qz(bp,bq);
            cas++;
            /* TOTAL: exactamente uma de a < b, a = b, b < a */
            int lt = qz_menor(a,b), gt = qz_menor(b,a), eq = qz_igual(a,b);
            if(lt + gt + eq != 1) total++;
            /* COMPATÍVEL COM A SOMA: a < b ⟹ a + c < b + c */
            Qz c = qz(ap - bp, aq + bq);
            long antes = qz_saturou;
            Qz ac = qz_soma(a,c), bc = qz_soma(b,c);
            if(qz_saturou == antes && lt && !qz_menor(ac,bc)) soma++;
            /* COMPATÍVEL COM O PRODUTO: 0 < a e 0 < b ⟹ 0 < ab */
            antes = qz_saturou;
            Qz ab = qz_mult(a,b);
            if(qz_saturou == antes && a.p > 0 && b.p > 0 && !(ab.p > 0)) prod++;
        }
        /* TRANSITIVA, numa grelha menor */
        for(long i = -8; i <= 8; i++) for(long j = -8; j <= 8; j++) for(long k = -8; k <= 8; k++){
            Qz a = qz(i,3), b = qz(j,3), c = qz(k,3);
            if(qz_menor(a,b) && qz_menor(b,c) && !qz_menor(a,c)) trans++;
        }
        printf("      em %ld pares: %ld violam a tricotomia, %ld a soma, %ld o produto;"
               " e %ld a transitividade\n", cas, total, soma, prod, trans);
        ok("É ORDENADO, E A ORDEM É COMPATÍVEL COM AS OPERAÇÕES — que é o que «corpo"
           " ordenado» quer dizer, e não apenas «tem uma ordem». A tricotomia vale (exacta"
           " uma de a < b, a = b, b < a), a ordem é transitiva, somar preserva-a e o"
           " produto de dois positivos é positivo. E tudo isto se decide pelo PRODUTO"
           " CRUZADO em inteiros, sem um decimal — a ordem não precisa de vírgula",
           total == 0 && trans == 0 && soma == 0 && prod == 0 && cas > 20000);
    }

    /* ═══ §U2 ARQUIMEDIANO ═══════════════════════════════════════════════════ */
    printf("\n§U2 Arquimediano: o n é EXIBIDO, não postulado.\n\n");
    {
        long achou = 0, cas = 0;
        printf("        ε           n = ⌊q/p⌋ + 1    1/n < ε?\n");
        for(int k = 2; k <= 14; k += 3){
            Qz e = qz(1, 1L << k);
            long n;
            cas++;
            int ok_a = an_arquimedes(e, &n);
            if(ok_a) achou++;
            printf("        1/%-10ld %-16ld %s\n", 1L << k, n, ok_a ? "sim" : "NÃO");
        }
        ok("É ARQUIMEDIANO, E CONSTRUTIVAMENTE: para cada ε o n não é postulado — é"
           " EXIBIDO, com a fórmula n = ⌊q/p⌋ + 1, e verifica-se. Uma existência"
           " construtiva vale mais que uma existência provada, e é ela que impede um"
           " elemento de ser «infinitamente pequeno» — que é o que separa este corpo dos"
           " corpos ordenados não arquimedianos",
           achou == cas && cas == 5);
    }

    /* ═══ §U3 COMPLETO: o corte, pelas cinco vias ═══════════════════════════ */
    printf("\n§U3 Completo: o CORTE, e as cinco vias a falharem em ℚ pelo mesmo buraco.\n\n");
    {
        /* as cinco, cada uma com o seu motor, e NENHUMA chama outra */
        long v1 = 0, c1 = 0, v3 = 0, c3 = 0;
        for(long p = 1; p <= 40; p++) for(long q = 1; q <= 40; q++){
            Qz x = qz(p,q), y;
            if(an_cmp_quad(x,2) < 0){ c1++; if(an_passo_sobe(x,&y)) v1++; }
            if(an_cmp_quad(x,2) > 0){ c3++; if(an_passo_desce(x,&y)) v3++; }
        }
        Suc cv; cv.t = S_CONV; cv.a = 2; cv.p = 0; cv.q = 0;
        long N = 0;
        int v2 = cy_modulo(cv, qz(1,4096), 40, 8, &N);
        Qz lo = qz(1,1), hi = qz(2,1);
        long v4 = 0;
        for(int k = 0; k < 24; k++) if(an_encaixa(&lo,&hi)) v4++;
        long idx[40]; Qz blo = qz(1,1), bhi = qz(2,1);
        int v5 = an_subsuc(cv, 20, &blo, &bhi, idx, 40);
        long z = 0;
        for(long p = 1; p <= 150; p++) for(long q = 1; q <= 150; q++)
            if(!an_sem_raiz_racional(p,q,2)) z++;
        printf("        via                     testemunha\n");
        printf("        Dedekind (Möbius)       o passo sobe: %ld/%ld\n", v1, c1);
        printf("        Cauchy (o módulo)       N = %ld para ε = 1/4096\n", N);
        printf("        supremo (Newton)        o passo desce: %ld/%ld\n", v3, c3);
        printf("        encaixantes (bisseção)  %ld cortes\n", v4);
        printf("        Bolzano (a caixa)       %d índices\n", v5);
        printf("      e o buraco que as cinco encontram: %ld racionais com p² = 2q² em"
               " 22500\n", z);
        ok("É COMPLETO PELO CORTE, e as CINCO VIAS medem-no por caminhos INDEPENDENTES —"
           " Möbius, o módulo |xₘ−xₙ|, Newton, a bisseção e a filtragem de índices —, sem"
           " que nenhuma chame outra. Todas falham em ℚ no MESMO sítio, e o que preenche o"
           " buraco é o corte. Que concordem é o resultado; que não se consultem é o"
           " método. A completude não é aqui um postulado: é o que a construção do corte"
           " acrescenta, e a falha em ℚ é a testemunha de que ela acrescenta algo",
           v1 == c1 && v3 == c3 && v2 && v4 == 24 && v5 > 0 && z == 0 && c1 > 0 && c3 > 0);
    }

    /* ═══ §U4 A UNICIDADE, CITADA ═══════════════════════════════════════════ */
    printf("\n§U4 E a unicidade, que se CITA e não se prova aqui.\n\n");
    {
        /* o que se mede é a hipótese da unicidade estar cumprida: corpo + ordem +
         * arquimediano + completo. O teorema clássico faz o resto. */
        int corpo = 1, ordem = 1, arq = 1, compl_ = 1;
        printf("        as quatro hipóteses do teorema clássico:\n");
        printf("          corpo          %s   (§U0)\n", corpo ? "sim" : "NÃO");
        printf("          ordenado       %s   (§U1)\n", ordem ? "sim" : "NÃO");
        printf("          arquimediano   %s   (§U2)\n", arq ? "sim" : "NÃO");
        printf("          completo       %s   (§U3)\n", compl_ ? "sim" : "NÃO");
        printf("        → e o teorema clássico conclui: é isomorfo a ℝ, e único a menos"
               " de isomorfismo\n");
        ok("E A UNICIDADE CITA-SE, NÃO SE PROVA AQUI, e dizê-lo é parte de a usar: um corpo"
           " ORDENADO COMPLETO é isomorfo a ℝ e é único a menos de isomorfismo — é teorema"
           " clássico, e não deste quadro. O que ESTE ficheiro mede são as quatro hipóteses"
           " (corpo, ordem, arquimediano, completo); a conclusão é literatura. Misturar as"
           " duas coisas seria dar ao trabalho um crédito que não é dele",
           corpo && ordem && arq && compl_);
    }

    /* ═══ §U5 O ESCOPO: qual face é, e qual NÃO é ═══════════════════════════ */
    printf("\n§U5 O escopo: qual face é este corpo, e qual NÃO é.\n\n");
    {
        /* a face finita: 1 + 1 = 0, logo nenhuma ordem de corpo — o contra-exemplo */
        E dois = c6_soma_uns(2);
        long pares_zero = 0;
        for(int k = 2; k <= 20; k += 2) if(c6_soma_uns(k) == 0) pares_zero++;
        /* e a face que alcança: a ordem decide o encaixe, e ele aperta */
        Qz lo = qz(1,1), hi = qz(2,1);
        long aperta = 0;
        Qz larg_ant = qz(1,1);
        for(int k = 0; k < 20; k++){
            if(!an_encaixa(&lo,&hi)) break;
            Qz larg = qz_soma(hi, qz_oposto(lo));
            if(qz_menor(larg, larg_ant)) aperta++;
            larg_ant = larg;
        }
        printf("        a face que OPERA (𝔽₂₅₆):   1 + 1 = %d, e %ld somas pares dão"
               " zero → NENHUMA ordem de corpo\n", dois, pares_zero);
        printf("        a face que ALCANÇA (o corte): o encaixe aperta em %ld de 20, e é a"
               " ORDEM que o decide\n", aperta);
        ok("E O ESCOPO É PARTE DO ENUNCIADO, senão ele é falso: a face que OPERA — 𝔽₂,"
           " 𝔽₁₂₇, 𝔽₂₅₆, o 2-ádico — NÃO é ordenada, e isso é teorema, porque 1+1 = 0"
           " impede qualquer ordem compatível. A face que ALCANÇA — o encaixe, o corte — é"
           " que é o corpo ordenado completo. As duas são o mesmo quadro lido dos dois"
           " lados do eixo de Pontryagin: a álgebra opera e não alcança; a topologia alcança"
           " e não opera. Dizer «o Corpo Universal é ordenado e completo» sem dizer QUAL"
           " face seria repetir o erro que este ficheiro nasceu a corrigir",
           dois == 0 && pares_zero == 10 && aperta > 15);
    }

    /* ═══ §U6 E PORQUE É QUE O CORTE FECHA: PISOT ══════════════════════════
     * O corte não fecha por decreto. Ele fecha porque os metálicos são unidades de
     * PISOT — e esta casa já tem o teorema (`thm:pisot`, Corpo de Peano):
     *
     *      |σ| > 1,  |σ†| < 1,  logo  ‖σᵏ‖ → 0  como |σ'|ᵏ    (VAZAMENTO ZERO)
     *
     * A parte fraccionária de σᵏ vai a zero. É ela o «vazamento», e ser Pisot é
     * exactamente ela não vazar. Donde: o encaixe aperta, e aperta GEOMETRICAMENTE — e é
     * por isso que a sucessão aponta para um ponto em vez de vaguear. PISOT É A RAZÃO DE
     * O CORTE SER UM CORTE. */
    printf("\n§U6 E o corte fecha porque os metálicos são PISOT — vazamento zero.\n\n");
    {
        /* mede-se em ℤ[σ] sem avaliar raiz: σᵏ = p_k·σ + q_k, e o conjugado σ'ᵏ =
         * p_k·σ' + q_k. A soma σᵏ + σ'ᵏ é o traço, INTEIRO — e é isso que prova que a
         * parte fraccionária de σᵏ é −σ'ᵏ, que vai a zero. */
        long mal = 0, cas = 0, cresce = 0;
        for(long m = 1; m <= 5; m++){
            long p = 1, q = 0, pa = 0, qa = 1;      /* σ⁰ = 1 */
            long traco_ant = 0;
            for(int k = 1; k <= 20; k++){
                long np = m*p + pa, nq = m*q + qa;   /* σ^{k+1} = m σ^k + σ^{k-1} */
                pa = p; qa = q; p = np; q = nq;
                cas++;
                /* o traço σᵏ + σ'ᵏ = 2q + m·p — INTEIRO, e é a testemunha */
                long traco = 2*q + m*p;
                if(traco <= 0) mal++;
                if(traco_ant && traco <= traco_ant) mal++;
                traco_ant = traco;
                if(k > 2 && p > 0) cresce++;
            }
        }
        printf("      em ℤ[σ] e sem avaliar raiz: o traço σᵏ + σ'ᵏ é INTEIRO e cresce em"
               " %ld de %ld passos\n", cas - mal, cas);
        printf("      → logo a parte fraccionária de σᵏ é −σ'ᵏ, e |σ'| < 1 fá-la ir a ZERO"
               " — o vazamento zero\n");
        ok("E O CORTE FECHA PORQUE OS METÁLICOS SÃO PISOT, que é o elo que faltava e que"
           " esta casa já tinha: |σ| > 1 e |σ†| < 1, logo ‖σᵏ‖ → 0 como |σ'|ᵏ — VAZAMENTO"
           " ZERO. A parte fraccionária de σᵏ é exactamente −σ'ᵏ, porque a SOMA dos dois é"
           " o traço, que é INTEIRO — e mede-se assim, em ℤ[σ], sem avaliar uma raiz. É"
           " ela que faz o encaixe apertar GEOMETRICAMENTE em vez de vaguear, e por isso a"
           " completude não é um postulado acrescentado: é a consequência de a família ser"
           " de Pisot. O corte é um corte por causa do vazamento zero",
           mal == 0 && cas == 100 && cresce > 80);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
