#define _GNU_SOURCE
/* tex.c — O CORPO TRADUTOR DE FORMATO: .tex -> PDF, sem TeX Live e sem dependência nenhuma.
 *
 * O Aarão: "a assistente vai precisar compilar os .tex, senão como teremos os notebooks? Da mesma
 * forma de sempre, usando o corpo prismático. Fica tudo no corpo solar, o corpo lunar desenrola.
 * Corpo diferencial como sempre, corpo de corpos. Já temos o formato tex e outros mapeados. Vê o
 * corpo tradutor e avança com eles. Já estamos fazendo o mesmo para tradução — é tudo uma coisa
 * só, precisa unificar a tradução global."
 *
 * E é mesmo uma coisa só. Nada aqui é máquina nova:
 *
 *   a DESCIDA        já está no caminho.h — e lá está escrito que "o que muda de formato para
 *                    formato NÃO é a descida: é como cada um MARCA o nível". JSON marca com o
 *                    parêntese, Markdown com a contagem de '#'. O LaTeX marca com a BARRA e as
 *                    seccionadoras. Trocar de formato é trocar quem lê a marca. O .tex não abre
 *                    lugar novo: veste a roupa que o analisador já sabe despir.
 *   o LÉXICO         já está no traduz.c §T1 — "o léxico é a roupa geral do idioma, e a tradução
 *                    literal entre palavras usa ele; o resto é transformação mecânica". Aqui o
 *                    léxico é comando -> glifo (\alpha -> a da Symbol), e é a MESMA tabela de
 *                    pares: entra de um lado, sai do outro, e o de volta é ela ao contrário.
 *   o PRISMÁTICO     é a justificação, e é literal. O prisma.c é "o triângulo deformado até
 *                    PREENCHER A ÁREA INTEIRA". Uma linha de texto chega curta e tem de encher a
 *                    largura da coluna: deforma-se o espaço até a área fechar. Encher a área é o
 *                    prismático, e é aqui que ele trabalha.
 *   SOLAR / LUNAR    o solar GUARDA (o documento entra e fica em estrutura), o lunar DESENROLA (a
 *                    estrutura sai em página). São o par do koch.c: o que é reversível atravessa.
 *                    E a prova de que atravessou é o §X6 — o texto que sai do PDF é o que entrou.
 *   DIFERENCIAL      o corpo de corpos: uma descida, e cada formato é uma instância. Acrescentar
 *                    o LaTeX é acrescentar uma Assinatura, não um compilador.
 *
 * O PDF sai SEM COMPRESSÃO de propósito. Não é preguiça: um stream em texto puro é auditável, é
 * lido de volta por este mesmo programa no §X6, e mantém a regra do repo — o que vai ao ar tem de
 * ser reproduzível e verificável a partir da fonte. Um FlateDecode exigiria zlib, e a dependência
 * que se corta é o motivo de isto existir.
 *
 * E a RAM: escreve-se direto no ficheiro, com o /Length em objeto INDIRETO para não ter de
 * acumular o stream em memória à espera de saber o tamanho. Um documento de 100 páginas custa o
 * buffer de uma página.
 *
 *   §X1  a descida: a marca do LaTeX, no mesmo mecanismo do caminho.h
 *   §X2  o léxico: comando -> glifo, e a volta é a mesma tabela ao contrário
 *   §X3  o prismático: encher a área — a justificação mede a linha e deforma o espaço
 *   §X4  o solar guarda, o lunar desenrola: a paginação
 *   §X5  o PDF sai VÁLIDO — cabeçalho, objetos, xref, trailer, e a xref aponta certo
 *   §X6  A VOLTA: o texto extraído do PDF é o texto que entrou. Resíduo 0.
 *
 *   cc -O2 -std=c99 tex.c -lm -o tex && ./tex              (mede)
 *   ./tex documento.tex documento.pdf                      (compila)
 *
 * O QUE ESTA MEDIDO E O QUE NAO ESTA — para nao se ler isto como mais do que e:
 *
 *   Compila os tres documentos do repo: teoria.tex (24pp, 92k glifos), catalogo.tex (60pp,
 *   227k glifos) e dicionario.tex (2pp). Sondei DEZ regioes espalhadas pelo catalogo e em
 *   oito delas as 8 palavras longas atravessam; nas outras duas, 7 e 5 de 8.
 *
 *   O QUE ESTAVA AQUI ESCRITO JA' NAO E' VERDADE, e a nota fica porque a medida e' que
 *   manda. Dizia que ha construcoes que o tradutor COME — «lexico» oito vezes no fonte e
 *   zero no PDF — e que por isso ele nao substitui o pdflatex. MEDIDO agora, palavra a
 *   palavra nos tres documentos: lexico 7:7, roupa 47:47, travessao 3:3. As que a conta
 *   ainda acusa sao artefactos DELA — `llrrrcc` e `lrrrl` sao especificacoes de coluna,
 *   `cvenom` e `cyasmin` sao nomes de cores, `ssimetria` e' «assimetria» partida pelo
 *   proprio regex.
 *
 *   O ultimo caso real era outro e era pior que comer: PARTIR. «travessao» saia
 *   «travess»/«ao», sem hifen, dentro de uma celula mais estreita que a palavra — e as
 *   larguras das colunas sao Fibonacci, que e' escolha minha e nao sai do conteudo.
 *   Transbordar e' feio; partir uma palavra a meio sem sinal e' ERRADO, porque o leitor
 *   nao tem como saber que foi o compositor. Enquanto a largura nao sair do conteudo, a
 *   palavra ganha.
 *
 *   E a tipografia e mais densa que a do TeX (24pp contra 31, 60 contra 72). Nao ha
 *   ligaduras, nao ha hifenizacao, nao ha tabelas nem matematica em display — o $x^2$ sai
 *   como texto, nao como expoente.
 */
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>   /* a costura da saída: um formatador variádico próprio (o tradutor fá-lo) */

#ifndef TEX_COM_LIBC_WASM
/* No wasm estes vivem em tools/libc.c (662-663); o nativo perdeu-os quando a
 * partitura passou a só-WASM (0aa6ace) e o link partia em silêncio — o
 * «NÃO COMPILOU» que não diz FALHA. Os defaults são os do próprio libc. */
int INTERFACE_N = 6;   /* hexal por defeito */
int LADO_N = 0;        /* 0=Hurwitz (bilinear); 1=Gentil */
#endif
static int INTERFACE_DECL = 0; /* \interfacen{N} no .tex: declara o ciclo (6,12,24…) */
void tex_interfacen(long v){
    if(v == 0) INTERFACE_DECL = 0;
    else if(v >= 6 && v <= 384) INTERFACE_DECL = (int)v;
}
static void interface_do_tex(const char *s, long n){
    for(long i = 0; i + 12 < n; i++){
        if(s[i] != '\\' || strncmp(s + i + 1, "interfacen", 10)) continue;
        long q = i + 11;
        while(q < n && (s[q] == ' ' || s[q] == '\n' || s[q] == '\t' || s[q] == '\r')) q++;
        if(q >= n || s[q] != '{') continue;
        char *ep = NULL;
        long v = strtol(s + q + 1, &ep, 10);
        if(ep != s + q + 1 && v >= 6 && v <= 384){ INTERFACE_DECL = (int)v; return; }
    }
}
#include "spline.h"   /* a carta da fonte: a largura vem da CURVA */
#include "disco.h"    /* «o ficheiro É o vector»: os buffers grandes vivem no DISCO */
#include "le_num.h"   /* o strtod e o hex do núcleo, sem libc --- fonte única, travada em str2dbl_dual.c */
#ifndef TEX_COM_LIBC_WASM
#include "reta.h"     /* rt_le_decimal_end — parse exacto na fronteira I/O (Fase C) */
#else
/* wasm: só rt_le_decimal_end — reta.h inteira puxa RtIdaVolta… que o traduz não traduz */
static int rt_le_decimal_end(const char *s, int *sinal, long *p, long *q, const char **end){
    int i = 0;
    while(s[i] == ' ' || s[i] == '\t') i++;
    *sinal = 1;
    if(s[i] == '-'){ *sinal = -1; i++; }
    else if(s[i] == '+') i++;
    long v = 0, d = 1;
    int digitos = 0;
    while(s[i] >= '0' && s[i] <= '9'){
        if(v > 922337203685477580L) return 0;
        v = v*10 + (s[i]-'0'); i++; digitos++;
    }
    if(s[i] == '.'){
        i++;
        while(s[i] >= '0' && s[i] <= '9'){
            if(v > 922337203685477580L || d > 922337203685477580L) return 0;
            v = v*10 + (s[i]-'0'); d *= 10; i++; digitos++;
        }
    }
    if(!digitos) return 0;
    if(s[i] == 'e' || s[i] == 'E'){
        i++;
        int es = 1;
        if(s[i] == '-'){ es = -1; i++; }
        else if(s[i] == '+') i++;
        int e = 0;
        while(s[i] >= '0' && s[i] <= '9'){ e = e*10 + (s[i]-'0'); i++; if(e > 30) return 0; }
        for(int k = 0; k < e; k++){
            if(es > 0){ if(v > 922337203685477580L) return 0; v *= 10; }
            else       { if(d > 922337203685477580L) return 0; d *= 10; }
        }
    }
    *p = v; *q = d;
    if(end) *end = s + i;
    return 1;
}
#endif

/* ── 0 DE MEMÓRIA: OS BUFFERS GRANDES VÃO PARA O DISCO ───────────────────────────────
 *
 * Estavam 9,6 MB em `.bss` — seis vectores estáticos, um deles de 4 MB para a fonte. O
 * `lib/disco.h` existe para isto e diz o padrão: «o ficheiro É o vector, sem cópia». O que
 * fica em memória é um PONTEIRO, e o kernel pagina o que for preciso.
 *
 * E a razão não é o preço: é que não há o que dissipar. A DRAM reconstrói o bit milhares de
 * vezes por segundo e cada reconstrução passa por apagar; o disco escrito uma vez fica
 * quieto. Nesta dimensão disco > memória, e não como troca de custo — como ordem. */
/* O NÚCLEO pede memória por INDIRECÇÃO (como o g_carrega): o wrapper (nativo) aponta g_disco para
 * o mmap de verdade (disco_u8, endereço fixo, sem RAM); o host (wasm) aponta-o para uma fatia da
 * memória linear (onde o slot É a memória). disco_buf continua a cachear o ponteiro por slot. */
static char *(*g_disco)(int i, const char *nome, long n);
/* cache dos 16 slots: 0–2 = banco (bestiário); 3–15 = rascunho da estrela (recua com o 1 bit) */
static char *DISCO_M[16];
static char *disco_buf(int i, long n){
    static const char *nome[16] = {
        "../dados/tex_estilo.bin", "../dados/tex_classe.bin", "../dados/tex_idioma.bin",
        "../dados/tex_fonte.bin",  "../dados/tex_corpo.bin",  "../dados/tex_volta.bin",
        "../dados/tex_toc.bin",    "../dados/tex_mac.bin",    "../dados/tex_hif.bin",
        "../dados/tex_cores.bin",  "../dados/tex_desc.bin",   "../dados/tex_pag.bin",
        "../dados/tex_x12.bin",    "../dados/tex_x13.bin",
        "../dados/tex_x14.bin",    "../dados/tex_x15.bin" };
#ifdef TEX_COM_LIBC_WASM
    /* Rascunho 3–15: após volta SLOT_PTR=0; se DISCO_M ficar, é UAF.
     * Invalida por !SLOT_PTR (tipografia) ou >= MARCO (páginas). Banco 0–2 fica. */
    if(i >= 3){
        if(i >= 7 && i <= 10){
            if(DISCO_M[i] && !SLOT_PTR[i]) DISCO_M[i] = 0;
        } else {
            if(DISCO_M[i] && MARCO && DISCO_M[i] >= (char*)MARCO) DISCO_M[i] = 0;
        }
    }
#endif
    if(!DISCO_M[i]) DISCO_M[i] = g_disco(i, nome[i], n);
    return DISCO_M[i];
}

/* A COSTURA DA ENTRADA (simétrica da Saida): o NÚCLEO não abre ficheiros --- carrega por INDIRECÇÃO.
 * `g_carrega` é um ponteiro de função (um ÍNDICE, que o tradutor chama por call_indirect): o wrapper
 * (nativo) aponta-o para `carrega_nativo` (fopen+fread por demanda); o host (wasm) aponta-o para um
 * leitor de slot pré-carregado. Assim o núcleo tem a CHAMADA mas não o fopen. */
static long (*g_carrega)(const char *nome, int i, long cap);

/* a versão do WRAPPER (nativo): lê `nome` para o slot `i` por fopen+fread. No ficheiro do núcleo
 * (tex_core.c, wasm) esta função NÃO entra --- só o ponteiro e a chamada. */

/* E AS TABELAS TAMBÉM. Não há razão para umas ficarem: a régua que reside são dezenas de
 * bytes — os nomes das quatro secções, as cartas das fontes —, e tudo o que CRESCE COM O
 * DOCUMENTO é disco. O sumário, as macros, a hifenização, as cores, os desenhos e a coluna
 * de páginas da passagem anterior somavam 147 KB em `.bss` e agora são seis ponteiros. */
/* a macro-de-função TABELA saiu: o tradutor expande macros de NOME e mais nada, e os
 * cinco usos escrevem-se por extenso — o mesmo texto que a macro dava, agora à vista */

/* ─────────────────────────────────────────────────────────────────────────────
 * §X1  A DESCIDA — a marca do LaTeX
 *
 * O caminho.h tem  typedef struct { const char *nome; char marca; ... } Assinatura,  e a regra de
 * que o nível se lê contando a marca. No Markdown a marca é '#' e o nível é quantos há. No LaTeX a
 * marca é '\' e o nível está no NOME do comando — sub- por sub-. É a mesma descida: o que muda é
 * quem lê a marca.
 * ───────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; char marca; int nivel; } Sec;

/* o nível de uma seccionadora do LaTeX. 0 = não é seccionadora. */
static const Sec SECS[] = {
    {"part",          '\\', 1}, {"chapter",       '\\', 1},
    {"section",       '\\', 2}, {"subsection",    '\\', 3},
    {"subsubsection", '\\', 4}, {"paragraph",     '\\', 5},
};
#define NSECS ((int)(sizeof SECS / sizeof SECS[0]))


/* ─── A NUMERAÇÃO, E OS NOMES QUE VÊM DO IDIOMA ─────────────────────────────────────
 *
 * Faltavam 82 palavras contra o pdflatex, e eram todas a mesma coisa: os números dos
 * capítulos e secções, os romanos das partes, e as palavras «Capítulo», «Parte»,
 * «Sumário», «Resumo». Os números DERIVAM-SE — são contadores com reposição dos níveis
 * abaixo. As palavras não: são do IDIOMA.
 *
 * E escrevê-las aqui seria a referência escrita à mão — mudar o babel para outro idioma
 * não mudaria nada, e o medidor passaria na mesma. Por isso LÊEM-SE DO BABEL INSTALADO:
 * a opção sai do `\usepackage[...]{babel}` do estilo.tex, e as palavras saem do `.ldf`
 * correspondente. É a mesma fonte de onde o pdflatex as tira.
 *
 * Sem o babel à mão, os nomes ficam VAZIOS e numera-se só o número — degradar é honesto,
 * inventar a palavra não é. */
static char NOME_CAP[48] = "", NOME_PARTE[48] = "", NOME_RESUMO[48] = "", NOME_SUMARIO[48] = "";
static char NOME_REFS[48] = "";
static int NOMES_LIDOS = 0;

/* `Cap\'{\i}tulo` → `Capítulo`: o acento agudo do TeX, em WinAnsi (que é o que se escreve) */
static void tex_para_winansi(const char *in, char *out, size_t cap){
    size_t o = 0;
    static const char VOG[] = "aeiouAEIOU";
    static const unsigned char AGU[] = {0xE1,0xE9,0xED,0xF3,0xFA,0xC1,0xC9,0xCD,0xD3,0xDA};
    static const unsigned char TIL[] = {0xE3,0,0,0xF5,0,0xC3,0,0,0xD5,0};
    static const unsigned char CIR[] = {0xE2,0xEA,0xEE,0xF4,0xFB,0xC2,0xCA,0xCE,0xD4,0xDB};
    for(size_t i = 0; in[i] && o + 2 < cap; ){
        if(in[i] == '\\' && (in[i+1] == '\'' || in[i+1] == '~' || in[i+1] == '^')){
            char ac = in[i+1]; size_t k = i + 2;
            if(in[k] == '{') k++;
            char v = in[k];
            if(v == '\\' && in[k+1] == 'i'){ v = 'i'; k += 2; } else k++;   /* o \i sem pingo */
            if(in[k] == '}') k++;
            const char *pos = strchr(VOG, v);
            if(pos){
                int t = (int)(pos - VOG);
                unsigned char c = ac == '\'' ? AGU[t] : (ac == '~' ? TIL[t] : CIR[t]);
                if(c) out[o++] = (char)c; else out[o++] = v;
            } else out[o++] = v;
            i = k; continue;
        }
        if(in[i] == '\\' || in[i] == '{' || in[i] == '}'){ i++; continue; }
        out[o++] = in[i++];
    }
    out[o] = 0;
}

/* Tabelas do acento TeX (dados, não função — MAX_FUN=256).
 * Arquitectura: COMPÕE só com base vogal; sem base o glifo fica AO MEIO. */
static const char AC_VOG[] = "aeiouAEIOU";
static const unsigned char AC_AGU[] = {0xE1,0xE9,0xED,0xF3,0xFA,0xC1,0xC9,0xCD,0xD3,0xDA};
static const unsigned char AC_TIL[] = {0xE3,0,0,0xF5,0,0xC3,0,0,0xD5,0};
static const unsigned char AC_CIR[] = {0xE2,0xEA,0xEE,0xF4,0xFB,0xC2,0xCA,0xCE,0xD4,0xDB};

/* ─── O ESTILO LÊ-SE UMA VEZ ────────────────────────────────────────────────────────
 *
 * O `espaco_titulo` e o `regua_do_comando` abriam o `estilo.tex` e liam um megabyte A CADA
 * CHAMADA — e são chamados por cada título. Com 185 títulos, duas chamadas cada e três
 * passagens do sumário, são mais de mil leituras do mesmo ficheiro.
 *
 * MEDIDO: `real 3,12s` com `user+sys 1,41s` — 1,7 segundos que não eram CPU nem sistema.
 * Isso é ESPERA, e o que espera é o disco. Ler o mesmo ficheiro mil vezes não é lento por
 * ser muito trabalho: é lento por ser trabalho REPETIDO, e o repetido não acrescenta nada
 * — é dissipação com outro rosto. */
static char *le_tudo(const char *nome, long *n);   /* a leitura única passa por AQUI, sem malloc próprio */

/* O estilo lê-se UMA vez, mas sem cache com estado em `.bss`: os três valores são
 * FUNÇÃO-ESTÁTICOS (o disco do tradutor zera-os, o nativo também), e a leitura delega no
 * `le_tudo` --- não há segundo `malloc`, nem ponteiro global `= NULL` que o tradutor recuse. */
/* O estilo lê-se UMA vez por composição — sem cache entre documentos (neuronio: ausência). */
static char *ESTILO_BUF;
static long  ESTILO_LN;
static int   ESTILO_LIDO;
static const char *estilo_texto(long *n){
    if(!ESTILO_LIDO){
        ESTILO_LIDO = 1;
        /* o estilo entra pela COSTURA (g_carrega), no slot 0 (tex_estilo.bin) --- sem malloc
         * próprio: nativo faz fopen+fread para o slot, wasm aponta-o ao slot pré-carregado. */
        long r = g_carrega("../estilo.tex", 0, 1 << 16);
        if(r < 0) r = g_carrega("estilo.tex", 0, 1 << 16);
        if(r >= 0){ ESTILO_BUF = disco_buf(0, 1 << 16); ESTILO_LN = r; }
    }
    if(n) *n = ESTILO_LN;
    return ESTILO_BUF;
}

static char *le_tudo(const char *nome, long *n);
static int utf8_glifo(const unsigned char *s, int *cons);
/* a inversa do `winansi_para_unicode`: o ficheiro está em UTF-8 e o PDF escreve WinAnsi */
static int unicode_para_winansi(int u){
    if(u < 0x100) return u;                 /* Latin-1 é WinAnsi de 0xA0 a 0xFF */
    switch(u){
        case 0x2014: return 0x97; case 0x2013: return 0x96;
        case 0x201C: return 0x93; case 0x201D: return 0x94;
        case 0x2018: return 0x91; case 0x2019: return 0x92;
        case 0x266A: return 0x81;           /* ♪ — slot livre do WinAnsi (0x81) */
        default: return '?';
    }
}

/* ─── OS NOMES DO IDIOMA: DENTRO DO SISTEMA ─────────────────────────────────────────
 *
 * O Aarão: «que dependência de TeX Live, se estamos a fazer um interpretador?»
 *
 * Tem razão, e era absurdo: eu chamava `kpsewhich` para ir buscar o `.ldf` do babel e o
 * `size11.clo` da classe. Um interpretador que precisa do TeX Live instalado para correr
 * não substitui coisa nenhuma — e o `popen` custava 76 ms cada, três vezes.
 *
 * Os valores foram extraídos UMA VEZ e vivem em `lib/classe/`. É a mesma decisão das
 * fontes: o que o sistema precisa está no sistema. */


/* os contadores — e a reposição, que é o que faz `1.1` voltar a `2.1` no capítulo seguinte */
static long C_PARTE = 0, C_CAP = 0, C_SEC = 0, C_SUB = 0, C_SSUB = 0;

/* o romano, derivado — não uma tabela de trinta entradas */
static void romano(long v, char *o, size_t cap){
    static const int  V[] = { 1000,900,500,400,100,90,50,40,10,9,5,4,1 };
    static const char *S[] = { "M","CM","D","CD","C","XC","L","XL","X","IX","V","IV","I" };
    size_t k = 0; o[0] = 0;
    for(int i = 0; i < 13 && k + 4 < cap; i++)
        while(v >= V[i]){ size_t l = strlen(S[i]); memcpy(o + k, S[i], l); k += l; o[k] = 0; v -= V[i]; }
}

/* a formatação inteira em char* --- o snprintf que o tradutor não tem. Anexam e devolvem o novo fim. */
static char *ap_str(char *o, const char *s){ while(*s) *o++ = *s++; return o; }
static char *ap_num(char *o, long v){
    if(v < 0){ *o++ = '-'; v = -v; }
    char t[24]; int n = 0;
    if(v == 0) t[n++] = '0'; else while(v){ t[n++] = (char)('0' + v % 10); v /= 10; }
    while(n) *o++ = t[--n];
    return o;
}

/* o CAMINHO da numeração: C_CAP.C_SEC.C_SUB.C_SSUB até `niveis` --- uma operação, não três ramas
 * repetidas. É a descida em números: cada nível anexa o seu contador, e o ponto é o separador. */
static char *num_caminho(char *p, int niveis){
    long c[4]; c[0] = C_CAP; c[1] = C_SEC; c[2] = C_SUB; c[3] = C_SSUB;   /* atribui: o traduz não inicializa array local não-static */
    for(int i = 0; i < niveis; i++){ if(i) p = ap_str(p, "."); p = ap_num(p, c[i]); }
    return p;
}

/* o rótulo do título: devolve o comprimento escrito em `o`, ou 0 se não se numera */
static int rotulo_seccao(const char *cmd, int estrela, char *o, size_t cap){
    (void)cap;                                  /* os rótulos são curtos; o buffer é folgado */
    o[0] = 0;
    if(estrela) return 0;                       /* `\chapter*` não numera nem incrementa */
    char *p = o;
    if(!strcmp(cmd, "part")){
        char r[16]; C_PARTE = C_PARTE + 1; romano(C_PARTE, r, sizeof r);
        p = ap_str(p, NOME_PARTE); if(NOME_PARTE[0]) p = ap_str(p, " "); p = ap_str(p, r);
    } else if(!strcmp(cmd, "chapter")){
        C_CAP = C_CAP + 1; C_SEC = 0; C_SUB = 0; C_SSUB = 0;
        p = ap_str(p, NOME_CAP); if(NOME_CAP[0]) p = ap_str(p, " "); p = ap_num(p, C_CAP);
    } else if(!strcmp(cmd, "section")){
        C_SEC = C_SEC + 1; C_SUB = 0; C_SSUB = 0;      p = num_caminho(p, 2);
    } else if(!strcmp(cmd, "subsection")){
        C_SUB = C_SUB + 1; C_SSUB = 0;                 p = num_caminho(p, 3);
    } else if(!strcmp(cmd, "subsubsection")){
        C_SSUB = C_SSUB + 1;                           p = num_caminho(p, 4);
    } else return 0;
    *p = 0;
    return (int)(p - o);
}

/* a mesma conta SEM mover os contadores — para o sumário ler o rótulo que o título vai ter */
static int rotulo_seccao_ver(const char *cmd, int estrela, char *o, size_t cap){
    long a = C_PARTE, b = C_CAP, c = C_SEC, d = C_SUB, e2 = C_SSUB;
    int r = rotulo_seccao(cmd, estrela, o, cap);
    C_PARTE = a; C_CAP = b; C_SEC = c; C_SUB = d; C_SSUB = e2;
    /* NO SUMÁRIO o capítulo é só o NÚMERO, como no pdflatex: «Capítulo N» é o rótulo do
     * CORPO; o índice põe «N  Título». (A parte mantém «Parte I»; as secções já são «N.M».)
     * Tira-se o prefixo «NOME_CAP » da entrada do sumário, sem libc --- só `strcmp`, que já
     * se usa aqui. Media-se: o nosso índice trazia «Capítulo N» e o pdflatex não, e a
     * sequência saía a dobrar (296 contra 148, TOC+corpo contra corpo). */
    if(!estrela && !strcmp(cmd, "chapter") && NOME_CAP[0]){
        int L = 0; while(NOME_CAP[L]) L++;
        int igual = (o[0] != 0);
        for(int k = 0; k < L && igual; k++) if(o[k] != NOME_CAP[k]) igual = 0;
        if(igual && o[L] == ' '){
            int w = 0, rd = L + 1;
            while(o[rd]) o[w++] = o[rd++];
            o[w] = 0; r = w;
        }
    }
    return r;
}


static int sec_nivel(const char *cmd){
    for(int i = 0; i < NSECS; i++) if(!strcmp(cmd, SECS[i].nome)) return SECS[i].nivel;
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * §X2  O LÉXICO — comando -> glifo
 *
 * O traduz.c §T1: "o léxico no banco: palavra por palavra, e nada mais." Aqui a palavra é o
 * comando e a tradução é o glifo. A fonte Symbol é uma das catorze de base do PDF — não se embute
 * nada, e o grego e a matemática saem sem um único byte de fonte no ficheiro.
 * ───────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *cmd; int glifo; int simb; } Par;   /* simb=1 -> fonte Symbol */

static const Par LEXICO[] = {
    /* o grego minúsculo: a Symbol põe-no no lugar da latina correspondente */
    {"alpha",'a',1},{"beta",'b',1},{"gamma",'g',1},{"delta",'d',1},{"epsilon",'e',1},
    {"varepsilon",'e',1},{"zeta",'z',1},{"eta",'h',1},{"theta",'q',1},{"iota",'i',1},
    {"kappa",'k',1},{"lambda",'l',1},{"mu",'m',1},{"nu",'n',1},{"xi",'x',1},{"pi",'p',1},
    {"rho",'r',1},{"sigma",'s',1},{"tau",'t',1},{"upsilon",'u',1},{"phi",'f',1},
    {"varphi",'j',1},{"chi",'c',1},{"psi",'y',1},{"omega",'w',1},
    /* e o maiúsculo */
    {"Gamma",'G',1},{"Delta",'D',1},{"Theta",'Q',1},{"Lambda",'L',1},{"Xi",'X',1},
    {"Pi",'P',1},{"Sigma",'S',1},{"Phi",'F',1},{"Psi",'Y',1},{"Omega",'W',1},
    /* os operadores e as relações */
    /* o × e o · EXISTEM no WinAnsi da fonte embutida — saem do desenho do documento,
     * não da Symbol de fora; o ± e o ÷ também (0xB1, 0xF7 do WinAnsi) */
    {"times",0xD7,0},{"cdot",0xB7,0},{"pm",0xB1,0},{"div",0xF7,0},
    {"le",0xA3,1},{"leq",0xA3,1},{"ge",0xB3,1},{"geq",0xB3,1},{"ne",0xB9,1},{"neq",0xB9,1},
    {"equiv",0xBA,1},{"approx",0xBB,1},{"sim",0x7E,1},{"propto",0xB5,1},
    {"in",0xCE,1},{"notin",0xCF,1},{"subset",0xCC,1},{"subseteq",0xCD,1},{"supset",0xC9,1},
    {"cup",0xC8,1},{"cap",0xC7,1},{"emptyset",0xC6,1},{"forall",0x22,1},{"exists",0x24,1},
    {"to",0xAE,1},{"rightarrow",0xAE,1},{"mapsto",0xAE,1},{"leftarrow",0xAC,1},
    {"uparrow",0xAD,1},{"downarrow",0xAF,1},
    {"Rightarrow",0xDE,1},{"Leftarrow",0xDC,1},{"leftrightarrow",0xAB,1},{"Leftrightarrow",0xDB,1},
    {"infty",0xA5,1},{"partial",0xB6,1},{"nabla",0xD1,1},{"sqrt",0xD6,1},
    {"sum",0xE5,1},{"prod",0xD5,1},{"int",0xF2,1},
    {"langle",0xE1,1},{"rangle",0xF1,1},{"oplus",0xC5,1},{"otimes",0xC4,1},
    {"perp",0x5E,1},{"top",0xA1,1},{"angle",0xD0,1},{"cong",0x40,1},{"aleph",0xC0,1},
    {"star",0x2A,1},{"circ",0xB0,1},{"bullet",0xB7,1},{"ldots",0xBC,1},{"dots",0xBC,1},
    {"cdots",0xA2,1},{"vdots",0xA4,1},{"ddots",0xA6,1},{"dotsb",0xA2,1},{"dotsc",0xBC,1},
    {"dotsm",0xA2,1},{"dotsi",0xA2,1},{"dotso",0xBC,1},
    {"Re",0xC2,1},{"Im",0xC1,1},{"wp",0xC3,1},{"neg",0xD8,1},{"wedge",0xD9,1},{"vee",0xDA,1},
    /* setas longas: o papel usa \longrightarrow (corpo_topologico); a Symbol só tem a curta —
     * a geometria do átomo Rel é a mesma, o glifo é o que há */
    {"longrightarrow",0xAE,1},{"longleftarrow",0xAC,1},
    {"Longrightarrow",0xDE,1},{"Longleftarrow",0xDC,1},
    /* e os que são da própria latina */
    {"{",'{',0},{"}",'}',0},{"$",'$',0},{"%",'%',0},{"&",'&',0},{"_",'_',0},{"#",'#',0},
    /* os do modo matemático que a latina realiza: o `\colon` é o dois-pontos tipado
     * (T\colon V\to V perdia os dois pontos), o `\dagger` é o punhal do WinAnsi (0x86),
     * e o \quad/\qquad são espaço — espaçamento SOMA, e um comando comido sem espaço COLA */
    {"colon",':',0},{"dagger",0x86,0},{"backslash",'\\',0},{"mid",'|',0},
    /* partitura: o ♪ já está na OTF (musicalnote); o léxico só mapeia o comando → glifo,
     * como o grego. Slot WinAnsi 0x81 ↔ U+266A (ver winansi_para_unicode). */
    {"note",0x81,0},{"musicnote",0x81,0},{"quarternote",0x81,0},
};
#define NLEX ((int)(sizeof LEXICO / sizeof LEXICO[0]))


/* ─── AS LIGADURAS SÃO UMA TABELA, como o léxico ─────────────────────────────────────
 *
 * O Aarão: «o travessão é caractere igual aos demais.» E é --- o que não era igual era o
 * TRATAMENTO: eu tinha escrito o mesmo `if(g=='-' && s[i+1]=='-')` em DOIS laços, o do
 * texto e o dos títulos, e por isso o travessão funcionava num sítio e não no outro até
 * eu copiar o `if` para lá. Um caractere que precisa de um `if` em cada sítio por onde
 * passa não está no sítio certo.
 *
 * Aqui é uma tabela consultada por quem compõe, e vale para todos os laços de uma vez.
 * Entram também as aspas do TeX, que estavam simplesmente em falta: ``texto'' saía com
 * as crases e as plicas literais. */
static const struct { const char *ent; int glifo; } LIGA[] = {
    { "---", 0x97 },   /* travessão   — em WinAnsi, que é o que o PDF escreve */
    { "--",  0x96 },   /* traço-de-N */
    { "``",  0x93 },   /* aspa dupla a abrir */
    { "''",  0x94 },   /* e a fechar */
    { "`",   0x91 },   /* aspa simples a abrir */
};
#define N_LIGA ((int)(sizeof LIGA / sizeof LIGA[0]))

/* a ligadura que começa em `s+i`, ou 0. `*consome` diz quantos bytes ela gasta. */
static int liga_acha(const char *s, long n, long i, int *consome){
    for(int t = 0; t < N_LIGA; t++){
        long l = (long)strlen(LIGA[t].ent);
        if(i + l <= n && !memcmp(s + i, LIGA[t].ent, (size_t)l)){ *consome = (int)l; return LIGA[t].glifo; }
    }
    *consome = 0; return 0;
}

static const Par *lex_acha(const char *cmd){
    for(int i = 0; i < NLEX; i++) if(!strcmp(cmd, LEXICO[i].cmd)) return &LEXICO[i];
    return NULL;
}
/* a VOLTA: a mesma tabela lida ao contrário — glifo -> comando. Não é outra tabela; é a de cima
 * percorrida do outro lado, que é o que o traduz.c §T4 faz com o léxico das palavras. */
static const char *lex_volta(int glifo, int simb){
    for(int i = 0; i < NLEX; i++)
        if(LEXICO[i].glifo == glifo && LEXICO[i].simb == simb) return LEXICO[i].cmd;
    return NULL;
}

/* UTF-8 -> WinAnsi (Latin-1). O .tex vem em UTF-8 e a fonte de base fala Latin-1: os acentos
 * do português cabem todos, e o que não couber sai como '?' em vez de sair partido em dois bytes. */
static int utf8_glifo(const unsigned char *s, int *consumido){
    if(s[0] < 0x80){ *consumido = 1; return s[0]; }
    if((s[0] & 0xE0) == 0xC0 && (s[1] & 0xC0) == 0x80){
        *consumido = 2;
        int cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        return cp <= 0xFF ? cp : '?';
    }
    if((s[0] & 0xF0) == 0xE0){
        *consumido = 3;
        int cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        switch(cp){                                   /* os poucos que valem a pena mapear */
            case 0x2018: return 0x91; case 0x2019: return 0x92;
            case 0x201C: return 0x93; case 0x201D: return 0x94;
            case 0x2013: return 0x96; case 0x2014: return 0x97;
            case 0x2026: return 0x85; case 0x00A0: return ' ';
            case 0x266A: return 0x81;                 /* ♪ musicalnote — na OTF do documento */
        }
        return '?';
    }
    *consumido = 1; return '?';
}

/* ─────────────────────────────────────────────────────────────────────────────
 * §X3  O PRISMÁTICO — encher a área
 *
 * O prisma.c: "a cifra real é o triângulo; precisamos deformá-lo de modo a PREENCHER A ÁREA
 * INTEIRA". A linha de texto é exatamente isso: chega com um comprimento qualquer e tem de encher
 * a largura da coluna. Mede-se a linha com as larguras reais dos glifos e deforma-se o espaço até
 * a área fechar — o resíduo da justificação é o que sobra da deformação, e tem de ser 0.
 * ───────────────────────────────────────────────────────────────────────────── */

/* as larguras da Helvetica e da Helvetica-Bold, em milésimos de em, de 32 a 126.
 * São a métrica publicada das fontes de base do PDF — sem elas não se mede a linha, e sem medir a
 * linha não se enche a área. */
static const short W_REG[95] = {
 278,278,355,556,556,889,667,191,333,333,389,584,278,333,278,278,
 556,556,556,556,556,556,556,556,556,556,278,278,584,584,584,556,
1015,667,667,722,722,667,611,778,722,278,500,667,556,833,722,778,
 667,778,722,667,611,722,667,944,667,667,611,278,278,278,469,556,
 333,556,556,500,556,556,278,556,556,222,222,500,222,833,556,556,
 556,556,333,500,278,556,500,722,500,500,500,334,260,334,584};
static const short W_NEG[95] = {
 278,333,474,556,556,889,722,238,333,333,389,584,278,333,278,278,
 556,556,556,556,556,556,556,556,556,556,333,333,584,584,584,611,
 975,722,722,722,722,667,611,778,722,278,556,722,611,833,722,778,
 667,778,722,667,611,722,667,944,667,667,611,333,278,333,584,556,
 333,556,611,556,611,556,333,611,611,278,278,556,278,889,611,611,
 611,611,389,556,333,611,556,778,556,556,500,389,280,389,584};

#define F_REG 0
#define F_NEG 1
/* A ORDEM É A DA TORRE, e a Symbol vem DEPOIS. `CARTAS[]` abre regular, negra, itálica e
 * versaletes por esta ordem — os eixos peso, inclinação e caixa —, e ter a Symbol no 2
 * fazia `carta_do_corpo(F_SIM,...)` devolver a ITÁLICA. Uma colisão de índices, e o
 * sintoma era «Enredo» em minúsculas onde o gabarito tem ENREDO em versaletes. */
#define F_ITA 2                                   /* a itálica  — a estaca da INCLINAÇÃO */
#define F_VER 3                                   /* versaletes — a estaca da CAIXA */
#define F_MON 4                                   /* monoespaçada — a estaca da LARGURA:
                                                   * todos os glifos com a mesma caixa */
#define F_SIM 5                                   /* a Symbol, que não tem par */
/* a itálica MATEMÁTICA — as VARIÁVEIS. Não é a itálica do texto: a de texto tem o V
 * com bearing 209 e avanço 743; a matemática (o cmmi do gabarito) tem 56 e 583. Era
 * o «End( V)» com buraco: a letra certa medida pela régua errada. */
#define F_MAT 6
/* e a variável em contexto NEGRO (o \medido, o \textbf com fórmula dentro): o
 * alfabeto bold italic da LM Math — os dígitos já herdavam a negra, as letras
 * desviavam para a F_MAT e perdiam o peso */
#define F_MTB 7
/* e a SÍMBOLO NEGRA: o grego bold REAL da referência (U+1D6C2…) — o \pi de um
 * \medido saía no peso normal */
#define F_SMB 8
/* e a NEGRA ITÁLICA de texto — a referência lmroman10-bolditalic tal e qual:
 * o \emph dentro de um contexto negro compõe bold italic, não perde o peso */
#define F_NIT 9
#define N_FONTES 5

/* A CARTA, aberta uma vez. Se a fonte estiver no sistema a largura vem da CURVA; se não estiver,
 * cai na tabela — e isso é dito na saída, nunca em silêncio, porque as duas não medem o mesmo
 * para os acentuados (a tabela não os tem, e eu punha 556 a olho). */
/* AS CARTAS SÃO UMA TABELA, e não duas variáveis. O interpretador tinha `CARTA_R` e
 * `CARTA_N` — duas fontes fixas, e um terceiro slot para a Symbol — e por isso não havia
 * onde pôr a itálica nem os versaletes que o corpo do `\gkcapa` pede. Uma variante nova
 * exigia uma variável nova, um `if` novo e um `/F` novo à mão.
 *
 * Aqui é um vector: abre-se o que for preciso, e o índice É o número da fonte no PDF. */
#define MAX_CARTA 10
static Ttf CARTAS[MAX_CARTA];
static const char *CARTA_NOME[MAX_CARTA];
static int N_CARTA = 0;
static int CARTA_SIM = 0;      /* a símbolo (CM) abriu? senão, W_SIM e a Symbol de fora */
static int CARTA_MAT = 0;
static int CARTA_MTB = 0;      /* a itálica matemática NEGRA abriu? */
static int CARTA_SMB = 0;      /* a símbolo NEGRA (o grego bold) abriu? */
static int CARTA_NIT = 0;      /* a negra itálica de texto abriu? */      /* a itálica matemática (as variáveis) abriu? senão, F_ITA */
#define CARTA_R (CARTAS[0])
#define CARTA_N (CARTAS[1])
static int  CARTA = 0;
static int  FONTE_OTF = 0;
static long EMBP[64] = {0};   /* o FontDescriptor de cada PAR (variante, corpo) */   /* a fonte embutida é OpenType (CFF), não TrueType */

/* ─── O DESENHO É DO CORPO: um sistema vivo não tem as caixas todas iguais ───────────
 *
 * Eu abria UM desenho — o de 10 pt — e escalava-o para todos os corpos. Na Computer
 * Modern isso está errado por construção: cada tamanho tem o seu desenho, com traço e
 * largura próprios. MEDIDO contra o gabarito, todas as palavras saíam mais largas:
 * «Dourado» +3,6%, «Kingdom» +7,0%, «xadrez:» +5,3% --- sistematicamente, que é a marca
 * de uma régua e não de um erro.
 *
 * Aqui abre-se um por corpo. O `DESENHOS` do spline.h diz qual, e o índice é o par
 * (variante, corpo) --- que é a assinatura da torre com mais um eixo. */
#define MAX_DES 24
#define TAM_DES  ((long)(MAX_DES * sizeof(Ttf)))
#define DES_C   ((Ttf*)disco_buf(10, TAM_DES + 32768))  /* +Est (compila), fora do SP */
static long DES_CORPO[MAX_DES];                    /* mantissas 10^-3, a régua do Tf */
static int DES_VAR[MAX_DES], N_DES = 0;
static long CORPO_CORRENTE = 0;

/* a carta para esta variante e este corpo, abrindo-a se ainda não estiver aberta */
static const Ttf *carta_do_corpo(int variante, long corpo){
    if(corpo <= 0) return &CARTAS[variante < N_CARTA ? variante : 0];
    for(int i = 0; i < N_DES; i++)
        if(DES_VAR[i] == variante && DES_CORPO[i] > corpo - 10 && DES_CORPO[i] < corpo + 10)
            return &DES_C[i];
    if(N_DES >= MAX_DES) return &CARTAS[variante < N_CARTA ? variante : 0];
    const char *nome = spline_por_corpo(corpo, variante);
    /* a régua do caminho: NÃO no quadro (o traduz + `char*[2]` local partia o bx). */
    static char c1[256], c2[256]; char *pp;
    pp = ap_str(c1, "lib/fontes/");    pp = ap_str(pp, nome); *pp = 0;
    pp = ap_str(c2, "../lib/fontes/"); pp = ap_str(pp, nome); *pp = 0;
    if(!ttf_abre(&DES_C[N_DES], c1) && !ttf_abre(&DES_C[N_DES], c2))
        return &CARTAS[variante < N_CARTA ? variante : 0];
    DES_CORPO[N_DES] = corpo; DES_VAR[N_DES] = variante; N_DES++;
    return &DES_C[N_DES - 1];
}                            /* 0 = tabela, 1 = curva */

static int CARTA_TENTADO = 0;   /* 1 bit (neuronio): presença abre; ausência (=0) a cada compila */
static void carta_abre(void){
    if(CARTA_TENTADO) return;
    CARTA_TENTADO = 1;
    CARTA = spline_abre_alguma(&CARTAS[0], SPLINE_REG, SPLINE_NCAND, &CARTA_NOME[0])
         && spline_abre_alguma(&CARTAS[1], SPLINE_NEG, SPLINE_NCAND, &CARTA_NOME[1]);
    N_CARTA = CARTA ? 2 : 0;
    /* e as restantes variantes, se estiverem: a itálica e os versaletes. Não é um caso
     * especial de cada uma — é o mesmo `abre` com outro ficheiro. */
    if(CARTA){
        static const char *IT[] = { "lib/fontes/documento-italica.otf",
                                    "../lib/fontes/documento-italica.otf" };
        static const char *CC[] = { "lib/fontes/documento-versalete.otf",
                                    "../lib/fontes/documento-versalete.otf" };
        if(spline_abre_alguma(&CARTAS[N_CARTA], IT, 2, &CARTA_NOME[N_CARTA])) N_CARTA++;
        if(spline_abre_alguma(&CARTAS[N_CARTA], CC, 2, &CARTA_NOME[N_CARTA])) N_CARTA++;
        static const char *MO[] = { "lib/fontes/documento-mono.otf",
                                    "../lib/fontes/documento-mono.otf" };
        if(spline_abre_alguma(&CARTAS[N_CARTA], MO, 2, &CARTA_NOME[N_CARTA])) N_CARTA++;
        if(N_CARTA < 6) N_CARTA = 6;
        /* e a SIMBOLO: o desenho CM do gabarito (a LM Math), com o cmap posto NA POSIÇÃO
         * dos códigos que o léxico usa — a mesma régua, a roupa certa. Gerada uma vez
         * (lib/fontes/documento-simbolo.otf) e embutida como as outras. */
        { static const char *SI[] = { "lib/fontes/documento-simbolo.otf",
                                      "../lib/fontes/documento-simbolo.otf" };
          if(spline_abre_alguma(&CARTAS[F_SIM], SI, 2, &CARTA_NOME[F_SIM])) CARTA_SIM = 1; }
        /* e as VARIÁVEIS: o alfabeto itálico matemático (U+1D434…) posto nas posições
         * ASCII, com o quadrado em (U+2003) no 0xA0 — a mesma roupa da símbolo */
        { static const char *VA[] = { "lib/fontes/documento-varia.otf",
                                      "../lib/fontes/documento-varia.otf" };
          if(spline_abre_alguma(&CARTAS[F_MAT], VA, 2, &CARTA_NOME[F_MAT])) CARTA_MAT = 1; }
        { static const char *VB[] = { "lib/fontes/documento-varia-negra.otf",
                                      "../lib/fontes/documento-varia-negra.otf" };
          if(spline_abre_alguma(&CARTAS[F_MTB], VB, 2, &CARTA_NOME[F_MTB])) CARTA_MTB = 1; }
        { static const char *SB[] = { "lib/fontes/documento-simbolo-negra.otf",
                                      "../lib/fontes/documento-simbolo-negra.otf" };
          if(spline_abre_alguma(&CARTAS[F_SMB], SB, 2, &CARTA_NOME[F_SMB])) CARTA_SMB = 1; }
        { static const char *NI[] = { "lib/fontes/documento-negra-italica.otf",
                                      "../lib/fontes/documento-negra-italica.otf" };
          if(spline_abre_alguma(&CARTAS[F_NIT], NI, 2, &CARTA_NOME[F_NIT])) CARTA_NIT = 1; }
    }
}

/* QUANTAS VEZES SE CHUTOU — e o número tem de ser ZERO num documento cuja fonte está
 * embutida. É o contador que faz o chute deixar de ser silencioso. */
static long CHUTES = 0, CHUTE_G[8], N_CHUTE_G = 0;

/* O CÓDIGO DA PÁGINA PARA UNICODE — e era aqui que os 106 chutes nasciam.
 *
 * O PDF escreve o texto em WinAnsiEncoding, onde o travessão é 151 e o meio-travessão 150.
 * Mas o `cmap` de uma TTF é indexado por UNICODE, e lá o travessão é U+2014. Procurar o 151
 * no cmap não acha nada — a fonte TEM o glifo, e eu perguntava pelo número errado.
 *
 * O resultado era o chute: 556 milésimos para um travessão que mede 1000. E como espaçar
 * SOMA, cada travessão desalinhava toda a linha a partir dali — que é exactamente o «uns
 * espaços ficaram maiores» que o Aarão viu. */
int winansi_para_unicode(int g)
{
    switch(g){
    case 0x80: return 0x20AC;  case 0x81: return 0x266A;  /* ♪ musicalnote (OTF) */
    case 0x82: return 0x201A;  case 0x83: return 0x0192;
    case 0x84: return 0x201E;  case 0x85: return 0x2026;  case 0x86: return 0x2020;
    case 0x87: return 0x2021;  case 0x88: return 0x02C6;  case 0x89: return 0x2030;
    case 0x8A: return 0x0160;  case 0x8B: return 0x2039;  case 0x8C: return 0x0152;
    case 0x8E: return 0x017D;  case 0x91: return 0x2018;  case 0x92: return 0x2019;
    case 0x93: return 0x201C;  case 0x94: return 0x201D;  case 0x95: return 0x2022;
    case 0x96: return 0x2013;  case 0x97: return 0x2014;  case 0x98: return 0x02DC;
    case 0x99: return 0x2122;  case 0x9A: return 0x0161;  case 0x9B: return 0x203A;
    case 0x9C: return 0x0153;  case 0x9E: return 0x017E;  case 0x9F: return 0x0178;
    default:   return g;
    }
}

/* AS LARGURAS DA SYMBOL, publicadas — sao das catorze fontes de base do PDF, como as da
 * Helvetica que ja' estao acima. Nao se inventam: estao na especificacao.
 *
 * Eu dava 549 a TODA a Symbol, e a seta (174) mede 987 — erro de 438 milesimos, quase
 * metade de um em, e ela aparece 604 vezes no catalogo. As 642 invasoes que restavam
 * envolviam TODAS um simbolo: 100%.
 *
 * E o erro pequeno PROPAGA-SE: e' o caos. Por isso o residuo tem de ser ZERO — num sistema
 * reversivel um residuo nao-nulo nao fica pequeno, cresce. */
static const short W_SIM[224] = {
    250,333,713,500,549,833,778,439,333,333,500,549,
    250,549,250,278,500,500,500,500,500,500,500,500,
    500,500,278,278,549,549,549,444,549,722,667,722,
    612,611,763,603,722,333,631,722,686,889,722,722,
    768,741,556,592,611,690,439,768,645,795,611,333,
    863,333,658,500,500,631,549,549,494,439,521,411,
    603,329,603,549,549,576,521,549,549,521,549,603,
    439,576,713,686,493,686,494,480,200,480,549,0,
    0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,620,247,549,
    167,713,500,753,753,753,753,1042,987,603,987,603,
    400,549,411,549,549,713,494,460,549,549,549,549,
    1000,603,1000,658,823,686,795,987,768,768,823,768,
    768,713,713,713,713,713,713,713,768,713,790,790,
    890,823,549,250,713,603,603,1042,987,603,987,603,
    494,329,790,790,786,713,384,384,384,384,384,384,
    494,494,494,494,0,329,274,686,686,686,384,384,
    384,384,384,384,494,494,494,0,
};

/* a largura para a TABELA /Widths: um codigo que o WinAnsi nao define nao e' um chute — e'
 * uma casa vazia da tabela, e vale 0 porque nunca sera' desenhada. Contar isto como chute era
 * acusar a emissao da tabela pelo que a codificacao nao tem. */
static int largura(int g, int fonte);
/* a largura com o corpo EXPLICITO: a operação passa-lho, em vez de o ler de uma global.
 * O `CORPO_CORRENTE` era estado escondido — e estado escondido é o sítio onde as duas
 * réguas nascem, seis vezes num dia. */
static int largura_de(int g, int fonte, long corpo){
    long guarda = CORPO_CORRENTE;
    CORPO_CORRENTE = corpo;
    int w = largura(g, fonte);
    CORPO_CORRENTE = guarda;
    return w;
}
/* a largura de um glifo em MILÉSIMOS de em, da carta da fonte: avanço · 1000 / upem. É a conta que
 * se repetia por todo o lado (a régua da fonte); agora uma só. */
static int avanco_mil(const Ttf *t, int gi){ return (int)((long)ttf_avanco(t, gi) * 1000 / t->upem); }

static int largura(int g, int fonte){
    /* Claves: avanço Emmentaler já em Td (10^-3 pt).
     * NÃO multiplicar por corpo em mede(); desenrola usa o mesmo Td.
     * (sem função nova — MAX_FUN=256 no wasm) */
    if(g == 0x82) return 11266;
    if(g == 0x83) return 11775;
    if(g == 0x84) return 10000;
    /* O ESPACO E' UMA LETRA, e tem caixa como qualquer outra: mede 277 na Liberation e 260
     * noutra fonte — nao e' um vazio sem largura nem um numero a parte. Por isso vem da FONTE,
     * como o `o` vem, e nao de uma constante.
     *
     * E era aqui que ele se perdia: `if(fonte == F_SIM) return 549` devolvia 549 para TUDO na
     * Symbol, incluindo o espaco. O espaco da Symbol mede 250, e eu dava-lhe 549 — mais do
     * dobro. Uma constante para uma fonte inteira e' o chute outra vez, com outro rosto. */
    if(fonte == F_MAT || fonte == F_MTB){
        carta_abre();
        /* as variáveis: as posições da carta SÃO os códigos, como na símbolo */
        if(fonte == F_MTB && CARTA_MTB){
            int gi = ttf_glifo(&CARTAS[F_MTB], g);
            if(gi) return avanco_mil(&CARTAS[F_MTB], gi);
        }
        if(CARTA_MAT){
            int gi = ttf_glifo(&CARTAS[F_MAT], g);
            if(gi) return avanco_mil(&CARTAS[F_MAT], gi);
        }
        return largura(g, F_ITA);                  /* sem a carta, a itálica do texto */
    }
    if(fonte == F_NIT){
        carta_abre();
        if(CARTA_NIT){
            int gi = ttf_glifo(&CARTAS[F_NIT], winansi_para_unicode(g));
            if(gi) return avanco_mil(&CARTAS[F_NIT], gi);
        }
        return largura(g, F_NEG);                  /* sem a carta, a negra romana */
    }
    if(fonte == F_SMB){
        carta_abre();
        if(CARTA_SMB){
            int gi = ttf_glifo(&CARTAS[F_SMB], g);
            if(gi) return avanco_mil(&CARTAS[F_SMB], gi);
        }
        return largura(g, F_SIM);                  /* sem a negra, o peso normal */
    }
    if(fonte == F_SIM){
        carta_abre();
        /* a símbolo do documento (o desenho CM): as posições da carta SÃO os códigos */
        if(CARTA_SIM){
            int gi = ttf_glifo(&CARTAS[F_SIM], g);
            if(gi) return avanco_mil(&CARTAS[F_SIM], gi);
        }
        /* o espaco existe em qualquer fonte, e a largura dele le-se */
        if(g == ' ' && CARTA) return avanco_mil(&CARTA_R, ttf_glifo(&CARTA_R, ' '));
        /* e os simbolos: a largura PUBLICADA da Symbol, e nao um numero para todos */
        return (g >= 32 && g <= 255) ? W_SIM[g - 32] : 0;
    }
    if(g > 0 && g < 32) return 0;    /* controlo (\n, e os separadores 2/3 da matriz): sem caixa */
    carta_abre();
    if(CARTA){
        /* A CARTA E' A DO PAR (variante, corpo), e nao um ternario entre duas.
         *
         * Aqui estava `(fonte == F_NEG) ? &CARTA_N : &CARTA_R` — que devolve a REGULAR
         * para tudo o que nao e' negra, incluindo a italica e os versaletes. As larguras
         * saiam da regular e o glifo desenhado vinha do versalete, que e' mais largo: o
         * «ENREDO» tinha o RED colado. MEDIDO: 735 declarado onde o d-cc1728 tem 728.
         *
         * E' a quinta vez hoje com o mesmo rosto: DUAS REGUAS PARA O MESMO OBJECTO. Cada
         * vez que uma sobra num sitio esquecido, o sintoma e' letras por cima de letras. */
        const Ttf *t = carta_do_corpo(fonte, CORPO_CORRENTE);
        int gi = ttf_glifo(t, winansi_para_unicode(g));
        /* o glifo 0 é o .notdef: se a fonte não tem o caractere, não se inventa uma largura */
        if(gi) return avanco_mil(t, gi);
    }
    /* AQUI ESTAVA O CHUTE, e ele é o que o Aarão apontou: «você aumentou o espaço para não
     * colapsar mas outros espaços ficaram maiores». Um valor inventado para um glifo faz a
     * linha inteira desalinhar a partir dali, porque ESPAÇAR SOMA — o erro de um propaga-se a
     * todos os seguintes.
     *
     * E a tabela W_REG é da Helvetica: usá-la quando a fonte embutida é outra é medir por uma
     * régua e desenhar com outra, que foi exactamente o defeito de ontem.
     *
     * Não se tira o chute — não há por onde medir se a fonte não abriu. CONTA-SE. Um chute
     * contado é um defeito visível; um chute calado é o texto tosco sem se saber porquê. */
    CHUTES++;
    if(N_CHUTE_G < 8){
        int novo = 1;
        for(long i = 0; i < N_CHUTE_G; i++) if(CHUTE_G[i] == g) novo = 0;
        if(novo) CHUTE_G[N_CHUTE_G++] = g;
    }
    if(g < 32 || g > 126) return 556;
    return fonte == F_NEG ? W_NEG[g - 32] : W_REG[g - 32];
}

/* um pedaço de texto já traduzido: glifo + fonte. É o que o solar guarda. */
/* O GLIFO LEVA O SEU EXPOENTE, e o expoente é o corpo do número (grau 2 ou grau 4):
 * `e` carrega os DOIS sinais do par — o sinal do VALOR (aditivo: +1 sobrescrito, -1
 * subscrito, a Lei 1) e o grau da ESCALA (multiplicativo: |e|=1 é a dobra σ⁻², |e|=2 é a
 * dobra dupla σ⁻⁴ — o expoente de expoente). Zero é o texto corrido. */
typedef struct { unsigned char g; unsigned char f; signed char e; } Gl;

#define MAXLIN 4096
/* A LINHA LEVA A SUA LARGURA. O `desenrola` justificava sempre contra COL — a largura da
 * PÁGINA — e numa célula de tabela isso estica-a até à margem direita, invadindo as colunas
 * seguintes. Ele não conhece a tabela nem tem de conhecer: recebe a largura com a linha. */
typedef struct { Gl g[MAXLIN]; int n; int nivel; int recuo; int larg; long deg; int centra; } Linha;

/* ═══ A RÉGUA DO NÚMERO: VÍRGULA-FIXA NA RÉGUA DO PRÓPRIO FICHEIRO ═══════════════════
 *
 * A teoria é discreta e o número dela é o CORPO DO NÚMERO (grau 4): mantissa INTEIRA e o
 * sinal da escala — vírgula-fixa, ±m·10^±k. E a escala não é escolhida aqui: é a que o
 * PRÓPRIO FORMATO declara — o `Tf` e o `Td` do PDF escrevem TRÊS decimais, o `/Widths` é
 * inteiro por-mil. Logo toda a quantidade corre como mantissa inteira nessa régua (10^-3
 * de ponto), as contas fazem-se por PRODUTO CRUZADO de inteiros, e a única divisão é a
 * que a régua do ficheiro força AO ESCREVER — uma por quantidade escrita, o único
 * arredondamento, o correcto. Dividir no meio da conta amputa, e o amputado propaga. */
#define EIXO_ESCALA  1
#define EIXO_ESPACO  -1
#define EIXO_LARGURA 0
static long escala_de_degrau(long degrau, int eixo);

/* O EXPOENTE VIA CORPO, GRAU 2 OU GRAU 4, PELA DOURADA. O sobrescrito é o MESMO corpo com
 * o degrau composto: a escala é a dourada (base·φ^(k/3), os degraus do estilo), e a dobra
 * é a razão de DOIS DEGRAUS DA PRÓPRIA TABELA por produto cruzado — grau 2: E[1]/E[3]
 * (σ⁻²); grau 4: E[0]/E[4] (σ⁻⁴). (Os índices evitam o degrau 2, que é o \normalsize da
 * classe.) A divisão única produz o corpo que o `Tf` ESCREVE — é a régua do ficheiro a
 * decidir, não uma perda no meio. E o DESLOCAMENTO é o dual aditivo do salto
 * multiplicativo: o que a escala tira, o espaço recebe, com o sinal da Lei 1. */
/* ─── A CORTE: ONDE CADA RÉGUA SE APLICA ─────────────────────────────────────────────
 *
 * A arquitectura deste motor é a corte de fora do centro, e cada camada tem o
 * seu dono — a revisão é esta, e quem tocar numa camada consulta a régua dela:
 *
 *   ADA (⊕, n=2⊗6 — gerar, o Corpo Criativo): o eixo ADITIVO. Tudo o que SOMA:
 *   as translações (kern, subida, entrelinha, as bandas fronteira-a-fronteira)
 *   e a IDA-E-VOLTA (o dual, n=2): a volta byte a byte, o refaz — costuradas
 *   pela interface (n=6, o relógio que desenha o rasto).
 *
 *   PENNY (φ ⊕, n=5⊗6 — estruturar, a cifra áurea): o eixo MULTIPLICATIVO.
 *   Tudo o que ESCALA: a escala dourada do estilo (φ^{k/3}, o degrau é o
 *   expoente), a razão da espiral (E1/E3), os corpos por nível — o ponto fixo
 *   (n=5, o i) casado com a interface (n=6).
 *
 *   ALONZO (z↦z²+c, o Y — a composição pura): a COMPOSIÇÃO. Os giros
 *   compostos (f∘f), os estados-trajetória da espiral, as regiões dentro de
 *   regiões (caixa, fronteira, matriz, tabela, underbrace — o X14 é dele), e
 *   o ponto fixo da composição: o i, a rotação dimensional (a pental, que
 *   Alonzo comanda).
 *
 *   CAELUM (⊗, n=8⊗5 — o esqueleto): a MALHA QUE AGUENTA PESO. As estruturas
 *   (Gl, as corridas, os vãos, as tabelas-região, os XObjects), e a
 *   transformada da lei 8 (a NTT em Z_65537, N=2^8 — a assinatura inteira,
 *   tests/relogio_curva §R8) sobre a escala áurea (n=5).
 *
 * E o selo COMUM da corte é FP=1 — o fator de potência unitário, |det|=1, a
 * conservação da área (o X13): nenhuma régua da corte a viola, e é por isso
 * que as quatro compõem sem se pisarem. Uma lei espacial; múltiplas
 * realizações — e cada realização tem dono. */

/* ─── A SEMENTE: A CONFIGURAÇÃO DA ESTRELA ───────────────────────────────────────────
 *
 * Todos os espaçamentos e proporções da tipografia derivam DESTA semente pela
 * espiral — o respiro é a dobra sobre SEM_RESP, a caixa natural é a ascendente
 * e a descendente, o traço é SEM_TRACO no peso regular (e engrossa pela razão
 * da própria referência). Mudar a semente muda o paper inteiro de uma vez: é a
 * configuração da estrela, não trinta e cinco números espalhados. A razão da
 * espiral não está aqui porque vem da ESCALA (E1/E3, a dourada) — a semente
 * da escala é o próprio estilo. E a conservação da área é a LEI: o que estica
 * num eixo declara-se no par (sh, sv) da instância, legível por quem mede. */
#define SEM_RESP    3          /* o divisor do respiro: g2 = dobra/SEM_RESP    */
#define SEM_ASC_N  17          /* a ascendente da caixa natural: 17/20 do corpo */
#define SEM_ASC_D  20
#define SEM_DESC    4          /* a descendente: corpo/SEM_DESC                 */
#define SEM_TRACO 400          /* a espessura do traço, no peso regular         */
/* a semente é ESTADO da estrela — os valores acima são a ORIGEM, e o estado
 * pode girar (o teste de sementes do §X13: sementes diferentes, geometrias
 * diferentes, a MESMA lei — cada trajetória conserva a sua área). O wrapper
 * pode reescrevê-la pela CONFIG (le_semente, a declaração \gksemente do
 * estilo): a semente é configuração, não constante enterrada. */
static long SEM_V[5] = { SEM_RESP, SEM_ASC_N, SEM_ASC_D, SEM_DESC, SEM_TRACO };
static long sem_resp(long d){ return d / SEM_V[0]; }
static long sem_asc(long c){ return c * SEM_V[1] / SEM_V[2]; }
static long sem_desc(long c){ return c / SEM_V[3]; }
/* o eixo do delimitador — a meia-altura da caixa natural, DERIVADA da semente:
 * (asc − desc)/2, que na origem (17/20, 1/4) dá os 3/10 que estavam escritos
 * à mão em três sítios. Gira com a semente, como tudo. */
static long sem_eixo(long c){ return (sem_asc(c) - sem_desc(c)) / 2; }

/* ─── CAIXA `\boxed`: UMA LEI (Maestro ⊕ Metrónomo) ─────────────────────────────
 * pad = dobra+respiro = λ⁺ (Maestro projecta a moldura).
 * m3  = g2(pad)       = externo H (não come a coluna).
 * No fecho/topo o Metrónomo lê a aresta e atesta λ⁺+λ⁻=0 (thm:metronomo).
 * A fórmula repete-se inline (o traduz wasm não engole macro do/while). */

/* ─── A ESPIRAL GERAL: o relógio comanda soma E multiplicação ────────────────────────
 *
 * A semente é (escala inicial, espaçamento inicial) = (o corpo, a dobra da escala
 * dourada). Cada GIRO compõe as duas operações: a escala MULTIPLICA pela razão do
 * degrau e a subida SOMA exactamente O QUE A ESCALA TIROU nesse giro — a conservação
 * é por construção, e é a mesma dos medidores (§X8). Ida e volta: o sinal do giro é
 * a direção (expoente sobe, índice desce); desfazer é repor o estado de fora.
 *
 * O estado é a TRAJETÓRIA — (nível, máscara de sinais) — independente do corpo:
 * estampa-se no Gl.e como índice 16..127 numa tabela global, sem reset. As escalas
 * derivam-se por nível PELO CAMINHO DOS DEGRAUS (nível 1 = E[1]/E[3], nível 2 =
 * E[0]/E[4] — as dobras que já eram a régua) e daí para cima pela razão composta:
 * a espiral não muda os andares medidos, PROLONGA-OS. */
#define ESP_0 16
#define TORRE_NTT_MAX 4096   /* min(dim·32, 4096) — Z_65537 aguenta até 2^16 */
static int ESP_NV[112]; static unsigned ESP_SG[112]; static int N_ESP = 0;
static long esp_escala(long corpo_m, int nv){
    if(nv <= 0) return corpo_m;
    long a1 = escala_de_degrau(1, EIXO_ESCALA), b1 = escala_de_degrau(3, EIXO_ESCALA);
    long a0 = escala_de_degrau(0, EIXO_ESCALA), b0 = escala_de_degrau(4, EIXO_ESCALA);
    if(a1 <= 0 || b1 <= 0 || a1 >= b1) return corpo_m;
    if(nv == 1) return corpo_m * a1 / b1;
    long esc = (a0 > 0 && b0 > 0 && a0 < b0) ? corpo_m * a0 / b0
                                             : (corpo_m * a1 / b1) * a1 / b1;
    for(int k = 3; k <= nv; k++) esc = esc * a1 / b1;
    return esc;
}
static int esp_gira(int atual, int dir){
    int nv = 0; unsigned sg = 0;
    if(atual >= ESP_0){ int k = atual - ESP_0; nv = ESP_NV[k]; sg = ESP_SG[k]; }
    int  nv2 = nv + 1;
    unsigned sg2 = sg | ((dir < 0 ? 1u : 0u) << nv);
    if(nv2 > 31) return atual;                     /* mais fundo que isso não há tinta */
    for(int t = 0; t < N_ESP; t++)
        if(ESP_NV[t] == nv2 && ESP_SG[t] == sg2) return ESP_0 + t;
    if(N_ESP >= 112) return dir > 0 ? 2 : -2;      /* a tabela encheu: o degrau antigo */
    ESP_NV[N_ESP] = nv2; ESP_SG[N_ESP] = sg2;
    int r = ESP_0 + N_ESP++;
    { int alc = nv2, t2 = 0; while(t2 < N_ESP){ if(ESP_NV[t2] > alc) alc = ESP_NV[t2]; t2++; }
      int iface = 6, ik = alc / 3; while(ik > 0){ iface *= 2; ik--; }
      { extern int INTERFACE_N; extern int LADO_N;
        if(iface >= 6) INTERFACE_N = iface;
        LADO_N = (alc >= 3) ? 1 : 0;
      } }
    return r;
}
/* ─── A PENTAL, GERAL: o vector do giro ──────────────────────────────────────────────
 *
 * Alonzo (a composição) comanda a Lei 5: o ponto fixo da composição é a unidade
 * imaginária — ν(x) = −1/x fixa x² = −1, o bit i — e o i é a ROTAÇÃO
 * DIMENSIONAL: não roda dentro do plano, roda ENTRE dimensões. O eixo
 * horizontal vira o vertical (M = J·i, o transporte, o quarto de volta), e na
 * torre é o que leva um andar ao seguinte (dim A_{n+1} = 2·dim A_n, com
 * x† = −1/x e x·x† = −1). O deslocamento de um giro é por isso UM vector,
 * declarado uma vez: a componente real é o respiro (horizontal, o kern), a
 * imaginária é o passo com o seu respiro (vertical, a subida). A direção da
 * espiral é CONSTANTE em todos os níveis — kern:subida = 1:4 — que é o que faz
 * dela uma espiral verdadeira e não dois ajustes; quem precisa dos eixos
 * trocados multiplica por i, não inventa outra régua. */
static long esp_passo_nv(long corpo_m, int nv){
    long e0 = esp_escala(corpo_m, nv - 1), e1 = esp_escala(corpo_m, nv);
    long passo = e0 - e1;
    { extern int LADO_N;
      if(LADO_N && e0 > 0 && nv > 1){
          /* Gentil (nne.c): norma multiplicativa — o passo compõe pela razão, não pela tábua bilinear */
          passo = corpo_m * (e0 - e1) / e0;
      }
    }
    return passo;
}
static long esp_kern_nv(long corpo_m, int nv){      /* Re: o respiro do giro */
    return sem_resp(esp_passo_nv(corpo_m, nv));
}
static long esp_sobe_nv(long corpo_m, int nv){      /* Im: o passo e o seu respiro */
    long p2 = esp_passo_nv(corpo_m, nv);
    return p2 + sem_resp(p2);
}

static long esp_sobe(long corpo_m, int e){
    int k = e - ESP_0, nv = ESP_NV[k]; unsigned sg = ESP_SG[k];
    long sobe = 0, esc_ant = corpo_m;
    for(int t = 1; t <= nv; t++){
        long passo = esp_sobe_nv(corpo_m, t);       /* o Im do vector do giro */
        sobe += (sg & (1u << (t - 1))) ? -passo : passo;
        esc_ant = esc_ant;
    }
    return sobe;
}
/* a mesma soma para a torre toda-positiva — T+T* por andar (thm:tecidos), indução */
static long esp_sobe_torre(long corpo_m, int nv){
    long sobe = 0;
    for(int t = 1; t <= nv; t++) sobe += esp_sobe_nv(corpo_m, t);   /* Σ(passo+respiro) */
    return sobe;
}
static long corpo_exp_m(long corpo_m, int e){
    if(!e) return corpo_m;
    if(e >= 16) return esp_escala(corpo_m, ESP_NV[e - 16]);      /* a espiral: a escala do nível */
    if(e == 4 || e == -4 || e == 5 || e == -8 || e == -5) return corpo_m;   /* pilha, radicando,
                                                    * matriz normal, espaço não-quebrável */
    long a = (e == 2 || e == -2) ? escala_de_degrau(0, EIXO_ESCALA) : escala_de_degrau(1, EIXO_ESCALA);
    long b = (e == 2 || e == -2) ? escala_de_degrau(4, EIXO_ESCALA) : escala_de_degrau(3, EIXO_ESCALA);
    if(a <= 0 || b <= 0 || a >= b) return corpo_m;         /* sem escala não há dobra — e diz-se */
    return corpo_m * a / b;                                /* produto cruzado, UMA divisão: o Tf */
}
static long sobe_exp_m(long corpo_m, int e){               /* a subida: o que a escala tirou */
    if(!e || e == 8 || e == -8 || e == -5) return 0;   /* matriz e nbsp: sem subida */
    if(e >= 16){
        /* A REVERSÃO DA PARTITURA: o giro soma SEMPRE com sinais. O lado
         * Gentil já entra pelo PASSO (esp_passo_nv, via LADO_N); trocar a
         * soma pela torre toda-positiva esquecia os sinais e matava a
         * inversa — o índice deixava de descer o que o expoente sobe (§X9)
         * e o vão sup–sub colapsava (§X16). Para torres todas-positivas as
         * duas somas coincidem, logo a indução T+T* (esp_sobe_torre) nada
         * perde: ela é a régua dos medidores da torre, não do giro. */
        return esp_sobe(corpo_m, e);                     /* espiral com sinais, inversa exata */
    }
    long d = corpo_m - corpo_exp_m(corpo_m, e);
    return e > 0 ? d : -d;
}

/* a largura da linha: o produto largura(por-mil do /Widths) × corpo(mantissa do Tf)
 * acumula EXACTO em 10^-6, e divide-se UMA vez no fim — a régua do Td, não por glifo */
static long mede(const Gl *g, int n, long corpo_m){   /* devolve na régua do Td (10^-3 pt) */
    long w = 0;
    /* o par expressão+rótulo do underbrace vale max(X, rótulo): o rótulo vive por baixo
     * e só conta o EXCESSO sobre o segmento que o antecede — a mesma conta do desenrola */
    long seg = 0, rot = 0; int em_rot = 0;
    long n4 = 0, d4 = 0; int em4 = 0;
    long m_cw[8]; long m_cel = 0; int em8 = 0, m_c = 0, m_nc = 0;
    for(int i = 0; i < n; i++){
        /* Claves: avanço já em Td — acumula ×1000 como o resto (divide no return).
         * Inline (MAX_FUN): 0x82/83/84 = G/F/C Emmentaler. */
        if(g[i].g == 0x82 || g[i].g == 0x83 || g[i].g == 0x84){
            long td = (g[i].g == 0x82) ? 11266L : (g[i].g == 0x83) ? 11775L : 10000L;
            long wc = td * 1000;
            w += wc; seg += wc;
            continue;
        }
        long wg = (long)largura(g[i].g, g[i].f) * corpo_exp_m(corpo_m, g[i].e);
        if(g[i].e == 5 && (i == 0 || (g[i-1].e != 5 && g[i-1].e != 2 && g[i-1].e != -2
                                       && g[i-1].e < 16)))
            w += 5 * (corpo_m - corpo_exp_m(corpo_m, 1)) / 3 * 1000;   /* o gancho da raiz,
                                       * NA RÉGUA DO ACUMULADOR (10^-6): somava 10^-3 e valia
                                       * mil vezes menos — a linha com raiz media curta */
        /* A MATRIZ MEDE O BLOCO: o máximo de cada coluna, somado — o esquema das tabelas */
        if(g[i].e == 8 || g[i].e == -8){
            if(!em8){ em8 = 1; m_c = 0; m_nc = 1; m_cel = 0; for(int t = 0; t < 8; t++) m_cw[t] = 0; }
            int gk = g[i].g;
            if(gk == 2 || gk == 3){
                if(m_cel > m_cw[m_c]) m_cw[m_c] = m_cel;
                m_cel = 0;
                if(gk == 2){ if(m_c < 7) m_c++; if(m_c + 1 > m_nc) m_nc = m_c + 1; }
                else m_c = 0;
            } else if(gk >= 4 && gk <= 7)
                w += (long)largura(gk <= 5 ? (gk == 4 ? '(' : ')') : (gk == 6 ? '[' : ']'),
                                   g[i].f) * corpo_m;
            else m_cel += wg;
            continue;
        }
        if(em8){
            if(m_cel > m_cw[m_c]) m_cw[m_c] = m_cel;
            long bloco = (long)(m_nc - 1) * (corpo_m - corpo_exp_m(corpo_m, 1)) * 1000;
            for(int t = 0; t < m_nc; t++) bloco += m_cw[t];
            w += bloco; seg += bloco; m_cel = 0; em8 = 0;
        }
        if(g[i].e >= 16 && (i == 0 || g[i-1].e == 0
                            || (g[i-1].e >= 16 && ESP_NV[g[i].e-16] > ESP_NV[g[i-1].e-16]))){
            long ka = (i > 0 && g[i-1].e >= 16) ? corpo_exp_m(corpo_m, g[i-1].e) : corpo_m;
            long kx = sem_resp(ka - corpo_exp_m(corpo_m, g[i].e));
            if(kx > 0){ w += kx * 1000; seg += kx * 1000; }
        }
        if(g[i].g == 8 || g[i].g == 9){
            long cbx = corpo_exp_m(corpo_m, g[i].e);
            long dvl = cbx - corpo_exp_m(corpo_m, 1);
            if(dvl <= 0 || dvl >= cbx) dvl = cbx / 5;
            long pad = dvl + sem_resp(dvl);       /* λ⁺ */
            long m3  = sem_resp(pad);             /* externo H = g2 */
            long lado = (pad + m3) * 1000;        /* um lado: abre OU fecha */
            w += lado; seg += lado; continue; }
        if(g[i].g >= 4 && g[i].g <= 13 && g[i].e != 8 && g[i].e != -8){
            int c2 = (g[i].g == 4) ? '(' : (g[i].g == 5) ? ')' : (g[i].g == 6) ? '['
                   : (g[i].g == 7) ? ']' : (g[i].g == 10) ? '{'
                   : (g[i].g == 11) ? '}' : 0xE1;
            long wd = (long)largura(c2, g[i].g >= 12 ? F_SIM : g[i].f) * corpo_m;
            w += wd; seg += wd; continue; }
        if(g[i].e == 4 || g[i].e == 6 || g[i].e == 7){ n4 += wg; em4 = 1; continue; }
        if(g[i].e == -4 || g[i].e == -6 || g[i].e == -7){ d4 += wg; em4 = 1; continue; }
        if(em4){ long m4 = n4 > d4 ? n4 : d4; w += m4; seg += m4; n4 = d4 = 0; em4 = 0; }
        if(g[i].e == -3){ em_rot = 1; rot += wg; continue; }
        if(em_rot){ if(rot > seg) w += rot - seg; seg = 0; rot = 0; em_rot = 0; }
        w += wg; seg += wg;
    }
    if(em8){
        if(m_cel > m_cw[m_c]) m_cw[m_c] = m_cel;
        long bloco = (long)(m_nc - 1) * (corpo_m - corpo_exp_m(corpo_m, 1)) * 1000;
        for(int t = 0; t < m_nc; t++) bloco += m_cw[t];
        w += bloco;
    }
    if(em4){ long m4 = n4 > d4 ? n4 : d4; w += m4; }
    if(em_rot && rot > seg) w += rot - seg;
    return w / 1000;
}

/* a largura NATURAL de uma célula de tabela, medida do fonte: os comandos consomem-se,
 * o \textbf mede na negra e o \emph na itálica (o desenho certo, não um fator), o léxico
 * dá os símbolos, e o resto mede glifo a glifo — o produto exacto em 10^-6, como o mede.
 * É a régua do LaTeX: a coluna tem a largura do seu conteúdo, não uma repartição minha. */
static const Par *lex_acha(const char *cmd);
static long mede_celula(const char *s, long a, long b, long corpo){
    long w6 = 0; int f = F_REG;
    for(long i = a; i < b; ){
        char c = s[i];
        if(c == '\\'){
            long j = i + 1;
            if(j < b && !isalpha((unsigned char)s[j])){
                if(s[j] == ',' || s[j] == ' ' || s[j] == ';') w6 += (long)largura(' ', f) * corpo;
                i = j + 1; continue;
            }
            char cm2[24]; int k = 0;
            while(j < b && isalpha((unsigned char)s[j]) && k < 23) cm2[k++] = s[j++];
            cm2[k] = 0;
            if(!strcmp(cm2, "textbf")) f = F_NEG;
            else if(!strcmp(cm2, "emph") || !strcmp(cm2, "textit")) f = (N_CARTA > F_ITA) ? F_ITA : F_REG;
            else { const Par *P = lex_acha(cm2);
                   if(P) w6 += (long)largura(P->glifo, P->simb ? F_SIM : f) * corpo; }
            i = j; continue;
        }
        if(c == '{'){ i++; continue; }
        if(c == '}'){ f = F_REG; i++; continue; }
        if(c == '$' || c == '^' || c == '_'){ i++; continue; }   /* o expoente mede largo: não quebra */
        if(c == '~' || c == '\n' || c == '\t'){ w6 += (long)largura(' ', f) * corpo; i++; continue; }
        int cons; int g = utf8_glifo((const unsigned char*)s + i, &cons);
        w6 += (long)largura(g, f) * corpo;
        i += cons ? cons : 1;
    }
    return w6;
}

/* A DEFORMAÇÃO: dados n_esp espaços e uma folga, quanto se acrescenta a cada espaço. E o resíduo,
 * que é a folga que a deformação NÃO absorveu — tem de ser 0 até ao arredondamento do último. */
static long deforma(long folga, int n_esp, long *por_espaco){
    if(n_esp <= 0){ *por_espaco = 0; return folga; }
    *por_espaco = folga / n_esp;
    return folga - *por_espaco * n_esp;            /* o que sobra, sempre < n_esp */
}

/* ─────────────────────────────────────────────────────────────────────────────
 * §X4/§X5  O LUNAR DESENROLA — a página, e o PDF
 * ───────────────────────────────────────────────────────────────────────────── */

/* A FOLHA É EXACTA, e escreve-se exacta. A4 são 210 x 297 mm, que em pontos dá
 * 595,276 x 841,890 — e estava aqui 595 x 842, arredondado. Isso é dissipação no sentido
 * literal: o valor verdadeiro perde-se ao ser escrito, e não volta. O gabarito escreve
 * 595.276 x 841.89; nós escrevíamos meio ponto ao lado na altura.
 *
 * As contas do desenho continuam em pontos inteiros — mudá-las mexe na quebra de linha e
 * isso mede-se à parte. O que aqui se corrige é o que se ESCREVE. */
#define A4_LM   595276L                        /* milésimos de ponto: 210 mm */
#define A4_AM   841890L                        /* 297 mm */
#define A4_L    (A4_LM / 1000)
#define A4_A    (A4_AM / 1000)
/* A MARGEM SAI DO `geometry` DO ESTILO, não deste ficheiro. Estava 64 e o estilo declara
 * `margin=2.6cm`, que são 73,7 pt — quase 10 pt de diferença em cada lado, e a coluna toda
 * mais larga do que o documento pede. Um número escrito à mão, como o `8` do espaçamento
 * e o `10` do corpo antes dele. */
/* a régua TeX→pontos, UMA só: cm/mm/in/ex/em e o resto em pontos. Estava escrita cinco vezes, cada
 * uma com um conjunto de unidades diferente (umas sem `in`, uma com `ex`/`em`) --- cinco réguas para
 * a mesma conversão. Agora uma, completa e consistente. */
/* O NÚMERO LÊ-SE COMO O CORPO DO NÚMERO MANDA: mantissa inteira, vírgula-fixa, na régua
 * do ficheiro (10^-3). Os valores do estilo têm até três casas — a leitura é EXACTA, zero
 * arredondamento; com uma quarta casa arredonda UMA vez, o único e o correcto. É o
 * str2dbl de lib/le_num.h sem o passo final para o contínuo: a mantissa fica inteira. */
/* Fase C: rt_le_decimal_end + unidade 10^-3 (milésimos de ponto). Arredonda na 4.ª casa. */
static long fixo_mil_de_pq(int sg, long p, long q){
    if(q <= 0) return 0;
    int64_t num = (int64_t)p * 1000 + q / 2;
    int64_t v = num / q;
    return sg < 0 ? -(long)v : (long)v;
}
static long fixo_mil(const char *s, const char **end){
    int sg; long p, q; const char *e;
    if(!rt_le_decimal_end(s, &sg, &p, &q, &e)){ if(end) *end = s; return 0; }
    if(end) *end = e;
    return fixo_mil_de_pq(sg, p, q);
}
static int32_t fixo_mil_i32(const char *s, const char **end){
    long r = fixo_mil(s, end);
    if(end && *end == s) return 0;
    if(r < (long)INT32_MIN || r > (long)INT32_MAX) return 0;
    return (int32_t)r;
}
/* a unidade é uma RAZÃO de inteiros — 72 pt por polegada, 2,54 cm por polegada — e o
 * valor converte-se por produto cruzado, com a divisão única do escrever */
static void unidade_razao(const char *u, long *num, long *den){
    if(!strcmp(u, "cm")){ *num = 72000; *den = 2540;  return; }
    if(!strcmp(u, "mm")){ *num = 72000; *den = 25400; return; }
    if(!strcmp(u, "in")){ *num = 72;    *den = 1;     return; }
    if(!strcmp(u, "ex")){ *num = 9;     *den = 2;     return; }
    if(!strcmp(u, "em")){ *num = 21;    *den = 2;     return; }
    *num = 1; *den = 1;                           /* pt (ou vazio): o ponto é a unidade base */
}
/* lê "Nunidade" (ex. "6mm", "3pt") e devolve a mantissa EM 10^-3 DE PONTO, ou -1 */
static long medida_mil(const char *str){
    const char *end;
    long v = fixo_mil(str, &end);
    if(end == str || v <= 0) return -1;                      /* sem número, ou não positivo: como antes */
    char u[8]; int k = 0;                                    /* %2[a-z]: até duas minúsculas */
    while(k < 2 && end[k] >= 'a' && end[k] <= 'z'){ u[k] = end[k]; k++; }
    u[k] = 0;
    long num, den; unidade_razao(u, &num, &den);
    return (2 * v * num + den) / (2 * den);                  /* produto cruzado, UMA divisão */
}

/* A MARGEM é uma VARIÁVEL (o carrega_config enche-a), não uma chamada ao parser: o núcleo lê o
 * valor, não o sscanf. margem_estilo (o parser) fica do lado do wrapper. */
static long MARGEM_V = 64;
static long TABCOLSEP = 6000;   /* o \tabcolsep: 6pt é o do LaTeX; o \setlength grava-o */
/* OS TEOREMAS DO ESTILO: a família dos \newtheorem, lida pelo wrapper (le_teoremas).
 * O estilo declara UM contador para todos, preso ao capítulo — e é isso que se realiza:
 * C_TEO sobe por ambiente, zera quando o C_CAP muda. */
static struct { char amb[24]; char nome[32]; int ita; } TEOR[20];   /* ita: \theoremstyle{plain} */
/* O CABEÇALHO do fancyhdr: a esquerda é o texto FIXO lido do \fancyhead[L] do estilo
 * (o wrapper enche CAB_ESQ), a direita é a marca do capítulo (o \chaptermark) — e só
 * se desenha quando há marca, como o gabarito faz nas páginas planas. */
static char CAB_ESQ[64] = "";
static char CAB_DIR[96] = "";
static int  N_TEOR = -1;
static long C_TEO = 0, C_TEO_CAP = -1;
#define MARGEM   MARGEM_V
#define COL     (A4_L - 2*MARGEM)
#define CORPO    10                                /* o corpo do texto, em pontos */
#define ENTRE    14                                /* a entrelinha */
/* A VERTICAL VIVE EM MILÉSIMOS, como a horizontal sempre viveu. Estava em pontos INTEIROS,
 * e por isso a entrelinha de 13,6 era arredondada para 14 A CADA LINHA: o gabarito desce
 * 13,549 e nós descíamos 14,0. Vinte e sete páginas de diferença nasciam daí, e a conversão
 * tinha resíduo — que é o que aqui não se admite. */
#define PT       1000L
#define TOPO    (A4_AM - MARGEM*PT)
#define FUNDO   (MARGEM*PT)

#define MAXOBJ 8192

/* ── A COSTURA DA SAÍDA ──────────────────────────────────────────────────────────────
 * O destino do PDF era um `FILE*`. Para o núcleo subir a wasm não pode haver `FILE*` (nem
 * `fprintf` da libc): a saída é um BUFFER com cursor, e o formato dos números é inteiro.
 * FEITO: a `Saida` É slot+cursor (buf, cur, len, cap), e `s_byte` escreve em `buf[cur]` ---
 * o `FILE*` só existe no wrapper nativo (`main`), que ESCOA o slot para o ficheiro por um
 * `fwrite(sf.buf, 1, sf.len, f)` no fim. Todas as escritas do núcleo passam por
 * `s_fmt/s_bytes/s_byte/s_pos/s_vai` + o formatador inteiro `s_fix/s_num`, sem libc. */
typedef struct {
    int cur;                                       /* cursor — int: o traduz alinha i32 sem pad */
    int len;
    int cap;
    int perdeu;
    unsigned char *buf;
} Saida;

typedef struct {
    Saida sf, sfundo;                              /* a saída do PDF e o stream do fundo, em slots */
    int   fundo_on;                                /* há stream de fundo aberto? (era o ponteiro fundo) */
    long *off;                                     /* no DISCO, não na pilha */
    int  nobj;
    int  *pag; int npag;                           /* idem: no DISCO */
    /* O Y DO LAPIS E' INTEIRO. Era `double`, e um double ACUMULA: as tres celulas de uma fila
     * saiam em 728,78 · 729,00 · 729,22 — alinhadas ao centesimo e nao IGUAIS. E ao centesimo
     * nao basta: o lado do tesseracto e' inteiro, e uma linha recta nao tem virgula.
     *
     * Com inteiro o residuo e' ZERO POR CONSTRUCAO — nao ha' onde o erro se acumular, porque
     * nao ha' fraccao a arrastar. E' a regra que este projecto ja' tinha e eu nao apliquei
     * aqui: inteiro desde o primeiro rascunho, e nao «float agora, exacto depois». */
    long y;                        /* onde vai o lápis — em MILÉSIMOS de ponto, exacto */
    int  aberta;                                   /* há página aberta? */
    long len_obj;                                  /* o objeto /Length pendente */
    long stream_ini;
    long caixa_y;                                  /* onde a caixa abriu (10^-3); <0 = nenhuma */
    int  plana;            /* esta página é `plain`: capa, sumário 1, resumo, abertura de capítulo */
    long num;              /* o número IMPRESSO: reinicia no sumário e no resumo, como o book */
    int  sem_pe;           /* capa e resumo não mostram o pé (o estilo `empty` de lá) */
    long   caixas, reguas;                         /* o que se desenhou, para se poder contar */
    long   n_fundo;                                /* quantas operações lá foram */
    int    abriu_agora;    /* o `desenrola` abriu página? A tabela precisa de saber: o seu
                            * `tab_y` fica a apontar para a página anterior, e a célula
                            * seguinte nasceria fora do papel. */
    int    fo, flo;                                /* os objectos do stream do fundo */
} Pdf;

/* os cinco da costura: escrevem no SLOT (buf[cur]), sem libc --- o núcleo não vê o FILE*. */
static void s_byte(Saida *s, int c);
static void s_bytes(Saida *s, const void *b, long n);
/* um inteiro em decimal, com largura opcional e enchimento (para o `%010ld` da xref) */
static void s_num(Saida *s, long v, int width, char pad){
    char t[24]; int tn = 0; int neg = 0; unsigned long u;
    if(v < 0){ neg = 1; u = (unsigned long)(-(v + 1)) + 1UL; } else u = (unsigned long)v;
    if(u == 0) t[tn++] = '0'; else while(u){ t[tn++] = (char)('0' + u % 10); u /= 10; }
    for(int k = tn + neg; k < width; k++) s_byte(s, pad);
    if(neg) s_byte(s, '-');
    while(tn) s_byte(s, t[--tn]);
}
/* o mini-formatador: %d %ld %s %c %% e largura/zero (%010ld). SEM float (a fracção vai por
 * s_fix) e SEM vfprintf --- é o que o tradutor sabe compilar, uma variádica de utilizador. */
/* buffer do formatador — snprintf (variadica directa, medida) + s_bytes */
static char S_FMT_BUF[2048];
static void s_flush(Saida *s){
    int n = 0; while(S_FMT_BUF[n]) n = n + 1;
    s_bytes(s, S_FMT_BUF, n);
}
static void s_bytes(Saida *s, const void *b, long n){
    if(s->cur + n <= s->cap){ memcpy(s->buf + s->cur, b, (size_t)n); s->cur += n; if(s->cur > s->len) s->len = s->cur; }
    else s->perdeu += n;                           /* recusou por inteiro: conta-se, não se cala */
}
static void s_byte (Saida *s, int c){
    if(s->cur < s->cap){ s->buf[s->cur++] = (unsigned char)c; if(s->cur > s->len) s->len = s->cur; }
    else s->perdeu++;
}
static long s_pos  (Saida *s){ return s->cur; }                 /* o cursor É o ftell */
static void s_vai  (Saida *s, long off){ s->cur = off; }        /* seek: reposiciona o cursor, escreve por cima */

/* o formatador INTEIRO: imprime `val` (em unidades de 10^-nd) como N.ddd, só com bytes ---
 * é o `%.Nf` sem double e sem vfprintf, que o tradutor não tem. Para milésimos exactos dá o
 * mesmo que o `%.3f` dava (n/1000.0 arredonda ao milésimo = n), agora por construção. */
static void s_fix(Saida *s, long val, int nd){
    if(val < 0){ s_byte(s, '-'); val = -val; }
    long um = 1; for(int k = 0; k < nd; k++) um *= 10;
    long ip = val / um, fp = val % um;
    char t[24]; int tn = 0;
    if(ip == 0) t[tn++] = '0'; else while(ip){ t[tn++] = (char)('0' + ip % 10); ip /= 10; }
    while(tn) s_byte(s, t[--tn]);
    s_byte(s, '.');
    for(int k = nd - 1; k >= 0; k--){ long d = 1; for(int m = 0; m < k; m++) d *= 10; s_byte(s, (char)('0' + (fp / d) % 10)); }
}
/* o traço da caixa escreve DOIS decimais — a régua que esse operador usa. A quantidade
 * entra na régua do Td (mantissa 10^-3) e a divisão por dez, arredondada, é a ÚNICA — a
 * do escrever, como manda o corpo do número. */
static void s_c(Saida *s, long v_m){ s_fix(s, (v_m + (v_m >= 0 ? 5 : -5)) / 10, 2); }

/* O .TEX ORIGINAL VIAJA NO PDF, invisível. Os comentários e a marcação não vão à página, mas
 * não se perdem: guardam-se num objecto que o leitor ignora e a volta lê. A composição deixa
 * de ser o buraco negro que eu inventei — passa a REVERTER, porque nada se apaga. */
/* O .tex original não se COPIA: guarda-se só o ENDEREÇO do slot de entrada (o caminho), e a
 * composição transmite-o do slot direto para o PDF. Nada se grava — a assinatura cavalga o
 * corpo, não vive num buffer. */
static const char *FONTE_TEX = 0;              /* o caminho do slot de entrada, não o conteúdo */

/* ─── O DESENHO: as cores saem do estilo.tex e o caminho é o do desenha.c ─────────────
 * Nenhuma primitiva nova: `m`/`l` fazem o caminho, `f` preenche, `S` traça. É o mesmo
 * operador que desenha o glifo, com outro grau — a régua é grau 1, o contorno é grau 2.
 *
 * As cores NÃO estão escritas aqui: leem-se de ../estilo.tex, que é onde o design vive.
 * Escrevê-las seria a referência à mão, e mudar a cor lá deixaria de mudar o que sai. */
typedef struct { char nome[32]; long r, g, b; } Cor;
#define CORES   ((Cor*)disco_buf(9, (long)(64 * sizeof(Cor))))
static long N_CORES = -1;

/* hex1/hex2 (o `sscanf %2x`) vêm de lib/le_num.h --- fonte única, travada em str2dbl_dual.c */

/* ─── A ESCALA TIPOGRÁFICA, lida do estilo.tex como as cores ─────────────────────────
 * O design tem uma ESCALA, e ela não é uma lista de tamanhos escolhidos: as razões entre
 * degraus consecutivos são 1,1732 · 1,1745 · 1,1743 · 1,1736 · 1,1742 — e φ^(1/3) = 1,1740.
 * A escala é geométrica na RAIZ CÚBICA DO ÁUREO, e a entrelinha é 1,4497 do corpo em todos.
 *
 * E o tradutor ignorava-a: usava CORPO 10 e ENTRE 14 — dois números escritos à mão, com a
 * razão 1,4 em vez de 1,4497, e nenhum degrau da escala. Daí o texto sair grosso: 10pt onde
 * o design manda 10,50, e sem a hierarquia dos sete tamanhos.
 *
 * Aqui lê-se \fontsize{corpo}{entrelinha} do estilo.tex, pela mesma porta das cores — e por
 * isso mudar a escala lá muda o que sai daqui. */
typedef struct { long corpo, entre; } Degrau;   /* mantissas na régua do Tf: 10^-3 pt */
static Degrau ESCALA[16];
/* O degrau que a EXPANSÃO trouxe. A avaliação de `\gktit` devolve `\fontsize{23.42}{33.95}`,
 * e esse número não é uma escolha deste ficheiro — é o degrau da dourada que o autor escreveu
 * no estilo.tex. Quando >= 0, manda sobre o nível da secção: quem sabe o tamanho é a fonte. */
static char COR_TEXTO[24] = "";   /* a cor corrente, do \\titleformat */
static long HIFENS = 0;   /* quantas vezes se desceu ao nível da sílaba */
static int COR_PROF = -1;  /* a profundidade onde a cor foi posta */
/* ─── O SUMÁRIO: duas passagens, como o `.aux` do LaTeX ──────────────────────────────
 *
 * Um sumário precisa de saber em que página cai cada título — e isso só se sabe depois de
 * compor. O LaTeX resolve-o com o `.aux`: a primeira corrida escreve, a segunda lê. Aqui é
 * a mesma coisa sem ficheiro: compõe-se uma vez para um destino descartável, recolhem-se
 * as entradas, e compõe-se outra vez com elas.
 *
 * É a mesma mecânica que já centra a capa (medir e refazer), um nível acima: lá era uma
 * página, aqui é o documento. E é preciso porque o sumário MUDA a paginação do que vem
 * depois — a segunda passagem não dá as mesmas páginas que a primeira, e por isso corre-se
 * até estabilizar, no máximo três vezes. */
#define MAX_TOC 512
typedef struct { int nivel; char rot[32]; char txt[160]; int pag; } Toc;
#define TOC     ((Toc*)disco_buf(6, (long)(MAX_TOC * sizeof(Toc))))
static int  N_TOC = 0, TOC_LE = 0;      /* LE: estamos na passagem que ESCREVE o sumário */
static int  TOC_I = 0;                  /* qual entrada se lê a seguir */

static long SALTA_DE = -1, SALTA_ATE = -1;   /* o ramo do `\ifSubfiles` que nao e' o nosso */
static int CENTRA = 0;   /* dentro de `center`: a linha centra-se em vez de justificar */
static char *le_tudo(const char *nome, long *n);
static long Y_CAPA = -1;
static long CAPA_POS = 0, CAPA_I = 0, CAPA_Y = 0, CAPA_FUN = 0;
static int  CAPA_NF = 0;
static int  CAPA_PAG = 0;
static long CAPA_ALT = 0;   /* a altura MEDIDA da capa (régua do Td), na 2.ª passagem */
static long DEG_FORCADO = -1;
/* E O DEGRAU TEM ESCOPO DE GRUPO, como no LaTeX. Sem isto, um `\fontsize` que a expansão
 * trouxe dentro de `{...}` contaminava TUDO o que vinha depois — incluindo os títulos, que
 * então já não usavam o degrau do seu nível. MEDIDO: o PDF tinha 112 transições para o
 * degrau do título onde o documento declara 155 capítulos. O `\normalsize` repunha, mas só
 * onde alguém se lembrasse de o escrever, e isso não é escopo: é sorte. */
static int PROF = 0;            /* a profundidade de chaveta corrente */
static int DEG_PROF = -1;       /* a profundidade em que o degrau corrente foi posto */
static long fecha_chave(const char *s, long n, long i);
/* salta até ao próximo `{` (ou ao fim): o preâmbulo de ler um grupo `{…}`, repetido por todo o
 * lado no tratamento dos comandos. Uma só régua. (o `for` em vez de `while` é de propósito.) */
static long ate_abre(const char *s, long q, long n){ for(; q < n && s[q] != '{'; q++){} return q; }
static long N_ESCALA = -1;


/* ─── O CORPO DO TEXTO É O `\normalsize` DA CLASSE, e não um degrau da escala ────────
 *
 * O `\gktexto` do estilo só é usado na CAPA (linha 196) --- o corpo do texto corre com o
 * `\normalsize`, e quem o define é a CLASSE que o documento pede. MEDIDO no gabarito: o
 * texto sai a **10,9091 pt** com entrelinha **13,50**, e eu compunha a 10,50 com 15,00.
 * Nem o corpo nem a razão eram os que eu usava (1,2375 contra 1,4286).
 *
 * E a cadeia está toda no sistema, como a do babel:
 *
 *     livro.tex     \documentclass[11pt,a4paper]
 *     size11.clo    \@setfontsize\normalsize\@xipt{13.6}
 *     latex.ltx     \def\@xipt{10.95}
 *
 * Lê-se de lá, e não se escreve aqui. Sem a classe à mão fica o que havia. */
static long CLASSE_CORPO = 0, CLASSE_ENTRE = 0;   /* mantissas 10^-3, lidas da classe */


/* ═══ A OPERAÇÃO ÚNICA: um corpo tem ASSINATURA, e tudo o resto lê-se dela ══════════
 *
 * O Aarão: «revisa todo o interpretador numa única operação para tudo — só muda a
 * assinatura e a composição de corpos; fontes, tamanhos, espaçamentos, tudo centralizado
 * numa operação global baseada na primeira e segunda leis».
 *
 * Havia DEZ funções a responder à mesma pergunta --- `escala_corpo`, `escala_entre`,
 * `largura`, `carta_do_corpo`, `fpdf_regista`, `espaco_titulo`, `degrau_do_comando`,
 * `cor_do_comando`, `spline_por_corpo`, `corpo_derivado` --- com setenta chamadas. E é uma
 * pergunta só: *dado um corpo tipográfico, qual é a medida?*
 *
 * A ASSINATURA de um corpo tipográfico são dois inteiros:
 *
 *     (variante, degrau)      a variante é a estaca — peso, inclinação, caixa, largura
 *                             o degrau é o expoente da escala — `base · σ^k`
 *
 * E o que se pede dele é o TRIAL, um eixo por estado:
 *
 *     +1   ESCALA        o corpo — multiplica, e é `σ^k`
 *     -1   ESPAÇAMENTO   a entrelinha — soma, e é o dual da escala
 *      0   ATRAVESSA     a largura de um glifo — o que passa de um lado ao outro
 *
 * Não há quarto eixo porque o trial não tem quarto estado (`teoria.tex`, thm:trial). E as
 * duas leis estão nos dois lados: a Lei 1 dá o dual de cada eixo, a Lei 2 dá o passo entre
 * degraus. */
typedef struct { int var; long deg; } Corpo;

/* (EIXO_* e a declaração de escala_de_degrau vivem junto ao `mede`, que já os usa) */
static int    largura_de(int g, int fonte, long corpo);

/* A OPERAÇÃO. Uma só, e o corpo é CAMPO — como o `MOVE` do corpo_analitico, onde «a mesma
 * instrução serve 500 corpos diferentes sem uma instrução nova, porque o corpo é campo e
 * não opcode». O eixo é o próprio parâmetro: ESCALA e ESPACO são a mesma leitura, campos duais. */
static long medida(Corpo c, int eixo, long glifo){
    if(eixo == EIXO_LARGURA)
        return largura_de((int)glifo, c.var, escala_de_degrau(c.deg, EIXO_ESCALA));
    return escala_de_degrau(c.deg, eixo);           /* eixo é EIXO_ESCALA ou EIXO_ESPACO */
}

/* E A COMPOSIÇÃO DE CORPOS. Um corpo compõe-se com outro pela assinatura, e o resultado é
 * um corpo: a variante do primeiro com o degrau do segundo, ou o passo de um sobre o outro.
 * É `A ⊗ B` --- e o que sai é da mesma espécie do que entrou, que é o que faz dela uma
 * operação e não uma conversão. */
static Corpo compoe(Corpo a, Corpo b){ Corpo r; r.var = a.var; r.deg = b.deg; return r; }

/* e o DUAL de um corpo, pela Lei 1: o degrau troca de sinal em torno do zero. O produto dos
 * corpos de `c` e `c†` é `base²` --- a alfândega |N|=1 lida na escala. */

/* ── as leituras da operação, uma linha cada ────────────────────────────────────────
 * Não são funções novas: são a MESMA operação com o eixo fixado. Ficam com os nomes
 * antigos porque setenta chamadas os usam, e trocar os nomes não muda o que se mede. */
static long escala_corpo(long deg){ Corpo c; c.var = F_REG; c.deg = deg; return medida(c, EIXO_ESCALA, 0); }
static long escala_entre(long deg){ Corpo c; c.var = F_REG; c.deg = deg; return medida(c, EIXO_ESPACO, 0); }

/* o degrau da escala: 0 é o mais pequeno. O texto corrido é o `gktexto`, que é o do meio. */
/* ─── UMA FONTE PDF POR (VARIANTE, DEGRAU) ──────────────────────────────────────────
 *
 * O PDF declara UM /Widths por fonte, e as larguras variam com o corpo, porque cada corpo
 * tem o seu desenho. Com uma fonte so', as larguras escritas no ficheiro sao de um desenho
 * e o leitor usa-as para todos: abrir os outros para MEDIR nao muda o que ele usa para
 * DESENHAR. Era a segunda regua, desta vez dentro do proprio PDF.
 *
 * O gabarito tem 32 objectos de fonte por isto — SFBX2488, SFBX1440 e SFBX1200 sao fontes
 * SEPARADAS. Aqui o indice e' o par, e e' deterministico: variante*16 + degrau. */
#define MAX_FPDF 64
#define N_FIXA   16       /* fontes declaradas no PDF: 3 variantes x degraus, com folga */
static int FPDF[MAX_FPDF];
static int FPDF_VAR[MAX_FPDF];
static long FPDF_CORPO[MAX_FPDF];                  /* mantissas 10^-3 — a régua do Tf */
static int N_FPDF = 0;

static int fpdf_regista(int variante, long corpo){
    int k;
    /* o k da símbolo era 48 — e COLIDIA com (F_VER, degrau 0), o versalete \gknota do
     * cabeçalho: 3·16+0 = 48. O ⊕ passava a desenhar-se pela versalete — o Å com anel.
     * O espaço próprio da variante 5 é 80..95, e ninguém mais lá chega. */
    if(variante == F_SIM) k = F_SIM * 16;
    else if(variante == F_MAT) k = F_MAT * 16;     /* 96..111, espaço próprio como a símbolo */
    else if(variante == F_MTB) k = F_MTB * 16;     /* 112..127 */
    else if(variante == F_SMB) k = F_SMB * 16;     /* 128..143 */
    else if(variante == F_NIT) k = F_NIT * 16;     /* 144..159 */
    else {
        long d = 0, dmin = 1L << 60;
        for(long t = 0; t < N_ESCALA; t++){
            long x = ESCALA[t].corpo - corpo; if(x < 0) x = -x;
            if(x < dmin){ dmin = x; d = t; }
        }
        k = variante * 16 + (int)d;
    }
    for(int i = 0; i < N_FPDF; i++) if(FPDF[i] == k) return i;
    if(N_FPDF >= MAX_FPDF) return 0;
    FPDF_VAR[N_FPDF] = variante; FPDF_CORPO[N_FPDF] = corpo;
    FPDF[N_FPDF] = k; return N_FPDF++;
}

#define D_NOTA  0
#define D_COD   1
#define D_TEXTO 2                                      /* o corpo do texto */
/* ─── A ESCALA É A DOURADA, E O DEGRAU É UM EXPOENTE ─────────────────────────────────
 *
 * O `corpo_analitico.tex` §renorm: «renormalizar é mudar a régua sem mudar o objecto» e «a
 * escala não sobe continuamente: sobe por DEGRAUS, e os degraus são os metais». O número
 * muda com a régua; a RAZÃO não.
 *
 * MEDIDO nos degraus que o estilo declara --- 7,62 · 8,94 · 10,50 · 12,33 · 14,47 · 16,99 ·
 * 23,42: as razões consecutivas são 1,17323 · 1,17450 · 1,17429 · 1,17356 · 1,17415, e
 * $\varphi^{1/3} = 1,17398$. Resíduo 0,00017. E o último salto é 1,37846, que é
 * $\varphi^{1/3}$ ao QUADRADO: dois degraus de uma vez.
 *
 * Logo a escala inteira é `base · φ^(k/3)`, e um tamanho não se procura numa tabela: o
 * degrau É o expoente. Isto é o que resolve os tamanhos de uma vez em vez de um a um --- e
 * é por isso que o `\part`, que o estilo NÃO declara em `\titleformat`, não precisa de caso
 * especial nenhum: o seu k sai da posição, como o de todos os outros. */
/* (a razão da escala não se calcula mais aqui: quem precisa da dobra lê DOIS degraus da
 * tabela e cruza — corpo_exp_m —, e o degrau que o estilo não declara sai por posição em
 * degrau_do_comando. O corpo_derivado e a razao_escala em vírgula flutuante morreram.) */

/* O CORPO E A ENTRELINHA SÃO UMA OPERAÇÃO, o eixo escolhe o campo --- eram duas funções com o mesmo
 * clamp (a redundância que escondia a assinatura). A Lei 1: os dois lados da escala (o corpo que
 * multiplica, a entrelinha que soma) lidos da MESMA tabela pelo MESMO caminho.
 *   O TEXTO CORRIDO é o `\normalsize` da classe, não o degrau `gktexto` da escala (esse só na capa);
 *   os outros são os do estilo, porque é o `\titleformat` que os pede. */
static long escala_de_degrau(long degrau, int eixo){
    long classe = (eixo == EIXO_ESCALA) ? CLASSE_CORPO : CLASSE_ENTRE;     /* enchidos pelo carrega_config */
    if(degrau == D_TEXTO){ if(classe > 0) return classe; }
    if(N_ESCALA <= 0) return (eixo == EIXO_ESCALA) ? 10000 : 14000;        /* sem estilo: o que havia */
    if(degrau < 0) degrau = 0;
    if(degrau >= N_ESCALA) degrau = N_ESCALA - 1;
    return (eixo == EIXO_ESCALA) ? ESCALA[degrau].corpo : ESCALA[degrau].entre;
}
#define D_NOTA  0
#define D_COD   1
#define D_SUB   3
#define D_SEC   4
#define D_CAP   5
/* o degrau que o `\titleformat` do estilo manda para um nível, ou -1 se não o declara.
 * A ida usava D_CAP fixo (16,99) enquanto o estilo manda `\gktit` (23,42) para o capítulo
 * — e foi a VOLTA que o revelou: ela lia 11 blocos no degrau do título onde o documento
 * tem 148 capítulos. Os dois lados passam a ler a mesma tabela. */
int winansi_para_unicode(int u);
static long degrau_do_comando(const char *cmd);
static long degrau_de(long corpo);
static void espaco_titulo(const char *cmd, long deg, long *antes, long *depois);
static const char *cor_do_comando(const char *cmd);
static int regua_do_comando(const char *cmd, long *esp, char *cor, size_t nc);
static void poe_regua(Pdf *p, long x1, long x2, long y, long esp, const char *cor);
static int cor_de(const char *nome, long *r, long *g, long *b);
/* o degrau do capítulo LÊ-SE do `\titleformat{\chapter}` em vez de ser o `D_CAP` fixo.
 * MEDIDO: o estilo manda `\gktit` (23,42) e a ida compunha a 16,99 — e `section` e
 * `subsection` batiam, só o capítulo é que não. Foi a VOLTA que o apanhou: ela lia 1 bloco
 * no degrau do título onde o documento tem 148 capítulos. */
static long d_cap(void){ long d = degrau_do_comando("chapter"); return d >= 0 ? d : D_CAP; }
#define D_TIT   6

/* a cor na régua do `rg` (três decimais): a fracção n/255 escreve-se arredondada UMA vez,
 * por produto cruzado — (2n·1000 + 255) / (2·255) é o arredondado de n·1000/255 */
static int cor_de(const char *nome, long *r, long *g, long *b){
    for(long i = 0; i < N_CORES; i++)
        if(!strcmp(CORES[i].nome, nome)){
            *r = (2L * CORES[i].r * 1000 + 255) / 510;
            *g = (2L * CORES[i].g * 1000 + 255) / 510;
            *b = (2L * CORES[i].b * 1000 + 255) / 510;
            return 1;
        }
    return 0;
}

/* um retângulo preenchido: caminho fechado + f — E VAI PARA O STREAM DO FUNDO.
 * É a diferença que faz o fundo poder existir: escrito no primeiro stream, ele pinta ANTES
 * do texto por muito que se escreva depois. A barra também vem por aqui — ela nunca precisou,
 * porque vive na margem, mas não há razão para ter dois caminhos onde um serve. */
static void poe_rect(Pdf *p, long x, long y, long w, long h, const char *cor){
    long r, g, b;                                  /* tudo na régua do Td: 10^-3 pt */
    if(!p->aberta || !p->fundo_on || !cor_de(cor, &r, &g, &b)) return;
    Saida *s = &p->sfundo;
    snprintf(S_FMT_BUF, 2048, "q "); s_flush(s); s_fix(s,r,3); s_byte(s,' '); s_fix(s,g,3);
    s_byte(s,' '); s_fix(s,b,3); snprintf(S_FMT_BUF, 2048, " rg "); s_flush(s);
    s_c(s,x);   s_byte(s,' '); s_c(s,y);   snprintf(S_FMT_BUF, 2048, " m "); s_flush(s);
    s_c(s,x+w); s_byte(s,' '); s_c(s,y);   snprintf(S_FMT_BUF, 2048, " l "); s_flush(s);
    s_c(s,x+w); s_byte(s,' '); s_c(s,y+h); snprintf(S_FMT_BUF, 2048, " l "); s_flush(s);
    s_c(s,x);   s_byte(s,' '); s_c(s,y+h); snprintf(S_FMT_BUF, 2048, " l f Q\n"); s_flush(s);
    p->n_fundo = p->n_fundo + 1;
    p->caixas = p->caixas + 1;
}

/* uma régua: dois pontos, traçado. Grau 1 — não tem par, é transporte. */
static void poe_regua(Pdf *p, long x1, long x2, long y, long esp, const char *cor){
    long r, g, b;                                  /* tudo na régua do Td: 10^-3 pt */
    if(!p->aberta || !cor_de(cor, &r, &g, &b)) return;
    Saida *s = &p->sf;
    snprintf(S_FMT_BUF, 2048, "q "); s_flush(s); s_fix(s,r,3); s_byte(s,' '); s_fix(s,g,3);
    s_byte(s,' '); s_fix(s,b,3); snprintf(S_FMT_BUF, 2048, " RG "); s_flush(s);
    s_c(s,esp); snprintf(S_FMT_BUF, 2048, " w "); s_flush(s); s_c(s,x1); s_byte(s,' '); s_c(s,y); snprintf(S_FMT_BUF, 2048, " m "); s_flush(s);
    s_c(s,x2); s_byte(s,' '); s_c(s,y); snprintf(S_FMT_BUF, 2048, " l S Q\n"); s_flush(s);
    p->reguas = p->reguas + 1;
}

/* uma POLILINHA: os polinómios de grau um no plano — os troços do desenho, como as
 * splines das cartas. Os pontos em milésimos; juntas redondas para o traço emendar. */
static void poe_poli(Pdf *p, const long *xs, const long *ys, int np, long esp, const char *cor){
    long r, g, b;
    if(!p->aberta || np < 2 || !cor_de(cor, &r, &g, &b)) return;
    Saida *s = &p->sf;
    snprintf(S_FMT_BUF, 2048, "q "); s_flush(s); s_fix(s,r,3); s_byte(s,' '); s_fix(s,g,3);
    s_byte(s,' '); s_fix(s,b,3); snprintf(S_FMT_BUF, 2048, " RG 1 j 1 J "); s_flush(s);
    s_c(s, esp); snprintf(S_FMT_BUF, 2048, " w "); s_flush(s);
    s_c(s, xs[0]); s_byte(s,' '); s_c(s, ys[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(s);
    for(int k = 1; k < np; k++){ s_c(s, xs[k]); s_byte(s,' '); s_c(s, ys[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(s); }
    snprintf(S_FMT_BUF, 2048, "S Q\n"); s_flush(s);
    p->reguas = p->reguas + 1;
}

/* Contornos Emmentaler (clefs.G / clefs.F) → milésimos; y=0 = linha da clave.
 * Dados (não função) — o teto MAX_FUN do wasm não cresce. */
#define CLEF_ESP 4394L   /* 1,55 mm = vão do pentagrama */
/* Avanço da clave na régua do Td (10^-3 pt): 0x82→11266, 0x83→11775, 0x84→10000.
 * Contorno Emmentaler em Td absoluto; NÃO é milésimo-de-em × corpo. */
#define N_CG0 37
static const long CG0x[] = {6609,5892,7826,8598,6450,6324,6187,4993,3480,3539,3926,2845,35,3579,6683,7485,7523,7500,6648,4002,2566,2969,4289,4653,3111,1304,2144,5999,8226,8258,8237,8512,9456,11226,10108,6782,6609};
static const long CG0y[] = {4605,7341,11218,16254,20933,20985,20973,19911,15801,13759,11378,7945,1969,-4077,-4568,-4870,-6081,-6808,-9379,-10323,-9263,-9361,-8876,-7000,-5888,-6985,-10148,-10831,-7153,-6387,-5211,-4172,-3656,-967,3205,4620,4605};
#define N_CG1 25
static const long CG1x[] = {1301,2467,5044,5468,5853,3774,3041,3542,5097,5176,5255,5528,5642,5613,5519,4438,4095,4664,6275,6907,7382,6668,5941,2731,1301};
static const long CG1y[] = {492,4291,7575,6037,4499,3197,1037,-767,-2215,-2239,-2250,-2129,-1863,-1731,-1599,-598,562,1938,2654,-492,-3744,-3823,-3849,-2568,492};
#define N_CG2 9
static const long CG2x[] = {7171,7452,7523,6767,4956,4521,4324,5095,7171};
static const long CG2y[] = {18367,17266,16100,13061,10405,12024,13815,16499,18367};
#define N_CG3 7
static const long CG3x[] = {8103,7641,7048,9274,10089,9544,8103};
static const long CG3y[] = {-3568,-411,2654,1505,-685,-2423,-3568};
#define N_CF0 21
static const long CF0x[] = {4078,549,276,1969,3489,3485,1969,1565,1979,4078,6726,5995,-158,-222,-197,0,105,4486,9227,6756,4078};
static const long CF0y[] = {4622,2485,-610,-1529,-602,1219,2092,2026,3088,4166,2043,-3531,-8630,-8735,-8902,-9016,-8988,-6764,211,4115,4622};
#define N_CF1 7
static const long CF1x[] = {9790,10235,11137,11583,11137,10235,9790};
static const long CF1y[] = {2197,1424,1424,2197,2970,2970,2197};
#define N_CF2 7
static const long CF2x[] = {9790,10235,11137,11583,11137,10235,9790};
static const long CF2y[] = {-2197,-2970,-2970,-2197,-1424,-1424,-2197};

static int obj_novo(Pdf *p){
    p->nobj = p->nobj + 1; p->off[p->nobj] = s_pos(&p->sf);
    return p->nobj;
}

static void pdf_abre(Pdf *p, unsigned char *buf, long cap){
    memset(p, 0, sizeof *p);
    p->off = (long*)disco_buf(12, (long)(MAXOBJ * sizeof(long)));
    p->pag = (int *)disco_buf(13, (long)(MAXOBJ * sizeof(int)));
    p->sf.buf = buf; p->sf.cur = 0; p->sf.len = 0; p->sf.cap = (int)cap;
    /* cabeçalho por s_byte — sem formatador (a prova do caminho do slot) */
    { const char *h = "%PDF-1.4\n%";
      while(*h){ s_byte(&p->sf, (unsigned char)*h); h = h + 1; }
      s_byte(&p->sf, 0xE2); s_byte(&p->sf, 0xE3); s_byte(&p->sf, 0xCF); s_byte(&p->sf, 0xD3);
      s_byte(&p->sf, 10); }
    p->nobj = 3 + N_FIXA;                      /* 1 catálogo, 2 páginas, N_FIXA fontes,
                                                * e o 19: os Resources partilhados */
}

/* abre um stream cujo /Length é INDIRECTO (aponta um objecto que se fecha depois): regista o offset,
 * escreve o cabeçalho, e devolve o início do stream. Era a mesma sequência para o conteúdo e o fundo. */
static long abre_stream_ind(Pdf *p, int obj, int lenobj){
    p->off[obj] = s_pos(&p->sf);
    snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Length %d 0 R>>stream\n", obj, lenobj); s_flush(&p->sf);
    return s_pos(&p->sf);
}

static void pagina_abre(Pdf *p){
    p->plana = 0;                    /* fancy é o default; quem abre plain marca depois */
    p->num = p->num + 1; p->sem_pe = 0;
    /* DOIS STREAMS, E O /Contents ACEITA UM ARRAY.
     *
     * Um stream de PDF é sequencial: o que se escreve depois pinta por cima. Por isso o fundo
     * de uma caixa não podia vir no fim — tapava o texto —, e ficou por ligar quando a barra
     * já estava. A saída não é guardar a página em memória (não há RAM aqui): é o próprio
     * formato. O /Contents aceita [A B], e o leitor CONCATENA os dois na ordem em que estão.
     *
     *      stream A   os fundos    escrito num temporário enquanto a página corre
     *      stream B   o texto      escrito direto, como sempre foi
     *
     * Assim o fundo fica por baixo sem se saber a altura da caixa antes de a fechar — porque a
     * ordem em que se ESCREVE deixou de ser a ordem em que se PINTA. É a mesma separação que o
     * sistema faz em toda a parte: o que se guarda e o que se lê são dois sentidos, e aqui os
     * dois streams são os dois sentidos da página. */
    int po = obj_novo(p);
    p->pag[p->npag] = po; p->npag = p->npag + 1;
    int fo = po + 1, flo = po + 2;                 /* o fundo e o seu /Length */
    int co = po + 3, lo  = po + 4;                 /* o texto e o seu /Length */
    p->nobj = lo;
    /* O DICIONARIO DECLARA AS N_FIXA, e o laco do fecho escreve as N_FIXA. As paginas
     * fecham-se ANTES de se saber quantos pares (variante, corpo) o documento usou, e por
     * isso o numero e' fixo e generoso: os que sobram apontam para a fonte base e nao
     * custam nada. Declarar menos do que se usa deixa `/F` sem objecto, e o leitor recusa
     * a pagina inteira. */
    char dicf[N_FIXA * 20 + 4]; int dl = 0;
    for(int k = 1; k <= N_FIXA; k++){
        char *q = ap_str(dicf + dl, "/F"); q = ap_num(q, k); q = ap_str(q, " ");
        q = ap_num(q, 2 + k); q = ap_str(q, " 0 R"); *q = 0; dl = (int)(q - dicf);
    }
    snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 ", po); s_flush(&p->sf);
    s_fix(&p->sf, A4_LM, 3); s_byte(&p->sf, ' '); s_fix(&p->sf, A4_AM, 3);
    (void)dicf;
    snprintf(S_FMT_BUF, 2048, "]/Resources %d 0 R/Contents[%d 0 R %d 0 R]>>endobj\n", 3 + N_FIXA, fo, co); s_flush(&p->sf);
    /* o fundo vai para um slot próprio e só se copia no fecho — é lá que se sabe o que ele tem */
    p->sfundo.buf = (unsigned char*)disco_buf(15, 1L << 20);
    p->sfundo.cur = 0; p->sfundo.len = 0; p->sfundo.cap = 1L << 20;
    p->fundo_on = 1;
    p->n_fundo = 0;
    p->fo = fo; p->flo = flo;
    p->stream_ini = abre_stream_ind(p, co, lo);      /* o stream do conteúdo */
    p->len_obj = lo;
    p->y = TOPO;
    p->aberta = 1;
    p->abriu_agora = 1;
}

/* fecha um stream cujo /Length é INDIRECTO: o `endstream endobj`, e depois o objecto do comprimento
 * (`N 0 obj LEN endobj`) com o offset registado. Era a mesma sequência para o conteúdo e o fundo. */
static void fecha_stream_ind(Pdf *p, long lenobj, long len){
    snprintf(S_FMT_BUF, 2048, "endstream\nendobj\n"); s_flush(&p->sf);
    p->off[lenobj] = s_pos(&p->sf);
    snprintf(S_FMT_BUF, 2048, "%ld 0 obj %ld endobj\n", lenobj, len); s_flush(&p->sf);
}

static void poe_pedaco(Saida *f, const Gl *g, int i, int j, int fonte, long corpo,
                       long x_m, long y_m, long espaco_extra);
static void pagina_fecha(Pdf *p){
    if(!p->aberta) return;
    /* O PÉ DE PÁGINA: o número, centrado na coluna, no vão MEDIDO do gabarito —
     * a baseline fica 31,433 pt abaixo do bloco de texto (42,27 pt do fundo do papel
     * no gabarito de margem 2,6 cm). Não é um número escolhido: foi lido do pdflatex. */
    {   /* o pé e o cabeçalho são \gknota (o degrau 0) cor `regua`, como o estilo manda */
        char cor_fora[24]; { char *q = ap_str(cor_fora, COR_TEXTO); *q = 0; }
        { char *q = ap_str(COR_TEXTO, "regua"); *q = 0; }
        long corpo = escala_de_degrau(0, EIXO_ESCALA);
        long guarda = CORPO_CORRENTE; CORPO_CORRENTE = corpo;
        char np[12]; { char *q = ap_num(np, p->num); *q = 0; }   /* a série do book */
        Gl g[96]; int ng = 0;
        for(int k = 0; np[k] && ng < 12; k++){ g[ng].g = (unsigned char)np[k];
                                               g[ng].f = 0; g[ng].e = 0; ng++; }
        long w6 = 0;
        for(int k = 0; k < ng; k++) w6 += (long)largura(g[k].g, 0) * corpo;
        long x = MARGEM * 1000L + (COL * 1000L - w6 / 1000) / 2;
        if(!p->sem_pe)
            poe_pedaco(&p->sf, g, 0, ng, 0, corpo, x, MARGEM * PT - 31433, 0);
        /* O CABEÇALHO DO FANCYHDR, só quando há capítulo aberto (a marca existe): a
         * esquerda é o \fancyhead[L] lido do estilo, a direita é o \leftmark, os dois
         * em versaletes, com a régua de 0,4 pt por baixo — baseline MEDIDA no gabarito
         * (47,12 pt do topo do papel). */
        if(CAB_ESQ[0] && !p->plana){
            int fv = N_CARTA > F_VER ? F_VER : F_REG;
            long yc = A4_AM - 47120;
            ng = 0;
            for(int k = 0; CAB_ESQ[k] && ng < 96; k++){ g[ng].g = (unsigned char)CAB_ESQ[k];
                                                        g[ng].f = (unsigned char)fv; g[ng].e = 0; ng++; }
            poe_pedaco(&p->sf, g, 0, ng, fv, corpo, MARGEM * 1000L, yc, 0);
            ng = 0; w6 = 0;
            for(int k = 0; CAB_DIR[k] && ng < 96; k++){ g[ng].g = (unsigned char)CAB_DIR[k];
                                                        g[ng].f = (unsigned char)fv; g[ng].e = 0; ng++; }
            for(int k = 0; k < ng; k++) w6 += (long)largura(g[k].g, fv) * corpo;
            poe_pedaco(&p->sf, g, 0, ng, fv, corpo, (MARGEM + COL) * 1000L - w6 / 1000, yc, 0);
            poe_regua(p, MARGEM * 1000L, (MARGEM + COL) * 1000L, yc - 3000, 400, "regua");
        }
        CORPO_CORRENTE = guarda;
        { char *q = ap_str(COR_TEXTO, cor_fora); *q = 0; }
    }
    long fim = s_pos(&p->sf);
    fecha_stream_ind(p, p->len_obj, fim - p->stream_ini);        /* o conteúdo */
    /* e agora o PRIMEIRO stream — o fundo. Escreve-se DEPOIS no ficheiro e é lido ANTES pelo
     * leitor, porque o /Contents já diz a ordem. A posição no ficheiro e a ordem de pintura
     * deixaram de ser a mesma coisa, e é isso que resolve o problema. */
    long fi = abre_stream_ind(p, p->fo, p->flo);      /* o stream do fundo */
    if(p->fundo_on){
        s_bytes(&p->sf, p->sfundo.buf, p->sfundo.len);   /* o fundo inteiro para a saída, de uma vez */
        p->fundo_on = 0;
    }
    long ff = s_pos(&p->sf);
    fecha_stream_ind(p, p->flo, ff - fi);                        /* o fundo */
    p->aberta = 0;
}

/* escreve um pedaço de glifos numa só fonte, escapando o que o PDF exige */
/* ═══ O DESENHO PELAS ASSINATURAS: cada glifo escreve-se UMA vez ════════════════════
 *
 * A assinatura de um caractere é o seu contorno na carta — polinómios no plano, grau 2
 * na glyf e grau 3 na CFF. Ela escreve-se UMA vez, em unidades da fonte (inteiros,
 * exactos), como um Form XObject; cada uso na página é só posição e escala (`cm /G Do`)
 * — o corpo é campo, não opcode, como o MOVE que serve quinhentos corpos. */
#define MAX_XGC 32
static const Ttf *XG_CARTA[MAX_XGC];
static int N_XGC = 0;
static unsigned char XG_USADO[MAX_XGC * 256];   /* achatado: o traduz indexa 1D */
static Contorno XG_CT;                          /* um contorno de cada vez, sem malloc */
static int XG_ID[MAX_XGC * 256];
static long CAM_DX[2048], CAM_DY[2048], CAM_AC[2049];  /* rasto do relógio: fora do quadro */

static int xg_idx(const Ttf *carta){
    for(int i = 0; i < N_XGC; i++) if(XG_CARTA[i] == carta) return i;
    if(N_XGC < MAX_XGC){ int r = N_XGC; XG_CARTA[r] = carta; N_XGC = N_XGC + 1; return r; }
    return 0;
}

/* o caminho de um Contorno, em UNIDADES DA FONTE — inteiros, sem um arredondamento:
 * grau 3 sai como `c`; a quadrática sobe a cúbica pela linha de Pascal (2/3), com o
 * ponto implícito entre dois controlos seguidos da glyf */
/* (o ponto do contorno lê-se inteiro por contorno_xi/yi do spline.h — a fracção da
 * CFF arredonda-se onde ela mora, e aqui só entram inteiros) */

/* avalia o troço em t = j/S por de Casteljau INTEIRO: numerador exacto, UMA divisão
 * arredondada por coordenada — a do escrever. Cúbica em S³, quadrática em S². */
static long rasto_c3(long p0, long p1, long p2, long p3, long j, long S){
    long u = S - j;
    long num = u*u*u*p0 + 3*u*u*j*p1 + 3*u*j*j*p2 + j*j*j*p3, den = S*S*S;
    return (2*num + (num >= 0 ? den : -den)) / (2*den);
}
static long rasto_c2(long p0, long p1, long p2, long j, long S){
    long u = S - j;
    long num = u*u*p0 + 2*u*j*p1 + j*j*p2, den = S*S;
    return (2*num + (num >= 0 ? den : -den)) / (2*den);
}

static void escreve_caminho(Saida *f, const Contorno *ct, int cff){
    /* A ASSINATURA EXTRAI-SE DA REFERÊNCIA UMA VEZ, E A REFERÊNCIA DISPENSA-SE.
     * A cadeia de Béziers da carta é o gabarito: dela tira-se o rasto denso (de
     * Casteljau inteiro), e sobre ele o RELÓGIO põe as suas marcas — N pontos
     * uniformes no tempo dele (o arco, na régua L1 da grelha), com N a DOBRAR até a
     * corda fechar na régua (upem/100). O desenho que vai ao papel é SÓ as marcas do
     * relógio: a segmentação da referência não aparece — foi consultada e dispensada.
     * Sem π, sem raiz, sem double; a divisão é a do escrever. (A lei operacional:
     * tests/luz_periodo.c, tests/relogio_curva.c — quem decide é o relógio.) */
    long REGUA = 10;                                /* upem/100: o por-unidade */
    int a = 0;
    for(int cc = 0; cc < ct->nc; cc++){
        int z2 = ct->fim[cc]; int np = z2 - a + 1;
        if(np < 2){ a = z2 + 1; continue; }
        int s0 = -1;
        for(int t = a; t <= z2; t++) if(ct->p[t].onda){ s0 = t; break; }
        if(s0 < 0){ a = z2 + 1; continue; }
        /* 1. o rasto denso da REFERÊNCIA: S=8 por troço, pontos SOBRE a curva */
        int nd = 0;
        for(int t2 = 1; t2 <= np; ){
            int q1 = a + (s0 - a + t2) % np;
            long P0x = contorno_xi(ct, a + (s0 - a + t2 - 1) % np);
            long P0y = contorno_yi(ct, a + (s0 - a + t2 - 1) % np);
            if(ct->p[q1].onda){
                if(nd < 2046){ CAM_DX[nd] = P0x; CAM_DY[nd] = P0y; nd++;
                               CAM_DX[nd] = contorno_xi(ct, q1); CAM_DY[nd] = contorno_yi(ct, q1); nd++; }
                t2++; continue;
            }
            long C1x = contorno_xi(ct, q1), C1y = contorno_yi(ct, q1);
            long C2x, C2y, P3x, P3y; int grau3, salto;
            int q2 = a + (s0 - a + t2 + 1) % np;
            if(cff){
                int q3 = a + (s0 - a + t2 + 2) % np;
                C2x = contorno_xi(ct, q2); C2y = contorno_yi(ct, q2);
                P3x = contorno_xi(ct, q3); P3y = contorno_yi(ct, q3);
                grau3 = 1; salto = 3;
            } else {
                if(ct->p[q2].onda || q2 == q1){
                    P3x = contorno_xi(ct, q2); P3y = contorno_yi(ct, q2); salto = 2;
                } else {
                    P3x = (C1x + contorno_xi(ct, q2) + 1) / 2;
                    P3y = (C1y + contorno_yi(ct, q2) + 1) / 2; salto = 1;
                }
                C2x = C2y = 0; grau3 = 0;
            }
            for(long j = 0; j < 8 && nd < 2047; j++){
                long x, y;
                if(grau3){ x = rasto_c3(P0x, C1x, C2x, P3x, j, 8);
                           y = rasto_c3(P0y, C1y, C2y, P3y, j, 8); }
                else     { x = rasto_c2(P0x, C1x, P3x, j, 8);
                           y = rasto_c2(P0y, C1y, P3y, j, 8); }
                CAM_DX[nd] = x; CAM_DY[nd] = y; nd++;
            }
            t2 += salto;
        }
        if(nd < 3){ a = z2 + 1; continue; }
        /* 2. o arco L1 (a régua da grelha, inteira) acumulado sobre o rasto denso */
        CAM_AC[0] = 0;
        for(int k = 0; k < nd; k++){
            int k2 = (k + 1) % nd;
            long dx = CAM_DX[k2] - CAM_DX[k], dy = CAM_DY[k2] - CAM_DY[k];
            if(dx < 0) dx = -dx;
            if(dy < 0) dy = -dy;
            CAM_AC[k + 1] = CAM_AC[k] + dx + dy;
        }
        long L = CAM_AC[nd];
        if(L <= 0){ a = z2 + 1; continue; }
        /* 3. as MARCAS DO RELÓGIO: N uniformes no arco, N dobra até a corda fechar —
         * mede-se no ponto do rasto denso mais próximo do meio de cada corda */
        long N = 16;
        for(int dob = 0; dob < 6; dob++){
            long pior = 0; int jj = 0;
            for(long m = 0; m < N; m++){
                long a0 = L * m / N, a1 = L * (m + 1) / N, am = (a0 + a1) / 2;
                while(jj + 1 < nd && CAM_AC[jj + 1] < am) jj++;
                long segm = CAM_AC[jj+1] - CAM_AC[jj];
                long mxi = CAM_DX[jj] + (segm ? (CAM_DX[(jj+1)%nd] - CAM_DX[jj]) * (am - CAM_AC[jj]) / segm : 0);
                long myi = CAM_DY[jj] + (segm ? (CAM_DY[(jj+1)%nd] - CAM_DY[jj]) * (am - CAM_AC[jj]) / segm : 0);
                /* a corda desta marca */
                long x0, y0, x1, y1; int j0 = 0, j1 = 0; (void)j0; (void)j1;
                { int t3 = 0; while(t3 + 1 < nd && CAM_AC[t3 + 1] < a0) t3++;
                  long seg = CAM_AC[t3+1] - CAM_AC[t3];
                  x0 = CAM_DX[t3] + (seg ? (CAM_DX[(t3+1)%nd] - CAM_DX[t3]) * (a0 - CAM_AC[t3]) / seg : 0);
                  y0 = CAM_DY[t3] + (seg ? (CAM_DY[(t3+1)%nd] - CAM_DY[t3]) * (a0 - CAM_AC[t3]) / seg : 0); }
                { int t3 = 0; while(t3 + 1 < nd && CAM_AC[t3 + 1] < a1) t3++;
                  long seg = CAM_AC[t3+1] - CAM_AC[t3];
                  x1 = CAM_DX[t3] + (seg ? (CAM_DX[(t3+1)%nd] - CAM_DX[t3]) * (a1 - CAM_AC[t3]) / seg : 0);
                  y1 = CAM_DY[t3] + (seg ? (CAM_DY[(t3+1)%nd] - CAM_DY[t3]) * (a1 - CAM_AC[t3]) / seg : 0); }
                long dx = mxi - (x0 + x1) / 2, dy = myi - (y0 + y1) / 2;
                if(dx < 0) dx = -dx;
                if(dy < 0) dy = -dy;
                if(dx > pior) pior = dx;
                if(dy > pior) pior = dy;
            }
            if(pior < REGUA || N >= 512) break;
            N = N * 2;
        }
        /* 4. o desenho: SÓ as marcas — a referência já foi dispensada */
        for(long m = 0; m <= N; m++){
            long am = L * m / N;
            int t3 = 0; while(t3 + 1 < nd && CAM_AC[t3 + 1] < am) t3++;
            long seg = CAM_AC[t3+1] - CAM_AC[t3];
            long x = CAM_DX[t3] + (seg ? (CAM_DX[(t3+1)%nd] - CAM_DX[t3]) * (am - CAM_AC[t3]) / seg : 0);
            long y = CAM_DY[t3] + (seg ? (CAM_DY[(t3+1)%nd] - CAM_DY[t3]) * (am - CAM_AC[t3]) / seg : 0);
            snprintf(S_FMT_BUF, 2048, "%ld %ld %s ", x, y, m == 0 ? "m" : "l"); s_flush(f);
        }
        snprintf(S_FMT_BUF, 2048, "h "); s_flush(f);
        a = z2 + 1;
    }
    snprintf(S_FMT_BUF, 2048, "f"); s_flush(f);
}

static void poe_pedaco(Saida *f, const Gl *g, int i, int j, int fonte, long corpo,
                       long x_m, long y_m, long espaco_extra){
    /* O TEXTO DESENHA-SE PELAS ASSINATURAS: cada glifo é `cm /G Do` — a posição e a
     * escala corpo/upem sobre o contorno escrito UMA vez no XObject. Não há Tj: o
     * `.tex` viaja no FonteTeX e a volta exacta é por ele. */
    { long r, gg, b;
      if(COR_TEXTO[0] && cor_de(COR_TEXTO, &r, &gg, &b)){
          snprintf(S_FMT_BUF, 2048, "q "); s_flush(f); s_fix(f, r, 3); s_byte(f, ' ');
          s_fix(f, gg, 3); s_byte(f, ' '); s_fix(f, b, 3); snprintf(S_FMT_BUF, 2048, " rg\n"); s_flush(f);
      } else snprintf(S_FMT_BUF, 2048, "q 0 0 0 rg\n"); s_flush(f); }
    long guarda = CORPO_CORRENTE; CORPO_CORRENTE = corpo;
    const Ttf *carta = (fonte == F_SIM && CARTA_SIM) ? &CARTAS[F_SIM]
                     : (fonte == F_MAT && CARTA_MAT) ? &CARTAS[F_MAT]
                     : (fonte == F_MTB && CARTA_MTB) ? &CARTAS[F_MTB]
                     : (fonte == F_SMB && CARTA_SMB) ? &CARTAS[F_SMB]
                     : (fonte == F_NIT && CARTA_NIT) ? &CARTAS[F_NIT]
                                                     : carta_do_corpo(fonte, corpo);
    long x = x_m;
    for(int k = i; k < j; k++){
        int gb = g[k].g;
        long av = (long)largura(gb, fonte) * corpo / 1000;
        if(carta && carta->upem && gb > ' '){
            int uni = (fonte == F_SIM || fonte == F_MAT || fonte == F_MTB
                    || fonte == F_SMB)
                    ? gb : (gb < 0x80 ? gb : winansi_para_unicode(gb));
            int gi = ttf_glifo(carta, uni);
            if(gi){
                int ix = xg_idx(carta);
                XG_USADO[ix * 256 + gb] = 1;
                /* a escala corpo/upem em milionésimos: UMA divisão, a do escrever —
                 * e a glyf mora no espaço ×6 (a elevação exacta), que a escala desfaz */
                long sc = corpo * 1000 / carta->upem;   /* o rasto escreve em unidades da fonte, x1 */
                snprintf(S_FMT_BUF, 2048, "q "); s_flush(f); s_fix(f, sc, 6); snprintf(S_FMT_BUF, 2048, " 0 0 "); s_flush(f); s_fix(f, sc, 6);
                s_byte(f, ' '); s_fix(f, x, 3); s_byte(f, ' '); s_fix(f, y_m, 3);
                snprintf(S_FMT_BUF, 2048, " cm /G%d_%d Do Q\n", ix, gb); s_flush(f);
            }
        }
        x += av;
        if(gb == ' ') x += espaco_extra;
    }
    snprintf(S_FMT_BUF, 2048, "Q\n"); s_flush(f);
    CORPO_CORRENTE = guarda;
}

/* UM GLIFO COM BOOST: a instância assimétrica `q sh 0 0 sv x y cm` com sh·sv = sc² —
 * o delimitador que estica pela ÁREA INTERNA conserva a área da tinta (|det|=1, o
 * boost incompressível de tests/curvatura.c): sobe pela hipérbole, não pelo círculo. */
static void poe_glifo_boost(Pdf *p, int g, int fonte, long corpo,
                            long x_m, long y_m, long k_num, long k_den){
    const Ttf *carta = (fonte == F_SIM && CARTA_SIM) ? &CARTAS[F_SIM]
                     : (fonte == F_MAT && CARTA_MAT) ? &CARTAS[F_MAT]
                                                     : carta_do_corpo(fonte, corpo);
    if(!carta || !carta->upem) return;
    int uni = (fonte == F_SIM || fonte == F_MAT) ? g : (g < 0x80 ? g : winansi_para_unicode(g));
    int gi = ttf_glifo(carta, uni);
    if(!gi) return;
    int ix = xg_idx(carta);
    XG_USADO[ix * 256 + g] = 1;
    long sc = corpo * 1000 / carta->upem;
    /* a fronteira estica na vertical (sv = sc·k); a largura fica a NECESSÁRIA do
     * traço (sh = sc) — «W = W_necessário + 2m» — e a área da região transforma
     * por sx·sy, declarada na própria instância: quem mede, lê os dois. */
    long sh = sc, sv = sc * k_num / k_den;
    Saida *f = &p->sf;
    snprintf(S_FMT_BUF, 2048, "q 0 0 0 rg\n"); s_flush(f);
    snprintf(S_FMT_BUF, 2048, "q "); s_flush(f); s_fix(f, sh, 6); snprintf(S_FMT_BUF, 2048, " 0 0 "); s_flush(f); s_fix(f, sv, 6);
    s_byte(f, ' '); s_fix(f, x_m, 3); s_byte(f, ' '); s_fix(f, y_m, 3);
    snprintf(S_FMT_BUF, 2048, " cm /G%d_%d Do Q\nQ\n", ix, g); s_flush(f);
}

/* a espessura do traço desenhado (raiz, barra, moldura) no peso do CONTEXTO: a
 * regular é 400; a negra engrossa pela razão da PRÓPRIA referência — o avanço do
 * ponto na carta negra sobre a regular. Derivada, cacheada, uma divisão. */
static long esp_traco(int fonte){
    static long esp_neg = 0;
    if(fonte != F_NEG && fonte != F_MTB) return SEM_V[4];
    if(!esp_neg){
        long wr = largura('.', F_REG), wn = largura('.', F_NEG);
        esp_neg = (wr > 0 && wn > wr) ? SEM_V[4] * wn / wr : SEM_V[4];
    }
    return esp_neg;
}

/* o LUNAR desenrola uma linha na página, deformando o espaço se for para justificar */
/* desenha uma linha numa POSIÇÃO dada, e desce o `y` só se `desce` — o sumário precisa de
 * pôr o texto à esquerda e o número à direita NA MESMA linha, e o `desenrola` normal desce
 * sempre. Sem isto o número caía na linha seguinte. */
/* pinta meia pilha do \frac (o vão [a,b) do numerador ou do denominador): as
 * sub-corridas internas — os expoentes ±6/±7 — sobem/descem a dobra DENTRO do bloco */
static long pinta_meia_pilha(Pdf *p, const Linha *L, int a, int b, long corpo,
                             long x, long base_y, long dv){
    long xn = x;
    int k = a;
    while(k < b){
        int m = k, f2 = L->g[k].f, e2 = L->g[k].e;
        while(m < b && L->g[m].f == f2 && L->g[m].e == e2) m++;
        long cp2 = corpo_exp_m(corpo, e2);
        long w6 = 0;
        for(int t = k; t < m; t++) w6 += (long)largura(L->g[t].g, f2) * cp2;
        long dy = (e2 == 6 || e2 == -6) ? dv : (e2 == 7 || e2 == -7) ? -dv : 0;
        poe_pedaco(&p->sf, L->g, k, m, f2, cp2, xn, base_y + dy, 0);
        xn += w6 / 1000;
        k = m;
    }
    return xn - x;
}

static void desenrola_em(Pdf *p, const Linha *L, long x0_m, int desce){
    if(!L->n){ if(desce) p->y -= escala_entre(D_TEXTO); return; }
    /* E ABRE PAGINA quando nao cabe — o `desenrola` normal fa-lo e este nao fazia: o
     * sumario inteiro caia numa pagina so', com o `y` a descer para negativo. O gabarito
     * gasta NOVE paginas com ele. */
    if(p->y < MARGEM*PT + escala_entre(D_TEXTO)){ pagina_fecha(p); pagina_abre(p); }
    long corpo = escala_corpo(L->deg >= 0 ? L->deg : D_TEXTO);
    CORPO_CORRENTE = corpo;
    /* O X ACUMULA EXACTO: o produto largura(por-mil) × corpo(mantissa) soma em 10^-6 sem
     * dividir por glifo — a divisão é UMA por pedaço, a do Td que se escreve. Dividir no
     * meio amputa, e o amputado soma ao longo da linha. */
    int i = 0; long xm = x0_m;
    while(i < L->n){
        int gk = L->g[i].g;
        /* Claves musicais (Emmentaler): 0x82 sol, 0x83 fá, 0x84 percussão.
         * Pintam na baseline da linha — mesma âncora das notas. */
        if(gk == 0x82 || gk == 0x83 || gk == 0x84){
            long x0 = xm;
            long yb = p->y;
            long yref = yb;
            if(gk == 0x82) yref = yb - CLEF_ESP;
            else if(gk == 0x83) yref = yb + CLEF_ESP;
            long rr, gg, bb;
            if(p->aberta && cor_de("tinta", &rr, &gg, &bb)){
                Saida *sf = &p->sf;
                snprintf(S_FMT_BUF, 2048, "q "); s_flush(sf);
                s_fix(sf, rr, 3); s_byte(sf, ' '); s_fix(sf, gg, 3);
                s_byte(sf, ' '); s_fix(sf, bb, 3);
                snprintf(S_FMT_BUF, 2048, " rg "); s_flush(sf);
                if(gk == 0x82){
                    s_c(sf, x0 + CG0x[0]); s_byte(sf, ' '); s_c(sf, yref + CG0y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG0; k++){ s_c(sf, x0 + CG0x[k]); s_byte(sf, ' '); s_c(sf, yref + CG0y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CG1x[0]); s_byte(sf, ' '); s_c(sf, yref + CG1y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG1; k++){ s_c(sf, x0 + CG1x[k]); s_byte(sf, ' '); s_c(sf, yref + CG1y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CG2x[0]); s_byte(sf, ' '); s_c(sf, yref + CG2y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG2; k++){ s_c(sf, x0 + CG2x[k]); s_byte(sf, ' '); s_c(sf, yref + CG2y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CG3x[0]); s_byte(sf, ' '); s_c(sf, yref + CG3y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG3; k++){ s_c(sf, x0 + CG3x[k]); s_byte(sf, ' '); s_c(sf, yref + CG3y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h f* Q\n"); s_flush(sf);
                } else if(gk == 0x83){
                    s_c(sf, x0 + CF0x[0]); s_byte(sf, ' '); s_c(sf, yref + CF0y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CF0; k++){ s_c(sf, x0 + CF0x[k]); s_byte(sf, ' '); s_c(sf, yref + CF0y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CF1x[0]); s_byte(sf, ' '); s_c(sf, yref + CF1y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CF1; k++){ s_c(sf, x0 + CF1x[k]); s_byte(sf, ' '); s_c(sf, yref + CF1y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CF2x[0]); s_byte(sf, ' '); s_c(sf, yref + CF2y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CF2; k++){ s_c(sf, x0 + CF2x[k]); s_byte(sf, ' '); s_c(sf, yref + CF2y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h f* Q\n"); s_flush(sf);
                } else {
                    long d = CLEF_ESP / 3;
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb + d); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb + d); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb + d + 900); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb + d + 900); snprintf(S_FMT_BUF, 2048, " l h "); s_flush(sf);
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb - d - 900); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb - d - 900); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb - d); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb - d); snprintf(S_FMT_BUF, 2048, " l h f Q\n"); s_flush(sf);
                }
                p->reguas = p->reguas + 1;
            }
            xm += (gk == 0x82) ? 11266L : (gk == 0x83) ? 11775L : 10000L;
            i++; continue;
        }
        int j = i, fonte = L->g[i].f, ex = L->g[i].e;
        while(j < L->n && L->g[j].f == fonte && L->g[j].e == ex
              && L->g[j].g != 0x82 && L->g[j].g != 0x83 && L->g[j].g != 0x84) j++;
        /* o expoente é o mesmo corpo com o degrau composto — a dobra, inteira —, posto na
         * subida que a escala libertou (o dual aditivo, com o sinal da Lei 1) */
        long cpm = corpo_exp_m(corpo, ex);
        poe_pedaco(&p->sf, L->g, i, j, fonte, cpm, xm, p->y + sobe_exp_m(corpo, ex), 0);
        long w6 = 0;
        for(int k = i; k < j; k++) w6 += (long)largura(L->g[k].g, fonte) * cpm;
        xm += w6 / 1000;                    /* UMA divisão por pedaço: a régua do Td */
        i = j;
    }

    if(desce) p->y -= escala_entre(D_TEXTO);
}

/* A REGIÃO DA MATRIZ, UMA RÉGUA SÓ — e DINÂMICA (a lei 8 dual: a mesma lei
 * reaplica-se ao corpo que ela constrói). As filas empilham à volta do eixo pela
 * altura MEDIDA das células — a pilha do \frac lá dentro incluída — e esta régua
 * é a que o ramo que pinta E os medidores de vão (a linha, o \bigl, o ∫) leem:
 * a fronteira que a envolve cobre esta área mais um ponto de semente (a margem,
 * posta por quem chama). Devolve o fim do bloco; topo e fundo saem RELATIVOS à
 * baseline; ft/ff/nf, se pedidos, dão as filas a quem pinta. */
static int matriz_regiao(const Gl *g, int n, int k0, long corpo,
                         long *topo, long *fundo, long *ft, long *ff, int *nf_out){
    int ex = g[k0].e;
    int jj = k0;
    while(jj < n){
        int e2 = g[jj].e;
        if(e2 == ex){ jj++; continue; }
        if(e2 == 2 || e2 == -2 || e2 >= 16){
            int k2 = jj;
            while(k2 < n && (g[k2].e == 2 || g[k2].e == -2 || g[k2].e >= 16)) k2++;
            if(k2 < n && g[k2].e == ex){ jj = k2; continue; }
        }
        break;
    }
    long dv = corpo - corpo_exp_m(corpo, 1);
    long cb0 = corpo_exp_m(corpo, ex);
    long fta[16], ffa[16];
    if(!ft) ft = fta;
    if(!ff) ff = ffa;
    for(int t = 0; t < 16; t++){ ft[t] = sem_asc(cb0); ff[t] = -(sem_desc(cb0)); }
    int r4 = 0, nf = 1;
    for(int k = k0; k < jj; k++){
        int gk = g[k].g;
        if(gk == 3){ nf++; if(r4 < 15) r4++; continue; }
        if(gk < ' ') continue;
        int eb = g[k].e;
        long cb2 = corpo_exp_m(corpo, eb), sb = sobe_exp_m(corpo, eb);
        if(eb == 4)  sb =  2 * dv + sem_resp(dv);     /* a pilha dentro da célula */
        if(eb == -4) sb = -2 * dv - sem_resp(dv);
        if(r4 < 16){
            if(sb + sem_asc(cb2) > ft[r4]) ft[r4] = sb + sem_asc(cb2);
            if(sb - sem_desc(cb2) < ff[r4]) ff[r4] = sb - sem_desc(cb2);
        }
    }
    long total_h = (long)(nf - 1) * sem_resp(dv);
    for(int t = 0; t < nf && t < 16; t++) total_h += ft[t] - ff[t];
    *topo  = dv + total_h / 2;
    *fundo = dv - total_h / 2;
    if(nf_out) *nf_out = nf;
    return jj;
}

/* `\boxed` multi-linha: o 8 abre numa linha, o 9 fecha noutra (frase longa em
 * display). Sem isto a moldura só existia se 8 e 9 fossem na mesma linha.
 * BX_Y0/Y1 = arestas MEDIDAS (Maestro projectou; Metrónomo lê e atesta). */
static int  BX_ON;
static long BX_X0, BX_Y0, BX_Y1, BX_XM, BX_PAD, BX_M3;
static int  BX_FECHOU;    /* esta linha fechou: atestar fundo λ⁺+λ⁻=0 */
static int  BX_ATESTOU;   /* há aresta inferior válida para o próximo topo */

static void desenrola(Pdf *p, const Linha *L, int justifica){
    /* NUMA CÉLULA NENHUMA linha justifica, e não só a última: numa coluna `l` o alinhamento é
     * à esquerda em todas. A justificação é do parágrafo, e uma célula não é um parágrafo. */
    if(L->larg > 0) justifica = 0;
    /* O CORPO E A ENTRELINHA SAEM DA ESCALA, e o nível escolhe o degrau. Antes eram 10 e 14
     * escritos à mão — e a razão 1,4 em vez de 1,4497, sem hierarquia nenhuma. */
    if(!L->n){ p->y -= escala_entre(D_TEXTO); return; }
    /* a escala corre em MANTISSAS (10^-3, a régua do Tf): a fracção que o Tf aceita já é
     * esta — não há conversão nenhuma a arredondar aqui */
    long corpo = ((L->deg >= 0 ? escala_corpo(L->deg)
                     : L->nivel   ? escala_corpo(L->nivel <= 1 ? d_cap()
                                             : (L->nivel == 2 ? D_SEC : D_SUB))
                                : escala_corpo(D_TEXTO)));
    CORPO_CORRENTE = corpo;   /* o desenho segue o corpo, e quem o sabe é quem compõe */
    /* e a ALTURA DA LINHA sai da mesma escala: entrelinha/corpo = 1,4497 em TODOS os degraus
     * do estilo.tex, e era 1,4 aqui. A diferença é pequena e é o que faz o texto parecer
     * apertado — a entrelinha é o que dá ar à página. */
    long alt  = ((L->nivel ? escala_entre(L->nivel <= 1 ? D_CAP
                                              : (L->nivel == 2 ? D_SEC : D_SUB))
                                : escala_entre(D_TEXTO)));

    /* Quebra de página: se a moldura está aberta, MANTÉM BX_ON — o 9 na página
     * nova ainda precisa das arestas. Só limpa o atestado entre caixas. */
    if(!p->aberta || p->y - alt < FUNDO){
        BX_ATESTOU = 0;
        pagina_fecha(p); pagina_abre(p);
        if(BX_ON) BX_Y1 = p->y + sem_asc(corpo) + BX_PAD;  /* topo nesta página */
    }
    else p->abriu_agora = 0;   /* A BANDEIRA LIMPA-SE AQUI, e não só quando alguém a lê.
                                *
                                * `pagina_abre` levanta-a e ela ficava levantada até ao PRIMEIRO
                                * `&` do documento — que podia vir páginas depois. Esse `&` via-a
                                * e sobrescrevia o `tab_y` com a posição de então, e a primeira
                                * fila de cada tabela nascia desalinhada: a célula 1 no sítio
                                * certo e a 2 uma linha abaixo.
                                *
                                * Uma bandeira que se levanta e espera que alguém a baixe é um
                                * estado que só se LIGA — o mesmo defeito do Tw, e do modo
                                * matemático antes dele. Ela vale para a linha seguinte, e a
                                * linha seguinte é esta. */
    /* A TRANSLAÇÃO VERTICAL É A HORIZONTAL RODADA (o J do transporte: M = J·i,
     * «mover sem perder» — e «o espaçamento SOMA» vale nos dois eixos). A linha
     * mede o seu TOPO e FUNDO reais pelos estados — a espiral, a pilha do \frac,
     * o radicando, a matriz — e o excesso sobre a caixa natural soma na descida:
     * antes o do topo (não tocar a linha de cima), depois o do fundo (não ser
     * tocada pela de baixo). A equação com numerador alto tocava o parágrafo. */
    long lin_topo = 0, lin_fundo = 0;
    int  teve_caixa = 0;
    int  bx_ja = BX_ON;                            /* continuação vs caixa nova */
    { long dvl = corpo - corpo_exp_m(corpo, 1);
      long t0 = sem_asc(corpo), f0 = -(sem_desc(corpo));
      int tem_caixa = BX_ON;   /* continuação: moldura aberta na linha de cima */
      lin_topo = t0; lin_fundo = f0;
      for(int kb = 0; kb < L->n; kb++){
          int eb = L->g[kb].e; int gk2 = L->g[kb].g;
          if(gk2 == 8 || gk2 == 9) tem_caixa = 1;      /* a moldura conta, lá em baixo */
          if(gk2 >= 4 && gk2 <= 14) continue;          /* controlos + marcador underbrace */
          long cb = corpo_exp_m(corpo, eb), sb = sobe_exp_m(corpo, eb);
          if(eb == 4)  sb =  2 * dvl + sem_resp(dvl);
          if(eb == -4) sb = -2 * dvl - sem_resp(dvl);
          if(eb == 5)  sb = dvl;                       /* o vinculum sobe uma dobra */
          if(eb == -3){
              /* underbrace: compõe chaveta+rótulo (corpo_topologico 660, 842). O e=-3
               * sozinho só descia a dobra do script — a moldura cortava o rótulo. */
              int i14 = kb; while(i14 > 0 && L->g[i14].g != 14) i14--;
              long fundo = -(sem_desc(corpo));
              for(int t = i14; t < kb; t++){
                  int et = L->g[t].e, gt = L->g[t].g;
                  if((gt >= 4 && gt <= 14) || et == -3) continue;
                  long ct = corpo_exp_m(corpo, et), st = sobe_exp_m(corpo, et);
                  if(et == 4)  st =  2 * dvl + sem_resp(dvl);
                  if(et == -4) st = -2 * dvl - sem_resp(dvl);
                  if(st - sem_desc(ct) < fundo) fundo = st - sem_desc(ct);
              }
              long h3 = sem_resp(dvl);
              /* mesma geometria do pintor (yr2 - desc) + folga h3 sob a tinta */
              long tip = fundo - 2 * h3;
              long lab = fundo - dvl - sem_asc(cb) - sem_desc(cb) - h3;
              long bot = tip < lab ? tip : lab;
              if(bot < lin_fundo) lin_fundo = bot;
              continue;
          }
          if(eb == 8 || eb == -8){
              /* a região REAL da matriz, pela régua única (o cone mede o que a
               * espiral compôs — o par não se move) */
              long tp, fd;
              kb = matriz_regiao(L->g, L->n, kb, corpo, &tp, &fd, 0, 0, 0) - 1;
              if(tp > lin_topo) lin_topo = tp;
              if(fd < lin_fundo) lin_fundo = fd;
              continue;
          }
          if(sb + sem_asc(cb) > lin_topo) lin_topo = sb + sem_asc(cb);
          if(sb - sem_desc(cb) < lin_fundo)      lin_fundo = sb - sem_desc(cb);
      }
      /* Maestro: pad = λ⁺ projecta a moldura. Externo V: Metrónomo na aresta. */
      if(tem_caixa){
          long dvl2 = dvl; if(dvl2 <= 0) dvl2 = corpo / 5;
          long pad = dvl2 + sem_resp(dvl2);
          lin_topo += pad; lin_fundo -= pad;
          teve_caixa = 1;
      }
      lin_topo  = lin_topo - t0;                       /* só o EXCESSO */
      lin_fundo = f0 - lin_fundo; }
    BX_FECHOU = 0;
    p->y -= alt + lin_topo;
    /* Metrónomo no TOPO: se há aresta inferior atestada e esta linha abre caixa
     * nova, a aresta superior não invade (mesma reflexão λ⁺+λ⁻=0). */
    if(teve_caixa && !bx_ja && BX_ATESTOU){
        long dvl = corpo - corpo_exp_m(corpo, 1);
        if(dvl <= 0) dvl = corpo / 5;
        long pad = dvl + sem_resp(dvl);
        long topo = p->y + sem_asc(corpo) + pad;
        long lim  = BX_Y0 - BX_PAD;
        if(topo > lim) p->y -= (topo - lim);
        BX_ATESTOU = 0;
    }

    long xm = (MARGEM + L->recuo) * 1000L;         /* o x em MILÉSIMOS inteiros, exacto */
    /* Continuação multi-linha do `\boxed`: alinha à aresta já projectada.
     * Caixas FECHADAS na linha podem centrar — o `mede` já leva pad+m3
     * (a mesma lei; sem excepção por página). */
    if(BX_ON){
        justifica = 0;
        xm = BX_X0 + BX_PAD;
    } else if(L->centra && L->larg <= 0){
        justifica = 0;
        long larg_c = mede(L->g, L->n, corpo);
        long sobra_m = (long)(COL - L->recuo) * 1000L - larg_c;
        if(sobra_m > 0) xm += sobra_m / 2;
    }
    { int n8 = 0, n9 = 0;
      for(int t = 0; t < L->n; t++){
          if(L->g[t].g == 8) n8++;
          if(L->g[t].g == 9) n9++;
      }
      if(n8 > n9) justifica = 0;   /* moldura aberta nesta linha: não estica */
    }
    long extra = 0;
    if(justifica && !L->nivel){
        long larg = mede(L->g, L->n, corpo);
        long alvo = (long)((L->larg > 0 ? L->larg : COL - L->recuo)) * 1000;
        int esp = 0;
        /* CONTAM-SE TODOS OS ESPACOS, e nao so' os que estao fora da Symbol. Um espaco e' uma
         * letra em qualquer fonte, e a justificacao estica-o em qualquer uma — excluir os da
         * Symbol fazia com que uma linha com simbolos recebesse menos alargamento do que a
         * conta pedia, e o que faltava aparecia no fim. */
        for(int i = 0; i < L->n; i++) if(L->g[i].g == ' ' && L->g[i].e != -3) esp++;
        if(larg < alvo){
            long resto = deforma(alvo - larg, esp, &extra);
            (void)resto;                            /* medido a sério no §X3 */
        }
    }
    /* parte por fonte: cada troca de fonte é um pedaço, porque um Tj só fala uma fonte —
     * e a troca de expoente também parte, porque o pedaço tem UM corpo e UMA altura */
    int i = 0;
    long seg_x0 = xm; int teve_rotulo = 0; int seg_i14 = 0;
    long caixa_x0 = xm; int caixa_i = -1;   /* -1: ainda não abriu nesta linha */
    long DEL_X[8]; int DEL_I[8], n_del = 0;
    int nfr_i = -1, nfr_j = -1, den_i = -1, den_j = -1;
    long front_kn = 1;    /* a dilatação da fronteira composta (o ∫ pelo vão), como RAZÃO:
                           * «dilatar vira transladar» (a transformada dourada, catalogo
                           * sec:dourada) — no lado dual do limite ela entra na TRANSLAÇÃO,
                           * um produto cruzado no inteiro; a escala do glifo fica no seu
                           * degrau da espiral. Morre no primeiro glifo de nível zero. */
    long front_kd = 1;
    int  front_on = 0;    /* e o PAR de limites é UM eixo (Lei 1, §X8: sobe e desce a
                           * mesma distância): os dois grupos do giro partilham o x — cada
                           * um compõe por dentro em sequência, e o avanço é o máximo */
    int  front_sg = 0;
    long front_x0 = 0, front_wmax = 0;
    while(i < L->n){
        int gk0 = L->g[i].g;
        /* Claves (0x82 sol / 0x83 fá / 0x84 perc): baseline da linha, com as notas. */
        if(gk0 == 0x82 || gk0 == 0x83 || gk0 == 0x84){
            long x0 = xm, yb = p->y, yref = yb;
            if(gk0 == 0x82) yref = yb - CLEF_ESP;
            else if(gk0 == 0x83) yref = yb + CLEF_ESP;
            long rr, gg, bb;
            if(p->aberta && cor_de("tinta", &rr, &gg, &bb)){
                Saida *sf = &p->sf;
                snprintf(S_FMT_BUF, 2048, "q "); s_flush(sf);
                s_fix(sf, rr, 3); s_byte(sf, ' '); s_fix(sf, gg, 3);
                s_byte(sf, ' '); s_fix(sf, bb, 3);
                snprintf(S_FMT_BUF, 2048, " rg "); s_flush(sf);
                if(gk0 == 0x82){
                    s_c(sf, x0 + CG0x[0]); s_byte(sf, ' '); s_c(sf, yref + CG0y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG0; k++){ s_c(sf, x0 + CG0x[k]); s_byte(sf, ' '); s_c(sf, yref + CG0y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CG1x[0]); s_byte(sf, ' '); s_c(sf, yref + CG1y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG1; k++){ s_c(sf, x0 + CG1x[k]); s_byte(sf, ' '); s_c(sf, yref + CG1y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CG2x[0]); s_byte(sf, ' '); s_c(sf, yref + CG2y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG2; k++){ s_c(sf, x0 + CG2x[k]); s_byte(sf, ' '); s_c(sf, yref + CG2y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CG3x[0]); s_byte(sf, ' '); s_c(sf, yref + CG3y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CG3; k++){ s_c(sf, x0 + CG3x[k]); s_byte(sf, ' '); s_c(sf, yref + CG3y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h f* Q\n"); s_flush(sf);
                } else if(gk0 == 0x83){
                    s_c(sf, x0 + CF0x[0]); s_byte(sf, ' '); s_c(sf, yref + CF0y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CF0; k++){ s_c(sf, x0 + CF0x[k]); s_byte(sf, ' '); s_c(sf, yref + CF0y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CF1x[0]); s_byte(sf, ' '); s_c(sf, yref + CF1y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CF1; k++){ s_c(sf, x0 + CF1x[k]); s_byte(sf, ' '); s_c(sf, yref + CF1y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h "); s_flush(sf);
                    s_c(sf, x0 + CF2x[0]); s_byte(sf, ' '); s_c(sf, yref + CF2y[0]); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    for(int k = 1; k < N_CF2; k++){ s_c(sf, x0 + CF2x[k]); s_byte(sf, ' '); s_c(sf, yref + CF2y[k]); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf); }
                    snprintf(S_FMT_BUF, 2048, "h f* Q\n"); s_flush(sf);
                } else {
                    long d = CLEF_ESP / 3;
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb + d); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb + d); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb + d + 900); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb + d + 900); snprintf(S_FMT_BUF, 2048, " l h "); s_flush(sf);
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb - d - 900); snprintf(S_FMT_BUF, 2048, " m "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb - d - 900); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 9000); s_byte(sf, ' '); s_c(sf, yb - d); snprintf(S_FMT_BUF, 2048, " l "); s_flush(sf);
                    s_c(sf, x0 + 1500); s_byte(sf, ' '); s_c(sf, yb - d); snprintf(S_FMT_BUF, 2048, " l h f Q\n"); s_flush(sf);
                }
                p->reguas = p->reguas + 1;
            }
            xm += (gk0 == 0x82) ? 11266L : (gk0 == 0x83) ? 11775L : 10000L;
            i++; continue;
        }
        int j = i, fonte = L->g[i].f, ex = L->g[i].e;
        while(j < L->n && L->g[j].f == fonte && L->g[j].e == ex
                       && !(L->g[j].g >= 4 && L->g[j].g <= 14
                            && L->g[j].e != 8 && L->g[j].e != -8)
                       && L->g[j].g != 0x82 && L->g[j].g != 0x83 && L->g[j].g != 0x84) j++;
        /* A MOLDURA DO \boxed: o 8 regista onde a caixa abre, o 9 desenha o
         * rectângulo em volta — com o respiro do TeX (corpo/5), pelos corpos,
         * como a raiz: um traço fechado de cinco pontos. */
        if(L->g[i].g == 8 || L->g[i].g == 9){
            /* pad=λ⁺, m3=g2. Fecho: Metrónomo lê y0 (thm:metronomo). */
            long cbx = corpo_exp_m(corpo, L->g[i].e);
            long dvl = cbx - corpo_exp_m(corpo, 1);
            if(dvl <= 0 || dvl >= cbx) dvl = cbx / 5;
            long pad = dvl + sem_resp(dvl);
            long m3  = sem_resp(pad);
            if(L->g[i].g == 8){
                xm += m3; caixa_x0 = xm; caixa_i = i; xm += pad;
                BX_ON = 1; BX_X0 = caixa_x0; BX_PAD = pad; BX_M3 = m3;
                BX_Y1 = p->y + sem_asc(corpo) + pad;
                BX_XM = xm;
            }
            else {
                long xs[5], ys[5];
                /* A MOLDURA MEDE O QUE EMBRULHA: topo e fundo saem dos ESTADOS do
                 * conteúdo (a subida da espiral ± a caixa do corpo do nível, e a
                 * pilha do \frac nos seus dois vãos) — a altura fixa cortava a
                 * fração dentro da caixa */
                long y0 = p->y - sem_desc(corpo), y1 = p->y + sem_asc(corpo);
                int ini_cx = caixa_i;
                /* continuação (9 só nesta linha) ou fecho órfão após quebra de
                 * página: nunca indexar g[-1]. */
                if(caixa_i < 0){
                    ini_cx = 0;
                    if(BX_ON || BX_PAD > 0){
                        y1 = BX_Y1;
                        caixa_x0 = BX_X0; pad = BX_PAD; m3 = BX_M3;
                    }
                }
                { long dvb = corpo - corpo_exp_m(corpo, 1);
                  for(int kb = ini_cx; kb < i; kb++){
                      int eb = L->g[kb].e; int gk2 = L->g[kb].g;
                      if(gk2 >= 4 && gk2 <= 14) continue;   /* controlos + marcador ub */
                      long cb = corpo_exp_m(corpo, eb), sb = sobe_exp_m(corpo, eb);
                      if(eb == 4)  sb =  2 * dvb + sem_resp(dvb);
                      if(eb == -4) sb = -2 * dvb - sem_resp(dvb);
                      if(eb == -3){
                          /* compõe chaveta+rótulo do underbrace — senão a moldura
                           * corta o conteúdo (corpo_topologico \[boxed\] 660 e 842) */
                          int i14 = kb; while(i14 > ini_cx && L->g[i14].g != 14) i14--;
                          long fundo = -(sem_desc(corpo));
                          for(int t = i14; t < kb; t++){
                              int et = L->g[t].e, gt = L->g[t].g;
                              if((gt >= 4 && gt <= 14) || et == -3) continue;
                              long ct = corpo_exp_m(corpo, et), st = sobe_exp_m(corpo, et);
                              if(et == 4)  st =  2 * dvb + sem_resp(dvb);
                              if(et == -4) st = -2 * dvb - sem_resp(dvb);
                              if(st - sem_desc(ct) < fundo) fundo = st - sem_desc(ct);
                          }
                          long h3 = sem_resp(dvb);
                          long tip = fundo - 2 * h3;
                          long lab = fundo - dvb - sem_asc(cb) - sem_desc(cb) - h3;
                          long bot = tip < lab ? tip : lab;
                          if(bot < y0 - p->y) y0 = p->y + bot;
                          continue;
                      }
                      if(eb == 8 || eb == -8){
                          /* a região da matriz/array: senão a moldura media glifo
                           * a glifo no exp ±8 e saía uma faixa a cortar o meio */
                          long tp, fd;
                          kb = matriz_regiao(L->g, L->n, kb, corpo, &tp, &fd, 0, 0, 0) - 1;
                          if(tp > y1 - p->y) y1 = p->y + tp;
                          if(fd < y0 - p->y) y0 = p->y + fd;
                          continue;
                      }
                      if(sb + sem_asc(cb) > y1 - p->y) y1 = p->y + sb + sem_asc(cb);
                      if(sb - sem_desc(cb) < y0 - p->y)       y0 = p->y + sb - sem_desc(cb);
                  } }
                /* padding interno na VERTICAL — a moldura deixa de tocar o texto */
                y0 -= pad; y1 += pad;
                if(BX_ON && BX_Y1 > y1) y1 = BX_Y1;
                /* aresta direita = fim do texto + pad interno (nunca o texto na linha) */
                long x1 = BX_XM + BX_PAD;
                if(xm + pad > x1) x1 = xm + pad;
                if(caixa_x0 + 2 * BX_PAD > x1) x1 = caixa_x0 + 2 * BX_PAD;
                xs[0] = caixa_x0; ys[0] = y0;
                xs[1] = x1; ys[1] = y0;
                xs[2] = x1; ys[2] = y1;
                xs[3] = caixa_x0; ys[3] = y1;
                xs[4] = caixa_x0; ys[4] = y0;
                poe_poli(p, xs, ys, 5, esp_traco(fonte), "tinta");
                /* Metrónomo: a aresta inferior é o que se LEU — não se estima depois */
                BX_Y0 = y0; BX_PAD = pad; BX_M3 = m3; BX_FECHOU = 1;
                xm += pad + m3;
                BX_ON = 0;
            }
            i++; continue;
        }
        long cpm = corpo_exp_m(corpo, ex);
        if(ex == 4 || ex == 6 || ex == 7){
            if(nfr_i < 0) nfr_i = i;
            nfr_j = j; i = j; continue;               /* o numerador difere-se, vão inteiro */
        }
        if(ex == -4 || ex == -6 || ex == -7){
            if(den_i < 0) den_i = i;
            den_j = j; i = j; continue;               /* o denominador idem */
        }
        if(den_i >= 0){
            /* A PILHA DO \frac fecha aqui: os dois vãos (com os expoentes internos ±6/±7
             * DENTRO deles) pintam centrados na largura MAX, com o traço no eixo */
            long wN6 = 0, wD6 = 0;
            for(int k = nfr_i; k >= 0 && k < nfr_j; k++)
                wN6 += (long)largura(L->g[k].g, L->g[k].f) * corpo_exp_m(corpo, L->g[k].e);
            for(int k = den_i; k < den_j; k++)
                wD6 += (long)largura(L->g[k].g, L->g[k].f) * corpo_exp_m(corpo, L->g[k].e);
            long wN = wN6 / 1000, wD = wD6 / 1000, wM = wN > wD ? wN : wD;
            long dv = corpo - corpo_exp_m(corpo, 1);   /* o degrau do expoente: a régua */
            if(nfr_i >= 0)
                pinta_meia_pilha(p, L, nfr_i, nfr_j, corpo,
                                 xm + (wM - wN) / 2, p->y + 2 * dv + sem_resp(dv), dv);
            pinta_meia_pilha(p, L, den_i, den_j, corpo,
                             xm + (wM - wD) / 2, p->y - 2 * dv - sem_resp(dv), dv);
            poe_regua(p, xm, xm + wM, p->y + dv,
                      esp_traco(nfr_i >= 0 ? L->g[nfr_i].f : fonte), "tinta");
            xm += wM; nfr_i = -1; den_i = -1;
            /* (a pilha já NÃO marca teve_rotulo: o pé dela entra no fundo MEDIDO da
             * linha — a meia entrelinha antiga somava com ele, duas réguas) */
            /* e o glifo corrente segue normal, já fora da pilha */
        }
        /* O DELIMITADOR É A FRONTEIRA DA REGIÃO — ∂B_interno: 4/6/10 abrem ( [ {,
         * 5/7/11 fecham. UMA regra para os três tipos: o relógio entrega a região
         * (os estados internos, pilha e matriz incluídas), o tipo só escolhe que
         * fronteira desenhar. O tamanho é o BOOST que conserva a área da tinta
         * (sv=k, sh=1/k, |det|=1 — o incompressível), e a margem é a dobra da
         * semente, não uma constante do parêntese. */
        if(L->g[i].g >= 4 && L->g[i].g <= 13 && L->g[i].g != 8 && L->g[i].g != 9
           && ex != 8 && ex != -8){
            int gk = L->g[i].g;
            int fdel = (gk == 12 || gk == 13) ? F_SIM : fonte;   /* o ⟨⟩ mora na símbolo */
            if(gk == 4 || gk == 6 || gk == 10 || gk == 12){
                if(n_del < 8){ DEL_X[n_del] = xm; DEL_I[n_del] = i; n_del++; }
                xm += (long)largura(gk == 4 ? '(' : gk == 6 ? '[' : gk == 10 ? '{' : 0xE1,
                                    fdel) * corpo / 1000;
            } else {
                int ga = (gk == 5) ? '(' : (gk == 7) ? '[' : (gk == 13) ? 0xE1 : '{';
                int gf = (gk == 5) ? ')' : (gk == 7) ? ']' : (gk == 13) ? 0xF1 : '}';
                long x_a = xm; int i_a = i;
                if(n_del > 0){ n_del--; x_a = DEL_X[n_del]; i_a = DEL_I[n_del]; }
                long dvb = corpo - corpo_exp_m(corpo, 1);
                long y0 = -(sem_desc(corpo)), y1 = sem_asc(corpo);
                for(int kb = i_a + 1; kb < i; kb++){
                    int eb = L->g[kb].e;
                    long cb = corpo_exp_m(corpo, eb), sb = sobe_exp_m(corpo, eb);
                    if(eb == 4)  sb =  2 * dvb + sem_resp(dvb);
                    if(eb == -4) sb = -2 * dvb - sem_resp(dvb);
                    if(eb == 8 || eb == -8){
                        /* a região REAL da matriz, pela régua única: a fronteira
                         * cobre a área interna mais o ponto de semente (o m2) */
                        long tp, fd;
                        kb = matriz_regiao(L->g, L->n, kb, corpo, &tp, &fd, 0, 0, 0) - 1;
                        if(tp > y1) y1 = tp;
                        if(fd < y0) y0 = fd;
                        continue;
                    }
                    if(sb + sem_asc(cb) > y1) y1 = sb + sem_asc(cb);
                    if(sb - sem_desc(cb) < y0)      y0 = sb - sem_desc(cb);
                }
                long m2 = sem_resp(dvb);                     /* a margem: da semente */
                /* a fronteira natural também respira as SUAS margens: sem isto o
                 * controlo (conteúdo baixo) esticava sempre — e k=1 é a mutação */
                /* a caixa natural é a MESMA do varrimento (17/20 acima, 1/4 abaixo):
                 * duas réguas aqui davam k=1,05 até no vazio */
                long H = (y1 - y0) + 2 * m2, Hn = (sem_asc(corpo) + sem_desc(corpo)) + 2 * m2;
                long k_num = H > Hn ? H : Hn, k_den = Hn;
                long centro = p->y + (y0 + y1) / 2;
                long yd = centro - sem_eixo(corpo) * k_num / k_den;
                poe_glifo_boost(p, ga, fdel, corpo, x_a, yd, k_num, k_den);
                poe_glifo_boost(p, gf, fdel, corpo, xm, yd, k_num, k_den);
                xm += (long)largura(gf, fdel) * corpo / 1000;
            }
            i++; continue;
        }
        if(ex == 8 || ex == -8){
            /* A MATRIZ É UMA TABELA EM MINIATURA: o esquema das tabelas dentro da
             * fórmula — cada coluna na largura MÁXIMA das suas células, cada célula
             * centrada na coluna, as filas empilhadas à volta do eixo (o mesmo centro
             * da barra do \frac). small (8) na dobra, normal (-8) no corpo cheio. */
            int jj = j;
            while(jj < L->n){
                int e2 = L->g[jj].e;
                if(e2 == ex){ jj++; continue; }
                if(e2 == 2 || e2 == -2 || e2 >= 16){     /* expoente dentro da célula */
                    int k2 = jj;
                    while(k2 < L->n && (L->g[k2].e == 2 || L->g[k2].e == -2
                                     || L->g[k2].e >= 16)) k2++;
                    if(k2 < L->n && L->g[k2].e == ex){ jj = k2; continue; }
                }
                break;
            }
            long dv = corpo - corpo_exp_m(corpo, 1);
            long colw6[8]; int nc2 = 0;
            { int c3 = 0; long cel6 = 0;
              for(int t = 0; t < 8; t++) colw6[t] = 0;
              for(int k = i; k < jj; k++){
                  int gk = L->g[k].g;
                  if(gk == 2 || gk == 3){
                      if(cel6 > colw6[c3]) colw6[c3] = cel6;
                      cel6 = 0;
                      if(gk == 2){ if(c3 < 7) c3++; if(c3 + 1 > nc2) nc2 = c3 + 1; }
                      else c3 = 0;
                  } else cel6 += (long)largura(gk, L->g[k].f) * corpo_exp_m(corpo, L->g[k].e);
              }
              if(cel6 > colw6[c3]) colw6[c3] = cel6;
              if(c3 + 1 > nc2) nc2 = c3 + 1; }
            /* AS FILAS EMPILHAM POR ALTURAS MEDIDAS — e a medida é a RÉGUA ÚNICA
             * (matriz_regiao): a mesma que os vãos leem. O cone (a região) mede o
             * que a espiral (as células) compôs, e o par não se move. */
            int nfilas = 1;
            long ftopo[16], ffundo[16], base_r[16];
            long topo_r = 0, fundo_r = 0;
            matriz_regiao(L->g, L->n, i, corpo, &topo_r, &fundo_r, ftopo, ffundo, &nfilas);
            { long resp_f = sem_resp(dv);
              long yy = p->y + topo_r;
              for(int t = 0; t < nfilas && t < 16; t++){
                  base_r[t] = yy - ftopo[t];
                  yy = base_r[t] + ffundo[t] - resp_f;
              } }
            long w_tot = (nc2 - 1) * dv;                 /* o respiro entre colunas */
            for(int t = 0; t < nc2; t++) w_tot += colw6[t] / 1000;
            /* OS DELIMITADORES COMPÕEM-SE DA ASSINATURA À MEDIDA DA REGIÃO — não há
             * glifo de corpo fixo: o relógio avalia a curva no grau que a região pede,
             * e é a MESMA lei do \bigl e do integral (a fronteira do vão, o boost que
             * conserva a tinta). A região é a que as filas mediram: do topo da primeira
             * ao fundo da última, com o respiro da semente. */
            long mat_kn = 1, mat_kd = 1, mat_yd = p->y;
            { long m2 = sem_resp(dv);
              long H = (topo_r - fundo_r) + 2 * m2;
              long Hn = (sem_asc(corpo) + sem_desc(corpo)) + 2 * m2;
              mat_kn = H > Hn ? H : Hn; mat_kd = Hn;
              mat_yd = p->y + (topo_r + fundo_r) / 2 - sem_eixo(corpo) * mat_kn / mat_kd; }
            { int ga = (L->g[i].g == 4) ? '(' : (L->g[i].g == 6) ? '[' : 0;
              if(ga){ poe_glifo_boost(p, ga, fonte, corpo, xm, mat_yd, mat_kn, mat_kd);
                      xm += (long)largura(ga, fonte) * corpo / 1000; } }
            /* desenha por célula: sub-corridas de (fonte, marca), centradas na coluna */
            { int k = i, r3 = 0, c3 = 0; long x0c = xm;
              long cel6 = 0; int cel_i = k; long x_base = xm;
              while(k <= jj){
                  int fim_cel = (k == jj) || L->g[k].g == 2 || L->g[k].g == 3;
                  if(!fim_cel){ k++; continue; }
                  /* pinta a célula [cel_i, k): centro na coluna, fila r3 */
                  cel6 = 0;
                  for(int t = cel_i; t < k; t++)
                      cel6 += (long)largura(L->g[t].g, L->g[t].f) * corpo_exp_m(corpo, L->g[t].e);
                  { long xg = x0c + (colw6[c3] - cel6) / 2000;
                    long yr = base_r[r3 < 16 ? r3 : 15];
                    int t = cel_i;
                    while(t < k){
                        int t2 = t, f2 = L->g[t].f, e2 = L->g[t].e;
                        while(t2 < k && L->g[t2].f == f2 && L->g[t2].e == e2) t2++;
                        long cp2 = corpo_exp_m(corpo, e2), wk6 = 0;
                        for(int u = t; u < t2; u++) wk6 += (long)largura(L->g[u].g, f2) * cp2;
                        poe_pedaco(&p->sf, L->g, t, t2, f2, cp2, xg,
                                   yr + sobe_exp_m(corpo, e2 == ex ? 0 : e2), 0);
                        xg += wk6 / 1000; t = t2;
                    } }
                  if(k == jj) break;
                  if(L->g[k].g == 2){ x0c += colw6[c3] / 1000 + dv; if(c3 < 7) c3++; }
                  else { c3 = 0; r3++; x0c = x_base; }
                  k++; cel_i = k;
              } }
            xm += w_tot;
            { int gf = (L->g[jj-1].g == 5) ? ')' : (L->g[jj-1].g == 7) ? ']' : 0;
              if(gf){ poe_glifo_boost(p, gf, fonte, corpo, xm, mat_yd, mat_kn, mat_kd);
                      xm += (long)largura(gf, fonte) * corpo / 1000; } }
            i = jj; continue;
        }
        if(ex == 5){
            /* A RAIZ DESENHA-SE PELOS CORPOS: uma polilinha — o gancho, a diagonal e o
             * vinculum num só traço emendado —, dimensionada pela ASSINATURA: o corpo, a
             * sua dobra (dv) e o CapHeight 700/1000 que o descritor da fonte declara. */
            /* E O RADICANDO É UM SÓ, mesmo com expoente lá dentro: o \sqrt{x^2+4}
             * marca x,+,4 com 5 e o 2 com ±2, e um radical por corrida partia o
             * vínculo em dois — «√x² √+4». O vão vai até ao último glifo do
             * radicando: um ±2 só pertence se ainda houver um 5 à frente. */
            int jj = j;
            while(jj < L->n){
                int e2 = L->g[jj].e;
                if(e2 == 5){ jj++; continue; }
                if(e2 == 2 || e2 == -2 || e2 >= 16){
                    int k2 = jj;
                    while(k2 < L->n && (L->g[k2].e == 2 || L->g[k2].e == -2
                                     || L->g[k2].e >= 16)) k2++;
                    if(k2 < L->n && L->g[k2].e == 5){ jj = k2; continue; }
                }
                break;
            }
            long w6 = 0; int tem_exp = 0;
            for(int k = i; k < jj; k++){
                w6 += (long)largura(L->g[k].g, L->g[k].f) * corpo_exp_m(corpo, L->g[k].e);
                if(L->g[k].e != 5) tem_exp = 1;
            }
            long dv = corpo - corpo_exp_m(corpo, 1);
            long g2 = sem_resp(dv);                              /* o respiro, da mesma dobra */
            /* o vinculum sobre o CapHeight — e um expoente interior levanta-o meia dobra */
            long vy = p->y + corpo * 7 / 10 + g2 + (tem_exp ? dv / 2 : 0);
            long xs[4], ys[4];
            xs[0] = xm;          ys[0] = p->y + dv;        /* o ombro do gancho */
            xs[1] = xm + dv / 2; ys[1] = p->y - sem_resp(dv);    /* o vértice, abaixo da base */
            xs[2] = xm + dv;     ys[2] = vy;               /* a diagonal até ao topo */
            xs[3] = xm + dv + g2 + w6 / 1000 + g2; ys[3] = vy;   /* o vinculum */
            poe_poli(p, xs, ys, 4, esp_traco(fonte), "tinta");
            /* pinta por sub-corridas de (fonte, marca): cada uma no seu corpo e altura */
            { long xg = xm + dv + g2; int k = i;
              while(k < jj){
                  int k2 = k, f2 = L->g[k].f, e2 = L->g[k].e;
                  while(k2 < jj && L->g[k2].f == f2 && L->g[k2].e == e2) k2++;
                  long cp2 = corpo_exp_m(corpo, e2), wk6 = 0;
                  for(int t = k; t < k2; t++) wk6 += (long)largura(L->g[t].g, f2) * cp2;
                  poe_pedaco(&p->sf, L->g, k, k2, f2, cp2, xg,
                             p->y + sobe_exp_m(corpo, e2), extra);
                  xg += wk6 / 1000; k = k2;
              } }
            xm += dv + g2 + w6 / 1000 + g2;
            i = jj; continue;
        }
        if(L->g[i].g == 14){             /* o começo do vão do underbrace */
            seg_x0 = xm; seg_i14 = i;
            i++; continue;
        }
        if(ex == -5){                    /* o espaço não-quebrável compõe como espaço */
            long av5 = (long)largura(' ', fonte) * corpo / 1000;
            xm += av5;
            i = j; continue;
        }
        if(ex == -3){
            /* O RÓTULO DO UNDERBRACE: centra-se POR BAIXO do segmento que o antecede,
             * meia entrelinha abaixo, e NÃO avança a linha — a banda de baixo é dele */
            /* e a CHAVETA desenha-se PELOS CORPOS, como a raiz: sete pontos — os
             * ombros, o vinco central para baixo — na dobra da semente, com o
             * traço no peso do contexto */
            long fundo_seg = -(sem_desc(corpo));
            { long dvf = corpo - corpo_exp_m(corpo, 1);
              for(int kb = seg_i14; kb < i; kb++){
                  int eb = L->g[kb].e; int gk2 = L->g[kb].g;
                  if(gk2 >= 4 && gk2 <= 14) continue;
                  long cb = corpo_exp_m(corpo, eb), sb = sobe_exp_m(corpo, eb);
                  if(eb == 4)  sb =  2 * dvf + sem_resp(dvf);
                  if(eb == -4) sb = -2 * dvf - sem_resp(dvf);
                  if(sb - sem_desc(cb) < fundo_seg) fundo_seg = sb - sem_desc(cb);
              } }
            { long dv3 = corpo - corpo_exp_m(corpo, 1);
              long h3 = sem_resp(dv3), yb = p->y + fundo_seg - h3;
              long x0b = seg_x0, x1b = xm, xmb = (x0b + x1b) / 2;
              if(x1b - x0b > 4 * h3){
                  long xs[7], ys[7];
                  xs[0] = x0b;       ys[0] = yb + h3;
                  xs[1] = x0b + h3;  ys[1] = yb;
                  xs[2] = xmb - h3;  ys[2] = yb;
                  xs[3] = xmb;       ys[3] = yb - h3;
                  xs[4] = xmb + h3;  ys[4] = yb;
                  xs[5] = x1b - h3;  ys[5] = yb;
                  xs[6] = x1b;       ys[6] = yb + h3;
                  poe_poli(p, xs, ys, 7, esp_traco(fonte), "tinta");
              } }
            long wl6 = 0;
            for(int k = i; k < j; k++) wl6 += (long)largura(L->g[k].g, fonte) * cpm;
            long xl = seg_x0 + (xm - seg_x0 - wl6 / 1000) / 2;
            if(xl < seg_x0) xl = seg_x0;
            /* abaixo da CHAVETA: o vinco desce h3 e o rótulo desce a sua ascendente
             * — riscava o texto quando ficava a meia entrelinha fixa */
            { long dv3 = corpo - corpo_exp_m(corpo, 1);
              long yr2 = p->y + fundo_seg - dv3 - sem_asc(cpm);
              poe_pedaco(&p->sf, L->g, i, j, fonte, cpm, xl, yr2, 0); }
            /* o rótulo mais largo que a expressão EMPURRA a linha: o bloco vale o máximo
             * dos dois, e é o excesso que o mede também conta */
            { long xr = xl + wl6 / 1000; if(xr > xm) xm = xr; }
            seg_x0 = xm; teve_rotulo = 1;
            i = j; continue;
        }
        if(ex >= 16 && (i == 0 || (L->g[i-1].e < 16 && L->g[i-1].e == 0)
                        || (L->g[i-1].e >= 16 && ESP_NV[ex - 16] > ESP_NV[L->g[i-1].e - 16]))){
            /* a semente do expoente EMPURRA para a direita: o terço do passo do
             * giro (o mesmo respiro da subida), na razão da espiral */
            long e_ant = (i > 0 && L->g[i-1].e >= 16) ? L->g[i-1].e : 0;
            long kx = sem_resp(corpo_exp_m(corpo, (int)e_ant) - corpo_exp_m(corpo, ex));
            if(kx > 0) xm += kx;
        }
        /* O INTEGRAL É FRONTEIRA DO QUE MEDE — como o delimitador: o ∫ não tem
         * tamanho próprio, estica pelo vão da LINHA que governa, com o mesmo boost
         * que conserva a tinta (sv=k, sh=1). Numa linha plana a medida dá k=1 e o
         * corpo do texto fica; a pilha do \frac em destaque dá o k>1 do gabarito.
         * A régua é a medida, não um degrau à parte. */
        if(fonte == F_SIM && ex == 0 && L->g[i].g == 0xF2){
            int puro = 1;
            for(int t = i; t < j; t++) if(L->g[t].g != 0xF2) puro = 0;
            if(puro){
                long dvb = corpo - corpo_exp_m(corpo, 1);
                long y0 = -(sem_desc(corpo)), y1 = sem_asc(corpo);
                for(int kb = 0; kb < L->n; kb++){
                    int eb = L->g[kb].e;
                    long cb = corpo_exp_m(corpo, eb), sb = sobe_exp_m(corpo, eb);
                    if(eb == 4)  sb =  2 * dvb + sem_resp(dvb);
                    if(eb == -4) sb = -2 * dvb - sem_resp(dvb);
                    if(eb == 8 || eb == -8){
                        /* a mesma régua única da região da matriz */
                        long tp, fd;
                        kb = matriz_regiao(L->g, L->n, kb, corpo, &tp, &fd, 0, 0, 0) - 1;
                        if(tp > y1) y1 = tp;
                        if(fd < y0) y0 = fd;
                        continue;
                    }
                    if(sb + sem_asc(cb) > y1) y1 = sb + sem_asc(cb);
                    if(sb - sem_desc(cb) < y0) y0 = sb - sem_desc(cb);
                }
                long m2 = sem_resp(dvb);
                long H = (y1 - y0) + 2 * m2, Hn = (sem_asc(corpo) + sem_desc(corpo)) + 2 * m2;
                long k_num = H > Hn ? H : Hn, k_den = Hn;
                long centro = p->y + (y0 + y1) / 2;
                long yd = centro - sem_eixo(corpo) * k_num / k_den;
                long xg = xm;
                for(int t = i; t < j; t++){
                    poe_glifo_boost(p, 0xF2, F_SIM, corpo, xg, yd, k_num, k_den);
                    xg += (long)largura(0xF2, F_SIM) * corpo / 1000;
                }
                xm = xg;
                /* E A DILATAÇÃO DA FRONTEIRA PASSA AO LADO DUAL COMO TRANSLAÇÃO:
                 * são os duais — escala de um lado, soma do outro (Mellin: dilatar
                 * vira transladar; o log é o isomorfismo e o relógio fá-lo por
                 * involução, no inteiro). O limite compõe pelo caminho comum da
                 * espiral, no SEU degrau; só a subida do giro recebe o k, por um
                 * produto cruzado. Em linha plana k=1 e nada muda. */
                front_kn = k_num; front_kd = k_den;
                front_on = 1; front_sg = 0; front_wmax = 0;
                i = j; continue;
            }
        }
        /* a translação do giro recebe a dilatação da fronteira (os duais: escala↔soma);
         * a escala do glifo é a do seu degrau. O nível zero devolve a razão a um. */
        if(!ex){
            if(front_sg && xm - front_x0 < front_wmax) xm = front_x0 + front_wmax;
            front_on = 0; front_sg = 0; front_kn = 1; front_kd = 1;
        } else if(front_on){
            /* o par dos limites partilha o eixo: o grupo do outro sinal volta ao x0.
             * E o ESPAÇAMENTO não se põe: a semente declarou-o uma vez e ele
             * propaga-se sozinho pelas involuções de escala — sem_resp do passo do
             * degrau do próprio grupo, com o sinal do grupo (o sub abaixo e à
             * esquerda, o sup acima). Nenhum termo é deste sítio: é a semente lida
             * neste degrau. */
            int s = sobe_exp_m(corpo, ex) < 0 ? -1 : 1;
            int novo = 0;
            if(!front_sg){ front_sg = s; front_x0 = xm; front_wmax = 0; novo = 1; }
            else if(s != front_sg){
                long w1 = xm - front_x0;
                if(w1 > front_wmax) front_wmax = w1;
                xm = front_x0; front_sg = s; novo = 1;
            }
            if(novo && s < 0) xm -= sem_resp(corpo - cpm);
        }
        poe_pedaco(&p->sf, L->g, i, j, fonte, cpm, xm,
                   p->y + sobe_exp_m(corpo, ex) * front_kn / front_kd
                        + (front_on && ex ? front_sg * sem_resp(corpo - cpm) : 0), extra);
        long w6 = 0, wesp = 0;
        for(int k = i; k < j; k++){
            w6 += (long)largura(L->g[k].g, fonte) * cpm;
            if(L->g[k].g == ' ') wesp += extra;   /* o espaco e' letra em qualquer fonte */
        }
        xm += w6 / 1000 + wesp;    /* o produto exacto, UMA divisão por pedaço; o Tw soma */
        if(front_on && front_sg && xm - front_x0 > front_wmax) front_wmax = xm - front_x0;
        i = j;
    }
    if(den_i >= 0){                        /* a pilha que fecha a linha pinta aqui */
        long wN6 = 0, wD6 = 0;
        for(int k = nfr_i; k >= 0 && k < nfr_j; k++)
            wN6 += (long)largura(L->g[k].g, L->g[k].f) * corpo_exp_m(corpo, L->g[k].e);
        for(int k = den_i; k < den_j; k++)
            wD6 += (long)largura(L->g[k].g, L->g[k].f) * corpo_exp_m(corpo, L->g[k].e);
        long wN = wN6 / 1000, wD = wD6 / 1000, wM = wN > wD ? wN : wD;
        long dv = corpo - corpo_exp_m(corpo, 1);
        if(nfr_i >= 0)
            pinta_meia_pilha(p, L, nfr_i, nfr_j, corpo,
                             xm + (wM - wN) / 2, p->y + 2 * dv + sem_resp(dv), dv);
        pinta_meia_pilha(p, L, den_i, den_j, corpo,
                         xm + (wM - wD) / 2, p->y - 2 * dv - sem_resp(dv), dv);
        poe_regua(p, xm, xm + wM, p->y + dv,
                  esp_traco(nfr_i >= 0 ? L->g[nfr_i].f : F_REG), "tinta");
        /* (sem teve_rotulo: o pé da pilha entra no fundo MEDIDO — uma régua só) */
    }
    /* a banda do rótulo gasta meia entrelinha: a linha seguinte não cai em cima dela */
    if(teve_rotulo) p->y -= escala_entre(D_TEXTO) / 2;
    /* e o FUNDO da linha: o que desceu além da caixa natural (o pé da pilha, o
     * índice fundo) empurra a linha seguinte — a outra metade da translação rodada */
    p->y -= lin_fundo;
    /* largura máxima das linhas do `\boxed` aberto (para a moldura única) */
    if(BX_ON && xm > BX_XM) BX_XM = xm;
    /* Metrónomo (thm:metronomo): mede pela metade refletida. O Maestro projectou
     * a moldura até BX_Y0 (λ⁺=pad); a volta dual põe o lápis em BX_Y0 − pad
     * (λ⁻=−λ⁺). Resíduo 0 = aresta↔cursor. BX_ATESTOU guarda a aresta para o
     * topo da próxima caixa. */
    if(BX_FECHOU){
        long alvo = BX_Y0 - BX_PAD;          /* λ⁺ + λ⁻ = 0 */
        if(p->y > alvo) p->y = alvo;
        BX_FECHOU = 0; BX_ATESTOU = 1;
    }
}

static void pdf_fecha(Pdf *p){
    pagina_fecha(p);
    /* o objeto Pages sai no FIM — só agora se sabe quantas páginas há. A xref dá o offset, e a
     * ordem no ficheiro é livre: um objeto pode estar em qualquer parte. */
    p->off[2] = s_pos(&p->sf);
    snprintf(S_FMT_BUF, 2048, "2 0 obj<</Type/Pages/Count %d/Kids[", p->npag); s_flush(&p->sf);
    for(int i = 0; i < p->npag; i++){ snprintf(S_FMT_BUF, 2048, "%d 0 R ", p->pag[i]); s_flush(&p->sf); }
    snprintf(S_FMT_BUF, 2048, "]>>endobj\n"); s_flush(&p->sf);
    p->off[1] = s_pos(&p->sf);
    snprintf(S_FMT_BUF, 2048, "1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj\n"); s_flush(&p->sf);
    /* ─── AS FONTES: EMBUTIDAS, e não declaradas ───────────────────────────────────────
     *
     * Até aqui escrevia-se `/BaseFont/Helvetica` e o leitor desenhava com o que tivesse. A
     * spline lia-se da TTF para MEDIR a largura e não se emitia — logo a fonte que aparecia
     * não era a do design, e nenhuma escala a corrigia. Era o texto tosco.
     *
     * E A SAÍDA NÃO É EMITIR CADA GLIFO COMO CAMINHO, embora o mecanismo esteja provado no
     * backend_ttf.c. Se cada letra virasse um caminho, o PDF deixava de TER texto: só desenho.
     * A volta partia-se — o `pdftotext` não acharia uma palavra, e três medidores que contam
     * palavras no PDF caíam com razão. Desenhar o texto é APAGAR o texto, e apagar não se
     * desfaz.
     *
     * Embute-se a TTF. O PDF leva o ficheiro dentro (/FontFile2), usa os seus glifos, e o
     * texto continua a ser texto: os dois sentidos ficam. É a mesma escolha de sempre — a que
     * guarda o dual. */
    long fonte_emb = 0;
    {
        /* AS FONTES JA NAO SE EMBUTEM: nao ha um Tj no papel — o desenho e' as marcas
         * do relogio, e o texto viaja no FonteTeX. Os objectos 3..18 ficam como cascas
         * minimas so' para o xref nao se mover. */
        fonte_emb = N_FPDF ? EMBP[0] : 0;
    }
    for(int i = 0; i < N_FIXA; i++){
        p->off[3+i] = s_pos(&p->sf);
        snprintf(S_FMT_BUF, 2048, "%d 0 obj<<>>endobj\n", 3 + i); s_flush(&p->sf);
    }
    (void)fonte_emb;
    /* AS ASSINATURAS USADAS, escritas UMA vez cada: o contorno em unidades da fonte
     * (inteiros, exactos), um Form XObject por par carta·glifo — e o objecto partilhado
     * dos Resources que as nomeia para todas as páginas. Cada USO na página foi só
     * `cm /G Do`: a posição e a escala, o corpo como campo. */
    {
        for(int ix = 0; ix < N_XGC; ix++)
            for(int gb2 = 33; gb2 < 256; gb2++){
                if(!XG_USADO[ix * 256 + gb2]) continue;
                const Ttf *ca = XG_CARTA[ix];
                int uni = (ca == &CARTAS[F_SIM] || ca == &CARTAS[F_MAT]
                        || ca == &CARTAS[F_MTB] || ca == &CARTAS[F_SMB]) ? gb2
                         : (gb2 < 0x80 ? gb2 : winansi_para_unicode(gb2));
                int gi = ttf_glifo(ca, uni);
                int okc = gi ? (ca->cff ? cff_contorno(ca, gi, &XG_CT)
                                        : ttf_contorno(ca, gi, &XG_CT)) : 0;
                int of2 = p->nobj + 1, ol2 = p->nobj + 2; p->nobj = ol2;
                p->off[of2] = s_pos(&p->sf);
                snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Type/XObject/Subtype/Form"
                              "/BBox[-3000 -2000 6000 3000]/Length %d 0 R>>stream\n",
                      of2, ol2); s_flush(&p->sf);
                long ini2 = s_pos(&p->sf);
                if(okc && XG_CT.n) escreve_caminho(&p->sf, &XG_CT, ca->cff ? 1 : 0);
                long fim2 = s_pos(&p->sf);
                fecha_stream_ind(p, ol2, fim2 - ini2);
                XG_ID[ix * 256 + gb2] = of2;
            }
        int ro = 3 + N_FIXA;
        p->off[ro] = s_pos(&p->sf);
        snprintf(S_FMT_BUF, 2048, "%d 0 obj<</XObject<<", ro); s_flush(&p->sf);
        for(int ix = 0; ix < N_XGC; ix++)
            for(int gb2 = 33; gb2 < 256; gb2++)
                if(XG_USADO[ix * 256 + gb2] && XG_ID[ix * 256 + gb2]){
                    snprintf(S_FMT_BUF, 2048, "/G%d_%d %d 0 R", ix, gb2, XG_ID[ix * 256 + gb2]); s_flush(&p->sf);
                }
        snprintf(S_FMT_BUF, 2048, ">>>>endobj\n"); s_flush(&p->sf);
    }
    /* O .TEX ORIGINAL, INVISÍVEL: um objecto que a página não referencia. O leitor de PDF
     * ignora-o — não está em /Contents nem em árvore nenhuma —, mas está lá, e a volta lê-o
     * pelo marcador /Type/FonteTeX. É a metade que a estrela guarda para não apagar. */
    /* O SELO DE CAELUM — a lei 8 assina o ESQUELETO (N=2^8). TorreDim/TorreN sobem
     * com a torre; o Sel[256] mantém-se — é o ciclo base; a torre continua no Gentil. */
    { int torre_alc = 0;
      { int t = 0; while(t < N_ESP){ if(ESP_NV[t] > torre_alc) torre_alc = ESP_NV[t]; t++; } }
      int torre_dim = 2; { int k = 0; while(k < torre_alc){ torre_dim *= 2; k++; } }
      int torre_lado = (torre_alc >= 3) ? 1 : 0;
      int torre_iface = 6, ik = torre_alc / 3;
      while(ik > 0){ torre_iface *= 2; ik--; }
      if(INTERFACE_DECL >= 6) torre_iface = INTERFACE_DECL;
      { extern int INTERFACE_N; extern int LADO_N;
        if(torre_iface >= 6) INTERFACE_N = torre_iface;
        LADO_N = torre_lado;
      }
      long A[256], S[256];
      long At[TORRE_NTT_MAX], St[TORRE_NTT_MAX];
      int torre_ntt = 0;
      if(torre_dim >= 16){
          torre_ntt = torre_dim * 32;
          if(torre_ntt > TORRE_NTT_MAX) torre_ntt = TORRE_NTT_MAX;
          for(int t = 0; t < torre_ntt; t++) At[t] = 0;
      }
      for(int t = 0; t < 256; t++) A[t] = 0;
      { long q = 0; const unsigned char *z = p->sf.buf; long len = s_pos(&p->sf);
        while(q + 26 < len){
            if(memcmp(z + q, "/Type/XObject/Subtype/Form", 26)){ q++; continue; }
            long a2 = q;
            while(a2 + 7 < len && memcmp(z + a2, "stream\n", 7)) a2++;
            a2 += 7;
            long b2 = a2, k2 = 0;
            while(b2 + 9 < len && memcmp(z + b2, "endstream", 9)){
                A[k2 & 255] = (A[k2 & 255] + z[b2]) % 65537;
                if(torre_ntt > 0) At[k2 & (torre_ntt - 1)] = (At[k2 & (torre_ntt - 1)] + z[b2]) % 65537;
                b2++; k2++;
            }
            q = b2 + 9;
        } }
      { long raiz = 1, b3 = 3, e3 = 256;          /* 3^256 mod 65537: a raiz de ordem 256 */
        while(e3 > 0){
            if(e3 & 1) raiz = raiz * b3 % 65537;
            b3 = b3 * b3 % 65537; e3 >>= 1;
        }
        for(int j2 = 0; j2 < 256; j2++){
            long acc = 0, w2 = 1, passo2 = 1;
            { long e4 = j2, b4 = raiz;
              while(e4 > 0){
                  if(e4 & 1) passo2 = passo2 * b4 % 65537;
                  b4 = b4 * b4 % 65537; e4 >>= 1;
              } }
            for(int t = 0; t < 256; t++){
                acc = (acc + A[t] * w2) % 65537;
                w2 = w2 * passo2 % 65537;
            }
            S[j2] = acc;
        } }
      { int obj = p->nobj + 1; p->nobj = obj;
        p->off[obj] = s_pos(&p->sf);
        snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Type/AssinaturaOito/N 256/P 65537"
                    "/TorreDim %d/TorreN %d/Sel[", obj, torre_dim, torre_dim); s_flush(&p->sf);
        for(int t = 0; t < 256; t++){ snprintf(S_FMT_BUF, 2048, "%ld ", S[t]); s_flush(&p->sf); }
        snprintf(S_FMT_BUF, 2048, "]>>endobj\n"); s_flush(&p->sf); }
      /* Gentil (dim≥16): selo complementar — NTT min(TORRE_NTT_MAX, TorreDim·32).
       * O Sel[256] base (Lei 8) mantém-se; a torre sobe por indução T+T* (thm:tecidos). */
      if(torre_ntt > 0){
        long raiz2 = 1, b5 = 3, e5 = 65536 / torre_ntt;
        while(e5 > 0){
            if(e5 & 1) raiz2 = raiz2 * b5 % 65537;
            b5 = b5 * b5 % 65537; e5 >>= 1;
        }
        for(int j3 = 0; j3 < torre_ntt; j3++){
            long acc = 0, w3 = 1, passo3 = 1;
            { long e6 = j3, b6 = raiz2;
              while(e6 > 0){
                  if(e6 & 1) passo3 = passo3 * b6 % 65537;
                  b6 = b6 * b6 % 65537; e6 >>= 1;
              } }
            for(int t = 0; t < torre_ntt; t++){
                acc = (acc + At[t] * w3) % 65537;
                w3 = w3 * passo3 % 65537;
            }
            St[j3] = acc;
        }
        { int obj = p->nobj + 1; p->nobj = obj;
          p->off[obj] = s_pos(&p->sf);
          snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Type/AssinaturaTorre/N %d/P 65537"
                      "/TorreDim %d/Sel[", obj, torre_ntt, torre_dim); s_flush(&p->sf);
          for(int t = 0; t < torre_ntt; t++){ snprintf(S_FMT_BUF, 2048, "%ld ", St[t]); s_flush(&p->sf); }
          snprintf(S_FMT_BUF, 2048, "]>>endobj\n"); s_flush(&p->sf); } }
    /* A SEMENTE VIAJA COM O DOCUMENTO, sempre — mesmo sem o .tex embutido: a
     * configuração da estrela é estado do documento, não constante do motor.
     * Alcance = max nível da espiral (subida do inversor); Dim = 2^(alcance+1)
     * na torre da estrela (2,4,8,16,…) — Hurwitz marca a dobra discreta, a torre
     * segue; π_n sai desta Dim (luz_periodo / Lyapunov dualizado).
     * Lado 0 = Hurwitz (contar); 1 = Gentil/Lebesgue (integrar). Interface = 6·2^(⌊alcance/3⌋).
     * Norma 1 = Gentil (‖xy‖=‖x‖‖y‖, nne.c) activo na espiral. */
    { int alcance = torre_alc, dim = torre_dim, lado = torre_lado, iface = torre_iface;
      int regua_C = 12 + 6 * (alcance / 3);        /* ordem do dual: C sobe a cada 3 andares */
      int regua_L = 30;                            /* mantissa completa (S=2^30); a ordem é C */
      int regua = regua_L * 100 + regua_C;         /* /Regua 3012|3018|3024 — cabe na fita */
      long S = 1073741824L;
      int lg = 0, d = dim;
      while(d > 1){ lg++; d >>= 1; }
      int iters = (dim > 1) ? (lg + dim - 2) : 1;   /* log2(q_dim)−1, sem overflow */
      if(iters > regua_C) iters = regua_C;            /* régua deste andar, não cap global */
      long piN = 0;
      if(lado){
          /* Gentil/Lebesgue: meia-corda — cap cresce a cada 3 andares (corpo_topologico) */
          int64_t sq = S;
          for(int s = 0; s < iters; s++){
              int64_t sq2 = (sq * sq) / S, inner = S - sq2;
              if(inner < 0) inner = 0;
              { uint64_t ux = (uint64_t)inner * (uint64_t)S;
                uint64_t g = ux, h = (g + 1) / 2;
                while(h < g){ g = h; h = (g + ux / g) / 2; }
                inner = (int64_t)g; }
              { uint64_t ux = (uint64_t)(2 * S + 2 * inner) * (uint64_t)S;
                uint64_t g = ux, h = (g + 1) / 2;
                while(h < g){ g = h; h = (g + ux / g) / 2; }
                if(g <= 0) g = 1;
                sq = (sq * S) / (int64_t)g; }
          }
          { uint64_t num = (uint64_t)sq * 1000000000u;
            int e = iters + 1;
            while(e > 0){ num <<= 1; e--; }
            piN = (long)(num / (uint64_t)S); }
      } else {
          long q = (dim > 0 && dim < 31) ? ((long)dim << (dim - 1)) : (1L << 30);
          int64_t c = -S;
          for(int s = 0; s < iters; s++){
              int64_t t = (S + c) / 2;
              { uint64_t ux = (uint64_t)t * (uint64_t)S;
                uint64_t g = ux, h = (g + 1) / 2;
                while(h < g){ g = h; h = (g + ux / g) / 2; }
                c = (int64_t)g; }
          }
          { int64_t t = (S - c) / 2;
            uint64_t g, ux = (uint64_t)t * (uint64_t)S;
            g = ux; { uint64_t h = (g + 1) / 2;
            while(h < g){ g = h; h = (g + ux / g) / 2; } }
            { uint64_t num = g * 1000000000u;
              int e = iters + 1;
              while(e > 0){ num <<= 1; e--; }
              piN = (long)(num / (uint64_t)S); } }
      }
      int obj = p->nobj + 1; p->nobj = obj;
      p->off[obj] = s_pos(&p->sf);
      snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Type/SementeEstrela"
                    "/Resp %ld/AscN %ld/AscD %ld/Desc %ld/Traco %d"
                    "/RazaoN %ld/RazaoD %ld"
                    "/Alcance %d/Dim %d/Induc %d/Lado %d/Interface %d/Norma %d"
                    "/Regua %d/PiN %ld>>endobj\n",
            obj, SEM_V[0], SEM_V[1], SEM_V[2], SEM_V[3], SEM_TRACO,
            N_ESCALA > 3 ? escala_de_degrau(1, EIXO_ESCALA) : 0,
            N_ESCALA > 3 ? escala_de_degrau(3, EIXO_ESCALA) : 0,
            alcance, dim, alcance, lado, iface, lado, regua, piN); s_flush(&p->sf); } }
    if(FONTE_TEX){
        /* o SOURCE é um corpo, e entra pela mesma porta que as fontes --- o cruzamento do viveiro:
         * carrega-se para o slot 4 (nativo fopen+fread, wasm pré-carregado) e transmite-se ao PDF. */
        long len = g_carrega(FONTE_TEX, 4, 1 << 22);
        if(len > 0){
            unsigned char *src = (unsigned char*)disco_buf(4, 1 << 22);
            int obj = p->nobj + 1; p->nobj = obj;
            p->off[obj] = s_pos(&p->sf);
            snprintf(S_FMT_BUF, 2048, "%d 0 obj<</Type/FonteTeX/Length %ld>>stream\n", obj, len); s_flush(&p->sf);
            s_bytes(&p->sf, src, len);                  /* o corpo inteiro, de uma vez */
            snprintf(S_FMT_BUF, 2048, "\nendstream\nendobj\n"); s_flush(&p->sf);
        }
    }
    long xref = s_pos(&p->sf);
    snprintf(S_FMT_BUF, 2048, "xref\n0 %d\n0000000000 65535 f \n", p->nobj + 1); s_flush(&p->sf);
    for(int i = 1; i <= p->nobj; i++){ snprintf(S_FMT_BUF, 2048, "%010ld 00000 n \n", p->off[i]); s_flush(&p->sf); }
    snprintf(S_FMT_BUF, 2048, "trailer<</Size %d/Root 1 0 R>>\nstartxref\n%ld\n%%%%EOF\n", p->nobj + 1, xref); s_flush(&p->sf);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * A DESCIDA sobre o .tex — e é a única passagem pelo texto
 * ───────────────────────────────────────────────────────────────────────────── */

typedef struct {
    Pdf *p;
    Linha L;
    int  fonte;            /* a fonte corrente */
    int  mat;              /* dentro de $...$ */
    /* o expoente corrente: o corpo do número que o glifo carrega (±1 grau 2, ±2 grau 4).
     * `exp1` é a marca de um token só (x^2); a pilha guarda o exp de fora de cada grupo
     * `^{...}`, fechado quando o PROF cai ao valor registado. */
    int  exp;
    int  exp1;
    int  exp_volta;
    int  exp_pf[8];
    int  exp_ant[8];
    int  exp_frac[8];      /* 1: numerador de \frac (ao fechar, emite '/' e abre o de baixo) */
    int  nexp;
    /* A FONTE REPÕE-SE POR PILHA, como o exp: cada comando de variante regista
     * (PROF, fonte de fora), e o `}` que traz o PROF de volta repõe. A heurística
     * antiga espreitava o próximo char e SÓ repunha a negra — o `\code{...}` deixava
     * a monoespaçada ligada até ao fim do parágrafo: «).» saía mono. */
    int  fp_p[16];
    int  fp_f[16];
    int  nfp;
    /* A FÓRMULA NÃO HERDA A INCLINAÇÃO DO TEXTO. O corpo do teorema compõe em
     * itálica, mas a matemática tem o SEU par: a variável vai à cmmi (a regra
     * própria, adiante) e a ESTRUTURA — dígitos, parênteses, o delimitador da
     * matriz — é romana. Guarda-se aqui a fonte do texto à entrada da fórmula
     * e repõe-se à saída: uma regra na porta, não um ajuste por sítio. */
    int  fonte_txt;
    /* A MATRIZ É UMA TABELA EM MINIATURA dentro da fórmula: o mesmo esquema — células
     * por coluna, filas por \\ — só que empilhada à volta do eixo, na linha. As marcas
     * são ±8 (8 = smallmatrix, na dobra; -8 = pmatrix, corpo cheio), e os separadores
     * são glifos de CONTROLO (2 = coluna, 3 = fila), invisíveis e sem largura. */
    int  matriz;           /* 0 fora; ±8 dentro */
    int  matriz_ant;       /* o exp de fora, reposto no \end */
    int  matriz_par;       /* o delimitador de fecho: ')' , ']' ou 0 */
    int  centra_mat;       /* o CENTRA de fora do \[...\], para repor no \] */
    int  ub;               /* viu \underbrace: o proximo _{...} e o ROTULO (-3) */
    int  ub_pf;            /* a PROF do underbrace: um ^ interno nao mata o ub */
    int  disp;             /* matematica de DESTAQUE (\[ ou align): o \frac empilha */
    int  morf_h;           /* Teorema Morfológico: dilata a linha à largura (W, Lei~4) */
    int  estrela;          /* interface Estrela activa (orquestra) --- valida H */
    int  morf_v;           /* dilatação vertical [0,H] — requer estrela / Ind^8 */
    int  morf_vn;          /* quantas vozes no bloco morf_v */
    int  morf_vi;          /* índice da próxima voz */
    long morf_vy0;         /* y no topo do bloco (suporte H) */
    int  recuo;
    int  item;
    long glifos;           /* quantos glifos saíram — o solar conta o que guardou */
    /* A TABELA. Não é um caso especial: é POSIÇÃO — a mesma coisa que já se faz com a linha,
     * só que a coluna também conta. Cada célula compõe-se no seu x, e o `&` muda de coluna
     * como o `\\` muda de linha. O que faltava não era saber desenhar: era contar as colunas. */
    int  tab;              /* dentro de tabular/longtable? */
    int  tab_ncol;         /* quantas colunas o preâmbulo declara */
    int  tab_col;          /* em qual se está */
    long tab_x0;           /* onde a tabela começa — inteiro */
    long tab_y;            /* o Y do INÍCIO da fila: cada célula volta a ele */
    long tab_ymin;         /* e o Y mais baixo que alguma célula desta fila atingiu */
    int  tab_pag;          /* em que página esse mínimo foi tirado — ver abaixo */
    long tab_larg;         /* a largura da coluna corrente */
    long tab_tot;          /* a soma das colunas: a largura do bloco da tabela */
    long tab_w[16];        /* AS LARGURAS, uma por coluna — e a soma FECHA em COL, exacta.
                            * As coordenadas são a ÁREA, e duas coordenadas ordenam a tabela:
                            * a coluna e a fila. Repartir por COL/n perde o resto (1 a 3 pt) e
                            * a conservação falha logo à entrada.
                            *
                            * A repartição é ÁUREA, em Fibonacci — o áureo em inteiros — porque
                            * 1/φ + 1/φ² = 1 é exacta e a soma dos F fecha sem resto. E a última
                            * coluna leva o que sobra, como a área negra de Hilbert leva o que
                            * falta: as duas somam UM em todo ponto. */
} Est;

/* o estado do compila vive no disco (slot 10, após as cartas), não no SP */
#define EST_E  ((Est*)((char*)DES_C + TAM_DES))

/* liga uma variante REGISTANDO a de fora na pilha: o `}` que traz o PROF de
 * volta repõe-na — a mesma mecânica do exp_pf, uma régua só para todas */
static void fonte_poe(Est *e, int f){
    if(e->nfp < 16){ e->fp_p[e->nfp] = PROF; e->fp_f[e->nfp] = e->fonte; e->nfp++; }
    e->fonte = f;
}

/* as duas metades da porta do modo matemático: a inclinação do texto fica à
 * porta (F_ITA→F_REG, e a negra itálica ao peso, F_NIT→F_NEG) e devolve-se à
 * saída — o dual da regra da variável, que manda a letra à itálica da cmmi */
static void mat_entra(Est *e){
    if(e->fonte == F_ITA){ e->fonte_txt = F_ITA; e->fonte = F_REG; }
    else if(e->fonte == F_NIT){ e->fonte_txt = F_NIT; e->fonte = F_NEG; }
}
static void mat_sai(Est *e){
    if(e->fonte_txt){ e->fonte = e->fonte_txt; e->fonte_txt = 0; }
}

static void empurra(Est *e, int g, int f){
    /* o símbolo em contexto NEGRO vai à símbolo negra — SE a referência tem a
     * curva bold (o grego tem; ∈ e afins ficam no peso normal, como o pdflatex) */
    if(f == F_SIM && e->fonte == F_NEG && CARTA_SMB
       && ttf_glifo(&CARTAS[F_SMB], g)) f = F_SMB;
    if(e->L.n == 0){ e->L.deg = DEG_FORCADO; e->L.centra = CENTRA; }
    if(e->L.n < MAXLIN - 1){ e->L.g[e->L.n].g = (unsigned char)g; e->L.g[e->L.n].f = (unsigned char)f;
                             e->L.g[e->L.n].e = (signed char)e->exp; e->L.n++; }
    e->glifos = e->glifos + 1;
    /* a marca de um token só devolve o exp de fora — o dual fecha no próprio glifo */
    if(e->exp1){ e->exp = e->exp_volta; e->exp1 = 0; }
}
/* um espaço, mas só se o último glifo não é já um espaço --- evita o espaço dobrado. Estava escrito
 * três vezes por extenso. */
static void espaco_se_falta(Est *e){
    if(!(e->L.n && e->L.g[e->L.n-1].g != ' ')) return;
    if(e->ub){ int x0 = e->exp; e->exp = -5;      /* no vão do underbrace: não quebra */
               empurra(e, ' ', e->fonte); e->exp = x0; return; }
    empurra(e, ' ', e->fonte);
}

/* quebra a linha corrente onde ela deixa de caber, e desenrola. O que sobra fica para a seguinte. */

/* ─── DESCER AO NÍVEL SEGUINTE: e QUEM DECIDE É O DOCUMENTO ─────────────────────────
 *
 * O `catalogo.tex:5585` dá a regra: «não é guloso por SUBCONJUNTO --- é preencher o NÍVEL
 * COMPLETO e passar ao próximo. Isso é representação POSICIONAL, e o posicional é único. E
 * A CADEIA TEM DE DESCER ATÉ O OURO PURO: parando no último mineral, o resto fica preso
 * abaixo dele e NADA ZERA.»
 *
 * A linha obedece a isso: encher com palavras é o nível grosso, e parar aí deixa o resto
 * preso --- é o esticamento que nunca zera.
 *
 * MAS O NÍVEL ABAIXO NÃO SOU EU QUE O ESCOLHO. Escrevi primeiro uma separação silábica
 * derivada da estrutura da palavra, e ela funciona --- `levan-`/`ta`, `gran-`, `dezes-`,
 * cortes válidos --- e MEDIDO deu 62 cortes onde o `pdflatex` faz ZERO. O estilo diz
 * porquê, e diz de duas maneiras: `\sloppy` e `\emergencystretch=3em` mandam preferir
 * esticar a partir palavras, e o `\hyphenation{...}` lista NOMINALMENTE as que podem ser
 * partidas --- PON-TRY-A-GIN, es-pa-co-tem-po-ral, cris-ta-li-no.
 *
 * Descer de nível está certo; inventar o nível é que não. A cadeia desce até onde o
 * documento a deixa descer, e a lista é a régua. */
#define MAX_HIF 64
typedef struct { char pal[48]; char cortes[16]; int nc; } Hif;
#define HIF     ((Hif*)disco_buf(8, (long)(MAX_HIF * sizeof(Hif))))
static int N_HIF = -1;


/* os cortes que o documento AUTORIZA para esta palavra, ou 0 se não a nomeia */
static int cortes_do_documento(const Gl *g, int ini, int fim, int *pos, int cap){
    int len = fim - ini;
    if(len < 3 || len > 46) return 0;
    char pal[48];
    for(int i = 0; i < len; i++) pal[i] = (char)g[ini+i].g;
    pal[len] = 0;
    for(int t = 0; t < N_HIF; t++){
        if(strcmp(HIF[t].pal, pal)) continue;
        int n = 0;
        for(int c = 0; c < HIF[t].nc && n < cap; c++) pos[n++] = ini + HIF[t].cortes[c];
        return n;
    }
    return 0;
}

static void quebra_e_desenrola(Est *e, int ultima){
    long corpo = ((e->L.deg >= 0 ? escala_corpo(e->L.deg)
                     : e->L.nivel ? escala_corpo(e->L.nivel <= 1 ? d_cap()
                                  : (e->L.nivel == 2 ? D_SEC : D_SUB))
                                  : escala_corpo(D_TEXTO)));
    CORPO_CORRENTE = corpo;   /* medir com o desenho DESTE corpo, e nao de outro */
    /* A LARGURA DISPONÍVEL É A DA COLUNA, dentro de uma tabela — e não a da página.
     *
     * Sem isto a célula quebrava só ao chegar à margem direita, logo transbordava para a
     * coluna seguinte e a palavra da coluna ao lado ficava por baixo. As invasões subiram de
     * 0 para 2841 no catálogo assim que a tabela passou a compor — e o defeito não era a
     * tabela: era a quebra a usar a régua errada. Dentro de uma coluna a régua é a coluna. */
    /* a goteira é UM \tabcolsep: a coluna já leva o sep dos dois lados na largura, e o
     * conteúdo fica com coluna − sep — o 6 escrito à mão saiu com a repartição Fibonacci */
    if(e->tab) e->L.larg = (int)(e->tab_larg - (TABCOLSEP + 500) / 1000);
    else       e->L.larg = 0;
    long alvo = (long)(e->L.larg > 0 ? e->L.larg : COL - e->L.recuo) * 1000;
    /* A quebra é GREEDY: enche até não caber e corta no último espaço.
     *
     * Tentei substituí-la pela segunda coordenada da cruz --- `x ⊗ x†` com a estaca
     * `w ↦ A − w`, minimizada por programação dinâmica sobre o parágrafo. A leitura é
     * fiel: a soma é constante e não distingue (thm:cruzunica), o produto é zero na
     * coluna cheia e máximo no ponto fixo `A/2`, e é o mesmo `4k(N−k)` da secção
     * normaregua.
     *
     * MEDIDO, e ficou PIOR onde importa: o esticamento médio desceu de 0,970 para 0,836,
     * mas o PIOR CASO subiu de 9,5 para 397,4 pt e o desvio de 0,69 para 8,50. Uma média
     * melhor com um pior caso quarenta vezes maior não é uma composição melhor --- é a
     * mesma tinta mal distribuída, e quem lê vê o pior caso, não a média.
     *
     * Fica registado porque o próximo a olhar para isto merece saber que foi tentado e o
     * que faltou: não foi a régua (essa é a da teoria), foi eu não ter percebido de onde
     * vinham as linhas de 397 pt --- três hipóteses erradas seguidas, e nenhuma medida a
     * confirmá-las. */
    while(e->L.n){
        int corte = e->L.n, ate = 0; long w6 = 0; int corta_palavra = 0;
        long alvo6 = alvo * 1000;      /* o produto cruzado: compara-se em 10^-6, sem dividir */
        /* Reserva H do `\boxed` incompleto: a mesma quantidade do pintor
         * (pad+m3 por aresta). É a projecção π_k tipográfica — o que falta
         * no buffer conta-se como se o Maestro já tivesse fechado a moldura. */
        long falta_cx = 0;
        { long dvl = corpo - corpo_exp_m(corpo, 1);
          if(dvl <= 0) dvl = corpo / 5;
          long pad = dvl + sem_resp(dvl), m3 = sem_resp(pad);
          long lado = (pad + m3) * 1000;
          int n8 = 0, n9 = 0;
          for(int t = 0; t < e->L.n; t++){
              if(e->L.g[t].g == 8) n8++;
              if(e->L.g[t].g == 9) n9++;
          }
          int abertas = n8 - n9;
          /* BX_ON sem 8 no buffer: falta o fecho (uma aresta) — não somar 2× */
          if(BX_ON && n8 == 0) abertas++;
          if(abertas > 0) falta_cx += lado * (long)abertas;
        }
        long seg6 = 0, rot6 = 0; int em_rot = 0;
        long n46 = 0, d46 = 0; int em46 = 0;
        long q_cw[8]; long q_cel = 0; int q8 = 0, q_c = 0, q_nc = 0;
        int em_caixa = 0, caixa_ini = -1;   /* \boxed: átomo — não parte no meio */
        for(int i = 0; i < e->L.n; i++){
            long wg;
            if(e->L.g[i].g == 0x82 || e->L.g[i].g == 0x83 || e->L.g[i].g == 0x84){
                /* mesma régua Td que mede()/desenrola — sem ×corpo (MAX_FUN) */
                long td = (e->L.g[i].g == 0x82) ? 11266L
                        : (e->L.g[i].g == 0x83) ? 11775L : 10000L;
                wg = td * 1000;
                w6 += wg; seg6 += wg;
                continue;
            }
            wg = (long)largura(e->L.g[i].g, e->L.g[i].f) * corpo_exp_m(corpo, e->L.g[i].e);
            if(e->L.g[i].e == 5 && (i == 0 || (e->L.g[i-1].e != 5 &&
                                    e->L.g[i-1].e != 2 && e->L.g[i-1].e != -2
                                    && e->L.g[i-1].e < 16)))
                w6 += 5 * (corpo - corpo_exp_m(corpo, 1)) / 3 * 1000;   /* o gancho na régua
                                       * do acumulador (10^-6) — o ±2 interior não o recomeça */
            /* A MATRIZ MEDE O BLOCO (máx de cada coluna, somado) e não se parte */
            if(e->L.g[i].e == 8 || e->L.g[i].e == -8){
                if(!q8){ q8 = 1; q_c = 0; q_nc = 1; q_cel = 0; for(int t = 0; t < 8; t++) q_cw[t] = 0; }
                int gk = e->L.g[i].g;
                if(gk == 2 || gk == 3){
                    if(q_cel > q_cw[q_c]) q_cw[q_c] = q_cel; q_cel = 0;
                    if(gk == 2){ if(q_c < 7) q_c++; if(q_c + 1 > q_nc) q_nc = q_c + 1; }
                    else q_c = 0;
                } else if(gk >= 4 && gk <= 7)
                    w6 += (long)largura(gk <= 5 ? (gk == 4 ? '(' : ')') : (gk == 6 ? '[' : ']'),
                                        e->L.g[i].f) * corpo;
                else q_cel += wg;
            }
            else if(q8){
                if(q_cel > q_cw[q_c]) q_cw[q_c] = q_cel;
                long bloco = (long)(q_nc - 1) * (corpo - corpo_exp_m(corpo, 1)) * 1000;
                for(int t = 0; t < q_nc; t++) bloco += q_cw[t];
                w6 += bloco; seg6 += bloco; q_cel = 0; q8 = 0;
                i--; continue;              /* o glifo corrente reprocessa-se fora do bloco */
            }
            else
            if(e->L.g[i].e >= 16 && (i == 0 || e->L.g[i-1].e == 0
                    || (e->L.g[i-1].e >= 16
                        && ESP_NV[e->L.g[i].e-16] > ESP_NV[e->L.g[i-1].e-16]))){
                long ka = (i > 0 && e->L.g[i-1].e >= 16)
                        ? corpo_exp_m(corpo, e->L.g[i-1].e) : corpo;
                long kx = sem_resp(ka - corpo_exp_m(corpo, e->L.g[i].e));
                if(kx > 0){ w6 += kx * 1000; seg6 += kx * 1000; }
            }
            if(e->L.g[i].g == 8){ em_caixa = 1; caixa_ini = i; }
            if(e->L.g[i].g == 8 || e->L.g[i].g == 9){
                /* mesma régua: pad + m3 por aresta */
                long cbx = corpo_exp_m(corpo, e->L.g[i].e);
                long dvl = cbx - corpo_exp_m(corpo, 1);
                if(dvl <= 0 || dvl >= cbx) dvl = cbx / 5;
                long pad = dvl + sem_resp(dvl);
                long m3  = sem_resp(pad);
                long lado = (pad + m3) * 1000;       /* um lado: abre OU fecha */
                w6 += lado; seg6 += lado;
                if(e->L.g[i].g == 9){ em_caixa = 0; caixa_ini = -1; }
            }
            else if(e->L.g[i].g >= 4 && e->L.g[i].g <= 13
                    && e->L.g[i].e != 8 && e->L.g[i].e != -8){
                int c2 = (e->L.g[i].g == 4) ? '(' : (e->L.g[i].g == 5) ? ')'
                       : (e->L.g[i].g == 6) ? '[' : (e->L.g[i].g == 7) ? ']'
                       : (e->L.g[i].g == 10) ? '{'
                       : (e->L.g[i].g == 11) ? '}' : 0xE1;
                long wd = (long)largura(c2, e->L.g[i].g >= 12 ? F_SIM : e->L.g[i].f) * corpo;
                w6 += wd; seg6 += wd; }
            else if(e->L.g[i].e == 4 || e->L.g[i].e == 6 || e->L.g[i].e == 7){ n46 += wg; em46 = 1; }
            else if(e->L.g[i].e == -4 || e->L.g[i].e == -6 || e->L.g[i].e == -7){ d46 += wg; em46 = 1; }
            else if(e->L.g[i].e == -3){ em_rot = 1; rot6 += wg; }
            else {
                if(em46){ long m4 = n46 > d46 ? n46 : d46; w6 += m4; seg6 += m4;
                          n46 = d46 = 0; em46 = 0; }
                if(em_rot){ if(rot6 > seg6) w6 += rot6 - seg6; seg6 = 0; rot6 = 0; em_rot = 0; }
                w6 += wg; seg6 += wg;
            }
            if(w6 + falta_cx > alvo6){
                /* `\boxed` curto no meio da linha: corta ANTES (átomo). Se a caixa
                 * É a linha (display) e não cabe, parte por dentro — senão o
                 * texto de L945 saía numa só linha a furar a margem e a moldura
                 * ia com ele (corpo_topologico \[boxed\] longo). */
                if(em_caixa && caixa_ini > 0 && !ate)
                    corte = caixa_ini;
                else if(em_caixa && caixa_ini > 0 && ate && ate < caixa_ini)
                    corte = ate;
                else
                    corte = ate ? ate : i;
                break;
            }
            /* espaço e `\quad`(0xA0): pontos de quebra. Dentro do `\boxed` também,
             * quando a caixa começa a linha — senão a frase longa não parte. */
            { int gg = e->L.g[i].g;
              if((gg == ' ' || gg == 0xA0) && e->L.g[i].e != -3 && e->L.g[i].e != -5
                 && (!em_caixa || caixa_ini == 0 || BX_ON))
                  ate = i; }
        }
        /* DESCER AO NÍVEL SEGUINTE. A palavra que não coube deixa o resto preso; parte-se
         * pela sílaba e enche-se o que ainda cabe. É a mesma regra posicional, um nível
         * abaixo — e sem ela o esticamento nunca zera. */
        if(corte < e->L.n){
            int pa = corte; while(pa < e->L.n && e->L.g[pa].g != ' ') pa++;   /* fim da palavra */
            int ini = corte; while(ini > 0 && e->L.g[ini-1].g != ' ') ini--;  /* e o início */
            if(pa - ini >= 4){
                int pos[16];
                int ns = cortes_do_documento(e->L.g, ini, pa, pos, 16);
                long base6 = 0;                    /* tudo em 10^-6: o produto, sem dividir */
                for(int k2 = 0; k2 < ini; k2++)
                    base6 += (long)largura(e->L.g[k2].g, e->L.g[k2].f) * corpo_exp_m(corpo, e->L.g[k2].e);
                long wh6 = (long)largura('-', e->L.g[ini].f) * corpo;
                int melhor = -1;
                for(int t = 0; t < ns; t++){
                    long w2 = base6;
                    for(int k2 = ini; k2 < pos[t]; k2++)
                        w2 += (long)largura(e->L.g[k2].g, e->L.g[k2].f) * corpo_exp_m(corpo, e->L.g[k2].e);
                    if(w2 + wh6 <= alvo6) melhor = pos[t];    /* o maior que ainda cabe */
                }
                if(melhor > 0){ corte = melhor; corta_palavra = 1; }
                HIFENS += (melhor > 0);
            }
        }
        /* UMA PALAVRA NÃO SE PARTE SEM SINAL. Dentro de uma célula a coluna pode ser mais
         * estreita que a palavra --- as larguras são Fibonacci, escolha minha e não
         * derivada --- e aí o corte caía a meio: «travess»/«ão», sem hífen e sem aviso.
         * Transbordar é feio; partir uma palavra a meio sem sinal é ERRADO, e o leitor não
         * tem como saber que foi o compositor. Enquanto a largura da coluna não sair do
         * conteúdo, a palavra ganha. */
        if(corte > 0 && corte < e->L.n && !corta_palavra
           && e->L.g[corte].g != ' ' && e->L.g[corte-1].g != ' '){
            int q = corte; while(q < e->L.n && e->L.g[q].g != ' ') q++;
            corte = q;
        }
        if(corte <= 0) corte = 1;
        /* IN-PLACE: a estrela não armazena (corpo_analitico §estrela). No traduz uma
         * `Linha` local no quadro é buffer a mais — e `Linha out = e->L` nem sequer
         * copia `g[]` (estrutura vale o endereço). Emite-se o prefixo sobre a própria
         * linha: baixa-se `n`, desenrola, repõe. */
        int n_orig = e->L.n;
        int n_emit = corte;
        while(n_emit && (e->L.g[n_emit-1].g == ' ' || e->L.g[n_emit-1].g == 0xA0)) n_emit--;
        /* partiu no meio do `\boxed`: o 8 fica na 1.ª linha e o 9 na última —
         * a moldura multi-linha fecha-se no pintor (BX_*), sem injectar 8/9. */
        unsigned char gg = 0, ff = 0; signed char ee = 0; int com_hifen = 0;
        if(corta_palavra && n_emit > 0 && n_emit < MAXLIN){
            gg = e->L.g[n_emit].g; ff = e->L.g[n_emit].f; ee = e->L.g[n_emit].e;
            e->L.g[n_emit].g = '-';
            e->L.g[n_emit].f = e->L.g[n_emit-1].f;
            e->L.g[n_emit].e = e->L.g[n_emit-1].e;
            n_emit++; com_hifen = 1;
        }
        e->L.n = n_emit;
        int fim = (corte == n_orig);
        /* moldura aberta: não estica (a reserva H já é a projecção). Fechada: ok. */
        { int jcx = BX_ON, n8 = 0, n9 = 0;
          for(int t = 0; t < n_emit; t++){
              if(e->L.g[t].g == 8) n8++;
              if(e->L.g[t].g == 9) n9++;
          }
          if(n8 > n9) jcx = 1;
          /* morf_h (thm:morfologico): dilata mesmo a última linha — ocupa [0,W]. */
          desenrola(e->p, &e->L, (e->morf_h || !(fim && ultima)) && !e->L.nivel && !jcx);
        }
        if(com_hifen){ n_emit--; e->L.g[n_emit].g = gg; e->L.g[n_emit].f = ff; e->L.g[n_emit].e = ee; }
        e->L.n = n_orig;
        if(fim){ e->L.n = 0; break; }
        int k = corte;
        if(!corta_palavra) while(k < e->L.n && (e->L.g[k].g == ' ' || e->L.g[k].g == 0xA0)) k++;
        memmove(e->L.g, e->L.g + k, (size_t)(e->L.n - k) * sizeof(Gl));
        e->L.n -= k;
    }
}

static void fecha_paragrafo(Est *e){
    if(e->L.n) quebra_e_desenrola(e, 1);
    e->L.n = 0; e->L.nivel = 0; e->L.recuo = e->recuo;
    e->morf_h = 0;   /* a dilatação vale um parágrafo (uma voz) */
}

/* A BIBLIOGRAFIA: os \bibitem recolhem-se POR ORDEM antes de compor — a chave
 * rotula, o NÚMERO cita, como o gabarito numera. A tabela reenche-se a cada
 * passagem (a mesma mecânica do sumário). */
static char BIB_CHAVE[3072];   /* 64 chaves x 48, achatado: o traduz indexa 1D */
static int N_BIB = 0;
static void bib_recolhe(const char *s, long n){
    N_BIB = 0;
    for(long i = 0; i + 9 < n; i++){
        if(s[i] != '\\' || strncmp(s + i, "\\bibitem{", 9)) continue;
        long a = i + 9; int k = 0;
        while(a < n && s[a] != '}' && k < 47){ BIB_CHAVE[N_BIB * 48 + k] = s[a]; k++; a++; }
        BIB_CHAVE[N_BIB * 48 + k] = 0;
        if(k && N_BIB < 63) N_BIB++;
        i = a;
    }
}
static int bib_num(const char *c, int len){
    for(int t = 0; t < N_BIB; t++)
        if((int)strlen(BIB_CHAVE + t * 48) == len && !strncmp(BIB_CHAVE + t * 48, c, (size_t)len))
            return t + 1;
    return 0;
}

static void compila(const char *s, Pdf *p, long *glifos){
    Est *e = EST_E;
    memset(e, 0, sizeof(Est));
    interface_do_tex(s, (long)strlen(s));
    BX_ON = 0; BX_ATESTOU = 0; BX_FECHOU = 0;
    BX_Y0 = 0; BX_Y1 = 0; BX_X0 = 0; BX_XM = 0; BX_PAD = 0; BX_M3 = 0;
    e->p = p; e->fonte = F_REG; e->nfp = 0;
    long i = 0, n = (long)strlen(s);
    bib_recolhe(s, n);

    /* o preâmbulo não se desenrola: começa-se no \begin{document} se ele existir */
    const char *doc = strstr(s, "\\begin{document}");
    if(doc) i = (doc - s) + 16;

    while(i < n){
        /* o ramo do `\ifSubfiles` que nao e' o nosso salta-se ANTES de tudo: se o
         * salto vier depois do handler de comandos, o `\part*` ja' foi composto. */
        if(SALTA_DE >= 0 && i >= SALTA_DE && i < SALTA_ATE){ i = SALTA_ATE; SALTA_DE = -1; continue; }
        unsigned char c = (unsigned char)s[i];

        if(c == '%'){                                   /* o comentário do LaTeX vai até ao fim da linha */
            while(i < n && s[i] != '\n') i++;
            continue;
        }
        if(c == '\n'){
            long j = i; int brancas = 0;
            while(j < n && (s[j] == '\n' || s[j] == ' ' || s[j] == '\t' || s[j] == '\r')){
                if(s[j] == '\n') brancas++;
                j++;
            }
            if(brancas >= 2){
                fecha_paragrafo(e); p->y -= 5*PT;
                /* o TeX tambem nao deixa uma formula atravessar paragrafo ("Missing $ inserted").
                 * Sem isto, UM cifrao desirmanado apaga o resto do documento — e foi exatamente
                 * o que aconteceu. Fechar aqui limita o dano de qualquer $ solto a um paragrafo. */
                e->mat = 0; e->fonte = F_REG; e->nfp = 0; e->fonte_txt = 0;
                e->exp = 0; e->exp1 = 0; e->nexp = 0;     /* o expoente morre com a fórmula */
                i = j; continue;
            }
            if(!e->mat) espaco_se_falta(e);   /* na fórmula o \n é só ar: o TeX ignora-o */
            i = j; continue;
        }
        /* O `&` MUDA DE COLUNA — e é só isso: fecha a célula onde está e abre a seguinte.
         * Fora de tabela é um caractere como outro qualquer, e por isso ia parar à página. */
        if(c == '&' && e->matriz){          /* a coluna da matriz: separador de controlo */
            empurra(e, 2, e->fonte);
            i++; continue;
        }
        if(c == '&' && e->mat && !e->tab){   /* o ponto de alinhamento do align: engole-se */
            espaco_se_falta(e);
            i++; continue;
        }
        if(c == '&' && e->tab){
            /* A CÉLULA NÃO SE JUSTIFICA. O `0` aqui dizia «não é a última linha», e o efeito
             * é justificar: as palavras espalham-se até à margem da coluna, e uma célula de
             * duas palavras sai com um vão enorme no meio — «o ... Operador», «tudo se move
             * em volta». Numa coluna `l` o texto alinha à esquerda e acabou.
             *
             * O comentário que eu tinha escrito ao lado dizia «sem justificar» — e o código
             * fazia o contrário. Escrevi a intenção e passei o argumento oposto. */
            if(e->L.n) quebra_e_desenrola(e, 1);       /* 1 = última: NÃO justifica */
            e->L.n = 0;
            e->tab_col++;
            if(e->tab_col >= e->tab_ncol) e->tab_col = e->tab_ncol - 1;
            {   long r = e->tab_x0 - MARGEM;      /* a coluna conta a partir do bloco */
                for(int k = 0; k < e->tab_col && k < 16; k++) r += e->tab_w[k];
                e->L.recuo = (int)r;
                e->tab_larg = e->tab_w[e->tab_col < 16 ? e->tab_col : 15];
            }
            /* SOBE-SE EXACTAMENTE O QUE SE DESCEU, e não o valor exacto da escala: o
             * `desenrola` desce `alt`, que é INTEIRO (arredondado). Subir 15,22 quando se
             * desceu 15 deixa 0,22 pt por célula — e numa tabela de quatro colunas são 0,66 pt
             * por fila, que acumulam ao longo da tabela. É o mesmo caos das palavras, na
             * vertical: o resíduo pequeno propaga-se. */
            /* AS CÉLULAS SÃO INDEPENDENTES: cada uma começa no TOPO DA FILA.
             *
             * É a dourada aplicada. A ALTURA de uma célula é uma DILATAÇÃO — quantas vezes a
             * coluna cabe no conteúdo, uma divisão, o eixo R⁺ — e a POSIÇÃO da fila é uma
             * TRANSLAÇÃO, uma soma, o eixo aditivo. São eixos DIFERENTES, e a lei diz o que
             * isso obriga: «a dilatação SÓ GIRA A FASE», |λ^{-iτ}| = 1 — mudar a escala de uma
             * coisa NÃO MOVE AS OUTRAS.
             *
             * Eu somava uma entrelinha por célula, o que é medir a dilatação com a régua da
             * translação: uma célula de duas linhas empurrava a vizinha uma linha abaixo. Era
             * uma célula a pular linha para dar espaço a outra — e isso não existe.
             *
             * Agora cada uma parte de `tab_y`, o topo da fila, e o que ela gastou fica só em
             * `tab_ymin` — que serve para a FILA descer no fim, e não para a vizinha se
             * acomodar. A fila desce o MÁXIMO das alturas, nunca a soma. */
            /* E REGISTA-SE O QUE ELA GASTOU, antes de repor. Sem isto ninguém conta a altura
             * da célula, o `tab_ymin` fica no topo, e a fila desce uma linha só — as filas
             * amontoavam-se na primeira coluna. É a outra metade da lei: a dilatação não move
             * as vizinhas, mas a FILA tem de saber qual foi a maior. */
            /* E SE A PAGINA VIROU DENTRO DA CELULA, o topo da fila e' o topo da pagina NOVA.
             *
             * Sem isto o `tab_y` guarda um `y` da pagina anterior e a celula seguinte nasce la'
             * — ou, pior, a altura da celula que acabou de ser composta perde-se e a fila
             * seguinte cai por cima dela. Era o `negra` e o `rei` sobrepostos na tabela que
             * atravessa da pagina 9 para a 10. */
            /* A página só muda o topo da fila se a célula a ABRIU (abriu_agora).
             * Comparar npag≠tab_pag com tab_pag por inicializar fazia CADA `&`
             * baixar tab_y — a involução da fila saía da diagonal (escada). */
            if(e->p->abriu_agora){ e->tab_y = e->p->y; e->tab_ymin = e->p->y;
                                  e->tab_pag = e->p->npag; e->p->abriu_agora = 0; }
            else if(e->p->y < e->tab_ymin) e->tab_ymin = e->p->y;
            e->p->y = e->tab_y;
            i++; continue;
        }
        /* O `~` E' UM ESPACO — o nao-quebravel do TeX. Sem isto sai como til literal e cola
         * as duas palavras: «Livro~I» em vez de «Livro I». */
        if(c == '~'){
            espaco_se_falta(e);
            i++; continue;
        }
        if(c == ' ' || c == '\t'){
            /* NO MODO MATEMÁTICO O ESPAÇO NÃO É GLIFO: o TeX ignora-o, e o espaçamento
             * vem das regras (relações, binários, \quad). O «) ,» do fim de fórmula
             * colava um espaço antes da vírgula que o gabarito não tem. */
            if(!e->mat) espaco_se_falta(e);
            i++; continue;
        }
        if(c == '$'){
            if(i + 1 < n && s[i+1] == '$'){ i += 2; } else i++;
            e->mat = !e->mat;
            /* a fórmula fechou: o expoente não sobrevive ao cifrão — um estado que só se
             * liga apaga o que vem depois, e este desliga-se com o modo */
            if(!e->mat){ e->exp = 0; e->exp1 = 0; e->nexp = 0; mat_sai(e); }
            else mat_entra(e);
            continue;
        }
        /* O EXPOENTE DO MODO MATEMÁTICO: `^` sobe, `_` desce — o corpo do número, grau 2
         * ou grau 4. O sinal é a Lei 1 (o valor); a escala é a dobra (σ⁻², e σ⁻⁴ no
         * expoente de expoente). Um grupo `^{...}` marca até a chaveta fechar; um token só
         * marca um glifo e devolve. Fora do modo, o caractere segue literal, como estava. */
        if(e->mat && (c == '^' || c == '_')){
            int sinal = (c == '^') ? 1 : -1;
            int niv = e->exp ? 2 : 1;                   /* dentro de expoente: a dobra dupla */
            long j2 = i + 1;
            while(j2 < n && (s[j2] == ' ' || s[j2] == '\t')) j2++;
            if(j2 < n && s[j2] == '{'){
                if(e->nexp < 8){ e->exp_pf[e->nexp] = PROF; e->exp_ant[e->nexp] = e->exp;
                                e->exp_frac[e->nexp] = 0; e->nexp++; }
                /* o `_` de um \underbrace é o RÓTULO: marca própria (-3); e DENTRO da
                 * pilha do \frac o expoente fica NO bloco: ±6 sobe, ±7 desce, na dobra */
                if(c == '_' && e->ub && PROF == e->ub_pf) e->exp = -3;
                else if(e->exp == 4)  e->exp = (c == '^') ? 6 : 7;
                else if(e->exp == -4) e->exp = (c == '^') ? -6 : -7;
                else if(e->exp == 6 || e->exp == 7 || e->exp == -6 || e->exp == -7)
                    ;                        /* já no bloco da pilha: fica nele */
                /* o GIRO DA ESPIRAL: do nível corrente (ou da base, se o estado é o
                 * radicando/matriz), escala e subida compõem-se de uma vez — o
                 * expoente de expoente deixa de ter teto em dois */
                else e->exp = esp_gira(e->exp >= 16 ? e->exp : 0, sinal);
                (void)niv;
                /* o ub só morre ao SEU nível: um ^ dentro do segmento não o mata —
                 * «2^n» dentro do \underbrace roubava o rótulo, que saía subscrito */
                if(PROF == e->ub_pf || e->exp == -3) e->ub = 0;
                PROF++;                                /* a chaveta consome-se aqui */
                i = j2 + 1; continue;
            }
            e->exp_volta = e->exp;
            if(e->exp == 4)       e->exp = (c == '^') ? 6 : 7;
            else if(e->exp == -4) e->exp = (c == '^') ? -6 : -7;
            else if(e->exp == 6 || e->exp == 7 || e->exp == -6 || e->exp == -7)
                ;                            /* idem: o bloco segura o aninhado */
            else e->exp = esp_gira(e->exp >= 16 ? e->exp : 0, sinal);
            e->exp1 = 1;
            i = j2; continue;
        }
        /* AS CHAVETAS CONTAM-SE AQUI, que é onde elas são vistas. O contador de
         * profundidade estava mais abaixo, depois deste `continue`, e por isso o `PROF`
         * nunca mudava e NENHUM escopo fechava: o `\color{cVenom}` do `\Ven` pintava o
         * resto do documento. MEDIDO: 57 usos de rgb(0,616 0 0,776) numa página que o
         * gabarito tem a preto. */
        if(c == '{' || c == '}'){
            if(c == '{') PROF++;
            else {
                if(PROF > 0) PROF--;
                if(DEG_PROF >= 0 && PROF < DEG_PROF){ DEG_FORCADO = -1; DEG_PROF = -1; }
                if(COR_PROF >= 0 && PROF < COR_PROF){ COR_TEXTO[0] = 0;  COR_PROF = -1; }
                /* a fonte do grupo que fechou repõe-se pela pilha */
                while(e->nfp > 0 && e->fp_p[e->nfp-1] >= PROF){ e->nfp--; e->fonte = e->fp_f[e->nfp]; }
                /* o grupo do expoente fecha com a sua chaveta: o exp de fora volta — e o
                 * NUMERADOR de um \frac, ao fechar, emite a barra e abre o denominador */
                while(e->nexp > 0 && PROF <= e->exp_pf[e->nexp-1]){
                    int fr = e->exp_frac[e->nexp-1];
                    e->nexp--; e->exp = e->exp_ant[e->nexp];
                    if(fr == 3) empurra(e, 9, e->fonte);   /* fecha a moldura do \boxed */
                    if((fr == 1 || fr == 2) && i + 1 < n && s[i+1] == '{'){
                        if(fr == 1) empurra(e, '/', e->fonte);
                        e->exp_pf[e->nexp] = PROF; e->exp_ant[e->nexp] = e->exp;
                        e->exp_frac[e->nexp] = 0; e->nexp++;
                        e->exp = (fr == 2) ? -4 : -1; PROF++;
                        i++;                       /* consome também o '{' do denominador */
                        break;
                    }
                }
            }
            i++; continue;
        }

        if(c == '\\'){
            long j = i + 1;
            if(j < n && !isalpha((unsigned char)s[j])){  /* \\ , \{ , \% , \_ ... */
                if(s[j] == '\\' && e->matriz){           /* a fila da matriz */
                    empurra(e, 3, e->fonte);
                    i = j + 1; continue;
                }
                if(s[j] == '\\'){
                    /* E O `\\[3pt]` LEVA ARGUMENTO OPCIONAL — o espaço extra a seguir. Sem o
                     * comer, o `[3pt]` sai como TEXTO e cai por cima da célula seguinte. */
                    long q2 = j + 1;
                    while(q2 < n && (s[q2] == ' ' || s[q2] == '\t')) q2++;
                    /* O `\\[6mm]` É ESPAÇAMENTO, E ESPAÇAMENTO SOMA. O `escala_espaco.c`
                     * di-lo e é o par que não se parte: «o espaçamento SOMA (x+w, a posição
                     * avança), a escala MULTIPLICA (w·s, o tamanho estica)». Este argumento
                     * era lido e DEITADO FORA — e por isso a capa saía toda comprimida no
                     * terço de cima, com os sete espaços que o estilo declara a valer zero. */
                    long vsalto = 0;
                    if(q2 < n && s[q2] == '['){
                        long m = medida_mil(s + q2 + 1);
                        if(m >= 0) vsalto = m;
                        while(q2 < n && s[q2] != ']') q2++;
                        if(q2 < n) j = q2;
                    }
                    /* O `\\` FECHA A LINHA DA TABELA: volta à primeira coluna e desce UMA vez.
                     *
                     * Sem isto cada célula descia a sua, e uma tabela de 4 colunas gastava 4
                     * linhas por fila — o texto corrido que se via. A conta é a mesma da página:
                     * o `&` anda em x e o `\\` anda em y. */
                    if(!e->tab && vsalto > 0){ fecha_paragrafo(e); e->p->y -= vsalto; }
                    if(e->tab){
                        if(e->L.n) quebra_e_desenrola(e, 1);   /* idem: a célula não justifica */
                        e->L.n = 0;
                        if(e->p->abriu_agora){ e->tab_ymin = e->p->y; e->tab_pag = e->p->npag;
                                              e->p->abriu_agora = 0; }
                        /* O MÍNIMO NÃO ATRAVESSA A PÁGINA. Se a fila virou de página no meio,
                         * o `tab_ymin` da anterior é menor que QUALQUER y da nova, e o `min`
                         * mantinha-o: a fila seguinte começava no fundo e deixava a página
                         * quase toda em branco. Vi-o na página 3 do enredo — uma linha no
                         * topo, outra no fundo, e um buraco de mil pontos entre elas. */
                        if(e->p->npag != e->tab_pag){ e->tab_ymin = e->p->y; e->tab_pag = e->p->npag; }
                        else if(e->p->y < e->tab_ymin) e->tab_ymin = e->p->y;
                        e->tab_col = 0;
                        e->L.recuo = (int)(e->tab_x0 - MARGEM);
                        e->tab_larg = e->tab_w[0];
                        e->p->y = e->tab_ymin;           /* a fila desce o que a mais alta gastou */
                        /* A QUEBRA DE PÁGINA DECIDE-SE POR FILA, e não por célula.
                         *
                         * O `desenrola` abre página quando a LINHA não cabe — e numa tabela cada
                         * célula chama-o de sua vez, logo a primeira ficava numa página e a
                         * segunda na seguinte, com a fila partida ao meio. E o `tab_y`, que é o
                         * topo da fila, apontava para a página anterior: a célula seguinte
                         * nascia fora do papel.
                         *
                         * Aqui olha-se ANTES de compor a fila: se ela não cabe, abre-se a página
                         * agora, e a fila INTEIRA vai para lá. A unidade que atravessa a
                         * fronteira é a fila, não a célula — como no texto é a linha e não a
                         * palavra. */
                        {   long alt_fila = escala_entre(D_TEXTO);   /* já na régua do Td */
                            if(e->p->y - alt_fila < FUNDO){
                                pagina_fecha(e->p);
                                pagina_abre(e->p);
                                /* e as réguas de topo repetem-se? não: o que se repete é o
                                 * ESPAÇO. Uma régua a mais seria conteúdo inventado. */
                            }
                        }
                        e->tab_y = e->p->y;
                        e->tab_ymin = e->p->y;
                        i = j + 1; continue;
                    }
                    fecha_paragrafo(e); i = j + 1; continue;
                }
                /* O `\[` ... `\]` E' A FORMULA DE DESTAQUE: o modo matematico liga numa
                 * linha propria, centrada — como o pdflatex a poe. Sem esta porta a formula
                 * saia LITERAL no meio do paragrafo, com `[`, `^` e `_` crus — foi o resumo
                 * quebrado do teoria.tex. */
                if(s[j] == '[' || s[j] == ']'){
                    fecha_paragrafo(e);
                    if(s[j] == '['){ e->centra_mat = CENTRA; CENTRA = 1; e->mat = 1; e->disp = 1; mat_entra(e); }
                    else { CENTRA = e->centra_mat; e->mat = 0; e->disp = 0; e->exp = 0; e->exp1 = 0; e->nexp = 0; mat_sai(e); }
                    i = j + 1; continue;
                }
                /* `\'` `\~` `\^`: compõem com base; sem base o glifo fica AO MEIO
                 * (o `~` da fonte de texto é acento ALTO — «Hurwitz~» flutuava). */
                if(s[j] == '\'' || s[j] == '~' || s[j] == '^'){
                    char ac = s[j]; long k = j + 1; int abriu = 0;
                    if(k < n && s[k] == '{'){ abriu = 1; k++; }
                    char v = 0;
                    if(k < n){
                        if(s[k] == '\\' && k + 1 < n && s[k+1] == 'i'){ v = 'i'; k += 2; }
                        else if(s[k] != '}'){ v = s[k]; k++; }
                    }
                    if(abriu){ while(k < n && s[k] != '}') k++; if(k < n) k++; }
                    unsigned char comp = 0;
                    if(v){ const char *pv = strchr(AC_VOG, v);
                                if(pv){ int tv = (int)(pv - AC_VOG); unsigned char cv = 0;
                                    if(ac == '\x27') cv = AC_AGU[tv];
                                    else if(ac == '~') cv = AC_TIL[tv];
                                    else if(ac == '^') cv = AC_CIR[tv];
                                    if(cv) comp = cv; } }
                    if(comp) empurra(e, comp, e->fonte);
                    else {
                        /* sem composição: glifo ao meio; Symbol `sim` para o til */
                        if(ac == '~') empurra(e, 0x7E, F_SIM);
                        else if(ac == '\'') empurra(e, 0xB4, e->fonte);
                        else empurra(e, 0x88, e->fonte);   /* ˆ WinAnsi, ao meio */
                        if(v) empurra(e, (unsigned char)v, e->fonte);
                    }
                    i = k; continue;
                }
                char um[2]; um[0] = s[j]; um[1] = 0;
                const Par *P = lex_acha(um);
                if(P) empurra(e, P->glifo, P->simb ? F_SIM : e->fonte);
                else if(s[j] != ',' && s[j] != ' ' && s[j] != '!' && s[j] != ';')
                    empurra(e, (unsigned char)s[j], e->fonte);
                else espaco_se_falta(e);   /* o \; depois de um `=` espacejado não DOBRA */
                i = j + 1; continue;
            }
            char cmd[64]; int k = 0;
            while(j < n && isalpha((unsigned char)s[j]) && k < 63) cmd[k++] = s[j++];
            cmd[k] = 0;
            while(j < n && (s[j] == '*' )) j++;
            /* A RÉGUA DO TeX: o espaço a seguir a um nome de controlo COME-SE — é o
             * que o pdflatex faz. Sem isto, «\footnotesize tests» (a expansão do
             * \code) punha um espaço mono antes do argumento: «( tests/...». Só o
             * espaço, não o \n: o \n é do parágrafo. */
            while(j < n && (s[j] == ' ' || s[j] == '\t')) j++;

            int nv = sec_nivel(cmd);
            if(nv){
                /* O TÍTULO MANDA NO SEU PRÓPRIO CORPO. Um `\fontsize` herdado do texto à
                 * volta ganhava ao nível — MEDIDO: 186 títulos de nível 1 compostos e só
                 * 112 a chegar ao degrau que o `\titleformat` declara. No LaTeX é o
                 * formato da secção que se aplica, não o que estava em vigor antes. */
                long deg_fora = DEG_FORCADO, prof_fora = DEG_PROF;
                DEG_FORCADO = degrau_do_comando(cmd); DEG_PROF = -1;
                char cor_fora[24]; { char *q = ap_str(cor_fora, COR_TEXTO); *q = 0; }
                { const char *cc = cor_do_comando(cmd); char *q = ap_str(COR_TEXTO, cc ? cc : ""); *q = 0; } /* A MARCA: o nível vem do nome */
                fecha_paragrafo(e);
                { long a = 0, d2 = 0;
                  long dg = degrau_do_comando(cmd);
                  espaco_titulo(cmd, dg, &a, &d2);
                  /* o `antes` do estilo é NEGATIVO no capítulo (-14pt), e no LaTeX isso é
                   * relativo à PÁGINA NOVA que o `\chapter` abre. Sem essa quebra, aplicar o
                   * negativo faz o título subir por cima do texto anterior — foi o que se viu
                   * na página 3. O piso é a entrelinha do próprio degrau: um título nunca
                   * pode invadir o que veio antes, e esse número sai da escala, não daqui. */
                  long piso = (dg >= 0 && dg < N_ESCALA) ? ESCALA[dg].entre / 2 : 8000;
                  p->y -= (a > piso ? a : piso); }
                while(j < n && s[j] != '{') j++;
                if(j < n) j++;
                /* `\part` e `\chapter` ABREM PAGINA. E' o que a classe `book` faz, e o
                 * gabarito mostra-o: as partes tem pagina propria (19 e 6 palavras nas
                 * paginas 15 e 60) e os capitulos sao consecutivos — 17, 18, 19, 20, 21.
                 * Sao 155 capitulos e 30 partes: 185 quebras que eu nao fazia, e e' isso
                 * que dava 282 paginas onde o gabarito tem 365. */
                if(nv == 1 && p->y < TOPO - PT && !p->abriu_agora){
                    pagina_fecha(p); pagina_abre(p);
                }
                if(nv == 1) p->plana = 1;             /* a abertura de capítulo é plain */
                /* A CAPA DE PARTE: `\part` tem pagina propria, o rotulo numa linha e o
                 * titulo noutra, os dois CENTRADOS — e nada mais na pagina. MEDIDO no
                 * gabarito: «Parte I» a y=288 centrado em x=263,7, e o titulo a y=335 numa
                 * caixa centrada. E' o que a classe `book` faz com o `\part`. */
                int e_parte = !strcmp(cmd, "part");
                if(e_parte){
                    p->y -= 216*PT;                    /* o gabarito poe o rotulo a y=288 */
                    CENTRA = 1;
                    char rr[64];
                    if(rotulo_seccao_ver(cmd, (i + 1 < n && s[i+5] == '*'), rr, sizeof rr) && rr[0]){
                        /* o rotulo da parte nao e' corpo de texto: o gabarito compoe-o a
                         * 18,27 de altura, que e' o degrau da SECCAO. */
                        /* pelo DEG_FORCADO e nao pelo `L.deg`: o `empurra` sobrescreve o
                         * `L.deg` com o forcado a cada linha nova, e por isso por-lo
                         * directamente nao pegava. */
                        long dg = DEG_FORCADO;
                        DEG_FORCADO = degrau_do_comando("section");
                        e->fonte = F_REG; e->nfp = 0;
                        for(int t2 = 0; rr[t2]; t2++) empurra(e, (unsigned char)rr[t2], F_REG);
                        quebra_e_desenrola(e, 1);
                        e->L.n = 0; DEG_FORCADO = dg;
                        p->y -= 18*PT;
                    }
                }
                int prof = 1; e->L.nivel = nv; e->fonte = F_NEG; e->L.recuo = 0;
                {   /* o `*` vem logo a seguir ao nome, antes do `{` do título */
                    long z = i + 1; while(z < n && isalpha((unsigned char)s[z])) z++;
                    char rot[64];
                    int estrela = (z < n && s[z] == '*');
                    /* REGISTA-SE A ENTRADA: o rótulo, o texto e a PÁGINA. Na primeira
                     * passagem enche-se a tabela; na segunda, ela já está cheia e o
                     * `\tableofcontents` compõe-na. */
                    if(!TOC_LE && N_TOC < MAX_TOC && nv <= 3){
                        Toc *t = &TOC[N_TOC];
                        t->nivel = nv; t->pag = p->npag;
                        rotulo_seccao_ver(cmd, estrela, t->rot, sizeof t->rot);
                        /* o `j` JA' aponta ao conteudo do titulo — o handler avancou-o.
                         * Procurar o `{` seguinte apanhava o do `\addcontentsline{toc}` ou
                         * o do `\begin{flushright}`, e o sumario saia com «toc» e
                         * «flushright» por titulos. */
                        long z2 = j - 1; int dd2 = 1; long f2 = j;
                        while(f2 < n && dd2){
                            if(s[f2] == '{') dd2++;
                            else if(s[f2] == '}'){ if(!--dd2) break; }
                            f2++;
                        }
                        int k2 = 0;
                        for(long w = z2 + 1; f2 > z2 && w < f2 && k2 < 159; w++){
                            if(s[w] == '\\'){ while(w + 1 < n && isalpha((unsigned char)s[w+1])) w++; continue; }
                            /* `$` `\{` e `_`/`^` da matemática: o sumário não é o compositor —
                             * senão `\mathcal{C}_K` virava «C_K» literal (secção 0.7) */
                            if(s[w] == '{' || s[w] == '}' || s[w] == '$'
                            || s[w] == '_' || s[w] == '^') continue;
                            t->txt[k2++] = s[w];
                        }
                        t->txt[k2] = 0;
                        if(k2) N_TOC++;
                    }
                    /* A MARCA DO CABEÇALHO (o \chaptermark do estilo): «N. Título» em
                     * WinAnsi, posta quando o capítulo abre — o cabeçalho só existe
                     * depois de haver capítulo, como no fancyhdr do gabarito */
                    if(nv == 1 && !strcmp(cmd, "chapter") && !estrela){
                        char *pz = ap_num(CAB_DIR, C_CAP); pz = ap_str(pz, ". ");
                        long z2 = j - 1, dd2 = 1, f2 = j;
                        while(f2 < n && dd2){
                            if(s[f2] == '{') dd2++;
                            else if(s[f2] == '}'){ if(!--dd2) break; }
                            f2++;
                        }
                        int k2 = (int)(pz - CAB_DIR);
                        for(long w = z2 + 1; w < f2 && k2 < 90; ){
                            if(s[w] == '\\'){ while(w + 1 < n && isalpha((unsigned char)s[w+1])) w++; w++; continue; }
                            if(s[w] == '{' || s[w] == '}' || s[w] == '$'
                            || s[w] == '_' || s[w] == '^'){ w++; continue; }
                            int cs2; int g2 = utf8_glifo((const unsigned char*)s + w, &cs2);
                            CAB_DIR[k2++] = (char)g2; w += cs2 ? cs2 : 1;
                        }
                        CAB_DIR[k2] = 0;
                    }
                    if(e_parte) rotulo_seccao(cmd, estrela, rot, sizeof rot);
                    if(!e_parte && rotulo_seccao(cmd, estrela, rot, sizeof rot)){
                        /* display (estilo): «Capítulo N» em gknota na linha de cima;
                         * o título grande vem só a seguir — não «Capítulo N  Título» colado */
                        int e_disp = 0;
                        if(!strcmp(cmd, "chapter")){
                            const char *bdisp = estilo_texto(NULL);
                            if(bdisp){
                                const char *qd = strstr(bdisp, "titleformat{\\chapter}");
                                if(qd){
                                    const char *fimd = strstr(qd + 1, "\\titleformat");
                                    const char *dd = strstr(qd, "[display]");
                                    if(dd && (!fimd || dd < fimd)) e_disp = 1;
                                }
                            }
                        }
                        if(e_disp){
                            long dg = DEG_FORCADO;
                            DEG_FORCADO = 0;          /* gknota — rótulo pequeno */
                            e->fonte = F_REG; e->nfp = 0;
                            { const char *co = "ouro";
                              char *qc = ap_str(COR_TEXTO, co); *qc = 0; }
                            for(int t = 0; rot[t]; t++) empurra(e, (unsigned char)rot[t], F_REG);
                            quebra_e_desenrola(e, 1);
                            e->L.n = 0; DEG_FORCADO = dg;
                            COR_TEXTO[0] = 0;
                            p->y -= 6*PT;             /* titlespacing label→título ≈ 6pt */
                        } else {
                            for(int t = 0; rot[t]; t++) empurra(e, (unsigned char)rot[t], F_NEG);
                            /* dois espaços: é o que o LaTeX põe entre o número e o título */
                            empurra(e, ' ', F_NEG); empurra(e, ' ', F_NEG);
                        }
                    }
                }
                { int tmat = 0;                       /* equações no título: $...$ */
                while(j < n && prof){
                    if(s[j] == '{') prof++;
                    else if(s[j] == '}'){ if(!--prof) break; }
                    else {
                        if(s[j] == '$'){ tmat = !tmat; j++; continue; }
                        if(s[j] == '\\'){                /* um comando dentro do título */
                            long q = j + 1;
                            /* acentos `\'` `\~` `\^`: compõem; sem base → meio */
                            if(q < n && (s[q] == '\'' || s[q] == '~' || s[q] == '^')){
                                char ac = s[q]; long k = q + 1; int abriu = 0;
                                if(k < n && s[k] == '{'){ abriu = 1; k++; }
                                char v = 0;
                                if(k < n){
                                    if(s[k] == '\\' && k + 1 < n && s[k+1] == 'i'){ v = 'i'; k += 2; }
                                    else if(s[k] != '}'){ v = s[k]; k++; }
                                }
                                if(abriu){ while(k < n && s[k] != '}') k++; if(k < n) k++; }
                                unsigned char comp = 0;
                                if(v){ const char *pv = strchr(AC_VOG, v);
                                if(pv){ int tv = (int)(pv - AC_VOG); unsigned char cv = 0;
                                    if(ac == '\x27') cv = AC_AGU[tv];
                                    else if(ac == '~') cv = AC_TIL[tv];
                                    else if(ac == '^') cv = AC_CIR[tv];
                                    if(cv) comp = cv; } }
                                if(comp) empurra(e, comp, F_NEG);
                                else {
                                    if(ac == '~') empurra(e, 0x7E, F_SIM);
                                    else if(ac == '\'') empurra(e, 0xB4, F_NEG);
                                    else empurra(e, 0x88, F_NEG);
                                    if(v) empurra(e, (unsigned char)v, F_NEG);
                                }
                                j = k; continue;
                            }
                            char c2[64]; int k2 = 0;
                            while(q < n && isalpha((unsigned char)s[q]) && k2 < 63) c2[k2++] = s[q++];
                            c2[k2] = 0;
                            /* `\mathcal{C}` `\mathbb{R}` no título: come o argumento —
                             * senão saía «C_K» literal (corpo_topologico §0.7) */
                            if(!strcmp(c2, "mathbb") || !strcmp(c2, "mathcal")
                            || !strcmp(c2, "mathrm") || !strcmp(c2, "mathit")
                            || !strcmp(c2, "mathbf") || !strcmp(c2, "text")
                            || !strcmp(c2, "textrm") || !strcmp(c2, "operatorname")){
                                long qa = q;
                                while(qa < n && (s[qa] == ' ' || s[qa] == '\t')) qa++;
                                if(qa < n && s[qa] == '{'){
                                    long fa = fecha_chave(s, n, qa);
                                    if(fa > 0){
                                        int fm = (!strcmp(c2, "mathit") || !strcmp(c2, "mathcal"))
                                               ? ((CARTA_MAT) ? F_MAT : (N_CARTA > F_ITA) ? F_ITA : F_NEG)
                                               : F_NEG;
                                        for(long z = qa + 1; z < fa; z++){
                                            int ch = (unsigned char)s[z];
                                            if(ch == ' ' || ch == '\\') continue;
                                            if(!strcmp(c2, "mathbb")){
                                                int bb = (ch == 'Z') ? 0xA7 : (ch == 'Q') ? 0xA8
                                                       : (ch == 'R') ? 0xA9 : (ch == 'N') ? 0xAA
                                                       : (ch == 'F') ? 0xAF : (ch == 'H') ? 0xB2
                                                       : (ch == 'O') ? 0xBD : (ch == 'C') ? 0xBE
                                                       : (ch == 'T') ? 0xBF : (ch == 'P') ? 0xCA
                                                       : (ch == 'K') ? 0xCB : 0;
                                                if(bb) empurra(e, bb, F_SIM);
                                                else empurra(e, ch, F_NEG);
                                            } else empurra(e, ch, fm);
                                        }
                                        j = fa + 1; continue;
                                    }
                                } else if(qa < n && s[qa] != '\\' && s[qa] != '$'){
                                    /* `\mathcal L` sem chavetas: um token */
                                    int fm = (!strcmp(c2, "mathit") || !strcmp(c2, "mathcal"))
                                           ? ((CARTA_MAT) ? F_MAT : (N_CARTA > F_ITA) ? F_ITA : F_NEG)
                                           : F_NEG;
                                    int ch = (unsigned char)s[qa];
                                    if(!strcmp(c2, "mathbb")){
                                        int bb = (ch == 'Z') ? 0xA7 : (ch == 'Q') ? 0xA8
                                               : (ch == 'R') ? 0xA9 : (ch == 'N') ? 0xAA
                                               : (ch == 'F') ? 0xAF : (ch == 'H') ? 0xB2
                                               : (ch == 'O') ? 0xBD : (ch == 'C') ? 0xBE
                                               : (ch == 'T') ? 0xBF : (ch == 'P') ? 0xCA
                                               : (ch == 'K') ? 0xCB : 0;
                                        if(bb) empurra(e, bb, F_SIM);
                                        else empurra(e, ch, F_NEG);
                                    } else empurra(e, ch, fm);
                                    j = qa + 1; continue;
                                }
                            }
                            const Par *P = lex_acha(c2);
                            if(P) empurra(e, P->glifo, P->simb ? F_SIM : F_NEG);
                            j = q; continue;
                        }
                        /* o `~` do título também é espaço (Hurwitz~8) */
                        if(s[j] == '~'){ empurra(e, ' ', F_NEG); j++; continue; }
                        /* `_` / `^` na matemática do título: sobe/desce — senão
                         * `$\mathcal{C}_K$` saía com underscore literal */
                        if(tmat && (s[j] == '_' || s[j] == '^')){
                            int sinal = (s[j] == '^') ? 1 : -1;
                            long j3 = j + 1;
                            while(j3 < n && (s[j3] == ' ' || s[j3] == '\t')) j3++;
                            int e0 = e->exp;
                            e->exp = esp_gira(e0 >= 16 ? e0 : 0, sinal);
                            if(j3 < n && s[j3] == '{'){
                                long fa = fecha_chave(s, n, j3);
                                if(fa > 0){
                                    for(long z = j3 + 1; z < fa; ){
                                        if(s[z] == '\\'){
                                            long q = z + 1; char c3[32]; int k3 = 0;
                                            while(q < fa && isalpha((unsigned char)s[q]) && k3 < 31)
                                                c3[k3++] = s[q++];
                                            c3[k3] = 0;
                                            const Par *P3 = lex_acha(c3);
                                            if(P3) empurra(e, P3->glifo, P3->simb ? F_SIM : F_NEG);
                                            z = q; continue;
                                        }
                                        if(s[z] == '{' || s[z] == '}'){ z++; continue; }
                                        int cs3; int g3 = utf8_glifo((const unsigned char*)s + z, &cs3);
                                        if(((g3 >= 'a' && g3 <= 'z') || (g3 >= 'A' && g3 <= 'Z'))
                                           && N_CARTA > F_ITA)
                                            empurra(e, g3, CARTA_MAT ? F_MAT : F_ITA);
                                        else empurra(e, g3, F_NEG);
                                        z += cs3 ? cs3 : 1;
                                    }
                                    e->exp = e0; j = fa + 1; continue;
                                }
                            }
                            if(j3 < n){
                                int cs3; int g3 = utf8_glifo((const unsigned char*)s + j3, &cs3);
                                if(((g3 >= 'a' && g3 <= 'z') || (g3 >= 'A' && g3 <= 'Z'))
                                   && N_CARTA > F_ITA)
                                    empurra(e, g3, CARTA_MAT ? F_MAT : F_ITA);
                                else empurra(e, g3, F_NEG);
                                e->exp = e0;
                                j = j3 + (cs3 ? cs3 : 1); continue;
                            }
                            e->exp = e0; j++; continue;
                        }
                        int cons; int g = utf8_glifo((const unsigned char*)s + j, &cons);
                        /* a variável da equação do título compõe na itálica matemática,
                         * como no corpo — «$\pi$» punha os cifrões na página */
                        if(tmat && ((g >= 'a' && g <= 'z') || (g >= 'A' && g <= 'Z'))
                           && N_CARTA > F_ITA){
                            empurra(e, g, CARTA_MAT ? F_MAT : F_ITA);
                            j += cons; continue;
                        }
                        /* A LIGADURA TAMBÉM AQUI. O laço dos títulos é outro, e por isso o
                         * travessão saía com três hífenes só nas secções: «a floresta e o mar
                         * --- o que estava aqui». Um defeito com dois laços é dois defeitos. */
                        { int cl = 0, lg = liga_acha(s, n, j, &cl);
                          if(lg){ empurra(e, lg, F_NEG); j += cl; continue; } }
                        if(g != '{' && g != '}') empurra(e, g, F_NEG);
                        j += cons; continue;
                    }
                    j++;
                } }
                fecha_paragrafo(e);
                if(e_parte){ CENTRA = 0; pagina_fecha(p); pagina_abre(p); p->plana = 1; }
                { long a = 0, d2 = 0;
                  espaco_titulo(cmd, degrau_do_comando(cmd), &a, &d2);
                  /* A RÉGUA DOURADA, que o `\titleformat` declara no grupo final:
                   * `[{\vspace{2mm}\color{ouro}\titlerule[1.2pt]}]`. É uma linha por
                   * baixo do título, e o tradutor já as desenha — só não a ia buscar. */
                  long esp = 0; char cr[24]; cr[0] = 0;
                  if(regua_do_comando(cmd, &esp, cr, sizeof cr) && esp > 0){
                      p->y -= d2 * 2 / 5;
                      poe_regua(p, MARGEM * 1000L, (MARGEM + COL) * 1000L,
                                p->y, esp, cr[0] ? cr : "ouro");
                      p->y -= d2 * 3 / 5;
                  } else p->y -= d2; }
                e->fonte = F_REG; e->nfp = 0; e->L.nivel = 0;
                DEG_FORCADO = deg_fora; DEG_PROF = (int)prof_fora;   /* e repõe-se ao sair */
                { char *q = ap_str(COR_TEXTO, cor_fora); *q = 0; }
                i = j + 1; continue;
            }
            /* A itálica e os versaletes ESTÃO ABERTOS (CARTAS[2] e CARTAS[3]) e ainda não
             * são escolhidos aqui: escolher sem os escrever no PDF declara `/F4` e `/F5`
             * que não existem, e o leitor recusa a página inteira — «Unknown font type».
             * Falta o objecto de fonte e o FontDescriptor de cada um. Meia coisa é pior
             * que nenhuma, e por isso a escolha fica desligada até o embutimento existir. */
            /* cada estaca tem o seu comando, e são eixos independentes */
            if(!strcmp(cmd, "textit") || !strcmp(cmd, "itshape")){
                if(e->fonte == F_NEG && CARTA_NIT) fonte_poe(e, F_NIT);
                else if(N_CARTA > F_ITA) fonte_poe(e, F_ITA);
                i = j; continue;
            }
            /* `\texttt` e `\code` pedem a MONOESPACADA, e ela e' uma estaca propria: a da
             * LARGURA. Estavam mapeados para a negra, e por isso a largura vinha de uma
             * fonte e o glifo de outra — as duas invasoes que restavam no enredo, em
             * `\texttt{broca-so} & $688$`, eram isso. */
            if(!strcmp(cmd, "texttt") || !strcmp(cmd, "ttfamily") || !strcmp(cmd, "code")){
                if(N_CARTA > F_MON) fonte_poe(e, F_MON);
                i = j; continue;
            }
            if(!strcmp(cmd, "textsc") || !strcmp(cmd, "scshape")){
                if(N_CARTA > F_VER) fonte_poe(e, F_VER);
                i = j; continue;
            }
            /* o \emph É ITÁLICA — o «formas de medida» do resumo saía a negra porque
             * este fallback era de quando só havia a Helvetica-Bold. A negra fica
             * como último recurso se a carta itálica não abriu. */
            if(!strcmp(cmd, "emph") || !strcmp(cmd, "textit")){
                /* a COMPOSIÇÃO das variantes sai da referência: itálica em contexto
                 * negro é a bold italic REAL (lmroman10-bolditalic), não a itálica
                 * a perder o peso — «todos» num \medido */
                fonte_poe(e, (e->fonte == F_NEG && CARTA_NIT) ? F_NIT
                             : (N_CARTA > F_ITA) ? F_ITA : F_NEG);
                i = j; continue;                        /* o } repõe pela pilha */
            }
            if(!strcmp(cmd, "textbf") ||
               !strcmp(cmd, "textsc") || !strcmp(cmd, "code")  || !strcmp(cmd, "texttt")){
                fonte_poe(e, F_NEG);
                i = j; continue;
            }
            if(!strcmp(cmd, "item")){
                fecha_paragrafo(e);
                e->L.recuo = e->recuo;
                empurra(e, 0xB7, F_SIM); empurra(e, ' ', F_REG);
                i = j; continue;
            }
            /* AS RÉGUAS DO BOOKTABS — e é aqui que o design entra no PDF. Cada uma é o
             * mesmo operador de caminho com grau 1: dois pontos e um traçado. A espessura
             * distingue-as, e a cor sai do estilo.tex como tudo o resto. */
            /* OS COMANDOS DE INDICE NAO SAO TEXTO: `\tocpart{X}`, `\tocchapter{X}` e afins
             * escrevem no sumario, nao na pagina. Sem os reconhecer, o NOME do comando saia
             * colado ao argumento — «tocpartO Enredo», «tocchapterIntroducao» — e o leitor via
             * o nome da instrucao no meio do texto. */
            if(!strncmp(cmd, "toc", 3) || !strcmp(cmd, "addcontentsline")
               || !strcmp(cmd, "markboth") || !strcmp(cmd, "markright")){
                long q = j;
                while(q < n && (s[q] == ' ' || s[q] == '{')){          /* come os argumentos */
                    if(s[q] == '{'){ int d2 = 1; q++;
                        while(q < n && d2){ if(s[q]=='{') d2++; else if(s[q]=='}') d2--; q++; }
                    } else q++;
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "toprule") || !strcmp(cmd, "midrule") || !strcmp(cmd, "bottomrule")
               || !strcmp(cmd, "hline")){
                fecha_paragrafo(e);
                long esp = (cmd[0] == 'm' || cmd[0] == 'h') ? 500 : 1000;   /* mid fina, top/bottom grossa */
                /* A REGUA VAI NO VAO ENTRE AS LINHAS, e nao em cima do texto.
                 *
                 * Eu desenhava em `y + 4`. O `y` e' a LINHA DE BASE da linha seguinte, e o
                 * texto dela sobe dali ate' ao ascendente — medido, 9,2 pt para um corpo de 11.
                 * Logo `y + 4` cai a meio da altura-de-x, e a regua RISCA as letras: 9 de 12
                 * cortavam palavras.
                 *
                 * O vao livre e' entre o descendente da linha de cima e o ascendente da de
                 * baixo — medido, 2,9 pt de altura. A regua vai la', e a altura toma-se do
                 * CORPO da linha, que e' o que a escala manda: para 11 pt, o ascendente e' 0,84
                 * do corpo. Nao se escolhe um numero: le-se o corpo e multiplica-se.
                 *
                 * (E' o mesmo defeito das palavras a montar, na vertical: eu punha a tinta a
                 * partir de um numero meu em vez do que a linha ocupa.) */
                /* E NAO SE ADIVINHA O VAO: ABRE-SE UM.
                 *
                 * A tentativa anterior punha a regua em `y + corpo` a contar que caisse nos
                 * 2,9 pt entre as bandas. Nao cai: das 12 reguas, 7 continuavam a cortar — umas
                 * a 11,9 pt do topo da banda (a linha de CIMA) e outras a 1,9 (a de BAIXO). Um
                 * vao de 2,9 pt nao se acerta com um numero; e' menos que a espessura do erro.
                 *
                 * Desce-se meia entrelinha, desenha-se, desce-se a outra metade. Assim a regua
                 * fica no MEIO de um vao que ela propria abriu, e nao ha' numero a acertar: o
                 * espaco existe porque foi feito. E' a mesma frase de sempre — em vez de medir
                 * contra uma regua minha, faz-se o objecto ter a propriedade. */
                /* E O VAO E' UMA ENTRELINHA INTEIRA, nao meia.
                 *
                 * Com meia, a regua ficava a 1,1 pt do topo da banda seguinte — a raspar os
                 * ascendentes. E a conta di-lo: meia entrelinha sao 7,6 pt e o ascendente sobe
                 * 9,2. Nao chegava, e 1,5 pt de folga nao e' folga.
                 *
                 * Com uma entrelinha inteira a regua ocupa o lugar de UMA LINHA que nao existe:
                 * nao ha' texto onde ela esta' porque ali nao cabe texto nenhum. Deixa de haver
                 * numero a acertar — o espaco nao se mede, faz-se. */
                long linha = escala_entre(D_TEXTO);              /* já na régua do Td */
                /* a régua vai na LARGURA DO BLOCO da tabela, não na coluna inteira —
                 * como no gabarito, onde o toprule mede o que a tabela mede */
                long rx0 = e->tab ? e->tab_x0 : MARGEM;
                long rx1 = e->tab && e->tab_tot > 0 ? e->tab_x0 + e->tab_tot : MARGEM + COL;
                e->p->y -= linha / 2;
                poe_regua(e->p, rx0 * 1000L, rx1 * 1000L, e->p->y, esp, "tinta");
                e->p->y -= linha - linha / 2;        /* o resto: a soma FECHA, resíduo 0 */
                /* E A TABELA TEM DE SABER: a régua mexeu no lápis, e o topo da fila é agora
                 * outro. Sem isto o `tab_y` guardava a posição de ANTES da régua, e a fila
                 * seguinte nascia por cima dela — o cabeçalho ficava sobre a linha de topo. */
                if(e->tab){ e->tab_y = e->p->y; e->tab_ymin = e->p->y; e->tab_pag = e->p->npag;
                    /* E A FILA SEGUINTE COMEÇA NA COLUNA 0 DO BLOCO: o fecha_paragrafo
                     * de cima apagou o recuo, e a primeira célula a seguir a um
                     * \toprule/\midrule nascia na MARGEM — «de \ para» e a primeira
                     * fila saíam fora da tabela, as outras dentro. */
                    e->tab_col = 0;
                    e->L.recuo = (int)(e->tab_x0 - MARGEM);
                    e->tab_larg = e->tab_w[0];
                }
                i = j; continue;
            }
            /* A MATRIZ: o mesmo esquema das tabelas, dentro da fórmula. O p/b põe o
             * delimitador; o small compõe na dobra. O conteúdo marca-se ±8 e os
             * separadores empurram-se como glifos de controlo (2 = &, 3 = \\).
             * O `array` É MATRIZ (com preâmbulo): sem isto caía no `tabular` de
             * texto, `fecha_paragrafo` partia o `\boxed` e a moldura saía vazia
             * (corpo_topologico L572). */
            if((!strcmp(cmd, "begin") || !strcmp(cmd, "end")) && e->mat){
                long q = ate_abre(s, j, n);
                if(q < n){
                    const char *nm = s + q + 1;
                    int small = 0, par = 0, colch = 0, ehm = 0, eh_array = 0;
                    if(!strncmp(nm, "smallmatrix}", 12)){ small = 1; ehm = 1; }
                    else if(!strncmp(nm, "psmallmatrix}", 13)){ small = 1; par = 1; ehm = 1; }
                    else if(!strncmp(nm, "pmatrix}", 8)){ par = 1; ehm = 1; }
                    else if(!strncmp(nm, "bmatrix}", 8)){ colch = 1; ehm = 1; }
                    else if(!strncmp(nm, "matrix}", 7)) ehm = 1;
                    else if(!strncmp(nm, "array}", 6)){ ehm = 1; eh_array = 1; }
                    if(ehm){
                        long f2 = fecha_chave(s, n, q);
                        if(cmd[0] == 'b'){
                            e->matriz_ant = e->exp;
                            e->matriz_par = par ? 5 : (colch ? 7 : 0);
                            e->exp = small ? 8 : -8; e->matriz = e->exp;
                            /* o delimitador é do VÃO: controlo 4/6, e o pintor
                             * centra-o no eixo da matriz */
                            if(par) empurra(e, 4, e->fonte);
                            if(colch) empurra(e, 6, e->fonte);
                        } else {
                            if(e->matriz_par){ empurra(e, e->matriz_par, e->fonte); e->matriz_par = 0; }
                            e->exp = e->matriz_ant; e->matriz = 0;
                        }
                        i = f2 > 0 ? f2 + 1 : j;
                        /* `\begin{array}{ccc}`: o preâmbulo come-se (alinhamento
                         * ainda é o do pintor — centrado por coluna) */
                        if(cmd[0] == 'b' && eh_array){
                            long qa = ate_abre(s, i, n);
                            long fa = fecha_chave(s, n, qa);
                            if(fa > 0) i = fa + 1;
                        }
                        continue;
                    }
                }
            }
            if(!strcmp(cmd, "begin") || !strcmp(cmd, "end")){
                /* O `abstract` E' UMA PAGINA PROPRIA, e o titulo vem do babel — como o
                 * «Capitulo». MEDIDO no gabarito: pagina 2 so' com o resumo, «Resumo»
                 * centrado a x=275,9, o texto na coluna INTEIRA (451pt, sem recuo), e o
                 * bloco centrado na vertical (centro 357,7 numa pagina que centra em 420,9). */
                { long q = j; q = ate_abre(s, q, n);
                if(q < n && !strncmp(s + q + 1, "abstract}", 9)){
                    /* le-se ANTES de fechar o paragrafo: o `fecha_paragrafo` pode descer o
                     * `y` e mascarar uma pagina que ainda esta' vazia */
                    int vazia = p->abriu_agora;
                    fecha_paragrafo(e);
                    if(!vazia) vazia = (p->y >= TOPO - PT);
                    if(cmd[0] == 'b'){
                        /* so' se abre pagina se a actual tiver alguma coisa: o `\maketitle`
                         * ja' abriu uma ao fechar a capa, e abrir outra deixava a 2 EM
                         * BRANCO com o resumo a cair na 3. */
                        if(!vazia){ pagina_fecha(p); pagina_abre(p); }
                        p->plana = 1;                 /* o resumo é página plain */
                        p->num = 1; p->sem_pe = 1;    /* o main reinicia aqui, pé escondido */
                        /* o titulo, centrado, no corpo do texto e a negro */
                        p->y -= 170*PT;
                        CENTRA = 1; e->fonte = F_NEG;
                        for(int t = 0; NOME_RESUMO[t]; t++)
                            empurra(e, (unsigned char)NOME_RESUMO[t], F_NEG);
                        quebra_e_desenrola(e, 1);
                        e->L.n = 0; CENTRA = 0; e->fonte = F_REG; e->nfp = 0;
                        p->y -= 10*PT;
                    } else if(!vazia){ pagina_fecha(p); pagina_abre(p); }
                    long f = fecha_chave(s, n, q);
                    i = f > 0 ? f + 1 : j; continue;
                }
                /* a BIBLIOGRAFIA: o \section*{Referências} do gabarito — o título vem
                 * do idioma (refname), o argumento {99} consome-se, e cada \bibitem
                 * rotula [n] pela ordem recolhida. Os espaços derivam do degrau da
                 * secção, não de um número posto. */
                if(q < n && !strncmp(s + q + 1, "thebibliography}", 16)){
                    fecha_paragrafo(e);
                    long f = fecha_chave(s, n, q);
                    if(cmd[0] == 'b'){
                        long q9 = (f > 0 && f + 1 < n) ? ate_abre(s, f + 1, n) : -1;
                        long f9 = (q9 > 0 && q9 < n) ? fecha_chave(s, n, q9) : -1;
                        long dgS = degrau_do_comando("section");
                        long cS = escala_corpo(dgS);
                        p->y -= cS;
                        if(NOME_REFS[0]){
                            long dg = DEG_FORCADO;
                            DEG_FORCADO = dgS;
                            e->fonte = (N_CARTA > 1) ? F_NEG : F_REG; e->nfp = 0;
                            for(int t2 = 0; NOME_REFS[t2]; t2++)
                                empurra(e, (unsigned char)NOME_REFS[t2], e->fonte);
                            quebra_e_desenrola(e, 0);
                            e->L.n = 0; DEG_FORCADO = dg;
                            e->fonte = F_REG;
                            p->y -= sem_resp(cS);
                        }
                        i = (f9 > 0) ? f9 + 1 : (f > 0 ? f + 1 : j);
                        continue;
                    }
                    i = f > 0 ? f + 1 : j; continue;
                }
                /* o `align`/`equation`/`gather` É matemática de destaque, como o \[ — sem
                 * esta porta o modo ficava desligado e os `&` e o \tag saíam LITERAIS
                 * (as duas leis da página 19, quebradas) */
                if(q < n && (!strncmp(s + q + 1, "align", 5) || !strncmp(s + q + 1, "equation", 8)
                          || !strncmp(s + q + 1, "gather", 6) || !strncmp(s + q + 1, "eqnarray", 8))){
                    /* O `aligned`/`gathered` é SUB-ambiente DENTRO da matemática: o pai
                     * (`\[` ou `align`) já ligou o modo. Tratá-lo como porta de display
                     * CLOBBERAVA o centra_mat do pai (guardava 1 por cima do 0), e no `\]`
                     * o CENTRA ficava preso: dali em diante TUDO centrava — prosa no meio
                     * da página e as células das tabelas UMAS SOBRE AS OUTRAS (o
                     * duplo-desenho que a auditoria de 14/08 apanhou: «sim»+«sim» no
                     * mesmo x, págs 422/423/425/434 do catálogo). O nome com `ed` só
                     * consome a chave e fica no modo do pai. */
                    if(!strncmp(s + q + 1, "aligned", 7) || !strncmp(s + q + 1, "gathered", 8)){
                        long f = fecha_chave(s, n, q);
                        i = f > 0 ? f + 1 : j; continue;
                    }
                    fecha_paragrafo(e);
                    if(cmd[0] == 'b'){ e->centra_mat = CENTRA; CENTRA = 1; e->mat = 1; e->disp = 1; mat_entra(e); }
                    else { CENTRA = e->centra_mat; e->mat = 0; e->disp = 0; e->exp = 0; e->exp1 = 0; e->nexp = 0; mat_sai(e); }
                    long f = fecha_chave(s, n, q);
                    i = f > 0 ? f + 1 : j; continue;
                }
                /* o `center` TAMBÉM se decide AQUI: o handler de baixo nunca corria — este
                 * despacho apanha todo `begin`/`end` primeiro, e o CENTRA ficava por pôr */
                if(q < n && !strncmp(s + q + 1, "center}", 7)){
                    fecha_paragrafo(e);
                    CENTRA = (cmd[0] == 'b');
                    long f = fecha_chave(s, n, q);
                    i = f > 0 ? f + 1 : j; continue;
                }
                }
                {   /* `\begin{minipage}{15.4cm}` traz um argumento a seguir ao nome, e ele
                     * saía como TEXTO: «15.4cm» solto antes do aviso legal, na primeira
                     * página. O handler que eu tinha posto mais abaixo nunca corria, porque
                     * este apanha o `begin` primeiro — dois handlers para o mesmo comando é
                     * o mesmo defeito dos dois laços do travessão. */
                    long q2 = j; while(q2 < n && s[q2] != '{') q2++;
                    if(q2 < n && !strncmp(s + q2 + 1, "minipage", 8)){
                        long f = fecha_chave(s, n, q2);
                        if(f > 0){ long g2 = f + 1;
                            while(g2 < n && s[g2] == '[') { while(g2 < n && s[g2] != ']') g2++; g2++; }
                            if(g2 < n && s[g2] == '{'){ long f2 = fecha_chave(s, n, g2);
                                                        if(f2 > 0){ i = f2 + 1; continue; } }
                            i = f + 1; continue; }
                    }
                }
                int abre = (cmd[0] == 'b');
                while(j < n && s[j] != '{') j++;
                long a = ++j; while(j < n && s[j] != '}') j++;
                char amb[64]; long ln = j - a; if(ln > 63) ln = 63;
                memcpy(amb, s + a, (size_t)ln); amb[ln] = 0;
                fecha_paragrafo(e);
                /* A CAIXA — o tcolorbox do catálogo é `boxrule=0pt, leftrule=2pt`: a moldura
                 * É a barra da esquerda, e mais nada. Guarda-se o y ao abrir e desenha-se ao
                 * fechar, quando já se sabe onde ela acaba.
                 *
                 * E desenha-se SÓ a barra, não o fundo: um stream de PDF é sequencial, e um
                 * fundo escrito depois do texto TAPA-O. A barra vive na margem, à esquerda de
                 * onde o texto cai, e por isso pode vir no fim. O fundo pede dois streams (o
                 * /Contents aceita um array) e fica nomeado, não escondido. */
                /* SÓ o tcolorbox tem caixa: o estilo declara `teorema`, `obs` e a família
                 * inteira por \newtheorem (amsthm) — o gabarito compõe-nos SEM fundo, e a
                 * caixa que eu lhes pintava caía 8pt por cima da linha de cima. */
                if(!strcmp(amb, "tcolorbox")){
                    if(abre){ e->p->caixa_y = e->p->y; }
                    else if(e->p->caixa_y > 0){
                        long alt = e->p->caixa_y - e->p->y;        /* na régua do Td */
                        if(alt > 0 && alt < 720000){             /* na mesma página */
                            /* o tcolorbox do catálogo: colback=ouroclaro!35, leftrule=2pt.
                             * São os DOIS — o fundo e a barra —, e agora os dois cabem,
                             * porque o fundo vai no primeiro stream e pinta por baixo. */
                            poe_rect(e->p, (MARGEM - 10) * 1000L, e->p->y + 2000, (COL + 14) * 1000L, alt + 6000, "ouroclaro");
                            poe_rect(e->p, (MARGEM - 10) * 1000L, e->p->y + 2000, 2000, alt + 6000, "ouro");
                        }
                        e->p->caixa_y = -1;
                    }
                }
                /* O VERBATIM E LITERAL — e foi aqui que eu perdi metade do catalogo.
                 * A linha 1532 do catalogo.tex e "$ MARTELO 2083236890 ..." dentro de um
                 * verbatim. O '$' ali e um cifrao de prompt, nao um delimitador de formula; mas
                 * eu tratava-o como delimitador, e como o numero deles era IMPAR o modo
                 * matematico ficava ligado ATE AO FIM DO DOCUMENTO. Dai em diante toda letra
                 * latina ia para a Symbol, e como o 'e' acentuado nao e isalpha() as palavras
                 * partiam-se em pedacos de fontes diferentes: 'lexico' saia como (l)(e)(xico),
                 * tres Tj distintos. O texto estava la e a palavra tinha deixado de existir.
                 *
                 * Um estado que so se LIGA e nunca se desliga sozinho apaga o que vem depois, e
                 * o dano nao aparece onde nasce — aparece 500 linhas adiante. */
                /* A TABELA ABRE: conta as colunas do preâmbulo e reparte a largura.
                 *
                 * O preâmbulo é `{@{}llrr@{}}` ou `{@{}p{0.3\textwidth}p{0.6\textwidth}@{}}`.
                 * Contam-se as letras de coluna — l, r, c, p — e ignora-se o resto: o `@{...}`
                 * é espaçamento e o `{...}` do `p` é a largura, que aqui se reparte por igual
                 * porque a coluna é POSIÇÃO e não conteúdo.
                 *
                 * E não é um caso especial: uma célula é uma linha que começa noutro x. O que
                 * faltava não era saber desenhar — era CONTAR AS COLUNAS. */
                if(!strcmp(amb, "tabular") || !strcmp(amb, "longtable")
                   || !strcmp(amb, "tabularx")
                   || (!strcmp(amb, "array") && !e->mat)){
                    if(abre){
                        fecha_paragrafo(e);
                        long q = j + 1;
                        q = ate_abre(s, q, n);   /* o preâmbulo */
                        int nc = 0, d2 = 0;
                        for(q++; q < n; q++){
                            if(s[q] == '{') d2++;
                            else if(s[q] == '}'){ if(!d2) break; d2--; }
                            else if(!d2 && (s[q]=='l'||s[q]=='r'||s[q]=='c'||s[q]=='p'||s[q]=='X')) nc++;
                        }
                        e->tab = 1;
                        e->tab_ncol = nc > 0 ? nc : 1;
                        e->tab_col = 0;
                        e->tab_x0 = MARGEM;
                        /* AS LARGURAS SÃO AS DO CONTEÚDO, como no LaTeX: mede-se cada
                         * célula até ao \end, a coluna é o MÁXIMO dos seus, mais o
                         * \tabcolsep dos dois lados. A repartição Fibonacci era escolha
                         * minha («e não derivada»), e o gabarito derruba-a — «a unidade é
                         * dual» quebrava onde o pdflatex dá à coluna a largura natural.
                         * Se o total não couber, reparte-se na proporção do conteúdo. */
                        int nfilas = 0;
                        {   long corpo_t = escala_corpo(D_TEXTO);
                            long max6[16]; for(int k = 0; k < 16; k++) max6[k] = 0;
                            long z = q + 1, ini2 = z; int col = 0, d4 = 0, mediu = 0;
                            while(z < n){
                                if(s[z] == '{') d4++;
                                else if(s[z] == '}'){ if(d4) d4--; }
                                else if(!d4 && s[z] == '\\' && !strncmp(s+z+1, "end{", 4)) break;
                                else if(!d4 && s[z] == '&'){
                                    long w = mede_celula(s, ini2, z, corpo_t);
                                    if(col < 16 && w > max6[col]) max6[col] = w;
                                    if(w > 0) mediu = 1;
                                    col++; ini2 = z + 1; }
                                else if(!d4 && s[z] == '\\' && s[z+1] == '\\'){
                                    long w = mede_celula(s, ini2, z, corpo_t);
                                    if(col < 16 && w > max6[col]) max6[col] = w;
                                    if(w > 0) mediu = 1;
                                    nfilas++;
                                    col = 0; z++; ini2 = z + 1; }
                                z++;
                            }
                            { long w = mede_celula(s, ini2, z < n ? z : n, corpo_t);
                              if(col < 16 && w > max6[col]) max6[col] = w; if(w > 0) mediu = 1; }
                            long tcs = (TABCOLSEP + 500) / 1000, tot = 0;
                            for(int k = 0; k < e->tab_ncol && k < 16; k++){
                                e->tab_w[k] = max6[k] / 1000000 + 1 + 2 * tcs;
                                tot += e->tab_w[k];
                            }
                            if(!mediu){ /* sem conteúdo mensurável: reparte por igual, e diz-se */
                                tot = 0;
                                for(int k = 0; k < e->tab_ncol && k < 16; k++){
                                    e->tab_w[k] = COL / e->tab_ncol; tot += e->tab_w[k]; }
                            }
                            if(tot > COL){
                                long t2 = 0;
                                for(int k = 0; k < e->tab_ncol && k < 16; k++){
                                    e->tab_w[k] = e->tab_w[k] * COL / tot; t2 += e->tab_w[k]; }
                                int ult = (e->tab_ncol <= 16 ? e->tab_ncol : 16) - 1;
                                e->tab_w[ult] += COL - t2;
                                tot = COL;
                            }
                            e->tab_tot = tot;
                            /* dentro de `center` a tabela centra-se como BLOCO, como lá */
                            e->tab_x0 = CENTRA ? MARGEM + (COL - tot) / 2 : MARGEM;
                            e->tab_larg = e->tab_w[0];
                        }
                        e->L.recuo = (int)(e->tab_x0 - MARGEM);
                        /* A TABELA É UMA REGIÃO (a régua do eval): mede-se a ALTURA antes
                         * de abrir — as filas mais as três réguas do booktabs. Se não
                         * couber no resto da página e couber numa inteira, vai INTEIRA
                         * para a seguinte: o toprule ficava órfão no fundo da página e o
                         * cabeçalho desnivelava na quebra. */
                        {   long alt_tab = (long)(nfilas + 4) * escala_entre(D_TEXTO);
                            if(e->p->y - alt_tab < FUNDO && alt_tab < TOPO - FUNDO
                               && !e->p->abriu_agora){
                                pagina_fecha(e->p); pagina_abre(e->p);
                            } }
                        /* O RECUO VEM ANTES do `tab_y`, e não depois. Guardando o topo da
                         * fila e só então recuando, a primeira fila nasce 4 pt acima de onde a
                         * tabela começa — e o cabeçalho caía por cima da régua de topo. */
                        e->p->y -= 4*PT;
                        e->tab_y = e->p->y;
                        e->tab_ymin = e->p->y;
                        e->tab_pag = e->p->npag;   /* senão o 1.º `&` via npag≠tab_pag
                                                   * e baixava tab_y — escada (diagonal) */
                        i = q + 1; continue;
                    } else {
                        fecha_paragrafo(e);
                        e->tab = 0; e->tab_col = 0;
                        e->L.recuo = e->recuo;     /* o texto a seguir não herda a coluna */
                        e->L.larg = 0;
                        e->p->y -= 4;
                        i = j; continue;
                    }
                }
                if(abre && (!strcmp(amb, "verbatim") || !strcmp(amb, "Verbatim")
                         || !strcmp(amb, "lstlisting") || !strcmp(amb, "minted"))){
                    char fim[80];
                    { char *q = ap_str(fim, "\\end{"); q = ap_str(q, amb); q = ap_str(q, "}"); *q = 0; }
                    const char *f = strstr(s + j, fim);
                    long ate = f ? (f - s) : n;
                    e->fonte = F_NEG; e->L.recuo = e->recuo + 12;
                    for(long q = j + 1; q < ate; ){
                        if(s[q] == '\n'){
                            fecha_paragrafo(e);
                            e->L.recuo = e->recuo + 12;
                            q++; continue;
                        }
                        int cons; int g = utf8_glifo((const unsigned char*)s + q, &cons);
                        empurra(e, g, F_NEG);          /* SEM interpretar: nem $, nem barra, nem chaves */
                        q += cons;
                    }
                    fecha_paragrafo(e);
                    e->fonte = F_REG; e->nfp = 0; e->L.recuo = e->recuo;
                    i = f ? ate + (long)strlen(amb) + 6 : n;
                    continue;
                }
                /* OS TEOREMAS (amsthm): o rótulo compõe-se como no pdflatex — «Nome CAP.N
                 * (opcional).» a negro, o contador UM para a família, preso ao capítulo */
                {   int tfeito = 0;
                    for(int t = 0; t < N_TEOR; t++) if(!strcmp(amb, TEOR[t].amb)){
                        if(!abre){ e->fonte = F_REG; e->nfp = 0; break; }   /* o corpo acabou: a fonte volta */
                        if(C_TEO_CAP != C_CAP){ C_TEO_CAP = C_CAP; C_TEO = 0; }
                        C_TEO = C_TEO + 1;
                        for(const char *z2 = TEOR[t].nome; *z2; z2++)
                            empurra(e, (unsigned char)*z2, F_NEG);
                        empurra(e, ' ', F_NEG);
                        { char nb[24]; char *pz = ap_num(nb, C_CAP); pz = ap_str(pz, ".");
                          pz = ap_num(pz, C_TEO); *pz = 0;
                          for(int k2 = 0; nb[k2]; k2++) empurra(e, (unsigned char)nb[k2], F_NEG); }
                        long q2 = j + 1;
                        if(q2 < n && s[q2] == '['){       /* o nome do teorema, entre parênteses */
                            empurra(e, ' ', F_REG); empurra(e, '(', F_REG);
                            q2++;
                            while(q2 < n && s[q2] != ']'){
                                /* o título não é texto cru: o \emph{...} compunha
                                 * LITERAL — «é \emph{origem}» na página. Itálica
                                 * para o emph, léxico para os símbolos. */
                                if(s[q2] == '\\'){
                                    long j3 = q2 + 1;
                                    if(j3 < n && (s[j3] == '\'' || s[j3] == '~' || s[j3] == '^')){
                                        char ac = s[j3]; long k = j3 + 1; int abriu = 0;
                                        if(k < n && s[k] == '{'){ abriu = 1; k++; }
                                        char v = 0;
                                        if(k < n){
                                            if(s[k] == '\\' && k + 1 < n && s[k+1] == 'i'){ v = 'i'; k += 2; }
                                            else if(s[k] != '}' && s[k] != ']'){ v = s[k]; k++; }
                                        }
                                        if(abriu){ while(k < n && s[k] != '}' && s[k] != ']') k++;
                                                   if(k < n && s[k] == '}') k++; }
                                        unsigned char comp = 0;
                                        if(v){ const char *pv = strchr(AC_VOG, v);
                                if(pv){ int tv = (int)(pv - AC_VOG); unsigned char cv = 0;
                                    if(ac == '\x27') cv = AC_AGU[tv];
                                    else if(ac == '~') cv = AC_TIL[tv];
                                    else if(ac == '^') cv = AC_CIR[tv];
                                    if(cv) comp = cv; } }
                                        if(comp) empurra(e, comp, F_REG);
                                        else {
                                            if(ac == '~') empurra(e, 0x7E, F_SIM);
                                            else if(ac == '\'') empurra(e, 0xB4, F_REG);
                                            else empurra(e, 0x88, F_REG);
                                            if(v) empurra(e, (unsigned char)v, F_REG);
                                        }
                                        q2 = k; continue;
                                    }
                                    char c3[24]; int k3 = 0;
                                    while(j3 < n && isalpha((unsigned char)s[j3]) && k3 < 23)
                                        c3[k3++] = s[j3++];
                                    c3[k3] = 0;
                                    if(!strcmp(c3, "emph") || !strcmp(c3, "textit")){
                                        long qa = ate_abre(s, j3, n), fa = fecha_chave(s, n, qa);
                                        if(fa > 0){
                                            for(long z3 = qa + 1; z3 < fa; z3++){
                                                int cs3; int g3 = utf8_glifo((const unsigned char*)s + z3, &cs3);
                                                empurra(e, g3, (N_CARTA > F_ITA) ? F_ITA : F_REG);
                                                z3 += (cs3 ? cs3 : 1) - 1;
                                            }
                                            q2 = fa + 1; continue;
                                        }
                                        q2 = j3; continue;
                                    }
                                    /* \mathbb{R}, \mathrm{...} no título: come o comando e
                                     * compõe o argumento (R, texto) — senão saía «mathbbR» */
                                    if(!strcmp(c3, "mathbb") || !strcmp(c3, "mathrm")
                                       || !strcmp(c3, "mathit") || !strcmp(c3, "mathbf")
                                       || !strcmp(c3, "text")){
                                        long qa = ate_abre(s, j3, n), fa = fecha_chave(s, n, qa);
                                        if(fa > 0){
                                            int fm = (!strcmp(c3, "mathit") && N_CARTA > F_ITA)
                                                   ? F_ITA : F_REG;
                                            for(long z3 = qa + 1; z3 < fa; z3++){
                                                if(s[z3] == '$') continue;
                                                int cs3; int g3 = utf8_glifo((const unsigned char*)s + z3, &cs3);
                                                empurra(e, g3, fm);
                                                z3 += (cs3 ? cs3 : 1) - 1;
                                            }
                                            q2 = fa + 1; continue;
                                        }
                                        q2 = j3; continue;
                                    }
                                    { const Par *P3 = lex_acha(c3);
                                      if(P3) empurra(e, P3->glifo, P3->simb ? F_SIM : F_REG); }
                                    q2 = j3; continue;
                                }
                                /* `$...$` no título opcional: o pdflatex entra em matemática;
                                 * pintar o `$` dava «max-cut $=$ massa» literal (thm 0.12). */
                                if(s[q2] == '$'){ q2++; continue; }
                                /* o `~` é espaço não-quebrável — «Hurwitz~8», não til alto */
                                if(s[q2] == '~'){ empurra(e, ' ', F_REG); q2++; continue; }
                                /* ^{a} / ^a: come o circumflexo e pinta o expoente (sem `$`) */
                                if(s[q2] == '^'){
                                    long j3 = q2 + 1;
                                    if(j3 < n && s[j3] == '{'){
                                        long fa = fecha_chave(s, n, j3);
                                        if(fa > 0){
                                            for(long z3 = j3 + 1; z3 < fa; z3++){
                                                if(s[z3] == '$') continue;
                                                int cs3; int g3 = utf8_glifo((const unsigned char*)s + z3, &cs3);
                                                empurra(e, g3, F_REG);
                                                z3 += (cs3 ? cs3 : 1) - 1;
                                            }
                                            q2 = fa + 1; continue;
                                        }
                                    } else if(j3 < n){
                                        int cs3; int g3 = utf8_glifo((const unsigned char*)s + j3, &cs3);
                                        empurra(e, g3, F_REG);
                                        q2 = j3 + (cs3 ? cs3 : 1); continue;
                                    }
                                }
                                int cs2; int g2 = utf8_glifo((const unsigned char*)s + q2, &cs2);
                                empurra(e, g2, F_REG); q2 += cs2 ? cs2 : 1;
                            }
                            empurra(e, ')', F_REG);
                            if(q2 < n) q2++;
                        }
                        empurra(e, '.', F_NEG); empurra(e, ' ', F_REG);
                        /* o corpo segue no estilo da família: `plain` é itálico */
                        if(TEOR[t].ita && N_CARTA > F_ITA) e->fonte = F_ITA;
                        i = q2; tfeito = 1; break;
                    }
                    if(tfeito) continue;
                }
                if(!strcmp(amb, "itemize") || !strcmp(amb, "enumerate") || !strcmp(amb, "description"))
                    e->recuo = abre ? e->recuo + 18 : (e->recuo >= 18 ? e->recuo - 18 : 0);
                if(!strcmp(amb, "document") && !abre) break;
                e->L.recuo = e->recuo;
                i = j + 1;
                /* o opcional do amsthm — `\begin{teorema}[nome]` — consome-se: saía como
                 * texto «[a matriz é a codificação do tempo]» na página */
                if(abre && i < n && s[i] == '['){ while(i < n && s[i] != ']') i++; if(i < n) i++; }
                continue;
            }
            /* ─── AS FOLHAS: onde a avaliação pára ────────────────────────────────
             * A avaliação nas raízes tem de aterrar em algo, e o `universal.c` diz onde:
             * §U4, as FOLHAS DE FROBENIUS — os fixos, o subcorpo primo. Aqui são as
             * primitivas: `\fontsize`, `\color`, `\rule`. Uma macro expande até dar nelas,
             * e elas não expandem mais.
             *
             * Sem as tratar, a expansão da capa despejava os argumentos como texto:
             * «23.4233.95tinta Reino Dourado», «ouro15.4cm1.4pt». O número 23,42 não é
             * lixo — É O DEGRAU DA DOURADA, escrito pelo autor no estilo.tex. Só faltava
             * ligá-lo à escala que este ficheiro já lia da mesma fonte. */
            /* `\setcounter{chapter}{0}` — MAIS UMA FOLHA, e uma que não se adivinha:
             * o enredo.tex repõe o capítulo na linha 5625, à mão, e o pdflatex obedece.
             * Sem isto os meus capítulos iam 1..148 enquanto o original vai 1..54 e depois
             * 1..94 — MEDIDO: numerar sem esta primitiva punha 73 números errados na
             * página, o que é pior do que não numerar. */
            /* UMA DECLARAÇÃO NÃO É TEXTO. O corpo do `\gkcapa` traz dois
             * `\providecommand` aninhados (o `\Prj` e o `\Trf`), e a expansão emitia-os
             * como conteúdo: `\mathbb{P}` virava `P`, `\mathcal{T}` virava `T`, e a capa
             * abria com «PP TT». Consomem-se os dois argumentos e não sai nada. */
            if(!strcmp(cmd, "begin") || !strcmp(cmd, "end")){
                long q = j; q = ate_abre(s, q, n);
                if(q < n && !strncmp(s + q + 1, "center}", 7)){
                    fecha_paragrafo(e);
                    CENTRA = (cmd[0] == 'b');
                    long f = fecha_chave(s, n, q);
                    i = f > 0 ? f + 1 : j; continue;
                }
                /* `\begin{minipage}{15.4cm}` traz um argumento a seguir ao nome, e ele saía
                 * como texto: «15.4cm» solto antes do aviso legal da capa. */
                if(q < n && cmd[0] == 'b' && !strncmp(s + q + 1, "minipage", 8)){
                    long f = fecha_chave(s, n, q);
                    if(f > 0){ long g2 = f + 1;
                        while(g2 < n && (s[g2]=='[' )){ while(g2 < n && s[g2] != ']') g2++; g2++; }
                        if(g2 < n && s[g2] == '{'){ long f2 = fecha_chave(s, n, g2);
                                                    if(f2 > 0){ i = f2 + 1; continue; } }
                        i = f + 1; continue; }
                }
            }
            if(!strcmp(cmd, "providecommand") || !strcmp(cmd, "newcommand") ||
               !strcmp(cmd, "renewcommand") || !strcmp(cmd, "definecolor") ||
               !strcmp(cmd, "setlength") || !strcmp(cmd, "hyphenation") ||
               !strcmp(cmd, "label")){
                long q = j;
                int nar = (!strcmp(cmd, "hyphenation") || !strcmp(cmd, "label")) ? 1 : 2;
                if(!strcmp(cmd, "definecolor")) nar = 3;
                for(int t = 0; t < nar && q < n; t++){
                    while(q < n && (s[q] == ' ' || s[q] == '\t' || s[q] == '\n')) q++;
                    /* `\setlength\tabcolsep{5pt}`: o primeiro argumento é uma SEQUÊNCIA DE
                     * CONTROLO, sem chavetas. Procurar o `{` saltava por cima dela e comia
                     * o `{tabular}` do `\begin` seguinte — o preâmbulo `@llll@` vazava como
                     * texto e a tabela do resumo nunca abria. O `\nome` conta como o
                     * argumento, como no minipage. */
                    if(q < n && s[q] == '\\'){
                        long q0 = ++q;
                        while(q < n && isalpha((unsigned char)s[q])) q++;
                        /* o comprimento CONHECIDO grava-se: o \tabcolsep é régua da tabela */
                        if(q - q0 == 9 && !strncmp(s + q0, "tabcolsep", 9)){
                            long qq = q; while(qq < n && (s[qq] == ' ' || s[qq] == '\t')) qq++;
                            if(qq < n && s[qq] == '{'){
                                long v = medida_mil(s + qq + 1);
                                if(v > 0) TABCOLSEP = v;
                            }
                        }
                        continue;
                    }
                    while(q < n && s[q] != '{' && s[q] != '[') q++;
                    if(q < n && s[q] == '['){ while(q < n && s[q] != ']') q++; q++; t--; continue; }
                    long f = fecha_chave(s, n, q); if(f < 0) break; q = f + 1;
                }
                i = q; continue;
            }
            /* `\rule{15.4cm}{0pt}` de espessura ZERO é um espaçador, não uma régua — e os
             * seus argumentos saíam como texto: «15.4cm0pt» antes do aviso legal. */
            /* `\ifSubfilesClassLoaded{A}{B}` — compilado SOZINHO usa A, dentro do livro
             * usa B. Este tradutor compila o ficheiro sozinho, logo A. Eu compunha OS DOIS,
             * e por isso a pagina 2 saia com o `\part*{O Enredo}` do ramo que nao e' o
             * nosso, empurrando o resumo para a 3. */
            if(!strcmp(cmd, "ifSubfilesClassLoaded")){
                long q = j; q = ate_abre(s, q, n);
                long f1 = fecha_chave(s, n, q);
                if(f1 > 0){
                    long q2 = f1 + 1;
                    while(q2 < n && (s[q2]==' '||s[q2]=='\n'||s[q2]=='\t'||s[q2]=='%')){
                        if(s[q2]=='%'){ while(q2 < n && s[q2] != '\n') q2++; } else q2++;
                    }
                    long f2 = (q2 < n && s[q2]=='{') ? fecha_chave(s, n, q2) : -1;
                    /* compoe-se o PRIMEIRO e salta-se o segundo: a chaveta que abre o
                     * primeiro fica, e o `}` dela fecha normalmente. Marca-se onde o
                     * segundo comeca para o consumir quando la' chegarmos. */
                    if(f2 > 0){ SALTA_DE = q2; SALTA_ATE = f2 + 1; }
                    i = q + 1; continue;
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "setcounter") || !strcmp(cmd, "addtocounter")){
                long q = j; char nc[32]; int kc = 0;
                q = ate_abre(s, q, n);
                for(long t = q + 1; t < n && s[t] != '}' && kc < 31; t++) nc[kc++] = s[t];
                nc[kc] = 0;
                long f1 = fecha_chave(s, n, q);
                long v = 0; int tem = 0;
                if(f1 > 0 && f1 + 1 < n && s[f1+1] == '{'){ v = atol(s + f1 + 2); tem = 1;
                    long f2 = fecha_chave(s, n, f1 + 1); if(f2 > 0) q = f2 + 1; else q = f1 + 1; }
                else if(f1 > 0) q = f1 + 1;
                if(tem){
                    int add = (cmd[0] == 'a');
                    long *alvo_c = !strcmp(nc,"chapter")    ? &C_CAP
                                 : !strcmp(nc,"part")       ? &C_PARTE
                                 : !strcmp(nc,"section")    ? &C_SEC
                                 : !strcmp(nc,"subsection") ? &C_SUB : NULL;
                    if(alvo_c){
                        *alvo_c = add ? *alvo_c + v : v;
                        /* repor um nível repõe os de baixo, como o LaTeX faz */
                        if(alvo_c == &C_CAP){ C_SEC = 0; C_SUB = 0; C_SSUB = 0; }
                        else if(alvo_c == &C_SEC){ C_SUB = 0; C_SSUB = 0; }
                    }
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "fontsize")){
                long q = j;
                q = ate_abre(s, q, n);
                /* só c1 (o corpo) se usa --- o sscanf "%lf}{%lf}" pedia >=1, basta c1 parseado */
                const char *pf = s + q + 1, *ef;
                long c1 = fixo_mil(pf, &ef);
                if(ef != pf && c1 > 0){
                    /* o degrau é o da escala mais perto — e nunca uma medida nova: se o
                     * tamanho não estiver na escala, é a escala que decide, não este `if` */
                    long melhor = -1, dmin = 1L << 60;
                    for(long t = 0; t < N_ESCALA; t++){
                        long d = ESCALA[t].corpo - c1; if(d < 0) d = -d;
                        if(d < dmin){ dmin = d; melhor = t; }
                    }
                    if(melhor >= 0){ DEG_FORCADO = melhor; DEG_PROF = PROF; }
                }
                /* consomem-se os dois argumentos */
                for(int t = 0; t < 2 && q < n; t++){
                    q = ate_abre(s, q, n);
                    long f = fecha_chave(s, n, q); if(f < 0) break; q = f + 1;
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "selectfont") || !strcmp(cmd, "normalsize")){
                if(cmd[0] == 'n'){ DEG_FORCADO = -1; DEG_PROF = -1; }
                i = j; continue;
            }
            /* `\color{ouro}` PINTA, não se deita fora. O corpo do `\gkcapa` declara três
             * cores --- `tinta` no «Reino Dourado», `ouro` no título, `regua` no subtítulo
             * e na epígrafe --- e eu consumia-as todas: a capa saía preta onde o pdflatex a
             * tem dourada. E `\color` sem grupo vale até ao fim do grupo que o contém. */
            if(!strcmp(cmd, "color") || !strcmp(cmd, "textcolor")){
                long q = j; q = ate_abre(s, q, n);
                if(q < n){
                    long f = fecha_chave(s, n, q);
                    if(f > 0){
                        long ln = f - q - 1; if(ln > 23) ln = 23;
                        memcpy(COR_TEXTO, s + q + 1, (size_t)ln); COR_TEXTO[ln] = 0;
                        COR_PROF = PROF;
                        i = f + 1; continue;
                    }
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "morfh")){
                /* Teorema Morfológico (thm:morfologico): coordenada W — Lei~4.
                 * Dilata o parágrafo à largura do suporte [0,W]. */
                e->morf_h = 1;
                i = j; continue;
            }
            if(!strcmp(cmd, "ClaveG") || !strcmp(cmd, "ClaveF") || !strcmp(cmd, "ClaveP")){
                /* Claves reais: glifos especiais 0x82/83/84 pintados em desenrola
                 * na mesma baseline das notas (não no y do título). */
                int gclef = (cmd[5] == 'F') ? 0x83 : (cmd[5] == 'P') ? 0x84 : 0x82;
                empurra(e, gclef, e->fonte);
                i = j; continue;
            }
            if(!strcmp(cmd, "estrela")){
                /* Interface Estrela (Def. orquestra): valida a coordenada H.
                 * Sem isto, \morfv não dilata — Ind^8 exige a estrela. */
                e->estrela = 1;
                i = j; continue;
            }
            if(!strcmp(cmd, "interfacen")){
                /* Declara o ciclo hexal no /SementeEstrela: 6, 12, 24…
                 * Sobrescreve o derivado do alcance da espiral — o paper
                 * diz em que ciclo da família vive (Lei 6 computacional). */
                long q = ate_abre(s, j, n);
                if(q < n && s[q] == '{'){
                    const char *pr = s + q + 1;
                    char *ep = NULL;
                    long v = strtol(pr, &ep, 10);
                    if(ep != pr && v >= 6 && v <= 384) INTERFACE_DECL = (int)v;
                    long f = fecha_chave(s, n, q);
                    if(f >= 0) q = f + 1;
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "morfv")){
                /* Coordenada H (thm:morfologico): Ind^8 / Estrela. Sem estrela,
                 * a altura não é válida — não dilata. */
                fecha_paragrafo(e);
                long q = ate_abre(s, j, n);
                int nv = 0;
                if(q < n && s[q] == '{'){
                    const char *pr = s + q + 1, *e1;
                    long vv = fixo_mil(pr, &e1);
                    long f = fecha_chave(s, n, q);
                    if(f >= 0) q = f + 1;
                    if(e1 != pr && vv > 0) nv = (int)(vv / 1000);
                }
                if(e->estrela && nv > 0){
                    e->morf_v = 1; e->morf_vn = nv; e->morf_vi = 0;
                    e->morf_vy0 = e->p->y;
                } else {
                    e->morf_v = 0; e->morf_vn = 0; e->morf_vi = 0;
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "endmorfv")){
                fecha_paragrafo(e);
                e->morf_v = 0; e->morf_vn = 0; e->morf_vi = 0;
                i = j; continue;
            }
            if(!strcmp(cmd, "morfvstep")){
                /* Coordenada H (Ind^8/Estrela): dilata vãos entre vozes em [0,H].
                 * Reserva h_min por voz para o corpo não cair sob FUNDO. */
                fecha_paragrafo(e);
                if(e->morf_v && e->estrela && e->morf_vn > 0){
                    if(e->morf_vi == 0 && e->p->y < e->morf_vy0)
                        e->morf_vy0 = e->p->y;   /* topo real após título */
                    long h_min = 48 * PT;   /* ~ footprint de uma voz pentagrama */
                    long Hsup = e->morf_vy0 - FUNDO;
                    if(Hsup < h_min) Hsup = h_min;
                    long n = e->morf_vn;
                    long passo = (n > 1) ? (Hsup - h_min) / (n - 1) : 0;
                    if(passo < 0) passo = 0;
                    long alvo = e->morf_vy0 - e->morf_vi * passo;
                    if(alvo < FUNDO + h_min) alvo = FUNDO + h_min;
                    if(e->p->y > alvo) e->p->y = alvo;
                    if(e->morf_vi < e->morf_vn) e->morf_vi++;
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "reguacol")){
                /* Régua dourada na largura COL (= suporte W). Sem cm mágico: a
                 * morfologia e a pauta partilham o mesmo [0,W]. */
                fecha_paragrafo(e);
                long q = ate_abre(s, j, n);
                long a2 = 320; char u2[8]; u2[0] = 'p'; u2[1] = 't'; u2[2] = 0;
                if(q < n && s[q] == '{'){
                    const char *pr = s + q + 1, *e1;
                    long v2 = fixo_mil(pr, &e1);
                    long f = fecha_chave(s, n, q);
                    if(f >= 0) q = f + 1;
                    if(e1 != pr && v2 > 0){
                        int k2 = 0; while(k2 < 2 && e1[k2] >= 'a' && e1[k2] <= 'z'){ u2[k2] = e1[k2]; k2++; }
                        u2[k2] = 0; a2 = v2;
                    }
                }
                { long n2, d2u; unidade_razao(u2, &n2, &d2u);
                  long l2 = (2 * a2 * n2 + d2u) / (2 * d2u);
                  if(l2 < 1) l2 = 320;
                  poe_rect(e->p, MARGEM * 1000L, e->p->y, COL * 1000L, l2, "ouro");
                  e->p->y -= l2 + 4000;
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "pipagina")){
                /* π_página: a página é dimensão do relógio (eval). O Maestro projecta o
                 * bloco para P_{n+1} se não couber — mesma régua da tabela-região.
                 * P_n → P_{n+1} muda o suporte, não o estado (relógio, voz, Π, linha).
                 * Sob \morfv (coordenada H), o bloco já foi projectado: não partir. */
                fecha_paragrafo(e);
                long q = ate_abre(s, j, n);
                if(q < n && s[q] == '{'){
                    const char *pr = s + q + 1, *e1;
                    long vv = fixo_mil(pr, &e1);
                    long f = fecha_chave(s, n, q);
                    if(f >= 0) q = f + 1;
                    if(!e->morf_v && e1 != pr && vv > 0){
                        char uu[8]; int ku = 0;
                        while(ku < 2 && e1[ku] >= 'a' && e1[ku] <= 'z'){ uu[ku] = e1[ku]; ku++; }
                        uu[ku] = 0;
                        long nu, du; unidade_razao(uu, &nu, &du);
                        long alt = (2 * vv * nu + du) / (2 * du);
                        if(e->p->y - alt < FUNDO && alt < TOPO - FUNDO
                           && !e->p->abriu_agora){
                            pagina_fecha(e->p); pagina_abre(e->p);
                        }
                    }
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "rule") ||
               !strcmp(cmd, "vspace") || !strcmp(cmd, "hspace")){
                /* `\rule{larg}{esp}` é uma RÉGUA. `\vspace{±}` move y (involução vertical:
                 * positivo desce; negativo sobe — ancla a nota no cone/pentagrama). */
                long q = j;
                long a1 = 0, a2 = 0; char u1[8]; char u2[8]; u1[0] = 0; u2[0] = 0;
                int nar = (cmd[0] == 'r') ? 2 : 1;
                long ini = q;
                q = ate_abre(s, q, n);
                if(q < n && cmd[0] == 'r'){
                    /* "a1u1}{a2u2}" --- dois (mantissa+unidade) com o }{ no meio: fixo_mil + até 2 minúsculas */
                    const char *pr = s + q + 1, *e1;
                    long v1 = fixo_mil(pr, &e1);
                    if(e1 != pr){
                        a1 = v1;
                        int k = 0; while(k < 2 && e1[k] >= 'a' && e1[k] <= 'z'){ u1[k] = e1[k]; k++; } u1[k] = 0;
                        const char *af = e1 + k;
                        if(af[0] == '}' && af[1] == '{'){
                            const char *pr2 = af + 2, *e2;
                            long v2 = fixo_mil(pr2, &e2);
                            if(e2 != pr2){
                                a2 = v2;
                                int k2 = 0; while(k2 < 2 && e2[k2] >= 'a' && e2[k2] <= 'z'){ u2[k2] = e2[k2]; k2++; } u2[k2] = 0;
                            }
                        }
                    }
                }
                if(cmd[0] == 'v' && q < n && s[q] == '{'){
                    /* \vspace{±Nmm}: y -= m (m<0 sobe — notas no cone/pera).
                     * Parse local (sem função nova: MAX_FUN no wasm). */
                    const char *pr = s + q + 1, *e1;
                    long vv = fixo_mil(pr, &e1);
                    long f = fecha_chave(s, n, q);
                    if(f >= 0) q = f + 1;
                    if(e1 != pr){
                        char uu[8]; int ku = 0;
                        while(ku < 2 && e1[ku] >= 'a' && e1[ku] <= 'z'){ uu[ku] = e1[ku]; ku++; }
                        uu[ku] = 0;
                        long nu, du; unidade_razao(uu, &nu, &du);
                        long mag = vv < 0 ? -vv : vv;
                        long m = (2 * mag * nu + du) / (2 * du);
                        if(vv < 0) m = -m;
                        fecha_paragrafo(e); e->p->y -= m;
                    }
                    i = q; continue;
                }
                for(int t = 0; t < nar && q < n; t++){
                    q = ate_abre(s, q, n);
                    long f = fecha_chave(s, n, q); if(f < 0){ q = ini; break; } q = f + 1;
                }
                if(cmd[0] == 'r' && a1 > 0){
                    long n1, d1, n2, d2u;
                    unidade_razao(u1, &n1, &d1);
                    long l1 = (2 * a1 * n1 + d1) / (2 * d1);
                    if(a2 > 0){
                        /* régua visível */
                        unidade_razao(u2, &n2, &d2u);
                        long l2 = (2 * a2 * n2 + d2u) / (2 * d2u);
                        fecha_paragrafo(e);
                        poe_rect(p, MARGEM * 1000L, e->p->y, l1, l2, "ouro");
                        e->p->y -= l2 + 4000;
                    } else {
                        /* \rule{W}{0pt} = strut horizontal (partitura \E):
                         * avança X sem tinta — senão as lanes diagonizam. */
                        long acc = 0;
                        int fm = (CARTA_MAT ? F_MAT : e->fonte);
                        while(acc + 5000 < l1 && e->L.n < MAXLIN - 2){
                            empurra(e, 0xA0, fm);   /* em-quad */
                            acc += 10000;
                        }
                        while(acc < l1 && e->L.n < MAXLIN - 2){
                            empurra(e, ' ', e->fonte);
                            acc += 3330;
                        }
                    }
                }
                i = q; continue;
            }
            /* `\title{...}` é a CAPA, e o `\maketitle` centra-a e dá-lhe página própria.
             * Sem isto ela saía encostada à margem esquerda e colada ao texto do capítulo
             * seguinte — que é a primeira coisa que se vê ao abrir o documento. */
            if(!strcmp(cmd, "title")){
                fecha_paragrafo(e);
                CENTRA = 1;
                p->plana = 1;                  /* a capa é página sem cabeçalho */
                p->sem_pe = 1; p->num = 0;     /* e sem pé — e fora da conta */
                /* E CENTRA NOS DOIS EIXOS. A capa do gabarito tem o centro em y=392,3 numa
                 * página de 841,9 --- centrada. É a mesma involução que já faz o centrar
                 * horizontal, no outro eixo: `y ↦ H − y`, com ponto fixo em H/2.
                 *
                 * A altura não se adivinha: SOMA-SE do que o corpo declara --- a entrelinha
                 * de cada degrau `\gk*` que ele usa, mais os `\\[Xmm]` que ele pede. Os dois
                 * números vêm do estilo, nenhum daqui. */
                /* A ALTURA MEDE-SE, NAO SE ESTIMA. Compoe-se a capa uma vez, le-se quanto
                 * desceu, rebobina-se o ficheiro e compoe-se outra vez na posicao certa.
                 * A soma exacta e' a que o proprio compositor faz — cada caractere com a
                 * sua caixa, cada linha com a sua entrelinha — e nenhuma conta minha a
                 * repete melhor do que ele. */
                if(CAPA_ALT <= 0){
                    CAPA_POS = s_pos(&p->sf);
                    CAPA_FUN = p->fundo_on ? s_pos(&p->sfundo) : 0;
                    CAPA_NF  = p->n_fundo;
                    CAPA_I   = i;
                    CAPA_Y   = p->y;
                    CAPA_PAG = p->npag;
                }
                { long h = 0; long q = j; int d3 = 0;   /* a altura estimada, régua do Td */
                  while(q < n){
                      if(s[q] == '{') d3++;
                      else if(s[q] == '}'){ d3 = d3 - 1; if(d3 <= 0) break; }
                      else if(s[q] == '\\' && q + 2 < n){
                          if(!strncmp(s+q+1, "gk", 2)){
                              /* e QUANTAS LINHAS este bloco ocupa: um bloco que não cabe na
                               * coluna gasta várias entrelinhas, e contar UMA por bloco dava
                               * 239,6 onde a capa mede 254 --- o aviso legal sozinho quebra
                               * em quatro. Mede-se o texto até ao fim do grupo. */
                              /* o degrau que este nome usa: a sua entrelinha entra na conta */
                              char gk[24]; int k2 = 0; const char *z = s + q + 1;
                              while(*z && isalpha((unsigned char)*z) && k2 < 23) gk[k2++] = *z++;
                              gk[k2] = 0;
                              char alvo[64]; char *ka = ap_str(alvo, "{\\"); ka = ap_str(ka, gk); ka = ap_str(ka, "}{\\fontsize{"); *ka = 0;
                              const char *est = estilo_texto(NULL);   /* o slot cacheado, sem le_tudo/malloc */
                              if(est){ const char *d4 = strstr(est, alvo);
                                       long c1 = 0, c2 = 0;
                                       int _ok = 0;
                                       if(d4){ const char *pc = d4 + strlen(alvo), *e1;   /* "N}{N" por fixo_mil, exacto */
                                               long v1 = fixo_mil(pc, &e1);
                                               if(e1 != pc && e1[0] == '}' && e1[1] == '{'){
                                                   const char *e2; long v2 = fixo_mil(e1 + 2, &e2);
                                                   if(e2 != e1 + 2){ c1 = v1; c2 = v2; _ok = 1; } } }
                                       if(_ok){
                                           /* o texto deste bloco: do fim do nome até fechar o grupo */
                                           /* CADA CARACTERE COM A SUA CAIXA, e medida no
                                            * desenho DESTE corpo — sem isto somam-se as
                                            * caixas de 10pt para um bloco de 23,42, e a
                                            * altura sai curta: 239,6 onde a capa mede 254. */
                                           CORPO_CORRENTE = c1;
                                           long t = q + 1 + k2; int dd = 1; long largo = 0;
                                           while(t < n && dd){
                                               if(s[t]=='{') dd++;
                                               else if(s[t]=='}'){ if(!--dd) break; }
                                               else if((unsigned char)s[t] >= 32 && s[t] != '\\')
                                                   largo += largura((unsigned char)s[t], 0);
                                               t++;
                                           }
                                           /* largo (por-mil) × c1 (mantissa 10^-3) é exacto em
                                            * 10^-6; UMA divisão leva-o a pontos, a régua da coluna */
                                           long linhas = 1 + (largo * c1 / 1000000L) / (COL > 0 ? COL : 1);
                                           h += c2 * linhas;
                                       }
                                       }
                          } else if(s[q+1] == '\\' && s[q+2] == '['){
                              long m = medida_mil(s + q + 3);
                              if(m >= 0) h += m;
                          }
                      }
                      q++;
                  }
                  /* A ESTIMATIVA ERRA, E MEDI-LA CUSTA UMA PASSAGEM. Somar as entrelinhas
                   * declaradas dá 239,6 onde a capa mede 275,1 --- faltam os blocos que
                   * quebram em duas linhas, e esses só se sabem compondo. Guarda-se o `y` de
                   * partida e, no `\maketitle`, a altura real é a diferença: a segunda
                   * passagem sobre a MESMA capa já sabe onde a pôr.
                   *
                   * Enquanto a segunda passagem não existir, usa-se a estimativa e diz-se
                   * quanto ela erra --- que é o que a medida acima faz. */
                  /* na segunda passagem usa-se a altura MEDIDA; na primeira, a estimada,
                   * que so' serve para a capa nao comecar no topo enquanto se mede */
                  long alt = CAPA_ALT > 0 ? CAPA_ALT : h;
                  if(alt > 0 && alt < A4_AM){
                      long topo_certo = (A4_AM + alt) / 2;
                      if(p->y > topo_certo) p->y = topo_certo;
                  }
                  Y_CAPA = p->y; }
                i = j; continue;
            }
            /* O SUMÁRIO compõe a tabela que a passagem anterior encheu: cada entrada com
             * o seu rótulo, o texto e o número da página, e o recuo pelo nível. Na primeira
             * passagem a tabela está vazia e não sai nada — é isso que faz a paginação
             * mudar entre as duas, e por isso se corre até estabilizar. */
            if(!strcmp(cmd, "tableofcontents") && TOC_LE && N_TOC > 0){
                fecha_paragrafo(e);
                if(p->y < TOPO - PT && !p->abriu_agora){ pagina_fecha(p); pagina_abre(p); }
                p->plana = 1;                          /* a 1.a página do sumário é plain */
                p->num = 1;                            /* a série do sumário começa aqui */
                { char *qz = ap_str(CAB_DIR, NOME_SUMARIO); *qz = 0; }   /* a marca */
                /* o título, no degrau do capítulo */
                e->L.deg = degrau_do_comando("chapter");
                e->fonte = F_NEG;
                for(int t = 0; NOME_SUMARIO[t]; t++)
                    empurra(e, (unsigned char)NOME_SUMARIO[t], F_NEG);
                quebra_e_desenrola(e, 1);
                e->L.n = 0; e->L.deg = -1; e->fonte = F_REG; e->nfp = 0;
                p->y -= 24*PT;
                for(int t = 0; t < N_TOC; t++){
                    Toc *q2 = &TOC[t];
                    e->L.recuo = (q2->nivel - 1) * 14;
                    if(q2->nivel == 1) p->y -= 5*PT;
                    e->fonte = q2->nivel == 1 ? F_NEG : F_REG;
                    /* O TEXTO E' UTF-8, e empurra-se GLIFO a glifo — nao byte a byte. Os
                     * acentos tem dois bytes, e empurra-los sozinhos dava «IntroduÃ§Ã£o». */
                    /* o ROTULO ja' vem em WinAnsi — e' o que o `tex_para_winansi` produziu
                     * ao ler o babel. So' o TEXTO do titulo e' que vem do fonte em UTF-8.
                     * Ler o rotulo como UTF-8 dava «Cap?lo». */
                    for(int k2 = 0; q2->rot[k2]; k2++)
                        empurra(e, (unsigned char)q2->rot[k2], e->fonte);
                    if(q2->rot[0]){ empurra(e, ' ', e->fonte); empurra(e, ' ', e->fonte); }
                    for(int k2 = 0; q2->txt[k2]; ){
                        if(q2->txt[k2] == '-' && q2->txt[k2+1] == '-'){
                            if(q2->txt[k2+2] == '-'){ empurra(e, 0x97, e->fonte); k2 += 3; }
                            else { empurra(e, 0x96, e->fonte); k2 += 2; }
                            continue;
                        }
                        int cs; int gl = utf8_glifo((const unsigned char*)q2->txt + k2, &cs);
                        empurra(e, gl, e->fonte); k2 += cs;
                    }
                    /* a linha NAO justifica e o numero vai no fim: encher de espacos fazia-a
                     * quebrar, e a quebra justificava a primeira metade a' largura toda. */
                    { e->L.larg = 0;
                      long cp = escala_corpo(D_TEXTO);
                      long larg = mede(e->L.g, e->L.n, cp);
                      char np[8]; { char *q = ap_num(np, q2->pag); *q = 0; }
                      long lnp6 = 0;               /* o produto exacto em 10^-6 */
                      for(int k2 = 0; np[k2]; k2++) lnp6 += (long)largura((unsigned char)np[k2], F_NEG) * cp;
                      /* o texto à esquerda — in-place, sem segunda Linha no quadro */
                      if(e->L.n) desenrola_em(p, &e->L, (MARGEM + e->L.recuo) * 1000L, 0);
                      /* o número reutiliza a mesma linha: o texto já saiu */
                      e->L.n = 0;
                      for(int k2 = 0; np[k2]; k2++){ e->L.g[e->L.n].g = (unsigned char)np[k2];
                                                     e->L.g[e->L.n].f = F_NEG;
                                                     e->L.g[e->L.n].e = 0; e->L.n++; }
                      desenrola_em(p, &e->L, (MARGEM + COL) * 1000L - lnp6 / 1000, 1);
                      (void)larg;
                    }
                    e->L.n = 0; e->L.recuo = 0; e->fonte = F_REG; e->nfp = 0;
                }
                pagina_fecha(p); pagina_abre(p);
                i = j; continue;
            }
            if(!strcmp(cmd, "maketitle") || !strcmp(cmd, "tableofcontents") ||
               !strcmp(cmd, "newpage")   || !strcmp(cmd, "clearpage")){
                fecha_paragrafo(e);
                /* o `\maketitle` fecha a capa: o que vem a seguir começa em página nova */
                if(cmd[0] == 'm' && CENTRA && CAPA_ALT <= 0 && Y_CAPA > 0){
                    /* mediu-se: rebobina-se e faz-se outra vez, agora com o numero certo */
                    CAPA_ALT = Y_CAPA - p->y;
                    s_vai(&p->sf, CAPA_POS);
                    /* E O FUNDO TAMBEM. As reguas vao para outro ficheiro, e rebobinar so'
                     * o principal deixava-as escritas DUAS vezes — quatro reguas na capa
                     * onde o gabarito tem duas. Um stream esquecido e' meia reversao. */
                    if(p->fundo_on){ s_vai(&p->sfundo, CAPA_FUN); p->n_fundo = CAPA_NF; }
                    p->y = CAPA_Y; p->npag = CAPA_PAG; p->num = CAPA_PAG;
                    i = CAPA_I; e->L.n = 0;
                    continue;
                }
                if(cmd[0] == 'm' && CENTRA){
                    /* A CAPA FECHA-SE INTEIRA: o degrau e a cor que ela pos morrem com
                     * ela. Sem isto o `\gknota` (7,62) do aviso legal atravessava para o
                     * RESUMO — MEDIDO, o texto da pagina 2 saia a 7,620 onde a classe manda
                     * 10,95, e a palavra «campanha» media 35,97 contra 48,21 do gabarito. */
                    CENTRA = 0; Y_CAPA = -1;
                    DEG_FORCADO = -1; DEG_PROF = -1;
                    COR_TEXTO[0] = 0; COR_PROF = -1;
                    e->fonte = F_REG; e->nfp = 0; e->L.deg = -1;
                    pagina_fecha(p); pagina_abre(p); }
                if(cmd[0] == 'n' || cmd[0] == 'c'){ pagina_fecha(p); pagina_abre(p); }
                i = j; continue;
            }
            if(!strcmp(cmd, "underbrace")){
                e->ub = 1; e->ub_pf = PROF;
                empurra(e, 14, e->fonte);       /* o começo do vão: a chaveta vai daqui */
                i = j; continue;
            }
            /* a MOLDURA do \boxed: o 8 abre, o fecho do grupo empurra o 9, e o
             * pintor desenha o rectângulo em volta — antes, a caixa sumia */
            if(!strcmp(cmd, "boxed") && e->mat){
                long q2 = ate_abre(s, j, n);
                if(q2 < n && e->nexp < 8){
                    empurra(e, 8, e->fonte);
                    e->exp_pf[e->nexp] = PROF; e->exp_ant[e->nexp] = e->exp;
                    e->exp_frac[e->nexp] = 3; e->nexp++;
                    PROF++;
                    i = q2 + 1; continue;
                }
                i = j; continue;
            }
            /* o \quad é UM EM e o \qquad DOIS — o quadrado em (0xA0 → U+2003) lido da
             * carta das variáveis, não um espaço de palavra: media 10,3 pt onde o
             * gabarito tem 24,6. Sem a carta, cai no espaço como dantes. */
            if(!strcmp(cmd, "quad") || !strcmp(cmd, "qquad")){
                if(CARTA_MAT){
                    empurra(e, 0xA0, F_MAT);
                    if(cmd[1] == 'q') empurra(e, 0xA0, F_MAT);
                } else espaco_se_falta(e);
                i = j; continue;
            }
            /* o \operatorname é ROMANO: End, Hom, tr compõem na regular, não na
             * itálica das variáveis — é o que o gabarito faz. E o \text/\mbox é a
             * MESMA porta: texto dentro da fórmula, com os espaços que o modo
             * matemático de fora engole. */
            if((!strcmp(cmd, "operatorname") || !strcmp(cmd, "text")
             || !strcmp(cmd, "mbox") || !strcmp(cmd, "textrm")
             || !strcmp(cmd, "emph") || !strcmp(cmd, "textit")) && e->mat){
                /* o \emph DENTRO da fórmula: a mesma porta, mas em itálica —
                 * «(o \emph{traço})» saía com o comando escrito na página.
                 * E a porta RESPEITA O CONTEXTO: \text{contador} num \medido
                 * compõe na negra, e o \emph aí é a bold italic da referência */
                int eh_ita = (cmd[0] == 'e' || (cmd[0] == 't' && cmd[4] == 'i'));
                int f_txt = eh_ita
                          ? ((e->fonte == F_NEG && CARTA_NIT) ? F_NIT
                             : (N_CARTA > F_ITA) ? F_ITA : F_REG)
                          : (e->fonte == F_NEG ? F_NEG : F_REG);
                long q2 = ate_abre(s, j, n), f2 = fecha_chave(s, n, q2);
                if(f2 > 0){
                    for(long z2 = q2 + 1; z2 < f2; z2++){
                        /* os comandos DENTRO do texto processam-se: o \textbf{Lei 1}
                         * do align saía com «\textbf» escrito — negra para o textbf,
                         * léxico para os símbolos, e as ligaduras («---») também */
                        if(s[z2] == '\\'){
                            long j3 = z2 + 1;
                            if(j3 < f2 && (s[j3] == '\'' || s[j3] == '~' || s[j3] == '^')){
                                char ac = s[j3]; long k = j3 + 1; int abriu = 0;
                                if(k < f2 && s[k] == '{'){ abriu = 1; k++; }
                                char v = 0;
                                if(k < f2){
                                    if(s[k] == '\\' && k + 1 < f2 && s[k+1] == 'i'){ v = 'i'; k += 2; }
                                    else if(s[k] != '}'){ v = s[k]; k++; }
                                }
                                if(abriu){ while(k < f2 && s[k] != '}') k++; if(k < f2) k++; }
                                unsigned char comp = 0;
                                if(v){ const char *pv = strchr(AC_VOG, v);
                                if(pv){ int tv = (int)(pv - AC_VOG); unsigned char cv = 0;
                                    if(ac == '\x27') cv = AC_AGU[tv];
                                    else if(ac == '~') cv = AC_TIL[tv];
                                    else if(ac == '^') cv = AC_CIR[tv];
                                    if(cv) comp = cv; } }
                                if(comp) empurra(e, comp, f_txt);
                                else {
                                    if(ac == '~') empurra(e, 0x7E, F_SIM);
                                    else if(ac == '\'') empurra(e, 0xB4, f_txt);
                                    else empurra(e, 0x88, f_txt);
                                    if(v) empurra(e, (unsigned char)v, f_txt);
                                }
                                z2 = k - 1; continue;
                            }
                            char c3[24]; int k3 = 0;
                            while(j3 < f2 && isalpha((unsigned char)s[j3]) && k3 < 23)
                                c3[k3++] = s[j3++];
                            c3[k3] = 0;
                            if(!strcmp(c3, "textbf") || !strcmp(c3, "emph")
                            || !strcmp(c3, "textit")){
                                int f4 = (c3[0] == 't' && c3[4] == 'b')
                                       ? F_NEG
                                       : (f_txt == F_NEG && CARTA_NIT) ? F_NIT
                                       : (N_CARTA > F_ITA) ? F_ITA : F_NEG;
                                long qa = ate_abre(s, j3, n), fa = fecha_chave(s, n, qa);
                                if(fa > 0 && fa < f2){
                                    for(long z3 = qa + 1; z3 < fa; z3++){
                                        { int cl = 0, lg = liga_acha(s, n, z3, &cl);
                                          if(lg){ empurra(e, lg, f4); z3 += cl - 1; continue; } }
                                        int cs3; int g3 = utf8_glifo((const unsigned char*)s + z3, &cs3);
                                        empurra(e, g3, f4);
                                        z3 += (cs3 ? cs3 : 1) - 1;
                                    }
                                    z2 = fa; continue;
                                }
                                z2 = j3 - 1; continue;
                            }
                            { const Par *P3 = lex_acha(c3);
                              if(P3) empurra(e, P3->glifo, P3->simb ? F_SIM : f_txt); }
                            z2 = j3 - 1; continue;
                        }
                        if(s[z2] == '{' || s[z2] == '}') continue;
                        /* `~` dentro de \text: espaço, não til alto («Hurwitz~8») */
                        if(s[z2] == '~'){ espaco_se_falta(e); continue; }
                        { int cl = 0, lg = liga_acha(s, n, z2, &cl);
                          if(lg){ empurra(e, lg, f_txt); z2 += cl - 1; continue; } }
                        int cs2; int g2 = utf8_glifo((const unsigned char*)s + z2, &cs2);
                        empurra(e, g2, f_txt);
                        z2 += (cs2 ? cs2 : 1) - 1;
                    }
                    i = f2 + 1; continue;
                }
                i = j; continue;
            }
            /* OS DELIMITADORES QUE MEDEM O QUE EMBRULHAM: \left/\right e a família
             * \bigl/\bigr não têm tamanho próprio — o tamanho é a ÁREA INTERNA, e
             * o pintor dá-lho pelo boost. Controlo: 4/6/10 abrem ( [ {, 5/7/11
             * fecham. O `.` do \right. não põe nada. */
            if(e->mat && (!strcmp(cmd, "left") || !strcmp(cmd, "right")
                      || !strcmp(cmd, "bigl") || !strcmp(cmd, "bigr")
                      || !strcmp(cmd, "Bigl") || !strcmp(cmd, "Bigr")
                      || !strcmp(cmd, "biggl") || !strcmp(cmd, "biggr")
                      || !strcmp(cmd, "Biggl") || !strcmp(cmd, "Biggr"))){
                int ch = (j < n) ? s[j] : 0;
                long j2b = j + 1;
                int ctl = 0;
                if(ch == '\\' && j2b < n){
                    if(isalpha((unsigned char)s[j2b])){
                        /* o delimitador é um COMANDO: \langle, \rangle, \lbrace…
                         * — lia-se só a 1.a letra e «angle» caía na página */
                        char c3[16]; int k3 = 0;
                        while(j2b < n && isalpha((unsigned char)s[j2b]) && k3 < 15)
                            c3[k3++] = s[j2b++];
                        c3[k3] = 0;
                        if(!strcmp(c3, "langle")) ctl = 12;
                        else if(!strcmp(c3, "rangle")) ctl = 13;
                        else if(!strcmp(c3, "lbrace")) ctl = 10;
                        else if(!strcmp(c3, "rbrace")) ctl = 11;
                        else if(!strcmp(c3, "lvert") || !strcmp(c3, "rvert")
                             || !strcmp(c3, "vert")) ch = '|';
                        else { const Par *P3 = lex_acha(c3);
                               if(P3) empurra(e, P3->glifo, P3->simb ? F_SIM : e->fonte);
                               i = j2b; continue; }
                    } else { ch = s[j2b]; j2b++; }
                }
                if(!ctl) ctl = (ch == '(') ? 4 : (ch == ')') ? 5
                             : (ch == '[') ? 6 : (ch == ']') ? 7
                             : (ch == '{') ? 10 : (ch == '}') ? 11 : 0;
                if(ctl) empurra(e, ctl, e->fonte);
                else if(ch == '|') empurra(e, '|', e->fonte);
                i = j2b; continue;
            }
            /* os COMPRIMENTOS com dimensão colada — \itemsep1pt, \parskip2mm — são
             * atribuições, não texto: o comando era consumido e o «1pt» sobrava
             * escrito na página (linha 615) */
            if(!strcmp(cmd, "itemsep") || !strcmp(cmd, "parsep") || !strcmp(cmd, "topsep")
            || !strcmp(cmd, "parskip") || !strcmp(cmd, "labelsep") || !strcmp(cmd, "partopsep")
            || !strcmp(cmd, "itemindent") || !strcmp(cmd, "parindent")
            || !strcmp(cmd, "listparindent") || !strcmp(cmd, "leftmargin")){
                long q2 = j; int teve_dig = 0;
                while(q2 < n && s[q2] == ' ') q2++;
                if(q2 < n && (s[q2] == '+' || s[q2] == '-')) q2++;
                while(q2 < n && ((s[q2] >= '0' && s[q2] <= '9') || s[q2] == '.')){ q2++; teve_dig = 1; }
                if(teve_dig){ while(q2 < n && isalpha((unsigned char)s[q2])) q2++; j = q2; }
                i = j; continue;
            }
            /* o \mathbb: os BLACKBOARD extraídos da referência para a símbolo —
             * «\mathbb{Z}» saía com o comando escrito na página.
             * Sem `{` imediato: UM token (como o TeX). `ate_abre` até ao próximo
             * `{` qualquer saltava ao `{tabular}` do `\end` e a tabela nunca
             * fechava — corpo_topologico `\mathrm{id}_{\mathcal L}` na catálogo. */
            if(!strcmp(cmd, "mathbb")){
                long q2 = j;
                while(q2 < n && (s[q2] == ' ' || s[q2] == '\t')) q2++;
                if(q2 < n && s[q2] == '{'){
                    long f2 = fecha_chave(s, n, q2);
                    if(f2 > 0){
                        for(long z2 = q2 + 1; z2 < f2; z2++){
                            int ch2 = (unsigned char)s[z2];
                            int bb = (ch2 == 'Z') ? 0xA7 : (ch2 == 'Q') ? 0xA8
                                   : (ch2 == 'R') ? 0xA9 : (ch2 == 'N') ? 0xAA
                                   : (ch2 == 'F') ? 0xAF : (ch2 == 'H') ? 0xB2
                                   : (ch2 == 'O') ? 0xBD : (ch2 == 'C') ? 0xBE
                                   : (ch2 == 'T') ? 0xBF : (ch2 == 'P') ? 0xCA
                                   : (ch2 == 'K') ? 0xCB : 0;
                            if(bb) empurra(e, bb, F_SIM);
                            else if(ch2 != ' ') empurra(e, ch2, e->fonte);
                        }
                        i = f2 + 1; continue;
                    }
                } else if(q2 < n && s[q2] != '\\' && s[q2] != '$' && s[q2] != '&'){
                    int ch2 = (unsigned char)s[q2];
                    int bb = (ch2 == 'Z') ? 0xA7 : (ch2 == 'Q') ? 0xA8
                           : (ch2 == 'R') ? 0xA9 : (ch2 == 'N') ? 0xAA
                           : (ch2 == 'F') ? 0xAF : (ch2 == 'H') ? 0xB2
                           : (ch2 == 'O') ? 0xBD : (ch2 == 'C') ? 0xBE
                           : (ch2 == 'T') ? 0xBF : (ch2 == 'P') ? 0xCA
                           : (ch2 == 'K') ? 0xCB : 0;
                    if(bb) empurra(e, bb, F_SIM);
                    else empurra(e, ch2, e->fonte);
                    i = q2 + 1; continue;
                }
                i = j; continue;
            }
            /* `\mathcal{C}`: come o argumento (itálica); senão o comando sumia e o
             * `_K` do título/`$...$` podia ficar literal conforme o laço.
             * `\mathcal L` (sem chavetas) é um token — não `ate_abre` até `\end{…}`. */
            if(!strcmp(cmd, "mathcal") || !strcmp(cmd, "mathscr")){
                long q2 = j;
                while(q2 < n && (s[q2] == ' ' || s[q2] == '\t')) q2++;
                int fm = CARTA_MAT ? F_MAT : (N_CARTA > F_ITA) ? F_ITA : e->fonte;
                if(q2 < n && s[q2] == '{'){
                    long f2 = fecha_chave(s, n, q2);
                    if(f2 > 0){
                        for(long z2 = q2 + 1; z2 < f2; z2++){
                            int ch2 = (unsigned char)s[z2];
                            if(ch2 != ' ' && ch2 != '\\') empurra(e, ch2, fm);
                        }
                        i = f2 + 1; continue;
                    }
                } else if(q2 < n && s[q2] != '\\' && s[q2] != '$' && s[q2] != '&'){
                    empurra(e, (unsigned char)s[q2], fm);
                    i = q2 + 1; continue;
                }
                i = j; continue;
            }
            /* as SETAS ROTULADAS: a seta da símbolo com o rótulo por cima — o
             * rótulo é um giro da espiral, como qualquer expoente. O
             * \xleftrightarrow{J} da tabela primal/dual sobrava só o «J» */
            if(e->mat && (!strcmp(cmd, "xleftrightarrow") || !strcmp(cmd, "xrightarrow")
                      || !strcmp(cmd, "xleftarrow") || !strcmp(cmd, "xmapsto"))){
                int seta = strstr(cmd, "leftright") ? 0xAB
                         : strstr(cmd, "right") || strstr(cmd, "mapsto") ? 0xAE : 0xAC;
                long q2 = ate_abre(s, j, n), f2 = fecha_chave(s, n, q2);
                empurra(e, seta, F_SIM);
                if(f2 > 0){
                    int e0 = e->exp;
                    e->exp = esp_gira(e->exp >= 16 ? e->exp : 0, 1);
                    for(long z2 = q2 + 1; z2 < f2; z2++){
                        if(s[z2] == '\\'){
                            while(z2 + 1 < f2 && isalpha((unsigned char)s[z2+1])) z2++;
                            continue;
                        }
                        if(s[z2] == ' ') continue;
                        int cs2; int g2 = utf8_glifo((const unsigned char*)s + z2, &cs2);
                        if(isalpha(g2) && g2 < 128)
                            empurra(e, g2, CARTA_MAT ? F_MAT : F_ITA);
                        else empurra(e, g2, e->fonte);
                        z2 += (cs2 ? cs2 : 1) - 1;
                    }
                    e->exp = e0;
                    i = f2 + 1; continue;
                }
                i = j; continue;
            }
            /* os OPERADORES NOMEADOS do TeX — \det, \dim, \log… — compõem romanos,
             * como o \operatorname: o «det» de $\det=-1$ sumia e ficava «que é = -1» */
            if(e->mat){
                static const char *OPN[] = {"det","dim","ker","deg","log","exp","ln",
                    "lim","max","min","sup","inf","gcd","hom","arg","sin","cos","tan",
                    "cot","bmod","mod","sinh","cosh","tanh","coth","sec","csc",
                    "arcsin","arccos","arctan",0};
                int oo = -1;
                for(int t2 = 0; OPN[t2]; t2++) if(!strcmp(cmd, OPN[t2])){ oo = t2; break; }
                if(oo >= 0){
                    for(const char *z2 = OPN[oo]; *z2; z2++)
                        empurra(e, (unsigned char)*z2,
                                e->fonte == F_NEG ? F_NEG : F_REG);
                    i = j; continue;
                }
            }
            /* os integrais compostos: \iint são DOIS ∫, \iiint três, \oint o simples —
             * sem esta porta o comando sumia e o _{M} ficava órfão a flutuar */
            if(!strcmp(cmd, "iint") || !strcmp(cmd, "iiint") || !strcmp(cmd, "oint")){
                int nn = cmd[0] == 'o' ? 1 : (cmd[2] == 'i' ? 3 : 2);
                for(int t2 = 0; t2 < nn; t2++) empurra(e, 0xF2, F_SIM);
                i = j; continue;
            }
            if(!strcmp(cmd, "sqrt") && e->mat){
                /* a raiz NÃO é um glifo colado: desenha-se no desenrola, pelos corpos —
                 * aqui só se marca o radicando (5) */
                long q2 = j;
                while(q2 < n && (s[q2] == ' ' || s[q2] == '\t')) q2++;
                if(q2 < n && s[q2] == '{' && e->nexp < 8){
                    e->exp_pf[e->nexp] = PROF; e->exp_ant[e->nexp] = e->exp;
                    e->exp_frac[e->nexp] = 0; e->nexp++;
                    e->exp = 5; PROF++;                 /* o radicando leva o vinculum */
                    i = q2 + 1; continue;
                }
                if(q2 < n && s[q2] == '\\'){            /* \sqrt\delta: o radicando é comando */
                    long j3 = q2 + 1; char c3[24]; int k3 = 0;
                    while(j3 < n && isalpha((unsigned char)s[j3]) && k3 < 23) c3[k3++] = s[j3++];
                    c3[k3] = 0;
                    const Par *P3 = lex_acha(c3);
                    if(P3){ int e0 = e->exp; e->exp = 5;
                            empurra(e, P3->glifo, P3->simb ? F_SIM : e->fonte);
                            e->exp = e0; }
                    i = j3; continue;
                }
                if(q2 < n){                            /* \sqrt D: um token */
                    int e0 = e->exp; e->exp = 5;
                    empurra(e, (unsigned char)s[q2], e->fonte);
                    e->exp = e0; i = q2 + 1; continue;
                }
                i = j; continue;
            }
            /* o \frac na régua da linha: numerador sobe, barra, denominador desce (¹/ₐ) —
             * a forma inline; a pilha vertical com traço é do display, e fica dita */
            if(!strcmp(cmd, "frac") || !strcmp(cmd, "tfrac") || !strcmp(cmd, "dfrac")){
                long q2 = j;
                while(q2 < n && (s[q2] == ' ' || s[q2] == '\t')) q2++;
                if(q2 < n && s[q2] == '{' && e->nexp < 8){
                    int pilha = (e->disp && e->exp == 0);   /* no display: a pilha vertical */
                    e->exp_pf[e->nexp] = PROF; e->exp_ant[e->nexp] = e->exp;
                    e->exp_frac[e->nexp] = pilha ? 2 : 1; e->nexp++;
                    e->exp = pilha ? 4 : 1; PROF++;
                    i = q2 + 1; continue;
                }
                /* a forma sem chavetas — `\frac1a`, `\frac1\sigma` — é um TOKEN por
                 * lado, como no TeX: um caractere, ou um COMANDO do léxico. E no display
                 * empilha como a forma de chavetas — o «\sigma» saía LITERAL, e a
                 * fração deitada onde o gabarito tem a vertical. */
                if(q2 < n){
                    int gtok[2], ftok[2], nt = 0; long z2 = q2;
                    while(nt < 2 && z2 < n){
                        if(s[z2] == '\\'){
                            long j3 = z2 + 1; char c3[24]; int k3 = 0;
                            while(j3 < n && isalpha((unsigned char)s[j3]) && k3 < 23)
                                c3[k3++] = s[j3++];
                            c3[k3] = 0;
                            const Par *P3 = lex_acha(c3);
                            if(!P3) break;
                            gtok[nt] = P3->glifo;
                            ftok[nt] = P3->simb ? F_SIM : e->fonte; nt++;
                            z2 = j3;
                        } else {
                            int gz = (unsigned char)s[z2];
                            gtok[nt] = gz;
                            ftok[nt] = (isalpha(gz) && N_CARTA > F_ITA)
                                     ? (CARTA_MAT ? F_MAT : F_ITA) : e->fonte;
                            nt++; z2++;
                        }
                        while(z2 < n && (s[z2] == ' ' || s[z2] == '\t')) z2++;
                    }
                    if(nt == 2){
                        int e0 = e->exp;
                        if(e->disp && e->exp == 0){
                            e->exp = 4;  empurra(e, gtok[0], ftok[0]);
                            e->exp = -4; empurra(e, gtok[1], ftok[1]);
                        } else {
                            e->exp = 1;  empurra(e, gtok[0], ftok[0]);
                            e->exp = e0; empurra(e, '/', e->fonte);
                            e->exp = -1; empurra(e, gtok[1], ftok[1]);
                        }
                        e->exp = e0;
                        i = z2; continue;
                    }
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "cite")){      /* a citação RESOLVE: [n] pelo número do \bibitem
                                            * na ordem da bibliografia — como o gabarito; a
                                            * chave só rotula se a bibliografia não a tem */
                long q2 = ate_abre(s, j, n), f2 = fecha_chave(s, n, q2);
                if(f2 > 0){
                    empurra(e, '[', e->fonte);
                    long a2 = q2 + 1;
                    while(a2 < f2){
                        long b2 = a2;
                        while(b2 < f2 && s[b2] != ',') b2++;
                        int nm = bib_num(s + a2, (int)(b2 - a2));
                        if(nm > 0){
                            if(nm >= 10) empurra(e, '0' + nm / 10, e->fonte);
                            empurra(e, '0' + nm % 10, e->fonte);
                        } else
                            for(long z2 = a2; z2 < b2; z2++)
                                empurra(e, (unsigned char)s[z2], e->fonte);
                        if(b2 < f2){ empurra(e, ',', e->fonte); empurra(e, ' ', e->fonte); }
                        a2 = b2 + 1;
                        while(a2 < f2 && s[a2] == ' ') a2++;
                    }
                    empurra(e, ']', e->fonte);
                    i = f2 + 1; continue;
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "bibitem")){   /* a entrada: parágrafo novo com o rótulo [n] */
                long q2 = ate_abre(s, j, n), f2 = fecha_chave(s, n, q2);
                if(f2 > 0){
                    fecha_paragrafo(e); p->y -= 3*PT;
                    int nm = bib_num(s + q2 + 1, (int)(f2 - q2 - 1));
                    empurra(e, '[', e->fonte);
                    if(nm >= 10) empurra(e, '0' + nm / 10, e->fonte);
                    empurra(e, '0' + (nm > 0 ? nm % 10 : 0), e->fonte);
                    empurra(e, ']', e->fonte);
                    empurra(e, ' ', e->fonte);
                    i = f2 + 1; continue;
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "url")){       /* o endereço compõe na mono, como o \code */
                long q2 = ate_abre(s, j, n), f2 = fecha_chave(s, n, q2);
                if(f2 > 0){
                    int fu = (N_CARTA > F_MON) ? F_MON : e->fonte;
                    for(long z2 = q2 + 1; z2 < f2; z2++)
                        empurra(e, (unsigned char)s[z2], fu);
                    i = f2 + 1; continue;
                }
                i = j; continue;
            }
            if(!strcmp(cmd, "tag")){       /* o rótulo da equação: (L1), como o pdflatex põe */
                long q2 = ate_abre(s, j, n), f2 = fecha_chave(s, n, q2);
                if(f2 > 0){
                    espaco_se_falta(e); empurra(e, '(', e->fonte);
                    for(long z2 = q2 + 1; z2 < f2; z2++) empurra(e, (unsigned char)s[z2], e->fonte);
                    empurra(e, ')', e->fonte);
                    i = f2 + 1; continue;
                }
                i = j; continue;
            }
            const Par *P = lex_acha(cmd);
            if(P){
                /* COMPOSIÇÃO DAS GEOMETRIAS (corpo_topologico / Alonzo): Bin (∘,⊕,⊗,×)
                 * e Rel (→,≤) espacejam pela semente no nível 0. Sem função nova
                 * (MAX_FUN). Antes só setas da lista antiga; circ/oplus saíam colados. */
                int at = 0;   /* 0 Ord, 1 Bin, 2 Rel */
                if(e->mat){
                    if(!strcmp(cmd, "times") || !strcmp(cmd, "pm") || !strcmp(cmd, "div")
                    || !strcmp(cmd, "oplus") || !strcmp(cmd, "otimes") || !strcmp(cmd, "circ")
                    || !strcmp(cmd, "bullet")|| !strcmp(cmd, "star")
                    || !strcmp(cmd, "wedge") || !strcmp(cmd, "vee")
                    || !strcmp(cmd, "cap")   || !strcmp(cmd, "cup")) at = 1;
                    else if(!strcmp(cmd, "to") || !strcmp(cmd, "rightarrow")
                         || !strcmp(cmd, "longrightarrow") || !strcmp(cmd, "mapsto")
                         || !strcmp(cmd, "leftarrow") || !strcmp(cmd, "longleftarrow")
                         || !strcmp(cmd, "Rightarrow") || !strcmp(cmd, "Leftarrow")
                         || !strcmp(cmd, "Longrightarrow") || !strcmp(cmd, "Longleftarrow")
                         || !strcmp(cmd, "leftrightarrow") || !strcmp(cmd, "Leftrightarrow")
                         || !strcmp(cmd, "le") || !strcmp(cmd, "leq")
                         || !strcmp(cmd, "ge") || !strcmp(cmd, "geq")
                         || !strcmp(cmd, "ne") || !strcmp(cmd, "neq")
                         || !strcmp(cmd, "equiv") || !strcmp(cmd, "approx")
                         || !strcmp(cmd, "sim") || !strcmp(cmd, "propto")
                         || !strcmp(cmd, "in") || !strcmp(cmd, "notin")
                         || !strcmp(cmd, "subset") || !strcmp(cmd, "subseteq")
                         || !strcmp(cmd, "supset") || !strcmp(cmd, "cong")
                         || !strcmp(cmd, "perp") || !strcmp(cmd, "colon")
                         || !strcmp(cmd, "mid")) at = 2;
                }
                if(at && e->exp == 0) espaco_se_falta(e);
                empurra(e, P->glifo, P->simb ? F_SIM : e->fonte);
                if(at && e->exp == 0) espaco_se_falta(e);
                i = j; continue;
            }
            /* um comando que não está no léxico não vira lixo na página: consome-se e segue */
            i = j; continue;
        }

        int cons; int g = utf8_glifo((const unsigned char*)s + i, &cons);
        /* SO ASCII vai para a Symbol. Um caractere acentuado nunca e matematica, e deixar o
         * isalpha() decidir por ele partia a palavra ao meio: com o modo ligado, 'coracao' saia
         * 'cora' na Symbol e o resto na regular — dois Tj, e a palavra deixava de existir. Foi
         * o mesmo dano do cifrao no verbatim, por outra porta. */
        /* NO MODO MATEMATICO A LETRA LATINA NAO E' GREGA, e esta linha dizia que era.
         *
         * Tinha `(e->mat && isalpha(g)) ? F_SIM : e->fonte` — toda a letra latina dentro de $...$
         * ia para a Symbol. E na Symbol o `i` e' iota, o `u` e' upsilon, o `v` e' pi-variante:
         * «\sum_i u_i v_i» saía «∑_ι υ_ι ϖ_ι». Sete mil e cento e vinte e uma letras trocadas
         * no catalogo.
         *
         * Os gregos JA' eram tratados dez linhas acima, pela tabela do lexico: `\alpha` tem
         * `simb=1` e vai para a Symbol pelo caminho certo. Esta linha era uma segunda regra a
         * fazer o mesmo trabalho para quem nao lhe pertencia — a regua aplicada onde nao cabe.
         *
         * E o dano nao era so' o glifo: cada troca de fonte QUEBRA o pedaco, logo o modo
         * matematico partia o texto em Tj de uma letra. 27 158 dos 84 214 Tj tinham um so'
         * glifo — 32%. Um defeito, dois sintomas.
         *
         * A variavel matematica compoe-se na fonte do TEXTO. (O italico e' o que a tipografia
         * manda, e fica nomeado: o tradutor ainda nao tem a variante italica embutida, e por
         * isso usa a regular — que e' a letra certa com o corte errado, e nao outra letra.) */
        /* AS LIGADURAS DO TRAVESSÃO: `---` é UM caractere (WinAnsi 151 = U+2014), não três hífenes, e
         * `--` é o traço-de-N. O tradutor já tem a largura de ambos desde a correcção do
         * WinAnsi — só lhes faltava a porta. Sem isto o título do enredo saía «O Enredo ---
         * a partida sem fim» com três hífenes onde o original tem um traço. */
        { int cl = 0, lg = liga_acha(s, n, i, &cl);
          if(lg){ empurra(e, lg, e->fonte); i += cl; continue; } }
        /* o `*` da matemática é o ∗ do eixo (a símbolo), não o asterisco alto do
         * texto — é o T^* do gabarito, discreto e centrado */
        if(e->mat && g == '*'){
            empurra(e, 0x2A, F_SIM);
            i += cons; continue;
        }
        /* a vírgula é PONTUAÇÃO na fórmula: espaço depois, nunca antes — «(V), \qquad»
         * do gabarito. O espaço de antes já não entra (o modo matemático engole-o). */
        if(e->mat && g == ','){
            empurra(e, ',', e->fonte); espaco_se_falta(e);
            i += cons; continue;
        }
        /* AS RELAÇÕES DO TECLADO ESPACEJAM no modo matemático: o `=` sempre;
         * o `+` e o `-` só quando BINÁRIOS. O vão é o da semente (espaco_atomo),
         * a mesma geometria de `\oplus`/`\circ` — composição, não espaço à parte. */
        if(e->mat && (g == '=' || g == '<' || g == '>' || g == '+' || g == '-')){
            int bin = (g == '=' || g == '<' || g == '>');
            if(!bin && e->L.n && e->L.g[e->L.n-1].e == (signed char)e->exp){
                /* o anterior tem de estar NO MESMO nível: o `-` que abre um expoente
                 * (f^{-1}) é unário — o `f` de trás é a base, não um operando */
                int ant = e->L.g[e->L.n-1].g;
                bin = (ant >= '0' && ant <= '9') || (ant >= 'a' && ant <= 'z')
                   || (ant >= 'A' && ant <= 'Z') || ant == ')' || ant == ']' || ant > 127;
            }
            /* o menos da matemática é o − comprido da símbolo (U+2212), não o hífen */
            { int gm = (g == '-' && CARTA_SIM) ? 0xAD : g;
              int fm = (g == '-' && CARTA_SIM) ? F_SIM : e->fonte;
              if(bin && e->exp == 0){ espaco_se_falta(e); empurra(e, gm, fm); espaco_se_falta(e); }
              else empurra(e, gm, fm); }
            i += cons; continue;
        }
        /* A VARIÁVEL MATEMÁTICA COMPÕE-SE NA ITÁLICA — «o itálico é o que a tipografia
         * manda», e agora a variante existe e embute-se (a mesma porta do \emph). SÓ a
         * letra latina ASCII: um acento nunca é matemática, e o grego já vem do léxico. */
        if(e->mat && ((g >= 'a' && g <= 'z') || (g >= 'A' && g <= 'Z')) && N_CARTA > F_ITA){
            /* a variável vai para a carta MATEMÁTICA se ela abriu: é a métrica do
             * cmmi (V: bearing 56, avanço 583) e não a da itálica de texto (209, 743) */
            empurra(e, g, (e->fonte == F_NEG && CARTA_MTB) ? F_MTB
                          : (CARTA_MAT ? F_MAT : F_ITA));
            i += cons; continue;
        }
        /* a chaveta abre e fecha o escopo do degrau: ao sair do grupo onde o `\fontsize`
         * foi posto, o degrau volta ao que estava — que é o que o LaTeX faz */
        if(g == '{') PROF++;
        else if(g == '}'){
            if(PROF > 0) PROF--;
            if(DEG_PROF >= 0 && PROF < DEG_PROF){ DEG_FORCADO = -1; DEG_PROF = -1; }
            if(COR_PROF >= 0 && PROF < COR_PROF){ COR_TEXTO[0] = 0; COR_PROF = -1; }
            while(e->nfp > 0 && e->fp_p[e->nfp-1] >= PROF){ e->nfp--; e->fonte = e->fp_f[e->nfp]; }
        }
        empurra(e, g, e->fonte);
        i += cons;
    }
    fecha_paragrafo(e);
    *glifos = e->glifos;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * §X6  A VOLTA — o texto sai do PDF que entrou no .tex
 * ───────────────────────────────────────────────────────────────────────────── */

/* lê os (…) Tj do PDF sem compressão e devolve os glifos, na ordem. É a volta do §X4/§X5. */
static long extrai(const char *pdf, long n, char *out, long lim){
    /* a fita de glifos, na régua nova: cada `/Gix_gb Do` é um glifo desenhado — o byte
     * sai do próprio nome da assinatura, na ordem da página */
    long o = 0;
    for(long i = 0; i + 2 < n; i++){
        if(pdf[i] != '/' || pdf[i+1] != 'G') continue;
        long q = i + 2, gb = -1;
        while(q < n && pdf[q] >= '0' && pdf[q] <= '9') q++;
        if(q < n && pdf[q] == '_'){
            q++; gb = 0;
            while(q < n && pdf[q] >= '0' && pdf[q] <= '9'){ gb = gb*10 + (pdf[q]-'0'); q++; }
            while(q < n && pdf[q] == ' ') q++;
            if(q + 1 < n && pdf[q] == 'D' && pdf[q+1] == 'o' && gb > 0 && o < lim - 1){
                out[o] = (char)gb; o = o + 1; }
            i = q;
        }
    }
    out[o] = 0; return o;
}

/* ───────────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0, saltadas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    /* o idioma da bateria: sem isto ela conta UMA unidade grossa (o exit) em vez das que ha */
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}
/* A TERCEIRA PALAVRA — a mesma de lib/unidade.h, e por isso repetida aqui: este ficheiro
 * tem a SUA propria implementacao de ok(), porque a da lib despeja por atexit e aqui a
 * ordem da saida importa. As duas falam a mesma lingua com a bateria, e agora tambem esta.
 *
 * Ela serve o caso em que a medida nao se faz por falta de um recurso de FORA — a fonte
 * que nao esta instalada. Antes isso era um puts, e um puts nao entra em conta nenhuma:
 * numa maquina sem a Liberation o medidor emitia nove unidades a menos e o total da
 * bateria descia sem se saber porque. */
static void saltou(const char *q, const char *porque){
    saltadas++;
    printf("#UNIT salta %s  [%s]\n", q, porque);
    printf("  [salta] %s — NAO MEDIDO: %s\n", q, porque);
}



/* ─────────────────────────────── A AVALIAÇÃO NAS RAÍZES: expandir, não reconhecer ──
 *
 * O Aarão: «em corpo estelar tem o código do universal, isso deve servir para fazer um
 * tradutor latex→pdf».
 *
 * E serve, e a razão é a do `universal.c`: **a transformada universal é a que leva
 * CONVOLUÇÃO em PRODUTO, e quem faz isso é a AVALIAÇÃO NAS RAÍZES**. Avaliar num zero é
 * um homomorfismo de anéis — leva o produto no produto, casa a casa —, e é por ser
 * homomorfismo que não precisa de saber o que está a traduzir.
 *
 * `\gkcapa{A}{B}{C}` é exactamente isso: o comando é o polinómio, os argumentos são o
 * ponto, e traduzir é AVALIAR. A definição já existe, escrita pelo autor no `estilo.tex`:
 *
 *     \providecommand{\gkcapa}[3]{ \title{... #1 ... #2 ... #3 ...} }
 *
 * O que este tradutor fazia era o contrário — reconhecer por nome, com um `strcmp` por
 * comando. MEDIDO antes desta passagem: **74 macros definidas no estilo.tex, 2
 * reconhecidas, e 4664 usos nos três documentos**. As outras 72 caíam como texto corrido:
 * era por isso que a capa do enredo saía «O Enredo --- a partida sem fim contado na
 * linguagem do xadrez ... ele a conta» numa linha só, a 11 pt.
 *
 * E o `strcmp` não é só incompleto — é a estrutura errada, porque põe o tradutor a
 * PRIVILEGIAR os nomes que conhece. Com a avaliação, a fonte é que manda: cada macro nova
 * que o autor escreva no `estilo.tex` passa a ser traduzida sem se lhe tocar aqui.
 */
#define MAX_MAC 512
typedef struct { char nome[48]; int nargs; char *corpo; } Macro;
#define MAC     ((Macro*)disco_buf(7, (long)(MAX_MAC * sizeof(Macro))))
static int N_MAC = 0;
long EXPANDIDAS = 0;          /* quantas avaliações se fizeram — a bateria lê isto */
long tex_expandidas(void){ return EXPANDIDAS; }
int tex_n_mac(void){ return N_MAC; }

/* o argumento entre chavetas a partir de `i` (que aponta ao `{`), com aninhamento */
static long fecha_chave(const char *s, long n, long i){
    if(i >= n || s[i] != '{') return -1;
    int d = 1; long q = i + 1;
    while(q < n && d){ if(s[q]=='{') d++; else if(s[q]=='}') d--; if(d) q++; }
    return d ? -1 : q;                       /* índice do `}` que fecha */
}

/* recolhe `\newcommand{\nome}[n]{corpo}` — e `\provide`/`\renew`, que só diferem no efeito */

/* A MEDIDA VEM ANTES DA ESCRITA. Uma passagem CONTA quanto a expansão ocupa (o == 0, o byte
 * escreve-se em lado nenhum, só se soma), aloca-se EXACTO, e a segunda ESCREVE. Sem dobrar
 * capacidade, sem realloc, sem aproximar por potências — o tamanho é contado, não adivinhado.
 * A lógica corre UMA vez, parametrizada por `o`: contar e escrever são a mesma passagem com o
 * destino trocado, que é o medir-pela-metade do corpo_analitico. */

/* uma passagem de avaliação: devolve buffer novo, `*n` actualizado, e quantas avaliou */

/* avalia até estabilizar — as macros chamam-se umas às outras, e a profundidade é finita
 * por construção (o TeX rejeita recursão sem fim); o tecto é uma rede, não uma escolha */


/* ─── A ABSORÇÃO: pdf → estrela, que é o outro sentido do MOVE ──────────────────────
 *
 * O `corpo_analitico.tex` diz como se mede aqui, e diz-o na última linha da especificação:
 *
 *     medir | resíduo 0, NÃO COMPARAÇÃO | a prova dos nove: resolver e provar
 *
 * e, sem margem: «uma asserção que compara contra um valor escrito passa no objecto certo
 * E NO TROCADO --- ela verifica a aritmética de quem a escreveu, não o objecto. A REVERSÃO
 * SEPARA-OS.»
 *
 * O `tools/compara.js` é uma comparação contra o pdflatex. Ajustar o tradutor até bater com
 * ele é verificar a aritmética do pdflatex. Isto é o outro caminho: a estrela tem `MOVE` nos
 * dois sentidos --- `-1` emite (compor o PDF), `+1` absorve (lê-lo de volta) --- e a medida
 * é o RESÍDUO da volta, sem oráculo nenhum.
 *
 *     enredo.tex --[emite]--> A.pdf --[absorve]--> B.tex --[emite]--> B.pdf
 *
 * e o resíduo é `corpo(A) − corpo(B)`. Zero quer dizer que a volta fecha.
 *
 * E é COMPILADO e não literal, que é o que o `estrela_emite.c` já dizia: o que atravessa
 * não é o texto, é o CORPO --- a sequência de (glifo, x, y, corpo, fonte). Um PDF não guarda
 * `\emph` nem comentários, e exigi-los de volta seria exigir o que não foi escrito. O que
 * tem de voltar é o que foi POSTO NA PÁGINA. */

/* ─── A ABSORÇÃO IRRADIA: sem fila, sem buffer, sem RAM ────────────────────────────
 *
 * O `corpo_analitico.tex` responde à pergunta «porque não fecha a volta?» em dois sítios:
 *
 *     «não há RAM. O estado vive no disco, endereçado por MOVE»                    (§estrela)
 *     «E UMA ESTRELA NÃO TEM FILA. Não tem buffer, não tem memória, não guarda
 *      para depois: IRRADIA.»                                                    (§ordenação)
 *
 * A minha primeira versão fazia o contrário: lia tudo para `Posto v[4000000]`, e só depois
 * escrevia. MEDIDO: **366 MB de RAM estática** — e com tecto, que é o que uma fila sempre
 * tem. Aqui lê-se e escreve-se na MESMA passagem, e o estado residente é uma linha de
 * variáveis: não há vector nenhum, e o tamanho do documento deixa de ser um limite.
 *
 * `MOVE(slot, sentido)`: `-1` emite (compor), `+1` absorve (ler de volta). É a mesma porta.
 */

/* varre um PDF composto por este ficheiro e chama `poe` em cada glifo posto, por ordem.
 * Os streams saem sem compressão — é por isso que a volta não precisa de biblioteca. */

/* ── emitir `.tex` a partir do corpo, à medida que ele chega ────────────────────────── */
/* ── A ASSINATURA DA ESTRUTURA: o que a roupa perde está na GEOMETRIA ────────────────
 *
 * A minha dúvida era «a marcação não volta — o `.tex` da volta não tem `\chapter` nem
 * parágrafos, e por isso recompõe numa página». O repositório responde, e responde no
 * `letra_assinatura.c`, com a frase que o Aarão me disse sobre as letras:
 *
 *     «CADA LETRA TEM UMA ASSINATURA — e ela NÃO MUDA COM A FONTE. A geometria muda
 *      toda: as coordenadas, o avanço, a espessura. A assinatura não.»
 *
 * O mesmo vale um andar acima. A estrutura não se perdeu: **está codificada na geometria**,
 * e a assinatura de um bloco lê-se dela sem adivinhar nada:
 *
 *     o CORPO      diz o degrau da escala --- 23,42 é título, 10,50 é texto
 *     o SALTO em y maior que a entrelinha do degrau: acabou o parágrafo
 *     o RECUO em x  a primeira linha de um parágrafo começa mais à direita
 *
 * E nenhum destes é um limiar meu: o corpo compara-se com a ESCALA que o `estilo.tex`
 * declara, e a entrelinha do degrau vem da mesma tabela. Onde eu escrevia um número,
 * pergunta-se à escala. */
/* o typedef Escreve (tem FILE*) vive no wrapper tex.c --- é do escritor da volta, não do núcleo. */

/* ── QUAL COMANDO USA QUAL CORPO: lido do estilo, não escolhido aqui ─────────────────
 * O `estilo.tex` declara os dois lados: `\providecommand{\gktit}{\fontsize{23.42}...}` dá
 * o corpo do degrau nomeado, e `\titleformat{\chapter}...{\gktit}` diz que nível o usa.
 * A minha primeira versão adivinhava pela POSIÇÃO na escala (`d >= N_ESCALA-1 ? chapter`)
 * --- e adivinhou mal: MEDIDO, escreveu 1 `\chapter` onde o documento tem 148. */
static struct { char cmd[24]; long corpo; char cor[24]; } NIVEL_CORPO[8];
static int N_NIVEL = -1;


/* ── O ESPAÇO À VOLTA DE UM TÍTULO ──────────────────────────────────────────────────
 * O código punha `p->y -= 8` para todos, e 8 não é do estilo nem do LaTeX: é um número
 * escrito à mão. Daí os títulos colarem-se ao texto de cima e de baixo.
 *
 * Onde o estilo declara `\titlespacing`, é esse que manda. Onde não declara --- e só o
 * `\chapter` o faz --- o espaço sai da ESCALA, que é a régua deste documento: antes, a
 * entrelinha do próprio degrau; depois, a do texto. Não se trazem os valores do LaTeX de
 * fora, porque uma régua de outro corpo não transporta (teoria, thm:transporte). */
static void espaco_titulo(const char *cmd, long deg, long *antes, long *depois){
    *antes = 0; *depois = 0;
    /* pela CACHE: esta funcao e' chamada por cada titulo, e lia um megabyte de cada vez */
    { const char *b = estilo_texto(NULL);
    if(b){
        char alvo[64]; char *ka = ap_str(alvo, "titlespacing*{\\"); ka = ap_str(ka, cmd); *ka++ = '}'; *ka = 0;
        const char *q = strstr(b, alvo);
        if(!q){ ka = ap_str(alvo, "titlespacing{\\"); ka = ap_str(ka, cmd); *ka++ = '}'; *ka = 0; q = strstr(b, alvo); }
        if(q){
            long a = 0, c = 0;
            /* `{esquerda}{antes}{depois}` — o primeiro salta-se */
            const char *z = q + strlen(alvo);
            while(*z && *z != '{') z++;
            if(*z){ z++; while(*z && *z != '{') z++; }
            int _ok = 0;
            /* `{N`: o `{` literal, a mantissa por fixo_mil — vírgula-fixa, exacta */
            if(*z == '{'){ const char *e; long av = fixo_mil(z + 1, &e); if(e != z + 1){ a = av; _ok = 1; } }
            if(_ok){
                while(*z && *z != '}') z++; if(*z) z++;
                if(*z == '{'){ const char *e; long cv = fixo_mil(z + 1, &e); if(e != z + 1){ *antes = a; *depois = cv; return; } }
            }
            (void)c;
        }
    }
    }
    if(deg >= 0 && deg < N_ESCALA){
        *antes = ESCALA[deg].entre / 2;
        *depois = (N_ESCALA > D_TEXTO ? ESCALA[D_TEXTO].entre : 15000) / 2;
    }
}

/* a régua que o `\titleformat` declara no grupo final `[...]`, se declarar */
static int regua_do_comando(const char *cmd, long *esp, char *cor, size_t nc){
    *esp = 0; if(nc) cor[0] = 0;
    const char *b = estilo_texto(NULL);
    if(!b) return 0;
    char alvo[64]; char *ka = ap_str(alvo, "titleformat{\\"); ka = ap_str(ka, cmd); *ka++ = '}'; *ka = 0;
    const char *q = strstr(b, alvo);
    if(!q) return 0;
    const char *fim = strstr(q + 1, "\\titleformat");
    const char *r = strstr(q, "\\titlerule");
    if(!r || (fim && r > fim)) return 0;
    /* `\titlerule[1.2pt]` — a espessura; sem o opcional o LaTeX usa 0,4pt */
    if(r[10] == '['){ const char *e; long v = fixo_mil(r + 11, &e); if(e != r + 11) *esp = v; } else *esp = 400;
    const char *c = strstr(q, "\\color{");
    /* a cor da régua é a última antes dela, que é a do próprio grupo `[...]` */
    for(const char *z = q; (z = strstr(z, "\\color{")) != NULL && z < r; z += 7) c = z;
    if(c && c < r && nc){
        const char *w = c + 7; size_t t = 0;
        while(*w && *w != '}' && t + 1 < nc) cor[t++] = *w++;
        cor[t] = 0;
    }
    return 1;
}

/* a cor que o `\titleformat` declara para este nível, ou NULL se não declara */
static const char *cor_do_comando(const char *cmd){
    for(int t = 0; t < N_NIVEL; t++)
        if(!strcmp(NIVEL_CORPO[t].cmd, cmd) && NIVEL_CORPO[t].cor[0])
            return NIVEL_CORPO[t].cor;
    return NULL;
}

/* e o inverso, que é o que a IDA precisa: o degrau que este nível deve usar */
static long degrau_do_comando(const char *cmd){
    for(int t = 0; t < N_NIVEL; t++)
        if(!strcmp(NIVEL_CORPO[t].cmd, cmd)) return degrau_de(NIVEL_CORPO[t].corpo);
    /* O QUE O ESTILO NAO DECLARA DERIVA-SE, e nao se inventa. O `\part` nao tem
     * `\titleformat` — mas tem POSICAO na hierarquia, e a escala e' geometrica: o degrau
     * e' o do nivel abaixo mais um passo. Sem isto o `\part` caia no `\gktit` do capitulo
     * e saia a 25,76 de altura onde o gabarito tem 21,91. */
    if(!strcmp(cmd, "part")){
        long d = degrau_do_comando("chapter");
        return d >= 0 ? d - 1 : -1;          /* uma razao ABAIXO do capitulo */
    }
    return -1;
}

/* o comando cujo corpo é este, ou NULL se nenhum nível o usa (logo é texto corrido) */
static const char *comando_de_corpo(long corpo){
    for(int t = 0; t < N_NIVEL; t++)
        if(NIVEL_CORPO[t].corpo > corpo - 10 && NIVEL_CORPO[t].corpo < corpo + 10)
            return NIVEL_CORPO[t].cmd;
    return NULL;
}

/* o degrau da escala a que um corpo pertence, ou -1 se não é nenhum */
static long degrau_de(long corpo){
    for(long t = 0; t < N_ESCALA; t++)
        if(ESCALA[t].corpo > corpo - 10 && ESCALA[t].corpo < corpo + 10) return t;
    return -1;
}
