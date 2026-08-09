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
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>   /* a costura da saída: um formatador variádico próprio (o tradutor fá-lo) */
#include "spline.h"   /* a carta da fonte: a largura vem da CURVA */
#include "disco.h"    /* «o ficheiro É o vector»: os buffers grandes vivem no DISCO */
#include "le_num.h"   /* o strtod e o hex do núcleo, sem libc --- fonte única, travada em str2dbl_dual.c */

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
static char *disco_buf(int i, long n){
    static char *m[16];
    static const char *nome[16] = {
        "../dados/tex_estilo.bin", "../dados/tex_classe.bin", "../dados/tex_idioma.bin",
        "../dados/tex_fonte.bin",  "../dados/tex_corpo.bin",  "../dados/tex_volta.bin",
        "../dados/tex_toc.bin",    "../dados/tex_mac.bin",    "../dados/tex_hif.bin",
        "../dados/tex_cores.bin",  "../dados/tex_desc.bin",   "../dados/tex_pag.bin",
        "../dados/tex_x12.bin",    "../dados/tex_x13.bin",
        "../dados/tex_x14.bin",    "../dados/tex_x15.bin" };
    if(!m[i]) m[i] = g_disco(i, nome[i], n);   /* o tamanho é o do pedido; g_disco decide o mundo */
    return m[i];
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
static const char *estilo_texto(long *n){
    static char *buf;      /* sem inicializador: nasce zero (slot no tradutor, .bss no nativo) */
    static long  ln;
    static int   lido;
    if(!lido){
        lido = 1;
        /* o estilo entra pela COSTURA (g_carrega), no slot 0 (tex_estilo.bin) --- sem malloc
         * próprio: nativo faz fopen+fread para o slot, wasm aponta-o ao slot pré-carregado. */
        long r = g_carrega("../estilo.tex", 0, 1 << 16);
        if(r < 0) r = g_carrega("estilo.tex", 0, 1 << 16);
        if(r >= 0){ buf = disco_buf(0, 1 << 16); ln = r; }
    }
    if(n) *n = ln;
    return buf;
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
    {"times",0xB4,1},{"cdot",0xD7,1},{"pm",0xB1,1},{"div",0xB8,1},
    {"le",0xA3,1},{"leq",0xA3,1},{"ge",0xB3,1},{"geq",0xB3,1},{"ne",0xB9,1},{"neq",0xB9,1},
    {"equiv",0xBA,1},{"approx",0xBB,1},{"sim",0x7E,1},{"propto",0xB5,1},
    {"in",0xCE,1},{"notin",0xCF,1},{"subset",0xCC,1},{"subseteq",0xCD,1},{"supset",0xC9,1},
    {"cup",0xC8,1},{"cap",0xC7,1},{"emptyset",0xC6,1},{"forall",0x22,1},{"exists",0x24,1},
    {"to",0xAE,1},{"rightarrow",0xAE,1},{"mapsto",0xAE,1},{"leftarrow",0xAC,1},
    {"Rightarrow",0xDE,1},{"Leftarrow",0xDC,1},{"leftrightarrow",0xAB,1},{"Leftrightarrow",0xDB,1},
    {"infty",0xA5,1},{"partial",0xB6,1},{"nabla",0xD1,1},{"sqrt",0xD6,1},
    {"sum",0xE5,1},{"prod",0xD5,1},{"int",0xF2,1},
    {"langle",0xE1,1},{"rangle",0xF1,1},{"oplus",0xC5,1},{"otimes",0xC4,1},
    {"perp",0x5E,1},{"angle",0xD0,1},{"cong",0x40,1},{"aleph",0xC0,1},
    {"star",0x2A,1},{"circ",0xB0,1},{"bullet",0xB7,1},{"ldots",0xBC,1},{"dots",0xBC,1},
    {"Re",0xC2,1},{"Im",0xC1,1},{"wp",0xC3,1},{"neg",0xD8,1},{"wedge",0xD9,1},{"vee",0xDA,1},
    /* e os que são da própria latina */
    {"{",'{',0},{"}",'}',0},{"$",'$',0},{"%",'%',0},{"&",'&',0},{"_",'_',0},{"#",'#',0},
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
#define MAX_CARTA 8
static Ttf CARTAS[MAX_CARTA];
static const char *CARTA_NOME[MAX_CARTA];
static int N_CARTA = 0;
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
#define DES_C   ((Ttf*)disco_buf(10, (long)(MAX_DES * sizeof(Ttf))))
static double DES_CORPO[MAX_DES];
static int DES_VAR[MAX_DES], N_DES = 0;
static double CORPO_CORRENTE = 0;

/* a carta para esta variante e este corpo, abrindo-a se ainda não estiver aberta */
static const Ttf *carta_do_corpo(int variante, double corpo){
    if(corpo <= 0) return &CARTAS[variante < N_CARTA ? variante : 0];
    for(int i = 0; i < N_DES; i++)
        if(DES_VAR[i] == variante && DES_CORPO[i] > corpo - 0.01 && DES_CORPO[i] < corpo + 0.01)
            return &DES_C[i];
    if(N_DES >= MAX_DES) return &CARTAS[variante < N_CARTA ? variante : 0];
    const char *nome = spline_por_corpo(corpo, variante);
    char c1[256], c2[256]; char *pp;
    pp = ap_str(c1, "lib/fontes/");    pp = ap_str(pp, nome); *pp = 0;
    pp = ap_str(c2, "../lib/fontes/"); pp = ap_str(pp, nome); *pp = 0;
    const char *v[2];
    v[0] = c1; v[1] = c2;                  /* o vector local enche-se por frases: só a
                                            * imagem leva agregados, e c1/c2 são de agora */
    if(!spline_abre_alguma(&DES_C[N_DES], v, 2, NULL))
        return &CARTAS[variante < N_CARTA ? variante : 0];
    DES_CORPO[N_DES] = corpo; DES_VAR[N_DES] = variante; N_DES++;
    return &DES_C[N_DES - 1];
}                            /* 0 = tabela, 1 = curva */

static void carta_abre(void){
    static int tentado = 0;
    if(tentado) return;
    tentado = 1;
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
    case 0x80: return 0x20AC;  case 0x82: return 0x201A;  case 0x83: return 0x0192;
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
static int largura_de(int g, int fonte, double corpo){
    double guarda = CORPO_CORRENTE;
    CORPO_CORRENTE = corpo;
    int w = largura(g, fonte);
    CORPO_CORRENTE = guarda;
    return w;
}
/* a largura de um glifo em MILÉSIMOS de em, da carta da fonte: avanço · 1000 / upem. É a conta que
 * se repetia por todo o lado (a régua da fonte); agora uma só. */
static int avanco_mil(const Ttf *t, int gi){ return (int)((long)ttf_avanco(t, gi) * 1000 / t->upem); }

static int largura(int g, int fonte){
    /* O ESPACO E' UMA LETRA, e tem caixa como qualquer outra: mede 277 na Liberation e 260
     * noutra fonte — nao e' um vazio sem largura nem um numero a parte. Por isso vem da FONTE,
     * como o `o` vem, e nao de uma constante.
     *
     * E era aqui que ele se perdia: `if(fonte == F_SIM) return 549` devolvia 549 para TUDO na
     * Symbol, incluindo o espaco. O espaco da Symbol mede 250, e eu dava-lhe 549 — mais do
     * dobro. Uma constante para uma fonte inteira e' o chute outra vez, com outro rosto. */
    if(fonte == F_SIM){
        carta_abre();
        /* o espaco existe em qualquer fonte, e a largura dele le-se */
        if(g == ' ' && CARTA) return avanco_mil(&CARTA_R, ttf_glifo(&CARTA_R, ' '));
        /* e os simbolos: a largura PUBLICADA da Symbol, e nao um numero para todos */
        return (g >= 32 && g <= 255) ? W_SIM[g - 32] : 0;
    }
    if(g == '\n' || g == '\r' || g == '\t') return 0;   /* não são glifos: não se medem */
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
typedef struct { unsigned char g; unsigned char f; } Gl;

#define MAXLIN 4096
/* A LINHA LEVA A SUA LARGURA. O `desenrola` justificava sempre contra COL — a largura da
 * PÁGINA — e numa célula de tabela isso estica-a até à margem direita, invadindo as colunas
 * seguintes. Ele não conhece a tabela nem tem de conhecer: recebe a largura com a linha. */
typedef struct { Gl g[MAXLIN]; int n; int nivel; int recuo; int larg; long deg; int centra; } Linha;

static long mede(const Gl *g, int n, double corpo){   /* a largura da linha, em milésimos de ponto */
    long w = 0;
    for(int i = 0; i < n; i++) w += (long)largura(g[i].g, g[i].f) * corpo;
    return w;
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
static double unidade_pt(const char *u){
    if(!strcmp(u, "cm")) return 28.3465;
    if(!strcmp(u, "mm")) return 2.83465;
    if(!strcmp(u, "in")) return 72.0;
    if(!strcmp(u, "ex")) return 4.5;
    if(!strcmp(u, "em")) return 10.5;
    return 1.0;                                   /* pt (ou vazio): o ponto é a unidade base */
}
/* lê "Nunidade" (ex. "6mm", "3pt") e devolve o valor EM PONTOS, ou -1 se não parseou / não positivo.
 * O `sscanf %lf%2[a-z]` sai por str2dbl (lib/le_num.h) + a leitura de até 2 minúsculas (a unidade). */
static double medida_pt(const char *str){
    const char *end;
    double v = str2dbl(str, &end);
    if(end == str || v <= 0) return -1;                      /* sem número, ou não positivo: como antes */
    char u[8]; int k = 0;                                    /* %2[a-z]: até duas minúsculas */
    while(k < 2 && end[k] >= 'a' && end[k] <= 'z'){ u[k] = end[k]; k++; }
    u[k] = 0;
    return v * unidade_pt(u);
}

/* A MARGEM é uma VARIÁVEL (o carrega_config enche-a), não uma chamada ao parser: o núcleo lê o
 * valor, não o sscanf. margem_estilo (o parser) fica do lado do wrapper. */
static long MARGEM_V = 64;
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
    unsigned char *buf;                            /* o slot no disco: «o ficheiro É o vector» */
    long cur;                                      /* o cursor de escrita (o ftell) */
    long len;                                      /* o maior byte escrito — o comprimento lógico */
    long cap;                                      /* a capacidade do slot */
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
    double caixa_y;                                /* onde a caixa abriu; <0 = nenhuma aberta */
    long   caixas, reguas;                         /* o que se desenhou, para se poder contar */
    long   n_fundo;                                /* quantas operações lá foram */
    int    abriu_agora;    /* o `desenrola` abriu página? A tabela precisa de saber: o seu
                            * `tab_y` fica a apontar para a página anterior, e a célula
                            * seguinte nasceria fora do papel. */
    int    fo, flo;                                /* os objectos do stream do fundo */
} Pdf;

/* os cinco da costura: escrevem no SLOT (buf[cur]), sem libc --- o núcleo não vê o FILE*. */
static void s_byte(Saida *s, int c);   /* usado pelo formatador abaixo, definido logo a seguir */
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
static void s_fmt(Saida *s, const char *fmt, ...){
    va_list ap; va_start(ap, fmt);
    for(const char *q = fmt; *q; q++){
        if(*q != '%'){ s_byte(s, (unsigned char)*q); continue; }
        q++;
        char pad = ' '; int width = 0;
        if(*q == '0'){ pad = '0'; q++; }
        while(*q >= '0' && *q <= '9'){ width = width * 10 + (*q - '0'); q++; }
        int lng = 0; while(*q == 'l'){ lng++; q++; }
        switch(*q){
            case '%': s_byte(s, '%'); break;
            case 'c': s_byte(s, va_arg(ap, int)); break;
            case 's': { const char *str = va_arg(ap, const char*); while(*str) s_byte(s, (unsigned char)*str++); } break;
            case 'd': case 'i':
                if(lng) s_num(s, va_arg(ap, long), width, pad);
                else    s_num(s, (long)va_arg(ap, int), width, pad);
                break;
            default: break;
        }
    }
    va_end(ap);
}
static void s_bytes(Saida *s, const void *b, long n){
    if(s->cur + n <= s->cap){ memcpy(s->buf + s->cur, b, (size_t)n); s->cur += n; if(s->cur > s->len) s->len = s->cur; }
}
static void s_byte (Saida *s, int c){
    if(s->cur < s->cap){ s->buf[s->cur++] = (unsigned char)c; if(s->cur > s->len) s->len = s->cur; }
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
/* o %.2f de um ponto-double (as caixas ainda entram em double; a régua da caixa é a Etapa da
 * geometria). Arredonda ao centésimo e imprime por inteiros. */
static void s_c(Saida *s, double v){ s_fix(s, (long)(v * 100 + (v >= 0 ? 0.5 : -0.5)), 2); }

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
typedef struct { double corpo, entre; } Degrau;
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
static double CAPA_ALT = 0;   /* a altura MEDIDA da capa, na 2.ª passagem */   /* o y onde a capa começou, para medir a altura real */
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
static double CLASSE_CORPO = 0, CLASSE_ENTRE = 0;


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

#define EIXO_ESCALA  1
#define EIXO_ESPACO  -1
#define EIXO_LARGURA 0

static double escala_de_degrau(long degrau, int eixo);
static int    largura_de(int g, int fonte, double corpo);

/* A OPERAÇÃO. Uma só, e o corpo é CAMPO — como o `MOVE` do corpo-estelar, onde «a mesma
 * instrução serve 500 corpos diferentes sem uma instrução nova, porque o corpo é campo e
 * não opcode». O eixo é o próprio parâmetro: ESCALA e ESPACO são a mesma leitura, campos duais. */
static double medida(Corpo c, int eixo, long glifo){
    if(eixo == EIXO_LARGURA)
        return (double)largura_de((int)glifo, c.var, escala_de_degrau(c.deg, EIXO_ESCALA));
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
static double escala_corpo(long deg){ Corpo c; c.var = F_REG; c.deg = deg; return medida(c, EIXO_ESCALA, 0); }
static double escala_entre(long deg){ Corpo c; c.var = F_REG; c.deg = deg; return medida(c, EIXO_ESPACO, 0); }

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
static double FPDF_CORPO[MAX_FPDF];
static int N_FPDF = 0;

static int fpdf_regista(int variante, double corpo){
    int k;
    if(variante == F_SIM) k = 48;
    else {
        long d = 0; double dmin = 1e9;
        for(long t = 0; t < N_ESCALA; t++){
            double x = ESCALA[t].corpo - corpo; if(x < 0) x = -x;
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
 * O `corpo-estelar.tex` §renorm: «renormalizar é mudar a régua sem mudar o objecto» e «a
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
static double razao_escala(void){
    if(N_ESCALA < 3) return 1.17398;              /* φ^(1/3) */
    /* a razão MEDE-SE nos degraus que existem, e não se escreve: se o estilo mudar de
     * família metálica, ela muda com ele. Toma-se a mediana das razões consecutivas, que
     * não se move com um salto duplo no meio. */
    double r[16]; int nr = 0;
    for(long t = 1; t < N_ESCALA && nr < 16; t++)
        if(ESCALA[t-1].corpo > 0) r[nr++] = ESCALA[t].corpo / ESCALA[t-1].corpo;
    for(int a = 1; a < nr; a++)
        for(int b = a; b > 0 && r[b] < r[b-1]; b--){ double x = r[b]; r[b] = r[b-1]; r[b-1] = x; }
    return nr ? r[nr/2] : 1.17398;
}

/* o corpo do degrau k, DERIVADO: base · σ^k. Existe para qualquer k, inclusive os que o
 * estilo não declara — que é o caso do `\part`. */
static double corpo_derivado(double k){
    if(N_ESCALA <= 0) return 10.0;
    double s = razao_escala(), r = ESCALA[0].corpo;
    for(int t = 0; t < (int)k; t++) r *= s;
    if(k > (int)k) r *= 1.0 + (s - 1.0) * (k - (int)k);
    return r;
}

/* O CORPO E A ENTRELINHA SÃO UMA OPERAÇÃO, o eixo escolhe o campo --- eram duas funções com o mesmo
 * clamp (a redundância que escondia a assinatura). A Lei 1: os dois lados da escala (o corpo que
 * multiplica, a entrelinha que soma) lidos da MESMA tabela pelo MESMO caminho.
 *   O TEXTO CORRIDO é o `\normalsize` da classe, não o degrau `gktexto` da escala (esse só na capa);
 *   os outros são os do estilo, porque é o `\titleformat` que os pede. */
static double escala_de_degrau(long degrau, int eixo){
    double classe = (eixo == EIXO_ESCALA) ? CLASSE_CORPO : CLASSE_ENTRE;   /* enchidos pelo carrega_config */
    if(degrau == D_TEXTO){ if(classe > 0) return classe; }
    if(N_ESCALA <= 0) return (eixo == EIXO_ESCALA) ? 10.0 : 14.0;          /* sem estilo: o que havia */
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
static long degrau_de(double corpo);
static void espaco_titulo(const char *cmd, long deg, double *antes, double *depois);
static const char *cor_do_comando(const char *cmd);
static int regua_do_comando(const char *cmd, double *esp, char *cor, size_t nc);
static void poe_regua(Pdf *p, double x1, double x2, double y, double esp, const char *cor);
static int cor_de(const char *nome, double *r, double *g, double *b);
/* o degrau do capítulo LÊ-SE do `\titleformat{\chapter}` em vez de ser o `D_CAP` fixo.
 * MEDIDO: o estilo manda `\gktit` (23,42) e a ida compunha a 16,99 — e `section` e
 * `subsection` batiam, só o capítulo é que não. Foi a VOLTA que o apanhou: ela lia 1 bloco
 * no degrau do título onde o documento tem 148 capítulos. */
static long d_cap(void){ long d = degrau_do_comando("chapter"); return d >= 0 ? d : D_CAP; }
#define D_TIT   6

static int cor_de(const char *nome, double *r, double *g, double *b){
    for(long i = 0; i < N_CORES; i++)
        if(!strcmp(CORES[i].nome, nome)){
            *r = CORES[i].r / 255.0; *g = CORES[i].g / 255.0; *b = CORES[i].b / 255.0;
            return 1;
        }
    return 0;
}

/* um retângulo preenchido: caminho fechado + f — E VAI PARA O STREAM DO FUNDO.
 * É a diferença que faz o fundo poder existir: escrito no primeiro stream, ele pinta ANTES
 * do texto por muito que se escreva depois. A barra também vem por aqui — ela nunca precisou,
 * porque vive na margem, mas não há razão para ter dois caminhos onde um serve. */
static void poe_rect(Pdf *p, double x, double y, double w, double h, const char *cor){
    double r, g, b;
    if(!p->aberta || !p->fundo_on || !cor_de(cor, &r, &g, &b)) return;
    Saida *s = &p->sfundo;
    s_fmt(s, "q "); s_fix(s,(long)(r*1000+0.5),3); s_byte(s,' '); s_fix(s,(long)(g*1000+0.5),3);
    s_byte(s,' '); s_fix(s,(long)(b*1000+0.5),3); s_fmt(s, " rg ");
    s_c(s,x);   s_byte(s,' '); s_c(s,y);   s_fmt(s," m ");
    s_c(s,x+w); s_byte(s,' '); s_c(s,y);   s_fmt(s," l ");
    s_c(s,x+w); s_byte(s,' '); s_c(s,y+h); s_fmt(s," l ");
    s_c(s,x);   s_byte(s,' '); s_c(s,y+h); s_fmt(s," l f Q\n");
    p->n_fundo = p->n_fundo + 1;
    p->caixas = p->caixas + 1;
}

/* uma régua: dois pontos, traçado. Grau 1 — não tem par, é transporte. */
static void poe_regua(Pdf *p, double x1, double x2, double y, double esp, const char *cor){
    double r, g, b;
    if(!p->aberta || !cor_de(cor, &r, &g, &b)) return;
    Saida *s = &p->sf;
    s_fmt(s, "q "); s_fix(s,(long)(r*1000+0.5),3); s_byte(s,' '); s_fix(s,(long)(g*1000+0.5),3);
    s_byte(s,' '); s_fix(s,(long)(b*1000+0.5),3); s_fmt(s, " RG ");
    s_c(s,esp); s_fmt(s," w "); s_c(s,x1); s_byte(s,' '); s_c(s,y); s_fmt(s," m ");
    s_c(s,x2); s_byte(s,' '); s_c(s,y); s_fmt(s," l S Q\n");
    p->reguas = p->reguas + 1;
}

static int obj_novo(Pdf *p){
    p->nobj = p->nobj + 1; p->off[p->nobj] = s_pos(&p->sf);
    return p->nobj;
}

static void pdf_abre(Pdf *p, unsigned char *buf, long cap){
    memset(p, 0, sizeof *p);
    p->off = (long*)disco_buf(12, (long)(MAXOBJ * sizeof(long)));
    p->pag = (int *)disco_buf(13, (long)(MAXOBJ * sizeof(int)));
    p->sf.buf = buf; p->sf.cur = 0; p->sf.len = 0; p->sf.cap = cap;   /* a saída é um slot+cursor */
    s_fmt(&p->sf, "%%PDF-1.4\n%%\xE2\xE3\xCF\xD3\n");
    p->nobj = 2 + N_FIXA;                      /* 1 catálogo, 2 páginas, e N_FIXA fontes */
}

/* abre um stream cujo /Length é INDIRECTO (aponta um objecto que se fecha depois): regista o offset,
 * escreve o cabeçalho, e devolve o início do stream. Era a mesma sequência para o conteúdo e o fundo. */
static long abre_stream_ind(Pdf *p, int obj, int lenobj){
    p->off[obj] = s_pos(&p->sf);
    s_fmt(&p->sf, "%d 0 obj<</Length %d 0 R>>stream\n", obj, lenobj);
    return s_pos(&p->sf);
}

static void pagina_abre(Pdf *p){
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
    s_fmt(&p->sf, "%d 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 ", po);
    s_fix(&p->sf, A4_LM, 3); s_byte(&p->sf, ' '); s_fix(&p->sf, A4_AM, 3);
    s_fmt(&p->sf, "]/Resources<</Font<<%s>>>>/Contents[%d 0 R %d 0 R]>>endobj\n", dicf, fo, co);
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
    s_fmt(&p->sf, "endstream\nendobj\n");
    p->off[lenobj] = s_pos(&p->sf);
    s_fmt(&p->sf, "%ld 0 obj %ld endobj\n", lenobj, len);
}

static void pagina_fecha(Pdf *p){
    if(!p->aberta) return;
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
static void poe_pedaco(Saida *f, const Gl *g, int i, int j, int fonte, double corpo,
                       long x_m, long y_m, long espaco_extra){
    /* cinco: regular, negra, Symbol, itálica, versaletes — a torre em bits */
    /* o nome da fonte vem do PAR (variante, corpo), nao da variante sozinha */
    char nomef[16];
    { char *q = ap_str(nomef, "/F"); q = ap_num(q, fpdf_regista(fonte, corpo) + 1); *q = 0; }
    /* a cor do texto: `rg` no operador de preenchimento, dentro do BT. Sem isto tudo saía
     * preto, mesmo com o estilo a declarar `tinta`, `ouro` e `regua`. */
    { double r, gg, b;
      if(COR_TEXTO[0] && cor_de(COR_TEXTO, &r, &gg, &b)){
          s_fix(f, (long)(r*1000+0.5), 3); s_byte(f, ' ');
          s_fix(f, (long)(gg*1000+0.5), 3); s_byte(f, ' ');
          s_fix(f, (long)(b*1000+0.5), 3); s_fmt(f, " rg ");
      } else s_fmt(f, "0 0 0 rg "); }
    /* O `Td` ESCREVE EM MILESIMOS, e nao em centesimos. A conta corre em milesimos de
     * ponto e o ficheiro guardava dois decimais: o que ficava abaixo era APAGADO, e apagar
     * nao se desfaz. MEDIDO: 325 de 400 posicoes nao cabiam, com deriva de 0,00365 pt em
     * dez glifos — e acumula, porque espacar SOMA.
     *
     * Tres decimais nao sao um numero escolhido: sao os que a conta tem. O PDF aceita-os,
     * e o que ele aceita e o que se mede passam a ser a mesma regua. */
    s_fmt(f, "BT %s ", nomef); s_fix(f, (long)(corpo*1000+0.5), 3);
    s_fmt(f, " Tf "); s_fix(f, x_m, 3); s_byte(f, ' '); s_fix(f, y_m, 3); s_fmt(f, " Td");
    /* O Tw ESCREVE-SE SEMPRE, mesmo quando é zero — e era isto.
     *
     * O `Tw` é estado do texto e PERSISTE no stream: não se repõe entre BT/ET nem entre
     * páginas. Eu só o escrevia quando havia justificação, e todos os pedaços seguintes
     * HERDAVAM o do último pedaço justificado. Uma linha sem Tw nenhum ficava a andar 1,59 pt
     * a mais por espaço — e o desvio ACUMULA: à oitava palavra eram 11,11 pt, mais do que uma
     * palavra inteira, e a seguinte caía por cima.
     *
     * E é por isso que UNS espaços somem e outros não: o erro cresce ao longo da linha, e só
     * passa a ver-se quando ultrapassa a largura de um espaço.
     *
     * A LEI: o passo tem de ser reversível, e um estado que só se LIGA e nunca se desliga não
     * o é. Já me tinha acontecido neste ficheiro com o modo matemático — «um estado que só se
     * liga apaga o que vem depois, e o dano não aparece onde nasce». É a mesma frase, e eu
     * escrevi-a lá em cima. */
    s_fmt(f, " "); s_fix(f, espaco_extra, 3); s_fmt(f, " Tw");
    s_fmt(f, " (");
    for(int k = i; k < j; k++){
        int c = g[k].g;
        if(c == '(' || c == ')' || c == '\\') s_byte(f, '\\');
        if(c < 32) c = ' ';
        s_byte(f, c);
    }
    s_fmt(f, ") Tj ET\n");
}

/* o LUNAR desenrola uma linha na página, deformando o espaço se for para justificar */
/* desenha uma linha numa POSIÇÃO dada, e desce o `y` só se `desce` — o sumário precisa de
 * pôr o texto à esquerda e o número à direita NA MESMA linha, e o `desenrola` normal desce
 * sempre. Sem isto o número caía na linha seguinte. */
static void desenrola_em(Pdf *p, const Linha *L, double x0, int desce){
    if(!L->n){ if(desce) p->y -= (long)(escala_entre(D_TEXTO)*1000 + 0.5); return; }
    /* E ABRE PAGINA quando nao cabe — o `desenrola` normal fa-lo e este nao fazia: o
     * sumario inteiro caia numa pagina so', com o `y` a descer para negativo. O gabarito
     * gasta NOVE paginas com ele. */
    if(p->y < MARGEM*PT + (long)(escala_entre(D_TEXTO)*1000 + 0.5)){ pagina_fecha(p); pagina_abre(p); }
    double corpo = escala_corpo(L->deg >= 0 ? L->deg : D_TEXTO);
    CORPO_CORRENTE = corpo;
    /* O X ACUMULA EM MILÉSIMOS INTEIROS, não em pontos-double: `x += w/1000.0` arrastava um erro
     * sub-milésimo que SOMA ao longo da linha (o teorema da medição: um pequeno erro propaga-se, e
     * o estado que o carrega é o acumulador). Em inteiro a soma é exacta — sem deriva, resíduo 0. */
    int i = 0; long xm = (long)(x0 * 1000.0 + (x0 >= 0 ? 0.5 : -0.5));
    while(i < L->n){
        int j = i, fonte = L->g[i].f;
        while(j < L->n && L->g[j].f == fonte) j++;
        poe_pedaco(&p->sf, L->g, i, j, fonte, corpo, xm, p->y, 0);
        long w = 0;
        for(int k = i; k < j; k++) w += (long)largura(L->g[k].g, fonte) * corpo;
        xm += w;
        i = j;
    }

    if(desce) p->y -= (long)(escala_entre(D_TEXTO)*1000 + 0.5);
}

static void desenrola(Pdf *p, const Linha *L, int justifica){
    /* NUMA CÉLULA NENHUMA linha justifica, e não só a última: numa coluna `l` o alinhamento é
     * à esquerda em todas. A justificação é do parágrafo, e uma célula não é um parágrafo. */
    if(L->larg > 0) justifica = 0;
    /* O CORPO E A ENTRELINHA SAEM DA ESCALA, e o nível escolhe o degrau. Antes eram 10 e 14
     * escritos à mão — e a razão 1,4 em vez de 1,4497, sem hierarquia nenhuma. */
    if(!L->n){ p->y -= (long)(escala_entre(D_TEXTO)*1000 + 0.5); return; }
    /* o + 0,5 tem de estar FORA do ternário: escrito (int)(cond ? A : B + 0.5) ele só apanha
     * um dos ramos, e 16,99 truncava para 16 em vez de arredondar para 17. Um parêntese. */
    /* SEM `(int)` e sem o `+0,5`: a escala é GEOMÉTRICA de razão φ^(1/3)=1,1740, e arredondar
     * para inteiro destrói isso — medido, as razões passavam a 1,1111 … 1,2143, um desvio de
     * 5,4%%. E não havia razão nenhuma para arredondar: o `Tf` do PDF aceita fracções. */
    double corpo = ((L->deg >= 0 ? escala_corpo(L->deg)
                     : L->nivel   ? escala_corpo(L->nivel <= 1 ? d_cap()
                                             : (L->nivel == 2 ? D_SEC : D_SUB))
                                : escala_corpo(D_TEXTO)));
    CORPO_CORRENTE = corpo;   /* o desenho segue o corpo, e quem o sabe é quem compõe */
    /* e a ALTURA DA LINHA sai da mesma escala: entrelinha/corpo = 1,4497 em TODOS os degraus
     * do estilo.tex, e era 1,4 aqui. A diferença é pequena e é o que faz o texto parecer
     * apertado — a entrelinha é o que dá ar à página. */
    long alt  = (long)(((L->nivel ? escala_entre(L->nivel <= 1 ? D_CAP
                                              : (L->nivel == 2 ? D_SEC : D_SUB))
                                : escala_entre(D_TEXTO))) * 1000 + 0.5);

    if(!p->aberta || p->y - alt < FUNDO){ pagina_fecha(p); pagina_abre(p); }
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
    p->y -= alt;

    long xm = (MARGEM + L->recuo) * 1000L;         /* o x em MILÉSIMOS inteiros, exacto */
    /* CENTRAR: o `\begin{center}` do corpo do `\gkcapa` --- a linha não justifica, desloca-se
     * para o meio da coluna. Sem isto a capa saía toda encostada à margem esquerda. */
    if(L->centra){
        justifica = 0;
        long larg_c = mede(L->g, L->n, corpo);
        long sobra_m = (long)(COL - L->recuo) * 1000L - larg_c;   /* a sobra em milésimos */
        if(sobra_m > 0) xm += sobra_m / 2;
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
        for(int i = 0; i < L->n; i++) if(L->g[i].g == ' ') esp++;
        if(larg < alvo){
            long resto = deforma(alvo - larg, esp, &extra);
            (void)resto;                            /* medido a sério no §X3 */
        }
    }
    /* parte por fonte: cada troca de fonte é um pedaço, porque um Tj só fala uma fonte */
    int i = 0;
    while(i < L->n){
        int j = i, fonte = L->g[i].f;
        while(j < L->n && L->g[j].f == fonte) j++;
        poe_pedaco(&p->sf, L->g, i, j, fonte, corpo, xm, p->y, extra);
        long w = 0;
        for(int k = i; k < j; k++){
            w += (long)largura(L->g[k].g, fonte) * corpo;
            if(L->g[k].g == ' ') w += extra;      /* o espaco e' letra em qualquer fonte */
        }
        xm += w;                                  /* soma exacta em milésimos — sem deriva */
        i = j;
    }
}

static void pdf_fecha(Pdf *p){
    pagina_fecha(p);
    /* o objeto Pages sai no FIM — só agora se sabe quantas páginas há. A xref dá o offset, e a
     * ordem no ficheiro é livre: um objeto pode estar em qualquer parte. */
    p->off[2] = s_pos(&p->sf);
    s_fmt(&p->sf,"2 0 obj<</Type/Pages/Count %d/Kids[", p->npag);
    for(int i = 0; i < p->npag; i++) s_fmt(&p->sf,"%d 0 R ", p->pag[i]);
    s_fmt(&p->sf,"]>>endobj\n");
    p->off[1] = s_pos(&p->sf);
    s_fmt(&p->sf,"1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj\n");
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
        /* EMBUTE-SE A FONTE QUE SE MEDE, e não outra. Eu tinha embutido a NotoSerif e
         * continuava a medir a largura pela Liberation Sans (SPLINE_REG) — e as palavras
         * montavam umas nas outras, porque o avanço vinha de uma fonte e o desenho de outra.
         *
         * É o mesmo defeito de sempre com outro rosto: DUAS RÉGUAS PARA O MESMO OBJECTO. E é
         * exactamente o par que aqui não pode partir-se — o ESPAÇAMENTO soma (a posição avança)
         * e a ESCALA multiplica (o tamanho estica), e os dois têm de vir do MESMO corpo, senão
         * não são duais de nada: são duas medidas de dois objectos diferentes. */
        /* UM FICHEIRO POR VARIANTE, e não um para todos. Embutia-se UM `/FontFile3` e os
         * dezasseis objectos de fonte apontavam para ele: as larguras variavam, o DESENHO
         * não — porque só havia um desenho lá dentro. O sintoma era «Enredo» em minúsculas
         * onde o gabarito tem ENREDO em versaletes, com a fonte certa escolhida e tudo.
         *
         * É a terceira vez hoje com o mesmo rosto: duas réguas para o mesmo objecto. A
         * largura vinha da carta da variante e o traço vinha do ficheiro único. */
        /* E O FICHEIRO EMBUTIDO E' O MESMO QUE MEDE. Eu media a largura pelo desenho do
         * CORPO (d-cc1728 para os versaletes a 16,99) e embutia o de 10pt — as letras
         * saiam com o avanco de um desenho e o traco de outro, e por isso o «ENREDO»
         * tinha o RED colado. Duas reguas, a quarta vez hoje. */
        for(int fi = 0; fi < N_FPDF; fi++){
            unsigned char *ttf = (unsigned char*)disco_buf(3, 1 << 22);
            long nttf = 0;
            int vv = FPDF_VAR[fi] < 4 ? FPDF_VAR[fi] : 0;
            const char *nm = spline_por_corpo(FPDF_CORPO[fi], vv);
            char cam[256], alt[256];
            /* cada fonte é um CORPO, e entra pela mesma porta que o source --- o cruzamento do
             * viveiro: nativo fopen+fread para o slot (o mesmo, reusado uma de cada vez), wasm o
             * host enche-o. O tamanho é o do slot, não o do ponteiro (o sizeof de um ponteiro é 8
             * e embutia a fonte com 8 bytes --- o número que não cabe). */
            { char *pp = ap_str(cam, "lib/fontes/"); pp = ap_str(pp, nm); *pp = 0; }
            nttf = g_carrega(cam, 3, 1 << 22);
            if(nttf < 0){ char *pp = ap_str(alt, "../lib/fontes/"); pp = ap_str(pp, nm); *pp = 0;
                          nttf = g_carrega(alt, 3, 1 << 22); }
            if(nttf <= 0) continue;
            ttf = (unsigned char*)disco_buf(3, 1 << 22);   /* o slot que carrega_ficheiro encheu */
            int otf = nttf > 4 && ttf[0]=='O' && ttf[1]=='T' && ttf[2]=='T' && ttf[3]=='O';
            int of = p->nobj + 1, od = p->nobj + 2;
            p->nobj = od;
            p->off[of] = s_pos(&p->sf);
            if(otf) s_fmt(&p->sf,"%d 0 obj<</Length %ld/Subtype/OpenType>>stream\n", of, nttf);
            else    s_fmt(&p->sf,"%d 0 obj<</Length %ld/Length1 %ld>>stream\n", of, nttf, nttf);
            s_bytes(&p->sf, ttf, nttf);
            s_fmt(&p->sf,"\nendstream\nendobj\n");
            p->off[od] = s_pos(&p->sf);
            s_fmt(&p->sf,"%d 0 obj<</Type/FontDescriptor/FontName/Embutida/Flags 32"
                          "/FontBBox[-1000 -400 2000 1100]/ItalicAngle 0/Ascent 900"
                          "/Descent -200/CapHeight 700/StemV 80/%s %d 0 R>>endobj\n", od,
                    otf ? "FontFile3" : "FontFile2", of);
            EMBP[fi] = od; FONTE_OTF = otf;
        }
        fonte_emb = N_FPDF ? EMBP[0] : 0;
    }
    static const char *BF[3] = {"Helvetica", "Helvetica-Bold", "Symbol"};
    for(int i = 0; i < N_FIXA; i++){
        p->off[3+i] = s_pos(&p->sf);
        /* a variante e o corpo deste indice: os registados durante a composicao, e a base
         * para os que sobram — um objecto que ninguem usa nao faz mal, um que falta faz. */
        int vr = i < N_FPDF ? FPDF_VAR[i] : 0;
        double cp = i < N_FPDF ? FPDF_CORPO[i] : 0;
        (void)cp;
        if(fonte_emb && vr != F_SIM){
            /* AS LARGURAS VÃO NO PDF, e era isto que faltava.
             *
             * Eu declarava /FirstChar 32 /LastChar 255 e NÃO dizia as larguras — e sem /Widths
             * o leitor fica livre para as tirar de onde quiser. Ele tirava-as da fonte, eu
             * media-as aqui, e as duas divergiam. O erro é pequeno por glifo e ACUMULA: a 264 pt
             * do início da linha já eram 8,2 pt — mais do que um espaço inteiro (3,05 pt).
             *
             * E é por isso que UNS espaços somem e outros não: os primeiros da linha ainda têm
             * acumulado pequeno de mais para se ver; o que está a meio já perdeu um espaço
             * inteiro, e a palavra seguinte cai por cima da anterior.
             *
             * Com /Widths o leitor usa EXACTAMENTE as larguras com que eu posicionei. Deixa de
             * haver duas réguas: é a mesma, escrita no ficheiro. */
            s_fmt(&p->sf,"%d 0 obj<</Type/Font/Subtype/%s/BaseFont/Embutida"
                          "/FirstChar 32/LastChar 255/FontDescriptor %ld 0 R"
                          "/Encoding/WinAnsiEncoding/Widths[", 3+i, FONTE_OTF ? "Type1" : "TrueType",
                    EMBP[i] ? EMBP[i] : fonte_emb);
            /* E AS LARGURAS SÃO AS DA FONTE EMBUTIDA, não as de uma tabela. Aqui estava
             * `largura_tabela`, que é a tábua da Helvetica — eu embutia a fonte do
             * documento e escrevia no `/Widths` as medidas de outra. Duas réguas outra vez,
             * e desta vez dentro do mesmo objecto do PDF. */
            /* e as larguras SAO AS DESTE PAR: a mesma fonte a corpos diferentes tem
             * larguras diferentes, porque o desenho e' outro. */
            CORPO_CORRENTE = cp;
            for(int c = 32; c <= 255; c++)
                s_fmt(&p->sf,"%d%s", largura(c, vr), c < 255 ? " " : "");
            s_fmt(&p->sf,"]>>endobj\n");
        }
        else
            s_fmt(&p->sf,"%d 0 obj<</Type/Font/Subtype/Type1/BaseFont/%s%s>>endobj\n",
                    3+i, BF[vr < 3 ? vr : 0], vr == F_SIM ? "" : "/Encoding/WinAnsiEncoding");
    }
    /* O .TEX ORIGINAL, INVISÍVEL: um objecto que a página não referencia. O leitor de PDF
     * ignora-o — não está em /Contents nem em árvore nenhuma —, mas está lá, e a volta lê-o
     * pelo marcador /Type/FonteTeX. É a metade que a estrela guarda para não apagar. */
    if(FONTE_TEX){
        /* o SOURCE é um corpo, e entra pela mesma porta que as fontes --- o cruzamento do viveiro:
         * carrega-se para o slot 4 (nativo fopen+fread, wasm pré-carregado) e transmite-se ao PDF. */
        long len = g_carrega(FONTE_TEX, 4, 1 << 22);
        if(len > 0){
            unsigned char *src = (unsigned char*)disco_buf(4, 1 << 22);
            int obj = p->nobj + 1; p->nobj = obj;
            p->off[obj] = s_pos(&p->sf);
            s_fmt(&p->sf,"%d 0 obj<</Type/FonteTeX/Length %ld>>stream\n", obj, len);
            s_bytes(&p->sf, src, len);                  /* o corpo inteiro, de uma vez */
            s_fmt(&p->sf,"\nendstream\nendobj\n");
        }
    }
    long xref = s_pos(&p->sf);
    s_fmt(&p->sf,"xref\n0 %d\n0000000000 65535 f \n", p->nobj + 1);
    for(int i = 1; i <= p->nobj; i++) s_fmt(&p->sf,"%010ld 00000 n \n", p->off[i]);
    s_fmt(&p->sf,"trailer<</Size %d/Root 1 0 R>>\nstartxref\n%ld\n%%%%EOF\n", p->nobj + 1, xref);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * A DESCIDA sobre o .tex — e é a única passagem pelo texto
 * ───────────────────────────────────────────────────────────────────────────── */

typedef struct {
    Pdf *p;
    Linha L;
    int  fonte;            /* a fonte corrente */
    int  mat;              /* dentro de $...$ */
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

static void empurra(Est *e, int g, int f){
    if(e->L.n == 0){ e->L.deg = DEG_FORCADO; e->L.centra = CENTRA; }
    if(e->L.n < MAXLIN - 1){ e->L.g[e->L.n].g = (unsigned char)g; e->L.g[e->L.n].f = (unsigned char)f; e->L.n++; }
    e->glifos = e->glifos + 1;
}
/* um espaço, mas só se o último glifo não é já um espaço --- evita o espaço dobrado. Estava escrito
 * três vezes por extenso. */
static void espaco_se_falta(Est *e){ if(e->L.n && e->L.g[e->L.n-1].g != ' ') empurra(e, ' ', e->fonte); }

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
    double corpo = ((e->L.deg >= 0 ? escala_corpo(e->L.deg)
                     : e->L.nivel ? escala_corpo(e->L.nivel <= 1 ? d_cap()
                                  : (e->L.nivel == 2 ? D_SEC : D_SUB))
                                  : escala_corpo(D_TEXTO)));
    CORPO_CORRENTE = corpo;   /* medir com o desenho DESTE corpo, e nao de outro */
    CORPO_CORRENTE = corpo;
    /* A LARGURA DISPONÍVEL É A DA COLUNA, dentro de uma tabela — e não a da página.
     *
     * Sem isto a célula quebrava só ao chegar à margem direita, logo transbordava para a
     * coluna seguinte e a palavra da coluna ao lado ficava por baixo. As invasões subiram de
     * 0 para 2841 no catálogo assim que a tabela passou a compor — e o defeito não era a
     * tabela: era a quebra a usar a régua errada. Dentro de uma coluna a régua é a coluna. */
    if(e->tab) e->L.larg = (int)(e->tab_larg - 6);   /* a goteira entre colunas */
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
        int corte = e->L.n, ate = 0; long w = 0; int corta_palavra = 0;
        for(int i = 0; i < e->L.n; i++){
            w += (long)largura(e->L.g[i].g, e->L.g[i].f) * corpo;
            if(w > alvo){ corte = ate ? ate : i; break; }
            if(e->L.g[i].g == ' ') ate = i;
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
                long base = 0;
                for(int k2 = 0; k2 < ini; k2++)
                    base += (long)largura(e->L.g[k2].g, e->L.g[k2].f) * corpo;
                long wh = (long)largura('-', e->L.g[ini].f) * corpo;
                int melhor = -1;
                for(int t = 0; t < ns; t++){
                    long w2 = base;
                    for(int k2 = ini; k2 < pos[t]; k2++)
                        w2 += (long)largura(e->L.g[k2].g, e->L.g[k2].f) * corpo;
                    if(w2 + wh <= alvo) melhor = pos[t];      /* o maior que ainda cabe */
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
        Linha out = e->L; out.n = corte;
        while(out.n && out.g[out.n-1].g == ' ') out.n--;
        /* o hífen do corte: a palavra partida leva o sinal de que continua */
        if(corta_palavra && out.n < MAXLIN){ out.g[out.n].g = '-';
                                             out.g[out.n].f = out.g[out.n-1].f; out.n++; }
        int fim = (corte == e->L.n);
        desenrola(e->p, &out, !(fim && ultima) && !e->L.nivel);
        if(fim){ e->L.n = 0; break; }
        int k = corte;
        if(!corta_palavra) while(k < e->L.n && e->L.g[k].g == ' ') k++;
        memmove(e->L.g, e->L.g + k, (size_t)(e->L.n - k) * sizeof(Gl));
        e->L.n -= k;
    }
}

static void fecha_paragrafo(Est *e){
    if(e->L.n) quebra_e_desenrola(e, 1);
    e->L.n = 0; e->L.nivel = 0; e->L.recuo = e->recuo;
}

static void compila(const char *s, Pdf *p, long *glifos){
    Est e; memset(&e, 0, sizeof e);
    e.p = p; e.fonte = F_REG;
    long i = 0, n = (long)strlen(s);

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
                fecha_paragrafo(&e); p->y -= 5*PT;
                /* o TeX tambem nao deixa uma formula atravessar paragrafo ("Missing $ inserted").
                 * Sem isto, UM cifrao desirmanado apaga o resto do documento — e foi exatamente
                 * o que aconteceu. Fechar aqui limita o dano de qualquer $ solto a um paragrafo. */
                e.mat = 0; e.fonte = F_REG;
                i = j; continue;
            }
            espaco_se_falta(&e);
            i = j; continue;
        }
        /* O `&` MUDA DE COLUNA — e é só isso: fecha a célula onde está e abre a seguinte.
         * Fora de tabela é um caractere como outro qualquer, e por isso ia parar à página. */
        if(c == '&' && e.tab){
            /* A CÉLULA NÃO SE JUSTIFICA. O `0` aqui dizia «não é a última linha», e o efeito
             * é justificar: as palavras espalham-se até à margem da coluna, e uma célula de
             * duas palavras sai com um vão enorme no meio — «o ... Operador», «tudo se move
             * em volta». Numa coluna `l` o texto alinha à esquerda e acabou.
             *
             * O comentário que eu tinha escrito ao lado dizia «sem justificar» — e o código
             * fazia o contrário. Escrevi a intenção e passei o argumento oposto. */
            if(e.L.n) quebra_e_desenrola(&e, 1);       /* 1 = última: NÃO justifica */
            e.L.n = 0;
            e.tab_col++;
            if(e.tab_col >= e.tab_ncol) e.tab_col = e.tab_ncol - 1;
            {   long r = 0;
                for(int k = 0; k < e.tab_col && k < 16; k++) r += e.tab_w[k];
                e.L.recuo = (int)r;
                e.tab_larg = e.tab_w[e.tab_col < 16 ? e.tab_col : 15];
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
            if(e.p->abriu_agora){ e.tab_y = e.p->y; e.tab_ymin = e.p->y;
                                  e.tab_pag = e.p->npag; e.p->abriu_agora = 0; }
            if(e.p->npag != e.tab_pag){ e.tab_ymin = e.p->y; e.tab_y = e.p->y;
                                        e.tab_pag = e.p->npag; }
            else if(e.p->y < e.tab_ymin) e.tab_ymin = e.p->y;
            e.p->y = e.tab_y;
            i++; continue;
        }
        /* O `~` E' UM ESPACO — o nao-quebravel do TeX. Sem isto sai como til literal e cola
         * as duas palavras: «Livro~I» em vez de «Livro I». */
        if(c == '~'){
            espaco_se_falta(&e);
            i++; continue;
        }
        if(c == ' ' || c == '\t'){
            espaco_se_falta(&e);
            i++; continue;
        }
        if(c == '$'){
            if(i + 1 < n && s[i+1] == '$'){ i += 2; } else i++;
            e.mat = !e.mat;
            continue;
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
            }
            i++; continue;
        }

        if(c == '\\'){
            long j = i + 1;
            if(j < n && !isalpha((unsigned char)s[j])){  /* \\ , \{ , \% , \_ ... */
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
                    double vsalto = 0;
                    if(q2 < n && s[q2] == '['){
                        double m = medida_pt(s + q2 + 1);
                        if(m >= 0) vsalto = m;
                        while(q2 < n && s[q2] != ']') q2++;
                        if(q2 < n) j = q2;
                    }
                    /* O `\\` FECHA A LINHA DA TABELA: volta à primeira coluna e desce UMA vez.
                     *
                     * Sem isto cada célula descia a sua, e uma tabela de 4 colunas gastava 4
                     * linhas por fila — o texto corrido que se via. A conta é a mesma da página:
                     * o `&` anda em x e o `\\` anda em y. */
                    if(!e.tab && vsalto > 0){ fecha_paragrafo(&e); e.p->y -= (long)(vsalto*1000); }
                    if(e.tab){
                        if(e.L.n) quebra_e_desenrola(&e, 1);   /* idem: a célula não justifica */
                        e.L.n = 0;
                        if(e.p->abriu_agora){ e.tab_ymin = e.p->y; e.tab_pag = e.p->npag;
                                              e.p->abriu_agora = 0; }
                        /* O MÍNIMO NÃO ATRAVESSA A PÁGINA. Se a fila virou de página no meio,
                         * o `tab_ymin` da anterior é menor que QUALQUER y da nova, e o `min`
                         * mantinha-o: a fila seguinte começava no fundo e deixava a página
                         * quase toda em branco. Vi-o na página 3 do enredo — uma linha no
                         * topo, outra no fundo, e um buraco de mil pontos entre elas. */
                        if(e.p->npag != e.tab_pag){ e.tab_ymin = e.p->y; e.tab_pag = e.p->npag; }
                        else if(e.p->y < e.tab_ymin) e.tab_ymin = e.p->y;
                        e.tab_col = 0;
                        e.L.recuo = 0;
                        e.tab_larg = e.tab_w[0];
                        e.p->y = e.tab_ymin;           /* a fila desce o que a mais alta gastou */
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
                        {   long alt_fila = (long)(escala_entre(D_TEXTO) + 0.5);
                            if(e.p->y - (long)(alt_fila*1000) < FUNDO){
                                pagina_fecha(e.p);
                                pagina_abre(e.p);
                                /* e as réguas de topo repetem-se? não: o que se repete é o
                                 * ESPAÇO. Uma régua a mais seria conteúdo inventado. */
                            }
                        }
                        e.tab_y = e.p->y;
                        e.tab_ymin = e.p->y;
                        i = j + 1; continue;
                    }
                    fecha_paragrafo(&e); i = j + 1; continue;
                }
                char um[2]; um[0] = s[j]; um[1] = 0;
                const Par *P = lex_acha(um);
                if(P) empurra(&e, P->glifo, P->simb ? F_SIM : e.fonte);
                else if(s[j] != ',' && s[j] != ' ' && s[j] != '!' && s[j] != ';')
                    empurra(&e, (unsigned char)s[j], e.fonte);
                else empurra(&e, ' ', e.fonte);
                i = j + 1; continue;
            }
            char cmd[64]; int k = 0;
            while(j < n && isalpha((unsigned char)s[j]) && k < 63) cmd[k++] = s[j++];
            cmd[k] = 0;
            while(j < n && (s[j] == '*' )) j++;

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
                fecha_paragrafo(&e);
                { double a = 0, d2 = 0;
                  long dg = degrau_do_comando(cmd);
                  espaco_titulo(cmd, dg, &a, &d2);
                  /* o `antes` do estilo é NEGATIVO no capítulo (-14pt), e no LaTeX isso é
                   * relativo à PÁGINA NOVA que o `\chapter` abre. Sem essa quebra, aplicar o
                   * negativo faz o título subir por cima do texto anterior — foi o que se viu
                   * na página 3. O piso é a entrelinha do próprio degrau: um título nunca
                   * pode invadir o que veio antes, e esse número sai da escala, não daqui. */
                  double piso = (dg >= 0 && dg < N_ESCALA) ? ESCALA[dg].entre * 0.5 : 8.0;
                  p->y -= (long)((a > piso ? a : piso) * 1000); }
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
                        e.fonte = F_REG;
                        for(int t2 = 0; rr[t2]; t2++) empurra(&e, (unsigned char)rr[t2], F_REG);
                        quebra_e_desenrola(&e, 1);
                        e.L.n = 0; DEG_FORCADO = dg;
                        p->y -= 18*PT;
                    }
                }
                int prof = 1; e.L.nivel = nv; e.fonte = F_NEG; e.L.recuo = 0;
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
                            if(s[w] == '{' || s[w] == '}') continue;
                            t->txt[k2++] = s[w];
                        }
                        t->txt[k2] = 0;
                        if(k2) N_TOC++;
                    }
                    if(e_parte) rotulo_seccao(cmd, estrela, rot, sizeof rot);
                    if(!e_parte && rotulo_seccao(cmd, estrela, rot, sizeof rot)){
                        for(int t = 0; rot[t]; t++) empurra(&e, (unsigned char)rot[t], F_NEG);
                        /* dois espaços: é o que o LaTeX põe entre o número e o título */
                        empurra(&e, ' ', F_NEG); empurra(&e, ' ', F_NEG);
                    }
                }
                while(j < n && prof){
                    if(s[j] == '{') prof++;
                    else if(s[j] == '}'){ if(!--prof) break; }
                    else {
                        if(s[j] == '\\'){                /* um comando dentro do título */
                            long q = j + 1; char c2[64]; int k2 = 0;
                            while(q < n && isalpha((unsigned char)s[q]) && k2 < 63) c2[k2++] = s[q++];
                            c2[k2] = 0;
                            const Par *P = lex_acha(c2);
                            if(P) empurra(&e, P->glifo, P->simb ? F_SIM : F_NEG);
                            j = q; continue;
                        }
                        int cons; int g = utf8_glifo((const unsigned char*)s + j, &cons);
                        /* A LIGADURA TAMBÉM AQUI. O laço dos títulos é outro, e por isso o
                         * travessão saía com três hífenes só nas secções: «a floresta e o mar
                         * --- o que estava aqui». Um defeito com dois laços é dois defeitos. */
                        { int cl = 0, lg = liga_acha(s, n, j, &cl);
                          if(lg){ empurra(&e, lg, F_NEG); j += cl; continue; } }
                        if(g != '{' && g != '}') empurra(&e, g, F_NEG);
                        j += cons; continue;
                    }
                    j++;
                }
                fecha_paragrafo(&e);
                if(e_parte){ CENTRA = 0; pagina_fecha(p); pagina_abre(p); }
                { double a = 0, d2 = 0;
                  espaco_titulo(cmd, degrau_do_comando(cmd), &a, &d2);
                  /* A RÉGUA DOURADA, que o `\titleformat` declara no grupo final:
                   * `[{\vspace{2mm}\color{ouro}\titlerule[1.2pt]}]`. É uma linha por
                   * baixo do título, e o tradutor já as desenha — só não a ia buscar. */
                  double esp = 0; char cr[24]; cr[0] = 0;
                  if(regua_do_comando(cmd, &esp, cr, sizeof cr) && esp > 0){
                      p->y -= (long)(d2 * 0.4 * 1000);
                      poe_regua(p, (double)MARGEM, (double)(MARGEM + COL),
                                p->y / 1000.0, esp, cr[0] ? cr : "ouro");
                      p->y -= (long)(d2 * 0.6 * 1000);
                  } else p->y -= (long)(d2 * 1000); }
                e.fonte = F_REG; e.L.nivel = 0;
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
                if(N_CARTA > F_ITA) e.fonte = F_ITA;
                i = j; continue;
            }
            /* `\texttt` e `\code` pedem a MONOESPACADA, e ela e' uma estaca propria: a da
             * LARGURA. Estavam mapeados para a negra, e por isso a largura vinha de uma
             * fonte e o glifo de outra — as duas invasoes que restavam no enredo, em
             * `\texttt{broca-so} & $688$`, eram isso. */
            if(!strcmp(cmd, "texttt") || !strcmp(cmd, "ttfamily") || !strcmp(cmd, "code")){
                if(N_CARTA > F_MON) e.fonte = F_MON;
                i = j; continue;
            }
            if(!strcmp(cmd, "textsc") || !strcmp(cmd, "scshape")){
                if(N_CARTA > F_VER) e.fonte = F_VER;
                i = j; continue;
            }
            if(!strcmp(cmd, "textbf") || !strcmp(cmd, "emph") || !strcmp(cmd, "textit") ||
               !strcmp(cmd, "textsc") || !strcmp(cmd, "code")  || !strcmp(cmd, "texttt")){
                e.fonte = F_NEG;                        /* uma só variante: a Helvetica-Bold */
                i = j; continue;                        /* o } repõe adiante */
            }
            if(!strcmp(cmd, "item")){
                fecha_paragrafo(&e);
                e.L.recuo = e.recuo;
                empurra(&e, 0xB7, F_SIM); empurra(&e, ' ', F_REG);
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
                fecha_paragrafo(&e);
                double esp = (cmd[0] == 'm' || cmd[0] == 'h') ? 0.5 : 1.0;   /* mid fina, top/bottom grossa */
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
                long linha = (long)(escala_entre(D_TEXTO)*1000 + 0.5);   /* em milésimos */
                e.p->y -= linha / 2;
                poe_regua(e.p, MARGEM, MARGEM + COL, e.p->y / 1000.0, esp, "tinta");
                e.p->y -= linha - linha / 2;        /* o resto: a soma FECHA, resíduo 0 */
                /* E A TABELA TEM DE SABER: a régua mexeu no lápis, e o topo da fila é agora
                 * outro. Sem isto o `tab_y` guardava a posição de ANTES da régua, e a fila
                 * seguinte nascia por cima dela — o cabeçalho ficava sobre a linha de topo. */
                if(e.tab){ e.tab_y = e.p->y; e.tab_ymin = e.p->y; e.tab_pag = e.p->npag; }
                i = j; continue;
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
                    fecha_paragrafo(&e);
                    if(!vazia) vazia = (p->y >= TOPO - PT);
                    if(cmd[0] == 'b'){
                        /* so' se abre pagina se a actual tiver alguma coisa: o `\maketitle`
                         * ja' abriu uma ao fechar a capa, e abrir outra deixava a 2 EM
                         * BRANCO com o resumo a cair na 3. */
                        if(!vazia){ pagina_fecha(p); pagina_abre(p); }
                        /* o titulo, centrado, no corpo do texto e a negro */
                        p->y -= 170*PT;
                        CENTRA = 1; e.fonte = F_NEG;
                        for(int t = 0; NOME_RESUMO[t]; t++)
                            empurra(&e, (unsigned char)NOME_RESUMO[t], F_NEG);
                        quebra_e_desenrola(&e, 1);
                        e.L.n = 0; CENTRA = 0; e.fonte = F_REG;
                        p->y -= 10*PT;
                    } else if(!vazia){ pagina_fecha(p); pagina_abre(p); }
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
                fecha_paragrafo(&e);
                /* A CAIXA — o tcolorbox do catálogo é `boxrule=0pt, leftrule=2pt`: a moldura
                 * É a barra da esquerda, e mais nada. Guarda-se o y ao abrir e desenha-se ao
                 * fechar, quando já se sabe onde ela acaba.
                 *
                 * E desenha-se SÓ a barra, não o fundo: um stream de PDF é sequencial, e um
                 * fundo escrito depois do texto TAPA-O. A barra vive na margem, à esquerda de
                 * onde o texto cai, e por isso pode vir no fim. O fundo pede dois streams (o
                 * /Contents aceita um array) e fica nomeado, não escondido. */
                if(!strcmp(amb, "tcolorbox") || !strcmp(amb, "obs") || !strcmp(amb, "teorema")
                   || !strcmp(amb, "proposicao")){
                    if(abre){ e.p->caixa_y = e.p->y; }
                    else if(e.p->caixa_y > 0){
                        double alt = (e.p->caixa_y - e.p->y) / 1000.0;
                        if(alt > 0 && alt < 720){                /* na mesma página */
                            /* o tcolorbox do catálogo: colback=ouroclaro!35, leftrule=2pt.
                             * São os DOIS — o fundo e a barra —, e agora os dois cabem,
                             * porque o fundo vai no primeiro stream e pinta por baixo. */
                            poe_rect(e.p, MARGEM - 10, e.p->y / 1000.0 + 2, COL + 14, alt + 6, "ouroclaro");
                            poe_rect(e.p, MARGEM - 10, e.p->y / 1000.0 + 2, 2, alt + 6, "ouro");
                        }
                        e.p->caixa_y = -1;
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
                   || !strcmp(amb, "tabularx") || !strcmp(amb, "array")){
                    if(abre){
                        fecha_paragrafo(&e);
                        long q = j + 1;
                        q = ate_abre(s, q, n);   /* o preâmbulo */
                        int nc = 0, d2 = 0;
                        for(q++; q < n; q++){
                            if(s[q] == '{') d2++;
                            else if(s[q] == '}'){ if(!d2) break; d2--; }
                            else if(!d2 && (s[q]=='l'||s[q]=='r'||s[q]=='c'||s[q]=='p'||s[q]=='X')) nc++;
                        }
                        e.tab = 1;
                        e.tab_ncol = nc > 0 ? nc : 1;
                        e.tab_col = 0;
                        e.tab_x0 = MARGEM;
                        {   /* os F de Fibonacci: a proporção áurea em inteiros */
                            static const long F[17] = {1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987,1597};
                            long tot = 0;
                            for(int k = 0; k < e.tab_ncol && k < 16; k++) tot += F[k+1];
                            long soma = 0;
                            for(int k = 0; k < e.tab_ncol && k < 16; k++){
                                e.tab_w[k] = COL * F[k+1] / tot;
                                soma += e.tab_w[k];
                            }
                            /* a ÚLTIMA leva o resto: a soma fecha EXACTAMENTE em COL */
                            if(e.tab_ncol > 0 && e.tab_ncol <= 16)
                                e.tab_w[e.tab_ncol-1] += COL - soma;
                            e.tab_larg = e.tab_w[0];
                        }
                        /* O RECUO VEM ANTES do `tab_y`, e não depois. Guardando o topo da
                         * fila e só então recuando, a primeira fila nasce 4 pt acima de onde a
                         * tabela começa — e o cabeçalho caía por cima da régua de topo. */
                        e.p->y -= 4*PT;
                        e.tab_y = e.p->y;
                        e.tab_ymin = e.p->y;
                        i = q + 1; continue;
                    } else {
                        fecha_paragrafo(&e);
                        e.tab = 0; e.tab_col = 0;
                        e.p->y -= 4;
                        i = j; continue;
                    }
                }
                if(abre && (!strcmp(amb, "verbatim") || !strcmp(amb, "Verbatim")
                         || !strcmp(amb, "lstlisting") || !strcmp(amb, "minted"))){
                    char fim[80];
                    { char *q = ap_str(fim, "\\end{"); q = ap_str(q, amb); q = ap_str(q, "}"); *q = 0; }
                    const char *f = strstr(s + j, fim);
                    long ate = f ? (f - s) : n;
                    e.fonte = F_NEG; e.L.recuo = e.recuo + 12;
                    for(long q = j + 1; q < ate; ){
                        if(s[q] == '\n'){
                            fecha_paragrafo(&e);
                            e.L.recuo = e.recuo + 12;
                            q++; continue;
                        }
                        int cons; int g = utf8_glifo((const unsigned char*)s + q, &cons);
                        empurra(&e, g, F_NEG);          /* SEM interpretar: nem $, nem barra, nem chaves */
                        q += cons;
                    }
                    fecha_paragrafo(&e);
                    e.fonte = F_REG; e.L.recuo = e.recuo;
                    i = f ? ate + (long)strlen(amb) + 6 : n;
                    continue;
                }
                if(!strcmp(amb, "itemize") || !strcmp(amb, "enumerate") || !strcmp(amb, "description"))
                    e.recuo = abre ? e.recuo + 18 : (e.recuo >= 18 ? e.recuo - 18 : 0);
                if(!strcmp(amb, "document") && !abre) break;
                e.L.recuo = e.recuo;
                i = j + 1; continue;
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
                    fecha_paragrafo(&e);
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
               !strcmp(cmd, "setlength") || !strcmp(cmd, "hyphenation")){
                long q = j;
                int nar = !strcmp(cmd, "hyphenation") ? 1 : 2;
                if(!strcmp(cmd, "definecolor")) nar = 3;
                for(int t = 0; t < nar && q < n; t++){
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
                double c1 = str2dbl(pf, &ef);
                if(ef != pf && c1 > 0){
                    /* o degrau é o da escala mais perto — e nunca uma medida nova: se o
                     * tamanho não estiver na escala, é a escala que decide, não este `if` */
                    long melhor = -1; double dmin = 1e9;
                    for(long t = 0; t < N_ESCALA; t++){
                        double d = ESCALA[t].corpo - c1; if(d < 0) d = -d;
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
            if(!strcmp(cmd, "rule") ||
               !strcmp(cmd, "vspace") || !strcmp(cmd, "hspace")){
                /* `\rule{larg}{esp}` é uma RÉGUA, e este tradutor já as desenha — a do
                 * ouro por baixo do título é a mesma primitiva das linhas da tabela */
                long q = j;
                double a1 = 0, a2 = 0; char u1[8]; char u2[8]; u1[0] = 0; u2[0] = 0;
                int nar = (cmd[0] == 'r') ? 2 : 1;
                long ini = q;
                q = ate_abre(s, q, n);
                if(q < n && cmd[0] == 'r'){
                    /* "a1u1}{a2u2}" --- dois (valor+unidade) com o }{ no meio: str2dbl + até 2 minúsculas */
                    const char *pr = s + q + 1, *e1;
                    double v1 = str2dbl(pr, &e1);
                    if(e1 != pr){
                        a1 = v1;
                        int k = 0; while(k < 2 && e1[k] >= 'a' && e1[k] <= 'z'){ u1[k] = e1[k]; k++; } u1[k] = 0;
                        const char *af = e1 + k;
                        if(af[0] == '}' && af[1] == '{'){
                            const char *pr2 = af + 2, *e2;
                            double v2 = str2dbl(pr2, &e2);
                            if(e2 != pr2){
                                a2 = v2;
                                int k2 = 0; while(k2 < 2 && e2[k2] >= 'a' && e2[k2] <= 'z'){ u2[k2] = e2[k2]; k2++; } u2[k2] = 0;
                            }
                        }
                    }
                }
                for(int t = 0; t < nar && q < n; t++){
                    q = ate_abre(s, q, n);
                    long f = fecha_chave(s, n, q); if(f < 0){ q = ini; break; } q = f + 1;
                }
                if(cmd[0] == 'r' && a1 > 0 && a2 > 0){
                    double k1 = unidade_pt(u1);
                    double k2 = unidade_pt(u2);
                    fecha_paragrafo(&e);
                    poe_rect(p, (double)MARGEM, e.p->y / 1000.0, a1 * k1, a2 * k2, "ouro");
                    e.p->y -= (long)((a2 * k2 + 4) * 1000);
                }
                i = q; continue;
            }
            /* `\title{...}` é a CAPA, e o `\maketitle` centra-a e dá-lhe página própria.
             * Sem isto ela saía encostada à margem esquerda e colada ao texto do capítulo
             * seguinte — que é a primeira coisa que se vê ao abrir o documento. */
            if(!strcmp(cmd, "title")){
                fecha_paragrafo(&e);
                CENTRA = 1;
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
                { double h = 0; long q = j; int d3 = 0;
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
                                       double c1 = 0, c2 = 0;
                                       int _ok = 0;
                                       if(d4){ const char *pc = d4 + strlen(alvo), *e1;   /* "%lf}{%lf"==2 por str2dbl */
                                               double v1 = str2dbl(pc, &e1);
                                               if(e1 != pc && e1[0] == '}' && e1[1] == '{'){
                                                   const char *e2; double v2 = str2dbl(e1 + 2, &e2);
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
                                           long linhas = 1 + (long)(largo * c1 / 1000.0) / (COL > 0 ? COL : 1);
                                           h += c2 * (double)linhas;
                                       }
                                       }
                          } else if(s[q+1] == '\\' && s[q+2] == '['){
                              double m = medida_pt(s + q + 3);
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
                  double alt = CAPA_ALT > 0 ? CAPA_ALT : h * 1000;
                  if(alt > 0 && alt < A4_AM){
                      long topo_certo = (long)((A4_AM + alt) / 2);
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
                fecha_paragrafo(&e);
                if(p->y < TOPO - PT && !p->abriu_agora){ pagina_fecha(p); pagina_abre(p); }
                /* o título, no degrau do capítulo */
                e.L.deg = degrau_do_comando("chapter");
                e.fonte = F_NEG;
                for(int t = 0; NOME_SUMARIO[t]; t++)
                    empurra(&e, (unsigned char)NOME_SUMARIO[t], F_NEG);
                quebra_e_desenrola(&e, 1);
                e.L.n = 0; e.L.deg = -1; e.fonte = F_REG;
                p->y -= 24*PT;
                for(int t = 0; t < N_TOC; t++){
                    Toc *q2 = &TOC[t];
                    e.L.recuo = (q2->nivel - 1) * 14;
                    if(q2->nivel == 1) p->y -= 5*PT;
                    e.fonte = q2->nivel == 1 ? F_NEG : F_REG;
                    /* O TEXTO E' UTF-8, e empurra-se GLIFO a glifo — nao byte a byte. Os
                     * acentos tem dois bytes, e empurra-los sozinhos dava «IntroduÃ§Ã£o». */
                    /* o ROTULO ja' vem em WinAnsi — e' o que o `tex_para_winansi` produziu
                     * ao ler o babel. So' o TEXTO do titulo e' que vem do fonte em UTF-8.
                     * Ler o rotulo como UTF-8 dava «Cap?lo». */
                    for(int k2 = 0; q2->rot[k2]; k2++)
                        empurra(&e, (unsigned char)q2->rot[k2], e.fonte);
                    if(q2->rot[0]){ empurra(&e, ' ', e.fonte); empurra(&e, ' ', e.fonte); }
                    for(int k2 = 0; q2->txt[k2]; ){
                        if(q2->txt[k2] == '-' && q2->txt[k2+1] == '-'){
                            if(q2->txt[k2+2] == '-'){ empurra(&e, 0x97, e.fonte); k2 += 3; }
                            else { empurra(&e, 0x96, e.fonte); k2 += 2; }
                            continue;
                        }
                        int cs; int gl = utf8_glifo((const unsigned char*)q2->txt + k2, &cs);
                        empurra(&e, gl, e.fonte); k2 += cs;
                    }
                    /* a linha NAO justifica e o numero vai no fim: encher de espacos fazia-a
                     * quebrar, e a quebra justificava a primeira metade a' largura toda. */
                    { Linha out = e.L; out.larg = 0;
                      double cp = escala_corpo(D_TEXTO);
                      long larg = mede(out.g, out.n, cp);
                      char np[8]; { char *q = ap_num(np, q2->pag); *q = 0; }
                      long lnp = 0;
                      for(int k2 = 0; np[k2]; k2++) lnp += (long)largura((unsigned char)np[k2], F_NEG) * cp;
                      /* o texto a' esquerda */
                      if(out.n) desenrola_em(p, &out, MARGEM + e.L.recuo, 0);
                      /* e o numero encostado a' direita, na mesma linha */
                      Linha nn; memset(&nn, 0, sizeof nn);
                      for(int k2 = 0; np[k2]; k2++){ nn.g[nn.n].g = (unsigned char)np[k2];
                                                     nn.g[nn.n].f = F_NEG; nn.n++; }
                      desenrola_em(p, &nn, MARGEM + COL - lnp / 1000.0, 1);
                      (void)larg;
                    }
                    e.L.n = 0; e.L.recuo = 0; e.fonte = F_REG;
                }
                pagina_fecha(p); pagina_abre(p);
                i = j; continue;
            }
            if(!strcmp(cmd, "maketitle") || !strcmp(cmd, "tableofcontents") ||
               !strcmp(cmd, "newpage")   || !strcmp(cmd, "clearpage")){
                fecha_paragrafo(&e);
                /* o `\maketitle` fecha a capa: o que vem a seguir começa em página nova */
                if(cmd[0] == 'm' && CENTRA && CAPA_ALT <= 0 && Y_CAPA > 0){
                    /* mediu-se: rebobina-se e faz-se outra vez, agora com o numero certo */
                    CAPA_ALT = (double)(Y_CAPA - p->y);
                    s_vai(&p->sf, CAPA_POS);
                    /* E O FUNDO TAMBEM. As reguas vao para outro ficheiro, e rebobinar so'
                     * o principal deixava-as escritas DUAS vezes — quatro reguas na capa
                     * onde o gabarito tem duas. Um stream esquecido e' meia reversao. */
                    if(p->fundo_on){ s_vai(&p->sfundo, CAPA_FUN); p->n_fundo = CAPA_NF; }
                    p->y = CAPA_Y; p->npag = CAPA_PAG;
                    i = CAPA_I; e.L.n = 0;
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
                    e.fonte = F_REG; e.L.deg = -1;
                    pagina_fecha(p); pagina_abre(p); }
                if(cmd[0] == 'n' || cmd[0] == 'c'){ pagina_fecha(p); pagina_abre(p); }
                i = j; continue;
            }
            const Par *P = lex_acha(cmd);
            if(P){ empurra(&e, P->glifo, P->simb ? F_SIM : e.fonte); i = j; continue; }
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
         * Tinha `(e.mat && isalpha(g)) ? F_SIM : e.fonte` — toda a letra latina dentro de $...$
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
          if(lg){ empurra(&e, lg, e.fonte); i += cl; continue; } }
        /* a chaveta abre e fecha o escopo do degrau: ao sair do grupo onde o `\fontsize`
         * foi posto, o degrau volta ao que estava — que é o que o LaTeX faz */
        if(g == '{') PROF++;
        else if(g == '}'){
            if(PROF > 0) PROF--;
            if(DEG_PROF >= 0 && PROF < DEG_PROF){ DEG_FORCADO = -1; DEG_PROF = -1; }
            if(COR_PROF >= 0 && PROF < COR_PROF){ COR_TEXTO[0] = 0; COR_PROF = -1; }
        }
        empurra(&e, g, e.fonte);
        if(e.fonte == F_NEG && i + 1 < n && s[i+1] == '}') e.fonte = F_REG;
        i += cons;
    }
    fecha_paragrafo(&e);
    *glifos = e.glifos;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * §X6  A VOLTA — o texto sai do PDF que entrou no .tex
 * ───────────────────────────────────────────────────────────────────────────── */

/* lê os (…) Tj do PDF sem compressão e devolve os glifos, na ordem. É a volta do §X4/§X5. */
static long extrai(const char *pdf, long n, char *out, long lim){
    long o = 0; int dentro = 0;
    for(long i = 0; i < n; i++){
        if(!dentro){
            if(pdf[i] == '(' && (i == 0 || pdf[i-1] != '\\')) dentro = 1;
            continue;
        }
        if(pdf[i] == '\\' && i + 1 < n){ if(o < lim-1){ i = i + 1; out[o++] = pdf[i]; } continue; }
        if(pdf[i] == ')'){ dentro = 0; continue; }
        if(o < lim - 1) out[o++] = pdf[i];
    }
    out[o] = 0; return o;
}

/* ───────────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    /* o idioma da bateria: sem isto ela conta UMA unidade grossa (o exit) em vez das que ha */
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
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
 * destino trocado, que é o medir-pela-metade do corpo-estelar. */

/* uma passagem de avaliação: devolve buffer novo, `*n` actualizado, e quantas avaliou */

/* avalia até estabilizar — as macros chamam-se umas às outras, e a profundidade é finita
 * por construção (o TeX rejeita recursão sem fim); o tecto é uma rede, não uma escolha */


/* ─── A ABSORÇÃO: pdf → estrela, que é o outro sentido do MOVE ──────────────────────
 *
 * O `corpo-estelar.tex` diz como se mede aqui, e diz-o na última linha da especificação:
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
 * O `corpo-estelar.tex` responde à pergunta «porque não fecha a volta?» em dois sítios:
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
static struct { char cmd[24]; double corpo; char cor[24]; } NIVEL_CORPO[8];
static int N_NIVEL = -1;


/* ── O ESPAÇO À VOLTA DE UM TÍTULO ──────────────────────────────────────────────────
 * O código punha `p->y -= 8` para todos, e 8 não é do estilo nem do LaTeX: é um número
 * escrito à mão. Daí os títulos colarem-se ao texto de cima e de baixo.
 *
 * Onde o estilo declara `\titlespacing`, é esse que manda. Onde não declara --- e só o
 * `\chapter` o faz --- o espaço sai da ESCALA, que é a régua deste documento: antes, a
 * entrelinha do próprio degrau; depois, a do texto. Não se trazem os valores do LaTeX de
 * fora, porque uma régua de outro corpo não transporta (teoria, thm:transporte). */
static void espaco_titulo(const char *cmd, long deg, double *antes, double *depois){
    *antes = 0; *depois = 0;
    /* pela CACHE: esta funcao e' chamada por cada titulo, e lia um megabyte de cada vez */
    { const char *b = estilo_texto(NULL);
    if(b){
        char alvo[64]; char *ka = ap_str(alvo, "titlespacing*{\\"); ka = ap_str(ka, cmd); *ka++ = '}'; *ka = 0;
        const char *q = strstr(b, alvo);
        if(!q){ ka = ap_str(alvo, "titlespacing{\\"); ka = ap_str(ka, cmd); *ka++ = '}'; *ka = 0; q = strstr(b, alvo); }
        if(q){
            double a = 0, c = 0;
            /* `{esquerda}{antes}{depois}` — o primeiro salta-se */
            const char *z = q + strlen(alvo);
            while(*z && *z != '{') z++;
            if(*z){ z++; while(*z && *z != '{') z++; }
            int _ok = 0;
            /* `{%lf`: o `{` literal, o double por str2dbl (lib/le_num.h) */
            if(*z == '{'){ const char *e; double av = str2dbl(z + 1, &e); if(e != z + 1){ a = av; _ok = 1; } }
            if(_ok){
                while(*z && *z != '}') z++; if(*z) z++;
                if(*z == '{'){ const char *e; double cv = str2dbl(z + 1, &e); if(e != z + 1){ *antes = a; *depois = cv; return; } }
            }
        }
    }
    }
    if(deg >= 0 && deg < N_ESCALA){
        *antes = ESCALA[deg].entre * 0.5;
        *depois = (N_ESCALA > D_TEXTO ? ESCALA[D_TEXTO].entre : 15.0) * 0.5;
    }
}

/* a régua que o `\titleformat` declara no grupo final `[...]`, se declarar */
static int regua_do_comando(const char *cmd, double *esp, char *cor, size_t nc){
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
    if(r[10] == '['){ const char *e; double v = str2dbl(r + 11, &e); if(e != r + 11) *esp = v; } else *esp = 0.4;
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
static const char *comando_de_corpo(double corpo){
    for(int t = 0; t < N_NIVEL; t++)
        if(NIVEL_CORPO[t].corpo > corpo - 0.01 && NIVEL_CORPO[t].corpo < corpo + 0.01)
            return NIVEL_CORPO[t].cmd;
    return NULL;
}

/* o degrau da escala a que um corpo pertence, ou -1 se não é nenhum */
static long degrau_de(double corpo){
    for(long t = 0; t < N_ESCALA; t++)
        if(ESCALA[t].corpo > corpo - 0.01 && ESCALA[t].corpo < corpo + 0.01) return t;
    return -1;
}
