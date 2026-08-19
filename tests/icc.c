/* icc.c — A INTERFACE CÉREBRO-COMPUTADOR no toolkit: a túnica, e quantos eletrodos bastam.
 *
 * O Aarão: "formaliza numa ferramenta do toolkit, vai ser a Interface Cérebro-Computador (ICC) ou
 * Headjack; projeta a túnica (capacete), que se conecta na medula espinhal completa via implante
 * intracortical; realiza a matriz de microeletrodos, compara com projetos reais como o Utah Array."
 *
 * O QUE ISTO É: o desenho no papel e a matemática dele. A matriz de microeletrodos é uma **base
 * amostrada**, a túnica é o **par de torres** (a aferente desce, a eferente sobe), e o
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
 *   §I2  NYQUIST ESPACIAL: o passo tem de ser menor que meio período, e isso é 2·passo < período
 *   §I3  o UTAH ARRAY pelos números públicos — e onde ele cai na conta
 *   §I4  a TÚNICA: as duas torres, aferente e eferente, e elas são ADJUNTAS
 *   §I5  a ida-e-volta: resíduo 0 com a base completa, e a curva da dose
 *   §I6  o que isto é, e o que não é
 *
 * LEI vs TRANSPORTE. passo < período/2 em vírgula, cos da onda, interpolação e resíduo 1e-12
 * eram o método. A lei é 2·passo < período em ℤ, k mudanças de sinal, posto 1 pelos menores
 * 2×2 = 0, e ⟨Af,c⟩ = ⟨f,Aᵀc⟩ exacto na matriz 0-1.
 *
 *   cc -O2 -std=c99 -I lib tests/icc.c -o icc && ./icc
 */
#include <stdio.h>
#include "unidade.h"

#define LADO  16

/* Nyquist espacial em ℤ: o passo tem de ser menor que meio período. */
static int nyquist(long passo, long periodo){
    return 2 * passo < periodo;
}

/* k meias-ondas no lado: k mudanças de sinal, sem cos. */
static void meias(int *g, int n, int k){
    for(int i = 0; i < n; i++){
        int s = 1;
        for(int t = 1; t <= k; t++) if(i >= (t * n) / (k + 1)) s = -s;
        g[i] = s;
    }
}

typedef struct { const char *nome; int nx, ny; long passo, comp, area; } Dispositivo;

static const Dispositivo REAIS[] = {
    { "Utah Array",      10, 10, 400,  1500, 16 },
    { "Utah (48 ch)",     6,  8, 400,  1000,  7 },
    { "Michigan probe",   1, 16,  50,  5000,  0 },
    { "Neuropixels 1.0",  2, 480,  20, 10000,  0 },
};
#define NREAIS ((int)(sizeof REAIS / sizeof REAIS[0]))

#define COLUNA      500
#define MINICOLUNA   50

int main(void){
    puts("icc.c — A INTERFACE CEREBRO-COMPUTADOR: a tunica, e quantos eletrodos bastam\n");

    /* ── §I2 ─────────────────────────────────────────────────────────────── */
    puts("§I2  NYQUIST ESPACIAL: 2·passo < periodo, em ℤ");
    puts("     Nao e criterio de engenharia — e o teorema da amostragem, e ele falha de forma");
    puts("     verificavel. Mede-se nos dois lados: cumpre e viola.\n");
    {
        int acima = 0, abaixo = 0, casos = 0;
        printf("     %10s %12s %10s\n", "passo", "periodo", "Nyquist");
        long periodo = 1000;
        long passos[2] = { 400, 600 };            /* 800<1000 cumpre; 1200>1000 viola */
        for(int ip = 0; ip < 2; ip++){
            long passo = passos[ip];
            int ny = nyquist(passo, periodo);
            printf("     %10ld %12ld %10s\n", passo, periodo, ny ? "cumpre" : "viola");
            if(ny) acima++; else abaixo++;
            casos++;
        }
        ok("quando 2·passo < periodo CUMPRE; quando 2·passo >= periodo, VIOLA",
           acima == 1 && abaixo == 1 && casos == 2);

        /* FREQUENCIA: k meias-ondas dao k mudancas de sinal. Mutar k na formula
         * e nao na contagem e o gume — o cos() escondia isto atras da malha. */
        {
            int bate = 0, testados = 0;
            printf("      k pedido   mudancas de sinal em x   mudancas em y\n");
            for(int k = 1; k <= 5; k++){
                int gx[LADO], gy[LADO];
                meias(gx, LADO, k);
                meias(gy, LADO, 1);
                int f[LADO][LADO];
                for(int i = 0; i < LADO; i++)
                    for(int j = 0; j < LADO; j++) f[i][j] = gx[i] * gy[j];
                int trocas_x = 0, trocas_y = 0;
                for(int i2 = 1; i2 < LADO; i2++)
                    if(f[i2][0] != f[i2-1][0]) trocas_x++;
                for(int j2 = 1; j2 < LADO; j2++)
                    if(f[0][j2] != f[0][j2-1]) trocas_y++;
                printf("      %-10d %-24d %d\n", k, trocas_x, trocas_y);
                if(trocas_x == k && trocas_y == 1) bate++;
                testados++;
            }
            printf("\n");
            ok("o campo tem a FREQUENCIA pedida: k meias-ondas dao k mudancas de sinal, em 5 valores de k",
               bate == testados && testados == 5);
        }

        /* SEPARABILIDADE: f = g(x)h(y) e posto 1 — todo menor 2x2 anula. A soma nao. */
        {
            int gx[LADO], gy[LADO];
            int f[LADO][LADO], g[LADO][LADO];
            meias(gx, LADO, 3);
            meias(gy, LADO, 2);
            for(int i = 0; i < LADO; i++)
                for(int j = 0; j < LADO; j++){
                    f[i][j] = gx[i] * gy[j];         /* posto 1 */
                    g[i][j] = gx[i] + gy[j];         /* a soma, o gume */
                }
            const int B = 12;
            int menores = 0, nulos = 0, nulos_soma = 0;
            for(int i = 0; i < B; i++) for(int k = i+1; k < B; k++)
            for(int j = 0; j < B; j++) for(int l = j+1; l < B; l++){
                long prod = (long)f[i][j]*f[k][l] - (long)f[i][l]*f[k][j];
                long soma = (long)g[i][j]*g[k][l] - (long)g[i][l]*g[k][j];
                menores++;
                if(prod == 0) nulos++;
                if(soma == 0) nulos_soma++;
            }
            int previsto = (B*(B-1)/2)*(B*(B-1)/2);
            printf("      SEPARABILIDADE: %d menores 2x2 (C(%d,2)^2 = %d), nulos no produto: %d\n",
                   menores, B, previsto, nulos);
            printf("      e na SOMA gx+gy os nulos sao %d — posto 1 distingue as duas\n\n", nulos_soma);
            ok("o campo e SEPARAVEL, f(x,y) = g(x).h(y): todo menor 2x2 anula — posto 1",
               nulos == menores && menores == previsto && nulos_soma < menores);
        }
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
            int nc = nyquist(d->passo, COLUNA);
            int nm = nyquist(d->passo, MINICOLUNA);
            cobre_coluna += nc; cobre_mini += nm;
            printf("     %-18s %5d %8ldu %9s %11s   %s\n", d->nome, d->nx*d->ny, d->passo,
                   nc ? "SIM" : "nao", nm ? "SIM" : "nao",
                   nc ? (nm ? "as duas" : "so colunas") : "sub-amostra");
        }
        /* o Utah: 400 contra o limite 250. Se Nyquist fosse passo < periodo (sem o 2),
         * 400 < 500 «cumpriria» e esta ok cairia — e o gume. */
        ok("o UTAH ARRAY sub-amostra as colunas corticais: passo 400 contra o limite de 250",
           REAIS[0].passo > COLUNA/2 && !nyquist(REAIS[0].passo, COLUNA));
        ok("e o NEUROPIXELS cumpre-o com folga: passo 20, e ate as minicolunas de 50",
           nyquist(REAIS[3].passo, COLUNA) && nyquist(REAIS[3].passo, MINICOLUNA));
        printf("     -> %d de %d dispositivos cumprem Nyquist para colunas; %d para minicolunas.\n",
               cobre_coluna, NREAIS, cobre_mini);
        puts("        Isto nao e critica ao Utah Array: ele nao foi desenhado para reconstruir");
        puts("        o campo, foi para isolar neuronios um a um, e nisso e excelente. Mas para");
        puts("        a TRANSFUSAO — que precisa da volta exata — a conta e esta, e ela e dura.\n");
        ok("dois cobrem colunas (Michigan+Neuropixels) e so o Neuropixels cobre minicolunas",
           cobre_coluna == 2 && cobre_mini == 1);
    }

    /* ── §I4  A TÚNICA ───────────────────────────────────────────────────── */
    puts("§I4  A TUNICA: as duas torres, e elas sao ADJUNTAS");
    puts("     A aferente DESCE (o campo entra, e o sistema le); a eferente SOBE (o sistema");
    puts("     escreve, e o campo sai). Sao o par do §B12, e a adjuncao mede-se.\n");
    {
        /* A 0-1: eletrodo e le o ponto 2e. ⟨Af,c⟩ = ⟨f,Aᵀc⟩ em ℤ, exacto.
         * O gume e Aᵀ errada (deslocar um indice): as somas deixam de bater. */
        enum { N = 8, NC = 4 };
        long A[NC][N];
        for(int e = 0; e < NC; e++)
            for(int p = 0; p < N; p++) A[e][p] = (p == 2*e) ? 1 : 0;
        long f[N] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        long c[NC] = { 3, -1, 4, 2 };
        long Af[NC], ATc[N];
        for(int e = 0; e < NC; e++){
            long s = 0; for(int p = 0; p < N; p++) s += A[e][p] * f[p];
            Af[e] = s;
        }
        for(int p = 0; p < N; p++){
            long s = 0; for(int e = 0; e < NC; e++) s += A[e][p] * c[e];
            ATc[p] = s;
        }
        long e1 = 0, e2 = 0;
        for(int e = 0; e < NC; e++) e1 += Af[e] * c[e];
        for(int p = 0; p < N; p++)  e2 += f[p] * ATc[p];
        /* transposta deslocada: nao e adjunta */
        long e2_mau = 0;
        for(int p = 0; p < N; p++){
            long s = 0; for(int e = 0; e < NC; e++) s += A[e][(p+1)%N] * c[e];
            e2_mau += f[p] * s;
        }
        ok("a AFERENTE e a EFERENTE sao adjuntas: <A f, c> = <f, A^T c>, ZERO EXACTO em ℤ",
           e1 == e2);
        ok("e o gume segura: deslocar A^T um indice parte a adjuncao",
           e2_mau != e1);
        printf("     -> <Af,c> = %ld e <f,A'c> = %ld (deslocada %ld).\n", e1, e2, e2_mau);
        puts("        E o mesmo par do robo.c (J e J^T) e do §B12: uma torre le, a outra escreve,");
        puts("        e a adjuncao e o que garante que o que se escreve e o que se leu.\n");
    }

    /* ── §I5  a ida-e-volta ──────────────────────────────────────────────── */
    puts("§I5  A IDA-E-VOLTA: quantos canais para fechar com residuo 0");
    puts("     A pergunta do reconstroi.c noutra coordenada. La foram n+2 termos; aqui e uma");
    puts("     condicao de amostragem, e tem forma fechada.\n");
    {
        int k = 4;                                   /* 4 meias-ondas no lado */
        long periodo = 2L * LADO / k;                /* 8 celulas */
        int orig[LADO];
        meias(orig, LADO, k);
        printf("     campo com %d meias-ondas: periodo %ld celulas, limite de Nyquist %ld\n\n",
               k, periodo, periodo/2);
        printf("     %8s %10s %12s %10s\n", "canais", "passo", "residuo", "Nyquist");
        long ant = -1, casos_n = 0, nao_sobe = 0, zeros = 0, viola_e_falha = 0, violou = 0;
        for(int nx = 2; nx <= LADO; nx *= 2){
            long passo = LADO / nx;
            int canais[LADO];
            for(int a = 0; a < nx; a++){
                int i = (2*a + 1) * LADO / (2*nx);
                if(i >= LADO) i = LADO - 1;
                canais[a] = orig[i];
            }
            long res = 0;
            for(int i = 0; i < LADO; i++){
                int a = i * nx / LADO;
                if(a >= nx) a = nx - 1;
                long d = canais[a] - orig[i];
                res += d*d;
            }
            int ny = nyquist(passo, periodo);
            printf("     %8d %10ld %12ld %10s\n", nx, passo, res, ny ? "cumpre" : "viola");
            casos_n++;
            if(ant >= 0 && res <= ant) nao_sobe++;
            if(res == 0) zeros++;
            if(!ny){ violou++; if(res > 0) viola_e_falha++; }
            ant = res;
        }
        ok("com a grelha completa o residuo e ZERO; violar Nyquist deixa residuo",
           zeros >= 1 && viola_e_falha == violou && violou > 0);
        ok("e o residuo nao SOBE a cada duplicacao de canais",
           casos_n == 4 && nao_sobe == casos_n - 1);
        /* Nyquist necessario: 2·passo >= periodo implica residuo > 0.
         * Suficiente para vizinho na grelha inteira: nx = LADO, passo = 1. */
        ok("Nyquist e NECESSARIO: violar o passo deixa residuo, sem excecao",
           violou > 0 && viola_e_falha == violou);
        ok("e com um eletrodo por celula a volta fecha — residuo 0, sem 1e-12",
           zeros >= 1);
        puts("        Nyquist diz que a informacao esta la; recupera-la com vizinho pede a");
        puts("        grelha completa. O criterio de engenharia e mais duro que o teorema.\n");
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

    printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
    return falhas != 0;
}
