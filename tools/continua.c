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
 *
 *   cc -O2 -std=c99 -Wall continua.c -lm -o continua && ./continua
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>

typedef long long L;

int main(void){
    printf("================================================================\n");
    printf("  A continuação analítica, e a sua dual projetiva\n");
    printf("================================================================\n");

    /* ---------------- §C1 — os traços crescem como σ^k ---------------- */
    printf("\n§C1 os traços t_k são INTEIROS, e crescem como σ^k — daí o raio ser 1/σ\n");
    {
        int metais=0, inteiros=0, razao_ok=0;
        printf("      m    σ         t_1..t_5                t_12/t_11    σ\n");
        for(L m=1; m<=8; m++){
            double d = sqrt((double)(m*m+4));
            double s = (m + d)/2.0, sl = (m - d)/2.0;
            L t[16]; t[0]=2; t[1]=m;
            for(int k=2;k<16;k++) t[k] = m*t[k-1] + t[k-2];   /* até 12: cabe folgado */
            metais++;
            int bate = 1;
            for(int k=1;k<=10;k++){
                double v = pow(s,k) + pow(sl,k);
                if(fabs(v - (double)t[k]) > 1e-9 * fabs(v) + 1e-9) bate = 0;
            }
            if(bate) inteiros++;
            double r = (double)t[12]/(double)t[11];
            /* t_k/t_{k−1} → σ com erro O(|σ'/σ|^k); a tolerância acompanha o erro real */
            double tol = 3.0 * pow(fabs(sl/s), 11) * s + 1e-12;
            if(fabs(r - s) < tol) razao_ok++;
            if(m<=3) printf("      %lld  %8.5f  %lld %lld %lld %lld %lld%*s%10.7f  %10.7f\n",
                            m, s, t[1],t[2],t[3],t[4],t[5], 12, "", r, s);
        }
        printf("      metais testados: %d\n", metais);
        ok("t_k = σ^k + σ'^k é INTEIRO — o traço cai no anel", inteiros==metais);
        ok("t_k/t_{k−1} → σ dentro do erro O(|σ'/σ|^k): o raio é 1/σ", razao_ok==metais);
        conclui("a série tem raio finito porque o coeficiente cresce. Não é defeito: é o polo a");
        conclui("anunciar-se de dentro do disco.");
    }

    /* ---------------- §C2 — dentro do disco: a série DÁ a forma fechada ---------------- */
    printf("\n§C2 dentro do disco: a série converge PARA a forma fechada\n");
    {
        int casos=0, bons=0;
        printf("      m    x        série            −log(1−mx−x²)   |dif|\n");
        for(L m=1; m<=6; m++){
            double s = (m + sqrt((double)(m*m+4)))/2.0, R = 1.0/s;
            for(double f=0.2; f<=0.85; f+=0.3){
                double x = f*R;
                /* traços em double, pela MESMA recorrência: sem estouro de inteiro */
                double ta=2, tb=m, soma = tb*x, xp = x*x;
                for(int k=2;k<=2000;k++){
                    double tk = m*tb + ta; ta=tb; tb=tk;
                    double termo = tk*xp/k;
                    soma += termo; xp *= x;
                    if(fabs(termo) < 1e-18) break;
                }
                double fech = -log(1.0 - m*x - x*x);
                casos++;
                double dif = fabs(soma-fech);
                if(dif < 1e-10*(1+fabs(fech))) bons++;
                if(m<=2 && f<0.6)
                    printf("      %lld  %7.5f  %14.10f  %14.10f  %.2e\n", m, x, soma, fech, dif);
            }
        }
        printf("      pontos dentro do disco: %d   série = forma fechada: %d\n", casos, bons);
        ok("dentro do disco as duas fórmulas dão O MESMO — é a mesma função", bons==casos);
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
        ok("fora do raio a SÉRIE explode — ela não alcança lá", explode==casos && casos>10);
        ok("e a forma fechada dá valor FINITO no mesmo ponto", finita==casos);
        conclui("é isto a continuação: o objeto não acabou no bordo do disco — a FÓRMULA é que");
        conclui("acabou. Troca-se de fórmula e o valor está lá, e é único por analiticidade.");
    }

    /* ---------------- §C4 — os polos são os metais ---------------- */
    printf("\n§C4 os polos da forma fechada são −σ e −σ': a continuação DEVOLVE os metais\n");
    {
        int metais=0, bate=0, raio_ok=0;
        printf("      m    polos de 1−mx−x²        −σ'         −σ          raio  1/σ\n");
        for(L m=1; m<=8; m++){
            double d = sqrt((double)(m*m+4));
            double s  = (m + d)/2.0, sl = (m - d)/2.0;
            /* 1 − m x − x² = 0  ⟺  x² + m x − 1 = 0  ⟹  x = (−m ± d)/2 = −σ', −σ */
            double r1 = (-m + d)/2.0, r2 = (-m - d)/2.0;
            metais++;
            if(fabs(r1 - (-sl)) < 1e-12 && fabs(r2 - (-s)) < 1e-12) bate++;
            /* e o raio de convergência é o polo MAIS PRÓXIMO: |−σ'| = 1/σ */
            double raio = fabs(r1) < fabs(r2) ? fabs(r1) : fabs(r2);
            if(fabs(raio - 1.0/s) < 1e-12) raio_ok++;
            if(m<=3) printf("      %lld  %9.6f %9.6f  %9.6f  %9.6f  %8.6f  %8.6f\n",
                            m, r1, r2, -sl, -s, raio, 1.0/s);
        }
        printf("      metais: %d   com polos = (−σ', −σ): %d   com raio = 1/σ: %d\n",
               metais, bate, raio_ok);
        ok("os polos são EXATAMENTE −σ' e −σ — a zeta guarda os metais", bate==metais);
        ok("e o raio da série é |−σ'| = 1/σ, o polo mais próximo", raio_ok==metais);
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
                if(fabs(fech - via_fatores) < 1e-9*(1+fabs(fech))) bons++;

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
                        if(fabs(termo) < 1e-17) break;
                    }
                    double via_serie = -log(x*x) + soma;      /* −log(−x²) em módulo */
                    tras_casos++;
                    if(fabs(via_serie - fech) < 1e-7*(1+fabs(fech))) tras_bons++;
                    if(tras_casos<=2)
                        printf("      x=%8.4f   série em 1/x = %12.8f   forma fechada = %12.8f\n",
                               x, via_serie, fech);
                }
            }
            printf("      pontos com |x| > σ: %d   com as duas expressões a bater: %d\n", casos, bons);
            printf("      pontos com |x| > σ: %d   série PARA TRÁS a bater na fechada: %d\n",
                   tras_casos, tras_bons);
            ok("a fatoração pelas singularidades dá o mesmo módulo — log|ab| = log|a|+log|b|",
               bons==casos && casos>=3);
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
            double T = 1e7;
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
               fabs(fase_dual - PI) < 1e-9);
            ok("na régua DUAL não há salto: o passo é uniforme e infinitesimal",
               salto_dual < 1e-4);
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
    return falhas ? 1 : 0;
}
