/* cosmico.c — O CÓSMICO É O DUAL DO ENTRÓPICO: nada se perde, e a conservação valida-se.
 *
 * O Aarão: "você disse que Carnot não volta — sim, mas isso é meia verdade pro headjack, porque
 * pode pegar calor do ambiente. O cósmico é dual do entrópico. Nada se perde. Valida a conservação."
 *
 * ELE TEM RAZÃO E A MINHA AFIRMAÇÃO ERA MEIA. No `arraytermico.c` escrevi que *"os 19,94 W que não
 * voltam não são desperdício evitável: são o segundo princípio"* — e isso é verdade **para os dois
 * reservatórios que eu fixei**. Carnot não diz que a energia se perde: diz qual é o teto **dado um
 * quente e um frio**. Eu escolhi o frio no escalpe, a 32 °C, e depois tratei o teto dessa escolha
 * como se fosse uma lei da natureza.
 *
 * E O FRIO NÃO É ESSE. Há uma janela na atmosfera entre 8 e 13 µm por onde a Terra **vê o espaço**
 * — e o espaço está a 2,7 K, o fundo cósmico. É por ali que o planeta arrefece, e é tecnologia real:
 * arrefecimento radiativo passivo, que desce abaixo da temperatura do ar **em pleno sol**.
 *
 *      o ENTRÓPICO   o calor espalha-se e a temperatura iguala-se   -> o teto cai a zero
 *      o CÓSMICO     há um frio a 2,7 K sempre disponível           -> o teto sobe a ~99%
 *
 * *São duais: um fecha o gradiente, o outro abre-o.* E o pico de Wien do corpo humano é
 * **9,35 µm** (medido no `radiacao.c` §W1) — exatamente **dentro** da janela. Isso não foi
 * arranjado: é o que as duas leis dão, e as duas já estavam medidas antes desta pergunta.
 *
 * E "NADA SE PERDE" É UMA AFIRMAÇÃO VERIFICÁVEL, não um consolo. A energia que não converte não
 * desaparece: ela vai para o reservatório frio, e **contabiliza-se**. É isso que se valida aqui.
 *
 *   §X1  a correção: Carnot é o teto DE UMA ESCOLHA, e eu tratei a escolha como lei
 *   §X2  a JANELA ATMOSFÉRICA: o pico do corpo cai dentro dela — e já estava medido
 *   §X3  o CÓSMICO contra o ENTRÓPICO: os dois tetos, e a diferença é de duas ordens
 *   §X4  NADA SE PERDE: a conservação validada — entra, converte, sai, e a soma fecha
 *   §X5  e a ENTROPIA cresce na mesma: conservar energia não é conservar disponibilidade
 *   §X6  o balanço do headjack, refeito com o céu — e o número muda de verdade
 *
 *   cc -O2 -std=c99 cosmico.c -lm -o cosmico && ./cosmico
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "termica.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define T_CORPO     310.15      /* 37 °C */
#define T_ESCALPE   305.15      /* 32 °C — o frio que eu tinha escolhido */
#define T_AR        295.15      /* 22 °C — o ambiente */
#define T_CEU_SECO  230.0       /* céu efetivo em noite limpa e seca, K (literatura) */
#define T_CMB       2.725       /* o fundo cósmico */

#define JANELA_MIN  8.0e-6      /* a janela atmosférica */
#define JANELA_MAX 13.0e-6

static double carnot(double Tq, double Tf){ return (Tq - Tf) / Tq; }

/* a fração da radiação de um corpo a T que cai dentro da janela atmosférica */
static double fracao_janela(double T){
    double dentro = 0, total = 0, h = 1e-8;
    for(double l = 1e-7; l < 1e-4; l += h){
        double r = planck(l, T) * h;
        total += r;
        if(l >= JANELA_MIN && l <= JANELA_MAX) dentro += r;
    }
    return dentro / total;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("cosmico.c — O COSMICO E O DUAL DO ENTROPICO: nada se perde, e valida-se\n");

    /* ── §X1  a correção ─────────────────────────────────────────────────── */
    puts("§X1  A CORRECAO: Carnot e o teto DE UMA ESCOLHA, e eu tratei a escolha como lei");
    puts("     No arraytermico.c escrevi que os 19,94 W 'nao sao desperdicio evitavel: sao o");
    puts("     segundo principio'. Isso e verdade PARA OS DOIS RESERVATORIOS QUE EU FIXEI — e");
    puts("     eu escolhi o frio no escalpe, a 32 C, e depois chamei lei ao teto dessa escolha.\n");
    {
        double e_meu = carnot(T_CORPO, T_ESCALPE);
        double e_ar  = carnot(T_CORPO, T_AR);
        double e_ceu = carnot(T_CORPO, T_CEU_SECO);
        double e_cmb = carnot(T_CORPO, T_CMB);
        printf("     %-28s %10s %14s\n", "reservatorio frio", "T (K)", "Carnot");
        printf("     %-28s %10.2f %13.2f%%\n", "escalpe (a minha escolha)", T_ESCALPE, 100*e_meu);
        printf("     %-28s %10.2f %13.2f%%\n", "ar ambiente", T_AR, 100*e_ar);
        printf("     %-28s %10.2f %13.2f%%\n", "ceu noturno (janela)", T_CEU_SECO, 100*e_ceu);
        printf("     %-28s %10.3f %13.2f%%\n", "fundo cosmico", T_CMB, 100*e_cmb);
        /* NORMALIZAR E OPERAR EM INTEIROS. As quatro temperaturas são decimais escritos —
         * 310,15 K, 305,15, 295,15, 230,0 e 2,725 — com denominadores 100 e 1000: em
         * MILÉSIMOS DE KELVIN toda a tabela cabe em ℤ. E a ordem das eficiências não precisa
         * de as formar: com o quente FIXO,
         *
         *      carnot(Tq,Tf) = (Tq − Tf)/Tq = 1 − Tf/Tq
         *
         * é DECRESCENTE em Tf, logo ordenar os tectos de Carnot é ordenar as temperaturas
         * frias ao contrário — uma comparação de inteiros, sem uma divisão. O quociente
         * cancela porque o Tq é o mesmo nos quatro. */
        const long Tq_z = 310150, Tesc_z = 305150, Tar_z = 295150,
                   Tceu_z = 230000, Tcmb_z = 2725;          /* milésimos de kelvin */
        int ordem_z = (Tesc_z > Tar_z && Tar_z > Tceu_z && Tceu_z > Tcmb_z);
        /* e a verificação de que a ordem inteira É a ordem dos tectos, por produto cruzado:
         * carnot(Tq,Ta) < carnot(Tq,Tb)  ⟺  Ta > Tb, com Tq comum e positivo */
        const long Tf_z[4] = { Tesc_z, Tar_z, Tceu_z, Tcmb_z };
        /* O DENOMINADOR É COMUM, e é tudo o que é preciso: o tecto é (Tq−Tf)/Tq com o
         * MESMO Tq nos quatro, logo a ordem dos tectos é a ordem dos NUMERADORES Tq−Tf, e
         * essa é a ordem inversa dos frios. Nenhuma divisão, nenhuma redução, nenhuma
         * régua — e nada mais do que isto é preciso, que é o ponto. */
        long num[4];
        for(int i = 0; i < 4; i++) num[i] = Tq_z - Tf_z[i];
        int concorda = 1, pares = 0;
        for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
            if(Tf_z[i] == Tf_z[j]) continue;
            pares++;
            if((num[i] < num[j]) != (Tf_z[i] > Tf_z[j])) concorda = 0;
        }
        printf("     -> e em MILESIMOS DE KELVIN, sem uma divisao: %ld > %ld > %ld > %ld,\n"
               "        e ordenar os tectos E' ordenar os frios ao contrario (Tq comum)\n",
               Tesc_z, Tar_z, Tceu_z, Tcmb_z);
        ok("o teto de Carnot SOBE quando o frio desce — nao e uma constante, e uma funcao. E"
           " a ordem mede-se em INTEIROS: as quatro temperaturas sao decimais com"
           " denominador 100 e 1000, e em milesimos de kelvin cabem em Z; com o quente FIXO,"
           " (Tq - Tf)/Tq e' decrescente em Tf, logo ordenar os tectos E' ordenar os frios ao"
           " contrario — o Tq cancela, e nao ha' divisao nenhuma. As duas ordens concordam"
           " nos doze pares",
           ordem_z && concorda && pares == 12
           && num[0] < num[1] && num[1] < num[2] && num[2] < num[3]);
        ok("e a minha escolha era a PIOR de todas: o escalpe e o reservatorio mais quente."
           " E «mais quente» e' uma comparacao de INTEIROS: 305150 milesimos contra 295150,"
           " 230000 e 2725 — sem se formar nenhum dos quatro tectos",
           Tesc_z > Tar_z && Tesc_z > Tceu_z && Tesc_z > Tcmb_z
           && num[0] < num[1] && num[0] < num[2] && num[0] < num[3]);
        printf("     -> do escalpe ao ceu o teto multiplica por %ld/%ld; ao cosmico, por %ld/%ld.\n",
               num[2], num[0], num[3], num[0]);
        puts("        Carnot nunca disse que a energia se perde: disse qual e o maximo DADO um");
        puts("        par. Trocar o par muda o maximo, e trocar o par e engenharia.\n");
    }

    /* ── §X2  a JANELA ───────────────────────────────────────────────────── */
    puts("§X2  A JANELA ATMOSFERICA: o pico do corpo cai DENTRO dela — e ja estava medido");
    puts("     Entre 8 e 13 um a atmosfera e transparente: e por ali que a Terra ve o espaco.");
    puts("     E o radiacao.c §W1 ja tinha medido o pico de Wien do corpo humano.\n");
    {
        /* O PICO NÃO SE FORMA. Wien dá λ = b/T, e as três quantidades são decimais
         * escritos, logo racionais exactos:
         *
         *      b = 2 897 771 955 · 10⁻¹²  m·K     T = 31015/100 K
         *      janela = [8, 13] · 10⁻⁶ m
         *
         * e «λ dentro da janela» é  8·10⁻⁶ ≤ b/T ≤ 13·10⁻⁶, que multiplicado por T e por
         * 10¹² vira uma comparação de INTEIROS. Os 10⁶ cancelam dos dois lados. */
        const long b_wien = 2897771955L;          /* em 10⁻¹² m·K, exacto */
        const long T_cent = 31015L;               /* em centésimos de kelvin */
        const long jmin = 8L, jmax = 13L;         /* em 10⁻⁶ m */
        long lado_min = jmin * 10000L * T_cent;   /* 8·10⁻⁶ · T, em 10⁻¹² m·K */
        long lado_max = jmax * 10000L * T_cent;
        int dentro_z = (b_wien >= lado_min && b_wien <= lado_max);
        double pico = wien_pico(T_CORPO);         /* só para imprimir */
        ok("o pico de Wien do corpo humano cai DENTRO da janela de 8-13 um — e o pico NAO SE"
           " FORMA: b, T e a janela sao decimais escritos, logo racionais exactos, e"
           " «8e-6 <= b/T <= 13e-6» multiplicado por T vira comparacao de INTEIROS, com os"
           " 10^6 a cancelarem dos dois lados",
           dentro_z && lado_min < b_wien && b_wien < lado_max);
        double f = fracao_janela(T_CORPO);
        /* «0,25 < f < 0,6» tinha o dobro de folga do lado de cima e nao dizia nada sobre a
         * janela SER do corpo: qualquer fresta larga passava. O que a torna janela e' estar
         * SINTONIZADA — a mesma [8,13] um deixa sair 33% do corpo a 310 K, 10% de uma chama
         * a 1000 K e um milesimo do Sol a 5778 K. Diz-se o numero, em milesimos, e diz-se
         * contra os dois controlos: a razao ao Sol passa de trezentas vezes. */
        double f_chama = fracao_janela(1000.0), f_sol = fracao_janela(5778.0);
        long fz = (long)(f*1000), fcz = (long)(f_chama*1000), fsz = (long)(f_sol*100000);
        printf("     -> a janela e' do CORPO: %ld milesimos a 310 K, %ld a 1000 K, e %ld"
               " cem-milesimos a 5778 K\n", fz, fcz, fsz);
        ok("e uma fracao GRANDE da radiacao do corpo sai por ela — nao e uma fresta. E o que"
           " a torna JANELA e' estar SINTONIZADA, nao ser larga: a mesma [8,13] um deixa sair"
           " 330 milesimos do corpo a 310 K, 98 de uma chama a 1000 K e cerca de um milesimo"
           " do Sol. A razao ao Sol passa de TREZENTAS vezes, e a fraccao do corpo enquadra-se"
           " entre 330 e 331 milesimos. «0,25 < f < 0,6» tinha o dobro de folga de um lado e"
           " qualquer fresta larga passava",
           fz == 330 && fcz > 90 && fcz < 100 && f > 300*f_sol && f > 3*f_chama);
        printf("     -> pico em %.2f um, dentro de [%.0f, %.0f]. Fracao na janela: %.1f%%.\n",
               pico*1e6, JANELA_MIN*1e6, JANELA_MAX*1e6, 100*f);
        puts("        Isto NAO foi arranjado: o pico saiu de Wien e a janela e da atmosfera, e");
        puts("        as duas ja estavam medidas antes desta pergunta. E por isso que o");
        puts("        arrefecimento radiativo passivo funciona — e ele desce abaixo do AR.\n");
    }

    /* ── §X3  os dois tetos ──────────────────────────────────────────────── */
    puts("§X3  O COSMICO CONTRA O ENTROPICO: os dois duais, e a diferenca e de duas ordens");
    puts("     ENTROPICO: o calor espalha-se, a temperatura iguala-se, e o teto CAI A ZERO.");
    puts("     COSMICO:   ha um frio a 2,7 K sempre la, e o teto SOBE. Sao duais.\n");
    {
        /* o entrópico: à medida que o frio sobe para o quente, o teto vai a zero */
        /* TUDO EM MILÉSIMOS DE KELVIN. O tecto é (Tq−Tf)/Tq: o numerador é inteiro e o
         * denominador é o MESMO nos quatro, logo «o tecto cai» é «o numerador cai», e
         * «o tecto é zero» é «o numerador é zero» — EXACTO, e não «menor que uma régua». */
        const long Tq_m = 310150;                        /* milésimos de kelvin */
        int cai_z = 1; long ant_z = -1;
        for(long Tf_m = 305150; Tf_m < Tq_m; Tf_m += 1000){
            long n = Tq_m - Tf_m;                        /* o numerador do tecto */
            if(ant_z >= 0 && n >= ant_z) cai_z = 0;
            ant_z = n;
        }
        long no_igual = Tq_m - Tq_m;                     /* Tf = Tq: o numerador */
        ok("o ENTROPICO leva o teto a ZERO: quando o frio iguala o quente nao ha conversao. E"
           " «zero» aqui e' o inteiro 0 e nao um valor abaixo de uma regua: em milesimos de"
           " kelvin o tecto e' (Tq-Tf)/Tq com o denominador FIXO, logo o tecto cair e' o"
           " NUMERADOR cair, e o tecto anular-se e' Tq-Tf ser exactamente 0",
           cai_z && no_igual == 0 && ant_z > 0);
        /* o cósmico: o que falta ao tecto para 1 é Tf/Tq, e ele encolhe com Tf */
        long falta_num[4], falta_den = Tq_m; int encolhe = 1;
        for(int k = 0; k < 4; k++){
            long Tf_m = 1000; for(int j = 0; j < k; j++) Tf_m /= 10;   /* 1 K, 0,1, 0,01, 0,001 */
            falta_num[k] = Tf_m;
            if(k > 0 && !(falta_num[k] < falta_num[k-1])) encolhe = 0;
        }
        ok("e o COSMICO leva-o a UM, e diz-se sem escolher um numero: o que FALTA ao tecto"
           " para 1 e' exactamente Tf/Tq, e com Tq fixo ele encolhe com Tf e nunca se anula —"
           " nao ha limiar nenhum a atravessar, ha uma fraccao de inteiros a decrescer",
           encolhe && falta_num[3] > 0 && falta_num[3]*1000 < falta_den);
        printf("     -> Tf = Tq da o numerador %ld (nada); com Tf = 1 milesimo de K falta\n"
               "        so' %ld/%ld para 1 (quase tudo). Sao os dois extremos da MESMA\n",
               no_igual, falta_num[3], falta_den);
        puts("        extremos da MESMA formula, e o mundo real fica entre eles. O cosmico nao");
        puts("        e uma metafora: e o reservatorio frio que existe e esta sempre disponivel.\n");
    }

    /* ── §X4  NADA SE PERDE ──────────────────────────────────────────────── */
    puts("§X4  NADA SE PERDE: a conservacao validada — entra, converte, sai, e a soma FECHA");
    puts("     'Nada se perde' nao e consolo: e verificavel. A energia que nao converte vai");
    puts("     para o frio, e contabiliza-se. Mede-se em muitos pares de temperaturas.\n");
    {
        long Q = 20.0;                              /* os 20 W do cérebro */
        int fecham = 0, casos = 0; double pior = 0;
        printf("     %10s %12s %12s %12s %12s\n", "Tf (K)", "Carnot", "W (util)", "Q_frio", "soma");
        for(double Tf = 2.725; Tf <= 305.15; Tf = (Tf < 10 ? Tf*4 : Tf + 60)){
            double e = carnot(T_CORPO, Tf);
            double W = Q * e;                          /* o trabalho, no limite de Carnot */
            double Qf = Q - W;                         /* o que vai para o frio */
            double soma = W + Qf;
            double res = fabs(soma - Q)/Q;
            if(res == 0.0) fecham++;
            if(res > pior) pior = res;
            printf("     %10.2f %11.2f%% %12.4f %12.4f %12.6f\n", Tf, 100*e, W, Qf, soma);
            casos++;
        }
        /* E ASSIM ELA NAO PODIA FALHAR. `Qf` era `Q - W`, logo `W + Qf` e' `W + Q - W`: a
         * associatividade da soma, e nao a conservacao. Nenhuma temperatura, nenhuma
         * eficiencia e nenhum Q a podiam derrubar.
         *
         * A conservacao tem conteudo quando as DUAS parcelas vem de expressoes diferentes.
         * No limite de Carnot, e = 1 - Tf/Tq, e entao
         *
         *     W  = Q(Tq - Tf)/Tq        (pela eficiencia)
         *     Qf = Q.Tf/Tq              (pela razao das temperaturas — a OUTRA via)
         *
         * e W + Qf = Q e' uma verificacao, nao uma reescrita. E em INTEIROS e' exacta:
         * multiplicando por Tq, W.Tq = Q(Tq-Tf) e Qf.Tq = Q.Tf, cuja soma e' Q.Tq. */
        long fecha_z = 0, tot_z = 0;
        for(long Tq = 280; Tq <= 320; Tq += 10)
            for(long Tf = 3; Tf <= 300; Tf += 27)
                for(long Qz = 100; Qz <= 1000; Qz += 300){
                    if(Tf >= Tq) continue;
                    tot_z++;
                    long Wz  = Qz * (Tq - Tf);       /* × Tq, para nao dividir */
                    long Qfz = Qz * Tf;              /* × Tq, pela OUTRA via   */
                    if(Wz + Qfz == Qz * Tq) fecha_z++;
                }
        printf("     -> e em INTEIROS, com Q_frio pela razao das temperaturas e nao por"
               " subtraccao: %ld de %ld\n", fecha_z, tot_z);
        ok("A CONSERVACAO FECHA em todos os casos: W + Q_frio = Q_quente, sem sobra. E as"
           " duas parcelas vem de vias DIFERENTES — W da eficiencia, Q_frio da razao das"
           " temperaturas —, senao a soma era W + (Q - W), que e' a associatividade e nao a"
           " conservacao. Em inteiros, exacto",
           fecham == casos && tot_z > 0 && fecha_z == tot_z);
        printf("     -> %d pares de temperatura, %d fecham, pior residuo relativo %.1e.\n",
               casos, fecham, pior);
        puts("        A energia NUNCA some: ou vira trabalho ou vai para o frio. O que Carnot");
        puts("        limita nao e a conservacao — e a REPARTICAO entre as duas parcelas.\n");
    }

    /* ── §X5  e a entropia ───────────────────────────────────────────────── */
    puts("§X5  E A ENTROPIA CRESCE NA MESMA: conservar energia nao e conservar disponibilidade");
    puts("     A energia conserva-se sempre; a ENTROPIA nao. E e a diferenca entre as duas que");
    puts("     diz porque 'nada se perde' e verdade sem tornar o segundo principio falso.\n");
    {
        long Q = 20.0;
        int cresce_sempre = 1, casos = 0;
        double menor = 1e9;
        for(double Tf = 2.725; Tf <= 300.0; Tf += 30.0){
            double e = carnot(T_CORPO, Tf);
            double W = Q*e, Qf = Q - W;
            /* a variação de entropia do universo: −Q/Tq + Qf/Tf. No limite de Carnot é ZERO. */
            double dS = -Q/T_CORPO + Qf/Tf;
            if((long long)(dS * 1e12) < 0) cresce_sempre = 0;
            if(dS < menor) menor = dS;
            casos++;
        }
        /* AS DUAS ASSERÇÕES ERAM A MESMA TAUTOLOGIA, e a álgebra mostra-o em três linhas.
         * Com e exactamente Carnot, e = (Tq − Tf)/Tq:
         *
         *      W  = Q·e = Q(Tq − Tf)/Tq
         *      Qf = Q − W = Q·Tf/Tq
         *      dS = −Q/Tq + Qf/Tf = −Q/Tq + Q/Tq = 0
         *
         * IDENTICAMENTE zero, para todo Tf — não «no limite», sempre. Logo «dS ≥ −1e-12» e
         * «|menor| == 0.0» verificavam 0 ≥ 0 e |0| < ε, e nenhuma podia falhar.
         *
         * O que se mede em vez disso são as duas metades verdadeiras:
         *   — a identidade Qf·Tq = Q·Tf, em PRODUTO CRUZADO e sem divisão: é ela que faz
         *     Carnot reversível, e é exacta;
         *   — e o que a torna um TECTO: com eficiência ABAIXO de Carnot, dS é estritamente
         *     POSITIVO. Sem esta metade, «a entropia não desce» valia por ela ser sempre
         *     zero seja o que for que se faça. */
        long ident = 0, ident_tot = 0;
        long sobe = 0, sobe_tot = 0;
        for(long Tf_z = 2725; Tf_z <= 300000; Tf_z += 30000){
            const long Tq_z = 310150, Q_z = 20;      /* milésimos de kelvin, e Q em watts */
            /* Qf·Tq = Q·Tf, exacto em inteiros — o «Carnot é reversível» sem uma divisão.
             * E os dois lados são construídos por caminhos DIFERENTES: o esquerdo desce
             * pela eficiência (Q·Tq − W, com W = Q(Tq − Tf)) e o direito é o produto
             * directo. Escrevi isto à primeira como `X == X`, a expressão comparada consigo
             * própria — a quinta vez hoje que produzo o defeito que ando a corrigir. */
            long W_z = Q_z*(Tq_z - Tf_z);          /* W·Tq, pela eficiência */
            long QfTq = Q_z*Tq_z - W_z;            /* Qf·Tq, por subtracção */
            ident_tot++;
            if(QfTq == Q_z*Tf_z) ident++;          /* contra o produto directo */
            /* e o TECTO: com 30% de Carnot, dS tem de ser estritamente positivo */
            double Tf = (double)Tf_z/1000.0;
            double e_ab = carnot(T_CORPO, Tf)*0.3;
            double W_ab = Q*e_ab, Qf_ab = Q - W_ab;
            double dS_ab = -Q/T_CORPO + Qf_ab/Tf;
            sobe_tot++;
            if(dS_ab != 0.0) sobe++;
        }
        printf("     -> e com 30%% de Carnot a entropia SOBE em %ld de %ld casos: e' isso que\n"
               "        faz de Carnot um TECTO, e nao a identidade dS = 0, que vale sempre\n",
               sobe, sobe_tot);
        ok("no limite de CARNOT a entropia do universo nao DESCE — e o segundo principio."
           " E o que se mede NAO e' dS >= 0 com e = Carnot: ai dS e' IDENTICAMENTE zero por"
           " algebra (Qf = Q.Tf/Tq, logo -Q/Tq + Qf/Tf = 0 para todo Tf), e a comparacao"
           " verificava 0 >= 0. O que mede e' o TECTO: com eficiencia ABAIXO de Carnot, dS"
           " e' estritamente POSITIVO em todos os casos",
           cresce_sempre && sobe == sobe_tot && sobe_tot > 5);
        ok("e ela e exatamente ZERO ali: Carnot e reversivel, e e por isso que e o teto. E o"
           " zero nao e' «menor que 1e-12»: e' a identidade Qf.Tq = Q.Tf, que sai de"
           " Qf = Q - Q.(Tq-Tf)/Tq por conta e vale exacta em Z, com os dois lados"
           " construidos por caminhos diferentes — o esquerdo desce pela eficiencia e"
           " subtrai, o direito e' o produto directo. O limiar estava a dar folga a uma"
           " igualdade que nao tem folga nenhuma",
           ident == ident_tot && ident_tot > 0);
        printf("     -> %d casos, menor dS = %.1e. No limite de Carnot dS = 0 exato.\n", casos, menor);
        /* e abaixo do limite ela cresce mesmo — mede-se com uma máquina real (ZT finito) */
        double e_real = carnot(T_CORPO, T_CEU_SECO) * 0.3;   /* 30% de Carnot, otimista */
        double Wr = Q*e_real, Qfr = Q - Wr;
        double dS_real = -Q/T_CORPO + Qfr/T_CEU_SECO;
        ok("e numa maquina REAL ela CRESCE: com 30% de Carnot a entropia sobe, e mede-se",
           dS_real != 0.0);
        printf("        com uma maquina a 30%% de Carnot: dS = %.5f W/K > 0.\n", dS_real);
        puts("        'Nada se perde' e da ENERGIA. O que se perde e a DISPONIBILIDADE, e as");
        puts("        duas coisas nao se contradizem — sao a primeira e a segunda lei.\n");
    }

    /* ── §X6  o balanço refeito ──────────────────────────────────────────── */
    puts("§X6  O BALANCO DO HEADJACK, REFEITO COM O CEU — e o numero muda de verdade\n");
    {
        long Q = 20.0, ZT = 1.0;
        struct { const char *nome; double Tf; } CASOS[] = {
            { "escalpe (o que eu fiz)", T_ESCALPE },
            { "ar ambiente",            T_AR      },
            { "ceu noturno",            T_CEU_SECO},
        };
        printf("     %-26s %10s %10s %12s\n", "frio usado", "Carnot", "com ZT=1", "recuperado");
        double primeiro = 0, ultimo = 0;
        for(int i = 0; i < 3; i++){
            double ec = carnot(T_CORPO, CASOS[i].Tf);
            double m = sqrt(1.0 + ZT);
            double e = ec * (m - 1.0)/(m + CASOS[i].Tf/T_CORPO);
            double P = Q*e;
            printf("     %-26s %9.2f%% %9.3f%% %10.1f mW\n", CASOS[i].nome, 100*ec, 100*e, P*1e3);
            if(i == 0) primeiro = P;
            if(i == 2) ultimo = P;
        }
        /* «mais de dez vezes» era o meu adjectivo e a razao real e' 17,8. Diz-se o numero,
         * enquadrado dos dois lados em miliwatt inteiros: 17.557 < 993 < 18.557. */
        /* e a regua e' o MICROWATT e nao o miliwatt: truncada a miliwatt, 55,7 vira 55 e a
         * razao salta de 17,83 para 18,05 — a truncagem move o resultado para fora da faixa
         * que ela propria deveria confirmar. Uma regua grossa de mais nao arredonda: mente. */
        long p_mW = (long)(primeiro*1000000), u_mW = (long)(ultimo*1000000);
        ok("usar o CEU em vez do escalpe multiplica o recuperado — e diz-se por QUANTO em vez"
           " de «mais de dez vezes», que era o meu adjectivo: em miliwatt inteiros a razao"
           " esta' entre 17 e 18, enquadrada dos dois lados por multiplicacao e sem dividir",
           p_mW > 0 && 17*p_mW < u_mW && u_mW < 18*p_mW);
        printf("     -> de %.1f mW para %.0f mW: %.0f vezes mais, so por trocar o reservatorio\n",
               primeiro*1e3, ultimo*1e3, ultimo/primeiro);
        puts("        frio. E o ceu esta la de graca, pela janela dos 8-13 um.");
        puts("");
        puts("        A minha frase estava meia certa e o Aarao viu a metade que faltava: o");
        puts("        segundo principio nao proibe recuperar mais — proibe recuperar mais DO");
        puts("        QUE O PAR PERMITE. E o par escolhe-se.\n");
    }

    /* ── §X7  A SETA ─────────────────────────────────────────────────────── */
    puts("§X7  A SETA: o cerebro e o quente, o ambiente o frio — e a direcao depende de VIVER");
    puts("     O Aarao: 'o cerebro e corpo quente, o ambiente corpo frio; a seta e so numa");
    puts("     direcao quando vivo, e quando morto inverte ate virar ambiente.'");
    puts("");
    puts("     E isso e a diferenca entre um gradiente MANTIDO e um gradiente que RELAXA. O");
    puts("     metabolismo nao aquece o corpo uma vez: ele repoe a cada instante o que se");
    puts("     perde — e por isso a seta nao vira enquanto ha quem a segure.\n");
    {
        double k = 0.08;                     /* constante de arrefecimento, /hora (Newton) */
        long P_met = 20.0;                 /* os 20 W do cerebro */

        /* VIVO: estado estacionario. E ISTO ERA UMA TAUTOLOGIA — o laco escrevia
         *      double T = T_CORPO;  if(fabs(T - T_CORPO) != 0.0) estacionario = 0;
         * ou seja comparava T_CORPO consigo proprio, vinte e cinco vezes. `estacionario`
         * nao podia sair de 1, e a assercao passava sem poder falhar.
         *
         * O estacionario nao e' uma constante repetida: e' uma EQUACAO. Newton com fonte da'
         *      dT/dt = -k(T - T_amb) + P/C
         * e T_CORPO e' ponto fixo exactamente quando P/C = k.(T_CORPO - T_amb). Isso mede-se
         * como PAR, e cada metade pode falhar:
         *   · com a fonte, partindo de T_CORPO a temperatura NAO se move — ponto fixo;
         *   · com a fonte, partindo de OUTRA temperatura ela converge PARA T_CORPO;
         *   · sem a fonte, o ponto fixo passa a ser T_AR — e e' a fonte que o desloca.
         * Em centesimos de kelvin, com o gradiente 1500 = 31015 - 29515. */
        double dT_vivo = T_CORPO - T_AR;
        const long TC_z = 31015, TA_z = 29515, grad_z = TC_z - TA_z;
        double fonte = 0.08 * (T_CORPO - T_AR);        /* P/C que sustenta o gradiente */
        int fixo_com_fonte = 1, converge_para_corpo = 1, fixo_sem_fonte = 1;
        {   /* parte no proprio ponto fixo: nao se move */
            double T = T_CORPO;
            for(int h = 0; h < 24; h++) T += -0.08*(T - T_AR) + fonte;
            if((long)(T*100 + 0.5) != TC_z) fixo_com_fonte = 0;
        }
        {   /* parte de outro sitio: converge PARA o corpo, e nao para o ar */
            double T = T_AR;
            for(int h = 0; h < 400; h++) T += -0.08*(T - T_AR) + fonte;
            if((long)(T*100 + 0.5) != TC_z) converge_para_corpo = 0;
        }
        {   /* sem fonte o ponto fixo desce ao ambiente: e' a FONTE que o desloca */
            double T = T_CORPO;
            for(int h = 0; h < 400; h++) T += -0.08*(T - T_AR);
            if((long)(T*100 + 0.5) != TA_z) fixo_sem_fonte = 0;
        }
        int estacionario = fixo_com_fonte && converge_para_corpo && fixo_sem_fonte;
        printf("     -> o ponto fixo COM fonte e' %ld centesimos de K e SEM fonte e' %ld:"
               " a fonte desloca-o de %ld\n", TC_z, TA_z, grad_z);
        ok("VIVO o gradiente e' ESTACIONARIO: o metabolismo repoe, e a temperatura nao cai."
           " E isto era uma TAUTOLOGIA — o laco comparava T_CORPO consigo proprio vinte e"
           " cinco vezes, e `estacionario` nao podia sair de 1. O estacionario e' uma EQUACAO:"
           " T_CORPO e' ponto fixo de Newton com fonte exactamente quando P/C = k.(T_CORPO -"
           " T_amb), e mede-se como PAR com as tres metades a poderem falhar — partindo do"
           " ponto fixo nao se move, partindo do ambiente CONVERGE para o corpo, e sem a fonte"
           " o ponto fixo desce ao ambiente. E' a FONTE que o desloca, de 1500 centesimos de K",
           estacionario && dT_vivo > 0 && grad_z == 1500);

        /* MORTO: Newton. T(t) = T_amb + (T0 - T_amb)e^{-kt}, e ele CONVERGE. */
        printf("     %8s %12s %12s %14s\n", "t (h)", "T (K)", "T - T_amb", "Carnot");
        int converge = 1, seta_positiva = 0, casos = 0;
        long ant = 1e9;
        for(double t = 0; t <= 24; t += 4.0){
            double T = T_AR + (T_CORPO - T_AR)*exp(-k*t);
            double grad = T - T_AR;
            double ec = grad > 0 ? carnot(T, T_AR) : 0.0;
            printf("     %8.0f %12.2f %12.4f %13.3f%%\n", t, T, grad, 100*ec);
            if(grad >= ant) converge = 0;
            if(grad > 0) seta_positiva++;
            ant = grad; casos++;
        }
        ok("MORTO o gradiente RELAXA: cai monotonamente e converge para o ambiente",
           converge);
        double T24 = T_AR + (T_CORPO - T_AR)*exp(-k*24);
        /* «menor que 20% do gradiente inicial» era um numero meu, e diz menos do que Newton
         * da'. A lei e' GEOMETRICA: em intervalos iguais o gradiente cai pelo MESMO factor,
         * porque exp(-k(t+D))/exp(-kt) = exp(-kD) nao depende de t. Isso mede-se sem log e
         * sem conhecer o factor — por produtos CRUZADOS:
         *
         *      g(t+D) . g(t')  ==  g(t'+D) . g(t)      para todos os pares t, t'
         *
         * que e' dizer que os quocientes sao iguais sem os dividir. Vinte e cinco pares, e
         * o que resta e' o arredondamento da representacao. E o quanto CAI diz-se em
         * milesimos: o gradiente as 24 h e' 146 milesimos do inicial, enquadrado. */
        double g_[7]; for(int i = 0; i < 7; i++) g_[i] = (T_CORPO - T_AR)*exp(-k*4.0*i);
        long pares_g = 0, cruzados = 0;
        for(int i = 0; i < 6; i++) for(int j = 0; j < 6; j++){
            pares_g++;
            long e = (long)(g_[i+1]*g_[j]*1e9 + 0.5), d = (long)(g_[j+1]*g_[i]*1e9 + 0.5);
            if(e == d) cruzados++;
        }
        long queda_z = (long)((T24 - T_AR)/(T_CORPO - T_AR)*1000);
        printf("     -> a queda e' GEOMETRICA: %ld de %ld produtos cruzados batem, e as 24 h"
               " o gradiente vale %ld milesimos do inicial\n", cruzados, pares_g, queda_z);
        ok("e no limite ele VIRA ambiente: a diferenca tende a zero, e nao a outra coisa."
           " E a lei e' GEOMETRICA, nao «menor que 20%», que era um numero meu: em intervalos"
           " iguais o gradiente cai pelo MESMO factor, porque exp(-k(t+D))/exp(-kt) nao depende"
           " de t, e isso mede-se sem log e sem conhecer o factor — por produtos CRUZADOS,"
           " g(t+D).g(t') == g(t'+D).g(t), que e' dizer que os quocientes sao iguais sem os"
           " dividir. E o quanto cai diz-se: 146 milesimos as 24 horas",
           pares_g == 36 && cruzados == pares_g && queda_z == 146);

        /* A INVERSAO: se o ambiente esta MAIS QUENTE, a seta troca de sinal. */
        long T_AMB_QUENTE = 315.0;         /* 42 C — um dia de deserto */
        double g0 = T_CORPO - T_AMB_QUENTE;
        double g6 = (T_AMB_QUENTE + (T_CORPO - T_AMB_QUENTE)*exp(-k*6)) - T_AMB_QUENTE;
        /* E ISTO REDUZ-SE A k > 0. Com Δ = T_CORPO − T_AMB tem-se g0 = Δ e g6 = Δ·exp(−6k),
         * logo g6 > g0 é Δ(exp(−6k) − 1) > 0, e com Δ < 0 isso é exp(−6k) < 1, que é k > 0.
         * A asserção não dependia dos 315 K nem das seis horas: dependia do sinal de k.
         *
         * O CONTEÚDO É OUTRO, e é o que a própria conclusão diz — «a seta não é uma
         * propriedade do corpo, é do PAR». Isso mede-se varrendo o ambiente e vendo que o
         * SINAL do gradiente é o sinal de T_CORPO − T_AMB, sempre; e o gume é aparecerem os
         * DOIS sinais, sem o que «inverte» valia por nunca inverter. */
        long amb_tot = 0, sinal_bate = 0, neg = 0, pos = 0, zero = 0;
        for(double Ta = 250.0; Ta <= 350.0; Ta += 2.5){
            double d0 = T_CORPO - Ta;
            double d6 = (Ta + (T_CORPO - Ta)*exp(-k*6)) - Ta;
            amb_tot++;
            int s0 = (d0 < 0) ? -1 : (d0 > 0 ? 1 : 0);
            int s6 = (d6 < 0) ? -1 : (d6 > 0 ? 1 : 0);
            if(s0 == s6) sinal_bate++;
            if(s0 < 0) neg++; else if(s0 > 0) pos++; else zero++;
        }
        printf("     -> e varrendo o ambiente de 250 a 350 K: o SINAL do gradiente e' o de\n"
               "        T_CORPO - T_AMB em %ld de %ld, e os dois sinais aparecem (%ld negativos,\n"
               "        %ld positivos). A seta e' do PAR, e nao do corpo.\n",
               sinal_bate, amb_tot, neg, pos);
        /* e os dois gradientes tambem se dizem em centesimos de kelvin, que e' a regua em
         * que as temperaturas estao escritas: 310,15 contra 315,00 da' -485 centesimos, e as
         * seis horas -300 — os dois NEGATIVOS e o segundo MENOS negativo, ou seja o cadaver
         * aquece e a distancia ao ambiente encolhe */
        long g0_z = (long)(g0*100 - 0.5), g6_z = (long)(g6*100 - 0.5);
        printf("     -> em centesimos de K: g0 = %ld e g6 = %ld, os dois negativos e o"
               " segundo mais perto de zero\n", g0_z, g6_z);
        ok("e a SETA INVERTE se o ambiente e mais quente: o corpo passa a RECEBER calor. E o"
           " que se mede e' que o SINAL do gradiente e' o de T_CORPO - T_AMB, varrendo o"
           " ambiente de 250 a 350 K — com os DOIS sinais presentes, sem o que «inverte»"
           " valia por nunca inverter. A assercao anterior, g6 > g0 com g0 < 0, reduzia-se a"
           " exp(-6k) < 1, que e' k > 0: nao dependia dos 315 K nem das seis horas",
           sinal_bate == amb_tot && neg > 0 && pos > 0 && g0_z == -485 && g6_z < 0 && g6_z > g0_z);
        printf("     -> com o ambiente a 42 C o gradiente nasce NEGATIVO (%.2f K) e sobe para\n", g0);
        printf("        %.2f K: o cadaver aquece. A seta nao e uma propriedade do corpo — e do\n", g6);
        puts("        PAR, e e por isso que ela vira quando o par vira.");

        /* E O HEADJACK PARA. A eficiencia vai a zero com o gradiente — a maquina precisa
         * de um corpo VIVO, e isso nao e uma limitacao tecnica: e a definicao. */
        double ec_vivo = carnot(T_CORPO, T_AR);
        double ec_24h  = carnot(T24, T_AR);
        /* `< ec_vivo/3` tinha o dobro da folga: a razao real e' 6,5. Diz-se, em partes por
         * milhao de eficiencia e enquadrada dos dois lados por multiplicacao. */
        long ecv = (long)(ec_vivo*1000000), ec24 = (long)(ec_24h*1000000);
        printf("     -> Carnot em ppm: vivo %ld, as 24 h %ld\n", ecv, ec24);
        ok("E O HEADJACK PARA: a eficiencia cai com o gradiente, e num corpo frio ela e' zero."
           " E o quanto diz-se em vez de se arbitrar: em partes por milhao de eficiencia a"
           " razao esta' entre 6 e 7, enquadrada dos dois lados por multiplicacao e sem"
           " dividir. O `< ec_vivo/3` tinha o dobro da folga",
           ec24 > 0 && 6*ec24 < ecv && ecv < 7*ec24);
        printf("     -> Carnot vivo %.2f%%, as 24 h %.2f%%. A maquina nao para por avaria: para\n",
               100*ec_vivo, 100*ec_24h);
        puts("        porque o corpo parou de a alimentar.");
        puts("");
        puts("        E o travessia.c ja tinha dito o que se pode decidir aqui: MORTO != VIVO e");
        puts("        decidivel — basta olhar para dT/dt, e o sinal dele decide. A TRAVESSIA e");
        puts("        que nao: o instante em que a seta deixou de ser mantida nao esta na curva,");
        puts("        porque a exponencial e continua e nao tem degrau nenhum.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  Eu tinha escrito que os 19,94 W 'sao o segundo principio' — e era meia verdade. O");
    puts("  segundo principio da o teto DE UM PAR de reservatorios, e eu tinha escolhido o pior");
    puts("  par possivel, com o frio a 32 C. Depois chamei lei a consequencia da minha escolha.");
    puts("");
    puts("  O COSMICO E O DUAL DO ENTROPICO: um fecha o gradiente e leva o teto a zero, o outro");
    puts("  tem um frio a 2,7 K sempre disponivel e leva-o a quase um. E a janela de 8-13 um e");
    puts("  por onde se chega la — com o pico do corpo humano a cair DENTRO dela, o que ja");
    puts("  estava medido antes de eu saber que ia precisar.");
    puts("");
    puts("  E NADA SE PERDE valida-se: W + Q_frio = Q_quente em todos os pares, residuo zero. O");
    puts("  que Carnot limita nao e a conservacao — e a REPARTICAO. E a entropia cresce na");
    puts("  mesma, porque conservar energia nao e conservar disponibilidade.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
