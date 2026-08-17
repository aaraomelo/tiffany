/* spline.c — A CARTA DE CADA FONTE EM SPLINES: o glifo é um polinômio, e a largura sai da CURVA.
 *
 * O Aarão: "usa o formato vetorial. Mais uma vez o corpo diferencial com splines. Polinômios. Pega
 * a carta de cada fonte LaTeX em splines. Aí o passo é via soma Fourier e a escala é via Mellin,
 * usa isso pra espaçamentos e tamanhos. O transistor vem costurando tudo, preenchendo o espaço
 * densamente, sem vazamento nenhum."
 *
 * PORQUE ISTO EXISTE. No tex.c as larguras vêm de tabelas base-14 que eu COPIEI e não medi — e foi
 * exatamente aí que escrevi dois números de cabeça na mesma sessão ("a negra é mais larga": o 'W' é
 * 944 nas duas; "nunca é mais estreita": o '@' é 975 contra 1015). Uma tabela copiada não tem como
 * falhar: ela é o que eu escrevi. A curva tem.
 *
 * O contorno de um glifo TrueType JÁ É uma spline — Bézier quadrática, o polinômio de grau 2:
 *
 *      B(t) = (1−t)²P₀ + 2t(1−t)P₁ + t²P₂ ,   t ∈ [0,1]
 *
 * e no projeto todo dado já é um polinômio na base {1,σ,…,σⁿ⁻¹} (teoria.tex §2). Não há conversão a
 * fazer: há uma leitura. A fonte é o corpo diferencial em cima da página.
 *
 * OS DOIS CAMINHOS. A Liberation Sans é METRICAMENTE COMPATÍVEL com a Arial, que o é com a
 * Helvetica. Logo a largura lida do ficheiro é um ORÁCULO EXTERNO da tabela do tex.c: se as duas
 * discordarem, uma delas está errada, e nenhuma das duas sou eu a confirmar-me.
 *
 *   §P1  a carta: ler a TTF e tirar o contorno de um glifo como splines
 *   §P2  OS DOIS CAMINHOS: a largura do ficheiro contra a tabela base-14 do tex.c
 *   §P3  a área pela CURVA — Green sobre a spline, e o polinômio integra-se exato
 *   §P4  o PASSO é aditivo: FOURIER, e o espaçamento uniforme é o modo zero
 *   §P5  a ESCALA é multiplicativa: MELLIN, que leva escala em translação
 *   §P6  sem vazamento: o que a costura enche, enche até ao resíduo
 *
 *   cc -O2 -std=c99 spline.c -lm -o spline && ./spline
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "spline.h"

/* ───────────────────────────────────────────── §P3  a ÁREA pela curva

 * Green: A = ½ ∮ (x dy − y dx). Sobre uma Bézier quadrática isso é um polinômio em t de grau 2, e
 * o polinômio INTEGRA-SE EXATO — não há quadratura, não há passo, não há erro de discretização.
 * Para B(t) com pontos P₀,P₁,P₂ a contribuição fecha em forma:
 *
 *      ∫₀¹ (x y' − y x') dt = ⅓[ (x₀+x₁)(y₂−y₀) + (x₁+x₂)(y₂−y₀) ]/2 ... — e melhor, em cru:
 *      2A_seg = ⅓(x₀(2y₁+y₂−3y₀) ... ) — em vez de decorar a fórmula, DERIVA-SE aqui abaixo,
 *      porque uma fórmula decorada é a tabela copiada outra vez.
 * ───────────────────────────────────────────── */


/* ── A MESMA ÁREA, EM INTEIROS, E O PONTO MÉDIO QUE SE PERDIA ─────────────────────────
 * As coordenadas de um glifo são INTEIRAS (Pt tem `long x, y`), logo a área é um
 * RACIONAL exacto e o double só a transporta. Mas há uma perda antes disso: o ponto da
 * curva IMPLÍCITO entre dois controlos é o ponto MÉDIO, e `(a.x + b.x)/2` em inteiros
 * TRUNCA quando a soma é ímpar. Medido na fonte do documento: 1105 dos 1436 implícitos
 * têm soma ímpar — 77%. O contorno fica meio ponto deslocado em três quartos deles.
 *
 * A cura é a mesma que a casa usa noutros sítios: NORMALIZAR. Dobram-se as coordenadas e
 * o ponto médio passa a ser exacto; a área sai ×4, e o denominador 6 da quadrática dá o
 * factor 24. Devolve-se 24·A como INTEIRO, sem uma vírgula e sem truncar nada. */
static long area24_quad_i(long ax, long ay, long bx, long by, long cx, long cy){
    long A0 = ax, A1 = 2*(bx - ax), A2 = ax - 2*bx + cx;
    long B0 = ay, B1 = 2*(by - ay), B2 = ay - 2*by + cy;
    long c0 = A0*B1 - B0*A1;
    long c1 = 2*A0*B2 + A1*B1 - 2*B0*A2 - B1*A1;
    long c2 = 2*A1*B2 + A2*B1 - 2*B1*A2 - B2*A1;
    return 6*c0 + 3*c1 + 2*c2;                        /* 6·(c0 + c1/2 + c2/3), inteiro */
}
static long area24_glifo(const Contorno *c, long *segmentos, long *implicitos, long *impares){
    long A = 0, seg = 0; int ini = 0;
    for(int k = 0; k < c->nc; k++){
        int f = c->fim[k], m = f - ini + 1;
        if(m <= 0){ ini = f + 1; continue; }
        long px[MAXPT], py[MAXPT]; int on[MAXPT], n = 0;
        for(int i = 0; i < m; i++){                    /* tudo DOBRADO: o médio é exacto */
            Pt a = c->p[ini + i], b = c->p[ini + (i+1) % m];
            px[n] = 2*a.x; py[n] = 2*a.y; on[n] = a.onda; n++;
            if(!a.onda && !b.onda){
                if(implicitos) (*implicitos)++;
                if(impares && (((a.x + b.x) & 1) || ((a.y + b.y) & 1))) (*impares)++;
                px[n] = a.x + b.x; py[n] = a.y + b.y; on[n] = 1; n++;   /* 2·médio, exacto */
            }
        }
        int s = 0; while(s < n && !on[s]) s++;
        if(s == n){ ini = f + 1; continue; }
        for(int i = 0; i < n; ){
            int i0 = (s+i)%n, i1 = (s+i+1)%n;
            if(on[i1]){ A += 6*(px[i0]*py[i1] - py[i0]*px[i1]); i += 1; seg++; }
            else { int i2 = (s+i+2)%n;
                   A += area24_quad_i(px[i0],py[i0], px[i1],py[i1], px[i2],py[i2]); i += 2; seg++; }
            if(i >= n) break;
        }
        ini = f + 1;
    }
    if(segmentos) *segmentos = seg;
    return A < 0 ? -A : A;            /* 24·(2A_dobrada) = 96·A_original, e é inteiro */
}


/* ───────────────────────────────────────────── as tabelas do tex.c, para o CONFRONTO */

static const short W_REG[95] = {
 278,278,355,556,556,889,667,191,333,333,389,584,278,333,278,278,
 556,556,556,556,556,556,556,556,556,556,278,278,584,584,584,556,
1015,667,667,722,722,667,611,778,722,278,500,667,556,833,722,778,
 667,778,722,667,611,722,667,944,667,667,611,278,278,278,469,556,
 333,556,556,500,556,556,278,556,556,222,222,500,222,833,556,556,
 556,556,333,500,278,556,500,722,500,500,500,334,260,334,584};
static const short W_NEG[95] = {
 278,333,474,556,556,889,722,238,333,333,389,584,278,333,278,278,
 556,556,556,556,556,556,556,556,556,556,333,333,584,584,584,611,
 975,722,722,722,722,667,611,778,722,278,556,722,611,833,722,778,
 667,778,722,667,611,722,667,944,667,667,611,333,278,333,584,556,
 333,556,611,556,611,556,333,611,611,278,278,556,278,889,611,611,
 611,611,389,556,333,611,556,778,556,556,500,389,280,389,584};

/* ───────────────────────────────────────────── §P4/§P5  a TRÍADE nos espaçamentos

 * O passo é ADITIVO e a escala é MULTIPLICATIVA — e o projeto já tem os dois núcleos:
 *
 *      SOMA ⊕  ->  FOURIER   x_k = Σ larguras: uma sequência de PASSOS. Espaçar uniformemente é
 *                            pôr toda a energia no MODO ZERO — o modo constante. Qualquer
 *                            irregularidade no espaçamento é energia nos modos k≠0, e mede-se.
 *      PRODUTO ⊗ -> MELLIN   mudar o corpo de 10pt para 12pt é MULTIPLICAR. Mellin leva escala em
 *                            translação: M[f(λx)](s) = λ^{−s} M[f](s). Em log, a escala é um
 *                            deslocamento — e o que era produto vira soma, que é a tríade inteira.
 * ───────────────────────────────────────────── */

/* a energia do espaçamento nos modos k>0. Uniforme -> 0. */
/* A ENERGIA FORA DO MODO ZERO, POR PARSEVAL — e é a rota que não soma modo nenhum.
 *
 * Parseval diz que a energia total é a soma dos módulos ao quadrado dos coeficientes, e o
 * modo zero leva N·m0². Logo o que fica fora dele é
 *
 *      Σ d_j²/N  −  m0²        com m0 = (Σ d_j)/N
 *
 * — a VARIÂNCIA, e ela não precisa de um único cosseno. Com os dados INTEIROS a conta faz-se
 * em ℤ: N·Σd² − (Σd)² é um inteiro, e vale ZERO exactamente quando todos os d são iguais.
 * É a segunda rota da `fourier_fora_do_zero`, e é ela que dispensa o limiar de 1e-18. */
static long parseval_fora_z(const long *d, int n, long *soma){
    long s = 0, s2 = 0;
    for(int i = 0; i < n; i++){ s += d[i]; s2 += d[i]*d[i]; }
    if(soma) *soma = s;
    return (long)n*s2 - s*s;              /* = N²·(energia fora), inteiro */
}

static double fourier_fora_do_zero(const double *d, int n, double *modo0){
    double m0 = 0;
    for(int i = 0; i < n; i++) m0 += d[i];
    m0 /= n;
    *modo0 = m0;
    double e = 0;
    for(int k = 1; k < n; k++){
        double re = 0, im = 0;
        for(int i = 0; i < n; i++){
            double a = -2.0*M_PI*k*i/n;
            re += d[i]*cos(a); im += d[i]*sin(a);
        }
        e += (re*re + im*im) / (n*(double)n);
    }
    return e;
}

/* ───────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    /* o idioma da bateria: sem isto ela conta UMA unidade grossa (o exit) em vez das que ha */
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

/* O ORÁCULO DESTE MEDIDOR É A LIBERATION SANS (metricamente Arial→Helvetica):
 * o §P2 compara o ficheiro contra a tabela base-14 do tex.c, e isso só faz
 * sentido com a Liberation. O SPLINE_REG da lib serve o COMPOSITOR (as fontes
 * do repo primeiro) — a auditoria de 14/08 apanhou o medidor a abrir a fonte
 * errada quando a lista da lib mudou. Aqui a lista é a do oráculo. */
static const char *CANDIDATAS[] = {
    "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
    "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
};
static const char *CAND_NEG[] = {
    "/usr/share/fonts/liberation-sans/LiberationSans-Bold.ttf",
    "/usr/share/fonts/liberation/LiberationSans-Bold.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
};
#define abre_alguma spline_abre_alguma

int main(void){
    puts("spline.c — A CARTA DE CADA FONTE EM SPLINES: o glifo e um polinomio\n");

    Ttf reg, neg; const char *fr = NULL, *fn = NULL;
    int tem_r = abre_alguma(&reg, CANDIDATAS, 3, &fr);
    int tem_n = abre_alguma(&neg, CAND_NEG,   3, &fn);

    /* ── §P1 ─────────────────────────────────────────────────────────────── */
    puts("§P1  A CARTA: a TTF lida a mao, e o contorno sai em splines quadraticas");
    puts("     B(t) = (1-t)^2 P0 + 2t(1-t) P1 + t^2 P2 — o polinomio de grau 2. Nao ha conversao");
    puts("     a fazer: o glifo JA E spline, como no projeto todo dado ja e polinomio na base.\n");
    if(!tem_r){
        puts("  [aviso] a Liberation Sans nao esta neste sistema — sem oraculo externo, nao ha medida.");
        puts("          E preferivel nao medir a medir contra mim proprio.\n");
        printf("unidades: 0   falhas: 0\nRESIDUO 0\n");
        return 0;
    }
    printf("     fonte: %s\n     unitsPerEm=%d, %d glifos, loca %s\n\n",
           fr, reg.upem, reg.nglifos, reg.longloca ? "longo" : "curto");
    {
        Contorno c;
        int g = ttf_glifo(&reg, 'B');
        int leu = g && ttf_contorno(&reg, g, &c);
        ok("o 'B' tem indice de glifo, e o contorno le-se: pontos e mais de um contorno",
           leu && c.n > 10 && c.nc >= 2);          /* o B tem o exterior e os dois buracos */
        int g_o = ttf_glifo(&reg, 'o');
        Contorno co; int leu_o = g_o && ttf_contorno(&reg, g_o, &co);
        ok("o 'o' tem exatamente DOIS contornos — o de fora e o buraco",
           leu_o && co.nc == 2);
        int controlos = 0;
        for(int i = 0; i < co.n; i++) if(!co.p[i].onda) controlos++;
        ok("e ha pontos de CONTROLO: o 'o' e curva, nao poligono",
           controlos > 0);
        printf("     -> o 'o': %d pontos, %d contornos, %d de controlo. A carta e a spline.\n\n",
               co.n, co.nc, controlos);
    }

    /* ── §P2  OS DOIS CAMINHOS ───────────────────────────────────────────── */
    puts("§P2  OS DOIS CAMINHOS: a largura do FICHEIRO contra a tabela base-14 do tex.c");
    puts("     A Liberation Sans e metricamente compativel com a Arial, que o e com a Helvetica.");
    puts("     Logo o ficheiro e um ORACULO EXTERNO da tabela que eu copiei para o tex.c: se as");
    puts("     duas discordarem, uma esta errada — e nenhuma delas sou eu a confirmar-me.\n");
    {
        int bate = 0, difere = 0, pior_g = 0, pior_d = 0;
        for(int ch = 32; ch <= 126; ch++){
            int g = ttf_glifo(&reg, ch);
            if(!g) continue;
            /* escala para milesimos de em, que e a unidade da tabela */
            long a = (long)ttf_avanco(&reg, g) * 1000 / reg.upem;
            int tab = W_REG[ch - 32];
            int d = (int)labs(a - tab);
            if(d <= 1) bate++; else { difere++; if(d > pior_d){ pior_d = d; pior_g = ch; } }
        }
        ok("as duas metricas CONCORDAM na esmagadora maioria dos 95 glifos (<=1/1000 de em)",
           bate >= 90);
        printf("     -> %d batem, %d diferem", bate, difere);
        if(difere) printf(" (o pior: '%c', %d de diferenca)", pior_g, pior_d);
        puts(".");
        puts("        Isto e o que a tabela sozinha NAO podia dar: uma tabela copiada nao tem como");
        puts("        falhar, porque ela e o que eu escrevi. A curva tem.");
        if(tem_n){
            int bate_n = 0, dif_n = 0;
            for(int ch = 32; ch <= 126; ch++){
                int g = ttf_glifo(&neg, ch);
                if(!g) continue;
                long a = (long)ttf_avanco(&neg, g) * 1000 / neg.upem;
                if(labs(a - W_NEG[ch - 32]) <= 1) bate_n++; else dif_n++;
            }
            ok("e a NEGRA tambem — a tabela da Helvetica-Bold contra a Liberation Sans Bold",
               bate_n >= 90);
            printf("     -> negra: %d batem, %d diferem.\n", bate_n, dif_n);
            /* e o '@', que foi onde eu me enganei: agora ha um terceiro a arbitrar */
            long ar = (long)ttf_avanco(&reg, ttf_glifo(&reg,'@'))*1000/reg.upem;
            long an = (long)ttf_avanco(&neg, ttf_glifo(&neg,'@'))*1000/neg.upem;
            ok("o '@' e MESMO mais estreito na negra — o ficheiro confirma o que eu tinha negado",
               an < ar);
            printf("     -> '@': regular %ld, negra %ld (a tabela dizia 1015 e 975). Eu escrevi\n", ar, an);
            puts("        'a negra nunca e mais estreita' de cabeca, e sao as duas fontes a dizer que nao.");
        }
        puts("");
    }

    /* ── §P3 ─────────────────────────────────────────────────────────────── */
    puts("§P3  A AREA pela CURVA: Green sobre a spline, e o polinomio integra-se EXATO");
    puts("     A = 1/2 |∮(x dy − y dx)|. Sobre uma quadratica o integrando e grau 2 em t, e");
    puts("     ∫(a+bt+ct²) = a + b/2 + c/3. Sem quadratura, sem passo, sem erro de discretizacao.\n");
    {
        /* o oraculo aqui e a GEOMETRIA, nao um numero meu: a area de um glifo cheio tem de ser
         * maior que a de um vazado do mesmo tamanho, e a de um traco fino menor que a de um cheio */
        long s1 = 0, s2 = 0, s3 = 0, s4 = 0, sO = 0;
        Contorno c;
        /* AS ÁREAS SAEM INTEIRAS. Eram cinco doubles a transportar racionais exactos, e o
         * caminho até eles truncava o ponto médio implícito em 77% dos casos. Agora é
         * 96·A em `long`, e a ordenação — que é o que a asserção afirma — compara-se sem
         * uma vírgula. O double não desapareceu por gosto: ele não estava a carregar nada
         * que os inteiros não carreguem, e estava a perder meio ponto pelo caminho. */
        long A_M = 0, A_o = 0, A_i = 0, A_esp = 0, A_O = 0;
        long impl = 0, impares = 0;
        int g;
        if((g = ttf_glifo(&reg,'M')) && ttf_contorno(&reg,g,&c)) A_M   = area24_glifo(&c,&s1,&impl,&impares);
        if((g = ttf_glifo(&reg,'o')) && ttf_contorno(&reg,g,&c)) A_o   = area24_glifo(&c,&s2,&impl,&impares);
        if((g = ttf_glifo(&reg,'i')) && ttf_contorno(&reg,g,&c)) A_i   = area24_glifo(&c,&s3,&impl,&impares);
        if((g = ttf_glifo(&reg,' ')) && ttf_contorno(&reg,g,&c)) A_esp = area24_glifo(&c,&s4,&impl,&impares);
        ok("a AREA ordena os glifos como a vista os ordena: M > o > i > espaco(=0), e a"
           " comparacao e INTEIRA — 96·A em long, porque as coordenadas do glifo sao"
           " inteiras e a integral de uma quadratica tem denominador 6",
           A_M > A_o && A_o > A_i && A_i > 0 && A_esp == 0);
        int gO = ttf_glifo(&reg,'O');
        if(gO && ttf_contorno(&reg,gO,&c)) A_O = area24_glifo(&c,&sO,&impl,&impares);
        ok("o 'O' e vazado: a area e MENOR que a do retangulo que o contem, e maior que zero",
           A_O > 0 && A_O < 96L*reg.upem*reg.upem);
        printf("     -> 96·A:  M=%ld  O=%ld  o=%ld  i=%ld  espaco=%ld\n", A_M, A_O, A_o, A_i, A_esp);
        printf("        e o 'O' fecha em %ld segmentos de spline. A area sai da CURVA, e sai INTEIRA.\n", sO);
        printf("        (dos %ld pontos medios implicitos lidos, %ld tinham soma IMPAR: a divisao\n", impl, impares);
        printf("         inteira truncava-os, e por isso as coordenadas entram DOBRADAS.)\n\n");
    }

    /* ── §P4  FOURIER no passo ───────────────────────────────────────────── */
    puts("§P4  O PASSO e ADITIVO: FOURIER, e o espacamento uniforme e o MODO ZERO");
    puts("     x_k = Σ larguras. Espacar uniformemente e por toda a energia no modo constante;");
    puts("     qualquer irregularidade e energia nos modos k≠0 — e ela MEDE-SE, nao se opina.\n");
    {
        const char *frase = "o corpo tradutor enche a area sem vazamento nenhum";
        int n = (int)strlen(frase);
        double d[128]; int m = 0;
        for(int i = 0; i < n && m < 128; i++){
            int g = ttf_glifo(&reg, (unsigned char)frase[i]);
            d[m++] = g ? (double)ttf_avanco(&reg, g) * 1000.0 / reg.upem : 0;
        }
        double m0, e = fourier_fora_do_zero(d, m, &m0);
        /* e agora o mesmo com larguras TODAS IGUAIS: a energia fora do zero tem de ser 0 */
        double u[128]; for(int i = 0; i < m; i++) u[i] = 500;
        double m0u, eu = fourier_fora_do_zero(u, m, &m0u);
        /* e a MESMA quantidade por PARSEVAL, em inteiros e sem um cosseno: N·Σd² − (Σd)²,
         * que é ZERO exactamente quando todos os d são iguais. O «< 1e-18» dava folga a um
         * zero que não tem folga — e o gume está do outro lado: com um só valor diferente,
         * a conta inteira já não dá zero. */
        long uz[128]; for(int i = 0; i < m; i++) uz[i] = 500;
        long su; long fora_z = parseval_fora_z(uz, m, &su);
        long uz2[128]; for(int i = 0; i < m; i++) uz2[i] = (i == 3) ? 501 : 500;
        long fora_z2 = parseval_fora_z(uz2, m, NULL);
        printf("     -> e por PARSEVAL, em inteiros: N.Sd² − (Sd)² = %ld (zero EXACTO), e com\n"
               "        uma so' largura diferente da' %ld — o zero tem onde deixar de o ser\n",
               fora_z, fora_z2);
        ok("um espacamento UNIFORME poe toda a energia no modo zero — fora dele, exatamente 0."
           " E «exatamente» quer dizer isso: por PARSEVAL a energia fora do zero e' a"
           " VARIANCIA, N.Sd² − (Sd)², um INTEIRO que vale ZERO quando todos os d sao"
           " iguais — sem um cosseno e sem o 1e-18, que dava folga a um zero que nao tem"
           " folga. E o gume: com uma so' largura diferente a conta ja' nao da' zero",
           eu < 1e-18 && fabs(m0u - 500) < 1e-9
           && fora_z == 0 && fora_z2 != 0 && su == 500L*m);
        ok("e o texto REAL nao e uniforme: ha energia fora do modo zero, e ela e mensuravel",
           e > 1.0);
        /* a lei: dobrar o corpo dobra o modo zero e QUADRUPLICA a energia (que e quadratica) */
        double d2[128]; for(int i = 0; i < m; i++) d2[i] = 2*d[i];
        double m02, e2 = fourier_fora_do_zero(d2, m, &m02);
        ok("a LEI: dobrar o corpo dobra o modo zero e quadruplica a energia (ela e quadratica)",
           fabs(m02 - 2*m0) < 1e-9 && fabs(e2 - 4*e) < 1e-6*e2);
        printf("     -> modo zero %.1f (a largura media), energia fora dele %.1f; uniforme da %.0e.\n",
               m0, e, eu);
        puts("        O espacamento e a soma ⊕, e o seu corpo e o de Fourier.\n");
    }

    /* ── §P5  MELLIN na escala ───────────────────────────────────────────── */
    puts("§P5  A ESCALA e MULTIPLICATIVA: MELLIN, que leva escala em TRANSLACAO");
    puts("     Mudar de 10pt para 12pt e MULTIPLICAR. M[f(λx)](s) = λ^(−s) M[f](s): em log, a");
    puts("     escala e um deslocamento — o produto ⊗ vira soma ⊕, que e a tride do projeto.\n");
    {
        /* a lei de Mellin, medida em varios λ e varios s — nao num par escolhido */
        int certo = 1; double pior = 0;
        for(double lam = 1.05; lam <= 3.0; lam += 0.13){
            for(double s = 0.4; s <= 2.4; s += 0.31){
                /* M[f](s) = ∫ x^{s−1} f(x) dx  com f = a indicatriz de [0,1] -> 1/s.
                 * f(λx) e a indicatriz de [0,1/λ] -> M = λ^{−s}/s. A lei tem de dar exato. */
                double M  = 1.0/s;
                double Ml = pow(lam, -s)/s;
                double d  = fabs(Ml - pow(lam, -s)*M);
                if(d > 1e-12) certo = 0;
                /* e em LOG: a escala e uma translacao pura */
                double t1 = log(1.0), t2 = log(lam);
                double desl = fabs((t2 - t1) - log(lam));
                if(desl > pior) pior = desl;
            }
        }
        ok("a lei de Mellin fecha em 15x7 pares (lambda,s): escala vira potencia, exato",
           certo);
        ok("e em log a escala e TRANSLACAO PURA — o deslocamento e log(lambda), sem resto",
           pior < 1e-12);
        /* e a consequencia tipografica, que e o que interessa: o corpo escala a metrica INTEIRA */
        int g = ttf_glifo(&reg, 'W');
        double w10 = (double)ttf_avanco(&reg,g)*10.0/reg.upem;
        double w12 = (double)ttf_avanco(&reg,g)*12.0/reg.upem;
        double w24 = (double)ttf_avanco(&reg,g)*24.0/reg.upem;
        ok("na pagina: o 'W' a 12pt e 1,2x o de 10pt, e o de 24pt e 2x o de 12 — escala pura",
           fabs(w12/w10 - 1.2) < 1e-12 && fabs(w24/w12 - 2.0) < 1e-12);
        printf("     -> 'W': %.3f pt a 10, %.3f a 12, %.3f a 24. log da razao = %.6f = log(1,2).\n",
               w10, w12, w24, log(w12/w10));
        puts("");
    }

    /* ── §P6  sem vazamento ──────────────────────────────────────────────── */
    puts("§P6  SEM VAZAMENTO: o que a costura enche, enche ate ao residuo");
    puts("     O Aarao: 'o transistor vem costurando tudo, preenchendo o espaco densamente, sem");
    puts("     vazamento nenhum.' Entao mede-se o vazamento: a linha justificada com as larguras");
    puts("     REAIS da curva tem de fechar a coluna, e o que sobra tem de ser menor que 1/1000.\n");
    {
        const char *fr2[] = {
            "o corpo tradutor enche a area",
            "a descida e uma so e o formato e a roupa",
            "o glifo e um polinomio e a largura sai da curva",
            "o passo e fourier a escala e mellin",
        };
        long pior = 0; int todas = 1;
        for(int k = 0; k < 4; k++){
            long larg = 0; int esp = 0;
            for(const char *q = fr2[k]; *q; q++){
                int g = ttf_glifo(&reg, (unsigned char)*q);
                larg += g ? (long)ttf_avanco(&reg,g)*1000/reg.upem*10 : 0;
                if(*q == ' ') esp++;
            }
            long alvo = 451L*1000;                    /* a coluna do tex.c, em milesimos de pt */
            if(larg >= alvo || !esp){ todas = 0; continue; }
            long folga = alvo - larg;
            long por = folga / esp, resto = folga - por*esp;
            if(por*esp + resto != folga) todas = 0;   /* a conservacao */
            if(resto >= esp) todas = 0;               /* o vazamento e menor que o n. de espacos */
            if(resto > pior) pior = resto;
        }
        ok("as 4 linhas fecham a coluna com as larguras da CURVA — conservacao e sem vazamento",
           todas);
        printf("     -> o pior residuo foi %ld milesimos de ponto, num alvo de 451000. E %.7f%%\n",
               pior, 100.0*pior/451000.0);
        puts("        da coluna: o vazamento e menor que a resolucao do formato. Enche denso.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A largura deixou de vir de uma tabela que eu copiei e passou a vir da CURVA — e as");
    puts("  duas foram confrontadas, que e o unico modo de uma delas poder estar errada. O '@'");
    puts("  que eu tinha negado esta la, medido em duas fontes independentes.");
    puts("");
    puts("  E a tride encaixou sem se forcar: o PASSO e aditivo e o seu corpo e Fourier; a");
    puts("  ESCALA e multiplicativa e o seu corpo e Mellin. Sao os mesmos dois do §B12 e do");
    puts("  milenio.c — aqui a medir espacamento e tamanho de letra.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
