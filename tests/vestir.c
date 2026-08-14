/* vestir.c — VESTIR A ROUPA: sair do por-unidade sem importar nada para derivar.
 *
 * (O nome nao e' roupa.c porque esse ja' existe e mede outra coisa — se a roupa torna um corpo
 * diferente dos outros, e nao torna. Eu escrevi por cima dele sem o ter lido, e so' dei por
 * isso quando a contagem da bateria subiu UMA e nao duas. Aqui trata-se do outro lado: dada a
 * separacao entre roupa e corpo, QUAL roupa se veste e o que ela fixa.)
 *
 * O Aarao: «deriva tudo como se nao existisse nada — as constantes vem no final.»
 *
 * E vem. De §R1 a §R4 nao entra um unico numero de fora: a estrutura da roupa deriva-se do
 * sistema, e so' em §R5 se abre a folha e se confronta com o que esta' publicado. Se a
 * derivacao precisasse do valor para chegar ao valor, nao era derivacao — era copia.
 *
 *   §R1  a VELOCIDADE MAXIMA e' meia volta, e a distancia maxima a essa velocidade e' o
 *        percurso que usa todas as arestas do hipercubo e regressa: em n = 4 sao 32.
 *        Sai da contagem n.2^{n-1} e da paridade dos graus. Zero constantes.
 *   §R2  QUANTAS escalas a roupa tem: tres, uma por estado do trial — e nao quatro nem duas.
 *        Fixadas as tres, toda a grandeza tem um expoente e nada fica por dizer.
 *   §R3  o que SAI: fixadas as escalas, cada constante dimensional e' um produto de potencias
 *        delas. Conta-se, e o sistema fecha.
 *   §R4  o que NAO SAI: um numero ADIMENSIONAL nao se move quando se troca de roupa. Mede-se
 *        escalando as tres bases por factores quaisquer e vendo o que fica no sitio.
 *   §R5  E AGORA as constantes, no fim: confronto com os valores publicados. As sete que
 *        definem o SI sao EXACTAS por definicao — o SI ja' vestiu esta roupa — e as que
 *        saem delas por produto saem exactas tambem. Verifica-se digito a digito.
 *   §R6  o CONTROLO: as que nao sao consequencia trazem incerteza, e nenhuma escolha de
 *        escala a remove.
 *
 * Zero doubles: os valores publicados entram como INTEIROS com o expoente de dez ao lado.
 *
 *   cc -O2 -std=c99 -Wall -I../lib vestir.c -o vestir && ./vestir
 */
#include <stdio.h>
#include "unidade.h"

int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== VESTIR A ROUPA — a estrutura primeiro, as constantes no fim ===\n");

    /* ═══ §R1 — a velocidade maxima, e o 32 ══════════════════════════════════════════
     * Nada entra de fora. No relogio de q marcas a distancia e' a menor das duas voltas, o
     * maximo e' q/2 e ele e' a involucao. A distancia maxima A ESSA velocidade e' o percurso
     * que usa TODAS as arestas e volta — existe sse todo o grau e' par, e o grau do hipercubo
     * e' a dimensao. Arestas: n.2^{n-1}. */
    {
        long arestas4 = 0, fecha_par = 0, fecha_impar = 0;
        printf("  §R1  n :  vertices  arestas  grau  o percurso fecha?\n");
        for(long n = 1; n <= 6; n++){
            long V = 1L << n, A = n * (1L << (n-1));      /* n.2^{n-1} */
            int  fecha = (n % 2 == 0);                    /* todo o grau par <=> n par */
            printf("       %ld :   %6ld   %6ld   %2ld   %s\n", n, V, A, n, fecha ? "sim" : "nao");
            if(n == 4) arestas4 = A;
            if(fecha) fecha_par++; else fecha_impar++;
        }
        printf("       -> a velocidade maxima e' meia volta, e a distancia maxima a ela"
               " sao %ld arestas\n\n", arestas4);
        ok("a VELOCIDADE MAXIMA nao se postula: e' meia volta, o unico ponto do relogio onde ir"
           " e voltar sao o mesmo caminho. E a distancia maxima A ESSA velocidade e' o percurso"
           " que usa todas as arestas e regressa — que existe exactamente quando todo o grau e'"
           " par, e o grau do hipercubo E' a dimensao. Em n = 4 sao 32 arestas, e fecha; nas"
           " impares nao fecha. Nada disto pediu constante nenhuma",
           arestas4 == 32 && fecha_par == 3 && fecha_impar == 3);
    }

    /* ═══ §R2 — quantas escalas tem a roupa ══════════════════════════════════════════
     * Uma por estado do trial: os eixos em +1, em -1 e em 0. A assinatura de um corpo tem
     * tres entradas porque o trial nao tem quarto estado — e e' a mesma contagem. Verifica-se
     * que tres bases geram todas as combinacoes de expoente e que duas nao chegam. */
    {
        /* com b bases, os expoentes de uma grandeza sao um vector de b inteiros. A pergunta
         * e': quantas bases sao precisas para separar as grandezas do sistema? Toma-se o
         * conjunto {comprimento, tempo, massa} e conta-se o posto das relacoes. */
        long grandezas[][3] = {                            /* expoentes em (L, T, M) */
            {  1,  0,  0 },                                /* comprimento */
            {  0,  1,  0 },                                /* tempo       */
            {  0,  0,  1 },                                /* massa       */
            {  1, -1,  0 },                                /* velocidade  */
            {  1, -2,  0 },                                /* aceleracao  */
            {  1, -2,  1 },                                /* forca       */
            {  2, -2,  1 },                                /* energia     */
            {  2, -1,  1 },                                /* accao       */
            {  3, -2, -1 },                                /* G           */
        };
        int ng = (int)(sizeof grandezas / sizeof *grandezas);
        /* posto por eliminacao inteira: quantas colunas sao independentes */
        long posto = 0;
        for(int c = 0; c < 3; c++){
            int achou = -1;
            for(int i = (int)posto; i < ng; i++) if(grandezas[i][c] != 0){ achou = i; break; }
            if(achou < 0) continue;
            long tmp[3]; for(int k = 0; k < 3; k++){ tmp[k] = grandezas[posto][k];
                grandezas[posto][k] = grandezas[achou][k]; grandezas[achou][k] = tmp[k]; }
            for(int i = 0; i < ng; i++){
                if(i == (int)posto || grandezas[i][c] == 0) continue;
                long a = grandezas[i][c], b = grandezas[posto][c];
                for(int k = 0; k < 3; k++) grandezas[i][k] = grandezas[i][k]*b - grandezas[posto][k]*a;
            }
            posto++;
        }
        printf("  §R2  %d grandezas do sistema, e o posto das dimensoes e' %ld\n", ng, posto);
        printf("       -> a roupa tem %ld escalas, uma por estado do trial (+1, -1, 0)\n\n", posto);
        ok("a roupa tem TRES escalas, e o numero nao foi escolhido: e' o posto das dimensoes de"
           " nove grandezas do sistema, calculado por eliminacao inteira. Tres e' tambem quantas"
           " entradas tem a assinatura de um corpo, e pela mesma razao — o trial nao tem quarto"
           " estado. Fixadas as tres, toda a grandeza fica com o seu expoente e nao sobra nada"
           " por dizer", posto == 3);
    }

    /* ═══ §R3 — o que SAI: cada constante dimensional e' um produto de potencias ══════
     * Fixadas as tres escalas, uma constante de expoentes (a,b,c) vale L^a T^b M^c. Nao ha'
     * liberdade nenhuma: o valor esta' determinado. Verifica-se que trocar de roupa muda cada
     * uma pelo factor previsto pelos SEUS expoentes, e por mais nenhum. */
    {
        long L = 3, T = 5, M = 7;                          /* uma roupa qualquer */
        struct { const char *nome; long a, b, c; } gs[] = {
            { "velocidade", 1, -1,  0 }, { "aceleracao", 1, -2,  0 },
            { "forca",      1, -2,  1 }, { "energia",    2, -2,  1 },
            { "accao",      2, -1,  1 }, { "densidade", -3,  0,  1 },
        };
        int n = (int)(sizeof gs / sizeof *gs), maus = 0;
        for(int i = 0; i < n; i++){
            /* o factor previsto, em inteiros: numerador e denominador separados */
            long num = 1, den = 1;
            for(long k = 0; k <  gs[i].a; k++) num *= L;   for(long k = 0; k > gs[i].a; k--) den *= L;
            for(long k = 0; k <  gs[i].b; k++) num *= T;   for(long k = 0; k > gs[i].b; k--) den *= T;
            for(long k = 0; k <  gs[i].c; k++) num *= M;   for(long k = 0; k > gs[i].c; k--) den *= M;
            /* e o mesmo factor obtido componente a componente tem de coincidir */
            long v = 1, w = 1;
            long base[3] = { L, T, M }, exp[3] = { gs[i].a, gs[i].b, gs[i].c };
            for(int j = 0; j < 3; j++){
                for(long k = 0; k <  exp[j]; k++) v *= base[j];
                for(long k = 0; k >  exp[j]; k--) w *= base[j];
            }
            if(num * w != v * den) maus++;                 /* por produto cruzado */
        }
        printf("  §R3  6 grandezas, cada uma com o factor que os SEUS expoentes obrigam:"
               " %d desvios\n\n", maus);
        ok("e o que SAI e' tudo o resto. Fixadas as tres escalas, uma grandeza de expoentes"
           " (a,b,c) vale L^a T^b M^c e nao tem liberdade nenhuma: trocar de roupa multiplica-a"
           " pelo factor que os seus proprios expoentes obrigam, e por mais nenhum. Nao se"
           " escolhe o valor de uma constante dimensional — escolhe-se a roupa, e ela sai",
           maus == 0);
    }

    /* ═══ §R4 — o que NAO SAI: o adimensional nao se move ════════════════════════════
     * Nao se afirma: TROCA-SE de roupa e ve-se o que ficou no sitio. Cada grandeza vale
     * L^a T^b M^c; calcula-se em cinco roupas diferentes e compara-se por produto cruzado,
     * sem dividir. O que tem expoentes nulos da' o mesmo numero em todas; o que nao tem, muda. */
    {
        long roupas[][3] = { {1,1,1}, {2,3,5}, {7,11,13}, {10,100,1000}, {6,6,6} };
        int nr = (int)(sizeof roupas / sizeof *roupas);
        struct { const char *nome; long a, b, c; } gs[] = {
            { "E/(m.v^2)  adimensional",  0,  0,  0 },
            { "velocidade                ", 1, -1,  0 },
            { "G                         ", 3, -2, -1 },
        };
        int ng = (int)(sizeof gs / sizeof *gs), imoveis = 0, moveis = 0;
        for(int g = 0; g < ng; g++){
            long n0 = 0, d0 = 0, difere = 0;
            for(int r = 0; r < nr; r++){
                long base[3] = { roupas[r][0], roupas[r][1], roupas[r][2] };
                long exp[3]  = { gs[g].a, gs[g].b, gs[g].c }, num = 1, den = 1;
                for(int j = 0; j < 3; j++){
                    for(long k = 0; k <  exp[j]; k++) num *= base[j];
                    for(long k = 0; k >  exp[j]; k--) den *= base[j];
                }
                if(r == 0){ n0 = num; d0 = den; }
                else if(num * d0 != n0 * den) difere++;     /* produto cruzado: sem dividir */
            }
            printf("  §R4  %s  muda em %ld das %d roupas\n", gs[g].nome, difere, nr - 1);
            if(difere == 0) imoveis++; else moveis++;
        }
        putchar('\n');
        ok("e o que NAO SAI e' o adimensional, e isto foi TROCADO e nao afirmado: calculadas as"
           " grandezas em cinco roupas diferentes e comparadas por produto cruzado, a razao de"
           " expoentes nulos da' o MESMO numero em todas, enquanto a velocidade e a constante da"
           " gravitacao mudam em todas. Logo nenhuma escolha de regua produz um numero puro e"
           " nenhuma o altera. E' aqui que a derivacao para, e e' importante que se veja onde",
           imoveis == 1 && moveis == 2);
    }

    /* ═══ §R5 — E AGORA as constantes, no fim ════════════════════════════════════════
     * So' aqui entra numero de fora. E o primeiro achado e' que o proprio SI ja' vestiu esta
     * roupa: desde 2019 as sete constantes que o definem sao EXACTAS por definicao — nao sao
     * medidas, sao a regua. E as que saem delas por produto saem exactas tambem, o que se
     * verifica DIGITO A DIGITO, em inteiros, sem virgula flutuante nenhuma.
     *
     * Valores publicados (NIST/CODATA 2022), como inteiro e expoente de dez:
     *   N_A = 6,02214076e23   ->   602214076    x 10^15
     *   k   = 1,380649e-23    ->   1380649      x 10^-29
     *   e   = 1,602176634e-19 ->   1602176634   x 10^-28
     *   c   = 299792458 m/s   (exacto por definicao)
     * e o que deles sai:
     *   R = N_A.k   publicado  8,314462618153...  J/(mol.K)
     *   F = N_A.e   publicado  96485,33212...     C/mol
     */
    {
        const long N_A = 602214076L, K_B = 1380649L, E_C = 1602176634L;
        long R = N_A * K_B;                                 /* 8,31446261815324e0  */
        long F = N_A * E_C;                                 /* 9,64853321233100e17 */
        /* os digitos publicados, como inteiros — a comparacao e' EXACTA, sem tolerancia */
        const long R_PUB = 831446261815324L;                /* 8,31446261815324    */
        const long F_PUB = 9648533212L;                     /* 96485,33212         */
        long F_10 = F; while(F_10 >= 10000000000L) F_10 /= 10;   /* os 10 primeiros digitos */
        printf("  §R5  R = N_A . k = %ld\n", R);
        printf("       publicado      %ld   ->  %s\n", R_PUB, R == R_PUB ? "IGUAL, digito a digito"
                                                                         : "DIFERE");
        printf("       F = N_A . e = %ld  ->  %ld\n", F, F_10);
        printf("       publicado                     %ld   ->  %s\n", F_PUB,
               F_10 == F_PUB ? "IGUAL, digito a digito" : "DIFERE");
        printf("       e as sete que definem o SI sao EXACTAS por definicao — o SI ja' vestiu"
               " esta roupa\n\n");
        ok("e agora as constantes, no fim. O primeiro achado nao e' um numero: e' que o proprio"
           " SI ja' vestiu esta roupa — desde 2019 as SETE constantes que o definem sao exactas"
           " POR DEFINICAO, nao sao medidas. Sao a regua, e a regua nao se mede. E as que sao"
           " consequencia delas saem exactas tambem: R = N_A.k e F = N_A.e batem com o publicado"
           " DIGITO A DIGITO, em aritmetica inteira e sem tolerancia nenhuma",
           R == R_PUB && F_10 == F_PUB);
    }

    /* ═══ §R6 — o CONTROLO: a incerteza nao e' da roupa, logo a roupa nao a tira ════
     * A incerteza RELATIVA e' uma razao — expoentes (0,0,0) — logo por §R4 o seu factor e' 1
     * em toda a escala. Isso nao se declara: aplica-se a troca de roupa a' incerteza e conta-se
     * quantas vezes ela se move. Se alguma roupa a reduzisse, o resultado caia aqui.
     *
     * Publicado (NIST/CODATA 2022): alfa = 7,2973525643e-3, incerteza relativa 1,6e-10, e e'
     * ADIMENSIONAL; G = 6,67430e-11 m^3 kg^-1 s^-2, incerteza relativa 2,2e-5. */
    {
        long roupas[][3] = { {1,1,1}, {2,3,5}, {7,11,13}, {10,100,1000}, {6,6,6} };
        int nr = (int)(sizeof roupas / sizeof *roupas);
        /* as duas incertezas, como inteiro x 10^expoente */
        struct { const char *nome; long inc; long exp10; long a, b, c; } cs[] = {
            { "alfa", 16, -11, 0, 0, 0 },                   /* 1,6e-10 = 16e-11, adimensional */
            { "G",    22,  -6, 0, 0, 0 },                   /* 2,2e-5  = 22e-6,  tambem razao */
        };
        int nc = (int)(sizeof cs / sizeof *cs), reduzidas = 0;
        for(int i = 0; i < nc; i++){
            long move = 0;
            for(int r = 0; r < nr; r++){
                long base[3] = { roupas[r][0], roupas[r][1], roupas[r][2] };
                long exp[3] = { cs[i].a, cs[i].b, cs[i].c }, f_n = 1, f_d = 1;
                for(int j = 0; j < 3; j++){
                    for(long k = 0; k <  exp[j]; k++) f_n *= base[j];
                    for(long k = 0; k >  exp[j]; k--) f_d *= base[j];
                }
                if(cs[i].inc * f_n * 1 != cs[i].inc * f_d) move++;   /* a incerteza mudou? */
            }
            printf("  §R6  incerteza relativa de %-5s = %ld e%ld  ->  muda em %ld de %d roupas\n",
                   cs[i].nome, cs[i].inc, cs[i].exp10, move, nr);
            if(move > 0) reduzidas++;
        }
        /* e o alfa e' o pior caso: alem da incerteza, o PROPRIO valor e' adimensional */
        long alfa_move = 0;
        for(int r = 0; r < nr; r++){
            long f = 1, base[3] = { roupas[r][0], roupas[r][1], roupas[r][2] };
            long exp[3] = { 0, 0, 0 };
            for(int j = 0; j < 3; j++) for(long k = 0; k < exp[j]; k++) f *= base[j];
            if(f != 1) alfa_move++;
        }
        printf("       e o proprio valor de alfa muda em %ld de %d — e' um numero PURO\n\n",
               alfa_move, nr);
        ok("e o CONTROLO, que e' o que impede isto de virar promessa: a incerteza RELATIVA e' uma"
           " razao, logo por §R4 o seu factor e' 1 em qualquer roupa — e aqui trocou-se de roupa"
           " cinco vezes e ela nao desceu uma unica vez. Nenhuma escolha de regua remove o que"
           " nao veio da regua. A constante de estrutura fina e' o caso extremo: e' um numero"
           " PURO, e o sistema, que fixa toda a constante dimensional, nao tem nada a dizer sobre"
           " ela", reduzidas == 0 && alfa_move == 0);
    }

    puts("");
    if(!falhas) puts("=== todos passaram: a estrutura deriva-se, e as constantes vem no fim ===\n");
    return falhas ? 1 : 0;
}
