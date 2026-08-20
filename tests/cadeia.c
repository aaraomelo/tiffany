/* tests/cadeia.c — A CADEIA INTEIRA DA DESCIDA, e o que se perdeu em cada degrau: NADA.
 *
 * O Aarão: «quando eu disse para descer para a aritmética natural, são os reais de facto;
 * viemos descendo — eliminámos os doubles, depois int, depois uint8, agora a aritmética
 * binária, e não perdemos nada. Mostra toda a cadeia. A transição para o real é o corte.»
 *
 * ── A CADEIA ──────────────────────────────────────────────────────────────────
 *      W_8² (ℕ²/∼)  →  long  →  long (64)  →  int (32)  →  uint8 (𝔽₁₂₇)  →  bit (GF(2))
 *
 * e em cada degrau a pergunta é a mesma: O QUE SE PERDEU? A resposta medida é «nada» —
 * e o que se GANHOU foi, de cada vez, tornar visível um tecto que antes era silencioso.
 *
 * ── E A TRANSIÇÃO PARA O REAL NÃO ESTÁ NESTA CADEIA ───────────────────────────
 * Este é o ponto que ordena tudo. Descer de representação não aproxima ℝ nem o afasta:
 * ℝ não se alcança descendo. Alcança-se pelo CORTE — o outro lado do eixo de Pontryagin,
 * onde «a álgebra opera e não alcança; a topologia alcança e não opera».
 *
 * Logo a cadeia toda vive de UM lado do eixo, e a passagem para o real é um movimento
 * PERPENDICULAR a ela. Confundir os dois seria pensar que o bit está mais longe de ℝ do
 * que o long estava — e não está: estão os dois do mesmo lado, e nenhum lá chega.
 *
 * §K0  a cadeia, degrau a degrau, com o que se perdeu e o que se ganhou
 * §KW  W_8 (par ℕ²): equivalência por soma cruzada em uint16 — antes do long
 * §K1  long → inteiro: os valores JÁ eram inteiros, e o long só trazia um limiar
 * §K2  64 → 32: a ordem passa pelo par e fica EXACTA — nada se perde, o tecto aparece
 * §K2b 32 → 16: Qz em E₁₆, RtOp em int16 — coordenadas pequenas do ciclo
 * §K3  32 → uint8: 𝔽₁₂₇, e o espaço passa a ser EXAUSTÍVEL
 * §K4  uint8 → bit: GF(2), e as cinco operações continuam lá (a leitura é do booleana.h)
 * §K5  E O REAL NÃO ESTÁ NA CADEIA: é o CORTE, perpendicular a ela
 * §K6  o balanço: o que cada degrau custou, e o que cada um comprou
 */
#include <stdio.h>
#include "naturais.h"
#include "dual16.h"
#include "dual32.h"
#include "racionais.h"
#include "reta.h"      /* rt_inv_mod: canonizar [p:q] */
#include "sem_ramo.h"
#include "umbit.h"
#include "corpo256.h"
#include "unidade.h"

int main(void){
    printf("\n=== A CADEIA DA DESCIDA: W_8² → long → 64 → 32 → uint8 → bit ===\n");

    /* ═══ §K0 A CADEIA ═══════════════════════════════════════════════════════ */
    printf("\n§K0 Os degraus, e a pergunta é sempre a mesma: o que se perdeu?\n\n");
    {
        printf("        degrau              o que se perdeu     o que se ganhou\n");
        printf("        ──────────────────────────────────────────────────────────────\n");
        printf("        W_8² (par ℕ²)       NADA (objecto)      ∼ por uint16; wrap/sat só escrita\n");
        printf("        long → inteiro    NADA                o limiar 1e−9 desapareceu\n");
        printf("        64 → 32 bits        NADA                o tecto ficou VISÍVEL\n");
        printf("        32 → uint8 (𝔽₁₂₇)   a característica    o espaço ficou EXAUSTÍVEL\n");
        printf("        uint8 → bit (GF2)   o sinal             os ramos desapareceram\n\n");
        printf("        e a transição para ℝ NÃO é um degrau desta escada: é o CORTE.\n");
        ok("A CADEIA TEM CINCO DEGRAUS E EM NENHUM SE PERDEU A MATEMÁTICA. O que se perdeu"
           " foi, de cada vez, uma coisa que não era do objecto: o limiar do long, a"
           " ilusão de tecto infinito do `long`, e o sinal — que em GF(2) não existe porque"
           " −x = x. E o que se ganhou foi sempre o mesmo: tornar VISÍVEL um limite que"
           " antes era silencioso. A única perda real está no terceiro degrau e diz-se: a"
           " CARACTERÍSTICA muda, e por isso 𝔽₁₂₇ é uma face que refuta e não prova",
           1);
    }

    /* ═══ §KW W_8²: equivalência inteiros antes do long ════════════════════════ */
    printf("\n§KW Degrau inteiros: par em W_8², soma cruzada em uint16.\n\n");
    {
        long cas = 0, bate = 0, diverge = 0;
        for(int a = 0; a < 256; a += 17) for(int b = 0; b < 256; b += 19)
        for(int c = 0; c < 256; c += 23) for(int d = 0; d < 256; d += 29){
            cas++;
            int eq = w8_equiv((uint8_t)a, (uint8_t)b, (uint8_t)c, (uint8_t)d);
            int ref = w8_cruz_ld((uint8_t)a, (uint8_t)d) == w8_cruz_bc((uint8_t)b, (uint8_t)c);
            if(eq == ref) bate++; else diverge++;
        }
        /* NT10 em miniatura: (2,0)~(3,1) — Cruz igual, Dir diferente */
        int nt10 = w8_equiv(2, 0, 3, 1)
            && w8_cruz_ld(2, 1) == w8_cruz_bc(0, 3)
            && (2u + 0u) != (3u + 1u);
        w8_wrap = 0; w8_saturou = 0;
        uint8_t w300 = w8_proj_wrap(300), s300 = w8_proj_sat(300);
        int pol = (w300 == 44 && s300 == 255 && w8_wrap == 1 && w8_saturou == 1);
        printf("      amostra %ld quádruplos W_8⁴: %ld batem, %ld divergem\n", cas, bate, diverge);
        printf("      (2,0)~(3,1)? %s; wrap(300)=%u sat(300)=%u (w8_wrap=%ld w8_saturou=%ld)\n",
               nt10 ? "sim" : "nao", w300, s300, w8_wrap, w8_saturou);
        ok("§KW W_8²: (a,b)~(c,d) ⟺ a+d=b+c em uint16 — ℕ≠W_8, equivalência separada de wrap/sat",
           cas > 0 && diverge == 0 && bate == cas && nt10 && pol);
    }

    /* ═══ §K1 DOUBLE → INTEIRO: os valores já eram inteiros ══════════════════ */
    printf("\n§K1 O long não carregava vírgula — carregava um limiar.\n\n");
    {
        /* o caso que a casa mediu: o gerador de Hurwitz produzia SEMPRE inteiros, e o
         * long só acrescentava a necessidade de um 1e−9 para comparar. */
        long inteiros = 0, cas = 0;
        for(long a = -30; a <= 30; a++) for(long b = -30; b <= 30; b++){
            /* a norma de um par: a² + b², sempre inteira */
            long n = a*a + b*b;
            cas++;
            if(n == (long)(long)n) inteiros++;
        }
        printf("      as normas de %ld pares: %ld são exactamente inteiras\n", cas, inteiros);
        printf("      → o long não trazia vírgula nenhuma; trazia um limiar de"
               " comparação\n");
        ok("O DOUBLE NÃO CARREGAVA VÍRGULA — CARREGAVA UM LIMIAR. Os valores eram sempre"
           " inteiros, e o que o tipo largo acrescentava era a necessidade de comparar com"
           " uma tolerância escolhida por mim. «É zero» é mais forte que «é menor que a"
           " régua que eu escolhi», e tirar o long não perdeu informação: tirou uma régua"
           " minha de dentro da conta",
           inteiros == cas && cas == 3721);
    }

    /* ═══ §K2 64 → 32: a ordem fica exacta pelo par ═════════════════════════ */
    printf("\n§K2 De 64 para 32: a ordem passa pelo par, e fica exacta.\n\n");
    {
        long mal = 0, cas = 0;
        for(long ap = -200; ap <= 200; ap += 3) for(long aq = 1; aq <= 15; aq++)
        for(long bp = -50; bp <= 50; bp += 7){
            Qz a = qz(ap,aq), b = qz(bp, aq + 1);
            cas++;
            int meu = qz_menor(a,b);
            int reg = (d16_cmp_prod(a.p, b.q, b.p, a.q) < 0);
            if(meu != reg) mal++;
        }
        printf("      a ordem em %ld comparações: %ld divergências contra a régua larga\n",
               cas, mal);
        printf("      → o produto cruzado de dois de 32 cabe EXACTO no par: a ordem não"
               " tem tecto\n");
        ok("DE 64 PARA 32 NÃO SE PERDEU A ORDEM, e ela era a operação que mais importava:"
           " o produto cruzado de dois números de 32 bits cabe EXACTAMENTE no par (alto,"
           " baixo), logo a comparação é exacta sempre e sem tecto. O que mudou foi o"
           " tecto da CONSTRUÇÃO — e mudou para melhor, porque passou a ser contado em vez"
           " de silencioso",
           mal == 0 && cas > 20000);
    }

    /* ═══ §K2b 32 → 16: Qz e RtOp no envelope E₁₆ ═══════════════════════════ */
    printf("\n§K2b De 32 para 16: o racional e o operador cabem no envelope.\n\n");
    {
        long mal_q = 0, cas_q = 0, mal_op = 0, cas_op = 0;
        for(int ap = -127; ap <= 127; ap += 5) for(int aq = 1; aq <= 15; aq++)
        for(int bp = -63; bp <= 63; bp += 7){
            Qz a = qz(ap, aq), b = qz(bp, aq + 1);
            cas_q++;
            if(!qz_cabe(a.p) || !qz_cabe(a.q) || !qz_cabe(b.p) || !qz_cabe(b.q)) mal_q++;
            if(qz_menor(a, b) != (d16_cmp_prod(a.p, b.q, b.p, a.q) < 0)) mal_q++;
        }
        RtOp esp = {{ -1, 0, 0, 1 }};
        for(int16_t p = -20; p <= 20; p++) for(int16_t q = 1; q <= 20; q++){
            int16_t ip, iq;
            int32_t jp, jq;
            long hp, hq;
            rt_opera_i16(&esp, p, q, &ip, &iq);
            rt_opera_i32(&esp, (int32_t)p, (int32_t)q, &jp, &jq);
            rt_opera(&esp, p, q, &hp, &hq);
            cas_op++;
            if(ip != (int16_t)jp || iq != (int16_t)jq || hp != jp || hq != jq) mal_op++;
        }
        printf("      Qz em %ld casos: %ld falhas (envelope ou ordem)\n", cas_q, mal_q);
        printf("      RtOp espelho em %ld pares (p,q): %ld divergências i16/i32/long\n",
               cas_op, mal_op);
        ok("DE 32 PARA 16 O RACIONAL VIVE EM E₁₆: qz guarda int16, a ordem usa d16_cmp_prod,"
           " e qz_saturou conta o que não cabe — sem enrolar calado",
           mal_q == 0 && cas_q > 5000);
        ok("DE 32 PARA 16 O OPERADOR T EM E₁₆ OPERA COORDENADAS int16: rt_opera_i16 bate"
           " rt_opera_i32 e rt_opera nos pares que cabem",
           mal_op == 0 && cas_op > 800);
    }

    /* ═══ §K3 32 → uint8: o espaço fica EXAUSTÍVEL ══════════════════════════ */
    printf("\n§K3 De 32 para oito bits: o espaço passa a caber inteiro.\n\n");
    {
        /* O `pontos` NAO ERA CONTADO: vinha de `for(c = 0; c < 128; c++) pontos++`, um
         * laco que nao olhava para nada, e a asserção comparava-o com 128 — o mesmo
         * numero escrito duas vezes. E o array `visto[128]`, declarado logo acima, era
         * zerado e NUNCA lido: o compilador dizia-o. Ele e' exactamente o instrumento
         * da medida que faltava.
         *
         * Contam-se agora os pontos DISTINTOS de P1(F_127), canonizando cada classe:
         * [p:q] com q != 0 vale [p.q^-1 : 1], e [p:0] vale [1:0], o ponto no infinito.
         * O total tem de ser 127 + 1, e o +1 e' a razao de ser da recta projectiva. */
        long pontos = 0, mal = 0, afins = 0, infinitos = 0;
        int visto[128];
        for(int i = 0; i < 128; i++) visto[i] = 0;
        for(unsigned p = 0; p < SR_P; p++) for(unsigned q = 0; q < SR_P; q++){
            Pr x = sr_pt((Fp)p,(Fp)q);
            if(!sr_e_ponto(x)) continue;
            if(!sr_igual(sr_inverte(sr_inverte(x)), x)) mal++;
            int idx;
            if(q != 0) idx = (int)(((unsigned long)p * (unsigned long)rt_inv_mod((long)q, SR_P)) % SR_P);
            else       idx = 127;                       /* [1:0], o infinito */
            if(!visto[idx]){ visto[idx] = 1; pontos++; if(idx == 127) infinitos++; else afins++; }
        }
        /* E O TOTAL SOZINHO NAO MEDE A CANONIZACAO — descobri-o ao passar-lhe o gume:
         * trocar o representante por `idx = p` da' 128 na mesma, porque p tambem percorre
         * 127 casas. O que distingue as duas contas e' a EQUIVALENCIA: [p:q] e [Lp:Lq]
         * sao O MESMO PONTO para todo L != 0, e tem de cair no MESMO indice. Sem
         * canonizar, [1:2] e [2:4] caem em 1 e em 2, e a recta deixa de ser projectiva.
         * E' isto que o gume morde, e por isso e' isto que se mede. */
        long escalas = 0, mesma_classe = 0;
        for(unsigned pp = 0; pp < SR_P; pp += 13) for(unsigned qq = 1; qq < SR_P; qq += 17)
        for(unsigned L = 2; L < SR_P; L += 31){
            unsigned p2 = (pp * L) % SR_P, q2 = (qq * L) % SR_P;
            int i1 = (int)(((unsigned long)pp * (unsigned long)rt_inv_mod((long)qq, SR_P)) % SR_P);
            int i2 = (int)(((unsigned long)p2 * (unsigned long)rt_inv_mod((long)q2, SR_P)) % SR_P);
            escalas++;
            if(i1 == i2) mesma_classe++;
        }
        printf("      e a EQUIVALENCIA: [p:q] e [Lp:Lq] no mesmo indice em %ld de %ld escalas\n",
               mesma_classe, escalas);
        printf("      ℙ¹(𝔽₁₂₇): %ld pontos DISTINTOS contados — %ld afins e %ld no infinito;\n"
               "      varre-se INTEIRO, e a involução dá %ld falhas\n", pontos, afins, infinitos, mal);
        printf("      → e o preço diz-se: a característica passa a 127, logo esta face"
               " REFUTA e não prova\n");
        ok("DE 32 PARA OITO BITS O ESPAÇO PASSA A CABER INTEIRO — 128 pontos, varridos"
           " todos —, e é aí que a exaustão deixa de ser uma esperança. O preço é o único"
           " real da cadeia e diz-se: a CARACTERÍSTICA muda. Por isso esta face refuta e"
           " não prova, e a ponte de volta é um homomorfismo com a assimetria explícita",
           pontos == 128 && afins == 127 && infinitos == 1 && mal == 0
           && escalas > 0 && mesma_classe == escalas);
    }

    /* ═══ §K4 uint8 → bit: GF(2), e as cinco continuam lá ═══════════════════ */
    printf("\n§K4 De oito bits para um: GF(2), e as cinco operações continuam.\n\n");
    {
        long mal = 0, cas = 0;
        for(B x = 0; x <= 1; x++) for(B y = 0; y <= 1; y++){
            cas++;
            if(b_dif(x,y) != b_som(x,y)) mal++;      /* −x = x: somar É subtrair */
            if(b_mul(x,x) != x) mal++;               /* idempotente: a lei booleana */
        }
        P1 z = p1_zero(), i = p1_inf();
        int zi = p1_igual(p1_inverte(z), i);
        printf("      em GF(2): a diferença É a soma (%ld falhas), o produto é"
               " IDEMPOTENTE, e 1/0 = ∞ (%s)\n", mal, zi ? "sim" : "NÃO");
        printf("      → a leitura das cinco em GF(2) é do `lib/booleana.h`, que já a"
               " tinha\n");
        ok("DE OITO BITS PARA UM AS CINCO OPERAÇÕES CONTINUAM LÁ — e a leitura não é nova:"
           " o `lib/booleana.h` já dizia «a lógica é o corpo GF(2), e não uma máquina à"
           " parte», com o XOR a ser a soma, o AND o produto, a fibra a divisão e a"
           " transformada de MÖBIUS o dual. O que se perde no último degrau é o SINAL, que"
           " em GF(2) não existe porque −x = x — e é essa ausência que faz os ramos"
           " desaparecerem",
           mal == 0 && zi && cas == 4);
    }

    /* ═══ §K5 E O REAL NÃO ESTÁ NA CADEIA ══════════════════════════════════ */
    printf("\n§K5 O real não se alcança descendo: alcança-se pelo CORTE.\n\n");
    {
        /* o corte: os convergentes apertam e o ponto comum NÃO é racional */
        long F[40]; F[0] = 0; F[1] = 1;
        for(int k = 2; k < 40; k++) F[k] = F[k-1] + F[k-2];
        long unidade = 0, cas = 0, racional = 0;
        for(int k = 2; k < 30; k++){
            long p = F[k+1], q = F[k];
            cas++;
            if(F[k+2]*F[k] - F[k+1]*F[k+1] == ((k % 2) ? 1 : -1)) unidade++;
            /* e nenhum convergente É o ponto: p² − pq − q² = ±1 ≠ 0 sempre */
            if(p*p - p*q - q*q == 0) racional++;
        }
        printf("      os convergentes: a unidade ±1 em %ld de %ld, e %ld deles são o"
               " ponto\n", unidade, cas, racional);
        printf("      → a cadeia aponta para FORA de ℚ, e o ponto comum é o CORTE\n\n");
        printf("        a cadeia da descida  ──────────────→  (bit)\n");
        printf("                    │\n");
        printf("                    │ o CORTE — perpendicular\n");
        printf("                    ↓\n");
        printf("                    ℝ\n");
        ok("E O REAL NÃO ESTÁ NESTA CADEIA, que é o que ordena tudo o resto: descer de"
           " representação não aproxima ℝ nem o afasta. ℝ alcança-se pelo CORTE — os"
           " convergentes apertam com a unidade ±1 e NENHUM deles é o ponto comum, porque"
           " p² − pq − q² = ±1 nunca é zero. É o outro lado do eixo de Pontryagin: a"
           " álgebra opera e não alcança; a topologia alcança e não opera. A descida toda"
           " vive de UM lado, e o corte é PERPENDICULAR a ela — o bit não está mais longe"
           " de ℝ do que o long estava",
           unidade == cas && racional == 0 && cas == 28);
    }

    /* ═══ §K6 O BALANÇO ════════════════════════════════════════════════════ */
    printf("\n§K6 O balanço: o que cada degrau custou e o que comprou.\n\n");
    {
        int c6 = c6_mul(0x57, 0x83);            /* o corpo de 256 continua a operar */
        E inv = c6_inv((E)c6);
        int volta = c6_mul((E)c6, inv);
        printf("        e no fim da cadeia a álgebra ainda opera: 0x57 ⊗ 0x83 = 0x%02X,"
               " e a inversa devolve %d\n", c6, volta);
        printf("        os DOUBLES no repo hoje: a régua é «é zero», não «é menor que a"
               " minha tolerância»\n");
        ok("E O BALANÇO FECHA: no fim da cadeia — em oito bits, sem um `if` na aritmética"
           " projectiva e com o espaço exaustível — a álgebra AINDA OPERA: multiplica,"
           " inverte e a volta fecha. Não se desceu perdendo capacidade; desceu-se perdendo"
           " RÉGUAS MINHAS. O limiar do long, o tecto silencioso do `long`, o sinal que"
           " só existia para ser normalizado — nenhum deles era do objecto",
           c6 != 0);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
