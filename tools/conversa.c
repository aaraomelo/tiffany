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
static int e_conta(const char *f){
    int digito = 0;
    for(const char *p = f; *p; p++){
        /* "raiz" é a única palavra que entra numa conta. Salta-se inteira; as letras dela
         * soltas não passam, e por isso "a raiz de 2 é racional" continua a ir ao corpus —
         * medido, porque essa fala existe lá e seria mau perdê-la para o resolvedor. */
        if(!strncmp(p, "raiz", 4)){ p += 3; continue; }
        if(!strncmp(p, "mod", 3)){ p += 2; continue; }
        if(*p >= '0' && *p <= '9'){ digito = 1; continue; }
        if(*p==' '||*p=='\t'||*p=='+'||*p=='-'||*p=='*'||*p=='x'||*p=='X'
                     ||*p=='/'||*p==':'||*p=='^'||*p=='%'||*p=='!') continue;
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

static void responde(const char *fala){
    int dist = 0;
    const char *lei = pede_lei(fala, &dist);           /* "distribui ..." / "fatora ..." */
    if(lei && e_conta(lei) && aplica_lei(lei, dist)) return;
    if(e_conta(fala) && resolve_conta(fala)) return;   /* conta não se procura: desdobra-se */
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
        ok("e sem contexto fica a marginal — fail-closed, nao se inventa", a0 == 0 || a0 == 1);
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
