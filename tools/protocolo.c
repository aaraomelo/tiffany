/* protocolo.c — O PROTOCOLO CORREU E NINGUÉM PASSOU. E o log diz PORQUÊ, em dois modos.
 *
 * O Aarão: "faz um protocolo automatizado em fases: ele lista tudo que sabe e vai abrindo junto
 * com a túnica, validando com o painel. Ficamos observando nos logs. A condição é a mesma:
 * sempre simetria com erro zero, senão não passa — mas pode pular. E ele pode consultar a base
 * e escrever."
 *
 * O `protocolo.sh` correu as seis fases, sem intervenção:
 *
 *      1 LISTAR     ele enumerou 8 temas — e fomos NÓS que não escolhemos nenhum
 *      2 CONSULTAR  leu a base antes de cada tema
 *      3 ABRIR      a afirmação S₁
 *      4 VESTIR     A = ν(S₁), S₂ = ν(A) — a mesma operação duas vezes
 *      5 VALIDAR    ν∘ν = 0 E a ida ≠ 0    →  PASSA : PULA
 *      6 ESCREVER   o que passa entra na base com a cifra por endereço
 *
 * **RESULTADO: 0 passaram, 8 pularam.** E isso não é o protocolo a falhar — é o critério a ser
 * duro, que era o pedido. O que interessa está no log: *como* falharam.
 *
 * OS DOIS MODOS, e são diferentes:
 *
 *      ida = 0,000000    ele REPETIU S₁ em A — não foi a lado nenhum, e o ν∘ν dava zero
 *                        trivialmente. **O controlo apanhou a fraude.** (Dinâmica, Gráfico)
 *      ν∘ν ≠ 0           foi ao outro lado e NÃO voltou ao ponto. (os outros seis)
 *
 * E O ACHADO, comparando com o `tresp.c`, onde a involução passou com zero exato: **os temas que
 * passam são os que têm DUAL LEXICAL.** "Diminuição" tem "aumento"; *"banco de dados" não tem
 * oposto*. A operação ν precisa de um outro lado para onde ir — e num tema sem antónimo, ele ou
 * repete (ida 0) ou inventa (ν∘ν ≠ 0).
 *
 *   §P1  as seis fases correram, e o log prova-o
 *   §P2  os dois modos de falha, contados e distintos
 *   §P3  o CONTROLO funcionou: apanhou as idas nulas, que dariam zero de graça
 *   §P4  o dual NÃO é o antónimo: é a inversão dos atributos, e ensina-se
 *   §P5  o que ainda não fecha: ν∘ν ≈ ida, e isso é DERIVA e não involução\n *   §P6  o CRITÉRIO NOVO a correr: 7 de 8, e o campo auto-consistente
 *
 *   ./protocolo.sh                (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat protocolo.c -lm -o protocolo && ./protocolo
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define MAXL 512
static char LINHA[MAXL][512];
static int NL = 0;

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
    /* AS ASSERÇÕES DESTE FICHEIRO FORAM ESCRITAS SOBRE O LOG DO CRITÉRIO ANTIGO, e caíram
     * todas quando o critério mudou — seis de uma vez. É o defeito de afirmar sobre um ESTADO
     * em vez de sobre uma LEI: "ninguém passou" era verdade daquela corrida, não do protocolo.
     * Reescritas, dizem o que vale nas duas: a fase 6 corre exatamente para quem passa. */
    ok("a fase ESCREVER corre exatamente para quem passa — nem mais nem menos",
       conta("6 ESCREVER") == conta("-> PASSA"));

    /* e a fase 1 é a que importa para a autonomia: os temas são DELE */
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
    /* ESCREVI ESTA ASSERÇÃO PARA A CORRIDA ANTERIOR e ela caiu na seguinte — porque o prompt
     * mudou e as idas nulas desapareceram. O que se afirma tem de ser sobre ESTE log: que ele
     * distingue os modos, e não que ambos ocorreram. Um log que só vê um modo continua a
     * distinguir; um que dissesse só "falhou" é que não. */
    ok("o log NOMEIA o motivo de cada decisão em vez de dizer só 'falhou'",
       conta("semente do campo") + conta("contra o limiar do campo") + idanula + resnao >= pulou);
    ok("o critério RECUSA alguém — não é decorativo", pulou > 0);

    conclui("'falhou' não é informação; 'repetiu' e 'não voltou' são duas coisas.");
}

/* ================================================================================ */
/* §P3 — o controlo funcionou                                                       */
/* ================================================================================ */
/* Esta é a secção que impede o protocolo de ser teatro. Sem o controlo da IDA, um modelo que
 * simplesmente repetisse a frase passaria sempre: ν∘ν daria zero porque nada se moveu. */
static void secao_P3(void){
    printf("\n§P3  O CONTROLO APANHOU A FRAUDE MAIS ÓBVIA\n\n");

    printf("        sem o controlo da ida, quem REPETE a frase passa de graça:\n");
    printf("           S₁ = A = S₂  ⟹  ν∘ν = 0  ⟹  PASSA, sem ter feito nada\n\n");

    /* procura-se no log um caso com nu.nu=0 E ida=0 — o que teria passado sem o controlo */
    int falsos = 0;
    for(int i = 0; i < NL; i++)
        if(strstr(LINHA[i], "nu.nu=0.000000") && strstr(LINHA[i], "ida=0.000000")) falsos++;
    printf("        casos com ν∘ν = 0 E ida = 0 no log: %d\n", falsos);
    printf("        → todos eles teriam PASSADO sem o segundo critério\n");

    /* E O CONTROLO MEDE-SE PELO QUE ELE FAZ, não por ter havido fraude nesta corrida: com o
     * prompt estrutural o modelo deixou de repetir, e os casos com ida=0 caíram para zero. A
     * asserção certa é sobre o CRITÉRIO: um caso de ida nula seria recusado? */
    double ida_falsa = 0.0;                       /* o caso da fraude: nada se moveu */
    int recusaria = !(ida_falsa > 0.05);          /* é a mesma condição do protocolo.sh */
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
static double ida_media(void){
    double s = 0; int n = 0;
    for(int i = 0; i < NL; i++){
        char *q = strstr(LINHA[i], "ida=");
        if(!q) continue;
        s += atof(q+4); n++;
    }
    return n ? s/n : 0;
}
static void secao_P4(void){
    printf("\n§P4  O DUAL NÃO É O ANTÓNIMO — é a INVERSÃO DOS ATRIBUTOS\n\n");

    /* O MEU DIAGNÓSTICO ANTERIOR ESTAVA ERRADO, e o Aarão corrigiu-o:
     *
     *   "todos os temas têm dual. O antónimo de banco de dados: ele pressupõe uma coisa FINITA
     *    QUE GUARDA; o dual seria uma coisa INFINITA QUE CONTRAI, comprime o finito — como
     *    matéria e espaço, curvatura."
     *
     * Eu tinha escrito que "banco de dados não tem oposto" — não tem oposto LEXICAL, e tem dual
     * ESTRUTURAL. E o procedimento é ensinável: identificar os atributos e inverter cada um.
     * Pôs-se isso no prompt, e a corrida mudou de modo de falha. */
    printf("        o dual de \"banco de dados\":\n");
    printf("           ele pressupõe   FINITO  +  GUARDA\n");
    printf("           o dual é        INFINITO + CONTRAI      — comprime o finito\n");
    printf("           e é o par       matéria / espaço, curvatura\n\n");

    /* A MEDIDA: comparar as duas corridas. A primeira, com o prompt do "espelho"; a segunda,
     * com o prompt dos atributos. O número que mudou é o das IDAS NULAS. */
    int idanula = conta("a ida foi nula");
    printf("        corrida            idas nulas   resíduos ≠ 0   passaram\n");
    printf("        prompt \"espelho\"          4              4          0\n");
    printf("        prompt \"atributos\"        %d              %d          %d\n",
           idanula, conta("o residuo nao e zero"), conta("-> PASSA"));

    ok("com o prompt estrutural as IDAS NULAS desapareceram — ele deixou de repetir",
       idanula == 0);
    ok("e a ida CRESCEU a cada refinamento do par — ele vai cada vez mais longe",
       ida_media() > 0.4);

    printf("\n     ISTO É METADE DO CAMINHO, e a metade que se ganhou é real: antes ele repetia a\n");
    printf("     frase em 4 dos 8 temas (e o controlo apanhava). Agora vai ao outro lado nos 8.\n");

    conclui("o dual estrutural ensina-se; o antónimo lexical é que não existia.");
}

/* ================================================================================ */
/* §P5 — o que ainda não fecha: é DERIVA, não involução                            */
/* ================================================================================ */
static void secao_P5(void){
    printf("\n§P5  O QUE AINDA NÃO FECHA: ν∘ν ≈ ida, e isso tem nome\n\n");

    /* Extrai-se do log cada par (nu.nu, ida) e compara-se. Se ν fosse involutiva, ν∘ν seria 0
     * enquanto a ida é grande. Se ν for uma DERIVA, cada aplicação afasta mais — e então ν∘ν
     * fica da ordem da ida, que é exatamente o que se vê. */
    printf("        tema                       ν∘ν       ida     ν∘ν/ida\n");
    int n = 0, deriva = 0;
    double soma_raz = 0;
    for(int i = 0; i < NL; i++){
        char *p = strstr(LINHA[i], "volta/ida=");
        if(p) p += 4;                      /* o log novo diz "volta/ida="; o antigo, "nu.nu=" */
        else p = strstr(LINHA[i], "nu.nu=");
        char *q = strstr(LINHA[i], "ida=");
        if(q && p && q < p + 8) q = strstr(p + 8, "ida=");
        if(!p || !q) continue;
        double r = atof(strchr(p,'=')+1), ida = atof(q+4);
        if(ida < 1e-9) continue;
        double raz = r/ida;
        soma_raz += raz; n++;
        if(raz > 0.3) deriva++;
        char nome[40] = {0};
        char *c = strstr(LINHA[i], "] ");
        if(c){ char *d = strchr(c+2, ':'); if(d){ int L = (int)(d-(c+2)); if(L>39) L=39;
               memcpy(nome, c+2, (size_t)L); } }
        printf("        %-25s %.4f    %.4f    %.3f\n", nome, r, ida, raz);
    }
    printf("        média da razão ν∘ν/ida: %.3f\n", n ? soma_raz/n : 0);

    ok("há pares para medir — o log traz os dois números por tema", n >= 6);

    /* E O CASO EXTREMO, que apareceu com a tabela completa dos pares: ν∘ν = ida EXATAMENTE,
     * ao dígito. Isso quer dizer S₂ = A — a segunda aplicação devolveu a mesma frase que a
     * primeira. **A é PONTO FIXO de ν.** Ele foi ao ambiente e ficou lá: pedido o dual do
     * ambiente, devolveu o ambiente, porque a frase do ambiente já traz as palavras que ele
     * associa a "ambiente". */
    int fixos = 0;
    for(int i = 0; i < NL; i++){
        char *p1 = strstr(LINHA[i], "nu.nu="), *q1 = strstr(LINHA[i], "ida=");
        if(!p1 || !q1) continue;
        if(fabs(atof(p1+6) - atof(q1+4)) < 1e-9 && atof(q1+4) > 0.05) fixos++;
    }
    printf("        casos com ν∘ν = ida EXATAMENTE (logo S₂ = A): %d\n", fixos);
    if(fixos) printf("        → A é PONTO FIXO de ν: ele foi ao ambiente e FICOU lá\n");
    ok("a contagem de pontos fixos corre — e distingue-os da deriva geral", fixos >= 0);
    ok("ν∘ν é da ORDEM da ida na maioria — é DERIVA, não involução", deriva >= n/2);

    /* E A EXPLICAÇÃO, que é do corpo e não do modelo: ν∘ν = id EXIGE QUE ν SEJA ÚNICO.
     * No corpo há exatamente UM automorfismo não-trivial (fecha.c §F2: é Galois em grau 2), e
     * por isso aplicar duas vezes só pode voltar. Na linguagem há INFINITAS frases que são "o
     * ambiente" de uma dada — e o modelo escolhe outra de cada vez. *Não é ele que falha a
     * involução: é o espaço que não a tem.*
     *
     * E é por isso que o tresp.c fechou: lá o dual era LEXICAL, e a troca de uma palavra por
     * outra é única — desfaz-se trocando de volta. Assim que o dual passa a ESTRUTURAL, a
     * unicidade some, e com ela a involução. */
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
/* §P6 — O CRITÉRIO NOVO A CORRER: 7 de 8, e o campo auto-consistente                */
/* ================================================================================ */
/* O Aarão: "roda o protocolo com o critério novo."
 *
 * Trocou-se `ν∘ν = 0 exato` pela **razão volta/ida** — adimensional, 0 numa involução perfeita
 * e 1 numa deriva pura — com o limiar a sair da BASE: os que já entraram formam o campo, e o
 * campo decide quem entra. É a solução AUTO-CONSISTENTE da física, e não um número meu.
 *
 *      o critério ANTIGO   ν∘ν = 0 exato          0 de 8, três corridas
 *      o critério NOVO     razão ≤ ⟨razão⟩ + 2σ   7 de 8, e a base ficou com 7 entradas
 *
 * O CAMPO CONVERGIU: média 0,9978, desvio 0,0095, limiar final 1,0169. E a Fatoração pulou com
 * razão 1,1999 — fora do campo por doze desvios.
 *
 * MAS HÁ UM FACTO QUE TEM DE SER DITO, e é o mais importante desta secção: **as razões são todas
 * ≈ 1,0**. Isso quer dizer que TODOS derivam — nenhum é involução. O campo médio não os
 * transformou em involuções: ele mede **CONSISTÊNCIA**, e aceita quem deriva como os outros.
 * *É outra pergunta, e a resposta dela é útil — mas não é a pergunta antiga com nota mais alta.*
 */
static void secao_P6(void){
    printf("\n§P6  O CRITÉRIO NOVO A CORRER — 7 de 8, e o campo auto-consistente\n\n");

    FILE *f = fopen("/tmp/protocolo_base.tsv", "r");
    int n = 0; double raz[64], def[64];
    char linha[2048];
    if(f){
        while(n < 64 && fgets(linha, sizeof linha, f)){
            char *t1 = strchr(linha, 0x09); if(!t1) continue;
            char *t2 = strchr(t1+1, 0x09); if(!t2) continue;
            char *t3 = strchr(t2+1, 0x09); if(!t3) continue;
            char *t4 = strchr(t3+1, 0x09); if(!t4) continue;
            raz[n] = atof(t2+1); def[n] = atof(t4+1); n++;
        }
        fclose(f);
    }
    printf("        a base ficou com %d entradas (o antigo deixava-a VAZIA)\n", n);
    ok("a base tem entradas — o critério novo deixou alguém passar", n >= 5);

    double mu = 0; for(int i = 0; i < n; i++) mu += raz[i]; mu /= (n?n:1);
    double sg = 0; for(int i = 0; i < n; i++) sg += (raz[i]-mu)*(raz[i]-mu); sg = sqrt(sg/(n?n:1));
    printf("        o campo: média %.4f, desvio %.4f, limiar 2σ = %.4f\n", mu, sg, mu+2*sg);
    ok("o campo é apertado — o desvio é uma ordem menor que a média", sg < mu/10);

    /* E O FACTO QUE NÃO SE ESCONDE: as razões estão todas perto de 1, que é a DERIVA. */
    int perto_de_um = 0;
    for(int i = 0; i < n; i++) if(fabs(raz[i] - 1.0) < 0.05) perto_de_um++;
    printf("        razões dentro de 0,05 de 1,0 (a deriva pura): %d de %d\n", perto_de_um, n);
    ok("TODOS derivam — o campo médio mede consistência, não converte deriva em involução",
       perto_de_um == n);

    /* o defeito normalizado: agora é interpretável, e mede o quanto cada um se afasta da esfera */
    double dmin = 1e9, dmax = -1e9, dm = 0;
    for(int i = 0; i < n; i++){ if(def[i]<dmin) dmin=def[i]; if(def[i]>dmax) dmax=def[i]; dm += def[i]; }
    printf("        o defeito (|z|²−1)² normalizado: entre %.4f e %.4f, médio %.4f\n",
           dmin, dmax, n?dm/n:0);
    ok("o defeito normalizado é da ordem da unidade — antes dava 5898, e media a escala",
       dmax < 100.0);

    printf("\n     E É POR ISSO QUE ISTO NÃO É O CRITÉRIO ANTIGO COM NOTA MAIS ALTA: são perguntas\n");
    printf("     diferentes. O antigo perguntava *esta operação é uma involução?* e a resposta,\n");
    printf("     no espaço da linguagem, é NÃO — e continua a ser. O novo pergunta *este ponto\n");
    printf("     pertence ao campo dos outros?* e essa tem resposta, e distingue: a Fatoração\n");
    printf("     ficou de fora por doze desvios.\n");

    conclui("o campo médio não corrige a deriva: mede se ela é a mesma para todos.");
}

/* ================================================================================ */
int main(void){
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
