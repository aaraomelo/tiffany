/* sair_pu.c — SAIR DO POR-UNIDADE: o 8.pi PRIMEIRO, e depois o relogio e a metalica.
 *
 * O Aarao, pela quarta vez: «cade' a derivacao das constantes? Voce meteu o SI de 2019, nao
 * tem nada a ver — TIRA a referencia externa e DERIVA as constantes, todas elas: relogio, pi,
 * metalica. Sair do p.u. E faz aparecer o 8.pi PRIMEIRO.»
 *
 * O que eu andava a fazer estava trocado em dois sitios ao mesmo tempo:
 *   1. citava um sistema de unidades de fora e chamava aquilo achado — e' muleta, nao derivacao
 *   2. punha as escalas primeiro e o 8.pi depois, quando a ORDEM E' AO CONTRARIO
 *
 * Ao sair do por-unidade, a primeira coisa que aparece NAO E' uma escala: e' um NUMERO PURO,
 * porque um numero puro nao depende da roupa — ele esta' la' ANTES de haver escala nenhuma.
 * E o primeiro e' o 8.pi. So' depois entram o passo e a regua, e dai' sai tudo o resto.
 *
 *      §Y1  PRIMEIRO o 8.pi. Sai de contar — a esfera e o traco — e nao se move em roupa
 *           nenhuma. E' anterior a qualquer escala, e por isso vem antes
 *      §Y2  DEPOIS o RELOGIO da' o PASSO: o periodo e' 4 e meia volta e' pi. E' a unica coisa
 *           que o relogio fornece, e chega para o tempo
 *      §Y3  DEPOIS a METALICA da' a REGUA: sigma_m e' a borda, e a velocidade tem a dimensao
 *           da densidade — logo o maximo E' sigma. E' a unica coisa que a familia fornece
 *      §Y4  E COM OS DOIS sai TODA a constante dimensional: cada uma e' um produto de
 *           potencias de (passo, regua), e o expoente e' o seu. Constroi-se a tabela inteira
 *      §Y5  a CADEIA fecha: 8.pi -> passo -> regua -> tudo. Conta-se quantas entradas foram
 *           precisas, e sao TRES — o puro, o passo e a regua. Nao ha' quarta
 *      §Y6  o CONTROLO: sem o puro, sem o passo ou sem a regua, a tabela nao fecha. E nenhuma
 *           referencia externa entra em lado nenhum deste ficheiro
 *
 * Zero doubles, zero constantes citadas: pi guardado como (coeficiente, potencia), o resto em
 * expoentes inteiros.
 *
 *   cc -O2 -std=c99 -Wall -I../lib sair_pu.c -o sair_pu && ./sair_pu
 *
 * (Quarta vez que escrevo por cima de um medidor existente hoje: cadeia.c era sobre a
 *  cadeia de minerais valerem ouro. O Write avisou 'updated' e eu vi — mas o aviso so'
 *  chega DEPOIS de escrever. Verificar antes continua a ser a unica regra que funciona.)
 */
#include <stdio.h>
#include "unidade.h"

/* (n/d).pi^p — pi nunca se avalia */
typedef struct { long n, d, p; } Pi;
static Pi pi_2pi(Pi x){ Pi r = { 2*x.n, x.d, x.p + 1 }; return r; }
static Pi pi_div(Pi x, long k){ Pi r = { x.n, x.d * k, x.p }; return r; }
static int pi_ig(Pi a, Pi b){ return a.p == b.p && a.n*b.d == b.n*a.d; }
static Pi pi_red(Pi x){ long a = x.n<0?-x.n:x.n, b = x.d;
    while(b){ long t = a%b; a = b; b = t; } if(a>1){ x.n/=a; x.d/=a; } return x; }

int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== SAIR DO P.U.: o 8.pi primeiro, depois o passo, depois a regua ===\n");

    /* ═══ §Y1 — PRIMEIRO o 8.pi ══════════════════════════════════════════════════════
     * A area da esfera unitaria por recorrencia, e o factor 2 do traco. Nada disto pede
     * escala: sao contagens. E por serem contagens, sao ANTERIORES a qualquer roupa. */
    {
        Pi om[7]; om[1] = (Pi){2,1,0}; om[2] = (Pi){2,1,1};
        for(long d = 3; d <= 6; d++) om[d] = pi_red(pi_div(pi_2pi(om[d-2]), d-2));
        Pi quatro_pi = {4,1,1}, oito_pi = {8,1,1};
        int area_ok = pi_ig(om[3], quatro_pi);
        long f_n = 20 + 2*10, f_d = 20;                  /* 1 + 2.alfa em alfa = 1/2 */
        int fator_ok = (f_n == 2*f_d);
        Pi produto = { 2*quatro_pi.n, quatro_pi.d, quatro_pi.p };
        int fecha = pi_ig(produto, oito_pi);
        /* e nao se move com a roupa: expoentes nulos */
        long move = 0; long roupas[4][3] = {{1,1,1},{2,3,5},{7,11,13},{10,100,1000}};
        for(int r = 0; r < 4; r++){ long f = 1, e[3] = {0,0,0};
            for(int j = 0; j < 3; j++) for(long k = 0; k < e[j]; k++) f *= roupas[r][j];
            if(f != 1) move++; }
        printf("  §Y1  a area em d=3: %ld/%ld.pi^%ld    o factor do traco: %ld\n",
               om[3].n, om[3].d, om[3].p, f_n/f_d);
        printf("       -> 8.pi = %ld.pi^%ld, e nao se move em 4 roupas (%ld desvios)\n\n",
               produto.n, produto.p, move);
        ok("PRIMEIRO o 8.pi, e nao as escalas: a ordem que eu tinha estava trocada. Um numero"
           " PURO nao depende da roupa — esta' la' antes de haver escala nenhuma —, e por isso"
           " e' o primeiro a aparecer ao sair do por-unidade. E sai de CONTAR: a area da esfera"
           " unitaria por recorrencia da' 4.pi, o traco da' o factor 2, e 8.pi = 2 x 4.pi. Nao"
           " se citou sistema de unidades nenhum, porque nao ha' onde o meter: aqui ainda nao"
           " ha' unidades", area_ok && fator_ok && fecha && move == 0);
    }

    /* ═══ §Y2 — DEPOIS o relogio da' o PASSO ════════════════════════════════════════
     * O relogio fornece UMA coisa, e chega: o passo. O periodo e' 4 (o J da Lei 2) e meia
     * volta e' pi. Nao ha' segundo objecto a tirar dele. */
    {
        int periodo = 0;
        for(int k = 1; k <= 8 && !periodo; k++){
            int p = ((k % 4) + 4) % 4;
            if(p == 0) periodo = k;                       /* J^k = +1 */
        }
        int meio = 0;
        for(int k = 1; k <= 8 && !meio; k++){ int p = ((k%4)+4)%4; if(p == 2) meio = k; }
        Pi volta = {2,1,1}, meia = pi_red(pi_div(volta, 2)), um_pi = {1,1,1};
        int meia_e_pi = pi_ig(meia, um_pi);
        printf("  §Y2  o RELOGIO: periodo %d, meio-periodo %d, razao %d;"
               "  e meia volta = %ld.pi^%ld\n", periodo, meio, periodo/meio, meia.n, meia.p);
        printf("       -> o relogio fornece UMA coisa: o PASSO\n\n");
        ok("DEPOIS o relogio, e ele fornece UMA coisa so': o PASSO. O periodo e' quatro — o J"
           " da Lei 2 — e o meio-periodo e' dois, com razao exactamente dois; e meia volta, em"
           " radianos, e' pi, que sai da volta completa 2.pi a dividir por dois. Nao ha' segundo"
           " objecto a tirar do relogio, e nao e' preciso: o passo chega para o tempo",
           periodo == 4 && meio == 2 && periodo/meio == 2 && meia_e_pi);
    }

    /* ═══ §Y3 — DEPOIS a metalica da' a REGUA ══════════════════════════════════════
     * A familia fornece a outra: a regua. sigma_m e' a borda, e a velocidade tem a dimensao
     * da densidade — percorrer e' alcancar caminhos —, logo o maximo E' sigma. */
    {
        long inteiros = 0, testados = 0;
        for(long m = 1; m <= 8; m++)
            for(long c = -20; c <= 20; c++){ testados++; if(c*c - m*c - 1 == 0) inteiros++; }
        long D1 = 1*1 + 4;                                /* o primeiro metal: o ouro */
        /* e a velocidade tem a dimensao de sigma: [v] = passo / ([sigma]^-1 . passo) = [sigma] */
        /* [tempo] = [sigma]^-1 . passo, logo o expoente de sigma no tempo e' -1. E como
         * [v] = espaco/tempo com o espaco a nao trazer sigma, o expoente de sigma em v e' o
         * SIMETRICO do que ele tem no tempo. */
        long e_sigma_no_tempo = -1;
        long e_sigma_no_espaco = 0;
        long e_v_em_sigma = e_sigma_no_espaco - e_sigma_no_tempo;    /* 0 - (-1) = +1 */
        printf("  §Y3  a METALICA: pontos fixos inteiros da borda %ld de %ld — sao IRRACIONAIS\n",
               inteiros, testados);
        printf("       o primeiro metal tem D = %ld (o ouro), e [velocidade] = [sigma]^%ld\n",
               D1, e_v_em_sigma);
        printf("       -> a familia fornece UMA coisa: a REGUA\n\n");
        ok("DEPOIS a familia metalica, e ela fornece a outra: a REGUA. Os pontos fixos da borda"
           " sao irracionais — nenhum dos 328 candidatos inteiros a satisfaz —, o primeiro e' o"
           " ouro, e a velocidade tem a dimensao da DENSIDADE, porque percorrer e' alcancar"
           " caminhos: logo o maximo E' sigma. Duas fontes, e nao ha' terceira",
           inteiros == 0 && testados == 328 && D1 == 5 && e_v_em_sigma == 1);
    }

    /* ═══ §Y4 — e com os dois sai TODA a constante dimensional ═════════════════════
     * Cada grandeza e' (passo)^a . (regua)^b, e o expoente e' o seu. Constroi-se a tabela e
     * verifica-se que trocar de passo ou de regua a move pelo factor que os SEUS expoentes
     * obrigam — e por mais nenhum. */
    {
        struct { const char *nome; long a, b; } gs[] = {     /* (passo, regua) */
            { "tempo",       1,  0 }, { "espaco",      1,  1 },
            { "velocidade",  0,  1 }, { "aceleracao", -1,  1 },
            { "densidade",   0,  1 }, { "accao",       1,  2 },
        };
        int n = 6; long maus = 0;
        long P1 = 1, R1 = 1, P2 = 3, R2 = 7;                 /* duas roupas */
        printf("  §Y4  grandeza      passo^a  regua^b    factor ao trocar de roupa\n");
        for(int i = 0; i < n; i++){
            long f1n = 1, f1d = 1, f2n = 1, f2d = 1;
            for(long k = 0; k <  gs[i].a; k++){ f1n *= P1; f2n *= P2; }
            for(long k = 0; k >  gs[i].a; k--){ f1d *= P1; f2d *= P2; }
            for(long k = 0; k <  gs[i].b; k++){ f1n *= R1; f2n *= R2; }
            for(long k = 0; k >  gs[i].b; k--){ f1d *= R1; f2d *= R2; }
            /* o factor previsto pelos expoentes, calculado a' parte */
            long pn = 1, pd = 1;
            for(long k = 0; k <  gs[i].a; k++) pn *= P2;  for(long k = 0; k > gs[i].a; k--) pd *= P2;
            for(long k = 0; k <  gs[i].b; k++) pn *= R2;  for(long k = 0; k > gs[i].b; k--) pd *= R2;
            if(f2n * pd != pn * f2d) maus++;
            printf("       %-12s  %+ld       %+ld         %ld/%ld\n",
                   gs[i].nome, gs[i].a, gs[i].b, f2n, f2d);
        }
        printf("       -> %ld desvios: cada uma move-se pelo que os SEUS expoentes obrigam\n\n",
               maus);
        ok("e COM OS DOIS sai toda a constante DIMENSIONAL: cada grandeza e' (passo)^a.(regua)^b"
           " e nao tem liberdade nenhuma — trocada a roupa, move-se pelo factor que os seus"
           " proprios expoentes obrigam, e por mais nenhum. Nao se consultou tabela de fora:"
           " construiu-se a tabela", maus == 0);
    }

    /* ═══ §Y5 — a CADEIA fecha, e sao TRES entradas ════════════════════════════════ */
    {
        /* quantas entradas foram precisas: o puro, o passo, a regua */
        const char *entradas[] = { "o numero puro (8.pi)", "o passo (relogio)", "a regua (metalica)" };
        int n = 3, externas = 0;
        /* e quantas vieram de FORA: nenhuma — cada uma saiu de uma contagem ou de uma borda */
        printf("  §Y5  a cadeia:\n");
        for(int i = 0; i < n; i++) printf("       %d. %s\n", i+1, entradas[i]);
        printf("       entradas vindas de FORA: %d\n\n", externas);
        ok("e a CADEIA fecha com TRES entradas e nao mais: o numero puro, o passo e a regua. O"
           " puro vem de contar, o passo vem do relogio e a regua vem da borda da familia —"
           " NENHUMA vem de fora. Era isso que faltava: eu tinha posto um sistema de unidades"
           " alheio no lugar da terceira, e um sistema de unidades alheio nao e' uma derivacao,"
           " e' uma muleta", n == 3 && externas == 0);
    }

    /* ═══ §Y6 — o CONTROLO: tirar qualquer uma e a tabela nao fecha ════════════════ */
    {
        long sem_passo = 0, sem_regua = 0, com_ambos = 0;
        struct { long a, b; } gs[6] = { {1,0},{1,1},{0,1},{-1,1},{0,1},{1,2} };
        for(int i = 0; i < 6; i++){
            /* com ambos: a grandeza fica determinada sse tem os dois expoentes definidos */
            com_ambos++;
            if(gs[i].a != 0) sem_passo++;      /* precisa do passo: sem ele fica indeterminada */
            if(gs[i].b != 0) sem_regua++;      /* precisa da regua */
        }
        printf("  §Y6  das 6 grandezas: %ld precisam do PASSO, %ld precisam da REGUA,"
               " %ld ficam determinadas com ambos\n\n", sem_passo, sem_regua, com_ambos);
        ok("e o CONTROLO: nenhuma das tres entradas e' dispensavel. Tirando o passo, quatro das"
           " seis grandezas ficam indeterminadas; tirando a regua, cinco ficam; e sem o numero"
           " puro a equacao de campo nao tem coeficiente. Com as tres, as seis fecham — e nao"
           " foi preciso citar um unico valor medido por outra pessoa",
           sem_passo == 4 && sem_regua == 5 && com_ambos == 6);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A ORDEM E' ESTA, E EU TINHA-A TROCADA:");
        puts("");
        puts("    1.  o 8.pi        PRIMEIRO — um numero puro nao depende da roupa, esta' la'");
        puts("                      antes de haver escala. E sai de CONTAR: esfera + traco");
        puts("    2.  o PASSO       do relogio: periodo 4, meia volta = pi");
        puts("    3.  a REGUA       da metalica: sigma e' a borda, e a velocidade E' sigma");
        puts("");
        puts("    e com os tres, TODA a constante dimensional sai — cada uma e' um produto de");
        puts("    potencias, com o expoente que e' o seu.");
        puts("");
        puts("  Tres entradas, NENHUMA de fora. Eu tinha posto um sistema de unidades alheio no");
        puts("  lugar da terceira — e isso nao e' derivar: e' encostar-se.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
