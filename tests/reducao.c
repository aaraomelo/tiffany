/* tests/reducao.c — A PONTE ENTRE AS FACES: 𝔽₁₂₇ não prova, mas REFUTA.
 *
 * O Teorema do Gato: a lei é universal, a face é da instância. Este ficheiro mede a ponte
 * entre as duas faces desta casa — ℚ, onde a matemática vive, e 𝔽₁₂₇, onde tudo é
 * exaustível e não há um ramo — e mede sobretudo o que ela PODE e o que NÃO pode dizer.
 *
 * §D0  a redução é um HOMOMORFISMO: leva soma em soma e produto em produto
 * §D1  e é TOTAL, o que só é possível por causa do ∞: 127 | q dá o POLO
 * §D2  comuta com a inversão e com a acção de Möbius — as duas faces fazem o mesmo
 * §D3  E ENTÃO ELA REFUTA: uma identidade falsa em ℚ cai em 𝔽₁₂₇, exaustivamente
 * §D4  e o que ela NÃO prova: coincidir em 𝔽₁₂₇ não é ser igual em ℚ — com testemunha
 * §D5  o custo: o refutador varre TODOS os casos e não tem um ramo
 */
#include <stdio.h>
#include "dual16.h"
#include "dual32.h"
#include "racionais.h"
#include "reta.h"
#include "reducao.h"
#include "unidade.h"

int main(void){
    printf("\n=== A PONTE ENTRE AS FACES: 𝔽₁₂₇ refuta, não prova ===\n");

    /* ═══ §D0 A REDUÇÃO É HOMOMORFISMO ═══════════════════════════════════════ */
    printf("\n§D0 Leva soma em soma e produto em produto.\n\n");
    {
        long mal_s = 0, mal_m = 0, cas = 0, fora = 0;
        for(long ap = -30; ap <= 30; ap++) for(long aq = 1; aq <= 12; aq++)
        for(long bp = -20; bp <= 20; bp += 3) for(long bq = 1; bq <= 12; bq++){
            Qz a = qz(ap,aq), b = qz(bp,bq);
            long antes = qz_saturou;
            Qz s = qz_soma(a,b), m = qz_mult(a,b);
            if(qz_saturou != antes){ fora++; continue; }
            Pr ra, rb, rs, rm;
            if(!rd_de_qz(a,&ra) || !rd_de_qz(b,&rb)) { fora++; continue; }
            if(!rd_de_qz(s,&rs) || !rd_de_qz(m,&rm)) { fora++; continue; }
            cas++;
            /* a soma em ℙ¹(𝔽ₚ): [p:q]+[r:s] = [ps+rq : qs] */
            Pr soma = sr_pt(sr_som(sr_mul(ra.p,rb.q), sr_mul(rb.p,ra.q)),
                            sr_mul(ra.q,rb.q));
            Pr prod = sr_pt(sr_mul(ra.p,rb.p), sr_mul(ra.q,rb.q));
            if(sr_e_ponto(soma) && !sr_igual(soma, rs)) mal_s++;
            if(sr_e_ponto(prod) && !sr_igual(prod, rm)) mal_m++;
        }
        printf("      em %ld pares: soma %ld divergências, produto %ld;  fora do"
               " domínio %ld\n", cas, mal_s, mal_m, fora);
        ok("A REDUÇÃO É UM HOMOMORFISMO: reduzir e somar dá o mesmo que somar e reduzir, e"
           " o mesmo para o produto. É esta propriedade — e só ela — que dá à face pequena"
           " o direito de dizer alguma coisa sobre a grande: uma identidade que vale em ℚ"
           " TEM de valer na redução",
           mal_s == 0 && mal_m == 0 && cas > 10000);
    }

    /* ═══ §D1 A REDUÇÃO É TOTAL, e é o ∞ que a torna possível ═══════════════ */
    printf("\n§D1 Total — e 127 | q dá o POLO, não um erro.\n\n");
    {
        long polos = 0, ok_polo = 0, cas = 0, recusas = 0;
        for(long p = -60; p <= 60; p++) for(long q = 1; q <= 400; q++){
            Qz x = qz(p,q);
            Pr r;
            cas++;
            if(!rd_de_qz(x,&r)){ recusas++; continue; }
            if(rd_e_polo(x)){ polos++; if(sr_e_inf(r)) ok_polo++; }
        }
        printf("      %ld racionais reduzidos: %ld recusados; e %ld têm 127 | q — dos"
               " quais %ld dão o ∞\n", cas, recusas, polos, ok_polo);
        ok("E A REDUÇÃO É TOTAL, o que só é possível POR CAUSA DO ∞: quando 127 divide o"
           " denominador, em ℚ isso seria «não se pode reduzir»; aqui dá [x:0] = ∞, que é"
           " o ponto certo — é o POLO. É o Corolário 0 ↔ ∞ a trabalhar num sítio novo, e"
           " sem ele esta ponte teria um caso especial logo à entrada",
           polos > 0 && ok_polo == polos && recusas == 0 && cas > 40000);
    }

    /* ═══ §D2 COMUTA COM A INVERSÃO E COM MÖBIUS ═══════════════════════════ */
    printf("\n§D2 As duas faces fazem o mesmo: a redução comuta com as operações.\n\n");
    {
        long mal_i = 0, mal_g = 0, cas = 0;
        for(long p = -50; p <= 50; p++) for(long q = 1; q <= 30; q++){
            Qz x = qz(p,q);
            if(x.p == 0) continue;
            Qz ix;
            if(!qz_inverso(x, &ix)) continue;
            Pr rx, rix;
            if(!rd_de_qz(x,&rx) || !rd_de_qz(ix,&rix)) continue;
            cas++;
            if(!sr_igual(sr_inverte(rx), rix)) mal_i++;
            /* e o gato: (2x+1)/x em ℚ contra o mesmo em 𝔽ₚ */
            Qz num = qz_soma(qz_mult(qz(2,1), x), qz(1,1));
            long antes = qz_saturou;
            Qz gx;
            int deu = qz_divide(num, x, &gx);
            if(!deu || qz_saturou != antes) continue;
            Pr rg;
            if(!rd_de_qz(gx,&rg)) continue;
            if(!sr_igual(sr_gato(2, rx), rg)) mal_g++;
        }
        printf("      em %ld racionais: inversão %ld divergências, gato %ld\n",
               cas, mal_i, mal_g);
        ok("E A REDUÇÃO COMUTA COM AS OPERAÇÕES: inverter em ℚ e reduzir dá o mesmo que"
           " reduzir e inverter, e o mesmo para o gato. As duas faces fazem literalmente a"
           " mesma coisa — o que muda é que numa delas há 128 pontos e na outra há"
           " infinitos, e é essa diferença que decide qual serve para quê",
           mal_i == 0 && mal_g == 0 && cas > 1000);
    }

    /* ═══ §D3 E ENTÃO ELA REFUTA ═══════════════════════════════════════════ */
    printf("\n§D3 O que a ponte dá: um refutador EXAUSTIVO e barato.\n\n");
    {
        /* uma identidade VERDADEIRA: (a+b)² = a² + 2ab + b² — não pode ser refutada */
        long refutou_verdade = 0, cas_v = 0;
        for(long ap = -20; ap <= 20; ap++) for(long bp = -20; bp <= 20; bp++){
            Qz a = qz(ap,3), b = qz(bp,5);
            Qz s = qz_soma(a,b), esq = qz_mult(s,s);
            Qz dir = qz_soma(qz_soma(qz_mult(a,a), qz_mult(qz(2,1), qz_mult(a,b))),
                             qz_mult(b,b));
            cas_v++;
            if(rd_refuta(esq, dir) == 1) refutou_verdade++;
        }
        /* uma identidade FALSA: (a+b)² = a² + b² — tem de ser refutada, e depressa */
        long refutou_falso = 0, cas_f = 0;
        for(long ap = -20; ap <= 20; ap++) for(long bp = -20; bp <= 20; bp++){
            Qz a = qz(ap,3), b = qz(bp,5);
            Qz s = qz_soma(a,b), esq = qz_mult(s,s);
            Qz dir = qz_soma(qz_mult(a,a), qz_mult(b,b));
            cas_f++;
            if(rd_refuta(esq, dir) == 1) refutou_falso++;
        }
        printf("      (a+b)² = a²+2ab+b² (VERDADE): refutada %ld vezes em %ld — tem de"
               " ser 0\n", refutou_verdade, cas_v);
        printf("      (a+b)² = a²+b²     (FALSA):   refutada %ld vezes em %ld\n",
               refutou_falso, cas_f);
        ok("E ENTÃO A FACE PEQUENA REFUTA, que é o que esta casa não tinha: uma identidade"
           " falsa em ℚ cai na redução, e cai sobre TODOS os casos, em milissegundos, sem"
           " um ramo e sem nada crescer. A verdadeira nunca é refutada — se fosse, o"
           " refutador estava errado e não a identidade. São os dois controlos que esta"
           " casa exige de qualquer buscador: um regime onde tem de achar e um onde tem de"
           " voltar vazio",
           refutou_verdade == 0 && refutou_falso > cas_f/2 && cas_v > 1000);
    }

    /* ═══ §D4 E O QUE ELA NÃO PROVA ════════════════════════════════════════ */
    printf("\n§D4 O que a ponte NÃO diz — com testemunha.\n\n");
    {
        /* dois racionais DIFERENTES cuja redução coincide: a testemunha do limite */
        long coincide = 0, cas = 0;
        Qz t1 = qz(0,1), t2 = qz(0,1);
        for(long p = 1; p <= 400 && !coincide; p++){
            Qz a = qz(1,1), b = qz(1 + p*127, 1);
            cas++;
            if(rd_refuta(a,b) == 0 && !qz_igual(a,b)){ coincide++; t1 = a; t2 = b; }
        }
        printf("      testemunha: %d/%d e %d/%d são DIFERENTES em ℚ e IGUAIS na redução\n",
               t1.p, t1.q, t2.p, t2.q);
        printf("      (porque diferem por um múltiplo de 127 — e a redução não vê isso)\n");
        ok("E O QUE A PONTE NÃO PROVA DIZ-SE, COM TESTEMUNHA: coincidir na redução NÃO é"
           " ser igual em ℚ. 1 e 128 são o mesmo ponto em 𝔽₁₂₇ e não são o mesmo racional."
           " Logo a face pequena é um REFUTADOR e não um demonstrador — pode dizer «isto é"
           " falso», nunca «isto é verdadeiro». Uma ponte que se usasse para provar seria"
           " a insinuação arquitectónica que esta casa persegue",
           coincide == 1 && !qz_igual(t1,t2));
    }

    /* ═══ §D5 O CUSTO ══════════════════════════════════════════════════════ */
    printf("\n§D5 O custo: exaustivo, sem ramo, e nada cresce.\n\n");
    {
        long cas = 0, sat = 0;
        long antes = qz_saturou;
        for(unsigned p = 0; p < SR_P; p++) for(unsigned q = 0; q < SR_P; q++){
            Pr x = sr_pt((Fp)p,(Fp)q);
            if(!sr_e_ponto(x)) continue;
            cas++;
            Pr y = sr_gato(2, x);
            if(!sr_e_ponto(y)) sat++;
        }
        printf("      %ld pontos varridos na face pequena: %ld saíram do domínio;"
               " saturações do racional: %ld\n", cas, sat, qz_saturou - antes);
        printf("      e as recusas da ponte ([0:0] vindo de um racional): %ld\n",
               rd_recusas);
        ok("E O CUSTO É O ARGUMENTO FINAL: a face pequena varre-se INTEIRA — 16128 pares —"
           " sem um ramo, sem uma saturação e sem que nada cresça. É por isso que ela é o"
           " sítio certo para desmentir: desmentir é barato lá e caro em ℚ, e a ponte é"
           " que permite trazer o desmentido de volta. Provar continua a ser trabalho de"
           " ℚ; DESMENTIR passa a ser trabalho de 𝔽₁₂₇",
           sat == 0 && cas == 16128);
    }

    /* ═══ §D6 A PONTE LÊ E₁₆: int16 mod 127, sem widen silencioso ═══════════ */
    printf("\n§D6 A redução lê p e q em E₁₆ — mod 127 directo.\n\n");
    {
        long mal = 0, cas = 0, col = 0;
        for(int16_t p = -126; p <= 126; p++) for(int16_t q = 1; q <= 126; q++){
            Qz x = { p, q };
            Pr r;
            cas++;
            if(!rd_de_qz(x, &r)){ mal++; continue; }
            int pm = (p % (int)SR_P + (int)SR_P) % (int)SR_P;
            int qm = (q % (int)SR_P + (int)SR_P) % (int)SR_P;
            if(qm == 0){ if(!sr_e_inf(r)) mal++; continue; }
            /* forma afim [pm·qm⁻¹ : 1] quando qm ≠ 0 */
            long inv = rt_inv_mod((long)qm, (long)SR_P);
            Pr esp = sr_pt((Fp)((long)pm * inv % (long)SR_P), (Fp)1);
            if(!sr_igual(r, esp)) mal++;
            /* colisão mod 127: distintos em ℚ podem coincidir — contar à parte */
            for(int16_t p2 = p + 1; p2 <= p + 127 && p2 <= 126; p2 += 127)
                if(rd_de_qz((Qz){p2, q}, &r) && sr_igual(r, esp)) col++;
        }
        printf("      %ld pares (p,q) int16: %ld falhas; colisões mod 127 contadas %ld\n",
               cas, mal, col);
        ok("A REDUÇÃO LÊ E₁₆ SEM WIDEN: p mod 127 e q mod 127 saem dos int16 directamente,"
           " e a forma afim bate sr_pt — a ponte herda o envelope da migração",
           mal == 0 && cas > 15000);
    }

    printf("\n=== %ld asserções, %ld falhas, %ld recusas da ponte ===\n",
           unidades, falhas, rd_recusas);
    return falhas ? 1 : 0;
}
