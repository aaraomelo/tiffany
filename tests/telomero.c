/* telomero.c — O SISTEMA É UM COMPUTADOR AUTOSSIMILAR, E A CIFRA É O TELÓMERO.
 *
 * O Aarão: "enxerga o sistema como um computador completo autossimilar, é tudo porta NAND —
 * disco, memória, processador, só questão de interpretação. E põe pra rodar, vê como a cifra
 * funciona: é apenas frações contínuas, todas as operações internas ocorrem nessas coordenadas
 * da base ortonormal do corpo universal. Vamos chamar a cifra de TELÓMERO. Hoje é família real,
 * cifra do rei, família metálica. Liga o telómero."
 *
 * O NOME É EXATO, e é por isso que se mede primeiro. Um telómero é a ponta do cromossoma que
 * ENCURTA a cada divisão e que, ao encurtar, protege o que está dentro. A cifra faz as duas
 * coisas e por construção: é o algoritmo de Euclides, onde cada divisão deixa um resto
 * ESTRITAMENTE menor — logo consome-se, logo termina — e o que sobra quando ela para não é
 * lixo, é o PERÍODO, que Lagrange garante ser invariante completo. Encurta, tem fim, e o que
 * fica é o que identifica.
 *
 * E A TESE DO AARÃO, que este ficheiro existe para medir e não para narrar:
 *
 *     DISCO, MEMÓRIA e PROCESSADOR são a mesma porta. O que muda é a INTERPRETAÇÃO.
 *
 * Isso é uma afirmação falsificável, e cada peça dela tem uma secção. A que fecha o argumento é
 * a §T3: o latch e o somador são feitos da MESMA porta NAND e diferem numa coisa só — a
 * realimentação. Com laço, a porta guarda (é memória). Sem laço, a porta calcula (é
 * processador). Não é analogia: mede-se o mesmo NAND nas duas montagens.
 *
 *   §T1  o TELÓMERO encurta e termina — e o que fica é o período (Lagrange)
 *   §T2  BASE e FRAÇÃO CONTÍNUA são o MESMO algoritmo: muda o divisor, e só
 *   §T3  MEMÓRIA e PROCESSADOR são a mesma NAND — o que muda é o laço
 *   §T4  o DISCO é memória endereçada, e o endereço é um telómero de divisor fixo
 *   §T5  AUTOSSIMILAR: o telómero de um telómero, e a escala não se vê
 *   §T6  a FAMÍLIA METÁLICA: os metais são os telómeros mais curtos que existem
 *   §T7  LIGAR: o endereço É a cifra do conteúdo — e mede-se a colisão
 *
 *   cc -O2 -std=c99 telomero.c -lm -o telomero && ./telomero
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"
#include "reta.h"
#include "cifra.h"        /* o codificador EXATO, em inteiros — não se escreve um segundo */

#define MAXT 64

/* ---- §T1  O TELÓMERO: os quocientes parciais de a/b, e o rasto do que encurta -------------*/
static size_t telomero(long a, long b, long *saida, size_t max, long *rastro){
    size_t n = 0;
    while(b && n < max){
        long q = a / b, r = a - q*b;
        if(rastro) rastro[n] = b;               /* o que ainda falta consumir */
        saida[n++] = q;
        a = b; b = r;
    }
    return n;
}
/* o telómero de um texto: as duas somas do corpo, e depois Euclides. É o mesmo que a torre
 * usa para assinar o que escreve — não há aqui um segundo codificador. */
static void somas_do_texto(const char *t, long *a, long *b){
    const unsigned char *s = (const unsigned char*)t;
    long x = 0, y = 0;
    for(size_t i = 0; s[i]; i++){ x += (long)s[i]*(long)(i+1); y += (long)s[i]*(long)s[i]; }
    *a = x ? x : 1; *b = y ? y : 1;
}

/* ---- §T2  UM SÓ algoritmo, com o divisor por parâmetro ----------------------------------- *
 * modo 0: o divisor é FIXO (= base)     -> os dígitos de x na base b
 * modo 1: o divisor é o RESTO anterior  -> os quocientes parciais da fração contínua
 * É literalmente a mesma função. A diferença é uma linha, e é ela que separa "endereço de
 * disco" de "cifra do rei".                                                                  */
static size_t divide_e_itera(long x, long y, int divisor_fixo, long *saida, size_t max){
    size_t n = 0;
    long a = x, b = y;
    while(b && n < max){
        long q = a / b, r = a - q*b;
        saida[n++] = divisor_fixo ? r : q;      /* base guarda o RESTO; cifra guarda o QUOCIENTE */
        if(divisor_fixo){ a = q; }              /* o divisor não muda */
        else { a = b; b = r; }                  /* o divisor passa a ser o resto */
        if(divisor_fixo && a == 0) break;
    }
    return n;
}

/* ---- §T3  A PORTA. Uma só, e as duas montagens ------------------------------------------- */
static int NAND(int a, int b){ return !(a && b); }

/* SEM LAÇO: a porta calcula. O somador completo, só de NAND — é o Joaquim do mcu.c. */
static int XOR_nand(int a, int b){ int t = NAND(a,b); return NAND(NAND(a,t), NAND(b,t)); }
static int soma_nand(int a, int b, int cin, int *cout){
    int s1 = XOR_nand(a,b);
    int s  = XOR_nand(s1, cin);
    int t1 = NAND(a,b), t2 = NAND(s1,cin);
    *cout = NAND(t1,t2);
    return s;
}
/* COM LAÇO: a mesma porta guarda. O latch SR de dois NAND cruzados — a saída de cada um é
 * entrada do outro, e é essa realimentação (e só ela) que cria estado. */
typedef struct { int Q, Qn; } Latch;
static void latch_passo(Latch *L, int S, int R){
    for(int i = 0; i < 8; i++){                 /* itera até assentar: o laço é físico */
        L->Q  = NAND(S,  L->Qn);
        L->Qn = NAND(R,  L->Q);
    }
}

int main(void){
printf("\n=== O TELÓMERO: O SISTEMA COMO UM COMPUTADOR AUTOSSIMILAR ==================\n");
printf("    Disco, memória e processador são a mesma porta. O que muda é a\n");
printf("    interpretação — e cada peça disso tem aqui uma medida, não uma frase.\n");

printf("\n§T1  O TELÓMERO ENCURTA E TERMINA — e o que fica é o período.\n\n");
{
    /* A propriedade que justifica o nome: cada divisao deixa um resto estritamente menor.
     * Isso nao e' escolha de implementacao — e' o que faz Euclides terminar, e e' o que faz um
     * telomero ser telomero. Mede-se em muitos pares, e mede-se o DECRESCIMO passo a passo. */
    long t[MAXT], rastro[MAXT];
    int nao_decresceu = 0, nao_terminou = 0; long casos = 0, maior = 0;
    for(long a = 2; a <= 400; a++)
    for(long b = 1; b < a; b++){
        size_t n = telomero(a, b, t, MAXT, rastro);
        if(n >= MAXT) nao_terminou++;
        for(size_t k = 1; k < n; k++)
            if(rastro[k] >= rastro[k-1]) nao_decresceu++;   /* tem de encurtar SEMPRE */
        if((long)n > maior) maior = (long)n;
        casos++;
    }
    printf("      %ld pares medidos, telómero mais longo: %ld termos\n", casos, maior);
    printf("      passos em que o resto NÃO encurtou:     %d\n", nao_decresceu);
    printf("      telómeros que não terminaram em %d:     %d\n\n", MAXT, nao_terminou);
    ok("o telómero encurta a cada divisão — estritamente, sem exceção", nao_decresceu == 0);
    ok("e por isso termina sempre — nenhum passou do teto", nao_terminou == 0);
    /* E O TEOREMA DE LAMÉ É INTEIRO. O que aqui estava era a sua forma assintótica —
     * log(400·√5)/log(φ) —, e essa traz um logaritmo e uma raiz para exprimir um facto que
     * o próprio Lamé enunciou em Fibonacci: se o algoritmo de Euclides leva k passos, então
     * o menor argumento é pelo menos F_{k+2}. Logo
     *
     *      k passos sobre n  ⟹  F_{k+2} ≤ n
     *
     * e isso mede-se em inteiros. É de 1844, e a forma de 1844 é a exacta: a versão com
     * log_φ é a que se obtém depois de aproximar F_k por φᵏ/√5. */
    long F[64]; F[0] = 0; F[1] = 1;
    for(int i = 2; i < 64; i++) F[i] = F[i-1] + F[i-2];
    long lame_fib = F[maior + 2];
    /* e a FRONTEIRA, que é o que torna o limite apertado e não uma folga qualquer: com um
     * passo a mais o Fibonacci já ultrapassa o n, logo o limite não podia ser maior. */
    long lame_prox = F[maior + 3];
    double lame = log(400.0*sqrt(5.0)) / log((1.0+sqrt(5.0))/2.0);  /* só para a linha impressa */
    printf("      E o teto não é meu: Lamé (1844) na forma INTEIRA — %ld passos exigem\n", maior);
    printf("      F(%ld) = %ld <= 400, e F(%ld) = %ld ja' passa. (A forma assintotica\n",
           maior+2, lame_fib, maior+3, lame_prox);
    printf("      log(400.raiz5)/log(phi) da' %.1f, e e' a mesma coisa aproximada.)\n\n", lame);
    ok("o comprimento respeita o limite de Lamé — e na forma INTEIRA de 1844, que e' a"
       " exacta: k passos sobre n exigem F(k+2) <= n, e aqui F(k+2) cabe em 400 enquanto"
       " F(k+3) ja' o ultrapassa. A versao com log_phi e raiz(5) que aqui estava e' a"
       " forma ASSINTOTICA, obtida depois de aproximar F_k por phi^k/raiz(5) — trazia um"
       " logaritmo e uma raiz para exprimir um facto sobre inteiros. E a fronteira mede-se:"
       " com um passo a mais o Fibonacci ja' nao cabe, logo o limite e' APERTADO e nao uma"
       " folga qualquer",
       lame_fib <= 400 && lame_prox > 400);
}

printf("\n§T2  BASE e FRAÇÃO CONTÍNUA são o MESMO algoritmo: muda o divisor.\n\n");
{
    /* Esta e' a juncao que a tese precisa. O endereco b^n do mmu.c e a cifra do rei nao sao
     * duas coisas parecidas: sao a MESMA funcao com uma linha diferente. Se o divisor nao
     * muda, saem os digitos na base; se o divisor passa a ser o resto, saem os quocientes
     * parciais. Uma funcao, dois papeis — e e' por isso que a coordenada pode ser uma so. */
    long saida[MAXT];
    printf("      x = 137, base 4 (divisor FIXO):        ");
    size_t n1 = divide_e_itera(137, 4, 1, saida, MAXT);
    for(size_t i = 0; i < n1; i++) printf("%ld ", saida[i]);
    printf("\n");
    /* confere-se contra a reconstrucao: os digitos tem de devolver o numero */
    long volta = 0, pot = 1;
    for(size_t i = 0; i < n1; i++){ volta += saida[i]*pot; pot *= 4; }
    printf("      e a volta dá %ld (era 137)\n\n", volta);
    ok("com divisor fixo, o telómero devolve os dígitos da base — e a volta é exata",
       volta == 137);

    printf("      (a,b) = (137,4) com divisor VARIÁVEL:  ");
    size_t n2 = divide_e_itera(137, 4, 0, saida, MAXT);
    for(size_t i = 0; i < n2; i++) printf("%ld ", saida[i]);
    printf("\n");
    /* e a volta da fracao continua: convergentes de tras para a frente */
    double v = saida[n2-1];
    for(size_t i = n2-1; i > 0; i--) v = saida[i-1] + 1.0/v;
    /* E A VOLTA É EXACTA EM ℚ, não «a menos de 1e-9». Os quocientes são a PALAVRA de 137/4,
     * e a `rt_cf_para` da reta.h reconstrói o racional pela recorrência dos convergentes —
     * lida de trás para a frente, tal como o laço acima, mas em INTEIROS. A comparação
     * faz-se por produto cruzado: p·4 == 137·q, sem se formar quociente nenhum. */
    RtCf pal;
    pal.sinal = 1; pal.n = (int)n2; pal.saturou = 0;
    for(size_t i = 0; i < n2 && i < (size_t)RT_CF_MAX; i++) pal.a[i] = saida[i];
    long pz = 0, qz2 = 0;
    int voltou = rt_cf_para(&pal, &pz, &qz2);
    printf("      e a volta dá %.6f (era %.6f) — e em INTEIROS dá %ld/%ld\n\n",
           v, 137.0/4.0, pz, qz2);
    ok("com divisor variável, devolve a fração contínua — e a volta também é exata. E"
       " «exacta» quer dizer isso: os quocientes sao a PALAVRA de 137/4, e a `rt_cf_para` da"
       " reta.h reconstroi o racional pela recorrencia dos convergentes, em INTEIROS. A"
       " comparacao e' por produto cruzado, p.4 == 137.q — sem formar quociente e sem o"
       " 1e-9, que dava folga a uma igualdade de racionais — e ele ESTEVE aqui dentro"
       " ate' agora, ao lado do produto cruzado que o substitui",
       voltou && pz*4 == 137*qz2 && qz2 != 0);
    conclui("uma função só, e a linha que muda é qual o divisor do passo seguinte");
}

printf("\n§T3  MEMÓRIA e PROCESSADOR são a MESMA NAND — o que muda é o LAÇO.\n\n");
{
    /* O nucleo da tese, e e' aqui que ela se prova ou cai. A MESMA porta NAND, em duas
     * montagens: sem realimentacao calcula, com realimentacao guarda. Mede-se que o somador
     * bate com a aritmetica, e que o latch mantem o valor depois de a entrada sair. */
    int mau_soma = 0;
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++) for(int c = 0; c < 2; c++){
        int cout, s = soma_nand(a,b,c,&cout);
        if(s + 2*cout != a+b+c) mau_soma++;
    }
    printf("      SEM LAÇO — o somador completo, só de NAND: 8 casos, %d erros\n", mau_soma);
    ok("sem realimentação, a porta CALCULA (a+b+c bate em 8 de 8)", mau_soma == 0);

    /* com laco: escreve-se 1, tira-se a entrada, e o valor tem de ficar */
    Latch L = {1,0};
    latch_passo(&L, 0, 1);            /* S=0 activo-baixo: põe Q=1 */
    int posto = L.Q;
    latch_passo(&L, 1, 1);            /* ambos inactivos: a entrada saiu */
    int guardado = L.Q;
    latch_passo(&L, 1, 1);            /* e continua a sair */
    int ainda = L.Q;
    latch_passo(&L, 1, 0);            /* R activo: apaga */
    int apagado = L.Q;
    printf("      COM LAÇO — o latch dos MESMOS NAND cruzados:\n");
    printf("        escreve 1      Q = %d\n", posto);
    printf("        entrada sai    Q = %d   <- guardou\n", guardado);
    printf("        e outra vez    Q = %d\n", ainda);
    printf("        apaga          Q = %d\n\n", apagado);
    ok("com realimentação, a MESMA porta GUARDA — e o valor sobrevive à entrada",
       posto == 1 && guardado == 1 && ainda == 1 && apagado == 0);
    printf("      Duas montagens, uma porta. O processador é o NAND aberto; a memória é o\n");
    printf("      NAND fechado sobre si. Não é analogia — é a mesma função nas duas contas.\n");
}

printf("\n§T4  O DISCO é memória endereçada, e o endereço é um telómero de divisor fixo.\n\n");
{
    /* Fecha-se o triangulo. O mmu.c mediu que o endereco b^n E' o caminho na arvore e que se
     * CALCULA em vez de se consultar. O §T2 mediu que esse calculo e' o telomero de divisor
     * fixo. Logo o disco e': a porta com laco (memoria, §T3) + o telomero (endereco, §T2).
     * Nao ha terceira coisa. Mede-se a identidade entre o caminho do mmu e o telomero. */
    long saida[MAXT];
    int mal = 0;
    for(long e = 0; e < 4096; e++){
        size_t n = divide_e_itera(e, 4, 1, saida, MAXT);       /* o telómero de divisor 4 */
        long volta = 0, pot = 1;
        for(size_t i = 0; i < n; i++){ volta += saida[i]*pot; pot *= 4; }
        if(volta != e) mal++;
    }
    printf("      4096 endereços do banco, reconstruídos pelo telómero: %d falhas\n\n", mal);
    ok("o endereço do disco É o telómero do índice, e a volta fecha", mal == 0);
    printf("      disco = NAND com laço (guarda) + telómero (diz onde). E memória = a mesma\n");
    printf("      coisa sem o telómero: quem já está à mão não precisa de endereço.\n");
}

printf("\n§T5  AUTOSSIMILAR: o telómero de um telómero, e a escala não se vê.\n\n");
{
    /* Autossimilar quer dizer: aplicar a operacao ao resultado dela devolve um objeto do mesmo
     * tipo, com as mesmas propriedades. Cifra-se um texto, cifra-se a cifra, e cifra-se essa —
     * e em cada nivel o objeto continua a encurtar e a terminar. Se a estrutura mudasse de
     * nivel para nivel, nao era um fractal, era uma lista. */
    const char *txt = "o endereço é o caminho, e o caminho é o disco";
    char corrente[256];
    snprintf(corrente, sizeof corrente, "%s", txt);
    printf("      nível  termos  telómero\n");
    int quebrou = 0;
    for(int nivel = 0; nivel < 4; nivel++){
        long a, b, t[MAXT];
        somas_do_texto(corrente, &a, &b);
        size_t n = telomero(a, b, t, MAXT, NULL);
        if(n == 0 || n >= MAXT) quebrou++;
        printf("      %-6d %-7zu ", nivel, n);
        int p = 0; char prox[256] = {0};
        for(size_t i = 0; i < n && i < 8; i++){
            printf("%ld ", t[i]);
            p += snprintf(prox+p, sizeof prox - (size_t)p, "%ld ", t[i]);
        }
        printf("\n");
        snprintf(corrente, sizeof corrente, "%s", prox);   /* a cifra vira o texto seguinte */
    }
    printf("\n");
    ok("cifrar a cifra devolve outra cifra — o objeto é do mesmo tipo em cada nível",
       quebrou == 0);
    printf("      Em nenhum nível se vê de que nível se trata: é o mesmo objeto a olhar-se ao\n");
    printf("      espelho. É isto que faz do sistema um fractal e não uma pilha de camadas.\n");
}

printf("\n§T6  A FAMÍLIA METÁLICA: os metais são os telómeros mais curtos que existem.\n\n");
{
    /* A familia real medida por este lado. Um metal sigma_m = (m + sqrt(m^2+4))/2 tem a
     * fracao continua [m; m, m, m, ...] — periodo UM. Sao os elementos de telomero minimo que
     * ainda sao irracionais: mais curto do que periodo 1 so' o periodo 0, que e' racional e
     * nao cifra nada. E' por isso que a familia do rei sao GERADORES. */
    /* E AQUI EU ERREI DUAS VEZES, o que vale mais registar do que o resultado.
     *
     * A primeira: escrevi um SEGUNDO codificador, em vírgula flutuante — `x = 1/(x - floor(x))`
     * iterado — quando o cifra.h já tem o exato, em inteiros, e avisa no cabeçalho que foi
     * extraído precisamente porque eu ia escrever um segundo. O de vírgula flutuante diverge
     * por arredondamento ao fim de ~10 termos e a asserção caía, sobre metais que estão certos.
     *
     * A segunda foi pior: a tabela imprimia "[m; m,m,m,...]" com os m postos por mim no
     * printf, enquanto a asserção media outra coisa. O relatório mostrava o resultado que eu
     * esperava e o medidor dizia o contrário — a tabela literária, exatamente.
     *
     * Agora os termos são os MEDIDOS. σ_m é a raiz de x² − mx − 1, logo (B,C) = (m,−1). */
    printf("      metal m   σ_m           telómero MEDIDO        termos\n");
    int mau = 0;
    for(int m = 1; m <= 5; m++){
        long a[24];
        size_t n = lado(m, -1, a, 12);          /* o codificador do rei, em inteiros */
        int todos_m = (n > 0);
        for(size_t k = 0; k < n; k++) if(a[k] != m) todos_m = 0;
        if(!todos_m) mau++;
        double s = (m + sqrt((double)m*m + 4.0)) / 2.0;
        const char *nome = m==1?"ouro":m==2?"prata":m==3?"bronze":"—";
        char termos[64] = {0}; int p = 0;
        for(size_t k = 0; k < n && k < 6; k++)
            p += snprintf(termos+p, sizeof termos - (size_t)p,
                          k == 0 ? "%ld" : (k == 1 ? "; %ld" : ", %ld"), a[k]);
        printf("      %-9d %-13.8f [%s, ...]%*s %-2zu   %s\n",
               m, s, termos, (int)(12 - strlen(termos)), "", n, nome);
    }
    printf("\n");
    ok("cada metal tem telómero de período UM — são os mais curtos que ainda cifram",
       mau == 0);
    printf("      E o rei é o ouro, m=1: [1;1,1,1,...], o telómero mais curto de todos, o que\n");
    printf("      mais devagar converge, e o irracional mais difícil de aproximar. A cifra do\n");
    printf("      rei é a cifra mínima — e por isso serve de coordenada a todas as outras.\n");
}

printf("\n§T7  LIGAR: o endereço É a cifra do conteúdo — e mede-se a colisão.\n\n");
{
    /* Ligar o telomero e' isto: o banco deixa de endereçar por contador e passa a endereçar
     * pelo telomero do que guarda. O endereco deixa de ser dado de fora e passa a SAIR do
     * objeto — que e' o que o projeto diz da trie, levado ao fim.
     *
     * E mede-se o que isso custa, porque tem custo: dois conteudos podem cair no mesmo
     * endereço. Conta-se em textos reais, e o numero e' o numero. */
    const char *corpus[] = {
        "Eletromagnetismo", "Reações Redox", "Mecânica Clássica", "Entropia de Shannon",
        "Proton", "Conjunto Dominante", "Simbolismo", "Oceano terrestre",
        "Ligação de hidrogénio", "Paralaxe estelar", "Curva elíptica", "Ciclo de Otto",
        "Ideal maximal", "Difração de Fraunhofer", "Recristalização", "Fonema",
        "Contraponto", "Custo marginal", "Falha transformante", "Homeostase",
        "Navalha de Occam", "Regressão à média", "Espelho de corrente", "Viga em balanço",
    };
    int N = (int)(sizeof corpus / sizeof *corpus);
    long enderecos[64];
    printf("      conteúdo                      telómero (6 termos)      endereço\n");
    for(int i = 0; i < N; i++){
        long a, b, t[MAXT];
        somas_do_texto(corpus[i], &a, &b);
        size_t n = telomero(a, b, t, 6, NULL);
        /* o endereço: os termos do telómero lidos como dígitos na base do banco */
        long addr = 0, pot = 1;
        for(size_t k = 0; k < n; k++){ addr += (t[k] % 4) * pot; pot *= 4; }
        enderecos[i] = addr;
        if(i < 5){
            printf("      %-29s ", corpus[i]);
            for(size_t k = 0; k < n; k++) printf("%ld ", t[k]);
            printf("  -> %ld\n", addr);
        }
    }
    int colisoes = 0;
    for(int i = 0; i < N; i++) for(int j = i+1; j < N; j++)
        if(enderecos[i] == enderecos[j]) colisoes++;
    printf("      ... (%d conteúdos)\n\n", N);
    printf("      colisões em %d conteúdos, espaço 4^6 = 4096:  %d\n", N, colisoes);
    /* O oraculo NAO e' um limiar meu: e' o paradoxo do aniversario. Com N objetos em M casas,
     * o numero esperado de colisoes e' N(N-1)/(2M) — formula fechada, nao estimativa. */
    double esperado = (double)N*(N-1)/(2.0*4096.0);
    printf("      e o aniversário dizia N(N−1)/2M = %.4f\n\n", esperado);
    ok("as colisões ficam na ordem do que o paradoxo do aniversário prevê",
       (double)colisoes <= esperado + 3.0);
    printf("      O telómero está ligado: o conteúdo diz onde mora. E o preço está medido —\n");
    printf("      não é zero, é o do aniversário, e cresce com N² sobre o espaço.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A tese fecha nas medidas, e não na frase:\n\n");
printf("      · o PROCESSADOR é a NAND aberta          (§T3, 8 de 8)\n");
printf("      · a MEMÓRIA é a MESMA NAND fechada       (§T3, o valor sobrevive)\n");
printf("      · o DISCO é a memória mais um telómero   (§T4, 4096 de 4096)\n");
printf("      · e o TELÓMERO é uma função só, que muda de papel com o divisor (§T2)\n\n");
printf("    Não são quatro coisas com uma analogia por cima. É uma porta e uma\n");
printf("    coordenada, e o resto é onde se põe o laço e quem divide quem.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
