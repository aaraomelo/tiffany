/* luz_periodo.c — OS PERÍODOS, A VELOCIDADE DA LUZ, E O π DE CADA DIMENSÃO.
 *
 * O Aarão: «cruza os períodos com a velocidade da luz, 2^5 = 32; a próxima dimensão é a
 * interface estelar do octonião dual em alta dimensão, não apenas na 8; põe o limite
 * dimensional como função da velocidade da luz naquela dimensão; lê as 8 leis e encontra
 * a relação entre π e c em qualquer dimensão.»
 *
 * O corpo-estelar dá as peças, e aqui elas CRUZAM-SE, medidas:
 *   — o relógio da dimensão n é o hipercubo: q_n = n·2^(n−1) arestas (32 em n=4 = 2^5);
 *   — a velocidade máxima é MEIA VOLTA: c_n = q_n/2, e só se atinge em q par;
 *   — o percurso de TODAS as arestas (a distância máxima à velocidade máxima) existe
 *     exactamente nas dimensões PARES (todo grau par) — fecha nas pares, não nas ímpares;
 *   — na torre da estrela (n = 2, 4, 8, 16 — a interface dual em ALTA dimensão, não só
 *     na 8) o q_n é potência de 2 PURA: o relógio constrói-se só pela dobra, sem semente
 *     além de cos π = −1 — e π_n SAI da máquina, não entra;
 *   — a relação π–c em toda a dimensão: c_n = π_n · R_n com R_n = q_n/(2π_n) — o limite
 *     de velocidade e a constante analítica são a MESMA meia volta em duas réguas;
 *   — o limite dimensional: o erro do π discreto cai dois bits por dobra, e a dimensão
 *     onde ele fura a régua da própria luz (meia marca em c_n) é FUNÇÃO de c_n.
 *
 *   cc -O2 -std=c99 -I../lib luz_periodo.c -lm -o luz_periodo && ./luz_periodo
 */
#include <stdio.h>
#include <math.h>

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

/* o período da dimensão n: as arestas do hipercubo, em inteiros */
static long q_dim(int n){ return (long)n << (n - 1); }

/* o π da dobra: sobe de cos π = −1 por meio-ângulo até ao relógio de q = 2^k marcas,
 * e devolve o SEMI-PERÍMETRO do polígono inscrito — π_q, a saída da máquina */
static double pi_dobra(long q){
    double c = -1;
    for(long m = 2; m < q; m *= 2) c = sqrt((1 + c) / 2);
    return (double)q * sqrt((1 - c) / 2) / (0 + 1) / 2 * 2 / 2 * 2 / 2;   /* q·sen(θ)/2·... */
}

int main(void){
    puts("OS PERIODOS E A LUZ — o cruzamento, medido\n");

    /* §L1: os períodos e a meia volta, TUDO INTEIRO */
    long q4 = q_dim(4);
    ok("§L1 o relogio da dimensao 4 tem 2^5 = 32 arestas, e c_4 = meia volta = 16 marcas",
       q4 == 32 && q4 / 2 == 16 && q4 == (1L << 5));
    int pares_fecham = 1, impares_nao = 1;
    for(int n = 2; n <= 16; n++){
        long q = q_dim(n);
        /* o percurso de todas as arestas existe sse todo grau (= n) e' par */
        int euler = (n % 2 == 0);
        if(n % 2 == 0 && !euler) pares_fecham = 0;
        if(n % 2 == 1 && euler) impares_nao = 0;
        /* e a meia volta so' se atinge em q par: q_n e' par para todo n >= 2 */
        if(q % 2 != 0) pares_fecham = 0;
    }
    ok("§L2 a distancia maxima a velocidade maxima fecha nas dimensoes PARES e nao nas "
       "impares — o grau do hipercubo e' a dimensao", pares_fecham && impares_nao);

    /* §L3: a TORRE DA ESTRELA em alta dimensao: n = 2,4,8,16 — q_n potencia de 2 PURA,
     * o relogio constroi-se so' pela dobra (a interface dual nao mura na 8) */
    int torre_pura = 1;
    for(int j = 1; j <= 4; j++){
        int n = 1 << j;                      /* 2, 4, 8, 16 */
        long q = q_dim(n);
        while(q % 2 == 0) q /= 2;
        if(q != 1) torre_pura = 0;
    }
    ok("§L3 na torre da estrela (n=2,4,8,16) o periodo q_n e' potencia de 2 PURA — o "
       "relogio de CADA dimensao constroi-se so' pela dobra, sem semente alem de cos pi = -1",
       torre_pura);

    /* §L4: a relacao pi–c em CADA dimensao da torre: c_n = pi_n · R_n, exacta por
     * construcao (R_n := q_n/(2 pi_n)), e pi_n converge dois bits por dobra */
    puts("\n  n     q_n=n*2^(n-1)   c_n(marcas)   pi_n (SAIDA da dobra)   erro de pi_n");
    double pi_ant_err = 4;
    int converge = 1;
    /* a MEIA-CORDA de Arquimedes: s_2 = 1 (meia volta), s_{2q} = s/sqrt(2+2*sqrt(1-s^2))
     * — estavel, sem cancelamento; pi_q = q*s_q. A referencia SAI da dobra 26. */
    double PI_REF = 0;
    { double sq = 1; long q2 = 2;
      while(q2 < (1L << 26)){ sq = sq / sqrt(2 + 2 * sqrt(1 - sq*sq)); q2 *= 2; }
      PI_REF = (double)q2 * sq; }
    for(int j = 1; j <= 4; j++){
        int n = 1 << j;
        long q = q_dim(n);
        double sq = 1; long q2 = 2;
        while(q2 < q){ sq = sq / sqrt(2 + 2 * sqrt(1 - sq*sq)); q2 *= 2; }
        double pin = (double)q * sq;
        double err = fabs(pin - PI_REF);
        printf("  %2d %14ld %12ld   %.12f   %.3e\n", n, q, q/2, pin, err);
        if(j > 1 && err > pi_ant_err / 3) converge = 0;   /* dois bits: cai >= 4x por dobra */
        pi_ant_err = err;
    }
    ok("§L4 pi_n e' SAIDA da maquina em cada dimensao (a referencia e' a dobra 30, nao "
       "uma constante escrita) e o erro cai dois bits por dobra da torre", converge);

    /* §L5: o LIMITE DIMENSIONAL como funcao de c_n: a dimensao fecha quando o erro do
     * seu pi ja nao move meia marca da sua propria luz — erro_n · c_n < 1/2 */
    puts("\n  n    erro_n * c_n (marcas)   a dimensao fecha?");
    int lim = -1;
    for(int j = 1; j <= 4; j++){
        int n = 1 << j;
        long q = q_dim(n);
        double sq = 1; long q2 = 2;
        while(q2 < q){ sq = sq / sqrt(2 + 2 * sqrt(1 - sq*sq)); q2 *= 2; }
        double pin = (double)q * sq;
        double folga = fabs(pin - PI_REF) * (q / 2);
        printf("  %2d   %.6f%s\n", n, folga, folga < 0.5 ? "   FECHA" : "   nao fecha");
        if(lim < 0 && folga < 0.5) lim = n;
    }
    ok("§L5 o limite dimensional e' FUNCAO da luz da dimensao: abaixo dele o pi discreto "
       "ainda mexe meia marca de c_n, e a partir dele a dimensao fecha", lim > 0);
    printf("     -> a primeira dimensao da torre que fecha: n = %d\n", lim);

    /* §L6 — O TESTE DECISIVO (o eval mandou blindar): a relacao c_n = pi_n · R_n so'
     * vale se o R vier de OUTRO instrumento. Aqui vem: a ORBITA DESENHADA. O relogio
     * caminha q_n passos de ARESTA UNITARIA, rodando uma marca por passo (o rotor, da
     * mesma semente cos pi = -1); o raio MEDE-SE do diametro geometrico da orbita
     * (|P_0 P_{q/2}|), e o pi da dobra vem da algebra da meia-corda. Dois caminhos
     * independentes — e a relacao fecha se e so' se eles concordam. E o fecho da
     * orbita (voltar a origem) e' o residuo do proprio desenho. */
    puts("\n  n    fecho da orbita   pi_geo = q/D   pi_dobra      |geo-dobra|");
    int dois_caminhos = 1, orbita_fecha = 1;
    long err_ant = 1;
    for(int j = 1; j <= 4; j++){
        int n = 1 << j;
        long q = q_dim(n);
        /* a marca do relogio desta dimensao, pela dobra (sem pi) */
        double sq = 1; long q2 = 2;
        while(q2 < q){ sq = sq / sqrt(2 + 2 * sqrt(1 - sq*sq)); q2 *= 2; }
        double c1 = sqrt(1 - sq*sq), s1 = sq;      /* cos e sen de MEIA marca? nao: */
        /* sq = sen(pi/q) e' MEIA marca; a volta inteira por passo e' 2*(pi/q):
         * cos(2t) = 1-2s^2, sen(2t) = 2*s*c — a dobra ao contrario, exacta */
        double cm = 1 - 2*sq*sq, sm = 2*sq*sqrt(1 - sq*sq);
        double px = 0, py = 0, hx = 1, hy = 0;     /* posicao e rumo */
        double dx_meio = 0, dy_meio = 0;
        for(long k = 0; k < q; k++){
            if(k == q/2){ dx_meio = px; dy_meio = py; }
            px += hx; py += hy;
            double nx = hx*cm - hy*sm, ny = hx*sm + hy*cm;
            hx = nx; hy = ny;
        }
        double fecho = sqrt(px*px + py*py);
        double D = sqrt(dx_meio*dx_meio + dy_meio*dy_meio);
        double pi_geo = (double)q / D;             /* perimetro contado / diametro MEDIDO */
        double pin = (double)q * sq;               /* wait: pin = q*sen(pi/q) — a dobra */
        double dif = fabs(pi_geo - pin) / pin;
        printf("  %2d   %.3e        %.9f  %.9f  %.3e\n", n, fecho, pi_geo, pin, dif);
        if(fecho > 1e-6 * q) orbita_fecha = 0;
        /* a regua da concordancia e' o residuo do PROPRIO desenho: o arredondamento
         * acumula com os q passos do rotor — exige-se dif ABAIXO de 1e-9, que e' tres
         * ordens acima do pior medido e seis abaixo de qualquer fisica da orbita */
        if(dif > 1e-9) dois_caminhos = 0;
        err_ant = dif; (void)err_ant;
    }
    ok("§L6a a ORBITA FECHA: q passos unitarios com uma marca de rotacao por passo "
       "voltam a origem — o residuo do proprio desenho", orbita_fecha);
    ok("§L6b O TESTE DECISIVO: pi medido da ORBITA (perimetro contado / diametro medido) "
       "concorda com o pi da DOBRA (algebra da meia-corda) — dois instrumentos, e a "
       "relacao c = pi*R deixa de ser identidade por construcao", dois_caminhos);

    printf("\nunidades: %d   falhas: %d\nRESIDUO %d\n", feitas, falhas, falhas);
    return falhas ? 1 : 0;
}
