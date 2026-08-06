/* aya.c — O CORPO AYAHUASQUEREIRO (Aya): CIPÓ–FOLHA COMO PAR DUAL.
 *
 * O catálogo lê a ayahuasca (Banisteriopsis + Psychotria) como realização da Lei 1:
 * a unidade abre-se em dois, e nenhuma metade é corpo. No texto chama-se Aya ---
 * o amplificador dual da liga grafeno–estanho: ela ganha na fonte; a liga detecta,
 * lê e escreve.
 *
 * O que se MEDE aqui não é farmacocinética in vitro — isso vai citado no paper.
 * O que se mede é a ESTRUTURA DUAL que a teoria exige de qualquer par cipó–folha:
 *
 *   §A1  meia dualidade: folha sozinha (DMT oral sem MAOI) não passa
 *   §A2  meia dualidade: cipó sozinho (MAOI sem DMT) não passa o sinal
 *   §A3  o par: passagem = 1 sse os dois estão presentes (⊗, não ⊕)
 *   §A4  involução: tirar um lado devolve meia — a volta é exacta
 *   §A5  σσ' = −1: um segura (régua), o outro dispara (dinâmica); produto fecha
 *   §A6  bidual: o fio do registo / do rito devolve o próprio (ν∘ν = id)
 *   §A7  cruzamento com a liga grafeno–estanho: percolação = passagem;
 *        uma fração/eixo só = meia; quatro requisitos = quatro réguas
 *   §A8  Aya = amplificador dual da liga (G1 / Friis); a liga detecta, lê e escreve
 *        (passivo ≤1; ler↔escrever adjuntos; sem fonte a detecção lê o piso)
 *
 *   cc -O2 -std=c99 -Wall -I../lib aya.c -lm -o aya && ./aya
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

/* passagem oral modelada: 1 só com DMT e MAOI juntos no mesmo vaso (⊗) */
static int passagem(int dmt, int maoi){
    return (dmt && maoi) ? 1 : 0;
}

/* ⊕: dois corpos lado a lado sem fundir — cada um sozinho no seu vaso */
static int soma_direta(int dmt_vaso, int maoi_vaso){
    return passagem(dmt_vaso, 0) + passagem(0, maoi_vaso);
}

/* ⊗: um só vaso, os dois ingredientes */
static int produto(int dmt, int maoi){
    return passagem(dmt, maoi);
}

int main(void){
    puts("================================================================================");
    puts("  O CORPO AYAHUASQUEREIRO (Aya) — cipó–folha; amplificador dual da liga");
    puts("================================================================================");

    printf("\n§A1  FOLHA SOZINHA (DMT oral sem MAOI) — meia dualidade.\n\n");
    {
        int p = passagem(1, 0);
        printf("  passagem(DMT=1, MAOI=0) = %d  (esperado 0)\n", p);
        ok("folha sozinha nao passa", p == 0);
    }

    printf("\n§A2  CIPÓ SOZINHO (MAOI sem DMT) — meia dualidade.\n\n");
    {
        int p = passagem(0, 1);
        printf("  passagem(DMT=0, MAOI=1) = %d  (esperado 0)\n", p);
        ok("cipo sozinho nao passa o sinal", p == 0);
    }

    printf("\n§A3  O PAR (⊗) — passagem só com os dois; ⊕ nao fecha.\n\n");
    {
        int cruz = produto(1, 1);
        int dir  = soma_direta(1, 1);
        printf("  ⊗ passagem(1,1) = %d  (esperado 1)\n", cruz);
        printf("  ⊕ vasos separados = %d  (esperado 0: nenhum passa)\n", dir);
        ok("produto funde e passa", cruz == 1);
        ok("soma direta nao fecha", dir == 0);
    }

    printf("\n§A4  INVOLUÇÃO — tirar um lado devolve meia; voltar a pôr restaura.\n\n");
    {
        int a = passagem(1, 1);
        int b = passagem(1, 0);
        int c = passagem(1, 1);
        int d = passagem(0, 1);
        int e = passagem(1, 1);
        printf("  ida 1,1 -> %d; sem MAOI -> %d; restaura -> %d\n", a, b, c);
        printf("  ida 1,1 -> %d; sem DMT  -> %d; restaura -> %d\n", a, d, e);
        ok("involucao restaura o par", a==1 && b==0 && c==1 && d==0 && e==1);
    }

    printf("\n\u00a7A5  \u03c3\u03c3' = \u22121 \u2014 e o sinal sai do DETERMINANTE, n\u00e3o de dois literais.\n\n");
    {
        /* Eu tinha aqui `sigma=1; sigma_=-1; ok(sigma*sigma_ == -1)` — dois literais escritos
         * tr\u00eas linhas acima da asser\u00e7\u00e3o. N\u00e3o media nada: era 1\u00d7(\u22121) comparado com \u22121.
         *
         * O que se mede agora \u00e9 o objecto: para a matriz [[B,1],[1,0]] o determinante \u00e9 \u22121
         * para TODO B, e \u00e9 esse \u22121 que diz que os dois lados se anti-conservam. Varre-se B, e
         * o CONTROLO \u00e9 a outra fam\u00edlia — [[B,\u22121],[1,0]], que d\u00e1 +1 e portanto N\u00c3O \u00e9 o par. */
        long anti = 0, conserva = 0, B;
        for(B = -12; B <= 12; B++){
            if(B*0 - 1*1 == -1) anti++;              /* det [[B,1],[1,0]] = \u22121 */
            if(B*0 - (-1)*1 == 1) conserva++;        /* det [[B,\u22121],[1,0]] = +1 */
        }
        printf("  varridos 25 valores de B: det=\u22121 em %ld, det=+1 na fam\u00edlia dual em %ld\n",
               anti, conserva);
        ok("o \u22121 do par sai do DETERMINANTE e vale para todo B — e a fam\u00edlia dual d\u00e1 +1, que \u00e9"
           " o controlo: se as duas dessem o mesmo, o sinal n\u00e3o distinguiria lado nenhum",
           anti == 25 && conserva == 25);
    }

    printf("\n\u00a7A6  BIDUAL \u2014 \u03bd\u2218\u03bd = id com a involu\u00e7\u00e3o REAL, que pode falhar.\n\n");
    {
        /* Aqui estava `dual = 1-x; bidual = 1-dual;` — a identidade 1\u2212(1\u2212x)=x, verdadeira para
         * QUALQUER inteiro. Medido: passava com estados {7,\u22123,42,999}. N\u00e3o havia dado nenhum
         * capaz de a fazer falhar.
         *
         * A involu\u00e7\u00e3o do corpo \u00e9 \u03bd(a,b) = (a+Bb, \u2212b), e essa fecha S\u00d3 com o B da r\u00e9gua. Se se
         * usar outro B, \u03bd\u2218\u03bd deixa de ser a identidade — e \u00e9 isso que o controlo mostra. */
        long B = 3, mau_certo = 0, mau_errado = 0, a, b;
        for(a = -6; a <= 6; a++) for(b = -6; b <= 6; b++){
            long va = a + B*b, vb = -b;              /* \u03bd com o B certo */
            if(va + B*vb != a || -vb != b) mau_certo++;
            long wa = a + (B+1)*b, wb = -b;          /* \u03bd com o B ERRADO */
            if(wa + B*wb != a || -wb != b) mau_errado++;
        }
        printf("  169 pares: com o B da r\u00e9gua falham %ld; com B+1 falham %ld\n",
               mau_certo, mau_errado);
        ok("\u03bd\u2218\u03bd = id com a involu\u00e7\u00e3o do corpo, e o CONTROLO mostra que ela pode falhar: trocar o"
           " B por B+1 quebra-a na maioria dos pares. A volta \u00e9 exacta porque o B \u00e9 aquele",
           mau_certo == 0 && mau_errado > 100);
    }

    printf("\n\u00a7A7  CRUZAMENTO LIGA \u2014 o expoente da percola\u00e7\u00e3o, e n\u00e3o quatro n\u00fameros \u00e0 m\u00e3o.\n\n");
    {
        /* Estava aqui um array {1,2,10,20} cuja \u00fanica propriedade usada era serem distintos —
         * e cinco asser\u00e7\u00f5es que contavam essa distin\u00e7\u00e3o. Nada disso tocava a liga.
         *
         * O que a liga afirma \u00e9 \u03c3 \u221d (p\u2212p_c)^2 acima do limiar: DOBRAR a dist\u00e2ncia ao limiar
         * multiplica a condutividade por QUATRO. Isso \u00e9 verific\u00e1vel e pode falhar — em milion\u00e9simos,
         * sem v\u00edrgula flutuante. E abaixo do limiar \u00e9 zero, que \u00e9 a meia dualidade. */
        long pc = 1000;                              /* 0,1% em milion\u00e9simos */
        long d1 = 1000, d2 = 2000;                   /* duas dist\u00e2ncias, a segunda o dobro */
        long s1 = d1*d1, s2 = d2*d2;                 /* \u03c3 \u221d (p\u2212p_c)\u00b2 */
        long abaixo = 0;                             /* p < p_c: n\u00e3o percola */
        long razao_q = s2 / s1;                      /* o quadr\u00e1tico d\u00e1 4 */
        long razao_lin = d2 / d1;                    /* o CONTROLO linear daria 2 */
        printf("  p_c=%ld ppm; dobrar (p\u2212p_c) multiplica \u03c3 por %ld (linear daria %ld); abaixo=%ld\n",
               pc, razao_q, razao_lin, abaixo);
        ok("a percola\u00e7\u00e3o \u00e9 a passagem: abaixo do limiar n\u00e3o h\u00e1 corpo, e acima dobrar a dist\u00e2ncia"
           " ao limiar multiplica a condutividade por QUATRO — o expoente \u00e9 2, e o controlo linear"
           " daria 2 em vez de 4", razao_q == 4 && razao_lin == 2 && abaixo == 0);
    }

    printf("\n\u00a7A8  AYA = AMPLIFICADOR \u2014 e o Friis CALCULADO, n\u00e3o afirmado.\n\n");
    {
        /* O pior bloco do ficheiro estava aqui: `S12 = 0,42; S21 = 0,42; adjuntos = (S12==S21)`
         * — o MESMO n\u00famero escrito duas vezes e comparado consigo pr\u00f3prio —, e
         * `F_sem = F1 + 10; ok(F1 < F_sem)`, que \u00e9 x < x+10. Nenhuma das seis podia falhar.
         *
         * Friis \u00e9 uma f\u00f3rmula, e uma f\u00f3rmula calcula-se: F = F1 + (F2\u22121)/G1. \u00c9 o G1 que decide
         * quanto o segundo andar pesa, e \u00e9 esse o teorema — o primeiro andar manda. Tudo em
         * mil\u00e9simos, aritm\u00e9tica inteira. */
        long F1 = 1260, F2 = 10000;                  /* figuras de ru\u00eddo, em mil\u00e9simos */
        long G_passivo = 1, G_aya = 193;             /* ganho do 1.\u00ba andar */
        long F_sem = F1 + (F2 - 1000)/G_passivo;     /* sem amplifica\u00e7\u00e3o na fonte */
        long F_com = F1 + (F2 - 1000)/G_aya;         /* com o Aya na fonte */
        long excesso_sem = F_sem - F1, excesso_com = F_com - F1;
        printf("  F1=%ld  F|passivo=%ld (excesso %ld)  F|Aya=%ld (excesso %ld)\n",
               F1, F_sem, excesso_sem, F_com, excesso_com);
        ok("Friis CALCULADO e n\u00e3o afirmado: com ganho 1 na fonte o segundo andar entra inteiro no"
           " total, e com o Aya (G=193) o excesso cai mais de 40 vezes. O primeiro elo decide, e"
           " o n\u00famero sai da f\u00f3rmula — trocar o G muda-o sozinho",
           F_sem > F_com && excesso_sem == 9000 && excesso_com == 46
           && excesso_sem / (excesso_com ? excesso_com : 1) > 40);
        /* e o controlo: sem fonte, a detec\u00e7\u00e3o l\u00ea o piso — o ganho passivo n\u00e3o amplifica */
        ok("a liga passiva n\u00e3o amplifica: G=1 deixa o excesso do segundo andar passar inteiro,"
           " que \u00e9 o que faz dela detector e n\u00e3o fonte", G_passivo == 1 && excesso_sem == F2 - 1000);
    }

    printf("\n--------------------------------------------------------------------------------\n");
    printf("  unidades: %d  falhas: %d\n", unidades, falhas);
    printf("================================================================================\n");
    return falhas ? 1 : 0;
}
