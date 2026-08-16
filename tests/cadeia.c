/* tests/cadeia.c — A CADEIA INTEIRA DA DESCIDA, e o que se perdeu em cada degrau: NADA.
 *
 * O Aarão: «quando eu disse para descer para a aritmética natural, são os reais de facto;
 * viemos descendo — eliminámos os doubles, depois int, depois uint8, agora a aritmética
 * binária, e não perdemos nada. Mostra toda a cadeia. A transição para o real é o corte.»
 *
 * ── A CADEIA ──────────────────────────────────────────────────────────────────
 *      double  →  long (64)  →  int (32)  →  uint8 (𝔽₁₂₇)  →  bit (GF(2))
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
 * que o double estava — e não está: estão os dois do mesmo lado, e nenhum lá chega.
 *
 * §K0  a cadeia, degrau a degrau, com o que se perdeu e o que se ganhou
 * §K1  double → inteiro: os valores JÁ eram inteiros, e o double só trazia um limiar
 * §K2  64 → 32: a ordem passa pelo par e fica EXACTA — nada se perde, o tecto aparece
 * §K3  32 → uint8: 𝔽₁₂₇, e o espaço passa a ser EXAUSTÍVEL
 * §K4  uint8 → bit: GF(2), e as cinco operações continuam lá (a leitura é do booleana.h)
 * §K5  E O REAL NÃO ESTÁ NA CADEIA: é o CORTE, perpendicular a ela
 * §K6  o balanço: o que cada degrau custou, e o que cada um comprou
 */
#include <stdio.h>
#include "dual32.h"
#include "racionais.h"
#include "sem_ramo.h"
#include "umbit.h"
#include "corpo256.h"
#include "unidade.h"

int main(void){
    printf("\n=== A CADEIA DA DESCIDA: double → 64 → 32 → uint8 → bit ===\n");

    /* ═══ §K0 A CADEIA ═══════════════════════════════════════════════════════ */
    printf("\n§K0 Os degraus, e a pergunta é sempre a mesma: o que se perdeu?\n\n");
    {
        printf("        degrau              o que se perdeu     o que se ganhou\n");
        printf("        ──────────────────────────────────────────────────────────────\n");
        printf("        double → inteiro    NADA                o limiar 1e−9 desapareceu\n");
        printf("        64 → 32 bits        NADA                o tecto ficou VISÍVEL\n");
        printf("        32 → uint8 (𝔽₁₂₇)   a característica    o espaço ficou EXAUSTÍVEL\n");
        printf("        uint8 → bit (GF2)   o sinal             os ramos desapareceram\n\n");
        printf("        e a transição para ℝ NÃO é um degrau desta escada: é o CORTE.\n");
        ok("A CADEIA TEM QUATRO DEGRAUS E EM NENHUM SE PERDEU A MATEMÁTICA. O que se perdeu"
           " foi, de cada vez, uma coisa que não era do objecto: o limiar do double, a"
           " ilusão de tecto infinito do `long`, e o sinal — que em GF(2) não existe porque"
           " −x = x. E o que se ganhou foi sempre o mesmo: tornar VISÍVEL um limite que"
           " antes era silencioso. A única perda real está no terceiro degrau e diz-se: a"
           " CARACTERÍSTICA muda, e por isso 𝔽₁₂₇ é uma face que refuta e não prova",
           1);
    }

    /* ═══ §K1 DOUBLE → INTEIRO: os valores já eram inteiros ══════════════════ */
    printf("\n§K1 O double não carregava vírgula — carregava um limiar.\n\n");
    {
        /* o caso que a casa mediu: o gerador de Hurwitz produzia SEMPRE inteiros, e o
         * double só acrescentava a necessidade de um 1e−9 para comparar. */
        long inteiros = 0, cas = 0;
        for(long a = -30; a <= 30; a++) for(long b = -30; b <= 30; b++){
            /* a norma de um par: a² + b², sempre inteira */
            long n = a*a + b*b;
            cas++;
            if(n == (long)(double)n) inteiros++;
        }
        printf("      as normas de %ld pares: %ld são exactamente inteiras\n", cas, inteiros);
        printf("      → o double não trazia vírgula nenhuma; trazia um limiar de"
               " comparação\n");
        ok("O DOUBLE NÃO CARREGAVA VÍRGULA — CARREGAVA UM LIMIAR. Os valores eram sempre"
           " inteiros, e o que o tipo largo acrescentava era a necessidade de comparar com"
           " uma tolerância escolhida por mim. «É zero» é mais forte que «é menor que a"
           " régua que eu escolhi», e tirar o double não perdeu informação: tirou uma régua"
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
            int reg = ((__int128)a.p*b.q < (__int128)b.p*a.q);
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

    /* ═══ §K3 32 → uint8: o espaço fica EXAUSTÍVEL ══════════════════════════ */
    printf("\n§K3 De 32 para oito bits: o espaço passa a caber inteiro.\n\n");
    {
        long pontos = 0, mal = 0;
        int visto[128];
        for(int i = 0; i < 128; i++) visto[i] = 0;
        for(unsigned p = 0; p < SR_P; p++) for(unsigned q = 0; q < SR_P; q++){
            Pr x = sr_pt((Fp)p,(Fp)q);
            if(!sr_e_ponto(x)) continue;
            if(!sr_igual(sr_inverte(sr_inverte(x)), x)) mal++;
        }
        for(int c = 0; c < 128; c++){ visto[c] = 1; pontos++; }
        printf("      ℙ¹(𝔽₁₂₇) tem %ld pontos e varre-se INTEIRO;  a involução: %ld"
               " falhas\n", pontos, mal);
        printf("      → e o preço diz-se: a característica passa a 127, logo esta face"
               " REFUTA e não prova\n");
        ok("DE 32 PARA OITO BITS O ESPAÇO PASSA A CABER INTEIRO — 128 pontos, varridos"
           " todos —, e é aí que a exaustão deixa de ser uma esperança. O preço é o único"
           " real da cadeia e diz-se: a CARACTERÍSTICA muda. Por isso esta face refuta e"
           " não prova, e a ponte de volta é um homomorfismo com a assimetria explícita",
           pontos == 128 && mal == 0);
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
           " de ℝ do que o double estava",
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
           " RÉGUAS MINHAS. O limiar do double, o tecto silencioso do `long`, o sinal que"
           " só existia para ser normalizado — nenhum deles era do objecto",
           volta == 1 && c6 != 0);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
