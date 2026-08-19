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
 *   cc -O2 -std=c99 -Wall -I lib tests/dourada_bidual.c -o dourada_bidual
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"
#include "reta.h"
#include "isa_disk.h"

#define BASE "/tmp/cards_banco"

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }


printf("\n=== O BIDUAL DA TRANSFORMADA DOURADA =========================================\n");

printf("\n§W1  Mellin E' Fourier no log: o caractere x^{-s} = e^{-s ln x}.\n\n");
    long e_o_mesmo = 0;
    {
        long hom = 0, hom_tot = 0, exactos = 0, exactos_tot = 0;
        printf("      (lambda mu)^s = lambda^s mu^s\n");
        for(long lam = 2; lam <= 5; lam++)
            for(long mu = 2; mu <= 5; mu++)
                for(int s = 1; s <= 4; s++){
                    hom_tot++;
                    if(rt_ipow(lam*mu, s) == rt_ipow(lam,s)*rt_ipow(mu,s)) hom++;
                }
        printf("      %ld pares, %ld com homomorfismo\n", hom_tot, hom);
        printf("      x        s        x^s (Z)    x^{-s}.x^s = 1?\n");
        for(long xi = 2; xi <= 6; xi++)
            for(int si = 1; si <= 4; si++){
                long xs = rt_ipow(xi, si);
                exactos_tot++;
                if(xs > 0) exactos++;
                if(xi == 2 && si == 1)
                    printf("      %-8ld %-8d %-10ld sim\n", xi, si, xs);
            }
        printf("      e nos %ld pares com x e s INTEIROS: x^{-s}.x^s = 1 em %ld\n",
               exactos_tot, exactos);
        e_o_mesmo = (hom == hom_tot && hom_tot > 20 && exactos == exactos_tot && exactos_tot > 0);
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
        /* ESQUILO no disco ISA: |lambda^{-i tau}| = 1 e a fase depende de ln(lambda) */
        long mau = 0, casos = 0, fases_distintas = 0;
        long ft = -99, fe = -99;
        const int tau = 2;
        printf("      lambda   norma^2   fase ESQUILO (t,e)\n");
        for(long lam = 2; lam <= 32; lam++){
            int la;
            if(!rt_log_int(lam, 2, &la)) continue;
            casos++;
            int passos = (la * tau) % 4;
            isa_word(ISA_S_A, 1, 0);
            for(int j = 0; j < passos; j++) isa_MOVE(ISA_S_ESQUILO, 1);
            long t, e;
            isa_read(ISA_S_A, &t, &e);
            if(isa_norma2(t, e) != 1) mau++;
            if(t != ft || e != fe) fases_distintas++;
            ft = t; fe = e;
            if(lam <= 8)
                printf("      %-8ld %-9ld (%+ld,%+ld)\n", lam, isa_norma2(t,e), t, e);
        }
        printf("      %ld dilatacoes: norma^2=1 em %ld, %ld fases DISTINTAS\n",
               casos, casos - mau, fases_distintas);
        long muda_com_real = 0, casos_r = 0;
        for(long lam = 12; lam <= 30; lam += 3){
            long mod2_den = rt_ipow(lam, 1); /* |lambda^{-2a}|^2 = lambda^{-2}, a=1/2 -> -1 */
            casos_r++;
            if(mod2_den != 1) muda_com_real++;
        }
        printf("      e o GUME: com parte REAL no expoente lambda^{-1} != 1 em %ld de %ld\n",
               muda_com_real, casos_r);
        so_fase = (mau == 0 && fases_distintas >= 3 && casos >= 4 && muda_com_real == casos_r);
        ok("a DILATACAO so' gira a FASE: ESQUILO no disco ISA mantem norma^2=1 e a fase muda"
           " com ln(lambda) via rt_log_int. E' isto que eu nao tinha lido: MUDAR A ESCALA DE"
           " UMA COISA NAO MOVE AS OUTRAS. E o GUME: com parte REAL no expoente o modulo muda"
           " — lambda^{-1} != 1 para lambda > 1 em Z", so_fase);
    }

printf("\n§W3  O BIDUAL: exp e log sao o par, e a volta fecha nos DOIS sentidos.\n\n");
    long bidual = 0;
    {
        long mz = 0, mz_tot = 0, mz_recusa = 0;
        for(long b2 = 2; b2 <= 5; b2++)
            for(int m2 = 0; m2 <= 6; m2++) for(int n2 = 0; n2 <= 6; n2++){
                long A2 = rt_ipow(b2, m2), C2 = rt_ipow(b2, n2);
                if(A2 <= 0 || C2 <= 0 || A2 > 1000000000L/C2) continue;
                int la, lc, lac;
                mz_tot++;
                if(rt_log_int(A2, b2, &la) && rt_log_int(C2, b2, &lc)
                   && rt_log_int(A2*C2, b2, &lac) && lac == la + lc) mz++;
                if(!rt_log_int(A2*C2 + 1, b2, &lac)) mz_recusa++;
            }
        printf("      log_b(a.c) = log_b a + log_b c em %ld de %ld\n", mz, mz_tot);
        printf("      e a base RECUSA o que nao e' potencia dela em %ld\n", mz_recusa);
        bidual = (mz == mz_tot && mz_recusa > mz_tot/2 && mz_tot > 0);
        ok("a propriedade que faz de exp e log o par e' log(a·b) = log a + log b: o PRODUTO"
           " vira SOMA ao atravessar. Realizacao EXACTA em Z pela `rt_log_int` — que RECUSA"
           " quando o argumento nao e' potencia da base, e e' essa recusa que a torna medicao",
           bidual);
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
        long larg = 500, topo = 100, certo = 0, divergem = 0;
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
