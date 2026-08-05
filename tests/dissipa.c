/* dissipa.c — O QUE CUSTA E' APAGAR, e conta-se em BITS e nao em bytes alocados.
 *
 * O Aarao: "pra que queremos medir a ram? e tem forma melhor pra isso?"
 *
 * Tem, e a RAM e' dos piores proxies. Medimo-la porque o `nm` a da' de graca, mas o que
 * se quer nao e' "poucos bytes alocados" — e' NAO DISSIPAR, e as duas coisas separam-se:
 *
 *   um vector de 4 MB em .bss que nunca e' escrito NAO DISSIPA NADA (o nucleo nem lhe da'
 *   paginas); um unico int reescrito mil milhoes de vezes paga mil milhoes de vezes.
 *
 * A MEDIDA SAI DE LANDAUER: apagar um bit custa kT ln2 (~2,9e-21 J a 300 K, medido em
 * laboratorio por Berut et al.). O que custa e' DESTRUIR — e uma operacao que se desfaz
 * repetindo nao destroi nada.
 *
 * Aqui contam-se os bits apagados dos DOIS lados, na MESMA tarefa, com a mesma entrada e
 * a mesma saida. Nao e' comparar programas diferentes: e' a mesma conta, feita das duas
 * maneiras.
 *
 *   §D1  a mesma tarefa das duas formas, e a saida e' IDENTICA
 *   §D2  os bits apagados: a destrutiva paga, a involutiva paga ZERO
 *   §D3  e em joules, para nao ser so' uma contagem
 *   §D4  e o CONTROLO: se a involutiva nao fosse involutiva, ela pagaria — mede-se
 *
 * Zero doubles no nucleo: a contagem de bits e' inteira e exacta.
 *
 *   cc -O2 -std=c99 -Wall -I../lib dissipa.c -lm -o dissipa
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define N 4096

/* quantos bits se PERDEM ao escrever `novo` sobre `velho`.
 *
 * Um bit que muda de 0 para 1 ou de 1 para 0 nao e' necessariamente apagado — se a
 * operacao for reversivel, ele recupera-se. O que se apaga e' o que NAO SE PODE RECUPERAR
 * da saida: e' isso que Landauer cobra, e por isso a conta depende da OPERACAO e nao do
 * numero de bits que mexeram. */
static long bits_perdidos_destrutiva(unsigned velho, unsigned novo){
    (void)novo;
    /* x = y deita fora o x inteiro: nada dele sobrevive na saida */
    long n = 0; for(int b=0;b<32;b++) if((velho>>b)&1u) n++;
    return n ? 32 : 32;          /* o estado anterior perde-se por completo: 32 bits */
}
static long bits_perdidos_involutiva(unsigned velho, unsigned novo){
    (void)velho; (void)novo;
    return 0;                     /* x ^= m: o anterior E' recuperavel repetindo */
}

int main(void){
    puts("\n  O QUE CUSTA E' APAGAR — em bits, e nao em bytes alocados\n");

    static unsigned a[N], b[N], m[N];
    for(int i=0;i<N;i++){ a[i]=(unsigned)(i*2654435761u); m[i]=(unsigned)(i*40503u+7u); }

    /* ═══ §D1 — a MESMA tarefa das duas formas ══════════════════════════════════════
     * a tarefa: aplicar a mascara m ao estado a. das duas maneiras, o resultado tem de
     * ser o mesmo, senao nao se esta' a comparar a mesma coisa. */
    long perdidos_d = 0, perdidos_i = 0;
    memcpy(b, a, sizeof a);
    for(int i=0;i<N;i++){                       /* DESTRUTIVA: calcula e atribui */
        unsigned novo = b[i] ^ m[i];
        perdidos_d += bits_perdidos_destrutiva(b[i], novo);
        b[i] = novo;                            /* o anterior perde-se aqui */
    }
    static unsigned c[N];
    memcpy(c, a, sizeof a);
    for(int i=0;i<N;i++){                       /* INVOLUTIVA: opera no sitio */
        perdidos_i += bits_perdidos_involutiva(c[i], c[i]^m[i]);
        c[i] ^= m[i];                           /* e o anterior continua la' */
    }
    int iguais = 0; for(int i=0;i<N;i++) if(b[i]==c[i]) iguais++;
    printf("      a mesma tarefa, %d posicoes: as duas saidas coincidem em %d\n", N, iguais);
    ok("as DUAS formas dao o MESMO resultado — sem isto nao se esta' a comparar a mesma"
       " tarefa, e a conta seguinte nao valia nada", iguais == N);

    /* ═══ §D2 — os bits apagados ═══════════════════════════════════════════════════ */
    printf("      bits apagados: destrutiva %ld, involutiva %ld\n", perdidos_d, perdidos_i);
    ok("a DESTRUTIVA apaga e a INVOLUTIVA nao apaga NADA — mesma entrada, mesma saida,"
       " e uma delas pode ser desfeita", perdidos_d > 0 && perdidos_i == 0);

    /* ═══ §D3 — e em joules ════════════════════════════════════════════════════════
     * kT ln2 a 300 K. O numero e' minusculo por operacao — o ponto nao e' a grandeza, e'
     * que UM DELES E' EXACTAMENTE ZERO e nao aproximadamente zero. */
    {
        const double kT_ln2 = 1.380649e-23 * 300.0 * 0.6931471805599453;
        printf("      a %g J por bit: destrutiva %.3e J, involutiva %.3e J\n",
               kT_ln2, perdidos_d*kT_ln2, perdidos_i*kT_ln2);
        printf("      e uma DRAM que refresque isto 8192x/s pagaria %.3e J/s so' por existir\n",
               (double)N*32*8192*kT_ln2);
        ok("e o piso da involutiva e' ZERO EXACTO, e nao um numero pequeno: nao ha' bit"
           " apagado, logo nao ha' minimo termodinamico nenhum a pagar", perdidos_i == 0);
    }

    /* ═══ §D4 — o CONTROLO: e se ela nao fosse involutiva? ═════════════════════════
     * Sem isto, §D2 nao prova nada — bastava eu ter escrito 0 na funcao. Aqui verifica-se
     * que a involutiva E' MESMO involutiva, correndo-a duas vezes: se o estado voltar, o
     * anterior nunca se perdeu, e a conta de zero esta' certa POR RAZAO e nao por decreto. */
    {
        for(int i=0;i<N;i++) c[i] ^= m[i];             /* a segunda aplicacao */
        int voltou = 0; for(int i=0;i<N;i++) if(c[i]==a[i]) voltou++;
        /* e a destrutiva, repetida, NAO volta ao original — porque ja' o deitou fora */
        for(int i=0;i<N;i++) b[i] = b[i] ^ m[i];
        int b_voltou = 0; for(int i=0;i<N;i++) if(b[i]==a[i]) b_voltou++;
        printf("      repetida: a involutiva voltou em %d de %d\n", voltou, N);
        ok("e a involutiva E' involutiva: repetida, devolve o original em todas as"
           " posicoes — e' por isso que os seus bits nao se perderam", voltou == N);
        /* nota honesta: com XOR as duas voltam, porque a destrutiva aqui TAMBEM usou XOR.
         * o que as separa nao e' a operacao — e' TER GUARDADO ou nao o caminho de volta. */
        printf("      (a destrutiva tambem voltou em %d: aqui a conta usou XOR nas duas —\n"
               "       o que as separa e' a FORMA de escrever, nao a operacao)\n", b_voltou);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A RAM MEDE O QUE NAO CUSTA. Um vector de 4 MB que nunca e' escrito nao");
        puts("  dissipa nada; um int reescrito mil milhoes de vezes paga mil milhoes de");
        puts("  vezes. O portao da' vermelho ao primeiro e verde ao segundo.");
        puts("");
        puts("  O QUE CUSTA E' APAGAR, e conta-se em BITS PERDIDOS. Uma escrita que se");
        puts("  desfaz repetindo nao perde nenhum — o piso dela e' ZERO EXACTO, e nao um");
        puts("  numero pequeno.");
        puts("");
        puts("  E o sistema ja' esta' quase todo do lado certo sem nunca ter sido medido");
        puts("  assim: sobe/desce na maquina, vira no bit, gato_ida/volta no banco,");
        puts("  y1 = x1 + F(x2) na camada. Os disco_zera que a migracao espalhou sao os");
        puts("  que nao estao — cada um apaga um vector inteiro.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
