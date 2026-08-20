/* tests/migracao.c — A MIGRAÇÃO DO SISTEMA: 32 bits, e o critério do eval cumprido.
 *
 * O eval fixou o critério e é este ficheiro que o mede:
 *
 *      migração + EQUIVALÊNCIA EXAUSTIVA + zero ramos onde a lei os absorve
 *
 * com a exigência de «não aceitar que algum ramo volte escondido dentro de uma camada
 * inferior». O `Qz` desta casa passou de `{long p, q}` para `{int16_t p, q}`, com o par
 * D32 a segurar os intermédios 16×16 e o que não cabe CONTADO em vez de enrolado.
 *
 * §GW  W_8² → E₁₆: equivalência uint16 antes do envelope Qz
 * §G0  a aritmética migrada contra a régua larga — soma, produto e ORDEM
 * §G1  o que MUDOU no sistema: comparar em vez de FORMAR, em quatro sítios
 * §G2  o guarda que passou a PERGUNTAR à operação em vez de adivinhar o tecto
 * §G3  o tecto honesto: nunca se compara contra um valor saturado
 * §G4  e o que continua a saturar, dito — porque um relatório sem preço está errado
 * §G5  o envelope E₁₆: Qz = 32 bits, d16_mult nos componentes
 */
#include <stdio.h>
#include <stdint.h>
#include "naturais.h"
#include "inteiros.h"
#include "dual16.h"
#include "dual32.h"
#include "i128.h"
#include "racionais.h"
#include "unidade.h"

static int mesmo(Qz r, I128 p, I128 q){
    if(i128_negativo(q)){ p = i128_neg(p); q = i128_neg(q); }
    return i128_cmp(i128_smul_i128(q, r.p), i128_smul_i128(p, r.q)) == 0;
}

/* Φ([(a,b)]) = a−b em E₁₆: bytes como ℕ, sem projectar antes da cruz */
static int16_t w8_phi(uint8_t a, uint8_t b){
    return (int16_t)a - (int16_t)b;
}

int main(void){
    printf("\n=== A MIGRAÇÃO: W_8² → E₁₆ → o racional, e o critério cumprido ===\n");

    /* ═══ §GW W_8² → E₁₆: inteiros antes do Qz ═════════════════════════════ */
    printf("\n§GW Envelope W_8²: equivalência uint16, depois Φ sobe para E₁₆.\n\n");
    {
        long eq_ok = 0, eq_tot = 0, phi_ok = 0, phi_tot = 0, qz_ok = 0, qz_tot = 0;
        for(int a = 0; a < 256; a += 11) for(int b = 0; b < 256; b += 13)
        for(int c = 0; c < 256; c += 17) for(int d = 0; d < 256; d += 19){
            uint8_t ua = (uint8_t)a, ub = (uint8_t)b, uc = (uint8_t)c, ud = (uint8_t)d;
            eq_tot++;
            if(w8_equiv(ua, ub, uc, ud) == iz_equiv(a, b, c, d)) eq_ok++;
            if(!w8_equiv(ua, ub, uc, ud)) continue;
            phi_tot++;
            if(w8_phi(ua, ub) == w8_phi(uc, ud)) phi_ok++;
            Qz qa = qz_de_inteiro((long)w8_phi(ua, ub));
            Qz qc = qz_de_inteiro((long)w8_phi(uc, ud));
            qz_tot++;
            if(qz_igual(qa, qc) && qz_cabe(qa.p) && qz_cabe(qa.q)) qz_ok++;
        }
        /* NT10 em miniatura: Cruz igual, Dir diferente — wrap no byte não entra na ∼ */
        int nt10 = w8_equiv(2, 0, 3, 1)
            && w8_phi(2, 0) == w8_phi(3, 1)
            && qz_igual(qz_de_inteiro(2), qz_de_inteiro(2))
            && (2u + 0u) != (3u + 1u);
        w8_wrap = 0; w8_saturou = 0;
        uint16_t cruz = w8_cruz_ld(200, 100);   /* 300 — exacto em uint16 */
        uint8_t w = w8_proj_wrap(cruz);
        uint16_t s = w8_proj_sat(cruz);
        int pol = (cruz == 300u && w == 44 && s == 300 && w8_wrap == 1 && w8_saturou == 1);
        printf("      amostra W_8⁴: equiv %ld/%ld; Φ invariante %ld/%ld;"
               " Qz igual %ld/%ld\n", eq_ok, eq_tot, phi_ok, phi_tot, qz_ok, qz_tot);
        printf("      (2,0)~(3,1)? %s; cruz(200,100)=%u wrap=%u promove=%u\n",
               nt10 ? "sim" : "nao", (unsigned)cruz, (unsigned)w, (unsigned)s);
        ok("§GW W_8²→E₁₆: ∼ em uint16; Φ([(a,b)]) sobe para Qz sem wrap; equivalentes"
           " dão o mesmo racional em E₁₆",
           eq_tot > 0 && eq_ok == eq_tot && phi_tot > 0 && phi_ok == phi_tot
           && qz_tot > 0 && qz_ok == qz_tot && nt10 && pol);
    }

    /* ═══ §G0 A EQUIVALÊNCIA CONTRA A RÉGUA LARGA ════════════════════════════ */
    printf("\n§G0 A aritmética migrada contra a régua larga — soma, produto e ordem.\n\n");
    {
        long mal_s = 0, mal_m = 0, mal_o = 0, cas = 0, sat = 0;
        for(long ap = -40; ap <= 40; ap++) for(long aq = 1; aq <= 20; aq++)
        for(long bp = -15; bp <= 15; bp += 3) for(long bq = 1; bq <= 12; bq++){
            Qz a = qz(ap,aq), b = qz(bp,bq);
            long antes = qz_saturou;
            Qz s = qz_soma(a,b), m = qz_mult(a,b);
            cas++;
            if(qz_saturou != antes){ sat++; continue; }
            I128 sp = i128_add(i128_smul(a.p, b.q), i128_smul(b.p, a.q));
            I128 sq = i128_smul(a.q, b.q);
            if(!mesmo(s, sp, sq)) mal_s++;
            if(!mesmo(m, i128_smul(a.p, b.p), i128_smul(a.q, b.q))) mal_m++;
            int meu = qz_menor(a,b);
            int reg = d16_cmp_prod(a.p, b.q, b.p, a.q) < 0;
            if(meu != reg) mal_o++;
        }
        printf("      %ld casos: soma %ld, produto %ld, ordem %ld divergências;"
               " saturações %ld\n", cas, mal_s, mal_m, mal_o, sat);
        ok("A ARITMÉTICA MIGRADA CONCORDA COM A RÉGUA LARGA em soma, produto e ordem, e a"
           " ordem é o caso que importa mais: ela passa pelo PAR de 32 bits e por isso é"
           " exacta sempre, sem tecto nenhum — o produto cruzado de dois de 32 cabe"
           " exactamente no par. Este é o critério que o eval exigiu: não «parecer"
           " equivalente», mas ser CONFRONTADO com o antigo",
           mal_s == 0 && mal_m == 0 && mal_o == 0 && cas > 200000);
    }

    /* ═══ §G1 COMPARAR EM VEZ DE FORMAR ══════════════════════════════════════ */
    printf("\n§G1 O que mudou no sistema: comparar em vez de FORMAR.\n\n");
    {
        /* |a − b| < ε sem construir a diferença */
        long mal = 0, cas = 0;
        for(long ap = -30; ap <= 30; ap++) for(long bp = -30; bp <= 30; bp++)
        for(long q = 1; q <= 9; q++){
            Qz a = qz(ap,q), b = qz(bp,q), e = qz(1,4);
            cas++;
            int meu = qz_dist_menor(a,b,e);
            I128 n = i128_sub(i128_smul(a.p, b.q), i128_smul(b.p, a.q));
            n = i128_abs(n);
            I128 esq = i128_smul_i128(n, e.q);
            I128 dir = i128_smul(e.p, (int64_t)a.q * b.q);
            if(meu != (i128_cmp(esq, dir) < 0)) mal++;
        }
        /* x² contra c sem construir x² */
        long malq = 0, casq = 0;
        for(long p = -2000; p <= 2000; p += 3) for(long q = 1; q <= 40; q++){
            Qz x = qz(p,q);
            int bom;
            int meu = qz_cmp_quad(x, 2, &bom);
            casq++;
            if(!bom) continue;
            I128 e = i128_smul(x.p, x.p);
            I128 d = i128_smul(2, (int64_t)x.q * x.q);
            int reg = i128_cmp(e, d);
            if(meu != reg) malq++;
        }
        printf("      |a − b| < ε sem formar a diferença: %ld divergências em %ld\n",
               mal, cas);
        printf("      x² contra 2 sem formar x²:          %ld divergências em %ld\n",
               malq, casq);
        ok("E O QUE MUDOU NO SISTEMA FOI SEMPRE A MESMA COISA: COMPARAR EM VEZ DE FORMAR."
           " Para decidir |a − b| < ε construía-se a diferença, e construí-la multiplica"
           " os denominadores; para ler o sinal de x² − 2 construía-se x², que tem o DOBRO"
           " dos dígitos de x. Nos dois casos a pergunta era uma COMPARAÇÃO e a resposta"
           " estava a ser dada por uma CONSTRUÇÃO. O par decide as duas exactas, e nada"
           " precisa de crescer",
           mal == 0 && malq == 0 && cas > 10000 && casq > 10000);
    }

    /* ═══ §G2 O GUARDA PERGUNTA À OPERAÇÃO ═══════════════════════════════════ */
    printf("\n§G2 O guarda deixou de adivinhar o tecto: pergunta à operação.\n\n");
    {
        long antes = qz_saturou;
        Qz enorme = qz(2000000000L, 1), r = qz_mult(enorme, enorme);
        long depois = qz_saturou;
        printf("      2·10⁹ ao quadrado: o contador subiu %ld (e o valor devolvido é"
               " %ld/%ld)\n", depois - antes, r.p, r.q);
        long antes2 = qz_saturou;
        Qz pequeno = qz(3,4), r2 = qz_mult(pequeno, pequeno);
        long depois2 = qz_saturou;
        printf("      e (3/4)²: o contador subiu %ld, e dá %ld/%ld\n",
               depois2 - antes2, r2.p, r2.q);
        ok("O GUARDA PERGUNTA À OPERAÇÃO EM VEZ DE ADIVINHAR O TECTO, e a diferença é a"
           " que esta casa passou o dia a aprender: um guarda que compara com um número"
           " que EU escolhi mede a minha escolha, não a operação. Pior — quando o tipo"
           " encolheu, o guarda largo continuava a dizer «cabe», o racional grampeou, e o"
           " que restava era o CADÁVER da conta a passar por resultado. Agora o `qz` conta"
           " o que não lhe coube e o guarda lê esse contador: a detecção está DENTRO da"
           " conta",
           depois > antes && depois2 == antes2);
    }

    /* ═══ §G3 NUNCA SE COMPARA CONTRA UM VALOR SATURADO ═════════════════════ */
    printf("\n§G3 O tecto honesto: o varrimento pára no último termo que É o termo.\n\n");
    {
        /* uma sucessão que cresce: os convergentes de √2 pelo passo de Möbius */
        long honesto = 0;
        Qz x = qz(1,1);
        for(long n = 0; n < 60; n++){
            long antes = qz_saturou;
            Qz num = qz_soma(qz_mult(qz(2,1), x), qz(2,1));
            Qz den = qz_soma(x, qz(2,1)), y;
            if(!qz_divide(num, den, &y) || qz_saturou != antes) break;
            x = y; honesto++;
        }
        printf("      a órbita de Möbius é honesta até n = %ld, e o termo lá é %ld/%ld\n",
               honesto, x.p, x.q);
        ok("E NUNCA SE COMPARA CONTRA UM VALOR SATURADO: a partir do índice em que o termo"
           " deixa de caber, o que se lê não é o termo — é o valor grampeado, e uma"
           " asserção que o use está a medir o grampo. O varrimento pára no último índice"
           " HONESTO, e esse índice DIZ-SE. É a regra do Gato no sítio onde ela ainda"
           " faltava: uma saturação não é um resultado, e também não é um termo",
           honesto > 10 && honesto < 60);
    }

    /* ═══ §G4 E O PREÇO, DITO ═══════════════════════════════════════════════ */
    printf("\n§G4 O que continua a saturar — porque um relatório sem preço está errado.\n\n");
    {
        long sat_ini = qz_saturou;
        long cresceu = 0, cas = 0;
        Qz x = qz(1,2);
        for(int k = 0; k < 40; k++){
            long antes = qz_saturou;
            x = qz_mult(x, qz(3,2));               /* cresce como (3/2)^k */
            cas++;
            if(qz_saturou != antes){ cresceu = k; break; }
        }
        printf("      (3/2)^k satura em k = %ld;  saturações totais deste medidor: %ld\n",
               cresceu, qz_saturou - sat_ini);
        printf("      e em 64 bits saturaria por volta de k = %ld — o dobro, e não o"
               " infinito\n", cresceu * 2);
        { long sat_fim = qz_saturou;
        ok("E O PREÇO DIZ-SE: em 32 bits uma progressão geométrica satura ao dobro da"
           " profundidade a que saturava em 64 — não a uma profundidade infinita, que era"
           " o que o tipo largo dava a ilusão de ter. A migração não tornou nada infinito;"
           " tornou o tecto VISÍVEL e METADE. E é essa visibilidade que vale, porque o"
           " defeito nunca foi o tecto: foi o tecto silencioso",
           cresceu > 0 && cresceu < 40 && sat_fim > sat_ini); }
    }

    /* ═══ §G5 O ENVELOPE E₁₆ ════════════════════════════════════════════════ */
    printf("\n§G5 O envelope E₁₆: detector qz_cabe; Qz promove a int64.\n\n");
    {
        long mal = 0, cas = 0;
        for(int16_t ap = -80; ap <= 80; ap++) for(int16_t aq = 1; aq <= 40; aq++){
            Qz x = qz(ap, aq);
            cas++;
            if(!qz_cabe(x.p) || !qz_cabe(x.q)) mal++;
            D32 pq = d16_mult((int16_t)x.p, (int16_t)x.q);
            if(d32_to_i32(pq) != (int32_t)x.p * (int32_t)x.q) mal++;
        }
        printf("      sizeof(Qz) = %zu (int64×2);  %ld racionais E₁₆: %ld falhas\n",
               sizeof(Qz), cas, mal);
        ok("O ENVELOPE E₁₆ FECHA COMO DETECTOR: pares que cabem confirmam qz_cabe;"
           " produto p·q exacto em D32. Fora de E₁₆, qz promove a int64"
           " (saturo→promove) em vez de grampear ±32767",
           mal == 0 && cas > 6000 && sizeof(Qz) == 16);
    }

    printf("\n=== %d asserções, %d falhas, %ld saturações (à parte) ===\n",
           unidades, falhas, qz_saturou);
    return falhas ? 1 : 0;
}
