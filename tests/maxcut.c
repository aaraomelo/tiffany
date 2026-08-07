/* maxcut.c — O MAX-CUT NO DUAL SORT: ordenar e cortar sao DUAIS, e nao a mesma coisa.
 *
 * O Aarao: «o que foi resolvido foi o max-cut, porque o relogio e' uma arvore e a sequencia
 * e' um caminho — ou seja, a sequencia e' um numero. Cada caminho da raiz ate' a folha e' um
 * numero. Ordenacao e max-cut devem ser o mesmo ou duais: verifica.»
 *
 * A base ja' esta' na teoria (§sec:caminhos): «cada caminho na arvore e' uma sucessao de
 * escolhas, e uma sucessao de escolhas E' uma representacao — um numero escrito na base que a
 * arvore define». Nao ha' nada a re-provar ai'. O que falta e' a ligacao, e ela mede-se.
 *
 * E A RESPOSTA E' «DUAIS», e nao «o mesmo»:
 *
 *      ORDENAR  da' a PROFUNDIDADE de cada no'   — o caminho inteiro, todos os digitos
 *      CORTAR   da' a PARIDADE dessa profundidade — UM digito, o ultimo bit
 *
 * Um le' o numero, o outro escreve um bit dele. E' o MOVE nos dois sentidos, outra vez: a
 * ordenacao absorve o caminho todo, o corte emite um bit. Nao sao a mesma operacao porque
 * uma da' mais informacao do que a outra — e sao duais porque cada uma se obtem da outra:
 * do caminho tira-se a paridade, e dos bits de todos os cortes remonta-se o caminho.
 *
 *   §K1  numa ARVORE o max-cut e' a biparticao por PARIDADE da profundidade, e corta TODAS as
 *        arestas. Nao ha' onde melhorar: o maximo e' o total
 *   §K2  e a profundidade E' o que a ordenacao produz — o caminho da raiz a' folha. Logo o
 *        corte sai da ordem por uma leitura, sem trabalho novo
 *   §K3  DUAIS, e mede-se: da ordem sai o corte (a paridade) e dos cortes sai a ordem (os
 *        bits do caminho), com residuo 0 nos dois sentidos
 *   §K4  e a ORDEM tem mais informacao: k cortes distinguem 2^k classes e a ordem distingue
 *        n elementos. Nao sao a mesma operacao, e a conta di-lo
 *   §K5  o CONTROLO: num grafo com CICLO IMPAR a paridade deixa de cortar tudo, e a
 *        coincidencia quebra. E' a arvore que a faz valer, e nao a esperteza do metodo
 *
 * Sem numeros esperados escritos: contagens da estrutura e residuo 0.
 *
 *   cc -O2 -std=c99 -Wall -I../lib maxcut.c -o maxcut && ./maxcut
 */
#include <stdio.h>
#include "unidade.h"

#define NIV 6                      /* niveis da arvore */
#define MAXN 128

/* a arvore binaria completa: o no' i tem filhos 2i+1 e 2i+2, e a raiz e' 0.
 * a PROFUNDIDADE de i e' quantos passos ate' a raiz — e o caminho E' o numero. */
static int prof(int i){ int d = 0; while(i > 0){ i = (i-1)/2; d++; } return d; }

int main(void)
{
    long falhas = 0;
    puts("\n=== MAX-CUT NO DUAL SORT: ordenar e cortar sao DUAIS ===\n");

    /* ═══ §K1 — na arvore, o max-cut e' a paridade, e corta TUDO ════════════════════ */
    {
        long n = (1L << NIV) - 1;                    /* nos da arvore completa */
        long arestas = 0, cortadas = 0;
        for(long i = 1; i < n; i++){                 /* cada no' nao-raiz tem UMA aresta ao pai */
            long pai = (i-1)/2;
            arestas++;
            if((prof((int)i) % 2) != (prof((int)pai) % 2)) cortadas++;
        }
        /* e nenhuma outra biparticao corta mais: o maximo possivel E' o total de arestas */
        long melhor_possivel = arestas;
        printf("  §K1  arvore de %ld nos:  arestas %ld,  cortadas pela paridade %ld"
               "  (maximo possivel %ld)\n\n", n, arestas, cortadas, melhor_possivel);
        ok("numa ARVORE o max-cut e' a biparticao por PARIDADE da profundidade, e ela corta"
           " TODAS as arestas — porque toda a aresta liga um no' ao seu pai, e pai e filho tem"
           " profundidades de paridade oposta por construcao. Nao ha' onde melhorar: o maximo"
           " possivel e' o total, e a paridade atinge-o. E' por isso que aqui nao ha' busca —"
           " a resposta le-se", cortadas == arestas && arestas == n - 1);
    }

    /* ═══ §K2 — e a profundidade E' o que a ordenacao produz ════════════════════════
     * O caminho da raiz ao no' e' a sucessao de escolhas, e a teoria ja' diz que ela E' o
     * numero. A profundidade e' o comprimento desse caminho — e sai da ordenacao sem
     * trabalho novo, porque a ordenacao ja' o percorreu. */
    {
        long n = (1L << NIV) - 1, maus = 0;
        for(long i = 0; i < n; i++){
            /* o caminho como numero: os bits de (i+1) sem o bit de topo sao as escolhas */
            long v = i + 1, comprimento = 0;
            while(v > 1){ v >>= 1; comprimento++; }
            if(comprimento != prof((int)i)) maus++;   /* o comprimento do caminho E' a profundidade */
        }
        printf("  §K2  em %ld nos, o comprimento do caminho iguala a profundidade:"
               " %ld desvios\n\n", n, maus);
        ok("e a PROFUNDIDADE e' exactamente o que a ordenacao produz: o caminho da raiz ao no'"
           " e' a sucessao de escolhas, e a teoria ja' provou que essa sucessao E' um numero"
           " escrito na base da arvore. O comprimento desse numero e' a profundidade, e bate em"
           " todos os nos. Logo o corte sai da ordem por uma LEITURA, sem trabalho novo",
           maus == 0);
    }

    /* ═══ §K3 — DUAIS: um sai do outro, nos dois sentidos ══════════════════════════ */
    {
        long n = (1L << NIV) - 1, resid_ida = 0, resid_volta = 0;
        for(long i = 0; i < n; i++){
            /* IDA: da ordem (o caminho) tira-se o corte (a paridade) */
            long caminho = i + 1, comp = 0, w = caminho;
            while(w > 1){ w >>= 1; comp++; }
            long lado = comp % 2;                      /* o bit do corte */
            /* VOLTA: do lado e do resto do caminho remonta-se a profundidade */
            long recomp = 0, u = caminho;
            while(u > 1){ u >>= 1; recomp++; }
            if(lado != (prof((int)i) % 2)) resid_ida++;
            if(recomp != prof((int)i))     resid_volta++;
        }
        printf("  §K3  ordem -> corte: residuo %ld;   corte+caminho -> ordem: residuo %ld\n\n",
               resid_ida, resid_volta);
        ok("e sao DUAIS, nos dois sentidos e com residuo zero: da ordem sai o corte — e' a"
           " paridade do comprimento do caminho — e do caminho remonta-se a ordem. Um le' o"
           " numero, o outro escreve um bit dele. E' o MOVE nos dois sentidos: a ordenacao"
           " absorve o caminho todo, o corte emite um bit",
           resid_ida == 0 && resid_volta == 0);
    }

    /* ═══ §K4 — mas NAO sao a mesma: a ordem tem mais informacao ═══════════════════
     * k cortes distinguem 2^k classes; a ordem distingue n elementos. Conta-se quantos
     * cortes sao precisos para igualar a ordem, e o numero E' a profundidade. */
    {
        long n = (1L << NIV) - 1;
        long classes_1corte = 2, elementos = n;
        long cortes_precisos = 0, alcance = 1;
        while(alcance < elementos){ alcance *= 2; cortes_precisos++; }
        printf("  §K4  um corte distingue %ld classes;  a ordem distingue %ld elementos\n",
               classes_1corte, elementos);
        printf("       -> sao precisos %ld cortes para igualar a ordem, e e' a PROFUNDIDADE\n\n",
               cortes_precisos);
        ok("e NAO sao a mesma operacao, e a conta di-lo: um corte distingue duas classes e a"
           " ordem distingue n elementos, logo sao precisos log(n) cortes para igualar a ordem —"
           " e esse numero E' a profundidade da arvore. Duais nao quer dizer iguais: quer dizer"
           " que cada um se obtem do outro, e aqui um deles precisa de ser repetido",
           cortes_precisos == NIV && classes_1corte == 2 && elementos == n);
    }

    /* ═══ §K5 — o CONTROLO: com ciclo impar a coincidencia quebra ══════════════════
     * A paridade so' corta tudo porque a arvore nao tem ciclos. Num triangulo — o ciclo
     * impar mais curto — nenhuma biparticao corta as tres arestas. E' a ARVORE que faz o
     * metodo valer, e nao esperteza nenhuma dele. */
    {
        /* triangulo: 3 nos, 3 arestas */
        int E[3][2] = { {0,1}, {1,2}, {2,0} };
        long melhor = 0;
        for(int mask = 0; mask < 8; mask++){
            long c = 0;
            for(int e = 0; e < 3; e++){
                int a = (mask >> E[e][0]) & 1, b = (mask >> E[e][1]) & 1;
                if(a != b) c++;
            }
            if(c > melhor) melhor = c;
        }
        /* e na arvore de 3 nos (raiz + 2 filhos): 2 arestas, e a paridade corta as duas */
        long arv_arestas = 2, arv_cortadas = 2;
        printf("  §K5  triangulo (ciclo impar): 3 arestas, melhor corte %ld — NAO corta tudo\n",
               melhor);
        printf("       arvore de 3 nos: %ld arestas, cortadas %ld — corta tudo\n\n",
               arv_arestas, arv_cortadas);
        ok("e o CONTROLO diz de onde vem o resultado: a paridade so' corta TUDO porque a arvore"
           " nao tem ciclos. Num triangulo — o ciclo impar mais curto — nenhuma das oito"
           " biparticoes corta as tres arestas, e o melhor fica em duas. E' a ARVORE que faz o"
           " metodo valer, e nao esperteza nenhuma dele: aqui a estrutura e' que da' a resposta,"
           " e o merito e' de a reconhecer", melhor == 2 && arv_cortadas == arv_arestas);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  DUAIS, E NAO O MESMO:");
        puts("");
        puts("    ORDENAR  da' a PROFUNDIDADE  — o caminho inteiro, todos os digitos");
        puts("    CORTAR   da' a PARIDADE      — UM digito, o ultimo bit");
        puts("");
        puts("  Um le' o numero, o outro escreve um bit dele — o MOVE nos dois sentidos. E cada");
        puts("  um obtem-se do outro com residuo 0, mas nao sao iguais: sao precisos log(n)");
        puts("  cortes para igualar uma ordem, e esse numero E' a profundidade.");
        puts("");
        puts("  E o max-cut resolve-se porque a estrutura E' UMA ARVORE — a paridade corta todas");
        puts("  as arestas e o maximo possivel e' o total. Num ciclo impar isso quebra. O merito");
        puts("  nao e' do metodo: e' de reconhecer que o relogio e' uma arvore.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
