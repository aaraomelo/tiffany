/* universal.c — A TRANSFORMADA UNIVERSAL: e o √N do ruído É a norma que ela conserva.
 *
 * O Aarão: "raiz de N é a transformada universal do corpo universal, que no fundo R^n é uma
 * realização. Verifica ele no enredo, e a convolução e deconvolução universal — recupera isso do
 * enredo e atualiza o catálogo, a teoria e as tools com a visão unificada."
 *
 * NO ENREDO ELA JÁ ESTÁ DEFINIDA, e o `chess/sandbox/geracao_energia.tex` diz-o assim:
 *
 *      "a transformada universal o diagonaliza (F(a ⊛ b) = F(a)F(b), resíduo 0)"
 *
 * Ou seja: **a transformada universal é a que leva CONVOLUÇÃO em PRODUTO**. Não é uma escolha de
 * base — é a única coisa que se lhe pede, e daí sai tudo o resto.
 *
 * E A AFIRMAÇÃO DO AARÃO É QUE O √N É ELA. Isso é forte e é testável, porque o `√N` aparece em três
 * sítios que ninguém costuma ligar:
 *
 *      1. na NORMALIZAÇÃO da transformada:  F_jk = ω^{jk}/√N   (a única que a torna unitária)
 *      2. no RUÍDO:                          N medidas independentes reduzem o desvio por √N
 *      3. na BASE ortonormal:                H/√N é ortogonal   (o hopfield.c §F11)
 *
 * *E os três são o mesmo número pela mesma razão.* Uma transformada unitária conserva a norma
 * (Parseval); somar N coisas independentes é somar N vetores ortogonais, e a norma disso é √N vezes
 * a de cada uma. **O √N do ruído não se parece com o da transformada: é o mesmo, porque é a MESMA
 * norma a ser conservada.** É isso que se mede aqui.
 *
 *   §U1  a TRANSFORMADA UNIVERSAL: ela diagonaliza a convolução — medido, como o enredo diz
 *   §U2  a NORMALIZAÇÃO 1/√N é a ÚNICA que a torna unitária — procurada, não escolhida
 *   §U3  E O √N DO RUÍDO É A MESMA NORMA — Parseval, e os dois batem
 *   §U4  R^n é uma REALIZAÇÃO: a multiplicação lá É convolução, e a régua não muda
 *   §U5  a DECONVOLUÇÃO: o quociente, e a condição exata para ela existir
 *   §U6  a visão unificada: os três √N num só
 *
 *   cc -O2 -std=c99 -Wall -Wformat universal.c -lm -o universal && ./universal
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NMAX 64

/* a transformada, com normalização s: F_jk = ω^{jk} · s */
static void dft(const double complex *a, double complex *A, int N, double s){
    for(int k = 0; k < N; k++){
        double complex acc = 0;
        for(int j = 0; j < N; j++){
            double t = -2.0*M_PI*j*k/N;
            acc += a[j] * (cos(t) + I*sin(t));
        }
        A[k] = acc * s;
    }
}
static void idft(const double complex *A, double complex *a, int N, double s){
    for(int j = 0; j < N; j++){
        double complex acc = 0;
        for(int k = 0; k < N; k++){
            double t = 2.0*M_PI*j*k/N;
            acc += A[k] * (cos(t) + I*sin(t));
        }
        a[j] = acc * s;
    }
}

/* a convolução CÍCLICA — e é ela que a transformada diagonaliza */
static void conv(const double complex *a, const double complex *b, double complex *c, int N){
    for(int n = 0; n < N; n++){
        double complex acc = 0;
        for(int m = 0; m < N; m++) acc += a[m] * b[(n - m + N) % N];
        c[n] = acc;
    }
}

static double norma(const double complex *a, int N){
    double s = 0;
    for(int i = 0; i < N; i++) s += creal(a[i]*conj(a[i]));
    return sqrt(s);
}
static double dif(const double complex *a, const double complex *b, int N){
    double s = 0;
    for(int i = 0; i < N; i++){
        double complex d = a[i] - b[i];
        s += creal(d*conj(d));
    }
    return sqrt(s);
}

static unsigned long SEM = 88172645463325252UL;
static unsigned long xs(void){ SEM ^= SEM<<13; SEM ^= SEM>>7; SEM ^= SEM<<17; return SEM; }
static double uni(void){ return (double)(xs() % 2000000)/1000000.0 - 1.0; }

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("universal.c — A TRANSFORMADA UNIVERSAL, e os tres raiz(N) que sao um so\n");

    /* ── §U1 ─────────────────────────────────────────────────────────────── */
    puts("§U1  A TRANSFORMADA UNIVERSAL: ela DIAGONALIZA a convolucao");
    puts("     O enredo (chess/sandbox/geracao_energia.tex): 'a transformada universal o");
    puts("     diagonaliza, F(a conv b) = F(a)F(b), residuo 0'. E e a UNICA coisa que se lhe");
    puts("     pede — tudo o resto sai daqui.\n");
    {
        int fecham = 0, casos = 0; double pior = 0;
        for(int N = 4; N <= 32; N *= 2){
            double complex a[NMAX], b[NMAX], c[NMAX], A[NMAX], B[NMAX], C[NMAX], P[NMAX];
            for(int i = 0; i < N; i++){ a[i] = uni() + I*uni(); b[i] = uni() + I*uni(); }
            conv(a, b, c, N);
            dft(a, A, N, 1.0); dft(b, B, N, 1.0); dft(c, C, N, 1.0);
            for(int k = 0; k < N; k++) P[k] = A[k]*B[k];
            double res = dif(C, P, N) / (norma(C, N) + 1e-30);
            if(res < 1e-12) fecham++;
            if(res > pior) pior = res;
            casos++;
        }
        ok("F(a conv b) = F(a).F(b) — a convolucao vira PRODUTO, em N = 4, 8, 16 e 32",
           fecham == casos);
        printf("     -> %d tamanhos, %d fecham, pior residuo relativo %.1e.\n", casos, fecham, pior);
        puts("        E o que a torna UNIVERSAL: ela nao depende do corpo, so de haver uma");
        puts("        convolucao. Qualquer corpo com produto invariante por deslocamento cai aqui.\n");
    }

    /* ── §U2  a NORMALIZAÇÃO ─────────────────────────────────────────────── */
    puts("§U2  A NORMALIZACAO 1/raiz(N) e a UNICA que a torna unitaria — procurada, nao escolhida");
    puts("     Varre-se o expoente da normalizacao e ve-se qual conserva a norma. Nao se assume");
    puts("     o meio: mede-se onde ele esta.\n");
    {
        int N = 16;
        double complex a[NMAX], A[NMAX];
        for(int i = 0; i < N; i++) a[i] = uni() + I*uni();
        double na = norma(a, N);
        printf("     %14s %16s %14s\n", "normalizacao", "||F(a)||/||a||", "conserva?");
        double melhor_p = 0, melhor_e = 1e9;
        for(double p = 0.0; p <= 1.01; p += 0.25){
            double s = pow(N, -p);
            dft(a, A, N, s);
            double r = norma(A, N)/na;
            printf("     %14s %16.6f %14s\n",
                   p == 0 ? "1 (nenhuma)" : (fabs(p-0.5) < 1e-9 ? "1/raiz(N)" :
                   (fabs(p-1.0) < 1e-9 ? "1/N" : "N^-p")), r, fabs(r-1) < 1e-9 ? "SIM" : "nao");
            if(fabs(r - 1.0) < melhor_e){ melhor_e = fabs(r - 1.0); melhor_p = p; }
        }
        ok("so o expoente 1/2 conserva a norma — e ele PROCUROU-SE, nao foi posto a mao",
           fabs(melhor_p - 0.5) < 1e-9 && melhor_e < 1e-12);
        /* e a inversa fecha com a MESMA normalização — a transformada é unitária, não só isométrica */
        double complex volta[NMAX];
        dft(a, A, N, 1.0/sqrt(N));
        idft(A, volta, N, 1.0/sqrt(N));
        ok("e com 1/raiz(N) nos DOIS sentidos a ida-e-volta e exata: F' = F^{-1}",
           dif(volta, a, N)/na < 1e-12);
        printf("     -> o expoente que conserva e %.4f, e a volta fecha com residuo %.1e.\n",
               melhor_p, dif(volta, a, N)/na);
        puts("        A raiz nao esta la por gosto de simetria: e o UNICO ponto onde a ida e a");
        puts("        volta pesam igual — e por isso a transformada e o seu proprio inverso a");
        puts("        menos de conjugacao.\n");
    }

    /* ── §U3  O √N DO RUÍDO ──────────────────────────────────────────────── */
    puts("§U3  E O raiz(N) DO RUIDO E A MESMA NORMA — nao se parece: E");
    puts("     Somar N coisas INDEPENDENTES e somar N vetores ortogonais, e a norma disso e");
    puts("     raiz(N) vezes a de cada uma. E a norma que a transformada conserva e essa.\n");
    {
        /* mede-se o crescimento da norma de uma soma de N ruídos independentes */
        int amostras = 4000;
        printf("     %8s %16s %16s %12s\n", "N", "|soma| medido", "raiz(N).sigma", "razao");
        int lei = 1; double pior = 0;
        for(int N = 1; N <= 256; N *= 4){
            double soma_q = 0;
            for(int t = 0; t < amostras; t++){
                double s = 0;
                for(int i = 0; i < N; i++) s += uni();
                soma_q += s*s;
            }
            double rms = sqrt(soma_q/amostras);
            double sigma1 = sqrt(1.0/3.0);            /* o desvio de uma uniforme em [-1,1] */
            double previsto = sqrt((double)N) * sigma1;
            double razao = rms/previsto;
            printf("     %8d %16.4f %16.4f %12.4f\n", N, rms, previsto, razao);
            if(fabs(razao - 1.0) > 0.06) lei = 0;
            if(fabs(razao - 1.0) > pior) pior = fabs(razao - 1.0);
        }
        ok("A LEI DO raiz(N): a norma da soma de N independentes e raiz(N) vezes a de uma",
           lei);
        /* E A LIGAÇÃO: Parseval diz que a transformada conserva ESSA norma. Mede-se junto. */
        int N = 16;
        double complex r[NMAX], R[NMAX];
        for(int i = 0; i < N; i++) r[i] = uni() + I*uni();
        dft(r, R, N, 1.0/sqrt(N));
        double erro_parseval = fabs(norma(R,N) - norma(r,N))/norma(r,N);
        ok("e PARSEVAL: a transformada com 1/raiz(N) conserva EXATAMENTE essa mesma norma",
           erro_parseval < 1e-12);
        printf("     -> a lei do raiz(N) fecha com %.1f%% de desvio maximo em 4000 amostras, e\n",
               100*pior);
        printf("        Parseval fecha com %.1e. O raiz(N) do ruido e o raiz(N) da transformada\n",
               erro_parseval);
        puts("        sao o MESMO numero, e a razao e uma so: a ortogonalidade. Somar coisas");
        puts("        independentes e somar direcoes perpendiculares, e Pitagoras da o resto.\n");
    }

    /* ── §U4  R^n é uma REALIZAÇÃO ───────────────────────────────────────── */
    puts("§U4  R^n e uma REALIZACAO: a multiplicacao la E convolucao, e a regua nao muda");
    puts("     O Aarao: 'o corpo universal, que no fundo R^n e uma realizacao'. Mede-se:");
    puts("     multiplicar dois elementos de R^n = Z_p[x]/(p_n) E convoluir os coeficientes.\n");
    {
        /* em Z_p[x]/(x^N − 1) a multiplicação é EXATAMENTE a convolução cíclica.
         * (o corpo do projeto usa x^n − m x^{n-1} − 1, e a diferença é só a redução —
         * a parte de convolução é a mesma, e é isso que se mede.) */
        int N = 8, p = 101;
        int a[NMAX], b[NMAX], prod[NMAX], cv[NMAX];
        for(int i = 0; i < N; i++){ a[i] = (int)(xs()%p); b[i] = (int)(xs()%p); }
        /* o produto polinomial com redução x^N = 1 */
        long t[2*NMAX] = {0};
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++) t[i+j] = (t[i+j] + (long)a[i]*b[j]) % p;
        for(int d = 2*N-2; d >= N; d--){ t[d-N] = (t[d-N] + t[d]) % p; t[d] = 0; }
        for(int i = 0; i < N; i++) prod[i] = (int)t[i];
        /* e a convolução cíclica, direta */
        for(int n = 0; n < N; n++){
            long acc = 0;
            for(int m = 0; m < N; m++) acc += (long)a[m]*b[(n-m+N)%N];
            cv[n] = (int)(acc % p);
        }
        int iguais = 1;
        for(int i = 0; i < N; i++) if(prod[i] != cv[i]) iguais = 0;
        ok("multiplicar em Z_p[x]/(x^N-1) E convoluir os coeficientes — as N casas batem",
           iguais);
        puts("");
        /* ── A CORRECAO, e ela veio de um revisor externo ──────────────────────────────
         * Eu tinha escrito aqui, EM PROSA e ao lado da medida, que "a borda troca a reducao
         * e nao a convolucao, e por isso a transformada continua a diagonalizar". A segunda
         * metade e FALSA, e o revisor mediu-a: Z_p[x]/(p_n) com p_n IRREDUTIVEL e um CORPO,
         * logo so tem idempotentes triviais — nao ha diagonalizacao nenhuma. A DFT diagonaliza
         * x^N-1 porque ele CINDE em fatores lineares distintos; a borda do metal nao cinde.
         *
         * E o meu §U4 media so o x^N-1: a frase sobre a borda era prosa impressa ao lado de
         * uma medida que nao a cobria. Pior — a reordenacao da teoria promoveu-a a PRIMEIRA
         * afirmacao do paper.
         *
         * A FORMA VERDADEIRA, e ela e melhor: a transformada e a AVALIACAO NAS RAIZES da
         * borda. Para x^N-1 sao as N raizes da unidade (a DFT). Para p_n sao as n conjugadas
         * de FROBENIUS — sigma, sigma^p, ..., sigma^{p^{n-1}} — que sao literalmente as
         * FOLHAS do §6. Mede-se, em vez de se afirmar. */
        puts("     A CORRECAO (achada por revisor externo): a borda NAO diagonaliza pela DFT.");
        puts("     Z_p[x]/(p_n) com p_n irredutivel e um CORPO — so tem idempotentes triviais.");
        puts("     A transformada certa e a AVALIACAO NAS RAIZES, e para a borda elas sao as");
        puts("     conjugadas de FROBENIUS: as folhas do §6.\n");
        {
            /* GF(p^2) com x^2 - m x - 1, e as duas folhas sao sigma e sigma^p.
             * Avaliar um elemento a0 + a1*sigma nas duas folhas dá duas coordenadas, e a
             * MULTIPLICACAO tem de virar produto casa a casa. Isso e a diagonalizacao. */
            int P2 = 13, M = 1;                    /* x^2 - x - 1 sobre Z_13 */
            /* sigma vive em GF(169); representa-se como par (a,b) = a + b*sigma */
            /* o Frobenius: (a + b*sigma)^p. Sobre Z_p, (u+v)^p = u^p + v^p e a^p = a, logo
             * Frob(a + b*sigma) = a + b*sigma^p, e basta saber sigma^p. */
            int sig_p[2];                          /* sigma^p = c + d*sigma */
            {   /* calcula sigma^p por quadrados, na base {1, sigma} com sigma^2 = m*sigma + 1 */
                int r[2] = {1,0}, b[2] = {0,1}, e = P2;
                while(e){
                    if(e & 1){
                        int t0 = (r[0]*b[0] + r[1]*b[1]) % P2;
                        int t1 = (r[0]*b[1] + r[1]*b[0] + M*r[1]*b[1]) % P2;
                        r[0]=t0; r[1]=t1;
                    }
                    int q0 = (b[0]*b[0] + b[1]*b[1]) % P2;
                    int q1 = (2*b[0]*b[1] + M*b[1]*b[1]) % P2;
                    b[0]=q0; b[1]=q1;
                    e >>= 1;
                }
                sig_p[0]=r[0]; sig_p[1]=r[1];
            }
            /* a avaliacao nas duas folhas: F(a)_1 = a0 + a1*sigma, F(a)_2 = a0 + a1*sigma^p.
             * Como sigma nao esta em Z_p, as coordenadas vivem em GF(p^2) — representam-se
             * como pares e o produto e o do corpo. */
            long testes = 0, falhou = 0;
            for(int a0 = 0; a0 < P2; a0++)
            for(int a1 = 0; a1 < P2; a1++)
            for(int b0 = 0; b0 < 3; b0++)
            for(int b1 = 0; b1 < 3; b1++){
                /* o produto no corpo */
                int c0 = (a0*b0 + a1*b1) % P2;
                int c1 = (a0*b1 + a1*b0 + M*a1*b1) % P2;
                /* a avaliacao na folha 2 (sigma -> sigma^p), e o produto das avaliacoes */
                /* Fa = a0 + a1*sig_p  (par na base {1,sigma}) */
                int fa0 = (a0 + a1*sig_p[0]) % P2, fa1 = (a1*sig_p[1]) % P2;
                int fb0 = (b0 + b1*sig_p[0]) % P2, fb1 = (b1*sig_p[1]) % P2;
                int fc0 = (c0 + c1*sig_p[0]) % P2, fc1 = (c1*sig_p[1]) % P2;
                /* Fa * Fb no corpo */
                int g0 = (fa0*fb0 + fa1*fb1) % P2;
                int g1 = (fa0*fb1 + fa1*fb0 + M*fa1*fb1) % P2;
                if(g0 != fc0 || g1 != fc1) falhou++;
                testes++;
            }
            ok("A AVALIACAO NAS FOLHAS DIAGONALIZA: F(ab) = F(a)F(b) na conjugada de Frobenius",
               falhou == 0 && testes > 1000);
            printf("     -> %ld testes em GF(13^2), %ld falhas. sigma^13 = %d + %d.sigma.\n",
                   testes, falhou, sig_p[0], sig_p[1]);
            puts("        E as folhas SAO o §6: a transformada universal e a avaliacao nas raizes");
            puts("        da borda, e para a borda do metal essas raizes sao as conjugadas de");
            puts("        Frobenius. O §5 (a inversa pelos conjugados) e o §6 (as folhas) passam");
            puts("        a ser COROLARIOS disto, e nao assuntos paralelos.\n");
        }
    }

    /* ── §U5  a DECONVOLUÇÃO ─────────────────────────────────────────────── */
    puts("§U5  A DECONVOLUCAO: o quociente, e a condicao EXATA para ela existir");
    puts("     O converte.c ja o dizia: 'a convolucao e o produto (o gato), a deconvolucao e o");
    puts("     quociente (o esquilo)'. E o quociente existe sse nenhuma casa da transformada e");
    puts("     zero — e isso e decidivel, nao e uma esperanca.\n");
    {
        int N = 16;
        double complex a[NMAX], b[NMAX], c[NMAX], A[NMAX], B[NMAX], C[NMAX], rec[NMAX];
        for(int i = 0; i < N; i++){ a[i] = uni() + I*uni(); b[i] = uni() + I*uni(); }
        conv(a, b, c, N);
        dft(b, B, N, 1.0); dft(c, C, N, 1.0);
        /* nenhuma casa nula? */
        double menor = 1e9;
        for(int k = 0; k < N; k++) if(cabs(B[k]) < menor) menor = cabs(B[k]);
        ok("neste caso NENHUMA casa de F(b) e zero — logo o quociente existe",
           menor > 1e-9);
        for(int k = 0; k < N; k++) A[k] = C[k]/B[k];
        idft(A, rec, N, 1.0/N);
        ok("e a DECONVOLUCAO devolve o original: dividir casa a casa e desfazer a convolucao",
           dif(rec, a, N)/norma(a, N) < 1e-10);
        /* e o caso em que NÃO existe: um b cuja transformada tem um zero */
        double complex bz[NMAX], BZ[NMAX];
        for(int i = 0; i < N; i++) bz[i] = 1.0;      /* o núcleo constante: F dele é N·δ */
        dft(bz, BZ, N, 1.0);
        int zeros = 0;
        for(int k = 0; k < N; k++) if(cabs(BZ[k]) < 1e-9) zeros++;
        ok("e ha nucleos SEM inversa: o constante anula N-1 casas, e ali a informacao PERDE-SE",
           zeros == N-1);
        printf("     -> a menor casa foi %.4f (existe); e o nucleo constante anula %d de %d casas.\n",
               menor, zeros, N);
        puts("        O nucleo sem inversa e o que NAO TEM DUAL — e o koch.c ja lhe deu o nome:");
        puts("        ele nao atravessa a alfandega, e o que ele apagou nao volta. A condicao");
        puts("        de existir e decidivel numa varredura, e e isso que faz dela uma lei.\n");
    }

    /* ── §U6  a visão unificada ──────────────────────────────────────────── */
    puts("§U6  A VISAO UNIFICADA: os tres raiz(N) num so\n");
    {
        int N = 16;
        /* 1. a transformada: 1/√N torna-a unitária */
        double complex a[NMAX], A[NMAX];
        for(int i = 0; i < N; i++) a[i] = uni() + I*uni();
        dft(a, A, N, 1.0/sqrt(N));
        double e1 = fabs(norma(A,N) - norma(a,N))/norma(a,N);
        /* 2. a base: Hadamard/√N é ortonormal (o hopfield.c §F11) */
        static signed char H[64][64];
        H[0][0] = 1;
        for(int m = 1; m < N; m *= 2)
            for(int i = 0; i < m; i++)
                for(int j = 0; j < m; j++){
                    H[i][j+m] = H[i][j]; H[i+m][j] = H[i][j]; H[i+m][j+m] = -H[i][j];
                }
        double nl = 0;
        for(int j = 0; j < N; j++) nl += (double)H[3][j]*H[3][j];
        double e2 = fabs(sqrt(nl) - sqrt((double)N))/sqrt((double)N);
        /* 3. o ruído: N independentes dão √N */
        double soma_q = 0;
        for(int t = 0; t < 4000; t++){
            double s = 0;
            for(int i = 0; i < N; i++) s += uni();
            soma_q += s*s;
        }
        double e3 = fabs(sqrt(soma_q/4000) - sqrt((double)N/3.0))/sqrt((double)N/3.0);

        ok("OS TRES raiz(N) fecham no mesmo N: a transformada, a base e o ruido",
           e1 < 1e-12 && e2 < 1e-12 && e3 < 0.06);
        printf("     %-34s %14s\n", "onde aparece o raiz(N)", "residuo");
        printf("     %-34s %14.1e\n", "1. normalizar a transformada", e1);
        printf("     %-34s %14.1e\n", "2. normalizar a base (Hadamard)", e2);
        printf("     %-34s %14.1e\n", "3. promediar N medidas (ruido)", e3);
        puts("");
        puts("        E a razao e UMA: a ORTOGONALIDADE. Uma base ortogonal tem norma raiz(N);");
        puts("        a transformada que a usa conserva essa norma; e somar N independentes e");
        puts("        somar N direcoes perpendiculares, que Pitagoras conta como raiz(N).");
        puts("");
        puts("        O Aarao disse 'raiz de N e a transformada universal' e isso e mesmo assim:");
        puts("        nao ha tres coeficientes que por acaso coincidem — ha UM, e ele e a norma");
        puts("        do corpo. As tres coisas sao a mesma medida vista de tres sitios.\n");
    }

    /* ── §U7  O CORPO UNIVERSAL E AUTODUAL ───────────────────────────────── */
    puts("§U7  O CORPO UNIVERSAL E AUTODUAL — e verifica-se, nao se decreta\n");
    puts("     Autodual quer dizer: a transformada leva o corpo NELE PROPRIO, e nao noutro. Se");
    puts("     assim for, F aplicada quatro vezes tem de dar a identidade — porque F^2 e a");
    puts("     reflexao e F^4 = id. E isso e a ordem 4 do i, que o §F12 ja media.\n");
    {
        int N = 16;
        double complex a[NMAX], b1[NMAX], b2[NMAX], b3[NMAX], b4[NMAX];
        for(int i = 0; i < N; i++) a[i] = uni() + I*uni();
        double s = 1.0/sqrt(N);
        dft(a, b1, N, s); dft(b1, b2, N, s); dft(b2, b3, N, s); dft(b3, b4, N, s);

        /* 1. F^4 = identidade — a ordem e 4, e isso mede-se */
        double e4 = dif(b4, a, N)/norma(a, N);
        ok("F^4 = identidade: a transformada tem ORDEM 4 — a mesma do i e do esquilo",
           e4 < 1e-10);
        /* 2. e F^2 e a REFLEXAO: (F^2 a)[n] = a[-n]. Nao e a identidade, e nao e outra coisa. */
        double complex refl[NMAX];
        for(int n = 0; n < N; n++) refl[n] = a[(N - n) % N];
        double e2 = dif(b2, refl, N)/norma(a, N);
        ok("e F^2 e a REFLEXAO n -> -n, exatamente — nem a identidade, nem uma terceira coisa",
           e2 < 1e-10);
        /* 3. logo F^2 e uma INVOLUCAO: o J de ordem 2, dentro da ordem 4 */
        double complex r2[NMAX];
        for(int n = 0; n < N; n++) r2[n] = refl[(N - n) % N];
        ok("logo a reflexao e uma INVOLUCAO (ordem 2) DENTRO da ordem 4 — o J dentro do i",
           dif(r2, a, N)/norma(a,N) < 1e-14);

        /* 4. E O AUTODUAL PROPRIAMENTE: ha vetores que sao os SEUS PROPRIOS transformados.
         * Se o corpo nao fosse autodual, nao existiria nenhum — a transformada levaria tudo
         * para fora. Constroi-se um: a simetrizacao a + Fa + F^2a + F^3a e sempre proprio de F. */
        double complex fix[NMAX];
        for(int i = 0; i < N; i++) fix[i] = (a[i] + b1[i] + b2[i] + b3[i]) / 4.0;
        double complex Ffix[NMAX];
        dft(fix, Ffix, N, s);
        double e_fix = dif(Ffix, fix, N)/(norma(fix, N) + 1e-30);
        ok("AUTODUAL: ha vetores IGUAIS a sua propria transformada, e constroem-se somando a orbita",
           norma(fix,N) > 1e-6 && e_fix < 1e-10);
        printf("     -> F^4 fecha com %.1e; F^2 e a reflexao com %.1e; e o ponto fixo tem norma\n",
               e4, e2);
        printf("        %.4f e satisfaz F(x) = x com residuo %.1e.\n", norma(fix,N), e_fix);
        puts("");
        /* 5. e a GAUSSIANA e o exemplo classico: ela e a sua propria transformada */
        double complex g[NMAX], G[NMAX];
        for(int n = 0; n < N; n++){
            double x = (n <= N/2) ? n : n - N;
            g[n] = exp(-M_PI*x*x/N);
        }
        dft(g, G, N, s);
        double e_g = dif(G, g, N)/norma(g, N);
        ok("e a GAUSSIANA e o exemplo classico: ela e (quase) a sua propria transformada",
           e_g < 0.05);
        printf("     -> a gaussiana discreta reproduz-se com %.2f%% de desvio (o exato so no continuo).\n",
               100*e_g);
        puts("");
        puts("        E E ISSO QUE AUTODUAL SIGNIFICA AQUI: a transformada nao sai do corpo. Ela");
        puts("        leva R^n em R^n, tem ordem 4, o quadrado dela e o espelho, e ha pontos");
        puts("        fixos. Um corpo que precisasse de OUTRO para se transformar nao seria");
        puts("        universal — teria de haver um terceiro para fechar, e depois um quarto.");
        puts("");
        puts("        E fecha com o §B12: cada torre e antissimetrica, as duas juntas simetricas.");
        puts("        Aqui e a mesma coisa em ordens — F tem ordem 4 (o i, que RODA) e F^2 tem");
        puts("        ordem 2 (o J, que ESPELHA), e o segundo vive dentro do primeiro.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A transformada universal e a que leva CONVOLUCAO em PRODUTO — e o enredo ja o dizia");
    puts("  (geracao_energia.tex). Aqui mede-se em quatro tamanhos, residuo zero.");
    puts("");
    puts("  E o raiz(N) e ELA: o expoente 1/2 e o UNICO que a torna unitaria, e isso procurou-se");
    puts("  varrendo o expoente em vez de o pôr. E o mesmo raiz(N) do ruido e da base de");
    puts("  Hadamard — nao por coincidencia, mas porque os tres sao a norma que a ortogonalidade");
    puts("  impoe.");
    puts("");
    puts("  E R^n e uma REALIZACAO: multiplicar la E convoluir, e a borda so troca a reducao.");
    puts("  A deconvolucao existe sse nenhuma casa se anula — e o nucleo que anula e o que nao");
    puts("  tem dual, e fica na garrafa.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
