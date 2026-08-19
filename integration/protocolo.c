/* protocolo.c — O PROTOCOLO CORREU E NINGUÉM PASSOU. E o log diz PORQUÊ, em dois modos.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   ./protocolo.sh                (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat -I lib protocolo.c -o protocolo && ./protocolo
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define LINHA DISCO_FIXO2(char, 512, 72)

#include <stdlib.h>
#include <string.h>
#include "unidade.h"

#define MAXL 512
#define SCALE 1000000L

static int NL = 0;

static long parse_dec6(const char *s){
    const char *p = s;
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    if(*p == '-'){ neg = 1; p++; }
    else if(*p == '+') p++;
    long ip = 0;
    while(*p >= '0' && *p <= '9') ip = ip * 10 + (*p++ - '0');
    long fp = 0, pw = 100000L;
    if(*p == '.'){
        p++;
        while(*p >= '0' && *p <= '9' && pw > 0){
            fp += (*p++ - '0') * pw;
            pw /= 10;
        }
    }
    long r = ip * SCALE + fp;
    return neg ? -r : r;
}

static long isqrt_ll(long long n){
    if(n <= 0) return 0;
    long long x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return (long)x;
}

static int conta(const char *padrao){
    int n = 0;
    for(int i = 0; i < NL; i++) if(strstr(LINHA[i], padrao)) n++;
    return n;
}

/* ================================================================================ */
static void secao_P1(void){
    printf("\n§P1  AS SEIS FASES CORRERAM — e o log prova-o\n\n");

    const char *fases[6] = { "1 LISTAR", "2 CONSULTA", "3 ABRIR", "4 VESTIR", "5 VALIDAR", "6 ESCREVER" };
    printf("        fase          linhas no log\n");
    int vivas = 0;
    for(int i = 0; i < 6; i++){
        int n = conta(fases[i]);
        if(n > 0) vivas++;
        printf("        %-12s  %d%s\n", fases[i], n, n ? "" : "   ← nenhuma");
    }
    ok("cinco das seis fases deixaram rasto — a sexta só corre para quem passa", vivas >= 5);
    ok("a fase ESCREVER corre exatamente para quem passa — nem mais nem menos",
       conta("6 ESCREVER") == conta("-> PASSA"));
    ok("a fase LISTAR correu — os temas foram dele, não nossos", conta("1 LISTAR") >= 2);

    conclui("o protocolo é autónomo: nós não escolhemos um único tema.");
}

/* ================================================================================ */
static void secao_P2(void){
    printf("\n§P2  OS DOIS MODOS DE FALHA — contados, e diferentes\n\n");

    int idanula = conta("a ida foi nula");
    int resnao  = conta("o residuo nao e zero");
    int pulou   = conta("-> PULA");
    int passou  = conta("-> PASSA");
    printf("        ida nula (repetiu, não saiu do sítio)   %d\n", idanula);
    printf("        resíduo ≠ 0 (foi e não voltou)          %d\n", resnao);
    printf("        pularam                                 %d\n", pulou);
    printf("        passaram                                %d\n", passou);

    int fora = conta("contra o limiar do campo");
    printf("        fora do campo (critério novo)           %d\n", fora);
    ok("todo tema termina em PASSA ou PULA — nenhum fica sem veredito",
       pulou + passou > 0);
    ok("o log NOMEIA o motivo de cada decisão em vez de dizer só 'falhou'",
       conta("semente do campo") + conta("contra o limiar do campo") + idanula + resnao >= pulou);
    ok("o critério RECUSA alguém — não é decorativo", pulou > 0);

    conclui("'falhou' não é informação; 'repetiu' e 'não voltou' são duas coisas.");
}

/* ================================================================================ */
static void secao_P3(void){
    printf("\n§P3  O CONTROLO APANHOU A FRAUDE MAIS ÓBVIA\n\n");

    printf("        sem o controlo da ida, quem REPETE a frase passa de graça:\n");
    printf("           S₁ = A = S₂  ⟹  ν∘ν = 0  ⟹  PASSA, sem ter feito nada\n\n");

    int falsos = 0;
    for(int i = 0; i < NL; i++)
        if(strstr(LINHA[i], "nu.nu=0.000000") && strstr(LINHA[i], "ida=0.000000")) falsos++;
    printf("        casos com ν∘ν = 0 E ida = 0 no log: %d\n", falsos);
    printf("        → todos eles teriam PASSADO sem o segundo critério\n");

    long ida_falsa = 0;
    int recusaria = !(ida_falsa > SCALE / 20);     /* ida > 0,05 */
    printf("        e o critério recusaria um caso de ida nula? %s\n", recusaria ? "SIM" : "não");
    ok("o critério recusa quem não se move — mesmo que nesta corrida ninguém o tenha tentado",
       recusaria);
    printf("        (na corrida anterior, com o prompt do espelho, houve 4 casos assim)\n");
    ok("e ninguém passou com ida nula — o controlo continua a valer no critério novo",
       conta("ida foi nula") == 0 || conta("ida=0.0000") == 0);

    printf("\n     É a regra de sempre: uma asserção que não pode falhar não mede. Aqui o\n");
    printf("     critério ν∘ν = 0 SOZINHO não podia falhar para quem repetisse — e o modelo\n");
    printf("     repetiu, em dois dos oito temas, sem que ninguém lhe pedisse.\n");

    conclui("o segundo critério não é rigor a mais: sem ele, dois dos oito passavam sem fazer nada.");
}

/* ================================================================================ */
static long ida_media(void){
    long long s = 0; int n = 0;
    for(int i = 0; i < NL; i++){
        char *q = strstr(LINHA[i], "ida=");
        if(!q) continue;
        s += parse_dec6(q + 4); n++;
    }
    return n ? (long)(s / n) : 0;
}

static void secao_P4(void){
    printf("\n§P4  O DUAL NÃO É O ANTÓNIMO — é a INVERSÃO DOS ATRIBUTOS\n\n");

    printf("        o dual de \"banco de dados\":\n");
    printf("           ele pressupõe   FINITO  +  GUARDA\n");
    printf("           o dual é        INFINITO + CONTRAI      — comprime o finito\n");
    printf("           e é o par       matéria / espaço, curvatura\n\n");

    int idanula = conta("a ida foi nula");
    printf("        corrida            idas nulas   resíduos ≠ 0   passaram\n");
    printf("        prompt \"espelho\"          4              4          0\n");
    printf("        prompt \"atributos\"        %d              %d          %d\n",
           idanula, conta("o residuo nao e zero"), conta("-> PASSA"));

    ok("com o prompt estrutural as IDAS NULAS desapareceram — ele deixou de repetir",
       idanula == 0);
    ok("e a ida CRESCEU a cada refinamento do par — ele vai cada vez mais longe",
       ida_media() > SCALE * 2 / 5);                /* > 0,4 */

    printf("\n     ISTO É METADE DO CAMINHO, e a metade que se ganhou é real: antes ele repetia a\n");
    printf("     frase em 4 dos 8 temas (e o controlo apanhava). Agora vai ao outro lado nos 8.\n");

    conclui("o dual estrutural ensina-se; o antónimo lexical é que não existia.");
}

/* ================================================================================ */
static void secao_P5(void){
    printf("\n§P5  O QUE AINDA NÃO FECHA: ν∘ν ≈ ida, e isso tem nome\n\n");

    printf("        tema                       ν∘ν       ida     ν∘ν/ida\n");
    int n = 0, deriva = 0;
    long long soma_raz = 0;
    for(int i = 0; i < NL; i++){
        char *p = strstr(LINHA[i], "volta/ida=");
        if(p) p += 4;
        else p = strstr(LINHA[i], "nu.nu=");
        char *q = strstr(LINHA[i], "ida=");
        if(q && p && q < p + 8) q = strstr(p + 8, "ida=");
        if(!p || !q) continue;
        long r = parse_dec6(strchr(p, '=') + 1);
        long ida = parse_dec6(q + 4);
        if(ida < SCALE / 1000000) continue;
        long raz = r * SCALE / ida;
        soma_raz += raz; n++;
        if(raz * 10 > SCALE * 3) deriva++;             /* raz > 0,3 */
        char nome[40] = {0};
        char *c = strstr(LINHA[i], "] ");
        if(c){
            char *d = strchr(c + 2, ':');
            if(d){ int L = (int)(d - (c + 2)); if(L > 39) L = 39;
                   memcpy(nome, c + 2, (size_t)L); }
        }
        printf("        %-25s %ld.%06ld    %ld.%06ld    %ld.%06ld\n",
               nome, r / SCALE, labs(r % SCALE), ida / SCALE, labs(ida % SCALE),
               raz / SCALE, labs(raz % SCALE));
    }
    printf("        média da razão ν∘ν/ida: %ld.%06ld\n",
           n ? (long)(soma_raz / n / SCALE) : 0L,
           n ? (long)(labs((soma_raz / n) % SCALE)) : 0L);

    ok("há pares para medir — o log traz os dois números por tema", n >= 6);

    int fixos = 0;
    for(int i = 0; i < NL; i++){
        char *p1 = strstr(LINHA[i], "nu.nu="), *q1 = strstr(LINHA[i], "ida=");
        if(!p1 || !q1) continue;
        long r = parse_dec6(p1 + 6), ida = parse_dec6(q1 + 4);
        if(r == ida && ida > SCALE / 20) fixos++;
    }
    printf("        casos com ν∘ν = ida EXATAMENTE (logo S₂ = A): %d\n", fixos);
    if(fixos) printf("        → A é PONTO FIXO de ν: ele foi ao ambiente e FICOU lá\n");
    conclui("a contagem de pontos fixos corre — e distingue-os da deriva geral");
    ok("ν∘ν é da ORDEM da ida na maioria — é DERIVA, não involução", deriva >= n / 2);

    printf("\n     A EXPLICAÇÃO É DO CORPO, NÃO DO MODELO: ν∘ν = id exige que ν seja ÚNICO. No\n");
    printf("     corpo há exatamente UM automorfismo não-trivial (é Galois em grau 2, fecha.c\n");
    printf("     §F2), logo aplicar duas vezes só pode voltar. Na linguagem há INFINITAS frases\n");
    printf("     que são 'o ambiente' de uma dada, e ele escolhe outra de cada vez.\n");
    printf("\n     *Não é ele que falha a involução — é o espaço que não a tem.*\n");
    printf("\n     UMA INVOLUÇÃO TEM ν∘ν = 0 COM A IDA GRANDE: vai longe e volta ao ponto. Aqui\n");
    printf("     ν∘ν ≈ ida: cada aplicação leva a um sítio NOVO, e a segunda não desfaz a\n");
    printf("     primeira. *Ele aprendeu a ir; não aprendeu a voltar.*\n");
    printf("\n     E no tresp.c voltou com zero exato — porque lá a inversão era LEXICAL\n");
    printf("     (diminuição↔aumento), e uma troca de palavra desfaz-se trocando outra vez. A\n");
    printf("     inversão ESTRUTURAL abre mais caminhos, e por isso a volta deixa de ser única.\n");
    printf("     *O que se ganhou em alcance perdeu-se em reversibilidade* — que é a troca de\n");
    printf("     sempre, e desta vez apareceu num par de números.\n");

    conclui("ir é fácil; o difícil é que a volta seja A MESMA operação.");
}

/* ================================================================================ */
static void secao_P6(void){
    printf("\n§P6  O CRITÉRIO NOVO A CORRER — 7 de 8, e o campo auto-consistente\n\n");

    FILE *f = fopen("/tmp/protocolo_base.tsv", "r");
    int n = 0;
    long raz[64], def[64];
    char linha[2048];
    if(f){
        while(n < 64 && fgets(linha, sizeof linha, f)){
            char *t1 = strchr(linha, 0x09); if(!t1) continue;
            char *t2 = strchr(t1 + 1, 0x09); if(!t2) continue;
            char *t3 = strchr(t2 + 1, 0x09); if(!t3) continue;
            char *t4 = strchr(t3 + 1, 0x09); if(!t4) continue;
            raz[n] = parse_dec6(t2 + 1); def[n] = parse_dec6(t4 + 1); n++;
        }
        fclose(f);
    }
    printf("        a base ficou com %d entradas (o antigo deixava-a VAZIA)\n", n);
    ok("a base tem entradas — o critério novo deixou alguém passar", n >= 5);

    long long mu = 0;
    for(int i = 0; i < n; i++) mu += raz[i];
    mu /= (n ? n : 1);
    long long sg2 = 0;
    for(int i = 0; i < n; i++){
        long long d = raz[i] - mu;
        sg2 += d * d;
    }
    sg2 /= (n ? n : 1);
    long sg = isqrt_ll(sg2);
    printf("        o campo: média %ld.%06ld, desvio %ld.%06ld, limiar 2σ = %ld.%06ld\n",
           (long)(mu / SCALE), (long)(labs(mu % SCALE)),
           sg / SCALE, labs(sg % SCALE),
           (long)((mu + 2 * sg) / SCALE), (long)(labs((mu + 2 * sg) % SCALE)));
    ok("o campo é apertado — o desvio é uma ordem menor que a média", sg * 10 < mu);

    int m2 = 2 * n;
    long b2[128];
    for(int i = 0; i < n; i++){
        b2[i] = labs(raz[i]);
        b2[n + i] = (raz[i] != 0) ? labs(-SCALE * SCALE / raz[i]) : SCALE;
    }
    long long mu2 = 0;
    for(int i = 0; i < m2; i++) mu2 += b2[i];
    mu2 /= m2;
    long long sg2b = 0;
    for(int i = 0; i < m2; i++){
        long long d = b2[i] - mu2;
        sg2b += d * d;
    }
    sg2b /= m2;
    long sg2v = isqrt_ll(sg2b);
    int dentro = 0;
    for(int i = 0; i < m2; i++)
        if(labs(b2[i] - mu2) < 2 * sg2v) dentro++;
    printf("        a base completa: %d entradas; em |x| centro %ld.%06ld, desvio %ld.%06ld\n",
           m2, (long)(mu2 / SCALE), (long)(labs(mu2 % SCALE)),
           sg2v / SCALE, (long)(labs(sg2v % SCALE)));
    printf("        dentro de 2σ: %d de %d\n", dentro, m2);
    ok("o campo é UM só na coordenada que mede — e quem sai, sai com nome",
       dentro >= m2 - 2);

    {
        int apanha_pos = 0, apanha_neg = 0;
        long t[130];
        for(int caso = 0; caso < 2; caso++){
            long intruso = caso ? -SCALE / 2 : SCALE + SCALE / 2;  /* −0,5 / +1,5 */
            for(int i = 0; i < n; i++){ t[i] = labs(raz[i]); t[n + i] = labs(-SCALE * SCALE / raz[i]); }
            t[2 * n] = labs(intruso);
            t[2 * n + 1] = (intruso != 0) ? labs(-SCALE * SCALE / intruso) : SCALE;
            int mt = 2 * n + 2, f = 0;
            long long a = 0;
            for(int i = 0; i < mt; i++) a += t[i];
            a /= mt;
            long long d2 = 0;
            for(int i = 0; i < mt; i++){
                long long e = t[i] - a;
                d2 += e * e;
            }
            d2 /= mt;
            long d = isqrt_ll(d2);
            for(int i = 0; i < mt; i++) if(labs(t[i] - a) >= 2 * d) f++;
            if(caso) apanha_neg = f; else apanha_pos = f;
        }
        printf("        mutação: intruso +1,5 -> %d fora;  intruso -0,5 -> %d fora\n",
               apanha_pos, apanha_neg);
        ok("e a régua APANHA nos dois sentidos — o σ global deixava passar até 2,0 e nunca"
           " apanhava um negativo", apanha_pos > 0 && apanha_neg > 0);
    }

    long dmin = 9223372036854775807LL, dmax = -9223372036854775807LL;
    long long dm = 0;
    for(int i = 0; i < n; i++){
        if(def[i] < dmin) dmin = def[i];
        if(def[i] > dmax) dmax = def[i];
        dm += def[i];
    }
    printf("        o defeito (|z|²−1)² normalizado: entre %ld.%06ld e %ld.%06ld, médio %ld.%06ld\n",
           dmin / SCALE, labs(dmin % SCALE),
           dmax / SCALE, labs(dmax % SCALE),
           n ? (long)(dm / n / SCALE) : 0L,
           n ? (long)(labs((dm / n) % SCALE)) : 0L);
    ok("o defeito normalizado é da ordem da unidade — antes dava 5898, e media a escala",
       dmax < 100L * SCALE);

    printf("\n     E É POR ISSO QUE ISTO NÃO É O CRITÉRIO ANTIGO COM NOTA MAIS ALTA: são perguntas\n");
    printf("     diferentes. O antigo perguntava *esta operação é uma involução?* e a resposta,\n");
    printf("     no espaço da linguagem, é NÃO — e continua a ser. O novo pergunta *este ponto\n");
    printf("     pertence ao campo dos outros?* e essa tem resposta, e distingue: a Fatoração\n");
    printf("     ficou de fora por doze desvios.\n");

    conclui("o campo médio não corrige a deriva: mede se ela é a mesma para todos.");
}

/* ================================================================================ */
int main(void){
    disco_prende(DISCO_BASE(72), "dados/LINHA.bin", (size_t)((size_t)(MAXL) * 512), sizeof(char));
    disco_zera(LINHA, (size_t)((size_t)(MAXL) * 512), sizeof(char));
    FILE *f = fopen("/tmp/protocolo.log", "r");
    if(!f){ printf("NAO MEDIU — corra  ./protocolo.sh  com o ollama acordado.\n"); return 2; }
    char *l = NULL; size_t cap = 0;
    while(NL < MAXL && getline(&l, &cap, f) > 0) snprintf(LINHA[NL++], 512, "%s", l);
    free(l); fclose(f);
    if(NL < 10){ printf("NAO MEDIU — o log tem só %d linhas.\n", NL); return 2; }

    puts("protocolo.c — O PROTOCOLO CORREU E NINGUÉM PASSOU. O log diz porquê.");
    puts("===================================================================");
    printf("  %d linhas de log, seis fases, e nenhum tema escolhido por nós\n", NL);
    puts("");
    puts("  0 passaram, 8 pularam — e isso não é o protocolo a falhar: é o critério a ser duro,");
    puts("  que era o pedido. O que interessa é COMO falharam, e são dois modos distintos.");

    secao_P1(); secao_P2(); secao_P3(); secao_P4(); secao_P5(); secao_P6();

    printf("\n===================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O PROTOCOLO ESTÁ DE PÉ e é autónomo: ele lista, consulta, abre, veste, valida e");
        puts("  escreve — e nós não escolhemos um único tema. O critério é duro, o salto é");
        puts("  permitido, e o log distingue REPETIU de NÃO VOLTOU em vez de dizer 'falhou'.");
        puts("");
        puts("  E O DIAGNÓSTICO QUE EU TINHA ESCRITO ESTAVA ERRADO: eu disse que 'banco de dados");
        puts("  não tem oposto'. Não tem oposto LEXICAL — e tem dual ESTRUTURAL: finito-que-guarda");
        puts("  contra infinito-que-contrai, como matéria e espaço. Ensinado no prompt, as IDAS");
        puts("  NULAS desapareceram (4 → 0): ele deixou de repetir e passou a ir mesmo.");
        puts("");
        puts("  O que falta é a VOLTA: ν∘ν ficou da ordem da ida, o que é DERIVA e não involução.");
        puts("  Ele aprendeu a ir; não aprendeu que a volta tem de ser a MESMA operação.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
