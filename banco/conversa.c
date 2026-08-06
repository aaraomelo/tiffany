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
#include "algebra.h"
#include "edo.h"
#include "poli.h"
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

static Slot le(long i){ Slot s = {0,0}; pread(fd, &s, SL, i*SL); return s; }
static void grava(long i, Slot s){ pwrite(fd, &s, SL, i*SL); }

/* O cabeçalho: [0] = primeiro slot livre, [1] = quantos pares aprendidos. */
#define H_LIVRE 0
#define H_PARES 1
#define RAIZ    2
#define LARG    6            /* filhos por registo de nó — o resto encadeia */
/* nó = [nfilhos | resposta][s1|f1][s2|f2]...[s6|f6][0|continuação] = 8 slots */
#define NOSL    8

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
static void aprende(const char *fala, const char *resp){
    no_banco(banco_da(fala));                      /* nao ha repartidor: a cifra diz */
    long no = RAIZ;
    for(const char *p = fala; *p; ) no = filho(no, prox_simb(&p), 1);
    Slot cab = le(no);
    long r = poe_texto(resp);
    Slot nc = { cab.a, r }; grava(no, nc);          /* uma resposta só: a nova substitui */
    Slot pc = le(H_PARES); pc.a++; grava(H_PARES, pc);
    printf("aprendido — %ld par(es) no corpus\n", pc.a);
}
/* EROSÃO: o prefixo. Desce enquanto houver caminho e devolve a resposta mais funda que viu. */
static long erosao(const char *fala, int *fundo){
    long no = RAIZ, achou = 0; *fundo = 0;
    int d = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) break;
        no = f; d++;
        Slot cab = le(no);
        if(cab.b){ achou = cab.b; *fundo = d; }
    }
    return achou;
}
/* DILATAÇÃO: a subsequência. Salta um símbolo quando o caminho morre — a fala com ruído. */
static long dilatacao(const char *fala, int *fundo){
    long no = RAIZ, achou = 0; *fundo = 0;
    int d = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) continue;                            /* o símbolo não serve: salta-o */
        no = f; d++;
        Slot cab = le(no);
        if(cab.b){ achou = cab.b; *fundo = d; }
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
    while(*p && n < max){
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
            if(cab.b){ ultima = cab.b; fim = q; }   /* o terminal mais fundo deste troco */
        }
        if(!ultima){                                 /* nada comeca aqui: avanca um simbolo */
            if(!*p) break;
            prox_simb(&p);
            continue;
        }
        if(n < 8) torc_banco[n] = banco_da(p);
        saida[n++] = ultima;
        p = fim;                                     /* recomeca da raiz com o que sobrou */
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
    double a[NCAND], s[NCAND], w[NCAND], c[NCAND][NCAND];
    double z = 0;
    for(int k = 0; k < n; k++){ a[k] = prof[k] > 0 ? prof[k] : 1; z += a[k]; }
    for(int k = 0; k < n; k++) s[k] = a[k] / z;                 /* iteracao 0: a MARGINAL */
    /* a compatibilidade: o que a candidata partilha com o caminho andado, e com as outras */
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
            size_t p = 0;
            for(size_t t = 0; cand[k][t]; t++) if(vis[(unsigned char)cand[k][t]]) p++;
            /* e as palavras inteiras pesam mais que as letras soltas */
            size_t pal = 0;
            { char cp[1024]; snprintf(cp, sizeof cp, "%s", ctx);
              char *tk = strtok(cp, " ,.;:!?");
              while(tk){ if(strlen(tk) > 3 && strstr(cand[k], tk)) pal += strlen(tk); tk = strtok(NULL, " ,.;:!?"); } }
            size_t q = 0;
            while(cand[k][q] && cand[j][q] && cand[k][q] == cand[j][q]) q++;
            c[k][j] = (double)(p + 8*pal + q) / 64.0;
        }
    const double m = 1.0, eps = 1e-13;
    double d = 1;
    for(int it = 0; it < 60 && d > eps; it++){
        for(int k = 0; k < n; k++) w[k] = s[k];
        d = 0; double zz = 0; double nv[NCAND];
        for(int k = 0; k < n; k++){
            double viz = 0;
            for(int j = 0; j < n; j++) if(j != k) viz += w[j] * c[k][j];
            nv[k] = a[k] * (m + viz);
            zz += nv[k];
        }
        for(int k = 0; k < n; k++){
            double v = zz > 0 ? nv[k]/zz : 0;
            double dd = v - s[k]; if(dd < 0) dd = -dd; if(dd > d) d = dd;
            s[k] = v;
        }
    }
    int b = 0;
    for(int k = 1; k < n; k++) if(s[k] > s[b]) b = k;
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
        if(cab.b){ achou = cab.b; *fundo = d; poe_onde(no);
                   char c[2048]; cam_le(c, sizeof c);
                   size_t l0 = strlen(c);
                   if(l0 + strlen(fala) + 1 < sizeof c){ strcat(c, fala); cam_poe(c); } }
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
        if(fabs(p.c[k]) < 1e-12) continue;
        printf("%s", (k == p.n) ? "" : (p.c[k] < 0 ? " - " : " + "));
        double a2 = (k == p.n) ? p.c[k] : fabs(p.c[k]);
        if(fabs(a2 - 1) > 1e-12 || k == 0) printf("%g", a2);
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
            if(fabs(p.c[k]) < 1e-12) continue;
            printf("%s", (k == p.n) ? "" : (p.c[k] < 0 ? " - " : " + "));
            double a2 = (k == p.n) ? p.c[k] : fabs(p.c[k]);
            if(fabs(a2 - 1) > 1e-12 || k == 0) printf("%g", a2);
            if(k >= 1) printf("x");
            if(k >= 2) printf("^%d", k);
        }
        printf(")   com σ a raiz, por construção\n");
        if(p.n == 2)
            if(fabs(p.c[1]) < 1e-12) printf("     e a borda:  s^2 = %g\n", -p.c[0]);
            else printf("     e a borda:  s^2 = %g%s%gs\n", -p.c[0],
                        -p.c[1] < 0 ? " - " : " + ", fabs(p.c[1]));
        printf("     (aproximá-la em decimal seria SAIR do corpo para dar um número que já não\n");
        printf("      é raiz de nada — e é exatamente o que este sistema não faz)\n\n");
    }

    if(p.n == 2){
        double D = p.c[1]*p.c[1] - 4*p.c[0];
        printf("   e em grau 2 a assinatura cabe num número: Δ = %g, logo %s\n", D,
               D > 0 ? "HIPERBÓLICO" : D < 0 ? "ELÍPTICO" : "PARABÓLICO");
    } else
        printf("   (em grau 2 isto seria o Δ; acima dele classifica o par (r,s), e há %d\n"
               "    assinaturas possíveis em grau %d)\n", p.n/2 + 1, p.n);
    return 1;
}

/* O SISTEMA. "x' = x + 2y ; y' = 3x + 4y" — e o que ele mostra é que a régua do sistema É a
 * régua (B, C) do catálogo: para 2x2 o característico é λ² − tr·λ + det, logo B = −traço e
 * C = determinante, sem tradução nenhuma. */
static int sis_le(const char *f, double *a, double *b, double *c, double *d){
    const char *pv = strchr(f, ';');
    if(!pv) return 0;
    double m[2][2] = {{0,0},{0,0}};
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
            double sinal = 1;
            if(*p == '+'){ p++; }
            else if(*p == '-'){ sinal = -1; p++; }
            while(p < fim && *p == ' ') p++;
            double v = 0; int tem = 0;
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
    double a,b,c,d;
    return strchr(f, ';') && strstr(f, "'") && sis_le(f, &a,&b,&c,&d);
}
static int resolve_sistema(const char *f){
    double a,b,c,d;
    if(!sis_le(f, &a,&b,&c,&d)) return 0;
    double T = a + d, De = a*d - b*c, D = T*T - 4*De;
    printf("   %s\n", f);
    printf(" = x' = Ax, com A = [[%g,%g],[%g,%g]]\n", a, b, c, d);
    printf("   traço %g, determinante %g, Δ = tr² - 4det = %g\n", T, De, D);
    printf("   e a régua do sistema É a do catálogo: B = -traço = %g, C = det = %g\n", -T, De);
    printf("   logo %s\n", D > 0 ? "HIPERBÓLICO — o gato, cresce e gasta"
                        : D < 0 ? "ELÍPTICO — o esquilo, gira e não gasta"
                                : "PARABÓLICO — a fronteira, e é onde entra o t");
    double re;
    if(D > 0){
        double r1 = (T + sqrt(D))/2, r2 = (T - sqrt(D))/2;
        printf("   os autovalores são %.9f e %.9f\n", r1, r2);
        re = r1 > r2 ? r1 : r2;
    } else if(D < 0){
        printf("   os autovalores são %.6f ± %.6f i\n", T/2, sqrt(-D)/2);
        re = T/2;
    } else {
        printf("   o autovalor é duplo: %.6f\n", T/2);
        re = T/2;
    }
    printf("   (regime: %s — Re máx = %+.3f)\n",
           re > 1e-9 ? "CAOS, diverge" : re < -1e-9 ? "CRISTAL, colapsa no ponto fixo"
                                                    : "BORDA, orbita e conserva", re);
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
    double B = (double)e.Bp/e.Bq, C = (double)e.Cp/e.Cq, D = B*B - 4*C;
    printf("   %s\n", f);
    printf(" = a característica é  L^2 %c %g L %c %g = 0\n",
           B < 0 ? '-' : '+', B < 0 ? -B : B, C < 0 ? '-' : '+', C < 0 ? -C : C);
    printf("   e isso É a borda do corpo:  %s   (o D no lugar do s)\n", bt);
    printf("   Δ = %g, logo %s\n", D,
           D > 0 ? "HIPERBÓLICO — o gato, cresce e gasta"
         : D < 0 ? "ELÍPTICO — o esquilo, gira e não gasta"
                 : "PARABÓLICO — a fronteira, o absorvente");
    if(D > 0){
        double r1 = (-B + sqrt(D))/2, r2 = (-B - sqrt(D))/2;
        printf("   as raízes são %.9f e %.9f, e a solução é\n", r1, r2);
        printf("     y = A·e^(%.6f t) + B·e^(%.6f t)\n", r1, r2);
        double re = r1 > r2 ? r1 : r2;
        printf("   (regime: %s — Re máx = %+.3f)\n",
               re > 1e-9 ? "CAOS, diverge" : re < -1e-9 ? "CRISTAL, colapsa no ponto fixo"
                                                        : "BORDA", re);
        if(fabs(B + 1) < 1e-12 && fabs(C + 1) < 1e-12)
            printf("   (e esta é a do OURO: as raízes são φ e -1/φ, e a mesma recorrência em\n"
                   "    passos inteiros é Fibonacci)\n");
    } else if(D < 0){
        double a2 = -B/2, w = sqrt(-D)/2;
        printf("   as raízes são %.6f ± %.6f i, e a solução é\n", a2, w);
        printf("     y = e^(%.6f t)·(A·cos(%.6f t) + B·sen(%.6f t))\n", a2, w, w);
        printf("   (regime: %s)\n", fabs(a2) < 1e-12 ? "BORDA — orbita, a norma conserva-se"
                                   : a2 < 0 ? "CRISTAL — oscila e amortece" : "CAOS — oscila e cresce");
        if(fabs(B) < 1e-12 && fabs(C - 1) < 1e-12)
            printf("   (e esta é a do i: a borda s^2 = -1, e a solução é a ROTAÇÃO)\n");
    } else {
        double r = -B/2;
        printf("   a raiz é dupla, %.6f, e a solução é\n", r);
        printf("     y = (A + B·t)·e^(%.6f t)\n", r);
        printf("   (regime: a fronteira — é aqui que o discriminante se anula)\n");
    }
    if(fo.tipo != F_NENHUMA){
        /* A NÃO HOMOGÉNEA: a fonte desloca o corpo livre. */
        char yp[192];
        int r = edo_particular(B, C, fo, yp, sizeof yp);
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
            double v0 = (double)p0/q0, v1 = (double)p1/q1, v2 = (double)p2/q2;
            if(fabs((v2 - v1) - (v1 - v0)) > 1e-9) lados_lineares = 0;
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
static double circ_valor(const char *s, int *ok_){
    char *fim;
    double v = strtod(s, &fim);
    if(fim == s){ *ok_ = 0; return 0; }
    *ok_ = 1;
    while(*fim == ' ') fim++;
    switch(*fim){                                  /* os sufixos da bancada */
        case 'p': return v*1e-12;
        case 'n': return v*1e-9;
        case 'u': return v*1e-6;
        case 'm': return v*1e-3;
        case 'k': case 'K': return v*1e3;
        case 'M': return v*1e6;
        default: return v;
    }
}
static int circ_le(const char *f, const char *chave, double *v, int quantos){
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
static int resolve_circuito(const char *f){
    double v[4];
    printf("   %s\n", f);
    if(circ_le(f, "rlc", v, 3)){
        double R = v[0], L = v[1], C = v[2];
        double w0 = el_ressonancia(L,C), D = el_delta(R,L,C), Rc = 2.0*sqrt(L/C);
        double complex Z = el_rlc(R,L,C,w0);
        printf(" = R = %g Ω, L = %g H, C = %g F\n", R, L, C);
        printf("   a borda é  L·s² + R·s + 1/C = 0  — a MESMA das EDs, com s no lugar do σ\n");
        printf("   ω₀ = 1/√(LC) = %.6f rad/s   (f₀ = %.4f Hz)\n", w0, w0/(2*M_PI));
        printf("   Z(ω₀) = %.6f %+.6fj Ω,  logo Im Z = %.1e e FP = %.9f\n",
               creal(Z), cimag(Z), cimag(Z), el_fp(Z));
        printf("   Δ = R² - 4L/C = %g,  e R crítico = 2√(L/C) = %.4f Ω\n", D, Rc);
        printf("   logo é %s%s\n",
               D < -1e-12 ? "SUBAMORTECIDO: oscila e decai (Δ<0, o par conjugado — o círculo)"
             : D >  1e-12 ? "SOBREAMORTECIDO: volta sem oscilar (Δ>0, duas reais — a hipérbole)"
                          : "CRÍTICO: a raiz é DUPLA (Δ=0, a fronteira ε²=0)",
               fabs(D) <= 1e-12 ? " — e a 2ª solução entra como t·e^{st}" : "");
        printf("   (na ressonância o +1 do indutor cancela o -1 do capacitor: nada volta,\n");
        printf("    toda a potência é ativa. É o casamento — o cone nulo σ=1 em circuito)\n");
        return 1;
    }
    if(circ_le(f, "ressonancia", v, 2) || circ_le(f, "ressonância", v, 2)){
        double L = v[0], C = v[1], w0 = el_ressonancia(L,C);
        printf(" = L = %g H, C = %g F\n", L, C);
        printf("   ω₀ = 1/√(LC) = %.6f rad/s,  f₀ = %.4f Hz\n", w0, w0/(2*M_PI));
        printf("   Z₀ = √(L/C) = %.6f Ω    (a média geométrica — o metal, La Hire)\n",
               el_Z0(L,C));
        printf("   (o indutor tem multiplicidade +1 e o capacitor -1; somam 0, que é o\n");
        printf("    resistor — e é por isso que na ressonância só sobra o R)\n");
        return 1;
    }
    if(circ_le(f, "serie", v, 2) || circ_le(f, "série", v, 2)){
        printf(" = em SÉRIE as impedâncias SOMAM — é Kirchhoff, a operação ⊕\n");
        printf("   %g + %g = %g Ω\n", v[0], v[1], v[0]+v[1]);
        return 1;
    }
    if(circ_le(f, "paralelo", v, 2)){
        double g = 1/v[0] + 1/v[1];
        printf(" = em PARALELO somam as CONDUTÂNCIAS — o mesmo ⊕, no dual\n");
        printf("   1/%g + 1/%g = %g S,  logo Z = %g Ω\n", v[0], v[1], g, 1/g);
        printf("   (série e paralelo são o par dual Z ⋈ Y: a mesma soma, dos dois lados)\n");
        return 1;
    }
    if(circ_le(f, "divisor", v, 2)){
        double a = v[1]/(v[0]+v[1]);
        printf(" = o DIVISOR é o PRODUTO (⊗): o ganho α, e compor divisores MULTIPLICA\n");
        printf("   α = R2/(R1+R2) = %g/(%g+%g) = %.9f\n", v[1], v[0], v[1], a);
        printf("   e V_out = α·V_in;  dois em cascata dão α₁·α₂, não α₁+α₂\n");
        return 1;
    }
    if(circ_le(f, "wheatstone", v, 3)){
        double complex zx = el_wheatstone(v[0], v[1], v[2]);
        double complex d = el_detector(v[0], v[1], v[2], zx, 10.0);
        printf(" = a ponte mede por ANULAÇÃO: ajusta-se até o detector ler ZERO\n");
        printf("   equilíbrio Z₁·Z_x = Z₂·Z₃  ->  Z_x = Z₂·Z₃/Z₁ = %g·%g/%g = %.6f Ω\n",
               v[1], v[2], v[0], creal(zx));
        printf("   e o detector lê %.2e no equilíbrio  (com 10 V na ponte)\n", cabs(d));
        printf("   (não se lê o valor num mostrador, que teria a precisão do mostrador:\n");
        printf("    lê-se a RAZÃO no ponto de resíduo 0 — e a razão é exata)\n");
        return 1;
    }
    if(circ_le(f, "amplificador", v, 2) || circ_le(f, "ganho", v, 2)){
        double Ic = v[0], Rc = v[1], gm = Ic/VT;
        printf(" = o AMPLIFICADOR é o transistor DENTRO da janela ativa\n");
        printf("   gm = dIc/dVbe = Ic/VT = %g/%g = %.4f A/V   (a transcondutância)\n",
               Ic, VT, gm);
        printf("   A_v = -gm·Rc = %.4f      (o ganho, em emissor comum)\n", -gm*Rc);
        printf("   AMPLIFICAR É LINEARIZAR: gm é a DERIVADA da exponencial no ponto de\n");
        printf("   operação — é a parte ε de f(a+bε) = f(a) + f'(a)·b·ε, com ε² = 0.\n");
        printf("   (e por isso o ganho depende do ponto de operação; com realimentação ele\n");
        printf("    vira 1/β, uma RAZÃO de resistores — e a razão é exata)\n");
        return 1;
    }
    if(circ_le(f, "logica", v, 2) || circ_le(f, "lógica", v, 2) || circ_le(f, "porta", v, 2)){
        int a = v[0] != 0, b = v[1] != 0;
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
    if(circ_le(f, "somador", v, 3) || circ_le(f, "somador", v, 2)){
        int a = v[0] != 0, b = v[1] != 0, ci = (v[2] == 0 || v[2] == 1) ? (int)v[2] : 0;
        int s = (a != b) != ci, co = (a&&b) || (ci && (a!=b));
        printf(" = o SOMADOR COMPLETO, em portas:\n");
        printf("   s    = a ⊕ b ⊕ cin              = %d\n", s);
        printf("   cout = (a∧b) ∨ (cin ∧ (a⊕b))   = %d\n", co);
        printf("   e a aritmética direta: %d + %d + %d = %d, que em binário é %d%d  <- O MESMO\n",
               a, b, ci, a+b+ci, co, s);
        printf("   (dois caminhos: as portas e a conta. Um somador que só fecha num deles\n");
        printf("    não está validado — está adivinhado)\n");
        return 1;
    }
    if(circ_le(f, "transistor", v, 1)){
        double V = v[0], Is = 1e-14, Ic = el_shockley(V, Is);
        printf(" = SHOCKLEY: I = Is·(e^{V/VT} - 1),  com VT = %.6f V a 300 K\n", VT);
        printf("   V = %g V  ->  I = %.6e A   (%.4f mA)\n", V, Ic, Ic*1e3);
        printf("   e o inverso: V = VT·ln(I/Is + 1) = %.6f V\n", el_shockley_inv(Ic, Is));
        printf("   AQUI VIVE O OPERADOR. Π = exp∘Σ∘log é esta equação:\n");
        printf("   I(V₁+V₂) = I(V₁)·I(V₂)/Is — a SOMA de tensões vira PRODUTO de correntes.\n");
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
    char a[1024];
    inv_troca(a, sizeof a, s, "vezes", "x");
    inv_troca(d, dn,      a, "mais",  "+");
}

static void responde(const char *fala){
    if(e_circuito(fala) && resolve_circuito(fala)) return; /* a tríade em volts e amperes */
    if(e_poli(fala) && resolve_poli(fala)) return;         /* p(x) = q(x), de qualquer grau */
    if(e_sistema(fala) && resolve_sistema(fala)) return;   /* x' = Ax, e a régua é (−tr, det) */
    if(e_edo(fala) && resolve_edo(fala)) return;           /* a ED declara o corpo pela borda */
    if(e_algebra(fala) && resolve_algebra(fala)) return;   /* o corpo vem declarado na fala */
    {   /* a equação vem antes de tudo: '=' na fala é resolver, e não avaliar */
        char esq[512], dir[512];
        if(e_equacao(fala, esq, dir, sizeof esq) && resolve_eq(esq, dir)) return;
    }
    int dist = 0;
    const char *lei = pede_lei(fala, &dist);           /* "distribui ..." / "fatora ..." */
    if(lei && e_conta(lei) && aplica_lei(lei, dist)) return;
    if(e_conta(fala) && resolve_conta(fala)) return;   /* conta não se procura: desdobra-se */
    {   /* e se não era conta, DESDOBRA-SE O CONE e pergunta-se outra vez: "3 vezes 3" é
         * "3 x 3" escrito comprimido. O corpus não vê esta forma — só o resolvedor. */
        char cone[1024]; desdobra_entrada(cone, sizeof cone, fala);
        if(strcmp(cone, fala) && e_conta(cone) && resolve_conta(cone)) return;
    }
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
            char t[1024]; le_texto(v[k], t, sizeof t);
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
    const char *via = "daqui (a conversa continua)";
    if(fio >= 0){ no_banco(fio); r = desce_daqui(fala, &d); }
    if(!r){                                        /* nao continua: e comeco */
        no_banco(banco_da(fala));
        r = desce_daqui(fala, &d);
        if(!r){ r = erosao(fala, &d); via = "erosão (prefixo)"; }
    }
    if(!r){
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
        /* ANTES DO DECRETO: perguntar ao barramento. Nao sei nao e o fim — e o fim do que EU sei. */
        char outra[1024];
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
    char t[1024]; le_texto(r, t, sizeof t);
    printf("%s\n", t);
    printf("   (%s, %d símbolo(s) de caminho)\n", via, d);
}

/* O MEDIDOR. Sem argumentos, a assistente mede-se a si propria — e as asserções são o que ela
 * promete: as tres reguas do morfico, o decreto a recusar, e o acento a nao partir o caminho. */
#include "unidade.h"
/* nos testes, chamar a regua direto exige escolher o banco — o que o responde() faz por dentro.
 * Estes atalhos poem o teste no MESMO caminho do programa, que foi o que faltou da primeira vez. */
static long t_erosao(const char *f, int *d){ no_banco(banco_da(f)); return erosao(f, d); }
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

    printf("\n§C2  DILATACAO: a subsequencia — a fala com ruido, antes ou no meio.\n\n");
    r = t_erosao("hmm quem és tu?", &d);
    printf("      pela erosao            %s\n", r ? "achou" : "nao acha (o ruido a frente mata o prefixo)");
    long r2 = t_dilata("hmm quem és tu?", &d); le_texto(r2, t, sizeof t);
    printf("      pela dilatacao         %s  (%d simbolos)\n", t, d);
    ok("o que a erosao perde por ruido a frente, a dilatacao acha", !r && r2);
    long r3 = t_dilata("quem, afinal, és tu", &d);
    ok("e acha tambem com o ruido NO MEIO", r3 != 0);

    printf("\n§C3  TORCAO: duas falas no mesmo canal, desentrelacadas.\n\n");
    { long v[8];
      int n = t_torcao("bom dia quem és tu", v, 8);
      printf("      \"bom dia quem és tu\"  -> %d fala(s) achada(s):\n", n);
      for(int k = 0; k < n; k++){ no_banco(torc_banco[k]); le_texto(v[k], t, sizeof t); printf("        %s\n", t); }
      ok("a torcao desentrelaca as DUAS falas de uma so linha", n == 2);
      int m = t_torcao("bom dia", v, 8);
      ok("e uma fala sozinha continua a ser uma so", m == 1);
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
            double sa,sb,sc,sd;
            int q1 = e_sistema("x' = y ; y' = -x");
            int q2 = e_sistema("o gato e o esquilo ; os dois lados");
            sis_le("x' = y ; y' = -x", &sa,&sb,&sc,&sd);
            double T = sa+sd, De = sa*sd-sb*sc;
            Edo eo; edo_le("y'' = -y", &eo);
            double B = (double)eo.Bp/eo.Bq, C = (double)eo.Cp/eo.Cq;
            printf("\n      \"x' = y ; y' = -x\"   -> sistema? %s, A = [[%g,%g],[%g,%g]]\n",
                   q1 ? "sim" : "nao", sa,sb,sc,sd);
            printf("      traco %g, det %g  ->  B = -tr = %g, C = det = %g\n", T, De, -T, De);
            printf("      e a ED  y'' = -y  da  B = %g, C = %g   <- O MESMO\n", B, C);
            printf("      \"o gato e o esquilo ; …\" -> sistema? %s   <- vai as reguas\n\n",
                   q2 ? "sim" : "nao");
            ok("a regua do SISTEMA e a da ED sao a mesma: (B,C) = (-traco, det)",
               q1 && !q2 && -T == B && De == C);
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
            double L = 1e-3, C = 1e-6, Rc = 2.0*sqrt(L/C);
            double Dsub = el_delta(Rc*0.3, L, C), Dcri = el_delta(Rc, L, C);
            double Dsob = el_delta(Rc*3.0, L, C);
            double complex Z = el_rlc(20, L, C, el_ressonancia(L,C));
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
            printf("      R = %.1f  ->  Δ = %+.1f  (subamortecido)\n", Rc*0.3, Dsub);
            printf("      R = %.1f  ->  Δ = %+.1f  (CRÍTICO — a raiz dupla, ε² = 0)\n", Rc, Dcri);
            printf("      R = %.1f ->  Δ = %+.1f  (sobreamortecido)\n", Rc*3.0, Dsob);
            printf("      e na ressonância Im Z = %.1e, FP = %.9f  <- o casamento\n\n",
                   cimag(Z), el_fp(Z));
            printf("      \"amplificador 1m 1k\"  -> circuito? %s\n", c4 ? "sim" : "nao");
            printf("      \"logica 1 0\"          -> circuito? %s\n", c5 ? "sim" : "nao");
            printf("      \"somador 1 1 1\"       -> circuito? %s\n", c6 ? "sim" : "nao");
            printf("      \"o amplificador tem ganho alto\"    -> %s   <- vai as reguas\n",
                   n4 ? "SIM (mau)" : "nao");
            printf("      \"a porta logica nand e universal\"  -> %s   <- vai as reguas\n\n",
                   n5 ? "SIM (mau)" : "nao");
            ok("a porta do circuito abre para as contas e NAO para a fala portuguesa",
               c1 && c2 && c3 && c4 && c5 && c6 && !n1 && !n2 && !n3 && !n4 && !n5);
            {   /* os dois regimes do MESMO dispositivo, medidos lado a lado */
                double Is = 1e-14, V = 0.60, h = 1e-7;
                double Ic = Is*exp(V/VT);
                double gm_num = (Is*exp((V+h)/VT) - Is*exp((V-h)/VT))/(2*h);
                int mal = 0;
                if(fabs(gm_num - Ic/VT)/(Ic/VT) > 1e-6) mal++;
                for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
                    if((a&&b) != (a*b)%2) mal++;          /* AND e o produto de GF(2) */
                    if((a!=b) != (a+b)%2) mal++;          /* XOR e a soma de GF(2)    */
                    if(!(!a) != a) mal++;                 /* NOT e a dobra, ordem 2   */
                }
                printf("      gm em Vbe=0,60: derivada %.4f = Ic/VT %.4f  <- amplificar E derivar\n",
                       gm_num*1e3, Ic/VT*1e3);
                printf("      e chaveando: AND = x de GF(2), XOR = + de GF(2), NOT = dobra\n\n");
                ok("os dois regimes do MESMO dispositivo: derivada na janela, GF(2) fora dela",
                   mal == 0);
            }
            ok("o RLC cai na mesma regua das EDs, e o critico e a raiz dupla (Delta = 0)",
               Dsub < 0 && fabs(Dcri) < 1e-9 && Dsob > 0 && fabs(el_fp(Z) - 1.0) < 1e-12);
            /* e o OPERADOR: Shockley leva soma de tensoes em produto de correntes */
            {
                double Is = 1e-14, V1 = 0.35, V2 = 0.28;
                double p = Is*exp(V1/VT)*exp(V2/VT), s = Is*exp((V1+V2)/VT);
                printf("      I(%.2f)·I(%.2f)/Is = %.6e\n", V1, V2, p);
                printf("      I(%.2f + %.2f)     = %.6e   <- O MESMO\n\n", V1, V2, s);
                ok("o transistor E o operador: a SOMA de tensoes vira PRODUTO de correntes",
                   fabs(p-s)/s < 1e-11);
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
            no_banco(banco_da("dois lados que"));
            long ambigua = evolucao("dois lados que", &d, &p2);
            no_banco(banco_da("dois lados que comuta"));
            long unica   = evolucao("dois lados que comuta", &d, &p2);
            printf("      \"dois lados que\"        -> %s   <- RAMIFICA (comutam|separam)\n",
                   ambigua ? "respondeu" : "cala-se");
            printf("      \"dois lados que comuta\" -> %s   <- caminho unico\n\n",
                   unica ? "respondeu" : "cala-se");
            ok("a evolucao NAO ADIVINHA: desce so enquanto o caminho e UNICO. Onde o no"
               " ramifica a fala e ambigua e ela cala-se — a continuacao forcada nao e um"
               " palpite, e a unica que existe", !ambigua && unica);
        }

        printf("      A precedencia nao esta escrita em tabela nenhuma: cai da ORDEM das dobras,\n");
        printf("      o mais fundo primeiro e dentro dele o x antes do +. E os tres delimitadores\n");
        printf("      sao o mesmo — quem manda e a profundidade, nao a roupa.\n");
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
