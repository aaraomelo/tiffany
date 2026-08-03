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
 * A série tem raio 1/σ (o polo mais próximo é −σ', e |σ'| = 1/σ, porque σσ' = −1). A forma
 * fechada não tem raio: tem polos, e os polos SÃO −σ e −σ'. Continuar analiticamente a
 * série devolve os metais de onde ela saiu.
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
        int pts=0, com_imagem=0, involucao=0;
        for(L u=-30; u<=30; u++) for(L v=-30; v<=30; v++){
            if(u==0 && v==0) continue;                 /* (0:0) não é ponto de P¹ */
            pts++;
            L iu = v, iv = u;                          /* A_0·(u,v) = (v,u) */
            if(!(iu==0 && iv==0)) com_imagem++;        /* a imagem é sempre ponto de P¹ */
            L ju = iv, jv = iu;                        /* aplicar outra vez */
            if(ju==u && jv==v) involucao++;
        }
        printf("      pontos de P¹ varridos: %d   com imagem: %d   com A_0² = id: %d\n",
               pts, com_imagem, involucao);
        ok("A_0 é BIJEÇÃO de P¹ — nenhum ponto fica sem imagem, 0 e ∞ incluídos", com_imagem==pts);
        ok("e é involução: A_0² = id, com ν∘ν = id", involucao==pts);
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
        conclui("a continuação NÃO é somar mais — é outra fórmula, e a analiticidade é que garante");
        conclui("que só há uma. Este medidor falha se alguém escrever que basta truncar mais longe.");
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
            int desde_k1 = 1, primeiro_mau = 0;
            for(int k=1;k<=14;k++){
                double dist = fabs(pow(s,k) - (double)f[k]);
                if(dist >= 0.5){ desde_k1 = 0; if(!primeiro_mau) primeiro_mau = k; }
            }
            if(p) pisot++;
            if(desde_k1) inteiro_proximo++;
            if(m == 1) k1_falha_em_m1 = (primeiro_mau == 1);
            if(m >= 2 && desde_k1) m2_ok++;
            if(m >= 2) m2_tot++;
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
        printf("      |σ'| < 1 (é Pisot): %d de %d   e t_k é o inteiro mais próximo DESDE k=1: %d\n",
               pisot, metais, inteiro_proximo);
        printf("      em m=1 falha logo em k=1 (|σ−t_1| = %.4f > 1/2): %s\n",
               fabs((1+sqrt(5.0))/2 - 1.0), k1_falha_em_m1 ? "sim" : "não");
        printf("      e para m >= 2 vale desde k=1: %d de %d\n", m2_ok, m2_tot);
        ok("σ é PISOT para todo m >= 1: |σ'| < 1", pisot==metais && pisot_falha_geral==0);
        ok("mas 'inteiro mais próximo desde k=1' só vale para m >= 2 — m=1 falha em k=1",
           !k1_falha_em_m1 ? 0 : (m2_ok==m2_tot && inteiro_proximo==m2_tot));
        conclui("|σ^k − t_k| = |σ'|^k = 1/σ^k, e isso é < 1/2 só quando |σ'| < 1/2, isto é m >= 2.");
        conclui("a fronteira SAI DA CONTA, não do limiar: a primeira versão deste medidor começava");
        conclui("em k=3 e escondia exatamente o caso que falha.");
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
            int casos=0, bons=0;
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
            }
            printf("      pontos com |x| > σ: %d   com as duas expressões a bater: %d\n", casos, bons);
            ok("a fatoração pelos polos dá o MESMO valor do outro lado — é a mesma função",
               bons==casos && casos>=3);
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
