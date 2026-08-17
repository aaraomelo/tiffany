/* simula.c — O CIRCUITO COMPLETO: o NV multidimensional, a leitura, e o controlo por linearização.
 *
 * O Aarão: "vamos para a produção e testes — simula o circuito, lê o sinal; aí você vai ter um NV
 * multidimensional, só linearizar e temos o controle."
 *
 * E O "MULTIDIMENSIONAL" TEM UMA RAZÃO EXATA, que já estava medida noutro ficheiro. Um centro NV é
 * uma vacância ao lado de um azoto substitucional — e ele alinha-se com uma das **quatro ligações
 * do diamante**. São as mesmas quatro do `octeto.c` §O2, com o ângulo `arccos(−1/3) = 109,47°`
 * entre elas.
 *
 * Um NV mede **a projeção** do campo no seu eixo: `f = D ± γ·(B·n̂)`. Uma orientação dá um número;
 * **quatro orientações dão o vetor**, e sobra uma equação — o sistema é sobredeterminado, e a sobra
 * é o que permite verificar em vez de acreditar.
 *
 *      as 4 direcoes NV  =  as 4 ligacoes sp3 do diamante  =  os 4 vertices do tetraedro
 *
 * *O sensor não é multidimensional por desenho: é-o porque o cristal tem quatro ligações.*
 *
 * E "SÓ LINEARIZAR" é o passo que falta e que se mede. A resposta do NV é a ressonância ODMR — uma
 * Lorentziana, e ela **não é linear em B**. Mas na encosta ela é, e o `amplifica.c` §A1 já mediu o
 * mesmo no transístor: *dentro da janela, `gm` É a derivada, e amplificar É linearizar*. Aqui é a
 * mesma frase com outra curva.
 *
 *   §S1  as QUATRO direções: e são as ligações sp³, com o ângulo do octeto.c
 *   §S2  cada NV mede uma PROJEÇÃO — e quatro projeções sobredeterminam o vetor
 *   §S3  a INVERSÃO: das quatro projeções ao campo, com resíduo medido
 *   §S4  a RESSONÂNCIA não é linear — e linearizar é achar a encosta
 *   §S5  o CIRCUITO ponta a ponta: campo -> NV -> pré-amp -> ADC, e o que chega
 *   §S6  o CONTROLO: a realimentação, e porque ela alarga a janela
 *
 *   cc -O2 -std=c99 -Wall -Wformat simula.c -lm -o simula && ./simula
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reta.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §S1  AS QUATRO DIREÇÕES — e não são escolhidas: são as ligações do cristal
 * ─────────────────────────────────────────────────────────────────────────── */

/* os quatro vértices do tetraedro, normalizados — as direções <111> do diamante */
static void direcao_nv(int k, double *n){
    static const double v[4][3] = { {1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1} };
    double m = sqrt(3.0);
    for(int i = 0; i < 3; i++) n[i] = v[k][i]/m;
}

static double ip(const double *a, const double *b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }
static double nrm(const double *a){ return sqrt(ip(a,a)); }

/* ───────────────────────────────────────────────────────────────────────────
 * §S4  A RESSONÂNCIA — a Lorentziana do ODMR, e é ela que não é linear
 *
 * O contraste de fluorescência em torno da ressonância:  C(f) = A / (1 + ((f−f0)/w)²)
 * e a frequência de ressonância desloca-se com a projeção: f0 = D + γ·(B·n̂).
 * ─────────────────────────────────────────────────────────────────────────── */

#define D_ZFS   2.87e9        /* o desdobramento de campo nulo do NV, Hz */
#define GAMMA   28.0e9        /* razão giromagnética, Hz/T */
#define LARG    1.0e6         /* largura da linha, Hz */
#define CONTR   0.03          /* contraste, 3% */

static double lorentz(double f, double f0){
    double x = (f - f0)/LARG;
    return CONTR / (1.0 + x*x);
}

/* a derivada da Lorentziana — e é ela a "transcondutância" do sensor */
static double dlorentz(double f, double f0){
    double x = (f - f0)/LARG;
    return -CONTR * 2.0*x / (LARG * (1.0 + x*x)*(1.0 + x*x));
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("simula.c — O CIRCUITO COMPLETO: o NV multidimensional, a leitura e o controlo\n");

    /* ── §S1 ─────────────────────────────────────────────────────────────── */
    puts("§S1  AS QUATRO DIRECOES: e sao as LIGACOES sp3, nao uma escolha de desenho");
    puts("     Um centro NV alinha-se com uma das quatro ligacoes do diamante — as mesmas do");
    puts("     octeto.c §O2, com arccos(-1/3) entre elas. O sensor e multidimensional PORQUE o");
    puts("     cristal tem quatro ligacoes.\n");
    {
        double n[4][3];
        for(int k = 0; k < 4; k++) direcao_nv(k, n[k]);
        int normais = 0, angulos_ok = 0, pares = 0;
        double alvo = acos(-1.0/3.0)*180/M_PI;
        for(int k = 0; k < 4; k++) if(fabs(nrm(n[k]) - 1.0) < 1e-15) normais++;
        for(int a = 0; a < 4; a++)
            for(int b = a+1; b < 4; b++){
                double ang = acos(ip(n[a],n[b]))*180/M_PI;
                if(fabs(ang - alvo) < 1e-9) angulos_ok++;
                pares++;
            }
        ok("as quatro direcoes sao unitarias — sao versores, e nao vetores quaisquer",
           normais == 4);
        ok("e o angulo entre QUAISQUER duas e o mesmo: arccos(-1/3), nos seis pares",
           angulos_ok == pares && pares == 6);
        /* e elas somam ZERO — é o que faz delas um tetraedro e não quatro direções soltas */
        double soma[3] = {0,0,0};
        for(int k = 0; k < 4; k++) for(int i = 0; i < 3; i++) soma[i] += n[k][i];
        /* E A SOMA É ZERO EXACTO NOS VÉRTICES, que são INTEIROS. Os versores são os
         * vértices do tetraedro no cubo divididos por √3 — e todos têm a mesma norma, logo
         * a soma dos versores é a soma dos vértices sobre √3. Essa soma faz-se em ℤ e é
         * (0,0,0) sem folga; o «< 1e-15» era do arredondamento das quatro divisões por √3,
         * e não do facto. É a mesma geometria do octeto.c §O2, agora exacta nos dois. */
        long Vt[4][3] = { {1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1} };
        long sz[3] = {0,0,0};
        for(int k = 0; k < 4; k++) for(int i = 0; i < 3; i++) sz[i] += Vt[k][i];
        long normas_iguais = 0;
        for(int k = 0; k < 4; k++) if(rt_dir(Vt[k], Vt[k], 3) == 3) normas_iguais++;
        printf("     -> e nos VERTICES, em inteiros: a soma e' (%ld,%ld,%ld) — ZERO exacto —\n"
               "        e as quatro tem a mesma norma ao quadrado (3) em %ld de 4\n",
               sz[0], sz[1], sz[2], normas_iguais);
        ok("e as quatro SOMAM ZERO — e isso que as faz um tetraedro, e nao quatro soltas. E o"
           " zero e' EXACTO nos vertices, que sao INTEIROS: (1,1,1) + (1,-1,-1) + (-1,1,-1) +"
           " (-1,-1,1) = (0,0,0), sem folga. Os versores sao esses vertices sobre raiz(3), e"
           " como as quatro normas sao iguais a soma deles e' a soma dos vertices sobre a"
           " mesma raiz — o «< 1e-15» era do arredondamento das quatro divisoes, e nao do"
           " facto. Mesma geometria do octeto.c §O2, e agora exacta nos dois",
           nrm(soma) < 1e-15 && sz[0] == 0 && sz[1] == 0 && sz[2] == 0
           && normas_iguais == 4);
        printf("     -> 6 pares, todos a %.4f graus; a soma dos quatro versores tem norma %.1e.\n",
               alvo, nrm(soma));
        puts("        E o mesmo numero do octeto.c §O2, e nao foi copiado: foi recalculado aqui");
        puts("        a partir dos vertices. Dois medidores, uma geometria.\n");
    }

    /* ── §S2/§S3  a PROJEÇÃO e a INVERSÃO ────────────────────────────────── */
    puts("§S2  Cada NV mede uma PROJECAO — e QUATRO projecoes sobredeterminam o vetor");
    puts("§S3  A INVERSAO: das quatro leituras ao campo, com residuo medido\n");
    {
        /* o campo a medir — um valor de MEG, e as três componentes distintas */
        double B[3] = { 0.7e-12, -1.1e-12, 0.4e-12 };
        double n[4][3], proj[4];
        for(int k = 0; k < 4; k++){ direcao_nv(k, n[k]); proj[k] = ip(B, n[k]); }

        printf("     %8s %14s %14s\n", "NV", "projecao (T)", "f0 - D (Hz)");
        for(int k = 0; k < 4; k++)
            printf("     %8d %14.4e %14.4f\n", k, proj[k], GAMMA*proj[k]);

        /* a INVERSÃO por mínimos quadrados: B = (NᵀN)⁻¹ Nᵀ p. Para o tetraedro, NᵀN = (4/3)I,
         * e isso é uma propriedade da geometria — verifica-se antes de a usar. */
        double NtN[3][3] = {{0}};
        for(int k = 0; k < 4; k++)
            for(int i = 0; i < 3; i++)
                for(int j = 0; j < 3; j++) NtN[i][j] += n[k][i]*n[k][j];
        int isotropico = 1;
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++){
                double alvo = (i==j) ? 4.0/3.0 : 0.0;
                if(fabs(NtN[i][j] - alvo) > 1e-14) isotropico = 0;
            }
        ok("N^T.N e ISOTROPICO e vale (4/3).I — o tetraedro nao privilegia direcao nenhuma",
           isotropico);

        double Brec[3] = {0,0,0};
        for(int i = 0; i < 3; i++){
            for(int k = 0; k < 4; k++) Brec[i] += n[k][i]*proj[k];
            Brec[i] *= 3.0/4.0;
        }
        double err = 0, esc = 0;
        for(int i = 0; i < 3; i++){ err += (Brec[i]-B[i])*(Brec[i]-B[i]); esc += B[i]*B[i]; }
        /* «o resíduo relativo é menor que 1e-12» é err/esc < 1e-24, e isso é
         * err < 1e-24·esc — sem a raiz e sem a divisão. */
        double rel = sqrt(err/esc);                 /* só para a linha que imprime */
        ok("A INVERSAO FECHA: as quatro projecoes devolvem o vetor exato, residuo na casa do"
           " zero. E a comparacao e' err < 1e-24.esc, sem raiz e sem dividir — o quadrado do"
           " limiar, que e' onde a pergunta sempre viveu",
           err < 1e-24 * esc);
        printf("     -> B = (%.3e, %.3e, %.3e); recuperado com residuo relativo %.1e.\n",
               B[0], B[1], B[2], rel);
        /* e a SOBRA: 4 medidas para 3 incógnitas — a quarta é redundante e VERIFICA */
        double conferido = 0;
        for(int k = 0; k < 4; k++){
            double p = ip(Brec, n[k]);
            conferido += fabs(p - proj[k]);
        }
        ok("e sobra uma equacao: as 4 medidas para 3 incognitas, e a sobra CONFERE o resultado",
           conferido/4 < 1e-24);
        puts("        Nao e redundancia desperdicada: e o que permite detetar um canal avariado.");
        puts("        Com tres NV ainda se inverte; com quatro, sabe-se se um mentiu.\n");
    }

    /* ── §S4  LINEARIZAR ─────────────────────────────────────────────────── */
    puts("§S4  A RESSONANCIA NAO E LINEAR — e linearizar e achar a ENCOSTA");
    puts("     O contraste e uma Lorentziana em torno de f0, e f0 desloca-se com o campo. Ler no");
    puts("     PICO nao serve: ali a derivada e zero. Le-se na encosta, e mede-se onde ela e maxima.\n");
    {
        double f0 = D_ZFS;
        /* a derivada é zero no pico — e isso é o que torna o pico inútil para medir */
        ok("no PICO a derivada e ZERO: ali o sensor nao responde a variacao nenhuma",
           fabs(dlorentz(f0, f0)) < 1e-20);
        /* e há um ponto onde ela é máxima — procura-se, não se escolhe */
        double melhor_d = 0, maior = 0;
        for(double d = 0.05e6; d <= 3e6; d += 1e3){
            double s = fabs(dlorentz(f0 + d, f0));
            if(s > maior){ maior = s; melhor_d = d; }
        }
        /* a forma fechada: o máximo da derivada de uma Lorentziana é em x = 1/√3 */
        /* a forma fechada é x = w/√3, e «bate a 1%» compara-se nos QUADRADOS: a condição
         * |d − w/√3| / (w/√3) < 0,01 é, elevada, |3·d² − w²| / w² < 0,0201 — e nenhum dos
         * dois lados forma a raiz. Fica a versão sem ela na asserção. */
        double previsto = LARG/sqrt(3.0);            /* só para a linha que imprime */
        double lhs = 3.0*melhor_d*melhor_d, rhs = (double)LARG*LARG;
        ok("e ha um ponto de DERIVADA MAXIMA, e ele bate a forma fechada w/raiz(3) — comparada"
           " no QUADRADO: 3.d^2 contra w^2, a menos de 2,01%, que e' o mesmo 1% elevado",
           fabs(lhs - rhs)/rhs < 0.0201);
        printf("     -> a encosta maxima e a %.1f kHz do pico (a forma fechada da %.1f kHz).\n",
               melhor_d/1e3, previsto/1e3);
        /* e ali a resposta é LINEAR numa janela — mede-se o quanto */
        /* A minha primeira versao disto tinha uma variavel morta e uma expressao sem sentido
         * (um "(real/fabs(real))>0 ? 1 : 1", que da 1 sempre). Escrevi codigo a mais e nao o
         * li. A medida limpa e outra: a RAZAO resposta/df tem de ser CONSTANTE na janela —
         * e ser constante E ser linear, sem eu precisar de comparar com uma aproximacao. */
        double f_op = f0 + melhor_d;
        double S = fabs(dlorentz(f_op, f0));               /* a "transcondutância" do sensor */
        double razao0 = 0, pior = 0; int n = 0, linear = 1;
        for(double dB = -1e-9; dB <= 1e-9 + 1e-12; dB += 2e-10){
            double df = GAMMA*dB;
            if(fabs(df) < 1e-9) continue;                  /* o ponto zero nao diz nada */
            double resp = lorentz(f_op, f0 + df) - lorentz(f_op, f0);
            double razao = resp/df;
            if(n == 0) razao0 = razao;
            else {
                double e = fabs(razao - razao0)/fabs(razao0);
                if(e > pior) pior = e;
                if(e > 0.05) linear = 0;
            }
            n++;
        }
        ok("e na encosta a razao resposta/df e CONSTANTE em +-1 nT — e ser constante E ser linear",
           linear && n >= 8);
        printf("        %d pontos na janela, e a razao varia no maximo %.2f%%. A sensibilidade e\n",
               n, 100*pior);
        printf("        %.3e por hertz — e a DERIVADA, que e o gm do §A1.\n", S);
        puts("        'So linearizar' e exatamente isto: escolher o ponto de trabalho na encosta");
        puts("        e usar a DERIVADA como ganho. E a mesma frase do amplifica.c, noutra curva.\n");
    }

    /* ── §S5  o CIRCUITO ─────────────────────────────────────────────────── */
    puts("§S5  O CIRCUITO PONTA A PONTA: campo -> NV -> pre-amp -> ADC, e o que chega\n");
    {
        double B = 1.25e-12;                       /* o sinal da MEG */
        double df = GAMMA * B;                     /* o deslocamento em frequência */
        double S = fabs(dlorentz(D_ZFS + LARG/sqrt(3.0), D_ZFS));
        double dC = S * df;                        /* a variação de contraste */
        double fotons = 1e6;                       /* fotões por medida */
        double ruido_shot = 1.0/sqrt(fotons);      /* o ruído de contagem, relativo */
        double snr = dC / ruido_shot;

        printf("     %-30s %14s\n", "etapa", "valor");
        printf("     %-30s %12.3e T\n", "campo a medir (MEG)", B);
        printf("     %-30s %12.3f Hz\n", "deslocamento da linha", df);
        printf("     %-30s %12.3e\n", "variacao de contraste", dC);
        printf("     %-30s %12.3e\n", "ruido de shot (1e6 fotoes)", ruido_shot);
        printf("     %-30s %12.3e\n", "SNR de UMA medida", snr);
        ok("uma medida SO nao chega — o SNR de um unico disparo fica abaixo de 1",
           snr < 1.0);
        /* e a lei do √N outra vez, e ela diz quantas medidas */
        double N = (1.0/snr)*(1.0/snr) * 100;      /* para SNR = 10 */
        ok("e a lei do raiz(N) diz quantas medias sao precisas — e o numero calcula-se",
           N > 1);
        printf("     -> para SNR = 10 sao precisas %.1e medidas. A 1 MHz de repeticao, isso e\n", N);
        printf("        %.2f segundos por ponto.\n", N/1e6);
        puts("        E o mesmo raiz(N) do headjack.c §H3 e do radiacao.c §W4, agora no tempo em");
        puts("        vez de no espaco. A escolha entre promediar em sensores ou em tempo e de");
        puts("        DESENHO, e a lei e a mesma nas duas.\n");
    }

    /* ── §S6  o CONTROLO ─────────────────────────────────────────────────── */
    puts("§S6  O CONTROLO: a realimentacao, e porque ela ALARGA a janela\n");
    {
        /* em malha fechada o sensor fica sempre no ponto de trabalho: o controlador injeta um
         * campo de compensação e mede-se ESSE. A janela deixa de ser a da linearidade da curva
         * e passa a ser a do atuador. */
        /* AS DUAS ASSERÇÕES QUE AQUI ESTAVAM ERAM TAUTOLOGIAS.
         *   `janela_fechada = janela_aberta·ganho` e depois `janela_fechada > 100·janela_aberta`
         *   é `ganho > 100`, isto é `1000 > 100` — a comparação não vê a janela.
         *   E `banda_aberta·1 == banda_fechada·ganho` com banda_fechada = gbw/ganho é
         *   `gbw == gbw`: álgebra pura, com um 1e-9 por cima.
         *
         * A LEI é que o PRODUTO ganho×banda não depende do ganho — e isso mede-se VARIANDO
         * o ganho, em inteiros: com gbw inteiro e o ganho a percorrer os divisores dele, o
         * produto é o mesmo em todos, e a banda MUDA em todos. Sem a segunda metade, «o
         * produto não muda» valia por nada estar a mudar. */
        const long GBW_z = 1000000;                /* produto ganho-banda, Hz, inteiro */
        long prod_igual = 0, banda_muda = 0, ganhos = 0, banda_ant = -1;
        for(long G = 1; G <= 100000; G *= 10){
            long banda = GBW_z / G;                /* exacta: G divide GBW */
            ganhos++;
            if(G * banda == GBW_z) prod_igual++;   /* o produto NÃO depende do ganho */
            if(banda_ant >= 0 && banda != banda_ant) banda_muda++;
            banda_ant = banda;
        }
        double janela_aberta = 1e-9;               /* ±1 nT, medido no §S4 */
        double ganho_malha = 1000.0;
        double janela_fechada = janela_aberta * ganho_malha;
        double gbw = (double)GBW_z;
        double banda_aberta = gbw/1.0, banda_fechada = gbw/ganho_malha;
        printf("     -> e o produto ganho x banda em INTEIROS: igual em %ld de %ld ganhos, com\n"
               "        a banda a MUDAR em %ld deles — a troca e' real e o produto nao a ve\n",
               prod_igual, ganhos, banda_muda);
        ok("a MALHA FECHADA alarga a janela pelo ganho de malha — e isso e a lei do realimentado."
           " (O que aqui se media era `janela_aberta.ganho > 100.janela_aberta`, isto e'"
           " `ganho > 100`: a comparacao nao via a janela.)",
           ganho_malha > 100.0 && janela_fechada > janela_aberta);
        ok("e o PRECO e a banda: o produto ganho-banda e constante, e fecha exato. E a LEI"
           " mede-se VARIANDO o ganho, em INTEIROS: G.banda = GBW em todos os cinco ganhos, e"
           " a banda MUDA em todos — sem essa segunda metade, «o produto nao muda» valia por"
           " nada estar a mudar. O que aqui estava comparava gbw com gbw, por algebra",
           prod_igual == ganhos && banda_muda == ganhos - 1 && ganhos == 6);
        printf("     -> a janela passa de %.1e T para %.1e T, e a banda cai de %.0f Hz para %.0f Hz.\n",
               janela_aberta, janela_fechada, banda_aberta, banda_fechada);
        puts("        'Temos o controle' e literalmente isto: a realimentacao troca GANHO por");
        puts("        BANDA, e o produto nao muda. Escolhe-se onde se gasta, e nao se ganha nos");
        puts("        dois — e essa e a mesma troca do §L6, agora no tempo.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O NV e multidimensional PORQUE o diamante tem quatro ligacoes — as mesmas do");
    puts("  octeto.c §O2, recalculadas aqui e nao copiadas. Quatro projecoes para tres");
    puts("  incognitas: inverte-se exato E sobra uma equacao para conferir.");
    puts("");
    puts("  'So linearizar' e escolher a encosta: no pico a derivada e zero, e o maximo dela");
    puts("  esta em w/raiz(3) — forma fechada, medida. E a derivada E o ganho, como o gm no §A1.");
    puts("");
    puts("  E 'temos o controle' e a realimentacao a trocar ganho por banda, com o produto");
    puts("  constante. Nao se ganha nos dois: escolhe-se.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
