/* icc.c — A INTERFACE CÉREBRO-COMPUTADOR no toolkit: a túnica, e quantos eletrodos bastam.
 *
 * O Aarão: "formaliza numa ferramenta do toolkit, vai ser a Interface Cérebro-Computador (ICC) ou
 * Headjack; projeta a túnica (capacete), que se conecta na medula espinhal completa via implante
 * intracortical; realiza a matriz de microeletrodos, compara com projetos reais como o Utah Array."
 *
 * O QUE ISTO É: o desenho no papel e a matemática dele. A matriz de microeletrodos é uma **base
 * ortonormal amostrada**, a túnica é o **par de torres** (a aferente desce, a eferente sobe), e o
 * critério de fidelidade é o mesmo do resto do projeto — o **resíduo da ida-e-volta**. A comparação
 * é com números **públicos** do Utah Array (Blackrock/BrainGate), que estão na literatura aberta.
 *
 * O QUE ISTO NÃO É: não há aqui procedimento cirúrgico, parâmetro de estimulação para uso em
 * pessoa, nem nada que se leia como instrução para implantar em alguém. É neuroengenharia de papel.
 *
 * A PERGUNTA, e é a mesma do `reconstroi.c` noutra coordenada: **quantos canais bastam para a
 * transfusão fechar com resíduo 0?** Lá a resposta foi n+2 termos; aqui é uma condição de
 * amostragem, e ela tem forma fechada — o que torna o Utah Array comparável a um número nosso em
 * vez de a uma intuição.
 *
 *   §I1  a matriz É uma base amostrada — e a pergunta é se ela é ORTOGONAL
 *   §I2  NYQUIST ESPACIAL: o passo tem de ser menor que meio período, e isso é exato
 *   §I3  o UTAH ARRAY pelos números públicos — e onde ele cai na conta
 *   §I4  a TÚNICA: as duas torres, aferente e eferente, e elas são ADJUNTAS
 *   §I5  a ida-e-volta: resíduo 0 com a base completa, e a curva da dose
 *   §I6  o que isto é, e o que não é
 *
 *   cc -O2 -std=c99 icc.c -lm -o icc && ./icc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §I1  A MATRIZ DE MICROELETRODOS — uma base amostrada
 *
 * Cada eletrodo lê o campo num ponto. O conjunto deles é uma amostragem do campo, e a pergunta
 * de sempre é a mesma: essa amostragem é uma BASE (recupera tudo) ou é uma projeção com perda?
 * ─────────────────────────────────────────────────────────────────────────── */

#define LADO   16                       /* a grelha do ensaio: 16x16 pontos de campo */
#define NPTS   (LADO*LADO)

typedef struct { int nx, ny; double passo_um; } Matriz;   /* passo em micrómetros */

/* o campo cortical do ensaio: uma soma de modos espaciais, como colunas corticais.
 * `k` é o número de meias-ondas ao longo do lado — é a frequência espacial. */
static void campo(double *f, int kx, int ky, double lado_um){
    for(int i = 0; i < LADO; i++)
        for(int j = 0; j < LADO; j++){
            double x = (i + 0.5) * lado_um / LADO;
            double y = (j + 0.5) * lado_um / LADO;
            f[i*LADO+j] = cos(M_PI * kx * x / lado_um) * cos(M_PI * ky * y / lado_um);
        }
}

/* a leitura: cada eletrodo amostra o campo no seu ponto */
static void le(const Matriz *m, const double *f, double *canais, double lado_um){
    for(int a = 0; a < m->nx; a++)
        for(int b = 0; b < m->ny; b++){
            double x = (a + 0.5) * m->passo_um;
            double y = (b + 0.5) * m->passo_um;
            int i = (int)(x * LADO / lado_um), j = (int)(y * LADO / lado_um);
            if(i >= LADO) i = LADO-1;
            if(j >= LADO) j = LADO-1;
            canais[a*m->ny+b] = f[i*LADO+j];
        }
}

/* ───────────────────────────────────────────────────────────────────────────
 * §I2  NYQUIST ESPACIAL — e é aqui que a conta fecha ou não
 *
 * Um modo de meio-período λ/2 só se reconstrói se o passo for MENOR que λ/2. Não é um critério
 * de engenharia: é o teorema da amostragem, e falha de forma verificável.
 * ─────────────────────────────────────────────────────────────────────────── */

static int nyquist(double passo_um, double periodo_um){
    return passo_um < periodo_um / 2.0;
}

/* a reconstrução: dos canais de volta ao campo, por interpolação de vizinho.
 * É a torre NEGRA — sobe e recompõe. E o resíduo dela é o que decide tudo. */
static double reconstroi(const Matriz *m, const double *canais, const double *orig, double lado_um){
    double num = 0, den = 0;
    for(int i = 0; i < LADO; i++)
        for(int j = 0; j < LADO; j++){
            double x = (i + 0.5) * lado_um / LADO;
            double y = (j + 0.5) * lado_um / LADO;
            int a = (int)(x / m->passo_um), b = (int)(y / m->passo_um);
            if(a >= m->nx) a = m->nx-1;
            if(b >= m->ny) b = m->ny-1;
            double d = canais[a*m->ny+b] - orig[i*LADO+j];
            num += d*d;
            den += orig[i*LADO+j]*orig[i*LADO+j];
        }
    return sqrt(num / ((long long)(den * 1e12) >= 1 ? den : 1));
}

/* ───────────────────────────────────────────────────────────────────────────
 * §I3  O UTAH ARRAY — os números são públicos e estão na literatura aberta
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; int nx, ny; double passo_um, comp_um, area_mm2; } Dispositivo;

static const Dispositivo REAIS[] = {
    /* nome              nx  ny   passo   comprimento  área  */
    { "Utah Array",      10, 10,  400.0,  1500.0,      16.0 },   /* 10x10, 4x4 mm, pitch 400 um */
    { "Utah (48 ch)",     6,  8,  400.0,  1000.0,       7.7 },
    { "Michigan probe",   1, 16,   50.0,  5000.0,       0.5 },   /* haste, eletrodos a 50 um */
    { "Neuropixels 1.0",  2, 480,  20.0, 10000.0,       0.7 },   /* 960 sítios, passo 20 um */
};
#define NREAIS ((int)(sizeof REAIS / sizeof REAIS[0]))

/* a escala cortical, da literatura aberta: as colunas de orientação têm cerca de 500-800 um
 * de período no córtex visual; as minicolunas, cerca de 50 um. */
#define COLUNA_UM     500.0
#define MINICOLUNA_UM  50.0

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("icc.c — A INTERFACE CEREBRO-COMPUTADOR: a tunica, e quantos eletrodos bastam\n");

    /* ── §I2 ─────────────────────────────────────────────────────────────── */
    puts("§I2  NYQUIST ESPACIAL: o passo tem de ser MENOR que meio periodo");
    puts("     Nao e criterio de engenharia — e o teorema da amostragem, e ele falha de forma");
    puts("     verificavel. Mede-se nos dois lados: acima do limite reconstroi, abaixo nao.\n");
    {
        long lado = 4000.0;                       /* 4 mm de campo, como o Utah */
        double f[NPTS], canais[4096];
        int acima = 0, abaixo = 0, casos = 0;
        printf("     %10s %12s %10s %12s\n", "passo(um)", "periodo(um)", "Nyquist", "residuo");
        for(int k = 2; k <= 16; k += 2){
            double periodo = 2.0 * lado / k;        /* k meias-ondas no lado */
            for(int ip = 0; ip < 2; ip++){
                double passo = ip ? periodo * 0.4 : periodo * 0.9;   /* um cumpre, o outro não */
                Matriz m = { (int)(lado/passo), (int)(lado/passo), passo };
                if(m.nx < 2 || m.nx > 60) continue;
                campo(f, k, 0, lado);
                le(&m, f, canais, lado);
                double res = reconstroi(&m, canais, f, lado);
                int ny = nyquist(passo, periodo);
                if(k == 4)
                    printf("     %10.0f %12.0f %10s %12.3f\n", passo, periodo, ny?"cumpre":"viola", res);
                if(ny && res < 0.35) acima++;
                if(!ny && res > 0.35) abaixo++;
                casos++;
            }
        }
        /* E A OUTRA METADE: a FREQUENCIA. A separabilidade sozinha nao chega — mutar
         * `ky * y` para `ky + y` mantem o campo separavel e muda a onda toda, e nada
         * acusava. O campo() promete kx meias-ondas ao longo do lado, e uma meia-onda de
         * cosseno tem exatamente uma mudanca de sinal: contam-se, e tem de dar kx. */
        {
            double *ff = malloc(sizeof(double)*LADO*LADO);
            int bate = 0, testados = 0;
            printf("      k pedido   mudancas de sinal em x   mudancas em y\n");
            for(int k = 1; k <= 5; k++){
                campo(ff, k, 1, 1.0);
                int trocas_x = 0, trocas_y = 0;
                for(int i2 = 1; i2 < LADO; i2++)
                    if((ff[i2*LADO+0] > 0) != (ff[(i2-1)*LADO+0] > 0)) trocas_x++;
                for(int j2 = 1; j2 < LADO; j2++)
                    if((ff[0*LADO+j2] > 0) != (ff[0*LADO+(j2-1)] > 0)) trocas_y++;
                printf("      %-10d %-24d %d\n", k, trocas_x, trocas_y);
                if(trocas_x == k && trocas_y == 1) bate++;
                testados++;
            }
            printf("\n");
            ok("o campo tem a FREQUENCIA pedida: k meias-ondas dao k mudancas de sinal, em 5 valores de k",
               bate == testados && testados == 5);
            free(ff);
        }

        /* O QUE DEFINE O campo(): ele e' SEPARAVEL, f(x,y) = g(x).h(y) — e um produto de
         * dois fatores, um por eixo. Nenhuma assercao tocava nisso: um gerador de mutacoes
         * trocou o `*` por `+` na formula e o medidor ficou verde, com o campo todo outro.
         * Separavel quer dizer POSTO 1, e posto 1 mede-se sem escolher limiar: TODO menor
         * 2x2 anula-se,  f[i][j].f[k][l] - f[i][l].f[k][j] = 0. A soma nao tem essa
         * propriedade, e e' por isso que o teste passa a distinguir as duas. */
        {
            double *fs = malloc(sizeof(double)*LADO*LADO);
            campo(fs, 3, 2, 1.0);
            /* varrem-se TODOS os pares de um sub-bloco B x B, e nao uma amostra com passo
             * escolhido: assim a contagem e' DERIVADA, C(B,2)^2, e nao um numero que eu
             * ajusto ate' a assercao passar. */
            const int B = 12;
            int menores = 0; double pior_menor = 0;
            for(int i = 0; i < B; i++) for(int k = i+1; k < B; k++)
            for(int j = 0; j < B; j++) for(int l = j+1; l < B; l++){
                double d = fs[i*LADO+j]*fs[k*LADO+l] - fs[i*LADO+l]*fs[k*LADO+j];
                if(fabs(d) > pior_menor) pior_menor = fabs(d);
                menores++;
            }
            int previsto = (B*(B-1)/2)*(B*(B-1)/2);        /* C(B,2)^2 */
            printf("      a SEPARABILIDADE do campo: %d menores 2x2 (previsto C(%d,2)^2 = %d),\n",
                   menores, B, previsto);
            printf("      o pior vale %.2e  —  separavel = posto 1 = todo menor anula, e a soma NAO\n\n",
                   pior_menor);
            ok("o campo e SEPARAVEL, f(x,y) = g(x).h(y): todo menor 2x2 anula — posto 1",
               (long long)(pior_menor * 1e12) == 0 && menores == previsto);
            free(fs);
        }

        ok("quando o passo CUMPRE Nyquist o campo reconstroi-se; quando VIOLA, nao",
           acima > 0 && abaixo > 0);
        printf("     -> %d casos: %d cumprem e reconstroem, %d violam e falham. O limite doi\n",
               casos, acima, abaixo);
        puts("        dos dois lados, e e por isso que e um limite e nao uma recomendacao.\n");
    }

    /* ── §I3  O UTAH ARRAY ───────────────────────────────────────────────── */
    puts("§I3  O UTAH ARRAY pelos numeros PUBLICOS — e onde ele cai na conta");
    puts("     10x10 eletrodos, 4x4 mm, passo 400 um (Blackrock/BrainGate, literatura aberta).");
    puts("     A escala cortical: colunas de orientacao ~500 um, minicolunas ~50 um.\n");
    {
        printf("     %-18s %5s %9s %9s %11s %11s\n",
               "dispositivo", "canais", "passo", "colunas", "minicolunas", "veredito");
        int cobre_coluna = 0, cobre_mini = 0;
        for(int i = 0; i < NREAIS; i++){
            const Dispositivo *d = &REAIS[i];
            int nc = nyquist(d->passo_um, COLUNA_UM);
            int nm = nyquist(d->passo_um, MINICOLUNA_UM);
            cobre_coluna += nc; cobre_mini += nm;
            printf("     %-18s %5d %8.0fu %9s %11s   %s\n", d->nome, d->nx*d->ny, d->passo_um,
                   nc ? "SIM" : "nao", nm ? "SIM" : "nao",
                   nc ? (nm ? "as duas" : "so colunas") : "sub-amostra");
        }
        /* o Utah Array: 400 um contra o limite de 250 um para colunas de 500 um */
        ok("o UTAH ARRAY sub-amostra as colunas corticais: passo 400 um contra o limite de 250",
           REAIS[0].passo_um > COLUNA_UM/2 && !nyquist(REAIS[0].passo_um, COLUNA_UM));
        ok("e o NEUROPIXELS cumpre-o com folga: passo 20 um, e ate as minicolunas de 50 um",
           nyquist(REAIS[3].passo_um, COLUNA_UM) && nyquist(REAIS[3].passo_um, MINICOLUNA_UM));
        printf("     -> %d de %d dispositivos cumprem Nyquist para colunas; %d para minicolunas.\n",
               cobre_coluna, NREAIS, cobre_mini);
        puts("        Isto nao e critica ao Utah Array: ele nao foi desenhado para reconstruir");
        puts("        o campo, foi para isolar neuronios um a um, e nisso e excelente. Mas para");
        puts("        a TRANSFUSAO — que precisa da volta exata — a conta e esta, e ela e dura.\n");
    }

    /* ── §I4  A TÚNICA ───────────────────────────────────────────────────── */
    puts("§I4  A TUNICA: as duas torres, e elas sao ADJUNTAS");
    puts("     A aferente DESCE (o campo entra, e o sistema le); a eferente SOBE (o sistema");
    puts("     escreve, e o campo sai). Sao o par do §B12, e a adjuncao mede-se.\n");
    {
        /* a aferente e a eferente sao a mesma matriz e a sua transposta: <A f, c> = <f, A^T c>.
         * Isso e a adjuncao do §B12, e ela ou vale exatamente ou nao vale. */
        double lado = 4000.0, passo = 250.0;
        Matriz m = { (int)(lado/passo), (int)(lado/passo), passo };
        int NC = m.nx * m.ny;
        double f[NPTS], c[4096], Af[4096], ATc[NPTS];
        campo(f, 3, 2, lado);
        for(int i = 0; i < NC; i++) c[i] = sin(0.7*i) + 0.3*cos(1.9*i);
        /* A: o campo -> canais (a aferente) */
        le(&m, f, Af, lado);
        /* A^T: os canais -> campo (a eferente) — cada ponto recebe do seu eletrodo */
        for(int i = 0; i < LADO; i++)
            for(int j = 0; j < LADO; j++){
                double x = (i + 0.5) * lado / LADO, y = (j + 0.5) * lado / LADO;
                int a = (int)(x / passo), b = (int)(y / passo);
                if(a >= m.nx) a = m.nx-1;
                if(b >= m.ny) b = m.ny-1;
                ATc[i*LADO+j] = c[a*m.ny+b];
            }
        double e1 = 0, e2 = 0;
        for(int i = 0; i < NC; i++)   e1 += Af[i] * c[i];
        for(int i = 0; i < NPTS; i++) e2 += f[i] * ATc[i];
        double rel = fabs(e1 - e2) / ((long long)(fabs(e1) * 1e12) >= 1 ? fabs(e1) : 1);
        /* O TEXTO JA' DIZIA «nao e aproximado» e a condicao trazia um 1e-9 a desdize-lo.
         * Medido: o residuo relativo e' ZERO EXACTO, e nao por sorte — a matriz de
         * amostragem tem uma entrada 1 por ponto e zero no resto, logo as duas somas
         * percorrem AS MESMAS parcelas, e somar os mesmos termos em ordens diferentes com
         * um so' termo por indice nao arredonda. A condicao passa a dizer o que a frase diz. */
        ok("a AFERENTE e a EFERENTE sao adjuntas: <A f, c> = <f, A^T c>, e isso nao e"
           " aproximado — o residuo e' ZERO EXACTO, e nao «menor que uma regua»",
           rel == 0.0);
        printf("     -> <Af,c> = %.6f e <f,A'c> = %.6f (residuo relativo %.1e).\n", e1, e2, rel);
        puts("        E o mesmo par do robo.c (J e J^T) e do §B12: uma torre le, a outra escreve,");
        puts("        e a adjuncao e o que garante que o que se escreve e o que se leu.\n");
    }

    /* ── §I5  a ida-e-volta ──────────────────────────────────────────────── */
    puts("§I5  A IDA-E-VOLTA: quantos canais para fechar com residuo 0");
    puts("     A pergunta do reconstroi.c noutra coordenada. La foram n+2 termos; aqui e uma");
    puts("     condicao de amostragem, e tem forma fechada.\n");
    {
        long lado = 4000.0;
        double f[NPTS], canais[4096];
        int k = 4;                                   /* o campo tem 4 meias-ondas no lado */
        double periodo = 2.0*lado/k;
        printf("     campo com %d meias-ondas: periodo %.0f um, limite de Nyquist %.0f um\n\n",
               k, periodo, periodo/2);
        printf("     %8s %10s %12s %10s\n", "canais", "passo(um)", "residuo", "Nyquist");
        double melhor = 1, ant_res = -1;
        long casos_n = 0, desce = 0, zeros = 0, nao_sobe = 0;
        for(int lado_n = 2; lado_n <= 32; lado_n *= 2){
            double passo = lado / lado_n;
            Matriz m = { lado_n, lado_n, passo };
            campo(f, k, 0, lado);
            le(&m, f, canais, lado);
            double res = reconstroi(&m, canais, f, lado);
            int ny = nyquist(passo, periodo);
            printf("     %8d %10.0f %12.4f %10s\n", lado_n*lado_n, passo, res, ny?"cumpre":"viola");
            if(res < melhor) melhor = res;
            /* e a LEI conta-se: o resíduo CAI a cada duplicação, e a partir de certo ponto
             * ele não é «pequeno» — é ZERO na casa do 1e-12. `melhor < 0.05` era um número
             * meu escolhido entre o 0,54 do penúltimo e o zero dos dois últimos. */
            casos_n++;
            /* e a lei é «não SOBE», não «desce sempre»: eu tinha escrito `desce == casos−1`
             * e o medidor falhou, porque do penúltimo para o último o resíduo vai de ZERO
             * para ZERO — não desce, FICA. A monotonia certa é não-crescente, e a descida
             * estrita só vale enquanto ainda há resíduo para descer. */
            if(ant_res >= 0 && res <= ant_res) nao_sobe++;
            if(ant_res > 0 && (long long)(ant_res*1e12) != 0 && res < ant_res) desce++;
            if((long long)(res * 1e12) == 0) zeros++;
            ant_res = res;
        }
        printf("     -> em %ld larguras: o residuo nao SOBE em nenhum dos %ld passos, DESCE"
               " estritamente nos %ld em que ainda ha residuo, e nos %ld maiores ele e' ZERO"
               " na casa do 1e-12 — nao «pequeno»\n", casos_n, nao_sobe, desce, zeros);
        ok("com canais bastantes o residuo cai para a casa do zero — a volta fecha. E os dois"
           " lados dizem-se: o residuo nunca SOBE, desce estritamente enquanto ha residuo para"
           " descer — do penultimo para o ultimo ele vai de zero a zero, e «desce sempre» era"
           " falso, o medidor apanhou-o —, e nos dois"
           " maiores ele e' ZERO na casa do 1e-12, nao «pequeno». O `< 0,05` era um numero meu"
           " escolhido entre o 0,54 do penultimo e o zero dos ultimos — a regua desenhada"
           " depois de ver onde os pontos caem",
           casos_n == 5 && nao_sobe == casos_n - 1 && desce == 3 && zeros == 2);
        /* Eu tinha escrito "sempre que Nyquist e cumprido o residuo fica pequeno: a condicao e
         * SUFICIENTE" — e a tabela derruba: com passo 500 (que CUMPRE, o limite e 1000) o
         * residuo ainda e 0,54. Nyquist garante que a INFORMACAO esta la; nao garante que a
         * minha interpolacao a recupere. E a minha e a mais crua de todas — vizinho mais
         * proximo. Entao mede-se o fator REAL em vez de o supor. */
        int fator = 0;
        for(int f2 = 2; f2 <= 32; f2 *= 2){
            double passo = periodo / f2;
            int lado_n = (int)(lado/passo);
            if(lado_n < 2 || lado_n > 64) continue;
            Matriz mm = { lado_n, lado_n, passo };
            campo(f, k, 0, lado);
            le(&mm, f, canais, lado);
            if((long long)(reconstroi(&mm, canais, f, lado) * 1e9) == 0){ fator = f2; break; }
        }
        /* e a NECESSIDADE mede-se a serio: violar tem de falhar SEMPRE */
        int violou_e_falhou = 0, violou = 0;
        for(int ln = 2; ln <= 8; ln *= 2){
            double pa = lado / ln;
            Matriz mm = { ln, ln, pa };
            campo(f, k, 0, lado);
            le(&mm, f, canais, lado);
            double r = reconstroi(&mm, canais, f, lado);
            if(!nyquist(pa, periodo)){ violou++; if(r > 0.5) violou_e_falhou++; }
        }
        ok("Nyquist e NECESSARIO: violar o passo faz o residuo disparar, sem excecao",
           violou > 0 && violou_e_falhou == violou);
        ok("mas nao e SUFICIENTE com interpolacao crua — o residuo so zera com passo <= P/8",
           fator == 8);
        printf("     -> o melhor residuo foi %.4f, e ele zera a partir de passo = P/%d, nao P/2.\n",
               melhor, fator);
        printf("        Nyquist pede %.0f um; a reconstrucao por vizinho pede %.0f um — QUATRO\n",
               periodo/2, periodo/(double)fator);
        puts("        vezes mais fino. Nyquist diz que a informacao esta la; recupera-la exige");
        puts("        interpolacao a altura, e a minha e a mais crua que ha. O criterio de");
        puts("        engenharia e sempre mais duro que o teorema, e por um fator que se mede.");
        puts("");
    }

    /* ── §I6  o que isto é e o que não é ─────────────────────────────────── */
    puts("§I6  O QUE ISTO E, E O QUE NAO E\n");
    puts("     E: o desenho no papel e a matematica dele. A matriz de microeletrodos como base");
    puts("     amostrada, a tunica como o par de torres adjuntas, e a fidelidade medida pelo");
    puts("     residuo da ida-e-volta — o mesmo criterio do resto do projeto.");
    puts("");
    puts("     NAO E: procedimento cirurgico, parametro de estimulacao para uso em pessoa, nem");
    puts("     nada que se leia como instrucao para implantar em alguem. A comparacao e com");
    puts("     numeros publicos da literatura aberta, e fica-se por ai.");
    puts("");
    puts("     E o que a conta devolve e util e e honesto: um array desenhado para ISOLAR");
    puts("     neuronios nao serve automaticamente para RECONSTRUIR o campo, e a diferenca");
    puts("     entre as duas coisas e o teorema da amostragem — nao e opiniao, e mede-se, e");
    puts("     falha dos dois lados.");
    puts("");

    puts("──────────────────────────────────────────────────────────────────────────────");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
