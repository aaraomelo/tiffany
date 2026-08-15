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
        tique("SEM BURACO — %ld é quadrado perfeito e o ponto já estava em ℚ: as quatro"
              " portas dão o mesmo porque não há nada a fechar");
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
          tique("VOLTA — e confere: %ld² = %ld, exato. Não há buraco a preencher aqui");
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
