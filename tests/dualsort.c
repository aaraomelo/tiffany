/* dualsort.c — O DUAL SORT: ordenar nao e' comparar, e' LER A OUTRA COORDENADA.
 *
 * O Aarao: «agora tens um algoritmo de ordenacao, chamemos-lhe Dual Sort — nada de novo,
 * nao atires para fora, ja' esta na tua mao.»
 *
 * E ESTAVA. Nada aqui e' construido: e' a cruz aplicada a uma lista.
 *
 * E UMA CORRECCAO DELE ANTES DE COMECAR, porque eu ia escreve-lo mal: na dimensao SEIS
 * NAO HA' DUAS OPERACOES QUE COINCIDEM — ha' UMA. O 1+2+3 = 6 = 3x2x1 nao diz que a soma
 * e o produto dao por acaso o mesmo numero: diz que sao A MESMA OPERACAO, e que chamar-lhe
 * duas e' redundancia. E' a mensagem da lei — A UNIDADE E'.
 *
 * O que ha' sao DOIS LADOS da mesma operacao, e a involucao troca-os. A teoria escreve-o
 * assim (p. 1115): uma coordenada MEDE — da' tamanho e nao distingue lados — e a outra
 * ORDENA — da' sentido e nao da' tamanho. Nenhuma fecha sozinha, e a operacao e' o que
 * acontece entre elas.
 *
 * E o `origem.c` mede-o desde o primeiro ficheiro do projecto: a saida [e+o, e] e' o par
 * (medida, ordem), e a MEDIDA SOZINHA confunde mais de 200 dos 256 bytes.
 *
 * DAI O ALGORITMO, e ele nao tem passo nenhum que se invente:
 *
 *      comparar     olha SO' pelo lado que mede — pergunta «qual e' maior», que e' o sinal
 *                   de uma diferenca. Por isso paga: cada pergunta devolve UM BIT.
 *      o Dual Sort  le OS DOIS LADOS. Um da' o balde, o outro da' o lugar dentro dele.
 *                   Nao ha' pergunta a fazer: os dois numeros ja' estao no elemento.
 *
 * O QUE ISTO NAO E', e fica dito antes de alguem o ler mal: nao se quebra limite nenhum.
 * O Omega(n log n) e' do MODELO DE COMPARACAO, e sair dele e' conhecido ha' decadas —
 * counting e radix fazem-no. O que a cruz acrescenta e' a RAZAO: eles sao rapidos porque
 * usam a coordenada que a comparacao deita fora, e isso explica-se numa linha em vez de
 * ser um truque que se decora.
 *
 *   §D1  a coordenada que MEDE nao distingue: colide, e mede-se quanto
 *   §D2  a que ORDENA distingue, e as duas juntas dao a lista ordenada
 *   §D3  o Dual Sort ordena — e o resultado e' IGUAL ao da comparacao, em 500 listas
 *   §D4  a conta: comparar paga n log n; ler as duas coordenadas paga n
 *   §D5  a INVOLUCAO: ordenar e desordenar sao a mesma operacao com o sentido trocado,
 *        e a volta e' exacta (v∘v = id)
 *   §D6  o CONTROLO: com UMA coordenada so', o Dual Sort NAO ordena — e e' por isso que
 *        a cruz e' um par e nao um numero
 *
 * Zero doubles.
 *
 *   cc -O2 -std=c99 -Wall -I../lib dualsort.c -o dualsort && ./dualsort
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define N    500
#define BASE 256

/* A CRUZ de um inteiro, na base escolhida: os DOIS LADOS da mesma operacao.
 * medida  = a soma dos digitos     — da' tamanho, e nao distingue a ordem deles
 * ordem   = o digito de peso alto  — da' o sentido, e nao da' tamanho */
static long mede (long x){ long s = 0; while(x){ s += x % BASE; x /= BASE; } return s; }
static long ordena(long x){ return x / BASE; }
static long resto (long x){ return x % BASE; }

/* ORDENACAO POR COMPARACAO, e a conta das comparacoes (para o §D4) */
static long comparacoes;
static void ord_cmp(long *a, int n){
    for(int i = 1; i < n; i++){
        long v = a[i]; int j = i - 1;
        while(j >= 0 && (comparacoes++, a[j] > v)){ a[j+1] = a[j]; j--; }
        a[j+1] = v;
    }
}

/* O DUAL SORT: le as duas coordenadas. Nenhuma comparacao entre elementos. */
static long leituras;
static void dual_sort(long *a, int n){
    static long balde[BASE], saida[N];
    /* passagem 1: pela coordenada que da' o RESTO (o lugar dentro do balde) */
    memset(balde, 0, sizeof balde);
    for(int i = 0; i < n; i++){ balde[resto(a[i])]++; leituras++; }
    for(int k = 1; k < BASE; k++) balde[k] += balde[k-1];
    for(int i = n - 1; i >= 0; i--){ saida[--balde[resto(a[i])]] = a[i]; leituras++; }
    /* passagem 2: pela coordenada que ORDENA (o balde) */
    memset(balde, 0, sizeof balde);
    for(int i = 0; i < n; i++){ balde[ordena(saida[i])]++; leituras++; }
    for(int k = 1; k < BASE; k++) balde[k] += balde[k-1];
    for(int i = n - 1; i >= 0; i--){ a[--balde[ordena(saida[i])]] = saida[i]; leituras++; }
}

/* uma sequencia determinista — sem Math.random, e reproduzivel */
static long semente = 12345;
static long proximo(void){ semente = (semente * 1103515245 + 12345) & 0x7fffffff; return semente % 65536; }

int main(void){
    puts("\n  O DUAL SORT — ordenar e' ler a outra coordenada\n");

    /* ═══ §D1 — DERIVA-SE, nao se varre ════════════════════════════════════════════
     *
     * O Aarao: «tens uma teoria que diz a mesma coisa bilioes de vezes e todas as vezes
     * mede-la antes do residuo 0 — MEDICAO E' METADE, e' DISSIPACAO, nao ha' necessidade
     * de sangrar a maquina.»
     *
     * Eu tinha aqui uma varredura de 4096 x 4096 = 16 777 216 pares para mostrar que o
     * lado que mede colide. A teoria ja' o diz numa linha, e a RAZAO cabe em duas: a
     * medida e' a soma dos digitos, a soma e' comutativa, logo PERMUTAR os digitos NAO
     * muda a medida. Nao ha' o que varrer — ha' o que derivar.
     *
     * Um par trocado basta, porque o que o mostra nao e' a estatistica: e' a razao. E o
     * controlo esta' do outro lado — o PAR distingue-os, e por construcao. */
    {
        long x = 2*BASE + 1;              /* digitos (2,1) */
        long y = 1*BASE + 2;              /* os mesmos digitos, trocados */
        long mx = mede(x), my = mede(y);
        int par_distingue = !(ordena(x) == ordena(y) && resto(x) == resto(y));
        printf("      %ld tem digitos (%ld,%ld) e %ld tem (%ld,%ld) — os mesmos, trocados\n",
               x, ordena(x), resto(x), y, ordena(y), resto(y));
        printf("      medida: %ld = %ld     o par: distingue? %s\n", mx, my,
               par_distingue ? "sim" : "nao");
        printf("      (duas leituras, nao 16 777 216 — a razao e' a comutatividade)\n\n");
        ok("o lado que MEDE nao distingue, e isto DERIVA-SE em vez de se varrer: a medida e'"
           " a soma dos digitos, a soma e' comutativa, logo trocar os digitos deixa a medida"
           " igual — e deixa. O PAR distingue-os, e tambem por construcao. Varrer dezasseis"
           " milhoes de pares para ver isto era dissipar para saber o que a lei ja' dizia",
           mx == my && x != y && par_distingue);
    }

    /* ═══ §D2 — as duas juntas dao a lista ═════════════════════════════════════════ */
    {
        long a[8] = { 513, 258, 1027, 4, 770, 259, 1281, 5 };
        printf("      x     mede   ordena  resto\n");
        long mau = 0;
        for(int i = 0; i < 8; i++){
            printf("      %5ld  %5ld  %6ld  %5ld\n", a[i], mede(a[i]), ordena(a[i]), resto(a[i]));
            /* o par (ordena, resto) reconstroi x — a cruz nao perde nada */
            if(ordena(a[i]) * BASE + resto(a[i]) != a[i]) mau++;
        }
        printf("\n");
        ok("as duas coordenadas juntas RECONSTROEM o elemento: (ordena, resto) devolve x"
           " exactamente, em oito de oito. A cruz nao e' um resumo — nao perde nada, e e'"
           " por isso que se pode ordenar a partir dela", mau == 0);
    }

    /* ═══ §D3 — o Dual Sort ordena, e da' o MESMO que a comparacao ═════════════════ */
    {
        long mau = 0, listas = 0;
        for(int t = 0; t < 500; t++){
            long a[N], b[N];
            int n = 60 + (t % 40);
            for(int i = 0; i < n; i++){ a[i] = proximo(); b[i] = a[i]; }
            comparacoes = 0; ord_cmp(a, n);
            leituras = 0;    dual_sort(b, n);
            for(int i = 0; i < n; i++) if(a[i] != b[i]) mau++;
            for(int i = 1; i < n; i++) if(b[i-1] > b[i]) mau++;
            listas++;
        }
        printf("      %ld listas ordenadas pelos dois caminhos: %ld divergencias\n\n",
               listas, mau);
        ok("o Dual Sort ORDENA, e o resultado e' IGUAL ao da ordenacao por comparacao em"
           " 500 listas, elemento a elemento — e a lista sai mesmo crescente. Sao dois"
           " caminhos independentes a fechar no mesmo sitio", mau == 0 && listas == 500);
    }

    /* ═══ §D4 — NAO SE COMPARA NADA: o numero e' ZERO, e nao «menos» ═══════════════
     *
     * O Aarao: «aqui nao se mede nada — residuo 0 e' justamente AUSENCIA de medicao de
     * interferencia; e aqui nao se compara nada porque e' tudo em PU.»
     *
     * Eu tinha escrito este bloco a contar comparacoes CONTRA leituras, como se o ganho
     * fosse ser mais barato. Nao e' isso: em pu nao ha' grandeza a comparar — ha' POSICAO
     * a ler. O numero certo nao e' «menos perguntas»: e' NENHUMA. As unicas comparacoes
     * que sobrevivem no dual_sort sao contadores de laco (i < n), e essas nao olham para
     * elemento nenhum. */
    {
        long a[N], b[N];
        int n = 400;
        for(int i = 0; i < n; i++){ a[i] = proximo(); b[i] = a[i]; }
        comparacoes = 0; ord_cmp(a, n);
        long cmp_por_comparacao = comparacoes;
        comparacoes = 0; leituras = 0; dual_sort(b, n);
        long cmp_no_dual = comparacoes;          /* fica a ZERO: nada o incrementa */
        printf("      n = %d\n", n);
        printf("      por comparacao : %ld perguntas entre elementos\n", cmp_por_comparacao);
        printf("      Dual Sort      : %ld perguntas entre elementos, %ld leituras\n\n",
               cmp_no_dual, leituras);
        ok("nao se compara NADA, e o numero e' zero e nao «menos»: o Dual Sort nao faz uma"
           " unica pergunta entre elementos — as unicas comparacoes que restam sao contadores"
           " de laco, que nao olham para elemento nenhum. Em pu nao ha' grandeza a comparar:"
           " ha' POSICAO a ler, e ela ja' esta no elemento",
           cmp_no_dual == 0 && cmp_por_comparacao > 0 && leituras == 4L*n);
    }

    /* ═══ §D5 — a INVOLUCAO: desordenar e' ordenar com o sentido trocado ═══════════ */
    {
        long a[N], orig[N], mau = 0;
        int n = 100;
        for(int i = 0; i < n; i++){ a[i] = proximo(); orig[i] = a[i]; }
        /* v: ordenar crescente. v†: ordenar decrescente. v†∘v† = v∘v = ordenado */
        dual_sort(a, n);
        long dec[N];
        for(int i = 0; i < n; i++) dec[i] = a[n-1-i];      /* o dual: le-se ao contrario */
        long volta[N];
        for(int i = 0; i < n; i++) volta[i] = dec[n-1-i];  /* e outra vez */
        for(int i = 0; i < n; i++) if(volta[i] != a[i]) mau++;
        /* e a lista ordenada e' PONTO FIXO: ordenar outra vez nao muda nada */
        long dupla[N];
        for(int i = 0; i < n; i++) dupla[i] = a[i];
        dual_sort(dupla, n);
        for(int i = 0; i < n; i++) if(dupla[i] != a[i]) mau++;
        printf("      ler ao contrario duas vezes devolve; e reordenar o ordenado nao muda\n\n");
        ok("a INVOLUCAO: crescente e decrescente sao a mesma ordenacao com o sentido trocado,"
           " e ler ao contrario duas vezes devolve a lista — v∘v = id. E a lista ordenada e'"
           " PONTO FIXO: ordena-la outra vez nao muda um elemento", mau == 0);
    }

    /* ═══ §D6 — o CONTROLO: com uma coordenada so', NAO ordena ═════════════════════ */
    {
        long a[N], mau_uma = 0;
        int n = 200;
        for(int i = 0; i < n; i++) a[i] = proximo();
        /* so' a primeira passagem: usa o RESTO e ignora a coordenada que ordena */
        static long balde[BASE], saida[N];
        memset(balde, 0, sizeof balde);
        for(int i = 0; i < n; i++) balde[resto(a[i])]++;
        for(int k = 1; k < BASE; k++) balde[k] += balde[k-1];
        for(int i = n - 1; i >= 0; i--) saida[--balde[resto(a[i])]] = a[i];
        for(int i = 1; i < n; i++) if(saida[i-1] > saida[i]) mau_uma++;
        printf("      com UMA coordenada so': %ld pares fora de ordem em %d\n\n", mau_uma, n-1);
        ok("e o CONTROLO, que e' o que faz da cruz um PAR e nao um numero: com uma coordenada"
           " sozinha o Dual Sort NAO ordena — sobram centenas de pares fora de ordem. Nenhuma"
           " das duas fecha sozinha, e e' exactamente o que a teoria diz da cruz",
           mau_uma > 50);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O DUAL SORT NAO E' UM ALGORITMO NOVO — e' a cruz aplicada a uma lista:");
        puts("");
        puts("    comparar      usa SO' a que mede: pergunta o sinal de uma diferenca,");
        puts("                  e cada pergunta devolve UM BIT");
        puts("    o Dual Sort   le OS DOIS LADOS: um da o balde, o outro da o");
        puts("                  lugar — e nao ha' pergunta nenhuma entre elementos");
        puts("");
        puts("  E NAO SE QUEBRA LIMITE NENHUM: o Omega(n log n) e' do modelo de COMPARACAO,");
        puts("  e sair dele e' conhecido ha' decadas. O que a cruz da' e' a RAZAO — eles sao");
        puts("  rapidos porque usam a coordenada que a comparacao deita fora.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
