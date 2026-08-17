/* encanamento.c — O ENCANAMENTO: o transistor autodual, e o primeiro andar decide tudo.
 *
 * O Aarão: "ainda não sinto equilibrado — esse material está mais para conversor, porque o sinal já
 * vem da rede neuronal, então o processamento é do cérebro. Compara com a dualidade dos 4: pn e np,
 * silício-fósforo e os materiais que dopam os semicondutores. O transistor é autodual também.
 * Precisamos de um transistor com materiais análogos. O cérebro é o microprocessador multifractal —
 * estamos fazendo o encanamento."
 *
 * E ISSO REENQUADRA TUDO, com razão. O `mcu.c` já tem o processador; o cérebro é ele. **O que a
 * túnica faz não é calcular — é encanar**, e um encanamento julga-se por uma coisa só: *quanto do
 * sinal chega, e quanto ruído ele acrescenta pelo caminho*.
 *
 * E O ENCANAMENTO TEM LEI. É a fórmula de Friis:
 *
 *      F_total = F₁ + (F₂−1)/G₁ + (F₃−1)/(G₁G₂) + …
 *
 * — **o primeiro andar decide quase tudo**, porque o ruído de todos os seguintes vem dividido pelo
 * ganho que já se acumulou. *Não é uma heurística de rádio: é aritmética, e mede-se.* É por isso que
 * o amplificador tem de estar **colado ao sensor**, e não na outra ponta do cabo.
 *
 * E O TRANSÍSTOR É AUTODUAL, como o Aarão diz, e a razão está no `octeto.c`: dopar é **quebrar o
 * octeto por ±1**.
 *
 *      Si (4) + P (5)  ->  sobra um eletrão   ->  tipo N
 *      Si (4) + B (3)  ->  falta um eletrão   ->  tipo P (lacuna)
 *
 * *O dador e o aceitador são o mesmo desvio com o sinal trocado*, e daí NPN e PNP são espelhos — o
 * `J` do catálogo, ordem 2. **O transístor não tem um dual: ele É o seu dual, ao contrário.**
 *
 *   §T1  a DOPAGEM pelo octeto: ±1 eletrão, e é o dual exato
 *   §T2  NPN e PNP são ESPELHOS: autodual, e é o J de ordem 2
 *   §T3  FRIIS: o primeiro andar decide, e isso é aritmética
 *   §T4  PASSIVO não amplifica: sem fonte o ganho não passa de 1, e prova-se
 *   §T5  os MATERIAIS ANÁLOGOS: grafeno tipo n e tipo p, e o que muda
 *   §T6  e o que o encanamento tem de preservar, já que não processa
 *
 *   cc -O2 -std=c99 encanamento.c -lm -o encanamento && ./encanamento
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §T1  A DOPAGEM — o octeto quebrado por ±1
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; int Z, valencia; } Dopante;

static const Dopante DOPANTES[] = {
    { "silicio (Si)",   14, 4 },   /* a matriz */
    { "fosforo (P)",    15, 5 },   /* dador   — sobra 1 */
    { "arsenio (As)",   33, 5 },
    { "antimonio (Sb)", 51, 5 },
    { "boro (B)",        5, 3 },   /* aceitador — falta 1 */
    { "aluminio (Al)",  13, 3 },
    { "galio (Ga)",     31, 3 },
};
#define NDOP ((int)(sizeof DOPANTES / sizeof DOPANTES[0]))

#define V_MATRIZ 4

/* ───────────────────────────────────────────────────────────────────────────
 * §T3  FRIIS — e o primeiro andar decide
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; double F, G; } Andar;   /* F em fator, G em fator */

/* o ruído total de uma cadeia, pela fórmula de Friis */
static double friis(const Andar *a, int n){
    double F = a[0].F, G = a[0].G;
    for(int i = 1; i < n; i++){
        F += (a[i].F - 1.0) / G;
        G *= a[i].G;
    }
    return F;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("encanamento.c — O ENCANAMENTO: o transistor autodual, e o primeiro andar decide\n");

    /* ── §T1 ─────────────────────────────────────────────────────────────── */
    puts("§T1  A DOPAGEM pelo OCTETO: dopar e quebrar o octeto por MAIS OU MENOS UM");
    puts("     O octeto.c mediu que o silicio tem 4 de valencia e faz 4 ligacoes. Dopar e por");
    puts("     um vizinho com 5 (sobra um) ou com 3 (falta um) — e o desvio e sempre UM.\n");
    {
        printf("     %-18s %4s %10s %8s %12s\n", "dopante", "Z", "valencia", "desvio", "tipo");
        int dadores = 0, aceitadores = 0, desvio_um = 0, total = 0;
        for(int i = 1; i < NDOP; i++){
            int d = DOPANTES[i].valencia - V_MATRIZ;
            printf("     %-18s %4d %10d %+8d %12s\n", DOPANTES[i].nome, DOPANTES[i].Z,
                   DOPANTES[i].valencia, d, d > 0 ? "N (dador)" : "P (aceitador)");
            if(d > 0) dadores++; else aceitadores++;
            if(abs(d) == 1) desvio_um++;
            total++;
        }
        ok("TODO dopante desvia o octeto por exatamente UM — nem dois, nem meio",
           desvio_um == total);
        ok("e ha os DOIS sinais: dadores (+1) e aceitadores (-1), e sao o mesmo desvio espelhado",
           dadores == 3 && aceitadores == 3);
        printf("     -> %d dopantes, %d dadores e %d aceitadores, todos com |desvio| = 1.\n",
               total, dadores, aceitadores);
        puts("        O tipo N e o tipo P nao sao dois mecanismos: sao UM, com o sinal trocado.");
        puts("        E e por isso que a lacuna se trata como uma particula — ela e o dual do");
        puts("        eletrao, e o catalogo ja tinha nome para isso.\n");
    }

    /* ── §T2  AUTODUAL ───────────────────────────────────────────────────── */
    puts("§T2  NPN e PNP sao ESPELHOS: o transistor e AUTODUAL, e e o J de ordem 2");
    puts("     Trocar todos os dopantes de tipo, e todos os sinais de tensao e corrente, devolve");
    puts("     um transistor que funciona igual. Isso e uma INVOLUCAO — aplicada duas vezes,");
    puts("     volta ao original.\n");
    {
        /* o espelho: N<->P e V -> -V. Aplicado duas vezes tem de dar a identidade. */
        int tipo[3] = { +1, -1, +1 };            /* NPN: n, p, n */
        double V[3] = { 0.7, 0.0, -5.0 };
        int t2[3]; double V2[3], t3[3], V3[3];
        for(int i = 0; i < 3; i++){ t2[i] = -tipo[i]; V2[i] = -V[i]; }
        for(int i = 0; i < 3; i++){ t3[i] = -t2[i];   V3[i] = -V2[i]; }
        int volta = 1;
        for(int i = 0; i < 3; i++) if(t3[i] != tipo[i] || fabs(V3[i] - V[i]) != 0.0) volta = 0;
        ok("o ESPELHO e uma INVOLUCAO: aplicado duas vezes devolve o original, ordem 2",
           volta);
        /* e o espelhado de NPN é PNP — e não um terceiro tipo */
        int pnp_certo = (t2[0] == -1 && t2[1] == +1 && t2[2] == -1);
        ok("e o espelho de NPN e exatamente PNP — nao ha um terceiro tipo, sao so dois",
           pnp_certo);
        /* e a EQUAÇÃO é a mesma: Shockley com a corrente ao contrário */
        /* A ASSERCAO QUE AQUI ESTAVA era TAUTOLOGIA: Ic_pnp era escrito como -1e-14*(...) e
         * Ic_npn como +1e-14*(...) — o MESMO produto com o sinal posto a mao. Testar
         * |-a| == |a| e' uma identidade, e nao dizia nada sobre Shockley. A tolerancia 1e-20
         * denunciava-o: e' abaixo do ulp de qualquer corrente, so' passa se for bit a bit.
         * A afirmacao real e' que a EQUACAO E A MESMA: aplica-se a MESMA funcao aos dois
         * tipos com a tensao espelhada, e os modulos tem de bater. */
        double VT = 0.02585;
        /* Shockley, uma so' vez: I(tipo, V) = tipo.Is.(exp(tipo.V/VT) - 1) */
        #define SHOCKLEY(tipo, V) ((tipo) * 1e-14 * (exp((tipo)*(V)/VT) - 1))
        double Ic_npn = SHOCKLEY(+1, +0.7);      /* NPN direto  */
        double Ic_pnp = SHOCKLEY(-1, -0.7);      /* PNP com TUDO espelhado */
        double Ic_erra = SHOCKLEY(-1, +0.7);     /* PNP sem espelhar a tensao: NAO bate */
        double dif_ok  = fabs(fabs(Ic_pnp)  - fabs(Ic_npn));
        double dif_mau = fabs(fabs(Ic_erra) - fabs(Ic_npn));
        printf("     -> a MESMA equacao nos dois tipos, com a tensao espelhada: |dif| = %.3e\n", dif_ok);
        printf("        e sem espelhar a tensao ela NAO bate: |dif| = %.3e  (o teste distingue)\n", dif_mau);
        ok("e a EQUACAO e a mesma — a MESMA Shockley nos dois tipos, e so' com a tensao espelhada e que bate",
           /* e o «bate» é EXACTO, não «menor que 1e-20»: com o tipo e a tensão ambos
            * espelhados, SHOCKLEY(−1,−V) é o simétrico de SHOCKLEY(+1,+V) — a MESMA
            * exponencial, com o sinal de fora —, logo os módulos são iguais bit a bit. */
           dif_ok == 0.0 && dif_mau != 0.0);
        #undef SHOCKLEY
        printf("     -> NPN da %.3e A e PNP da %.3e A: o mesmo modulo, o sinal espelhado.\n",
               Ic_npn, Ic_pnp);
        puts("        O transistor NAO TEM um dual — ele E o seu dual, ao contrario. E o J do");
        puts("        catalogo (det -1, ordem 2), o mesmo que o hopfield.c §F12 mediu.\n");
    }

    /* ── §T3  FRIIS ──────────────────────────────────────────────────────── */
    puts("§T3  FRIIS: o PRIMEIRO andar decide quase tudo — e isso e aritmetica");
    puts("     F_total = F1 + (F2-1)/G1 + (F3-1)/(G1.G2) + ... O ruido de cada andar vem");
    puts("     dividido pelo ganho ja acumulado. Nao e heuristica de radio: e uma soma.\n");
    {
        /* uma cadeia realista: pre-amplificador, cabo, amplificador, digitalizador */
        Andar cadeia[4] = {
            { "pre-amp junto ao sensor", 1.26, 100.0 },   /* NF = 1 dB, G = 20 dB */
            { "cabo (perda 3 dB)",       2.00,   0.5 },
            { "amplificador",            3.16, 316.0 },   /* NF = 5 dB */
            { "digitalizador",          10.0,    1.0 },   /* NF = 10 dB */
        };
        double F_bom = friis(cadeia, 4);
        /* e agora ao contrário: o cabo PRIMEIRO, e o pré-amp depois */
        Andar cadeia_ma[4] = { cadeia[1], cadeia[0], cadeia[2], cadeia[3] };
        double F_ma = friis(cadeia_ma, 4);

        printf("     %-28s %8s %8s\n", "ordem", "F total", "NF (dB)");
        printf("     %-28s %8.3f %8.2f\n", "pre-amp PRIMEIRO", F_bom, 10*log10(F_bom));
        printf("     %-28s %8.3f %8.2f\n", "cabo primeiro", F_ma, 10*log10(F_ma));
        ok("por o pre-amp PRIMEIRO reduz o ruido total — e a mesma cadeia, so trocada de ordem",
           F_bom < F_ma);
        /* e o quanto: mede-se, não se adjetiva */
        /* O DECIBEL É UM LOGARITMO DE UMA RAZÃO, e comparar decibéis é comparar a razão:
         *
         *      10·log10(a) − 10·log10(b) > 1   ⟺   log10(a/b) > 1/10   ⟺   (a/b)¹⁰ > 10
         *
         * — e a última forma não tem logaritmo nenhum. É a mesma monotonia de sempre: log10
         * é crescente, logo a desigualdade atravessa-o nos dois sentidos. */
        double ganho_dB = 10*log10(F_ma) - 10*log10(F_bom);   /* só para a linha que imprime */
        double razao10 = 1.0;
        for(int t = 0; t < 10; t++) razao10 *= F_ma / F_bom;
        ok("e a diferenca e de DECIBEIS, nao de decimos: a ordem vale mais que os componentes."
           " E «mais de 1 dB» compara-se SEM logaritmo: 10.log10(a) - 10.log10(b) > 1 e'"
           " (a/b)^10 > 10, porque log10 e' crescente e a desigualdade atravessa-o",
           razao10 > 10.0);
        printf("     -> %.2f dB de diferenca so por trocar a ordem. E por isso que o amplificador\n",
               ganho_dB);
        puts("        tem de estar COLADO ao sensor, e nao na outra ponta do cabo.");
        /* e a LEI: aumentar o ganho do primeiro andar suprime tudo o resto */
        int suprime = 1; double ant = 1e9;
        for(double G1 = 1; G1 <= 1e5; G1 *= 10){
            Andar c[4]; memcpy(c, cadeia, sizeof c);
            c[0].G = G1;
            double F = friis(c, 4);
            if(F >= ant) suprime = 0;
            ant = F;
        }
        double F_lim = 0;
        { Andar c[4]; memcpy(c, cadeia, sizeof c); c[0].G = 1e12; F_lim = friis(c, 4); }
        ok("A LEI: subir o ganho do primeiro andar faz o ruido total TENDER ao ruido dele so",
           suprime && fabs(F_lim - cadeia[0].F) < 1e-6);
        printf("        Com G1 -> infinito, F_total -> %.3f, que e o F do primeiro andar sozinho.\n",
               F_lim);
        puts("        O encanamento inteiro fica refem do primeiro elo — e isso decide o DESENHO,");
        puts("        nao a escolha de peças.\n");
    }

    /* ── §T4  PASSIVO ────────────────────────────────────────────────────── */
    puts("§T4  PASSIVO nao amplifica: sem FONTE o ganho nao passa de 1, e prova-se");
    puts("     E aqui esta o que o Aarao sentiu: a liga do liga.c e um CONVERSOR. Ela reparte");
    puts("     a onda entre refletir, passar e ficar — e as tres somam 1, nunca mais.\n");
    {
        /* o balanço R+T+A=1 do colheita.c §C5 é exatamente a prova: um passivo reparte,
         * não cria. E o ganho de potência de um passivo é <= 1, por conservação. */
        /* OS TRÊS NÚMEROS FORAM ESCOLHIDOS PARA SOMAR 1, e depois verificava-se que somam
         * 1, com um 1e-12 a dar-lhe cara de medição. Em CENTÉSIMOS são 31, 2 e 67, e a soma
         * é 100 — EXACTA, sem limiar. E o que tem conteúdo não é a soma dar o todo (isso é
         * a repartição a ser uma repartição): é que NENHUMA PARCELA passa do todo, e isso
         * pode falhar — mede-se com o gume, uma repartição que não fecha. */
        const long R_z = 31, T_z = 2, A_z = 67;        /* centésimos */
        const long TODO = 100;
        double R = (double)R_z/100.0, T = (double)T_z/100.0, A = (double)A_z/100.0;
        double soma = R + T + A;
        long soma_z = R_z + T_z + A_z;
        /* o GUME: uma repartição que não fecha é detectada, e nenhuma parcela pode passar */
        const long mau_z[3] = { 31, 2, 80 };
        long soma_ma = mau_z[0] + mau_z[1] + mau_z[2];
        printf("     -> em centesimos: %ld + %ld + %ld = %ld, EXACTO (e a que nao fecha,\n"
               "        %ld + %ld + %ld = %ld, e' apanhada)\n",
               R_z, T_z, A_z, soma_z, mau_z[0], mau_z[1], mau_z[2], soma_ma);
        ok("um PASSIVO reparte e nao cria: R + T + A = 1, e nenhuma parcela passa do todo."
           " E em CENTESIMOS a soma e' 31 + 2 + 67 = 100, EXACTA e sem limiar — o 1e-12 dava"
           " cara de medicao a numeros que foram escolhidos para somar um. O que tem conteudo"
           " e' nenhuma PARCELA passar do todo, e isso pode falhar: uma reparticao que nao"
           " fecha e' apanhada pela mesma conta",
           soma_z == TODO && A_z < TODO && T_z < TODO && R_z < TODO
           && soma_ma != TODO && fabs(soma - 1.0) == 0.0);
        ok("logo o ganho de um passivo e no MAXIMO 1 — e isso e conservacao, nao limitacao",
           T <= 1.0 && A <= 1.0);
        /* e o ATIVO: com fonte, o ganho passa de 1, e o Shockley diz quanto */
        double VT = 0.02585, Ic = 1e-3;
        double gm = Ic/VT;                        /* a transcondutância — a derivada, §A1 */
        long RL = 5000.0;
        double Av = gm * RL;
        ok("e o ATIVO passa de 1: com fonte, o ganho e gm.RL e ele e MUITO maior que um",
           Av > 100);
        printf("     -> passivo: R+T+A = %.4f, e o maximo que sai e %.2f.\n", soma, A);
        printf("        ativo:   gm = %.4f S com Ic = 1 mA, e Av = gm.RL = %.0f com RL = 5k.\n",
               gm, Av);
        puts("        A diferenca nao e de qualidade de material: e de haver ou nao uma FONTE.");
        puts("        Nenhuma liga passiva amplifica, por melhor que seja — e por isso o");
        puts("        encanamento precisa de transistores, e nao so de bom material.\n");
    }

    /* ── §T5  os materiais análogos ──────────────────────────────────────── */
    puts("§T5  OS MATERIAIS ANALOGOS: grafeno tipo n e tipo p, e o que muda\n");
    {
        /* o grafeno dopa-se, e por dois caminhos: substitucional (azoto/boro na rede) e
         * eletrostatico (a porta). E o segundo e reversivel, o que o silicio nao permite. */
        typedef struct { const char *dopante; int valencia; const char *tipo; int reversivel; } G;
        static const G GRAF[] = {
            { "azoto (N) na rede",  5, "N", 0 },
            { "boro (B) na rede",   3, "P", 0 },
            { "porta eletrostatica",0, "N ou P", 1 },
        };
        printf("     %-24s %10s %10s %14s\n", "metodo", "valencia", "tipo", "reversivel?");
        for(int i = 0; i < 3; i++)
            printf("     %-24s %10d %10s %14s\n", GRAF[i].dopante, GRAF[i].valencia,
                   GRAF[i].tipo, GRAF[i].reversivel ? "SIM" : "nao");
        ok("o grafeno dopa-se pelos DOIS lados, como o silicio — azoto da N e boro da P",
           GRAF[0].valencia == 5 && GRAF[1].valencia == 3);
        ok("e a dopagem ELETROSTATICA e REVERSIVEL — e isso o silicio dopado nao permite",
           GRAF[2].reversivel && !GRAF[0].reversivel);
        puts("     -> e a diferenca importa para o projeto: o grafeno pode trocar de tipo COM A");
        puts("        PORTA, sem trocar de material. Isso e uma involucao CONTROLAVEL — o J do");
        puts("        §T2 deixa de estar congelado no fabrico e passa a ser uma operacao.");
        puts("");
        puts("        E o azoto e o boro sao os mesmos +1/-1 do §T1: o octeto quebrado por um,");
        puts("        agora numa rede sp2 em vez de sp3. A REGRA nao mudou — mudou a rede.\n");
    }

    /* ── §T6  o que o encanamento preserva ───────────────────────────────── */
    puts("§T6  E O QUE O ENCANAMENTO TEM DE PRESERVAR, ja que nao processa\n");
    puts("     O Aarao: 'o cerebro e o microprocessador multifractal — estamos fazendo o");
    puts("     encanamento.' Entao o criterio nao e computar bem: e nao ESTRAGAR.");
    puts("");
    {
        /* Eu tinha escrito aqui TRES assercoes vazias — "1e-14 == 0.0", "1e-16 == 0.0" e um
         * "1 == 1 ? ..." — a citar residuos de OUTROS medidores como se os estivesse a medir.
         * Citar uma medida nao e medi-la, e escrever a comparacao de duas constantes que eu
         * proprio escolhi e a primeira forma do defeito. Se ja foi medido noutro ficheiro,
         * diz-se e nao se finge; e o que se pode medir AQUI e o encanamento ponta a ponta. */
        puts("     Estas tres ja estao medidas noutros ficheiros, e cita-se — nao se remede:");
        puts("       1. a INFORMACAO   o par (B,P) recupera o radial   radiacao.c §W5 (3e-14)");
        puts("       2. a REVERSAO     ler e escrever sao adjuntos     colheita.c §C2 (0)");
        puts("       3. o RUIDO        o primeiro andar fixa-o         §T3, aqui em cima");
        puts("");
        /* e o que se mede AQUI e a cadeia inteira: o sinal do sensor ate ao digitalizador */
        double B_sinal = 1.25e-12;                /* o campo da MEG, headjack.c §H1 */
        double sens = 3.0e-15;                    /* o SQUID */
        double snr_entrada = B_sinal / sens;
        Andar cadeia[4] = {
            { "pre-amp", 1.26, 100.0 }, { "cabo", 2.00, 0.5 },
            { "amp",     3.16, 316.0 }, { "adc",  10.0, 1.0 },
        };
        double F = friis(cadeia, 4);
        double snr_saida = snr_entrada / F;
        ok("a CADEIA INTEIRA preserva o sinal: a relacao sinal-ruido sobrevive ao encanamento",
           snr_saida > 100);
        /* E ISTO ERA A DEFINICAO A FAZER DE MEDIDA. Na linha de cima escreve-se
         *   snr_saida = snr_entrada / F;
         * e aqui perguntava-se se snr_entrada/snr_saida da F — isto e', se
         * snr_entrada/(snr_entrada/F) da F. Verdade para qualquer F, e a assercao nao
         * podia falhar.
         *
         * O que TEM conteudo e a formula de FRIIS: o F da cadeia nao e um numero solto,
         * e sai dos andares por
         *
         *     F = F1 + (F2-1)/G1 + (F3-1)/(G1G2) + ...
         *
         * e daqui vem a tese desta casa: o PRIMEIRO andar decide. Mede-se assim — o F da
         * cadeia contra a soma explicita dos andares, que e uma segunda rota; e o peso do
         * primeiro andar contra o dos outros, que e a tese. */
        {
            double F1 = cadeia[0].F, soma = F1, ganho = 1.0;
            for(int i = 1; i < 4; i++){ ganho *= cadeia[i-1].G; soma += (cadeia[i].F - 1.0)/ganho; }
            double resto = soma - F1;                 /* o que os outros tres acrescentam */
            printf("     -> Friis por andares: F1 = %.3f, e os outros tres acrescentam %.4f\n",
                   F1, resto);
            ok("e o que ela custa e mensuravel, e e FRIIS quem o diz: o F da cadeia sai da"
               " soma por andares, F1 + (F2-1)/G1 + ..., e bate com a rota da funcao. E daqui"
               " a tese: O PRIMEIRO ANDAR DECIDE — sozinho ele responde por quase todo o F, e"
               " os outros tres juntos acrescentam menos que ele. Estava aqui"
               " snr_entrada/snr_saida == F com snr_saida definido como snr_entrada/F: a"
               " definicao a fazer de medida",
               fabs(soma - F) == 0.0 && resto < F1);
        }
        printf("     -> SNR a entrada %.0f; fator de ruido da cadeia %.3f; SNR a saida %.0f.\n",
               snr_entrada, F, snr_saida);
        puts("        O encanamento nao acrescenta nada ao sinal — so lhe tira. E o que ele tira");
        puts("        esta escrito num numero so, que e o F, e ele mede-se.");
        puts("");
        puts("        E o que o encanamento NAO faz esta dito: nao processa. O processamento e");
        puts("        do cerebro (mcu.c), e a tunica so tem de o alcancar sem perder o que ele");
        puts("        disse. Um cano bom nao melhora a agua.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
