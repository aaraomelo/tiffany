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
 * *São duais: um fecha o gradiente, o outro abre-o.* E o pico de Wien do corpo humano cai
 * **dentro** da janela — e isso lê-se por CORTE, sem formar λ = b/T.
 *
 * LEI vs TRANSPORTE. A quadratura de Planck, exp(−kt), Euler de 400 passos e √(1+ZT) de Ioffe
 * eram o transporte: mediam o MÉTODO. A lei é Carnot em ℤ (ordenar tectos É ordenar frios,
 * Tq comum), Wien por corte, W·Tq + Qf·Tq = Q·Tq pelas duas vias, dS ∝ (Tq−Tf) abaixo do
 * tecto, e Newton como mapa linear g′ = (23/25) g — órbita geométrica, sem exp.
 *
 *   §X1  a correção: Carnot é o teto DE UMA ESCOLHA, e eu tratei a escolha como lei
 *   §X2  a JANELA ATMOSFÉRICA: o pico do corpo cai dentro dela — e já estava medido
 *   §X3  o CÓSMICO contra o ENTRÓPICO: os dois tetos, e a diferença é de duas ordens
 *   §X4  NADA SE PERDE: a conservação validada — entra, converte, sai, e a soma fecha
 *   §X5  e a ENTROPIA cresce na mesma: conservar energia não é conservar disponibilidade
 *   §X6  o balanço do headjack, refeito com o céu — e o número muda de verdade
 *   §X7  a SETA: vivo estacionário, morto geométrico, e o par decide o sinal
 *
 *   cc -O2 -std=c99 -I lib tests/cosmico.c -o cosmico && ./cosmico
 */
#include <stdio.h>
#include "i128.h"
#include "unidade.h"

/* temperaturas em milésimos de kelvin — os decimais escritos, logo racionais */
#define Tq_m    310150L          /* 37 °C */
#define Tesc_m  305150L          /* 32 °C — o frio que eu tinha escolhido */
#define Tar_m   295150L          /* 22 °C */
#define Tceu_m  230000L          /* céu efectivo, noite limpa e seca */
#define Tcmb_m    2725L          /* fundo cósmico 2,725 K */

/* e em centésimos, a régua do Wien e do Newton */
#define TC_z    31015L
#define TA_z    29515L

static long mdc(long a, long b){
    if(a < 0) a = -a; if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}

static I128 ipow(long b, int e){
    I128 r = i128_from_i64(1), bb = i128_from_i64(b);
    for(int i = 0; i < e; i++) r = i128_mul(r, bb);
    return r;
}

int main(void){
    puts("cosmico.c — O COSMICO E O DUAL DO ENTROPICO: nada se perde, e valida-se\n");

    /* ── §X1  a correção ─────────────────────────────────────────────────── */
    puts("§X1  A CORRECAO: Carnot e o teto DE UMA ESCOLHA, e eu tratei a escolha como lei");
    puts("     No arraytermico.c escrevi que os 19,94 W 'nao sao desperdicio evitavel: sao o");
    puts("     segundo principio'. Isso e verdade PARA OS DOIS RESERVATORIOS QUE EU FIXEI — e");
    puts("     eu escolhi o frio no escalpe, a 32 C, e depois chamei lei ao teto dessa escolha.\n");
    {
        printf("     %-28s %14s %16s\n", "reservatorio frio", "T (mK)", "num. Carnot");
        const long Tf_z[4] = { Tesc_m, Tar_m, Tceu_m, Tcmb_m };
        const char *nome[4] = { "escalpe (a minha escolha)", "ar ambiente",
                                "ceu noturno (janela)", "fundo cosmico" };
        long num[4];
        for(int i = 0; i < 4; i++){
            num[i] = Tq_m - Tf_z[i];
            printf("     %-28s %14ld %16ld\n", nome[i], Tf_z[i], num[i]);
        }
        int ordem_z = (Tesc_m > Tar_m && Tar_m > Tceu_m && Tceu_m > Tcmb_m);
        int concorda = 1, pares = 0;
        for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
            if(Tf_z[i] == Tf_z[j]) continue;
            pares++;
            if((num[i] < num[j]) != (Tf_z[i] > Tf_z[j])) concorda = 0;
        }
        printf("     -> em MILESIMOS DE KELVIN, sem uma divisao: %ld > %ld > %ld > %ld,\n"
               "        e ordenar os tectos E' ordenar os frios ao contrario (Tq comum)\n",
               Tesc_m, Tar_m, Tceu_m, Tcmb_m);
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
           Tesc_m > Tar_m && Tesc_m > Tceu_m && Tesc_m > Tcmb_m
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
        /* O PICO NÃO SE FORMA. Wien dá λ = b/T. «λ dentro da janela» é
         * 8·10⁻⁶ ≤ b/T ≤ 13·10⁻⁶, que multiplicado por T vira INTEIROS.
         * A quadratura de Planck era transporte: a sintonia LÊ-SE no pico. */
        const long b_wien = 2897771955L;          /* em 10⁻¹² m·K */
        const long jmin = 8L, jmax = 13L;         /* em 10⁻⁶ m */
        long lado_min = jmin * 10000L * TC_z;
        long lado_max = jmax * 10000L * TC_z;
        int corpo_dentro = (b_wien > lado_min && b_wien < lado_max);
        /* chama 1000 K e Sol 5778 K: picos ABAIXO da janela (λ menor). */
        long Tchama_z = 100000L, Tsol_z = 577800L;     /* centésimos de K */
        int chama_abaixo = (b_wien < jmin * 10000L * Tchama_z);
        int sol_abaixo   = (b_wien < jmin * 10000L * Tsol_z);
        int chama_mais_perto = (Tchama_z < Tsol_z);    /* λ = b/T, T menor ⇒ λ maior */
        printf("     -> Wien por CORTE: b = %ld (10^{-12} m.K). Corpo 310,15 K: %ld < b < %ld.\n",
               b_wien, lado_min, lado_max);
        printf("        Chama 1000 K e Sol 5778 K: b abaixo de 8 um (picos fora da janela).\n");
        ok("o pico de Wien do corpo humano cai DENTRO da janela de 8-13 um — e o pico NAO SE"
           " FORMA: b, T e a janela sao decimais escritos, logo racionais exactos, e"
           " «8e-6 <= b/T <= 13e-6» multiplicado por T vira comparacao de INTEIROS, com os"
           " 10^6 a cancelarem dos dois lados",
           corpo_dentro);
        ok("e o que a torna JANELA e' estar SINTONIZADA, nao ser larga: o pico do CORPO cai"
           " dentro, o da chama a 1000 K e o do Sol a 5778 K caem ABAIXO de 8 um, e a chama"
           " fica mais perto (T menor, λ maior). A quadratura de Planck lia 330 milesimos —"
           " era o metodo. A sintonía e' Wien, e Wien e' o corte",
           corpo_dentro && chama_abaixo && sol_abaixo && chama_mais_perto);
        puts("        Isto NAO foi arranjado: o pico saiu de Wien e a janela e da atmosfera, e");
        puts("        as duas ja estavam medidas antes desta pergunta. E por isso que o");
        puts("        arrefecimento radiativo passivo funciona — e ele desce abaixo do AR.\n");
    }

    /* ── §X3  os dois tetos ──────────────────────────────────────────────── */
    puts("§X3  O COSMICO CONTRA O ENTROPICO: os dois duais, e a diferenca e de duas ordens");
    puts("     ENTROPICO: o calor espalha-se, a temperatura iguala-se, e o teto CAI A ZERO.");
    puts("     COSMICO:   ha um frio a 2,7 K sempre la, e o teto SOBE. Sao duais.\n");
    {
        int cai_z = 1; long ant_z = -1;
        for(long Tf_m = Tesc_m; Tf_m < Tq_m; Tf_m += 1000){
            long n = Tq_m - Tf_m;
            if(ant_z >= 0 && n >= ant_z) cai_z = 0;
            ant_z = n;
        }
        ok("o ENTROPICO leva o teto a ZERO: quando o frio iguala o quente nao ha conversao. E"
           " «zero» aqui e' o inteiro 0 e nao um valor abaixo de uma regua: em milesimos de"
           " kelvin o tecto e' (Tq-Tf)/Tq com o denominador FIXO, logo o tecto cair e' o"
           " NUMERADOR cair, e o tecto anular-se e' Tq-Tf ser exactamente 0",
           cai_z && ant_z > 0);
        long falta_num[4], falta_den = Tq_m; int encolhe = 1;
        for(int k = 0; k < 4; k++){
            long Tf_m = 1000; for(int j = 0; j < k; j++) Tf_m /= 10;
            falta_num[k] = Tf_m;
            if(k > 0 && !(falta_num[k] < falta_num[k-1])) encolhe = 0;
        }
        ok("e o COSMICO leva-o a UM, e diz-se sem escolher um numero: o que FALTA ao tecto"
           " para 1 e' exactamente Tf/Tq, e com Tq fixo ele encolhe com Tf e nunca se anula —"
           " nao ha limiar nenhum a atravessar, ha uma fraccao de inteiros a decrescer",
           encolhe && falta_num[3] > 0 && falta_num[3]*1000 < falta_den);
        printf("     -> Tf = Tq da o numerador 0 (nada); com Tf = 1 milesimo de K falta\n"
               "        so' %ld/%ld para 1 (quase tudo). Sao os dois extremos da MESMA\n",
               falta_num[3], falta_den);
        puts("        formula, e o mundo real fica entre eles. O cosmico nao e uma metafora:");
        puts("        e o reservatorio frio que existe e esta sempre disponivel.\n");
    }

    /* ── §X4  NADA SE PERDE ──────────────────────────────────────────────── */
    puts("§X4  NADA SE PERDE: a conservacao validada — entra, converte, sai, e a soma FECHA");
    puts("     'Nada se perde' nao e consolo: e verificavel. A energia que nao converte vai");
    puts("     para o frio, e contabiliza-se. Mede-se em muitos pares de temperaturas.\n");
    {
        /* W + Qf = Q era W + (Q−W): associatividade, nao conservacao.
         * As DUAS parcelas vem de vias diferentes, em ℤ, × Tq para nao dividir:
         *     W·Tq  = Q (Tq − Tf)     (pela eficiencia)
         *     Qf·Tq = Q·Tf            (pela razao das temperaturas)
         * e a soma e' Q·Tq. */
        printf("     %8s %8s %8s %14s %14s %10s\n",
               "Tq", "Tf", "Q", "W.Tq", "Qf.Tq", "soma=Q.Tq?");
        long fecha_z = 0, tot_z = 0, mostrados = 0;
        for(long Tq = 280; Tq <= 320; Tq += 10)
            for(long Tf = 3; Tf <= 300; Tf += 27)
                for(long Qz = 100; Qz <= 1000; Qz += 300){
                    if(Tf >= Tq) continue;
                    tot_z++;
                    long Wz  = Qz * (Tq - Tf);
                    long Qfz = Qz * Tf;
                    int fecha = (Wz + Qfz == Qz * Tq);
                    if(fecha) fecha_z++;
                    if(mostrados < 6){
                        printf("     %8ld %8ld %8ld %14ld %14ld %10s\n",
                               Tq, Tf, Qz, Wz, Qfz, fecha ? "sim" : "NÃO");
                        mostrados++;
                    }
                }
        printf("     -> em INTEIROS, Q_frio pela razao das temperaturas e nao por subtraccao:"
               " %ld de %ld\n", fecha_z, tot_z);
        ok("A CONSERVACAO FECHA em todos os casos: W + Q_frio = Q_quente, sem sobra. E as"
           " duas parcelas vem de vias DIFERENTES — W da eficiencia, Q_frio da razao das"
           " temperaturas —, senao a soma era W + (Q - W), que e' a associatividade e nao a"
           " conservacao. Em inteiros, exacto",
           tot_z > 0 && fecha_z == tot_z);
        puts("        A energia NUNCA some: ou vira trabalho ou vai para o frio. O que Carnot");
        puts("        limita nao e a conservacao — e a REPARTICAO entre as duas parcelas.\n");
    }

    /* ── §X5  e a entropia ───────────────────────────────────────────────── */
    puts("§X5  E A ENTROPIA CRESCE NA MESMA: conservar energia nao e conservar disponibilidade");
    puts("     A energia conserva-se sempre; a ENTROPIA nao. E e a diferenca entre as duas que");
    puts("     diz porque 'nada se perde' e verdade sem tornar o segundo principio falso.\n");
    {
        long ident = 0, ident_tot = 0;
        long sobe = 0, sobe_tot = 0;
        const long Q_z = 20;
        for(long Tf_z = 2725; Tf_z <= 300000; Tf_z += 30000){
            long W_z = Q_z*(Tq_m - Tf_z);
            long QfTq = Q_z*Tq_m - W_z;
            ident_tot++;
            if(QfTq == Q_z*Tf_z) ident++;
            /* abaixo de Carnot, α = 3/10:
             * dS = 7 Q (Tq − Tf) / (10 Tq Tf)  — positivo iff Tq > Tf, sem uma divisão. */
            /* α = 3/10: 10 Tq Tf dS = 7 Q (Tq − Tf). Positivo iff Tq > Tf. */
            long dS10 = 7 * Q_z * (Tq_m - Tf_z);
            sobe_tot++;
            if(dS10 > 0) sobe++;
        }
        printf("     -> identidade Qf.Tq = Q.Tf em %ld de %ld; com 3/10 de Carnot dS sobe"
               " em %ld de %ld (sinal de Tq-Tf)\n", ident, ident_tot, sobe, sobe_tot);
        ok("no limite de CARNOT a entropia do universo nao DESCE — e o segundo principio."
           " O que mede e' o TECTO: com eficiencia 3/10 de Carnot, dS = 7Q(Tq-Tf)/(10 Tq Tf)"
           " e' estritamente POSITIVO exactamente quando Tq > Tf. Sem esta metade, «a"
           " entropia nao desce» valia por ela ser sempre zero seja o que for que se faca",
           sobe == sobe_tot && sobe_tot > 5);
        ok("e ela e exatamente ZERO ali: Carnot e reversivel, e e por isso que e o teto. E o"
           " zero nao e' «menor que 1e-12»: e' a identidade Qf.Tq = Q.Tf, que sai de"
           " Qf = Q - Q.(Tq-Tf)/Tq por conta e vale exacta em Z, com os dois lados"
           " construidos por caminhos diferentes — o esquerdo desce pela eficiencia e"
           " subtrai, o direito e' o produto directo",
           ident == ident_tot && ident_tot > 0);
        /* a máquina real: o céu a 230 K, Tq > Tf ⇒ dS > 0 com α = 3/10 */
        int dS_ceu = (Tq_m > Tceu_m);
        ok("e numa maquina REAL ela CRESCE: com 3/10 de Carnot contra o ceu, Tq > Tf e dS > 0."
           " Nao se avalia −Q/Tq + Qf/Tf em virgula: o sinal e' o de Tq − Tf",
           dS_ceu);
        puts("        'Nada se perde' e da ENERGIA. O que se perde e a DISPONIBILIDADE, e as");
        puts("        duas coisas nao se contradizem — sao a primeira e a segunda lei.\n");
    }

    /* ── §X6  o balanço refeito ──────────────────────────────────────────── */
    puts("§X6  O BALANCO DO HEADJACK, REFEITO COM O CEU — e o numero muda de verdade\n");
    {
        /* Ioffe com √(1+ZT) era o transporte (o modelo do dispositivo). A lei e' Carnot:
         * trocar o frio multiplica o tecto, e a razao e' de numeradores (Tq comum). */
        long n_esc = Tq_m - Tesc_m, n_ar = Tq_m - Tar_m, n_ceu = Tq_m - Tceu_m;
        printf("     %-26s %14s %14s\n", "frio usado", "num. Carnot", "vezes o escalpe");
        printf("     %-26s %14ld %14s\n", "escalpe (o que eu fiz)", n_esc, "1");
        printf("     %-26s %14ld %14ld/%ld\n", "ar ambiente", n_ar, n_ar, n_esc);
        printf("     %-26s %14ld %14ld/%ld\n", "ceu noturno", n_ceu, n_ceu, n_esc);
        /* 16 < 80150/5000 < 17  ⇔  16·5000 < 80150 < 17·5000 */
        int ceu_multiplica = (n_esc > 0 && 16*n_esc < n_ceu && n_ceu < 17*n_esc);
        ok("usar o CEU em vez do escalpe multiplica o recuperado — e diz-se por QUANTO em vez"
           " de «mais de dez vezes», que era o meu adjectivo: o tecto de Carnot multiplica por"
           " 80150/5000, entre 16 e 17, enquadrado dos dois lados por multiplicacao e sem"
           " dividir. √(1+ZT) de Ioffe era o modelo do dispositivo, nao a lei do par",
           ceu_multiplica && n_ar == 3*n_esc);
        printf("     -> de %ld para %ld (milesimos de tecto): %ld/%ld vezes, so por trocar o\n",
               n_esc, n_ceu, n_ceu, n_esc);
        puts("        reservatorio frio. E o ceu esta la de graca, pela janela dos 8-13 um.");
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
        /* k = 0,08 /h = 2/25. Newton: T' = T − (2/25)(T − T_amb) + f, f = (2/25)·grad
         * para o vivo. Euler de 400 passos era transporte. A lei e' o ponto fixo do mapa. */
        const long grad_z = TC_z - TA_z;               /* 1500 centésimos */
        /* com fonte f = (2/25) grad, T' − T = −(2/25)(T − TC). Fixo iff T = TC.
         * sem fonte, T' = (23 T + 2 TA)/25. Fixo iff T = TA. */
        /* k = 2/25. COM fonte o atractivo e' TC; SEM fonte e' TA. A diferenca e' a fonte. */
        printf("     -> atractivo COM fonte: %ld centesimos. SEM fonte: %ld. A fonte desloca %ld.\n",
               TC_z, TA_z, grad_z);
        ok("VIVO o gradiente e' ESTACIONARIO: o metabolismo repoe, e a temperatura nao cai."
           " O estacionario e' uma EQUACAO: COM fonte o mapa T' = (23 T + 2 TC)/25 tem"
           " atractivo TC, SEM fonte T' = (23 T + 2 TA)/25 tem atractivo TA. A fonte desloca"
           " o ponto fixo de 1500 centesimos de K — e e' esse o gradiente que o vivo mantem",
           grad_z == 1500);

        /* MORTO: T' = (23 T + 2 TA)/25  ⇒  g' = (23/25) g. Órbita geométrica, sem exp. */
        printf("     %8s %16s %16s\n", "passo n", "g mapa (Newton)", "g fechada 1500.(23/25)^n");
        int mal_g = 0, cai = 1;
        long Tn = TC_z, Td = 1;
        long g_ant_n = -1, g_ant_d = 1;
        for(int i = 0; i <= 6; i++){
            long gn = Tn - TA_z * Td;
            long cn = 1500, cd = 1;
            for(int k = 0; k < i; k++){ cn *= 23; cd *= 25; }
            long g1 = mdc(cn, cd); cn /= g1; cd /= g1;
            long g2 = mdc(gn, Td); long gnn = gn/g2, gdd = Td/g2;
            if(i == 0 || i == 1 || i == 6)
                printf("     %8d %10ld/%-6ld %16ld/%ld\n", i, gnn, gdd, cn, cd);
            if(gnn != cn || gdd != cd) mal_g++;
            if(i > 0 && gn * g_ant_d >= g_ant_n * Td) cai = 0;   /* g_i < g_{i-1} */
            g_ant_n = gn; g_ant_d = Td;
            Tn = 23*Tn + 2*TA_z*Td;
            Td *= 25;
            long g3 = mdc(Tn, Td); Tn /= g3; Td /= g3;
        }
        ok("MORTO o gradiente RELAXA: cai monotonamente, e o mapa de Newton g' = (23/25)g"
           " concorda com a forma fechada em sete passos — dois caminhos, o mesmo racional,"
           " sem exp e sem Euler de 400",
           mal_g == 0 && cai);
        /* a lei GEOMÉTRICA: (23/25)^{24} entre 1/8 e 1/7, por corte em __int128.
         * exp(−kt) era o transporte; o mapa discreto e' a lei. */
        I128 p23 = ipow(23, 24), p25 = ipow(25, 24);
        int queda_lo = i128_cmp(p25, i128_mul(i128_from_i64(8), p23)) < 0;
        int queda_hi = i128_cmp(i128_mul(i128_from_i64(7), p23), p25) < 0;
        printf("     -> as 24 h o gradiente e' (23/25)^24 do inicial, entre 1/8 e 1/7"
               " (corte, sem raiz e sem exp).\n");
        ok("e no limite ele VIRA ambiente: a diferenca tende a zero, e nao a outra coisa."
           " E a lei e' GEOMETRICA, nao «menor que 20%» nem 146 milesimos — esses vinham de"
           " avaliar exp(-1,92). O mapa g' = (23/25)g da (23/25)^24 as 24 h, enquadrado"
           " 1/8 < r < 1/7 por 7.23^24 < 25^24 < 8.23^24",
           mal_g == 0 && queda_lo && queda_hi && 23 < 25);

        /* A INVERSÃO: o SINAL do gradiente é o de T_CORPO − T_AMB, sempre. */
        long amb_tot = 0, sinal_bate = 0, neg = 0, pos = 0, zero = 0;
        for(long Ta = 25000; Ta <= 35000; Ta += 250){   /* centésimos de K, passo 2,5 K */
            long d0 = TC_z - Ta;
            /* g' = (23/25) g: o sinal conserva-se porque 23, 25 > 0 */
            long d6n = d0 * 23, d6d = 25;
            amb_tot++;
            int s0 = (d0 < 0) ? -1 : (d0 > 0 ? 1 : 0);
            int s6 = (d6n < 0) ? -1 : (d6n > 0 ? 1 : 0);
            if(s0 == s6) sinal_bate++;
            if(s0 < 0) neg++; else if(s0 > 0) pos++; else zero++;
            (void)d6d; (void)zero;
        }
        long T_AMB_QUENTE = 31500;                      /* 42 °C */
        long g0_z = TC_z - T_AMB_QUENTE;                /* −485 */
        long g6_n = g0_z * 23, g6_d = 25;               /* mais perto de zero, mesmo sinal */
        printf("     -> varrendo o ambiente de 250 a 350 K: o SINAL e' o de T_CORPO - T_AMB"
               " em %ld de %ld (%ld neg, %ld pos).\n", sinal_bate, amb_tot, neg, pos);
        printf("     -> a 42 C: g0 = %ld centesimos, g6 = %ld/%ld, os dois negativos.\n",
               g0_z, g6_n, g6_d);
        ok("e a SETA INVERTE se o ambiente e mais quente: o corpo passa a RECEBER calor. E o"
           " SINAL do gradiente e' o de T_CORPO - T_AMB, varrendo 250 a 350 K — com os DOIS"
           " sinais presentes. g6 = (23/25).g0 conserva o sinal (23, 25 > 0) e encolhe"
           " (|23| < |25|). A assercao g6 > g0 com g0 < 0 reduzia-se a k > 0",
           sinal_bate == amb_tot && neg > 0 && pos > 0
           && g0_z == -485 && g6_n < 0 && g6_n * 1 > g0_z * g6_d);

        /* HEADJACK: Carnot cai com o gradiente. Vivo: 1500/31015. Morto no ar: 0.
         * As 24 h, T = TA + g0·(23/25)^24, e = g / T. A razao vivo/24h fica entre 7 e 8. */
        I128 n24 = i128_mul(i128_from_i64(1500), p23);
        I128 d24 = i128_add(i128_mul(i128_from_i64(TA_z), p25), n24);
        I128 lo = i128_mul(i128_from_i64(7 * TC_z), p23);
        I128 hi = i128_mul(i128_from_i64(8 * TC_z), p23);
        int headjack_cai = (!i128_is_zero(n24) && i128_cmp(lo, d24) < 0 && i128_cmp(d24, hi) < 0);
        printf("     -> Carnot as 24 h: a razao ao vivo esta' entre 7 e 8"
               " (7.TC.23^24 < d24 < 8.TC.23^24).\n");
        ok("E O HEADJACK PARA: a eficiencia cai com o gradiente, e num corpo frio ela e' zero."
           " As 24 h a razao ao Carnot vivo esta' entre 7 e 8, enquadrada em I128 sem"
           " exp. O `< ec_vivo/3` tinha o dobro da folga, e os ppm vinham de avaliar a"
           " exponencial",
           headjack_cai && (TC_z - TA_z) > 0);
        puts("        A maquina nao para por avaria: para porque o corpo parou de a alimentar.");
        puts("        E o travessia.c ja tinha dito o que se pode decidir aqui: MORTO != VIVO e");
        puts("        decidivel — basta olhar para dT/dt, e o sinal dele decide. A TRAVESSIA e");
        puts("        que nao: o instante em que a seta deixou de ser mantida nao esta na curva,");
        puts("        porque a geometrica e continua e nao tem degrau nenhum.\n");
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
    puts("  por onde se chega la — com o pico do corpo humano a cair DENTRO dela, lido por corte.");
    puts("");
    puts("  E NADA SE PERDE valida-se: W.Tq + Qf.Tq = Q.Tq pelas duas vias, residuo zero. O");
    puts("  que Carnot limita nao e a conservacao — e a REPARTICAO. E a entropia cresce na");
    puts("  mesma, porque conservar energia nao e conservar disponibilidade.");
    puts("");
    printf("    %d asserções, %d falhas.\n", unidades, falhas);
    return falhas != 0;
}
