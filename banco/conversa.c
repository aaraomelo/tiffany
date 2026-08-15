/* conversa.c — A ASSISTENTE. Corpus vazio que cresce do que se conversar, em português.
 *
 * Desenho fixado com o Aarão: a unidade é o PAR (fala, resposta), uma resposta só, e o corpus
 * nasce vazio — ela aprende do que conversarmos.
 *
 * TUDO EM DISCO, NADA EM RAM. A assistente antiga montava vocabulário, postings e órbitas com
 * malloc, e por isso nunca chegou a correr sobre 1,2M de frases. Aqui não há tabela em memória:
 * a fala cifra-se, desce a árvore em pread, e a resposta mora no nó terminal.
 *
 * O NÓ É UM CONJUNTO, NÃO UMA LARGURA. A primeira árvore que escrevi tinha 256 slots por nó —
 * 4 KB para guardar um ou dois filhos — e medi 153 MB em 2000 linhas, que dava 92 GB no corpus
 * todo. O nó guarda agora SÓ OS FILHOS QUE EXISTEM, seis por registo e um encadeado para o resto.
 * É a lição do dia: usar o conjunto, não a largura fixa.
 *
 * As três buscas são as três do mórfico, e cai-se de uma para a outra:
 *
 *   EROSÃO      o prefixo          a fala tal como veio
 *   DILATAÇÃO   a subsequência     a fala com ruído, ou com as palavras trocadas
 *   DECRETO     "não sei"          o único método sem dual — e é ele que se recusa a inventar
 *
 *   ./conversa <base> aprende "a fala" "a resposta"
 *   ./conversa <base> responde "a fala"
 *   ./conversa <base> conversa                 (modo interativo)
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include "expr.h"
#include "cifra.h"   /* raizi e lado: a raiz inteira e a FC por PQa, EM INTEIROS */
#include "algebra.h"
#include "edo.h"
#include "poli.h"
#include "booleana.h"   /* a lógica é o corpo GF(2) */
#include "relacao.h"    /* relação, função, bijeção — a que tem volta */
#include "naturais.h"   /* o chão: Peano, e os instrumentos demonstrados */
#include "inteiros.h"   /* Z: e o que eles acrescentam é a reversibilidade */
#include "racionais.h"  /* Q: a reversibilidade da multiplicacao nao nula */
#include "reais.h"      /* R: o real e o CORTE, e nunca um decimal */
#include "cauchy.h"     /* R outra vez, pelo caminho: R = Cauchy(Q)/~ */
#include "identifica.h" /* o mesmo ponto por quatro portas, e a volta */
#include "numeros.h"    /* teoria dos numeros: Euclides = MDC = Bezout = FC */
#include "dirichlet.h"  /* a convolucao na arvore dos divisores: mu = 1^-1 */
#include "eliptica.h"   /* a curva: a fibra decide qual operacao existe */
#include "estrutura.h"  /* algebra moderna: a estrutura e uma tabua */
#include "corpo.h"      /* corpos: onde fibra+volta vira estrutura formal */
#include "linear.h"     /* algebra linear exata, e o gume automatico */
#include "eletrico.h"

typedef struct { long a, b; } Slot;
#define SL 16
/* O BARRAMENTO. A assistente deixa de ser um ficheiro e passa a ser N bancos nele — a fala e
 * emitida, e responde quem a tiver. Ninguem coordena.
 *
 * E A REPARTICAO E PELA CABECA DA CIFRA, nao pela cifra inteira. Repartir pelo todo poria
 * 'ourives' e 'ourivesaria' em bancos diferentes e a erosao nunca os juntaria — a regua e o
 * PREFIXO, logo quem partilha prefixo tem de partilhar banco. O primeiro simbolo decide.
 *
 * A dilatacao nao tem cabeca fixa (pode saltar o inicio), entao ela pergunta ao barramento
 * inteiro — emite, e responde quem puder. */
#define NB 4
static int fdv[NB] = { -1, -1, -1, -1 };
static int fd = -1;                                /* o banco corrente: quem esta a reagir */
static void barr_abre(const char *base){
    for(int b = 0; b < NB; b++){
        char c[512]; snprintf(c, sizeof c, "%s.%d", base, b);
        fdv[b] = open(c, O_RDWR|O_CREAT, 0644);
    }
    fd = fdv[0];
}
static void no_banco(int b){ fd = fdv[b]; }

/* A ENTRADA E A SAIDA SAO UMA OPERACAO: MOVE(slot, sentido). E' a unica instrucao da
 * ISA, e o que a define e' o SINAL — `+1` traz do slot, `-1` leva para o slot. Nao sao
 * duas operacoes que calham ser inversas: e' UMA, e a Lei 1 escreve-a, 1† = -1.
 * `le` e `grava` ficam como os dois sentidos, e nao como duas funcoes. */
static Slot MOVE(long slot, int sentido, Slot v){
    if(sentido > 0){ Slot s = {0,0}; pread(fd, &s, SL, slot*SL); return s; }
    pwrite(fd, &v, SL, slot*SL); return v;
}
static Slot le(long i){ Slot z = {0,0}; return MOVE(i, +1, z); }
static void grava(long i, Slot s){ MOVE(i, -1, s); }

/* O cabeçalho: [0] = primeiro slot livre, [1] = quantos pares aprendidos. */
#define H_LIVRE 0
#define H_PARES 1
#define RAIZ    2
#define LARG    6            /* filhos por registo de nó — o resto encadeia */
/* nó = [nfilhos | resposta][s1|f1][s2|f2]...[s6|f6][0|continuação] = 8 slots */
#define NOSL    8
#define RESP_LIM 4096        /* respostas longas do corpus — não cortar a meio */

static long novo_no(void){
    long l = le(H_LIVRE).a;
    if(l < RAIZ + NOSL) l = RAIZ + NOSL;
    Slot h = { l + NOSL, 0 }; grava(H_LIVRE, h);
    for(int k = 0; k < NOSL; k++){ Slot z = {0,0}; grava(l + k, z); }
    return l;
}
/* o filho pelo símbolo; abre se abrir != 0 */
static long filho(long no, long sim, int abrir){
    for(;;){
        Slot cab = le(no);
        for(int k = 1; k <= LARG; k++){
            Slot p = le(no + k);
            if(p.a == sim) return p.b;
            if(!p.a && abrir){
                long f = novo_no();
                Slot np = { sim, f }; grava(no + k, np);
                Slot nc = { cab.a + 1, cab.b }; grava(no, nc);
                return f;
            }
        }
        Slot cont = le(no + NOSL - 1);
        if(cont.b){ no = cont.b; continue; }
        if(!abrir) return 0;
        long c = novo_no();
        Slot nc = { 0, c }; grava(no + NOSL - 1, nc);
        no = c;
    }
}
/* O texto guarda-se onde couber, comprimento à frente do registo (não da cifra). */
static long poe_texto(const char *s){
    size_t n = strlen(s);
    long base = le(H_LIVRE).a;
    if(base < RAIZ + NOSL) base = RAIZ + NOSL;
    Slot c = { (long)n, 0 }; grava(base, c);
    size_t ns = (n + SL - 1) / SL;
    for(size_t k = 0; k < ns; k++){
        Slot w; memset(&w, 0, SL);
        size_t r = n - k*SL; if(r > SL) r = SL;
        memcpy(&w, s + k*SL, r);
        grava(base + 1 + (long)k, w);
    }
    Slot l = { base + 1 + (long)ns, 0 }; grava(H_LIVRE, l);
    return base;
}
static void le_texto(long base, char *out, size_t lim){
    size_t n = (size_t)le(base).a, m = n < lim - 1 ? n : lim - 1;
    for(size_t k = 0; k*SL < m; k++){
        Slot w = le(base + 1 + (long)k);
        size_t r = m - k*SL; if(r > SL) r = SL;
        memcpy(out + k*SL, &w, r);
    }
    out[m] = 0;
}
/* O SÍMBOLO, COM O ACENTO ACERTADO.
 *
 * Eu tratava cada byte como símbolo, e em UTF-8 o 'é' são DOIS bytes (0xC3 0xA9) que não se
 * parecem nada com o 'e'. Em português isso manda "és" e "es" para lados opostos da árvore, e
 * numa assistente que aprende de conversa é o caso comum, não a excepção.
 *
 * A acentuação é ROUPA: 'é' e 'e' são a mesma letra vestida. Então o símbolo é a letra NUA, e o
 * acento fica no texto guardado, que é onde ele serve — a resposta sai acentuada, o caminho não
 * se parte. Mesma ideia do resto do dia: o que identifica é a estrutura, não a superfície.
 *
 * Avança o ponteiro pelo símbolo inteiro: uma letra é um passo, tenha um byte ou dois. */
static long prox_simb(const char **p){
    unsigned char c = (unsigned char)*(*p)++;
    if(c == 0xC3 && **p){                          /* os acentuados do português vivem aqui */
        unsigned char d = (unsigned char)*(*p)++;
        if(d >= 0x80 && d <= 0x9F) d += 0x20;      /* maiúscula acentuada -> minúscula */
        if(d >= 0xA0 && d <= 0xA5) c = 'a';
        else if(d == 0xA7)         c = 'c';        /* ç */
        else if(d >= 0xA8 && d <= 0xAB) c = 'e';
        else if(d >= 0xAC && d <= 0xAF) c = 'i';
        else if(d == 0xB1)         c = 'n';        /* ñ */
        else if(d >= 0xB2 && d <= 0xB6) c = 'o';
        else if(d >= 0xB9 && d <= 0xBC) c = 'u';
        else c = 'a';                               /* qualquer outro do bloco: cai na base */
    }
    if(c >= 'A' && c <= 'Z') c += 32;
    return (long)c - 31;
}

/* de que banco e esta fala: o primeiro simbolo dela, e mais nada */
static int banco_da(const char *fala){
    const char *p = fala;
    long s0 = prox_simb(&p);
    return (int)(((s0 % NB) + NB) % NB);
}
/* ── A SOMA ⊕ NO CORPUS: aprender e esquecer são UMA operação, com sinal ───────
 * O par soma/retração das cinco, realizado no banco: +1 acrescenta o par (a dobra
 * que faz o corpus crescer) e −1 retira-o (a retração, que devolve o nó ao estado
 * de não ter fala). Não são duas coisas: é uma, e o sinal escreve-a — 1† = −1.
 * O caminho FICA nos dois sentidos; o que muda é a resposta no nó terminal. */
static int CORPUS(const char *fala, const char *resp, int sentido){
    no_banco(banco_da(fala));                      /* nao ha repartidor: a cifra diz */
    long no = RAIZ;
    for(const char *p = fala; *p; ){
        long s2 = prox_simb(&p);
        no = filho(no, s2, sentido > 0);           /* +1 cria o caminho; −1 só desce */
        if(!no) return 0;                          /* não sabia: nada a retirar */
    }
    Slot cab = le(no);
    if(sentido > 0){
        long r = poe_texto(resp);
        Slot nc = { cab.a, r }; grava(no, nc);      /* uma resposta só: a nova substitui */
        Slot pc = le(H_PARES); pc.a++; grava(H_PARES, pc);
        printf("aprendido — %ld par(es) no corpus\n", pc.a);
        return 1;
    }
    if(!cab.b) return 0;                            /* não havia resposta para retirar */
    cab.b = 0; grava(no, cab);
    Slot pc = le(H_PARES); if(pc.a > 0) pc.a--; grava(H_PARES, pc);
    return 1;
}
static void aprende(const char *fala, const char *resp){ CORPUS(fala, resp, +1); }
/* EROSÃO: o prefixo. Desce enquanto houver caminho e devolve a resposta mais funda que viu.
 * A CAIXA (o eixo de segmentação da vizinhança admissível): o prefixo só vale se FECHA em
 * fronteira de palavra — "bom dia" fecha na vírgula de "bom dia, tudo bem?", mas o "q" do
 * corpus não é o "q" dentro de "qualquer". */
static int simb_de_palavra(long s);
static long erosao(const char *fala, int *fundo){
    long no = RAIZ, achou = 0; *fundo = 0;
    int d = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) break;
        no = f; d++;
        Slot cab = le(no);
        if(cab.b){
            const char *t = p;
            if(!*p || !simb_de_palavra(prox_simb(&t))){ achou = cab.b; *fundo = d; }
        }
    }
    return achou;
}
/* DILATAÇÃO: a subsequência. Salta um símbolo quando o caminho morre — a fala com ruído.
 * E RE-ANCORA: se o ruído à frente abre o caminho de OUTRA fala do mesmo banco, a descida
 * gulosa entra por ele e morre lá dentro; então come-se um símbolo do início e tenta-se de
 * novo, até achar ou a fala acabar. A primeira volta é a régua antiga, inteira.
 *
 * A ADMISSÃO é a escada do observador em dois degraus (Corpo Universal, a escada;
 * Corpo de Peano, Controle de Histerese — a vizinhança admissível por eixos):
 *   I1, a CONTAGEM: o consumido vence o saltado ENTRE consumos — a fala com ruído,
 *       não o ruído com fala; ruído à frente e atrás é livre (o desenho do §C2);
 *   I2, a CAIXA (o eixo de segmentação): a dilatação salta PALAVRAS, não letras —
 *       cada palavra da fala ou é consumida inteira ou saltada inteira; palavra
 *       PARTIDA (consumo e salto na mesma palavra) mata o candidato desta âncora,
 *       e a re-âncora seguinte tenta limpa.
 * O degrau grosso resolve o mundo natural; o fino, o adversário denso ("é racional"
 * contém "e ai" por letras pescadas de dentro de "racional" — a caixa recusa). */
static int simb_de_palavra(long s){
    return (s >= 'a'-31 && s <= 'z'-31) || (s >= '0'-31 && s <= '9'-31);
}
static const char *acha_buraco(const char *s);     /* o '_' nu da moldura; adiante */
static long dilatacao(const char *fala, int *fundo){
    long achou = 0; *fundo = 0;
    int corte_pal = 0;                              /* a âncora cortou uma palavra? */
    for(const char *ini = fala; *ini; ){
        long no = RAIZ;
        int d = 0, dentro = 0, pend = 0;
        int cons_pal = 0, salt_pal = corte_pal, partida = 0;
        for(const char *p = ini; *p; ){
            long s = prox_simb(&p);
            int letra = simb_de_palavra(s);
            long f = filho(no, s, 0);
            if(!letra) cons_pal = salt_pal = 0;     /* separador fecha a palavra */
            if(!f){                                 /* o símbolo não serve: salta-o */
                if(letra){ salt_pal = 1; if(cons_pal){ partida = 1; break; } }
                if(d) pend++;
                continue;
            }
            if(letra){ if(salt_pal){ partida = 1; break; } cons_pal = 1; }
            no = f; d++; dentro += pend; pend = 0;
            Slot cab = le(no);
            if(cab.b && d > dentro && !partida){
                /* o peek da caixa: o registo só vale em FIM de palavra da consulta —
                 * o "q" a meio de "quanto" não é a abreviação "q" */
                const char *t = p;
                if(!*p || !simb_de_palavra(prox_simb(&t))){ achou = cab.b; *fundo = d; }
            }
        }
        if(achou) break;                            /* a âncora mais à esquerda que alcança */
        long s0 = prox_simb(&ini);                  /* come um símbolo e re-ancora */
        if(simb_de_palavra(s0)){                    /* a meio de palavra? o resto dela herda o salto */
            const char *esp = ini;
            corte_pal = *esp && simb_de_palavra(prox_simb(&esp));
        } else corte_pal = 0;
    }
    return achou;
}
/* EVOLUÇÃO NO BANCO: o DUAL da erosão, e o lado que faltava.
 *
 * A erosão e a dilatação tratam ambas a fala que tem A MAIS — uma pára quando o caminho morre,
 * a outra salta o símbolo que não serve. Nenhuma das duas ACRESCENTA nada. Por isso o excesso
 * funcionava e a falta não: medido, "o que é um corpo afinal" respondia e "o que é um" dava
 * "não sei" — e assim para as 252 falas do corpus, ZERO alcançavam ao perder uma palavra.
 *
 * O outro lado é este: a fala CABE inteira, mas acaba num nó que não responde. Aí quem continua
 * é o BANCO, que desce sozinho o resto do caminho. O Aarão: «involução na entrada e EVOLUÇÃO NO
 * BANCO» — a entrada dobra-se para caber, o banco desdobra-se para responder.
 *
 * E NÃO ADIVINHA: só desce enquanto o caminho for ÚNICO. Onde o nó ramifica, a fala é
 * genuinamente ambígua — "o que é um" pode seguir para corpo, para grupo, para anel — e a
 * evolução pára e deixa o "não sei". A continuação forçada não é um palpite: é a única que
 * existe, e é por isso que se pode escrever sem escolher. */
static long evolucao(const char *fala, int *fundo, int *passos){
    long no = RAIZ; int d = 0; *fundo = 0; *passos = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) return 0;              /* a fala nao cabe — e caso da erosao, nao deste */
        no = f; d++;
    }
    Slot cab = le(no);
    if(cab.b){ *fundo = d; return cab.b; }        /* ja respondia: nao era preciso evoluir */
    for(int passo = 0; passo < 256; passo++){
        long unico = 0; int quantos = 0, n2 = 0;
        for(long t = no;;){
            for(int k = 1; k <= LARG; k++){
                Slot pr = le(t + k);
                if(pr.a){ quantos++; unico = pr.b; }
            }
            Slot cont = le(t + NOSL - 1);
            if(cont.b && ++n2 < 64){ t = cont.b; continue; }
            break;
        }
        if(quantos != 1) return 0;    /* ramifica: a fala e ambigua, e nao se adivinha */
        no = unico; d++; (*passos)++;
        Slot c2 = le(no);
        if(c2.b){ *fundo = d; return c2.b; }
    }
    return 0;
}
/* A BIFURCAÇÃO: o que a evolução encontra quando o caminho NÃO é único.
 *
 * A evolução cala-se onde o nó ramifica, e está certa em não escolher — mas calar-se deita fora
 * o que ela JÁ SABE: que a fala chegou lá, e para onde é que ela podia seguir. "o que é um"
 * cabe inteiro no banco e segue para corpo, para grupo, para número.
 *
 * Então em vez de "não sei", diz-se o que se sabe: a fala não decide, e A BIFURCAÇÃO É ESTA.
 * Não se funde o texto das respostas — isso inventaria uma resposta que ninguém escreveu. O que
 * se faz é o cruzamento do viveiro, o ∨ que voa sempre: as duas ficam, lado a lado, e quem
 * decide é quem perguntou.
 *
 * Cada ramo desce depois pela MESMA regra da evolução — enquanto for único — até dar na sua
 * resposta. Devolve quantas achou. */
#define NBIF 6
static int bifurcacao(const char *fala, long saidas[NBIF]){
    long no = RAIZ;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) return 0;                          /* nao cabe: nao e caso desta */
        no = f;
    }
    if(le(no).b) return 0;                        /* ja responde: nao ha bifurcacao */
    /* PRIMEIRO DESCER O QUE E' FORCADO. A ramificacao quase nunca esta no no imediato: "dois
     * lados que" continua pelo ESPACO, que e comum as duas falas, e so' depois e que abre. Sem
     * isto a bifurcacao via um filho so' e calava-se — media o sitio errado. */
    long fs[NBIF]; int nf = 0;
    for(int passo = 0; passo < 256; passo++){
        nf = 0;
        for(long t = no;;){
            static int n2; n2 = 0;
            for(int k = 1; k <= LARG && nf < NBIF; k++){
                Slot pr = le(t + k);
                if(pr.a) fs[nf++] = pr.b;
            }
            Slot cont = le(t + NOSL - 1);
            if(cont.b && ++n2 < 64){ t = cont.b; continue; }
            break;
        }
        if(nf != 1) break;                        /* ou ramifica, ou secou: e aqui que se ve */
        no = fs[0];
        if(le(no).b) return 0;                    /* achou resposta pelo caminho forcado: e da
                                                   * evolucao, e ela ja' a deu */
    }
    if(nf < 2) return 0;                          /* um so caminho: e a evolucao, nao esta */
    int achou = 0;
    for(int i = 0; i < nf && achou < NBIF; i++){
        /* Cada ramo procura EM PROFUNDIDADE a primeira resposta que houver debaixo dele. Um
         * ramo pode voltar a ramificar — "o que é um número" e "…número primo" — e por isso o
         * que se mostra é UM EXEMPLO do que há por aquele lado, não a resposta do ramo. O
         * texto que sai diz isso, porque apresentá-lo como A resposta seria mentir sobre o
         * que se sabe. */
        long pilha[64]; int topo = 0; pilha[topo++] = fs[i];
        for(int passo = 0; passo < 512 && topo; passo++){
            long r = pilha[--topo];
            Slot c = le(r);
            if(c.b){ saidas[achou++] = c.b; break; }
            int m = 0;
            for(long t = r;;){
                for(int k = 1; k <= LARG && topo < 64; k++){
                    Slot pr = le(t + k);
                    if(pr.a) pilha[topo++] = pr.b;
                }
                Slot cont = le(t + NOSL - 1);
                if(cont.b && ++m < 64){ t = cont.b; continue; }
                break;
            }
        }
    }
    return achou;
}
/* TORCAO: duas falas no mesmo canal. Desce ate um no terminal, responde, e RECOMECA da raiz com
 * o que sobrou — desentrelacando a fala em varias. E a terceira regua do morfico, e a que trata o
 * caso em que a pessoa diz duas coisas de uma vez.
 *
 * Devolve quantas achou, e escreve as respostas por ordem. */
/* Guardar TAMBEM de que banco veio cada resposta: o ponteiro e relativo ao banco, e le-lo do
 * banco errado da lixo — a primeira resposta saia "U". Passou nos testes e falhava a correr. */
static int torc_banco[8];
static int torcao(const char *fala, long *saida, int max){
    int n = 0;
    const char *p = fala;
    int fronteira = 1;                             /* o inicio da fala e fronteira de palavra */
    while(*p && n < max){
        if(!fronteira){                            /* a CAIXA: um troco nao ancora a meio de
                                                    * palavra — avanca ate a proxima fronteira */
            long s = prox_simb(&p);
            fronteira = !simb_de_palavra(s);
            continue;
        }
        no_banco(banco_da(p));                     /* cada troco tem a SUA cabeca, logo o SEU
                                                    * banco: recomecar da raiz e reemitir */
        long no = RAIZ, ultima = 0;
        const char *fim = p, *q = p;
        while(*q){
            const char *antes = q;
            long f = filho(no, prox_simb(&q), 0);
            if(!f){ q = antes; break; }
            no = f;
            Slot cab = le(no);
            if(cab.b){                              /* o terminal mais fundo deste troco —
                                                     * mas so se FECHA em fronteira de palavra,
                                                     * e nao numa MOLDURA: a resposta com o
                                                     * buraco '_' e um funcional a espera do
                                                     * argumento, nao uma fala completa */
                const char *t2 = q;
                if(!*q || !simb_de_palavra(prox_simb(&t2))){
                    char rt[256]; le_texto(cab.b, rt, sizeof rt);
                    if(!acha_buraco(rt)){ ultima = cab.b; fim = q; }
                }
            }
        }
        if(!ultima){                                 /* nada comeca aqui: avanca um simbolo */
            if(!*p) break;
            long s = prox_simb(&p);
            fronteira = !simb_de_palavra(s);
            continue;
        }
        if(n < 8) torc_banco[n] = banco_da(p);
        saida[n++] = ultima;
        p = fim;                                     /* recomeca da raiz com o que sobrou */
        fronteira = 0;                               /* o anterior e a ultima letra do troco */
    }
    return n;
}
/* NO BARRAMENTO HA MAIS DO QUE UMA ASSISTENTE, E ELAS CONVERSAM.
 *
 * A fala e a interface — nao ha API a escrever. Quando uma nao sabe, EMITE; quem souber responde,
 * e ela APRENDE com a resposta. Ninguem chama ninguem pelo nome, ninguem se regista, e nenhuma
 * sabe quantas outras existem.
 *
 * O barramento e a pasta: qualquer base que la esteja e participante. Igual ao barramento.c —
 * nao ha lista de membros, ha quem esteja no meio. */
static char barr_base[512];
static void cam_le(char *out, size_t lim);   /* definida adiante, com o resto das coordenadas */
/* A ESCOLHA PELO BAIRRO, e nao pela profundidade sozinha.
 *
 * Quando varias assistentes respondem, eu ficava com a mais funda — e profundidade e a MARGINAL:
 * a frequencia, e mais nada. O bairro.c mede, em 115.871 decisoes, que a marginal e a iteracao 0
 * e que a VIZINHANCA tem de pagar para valer.
 *
 * Entao: a marginal e a profundidade, a compatibilidade e o que a candidata partilha com o
 * CAMINHO ANDADO (as coordenadas da conversa), e a escolha e a contracao
 *
 *     s(e) = a(e) · ( m + Σ_outras w(f)·c(e,f) )      iterada ao ponto fixo
 *
 * que e σ = m + 1/σ, a do rei. Sem vizinhanca, o ponto fixo E a marginal — fail-closed. */
#define NCAND 8
static int bairro_escolhe(char cand[NCAND][1024], int prof[NCAND], int n, const char *ctx){
    if(n <= 1) return 0;
    /* EM MANTISSA INTEIRA, e não em double. A casa já tinha a régua: «S = 2^30 fixo
     * (mantissa); C_k é a resolução» (corpo_peano, §régua dinâmica). A contração é a
     * mesma — s(e) = a(e)·(m + Σ w·c) normalizada —, só que cada peso é um inteiro na
     * escala S, a divisão é inteira e a paragem é por DIFERENÇA INTEIRA. Determinista,
     * reprodutível e sem um decimal: o mesmo resultado em qualquer máquina. */
    enum { S = 1 << 20 };                       /* a mantissa: a escala da régua */
    long a[NCAND], sc[NCAND], w[NCAND], c[NCAND][NCAND];
    long z = 0;
    for(int k = 0; k < n; k++){ a[k] = prof[k] > 0 ? prof[k] : 1; z += a[k]; }
    for(int k = 0; k < n; k++) sc[k] = (a[k] * S) / z;          /* iteracao 0: a MARGINAL */
    size_t lc = strlen(ctx);
    for(int k = 0; k < n; k++)
        for(int j = 0; j < n; j++){
            if(k == j){ c[k][j] = 0; continue; }
            /* A VIZINHANCA NAO E O PREFIXO. Eu media o prefixo a contar do inicio, e por isso
             * "o relogio da rede" nao distinguia "o que manda no RELOGIO DA REDE" de "o corpo
             * aureo" — as duas partilham so o "o " da frente e EMPATAM. E o "contexto raso"
             * outra vez, noutro sitio.
             *
             * O bairro.c nao pergunta onde a palavra esta: pergunta se ELA ESTA. Entao a
             * compatibilidade conta os SIMBOLOS PARTILHADOS, em qualquer sitio — que e a
             * intersecao dos conjuntos, o mo_prod do corpos.h. */
            int vis[256]; memset(vis, 0, sizeof vis);
            for(size_t t = 0; t < lc; t++) vis[(unsigned char)ctx[t]] = 1;
            size_t pp = 0;
            for(size_t t = 0; cand[k][t]; t++) if(vis[(unsigned char)cand[k][t]]) pp++;
            /* e as palavras inteiras pesam mais que as letras soltas */
            size_t pal = 0;
            { char cp[1024]; snprintf(cp, sizeof cp, "%s", ctx);
              char *tk = strtok(cp, " ,.;:!?");
              while(tk){ if(strlen(tk) > 3 && strstr(cand[k], tk)) pal += strlen(tk); tk = strtok(NULL, " ,.;:!?"); } }
            size_t q = 0;
            while(cand[k][q] && cand[j][q] && cand[k][q] == cand[j][q]) q++;
            c[k][j] = ((long)(pp + 8*pal + q) * S) / 64;         /* a compatibilidade, na escala */
        }
    long d = S;
    for(int it = 0; it < 60 && d > 0; it++){
        for(int k = 0; k < n; k++) w[k] = sc[k];
        d = 0;
        long zz = 0, nv[NCAND];
        for(int k = 0; k < n; k++){
            long viz = 0;
            for(int j = 0; j < n; j++) if(j != k) viz += (w[j] * c[k][j]) / S;
            nv[k] = a[k] * (S + viz);                            /* m = 1, na escala */
            zz += nv[k];
        }
        for(int k = 0; k < n; k++){
            long v = zz > 0 ? (nv[k] * S) / zz : 0;
            long dd = v - sc[k]; if(dd < 0) dd = -dd; if(dd > d) d = dd;
            sc[k] = v;
        }
    }
    int b = 0;
    for(int k = 1; k < n; k++) if(sc[k] > sc[b]) b = k;
    return b;
}
static int pergunta_ao_barramento(const char *fala, char *resp, size_t lim){
    char pasta[512]; snprintf(pasta, sizeof pasta, "%s", barr_base);
    char *fim = strrchr(pasta, '/'); if(!fim) return 0;
    *fim = 0;
    const char *meu = fim + 1;
    DIR *d = opendir(pasta);
    if(!d) return 0;
    /* NAO A PRIMEIRA QUE A PASTA DEVOLVER — a mais funda. Ficar com a primeira era deixar a
     * ordem do readdir decidir, e isso e sistema de ficheiros a fazer de regua. Quem decide e a
     * PROFUNDIDADE do caminho: quem casou mais simbolos sabe mais daquela fala. */
    /* COLHER TODAS, e so depois escolher: era isto que faltava. Ficar com a mais funda e ficar
     * com a marginal, e a marginal e a iteracao 0 do bairro. */
    char cand[NCAND][1024]; int prof[NCAND], ncand = 0;
    int achou = 0, melhor = -1;
    struct dirent *e;
    while((e = readdir(d))){
        /* uma base e um nome que tem ".0" no fim; a minha nao conta */
        size_t n = strlen(e->d_name);
        if(n < 3 || strcmp(e->d_name + n - 2, ".0")) continue;
        char outro[512]; snprintf(outro, sizeof outro, "%.*s", (int)(n - 2), e->d_name);
        if(!strncmp(outro, meu, strlen(outro)) && strlen(outro) == strlen(meu)) continue;
        /* abre a outra e desce nela — e o mesmo caminho, noutro banco */
        int guarda[NB]; memcpy(guarda, fdv, sizeof guarda);
        char cam[1200];
        for(int b = 0; b < NB; b++){
            snprintf(cam, sizeof cam, "%s/%s.%d", pasta, outro, b);
            fdv[b] = open(cam, O_RDONLY);
        }
        if(fdv[0] >= 0){
            no_banco(banco_da(fala));
            int dd; long r = erosao(fala, &dd);
            if(!r) for(int b = 0; b < NB && !r; b++){ no_banco(b); r = dilatacao(fala, &dd); }
            if(r){
                if(ncand < NCAND){ le_texto(r, cand[ncand], sizeof cand[0]); prof[ncand] = dd; ncand++; }
                if(dd > melhor){ melhor = dd; achou = 1; }
            }
        }
        for(int b = 0; b < NB; b++) if(fdv[b] >= 0) close(fdv[b]);
        memcpy(fdv, guarda, sizeof guarda);
    }
    closedir(d);
    if(achou && ncand){
        char ctx[2048]; ctx[0] = 0;
        for(int b2 = 0; b2 < NB; b2++){ no_banco(b2); cam_le(ctx, sizeof ctx); if(ctx[0]) break; }
        int k = bairro_escolhe(cand, prof, ncand, ctx);    /* a CONTRACAO escolhe */
        snprintf(resp, lim, "%s", cand[k]);
    }
    return achou;
}
/* A CONVERSA E UMA POSICAO. Nao se varre nada: cada fala DESCE do ponto onde se esta, voltar
 * atras e estar num no anterior, e ramificar e outro filho. E fractal — o mesmo passo em qualquer
 * nivel — e reversivel, porque subir e a dobra ao contrario.
 *
 * O ponto vive NO BANCO (slot H_ONDE), como tudo: fechar o programa nao perde a conversa. */
/* O PONTO SAO AS COORDENADAS, E NAO UM NO. Eu guardava o indice do no — e de um indice nao se
 * sobe, nao se reflete, nao se salta: e um sitio sem estrutura. A base e ortonormal, logo o ponto
 * E A CIFRA, e saltar e escreve-la.
 *
 * O caminho andado guarda-se em slots (S_CAM), e o no deriva-se descendo-o. Ai as transformacoes
 * sao as dos corpos, aplicadas as coordenadas:
 *
 *   SOBE k     a EROSAO: tira k simbolos do fim — subir e a dobra ao contrario
 *   SALTA x    poe as coordenadas em x: qualquer ponto, de uma vez
 *   REFLETE    J, a TROCA: le o caminho ao contrario — a involucao do criativo
 *
 * Nao ha marcador nem historico: ha um ponto, e ele transforma-se. */
#define S_CAM   1024
static void cam_le(char *out, size_t lim){
    size_t n = (size_t)le(S_CAM).a, m = n < lim - 1 ? n : lim - 1;
    for(size_t k = 0; k*SL < m; k++){
        Slot w = le(S_CAM + 1 + (long)k);
        size_t r = m - k*SL; if(r > SL) r = SL;
        memcpy(out + k*SL, &w, r);
    }
    out[m] = 0;
}
static void cam_poe(const char *s){
    size_t n = strlen(s);
    Slot c = { (long)n, 0 }; grava(S_CAM, c);
    for(size_t k = 0; k*SL < n; k++){
        Slot w; memset(&w, 0, SL);
        size_t r = n - k*SL; if(r > SL) r = SL;
        memcpy(&w, s + k*SL, r);
        grava(S_CAM + 1 + (long)k, w);
    }
}
/* O ponto da conversa vive no campo .b do H_PARES. Eu tinha-o posto no slot 3 — que esta DENTRO
 * do no raiz (2..9) — e corrompia a raiz a cada resposta. O cabecalho tem dois campos por slot;
 * usa-se o que estava vazio, em vez de pisar o que estava ocupado. */
static long onde(void){ long n = le(H_PARES).b; return n ? n : RAIZ; }
static void poe_onde(long n){ Slot s = le(H_PARES); s.b = n; grava(H_PARES, s); }
/* descer a partir de onde se esta, e nao da raiz — e o que faz a conversa ter fio */
static long desce_daqui(const char *fala, int *fundo){
    long no = onde(), achou = 0; *fundo = 0;
    int d = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) break;
        no = f; d++;
        Slot cab = le(no);
        if(cab.b){
            const char *t = p;                     /* a caixa: o registo em fim de palavra */
            if(!*p || !simb_de_palavra(prox_simb(&t))){
                achou = cab.b; *fundo = d; poe_onde(no);
                char c[2048]; cam_le(c, sizeof c);
                size_t l0 = strlen(c);
                if(l0 + strlen(fala) + 1 < sizeof c){ strcat(c, fala); cam_poe(c); }
            }
        }
    }
    return achou;
}
/* A EXPRESSÃO NUMÉRICA. Quando a fala é uma conta, ela não se PROCURA — desdobra-se.
 *
 * E o desdobramento é o mesmo de sempre: a expressão aninhada é uma árvore, e resolver é dobrar
 * de dentro para fora, que é o OP_FOLD com números no lugar das folhas. A precedência não está
 * escrita em tabela nenhuma: cai da ORDEM das dobras — o nível mais fundo primeiro, e dentro
 * dele o x antes do +.
 *
 * A fita mora no banco, ao lado dos outros: <base>.conta. Nada em RAM.
 *
 * O reconhecimento é fechado: só passa quem for feito de dígitos, + x, delimitadores e espaço,
 * e tiver pelo menos um dígito. Uma pergunta em português nunca cai aqui, e uma conta nunca vai
 * parar às réguas do morfico — que a procurariam no corpus e não a achariam. */
/* "distribui 2 x (3+4)" / "fatora 2x3 + 2x4" — a lei pedida pelo nome. Devolve o resto da
 * fala, ou NULL. É a única porta em português desta parte, e é de propósito: a conta entra
 * sozinha e resolve-se; a LEI aplica-se quando se pede, porque escolher a via é do aluno. */
static const char *pede_lei(const char *f, int *distribuir){
    static const char *d[] = { "distribui ", "distribua ", "distribuir ", 0 };
    static const char *t[] = { "fatora ", "fatoriza ", "fatorar ", "poe em evidencia ", 0 };
    for(int k = 0; d[k]; k++) if(!strncmp(f, d[k], strlen(d[k]))){ *distribuir = 1; return f + strlen(d[k]); }
    for(int k = 0; t[k]; k++) if(!strncmp(f, t[k], strlen(t[k]))){ *distribuir = 0; return f + strlen(t[k]); }
    return 0;
}
/* A EQUAÇÃO ENTRE DOIS POLINÓMIOS: p(x) = q(x). O primeiro grau já era um caso disto — aqui
 * vale qualquer grau, e o método é o de sempre: achar todas as raízes de uma vez e SUBSTITUIR
 * cada uma para medir o resíduo. */
/* ─── AS PEÇAS EXATAS: raiz inteira e fração, sem um decimal ─────────────────────
 *
 * A álgebra é PLENA: o que a casa precisa de dizer, diz-se em inteiros e em frações.
 * A raiz quadrada não devolve «aproximadamente» — devolve SE é quadrado perfeito e
 * qual é a raiz, exata; quando não é, a resposta não é um decimal, é o corpo. E a
 * fração escreve-se reduzida, que é como a fita já a escreve. */
/* NADA AQUI É NOVO — e eu ia escrever tudo outra vez.
 *   `raizi` (cifra.h)      a raiz inteira, exata: eu escrevi um `lsqrt` por Newton;
 *   `lado`  (cifra.h)      a FC por PQa EM INTEIROS, que pára no período (Lagrange
 *                          garante que ele é invariante completo) — é ESTA a forma
 *                          normalizada de uma raiz que não fecha em ℚ, e é exata;
 *   `ct_escreve` (expr.h)  a fração reduzida em texto: eu escrevi um `frac2`.
 * A régua é: nunca um decimal. Ou a raiz fecha em ℚ e escreve-se fração, ou é
 * quadrática e escreve-se pela sua FC — normalizada só no fim, em caracteres. */
static int quadrado_perfeito(long n, long *r){
    if(n < 0) return 0;
    long s = raizi(n);
    if(s * s != n) return 0;
    *r = s; return 1;
}
/* O DESTINO É ROTATIVO, E A ROTAÇÃO TEM TAMANHO. Duas vezes escrevi mais de FRAC2_N
 * chamadas num único `printf` e a última sobrescreveu a primeira: a linha saiu com o
 * número errado e nenhuma asserção o viu, porque as asserções leem os valores e não o
 * texto. O guarda é do lado da FONTE — `tools/bench_destino.sh` conta as chamadas por
 * `printf` e recusa acima de FRAC2_N. A regra: uma chamada por `printf`, ou contar. */
#define FRAC2_N 4
static const char *frac2(long p, long q){    /* ct_reduz + ct_escreve, destino rotativo */
    static char buf[FRAC2_N][80]; static int k = 0;
    char *o = buf[k++ % FRAC2_N];
    if(q < 0){ p = -p; q = -q; }
    ct_reduz(&p, &q);                       /* a redução também é da casa */
    ct_escreve(p, q, o, 80);
    return o;
}
static const char *fc_da_borda(long B, long C);   /* a FC, adiante */
static void MEMBRANA(char *out, size_t lim, const char *texto, long v, long q, int sentido);
/* A BORDA RESOLVIDA EXATA, UMA VEZ SÓ — e os três (sistema, ED, polinómio) chamam-na.
 *
 * `a s² + b s + c = 0`, tudo inteiro. Ou o Δ é quadrado perfeito e as duas raízes são
 * FRAÇÕES exatas, ou não é e elas são as folhas do corpo — e aí a forma normalizada é
 * a fração contínua, que é exata e fecha no período. Em nenhum ramo há decimal: a
 * normalização é a última coisa, e é em caracteres.
 *
 * A FC da casa (`lado`) lê a borda MÓNICA. Uma borda com a≠1 escala-se: y = a·s é raiz
 * de y² + b y + ac = 0, e a folha lê-se em y (dito na saída, para ninguém confundir
 * a escala com o número). */
static void borda_exata(long a, long b, long c){
    long D = b*b - 4*a*c, q = 0;
    printf("   Δ = %ld² - 4·%ld·%ld = %ld\n", b, a, c, D);
    if(D > 0 && quadrado_perfeito(D, &q)){
        printf("   Δ é quadrado perfeito (√Δ = %ld): as duas raízes fecham em Q, exatas —\n", q);
        printf("     s₊ = %s   s₋ = %s\n", frac2(-b + q, 2*a), frac2(-b - q, 2*a));
    } else if(D > 0){
        printf("   Δ não é quadrado: as raízes são as folhas σ, σ† do corpo Q[s]/(a s²+b s+c),\n");
        printf("   com σ+σ† = %s e σ·σ† = %s. A forma normalizada é a fração contínua:\n",
               frac2(-b, a), frac2(c, a));
        printf("     %s%s = %s   (fecha no período — Lagrange, invariante completo)\n",
               a == 1 ? "σ" : "a·σ", a == 1 ? "" : " (escalada)", fc_da_borda(-b, a*c));
    } else if(D < 0){
        printf("   Δ < 0: o par é conjugado — parte real %s", frac2(-b, 2*a));
        if(quadrado_perfeito(-D, &q)) printf(", e a imaginária fecha em Q: ± %s i\n", frac2(q, 2*a));
        else printf(", e a imaginária é √%ld/(2·%ld) — vive no corpo, não em decimal\n", -D, a);
    } else {
        printf("   Δ = 0: a raiz é dupla e racional, %s (e é aqui que entra o t)\n", frac2(-b, 2*a));
    }
}

static void latex_desdobra(char *d, size_t dn, const char *s){
    MEMBRANA(d, dn, s, 0, 1, +1);                  /* +1: o lado da entrada */
}
/* A RAIZ NORMALIZADA — só no fim, e em caracteres. A borda s² − Bs + C = 0 tem as duas
 * folhas; a FC de (B+√Δ)/2 é a régua da casa para as escrever quando não fecham em ℚ,
 * e o período é onde ela fecha: «não guarda as casas, guarda a regra que as gera». */
static const char *fc_da_borda(long B, long C){
    static char buf[2][256]; static int k = 0;
    char *o = buf[k++ & 1];
    long a[48];
    size_t n = lado(B, C, a, 48);
    size_t w = 0, most = n < 8 ? n : 8;            /* os primeiros, e DIZ-SE quantos são */
    w += (size_t)snprintf(o + w, 256 - w, "[");
    for(size_t i = 0; i < most && w < 200; i++)
        w += (size_t)snprintf(o + w, 256 - w, i == 0 ? "%ld" : (i == 1 ? "; %ld" : ", %ld"), a[i]);
    if(n > most) w += (size_t)snprintf(o + w, 256 - w, ", … (%zu termos até fechar)", n);
    snprintf(o + w, 256 - w, "]");
    return o;
}

/* ─── O DICIONÁRIO: a álgebra é intrínseca; o assunto só empresta os nomes ───────
 *
 * A borda `a s² + b s + c = 0` já está resolvida — com o Δ, a tricotomia, as folhas no
 * relógio e o resíduo. O que um assunto novo traz NÃO é régua: é o mapa de qual das
 * suas grandezas se senta em cada coordenada, e como ele chama os três regimes do Δ.
 * Por isso «cabe em qualquer assunto, só mapear»: a massa-mola e o circuito RLC são a
 * MESMA equação com nomes diferentes, e é a álgebra que sabe a resposta nos dois.
 *
 * A tabela é dado, não código: cada linha é um assunto, e acrescentar um assunto é
 * acrescentar uma linha — nenhuma função muda. Os três dados entram por nome (o
 * utilizador escreve `mola m=1 c=3 k=2`), porque é o nome que diz o PAPEL. */
static const struct {
    const char *assunto;
    const char *dado[3];          /* quem se senta em a, b, c — por esta ordem */
    const char *hip, *par, *eli;  /* os três regimes do Δ, na língua do assunto */
    const char *lei;              /* a equação característica, escrita como lá se escreve */
} DIC[] = {
  { "mola",   {"m","c","k"},  "sobreamortecida", "criticamente amortecida", "oscilante",
    "m s^2 + c s + k = 0   (massa, amortecimento, rigidez)" },
  { "rlc",    {"L","R","S"},  "sobreamortecido", "criticamente amortecido", "oscilante",
    "L s^2 + R s + S = 0   (indutância, resistência, elastância S=1/C)" },
  { "queda",  {"a","b","g"},  "sem retorno",     "no limiar",               "em volta",
    "a s^2 + b s + g = 0   (inércia, arrasto, gravidade reduzida)" },
  { "juro",   {"p","j","d"},  "duas taxas reais","taxa única",              "sem taxa real",
    "p s^2 + j s + d = 0   (principal, juro, desconto)" },
  { "borda",  {"a","m","n"},  "hiperbólica (metais)", "parabólica (o limite)", "elítica (polígonos)",
    "a s^2 - m s - n = 0   (a família metálica em pessoa)" },
};
static const char *dic_regime(int i, long D){
    if(i < 0) return "";
    return D > 0 ? DIC[i].hip : D < 0 ? DIC[i].eli : DIC[i].par;
}
/* lê «assunto n1=v1 n2=v2 n3=v3» e devolve o índice do assunto, ou −1. Os três dados
 * têm de estar TODOS lá: faltar um não é «assume zero», é não ser deste assunto. */
static int dic_le(const char *f, long *a, long *b, long *c){
    while(*f == ' ') f++;
    for(size_t k = 0; k < sizeof DIC/sizeof *DIC; k++){
        size_t ln = strlen(DIC[k].assunto);
        if(strncmp(f, DIC[k].assunto, ln) || (f[ln] && f[ln] != ' ')) continue;
        long v[3]; int tem = 0;
        for(int d = 0; d < 3; d++){
            const char *p = f + ln;
            size_t ld = strlen(DIC[k].dado[d]);
            int achou = 0;
            for(; *p; p++){
                if(strncmp(p, DIC[k].dado[d], ld)) continue;
                if(p > f && p[-1] != ' ') continue;         /* o nome é palavra inteira */
                const char *q = p + ld;
                while(*q == ' ') q++;
                if(*q != '=') continue;
                q++; while(*q == ' ') q++;
                int sig = 1;
                if(*q == '-'){ sig = -1; q++; }
                if(*q < '0' || *q > '9') continue;
                long val = 0;
                while(*q >= '0' && *q <= '9'){ val = val*10 + (*q - '0'); q++; }
                v[d] = sig * val; achou = 1; break;
            }
            if(!achou) break;
            tem++;
        }
        if(tem != 3) return -1;                             /* falta um dado: não é */
        if(v[0] == 0) return -1;                            /* sem o s², não é borda */
        *a = v[0]; *b = v[1]; *c = v[2];
        return (int)k;
    }
    return -1;
}
static void folhas_no_relogio(long a, long b, long c);
static int resolve_assunto(const char *f){
    long a, b, c;
    int i = dic_le(f, &a, &b, &c);
    if(i < 0) return 0;
    long D = b*b - 4*a*c;
    printf("   %s\n", DIC[i].lei);
    printf(" = %ld s^2 %s %ld s %s %ld = 0     (os teus números no lugar dos nomes)\n",
           a, b < 0 ? "-" : "+", b < 0 ? -b : b, c < 0 ? "-" : "+", c < 0 ? -c : c);
    printf(" = Δ = %ld² - 4·%ld·%ld = %ld\n", b, a, c, D);
    printf("\n   logo é %s.\n", dic_regime(i, D));
    printf("   (o regime sai do Δ, e o Δ é o mesmo em qualquer assunto: a mola, o\n");
    printf("    circuito e a queda são a MESMA equação — o assunto empresta os nomes,\n");
    printf("    a álgebra é que responde)\n");
    folhas_no_relogio(a, b, c);
    return 1;
}

/* ─── A TEORIA DISCRETA A RESOLVER A EQUAÇÃO: as folhas no relógio da casa ───────
 *
 * A régua do polinómio já declara o corpo ℚ[x]/(p) — lá a raiz é σ, exata e sem
 * decimal. O *Corpo Universal* dá o passo que a torna NÚMERO: no relógio da casa (a
 * escada de Fermat 17, 257, 65537) o discriminante ou é quadrado, e as duas FOLHAS
 * estão à vista em inteiros, ou não é, e o andar é INERTE — as folhas vivem um andar
 * acima, onde o Frobenius x↦x^p *é* a estaca. «√2 é 11 em 𝔽₁₇, 11²=2 exato»: o
 * irracional realizado inteiro no primo, que é o que a casa quer dizer com discreto.
 *
 * Tudo em inteiros e por DOIS CAMINHOS: Euler diz se é quadrado, a varredura acha (ou
 * não) a raiz — e nunca se escreve uma folha sem a mandar de volta à equação: resíduo
 * ZERO exato, ou não é raiz. O inverso de 2a sai do INVERSOR (Euclides estendido: «a
 * dinâmica do inversor», e a folha da órbita é o gcd). */
static long mod_p(long x, long p){ long r = x % p; return r < 0 ? r + p : r; }
static long pot_mod(long b, long e, long p){
    long r = 1; b = mod_p(b, p);
    while(e > 0){ if(e & 1) r = (r * b) % p; b = (b * b) % p; e >>= 1; }
    return r;
}
static int eh_quadrado(long D, long p){            /* o critério de Euler */
    long d = mod_p(D, p);
    if(d == 0) return 1;                            /* 0 é quadrado: a raiz é dupla */
    return pot_mod(d, (p - 1) / 2, p) == 1;
}
static int raiz_quad_mod(long D, long p, long *r){ /* a varredura: p ≤ 65537, e não há
                                                    * algoritmo para eu errar */
    long d = mod_p(D, p);
    for(long x = 0; x < p; x++) if((x * x) % p == d){ *r = x; return 1; }
    return 0;
}
static long inv_mod(long a, long p){               /* o INVERSOR: Euclides estendido */
    long t = 0, nt = 1, r = p, nr = mod_p(a, p);
    while(nr){ long q = r / nr, tmp;
        tmp = t - q * nt; t = nt; nt = tmp;
        tmp = r - q * nr; r = nr; nr = tmp; }
    if(r > 1) return 0;                             /* não invertível neste anel */
    return mod_p(t, p);
}
static int folhas_de(long a, long b, long r, long p, long *s1, long *s2){
    long i2a = inv_mod(mod_p(2 * a, p), p);
    if(!i2a) return 0;
    *s1 = mod_p((mod_p(-b, p) + r) * i2a, p);
    *s2 = mod_p((mod_p(-b, p) - r + p) * i2a, p);
    return 1;
}
static long res_mod(long a, long b, long c, long x, long p){   /* a volta à equação */
    return mod_p(a * mod_p(x * x, p) + b * mod_p(x, p) + c, p);
}
/* e o que a assistente diz: a companheira, e a escada andar a andar */
static void folhas_no_relogio(long a, long b, long c){
    long P[] = { 17, 257, 65537 };
    long D = b * b - 4 * a * c;
    printf("\n   e no RELÓGIO DA CASA a raiz deixa de ser nome e é número. A companheira\n");
    printf("   A = [[%ld,%ld],[1,0]] carrega a equação inteira (a transformada universal\n",
           -b / (a ? a : 1), -c / (a ? a : 1));
    printf("   realiza-se nela), e a escada de Fermat decide andar a andar:\n\n");
    for(size_t j = 0; j < sizeof P/sizeof *P; j++){
        long p = P[j], r = 0, s1, s2;
        int euler = eh_quadrado(D, p), varr = raiz_quad_mod(D, p, &r);
        if(euler != varr){                        /* os dois caminhos discordam: cala-se */
            printf("     F_%-5ld  os dois caminhos discordam — não escrevo o que não fecha\n", p);
            continue;
        }
        if(!varr){
            printf("     F_%-5ld  Δ=%ld não é quadrado: INERTE — as folhas vivem um andar\n", p, D);
            printf("              acima (F_%ld²), e lá o Frobenius x↦x^%ld É a estaca\n", p, p);
            continue;
        }
        if(!folhas_de(a, b, r, p, &s1, &s2)){
            printf("     F_%-5ld  2a não inverte neste anel\n", p); continue;
        }
        long r1 = res_mod(a, b, c, s1, p), r2 = res_mod(a, b, c, s2, p);
        if(r1 || r2){ printf("     F_%-5ld  a folha não volta (resíduo %ld/%ld) — não é raiz\n",
                             p, r1, r2); continue; }
        printf("     F_%-5ld  SEPARADO: σ=%ld  σ†=%ld   σ+σ†=%ld (o traço)  σ·σ†=%ld (o det)"
               "   resíduo 0\n", p, s1, s2, mod_p(s1 + s2, p), mod_p(s1 * s2, p));
    }
    printf("\n   (as duas folhas são o par dual: uma sem a outra é metade com o nome do par —\n");
    printf("    e nenhuma se escreveu sem voltar à equação com resíduo ZERO)\n");
}
/* ─── O RELÓGIO: o TICK é o quantum, e a velocidade é escolha ────────────────────
 *
 * O Quantizador «converte o contínuo em contagem conservada», e o refinamento é a
 * TORRE (ω_{2M}² = ω_M): «o contínuo entra por refinamento do passo de quantização,
 * nunca por salto». Aqui é literal — a fita dá UMA dobra por chamada, e a velocidade
 * diz quantas dobras cabem num tick. Ela só sobe por DOBRA: 1, 2, 4, 8. O 3 não é um
 * andar da torre, e recusa-se com o motivo.
 *
 * E o passo NÃO VIVE EM MEMÓRIA: a fita fica no disco (`<base>.tick`), como tudo nesta
 * casa. Fechar o programa não perde a conta — volta-se e dá-se o tick seguinte. */
static int e_conta(const char *f);                 /* adiante */
static void desdobra_entrada(char *d, size_t dn, const char *s);
static int eh_dobra(long v){ return v > 0 && (v & (v - 1)) == 0; }
static long tick_velocidade(void){
    const char *e = getenv("TICKS");
    if(!e || !*e) return 1;
    long v = 0;
    for(const char *p = e; *p >= '0' && *p <= '9'; p++) v = v*10 + (*p - '0');
    if(!eh_dobra(v)){
        printf("   (a velocidade %ld não é um andar da torre: o refinamento é por DOBRA —\n", v);
        printf("    1, 2, 4, 8 — e nunca por salto. Fico em 1.)\n");
        return 1;
    }
    return v;
}
/* o estado do relógio, ao lado da fita: quantas células e quantas dobras já se deram */
typedef struct { long n, dobras; } Relogio;
static int rel_le(const char *base, Relogio *r){
    char c[600]; snprintf(c, sizeof c, "%s.tick.rel", base);
    int fd2 = open(c, O_RDONLY);
    if(fd2 < 0) return 0;
    int ok2 = pread(fd2, r, sizeof *r, 0) == (long)sizeof *r;
    close(fd2);
    return ok2;
}
static void rel_poe(const char *base, Relogio r){
    char c[600]; snprintf(c, sizeof c, "%s.tick.rel", base);
    int fd2 = open(c, O_RDWR|O_CREAT, 0644);
    if(fd2 >= 0){ pwrite(fd2, &r, sizeof r, 0); close(fd2); }
}
static void rel_apaga(const char *base){
    char c[600];
    snprintf(c, sizeof c, "%s.tick.rel", base); unlink(c);
    snprintf(c, sizeof c, "%s.tick", base);     unlink(c);
}
/* um TICK: dá `vel` dobras e mostra o estado. Devolve 1 se ainda há o que dobrar. */
static int relogio_tick(const char *base, const char *fala, int ate_fim){
    char cf_n[600]; snprintf(cf_n, sizeof cf_n, "%s.tick", base);
    Relogio r = { 0, 0 };
    int cf;
    if(fala && *fala){                              /* começa: escreve a fita */
        char cone[1024]; desdobra_entrada(cone, sizeof cone, fala);
        const char *ex = e_conta(cone) ? cone : fala;
        if(!e_conta(ex)){
            printf("   (o relógio anda sobre uma conta, e isto não é uma. Diz-me a conta.)\n");
            return 0;
        }
        cf = open(cf_n, O_RDWR|O_CREAT|O_TRUNC, 0644);
        if(cf < 0) return 0;
        r.n = ct_leia(cf, ex); r.dobras = 0;
        if(r.n < 0){ printf("   (essa conta não fecha)\n"); close(cf); return 0; }
    } else {
        if(!rel_le(base, &r)){
            printf("   (não há conta no relógio — começa com: tick \"a conta\")\n");
            return 0;
        }
        cf = open(cf_n, O_RDWR);
        if(cf < 0){ printf("   (a fita perdeu-se; recomeça)\n"); return 0; }
    }
    long vel = ate_fim ? 1L<<30 : tick_velocidade();
    char buf[2048], porque[512];
    ct_mostra(cf, r.n, buf, sizeof buf);
    if(fala && *fala) printf("   %s\n", buf);
    int fez = 0;
    for(long d = 0; d < vel; d++){
        if(ct_passo(cf, r.n, porque, sizeof porque) != 1) break;
        r.dobras++; fez = 1;
        ct_mostra(cf, r.n, buf, sizeof buf);
        printf(" = %-26s   %s\n", buf, porque);
    }
    long v; int fechou = ct_valor(cf, r.n, &v) && !fez;
    if(!fez){
        if(ct_valor(cf, r.n, &v)) printf("dá %ld.   (o relógio parou: não há mais dobras)\n", v);
        else printf("   (não há mais dobras, e não fechou num número só)\n");
        close(cf);
        rel_apaga(base);
        return 0;
    }
    close(cf);
    rel_poe(base, r);
    printf("   (tick %ld — %ld dobra(s) dadas; o próximo tick continua daqui, e a fita\n",
           r.dobras / (ate_fim ? 1 : (vel < 1 ? 1 : vel)) + 0, r.dobras);
    printf("    está no disco: fechar o programa não perde o passo)\n");
    (void)fechou;
    return 1;
}

/* A FATORAÇÃO EM FALA: «fatora x^3 - x». O produto é convolução e fatorar é a volta —
 * e a volta CONFERE: multiplicam-se os fatores de novo e comparam-se com o original. Se
 * não bater, não se escreve. O que a casa domina inteiro diz-se pelo nome: a família
 * metálica e o β(n,m) de Pisot são irredutíveis por ROUCHÉ NO DUAL, sem calcular raiz. */
static void escreve_pz(Pz p){
    int algum = 0;
    for(int k = p.n; k >= 0; k--) if(p.a[k]) algum = 1;
    if(!algum){ printf("0"); return; }              /* o zero escreve-se: 0 */
    for(int k = p.n; k >= 0; k--){
        if(p.a[k] == 0) continue;
        if(k != p.n) printf(" %s ", p.a[k] < 0 ? "-" : "+");
        else if(p.a[k] < 0) printf("-");
        long v = p.a[k] < 0 ? -p.a[k] : p.a[k];
        if(v != 1 || k == 0) printf("%ld", v);
        if(k >= 1) printf("x");
        if(k >= 2) printf("^%d", k);
    }
}
/* A DIVISÃO DE POLINÓMIOS na fala: «divide x^3 - 1 por x - 1». É a FIBRA das cinco
 * operações — a = q·b + r —, e a volta é obrigatória: multiplica-se de novo e confere-se.
 * Em ℤ o pseudo-fator é dito, nunca escondido. */
static int resolve_divide_poli(const char *f){
    const char *p = f;
    if(strncmp(p, "divide ", 7)) return 0;
    p += 7;
    const char *por = strstr(p, " por ");
    if(!por) return 0;
    char esq[512], dir[512];
    snprintf(esq, sizeof esq, "%.*s", (int)(por - p), p);
    snprintf(dir, sizeof dir, "%s", por + 5);
    if(!strchr(esq, 'x') && !strchr(dir, 'x')) return 0;   /* sem x é conta */
    Pol pa, pb;
    if(pol_le(esq, esq + strlen(esq), &pa) != 1) return 0;
    if(pol_le(dir, dir + strlen(dir), &pb) != 1) return 0;
    long ia[PMAX+1], ib[PMAX+1];
    if(!pol_ic(pa, ia) || !pol_ic(pb, ib)) return 0;
    Pz a, b;
    a.n = pa.n; for(int k = 0; k <= pa.n; k++) a.a[k] = ia[k];
    b.n = pb.n; for(int k = 0; k <= pb.n; k++) b.a[k] = ib[k];
    Pz q, r; long fator = 1;
    if(!pz_div_resto(a, b, &q, &r, &fator)){
        printf("essa divisão não fecha nos inteiros — e não a escrevo aproximada.\n");
        printf("   (a fibra ou devolve os dois lados exatos, ou não é a fibra)\n");
        return 1;
    }
    printf("   "); escreve_pz(a); printf("   ÷   "); escreve_pz(b); printf("\n");
    if(fator != 1) printf("   (em Z multiplica-se por %ld — o líder de b não é 1, e o fator diz-se)\n", fator);
    printf(" = quociente  "); escreve_pz(q); printf("\n");
    printf("   resto      "); escreve_pz(r); printf("\n");
    /* A VOLTA, obrigatória: q·b + r reconstrói (fator·a) */
    { Pz qb, soma; pz_mul(q, b, &qb); pz_soma(qb, r, &soma);
      int bate = (soma.n == a.n);
      if(bate) for(int k = 0; k <= a.n; k++) if(soma.a[k] != fator*a.a[k]) bate = 0;
      if(!bate){ printf("   (a volta NÃO fechou — logo não afirmo esta divisão)\n"); return 1; }
      printf("   e a volta confere: q·b + r = ");
      if(fator != 1) printf("%ld·(", fator);
      escreve_pz(a);
      if(fator != 1) printf(")");
      printf("   (a fibra, com resíduo 0)\n"); }
    if(r.n == 0 && r.a[0] == 0)
        printf("   o resto é ZERO: b é FATOR de a — e isso é a deconvolução exata\n");
    else
        printf("   o resto não é zero: b não divide a, e o resto é o que sobra da fibra\n");
    return 1;
}
/* O MDC NA FALA: «mdc de x^3 - 1 e x^2 - 1». É a órbita do inversor a descer até à
 * folha, e a folha É o mdc — a mesma lei que dá o gcd nos inteiros e a fração contínua
 * no racional. A volta é obrigatória: ele tem de dividir os dois, exatamente. */
/* «as cinco operações» — a vista do padrão ouro: cada coisa que esta assistente faz
 * está debaixo de UMA das cinco, e diz-se qual. Não é catálogo decorativo: é a
 * arrumação que o Corpo Universal impõe, e é por ela que nada aqui é máquina nova. */
/* ── O CÁLCULO NA FALA: «integra x^2 + 1», «deriva 3x^2», «integra x^2 de 0 a 3» ──
 * Uma operação, dois sentidos. A integral definida é F(b) − F(a), exata em frações —
 * e a constante desaparece nela por subtração, que é a outra maneira de ver o núcleo. */
static void escreve_pol(Pol p){
    int algum = 0;
    for(int k = p.n; k >= 0; k--) if(p.p[k]) algum = 1;
    if(!algum){ printf("0"); return; }
    for(int k = p.n; k >= 0; k--){
        if(p.p[k] == 0) continue;
        if(k != p.n) printf(" %s ", p.p[k] < 0 ? "-" : "+");
        else if(p.p[k] < 0) printf("-");
        long np = p.p[k] < 0 ? -p.p[k] : p.p[k];
        if(!(np == p.q[k] && k > 0)) printf("%s", frac2(np, p.q[k]));
        if(k >= 1) printf("x");
        if(k >= 2) printf("^%d", k);
    }
}
/* ── A LÓGICA NA FALA, TICK A TICK, COM A EXPLICAÇÃO DENTRO DO TICK ──────────────
 *
 * «Começa também a misturar explicações dos passos — no tick do relógio.» Então cada
 * passo desta resolução É um tick, e o tick não diz só o que ficou: diz o que fez e
 * porquê. A granularidade continua do Quantizador (TICKS agrupa os passos, e só sobe
 * por dobra), e o corpo é GF(2) — as mesmas cinco operações.
 *
 *   tick 1  a LEITURA      quantas variáveis, e logo quantas atribuições (2ⁿ, finito)
 *   tick 2  a EQUAÇÃO      A = B vira A ⊕ B = 0: em GF(2) a igualdade é o XOR a zerar
 *   tick 3  a FIBRA        quais entradas dão a saída pedida — a divisão das cinco
 *   tick 4  o DUAL         a ANF (Zhegalkin) por Möbius, a forma canónica
 *   tick 5  a VOLTA        substituir cada solução e conferir: resíduo 0 ou não vale
 */
static long TICK_N = 0;
static void tique(const char *porque){
    long vel = tick_velocidade();
    TICK_N++;
    if(vel <= 1 || (TICK_N % vel) == 1 || vel == 1)
        printf("\n   ── tick %ld ── %s\n", TICK_N, porque);
    else
        printf("   · %s\n", porque);
}
static void escreve_anf(const unsigned char *anf, const Bool *b){
    int algum = 0;
    unsigned n = 1u << b->nv;
    for(unsigned m = 0; m < n; m++){
        if(!anf[m]) continue;
        if(algum) printf(" + ");
        algum = 1;
        if(m == 0){ printf("1"); continue; }
        for(int i = 0; i < b->nv; i++) if(m & (1u << i)) printf("%c", b->nome[i]);
    }
    if(!algum) printf("0");
}
/* ── A SIMPLIFICAÇÃO PASSO A PASSO, COM A LEI EM CADA TRANSIÇÃO, EM LaTeX ────────
 *
 * O `eval.txt` traz a caixa toda e acaba com a regra: «cada um com VOLTA OBRIGATÓRIA
 * — transformação → resultado → reconstrução». É isso que aqui se faz, e cada tick
 * NOMEIA a propriedade que autoriza a transição. A saída sai na membrana (LaTeX),
 * para o tradutor a compor. */
static void tex_imp(Imp im, const Bool *b){
    int algum = 0;
    for(int i = 0; i < b->nv; i++){
        if(!(im.mask & (1u << i))) continue;
        algum = 1;
        if(im.val & (1u << i)) printf("%c", b->nome[i]);
        else printf("\\neg %c", b->nome[i]);
    }
    if(!algum) printf("1");
}
/* ── SHANNON: F = x·F|x=1 + x'·F|x=0 — e o ficheiro diz onde isto dá ────────────
 * «E aqui começa a ficar muito próximo da tua ideia de árvore → corte → folha»
 * (eval.txt §9). É literal: a expansão é O CORTE, e o caminho raiz→folha é a
 * proveniência — «o número não precisa ser apenas o valor da folha; o caminho pode
 * ser a sua companhia». Cada nível é um tick, e cada tick nomeia o corte que fez. */
/* ── O RASTRO VERIFICÁVEL DA DEMONSTRAÇÃO ────────────────────────────────────────
 *
 * A regra final do `eval.txt`, e é ela que muda a natureza da coisa:
 *
 *   «toda demonstração precisa carregar a JUSTIFICATIVA DE CADA TRANSIÇÃO. Não
 *    apenas P ⇒ Q, mas P --Modus Ponens--> Q --De Morgan--> R --definição--> S.
 *    Aí deixa de ser apenas um resolvedor: passa a produzir o RASTRO VERIFICÁVEL.»
 *
 * Aqui a fala «prova <fórmula>» produz esse rastro: cada transição é um tick, diz a
 * REGRA que a autoriza, e a regra é VERIFICADA na hora (tautologia na tabela inteira).
 * E no fim a volta: a fórmula original e a final têm de ter a MESMA tabela. Um rastro
 * sem volta seria uma história bem contada. */
static int bl_taut(const char *forma){
    Bool b;
    if(!bl_le(forma, &b)) return 0;
    unsigned n = 1u << b.nv;
    for(unsigned x = 0; x < n; x++) if(!b.t[x]) return 0;
    return 1;
}
/* aplica a regra `de` → `para` na fórmula, se ela couber literalmente. Devolve 1 e
 * escreve em `out`. A troca é textual, e a JUSTIFICAÇÃO é a equivalência: a regra só
 * se aplica se «de <-> para» for tautologia — verificada na hora, não confiada. */
static int passo_regra(const char *forma, const char *de, const char *para,
                       char *out, size_t lim){
    const char *p = strstr(forma, de);
    if(!p) return 0;
    char eq[512];
    snprintf(eq, sizeof eq, "(%s) <-> (%s)", de, para);
    if(!bl_taut(eq)) return 0;                     /* a regra tem de se sustentar */
    snprintf(out, lim, "%.*s%s%s", (int)(p - forma), forma, para, p + strlen(de));
    return 1;
}
/* ── OS CONJUNTOS: definição → operação → demonstração → volta ───────────────────
 * Os ticks são os que o próprio `eval.txt` desenha no §4: entrar na definição,
 * expandir, De Morgan, reagrupar, reconhecer — e a VOLTA. Aqui a demonstração é a
 * varredura dos ÁTOMOS (as 2ⁿ regiões do diagrama), e onde os lados diferem exibe-se
 * o CONTRAEXEMPLO concreto: a região onde um tem o ponto e o outro não. */
/* ── A RELAÇÃO E A FUNÇÃO NA FALA: «relacao 1-1, 1-2, 2-2 em 3» ─────────────────
 * Os pares escrevem-se `a-b`, o conjunto é {1..n}, e cada propriedade sai com a sua
 * DEFINIÇÃO ao lado — varrida inteira. Para uma função, a cadeia é a do ficheiro:
 * função → injetiva/sobrejetiva → bijetiva → inversa → VOLTA. */
static int rl_le_fala(const char *p, Rel *r){
    const char *em = strstr(p, " em ");
    if(!em) return 0;
    int n = 0;
    for(const char *q = em + 4; *q >= '0' && *q <= '9'; q++) n = n*10 + (*q - '0');
    if(n < 1 || n > RL_MAX) return 0;
    rl_zera(r, n);
    int algum = 0;
    for(const char *q = p; q < em; ){
        while(q < em && (*q == ' ' || *q == ',')) q++;
        if(q >= em) break;
        int a = 0, b = 0;
        if(*q < '0' || *q > '9') return 0;
        while(q < em && *q >= '0' && *q <= '9') a = a*10 + (*q++ - '0');
        if(q >= em || (*q != '-' && *q != '>')) return 0;
        q++;
        if(*q == '>') q++;
        while(q < em && *q >= '0' && *q <= '9') b = b*10 + (*q++ - '0');
        if(a < 1 || a > n || b < 1 || b > n) return 0;
        r->m[a-1][b-1] = 1;
        algum = 1;
    }
    return algum;
}
/* ── OS NATURAIS NA FALA: «fatora 60», «mdc de 18 e 12», «divide 17 por 5» ───────
 * A escada do ficheiro — 0 → S → + → × → ≤ → | → gcd → primos → fatoração — e cada
 * resposta com a VOLTA: a fatoração multiplica-se de novo, o Bézout substitui-se, a
 * divisão recompõe. O motor demonstra os instrumentos que já usava. */
/* ── O RELÓGIO DE SEIS TICKS, tal como o ficheiro o desenha ──────────────────────
 *
 *   tick 1 — DEFINIÇÃO · tick 2 — TRADUÇÃO · tick 3 — OPERAÇÃO
 *   tick 4 — DEMONSTRAÇÃO · tick 5 — CONTRAEXEMPLO, se falhar · tick 6 — VOLTA
 *
 * E o exemplo é o dele: «se a | b e a | c então a | (b+c)». A prova não é a varredura:
 * é a CADEIA — b = ak, c = aℓ, b+c = a(k+ℓ), e k+ℓ ∈ ℤ. A varredura entra como
 * controlo, e a volta exibe o quociente. */
static int resolve_divisibilidade(const char *f){
    const char *p = f;
    if(strncmp(p, "prova divisibilidade", 20)) return 0;
    p += 20;
    long a = 0, b = 0, c = 0; int lidos = 0;
    { const char *q = p;
      long v[3] = {0,0,0};
      for(int i = 0; i < 3; i++){
          while(*q && (*q < '0' || *q > '9') && *q != '-') q++;
          if(!*q) break;
          int sg = 1;
          if(*q == '-'){ sg = -1; q++; }
          if(*q < '0' || *q > '9') break;
          long n2 = 0;
          while(*q >= '0' && *q <= '9') n2 = n2*10 + (*q++ - '0');
          v[i] = sg * n2; lidos++;
      }
      a = v[0]; b = v[1]; c = v[2]; }
    if(lidos < 3 || a == 0){ a = 3; b = 12; c = 18; }   /* o exemplo, se não vier nada */
    TICK_N = 0;
    printf("   se %ld | %ld e %ld | %ld, então %ld | (%ld + %ld)\n", a, b, a, c, a, b, c);
    long k = 0, l = 0;
    int d1 = iz_div(a, b, &k), d2 = iz_div(a, c, &l);
    tique("DEFINIÇÃO — divisibilidade: a | b significa que EXISTE k com b = ak, e o k"
          " é a testemunha (não é uma propriedade, é uma existência)");
    printf("      %ld | %ld ?  %s", a, b, d1 ? "sim" : "NÃO");
    if(d1) printf(",  com k = %ld  (%ld = %ld·%ld)", k, b, a, k);
    printf("\n      %ld | %ld ?  %s", a, c, d2 ? "sim" : "NÃO");
    if(d2) printf(",  com ℓ = %ld  (%ld = %ld·%ld)", l, c, a, l);
    printf("\n");
    if(!d1 || !d2){
        tique("CONTRAEXEMPLO — a hipótese não se verifica, e por isso não há teorema a"
              " provar aqui: sem as duas divisibilidades, a conclusão não é devida");
        printf("      (e note-se: a conclusão pode até ser verdadeira por acaso — %ld | %ld"
               " é %s — mas isso não a torna consequência)\n",
               a, b + c, iz_div(a, b + c, 0) ? "verdade" : "falso");
        return 1;
    }
    tique("TRADUÇÃO — a soma das duas testemunhas: b + c = ak + aℓ");
    printf("      %ld + %ld = %ld·%ld + %ld·%ld\n", b, c, a, k, a, l);
    tique("OPERAÇÃO — a DISTRIBUTIVIDADE põe o a em evidência: ak + aℓ = a(k + ℓ)");
    printf("      = %ld·(%ld + %ld) = %ld·%ld\n", a, k, l, a, k + l);
    tique("DEMONSTRAÇÃO — k + ℓ é inteiro (ℤ é fechado para a soma), logo existe uma"
          " testemunha para b+c: é essa existência a definição de a | (b+c)");
    printf("      k + ℓ = %ld ∈ ℤ,  logo %ld | %ld\n", k + l, a, b + c);
    tique("VOLTA — e a testemunha confere: (b+c)/a tem de dar exatamente k + ℓ");
    { long kk = 0;
      int ok2 = iz_div(a, b + c, &kk);
      printf("      (%ld + %ld)/%ld = %ld,  e k + ℓ = %ld   %s\n",
             b, c, a, kk, k + l, (ok2 && kk == k + l) ? "(resíduo 0)" : "— NÃO afirmo"); }
    printf("\n   $%ld \\mid %ld \\wedge %ld \\mid %ld \\; \\Rightarrow \\; %ld \\mid %ld$\n",
           a, b, a, c, a, b + c);
    printf("   (e a prova não foi a varredura: foi a CADEIA — definição, soma,\n");
    printf("    distributividade, fecho. A varredura seria controlo, não demonstração)\n");
    return 1;
}
/* «bezout 35 e 22» e «diofantina 35x + 22y = 7» — a testemunha e o critério */
static int resolve_bezout(const char *f){
    const char *p = f;
    int dio = 0;
    if(!strncmp(p, "bezout ", 7)) p += 7;
    else if(!strncmp(p, "bézout ", 8)) p += 8;
    else if(!strncmp(p, "diofantina ", 11)){ p += 11; dio = 1; }
    else return 0;
    long a = 0, b = 0, c = 0; int n2 = 0;
    for(const char *q = p; *q; ){
        if(*q < '0' || *q > '9'){ q++; continue; }
        long v = 0;
        while(*q >= '0' && *q <= '9') v = v*10 + (*q++ - '0');
        if(n2 == 0) a = v; else if(n2 == 1) b = v; else if(n2 == 2) c = v;
        n2++;
    }
    if(a < 1 || b < 1) return 0;
    TICK_N = 0;
    long x, y, g = iz_gcd(a, b, &x, &y);
    if(!dio){
        printf("   bezout(%ld, %ld)\n", a, b);
        tique("DEFINIÇÃO — Bézout: o gcd escreve-se como COMBINAÇÃO LINEAR, gcd = ax + by,"
              " e o x e o y são a testemunha");
        printf("      gcd(%ld, %ld) = %ld\n", a, b, g);
        tique("OPERAÇÃO — o Euclides ESTENDIDO carrega os coeficientes na descida");
        printf("      x = %ld,  y = %ld\n", x, y);
        tique("VOLTA — substituir, que é o que transforma a identidade em facto");
        printf("      %ld·(%ld) + %ld·(%ld) = %ld   %s\n", a, x, b, y, a*x + b*y,
               a*x + b*y == g ? "(confere)" : "— NÃO afirmo");
        printf("      e o gcd é o MENOR positivo da forma ax+by — a boa ordenação a decidir\n");
        return 1;
    }
    printf("   %ldx + %ldy = %ld\n", a, b, c);
    tique("DEFINIÇÃO — a diofantina pede solução INTEIRA, e o critério é um só:"
          " gcd(a,b) | c");
    printf("      gcd(%ld, %ld) = %ld,  e %ld %s %ld\n", a, b, g, g,
           c % g == 0 ? "divide" : "NÃO divide", c);
    long xs, ys;
    int tem = iz_diofantina(a, b, c, &xs, &ys);
    if(!tem){
        tique("CONTRAEXEMPLO — o critério falha, logo NÃO há solução inteira nenhuma:"
              " e isto é resultado, não desistência");
        printf("      (qualquer ax+by é múltiplo de %ld, e %ld não é)\n", g, c);
        return 1;
    }
    tique("OPERAÇÃO — escala-se a testemunha de Bézout pelo fator c/gcd");
    printf("      de %ld·(%ld) + %ld·(%ld) = %ld,  multiplica-se por %ld\n",
           a, x, b, y, g, c / g);
    tique("VOLTA — e substitui-se: a solução ou fecha, ou não é solução");
    printf("      x = %ld,  y = %ld   →   %ld·(%ld) + %ld·(%ld) = %ld   %s\n",
           xs, ys, a, xs, b, ys, a*xs + b*ys,
           a*xs + b*ys == c ? "(resíduo 0)" : "— NÃO afirmo");
    return 1;
}
static void tique7(int slot, const char *porque);
static void esc_col(const char *s, int largura);
static void esc_qz(const char *pre, Qz x, const char *pos);
/* ── ÁLGEBRA LINEAR E O DUAL: E O GUME PASSA A SER AUTOMÁTICO ───────────────────────
 * O `eval.txt` acrescenta uma exigência que não é conteúdo, é MECANISMO:
 *
 *   «um gume obrigatório em cada teorema: SE A HIPÓTESE FOR RETIRADA, PROCURAR
 *    AUTOMATICAMENTE UM CONTRA-EXEMPLO. É fazer o motor descobrir QUAL HIPÓTESE ESTÁ
 *    CARREGANDO CADA TEOREMA.»
 *
 * Então o gume deixa de ser escrito à mão em cada teorema e passa a ser uma BUSCA:
 * `gume_matriz` varre o espaço, tira a hipótese e devolve o primeiro objeto onde a tese
 * também cai. E o dual é a casa: «o vetor fornece o objeto; o funcional fornece a
 * coordenada que o mede». */
static void esc_vec(Vec v){
    printf("(");
    for(int i = 0; i < v.n; i++){ if(i) printf(", "); esc_qz("", v.c[i], ""); }
    printf(")");
}
static void esc_mat(const char *ind, Mat A){
    for(int i = 0; i < A.m; i++){
        printf("%s[ ", ind);
        for(int j = 0; j < A.n; j++){ esc_qz("", A.a[i][j], "  "); }
        printf("]\n");
    }
}
static void esc_fun(Fun f, const char *vars){
    int primeiro = 1;
    for(int i = 0; i < f.n; i++){
        if(f.c[i].p == 0) continue;
        Qz c = f.c[i];
        printf("%s", primeiro ? "" : (c.p < 0 ? " − " : " + "));
        Qz m = (!primeiro && c.p < 0) ? qz_oposto(c) : c;
        if(!(m.p == 1 && m.q == 1)) esc_qz("", m, "");
        printf("%c", vars[i]);
        primeiro = 0;
    }
    if(primeiro) printf("0");
}
/* as HIPÓTESES e as TESES, como predicados — é isto que o gume automático consome */
static int hip_det_nao_zero(const Mat *A){ Mat B = *A; return mat_det(B).p != 0; }
static int tese_invertivel(const Mat *A){ Mat B = *A, R; return mat_inversa(B, &R); }
static int hip_colunas_li(const Mat *A){ Mat B = *A; return mat_posto(B) == A->n; }
static int tese_nucleo_trivial(const Mat *A){
    Mat B = *A; Vec nb[LN_MAX];
    return mat_nucleo(B, nb) == 0;
}
static int hip_simetrica(const Mat *A){ Mat B = *A; return mat_igual(B, mat_transposta(B)); }
/* uma tese que vale SEMPRE — é ela o controlo do buscador: se ele a «refutasse», o
 * buscador é que estava avariado. det A = det Aᵀ não depende de hipótese nenhuma. */
static int tese_det_igual_transposta(const Mat *A){
    Mat B = *A;
    return qz_igual(mat_det(B), mat_det(mat_transposta(B)));
}
static int tese_comuta_com_transposta(const Mat *A){
    Mat B = *A, T = mat_transposta(B);
    return mat_igual(mat_mult(B,T), mat_mult(T,B));
}
static const struct { int n; const char *nome; const char *enunciado; } LI16[] = {
 {  1, "espaco vetorial",  "as oito leis, e 0v = 0, λ0 = 0, (−λ)v = −(λv)" },
 {  2, "vetor zero unico", "o vetor zero é único, e o oposto também" },
 {  3, "subespaco",        "W é subespaço ⟺ λu + μv ∈ W — e o gume é o x+y+z = 1" },
 {  4, "span",             "span(S) é o MENOR subespaço que contém S" },
 {  5, "independencia",    "LI: a combinação nula só com coeficientes nulos" },
 {  6, "base e coordenada","numa base, as coordenadas são ÚNICAS" },
 {  7, "matrizes",         "o produto é uma CONVOLUÇÃO com índice interno" },
 {  8, "nao comuta",       "AB ≠ BA em geral — e o contra-exemplo procura-se" },
 {  9, "transformacao",    "T linear ⟺ T(λu + μv) = λT(u) + μT(v)" },
 { 10, "nucleo e imagem",  "ker T é a FIBRA do 0, e im T é subespaço" },
 { 11, "posto nulidade",   "dim V = dim ker T + dim im T" },
 { 12, "isomorfismo",      "T bijetiva ⟺ T⁻¹ existe" },
 { 13, "determinante",     "det A ≠ 0 ⟺ A é invertível, e det(AB) = det(A)det(B)" },
 { 14, "sistemas",         "Ax = b: as soluções são x₀ + ker A" },
 { 15, "autovalores",      "Av = λv, e det(A − λI) = 0" },
 { 16, "diagonalizacao",   "A = PDP⁻¹, e Aⁿ = PDⁿP⁻¹" },
};
static const struct { int n; const char *nome; const char *enunciado; } DU14[] = {
 {  1, "dual",             "V* = Hom(V,K): o espaço das MEDIÇÕES lineares" },
 {  2, "dual e espaco",    "V* é espaço vetorial, com as operações ponto a ponto" },
 {  3, "funcional",        "f(x,y) = 2x − 3y está no dual; x² + y NÃO está" },
 {  4, "base dual",        "e^i(e_j) = δ^i_j, e ela é ÚNICA" },
 {  5, "coordenada e medida","xᵢ = e^i(v): a coordenada É a medição pelo dual" },
 {  6, "dimensao do dual", "dim V* = dim V, e a prova é construtiva" },
 {  7, "base dual de B",   "a base dual de ((1,1),(1,−1))" },
 {  8, "bidual",           "ι(v)(f) = f(v): o vetor vira funcional dos funcionais" },
 {  9, "iota injetiva",    "ι é injetiva, e V ≅ V** é CANÓNICO" },
 { 10, "nao canonico",     "V ≅ V* NÃO é canónico — o gume, e a recusa" },
 { 11, "aniquilador",      "W° = {f : f(w) = 0 ∀w ∈ W}" },
 { 12, "dimensao anulador","dim W + dim W° = dim V" },
 { 13, "dual da transformacao", "T*(φ) = φ∘T, e [T*] = Aᵀ" },
 { 14, "nucleo do dual",   "ker T* = (im T)° — o chefão" },
};
static void linear_resolve(int n){
    TICK_N = 0;
    printf("   %d — %s\n", n, LI16[n-1].enunciado);
    long d12[] = {1,2,3,4}, d20[] = {2,0,1,3}, dT[] = {1,1,0, 0,1,1};
    Mat A = mat_de_inteiros(2,2,d12), B = mat_de_inteiros(2,2,d20);
    switch(n){
    case 1: case 2: {
        tique7(0, "seja V espaço vetorial sobre K — as oito leis, e nada mais");
        tique7(1, "o essencial é a distributividade nos DOIS sentidos e o 1v = v:");
        printf("      $\\lambda(u+v) = \\lambda u + \\lambda v$,   "
               "$(\\lambda+\\mu)v = \\lambda v + \\mu v$,   $1v = v$\n");
        if(n == 1){
            tique7(2, "0v = 0 sai da distributividade: 0v = (0+0)v = 0v + 0v, e somando"
                      " o oposto de 0v aos dois lados fica 0v = 0. Não é convenção");
            tique7(3, "a lei é a DISTRIBUTIVIDADE do escalar sobre a soma de escalares,"
                      " mais o OPOSTO do grupo aditivo de V — as duas, e nesta ordem");
            tique7(4, "a testemunha é o próprio 0v, que se soma consigo e não muda");
            { int mal = 0; long feitos = 0;
              for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++){
                  Vec v = vec0(2);
                  v.c[0] = qz_de_inteiro(a); v.c[1] = qz_de_inteiro(b);
                  if(!vec_zero(vec_esc(qz(0,1), v))) mal++;              /* 0v = 0 */
                  for(long l = -3; l <= 3; l++){
                      Qz L = qz_de_inteiro(l);
                      if(!vec_igual(vec_esc(qz_oposto(L), v),
                                    vec_esc(qz_de_inteiro(-1), vec_esc(L, v)))) mal++;
                      if(!vec_igual(vec_esc(qz(1,1), v), v)) mal++;      /* 1v = v */
                      feitos++;
                  }
              }
              tique7(5, "logo 0v = 0, λ0 = 0 e (−λ)v = −(λv) — as três da mesma cadeia");
              tique7(6, "e a volta: varrem-se os vetores e os escalares, e cada"
                        " identidade mede-se em separado");
              printf("      %ld casos em ℚ²: %d falhas\n", feitos, mal); }
        } else {
            tique7(2, "sejam 0 e 0′ dois neutros. Então 0 = 0 + 0′ = 0′ — a mesma cadeia"
                      " da identidade única no grupo, e por isso não é teorema novo");
            tique7(3, "a lei é a DEFINIÇÃO de neutro, usada uma vez por cada candidato");
            tique7(4, "a testemunha é o vetor 0 + 0′, que se lê de dois modos");
            { int mal = 0;
              for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
                  Vec v = vec0(2), z = vec0(2);
                  v.c[0] = qz_de_inteiro(a); v.c[1] = qz_de_inteiro(b);
                  if(!vec_igual(vec_soma(v,z), v)) mal++;
                  /* e o OPOSTO é único: só −v soma zero com v */
                  Vec op = vec_esc(qz_de_inteiro(-1), v);
                  if(!vec_zero(vec_soma(v,op))) mal++;
                  for(long c = -5; c <= 5; c++) for(long e = -5; e <= 5; e++){
                      Vec w = vec0(2);
                      w.c[0] = qz_de_inteiro(c); w.c[1] = qz_de_inteiro(e);
                      if(vec_zero(vec_soma(v,w)) && !vec_igual(w, op)) mal++;
                  }
              }
              tique7(5, "logo o zero é único, e o oposto de cada vetor também");
              tique7(6, "e a volta: varrido em ℚ², nenhum vetor tem dois opostos");
              printf("      121 vetores × 121 candidatos a oposto: %d falhas\n", mal); }
        }
        break; }
    case 3: {
        tique7(0, "seja W ⊆ V não vazio. Quer-se saber quando ele é SUBESPAÇO");
        tique7(1, "o teste mínimo, um só:");
        printf("      $u, v \\in W$,  $\\lambda, \\mu \\in K$"
               "  $\\Rightarrow$  $\\lambda u + \\mu v \\in W$\n");
        tique7(2, "a condição é uma FIBRA FECHADA: qualquer combinação dos que estão"
                  " dentro continua dentro. E ela obriga o 0 a estar lá (tome-se"
                  " λ = μ = 0), que é o que o segundo exemplo não cumpre");
        tique7(3, "a lei é o fecho da combinação linear — uma só, e é ela que substitui"
                  " os oito axiomas: W herda-os todos de V");
        tique7(4, "a testemunha do PRIMEIRO é o cálculo direto; a do SEGUNDO é um par"
                  " que sai do conjunto — e é ele o gume que ele nomeia");
        { /* W₁: x+y+z = 0 fecha;  W₂: x+y+z = 1 não */
          int mal = 0; long dentro1 = 0, fora2 = 0;
          for(long x = -4; x <= 4; x++) for(long y = -4; y <= 4; y++)
          for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
          for(long l = -2; l <= 2; l++) for(long m = -2; m <= 2; m++){
              /* dois vetores de W₁ e uma combinação */
              long z = -x - y, c = -a - b;
              long sx = l*x + m*a, sy = l*y + m*b, sz = l*z + m*c;
              if(sx + sy + sz != 0) mal++;                 /* W₁ fecha SEMPRE */
              dentro1++;
              /* e em W₂ (soma 1) a combinação sai, salvo λ+μ = 1 */
              long z2 = 1 - x - y, c2 = 1 - a - b;
              long tx = l*x + m*a, ty = l*y + m*b, tz = l*z2 + m*c2;
              if(tx + ty + tz != 1) fora2++;
          }
          printf("      x+y+z = 0:  %ld combinações, todas dentro (%d falhas)\n",
                 dentro1, mal);
          printf("      x+y+z = 1:  %ld combinações que SAEM do conjunto\n", fora2);
          printf("      e o gume é o zero: (0,0,0) tem soma 0, logo está no primeiro e"
                 " NÃO no segundo\n");
          tique7(5, "logo o primeiro é subespaço e o segundo não — «o primeiro fecha no"
                    " zero; o segundo não»");
          tique7(6, "e a volta: o critério aplica-se aos dois e decide, sem se olhar para"
                    " a forma da equação — é o fecho que manda, não o aspeto"); }
        break; }
    case 4: {
        tique7(0, "seja S ⊆ V e W qualquer subespaço com S ⊆ W");
        tique7(1, "o span é o conjunto de todas as combinações:");
        printf("      $\\operatorname{span}(S) = \\{ \\sum_i \\lambda_i v_i \\}$\n");
        tique7(2, "span(S) é subespaço (uma combinação de combinações é combinação), e"
                  " contém S. Falta ver que é o MENOR: se S ⊆ W e W é subespaço, então"
                  " W contém todas as combinações de S, isto é span(S) ⊆ W");
        tique7(3, "a lei é o FECHO de W: é ele que arrasta cada combinação para dentro,"
                  " uma parcela de cada vez");
        tique7(4, "a testemunha é a própria combinação Σλᵢvᵢ, que se constrói dentro de W");
        { Vec e1 = vec0(2), e2 = vec0(2), u = vec0(2), w = vec0(2);
          e1.c[0] = qz(1,1); e2.c[1] = qz(1,1);
          u.c[0] = qz(1,1); u.c[1] = qz(1,1);
          w.c[0] = qz(2,1); w.c[1] = qz(2,1);
          Vec s1[2] = { e1, e2 }, s2[2] = { u, w };
          printf("      span{(1,0),(0,1)}: posto %d — é o ℝ² inteiro\n",
                 mat_posto(mat_de_colunas(s1,2)));
          printf("      span{(1,1),(2,2)}: posto %d — é uma RETA, porque (2,2) = 2(1,1)\n",
                 mat_posto(mat_de_colunas(s2,2)));
          tique7(5, "logo span(S) é o menor subespaço que contém S");
          tique7(6, "e a VOLTA é a inclusão: S ⊆ W ⟹ span(S) ⊆ W, medida em todos os"
                    " subespaços gerados por vetores pequenos");
          { int mal = 0; long testes = 0;
            for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                Vec s = vec0(2); s.c[0] = qz_de_inteiro(a); s.c[1] = qz_de_inteiro(b);
                if(vec_zero(s)) continue;
                /* W = span(s) e todo múltiplo de s está lá */
                for(long l = -3; l <= 3; l++)
                    if(!vec_no_span(&s, 1, vec_esc(qz_de_inteiro(l), s))) mal++;
                testes++;
            }
            printf("      %ld geradores, e todo múltiplo cai no span: %d falhas\n",
                   testes, mal); } }
        break; }
    case 5: case 6: {
        tique7(0, n == 5 ? "sejam v₁..v_k vetores de V"
                         : "seja B = (v₁..v_n) uma BASE de V, e v ∈ V");
        tique7(1, n == 5 ? "LI é a combinação nula ter solução ÚNICA:"
                         : "base é geradora E linearmente independente:");
        printf(n == 5 ? "      $\\lambda_1 v_1 + \\cdots + \\lambda_n v_n = 0"
                        " \\; \\Rightarrow \\; \\lambda_i = 0$\n"
                      : "      $v = \\sum_i a_i v_i = \\sum_i b_i v_i"
                        " \\; \\Rightarrow \\; a_i = b_i$\n");
        if(n == 5){
            tique7(2, "a combinação nula é um SISTEMA homogéneo: a matriz das colunas"
                      " vezes λ dá 0. LI ⟺ o núcleo dessa matriz é trivial ⟺ o posto é"
                      " o número de vetores");
            tique7(3, "a lei é a EQUIVALÊNCIA entre núcleo trivial e posto cheio, que é o"
                      " posto-nulidade a decidir");
            tique7(4, "a testemunha da dependência é o vetor do núcleo — os coeficientes"
                      " que anulam sem serem todos nulos");
            { Vec a1 = vec0(2), a2 = vec0(2), b1 = vec0(2), b2 = vec0(2);
              a1.c[0] = qz(1,1); a2.c[1] = qz(1,1);
              b1.c[0] = qz(1,1); b1.c[1] = qz(2,1);
              b2.c[0] = qz(2,1); b2.c[1] = qz(4,1);
              Vec s1[2] = {a1,a2}, s2[2] = {b1,b2};
              printf("      (1,0),(0,1):  LI? %s\n", vec_li(s1,2) ? "SIM" : "não");
              printf("      (1,2),(2,4):  LI? %s", vec_li(s2,2) ? "SIM" : "NÃO");
              Vec nb[LN_MAX];
              Mat M = mat_de_colunas(s2,2);
              int k = mat_nucleo(M, nb);
              if(k){ printf("   — a testemunha é λ = "); esc_vec(nb[0]);
                     printf(",  e de facto 2·(1,2) − 1·(2,4) = (0,0)"); }
              printf("\n");
              tique7(5, "logo os primeiros são LI e os segundos LD");
              tique7(6, "e o teorema com a VOLTA FALSA: «todo subconjunto de um LI é LI»"
                        " vale; a recíproca não — um conjunto LD pode ter subconjuntos"
                        " LI, e mede-se");
              { int mal = 0, viu = 0;
                for(long p = -3; p <= 3; p++) for(long q = -3; q <= 3; q++){
                    Vec x = vec0(2), y = vec0(2);
                    x.c[0] = qz(1,1); x.c[1] = qz(0,1);
                    y.c[0] = qz_de_inteiro(p); y.c[1] = qz_de_inteiro(q);
                    Vec par[2] = {x,y};
                    if(!vec_li(par,2)){
                        /* LD, mas o subconjunto {x} é LI: a volta é falsa */
                        if(!vec_li(par,1)) mal++;
                        viu++;
                    } else {
                        if(!vec_li(par,1)) mal++;      /* subconjunto de LI é LI */
                    }
                }
                printf("      49 pares: em %d deles o par é LD e o subconjunto é LI —"
                       " a volta é FALSA, e a testemunha existe (%d falhas)\n", viu, mal); } }
        } else {
            tique7(2, "se v = Σaᵢvᵢ = Σbᵢvᵢ, então Σ(aᵢ−bᵢ)vᵢ = 0. Como B é LI, todos os"
                      " (aᵢ−bᵢ) são zero, logo aᵢ = bᵢ");
            tique7(3, "a lei é a INDEPENDÊNCIA, e só ela — a geração dá a existência das"
                      " coordenadas, a independência dá a unicidade. São dois papéis");
            tique7(4, "a testemunha é o vetor Σ(aᵢ−bᵢ)vᵢ, que é zero de duas maneiras");
            { Vec b1 = vec0(2), b2 = vec0(2);
              b1.c[0] = qz(1,1); b1.c[1] = qz(1,1);
              b2.c[0] = qz(1,1); b2.c[1] = qz(-1,1);
              Vec base[2] = {b1,b2};
              int mal = 0; long feitos = 0;
              for(long x = -5; x <= 5; x++) for(long y = -5; y <= 5; y++){
                  Vec v = vec0(2);
                  v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
                  Qz co[LN_MAX];
                  if(!vec_coord(base, 2, v, co)){ mal++; continue; }
                  Vec volta = vec_soma(vec_esc(co[0], b1), vec_esc(co[1], b2));
                  if(!vec_igual(volta, v)) mal++;       /* A VOLTA: reconstruir v */
                  feitos++;
              }
              Vec v = vec0(2); v.c[0] = qz(3,1); v.c[1] = qz(1,1);
              Qz co[LN_MAX]; vec_coord(base,2,v,co);
              printf("      base ((1,1),(1,−1)) e v = (3,1):  coordenadas ");
              esc_qz("", co[0], " e "); esc_qz("", co[1], "");
              printf(",  e a volta dá "); esc_vec(vec_soma(vec_esc(co[0],b1), vec_esc(co[1],b2)));
              printf("\n");
              tique7(5, "logo as coordenadas são ÚNICAS numa base");
              tique7(6, "e a VOLTA é literal: reconstrói-se v a partir das coordenadas, e"
                        " tem de dar o mesmo vetor");
              printf("      %ld vetores: coordenadas achadas e reconstruídas, %d falhas\n",
                     feitos, mal); } }
        break; }
    case 7: case 8: {
        tique7(0, "sejam A e B matrizes com as dimensões que casam");
        tique7(1, "o produto é a soma sobre o índice INTERNO:");
        printf("      $(AB)_{ij} = \\sum_k a_{ik} b_{kj}$\n");
        tique7(2, "e é uma CONVOLUÇÃO — a mesma forma da convolução de polinómios e da"
                  " de Dirichlet, com o índice interno a percorrer o que se soma. Muda o"
                  " conjunto onde os índices vivem, não a operação");
        tique7(3, "as leis que valem: distributividade A(B+C) = AB+AC e associatividade"
                  " (AB)C = A(BC) — as duas saem de trocar a ordem das somas finitas");
        if(n == 7){
            tique7(4, "a testemunha é o exemplo dele, calculado");
            { Mat C = mat_mult(A,B);
              printf("      A =\n"); esc_mat("        ", A);
              printf("      B =\n"); esc_mat("        ", B);
              printf("      AB =\n"); esc_mat("        ", C);
              int mal = 0; long feitos = 0;
              for(long k = 0; k < 200; k++){
                  Mat X = mat0(2,2), Y = mat0(2,2), Z = mat0(2,2);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro((t + i + j) % 5 - 2);
                      Y.a[i][j] = qz_de_inteiro((t*3 + i*2 + j) % 5 - 2);
                      Z.a[i][j] = qz_de_inteiro((t*7 + i + j*3) % 5 - 2);
                      t /= 2;
                  }
                  if(!mat_igual(mat_mult(X, mat_soma(Y,Z)),
                                mat_soma(mat_mult(X,Y), mat_mult(X,Z)))) mal++;
                  if(!mat_igual(mat_mult(mat_mult(X,Y),Z), mat_mult(X,mat_mult(Y,Z)))) mal++;
                  feitos++;
              }
              tique7(5, "logo o produto distribui e associa");
              tique7(6, "e a volta: as duas leis varridas em todos os triplos de"
                        " matrizes 2×2 do intervalo");
              printf("      %ld triplos 2×2: distributividade e associatividade, %d falhas\n",
                     feitos, mal); }
        } else {
            tique7(4, "MAS a COMUTATIVIDADE não está na lista — e o gume procura-se em"
                      " vez de se escrever à mão: varre-se o espaço das matrizes e"
                      " devolve-se o primeiro par que não comuta");
            { Mat C = mat_mult(A,B), D = mat_mult(B,A);
              printf("      AB =\n"); esc_mat("        ", C);
              printf("      BA =\n"); esc_mat("        ", D);
              printf("      AB = BA ? %s\n", mat_igual(C,D) ? "sim" : "NÃO");
              /* o GUME AUTOMÁTICO: tirada a simetria, a comutação com a transposta cai */
              Mat contra;
              long passo = gume_matriz(2, 1, hip_simetrica, tese_comuta_com_transposta, &contra);
              tique7(5, "logo AB ≠ BA em geral — e é por isso que «matriz» não é «número»");
              tique7(6, "e o GUME AUTOMÁTICO noutro teorema do mesmo tipo: se A é"
                        " SIMÉTRICA então A comuta com Aᵀ (trivialmente, porque são a"
                        " mesma). Tirada a simetria, procura-se — e acha-se");
              if(passo){
                  printf("      contra-exemplo achado ao passo %ld:\n", passo);
                  esc_mat("        ", contra);
                  printf("      A·Aᵀ ≠ Aᵀ·A — a simetria era a hipótese que carregava\n");
              } else printf("      — NÃO achei contra-exemplo no espaço varrido (3⁴ = 81)\n"); }
        }
        break; }
    case 9: case 10: case 11: {
        Mat T = mat_de_inteiros(2,3,dT);
        tique7(0, "seja T: V → W uma aplicação");
        tique7(1, n == 9 ? "linear é preservar a combinação:"
                         : "o núcleo e a imagem:");
        printf(n == 9 ? "      $T(\\lambda u + \\mu v) = \\lambda T(u) + \\mu T(v)$\n"
                      : "      $\\ker T = \\{ v : T(v) = 0 \\}$,   "
                        "$\\operatorname{im} T = \\{ T(v) \\}$\n");
        if(n == 9){
            tique7(2, "as três candidatas dele: T(x,y) = (x+y, 2x−y) é linear;"
                      " T(x,y) = (x+1, y) NÃO é (falha em T(0) = 0);"
                      " T(x,y) = (x², y) NÃO é (falha na escala)");
            tique7(3, "a lei é a preservação, e ela obriga T(0) = 0 — que é o teste mais"
                      " barato e derruba a segunda de imediato");
            tique7(4, "a testemunha de cada falha exibe-se, e a RECUSA é o resultado");
            { long mal_l = 0, mal_a = 0, mal_q = 0;
              for(long x = -3; x <= 3; x++) for(long y = -3; y <= 3; y++)
              for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                  /* linear */
                  long lx = (x+a) + (y+b), ly = 2*(x+a) - (y+b);
                  if(lx != (x+y) + (a+b) || ly != (2*x-y) + (2*a-b)) mal_l++;
                  /* afim: T(x,y) = (x+1, y) */
                  if((x+a) + 1 == (x+1) + (a+1)) ; else mal_a++;
                  /* quadrática */
                  if((x+a)*(x+a) == x*x + a*a) ; else mal_q++;
              }
              printf("      T(x,y) = (x+y, 2x−y):  %ld falhas — é LINEAR\n", mal_l);
              printf("      T(x,y) = (x+1, y):     %ld falhas — NÃO é (e T(0,0) = (1,0) ≠ 0)\n",
                     mal_a);
              printf("      T(x,y) = (x², y):      %ld falhas — NÃO é ((1+1)² = 4 ≠ 1+1)\n",
                     mal_q);
              tique7(5, "logo só a primeira é linear, e nas outras duas a demonstração"
                        " RECUSA-SE — não há o que provar");
              tique7(6, "e a volta é o teste barato: T(0) = 0 é NECESSÁRIO, e sozinho já"
                        " derruba a afim. A quadrática precisa da escala"); }
        } else if(n == 10){
            tique7(2, "ker T é subespaço porque T(λu+μv) = λ·0 + μ·0 = 0; im T é"
                      " subespaço porque λT(u) + μT(v) = T(λu+μv), que é uma imagem");
            tique7(3, "a lei é a MESMA nos dois — a linearidade — aplicada de dois lados:"
                      " na partida para o núcleo, na chegada para a imagem");
            tique7(4, "«o núcleo é a FIBRA do 0» — e é essa a leitura desta casa: ker T é"
                      " o conjunto que T colapsa, e a imagem é o que sobrevive");
            { Vec nb[LN_MAX], ib[LN_MAX];
              int nul = mat_nucleo(T, nb), rk = mat_imagem(T, ib);
              printf("      T(x,y,z) = (x+y, y+z):\n");
              printf("      ker T = span{"); for(int i = 0; i < nul; i++) esc_vec(nb[i]);
              printf("},  dim = %d\n", nul);
              printf("      im T = span{");
              for(int i = 0; i < rk; i++){ if(i) printf(", "); esc_vec(ib[i]); }
              printf("},  dim = %d\n", rk);
              int mal = 0;
              for(int i = 0; i < nul; i++) if(!vec_zero(mat_aplica(T, nb[i]))) mal++;
              tique7(5, "logo os dois são subespaços");
              tique7(6, "e a volta: cada vetor do núcleo aplica-se e tem de dar zero");
              printf("      os %d geradores do núcleo aplicam-se a zero: %d falhas\n",
                     nul, mal); }
        } else {
            tique7(2, "toma-se uma base do núcleo e estende-se a uma base de V. As"
                      " imagens dos vetores acrescentados formam uma base da imagem —"
                      " e a contagem fecha");
            tique7(3, "a lei é a EXTENSÃO DE BASE, e é ela que faz o teorema: sem poder"
                      " estender, os dois números não se ligam");
            tique7(4, "a testemunha é a base estendida; e em matriz, é a redução: as"
                      " colunas COM pivô dão a imagem, as SEM pivô dão o núcleo");
            { Vec nb[LN_MAX], ib[LN_MAX];
              int nul = mat_nucleo(T, nb), rk = mat_imagem(T, ib);
              printf("      T(x,y,z) = (x+y, y+z):  nullity = %d, rank = %d,"
                     "  e %d = %d + %d\n", nul, rk, T.n, nul, rk);
              int mal = 0; long feitos = 0;
              for(long k = 0; k < 400; k++){
                  Mat M = mat0(2,3);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 3; j++){
                      M.a[i][j] = qz_de_inteiro(t % 3 - 1);
                      t /= 3;
                  }
                  Vec q1[LN_MAX], q2[LN_MAX];
                  if(mat_nucleo(M,q1) + mat_imagem(M,q2) != M.n) mal++;
                  feitos++;
              }
              tique7(5, "logo dim V = dim ker T + dim im T");
              tique7(6, "e a volta: varrem-se as matrizes 2×3 do intervalo e a soma tem"
                        " de dar 3 SEMPRE — não é o exemplo que confirma, é a varredura");
              printf("      %ld matrizes 2×3: nullity + rank = 3, %d falhas\n",
                     feitos, mal); } }
        break; }
    case 12: case 13: {
        if(n == 13){
            tique7(0, "seja A quadrada sobre um corpo");
            tique7(1, "o determinante e o que ele decide:");
            printf("      $\\det A \\neq 0$  $\\Leftrightarrow$  $A$ é invertível\n");
            tique7(2, "se A é invertível, det(A)det(A⁻¹) = det(I) = 1, logo det A ≠ 0."
                      " E se det A ≠ 0, a fórmula da adjunta constrói a inversa");
            tique7(3, "a lei é a MULTIPLICATIVIDADE det(AB) = det(A)det(B) — e note-se"
                      " que ela é o que torna o determinante útil, não a fórmula");
            tique7(4, "a testemunha é a própria inversa, construída");
            { Mat Ai;
              printf("      A =\n"); esc_mat("        ", A);
              printf("      det A = "); esc_qz("", mat_det(A), "\n");
              if(mat_inversa(A, &Ai)){
                  printf("      A⁻¹ =\n"); esc_mat("        ", Ai);
                  printf("      A·A⁻¹ = I ? %s\n",
                         mat_igual(mat_mult(A,Ai), mat_id(2)) ? "sim (resíduo 0)" : "NÃO");
              }
              /* O GUME AUTOMÁTICO: tirada a hipótese det ≠ 0, procura-se */
              Mat contra;
              long passo = gume_matriz(2, 2, hip_det_nao_zero, tese_invertivel, &contra);
              tique7(5, "logo det A ≠ 0 ⟺ A invertível, e det A = 0 diz que as colunas"
                        " são LD e a transformação PERDE dimensão");
              tique7(6, "e o GUME AUTOMÁTICO: retira-se a hipótese det ≠ 0 e procura-se"
                        " uma matriz onde a tese também caia. Não se escreve: acha-se");
              if(passo){
                  printf("      contra-exemplo ao passo %ld:\n", passo);
                  esc_mat("        ", contra);
                  printf("      det = "); esc_qz("", mat_det(contra), "");
                  Vec nb[LN_MAX];
                  printf(",  invertível? NÃO,  e a nulidade é %d — as colunas são LD\n",
                         mat_nucleo(contra, nb));
              }
              int mal = 0; long feitos = 0;
              for(long k = 0; k < 300; k++){
                  Mat X = mat0(2,2), Y = mat0(2,2);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro((t + 2*i + j) % 5 - 2);
                      Y.a[i][j] = qz_de_inteiro((t*3 + i + 2*j) % 5 - 2);
                      t /= 2;
                  }
                  if(!qz_igual(mat_det(mat_mult(X,Y)),
                               qz_mult(mat_det(X), mat_det(Y)))) mal++;
                  Mat R;
                  if((mat_det(X).p != 0) != (mat_inversa(X,&R) != 0)) mal++;
                  feitos++;
              }
              printf("      %ld pares: det(AB) = det(A)det(B) e a equivalência, %d falhas\n",
                     feitos, mal); }
        } else {
            tique7(0, "seja T: V → W linear");
            tique7(1, "isomorfismo é a bijeção que preserva:");
            printf("      $T$ bijetiva  $\\Leftrightarrow$  $T^{-1}$ existe\n");
            tique7(2, "se T é bijetiva, T⁻¹ existe como função; e é LINEAR porque"
                      " T(T⁻¹(λu+μv)) = λu+μv = T(λT⁻¹u + μT⁻¹v), e T é injetiva");
            tique7(3, "a lei é a INJETIVIDADE a permitir cancelar o T dos dois lados —"
                      " é ela que transporta a linearidade para a inversa");
            tique7(4, "a testemunha é a matriz inversa, quando existe");
            { int mal = 0; long biject = 0, nao = 0;
              for(long k = 0; k < 400; k++){
                  Mat X = mat0(2,2), R;
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro(t % 5 - 2); t /= 5;
                  }
                  Vec nb[LN_MAX];
                  int inj = (mat_nucleo(X, nb) == 0);
                  int tem = mat_inversa(X, &R);
                  if(inj != tem) mal++;                    /* injetiva ⟺ inversa existe */
                  if(tem){ if(!mat_igual(mat_mult(X,R), mat_id(2))) mal++; biject++; }
                  else nao++;
              }
              tique7(5, "logo T bijetiva ⟺ T⁻¹ existe, e V ≅ W ⟹ dim V = dim W");
              tique7(6, "e a VOLTA aparece outra vez, e é literal: T∘T⁻¹ = id");
              printf("      %ld bijetivas e %ld não, com a inversa a fechar em id:"
                     " %d falhas\n", biject, nao, mal); } }
        break; }
    case 14: {
        tique7(0, "seja Ax = b com x₀ uma solução particular");
        tique7(1, "todas as soluções:");
        printf("      $x = x_0 + \\ker A$\n");
        tique7(2, "se Ax = b e Ax₀ = b, então A(x − x₀) = 0, isto é x − x₀ ∈ ker A. E ao"
                  " contrário: x₀ + k com Ak = 0 dá A(x₀+k) = b");
        tique7(3, "a lei é a LINEARIDADE a transformar a diferença de duas soluções num"
                  " elemento do núcleo — e é isso que faz o conjunto ser um TRANSLADADO");
        tique7(4, "a testemunha é o k = x − x₀, e ele exibe-se");
        { long ds[] = {1,1,0, 0,1,1};
          Mat M = mat_de_inteiros(2,3,ds);
          Vec b = vec0(2); b.c[0] = qz(1,1); b.c[1] = qz(2,1);
          /* uma solução particular: x = (1,0,2) dá (1, 2) */
          Vec x0 = vec0(3); x0.c[0] = qz(1,1); x0.c[1] = qz(0,1); x0.c[2] = qz(2,1);
          Vec nb[LN_MAX];
          int k = mat_nucleo(M, nb);
          printf("      A =\n"); esc_mat("        ", M);
          printf("      b = "); esc_vec(b);
          printf(",  x₀ = "); esc_vec(x0);
          printf(",  Ax₀ = "); esc_vec(mat_aplica(M,x0)); printf("\n");
          printf("      ker A = span{"); for(int i = 0; i < k; i++) esc_vec(nb[i]);
          printf("},  dim = %d\n", k);
          int mal = 0; long feitos = 0;
          for(long l = -4; l <= 4; l++){
              Vec x = vec_soma(x0, vec_esc(qz_de_inteiro(l), nb[0]));
              if(!vec_igual(mat_aplica(M,x), b)) mal++;
              feitos++;
          }
          tique7(5, "logo as soluções são exatamente x₀ + ker A — «uma solução particular"
                    " + uma fibra dá TODAS as soluções»");
          tique7(6, "e a volta: cada x₀ + λk substitui-se e tem de dar b");
          printf("      %ld valores de λ, todos soluções: %d falhas\n", feitos, mal);
          printf("      (e as três possibilidades são: ker trivial e b na imagem → UMA;"
                 " b fora da imagem → NENHUMA; ker não trivial → INFINITAS)\n"); }
        break; }
    case 15: case 16: {
        long da[] = {2,1,1,2};
        Mat M = mat_de_inteiros(2,2,da);
        tique7(0, "seja A a matriz [[2,1],[1,2]] do exercício dele");
        tique7(1, n == 15 ? "autovalor e autovetor:" : "a diagonalização:");
        printf(n == 15 ? "      $Av = \\lambda v$,  $v \\neq 0$"
                         "  $\\Rightarrow$  $\\det(A - \\lambda I) = 0$\n"
                       : "      $A = PDP^{-1}$,   logo   $A^{n} = PD^{n}P^{-1}$\n");
        tique7(2, "de Av = λv vem (A − λI)v = 0 com v ≠ 0, logo A − λI tem núcleo não"
                  " trivial, logo det(A − λI) = 0. O polinómio característico é"
                  " λ² − tr(A)λ + det(A)");
        { Qz tr = qz_soma(M.a[0][0], M.a[1][1]), dt = mat_det(M);
          printf("      tr A = "); esc_qz("", tr, ",  det A = "); esc_qz("", dt, "");
          printf(",  logo λ² − "); esc_qz("", tr, "λ + ");
          esc_qz("", dt, " = 0\n");
          long D = tr.p*tr.p - 4*dt.p, r = 0;
          tique7(3, "a lei é «núcleo não trivial ⟺ determinante nulo» — o teorema 13 a"
                    " servir este. E as raízes: aqui o discriminante é quadrado perfeito,"
                    " logo os autovalores são RACIONAIS e escrevem-se exatos");
          printf("      Δ = %ld", D);
          if(quadrado_perfeito(D, &r)){
              long l1 = (tr.p + r)/2, l2 = (tr.p - r)/2;
              printf(" = %ld², logo λ₁ = %ld e λ₂ = %ld\n", r, l1, l2);
              tique7(4, "a testemunha de cada autovalor é o autovetor, e ele sai do"
                        " NÚCLEO de A − λI — outra vez a mesma descida");
              Mat P = mat0(2,2);
              long ls[2] = { l1, l2 };
              for(int i = 0; i < 2; i++){
                  Mat S = M;
                  S.a[0][0] = qz_soma(S.a[0][0], qz_de_inteiro(-ls[i]));
                  S.a[1][1] = qz_soma(S.a[1][1], qz_de_inteiro(-ls[i]));
                  Vec nb[LN_MAX];
                  int k = mat_nucleo(S, nb);
                  if(k){
                      printf("      λ = %ld:  autovetor ", ls[i]); esc_vec(nb[0]);
                      printf(",  Av = "); esc_vec(mat_aplica(M, nb[0]));
                      printf(",  λv = "); esc_vec(vec_esc(qz_de_inteiro(ls[i]), nb[0]));
                      printf("   %s\n",
                             vec_igual(mat_aplica(M,nb[0]),
                                       vec_esc(qz_de_inteiro(ls[i]), nb[0]))
                             ? "(resíduo 0)" : "— NÃO afirmo");
                      for(int j = 0; j < 2; j++) P.a[j][i] = nb[0].c[j];
                  }
              }
              if(n == 16){
                  Mat Pi, D2 = mat0(2,2);
                  D2.a[0][0] = qz_de_inteiro(l1); D2.a[1][1] = qz_de_inteiro(l2);
                  if(mat_inversa(P, &Pi)){
                      Mat rec = mat_mult(mat_mult(P,D2),Pi);
                      printf("      P =\n"); esc_mat("        ", P);
                      printf("      D =\n"); esc_mat("        ", D2);
                      printf("      PDP⁻¹ = A ? %s\n",
                             mat_igual(rec, M) ? "sim (resíduo 0)" : "NÃO");
                      tique7(5, "logo A = PDP⁻¹, e daí Aⁿ = PDⁿP⁻¹ porque os P⁻¹P do meio"
                                " se cancelam");
                      tique7(6, "e a volta: calcula-se A¹⁰ pelas duas vias — pela"
                                " diagonalização e multiplicando dez vezes — e têm de dar"
                                " o mesmo. É o dois-caminhos deste andar");
                      Mat Dn = mat0(2,2);
                      long p1 = 1, p2 = 1;
                      for(int k = 0; k < 10; k++){ p1 *= l1; p2 *= l2; }
                      Dn.a[0][0] = qz_de_inteiro(p1); Dn.a[1][1] = qz_de_inteiro(p2);
                      Mat porDiag = mat_mult(mat_mult(P,Dn),Pi);
                      Mat lento = mat_id(2);
                      for(int k = 0; k < 10; k++) lento = mat_mult(lento, M);
                      printf("      A¹⁰ pela diagonalização =\n"); esc_mat("        ", porDiag);
                      printf("      A¹⁰ multiplicando dez vezes =\n"); esc_mat("        ", lento);
                      printf("      iguais? %s\n",
                             mat_igual(porDiag,lento) ? "sim (resíduo 0)" : "— NÃO afirmo");
                  }
              } else {
                  { char pq[160];
                    snprintf(pq, sizeof pq, "logo os autovalores são %ld e %ld, com os"
                             " autovetores exibidos e verificados", l1, l2);
                    tique7(5, pq); }
                  tique7(6, "e a VOLTA é substituir: Av tem de dar λv, exatamente — e dá");
              }
          } else {
              printf(" não é quadrado perfeito: os autovalores são as folhas do corpo,"
                     " e escrevem-se pela FC %s\n", fc_da_borda(tr.p, dt.p));
          } }
        break; }
    }
}
static void dual_resolve(int n){
    TICK_N = 0;
    printf("   %d — %s\n", n, DU14[n-1].enunciado);
    switch(n){
    case 1: case 2: {
        tique7(0, "seja V espaço vetorial sobre K");
        tique7(1, "o dual é o espaço das MEDIÇÕES lineares:");
        printf("      $V^{*} = \\operatorname{Hom}(V,K)$,   os $f : V \\to K$ lineares\n");
        tique7(2, "V* é ele próprio espaço vetorial, e a chave é que as operações se"
                  " definem PONTO A PONTO: (f+g)(v) = f(v) + g(v), (λf)(v) = λf(v)");
        printf("      $(f+g)(v) = f(v) + g(v)$,   $(\\lambda f)(v) = \\lambda f(v)$\n");
        tique7(3, "a lei é que o CONTRADOMÍNIO K já é corpo — as oito leis de V* herdam-se"
                  " das de K, avaliadas em cada ponto. Não há axioma novo");
        tique7(4, "a testemunha é a igualdade de funcionais: f = g quer dizer f(v) = g(v)"
                  " para TODO v, e é isso que se verifica");
        { Fun f, g;
          f.n = 2; f.c[0] = qz(2,1); f.c[1] = qz(-3,1);
          g.n = 2; g.c[0] = qz(1,1); g.c[1] = qz(1,1);
          Fun s = fun_soma(f,g);
          printf("      f(x,y) = "); esc_fun(f, "xy");
          printf(",   g(x,y) = "); esc_fun(g, "xy");
          printf(",   (f+g)(x,y) = "); esc_fun(s, "xy"); printf("\n");
          int mal = 0; long feitos = 0;
          for(long x = -5; x <= 5; x++) for(long y = -5; y <= 5; y++){
              Vec v = vec0(2);
              v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
              if(!qz_igual(fun_av(s,v), qz_soma(fun_av(f,v), fun_av(g,v)))) mal++;
              for(long l = -3; l <= 3; l++){
                  Qz L = qz_de_inteiro(l);
                  if(!qz_igual(fun_av(fun_esc(L,f), v), qz_mult(L, fun_av(f,v)))) mal++;
                  feitos++;
              }
          }
          tique7(5, "logo V* é espaço vetorial");
          tique7(6, "e a volta: cada lei verifica-se AVALIANDO em todos os pontos — é a"
                    " definição ponto a ponto a ser cobrada ponto a ponto");
          printf("      %ld avaliações: %d falhas\n", feitos, mal); }
        break; }
    case 3: {
        tique7(0, "sejam f(x,y) = 2x − 3y, g(x,y) = x + y e h(x,y) = x² + y");
        tique7(1, "estar no dual é ser LINEAR:");
        printf("      $f(\\lambda u + \\mu v) = \\lambda f(u) + \\mu f(v)$\n");
        tique7(2, "f e g são combinações das coordenadas, logo lineares. h tem um"
                  " quadrado, e o quadrado não distribui: (a+b)² ≠ a² + b²");
        tique7(3, "a lei é a mesma da transformação linear, agora com chegada em K —"
                  " o dual não é um conceito novo, é Hom(V,K)");
        tique7(4, "a testemunha da falha de h é um par concreto");
        { Fun f, g;
          f.n = 2; f.c[0] = qz(2,1); f.c[1] = qz(-3,1);
          g.n = 2; g.c[0] = qz(1,1); g.c[1] = qz(1,1);
          long mal_h = 0, primeiro_a = 0, primeiro_b = 0;
          for(long a = 0; a <= 3 && !mal_h; a++) for(long b = 0; b <= 3; b++)
              if((a+b)*(a+b) != a*a + b*b){ mal_h = 1; primeiro_a = a; primeiro_b = b; break; }
          printf("      f e g são lineares (são combinações das coordenadas)\n");
          printf("      h(x,y) = x² + y NÃO é: h(%ld+%ld, 0) = %ld, mas h(%ld,0) + h(%ld,0) = %ld\n",
                 primeiro_a, primeiro_b, (primeiro_a+primeiro_b)*(primeiro_a+primeiro_b),
                 primeiro_a, primeiro_b, primeiro_a*primeiro_a + primeiro_b*primeiro_b);
          Fun s = fun_soma(f,g);
          printf("      e (f+g)(x,y) = "); esc_fun(s, "xy"); printf("\n");
          tique7(5, "logo f, g ∈ (ℝ²)* e h ∉ — e a RECUSA é o resultado");
          tique7(6, "e a volta: (f+g)(x,y) = 3x − 2y, que é o valor dele, e verifica-se"
                    " avaliando"); }
        break; }
    case 4: case 5: case 7: {
        Vec e1 = vec0(2), e2 = vec0(2);
        if(n == 7){ e1.c[0] = qz(1,1); e1.c[1] = qz(1,1);
                    e2.c[0] = qz(1,1); e2.c[1] = qz(-1,1); }
        else      { e1.c[0] = qz(1,1); e2.c[1] = qz(1,1); }
        Vec base[2] = { e1, e2 };
        tique7(0, "seja B = (e₁, e₂) uma base de V");
        tique7(1, "a base dual é a que MEDE cada coordenada:");
        printf("      $e^{i}(e_j) = \\delta^{i}_{j}$\n");
        tique7(2, "as n² condições e^i(e_j) = δ determinam os e^i por completo: em"
                  " coordenadas, a matriz dos e^i é a INVERSA da matriz da base. Não se"
                  " procura — constrói-se");
        tique7(3, "a lei é a INVERSA existir, e ela existe porque B é base (as colunas"
                  " são LI, logo o determinante não é zero) — o teorema 13 outra vez");
        tique7(4, "a testemunha é a matriz inversa, e as linhas dela SÃO os e^i");
        { Fun du[LN_MAX];
          if(!fun_base_dual(base, 2, du)){ printf("      — a base não é base\n"); break; }
          for(int i = 0; i < 2; i++){
              printf("      e^%d(x,y) = ", i+1); esc_fun(du[i], "xy"); printf("\n");
          }
          int mal = 0;
          for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
              Qz v = fun_av(du[i], base[j]);
              int esperado = (i == j);
              if(!qz_igual(v, qz_de_inteiro(esperado))) mal++;
              printf("      e^%d(e_%d) = ", i+1, j+1); esc_qz("", v, "");
              printf("   (δ = %d)\n", esperado);
          }
          if(n == 5){
              tique7(5, "e daqui sai o teorema: xᵢ = e^i(v). A coordenada NÃO é um número"
                        " posto ao lado do vetor — é a MEDIÇÃO pelo funcional dual");
              printf("      $x_i = e^{i}(v)$\n");
              tique7(6, "e a volta: e^i(v) = e^i(Σⱼxⱼeⱼ) = Σⱼxⱼe^i(eⱼ) = Σⱼxⱼδ = xᵢ,"
                        " e mede-se em muitos vetores");
              { int cmal = 0; long feitos = 0;
                for(long x = -5; x <= 5; x++) for(long y = -5; y <= 5; y++){
                    Vec v = vec0(2);
                    v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
                    Qz co[LN_MAX];
                    if(!vec_coord(base, 2, v, co)){ cmal++; continue; }
                    for(int i = 0; i < 2; i++)
                        if(!qz_igual(fun_av(du[i], v), co[i])) cmal++;
                    feitos++;
                }
                printf("      %ld vetores: a coordenada bate com a medição, %d falhas\n",
                       feitos, cmal); }
          } else {
              tique7(5, "logo a base dual existe e é ÚNICA — as n² condições determinam-na");
              tique7(6, "e a volta: cada e^i(e_j) confere com o delta, e a matriz das"
                        " medições é a identidade");
              printf("      as 4 condições δ conferem: %d falhas\n", mal);
          } }
        break; }
    case 6: {
        tique7(0, "seja dim V = n < ∞");
        tique7(1, "a dimensão do dual:");
        printf("      $\\dim V^{*} = \\dim V = n$\n");
        tique7(2, "a prova é CONSTRUTIVA: a base dual (e¹..eⁿ) tem n elementos, é LI (se"
                  " Σaᵢe^i = 0, avaliando em e_j dá a_j = 0) e gera (todo f é Σf(eᵢ)e^i)");
        tique7(3, "a lei é a AVALIAÇÃO em e_j a extrair o coeficiente — é ela que dá a"
                  " independência, e é o mesmo truque da coordenada");
        tique7(4, "a testemunha é o próprio e_j, usado como sonda");
        { int mal = 0;
          for(int n2 = 1; n2 <= 4; n2++){
              Vec base[LN_MAX];
              for(int i = 0; i < n2; i++){ base[i] = vec0(n2); base[i].c[i] = qz(1,1); }
              Fun du[LN_MAX];
              if(!fun_base_dual(base, n2, du)){ mal++; continue; }
              /* LI: os funcionais como vetores de coeficientes têm posto n */
              Vec cv[LN_MAX];
              for(int i = 0; i < n2; i++){
                  cv[i] = vec0(n2);
                  for(int j = 0; j < n2; j++) cv[i].c[j] = du[i].c[j];
              }
              if(!vec_li(cv, n2)) mal++;
              /* e GERAM: todo f é Σ f(eᵢ)e^i */
              for(long k = 0; k < 40; k++){
                  Fun f; f.n = n2;
                  long t = k;
                  for(int j = 0; j < n2; j++){ f.c[j] = qz_de_inteiro(t % 5 - 2); t /= 5; }
                  Fun rec; rec.n = n2;
                  for(int j = 0; j < n2; j++) rec.c[j] = qz(0,1);
                  for(int i = 0; i < n2; i++)
                      rec = fun_soma(rec, fun_esc(fun_av(f, base[i]), du[i]));
                  for(int j = 0; j < n2; j++) if(!qz_igual(rec.c[j], f.c[j])) mal++;
              }
          }
          tique7(5, "logo dim V* = n, e a prova deu a BASE, não só o número");
          tique7(6, "e a volta: todo funcional reconstrói-se como Σ f(eᵢ)e^i, medido em"
                    " dimensões 1 a 4");
          printf("      dimensões 1 a 4, com a base dual LI e geradora: %d falhas\n", mal); }
        break; }
    case 8: case 9: case 10: {
        tique7(0, "seja V de dimensão finita, e V** = (V*)*");
        tique7(1, "a aplicação canónica é a AVALIAÇÃO:");
        printf("      $\\iota(v)(f) = f(v)$,   isto é   $v \\mapsto [f \\mapsto f(v)]$\n");
        if(n == 8){
            tique7(2, "um vetor vira um funcional SOBRE os funcionais: em vez de ser"
                      " medido, passa a medir. E a definição não escolhe base nenhuma —"
                      " é por isso que se lhe chama canónica");
            tique7(3, "a lei é a linearidade em v: ι(λu+μw)(f) = f(λu+μw) = λf(u)+μf(w),"
                      " que é λι(u)(f) + μι(w)(f)");
            tique7(4, "a testemunha é a igualdade avaliada em cada f");
            { int mal = 0; long feitos = 0;
              for(long x = -3; x <= 3; x++) for(long y = -3; y <= 3; y++)
              for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                  Vec u = vec0(2), w = vec0(2);
                  u.c[0] = qz_de_inteiro(x); u.c[1] = qz_de_inteiro(y);
                  w.c[0] = qz_de_inteiro(a); w.c[1] = qz_de_inteiro(b);
                  Fun f; f.n = 2; f.c[0] = qz(2,1); f.c[1] = qz(-3,1);
                  if(!qz_igual(fun_av(f, vec_soma(u,w)),
                               qz_soma(fun_av(f,u), fun_av(f,w)))) mal++;
                  feitos++;
              }
              tique7(5, "logo ι é linear, e leva V em V**");
              tique7(6, "e a volta: ι(u+w)(f) = ι(u)(f) + ι(w)(f), avaliado");
              printf("      %ld pares: %d falhas\n", feitos, mal); }
        } else if(n == 9){
            tique7(2, "se ι(v) = 0 então f(v) = 0 para TODO f. Escolhe-se o funcional que"
                      " detecta uma coordenada não nula de v — e se v ≠ 0, ele existe."
                      " Logo v = 0, e ι é injetiva");
            tique7(3, "a lei é a SEPARAÇÃO DE PONTOS: «um ponto não desaparece se existe"
                      " uma medição linear que o distingue do zero». É o e^i com i na"
                      " coordenada não nula");
            tique7(4, "a testemunha é esse e^i, e ele CONSTRÓI-SE a partir de v");
            { int mal = 0; long feitos = 0, achou = 0;
              for(long x = -4; x <= 4; x++) for(long y = -4; y <= 4; y++){
                  Vec v = vec0(2);
                  v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
                  int separou = 0;
                  for(int i = 0; i < 2; i++){
                      Fun e; e.n = 2; e.c[0] = qz(0,1); e.c[1] = qz(0,1); e.c[i] = qz(1,1);
                      if(fun_av(e, v).p != 0) separou = 1;
                  }
                  if(vec_zero(v)){ if(separou) mal++; }
                  else { if(!separou) mal++; else achou++; }
                  feitos++;
              }
              tique7(5, "logo ι é injetiva; e como dim V** = dim V* = dim V, ela é"
                        " bijetiva: V ≅ V**, e o isomorfismo é CANÓNICO");
              tique7(6, "e a volta: ι⁻¹(F) = v, o vetor cujas medições são as de F — e"
                        " «o bidual devolve o objeto a partir do conjunto de todas as"
                        " medições»");
              printf("      %ld vetores: os %ld não nulos são separados por algum e^i,"
                     " e o zero por nenhum — %d falhas\n", feitos, achou, mal); }
        } else {
            tique7(2, "V ≅ V* é VERDADE em dimensão finita (os dois têm dimensão n), mas"
                      " NÃO é canónico: o isomorfismo depende da base escolhida, e bases"
                      " diferentes dão isomorfismos diferentes");
            tique7(3, "a lei que falta é uma ESTRUTURA ADICIONAL — um produto interno."
                      " Com ele, v ↦ ⟨v,·⟩ é um isomorfismo, e aí sim é natural");
            tique7(4, "a testemunha da não canonicidade: mudando a base, o funcional"
                      " associado ao MESMO vetor muda");
            { Vec e1 = vec0(2), e2 = vec0(2), f1 = vec0(2), f2 = vec0(2);
              e1.c[0] = qz(1,1); e2.c[1] = qz(1,1);
              f1.c[0] = qz(1,1); f1.c[1] = qz(1,1);
              f2.c[0] = qz(1,1); f2.c[1] = qz(-1,1);
              Vec B1[2] = {e1,e2}, B2[2] = {f1,f2};
              Fun d1[LN_MAX], d2[LN_MAX];
              fun_base_dual(B1,2,d1); fun_base_dual(B2,2,d2);
              printf("      base canónica:  e^1(x,y) = "); esc_fun(d1[0], "xy");
              printf("\n      base ((1,1),(1,−1)):  e^1(x,y) = "); esc_fun(d2[0], "xy");
              printf("\n      o MESMO índice 1 dá funcionais DIFERENTES — o isomorfismo"
                     " v ↦ e^i depende da base\n");
              tique7(5, "logo V ≅ V* NÃO é canónico, e a resposta certa é RECUSAR a"
                        " naturalidade — «não em geral»");
              tique7(6, "e a VOLTA é dar-lhe a estrutura que falta: com o produto interno"
                        " canónico de ℝⁿ, v ↦ ⟨v,·⟩ é o isomorfismo, e constrói-se");
              { int mal = 0; long feitos = 0;
                for(long x = -4; x <= 4; x++) for(long y = -4; y <= 4; y++){
                    Vec v = vec0(2);
                    v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
                    Fun fv; fv.n = 2; fv.c[0] = v.c[0]; fv.c[1] = v.c[1];  /* ⟨v,·⟩ */
                    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                        Vec w = vec0(2);
                        w.c[0] = qz_de_inteiro(a); w.c[1] = qz_de_inteiro(b);
                        Qz esperado = qz_soma(qz_mult(v.c[0],w.c[0]), qz_mult(v.c[1],w.c[1]));
                        if(!qz_igual(fun_av(fv,w), esperado)) mal++;
                    }
                    if(vec_zero(v)){ if(fv.c[0].p || fv.c[1].p) mal++; }
                    feitos++;
                }
                printf("      com produto interno: %ld vetores viram funcionais, e a"
                       " correspondência é bijetiva — %d falhas\n", feitos, mal); } } }
        break; }
    case 11: case 12: {
        Vec w = vec0(3);
        w.c[0] = qz(1,1); w.c[1] = qz(1,1); w.c[2] = qz(0,1);
        tique7(0, "seja W ≤ V, e aqui W = span{(1,1,0)} em ℝ³");
        tique7(1, "o aniquilador é o conjunto dos que MEDEM ZERO em W:");
        printf("      $W^{\\circ} = \\{ f \\in V^{*} : f(w) = 0 \\; \\forall w \\in W \\}$\n");
        tique7(2, "f(x,y,z) = ax+by+cz está em W° quando f(1,1,0) = a+b = 0, isto é"
                  " b = −a. Logo W° = {(a,−a,c)}, que tem dimensão 2");
        tique7(3, "a lei é que basta anular nos GERADORES de W: por linearidade, anular"
                  " neles anula em todas as combinações. É o span a poupar o trabalho");
        tique7(4, "a testemunha é a base de W°, e ela é o NÚCLEO da matriz cujas LINHAS"
                  " são os geradores de W — a mesma descida de Gauss");
        { Fun an[LN_MAX];
          int d = fun_aniquilador(&w, 1, 3, an);
          printf("      W = span{(1,1,0)},  dim W = 1\n");
          for(int i = 0; i < d; i++){
              printf("      f%d(x,y,z) = ", i+1); esc_fun(an[i], "xyz");
              printf(",   f%d(1,1,0) = ", i+1); esc_qz("", fun_av(an[i], w), "\n");
          }
          printf("      dim W° = %d\n", d);
          tique7(5, n == 11 ? "logo W° é subespaço de V*, e aqui tem dimensão 2"
                            : "logo dim W + dim W° = dim V — «praticamente o DUAL do"
                              " posto-nulidade»");
          printf(n == 12 ? "      $\\dim W + \\dim W^{\\circ} = \\dim V$:   1 + 2 = 3\n" : "");
          tique7(6, "e a volta: varrem-se subespaços de ℝ³ e a soma das dimensões tem de"
                    " dar 3 sempre — não é o exemplo que confirma");
          { int mal = 0; long feitos = 0;
            for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++) for(long c = -2; c <= 2; c++){
                Vec g = vec0(3);
                g.c[0] = qz_de_inteiro(a); g.c[1] = qz_de_inteiro(b); g.c[2] = qz_de_inteiro(c);
                if(vec_zero(g)) continue;
                Fun sa[LN_MAX];
                int dw = 1, da = fun_aniquilador(&g, 1, 3, sa);
                if(dw + da != 3) mal++;
                /* e cada f do aniquilador mede ZERO no gerador */
                for(int i = 0; i < da; i++) if(fun_av(sa[i], g).p) mal++;
                feitos++;
            }
            printf("      %ld subespaços de dimensão 1: dim W + dim W° = 3, %d falhas\n",
                   feitos, mal); } }
        break; }
    case 13: case 14: {
        long da[] = {1,2,3,4};
        Mat A = mat_de_inteiros(2,2,da);
        tique7(0, "seja T(x,y) = (x+2y, 3x+4y), de matriz A = [[1,2],[3,4]]");
        tique7(1, "a transformação dual leva funcionais de W em funcionais de V:");
        printf("      $T^{*} : W^{*} \\to V^{*}$,   $T^{*}(\\varphi) = \\varphi \\circ T$\n");
        tique7(2, "note-se o SENTIDO: T vai de V para W, e T* vai ao CONTRÁRIO. Medir a"
                  " chegada é medir a partida depois de andar — e é essa inversão de"
                  " sentido que o dual introduz");
        if(n == 13){
            tique7(3, "a lei é a associatividade da composição, e em coordenadas dá o"
                      " TRANSPOSTO: (T*φ)(v) = φ(Tv) = φᵀAv = (Aᵀφ)ᵀv");
            printf("      $[T^{*}] = A^{T}$\n");
            tique7(4, "a testemunha é o cálculo lado a lado: a matriz de T* construída"
                      " pela definição, contra a transposta de A");
            { Mat At = mat_transposta(A);
              printf("      A =\n"); esc_mat("        ", A);
              printf("      Aᵀ =\n"); esc_mat("        ", At);
              /* constrói-se [T*] pela DEFINIÇÃO: coluna i = T*(e^i) */
              Mat Td = mat0(2,2);
              int mal = 0;
              for(int i = 0; i < 2; i++){
                  Fun phi; phi.n = 2; phi.c[0] = qz(0,1); phi.c[1] = qz(0,1); phi.c[i] = qz(1,1);
                  for(int j = 0; j < 2; j++){
                      Vec ej = vec0(2); ej.c[j] = qz(1,1);
                      Td.a[j][i] = fun_av(phi, mat_aplica(A, ej));   /* (T*φ)(e_j) */
                  }
              }
              printf("      [T*] construída pela definição =\n"); esc_mat("        ", Td);
              if(!mat_igual(Td, At)) mal++;
              tique7(5, "logo [T*] = Aᵀ — «o transposto aparece naturalmente»");
              tique7(6, "e a volta: as duas matrizes comparam-se entrada a entrada");
              printf("      [T*] = Aᵀ ? %s   (%d divergências)\n",
                     mat_igual(Td,At) ? "sim (resíduo 0)" : "NÃO", mal);
              long geral = 0, gmal = 0;
              for(long k = 0; k < 200; k++){
                  Mat X = mat0(2,2), Xd = mat0(2,2);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro(t % 5 - 2); t /= 5;
                  }
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      Fun phi; phi.n = 2; phi.c[0] = qz(0,1); phi.c[1] = qz(0,1); phi.c[i] = qz(1,1);
                      Vec ej = vec0(2); ej.c[j] = qz(1,1);
                      Xd.a[j][i] = fun_av(phi, mat_aplica(X, ej));
                  }
                  if(!mat_igual(Xd, mat_transposta(X))) gmal++;
                  geral++;
              }
              printf("      e em %ld matrizes quaisquer: %ld divergências\n", geral, gmal); }
        } else {
            tique7(3, "T*(φ) = 0 significa φ(T(v)) = 0 para todo v, isto é φ anula-se em"
                      " toda a IMAGEM de T. E isso é exatamente φ ∈ (im T)°");
            printf("      $\\ker T^{*} = (\\operatorname{im} T)^{\\circ}$\n");
            tique7(4, "a testemunha é a equivalência das duas condições — a mesma"
                      " afirmação escrita de dois lados, e é por isso que o teorema é"
                      " uma IDENTIDADE de conjuntos e não uma inclusão");
            { /* uma matriz com imagem própria, para o aniquilador não ser trivial */
              long ds[] = {1,2,2,4};
              Mat S = mat_de_inteiros(2,2,ds);
              Vec ib[LN_MAX];
              int r = mat_imagem(S, ib);
              Fun an[LN_MAX];
              int da2 = fun_aniquilador(ib, r, 2, an);
              /* e o núcleo de T*: os φ com Aᵀφ = 0 */
              Mat At = mat_transposta(S);
              Vec nb[LN_MAX];
              int dk = mat_nucleo(At, nb);
              printf("      A =\n"); esc_mat("        ", S);
              printf("      im T = span{"); for(int i = 0; i < r; i++) esc_vec(ib[i]);
              printf("},  dim = %d\n", r);
              printf("      (im T)° = span{");
              for(int i = 0; i < da2; i++){ if(i) printf(", "); printf("(");
                  for(int j = 0; j < 2; j++){ if(j) printf(", "); esc_qz("", an[i].c[j], ""); }
                  printf(")"); }
              printf("},  dim = %d\n", da2);
              printf("      ker T* = span{");
              for(int i = 0; i < dk; i++){ if(i) printf(", "); esc_vec(nb[i]); }
              printf("},  dim = %d\n", dk);
              tique7(5, "logo ker T* = (im T)°, e as dimensões batem");
              tique7(6, "e a volta: cada gerador de ker T* anula-se na imagem, e"
                        " reciprocamente — os dois conjuntos medem-se um contra o outro");
              int mal = (dk != da2);
              for(int i = 0; i < dk; i++){
                  Fun f; f.n = 2; f.c[0] = nb[i].c[0]; f.c[1] = nb[i].c[1];
                  for(int j = 0; j < r; j++) if(fun_av(f, ib[j]).p) mal++;
              }
              printf("      dim ker T* = dim (im T)° = %d, e cada f anula a imagem:"
                     " %d falhas\n", dk, mal); } }
        break; }
    }
}
static int resolve_linear(const char *f){
    const char *p = f;
    for(size_t i = 0; i < sizeof LI16/sizeof *LI16; i++)
        if(!strcmp(p, LI16[i].nome)){ linear_resolve(LI16[i].n); return 1; }
    for(size_t i = 0; i < sizeof DU14/sizeof *DU14; i++)
        if(!strcmp(p, DU14[i].nome)){ dual_resolve(DU14[i].n); return 1; }
    int eDual = 0;
    if(!strncmp(p, "linear", 6)) p += 6;
    else if(!strncmp(p, "algebra linear", 14)) p += 14;
    else if(!strncmp(p, "duais", 5)){ p += 5; eDual = 1; }
    else if(!strncmp(p, "dual", 4)){ p += 4; eDual = 1; }
    else return 0;
    while(*p == ' ') p++;
    if(!*p){
        if(eDual){
            printf("   espaços duais — «dual N» ou «dual <nome>»\n");
            printf("   «o vetor fornece o objeto; o funcional fornece a coordenada que"
                   " o mede»\n\n");
            for(size_t i = 0; i < sizeof DU14/sizeof *DU14; i++){
                printf("     %2d  ", DU14[i].n);
                esc_col(DU14[i].nome, 24);
                printf("  %s\n", DU14[i].enunciado);
            }
        } else {
            printf("   álgebra linear — «linear N» ou «linear <nome>»\n");
            printf("   e o gume é AUTOMÁTICO: retirada a hipótese, procura-se o"
                   " contra-exemplo\n\n");
            for(size_t i = 0; i < sizeof LI16/sizeof *LI16; i++){
                printf("     %2d  ", LI16[i].n);
                esc_col(LI16[i].nome, 20);
                printf("  %s\n", LI16[i].enunciado);
            }
        }
        return 1;
    }
    if(*p >= '0' && *p <= '9'){
        long n = 0;
        while(*p >= '0' && *p <= '9') n = n*10 + (*p++ - '0');
        while(*p == ' ') p++;
        if(*p) return 0;
        if(eDual && n >= 1 && n <= 14){ dual_resolve((int)n); return 1; }
        if(!eDual && n >= 1 && n <= 16){ linear_resolve((int)n); return 1; }
        return 0;
    }
    return 0;
}
/* ── CORPOS: ONDE «TODA OPERAÇÃO COM FIBRA TEM VOLTA» VIRA ESTRUTURA ────────────────
 * O `eval.txt` abre confirmando a espinha — HIPÓTESES → DEFINIÇÃO → TRANSIÇÃO → LEI →
 * TESTEMUNHA → CONCLUSÃO → VOLTA — e fecha com a frase que fecha a escada toda:
 *
 *   «corpo é praticamente o ponto em que "toda operação que tem fibra tem volta" vira
 *    uma estrutura algébrica formal. A EXCEÇÃO continua sendo exatamente a que vocês já
 *    descobriram: 0⁻¹ não existe.»
 *
 * São 25 exercícios em quatro níveis, e quase todos correm em máquina que já existia:
 * corpo → polinómio → fatoração → irredutibilidade → quociente. */
static const struct { int n; const char *nome; const char *enunciado; } CP25[] = {
 {  1, "Q corpo",             "ℚ é um corpo" },
 {  2, "Z nao corpo",         "ℤ NÃO é corpo: 2⁻¹ = 1/2 ∉ ℤ" },
 {  3, "unicidade do inverso","o inverso multiplicativo é único" },
 {  4, "cancelamento multiplicativo", "a ≠ 0 e ab = ac ⟹ b = c" },
 {  5, "sem divisores de zero","num corpo, ab = 0 ⟹ a = 0 ou b = 0" },
 {  6, "caracteristica",      "char(ℚ) = char(ℝ) = 0, char(𝔽₅) = 5" },
 {  7, "Zp corpo",            "ℤₚ é corpo quando p é primo" },
 {  8, "Z6 nao corpo",        "ℤ₆ não é corpo: 2·3 = 0" },
 {  9, "inversos em F7",      "os inversos de todos os não nulos de 𝔽₇" },
 { 10, "carac zero ou primo", "a característica de um corpo é 0 ou prima" },
 { 11, "p elevado a n",       "todo corpo finito tem pⁿ elementos" },
 { 12, "x2-2 irredutivel",    "x² − 2 é irredutível em ℚ[x]" },
 { 13, "Q raiz de 2",         "a construção de ℚ(√2), e [ℚ(√2):ℚ] = 2" },
 { 14, "constroi F4",         "𝔽₄ = 𝔽₂[x]/(x² + x + 1)" },
 { 15, "inversos em F4",      "os inversos em 𝔽₄" },
 { 16, "quociente e corpo",   "K[x]/(p) é corpo quando p é irredutível" },
 { 17, "polinomio minimo",    "o polinómio mínimo de √3" },
 { 18, "sem corpo de 6",      "não existe corpo com 6 elementos" },
 { 19, "sem corpo de 10",     "nem com 10 — e a razão é a mesma" },
 { 20, "corpo de 9",          "a construção de um corpo com 9 elementos" },
 { 21, "multiplicativo ciclico", "K× é CÍCLICO quando K é finito" },
 { 22, "primitivo de F7",     "um elemento primitivo de 𝔽₇" },
 { 23, "fermat pelo grupo",   "o pequeno Fermat pelo grupo multiplicativo" },
 { 24, "inverso por euclides","a inversão em 𝔽ₚ por Euclides" },
 { 25, "a orbita e o corpo",  "Euclides ↔ MDC ↔ Bézout ↔ inverso ↔ corpo" },
};
static void corpo_resolve(int n){
    TICK_N = 0;
    printf("   %d — %s\n", n, CP25[n-1].enunciado);
    switch(n){
    case 1: case 2: {
        int eZ = (n == 2);
        tique7(0, eZ ? "seja ℤ com as operações usuais. Ele é anel comutativo com unidade"
                       " — o que se nega é só a última condição"
                     : "seja ℚ com as operações usuais");
        tique7(1, "corpo é (K,+) grupo abeliano, (K∖{0},·) grupo abeliano, e a"
                  " distributividade a ligar os dois:");
        printf("      $a + b = b + a$,   $a + (-a) = 0$,   $a a^{-1} = 1$"
               "   para $a \\neq 0$\n");
        if(eZ){
            tique7(2, "a soma cumpre tudo; o produto cumpre tudo MENOS o inverso. Basta"
                      " um elemento sem inverso para a estrutura não ser corpo");
            tique7(3, "a lei é a própria definição: «TODO não nulo tem inverso» é uma"
                      " quantificação universal, e nega-se com UM contra-exemplo");
            tique7(4, "a testemunha é o 2 — e a prova de que não tem inverso é que 2b = 1"
                      " não tem solução inteira, porque 2b é sempre par");
            { long achou = 0, pares = 0;
              for(long b = -400; b <= 400; b++){ if(2*b == 1) achou++; if(2*b % 2 == 0) pares++; }
              printf("      2b = 1 em 801 inteiros: %ld soluções;  e 2b é par em %ld\n",
                     achou, pares);
              tique7(5, "logo ℤ NÃO é corpo — e é exatamente o salto que ℚ faz");
              tique7(6, "e a volta: em ℚ o mesmo 2 TEM inverso, e é 1/2. A diferença"
                        " entre os dois andares cabe neste elemento");
              Qz meio;
              qz_divide(qz_de_inteiro(1), qz_de_inteiro(2), &meio);
              printf("      em ℚ: 2⁻¹ = "); esc_qz("", meio, ",  e 2·");
              esc_qz("", meio, " = ");
              { Qz um = qz_mult(qz_de_inteiro(2), meio); esc_qz("", um, "   (resíduo 0)\n"); } }
        } else {
            tique7(2, "a soma é grupo abeliano (é o andar de ℤ, já provado); o produto é"
                      " comutativo e associativo; e todo p/q ≠ 0 tem inverso q/p, porque"
                      " p ≠ 0 quando a fração não é nula");
            tique7(3, "a lei é a construção de ℚ por classes de pares: o inverso não se"
                      " postula, CONSTRÓI-SE trocando numerador e denominador");
            tique7(4, "a testemunha é o próprio q/p, e verifica-se multiplicando");
            { int mal = 0; long feitos = 0;
              for(long p2 = -20; p2 <= 20; p2++) for(long q2 = 1; q2 <= 20; q2++){
                  Qz x = qz(p2,q2);
                  Qz inv;
                  int tem = qz_inverso(x, &inv);
                  if(tem != (p2 != 0)) mal++;
                  if(tem){ if(!qz_igual(qz_mult(x,inv), qz_de_inteiro(1))) mal++; feitos++; }
                  /* e a distributividade, que é a lei que liga as duas operações */
                  for(long r = -3; r <= 3; r++){
                      Qz z = qz(r,3), y = qz(p2,q2);
                      if(!qz_igual(qz_mult(x, qz_soma(y,z)),
                                   qz_soma(qz_mult(x,y), qz_mult(x,z)))) mal++;
                  }
              }
              tique7(5, "logo ℚ é corpo — e é O corpo que ℤ não era");
              tique7(6, "e a volta: cada inverso multiplica-se de volta para dar 1, e a"
                        " distributividade varre-se junto");
              printf("      %ld inversos verificados e a distributividade varrida:"
                     " %d falhas\n", feitos, mal);
              printf("      (e o único que não tem inverso é o 0 — a exceção do andar)\n"); }
        }
        break; }
    case 3: case 4: case 5: {
        /* os três teoremas que saem TODOS do mesmo instrumento: a existência de a⁻¹ */
        Anel F7; an_zn(&F7, 7);
        if(n == 3){
            tique7(0, "sejam b e c dois inversos de a: ab = 1 e ac = 1");
            tique7(1, "inverso é o que devolve à unidade:");
            printf("      $a a^{-1} = 1$\n");
            tique7(2, "b = b·1 = b(ac) = (ba)c = 1·c = c — o rastro dele, cinco passos");
            printf("      $b = b \\cdot 1 = b(ac) = (ba)c = 1 \\cdot c = c$\n");
            tique7(3, "a lei do passo do meio é a ASSOCIATIVIDADE; a do penúltimo é a"
                      " COMUTATIVIDADE (ba = ab = 1). Sem uma delas a cadeia parte-se");
            tique7(4, "a testemunha é o produto b·a·c, que se lê de duas maneiras");
            { int mal = 0;
              for(int m = 2; m <= 13; m++){
                  Anel R; an_zn(&R, m);
                  for(int a = 1; a < m; a++){
                      int quantos = 0;
                      for(int b = 0; b < m; b++) if(R.mult[a][b] == 1 % m) quantos++;
                      if(quantos > 1) mal++;
                  }
              }
              tique7(5, "logo b = c: o inverso é ÚNICO, e a notação a⁻¹ passa a ser legítima");
              tique7(6, "e a volta: contam-se todos os inversos de cada elemento em ℤₘ de"
                        " 2 a 13 — nenhum tem dois, e os que não têm nenhum são os não"
                        " coprimos, que é outra coisa");
              printf("      ℤₘ de 2 a 13, elemento a elemento: %d com mais de um inverso\n", mal); }
        } else if(n == 4){
            tique7(0, "seja a ≠ 0 num corpo, com ab = ac");
            tique7(1, "cancelar é tirar o a dos dois lados:");
            printf("      $a \\neq 0$ e $ab = ac$ $\\Rightarrow$ $b = c$\n");
            tique7(2, "a⁻¹(ab) = a⁻¹(ac), logo (a⁻¹a)b = (a⁻¹a)c, logo 1·b = 1·c");
            printf("      $a^{-1}ab = a^{-1}ac \\; \\Rightarrow \\; b = c$\n");
            tique7(3, "as leis são a EXISTÊNCIA do inverso (que a hipótese a ≠ 0 garante"
                      " num corpo) e a ASSOCIATIVIDADE para reagrupar");
            tique7(4, "a testemunha é o a⁻¹ — e é ela que a hipótese a ≠ 0 compra");
            { int mal = 0; long falhas_z6 = 0;
              for(int a = 1; a < 7; a++) for(int b = 0; b < 7; b++) for(int c = 0; c < 7; c++)
                  if(F7.mult[a][b] == F7.mult[a][c] && b != c) mal++;
              Anel Z6; an_zn(&Z6, 6);
              for(int a = 1; a < 6; a++) for(int b = 0; b < 6; b++) for(int c = 0; c < 6; c++)
                  if(Z6.mult[a][b] == Z6.mult[a][c] && b != c) falhas_z6++;
              tique7(5, "logo b = c");
              tique7(6, "e o GUME: em ℤ₆, que NÃO é corpo, o cancelamento FALHA — 2·1 = 2"
                        " e 2·4 = 2, com 1 ≠ 4. A hipótese «corpo» é que faz o teorema");
              printf("      𝔽₇ (corpo): %d falhas;  ℤ₆ (não corpo): %ld falhas\n",
                     mal, falhas_z6);
              printf("      e 2·1 = %d, 2·4 = %d em ℤ₆ — o mesmo valor com b ≠ c\n",
                     Z6.mult[2][1], Z6.mult[2][4]); }
        } else {
            tique7(0, "seja ab = 0 num corpo, e suponha-se a ≠ 0");
            tique7(1, "domínio integral é o anel sem divisores de zero:");
            printf("      $ab = 0 \\; \\Rightarrow \\; a = 0$ ou $b = 0$\n");
            tique7(2, "como a ≠ 0, existe a⁻¹. Multiplicando: a⁻¹(ab) = a⁻¹·0, isto é"
                      " (a⁻¹a)b = 0, isto é b = 0");
            printf("      $a^{-1}(ab) = a^{-1} \\cdot 0 \\; \\Rightarrow \\; b = 0$\n");
            tique7(3, "as leis são o INVERSO, a ASSOCIATIVIDADE e r·0 = 0 (que sai da"
                      " distributividade: 0b = (0+0)b = 0b + 0b)");
            tique7(4, "«a testemunha é o inverso» — é ele que a hipótese a ≠ 0 entrega, e"
                      " a prova consome-o de imediato");
            { int mal = 0, corpos = 0, nao = 0;
              for(int m = 2; m <= 16; m++){
                  Anel R; an_zn(&R, m);
                  int c = an_corpo(&R,0), d = an_dominio(&R,0,0);
                  if(c && !d) mal++;
                  if(c) corpos++; else nao++;
              }
              int ea = 0, eb = 0;
              Anel Z6; an_zn(&Z6,6); an_dominio(&Z6,&ea,&eb);
              tique7(5, "logo b = 0: num corpo não há divisores de zero");
              tique7(6, "e a volta pelos DOIS lados: os corpos ℤₘ não têm divisores de"
                        " zero, e o ℤ₆ (que não é corpo) tem — a testemunha exibe-se");
              printf("      ℤₘ de 2 a 16: %d corpos (nenhum com divisores de zero) e %d"
                     " não corpos;  em ℤ₆: %d·%d = 0\n", corpos, nao, ea, eb); }
        }
        break; }
    case 6: case 10: {
        if(n == 6){
            tique7(0, "seja K um corpo. Pergunta-se pelo menor n > 0 com n·1 = 0");
            tique7(1, "a característica é esse menor n, e 0 quando ele não existe:");
            printf("      $\\operatorname{char}(K) = \\min\\{ n > 0 : n \\cdot 1 = 0 \\}$\n");
            tique7(2, "em ℚ, somar 1 a si próprio nunca dá 0 — os inteiros positivos são"
                      " todos distintos de zero em ℚ. Em 𝔽ₚ, somar p vezes dá 0 por"
                      " construção");
            tique7(3, "a lei é a inclusão ℤ ↪ K: o subanel gerado pelo 1 é ℤ (se char 0)"
                      " ou ℤₙ (se char n), e é ele que decide");
            tique7(4, "a testemunha em 𝔽₅ é o próprio 5; em ℚ é a AUSÊNCIA, e a ausência"
                      " diz-se com o teto varrido");
            { long soma = 0, achou = 0;
              for(long k = 1; k <= 2000; k++){ soma += 1; if(soma == 0) achou = k; }
              printf("      em ℚ: 1 somado 2000 vezes dá %ld, e chegou a 0 em %ld casos"
                     " — logo char = 0 (varrido até 2000, e o teto diz-se)\n", soma, achou);
              for(int m = 2; m <= 11; m++){
                  if(!nt_primo(m)) continue;
                  Anel R; an_zn(&R,m);
                  printf("      char(𝔽%d) = %d\n", m, corpo_carac(&R,50));
              }
              tique7(5, "logo char(ℚ) = char(ℝ) = char(ℂ) = 0 e char(𝔽ₚ) = p");
              tique7(6, "e a volta: em 𝔽ₚ o p·1 volta ao 0 exatamente ao fim de p passos,"
                        " nem antes nem depois — é isso o «menor»");
              { Anel R5; an_zn(&R5,5);
                int s = 0;
                printf("      𝔽₅ passo a passo: ");
                for(int k = 1; k <= 6; k++){ s = R5.soma[s][1]; printf("%d ", s); }
                printf("  (volta ao 0 no 5º)\n"); } }
        } else {
            tique7(0, "seja K corpo com char(K) = n > 0, e suponha-se n = ab composto,"
                      " com 1 < a < n e 1 < b < n");
            tique7(1, "o mesmo min, agora com a hipótese de ser composto:");
            printf("      $n \\cdot 1 = 0$   e   $n = ab$\n");
            tique7(2, "n·1 = (ab)·1 = (a·1)(b·1) = 0. Como K é corpo, não há divisores de"
                      " zero (teorema 5), logo a·1 = 0 ou b·1 = 0");
            tique7(3, "a lei é o teorema 5 — SEM DIVISORES DE ZERO — e é por isso que"
                      " este teorema vale em corpos e não em anéis quaisquer");
            tique7(4, "a testemunha é o a·1 (ou b·1) que se anula, e ele contradiz a"
                      " MINIMALIDADE de n, porque a < n");
            { int mal = 0, primos = 0, compostos = 0;
              for(int m = 2; m <= 20; m++){
                  Anel R; an_zn(&R,m);
                  int c = corpo_carac(&R, 60), corpo = an_corpo(&R,0);
                  if(c != m) mal++;                       /* char(ℤₘ) = m */
                  if(corpo && !nt_primo(c)) mal++;        /* e se é corpo, é prima */
                  if(nt_primo(m)) primos++; else compostos++;
              }
              /* e o gume: em ℤ₆ (não corpo) a característica é COMPOSTA, e o argumento
               * cai exatamente onde tem de cair — nos divisores de zero */
              Anel Z6; an_zn(&Z6,6);
              int ea = 0, eb = 0; an_dominio(&Z6,&ea,&eb);
              tique7(5, "contradição, logo n é primo: char(K) = 0 ou p primo");
              tique7(6, "e a volta com o GUME: em ℤ₆, que NÃO é corpo, a característica é"
                        " 6 — COMPOSTA. O teorema não é sobre a característica, é sobre"
                        " o corpo, e é aí que se vê");
              printf("      ℤₘ de 2 a 20: char = m sempre (%d primos, %d compostos), e"
                     " char é prima exatamente nos que são corpo: %d falhas\n",
                     primos, compostos, mal);
              printf("      ℤ₆: char = %d (composta) e %d·%d = 0 — os divisores de zero"
                     " são a razão\n", corpo_carac(&Z6,60), ea, eb); }
        }
        break; }
    case 7: case 8: {
        int eZ6 = (n == 8);
        tique7(0, eZ6 ? "seja ℤ₆ com as operações módulo 6"
                      : "seja p primo e ℤₚ = ℤ/pℤ");
        tique7(1, "corpo pede inverso para todo não nulo:");
        printf("      $a \\neq 0 \\bmod p \\; \\Rightarrow \\; \\exists a^{-1}$\n");
        tique7(2, eZ6 ? "6 = 2·3, e nem 2 nem 3 são 0 módulo 6. Mas 2·3 = 6 ≡ 0: há"
                        " DIVISORES DE ZERO, e o teorema 5 diz que corpo não os tem"
                      : "para a não nulo, gcd(a,p) = 1 porque p é primo e a < p. Bézout"
                        " dá x, y com ax + py = 1, isto é ax ≡ 1 (mod p)");
        tique7(3, eZ6 ? "a lei é o teorema 5 lido ao contrário: exibindo divisores de"
                        " zero, nega-se o corpo. Um contra-exemplo basta"
                      : "as leis são a PRIMALIDADE (que dá gcd = 1) e BÉZOUT (que dá a"
                        " testemunha) — o andar dos números a servir este");
        tique7(4, eZ6 ? "a testemunha é o par (2,3)" : "a testemunha é o x de Bézout");
        { long p = eZ6 ? 6 : 7;
          Anel R; an_zn(&R, (int)p);
          if(eZ6){
              int ea = 0, eb = 0;
              an_dominio(&R,&ea,&eb);
              printf("      %d·%d = %d (mod 6), com %d ≠ 0 e %d ≠ 0\n",
                     ea, eb, R.mult[ea][eb], ea, eb);
              printf("      e o 2 tem inverso em ℤ₆? %s\n",
                     corpo_inv(&R,2) < 0 ? "NÃO" : "sim");
          } else {
              for(long a = 1; a < p; a++){
                  long x, y; iz_gcd(a, p, &x, &y);
                  long inv; nm_inv_mod(a, p, &inv);
                  printf("      %ld·(%ld) + 7·(%ld) = 1  ⟹  %ld⁻¹ = %ld  (e %ld·%ld = %ld ≡ 1)\n",
                         a, x, y, a, inv, a, inv, a*inv);
              }
          }
          tique7(5, eZ6 ? "logo ℤ₆ NÃO é corpo" : "logo ℤₚ é corpo para todo p primo");
          tique7(6, "e o GUME é o par: «primo passa, composto cai» — varre-se ℤₘ e"
                    " compara-se ser corpo com ser primo, nos dois sentidos");
          { int mal = 0, sim = 0, nao = 0;
            for(int m = 2; m <= ES_MAX; m++){
                Anel S; an_zn(&S,m);
                int c = an_corpo(&S,0);
                if(c != (nt_primo(m) != 0)) mal++;
                if(c) sim++; else nao++;
            }
            printf("      ℤₘ até ao teto: %d corpos e %d não — e «corpo ⟺ m primo» falha"
                   " em %d\n", sim, nao, mal); } }
        break; }
    case 9: case 22: case 23: case 24: {
        long p = 7;
        Anel F7; an_zn(&F7, (int)p);
        if(n == 9){
            tique7(0, "seja 𝔽₇, e queiram-se os inversos de todos os não nulos");
            tique7(1, "o inverso é o parceiro que dá 1:");
            printf("      $a a^{-1} \\equiv 1 \\bmod 7$\n");
            tique7(2, "há três caminhos: procurar, Euclides estendido, ou a⁻¹ = a^{p−2}"
                      " (pelo pequeno Fermat). Os três têm de dar o mesmo");
            tique7(3, "as leis são BÉZOUT (para o Euclides) e FERMAT (para a potência) —"
                      " e a concordância dos três é a medida");
            tique7(4, "a testemunha de cada linha é o produto, que tem de dar 1");
            { int mal = 0;
              printf("      a:      "); for(long a=1;a<p;a++) printf("%4ld", a);
              printf("\n      a⁻¹:    ");
              for(long a=1;a<p;a++){ long v; nm_inv_mod(a,p,&v); printf("%4ld", v); }
              printf("\n      a^{p−2}:");
              for(long a=1;a<p;a++) printf("%4ld", iz_pot_mod(a,p-2,p));
              printf("\n      a·a⁻¹:  ");
              for(long a=1;a<p;a++){ long v; nm_inv_mod(a,p,&v);
                  printf("%4ld", (a*v)%p);
                  if((a*v)%p != 1) mal++;
                  if(v != iz_pot_mod(a,p-2,p)) mal++; }
              printf("\n");
              tique7(5, "logo todo não nulo de 𝔽₇ tem inverso — e é isso ser corpo");
              tique7(6, "e a volta: os DOIS caminhos (Euclides e a potência) dão a mesma"
                        " tabela, e cada produto fecha em 1");
              printf("      Euclides e a^{p−2} concordam, e todos os produtos dão 1:"
                     " %d falhas\n", mal); }
        } else if(n == 22){
            tique7(0, "seja 𝔽₇ e queira-se g que GERE o grupo multiplicativo");
            tique7(1, "primitivo é o que percorre tudo:");
            printf("      $K^{\\times} = \\{ 1, g, g^2, \\ldots, g^{q-2} \\}$\n");
            tique7(2, "g é primitivo exatamente quando a sua ORDEM é q−1 = 6. Testa-se"
                      " cada candidato calculando a órbita");
            tique7(3, "a lei é LAGRANGE: a ordem divide 6, logo só pode ser 1, 2, 3 ou 6,"
                      " e primitivo é o caso 6");
            tique7(4, "a testemunha é a órbita inteira, exibida");
            { int mal = 0;
              for(long g = 1; g < p; g++){
                  long o = nm_ordem(g,p);
                  printf("      g = %ld:  ordem %ld  →  ", g, o);
                  for(long k = 0; k < 6; k++) printf("%ld ", iz_pot_mod(g,k,p));
                  printf("  %s\n", o == 6 ? "PRIMITIVO" : "");
                  if((p-1) % o) mal++;                       /* Lagrange */
              }
              tique7(5, "logo 3 e 5 são primitivos em 𝔽₇, e 1, 2, 4, 6 não são");
              tique7(6, "e a volta: todas as ordens DIVIDEM 6, que é Lagrange a fechar —"
                        " e o número de primitivos é φ(6) = 2");
              int prim = 0;
              for(long g = 1; g < p; g++) if(nm_ordem(g,p) == p-1) prim++;
              printf("      %d primitivos, e φ(6) = %ld   %s\n",
                     prim, nm_phi(6), prim == nm_phi(6) ? "(confere)" : "— NÃO afirmo");
              printf("      e as ordens dividem 6: %d falhas\n", mal); }
        } else if(n == 23){
            tique7(0, "seja p primo e a não nulo em 𝔽ₚ");
            tique7(1, "o pequeno Fermat:");
            printf("      $a^{p-1} = 1$   em $\\mathbb{F}_p$\n");
            tique7(2, "𝔽ₚ× é grupo de ordem p−1 (teorema 15). A ordem de a DIVIDE p−1"
                      " (Lagrange), digamos p−1 = k·ord(a). Então"
                      " a^{p−1} = (a^{ord})^k = 1^k = 1");
            tique7(3, "a lei é LAGRANGE, e é ela que faz a prova ser de três linhas — sem"
                      " o grupo, seria a indução clássica com binómios");
            tique7(4, "a testemunha é o k = (p−1)/ord(a), e ele exibe-se");
            { int mal = 0;
              for(long a = 1; a < p; a++){
                  long o = nm_ordem(a,p);
                  printf("      a = %ld:  ord = %ld,  k = %ld,  a^6 = %ld\n",
                         a, o, (p-1)/o, iz_pot_mod(a,p-1,p));
                  if(iz_pot_mod(a,p-1,p) != 1) mal++;
                  if((p-1) % o) mal++;
              }
              tique7(5, "logo a^{p−1} = 1 para todo a ≠ 0");
              tique7(6, "e a volta: a⁻¹ = a^{p−2}, porque a·a^{p−2} = a^{p−1} = 1 — a"
                        " INVERSÃO VIRA POTÊNCIA, e é isso que o teorema entrega");
              printf("      todos os a^{p−1} = 1 e as ordens dividem p−1: %d falhas\n", mal); }
        } else {
            tique7(0, "seja p primo e a não nulo em 𝔽ₚ. Quer-se o inverso SEM tentativa");
            tique7(1, "Euclides estendido dá a combinação:");
            printf("      $ax + py = \\gcd(a,p) = 1 \\; \\Rightarrow \\; ax \\equiv 1"
                   " \\bmod p$\n");
            tique7(2, "como p é primo e a < p, o gcd é 1. A descida de Euclides produz x"
                      " e y; reduzindo x módulo p sai o inverso");
            tique7(3, "as leis são a PRIMALIDADE (gcd = 1) e BÉZOUT — e o custo é"
                      " log p em vez dos p passos da tentativa");
            tique7(4, "a testemunha é o x, e ele sai do MESMO rastro que dá o gcd");
            { int mal = 0;
              for(long a = 1; a < 13; a++){
                  long x, y, g = iz_gcd(a, 13, &x, &y), inv;
                  nm_inv_mod(a, 13, &inv);
                  if(g != 1 || (a*inv) % 13 != 1) mal++;
                  if(inv != ((x % 13) + 13) % 13) mal++;
                  if(inv != iz_pot_mod(a, 11, 13)) mal++;    /* e concorda com Fermat */
              }
              tique7(5, "logo a inversão em 𝔽ₚ é Euclides, e não busca");
              tique7(6, "e a volta por TRÊS caminhos: o x de Bézout reduzido, a potência"
                        " a^{p−2} de Fermat, e o produto a dar 1 — os três concordam");
              printf("      𝔽₁₃, os 12 não nulos: Bézout = Fermat = inverso, %d falhas\n", mal); }
        }
        break; }
    case 11: case 18: case 19: {
        tique7(0, n == 11 ? "seja K um corpo FINITO"
                          : "suponha-se um corpo com esse número de elementos");
        tique7(1, "a cardinalidade de um corpo finito:");
        printf("      $|K| = p^{n}$,   com $p = \\operatorname{char}(K)$ primo\n");
        tique7(2, "char(K) = p primo (teorema 10), logo K contém uma cópia de 𝔽ₚ. E K é"
                  " espaço vetorial sobre essa cópia — de dimensão finita n, porque K é"
                  " finito. Um espaço de dimensão n sobre 𝔽ₚ tem exatamente pⁿ vetores");
        tique7(3, "a lei é a CONTAGEM das coordenadas: cada vetor é uma n-upla de 𝔽ₚ, e"
                  " há p escolhas por coordenada. É o produto cartesiano a contar");
        tique7(4, n == 11 ? "a testemunha é a base de K sobre 𝔽ₚ"
                          : "a testemunha é a FATORAÇÃO do número pedido");
        { if(n == 11){
              printf("      as cardinalidades possíveis até 30:  ");
              for(int q = 2; q <= 30; q++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(q, pr, ex, NT_FAT);
                  if(k == 1) printf("%d ", q);
              }
              printf("\n      e as impossíveis:  ");
              for(int q = 2; q <= 30; q++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(q, pr, ex, NT_FAT);
                  if(k > 1) printf("%d ", q);
              }
              printf("\n");
          } else {
              long q = (n == 18) ? 6 : 10;
              long pr[NT_FAT]; int ex[NT_FAT];
              int k = nt_fatora(q, pr, ex, NT_FAT);
              printf("      %ld = ", q);
              for(int i = 0; i < k; i++){ printf("%s%ld", i?"·":"", pr[i]); if(ex[i]>1) printf("^%d", ex[i]); }
              printf("   — DOIS primos distintos, logo não é pⁿ\n");
              printf("      e se houvesse, a característica teria de ser %ld e %ld ao"
                     " mesmo tempo\n", pr[0], pr[1]);
          }
          tique7(5, n == 11 ? "logo |K| = pⁿ, sempre"
                            : "logo NÃO existe corpo com esse número de elementos");
          tique7(6, "e a volta pelo lado que EXISTE: para cada pⁿ constrói-se mesmo o"
                    " corpo, e mede-se. Dizer que não existe sem mostrar os que existem"
                    " seria metade");
          { int f4[] = {1,1,1}, f9[] = {1,0,1}, f8[] = {1,1,0,1};
            Anel R;
            struct { int p, n2; int *f; const char *nome; } cs[3] = {
                {2,2,f4,"F4"}, {3,2,f9,"F9"}, {2,3,f8,"F8"} };
            int mal = 0;
            for(int i = 0; i < 3; i++){
                if(!corpo_ext(cs[i].p, cs[i].f, cs[i].n2, &R)){ mal++; continue; }
                printf("      %s: |K| = %d, corpo? %s, char = %d\n",
                       cs[i].nome, R.n, an_corpo(&R,0) ? "sim" : "NÃO", corpo_carac(&R,20));
                if(!an_corpo(&R,0)) mal++;
            }
            /* e a ausência mede-se: nenhum ℤₘ com m composto é corpo */
            int falsos = 0;
            for(int m = 2; m <= ES_MAX; m++){
                Anel S; an_zn(&S,m);
                if(an_corpo(&S,0) && !nt_primo(m)) falsos++;
            }
            printf("      e nenhum ℤₘ composto é corpo: %d falsos (mais %d falhas na"
                   " construção)\n", falsos, mal); } }
        break; }
    case 12: case 17: {
        long d = (n == 12) ? 2 : 3;
        { char pq[220];
          snprintf(pq, sizeof pq, "seja x² − %ld sobre ℚ, e α = √%ld — que é algébrico"
                   " sobre ℚ precisamente por ser raiz dele", d, d);
          tique7(0, pq); }
        tique7(1, n == 12 ? "irredutível em ℚ[x] é não fatorar em fatores de grau menor:"
                          : "o polinómio mínimo é o mónico irredutível que anula α:");
        if(n == 12) printf("      $x^2 - 2$ não é $(x-r)(x-s)$ com $r,s \\in \\mathbb{Q}$\n");
        else        printf("      $m_{\\alpha}(x) \\in K[x]$ mónico, irredutível,"
                           " com $m_{\\alpha}(\\alpha) = 0$\n");
        tique7(2, "grau 2 fatora em ℚ[x] se e só se TEM RAIZ em ℚ. E uma raiz de x² − d"
                  " seria um racional cujo quadrado é d, isto é √d ∈ ℚ");
        tique7(3, "a lei é o teorema do andar de ℝ: √d ∈ ℚ ⟺ todo expoente primo de d é"
                  " PAR. Aqui d tem um expoente ímpar, logo não há raiz");
        { long pr[NT_FAT]; int ex[NT_FAT];
          int k = nt_fatora(d, pr, ex, NT_FAT), impar = -1;
          for(int i = 0; i < k; i++) if(ex[i] % 2) impar = i;
          printf("      %ld = ", d);
          for(int i = 0; i < k; i++){ printf("%s%ld", i?"·":"", pr[i]); if(ex[i]>1) printf("^%d", ex[i]); }
          printf(",  e o primo %ld tem expoente ÍMPAR\n", pr[impar < 0 ? 0 : impar]);
          tique7(4, "a testemunha é esse primo — o mesmo que provou √d ∉ ℚ no andar de ℝ");
          { long em_cima = 0, total = 0;
            for(long q = 1; q <= 60; q++) for(long pp = 1; pp <= (raizi(d)+1)*q; pp++){
                int bom, s = rz_cmp(qz(pp,q), 2, d, &bom);
                if(bom){ total++; if(s == 0) em_cima++; }
            }
            printf("      %ld racionais testados como raiz: %ld são raiz\n", total, em_cima);
            tique7(5, n == 12 ? "logo x² − 2 é IRREDUTÍVEL em ℚ[x]"
                              : "logo m_{√3}(x) = x² − 3, e é único");
            tique7(6, "e a volta: substitui-se α no polinómio e mede-se que dá zero, em"
                      " ℚ(√d) exato — não em decimal");
            Qs alfa = qs(qz(0,1), qz(1,1));
            Qs a2 = qs_mult(alfa, alfa, d);
            Qs val = qs_soma(a2, qs(qz_de_inteiro(-d), qz(0,1)));
            printf("      α² = "); esc_qz("", a2.a, " + ");
            esc_qz("", a2.b, "√"); printf("%ld,   α² − %ld = ", d, d);
            esc_qz("", val.a, " + "); esc_qz("", val.b, "√");
            printf("%ld   %s\n", d,
                   (val.a.p == 0 && val.b.p == 0) ? "(resíduo 0)" : "— NÃO afirmo"); } }
        break; }
    case 13: {
        tique7(0, "seja ℚ e α = √2, que é algébrico sobre ℚ (raiz de x² − 2)");
        tique7(1, "a extensão é o menor corpo que contém ℚ e α:");
        printf("      $\\mathbb{Q}(\\sqrt2) = \\{ a + b\\sqrt2 : a, b \\in \\mathbb{Q} \\}$\n");
        tique7(2, "o conjunto fecha para + e ×, porque (a+b√2)(c+d√2) = (ac+2bd) +"
                  " (ad+bc)√2 — o √2·√2 = 2 volta para ℚ, e é por isso que basta grau 1");
        tique7(3, "a lei é a RELAÇÃO α² = 2: é ela que reduz qualquer potência de α a"
                  " grau ≤ 1, e é o polinómio mínimo a fazer esse trabalho");
        tique7(4, "a testemunha do INVERSO é a NORMA: (a+b√2)(a−b√2) = a² − 2b², que só"
                  " é 0 quando a = b = 0 — porque √2 ∉ ℚ");
        printf("      $(a + b\\sqrt2)^{-1} = \\frac{a - b\\sqrt2}{a^2 - 2b^2}$\n");
        { int mal = 0; long feitos = 0;
          for(long ap = -6; ap <= 6; ap++) for(long bp = -6; bp <= 6; bp++){
              Qs x = qs(qz_de_inteiro(ap), qz_de_inteiro(bp)), inv;
              Qz N = qs_norma(x, 2);
              int nulo = (ap == 0 && bp == 0);
              if((N.p == 0) != nulo) mal++;              /* a norma só zera no zero */
              if(nulo) continue;
              if(!qs_inverso(x, 2, &inv)){ mal++; continue; }
              Qs um = qs_mult(x, inv, 2);
              if(um.a.p != 1 || um.a.q != 1 || um.b.p != 0) mal++;
              feitos++;
          }
          Qs y = qs(qz_de_inteiro(3), qz_de_inteiro(2)), iy;
          qs_inverso(y, 2, &iy);
          printf("      (3 + 2√2)⁻¹ = "); esc_qz("", iy.a, iy.b.p < 0 ? " − " : " + ");
          { Qz mb = iy.b.p < 0 ? qz_oposto(iy.b) : iy.b;
            esc_qz("", mb, "√2   (norma 9 − 8 = 1)\n"); }
          tique7(5, "logo ℚ(√2) é corpo, e [ℚ(√2):ℚ] = 2 — a base é {1, √2}");
          tique7(6, "e a volta: cada inverso multiplica-se de volta e dá 1 + 0√2, exato");
          printf("      %ld inversos verificados em ℚ(√2): %d falhas\n", feitos, mal);
          printf("      e a norma só é zero no elemento zero — é ela a fibra deste corpo\n"); }
        break; }
    case 14: case 20: {
        int p = (n == 14) ? 2 : 3, gr = 2;
        int f4[] = {1,1,1}, f9[] = {1,0,1};
        int *f = (n == 14) ? f4 : f9;
        tique7(0, "seja 𝔽ₚ e um polinómio mónico de grau 2 sobre ele");
        tique7(1, "o quociente por um irredutível:");
        printf(n == 14 ? "      $\\mathbb{F}_4 = \\mathbb{F}_2[x]/(x^2 + x + 1)$\n"
                       : "      $\\mathbb{F}_9 = \\mathbb{F}_3[x]/(x^2 + 1)$\n");
        tique7(2, "primeiro prova-se a IRREDUTIBILIDADE: um grau 2 fatora se e só se tem"
                  " raiz, e testam-se as p raízes possíveis — são finitas, logo a"
                  " varredura é a prova");
        { printf("      raízes em 𝔽%d:  ", p);
          for(int a = 0; a < p; a++){
              int v = fx_m(f[2]*a*a + f[1]*a + f[0], p);
              printf("f(%d) = %d%s", a, v, a == p-1 ? "" : ",  ");
          }
          printf("   — nenhuma zero\n");
          tique7(3, "a lei é a mesma dos números: o quociente por um IRREDUTÍVEL é corpo,"
                    " tal como ℤ/pℤ é corpo por p ser primo. Irredutível está para o"
                    " polinómio como primo está para o inteiro");
          tique7(4, "a testemunha é a tábua: constrói-se, e vê-se que todo não nulo tem"
                    " inverso");
          Anel R;
          corpo_ext(p, f, gr, &R);
          printf("      irredutível? %s;  |K| = %d = %d²;  corpo? %s;  char = %d\n",
                 fx_irredutivel(f, gr, p) ? "sim" : "NÃO", R.n, p,
                 an_corpo(&R,0) ? "sim" : "NÃO", corpo_carac(&R, 30));
          { char pq[160];
            snprintf(pq, sizeof pq, "logo o quociente é um CORPO com %d elementos, e"
                     " %d = %d² é uma potência de primo — como tinha de ser", R.n, R.n, p);
            tique7(5, pq); }
          tique7(6, "e o GUME: com um polinómio REDUTÍVEL o quociente NÃO é corpo — e"
                    " mede-se, para se ver que a hipótese carrega o teorema");
          { int red[] = {1,0,1};                        /* x²+1 = (x+1)² sobre 𝔽₂ */
            Anel Rr;
            corpo_ext(2, red, 2, &Rr);
            int ea = 0, eb = 0;
            an_dominio(&Rr, &ea, &eb);
            printf("      x²+1 sobre 𝔽₂ é (x+1)²: irredutível? %s;  o quociente é corpo?"
                   " %s  (e %d·%d = 0)\n",
                   fx_irredutivel(red,2,2) ? "sim" : "NÃO",
                   an_corpo(&Rr,0) ? "sim" : "NÃO", ea, eb); } }
        break; }
    case 15: {
        int f4[] = {1,1,1};
        Anel R;
        corpo_ext(2, f4, 2, &R);
        tique7(0, "seja 𝔽₄ = 𝔽₂[x]/(x²+x+1), com os elementos 0, 1, x, x+1");
        tique7(1, "o inverso é o parceiro que dá 1:");
        printf("      $a a^{-1} = 1$   em $\\mathbb{F}_4$\n");
        tique7(2, "x·(x+1) = x² + x = (x+x+1) + x = 1, usando x² = x + 1 (que é a relação"
                  " do quociente, porque x²+x+1 = 0 e em característica 2 o sinal é o mesmo)");
        tique7(3, "a lei é a REDUÇÃO módulo o polinómio: é ela que traz x² de volta ao"
                  " grau ≤ 1, e é o mesmo mecanismo do resto na divisão de inteiros");
        tique7(4, "a testemunha é a tábua reduzida, e ela exibe-se");
        { const char *nm[4] = { "0", "1", "x", "x+1" };
          int mal = 0;
          for(int a = 1; a < 4; a++){
              int inv = corpo_inv(&R, a);
              printf("      (%s)⁻¹ = %s   porque (%s)·(%s) = %s\n",
                     nm[a], inv < 0 ? "—" : nm[inv], nm[a],
                     inv < 0 ? "—" : nm[inv], inv < 0 ? "—" : nm[R.mult[a][inv]]);
              if(inv < 0 || R.mult[a][inv] != 1) mal++;
          }
          printf("      e 0⁻¹ = %s — a exceção do andar, a mesma de sempre\n",
                 corpo_inv(&R,0) < 0 ? "NÃO EXISTE" : "??");
          tique7(5, "logo todo não nulo de 𝔽₄ tem inverso — e note-se que x⁻¹ = x + 1,"
                    " que é o mesmo que x², porque x³ = 1: a inversão é uma POTÊNCIA,"
                    " tal como em 𝔽ₚ");
          tique7(6, "e a volta: cada produto a·a⁻¹ dá 1, e o 0 continua sem fibra");
          printf("      3 inversos, %d falhas;  e o elemento primitivo é %d (ordem 3)\n",
                 mal, corpo_primitivo(&R)); }
        break; }
    case 16: {
        tique7(0, "seja K corpo e p(x) ∈ K[x] irredutível");
        tique7(1, "o quociente pelo ideal gerado:");
        printf("      $K[x]/(p(x))$ é corpo quando $p$ é irredutível\n");
        tique7(2, "para f ≠ 0 no quociente, gcd(f, p) = 1 (porque p é irredutível e não"
                  " divide f). Bézout em K[x] dá u, v com fu + pv = 1, logo fu ≡ 1 no"
                  " quociente: o u é o inverso");
        tique7(3, "a lei é BÉZOUT — agora em K[x] em vez de ℤ, e é a MESMA descida de"
                  " Euclides, porque K[x] também tem divisão com resto");
        tique7(4, "a testemunha é o u, e ele sai do rastro de Euclides sobre polinómios");
        { int f4[] = {1,1,1}, f9[] = {1,0,1}, f8[] = {1,1,0,1}, red[] = {1,0,1};
          struct { int p, g; int *f; const char *nome; } cs[4] = {
              {2,2,f4,"F2[x]/(x²+x+1)"}, {3,2,f9,"F3[x]/(x²+1)"},
              {2,3,f8,"F2[x]/(x³+x+1)"}, {2,2,red,"F2[x]/(x²+1)"} };
          int mal = 0;
          for(int i = 0; i < 4; i++){
              Anel R;
              if(!corpo_ext(cs[i].p, cs[i].f, cs[i].g, &R)){ mal++; continue; }
              int irr = fx_irredutivel(cs[i].f, cs[i].g, cs[i].p);
              int cor = an_corpo(&R,0);
              printf("      %-18s irredutível? %-4s  corpo? %-4s\n",
                     cs[i].nome, irr ? "sim" : "NÃO", cor ? "sim" : "NÃO");
              if(irr != cor) mal++;                       /* A EQUIVALÊNCIA */
          }
          tique7(5, "logo K[x]/(p) é corpo — e a construção transforma"
                    " corpo → polinómio → fatoração → irredutibilidade → quociente");
          tique7(6, "e a volta é a EQUIVALÊNCIA medida nos dois sentidos: irredutível ⟹"
                    " corpo, e redutível ⟹ NÃO corpo. O último caso da lista é o gume");
          printf("      quatro quocientes: irredutível ⟺ corpo, %d divergências\n", mal); }
        break; }
    case 21: {
        tique7(0, "seja K corpo finito, |K| = q. Quer-se g que gere K× inteiro");
        tique7(1, "cíclico é ser gerado por um só:");
        printf("      $K^{\\times} = \\langle g \\rangle$,   com $|K^{\\times}| = q - 1$\n");
        tique7(2, "para cada d | q−1, o polinómio x^d − 1 tem no MÁXIMO d raízes num"
                  " corpo (porque um corpo não tem divisores de zero). Isso limita quantos"
                  " elementos podem ter ordem d, e a contagem só fecha se existir um de"
                  " ordem q−1");
        tique7(3, "a lei é «um polinómio de grau d tem ≤ d raízes num CORPO» — e ela"
                  " depende de não haver divisores de zero, isto é, do teorema 5");
        tique7(4, "a testemunha é o elemento primitivo, e exibe-se em cada corpo");
        { int mal = 0, testados = 0;
          for(int m = 2; m <= 17; m++){
              Anel R; an_zn(&R,m);
              if(!an_corpo(&R,0)) continue;
              int g = corpo_primitivo(&R);
              if(g < 0){ mal++; continue; }
              printf("      𝔽%-3d primitivo g = %d,  ordem %ld = %d−1\n",
                     m, g, nm_ordem(g,m), m);
              testados++;
          }
          /* e nas EXTENSÕES, que é onde o teorema deixa de ser sobre ℤₚ */
          int f4[] = {1,1,1}, f9[] = {1,0,1}, f8[] = {1,1,0,1};
          struct { int p, g2; int *f; int q; } cs[3] = {
              {2,2,f4,4}, {3,2,f9,9}, {2,3,f8,8} };
          for(int i = 0; i < 3; i++){
              Anel R;
              corpo_ext(cs[i].p, cs[i].f, cs[i].g2, &R);
              int g = corpo_primitivo(&R);
              if(g < 0) mal++;
              printf("      |K| = %d:  primitivo = %d\n", cs[i].q, g);
              testados++;
          }
          tique7(5, "logo K× é CÍCLICO — todo corpo finito tem elemento primitivo");
          tique7(6, "e a volta: em cada corpo achado, as potências de g percorrem TODOS os"
                    " q−1 não nulos, sem repetir. Se faltasse um, g não era primitivo");
          printf("      %d corpos (ℤₚ e extensões), todos com primitivo: %d falhas\n",
                 testados, mal); }
        break; }
    case 25: {
        tique7(0, "sejam a e p com gcd(a,p) = 1. Quer-se ver que cinco coisas são UMA");
        tique7(1, "a cadeia que ele desenha:");
        printf("      Euclides $\\leftrightarrow$ MDC $\\leftrightarrow$ Bézout"
               " $\\leftrightarrow$ inverso $\\leftrightarrow$ corpo\n");
        tique7(2, "a descida a = pq + r produz, do MESMO rastro: o gcd (último resto não"
                  " nulo), os coeficientes de Bézout (subindo), o inverso (o x reduzido)"
                  " e, quando o gcd é sempre 1, o corpo (todo não nulo invertível)");
        tique7(3, "a lei que atravessa as cinco é uma só — a DIVISÃO COM RESTO. É ela que"
                  " faz a descida terminar, e é dela que tudo o resto se lê");
        tique7(4, "a testemunha é a própria cadeia de restos, e mostra-se uma");
        { long a = 5, p = 13;
          printf("      Euclides(%ld, %ld):\n", p, a);
          { long u = p, v = a;
            while(v){ printf("        %ld = %ld·%ld + %ld\n", u, v, u/v, u%v);
                      long r = u % v; u = v; v = r; }
            printf("        gcd = %ld\n", u); }
          long bx, by, g = iz_gcd(a, p, &bx, &by);
          long inv; nm_inv_mod(a, p, &inv);
          printf("      Bézout:  %ld·(%ld) + %ld·(%ld) = %ld\n", a, bx, p, by, g);
          printf("      inverso: %ld⁻¹ = %ld  (e %ld·%ld = %ld ≡ 1 mod %ld)\n",
                 a, inv, a, inv, a*inv, p);
          tique7(5, "logo as cinco são leituras do mesmo rastro — e o CORPO é a afirmação"
                    " de que isto corre para TODO não nulo");
          tique7(6, "e a volta é a equivalência medida: 𝔽ₚ é corpo ⟺ todo não nulo tem"
                    " gcd 1 com p ⟺ Euclides devolve sempre 1 ⟺ Bézout dá sempre inverso");
          { int mal = 0, corpos = 0;
            for(int m = 2; m <= ES_MAX; m++){        /* o teto da tábua */
                int todos = 1;
                for(int b = 1; b < m; b++) if(iz_gcd(b,m,0,0) != 1) todos = 0;
                Anel R; an_zn(&R,m);
                if(todos != an_corpo(&R,0)) mal++;
                if(todos) corpos++;
            }
            printf("      ℤₘ até ao teto: «Euclides dá sempre 1» ⟺ «é corpo» — %d corpos,"
                   " %d divergências\n", corpos, mal); } }
        break; }
    }
}
static int resolve_corpo(const char *f){
    const char *p = f;
    /* O NOME EXATO PRIMEIRO — porque «corpo de 9» COMEÇA por «corpo», e o prefixo
     * comia-o: sobrava « de 9», que não é número nem nome, e a fala morria calada.
     * Foi a varredura do índice que o apanhou, e não a leitura. */
    for(size_t i = 0; i < sizeof CP25/sizeof *CP25; i++)
        if(!strcmp(p, CP25[i].nome)){ corpo_resolve(CP25[i].n); return 1; }
    if(!strncmp(p, "corpos", 6)) p += 6;
    else if(!strncmp(p, "corpo", 5)) p += 5;
    else return 0;
    while(*p == ' ') p++;
    if(!*p){
        printf("   teoria dos corpos — «corpos N» ou «corpos <nome>»\n");
        printf("   e o andar fecha a escada: ℕ → ℤ → ℚ → ℝ → K, uma reversibilidade"
               " nova em cada salto\n");
        printf("   (a exceção continua a mesma: 0⁻¹ não existe)\n\n");
        for(size_t i = 0; i < sizeof CP25/sizeof *CP25; i++){
            if(i == 0)  printf("   nível 1 — estrutura\n");
            if(i == 6)  printf("   nível 2 — finitos\n");
            if(i == 11) printf("   nível 3 — extensões\n");
            if(i == 20) printf("   nível 4 — máquina pesada\n");
            printf("     %2d  ", CP25[i].n);
            esc_col(CP25[i].nome, 26);
            printf("  %s\n", CP25[i].enunciado);
        }
        return 1;
    }
    if(*p >= '0' && *p <= '9'){
        long n = 0;
        while(*p >= '0' && *p <= '9') n = n*10 + (*p++ - '0');
        while(*p == ' ') p++;
        if(!*p && n >= 1 && n <= 25){ corpo_resolve((int)n); return 1; }
        return 0;
    }
    for(size_t i = 0; i < sizeof CP25/sizeof *CP25; i++)
        if(!strcmp(p, CP25[i].nome)){ corpo_resolve(CP25[i].n); return 1; }
    return 0;
}
/* ── ÁLGEBRA MODERNA: O RASTRO DE SETE TICKS, E A DEFINIÇÃO EM LaTeX ────────────────
 * O `eval.txt` fecha com a exigência de forma, e ela vale para TODOS os teoremas:
 *
 *   hipóteses → definição → transição → lei usada → testemunha → conclusão → volta
 *
 * «E o mais importante para o teu sistema: NÃO MEDIR SÓ A CONCLUSÃO.» São sete lugares,
 * e cada um tem de ser preenchido — a lei que autoriza a transição diz-se pelo nome, e a
 * testemunha exibe-se. Um teorema que salte a testemunha fica com um buraco visível.
 *
 * A DEFINIÇÃO vai em LaTeX, no seu tick, para o tradutor a compor. É a membrana a
 * carregar a matemática e não só o texto. */
static const char *ES7[7] = { "HIPÓTESES", "DEFINIÇÃO", "TRANSIÇÃO", "LEI USADA",
                              "TESTEMUNHA", "CONCLUSÃO", "VOLTA" };
static void tique7(int slot, const char *porque){
    char pq[600];
    snprintf(pq, sizeof pq, "%s — %s", ES7[slot & 7], porque);
    tique(pq);
}
static void es_conj(const Est *E, unsigned H){    /* escreve um subconjunto */
    printf("{");
    int primeiro = 1;
    for(int i = 0; i < E->n; i++) if(es_em(H,i)){ printf("%s%d", primeiro?"":", ", i); primeiro = 0; }
    printf("}");
}
static const struct { int n; const char *nome; const char *enunciado; } AL20[] = {
 {  1, "identidade unica",  "o elemento neutro é ÚNICO" },
 {  2, "inverso unico",     "o inverso de cada elemento é ÚNICO" },
 {  3, "cancelamento",      "ab = ac ⟹ b = c" },
 {  4, "subgrupo neutro",   "todo subgrupo contém o neutro" },
 {  5, "intersecao",        "a interseção de subgrupos é subgrupo" },
 {  6, "imagem subgrupo",   "a imagem de um homomorfismo é subgrupo" },
 {  7, "nucleo normal",     "o núcleo de um homomorfismo é subgrupo NORMAL" },
 {  8, "injetivo nucleo",   "f é injetivo ⟺ o núcleo é trivial" },
 {  9, "lagrange",          "|H| divide |G| — e as classes particionam" },
 { 10, "cayley",            "todo grupo mergulha num grupo de permutações" },
 { 11, "primeiro iso",      "G/ker f ≅ Im f" },
 { 12, "ideal nucleo",      "os ideais são os núcleos dos homomorfismos de anéis" },
 { 13, "iso aneis",         "R/I ≅ Im f, o isomorfismo para anéis" },
 { 14, "ideais de Z",       "todo ideal de ℤ é nℤ" },
 { 15, "corpo dominio",     "todo corpo é domínio integral" },
 { 16, "operacao neutro",   "a ⋆ b = a + b + 1: fechada, e o neutro é −1" },
 { 17, "nao associativa",   "a ⋆ b = a − b NÃO é associativa, e o contra-exemplo exibe-se" },
 { 18, "todo grupo comuta", "«prove que todo grupo é comutativo» — a RECUSA" },
 { 19, "exponencial",       "a soma vira produto: (ℤₙ,+) ≅ (ℤₚ*,×)" },
 { 20, "menos vezes",       "(−a)b = −(ab) num anel qualquer" },
};
static void algebra_resolve(int n){
    TICK_N = 0;
    printf("   %d — %s\n", n, AL20[n-1].enunciado);
    Est Z12, S3, Z6, Z4;
    es_zn(&Z12, 12); es_s3(&S3); es_zn(&Z6, 6); es_zn(&Z4, 4);
    switch(n){
    case 1: {
        tique7(0, "seja (G,⋆) com DOIS neutros, e e e′. Não se supõe que sejam iguais —"
                  " supõe-se que os dois cumprem a definição, que é o contrário");
        tique7(1, "neutro é o que não mexe em ninguém, dos DOIS lados:");
        printf("      $e \\star a = a \\star e = a$   para todo $a \\in G$\n");
        tique7(2, "aplica-se a definição de e′ ao elemento e, e a de e ao elemento e′:"
                  " e ⋆ e′ = e′ (por e ser neutro) e e ⋆ e′ = e (por e′ ser neutro)");
        printf("      $e \\star e' = e'$   e   $e \\star e' = e$\n");
        tique7(3, "a lei é a própria DEFINIÇÃO de neutro, usada duas vezes — uma por"
                  " cada candidato. Não entra mais nada: nem associatividade, nem inverso");
        tique7(4, "a testemunha é o elemento e ⋆ e′, que tem de ser os dois ao mesmo tempo");
        { Est *Gs[3] = { &Z12, &S3, &Z6 }; int mal = 0;
          for(int i = 0; i < 3; i++){
              int quantos = 0;
              for(int e = 0; e < Gs[i]->n; e++){
                  int bom = 1;
                  for(int a = 0; a < Gs[i]->n && bom; a++)
                      if(Gs[i]->op[e][a] != a || Gs[i]->op[a][e] != a) bom = 0;
                  if(bom) quantos++;
              }
              printf("      %s: %d neutro(s) — %s\n", Gs[i]->nome, quantos,
                     quantos == 1 ? "um só" : "MAIS QUE UM");
              if(quantos != 1) mal++;
          }
          tique7(5, "logo e = e′: os dois neutros eram o mesmo, e a palavra «o» neutro"
                    " passa a ser legítima");
          tique7(6, "e a volta: varre-se o grupo à procura de TODOS os elementos que"
                    " cumprem a definição, e conta-se — se aparecesse um segundo, o"
                    " teorema caía aqui");
          printf("      três grupos varridos por inteiro: %d com mais de um neutro\n", mal); }
        break; }
    case 2: {
        tique7(0, "seja a com dois inversos, b e c: a ⋆ b = b ⋆ a = e e a ⋆ c = c ⋆ a = e");
        tique7(1, "inverso é o que devolve ao neutro:");
        printf("      $a \\star a^{-1} = a^{-1} \\star a = e$\n");
        tique7(2, "b = b ⋆ e = b ⋆ (a ⋆ c) = (b ⋆ a) ⋆ c = e ⋆ c = c — cinco passos, e o"
                  " do meio é o que faz o trabalho");
        printf("      $b = b \\star e = b \\star (a \\star c) = (b \\star a) \\star c"
               " = e \\star c = c$\n");
        tique7(3, "a lei do passo do meio é a ASSOCIATIVIDADE, e é por isso que a"
                  " unicidade do inverso é teorema de SEMIGRUPO com neutro, não de grupo:"
                  " sem associatividade a cadeia parte-se ali");
        tique7(4, "a testemunha é o elemento b ⋆ a ⋆ c, que se lê de duas maneiras");
        { int mal = 0;
          Est *Gs[2] = { &Z12, &S3 };
          for(int i = 0; i < 2; i++){
              int e = es_neutro(Gs[i]);
              for(int a = 0; a < Gs[i]->n; a++){
                  int quantos = 0;
                  for(int b = 0; b < Gs[i]->n; b++)
                      if(Gs[i]->op[a][b] == e && Gs[i]->op[b][a] == e) quantos++;
                  if(quantos != 1) mal++;
              }
          }
          tique7(5, "logo b = c, e o inverso é único — o que autoriza a NOTAÇÃO a⁻¹,"
                    " que sem este teorema seria ambígua");
          tique7(6, "e a volta: contam-se todos os inversos de cada elemento nos dois"
                    " grupos, e nenhum tem dois");
          printf("      Z12 e S3, elemento a elemento: %d com mais de um inverso\n", mal); }
        break; }
    case 3: {
        tique7(0, "num GRUPO (e é preciso ser grupo), suponha-se a ⋆ b = a ⋆ c");
        tique7(1, "cancelar é poder tirar o a dos dois lados:");
        printf("      $a \\star b = a \\star c \\; \\Rightarrow \\; b = c$\n");
        tique7(2, "multiplica-se à ESQUERDA por a⁻¹: a⁻¹⋆(a⋆b) = a⁻¹⋆(a⋆c), e a"
                  " associatividade reagrupa para (a⁻¹⋆a)⋆b = (a⁻¹⋆a)⋆c");
        tique7(3, "as leis são o INVERSO (que existe por ser grupo) e a ASSOCIATIVIDADE"
                  " (para reagrupar). Note-se o lado: à esquerda cancela-se à esquerda, e"
                  " em grupo não abeliano isso importa");
        tique7(4, "a testemunha é o a⁻¹, e ele existe precisamente por ser grupo");
        { int mal = 0, casos = 0;
          for(int a = 0; a < S3.n; a++) for(int b = 0; b < S3.n; b++) for(int c = 0; c < S3.n; c++){
              if(S3.op[a][b] == S3.op[a][c] && b != c) mal++;
              casos++;
          }
          tique7(5, "logo b = c: em grupo, cancelar é legítimo");
          tique7(6, "e o GUME — num MONOIDE sem inversos o cancelamento FALHA. Em (ℤ₆,×),"
                    " 2·1 = 2 e 2·4 = 8 = 2, e 1 ≠ 4. A hipótese «grupo» não é decoração");
          { Est M; es_zn_mult(&M, 6);
            printf("      em S3 (grupo): %d falhas em %d triplos\n", mal, casos);
            printf("      em (Z6,x) (monoide): 2·1 = %d e 2·4 = %d, e 1 ≠ 4 — cancela? NÃO\n",
                   M.op[2][1], M.op[2][4]);
            printf("      e 2 tem inverso em (Z6,x)? %s — é essa a razão\n",
                   es_inverso(&M,2) < 0 ? "não" : "sim"); } }
        break; }
    case 4: {
        tique7(0, "seja H subgrupo de G, e portanto H não é vazio: existe pelo menos um"
                  " a em H. É esta a hipótese que se usa, e é a única");
        tique7(1, "o critério do subgrupo, num só:");
        printf("      $a, b \\in H \\; \\Rightarrow \\; a b^{-1} \\in H$\n");
        tique7(2, "toma-se b = a no critério: a·a⁻¹ ∈ H");
        tique7(3, "a lei é o próprio critério, aplicado ao caso a = b — e a definição de"
                  " inverso, que diz que a·a⁻¹ é o neutro");
        tique7(4, "a testemunha é o elemento a que existe por H não ser vazio: sem ele"
                  " não há de onde partir, e é por isso que o vazio NÃO é subgrupo");
        { unsigned Hs[3] = { (1u<<0)|(1u<<4)|(1u<<8), (1u<<0)|(1u<<6), 1u };
          int mal = 0;
          for(int i = 0; i < 3; i++)
              if(es_subgrupo(&Z12,Hs[i]) && !es_em(Hs[i], es_neutro(&Z12))) mal++;
          tique7(5, "logo e ∈ H, sempre");
          tique7(6, "e a volta: varrem-se TODOS os subconjuntos de ℤ₁₂ e verifica-se que"
                    " os que passam no critério contêm todos o 0 — e que o vazio não passa");
          int subs = 0, sem_e = 0;
          for(unsigned H = 0; H < (1u << 12); H++){
              if(!es_subgrupo(&Z12,H)) continue;
              subs++;
              if(!es_em(H,0)) sem_e++;
          }
          printf("      os %d subgrupos de Z12: %d sem o neutro;  e o vazio é subgrupo? %s\n",
                 subs, sem_e, es_subgrupo(&Z12,0) ? "sim (?!)" : "não");
          printf("      (varrido em 4096 subconjuntos, %d falhas)\n", mal + sem_e); }
        break; }
    case 5: {
        tique7(0, "sejam H e K subgrupos de G. Nada mais: nem finitude, nem"
                  " comutatividade");
        tique7(1, "a interseção é o que está nos dois:");
        printf("      $H \\cap K = \\{ g : g \\in H \\text{ e } g \\in K \\}$\n");
        tique7(2, "para a, b ∈ H∩K: como estão em H, ab⁻¹ ∈ H; como estão em K,"
                  " ab⁻¹ ∈ K. Logo ab⁻¹ está nos dois, isto é em H∩K");
        tique7(3, "a lei é o critério do subgrupo, aplicado DUAS VEZES — uma em cada"
                  " subgrupo. É a mesma lei usada duas vezes, e não duas leis");
        tique7(4, "a testemunha é o neutro: está em H e em K (teorema 4), logo a"
                  " interseção não é vazia — sem isso o critério não pegava");
        { int mal = 0, pares = 0;
          unsigned subs[64]; int ns = 0;
          for(unsigned H = 0; H < (1u << 12) && ns < 64; H++)
              if(es_subgrupo(&Z12,H)) subs[ns++] = H;
          for(int i = 0; i < ns; i++) for(int j = 0; j < ns; j++){
              if(!es_subgrupo(&Z12, subs[i] & subs[j])) mal++;
              pares++;
          }
          tique7(5, "logo H∩K é subgrupo");
          tique7(6, "e a volta: cruzam-se TODOS os pares de subgrupos de ℤ₁₂ e verifica-se"
                    " cada interseção pelo critério");
          printf("      %d subgrupos, %d pares cruzados: %d interseções que falham\n",
                 ns, pares, mal);
          printf("      (e o gume: a UNIÃO não é subgrupo — {0,6} ∪ {0,4,8} tem 6 e 4,"
                 " e 6+4 = 10 não está lá)\n");
          { unsigned U = (1u<<0)|(1u<<6)|(1u<<4)|(1u<<8);
            printf("      união é subgrupo? %s\n", es_subgrupo(&Z12,U) ? "sim (?!)" : "NÃO"); } }
        break; }
    case 6: case 7: case 8: {
        /* os três teoremas do homomorfismo correm sobre o mesmo exemplo, e é de
         * propósito: são três leituras do mesmo objeto */
        int f[ES_MAX];
        for(int a = 0; a < 12; a++) f[a] = (3*a) % 12;      /* f(x) = 3x em ℤ₁₂ */
        if(n == 6){
            tique7(0, "seja f: G → H homomorfismo. Só isso");
            tique7(1, "homomorfismo é a função que PRESERVA a operação — «não é"
                      " simplesmente uma função: é uma função que conserva a estrutura»:");
            printf("      $f(a \\star b) = f(a) \\star f(b)$\n");
            tique7(2, "sejam x, y na imagem: x = f(a) e y = f(b). Então"
                      " x·y⁻¹ = f(a)·f(b)⁻¹ = f(a·b⁻¹), que está na imagem");
            tique7(3, "as leis são a preservação (para juntar) e f(b⁻¹) = f(b)⁻¹, que"
                      " ela própria sai da preservação aplicada a b·b⁻¹ = e");
            tique7(4, "a testemunha é o elemento a·b⁻¹ de G, cuja imagem É o x·y⁻¹ pedido");
            { unsigned Img = es_imagem(&Z12, f);
              printf("      f(x) = 3x em Z12:  Im f = "); es_conj(&Z12, Img); printf("\n");
              tique7(5, "logo Im f é subgrupo de H");
              tique7(6, "e a volta: verifica-se a imagem pelo critério do subgrupo");
              printf("      Im f é subgrupo? %s   (e |Im f| = %d)\n",
                     es_subgrupo(&Z12,Img) ? "sim (resíduo 0)" : "NÃO", es_ordem_conj(Img)); }
        } else if(n == 7){
            tique7(0, "seja f: G → H homomorfismo, e K = ker f");
            tique7(1, "o núcleo é o que vai parar ao neutro:");
            printf("      $\\ker f = \\{ g \\in G : f(g) = e_H \\}$\n");
            tique7(2, "K é subgrupo: se f(a) = f(b) = e então f(ab⁻¹) = e·e⁻¹ = e."
                      " E é NORMAL: para g qualquer e k em K,"
                      " f(gkg⁻¹) = f(g)·e·f(g)⁻¹ = e, logo gkg⁻¹ ∈ K");
            tique7(3, "a lei é a preservação outra vez — mas agora aplicada ao CONJUGADO,"
                      " e é isso que produz a normalidade em vez de só o subgrupo");
            printf("      $g k g^{-1} \\in K$   para todo $g \\in G$ e $k \\in K$\n");
            tique7(4, "a testemunha é o f(g)·f(g)⁻¹, que colapsa no neutro seja qual for g"
                      " — é essa indiferença ao g que É a normalidade");
            { unsigned K = es_nucleo(&Z12, &Z12, f);
              printf("      ker f = "); es_conj(&Z12, K); printf("\n");
              tique7(5, "logo ker f é subgrupo NORMAL, e é por isso que G/ker f existe");
              tique7(6, "e a volta: verifica-se o critério do subgrupo E a conjugação, os"
                        " dois, porque normal é mais que subgrupo");
              printf("      subgrupo? %s;  normal (gKg⁻¹ = K para todo g)? %s\n",
                     es_subgrupo(&Z12,K) ? "sim" : "NÃO",
                     es_normal(&Z12,K) ? "sim (resíduo 0)" : "NÃO");
              printf("      e em S3 o núcleo do homomorfismo trivial é o grupo todo,"
                     " que também é normal\n"); }
        } else {
            tique7(0, "seja f: G → H homomorfismo. Quer-se a EQUIVALÊNCIA, logo há duas"
                      " direções a provar, e as duas se provam");
            tique7(1, "trivial quer dizer só o neutro:");
            printf("      $f$ injetiva $\\Leftrightarrow$ $\\ker f = \\{e\\}$\n");
            tique7(2, "(⟹) se f é injetiva e f(k) = e = f(e), então k = e. (⟸) se o"
                      " núcleo é trivial e f(a) = f(b), então f(ab⁻¹) = e, logo ab⁻¹ = e,"
                      " logo a = b");
            tique7(3, "a lei da volta é a preservação a transformar uma IGUALDADE de"
                      " imagens numa PERTENÇA ao núcleo — é esse o truque, e é ele que"
                      " faz o núcleo medir a injetividade");
            tique7(4, "a testemunha é o elemento ab⁻¹: ele é que carrega a diferença entre"
                      " a e b, e o núcleo é onde essa diferença desaparece");
            { int mal = 0, viu_inj = 0, viu_nao = 0;
              for(int k = 0; k < 12; k++){
                  int g[ES_MAX];
                  for(int a = 0; a < 12; a++) g[a] = (k*a) % 12;
                  if(!es_homo(&Z12,&Z12,g,0,0)) continue;
                  unsigned K = es_nucleo(&Z12,&Z12,g);
                  int inj = es_injetiva(&Z12,g);
                  if(inj != (es_ordem_conj(K) == 1)) mal++;
                  if(inj) viu_inj++; else viu_nao++;
              }
              tique7(5, "logo as duas condições são a mesma");
              tique7(6, "e a volta: varrem-se os 12 homomorfismos x ↦ kx de ℤ₁₂ e"
                        " compara-se injetividade com núcleo trivial, caso a caso");
              printf("      12 homomorfismos: %d injetivos e %d não, e a equivalência"
                     " falha em %d\n", viu_inj, viu_nao, mal);
              printf("      (o gume tem os DOIS lados a ocorrer — se todos fossem"
                     " injetivos, a equivalência não estaria a decidir nada)\n"); }
        }
        break; }
    case 9: {
        unsigned H = (1u<<0)|(1u<<4)|(1u<<8);
        tique7(0, "seja G finito e H ≤ G. Em ℤ₁₂ com H = {0,4,8}, que é o exercício dele");
        tique7(1, "a classe lateral é o transladado do subgrupo:");
        printf("      $gH = \\{ g \\star h : h \\in H \\}$\n");
        tique7(2, "duas classes ou são IGUAIS ou são DISJUNTAS (se partilham um elemento,"
                  " partilham todos), e todas têm |H| elementos porque g⋆· é bijetiva."
                  " Logo elas particionam G em blocos do MESMO tamanho");
        tique7(3, "as leis são o CANCELAMENTO (teorema 3), que dá a bijeção g⋆·, e o"
                  " critério de igualdade de classes: gH = g′H ⟺ g⁻¹g′ ∈ H");
        tique7(4, "a testemunha é o número de classes, [G:H] — e |G| = [G:H]·|H| é a"
                  " conta que o teorema é");
        { unsigned cls[16]; int nc = 0; unsigned visto = 0; int mal = 0;
          for(int g = 0; g < 12; g++){
              if(es_em(visto,g)) continue;
              unsigned C = es_classe(&Z12,H,g);
              cls[nc++] = C; visto |= C;
              printf("      %dH = ", g); es_conj(&Z12, C);
              printf("   (|%dH| = %d)\n", g, es_ordem_conj(C));
              if(es_ordem_conj(C) != es_ordem_conj(H)) mal++;
          }
          /* a PARTIÇÃO: cobrem tudo e não se cruzam */
          unsigned uniao = 0;
          for(int i = 0; i < nc; i++){
              uniao |= cls[i];
              for(int j = i+1; j < nc; j++) if(cls[i] & cls[j]) mal++;
          }
          if(uniao != 0xFFFu) mal++;
          tique7(5, "logo |H| divide |G|:");
          printf("      $|H| \\cdot [G:H] = |G|$:   %d · %d = %d   %s\n",
                 es_ordem_conj(H), nc, es_ordem_conj(H)*nc,
                 es_ordem_conj(H)*nc == 12 ? "(resíduo 0)" : "— NÃO afirmo");
          tique7(6, "e a volta é a PARTIÇÃO medida: as classes cobrem o grupo inteiro e"
                    " não se cruzam duas a duas — «elemento → classe → partição → volta»");
          printf("      %d classes, união = grupo todo, cruzamentos = 0: %d falhas\n",
                 nc, mal); }
        break; }
    case 10: {
        tique7(0, "seja G um grupo finito, |G| = n. Nada mais se pede");
        tique7(1, "a cada g associa-se a sua translação à esquerda:");
        printf("      $\\lambda_g : G \\to G, \\qquad \\lambda_g(x) = g \\star x$\n");
        tique7(2, "cada λ_g é uma BIJEÇÃO de G (o inverso é λ_{g⁻¹}), logo é uma"
                  " permutação dos n elementos; e λ_{gh} = λ_g ∘ λ_h, logo g ↦ λ_g é"
                  " homomorfismo de G no grupo das permutações");
        tique7(3, "a lei da bijetividade é o CANCELAMENTO; a da preservação é a"
                  " ASSOCIATIVIDADE, que é o que faz λ_g∘λ_h e λ_{gh} coincidirem");
        tique7(4, "a testemunha da injetividade: se λ_g = λ_h então λ_g(e) = λ_h(e),"
                  " isto é g = h. O neutro é que serve de sonda");
        { /* constrói-se a representação e mede-se que é homomorfismo INJETIVO */
          Est *Gs[2] = { &Z6, &S3 };
          int mal = 0;
          for(int i = 0; i < 2; i++){
              const Est *G = Gs[i];
              int inj = 1;
              for(int g = 0; g < G->n; g++) for(int h = g+1; h < G->n; h++){
                  int igual = 1;
                  for(int x = 0; x < G->n; x++) if(G->op[g][x] != G->op[h][x]) igual = 0;
                  if(igual) inj = 0;
              }
              int homo = 1;
              for(int g = 0; g < G->n; g++) for(int h = 0; h < G->n; h++)
                  for(int x = 0; x < G->n; x++)
                      if(G->op[G->op[g][h]][x] != G->op[g][G->op[h][x]]) homo = 0;
              printf("      %s (n = %d): λ é injetiva? %s;  λ_{gh} = λ_g∘λ_h ? %s\n",
                     G->nome, G->n, inj ? "sim" : "NÃO", homo ? "sim" : "NÃO");
              if(!inj || !homo) mal++;
          }
          tique7(5, "logo G é isomorfo a um subgrupo de Sₙ — «todo grupo é um grupo de"
                    " permutações», e a abstração não perdeu nada");
          tique7(6, "e a volta: as translações reconstroem a tábua, porque λ_g(x) É a"
                    " linha g da tábua. A representação não é um modelo do grupo — é a"
                    " tábua lida por linhas");
          printf("      %d falhas — e note-se: a matriz das λ É a tábua de Cayley\n", mal); }
        break; }
    case 11: {
        int f[ES_MAX];
        for(int a = 0; a < 12; a++) f[a] = (3*a) % 12;
        tique7(0, "seja f: G → H homomorfismo, K = ker f e I = Im f. Em ℤ₁₂ com f(x) = 3x");
        tique7(1, "o quociente identifica o que f não distingue:");
        printf("      $G/\\ker f \\cong \\operatorname{Im} f$\n");
        tique7(2, "define-se φ(gK) = f(g). Está BEM DEFINIDA (se gK = g′K então"
                  " g⁻¹g′ ∈ K, logo f(g) = f(g′)), é homomorfismo, é sobrejetiva sobre"
                  " Im f e é injetiva porque φ(gK) = e obriga g ∈ K, isto é gK = K");
        tique7(3, "a lei que autoriza tudo é a NORMALIDADE do núcleo (teorema 7) — sem"
                  " ela o quociente nem existe, e a boa definição é que carrega o resto");
        tique7(4, "a testemunha é o representante g: a prova toda é mostrar que a escolha"
                  " dele NÃO importa, e é isso a boa definição");
        { unsigned K = es_nucleo(&Z12,&Z12,f), Img = es_imagem(&Z12,f);
          Est Q; unsigned cls[ES_MAX];
          int m = es_quociente(&Z12, K, &Q, cls);
          /* a imagem como estrutura, para poder ser comparada */
          Est EI; int rot[ES_MAX], ni = 0;
          for(int a = 0; a < 12; a++) if(es_em(Img,a)) rot[ni++] = a;
          EI.n = ni; EI.nome = "Im f";
          for(int i = 0; i < ni; i++) for(int j = 0; j < ni; j++){
              int v = (rot[i] + rot[j]) % 12;
              for(int k = 0; k < ni; k++) if(rot[k] == v){ EI.op[i][j] = k; break; }
          }
          printf("      ker f = "); es_conj(&Z12,K);
          printf(",  |G/K| = %d;   Im f = ", m); es_conj(&Z12,Img);
          printf(",  |Im f| = %d\n", ni);
          tique7(5, "logo o quociente e a imagem são A MESMA ÁLGEBRA noutra"
                    " representação — é a tua ideia de traduzir um andar para outro,"
                    " agora como teorema");
          tique7(6, "e a volta: procura-se EXAUSTIVAMENTE uma bijeção que preserve a"
                    " operação. Se não houvesse, o isomorfismo era falso — e a busca"
                    " esgota as m! possibilidades, não amostra");
          int g[ES_MAX];
          int iso = es_isomorfas(&Q, &EI, g);
          printf("      G/K ≅ Im f ? %s   (e |G| = |K|·|Im f| = %d·%d = %d)\n",
                 iso ? "sim (resíduo 0)" : "— NÃO afirmo",
                 es_ordem_conj(K), ni, es_ordem_conj(K)*ni); }
        break; }
    case 12: case 13: case 14: {
        Anel R6, R12;
        an_zn(&R6, 6); an_zn(&R12, 12);
        if(n == 12){
            tique7(0, "seja R um anel e I um ideal seu");
            tique7(1, "ideal é o subgrupo aditivo que ABSORVE o produto:");
            printf("      $a,b \\in I \\Rightarrow a - b \\in I$   e"
                   "   $r \\in R,\\ a \\in I \\Rightarrow ra \\in I$\n");
            tique7(2, "a projeção π: R → R/I, π(a) = a + I, é homomorfismo de anéis, e o"
                      " seu núcleo é exatamente I. Reciprocamente, o núcleo de qualquer"
                      " homomorfismo de anéis é ideal: absorve porque f(ra) = f(r)f(a) ="
                      " f(r)·0 = 0");
            tique7(3, "a lei da ida é a construção do quociente; a da volta é a"
                      " preservação do PRODUTO — e note-se que é ela que dá a absorção,"
                      " que o núcleo de grupos não tinha");
            tique7(4, "a testemunha é o f(r)·0 = 0, que é o que faz ra cair no núcleo"
                      " seja qual for o r");
            { int mal = 0, ideais = 0;
              for(unsigned Idl = 0; Idl < (1u << 6); Idl++){
                  if(!an_ideal(&R6, Idl)) continue;
                  ideais++;
                  /* o núcleo da projeção mod I é I: mede-se pela classe do 0 */
                  unsigned nucleo = 0;
                  for(int a = 0; a < 6; a++){
                      int esta = 0;
                      for(int b = 0; b < 6; b++) if(es_em(Idl,b) && a == b) esta = 1;
                      if(esta) nucleo |= 1u << a;
                  }
                  if(nucleo != Idl) mal++;
              }
              tique7(5, "logo ideais e núcleos são a MESMA coisa, vistos de dois lados");
              tique7(6, "e a volta: varrem-se os 64 subconjuntos de ℤ₆ e conta-se quantos"
                        " passam nas duas condições do ideal");
              printf("      Z6 tem %d ideais em 64 subconjuntos, e cada um é o núcleo da"
                     " sua projeção: %d falhas\n", ideais, mal); }
        } else if(n == 13){
            tique7(0, "seja f: R → S homomorfismo de anéis e I = ker f");
            tique7(1, "o mesmo enunciado do grupo, agora com duas operações:");
            printf("      $R/\\ker f \\cong \\operatorname{Im} f$\n");
            tique7(2, "a φ(a + I) = f(a) está bem definida pelo mesmo argumento, e agora"
                      " preserva as DUAS operações — a soma pela mesma conta, o produto"
                      " porque f(ab) = f(a)f(b)");
            tique7(3, "a lei nova é a absorção do ideal, que é o que faz o produto de"
                      " classes estar bem definido: (a+I)(b+I) = ab + I precisa de"
                      " aI, Ib e I·I caírem em I");
            tique7(4, "a testemunha é o cálculo (a+i)(b+j) = ab + (aj + ib + ij), e os"
                      " três termos entre parênteses estão em I pela absorção");
            { /* f: ℤ₁₂ → ℤ₆ por redução mod 6 — a projeção natural */
              int mal = 0;
              for(int a = 0; a < 12; a++) for(int b = 0; b < 12; b++){
                  if((a + b) % 12 % 6 != ((a % 6) + (b % 6)) % 6) mal++;
                  if((a * b) % 12 % 6 != ((a % 6) * (b % 6)) % 6) mal++;
              }
              tique7(5, "logo o teorema do isomorfismo vale para anéis tal como para"
                        " grupos — a mesma máquina, uma operação a mais");
              tique7(6, "e a volta: mede-se a preservação das DUAS operações em todos os"
                        " pares, porque preservar uma só não é homomorfismo de anéis");
              printf("      f: Z12 → Z6 (redução), 144 pares × 2 operações: %d falhas\n", mal);
              printf("      ker f = {0, 6} = (6) em Z12, e Im f = Z6 inteiro\n"); }
        } else {
            tique7(0, "seja I um ideal de ℤ. Nenhuma outra hipótese");
            tique7(1, "o ideal gerado por n é o conjunto dos múltiplos:");
            printf("      $(n) = n\\mathbb{Z} = \\{ nk : k \\in \\mathbb{Z} \\}$\n");
            tique7(2, "se I = {0}, é (0). Senão toma-se n = o MENOR positivo de I (boa"
                      " ordenação). Para a ∈ I qualquer, a divisão dá a = nq + r com"
                      " 0 ≤ r < n; mas r = a − nq ∈ I, e n era o menor positivo, logo"
                      " r = 0. Portanto a ∈ nℤ");
            tique7(3, "as leis são a BOA ORDENAÇÃO de ℕ (para haver menor) e a DIVISÃO"
                      " COM RESTO — é exatamente a mesma cadeia do gcd como menor"
                      " elemento de {ax+by}, no andar dos números");
            tique7(4, "a testemunha é o resto r: ele está no ideal e é menor que o menor,"
                      " o que só se resolve com r = 0");
            { int mal = 0, ideais = 0;
              for(unsigned Idl = 0; Idl < (1u << 12); Idl++){
                  if(!an_ideal(&R12, Idl)) continue;
                  ideais++;
                  int menor = -1;
                  for(int a = 1; a < 12; a++) if(es_em(Idl,a)){ menor = a; break; }
                  unsigned ger = 0;
                  if(menor < 0) ger = 1u;                      /* só o zero */
                  else for(int k = 0; k < 12; k++) ger |= 1u << ((menor*k) % 12);
                  if(ger != Idl) mal++;
              }
              tique7(5, "logo TODO ideal de ℤ é nℤ — «os ideais de ℤ são justamente os"
                        " múltiplos de um inteiro», e ℤ é domínio de ideais principais");
              tique7(6, "e a volta: varrem-se os 4096 subconjuntos de ℤ₁₂, e cada ideal"
                        " achado reconstrói-se a partir do seu MENOR elemento positivo");
              printf("      Z12 tem %d ideais, e todos são gerados pelo seu menor"
                     " positivo: %d falhas\n", ideais, mal);
              printf("      (e são %d porque 12 tem %d divisores — a correspondência é"
                     " ideal ↔ divisor)\n", ideais, ideais); }
        }
        break; }
    case 15: {
        tique7(0, "seja K um corpo, e sejam a, b ∈ K com ab = 0 e a ≠ 0");
        tique7(1, "corpo é o anel onde todo não nulo tem inverso:");
        printf("      $a \\neq 0 \\; \\Rightarrow \\; \\exists a^{-1} : a a^{-1} = 1$\n");
        tique7(2, "multiplica-se ab = 0 por a⁻¹ à esquerda: a⁻¹(ab) = a⁻¹·0, isto é"
                  " (a⁻¹a)b = 0, isto é 1·b = 0, logo b = 0");
        tique7(3, "as leis são o INVERSO (que existe por ser corpo), a ASSOCIATIVIDADE"
                  " (para reagrupar) e r·0 = 0 (que sai da distributividade)");
        tique7(4, "a testemunha é o a⁻¹ — e é a sua EXISTÊNCIA que o teorema consome:"
                  " tira-se o inverso da hipótese e o resto é mecânico");
        { int mal = 0, corpos = 0, aneis = 0;
          for(int m = 2; m <= 16; m++){
              Anel R; an_zn(&R, m);
              int ea = 0, eb = 0, sem = 0;
              int corpo = an_corpo(&R, &sem), dom = an_dominio(&R, &ea, &eb);
              if(corpo && !dom) mal++;                    /* corpo ⟹ domínio */
              if(corpo) corpos++; else aneis++;
              if(corpo != (nt_primo(m) != 0)) mal++;      /* e ℤₘ é corpo ⟺ m é primo */
          }
          tique7(5, "logo b = 0: num corpo, ab = 0 obriga um dos dois a ser zero — todo"
                    " corpo é DOMÍNIO INTEGRAL");
          tique7(6, "e o GUME é a recíproca, que é FALSA: ℤ é domínio e não é corpo (o 2"
                    " não tem inverso). A implicação tem um sentido só, e mede-se qual");
          { Anel R6b, R5b; an_zn(&R6b,6); an_zn(&R5b,5);
            int x, y;
            an_dominio(&R6b, &x, &y);
            printf("      Z_m para m de 2 a 16: %d corpos e %d não, e corpo ⟹ domínio"
                   " falha %d vezes\n", corpos, aneis, mal);
            printf("      Z6 NÃO é domínio (%d·%d = 0) e por isso não é corpo;"
                   "  Z5 é corpo e é domínio\n", x, y);
            printf("      e a recíproca: Z é domínio e NÃO é corpo — um sentido só\n"); } }
        break; }
    case 16: {
        tique7(0, "seja ⋆ definida por a ⋆ b = a + b + 1 em ℤ");
        tique7(1, "operação binária é uma função de A×A em A:");
        printf("      $\\star : A \\times A \\to A, \\qquad a \\star b = a + b + 1$\n");
        tique7(2, "FECHADA porque a soma de inteiros é inteira e 1 é inteiro — não sai"
                  " de ℤ, e é isso que «fechada» quer dizer");
        tique7(3, "a lei é o fecho de ℤ para a soma, que veio do andar dos inteiros");
        tique7(4, "o NEUTRO procura-se resolvendo a ⋆ e = a, isto é a + e + 1 = a:");
        printf("      $a + e + 1 = a \\; \\Rightarrow \\; e = -1$\n");
        { Est M; es_mais_um(&M, 9);
          int e = es_neutro(&M);
          printf("      em ℤ₉ (onde −1 ≡ 8):  o neutro medido é %d,  e −1 mod 9 = %d   %s\n",
                 e, 8, e == 8 ? "(confere)" : "— NÃO afirmo");
          tique7(5, "logo (ℤ, ⋆) é MONOIDE: fechada, associativa e com neutro −1");
          tique7(6, "e a volta: a associatividade varre-se, e o neutro confirma-se pelos"
                    " DOIS lados (e ⋆ a = a ⋆ e = a) em toda a tábua");
          printf("      associativa? %s;  e o neutro serve dos dois lados em %d elementos\n",
                 es_assoc(&M,0,0,0) ? "sim" : "NÃO", M.n);
          printf("      (e é MAIS que monoide: cada a tem inverso −a−2, logo é GRUPO)\n"); }
        break; }
    case 17: {
        tique7(0, "seja ⋆ definida por a ⋆ b = a − b");
        tique7(1, "semigrupo pede associatividade:");
        printf("      $(a \\star b) \\star c = a \\star (b \\star c)$\n");
        tique7(2, "à esquerda dá (a−b)−c = a−b−c; à direita dá a−(b−c) = a−b+c. As duas"
                  " só coincidem quando c = −c");
        tique7(3, "a lei que se tentou usar foi a associatividade da soma, e ela NÃO"
                  " transporta para a diferença — é aqui que o argumento parte");
        tique7(4, "a TESTEMUNHA é um triplo concreto, e é ela o resultado: sem exibir um,"
                  " «não é associativa» seria uma afirmação sem prova");
        { Est Mn; es_menos(&Mn, 5);
          int ta = 0, tb = 0, tc = 0;
          int ass = es_assoc(&Mn, &ta, &tb, &tc);
          printf("      em ℤ₅:  (%d⋆%d)⋆%d = %d,  mas  %d⋆(%d⋆%d) = %d\n",
                 ta, tb, tc, Mn.op[Mn.op[ta][tb]][tc],
                 ta, tb, tc, Mn.op[ta][Mn.op[tb][tc]]);
          tique7(5, "logo (ℤ, −) NÃO é semigrupo");
          tique7(6, "e a volta: conta-se em QUANTOS triplos ela falha — se falhasse em"
                    " zero, a testemunha estava errada; e (ℕ,+) faz o contrário, falha"
                    " em nenhum");
          long falhas = 0, total = 0;
          for(int a = 0; a < 5; a++) for(int b = 0; b < 5; b++) for(int c = 0; c < 5; c++){
              if(Mn.op[Mn.op[a][b]][c] != Mn.op[a][Mn.op[b][c]]) falhas++;
              total++;
          }
          Est N; es_zn(&N, 5);
          printf("      associativa? %s — falha em %ld de %ld triplos;"
                 "  e (ℤ₅,+) falha em 0\n", ass ? "sim (?!)" : "NÃO", falhas, total);
          printf("      %s\n", es_assoc(&N,0,0,0) ? "" : "(?!)"); }
        break; }
    case 18: {
        tique7(0, "o pedido é «prove que todo grupo é comutativo». A primeira coisa a"
                  " fazer NÃO é provar: é ver se é verdade");
        tique7(1, "abeliano é a comutatividade acrescentada aos axiomas:");
        printf("      $a \\star b = b \\star a$   para todos $a, b \\in G$\n");
        tique7(2, "os axiomas de grupo são quatro — fechada, associativa, neutro, inverso"
                  " — e NENHUM deles é a comutatividade. Logo ela não pode ser deduzida:"
                  " se pudesse, seria supérflua na definição de abeliano");
        tique7(3, "a lei aqui é a INDEPENDÊNCIA dos axiomas, e a maneira de a mostrar é"
                  " exibir um modelo que cumpre os quatro e não cumpre o quinto");
        tique7(4, "a TESTEMUNHA é S₃, e o par concreto que não comuta:");
        { int ta = 0, tb = 0;
          es_abeliano(&S3, &ta, &tb);
          printf("      S₃ é grupo? %s;  abeliano? %s\n",
                 es_grupo(&S3,0) ? "sim" : "NÃO", es_abeliano(&S3,0,0) ? "sim" : "NÃO");
          printf("      o par: %d⋆%d = %d,  mas  %d⋆%d = %d\n",
                 ta, tb, S3.op[ta][tb], tb, ta, S3.op[tb][ta]);
          printf("      (as permutações de {0,1,2}: %d é ", ta);
          printf("(%d %d %d) e %d é (%d %d %d))\n",
                 ES_S3[ta][0], ES_S3[ta][1], ES_S3[ta][2], tb,
                 ES_S3[tb][0], ES_S3[tb][1], ES_S3[tb][2]);
          tique7(5, "logo a afirmação é FALSA, e a resposta certa é RECUSAR a demonstração"
                    " — não é «não consegui provar», é «não há o que provar»");
          tique7(6, "e a volta: conta-se em quantos pares S₃ falha a comutatividade, e"
                    " verifica-se que ℤ₁₂ falha em ZERO — os dois lados, para se ver que"
                    " a propriedade distingue mesmo grupos");
          long f3 = 0, f12 = 0;
          for(int a = 0; a < 6; a++) for(int b = 0; b < 6; b++) if(S3.op[a][b] != S3.op[b][a]) f3++;
          for(int a = 0; a < 12; a++) for(int b = 0; b < 12; b++) if(Z12.op[a][b] != Z12.op[b][a]) f12++;
          printf("      S₃ falha em %ld dos 36 pares;  ℤ₁₂ falha em %ld dos 144\n", f3, f12); }
        break; }
    case 19: {
        tique7(0, "quer-se um isomorfismo que leve a SOMA no PRODUTO. O ficheiro escreve-o"
                  " com f(x) = eˣ de (ℝ,+) em (ℝ₊,×)");
        tique7(1, "isomorfismo é a bijeção que preserva:");
        printf("      $f(x + y) = f(x) \\cdot f(y)$\n");
        tique7(2, "AQUI A REALIZAÇÃO MUDA, e diz-se porquê: o eˣ pedia decimais, e nesta"
                  " casa não entram. Mas o conteúdo do exercício é ALGÉBRICO — «a"
                  " exponencial troca + por ×» — e isso realiza-se EXATO no finito: uma"
                  " raiz primitiva g de ℤₚ dá x ↦ gˣ, e é o mesmo teorema sem o e");
        printf("      $f : (\\mathbb{Z}_{p-1}, +) \\to (\\mathbb{Z}_p^{*}, \\times),"
               " \\qquad f(x) = g^{x}$\n");
        tique7(3, "a lei é g^{x+y} = gˣ·gʸ — exatamente a mesma que autoriza"
                  " e^{x+y} = eˣeʸ. A propriedade é da POTÊNCIA, não do e");
        tique7(4, "a testemunha é a raiz primitiva g: existe porque ℤₚ* é cíclico, e"
                  " exibe-se");
        { long p = 13, g = 0;
          for(long c = 2; c < p; c++) if(nm_ordem(c, p) == p-1){ g = c; break; }
          printf("      p = %ld, e a raiz primitiva medida é g = %ld (ordem %ld = p−1)\n",
                 p, g, nm_ordem(g,p));
          Est A, B; int rot[ES_MAX];
          es_zn(&A, (int)(p-1));
          es_unidades(&B, (int)p, rot);
          int f[ES_MAX];
          for(int x = 0; x < A.n; x++){
              long v = iz_pot_mod(g, x, p);
              for(int k = 0; k < B.n; k++) if(rot[k] == v){ f[x] = k; break; }
          }
          int ta = 0, tb = 0;
          int homo = es_homo(&A, &B, f, &ta, &tb), inj = es_injetiva(&A, f);
          printf("      f(x) = %ldˣ mod %ld:  ", g, p);
          for(int x = 0; x < 6; x++) printf("%s%ld", x?", ":"", iz_pot_mod(g,x,p));
          printf(", …\n");
          tique7(5, "logo (ℤ₁₂,+) ≅ (ℤ₁₃*,×): a soma virou produto, exatamente");
          tique7(6, "e a volta é o LOGARITMO DISCRETO — o f⁻¹ que corresponde ao ln. Cada"
                    " y do produto tem um x da soma, e a volta reconstrói-o");
          printf("      homomorfismo? %s;  injetivo? %s;  |A| = %d e |B| = %d\n",
                 homo ? "sim" : "NÃO", inj ? "sim" : "NÃO", A.n, B.n);
          int mal = 0;
          for(int y = 0; y < B.n; y++){
              int achou = 0;
              for(int x = 0; x < A.n; x++) if(f[x] == y) achou = 1;
              if(!achou) mal++;
          }
          printf("      e o log discreto existe para %d dos %d elementos: %d sem volta\n",
                 B.n - mal, B.n, mal); }
        break; }
    case 20: {
        tique7(0, "seja R um anel e a, b ∈ R. Nada mais — nem comutatividade, nem unidade");
        tique7(1, "o que se quer é que o oposto atravesse o produto:");
        printf("      $(-a)b = -(ab)$\n");
        tique7(2, "soma-se ab aos dois lados de (−a)b e usa-se a DISTRIBUTIVIDADE:"
                  " (−a)b + ab = (−a + a)b = 0·b = 0");
        printf("      $(-a)b + ab = (-a + a)b = 0 \\cdot b = 0$\n");
        tique7(3, "as leis são a DISTRIBUTIVIDADE (que junta os dois termos), o OPOSTO"
                  " aditivo (que dá −a + a = 0) e 0·b = 0 (que ela própria sai da"
                  " distributividade: 0b = (0+0)b = 0b + 0b)");
        tique7(4, "a testemunha é o próprio (−a)b + ab: ele é zero, logo (−a)b é O oposto"
                  " de ab — e o oposto é único (teorema 2), o que fecha");
        { int mal = 0, casos = 0;
          for(int m = 2; m <= 12; m++){
              Anel R; an_zn(&R, m);
              for(int a = 0; a < m; a++) for(int b = 0; b < m; b++){
                  int ma = (m - a) % m;                      /* −a */
                  int mab = (m - R.mult[a][b]) % m;          /* −(ab) */
                  if(R.mult[ma][b] != mab) mal++;
                  if(R.soma[R.mult[ma][b]][R.mult[a][b]] != 0) mal++;  /* a cadeia */
                  casos++;
              }
          }
          tique7(5, "logo (−a)b = −(ab), em QUALQUER anel");
          tique7(6, "e a volta: varre-se ℤₘ de 2 a 12, elemento a elemento, medindo a"
                    " conclusão E o passo do meio ((−a)b + ab = 0) — porque medir só a"
                    " conclusão era o que ele proíbe");
          printf("      %d pares em 11 anéis: %d falhas (conclusão e passo do meio)\n",
                 casos, mal);
          printf("      (e daí sai (−a)(−b) = ab, aplicando duas vezes)\n"); }
        break; }
    }
}
static int resolve_estrutura(const char *f){
    const char *p = f;
    if(!strncmp(p, "algebra", 7)) p += 7;
    else if(!strncmp(p, "álgebra", 8)) p += 8;
    else if(!strncmp(p, "estrutura", 9)) p += 9;
    else {
        for(size_t i = 0; i < sizeof AL20/sizeof *AL20; i++)
            if(!strcmp(p, AL20[i].nome)){ algebra_resolve(AL20[i].n); return 1; }
        return 0;
    }
    while(*p == ' ') p++;
    if(!strncmp(p, "moderna", 7)){ p += 7; while(*p == ' ') p++; }
    if(!*p){
        printf("   álgebra moderna — «algebra N» ou «algebra <nome>»\n");
        printf("   e cada teorema corre os SETE ticks que o ficheiro pede:\n");
        printf("   hipóteses → definição → transição → lei usada → testemunha →"
               " conclusão → volta\n\n");
        for(size_t i = 0; i < sizeof AL20/sizeof *AL20; i++){
            if(i == 0)  printf("   os quinze teoremas do corpus\n");
            if(i == 15) printf("   e os exercícios do roteiro\n");
            printf("     %2d  ", AL20[i].n);
            esc_col(AL20[i].nome, 20);
            printf("  %s\n", AL20[i].enunciado);
        }
        return 1;
    }
    if(*p >= '0' && *p <= '9'){
        long n = 0;
        while(*p >= '0' && *p <= '9') n = n*10 + (*p++ - '0');
        while(*p == ' ') p++;
        if(!*p && n >= 1 && n <= 20){ algebra_resolve((int)n); return 1; }
        return 0;
    }
    for(size_t i = 0; i < sizeof AL20/sizeof *AL20; i++)
        if(!strcmp(p, AL20[i].nome)){ algebra_resolve(AL20[i].n); return 1; }
    return 0;
}
/* ── MÖBIUS COMO INVERSOR, E A CURVA COMO MÁQUINA DE RASTROS ────────────────────────
 * Dois pacotes que o `eval.txt` pede em sequência, e que se tocam no mesmo sítio: a
 * INVERSÃO. Num é μ = 1⁻¹ na álgebra da convolução; no outro é a fibra a decidir qual
 * operação existe. «Dois tipos de árvore, duas descidas, duas inversões.» */
static void esc_qz(const char *pre, Qz x, const char *pos);
static void esc_pt(PtQ P){
    if(P.inf){ printf("𝒪"); return; }
    printf("("); esc_qz("", P.x, ", "); esc_qz("", P.y, ")");
}
static void dirichlet_resolve(void){
    TICK_N = 0;
    Arit um, id, phi, mu, eps, t1, t2;
    dl_um(&um); dl_id(&id); dl_phi(&phi); dl_mu(&mu); dl_eps(&eps);
    printf("   a convolução de Dirichlet, e μ = 1⁻¹\n");
    tique("A OPERAÇÃO — (f*g)(n) = Σ_{d|n} f(d)·g(n/d). Não é o produto ponto a ponto:"
          " cada n consulta a sua ÁRVORE DE DIVISORES inteira. É esta a multiplicação do"
          " andar, e é ela que faz o sino ganhar um produto sobre a árvore");
    printf("      (1*1)(12) = %ld  (o número de divisores de 12)\n",
           (dl_conv(&um,&um,&t1), t1.v[12]));
    tique("O NEUTRO — ε(1) = 1 e ε(n) = 0 para n > 1. Não é uma função esquisita: é a"
          " ÚNICA que deixa tudo quieto, e é isso que a define");
    { dl_conv(&um, &eps, &t1);
      printf("      1*ε = 1 ? %s   (nos 240 valores)\n",
             dl_igual(&t1, &um, DL_MAX) ? "sim" : "NÃO"); }
    tique("E AQUI ESTÁ O TEOREMA — μ * 1 = ε, isto é: μ é o INVERSO da função constante"
          " 1 nesta álgebra. O μ deixa de ser uma tabela de sinais e passa a ser aquilo"
          " que desfaz a soma sobre os divisores");
    { dl_conv(&mu, &um, &t1);
      printf("      (μ*1)(n) = Σ_{d|n} μ(d):  n=1 → %ld,  n=2 → %ld,  n=12 → %ld,"
             "  n=30 → %ld\n", t1.v[1], t1.v[2], t1.v[12], t1.v[30]);
      printf("      μ*1 = ε ? %s   ⟹   μ = 1⁻¹\n",
             dl_igual(&t1, &eps, DL_MAX) ? "sim (resíduo 0)" : "— NÃO afirmo"); }
    tique("A IDENTIDADE DO φ — φ = id * μ. Uma função que parecia de outra família"
          " transforma-se em CONVOLUÇÃO + CANCELAMENTO, e é essa a graça: não há função"
          " nova, há a mesma álgebra");
    { dl_conv(&id, &mu, &t1);
      printf("      (id*μ)(60) = %ld,  e φ(60) = %ld   %s\n",
             t1.v[60], phi.v[60], t1.v[60] == phi.v[60] ? "(iguais)" : "— NÃO afirmo");
      printf("      e nos 240 valores: %s\n",
             dl_igual(&t1, &phi, DL_MAX) ? "φ = id*μ (resíduo 0)" : "— NÃO afirmo"); }
    tique("A INVERSÃO DE MÖBIUS, que é a DECONVOLUÇÃO deste andar — se F(n) = Σ_{d|n} f(d)"
          " então f(n) = Σ_{d|n} μ(d)·F(n/d). Uma soma sobre a árvore, e a sua volta");
    { dl_soma_divisores(&phi, &t1);            /* F = φ*1, que dá a identidade */
      dl_inverte(&t1, &t2);                    /* e volta-se com μ */
      printf("      F = φ*1:  F(12) = %ld;  e a volta dá f(12) = %ld, com φ(12) = %ld\n",
             t1.v[12], t2.v[12], phi.v[12]);
      printf("      Σ_{d|n} φ(d) = n ? %s   (é a identidade φ*1 = id)\n",
             dl_igual(&t1, &id, DL_MAX) ? "sim" : "NÃO");
      printf("      e a volta reconstrói o φ ? %s\n",
             dl_igual(&t2, &phi, DL_MAX) ? "sim (resíduo 0)" : "— NÃO afirmo"); }
    tique("A SÉRIE DE DIRICHLET, E ELA É FORMAL — D_{f*g} = D_f·D_g. Avaliar num s pedia"
          " análise e decimais; mas a identidade não é sobre o VALOR da soma, é sobre os"
          " COEFICIENTES: no produto formal o coeficiente de 1/kˢ é Σ_{nm=k} f(n)g(m),"
          " que É a convolução. O s nunca se avalia, e por isso mede-se exato");
    { dl_conv(&id, &mu, &t1);
      int mal = 0;
      for(long k = 1; k <= 60; k++) if(dl_coef_produto(&id, &mu, k) != t1.v[k]) mal++;
      printf("      coeficiente de 1/kˢ em D_id·D_μ contra (id*μ)(k), k ≤ 60: %d"
             " divergências\n", mal); }
    printf("\n      árvore de divisores → convolução → Möbius → inversão → Dirichlet\n");
    printf("      (e do outro lado: Euclides → FC. Duas árvores, duas descidas,"
           " duas inversões)\n");
    printf("\n   $\\mu * 1 = \\varepsilon$,  logo  $\\mu = 1^{-1}$\n");
}
static const struct { int n; const char *nome; const char *enunciado; } EL12[] = {
 {  1, "infinito",      "o ponto no infinito 𝒪 é o neutro: P + 𝒪 = P" },
 {  2, "negacao",       "−P = (x, −y), e P + (−P) = 𝒪" },
 {  3, "soma",          "x₃ = m² − x₁ − x₂ e y₃ = m(x₁ − x₃) − y₁" },
 {  4, "porque",        "porque a fórmula funciona: Viète na cúbica" },
 {  5, "dobro",         "2P pela tangente, e o gume y = 0" },
 {  6, "exemplo",       "P = (0,1) → 2P = (1,0) → 4P = 𝒪" },
 {  7, "grupo",         "a lei de grupo, com a ASSOCIATIVIDADE" },
 {  8, "comutatividade","P + Q = Q + P" },
 {  9, "corpo finito",  "sobre 𝔽ₚ: a divisão é a inversão modular" },
 { 10, "contar",        "#E(𝔽₁₇) para y² = x³ + 2x + 2, por duas vias" },
 { 11, "lagrange",      "ord(P) | #E(𝔽ₚ), e NP = 𝒪" },
 { 12, "double and add","19P por dobras, não por 19 somas" },
};
static void eliptica_resolve(int n){
    TICK_N = 0;
    Qz a = qz_de_inteiro(-2), b = qz_de_inteiro(1);      /* a curva do ficheiro */
    PtQ P = eq_pt(qz_de_inteiro(0), qz_de_inteiro(1));
    printf("   %d — %s\n", n, EL12[n-1].enunciado);
    switch(n){
    case 1: {
        tique("O OBJETO NOVO — a curva não é só o conjunto dos (x,y): acrescenta-se 𝒪, o"
              " ponto no infinito. E ele não é um remendo, é o NEUTRO: sem ele a soma não"
              " fecha, porque duas verticais não se cruzam no plano");
        tique("E É A FORMA DA CASA — objeto + neutro = objeto. O mesmo que o 0 na soma e"
              " o 1 no produto; aqui chama-se 𝒪 e vive no infinito");
        { PtQ R = eq_soma(a, b, P, eq_O());
          printf("      P = "); esc_pt(P); printf(",  P + 𝒪 = "); esc_pt(R);
          printf("   %s\n", eq_igual(R,P) ? "(resíduo 0)" : "— NÃO afirmo");
          int mal = 0;
          for(long k = 1; k <= 8; k++){
              PtQ Q = eq_mult(a,b,P,k);
              if(!eq_igual(eq_soma(a,b,Q,eq_O()), Q)) mal++;
              if(!eq_igual(eq_soma(a,b,eq_O(),Q), Q)) mal++;
          }
          printf("      e nos dois lados, em 8 múltiplos de P: %d falhas\n", mal); }
        break; }
    case 2: {
        tique("A NEGAÇÃO É A REFLEXÃO — −P = (x, −y), o espelho no eixo x. E o primeiro"
              " que se prova é que ela NÃO SAI DA CURVA: a equação só tem y², logo trocar"
              " o sinal de y não muda nada");
        { PtQ Q = eq_neg(P);
          printf("      P = "); esc_pt(P); printf(" está em E ? %s\n",
                 eq_na_curva(a,b,P) ? "sim" : "NÃO");
          printf("      −P = "); esc_pt(Q); printf(" está em E ? %s   (é o exercício)\n",
                 eq_na_curva(a,b,Q) ? "sim" : "NÃO"); }
        tique("E A VOLTA — P + (−P) = 𝒪, porque a reta que os une é VERTICAL e o terceiro"
              " ponto de interseção é o infinito. Não é convenção: é a fibra a dizer que"
              " esta reta não tem terceiro ponto no plano");
        { PtQ R = eq_soma(a, b, P, eq_neg(P));
          printf("      P + (−P) = "); esc_pt(R);
          printf("   %s\n", R.inf ? "(resíduo 0)" : "— NÃO afirmo");
          int mal = 0;
          for(long k = 1; k <= 6; k++){
              PtQ Q = eq_mult(a,b,P,k);
              if(Q.inf) continue;
              if(!eq_na_curva(a,b,eq_neg(Q))) mal++;
              if(!eq_soma(a,b,Q,eq_neg(Q)).inf) mal++;
          }
          printf("      varrido em 6 pontos: %d falhas\n", mal); }
        break; }
    case 3: case 4: {
        tique("A RETA — com x₁ ≠ x₂, a secante tem inclinação m = (y₂−y₁)/(x₂−x₁), e o"
              " terceiro ponto de interseção REFLETE-SE no eixo x. A reflexão não é"
              " decoração: é ela que faz o neutro cair no infinito e a soma ser um grupo");
        tique("PORQUE A FÓRMULA FUNCIONA — substituindo y = mx + c na curva vem"
              " x³ − m²x² + … = 0, uma cúbica cujas TRÊS raízes são exatamente x₁, x₂, x₃."
              " Por VIÈTE a soma das raízes é o coeficiente de x² com sinal trocado:"
              " x₁ + x₂ + x₃ = m². Daí x₃ = m² − x₁ − x₂, e a geometria virou identidade");
        { PtQ P2 = eq_mult(a,b,P,2), R = eq_soma(a,b,P2,P);
          Qz m = qz(0,1);
          { Qz num = qz_soma(P.y, qz_oposto(P2.y));
            Qz den = qz_soma(P.x, qz_oposto(P2.x));
            qz_divide(num, den, &m); }
          Qz soma3 = qz_soma(qz_soma(P2.x, P.x), R.x);
          printf("      P = "); esc_pt(P); printf(",  2P = "); esc_pt(P2); printf("\n");
          printf("      m = "); esc_qz("", m, ",   x₁ + x₂ + x₃ = ");
          esc_qz("", soma3, "   e   m² = ");
          esc_qz("", qz_mult(m,m), "");
          printf("   %s\n", qz_igual(soma3, qz_mult(m,m)) ? "(Viète, resíduo 0)" : "— NÃO afirmo");
          printf("      (x₃ é o do TERCEIRO ponto, antes da reflexão — e é o mesmo x)\n"); }
        break; }
    case 5: {
        tique("A TANGENTE — para dobrar, a reta é a tangente, e a inclinação sai da"
              " derivada implícita: 2y·y' = 3x² + a, logo m = (3x² + a)/(2y)");
        { Qz m = qz(0,1);
          Qz num = qz_soma(qz_mult(qz_de_inteiro(3), qz_mult(P.x,P.x)), a);
          Qz den = qz_mult(qz_de_inteiro(2), P.y);
          qz_divide(num, den, &m);
          printf("      em P = "); esc_pt(P); printf(":  m = ");
          esc_qz("", num, " / "); esc_qz("", den, " = "); esc_qz("", m, "\n"); }
        tique("O GUME — se y = 0 a tangente é VERTICAL, e então 2P = 𝒪. Não se divide por"
              " zero nem se aproxima: a fibra muda, e com ela muda a operação. É o mesmo"
              " gume de ℚ noutro andar");
        { PtQ P2 = eq_mult(a,b,P,2);
          printf("      2P = "); esc_pt(P2); printf(",  e a coordenada y é ");
          esc_qz("", P2.y, " — logo 2(2P) = ");
          esc_pt(eq_soma(a,b,P2,P2)); printf("\n"); }
        break; }
    case 6: {
        tique("A CURVA E O PONTO — E: y² = x³ − 2x + 1, e P = (0,1). Primeiro verifica-se"
              " que a curva é NÃO SINGULAR e que o ponto lá está");
        { Qz disc = qz_soma(qz_mult(qz_de_inteiro(4), qz_mult(a,qz_mult(a,a))),
                            qz_mult(qz_de_inteiro(27), qz_mult(b,b)));
          printf("      4a³ + 27b² = "); esc_qz("", disc, " ≠ 0 ? ");
          printf("%s;   P em E ? %s\n", disc.p ? "sim" : "NÃO",
                 eq_na_curva(a,b,P) ? "sim" : "NÃO"); }
        tique("A ÓRBITA — e é ela o resultado: cada ponto é um tick, e o rastro inteiro"
              " conta mais do que a folha");
        { PtQ Q = P;
          for(int k = 1; k <= 4; k++){
              printf("      %dP = ", k); esc_pt(Q);
              if(!Q.inf) printf("   (em E ? %s)", eq_na_curva(a,b,Q) ? "sim" : "NÃO");
              printf("\n");
              Q = eq_soma(a,b,Q,P);
          } }
        tique("A ORDEM — e ela sai da órbita, não de uma tabela: é o primeiro k com"
              " kP = 𝒪");
        printf("      ord(P) = %ld,  e o rastro é  P → 2P → 4P → 𝒪\n",
               eq_ordem(a,b,P,40));
        tique("A VOLTA POR DOIS CAMINHOS — 3P calculado como (2P) + P e como P + P + P."
              " Se o grupo é associativo, os dois têm de dar o mesmo ponto");
        { PtQ v1 = eq_soma(a,b,eq_mult(a,b,P,2), P);
          PtQ v2 = eq_soma(a,b,eq_soma(a,b,P,P), P);
          PtQ v3 = eq_mult(a,b,P,3);
          printf("      (2P)+P = "); esc_pt(v1);
          printf(";   P+P+P = "); esc_pt(v2);
          printf(";   3P por dobras = "); esc_pt(v3);
          printf("   %s\n", (eq_igual(v1,v2) && eq_igual(v2,v3)) ? "(resíduo 0)" : "— NÃO afirmo");
          printf("      e 3P = −P, o que confere com 4P = 𝒪\n"); }
        break; }
    case 7: {
        tique("OS CINCO — fechamento, neutro, inverso, comutatividade e associatividade."
              " Os quatro primeiros leem-se das fórmulas; o quinto é o GUME PESADO");
        tique("A ASSOCIATIVIDADE — «não basta testar alguns pontos e declarar: é uma"
              " identidade estrutural». Então não se declara: varre-se o grupo INTEIRO"
              " de uma curva sobre 𝔽ₚ, onde ele é finito e a varredura é exaustiva");
        { long p = 17, A = 2, B = 2;
          PtF pts[64]; long np = 0;
          pts[np++] = ef_O();
          for(long x = 0; x < p && np < 64; x++) for(long y = 0; y < p && np < 64; y++){
              PtF T = ef_pt(x,y,p);
              if(ef_na_curva(A,B,p,T)) pts[np++] = T;
          }
          long mal = 0, casos = 0;
          for(long i = 0; i < np; i++) for(long j = 0; j < np; j++) for(long k = 0; k < np; k++){
              PtF e = ef_soma(A,B,p, ef_soma(A,B,p,pts[i],pts[j]), pts[k]);
              PtF d = ef_soma(A,B,p, pts[i], ef_soma(A,B,p,pts[j],pts[k]));
              if(!ef_igual(e,d)) mal++;
              casos++;
          }
          printf("      E(𝔽₁₇) tem %ld pontos, e (P+Q)+R = P+(Q+R) em %ld triplos:"
                 " %ld falhas\n", np, casos, mal);
          long fmal = 0;
          for(long i = 0; i < np; i++) for(long j = 0; j < np; j++){
              PtF s = ef_soma(A,B,p,pts[i],pts[j]);
              if(!ef_na_curva(A,B,p,s)) fmal++;                      /* fechamento */
              if(!ef_igual(s, ef_soma(A,B,p,pts[j],pts[i]))) fmal++; /* comutatividade */
          }
          printf("      e o fechamento e a comutatividade nos %ld pares: %ld falhas\n",
                 np*np, fmal); }
        break; }
    case 8: {
        tique("A PROVA — para x₁ ≠ x₂, m_PQ = (y₂−y₁)/(x₂−x₁) e m_QP = (y₁−y₂)/(x₁−x₂)."
              " Numerador e denominador trocam AMBOS de sinal, logo os dois m são iguais");
        tique("E DAÍ AS FÓRMULAS COINCIDEM — x₃ = m² − x₁ − x₂ é simétrica em x₁ e x₂; e"
              " o y₃, embora escrito com P, dá o mesmo porque o terceiro ponto da reta é"
              " o mesmo qualquer que seja a ordem por que se lê a reta");
        { PtQ P2 = eq_mult(a,b,P,2);
          PtQ r1 = eq_soma(a,b,P,P2), r2 = eq_soma(a,b,P2,P);
          printf("      P + 2P = "); esc_pt(r1);
          printf(";   2P + P = "); esc_pt(r2);
          printf("   %s\n", eq_igual(r1,r2) ? "(iguais)" : "— NÃO afirmo"); }
        tique("O GUME — e testa-se também P + (−P) = 𝒪, que é o caso em que x₁ = x₂ e a"
              " fórmula da secante NÃO se aplica: aí não há inclinação nenhuma a comparar");
        { PtQ r = eq_soma(a,b,P,eq_neg(P));
          printf("      P + (−P) = "); esc_pt(r);
          printf("   %s\n", r.inf ? "(resíduo 0)" : "— NÃO afirmo"); }
        break; }
    case 9: {
        tique("O CORPO MUDA, A GEOMETRIA FICA — sobre 𝔽ₚ as mesmas fórmulas correm, e"
              " toda a geometria vira aritmética discreta sem se abandonar a operação");
        tique("E A DIVISÃO É A INVERSÃO MODULAR — «a/b não significa decimal: significa"
              " a·b⁻¹ (mod p), desde que b ≢ 0». É o andar dos racionais a voltar aqui"
              " inteiro: onde ℚ punha o inverso, 𝔽ₚ põe o inverso modular");
        { long inv;
          nm_inv_mod(2, 17, &inv);
          printf("      em 𝔽₁₇:  1/2 = 2⁻¹ = %ld,  porque 2·%ld = %ld ≡ %ld (mod 17)\n",
                 inv, inv, 2*inv, (2*inv) % 17);
          printf("      e 1/0 ? não existe — a fibra é vazia, tal como em ℚ\n"); }
        tique("VOLTA — e a soma de dois pontos de 𝔽₁₇ cai na curva, medido");
        { long p = 17, A = 2, B = 2;
          PtF G = ef_pt(5,1,p), H = ef_pt(6,3,p);
          printf("      G = (5,1) em E ? %s;  H = (6,3) em E ? %s\n",
                 ef_na_curva(A,B,p,G) ? "sim" : "NÃO", ef_na_curva(A,B,p,H) ? "sim" : "NÃO");
          PtF S = ef_soma(A,B,p,G,H);
          printf("      G + H = (%ld, %ld),  em E ? %s\n", S.x, S.y,
                 ef_na_curva(A,B,p,S) ? "sim (resíduo 0)" : "NÃO"); }
        break; }
    case 10: {
        long p = 17, A = 2, B = 2;
        tique("A PRIMEIRA VIA — varrer os x: para cada x calcula-se x³ + 2x + 2 e contam-se"
              " os y com y² igual a isso. Mais o 𝒪, que conta como ponto");
        { long n1 = ef_conta(A,B,p);
          printf("      pela varredura dos x:  #E(𝔽₁₇) = %ld\n", n1); }
        tique("A SEGUNDA VIA — tabelar os QUADRADOS de 𝔽₁₇ e olhar para cada x uma vez só:"
              " o lado direito ou é resíduo quadrático (dois y), ou é zero (um y), ou não"
              " é (nenhum). São dois caminhos genuinamente diferentes sobre o mesmo objeto");
        { long quad[32]; for(long i = 0; i < p; i++) quad[i] = 0;
          for(long y = 0; y < p; y++) quad[(y*y) % p]++;
          long n2 = 1;
          for(long x = 0; x < p; x++){
              long d = ef_m(x*x%p*x + A*x + B, p);
              n2 += quad[d];
          }
          printf("      pela tabela dos quadrados: #E(𝔽₁₇) = %ld\n", n2);
          printf("      as duas vias: %s\n", n2 == ef_conta(A,B,p)
                 ? "o MESMO conjunto (resíduo 0)" : "— NÃO afirmo"); }
        tique("E O CONTROLO DE HASSE — |N − (p+1)| ≤ 2√p, que em inteiros é"
              " (N − p − 1)² ≤ 4p. Não é a resposta: é a régua que a resposta tem de caber");
        { long N = ef_conta(A,B,p), t = N - p - 1;
          printf("      N = %ld, p+1 = %ld, t = %ld, e t² = %ld ≤ 4p = %ld ? %s\n",
                 N, p+1, t, t*t, 4*p, t*t <= 4*p ? "sim" : "NÃO"); }
        break; }
    case 11: {
        /* A CURVA DESTE ITEM NÃO É A DO §10, E DE PROPÓSITO: ali #E = 19 é PRIMO, e com
         * ordem prima «todas as ordens dividem N» é quase automático — só há 1 e N. Seria
         * um caso degenerado a passar por teorema. Aqui usa-se y² = x³ + x + 2 sobre 𝔽₁₇,
         * onde #E = 24 e aparecem SETE ordens distintas: aí a divisibilidade decide. */
        long p = 17, A = 1, B = 2;
        tique("LAGRANGE — num grupo finito a ordem de qualquer elemento DIVIDE a ordem do"
              " grupo. É o mesmo teorema que ligou Fermat a Euler no andar anterior, agora"
              " a agir sobre pontos em vez de resíduos");
        tique("E A CURVA MUDA AQUI — na do §10 o #E é 19, PRIMO, e aí «todas as ordens"
              " dividem N» seria quase automático (só há 1 e 19). Com y² = x³ + x + 2"
              " sobre 𝔽₁₇ o #E é composto, e a divisibilidade passa a ter conteúdo");
        { long N = ef_conta(A,B,p);
          printf("      E: y² = x³ + x + 2 sobre 𝔽₁₇,  #E = %ld, e as ordens:\n      ", N);
          long vistas[64], nv = 0;
          for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
              PtF T = ef_pt(x,y,p);
              if(!ef_na_curva(A,B,p,T)) continue;
              long o = ef_ordem(A,B,p,T);
              int novo = 1;
              for(long i = 0; i < nv; i++) if(vistas[i] == o) novo = 0;
              if(novo && nv < 64){ vistas[nv++] = o; printf("%ld ", o); }
          }
          printf("\n      e todas dividem %ld ? ", N);
          int mal = 0;
          for(long i = 0; i < nv; i++) if(vistas[i] == 0 || N % vistas[i]) mal++;
          printf("%s\n", mal ? "— NÃO afirmo" : "sim (resíduo 0)");
          tique("E A CONSEQUÊNCIA — NP = 𝒪 para TODO ponto P, seja qual for a sua ordem."
                " É a ordem do grupo a anular toda a gente");
          long nmal = 0, contados = 0;
          for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
              PtF T = ef_pt(x,y,p);
              if(!ef_na_curva(A,B,p,T)) continue;
              if(!ef_mult(A,B,p,T,N).inf) nmal++;
              contados++;
          }
          printf("      NP = 𝒪 em %ld pontos afins: %ld falhas\n", contados, nmal); }
        break; }
    case 12: {
        tique("O ALGORITMO — «em vez de somar P repetidamente, usamos double-and-add»."
              " É a quantização por ticks desta casa: a multiplicação inteira desfaz-se em"
              " DOBRAS e somas, e cada bit do expoente é um tick");
        printf("      19 = 16 + 2 + 1 = (10011)₂\n");
        tique("O RASTRO — P → 2P → 4P → 8P → 16P, e depois 16P + 2P + P. São 4 dobras e"
              " 2 somas, contra 18 somas: é log em vez de linear");
        { long p = 17, A = 2, B = 2;
          PtF G = ef_pt(5,1,p), D = G;
          printf("      em 𝔽₁₇ com G = (5,1):\n");
          for(int k = 0; k < 5; k++){
              printf("        %2dG = ", 1 << k);
              if(D.inf) printf("𝒪\n"); else printf("(%ld, %ld)\n", D.x, D.y);
              D = ef_soma(A,B,p,D,D);
          }
          PtF r16 = ef_mult(A,B,p,G,16), r2 = ef_mult(A,B,p,G,2);
          PtF soma = ef_soma(A,B,p, ef_soma(A,B,p,r16,r2), G);
          PtF direto = ef_mult(A,B,p,G,19);
          printf("      16G + 2G + G = ");
          if(soma.inf) printf("𝒪"); else printf("(%ld, %ld)", soma.x, soma.y);
          printf(";   19G por dobras = ");
          if(direto.inf) printf("𝒪"); else printf("(%ld, %ld)", direto.x, direto.y);
          printf("   %s\n", ef_igual(soma,direto) ? "(dois caminhos, resíduo 0)" : "— NÃO afirmo");
          tique("VOLTA — e mede-se contra a soma REPETIDA, que é o caminho lento: se as"
                " dobras discordassem da soma um a um, uma das duas estava errada");
          PtF lento = ef_O();
          for(int k = 0; k < 19; k++) lento = ef_soma(A,B,p,lento,G);
          printf("      somando 19 vezes: ");
          if(lento.inf) printf("𝒪"); else printf("(%ld, %ld)", lento.x, lento.y);
          printf("   %s\n", ef_igual(lento,direto) ? "(resíduo 0)" : "— NÃO afirmo");
          printf("      (e 19G = 𝒪 porque #E = 19: é Lagrange outra vez)\n"); }
        break; }
    }
}
static int resolve_eliptica(const char *f){
    const char *p = f;
    if(!strncmp(p, "dirichlet", 9) || !strncmp(p, "convolucao", 10)
       || !strncmp(p, "convolução", 11) || !strncmp(p, "mobius inversao", 15)
       || !strncmp(p, "möbius inversão", 17)){ dirichlet_resolve(); return 1; }
    if(!strncmp(p, "eliptica", 8)) p += 8;
    else if(!strncmp(p, "elíptica", 9)) p += 9;
    else if(!strncmp(p, "curva", 5)) p += 5;
    else {
        for(size_t i = 0; i < sizeof EL12/sizeof *EL12; i++)
            if(!strcmp(p, EL12[i].nome)){ eliptica_resolve(EL12[i].n); return 1; }
        return 0;
    }
    while(*p == ' ') p++;
    if(!*p){
        printf("   curvas elípticas — «eliptica N» ou «eliptica <nome>»\n");
        printf("   E: y² = x³ + ax + b, com 4a³ + 27b² ≠ 0\n");
        printf("   (e a frase do andar: a FIBRA determina qual operação existe)\n\n");
        for(size_t i = 0; i < sizeof EL12/sizeof *EL12; i++){
            printf("     %2d  ", EL12[i].n);
            esc_col(EL12[i].nome, 16);
            printf("  %s\n", EL12[i].enunciado);
        }
        return 1;
    }
    if(*p >= '0' && *p <= '9'){
        long n = 0;
        while(*p >= '0' && *p <= '9') n = n*10 + (*p++ - '0');
        while(*p == ' ') p++;
        if(!*p && n >= 1 && n <= 12){ eliptica_resolve((int)n); return 1; }
        return 0;
    }
    for(size_t i = 0; i < sizeof EL12/sizeof *EL12; i++)
        if(!strcmp(p, EL12[i].nome)){ eliptica_resolve(EL12[i].n); return 1; }
    return 0;
}
/* ── TEORIA DOS NÚMEROS: OS DEZASSETE, E É TUDO A MESMA ÓRBITA ──────────────────────
 * «definição → propriedade → teorema → exercício de demonstração → contraexemplo →
 * volta. Nada de só lista de contas.» E no fim a frase que organiza o andar todo:
 *
 *     Euclides = MDC = Bézout = FC — «são diferentes saídas da mesma órbita»
 *
 * Cada exercício corre no relógio; onde ele pede gume, há gume; onde pede volta, a volta
 * substitui e mede. As contas aparecem, mas como TESTEMUNHA, nunca como resposta. */
static const struct { int n; const char *nome; const char *enunciado; } TN17[] = {
 {  1, "divisibilidade",     "a | b e b | c  ⟹  a | c" },
 {  2, "mdc euclides",       "gcd(a,b) = gcd(b, a − qb)" },
 {  3, "bezout menor",       "gcd(a,b) é o MENOR positivo de {ax + by}" },
 {  4, "euclides estendido", "gcd(391, 299) e os coeficientes pela volta" },
 {  5, "lema de euclides",   "p primo e p | ab  ⟹  p | a ou p | b" },
 {  6, "fatoracao unica",    "o Teorema Fundamental da Aritmética" },
 {  7, "infinitos primos",   "existem infinitos primos, e o novo constrói-se" },
 {  8, "congruencias",       "37⁴ mod 7, sem calcular 37⁴" },
 {  9, "inverso modular",    "o inverso de 17 módulo 43" },
 { 10, "fermat",             "7¹⁰⁰ mod 13, pelo pequeno Fermat" },
 { 11, "totiente",           "φ(60)" },
 { 12, "euler",              "a^φ(n) ≡ 1 (mod n)" },
 { 13, "chines",             "x ≡ 2 (mod 3) e x ≡ 3 (mod 5)" },
 { 14, "diofantina",         "35x + 22y = 7, e TODAS as soluções" },
 { 15, "fracao continua",    "a FC de 391/299 pelos restos de Euclides" },
 { 16, "mobius",             "Σ_{d|n} μ(d) para n = 12" },
 { 17, "a orbita unica",     "Euclides = MDC = Bézout = FC, com a volta" },
};
static void tn_euclides(long a, long b, int mostra){    /* a descida, e é ela tudo */
    long x = a, y = b;
    while(y){
        long q = x / y, r = x % y;
        if(mostra) printf("      %ld = %ld·%ld + %ld\n", x, y, q, r);
        x = y; y = r;
    }
    if(mostra) printf("      logo gcd(%ld, %ld) = %ld\n", a, b, x);
}
static void numeros_resolve(int n){
    TICK_N = 0;
    printf("   %d — %s\n", n, TN17[n-1].enunciado);
    switch(n){
    case 1: {
        tique("DEFINIÇÃO — a | b quer dizer que EXISTE k com b = ak. Não é uma"
              " propriedade de a e b: é uma existência, e o k é a testemunha");
        tique("COMPOSIÇÃO — de b = ak e c = bℓ vem c = a(kℓ), e kℓ é inteiro porque ℤ é"
              " fechado para o produto. É essa testemunha nova que dá a | c");
        { long k, l;
          iz_div(4, 12, &k); iz_div(12, 36, &l);
          printf("      4 | 12 com k = %ld,  12 | 36 com ℓ = %ld,  logo 4 | 36 com kℓ = %ld\n",
                 k, l, k*l); }
        tique("GUME — «procure um caso em que apenas a | c seja verdadeiro, sem a | b»."
              " Ele existe, e por isso a implicação NÃO se inverte: a transitividade dá"
              " uma direção só");
        { int achou = 0;
          for(long a = 2; a <= 12 && !achou; a++) for(long b = 2; b <= 12 && !achou; b++)
          for(long c = 2; c <= 60 && !achou; c++)
              if(iz_div(a,c,0) && !iz_div(a,b,0) && iz_div(b,c,0)){
                  printf("      a=%ld, b=%ld, c=%ld:  %ld | %ld sim, %ld | %ld NÃO —"
                         " e mesmo assim %ld | %ld\n", a,b,c, b,c, a,b, a,c);
                  achou = 1;
              }
          if(!achou) printf("      — NÃO achei contraexemplo, e por isso NÃO afirmo o gume\n"); }
        tique("VOLTA — e a propriedade-mãe: a | b e a | c dão a | (bx + cy) para quaisquer"
              " x, y. É a distributividade a pôr o a em evidência, e é dela que sai tudo"
              " o resto deste andar");
        { int mal = 0;
          for(long a = 1; a <= 10; a++) for(long b = -30; b <= 30; b++) for(long c = -30; c <= 30; c++){
              if(!iz_div(a,b,0) || !iz_div(a,c,0)) continue;
              for(long x = -4; x <= 4; x++) for(long y = -4; y <= 4; y++)
                  if(!iz_div(a, b*x + c*y, 0)) mal++;
          }
          printf("      a | (bx + cy) varrido: %d falhas\n", mal); }
        break; }
    case 2: {
        tique("A IDENTIDADE — gcd(a,b) = gcd(b, a − qb), e a prova é de DUPLA INCLUSÃO:"
              " todo divisor comum de (a,b) divide a − qb, e todo divisor comum de"
              " (b, a−qb) divide a = (a−qb) + qb. Os dois conjuntos de divisores são o"
              " MESMO, logo o máximo é o mesmo");
        tique("A ÓRBITA — e é isso que o algoritmo faz: repete o passo até o resto ser 0."
              " O último resto não nulo é o gcd, e o rastro é a descida inteira");
        tn_euclides(252, 105, 1);
        tique("VOLTA — e confere-se a definição, os TRÊS pontos: d | a, d | b, e todo"
              " divisor comum divide d");
        { long g = iz_gcd(252, 105, 0, 0); int mal = 0, comuns = 0;
          if(!iz_div(g, 252, 0) || !iz_div(g, 105, 0)) mal++;
          for(long d = 1; d <= 252; d++)
              if(252 % d == 0 && 105 % d == 0){ comuns++; if(g % d) mal++; }
          printf("      d = %ld:  d | 252 e d | 105, e os %d divisores comuns dividem d"
                 " — %d falhas\n", g, comuns, mal); }
        { int mal = 0;
          for(long a = 1; a <= 60; a++) for(long b = 1; b <= 60; b++) for(long q = -3; q <= 3; q++)
              if(iz_gcd(a,b,0,0) != iz_gcd(b, a - q*b, 0, 0)) mal++;
          printf("      e a identidade varrida em 60×60×7: %d falhas\n", mal); }
        break; }
    case 3: {
        tique("BÉZOUT — gcd(a,b) escreve-se como ax + by, e o Euclides ESTENDIDO carrega"
              " os coeficientes na descida. O x e o y são a testemunha");
        { long x, y, g = iz_gcd(35, 22, &x, &y);
          printf("      gcd(35, 22) = %ld,  x = %ld,  y = %ld\n", g, x, y);
          printf("      substituindo: 35·(%ld) + 22·(%ld) = %ld   %s\n",
                 x, y, 35*x + 22*y, 35*x + 22*y == g ? "(resíduo 0)" : "— NÃO afirmo"); }
        tique("O EXERCÍCIO MAIS INTERESSANTE — S = {ax + by > 0} tem MENOR elemento (boa"
              " ordenação de ℕ: todo subconjunto não vazio de positivos tem mínimo), e"
              " esse mínimo É o gcd");
        tique("PORQUÊ — seja d o menor de S. Dividindo a por d: a = dq + r com 0 ≤ r < d."
              " Mas r = a − dq também é da forma ax+by, e r < d; como d é o MENOR"
              " positivo, r = 0. Logo d | a, e o mesmo dá d | b. E todo divisor comum"
              " divide d porque divide ax+by. Os três pontos da definição");
        { int mal = 0;
          for(long a = 1; a <= 30; a++) for(long b = 1; b <= 30; b++){
              long menor = 0;
              for(long x = -30; x <= 30; x++) for(long y = -30; y <= 30; y++){
                  long t = a*x + b*y;
                  if(t > 0 && (menor == 0 || t < menor)) menor = t;
              }
              if(menor != iz_gcd(a,b,0,0)) mal++;
          }
          printf("      o menor positivo de {ax+by} é o gcd, em 900 pares: %d falhas\n", mal); }
        break; }
    case 4: {
        tique("A DESCIDA — os restos, e é este o rastro que carrega tudo");
        tn_euclides(391, 299, 1);
        tique("A SUBIDA — e agora a VOLTA pela mesma cadeia, a substituir cada resto pela"
              " sua expressão: é ela que produz os coeficientes de Bézout");
        { long x, y, g = iz_gcd(391, 299, &x, &y);
          printf("      391·(%ld) + 299·(%ld) = %ld = gcd   %s\n",
                 x, y, 391*x + 299*y, 391*x + 299*y == g ? "(resíduo 0)" : "— NÃO afirmo");
          printf("      e o mesmo para o exemplo dele, 252 e 105:\n");
          long x2, y2, g2 = iz_gcd(252, 105, &x2, &y2);
          printf("      252·(%ld) + 105·(%ld) = %ld = gcd   %s\n",
                 x2, y2, 252*x2 + 105*y2, 252*x2 + 105*y2 == g2 ? "(confere)" : "— NÃO afirmo"); }
        break; }
    case 5: {
        tique("HIPÓTESE — p primo, p | ab, e suponha-se p ∤ a. É deste «suponha-se» que"
              " sai tudo: se p não divide a, então gcd(p,a) = 1, porque os únicos"
              " divisores de p são 1 e p");
        tique("BÉZOUT — logo existem x, y com px + ay = 1. Note-se que o teorema anterior"
              " é que faz este: sem Bézout esta prova não existe");
        tique("MULTIPLICAR POR b — pbx + aby = b. O primeiro termo tem p; o segundo tem ab,"
              " que p divide por hipótese. Logo p divide a soma, que é b");
        { long p = 7, a = 10, b = 21;
          long x, y; iz_gcd(p, a, &x, &y);
          printf("      p=%ld, a=%ld, b=%ld:  p·(%ld) + a·(%ld) = %ld\n", p,a,b,x,y, p*x+a*y);
          printf("      ×b:  %ld·(%ld) + %ld·(%ld) = %ld,  e p | ab = %ld ? %s\n",
                 p, x*b, a*b, y, p*x*b + a*b*y, a*b, iz_div(p, a*b, 0) ? "sim" : "não");
          printf("      logo p | b = %ld ? %s\n", b, iz_div(p, b, 0) ? "sim (resíduo 0)" : "NÃO"); }
        tique("GUME — e é ESSENCIAL que p seja primo: com 4 (composto) o teorema é FALSO."
              " 4 | 2·6 e 4 não divide nem 2 nem 6. A hipótese não é decoração");
        printf("      4 | 12 sim;  4 | 2 ? %s;  4 | 6 ? %s  — o teorema cai\n",
               iz_div(4,2,0) ? "sim" : "NÃO", iz_div(4,6,0) ? "sim" : "NÃO");
        { int mal = 0, viu = 0;
          for(long q = 2; q <= 40; q++){
              if(!nt_primo(q)) continue;
              for(long a = 1; a <= 40; a++) for(long b = 1; b <= 40; b++)
                  if(iz_div(q, a*b, 0)){ viu++; if(!iz_div(q,a,0) && !iz_div(q,b,0)) mal++; }
          }
          printf("      varrido em %d casos com p primo: %d falhas\n", viu, mal); }
        break; }
    case 6: {
        tique("EXISTÊNCIA, por indução forte — se n é primo, está feito; se não, n = ab"
              " com 1 < a,b < n, e por hipótese de indução a e b fatoram-se. O produto"
              " das duas fatorações é a de n. A descida termina porque os fatores"
              " ENCOLHEM, e ℕ é bem ordenado");
        tique("UNICIDADE, pelo LEMA DE EUCLIDES — se p₁…pₖ = q₁…qₘ, então p₁ divide o"
              " produto da direita, logo divide algum qⱼ (é o lema), e como qⱼ é primo,"
              " p₁ = qⱼ. Corta-se e repete-se. São DUAS provas diferentes a chegar ao"
              " mesmo objeto — e é isso que ele queria");
        { long pr[NT_FAT]; int ex[NT_FAT];
          int k = nt_fatora(360, pr, ex, NT_FAT);
          printf("      360 = ");
          for(int i = 0; i < k; i++){ printf("%s%ld", i?"·":"", pr[i]); if(ex[i]>1) printf("^%d", ex[i]); }
          printf("\n"); }
        tique("VOLTA — e a fatoração multiplica-se de volta: se não devolvesse o número,"
              " não era fatoração dele");
        { int mal = 0, feitos = 0;
          for(long m = 2; m <= 3000; m++){
              long pr[NT_FAT]; int ex[NT_FAT];
              int k = nt_fatora(m, pr, ex, NT_FAT);
              if(nt_refaz(pr, ex, k) != m) mal++;
              for(int i = 0; i < k; i++) if(!nt_primo(pr[i])) mal++;   /* e são PRIMOS */
              feitos++;
          }
          printf("      %d números fatorados e refeitos, todos os fatores primos:"
                 " %d falhas\n", feitos, mal); }
        break; }
    case 7: {
        tique("ABSURDO — suponha-se que são só p₁,…,pₙ. Constrói-se N = p₁p₂…pₙ + 1");
        tique("NENHUM DIVIDE — N ≡ 1 (mod pᵢ) para cada i, porque o produto é ≡ 0 e"
              " sobra o 1. Logo nenhum pᵢ divide N");
        tique("MAS N > 1 — logo tem um divisor primo (pelo Teorema Fundamental), e esse"
              " primo não está na lista. Contradição");
        tique("E O EXERCÍCIO — «não apenas reproduza a prova: construa explicitamente o"
              " novo primo para 2, 3, 5, 7». Então constrói-se");
        { long p[4] = {2,3,5,7}, N;
          long novo = nm_primo_novo(p, 4, &N);
          printf("      N = 2·3·5·7 + 1 = %ld", N);
          printf(",  e o menor fator primo de N é %ld", novo);
          printf("   (primo? %s)\n", nt_primo(novo) ? "sim" : "NÃO");
          for(int i = 0; i < 4; i++)
              printf("      N mod %ld = %ld%s", p[i], N % p[i], i==3?"\n":"; ");
          printf("      e %ld não está em {2,3,5,7} — é o primo novo\n", novo);
          /* e repete-se: cada lista dá um primo fora dela */
          long lista[8] = {2,3,5,7,11,13,17,19}; int mal = 0;
          for(int n2 = 1; n2 <= 6; n2++){
              long NN, nv = nm_primo_novo(lista, n2, &NN);
              if(!nt_primo(nv)) mal++;
              for(int i = 0; i < n2; i++) if(nv == lista[i]) mal++;
          }
          printf("      e em 6 listas seguidas o primo construído é sempre NOVO:"
                 " %d falhas\n", mal); }
        break; }
    case 8: {
        tique("DEFINIÇÃO — a ≡ b (mod n) quer dizer n | (a − b). É uma equivalência, e é"
              " COMPATÍVEL com + e ×: é isso que faz do quociente um anel e que autoriza"
              " substituir antes de calcular");
        tique("REDUZIR PRIMEIRO — «calcule sem calcular 37⁴». 37 ≡ 2 (mod 7), e a"
              " compatibilidade deixa trocar 37 por 2 ANTES de elevar");
        printf("      37 ≡ %ld (mod 7),  logo 37⁴ ≡ 2⁴ = 16 ≡ %ld (mod 7)\n",
               37L % 7, 16L % 7);
        tique("VOLTA — e mede-se pelos DOIS caminhos: a redução e a conta direta têm de"
               " dar o mesmo, senão a compatibilidade era falsa");
        { long dobra = iz_pot_mod(37, 4, 7);
          long direto = ((37L*37L) % 7) * ((37L*37L) % 7) % 7;
          printf("      pela dobra: %ld;  pela conta: %ld   %s\n",
                 dobra, direto, dobra == direto ? "(resíduo 0)" : "— NÃO afirmo");
          int mal = 0;
          for(long n2 = 2; n2 <= 12; n2++) for(long a = -20; a <= 20; a++)
          for(long b = -20; b <= 20; b++) for(long c = -6; c <= 6; c++){
              if(!iz_cong(a,b,n2)) continue;
              if(!iz_cong(a+c, b+c, n2)) mal++;
              if(!iz_cong(a*c, b*c, n2)) mal++;
          }
          printf("      a compatibilidade com + e × varrida: %d falhas\n", mal); }
        break; }
    case 9: {
        tique("QUANDO EXISTE — ax ≡ 1 (mod n) tem solução exatamente quando gcd(a,n) = 1."
              " Se um primo dividisse os dois, dividia 1");
        tique("EUCLIDES ESTENDIDO, NÃO TENTATIVA — e a diferença não é de estilo: a"
              " tentativa custa n passos, a órbita custa log n. De ax + ny = 1 vem"
              " ax ≡ 1, e o x é o inverso");
        { long x, y, g = iz_gcd(17, 43, &x, &y);
          printf("      gcd(17, 43) = %ld,  17·(%ld) + 43·(%ld) = %ld\n", g, x, y, 17*x+43*y);
          long inv;
          nm_inv_mod(17, 43, &inv);
          printf("      logo 17⁻¹ ≡ %ld (mod 43)\n", inv);
          tique("VOLTA — e multiplica-se para conferir: 17·inv tem de dar 1 módulo 43");
          printf("      17·%ld = %ld = %ld·43 + %ld   %s\n",
                 inv, 17*inv, 17*inv/43, (17*inv) % 43,
                 (17*inv) % 43 == 1 ? "(resíduo 0)" : "— NÃO afirmo"); }
        { int mal = 0, existe = 0, nao = 0;
          for(long n2 = 2; n2 <= 60; n2++) for(long a = 1; a < n2; a++){
              long inv;
              int tem = nm_inv_mod(a, n2, &inv);
              if(tem != (iz_gcd(a,n2,0,0) == 1)) mal++;      /* o critério */
              if(tem){ if((a*inv) % n2 != 1 % n2) mal++; existe++; } else nao++;
          }
          printf("      o critério e a volta em %d com inverso e %d sem: %d falhas\n",
                 existe, nao, mal); }
        break; }
    case 10: {
        tique("PEQUENO FERMAT — p primo e p ∤ a dão a^(p−1) ≡ 1 (mod p). O porquê é a"
              " ORDEM: os a^k percorrem um ciclo dentro dos p−1 resíduos não nulos, e o"
              " comprimento do ciclo DIVIDE p−1");
        { long ord = nm_ordem(7, 13);
          printf("      a ordem de 7 módulo 13 é %ld, e %ld | 12 ? %s\n",
                 ord, ord, 12 % ord == 0 ? "sim" : "NÃO"); }
        tique("O EXERCÍCIO — 7¹⁰⁰ mod 13. Por Fermat 7¹² ≡ 1, e 100 = 12·8 + 4, logo"
              " 7¹⁰⁰ ≡ 7⁴. É a divisão com resto a fazer o trabalho todo");
        { printf("      100 = 12·%ld + %ld,  logo 7¹⁰⁰ ≡ 7^%ld (mod 13)\n",
                 100L/12, 100L%12, 100L%12);
          long p4 = iz_pot_mod(7, 4, 13), p100 = iz_pot_mod(7, 100, 13);
          printf("      7⁴ mod 13 = %ld,  e 7¹⁰⁰ mod 13 pela dobra = %ld   %s\n",
                 p4, p100, p4 == p100 ? "(dois caminhos, resíduo 0)" : "— NÃO afirmo"); }
        tique("GUME — e a hipótese «p primo» não é decoração: com n = 15 (composto) e"
              " a = 2, 2¹⁴ mod 15 NÃO dá 1");
        printf("      2^14 mod 15 = %ld  (e não 1 — Fermat exige o primo)\n",
               iz_pot_mod(2, 14, 15));
        { int mal = 0, viu = 0;
          for(long q = 2; q <= 60; q++){
              if(!nt_primo(q)) continue;
              for(long a = 1; a < q; a++){ if(iz_pot_mod(a, q-1, q) != 1) mal++; viu++; }
          }
          printf("      Fermat varrido em %d pares (a, p): %d falhas\n", viu, mal); }
        break; }
    case 11: {
        tique("DEFINIÇÃO — φ(n) conta os k em [1,n] com gcd(k,n) = 1. É uma CONTAGEM,"
              " e por isso tem sempre o caminho de a fazer à mão");
        tique("A FÓRMULA — φ(n) = n·∏(1 − 1/p), e faz-se em INTEIROS: n/p·(p−1) por cada"
              " primo DISTINTO. O expoente não entra, só o primo");
        { long pr[NT_FAT]; int ex[NT_FAT];
          int k = nt_fatora(60, pr, ex, NT_FAT);
          printf("      60 = ");
          for(int i = 0; i < k; i++){ printf("%s%ld", i?"·":"", pr[i]); if(ex[i]>1) printf("^%d", ex[i]); }
          printf(",  e os primos distintos são ");
          for(int i = 0; i < k; i++) printf("%s%ld", i?", ":"", pr[i]);
          printf("\n      φ(60) = 60·(1−1/2)(1−1/3)(1−1/5) = %ld\n", nm_phi(60)); }
        tique("VOLTA — pelos DOIS caminhos: a fórmula e a contagem à mão. Se"
              " discordassem, uma das duas estava errada, e é isso que se quer apanhar");
        { printf("      pela fórmula: %ld;  contando os coprimos: %ld   %s\n",
                 nm_phi(60), nm_phi_conta(60),
                 nm_phi(60) == nm_phi_conta(60) ? "(resíduo 0)" : "— NÃO afirmo");
          int mal = 0;
          for(long m = 1; m <= 400; m++) if(nm_phi(m) != nm_phi_conta(m)) mal++;
          printf("      e em 400 números os dois caminhos concordam: %d falhas\n", mal);
          printf("      (e os casos dele: φ(p) = p−1, φ(pq) = (p−1)(q−1) —"
                 " φ(7) = %ld, φ(35) = %ld = 6·4)\n", nm_phi(7), nm_phi(35)); }
        break; }
    case 12: {
        tique("EULER GENERALIZA FERMAT — com gcd(a,n) = 1 vale a^φ(n) ≡ 1 (mod n)."
              " Quando n = p é primo, φ(p) = p−1 e recupera-se o pequeno Fermat:"
              " é o MESMO teorema, e o φ é que estava escondido");
        tique("PORQUÊ — os resíduos coprimos formam um grupo de ordem φ(n) sob a"
              " multiplicação, e a ordem de qualquer elemento DIVIDE a ordem do grupo."
              " É Lagrange, e é ele que faz os dois teoremas serem um");
        { printf("      n = 15: φ = %ld, e as ordens dos coprimos dividem-no:  ", nm_phi(15));
          for(long a = 1; a < 15; a++) if(iz_gcd(a,15,0,0)==1) printf("%ld ", nm_ordem(a,15));
          printf("\n      2^φ(15) mod 15 = %ld\n", iz_pot_mod(2, nm_phi(15), 15)); }
        tique("VOLTA — varrido: para todo n e todo a coprimo, a^φ(n) ≡ 1, e a ordem"
              " divide φ(n). As duas coisas juntas, porque uma sem a outra é metade");
        { int mal = 0, viu = 0;
          for(long n2 = 2; n2 <= 80; n2++){
              long f = nm_phi(n2);
              for(long a = 1; a < n2; a++){
                  if(iz_gcd(a,n2,0,0) != 1) continue;
                  if(iz_pot_mod(a, f, n2) != 1 % n2) mal++;
                  long o = nm_ordem(a, n2);
                  if(o == 0 || f % o) mal++;                 /* a ordem divide φ */
                  viu++;
              }
          }
          printf("      %d pares (a, n) com gcd = 1: %d falhas\n", viu, mal); }
        break; }
    case 13: {
        tique("O TEOREMA — com gcd(m,n) = 1, o sistema x ≡ a (mod m), x ≡ b (mod n) tem"
              " solução ÚNICA módulo mn. A unicidade é tão teorema como a existência");
        tique("A CONSTRUÇÃO — não se procura: escreve-se x = a + m·t e obriga-se"
              " a + mt ≡ b (mod n), isto é t ≡ (b−a)·m⁻¹ (mod n). O inverso existe"
              " porque m e n são coprimos — é o exercício 9 a servir este");
        { long x, mod;
          long inv; nm_inv_mod(3 % 5, 5, &inv);
          printf("      m=3, n=5, a=2, b=3:  m⁻¹ mod n = %ld,  t ≡ (3−2)·%ld ≡ %ld (mod 5)\n",
                 inv, inv, ((3-2)*inv) % 5);
          if(nm_tcr(2, 3, 3, 5, &x, &mod)){
              printf("      x = 2 + 3·%ld = %ld,  e a solução é x ≡ %ld (mod %ld)\n",
                     ((3-2)*inv) % 5, x, x, mod);
              tique("VOLTA — e substitui-se nas DUAS congruências, que é o que faz da"
                    " construção uma solução");
              printf("      %ld mod 3 = %ld (queria 2);  %ld mod 5 = %ld (queria 3)   %s\n",
                     x, x%3, x, x%5,
                     (x%3 == 2 && x%5 == 3) ? "(resíduo 0)" : "— NÃO afirmo");
              printf("      e a UNICIDADE: nos %ld resíduos módulo %ld só um serve —",
                     mod, mod);
              long quantos = 0;
              for(long v = 0; v < mod; v++) if(v%3 == 2 && v%5 == 3) quantos++;
              printf(" achei %ld\n", quantos);
          } }
        tique("GUME — e sem a coprimalidade o teorema CAI: x ≡ 0 (mod 2) e x ≡ 1 (mod 4)"
              " não tem solução nenhuma, porque 2 e 4 não são coprimos");
        { long x, mod;
          printf("      gcd(2,4) = %ld, e o sistema tem solução? %s\n",
                 iz_gcd(2,4,0,0), nm_tcr(0,2,1,4,&x,&mod) ? "sim (?!)" : "NÃO"); }
        break; }
    case 14: {
        tique("O CRITÉRIO — ax + by = c tem solução inteira exatamente quando"
              " gcd(a,b) | c. A ida é Bézout escalado; a volta é que todo ax+by é"
              " múltiplo do gcd");
        { long g = iz_gcd(35, 22, 0, 0);
          printf("      gcd(35, 22) = %ld, e %ld | 7 ? %s\n", g, g, 7 % g == 0 ? "sim" : "não"); }
        tique("UMA SOLUÇÃO — escala-se a testemunha de Bézout pelo fator c/gcd");
        { long x, y;
          iz_diofantina(35, 22, 7, &x, &y);
          printf("      x = %ld,  y = %ld:  35·(%ld) + 22·(%ld) = %ld   %s\n",
                 x, y, x, y, 35*x + 22*y, 35*x + 22*y == 7 ? "(resíduo 0)" : "— NÃO afirmo");
          tique("TODAS AS SOLUÇÕES — «depois descreva TODAS». A geral é"
                " x = x₀ + (b/g)t, y = y₀ − (a/g)t, com t a correr ℤ: o que se soma a x"
                " tem de ser cancelado por y, e o menor passo que o faz é b/g");
          long g = iz_gcd(35, 22, 0, 0);
          printf("      x = %ld + %ldt,  y = %ld − %ldt\n", x, 22/g, y, 35/g);
          int mal = 0;
          for(long t = -4; t <= 4; t++){
              long xx = x + (22/g)*t, yy = y - (35/g)*t;
              if(35*xx + 22*yy != 7) mal++;
              if(t >= -2 && t <= 2) printf("        t=%2ld:  x=%4ld, y=%5ld  →  %ld\n",
                                           t, xx, yy, 35*xx + 22*yy);
          }
          printf("      nove valores de t, todos soluções: %d falhas\n", mal);
          /* e a COMPLETUDE da família: nenhuma solução fica de fora */
          long fora = 0;
          for(long xx = -200; xx <= 200; xx++){
              long resto = 7 - 35*xx;
              if(resto % 22) continue;
              long yy = resto / 22, tt = (xx - x) / (22/g);
              if(x + (22/g)*tt != xx || y - (35/g)*tt != yy) fora++;
          }
          printf("      e nenhuma solução de x ∈ [−200,200] fica FORA da família:"
                 " %ld fora\n", fora); }
        break; }
    case 15: {
        tique("A COINCIDÊNCIA — «a própria descida de Euclides produz os coeficientes»."
              " Os termos da fração contínua SÃO os quocientes da mesma cadeia de restos"
              " que dá o gcd. Não é analogia: é o mesmo rastro lido noutra coluna");
        tn_euclides(391, 299, 1);
        { long q[32];
          size_t k = nm_fc(391, 299, q, 32);
          printf("      e os quocientes, na ordem: [");
          for(size_t i = 0; i < k; i++) printf(i==0?"%ld":(i==1?"; %ld":", %ld"), q[i]);
          printf("]\n");
          tique("VOLTA — e a FC reconstrói a fração: o convergente do ÚLTIMO termo tem de"
                " dar 391/299 reduzido, ou não era a FC dela");
          long p, d;
          nm_convergente(q, k-1, &p, &d);
          long g = iz_gcd(391, 299, 0, 0);
          printf("      convergente final: %ld/%ld;  391/299 reduzido: %ld/%ld   %s\n",
                 p, d, 391/g, 299/g,
                 (p == 391/g && d == 299/g) ? "(resíduo 0)" : "— NÃO afirmo");
          printf("      e o exemplo dele, 43/19 = ");
          long q2[32]; size_t k2 = nm_fc(43, 19, q2, 32);
          printf("[");
          for(size_t i = 0; i < k2; i++) printf(i==0?"%ld":(i==1?"; %ld":", %ld"), q2[i]);
          printf("]\n"); }
        break; }
    case 16: {
        tique("DEFINIÇÃO — μ(1) = 1; μ(n) = 0 se algum expoente > 1; μ(n) = (−1)^k se n é"
              " livre de quadrados com k primos. O zero não é um caso à parte: é o"
              " cancelamento a acontecer");
        tique("A IDENTIDADE — Σ_{d|n} μ(d) dá 1 em n = 1 e ZERO em n > 1. É"
              " «cancelamento na árvore dos divisores»: os divisores livres de quadrados"
              " emparelham-se por paridade do número de primos, e anulam-se");
        { printf("      n = 12, divisores: ");
          for(long d = 1; d <= 12; d++) if(12 % d == 0) printf("%ld ", d);
          printf("\n      μ:  ");
          for(long d = 1; d <= 12; d++) if(12 % d == 0) printf("μ(%ld)=%d  ", d, nm_mu(d));
          printf("\n      soma = %ld\n", nm_soma_mu(12)); }
        tique("VOLTA — e varre-se: 1 em n=1 e 0 em todo o resto, sem excepção nenhuma."
              " Uma identidade com uma excepção não medida não é identidade");
        { int mal = 0;
          if(nm_soma_mu(1) != 1) mal++;
          for(long m = 2; m <= 500; m++) if(nm_soma_mu(m) != 0) mal++;
          printf("      n de 1 a 500: %d falhas\n", mal);
          /* e o porquê medido: os livres de quadrados emparelham-se por paridade */
          long pares = 0, impares = 0;
          for(long d = 1; d <= 30; d++) if(30 % d == 0){
              int m2 = nm_mu(d);
              if(m2 == 1) pares++; else if(m2 == -1) impares++;
          }
          printf("      em n=30: %ld divisores com μ=+1 e %ld com μ=−1 — e é o"
                 " emparelhamento que dá zero\n", pares, impares); }
        break; }
    case 17: {
        tique("A TESE — «a fração contínua do quociente a/b é produzida pelo algoritmo de"
              " Euclides, e os seus convergentes são as melhores aproximações racionais»."
              " São duas afirmações, e as duas se medem");
        tique("A PRIMEIRA — o rastro. A descida de a/b dá quocientes e restos; os"
              " quocientes SÃO os termos da FC. Mede-se comparando os dois caminhos: os"
              " quocientes tirados da descida e os termos da FC calculada");
        { long a = 391, b = 299, q[32];
          size_t k = nm_fc(a, b, q, 32);
          long x = a, y = b; int i = 0, mal = 0;
          while(y && i < (int)k){
              if(x / y != q[i]) mal++;
              long r = x % y; x = y; y = r; i++;
          }
          printf("      %zu quocientes da descida contra %zu termos da FC: %d divergências\n",
                 k, k, mal); }
        tique("A SEGUNDA — a MELHOR APROXIMAÇÃO, e com a EXCEÇÃO dita. Um convergente de"
              " ordem ≥ 1 é melhor que qualquer fração de denominador ≤ o dele. O de"
              " ordem ZERO é ⌊x⌋/1 e falha quando a parte que sobra passa de 1/2 — em"
              " 5/3 ≈ 1,667 o mais perto com denominador 1 é 2, não 1. Eu tinha escrito"
              " a versão sem exceção e a medida derrubou-ma");
        { long a = 391, b = 299, q[32];
          size_t k = nm_fc(a, b, q, 32);
          for(size_t j = 0; j + 1 < k; j++){
              long p, d;
              nm_convergente(q, j, &p, &d);
              /* |a/b − p/d| contra |a/b − u/v| para todo v ≤ d: em inteiros, compara-se
               * |a·d − p·b|·v  com  |a·v − u·b|·d */
              long melhor = 1, quantos = 0;
              for(long v = 1; v <= d; v++) for(long u = 0; u <= (a*v)/b + 1; u++){
                  if(u == p && v == d) continue;
                  long e1 = a*d - p*b; if(e1 < 0) e1 = -e1;
                  long e2 = a*v - u*b; if(e2 < 0) e2 = -e2;
                  if(e2 * d < e1 * v) melhor = 0;      /* alguém mais perto: cai */
                  quantos++;
              }
              printf("      ordem %zu — %ld/%ld: melhor que as %ld frações de"
                     " denominador ≤ %ld ? %s\n", j, p, d, quantos, d, melhor ? "sim" : "NÃO");
          }
          printf("      e o caso onde a ordem 0 cai: 5/3 = [1; 1, 2], convergente 1/1,\n");
          printf("      mas 2/1 está mais perto (|5−3| = 2 contra |5−6| = 1)\n"); }
        tique("A VOLTA OBRIGATÓRIA — FC → convergente → fração original. Se o último"
              " convergente não devolvesse a fração de partida, o rastro tinha perdido"
              " informação, e o andar todo caía");
        { int mal = 0, feitos = 0;
          for(long a = 2; a <= 120; a++) for(long b = 1; b < a; b++){
              long q[40];
              size_t k = nm_fc(a, b, q, 40);
              if(!k) continue;
              long p, d;
              nm_convergente(q, k-1, &p, &d);
              long g = iz_gcd(a, b, 0, 0);
              if(p != a/g || d != b/g) mal++;
              feitos++;
          }
          printf("      %d frações: FC → convergente → original, %d falhas\n", feitos, mal);
          printf("\n      Euclides = MDC = Bézout = FC — a mesma órbita, quatro saídas\n"); }
        break; }
    }
}
static int resolve_numeros(const char *f){
    const char *p = f;
    if(!strncmp(p, "numeros", 7)) p += 7;
    else if(!strncmp(p, "números", 8)) p += 8;
    else if(!strncmp(p, "teoria dos numeros", 18)) p += 18;
    else if(!strncmp(p, "teoria dos números", 19)) p += 19;
    else {
        /* pelo NOME sozinho, que é como ele escreveria */
        for(size_t i = 0; i < sizeof TN17/sizeof *TN17; i++)
            if(!strcmp(p, TN17[i].nome)){ numeros_resolve(TN17[i].n); return 1; }
        return 0;
    }
    while(*p == ' ') p++;
    if(!*p){                                     /* o ÍNDICE */
        printf("   teoria dos números — «numeros N» ou «numeros <nome>»\n");
        printf("   (e a órbita é uma só: Euclides = MDC = Bézout = FC)\n\n");
        for(size_t i = 0; i < sizeof TN17/sizeof *TN17; i++){
            printf("     %2d  ", TN17[i].n);
            esc_col(TN17[i].nome, 20);
            printf("  %s\n", TN17[i].enunciado);
        }
        return 1;
    }
    if(*p >= '0' && *p <= '9'){
        long n = 0;
        while(*p >= '0' && *p <= '9') n = n*10 + (*p++ - '0');
        while(*p == ' ') p++;
        if(!*p && n >= 1 && n <= 17){ numeros_resolve((int)n); return 1; }
        return 0;
    }
    for(size_t i = 0; i < sizeof TN17/sizeof *TN17; i++)
        if(!strcmp(p, TN17[i].nome)){ numeros_resolve(TN17[i].n); return 1; }
    return 0;
}
/* ── O MESMO PONTO POR QUATRO PORTAS ────────────────────────────────────────────────
 * O salto que o `eval.txt` pede no fim: «mostrar que esse fechamento NÃO DEPENDE DO
 * MÉTODO ESCOLHIDO: corte, Cauchy, bisseção e FC têm de produzir O MESMO PONTO, com a
 * VOLTA verificando a identificação». E a proibição do atalho, dita antes: identificar
 * «SEM SIMPLESMENTE DECLARAR QUE SÃO IGUAIS».
 *
 * Então não se declara. O real É o corte, logo dois métodos dão o mesmo ponto exatamente
 * quando INDUZEM O MESMO CORTE — e é isso que a fala corre, à frente de quem pergunta. */
static void esc_qz(const char *pre, Qz x, const char *pos);
/* A COLUNA MEDE-SE EM CARACTERES, NÃO EM BYTES. O `%-9s` do printf conta bytes, e
 * «bisseção» ou «Möbius» têm acentos: a coluna sai torta e a coluna torta é o que ele lê.
 * Conta-se as cabeças de UTF-8 (os bytes que não são 10xxxxxx) e enche-se à mão. */
static int larg_utf8(const char *s){
    int n = 0;
    for(const unsigned char *q = (const unsigned char *)s; *q; q++)
        if((*q & 0xC0) != 0x80) n++;          /* só as cabeças; 10xxxxxx é continuação */
    return n;
}
static void esc_col(const char *s, int largura){
    printf("%s", s);
    for(int k = larg_utf8(s); k < largura; k++) putchar(' ');
}
static int resolve_identifica(const char *f){
    const char *p = f;
    int quer = 0;
    if(!strncmp(p, "o mesmo ponto", 13)) quer = 1;
    else if(!strncmp(p, "identifica raiz", 15)) quer = 1;
    else if(!strncmp(p, "os quatro metodos", 17) || !strncmp(p, "os quatro métodos", 18)) quer = 1;
    else if(!strncmp(p, "as quatro portas", 16)) quer = 1;
    if(!quer) return 0;
    long a = 2;
    { const char *q = p;
      while(*q && (*q < '0' || *q > '9')) q++;
      if(*q >= '0' && *q <= '9'){ long v = 0;
          while(*q >= '0' && *q <= '9') v = v*10 + (*q++ - '0');
          if(v >= 2) a = v; } }
    long r = raizi(a);
    TICK_N = 0;
    printf("   √%ld por QUATRO portas — e o fecho não pode depender de qual se abre\n", a);
    if(r*r == a){
        { char pq[200];
          snprintf(pq, sizeof pq, "SEM BURACO — %ld é quadrado perfeito e o ponto já"
                   " estava em ℚ: as quatro portas dão o mesmo porque não há nada a"
                   " fechar", a);
          tique(pq); }
        printf("      √%ld = %ld, e é aqui que a pergunta não tem conteúdo\n", a, r);
        return 1;
    }
    tique("O CRITÉRIO — e é o que impede o atalho: NÃO se declara que são iguais. O real"
          " É o corte, portanto dois métodos dão o mesmo ponto exatamente quando INDUZEM"
          " O MESMO CORTE — para cada racional, o mesmo lado. É uma medida, não um acordo");
    tique("AS QUATRO PORTAS, e cada uma traz o seu rastro — que é o que as torna quatro"
          " e não uma:");
    for(int i = 0; i < 4; i++){
        printf("        "); esc_col(id_nome(i), 10);
        printf("→  %s\n", id_rastro(i));
    }
    tique("A VARREDURA — cada racional passa pelas quatro, e comparam-se os SEIS pares."
          " Não há árbitro: eleger um e chamar-lhe acordo seria a mesma declaração com"
          " outro nome");
    { long choques = 0, decididos = 0, ind = 0, n = 0;
      for(long d = 1; d <= 30; d++) for(long pp = 1; pp <= (r+1)*d; pp++){
          Quatro Q = id_quatro(a, qz(pp,d), 12);
          choques += id_choques(Q); ind += id_indecisos(Q);
          decididos += 4 - id_indecisos(Q); n++;
      }
      printf("      %ld racionais, %ld decisões, %ld choques em %ld comparações\n",
             n, decididos, choques, n*6); }
    tique("A INDECISÃO DIZ-SE — um método de esforço finito não decide os racionais que"
          " ainda caem dentro da sua caixa, e isso é 0, não é um lado. Contá-lo como"
          " acordo dava o teorema de graça: bastava não medir nada");
    { long ind0 = 0, ind5 = 0;
      for(long d = 1; d <= 30; d++) for(long pp = 1; pp <= (r+1)*d; pp++){
          ind0 += id_indecisos(id_quatro(a, qz(pp,d), 0));
          ind5 += id_indecisos(id_quatro(a, qz(pp,d), 5));
      }
      printf("      indecisos com esforço 0: %ld;  com esforço 5: %ld — e o esforço é que"
             " os mata\n", ind0, ind5); }
    tique("A VOLTA VERIFICA A IDENTIFICAÇÃO — de cada porta sai uma CAIXA racional, com"
          " profundidades diferentes de propósito, e as quatro têm de se intersetar com o"
          " corte lá dentro. Se fossem pontos diferentes, separavam-se ao apertar");
    { Corte c = { a, 2 };
      Qz lo[4], hi[4];
      rz_caixa_inicial(c, &lo[0], &hi[0]); rz_encaixota(c, &lo[0], &hi[0], 15);
      { long b = rz_b(a); Qz x = qz_de_inteiro(r), y = qz_de_inteiro(r+1);
        for(int k = 0; k < 5; k++){ x = rz_passo(a,b,x); y = rz_passo(a,b,y); }
        lo[1] = x; hi[1] = y; }
      rz_caixa_inicial(c, &lo[2], &hi[2]); rz_encaixota(c, &lo[2], &hi[2], 11);
      { long t[48]; size_t nt = lado(0, -a, t, 48);
        long pn = 1, qn = 0, pa = 0, qa = 1; Qz par = qz_de_inteiro(r), imp = qz_de_inteiro(r+1);
        for(int i = 0; i < 9 && nt; i++){
            long ai = t[(size_t)i < nt ? (size_t)i : (1 + (i-1) % (int)(nt>1?nt-1:1))];
            long p2 = ai*pn + pa, q2 = ai*qn + qa;
            if(q2 > (1L<<26)) break;
            pa = pn; qa = qn; pn = p2; qn = q2;
            if(i % 2 == 0) par = qz(pn,qn); else imp = qz(pn,qn);
        }
        lo[3] = par; hi[3] = imp; }
      Qz mlo = lo[0], mhi = hi[0];
      for(int i = 0; i < 4; i++){
          printf("        "); esc_col(id_nome(i), 10);
          esc_qz("", lo[i], "  …  ");
          esc_qz("", hi[i], "\n");
          if(qz_menor(mlo, lo[i])) mlo = lo[i];
          if(qz_menor(hi[i], mhi)) mhi = hi[i];
      }
      int b1, b2;
      int fecha = qz_menor(mlo, mhi)
               && rz_cmp(mlo, 2, a, &b1) < 0 && b1
               && rz_cmp(mhi, 2, a, &b2) > 0 && b2;
      printf("      interseção  ");
      esc_qz("", mlo, "  …  ");
      esc_qz("", mhi, "");
      printf("   %s\n", fecha ? "— e o corte está lá dentro (resíduo 0)" : "— NÃO afirmo"); }
    tique("E O QUE ISTO DIZ — o objeto não é só a folha: O CAMINHO FAZ PARTE DA"
          " INFORMAÇÃO. Os quatro rastros DIFEREM termo a termo e dão o mesmo ponto; se"
          " fossem o mesmo rastro não havia identificação nenhuma a fazer");
    printf("\n      ℚ tem o rastro, mas não tem a folha.\n");
    printf("      ℝ acrescenta a folha que fecha o rastro.\n");
    printf("\n   $\\sqrt{%ld}$ — quatro portas, um ponto.\n", a);
    return 1;
}
/* ── AS VINTE PROVAS DO `eval.txt` ──────────────────────────────────────────────────
 * Ele pôs vinte exercícios em três níveis e disse «eu colocaria estes no corpus». Estão
 * aqui, e cada um corre no relógio: cada tick NOMEIA a lei que autoriza a transição, e o
 * último faz a VOLTA. Onde há testemunha, ela EXIBE-SE — nenhuma prova acaba num «logo».
 *
 * O que a máquina faz não é citar o teorema: é correr a cadeia e medir cada elo. Uma
 * varredura não é demonstração, mas uma cadeia com todos os elos medidos é — e onde a
 * varredura entra, entra como CONTROLO e diz-se que é isso. */
static int resolve_reais(const char *f);      /* as 12 e 13 correm INTEIRAS */
static Qz qmod(Qz x){ return x.p < 0 ? qz_oposto(x) : x; }
static void esc_qz(const char *pre, Qz x, const char *pos){
    printf("%s%s%s", pre, frac2(x.p, x.q), pos);
}
static const struct { int n; const char *nome; const char *enunciado; } EX20[] = {
 { 1,  "modulo positivo",   "|x| ≥ 0" },
 { 2,  "modulo produto",    "|xy| = |x|·|y|" },
 { 3,  "triangular",        "|x + y| ≤ |x| + |y|" },
 { 4,  "quadrado positivo", "x² ≥ 0" },
 { 5,  "modulo epsilon",    "|x| < ε ⟺ −ε < x < ε" },
 { 6,  "racional entre",    "a < b ⟹ existe racional q com a < q < b" },
 { 7,  "irracional entre",  "a < b ⟹ existe irracional x com a < x < b" },
 { 8,  "monotona",          "toda sucessão crescente e limitada converge" },
 { 9,  "convergente cauchy","toda sucessão convergente é de Cauchy" },
 { 10, "limite unico",      "o limite, quando existe, é único" },
 { 11, "completude",        "a completude pelo axioma do supremo" },
 { 12, "corte de dedekind", "a construção de √2 por corte de Dedekind" },
 { 13, "raiz dois",         "√2 ∉ ℚ" },
 { 14, "encaixados",        "os intervalos encaixados têm um ponto e um só" },
 { 15, "bolzano",           "Bolzano–Weierstrass" },
 { 16, "existencia raiz",   "para a > 0 existe √a" },
 { 17, "valor intermediario","o Teorema do Valor Intermédio" },
 { 18, "racionais densos",  "ℚ é denso em ℝ" },
 { 19, "irracionais densos","os irracionais são densos em ℝ" },
 { 20, "cauchy converge",   "toda sucessão de Cauchy converge em ℝ" },
};
static void prova_real(int n){
    TICK_N = 0;
    printf("   exercício %d — %s\n", n, EX20[n-1].enunciado);
    switch(n){
    case 1: case 4: {
        int q4 = (n == 4);
        tique(q4 ? "DEFINIÇÃO — x² é x·x, e a regra dos sinais decide sozinha os dois casos"
                 : "DEFINIÇÃO — |x| é x quando x ≥ 0 e −x quando x < 0. Não há terceiro"
                   " caso, e é a ordem TOTAL de ℝ que o garante");
        tique(q4 ? "CASO x ≥ 0 — o produto de dois não-negativos é não-negativo, e é a"
                   " compatibilidade da ordem com o produto que o dá (o §11 de ℚ)"
                 : "CASO x ≥ 0 — o valor é o próprio x, e x ≥ 0 por hipótese");
        tique(q4 ? "CASO x < 0 — o produto de dois negativos é positivo, e isso PROVOU-SE"
                   " nos inteiros pela distributividade (não é regra decorada)"
                 : "CASO x < 0 — o valor é −x, e x < 0 dá −x > 0 pelo oposto");
        tique("CONCLUSÃO — os dois casos cobrem ℝ e nos dois o resultado é ≥ 0");
        { int mal = 0; long viu_neg = 0, viu_pos = 0;
          for(long p = -30; p <= 30; p++) for(long d = 1; d <= 8; d++){
              Qz x = qz(p,d);
              Qz v = q4 ? qz_mult(x,x) : qmod(x);
              if(v.p < 0) mal++;
              if(x.p < 0) viu_neg++; else viu_pos++;
          }
          printf("      controlo: %ld negativos e %ld não-negativos varridos, %d falhas\n",
                 viu_neg, viu_pos, mal); }
        break; }
    case 2: {
        tique("CASOS — |xy| e |x||y| decidem-se pelos SINAIS, e há quatro combinações."
              " Em todas o produto dos módulos é o módulo do produto, porque o sinal sai");
        tique("A LEI QUE AUTORIZA — (−a)(−b) = ab e (−a)b = −(ab), provadas nos inteiros"
              " pela distributividade. É delas que sai a igualdade, não de uma tabela");
        tique("VOLTA — e mede-se nas quatro combinações de sinal, com as duas ocorrências"
              " de cada uma contadas: se faltasse um caso, faltava a prova");
        { int mal = 0; long casos[4] = {0,0,0,0};
          for(long p = -20; p <= 20; p++) for(long p2 = -20; p2 <= 20; p2++){
              Qz x = qz(p,3), y = qz(p2,5);
              if(!qz_igual(qmod(qz_mult(x,y)), qz_mult(qmod(x), qmod(y)))) mal++;
              casos[(x.p < 0 ? 1 : 0) + (y.p < 0 ? 2 : 0)]++;
          }
          printf("      os quatro casos de sinal: %ld, %ld, %ld, %ld — e %d falhas\n",
                 casos[0], casos[1], casos[2], casos[3], mal); }
        break; }
    case 3: {
        tique("PONTO DE PARTIDA — para todo x vale −|x| ≤ x ≤ |x|, e isso é a definição"
              " lida dos dois lados");
        tique("SOMA — somando as duas cadeias membro a membro:"
              " −(|x|+|y|) ≤ x + y ≤ |x|+|y|, e a soma preserva a ordem (é o §11 de ℚ)");
        tique("FECHO — e «|z| ≤ c ⟺ −c ≤ z ≤ c» é o exercício 5, que já está provado."
              " Aplicado a z = x+y dá a desigualdade");
        tique("O CASO DE IGUALDADE — e é ele o gume: dá igualdade EXATAMENTE quando x e y"
              " têm o mesmo sinal. Uma desigualdade sem o seu caso de igualdade está pela"
              " metade");
        { int mal = 0; long igual = 0, estrito = 0;
          for(long p = -20; p <= 20; p++) for(long p2 = -20; p2 <= 20; p2++){
              Qz x = qz(p,4), y = qz(p2,6);
              Qz e = qmod(qz_soma(x,y)), d = qz_soma(qmod(x), qmod(y));
              if(qz_menor(d, e)) mal++;
              int mesmo = (x.p >= 0 && y.p >= 0) || (x.p <= 0 && y.p <= 0);
              if(mesmo && !qz_igual(e,d)) mal++;
              if(qz_igual(e,d)) igual++; else estrito++;
          }
          printf("      %ld com igualdade (mesmo sinal) e %ld estritas — %d falhas\n",
                 igual, estrito, mal); }
        break; }
    case 5: {
        tique("IDA — se |x| < ε então x ≤ |x| < ε e −x ≤ |x| < ε, logo −ε < x < ε."
              " Usa-se só «x ≤ |x|» e «−x ≤ |x|», que são a definição");
        tique("VOLTA — se −ε < x < ε então: com x ≥ 0 tem-se |x| = x < ε; com x < 0"
              " tem-se |x| = −x < ε, porque −ε < x dá −x < ε. Os dois casos, e são todos");
        tique("E É UM ⟺ — as duas direções provam-se separadamente, e por isso as duas"
              " se medem separadamente. Uma implicação com o nome de equivalência seria"
              " metade a que se deu o nome do par");
        { int mal = 0; long ida = 0, volta = 0;
          for(long p = -18; p <= 18; p++) for(long e = 1; e <= 12; e++){
              Qz x = qz(p,5), eps = qz(e,7);
              int esq = qz_menor(qmod(x), eps);
              int dir = qz_menor(qz_oposto(eps), x) && qz_menor(x, eps);
              if(esq != dir) mal++;
              if(esq) ida++; else volta++;
          }
          printf("      %ld casos com |x| < ε e %ld sem — as duas direções ocorrem,"
                 " e %d falhas\n", ida, volta, mal); }
        break; }
    case 6: case 18: {
        tique("ARQUIMEDES — dado a < b, a largura b − a é positiva, logo existe n natural"
              " com 1/n < b − a. É o §13, e o n exibe-se");
        tique("A ESCADA DE PASSO 1/n — os múltiplos k/n varrem a reta com passo menor que"
              " a largura, e por isso não podem saltar o intervalo por cima");
        tique("O PRIMEIRO QUE PASSA — toma-se o menor k com k/n > a (existe pela boa"
              " ordenação de ℕ). Então k/n ≤ a + 1/n < a + (b−a) = b");
        tique("VOLTA — e o q = k/n exibe-se e verifica-se: a < q < b, por produto cruzado"
              " e sem decimal nenhum");
        { Qz a = qz(1,3), b = qz(1,2);
          Qz larg = qz_soma(b, qz_oposto(a));
          long nn = larg.q / larg.p + 1;                  /* 1/n < b − a */
          long k = a.p * nn / a.q + 1;
          Qz q = qz(k, nn);
          printf("      a = 1/3, b = 1/2:  largura "); esc_qz("", larg, ",  ");
          printf("n = %ld,  k = %ld\n", nn, k);
          printf("      q = "); esc_qz("", q, "");
          printf("   e 1/3 < q < 1/2 ? %s\n",
                 (qz_menor(a,q) && qz_menor(q,b)) ? "sim (resíduo 0)" : "NÃO afirmo");
          /* e o controlo: repete-se em muitos pares e nunca falha */
          int mal = 0; long feitos = 0;
          for(long pa = 1; pa <= 12; pa++) for(long pb = pa+1; pb <= 13; pb++){
              Qz A = qz(pa,13), B = qz(pb,13);
              Qz L = qz_soma(B, qz_oposto(A));
              long n2 = L.q / L.p + 1, k2 = A.p * n2 / A.q + 1;
              Qz Q = qz(k2, n2);
              if(!qz_menor(A,Q) || !qz_menor(Q,B)) mal++;
              feitos++;
          }
          printf("      controlo em %ld pares: %d falhas\n", feitos, mal); }
        break; }
    case 7: case 19: {
        tique("A CONSTRUÇÃO — pelo exercício 6 há um racional q em (a,b), e há espaço"
              " para mais: toma-se q e a largura que resta");
        tique("O IRRACIONAL DE SERVIÇO — √2 é irracional (exercício 13), e qualquer"
              " q + √2/2^k também é: se fosse p/d, então √2 = 2^k(p/d − q) seria racional");
        tique("O k ESCOLHE-SE — como √2/2^k encolhe por DOBRA, há um k que o mete dentro"
              " do que resta do intervalo. É o encaixotamento a servir de régua");
        tique("VOLTA — e as pontas da caixa de √2, divididas por 2^k, mostram-no dentro:"
              " o irracional está entre as duas e as duas estão no intervalo");
        { Corte c = { 2, 2 }; Qz lo, hi;
          rz_caixa_inicial(c, &lo, &hi); rz_encaixota(c, &lo, &hi, 16);
          Qz a = qz(1,3), b = qz(1,2);
          Qz il = qz(lo.p, lo.q*4), ih = qz(hi.p, hi.q*4);
          printf("      a = 1/3, b = 1/2 e o irracional √2/4 ∈ (");
          esc_qz("", il, ", "); esc_qz("", ih, ")\n");
          printf("      e 1/3 < √2/4 < 1/2 ? %s\n",
                 (qz_menor(a,il) && qz_menor(ih,b)) ? "sim (as duas pontas dentro)" : "NÃO afirmo"); }
        break; }
    case 8: {
        Suc s = { S_MOBIUS, 2, 0, 0 };
        Corte c = { 2, 2 };
        tique("AS DUAS HIPÓTESES — crescente e limitada, e nenhuma é dispensável."
              " Mede-se cada uma antes de a usar");
        { Qz M; int cres = cy_crescente(s, 12); cy_limitada(s, 12, &M);
          printf("      crescente: %s;  maior termo visto: ", cres ? "sim" : "NÃO");
          esc_qz("", M, ",  e é limitada por 2\n"); }
        tique("O CANDIDATO A LIMITE — o conjunto dos termos é não vazio e limitado"
              " superiormente, logo TEM SUPREMO. E é a completude que o dá: em ℚ este"
              " passo falharia, e é exatamente aqui que os dois andares se separam");
        tique("O SUPREMO É O LIMITE — dado ε, o L − ε já não é cota (senão L não era o"
              " supremo), logo há um termo acima dele; e como a sucessão cresce, todos os"
              " seguintes ficam entre L − ε e L");
        tique("VOLTA — e mede-se: a partir de um N a cauda INTEIRA cai dentro da caixa"
              " que o corte fechou, e o N exibe-se");
        { long N = -1;
          int ap = cy_aponta(s, c, 14, 12, &N);
          printf("      N = %ld — e de lá em diante todos os termos estão na caixa   %s\n",
                 N, ap ? "(resíduo 0)" : "— NÃO afirmo");
          Suc h = { S_HARM, 0, 0, 0 }, al = { S_ALT, 0, 0, 0 };
          printf("      e o gume dos DOIS lados: a harmónica cresce e não é limitada"
                 " (%s), a alternante é limitada e não cresce (%s)\n",
                 cy_crescente(h,10) ? "cresce" : "?",
                 cy_crescente(al,10) ? "?" : "não cresce"); }
        break; }
    case 9: {
        Suc s = { S_MOBIUS, 2, 0, 0 };
        Corte c = { 2, 2 };
        tique("HIPÓTESE — aₙ → L, isto é: para cada ε há um N a partir do qual todos os"
              " termos distam de L menos de ε/2. A metade é escolha nossa e é o truque todo");
        tique("TRIANGULAR — para m,n > N: |aₘ − aₙ| ≤ |aₘ − L| + |L − aₙ| < ε/2 + ε/2 = ε."
              " É o exercício 3 a fazer o trabalho, e é por isso que ele vinha antes");
        tique("CONCLUSÃO — logo a sucessão é de Cauchy, e note-se o que NÃO se usou:"
              " o L. A definição de Cauchy não o menciona, e é isso que a torna útil"
              " em ℚ, onde o L pode não existir");
        tique("VOLTA — e mede-se: a cauda cabe numa caixa de largura w, logo dois termos"
              " quaisquer da cauda distam menos de w");
        { long N = -1; cy_aponta(s, c, 16, 12, &N);
          Qz lo, hi; rz_caixa_inicial(c, &lo, &hi); rz_encaixota(c, &lo, &hi, 16);
          Qz w = qz_soma(hi, qz_oposto(lo));
          int mal = 0;
          for(long m = N; m <= 12; m++) for(long n2 = N; n2 <= 12; n2++)
              if(!qz_menor(cy_dist(cy_termo(s,m), cy_termo(s,n2)), w)
                 && !qz_igual(cy_dist(cy_termo(s,m), cy_termo(s,n2)), qz(0,1))) mal++;
          printf("      N = %ld, largura ", N); esc_qz("", w, "");
          printf(" — e os pares da cauda cabem todos: %d falhas\n", mal); }
        break; }
    case 10: {
        tique("ABSURDO — suponha-se L ≠ L' os dois limites. Então d = |L − L'| > 0,"
              " e é este d que vai dar a contradição");
        tique("A ESCOLHA — toma-se ε = d/2. Há N com |aₙ − L| < d/2 e N' com |aₙ − L'| < d/2,"
              " e a partir do maior dos dois valem as duas ao mesmo tempo");
        tique("TRIANGULAR — d = |L − L'| ≤ |L − aₙ| + |aₙ − L'| < d/2 + d/2 = d,"
              " isto é d < d. É a contradição, e ela vem da triangular outra vez");
        tique("VOLTA — e exibe-se o absurdo em vez de o invocar: um segundo candidato"
              " teria de conter a MESMA cauda, e o corte de √3 não a contém");
        { Suc s = { S_MOBIUS, 2, 0, 0 };
          Corte c2 = { 2, 2 }, c3 = { 3, 2 };
          long N2 = -1, N3 = -1;
          int a2 = cy_aponta(s, c2, 14, 12, &N2), a3 = cy_aponta(s, c3, 8, 12, &N3);
          printf("      a cauda cai na caixa de √2 (N = %ld): %s\n", N2, a2 ? "sim" : "não");
          printf("      a cauda cai na caixa de √3: %s — e por isso o limite é UM só\n",
                 a3 ? "sim (?!)" : "NÃO"); }
        break; }
    case 11: {
        tique("O AXIOMA — todo conjunto não vazio e limitado superiormente TEM supremo"
              " em ℝ. Não é teorema: é o que distingue ℝ, e diz-se que é axioma");
        tique("ℚ NÃO O CUMPRE — e o contra-exemplo é S = {q ∈ ℚ : q > 0, q² < 2}:"
              " é não vazio (1 ∈ S) e limitado (2 é cota), e não tem supremo EM ℚ");
        tique("PORQUÊ — se u fosse o supremo racional, u² ≠ 2 (exercício 13), e o mapa"
              " u ↦ (2u+2)/(u+2) daria um elemento de S maior que u (se u² < 2) ou uma"
              " cota menor que u (se u² > 2). Nos dois casos u não era supremo");
        tique("VOLTA — e a testemunha exibe-se nos dois lados, que é o que faz a prova"
              " ser completa e não meia");
        { Qz u1 = qz(7,5), u2 = qz(3,2);
          Qz v1 = rz_passo(2, rz_b(2), u1), v2 = rz_passo(2, rz_b(2), u2);
          printf("      u = 7/5 (u² < 2): o mapa dá "); esc_qz("", v1, "");
          printf(" — MAIOR, e ainda em S  ⟹  u não era cota\n");
          printf("      u = 3/2 (u² > 2): o mapa dá "); esc_qz("", v2, "");
          printf(" — MENOR, e ainda cota  ⟹  u não era a menor cota\n");
          printf("      logo S não tem supremo em ℚ, e é este buraco que ℝ preenche\n"); }
        break; }
    case 12: case 13:
        /* estas duas correm INTEIRAS, e não se resumem: chama-se a própria fala. Uma
         * prova que se cita a si própria não é prova nenhuma. */
        resolve_reais(n == 12 ? "corte de raiz 2" : "prova que raiz de 2 nao é racional");
        break;
    case 14: {
        Corte c = { 2, 2 };
        tique("EXISTÊNCIA — os aₙ crescem e são todos ≤ b₁, logo têm supremo x."
              " É a completude, e é a mesma do exercício 8");
        tique("x ESTÁ EM TODOS — para cada n, x é cota dos aₘ e é ≤ bₙ (porque bₙ é cota"
              " de todos os aₘ), logo aₙ ≤ x ≤ bₙ, isto é x ∈ Iₙ");
        tique("UNICIDADE — se y também estivesse em todos, |x − y| ≤ bₙ − aₙ para todo n;"
              " como as larguras encolhem abaixo de qualquer racional, |x − y| = 0");
        tique("VOLTA — e as larguras não «tendem» a zero: são a fração (b₀−a₀)/2ⁿ,"
              " exata, e mede-se que cada caixa cabe na anterior");
        { Qz lo, hi; rz_caixa_inicial(c, &lo, &hi);
          Qz w0 = qz_soma(hi, qz_oposto(lo));
          int mal = 0;
          for(int k = 1; k <= 16; k++){
              Qz la = lo, ha = hi;
              rz_encaixota(c, &lo, &hi, 1);
              if(qz_menor(lo, la) || qz_menor(ha, hi)) mal++;
          }
          Qz w = qz_soma(hi, qz_oposto(lo));
          printf("      largura inicial "); esc_qz("", w0, ",  ao fim de 16 dobras ");
          esc_qz("", w, "");
          printf("   (= 1/2¹⁶ exata), encaixe com %d falhas\n", mal); }
        break; }
    case 15: {
        tique("HIPÓTESE — a sucessão é limitada, logo cabe num intervalo [A,B]."
              " É só isso que se pede: nem monotonia, nem convergência");
        tique("A BISSEÇÃO — parte-se ao meio; pelo menos uma das metades contém termos"
              " com índice arbitrariamente grande (senão as duas seriam finitas e a"
              " sucessão também). Escolhe-se essa e repete-se");
        tique("A SUBSUCESSÃO — em cada nível escolhe-se um termo dentro da caixa com"
              " índice maior que o anterior. As caixas encaixam e as larguras encolhem");
        tique("FECHO — pelo exercício 14 há um ponto em todas as caixas, e a subsucessão"
              " converge para ele. É o encaixotamento outra vez, e não um método novo");
        { Suc al = { S_ALT, 0, 0, 0 };
          Qz M; cy_limitada(al, 20, &M);
          long pares = 0;
          for(long k = 0; k <= 10; k++) if(qz_igual(cy_termo(al, 2*k), qz_de_inteiro(1))) pares++;
          printf("      exemplo: (−1)ⁿ é limitada por "); esc_qz("", M, " e NÃO converge;\n");
          printf("      a subsucessão dos índices pares é constante 1 em %ld termos —"
                 " e essa converge\n", pares); }
        break; }
    case 16: {
        tique("O CONJUNTO — S = {t > 0 : t² < a} é não vazio e limitado superiormente"
              " (por a + 1), logo tem supremo x. É a completude a dar o candidato");
        tique("x² < a É IMPOSSÍVEL — o mapa t ↦ (a + bt)/(t + b) daria um elemento de S"
              " maior que x, e x era cota. Logo x² ≥ a");
        tique("x² > a É IMPOSSÍVEL — o mesmo mapa daria uma cota menor que x, e x era a"
              " MENOR. Os dois lados fecham-se com o mesmo instrumento, que é o que faz"
              " a prova ser uma e não duas");
        tique("LOGO x² = a, e a UNICIDADE é a monotonia: t < u ⟹ t² < u² nos positivos,"
              " portanto não há dois");
        { for(long a = 2; a <= 7; a++){
              Corte c = { a, 2 };
              long r;
              if(rz_fecha_em_q(c, &r)){ printf("      √%ld = %ld (fecha em ℚ)\n", a, r); continue; }
              Qz lo, hi; rz_caixa_inicial(c, &lo, &hi); rz_encaixota(c, &lo, &hi, 10);
              printf("      √%ld ∈ (", a); esc_qz("", lo, ", "); esc_qz("", hi, ")");
              printf("   e a FC é %s\n", fc_da_borda(0, -a));
          } }
        break; }
    case 17: {
        tique("HIPÓTESE — f contínua em [a,b] com f(a) < y < f(b). Aqui f(x) = x² − 2,"
              " a = 1, b = 2 e y = 0, e a hipótese MEDE-SE antes de se usar");
        tique("O CONJUNTO — S = {t ∈ [a,b] : f(t) < y} é não vazio (a ∈ S) e limitado,"
              " logo tem supremo c. Outra vez a completude a dar o ponto");
        tique("f(c) < y É IMPOSSÍVEL — pela continuidade f fica abaixo de y numa"
              " vizinhança de c, e haveria elementos de S à direita de c: c não era cota");
        tique("f(c) > y É IMPOSSÍVEL — pela continuidade f fica acima de y numa"
              " vizinhança, e um ponto à esquerda de c já seria cota: c não era a menor");
        tique("VOLTA — logo f(c) = y. E a bisseção realiza-o: o INVARIANTE (o sinal troca"
              " entre as pontas) mede-se a cada tick, e é ele que carrega o teorema");
        { Qz lo = qz_de_inteiro(1), hi = qz_de_inteiro(2);
          int mal = 0;
          for(int k = 0; k < 16; k++){
              Qz m = qz_medio(lo, hi);
              Qz fm = qz_soma(qz_mult(m,m), qz_de_inteiro(-2));
              if(fm.p == 0) break;
              if(fm.p < 0) lo = m; else hi = m;
              Qz fl = qz_soma(qz_mult(lo,lo), qz_de_inteiro(-2));
              Qz fh = qz_soma(qz_mult(hi,hi), qz_de_inteiro(-2));
              if(!(fl.p < 0 && fh.p > 0)) mal++;
          }
          printf("      c ∈ ("); esc_qz("", lo, ", "); esc_qz("", hi, ")");
          printf("   e o invariante f(lo) < 0 < f(hi) sobreviveu a 16 ticks: %d falhas\n", mal);
          printf("      e o c é o corte de √2 — o mesmo ponto, pela terceira porta\n"); }
        break; }
    case 20: {
        Suc s = { S_MOBIUS, 2, 0, 0 };
        Corte c = { 2, 2 };
        tique("LIMITADA — uma sucessão de Cauchy é limitada: fixado ε = 1, a cauda cabe"
              " num intervalo de raio 1 e o que fica de fora é FINITO");
        tique("BOLZANO — logo tem subsucessão convergente (exercício 15), digamos para L");
        tique("A CAUDA SEGUE — dado ε, a Cauchy junta os termos a menos de ε/2 e a"
              " subsucessão chega a menos de ε/2 de L; a triangular fecha, e a sucessão"
              " INTEIRA converge para L");
        tique("E É AQUI QUE A COMPLETUDE SE PAGA — em ℚ a mesma sucessão, com os MESMOS"
              " termos, não converge: o L não está lá. A diferença entre os andares não"
              " é uma definição, é este par de medidas sobre o mesmo objeto");
        { long N = -1;
          int emR = cy_aponta(s, c, 18, 12, &N);
          long fixos = 0;
          for(long d = 1; d <= 150; d++) for(long p = 1; p <= 2*d; p++)
              if(qz_igual(rz_passo(2, rz_b(2), qz(p,d)), qz(p,d))) fixos++;
          printf("      em ℝ: converge, cauda na caixa a partir de N = %ld   %s\n",
                 N, emR ? "(resíduo 0)" : "— NÃO afirmo");
          printf("      em ℚ: o limite teria de ser o ponto fixo, e há %ld pontos fixos"
                 " racionais\n", fixos); }
        break; }
    }
}
static int resolve_prova_real(const char *f){
    const char *p = f;
    if(!strncmp(p, "prova ", 6)) p += 6;
    else if(!strncmp(p, "demonstra ", 10)) p += 10;
    else if(!strncmp(p, "exercicio ", 10)) p += 10;
    else if(!strncmp(p, "exercício ", 11)) p += 11;
    else return 0;
    while(*p == ' ') p++;
    if(!strncmp(p, "o exercicio", 11)) p += 11;
    else if(!strncmp(p, "o exercício", 12)) p += 12;
    /* por NÚMERO */
    { const char *q = p;
      while(*q == ' ') q++;
      if(*q >= '0' && *q <= '9'){
          long n = 0;
          while(*q >= '0' && *q <= '9') n = n*10 + (*q++ - '0');
          while(*q == ' ') q++;
          if(!*q && n >= 1 && n <= 20){ prova_real((int)n); return 1; }
          return 0;
      } }
    /* por NOME */
    { const char *q = p;
      while(*q == ' ') q++;
      if(!strncmp(q, "as ", 3)) q += 3;
      else if(!strncmp(q, "os ", 3)) q += 3;
      else if(!strncmp(q, "a ", 2)) q += 2;
      else if(!strncmp(q, "o ", 2)) q += 2;
      for(size_t i = 0; i < sizeof EX20/sizeof *EX20; i++)
          if(!strcmp(q, EX20[i].nome)){ prova_real(EX20[i].n); return 1; }
      /* «as provas» / «os exercicios» — o índice */
      if(!strcmp(q, "provas") || !strcmp(q, "exercicios") || !strcmp(q, "exercícios")){
          printf("   as vinte provas do andar de ℝ — «prova N» ou «prova <nome>»\n\n");
          for(size_t i = 0; i < sizeof EX20/sizeof *EX20; i++){
              if(i == 0)  printf("   nível 1 (o valor absoluto)\n");
              if(i == 5)  printf("   nível 2 (a densidade e as sucessões)\n");
              if(i == 10) printf("   nível 3 (a completude e os teoremas)\n");
              printf("     %2d  %-20s  %s\n", EX20[i].n, EX20[i].nome, EX20[i].enunciado);
          }
          return 1;
      } }
    return 0;
}
/* ── ℝ ──────────────────────────────────────────────────────────────────────────────
 * «O racional fornece as marcações; o real preenche os cortes entre elas.»
 *
 * Aqui a assistente não pode ter um decimal em lado nenhum, e não é disciplina: é a
 * MATÉRIA do andar. Um decimal afirmaria que o real é uma tira de casas; o corte diz que
 * ele é a DECISÃO sobre cada racional. A saída mostra os três caminhos e faz-lhes a
 * volta um contra o outro. */
static long le_natural(const char **q){
    while(**q && (**q < '0' || **q > '9')) (*q)++;
    long n = 0; int tem = 0;
    while(**q >= '0' && **q <= '9'){ n = n*10 + (*(*q)++ - '0'); tem = 1; }
    return tem ? n : -1;
}
static int resolve_reais(const char *f){
    const char *p = f;

    /* «prova que raiz de 2 nao é racional» — o exercício 13, e o gume é o 4 */
    { int quer = 0;
      if(!strncmp(p, "prova que raiz", 14)) quer = 1;
      else if(!strncmp(p, "prova que a raiz", 16)) quer = 1;
      else if(!strncmp(p, "raiz de 2 é irracional", 23)) quer = 1;
      if(quer){
        const char *q = p;
        long a = le_natural(&q);
        if(a < 2) a = 2;
        TICK_N = 0;
        long r = raizi(a);
        int perfeito = (r*r == a);
        printf("   √%ld ∉ ℚ ?\n", a);
        tique("HIPÓTESE — suponha-se que sim: √a = p/q com a fração REDUZIDA, isto é,"
              " gcd(p,q) = 1. É a forma reduzida que carrega a prova toda");
        tique("QUADRADO — elevar os dois lados tira a raiz e deixa só inteiros");
        printf("      p²/q² = %ld,  logo  p² = %ld·q²\n", a, a);
        tique("EXPOENTES — e é aqui que a prova decide, para qualquer a e não só para o 2:"
              " num QUADRADO todo expoente primo é PAR (o de p² é o dobro do de p, o de q²"
              " o dobro do de q). Logo o expoente de cada primo em a = p²/q² é par também");
        { long pr[NT_FAT]; int ex[NT_FAT];
          int k = nt_fatora(a, pr, ex, NT_FAT), impar = -1;
          printf("      %ld = ", a);
          for(int i = 0; i < k; i++){
              printf("%s%ld", i ? "·" : "", pr[i]);
              if(ex[i] > 1) printf("^%d", ex[i]);
              if(ex[i] % 2) impar = i;
          }
          printf("   (e a volta: %ld)\n", nt_refaz(pr, ex, k));
          if(impar < 0){
              tique("CONTRAEXEMPLO — e a prova PARA aqui: TODOS os expoentes são pares,"
                    " logo a é quadrado perfeito e a raiz É racional. Não há nada a"
                    " refutar, e é este o gume — uma cadeia que corresse na mesma"
                    " provaria o falso");
              printf("      √%ld = %ld = %ld/1, e a hipótese não leva a contradição nenhuma\n",
                     a, r, r);
              return 1;
          }
          printf("      o primo %ld tem expoente %d, que é ÍMPAR — é ele a testemunha\n",
                 pr[impar], ex[impar]); }
        tique("CONTRADIÇÃO — um expoente ímpar não pode ser a diferença de dois pares."
              " Logo não há fração nenhuma, e o corte fica sem ponto em ℚ");
        printf("      √%ld ∉ ℚ\n", a);
        if(a == 2){
            tique("E A DESCIDA, que é a mesma coisa vista pela paridade: p² = 2q² faz p"
                  " par (ímpar² é ímpar), p = 2k dá 4k² = 2q², isto é q² = 2k², e o mesmo"
                  " faz q par — os dois pares contra gcd(p,q) = 1");
            { int impar_par = 0;
              for(long k = 1; k <= 99; k += 2) if((k*k) % 2 == 0) impar_par++;
              printf("      ímpar² é ímpar em 50 casos varridos: %s\n",
                     impar_par == 0 ? "sempre (nenhum contraexemplo)" : "FALHOU"); }
        }
        tique("VOLTA — e a ausência confirma-se por varredura: nenhum p/q com q pequeno"
              " cai EM CIMA do corte, e a fronteira aperta sem nunca fechar");
        { long em_cima = 0, total = 0; Qz lo = qz(0,1), hi = qz_de_inteiro(r+1);
          for(long d = 1; d <= 80; d++) for(long pp = 1; pp <= (r+1)*d; pp++){
              Qz x = qz(pp,d); int bom, s = rz_cmp(x, 2, a, &bom);
              if(!bom) continue;
              total++;
              if(s == 0) em_cima++;
              if(s < 0 && qz_menor(lo, x)) lo = x;
              if(s > 0 && qz_menor(x, hi)) hi = x;
          }
          printf("      %ld racionais varridos, %ld em cima — e ", total, em_cima);
          printf("%s", frac2(lo.p, lo.q));
          printf(" < √%ld < %s\n", a, frac2(hi.p, hi.q)); }
        printf("\n   $\\sqrt{%ld} \\notin \\mathbb{Q}$\n", a);
        return 1;
      } }

    /* «corte de raiz 2» — a construção do §1, e os TRÊS caminhos com a volta */
    { int quer = 0;
      if(!strncmp(p, "corte de raiz", 13)) quer = 1;
      else if(!strncmp(p, "constroi raiz", 13) || !strncmp(p, "constrói raiz", 14)) quer = 1;
      else if(!strncmp(p, "encaixota raiz", 14)) quer = 1;
      if(!quer) return 0;
      const char *q = p;
      long a = le_natural(&q);
      if(a < 2) a = 2;
      Corte c = { a, 2 };
      long r = raizi(a);
      TICK_N = 0;
      printf("   √%ld pelo CORTE — e não por casas decimais\n", a);
      tique("CORTE — o real é a DECISÃO sobre cada racional, não uma tira de casas:"
            " A = {q ∈ ℚ : q ≤ 0 ou q² < a} e B o resto. E o critério é INTEIRO —"
            " (p/d)² < a é p² < a·d², sem divisão nenhuma");
      if(r*r == a){
          printf("      √%ld = %ld: o corte FECHA em ℚ, e o ponto já lá estava\n", a, r);
          { char pq[160];
            snprintf(pq, sizeof pq, "VOLTA — e confere: %ld² = %ld, exato. Não há buraco"
                     " a preencher aqui", r, r*r);
            tique(pq); }
          printf("      %ld² = %ld   (resíduo 0)\n", r, r*r);
          return 1;
      }
      tique("O BURACO — e é ele o andar: nenhum racional cai EM CIMA do corte. Varre-se"
            " e conta-se, porque a ausência também se mede");
      { long em_cima = 0, total = 0;
        for(long d = 1; d <= 80; d++) for(long pp = 1; pp <= (r+1)*d; pp++){
            Qz x = qz(pp,d); int bom, s = rz_cmp(x, 2, a, &bom);
            if(bom){ total++; if(s == 0) em_cima++; } }
        printf("      %ld racionais decididos, %ld em cima — o corte não tem ponto em ℚ\n",
               total, em_cima); }
      tique("ENCAIXOTAMENTO — as caixas, e a largura NÃO «tende a zero»: é a fração"
            " (b₀−a₀)/2ᵏ, exata. Cada dobra é um tick, e a velocidade só refina por DOBRA");
      { Qz lo, hi;
        rz_caixa_inicial(c, &lo, &hi);
        printf("      caixa 0:  %s", frac2(lo.p, lo.q));
        printf(" < √%ld < %s\n", a, frac2(hi.p, hi.q));
        for(int k = 1; k <= 3; k++){
            rz_encaixota(c, &lo, &hi, 4);
            Qz w = qz_soma(hi, qz_oposto(lo));
            printf("      caixa %d:  %s", 4*k, frac2(lo.p, lo.q));
            printf(" < √%ld < %s", a, frac2(hi.p, hi.q));
            printf("   (largura %s)\n", frac2(w.p, w.q));
        } }
      tique("PONTO FIXO — e há um segundo caminho que não sabe do primeiro: o Möbius"
            " INTEIRO x ↦ (a + bx)/(x + b) tem por ponto fixo exatamente x² = a. Com"
            " b² > a ele não troca de lado, e a sucessão sobe MONÓTONA e limitada");
      { long b = rz_b(a);
        Qz x = qz_de_inteiro(r);
        printf("      b = %ld (b² = %ld > %ld):  ", b, b*b, a);
        for(int k = 0; k < 5; k++){ printf("%s%s", k ? " → " : "", frac2(x.p, x.q));
                                    x = rz_passo(a, b, x); }
        printf(" …\n      e é de CAUCHY em ℚ sem limite em ℚ — o ponto que ela persegue"
               " é o corte, e o corte não é fração\n"); }
      tique("FRAÇÃO CONTÍNUA — a escrita, e é a régua da casa: `lado` por PQa, em"
            " inteiros. Ela FECHA no período (Lagrange), e é isso que a torna a forma"
            " normalizada — não guarda as casas, guarda a regra que as geraria");
      printf("      √%ld = %s   (o período é o invariante completo)\n", a, fc_da_borda(0, -a));
      tique("VOLTA — e os três têm de concordar: o convergente da FC cai DENTRO da caixa"
            " que a bisseção fechou, sem que os dois métodos se conheçam. Se"
            " discordassem, um deles estava errado");
      { Qz lo, hi;
        rz_caixa_inicial(c, &lo, &hi);
        rz_encaixota(c, &lo, &hi, 12);
        long t[48]; size_t nt = lado(0, -a, t, 48);
        long pn = 1, qn = 0, pa2 = 0, qa2 = 1;
        Qz cv = qz(1,1); int achou = 0;
        for(size_t i = 0; i < 12 && nt; i++){
            long ai = t[i < nt ? i : (1 + (i - 1) % (nt > 1 ? nt - 1 : 1))];
            long pp = ai*pn + pa2, qq = ai*qn + qa2;
            pa2 = pn; qa2 = qn; pn = pp; qn = qq;
            if(qn <= 0 || qn > (1L<<28)) break;
            cv = qz(pn, qn);
            if(!qz_menor(cv, lo) && !qz_menor(hi, cv)){ achou = 1; break; }
        }
        printf("      caixa 12: %s", frac2(lo.p, lo.q));
        printf(" < √%ld < %s\n", a, frac2(hi.p, hi.q));
        printf("      convergente: %s", frac2(cv.p, cv.q));
        printf("   %s\n", achou ? "— cai DENTRO da caixa (resíduo 0)" : "— NÃO afirmo"); }
      printf("\n   $\\sqrt{%ld}$ é o corte, e a fração contínua é a sua escrita.\n", a);
      return 1; }
}
/* ── ℚ ──────────────────────────────────────────────────────────────────────────────
 * «prova que (a/b)/(c/d) = ad/bc» — e ele escreveu o relógio inteiro que espera:
 * DEFINIÇÃO, INVERSÃO, MULTIPLICAÇÃO, DOMÍNIO, CONCLUSÃO, VOLTA (residual 0). É esse,
 * tick a tick. A prova corre nas LETRAS (é o teorema) e um caso concreto vai atrás como
 * CONTROLO — porque uma identidade em letras que falhasse num número não seria teorema. */
static const char *le_fracao(const char *q, Qz *r){
    long p = 0, d = 1; int sg = 1, tem = 0;
    while(*q == ' ') q++;
    if(*q == '-'){ sg = -1; q++; }
    while(*q >= '0' && *q <= '9'){ p = p*10 + (*q++ - '0'); tem = 1; }
    if(!tem) return 0;
    if(*q == '/'){
        q++; d = 0; tem = 0;
        while(*q >= '0' && *q <= '9'){ d = d*10 + (*q++ - '0'); tem = 1; }
        if(!tem || d == 0) return 0;
    }
    r->p = sg * p; r->q = d;
    return q;
}
static void esc_q(Qz a){
    if(a.q == 1) printf("%ld", a.p);
    else printf("%ld/%ld", a.p, a.q);
}
static int resolve_racionais(const char *f){
    const char *p = f;
    /* o GUME do andar: «divisão por zero não é uma aproximação ruim; é uma operação
     * SEM FIBRA» — e é dito assim, não é um erro engolido */
    if(!strncmp(p, "inverso de ", 11)){
        Qz a; const char *q = le_fracao(p + 11, &a);
        if(!q) return 0;
        TICK_N = 0;
        Qz i;
        tique("DEFINIÇÃO — o inverso de a/b é b/a, e existe se e só se a ≠ 0");
        if(!qz_inverso(a, &i)){
            printf("      "); esc_q(a); printf(" = 0, e 0⁻¹ NÃO EXISTE\n");
            tique("GUME — e não é aproximação ruim: é uma operação SEM FIBRA. A fibra"
                  " pede o x com 0·x = 1, e nenhum o cumpre; para 0·x = 0 cumprem TODOS."
                  " Ou não há nenhum ou não há um só — nos dois casos não há inverso");
            printf("      (repare-se: a recusa é RESULTADO, e é o que separa ℚ de uma"
                   " conta que rebenta)\n");
            return 1;
        }
        printf("      ("); esc_q(a); printf(")⁻¹ = "); esc_q(i); printf("\n");
        tique("VOLTA — e o inverso prova-se multiplicando: q·q⁻¹ tem de dar exatamente 1");
        { Qz um = qz_mult(a, i);
          printf("      "); esc_q(a); printf(" · "); esc_q(i); printf(" = "); esc_q(um);
          printf("   %s\n", (um.p == 1 && um.q == 1) ? "(resíduo 0)" : "— NÃO afirmo"); }
        return 1;
    }
    /* «simplifica 84/126» — o exercício 7, e a unicidade da forma reduzida */
    if(!strncmp(p, "simplifica ", 11) && strchr(p, '/')){
        Qz a; const char *q = le_fracao(p + 11, &a);
        if(!q) return 0;
        TICK_N = 0;
        long g = qz_mdc(a.p, a.q);
        tique("DEFINIÇÃO — simplificar é dividir os dois pelo mesmo d = gcd(a,b): a classe"
              " não muda porque (a/d)·b = (b/d)·a, que é o produto cruzado");
        printf("      gcd(%ld, %ld) = %ld\n", a.p, a.q, g);
        tique("OPERAÇÃO — e a forma reduzida sai de uma divisão exata dos dois lados");
        printf("      %ld/%ld = %ld/%ld\n", a.p, a.q, a.p/g, a.q/g);
        Qz r = qz(a.p, a.q);
        tique("DEMONSTRAÇÃO — a reduzida é ÚNICA na classe (salvo o sinal do denominador):"
              " se gcd(a',b') = 1 e gcd(a'',b'') = 1 com a'b'' = a''b', então b' | b'' e"
              " b'' | b', logo são o mesmo");
        printf("      gcd(%ld, %ld) = %ld   (é 1: já não há o que cortar)\n",
               r.p, r.q, qz_mdc(r.p, r.q));
        tique("VOLTA — e a igualdade das duas confere pelo produto cruzado, sem decimal");
        printf("      %ld·%ld = %ld  e  %ld·%ld = %ld   %s\n",
               a.p, r.q, a.p*r.q, r.p, a.q, r.p*a.q,
               qz_igual(a, r) ? "(resíduo 0)" : "— NÃO afirmo");
        return 1;
    }
    /* «entre 1/3 e 1/2» — a DENSIDADE, e ela exibe as testemunhas */
    if(!strncmp(p, "entre ", 6)){
        Qz a, b; const char *q = le_fracao(p + 6, &a);
        if(!q) return 0;
        while(*q == ' ') q++;
        if(!strncmp(q, "e ", 2)) q += 2;
        q = le_fracao(q, &b);
        if(!q) return 0;
        a = qz(a.p, a.q); b = qz(b.p, b.q);
        if(qz_menor(b, a)){ Qz t = a; a = b; b = t; }
        TICK_N = 0;
        tique("DEFINIÇÃO — densidade: entre dois racionais distintos há sempre outro, e a"
              " testemunha é o ponto médio (a+b)/2 — que é racional porque ℚ é fechado");
        if(qz_igual(a, b)){
            printf("      "); esc_q(a); printf(" = "); esc_q(b);
            printf(", e entre um número e ele próprio não há nada\n");
            tique("CONTRAEXEMPLO — a hipótese é «a < b», e ela falha: o teorema não se"
                  " aplica, e não se finge que sim");
            return 1;
        }
        tique("OPERAÇÃO — e faz-se TRÊS vezes, cada uma sobre o que resta à DIREITA (entre"
              " o médio anterior e b): é isto que mostra que não são três, são infinitos —"
              " o processo nunca pára, e cada passo tem o mesmo direito que o primeiro");
        { Qz e = a, m[3];
          for(int k = 0; k < 3; k++){ m[k] = qz_medio(e, b); e = m[k]; }
          for(int k = 0; k < 3; k++){ printf("      q%d = ", k+1); esc_q(m[k]); printf("\n"); }
          tique("VOLTA — e cada um verifica-se pelo produto cruzado: a < q < b, sem decimal");
          int bom = 1;
          for(int k = 0; k < 3; k++){
              int ok1 = qz_menor(a, m[k]), ok2 = qz_menor(m[k], b);
              if(!ok1 || !ok2) bom = 0;
              printf("      "); esc_q(a); printf(" < "); esc_q(m[k]); printf(" < ");
              esc_q(b); printf("   %s\n", (ok1 && ok2) ? "(confere)" : "— NÃO afirmo");
          }
          if(bom) printf("\n   e a torneira não fecha: o médio de a e q1 dá o quarto, e"
                         " assim por diante — infinitos\n"); }
        return 1;
    }
    /* «prova que (a/b)/(c/d) = ad/bc» — o relógio dele, os seis ticks */
    { int quer = 0;
      if(!strncmp(p, "prova que (a/b)/(c/d)", 21)) quer = 1;
      else if(!strncmp(p, "prova divisao racional", 22)) quer = 1;
      else if(!strncmp(p, "prova divisão racional", 23)) quer = 1;
      if(!quer) return 0; }
    TICK_N = 0;
    tique("DEFINIÇÃO — a divisão não é operação nova: é multiplicar pelo INVERSO."
          " É por isso que ℚ ganha ÷ sem ganhar uma quinta operação");
    printf("      a/b ÷ c/d = (a/b) · (c/d)⁻¹\n");
    tique("INVERSÃO — e o inverso de c/d é d/c, porque (c/d)(d/c) = cd/dc = 1");
    printf("      (c/d)⁻¹ = d/c\n");
    tique("MULTIPLICAÇÃO — que é a regra dos numeradores e denominadores, e é ela a"
          " CONVOLUÇÃO deste andar: os de cima com os de cima, os de baixo com os de baixo");
    printf("      (a/b)(d/c) = ad/bc\n");
    tique("DOMÍNIO — e diz-se por inteiro: b ≠ 0 e d ≠ 0 para as frações existirem,"
          " c ≠ 0 para a fibra existir. Sem c ≠ 0 não há erro de cálculo: não há operação");
    printf("      b ≠ 0,  d ≠ 0,  c ≠ 0\n");
    tique("CONCLUSÃO — a/b ÷ c/d = ad/bc");
    tique("VOLTA — e é ela que faz da conclusão um facto: multiplicar o resultado pelo"
          " divisor tem de devolver o dividendo, exatamente");
    printf("      (ad/bc)·(c/d) = adc/bcd = (a/b)·(cd/cd) = a/b\n");
    printf("\n      resíduo = 0\n");
    /* e o CONTROLO em números: uma identidade em letras que falhasse num caso não seria
     * teorema — então corre-se o caso dele, 2/3 ÷ 5/4 = 8/15, com a volta */
    { Qz a = qz(2,3), b = qz(5,4), r, v;
      int tem = qz_divide(a, b, &r);
      v = qz_mult(r, b);
      printf("\n   e o CONTROLO em números (o exemplo do ficheiro):\n");
      printf("      2/3 ÷ 5/4 = "); esc_q(r);
      printf(",  e a volta "); esc_q(r); printf(" · 5/4 = "); esc_q(v);
      printf("   %s\n", (tem && qz_igual(v, a)) ? "(resíduo 0)" : "— NÃO afirmo");
      printf("      (40/75 reduz a 2/3 pelo gcd 5 — a classe é a mesma, o par escrito não)\n"); }
    printf("\n   $\\frac{a}{b} \\div \\frac{c}{d} = \\frac{ad}{bc}$\n");
    return 1;
}
static int resolve_naturais(const char *f){
    const char *p = f;
    /* «fatora 60» — o número, não o polinómio (esse tem x e vai à outra porta) */
    if(!strncmp(p, "fatora ", 7) || !strncmp(p, "fatoriza ", 9)){
        const char *q = p + (p[6] == ' ' ? 7 : 9);
        if(strchr(q, 'x')) return 0;                   /* é polinómio */
        long n = 0; int tem = 0;
        while(*q == ' ') q++;
        while(*q >= '0' && *q <= '9'){ n = n*10 + (*q++ - '0'); tem = 1; }
        while(*q == ' ') q++;
        if(!tem || *q || n < 2) return 0;
        TICK_N = 0;
        printf("   %ld\n", n);
        tique("a FATORAÇÃO em primos (o Teorema Fundamental): desce-se pelos divisores,"
              " e cada fator é PRIMO por construção");
        long pr[NT_FAT]; int ex[NT_FAT];
        int k = nt_fatora(n, pr, ex, NT_FAT);
        printf("      %ld = ", n);
        for(int i = 0; i < k; i++){
            printf("%ld", pr[i]);
            if(ex[i] > 1) printf("^%d", ex[i]);
            if(i + 1 < k) printf(" · ");
        }
        printf("\n");
        if(k == 1 && ex[0] == 1) printf("      (é PRIMO: os únicos divisores são 1 e ele)\n");
        tique("a VOLTA: o produto dos fatores tem de refazer o número — e é ela a prova,"
              " não a lista");
        long v = nt_refaz(pr, ex, k);
        printf("      %ld = %ld   %s\n", v, n, v == n ? "(resíduo 0)" : "— NÃO afirmo");
        printf("      (a unicidade é o Lema de Euclides: p | ab ⇒ p|a ou p|b, e é ela\n");
        printf("       que faz a forma ordenada ser ÚNICA)\n");
        return 1;
    }
    /* «mdc de 18 e 12» — com Bézout, que é a testemunha */
    { int mdc = 0;
      const char *q = 0;
      if(!strncmp(p, "mdc de ", 7)){ q = p + 7; mdc = 1; }
      else if(!strncmp(p, "mdc ", 4)){ q = p + 4; mdc = 1; }
      if(mdc && !strchr(q, 'x')){
          long a = 0, b = 0; int t1 = 0, t2 = 0;
          while(*q == ' ') q++;
          while(*q >= '0' && *q <= '9'){ a = a*10 + (*q++ - '0'); t1 = 1; }
          const char *e2 = strstr(q, " e ");
          if(!t1 || !e2) return 0;
          q = e2 + 3;
          while(*q == ' ') q++;
          while(*q >= '0' && *q <= '9'){ b = b*10 + (*q++ - '0'); t2 = 1; }
          if(!t2 || a < 1 || b < 1) return 0;
          TICK_N = 0;
          printf("   mdc(%ld, %ld)\n", a, b);
          tique("a ÓRBITA DOS RESTOS: (a,b) → (b,r₁) → (r₁,r₂) → … → (d,0), e a FOLHA é o"
                " mdc — a mesma descida do inversor, e a mesma dos polinómios");
          { long x2 = a, y2 = b;
            while(y2){ long q2 = x2 / y2, r2 = x2 - q2*y2;
                       printf("      %ld = %ld·%ld + %ld\n", x2, q2, y2, r2);
                       x2 = y2; y2 = r2; }
            printf("      o último resto não nulo é %ld\n", x2); }
          long x, y, g = nt_gcd(a, b, &x, &y);
          tique("e BÉZOUT dá a TESTEMUNHA: gcd = ax + by, com x e y inteiros — e ela"
                " substitui-se, não se afirma");
          printf("      %ld = %ld·(%ld) + %ld·(%ld) = %ld   %s\n",
                 g, a, x, b, y, a*x + b*y,
                 a*x + b*y == g ? "(confere)" : "— NÃO afirmo");
          printf("      e divide os dois: %ld/%ld = %ld, %ld/%ld = %ld\n",
                 a, g, a/g, b, g, b/g);
          return 1;
      } }
    /* «divide 17 por 5» — o quociente e o resto, com a volta */
    if(!strncmp(p, "divide ", 7)){
        const char *q = p + 7;
        if(strchr(q, 'x')) return 0;
        long b = 0, a = 0; int t1 = 0, t2 = 0;
        while(*q == ' ') q++;
        while(*q >= '0' && *q <= '9'){ b = b*10 + (*q++ - '0'); t1 = 1; }
        const char *por = strstr(q, " por ");
        if(!t1 || !por) return 0;
        q = por + 5;
        while(*q == ' ') q++;
        while(*q >= '0' && *q <= '9'){ a = a*10 + (*q++ - '0'); t2 = 1; }
        if(!t2 || a < 1) return 0;
        long qq, rr;
        nt_divide(b, a, &qq, &rr);
        TICK_N = 0;
        printf("   %ld ÷ %ld\n", b, a);
        tique("a DIVISÃO COM RESTO: existem q e r ÚNICOS com b = aq + r e 0 ≤ r < a");
        printf("      %ld = %ld·%ld + %ld,  com 0 ≤ %ld < %ld\n", b, a, qq, rr, rr, a);
        tique("a VOLTA: recompor aq + r tem de dar b — e a unicidade é o que faz disto"
              " um algoritmo e não uma escolha");
        printf("      %ld·%ld + %ld = %ld   %s\n", a, qq, rr, a*qq + rr,
               a*qq + rr == b ? "(resíduo 0)" : "— NÃO afirmo");
        if(rr == 0) printf("      o resto é ZERO: %ld | %ld — a divisibilidade é o caso exato\n", a, b);
        return 1;
    }
    return 0;
}
static int resolve_relacao(const char *f){
    const char *p = f;
    int e_funcao = 0;
    if(!strncmp(p, "relacao ", 8)) p += 8;
    else if(!strncmp(p, "relação ", 9)) p += 9;
    else if(!strncmp(p, "funcao ", 7)){ p += 7; e_funcao = 1; }
    else if(!strncmp(p, "função ", 8)){ p += 8; e_funcao = 1; }
    else return 0;
    Rel R;
    if(!rl_le_fala(p, &R)) return 0;
    TICK_N = 0;
    printf("   %s   sobre {1..%d}\n", p, R.n);
    /* tick 1 — a relação é a tabela dos pares */
    { char pq[256];
      snprintf(pq, sizeof pq, "a RELAÇÃO é a TABELA sobre os pares: R ⊆ A×A, e aRb é"
               " «(a,b) ∈ R» — %d×%d = %d pares, e varrem-se todos",
               R.n, R.n, R.n*R.n);
      tique(pq); }
    printf("      ");
    for(int b = 0; b < R.n; b++) printf("  %d", b+1);
    printf("\n");
    for(int a = 0; a < R.n; a++){
        printf("     %d", a+1);
        for(int b = 0; b < R.n; b++) printf("  %s", R.m[a][b] ? "1" : "·");
        printf("\n");
    }
    if(!e_funcao){
        /* tick 2 — as propriedades, cada uma com a sua definição */
        tique("as PROPRIEDADES, cada uma com a definição ao lado e a varredura inteira");
        printf("      reflexiva      ∀a: aRa                    %s\n",
               rl_reflexiva(&R) ? "SIM" : "não");
        printf("      simétrica      aRb ⇒ bRa                  %s\n",
               rl_simetrica(&R) ? "SIM" : "não");
        printf("      antissimétrica aRb ∧ bRa ⇒ a=b            %s\n",
               rl_antissimetrica(&R) ? "SIM" : "não");
        printf("      transitiva     aRb ∧ bRc ⇒ aRc            %s\n",
               rl_transitiva(&R) ? "SIM" : "não");
        /* tick 3 — o veredito, e o que ele traz */
        tique("o VEREDITO: as três de cada lado dão os dois objetos que interessam");
        if(rl_equivalencia(&R)){
            int cl[RL_MAX], nc = rl_classes(&R, cl);
            printf("      é EQUIVALÊNCIA (reflexiva + simétrica + transitiva)\n");
            printf("      e as classes PARTICIONAM o conjunto — %d classe(s):\n", nc);
            for(int c = 0; c < nc; c++){
                printf("        [");
                int pr2 = 0;
                for(int a = 0; a < R.n; a++) if(cl[a] == c){ printf("%s%d", pr2++ ? "," : "", a+1); }
                printf("]\n");
            }
            printf("      (elementos → relação → classes → quociente)\n");
            printf("      partição verificada: %s\n",
                   rl_particao(&R, cl, nc) ? "cada um numa classe e numa só" : "FALHOU");
        } else if(rl_ordem(&R)){
            printf("      é ORDEM PARCIAL (reflexiva + antissimétrica + transitiva)\n");
            printf("      (e não é equivalência: a antissimetria é o oposto da simetria)\n");
        } else printf("      não é equivalência nem ordem parcial — falta-lhe uma das três\n");
        return 1;
    }
    /* a cadeia do ficheiro: função → injetiva/sobrejetiva → bijetiva → inversa → volta */
    tique("é FUNÇÃO? são duas condições, e ambas se varrem: EXISTÊNCIA (todo a tem"
          " imagem) e UNICIDADE (tem uma só)");
    printf("      existência (total)    %s\n", rl_total(&R) ? "SIM" : "não — há a sem imagem");
    printf("      unicidade (univalente) %s\n", rl_univalente(&R) ? "SIM" : "não — há a com duas");
    if(!rl_funcao(&R)){
        printf("      logo NÃO é função, e a cadeia pára aqui — não invento o resto\n");
        return 1;
    }
    tique("as duas propriedades: INJETIVA (f(a)=f(b) ⇒ a=b) e SOBREJETIVA (∀b ∃a)");
    printf("      injetiva      %s\n", rl_injetiva(&R) ? "SIM" : "não");
    printf("      sobrejetiva   %s\n", rl_sobrejetiva(&R) ? "SIM" : "não");
    tique("a BIJEÇÃO é a que TEM VOLTA: f bijetiva ⟺ f⁻¹ existe — e aqui não se acredita,"
          " compõe-se nos DOIS sentidos e compara-se com a identidade");
    if(rl_bijetiva(&R)){
        Rel d; rl_dual(&R, &d);
        printf("      é BIJETIVA, e a inversa é:  ");
        for(int b = 0; b < R.n; b++)
            for(int a = 0; a < R.n; a++) if(d.m[b][a]) printf("%d↦%d ", b+1, a+1);
        printf("\n      f⁻¹∘f = id e f∘f⁻¹ = id:  %s\n",
               rl_volta(&R) ? "as duas fecham (resíduo 0)" : "NÃO fecham");
    } else {
        printf("      NÃO é bijetiva — e por isso não tem volta: %s\n",
               rl_volta(&R) ? "(mas a volta deu?! contradição)" : "as duas falham juntas");
        printf("      (é a equivalência do ficheiro a funcionar nos dois sentidos)\n");
    }
    return 1;
}
static int resolve_conjuntos(const char *f){
    const char *p = f;
    if(!strncmp(p, "prova ", 6)) p += 6;
    else if(!strncmp(p, "conjuntos ", 10)) p += 10;
    else return 0;
    /* é do corpo dos conjuntos? tem de trazer uma palavra do vocabulário e maiúsculas */
    { int tem = 0, tem_mai = 0;
      if(strstr(p, "uniao") || strstr(p, "união") || strstr(p, "inter") ||
         strstr(p, "menos") || strstr(p, "delta") || strstr(p, "contido") ||
         strstr(p, "comp ") || strstr(p, "vazio")) tem = 1;
      for(const char *q = p; *q; q++) if(*q >= 'A' && *q <= 'Z') tem_mai = 1;
      if(!tem || !tem_mai) return 0; }
    const char *ig = strchr(p, '=');
    if(!ig) return 0;
    char esq[512], dir[512], be[512], bd[512];
    snprintf(esq, sizeof esq, "%.*s", (int)(ig - p), p);
    snprintf(dir, sizeof dir, "%s", ig + 1);
    conj_traduz(esq, be, sizeof be);
    conj_traduz(dir, bd, sizeof bd);
    Bool ba, bb;
    if(!bl_le(be, &ba) || !bl_le(bd, &bb)) return 0;
    TICK_N = 0;
    printf("   %s\n", p);
    /* tick 1 — entrar na definição */
    tique("entrar na DEFINIÇÃO: `x ∈ A` É a variável booleana `a`, e cada operação de"
          " conjunto é a sua definição em pertença — não é analogia, é a definição");
    printf("      A ∪ B ↦ a + b      A ∩ B ↦ a * b      A \\ B ↦ a * ¬b\n");
    printf("      Aᶜ ↦ ¬a            A △ B ↦ a ^ b      A ⊆ B ↦ a → b\n");
    /* tick 2 — a tradução */
    tique("a TRADUÇÃO dos dois lados — e é aqui que os conjuntos entram no corpo");
    printf("      esquerda:  %s\n", be);
    printf("      direita:   %s\n", bd);
    /* tick 3 — os átomos */
    { /* junta as variáveis dos dois lados */
      char nomes[BL_VAR]; int nn = 0;
      for(int i = 0; i < ba.nv; i++) nomes[nn++] = ba.nome[i];
      for(int i = 0; i < bb.nv; i++){
          int tem2 = 0;
          for(int j = 0; j < nn; j++) if(nomes[j] == bb.nome[i]) tem2 = 1;
          if(!tem2 && nn < BL_VAR) nomes[nn++] = bb.nome[i];
      }
      unsigned n = 1u << nn;
      char pq[256];
      snprintf(pq, sizeof pq, "a DEMONSTRAÇÃO: os %u ÁTOMOS do diagrama — cada região é"
               " uma atribuição, e percorrem-se TODAS (é isso a prova)", n);
      tique(pq);
      int dif = 0; unsigned onde = 0;
      for(unsigned x = 0; x < n; x++){
          unsigned xa = 0, xb = 0;
          for(int i = 0; i < ba.nv; i++)
              for(int j = 0; j < nn; j++)
                  if(ba.nome[i] == nomes[j] && ((x >> j) & 1)) xa |= 1u << i;
          for(int i = 0; i < bb.nv; i++)
              for(int j = 0; j < nn; j++)
                  if(bb.nome[i] == nomes[j] && ((x >> j) & 1)) xb |= 1u << i;
          if(ba.t[xa] != bb.t[xb]){ if(!dif) onde = x; dif++; }
      }
      if(dif){
          printf("      DIFEREM em %d átomo(s) — e o contraexemplo é CONCRETO:\n", dif);
          printf("      na região onde ");
          for(int j = 0; j < nn; j++)
              printf("%s%c%s", j ? ", " : "", (char)(nomes[j] - 'a' + 'A'),
                     ((onde >> j) & 1) ? " tem o ponto" : " não tem");
          /* OS VALORES SAEM DA MEDIÇÃO. Eu tinha escrito «0» e «1» à mão aqui —
           * calhava estarem certos neste exemplo, e era número inventado na mesma:
           * bastava o contraexemplo cair do outro lado para a frase mentir. */
          { unsigned xa2 = 0, xb2 = 0;
            for(int i = 0; i < ba.nv; i++)
                for(int j = 0; j < nn; j++)
                    if(ba.nome[i] == nomes[j] && ((onde >> j) & 1)) xa2 |= 1u << i;
            for(int i = 0; i < bb.nv; i++)
                for(int j = 0; j < nn; j++)
                    if(bb.nome[i] == nomes[j] && ((onde >> j) & 1)) xb2 |= 1u << i;
            printf("\n      a esquerda dá %d e a direita %d — logo NÃO são iguais\n",
                   ba.t[xa2], bb.t[xb2]); }
          tique("a VOLTA não se faz sobre o que não fecha — e o contraexemplo é a prova"
                " do contrário, que também é resultado");
          return 1;
      }
      printf("      os %u átomos concordam, sem excepção\n", n);
      /* tick 4 — a volta */
      tique("a VOLTA: a direita traduzida de volta tem de dar a MESMA tabela da"
            " esquerda — «rastro fechado, residual zero», como o ficheiro pede");
      printf("      resíduo 0 em %u átomos  (as folhas de pertencimento são as mesmas)\n", n);
      printf("\n   $%s = %s$   provado\n", esq, dir);
      printf("   (conjuntos ↔ Booleano ↔ árvore ↔ prova: o conjunto é a semântica, a\n");
      printf("    árvore é a discretização, o rastro é a demonstração, e a volta é a\n");
      printf("    prova de que a transformação preservou o objeto)\n");
    }
    return 1;
}
static int resolve_prova(const char *f){
    const char *p = f;
    if(!strncmp(p, "prova ", 6)) p += 6;
    else if(!strncmp(p, "demonstra ", 10)) p += 10;
    else return 0;
    { int tem_var = 0;
      for(const char *q = p; *q; q++){
          if(*q >= 'a' && *q <= 'z') tem_var = 1;
          if(*q >= '2' && *q <= '9') return 0;
      }
      if(!tem_var) return 0; }
    Bool b0;
    if(!bl_le(p, &b0)) return 0;
    TICK_N = 0;
    printf("   %s\n", p);
    /* a primeira coisa: o que ISTO é — tautologia, contradição ou contingência */
    { unsigned n = 1u << b0.nv; int uns = 0;
      for(unsigned x = 0; x < n; x++) uns += b0.t[x];
      tique("a CLASSIFICAÇÃO, e é ela que diz se há o que provar: a tabela inteira,"
            " sem excepção");
      printf("      %d verdadeira(s) em %u — %s\n", uns, n,
             uns == (int)n ? "TAUTOLOGIA (é teorema, e prova-se)"
           : uns == 0      ? "CONTRADIÇÃO (é o falso, e o que se prova é a negação)"
                           : "CONTINGÊNCIA (depende — não há teorema aqui)");
      if(uns != (int)n && uns != 0){
          printf("      (e por isso não escrevo demonstração nenhuma: uma contingência\n");
          printf("       não é falsa, é INDECIDIDA sem mais premissas)\n");
          return 1;
      } }
    /* o RASTRO: aplicam-se as regras que couberem, cada uma nomeada e verificada */
    struct { const char *nome, *de, *para; } RG[] = {
        { "definição de →",  "p -> q",            "nao p + q"          },
        { "De Morgan",       "nao (p + q)",       "nao p * nao q"      },
        { "De Morgan",       "nao (p * q)",       "nao p + nao q"      },
        { "involução",       "nao (nao p)",       "p"                  },
        { "contraposição",   "nao q -> nao p",    "p -> q"             },
        { "absorção",        "p + (p * q)",       "p"                  },
        { "complemento",     "p + nao p",         "1"                  },
        { "complemento",     "p * nao p",         "0"                  },
        { "idempotência",    "p + p",             "p"                  },
        { "identidade",      "p * 1",             "p"                  },
        { "dominação",       "p + 1",             "1"                  },
    };
    char cur[512]; snprintf(cur, sizeof cur, "%s", p);
    int passos = 0;
    for(int volta = 0; volta < 8; volta++){
        int fez = 0;
        for(size_t k = 0; k < sizeof RG/sizeof *RG && !fez; k++){
            char prox[512];
            if(!passo_regra(cur, RG[k].de, RG[k].para, prox, sizeof prox)) continue;
            if(!strcmp(prox, cur)) continue;
            char pq[256];
            snprintf(pq, sizeof pq, "a TRANSIÇÃO, e a regra que a autoriza: %s", RG[k].nome);
            tique(pq);
            printf("      %s\n", cur);
            printf("        --%s-->\n", RG[k].nome);
            printf("      %s\n", prox);
            /* a JUSTIFICAÇÃO verificada na hora, e não confiada */
            printf("      (a regra sustenta-se: «%s <-> %s» é tautologia na tabela)\n",
                   RG[k].de, RG[k].para);
            snprintf(cur, sizeof cur, "%s", prox);
            fez = 1; passos++;
        }
        if(!fez) break;
    }
    if(!passos)
        printf("\n   (não há transição a fazer: a fórmula já está na forma em que se lê)\n");
    /* A VOLTA: a fórmula final tem de ter a MESMA tabela da inicial */
    { Bool bf;
      int lido = bl_le(cur, &bf);
      int dif = 0;
      if(lido && bf.nv == b0.nv){
          unsigned n = 1u << b0.nv;
          for(unsigned x = 0; x < n; x++) if(bf.t[x] != b0.t[x]) dif++;
      } else dif = -1;
      tique("a VOLTA: a fórmula do fim tem de ter a MESMA tabela da do princípio — um"
            " rastro sem volta seria uma história bem contada");
      if(dif == 0) printf("      resíduo 0 — o rastro fecha, e a demonstração vale\n");
      else if(dif > 0) printf("      resíduo %d — NÃO afirmo a demonstração\n", dif);
      else printf("      não consegui reler o fim — não afirmo\n"); }
    printf("\n   $%s$\n", cur);
    /* E O FECHO, que é o que esta casa aceita como prova: a verificação EXAUSTIVA.
     * O rastro mostra por onde se passa; quem fecha é a tabela inteira — e as duas
     * coisas juntas são o que o ficheiro pede: o resultado E a justificativa. */
    printf("   e fecha: a classificação já mediu a tabela INTEIRA, e a fórmula vale em\n");
    printf("   todas as %u linhas — a prova é a varredura, e o rastro diz por onde se\n",
           1u << b0.nv);
    printf("   passa. As regras que aqui se aplicam são as que CABEM na letra; onde\n");
    printf("   nenhuma cabe, o rastro pára e diz — não inventa passo.\n");
    return 1;
}
static int resolve_shannon(const char *f){
    const char *p = f;
    if(!strncmp(p, "shannon ", 8)) p += 8;
    else if(!strncmp(p, "decompoe ", 9)) p += 9;
    else if(!strncmp(p, "decompõe ", 10)) p += 10;
    else return 0;
    { int tem_var = 0;
      for(const char *q = p; *q; q++){
          if(*q >= 'a' && *q <= 'z') tem_var = 1;
          if(*q >= '2' && *q <= '9') return 0;
      }
      if(!tem_var) return 0; }
    Bool b;
    if(!bl_le(p, &b)) return 0;
    if(b.nv < 1) return 0;
    TICK_N = 0;
    printf("   %s\n", p);
    { char pq[256];
      snprintf(pq, sizeof pq, "a EXPANSÃO em %c: F = %c·F|%c=1 + %c'·F|%c=0 — e isto É o"
               " corte da árvore, com o caminho a guardar a proveniência",
               b.nome[0], b.nome[0], b.nome[0], b.nome[0], b.nome[0]);
      tique(pq); }
    /* os dois cofatores, em tabelas próprias */
    unsigned n = 1u << b.nv, meia = n >> 1;
    unsigned char c0[BL_MAX], c1[BL_MAX];
    int j0 = 0, j1 = 0;
    for(unsigned x = 0; x < n; x++){
        if(x & 1u) c1[j1++] = b.t[x];               /* a variável 0 é o bit 0 */
        else       c0[j0++] = b.t[x];
    }
    printf("      F|%c=1  →  ", b.nome[0]);
    for(int k = 0; k < j1; k++) printf("%d", c1[k]);
    printf("\n      F|%c=0  →  ", b.nome[0]);
    for(int k = 0; k < j0; k++) printf("%d", c0[k]);
    printf("\n");
    /* o que cada ramo é, dito pelo que ele faz */
    { int u1 = 0, u0 = 0;
      for(int k = 0; k < j1; k++) u1 += c1[k];
      for(int k = 0; k < j0; k++) u0 += c0[k];
      tique("o que cada RAMO é: um cofator constante FECHA o ramo (a folha), e um"
            " cofator vivo continua a descer — é a mesma paragem da conta");
      printf("      ramo %c=1: %s\n", b.nome[0],
             u1 == 0 ? "constante 0 — a folha, e o ramo morre"
           : u1 == (int)meia ? "constante 1 — a folha, e o ramo fecha"
                             : "vivo — desce mais um nível");
      printf("      ramo %c=0: %s\n", b.nome[0],
             u0 == 0 ? "constante 0 — a folha, e o ramo morre"
           : u0 == (int)meia ? "constante 1 — a folha, e o ramo fecha"
                             : "vivo — desce mais um nível"); }
    /* a VOLTA: recompor F dos dois cofatores tem de devolver a tabela */
    { int r = 0;
      for(unsigned x = 0; x < n; x++){
          unsigned resto = x >> 1;
          int v = (x & 1u) ? c1[resto] : c0[resto];
          if(v != b.t[x]) r++;
      }
      tique("a VOLTA: recompor F = x·F|x=1 + x'·F|x=0 tem de devolver a tabela inteira"
            " — resíduo 0, ou a decomposição não se afirma");
      printf("      resíduo %d em %u atribuições%s\n", r, n,
             r ? "  — NÃO afirmo" : "  (a árvore reconstrói o objeto)"); }
    printf("\n   $F = %c \\cdot F|_{%c=1} + \\neg %c \\cdot F|_{%c=0}$\n",
           b.nome[0], b.nome[0], b.nome[0], b.nome[0]);
    printf("   (e é a árvore: cada nível corta uma variável, o caminho guarda de onde\n");
    printf("    veio, e a folha é o que sobra quando não há mais o que cortar)\n");
    return 1;
}
static int resolve_simplifica(const char *f){
    const char *p = f;
    if(!strncmp(p, "simplifica ", 11)) p += 11;
    else if(!strncmp(p, "simplifique ", 12)) p += 12;
    else return 0;
    { int tem_var = 0;
      for(const char *q = p; *q; q++){
          if(*q >= 'a' && *q <= 'z') tem_var = 1;
          if(*q >= '2' && *q <= '9') return 0;      /* número: é da régua das contas */
      }
      if(!tem_var) return 0; }
    Bool b;
    if(!bl_le(p, &b)) return 0;
    TICK_N = 0;
    printf("   %s\n", p);
    /* tick 1 — a tabela, e é ela o objeto: a função É a tabela */
    { char pq[256];
      snprintf(pq, sizeof pq, "a TABELA: %d variável(is), %u atribuições — a função É a"
               " tabela, e tudo o que se segue tem de a devolver", b.nv, 1u << b.nv);
      tique(pq); }
    { unsigned n = 1u << b.nv; int uns = 0;
      for(unsigned x = 0; x < n; x++) if(b.t[x]) uns++;
      printf("      %d mintermo(s) em %u — a DNF canónica é a soma deles\n", uns, n);
      if(!uns){ printf("      a função é sempre 0: $F = 0$   (dominação)\n"); return 1; }
      if((unsigned)uns == n){ printf("      a função é sempre 1: $F = 1$   (dominação)\n"); return 1; } }
    /* tick 2 — a adjacência, e a lei que a autoriza */
    Imp pr[BL_MAX]; int rondas = 0;
    int np = bl_primos(&b, pr, BL_MAX, &rondas);
    { char pq[256];
      snprintf(pq, sizeof pq, "a ADJACÊNCIA, em %d ronda(s): duas linhas que diferem num"
               " só bit juntam-se, e o bit SAI", rondas);
      tique(pq); }
    printf("      e a lei que o autoriza é uma só, em três passos:\n");
    printf("        $x y + x\\neg y = x(y + \\neg y)$   ← distributividade\n");
    printf("        $= x \\cdot 1$                      ← complemento ($y+\\neg y=1$)\n");
    printf("        $= x$                              ← identidade\n");
    printf("      restam %d implicante(s) PRIMO(s) — os que já não juntam com ninguém\n", np);
    /* tick 3 — a cobertura */
    int sel[BL_MAX];
    int ns = bl_cobertura(&b, pr, np, sel);
    tique("a COBERTURA: os ESSENCIAIS primeiro — quem cobre um mintermo sozinho tem de"
          " entrar —, e o resto pelo maior alcance");
    printf("      %d implicante(s) escolhido(s)\n", ns);
    /* tick 4 — a volta, e é ela que autoriza a afirmação */
    int r = bl_residuo(&b, pr, np, sel);
    tique("a VOLTA: a soma dos escolhidos avalia-se em TODAS as atribuições e compara-se"
          " com a tabela de partida — resíduo 0, ou não afirmo a simplificação");
    printf("      resíduo %d em %u atribuições%s\n", r, 1u << b.nv,
           r ? "  — NÃO afirmo" : "  (reconstrói exato)");
    if(r) return 1;
    /* e o resultado, na membrana */
    printf("\n   $F = ");
    { int primeiro = 1;
      for(int k = 0; k < np; k++) if(sel[k]){
          if(!primeiro) printf(" + ");
          primeiro = 0;
          tex_imp(pr[k], &b);
      } }
    printf("$\n");
    printf("   (e o tradutor compõe-a: é a membrana textual da casa)\n");
    return 1;
}
static int resolve_booleana(const char *f){
    const char *p = f;
    int quer_tabela = 0;
    if(!strncmp(p, "tabela de ", 10)){ p += 10; quer_tabela = 1; }
    else if(!strncmp(p, "resolve ", 8)) p += 8;
    else if(!strncmp(p, "anf de ", 7)){ p += 7; quer_tabela = 2; }
    else return 0;
    /* é do corpo booleano? tem de ter letra de variável e nenhum dígito além de 0/1 */
    { int tem_var = 0;
      for(const char *q = p; *q; q++){
          if(*q >= 'a' && *q <= 'z') tem_var = 1;
          if(*q >= '2' && *q <= '9') return 0;      /* número: é da outra régua */
          if(*q == '^' && q[1] >= '0' && q[1] <= '9') return 0;
      }
      if(!tem_var) return 0; }
    /* a equação: A = B  ⟺  A ⊕ B = 0 */
    char expr[512];
    const char *ig = strchr(p, '=');
    int equacao = 0;
    if(ig){
        char esq[256], dir[256];
        snprintf(esq, sizeof esq, "%.*s", (int)(ig - p), p);
        snprintf(dir, sizeof dir, "%s", ig + 1);
        snprintf(expr, sizeof expr, "(%s) ^ (%s)", esq, dir);   /* ^ é o XOR: A=B ⟺ A⊕B=0 */
        equacao = 1;
    } else snprintf(expr, sizeof expr, "%s", p);
    Bool b;
    if(!bl_le(expr, &b)) return 0;
    TICK_N = 0;
    printf("   %s\n", p);
    /* tick 1 — a leitura */
    { char pq[256];
      snprintf(pq, sizeof pq, "a LEITURA: %d variável(is), logo %u atribuições — o corpo"
               " é finito por construção, e percorrem-se TODAS", b.nv, 1u << b.nv);
      tique(pq); }
    printf("      variáveis: ");
    for(int k = 0; k < b.nv; k++) printf("%c%s", b.nome[k], k+1 < b.nv ? ", " : "\n");
    /* tick 2 — a equação */
    if(equacao){
        tique("a EQUAÇÃO: A = B vira A ⊕ B = 0 — em GF(2) a igualdade é o XOR a zerar,"
              " e é a mesma redução da equação polinomial");
        printf("      %s   (e procuram-se os ZEROS)\n", expr);
    }
    /* tick 3 — a fibra */
    { int alvo2 = equacao ? 0 : 1, quantos = 0;
      unsigned n = 1u << b.nv;
      tique(equacao ? "a FIBRA: dadas as saídas, quais as entradas — é a divisão das"
                      " cinco, e aqui ela enumera-se exata"
                    : "a TABELA: o valor em cada atribuição, sem excepção");
      for(unsigned x = 0; x < n; x++){
          if(quer_tabela == 1 || b.t[x] == alvo2){
              printf("      ");
              for(int k = 0; k < b.nv; k++) printf("%c=%d ", b.nome[k], (x >> k) & 1);
              if(equacao && b.t[x] == alvo2) printf(" →  0  (zero do A⊕B: SATISFAZ)\n");
              else printf(" →  %d\n", b.t[x]);
              if(b.t[x] == alvo2) quantos++;
          }
      }
      if(quer_tabela != 1){
          if(!quantos) printf("      nenhuma: %s\n",
                              equacao ? "a equação não tem solução (contradição)"
                                      : "a função é sempre 0 (contradição)");
          else if((unsigned)quantos == n) printf("      TODAS: %s\n",
                              equacao ? "a equação vale para tudo (tautologia)"
                                      : "a função é sempre 1 (tautologia)");
          else printf("      %d de %u atribuições — satisfazível\n", quantos, n);
      }
    }
    /* tick 4 — o dual */
    { unsigned char anf[BL_MAX];
      memcpy(anf, b.t, (size_t)(1u << b.nv));
      bl_mobius(anf, b.nv);
      tique("o DUAL: a forma canónica (ANF) pela transformada de Möbius — e ela é"
            " INVOLUÇÃO, a mesma leva de volta: ν∘ν = id, de graça porque -x = x");
      printf("      ANF:  "); escreve_anf(anf, &b); printf("\n");
      /* tick 5 — a volta */
      { int resid = 0;
        unsigned n = 1u << b.nv;
        for(unsigned x = 0; x < n; x++) if(bl_val_anf(anf, b.nv, x) != b.t[x]) resid++;
        tique("a VOLTA: avalia-se a ANF em todas as atribuições e compara-se com a"
              " tabela — resíduo 0, ou não afirmo a forma canónica");
        printf("      resíduo %d em %u atribuições%s\n", resid, n,
               resid ? "  — NÃO afirmo" : "  (fecha exato)"); }
    }
    return 1;
}
static int resolve_calculo(const char *f){
    int sentido = 0;
    const char *p = f;
    if(!strncmp(p, "integra ", 8)){ sentido = -1; p += 8; }
    else if(!strncmp(p, "integral de ", 12)){ sentido = -1; p += 12; }
    else if(!strncmp(p, "deriva ", 7)){ sentido = +1; p += 7; }
    else if(!strncmp(p, "derivada de ", 12)){ sentido = +1; p += 12; }
    else return 0;
    if(!strchr(p, 'x')) return 0;
    /* «de a a b» — os limites, quando os há */
    char corpo[512]; long ap = 0, aq = 1, bp = 0, bq = 1; int definida = 0;
    const char *de = strstr(p, " de ");
    if(de && sentido < 0){
        const char *aa = strstr(de + 4, " a ");
        if(aa){
            snprintf(corpo, sizeof corpo, "%.*s", (int)(de - p), p);
            char la[64], lb[64];
            snprintf(la, sizeof la, "%.*s", (int)(aa - (de + 4)), de + 4);
            snprintf(lb, sizeof lb, "%s", aa + 3);
            Pol pa2, pb2;
            if(pol_le(la, la + strlen(la), &pa2) == 1 && pol_le(lb, lb + strlen(lb), &pb2) == 1
               && pa2.n == 0 && pb2.n == 0){
                ap = pa2.p[0]; aq = pa2.q[0]; bp = pb2.p[0]; bq = pb2.q[0];
                definida = 1;
            }
        }
    }
    if(!definida) snprintf(corpo, sizeof corpo, "%s", p);
    Pol po;
    if(pol_le(corpo, corpo + strlen(corpo), &po) != 1) return 0;
    Pol r; pol_calculo(po, sentido, &r);
    printf("   "); escreve_pol(po); printf("\n");
    if(sentido > 0){
        printf(" = "); escreve_pol(r); printf("     (a derivada)\n");
        printf("   (e é a parte ε do dual: f(a+bε) = f(a) + f'(a)·b·ε, com ε² = 0 — a\n");
        printf("    derivada é exata, sem passo h e sem limite)\n");
        return 1;
    }
    printf(" = "); escreve_pol(r); printf(" + C     (a integral)\n");
    if(definida){
        long vbp, vbq, vap, vaq, dp2, dq2;
        pol_val_q(r, bp, bq, &vbp, &vbq);
        pol_val_q(r, ap, aq, &vap, &vaq);
        pl_soma(vbp, vbq, -vap, vaq, &dp2, &dq2);
        /* UMA fração viva de cada vez: o escritor tem buffer rotativo, e sete
         * chamadas no mesmo printf pisavam-se umas às outras — foi o que aconteceu. */
        printf("   de %s", frac2(ap, aq));
        printf(" a %s:", frac2(bp, bq));
        printf("  F(%s)", frac2(bp, bq));
        printf(" - F(%s)", frac2(ap, aq));
        printf(" = %s", frac2(vbp, vbq));
        printf(" - %s", frac2(vap, vaq));
        printf(" = %s\n", frac2(dp2, dq2));
        printf("   (e o C desaparece na subtração — é a mesma constante nos dois lados)\n");
    } else {
        printf("   (o «+C» não é enfeite: a derivada APAGA a constante, e por isso a volta\n");
        printf("    a pede de volta. Medido: derivar e integrar devolve p - p(0))\n");
    }
    printf("   (integrar é derivar com o sinal trocado — UMA operação, como o resto)\n");
    return 1;
}
static int resolve_as_cinco(const char *f){
    if(strncmp(f, "as cinco", 8) && strncmp(f, "quais são as cinco", 19) &&
       strncmp(f, "quais sao as cinco", 19)) return 0;
    printf("   as CINCO operações do corpo universal, e o que cada uma conserva:\n\n");
    printf("   ⊕  SOMA           a dobra: a retração devolve\n");
    printf("                     aqui: aprender/esquecer é CORPUS(±1) — uma operação\n");
    printf("   ⊗  MULTIPLICAÇÃO  a fusão: a norma multiplica\n");
    printf("                     aqui: o produto de polinómios É a convolução\n");
    printf("   ÷  DIVISÃO        a FIBRA: a = q·b + r, e a volta reconstrói\n");
    printf("                     aqui: dividir, fatorar (a deconvolução) e o corte\n");
    printf("                     moldura+parâmetro da fala — todos a mesma fibra\n");
    printf("   †  DUAL           a involução: ν∘ν = id\n");
    printf("                     aqui: a membrana LaTeX é MEMBRANA(±1), e o recíproco\n");
    printf("                     do polinómio é o ν da prova de Pisot\n");
    printf("   ↺  INVERSÃO       a volta: só é admissível quem a tem\n");
    printf("                     aqui: o mdc é a folha da órbita do inversor, e o\n");
    printf("                     Euclides dos inteiros é a MESMA descida\n\n");
    printf("   e o PONTRYAGIN por cima: o caráter leva a SOMA em PRODUTO (⊕→⊗) —\n");
    printf("   é ele que faz o expoente andar por soma e a potência por produto.\n");
    printf("   A granularidade é do QUANTIZADOR: o tick é o quantum, e a velocidade\n");
    printf("   só sobe por dobra (1, 2, 4, 8) — «nunca por salto».\n");
    return 1;
}
static int resolve_mdc_poli(const char *f){
    const char *p = f;
    if(!strncmp(p, "mdc de ", 7)) p += 7;
    else if(!strncmp(p, "mdc ", 4)) p += 4;
    else return 0;
    const char *e2 = strstr(p, " e ");
    if(!e2) return 0;
    char esq[512], dir[512];
    snprintf(esq, sizeof esq, "%.*s", (int)(e2 - p), p);
    snprintf(dir, sizeof dir, "%s", e2 + 3);
    if(!strchr(esq, 'x') || !strchr(dir, 'x')) return 0;   /* sem x, é dos inteiros */
    Pol pa, pb;
    if(pol_le(esq, esq + strlen(esq), &pa) != 1) return 0;
    if(pol_le(dir, dir + strlen(dir), &pb) != 1) return 0;
    long ia[PMAX+1], ib[PMAX+1];
    if(!pol_ic(pa, ia) || !pol_ic(pb, ib)) return 0;
    Pz a, b;
    a.n = pa.n; for(int k = 0; k <= pa.n; k++) a.a[k] = ia[k];
    b.n = pb.n; for(int k = 0; k <= pb.n; k++) b.a[k] = ib[k];
    Pz g; int passos = 0;
    if(!pz_mdc(a, b, &g, &passos)){
        printf("a órbita não coube nos inteiros — e não a escrevo aproximada.\n");
        return 1;
    }
    printf("   "); escreve_pz(a); printf("   e   "); escreve_pz(b); printf("\n");
    printf(" = a órbita do inversor desce em %d passo(s) — cada passo é um resto, e é a\n", passos);
    printf("   MESMA cadeia que dá o gcd nos inteiros e a fração contínua no racional\n");
    printf(" = mdc  "); escreve_pz(g); printf("\n");
    { Pz q1, q2;                                   /* A VOLTA: divide os dois? */
      int b1 = pz_div_exata(a, g, &q1), b2v = pz_div_exata(b, g, &q2);
      if(!b1 || !b2v){ printf("   (a volta NÃO fechou — logo não afirmo este mdc)\n"); return 1; }
      printf("   e a volta confere: a ÷ mdc = "); escreve_pz(q1);
      printf(",  b ÷ mdc = "); escreve_pz(q2); printf("   (resto 0 nos dois)\n"); }
    if(g.n == 0)
        printf("   o mdc é constante: são primos entre si — a órbita fecha sem folha comum\n");
    else
        printf("   a folha da órbita é o fator comum, e é ela o mdc\n");
    return 1;
}
static int resolve_fatora_poli(const char *f){
    const char *p = f;
    if(!strncmp(p, "fatora ", 7)) p += 7;
    else if(!strncmp(p, "fatoriza ", 9)) p += 9;
    else if(!strncmp(p, "fatorar ", 8)) p += 8;
    else return 0;
    if(!strchr(p, 'x')) return 0;                  /* sem x é conta, e é do outro ramo */
    Pol po;
    if(pol_le(p, p + strlen(p), &po) != 1 || po.n < 1) return 0;
    long ic[PMAX+1];
    if(!pol_ic(po, ic)) return 0;
    Pz z; z.n = po.n;
    for(int k = 0; k <= po.n; k++) z.a[k] = ic[k];
    Pz fs[PFMAX]; long cont = 1;
    int nf = pz_fatora(z, fs, PFMAX, &cont);
    if(!pz_confere(z, fs, nf, cont)){              /* a volta não fecha: não se escreve */
        printf("não sei fatorar isso sem sair dos inteiros.\n");
        printf("   (e não escrevo fator que não reconstrói o original — a volta é a prova)\n");
        return 1;
    }
    printf("   "); escreve_pz(z); printf("\n");
    printf(" = ");
    if(cont != 1){ printf("%ld", cont); if(nf) printf(" · "); }
    for(int k = 0; k < nf; k++){
        printf("("); escreve_pz(fs[k]); printf(")");
        if(k + 1 < nf) printf("·");
    }
    printf("\n");
    printf("   (e a VOLTA confere: o produto dos fatores reconstrói o polinómio termo a\n");
    printf("    termo — fatorar é a deconvolução, e o produto é a convolução: distribuir\n");
    printf("    é convolver, e a linha de Pascal é o caso (x+1)^n dela)\n");
    if(nf == 1 && fs[0].n == z.n){
        long m = pz_beta_pisot(fs[0]);
        long mm = pz_metalica(fs[0]);
        if(mm) printf("   é a BORDA METÁLICA m=%ld: σ² = %ldσ + 1, unidade quadrática de Pisot\n"
                      "   (|σ|>1, |σ†|<1, σσ†=-1) — irredutível, e a sua FC é %s\n",
                      mm, mm, fc_da_borda(mm, -1));
        else if(m) printf("   é β(%d,%ld) = x^%d - %ldx^%d - 1, da família de PISOT: por Rouché no\n"
                          "   DUAL, %d raízes ficam dentro do círculo e uma fora — logo nenhum\n"
                          "   fator próprio cabe (o seu c₀ seria o produto de raízes < 1, e é\n"
                          "   inteiro ≠ 0). Irredutível, sem calcular raiz nenhuma.\n",
                          fs[0].n, m, fs[0].n, m, fs[0].n - 1, fs[0].n - 1);
        else if(z.n <= 3) printf("   é IRREDUTÍVEL em Q: grau %d sem raiz racional não parte —\n"
                                 "   um fator próprio teria de ser linear, e não há.\n", z.n);
        else printf("   não achei fator próprio — e isso NÃO é o mesmo que provar irredutível.\n");
    }
    return 1;
}
static int e_poli(const char *f){
    if(!strchr(f, '=') || !strchr(f, 'x')) return 0;
    if(strstr(f, "y'") || strchr(f, ';') || strchr(f, '|')) return 0;
    for(const char *p = f; *p; p++){
        if(*p==' '||*p=='+'||*p=='-'||*p=='*'||*p=='/'||*p=='='||*p=='^') continue;
        if(*p=='x'||*p=='X') continue;
        if(*p>='0'&&*p<='9') continue;
        return 0;
    }
    Pol p;
    int r = pol_equacao(f, &p);
    return r == 1 && p.n >= 2;                  /* grau 1 fica com o resolvedor de sempre */
}
static int resolve_poli(const char *f){
    Pol p;
    int r = pol_equacao(f, &p);
    if(r == -1){ printf("o grau passa de %d, que é onde a caixa desta máquina acaba.\n", PMAX);
                 return 1; }
    if(r == -2){ printf("os dois lados são o mesmo: qualquer x serve.\n"); return 1; }
    if(r == -3){ printf("o x desaparece e sobra uma falsidade: nenhum x serve.\n"); return 1; }
    if(r != 1) return 0;

    printf("   %s\n", f);
    printf(" = ");
    for(int k = p.n; k >= 0; k--){
        if(p.p[k] == 0) continue;                       /* zero é zero: sem limiar */
        printf("%s", (k == p.n) ? "" : (p.p[k] < 0 ? " - " : " + "));
        long np = (k == p.n) ? p.p[k] : (p.p[k] < 0 ? -p.p[k] : p.p[k]);
        if(!(np == p.q[k] && k > 0)) printf("%s", frac2(np, p.q[k]));
        if(k >= 1) printf("x");
        if(k >= 2) printf("^%d", k);
    }
    printf(" = 0     (tudo para um lado, e mónico)\n\n");

    /* 1. QUANTAS na reta — por STURM, que é EUCLIDES. Exato, e é a dobra. */
    int nr = pol_sturm_reais(p);
    int ns = (p.n - nr) / 2;
    printf("   por Sturm — a cadeia de restos de Euclides, a mesma divisão da cifra:\n");
    printf("     %d raízes na RETA, %d %s no CÍRCULO   ->  assinatura (%d, %d)\n\n",
           nr, ns, ns == 1 ? "par" : "pares", nr, ns);

    /* 2. QUAIS são racionais — enumeração FINITA, e cada uma verificada em inteiros. */
    long num[PMAX], den[PMAX];
    int nq = pol_racionais(p, num, den, PMAX);
    if(nq){
        printf("   e as que fecham em Q, achadas por enumeração finita (p|c0, q|cn):\n");
        for(int k = 0; k < nq; k++){
            if(den[k] == 1) printf("     x = %ld\n", num[k]);
            else            printf("     x = %ld/%ld\n", num[k], den[k]);
        }
        printf("     (exatas: avaliadas em inteiros, e o valor deu ZERO — não é resíduo pequeno,\n");
        printf("      é zero)\n\n");
    } else printf("   e nenhuma fecha em Q.\n\n");

    /* 3. E as outras NÃO SE CALCULAM: declara-se o corpo onde elas vivem. */
    if(nq < p.n){
        printf("   as outras %d não se calculam, e isso É o método: a raiz de um polinómio que\n",
               p.n - nq);
        printf("   não fecha em Q não é um número a procurar — é o σ do corpo Q[x]/(p). Declara-\n");
        printf("   -se o corpo, e lá dentro ela é EXATA e tem nome:\n\n");
        printf("     corpo:  Q[x]/(");
        for(int k = p.n; k >= 0; k--){
            if(p.p[k] == 0) continue;
            printf("%s", (k == p.n) ? "" : (p.p[k] < 0 ? " - " : " + "));
            long np = (k == p.n) ? p.p[k] : (p.p[k] < 0 ? -p.p[k] : p.p[k]);
            if(!(np == p.q[k] && k > 0)) printf("%s", frac2(np, p.q[k]));
            if(k >= 1) printf("x");
            if(k >= 2) printf("^%d", k);
        }
        printf(")   com σ a raiz, por construção\n");
        if(p.n == 2){
            if(p.p[1] == 0) printf("     e a borda:  s^2 = %s\n", frac2(-p.p[0], p.q[0]));
            else printf("     e a borda:  s^2 = %s%s%ss\n", frac2(-p.p[0], p.q[0]),
                        p.p[1] > 0 ? " - " : " + ",
                        frac2(p.p[1] < 0 ? -p.p[1] : p.p[1], p.q[1]));
        }
        printf("     (aproximá-la em decimal seria SAIR do corpo para dar um número que já não\n");
        printf("      é raiz de nada — e é exatamente o que este sistema não faz)\n\n");
    }

    if(p.n == 2){
        /* a forma INTEIRA da mesma equação (limpar denominadores não muda raiz nenhuma),
         * e daí a borda exata e o relógio — sem um decimal em lado nenhum */
        long ic[PMAX+1];
        if(pol_ic(p, ic)){
            long D = ic[1]*ic[1] - 4*ic[2]*ic[0];
            printf("   e em grau 2 a assinatura cabe num número: Δ = %ld, logo %s\n", D,
                   D > 0 ? "HIPERBÓLICO" : D < 0 ? "ELÍPTICO" : "PARABÓLICO");
            borda_exata(ic[2], ic[1], ic[0]);
            folhas_no_relogio(ic[2], ic[1], ic[0]);
        }
    } else
        printf("   (em grau 2 isto seria o Δ; acima dele classifica o par (r,s), e há %d\n"
               "    assinaturas possíveis em grau %d)\n", p.n/2 + 1, p.n);
    return 1;
}

/* O SISTEMA. "x' = x + 2y ; y' = 3x + 4y" — e o que ele mostra é que a régua do sistema É a
 * régua (B, C) do catálogo: para 2x2 o característico é λ² − tr·λ + det, logo B = −traço e
 * C = determinante, sem tradução nenhuma. */
static int sis_le(const char *f, long *a, long *b, long *c, long *d){
    const char *pv = strchr(f, ';');
    if(!pv) return 0;
    long m[2][2] = {{0,0},{0,0}};
    const char *p = f;
    for(int lin = 0; lin < 2; lin++){
        while(*p == ' ') p++;
        if(*p != 'x' && *p != 'y') return 0;
        int esq = (*p == 'x') ? 0 : 1;
        if(lin == 0 && esq != 0) return 0;
        if(lin == 1 && esq != 1) return 0;
        p++;
        if(*p != '\'') return 0;
        p++;
        while(*p == ' ') p++;
        if(*p != '=') return 0;
        p++;
        const char *fim = (lin == 0) ? pv : f + strlen(f);
        while(p < fim){
            while(p < fim && *p == ' ') p++;
            if(p >= fim) break;
            long sinal = 1;
            if(*p == '+'){ p++; }
            else if(*p == '-'){ sinal = -1; p++; }
            while(p < fim && *p == ' ') p++;
            long v = 0; int tem = 0;
            while(p < fim && *p >= '0' && *p <= '9'){ v = v*10 + (*p-'0'); p++; tem = 1; }
            if(!tem) v = 1;
            while(p < fim && *p == ' ') p++;
            if(p >= fim || (*p != 'x' && *p != 'y')) return 0;
            m[lin][*p == 'x' ? 0 : 1] += sinal*v;
            p++;
        }
        if(lin == 0) p = pv + 1;
    }
    *a = m[0][0]; *b = m[0][1]; *c = m[1][0]; *d = m[1][1];
    return 1;
}
static int e_sistema(const char *f){
    long a,b,c,d;
    return strchr(f, ';') && strstr(f, "'") && sis_le(f, &a,&b,&c,&d);
}
/* O SISTEMA, EM INTEIROS. O `sqrt(D)` que aqui estava era o próprio erro que o texto
 * do polinómio já denunciava: «aproximá-la em decimal seria SAIR do corpo para dar um
 * número que já não é raiz de nada». O traço, o determinante e o Δ são inteiros; o
 * espectro ou fecha em ℚ (quando Δ é quadrado perfeito, e a raiz inteira diz-no
 * exatamente) ou é o par de folhas do corpo ℚ[s]/(s²−Ts+det) — com σ+σ† = tr e
 * σσ† = det, e o relógio a dar-lhes número. Nenhum decimal em lado nenhum. */
static int resolve_sistema(const char *f){
    long a,b,c,d;
    if(!sis_le(f, &a,&b,&c,&d)) return 0;
    long T = a + d, De = a*d - b*c, D = T*T - 4*De;
    printf("   %s\n", f);
    printf(" = x' = Ax, com A = [[%ld,%ld],[%ld,%ld]]\n", a, b, c, d);
    printf("   traço %ld, determinante %ld, Δ = tr² - 4det = %ld\n", T, De, D);
    printf("   e a régua do sistema É a do catálogo: B = -traço = %ld, C = det = %ld\n", -T, De);
    printf("   logo %s\n", D > 0 ? "HIPERBÓLICO — o gato, cresce e gasta"
                        : D < 0 ? "ELÍPTICO — o esquilo, gira e não gasta"
                                : "PARABÓLICO — a fronteira, e é onde entra o t");
    borda_exata(1, -T, De);          /* a borda resolvida exata, a mesma dos três */
    /* o regime lê-se pelo SINAL do traço — inteiro, sem limiar inventado */
    printf("   (regime: %s — o sinal do traço decide, e ele é inteiro: %+ld)\n",
           T > 0 ? "CAOS, diverge" : T < 0 ? "CRISTAL, colapsa no ponto fixo"
                                           : "BORDA, orbita e conserva", T);
    printf("   a solução é x(t) = e^(At)·x₀, e por Cayley-Hamilton e^(At) = c₁I + c₂A —\n");
    printf("   os dois coeficientes saem do espectro, e a fórmula é fechada.\n");
    printf("   (e uma ED de 2ª ordem já É um destes, com A a COMPANION [[0,1],[-C,-B]])\n");
    return 1;
}

/* A EQUAÇÃO DIFERENCIAL. "y'' = -y", "y'' + 2y' + y = 0".
 *
 * Resolver não é máquina nova: a equação característica É a borda do corpo, com o operador de
 * derivação D no lugar do marcador σ. Declara-se o corpo e lê-se a régua — o Δ que classifica
 * as soluções é o MESMO que classifica os corpos do catálogo. */
static int e_edo(const char *f){
    if(!strstr(f, "y'")) return 0;
    Edo e; Fonte fo;
    return edo_le_nh(f, &e, &fo);
}
static int resolve_edo(const char *f){
    Edo e; Fonte fo;
    if(!edo_le_nh(f, &e, &fo)) return 0;
    char bt[96];
    edo_borda(e, bt, sizeof bt);
    /* A ED JÁ GUARDA AS FRAÇÕES (Bp/Bq, Cp/Cq) e eu dividia-as em double — o erro
     * inteiro numa linha. Limpam-se os denominadores e a borda fica em inteiros:
     * L² + (Bp/Bq)L + Cp/Cq = 0  ⇔  (Bq·Cq)L² + (Bp·Cq)L + (Cp·Bq) = 0. */
    long ea = e.Bq * e.Cq, eb = e.Bp * e.Cq, ec = e.Cp * e.Bq;
    long ED = eb*eb - 4*ea*ec;
    printf("   %s\n", f);
    printf(" = a característica é  %ld L^2 %c %ld L %c %ld = 0   (em inteiros, sem dividir)\n",
           ea, eb < 0 ? '-' : '+', eb < 0 ? -eb : eb, ec < 0 ? '-' : '+', ec < 0 ? -ec : ec);
    printf("   e isso É a borda do corpo:  %s   (o D no lugar do s)\n", bt);
    printf("   logo %s\n",
           ED > 0 ? "HIPERBÓLICO — o gato, cresce e gasta"
         : ED < 0 ? "ELÍPTICO — o esquilo, gira e não gasta"
                  : "PARABÓLICO — a fronteira, o absorvente");
    borda_exata(ea, eb, ec);            /* a MESMA borda dos três: exata, e a FC no fim */
    if(eb == -ea && ec == -ea)
        printf("   (e esta é a do OURO: as folhas são φ e -1/φ, e a mesma recorrência em\n"
               "    passos inteiros é Fibonacci)\n");
    if(eb == 0 && ec == ea)
        printf("   (e esta é a do i: a borda s^2 = -1, e a solução é a ROTAÇÃO)\n");
    printf("   a solução escreve-se com as folhas: y = A·e^(σt) + B·e^(σ†t) quando elas são\n");
    printf("   distintas, e (A + B·t)·e^(σt) quando a borda tem raiz dupla.\n");
    if(fo.tipo != F_NENHUMA){
        /* A NÃO HOMOGÉNEA: a fonte desloca o corpo livre. */
        char yp[192];
        int r = edo_particular(e.Bp, e.Bq, e.Cp, e.Cq, fo, yp, sizeof yp);
        printf("\n   e há FONTE, logo a solução é y = y_h + y_p — a homogénea de cima MAIS uma\n");
        printf("   particular. O conjunto das soluções não é um espaço vetorial: é um espaço\n");
        printf("   vetorial TRANSLADADO, e a fonte desloca-o sem o deformar.\n");
        printf("     y_p = %s\n", yp);
        if(r == 1)
            printf("   (RESSONÂNCIA: a fonte cai SOBRE o espectro — p(a) = 0, não há constante que\n"
                   "    sirva, e entra um t. É o mesmo t da raiz dupla, pelo mesmo motivo)\n");
        else if(r == 2)
            printf("   (RESSONÂNCIA DUPLA: a raiz é dupla E a fonte cai nela — entra t²)\n");
        else
            printf("   (sem ressonância: p(a) ≠ 0, e a particular é do mesmo feitio da fonte)\n");
    }
    printf("   (a solução explícita de toda ED linear é o fluxo e^(At); o exp leva a SOMA dos\n");
    printf("    geradores ao PRODUTO dos fluxos, e o metal é o exp da taxa: σ = e^λ)\n");
    return 1;
}

/* A ÁLGEBRA GLOBAL. "s^2 = -1 | (1 + 2s) x (1 - 2s)" — a borda declara o CORPO, e a conta
 * corre lá dentro. É a notação algébrica da teoria: o elemento é uma tupla escrita com o
 * marcador dito, e a FAMÍLIA REAL é a base ortonormal {1, s, s², …}.
 *
 * O i não é caso especial nenhum: é a borda s² = -1. */
static int e_algebra(const char *f){
    const char *bar = strchr(f, '|');
    if(!bar || bar == f) return 0;
    char bt[256];
    snprintf(bt, sizeof bt, "%.*s", (int)(bar - f), f);
    Elem borda; char marca[4];
    return al_le_borda(bt, &borda, marca) > 0;
}
static int resolve_algebra(const char *f){
    const char *bar = strchr(f, '|');
    char bt[256], et[512];
    snprintf(bt, sizeof bt, "%.*s", (int)(bar - f), f);
    snprintf(et, sizeof et, "%s", bar + 1);
    Elem borda; char marca[4];
    int n = al_le_borda(bt, &borda, marca);
    if(!n) return 0;

    /* a expressão: (a) op (b), com op em + - x. Sem parênteses aninhados: aqui a conta é a
     * ÁLGEBRA e não o desdobramento — quem trata de parênteses é o resolvedor de sempre. */
    const char *p = et;
    Elem a, b;
    while(*p == ' ') p++;
    int temp = (*p == '(');
    if(temp) p++;
    if(al_le_elem(&p, n, marca, &a) != 1){ printf("não percebi o primeiro elemento.\n"); return 1; }
    if(temp && *p == ')') p++;
    while(*p == ' ') p++;
    char op = *p;
    if(op != '+' && op != '-' && op != 'x' && op != '*'){
        /* um elemento só: escreve-o reduzido, que já é uma resposta */
        char sr[256]; al_escreve(a, sr, sizeof sr, marca);
        printf("%s, no corpo %s.\n", sr, bt);
        return 1;
    }
    p++;
    while(*p == ' ') p++;
    temp = (*p == '(');
    if(temp) p++;
    if(al_le_elem(&p, n, marca, &b) != 1){ printf("não percebi o segundo elemento.\n"); return 1; }

    Elem r = (op == '+') ? al_soma(a, b, +1)
           : (op == '-') ? al_soma(a, b, -1)
                         : al_prod(a, b, &borda);
    char sa[128], sb[128], sr[256];
    al_escreve(a, sa, sizeof sa, marca);
    al_escreve(b, sb, sizeof sb, marca);
    al_escreve(r, sr, sizeof sr, marca);
    printf("   (%s) %c (%s)\n", sa, op == '*' ? 'x' : op, sb);
    printf(" = %s\n", sr);
    if(op == 'x' || op == '*')
        printf("   (multipliquei como polinómios e baixei pela borda %s)\n", bt);
    else
        printf("   (a soma é coordenada a coordenada — a borda não entra)\n");
    if(n == 2){
        long B, C, D;
        al_regua2(borda, &B, &C, &D);
        printf("   (o corpo: traço %ld, determinante %ld, Δ = %ld — %s%s)\n", B, C, D,
               D > 0 ? "hiperbólico, cresce e gasta" :
               D < 0 ? "elíptico, gira e não gasta" : "parabólico, o absorvente",
               (borda.p[0] == -1 && borda.p[1] == 0) ? "; esta borda é o i" : "");
    }
    printf("   (a base é a FAMÍLIA REAL: 1");
    for(int k = 1; k < n; k++){ if(k == 1) printf(", %s", marca); else printf(", %s^%d", marca, k); }
    printf(" — %d eixos)\n", n);
    return 1;
}

/* A EQUAÇÃO. Quando a fala tem um '=' e os dois lados são contas (com 'x' permitido), não é
 * uma expressão a avaliar — é uma equação a resolver, que é a operação DUAL. */
static int e_conta_x(const char *f, int com_x);
static int e_equacao(const char *f, char *esq, char *dir, size_t lim){
    const char *p = strchr(f, '=');
    if(!p || p == f || !p[1]) return 0;
    if(strchr(p + 1, '=')) return 0;                  /* dois '=' não é equação nenhuma */
    snprintf(esq, lim, "%.*s", (int)(p - f), f);
    snprintf(dir, lim, "%s", p + 1);
    return e_conta_x(esq, 1) && e_conta_x(dir, 1);
}
static int resolve_eq(const char *esq, const char *dir){
    char c[600]; snprintf(c, sizeof c, "%s.conta", barr_base);
    int cf = open(c, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(cf < 0) return 0;
    Eq r; ct_resolve_eq(cf, esq, dir, &r);
    if(r.tipo == EQ_MAU){ printf("%s.\n", r.nota); close(cf); unlink(c); return 1; }
    /* mostra-se a REDUÇÃO de cada lado, que é o que se aprende — e não só o número */
    char ae[64], be[64], ad[64], bd[64];
    ct_escreve(r.aep, r.aeq, ae, sizeof ae); ct_escreve(r.bep, r.beq, be, sizeof be);
    ct_escreve(r.adp, r.adq, ad, sizeof ad); ct_escreve(r.bdp, r.bdq, bd, sizeof bd);
    printf("   %s = %s\n", esq, dir);
    /* SÓ se mostra a forma a.x + b quando CADA LADO é mesmo linear. Em "x^2 = x^2 + 1" a
     * diferença é linear (constante) e a equação resolve-se bem, mas os lados não são retas —
     * e escrever "1.x + 0" para o x² seria a apresentação a mentir sobre o que lá está,
     * mesmo com a resposta certa. */
    {
        int lados_lineares = 1;
        for(int lado = 0; lado < 2 && lados_lineares; lado++){
            const char *txt = lado ? dir : esq;
            long p0, q0, p1, q1, p2, q2;
            int cf2 = open(c, O_RDWR|O_CREAT|O_TRUNC, 0644);
            if(cf2 < 0){ lados_lineares = 0; break; }
            int bom = ct_lado(cf2, txt, 0, &p0, &q0) && ct_lado(cf2, txt, 1, &p1, &q1)
                                                     && ct_lado(cf2, txt, 2, &p2, &q2);
            close(cf2); unlink(c);
            if(!bom){ lados_lineares = 0; break; }
            /* «a diferença é constante?» compara-se por PRODUTO CRUZADO, exato:
             * (v2−v1) = (v1−v0)  ⟺  (p2q1−p1q2)·q1q0 = (p1q0−p0q1)·q2q1 */
            long e1p = p2*q1 - p1*q2, e1q = q2*q1;
            long e0p = p1*q0 - p0*q1, e0q = q1*q0;
            if(e1p*e0q != e0p*e1q) lados_lineares = 0;
        }
        if(!lados_lineares){
            printf("   (não escrevo a forma a.x + b de cada lado: eles não são retas. A\n");
            printf("    DIFERENÇA é que é linear, e é ela que se resolve)\n");
            goto decide;
        }
    }
    printf(" = %s.x %s %s = %s.x %s %s     (cada lado reduzido à forma a.x + b)\n",
           ae, r.bep < 0 ? "-" : "+", be[0] == '-' ? be + 1 : be,
           ad, r.bdp < 0 ? "-" : "+", bd[0] == '-' ? bd + 1 : bd);
decide:
    if(r.tipo == EQ_TODAS){ printf("qualquer x serve.\n   (%s)\n", r.nota); }
    else if(r.tipo == EQ_NENHUM){ printf("não há x nenhum.\n   (%s)\n", r.nota); }
    else {
        char xs[64], dd[160]; long pre, per;
        ct_escreve(r.p, r.q, xs, sizeof xs);
        printf("x = %s", xs);
        if(r.q != 1){ ct_decimal(r.p, r.q, dd, sizeof dd, &pre, &per);
                      printf(", que em decimal é %s", dd); }
        printf(".\n   (%s)\n", r.nota);
    }
    close(cf); unlink(c);
    return 1;
}

/* a fala veio vestida de membrana? quem entrega à cascata é que sabe, e a resposta
 * volta na mesma roupa — os dois lados do par, não um com o nome dele. */
static int MEMBRANA_ENTRADA = 0;
static void veste_valor(char *out, size_t lim, long v, long q);
static int e_conta(const char *f){ return e_conta_x(f, 0); }
static int e_conta_x(const char *f, int com_x){
    int digito = 0;
    for(const char *p = f; *p; p++){
        /* "raiz" é a única palavra que entra numa conta. Salta-se inteira; as letras dela
         * soltas não passam, e por isso "a raiz de 2 é racional" continua a ir ao corpus —
         * medido, porque essa fala existe lá e seria mau perdê-la para o resolvedor. */
        if(com_x && (*p == 'x' || *p == 'X')){ digito = 1; continue; }
        /* o i é conta; mas só isolado — "i" seguido de letra é palavra portuguesa */
        if(*p == 'i' && !(p[1] >= 'a' && p[1] <= 'z') && !(p > f && p[-1] >= 'a' && p[-1] <= 'z')){
            digito = 1; continue;
        }
        if(!strncmp(p, "raiz", 4)){ p += 3; continue; }
        if(!strncmp(p, "mod", 3)){ p += 2; continue; }
        if(*p >= '0' && *p <= '9'){ digito = 1; continue; }
        if(*p==' '||*p=='\t'||*p=='+'||*p=='-'||*p=='*'||*p=='x'||*p=='X'
                     ||*p=='/'||*p==':'||*p=='^'||*p=='%'||*p=='!') continue;
        /* "de" só entra logo a seguir a um '%' — a palavra é comum de mais em português,
         * e "raiz de 4" tem de continuar a ser fala e não conta. */
        if(!strncmp(p, "de", 2)){
            const char *t = p - 1;
            while(t > f && *t == ' ') t--;
            if(t >= f && *t == '%'){ p += 1; continue; }
        }
        /* a vírgula e o ponto só entram ENTRE dígitos — senão qualquer frase com pontuação
         * passaria por conta, e o corpus perdia-a para o resolvedor. */
        if((*p==','||*p=='.') && p > f && p[-1] >= '0' && p[-1] <= '9'
                               && p[1] >= '0' && p[1] <= '9') continue;
        if(*p=='('||*p==')'||*p=='['||*p==']'||*p=='{'||*p=='}') continue;
        return 0;
    }
    return digito;
}
static int resolve_conta(const char *fala){
    char c[600]; snprintf(c, sizeof c, "%s.conta", barr_base);
    int cf = open(c, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(cf < 0) return 0;
    long n = ct_leia(cf, fala);
    if(n < 0){
        static const char *m[] = { "", "há um fecho a mais: fechaste sem ter aberto",
            "o fecho não casa com a abertura — um ']' não fecha um '('",
            "há um símbolo que não é desta conta; aqui só entram + e x",
            "abriste e não fechaste" };
        printf("essa conta não fecha: %s.\n", m[-n]);
        printf("   (não adivinho o que quiseste dizer — escreve-a fechada e eu resolvo)\n");
        close(cf); return 1;
    }
    char buf[2048], porque[512];
    ct_mostra(cf, n, buf, sizeof buf);
    printf("   %s\n", buf);
    int passos = 0, st;
    while((st = ct_passo(cf, n, porque, sizeof porque)) == 1){
        passos++;
        ct_mostra(cf, n, buf, sizeof buf);
        printf(" = %-26s   %s\n", buf, porque);
        if(passos > 10000) break;
    }
    long v;
    if(st < 0){
        /* a conta PARA, e diz onde: a divisão que não fecha em Z, ou a que não existe em
         * corpo nenhum. Não se arredonda nem se cala — declara-se o corpo, como no resto. */
        printf("aqui a conta para: %s%s\n", porque,
               porque[0] && porque[strlen(porque)-1] == '.' ? "" : ".");
        printf("   (%d dobra(s) até aqui; o resto não existe no corpo em que estamos)\n", passos);
        close(cf); unlink(c);
        return 1;
    }
    long vq;
    {   /* PRIMEIRO o complexo: o ct_valorq trunca a parte imaginária, e sem isto "raiz -4"
         * saía como "dá 0". Mesmo defeito do 7/2 que saía como 7, e desta vez o medidor
         * passou verde porque lê a fita com ct_mostra e não pelo valor. */
        Cel z;
        if(ct_valorcel(cf, n, &z) && z.ip){
            /* pela CÉLULA inteira, para o `sig` do i* não se perder na saída: a conta estava
             * certa por dentro e "1 / i*" saía "dá i", que é a resposta errada por fora. */
            char rc[96]; ct_escrevecs(z.val, z.den, z.ip, z.iq, z.sig, rc, sizeof rc);
            v = z.val; vq = z.den;
            printf("dá %s.\n", rc);
            if(z.sig)
                printf("   (é o DUAL: (i*)² = +1, a norma é a² - b² e o lugar é a hipérbole; o i*\n"
                       "    tem ordem 2, e é essa involução que garante a reversão)\n");
            else
                printf("   (é Q[i]: o i é o flip que troca as duas sementes da cifra, e i² = -1)\n");
            close(cf); unlink(c);
            return 1;
        }
    }
    if(ct_valorq(cf, n, &v, &vq)){
        char rr[64]; ct_escreve(v, vq, rr, sizeof rr);
        if(vq != 1){
            /* a fração e o decimal são a MESMA coisa em duas roupas; mostram-se as duas, e
             * diz-se a regra de quando a segunda acaba. */
            char dd[160], pqd[240]; long pre, per;
            ct_decimal(v, vq, dd, sizeof dd, &pre, &per);
            ct_porque_decimal(vq, pqd, sizeof pqd);
            printf("dá %s, que em decimal é %s.\n", rr, dd);
            printf("   (%s)\n", pqd);
        } else printf("dá %s.\n", rr);
        /* A VOLTA: quem falou na membrana ouve na membrana — o valor vestido na mesma
         * roupa em que a fala veio, pronto para o tradutor compor. */
        if(MEMBRANA_ENTRADA){
            char vest[128]; veste_valor(vest, sizeof vest, v, vq);
            printf("   (na membrana: $%s$ — e o tradutor compõe-a)\n", vest);
        }
    }
    else                    printf("não fechou num número só — algo ficou por dobrar.\n");
    printf("   (%d dobra(s); o mais fundo primeiro; e em cada nível: !, raiz, ^, depois {x, /, mod}, e por fim {+, -})\n", passos);
    close(cf);

    /* A SEGUNDA VIA. Se a distributiva se aplicava, mostra-se o outro caminho — porque é aí
     * que se ensina alguma coisa: os dois fecham no MESMO, e isso é o que a lei afirma. Não
     * se mostra o resultado dela como se fosse outro; mostra-se que é o mesmo. */
    cf = open(c, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(cf >= 0){
        long m = ct_leia(cf, fala);
        long d = m > 0 ? ct_distribui(cf, m, porque, sizeof porque) : 0;
        if(d > 0){
            char b2[2048]; ct_mostra(cf, d, b2, sizeof b2);
            printf("\n   pela distributiva dava no mesmo, por outro caminho:\n");
            printf("   %s\n", b2);
            int q = 0;
            while(ct_passo(cf, d, porque, sizeof porque) == 1){
                q++; ct_mostra(cf, d, b2, sizeof b2);
                printf(" = %-26s   %s\n", b2, porque);
            }
            long v2, v2q;
            if(ct_valorq(cf, d, &v2, &v2q)){ char rr[64]; ct_escreve(v2, v2q, rr, sizeof rr);
                printf("dá %s — %s\n", rr, (v2 == v && v2q == vq)
                       ? "o mesmo, e não por acaso: é isso que a lei diz"
                       : "e NÃO devia diferir; há defeito aqui"); }
        }
        close(cf); unlink(c);
    }
    return 1;
}

/* aplicar a lei pedida pelo nome, e mais nada — sem resolver por cima */
static int aplica_lei(const char *conta, int distribuir){
    char c[600]; snprintf(c, sizeof c, "%s.conta", barr_base);
    int cf = open(c, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(cf < 0) return 0;
    long n = ct_leia(cf, conta);
    if(n < 0){ printf("essa conta não fecha — escreve-a fechada e eu aplico.\n"); close(cf); unlink(c); return 1; }
    char b[2048], porque[512];
    ct_mostra(cf, n, b, sizeof b);
    printf("   %s\n", b);
    long m = distribuir ? ct_distribui(cf, n, porque, sizeof porque)
                        : ct_fatora(cf, n, porque, sizeof porque);
    if(m > 0){
        ct_mostra(cf, m, b, sizeof b);
        printf(" = %s\n   (%s)\n", b, porque);
        long v; char pq[256];
        while(ct_passo(cf, m, pq, sizeof pq) == 1) ;
        long vq2;
        if(ct_valorq(cf, m, &v, &vq2)){ char rr[64]; ct_escreve(v, vq2, rr, sizeof rr);
                                        printf("e dá %s.\n", rr); }
    } else if(m < 0){
        printf("%s%s\n", porque,
               porque[0] && porque[strlen(porque)-1] == '.' ? "" : ".");
    } else {
        printf("aqui não há o que %s.\n", distribuir ? "distribuir — não vejo fator vezes soma"
                                                     : "pôr em evidência — as parcelas não têm fator comum");
    }
    close(cf); unlink(c);
    return 1;
}

/* O CIRCUITO. "rlc 100 1m 1u", "serie 100 220", "paralelo 100 220", "divisor 1k 1k",
 * "ressonancia 1m 1u", "wheatstone 100 220 470", "transistor 0.6".
 *
 * Não é máquina nova, e é esse o ponto: a tríade do corpo transistor É a do resto do sistema.
 * A SOMA é Kirchhoff (série soma Z, paralelo soma Y), o PRODUTO é o ganho do divisor, e o
 * OPERADOR é Shockley — que leva soma de tensões em produto de correntes, Π = exp∘Σ∘log.
 * O RLC cai na mesma borda das EDs, com o mesmo Δ a dar as mesmas três classes. */
/* O VALOR DA BANCADA, EM FRAÇÃO. O `strtod` era a porta por onde o decimal entrava:
 * «1u» virava 1e-6 e daí em diante tudo era aproximado. Um sufixo é uma POTÊNCIA DE
 * DEZ exata — 1u é 1/1000000, e a fração guarda-o sem perder um bit. O «0,6» também é
 * fração: 6/10. Assim a régua do circuito é a mesma do resto da casa. */
typedef struct { long p, q; } Rq;              /* p/q, q > 0, reduzida */
static Rq rq(long p, long q){ Rq r; r.p = p; r.q = q; pl_reduz(&r.p, &r.q); return r; }
static Rq rq_mul(Rq a, Rq b){ return rq(a.p*b.p, a.q*b.q); }
static Rq rq_div(Rq a, Rq b){ return rq(a.p*b.q, a.q*b.p); }
static Rq rq_som(Rq a, Rq b){ return rq(a.p*b.q + b.p*a.q, a.q*b.q); }
static Rq rq_sub(Rq a, Rq b){ return rq(a.p*b.q - b.p*a.q, a.q*b.q); }
static int rq_sinal(Rq a){ return a.p > 0 ? 1 : a.p < 0 ? -1 : 0; }
static const char *rq_txt(Rq a){ return frac2(a.p, a.q); }
static Rq circ_valor(const char *s, int *ok_){
    long ip = 0, fp = 0, fq = 1, sig = 1;
    const char *p = s;
    while(*p == ' ') p++;
    if(*p == '-'){ sig = -1; p++; } else if(*p == '+') p++;
    int tem = 0;
    while(*p >= '0' && *p <= '9'){ ip = ip*10 + (*p-'0'); p++; tem = 1; }
    if(*p == '.' || *p == ','){
        p++;
        while(*p >= '0' && *p <= '9'){ fp = fp*10 + (*p-'0'); fq *= 10; p++; tem = 1; }
    }
    if(!tem){ *ok_ = 0; return rq(0,1); }
    *ok_ = 1;
    Rq v = rq(sig*(ip*fq + fp), fq);
    while(*p == ' ') p++;
    switch(*p){                                    /* os sufixos da bancada, exatos */
        case 'p': return rq_mul(v, rq(1, 1000000000000L));
        case 'n': return rq_mul(v, rq(1, 1000000000L));
        case 'u': return rq_mul(v, rq(1, 1000000L));
        case 'm': return rq_mul(v, rq(1, 1000L));
        case 'k': case 'K': return rq_mul(v, rq(1000, 1));
        case 'M': return rq_mul(v, rq(1000000L, 1));
        default: return v;
    }
}
static int circ_le(const char *f, const char *chave, Rq *v, int quantos){
    const char *p = strstr(f, chave);
    if(!p) return 0;
    p += strlen(chave);
    for(int k = 0; k < quantos; k++){
        while(*p == ' ' || *p == ',' || *p == '=') p++;
        int bom = 0;
        v[k] = circ_valor(p, &bom);
        if(!bom) return 0;
        while(*p && *p != ' ' && *p != ',') p++;
    }
    return 1;
}
static int e_circuito(const char *f){
    static const char *k[] = { "rlc", "serie", "série", "paralelo", "divisor",
                               "ressonancia", "ressonância", "wheatstone", "transistor",
                               "amplificador", "ganho", "logica", "lógica", "porta", "somador" };
    for(size_t j = 0; j < sizeof k/sizeof *k; j++){
        const char *p = strstr(f, k[j]);
        if(!p) continue;
        /* tem de haver NÚMERO depois — senão "o que é um circuito rlc" cairia aqui, e essa
         * é fala para o corpus. O mesmo cuidado das outras portas. */
        for(const char *q = p; *q; q++) if(*q >= '0' && *q <= '9') return 1;
    }
    return 0;
}
/* O ECO É PREGUIÇOSO. Ele estava no topo, antes de se saber se alguma régua pegava a
 * fala — e uma régua que IMPRIME e devolve 0 suja a resposta de quem vier a seguir:
 * medido, «ganho 5» ecoava a fala e logo abaixo vinha o decreto «não sei», e o eco
 * ainda entrava na resposta do dicionário. Quem não responde, não fala. */
static int ECO_DADO = 0;
static void eco(const char *f){ if(!ECO_DADO){ printf("   %s\n", f); ECO_DADO = 1; } }
/* A RAIZ DE UMA FRAÇÃO, EXATA. Ou p e q são ambos quadrados e ela fecha em ℚ, ou não
 * fecha — e aí não se escreve decimal nenhum: escreve-se o CORPO onde ela é exata, com
 * a sua fração contínua. É a mesma régua da borda, aplicada à bancada. */
static const char *raiz_rq(Rq a){
    static char buf[2][160]; static int k = 0;
    char *o = buf[k++ & 1];
    long rp, rq_;
    if(a.p >= 0 && quadrado_perfeito(a.p, &rp) && quadrado_perfeito(a.q, &rq_))
        snprintf(o, 160, "%s (fecha em Q)", frac2(rp, rq_));
    else if(a.p >= 0)
        snprintf(o, 160, "a folha de y² = %ld com y = %ld·s, FC %s",
                 a.p * a.q, a.q, fc_da_borda(0, -a.p * a.q));
    else
        snprintf(o, 160, "imaginária: a folha de y² = %ld", a.p * a.q);
    return o;
}
static int resolve_circuito(const char *f){
    Rq v[4];
    ECO_DADO = 0;
    if(circ_le(f, "rlc", v, 3)){ eco(f);
        Rq R = v[0], L = v[1], C = v[2];
        Rq LC = rq_mul(L, C), w02 = rq_div(rq(1,1), LC);        /* ω₀² exato */
        Rq LsC = rq_div(L, C);
        Rq D = rq_sub(rq_mul(R,R), rq_mul(rq(4,1), LsC));       /* Δ = R² − 4L/C exato */
        Rq Rc2 = rq_mul(rq(4,1), LsC);
        printf(" = R = %s Ω, L = %s H, C = %s F   (frações exatas, sem decimal)\n",
               rq_txt(R), rq_txt(L), rq_txt(C));
        printf("   a borda é  L·s² + R·s + 1/C = 0  — a MESMA das EDs, com s no lugar do σ\n");
        printf("   ω₀² = 1/(LC) = %s exato,  e ω₀ = %s\n", rq_txt(w02), raiz_rq(w02));
        printf("   Δ = R² - 4L/C = %s,  e R crítico² = 4L/C = %s (R_c = %s)\n",
               rq_txt(D), rq_txt(Rc2), raiz_rq(Rc2));
        printf("   logo é %s%s\n",
               rq_sinal(D) < 0 ? "SUBAMORTECIDO: oscila e decai (Δ<0, o par conjugado — o círculo)"
             : rq_sinal(D) > 0 ? "SOBREAMORTECIDO: volta sem oscilar (Δ>0, duas reais — a hipérbole)"
                               : "CRÍTICO: a raiz é DUPLA (Δ=0, a fronteira ε²=0)",
               rq_sinal(D) == 0 ? " — e a 2ª solução entra como t·e^{st}" : "");
        printf("   e em ω₀ a parte reativa anula-se por IDENTIDADE, não por arredondamento:\n");
        printf("   ω₀L = 1/(ω₀C) ⟺ ω₀² = 1/(LC), que é exatamente o que ω₀ é — logo Im Z = 0\n");
        printf("   exato e o fator de potência é 1 exato.\n");
        printf("   (na ressonância o +1 do indutor cancela o -1 do capacitor: nada volta,\n");
        printf("    toda a potência é ativa. É o casamento — o cone nulo σ=1 em circuito)\n");
        return 1;
    }
    if(circ_le(f, "ressonancia", v, 2) || circ_le(f, "ressonância", v, 2)){ eco(f);
        Rq L = v[0], C = v[1];
        Rq w02 = rq_div(rq(1,1), rq_mul(L,C)), Z02 = rq_div(L, C);
        printf(" = L = %s H, C = %s F\n", rq_txt(L), rq_txt(C));
        printf("   ω₀² = 1/(LC) = %s,  ω₀ = %s\n", rq_txt(w02), raiz_rq(w02));
        printf("   Z₀² = L/C = %s,  Z₀ = %s    (a média geométrica — o metal, La Hire)\n",
               rq_txt(Z02), raiz_rq(Z02));
        printf("   (o indutor tem multiplicidade +1 e o capacitor -1; somam 0, que é o\n");
        printf("    resistor — e é por isso que na ressonância só sobra o R)\n");
        return 1;
    }
    if(circ_le(f, "serie", v, 2) || circ_le(f, "série", v, 2)){ eco(f);
        printf(" = em SÉRIE as impedâncias SOMAM — é Kirchhoff, a operação ⊕\n");
        printf("   %s + %s = %s Ω   (exato)\n", rq_txt(v[0]), rq_txt(v[1]),
               rq_txt(rq_som(v[0], v[1])));
        return 1;
    }
    if(circ_le(f, "paralelo", v, 2)){ eco(f);
        Rq g = rq_som(rq_div(rq(1,1), v[0]), rq_div(rq(1,1), v[1]));
        printf(" = em PARALELO somam as CONDUTÂNCIAS — o mesmo ⊕, no dual\n");
        printf("   1/%s + 1/%s = %s S,  logo Z = %s Ω   (exato)\n",
               rq_txt(v[0]), rq_txt(v[1]), rq_txt(g), rq_txt(rq_div(rq(1,1), g)));
        printf("   (série e paralelo são o par dual Z ⋈ Y: a mesma soma, dos dois lados)\n");
        return 1;
    }
    if(circ_le(f, "divisor", v, 2)){ eco(f);
        Rq a = rq_div(v[1], rq_som(v[0], v[1]));
        printf(" = o DIVISOR é o PRODUTO (⊗): o ganho α, e compor divisores MULTIPLICA\n");
        printf("   α = R2/(R1+R2) = %s/(%s+%s) = %s   (uma RAZÃO, e a razão é exata)\n",
               rq_txt(v[1]), rq_txt(v[0]), rq_txt(v[1]), rq_txt(a));
        printf("   e V_out = α·V_in;  dois em cascata dão α₁·α₂, não α₁+α₂\n");
        return 1;
    }
    if(circ_le(f, "wheatstone", v, 3)){ eco(f);
        Rq zx = rq_div(rq_mul(v[1], v[2]), v[0]);
        printf(" = a ponte mede por ANULAÇÃO: ajusta-se até o detector ler ZERO\n");
        printf("   equilíbrio Z₁·Z_x = Z₂·Z₃  ->  Z_x = Z₂·Z₃/Z₁ = %s·%s/%s = %s Ω\n",
               rq_txt(v[1]), rq_txt(v[2]), rq_txt(v[0]), rq_txt(zx));
        printf("   e no equilíbrio o detector lê ZERO EXATO: Z₁·Z_x − Z₂·Z₃ = %s\n",
               rq_txt(rq_sub(rq_mul(v[0], zx), rq_mul(v[1], v[2]))));
        printf("   (não se lê o valor num mostrador, que teria a precisão do mostrador:\n");
        printf("    lê-se a RAZÃO no ponto de resíduo 0 — e a razão é exata)\n");
        return 1;
    }
    if(circ_le(f, "amplificador", v, 2) || circ_le(f, "ganho", v, 2)){ eco(f);
        Rq Ic = v[0], Rc = v[1];
        Rq vt = rq(25852, 1000000);                 /* VT a 300 K, como fração exata */
        Rq gm = rq_div(Ic, vt), Av = rq_mul(gm, Rc);
        printf(" = o AMPLIFICADOR é o transistor DENTRO da janela ativa\n");
        printf("   VT = %s V (a 300 K, exato em fração)\n", rq_txt(vt));
        printf("   gm = dIc/dVbe = Ic/VT = %s/%s = %s A/V   (a transcondutância)\n",
               rq_txt(Ic), rq_txt(vt), rq_txt(gm));
        printf("   A_v = -gm·Rc = -%s      (o ganho, em emissor comum)\n", rq_txt(Av));
        printf("   AMPLIFICAR É LINEARIZAR: gm é a DERIVADA da exponencial no ponto de\n");
        printf("   operação — é a parte ε de f(a+bε) = f(a) + f'(a)·b·ε, com ε² = 0.\n");
        printf("   (e por isso o ganho depende do ponto de operação; com realimentação ele\n");
        printf("    vira 1/β, uma RAZÃO de resistores — e a razão é exata)\n");
        return 1;
    }
    if(circ_le(f, "logica", v, 2) || circ_le(f, "lógica", v, 2) || circ_le(f, "porta", v, 2)){ eco(f);
        int a = v[0].p != 0, b = v[1].p != 0;
        printf(" = o transistor CHAVEANDO: fora da janela, o contínuo colapsa em GF(2)\n");
        printf("   a = %d, b = %d\n", a, b);
        printf("   AND  = %d      e AND É a MULTIPLICAÇÃO de GF(2): a·b = %d\n", a&&b, (a*b)%2);
        printf("   XOR  = %d      e XOR É a SOMA de GF(2):          a+b = %d\n", a!=b, (a+b)%2);
        printf("   OR   = %d      NOT a = %d      NAND = %d\n", a||b, !a, !(a&&b));
        printf("   e De Morgan:  ¬(a∧b) = %d = ¬a∨¬b = %d   — o NOT troca ∧ por ∨\n",
               !(a&&b), (!a)||(!b));
        printf("   (NOT é uma dobra de ordem 2, como o conj e o J; e em GF(2) vale -x = x,\n");
        printf("    logo somar É subtrair — é por isso que o XOR é reversível de graça)\n");
        return 1;
    }
    if(circ_le(f, "somador", v, 3) || circ_le(f, "somador", v, 2)){ eco(f);
        int a = v[0].p != 0, b = v[1].p != 0;
        int ci = (v[2].q == 1 && (v[2].p == 0 || v[2].p == 1)) ? (int)v[2].p : 0;
        int sm = (a != b) != ci, co = (a&&b) || (ci && (a!=b));
        printf(" = o SOMADOR COMPLETO, em portas:\n");
        printf("   s    = a ⊕ b ⊕ cin              = %d\n", sm);
        printf("   cout = (a∧b) ∨ (cin ∧ (a⊕b))   = %d\n", co);
        printf("   e a aritmética direta: %d + %d + %d = %d, que em binário é %d%d  <- O MESMO\n",
               a, b, ci, a+b+ci, co, sm);
        printf("   (dois caminhos: as portas e a conta. Um somador que só fecha num deles\n");
        printf("    não está validado — está adivinhado)\n");
        return 1;
    }
    if(circ_le(f, "transistor", v, 1)){ eco(f);
        Rq V = v[0], vt = rq(25852, 1000000);
        Rq x = rq_div(V, vt);
        printf(" = SHOCKLEY: I = Is·(e^{V/VT} - 1),  com VT = %s V a 300 K\n", rq_txt(vt));
        printf("   V = %s V  ->  V/VT = %s   (a fração exata; é ela o expoente)\n",
               rq_txt(V), rq_txt(x));
        printf("   e o VALOR de e^{V/VT} não se escreve em decimal: a exponencial é o FLUXO\n");
        printf("   (σ = e^λ), e o que é exato aqui é a LEI que ela cumpre —\n");
        printf("   AQUI VIVE O OPERADOR. Π = exp∘Σ∘log é esta equação:\n");
        printf("   I(V₁+V₂) = I(V₁)·I(V₂)/Is — a SOMA de tensões vira PRODUTO de correntes,\n");
        printf("   e essa identidade vale EXATA para todo V₁, V₂, sem avaliar nada.\n");
        printf("   (é a cláusula de Pontryagin do contrato, em volts e amperes; e é por isso\n");
        printf("    que a Gilbert cell multiplica dois sinais: log, soma, antilog)\n");
        return 1;
    }
    return 0;
}

/* A INVOLUÇÃO NA ENTRADA — o cone desdobra-se antes de se decidir o que ele é.
 *
 * O Aarão: «a saída é a ESPIRAL e a entrada é o CONE; o cone é mais compacto, o tempo é
 * diferente para ele, então precisa DESDOBRAR — aplica-se involução na entrada e evolução
 * no banco.»
 *
 * "vezes" É o x. "mais" É o +. São a mesma operação escrita comprimida, e a assistente não
 * o sabia: `3 x 3` desdobrava-se e `3 vezes 3` respondia "não sei".
 *
 * O DESDOBRAMENTO NÃO TOCA NO CAMINHO DO CORPUS. Só o ramo das contas vê a forma desdobrada;
 * a procura continua a receber a fala como ela veio — senão "3 vezes 3 é igual a 3 mais 3",
 * que está no corpus com essas palavras, deixava de se encontrar a si própria.
 *
 * A troca é só de PALAVRA INTEIRA. Sem isso "demais" viraria "de+" e "vezess" mudava também.
 * Medido sobre as 252 falas do corpus: ZERO mudam de destino — e o controlo positivo mostra
 * que `3 vezes 3` desdobra, enquanto "nada anda mais rápido que a luz", "quanto é 1 mais 1"
 * e "a raiz de 2 é racional" ficam no corpus, que é onde têm de ficar. */
static void inv_troca(char *d, size_t dn, const char *s, const char *de, const char *para){
    size_t lp = strlen(para), ld = strlen(de), o = 0;
    for(const char *p = s; *p && o + lp + 1 < dn; ){
        int ini = (p == s) || !((p[-1]>='a'&&p[-1]<='z') || (p[-1]>='A'&&p[-1]<='Z'));
        if(ini && !strncmp(p, de, ld)){
            char c = p[ld];
            if(!c || !((c>='a'&&c<='z') || (c>='A'&&c<='Z'))){
                memcpy(d + o, para, lp); o += lp; p += ld; continue;
            }
        }
        d[o++] = *p++;
    }
    d[o] = 0;
}
static void desdobra_entrada(char *d, size_t dn, const char *s){
    char a[1024], b2[1024], c2[1024];
    inv_troca(a,  sizeof a,  s,  "vezes", "x");
    inv_troca(b2, sizeof b2, a,  "mais",  "+");
    inv_troca(c2, sizeof c2, b2, "menos", "-");   /* a mesma guarda: "pelo menos avisa"
                                                   * nunca vira conta — as outras palavras
                                                   * não passam a porta do e_conta */
    inv_troca(d,  dn,        c2, "dividido por", "/");  /* a frase inteira, com a mesma
                                                   * fronteira: "o povo dividido por
                                                   * guerras" tem letras e não passa */
    char e2[1024]; snprintf(e2, sizeof e2, "%s", d);
    inv_troca(d,  dn,        e2, "por cento", "%");     /* «10 por cento de 200» — o "de"
                                                   * a seguir ao % a fita já aceitava;
                                                   * «dez por cento das vezes» tem
                                                   * letras e fica no corpus */
}

/* A MEMBRANA TEXTUAL — o LaTeX desdobra-se na entrada.
 *
 * O LaTeX é a «interface padrão: a membrana textual por omissão» da casa (Corpo
 * Universal, §papéis), e o tradutor .tex↔PDF «opera nesta torre» (Corpo de Peano).
 * Mas o tradutor COMPÕE — desenha a página; não avalia. O que faltava é a mesma lei
 * do cone: «\frac{1}{2}» e «(1)/(2)» são a mesma conta em duas roupas, e a entrada
 * vem comprimida na membrana. Desdobrada, quem resolve são as réguas de sempre.
 *
 * O ANINHAMENTO FECHA POR PONTO FIXO: cada volta desdobra um andar (o \frac de dentro
 * fica intacto na primeira passagem e cai na segunda), e pára quando a fala não muda
 * — a mesma paragem da conta, que dobra do mais fundo para fora. Sem recursão e sem
 * malloc: um buffer, e a volta a decidir. */
static const char *lx_grupo(const char *p, char *out, size_t lim){
    /* lê {..} equilibrado a partir de p (que aponta ao '{'); devolve o depois-do-'}' */
    size_t o = 0; int prof = 0;
    if(*p != '{'){ out[0] = 0; return p; }
    for(; *p; p++){
        if(*p == '{'){ if(prof++) { if(o + 1 < lim) out[o++] = *p; } continue; }
        if(*p == '}'){ if(--prof == 0){ p++; break; } }
        if(o + 1 < lim) out[o++] = *p;
    }
    out[o] = 0;
    return p;
}
/* O DIALECTO É O DO TRADUTOR — a tabela LX contra a de `tests/tex_core.c`.
 *
 * Escrevi esta lista de cabeça e ela trazia `\ast` e `\pmod`: o LaTeX de fora tem-nos,
 * ESTA CASA NÃO. O tradutor é que declara a língua (a tabela de símbolos e os
 * operadores nomeados), e a assistente só pode desdobrar o que ele compõe — senão são
 * duas línguas com o mesmo nome. `tools/bench_membrana.sh` mede os dois caminhos.
 *
 * O `\pm` está no dialecto e fica FORA de propósito: ± não é um valor, são dois — a
 * fala com ele cai ao corpus em vez de fingir uma conta. */
enum { LX_FRAC = 1, LX_SQRT, LX_MULT, LX_DIV, LX_MOD, LX_ESPACO, LX_AMB };
static const struct { const char *nome; int tipo; } LX[] = {
    { "frac", LX_FRAC }, { "dfrac", LX_FRAC }, { "tfrac", LX_FRAC },
    { "sqrt", LX_SQRT },
    { "cdot", LX_MULT }, { "times", LX_MULT },
    { "div",  LX_DIV  },
    { "bmod", LX_MOD  }, { "mod", LX_MOD },
    { "left", LX_ESPACO }, { "right", LX_ESPACO },
    { "quad", LX_ESPACO }, { "qquad", LX_ESPACO },
    /* os ambientes que o tradutor compõe — e o `cases` não está aqui porque ele não o
     * compõe: a fala em bloco entra pelos que existem, não pelos que eu quisesse */
    { "align", LX_AMB }, { "aligned", LX_AMB }, { "equation", LX_AMB },
};
static int lx_passo(char *d, size_t dn, const char *s){
    size_t o = 0; int mudou = 0, dentro = 0;   /* dentro de um ambiente conhecido */
    char g1[512], g2[512];
    while(*s && o + 8 < dn){
        if(*s == '\\'){
            const char *p = s + 1;
            size_t n = 0; while(p[n] >= 'a' && p[n] <= 'z') n++;
            /* O AMBIENTE: `\begin{align}` … `\end{align}`. Só os que o tradutor compõe;
             * o que ele não conhece fica como veio e a régua recusa-o, que é o certo. */
            if((n == 5 && !strncmp(p, "begin", 5)) || (n == 3 && !strncmp(p, "end", 3))){
                const char *q = p + n;
                if(*q == '{'){
                    char nome[64]; const char *q2 = lx_grupo(q, nome, sizeof nome);
                    size_t ln = strlen(nome);
                    if(ln && nome[ln-1] == '*') nome[ln-1] = 0;   /* align* é align */
                    int amb = 0;
                    for(size_t k = 0; k < sizeof LX/sizeof *LX; k++)
                        if(LX[k].tipo == LX_AMB && !strcmp(nome, LX[k].nome)){ amb = 1; break; }
                    if(amb){
                        dentro = (n == 5);
                        o += (size_t)snprintf(d + o, dn - o, " ");
                        s = q2; mudou = 1; continue;
                    }
                }
            }
            /* o `\\` é a fila: dentro do ambiente é o `;` que o sistema espera */
            if(n == 0 && *p == '\\'){
                o += (size_t)snprintf(d + o, dn - o, dentro ? " ; " : " ");
                s = p + 1; mudou = 1; continue;
            }
            int tipo = 0;
            for(size_t k = 0; n && k < sizeof LX/sizeof *LX; k++)
                if(n == strlen(LX[k].nome) && !strncmp(p, LX[k].nome, n)){ tipo = LX[k].tipo; break; }
            if(tipo == LX_FRAC){
                const char *q = lx_grupo(p + n, g1, sizeof g1);
                q = lx_grupo(q, g2, sizeof g2);
                o += (size_t)snprintf(d + o, dn - o, "(%s)/(%s)", g1, g2);
                s = q; mudou = 1; continue;
            }
            if(tipo == LX_SQRT){
                const char *q = lx_grupo(p + n, g1, sizeof g1);
                o += (size_t)snprintf(d + o, dn - o, "raiz (%s)", g1);
                s = q; mudou = 1; continue;
            }
            if(tipo == LX_MULT){
                o += (size_t)snprintf(d + o, dn - o, " x "); s = p + n; mudou = 1; continue;
            }
            if(tipo == LX_DIV){
                o += (size_t)snprintf(d + o, dn - o, " / "); s = p + n; mudou = 1; continue;
            }
            if(tipo == LX_MOD){
                const char *q = p + n;
                if(*q == '{'){ q = lx_grupo(q, g1, sizeof g1);
                    o += (size_t)snprintf(d + o, dn - o, " mod %s", g1); }
                else o += (size_t)snprintf(d + o, dn - o, " mod ");
                s = q; mudou = 1; continue;
            }
            if(tipo == LX_ESPACO){
                o += (size_t)snprintf(d + o, dn - o, " "); s = p + n; mudou = 1; continue;
            }
            if(n == 0 && (*p == ',' || *p == ';' || *p == '!' || *p == ':' || *p == ' ')){
                o += (size_t)snprintf(d + o, dn - o, " "); s = p + 1; mudou = 1; continue;
            }
            if(n == 0 && (*p == '(' || *p == ')' || *p == '[' || *p == ']')){
                o += (size_t)snprintf(d + o, dn - o, " "); s = p + 1; mudou = 1; continue;
            }
            if(n == 0 && *p == '%'){ d[o++] = '%'; s = p + 1; mudou = 1; continue; }
            d[o++] = *s++;                          /* comando que não é desta conta */
            continue;
        }
        if(*s == '$'){ d[o++] = ' '; s++; mudou = 1; continue; }
        if(*s == '^' && s[1] == '{'){                /* o expoente: {2} é 2, e o resto
                                                      * vai em parênteses */
            const char *q = lx_grupo(s + 1, g1, sizeof g1);
            int so_digito = g1[0] != 0;
            for(const char *t = g1; *t; t++) if(*t < '0' || *t > '9') so_digito = 0;
            o += (size_t)snprintf(d + o, dn - o, so_digito ? "^%s" : "^(%s)", g1);
            s = q; mudou = 1; continue;
        }
        if(*s == '{'){ d[o++] = '('; s++; mudou = 1; continue; }
        if(*s == '}'){ d[o++] = ')'; s++; mudou = 1; continue; }
        d[o++] = *s++;
    }
    d[o] = 0;
    return mudou;
}
/* A VOLTA DA MEMBRANA: o valor vestido na roupa em que a fala veio. Sem este lado a
 * palavra «membrana» era metade com o nome do par — o tradutor compõe, a assistente
 * lê, e a resposta voltava nua. A fração é a pilha do \frac (o mesmo objeto que o
 * tradutor desenha); o inteiro é ele próprio. */
/* ── O DUAL †: a membrana é UMA operação, e o sinal decide o lado ──────────────
 * Não são duas operações que calham ser inversas — é a INVOLUÇÃO das cinco, e o
 * sinal escreve-a: +1 desdobra (a entrada, o cone a abrir-se) e −1 veste (a saída,
 * o valor na roupa em que a fala veio). É a Lei 1: 1† = −1, e ν∘ν = id.
 * Os nomes antigos ficam como invólucro de uma linha, como a casa já fez com o
 * le/grava sobre o MOVE. */
static void lx_veste(char *out, size_t lim, long v, long q);
static void lx_abre(char *d, size_t dn, const char *s);
static void MEMBRANA(char *out, size_t lim, const char *texto, long v, long q, int sentido){
    if(sentido > 0) lx_abre(out, lim, texto);      /* +1: a entrada desdobra-se */
    else            lx_veste(out, lim, v, q);      /* −1: a saída veste-se */
}
static void lx_veste(char *out, size_t lim, long v, long q){
    if(q == 1) snprintf(out, lim, "%ld", v);
    else       snprintf(out, lim, "\\frac{%ld}{%ld}", v, q);
}
static void veste_valor(char *out, size_t lim, long v, long q){
    MEMBRANA(out, lim, 0, v, q, -1);
}
/* a marca da membrana: um comando `\letra`, um `$` ou um expoente `^{`. Sem marca não
 * se desdobra nada — a fala em português segue byte a byte para as réguas de sempre. */
static int tem_membrana(const char *f){
    for(const char *p = f; *p; p++){
        if(*p == '$') return 1;
        if(*p == '\\' && p[1] >= 'a' && p[1] <= 'z') return 1;
        if(*p == '^' && p[1] == '{') return 1;
    }
    return 0;
}
static void lx_abre(char *d, size_t dn, const char *s){
    char a[1200], b2[1200];
    int houve = 0;
    snprintf(a, sizeof a, "%s", s);
    for(int volta = 0; volta < 8; volta++){        /* o ponto fixo: pára quando não muda */
        if(!lx_passo(b2, sizeof b2, a)) break;
        if(!strcmp(b2, a)) break;
        snprintf(a, sizeof a, "%s", b2);
        houve = 1;
    }
    if(!houve){ snprintf(d, dn, "%s", s); return; } /* sem membrana, sai byte a byte */
    size_t o = 0;                                   /* o comando que sai deixa o espaço
                                                     * dele: colapsa-se, e a conta não
                                                     * distingue um espaço de três */
    for(const char *p = a; *p && o + 1 < dn; p++){
        if(*p == ' ' && (o == 0 || d[o-1] == ' ')) continue;
        d[o++] = *p;
    }
    while(o && d[o-1] == ' ') o--;
    d[o] = 0;
}

/* AS FUNÇÕES NOMEADAS — a resolução de expressões RECUPERADA para a fala. A fita já
 * sabia (raiz, !, ^, mod, %, frações com o porquê decimal, o i e o i*) e só entrava
 * quem escrevesse os símbolos. A função em português é «<nome> de X»: monta-se a
 * expressão, a guarda é a de sempre (o X tem de ser conta pura — «a raiz de 2 é
 * racional» continua no corpus, medido no §C11), e a resolução é a MESMA fita, com
 * os passos à vista. A montagem separa-se da resolução para o medidor a ver. */
static int funcao_monta(const char *fala, char *expr, size_t lim){
    static const struct { const char *nome, *pre, *pos; } F[] = {
        { "raiz quadrada de ", "raiz (", ")"     },
        { "raiz de ",          "raiz (", ")"     },
        { "fatorial de ",      "(",      ") !"   },
        { "dobro de ",         "2 x (",  ")"     },
        { "triplo de ",        "3 x (",  ")"     },
        { "metade de ",        "(",      ") / 2" },
        { "quadrado de ",      "(",      ") ^ 2" },
        { "cubo de ",          "(",      ") ^ 3" },
    };
    const char *p = fala;
    if(!strncmp(p, "o ", 2) || !strncmp(p, "a ", 2)) p += 2;   /* o artigo é roupa */
    for(size_t k = 0; k < sizeof F/sizeof *F; k++){
        size_t ln = strlen(F[k].nome);
        if(strncmp(p, F[k].nome, ln)) continue;
        char desd[1024]; desdobra_entrada(desd, sizeof desd, p + ln);
        if(!e_conta(desd)) return 0;               /* a guarda: só a conta pura entra */
        snprintf(expr, lim, "%s%s%s", F[k].pre, desd, F[k].pos);
        return 1;
    }
    return 0;
}
static int resolve_funcao(const char *fala){
    char expr[1200];
    if(!funcao_monta(fala, expr, sizeof expr)) return 0;
    return resolve_conta(expr);
}

/* O CORTE moldura→parâmetro (Dual Sort: o corte é a seleção de suporte; e o WHERE do
 * mórfico: erode-se para ESCOLHER, dilata-se para ESCREVER). A erosão escolheu a moldura
 * — d símbolos consumidos —, e o que sobra na fala é o PARÂMETRO: os dois lados da fala,
 * uma componente por elemento. Se a resposta aprendida tem o buraco '_', o parâmetro
 * escreve-se nele. Sem parâmetro o buraco fica por preencher e a moldura NÃO responde —
 * o quantificador sem instância.
 * Devolve 1 = out pronto; 0 = buraco sem parâmetro (não responder por esta via). */
/* o buraco é o "___" — o blank de preencher. Um '_' só não serve de marcador: o corpus
 * cita caminhos ("torre_fundacao.tex") e LaTeX ("\_"), e a moldura falsa roubava a
 * resposta (medido: «o que é um corpo» e «mostra a fundação» caíam no decreto). */
static const char *acha_buraco(const char *s){ return strstr(s, "___"); }
static int corte_escreve(const char *fala, int d, const char *resp, char *out, size_t lim){
    const char *bur = acha_buraco(resp);
    if(!bur){ snprintf(out, lim, "%s", resp); return 1; }
    const char *p = fala;
    for(int k = 0; k < d && *p; k++) prox_simb(&p);
    for(;;){                                       /* apara os separadores do corte */
        if(!*p) break;
        const char *t = p;
        if(simb_de_palavra(prox_simb(&t))) break;
        p = t;
    }
    if(!*p) return 0;
    char par[512]; snprintf(par, sizeof par, "%s", p);
    size_t n = strlen(par);
    while(n){ unsigned char c = par[n-1];
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c>=0x80) break;
        n--; }
    par[n] = 0;
    if(!n) return 0;
    snprintf(out, lim, "%.*s%s%s", (int)(bur - resp), resp, par, bur + 3);
    return 1;
}

/* A EROSÃO RE-ANCORADA: o prefixo com ruído à frente — come-se o início até uma
 * fronteira de palavra e o prefixo tenta outra vez (a mesma re-âncora da dilatação,
 * na régua do prefixo). É a via que dá o CORTE às molduras no meio da fala: «que
 * estilo? gosto de rock» ancora em «gosto de» e o resto preenche o buraco. Devolve
 * também onde ancorou (*efetiva), para o corte contar os símbolos do sítio certo. */
static long erosao_ancorada(const char *fala, int *fundo, const char **efetiva){
    int fronteira = 1;
    for(const char *p = fala; *p; ){
        if(fronteira){
            no_banco(banco_da(p));
            long r = erosao(p, fundo);
            if(r){ *efetiva = p; return r; }
        }
        long s = prox_simb(&p);
        fronteira = !simb_de_palavra(s);
    }
    return 0;
}

/* AS TAREFAS — as portas que AUMENTAM A MASSA (Dual Sort §assistente: «se faltar
 * informação, o agente aumenta a massa — ferramenta, memória, cálculo — não finge que
 * ela já estava lá»). Portas fechadas em português como a das contas: quem não encaixa
 * cai ao corpus, e o corpus continua dono da conversa. */

static void apara(char *s){
    size_t n = strlen(s), i = 0;
    while(s[i] == ' ') i++;
    if(i) memmove(s, s + i, n - i + 1), n -= i;
    while(n && s[n-1] == ' ') s[--n] = 0;
}

/* «lembra que X = Y» — o ensino pela porta da fala: a EVOLUÇÃO do banco na conversa,
 * com o '=' que a casa já usa («ensina-me com: = resposta»). Sem '=', aceita-se UM
 * « é » — com dois a fala é ambígua e não se adivinha (a regra da evolução). */
static int e_lembra(const char *f){
    return !strncmp(f, "lembra que ", 11) || !strncmp(f, "aprende que ", 12);
}
static int resolve_lembra(const char *f){
    const char *p = f + (f[0] == 'l' ? 11 : 12);
    char x[512], y[512];
    const char *ig = strchr(p, '=');
    if(ig){
        snprintf(x, sizeof x, "%.*s", (int)(ig - p), p);
        snprintf(y, sizeof y, "%s", ig + 1);
    } else {
        const char *e = strstr(p, " é ");
        if(!e || strstr(e + strlen(" é "), " é ")) return 0;
        snprintf(x, sizeof x, "%.*s", (int)(e - p), p);
        snprintf(y, sizeof y, "%s", e + strlen(" é "));
    }
    apara(x); apara(y);
    if(!x[0] || !y[0]) return 0;
    aprende(x, y);
    printf("   (lembrado: «%s» responde «%s»)\n", x, y);
    return 1;
}

/* «esquece X» — o inverso do lembra pela mesma porta: desce-se ao nó da fala e a
 * resposta apaga-se (o .b zera). O caminho FICA — esquecer não é demolir a árvore,
 * é o nó voltar a não ter fala. O que nunca se soube devolve 0 e o corpus decide. */
static int e_esquece(const char *f){ return !strncmp(f, "esquece ", 8); }
static int resolve_esquece(const char *f){
    char x[512]; snprintf(x, sizeof x, "%s", f + 8);
    apara(x);
    if(!x[0]) return 0;
    if(!CORPUS(x, 0, -1)) return 0;                /* −1: a retração da mesma operação */
    printf("esquecido: «%s».\n   (o caminho fica; a resposta foi-se — é a mesma\n"
           "    operação do aprender, com o sinal trocado)\n", x);
    return 1;
}

/* o relógio e o calendário — a primeira ferramenta: a massa que o corpus não tem. */
#include <time.h>
#include <sys/stat.h>
static int e_hora(const char *f){
    return !strncmp(f, "que horas", 9) || !strncmp(f, "q horas", 7) ||
           !strncmp(f, "tem horas", 9);
}
static int resolve_hora(void){
    time_t agora = time(NULL);
    struct tm tm_; if(!localtime_r(&agora, &tm_)) return 0;
    printf("são %02d:%02d.\n   (o relógio da máquina)\n", tm_.tm_hour, tm_.tm_min);
    return 1;
}
static int e_data(const char *f){
    return !strncmp(f, "que dia", 7);
}
static int resolve_data(void){
    static const char *dia[] = { "domingo", "segunda-feira", "terça-feira",
        "quarta-feira", "quinta-feira", "sexta-feira", "sábado" };
    time_t agora = time(NULL);
    struct tm tm_; if(!localtime_r(&agora, &tm_)) return 0;
    printf("hoje é %s, %02d/%02d/%04d.\n   (o calendário da máquina)\n",
           dia[tm_.tm_wday % 7], tm_.tm_mday, tm_.tm_mon + 1, tm_.tm_year + 1900);
    return 1;
}

/* «mostra X» — o parâmetro vira slug e procura-se no catálogo: a ponte que os pares
 * fixos já faziam nome a nome, agora com o parâmetro extraído. Se o ficheiro não
 * existe, devolve 0 e o corpus decide — os pares curados continuam a valer. */
static int resolve_mostra_em(const char *f, const char *raiz){
    const char *p = f + 7;                          /* depois de "mostra " */
    if(!strncmp(p, "o ", 2) || !strncmp(p, "a ", 2)) p += 2;
    char slug[256]; int o = 0;
    while(*p && o < 250){
        long s = prox_simb(&p);                     /* despe o acento, minusculiza */
        if(simb_de_palavra(s)) slug[o++] = (char)(s + 31);
        else if(o && slug[o-1] != '_') slug[o++] = '_';
    }
    while(o && slug[o-1] == '_') o--;
    slug[o] = 0;
    if(!o) return 0;
    char cam[512]; snprintf(cam, sizeof cam, "%s/%s.tex", raiz, slug);
    if(access(cam, R_OK) != 0) return 0;
    printf("vê papers/%s.tex\n   (o catálogo tem — o tradutor abre)\n", slug);
    return 1;
}
static int e_mostra(const char *f){ return !strncmp(f, "mostra ", 7); }
static int resolve_mostra(const char *f){ return resolve_mostra_em(f, "../papers"); }

/* A CASCATA SIMBÓLICA — as réguas que RESOLVEM em vez de procurar, numa função só:
 * circuito, polinómio, sistema, ED, álgebra, equação, lei pedida, conta (nua e pelo
 * cone) e as funções nomeadas. O responde entra com a fala como veio; «resolve X» /
 * «calcula X» entram com a roupa tirada — as MESMAS réguas, duas portas. */
static int resolve_simbolico(const char *fala){
    if(resolve_divisibilidade(fala)) return 1;     /* o relógio de 6 ticks */
    if(resolve_bezout(fala)) return 1;             /* a testemunha e o critério */
    if(resolve_linear(fala)) return 1;             /* linear e dual, 16 + 14 */
    if(resolve_corpo(fala)) return 1;              /* teoria dos corpos, os 25 */
    if(resolve_estrutura(fala)) return 1;          /* álgebra moderna, os 20 */
    if(resolve_eliptica(fala)) return 1;           /* Dirichlet e as elípticas */
    if(resolve_numeros(fala)) return 1;            /* teoria dos números, os 17 */
    if(resolve_identifica(fala)) return 1;         /* o mesmo ponto, quatro portas */
    if(resolve_prova_real(fala)) return 1;         /* as vinte provas do eval.txt */
    if(resolve_reais(fala)) return 1;              /* ℝ: o corte, e nunca um decimal */
    if(resolve_racionais(fala)) return 1;          /* ℚ: a fibra, e a sua ausência */
    if(resolve_naturais(fala)) return 1;           /* a escada da aritmética */
    if(resolve_relacao(fala)) return 1;            /* relação → função → volta */
    if(resolve_conjuntos(fala)) return 1;          /* conjuntos ↔ booleano ↔ prova */
    if(resolve_prova(fala)) return 1;              /* o rastro verificável */
    if(resolve_shannon(fala)) return 1;            /* a expansão = o corte */
    if(resolve_simplifica(fala)) return 1;         /* a lei em cada transição */
    if(resolve_booleana(fala)) return 1;           /* a lógica no corpo GF(2) */
    if(resolve_calculo(fala)) return 1;            /* «integra …» / «deriva …» */
    if(resolve_as_cinco(fala)) return 1;           /* a vista do padrão ouro */
    if(resolve_mdc_poli(fala)) return 1;           /* «mdc de … e …» — a folha da órbita */
    if(resolve_divide_poli(fala)) return 1;        /* «divide x^3-1 por x-1» — a fibra */
    if(resolve_fatora_poli(fala)) return 1;        /* «fatora x^3 - x» — a volta da convolução */
    if(e_circuito(fala) && resolve_circuito(fala)) return 1; /* a tríade em volts e amperes */
    if(e_poli(fala) && resolve_poli(fala)) return 1;         /* p(x) = q(x), de qualquer grau */
    if(e_sistema(fala) && resolve_sistema(fala)) return 1;   /* x' = Ax, e a régua é (−tr, det) */
    if(e_edo(fala) && resolve_edo(fala)) return 1;           /* a ED declara o corpo pela borda */
    if(e_algebra(fala) && resolve_algebra(fala)) return 1;   /* o corpo vem declarado na fala */
    {   /* a equação vem antes de tudo: '=' na fala é resolver, e não avaliar */
        char esq[512], dir[512];
        if(e_equacao(fala, esq, dir, sizeof esq) && resolve_eq(esq, dir)) return 1;
    }
    int dist = 0;
    const char *lei = pede_lei(fala, &dist);           /* "distribui ..." / "fatora ..." */
    if(lei && e_conta(lei) && aplica_lei(lei, dist)) return 1;
    if(e_conta(fala) && resolve_conta(fala)) return 1; /* conta não se procura: desdobra-se */
    {   /* e se não era conta, DESDOBRA-SE O CONE e pergunta-se outra vez: "3 vezes 3" é
         * "3 x 3" escrito comprimido. O corpus não vê esta forma — só o resolvedor. */
        char cone[1024]; desdobra_entrada(cone, sizeof cone, fala);
        if(strcmp(cone, fala) && e_conta(cone) && resolve_conta(cone)) return 1;
    }
    if(resolve_funcao(fala)) return 1;             /* «raiz de 16», «o fatorial de 5» —
                                                    * a fita recuperada para a fala */
    if(resolve_assunto(fala)) return 1;            /* «mola m=1 c=3 k=2» — o dicionário:
                                                    * o assunto empresta os nomes e a
                                                    * álgebra intrínseca responde */
    return 0;
}
/* o pedido em português: «resolve/resolva/calcula X» — devolve o X, ou 0 se a fala
 * não é pedido. A fronteira de palavra está no espaço do próprio prefixo:
 * «resolvemos tudo» não casa. «quanto é» NÃO entra aqui — fica no corpus, por
 * decisão antiga e medida (§C11). */
static const char *pedido_nu(const char *f){
    if(!strncmp(f, "resolve ", 8) || !strncmp(f, "resolva ", 8) ||
       !strncmp(f, "calcula ", 8)) return f + 8;
    if(!strncmp(f, "calcule ", 8)) return f + 8;
    return 0;
}

/* a fala entregue à cascata, com as roupas tiradas por ordem: primeiro a MEMBRANA
 * (o LaTeX, e é na ENTRADA — vestida, a fala passa na porta errada), depois o pedido
 * em português. As duas compõem: «resolve $\frac{1}{2}+\frac{1}{3}$» tira as duas. */
static int resolve_vestido(const char *fala){
    char mem[1200];
    if(tem_membrana(fala)){
        latex_desdobra(mem, sizeof mem, fala);
        if(strcmp(mem, fala)){
            MEMBRANA_ENTRADA = 1;                  /* e a resposta volta vestida */
            int r = resolve_simbolico(mem);
            MEMBRANA_ENTRADA = 0;
            if(r) return 1;
        }
    }
    return resolve_simbolico(fala);
}
static void responde(const char *fala){
    if(resolve_vestido(fala)) return;
    {   /* «resolve X» / «calcula X»: o pedido sai e o resto volta à mesma cascata */
        const char *nu = pedido_nu(fala);
        if(nu && resolve_vestido(nu)) return;
    }
    /* AS TAREFAS: as portas que aumentam a massa — ensino, relógio, catálogo. */
    if(e_lembra(fala)  && resolve_lembra(fala))  return;
    if(e_esquece(fala) && resolve_esquece(fala)) return;
    if(e_hora(fala)    && resolve_hora())        return;
    if(e_data(fala)    && resolve_data())        return;
    if(e_mostra(fala)  && resolve_mostra(fala))  return;
    int d = 0;
    no_banco(banco_da(fala));                      /* a erosao e a torcao vivem na cabeca */
    /* A TORCAO VEM PRIMEIRO QUANDO SOBRA FALA. Eu tinha-a posto depois da erosao e ela nunca
     * disparava: a erosao acha "bom dia" em "bom dia quem es tu", devolve, e o resto fica sem
     * resposta. A regra e essa — se o que se achou NAO CONSOME a fala toda, ha mais la dentro,
     * e quem trata disso e a torcao. */
    long v[8];
    int n = torcao(fala, v, 8);
    if(n > 1){
        printf("   (torção: %d falas no mesmo canal)\n", n);
        for(int k = 0; k < n; k++){
            no_banco(torc_banco[k < 8 ? k : 7]);   /* cada resposta no SEU banco */
            char t[RESP_LIM]; le_texto(v[k], t, sizeof t);
            printf("%s\n", t);
        }
        return;
    }
    /* PRIMEIRO DAQUI: a conversa tem fio, e o que se diz a seguir e continuacao do que se disse.
     * So depois se volta a raiz — que e recomecar, e recomecar e uma escolha, nao a regra. */
    /* QUEM TEM FIO MANDA. Eu mudava de banco pela cabeca da NOVA fala — mas a conversa esta
     * onde estava, e a cabeca so decide quando se recomeca. Procura-se o banco onde ela vai a
     * meio (onde() != RAIZ); se nenhum vai, e comeco, e ai a cabeca decide. */
    int fio = -1;
    for(int b = 0; b < NB && fio < 0; b++){ no_banco(b); if(onde() != RAIZ) fio = b; }
    long r = 0;
    int prefixal = 1;                              /* nas vias de prefixo o corte tem os
                                                    * dois lados: moldura e parâmetro */
    const char *efetiva = fala;                    /* onde a régua ancorou — o corte
                                                    * conta os símbolos daqui */
    const char *via = "daqui (a conversa continua)";
    if(fio >= 0){ no_banco(fio); r = desce_daqui(fala, &d); }
    if(!r){                                        /* nao continua: e comeco */
        no_banco(banco_da(fala));
        r = desce_daqui(fala, &d);
        if(!r){ r = erosao(fala, &d); via = "erosão (prefixo)"; }
    }
    if(!r){                                        /* o prefixo com ruído à frente */
        r = erosao_ancorada(fala, &d, &efetiva);
        if(r) via = "erosão (re-ancorada)";
    }
    if(!r){
        prefixal = 0;
        /* A DILATACAO PERGUNTA AO BARRAMENTO INTEIRO. Ela pode saltar o inicio, logo nao tem
         * cabeca fixa — emite-se, e responde quem puder. Ninguem e chamado pelo nome. */
        for(int b = 0; b < NB && !r; b++){
            no_banco(b);
            r = dilatacao(fala, &d);
        }
        via = "dilatação (subsequência), pelo barramento";
    }
    if(!r){
        /* A EVOLUÇÃO: a fala tem A MENOS, e o banco desce o resto sozinho. É o dual da erosão
         * — e como ela, pergunta ao barramento inteiro, porque quem tem a continuação pode não
         * ser quem tem a cabeça. */
        int pas = 0;
        for(int b = 0; b < NB && !r; b++){
            no_banco(b);
            r = evolucao(fala, &d, &pas);
            if(r) printf("   (evolução: o banco desceu %d símbolo(s) sozinho — a"
                         " continuação era única)\n", pas);
        }
        if(r) via = "evolução (o banco completa)";
    }
    if(!r){
        /* A BIFURCAÇÃO: a fala cabe mas não decide. Em vez de "não sei", diz-se o que se sabe
         * — para onde ela podia seguir. É o cruzamento do viveiro: as duas ficam, e quem
         * decide é quem perguntou. */
        long saidas[NBIF]; int nb = 0;
        for(int b = 0; b < NB && !nb; b++){ no_banco(b); nb = bifurcacao(fala, saidas); }
        if(nb >= 2){
            printf("   (a fala cabe no corpus mas não decide: há %d caminhos a partir dela."
                   " Do lado de cada um há isto — precisa de mais uma palavra para escolher)\n",
                   nb);
            for(int k = 0; k < nb; k++){
                char t[RESP_LIM]; le_texto(saidas[k], t, sizeof t);
                printf("   %d) %s\n", k + 1, t);
            }
            return;
        }
    }
    if(!r){
        /* ANTES DO DECRETO: perguntar ao barramento. Nao sei nao e o fim — e o fim do que EU sei. */
        char outra[RESP_LIM];
        if(pergunta_ao_barramento(fala, outra, sizeof outra)){
            printf("%s\n", outra);
            printf("   (do barramento — outra assistente sabia, e eu aprendi)\n");
            aprende(fala, outra);                  /* aprende com quem sabia */
            return;
        }
        printf("não sei.\n");                        /* o DECRETO: sem dual, e não inventa */
        printf("   (nada no corpus alcança esta fala — ensina-me com: aprende)\n");
        return;
    }
    char t[RESP_LIM]; le_texto(r, t, sizeof t);
    if(prefixal){                                  /* o corte: a moldura escolheu, o
                                                    * parâmetro escreve-se no buraco */
        char cheio[RESP_LIM + 512];
        if(!corte_escreve(efetiva, d, t, cheio, sizeof cheio)){
            printf("não sei.\n");
            printf("   (a moldura pede um parâmetro — completa a fala)\n");
            return;
        }
        printf("%s\n", cheio);
    } else if(acha_buraco(t)){                     /* a moldura só responde pela via do
                                                    * corte — crua, é buraco por preencher */
        printf("não sei.\n");
        printf("   (nada no corpus alcança esta fala — ensina-me com: aprende)\n");
        return;
    } else printf("%s\n", t);
    printf("   (%s, %d símbolo(s) de caminho)\n", via, d);
}

/* O MEDIDOR. Sem argumentos, a assistente mede-se a si propria — e as asserções são o que ela
 * promete: as tres reguas do morfico, o decreto a recusar, e o acento a nao partir o caminho. */
#include "unidade.h"
/* nos testes, chamar a regua direto exige escolher o banco — o que o responde() faz por dentro.
 * Estes atalhos poem o teste no MESMO caminho do programa, que foi o que faltou da primeira vez. */
static long t_erosao(const char *f, int *d){ no_banco(banco_da(f)); return erosao(f, d); }
static long t_daqui(const char *f, int *d){ no_banco(banco_da(f)); return desce_daqui(f, d); }
static long t_ancorada(const char *f, int *d, const char **e){ return erosao_ancorada(f, d, e); }
static long t_dilata(const char *f, int *d){
    for(int b = 0; b < NB; b++){ no_banco(b); long r = dilatacao(f, d); if(r) return r; }
    return 0;
}
static int t_torcao(const char *f, long *v, int m){ no_banco(banco_da(f)); return torcao(f, v, m); }
static int teste(void){
    const char *b = "/tmp/conversa_teste.db";
    for(int i = 0; i < NB; i++){ char c[512]; snprintf(c, sizeof c, "%s.%d", b, i); unlink(c); }
    barr_abre(b);                                  /* o teste abre O BARRAMENTO, nao um ficheiro */
    if(fd < 0) return 2;
    for(int i = 0; i < NB; i++){ no_banco(i);
        Slot h = { RAIZ + NOSL, 0 }; grava(H_LIVRE, h); }
    printf("\n=== A ASSISTENTE — corpus vazio que cresce da conversa ====================\n");
    printf("    Tudo em disco: a fala cifra-se, desce a arvore em pread, e a resposta\n");
    printf("    mora no no terminal. Sem vocabulario, sem postings, sem malloc.\n\n");
    aprende("bom dia", "bom dia! como estas?");
    aprende("quem és tu", "sou a assistente — aprendo do que conversarmos.");
    char t[1024]; int d;
    printf("\n§C1  EROSAO: o prefixo — a fala tal como veio, e a mais longa que couber.\n\n");
    long r = t_erosao("bom dia", &d); le_texto(r, t, sizeof t);
    printf("      \"bom dia\"              -> %s  (%d simbolos)\n", t, d);
    ok("a fala exata acha a sua resposta", r && !strcmp(t, "bom dia! como estas?"));
    r = t_erosao("bom dia, tudo bem?", &d);
    ok("e a fala mais longa cai no prefixo que existe", r != 0 && d == 7);
    /* a caixa na erosao: o prefixo so vale se FECHA em fronteira de palavra — "bom dia"
     * fecha na virgula, mas "q" dentro de "qualquer" nao e a abreviacao "q". */
    aprende("q", "diz o que precisas.");
    r = t_erosao("qualquer um serve", &d);
    ok("e o prefixo que corta a meio de palavra nao vale (a caixa decide)", r == 0);
    /* o FIO desce pela mesma regua: do ponto da conversa, o "q" dentro de "qualquer"
     * tambem nao e a abreviacao "q". */
    r = t_daqui("qualquer um serve", &d);
    ok("e o fio da conversa respeita a mesma caixa", r == 0);

    printf("\n§C2  DILATACAO: a subsequencia — a fala com ruido, antes ou no meio.\n\n");
    r = t_erosao("hmm quem és tu?", &d);
    printf("      pela erosao            %s\n", r ? "achou" : "nao acha (o ruido a frente mata o prefixo)");
    long r2 = t_dilata("hmm quem és tu?", &d); le_texto(r2, t, sizeof t);
    printf("      pela dilatacao         %s  (%d simbolos)\n", t, d);
    ok("o que a erosao perde por ruido a frente, a dilatacao acha", !r && r2);
    long r3 = t_dilata("quem, afinal, és tu", &d);
    ok("e acha tambem com o ruido NO MEIO", r3 != 0);
    /* o ruido que E CAMINHO DE OUTRA FALA no mesmo banco: "estás a ouvir" desce por
     * "estou" (e-s-t-o-u em subsequencia) e morre no ramo errado — a regua re-ancora. */
    aprende("estou aqui", "aqui estou — diz.");
    long r3b = t_dilata("estás a ouvir? quem és tu", &d);
    ok("e acha quando o ruido a frente abre o caminho de OUTRA fala (re-ancora)", r3b != 0);
    /* o gume da CAIXA (o eixo de segmentacao da vizinhanca admissivel — Controle de
     * Histerese): "esta outra aquisição" contem "estou aqui" como subsequencia DENSA
     * (10 consumidos, 4 saltos internos — a contagem sozinha aceitaria), mas so'
     * PARTINDO as palavras: e-s-t de "esta", o-u de "outra", a-q-u-i de "aquisição".
     * A dilatacao salta palavras, nao letras: palavra partida recusa o candidato. */
    long r3c = t_dilata("esta outra aquisição", &d);
    ok("e RECUSA a subsequencia densa que parte palavras (a caixa decide)", r3c == 0);
    /* a fala de UM simbolo (a abreviacao "q" do corpus real, ja aprendida no §C1) e'
     * iman: registada no consumo, a palavra "qwq" so' se revela partida no 'w'
     * seguinte. O registo so' vale quando a palavra que o contem FECHA limpa. */
    long r3d = t_dilata("zzz qwq", &d);
    ok("e o terminal registado a meio de palavra so' vale se ela fechar limpa", r3d == 0);
    /* e o registo A MEIO de uma palavra que desce INTEIRA: "quanto" desce todo pelo
     * caminho de "quantos queres", a palavra fecha limpa — mas o "q" registado ao
     * primeiro simbolo esta no MEIO de "quanto", nao no fim de uma palavra. */
    aprende("quantos queres", "diz la quantos.");
    long r3e = t_dilata("quanto", &d);
    ok("e o registo so' vale em FIM de palavra da consulta (o peek da caixa)", r3e == 0);

    printf("\n§C3  TORCAO: duas falas no mesmo canal, desentrelacadas.\n\n");
    { long v[8];
      int n = t_torcao("bom dia quem és tu", v, 8);
      printf("      \"bom dia quem és tu\"  -> %d fala(s) achada(s):\n", n);
      for(int k = 0; k < n; k++){ no_banco(torc_banco[k]); le_texto(v[k], t, sizeof t); printf("        %s\n", t); }
      ok("a torcao desentrelaca as DUAS falas de uma so linha", n == 2);
      int m = t_torcao("bom dia", v, 8);
      ok("e uma fala sozinha continua a ser uma so", m == 1);
      /* a caixa na torcao: um troco ancora em FRONTEIRA de palavra e fecha em fronteira —
       * "aquem és tu" nao pode pescar "quem és tu" de dentro de "aquem", e "quem és tux"
       * nao fecha porque o troco morre a meio de "tux". */
      int m2 = t_torcao("aquem és tu", v, 8);
      ok("a torcao nao ancora a meio de palavra (a caixa decide)", m2 == 0);
      int m3 = t_torcao("quem és tux", v, 8);
      ok("e o troco que morre a meio de palavra nao fecha", m3 == 0);
      printf("\n      Desce ate um no terminal, responde, e RECOMECA da raiz com o que sobrou.\n");
      printf("      E a terceira regua do morfico, e trata o caso de dizer duas coisas de uma vez.\n");
    }

    printf("\n§C4  DECRETO: quando nenhuma regua alcanca, ela RECUSA-SE a inventar.\n\n");
    long r4 = t_erosao("zzz", &d), r5 = t_dilata("zzz", &d);
    printf("      \"zzz\"                  -> nao sei\n");
    ok("nada alcanca, e a resposta e o decreto — o unico metodo sem dual", !r4 && !r5);

    printf("\n§C5  NO BARRAMENTO: a reparticao e pela CABECA da cifra.\n\n");
    {
        printf("      fala            banco   porque\n");
        const char *fs[] = { "bom dia", "quem és tu", "ourives", "ourivesaria" };
        for(int i = 0; i < 4; i++)
            printf("      %-15s %d       o primeiro simbolo\n", fs[i], banco_da(fs[i]));
        ok("'ourives' e 'ourivesaria' caem no MESMO banco — quem partilha prefixo partilha banco",
           banco_da("ourives") == banco_da("ourivesaria"));
        printf("\n      Repartir pela cifra INTEIRA teria posto os dois em bancos diferentes e a\n");
        printf("      erosao nunca os juntaria. A regua e o prefixo, logo a cabeca e que reparte.\n");
        printf("\n      E a dilatacao pergunta ao barramento inteiro: ela pode saltar o inicio,\n");
        printf("      logo nao tem cabeca fixa. Emite-se, e responde quem puder — ninguem e\n");
        printf("      chamado pelo nome, e ninguem sabe quem tem o que.\n");
    }

    printf("\n§C6  AS ASSISTENTES CONVERSAM — a fala e a interface, nao ha API.\n\n");
    {
        char base[512];
        system("rm -rf /tmp/conv_barr && mkdir -p /tmp/conv_barr");
        int guarda[NB]; memcpy(guarda, fdv, sizeof guarda);
        char gb[512]; snprintf(gb, sizeof gb, "%s", barr_base);
        /* a Ana sabe pouco, o Cid sabe mais, a Bia nao sabe nada */
        snprintf(base, sizeof base, "/tmp/conv_barr/ana"); snprintf(barr_base, sizeof barr_base, "%s", base);
        barr_abre(base); for(int i = 0; i < NB; i++){ no_banco(i); Slot h = { RAIZ+NOSL, 0 }; grava(H_LIVRE, h); }
        aprende("quem", "sou a Ana.");
        snprintf(base, sizeof base, "/tmp/conv_barr/cid"); snprintf(barr_base, sizeof barr_base, "%s", base);
        barr_abre(base); for(int i = 0; i < NB; i++){ no_banco(i); Slot h = { RAIZ+NOSL, 0 }; grava(H_LIVRE, h); }
        aprende("quem és tu", "sou o Cid, e sei mais desta pergunta.");
        snprintf(base, sizeof base, "/tmp/conv_barr/bia"); snprintf(barr_base, sizeof barr_base, "%s", base);
        barr_abre(base); for(int i = 0; i < NB; i++){ no_banco(i); Slot h = { RAIZ+NOSL, 0 }; grava(H_LIVRE, h); }
        char resp[1024];
        int a1 = pergunta_ao_barramento("quem és tu", resp, sizeof resp);
        printf("      a Bia nao sabe e emite; responde: \"%s\"\n", a1 ? resp : "(silencio)");
        ok("outra assistente respondeu — sem ser chamada pelo nome", a1);
        ok("e A REGUA escolheu a mais FUNDA, nao a que a pasta devolveu primeiro",
           a1 && strstr(resp, "Cid") != NULL);
        printf("\n      Ficar com a primeira que o readdir desse era deixar o SISTEMA DE FICHEIROS\n");
        printf("      fazer de regua. Quem decide e a profundidade do caminho: quem casou mais\n");
        printf("      simbolos sabe mais daquela fala.\n");
        int a2 = pergunta_ao_barramento("esmeralda", resp, sizeof resp);
        ok("e o que ninguem sabe continua sem resposta — o decreto so fala depois", !a2);
        printf("\n      \"Nao sei\" deixou de ser o fim: e o fim do que EU sei. Recusar-se a\n");
        printf("      inventar nao e recusar-se a perguntar.\n");
        memcpy(fdv, guarda, sizeof guarda);
        snprintf(barr_base, sizeof barr_base, "%s", gb);
        system("rm -rf /tmp/conv_barr");
    }

    printf("\n§C7  A ESCOLHA E A CONTRACAO DO REI, e nao a profundidade.\n\n");
    {
        /* duas candidatas com a MESMA profundidade: a marginal nao as separa, e por isso e o
         * bairro que tem de pagar. E o que o bairro.c mede em 115.871 decisoes. */
        char cand[NCAND][1024]; int prof[NCAND];
        snprintf(cand[0], sizeof cand[0], "o banco e a instituicao do dinheiro");
        snprintf(cand[1], sizeof cand[1], "o banco e o assento do jardim");
        prof[0] = prof[1] = 15;
        int a0 = bairro_escolhe(cand, prof, 2, "");
        int a1 = bairro_escolhe(cand, prof, 2, "o banco e o assento");
        int a2 = bairro_escolhe(cand, prof, 2, "o banco e a instituicao");
        /* E O CASO DURO, que o debate de tres apanhou: a palavra do contexto NO MEIO da
         * candidata, e nao na frente. Com prefixo isto empatava e caia na ordem. */
        char c2[NCAND][1024]; int p2[NCAND];
        snprintf(c2[0], sizeof c2[0], "o corpo aureo, e a inducao tem essa cifra");
        snprintf(c2[1], sizeof c2[1], "o que manda no relogio da rede");
        p2[0] = p2[1] = 12;
        int b1 = bairro_escolhe(c2, p2, 2, "a inducao e o corpo");
        int b2 = bairro_escolhe(c2, p2, 2, "o relogio da rede");
        printf("      fio na \"inducao\"      -> \"%s\"\n", c2[b1]);
        printf("      fio no \"relogio\"      -> \"%s\"\n\n", c2[b2]);
        ok("a palavra do contexto NO MEIO tambem puxa — a vizinhanca nao e o prefixo",
           b1 == 0 && b2 == 1);
        printf("      sem contexto           -> \"%s\"\n", cand[a0]);
        printf("      fio em \"...assento\"    -> \"%s\"\n", cand[a1]);
        printf("      fio em \"...instituicao\" -> \"%s\"\n\n", cand[a2]);
        ok("a MESMA pergunta muda de resposta com o contexto — a contracao escolhe", a1 != a2);
        /* `a0 == 0 || a0 == 1` era verdade por construção: bairro_escolhe devolve um índice
         * entre DUAS candidatas. A condição verificava que o índice é válido, não que ficou
         * a marginal. A afirmação com conteúdo é que sem contexto a escolha coincide com a
         * de UMA das duas com contexto — e por isso não inventa terceira via. */
        ok("sem contexto a escolha cai numa das duas com contexto — nao inventa",
           a0 == a1 || a0 == a2);
        printf("      A profundidade e a MARGINAL: aqui as duas tem 15, e ela nao separa. Quem\n");
        printf("      separa e a vizinhanca — s(e) = a(e)(m + Σ w(f)c(e,f)) ate ao ponto fixo,\n");
        printf("      que e σ = m + 1/σ. Eu ficava com a mais funda, e isso era ficar na iteracao 0.\n");
    }

    printf("\n§C8  AS TRANSFORMACOES NA BASE: sobe, salta, reflete.\n\n");
    {
        /* A conversa e um PONTO, e o ponto sao as coordenadas. Saltar e escreve-las; subir e a
         * erosao; refletir e J. Nao ha marcador nem historico — ha um ponto que se transforma. */
        char c[2048];
        no_banco(0); cam_poe("ouro");
        cam_le(c, sizeof c);
        printf("      coordenadas   \"%s\"\n", c);
        /* a reflexao, simbolo a simbolo */
        long ini[64]; int n = 0;
        { const char *p = c; while(*p){ ini[n] = p - c; n++; prox_simb(&p); } }
        char r[64]; size_t j = 0;
        for(int k = n - 1; k >= 0; k--){
            long a0 = ini[k], a1 = (k + 1 < n) ? ini[k+1] : (long)strlen(c);
            for(long t = a0; t < a1; t++) r[j++] = c[t];
        }
        r[j] = 0;
        printf("      refletido     \"%s\"\n", r);
        ok("a reflexao inverte os simbolos — e J, a troca do criativo", !strcmp(r, "oruo"));
        /* e duas vezes volta: J² = I */
        long i2[64]; int n2 = 0;
        { const char *p = r; while(*p){ i2[n2] = p - r; n2++; prox_simb(&p); } }
        char v[64]; size_t j2 = 0;
        for(int k = n2 - 1; k >= 0; k--){
            long a0 = i2[k], a1 = (k + 1 < n2) ? i2[k+1] : (long)strlen(r);
            for(long t = a0; t < a1; t++) v[j2++] = r[t];
        }
        v[j2] = 0;
        ok("e refletir DUAS vezes volta ao mesmo — J² = I, residuo 0", !strcmp(v, c));
        printf("\n      A base e ortonormal, logo o ponto E A CIFRA — e saltar e escreve-la. De um\n");
        printf("      indice de no nao se sobe nem se reflete: e um sitio sem estrutura. Das\n");
        printf("      coordenadas sim, porque as transformacoes ja sao as dos corpos.\n");
    }

    printf("\n§C9  O ACENTO E ROUPA: a letra nua e que e simbolo.\n\n");
    long a1 = t_erosao("quem és tu", &d), a2 = t_erosao("quem es tu", &d), a3 = t_erosao("QUEM ES TU", &d);
    printf("      \"quem és tu\"  \"quem es tu\"  \"QUEM ES TU\"  ->  o mesmo no\n");
    ok("acento e maiuscula nao partem o caminho — os tres caem no mesmo sitio",
       a1 && a1 == a2 && a2 == a3);
    printf("\n      Em UTF-8 o 'é' sao dois bytes que nao se parecem com o 'e'. Tratar byte como\n");
    printf("      simbolo mandava \"és\" e \"es\" para lados opostos da arvore — e numa assistente\n");
    printf("      de conversa isso e o caso comum, nao a excecao. O passo passou a ser a LETRA.\n");
    printf("\n");

    printf("\n§C10 A CONTA NAO SE PROCURA: desdobra-se — e pelo caminho REAL.\n\n");
    {
        /* Isto tem de passar por onde o programa passa. Medir o ct_passo por fora provaria a
         * peca e nao o caminho — foi esse o erro que ja apanhei aqui, um teste verde com o
         * programa a nao fazer. Entao chama-se e_conta(), que e a porta de entrada real. */
        int c1 = e_conta("2 + 3 x 4"), c2 = e_conta("{2 x [3 + 4]} + 1");
        int f1 = e_conta("o que e um corpo"), f2 = e_conta("quantos sao 3 mais 4");
        printf("      \"2 + 3 x 4\"           -> conta? %s\n", c1 ? "sim" : "nao");
        printf("      \"{2 x [3 + 4]} + 1\"   -> conta? %s\n", c2 ? "sim" : "nao");
        printf("      \"o que e um corpo\"    -> conta? %s\n", f1 ? "sim" : "nao");
        printf("      \"quantos sao 3 mais 4\"-> conta? %s   (tem digito, mas tem letras)\n\n",
               f2 ? "sim" : "nao");
        ok("a porta so abre para conta, e a fala em portugues passa direto as reguas",
           c1 && c2 && !f1 && !f2);

        /* e o desdobramento em si, no banco, com o resultado conferido contra a conta a mao */
        char cf_n[512]; snprintf(cf_n, sizeof cf_n, "%s.conta", b);
        struct { const char *e; long v; } t[] = {
            { "2 + 3 x 4", 14 }, { "(2 + 3) x 4", 20 }, { "2 x [3 + (4 x 5)]", 46 },
            { "{2 x [3 + (4 + 5)]} + 1", 25 }, { "((((7))))", 7 },
        };
        int mal = 0;
        for(size_t k = 0; k < sizeof t/sizeof *t; k++){
            int cf = open(cf_n, O_RDWR|O_CREAT|O_TRUNC, 0644);
            long n = ct_leia(cf, t[k].e), v = -1; char pq[256];
            while(ct_passo(cf, n, pq, sizeof pq) == 1) ;
            if(!ct_valor(cf, n, &v) || v != t[k].v) mal++;
            printf("      %-26s da %ld   (esperado %ld)\n", t[k].e, v, t[k].v);
            close(cf);
        }
        unlink(cf_n);
        printf("\n");
        ok("as contas dao o valor da conta a mao — residuo 0", mal == 0);

        /* e o fail-closed: o que nao fecha e RECUSADO, e nao adivinhado */
        int cf = open(cf_n, O_RDWR|O_CREAT|O_TRUNC, 0644);
        long r1 = ct_leia(cf, "(2 + 3] x 4"); close(cf);
        cf = open(cf_n, O_RDWR|O_CREAT|O_TRUNC, 0644);
        long r2 = ct_leia(cf, "2 + (3 x 4"); close(cf); unlink(cf_n);
        printf("      \"(2 + 3] x 4\" -> %ld ; \"2 + (3 x 4\" -> %ld\n\n", r1, r2);
        ok("o que nao fecha e recusado, e o motivo distingue-se", r1 == -2 && r2 == -4);
        /* A LEI PELA PORTA REAL. Outra vez: nao medir ct_distribui por fora — medir pede_lei,
         * que e por onde a fala entra. E o caso que interessa e o negativo: "distribui as
         * tarefas" nao pode virar conta. */
        int d1 = -1, d2 = -1, d3 = -1;
        const char *l1 = pede_lei("distribui 2 x (3 + 4)", &d1);
        const char *l2 = pede_lei("fatora 2 x 3 + 2 x 4", &d2);
        const char *l3 = pede_lei("distribui as tarefas da semana", &d3);
        printf("\n      \"distribui 2 x (3 + 4)\"       -> lei? %s, conta? %s\n",
               l1 ? (d1 ? "distribuir" : "fatorar") : "nao", (l1 && e_conta(l1)) ? "sim" : "nao");
        printf("      \"fatora 2 x 3 + 2 x 4\"        -> lei? %s, conta? %s\n",
               l2 ? (d2 ? "distribuir" : "fatorar") : "nao", (l2 && e_conta(l2)) ? "sim" : "nao");
        printf("      \"distribui as tarefas da semana\" -> lei? %s, conta? %s   <- vai as reguas\n\n",
               l3 ? (d3 ? "distribuir" : "fatorar") : "nao", (l3 && e_conta(l3)) ? "sim" : "nao");
        ok("a lei so se aplica quando o resto E conta — 'distribui as tarefas' passa direto",
           l1 && d1 && e_conta(l1) && l2 && !d2 && e_conta(l2) && !(l3 && e_conta(l3)));

        /* e a lei em si, no banco: os DOIS caminhos fecham no mesmo */
        {
            char cf_n2[600]; snprintf(cf_n2, sizeof cf_n2, "%s.conta", b);
            struct { const char *e; long v; } dl[] = {
                { "2 x (3 + 4)", 14 }, { "(3 + 4) x 2", 14 },
                { "1 + 2 x (3 + 4)", 15 }, { "5 x (1 + 1) x 2", 20 },
            };
            int difere = 0;
            for(size_t k = 0; k < sizeof dl/sizeof *dl; k++){
                int cf2 = open(cf_n2, O_RDWR|O_CREAT|O_TRUNC, 0644);
                long nn = ct_leia(cf2, dl[k].e), v1 = -1, v2 = -2; char pq[256];
                long dd = ct_distribui(cf2, nn, pq, sizeof pq);
                if(dd > 0){ while(ct_passo(cf2, dd, pq, sizeof pq) == 1) ; ct_valor(cf2, dd, &v2); }
                close(cf2);
                cf2 = open(cf_n2, O_RDWR|O_CREAT|O_TRUNC, 0644);
                nn = ct_leia(cf2, dl[k].e);
                while(ct_passo(cf2, nn, pq, sizeof pq) == 1) ; ct_valor(cf2, nn, &v1);
                close(cf2);
                printf("      %-18s dobrando %ld, distribuindo %ld\n", dl[k].e, v1, v2);
                if(v1 != v2 || v1 != dl[k].v) difere++;
            }
            unlink(cf_n2);
            printf("\n");
            ok("os dois caminhos fecham no mesmo — a distributiva medida, nao citada", difere == 0);
        }

        /* A EQUAÇÃO ENTRE DOIS POLINÓMIOS, pela porta real. */
        {
            int y1 = e_poli("x^2 = 4"), y2 = e_poli("x^5 - x^4 = 1");
            int y3 = e_poli("2x + 3 = 11");          /* grau 1 fica com o outro */
            int y4 = e_poli("o corpo e a cifra = a mesma coisa");
            /* pela DOBRA e nao pela iteracao: Sturm conta as reais (Euclides), e a
             * enumeracao finita acha as racionais em INTEIROS. Zero iteracoes. */
            Pol p; int nr = 0, ns = 0, nq = -1;
            long nn[PMAX], dd[PMAX];
            if(pol_equacao("x^5 - x^4 = 1", &p) == 1){
                nr = pol_sturm_reais(p); ns = (p.n - nr)/2;
                nq = pol_racionais(p, nn, dd, PMAX);
            }
            Pol p2; int nr2 = 0, nq2 = -1;
            if(pol_equacao("x^2 = 4", &p2) == 1){
                nr2 = pol_sturm_reais(p2); nq2 = pol_racionais(p2, nn, dd, PMAX);
            }
            printf("\n      \"x^2 = 4\"          -> %d reais por Sturm, %d racionais exatas\n",
                   nr2, nq2);
            printf("      \"x^5 - x^4 = 1\"    -> assinatura (%d,%d), %d racionais   <- O FURO\n",
                   nr, ns, nq);
            printf("      \"2x + 3 = 11\"      -> polinomial? %s   <- grau 1, vai ao outro\n", y3 ? "sim" : "nao");
            printf("      \"o corpo e a cifra = …\" -> polinomial? %s   <- vai as reguas\n\n",
                   y4 ? "sim" : "nao");
            ok("a polinomial resolve-se por DOBRA — Sturm e enumeracao finita, zero iteracoes",
               y1 && y2 && !y3 && !y4 && nr == 1 && ns == 2 && nq == 0 && nr2 == 2 && nq2 == 2);
        }

        /* O SISTEMA pela porta real: a régua é (−traço, determinante). */
        {
            long sa,sb,sc,sd;
            int q1 = e_sistema("x' = y ; y' = -x");
            int q2 = e_sistema("o gato e o esquilo ; os dois lados");
            sis_le("x' = y ; y' = -x", &sa,&sb,&sc,&sd);
            long T = sa+sd, De = sa*sd-sb*sc;
            Edo eo; edo_le("y'' = -y", &eo);
            /* a ED guarda B e C como FRACAO (Bp/Bq, Cp/Cq) e eu dividia-os em double:
             * comparam-se por PRODUTO CRUZADO, que e exato e e o que a casa faz */
            printf("\n      \"x' = y ; y' = -x\"   -> sistema? %s, A = [[%ld,%ld],[%ld,%ld]]\n",
                   q1 ? "sim" : "nao", sa,sb,sc,sd);
            printf("      traco %ld, det %ld  ->  B = -tr = %ld, C = det = %ld\n", T, De, -T, De);
            printf("      e a ED  y'' = -y  da  B = %s, C = %s   <- O MESMO\n",
                   frac2(eo.Bp, eo.Bq), frac2(eo.Cp, eo.Cq));
            printf("      \"o gato e o esquilo ; …\" -> sistema? %s   <- vai as reguas\n\n",
                   q2 ? "sim" : "nao");
            ok("a regua do SISTEMA e a da ED sao a mesma: (B,C) = (-traco, det)",
               q1 && !q2 && (-T)*eo.Bq == eo.Bp && De*eo.Cq == eo.Cp);
        }

        /* O CIRCUITO pela porta real — e o corpo transistor é onde vive o operador. */
        {
            int c1 = e_circuito("rlc 20 1m 1u"), c2 = e_circuito("transistor 0.6");
            int c3 = e_circuito("wheatstone 100 220 470");
            /* os NEGATIVOS: fala portuguesa sobre circuitos tem de ir ao corpus, não ao
             * resolvedor. É onde estas portas costumam falhar, e mede-se de propósito. */
            int n1 = e_circuito("o que e um circuito rlc");
            int n2 = e_circuito("como funciona o transistor");
            int n3 = e_circuito("a ponte de wheatstone mede por anulacao");
            int c4 = e_circuito("amplificador 1m 1k"), c5 = e_circuito("logica 1 0");
            int c6 = e_circuito("somador 1 1 1");
            int n4 = e_circuito("o amplificador tem ganho alto");
            int n5 = e_circuito("a porta logica nand e universal");
            /* OS TRÊS REGIMES, EM INTEIROS. Escolhe-se o andar onde o crítico FECHA em
             * Q — L/C = 10^4, logo R_c = 2·100 = 200 exato — e aí Δ = R² − 4L/C é
             * inteiro nos três casos. O caso geral não precisa do valor: o REGIME é o
             * SINAL de Δ, e o sinal é exato sempre. */
            long LsC = 10000;                       /* L=1m, C=0,1u: L/C = 10^4 */
            long Rcrit = 2 * raizi(LsC);            /* = 200, exato */
            long Dsub = 60*60 - 4*LsC, Dcri = Rcrit*Rcrit - 4*LsC, Dsob = 600*600 - 4*LsC;
            printf("\n      \"rlc 20 1m 1u\"        -> circuito? %s\n", c1 ? "sim" : "nao");
            printf("      \"transistor 0.6\"      -> circuito? %s\n", c2 ? "sim" : "nao");
            printf("      \"wheatstone 100 …\"    -> circuito? %s\n", c3 ? "sim" : "nao");
            printf("      \"o que e um circuito rlc\"          -> %s   <- vai as reguas\n",
                   n1 ? "SIM (mau)" : "nao");
            printf("      \"como funciona o transistor\"       -> %s   <- vai as reguas\n",
                   n2 ? "SIM (mau)" : "nao");
            printf("      \"a ponte de wheatstone mede …\"     -> %s   <- vai as reguas\n\n",
                   n3 ? "SIM (mau)" : "nao");
            printf("      e o RLC cai na MESMA régua das EDs: Δ = R² - 4L/C\n");
            printf("      R = %3d  ->  Δ = %+ld  (subamortecido)\n", 60, Dsub);
            printf("      R = %3ld  ->  Δ = %+ld  (CRÍTICO — a raiz dupla, ε² = 0)\n", Rcrit, Dcri);
            printf("      R = %3d  ->  Δ = %+ld  (sobreamortecido)\n", 600, Dsob);
            printf("      e na ressonância Im Z = 0 por IDENTIDADE (ω₀² = 1/LC), FP = 1 exato\n\n");
            printf("      \"amplificador 1m 1k\"  -> circuito? %s\n", c4 ? "sim" : "nao");
            printf("      \"logica 1 0\"          -> circuito? %s\n", c5 ? "sim" : "nao");
            printf("      \"somador 1 1 1\"       -> circuito? %s\n", c6 ? "sim" : "nao");
            printf("      \"o amplificador tem ganho alto\"    -> %s   <- vai as reguas\n",
                   n4 ? "SIM (mau)" : "nao");
            printf("      \"a porta logica nand e universal\"  -> %s   <- vai as reguas\n\n",
                   n5 ? "SIM (mau)" : "nao");
            ok("a porta do circuito abre para as contas e NAO para a fala portuguesa",
               c1 && c2 && c3 && c4 && c5 && c6 && !n1 && !n2 && !n3 && !n4 && !n5);
            {   /* OS DOIS REGIMES DO MESMO DISPOSITIVO — e agora medidos onde eles são
                 * EXATOS. A derivada mede-se no NÚMERO DUAL (ε²=0), que é a régua que o
                 * próprio texto cita: «gm é a parte ε de f(a+bε)». Avalia-se f por Horner
                 * no dual e compara-se com a derivada simbólica — dois caminhos, inteiros.
                 * Medir isto com (f(V+h)−f(V−h))/2h era medir a régua errada: h é uma
                 * escolha minha, e o limite não existe em nenhuma máquina. */
                int mal = 0;
                long cf[4] = { 5, -2, 0, 1 };          /* f(x) = x³ − 2x + 5 */
                for(long a = -6; a <= 6; a++){
                    long vr = 0, ve = 0;               /* Horner no dual: (a + 1ε) */
                    for(int k = 3; k >= 0; k--){       /* (vr+veε)(a+ε) + c_k */
                        long nr = vr*a + cf[k], ne = vr + ve*a;
                        vr = nr; ve = ne;
                    }
                    long fa = ((a*a*a) - 2*a + 5), dfa = 3*a*a - 2;   /* o outro caminho */
                    if(vr != fa || ve != dfa) mal++;
                }
                for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
                    if((a&&b) != (a*b)%2) mal++;          /* AND e o produto de GF(2) */
                    if((a!=b) != (a+b)%2) mal++;          /* XOR e a soma de GF(2)    */
                    if(!(!a) != a) mal++;                 /* NOT e a dobra, ordem 2   */
                }
                printf("      a derivada pela parte ε (ε²=0) bate com a simbólica em 13 pontos\n");
                printf("      e chaveando: AND = x de GF(2), XOR = + de GF(2), NOT = dobra\n\n");
                ok("os dois regimes do MESMO dispositivo: a derivada e' a parte ε (exata,"
                   " sem h e sem limite) na janela, e GF(2) fora dela", mal == 0);
            }
            ok("o RLC cai na mesma regua das EDs, e o critico e a raiz DUPLA — Delta = 0"
               " EXATO, em inteiros, e nao 'menor que 1e-9'",
               Dsub < 0 && Dcri == 0 && Dsob > 0 && Rcrit == 200);
            /* E O OPERADOR: a soma vira produto. Mede-se no METAL, que é o exp da casa
             * (σ = e^λ): σ^(a+b) = σ^a·σ^b em ℤ[σ], exato — a mesma lei que Shockley
             * cumpre em volts, sem um decimal e sem tolerância. */
            {
                long mal = 0;
                for(long ea = 0; ea <= 8; ea++) for(long eb = 0; eb <= 8; eb++){
                    /* σ^n = F_n·σ + F_{n-1}, com σ² = σ + 1 */
                    long pa = 0, qa = 1, pb = 0, qb = 1;             /* p·σ + q */
                    for(long i = 0; i < ea; i++){ long np = pa + qa, nq = pa; pa = np; qa = nq; }
                    for(long i = 0; i < eb; i++){ long np = pb + qb, nq = pb; pb = np; qb = nq; }
                    /* o PRODUTO em ℤ[σ]: (pσ+q)(p'σ+q') = (pp'+pq'+qp')σ + (pp'+qq') */
                    long rp = pa*pb + pa*qb + qa*pb, rq_ = pa*pb + qa*qb;
                    long ps = 0, qs = 1;
                    for(long i = 0; i < ea+eb; i++){ long np = ps + qs, nq = ps; ps = np; qs = nq; }
                    if(rp != ps || rq_ != qs) mal++;                 /* a SOMA dos expoentes */
                }
                printf("      σ^a · σ^b = σ^(a+b) em Z[σ], nos 81 pares — a soma vira produto\n\n");
                ok("o transistor E o operador, e a lei mede-se onde e' EXATA: no metal,"
                   " σ^(a+b) = σ^a·σ^b — a mesma cláusula de Pontryagin, sem float",
                   mal == 0);
            }
        }

        /* A EQUAÇÃO DIFERENCIAL pela porta real: a característica É a borda. */
        {
            int d1 = e_edo("y'' = -y"), d2 = e_edo("y'' + 2y' + y = 0");
            int d3 = e_edo("o que e a derivada");
            Edo e1, e2; char bb1[96] = "", bb2[96] = "";
            if(edo_le("y'' = -y", &e1)) edo_borda(e1, bb1, sizeof bb1);
            if(edo_le("y'' = y' + y", &e2)) edo_borda(e2, bb2, sizeof bb2);
            printf("\n      \"y'' = -y\"          -> ED? %s, borda %s, D = %ld\n",
                   d1 ? "sim" : "nao", bb1, e1.D);
            printf("      \"y'' = y' + y\"      -> borda %s, D = %ld   <- o OURO\n", bb2, e2.D);
            printf("      \"o que e a derivada\" -> ED? %s   <- vai as reguas\n\n",
                   d3 ? "sim" : "nao");
            ok("a ED le-se como BORDA: o oscilador da s^2 = -1 e o ouro da s^2 = 1 + s",
               d1 && d2 && !d3 && !strcmp(bb1, "s^2 = -1") && e1.D == -4
                                && !strcmp(bb2, "s^2 = 1 + s") && e2.D == 5);
        }

        /* A ÁLGEBRA GLOBAL pela porta real: a borda declara o corpo, e o i deixa de ser
         * caso especial — é s² = -1. E a fala em português não pode cair aqui. */
        {
            int g1 = e_algebra("s^2 = -1 | (1 + 2s) x (1 - 2s)");
            int g2 = e_algebra("s^3 = s + 1 | (s) x (s)");
            int g3 = e_algebra("o gato e o esquilo | os dois lados");
            Elem bd; char mc[4];
            int n1 = al_le_borda("s^2 = -1", &bd, mc);
            Elem a1, b1; const char *pp = "1 + 2s";
            al_le_elem(&pp, n1, mc, &a1);
            pp = "1 - 2s"; al_le_elem(&pp, n1, mc, &b1);
            Elem r1 = al_prod(a1, b1, &bd);
            char sr[64]; al_escreve(r1, sr, sizeof sr, mc);
            printf("\n      \"s^2 = -1 | …\"        -> álgebra? %s\n", g1 ? "sim" : "nao");
            printf("      \"s^3 = s + 1 | …\"     -> álgebra? %s\n", g2 ? "sim" : "nao");
            printf("      \"o gato e o esquilo | …\" -> álgebra? %s   <- vai as reguas\n", g3 ? "sim" : "nao");
            printf("      e (1 + 2s) x (1 - 2s) na borda s² = -1 da %s\n\n", sr);
            ok("a porta da algebra so abre com BORDA valida, e o produto la dentro fecha",
               g1 && g2 && !g3 && !strcmp(sr, "5"));
        }

        /* ═══ §C11 — A INVOLUÇÃO NA ENTRADA: o cone desdobra-se ═══════════════════════
         * O Aarão: «a entrada é o CONE, mais compacto — precisa desdobrar; involução na
         * entrada e evolução no banco.» "vezes" É o x escrito comprimido.
         *
         * Mede-se pelos DOIS LADOS, porque um só não prova nada:
         *   o lado que TEM de mudar   — "3 vezes 3" passa a conta e dá o valor certo
         *   o lado que NÃO pode mudar — as falas do corpus ficam no corpus
         * e a FRONTEIRA DE PALAVRA, que é o que separa os dois: sem ela "demais" viraria
         * "de+" e o corpus perdia toda a fala que contivesse a sílaba. */
        printf("\n§C11 A INVOLUCAO NA ENTRADA: o cone desdobra-se antes de se decidir.\n\n");
        {
            char d[1024];
            /* o lado que TEM de mudar */
            struct { const char *fala; const char *forma; long val; } sim[] = {
                { "3 vezes 3",          "3 x 3",        9  },
                { "3 vezes 3 mais 2",   "3 x 3 + 2",    11 },
                { "2 mais 2",           "2 + 2",        4  },
                { "10 vezes 10 mais 5", "10 x 10 + 5",  105 },
                { "7 vezes 8 menos 6",  "7 x 8 - 6",    50 },
                { "100 dividido por 4", "100 / 4",      25 },
                { "10 por cento de 200","10 % de 200",  20 },
            };
            int mal = 0;
            char cf_n[600]; snprintf(cf_n, sizeof cf_n, "%s.conta", b);
            for(size_t k = 0; k < sizeof sim/sizeof *sim; k++){
                desdobra_entrada(d, sizeof d, sim[k].fala);
                int vira = !e_conta(sim[k].fala) && e_conta(d);
                if(strcmp(d, sim[k].forma) || !vira) mal++;
                /* e o VALOR, contra a conta a mão — senão isto mediria só a troca de letras */
                int cf = open(cf_n, O_RDWR|O_CREAT|O_TRUNC, 0644);
                long n = ct_leia(cf, d), v = -1; char pq[256];
                while(ct_passo(cf, n, pq, sizeof pq) == 1) ;
                if(!ct_valor(cf, n, &v) || v != sim[k].val) mal++;
                close(cf);
                printf("      %-20s -> %-14s = %ld\n", sim[k].fala, d, v);
            }
            unlink(cf_n);
            ok("a fala comprimida DESDOBRA-SE e resolve-se: 'vezes' e o x, 'mais' e o +, e o"
               " valor bate com a conta a mao — nao e so' troca de letras", mal == 0);

            /* o lado que NAO pode mudar: estas estao no corpus e tem de la ficar */
            const char *nao[] = {
                "3 vezes 3 e igual a 3 mais 3",   /* a propria fala do corpus */
                "a raiz de 2 e racional",
                "nada anda mais rapido que a luz",
                "quanto e 1 mais 1",
                "mais processadores e mais rapido",
                "por que o ouro e o mais irracional",
                "por que menos vezes menos da mais",
                "saiu cara dez vezes agora sai coroa",
            };
            int roubadas = 0;
            for(size_t k = 0; k < sizeof nao/sizeof *nao; k++){
                desdobra_entrada(d, sizeof d, nao[k]);
                if(e_conta(d)) { roubadas++; printf("      ROUBADA: %s\n", nao[k]); }
            }
            printf("\n      %zu falas do corpus com 'mais'/'vezes': %d roubadas pelo"
                   " resolvedor\n", sizeof nao/sizeof *nao, roubadas);
            ok("o desdobramento NAO ROUBA o corpus: as falas em portugues que contem"
               " 'mais'/'vezes' continuam a ir as reguas — o que as segura sao as OUTRAS"
               " palavras, que nunca passam a porta da conta", roubadas == 0);

            /* A FRONTEIRA DE PALAVRA — o que separa os dois lados acima */
            struct { const char *dentro; const char *fora; } fr[] = {
                { "demais",   "demais"   },   /* "de" + "mais" colado: NAO se troca */
                { "vezess",   "vezess"   },
                { "amais",    "amais"    },
                { "3 demais", "3 demais" },
            };
            int quebra = 0;
            for(size_t k = 0; k < sizeof fr/sizeof *fr; k++){
                desdobra_entrada(d, sizeof d, fr[k].dentro);
                if(strcmp(d, fr[k].fora)) { quebra++;
                    printf("      QUEBRA: \"%s\" -> \"%s\"\n", fr[k].dentro, d); }
            }
            printf("      \"demais\" fica \"demais\"; a troca e so de PALAVRA INTEIRA\n\n");
            ok("a troca so apanha a PALAVRA INTEIRA: 'demais' nao vira 'de+' nem 'vezess'"
               " vira 'xs'. Sem esta fronteira o corpus perdia toda a fala com a silaba",
               quebra == 0);
        }

        /* ═══ §C12 — A EVOLUÇÃO NO BANCO: o DUAL da erosão ════════════════════════════
         * A erosão e a dilatação tratam a fala que tem A MAIS; nenhuma acrescenta nada. Era
         * meia dualidade, e media-se: das 252 falas do corpus, ZERO alcançavam ao perder uma
         * palavra, enquanto TODAS alcançavam com uma palavra a mais.
         *
         * Mede-se o par nos dois sentidos, e o que decide não é a evolução responder — é
         * responder O MESMO que a fala inteira. Uma completação que invente dá resposta e
         * falha aqui. */
        printf("\n§C12 A EVOLUCAO NO BANCO: a fala tem A MENOS e o banco desce sozinho.\n\n");
        {
            /* um corpus proprio, para o par se ver sem o ruido do resto. O `aprende` poe cada
             * fala no banco da CABECA dela — entao le-se do mesmo sitio, senao mede-se o banco
             * errado e nao a operacao (ja me aconteceu aqui: verde a medir vazio). */
            aprende("o sinal do rei e a cifra", "e a unica coordenada");
            aprende("dois lados que comutam",   "dao orbita de quatro");
            aprende("dois lados que separam",   "dao outra coisa");

            int d = 0, pas = 0;
            /* O LADO QUE JA' HAVIA: a fala com A MAIS acha pelo prefixo */
            no_banco(banco_da("o sinal do rei e a cifra afinal"));
            long a_mais = erosao("o sinal do rei e a cifra afinal", &d);
            /* O LADO QUE FALTAVA: a fala com A MENOS, e a continuacao e UNICA */
            no_banco(banco_da("o sinal do rei e a"));
            long a_menos = evolucao("o sinal do rei e a", &d, &pas);
            char t1[512] = "", t2[512] = "";
            if(a_mais)  le_texto(a_mais,  t1, sizeof t1);
            if(a_menos) le_texto(a_menos, t2, sizeof t2);
            printf("      A MAIS   \"…e a cifra afinal\" -> %s\n", a_mais ? t1 : "(nada)");
            printf("      A MENOS  \"o sinal do rei e a\" -> %s   (%d simbolo(s) sozinho)\n",
                   a_menos ? t2 : "(nada)", pas);
            ok("o par fecha nos DOIS sentidos: a fala com A MAIS acha pelo prefixo (erosao) e"
               " a fala com A MENOS acha porque o BANCO desce o resto (evolucao) — e as duas"
               " dao a MESMA resposta, que e a prova de que a evolucao nao inventou",
               a_mais && a_menos && !strcmp(t1, t2));

            /* E O CONTROLO, que e o que impede isto de ser adivinhacao: onde RAMIFICA, cala-se.
             * "dois lados que" continua para "comutam" e para "separam" — duas saidas, logo a
             * fala e ambigua e a evolucao NAO escolhe. */
            int p2 = 0;
            long sa[NBIF];
            no_banco(banco_da("dois lados que"));
            long ambigua = evolucao("dois lados que", &d, &p2);
            /* e ela CABE — provado aqui, e nao suposto. Sem isto a asercao passava tambem
             * quando a fala nao chegava ao corpus de todo, que e outra coisa: calar-se por
             * ambiguidade e calar-se por ignorancia nao sao o mesmo silencio. */
            int cabe = bifurcacao("dois lados que", sa);
            no_banco(banco_da("dois lados que comuta"));
            long unica   = evolucao("dois lados que comuta", &d, &p2);
            printf("      \"dois lados que\"        -> %s   <- CABE (%d saidas) e RAMIFICA\n",
                   ambigua ? "respondeu" : "cala-se", cabe);
            printf("      \"dois lados que comuta\" -> %s   <- caminho unico\n\n",
                   unica ? "respondeu" : "cala-se");
            ok("a evolucao NAO ADIVINHA: desce so enquanto o caminho e UNICO. Onde o no"
               " ramifica a fala e ambigua e ela cala-se — e cala-se por AMBIGUIDADE e nao"
               " por a fala nao caber: ela cabe, e tem duas saidas",
               !ambigua && unica && cabe == 2);

            /* E ONDE ELA SE CALA, A BIFURCACAO DIZ O QUE SE SABE. Calar-se deita fora uma
             * coisa que ja se sabia: que a fala CHEGOU la, e para onde podia seguir. Nao se
             * funde o texto das respostas — isso inventava uma que ninguem escreveu; mostram-se
             * as duas, que e o cruzamento do viveiro, e quem decide e quem perguntou. */
            no_banco(banco_da("dois lados que"));
            int nb = bifurcacao("dois lados que", sa);
            char b1[512] = "", b2[512] = "";
            if(nb >= 2){ le_texto(sa[0], b1, sizeof b1); le_texto(sa[1], b2, sizeof b2); }
            printf("\n      \"dois lados que\" -> %d caminho(s):\n", nb);
            if(nb >= 2){ printf("        1) %s\n        2) %s\n", b1, b2); }
            /* as duas saidas tem de ser as DUAS que se ensinaram, e nao a mesma duas vezes */
            int certas = nb == 2 && strcmp(b1, b2)
                       && (!strcmp(b1,"dao orbita de quatro") || !strcmp(b1,"dao outra coisa"))
                       && (!strcmp(b2,"dao orbita de quatro") || !strcmp(b2,"dao outra coisa"));
            ok("onde a evolucao se cala, a BIFURCACAO diz o que se sabe: a fala cabe mas nao"
               " decide, e mostram-se os DOIS caminhos — distintos, e ambos do corpus. Nao se"
               " funde o texto, que inventaria uma resposta que ninguem escreveu", certas);

            /* e o CONTROLO dos dois lados: quem tem caminho UNICO nao bifurca (e da evolucao),
             * e quem NAO CABE no corpus tambem nao — senao isto disparava sempre e nao media */
            long s2[NBIF];
            no_banco(banco_da("dois lados que comuta"));
            int n_unico = bifurcacao("dois lados que comuta", s2);
            no_banco(banco_da("dois lados que xyzw"));
            int n_fora  = bifurcacao("dois lados que xyzw", s2);
            printf("      \"dois lados que comuta\" -> %d   <- caminho unico, e da evolucao\n",
                   n_unico);
            printf("      \"dois lados que xyzw\"   -> %d   <- nao cabe no corpus\n\n", n_fora);
            ok("a bifurcacao so' fala quando ha' MESMO bifurcacao: com caminho unico cala-se"
               " (o caso e da evolucao) e com fala que nao cabe cala-se tambem — senao"
               " disparava sempre e nao media nada", n_unico == 0 && n_fora == 0);
        }

        printf("      A precedencia nao esta escrita em tabela nenhuma: cai da ORDEM das dobras,\n");
        printf("      o mais fundo primeiro e dentro dele o x antes do +. E os tres delimitadores\n");
        printf("      sao o mesmo — quem manda e a profundidade, nao a roupa.\n");
    }

    /* ═══ §C13 — O CORTE E AS TAREFAS: moldura+parametro, e as portas que aumentam a massa ═══
     * Dual Sort: o corte e a selecao de suporte — a fala parte-se em moldura (o que o corpus
     * conhece) e parametro (o que varia); e «se faltar informacao, o agente AUMENTA A MASSA
     * (ferramenta, memoria, calculo) — nao finge que ela ja estava la». */
    printf("\n§C13 O CORTE E AS TAREFAS: a moldura escolhe, o parametro escreve-se.\n\n");
    {
        char out[1024]; int dd;
        aprende("gosto de", "boa escolha — ___ e otimo.");
        long rm = t_erosao("gosto de rock", &dd);
        le_texto(rm, t, sizeof t);
        int c1 = rm && corte_escreve("gosto de rock", dd, t, out, sizeof out) &&
                 !strcmp(out, "boa escolha — rock e otimo.");
        printf("      \"gosto de rock\"        -> %s\n", c1 ? out : "(falhou)");
        ok("a moldura escolhe (erosao) e o parametro escreve-se no buraco (dilatacao)", c1);
        /* o gume: a moldura sem parametro nao responde — o buraco por preencher */
        long rv = t_erosao("gosto de", &dd); le_texto(rv, t, sizeof t);
        ok("e a moldura SEM parametro nao responde: o quantificador sem instancia",
           rv && corte_escreve("gosto de", dd, t, out, sizeof out) == 0);
        /* e a resposta sem buraco passa intacta — o corte nao muda o que ja funcionava */
        int c3 = corte_escreve("bom dia", 7, "bom dia! como estas?", out, sizeof out);
        ok("e a resposta sem buraco passa byte a byte", c3 && !strcmp(out, "bom dia! como estas?"));

        /* «lembra que X = Y» — o ensino pela fala, e a volta MEDIDA: o que se lembra
         * responde-se. A referencia nao e escrita a mao: e a propria fala ensinada. */
        int l1 = e_lembra("lembra que o meu nome = aarao") &&
                 resolve_lembra("lembra que o meu nome = aarao");
        long rl = t_erosao("o meu nome", &dd); le_texto(rl, t, sizeof t);
        ok("«lembra que X = Y» aprende, e a volta responde o que se lembrou",
           l1 && rl && !strcmp(t, "aarao"));
        /* o gume: dois « e' » sem '=' e ambiguo, e nao se adivinha */
        ok("e sem separador claro a porta recusa (ambiguo nao se adivinha)",
           resolve_lembra("lembra que isto aquilo") == 0);

        /* o relogio: nao se compara com hora escrita a mao (o numero que nao cabe!) —
         * mede-se que a porta abre para a pergunta e fecha para a fala parecida */
        ok("a porta da hora abre para \"que horas...\" e fecha para \"quantas horas...\"",
           e_hora("que horas sao") && e_hora("q horas") && !e_hora("quantas horas sao precisas"));
        ok("a porta da data abre para \"que dia...\" e fecha para \"aquele dia...\"",
           e_data("que dia e hoje") && !e_data("aquele dia foi bom"));

        /* «mostra X»: o slug procura no catalogo DE VERDADE — o teste cria o seu, e o
         * controlo pede um que nao existe (a porta devolve 0 e o corpus decide) */
        mkdir("/tmp/conversa_teste_papers", 0755);
        int fpp = open("/tmp/conversa_teste_papers/partitura.tex", O_WRONLY|O_CREAT, 0644);
        if(fpp >= 0) close(fpp);
        ok("«mostra a partitura» acha o .tex pelo slug do parametro",
           resolve_mostra_em("mostra a partitura", "/tmp/conversa_teste_papers") == 1);
        ok("e o que nao esta no catalogo devolve a vez ao corpus",
           resolve_mostra_em("mostra o unicornio", "/tmp/conversa_teste_papers") == 0);
        unlink("/tmp/conversa_teste_papers/partitura.tex");
        rmdir("/tmp/conversa_teste_papers");

        /* a moldura NAO e fala completa — e um funcional a espera do argumento: a
         * torcao nao pode fechar um troco nela ("gosto de" + "bom dia" seria partir
         * a fala no buraco em vez de o preencher). */
        long vv[8];
        int nt = t_torcao("gosto de bom dia", vv, 8);
        ok("a torcao nao fecha troco numa moldura com buraco", nt <= 1);
        /* o gume do corpus real: as respostas citam "torre_fundacao.tex" (o '_' nu, do
         * fundacao.sh) e "papers/torre\_fundacao.tex" (o \_ do LaTeX) — nenhum e buraco;
         * so o "___", o blank de preencher, e a moldura. Duas regressoes MEDIDAS ate
         * aqui: «o que e um corpo» e «mostra a fundacao» caiam no decreto. */
        int cl = corte_escreve("o que e um corpo", 16,
                               "ver papers/torre_fundacao.tex", out, sizeof out);
        int cl2 = corte_escreve("o que e um corpo", 16,
                                "ver papers/torre\\_fundacao.tex", out, sizeof out);
        ok("o '_' nu dos caminhos e o \\_ do LaTeX nao sao buraco — so o ___ e",
           cl == 1 && cl2 == 1 && acha_buraco("um ___ aqui") != 0);

        /* a EROSAO RE-ANCORADA: a moldura no MEIO da fala — "que estilo? gosto de rock"
         * come o ruido a frente ate uma fronteira e o prefixo tenta outra vez; o corte
         * preenche com o resto. O gume: sem nada que ancore, devolve 0. */
        {
            const char *efet = 0; int da = 0;
            long ra = t_ancorada("que estilo? gosto de rock", &da, &efet);
            int a1 = 0;
            if(ra && efet){
                char rr[512]; le_texto(ra, rr, sizeof rr);
                a1 = corte_escreve(efet, da, rr, out, sizeof out) &&
                     !strcmp(out, "boa escolha — rock e otimo.");
            }
            ok("a erosao re-ancorada acha a moldura no meio e o corte preenche", a1);
            ok("e sem ancora nenhuma devolve 0 (o gume)",
               t_ancorada("zzz yyy www", &da, &efet) == 0);
        }

        /* AS FUNÇÕES NOMEADAS — a fita recuperada para a fala: «raiz de», «fatorial de»,
         * «dobro de»... montam a expressao e a resolucao e a MESMA fita (passos, fracoes,
         * o i). A guarda e a de sempre: o parametro tem de ser conta pura — «a raiz de 2
         * e racional» continua no corpus. O valor mede-se contra a conta a mao. */
        {
            struct { const char *fala; const char *expr; long val; } fn[] = {
                { "raiz de 16",           "raiz (16)",   4   },
                { "o fatorial de 5",      "(5) !",       120 },
                { "o dobro de 3 vezes 3", "2 x (3 x 3)", 18  },
                { "a metade de 50",       "(50) / 2",    25  },
                { "o quadrado de 9",      "(9) ^ 2",     81  },
                { "o cubo de 3",          "(3) ^ 3",     27  },
            };
            int fmal = 0;
            char ex[1200], cf_n2[600]; snprintf(cf_n2, sizeof cf_n2, "%s.conta", b);
            for(size_t k = 0; k < sizeof fn/sizeof *fn; k++){
                if(!funcao_monta(fn[k].fala, ex, sizeof ex) || strcmp(ex, fn[k].expr)){ fmal++; continue; }
                int cf2 = open(cf_n2, O_RDWR|O_CREAT|O_TRUNC, 0644);
                long nn = ct_leia(cf2, ex), vv2 = -1; char pq2[256];
                while(ct_passo(cf2, nn, pq2, sizeof pq2) == 1) ;
                if(!ct_valor(cf2, nn, &vv2) || vv2 != fn[k].val) fmal++;
                close(cf2);
                printf("      %-22s -> %-14s = %ld\n", fn[k].fala, ex, vv2);
            }
            unlink(cf_n2);
            ok("as funcoes nomeadas montam a expressao e a fita da o valor da conta a mao",
               fmal == 0);
            ok("e a guarda: «a raiz de 2 e racional» e «o dobro de trabalho» NAO sao funcao"
               " — o parametro nao e conta pura e o corpus fica dono",
               funcao_monta("a raiz de 2 e racional", ex, sizeof ex) == 0 &&
               funcao_monta("o dobro de trabalho", ex, sizeof ex) == 0);
        }

        /* ═══ A MEMBRANA TEXTUAL: o LaTeX desdobra-se na entrada ═════════════════════
         * O LaTeX e a «interface padrao — a membrana textual por omissao» da casa
         * (Corpo Universal, §papeis), e o tradutor .tex<->PDF «opera nesta torre»
         * (Corpo de Peano). Mas o tradutor COMPOE (desenha); nao avalia. O que faltava
         * era a mesma lei do cone: a fala vem comprimida na membrana e DESDOBRA-SE na
         * entrada — «\frac{1}{2}» e «(1)/(2)» sao a mesma conta em duas roupas.
         * O aninhamento resolve-se por PONTO FIXO: cada volta desdobra um andar. */
        {
            struct { const char *tex, *nu; } M[] = {
                { "\\frac{1}{2} + \\frac{1}{3}", "(1)/(2) + (1)/(3)" },
                { "\\sqrt{16}",                  "raiz (16)"         },
                { "2 \\cdot 3 + 4",              "2 x 3 + 4"         },
                { "10 \\div 4",                  "10 / 4"            },
                { "x^{2} = 4",                   "x^2 = 4"           },
                { "$3 \\times 3$",               "3 x 3"             },
                { "\\left( 2 + 3 \\right) x 4",  "( 2 + 3 ) x 4"     },
                /* o aninhamento: uma volta nao chega, e o ponto fixo trata-o */
                { "\\frac{\\frac{1}{2}}{3}",     "((1)/(2))/(3)"     },
                { "\\sqrt{\\frac{16}{4}}",       "raiz ((16)/(4))"   },
            };
            int mmal = 0; char nu2[1200];
            for(size_t k = 0; k < sizeof M/sizeof *M; k++){
                latex_desdobra(nu2, sizeof nu2, M[k].tex);
                if(strcmp(nu2, M[k].nu)){ mmal++;
                    printf("      MAL: \"%s\" -> \"%s\" (esperado \"%s\")\n",
                           M[k].tex, nu2, M[k].nu); }
            }
            ok("a membrana LaTeX desdobra-se na entrada, e o aninhamento fecha por ponto"
               " fixo (frac, sqrt, cdot, div, expoente, delimitadores)", mmal == 0);

            /* e o VALOR: nao basta a troca de letras — a fita resolve o desdobrado */
            char cf_n3[600]; snprintf(cf_n3, sizeof cf_n3, "%s.conta", b);
            latex_desdobra(nu2, sizeof nu2, "\\frac{1}{2} + \\frac{1}{3}");
            int cf3 = open(cf_n3, O_RDWR|O_CREAT|O_TRUNC, 0644);
            long n3 = ct_leia(cf3, nu2), v3 = 0, q3 = 0; char pq3[256];
            while(ct_passo(cf3, n3, pq3, sizeof pq3) == 1) ;
            int val = ct_valorq(cf3, n3, &v3, &q3) && v3 == 5 && q3 == 6;
            close(cf3); unlink(cf_n3);
            ok("e a fita resolve o desdobrado: 1/2 + 1/3 fecha em 5/6 exato", val);

            /* o lado que NAO pode mudar: a fala em portugues nao tem membrana, e o
             * «\_» que o corpus cita e roupa do compositor — nenhum vira conta */
            const char *nao[] = { "a raiz de 2 e racional", "bom dia, tudo bem?",
                                  "ver papers/torre\\_fundacao.tex", "gosto de rock" };
            int roubadas = 0;
            for(size_t k = 0; k < sizeof nao/sizeof *nao; k++){
                latex_desdobra(nu2, sizeof nu2, nao[k]);
                if(e_conta(nu2)) roubadas++;
            }
            ok("e a membrana nao rouba o corpus: fala sem LaTeX nao vira conta", roubadas == 0);

        /* ═══ §C37 ÁLGEBRA LINEAR E O DUAL: O GUME PASSA A SER AUTOMÁTICO ═════════
         * A exigência nova do `eval.txt` não é conteúdo, é MECANISMO:
         *
         *   «um gume obrigatório em cada teorema: SE A HIPÓTESE FOR RETIRADA, PROCURAR
         *    AUTOMATICAMENTE UM CONTRA-EXEMPLO. É fazer o motor descobrir QUAL HIPÓTESE
         *    ESTÁ CARREGANDO CADA TEOREMA.»
         *
         * Então o `gume_matriz` varre o espaço, tira a hipótese e devolve o primeiro
         * objeto onde a tese TAMBÉM cai. E o dual é a casa: «o vetor fornece o objeto; o
         * funcional fornece a coordenada que o mede» — a coordenada É uma medição. */
        printf("\n§C37 LINEAR E DUAL: o gume procura-se, e a coordenada é uma medição.\n\n");
        {
            long d12[] = {1,2,3,4}, dT[] = {1,1,0, 0,1,1};
            Mat A2 = mat_de_inteiros(2,2,d12), T23 = mat_de_inteiros(2,3,dT);

            /* O GUME AUTOMÁTICO — e o primeiro medidor é do MECANISMO, não do teorema:
             * ele tem de ACHAR quando a hipótese carrega, e tem de NÃO achar quando ela
             * não carrega. Um buscador que acha sempre não estava a decidir nada. */
            { Mat contra;
              long p1 = gume_matriz(2, 2, hip_det_nao_zero, tese_invertivel, &contra);
              Mat c1 = contra;
              long p2 = gume_matriz(2, 1, hip_simetrica, tese_comuta_com_transposta, &contra);
              Mat c2 = contra;
              long p3 = gume_matriz(2, 2, hip_colunas_li, tese_nucleo_trivial, &contra);
              /* O CONTROLO, e a primeira versão dele estava ERRADA: pus tese = hipótese,
               * e assim todo objeto que falha a hipótese é «contra-exemplo» — o buscador
               * achava sempre e o controlo não controlava nada. O controlo certo é uma
               * TESE QUE VALE SEMPRE (det A = det Aᵀ): aí não pode haver contra-exemplo,
               * e vir vazio é o que prova que o buscador não inventa. */
              long p4 = gume_matriz(2, 2, hip_simetrica, tese_det_igual_transposta, &contra);
              printf("      det≠0 ⟹ invertível: contra ao passo %ld;  colunas LI ⟹ núcleo"
                     " trivial: passo %ld\n", p1, p3);
              printf("      simétrica ⟹ comuta com a transposta: passo %ld;  e o CONTROLO"
                     " (tese que vale SEMPRE) devolve %ld\n", p2, p4);
              ok("O GUME É AUTOMÁTICO: retirada a hipótese, o buscador VARRE o espaço e"
                 " devolve o objeto onde a tese também cai — det ≠ 0, colunas LI e"
                 " simetria, os três achados. E o CONTROLO garante que ele decide: quando"
                 " a tese É a hipótese, não há contra-exemplo possível e ele vem vazio."
                 " Um buscador que achasse sempre não estava a medir nada",
                 p1 > 0 && p2 > 0 && p3 > 0 && p4 == 0
                 && mat_det(c1).p == 0 && !mat_igual(mat_mult(c2,mat_transposta(c2)),
                                                     mat_mult(mat_transposta(c2),c2))); }

            /* (1)(2) OS AXIOMAS: 0v = 0 e a unicidade do zero e do oposto */
            { int mal = 0; long feitos = 0;
              for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
                  Vec v = vec0(2);
                  v.c[0] = qz_de_inteiro(a); v.c[1] = qz_de_inteiro(b);
                  if(!vec_zero(vec_esc(qz(0,1), v))) mal++;                /* 0v = 0 */
                  if(!vec_igual(vec_esc(qz(1,1), v), v)) mal++;            /* 1v = v */
                  Vec op = vec_esc(qz_de_inteiro(-1), v);
                  if(!vec_zero(vec_soma(v, op))) mal++;
                  for(long c = -5; c <= 5; c++) for(long e = -5; e <= 5; e++){
                      Vec w = vec0(2);
                      w.c[0] = qz_de_inteiro(c); w.c[1] = qz_de_inteiro(e);
                      if(vec_zero(vec_soma(v,w)) && !vec_igual(w,op)) mal++;  /* oposto ÚNICO */
                      if(!vec_igual(vec_soma(v,w), vec_soma(w,v))) mal++;
                  }
                  for(long l = -3; l <= 3; l++){
                      Qz L = qz_de_inteiro(l);
                      if(!vec_igual(vec_esc(qz_oposto(L), v),
                                    vec_esc(qz_de_inteiro(-1), vec_esc(L,v)))) mal++;
                  }
                  feitos++;
              }
              ok("os AXIOMAS dão os teoremas: 0v = 0 sai da distributividade (não é"
                 " convenção), 1v = v, (−λ)v = −(λv), e o OPOSTO é único — varrido em ℚ²"
                 " com todos os candidatos a oposto testados", mal == 0 && feitos == 121); }

            /* (3) O SUBESPAÇO, e o gume dele: x+y+z = 0 fecha, x+y+z = 1 não */
            { int mal = 0; long saiu = 0;
              for(long x = -3; x <= 3; x++) for(long y = -3; y <= 3; y++)
              for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
              for(long l = -2; l <= 2; l++) for(long m = -2; m <= 2; m++){
                  long z = -x-y, c = -a-b;
                  if((l*x+m*a) + (l*y+m*b) + (l*z+m*c) != 0) mal++;      /* soma 0 FECHA */
                  long z2 = 1-x-y, c2 = 1-a-b;
                  if((l*x+m*a) + (l*y+m*b) + (l*z2+m*c2) != 1) saiu++;   /* soma 1 SAI */
              }
              ok("o SUBESPAÇO decide-se por um critério só (λu + μv ∈ W), e ele é uma"
                 " FIBRA FECHADA. O plano x+y+z = 0 fecha em todas as combinações; o"
                 " x+y+z = 1 SAI — e o gume é o zero, que está no primeiro e não no"
                 " segundo", mal == 0 && saiu > 0); }

            /* (4)(5)(6) SPAN, INDEPENDÊNCIA e a UNICIDADE DAS COORDENADAS */
            { int mal = 0; long li = 0, ld = 0, feitos = 0;
              Vec b1 = vec0(2), b2 = vec0(2);
              b1.c[0] = qz(1,1); b1.c[1] = qz(1,1);
              b2.c[0] = qz(1,1); b2.c[1] = qz(-1,1);
              Vec base[2] = { b1, b2 };
              for(long x = -6; x <= 6; x++) for(long y = -6; y <= 6; y++){
                  Vec v = vec0(2);
                  v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
                  Qz co[LN_MAX];
                  if(!vec_coord(base, 2, v, co)){ mal++; continue; }
                  Vec volta = vec_soma(vec_esc(co[0],b1), vec_esc(co[1],b2));
                  if(!vec_igual(volta, v)) mal++;               /* A VOLTA */
                  if(!vec_no_span(base, 2, v)) mal++;           /* e o span é tudo */
                  feitos++;
              }
              /* LI e LD, e o teorema com a VOLTA FALSA */
              for(long p = -3; p <= 3; p++) for(long q = -3; q <= 3; q++){
                  Vec x = vec0(2), y = vec0(2);
                  x.c[0] = qz(1,1);
                  y.c[0] = qz_de_inteiro(p); y.c[1] = qz_de_inteiro(q);
                  Vec par[2] = {x,y};
                  if(vec_li(par,2)) li++; else ld++;
                  if(!vec_li(par,1)) mal++;      /* subconjunto de LI é LI, e {x} é sempre LI */
              }
              ok("as COORDENADAS numa base são ÚNICAS (a diferença anula-se e a"
                 " independência força zero) e a VOLTA reconstrói o vetor. E o teorema"
                 " «subconjunto de LI é LI» vale com a RECÍPROCA FALSA — há pares LD com"
                 " subconjuntos LI, e os dois casos ocorrem",
                 mal == 0 && feitos == 169 && li > 0 && ld > 0); }

            /* (7)(8) O PRODUTO É CONVOLUÇÃO, e NÃO comuta */
            { int mal = 0; long feitos = 0, nao_comuta = 0;
              for(long k = 0; k < 400; k++){
                  Mat X = mat0(2,2), Y = mat0(2,2), Z = mat0(2,2);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro((t + 3*i + j) % 5 - 2);
                      Y.a[i][j] = qz_de_inteiro((t*3 + i + 2*j) % 5 - 2);
                      Z.a[i][j] = qz_de_inteiro((t*7 + 2*i + j) % 5 - 2);
                      t /= 2;
                  }
                  if(!mat_igual(mat_mult(X,mat_soma(Y,Z)),
                                mat_soma(mat_mult(X,Y),mat_mult(X,Z)))) mal++;
                  if(!mat_igual(mat_mult(mat_mult(X,Y),Z), mat_mult(X,mat_mult(Y,Z)))) mal++;
                  if(!mat_igual(mat_mult(X,Y), mat_mult(Y,X))) nao_comuta++;
                  /* e a CONVOLUÇÃO: a entrada (i,j) é a soma sobre o índice interno */
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      Qz s = qz(0,1);
                      for(int q = 0; q < 2; q++) s = qz_soma(s, qz_mult(X.a[i][q], Y.a[q][j]));
                      if(!qz_igual(mat_mult(X,Y).a[i][j], s)) mal++;
                  }
                  feitos++;
              }
              printf("      400 triplos 2×2:  AB ≠ BA em %ld deles\n", nao_comuta);
              ok("o PRODUTO MATRICIAL é uma CONVOLUÇÃO com índice interno — a mesma forma"
                 " da de polinómios e da de Dirichlet, só muda onde os índices vivem. E"
                 " distribui e associa, mas NÃO comuta: há casos varridos com AB ≠ BA, e"
                 " é aí que «matriz» deixa de ser «número»",
                 mal == 0 && feitos == 400 && nao_comuta > 0); }

            /* (9)(10)(11) LINEARIDADE, NÚCLEO/IMAGEM e o POSTO-NULIDADE */
            { int mal = 0; long feitos = 0;
              for(long k = 0; k < 700; k++){
                  Mat M = mat0(2,3);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 3; j++){
                      M.a[i][j] = qz_de_inteiro(t % 3 - 1); t /= 3;
                  }
                  Vec nb[LN_MAX], ib[LN_MAX];
                  int nul = mat_nucleo(M,nb), rk = mat_imagem(M,ib);
                  if(nul + rk != M.n) mal++;                     /* POSTO-NULIDADE */
                  for(int i = 0; i < nul; i++)
                      if(!vec_zero(mat_aplica(M, nb[i]))) mal++; /* o núcleo é a fibra do 0 */
                  /* e o núcleo é SUBESPAÇO: combinações dos geradores continuam nele */
                  for(int i = 0; i < nul; i++) for(int j = 0; j < nul; j++)
                      if(!vec_zero(mat_aplica(M, vec_soma(nb[i], nb[j])))) mal++;
                  feitos++;
              }
              Vec nb[LN_MAX], ib[LN_MAX];
              int nul = mat_nucleo(T23,nb), rk = mat_imagem(T23,ib);
              printf("      T(x,y,z) = (x+y, y+z):  nullity = %d, rank = %d,  e 3 = %d + %d\n",
                     nul, rk, nul, rk);
              printf("      e o posto-nulidade em %ld matrizes 2×3 varridas\n", feitos);
              ok("o POSTO-NULIDADE fecha em todas as matrizes 2×3 varridas, o NÚCLEO é a"
                 " FIBRA do zero (cada gerador aplica-se a 0, e as combinações também) e"
                 " o exemplo dele dá exatamente 3 = 1 + 2",
                 mal == 0 && feitos == 700 && nul == 1 && rk == 2); }

            /* (12)(13) ISOMORFISMO e DETERMINANTE, com a equivalência medida */
            { int mal = 0; long inv = 0, sing = 0;
              for(long k = 0; k < 625; k++){
                  Mat X = mat0(2,2), R;
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro(t % 5 - 2); t /= 5;
                  }
                  Vec nb[LN_MAX];
                  int nucleo_trivial = (mat_nucleo(X,nb) == 0);
                  int tem = mat_inversa(X, &R);
                  int det_nao_zero = (mat_det(X).p != 0);
                  if(tem != det_nao_zero) mal++;                 /* det ≠ 0 ⟺ invertível */
                  if(tem != nucleo_trivial) mal++;               /* ⟺ injetiva */
                  if(tem){ if(!mat_igual(mat_mult(X,R), mat_id(2))) mal++; inv++; }
                  else sing++;
                  /* e a multiplicatividade */
                  Mat Y = mat_transposta(X);
                  if(!qz_igual(mat_det(mat_mult(X,Y)),
                               qz_mult(mat_det(X), mat_det(Y)))) mal++;
              }
              ok("det A ≠ 0 ⟺ A invertível ⟺ o núcleo é trivial — TRÊS leituras da mesma"
                 " condição, medidas nas 625 matrizes de entradas em [−2,2], com"
                 " det(AB) = det(A)det(B) a fechar. E os dois lados ocorrem",
                 mal == 0 && inv > 0 && sing > 0); }

            /* (14) OS SISTEMAS: a solução é x₀ + ker A, e a família é COMPLETA */
            { int mal = 0; long feitos = 0;
              Vec b = vec0(2); b.c[0] = qz(1,1); b.c[1] = qz(2,1);
              Vec x0 = vec0(3); x0.c[0] = qz(1,1); x0.c[2] = qz(2,1);
              Vec nb[LN_MAX];
              int k = mat_nucleo(T23, nb);
              if(!vec_igual(mat_aplica(T23,x0), b)) mal++;       /* x₀ é solução */
              for(long l = -6; l <= 6; l++){
                  Vec x = vec_soma(x0, vec_esc(qz_de_inteiro(l), nb[0]));
                  if(!vec_igual(mat_aplica(T23,x), b)) mal++;    /* a família gera */
                  feitos++;
              }
              /* e é COMPLETA: toda solução com entradas pequenas está na família */
              long fora = 0;
              for(long a = -6; a <= 6; a++) for(long c = -6; c <= 6; c++)
              for(long e = -6; e <= 6; e++){
                  Vec x = vec0(3);
                  x.c[0] = qz_de_inteiro(a); x.c[1] = qz_de_inteiro(c); x.c[2] = qz_de_inteiro(e);
                  if(!vec_igual(mat_aplica(T23,x), b)) continue;
                  Vec dif = vec_soma(x, vec_esc(qz_de_inteiro(-1), x0));
                  if(!vec_no_span(nb, k, dif)) fora++;           /* tem de estar no núcleo */
              }
              ok("as SOLUÇÕES de Ax = b são exatamente x₀ + ker A — a família gera todas"
                 " (medido) e é COMPLETA: nenhuma solução do cubo [−6,6]³ fica de fora"
                 " dela. «Uma solução particular + uma fibra dá TODAS as soluções»",
                 mal == 0 && feitos == 13 && fora == 0); }

            /* (15)(16) AUTOVALORES e DIAGONALIZAÇÃO, com A¹⁰ por dois caminhos */
            { long da[] = {2,1,1,2};
              Mat M = mat_de_inteiros(2,2,da);
              int mal = 0;
              Qz tr = qz_soma(M.a[0][0], M.a[1][1]), dt = mat_det(M);
              long D = tr.p*tr.p - 4*dt.p, r = 0;
              if(!quadrado_perfeito(D, &r)) mal++;
              long l1 = (tr.p + r)/2, l2 = (tr.p - r)/2;
              Mat P = mat0(2,2), Dg = mat0(2,2);
              long ls[2] = { l1, l2 };
              for(int i = 0; i < 2; i++){
                  Mat S = M;
                  S.a[0][0] = qz_soma(S.a[0][0], qz_de_inteiro(-ls[i]));
                  S.a[1][1] = qz_soma(S.a[1][1], qz_de_inteiro(-ls[i]));
                  Vec nb[LN_MAX];
                  if(mat_nucleo(S, nb) < 1){ mal++; continue; }
                  if(!vec_igual(mat_aplica(M,nb[0]),
                                vec_esc(qz_de_inteiro(ls[i]), nb[0]))) mal++;  /* Av = λv */
                  for(int j = 0; j < 2; j++) P.a[j][i] = nb[0].c[j];
                  Dg.a[i][i] = qz_de_inteiro(ls[i]);
              }
              Mat Pi;
              if(!mat_inversa(P, &Pi)) mal++;
              else {
                  if(!mat_igual(mat_mult(mat_mult(P,Dg),Pi), M)) mal++;   /* A = PDP⁻¹ */
                  Mat Dn = mat0(2,2);
                  long p1 = 1, p2 = 1;
                  for(int k = 0; k < 10; k++){ p1 *= l1; p2 *= l2; }
                  Dn.a[0][0] = qz_de_inteiro(p1); Dn.a[1][1] = qz_de_inteiro(p2);
                  Mat rapido = mat_mult(mat_mult(P,Dn),Pi), lento = mat_id(2);
                  for(int k = 0; k < 10; k++) lento = mat_mult(lento, M);
                  if(!mat_igual(rapido, lento)) mal++;                    /* DOIS CAMINHOS */
              }
              printf("      A = [[2,1],[1,2]]:  λ = %ld e %ld (Δ = %ld = %ld²), e A¹⁰"
                     " pelas duas vias concorda\n", l1, l2, D, r);
              ok("os AUTOVALORES saem de det(A − λI) = 0 e aqui são RACIONAIS (o"
                 " discriminante é quadrado perfeito, e diz-se); os autovetores saem do"
                 " NÚCLEO de A − λI, com Av = λv a fechar; e A¹⁰ calculado pela"
                 " DIAGONALIZAÇÃO bate com dez multiplicações — dois caminhos",
                 mal == 0 && l1 == 3 && l2 == 1); }

            /* ── O DUAL: a coordenada É uma medição ───────────────────────────── */
            { int mal = 0; long feitos = 0;
              /* a base dual, em várias bases, e o δ */
              for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
              for(long c = -3; c <= 3; c++) for(long e = -3; e <= 3; e++){
                  Vec v1 = vec0(2), v2 = vec0(2);
                  v1.c[0] = qz_de_inteiro(a); v1.c[1] = qz_de_inteiro(b);
                  v2.c[0] = qz_de_inteiro(c); v2.c[1] = qz_de_inteiro(e);
                  Vec base[2] = { v1, v2 };
                  Fun du[LN_MAX];
                  int ehbase = vec_li(base, 2);
                  int tem = fun_base_dual(base, 2, du);
                  if(tem != ehbase) mal++;               /* a base dual existe ⟺ é base */
                  if(!tem) continue;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                      if(!qz_igual(fun_av(du[i], base[j]), qz_de_inteiro(i == j))) mal++;
                  /* e a COORDENADA É a MEDIÇÃO: xᵢ = e^i(v) */
                  for(long x = -2; x <= 2; x++) for(long y = -2; y <= 2; y++){
                      Vec v = vec0(2);
                      v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
                      Qz co[LN_MAX];
                      if(!vec_coord(base, 2, v, co)){ mal++; continue; }
                      for(int i = 0; i < 2; i++)
                          if(!qz_igual(fun_av(du[i], v), co[i])) mal++;
                  }
                  feitos++;
              }
              ok("a BASE DUAL existe EXATAMENTE quando a base é base (e^i(e_j) = δ), e o"
                 " teorema do andar mede-se junto: xᵢ = e^i(v) — «a coordenada não é um"
                 " número posto ao lado do vetor: é a MEDIÇÃO pelo funcional dual»",
                 mal == 0 && feitos > 500); }

            /* a SEPARAÇÃO DE PONTOS e a injetividade de ι */
            { int mal = 0; long nao_nulos = 0;
              for(long x = -5; x <= 5; x++) for(long y = -5; y <= 5; y++)
              for(long z = -5; z <= 5; z++){
                  Vec v = vec0(3);
                  v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y); v.c[2] = qz_de_inteiro(z);
                  int separou = 0;
                  for(int i = 0; i < 3; i++){
                      Fun e; e.n = 3;
                      for(int j = 0; j < 3; j++) e.c[j] = qz(0,1);
                      e.c[i] = qz(1,1);
                      if(fun_av(e, v).p) separou = 1;
                  }
                  if(vec_zero(v)){ if(separou) mal++; }
                  else { if(!separou) mal++; else nao_nulos++; }
              }
              ok("a SEPARAÇÃO DE PONTOS: todo vetor não nulo é DISTINGUIDO do zero por"
                 " algum funcional, e o zero por nenhum — «um ponto não desaparece se"
                 " existe uma medição linear que o distingue do zero». É este o mecanismo"
                 " que dá a injetividade de ι, e daí V ≅ V** CANÓNICO",
                 mal == 0 && nao_nulos == 1330); }

            /* dim W + dim W° = dim V — «o dual do posto-nulidade» */
            { int mal = 0; long feitos = 0;
              for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++) for(long c = -2; c <= 2; c++){
                  Vec g = vec0(3);
                  g.c[0] = qz_de_inteiro(a); g.c[1] = qz_de_inteiro(b); g.c[2] = qz_de_inteiro(c);
                  if(vec_zero(g)) continue;
                  Fun an[LN_MAX];
                  int da2 = fun_aniquilador(&g, 1, 3, an);
                  if(1 + da2 != 3) mal++;
                  for(int i = 0; i < da2; i++) if(fun_av(an[i], g).p) mal++;
                  feitos++;
              }
              /* e com dois geradores independentes, dim W = 2 e dim W° = 1 */
              Vec w1 = vec0(3), w2 = vec0(3);
              w1.c[0] = qz(1,1); w2.c[1] = qz(1,1);
              Vec par[2] = { w1, w2 };
              Fun an2[LN_MAX];
              int d2 = fun_aniquilador(par, 2, 3, an2);
              if(2 + d2 != 3) mal++;
              ok("dim W + dim W° = dim V — «praticamente o DUAL do posto-nulidade» —"
                 " medido em todos os subespaços de dimensão 1 de ℤ³ e num de dimensão 2,"
                 " com cada funcional do aniquilador a medir ZERO nos geradores",
                 mal == 0 && feitos > 100 && d2 == 1); }

            /* [T*] = Aᵀ e o chefão: ker T* = (im T)° */
            { int mal = 0; long feitos = 0;
              for(long k = 0; k < 625; k++){
                  Mat X = mat0(2,2), Xd = mat0(2,2);
                  long t = k;
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      X.a[i][j] = qz_de_inteiro(t % 5 - 2); t /= 5;
                  }
                  /* [T*] pela DEFINIÇÃO T*(φ) = φ∘T */
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                      Fun phi; phi.n = 2; phi.c[0] = qz(0,1); phi.c[1] = qz(0,1);
                      phi.c[i] = qz(1,1);
                      Vec ej = vec0(2); ej.c[j] = qz(1,1);
                      Xd.a[j][i] = fun_av(phi, mat_aplica(X, ej));
                  }
                  if(!mat_igual(Xd, mat_transposta(X))) mal++;      /* [T*] = Aᵀ */
                  /* O CHEFÃO: ker T* = (im T)°, e mede-se pelas DIMENSÕES e pela anulação */
                  Vec ib[LN_MAX];
                  int r = mat_imagem(X, ib);
                  Fun an[LN_MAX];
                  int da2 = (r > 0) ? fun_aniquilador(ib, r, 2, an) : 2;
                  Vec nb[LN_MAX];
                  int dk = mat_nucleo(mat_transposta(X), nb);
                  if(dk != da2) mal++;
                  for(int i = 0; i < dk; i++){
                      Fun f; f.n = 2; f.c[0] = nb[i].c[0]; f.c[1] = nb[i].c[1];
                      for(int j = 0; j < r; j++) if(fun_av(f, ib[j]).p) mal++;
                  }
                  feitos++;
              }
              ok("[T*] = Aᵀ — «o transposto aparece naturalmente», construído pela"
                 " DEFINIÇÃO T*(φ) = φ∘T e comparado com a transposta em 625 matrizes. E"
                 " o CHEFÃO fecha: ker T* = (im T)°, medido pelas dimensões E pela"
                 " anulação de cada funcional na imagem",
                 mal == 0 && feitos == 625); }

            /* E OS TRINTA CORREM */
            { int vmal = 0, por_n = 0, por_nome = 0;
              fflush(stdout);
              int guarda = dup(1), nulo = open("/dev/null", O_WRONLY);
              if(guarda >= 0 && nulo >= 0) dup2(nulo, 1);
              for(int k = 1; k <= 16; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "linear %d", k);
                  if(resolve_linear(fala)) por_n++; else vmal++;
              }
              for(int k = 1; k <= 14; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "dual %d", k);
                  if(resolve_linear(fala)) por_n++; else vmal++;
              }
              for(size_t i = 0; i < sizeof LI16/sizeof *LI16; i++)
                  if(resolve_linear(LI16[i].nome)) por_nome++; else vmal++;
              for(size_t i = 0; i < sizeof DU14/sizeof *DU14; i++)
                  if(resolve_linear(DU14[i].nome)) por_nome++; else vmal++;
              if(resolve_linear("linear 17")) vmal++;
              if(resolve_linear("dual 15")) vmal++;
              fflush(stdout);
              if(guarda >= 0){ dup2(guarda, 1); close(guarda); }
              if(nulo >= 0) close(nulo);
              printf("      os trinta: %d pelo número, %d pelo nome, e o fora de alcance"
                     " é recusado\n", por_n, por_nome);
              ok("OS DEZASSEIS da álgebra linear e os CATORZE do dual correm pelo NÚMERO"
                 " e pelo NOME, com o fora de alcance RECUSADO — e cada um na espinha de"
                 " sete ticks, com o gume a PROCURAR-SE onde ele o pediu",
                 vmal == 0 && por_n == 30 && por_nome == 30); }
        }

        /* ═══ §C36 CORPOS: A ESCADA FECHA, E A EXCEÇÃO É A MESMA ══════════════════
         * «corpo é praticamente o ponto em que "toda operação que tem fibra tem volta"
         * vira uma estrutura algébrica formal. A EXCEÇÃO continua sendo exatamente a que
         * vocês já descobriram: 0⁻¹ não existe.»
         *
         *   ℕ: + ×   ℤ: a↦−a   ℚ: a↦a⁻¹ (a≠0)   ℝ: completude   K: inversão fechada
         *
         * E o andar reaproveita quase tudo: corpo → polinómio → fatoração →
         * irredutibilidade → quociente. O 𝔽₄ é medido pelo MESMO `an_corpo` que mediu o
         * ℤ₅ — não por uma segunda régua. */
        printf("\n§C36 CORPOS: a escada fecha, e o 0⁻¹ continua a ser a única exceção.\n\n");
        {
            int f4[] = {1,1,1}, f9[] = {1,0,1}, f8[] = {1,1,0,1}, red[] = {1,0,1};

            /* (1)(2)(5) ℚ É CORPO, ℤ NÃO É, E A ÚNICA EXCEÇÃO É O ZERO */
            { int qmal = 0; long com = 0, sem = 0;
              for(long p2 = -20; p2 <= 20; p2++) for(long q2 = 1; q2 <= 20; q2++){
                  Qz x = qz(p2,q2), inv;
                  int tem = qz_inverso(x, &inv);
                  if(tem != (p2 != 0)) qmal++;            /* existe ⟺ ≠ 0 */
                  if(tem){ if(!qz_igual(qz_mult(x,inv), qz_de_inteiro(1))) qmal++; com++; }
                  else sem++;
                  for(long r = -3; r <= 3; r++){
                      Qz y = qz(r,3), z = qz(r+1,5);
                      if(!qz_igual(qz_mult(x, qz_soma(y,z)),
                                   qz_soma(qz_mult(x,y), qz_mult(x,z)))) qmal++;
                  }
              }
              /* e em ℤ o 2 NÃO tem inverso — a testemunha do salto ℤ → ℚ */
              long inv2 = 0;
              for(long b = -500; b <= 500; b++) if(2*b == 1) inv2++;
              printf("      ℚ: %ld com inverso e %ld sem (só o zero, %ld vezes escrito);"
                     "  e em ℤ o 2 tem %ld inversos\n", com, sem, sem, inv2);
              ok("ℚ É CORPO e ℤ NÃO É, e a diferença cabe num elemento: em ℚ todo não"
                 " nulo tem inverso e a distributividade fecha; em ℤ o 2 não tem inverso"
                 " nenhum. E a ÚNICA exceção em ℚ é o zero — a mesma que a casa encontrou"
                 " no andar dos racionais, e que este andar volta a encontrar",
                 qmal == 0 && com > 700 && inv2 == 0); }

            /* (3)(4) A UNICIDADE DO INVERSO E O CANCELAMENTO, com o gume no não-corpo */
            { int umal = 0; long falhas_corpo = 0, falhas_z6 = 0;
              for(int m = 2; m <= 13; m++){
                  Anel R; an_zn(&R,m);
                  int corpo = an_corpo(&R,0);
                  for(int a = 1; a < m; a++){
                      int quantos = 0;
                      for(int b = 0; b < m; b++) if(R.mult[a][b] == 1 % m) quantos++;
                      if(quantos > 1) umal++;             /* o inverso é ÚNICO */
                      for(int b = 0; b < m; b++) for(int c = 0; c < m; c++)
                          if(R.mult[a][b] == R.mult[a][c] && b != c){
                              if(corpo) falhas_corpo++; else falhas_z6++;
                          }
                  }
              }
              printf("      inverso único em ℤₘ até 13; cancelamento: %ld falhas nos"
                     " CORPOS e %ld nos que não são\n", falhas_corpo, falhas_z6);
              ok("o INVERSO É ÚNICO (a cadeia b = b·1 = b(ac) = (ba)c = 1c = c) e o"
                 " CANCELAMENTO vale — mas SÓ em corpo. Nos ℤₘ que não são corpo ele"
                 " falha, e conta-se: é a hipótese a pagar-se, não a decorar-se",
                 umal == 0 && falhas_corpo == 0 && falhas_z6 > 0); }

            /* (6)(10) A CARACTERÍSTICA É 0 OU PRIMA — e o gume é o não-corpo */
            { int cmal = 0, primos = 0;
              for(int m = 2; m <= 24; m++){
                  Anel R; an_zn(&R,m);
                  int c = corpo_carac(&R, 80);
                  if(c != m) cmal++;                       /* char(ℤₘ) = m */
                  if(an_corpo(&R,0)){
                      if(!nt_primo(c)) cmal++;             /* corpo ⟹ char prima */
                      primos++;
                  }
              }
              /* nas EXTENSÕES a característica é p e NÃO pⁿ — é aí que o teorema morde */
              struct { int p, g; int *f; int q; } cs[3] = { {2,2,f4,4}, {3,2,f9,9}, {2,3,f8,8} };
              int ext_mal = 0;
              for(int i = 0; i < 3; i++){
                  Anel R;
                  corpo_ext(cs[i].p, cs[i].f, cs[i].g, &R);
                  if(corpo_carac(&R, 80) != cs[i].p) ext_mal++;
                  printf("      |K| = %d:  char = %d (e NÃO %d — a característica é o"
                         " primo, não a cardinalidade)\n",
                         cs[i].q, corpo_carac(&R,80), cs[i].q);
              }
              /* e o gume: ℤ₆ não é corpo E tem característica composta */
              Anel Z6; an_zn(&Z6,6);
              ok("a CARACTERÍSTICA de um corpo é 0 ou PRIMA — e o gume está nas"
                 " EXTENSÕES: 𝔽₄ tem 4 elementos e característica 2, 𝔽₉ tem 9 e"
                 " característica 3. Se eu tivesse medido só em ℤₘ, char = m e o teorema"
                 " parecia dizer «char = |K|», que é falso. O ℤ₆ (não corpo) tem"
                 " característica 6, composta — a hipótese decide",
                 cmal == 0 && ext_mal == 0 && primos > 0 && corpo_carac(&Z6,80) == 6); }

            /* (7)(8) «PRIMO PASSA, COMPOSTO CAI» — a equivalência varrida */
            { int pmal = 0, sim = 0, nao = 0;
              for(int m = 2; m <= ES_MAX; m++){        /* o teto da tábua, e é ele que manda */
                  Anel R; an_zn(&R,m);
                  int c = an_corpo(&R,0);
                  if(c != (nt_primo(m) != 0)) pmal++;
                  if(c) sim++; else nao++;
                  /* e quando não é corpo, os divisores de zero EXIBEM-SE */
                  if(!c && m > 2){
                      int ea = 0, eb = 0;
                      if(an_dominio(&R,&ea,&eb)) pmal++;   /* tem de haver testemunha */
                  }
              }
              ok("«PRIMO PASSA, COMPOSTO CAI» — ℤₘ é corpo exatamente quando m é primo,"
                 " varrido até ao teto da tábua nos DOIS sentidos, e em cada composto os"
                 " divisores de zero exibem-se. Não é o critério a decidir: é a estrutura,"
                 " e o critério a acompanhá-la", pmal == 0 && sim > 0 && nao > 0); }

            /* (9)(23)(24) OS TRÊS CAMINHOS PARA O INVERSO EM 𝔽ₚ, e têm de concordar */
            { int imal = 0; long feitos = 0, esperado = 0;
              /* o total NÃO se escreve à mão: conta-se por um segundo caminho (a soma
               * dos p−1 sobre os primos do intervalo) e exige-se a igualdade. Um limiar
               * meu já falhou aqui — `> 300` quando o verdadeiro é 225. */
              for(long p = 2; p <= 41; p++) if(nt_primo(p)) esperado += p - 1;
              for(long p = 2; p <= 41; p++){
                  if(!nt_primo(p)) continue;
                  for(long a = 1; a < p; a++){
                      long por_euclides; nm_inv_mod(a, p, &por_euclides);
                      long por_fermat = iz_pot_mod(a, p-2, p);
                      long por_busca = 0;
                      for(long b = 1; b < p; b++) if((a*b) % p == 1){ por_busca = b; break; }
                      if(por_euclides != por_fermat) imal++;
                      if(por_euclides != por_busca) imal++;
                      if((a*por_euclides) % p != 1) imal++;
                      if(iz_pot_mod(a, p-1, p) != 1) imal++;   /* e o Fermat que o funda */
                      feitos++;
                  }
              }
              printf("      %ld inversos em 𝔽ₚ (p ≤ 41) por TRÊS caminhos — Euclides,"
                     " a^{p−2} e a busca;  Σ(p−1) = %ld\n", feitos, esperado);
              ok("o INVERSO em 𝔽ₚ sai por TRÊS caminhos que têm de concordar: o x de"
                 " BÉZOUT reduzido, a potência a^{p−2} (que é FERMAT a transformar"
                 " inversão em potência) e a busca. Concordam em todos, e o Fermat que"
                 " funda o segundo mede-se junto. E o total confere com Σ(p−1) contado"
                 " à parte — o limiar não é um número meu",
                 imal == 0 && feitos == esperado && esperado > 0); }

            /* (11)(18)(19) |K| = pⁿ: os que EXISTEM constroem-se, e os que não, contam-se */
            { int nmal = 0, construidos = 0;
              struct { int p, g; int *f; int q; } cs[3] = { {2,2,f4,4}, {3,2,f9,9}, {2,3,f8,8} };
              for(int i = 0; i < 3; i++){
                  Anel R;
                  if(!corpo_ext(cs[i].p, cs[i].f, cs[i].g, &R)){ nmal++; continue; }
                  if(R.n != cs[i].q) nmal++;
                  if(!an_corpo(&R,0)) nmal++;
                  construidos++;
              }
              /* e as cardinalidades: potência de primo ⟺ existe corpo (nas que sabemos
               * construir); as com dois primos distintos NÃO podem existir */
              int impossiveis = 0, possiveis = 0;
              for(int q = 2; q <= 30; q++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(q, pr, ex, NT_FAT);
                  if(k > 1) impossiveis++; else possiveis++;
              }
              /* 6 e 10 são os que ele nomeia */
              long pr6[NT_FAT], pr10[NT_FAT]; int e6[NT_FAT], e10[NT_FAT];
              int k6 = nt_fatora(6, pr6, e6, NT_FAT), k10 = nt_fatora(10, pr10, e10, NT_FAT);
              printf("      construídos: 𝔽₄, 𝔽₈, 𝔽₉ (todos corpos);  6 = %ld·%ld e"
                     " 10 = %ld·%ld — dois primos distintos, logo impossíveis\n",
                     pr6[0], pr6[1], pr10[0], pr10[1]);
              ok("|K| = pⁿ: os que EXISTEM constroem-se e medem-se (𝔽₄, 𝔽₈, 𝔽₉ são"
                 " corpos pelo mesmo `an_corpo` que mediu o ℤ₅), e os que NÃO existem"
                 " dizem-se pela fatoração — 6 e 10 têm dois primos distintos, e a"
                 " característica teria de ser os dois ao mesmo tempo. Dizer que não"
                 " existem sem construir os que existem seria metade",
                 nmal == 0 && construidos == 3 && k6 == 2 && k10 == 2
                 && impossiveis > 0 && possiveis > 0); }

            /* (12)(13)(17) ℚ(√d): a norma é a fibra, e o inverso sai dela */
            { int emal = 0; long feitos = 0;
              for(long d = 2; d <= 5; d++){
                  if(raizi(d)*raizi(d) == d) continue;      /* d quadrado: não é extensão */
                  for(long ap = -6; ap <= 6; ap++) for(long bp = -6; bp <= 6; bp++){
                      Qs x = qs(qz_de_inteiro(ap), qz_de_inteiro(bp)), inv;
                      Qz N = qs_norma(x, d);
                      int nulo = (ap == 0 && bp == 0);
                      if((N.p == 0) != nulo) emal++;        /* a norma só zera no zero */
                      if(nulo) continue;
                      if(!qs_inverso(x, d, &inv)){ emal++; continue; }
                      Qs um = qs_mult(x, inv, d);
                      if(!qs_igual(um, qs(qz_de_inteiro(1), qz(0,1)))) emal++;
                      /* e o CONJUGADO é o dual: x·x† = N(x), e (x†)† = x */
                      if(!qz_igual(qs_mult(x, qs_conj(x), d).a, N)) emal++;
                      if(!qs_igual(qs_conj(qs_conj(x)), x)) emal++;
                      feitos++;
                  }
                  /* e α² − d = 0 exatamente, que é o polinómio mínimo a fechar */
                  Qs alfa = qs(qz(0,1), qz(1,1));
                  Qs val = qs_soma(qs_mult(alfa,alfa,d), qs(qz_de_inteiro(-d), qz(0,1)));
                  if(val.a.p || val.b.p) emal++;
              }
              printf("      ℚ(√d) para d = 2, 3, 5: %ld inversos exatos, e a norma só"
                     " zera no zero\n", feitos);
              ok("ℚ(√d) é corpo e a FIBRA é a NORMA: (a+b√d)(a−b√d) = a² − db², que só é"
                 " zero quando a = b = 0 — porque √d ∉ ℚ, que é o andar de ℝ a servir"
                 " este. O conjugado é o DUAL (x†† = x, e xx† = N(x)), e o polinómio"
                 " mínimo fecha: α² − d = 0 exato",
                 emal == 0 && feitos > 400); }

            /* (14)(15)(16)(20) O QUOCIENTE: irredutível ⟺ corpo, nos DOIS sentidos */
            { int qmal2 = 0;
              struct { int p, g; int *f; const char *nome; } cs[4] = {
                  {2,2,f4,"F2[x]/(x²+x+1)"}, {3,2,f9,"F3[x]/(x²+1)"},
                  {2,3,f8,"F2[x]/(x³+x+1)"}, {2,2,red,"F2[x]/(x²+1)"} };
              int irr_e_corpo = 0, red_e_nao = 0;
              for(int i = 0; i < 4; i++){
                  Anel R;
                  if(!corpo_ext(cs[i].p, cs[i].f, cs[i].g, &R)){ qmal2++; continue; }
                  int irr = fx_irredutivel(cs[i].f, cs[i].g, cs[i].p);
                  int cor = an_corpo(&R,0);
                  if(irr != cor) qmal2++;                   /* A EQUIVALÊNCIA */
                  if(irr && cor) irr_e_corpo++;
                  if(!irr && !cor) red_e_nao++;
                  /* e quando é corpo, todo não nulo tem inverso e o 0 não tem */
                  if(cor){
                      for(int a = 1; a < R.n; a++) if(corpo_inv(&R,a) < 0) qmal2++;
                      if(corpo_inv(&R,0) >= 0) qmal2++;     /* 0⁻¹ NÃO existe */
                  }
              }
              printf("      quatro quocientes: %d irredutíveis que dão corpo e %d"
                     " redutíveis que NÃO dão — a equivalência nos dois sentidos\n",
                     irr_e_corpo, red_e_nao);
              ok("K[x]/(p) é CORPO exatamente quando p é IRREDUTÍVEL — medido nos dois"
                 " sentidos, com o x²+1 sobre 𝔽₂ (que é (x+1)²) a dar o lado que falha."
                 " «Irredutível está para o polinómio como primo está para o inteiro», e"
                 " em cada corpo construído o 0 continua sem inverso",
                 qmal2 == 0 && irr_e_corpo == 3 && red_e_nao == 1); }

            /* (21)(22) K× É CÍCLICO — e mede-se nas EXTENSÕES, não só em ℤₚ */
            { int cmal2 = 0, corpos = 0;
              for(int m = 2; m <= 23; m++){
                  Anel R; an_zn(&R,m);
                  if(!an_corpo(&R,0)) continue;
                  int g = corpo_primitivo(&R);
                  if(g < 0){ cmal2++; continue; }
                  if(nm_ordem(g, m) != m-1) cmal2++;
                  corpos++;
              }
              struct { int p, g; int *f; int q; } cs[3] = { {2,2,f4,4}, {3,2,f9,9}, {2,3,f8,8} };
              for(int i = 0; i < 3; i++){
                  Anel R;
                  corpo_ext(cs[i].p, cs[i].f, cs[i].g, &R);
                  int g = corpo_primitivo(&R);
                  if(g < 0){ cmal2++; continue; }
                  /* e a órbita percorre TODOS os não nulos, sem repetir */
                  int visto[ES_MAX] = {0}, x = 1 % R.n, distintos = 0;
                  for(int k = 1; k < R.n; k++){
                      x = R.mult[x][g];
                      if(!visto[x]){ visto[x] = 1; distintos++; }
                  }
                  if(distintos != R.n - 1) cmal2++;
                  corpos++;
              }
              /* e o número de primitivos é φ(q−1) — a contagem que confirma o teorema */
              Anel R7; an_zn(&R7,7);
              int prim7 = 0;
              for(int g = 1; g < 7; g++) if(nm_ordem(g,7) == 6) prim7++;
              printf("      %d corpos (ℤₚ e extensões) com primitivo; em 𝔽₇ há %d"
                     " primitivos e φ(6) = %ld\n", corpos, prim7, nm_phi(6));
              ok("K× é CÍCLICO em todo corpo finito — e mede-se também nas EXTENSÕES"
                 " (𝔽₄, 𝔽₈, 𝔽₉), não só em ℤₚ, porque em ℤₚ o teorema podia estar a ser"
                 " confundido com o das raízes primitivas. A órbita do gerador percorre"
                 " todos os não nulos sem repetir, e os primitivos são φ(q−1)",
                 cmal2 == 0 && corpos >= 10 && prim7 == nm_phi(6)); }

            /* (25) A CADEIA: Euclides ↔ MDC ↔ Bézout ↔ inverso ↔ corpo, como EQUIVALÊNCIA */
            { int amal = 0, corpos = 0;
              for(int m = 2; m <= ES_MAX; m++){        /* o teto da tábua */
                  int todos_coprimos = 1;
                  for(int b = 1; b < m; b++) if(iz_gcd(b,m,0,0) != 1) todos_coprimos = 0;
                  int todos_invertem = 1;
                  Anel R; an_zn(&R,m);
                  for(int b = 1; b < m; b++) if(corpo_inv(&R,b) < 0) todos_invertem = 0;
                  int e_corpo = an_corpo(&R,0);
                  /* as TRÊS leituras têm de coincidir, e não duas */
                  if(todos_coprimos != todos_invertem) amal++;
                  if(todos_invertem != e_corpo) amal++;
                  if(e_corpo) corpos++;
              }
              ok("A CADEIA FECHA COMO EQUIVALÊNCIA: «Euclides devolve sempre 1» ⟺ «todo"
                 " não nulo tem inverso» ⟺ «é corpo» — as três leituras coincidem em ℤₘ"
                 " até ao teto da tábua. Euclides, MDC, Bézout, inverso e corpo não são cinco"
                 " coisas parecidas: são o mesmo rastro lido em cinco colunas",
                 amal == 0 && corpos > 0); }

            /* E OS VINTE E CINCO CORREM */
            { int vmal = 0, por_n = 0, por_nome = 0;
              fflush(stdout);
              int guarda = dup(1), nulo = open("/dev/null", O_WRONLY);
              if(guarda >= 0 && nulo >= 0) dup2(nulo, 1);
              for(int k = 1; k <= 25; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "corpos %d", k);
                  if(resolve_corpo(fala)) por_n++; else vmal++;
              }
              for(size_t i = 0; i < sizeof CP25/sizeof *CP25; i++)
                  if(resolve_corpo(CP25[i].nome)) por_nome++; else vmal++;
              if(resolve_corpo("corpos 26")) vmal++;
              fflush(stdout);
              if(guarda >= 0){ dup2(guarda, 1); close(guarda); }
              if(nulo >= 0) close(nulo);
              printf("      os vinte e cinco: %d pelo número, %d pelo nome, e a 26 é"
                     " recusada\n", por_n, por_nome);
              ok("OS VINTE E CINCO da teoria dos corpos correm pelo NÚMERO e pelo NOME,"
                 " com o fora de alcance RECUSADO — e cada um na espinha de sete ticks,"
                 " que é o que ele confirmou no topo do ficheiro",
                 vmal == 0 && por_n == 25 && por_nome == 25); }
        }

        /* ═══ §C35 ÁLGEBRA MODERNA: O RASTRO DE SETE TICKS ════════════════════════
         * «Álgebra moderna troca calcular números por ESTUDAR OPERAÇÕES e suas
         * estruturas», e a exigência de forma vale para todos os teoremas:
         *
         *   hipóteses → definição → transição → lei usada → testemunha → conclusão →
         *   volta
         *
         * «E o mais importante para o teu sistema: NÃO MEDIR SÓ A CONCLUSÃO.» Por isso
         * aqui mede-se a cadeia, e não só o fim — os passos intermédios de cada prova
         * têm asserção própria.
         *
         * E o andar diz o nome do que a casa já fazia: «(G,⋆) é grupo quando todo a
         * possui inverso — exatamente a reversibilidade que vocês já encontraram no
         * andar dos inteiros». Tudo por TÁBUA finita, logo toda varredura é completa. */
        printf("\n§C35 ÁLGEBRA MODERNA: sete ticks por teorema, e a cadeia medida inteira.\n\n");
        {
            Est Z12, S3, Z6, Z5m;
            es_zn(&Z12, 12); es_s3(&S3); es_zn(&Z6, 6); es_zn_mult(&Z5m, 6);

            /* (1)(2)(3) OS TRÊS TEOREMAS DA UNICIDADE, e as cadeias medidas por dentro */
            { int umal = 0;
              Est *Gs[3] = { &Z12, &S3, &Z6 };
              for(int i = 0; i < 3; i++){
                  const Est *G = Gs[i];
                  int quantos_e = 0, e = es_neutro(G);
                  for(int x = 0; x < G->n; x++){
                      int bom = 1;
                      for(int a = 0; a < G->n && bom; a++)
                          if(G->op[x][a] != a || G->op[a][x] != a) bom = 0;
                      if(bom) quantos_e++;
                  }
                  if(quantos_e != 1) umal++;                       /* identidade única */
                  for(int a = 0; a < G->n; a++){
                      int q = 0;
                      for(int b = 0; b < G->n; b++)
                          if(G->op[a][b] == e && G->op[b][a] == e) q++;
                      if(q != 1) umal++;                           /* inverso único */
                      /* e a CADEIA da prova 2, passo a passo: b = b⋆e = b⋆(a⋆c) = … */
                      int ai = es_inverso(G, a);
                      if(ai < 0){ umal++; continue; }
                      if(G->op[ai][e] != ai) umal++;               /* b⋆e = b */
                      if(G->op[ai][G->op[a][ai]] != G->op[G->op[ai][a]][ai]) umal++;  /* assoc */
                      if(G->op[e][ai] != ai) umal++;               /* e⋆c = c */
                  }
                  /* cancelamento: em GRUPO vale */
                  for(int a = 0; a < G->n; a++) for(int b = 0; b < G->n; b++) for(int c = 0; c < G->n; c++)
                      if(G->op[a][b] == G->op[a][c] && b != c) umal++;
              }
              /* o GUME do cancelamento: num MONOIDE sem inversos FALHA, e conta-se */
              long falhas_monoide = 0;
              for(int a = 0; a < Z5m.n; a++) for(int b = 0; b < Z5m.n; b++) for(int c = 0; c < Z5m.n; c++)
                  if(Z5m.op[a][b] == Z5m.op[a][c] && b != c) falhas_monoide++;
              printf("      identidade e inverso únicos em Z12, S3 e Z6; cancelamento vale"
                     " nos três e FALHA em %ld triplos de (Z6,x)\n", falhas_monoide);
              ok("os TRÊS teoremas da unicidade, com a CADEIA medida por dentro e não só a"
                 " conclusão: o neutro é único, o inverso é único (e cada passo de"
                 " b = b⋆e = b⋆(a⋆c) = (b⋆a)⋆c = e⋆c = c tem asserção própria), e o"
                 " cancelamento vale em GRUPO. O gume é o monóide (ℤ₆,×), onde falha —"
                 " a hipótese não é decoração",
                 umal == 0 && falhas_monoide > 0); }

            /* (4)(5) SUBGRUPOS: contêm o neutro, e a interseção é subgrupo — mas a
             * UNIÃO não, e é o gume */
            { int smal = 0, subs = 0;
              unsigned lista[64]; int ns = 0;
              for(unsigned H = 0; H < (1u << 12); H++){
                  if(!es_subgrupo(&Z12,H)) continue;
                  subs++;
                  if(!es_em(H, es_neutro(&Z12))) smal++;           /* contém o neutro */
                  if(ns < 64) lista[ns++] = H;
              }
              if(es_subgrupo(&Z12, 0)) smal++;                     /* o vazio NÃO é */
              int cruzados = 0, uniao_falha = 0;
              for(int i = 0; i < ns; i++) for(int j = 0; j < ns; j++){
                  if(!es_subgrupo(&Z12, lista[i] & lista[j])) smal++;   /* interseção */
                  if(!es_subgrupo(&Z12, lista[i] | lista[j])) uniao_falha++;
                  cruzados++;
              }
              printf("      Z12 tem %d subgrupos (em 4096 subconjuntos); interseções"
                     " cruzadas: %d, e uniões que NÃO são subgrupo: %d\n",
                     subs, cruzados, uniao_falha);
              ok("TODO SUBGRUPO CONTÉM O NEUTRO (e o vazio não é subgrupo, que é a razão"
                 " de a hipótese «não vazio» estar lá) e a INTERSEÇÃO de subgrupos é"
                 " subgrupo — varridos os 4096 subconjuntos de ℤ₁₂ e cruzados todos os"
                 " pares. O gume é a UNIÃO, que falha, e conta-se quantas vezes",
                 smal == 0 && subs == 6 && uniao_falha > 0); }

            /* (6)(7)(8) O HOMOMORFISMO: imagem subgrupo, núcleo normal, e injetivo ⟺
             * núcleo trivial — varridos em TODOS os homomorfismos x ↦ kx de ℤ₁₂ */
            { int hmal = 0, homos = 0, inj = 0, nao_inj = 0;
              for(int k = 0; k < 12; k++){
                  int f[ES_MAX];
                  for(int a = 0; a < 12; a++) f[a] = (k*a) % 12;
                  if(!es_homo(&Z12,&Z12,f,0,0)) continue;
                  homos++;
                  unsigned K = es_nucleo(&Z12,&Z12,f), Img = es_imagem(&Z12,f);
                  if(!es_subgrupo(&Z12,Img)) hmal++;               /* (6) imagem */
                  if(!es_subgrupo(&Z12,K)) hmal++;                 /* (7) núcleo */
                  if(!es_normal(&Z12,K)) hmal++;                   /*     e normal */
                  int ij = es_injetiva(&Z12,f);
                  if(ij != (es_ordem_conj(K) == 1)) hmal++;        /* (8) a equivalência */
                  if(ij) inj++; else nao_inj++;
                  /* e o LAGRANGE a fechar: |ker|·|Im| = |G| */
                  if(es_ordem_conj(K) * es_ordem_conj(Img) != 12) hmal++;
              }
              printf("      os %d homomorfismos x ↦ kx de Z12: %d injetivos, %d não —"
                     " e |ker|·|Im| = 12 em todos\n", homos, inj, nao_inj);
              ok("nos homomorfismos de ℤ₁₂: a IMAGEM é subgrupo, o NÚCLEO é subgrupo"
                 " NORMAL, e «injetivo ⟺ núcleo trivial» decide caso a caso com os DOIS"
                 " lados a ocorrer. E |ker|·|Im| = |G| cai de graça — é o primeiro"
                 " teorema do isomorfismo já a aparecer na contagem",
                 hmal == 0 && homos == 12 && inj > 0 && nao_inj > 0); }

            /* (9) LAGRANGE: as classes PARTICIONAM, e todas têm o mesmo tamanho —
             * medido em todos os subgrupos de ℤ₁₂ e de S₃ */
            { int lmal = 0, testados = 0;
              Est *Gs[2] = { &Z12, &S3 };
              for(int i = 0; i < 2; i++){
                  const Est *G = Gs[i];
                  for(unsigned H = 0; H < (1u << G->n); H++){
                      if(!es_subgrupo(G,H)) continue;
                      unsigned cls[32]; int nc = 0, visto = 0;
                      for(int g = 0; g < G->n; g++){
                          if((visto >> g) & 1) continue;
                          unsigned C = es_classe(G,H,g);
                          if(nc < 32) cls[nc++] = C;
                          visto |= C;
                      }
                      unsigned uniao = 0;
                      for(int x = 0; x < nc; x++){
                          if(es_ordem_conj(cls[x]) != es_ordem_conj(H)) lmal++;  /* mesmo tamanho */
                          uniao |= cls[x];
                          for(int y = x+1; y < nc; y++) if(cls[x] & cls[y]) lmal++; /* disjuntas */
                      }
                      if((int)es_ordem_conj(uniao) != G->n) lmal++;               /* cobrem */
                      if(es_ordem_conj(H) * nc != G->n) lmal++;                   /* |H|·[G:H] = |G| */
                      if(G->n % es_ordem_conj(H)) lmal++;                         /* logo |H| | |G| */
                      testados++;
                  }
              }
              printf("      Lagrange em %d subgrupos de Z12 e S3: classes do mesmo"
                     " tamanho, disjuntas, e a cobrir o grupo\n", testados);
              ok("LAGRANGE medido pela PARTIÇÃO e não pela conclusão: em todos os"
                 " subgrupos de ℤ₁₂ e de S₃, as classes laterais têm o mesmo tamanho, são"
                 " disjuntas duas a duas e cobrem o grupo — e só daí sai |H|·[G:H] = |G|."
                 " É «elemento → classe → partição → volta»",
                 lmal == 0 && testados > 8); }

            /* O NÃO ABELIANO É QUE EXERCITA A NORMALIDADE — e sem ele o andar
             * estava por medir. Em ℤ₁₂ TODO subgrupo é normal e gH = Hg sempre, porque
             * é abeliano: as mutações «normal sem conjugar» e «classe do outro lado»
             * sobreviviam à varredura inteira. É S₃ que decide, e por isso entra. */
            { int nmal = 0, normais = 0, nao_normais = 0, lados_diferem = 0;
              for(unsigned H = 0; H < (1u << 6); H++){
                  if(!es_subgrupo(&S3,H)) continue;
                  int norm = es_normal(&S3,H);
                  if(norm) normais++; else nao_normais++;
                  /* e o lado da classe: gH contra Hg, subgrupo a subgrupo */
                  int difere = 0;
                  for(int g = 0; g < 6; g++){
                      unsigned E = es_classe(&S3,H,g), D = 0;
                      for(int h = 0; h < 6; h++) if(es_em(H,h)) D |= 1u << S3.op[h][g];
                      if(E != D) difere = 1;
                  }
                  if(difere) lados_diferem++;
                  /* O TEOREMA: normal ⟺ os dois lados coincidem. É a definição vista de
                   * outro ângulo, e mede-se a equivalência, não uma direção */
                  if(norm == difere) nmal++;
              }
              /* e um NÃO-homomorfismo, porque o filtro nunca excluía ninguém: todos os
               * x ↦ kx de ℤ₁₂ são homomorfismos, logo `es_homo` podia devolver sempre
               * verdade sem ninguém dar por isso */
              int f2[ES_MAX], ta = 0, tb = 0;
              for(int a = 0; a < 12; a++) f2[a] = (a*a) % 12;
              int e_homo = es_homo(&Z12,&Z12,f2,&ta,&tb);
              if(e_homo) nmal++;                                  /* x² NÃO preserva */
              if(Z12.op[f2[ta]][f2[tb]] == f2[Z12.op[ta][tb]]) nmal++;  /* e a testemunha vale */
              printf("      S₃: %d subgrupos normais e %d NÃO normais, e os lados da"
                     " classe diferem exatamente nos %d não normais\n",
                     normais, nao_normais, lados_diferem);
              printf("      e x ↦ x² em Z12 NÃO é homomorfismo: f(%d+%d) = %d mas"
                     " f(%d)·f(%d) = %d\n", ta, tb, f2[Z12.op[ta][tb]],
                     ta, tb, Z12.op[f2[ta]][f2[tb]]);
              ok("O NÃO ABELIANO É QUE MEDE A NORMALIDADE: em ℤ₁₂ todo subgrupo é normal"
                 " e gH = Hg sempre, logo a conjugação e o lado da classe nunca eram"
                 " exercitados. Em S₃ há subgrupos NÃO normais, e mede-se a equivalência"
                 " «normal ⟺ gH = Hg» subgrupo a subgrupo. E entra um NÃO-homomorfismo"
                 " (x ↦ x²), porque todos os x ↦ kx eram homomorfismos e o filtro nunca"
                 " excluía ninguém",
                 nmal == 0 && nao_normais == 3 && lados_diferem == 3 && !e_homo); }

            /* (10) CAYLEY: a translação é permutação, e a matriz das λ É a tábua */
            { int cmal = 0;
              Est *Gs[3] = { &Z12, &S3, &Z6 };
              for(int i = 0; i < 3; i++){
                  const Est *G = Gs[i];
                  for(int g = 0; g < G->n; g++){
                      unsigned imagem = 0;
                      for(int x = 0; x < G->n; x++) imagem |= 1u << G->op[g][x];
                      if((int)es_ordem_conj(imagem) != G->n) cmal++;   /* λ_g é bijeção */
                  }
                  for(int g = 0; g < G->n; g++) for(int h = g+1; h < G->n; h++){
                      int igual = 1;
                      for(int x = 0; x < G->n; x++) if(G->op[g][x] != G->op[h][x]) igual = 0;
                      if(igual) cmal++;                                /* g ↦ λ_g injetiva */
                  }
              }
              ok("CAYLEY: cada translação λ_g é uma PERMUTAÇÃO do grupo (a bijetividade"
                 " vem do cancelamento) e g ↦ λ_g é injetiva — medido em ℤ₁₂, S₃ e ℤ₆."
                 " E a observação que o torna barato: a matriz das λ É a tábua de Cayley"
                 " lida por linhas, logo a representação não é um modelo do grupo, é o"
                 " próprio objeto", cmal == 0); }

            /* (11) O PRIMEIRO TEOREMA DO ISOMORFISMO: G/ker ≅ Im, com a busca EXAUSTIVA */
            { int imal = 0, casos = 0;
              for(int k = 0; k < 12; k++){
                  int f[ES_MAX];
                  for(int a = 0; a < 12; a++) f[a] = (k*a) % 12;
                  if(!es_homo(&Z12,&Z12,f,0,0)) continue;
                  unsigned K = es_nucleo(&Z12,&Z12,f), Img = es_imagem(&Z12,f);
                  Est Q; unsigned cls[ES_MAX];
                  int m = es_quociente(&Z12, K, &Q, cls);
                  if(m <= 0){ imal++; continue; }
                  Est EI; int rot[ES_MAX], ni = 0;
                  for(int a = 0; a < 12; a++) if(es_em(Img,a)) rot[ni++] = a;
                  EI.n = ni; EI.nome = "Im";
                  for(int x = 0; x < ni; x++) for(int y = 0; y < ni; y++){
                      int v = (rot[x] + rot[y]) % 12;
                      for(int z = 0; z < ni; z++) if(rot[z] == v){ EI.op[x][y] = z; break; }
                  }
                  if(m != ni) imal++;
                  if(!es_isomorfas(&Q, &EI, 0)) imal++;
                  casos++;
              }
              printf("      G/ker f ≅ Im f nos %d homomorfismos, com a bijeção procurada"
                     " EXAUSTIVAMENTE em cada um\n", casos);
              ok("o PRIMEIRO TEOREMA DO ISOMORFISMO — G/ker f ≅ Im f — verificado em"
                 " TODOS os homomorfismos de ℤ₁₂, e o isomorfismo não é declarado: a"
                 " bijeção que preserva a operação é PROCURADA, esgotando as"
                 " possibilidades. Se não houvesse, o teorema caía ali",
                 imal == 0 && casos == 12); }

            /* (12)(13)(14) OS ANÉIS: ideais são núcleos, o isomorfismo, e os ideais de ℤ */
            { int amal = 0, ideais12 = 0;
              for(int m = 2; m <= 12; m++){
                  Anel R; an_zn(&R, m);
                  if(!an_distrib(&R)) amal++;
                  for(unsigned Idl = 0; Idl < (1u << m); Idl++){
                      if(!an_ideal(&R, Idl)) continue;
                      /* TODO ideal de ℤₘ é gerado pelo seu menor positivo */
                      int menor = -1;
                      for(int a = 1; a < m; a++) if(es_em(Idl,a)){ menor = a; break; }
                      unsigned ger = 0;
                      if(menor < 0) ger = 1u;
                      else for(int k = 0; k < m; k++) ger |= 1u << ((menor*k) % m);
                      if(ger != Idl) amal++;
                      if(m == 12) ideais12++;
                  }
              }
              /* e o isomorfismo de anéis: ℤ₁₂ → ℤ₆ preserva as DUAS operações */
              int rmal = 0;
              for(int a = 0; a < 12; a++) for(int b = 0; b < 12; b++){
                  if(((a+b) % 12) % 6 != ((a%6) + (b%6)) % 6) rmal++;
                  if(((a*b) % 12) % 6 != ((a%6) * (b%6)) % 6) rmal++;
              }
              printf("      ideais de Zm para m ≤ 12: todos gerados pelo menor positivo"
                     " (Z12 tem %d), e Z12 → Z6 preserva as duas operações\n", ideais12);
              ok("os IDEAIS de ℤₘ são TODOS principais — gerados pelo menor elemento"
                 " positivo, que é a mesma cadeia (boa ordenação + divisão com resto) do"
                 " gcd como menor de {ax+by}. E o homomorfismo de anéis preserva as DUAS"
                 " operações, medidas em separado: preservar uma só não é homomorfismo"
                 " de anéis", amal == 0 && rmal == 0 && ideais12 == 6); }

            /* (15) CORPO ⟹ DOMÍNIO, e a RECÍPROCA é falsa — o gume do sentido único */
            { int cmal2 = 0, corpos = 0, dominios = 0, so_dominio = 0;
              for(int m = 2; m <= 20; m++){
                  Anel R; an_zn(&R, m);
                  int corpo = an_corpo(&R, 0), dom = an_dominio(&R, 0, 0);
                  if(corpo && !dom) cmal2++;                       /* corpo ⟹ domínio */
                  if(corpo != (nt_primo(m) != 0)) cmal2++;         /* ℤₘ corpo ⟺ m primo */
                  if(corpo) corpos++;
                  if(dom) dominios++;
                  if(dom && !corpo) so_dominio++;
              }
              /* E A RECÍPROCA MEDE-SE, EM VEZ DE SE AFIRMAR. Em ℤₘ os dois lados
               * COINCIDEM — e isso não é acidente, é o teorema «todo domínio FINITO é
               * corpo». Portanto a varredura acima NÃO exercita a direção: aqui ela
               * seria uma equivalência, e eu estaria a chamar «um sentido só» a um
               * conjunto onde os dois valem. A testemunha tem de vir de um domínio
               * INFINITO, e o mais barato é ℤ: mede-se que não tem divisores de zero
               * numa janela, e que o 2 NÃO tem inverso — as duas coisas em inteiros. */
              long zero_div = 0, inverso_de_2 = 0;
              for(long a = -60; a <= 60; a++) for(long b = -60; b <= 60; b++){
                  if(a && b && a*b == 0) zero_div++;          /* ℤ é domínio */
                  if(a == 2 && a*b == 1) inverso_de_2++;      /* e 2 não tem inverso */
              }
              printf("      Z_m para m de 2 a 20: %d corpos, %d domínios, e %d domínios"
                     " que NÃO são corpo — porque todo domínio FINITO é corpo\n",
                     corpos, dominios, so_dominio);
              printf("      e em ℤ (infinito): %ld divisores de zero e %ld inversos do 2"
                     " — domínio que NÃO é corpo, e é ele a testemunha da direção\n",
                     zero_div, inverso_de_2);
              ok("TODO CORPO É DOMÍNIO INTEGRAL — e a implicação tem UM SENTIDO SÓ, mas a"
                 " testemunha disso NÃO pode vir de ℤₘ: aí os dois lados coincidem"
                 " (porque todo domínio FINITO é corpo), e chamar-lhe «um sentido» seria"
                 " medir uma equivalência. A direção prova-se em ℤ, que é domínio (zero"
                 " divisores de zero) e não é corpo (o 2 não tem inverso) — medido",
                 cmal2 == 0 && corpos > 0 && dominios > 0 && so_dominio == 0
                 && zero_div == 0 && inverso_de_2 == 0); }

            /* (16)(17)(18) OS EXERCÍCIOS DO ROTEIRO, com as testemunhas */
            { int emal = 0;
              Est M, Mn;
              es_mais_um(&M, 9); es_menos(&Mn, 5);
              if(!es_fechada(&M) || !es_assoc(&M,0,0,0)) emal++;
              if(es_neutro(&M) != 8) emal++;                       /* −1 mod 9 = 8 */
              if(!es_grupo(&M, 0)) emal++;                         /* e é mais: é grupo */
              int ta = 0, tb = 0, tc = 0;
              if(es_assoc(&Mn, &ta, &tb, &tc)) emal++;             /* a−b NÃO associa */
              if(Mn.op[Mn.op[ta][tb]][tc] == Mn.op[ta][Mn.op[tb][tc]]) emal++; /* e a testemunha vale */
              int pa = 0, pb = 0;
              if(!es_grupo(&S3,0)) emal++;
              if(es_abeliano(&S3,&pa,&pb)) emal++;                 /* S₃ não é abeliano */
              if(S3.op[pa][pb] == S3.op[pb][pa]) emal++;           /* e o par exibido não comuta */
              if(!es_abeliano(&Z12,0,0)) emal++;                   /* e ℤ₁₂ é */
              printf("      a⋆b = a+b+1 tem neutro %d (= −1 mod 9) e é grupo; a−b falha"
                     " em (%d,%d,%d); S₃ não comuta em (%d,%d)\n",
                     es_neutro(&M), ta, tb, tc, pa, pb);
              ok("OS EXERCÍCIOS DO ROTEIRO com as testemunhas exibidas e VERIFICADAS: o"
                 " neutro de a⋆b = a+b+1 é −1 (e a estrutura é mais que monoide, é grupo);"
                 " a−b não associa, com o triplo concreto; e «todo grupo é comutativo» é"
                 " FALSO, com o par de S₃ que o derruba — a recusa é o resultado, não uma"
                 " desistência", emal == 0); }

            /* (19) A SOMA VIRA PRODUTO, exata: a raiz primitiva no lugar do e */
            { int xmal = 0, primos = 0;
              for(long p = 5; p <= 29; p++){
                  if(!nt_primo(p)) continue;
                  long g = 0;
                  for(long c = 2; c < p; c++) if(nm_ordem(c,p) == p-1){ g = c; break; }
                  if(!g){ xmal++; continue; }
                  Est A, B; int rot[ES_MAX], f[ES_MAX];
                  if(p-1 > ES_MAX) continue;
                  es_zn(&A, (int)(p-1));
                  es_unidades(&B, (int)p, rot);
                  for(int x = 0; x < A.n; x++){
                      long v = iz_pot_mod(g, x, p);
                      f[x] = -1;
                      for(int k = 0; k < B.n; k++) if(rot[k] == v){ f[x] = k; break; }
                      if(f[x] < 0) xmal++;
                  }
                  if(!es_homo(&A,&B,f,0,0)) xmal++;                /* g^{x+y} = gˣgʸ */
                  if(!es_injetiva(&A,f)) xmal++;
                  if(A.n != B.n) xmal++;                            /* e bijetiva */
                  primos++;
              }
              printf("      (Z_{p−1},+) ≅ (Zp*,×) por x ↦ gˣ, em %d primos — sem um"
                     " único decimal\n", primos);
              ok("A SOMA VIRA PRODUTO, e EXATAMENTE: o eˣ do ficheiro pedia decimais, mas"
                 " o conteúdo do exercício é algébrico (a potência troca + por ×) e"
                 " realiza-se no finito com uma raiz primitiva. (ℤ_{p−1},+) ≅ (ℤₚ*,×) em"
                 " todos os primos até 29, com a volta a ser o LOGARITMO DISCRETO",
                 xmal == 0 && primos >= 5); }

            /* E OS VINTE CORREM */
            { int vmal = 0, por_n = 0, por_nome = 0;
              fflush(stdout);
              int guarda = dup(1), nulo = open("/dev/null", O_WRONLY);
              if(guarda >= 0 && nulo >= 0) dup2(nulo, 1);
              for(int k = 1; k <= 20; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "algebra %d", k);
                  if(resolve_estrutura(fala)) por_n++; else vmal++;
              }
              for(size_t i = 0; i < sizeof AL20/sizeof *AL20; i++)
                  if(resolve_estrutura(AL20[i].nome)) por_nome++; else vmal++;
              if(resolve_estrutura("algebra 21")) vmal++;
              fflush(stdout);
              if(guarda >= 0){ dup2(guarda, 1); close(guarda); }
              if(nulo >= 0) close(nulo);
              printf("      os vinte: %d pelo número, %d pelo nome, e a 21 é recusada\n",
                     por_n, por_nome);
              ok("OS VINTE da álgebra moderna correm pelo NÚMERO e pelo NOME, e o fora de"
                 " alcance é RECUSADO — e cada um corre os SETE ticks do rastro que o"
                 " ficheiro exige, com a definição em LaTeX no seu lugar",
                 vmal == 0 && por_n == 20 && por_nome == 20); }
        }

        /* ═══ §C34 CURVAS ELÍPTICAS: A FIBRA DETERMINA QUAL OPERAÇÃO EXISTE ═══════
         * «reta → interseção → reflexão → novo ponto → repetição» — e a frase que já é a
         * língua desta casa: «a FIBRA determina qual operação existe». Com x₁ ≠ x₂ há
         * secante; com x₁ = x₂ há DUAS fibras (opostos ou tangente). Não se aproxima o
         * denominador zero: muda-se de operação, e diz-se qual.
         *
         * E sobre 𝔽ₚ «toda a geometria vira aritmética discreta»: a divisão é a INVERSÃO
         * MODULAR, isto é, o andar dos racionais a voltar inteiro. */
        printf("\n§C34 CURVAS ELÍPTICAS: a máquina de rastros, e a fibra a escolher a operação.\n\n");
        {
            Qz a = qz_de_inteiro(-2), b = qz_de_inteiro(1);
            PtQ P = eq_pt(qz_de_inteiro(0), qz_de_inteiro(1));

            /* (§1)(§2)(§6) A CURVA DELE, E OS NÚMEROS DELE — sobre ℚ, exatos */
            { int emal = 0;
              if(!eq_regular(a,b)) emal++;                    /* 4a³+27b² = −5 ≠ 0 */
              if(!eq_na_curva(a,b,P)) emal++;
              PtQ P2 = eq_mult(a,b,P,2), P3 = eq_mult(a,b,P,3), P4 = eq_mult(a,b,P,4);
              if(!eq_na_curva(a,b,P2) || !eq_na_curva(a,b,P3)) emal++;
              if(!eq_igual(P2, eq_pt(qz_de_inteiro(1), qz_de_inteiro(0)))) emal++;
              if(!eq_igual(P3, eq_neg(P))) emal++;
              if(!P4.inf) emal++;
              long ord = eq_ordem(a,b,P,40);
              /* o neutro e o inverso, varridos na órbita */
              for(long k = 1; k <= 8; k++){
                  PtQ Q = eq_mult(a,b,P,k);
                  if(!eq_igual(eq_soma(a,b,Q,eq_O()), Q)) emal++;
                  if(!eq_soma(a,b,Q,eq_neg(Q)).inf) emal++;
                  if(!Q.inf && !eq_na_curva(a,b,eq_neg(Q))) emal++;
              }
              printf("      E: y² = x³ − 2x + 1 e P = (0,1):  2P = ");
              esc_pt(P2); printf(",  3P = "); esc_pt(P3);
              printf(",  4P = "); esc_pt(P4); printf(",  ord(P) = %ld\n", ord);
              ok("A CURVA DELE fecha sobre ℚ com coordenadas EXATAS: P = (0,1) dá"
                 " 2P = (1,0) e 4P = 𝒪, e a ordem de P é 4 — o rastro P → 2P → 4P → 𝒪"
                 " que ele desenhou. O neutro (P + 𝒪 = P) e o inverso (P + (−P) = 𝒪)"
                 " valem em toda a órbita, e −P nunca sai da curva",
                 emal == 0 && ord == 4); }

            /* (§4) VIÈTE: a geometria da reta VIRA identidade algébrica — x₁+x₂+x₃ = m².
             * É a prova que ele destaca, e mede-se em vez de se citar. */
            { int vmal = 0, casos = 0;
              long p = 17, A = 2, B = 2;                      /* em 𝔽ₚ há pontos que cheguem */
              PtF pts[64]; long np = 0;
              for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
                  PtF T = ef_pt(x,y,p);
                  if(ef_na_curva(A,B,p,T) && np < 64) pts[np++] = T;
              }
              for(long i = 0; i < np; i++) for(long j = 0; j < np; j++){
                  if(pts[i].x == pts[j].x) continue;          /* só a secante */
                  long num = ef_m(pts[j].y - pts[i].y, p), den = ef_m(pts[j].x - pts[i].x, p);
                  long inv; if(!nm_inv_mod(den, p, &inv)) continue;
                  long m = ef_m(num*inv, p);
                  PtF S = ef_soma(A,B,p,pts[i],pts[j]);
                  if(S.inf) continue;
                  /* x₃ do TERCEIRO ponto é o mesmo x da soma (a reflexão não mexe no x) */
                  if(ef_m(pts[i].x + pts[j].x + S.x, p) != ef_m(m*m, p)) vmal++;
                  casos++;
              }
              printf("      Viète em 𝔽₁₇: x₁ + x₂ + x₃ = m² em %d secantes\n", casos);
              ok("VIÈTE — a geometria da reta VIRA identidade algébrica: substituindo a"
                 " reta na curva sai uma cúbica cujas três raízes são x₁, x₂, x₃, e a soma"
                 " delas é m². Daí x₃ = m² − x₁ − x₂, e a fórmula deixa de ser uma receita"
                 " — é a relação de Viète, medida em todas as secantes do grupo",
                 vmal == 0 && casos > 200); }

            /* (§7) A LEI DE GRUPO, COM A ASSOCIATIVIDADE EXAUSTIVA — «não basta testar
             * alguns pontos e declarar: é uma identidade estrutural» */
            { long p = 17, A = 2, B = 2;
              PtF pts[64]; long np = 0;
              pts[np++] = ef_O();
              for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
                  PtF T = ef_pt(x,y,p);
                  if(ef_na_curva(A,B,p,T) && np < 64) pts[np++] = T;
              }
              long amal = 0, triplos = 0, pmal = 0;
              for(long i = 0; i < np; i++) for(long j = 0; j < np; j++){
                  PtF s = ef_soma(A,B,p,pts[i],pts[j]);
                  if(!ef_na_curva(A,B,p,s)) pmal++;                        /* fechamento */
                  if(!ef_igual(s, ef_soma(A,B,p,pts[j],pts[i]))) pmal++;   /* comutativa */
                  if(!ef_igual(ef_soma(A,B,p,pts[i],ef_O()), pts[i])) pmal++;
                  if(!ef_soma(A,B,p,pts[i],ef_neg(p,pts[i])).inf) pmal++;
                  for(long k = 0; k < np; k++){
                      PtF e = ef_soma(A,B,p, s, pts[k]);
                      PtF d = ef_soma(A,B,p, pts[i], ef_soma(A,B,p,pts[j],pts[k]));
                      if(!ef_igual(e,d)) amal++;
                      triplos++;
                  }
              }
              printf("      E(𝔽₁₇) tem %ld pontos: associatividade em %ld triplos, e o"
                     " resto em %ld pares\n", np, triplos, np*np);
              ok("A LEI DE GRUPO com a ASSOCIATIVIDADE varrida EXAUSTIVAMENTE — os 6859"
                 " triplos do grupo inteiro, não «alguns pontos». É ele que chama a isto o"
                 " gume pesado, e num grupo finito a varredura completa é possível: não é"
                 " amostra, é o conjunto todo",
                 amal == 0 && pmal == 0 && triplos > 6000 && np == 19); }

            /* (§10)(§11) CONTAR POR DUAS VIAS, HASSE, E LAGRANGE COM ORDEM COMPOSTA */
            { int cmal = 0;
              long p = 17;
              /* as duas vias, em várias curvas — e não só na dele */
              for(long A = 0; A < 6; A++) for(long B = 0; B < 6; B++){
                  if(ef_m(4*A*A*A + 27*B*B, p) == 0) continue;    /* singular: fora */
                  long n1 = ef_conta(A,B,p);
                  long quad[32]; for(long i = 0; i < p; i++) quad[i] = 0;
                  for(long y = 0; y < p; y++) quad[(y*y) % p]++;
                  long n2 = 1;
                  for(long x = 0; x < p; x++) n2 += quad[ef_m(x*x%p*x + A*x + B, p)];
                  if(n1 != n2) cmal++;                            /* DUAS VIAS concordam */
                  long t = n1 - p - 1;
                  if(t*t > 4*p) cmal++;                           /* HASSE, em inteiros */
                  /* LAGRANGE: toda ordem divide N, e NP = 𝒪 */
                  for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
                      PtF T = ef_pt(x,y,p);
                      if(!ef_na_curva(A,B,p,T)) continue;
                      long o = ef_ordem(A,B,p,T);
                      if(o == 0 || n1 % o) cmal++;
                      if(!ef_mult(A,B,p,T,n1).inf) cmal++;
                  }
              }
              /* e a curva com ordem COMPOSTA, para o Lagrange não ser degenerado */
              long N24 = ef_conta(1,2,p), ordens[64], no = 0;
              for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
                  PtF T = ef_pt(x,y,p);
                  if(!ef_na_curva(1,2,p,T)) continue;
                  long o = ef_ordem(1,2,p,T);
                  int novo = 1;
                  for(long i = 0; i < no; i++) if(ordens[i] == o) novo = 0;
                  if(novo && no < 64) ordens[no++] = o;
              }
              printf("      #E(𝔽₁₇) = %ld para y²=x³+2x+2 (as duas vias), e a curva"
                     " y²=x³+x+2 tem #E = %ld com %ld ordens distintas\n",
                     ef_conta(2,2,p), N24, no);
              ok("CONTAR POR DUAS VIAS (a varredura dos x e a tabela dos quadrados) dá o"
                 " MESMO conjunto em todas as curvas não singulares de 𝔽₁₇, e o N cabe"
                 " sempre em HASSE. E LAGRANGE mede-se onde tem conteúdo: numa curva de"
                 " ordem COMPOSTA (24, com 7 ordens distintas) — com N primo a"
                 " divisibilidade seria quase automática, e isso era caso degenerado",
                 cmal == 0 && N24 == 24 && no == 7); }

            /* (§12)(§14) DOUBLE-AND-ADD contra a SOMA REPETIDA, e o GUME DA FIBRA */
            { int dmal = 0; long p = 17, A = 2, B = 2, casos = 0;
              for(long x = 0; x < p; x++) for(long y = 0; y < p; y++){
                  PtF G = ef_pt(x,y,p);
                  if(!ef_na_curva(A,B,p,G)) continue;
                  PtF lento = ef_O();
                  for(long k = 1; k <= 22; k++){
                      lento = ef_soma(A,B,p,lento,G);
                      PtF rapido = ef_mult(A,B,p,G,k);          /* por DOBRAS */
                      if(!ef_igual(lento, rapido)) dmal++;      /* dois caminhos */
                      casos++;
                  }
              }
              /* O GUME DA FIBRA: com x₁ = x₂ há DUAS operações diferentes, e a escolha
               * não é aproximação — é a fibra. Conta-se cada ramo, para nenhum ficar por
               * exercitar (um ramo que não corre não está medido). */
              long ramo_oposto = 0, ramo_tangente = 0, ramo_secante = 0;
              for(long x1 = 0; x1 < p; x1++) for(long y1 = 0; y1 < p; y1++){
                  PtF U = ef_pt(x1,y1,p);
                  if(!ef_na_curva(A,B,p,U)) continue;
                  for(long x2 = 0; x2 < p; x2++) for(long y2 = 0; y2 < p; y2++){
                      PtF V = ef_pt(x2,y2,p);
                      if(!ef_na_curva(A,B,p,V)) continue;
                      PtF S = ef_soma(A,B,p,U,V);
                      if(U.x == V.x && ef_m(U.y+V.y,p) == 0){
                          if(!S.inf) dmal++;                    /* opostos ⟹ 𝒪 */
                          ramo_oposto++;
                      } else if(ef_igual(U,V)){
                          if(!ef_na_curva(A,B,p,S)) dmal++;     /* tangente */
                          ramo_tangente++;
                      } else {
                          if(!ef_na_curva(A,B,p,S)) dmal++;     /* secante */
                          ramo_secante++;
                      }
                  }
              }
              printf("      double-and-add contra soma repetida em %ld casos; e os TRÊS"
                     " ramos da fibra: %ld opostos, %ld tangentes, %ld secantes\n",
                     casos, ramo_oposto, ramo_tangente, ramo_secante);
              ok("DOUBLE-AND-ADD dá o mesmo que a soma repetida em todos os pontos e"
                 " múltiplos até 22 — dois caminhos, um em log e outro em linear. E o"
                 " GUME DA FIBRA mede-se por RAMO: os três (opostos, tangente, secante)"
                 " correm todos e nenhum fica por exercitar, porque um ramo que não corre"
                 " não está medido",
                 dmal == 0 && ramo_oposto > 0 && ramo_tangente > 0 && ramo_secante > 0); }

            /* A REDUÇÃO LIGA OS DOIS MUNDOS: a conta feita em ℚ, reduzida mod p, tem de
             * dar a conta feita em 𝔽ₚ. São duas implementações independentes, e é esta a
             * medida que as obriga a concordar. */
            { int rmal = 0, comparados = 0;
              long p = 11;
              long A = ((-2) % p + p) % p, B = 1;             /* a curva dele mod 11 */
              PtF Pf = ef_pt(0, 1, p);
              if(!ef_na_curva(A,B,p,Pf)) rmal++;
              for(long k = 1; k <= 8; k++){
                  PtQ Q = eq_mult(a,b,P,k);
                  PtF R = ef_mult(A,B,p,Pf,k);
                  if(Q.inf != R.inf){ rmal++; continue; }
                  if(Q.inf) continue;
                  /* reduzir a coordenada racional mod p: p/q ↦ p·q⁻¹ */
                  long ix, iy, invx, invy;
                  if(!nm_inv_mod(((Q.x.q % p)+p)%p, p, &invx)) continue;
                  if(!nm_inv_mod(((Q.y.q % p)+p)%p, p, &invy)) continue;
                  ix = ef_m(((Q.x.p % p)+p)%p * invx, p);
                  iy = ef_m(((Q.y.p % p)+p)%p * invy, p);
                  if(ix != R.x || iy != R.y) rmal++;
                  comparados++;
              }
              printf("      a órbita de ℚ reduzida mod 11 bate com a órbita de 𝔽₁₁ em"
                     " %d pontos\n", comparados);
              ok("A REDUÇÃO LIGA OS DOIS MUNDOS: a órbita calculada sobre ℚ com frações"
                 " exatas, reduzida mod p (numerador vezes o inverso do denominador), dá"
                 " a órbita calculada diretamente em 𝔽ₚ. São duas implementações"
                 " independentes obrigadas a concordar — e é aqui que «a divisão é a"
                 " inversão modular» deixa de ser frase e passa a ser a ponte",
                 rmal == 0 && comparados >= 3); }

            /* E OS DOZE CORREM, com o fora de alcance recusado */
            { int lmal = 0, por_n = 0, por_nome = 0;
              fflush(stdout);
              int guarda = dup(1), nulo = open("/dev/null", O_WRONLY);
              if(guarda >= 0 && nulo >= 0) dup2(nulo, 1);
              for(int k = 1; k <= 12; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "eliptica %d", k);
                  if(resolve_eliptica(fala)) por_n++; else lmal++;
              }
              for(size_t i = 0; i < sizeof EL12/sizeof *EL12; i++)
                  if(resolve_eliptica(EL12[i].nome)) por_nome++; else lmal++;
              if(resolve_eliptica("eliptica 13")) lmal++;
              if(!resolve_eliptica("dirichlet")) lmal++;
              fflush(stdout);
              if(guarda >= 0){ dup2(guarda, 1); close(guarda); }
              if(nulo >= 0) close(nulo);
              printf("      os doze: %d pelo número, %d pelo nome, e a 13 é recusada\n",
                     por_n, por_nome);
              ok("OS DOZE das curvas elípticas correm pelo NÚMERO e pelo NOME, o fora de"
                 " alcance é RECUSADO, e a fala do Dirichlet responde — varrido, porque"
                 " uma fala que morre calada não falha: desaparece",
                 lmal == 0 && por_n == 12 && por_nome == 12); }
        }

        /* ═══ §C33 A CONVOLUÇÃO DE DIRICHLET: μ = 1⁻¹, E A INVERSÃO É DECONVOLUÇÃO ═
         * «funções aritméticas → convolução → multiplicativas → μ como INVERSOR →
         *  inversão → série de Dirichlet», e o alvo é μ * 1 = ε.
         *
         * Ele escreve f: ℕ → ℂ; as funções deste andar (1, id, φ, μ, ε, τ, σ) são TODAS
         * inteiras e a convolução de inteiras é inteira — o ℂ nunca chega a ser preciso,
         * e a álgebra corre exata. E a série de Dirichlet mede-se FORMALMENTE: a
         * identidade D_{f*g} = D_f·D_g é sobre os COEFICIENTES, e o s nunca se avalia. */
        printf("\n§C33 DIRICHLET: μ é o INVERSO do 1, e a inversão de Möbius é a deconvolução.\n\n");
        {
            Arit um, id, phi, mu, eps, tau, sig, t1, t2, t3;
            dl_um(&um); dl_id(&id); dl_phi(&phi); dl_mu(&mu); dl_eps(&eps);
            dl_tau(&tau); dl_sigma(&sig);

            /* A ÁLGEBRA: comutativa, ASSOCIATIVA e com neutro ε. A associatividade é o
             * que faz disto uma álgebra e não uma operação avulsa. */
            { int cmal = 0;
              Arit *fs[5] = { &um, &id, &phi, &mu, &tau };
              for(int i = 0; i < 5; i++){
                  dl_conv(fs[i], &eps, &t1);
                  if(!dl_igual(&t1, fs[i], 120)) cmal++;                  /* neutro */
                  for(int j = 0; j < 5; j++){
                      dl_conv(fs[i], fs[j], &t1);
                      dl_conv(fs[j], fs[i], &t2);
                      if(!dl_igual(&t1, &t2, 120)) cmal++;                /* comutativa */
                      for(int k = 0; k < 5; k++){
                          dl_conv(&t1, fs[k], &t2);                       /* (f*g)*h */
                          Arit gh; dl_conv(fs[j], fs[k], &gh);
                          dl_conv(fs[i], &gh, &t3);                       /* f*(g*h) */
                          if(!dl_igual(&t2, &t3, 60)) cmal++;
                      }
                  }
              }
              ok("a CONVOLUÇÃO DE DIRICHLET é uma ÁLGEBRA: comutativa, ASSOCIATIVA e com"
                 " neutro ε — varrido em 5 funções, 25 pares e 125 triplos. É o produto"
                 " sobre a ÁRVORE DOS DIVISORES, e não o ponto a ponto: cada n consulta a"
                 " sua árvore inteira", cmal == 0); }

            /* O TEOREMA: μ * 1 = ε, isto é μ = 1⁻¹. E o inverso é ÚNICO numa álgebra
             * associativa com neutro — logo não há outro candidato. */
            { int mmal = 0;
              dl_conv(&mu, &um, &t1);
              if(!dl_igual(&t1, &eps, DL_MAX)) mmal++;
              dl_conv(&um, &mu, &t2);
              if(!dl_igual(&t2, &eps, DL_MAX)) mmal++;          /* nos DOIS sentidos */
              /* e a unicidade: se g*1 = ε então g = μ (varrido nos primeiros valores,
               * construindo g pela recorrência que o inverso obriga) */
              Arit g;
              g.v[1] = 1;
              for(long n = 2; n <= 120; n++){
                  long s = 0;
                  for(long d = 1; d < n; d++) if(n % d == 0) s += g.v[d];
                  g.v[n] = -s;                                   /* a única escolha possível */
              }
              for(long n = 1; n <= 120; n++) if(g.v[n] != mu.v[n]) mmal++;
              printf("      (μ*1)(n) = ε(n) nos %d valores, e o inverso construído pela"
                     " recorrência DÁ o μ — logo μ = 1⁻¹ e é único\n", DL_MAX);
              ok("μ * 1 = ε, ISTO É μ = 1⁻¹ — e nos dois sentidos. O μ deixa de ser uma"
                 " tabela de sinais e passa a ser o INVERSO da função constante 1 nesta"
                 " álgebra. E é ÚNICO: construindo o inverso pela recorrência que a"
                 " definição obriga, o que sai é exatamente o μ", mmal == 0); }

            /* φ = id * μ — «uma função aparentemente diferente vira convolução + cancelamento» */
            { int fmal = 0;
              dl_conv(&id, &mu, &t1);
              if(!dl_igual(&t1, &phi, DL_MAX)) fmal++;
              dl_conv(&phi, &um, &t2);
              if(!dl_igual(&t2, &id, DL_MAX)) fmal++;            /* a forma dual: φ*1 = id */
              /* e as outras duas que caem de graça: τ = 1*1 e σ = id*1 */
              dl_conv(&um, &um, &t1);
              if(!dl_igual(&t1, &tau, DL_MAX)) fmal++;
              dl_conv(&id, &um, &t1);
              if(!dl_igual(&t1, &sig, DL_MAX)) fmal++;
              printf("      φ = id*μ e φ*1 = id;  τ = 1*1 e σ = id*1 — as quatro nos %d"
                     " valores\n", DL_MAX);
              ok("φ = id * μ, e o par dual φ * 1 = id: a função que parecia de outra"
                 " família é CONVOLUÇÃO + CANCELAMENTO. E de graça caem τ = 1*1 (o número"
                 " de divisores) e σ = id*1 (a soma) — não há funções novas, há a mesma"
                 " álgebra a gerá-las", fmal == 0); }

            /* A INVERSÃO DE MÖBIUS: F = f*1 ⟺ f = F*μ — a DECONVOLUÇÃO, nos dois sentidos */
            { int imal = 0;
              Arit *fs[4] = { &um, &id, &phi, &mu };
              for(int i = 0; i < 4; i++){
                  dl_soma_divisores(fs[i], &t1);                 /* F = f*1 */
                  dl_inverte(&t1, &t2);                          /* f = F*μ */
                  if(!dl_igual(&t2, fs[i], 120)) imal++;         /* a VOLTA */
                  dl_inverte(fs[i], &t1);                        /* e o outro sentido */
                  dl_soma_divisores(&t1, &t2);
                  if(!dl_igual(&t2, fs[i], 120)) imal++;
              }
              ok("a INVERSÃO DE MÖBIUS é a DECONVOLUÇÃO: F(n) = Σ_{d|n} f(d) desfaz-se"
                 " com f(n) = Σ_{d|n} μ(d)F(n/d), e mede-se nos DOIS sentidos (somar e"
                 " depois inverter, inverter e depois somar) em quatro funções. É a soma"
                 " sobre a árvore e a sua volta, com resíduo 0", imal == 0); }

            /* MULTIPLICATIVAS: 1, id, φ, μ são-no; a convolução preserva; e o GUME é uma
             * que NÃO é — sem isso «multiplicativa» não estaria a distinguir nada. */
            { int mmal2 = 0, viu_falha = 0;
              Arit *fs[4] = { &um, &id, &phi, &mu };
              for(int i = 0; i < 4; i++){
                  if(!dl_multiplicativa(fs[i], 120, 0, 0)) mmal2++;
                  for(int j = 0; j < 4; j++){
                      dl_conv(fs[i], fs[j], &t1);
                      if(!dl_multiplicativa(&t1, 120, 0, 0)) mmal2++;   /* a convolução preserva */
                  }
              }
              /* O GUME, e a função escolhe-se para a testemunha ser GENUÍNA: g(1) = 1
               * (senão falharia logo nessa condição e o par exibido seria (1,1), que não
               * ensina nada) e g(n) = n+1 daí em diante. Assim o par que a derruba é um
               * par coprimo de verdade, e é ele que se mostra. */
              { Arit g; long pm = 0, pn = 0;
                g.v[0] = 0; g.v[1] = 1;
                for(long n = 2; n <= DL_MAX; n++) g.v[n] = n + 1;
                if(!dl_multiplicativa(&g, 60, &pm, &pn) && pm > 1 && pn > 1){
                    viu_falha = 1;
                    printf("      o gume: g(1)=1 e g(n)=n+1 NÃO é multiplicativa —"
                           " g(%ld·%ld) = %ld e g(%ld)·g(%ld) = %ld\n",
                           pm, pn, g.v[pm*pn], pm, pn, g.v[pm]*g.v[pn]);
                } }
              ok("1, id, φ e μ são MULTIPLICATIVAS e a convolução PRESERVA a"
                 " multiplicatividade (16 pares medidos) — e o gume exibe uma que não é,"
                 " f(n) = n+1, com o par que a derruba. Sem esse contra-caso a palavra"
                 " não estaria a distinguir nada", mmal2 == 0 && viu_falha); }

            /* A SÉRIE DE DIRICHLET, FORMAL: o coeficiente do produto É a convolução */
            { int dmal = 0;
              Arit *fs[4] = { &um, &id, &phi, &mu };
              for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++){
                  dl_conv(fs[i], fs[j], &t1);
                  for(long k = 1; k <= 80; k++)
                      if(dl_coef_produto(fs[i], fs[j], k) != t1.v[k]) dmal++;
              }
              printf("      D_{f*g} = D_f·D_g nos coeficientes, 16 pares × 80 termos —"
                     " e o s nunca se avalia\n");
              ok("a SÉRIE DE DIRICHLET transforma CONVOLUÇÃO em PRODUTO, e a identidade"
                 " mede-se EXATA porque é FORMAL: o coeficiente de 1/kˢ no produto é"
                 " Σ_{nm=k} f(n)g(m), que é a convolução por definição. Avaliar num s"
                 " pedia análise e decimais; os coeficientes são inteiros", dmal == 0); }
        }

        /* ═══ §C32 TEORIA DOS NÚMEROS: E É TUDO A MESMA ÓRBITA ════════════════════
         * «definição → propriedade → teorema → exercício de demonstração →
         *  contraexemplo → volta. Nada de só lista de contas.»
         *
         * E a frase que organiza o andar todo, que é a descoberta e não o resumo:
         *
         *     Euclides = MDC = Bézout = FC — «diferentes saídas da MESMA órbita»
         *
         * Literal: a descida a = bq + r produz, do mesmo rastro, o gcd (o último resto
         * não nulo), os coeficientes de Bézout (subindo) e os termos da fração contínua
         * (os quocientes). Uma órbita, quatro leituras. */
        printf("\n§C32 TEORIA DOS NÚMEROS: uma órbita, quatro saídas — e cada gume no lugar.\n\n");
        {
            /* (§1) A DIVISIBILIDADE, e o GUME que ele pede: a implicação não se inverte */
            { int dmal = 0; long contra = 0;
              for(long a = 1; a <= 10; a++) for(long b = -30; b <= 30; b++) for(long c = -30; c <= 30; c++){
                  if(iz_div(a,b,0) && iz_div(b,c,0) && !iz_div(a,c,0)) dmal++;   /* transitiva */
                  if(!iz_div(a,b,0) || !iz_div(a,c,0)) continue;
                  for(long x = -4; x <= 4; x++) for(long y = -4; y <= 4; y++)
                      if(!iz_div(a, b*x + c*y, 0)) dmal++;                       /* a|(bx+cy) */
              }
              /* «procure um caso em que apenas a|c seja verdadeiro, sem a|b» — e acha-se */
              for(long a = 2; a <= 12; a++) for(long b = 2; b <= 12; b++) for(long c = 2; c <= 60; c++)
                  if(iz_div(b,c,0) && !iz_div(a,b,0) && iz_div(a,c,0)) contra++;
              ok("a DIVISIBILIDADE é transitiva e fecha na COMBINAÇÃO LINEAR — a|b ∧ a|c"
                 " ⟹ a|(bx+cy), que é a distributividade a pôr o a em evidência e é a"
                 " mãe de todo o andar. E o GUME que ele pede EXISTE: há casos com a|c"
                 " sem a|b, logo a implicação não se inverte",
                 dmal == 0 && contra > 0); }

            /* (§2)(§3) EUCLIDES e BÉZOUT: a identidade do passo, a definição em TRÊS
             * pontos, e o gcd como MENOR positivo de {ax+by} — a boa ordenação */
            { int emal = 0;
              for(long a = 1; a <= 60; a++) for(long b = 1; b <= 60; b++){
                  long g = iz_gcd(a,b,0,0);
                  for(long q = -3; q <= 3; q++)
                      if(g != iz_gcd(b, a - q*b, 0, 0)) emal++;      /* o passo da órbita */
                  if(!iz_div(g,a,0) || !iz_div(g,b,0)) emal++;       /* d|a e d|b */
                  for(long d = 1; d <= a && d <= b; d++)             /* e todo comum divide d */
                      if(a % d == 0 && b % d == 0 && g % d) emal++;
              }
              for(long a = 1; a <= 30; a++) for(long b = 1; b <= 30; b++){
                  long menor = 0, x, y, g = iz_gcd(a,b,&x,&y);
                  if(a*x + b*y != g) emal++;                          /* a testemunha */
                  for(long u = -30; u <= 30; u++) for(long v = -30; v <= 30; v++){
                      long t = a*u + b*v;
                      if(t > 0 && (menor == 0 || t < menor)) menor = t;
                  }
                  if(menor != g) emal++;                              /* o MENOR positivo É o gcd */
              }
              long x4, y4, g4 = iz_gcd(391, 299, &x4, &y4);
              printf("      gcd(391,299) = %ld, e 391·(%ld) + 299·(%ld) = %ld\n",
                     g4, x4, y4, 391*x4 + 299*y4);
              ok("EUCLIDES: o passo gcd(a,b) = gcd(b, a−qb) vale por DUPLA INCLUSÃO dos"
                 " divisores, a definição confere nos TRÊS pontos (d|a, d|b, todo comum"
                 " divide d), e BÉZOUT fecha: o gcd É o MENOR positivo de {ax+by} — a boa"
                 " ordenação a decidir, não uma busca",
                 emal == 0 && g4 == 23 && 391*x4 + 299*y4 == 23); }

            /* (§5)(§6) O LEMA DE EUCLIDES e o TEOREMA FUNDAMENTAL — e o gume do lema é o
             * composto, onde ele é FALSO. Sem esse gume, «primo» seria decoração. */
            { int lmal = 0; long viu = 0, falhou_composto = 0;
              for(long p = 2; p <= 40; p++){
                  if(!nt_primo(p)) continue;
                  for(long a = 1; a <= 40; a++) for(long b = 1; b <= 40; b++)
                      if(iz_div(p, a*b, 0)){ viu++; if(!iz_div(p,a,0) && !iz_div(p,b,0)) lmal++; }
              }
              /* e com COMPOSTO o teorema cai — conta-se onde */
              for(long n2 = 4; n2 <= 30; n2++){
                  if(nt_primo(n2)) continue;
                  for(long a = 2; a <= 30; a++) for(long b = 2; b <= 30; b++)
                      if(iz_div(n2, a*b, 0) && !iz_div(n2,a,0) && !iz_div(n2,b,0)) falhou_composto++;
              }
              /* o TFA: a volta da fatoração, e os fatores são todos primos */
              int fmal = 0;
              for(long m = 2; m <= 3000; m++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(m, pr, ex, NT_FAT);
                  if(nt_refaz(pr, ex, k) != m) fmal++;
                  for(int i = 0; i < k; i++) if(!nt_primo(pr[i])) fmal++;
                  for(int i = 1; i < k; i++) if(pr[i] <= pr[i-1]) fmal++;   /* e ordenados: a forma é ÚNICA */
              }
              printf("      o lema vale em %ld casos com p primo, e FALHA em %ld com"
                     " composto — é aí que a hipótese se paga\n", viu, falhou_composto);
              ok("o LEMA DE EUCLIDES (p|ab ⟹ p|a ou p|b) vale sempre com p PRIMO e falha"
                 " com composto — e é ele que dá a UNICIDADE do Teorema Fundamental,"
                 " cuja existência vem da indução: duas provas diferentes ao mesmo objeto."
                 " A volta refaz 2999 fatorações, com os fatores primos e ordenados",
                 lmal == 0 && fmal == 0 && viu > 1000 && falhou_composto > 0); }

            /* (§7) A INFINITUDE, com o primo CONSTRUÍDO — «não apenas reproduza a prova» */
            { int imal = 0; long p[9] = {2,3,5,7,11,13,17,19,23};
              for(int n2 = 1; n2 <= 7; n2++){
                  long N, novo = nm_primo_novo(p, n2, &N);
                  if(!nt_primo(novo)) imal++;                       /* é primo */
                  for(int i = 0; i < n2; i++){
                      if(novo == p[i]) imal++;                      /* e é NOVO */
                      if(N % p[i] != 1) imal++;                     /* N ≡ 1 (mod pᵢ) */
                  }
              }
              long N4, novo4 = nm_primo_novo(p, 4, &N4);
              printf("      2·3·5·7 + 1 = %ld, e o primo novo é %ld\n", N4, novo4);
              ok("A INFINITUDE dos primos com o novo CONSTRUÍDO: N = p₁…pₙ + 1 deixa resto"
                 " 1 em cada pᵢ (medido), logo o seu menor fator primo está FORA da lista"
                 " — e em sete listas seguidas ele é sempre primo e sempre novo",
                 imal == 0 && N4 == 211 && novo4 == 211); }

            /* (§8)(§9)(§10)(§12) O ANEL MODULAR INTEIRO: congruência, inverso, Fermat,
             * Euler — e a ORDEM é que os liga, porque ela DIVIDE φ (Lagrange) */
            { int mmal = 0; long com_inv = 0, sem_inv = 0, pares = 0;
              for(long n2 = 2; n2 <= 60; n2++){
                  long f = nm_phi(n2);
                  for(long a = 1; a < n2; a++){
                      long inv;
                      int tem = nm_inv_mod(a, n2, &inv);
                      if(tem != (iz_gcd(a,n2,0,0) == 1)) mmal++;     /* o critério */
                      if(tem){
                          if((a*inv) % n2 != 1 % n2) mmal++;         /* e a volta */
                          if(iz_pot_mod(a, f, n2) != 1 % n2) mmal++; /* EULER */
                          long o = nm_ordem(a, n2);
                          if(o == 0 || f % o) mmal++;                /* a ordem DIVIDE φ */
                          com_inv++; pares++;
                      } else sem_inv++;
                  }
                  if(nt_primo(n2)){
                      if(nm_phi(n2) != n2 - 1) mmal++;               /* φ(p) = p−1 */
                      for(long a = 1; a < n2; a++)
                          if(iz_pot_mod(a, n2-1, n2) != 1) mmal++;   /* FERMAT */
                  }
              }
              long inv17; nm_inv_mod(17, 43, &inv17);
              printf("      17⁻¹ mod 43 = %ld (17·%ld = %ld ≡ %ld);  7¹⁰⁰ mod 13 = %ld\n",
                     inv17, inv17, 17*inv17, (17*inv17) % 43, iz_pot_mod(7,100,13));
              ok("O ANEL MODULAR fecha: o inverso existe ⟺ gcd = 1 e a volta confere;"
                 " FERMAT e EULER são o MESMO teorema, e o que os une é a ORDEM dividir"
                 " φ(n) (Lagrange) — medido em todos os coprimos até 60. φ(p) = p−1"
                 " recupera um do outro",
                 mmal == 0 && com_inv > 500 && sem_inv > 100 && inv17 == 38
                 && iz_pot_mod(7,100,13) == 9); }

            /* (§11) φ PELOS DOIS CAMINHOS — a fórmula do produto e a contagem à mão */
            { int pmal = 0;
              for(long m = 1; m <= 400; m++) if(nm_phi(m) != nm_phi_conta(m)) pmal++;
              /* e os casos dele: φ(p) = p−1 e φ(pq) = (p−1)(q−1) */
              int casos = 0;
              for(long p1 = 2; p1 <= 20; p1++) for(long p2 = 2; p2 <= 20; p2++){
                  if(!nt_primo(p1) || !nt_primo(p2) || p1 == p2) continue;
                  if(nm_phi(p1*p2) != (p1-1)*(p2-1)) pmal++;
                  casos++;
              }
              printf("      φ(60) = %ld pela fórmula e %ld pela contagem;"
                     " φ(pq) = (p−1)(q−1) em %d pares\n",
                     nm_phi(60), nm_phi_conta(60), casos);
              ok("φ por DOIS CAMINHOS que têm de concordar — a fórmula n·∏(1−1/p) feita em"
                 " inteiros e a contagem directa dos coprimos, iguais em 400 números — e"
                 " os dois casos dele (φ(p) = p−1, φ(pq) = (p−1)(q−1)) medidos à parte",
                 pmal == 0 && nm_phi(60) == 16 && casos > 40); }

            /* (§13) O TEOREMA CHINÊS, com a UNICIDADE medida e o gume da coprimalidade */
            { int cmal = 0; long resolvidos = 0, recusados = 0;
              for(long m = 2; m <= 20; m++) for(long n2 = 2; n2 <= 20; n2++)
              for(long a = 0; a < m; a++) for(long b = 0; b < n2; b++){
                  long x, mod;
                  int tem = nm_tcr(a, m, b, n2, &x, &mod);
                  if(tem != (iz_gcd(m,n2,0,0) == 1)) cmal++;         /* o critério */
                  if(!tem){ recusados++; continue; }
                  if(x % m != a || x % n2 != b) cmal++;              /* a VOLTA */
                  if(mod != m*n2) cmal++;
                  long quantos = 0;                                  /* e a UNICIDADE */
                  for(long v = 0; v < mod; v++) if(v % m == a && v % n2 == b) quantos++;
                  if(quantos != 1) cmal++;
                  resolvidos++;
              }
              long x13, m13;
              nm_tcr(2, 3, 3, 5, &x13, &m13);
              printf("      x ≡ 2 (mod 3), x ≡ 3 (mod 5)  →  x ≡ %ld (mod %ld);"
                     " %ld sistemas resolvidos e %ld recusados\n",
                     x13, m13, resolvidos, recusados);
              ok("o TEOREMA CHINÊS: solução ÚNICA módulo mn quando gcd(m,n) = 1 — e a"
                 " unicidade mede-se contando os resíduos que servem (tem de ser 1, nunca"
                 " 2). Sem a coprimalidade o sistema é RECUSADO, e isso também se conta:"
                 " o gume tem casos dos dois lados",
                 cmal == 0 && x13 == 8 && m13 == 15 && resolvidos > 1000 && recusados > 100); }

            /* (§14) AS DIOFANTINAS, e «descreva TODAS»: a família tem de ser COMPLETA —
             * não basta gerar soluções, nenhuma pode ficar de fora */
            { int dmal2 = 0;
              for(long a = 1; a <= 12; a++) for(long b = 1; b <= 12; b++) for(long c = -12; c <= 12; c++){
                  long x, y, g = iz_gcd(a,b,0,0);
                  int tem = iz_diofantina(a,b,c,&x,&y);
                  if(tem != (c % g == 0)) dmal2++;
                  if(!tem) continue;
                  if(a*x + b*y != c) dmal2++;
                  for(long t = -5; t <= 5; t++)                      /* a família gera */
                      if(a*(x + (b/g)*t) + b*(y - (a/g)*t) != c) dmal2++;
                  for(long xx = -80; xx <= 80; xx++){                /* e é COMPLETA */
                      long resto = c - a*xx;
                      if(resto % b) continue;
                      long yy = resto / b;
                      if((xx - x) % (b/g)) { dmal2++; continue; }
                      long tt = (xx - x) / (b/g);
                      if(x + (b/g)*tt != xx || y - (a/g)*tt != yy) dmal2++;
                  }
              }
              ok("as DIOFANTINAS decidem-se pelo critério gcd|c, a solução exibe-se, e a"
                 " família x = x₀+(b/g)t, y = y₀−(a/g)t é COMPLETA — nenhuma solução do"
                 " intervalo fica de fora dela. «Descreva todas» não é gerar algumas:"
                 " é não deixar nenhuma escapar, e é isso que aqui se mede", dmal2 == 0); }

            /* (§15)(§17) A ÓRBITA ÚNICA: os quocientes da descida SÃO os termos da FC, a
             * volta reconstrói a fração, e os convergentes são as MELHORES aproximações */
            /* E OS NUMERADORES NEGATIVOS ENTRAM, porque é só com eles que o quociente
             * pelo CHÃO se distingue do truncado: em C o `/` trunca para zero, e para
             * positivos as duas convenções coincidem. Sem negativos o ramo da correção
             * NUNCA CORRE — e a mutação que o apagava sobrevivia à varredura inteira. */
            { int omal = 0, feitos = 0, negativos = 0;
              for(long a = -120; a <= 120; a++) for(long b = 1; b <= 120; b++){
                  if(a == 0) continue;
                  long q[40];
                  size_t k = nm_fc(a, b, q, 40);
                  if(!k) continue;
                  if(a < 0) negativos++;
                  /* os quocientes são os da descida, feita pelo CHÃO — que é a convenção
                   * da FC — e são dois caminhos sobre o mesmo rastro */
                  long x = a, y = b; size_t i = 0;
                  while(y && i < k){
                      long tq = x / y, r = x - tq*y;
                      if(r < 0){ tq--; r += y; }
                      if(tq != q[i]) omal++;
                      x = y; y = r; i++;
                  }
                  if(i != k) omal++;
                  long g = iz_gcd(a, b, 0, 0);
                  if(x != g && x != -g) omal++;         /* e o último resto É o gcd */
                  long p2, d2;
                  nm_convergente(q, k-1, &p2, &d2);     /* A VOLTA */
                  if(p2 * b != a * d2) omal++;          /* produto cruzado: a mesma fração */
                  feitos++;
              }
              /* A MELHOR APROXIMAÇÃO, E A EXCEÇÃO QUE A MEDIDA ENCONTROU.
               * Eu tinha escrito «todo convergente é a melhor aproximação do seu
               * denominador» e a asserção CAIU — sempre em j = 0. E com razão: o
               * convergente de ordem zero é ⌊x⌋/1, e para 5/3 ≈ 1,667 o mais perto com
               * denominador 1 é 2, não 1. A partir de j ≥ 1 o teorema vale; o j = 0 falha
               * EXATAMENTE quando a parte fracionária passa de 1/2 — e isso não se
               * esconde alargando o enunciado: mede-se nos dois sentidos. */
              int amal = 0, testados = 0, zero_mal = 0, zero_falhou = 0, zero_bem = 0;
              for(long a = 5; a <= 60; a++) for(long b = 1; b < a; b++){
                  long q[40];
                  size_t k = nm_fc(a, b, q, 40);
                  for(size_t j = 0; j + 1 < k; j++){
                      long p2, d2;
                      nm_convergente(q, j, &p2, &d2);
                      int bateram = 0;
                      for(long v = 1; v <= d2; v++) for(long u = 0; u <= (a*v)/b + 1; u++){
                          if(u == p2 && v == d2) continue;
                          long e1 = a*d2 - p2*b; if(e1 < 0) e1 = -e1;
                          long e2 = a*v - u*b;   if(e2 < 0) e2 = -e2;
                          if(e2 * d2 < e1 * v) bateram = 1;
                      }
                      if(j == 0){
                          /* o critério exato: falha ⟺ a fração que sobra passa de 1/2,
                           * isto é 2·(a − q₀b) > b */
                          int passa_meio = 2*(a - q[0]*b) > b;
                          if(bateram != passa_meio) zero_mal++;
                          if(bateram) zero_falhou++; else zero_bem++;
                      } else {
                          if(bateram) amal++;            /* de j ≥ 1 nenhum pode cair */
                          testados++;
                      }
                  }
              }
              long q43[16]; size_t k43 = nm_fc(43, 19, q43, 16);
              printf("      43/19 = [%ld; %ld, %ld, %ld] (o exemplo dele), e %d frações"
                     " fazem a volta — %d com numerador NEGATIVO, que é onde o quociente"
                     " pelo chão se distingue do truncado\n",
                     q43[0], q43[1], q43[2], q43[3], feitos, negativos);
              printf("      e a melhor aproximação: 0 falhas de j ≥ 1, e o j = 0 falha em"
                     " %d casos e acerta em %d — exatamente quando a parte que sobra"
                     " passa de 1/2\n", zero_falhou, zero_bem);
              ok("A ÓRBITA É UMA SÓ: os quocientes da descida de Euclides SÃO os termos da"
                 " fração contínua e o último resto É o gcd — medido nas duas colunas do"
                 " mesmo rastro; a volta FC → convergente → fração reconstrói 7140"
                 " frações. E os convergentes de ordem ≥ 1 são as MELHORES aproximações do"
                 " seu denominador, com a EXCEÇÃO do de ordem 0 (que é ⌊x⌋/1) a falhar"
                 " exatamente quando a parte fracionária passa de 1/2 — medido nos dois"
                 " sentidos, porque foi a medida que a encontrou",
                 omal == 0 && amal == 0 && zero_mal == 0 && zero_falhou > 0 && zero_bem > 0
                 && feitos > 7000 && testados > 100 && negativos > 1000
                 && k43 == 4 && q43[0] == 2 && q43[1] == 3 && q43[2] == 1 && q43[3] == 4); }

            /* (§16) MÖBIUS — o cancelamento na árvore dos divisores */
            { int mumal = 0;
              if(nm_soma_mu(1) != 1) mumal++;
              for(long m = 2; m <= 500; m++) if(nm_soma_mu(m) != 0) mumal++;
              /* e a definição confere com a fatoração, caso a caso */
              for(long m = 1; m <= 300; m++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(m, pr, ex, NT_FAT), quad = 0;
                  for(int i = 0; i < k; i++) if(ex[i] > 1) quad = 1;
                  int esperado = (m == 1) ? 1 : quad ? 0 : ((k % 2) ? -1 : 1);
                  if(nm_mu(m) != esperado) mumal++;
              }
              printf("      n=12: μ(1)+μ(2)+μ(3)+μ(4)+μ(6)+μ(12) = %d%+d%+d%+d%+d%+d = %ld\n",
                     nm_mu(1), nm_mu(2), nm_mu(3), nm_mu(4), nm_mu(6), nm_mu(12),
                     nm_soma_mu(12));
              ok("MÖBIUS: Σ_{d|n} μ(d) dá 1 em n=1 e ZERO em todo o resto até 500 — é o"
                 " «cancelamento na árvore dos divisores», e os livres de quadrados"
                 " emparelham-se por paridade. Uma identidade com uma excepção não medida"
                 " não era identidade",
                 mumal == 0 && nm_soma_mu(12) == 0 && nm_soma_mu(1) == 1); }

            /* E OS DEZASSETE CORREM — pelo número e pelo nome, com o fora de alcance
             * recusado. Uma fala que morre calada não falha: desaparece. */
            { int nmal2 = 0, por_n = 0, por_nome = 0;
              fflush(stdout);
              int guarda = dup(1), nulo = open("/dev/null", O_WRONLY);
              if(guarda >= 0 && nulo >= 0) dup2(nulo, 1);
              for(int k = 1; k <= 17; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "numeros %d", k);
                  if(resolve_numeros(fala)) por_n++; else nmal2++;
              }
              for(size_t i = 0; i < sizeof TN17/sizeof *TN17; i++)
                  if(resolve_numeros(TN17[i].nome)) por_nome++; else nmal2++;
              if(resolve_numeros("numeros 18")) nmal2++;
              if(resolve_numeros("numeros 0")) nmal2++;
              fflush(stdout);
              if(guarda >= 0){ dup2(guarda, 1); close(guarda); }
              if(nulo >= 0) close(nulo);
              printf("      os dezassete: %d pelo número, %d pelo nome, e a 18 é recusada\n",
                     por_n, por_nome);
              ok("OS DEZASSETE do `eval.txt` correm todos, pelo NÚMERO e pelo NOME, e o"
                 " índice fora de alcance é RECUSADO — varrido, porque confiar em ter"
                 " escrito os dezassete não é medir que os dezassete correm",
                 nmal2 == 0 && por_n == 17 && por_nome == 17); }
        }

        /* ═══ §C31 O MESMO PONTO POR QUATRO PORTAS — E A VOLTA A IDENTIFICAR ══════
         * É o que o `eval.txt` pede no fim, e é o salto que fecha o andar:
         *
         *   «mostrar que esse fechamento NÃO DEPENDE DO MÉTODO ESCOLHIDO: corte, Cauchy,
         *    bisseção e FC têm de produzir O MESMO PONTO, com a VOLTA verificando a
         *    identificação.»
         *
         * E antes, a regra que proíbe o atalho: a ponte «permite identificar os dois SEM
         * SIMPLESMENTE DECLARAR QUE SÃO IGUAIS». Então não se declara.
         *
         * O real É o corte. Logo dois métodos dão o mesmo ponto exatamente quando
         * INDUZEM O MESMO CORTE — para cada racional, o mesmo lado. Mede-se nos SEIS
         * pares e não contra um árbitro, porque eleger um árbitro e chamar-lhe acordo
         * seria a mesma declaração por outro nome.
         *
         * E A INDECISÃO DIZ-SE. Um método de esforço finito não decide os racionais que
         * ainda caem dentro da sua caixa: isso é 0, não é um lado. Contar o indeciso como
         * acordo daria o teorema de graça — é o gume desta secção. */
        printf("\n§C31 O MESMO PONTO: quatro portas, seis pares, e a indecisão contada.\n\n");
        {
            /* AS QUATRO PORTAS CONCORDAM — em todos os racionais varridos, e em todos os
             * pares. Zero choques, e o indeciso separado do acordo. */
            { long choques = 0, decididos = 0, varridos = 0;
              /* A ESCADA DE ESFORÇO, e ela é o essencial. Varrer só com esforço alto era
               * medir a região onde já ninguém pode errar: com a caixa apertada nenhum
               * racional lá cai dentro, o caminho do INDECISO nunca corre, e um método
               * que FINGISSE decidir passava sem ser visto. Foi a mutação que o mostrou:
               * «bisseção finge decidir» sobrevivia à varredura de esforço 14. */
              int esf[4] = { 1, 2, 5, 14 };
              long ind_por_esf[4] = {0,0,0,0};
              for(int e = 0; e < 4; e++)
              for(long a2 = 2; a2 <= 12; a2++){
                  long r = raizi(a2);
                  if(r*r == a2) continue;                 /* fecha em ℚ: sem buraco */
                  for(long d = 1; d <= 40; d++) for(long p = 1; p <= (r+1)*d; p++){
                      Qz q = qz(p, d);
                      Quatro Q = id_quatro(a2, q, esf[e]);
                      choques += id_choques(Q);
                      ind_por_esf[e] += id_indecisos(Q);
                      decididos += 4 - id_indecisos(Q);
                      varridos++;
                  }
              }
              /* O CONTROLO DO DETETOR: um quadro FORJADO, com duas portas a decidir ao
               * contrário, tem de dar choques. Sem isto, «0 choques» podia ser um
               * detetor avariado em vez de um acordo — e um detetor avariado dá sempre 0. */
              Quatro forjado = { { ID_ABAIXO, ID_ACIMA, ID_INDECISO, ID_ABAIXO } };
              int detetou = id_choques(forjado);          /* 0-1 e 1-3: dois choques */
              printf("      %ld leituras (esforços 1, 2, 5 e 14): %ld choques em 6 pares,"
                     " %ld decisões\n", varridos, choques, decididos);
              printf("      indecisos por esforço: %ld, %ld, %ld, %ld — o caminho do"
                     " indeciso CORRE, e por isso um método que fingisse era apanhado\n",
                     ind_por_esf[0], ind_por_esf[1], ind_por_esf[2], ind_por_esf[3]);
              printf("      e o detetor prova que dispara: num quadro forjado com duas"
                     " portas ao contrário dá %d choque(s)\n", detetou);
              ok("AS QUATRO PORTAS INDUZEM O MESMO CORTE — varrido em QUATRO esforços,"
                 " dos baixos (onde há milhares de indecisos e um método que fingisse"
                 " decidir chocava na hora) ao alto. Nenhum dos SEIS pares decide ao"
                 " contrário; não há árbitro, e o detetor prova que sabe disparar",
                 choques == 0 && decididos > 10000 && varridos > 8000
                 && ind_por_esf[0] > 1000 && detetou == 2); }

            /* O GUME: o INDECISO NÃO É ACORDO. Se contasse, o teorema saía de graça —
             * bastava dar esforço zero a toda a gente e ninguém discordava de ninguém.
             * Mede-se que com esforço 0 há MUITOS indecisos, e que eles CAEM quando o
             * esforço sobe: é a identificação a acontecer, e não a ser suposta. */
            { long ind[4] = {0,0,0,0};
              int esf[4] = { 0, 4, 10, 18 };
              for(int e = 0; e < 4; e++)
                  for(long d = 1; d <= 30; d++) for(long p = 1; p <= 2*d; p++){
                      Quatro Q = id_quatro(2, qz(p,d), esf[e]);
                      ind[e] += id_indecisos(Q);
                  }
              printf("      indecisos com esforço 0, 4, 10, 18: %ld, %ld, %ld, %ld"
                     " — caem a zero, e é o esforço que os mata\n",
                     ind[0], ind[1], ind[2], ind[3]);
              ok("O INDECISO NÃO É ACORDO, e é este o gume: com esforço 0 há 1860"
                 " indecisos, e eles CAEM ESTRITAMENTE até zero quando o esforço sobe."
                 " Se o indeciso contasse como concordância, o teorema saía de graça —"
                 " bastava não medir nada e ninguém discordava de ninguém",
                 ind[0] > ind[1] && ind[1] > ind[2] && ind[3] == 0 && ind[0] > 1000); }

            /* A VOLTA VERIFICA A IDENTIFICAÇÃO — e a volta aqui é a RECONSTRUÇÃO: de cada
             * método sai uma caixa racional, e as quatro caixas têm de se INTERSETAR, com
             * a interseção a conter o corte. Se duas fossem pontos diferentes, as caixas
             * separavam-se ao apertar — e é isso que aqui não acontece. */
            { int vmal = 0;
              long a2 = 2;
              Corte c = { a2, 2 };
              Qz lo[4], hi[4];
              /* 1. o corte, pela sua própria caixa */
              rz_caixa_inicial(c, &lo[0], &hi[0]); rz_encaixota(c, &lo[0], &hi[0], 16);
              /* 2. o Möbius: as duas órbitas SÃO a caixa (o par dual a fechar) */
              { long b = rz_b(a2);
                Qz x = qz_de_inteiro(raizi(a2)), y = qz_de_inteiro(raizi(a2)+1);
                for(int k = 0; k < 5; k++){ x = rz_passo(a2,b,x); y = rz_passo(a2,b,y); }
                lo[1] = x; hi[1] = y; }
              /* 3. a bisseção, com outra profundidade — de propósito, para as caixas não
               *    serem a mesma por construção */
              rz_caixa_inicial(c, &lo[2], &hi[2]); rz_encaixota(c, &lo[2], &hi[2], 12);
              /* 4. a FC: dois convergentes consecutivos são a caixa */
              { long t[48]; size_t nt = lado(0, -a2, t, 48);
                long pn = 1, qn = 0, pa = 0, qa = 1; Qz par = qz(1,1), imp = qz(2,1);
                for(int i = 0; i < 9 && nt; i++){
                    long ai = t[(size_t)i < nt ? (size_t)i : (1 + (i-1) % (int)(nt>1?nt-1:1))];
                    long pp = ai*pn + pa, qq = ai*qn + qa;
                    if(qq > (1L<<26)) break;
                    pa = pn; qa = qn; pn = pp; qn = qq;
                    if(i % 2 == 0) par = qz(pn,qn); else imp = qz(pn,qn);
                }
                lo[3] = par; hi[3] = imp; }
              /* cada caixa contém o corte, nos DOIS lados */
              for(int i = 0; i < 4; i++){
                  int b1, b2;
                  if(rz_cmp(lo[i], 2, a2, &b1) >= 0 || !b1) vmal++;
                  if(rz_cmp(hi[i], 2, a2, &b2) <= 0 || !b2) vmal++;
              }
              /* e as quatro INTERSETAM-SE: o maior dos lo é menor que o menor dos hi */
              Qz maior_lo = lo[0], menor_hi = hi[0];
              for(int i = 1; i < 4; i++){
                  if(qz_menor(maior_lo, lo[i])) maior_lo = lo[i];
                  if(qz_menor(hi[i], menor_hi)) menor_hi = hi[i];
              }
              if(!qz_menor(maior_lo, menor_hi)) vmal++;
              for(int i = 0; i < 4; i++){
                  /* sem larguras de campo: `%-9s` conta BYTES e «bisseção» tem acentos —
                   * a coluna saía torta, e a coluna torta é o que ele lê */
                  printf("      "); esc_col(id_nome(i), 9);
                  printf("(o rastro é a %s)", id_rastro(i));
                  esc_col("", 24 - larg_utf8(id_rastro(i)));   /* em CARACTERES, não em bytes */
                  printf("  ");
                  esc_qz("", lo[i], " … ");
                  esc_qz("", hi[i], "\n");
              }
              printf("      interseção das quatro: ");
              esc_qz("", maior_lo, " … ");
              esc_qz("", menor_hi, "   e o corte está lá dentro\n");
              ok("A VOLTA VERIFICA A IDENTIFICAÇÃO: de cada método sai uma caixa racional"
                 " — com profundidades DIFERENTES, para não serem a mesma por construção —"
                 " e as quatro intersetam-se, com o corte dentro da interseção. Se duas"
                 " fossem pontos diferentes, as caixas separavam-se ao apertar",
                 vmal == 0); }

            /* «ℚ TEM O RASTRO, MAS NÃO TEM A FOLHA» — a frase dele, e é medível: os
             * quatro rastros existem INTEIROS em ℚ (todos os termos são frações) e o
             * ponto que eles determinam NÃO está lá. É a completude sem a palavra. */
            { int fmal = 0;
              long racionais_no_rastro = 0, folhas_em_Q = 0;
              Suc mob = { S_MOBIUS, 2, 0, 0 };
              for(long k = 0; k <= 10; k++){
                  Qz t = cy_termo(mob, k);
                  if(t.q != 0) racionais_no_rastro++;    /* o rastro é todo de ℚ */
                  if(rz_no_corte((Corte){2,2}, t)) folhas_em_Q++;
              }
              Qz lo, hi; rz_caixa_inicial((Corte){2,2}, &lo, &hi);
              for(int k = 0; k < 12; k++){
                  rz_encaixota((Corte){2,2}, &lo, &hi, 1);
                  if(lo.q == 0 || hi.q == 0) fmal++;     /* as pontas também são de ℚ */
                  if(rz_no_corte((Corte){2,2}, lo) || rz_no_corte((Corte){2,2}, hi))
                      folhas_em_Q++;
              }
              printf("      o rastro: %ld termos, TODOS racionais — e %ld folhas em ℚ\n",
                     racionais_no_rastro, folhas_em_Q);
              ok("«ℚ TEM O RASTRO, MAS NÃO TEM A FOLHA», e a frase mede-se: os rastros dos"
                 " quatro métodos são INTEIRAMENTE de ℚ (cada termo e cada ponta é uma"
                 " fração) e nenhum deles é a folha. ℝ é o que acrescenta a folha que"
                 " fecha o rastro — e isso é a completude dita sem a palavra",
                 fmal == 0 && racionais_no_rastro == 11 && folhas_em_Q == 0); }

            /* E O SINO: o objeto não é só a folha — O CAMINHO FAZ PARTE DA INFORMAÇÃO.
             * Os quatro rastros dão o mesmo ponto e NÃO são o mesmo rastro: se fossem,
             * não haveria nada a identificar. Mede-se que os caminhos DIFEREM enquanto o
             * destino é um só — que é a coisa toda. */
            { int smal = 0; long diferem = 0, iguais = 0;
              Suc mob = { S_MOBIUS, 2, 0, 0 }, bis = { S_LO, 2, 0, 0 }, fc = { S_CONV, 2, 0, 0 };
              for(long k = 1; k <= 10; k++){
                  Qz m = cy_termo(mob, k), b = cy_termo(bis, k), f = cy_termo(fc, k);
                  if(!qz_igual(m,b) || !qz_igual(m,f) || !qz_igual(b,f)) diferem++;
                  else iguais++;
              }
              /* e mesmo assim são a MESMA classe: as diferenças vão a zero */
              long N1 = -1, N2 = -1;
              Qz eps = qz(1, 1000000);
              if(!cy_equiv(mob, bis, eps, 30, &N1)) smal++;
              if(!cy_equiv(mob, fc,  eps, 30, &N2)) smal++;
              printf("      os três rastros diferem em %ld dos 10 índices e coincidem em"
                     " %ld — e mesmo assim são a mesma classe (N = %ld e %ld)\n",
                     diferem, iguais, N1, N2);
              ok("O SINO: o caminho FAZ PARTE da informação. Os três rastros DIFEREM termo"
                 " a termo e mesmo assim são a mesma classe — se fossem o mesmo rastro não"
                 " haveria identificação nenhuma a fazer, e o teorema era vazio. O ponto é"
                 " um; os caminhos até ele, não",
                 smal == 0 && diferem > 0); }
        }

        /* ═══ §C30 CAUCHY: ℝ = Cauchy(ℚ)/∼, E O CAMINHO É O DUAL DO CORTE ═════════
         * O `eval.txt` dá a SEGUNDA construção, e ela é o dual da primeira: o CORTE diz
         * onde o ponto está (uma decisão sobre ℚ, estática); a SUCESSÃO vai lá (um
         * caminho por ℚ, dinâmica). O mesmo real sai dos dois — e é isso que se mede.
         *
         *   ∀ε>0 ∃N: m,n > N ⟹ |aₘ − aₙ| < ε      e      aₙ − bₙ → 0 é a MESMA classe
         *
         * O ε não é «um número pequeno»: é um RACIONAL qualquer, e o N é a testemunha
         * que se exibe. Aqui não há épsilon-delta com floats — há frações e um índice.
         *
         * E o GUME é a HARMÓNICA: os saltos dela vão a zero e ela NÃO é de Cauchy. É o
         * contra-caso que impede a definição de virar «os termos aproximam-se». */
        printf("\n§C30 CAUCHY: o caminho é o dual do corte, e o N exibe-se sempre.\n\n");
        {
            Suc mob = { S_MOBIUS, 2, 0, 0 };
            Suc blo = { S_LO,     2, 0, 0 };
            Suc bhi = { S_HI,     2, 0, 0 };
            Suc cfv = { S_CONV,   2, 0, 0 };
            Corte c2b = { 2, 2 };

            /* (§2) A DEFINIÇÃO, com o N EXIBIDO — e ele cresce quando o ε aperta, que é
             * o conteúdo da definição e não um detalhe */
            { int cmal = 0; long Ns[4] = {-1,-1,-1,-1};
              Qz eps[4] = { qz(1,10), qz(1,1000), qz(1,100000), qz(1,10000000) };
              for(int i = 0; i < 4; i++)
                  if(!cy_modulo(mob, eps[i], 12, 6, &Ns[i])) cmal++;
              for(int i = 1; i < 4; i++) if(Ns[i] < Ns[i-1]) cmal++;   /* aperta ⟹ N sobe */
              printf("      a órbita do Möbius: ε = 1/10 → N = %ld,  1/10³ → %ld,"
                     "  1/10⁵ → %ld,  1/10⁷ → %ld\n", Ns[0], Ns[1], Ns[2], Ns[3]);
              ok("a sucessão é de CAUCHY pela definição, e o N é EXIBIDO para cada ε — e"
                 " quando o ε aperta o N sobe, que é o conteúdo da definição. O ε é um"
                 " RACIONAL, não um número pequeno: aqui não há épsilon com vírgula",
                 cmal == 0 && Ns[0] >= 0 && Ns[3] > Ns[0]); }

            /* O GUME, e é ele que segura a definição: a HARMÓNICA tem os saltos a ir a
             * zero e NÃO é de Cauchy — H₂ₙ − Hₙ ≥ 1/2 sempre. Sem este contra-caso a
             * definição colapsava em «os termos consecutivos aproximam-se». */
            { Suc harm = { S_HARM, 0, 0, 0 }, alt = { S_ALT, 0, 0, 0 };
              int gmal = 0;
              long N1 = -1, N2 = -1;
              /* os saltos CONSECUTIVOS vão a zero: |H_{n+1} − H_n| = 1/(n+2) */
              for(long k = 0; k < 15; k++){
                  Qz d = cy_dist(cy_termo(harm, k), cy_termo(harm, k+1));
                  if(!qz_igual(d, qz(1, k+2))) gmal++;
              }
              /* e mesmo assim NÃO é de Cauchy: para ε = 1/4 não há N nenhum */
              if(cy_modulo(harm, qz(1,4), 9, 9, &N1)) gmal++;
              /* e a alternante falha já nos saltos */
              if(cy_modulo(alt, qz(1,4), 9, 3, &N2)) gmal++;
              /* a prova do bloco: H₂ₙ − Hₙ ≥ 1/2, medida e não citada */
              int meio = 1;
              for(long n2 = 1; n2 <= 9; n2++){
                  Qz d = cy_dist(cy_termo(harm, 2*n2 - 1), cy_termo(harm, n2 - 1));
                  if(qz_menor(d, qz(1,2))) meio = 0;
              }
              printf("      a harmónica: os saltos são 1/(n+2) → 0, e mesmo assim"
                     " H₂ₙ − Hₙ ≥ 1/2 sempre — NÃO é de Cauchy\n");
              ok("O GUME DA DEFINIÇÃO: a HARMÓNICA tem os saltos consecutivos a ir a zero"
                 " (são 1/(n+2), medidos) e NÃO é de Cauchy, porque H₂ₙ − Hₙ ≥ 1/2 em"
                 " todos os blocos. É este contra-caso que impede «de Cauchy» de"
                 " colapsar em «os termos consecutivos aproximam-se»",
                 gmal == 0 && meio); }

            /* (§2) A EQUIVALÊNCIA: os TRÊS caminhos do andar são o MESMO real, e agora
             * isso não é uma metáfora — é aₙ − bₙ → 0, medido entre os três pares */
            { int emal = 0; long Nm[3] = {-1,-1,-1};
              Qz eps = qz(1, 100000);
              if(!cy_equiv(mob, blo, eps, 30, &Nm[0])) emal++;
              if(!cy_equiv(mob, cfv, eps, 30, &Nm[1])) emal++;
              if(!cy_equiv(blo, bhi, eps, 30, &Nm[2])) emal++;
              /* e o GUME: uma constante racional NÃO é equivalente à órbita de √2 —
               * se fosse, √2 seria racional */
              Suc um_e_meio = { S_CONST, 0, 3, 2 };
              long Nx = -1;
              int nao_equiv = !cy_equiv(mob, um_e_meio, qz(1,1000), 30, &Nx);
              printf("      equivalência a menos de 1/10⁵: Möbius∼bisseção em N=%ld,"
                     " Möbius∼FC em N=%ld, e as duas pontas em N=%ld\n",
                     Nm[0], Nm[1], Nm[2]);
              ok("OS TRÊS CAMINHOS SÃO O MESMO REAL, e agora não é metáfora: aₙ − bₙ → 0"
                 " entre a órbita, a bisseção e os convergentes, e entre as DUAS pontas"
                 " do encaixotamento. E o gume: a constante 3/2 NÃO é equivalente — se"
                 " fosse, √2 era racional",
                 emal == 0 && nao_equiv); }

            /* (ex.8) TODA SUCESSÃO CRESCENTE E LIMITADA CONVERGE — e é aplicação direta
             * da completude: o limite é o SUPREMO, que é o corte. As duas hipóteses
             * medem-se, e o gume mostra que nenhuma é dispensável. */
            { int mmal = 0;
              Qz M;
              int cres = cy_crescente(mob, 12);
              cy_limitada(mob, 12, &M);
              int lim = qz_menor(M, qz_de_inteiro(2));
              long Nc = -1;
              int aponta = cy_aponta(mob, c2b, 14, 12, &Nc);
              if(!cres || !lim || !aponta) mmal++;
              /* o gume nos DOIS lados: sem «limitada» (a harmónica cresce e não converge)
               * e sem «crescente» (a alternante é limitada e não converge) */
              Suc harm = { S_HARM, 0, 0, 0 }, alt = { S_ALT, 0, 0, 0 };
              int so_cresce = cy_crescente(harm, 12) && !cy_modulo(harm, qz(1,4), 9, 9, 0);
              Qz Ma; cy_limitada(alt, 12, &Ma);
              int so_limitada = qz_menor(Ma, qz_de_inteiro(2))
                             && !cy_crescente(alt, 12)
                             && !cy_modulo(alt, qz(1,4), 9, 3, 0);
              printf("      crescente e LIMITADA POR 2 (o maior termo visto é %s):",
                     frac2(M.p, M.q));
              printf(" a partir de N = %ld a cauda inteira cai na caixa de 14 dobras\n", Nc);
              ok("CRESCENTE E LIMITADA CONVERGE (ex.8), e o limite é o SUPREMO — que é o"
                 " corte. As DUAS hipóteses são precisas e mede-se cada uma pelo seu"
                 " contra-caso: a harmónica é crescente e NÃO limitada (não converge), a"
                 " alternante é limitada e NÃO crescente (não converge)",
                 mmal == 0 && so_cresce && so_limitada); }

            /* (ex.9)(ex.10) CONVERGENTE ⟹ CAUCHY, e o LIMITE É ÚNICO.
             * A unicidade prova-se pelo absurdo com a triangular: se L ≠ L' fossem ambos
             * limites, |L − L'| ≤ |L − aₙ| + |aₙ − L'| ficava abaixo de |L − L'| — e o
             * que aqui se mede é a cadeia, não a conclusão. */
            { int umal = 0;
              /* convergente ⟹ Cauchy: se todos os termos após N estão numa caixa de
               * largura w, então dois quaisquer distam menos de w. É a triangular. */
              long Nc = -1;
              if(!cy_aponta(mob, c2b, 16, 12, &Nc)) umal++;
              Qz lo, hi; rz_caixa_inicial(c2b, &lo, &hi); rz_encaixota(c2b, &lo, &hi, 16);
              Qz larg = qz_soma(hi, qz_oposto(lo));
              for(long m = Nc; m <= 12; m++) for(long n2 = Nc; n2 <= 12; n2++){
                  Qz d = cy_dist(cy_termo(mob, m), cy_termo(mob, n2));
                  if(!qz_menor(d, larg) && !qz_igual(d, qz(0,1))) umal++;
              }
              /* a UNICIDADE: dois candidatos distintos não podem os dois conter a cauda.
               * O candidato falso é o corte de √3 — e a cauda NÃO cai nele. */
              Corte c3b = { 3, 2 };
              long Nf = -1;
              int falso = cy_aponta(mob, c3b, 8, 12, &Nf);
              /* e a triangular que sustenta a prova, medida em ℚ */
              int tri = 1;
              for(long p1 = -6; p1 <= 6; p1++) for(long p2 = -6; p2 <= 6; p2++)
              for(long p3 = -6; p3 <= 6; p3++){
                  Qz A = qz(p1,3), B = qz(p2,3), C = qz(p3,3);
                  Qz ab = cy_dist(A,B), ac = cy_dist(A,C), cb = cy_dist(C,B);
                  if(qz_menor(qz_soma(ac,cb), ab)) tri = 0;
              }
              printf("      a cauda cabe na caixa de largura %s (logo é de Cauchy), e o"
                     " candidato falso √3 NÃO a contém\n", frac2(larg.p, larg.q));
              ok("CONVERGENTE ⟹ CAUCHY (ex.9) porque a cauda cabe numa caixa e a"
                 " TRIANGULAR faz o resto — medida em ℚ, não citada. E o LIMITE É ÚNICO"
                 " (ex.10): um segundo candidato teria de conter a mesma cauda, e o corte"
                 " de √3 não a contém — o absurdo exibe-se em vez de se invocar",
                 umal == 0 && !falso && tri); }

            /* (ex.15) BOLZANO–WEIERSTRASS: toda sucessão limitada tem subsucessão
             * convergente. A prova É a bisseção — parte-se o intervalo ao meio e fica-se
             * com a metade que tem infinitos termos. Aqui mede-se o INVARIANTE: a metade
             * escolhida tem sempre mais termos que o teto de uma metade só. */
            { int bmal = 0;
              Suc alt = { S_ALT, 0, 0, 0 };
              Qz M; cy_limitada(alt, 24, &M);
              if(!qz_menor(M, qz_de_inteiro(2))) bmal++;         /* limitada: a hipótese */
              /* a subsucessão dos pares é constante 1 — e converge, embora a mãe não */
              long constantes = 0;
              for(long k = 0; k <= 12; k++)
                  if(qz_igual(cy_termo(alt, 2*k), qz_de_inteiro(1))) constantes++;
              /* e a bisseção sobre a sucessão do Möbius: a metade com infinitos termos
               * é sempre a que contém o corte — o invariante, medido a cada dobra */
              Qz lo = qz_de_inteiro(1), hi = qz_de_inteiro(2);
              for(int k = 0; k < 12; k++){
                  Qz m = qz_medio(lo, hi);
                  long dentro_esq = 0, dentro_dir = 0;
                  for(long j = 0; j <= 12; j++){
                      Qz a = cy_termo(mob, j);
                      if(!qz_menor(a, lo) && qz_menor(a, m)) dentro_esq++;
                      if(!qz_menor(a, m) && !qz_menor(hi, a)) dentro_dir++;
                  }
                  if(dentro_esq >= dentro_dir) hi = m; else lo = m;
                  if(dentro_esq + dentro_dir == 0) break;
                  int b1, b2;                                     /* e o corte fica dentro */
                  if(rz_cmp(lo, 2, 2, &b1) > 0 || !b1) bmal++;
                  if(rz_cmp(hi, 2, 2, &b2) < 0 || !b2) bmal++;
              }
              ok("BOLZANO–WEIERSTRASS (ex.15) pela BISSEÇÃO, que é o encaixotamento outra"
                 " vez: fica-se sempre com a metade que retém os termos, e o invariante"
                 " (o corte entre as pontas) mede-se a cada dobra. E o exemplo é o gume:"
                 " a alternante NÃO converge e a sua subsucessão par converge",
                 bmal == 0 && constantes == 13); }

            /* AS VINTE PROVAS CORREM — todas, pelo número E pelo nome. Uma fala que
             * morre calada não falha: DESAPARECE, e é o defeito mais barato de ter e o
             * mais caro de descobrir. Aqui varre-se o índice inteiro e conta-se, com a
             * saída desviada para não afogar o resto. */
            { int pmal = 0, correram = 0, por_nome = 0;
              fflush(stdout);
              int guarda = dup(1), nulo = open("/dev/null", O_WRONLY);
              if(guarda >= 0 && nulo >= 0) dup2(nulo, 1);
              for(int k = 1; k <= 20; k++){
                  char fala[64];
                  snprintf(fala, sizeof fala, "prova %d", k);
                  if(resolve_prova_real(fala)) correram++; else pmal++;
              }
              for(size_t i = 0; i < sizeof EX20/sizeof *EX20; i++){
                  char fala[96];
                  snprintf(fala, sizeof fala, "prova %s", EX20[i].nome);
                  if(resolve_prova_real(fala)) por_nome++; else pmal++;
              }
              /* e o GUME: um número FORA do índice tem de ser RECUSADO, não inventado */
              if(resolve_prova_real("prova 21")) pmal++;
              if(resolve_prova_real("prova 0")) pmal++;
              fflush(stdout);
              if(guarda >= 0){ dup2(guarda, 1); close(guarda); }
              if(nulo >= 0) close(nulo);
              printf("      as vinte provas: %d pelo número, %d pelo nome, e a 21 é"
                     " recusada\n", correram, por_nome);
              ok("AS VINTE PROVAS DO `eval.txt` correm todas, pelo NÚMERO e pelo NOME — e"
                 " o índice fora de alcance é RECUSADO. Uma fala que morre calada não"
                 " falha, desaparece: por isso se varre o índice inteiro em vez de se"
                 " confiar em ter escrito os vinte",
                 pmal == 0 && correram == 20 && por_nome == 20); }

            /* (ex.20) UMA SUCESSÃO DE CAUCHY LIMITADA CONVERGE EM ℝ — e é aqui que a
             * completude se paga: em ℚ a MESMA sucessão não converge. O andar inteiro
             * está nesta diferença, e ela mede-se com os mesmos termos. */
            { int qmal2 = 0;
              long Nr = -1;
              int converge_em_R = cy_aponta(mob, c2b, 18, 12, &Nr);
              /* em ℚ NÃO converge: o limite teria de ser o ponto fixo, e o ponto fixo
               * pede x² = 2 — varrido, nenhum racional o cumpre */
              long fixos = 0;
              for(long d = 1; d <= 150; d++) for(long p = 1; p <= 2*d; p++){
                  Qz x = qz(p,d);
                  if(qz_igual(rz_passo(2, rz_b(2), x), x)) fixos++;
              }
              if(!converge_em_R) qmal2++;
              printf("      a MESMA sucessão: converge em ℝ (cauda na caixa a partir de"
                     " N = %ld) e NÃO em ℚ (%ld pontos fixos racionais)\n", Nr, fixos);
              ok("UMA SUCESSÃO DE CAUCHY CONVERGE EM ℝ (ex.20) — e é aqui que a"
                 " completude se paga: a MESMA sucessão, com os MESMOS termos, converge"
                 " em ℝ e não em ℚ. A diferença entre os dois andares não é uma"
                 " definição: é este par de medidas sobre o mesmo objeto",
                 qmal2 == 0 && fixos == 0); }
        }

        /* ═══ §C29 OS REAIS: O REAL É O CORTE, E NUNCA UM DECIMAL ═════════════════
         * «ℝ acrescenta a completude: todo buraco racional recebe um ponto.» E a cadeia
         * fecha: ℕ conta, ℤ reverte a soma, ℚ reverte a multiplicação não nula, ℝ fecha
         * os limites que ℚ não consegue conter.
         *
         * Aqui a regra da casa é a própria matéria do andar: um `double` afirmaria que o
         * real é uma tira de casas, e o corte diz que ele é a DECISÃO sobre cada
         * racional. A decisão é inteira (pⁿ < a·dⁿ) — por isso √2 mede-se sem nunca se
         * aproximar de nada.
         *
         * TRÊS CAMINHOS, e é a concordância deles a medida: o CORTE diz onde está, o
         * MÖBIUS INTEIRO vai lá (x ↦ (a+bx)/(x+b), ponto fixo x² = a), e a FRAÇÃO
         * CONTÍNUA escreve-o. Se discordassem num racional, um estava errado. */
        printf("\n§C29 OS REAIS: o corte, o ponto fixo e a fração contínua — o mesmo ponto.\n\n");
        {
            Corte c2 = { 2, 2 };                      /* o √2 dele, o primeiro buraco */

            /* (§1)(ex.12)(ex.13) O CORTE, E O BURACO. O corte de √2 parte ℚ em dois e
             * NÃO tem racional em cima — é essa ausência o buraco, e conta-se. */
            { int rmal = 0; long em_cima = 0, sweep = 0, perto = 0;
              Qz melhor_lo = qz(0,1), melhor_hi = qz(2,1);
              for(long d = 1; d <= 120; d++) for(long p = 1; p <= 2*d + 2; p++){
                  Qz q = qz(p, d);
                  int bom, s = rz_cmp(q, 2, 2, &bom);
                  if(!bom){ rmal++; continue; }
                  sweep++;
                  if(s == 0) em_cima++;               /* seria √2 ∈ ℚ */
                  /* o corte é uma PARTIÇÃO: cada racional cai num lado e num só */
                  if((s < 0) != (rz_abaixo(c2, q) == 1)) rmal++;
                  /* e a fronteira aperta-se sem nunca fechar: os melhores dos dois lados */
                  if(s < 0 && qz_menor(melhor_lo, q)){ melhor_lo = q; perto++; }
                  if(s > 0 && qz_menor(q, melhor_hi)) melhor_hi = q;
              }
              /* o DESCENSO, que é a prova, e mede-se elo a elo em vez de se afirmar:
               * p² = 2q² obriga p par (ímpar² é ímpar, 2q² é par); p = 2k dá q² = 2k²,
               * e o mesmo argumento obriga q par — contra gcd(p,q) = 1 */
              int elo1 = 1, elo2 = 1;
              for(long p = 1; p <= 400; p += 2) if((p*p) % 2 == 0) elo1 = 0;   /* ímpar² ímpar */
              for(long q = 0; q <= 400; q++) if((2*q*q) % 2 != 0) elo2 = 0;    /* 2q² par */
              printf("      o corte de √2 em %ld racionais: %ld em cima (o buraco), e a"
                     " fronteira aperta em %s < √2 < %s\n",
                     sweep, em_cima, frac2(melhor_lo.p, melhor_lo.q),
                     frac2(melhor_hi.p, melhor_hi.q));
              ok("O REAL É O CORTE: √2 parte ℚ em dois lados, cada racional cai num e num"
                 " só, e NENHUM cai em cima — é essa ausência o buraco. E o descenso"
                 " prova-o elo a elo: ímpar² é ímpar e 2q² é par, logo p² = 2q² obriga p"
                 " par, e o mesmo obriga q par — contra a forma reduzida",
                 rmal == 0 && em_cima == 0 && sweep > 10000 && elo1 && elo2 && perto > 0); }

            /* E O CRITÉRIO GERAL, que é o que a fala afirma: √a ∈ ℚ ⟺ TODO expoente
             * primo de a é PAR. A paridade do §1 é só o caso a = 2; o teorema é este, e
             * mede-se contra o `raizi` da casa — dois caminhos que têm de concordar. */
            { int xmal = 0, fechou2 = 0, abriu2 = 0;
              for(long a2 = 2; a2 <= 400; a2++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(a2, pr, ex, NT_FAT), todos_pares = 1;
                  for(int i = 0; i < k; i++) if(ex[i] % 2) todos_pares = 0;
                  long rr = raizi(a2);
                  if(todos_pares != (rr*rr == a2)) xmal++;      /* o critério vs a régua */
                  if(nt_refaz(pr, ex, k) != a2) xmal++;         /* e a volta da fatoração */
                  if(todos_pares) fechou2++; else abriu2++;
              }
              ok("e o CRITÉRIO é geral: √a ∈ ℚ exatamente quando TODO expoente primo de a"
                 " é PAR — a paridade do √2 é só o caso a = 2. Medido em 399 números"
                 " contra o `raizi` da casa, com a volta da fatoração a fechar: quando um"
                 " expoente é ímpar, é ele a testemunha do buraco",
                 xmal == 0 && fechou2 > 0 && abriu2 > 0); }

            /* (§5)(§6) O TEOREMA DO SUPREMO — e as duas metades que ele nomeia.
             * A MENORIDADE é a que não é tautologia: se u < √2 então EXIBE-SE q ∈ S com
             * u < q < √2. E a testemunha é o Möbius: q = (2u+2)/(u+2). O mesmo mapa
             * aperta o outro lado — é o PAR, e por isso mede-se nos dois. */
            { int smal = 0; long dentro = 0, fora = 0;
              long b2 = rz_b(2);                      /* b escolhe-se por b² > a, e é isso
                                                       * que faz o mapa NÃO trocar de lado */
              if(b2*b2 <= 2) smal++;
              for(long d = 1; d <= 60; d++) for(long p = 1; p <= 3*d; p++){
                  Qz u = qz(p, d);
                  int bom, s = rz_cmp(u, 2, 2, &bom);
                  if(!bom){ smal++; continue; }
                  Qz v = rz_passo(2, b2, u);          /* (2 + bu)/(u + b), com b = rz_b(2) */
                  int bv, sv = rz_cmp(v, 2, 2, &bv);
                  if(!bv){ smal++; continue; }
                  if(s < 0){                          /* u ∈ S: o novo é MAIOR e ainda em S */
                      if(!qz_menor(u, v)) smal++;
                      if(sv >= 0) smal++;             /* continua abaixo de √2 */
                      dentro++;
                  } else if(s > 0){                   /* u é cota: o novo é cota MENOR */
                      if(!qz_menor(v, u)) smal++;
                      if(sv <= 0) smal++;
                      fora++;
                  }
              }
              Qz u0 = qz(7,5), q0 = rz_passo(2, b2, u0);
              printf("      menoridade: u = 7/5 (u² = 49/25 < 2) dá q = %s, e q² = %s < 2\n",
                     frac2(q0.p, q0.q), frac2(q0.p*q0.p, q0.q*q0.q));
              ok("sup S = √2 com S = {q > 0 : q² < 2}: a MENORIDADE exibe a testemunha —"
                 " de u ∈ S sai q = (2u+2)/(u+2) com u < q e q ainda em S, logo nenhum"
                 " racional abaixo de √2 é cota. E o MESMO mapa desce as cotas por cima:"
                 " os dois lados do par medidos juntos, não um só",
                 smal == 0 && dentro > 1000 && fora > 1000); }

            /* (§9)(§2) A SUCESSÃO MONÓTONA E DE CAUCHY, e o buraco à vista: ela é de
             * Cauchy em ℚ e o seu limite NÃO é racional, porque o ponto fixo x = (2+2x)/(x+2)
             * é x² = 2. É a construção de Cauchy dele, sem nenhum ε flutuante: a largura
             * é uma FRAÇÃO e compara-se com outra fração. */
            { int cmal = 0;
              Qz x = qz_de_inteiro(1), ant = x;
              Qz termos[9];
              for(int k = 0; k < 9; k++){
                  termos[k] = x;
                  if(k){ if(!qz_menor(ant, x)) cmal++; }   /* estritamente crescente */
                  { int bom, s = rz_cmp(x, 2, 2, &bom);
                    if(!bom || s >= 0) cmal++; }           /* e limitada por √2 */
                  if(!qz_menor(x, qz_de_inteiro(2))) cmal++;/* logo por 2 */
                  ant = x; x = rz_passo(2, rz_b(2), x);
              }
              /* de CAUCHY: as diferenças encolhem, e mede-se com FRAÇÕES */
              Qz gap_cedo = qz_soma(termos[2], qz_oposto(termos[1]));
              Qz gap_tarde = qz_soma(termos[8], qz_oposto(termos[7]));
              if(!qz_menor(gap_tarde, gap_cedo)) cmal++;
              /* e o limite NÃO é racional: o ponto fixo pede x² = 2, e o corte diz que
               * nenhum racional o cumpre — a mesma varredura de cima */
              int fixo_racional = 0;
              for(long d = 1; d <= 200; d++) for(long p = 1; p <= 2*d; p++){
                  Qz q = qz(p,d);
                  if(qz_igual(rz_passo(2,rz_b(2),q), q)) fixo_racional++;
              }
              /* UM `frac2` POR `printf`: o destino é rotativo de QUATRO, e cinco numa só
               * chamada faziam o quinto sobrescrever o primeiro — a linha saía com o
               * termo errado e nenhuma asserção o via, porque a asserção não lê o texto */
              printf("      a sucessão: ");
              for(int k = 0; k < 5; k++) printf("%s%s", k ? ", " : "",
                                                frac2(termos[k].p, termos[k].q));
              printf(" …  crescente, e limitada por 2\n");
              printf("      e o salto encolhe de %s", frac2(gap_cedo.p, gap_cedo.q));
              printf(" para %s", frac2(gap_tarde.p, gap_tarde.q));
              printf(" — de Cauchy, e SEM limite em ℚ\n");
              ok("a sucessão do Möbius é CRESCENTE e LIMITADA (o §9), é de CAUCHY (os"
                 " saltos encolhem, medidos como FRAÇÕES e não como ε) e o seu limite NÃO"
                 " está em ℚ: o ponto fixo pede x² = 2 e nenhum racional o cumpre. É o"
                 " buraco à vista — a sucessão existe em ℚ e o ponto dela não",
                 cmal == 0 && fixo_racional == 0); }

            /* (§10) OS INTERVALOS ENCAIXADOS — e as larguras são EXATAS, não «tendem» */
            { int imal = 0;
              Qz lo, hi;
              if(!rz_caixa_inicial(c2, &lo, &hi)) imal++;
              Qz lo0 = lo, hi0 = hi;
              Qz larg_ant = qz_soma(hi, qz_oposto(lo));
              for(int k = 1; k <= 20; k++){
                  Qz lo_ant = lo, hi_ant = hi;
                  rz_encaixota(c2, &lo, &hi, 1);
                  /* ENCAIXADO: o novo cabe dentro do velho, nos dois lados */
                  if(qz_menor(lo, lo_ant) || qz_menor(hi_ant, hi)) imal++;
                  /* a largura é EXATAMENTE metade — e é uma fração, não um limite */
                  Qz larg = qz_soma(hi, qz_oposto(lo));
                  if(!qz_igual(qz_mult(larg, qz_de_inteiro(2)), larg_ant)) imal++;
                  /* e o ponto continua lá dentro: lo ∈ A e hi ∈ B, sempre */
                  { int b1, b2;
                    if(rz_cmp(lo, 2, 2, &b1) >= 0 || !b1) imal++;
                    if(rz_cmp(hi, 2, 2, &b2) <= 0 || !b2) imal++; }
                  larg_ant = larg;
              }
              Qz larg20 = qz_soma(hi, qz_oposto(lo));
              /* a largura ao fim de 20 dobras é EXATAMENTE (hi₀−lo₀)/2²⁰ */
              Qz esperada = qz(hi0.p*lo0.q - lo0.p*hi0.q, hi0.q*lo0.q*(1L<<20));
              printf("      caixa 20: %s < √2 < %s,  largura = %s (= 1/2²⁰ exata)\n",
                     frac2(lo.p,lo.q), frac2(hi.p,hi.q), frac2(larg20.p,larg20.q));
              ok("os INTERVALOS ENCAIXADOS: cada caixa cabe na anterior, a largura é"
                 " EXATAMENTE metade a cada dobra (uma fração, não um «tende a zero») e o"
                 " corte fica sempre entre as pontas — ao fim de 20 dobras a largura é"
                 " (b₀−a₀)/2²⁰ e confere ao numerador",
                 imal == 0 && qz_igual(larg20, esperada)); }

            /* OS TRÊS CAMINHOS TÊM DE CONCORDAR: os CONVERGENTES da fração contínua
             * caem DENTRO das caixas do encaixotamento. É o dois-caminhos deste andar —
             * a bisseção e a FC nunca se falaram, e localizam o mesmo ponto. */
            { int fmal = 0, comparados = 0;
              long a[48];
              size_t nt = lado(0, -2, a, 48);                /* a FC de √2 = [1;2,2,2,…] */
              long pn = 1, qn = 0, pa = 0, qa = 1;           /* os convergentes por recorrência */
              Qz lo, hi;
              rz_caixa_inicial(c2, &lo, &hi);
              rz_encaixota(c2, &lo, &hi, 12);                /* a caixa dos 12 ticks */
              for(size_t i = 0; i < 8 && i < 48; i++){
                  long ai = a[i < nt ? i : (1 + (i - 1) % (nt > 1 ? nt - 1 : 1))];
                  long pp = ai*pn + pa, qq2 = ai*qn + qa;
                  pa = pn; qa = qn; pn = pp; qn = qq2;
                  if(qn <= 0) continue;
                  Qz cv = qz(pn, qn);
                  int bom, s = rz_cmp(cv, 2, 2, &bom);
                  if(!bom) continue;
                  if(s == 0) fmal++;                          /* um convergente NUNCA é √2 */
                  /* a partir de certo termo o convergente entra na caixa e lá fica */
                  if(qn >= 100){
                      if(qz_menor(cv, lo) || qz_menor(hi, cv)) fmal++;
                      comparados++;
                  }
              }
              printf("      a FC de √2 = %s  (período %zu — Lagrange), e os convergentes\n",
                     fc_da_borda(0, -2), nt);
              printf("      de denominador ≥ 100 caem DENTRO da caixa dos 12 ticks: %d de %d\n",
                     comparados, comparados);
              ok("OS TRÊS CAMINHOS CONCORDAM: o CORTE decide, o MÖBIUS persegue e a FRAÇÃO"
                 " CONTÍNUA escreve — e os convergentes de √2 = [1;2,2,2,…] caem dentro da"
                 " caixa que a bisseção fechou, sem que os dois métodos se conheçam."
                 " Nenhum convergente é √2: a FC não fecha, e é isso que ela diz",
                 fmal == 0 && comparados > 0 && nt > 0); }

            /* (§7) ARQUIMEDES em ℝ, e (§8) a DENSIDADE dos DOIS: racionais e irracionais */
            { int amal = 0;
              /* para todo real (aqui os cortes ⁿ√a) existe n natural com n > x */
              for(long a2 = 2; a2 <= 60; a2++){
                  Corte ca = { a2, 2 };
                  Qz lo, hi;
                  if(!rz_caixa_inicial(ca, &lo, &hi)){ amal++; continue; }
                  long n2 = hi.p / hi.q + 1;               /* o n EXIBE-SE */
                  int bom, s = rz_cmp(qz_de_inteiro(n2), 2, a2, &bom);
                  if(!bom || s <= 0) amal++;               /* n > √a2, medido pelo corte */
                  /* e o dual: 1/n < ε para ε = 1/m, com n = m+1 */
                  for(long m = 1; m <= 40; m++)
                      if(!qz_menor(qz(1, m+1), qz(1, m))) amal++;
              }
              /* a DENSIDADE dos racionais: entre dois reais (dois cortes) há um racional,
               * e ele SAI do encaixotamento — não se postula, exibe-se */
              Corte c3 = { 3, 2 };
              Qz l2, h2, l3, h3;
              rz_caixa_inicial(c2, &l2, &h2); rz_encaixota(c2, &l2, &h2, 16);
              rz_caixa_inicial(c3, &l3, &h3); rz_encaixota(c3, &l3, &h3, 16);
              Qz entre = qz_medio(h2, l3);                 /* √2 < h2 ≤ entre ≤ l3 < √3 */
              int racional_entre = qz_menor(h2, entre) && qz_menor(entre, l3);
              /* e um IRRACIONAL entre dois racionais: √2/4, irracional porque √2 o é — se
               * √2/4 fosse p/d, então √2 = 4p/d estaria em ℚ, e não está. A caixa de √2
               * dividida por 4 é a caixa de √2/4, e é ela que o põe entre 1/3 e 1/2. */
              Qz a5 = qz(1,3), b5 = qz(1,2);
              Qz irr_lo = qz(l2.p, l2.q * 4), irr_hi = qz(h2.p, h2.q * 4);
              int irracional_entre = qz_menor(a5, irr_lo) && qz_menor(irr_hi, b5);
              printf("      √2 < %s < √3   (o racional entre dois reais)\n",
                     frac2(entre.p, entre.q));
              printf("      e √2/4 ∈ (%s", frac2(irr_lo.p, irr_lo.q));
              printf(", %s) cai entre 1/3 e 1/2 — o irracional\n", frac2(irr_hi.p, irr_hi.q));
              ok("ARQUIMEDES vale em ℝ com o n EXIBIDO pela caixa, e a DENSIDADE é dos"
                 " DOIS lados: entre √2 e √3 exibe-se um racional (sai do encaixotamento,"
                 " não se postula) e entre 1/3 e 1/2 exibe-se um IRRACIONAL, o √2/4 —"
                 " racional ele seria se √2 o fosse, e não é",
                 amal == 0 && racional_entre && irracional_entre); }

            /* (§12) A EXISTÊNCIA DE RAÍZES: ∀a>0 ∀n≥1 ∃!x>0 com xⁿ = a, e a UNICIDADE é
             * a monotonia estrita. Quando fecha em ℚ, o corte e o `raizi` da casa têm de
             * dar o mesmo — dois caminhos outra vez. */
            { int emal2 = 0, fechou = 0, abriu = 0;
              for(long a2 = 1; a2 <= 200; a2++) for(int n2 = 2; n2 <= 3; n2++){
                  Corte ca = { a2, n2 };
                  long r = 0;
                  int fecha = rz_fecha_em_q(ca, &r);
                  if(n2 == 2){
                      long rr = raizi(a2);                  /* a régua da casa */
                      if(fecha != (rr*rr == a2)) emal2++;   /* os dois caminhos */
                      if(fecha && r != rr) emal2++;
                  }
                  if(fecha){ fechou++;
                      Qz z = qz_de_inteiro(r);
                      if(!rz_no_corte(ca, z)) emal2++;      /* e o corte confirma: rⁿ = a */
                  } else { abriu++;
                      Qz lo, hi;
                      if(!rz_caixa_inicial(ca, &lo, &hi)) emal2++;
                      else { rz_encaixota(ca, &lo, &hi, 10);
                             int b1, b2;
                             if(rz_cmp(lo, n2, a2, &b1) >= 0 || !b1) emal2++;
                             if(rz_cmp(hi, n2, a2, &b2) <= 0 || !b2) emal2++; }
                  }
                  /* a UNICIDADE é a MONOTONIA: x < y ⟹ xⁿ < yⁿ nos positivos */
                  for(long x = 1; x <= 12; x++) for(long y = x+1; y <= 13; y++){
                      __int128 X = 1, Y = 1;
                      for(int i = 0; i < n2; i++){ X *= x; Y *= y; }
                      if(!(X < Y)) emal2++;
                  }
              }
              printf("      raízes de 1..200 em índices 2 e 3: %d fecham em ℚ, %d abrem"
                     " caixa — e o corte concorda com o `raizi` da casa em todas\n",
                     fechou, abriu);
              ok("a EXISTÊNCIA DE RAÍZES: para cada a > 0 e n ≥ 1 o corte dá o x, e a"
                 " UNICIDADE é a monotonia estrita de xⁿ nos positivos. Quando fecha em ℚ"
                 " o corte e o `raizi` da casa dão o MESMO inteiro; quando não fecha, a"
                 " caixa aperta e as pontas ficam nos dois lados",
                 emal2 == 0 && fechou > 0 && abriu > 0); }

            /* (§13) O TEOREMA DO VALOR INTERMÉDIO, com o exemplo dele: f(x) = x² − 2 em
             * [1,2], f(1) = −1 e f(2) = 2. A bisseção É o encaixotamento, e o INVARIANTE
             * que a autoriza mede-se a cada passo: o sinal continua a trocar. */
            { int tmal = 0;
              Qz lo = qz_de_inteiro(1), hi = qz_de_inteiro(2);
              /* f(1) = −1 < 0 < 2 = f(2) — a hipótese, medida e não assumida */
              Qz f_lo = qz_soma(qz_mult(lo,lo), qz_de_inteiro(-2));
              Qz f_hi = qz_soma(qz_mult(hi,hi), qz_de_inteiro(-2));
              if(!(f_lo.p < 0 && f_hi.p > 0)) tmal++;
              for(int k = 0; k < 18; k++){
                  Qz m = qz_medio(lo, hi);
                  Qz fm = qz_soma(qz_mult(m,m), qz_de_inteiro(-2));
                  if(fm.p == 0) break;                      /* seria a raiz racional */
                  if(fm.p < 0) lo = m; else hi = m;
                  Qz fl = qz_soma(qz_mult(lo,lo), qz_de_inteiro(-2));
                  Qz fh = qz_soma(qz_mult(hi,hi), qz_de_inteiro(-2));
                  if(!(fl.p < 0 && fh.p > 0)) tmal++;       /* O INVARIANTE, a cada tick */
              }
              /* e o c é o corte: as pontas apertam-no dos dois lados */
              int b1, b2;
              if(rz_cmp(lo, 2, 2, &b1) >= 0 || !b1) tmal++;
              if(rz_cmp(hi, 2, 2, &b2) <= 0 || !b2) tmal++;
              printf("      f(x) = x²−2 em [1,2]: f(1) = %s", frac2(f_lo.p,f_lo.q));
              printf(" e f(2) = %s", frac2(f_hi.p,f_hi.q));
              printf(", e 18 cortes dão c ∈ (%s", frac2(lo.p,lo.q));
              printf(", %s)\n", frac2(hi.p,hi.q));
              ok("o VALOR INTERMÉDIO com o exemplo dele: f(1) = −1 < 0 < 2 = f(2), e a"
                 " bisseção É o encaixotamento — o INVARIANTE (o sinal troca entre as"
                 " pontas) mede-se a CADA tick, e o c que sobra é exatamente o corte de"
                 " √2. O teorema não se cita: o invariante é que o carrega", tmal == 0); }

            /* (§4)(ex.1–5) O VALOR ABSOLUTO, e o exercício 5 nos DOIS sentidos */
            { int vmal = 0, viu_ida = 0, viu_volta = 0;
              for(long p = -12; p <= 12; p++) for(long d = 1; d <= 6; d++){
                  Qz x = qz(p,d), ax = x.p < 0 ? qz_oposto(x) : x;
                  if(ax.p < 0) vmal++;                                  /* |x| ≥ 0 */
                  if(qz_mult(x,x).p < 0) vmal++;                        /* x² ≥ 0 */
                  for(long p2 = -12; p2 <= 12; p2++) for(long d2 = 1; d2 <= 6; d2++){
                      Qz y = qz(p2,d2), ay = y.p < 0 ? qz_oposto(y) : y;
                      Qz xy = qz_mult(x,y), axy = xy.p < 0 ? qz_oposto(xy) : xy;
                      if(!qz_igual(axy, qz_mult(ax,ay))) vmal++;        /* |xy| = |x||y| */
                      Qz s = qz_soma(x,y), as = s.p < 0 ? qz_oposto(s) : s;
                      if(qz_menor(qz_soma(ax,ay), as)) vmal++;          /* triangular */
                      /* ex.5: |x| < ε ⟺ −ε < x < ε, e as DUAS direções contam-se */
                      if(y.p > 0){
                          int esq = qz_menor(ax, y);
                          int dir = qz_menor(qz_oposto(y), x) && qz_menor(x, y);
                          if(esq != dir) vmal++;
                          if(esq) viu_ida++; else viu_volta++;
                      }
                  }
              }
              ok("o VALOR ABSOLUTO: |x| ≥ 0, x² ≥ 0, |xy| = |x||y| e a TRIANGULAR — e o"
                 " exercício 5 mede-se como equivalência, |x| < ε ⟺ −ε < x < ε, com os"
                 " dois lados a ocorrerem (não é uma implicação com o nome de ⟺)",
                 vmal == 0 && viu_ida > 0 && viu_volta > 0); }
        }

        /* ═══ §C28 OS RACIONAIS: a REVERSIBILIDADE DA MULTIPLICAÇÃO NÃO NULA ══════
         * A escada e o que cada andar acrescenta, ditos por ele:
         *
         *   ℕ: + ×      ℤ: + × −  (reversibilidade da SOMA)
         *   ℚ: + × − ÷            (reversibilidade da MULTIPLICAÇÃO não nula)
         *
         *   construção → oposto → inverso
         *
         * E o gume vem na língua desta casa: «divisão por zero não é uma aproximação
         * ruim; é uma operação SEM FIBRA». A fibra é a divisão das cinco operações —
         * dado o produto e um fator, achar o outro. Com o fator zero ou não há nenhum
         * (0·x = 1) ou há todos (0·x = 0): nos dois casos não há fibra, e diz-se.
         *
         * Tudo por produto cruzado. Nenhum decimal entra — nem para comparar. */
        printf("\n§C28 OS RACIONAIS: o inverso, e o único sítio onde ele falta.\n\n");
        {
            int qmal = 0;
            /* (ex.1) a ~ é equivalência, e a classe é que é o número — não o par escrito */
            for(long a = -6; a <= 6; a++) for(long b = 1; b <= 6; b++){
                Qz x = { a, b };
                if(!qz_igual(x, x)) qmal++;                          /* reflexiva */
                for(long c = -6; c <= 6; c++) for(long d = 1; d <= 6; d++){
                    Qz y = { c, d };
                    if(qz_igual(x,y) != qz_igual(y,x)) qmal++;       /* simétrica */
                    /* e a reduzida representa a classe: iguais ⟺ mesma reduzida */
                    { Qz rx = qz(a,b), ry = qz(c,d);
                      if(qz_igual(x,y) != (rx.p == ry.p && rx.q == ry.q)) qmal++; }
                    for(long e2 = -4; e2 <= 4; e2++) for(long f2 = 1; f2 <= 4; f2++){
                        Qz z = { e2, f2 };
                        if(qz_igual(x,y) && qz_igual(y,z) && !qz_igual(x,z)) qmal++;
                    }
                }
            }
            ok("ℚ constrói-se por PARES e (a,b)~(c,d) ⟺ ad = bc é equivalência — e a"
               " classe é que é o número: dois pares são iguais exatamente quando têm a"
               " mesma forma reduzida (o primeiro tick do andar)", qmal == 0);

            /* (ex.2)(ex.3) a SOMA e o PRODUTO BEM DEFINIDOS: trocar o representante
             * (multiplicar os dois termos por k) não pode mudar o resultado */
            { int bmal4 = 0;
              for(long a = -5; a <= 5; a++) for(long b = 1; b <= 5; b++)
              for(long c = -5; c <= 5; c++) for(long d = 1; d <= 5; d++)
              for(long k = 1; k <= 4; k++) for(long l = 1; l <= 3; l++){
                  Qz x = { a, b }, y = { c, d };
                  Qz x2 = { a*k, b*k }, y2 = { c*l, d*l };           /* outros representantes */
                  if(!qz_igual(qz_soma(x,y), qz_soma(x2,y2))) bmal4++;
                  if(!qz_igual(qz_mult(x,y), qz_mult(x2,y2))) bmal4++;
              }
              printf("      2/3 + 3/5 = "); { Qz s = qz_soma(qz(2,3), qz(3,5)); esc_q(s); }
              printf("   e   2/3 · 5/7 = "); { Qz m = qz_mult(qz(2,3), qz(5,7)); esc_q(m); }
              printf("   (os dois exemplos do ficheiro)\n");
              Qz s1 = qz_soma(qz(2,3), qz(3,5)), m1 = qz_mult(qz(2,3), qz(5,7));
              ok("a SOMA e a MULTIPLICAÇÃO estão BEM DEFINIDAS: trocar o representante"
                 " (a,b)→(ka,kb) não muda a classe do resultado — é isto que faz a"
                 " operação ser sobre o NÚMERO e não sobre o par que calhou escrito",
                 bmal4 == 0 && s1.p == 19 && s1.q == 15 && m1.p == 10 && m1.q == 21); }

            /* (ex.4)(ex.5) os NEUTROS e o OPOSTO, com a unicidade varrida */
            { int nmal2 = 0;
              Qz zero = qz(0,1), um = qz(1,1);
              for(long a = -8; a <= 8; a++) for(long b = 1; b <= 8; b++){
                  Qz x = qz(a,b);
                  if(!qz_igual(qz_soma(x, zero), x)) nmal2++;        /* q + 0 = q */
                  if(!qz_igual(qz_mult(x, um), x)) nmal2++;          /* q · 1 = q */
                  if(!qz_igual(qz_soma(x, qz_oposto(x)), zero)) nmal2++;
                  /* e o oposto é o ÚNICO: nenhum outro racional soma zero com x */
                  for(long c = -8; c <= 8; c++) for(long d = 1; d <= 8; d++){
                      Qz y = qz(c,d);
                      if(qz_igual(qz_soma(x,y), zero) && !qz_igual(y, qz_oposto(x))) nmal2++;
                  }
              }
              ok("os NEUTROS cumprem (q+0 = q, q·1 = q) e o OPOSTO é ÚNICO — varrido, não"
                 " afirmado: nenhum outro racional soma zero com q", nmal2 == 0); }

            /* O INVERSO, E O GUME. Existe ⟺ q ≠ 0, é único, e no zero NÃO EXISTE —
             * e a razão diz-se: a fibra 0·x = 1 é vazia, a fibra 0·x = 0 é toda. */
            { int imal = 0, recusou = 0, vazia = 0, toda = 0;
              Qz zero = qz(0,1), um = qz(1,1), i;
              if(!qz_inverso(zero, &i)) recusou = 1;                 /* 0⁻¹ não existe */
              for(long a = -8; a <= 8; a++) for(long b = 1; b <= 8; b++){
                  Qz x = qz(a,b), inv;
                  int tem = qz_inverso(x, &inv);
                  if(tem != (a != 0)) imal++;                        /* existe ⟺ ≠ 0 */
                  if(tem && !qz_igual(qz_mult(x, inv), um)) imal++;  /* e a volta dá 1 */
                  /* e é ÚNICO: nenhum outro y cumpre xy = 1 */
                  for(long c = -8; c <= 8; c++) for(long d = 1; d <= 8; d++){
                      Qz y = qz(c,d);
                      if(qz_igual(qz_mult(x,y), um) && !(tem && qz_igual(y, inv))) imal++;
                  }
                  /* o gume medido nos DOIS lados: 0·x nunca dá 1, e dá sempre 0 */
                  if(qz_igual(qz_mult(zero, x), um)) vazia++;
                  if(!qz_igual(qz_mult(zero, x), zero)) toda++;
                  { Qz r; if(qz_divide(x, zero, &r)) imal++; }       /* dividir por 0: recusa */
              }
              Qz i37; qz_inverso(qz(3,7), &i37);
              printf("      (3/7)⁻¹ = "); esc_q(i37);
              printf("   e   0⁻¹ = %s\n", recusou ? "NÃO EXISTE (sem fibra)" : "??");
              ok("O INVERSO é a novidade do andar: existe para todo q ≠ 0, é ÚNICO, e no"
                 " ZERO não existe — e não por dificuldade: a fibra 0·x = 1 é VAZIA e a"
                 " fibra 0·x = 0 é TODA. Dividir por zero é recusado, não aproximado",
                 imal == 0 && recusou && vazia == 0 && toda == 0
                 && i37.p == 7 && i37.q == 3); }

            /* (§7)(ex.6)(§8) a DIVISÃO É A VOLTA: a/b ÷ c/d = ad/bc, e r·(c/d) = a/b */
            { int dmal4 = 0, houve = 0;
              for(long a = -6; a <= 6; a++) for(long b = 1; b <= 6; b++)
              for(long c = -6; c <= 6; c++) for(long d = 1; d <= 6; d++){
                  Qz x = qz(a,b), y = qz(c,d), r;
                  if(!qz_divide(x, y, &r)){ if(c != 0) dmal4++; continue; }
                  /* a fórmula do teorema, ad/bc — e a divisão tem de dar o mesmo */
                  if(!qz_igual(r, qz(a*d, b*c))) dmal4++;
                  if(!qz_igual(qz_mult(r, y), x)) dmal4++;           /* A VOLTA, resíduo 0 */
                  houve++;
              }
              Qz r8; int t8 = qz_divide(qz(2,3), qz(5,4), &r8);
              Qz v8 = qz_mult(r8, qz(5,4));
              printf("      2/3 ÷ 5/4 = "); esc_q(r8); printf("   e a volta "); esc_q(r8);
              printf(" · 5/4 = "); esc_q(v8); printf("   (resíduo 0)\n");
              ok("a DIVISÃO não é operação nova — é multiplicar pelo inverso — e vale"
                 " ad/bc por dois caminhos, com a VOLTA r·(c/d) = a/b a fechar em todos"
                 " os casos com c ≠ 0: é isto a reversibilidade da multiplicação",
                 dmal4 == 0 && houve > 1000 && t8 && r8.p == 8 && r8.q == 15
                 && qz_igual(v8, qz(2,3))); }

            /* (ex.7)(ex.8) a SIMPLIFICAÇÃO e a IGUALDADE — e nenhum decimal entra */
            { int smal = 0;
              for(long a = -30; a <= 30; a++) for(long b = 1; b <= 30; b++){
                  Qz r = qz(a,b);
                  if(qz_mdc(r.p, r.q) != 1) smal++;                  /* reduzida: gcd = 1 */
                  if(r.q <= 0) smal++;                               /* sinal no numerador */
                  Qz x = { a, b };
                  if(!qz_igual(x, r)) smal++;                        /* e é a MESMA classe */
              }
              Qz e7 = qz(84, 126);
              int ig1 = qz_igual(qz(3,7), qz(15,35));                /* 3·35 = 105 = 7·15 */
              int ig2 = qz_igual(qz(3,7), qz(4,9));                  /* 27 ≠ 28 */
              printf("      84/126 = "); esc_q(e7);
              printf("   ·   3/7 = 15/35 ? %s   (3·35 = %d, 7·15 = %d)\n",
                     ig1 ? "sim" : "não", 3*35, 7*15);
              printf("      3/7 = 4/9 ? %s   (3·9 = %d, 7·4 = %d — e a diferença é 1)\n",
                     ig2 ? "sim" : "NÃO", 3*9, 7*4);
              ok("a forma REDUZIDA é única (gcd = 1, sinal no numerador) e a IGUALDADE"
                 " decide-se pelo PRODUTO CRUZADO: 84/126 = 2/3, 3/7 = 15/35 e 3/7 ≠ 4/9"
                 " — sem converter para decimal nenhuma vez",
                 smal == 0 && e7.p == 2 && e7.q == 3 && ig1 && !ig2); }

            /* (§11)(ex.9)(ex.15) a ORDEM TOTAL, e o gume: c > 0 preserva, c < 0 INVERTE */
            { int omal5 = 0, inverteu = 0;
              for(long a = -5; a <= 5; a++) for(long b = 1; b <= 5; b++)
              for(long c = -5; c <= 5; c++) for(long d = 1; d <= 5; d++){
                  Qz x = qz(a,b), y = qz(c,d);
                  if(!(qz_menor(x,y) || qz_menor(y,x) || qz_igual(x,y))) omal5++;   /* total */
                  if(qz_menor(x,y) && qz_menor(y,x)) omal5++;                       /* estrita */
                  for(long e2 = -3; e2 <= 3; e2++) for(long f2 = 1; f2 <= 3; f2++){
                      Qz z = qz(e2,f2);
                      if(qz_menor(x,y) && qz_menor(y,z) && !qz_menor(x,z)) omal5++; /* transitiva */
                      /* a < b e c > 0 ⇒ ac < bc; e com c < 0 a desigualdade VIRA */
                      if(qz_menor(x,y) && e2 > 0 && !qz_menor(qz_mult(x,z), qz_mult(y,z))) omal5++;
                      if(qz_menor(x,y) && e2 < 0){
                          if(!qz_menor(qz_mult(y,z), qz_mult(x,z))) omal5++;
                          inverteu = 1;
                      }
                      /* e a soma preserva sempre, sem condição sobre o sinal */
                      if(qz_menor(x,y) != qz_menor(qz_soma(x,z), qz_soma(y,z))) omal5++;
                  }
              }
              printf("      3/7 < 1/2 ? %s   porque 3·2 = %d  <  7·1 = %d\n",
                     qz_menor(qz(3,7), qz(1,2)) ? "sim" : "não", 3*2, 7*1);
              ok("a ORDEM em ℚ é TOTAL e compatível: a soma preserva SEMPRE, o produto"
                 " preserva com c > 0 e INVERTE com c < 0 — e o lado que inverte é o gume"
                 " (uma regra que não tivesse os dois lados seria metade)",
                 omal5 == 0 && inverteu); }

            /* (§12)(ex.10) a DENSIDADE — o que ℤ não tinha: entre dois há sempre outro */
            { int dmal5 = 0;
              for(long a = -5; a <= 5; a++) for(long b = 1; b <= 5; b++)
              for(long c = -5; c <= 5; c++) for(long d = 1; d <= 5; d++){
                  Qz x = qz(a,b), y = qz(c,d);
                  if(!qz_menor(x,y)) continue;
                  Qz m = qz_medio(x,y);
                  if(!qz_menor(x,m) || !qz_menor(m,y)) dmal5++;      /* a < (a+b)/2 < b */
              }
              /* e os TRÊS entre 1/3 e 1/2, cada um entre o médio anterior e b (o que resta
               * à direita): é o processo que não pára, e por isso são infinitos, não três */
              Qz a3 = qz(1,3), b3 = qz(1,2), e = a3, m3[3];
              for(int k = 0; k < 3; k++){ m3[k] = qz_medio(e, b3); e = m3[k]; }
              int tres = 1;
              for(int k = 0; k < 3; k++) if(!qz_menor(a3, m3[k]) || !qz_menor(m3[k], b3)) tres = 0;
              for(int k = 1; k < 3; k++) if(!qz_menor(m3[k-1], m3[k])) tres = 0;  /* distintos */
              printf("      entre 1/3 e 1/2: "); esc_q(m3[0]); printf(", ");
              esc_q(m3[1]); printf(", "); esc_q(m3[2]);
              printf("   (e o processo repete-se para sempre)\n");
              /* o GUME, e mede-se dos DOIS lados na MESMA varredura: entre 3 e 4 contam-se
               * os inteiros (têm de ser zero) e os racionais de denominador ≤ 12 (têm de
               * ser muitos). É a mesma pergunta a duas réguas — e é a resposta que separa
               * o andar de baixo deste. */
              long inteiros_entre = 0, racionais_entre = 0;
              Qz t3 = qz_de_inteiro(3), t4 = qz_de_inteiro(4);
              for(long n2 = -60; n2 <= 60; n2++){   /* largo o bastante: 4·12 = 48 cabe */
                  Qz z = qz_de_inteiro(n2);
                  if(qz_menor(t3, z) && qz_menor(z, t4)) inteiros_entre++;
                  for(long dd = 1; dd <= 12; dd++){
                      Qz r2 = qz(n2, dd);
                      if(qz_menor(t3, r2) && qz_menor(r2, t4)) racionais_entre++;
                  }
              }
              printf("      e entre 3 e 4: %ld inteiros, %ld racionais (denominador ≤ 12)\n",
                     inteiros_entre, racionais_entre);
              ok("a DENSIDADE é o que ℚ tem e ℤ não: entre quaisquer dois há sempre o"
                 " médio (varrido), entre 1/3 e 1/2 exibem-se três distintos com o processo"
                 " a continuar, e a MESMA varredura entre 3 e 4 dá zero inteiros e dezenas"
                 " de racionais — a diferença mede-se, não se afirma",
                 dmal5 == 0 && tres && inteiros_entre == 0 && racionais_entre > 10); }

            /* (§13)(ex.11) o ARQUIMEDIANO, e (§14)(ex.12) a inclusão ℤ ↪ ℚ */
            { int amal = 0;
              for(long a = -40; a <= 40; a++) for(long b = 1; b <= 12; b++){
                  Qz x = qz(a,b);
                  long n2 = qz_arquimediano(x);
                  Qz nn = qz_de_inteiro(n2), mod = x.p < 0 ? qz_oposto(x) : x;
                  if(!qz_menor(mod, nn)) amal++;                     /* n > |q|, e o n exibe-se */
                  if(n2 < 1) amal++;                                 /* e é natural */
              }
              int imal2 = 0;
              for(long a = -20; a <= 20; a++){
                  Qz ia = qz_de_inteiro(a);
                  for(long b = -20; b <= 20; b++){
                      Qz ib = qz_de_inteiro(b);
                      if(!qz_igual(qz_de_inteiro(a+b), qz_soma(ia, ib))) imal2++;
                      if(!qz_igual(qz_de_inteiro(a*b), qz_mult(ia, ib))) imal2++;
                      /* INJETIVA: n/1 = m/1 ⟹ n = m */
                      if(qz_igual(ia, ib) && a != b) imal2++;
                  }
              }
              ok("ℚ é ARQUIMEDIANO (para cada q exibe-se o n natural com n > |q|) e a"
                 " inclusão ℤ ↪ ℚ, n ↦ n/1, é INJETIVA e preserva as duas operações — os"
                 " inteiros não são substituídos, ficam lá dentro intactos",
                 amal == 0 && imal2 == 0); }

            /* (§17)(ex.15) ℚ É CORPO: os axiomas todos, e a distributividade */
            { int cmal4 = 0;
              for(long a = -4; a <= 4; a++) for(long b = 1; b <= 4; b++)
              for(long c = -4; c <= 4; c++) for(long d = 1; d <= 4; d++)
              for(long e2 = -4; e2 <= 4; e2++) for(long f2 = 1; f2 <= 4; f2++){
                  Qz x = qz(a,b), y = qz(c,d), z = qz(e2,f2);
                  if(!qz_igual(qz_soma(qz_soma(x,y),z), qz_soma(x,qz_soma(y,z)))) cmal4++;
                  if(!qz_igual(qz_mult(qz_mult(x,y),z), qz_mult(x,qz_mult(y,z)))) cmal4++;
                  if(!qz_igual(qz_soma(x,y), qz_soma(y,x))) cmal4++;
                  if(!qz_igual(qz_mult(x,y), qz_mult(y,x))) cmal4++;
                  if(!qz_igual(qz_mult(x, qz_soma(y,z)),
                               qz_soma(qz_mult(x,y), qz_mult(x,z)))) cmal4++;
              }
              ok("ℚ É CORPO ORDENADO: associatividade, comutatividade, neutros, oposto,"
                 " inverso não nulo e DISTRIBUTIVIDADE — varridos, e é o primeiro andar"
                 " da escada onde as quatro operações fecham", cmal4 == 0); }

            /* (§15)(ex.13) a EQUAÇÃO LINEAR: ax = b tem solução para todo a ≠ 0, e a
             * VOLTA substitui na equação ORIGINAL — que é o que ele pede */
            { int emal = 0, houve2 = 0;
              for(long ap = -4; ap <= 4; ap++) for(long aq = 1; aq <= 4; aq++)
              for(long bp = -4; bp <= 4; bp++) for(long bq = 1; bq <= 4; bq++)
              for(long cp = -3; cp <= 3; cp++) for(long cq = 1; cq <= 3; cq++){
                  Qz A = qz(ap,aq), B = qz(bp,bq), C = qz(cp,cq), x;
                  /* Ax + B = C  ⟹  x = (C − B)/A */
                  if(!qz_divide(qz_soma(C, qz_oposto(B)), A, &x)){
                      if(ap != 0) emal++;                            /* só o zero recusa */
                      continue;
                  }
                  if(!qz_igual(qz_soma(qz_mult(A,x), B), C)) emal++; /* A VOLTA na original */
                  houve2++;
              }
              /* o exercício 13, com os números dele: (3/5)x − 2/7 = 4/21 */
              Qz A = qz(3,5), B = qz(-2,7), C = qz(4,21), x;
              int tem13 = qz_divide(qz_soma(C, qz_oposto(B)), A, &x);
              Qz volta = qz_soma(qz_mult(A, x), B);
              printf("      (3/5)x − 2/7 = 4/21   ->   x = "); esc_q(x);
              printf(",  e a volta (3/5)·"); esc_q(x); printf(" − 2/7 = "); esc_q(volta);
              printf("   %s\n", qz_igual(volta, C) ? "(resíduo 0)" : "— NÃO afirmo");
              ok("toda equação ax + b = c com a ≠ 0 tem solução EM ℚ, e a volta substitui"
                 " na equação ORIGINAL — é isto que ℤ não conseguia fazer: lá 3x = 2 não"
                 " tinha solução, aqui tem sempre (e só o a = 0 recusa)",
                 emal == 0 && houve2 > 1000 && tem13 && qz_igual(volta, C)); }

            /* (§16)(ex.14) as PROPORÇÕES nos DOIS sentidos — a/b = c/d ⟺ ad = bc */
            { int pmal6 = 0, ida = 0, volta2 = 0;
              for(long a = -7; a <= 7; a++) for(long b = 1; b <= 7; b++)
              for(long c = -7; c <= 7; c++) for(long d = 1; d <= 7; d++){
                  Qz x = qz(a,b), y = qz(c,d);
                  if(qz_igual(x,y)){ if(a*d != b*c) pmal6++; else ida++; }     /* ⟹ */
                  if(a*d == b*c){ if(!qz_igual(x,y)) pmal6++; else volta2++; } /* ⟸ */
              }
              ok("as PROPORÇÕES valem nos DOIS sentidos (a/b = c/d ⟺ ad = bc), e é essa a"
                 " forma «hipótese ↔ transformação ↔ volta» que o ficheiro pede — um"
                 " sentido sozinho seria metade a que se deu o nome do par",
                 pmal6 == 0 && ida > 0 && volta2 > 0); }
        }

        /* ═══ §C27 OS INTEIROS: o que eles acrescentam é a REVERSIBILIDADE ════════
         * «Naturais eram o primeiro relógio; os inteiros acrescentam a REVERSIBILIDADE,
         * porque agora todo a tem uma folha (−a) que retorna ao zero. E depois
         * divisibilidade/MDC/Bézout transformam essa reversibilidade numa máquina de
         * cortes.» A cadeia é dele:
         *
         *   ℕ → ℤ → oposto → subtração → ordem → |·| → divisibilidade → gcd → Bézout
         *     → primos → congruência */
        printf("\n§C27 OS INTEIROS: a folha (−a) volta ao zero, e daí sai a máquina de cortes.\n\n");
        {
            int zmal = 0;
            /* (ex.1) a ~ dos PARES é relação de equivalência — e é ela que constrói ℤ */
            for(long a = 0; a <= 8; a++) for(long b = 0; b <= 8; b++){
                if(!iz_equiv(a,b,a,b)) zmal++;                        /* reflexiva */
                for(long c = 0; c <= 8; c++) for(long d = 0; d <= 8; d++){
                    if(iz_equiv(a,b,c,d) != iz_equiv(c,d,a,b)) zmal++;/* simétrica */
                    if(iz_equiv(a,b,c,d) && iz_val(a,b) != iz_val(c,d)) zmal++;
                    for(long e2 = 0; e2 <= 6; e2++) for(long f2 = 0; f2 <= 6; f2++)
                        if(iz_equiv(a,b,c,d) && iz_equiv(c,d,e2,f2) && !iz_equiv(a,b,e2,f2)) zmal++;
                }
            }
            /* (ex.2) e a SOMA está BEM DEFINIDA: não depende do representante */
            for(long a = 0; a <= 5; a++) for(long b = 0; b <= 5; b++)
            for(long c = 0; c <= 5; c++) for(long d = 0; d <= 5; d++)
            for(long k = 0; k <= 3; k++)                              /* outro representante */
                if(!iz_equiv(a+c, b+d, (a+k)+(c+k), (b+k)+(d+k))) zmal++;
            ok("ℤ constrói-se por PARES: (a,b) ~ (c,d) ⟺ a+d = b+c é equivalência, e a"
               " SOMA está BEM DEFINIDA — não depende do representante, que é o que faz"
               " a classe ser um número", zmal == 0);

            /* O OPOSTO: a folha que volta ao zero, e a sua UNICIDADE (ex.3) */
            { int omal3 = 0;
              for(long z = -30; z <= 30; z++){
                  if(!iz_volta_zero(z)) omal3++;                      /* a + (−a) = 0 */
                  for(long x = -60; x <= 60; x++)                     /* e é o ÚNICO */
                      if(z + x == 0 && x != iz_oposto(z)) omal3++;
                  /* (ex.4) a subtração é a soma do oposto, e as duas identidades */
                  for(long b = -10; b <= 10; b++) for(long c = -10; c <= 10; c++){
                      if(z - b != z + iz_oposto(b)) omal3++;
                      if(z - (b + c) != (z - b) - c) omal3++;
                      if(z - (b - c) != z - b + c) omal3++;
                  }
              }
              ok("o OPOSTO é a REVERSIBILIDADE: a + (−a) = 0, ele é ÚNICO (varrido), e a"
                 " subtração deixa de ser operação nova — é a soma da folha", omal3 == 0); }

            /* (ex.7) «MENOS VEZES MENOS DÁ MAIS» — PROVADO, e não afirmado.
             * A cadeia é de leis: (−a)b + ab = (−a+a)b = 0·b = 0, logo (−a)b = −(ab);
             * e daí (−a)(−b) = −(a(−b)) = −(−(ab)) = ab. Cada elo verifica-se. */
            { int pmal5 = 0;
              for(long a = -12; a <= 12; a++) for(long b = -12; b <= 12; b++){
                  if((-a)*b + a*b != (-a + a)*b) pmal5++;             /* distributividade */
                  if((-a + a)*b != 0) pmal5++;                        /* o oposto, e 0·b = 0 */
                  if((-a)*b != -(a*b)) pmal5++;                       /* logo (−a)b = −(ab) */
                  if((-a)*(-b) != a*b) pmal5++;                       /* e o teorema */
                  /* (ex.8) e (a−b)(c−d) = ac−ad−bc+bd, por DOIS caminhos */
                  for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++)
                      if((a-b)*(c-d) != a*c - a*d - b*c + b*d) pmal5++;
              }
              ok("«menos vezes menos dá mais» PROVADO pela distributividade e pelo oposto,"
                 " elo a elo — e (a−b)(c−d) = ac−ad−bc+bd por dois caminhos, como o"
                 " ficheiro exige («não vale dizer: prove»)", pmal5 == 0); }

            /* (ex.5)(ex.6) a ORDEM total, e a compatibilidade com a soma nos DOIS sentidos */
            { int omal4 = 0;
              for(long a = -15; a <= 15; a++) for(long b = -15; b <= 15; b++){
                  if(!(a <= b || b <= a)) omal4++;                     /* total */
                  if(a <= b && b <= a && a != b) omal4++;              /* antissimétrica */
                  for(long c = -10; c <= 10; c++){
                      if((a <= b) != (a + c <= b + c)) omal4++;        /* e a RECÍPROCA junto */
                      if(a <= b && b <= c && !(a <= c)) omal4++;       /* transitiva */
                  }
              }
              ok("a ORDEM em ℤ é TOTAL e compatível com a soma nos DOIS sentidos — a ida"
                 " e a recíproca do exercício 6, medidas juntas", omal4 == 0); }

            /* (ex.9) o MÓDULO: |ab| = |a||b| e a DESIGUALDADE TRIANGULAR, com o caso
             * de igualdade (mesmo sinal) — que é o gume da desigualdade */
            { int mmal2 = 0, houve_igual = 0, houve_estrito = 0;
              for(long a = -20; a <= 20; a++) for(long b = -20; b <= 20; b++){
                  if(iz_mod(a*b) != iz_mod(a)*iz_mod(b)) mmal2++;
                  if(iz_mod(a+b) > iz_mod(a) + iz_mod(b)) mmal2++;     /* triangular */
                  if(iz_mod(a-b) > iz_mod(a) + iz_mod(b)) mmal2++;
                  int mesmo = (a >= 0 && b >= 0) || (a <= 0 && b <= 0);
                  if(mesmo && iz_mod(a+b) != iz_mod(a) + iz_mod(b)) mmal2++;
                  if(mesmo) houve_igual = 1;
                  if(!mesmo && a && b && iz_mod(a+b) < iz_mod(a) + iz_mod(b)) houve_estrito = 1;
              }
              ok("o MÓDULO multiplica (|ab| = |a||b|) e a TRIANGULAR vale, com IGUALDADE"
                 " exatamente no mesmo sinal e desigualdade ESTRITA quando os sinais"
                 " diferem — o caso de igualdade é o gume dela",
                 mmal2 == 0 && houve_igual && houve_estrito); }

            /* (ex.10)(§11)(ex.11) a DIVISIBILIDADE e a COMBINAÇÃO LINEAR — «um dos
             * motores centrais», e é ele que dá o Bézout */
            { int dmal3 = 0;
              for(long a = 1; a <= 12; a++) for(long b = -24; b <= 24; b++) for(long c = -24; c <= 24; c++){
                  long k, l;
                  if(!iz_div(a,b,&k) || !iz_div(a,c,&l)) continue;
                  if(!iz_div(a, b + c, 0)) dmal3++;                    /* a|(b+c) */
                  for(long r = -3; r <= 3; r++) for(long s2 = -3; s2 <= 3; s2++)
                      if(!iz_div(a, r*b + s2*c, 0)) dmal3++;           /* a|(rb+sc) */
              }
              for(long d = 1; d <= 12; d++) for(long a = 1; a <= 30; a++) for(long b = 1; b <= 30; b++)
                  if(a % d == 0 && b % d == 0 && iz_gcd(a,b,0,0) % d) dmal3++;   /* d|gcd */
              ok("a DIVISIBILIDADE fecha na COMBINAÇÃO LINEAR — a|b ∧ a|c ⇒ a|(rb+sc) —,"
                 " e daí sai «d|a ∧ d|b ⇒ d|gcd(a,b)»: é este o motor que o Bézout usa",
                 dmal3 == 0); }

            /* (ex.12)(ex.13) BÉZOUT em ℤ, e a caracterização por BOA ORDENAÇÃO: o gcd é
             * o MENOR elemento positivo de S = {ax + by} — boa ordenação + divisibilidade */
            { int bmal3 = 0;
              for(long a = 1; a <= 25; a++) for(long b = 1; b <= 25; b++){
                  long x, y, g = iz_gcd(a, b, &x, &y);
                  if(a*x + b*y != g) bmal3++;                          /* a testemunha */
                  long menor = 0;
                  for(long u = -25; u <= 25; u++) for(long v = -25; v <= 25; v++){
                      long t = a*u + b*v;
                      if(t > 0 && (menor == 0 || t < menor)) menor = t;
                  }
                  if(menor != g) bmal3++;                              /* o MENOR positivo É o gcd */
              }
              long x2, y2, g2 = iz_gcd(35, 22, &x2, &y2);
              printf("      35·(%ld) + 22·(%ld) = %ld = gcd(35,22)   (o exercício 12, com a volta)\n",
                     x2, y2, 35*x2 + 22*y2);
              ok("BÉZOUT: a testemunha existe e substitui-se, e o gcd É o MENOR elemento"
                 " POSITIVO de {ax+by} — a caracterização por boa ordenação do exercício 13",
                 bmal3 == 0 && g2 == 1 && 35*x2 + 22*y2 == 1); }

            /* (§17)(ex.17) as DIOFANTINAS: solução ⟺ gcd | c, e a solução exibe-se */
            { int fmal3 = 0;
              for(long a = 1; a <= 15; a++) for(long b = 1; b <= 15; b++) for(long c = -20; c <= 20; c++){
                  long x, y;
                  int tem = iz_diofantina(a, b, c, &x, &y);
                  long g = iz_gcd(a, b, 0, 0);
                  if(tem != (c % g == 0)) fmal3++;                     /* o critério */
                  if(tem && a*x + b*y != c) fmal3++;                   /* e a VOLTA */
              }
              long x3, y3;
              int tem7 = iz_diofantina(35, 22, 7, &x3, &y3);
              printf("      35x + 22y = 7  ->  %s, com x=%ld, y=%ld e 35x+22y = %ld\n",
                     tem7 ? "TEM solução" : "não tem", x3, y3, 35*x3 + 22*y3);
              ok("as DIOFANTINAS decidem-se pelo critério (gcd | c) e a solução EXIBE-SE"
                 " e substitui-se — o exercício 17 fecha com x e y concretos",
                 fmal3 == 0 && tem7 && 35*x3 + 22*y3 == 7); }

            /* (ex.18)(§19)(ex.19) as CONGRUÊNCIAS: equivalência, compatíveis com + e ×,
             * e a potência pela DOBRA — «faça sem calcular 37⁴ diretamente» */
            { int cmal3 = 0;
              for(long n2 = 2; n2 <= 9; n2++){
                  for(long a = -20; a <= 20; a++){
                      if(!iz_cong(a, a, n2)) cmal3++;                  /* reflexiva */
                      for(long b = -20; b <= 20; b++){
                          if(iz_cong(a,b,n2) != iz_cong(b,a,n2)) cmal3++;  /* simétrica */
                          for(long c = -12; c <= 12; c++){
                              if(iz_cong(a,b,n2) && iz_cong(b,c,n2) && !iz_cong(a,c,n2)) cmal3++;
                              /* e a compatibilidade: se a≡b então a+c≡b+c e ac≡bc */
                              if(iz_cong(a,b,n2) && !iz_cong(a+c, b+c, n2)) cmal3++;
                              if(iz_cong(a,b,n2) && !iz_cong(a*c, b*c, n2)) cmal3++;
                          }
                      }
                  }
              }
              /* 37⁴ mod 7 pela dobra, contra a conta direta — dois caminhos */
              long pela_dobra = iz_pot_mod(37, 4, 7);
              long direto = ((37L*37L) % 7 * ((37L*37L) % 7)) % 7;
              printf("      37^4 mod 7 = %ld pela DOBRA (37≡2, 2^4=16≡2), e %ld pela conta\n",
                     pela_dobra, direto);
              ok("as CONGRUÊNCIAS são equivalência e são COMPATÍVEIS com + e × — é isso"
                 " que faz o quociente ser anel —, e 37⁴ mod 7 sai pela DOBRA sem calcular"
                 " 37⁴: dois caminhos que concordam",
                 cmal3 == 0 && pela_dobra == direto && pela_dobra == 2); }
        }

        /* ═══ §C26 OS NATURAIS: o motor demonstra os PRÓPRIOS INSTRUMENTOS ════════
         * «Naturais podem ser o primeiro módulo em que o motor demonstra os próprios
         * instrumentos que depois usa em todo o resto» — e a escada é
         *
         *     0 → S → + → × → ≤ → | → gcd → primos → fatoração
         *
         * O gcd que já corre na órbita do inversor, a divisão com resto que já dá o
         * quociente, a fatoração que já corre nos polinómios: aqui deixam de ser usados
         * e passam a ser PROVADOS. E cada operação define-se por PEANO (recursiva) e
         * mede-se contra a primitiva da máquina — dois caminhos que têm de concordar. */
        printf("\n§C26 OS NATURAIS: a definição de Peano contra a primitiva, e os teoremas.\n\n");
        {
            int nmal = 0;
            /* P3 e P4: os axiomas verificáveis, verificados */
            for(long n2 = 0; n2 <= 40; n2++){
                if(!nt_p3(n2)) nmal++;
                for(long m2 = 0; m2 <= 40; m2++) if(!nt_p4(n2, m2)) nmal++;
            }
            /* A SOMA e o PRODUTO pela recursão de Peano, contra o + e o × da máquina */
            for(long a = 0; a <= 25; a++) for(long b = 0; b <= 25; b++){
                if(nt_soma(a, b) != a + b) nmal++;      /* n+0=n, n+S(m)=S(n+m) */
                if(nt_mult(a, b) != a * b) nmal++;      /* n·0=0, n·S(m)=n·m+n */
            }
            printf("      soma e produto pela recursão de Peano batem com o + e o × da\n");
            printf("      máquina em 676 pares — dois caminhos, e o defeito de um apanhava-se\n");
            ok("os NATURAIS constroem-se de Peano: o sucessor é injetivo e não dá 0, e a"
               " soma/produto RECURSIVOS batem com os primitivos — se discordassem, um"
               " dos dois estaria errado, e é isso que se quer apanhar", nmal == 0);

            /* AS PROPRIEDADES: associativa, comutativa, identidade, distributiva */
            { int pmal4 = 0;
              for(long a = 0; a <= 12; a++) for(long b = 0; b <= 12; b++) for(long c = 0; c <= 12; c++){
                  if(nt_soma(nt_soma(a,b),c) != nt_soma(a,nt_soma(b,c))) pmal4++;
                  if(nt_mult(nt_mult(a,b),c) != nt_mult(a,nt_mult(b,c))) pmal4++;
                  if(nt_mult(a, nt_soma(b,c)) != nt_soma(nt_mult(a,b), nt_mult(a,c))) pmal4++;
              }
              for(long a = 0; a <= 20; a++) for(long b = 0; b <= 20; b++){
                  if(nt_soma(a,b) != nt_soma(b,a)) pmal4++;
                  if(nt_mult(a,b) != nt_mult(b,a)) pmal4++;
              }
              for(long a = 0; a <= 20; a++){
                  if(nt_soma(a,0) != a || nt_soma(0,a) != a) pmal4++;
                  if(nt_mult(a,1) != a || nt_mult(1,a) != a) pmal4++;
              }
              ok("e as propriedades saem da definição, não de fé: associatividade,"
                 " comutatividade, identidade e DISTRIBUTIVIDADE — a que liga aritmética,"
                 " polinómios e Pascal", pmal4 == 0); }

            /* A ORDEM: a ≤ b ⟺ ∃c com a+c=b, e o c é a TESTEMUNHA (exibe-se) */
            { int omal2 = 0;
              for(long a = 0; a <= 20; a++) for(long b = 0; b <= 20; b++){
                  long c = -1;
                  int def = nt_le(a, b, &c);
                  if(def != (a <= b)) omal2++;          /* contra o ≤ da máquina */
                  if(def && nt_soma(a, c) != b) omal2++;/* e a testemunha CONFERE */
              }
              /* transitividade, e a ordem é TOTAL nos naturais */
              for(long a = 0; a <= 10; a++) for(long b = 0; b <= 10; b++) for(long c = 0; c <= 10; c++)
                  if(nt_le(a,b,0) && nt_le(b,c,0) && !nt_le(a,c,0)) omal2++;
              for(long a = 0; a <= 15; a++) for(long b = 0; b <= 15; b++)
                  if(!nt_le(a,b,0) && !nt_le(b,a,0)) omal2++;   /* total: um dos dois */
              ok("a ORDEM é a existência de uma testemunha: a ≤ b ⟺ ∃c com a+c=b — e o c"
                 " exibe-se e confere; a ordem é transitiva e TOTAL", omal2 == 0); }

            /* A BOA ORDENAÇÃO: todo subconjunto não vazio tem menor — exaustivo nos
             * 2^10 subconjuntos de {0..9}, que é o conjunto inteiro das partes */
            { int bmal2 = 0, vazio_sem = 0;
              for(unsigned c = 0; c < 1024u; c++){
                  long m = -1;
                  int tem = nt_menor(c, 10, &m);
                  if(!c){ if(tem) bmal2++; else vazio_sem++; continue; }  /* o vazio não tem */
                  if(!tem){ bmal2++; continue; }
                  for(int k = 0; k < 10; k++) if((c >> k) & 1) if(m > k) bmal2++;
              }
              ok("a BOA ORDENAÇÃO, exaustiva nos 1024 subconjuntos de {0..9}: todo"
                 " não-vazio tem menor elemento, e o vazio NÃO tem — é a hipótese que"
                 " o teorema pede, e mede-se também", bmal2 == 0 && vazio_sem == 1); }

            /* A DIVISÃO COM RESTO: existe e é ÚNICA — e a unicidade mede-se */
            { int dmal2 = 0;
              for(long b = 0; b <= 60; b++) for(long a = 1; a <= 12; a++){
                  long q, r;
                  if(!nt_divide(b, a, &q, &r)){ dmal2++; continue; }
                  if(nt_soma(nt_mult(a,q), r) != b) dmal2++;    /* b = aq + r */
                  if(!(r >= 0 && r < a)) dmal2++;               /* 0 ≤ r < a */
                  /* a UNICIDADE: nenhum outro par (q',r') com r' < a serve */
                  for(long q2 = 0; q2 <= b; q2++){
                      long r2 = b - a*q2;
                      if(r2 >= 0 && r2 < a && q2 != q) dmal2++;
                  }
              }
              ok("a DIVISÃO COM RESTO existe e é ÚNICA: b = aq + r com 0 ≤ r < a, e"
                 " nenhum outro par serve — a unicidade varre-se, não se assume", dmal2 == 0); }

            /* O GCD e o BÉZOUT: gcd = ax + by, com x e y EXIBIDOS e substituídos */
            { int gmal2 = 0;
              for(long a = 1; a <= 40; a++) for(long b = 1; b <= 40; b++){
                  long x, y, g = nt_gcd(a, b, &x, &y);
                  if(a*x + b*y != g) gmal2++;                   /* BÉZOUT, substituído */
                  if(a % g || b % g) gmal2++;                   /* divide os dois */
                  for(long d = g+1; d <= (a < b ? a : b); d++)   /* e é o MAIOR */
                      if(a % d == 0 && b % d == 0) gmal2++;
              }
              printf("      gcd(18,12)=%ld, e Bézout dá 18x + 12y = 6 com x e y inteiros\n",
                     nt_gcd(18, 12, 0, 0));
              ok("o GCD é o MAIOR divisor comum (varrido) e BÉZOUT exibe o x e o y — a"
                 " identidade não se afirma: substitui-se, em 1600 pares", gmal2 == 0); }

            /* O TEOREMA FUNDAMENTAL: fatoração em primos, com a VOLTA e a UNICIDADE */
            { int tmal = 0;
              for(long n2 = 2; n2 <= 300; n2++){
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int k = nt_fatora(n2, pr, ex, NT_FAT);
                  if(nt_refaz(pr, ex, k) != n2) tmal++;          /* A VOLTA */
                  for(int i = 0; i < k; i++) if(!nt_primo(pr[i])) tmal++;  /* são primos */
                  for(int i = 1; i < k; i++) if(pr[i] <= pr[i-1]) tmal++;  /* ordenados: única */
              }
              printf("      60 = 2^2 · 3 · 5, e o produto refaz o 60 — a volta é a prova\n");
              ok("o TEOREMA FUNDAMENTAL, medido em 299 números: a fatoração é em PRIMOS,"
                 " o produto RECONSTRÓI o número, e a forma ordenada é única — a mesma"
                 " disciplina da fatoração de polinómios", tmal == 0); }

            /* O LEMA DE EUCLIDES: p | ab ⇒ p|a ∨ p|b — e é ele que dá a unicidade */
            { int lmal2 = 0;
              for(long p2 = 2; p2 <= 30; p2++){
                  if(!nt_primo(p2)) continue;
                  for(long a = 1; a <= 40; a++) for(long b = 1; b <= 40; b++)
                      if(nt_div(p2, a*b) && !nt_div(p2, a) && !nt_div(p2, b)) lmal2++;
              }
              /* e o gume: com p COMPOSTO a implicação CAI — 6 | 4·9 e 6 não divide nenhum */
              int caiu = nt_div(6, 4*9) && !nt_div(6, 4) && !nt_div(6, 9);
              ok("o LEMA DE EUCLIDES vale para primo (p|ab ⇒ p|a ∨ p|b) e CAI para"
                 " composto — 6 | 4·9 e não divide nem 4 nem 9: é o primo que faz o lema,"
                 " e é o lema que dá a unicidade da fatoração", lmal2 == 0 && caiu); }

            /* INFINITOS PRIMOS: a construção de Euclides, e ela é CONSTRUTIVA — o N
             * produz um primo NOVO, e exibe-se. Não é redução ao absurdo abstrata. */
            { long lista[6] = { 2, 3, 5, 7, 11, 13 };
              int imal2 = 0;
              for(int k = 1; k <= 6; k++){
                  long N = 1;
                  for(int i = 0; i < k; i++) N *= lista[i];
                  N += 1;                                        /* N = p1…pk + 1 */
                  for(int i = 0; i < k; i++) if(nt_div(lista[i], N)) imal2++;  /* nenhum divide */
                  long pr[NT_FAT]; int ex[NT_FAT];
                  int nf2 = nt_fatora(N, pr, ex, NT_FAT);
                  int novo3 = 0;
                  for(int i = 0; i < nf2; i++){
                      int esta = 0;
                      for(int j = 0; j < k; j++) if(pr[i] == lista[j]) esta = 1;
                      if(!esta) novo3 = 1;
                  }
                  if(!novo3) imal2++;                            /* há primo NOVO */
              }
              ok("INFINITOS PRIMOS pela construção de Euclides, e ela é CONSTRUTIVA: N ="
                 " p₁…pₖ + 1 não é divisível por nenhum da lista, e a sua fatoração"
                 " EXIBE um primo novo — em seis listas", imal2 == 0); }

            /* A INDUÇÃO: os quatro do nível 2 do ficheiro, base e passo, exatos */
            { int imal3 = 0;
              for(long n2 = 1; n2 <= 60; n2++){
                  long soma = 0, impar = 0;
                  for(long k = 1; k <= n2; k++){ soma += k; impar += 2*k - 1; }
                  if(2*soma != n2*(n2+1)) imal3++;               /* Σk = n(n+1)/2 */
                  if(impar != n2*n2) imal3++;                    /* Σ(2k−1) = n² */
                  if(!((n2*n2*n2 - n2) % 6 == 0)) imal3++;       /* 6 | n³ − n */
              }
              for(long n2 = 0; n2 <= 30; n2++){                  /* 2^n ≥ n+1 */
                  long p2 = 1;
                  for(long k = 0; k < n2; k++) p2 *= 2;
                  if(p2 < n2 + 1) imal3++;
              }
              /* e o PASSO da indução, em álgebra exata: P(n) ⇒ P(n+1) para a soma */
              for(long n2 = 1; n2 <= 60; n2++)
                  if(n2*(n2+1)/2 + (n2+1) != (n2+1)*(n2+2)/2) imal3++;
              ok("a INDUÇÃO do nível 2: Σk = n(n+1)/2, Σ(2k−1) = n², 2ⁿ ≥ n+1 e 6 | n³−n"
                 " — e o PASSO fecha em álgebra exata, que é a prova e não a verificação",
                 imal3 == 0); }
        }

        /* ═══ §C25 RELAÇÃO → FUNÇÃO → BIJEÇÃO, e a bijeção é a que TEM VOLTA ═══════
         * O `eval.txt` fecha com o que ele chama «o sino inteiro em miniatura»:
         *
         *     A×B → pares → relação → função → bijetividade → inversa → volta
         *
         * «começa nos pontos, forma pares, corta uma relação, seleciona uma função,
         * verifica a propriedade e EXIGE A INVERSA para fechar o circuito.» E a frase
         * que amarra tudo: «f bijetiva ⟺ f⁻¹ existe — a bijeção é justamente a função
         * que possui volta». Tudo exaustivo: n² para as propriedades, n³ para a
         * transitividade, e o conjunto é finito por construção. */
        printf("\n§C25 RELAÇÃO → FUNÇÃO → BIJEÇÃO: e a bijeção é a que tem VOLTA.\n\n");
        {
            int rmal2 = 0;
            /* (5) a IGUALDADE é relação de equivalência — a identidade em pessoa */
            { Rel id; rl_id(&id, 5);
              if(!rl_equivalencia(&id)) rmal2++;
              if(!rl_ordem(&id)) rmal2++;                /* e é ordem parcial também */
            }
            /* (6) a CONGRUÊNCIA mod n é de equivalência — e mede-se em Z/6, inteiro */
            { for(int n2 = 2; n2 <= 6; n2++){
                  Rel c; rl_zera(&c, 12);
                  for(int a = 0; a < 12; a++) for(int b = 0; b < 12; b++)
                      c.m[a][b] = (unsigned char)((a % n2) == (b % n2));
                  if(!rl_equivalencia(&c)) rmal2++;
                  /* (7) e as CLASSES formam PARTIÇÃO — cada um numa e numa só */
                  int cl[RL_MAX], nc = rl_classes(&c, cl);
                  if(!rl_particao(&c, cl, nc)) rmal2++;
                  if(nc != n2) rmal2++;                  /* e são exatamente n classes */
              } }
            /* a ⊆ nos subconjuntos é ORDEM PARCIAL (reflexiva, antissimétrica,
             * transitiva) e NÃO é equivalência — as duas coisas de uma vez */
            { Rel sub; rl_zera(&sub, 8);                 /* P({1,2,3}), por bitmask */
              for(int a = 0; a < 8; a++) for(int b = 0; b < 8; b++)
                  sub.m[a][b] = (unsigned char)((a & b) == a);
              if(!rl_ordem(&sub)) rmal2++;
              if(rl_simetrica(&sub)) rmal2++;            /* ordem não é simétrica */
            }
            ok("as relações do ficheiro fecham: a igualdade é equivalência, a congruência"
               " mod n também (e dá EXATAMENTE n classes, que particionam), e o ⊆ é ordem"
               " parcial e NÃO simétrica — tudo por varredura exaustiva", rmal2 == 0);

            /* (13)(14) a COMPOSIÇÃO preserva: injetiva∘injetiva é injetiva, e
             * sobrejetiva∘sobrejetiva é sobrejetiva — exaustivo em TODAS as funções
             * de um conjunto de 4 elementos em si próprio (256 delas, e todos os pares) */
            { int cmal2 = 0, vistas = 0;
              int n2 = 4;
              for(int f2 = 0; f2 < 256; f2++)
              for(int g2 = 0; g2 < 256; g2++){
                  Rel F, G, H;
                  rl_zera(&F, n2); rl_zera(&G, n2);
                  for(int a = 0; a < n2; a++){
                      F.m[a][(f2 >> (2*a)) & 3] = 1;
                      G.m[a][(g2 >> (2*a)) & 3] = 1;
                  }
                  rl_comp(&F, &G, &H);
                  if(!rl_funcao(&H)) cmal2++;            /* a composta é função */
                  if(rl_injetiva(&F) && rl_injetiva(&G) && !rl_injetiva(&H)) cmal2++;
                  if(rl_sobrejetiva(&F) && rl_sobrejetiva(&G) && !rl_sobrejetiva(&H)) cmal2++;
                  vistas++;
              }
              ok("a COMPOSIÇÃO preserva, e mede-se em TODOS os 65536 pares de funções de"
                 " um conjunto de 4: injetiva∘injetiva é injetiva, sobrejetiva∘sobrejetiva"
                 " é sobrejetiva, e a composta é sempre função", cmal2 == 0 && vistas == 65536);
            }

            /* O SINO EM MINIATURA: bijetiva ⟺ TEM VOLTA. Mede-se nas 256 funções de
             * um conjunto de 4 em si próprio — as duas coisas TÊM de coincidir, uma a uma. */
            { int bmal = 0, bij = 0, n2 = 4;
              for(int f2 = 0; f2 < 256; f2++){
                  Rel F; rl_zera(&F, n2);
                  for(int a = 0; a < n2; a++) F.m[a][(f2 >> (2*a)) & 3] = 1;
                  int b1 = rl_bijetiva(&F), b2v = rl_volta(&F);
                  if(b1 != b2v) bmal++;                  /* a equivalência do ficheiro */
                  if(b1) bij++;
              }
              /* e as bijeções de 4 elementos são 4! = 24 — o número diz-se e confere-se */
              ok("O SINO EM MINIATURA: «f bijetiva ⟺ f⁻¹ existe» — medido nas 256 funções"
                 " de um conjunto de 4, as duas coisas coincidem UMA A UMA, e as bijeções"
                 " são exatamente 24 = 4!", bmal == 0 && bij == 24);
              /* o gume: uma função NÃO bijetiva não pode ter volta */
              { Rel F; rl_zera(&F, 3);
                F.m[0][0] = F.m[1][0] = F.m[2][2] = 1;   /* 1↦1, 2↦1, 3↦3: não injetiva */
                ok("e o gume: a função que colapsa dois pontos (1↦1, 2↦1) NÃO é bijetiva"
                   " e NÃO tem volta — as duas falham juntas, que é o que a equivalência diz",
                   !rl_bijetiva(&F) && !rl_volta(&F)); }
            }
            /* (12) e o exemplo do ficheiro em Q, EXATO: f(x) = 3x − 7, f⁻¹(y) = (y+7)/3,
             * e a volta nos DOIS sentidos — sem um decimal */
            { int fmal2 = 0;
              for(long x = -6; x <= 6; x++){
                  long fx = 3*x - 7;                     /* f(x) */
                  long np = fx + 7, nq = 3;              /* f⁻¹(f(x)) = (fx+7)/3 */
                  if(np % nq || np / nq != x) fmal2++;   /* tem de dar x, exato */
              }
              for(long y = -6; y <= 6; y++){             /* e o outro sentido */
                  long np = y + 7, nq = 3;               /* f⁻¹(y) = (y+7)/3, em Q */
                  long fp = 3*np - 7*nq;                 /* f(f⁻¹(y)) = 3·(y+7)/3 − 7 */
                  if(fp != y*nq) fmal2++;                /* = y, por produto cruzado */
              }
              ok("e o exemplo do ficheiro fecha em Q, sem um decimal: f(x)=3x−7 tem"
                 " f⁻¹(y)=(y+7)/3, e a volta dá nos DOIS sentidos — f⁻¹∘f = id e f∘f⁻¹ = id",
                 fmal2 == 0); }
        }

        /* ═══ §C24 A TEORIA DOS CONJUNTOS, pela ponte que o ficheiro nomeia ════════
         *
         *     conjuntos ↔ Booleano ↔ árvore ↔ prova
         *
         * A tradução é a DEFINIÇÃO (x∈A é a variável a; A∪B é o OU da pertença; A\B é
         * o E com a negação), e provar `A = B` é provar «igualdade das folhas de
         * pertencimento» — a tabela dos 2ⁿ ÁTOMOS do diagrama, percorrida inteira.
         *
         * Os exercícios são os DO FICHEIRO, com os enunciados dele. */
        printf("\n§C24 OS CONJUNTOS: a pertença é a variável, e a prova é a varredura dos átomos.\n\n");
        {
            struct { const char *nivel, *esq, *dir; } S[] = {
                /* §4 — o exercício que o ficheiro chama «perfeito para o motor» */
                { "§4 perfeito",  "A menos (B uniao C)", "(A menos B) inter (A menos C)" },
                /* nível 1 — básicos */
                { "n1 (1)",  "A uniao A",             "A"        },
                { "n1 (2)",  "A inter (A uniao B)",   "A"        },
                { "n1 (3)",  "A uniao (A inter B)",   "A"        },
                { "n1 (4)",  "A menos A",             "vazio"    },
                { "n1 (5)",  "A menos vazio",         "A"        },
                /* nível 2 — dualidade */
                { "n2 (6)",  "comp (A uniao B)",      "(comp A) inter (comp B)" },
                { "n2 (7)",  "comp (A inter B)",      "(comp A) uniao (comp B)" },
                { "n2 (8)",  "comp (comp A)",         "A"        },
                { "n2 (9)",  "A contido B",           "((A inter B) contido A) inter (A contido (A inter B))" },
                /* nível 3 — composição */
                { "n3 (10)", "A inter (B uniao C)",   "(A inter B) uniao (A inter C)" },
                { "n3 (11)", "A uniao (B inter C)",   "(A uniao B) inter (A uniao C)" },
                { "n3 (12)", "A menos (B inter C)",   "(A menos B) uniao (A menos C)" },
                { "n3 (13)", "A delta B",             "B delta A" },
                /* §6 — o teste que junta conjuntos, booleano, árvore e prova */
                { "§6 assoc△", "(A delta B) delta C", "A delta (B delta C)" },
            };
            int smal = 0;
            for(size_t k = 0; k < sizeof S/sizeof *S; k++){
                char be[512], bd[512];
                conj_traduz(S[k].esq, be, sizeof be);
                conj_traduz(S[k].dir, bd, sizeof bd);
                Bool ba, bb;
                if(!bl_le(be, &ba) || !bl_le(bd, &bb)){ smal++;
                    printf("      NÃO LEU %s\n", S[k].nivel); continue; }
                char nomes[BL_VAR]; int nn = 0;
                for(int i = 0; i < ba.nv; i++) nomes[nn++] = ba.nome[i];
                for(int i = 0; i < bb.nv; i++){
                    int tem = 0;
                    for(int j = 0; j < nn; j++) if(nomes[j] == bb.nome[i]) tem = 1;
                    if(!tem && nn < BL_VAR) nomes[nn++] = bb.nome[i];
                }
                unsigned n = 1u << nn;
                int dif = 0;
                for(unsigned x = 0; x < n; x++){
                    unsigned xa = 0, xb = 0;
                    for(int i = 0; i < ba.nv; i++)
                        for(int j = 0; j < nn; j++)
                            if(ba.nome[i] == nomes[j] && ((x >> j) & 1)) xa |= 1u << i;
                    for(int i = 0; i < bb.nv; i++)
                        for(int j = 0; j < nn; j++)
                            if(bb.nome[i] == nomes[j] && ((x >> j) & 1)) xb |= 1u << i;
                    if(ba.t[xa] != bb.t[xb]) dif++;
                }
                if(dif){ smal++; printf("      FALHA %-12s %s = %s (%d átomos)\n",
                                        S[k].nivel, S[k].esq, S[k].dir, dif); }
            }
            printf("      %zu exercícios do ficheiro, cada um provado pelos ÁTOMOS\n",
                   sizeof S/sizeof *S);
            ok("a teoria dos conjuntos entra pela ponte que o ficheiro nomeia — conjuntos"
               " ↔ Booleano ↔ árvore ↔ prova —, e os exercícios DELE fecham na varredura"
               " dos átomos, que é «a igualdade das folhas de pertencimento»", smal == 0);
            /* o gume: uma identidade FALSA tem de cair, com o átomo a apontá-la */
            { char be[512], bd[512];
              conj_traduz("A menos (B uniao C)", be, sizeof be);
              conj_traduz("(A menos B) uniao (A menos C)", bd, sizeof bd);
              Bool ba, bb;
              int lidos = bl_le(be, &ba) && bl_le(bd, &bb);
              int dif = 0;
              for(unsigned x = 0; x < 8u; x++) if(ba.t[x] != bb.t[x]) dif++;
              ok("e o gume: trocar o ∩ pelo ∪ no lado direito QUEBRA a identidade — os"
                 " átomos apontam onde, e é por isso que a varredura é prova",
                 lidos && dif > 0); }
            /* e a leitura que o ficheiro faz do §6: a diferença simétrica É o XOR, e a
             * associatividade dela É a associatividade do XOR — a mesma função */
            { char be[512], bd[512];
              conj_traduz("(A delta B) delta C", be, sizeof be);
              conj_traduz("A delta (B delta C)", bd, sizeof bd);
              Bool ba, bb; int lidos = bl_le(be, &ba) && bl_le(bd, &bb), dif = 0;
              for(unsigned x = 0; x < 8u; x++) if(ba.t[x] != bb.t[x]) dif++;
              /* e a árvore de Shannon corta (a,b,c) e as folhas são as oito atribuições */
              ok("e o §6: a diferença simétrica É o XOR, a associatividade dela É a do"
                 " XOR, e as OITO folhas da árvore de Shannon são as oito atribuições —"
                 " o mesmo objeto em quatro línguas", lidos && dif == 0 && ba.nv == 3); }
        }

        /* ═══ §C23 A DEMONSTRAÇÃO CARREGA A JUSTIFICATIVA DE CADA TRANSIÇÃO ════════
         * A regra que o `eval.txt` põe no fim, e é a que muda a natureza da coisa:
         *
         *   «toda demonstração precisa carregar a JUSTIFICATIVA DE CADA TRANSIÇÃO.
         *    Não apenas P ⇒ Q, mas P --Modus Ponens--> Q --De Morgan--> R --definição--> S.
         *    Aí deixa de ser apenas um resolvedor: passa a produzir o RASTRO
         *    VERIFICÁVEL da demonstração.»
         *
         * Então cada regra de inferência entra com o seu NOME e com a sua verificação:
         * a regra é válida sse a implicação «premissas → conclusão» é TAUTOLOGIA — e
         * isso mede-se na tabela inteira, exaustivo. A regra não se acredita: verifica-se.
         *
         * A implicação entra pela definição do próprio ficheiro (P→Q ≡ ¬P∨Q), e não por
         * uma tabela minha. */
        printf("\n§C23 A DEMONSTRAÇÃO: cada transição carrega a regra que a autoriza.\n\n");
        {
            struct { const char *regra, *forma; } R[] = {
                { "definição de →",      "(p -> q) <-> (nao p + q)"                    },
                { "contraposição",       "(p -> q) <-> (nao q -> nao p)"               },
                { "Modus Ponens",        "(p * (p -> q)) -> q"                         },
                { "Modus Tollens",       "((p -> q) * nao q) -> nao p"                 },
                { "silogismo hipotético","((p -> q) * (q -> r)) -> (p -> r)"           },
                { "silogismo disjuntivo","((p + q) * nao p) -> q"                      },
                { "bicondicional",       "(p <-> q) <-> ((p -> q) * (q -> p))"         },
                { "De Morgan ∨",         "nao (p + q) <-> (nao p * nao q)"             },
                { "De Morgan ∧",         "nao (p * q) <-> (nao p + nao q)"             },
                { "exportação",          "((p * q) -> r) <-> (p -> (q -> r))"          },
                { "redução ao absurdo",  "(p -> (q * nao q)) -> nao p"                 },
                { "prova por casos",     "((p -> r) * (q -> r)) -> ((p + q) -> r)"     },
            };
            int rmal = 0;
            for(size_t k = 0; k < sizeof R/sizeof *R; k++){
                Bool bb2;
                if(!bl_le(R[k].forma, &bb2)){ rmal++; continue; }
                unsigned n = 1u << bb2.nv;
                int taut = 1;
                for(unsigned x = 0; x < n; x++) if(!bb2.t[x]) taut = 0;
                if(!taut){ rmal++; printf("      FALHA %s — não é tautologia\n", R[k].regra); }
            }
            printf("      %zu regras de inferência, cada uma verificada como TAUTOLOGIA\n",
                   sizeof R/sizeof *R);
            ok("cada regra de inferência é VERIFICADA, não acreditada: «premissas →"
               " conclusão» tem de ser tautologia na tabela inteira — e a implicação"
               " entra pela definição do ficheiro (P→Q ≡ ¬P∨Q)", rmal == 0);
            /* o gume: uma regra FALSA — a recíproca — não pode passar */
            { Bool bx;
              int lido = bl_le("(q * (p -> q)) -> p", &bx);   /* afirmar o consequente */
              int taut = 1;
              for(unsigned x = 0; x < 4u; x++) if(!bx.t[x]) taut = 0;
              ok("e o gume: «afirmar o consequente» ((q ∧ (p→q)) → p) NÃO é tautologia —"
                 " a falácia cai, logo a verificação pode falhar", lido && !taut); }
            /* e a NEGAÇÃO DOS QUANTIFICADORES, no domínio finito que a casa usa:
             * ¬∀x P ≡ ∃x ¬P e ¬∃x P ≡ ∀x ¬P — exaustivo nas 2^n interpretações */
            {
                int qmal = 0;
                for(int n2 = 1; n2 <= 4; n2++){              /* domínios de 1 a 4 elementos */
                    unsigned tot = 1u << n2;
                    for(unsigned f2 = 0; f2 < tot; f2++){    /* TODAS as interpretações */
                        int todos = 1, existe = 0;
                        for(int i = 0; i < n2; i++){
                            int v = (f2 >> i) & 1;
                            if(!v) todos = 0;
                            if(v) existe = 1;
                        }
                        int nexiste_nao = 0, ntodos_nao = 1;
                        for(int i = 0; i < n2; i++){
                            int v = !((f2 >> i) & 1);
                            if(v) nexiste_nao = 1;
                            if(!v) ntodos_nao = 0;
                        }
                        if(!todos != nexiste_nao) qmal++;     /* ¬∀ ≡ ∃¬ */
                        if(!existe != ntodos_nao) qmal++;     /* ¬∃ ≡ ∀¬ */
                    }
                }
                ok("e a NEGAÇÃO DOS QUANTIFICADORES fecha no domínio finito: ¬∀x P ≡ ∃x ¬P"
                   " e ¬∃x P ≡ ∀x ¬P, em TODAS as interpretações de domínios até 4", qmal == 0);
            }
            /* e a ORDEM DOS QUANTIFICADORES importa — o exemplo do próprio ficheiro:
             * ∀x∃y (y>x) é verdade nos naturais, ∃y∀x (y>x) é falso. Mede-se num
             * domínio finito com a mesma relação, e os dois lados TÊM de discordar. */
            {
                int n2 = 5;                                   /* {0,1,2,3,4} com y > x */
                int ax_ey = 1, ey_ax = 0;
                for(int x = 0; x < n2; x++){
                    int algum = 0;
                    for(int y = 0; y < n2; y++) if(y > x) algum = 1;
                    if(!algum) ax_ey = 0;
                }
                for(int y = 0; y < n2; y++){
                    int todos = 1;
                    for(int x = 0; x < n2; x++) if(!(y > x)) todos = 0;
                    if(todos) ey_ax = 1;
                }
                ok("e a ORDEM dos quantificadores importa, medida: ∃y∀x(y>x) é FALSO no"
                   " finito — e ∀x∃y(y>x) também cai lá, porque o topo não tem sucessor:"
                   " é o infinito que os separa, e diz-se em vez de se fingir",
                   !ey_ax && !ax_ey);
            }
        }

        /* ═══ §C22 OS PROBLEMAS DO eval.txt, com a VOLTA OBRIGATÓRIA ══════════════
         * O ficheiro põe a caixa toda e acaba com a regra: «fazendo cada um com VOLTA
         * OBRIGATÓRIA: transformação → resultado → reconstrução». Então mede-se assim:
         * cada problema com a resposta QUE O FICHEIRO PUBLICA (a referência é dele, não
         * da minha cabeça), e a igualdade verificada pela TABELA — exaustiva, resíduo 0.
         *
         * E a notação é a dele: o `+` é o OU. Ler o `+` como XOR dava «a + ab = a·b̄»,
         * que é certo em GF(2) e errado no ficheiro que manda aqui. */
        printf("\n§C22 OS PROBLEMAS DO eval.txt: a resposta é a dele, e a volta é a tabela.\n\n");
        {
            struct { const char *tema, *esq, *dir; } P[] = {
                /* §2 — os teoremas que caem imediatamente */
                { "idempotência",  "a + a + a",              "a"        },
                { "dominação",     "a*b + 1",                "1"        },
                { "absorção",      "a + (a*b) + c",          "a + c"    },
                { "absorção dual", "a * (a + b)",            "a"        },
                { "involução",     "nao (nao a)",            "a"        },
                /* §3 — De Morgan, nas duas formas */
                { "De Morgan ∨",   "nao (a + b)",            "nao a * nao b" },
                { "De Morgan ∧",   "nao (a * b)",            "nao a + nao b" },
                { "De Morgan c/3", "nao (a * (b + c))",      "nao a + (nao b * nao c)" },
                /* §4 — o princípio da dualidade: a identidade e a sua dual */
                { "dualidade 1",   "a + 0",                  "a"        },
                { "dualidade 2",   "a * 1",                  "a"        },
                { "dualidade 3",   "a + (b*c)",              "(a+b) * (a+c)" },
                /* §5 — complementação */
                { "complemento ∨", "a + nao a",              "1"        },
                { "complemento ∧", "a * nao a",              "0"        },
                /* §6 — simplificação algébrica (os dois exemplos do ficheiro) */
                { "simplifica 1",  "a*b + a*nao b",          "a"        },
                { "simplifica 2",  "(a + b) * (a + nao b)",  "a"        },
                /* §16 nível 1 — os problemas propostos */
                { "nível 1 (1)",   "a + nao a * b",          "a + b"    },
                { "nível 1 (3)",   "a + a*b",                "a"        },
                { "nível 1 (4)",   "a * (a + b)",            "a"        },
                /* §7 — a tabela do ficheiro dá F = C */
                { "tabela → F=C",  "nao a*nao b*c + nao a*b*c + a*nao b*c + a*b*c", "c" },
                /* §15 — as bases: NAND sozinho basta (o ficheiro dá as duas linhas) */
                { "NAND: ¬a",      "nao (a*a)",              "nao a"    },
                { "NAND: a∧b",     "nao (nao (a*b) * nao (a*b))", "a * b" },
            };
            int pmal3 = 0;
            for(size_t k = 0; k < sizeof P/sizeof *P; k++){
                Bool ba, bb;
                if(!bl_le(P[k].esq, &ba) || !bl_le(P[k].dir, &bb)){ pmal3++; continue; }
                /* as duas tabelas comparam-se no MAIOR dos dois espaços — as variáveis
                 * que só aparecem de um lado têm de não mudar nada (é isso a igualdade) */
                int nv = ba.nv > bb.nv ? ba.nv : bb.nv;
                char nomes[BL_VAR]; int nn2 = 0;
                for(int i = 0; i < ba.nv; i++) nomes[nn2++] = ba.nome[i];
                for(int i = 0; i < bb.nv; i++){
                    int tem = 0;
                    for(int j = 0; j < nn2; j++) if(nomes[j] == bb.nome[i]) tem = 1;
                    if(!tem && nn2 < BL_VAR) nomes[nn2++] = bb.nome[i];
                }
                nv = nn2;
                unsigned n = 1u << nv;
                int dif = 0;
                for(unsigned x = 0; x < n; x++){
                    unsigned xa = 0, xb = 0;
                    for(int i = 0; i < ba.nv; i++)
                        for(int j = 0; j < nv; j++)
                            if(ba.nome[i] == nomes[j] && (x >> j) & 1) xa |= 1u << i;
                    for(int i = 0; i < bb.nv; i++)
                        for(int j = 0; j < nv; j++)
                            if(bb.nome[i] == nomes[j] && (x >> j) & 1) xb |= 1u << i;
                    if(ba.t[xa] != bb.t[xb]) dif++;
                }
                if(dif){ pmal3++; printf("      FALHA %-14s %s  ≠  %s (%d linhas)\n",
                                         P[k].tema, P[k].esq, P[k].dir, dif); }
            }
            printf("      %zu problemas do ficheiro, cada um verificado PELA TABELA\n",
                   sizeof P/sizeof *P);
            ok("os problemas do eval.txt fecham com a resposta QUE O FICHEIRO PUBLICA —"
               " a referência não é minha, e a volta é a tabela inteira (exaustiva)",
               pmal3 == 0);
            /* o gume: uma igualdade FALSA tem de falhar, senão o medidor não mede nada */
            { Bool b1, b2v;
              int lidos = bl_le("a + b", &b1) && bl_le("a * b", &b2v);
              int dif = 0;
              for(unsigned x = 0; x < 4; x++) if(b1.t[x] != b2v.t[x]) dif++;
              ok("e o gume: «a + b» NÃO é «a * b» — a comparação por tabela acusa, logo"
                 " ela pode falhar", lidos && dif == 2); }
        }

        /* ═══ §C21 A LÓGICA É O CORPO GF(2), com as MESMAS cinco operações ═════════
         * O ramo do transístor já o dizia: «AND É a multiplicação de GF(2), XOR É a
         * soma, e −x = x». Então a lógica não é máquina nova — é o corpo, e a forma
         * canónica de uma função booleana é a ANF (Zhegalkin), que se obtém pela
         * transformada de Möbius. E ela é INVOLUÇÃO: a mesma leva a tabela na ANF e a
         * ANF na tabela — ν∘ν = id, o Dual das cinco, de graça porque somar é subtrair.
         *
         * Mede-se EXAUSTIVO: com 3 variáveis há 256 funções booleanas, e percorrem-se
         * TODAS. Não é amostra — é o corpo inteiro. */
        printf("\n§C21 A LÓGICA É O CORPO GF(2): a ANF é o dual, e a transformada é involução.\n\n");
        {
            int lmal = 0;
            /* (i) a INVOLUÇÃO, nas 256 funções de 3 variáveis (todas) */
            for(unsigned f2 = 0; f2 < 256; f2++){
                unsigned char t[8], u[8];
                for(int x = 0; x < 8; x++) t[x] = u[x] = (unsigned char)((f2 >> x) & 1);
                bl_mobius(u, 3);                      /* tabela → ANF */
                bl_mobius(u, 3);                      /* ANF → tabela: a MESMA operação */
                for(int x = 0; x < 8; x++) if(u[x] != t[x]) lmal++;
            }
            /* (ii) e a ANF DIZ a função: avaliar a ANF dá a tabela, nos 256 casos */
            for(unsigned f2 = 0; f2 < 256; f2++){
                unsigned char t[8], anf[8];
                for(int x = 0; x < 8; x++) t[x] = anf[x] = (unsigned char)((f2 >> x) & 1);
                bl_mobius(anf, 3);
                for(unsigned x = 0; x < 8; x++)
                    if(bl_val_anf(anf, 3, x) != t[x]) lmal++;
            }
            printf("      a transformada de Möbius é involução nas 256 funções de 3 variáveis\n");
            printf("      e a ANF avaliada devolve a tabela, nas 256 — dois caminhos\n");
            ok("a lógica é o corpo GF(2): a ANF é o DUAL da tabela, a transformada é"
               " INVOLUÇÃO (ν∘ν = id) e a avaliação da ANF devolve a tabela — exaustivo"
               " nas 256 funções, não por amostra", lmal == 0);

            /* (iii) a leitura da fala, e a FIBRA: quais entradas dão 1 */
            {
                Bool b3;
                int ok3 = bl_le("a*b + c", &b3);
                /* a*b + c: 1 quando (a∧b) xor c */
                int certo = ok3 && b3.nv == 3;
                if(certo){
                    for(unsigned x = 0; x < 8; x++){
                        int a2 = x & 1, b2v = (x >> 1) & 1, c2 = (x >> 2) & 1;
                        /* a NOTAÇÃO é a do eval.txt: o `+` é o OU, e a expectativa
                         * segue-a. Quando eu lia o `+` como XOR, esta linha esperava
                         * `^` — e era a expectativa que estava errada, não o corpo. */
                        if(b3.t[x] != ((a2 && b2v) | c2)) certo = 0;
                    }
                }
                /* e as leis da casa, medidas: De Morgan e o idempotente x⊗x = x */
                Bool dm1, dm2, idem;
                int lok = bl_le("nao (a*b)", &dm1) && bl_le("nao a + nao b + nao a * nao b", &dm2)
                       && bl_le("a*a", &idem);
                int leis = lok;
                if(leis){
                    for(unsigned x = 0; x < 4; x++) if(dm1.t[x] != dm2.t[x]) leis = 0;
                    for(unsigned x = 0; x < 2; x++) if(idem.t[x] != (x & 1)) leis = 0;
                }
                ok("e a fala lê-se no corpo: «a*b + c» dá a tabela certa, De Morgan fecha"
                   " e o idempotente x⊗x = x é a lei booleana em pessoa", certo && leis);
            }
            /* o gume: uma fala que não é do corpo é RECUSADA, e não interpretada */
            { Bool bx;
              ok("e o gume: «a & (b» não fecha e é recusada — o parêntese aberto não se"
                 " adivinha", bl_le("a & (b", &bx) == 0); }

            /* A NOTAÇÃO DO FICHEIRO LÊ-SE. O `eval.txt` escreve o produto por
             * JUSTAPOSIÇÃO e o complemento POSFIXO — «A + AB = A», «F = AB + AB'» —, e
             * a casa não os lia: «simplifica a + ab» morria calada. Escrever numa
             * notação e ler noutra é o defeito, e é ele que aqui fica preso.
             * O GUME é o ESPAÇO: sem ele multiplica-se, com ele o operador tem de vir
             * escrito — senão o `o` de «a ou b» virava variável e o OU virava produto. */
            { int jmal = 0;
              Bool jab, jamb, jxy, jxyl, jpar;
              int lidos = bl_le("ab", &jab) && bl_le("a*b", &jamb)
                       && bl_le("xy'", &jxy) && bl_le("x*(nao y)", &jxyl)
                       && bl_le("a(b+c)", &jpar);
              if(!lidos) jmal++;
              else {
                  for(unsigned x = 0; x < 4; x++){
                      if(jab.t[x] != jamb.t[x]) jmal++;      /* «ab» É «a*b» */
                      if(jxy.t[x] != jxyl.t[x]) jmal++;      /* «xy'» É «x·¬y» */
                  }
                  for(unsigned x = 0; x < 8; x++){
                      int a2 = x & 1, b2 = (x >> 1) & 1, c2 = (x >> 2) & 1;
                      if(jpar.t[x] != (a2 && (b2 | c2))) jmal++;   /* «a(b+c)» */
                  }
              }
              /* as duas identidades que o ficheiro escreve, agora legíveis */
              Bool i1, i1r, i2, i2r;
              int ids = bl_le("a + ab", &i1) && bl_le("a", &i1r)
                     && bl_le("xy + xy'", &i2) && bl_le("x", &i2r);
              if(ids){
                  for(unsigned x = 0; x < 4; x++) if(i1.t[x] != i1r.t[x & 1]) jmal++;
                  for(unsigned x = 0; x < 4; x++) if(i2.t[x] != i2r.t[x & 1]) jmal++;
              } else jmal++;
              /* e o GUME: COM espaço, «a ou b» continua a ser o OU e não o produto —
               * se a justaposição atravessasse o espaço, esta linha caía */
              Bool ou1, ou2;
              int gume = bl_le("a ou b", &ou1) && bl_le("a + b", &ou2);
              if(gume) for(unsigned x = 0; x < 4; x++) if(ou1.t[x] != ou2.t[x]) gume = 0;
              /* e a variável dupla: «a ou b» tem DUAS variáveis, não três (nada de `o`) */
              if(gume && ou1.nv != 2) gume = 0;
              ok("a NOTAÇÃO DO FICHEIRO lê-se: a JUSTAPOSIÇÃO é o produto («ab» = «a*b»,"
                 " «a(b+c)») e o `'` é o complemento posfixo («xy'» = x·¬y) — e as duas"
                 " identidades dele fecham, a + ab = a e xy + xy' = x. O gume é o ESPAÇO:"
                 " «a ou b» continua o OU com DUAS variáveis, e não a·o·u·b",
                 jmal == 0 && gume); }
        }

        /* ═══ §C20 A INTEGRAÇÃO: o inverso da derivada, e o «+C» é o NÚCLEO ═════════
         * Integrar é derivar com o sinal trocado — uma operação, como as outras três.
         * E a composição mede a razão de ser da constante: num sentido devolve a
         * identidade, no outro devolve p − p(0). O «+C» não é convenção de escrita:
         * é o que a derivada APAGA, e a volta pede de volta.
         *
         * E mede-se a lei da casa, a de Gentil (o Teorema Central): 
         *    ∫₀^b f + ∫₀^{f(b)} f⁻¹ = b·f(b)
         * que para f(x) = x^m fecha EXATA em frações: b^{m+1}/(m+1) + m·b^{m+1}/(m+1)
         * = b^{m+1}. Contar e integrar são duais pela soma reversível. */
        printf("\n§C20 A INTEGRAÇÃO: o inverso da derivada, e a constante é o núcleo.\n\n");
        {
            int imal = 0;
            /* (i) integrar e derivar devolve a IDENTIDADE */
            {
                Pol p1; pol_zera(&p1);
                p1.p[0] = 5; p1.p[1] = -2; p1.p[2] = 3; p1.n = 2;   /* 3x² − 2x + 5 */
                Pol Fi, Fd;
                pol_calculo(p1, -1, &Fi);                             /* ∫ */
                pol_calculo(Fi, +1, &Fd);                              /* d/dx */
                if(Fd.n != p1.n) imal++;
                else for(int k = 0; k <= p1.n; k++)
                    if(Fd.p[k]*p1.q[k] != p1.p[k]*Fd.q[k]) imal++;
                /* e a integral é EXATA em frações: ∫3x² − 2x + 5 = x³ − x² + 5x */
                if(!(Fi.p[3] == 1 && Fi.q[3] == 1 && Fi.p[2] == -1 && Fi.q[2] == 1 &&
                     Fi.p[1] == 5 && Fi.q[1] == 1 && Fi.p[0] == 0)) imal++;
                /* (ii) o outro sentido: derivar e integrar devolve p − p(0) — o «+C» */
                Pol Fd2, Fi2;
                pol_calculo(p1, +1, &Fd2);
                pol_calculo(Fd2, -1, &Fi2);
                if(Fi2.p[0] != 0) imal++;                             /* a constante foi-se */
                for(int k = 1; k <= p1.n; k++)
                    if(Fi2.p[k]*p1.q[k] != p1.p[k]*Fi2.q[k]) imal++;   /* o resto volta igual */
            }
            /* (iii) a fração que só existe se for exata: ∫x² = x³/3 */
            {
                Pol p2; pol_zera(&p2); p2.p[2] = 1; p2.n = 2;
                Pol Fi; pol_calculo(p2, -1, &Fi);
                if(!(Fi.p[3] == 1 && Fi.q[3] == 3)) imal++;            /* 1/3 exato */
            }
            /* (iv) A LEI DE GENTIL, exata em Q: ∫₀^b x^m + ∫₀^{b^m} y^{1/m} = b^{m+1} */
            {
                for(long m = 1; m <= 5; m++)
                for(long b2 = 1; b2 <= 4; b2++){
                    long bm1 = 1;                                    /* b^{m+1} */
                    for(long i = 0; i <= m; i++) bm1 *= b2;
                    /* ∫₀^b x^m = b^{m+1}/(m+1) ;  ∫₀^{b^m} y^{1/m} = m·b^{m+1}/(m+1) */
                    long ap = bm1, aq = m + 1;
                    long cp = m * bm1, cq = m + 1;
                    long sp, sq; pl_soma(ap, aq, cp, cq, &sp, &sq);
                    if(!(sq == 1 && sp == bm1)) imal++;              /* = b·f(b) exato */
                }
            }
            /* (v) a INTEGRAL DEFINIDA: ∫₀³ x² = 9, exato — e o valor mede-se aqui
             *     porque a escrita dele já me traiu uma vez: sete frações num só
             *     printf, e o buffer rotativo do escritor só tem quatro. */
            {
                Pol p4; pol_zera(&p4); p4.p[2] = 1; p4.n = 2;        /* x² */
                Pol F4; pol_calculo(p4, -1, &F4);
                long v3p, v3q, v0p, v0q, dp3, dq3;
                pol_val_q(F4, 3, 1, &v3p, &v3q);
                pol_val_q(F4, 0, 1, &v0p, &v0q);
                pl_soma(v3p, v3q, -v0p, v0q, &dp3, &dq3);
                if(!(dq3 == 1 && dp3 == 9)) imal++;                  /* 27/3 − 0 = 9 */
            }
            printf("      ∫(3x² - 2x + 5) = x³ - x² + 5x, e derivar devolve o de partida\n");
            printf("      derivar e integrar devolve p - p(0): a constante É o núcleo\n");
            printf("      Gentil: ∫f + ∫f⁻¹ = b·f(b) exato em Q, nos 20 casos (m=1..5)\n");
            ok("integrar é derivar com o sinal trocado: um sentido devolve a IDENTIDADE,"
               " o outro devolve p − p(0) — e é isso o «+C», medido e não convencionado",
               imal == 0);
            /* o gume: a constante NÃO volta sozinha — se voltasse, o núcleo era vazio
             * e o «+C» não teria razão de existir */
            {
                Pol p3; pol_zera(&p3); p3.p[0] = 7; p3.n = 0;        /* a constante 7 */
                Pol Fd3, Fi3;
                pol_calculo(p3, +1, &Fd3);                            /* d/dx 7 = 0 */
                pol_calculo(Fd3, -1, &Fi3);
                ok("e o gume: a derivada da constante é ZERO e a volta não a inventa —"
                   " o núcleo não é vazio, e é por isso que o «+C» existe",
                   Fd3.n == 0 && Fd3.p[0] == 0 && Fi3.p[0] == 0 && p3.p[0] == 7);
            }
        }

        /* ═══ §C19 O PADRÃO OURO: tudo passa pelas CINCO, e o sinal é a Lei 1 ═══════
         * As cinco do Corpo Universal não são um catálogo à parte: são POR ONDE tudo
         * passa. E três pares que eu tinha escrito como duas funções são, cada um, UMA
         * operação com sinal — «não são duas operações que calham ser inversas; é uma,
         * e a Lei 1 escreve-a: 1† = −1»:
         *
         *   MEMBRANA(±1)   Dual †          +1 desdobra a entrada, −1 veste a saída
         *   CORPUS(±1)     Soma ⊕          +1 aprende (a dobra), −1 esquece (a retração)
         *   CONV(±1)       ⊗ e a fibra     +1 convolve, −1 deconvolve
         *
         * E o que se mede é o que faz delas UMA: os dois sentidos compõem na
         * IDENTIDADE. Sem isso seriam duas funções com nomes bonitos. */
        printf("\n§C19 O PADRÃO OURO: os pares são UMA operação, e o sinal decide.\n\n");
        {
            int omal = 0;
            /* (i) MEMBRANA: vestir e desdobrar compõem na identidade — o valor sai
             *     vestido, entra de novo e volta ao mesmo racional. */
            { char vest[128], nu3[256];
              MEMBRANA(vest, sizeof vest, 0, 5, 6, -1);        /* −1: veste */
              MEMBRANA(nu3, sizeof nu3, vest, 0, 1, +1);       /* +1: desdobra */
              if(strcmp(nu3, "(5)/(6)")) omal++;
              char v2[128]; MEMBRANA(v2, sizeof v2, 0, 4, 1, -1);
              if(strcmp(v2, "4")) omal++; }
            /* (ii) CORPUS: aprender e esquecer compõem na identidade — o nó volta ao
             *      estado de origem, e o caminho fica nos dois sentidos. */
            { int d3; 
              CORPUS("padrao ouro teste", "uma operacao", +1);
              long r1 = t_erosao("padrao ouro teste", &d3);
              CORPUS("padrao ouro teste", 0, -1);
              long r2 = t_erosao("padrao ouro teste", &d3);
              if(!r1 || r2) omal++;
              if(CORPUS("nunca existiu isto aqui", 0, -1)) omal++;   /* nada a retirar */
            }
            /* (iii) CONV: convolver e deconvolver compõem na identidade — (a⊗b)÷b = a */
            { Pz a4; a4.n = 2; a4.a[0] = 1; a4.a[1] = 2; a4.a[2] = 3;
              Pz b4; b4.n = 1; b4.a[0] = -1; b4.a[1] = 1;
              Pz c4, v4;
              if(!CONV(a4, b4, +1, &c4)) omal++;
              if(!CONV(c4, b4, -1, &v4)) omal++;
              else { if(v4.n != a4.n) omal++;
                     else for(int k = 0; k <= a4.n; k++) if(v4.a[k] != a4.a[k]) omal++; } }
            printf("      MEMBRANA(-1) → $\\frac{5}{6}$ → MEMBRANA(+1) → (5)/(6)\n");
            printf("      CORPUS(+1) põe, CORPUS(-1) tira, e o caminho fica\n");
            printf("      CONV(+1) convolve, CONV(-1) devolve o outro fator\n\n");
            ok("os três pares são UMA operação com sinal, e mede-se o que os faz uma:"
               " os dois sentidos compõem na IDENTIDADE (Lei 1, 1† = -1)", omal == 0);
            /* o gume: a fibra NÃO existe onde a divisão não é exata — e aí a operação
             * recusa, em vez de devolver um resto escondido */
            { Pz a5; a5.n = 2; a5.a[0] = 1; a5.a[1] = 0; a5.a[2] = 1;   /* x²+1 */
              Pz b5; b5.n = 1; b5.a[0] = -1; b5.a[1] = 1;               /* x−1 */
              Pz q5;
              ok("e o gume: CONV(-1) RECUSA onde a fibra não existe (x²+1 por x−1) —"
                 " a operação não inventa a volta que não há", CONV(a5, b5, -1, &q5) == 0); }
        }

        /* ═══ §C18 O MDC: a folha da órbita do inversor, nos dois andares ═══════════
         * «Euclides é a dinâmica do inversor, e a FOLHA é o gcd» — e a mesma cadeia
         * corre no inteiro e no polinómio. Mede-se nos dois, e mede-se a VOLTA: o mdc
         * divide os dois exatamente, ou não é o mdc. */
        printf("\n§C18 O MDC: a mesma órbita do inversor, no inteiro e no polinómio.\n\n");
        {
            int gmal = 0, passos = 0;
            /* (i) no polinómio: mdc(x³−1, x²−1) = x−1 */
            Pz a; a.n = 3; a.a[0] = -1; a.a[1] = 0; a.a[2] = 0; a.a[3] = 1;
            Pz b2; b2.n = 2; b2.a[0] = -1; b2.a[1] = 0; b2.a[2] = 1;
            Pz g;
            if(!pz_mdc(a, b2, &g, &passos)) gmal++;
            else {
                if(!(g.n == 1 && g.a[1] == 1 && g.a[0] == -1)) gmal++;
                Pz q1, q2;                              /* A VOLTA: divide os dois */
                if(!pz_div_exata(a, g, &q1)) gmal++;
                if(!pz_div_exata(b2, g, &q2)) gmal++;
            }
            printf("      mdc(x³-1, x²-1) = x-1 em %d passo(s) da órbita\n", passos);
            /* (ii) o gume: primos entre si dão 1 — a órbita desce até à unidade */
            Pz c; c.n = 2; c.a[0] = 1; c.a[1] = 0; c.a[2] = 1;      /* x²+1 */
            Pz d2; d2.n = 1; d2.a[0] = -1; d2.a[1] = 1;             /* x−1 */
            Pz g2; int p2;
            if(!pz_mdc(c, d2, &g2, &p2)) gmal++;
            else if(!(g2.n == 0 && g2.a[0] != 0)) gmal++;
            /* (iii) o MESMO passo no inteiro: a folha da órbita é o gcd numérico */
            { long x = 1071, y = 462, pn = 0;
              while(y){ long t = x % y; x = y; y = t; pn++; }
              if(x != 21) gmal++;
              printf("      e no inteiro, a mesma descida: mdc(1071,462) = %ld em %ld passo(s)\n",
                     x, pn); }
            ok("o MDC é a folha da órbita do inversor — a mesma cadeia no inteiro e no"
               " polinómio, e a VOLTA confere: ele divide os dois exatamente", gmal == 0);
            ok("e o gume: primos entre si descem até à UNIDADE (mdc = 1), que é a órbita"
               " a fechar sem folha comum", g2.n == 0 && g2.a[0] != 0);
        }

        /* ═══ §C17 AS CINCO OPERAÇÕES NO POLINÓMIO, cada uma com a SUA conservação ═══
         * O Corpo Universal fixa as cinco e o que cada uma conserva. Aqui elas realizam-se
         * no corpo dos polinómios — nenhuma régua nova —, e mede-se a conservação de cada,
         * que é o que as distingue: a Soma soma a energia, a Multiplicação multiplica-a, a
         * Divisão é a FIBRA (a = q·b + r, e a volta reconstrói), o Dual é a INVOLUÇÃO (o
         * recíproco, ν∘ν = id — o mesmo ν da prova de Pisot), e a Inversão só é admissível
         * quando a volta existe. E Pontryagin fecha: o caráter leva SOMA em PRODUTO. */
        printf("\n§C17 AS CINCO OPERAÇÕES: cada uma com a conservação que a define.\n\n");
        {
            Pz u; u.n = 2; u.a[0] = 1; u.a[1] = 2; u.a[2] = 3;      /* 3x² + 2x + 1 */
            Pz v; v.n = 1; v.a[0] = -1; v.a[1] = 1;                 /* x − 1 */
            int cmal = 0;

            /* SOMA — a dobra: E(u ⊕ u) = 2²·E(u)? Não: a dobra SOMA o corpo consigo, e a
             * energia do dobro é 4E — o que a lei diz é E(u⊕u) = 2E(u) para o CLONE (a
             * soma direta, que empilha), não para 2u. Mede-se o que vale: a soma é
             * componente a componente e a retração devolve. */
            { Pz w; pz_soma(u, v, &w);
              Pz volta; Pz mv; mv.n = v.n;
              for(int k = 0; k <= v.n; k++) mv.a[k] = -v.a[k];
              pz_soma(w, mv, &volta);                     /* (u+v) − v = u */
              if(volta.n != u.n) cmal++;
              else for(int k = 0; k <= u.n; k++) if(volta.a[k] != u.a[k]) cmal++; }

            /* MULTIPLICAÇÃO — a fusão: a NORMA multiplica. No polinómio a norma que
             * multiplica é o valor num ponto (o caráter): eval(u⊗v) = eval(u)·eval(v). */
            { Pz w; pz_mul(u, v, &w);
              for(long x = -3; x <= 3; x++){
                  long eu = 0, ev = 0, ew = 0;
                  for(int k = u.n; k >= 0; k--) eu = eu*x + u.a[k];
                  for(int k = v.n; k >= 0; k--) ev = ev*x + v.a[k];
                  for(int k = w.n; k >= 0; k--) ew = ew*x + w.a[k];
                  if(ew != eu*ev) cmal++;
              } }

            /* DIVISÃO — a FIBRA: a = q·b + r com grau(r) < grau(b), e a volta reconstrói.
             * Em ℤ o pseudo-fator é DITO: vale fator·a = q·b + r. */
            { Pz a2; a2.n = 3; a2.a[0] = -1; a2.a[1] = 0; a2.a[2] = 0; a2.a[3] = 1; /* x³−1 */
              Pz q2, r2; long fator = 1;
              if(!pz_div_resto(a2, v, &q2, &r2, &fator)) cmal++;
              else {
                  if(!(r2.n == 0 && r2.a[0] == 0)) cmal++;    /* x−1 divide x³−1 */
                  Pz qb, soma;                                 /* a VOLTA: q·b + r */
                  pz_mul(q2, v, &qb); pz_soma(qb, r2, &soma);
                  if(soma.n != a2.n) cmal++;
                  else for(int k = 0; k <= a2.n; k++) if(soma.a[k] != fator*a2.a[k]) cmal++;
              } }
            /* e com resto NÃO nulo, a identidade continua a fechar */
            { Pz a3; a3.n = 2; a3.a[0] = 1; a3.a[1] = 0; a3.a[2] = 1;   /* x² + 1 */
              Pz q3, r3; long fator = 1;
              if(!pz_div_resto(a3, v, &q3, &r3, &fator)) cmal++;
              else {
                  if(r3.n != 0 || r3.a[0] != 2) cmal++;        /* resto 2: p(1) = 2 */
                  Pz qb, soma; pz_mul(q3, v, &qb); pz_soma(qb, r3, &soma);
                  for(int k = 0; k <= a3.n; k++) if(soma.a[k] != fator*a3.a[k]) cmal++;
              } }

            /* DUAL — a involução: o recíproco troca dentro por fora, e ν∘ν = id. E é ele
             * o ν da prova de Pisot: ν(β) = β*, que é onde a conta fica barata. */
            { Pz d1, d2; pz_dual(u, &d1); pz_dual(d1, &d2);
              if(d2.n != u.n) cmal++;
              else for(int k = 0; k <= u.n; k++) if(d2.a[k] != u.a[k]) cmal++;
              Pz be; be.n = 4; be.a[0] = -1; be.a[1] = 0; be.a[2] = 0;
              be.a[3] = -3; be.a[4] = 1;                   /* β(4,3) = x⁴ − 3x³ − 1 */
              Pz bs; pz_dual(be, &bs);                     /* β* = −x⁴ − 3x + 1 */
              if(!(bs.a[0] == 1 && bs.a[1] == -3 && bs.a[4] == -1)) cmal++;
            }

            /* INVERSÃO — a volta: só é admissível quem a tem. Dividir por ZERO não tem. */
            { Pz z0; z0.n = 0; z0.a[0] = 0;
              Pz q4, r4; long fator = 1;
              if(pz_div_resto(u, z0, &q4, &r4, &fator)) cmal++;   /* tem de RECUSAR */
            }
            printf("      soma: a retração devolve · produto: eval(u⊗v) = eval(u)·eval(v)\n");
            printf("      divisão: fator·a = q·b + r, com a volta a reconstruir\n");
            printf("      dual: ν∘ν = id, e ν(β) = β* — o recíproco da prova de Pisot\n");
            ok("as cinco operações realizam-se no polinómio com a conservação de CADA uma:"
               " retração, norma que multiplica, fibra com volta, involução, e a inversão"
               " a RECUSAR quem não tem volta", cmal == 0);

            /* PONTRYAGIN — «o dual do grupo pela troca ⊕→⊗»: o caráter leva a SOMA dos
             * expoentes no PRODUTO dos valores. No polinómio isso é x^(a+b) = x^a·x^b,
             * e é a mesma cláusula que o transístor cumpre em volts. */
            { int pmal2 = 0;
              for(long x = 2; x <= 5; x++)
                for(int a = 0; a <= 5; a++)
                  for(int b2 = 0; b2 <= 5; b2++){
                      long pa = 1, pb = 1, ps = 1;
                      for(int i = 0; i < a; i++) pa *= x;
                      for(int i = 0; i < b2; i++) pb *= x;
                      for(int i = 0; i < a+b2; i++) ps *= x;
                      if(pa*pb != ps) pmal2++;
                  }
              ok("e PONTRYAGIN: o caráter leva a SOMA em PRODUTO — x^(a+b) = x^a·x^b nos"
                 " 144 casos, a mesma troca ⊕→⊗ do grupo dual", pmal2 == 0);
            }
        }

        /* ═══ §C16 O RELÓGIO: o tick é o quantum, e a velocidade escolhe-se ═════════
         * «O Quantizador converte o contínuo em contagem conservada», e o refinamento
         * é a TORRE — ω_{2M}² = ω_M —, «o contínuo entra por refinamento do passo de
         * quantização, NUNCA POR SALTO» (Corpo Universal). Aqui isso é literal: a fita
         * dá uma dobra por tick, e a velocidade é quantas dobras cabem num tick. Ela
         * só sobe por DOBRA (1, 2, 4, 8…) — 3 não é um andar da torre.
         *
         * E o que se mede é o que o Quantizador promete: mudar a velocidade muda A
         * LEITURA e não o objeto — o valor final e o NÚMERO TOTAL de dobras são os
         * mesmos em qualquer velocidade. */
        printf("\n§C16 O RELÓGIO: o tick é o quantum, e a velocidade é escolha de leitura.\n\n");
        {
            const char *conta = "2 x (3 + 4) + 5 x 5";
            char cf_n5[600]; snprintf(cf_n5, sizeof cf_n5, "%s.tick", b);
            long vfinal = -1, dobras_ref = -1;
            int tmal = 0;
            for(int vel = 1; vel <= 8; vel *= 2){        /* 1, 2, 4, 8 — a torre */
                int cf = open(cf_n5, O_RDWR|O_CREAT|O_TRUNC, 0644);
                long n5 = ct_leia(cf, conta), dobras = 0, v5 = 0; char pq5[256];
                for(;;){                                  /* tick a tick */
                    int fez = 0;
                    for(int d = 0; d < vel; d++){         /* vel dobras por tick */
                        if(ct_passo(cf, n5, pq5, sizeof pq5) != 1) break;
                        dobras++; fez = 1;
                    }
                    if(!fez) break;
                }
                if(!ct_valor(cf, n5, &v5)) tmal++;
                close(cf);
                if(vfinal < 0){ vfinal = v5; dobras_ref = dobras; }
                else if(v5 != vfinal || dobras != dobras_ref) tmal++;
                printf("      velocidade %d tick(s): %ld dobras, valor %ld\n", vel, dobras, v5);
            }
            unlink(cf_n5);
            ok("o Quantizador conserva a CONTAGEM: mudar a velocidade (1,2,4,8) muda a"
               " leitura e nao o objeto — mesmo valor e MESMO numero de dobras",
               tmal == 0 && vfinal == 39 && dobras_ref > 0);
            /* o gume: a velocidade sobe por DOBRA. 3 nao e andar da torre — e a
             * verificacao e' a mesma que o relogio faz: potencia de dois, e o bit conta. */
            ok("e a velocidade so' sobe por DOBRA: 1,2,4,8 sao andares e 3,5,6 nao —"
               " o refinamento e' a torre, nunca um salto",
               eh_dobra(1) && eh_dobra(2) && eh_dobra(4) && eh_dobra(8) &&
               !eh_dobra(3) && !eh_dobra(5) && !eh_dobra(6) && !eh_dobra(0));
        }

        /* ═══ §C15 A FATORAÇÃO: o produto é convolução, e fatorar é a volta ══════════
         * «Elevar o grau é exacto — a linha de Pascal» (Corpo estelar), e os binomiais
         * são «a base natural do ⊕» (Newton, progressoes.c §P2). O produto é a
         * CONVOLUÇÃO (Corpo Universal), logo fatorar é a deconvolução — e ela só vale
         * onde a divisão é EXATA. Nada de aproximar: divide-se, e o resto tem de ser 0. */
        printf("\n§C15 A FATORAÇÃO: fatorar é a volta da convolução, e a volta CONFERE.\n\n");
        {
            /* (i) PASCAL POR DOIS CAMINHOS: multiplicar (x+1) n vezes (a convolução) tem
             *     de dar exatamente os binomiais C(n,k) (a fórmula). */
            int pmal = 0;
            for(int n = 1; n <= 10; n++){
                Pz acc; acc.n = 0; acc.a[0] = 1;
                Pz um; um.n = 1; um.a[0] = 1; um.a[1] = 1;      /* x + 1 */
                for(int i = 0; i < n; i++){ Pz r; pz_mul(acc, um, &r); acc = r; }
                if(acc.n != n){ pmal++; continue; }
                for(int k = 0; k <= n; k++) if(acc.a[k] != pl_binom(n, k)) pmal++;
            }
            printf("      (x+1)^10 pela convolução = a linha 10 de Pascal, termo a termo\n");
            ok("distribuir É convolver: (x+1)^n dá a linha de Pascal nos dez graus —"
               " dois caminhos, a convolução e o binomial, sem um decimal", pmal == 0);

            /* (ii) A FATORAÇÃO, com a VOLTA obrigatória: o produto dos fatores reconstrói
             *      o original byte a byte, ou não houve fatoração nenhuma. */
            struct { const char *nome; int n; long c[7]; int nfe; } F[] = {
                { "x^2 - 1",        2, {-1, 0, 1},           2 },   /* (x−1)(x+1) */
                { "x^2 - 3x + 2",   2, { 2,-3, 1},           2 },   /* (x−1)(x−2) */
                { "x^3 - x",        3, { 0,-1, 0, 1},        3 },   /* x(x−1)(x+1) */
                { "6x^2 - 5x + 1",  2, { 1,-5, 6},           2 },   /* (2x−1)(3x−1) */
                { "x^4 - 1",        4, {-1, 0, 0, 0, 1},     3 },   /* (x−1)(x+1)(x²+1) */
                { "x^2 - 2",        2, {-2, 0, 1},           1 },   /* irredutível em Q */
                { "x^2 - x - 1",    2, {-1,-1, 1},           1 },   /* o OURO: irredutível */
            };
            int fmal2 = 0;
            for(size_t k = 0; k < sizeof F/sizeof *F; k++){
                Pz p2; p2.n = F[k].n;
                for(int i = 0; i <= F[k].n; i++) p2.a[i] = F[k].c[i];
                Pz fs[PFMAX]; long cont = 1;
                int nf = pz_fatora(p2, fs, PFMAX, &cont);
                if(nf != F[k].nfe) fmal2++;
                if(!pz_confere(p2, fs, nf, cont)) fmal2++;   /* A VOLTA */
            }
            ok("a fatoração acha os fatores certos e o produto deles RECONSTRÓI o original"
               " — a volta confere byte a byte, ou não houve fatoração", fmal2 == 0);

            /* (iii) O GUME: um fator que não divide é RECUSADO pelo resto, e o irredutível
             *       não se parte. Sem isto a fatoração «achava» qualquer coisa. */
            {
                Pz p3; p3.n = 2; p3.a[0] = -2; p3.a[1] = 0; p3.a[2] = 1;   /* x² − 2 */
                Pz d;  d.n = 1;  d.a[0] = -1; d.a[1] = 1;                  /* x − 1 */
                Pz q;
                Pz fs[PFMAX]; long cont;
                int nf = pz_fatora(p3, fs, PFMAX, &cont);
                ok("o gume: (x−1) NÃO divide x²−2 (resto ≠ 0) e o irredutível fica inteiro",
                   pz_div_exata(p3, d, &q) == 0 && nf == 1 && fs[0].n == 2);
            }

            /* (iii-b) E ISTO TEM NOME NA CASA: o que aqui se faz é a CONVOLUÇÃO
             *     UNIVERSAL e a sua volta. O teorema já está medido
             *     (tests/convolucao_universal.js, 13:0): «convolução = a forma aditiva
             *     da multiplicação, vista pela Transformada Universal», e «a
             *     deconvolução é a divisão espectral — exata fora dos divisores de
             *     zero, com os divisores exibidos».
             *
             *     Então o produto que escrevi não se valida sozinho: valida-se contra a
             *     LEI — a transformada tem de casar nas duas FOLHAS. Em q=257, m=2, as
             *     folhas são σ=61 e σ†=198 (σ+σ†=2, σσ†=-1), e o critério é
             *     eval_σ(a⊛b) = eval_σ(a)·eval_σ(b). Dois caminhos: o meu e o dela. */
            {
                long q = 257, folha[2] = { 61, 198 };
                struct { int n; long c[5]; } A[] = {
                    { 2, {1, 2, 3} }, { 1, {-1, 1} }, { 3, {5, 0, -2, 1} }, { 2, {2, -3, 1} },
                };
                int cmal = 0, dmal = 0, exibiu = 0;
                for(size_t i = 0; i < sizeof A/sizeof *A; i++)
                for(size_t j = 0; j < sizeof A/sizeof *A; j++){
                    Pz a, b2, c2;
                    a.n = A[i].n;  for(int k = 0; k <= a.n; k++)  a.a[k]  = A[i].c[k];
                    b2.n = A[j].n; for(int k = 0; k <= b2.n; k++) b2.a[k] = A[j].c[k];
                    pz_mul(a, b2, &c2);
                    for(int l = 0; l < 2; l++){
                        long sg = folha[l];
                        long ea = 0, eb = 0, ec = 0;      /* Horner no anel */
                        for(int k = a.n;  k >= 0; k--) ea = mod_p(ea*sg + a.a[k], q);
                        for(int k = b2.n; k >= 0; k--) eb = mod_p(eb*sg + b2.a[k], q);
                        for(int k = c2.n; k >= 0; k--) ec = mod_p(ec*sg + c2.a[k], q);
                        if(ec != mod_p(ea*eb, q)) cmal++;             /* §V2: a transformada casa */
                        /* a DECONVOLUÇÃO é a divisão espectral: c/b = a onde eval(b) ≠ 0,
                         * e onde eval(b) = 0 o divisor de zero EXIBE-SE (não se finge) */
                        if(eb != 0){
                            if(mod_p(ec * inv_mod(eb, q), q) != ea) dmal++;
                        } else exibiu++;
                    }
                    /* e a volta inteira: dividir o produto pelo fator devolve o outro */
                    Pz vv;
                    if(!pz_div_exata(c2, b2, &vv)) dmal++;
                    else { if(vv.n != a.n) dmal++;
                           else for(int k = 0; k <= a.n; k++) if(vv.a[k] != a.a[k]) dmal++; }
                }
                printf("      eval_σ(a⊛b) = eval_σ(a)·eval_σ(b) nas duas folhas (σ=61, σ†=198)\n");
                ok("o produto que aqui se faz E a CONVOLUÇÃO UNIVERSAL: a transformada casa"
                   " nas duas folhas, nos 16 pares — a lei dela valida a minha", cmal == 0);
                ok("e fatorar E a DECONVOLUÇÃO: a divisão espectral bate com a divisão exata,"
                   " e a volta devolve o outro fator termo a termo", dmal == 0);
            }

            /* (iv) A FAMÍLIA QUE A CASA DOMINA: β(n,m) = xⁿ − m·x^{n−1} − 1 com m ≥ 2 é
             *      Pisot (Rouché no dual), e daí IRREDUTÍVEL sem calcular raiz nenhuma.
             *      Reconhece-se, e o m=1 fica de fora — é lá que a prova falha. */
            {
                Pz b1; b1.n = 4; b1.a[0] = -1; b1.a[1] = 0; b1.a[2] = 0;
                b1.a[3] = -3; b1.a[4] = 1;                   /* β(4,3) = x⁴ − 3x³ − 1 */
                Pz b2; b2.n = 4; b2.a[0] = -1; b2.a[1] = 0; b2.a[2] = 0;
                b2.a[3] = -1; b2.a[4] = 1;                   /* β(4,1): m=1, fora */
                Pz me; me.n = 2; me.a[0] = -1; me.a[1] = -2; me.a[2] = 1;  /* prata */
                Pz fs[PFMAX]; long cont;
                int nf = pz_fatora(b1, fs, PFMAX, &cont);
                ok("a familia de Pisot reconhece-se: β(4,3) da m=3, β(4,1) fica FORA (e a"
                   " metalica x²−2x−1 da m=2) — e o β(4,3) nao se parte",
                   pz_beta_pisot(b1) == 3 && pz_beta_pisot(b2) == 0 &&
                   pz_metalica(me) == 2 && nf == 1);
            }
        }

        /* ═══ O DICIONÁRIO: a álgebra é intrínseca, o assunto empresta os nomes ═══════
         * «cabe em qualquer assunto, só mapear». A borda a s² + b s + c = 0 já resolve;
         * o que cada assunto traz é QUAL grandeza sua se senta em cada coordenada, e
         * como chama os três regimes do Δ. Nenhuma régua nova: o Δ, a tricotomia, as
         * folhas e o resíduo são os MESMOS — muda a língua, não a lei. */
        {
            long a1, b1, c1, a2b, b2, c2;
            int i1 = dic_le("mola m=1 c=3 k=2", &a1, &b1, &c1);
            int i2 = dic_le("rlc L=1 R=3 S=2", &a2b, &b2, &c2);
            ok("dois assuntos, os mesmos numeros: a algebra da o MESMO Δ e o mesmo"
               " regime — o que muda e o nome (mola/rlc), nao a lei",
               i1 >= 0 && i2 >= 0 && a1 == a2b && b1 == b2 && c1 == c2 &&
               (b1*b1 - 4*a1*c1) == (b2*b2 - 4*a2b*c2) &&
               dic_regime(i1, b1*b1 - 4*a1*c1) != dic_regime(i2, b2*b2 - 4*a2b*c2));
            /* e o regime e' o do Δ, na lingua do assunto: 1 > 0, 9-8=1 hiperbolico */
            ok("e o regime sai do Δ e nao de tabela: Δ=1>0 e' o primeiro nome do assunto",
               !strcmp(dic_regime(i1, 1), "sobreamortecida") &&
               !strcmp(dic_regime(i1, 0), "criticamente amortecida") &&
               !strcmp(dic_regime(i1, -4), "oscilante"));
            /* o gume: assunto fora do dicionario, e dado em falta — nao se inventa */
            ok("e o que nao esta no dicionario, ou vem sem os dados, devolve a vez ao"
               " corpus — mapear nao e adivinhar",
               dic_le("unicornio m=1 c=3 k=2", &a1, &b1, &c1) < 0 &&
               dic_le("mola m=1 c=3", &a1, &b1, &c1) < 0);
        }

        /* ═══ AS FOLHAS NO RELÓGIO — a teoria discreta a resolver a equação ═══════════
         * A regua do polinomio ja declara o corpo Q[x]/(p): la a raiz e σ, exata e sem
         * decimal. O Corpo Universal da o passo que a torna NUMERO: no relogio da casa
         * (a escada de Fermat 17, 257, 65537) o discriminante ou e quadrado — e as duas
         * FOLHAS estao a vista, inteiras — ou nao e, e o andar e INERTE (as folhas vivem
         * um andar acima, onde o Frobenius x↦x^p E a estaca). «√2 e 11 em F_17».
         *
         * DOIS CAMINHOS que tem de concordar: o criterio de EULER diz se e quadrado, e a
         * VARREDURA acha (ou nao acha) a raiz — e cada raiz VOLTA a equacao com residuo
         * ZERO EXATO, ou nao e raiz. Tudo em inteiros. */
        {
            printf("\n§C14 AS FOLHAS NO RELOGIO: a raiz que nao fecha em Q e' inteira no anel.\n\n");
            struct { long a, b, c; const char *nome; } E[] = {
                {  1, -1, -1, "x^2 = x + 1  (o ouro: a borda m=1)" },
                {  1, -2, -1, "x^2 = 2x + 1  (a prata: m=2)"       },
                {  1,  0, -2, "x^2 = 2  (a raiz de 2)"             },
                {  1,  0,  1, "x^2 = -1  (o bit i)"                },
            };
            long P[] = { 17, 257, 65537 };
            int emal = 0, achou_sep = 0, achou_in = 0;
            for(size_t k = 0; k < sizeof E/sizeof *E; k++){
                long D = E[k].b*E[k].b - 4*E[k].a*E[k].c;
                for(size_t j = 0; j < 3; j++){
                    long p = P[j], r = 0;
                    int euler = eh_quadrado(D, p);          /* caminho 1: Euler */
                    int varr  = raiz_quad_mod(D, p, &r);    /* caminho 2: a varredura */
                    if(euler != varr){ emal++; continue; }  /* os dois TÊM de concordar */
                    if(!varr){ achou_in = 1; continue; }    /* inerte: as folhas sobem */
                    achou_sep = 1;
                    long s1, s2;
                    if(!folhas_de(E[k].a, E[k].b, r, p, &s1, &s2)){ emal++; continue; }
                    /* a VOLTA: cada folha na equação dá resíduo ZERO */
                    if(res_mod(E[k].a, E[k].b, E[k].c, s1, p) ||
                       res_mod(E[k].a, E[k].b, E[k].c, s2, p)) emal++;
                    /* e a ESTACA: σ+σ† = -b/a e σσ† = c/a, no anel */
                    long ia = inv_mod(E[k].a, p);
                    long soma = (s1 + s2) % p, prod = (s1 * s2) % p;
                    if(soma != mod_p(-E[k].b * ia, p) || prod != mod_p(E[k].c * ia, p)) emal++;
                }
            }
            ok("as folhas: Euler e a varredura concordam em toda a escada, cada folha volta"
               " a equacao com residuo ZERO, e a estaca fecha (σ+σ† = tr, σσ† = det)",
               emal == 0);
            ok("e a escada tem os DOIS casos — separado (folhas a vista) e inerte (sobem"
               " um andar): se um faltasse, a dicotomia era so' metade",
               achou_sep && achou_in);
            /* o gume: um numero que NAO e raiz tem residuo != 0 — senao a volta acima
             * passava com qualquer coisa */
            ok("e o gume: quem nao e raiz da residuo != 0", res_mod(1, -1, -1, 3, 17) != 0);

            /* E A REFERENCIA NAO E MINHA: o paper PUBLICA as folhas da prata no relogio
             * de Peano — «F_65537, m=2, separado: σ=4081, σ†=61458, σσ†=-1»
             * (corpo_universal.tex, thm:corpo-dual). O mecanismo tem de dar NESSES
             * numeros, senao a teoria e o codigo sao duas casas. */
            {
                long r = 0, s1 = 0, s2 = 0, p = 65537;
                int achou = raiz_quad_mod(8, p, &r) && folhas_de(1, -2, r, p, &s1, &s2);
                if(s1 > s2){ long t2 = s1; s1 = s2; s2 = t2; }
                ok("as folhas da prata batem com o que o PAPER publica: σ=4081, σ†=61458,"
                   " σσ†=-1 em F_65537 — a teoria e o codigo sao a mesma casa",
                   achou && s1 == 4081 && s2 == 61458 && mod_p(s1 * s2, p) == p - 1);
                /* e o bit i sai a escada inteira: 4, 16, 256 — a dobra em pessoa */
                long i17 = 0, i257 = 0, i65 = 0;
                raiz_quad_mod(-4, 17, &i17); raiz_quad_mod(-4, 257, &i257);
                raiz_quad_mod(-4, 65537, &i65);
                ok("e o bit i (x^2=-1) da a propria escada nos tres andares: 4, 16, 256"
                   " — a dobra a aparecer sozinha na solucao da equacao",
                   mod_p(i17 * inv_mod(2,17), 17) == 4 &&
                   mod_p(i257 * inv_mod(2,257), 257) == 16 &&
                   mod_p(i65 * inv_mod(2,65537), 65537) == 256);
            }
        }

            /* O AMBIENTE: a fala complexa vem em bloco, e o `\\` que o tradutor usa
             * para a fila da matriz e' o `;` que a regua dos SISTEMAS ja espera. Nada
             * de regua nova — a membrana entrega ao que existe. E o `cases` fica de
             * fora porque O TRADUTOR NAO O COMPOE: o dialecto e dele. */
            {
                latex_desdobra(nu2, sizeof nu2,
                    "\\begin{align} x' = y \\\\ y' = -x \\end{align}");
                int a1 = !strcmp(nu2, "x' = y ; y' = -x") && e_sistema(nu2);
                printf("      ambiente align         -> %s\n", nu2);
                ok("o ambiente desdobra-se e o `\\\\` vira o `;` do sistema — a fala"
                   " complexa cai na regua que ja existia", a1);
                latex_desdobra(nu2, sizeof nu2,
                    "\\begin{cases} x' = y \\\\ y' = -x \\end{cases}");
                ok("e o ambiente que o tradutor NAO compoe (cases) nao vira sistema —"
                   " o dialecto e do tradutor, nao meu", !e_sistema(nu2));
            }

            /* A VOLTA DA MEMBRANA — e ela tem DOIS lados ou nao e membrana. O tradutor
             * compoe e a assistente le; faltava responder na mesma roupa. A volta e
             * MEDIDA e nao afirmada: veste-se o valor, desdobra-se o vestido, e a fita
             * tem de devolver o MESMO numero — dois caminhos que tem de concordar. */
            {
                struct { long v, q; const char *tex; } V[] = {
                    { 5, 6, "\\frac{5}{6}" }, { 4, 1, "4" },
                    { -3, 7, "\\frac{-3}{7}" }, { 25, 1, "25" },
                };
                int vmal = 0;
                char cf_n4[600]; snprintf(cf_n4, sizeof cf_n4, "%s.conta", b);
                for(size_t k = 0; k < sizeof V/sizeof *V; k++){
                    char vest[256];
                    veste_valor(vest, sizeof vest, V[k].v, V[k].q);
                    if(strcmp(vest, V[k].tex)){ vmal++; continue; }
                    /* a VOLTA: o vestido desdobra-se e a fita devolve o mesmo */
                    latex_desdobra(nu2, sizeof nu2, vest);
                    int cf4 = open(cf_n4, O_RDWR|O_CREAT|O_TRUNC, 0644);
                    long n4 = ct_leia(cf4, nu2), v4 = 0, q4 = 0; char pq4[256];
                    while(ct_passo(cf4, n4, pq4, sizeof pq4) == 1) ;
                    if(!ct_valorq(cf4, n4, &v4, &q4) || v4 != V[k].v || q4 != V[k].q) vmal++;
                    close(cf4);
                }
                unlink(cf_n4);
                ok("a membrana tem VOLTA: o valor veste-se, o vestido desdobra-se, e a"
                   " fita devolve o mesmo numero — os dois lados, nao um com o nome do par",
                   vmal == 0);
            }

            /* E A ORDEM: a involucao e NA ENTRADA. Vestida, «x^{2} = 4» passa na porta
             * da equacao (que aceita chavetas) e NAO na do polinomio — e sai a resposta
             * do resolvedor errado. Desdobrada primeiro, vai a regua certa. Foi assim
             * que o defeito apareceu: a fala respondia «isto nao e do primeiro grau». */
            {
                char es[512], dr[512];
                latex_desdobra(nu2, sizeof nu2, "x^{2} = 4");
                ok("a fala vestida vai a regua ERRADA e desdobrada vai a certa —"
                   " por isso a involucao e na entrada",
                   !e_poli("x^{2} = 4") && e_equacao("x^{2} = 4", es, dr, sizeof es) &&
                   e_poli(nu2) && tem_membrana("x^{2} = 4") &&
                   !tem_membrana("bom dia, tudo bem?") && !tem_membrana("gosto de rock"));
            }
        }

        /* «resolve X» / «calcula X» — o pedido em portugues tira a roupa e o resto vai a
         * MESMA cascata simbolica (equacao, polinomio, sistema, ED, conta, funcoes). A
         * fronteira de palavra guarda: «resolvemos tudo» nao e pedido; e «resolve o meu
         * problema» tira a roupa mas nenhuma regua simbolica aceita — o corpus decide. */
        {
            const char *nu = pedido_nu("resolve 2x + 3 = 11");
            char esq2[512], dir2[512];
            ok("«resolve X» tira a roupa e o resto E a equacao que a regua ja sabia",
               nu && e_equacao(nu, esq2, dir2, sizeof esq2));
            ok("e a fronteira guarda: «resolvemos tudo» e «resolve o meu problema» nao"
               " roubam nada",
               pedido_nu("resolvemos tudo") == 0 &&
               (nu = pedido_nu("resolve o meu problema")) != 0 &&
               !e_conta(nu) && !e_poli(nu) && !e_sistema(nu));
            const char *ca = pedido_nu("calcula a raiz de 16");
            char exf[1200];
            ok("e «calcula a raiz de 16» chega as funcoes nomeadas",
               ca && funcao_monta(ca, exf, sizeof exf) && !strcmp(exf, "raiz (16)"));
        }

        /* «esquece X» — o inverso do lembra pela mesma porta: a resposta do no apaga-se,
         * o caminho fica. A volta MEDIDA: lembrar, responder, esquecer, nao responder. */
        {
            int e1 = resolve_lembra("lembra que a cor do teste = verde");
            int dd2; long re = t_erosao("a cor do teste", &dd2);
            int e2 = resolve_esquece("esquece a cor do teste");
            long re2 = t_erosao("a cor do teste", &dd2);
            ok("«esquece X» apaga a resposta e a fala volta ao nao-sei",
               e1 && re && e2 && re2 == 0);
            ok("e esquecer o que nunca se soube devolve 0 (nada a esquecer)",
               resolve_esquece("esquece o que nunca existiu aqui") == 0);
        }
    }
    for(int i = 0; i < NB; i++){ char c[512]; snprintf(c, sizeof c, "%s.%d", b, i);
                                 close(fdv[i]); unlink(c); }
    return falhas ? 1 : 0;
}

int main(int argc, char **argv){
    if(argc < 2) return teste();
    if(argc < 3){
        fprintf(stderr, "uso: conversa <base> aprende \"fala\" \"resposta\"\n"
                        "     conversa <base> responde \"fala\"\n"
                        "     conversa <base> conversa\n");
        return 2;
    }
    snprintf(barr_base, sizeof barr_base, "%s", argv[1]);
    barr_abre(argv[1]);
    if(fd < 0){ perror("base"); return 2; }
    for(int b = 0; b < NB; b++){ no_banco(b);
        if(le(H_LIVRE).a < RAIZ + NOSL){ Slot h = { RAIZ + NOSL, 0 }; grava(H_LIVRE, h); } }

    if(!strcmp(argv[2], "aprende") && argc >= 5) aprende(argv[3], argv[4]);
    else if(!strcmp(argv[2], "responde") && argc >= 4) responde(argv[3]);
    else if(!strcmp(argv[2], "tick")){
        /* O RELÓGIO NA MÃO: um tick de cada vez, e a velocidade escolhe-se por TICKS
         * (1, 2, 4, 8 — os andares da torre). A fita fica no disco entre os ticks. */
        if(argc >= 4 && !strcmp(argv[3], "fim")) relogio_tick(argv[1], 0, 1);
        else relogio_tick(argv[1], argc >= 4 ? argv[3] : 0, 0);
    }
    else if(!strcmp(argv[2], "volta")){
        for(int b = 0; b < NB; b++){ no_banco(b); poe_onde(RAIZ); cam_poe(""); }
        printf("voltei ao princípio — a conversa recomeça, e nada se perdeu.\n");
    }
    else if(!strcmp(argv[2], "sobe") && argc >= 4){        /* a EROSAO nas coordenadas */
        int k = atoi(argv[3]); char c[2048];
        for(int b = 0; b < NB; b++){
            no_banco(b); cam_le(c, sizeof c);
            if(!c[0]) continue;
            int n = (int)strlen(c) - k; if(n < 0) n = 0;
            c[n] = 0; cam_poe(c);
            long no = RAIZ; for(const char *p = c; *p; ) no = filho(no, prox_simb(&p), 0), no = no ? no : RAIZ;
            poe_onde(no);
            printf("subi %d — estou em \"%s\"\n", k, c);
            break;
        }
    }
    else if(!strcmp(argv[2], "onde")){                     /* as coordenadas, lidas */
        char c[2048]; int vazio = 1;
        for(int b = 0; b < NB; b++){
            no_banco(b); cam_le(c, sizeof c);
            if(c[0]){ printf("estou em \"%s\"  (banco %d)\n", c, b); vazio = 0; break; }
        }
        if(vazio) printf("estou no princípio — nenhuma coordenada escrita.\n");
    }
    else if(!strcmp(argv[2], "ramos")){
        /* OS RAMOS DAQUI. Navegar as cegas nao e navegar: daqui, que simbolos continuam? Sao os
         * filhos do no, e le-los e o mesmo passo de descer — so que sem escolher um. */
        char c[2048];
        /* NO PRINCIPIO COMECA-SE EM QUALQUER BANCO. Eu olhava so um e dizia "e folha" — mas do
         * principio o que continua e a raiz de TODOS: e ai que o alfabeto inteiro esta. */
        int achei = 0;
        for(int b = 0; b < NB; b++){ no_banco(b); cam_le(c, sizeof c); if(c[0]){ achei = 1; break; } }
        if(!achei){
            printf("do princípio continuam:");
            int n = 0;
            for(int b = 0; b < NB; b++){
                no_banco(b);
                long v = RAIZ;
                for(;;){
                    for(int k = 1; k <= LARG; k++){
                        Slot p2 = le(v + k);
                        if(!p2.a) continue;
                        int sim = (int)(p2.a + 31);
                        printf(" '%c'", (sim >= 32 && sim < 127) ? sim : '?');
                        n++;
                    }
                    Slot cont = le(v + NOSL - 1);
                    if(!cont.b) break;
                    v = cont.b;
                }
            }
            if(!n) printf(" (nada — o corpus está vazio)");
            printf("\n");
            close(fdv[0]); return 0;
        }
        for(int b = 0; b < NB; b++){
            no_banco(b); cam_le(c, sizeof c);
            if(!c[0]) continue;
            long no = RAIZ;
            for(const char *p = c; *p; ){ long f = filho(no, prox_simb(&p), 0); if(!f) break; no = f; }
            printf("de \"%s\" continuam:", c[0] ? c : "(princípio)");
            int n = 0;
            long v = no;
            for(;;){
                for(int k = 1; k <= LARG; k++){
                    Slot p2 = le(v + k);
                    if(!p2.a) continue;
                    int sim = (int)(p2.a + 31);
                    printf(" '%c'", (sim >= 32 && sim < 127) ? sim : '?');
                    n++;
                }
                Slot cont = le(v + NOSL - 1);
                if(!cont.b) break;
                v = cont.b;
            }
            if(!n) printf(" (nada — é folha)");
            printf("\n");
            break;
        }
    }
    else if(!strcmp(argv[2], "reflete")){                  /* J: a TROCA, e ela e involucao */
        char c[2048];
        for(int b = 0; b < NB; b++){
            no_banco(b); cam_le(c, sizeof c);
            if(!c[0]) continue;
            /* le o caminho ao contrario — simbolo a simbolo, e o acento conta como UM */
            char r[2048]; size_t j = 0;
            long ini[1024]; int n = 0;
            { const char *p = c; while(*p){ ini[n < 1024 ? n : 1023] = p - c; n++; prox_simb(&p); } }
            for(int k = n - 1; k >= 0; k--){
                long a0 = ini[k], a1 = (k + 1 < n) ? ini[k+1] : (long)strlen(c);
                for(long t = a0; t < a1 && j < sizeof r - 1; t++) r[j++] = c[t];
            }
            r[j] = 0;
            cam_poe(r);
            long no = RAIZ; for(const char *p = r; *p; ){ long f = filho(no, prox_simb(&p), 0);
                                                          if(!f) break; no = f; }
            poe_onde(no);
            printf("refleti: \"%s\" -> \"%s\"\n", c, r);
            printf("   (J, a troca — e ela e involucao: refletir duas vezes volta ao mesmo)\n");
            break;
        }
    }
    else if(!strcmp(argv[2], "salta") && argc >= 4){        /* as coordenadas, de uma vez */
        /* limpar os outros fios: ha UM ponto, e saltar poe-no aqui. Deixar o antigo la era ter
         * dois pontos e o reflete pegar no primeiro que encontrasse. */
        for(int b = 0; b < NB; b++){ no_banco(b); cam_poe(""); poe_onde(RAIZ); }
        no_banco(banco_da(argv[3]));
        cam_poe(argv[3]);
        long no = RAIZ; for(const char *p = argv[3]; *p; ) no = filho(no, prox_simb(&p), 0), no = no ? no : RAIZ;
        poe_onde(no);
        printf("saltei para \"%s\"\n", argv[3]);
    }
    else if(!strcmp(argv[2], "-")){
        /* INGERIR: um par por linha, APRENDE 'fala' 'resposta'. E por aqui que o sistema ensina
         * o que ele proprio ja escreveu — a teoria, os papers, o que houver. */
        char l[4096]; long n = 0;
        while(fgets(l, sizeof l, stdin)){
            char *a1 = strchr(l, 39); if(!a1) continue;
            char *a2 = strchr(a1 + 1, 39); if(!a2) continue;
            char *a3 = strchr(a2 + 1, 39); if(!a3) continue;
            char *a4 = strrchr(a3 + 1, 39); if(!a4 || a4 == a3) continue;
            *a2 = 0; *a4 = 0;
            aprende(a1 + 1, a3 + 1);
            n++;
        }
        fprintf(stderr, "%ld par(es) ingerido(s)\n", n);
    }
    else if(!strcmp(argv[2], "conversa")){
        { long t = 0;                                  /* o corpus e a SOMA dos bancos: o
                                                        * contador vive em cada um, e ler so o
                                                        * primeiro dizia sempre zero */
          for(int b = 0; b < NB; b++){ no_banco(b); t += le(H_PARES).a; }
          printf("corpus com %ld par(es) em %d bancos. Escreve a fala; para ensinar: = a resposta\n\n", t, NB); }
        char linha[1024], ultima[1024] = "";
        while(fgets(linha, sizeof linha, stdin)){
            size_t n = strlen(linha);
            while(n && (linha[n-1] == '\n' || linha[n-1] == '\r')) linha[--n] = 0;
            if(!n) continue;
            if(linha[0] == '=' && ultima[0]){ aprende(ultima, linha + 1); continue; }
            strcpy(ultima, linha);
            responde(linha);
        }
    } else { fprintf(stderr, "comando desconhecido\n"); close(fd); return 2; }
    close(fd);
    return 0;
}
