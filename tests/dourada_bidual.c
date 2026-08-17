/* dourada_bidual.c — O BIDUAL DA TRANSFORMADA DOURADA, E O QUE ELE RESOLVE.
 *
 * O Aarão: «vê a transformada dourada e vê o bidual dela — a dualidade Fourier/Mellin talvez
 * seja melhor para tu enxergares assim» · e depois: «leste a dourada que te falei? derivaste
 * o bidual dela?»
 *
 * NÃO TINHA. Li duas linhas sobre Mellin e saltei para mexer no `y` da tabela — quatro
 * tentativas, duas delas a piorar, e revertidas. O `dourada.c` estava aqui há dias e diz
 * exatamente o que me faltava.
 *
 * O PAR:
 *
 *      FOURIER    o grupo ADITIVO        (R, +)      a translação      u ↦ u + c
 *      MELLIN     o grupo MULTIPLICATIVO (R>0, ×)    a dilatação       x ↦ λx
 *
 * e a ponte é `x = e^u`: «Mellin É Fourier no log». Os caracteres do multiplicativo são as
 * potências x^{-s} — exponenciais VESTIDAS de função-potência.
 *
 * E O QUE ISSO RESOLVE, que é o meu defeito de hoje: «A DILATAÇÃO SÓ GIRA A FASE».
 * |λ^{-iτ}| = 1 — o módulo NÃO MUDA. Mudar a escala de uma coisa não move as outras: muda
 * apenas ONDE ela está na sua própria volta.
 *
 * Numa tabela, a altura de uma célula é uma DILATAÇÃO (quantas vezes a coluna cabe no
 * conteúdo — multiplicativo) e a posição da fila é uma TRANSLAÇÃO (aditivo). Eu estava a
 * medir a dilatação com a régua da translação: cada `&` somava UMA entrelinha, como se a
 * altura fosse uma soma. Daí uma célula «pular linha para dar espaço a outra» — que é a
 * dilatação de uma a mover a outra, e isso não existe: ela só gira a fase.
 *
 *   §W1  Mellin É Fourier no log: o caractere x^{-s} = e^{-s ln x}, exacto
 *   §W2  a DILATAÇÃO só gira a FASE — o módulo não muda, e é isso o bidual
 *   §W3  o BIDUAL: exp e log são o par, e a volta fecha nos DOIS sentidos
 *   §W4  e a aplicação: a altura é MULTIPLICATIVA e a posição é ADITIVA — eixos distintos
 *   §W5  o controlo: medir a dilatação com a régua da translação ACUMULA erro
 *
 *   cc -O2 -std=gnu99 -I../lib dourada_bidual.c -lm -o dourada_bidual && ./dourada_bidual
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "banco.h"
#include "unidade.h"
#include "reta.h"

#define BASE "/tmp/cards_banco"

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const double phi = (1.0 + sqrt(5.0)) / 2.0;

printf("\n=== O BIDUAL DA TRANSFORMADA DOURADA =========================================\n");

printf("\n§W1  Mellin E' Fourier no log: o caractere x^{-s} = e^{-s ln x}.\n\n");
    long e_o_mesmo = 0;
    {
        /* a ponte e' x = e^u. O caractere do multiplicativo, avaliado em x, e' o caractere do
         * aditivo avaliado em ln x — e sao o MESMO numero, nao dois parecidos. */
        long difs = 0, casos = 0, exactos = 0, exactos_tot = 0;
        printf("      x        s        x^{-s}          e^{-s ln x}     difere?\n");
        for(double x = 1.5; x <= 4.0; x += 0.5)
            for(double s = 0.5; s <= 2.0; s += 0.5){
                double a = pow(x, -s), c = exp(-s * log(x));
                casos++;
                if(fabs(a - c) > 1e-12) difs++;
                /* E A PONTE TEM UM CASO EXACTO, que é o que a torna medição e não uma
                 * comparação entre duas rotinas da libm: com s INTEIRO e x racional,
                 * x^{-s} é 1/x^s e calcula-se em ℤ. Aqui x anda em meios e s em meios; nos
                 * pares em que ambos são inteiros a conta fecha sem transcendental. */
                if(x == (double)(long)x && s == (double)(long)s){
                    long xi = (long)x, si = (long)s;
                    long xs = rt_ipow(xi, (int)si);          /* x^s em ℤ, exacto */
                    exactos_tot++;
                    if(fabs(a*(double)xs - 1.0) < 1e-12) exactos++;   /* x^{-s}·x^s = 1 */
                }
                if(x == 2.0 && s == 1.0)
                    printf("      %-8.1f %-8.1f %-15.10f %-15.10f %s\n", x, s, a, c, fabs(a-c) < 1e-12 ? "nao" : "SIM");
            }
        printf("      %ld pares, %ld diferem\n", casos, difs);
        printf("      e nos %ld pares com x e s INTEIROS a ponte fecha em Z: x^{-s}.x^s = 1"
               " em %ld\n", exactos_tot, exactos);
        e_o_mesmo = (difs == 0 && casos > 20 && exactos == exactos_tot && exactos_tot > 0);
        ok("o caractere do multiplicativo avaliado em x E' o do aditivo avaliado em ln x — o"
           " MESMO numero, e nao dois parecidos. E' a ponte x = e^u, e e' por isso que «Mellin"
           " e' Fourier no log»: nao sao duas transformadas, e' uma vista em duas cartas. As"
           " duas coordenadas do mesmo objecto. E ha' um caso EXACTO que a torna medicao e"
           " nao uma comparacao entre duas rotinas da libm: com s INTEIRO e x racional,"
           " x^{-s} e' 1/x^s e calcula-se em Z pela `rt_ipow` — a ponte fecha sem"
           " transcendental nenhum", e_o_mesmo);
    }

printf("\n§W2  A DILATACAO so' GIRA A FASE: o modulo NAO muda.\n\n");
    long so_fase = 0;
    {
        /* AQUI ESTA' O QUE ME FALTAVA. |lambda^{-i tau}| = 1: dilatar nao encolhe nem estica o
         * modulo — move a FASE, e so' isso. Aplicado: mudar a escala de uma coisa nao move as
         * outras. */
        long mau = 0, casos = 0, fases_distintas = 0;
        double fase_ant = -99;
        printf("      lambda   |lambda^{-i tau}|   arg = -tau ln(lambda)\n");
        for(double lam = 1.2; lam <= 3.0; lam += 0.3){
            long tau = 2;                            /* é o inteiro 2: a vírgula só entrava
                                                      * para o compilador a tirar na promoção */
            /* O MÓDULO ESTAVA ESCRITO. `modulo = 1.0` e depois `|modulo − 1| > 1e-15` é a
             * constante comparada consigo própria — o comentário dizia «= 1, exacto» e o
             * código não o calculava. Agora calcula-se: λ^{-iτ} = cos(τ ln λ) − i·sin(τ ln λ),
             * e o módulo ao quadrado é cos² + sin². */
            double arg = -tau * log(lam);
            double re = cos(arg), im = sin(arg);
            double modulo2 = re*re + im*im;          /* |λ^{-iτ}|², e vale 1 */
            double modulo = modulo2;                 /* o quadrado é o que se compara */
            casos++;
            if(fabs(modulo2 - 1.0) > 1e-15) mau++;
            if(fabs(arg - fase_ant) > 1e-9) fases_distintas++;
            fase_ant = arg;
            if(lam < 1.9 && lam > 1.4)
                printf("      %-8.1f %-19.15f %-.10f\n", lam, modulo, arg);
        }
        printf("      %ld dilatacoes: modulo constante em %ld, e %ld fases DISTINTAS\n",
               casos, casos - mau, fases_distintas);
        /* as duas metades: o modulo tem de ser SEMPRE 1 e a fase tem de MUDAR. Se a fase
         * tambem nao mudasse, a dilatacao nao fazia nada; se o modulo mudasse, ela nao era
         * so' uma rotacao. */
        /* E O GUME, que faltava: com o expoente a ter parte REAL o módulo JÁ MUDA. É isso
         * que separa «só gira a fase» de «não faz nada» — sem esta metade, um módulo que
         * fosse sempre 1 seja qual for o expoente não dizia nada sobre a dilatação. */
        long muda_com_real = 0, casos_r = 0;
        for(double lam = 1.2; lam <= 3.0; lam += 0.3){
            double a = 0.5, tau = 2.0;               /* expoente −a − iτ: tem parte real */
            double mod2 = pow(lam, -2.0*a);          /* |λ^{-a-iτ}|² = λ^{-2a} */
            casos_r++;
            if(fabs(mod2 - 1.0) > 1e-9) muda_com_real++;
        }
        printf("      e o GUME: com parte REAL no expoente o modulo MUDA em %ld de %ld\n",
               muda_com_real, casos_r);
        so_fase = (mau == 0 && fases_distintas == casos && muda_com_real == casos_r);
        ok("a DILATACAO so' gira a FASE: o modulo e' 1 em todas e a fase muda em todas. E' isto"
           " que eu nao tinha lido, e e' o meu defeito de hoje escrito por extenso: MUDAR A"
           " ESCALA DE UMA COISA NAO MOVE AS OUTRAS. Numa tabela, a altura de uma celula e' uma"
           " dilatacao — e eu fazia-a empurrar as vizinhas, que e' tratar uma rotacao como uma"
           " translacao. As duas metades: se o modulo mudasse nao era rotacao; se a fase nao"
           " mudasse, a dilatacao nao fazia nada. E o modulo passou a ser CALCULADO —"
           " cos²(tau.ln lam) + sin²(tau.ln lam) — em vez de escrito: estava «modulo = 1.0»"
           " e depois comparado com 1, que e' a constante consigo propria. E o GUME: com"
           " parte REAL no expoente o modulo muda em todas, sem o que «so' gira a fase»"
           " valia por o modulo ser sempre um seja qual for o expoente", so_fase);
    }

printf("\n§W3  O BIDUAL: exp e log sao o par, e a volta fecha nos DOIS sentidos.\n\n");
    long bidual = 0;
    {
        /* o par aditivo/multiplicativo: log leva o produto a' soma, exp leva a soma ao produto.
         * E' bidual porque fecha dos DOIS lados — e uma so' das voltas nao prova. */
        long ida = 0, volta = 0, casos = 0;
        for(double u = -2.0; u <= 2.0; u += 0.25){
            if(fabs(log(exp(u)) - u) > 1e-12) ida++;
            casos++;
        }
        for(double x = 0.25; x <= 5.0; x += 0.25)
            if(fabs(exp(log(x)) - x) > 1e-12) volta++;
        /* e a PROPRIEDADE que faz deles o par: log(a·b) = log a + log b — o produto vira soma */
        long prop = 0, np = 0;
        for(double a = 1.5; a <= 4.0; a += 0.5)
            for(double c = 1.5; c <= 4.0; c += 0.5){
                np++;
                if(fabs(log(a*c) - (log(a) + log(c))) > 1e-12) prop++;
            }
        /* E O MORFISMO TEM REALIZAÇÃO EXACTA, que é o que separa a lei da libm. As duas
         * primeiras linhas — log(exp(u)) = u e exp(log(x)) = x — são f∘f⁻¹, e medem a
         * implementação: a composição de um par com a sua inversa devolve o argumento por
         * definição do par. A TERCEIRA é a lei, e essa vale numa base INTEIRA sem uma
         * vírgula: com a = bᵐ e c = bⁿ,
         *
         *      log_b(a·c) = m + n = log_b(a) + log_b(c)
         *
         * e o log inteiro é a `rt_log_int` da reta.h, que RECUSA quando o argumento não é
         * potência da base — o que a torna medição e não adivinha. */
        long mz = 0, mz_tot = 0, mz_recusa = 0;
        for(long b2 = 2; b2 <= 5; b2++)
            for(int m2 = 0; m2 <= 6; m2++) for(int n2 = 0; n2 <= 6; n2++){
                long A2 = rt_ipow(b2, m2), C2 = rt_ipow(b2, n2);
                if(A2 <= 0 || C2 <= 0 || A2 > 1000000000L/C2) continue;
                int la, lc, lac;
                mz_tot++;
                if(rt_log_int(A2, b2, &la) && rt_log_int(C2, b2, &lc)
                   && rt_log_int(A2*C2, b2, &lac) && lac == la + lc) mz++;
                /* e o lado que recusa: A·C + 1 não é potência da base */
                if(!rt_log_int(A2*C2 + 1, b2, &lac)) mz_recusa++;
            }
        printf("      log(exp(u)) = u        em %ld de %ld    residuo %ld   (f∘f⁻¹: a definição)\n",
               casos-ida, casos, ida);
        printf("      exp(log(x)) = x        residuo %ld                    (idem)\n", volta);
        printf("      log(a·b) = log a + log b   em %ld de %ld    — o PRODUTO vira SOMA\n", np-prop, np);
        printf("      e em base INTEIRA, sem virgula: log_b(a.c) = log_b a + log_b c em %ld de\n"
               "      %ld, e a base RECUSA o que nao e' potencia dela em %ld\n",
               mz, mz_tot, mz_recusa);
        bidual = (ida == 0 && volta == 0 && prop == 0
                  && mz == mz_tot && mz_recusa > mz_tot/2);
        ok("exp e log fecham nos DOIS sentidos, e a propriedade que faz deles o par e' log(a·b)"
           " = log a + log b: o PRODUTO vira SOMA ao atravessar. E' o mesmo par que a constante"
           " de integracao atravessa na cosmologia — entra aditiva e sai multiplicativa — e o"
           " mesmo que separa o espacamento (soma) da escala (produto). Uma so' das voltas nao"
           " provava: uma funcao pode ter inversa a' esquerda e nao a' direita.\n"
           "  E AS DUAS PRIMEIRAS LINHAS NAO MEDEM A LEI: log(exp(u)) = u e exp(log(x)) = x"
           " sao f∘f⁻¹, a definicao do par relida, e o que elas testam e' a implementacao da"
           " libm. A LEI e' a terceira, e ela tem realizacao EXACTA: com a = b^m e c = b^n,"
           " log_b(a.c) = m + n em INTEIROS, pela `rt_log_int` — que RECUSA quando o"
           " argumento nao e' potencia da base, e e' essa recusa que a torna medicao", bidual);
    }

printf("\n§W4  E a APLICACAO: a altura e' MULTIPLICATIVA, a posicao e' ADITIVA.\n\n");
    long eixos = 0;
    {
        /* o layout tem os dois eixos, e sao DIFERENTES:
         *
         *    a POSICAO da fila     aditiva          y_{n+1} = y_n - altura
         *    a ALTURA da celula    multiplicativa   linhas = ceil(conteudo / largura)
         *
         * A altura sai de uma DIVISAO — quantas vezes a coluna cabe no conteudo — e isso e'
         * Mellin. A posicao sai de uma SOMA, e isso e' Fourier. Medi-las com a mesma regua e'
         * o erro. */
        long conteudo[5] = { 400, 900, 1500, 2100, 300 };   /* em milesimos de em */
        long larg = 500;
        long soma_alturas = 0, max_altura = 0;
        printf("      celula   conteudo   linhas = ceil(cont/larg)\n");
        for(int k = 0; k < 5; k++){
            long l = (conteudo[k] + larg - 1) / larg;         /* a DILATACAO: uma divisao */
            soma_alturas += l;
            if(l > max_altura) max_altura = l;
            printf("      %-8d %-10ld %ld\n", k+1, conteudo[k], l);
        }
        printf("      a fila gasta %ld linhas (a MAIS ALTA), e nao %ld (a soma)\n",
               max_altura, soma_alturas);
        /* e e' esta a distincao: a fila desce o MAXIMO e nao a SOMA. Somar as alturas seria
         * tratar a dilatacao como translacao — cada celula a empurrar a seguinte. */
        /* E NAO SE ESCREVE O VALOR: escrevi `soma_alturas == 13` de cabeca e a soma e' 12.
         * A assercao acusou-me a referencia escrita a' mao — outra vez. O que se afirma e' a
         * RELACAO, que e' o que interessa e nao depende de eu somar bem: o maximo e' menor que
         * a soma, e ha' pelo menos uma celula com mais de uma linha (senao os dois coincidiam
         * e a distincao nao tinha consequencia). */
        long com_varias = 0;
        for(int k = 0; k < 5; k++)
            if((conteudo[k] + larg - 1) / larg > 1) com_varias++;
        eixos = (max_altura < soma_alturas && com_varias > 0 && max_altura > 1);
        ok("a altura de uma celula sai de uma DIVISAO — quantas vezes a coluna cabe no conteudo,"
           " que e' a dilatacao de Mellin — e a posicao da fila sai de uma SOMA, que e' a"
           " translacao de Fourier. E a fila desce O MAXIMO das alturas, nao a SOMA: somar seria"
           " cada celula a empurrar a seguinte, que e' tratar a dilatacao como translacao. E'"
           " exactamente o que eu fazia — cada `&` somava uma entrelinha", eixos);
    }

printf("\n§W5  O CONTROLO: medir a dilatacao com a regua da translacao ACUMULA.\n\n");
    {
        /* somar uma entrelinha por celula, quando a celula gasta N: o erro e' (N-1) por celula
         * e SOMA ao longo da fila. E' o meu defeito, contado. */
        /* E O CONTROLO TEM DE MEDIR O MECANISMO, e nao um caso que coincida. A primeira
         * versao somava 1 por celula e comparava com o maximo — e com 5 celulas de altura
         * maxima 5 os dois davam 5. COINCIDIRAM POR ACASO, e a assercao acusou.
         *
         * O que se mede e' a POSICAO DE CADA CELULA: pelo eixo certo todas comecam no topo da
         * fila; pelo meu, cada uma subia UMA entrelinha da posicao onde a anterior acabou. Com
         * uma celula de varias linhas, as posicoes DIVERGEM — e e' isso que se ve' na pagina. */
        long conteudo[4] = { 400, 1500, 400, 900 };
        long larg = 500, topo = 100, certo = 0, meu = 0, divergem = 0;
        long pos_certa = topo, pos_minha = topo;
        printf("      celula   linhas   pelo eixo certo   pelo meu (soma)\n");
        for(int k = 0; k < 4; k++){
            long l = (conteudo[k] + larg - 1) / larg;
            if(l > certo) certo = l;
            printf("      %-8d %-8ld %-17ld %ld\n", k+1, l, pos_certa, pos_minha);
            if(pos_certa != pos_minha) divergem++;
            /* o certo: a celula seguinte comeca NO TOPO DA FILA, sempre */
            /* o meu: descia l e subia 1 — logo acumulava (l-1) */
            pos_minha -= (l - 1);
        }
        meu = pos_minha;
        long desvio = pos_certa - pos_minha;
        printf("      as posicoes divergem em %ld de 4 celulas\n", divergem);
        printf("      o desvio e' %ld linha(s) — e ACUMULA a cada fila\n", desvio < 0 ? -desvio : desvio);
        printf("      e' o mesmo caos das palavras: o residuo pequeno propaga-se\n");
        ok("pelo eixo certo todas as celulas comecam NO TOPO DA FILA; pelo meu, cada uma subia"
           " uma entrelinha da posicao onde a anterior acabou — e as posicoes DIVERGEM assim que"
           " uma celula gasta mais de uma linha. E' o que se ve' na pagina: uma celula a pular"
           " linha para dar espaco a' outra. E o controlo teve de ser refeito porque o primeiro"
           " COINCIDIU POR ACASO — 5 celulas de altura maxima 5 davam o mesmo dos dois lados, e"
           " um controlo que coincide nao controla nada", divergem > 0 && desvio != 0);
    }

    {
        unsigned char v[200];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,1|dourada: Fourier no aditivo, Mellin no multiplicativo, e a ponte e' x = e^u");
        gravar(&b, "corpo/dourada", v, m);
    }

    fechar(&b);
printf("\n=== O BIDUAL DA DOURADA ====================================================\n");
printf("  FOURIER    o grupo ADITIVO         (R,+)      a translacao    u -> u + c\n");
printf("  MELLIN     o grupo MULTIPLICATIVO  (R>0,x)    a dilatacao     x -> lambda x\n");
printf("  e a ponte e' x = e^u: «Mellin E' Fourier no log».\n\n");
printf("  E O QUE ELE RESOLVE: A DILATACAO SO' GIRA A FASE. |lambda^{-i tau}| = 1 — o modulo\n");
printf("  nao muda. MUDAR A ESCALA DE UMA COISA NAO MOVE AS OUTRAS.\n\n");
printf("  Numa tabela: a altura da celula e' uma DILATACAO (quantas vezes a coluna cabe no\n");
printf("  conteudo) e a posicao da fila e' uma TRANSLACAO. Eu media a dilatacao com a regua\n");
printf("  da translacao — cada `&` somava UMA entrelinha — e dai uma celula «pular linha para\n");
printf("  dar espaco a outra». Uma dilatacao nao empurra ninguem: gira a fase.\n\n");
printf("  E A FILA DESCE O MAXIMO DAS ALTURAS, NAO A SOMA. Somar seria cada celula a empurrar\n");
printf("  a seguinte, e o desvio ACUMULA a cada fila — o mesmo caos das palavras.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — e os dois eixos ficam separados.\n\n");
    return 0;
}
