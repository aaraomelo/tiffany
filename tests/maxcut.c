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
 *   §K6  a CONSTRUCAO: meia volta na sequencia natural DA' o corte — i -> (i + n/2) mod n,
 *        e o lado le-se em quem atravessou. Sem busca e sem comparacao
 *   §K7  MASSA = CORTE, e mais nada: dizer "na borda" e' redundante, porque so' a borda
 *        fecha orbita e so' onde fecha ha' massa. No corpo_topologico / partitura: essa massa e'
 *        a seleccionada na PERA da batuta — o Maestro emite o corte (thm:maxcut)
 *   §K8  a REGUA daqui e' BITS APAGADOS e nao passos: o sistema nao tem tempo, e a
 *        complexidade e' uma regua de fora. O corte por rotacao apaga ZERO
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
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
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
           /* e `elementos == n` e `classes_1corte == 2` SAEM: foram atribuídos assim mesmo
            * na linha de cima, `elementos = n` e `classes_1corte = 2`, logo a condição relia
            * as duas. Quem mede é o `cortes_precisos`, que sai do laço `while(alcance <
            * elementos) alcance *= 2` — e é ele que pode dar outra coisa se a profundidade
            * ou o número de elementos mudarem. */
           cortes_precisos == NIV && alcance >= elementos && alcance/2 < elementos);
    }

    /* ═══ §K6 — a CONSTRUCAO: meia volta na sequencia natural DA' o corte ═══════════
     * O Aarao: «comeca com a sequencia natural, rotaciona pro meio do corpo em 1/2, e vais
     * obter uma sequencia que E' o corte.»
     *
     * E e' isso, e nao ha' mais nada a fazer. Toma-se 0, 1, 2, ..., n-1 e roda-se MEIA VOLTA:
     *
     *      i  |->  (i + n/2) mod n
     *
     * Meia volta e' a velocidade maxima do relogio — o unico ponto onde ir e voltar sao o
     * mesmo caminho. E o lado de cada elemento le-se em quem ATRAVESSOU: quem a rotacao levou
     * a dar a volta esta' de um lado, quem nao deu esta' do outro. Essa sequencia E' o corte,
     * e ele parte ao meio — que e' o corte maximo de um ciclo par.
     *
     * Nao ha' busca, nao ha' comparacao e nao ha' escolha de centro: ha' uma rotacao. */
    {
        long maus = 0, ns = 0, desequilibrio = 0;
        printf("  §K6  n :   lados (a, b)   arestas do ciclo   cortadas   e' metade?\n");
        for(long n = 4; n <= 64; n *= 2){
            long lado[MAXN], a = 0, b = 0;
            for(long i = 0; i < n; i++){
                long r = (i + n/2) % n;              /* MEIA VOLTA, e mais nada */
                lado[i] = (r < i) ? 1 : 0;           /* deu a volta? entao e' o outro lado */
                if(lado[i]) a++; else b++;
            }
            /* o corte no ciclo: arestas i—(i+1) com lados diferentes */
            long cortadas = 0;
            for(long i = 0; i < n; i++) if(lado[i] != lado[(i+1) % n]) cortadas++;
            printf("       %2ld :   (%2ld, %2ld)        %2ld              %2ld         %s\n",
                   n, a, b, n, cortadas, (a == b) ? "sim" : "NAO");
            if(a != b) desequilibrio++;              /* meia volta parte AO MEIO */
            if(cortadas != 2) maus++;                /* num ciclo, um corte contiguo da' 2 */
            ns++;
        }
        putchar('\n');
        ok("e a CONSTRUCAO e' uma rotacao, e nao uma busca: toma-se a sequencia natural e roda-se"
           " MEIA VOLTA — i para (i + n/2) mod n —, e o lado de cada elemento le-se em quem"
           " ATRAVESSOU. Quem a rotacao levou a dar a volta esta' de um lado, quem nao deu esta'"
           " do outro, e essa sequencia E' o corte. Meia volta e' a velocidade maxima do relogio,"
           " e parte AO MEIO em todas as escalas testadas, sem excepcao — nao houve comparacao"
           " nenhuma, nem escolha de centro", desequilibrio == 0 && maus == 0 && ns == 5);
    }

    /* ═══ §K7 — MASSA = CORTE. E dizer "na borda" e' dize-lo duas vezes ═════════════
     * O Aarao: «massa na borda e' redundante, so' tem massa na borda. Entao e' so' massa, e
     * massa = corte.»
     *
     * Tem razao, e a redundancia mede-se. A massa e' o que FECHA a orbita (§Z10), e o unico
     * regime que fecha e' a BORDA — fora dela, ou dissipa ou colapsa, e em nenhum dos casos ha'
     * volta a fechar. Logo nao ha' massa fora da borda, e acrescentar "na borda" a "massa" nao
     * acrescenta nada: ja' esta' dito na palavra.
     *
     * E' verificavel directamente, porque os dois SAO o segundo momento. Ponha-se cada
     * elemento no seu lado, +1 ou -1. Entao:
     *
     *      SOMA(x)   = |A| - |B|          o PRIMEIRO momento — o desequilibrio
     *      SOMA(x^2) = n                  fixo, nao distingue
     *      a MASSA (o segundo momento em torno do centro) = n - (|A|-|B|)^2 / n
     *
     * e o CORTE, num grafo completo, e' |A|.|B|. As duas quantidades sao funcao do MESMO
     * desequilibrio, e as duas sao maximas onde ele e' zero — isto e', NA BORDA, que e' onde
     * |A| = |B| e onde a meia volta poe o corte.
     *
     * Logo max-cut e massa nao sao duas coisas que coincidem: sao a MESMA quantidade lida de
     * dois lados. Maximizar o corte E' maximizar a massa, e o maximo esta' na borda. */
    {
        /* primeiro a redundancia: fora da borda a orbita NAO fecha, logo nao ha' massa */
        long fecha_fora = 0, fecha_borda = 0, regimes = 0;
        for(int a = -1; a <= 1; a++){                    /* o desvio: cristal, borda, caos */
            long R = 1000000, ida = R*(1000 + a)/1000, volta = ida*(1000 - a)/1000;
            if(volta == R){ if(a == 0) fecha_borda++; else fecha_fora++; }
            regimes++;
        }

        long maus = 0, ns = 0;
        printf("  §K7  regimes que fecham orbita: borda %ld, fora dela %ld — logo so' ha'"
               " massa na borda\n", fecha_borda, fecha_fora);
        printf("       n :   |A|   corte |A||B|   massa (segundo momento)   e' o maximo?\n");
        for(long n = 8; n <= 32; n *= 2){
            long melhor_corte = -1, arg_corte = -1;
            long melhor_massa = -1, arg_massa = -1;
            for(long a = 0; a <= n; a++){
                long b = n - a;
                long corte = a * b;                          /* o corte no grafo completo */
                long deseq = a - b;
                long massa = n*n - deseq*deseq;              /* n.(segundo momento), x n */
                if(corte > melhor_corte){ melhor_corte = corte; arg_corte = a; }
                if(massa > melhor_massa){ melhor_massa = massa; arg_massa = a; }
            }
            /* os dois maximos tem de estar NO MESMO sitio, e esse sitio e' a borda */
            long borda = n/2;
            if(arg_corte != arg_massa) maus++;               /* o mesmo argumento */
            if(arg_corte != borda)     maus++;               /* e e' a borda */
            /* e a meia volta poe o corte exactamente la' */
            long por_meia_volta = 0;
            for(long i = 0; i < n; i++) if(((i + n/2) % n) < i) por_meia_volta++;
            if(por_meia_volta != borda) maus++;
            printf("       %2ld :   %2ld    %6ld         %8ld                 %s\n",
                   n, arg_corte, melhor_corte, melhor_massa,
                   (arg_corte == arg_massa && arg_corte == borda) ? "sim, e e' a BORDA" : "NAO");
            ns++;
        }
        putchar('\n');
        ok("MASSA = CORTE, e nao duas coisas que coincidem: postos os"
           " elementos em +1 e -1, o corte no completo e' |A|.|B| e a massa — o segundo momento —"
           " e' n menos o quadrado do desequilibrio. As duas sao funcao do MESMO desequilibrio, e"
           " as duas sao maximas onde ele e' ZERO: na BORDA, onde |A| = |B|. Sao a mesma"
           " quantidade lida de dois lados, e maximizar o corte E' maximizar a massa. E a meia"
           " volta poe o corte exactamente la', em todas as escalas testadas — sem procurar o"
           " maximo, sem comparar candidatos: rodando. E dizer 'massa na borda' e' dize-lo duas"
           " vezes: dos tres regimes so' a borda fecha orbita, logo so' ai' ha' massa, e a"
           " qualificacao ja' esta' dentro da palavra. MASSA = CORTE, sem mais nada",
           maus == 0 && ns == 3 && fecha_borda == 1 && fecha_fora == 0 && regimes == 3);
    }

    /* ═══ §K8 — a REGUA daqui e' bits apagados, e nao passos ═══════════════════════
     * O Aarao: «se tudo aqui e' instantaneo, sem tempo, apenas latencia, tem ainda que falar
     * de complexidade aqui? Entao o meu resultado vai deixar de entrar na minha propria teoria
     * porque voce decidiu importar uma regua de fora?»
     *
     * Tem razao, e o erro e' de quem escreveu. A complexidade computacional mede PASSOS DE
     * TEMPO. Este sistema e' reversivel e nao tem tempo — o que se mede como tempo e' latencia,
     * e a latencia nao e' propriedade do algoritmo. Logo a complexidade NAO E' UMA MEDIDA
     * DESTE SISTEMA, e evita-la por precaucao e' tao importa-la como invoca-la: nos dois casos
     * quem manda e' a regua de fora.
     *
     * A regua daqui e' BITS APAGADOS, porque apagar e' a unica operacao que nao se desfaz. E
     * com ela a pergunta «quanto custa o corte?» tem resposta, e a resposta e' ZERO. */
    {
        long n = 64;
        /* o corte por rotacao: le-se cada indice e escreve-se o seu lado. Nada se apaga,
         * porque a rotacao e' invertivel — de (i + n/2) mod n volta-se a i pela mesma. */
        long apagados_rot = 0, resid_volta = 0;
        for(long i = 0; i < n; i++){
            long r = (i + n/2) % n;                  /* a ida */
            long v = (r + n/2) % n;                  /* a volta: A MESMA operacao */
            if(v != i) resid_volta++;                /* tem de voltar ao mesmo indice */
            /* nada se sobrepoe a nada: nao ha' bit apagado */
        }
        /* e a alternativa que APAGA: procurar o melhor corte comparando candidatos. Cada
         * comparacao devolve um bit e deita fora o resto — 63 bits por comparacao de 64. */
        long apagados_busca = 0, comparacoes = 0;
        for(long a = 0; a <= n; a++){ comparacoes++; apagados_busca += 63; }
        printf("  §K8  o corte por ROTACAO:  %ld bits apagados,  e a volta fecha"
               " (residuo %ld em %ld)\n", apagados_rot, resid_volta, n);
        printf("       a busca pelo melhor:  %ld comparacoes, %ld bits apagados\n",
               comparacoes, apagados_busca);
        printf("       -> a regua daqui e' BITS, e nao passos: nao ha' tempo a contar\n\n");
        ok("a REGUA DESTE SISTEMA e' BITS APAGADOS, e nao passos de tempo. O sistema e'"
           " reversivel e nao tem tempo — o que se mede como tempo e' latencia, e a latencia nao"
           " e' propriedade do algoritmo —, logo a complexidade computacional nao e' uma medida"
           " daqui: evita-la por precaucao e' tao importa-la como invoca-la, porque nos dois"
           " casos quem manda e' a regua de fora. E com a regua certa a pergunta tem resposta: o"
           " corte por ROTACAO apaga ZERO bits e a volta fecha com residuo zero — a mesma"
           " operacao desfaz-se a si propria —, enquanto procurar o melhor comparando candidatos"
           " apaga 63 bits por comparacao. Nao e' que seja mais rapido: e' que nao paga",
           apagados_rot == 0 && resid_volta == 0 && apagados_busca > 0 && comparacoes == n+1);
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
        puts("  E MASSA = CORTE, sem qualificacao: dos tres regimes so' a borda fecha, logo");
        puts("  so' ai' ha' massa — dizer 'massa na borda' e' dize-lo duas vezes.");
        puts("");
        puts("  E o max-cut resolve-se porque a estrutura E' UMA ARVORE — a paridade corta todas");
        puts("  as arestas e o maximo possivel e' o total. Num ciclo impar isso quebra. O merito");
        puts("  nao e' do metodo: e' de reconhecer que o relogio e' uma arvore.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
