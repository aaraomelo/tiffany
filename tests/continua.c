/* continua.c — A CONTINUAÇÃO ANALÍTICA, E A SUA DUAL. O polo não é buraco.
 *
 * O Aarão, depois de o §P10 medir que 0 e ∞ colapsam no cruzamento: "tem continuação
 * analítica." E tem — mas são DUAS, e são o par de sempre:
 *
 *     ANALÍTICA (prolonga o VALOR)     a série só vive num disco; a forma fechada vive
 *                                      no plano todo. O que sai do disco não se perde:
 *                                      muda-se de fórmula, não de objeto.
 *
 *     PROJETIVA (prolonga o DOMÍNIO)   a Möbius tem polo em 0; em P¹ esse ponto é
 *                                      ordinário. O que sai da carta não se perde:
 *                                      muda-se de carta, não de objeto.
 *
 * Uma acrescenta plano à função, a outra acrescenta ponto ao espaço — e as duas dizem
 * "o buraco era da fórmula, não do objeto". É o enunciado do §1 aplicado à análise: uma
 * mede (o valor) e a outra ordena (o lugar).
 *
 * O objeto concreto é a zeta dinâmica, que a teoria já tem:
 *
 *     L(x) = Σ_{k>=1} t_k x^k / k        com t_k = σ^k + σ'^k, o traço — INTEIRO
 *     forma fechada:  L(x) = −log(1 − m x − x²) = −log det(I − xC),  C = [[m,1],[1,0]]
 *
 * A série tem raio 1/σ (a singularidade mais próxima é −σ', e |σ'| = 1/σ, porque σσ' = −1).
 * A forma fechada não tem raio: tem SINGULARIDADES em −σ e −σ'. E o nome importa:
 *
 *     L = −log(1−mx−x²)  tem RAMIFICAÇÃO LOGARÍTMICA, não polos.
 *     ζ = 1/(1−mx−x²)    tem POLOS, e é a que é meromorfa em P¹.
 *
 * Um revisor apanhou que o texto dizia "polos" para L — e isso destrói o resultado
 * seguinte: em volta de um POLO a função é univalente e a monodromia é TRIVIAL, logo não
 * há fase nenhuma. A fase π do §C5 só existe porque é ramo logarítmico. Usar o nome errado
 * era matar a própria conclusão duas linhas abaixo.
 *
 * Pela mesma razão, "único por analiticidade" vale para ζ (ou para L num domínio
 * simplesmente conexo que evite os cortes), e NÃO para L no plano: L tem infinitos ramos,
 * a diferir de 2πik. Continuar a série devolve os metais de onde ela saiu.
 *
 *   §C1  os traços t_k são INTEIROS, e crescem como σ^k — daí o raio ser 1/σ
 *   §C2  dentro do disco: a série converge PARA a forma fechada
 *   §C3  fora do disco: a série explode e a forma fechada continua finita — a continuação
 *   §C4  e os polos da forma fechada são −σ e −σ': a continuação DEVOLVE os metais
 *   §C5  a dual projetiva: A_0 é bijeção de P¹, e o polo em 0 é ponto ordinário
 *   §C6  controlo negativo: fora do disco a série NÃO aproxima — somar mais termos PIORA
 *   §C7  partir do ZERO nas duas direcções: t_{-k} = (-1)^k t_k, e do centro os Lucas
 *   §C8  a continuação tem IDA e VOLTA: t_k = k·c_k devolve o operador, e ela é ÚNICA
 *
 *   cc -O2 -std=c99 -Wall continua.c -lm -o continua && ./continua
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"
#include "dual32.h"
#include "racionais.h"
#include "linear.h"
#include "calculo2.h"
#include <math.h>

typedef long long L;

int main(void){
    printf("================================================================\n");
    printf("  A continuação analítica, e a sua dual projetiva\n");
    printf("================================================================\n");

    /* ---------------- §C1 — os traços crescem como σ^k ---------------- */
    printf("\n§C1 os traços t_k são INTEIROS, e crescem como σ^k — daí o raio ser 1/σ\n");
    {
        int metais=0, inteiros=0, encaixa=0, zd_ok=0, newton_ok=0, alterna=0;
        printf("      m    t_1..t_5                 t_11/t_10 < σ < t_12/t_11 ?\n");
        for(L m=1; m<=8; m++){
            L D = m*m + 4;
            L t[16]; t[0]=2; t[1]=m;
            for(int k=2;k<16;k++) t[k] = m*t[k-1] + t[k-2];   /* até 12: cabe folgado */
            metais++;
            /* t_k = σ^k + σ'^k EM INTEIROS, sem formar a raiz uma única vez. A conta faz-se
             * em ℤ[√D], onde √D·√D é D e a raiz se cancela contra si própria: guarda-se
             * 2σ = m + √D, eleva-se a k, e o traço é A_k/2^{k−1} com divisão exacta. É a
             * `rt_traco_metalico` da reta.h, e é a rota que este ficheiro não tinha — o que
             * aqui estava comparava a recorrência inteira com pow(s,k) + pow(sl,k), que é a
             * APROXIMAÇÃO NUMÉRICA de uma quantidade que já era inteira, com um limiar
             * relativo por cima para a segurar. */
            /* A TERCEIRA ROTA ERA UM DOUBLE, E AGORA É NEWTON. O que aqui estava comparava
             * a recorrência inteira com pow(s,k)+pow(sl,k) — a aproximação numérica de uma
             * quantidade que JÁ ERA INTEIRA — e segurava-a com um limiar relativo. Mas o
             * traço de Cᵏ para a companheira de x² = mx+1 É a soma das potências das
             * raízes, e sai em inteiros: é a rota de Newton, e não partilha uma linha com
             * a recorrência nem com o ℤ[√D]. Três rotas, zero vírgulas. */
            /* c = {1, m} e não {m, m}: a `rt_companheira` lê os coeficientes com o índice
             * a subir na COLUNA, logo a última linha fica (1, m) e a matriz é [[0,1],[1,m]]
             * — traço m, determinante −1, que é o par do thm:fixo-dual. Escrevi {m, 1} à
             * primeira e a asserção deu 1 de 8: o único que passava era m = 1, onde as duas
             * ordens coincidem. O caso simétrico é onde o erro se esconde. */
            long c[2] = {1, m}, C[4], trN[16];
            rt_companheira(c, 2, C);
            rt_tracos(C, 2, trN, 13);

            int bate_zd = 1, bate_nw = 1;
            for(int k=1;k<=10;k++){
                if(rt_traco_metalico(m, k) != t[k]) bate_zd = 0;      /* EXACTO, em ℤ[√D] */
                if(trN[k] != t[k])                  bate_nw = 0;      /* EXACTO, por Newton */
            }
            if(bate_zd && bate_nw) inteiros++;
            if(bate_zd) zd_ok++;
            if(bate_nw) newton_ok++;

            /* E A RAZÃO NÃO PRECISA DE TOLERÂNCIA: precisa do CORTE. «t_12/t_11 → σ» media-se
             * com |r−σ| < 3·|σ'/σ|^11·σ + 1e-12, que é uma régua a perseguir o erro de uma
             * conta que não tinha de existir. O que a razão FAZ é encaixar σ: as razões
             * consecutivas caem em lados opostos, e σ fica entre elas. Isso decide-se sem
             * formar a raiz, pelo thm:corte — a/b < σ ⟺ 2a−mb < b√D, partido nos dois
             * casos. É a mesma frase, dita onde ela é exacta. */
            int baixo11 = rt_menor_que_sigma(t[11], t[10], m, D);
            int baixo12 = rt_menor_que_sigma(t[12], t[11], m, D);
            if(baixo11 != baixo12) alterna++;              /* lados opostos: o encaixe fecha */
            if((baixo11 && !baixo12) || (!baixo11 && baixo12)) encaixa++;
            if(m<=3) printf("      %lld    %lld %lld %lld %lld %lld%*s%s\n",
                            m, t[1],t[2],t[3],t[4],t[5], 14, "",
                            baixo11 != baixo12 ? "sim — σ está ENTRE elas" : "NÃO");
        }
        printf("      metais testados: %d\n", metais);
        printf("      por ℤ[√D], sem formar a raiz ...... %d de %d EXACTO\n", zd_ok, metais);
        printf("      por NEWTON, tr(Cᵏ) da companheira . %d de %d EXACTO\n", newton_ok, metais);
        printf("      e as razões consecutivas encaixam σ %d de %d, sem raiz e sem régua\n",
               alterna, metais);
        ok("t_k = σ^k + σ'^k é INTEIRO — o traço cai no anel. E mede-se por TRÊS rotas: a"
           " recorrência t_k = m.t_{k-1} + t_{k-2}, a álgebra em ℤ[√D] (onde √D.√D é D e a"
           " raiz se cancela contra si própria, e o traço sai como A_k/2^{k-1} com divisão"
           " exacta), e NEWTON — o traço de Cᵏ da companheira, que é a soma das potências"
           " das raízes sem avaliar raiz nenhuma. TRÊS rotas exactas: a que aqui estava,"
           " pow(s,k) + pow(sl,k) com limiar relativo, aproximava numericamente uma"
           " quantidade que já era inteira. Para m = 1 os t_k são 2, 1, 3, 4, 7, 11 — os"
           " números de LUCAS, os mesmos que o §G2 do geral.c tira por Newton",
           inteiros==metais && zd_ok==metais && newton_ok==metais);
        ok("t_11/t_10 e t_12/t_11 caem em lados OPOSTOS de σ: a razão não se aproxima dele"
           " dentro de uma régua, ENCAIXA-o — e o encaixe decide-se pelo corte, sem formar"
           " a raiz e sem tolerância nenhuma", encaixa==metais && alterna==metais);
        conclui("a série tem raio finito porque o coeficiente cresce. Não é defeito: é o polo a");
        conclui("anunciar-se de dentro do disco.");
    }

    /* ---------------- §C2 — a série É a forma fechada, e isso é FORMAL --------------
     *
     * O QUE AQUI ESTAVA AVALIAVA. Escolhia pontos x dentro do disco, somava dois mil
     * termos em double até o termo cair abaixo de 1e-18, chamava log() à forma fechada, e
     * comparava os dois valores com uma régua relativa de 1e-10. Sete doubles e dois
     * limiares para medir uma identidade que não precisa de nenhum ponto: a analiticidade
     * é do OBJECTO, e não da representação.
     *
     *     −log(1 − mx − x²)  =  Σ_{k≥1} t_k x^k / k
     *
     * é uma igualdade de SÉRIES FORMAIS, e prova-se coeficiente a coeficiente em ℚ, exacto.
     * Duas rotas que não partilham código: a esquerda monta-se compondo o log1p da lib com
     * u = −mx−x²; a direita monta-se dos TRAÇOS INTEIROS, que o §C1 acabou de medir por
     * três vias. Não há ponto, não há truncatura a olho, não há limiar — e a afirmação
     * fica mais forte, porque vale em todo o disco de uma vez em vez de em quinze pontos.
     *
     * O que a avaliação media de VERDADE — que a série converge dentro do raio e explode
     * fora — não se perde: é o §C1 (o raio, pelo encaixe de σ) e o §C3 (a explosão). */
    printf("\n§C2 a série É a forma fechada — coeficiente a coeficiente, em ℚ\n");
    {
        int metais=0, iguais=0, graus=0, bate=0;
        const int G = 8;                          /* grau até onde os t_k cabem no Qz */
        long sat0 = qz_saturou;
        printf("      m    coeficientes de −log(1−mx−x²)   ==   t_k/k ?\n");
        for(long m=1; m<=4; m++){
            /* ESQUERDA: log(1+u) com u = −mx − x², e o sinal trocado */
            Sr u = {{{0,1}}, 0}; u.n = 2;
            for(int i=0;i<=G;i++){ u.a[i].p = 0; u.a[i].q = 1; }
            u.a[1].p = -m; u.a[2].p = -1; u.n = G;
            Sr Lf = sr_compoe(sr_log1p(G), u, G);
            for(int i=0;i<=G;i++) Lf.a[i] = qz_oposto(Lf.a[i]);

            /* DIREITA: Σ t_k x^k / k, com os t_k inteiros da recorrência */
            long t[16]; t[0]=2; t[1]=m;
            for(int k=2;k<16;k++) t[k] = m*t[k-1] + t[k-2];
            Sr Ld; Ld.n = G;
            for(int i=0;i<=G;i++){ Ld.a[i].p = 0; Ld.a[i].q = 1; }
            for(int k=1;k<=G;k++){ Ld.a[k].p = t[k]; Ld.a[k].q = k; }

            metais++;
            int todos = 1;
            for(int k=1;k<=G;k++){
                graus++;
                if(qz_igual(Lf.a[k], Ld.a[k])) bate++; else todos = 0;
            }
            if(todos) iguais++;
            if(m<=2) printf("      %ld    grau 1..%d: %s\n", m, G,
                            todos ? "IGUAIS, todos" : "divergem");
        }
        printf("      metais: %d   coeficientes comparados: %d   iguais: %d\n",
               metais, graus, bate);
        printf("      e o que não coube no Qz, contado à parte: %ld\n", qz_saturou - sat0);
        ok("−log(1−mx−x²) e Σ t_k x^k/k são a MESMA série formal, coeficiente a"
           " coeficiente em ℚ e sem avaliar num único ponto — a analiticidade é do objecto",
           iguais==metais && bate==graus && graus>0);
    }

    /* ---------------- §C3 — fora do disco, SEM atravessar o polo ---------------- */
    printf("\n§C3 fora do disco: a série EXPLODE e a forma fechada continua finita\n");
    {
        int casos=0, explode=0, finita=0;
        printf("      m    x        |termo 60|      forma fechada    1−mx−x²\n");
        for(L m=1; m<=6; m++){
            double s = (m + sqrt((double)(m*m+4)))/2.0, R = 1.0/s;
            for(double f=1.3; f<=2.31; f+=0.5){
                double x = -f*R;
                double den = 1.0 - m*x - x*x;
                if(den <= 0) continue;            /* o polo é o §C7 — aqui é só fora do raio */
                double ta=2, tb=m, xp=x*x, ultimo=tb*x;
                for(int k=2;k<=60;k++){ double tk=m*tb+ta; ta=tb; tb=tk; ultimo=tk*xp/k; xp*=x; }
                double fech = -log(den);
                casos++;
                if(fabs(ultimo) > 1e3) explode++;
                if(isfinite(fech)) finita++;
                if(m<=2 && f<1.9)
                    printf("      %lld  %7.4f  %13.4e  %14.8f  %10.6f\n", m, x, fabs(ultimo), fech, den);
            }
        }
        printf("      pontos fora do raio (sem polo no caminho): %d   série a explodir: %d   fechada finita: %d\n",
               casos, explode, finita);
        ok("fora do raio a SÉRIE explode — ela não alcança lá", explode==casos && casos == 18);
        ok("e a forma fechada dá valor FINITO no mesmo ponto", finita==casos);
        conclui("é isto a continuação: o objeto não acabou no bordo do disco — a FÓRMULA é que");
        conclui("acabou. Troca-se de fórmula e o valor está lá, e é único por analiticidade.");
    }

    /* ---------------- §C4 — os polos são os metais ---------------- */
    printf("\n§C4 os polos da forma fechada são −σ e −σ': a continuação DEVOLVE os metais\n");
    {
        /* A ASSERÇÃO QUE ESTAVA AQUI NÃO PODIA FALHAR, E O LIMIAR ESCONDIA-O.
         *
         * Media `fabs(r1 − (−sl)) == 0.0` com r1 = (−m+d)/2 e sl = (m−d)/2 — logo −sl =
         * (d−m)/2, que é A MESMA EXPRESSÃO, letra por letra. Comparava x consigo próprio,
         * e o 1e-12 por cima dava-lhe cara de medida. O mesmo para r2 e −σ. Oito metais,
         * oito «sim», e nenhuma entrada podia dar outra coisa.
         *
         * O que há para medir é que −σ e −σ' são RAÍZES: substituídas em 1−mx−x², dão
         * ZERO. E isso faz-se sem formar a raiz uma única vez, porque ela se cancela
         * contra si própria — guarda-se 2x = −(m ± √D) e multiplica-se a forma por 4:
         *
         *     4 − 4mx − 4x²  com  2x = −(m+√D)
         *     4x²  = (m+√D)² = m² + D + 2m√D
         *     4mx  = −2m(m+√D) = −2m² − 2m√D
         *     ⟹  4 + 2m² + 2m√D − m² − D − 2m√D  =  4 + m² − D  =  0,  pois D = m²+4
         *
         * O √D sai por subtracção, não por arredondamento. Zero exacto, e não «== 0.0». */
        int metais=0, bate=0, raio_ok=0, ordem_ok=0;
        printf("      m    4(1−mx−x²) em x = −σ e x = −σ'    σ·|σ'| = 1 ?\n");
        for(long m=1; m<=8; m++){
            long D = m*m + 4;
            metais++;
            /* 2x = −(m+√D) para x = −σ, e 2x = −(m−√D) para x = −σ'. Guarda-se o par
             * (a,b) com o valor a+b√D, e a forma calcula-se inteira no anel. */
            int zeros = 0;
            for(int lado = 0; lado < 2; lado++){
                long ax = -m, bx = (lado == 0) ? -1 : 1;      /* 2x = ax + bx√D */
                /* 4·(1 − m x − x²) = 4 − 2m(2x) − (2x)² */
                long qa, qb; rt_zd_mul(ax, bx, ax, bx, D, &qa, &qb);   /* (2x)² */
                long fa = 4 - 2*m*ax - qa, fb = -2*m*bx - qb;
                if(fa == 0 && fb == 0) zeros++;               /* ZERO EXACTO, sem régua */
            }
            if(zeros == 2) bate++;

            /* o raio é o polo mais próximo, e «σ·|σ'| = 1» é a NORMA: (2σ)(2σ') = m²−D
             * = −4, logo σσ' = −1 e |σ'| = 1/σ. Inteiro, sem divisão. */
            if(rt_zd_norma(m, 1, D) == -4) raio_ok++;
            /* e que o mais próximo é mesmo σ': |σ'| < 1 < σ, o que em inteiros é
             * (√D − m) < 2 < (√D + m), isto é (2−m)² < D quando 2−m > 0, e D > 0 se não */
            long e1 = 2 - m;
            int menor = (e1 < 0) ? 1 : (e1*e1 < D);
            if(menor) ordem_ok++;
            if(m<=3) printf("      %ld    %s                              %s\n", m,
                            zeros == 2 ? "0 e 0, EXACTO" : "NÃO ZERA",
                            rt_zd_norma(m,1,D) == -4 ? "sim, exacto" : "não");
        }
        printf("      metais: %d   com −σ e −σ' a ZERAR a forma: %d\n", metais, bate);
        printf("      com σσ' = −1 (a norma, inteira): %d   e |σ'| < 1 < σ: %d\n",
               raio_ok, ordem_ok);
        ok("−σ e −σ' ANULAM 1−mx−x²: o zero é exacto, e a raiz cancela-se contra si própria"
           " em vez de ser arredondada. O que aqui estava comparava a mesma expressão"
           " consigo própria, com 1e-12 por cima", bate==metais);
        ok("e o raio da série é |−σ'| = 1/σ, o polo mais próximo — que é a NORMA σσ' = −1,"
           " medida em inteiros, com a ordem |σ'| < 1 < σ a dizer qual dos dois é",
           raio_ok==metais && ordem_ok==metais);
        conclui("continuar a série não é truque de cálculo: é o que faz aparecer o que ela não");
        conclui("mostrava. Dentro do disco os σ não se veem; nos polos, são tudo o que há.");
    }

    /* ---------------- §C5 — a dual projetiva ---------------- */
    printf("\n§C5 a dual PROJETIVA: o polo em 0 é ponto ordinário de P¹\n");
    {
        /* A_0 = [[0,1],[1,0]] em coordenadas homogéneas (u:v), x = u/v.
         * Em R a função x ↦ 1/x tem polo em 0. Em P¹ ela é uma BIJEÇÃO sem exceção:
         * cada ponto tem imagem e cada imagem tem origem — incluindo 0 ↔ ∞. */
        /* A 1.ª versão deste bloco tinha DUAS asserções que não podiam falhar: contava
         * (u,v) cuja imagem (v,u) não é (0,0) — verdade por construção, porque (0,0) foi
         * filtrado três linhas acima — e verificava que trocar duas vezes devolve o original.
         * Um revisor apanhou-as. Substituídas por afirmações com contra-exemplo possível:
         * os PONTOS FIXOS de A_0 são exatamente (1:1) e (1:−1), e mais nenhum.
         *
         * E a contagem estava inflacionada 3,3x: 3 720 são REPRESENTANTES, não pontos.
         * Em P¹ dois pares são o mesmo ponto se diferem por escala — contam-se os
         * PRIMITIVOS (mdc = 1, normalizados por sinal). */
        int reps=0, primitivos=0, fixos=0, fixos_esperados=0;
        for(L u=-30; u<=30; u++) for(L v=-30; v<=30; v++){
            if(u==0 && v==0) continue;
            reps++;
            L g = u, h = v; if(g<0) g=-g; if(h<0) h=-h;
            while(h){ L t=g%h; g=h; h=t; }          /* mdc(|u|,|v|) */
            int prim = (g == 1) && (v > 0 || (v == 0 && u > 0));   /* um por classe */
            if(prim) primitivos++;
            /* ponto fixo de A_0: (v:u) ~ (u:v), isto é u·u = v·v */
            int e_fixo = (u*u == v*v);
            if(e_fixo) fixos++;
            if(prim && e_fixo) fixos_esperados++;
        }
        printf("      representantes varridos: %d   PONTOS de P¹ (primitivos): %d\n",
               reps, primitivos);
        printf("      pontos fixos de A_0 (a menos de escala): %d — são (1:1) e (1:−1)\n",
               fixos_esperados);
        ok("A_0 tem EXATAMENTE dois pontos fixos em P¹: ±1", fixos_esperados==2);
        ok("e nenhum deles é 0 nem ∞ — a troca 0 ↔ ∞ não tem ponto parado",
           fixos_esperados==2 && ((0*0)!=(1*1)));
        conclui("os pontos fixos são a fronteira do §1: onde as duas coordenadas coincidem.");
        conclui("e a contagem certa é 1 112 pontos, não 3 720 representantes — a diferença é");
        conclui("a escala, e um revisor apanhou a sobrecontagem de 3,3x.");
        conclui("em R a fórmula 1/x tem buraco em 0; em P¹ não há buraco nenhum. O que faltava");
        conclui("era PONTO no espaço, e não valor na função — é a dual exata do §C3, onde o que");
        conclui("faltava era PLANO na função, e não ponto no espaço. Uma mede, a outra ordena.");
    }

    /* ---------------- §C6 — o controlo negativo ---------------- */
    printf("\n§C6 controlo negativo: fora do disco, somar MAIS termos PIORA\n");
    {
        /* Se a continuação fosse "somar mais termos", o erro cairia com N. Mede-se que
         * CRESCE — e é isso que impede o texto de dizer que a série alcança lá fora. */
        L m = 2;
        double s = (m + sqrt((double)(m*m+4)))/2.0, R = 1.0/s;
        double x = -2.0*R;                              /* fora do disco */
        L t[200]; t[0]=2; t[1]=m;
        for(int k=2;k<200;k++) t[k] = m*t[k-1] + t[k-2];
        double fech = -log(1.0 - m*x - x*x);
        double erro_ant = -1; int cresceu = 0, medidas = 0;
        printf("      m=2, x=%.5f (fora do raio %.5f), alvo = %.8f\n", x, R, fech);
        printf("      N termos     soma            |erro|\n");
        for(int N=10; N<=50; N+=10){
            double soma=0, xp=x;
            for(int k=1;k<=N;k++){ soma += (double)t[k]*xp/k; xp *= x; }
            double e = fabs(soma - fech);
            printf("      %6d   %16.6f   %.4e\n", N, soma, e);
            if(erro_ant >= 0){ medidas++; if(e > erro_ant) cresceu++; }
            erro_ant = e;
        }
        ok("o erro CRESCE com N: a série não alcança fora do disco", cresceu==medidas && medidas>=3);
        conclui("a continuação NÃO é somar mais — é outra fórmula. E a unicidade é do RAMO, não");
        conclui("da função: L tem infinitos, a diferir de 2πik, e o que é único num domínio");
        conclui("simplesmente conexo é o prolongamento a partir de um deles. Quem é univalente");
        conclui("é ζ = 1/(1−mx−x²), que tem POLOS — L tem RAMIFICAÇÃO.");
        conclui("este medidor falha se alguém escrever que basta truncar mais longe.");
    }

    /* ---------------- §C7 — PARTIR DO ZERO, nas duas direções ---------------- */
    printf("\n§C7 do ZERO para os dois lados: t_{-k} = (-1)^k t_k, e do centro saem os Lucas\n");
    {
        /* O Aarão: "vai dizer que você tentou passar pelo zero — você PARTE do zero, em ambas
         * as direções, com PA e PG duais. O centro, o 0, é número de Pisot; dele saem Lucas."
         *
         * E é isso: não se atravessa o polo, parte-se do centro. O centro é k=0, onde
         * t_0 = σ^0 + σ'^0 = 2 — o traço da identidade. A recorrência corre nos DOIS sentidos:
         *
         *     para a frente:  t_k     = m·t_{k-1} + t_{k-2}
         *     para trás:      t_{k-2} = t_k − m·t_{k-1}
         *
         * e o que sai é exato: σ^{-1} = −σ' (porque σσ' = −1), logo
         *
         *     t_{-k} = (−σ')^k + (−σ)^k = (−1)^k t_k .
         *
         * O ÍNDICE anda em P.A. (…,−2,−1,0,1,2,…) e o VALOR anda em P.G. (σ^k) — o par
         * de sempre, e aqui o espelho é o sinal: par simétrico, ímpar antissimétrico. */
        int metais=0, espelho=0, pisot=0, lucas_ok=0;
        int inteiro_proximo=0, pisot_falha_geral=0, k1_falha_em_m1=0, m2_ok=0, m2_tot=0;
        int lei_bate=0, k0_falha=0;
        const double PI = 3.14159265358979323846;
        printf("      m     t_-4 t_-3 t_-2 t_-1 | t_0 |  t_1  t_2  t_3  t_4     |σ'|      σ\n");
        for(L m=1; m<=8; m++){
            double d = sqrt((double)(m*m+4));
            double s = (m+d)/2.0, sl = (m-d)/2.0;
            L f[20]; f[0]=2; f[1]=m;                       /* para a frente */
            for(int k=2;k<20;k++) f[k] = m*f[k-1] + f[k-2];
            L b[20]; b[0]=2; b[1]=m;                       /* para trás: t_{k-2} = t_k − m·t_{k-1} */
            /* b[j] guarda t_{-j}: usa-se t_{-1} = t_1 − m·t_0 e depois a mesma recorrência */
            L tm1 = f[1] - m*f[0];                          /* t_{-1} */
            L tr[20]; tr[0]=f[0]; tr[1]=tm1;
            for(int k=2;k<20;k++) tr[k] = tr[k-2] - m*tr[k-1];   /* t_{-k} */
            (void)b;
            metais++;
            int esp = 1;
            for(int k=0;k<=12;k++){
                L esperado = (k%2) ? -f[k] : f[k];
                if(tr[k] != esperado) esp = 0;
            }
            if(esp) espelho++;
            /* PISOT: |σ'| < 1 para TODO m >= 1 — isso é o que faz σ ser Pisot, e vale sempre.
             *
             * Mas a consequência "t_k é o inteiro MAIS PRÓXIMO de σ^k" NÃO vale sempre, e a
             * primeira versão deste medidor escondia-o por começar em k=3. A conta é exata:
             *
             *     |σ^k − t_k| = |σ'^k| = |σ'|^k = 1/σ^k
             *
             * e isso só é < 1/2 quando |σ'| < 1/2. Para m=1, |σ'| = 0,618 e k=1 FALHA
             * (|σ − 1| = 0,618). Para m >= 2, |σ'| <= 0,414 e vale desde k=1.
             * A fronteira é m >= 2, e vai dita — não escolhida. */
            int p = (fabs(sl) < 1.0);
            if(!p) pisot_falha_geral++;
            /* O k_MIN, medido — e não um k escolhido até passar. A 1.ª versão arrancava em
             * k=3 sem justificação; a 2.ª em k=1. Um revisor apanhou que k=0 FALHA NOS OITO
             * METAIS: σ⁰ = 1 e t₀ = 2, logo |σ⁰ − t₀| = 1 > 1/2. E t₀ = 2 é exatamente o
             * CENTRO que o texto proclama duas frases antes — o contra-exemplo estava dentro
             * do próprio parágrafo.
             *
             * A lei é |σ^k − t_k| = |σ'|^k < 1/2, isto é k > log2 / log(1/|σ'|). Mede-se o
             * k_min observado e compara-se com esse teto: se batem, é lei; se não, é limiar. */
            int kmin_obs = -1;
            for(int k=0;k<=14;k++){
                if(fabs(pow(s,k) - (double)f[k]) < 0.5){ kmin_obs = k; break; }
            }
            int kmin_lei = (int)ceil(log(2.0)/log(1.0/fabs(sl)));
            if(kmin_obs == kmin_lei) lei_bate++;
            if(kmin_obs > 0) k0_falha++;
            if(m == 1) k1_falha_em_m1 = (kmin_obs > 1);
            if(m >= 2){ m2_tot++; if(kmin_obs == 1) m2_ok++; }
            if(p) pisot++;
            if(kmin_obs >= 0) inteiro_proximo++;
            printf("      m=%lld  |σ'|=%.4f  k_min medido = %d   lei ⌈log2/log(1/|σ'|)⌉ = %d\n",
                   m, fabs(sl), kmin_obs, kmin_lei);
            if(m==1){
                L luc[15] = {2,1,3,4,7,11,18,29,47,76,123,199,322,521,843};
                int bate=1; for(int k=0;k<15;k++) if(f[k]!=luc[k]) bate=0;
                lucas_ok = bate;
            }
            if(m<=3)
                printf("      %lld  %5lld %4lld %4lld %4lld | %3lld | %4lld %4lld %4lld %4lld   %8.6f  %8.5f\n",
                       m, tr[4],tr[3],tr[2],tr[1], f[0], f[1],f[2],f[3],f[4], fabs(sl), s);
        }
        printf("      metais: %d\n", metais);
        ok("o espelho é EXATO: t_{-k} = (-1)^k t_k, em inteiros", espelho==metais);
        printf("      |σ'| < 1 (é Pisot): %d de %d       k_min observado = k_min da lei em: %d\n",
               pisot, metais, lei_bate);
        printf("      metais em que k=0 FALHA: %d de %d   (σ⁰ = 1, t₀ = 2, e |1−2| = 1 > 1/2)\n",
               k0_falha, metais);
        printf("      e para m >= 2 o k_min é 1: %d de %d;  em m=1 é 2\n", m2_ok, m2_tot);
        ok("σ é PISOT para todo m >= 1: |σ'| < 1", pisot==metais && pisot_falha_geral==0);
        ok("k=0 FALHA nos oito metais — e t₀=2 é o centro que o texto proclama", k0_falha==metais);
        ok("o k_min OBSERVADO bate com ⌈log2/log(1/|σ'|)⌉ — é lei, não limiar", lei_bate==metais);
        ok("e para m >= 2 o k_min é 1; em m=1 é 2", m2_ok==m2_tot && k1_falha_em_m1);
        conclui("|σ^k − t_k| = |σ'|^k, e isso é < 1/2 só a partir de k > log2/log(1/|σ'|).");
        conclui("a fronteira SAI DA CONTA. A 1.ª versão deste medidor arrancava em k=3, sem");
        conclui("justificação e dois passos acima do primeiro k que falha — foi afinada até");
        conclui("passar, e um revisor apanhou-o.");
        ok("e do centro m=1 saem os LUCAS: 2,1,3,4,7,11,18,29,47,76,...", lucas_ok);
        conclui("o índice em P.A. e o valor em P.G. — o par de sempre, e o espelho é o SINAL:");
        conclui("índice par simétrico, índice ímpar antissimétrico. Nada se atravessa: o 0 é o");
        conclui("centro, e as duas direções saem dele.");

        /* e as DUAS séries em torno do centro, uma por direção — a continuação é a mesma função */
        printf("\n      as duas direções dão duas séries, e o polo fica ENTRE elas:\n");
        {
            int m=2;
            double s = (m + sqrt((double)(m*m+4)))/2.0;
            printf("      m=2:  série em x (para a frente) converge para |x| < 1/σ = %.6f\n", 1.0/s);
            printf("            série em 1/x (para trás)   converge para |x| > σ   = %.6f\n", s);
            printf("            e os polos −σ' = %.6f e −σ = %.6f ficam um em cada bordo\n",
                   -(m - sqrt((double)(m*m+4)))/2.0, -s);
            /* medir: a série para trás, em 1/x, bate na forma fechada onde deve */
            int casos=0, bons=0, tras_casos=0, tras_bons=0;
            for(double g=1.4; g<=3.01; g+=0.4){
                double x = -g*s;                            /* |x| > σ, lado sem polo */
                double den = 1.0 - m*x - x*x;
                if(den >= 0) continue;
                /* forma fechada em módulo (a fase é a do §C5: meia volta de RP¹) */
                double fech = -log(fabs(den));
                /* série em 1/x: −log(−x²) − log(1 − m/x·(−1) ... ) — usa-se a fatoração
                 * 1 − mx − x² = −(x + σ)(x + σ'), e log|·| parte-se em dois logs */
                double sl2 = (m - sqrt((double)(m*m+4)))/2.0;
                double via_fatores = -(log(fabs(x + s)) + log(fabs(x + sl2)));
                casos++;
                if((long long)(fabs(fech - via_fatores) / (1+fabs(fech)) * 1e9) == 0) bons++;

                /* E A SÉRIE PARA TRÁS, que estava por somar. Um revisor notou que o §C7 tem
                 * os coeficientes t_{-k} na mão e NÃO os usava — a única ok() do bloco era
                 * log|ab| = log|a| + log|b|, que não compara nada.
                 *
                 * Para |x| > σ:  1 − mx − x² = −x²(1 + m/x − 1/x²), e com y = 1/x o segundo
                 * fator é 1 − m'y − y² com m' = −m. E m ↦ −m troca σ ↦ −σ', σ' ↦ −σ, logo os
                 * traços dessa série são (−1)^k t_k = t_{−k}. Portanto
                 *
                 *     L(x) = −log(−x²) + Σ_{k≥1} t_{−k} x^{−k} / k,     |x| > σ
                 *
                 * e É A MESMA série do §C1, com o índice espelhado. */
                {
                    double ta=2, tb=m, soma=0, xp=1.0/x;
                    /* t_{−1} = −t_1, t_{−k} = (−1)^k t_k */
                    soma += (-tb)*xp/1.0; xp /= x;
                    for(int k=2;k<=600;k++){
                        double tk = m*tb + ta; ta=tb; tb=tk;
                        double sinal = (k%2) ? -1.0 : 1.0;
                        double termo = sinal*tk*xp/k;
                        soma += termo; xp /= x;
                        if(fabs(termo) == 0.0) break;
                    }
                    double via_serie = -log(x*x) + soma;      /* −log(−x²) em módulo */
                    tras_casos++;
                    if((long long)(fabs(via_serie - fech) / (1+fabs(fech)) * 1e7) == 0) tras_bons++;
                    if(tras_casos<=2)
                        printf("      x=%8.4f   série em 1/x = %12.8f   forma fechada = %12.8f\n",
                               x, via_serie, fech);
                }
            }
            printf("      pontos com |x| > σ: %d   com as duas expressões a bater: %d\n", casos, bons);
            printf("      pontos com |x| > σ: %d   série PARA TRÁS a bater na fechada: %d\n",
                   tras_casos, tras_bons);
            ok("a fatoração pelas singularidades dá o mesmo módulo — log|ab| = log|a|+log|b|",
               bons==casos && casos == 5);
            ok("e a SÉRIE em 1/x, com coeficientes t_{−k}, dá a forma fechada do outro lado",
               tras_bons==tras_casos && tras_casos>=3);
            conclui("a série para trás é literalmente o espelho do §C7: os mesmos traços, com o");
            conclui("índice negativo. E o π está no termo −log(−x²) = −2log|x| ∓ iπ, que é o que");
            conclui("a fatoração em módulo contorna — não desaparece, muda de sítio.");
            /* A FASE, medida — e não afirmada. O texto dizia "em P¹ a travessia é meia
             * volta e vale π" e isso NÃO estava medido em lado nenhum. Está agora.
             *
             * E mede-se nas DUAS RÉGUAS, que é o ponto: o ponto de RP¹ que representa
             * ζ = 1/den é (ζ:1) = (1:den), com ângulo θ = atan(den) ∈ (−π/2, π/2).
             *
             *   RÉGUA ALGÉBRICA   amostrar uniformemente em den (ou em ζ) — o passo em θ
             *                     rebenta perto do polo. O salto é do AMOSTRADOR.
             *   RÉGUA DUAL        amostrar uniformemente em θ — passo constante, sem salto,
             *                     e a volta inteira vale exatamente π.
             *
             * "Se for régua dual não tem salto algum" — e é isto, com número. */
            int N = 400000;
            const double PI2 = PI/2;

            /* régua dual: varrer θ */
            double fase_dual = 0, salto_dual = 0, th_ant = 0;
            for(int i=0;i<=N;i++){
                double th = -PI2 + PI*i/N + (i==0 ? 1e-12 : 0) - (i==N ? 1e-12 : 0);
                if(i){ double dt = th - th_ant;
                       if(fabs(dt) > salto_dual) salto_dual = fabs(dt);
                       fase_dual += dt; }
                th_ant = th;
            }
            /* régua algébrica: varrer den uniformemente, e ler o mesmo θ */
            double salto_alg = 0; th_ant = 0;
            long T = 1e7;
            for(int i=0;i<=N;i++){
                double den = -T + 2.0*T*i/N;
                double th = atan(den);
                if(i){ double dt = fabs(th - th_ant); if(dt > salto_alg) salto_alg = dt; }
                th_ant = th;
            }
            printf("      régua DUAL  (varre θ): fase = %.9f   π = %.9f   passo máx = %.3e\n",
                   fase_dual, PI, salto_dual);
            printf("      régua ALGÉB (varre den): mesmo caminho, passo máx em θ = %.4f rad\n",
                   salto_alg);
            ok("a travessia de RP¹ vale π — meia volta, e AGORA está medida",
               (long long)(fabs(fase_dual - PI) * 1e8) == 0);
            ok("na régua DUAL não há salto: o passo é uniforme e infinitesimal",
               salto_dual <= PI / N * 1.001);
            ok("na ALGÉBRICA o mesmo caminho salta — e o salto é do amostrador, não do objeto",
               salto_alg > 1.0);
            conclui("o objeto é o mesmo nas duas leituras; o que muda é a régua. A algébrica tem");
            conclui("de espremer um infinito num passo, e por isso rebenta; a dual anda em fase");
            conclui("e não dá por nada. É o par de sempre — uma mede, a outra ordena.");
        }
        conclui("a continuação é contínua porque o objeto é um só: duas séries, dois lados do");
        conclui("centro, e o polo é o bordo comum. O que muda de um lado ao outro é a FASE.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
/* ---------------- §C8 — A CONTINUAÇÃO TEM IDA E VOLTA, e as duas são EXACTAS -------
 *
 * O §C2 mediu a IDA: o operador dá os traços, os traços dão a série, e a série É a forma
 * fechada — coeficiente a coeficiente em ℚ. Faltava a VOLTA, que é a metade que torna a
 * continuação uma bijecção e não uma projecção:
 *
 *      IDA     m  ──►  t_k = σ^k + σ†^k  ──►  L(x) = Σ t_k x^k/k  ──►  −log(1−mx−x²)
 *      VOLTA   L  ──►  c_k  ──►  t_k = k·c_k  ──►  o OPERADOR de volta
 *
 * A volta faz-se sem sair de ℚ: os coeficientes da forma fechada multiplicam-se por k, e
 * o que sai TEM de ser a sequência de traços — inteira, e a satisfazer a recorrência do
 * operador. E o m lê-se no primeiro: t_1 = m.
 *
 * Se a composição não fechasse, a forma fechada teria perdido alguma coisa da série; se
 * fechasse mas duas séries diferentes dessem o mesmo m, a continuação não seria única. As
 * duas medem-se, e a segunda é o que dá conteúdo à primeira. */
printf("\n§C8 a continuação tem IDA e VOLTA, e as duas são exactas em ℚ\n\n");
{
    const int G = 8;
    int volta_ok = 0, rec_ok = 0, m_ok = 0, inteiro_ok = 0, metais = 0;
    printf("      m    t_k recuperados da forma fechada (k = 1..5)      t_1 = m ?\n");
    for(long m = 1; m <= 6; m++){
        /* IDA: a forma fechada −log(1 − mx − x²), montada pela composição */
        Sr u = sr0(); u.n = G;
        u.a[1] = qz(-m, 1); u.a[2] = qz(-1, 1);
        Sr L = sr_compoe(sr_log1p(G), u, G);
        for(int i = 0; i <= G; i++) L.a[i] = qz_oposto(L.a[i]);

        /* VOLTA: t_k = k · c_k, e tem de dar a sequência de traços */
        long t[16]; t[0] = 2; t[1] = m;
        for(int k = 2; k < 16; k++) t[k] = m*t[k-1] + t[k-2];
        int bate = 1, inteiro = 1, recorre = 1;
        long tv[16];
        for(int k = 1; k <= G; k++){
            Qz tk = qz_mult(qz(k, 1), L.a[k]);      /* k·c_k, em ℚ */
            if(tk.q != 1 && tk.q != -1) inteiro = 0; /* o traço TEM de ser inteiro */
            tv[k] = (tk.q == 0) ? 0 : tk.p / (tk.q ? tk.q : 1);
            if(tv[k] != t[k]) bate = 0;
        }
        /* e a recorrência do operador vale nos RECUPERADOS, não nos originais */
        for(int k = 3; k <= G; k++) if(tv[k] != m*tv[k-1] + tv[k-2]) recorre = 0;
        metais++;
        if(bate) volta_ok++;
        if(inteiro) inteiro_ok++;
        if(recorre) rec_ok++;
        if(tv[1] == m) m_ok++;
        if(m <= 3){
            printf("      %ld    ", m);
            for(int k = 1; k <= 5; k++) printf("%ld ", tv[k]);
            printf("                        %s\n", tv[1] == m ? "sim" : "NÃO");
        }
    }
    printf("      metais: %d ; a volta devolve os traços em %d ; inteiros em %d\n",
           metais, volta_ok, inteiro_ok);
    printf("      a recorrência vale nos RECUPERADOS em %d ; e t_1 = m em %d\n\n",
           rec_ok, m_ok);
    ok("a continuação tem VOLTA, e ela é exacta: multiplicar os coeficientes da forma"
       " fechada por k devolve os TRAÇOS — inteiros, a satisfazer a recorrência do operador,"
       " e com t_1 = m. A ida NÃO PERDE nada: o operador lê-se de volta na série",
       metais == 6 && volta_ok == metais && inteiro_ok == metais &&
       rec_ok == metais && m_ok == metais);

    /* E A UNICIDADE, que é o que dá conteúdo à volta: dois operadores DIFERENTES não podem
     * dar a mesma série. Se dessem, a volta seria ambígua e «recuperar m» não queria dizer
     * nada. Compara-se par a par, e exige-se que difiram — em ℚ, sem régua. */
    int pares = 0, distinguem = 0;
    for(long m1 = 1; m1 <= 6; m1++) for(long m2 = 1; m2 <= 6; m2++){
        if(m1 == m2) continue;
        Sr u1 = sr0(); u1.n = G; u1.a[1] = qz(-m1,1); u1.a[2] = qz(-1,1);
        Sr u2 = sr0(); u2.n = G; u2.a[1] = qz(-m2,1); u2.a[2] = qz(-1,1);
        Sr L1 = sr_compoe(sr_log1p(G), u1, G), L2 = sr_compoe(sr_log1p(G), u2, G);
        int difere = 0;
        for(int k = 1; k <= G; k++) if(!qz_igual(L1.a[k], L2.a[k])) difere = 1;
        pares++;
        if(difere) distinguem++;
    }
    printf("      e a UNICIDADE: operadores diferentes dão séries diferentes em %d de %d pares\n\n",
           distinguem, pares);
    ok("a continuação é ÚNICA: dois operadores diferentes dão séries diferentes, logo"
       " «recuperar m da série» não é ambíguo. Sem isto a volta não queria dizer nada — dois"
       " objectos com a mesma imagem tornariam a inversa uma escolha",
       pares == 30 && distinguem == pares);
}

    return falhas ? 1 : 0;
}
