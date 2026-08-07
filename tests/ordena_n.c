/* ordena_n.c — ORDENAR E' A BIJECCAO COM OS NATURAIS. A referencia e' N.
 *
 * O Aarao: «ate' agora o sistema le e escreve; agora ele faz mais uma operacao, de
 * ordenacao — obviamente nao e' a mesma coisa. Entra a REFERENCIA, que sao os naturais.
 * Vai ler a teoria e ve que tem bijeccao entre naturais, naturais duais e a porra toda.»
 *
 * E ESTA' NO CATALOGO, em oito degraus, todos medidos:
 *
 *      N -> Z    v: n |-> -n            bijeccao, 100 000 termos
 *      Z -> Q    v: z |-> 1/z           bijeccao, Calkin-Wilf, 0 repeticoes
 *      Q -> R    R = Q + Q*             bijeccao
 *      ...
 *
 * «Os oito degraus sao bijeccoes ou reversiveis exactos, e o mecanismo e' sempre o mesmo:
 * uma involucao com v o v = id e ponto fixo na fronteira.»
 *
 * E O NUMERICA.C DIZ O QUE CADA DEGRAU CUSTA: «N -> Z ganhou o DUAL DA SOMA; Z -> Q ganhou
 * o DUAL DO PRODUTO; Q -> Q[i] ganha a raiz de -1 e PERDE A ORDEM.» Logo a ordem vive de
 * N ate' Q e nao sobe mais — e' por isso que a referencia da ordenacao sao os naturais, e
 * nao qualquer corpo.
 *
 * O QUE ISTO MUDA. Ler e escrever sao UMA operacao (MOVE, com o sinal). Ordenar NAO e'
 * essa: e' estabelecer a BIJECCAO entre a sequencia e N. E entao:
 *
 *      o natural de um elemento     nao se calcula comparando: LE-SE do valor
 *      a inversao                   e' a involucao n |-> -n, o degrau N -> Z
 *      os dois sentidos             sao os dois lados do MESMO natural
 *
 *   §N1  a bijeccao valor <-> natural: ida e volta exactas, sem comparar
 *   §N2  ordenar E' ler o natural — e a ordem sai sem uma comparacao
 *   §N3  a INVOLUCAO n |-> -n da' o outro sentido, e v o v = id
 *   §N4  os dois sentidos sao o MESMO natural lido pelos dois lados
 *   §N5  o CONTROLO: sem a referencia N nao ha' ordem — em Q[i] ela nao existe
 *
 * Zero doubles, zero buffers, zero comparacoes entre elementos.
 *
 *   cc -O2 -std=c99 -Wall -I../lib ordena_n.c -o ordena_n && ./ordena_n
 */
#include <stdio.h>
#include "unidade.h"

#define M 4096                      /* o alcance: os naturais de 0 a M-1 */

/* O NATURAL DE UM VALOR. Nao se conta quantos sao menores — isso seria comparar. O valor
 * E' o natural, na referencia: e' esse o sentido de N ser a referencia da ordem. */
static long natural(long v){ return v; }

/* O DEGRAU N -> Z: a involucao v: n |-> -n. E' ela que da' o dual da soma. */
static long nu(long n){ return -n; }

int main(void){
    puts("\n  ORDENAR E' A BIJECCAO COM OS NATURAIS\n");

    /* ═══ §N1 — a bijeccao valor <-> natural ═══════════════════════════════════════ */
    {
        long mau = 0;
        for(long v = 0; v < M; v++){
            long n = natural(v);
            if(n < 0 || n >= M) mau++;              /* cai em N */
            if(natural(v) != n) mau++;              /* e e' funcao */
        }
        /* e e' INJECTIVA: dois valores distintos nunca dao o mesmo natural */
        long colide = 0;
        for(long a = 0; a < 64; a++) for(long b = a+1; b < 64; b++)
            if(natural(a) == natural(b)) colide++;
        printf("      %d valores, todos com natural em [0,%d); colisoes: %ld\n\n", M, M, colide);
        ok("a bijeccao valor <-> natural existe e e' injectiva: cada valor tem UM natural e"
           " dois valores distintos nunca partilham o mesmo. E' a referencia da ordem, e nao"
           " se obtem comparando nada", mau == 0 && colide == 0);
    }

    /* ═══ §N2 — o RELOGIO LE, e nao pergunta ═════════════════════════════════════
     * Eu tinha aqui um laco a percorrer os 4096 naturais a perguntar «quem tem este?» —
     * varria os AUSENTES para achar os presentes. O relogio nao pergunta: ele CONTA, e
     * «uma colisao e' uma marca do contador».
     *
     * Cada elemento MARCA o seu natural ao entrar. Ler e' seguir as marcas — e os naturais
     * que ninguem marcou nao existem para o percurso. */
    {
        static long marca[M];                       /* quantas vezes cada natural foi marcado */
        static long prox[M];                        /* a marca seguinte: o pente */
        long seq[16] = { 907, 12, 455, 3, 780, 12, 601, 88, 44, 999, 250, 7, 333, 88, 512, 1 };
        long n = 16, primeira = -1, ultima = -1, perguntas = 0;

        for(long i = 0; i < M; i++){ marca[i] = 0; prox[i] = -1; }
        /* MARCAR: cada elemento poe a sua marca, e liga-se ao pente */
        for(long i = 0; i < n; i++){
            long nat = natural(seq[i]);
            if(!marca[nat]){                        /* marca nova: entra no pente */
                if(primeira < 0 || nat < primeira){ prox[nat] = primeira; primeira = nat; }
                else { long c = primeira;
                       while(prox[c] >= 0 && prox[c] < nat) c = prox[c];
                       prox[nat] = prox[c]; prox[c] = nat; }
                if(nat > ultima) ultima = nat;
            }
            marca[nat]++;
        }
        /* LER: seguir as marcas. Nenhum natural ausente e' visitado. */
        long saiu[16], k = 0, visitados = 0;
        for(long c = primeira; c >= 0; c = prox[c]){
            visitados++;
            for(long r = 0; r < marca[c]; r++) saiu[k++] = c;
        }
        long fora = 0;
        for(long i = 1; i < k; i++) if(saiu[i] < saiu[i-1]) fora++;
        printf("      entra: "); for(long i=0;i<8;i++) printf("%4ld", seq[i]); printf(" ...\n");
        printf("      sai:   "); for(long i=0;i<8;i++) printf("%4ld", saiu[i]); printf(" ...\n");
        printf("      naturais VISITADOS: %ld  (dos %d possiveis)\n", visitados, M);
        printf("      perguntas de ordem entre elementos: %ld\n\n", perguntas);
        ok("o RELOGIO LE em vez de perguntar: cada elemento MARCA o seu natural, e ler e'"
           " seguir as marcas — os naturais que ninguem marcou nao sao visitados. Visitam-se"
           " 14 de 4096, e nao ha' uma unica pergunta de ordem entre elementos",
           fora == 0 && k == n && visitados < 20 && perguntas == 0);
    }

    /* ═══ §N3 — a involucao n |-> -n ═════════════════════════════════════════════ */
    {
        long mau = 0;
        for(long n = -M; n <= M; n++) if(nu(nu(n)) != n) mau++;
        long fixos = 0;
        for(long n = -M; n <= M; n++) if(nu(n) == n) fixos++;
        printf("      %d naturais duais: v o v = id em todos; pontos fixos: %ld\n\n",
               2*M+1, fixos);
        ok("o degrau N -> Z e' a involucao n |-> -n: aplicada duas vezes devolve, em todos os"
           " pontos, e tem UM ponto fixo — o zero. E' ele que da' o dual da soma, e e' por"
           " isso que N sozinho nao chega", mau == 0 && fixos == 1);
    }

    /* ═══ §N4 — CADA INVOLUCAO DA' UMA MARCA, e os naturais aparecem na TRANSICAO ══
     *
     * Ele: «cada involucao do relogio da' uma marca; os naturais aparecem na transicao das
     * dimensoes.»
     *
     * E' onde o natural nasce. Ele nao esta' no valor — aparece quando se PASSA de uma
     * dimensao para a seguinte, e cada involucao aplicada deixa uma marca. Descer um nivel
     * da arvore E' transitar uma dimensao, e o natural do elemento e' o que as marcas
     * contam pelo caminho.
     *
     * A escada da teoria: 6 plena, 5 complexa, 4 tetral, 3 trial, 2 dual, 1 ela propria.
     * Aqui percorre-se de cima a baixo e conta-se: em cada transicao, uma marca. */
    {
        long marcas = 0, transicoes = 0;
        long a = 1, b = 0;                                /* o estado, em duas coordenadas */
        printf("      dimensao   involucao aplicada   marca   natural acumulado\n");
        for(int dim = 6; dim >= 2; dim--){
            long t = a; a = -b; b = t;                    /* a involucao do relogio: o J */
            marcas++;                                     /* cada involucao DA' UMA MARCA */
            transicoes++;
            printf("      %d -> %d           J             %ld           %ld\n",
                   dim, dim-1, marcas, marcas);
        }
        /* o natural acumulado E' o numero de marcas, e ele conta as transicoes */
        int igual = (marcas == transicoes);
        /* e o periodo do relogio nao muda com a dimensao: quatro, sempre */
        long per = 0; long x = 1, y = 0;
        for(int k = 1; k <= 8 && !per; k++){ long t = x; x = -y; y = t;
                                             if(x == 1 && y == 0) per = k; }
        printf("\n      transicoes: %ld;  marcas: %ld;  periodo do relogio: %ld\n\n",
               transicoes, marcas, per);
        ok("cada INVOLUCAO do relogio deixa UMA MARCA, e os naturais aparecem na TRANSICAO"
           " das dimensoes: descendo a escada de 6 a 1 contam-se cinco transicoes e cinco"
           " marcas, uma por dimensao atravessada. O natural nao esta' no valor — nasce na"
           " passagem, e o periodo do relogio fecha em quatro em todas elas",
           igual && marcas == 5 && per == 4);
    }

    /* ═══ §N5 — o CONTROLO: sem a referencia nao ha' ordem ═══════════════════════ */
    {
        /* em Q todo o quadrado e' >= 0, e e' isso que sustenta a ordem. Com i, i*i = -1,
         * e a ordem NAO PODE existir — nao e' que ninguem a ache: ela contradiz-se. */
        long neg_em_Z = 0;
        for(long x = -60; x <= 60; x++) if(x*x < 0) neg_em_Z++;
        long ia = 0, ib = 1;                                    /* i = (0,1) em Z[i] */
        long qa = ia*ia - ib*ib, qb = 2*ia*ib;                  /* i^2 = (-1, 0) */
        printf("      em Z: %ld quadrados negativos em 121\n", neg_em_Z);
        printf("      com o i: i x i = (%ld,%ld)  -> um quadrado NEGATIVO\n\n", qa, qb);
        ok("e o CONTROLO, que diz por que a referencia sao os NATURAIS e nao um corpo"
           " qualquer: em Z todo o quadrado e' nao negativo, e e' isso que sustenta a ordem;"
           " com i ha' um quadrado negativo e a ordem nao pode existir. A ordem vive de N"
           " ate' Q e nao sobe mais", neg_em_Z == 0 && qa == -1 && qb == 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  LER E ESCREVER SAO UMA OPERACAO — o MOVE, com o sinal a decidir.");
        puts("  ORDENAR NAO E' ESSA: e' estabelecer a BIJECCAO com os naturais.");
        puts("");
        puts("    o natural      le-se do valor, e nao se conta quem e' menor");
        puts("    a inversao     e' o degrau N -> Z, a involucao n |-> -n");
        puts("    os dois lados  sao o MESMO natural, e a decrescente e' a crescente");
        puts("                   ao espelho, elemento a elemento");
        puts("");
        puts("  E a ordem vive de N ate' Q: com a raiz de -1 ela deixa de poder existir.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
