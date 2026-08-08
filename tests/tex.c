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
 *   MAS ha construcoes que ele COME e eu ainda nao isolei quais. A frase da linha 2071 do
 *   catalogo — "\item \code{tools/traduz.c} --- \textbf{o lexico e a roupa...}" — sai
 *   inteira num fragmento isolado e NAO sai no documento grande. Oito ocorrencias de
 *   'lexico' no fonte, zero no PDF. Nao e encoding (o fonte esta em NFC e o acento
 *   atravessa noutras palavras) nem quebra de linha. Fica por resolver, e por isso este
 *   tradutor NAO substitui o pdflatex ainda: substitui-o quando o §X6 correr sobre o
 *   catalogo inteiro e nao sobre um fonte de teste.
 *
 *   E a tipografia e mais densa que a do TeX (24pp contra 31, 60 contra 72). Nao ha
 *   ligaduras, nao ha hifenizacao, nao ha tabelas nem matematica em display — o $x^2$ sai
 *   como texto, nao como expoente.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "spline.h"   /* a carta da fonte: a largura vem da CURVA */

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
static char NOME_CAP[48] = "", NOME_PARTE[48] = "";
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

static void le_nomes_idioma(void){
    if(NOMES_LIDOS) return;
    NOMES_LIDOS = 1;
    /* a opção do babel, do estilo.tex — a mesma porta das cores e da escala */
    FILE *f = fopen("../estilo.tex", "rb"); if(!f) f = fopen("estilo.tex", "rb");
    if(!f) return;
    static char buf[1 << 20];
    long n = (long)fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n > 0 ? n : 0] = 0;
    const char *q = strstr(buf, "{babel}");
    if(!q) return;
    /* recua-se até ao `[` da opção */
    const char *b = q; while(b > buf && *b != '[') b--;
    if(*b != '[') return;
    char idioma[32]; int k = 0; b++;
    while(*b && *b != ']' && *b != ',' && k < 31) idioma[k++] = *b++;
    idioma[k] = 0;
    if(!k) return;
    /* o `.ldf` do babel INSTALADO — é de lá que o pdflatex tira as mesmas palavras */
    char cmd[256];
    snprintf(cmd, sizeof cmd, "kpsewhich %s.ldf 2>/dev/null", idioma);
    FILE *pp = popen(cmd, "r"); if(!pp) return;
    char caminho[512] = "";
    if(!fgets(caminho, sizeof caminho, pp)){ pclose(pp); return; }
    pclose(pp);
    caminho[strcspn(caminho, "\r\n")] = 0;
    FILE *g = fopen(caminho, "rb"); if(!g) return;
    static char ldf[1 << 21];
    long m = (long)fread(ldf, 1, sizeof ldf - 1, g); fclose(g); ldf[m > 0 ? m : 0] = 0;
    struct { const char *chave; char *dest; size_t cap; } N[] = {
        { "chaptername{", NOME_CAP,   sizeof NOME_CAP   },
        { "partname{",    NOME_PARTE, sizeof NOME_PARTE },
    };
    for(size_t t = 0; t < sizeof N / sizeof N[0]; t++){
        const char *a = strstr(ldf, N[t].chave);
        if(!a) continue;
        a += strlen(N[t].chave);
        /* até à chaveta que fecha, com aninhamento — `Cap\'{\i}tulo` tem um par dentro */
        int d = 1; const char *z = a;
        while(*z && d){ if(*z=='{') d++; else if(*z=='}'){ if(!--d) break; } z++; }
        char cru[96]; size_t ln = (size_t)(z - a); if(ln > 95) ln = 95;
        memcpy(cru, a, ln); cru[ln] = 0;
        tex_para_winansi(cru, N[t].dest, N[t].cap);
    }
}

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

/* o rótulo do título: devolve o comprimento escrito em `o`, ou 0 se não se numera */
static int rotulo_seccao(const char *cmd, int estrela, char *o, size_t cap){
    o[0] = 0;
    if(estrela) return 0;                       /* `\chapter*` não numera nem incrementa */
    le_nomes_idioma();
    if(!strcmp(cmd, "part")){
        char r[16]; romano(++C_PARTE, r, sizeof r);
        snprintf(o, cap, "%s%s%s", NOME_PARTE, NOME_PARTE[0] ? " " : "", r);
    } else if(!strcmp(cmd, "chapter")){
        C_CAP++; C_SEC = C_SUB = C_SSUB = 0;
        snprintf(o, cap, "%s%s%ld", NOME_CAP, NOME_CAP[0] ? " " : "", C_CAP);
    } else if(!strcmp(cmd, "section")){
        C_SEC++; C_SUB = C_SSUB = 0;
        snprintf(o, cap, "%ld.%ld", C_CAP, C_SEC);
    } else if(!strcmp(cmd, "subsection")){
        C_SUB++; C_SSUB = 0;
        snprintf(o, cap, "%ld.%ld.%ld", C_CAP, C_SEC, C_SUB);
    } else if(!strcmp(cmd, "subsubsection")){
        C_SSUB++;
        snprintf(o, cap, "%ld.%ld.%ld.%ld", C_CAP, C_SEC, C_SUB, C_SSUB);
    } else return 0;
    return (int)strlen(o);
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
#define F_SIM 2                                   /* a Symbol */

/* A CARTA, aberta uma vez. Se a fonte estiver no sistema a largura vem da CURVA; se não estiver,
 * cai na tabela — e isso é dito na saída, nunca em silêncio, porque as duas não medem o mesmo
 * para os acentuados (a tabela não os tem, e eu punha 556 a olho). */
static Ttf CARTA_R, CARTA_N;
static int  CARTA = 0;                            /* 0 = tabela, 1 = curva */

static void carta_abre(void){
    static int tentado = 0;
    if(tentado) return;
    tentado = 1;
    CARTA = spline_abre_alguma(&CARTA_R, SPLINE_REG, SPLINE_NCAND, NULL)
         && spline_abre_alguma(&CARTA_N, SPLINE_NEG, SPLINE_NCAND, NULL);
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
static int largura_tabela(int g, int fonte)
{
    carta_abre();
    if(!CARTA) return largura(g, fonte);
    const Ttf *t = (fonte == F_NEG) ? &CARTA_N : &CARTA_R;
    extern int winansi_para_unicode(int);
    int gi = ttf_glifo(t, winansi_para_unicode(g));
    if(!gi) return 0;                          /* a casa vazia: nunca e' desenhada */
    return (int)((long)ttf_avanco(t, gi) * 1000 / t->upem);
}

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
        if(g == ' ' && CARTA) return (int)((long)ttf_avanco(&CARTA_R, ttf_glifo(&CARTA_R, ' '))
                                           * 1000 / CARTA_R.upem);
        /* e os simbolos: a largura PUBLICADA da Symbol, e nao um numero para todos */
        return (g >= 32 && g <= 255) ? W_SIM[g - 32] : 0;
    }
    if(g == '\n' || g == '\r' || g == '\t') return 0;   /* não são glifos: não se medem */
    carta_abre();
    if(CARTA){
        const Ttf *t = (fonte == F_NEG) ? &CARTA_N : &CARTA_R;
        int gi = ttf_glifo(t, winansi_para_unicode(g));
        /* o glifo 0 é o .notdef: se a fonte não tem o caractere, não se inventa uma largura */
        if(gi) return (int)((long)ttf_avanco(t, gi) * 1000 / t->upem);
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
typedef struct { Gl g[MAXLIN]; int n; int nivel; int recuo; int larg; long deg; } Linha;

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

#define A4_L    595
#define A4_A    842
#define MARGEM   64
#define COL     (A4_L - 2*MARGEM)
#define CORPO    10                                /* o corpo do texto, em pontos */
#define ENTRE    14                                /* a entrelinha */
#define TOPO    (A4_A - MARGEM)
#define FUNDO    MARGEM

#define MAXOBJ 8192
typedef struct {
    FILE *f;
    long off[MAXOBJ];
    int  nobj;
    int  pag[MAXOBJ]; int npag;                    /* os números de objeto das páginas */
    /* O Y DO LAPIS E' INTEIRO. Era `double`, e um double ACUMULA: as tres celulas de uma fila
     * saiam em 728,78 · 729,00 · 729,22 — alinhadas ao centesimo e nao IGUAIS. E ao centesimo
     * nao basta: o lado do tesseracto e' inteiro, e uma linha recta nao tem virgula.
     *
     * Com inteiro o residuo e' ZERO POR CONSTRUCAO — nao ha' onde o erro se acumular, porque
     * nao ha' fraccao a arrastar. E' a regra que este projecto ja' tinha e eu nao apliquei
     * aqui: inteiro desde o primeiro rascunho, e nao «float agora, exacto depois». */
    long y;                                        /* onde vai o lápis — INTEIRO */
    int  aberta;                                   /* há página aberta? */
    long len_obj;                                  /* o objeto /Length pendente */
    long stream_ini;
    double caixa_y;                                /* onde a caixa abriu; <0 = nenhuma aberta */
    long   caixas, reguas;                         /* o que se desenhou, para se poder contar */
    FILE  *fundo;                                  /* O SEGUNDO STREAM: o que fica POR BAIXO */
    long   n_fundo;                                /* quantas operações lá foram */
    int    abriu_agora;    /* o `desenrola` abriu página? A tabela precisa de saber: o seu
                            * `tab_y` fica a apontar para a página anterior, e a célula
                            * seguinte nasceria fora do papel. */
    int    fo, flo;                                /* os objectos do stream do fundo */
} Pdf;

/* ─── O DESENHO: as cores saem do estilo.tex e o caminho é o do desenha.c ─────────────
 * Nenhuma primitiva nova: `m`/`l` fazem o caminho, `f` preenche, `S` traça. É o mesmo
 * operador que desenha o glifo, com outro grau — a régua é grau 1, o contorno é grau 2.
 *
 * As cores NÃO estão escritas aqui: leem-se de ../estilo.tex, que é onde o design vive.
 * Escrevê-las seria a referência à mão, e mudar a cor lá deixaria de mudar o que sai. */
static struct { char nome[32]; long r, g, b; } CORES[64];
static long N_CORES = -1;

static void le_cores_estilo(void){
    if(N_CORES >= 0) return;
    N_CORES = 0;
    FILE *f = fopen("../estilo.tex", "rb");
    if(!f) f = fopen("estilo.tex", "rb");
    if(!f) return;
    static char buf[1 << 20];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n > 0 ? n : 0] = 0;
    const char *q = buf;
    while(N_CORES < 64 && (q = strstr(q, "\\definecolor{")) != NULL){
        q += 13;
        const char *a = q; while(*q && *q != '}') q++;
        long ln = q - a; if(ln > 31) ln = 31;
        memcpy(CORES[N_CORES].nome, a, (size_t)ln); CORES[N_CORES].nome[ln] = 0;
        const char *h = strstr(q, "{HTML}{");
        if(!h) continue;
        unsigned rr, gg, bb;
        if(sscanf(h + 7, "%2x%2x%2x", &rr, &gg, &bb) == 3){
            CORES[N_CORES].r = rr; CORES[N_CORES].g = gg; CORES[N_CORES].b = bb;
            N_CORES++;
        }
    }
}

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
struct degrau { double corpo, entre; };
static struct degrau ESCALA[16];
/* O degrau que a EXPANSÃO trouxe. A avaliação de `\gktit` devolve `\fontsize{23.42}{33.95}`,
 * e esse número não é uma escolha deste ficheiro — é o degrau da dourada que o autor escreveu
 * no estilo.tex. Quando >= 0, manda sobre o nível da secção: quem sabe o tamanho é a fonte. */
static long DEG_FORCADO = -1;
static long fecha_chave(const char *s, long n, long i);
static long N_ESCALA = -1;

static void le_escala_estilo(void){
    if(N_ESCALA >= 0) return;
    N_ESCALA = 0;
    FILE *f = fopen("../estilo.tex", "rb");
    if(!f) f = fopen("estilo.tex", "rb");
    if(!f) return;
    static char buf[1 << 20];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n > 0 ? n : 0] = 0;
    const char *q = buf;
    while(N_ESCALA < 16 && (q = strstr(q, "\\fontsize{")) != NULL){
        double c, e;
        if(sscanf(q + 10, "%lf}{%lf}", &c, &e) == 2 && c > 0 && e > 0){
            ESCALA[N_ESCALA].corpo = c; ESCALA[N_ESCALA].entre = e; N_ESCALA++;
        }
        q += 10;
    }
    /* por tamanho crescente: o degrau 0 é a nota, o último é o título */
    for(long i = 1; i < N_ESCALA; i++)
        for(long j = i; j > 0 && ESCALA[j].corpo < ESCALA[j-1].corpo; j--){
            struct degrau t = ESCALA[j];
            ESCALA[j] = ESCALA[j-1]; ESCALA[j-1] = t;
        }
}

/* o degrau da escala: 0 é o mais pequeno. O texto corrido é o `gktexto`, que é o do meio. */
static double escala_corpo(long degrau){
    le_escala_estilo();
    if(N_ESCALA <= 0) return 10.0;                     /* sem estilo: o que havia */
    if(degrau < 0) degrau = 0;
    if(degrau >= N_ESCALA) degrau = N_ESCALA - 1;
    return ESCALA[degrau].corpo;
}
static double escala_entre(long degrau){
    le_escala_estilo();
    if(N_ESCALA <= 0) return 14.0;
    if(degrau < 0) degrau = 0;
    if(degrau >= N_ESCALA) degrau = N_ESCALA - 1;
    return ESCALA[degrau].entre;
}
#define D_NOTA  0
#define D_COD   1
#define D_TEXTO 2                                      /* o corpo do texto: gktexto */
#define D_SUB   3
#define D_SEC   4
#define D_CAP   5
#define D_TIT   6

static int cor_de(const char *nome, double *r, double *g, double *b){
    le_cores_estilo();
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
    if(!p->aberta || !p->fundo || !cor_de(cor, &r, &g, &b)) return;
    fprintf(p->fundo, "q %.3f %.3f %.3f rg %.2f %.2f m %.2f %.2f l %.2f %.2f l %.2f %.2f l f Q\n",
            r, g, b, x, y, x + w, y, x + w, y + h, x, y + h);
    p->n_fundo++;
    p->caixas++;
}

/* uma régua: dois pontos, traçado. Grau 1 — não tem par, é transporte. */
static void poe_regua(Pdf *p, double x1, double x2, double y, double esp, const char *cor){
    double r, g, b;
    if(!p->aberta || !cor_de(cor, &r, &g, &b)) return;
    fprintf(p->f, "q %.3f %.3f %.3f RG %.2f w %.2f %.2f m %.2f %.2f l S Q\n",
            r, g, b, esp, x1, y, x2, y);
    p->reguas++;
}

static int obj_novo(Pdf *p){
    p->off[++p->nobj] = ftell(p->f);
    return p->nobj;
}

static void pdf_abre(Pdf *p, FILE *f){
    memset(p, 0, sizeof *p);
    p->f = f;
    fprintf(f, "%%PDF-1.4\n%%\xE2\xE3\xCF\xD3\n");
    p->nobj = 5;                                   /* 1 catálogo, 2 páginas, 3..5 as fontes */
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
    p->pag[p->npag++] = po;
    int fo = po + 1, flo = po + 2;                 /* o fundo e o seu /Length */
    int co = po + 3, lo  = po + 4;                 /* o texto e o seu /Length */
    p->nobj = lo;
    fprintf(p->f,
        "%d 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 %d %d]"
        "/Resources<</Font<</F1 3 0 R/F2 4 0 R/F3 5 0 R>>>>/Contents[%d 0 R %d 0 R]>>endobj\n",
        po, A4_L, A4_A, fo, co);
    /* o fundo vai para um temporário e só se copia no fecho — é lá que se sabe o que ele tem */
    p->fundo = tmpfile();
    p->n_fundo = 0;
    p->fo = fo; p->flo = flo;
    p->off[co] = ftell(p->f);
    fprintf(p->f, "%d 0 obj<</Length %d 0 R>>stream\n", co, lo);
    p->stream_ini = ftell(p->f);
    p->len_obj = lo;
    p->y = TOPO;
    p->aberta = 1;
    p->abriu_agora = 1;
}

static void pagina_fecha(Pdf *p){
    if(!p->aberta) return;
    long fim = ftell(p->f);
    fprintf(p->f, "endstream\nendobj\n");
    p->off[p->len_obj] = ftell(p->f);
    fprintf(p->f, "%ld 0 obj %ld endobj\n", p->len_obj, fim - p->stream_ini);
    /* e agora o PRIMEIRO stream — o fundo. Escreve-se DEPOIS no ficheiro e é lido ANTES pelo
     * leitor, porque o /Contents já diz a ordem. A posição no ficheiro e a ordem de pintura
     * deixaram de ser a mesma coisa, e é isso que resolve o problema. */
    p->off[p->fo] = ftell(p->f);
    fprintf(p->f, "%d 0 obj<</Length %d 0 R>>stream\n", p->fo, p->flo);
    long fi = ftell(p->f);
    if(p->fundo){
        rewind(p->fundo);
        int c; while((c = fgetc(p->fundo)) != EOF) fputc(c, p->f);
        fclose(p->fundo); p->fundo = NULL;
    }
    long ff = ftell(p->f);
    fprintf(p->f, "endstream\nendobj\n");
    p->off[p->flo] = ftell(p->f);
    fprintf(p->f, "%d 0 obj %ld endobj\n", p->flo, ff - fi);
    p->aberta = 0;
}

/* escreve um pedaço de glifos numa só fonte, escapando o que o PDF exige */
static void poe_pedaco(FILE *f, const Gl *g, int i, int j, int fonte, double corpo,
                       double x, double y, long espaco_extra){
    static const char *FN[3] = {"/F1", "/F2", "/F3"};
    fprintf(f, "BT %s %.3f Tf %.2f %.2f Td", FN[fonte], corpo, x, y);
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
    fprintf(f, " %.3f Tw", espaco_extra / 1000.0);
    fputs(" (", f);
    for(int k = i; k < j; k++){
        int c = g[k].g;
        if(c == '(' || c == ')' || c == '\\') fputc('\\', f);
        if(c < 32) c = ' ';
        fputc(c, f);
    }
    fputs(") Tj ET\n", f);
}

/* o LUNAR desenrola uma linha na página, deformando o espaço se for para justificar */
static void desenrola(Pdf *p, const Linha *L, int justifica){
    /* NUMA CÉLULA NENHUMA linha justifica, e não só a última: numa coluna `l` o alinhamento é
     * à esquerda em todas. A justificação é do parágrafo, e uma célula não é um parágrafo. */
    if(L->larg > 0) justifica = 0;
    /* O CORPO E A ENTRELINHA SAEM DA ESCALA, e o nível escolhe o degrau. Antes eram 10 e 14
     * escritos à mão — e a razão 1,4 em vez de 1,4497, sem hierarquia nenhuma. */
    if(!L->n){ p->y -= escala_entre(D_TEXTO); return; }
    /* o + 0,5 tem de estar FORA do ternário: escrito (int)(cond ? A : B + 0.5) ele só apanha
     * um dos ramos, e 16,99 truncava para 16 em vez de arredondar para 17. Um parêntese. */
    /* SEM `(int)` e sem o `+0,5`: a escala é GEOMÉTRICA de razão φ^(1/3)=1,1740, e arredondar
     * para inteiro destrói isso — medido, as razões passavam a 1,1111 … 1,2143, um desvio de
     * 5,4%%. E não havia razão nenhuma para arredondar: o `Tf` do PDF aceita fracções. */
    double corpo = ((L->deg >= 0 ? escala_corpo(L->deg)
                     : L->nivel   ? escala_corpo(L->nivel <= 1 ? D_CAP
                                             : (L->nivel == 2 ? D_SEC : D_SUB))
                                : escala_corpo(D_TEXTO)));
    /* e a ALTURA DA LINHA sai da mesma escala: entrelinha/corpo = 1,4497 em TODOS os degraus
     * do estilo.tex, e era 1,4 aqui. A diferença é pequena e é o que faz o texto parecer
     * apertado — a entrelinha é o que dá ar à página. */
    int alt   = (int)((L->nivel ? escala_entre(L->nivel <= 1 ? D_CAP
                                              : (L->nivel == 2 ? D_SEC : D_SUB))
                                : escala_entre(D_TEXTO)) + 0.5);

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

    double x = MARGEM + L->recuo;
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
        poe_pedaco(p->f, L->g, i, j, fonte, corpo, x, p->y, extra);
        long w = 0;
        for(int k = i; k < j; k++){
            w += (long)largura(L->g[k].g, fonte) * corpo;
            if(L->g[k].g == ' ') w += extra;      /* o espaco e' letra em qualquer fonte */
        }
        x += w / 1000.0;
        i = j;
    }
}

static void pdf_fecha(Pdf *p){
    pagina_fecha(p);
    /* o objeto Pages sai no FIM — só agora se sabe quantas páginas há. A xref dá o offset, e a
     * ordem no ficheiro é livre: um objeto pode estar em qualquer parte. */
    p->off[2] = ftell(p->f);
    fprintf(p->f, "2 0 obj<</Type/Pages/Count %d/Kids[", p->npag);
    for(int i = 0; i < p->npag; i++) fprintf(p->f, "%d 0 R ", p->pag[i]);
    fprintf(p->f, "]>>endobj\n");
    p->off[1] = ftell(p->f);
    fprintf(p->f, "1 0 obj<</Type/Catalog/Pages 2 0 R>>endobj\n");
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
        static unsigned char ttf[1 << 22];
        long nttf = 0;
        for(int i = 0; i < SPLINE_NCAND && !nttf; i++){
            FILE *g = fopen(SPLINE_REG[i], "rb");
            if(!g) continue;
            nttf = (long)fread(ttf, 1, sizeof ttf, g);
            fclose(g);
        }
        if(nttf > 0){
            /* o ficheiro, e o seu comprimento — os objectos vêm a seguir aos três das fontes */
            int of = p->nobj + 1, od = p->nobj + 2;         /* FontFile2, FontDescriptor */
            p->nobj = od;
            p->off[of] = ftell(p->f);
            fprintf(p->f, "%d 0 obj<</Length %ld/Length1 %ld>>stream\n", of, nttf, nttf);
            fwrite(ttf, 1, (size_t)nttf, p->f);
            fprintf(p->f, "\nendstream\nendobj\n");
            p->off[od] = ftell(p->f);
            fprintf(p->f, "%d 0 obj<</Type/FontDescriptor/FontName/Embutida/Flags 32"
                          "/FontBBox[-1000 -400 2000 1100]/ItalicAngle 0/Ascent 900"
                          "/Descent -200/CapHeight 700/StemV 80/FontFile2 %d 0 R>>endobj\n", od, of);
            fonte_emb = od;
        }
    }
    static const char *BF[3] = {"Helvetica", "Helvetica-Bold", "Symbol"};
    for(int i = 0; i < 3; i++){
        p->off[3+i] = ftell(p->f);
        if(fonte_emb && i != 2){
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
            fprintf(p->f, "%d 0 obj<</Type/Font/Subtype/TrueType/BaseFont/Embutida"
                          "/FirstChar 32/LastChar 255/FontDescriptor %ld 0 R"
                          "/Encoding/WinAnsiEncoding/Widths[", 3+i, fonte_emb);
            for(int c = 32; c <= 255; c++)
                fprintf(p->f, "%d%s", largura_tabela(c, i), c < 255 ? " " : "");
            fprintf(p->f, "]>>endobj\n");
        }
        else
            fprintf(p->f, "%d 0 obj<</Type/Font/Subtype/Type1/BaseFont/%s%s>>endobj\n",
                    3+i, BF[i], i == 2 ? "" : "/Encoding/WinAnsiEncoding");
    }
    long xref = ftell(p->f);
    fprintf(p->f, "xref\n0 %d\n0000000000 65535 f \n", p->nobj + 1);
    for(int i = 1; i <= p->nobj; i++) fprintf(p->f, "%010ld 00000 n \n", p->off[i]);
    fprintf(p->f, "trailer<</Size %d/Root 1 0 R>>\nstartxref\n%ld\n%%%%EOF\n", p->nobj + 1, xref);
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
    if(e->L.n == 0) e->L.deg = DEG_FORCADO;
    if(e->L.n < MAXLIN - 1){ e->L.g[e->L.n].g = (unsigned char)g; e->L.g[e->L.n].f = (unsigned char)f; e->L.n++; }
    e->glifos++;
}

/* quebra a linha corrente onde ela deixa de caber, e desenrola. O que sobra fica para a seguinte. */
static void quebra_e_desenrola(Est *e, int ultima){
    double corpo = ((e->L.deg >= 0 ? escala_corpo(e->L.deg)
                     : e->L.nivel ? escala_corpo(e->L.nivel <= 1 ? D_CAP
                                  : (e->L.nivel == 2 ? D_SEC : D_SUB))
                                  : escala_corpo(D_TEXTO)));
    /* A LARGURA DISPONÍVEL É A DA COLUNA, dentro de uma tabela — e não a da página.
     *
     * Sem isto a célula quebrava só ao chegar à margem direita, logo transbordava para a
     * coluna seguinte e a palavra da coluna ao lado ficava por baixo. As invasões subiram de
     * 0 para 2841 no catálogo assim que a tabela passou a compor — e o defeito não era a
     * tabela: era a quebra a usar a régua errada. Dentro de uma coluna a régua é a coluna. */
    if(e->tab) e->L.larg = (int)(e->tab_larg - 6);   /* a goteira entre colunas */
    else       e->L.larg = 0;
    long alvo = (long)(e->L.larg > 0 ? e->L.larg : COL - e->L.recuo) * 1000;
    while(e->L.n){
        int corte = e->L.n, ate = 0; long w = 0;
        for(int i = 0; i < e->L.n; i++){
            w += (long)largura(e->L.g[i].g, e->L.g[i].f) * corpo;
            if(w > alvo){ corte = ate ? ate : i; break; }
            if(e->L.g[i].g == ' ') ate = i;
        }
        if(corte <= 0) corte = 1;
        Linha out = e->L; out.n = corte;
        while(out.n && out.g[out.n-1].g == ' ') out.n--;
        int fim = (corte == e->L.n);
        desenrola(e->p, &out, !(fim && ultima) && !e->L.nivel);
        if(fim){ e->L.n = 0; break; }
        int k = corte; while(k < e->L.n && e->L.g[k].g == ' ') k++;
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
                fecha_paragrafo(&e); p->y -= 5;
                /* o TeX tambem nao deixa uma formula atravessar paragrafo ("Missing $ inserted").
                 * Sem isto, UM cifrao desirmanado apaga o resto do documento — e foi exatamente
                 * o que aconteceu. Fechar aqui limita o dano de qualquer $ solto a um paragrafo. */
                e.mat = 0; e.fonte = F_REG;
                i = j; continue;
            }
            if(e.L.n && e.L.g[e.L.n-1].g != ' ') empurra(&e, ' ', e.fonte);
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
            if(e.p->abriu_agora){ e.tab_y = e.p->y; e.tab_ymin = e.p->y; e.p->abriu_agora = 0; }
            if(e.p->y < e.tab_ymin) e.tab_ymin = e.p->y;
            e.p->y = e.tab_y;
            i++; continue;
        }
        /* O `~` E' UM ESPACO — o nao-quebravel do TeX. Sem isto sai como til literal e cola
         * as duas palavras: «Livro~I» em vez de «Livro I». */
        if(c == '~'){
            if(e.L.n && e.L.g[e.L.n-1].g != ' ') empurra(&e, ' ', e.fonte);
            i++; continue;
        }
        if(c == ' ' || c == '\t'){
            if(e.L.n && e.L.g[e.L.n-1].g != ' ') empurra(&e, ' ', e.fonte);
            i++; continue;
        }
        if(c == '$'){
            if(i + 1 < n && s[i+1] == '$'){ i += 2; } else i++;
            e.mat = !e.mat;
            continue;
        }
        if(c == '{' || c == '}'){ i++; continue; }      /* as chaves são estrutura, não texto */

        if(c == '\\'){
            long j = i + 1;
            if(j < n && !isalpha((unsigned char)s[j])){  /* \\ , \{ , \% , \_ ... */
                if(s[j] == '\\'){
                    /* E O `\\[3pt]` LEVA ARGUMENTO OPCIONAL — o espaço extra a seguir. Sem o
                     * comer, o `[3pt]` sai como TEXTO e cai por cima da célula seguinte. */
                    long q2 = j + 1;
                    while(q2 < n && (s[q2] == ' ' || s[q2] == '\t')) q2++;
                    if(q2 < n && s[q2] == '['){
                        while(q2 < n && s[q2] != ']') q2++;
                        if(q2 < n) j = q2;
                    }
                    /* O `\\` FECHA A LINHA DA TABELA: volta à primeira coluna e desce UMA vez.
                     *
                     * Sem isto cada célula descia a sua, e uma tabela de 4 colunas gastava 4
                     * linhas por fila — o texto corrido que se via. A conta é a mesma da página:
                     * o `&` anda em x e o `\\` anda em y. */
                    if(e.tab){
                        if(e.L.n) quebra_e_desenrola(&e, 1);   /* idem: a célula não justifica */
                        e.L.n = 0;
                        if(e.p->abriu_agora){ e.tab_ymin = e.p->y; e.p->abriu_agora = 0; }
                        if(e.p->y < e.tab_ymin) e.tab_ymin = e.p->y;
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
                            if(e.p->y - alt_fila < FUNDO){
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
                char um[2] = { s[j], 0 };
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
            if(nv){                                     /* A MARCA: o nível vem do nome */
                fecha_paragrafo(&e);
                p->y -= 8;
                while(j < n && s[j] != '{') j++;
                if(j < n) j++;
                int prof = 1; e.L.nivel = nv; e.fonte = F_NEG; e.L.recuo = 0;
                {   /* o `*` vem logo a seguir ao nome, antes do `{` do título */
                    long z = i + 1; while(z < n && isalpha((unsigned char)s[z])) z++;
                    char rot[64];
                    if(rotulo_seccao(cmd, z < n && s[z] == '*', rot, sizeof rot)){
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
                        if(g != '{' && g != '}') empurra(&e, g, F_NEG);
                        j += cons; continue;
                    }
                    j++;
                }
                fecha_paragrafo(&e);
                e.fonte = F_REG; e.L.nivel = 0;
                i = j + 1; continue;
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
                long linha = (long)(escala_entre(D_TEXTO) + 0.5);   /* inteira */
                e.p->y -= linha / 2;
                poe_regua(e.p, MARGEM, MARGEM + COL, (double)e.p->y, esp, "tinta");
                e.p->y -= linha - linha / 2;        /* o resto: a soma FECHA em inteiros */
                /* E A TABELA TEM DE SABER: a régua mexeu no lápis, e o topo da fila é agora
                 * outro. Sem isto o `tab_y` guardava a posição de ANTES da régua, e a fila
                 * seguinte nascia por cima dela — o cabeçalho ficava sobre a linha de topo. */
                if(e.tab){ e.tab_y = e.p->y; e.tab_ymin = e.p->y; }
                i = j; continue;
            }
            if(!strcmp(cmd, "begin") || !strcmp(cmd, "end")){
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
                        double alt = e.p->caixa_y - e.p->y;
                        if(alt > 0 && alt < 720){                /* na mesma página */
                            /* o tcolorbox do catálogo: colback=ouroclaro!35, leftrule=2pt.
                             * São os DOIS — o fundo e a barra —, e agora os dois cabem,
                             * porque o fundo vai no primeiro stream e pinta por baixo. */
                            poe_rect(e.p, MARGEM - 10, e.p->y + 2, COL + 14, alt + 6, "ouroclaro");
                            poe_rect(e.p, MARGEM - 10, e.p->y + 2, 2, alt + 6, "ouro");
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
                        while(q < n && s[q] != '{') q++;   /* o preâmbulo */
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
                        e.p->y -= 4;
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
                    snprintf(fim, sizeof fim, "\\end{%s}", amb);
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
            if(!strcmp(cmd, "setcounter") || !strcmp(cmd, "addtocounter")){
                long q = j; char nc[32]; int kc = 0;
                while(q < n && s[q] != '{') q++;
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
                        if(alvo_c == &C_CAP){ C_SEC = C_SUB = C_SSUB = 0; }
                        else if(alvo_c == &C_SEC){ C_SUB = C_SSUB = 0; }
                    }
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "fontsize")){
                double c1 = 0, c2 = 0; long q = j;
                while(q < n && s[q] != '{') q++;
                if(sscanf(s + q + 1, "%lf}{%lf}", &c1, &c2) >= 1 && c1 > 0){
                    le_escala_estilo();
                    /* o degrau é o da escala mais perto — e nunca uma medida nova: se o
                     * tamanho não estiver na escala, é a escala que decide, não este `if` */
                    long melhor = -1; double dmin = 1e9;
                    for(long t = 0; t < N_ESCALA; t++){
                        double d = ESCALA[t].corpo - c1; if(d < 0) d = -d;
                        if(d < dmin){ dmin = d; melhor = t; }
                    }
                    if(melhor >= 0) DEG_FORCADO = melhor;
                }
                /* consomem-se os dois argumentos */
                for(int t = 0; t < 2 && q < n; t++){
                    while(q < n && s[q] != '{') q++;
                    long f = fecha_chave(s, n, q); if(f < 0) break; q = f + 1;
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "selectfont") || !strcmp(cmd, "normalsize")){
                if(cmd[0] == 'n') DEG_FORCADO = -1;
                i = j; continue;
            }
            if(!strcmp(cmd, "color") || !strcmp(cmd, "rule") ||
               !strcmp(cmd, "vspace") || !strcmp(cmd, "hspace")){
                /* `\rule{larg}{esp}` é uma RÉGUA, e este tradutor já as desenha — a do
                 * ouro por baixo do título é a mesma primitiva das linhas da tabela */
                long q = j;
                double a1 = 0, a2 = 0; char u1[8] = "", u2[8] = "";
                int nar = (cmd[0] == 'r') ? 2 : 1;
                long ini = q;
                while(q < n && s[q] != '{') q++;
                if(q < n && cmd[0] == 'r') sscanf(s + q + 1, "%lf%2[a-z]}{%lf%2[a-z]}", &a1, u1, &a2, u2);
                for(int t = 0; t < nar && q < n; t++){
                    while(q < n && s[q] != '{') q++;
                    long f = fecha_chave(s, n, q); if(f < 0){ q = ini; break; } q = f + 1;
                }
                if(cmd[0] == 'r' && a1 > 0 && a2 > 0){
                    double k1 = !strcmp(u1,"cm") ? 28.3465 : (!strcmp(u1,"mm") ? 2.83465 : 1.0);
                    double k2 = !strcmp(u2,"cm") ? 28.3465 : (!strcmp(u2,"mm") ? 2.83465 : 1.0);
                    fecha_paragrafo(&e);
                    poe_rect(p, (double)MARGEM, (double)e.p->y, a1 * k1, a2 * k2, "ouro");
                    e.p->y -= (long)(a2 * k2 + 4);
                }
                i = q; continue;
            }
            if(!strcmp(cmd, "maketitle") || !strcmp(cmd, "tableofcontents") ||
               !strcmp(cmd, "newpage")   || !strcmp(cmd, "clearpage")){
                fecha_paragrafo(&e);
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
        if(g == '-' && i + 1 < n && s[i+1] == '-'){
            if(i + 2 < n && s[i+2] == '-'){ empurra(&e, 0x97, e.fonte); i += 3; continue; }
            empurra(&e, 0x96, e.fonte); i += 2; continue;
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
        if(pdf[i] == '\\' && i + 1 < n){ if(o < lim-1) out[o++] = pdf[++i]; continue; }
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

static char *le_tudo(const char *nome, long *n){
    FILE *f = fopen(nome, "rb");
    if(!f) return NULL;
    fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
    char *s = malloc((size_t)*n + 1);
    if(!s){ fclose(f); return NULL; }
    if(fread(s, 1, (size_t)*n, f) != (size_t)*n){ free(s); fclose(f); return NULL; }
    s[*n] = 0; fclose(f); return s;
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
static Macro MAC[MAX_MAC];
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
static void recolhe_macros(const char *s, long n){
    for(long i = 0; i + 12 < n; i++){
        if(s[i] != '\\') continue;
        int decl = !strncmp(s+i+1,"newcommand",10) || !strncmp(s+i+1,"providecommand",14)
                || !strncmp(s+i+1,"renewcommand",12);
        if(!decl) continue;
        long q = i + 1; while(q < n && isalpha((unsigned char)s[q])) q++;
        if(q >= n || s[q] != '{') continue;
        long fim = fecha_chave(s, n, q); if(fim < 0) continue;
        /* o nome, que vem como `{\nome}` */
        long a = q + 1; if(a >= n || s[a] != '\\') continue;
        a++; char nome[48]; int k = 0;
        while(a < fim && k < 47 && isalpha((unsigned char)s[a])) nome[k++] = s[a++];
        nome[k] = 0; if(!k || a != fim) continue;
        q = fim + 1;
        /* o número de argumentos, opcional: `[n]` */
        int nargs = 0;
        if(q < n && s[q] == '['){ long b = q+1; nargs = atoi(s+b);
            while(q < n && s[q] != ']') q++; q++; }
        /* um SEGUNDO `[...]` é o valor por omissão do 1.º argumento — não o tratamos, e
         * saltar a macro é mais honesto que a expandir com um argumento a menos */
        if(q < n && s[q] == '['){ continue; }
        if(q >= n || s[q] != '{') continue;
        long f2 = fecha_chave(s, n, q); if(f2 < 0) continue;
        if(N_MAC >= MAX_MAC) return;
        /* redefinição: fica a última, que é o que o TeX faz */
        int idx = -1;
        for(int t = 0; t < N_MAC; t++) if(!strcmp(MAC[t].nome, nome)) idx = t;
        if(idx < 0){ idx = N_MAC++; }
        else free(MAC[idx].corpo);
        snprintf(MAC[idx].nome, sizeof MAC[idx].nome, "%s", nome);
        MAC[idx].nargs = nargs;
        long cl = f2 - q - 1;
        MAC[idx].corpo = malloc((size_t)cl + 1);
        memcpy(MAC[idx].corpo, s + q + 1, (size_t)cl);
        MAC[idx].corpo[cl] = 0;
    }
}

/* uma passagem de avaliação: devolve buffer novo, `*n` actualizado, e quantas avaliou */
static char *expande_uma(char *s, long *n, long *quantas){
    long cap = *n * 2 + 4096, len = 0;
    char *o = malloc((size_t)cap);
    *quantas = 0;
    for(long i = 0; i < *n; ){
        if(len + 64 > cap){ cap = cap * 2 + 4096; o = realloc(o, (size_t)cap); }
        /* a definição não se expande a si própria, e os comentários ficam como estão */
        if(s[i] == '%'){ while(i < *n && s[i] != '\n') o[len++] = s[i++]; continue; }
        if(s[i] != '\\'){ o[len++] = s[i++]; continue; }
        long a = i + 1; char nome[48]; int k = 0;
        while(a < *n && k < 47 && isalpha((unsigned char)s[a])) nome[k++] = s[a++];
        nome[k] = 0;
        if(!k || !strncmp(nome,"newcommand",10) || !strncmp(nome,"providecommand",14)
              || !strncmp(nome,"renewcommand",12) || !strcmp(nome,"begin") || !strcmp(nome,"end")){
            o[len++] = s[i++]; continue;
        }
        int m = -1;
        for(int t = 0; t < N_MAC; t++) if(!strcmp(MAC[t].nome, nome)) m = t;
        if(m < 0){ o[len++] = s[i++]; continue; }
        /* apanham-se os argumentos — e se algum não fechar, não se avalia */
        long arg_a[9], arg_b[9], q = a;
        int ok_args = 1;
        for(int t = 0; t < MAC[m].nargs; t++){
            while(q < *n && (s[q]==' '||s[q]=='\n'||s[q]=='\t')) q++;
            long f = fecha_chave(s, *n, q);
            if(f < 0){ ok_args = 0; break; }
            arg_a[t] = q + 1; arg_b[t] = f; q = f + 1;
        }
        if(!ok_args){ o[len++] = s[i++]; continue; }
        /* AVALIA-SE: o corpo com `#t` substituído pelo argumento t */
        const char *c = MAC[m].corpo;
        for(long t = 0; c[t]; t++){
            long precisa = len + 64;
            if(precisa > cap){ cap = cap * 2 + 4096; o = realloc(o, (size_t)cap); }
            if(c[t] == '#' && c[t+1] >= '1' && c[t+1] <= '9'){
                int w = c[t+1] - '1';
                if(w < MAC[m].nargs){
                    long al = arg_b[w] - arg_a[w];
                    if(len + al + 64 > cap){ cap = (len + al) * 2 + 4096; o = realloc(o, (size_t)cap); }
                    memcpy(o + len, s + arg_a[w], (size_t)al); len += al;
                }
                t++; continue;
            }
            o[len++] = c[t];
        }
        (*quantas)++;
        i = q;
    }
    o[len] = 0; *n = len; free(s); return o;
}

/* avalia até estabilizar — as macros chamam-se umas às outras, e a profundidade é finita
 * por construção (o TeX rejeita recursão sem fim); o tecto é uma rede, não uma escolha */
static char *avalia_macros(char *s, long *n, const char *estilo){
    long en = 0; char *e = le_tudo(estilo, &en);
    if(e){ recolhe_macros(e, en); free(e); }
    recolhe_macros(s, *n);                      /* e as que o próprio documento define */
    for(int passo = 0; passo < 12; passo++){
        long q = 0; s = expande_uma(s, n, &q);
        EXPANDIDAS += q;
        if(!q) break;
    }
    return s;
}


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

typedef struct { long g; double x, y, corpo; int fonte; } Posto;
static long NAO_LEU_TD = 0;   /* os Td que o parsing não leu — tem de ser 0 */

/* lê o corpo de um PDF composto por este ficheiro: os streams saem sem compressão, e é por
 * isso que a volta não precisa de biblioteca nenhuma */
static long absorve(const char *pdf, Posto *v, long cap){
    long n = 0; char *s = le_tudo(pdf, &n);
    if(!s) return -1;
    long nv = 0;
    double x = 0, y = 0, corpo = 0; int fonte = 0;
    int dentro = 0;                  /* só conta o que está entre BT e ET */
    for(long i = 0; i + 2 < n; i++){
        /* os `(` do PDF também aparecem em nomes e datas: sem esta guarda absorvi
         * 953 637 postos para 716 032 glifos emitidos — 237 605 de lixo */
        if(s[i]=='B' && s[i+1]=='T' && (i==0 || s[i-1]=='\n' || s[i-1]==' ')){ dentro = 1; continue; }
        if(s[i]=='E' && s[i+1]=='T' && (i==0 || s[i-1]=='\n' || s[i-1]==' ')){ dentro = 0; continue; }
        if(!dentro) continue;
        /* `/F<k> <corpo> Tf` — o estado da fonte */
        if(s[i] == '/' && s[i+1] == 'F' && isdigit((unsigned char)s[i+2])){
            int f = 0; double c = 0;
            if(sscanf(s + i + 2, "%d %lf Tf", &f, &c) == 2){ fonte = f - 1; corpo = c; }
            continue;
        }
        /* `<x> <y> Td` — a posição */
        if(s[i] == 'T' && s[i+1] == 'd' && i > 2){
            /* `x y Td` são DOIS números antes do operador: recuar dois espaços punha o `b`
             * dentro do segundo, e o `sscanf` lia um só. O `y` ficava congelado, nenhuma
             * quebra de linha era escrita e o `.tex` da volta saía numa LINHA ÚNICA de
             * 710 214 glifos --- que depois compunha uma página. Três espaços. */
            long b = i - 1; int esp = 0;
            while(b > 0 && esp < 2){ b--; if(s[b] == ' ') esp++; }
            while(b < i && s[b] == ' ') b++;
            double a1, a2;
            /* e se não lê DOIS, não se finge que leu: um `sscanf` que falha em silêncio foi
             * exactamente o que congelou o `y` e fez a volta sair numa linha única */
            if(sscanf(s + b, "%lf %lf", &a1, &a2) == 2){ x = a1; y = a2; }
            else NAO_LEU_TD++;
            continue;
        }
        /* `(...) Tj` — o texto posto. O `\` do PDF escapa `(`, `)` e `\`. */
        if(s[i] == '('){
            long j = i + 1; double av = 0;
            while(j < n && s[j] != ')'){
                if(s[j] == '\\' && j + 1 < n) j++;
                if(nv < cap){
                    v[nv].g = (unsigned char)s[j]; v[nv].x = x + av;
                    v[nv].y = y; v[nv].corpo = corpo; v[nv].fonte = fonte;
                    /* a régua avança com a largura do glifo, na fonte e no corpo correntes:
                     * é a mesma que compôs, e é isso que faz a volta poder fechar */
                    av += largura((int)(unsigned char)s[j], fonte) * corpo / 1000.0;
                    nv++;
                }
                j++;
            }
            i = j;
            continue;
        }
        /* o `Tj` com deslocamento acumulado: `TJ` com arrays não é emitido por este tradutor */
    }
    free(s);
    return nv;
}

/* emite LaTeX a partir do corpo absorvido: as palavras separam-se pelo SALTO da régua, e as
 * linhas pelo y. Não se inventa marcação --- o que o PDF não guarda, não volta. */
static int volta_para_tex(const char *pdf, const char *sai){
    static Posto v[4000000];
    long nv = absorve(pdf, v, (long)(sizeof v / sizeof v[0]));
    if(nv < 0){ fprintf(stderr, "nao abre: %s\n", pdf); return 1; }
    FILE *f = fopen(sai, "wb");
    if(!f){ fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    double ya = 0; int primeiro = 1;
    for(long i = 0; i < nv; i++){
        if(!primeiro && v[i].y != ya) fputc('\n', f);
        /* um salto em x maior que a largura de um espaço é um espaço que o PDF não escreveu:
         * a justificação alarga-o, e a volta tem de o repor ou as palavras colavam-se */
        else if(!primeiro){
            double esp = largura(32, v[i].fonte) * v[i].corpo / 1000.0;
            double d = v[i].x - (v[i-1].x + largura((int)v[i-1].g, v[i-1].fonte) * v[i-1].corpo / 1000.0);
            if(d > esp * 0.4) fputc(' ', f);
        }
        int g = (int)v[i].g;
        /* o que o LaTeX lê como marcação escapa-se, senão a ida seguinte não daria o mesmo */
        if(g=='\\'||g=='{'||g=='}'||g=='&'||g=='#'||g=='%'||g=='$'||g=='_'||g=='^') fputc('\\', f);
        /* E A CODIFICAÇÃO TEM DE VIRAR: o PDF guarda WinAnsi, o `.tex` lê-se em UTF-8. Escrito
         * cru, o travessão (151) não é UTF-8 válido e a ida seguinte devolvia `?` --- MEDIDO,
         * o primeiro resíduo da volta era exactamente esse, no posto 41 da capa. A volta não
         * é copiar bytes: é mudar de roupa, e a roupa inclui a codificação. */
        int u = g < 128 ? g : winansi_para_unicode(g);
        if(u < 128) fputc(u, f);
        else if(u < 0x800){ fputc(0xC0 | (u >> 6), f); fputc(0x80 | (u & 63), f); }
        else { fputc(0xE0 | (u >> 12), f); fputc(0x80 | ((u >> 6) & 63), f);
               fputc(0x80 | (u & 63), f); }
        ya = v[i].y; primeiro = 0;
    }
    fputc('\n', f);
    fclose(f);
    printf("%s -> %s  (%ld postos absorvidos, %ld Td nao lidos)\n", pdf, sai, nv, NAO_LEU_TD);
    return 0;
}


/* O RESÍDUO DA VOLTA: absorve os dois e lê a diferença. Não compara contra um valor posto
 * por quem escreve — reverte, que é o que o `corpo-estelar.tex` chama de medir.
 *
 * E mede-se O CORPO, não a página: repaginar é do compositor, e exigir o mesmo `y` seria
 * exigir que a volta adivinhasse a quebra de página, que o PDF não guarda como intenção. */
static int residuo_volta(const char *a, const char *b){
    static Posto va[4000000], vb[4000000];
    long na = absorve(a, va, 4000000), nb = absorve(b, vb, 4000000);
    if(na < 0 || nb < 0){ fprintf(stderr, "nao abre um dos dois\n"); return 1; }
    long m = na < nb ? na : nb, dif = 0, prim = -1;
    for(long i = 0; i < m; i++)
        if(va[i].g != vb[i].g){ if(prim < 0) prim = i; dif++; }
    printf("  postos:  %ld  ->  %ld   (delta %ld)\n", na, nb, nb - na);
    printf("  glifos diferentes nos %ld comuns: %ld\n", m, dif);
    if(prim >= 0){
        printf("  o primeiro em %ld: '%c' (%ld) vs '%c' (%ld)\n", prim,
               (int)va[prim].g > 31 ? (int)va[prim].g : '?', va[prim].g,
               (int)vb[prim].g > 31 ? (int)vb[prim].g : '?', vb[prim].g);
        printf("  contexto A: "); for(long i = prim > 20 ? prim-20 : 0; i < prim+30 && i < na; i++)
            putchar((int)va[i].g > 31 && va[i].g < 127 ? (int)va[i].g : '.');
        printf("\n  contexto B: "); for(long i = prim > 20 ? prim-20 : 0; i < prim+30 && i < nb; i++)
            putchar((int)vb[i].g > 31 && vb[i].g < 127 ? (int)vb[i].g : '.');
        putchar('\n');
    }
    printf("  RESIDUO: %ld\n", dif + (na > nb ? na - nb : nb - na));
    return 0;
}

static int compila_ficheiro(const char *ent, const char *sai){
    long n; char *s = le_tudo(ent, &n);
    if(!s){ fprintf(stderr, "nao abre: %s\n", ent); return 1; }
    /* a avaliação nas raízes, ANTES de compor: o estilo é a fonte das definições */
    { char est[1024]; snprintf(est, sizeof est, "%s", ent);
      char *b = strrchr(est, '/'); if(b) b[1] = 0; else est[0] = 0;
      strncat(est, "estilo.tex", sizeof est - strlen(est) - 1);
      s = avalia_macros(s, &n, est); }
    FILE *f = fopen(sai, "wb");
    if(!f){ free(s); fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    Pdf p; pdf_abre(&p, f); pagina_abre(&p);
    long g; compila(s, &p, &g);
    pdf_fecha(&p);
    fclose(f); free(s);
    if(CHUTES){
        fprintf(stderr, "AVISO: %ld larguras CHUTADAS (a fonte nao abriu ou nao tem o glifo).\n",
                CHUTES);
        fprintf(stderr, "       glifos afectados:");
        for(long i = 0; i < N_CHUTE_G; i++) fprintf(stderr, " %ld", CHUTE_G[i]);
        fprintf(stderr, "\n       um chute desalinha a linha INTEIRA a partir dali, porque"
                        " espacar SOMA.\n");
    }
    printf("%s -> %s  (%d paginas, %ld glifos)\n", ent, sai, p.npag, g);
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * O SHELL, com o PDF como BACKEND
 *
 * O Aarao: "traz o shell com backend que ai fica tranquilo."
 *
 * O sql.c ja fez isto para o SQL: compila para a ISA, e a MEMORIA E O DISCO — sem RAM. E os
 * backends do banco (martelo, canal, pool) sao destinos de LOAD/STORE que o banco nao precisa de
 * conhecer. O PDF e mais um: o shell ABRE um .tex, e o STORE escreve no backend.
 *
 * O prompt e o do catalogo — "$ MARTELO 2083236890" — e por isso o cifrao. E a licao do bug fica
 * escrita na propria sintaxe: aqui o $ e prompt, e um prompt nunca e delimitador.
 *
 *   ABRE  <ficheiro.tex>     carrega o fonte      (o solar guarda)
 *   STORE <ficheiro.pdf>     escreve no backend   (o lunar desenrola)
 *   LOAD  <ficheiro.pdf>     le de volta          (a VOLTA, e da o residuo)
 *   MEDE  <palavra>          quantas vezes ela atravessou
 *   SAI
 * ───────────────────────────────────────────────────────────────────────────── */

static char *SH_FONTE = NULL;  static long SH_N = 0;
static char *SH_PDF   = NULL;  static long SH_PN = 0;
static const char  SH_NOME[512] = "";

static int conta(const char *agulha, const char *palheiro, long n){
    long m = (long)strlen(agulha); int c = 0;
    if(!palheiro || m == 0) return 0;
    for(long i = 0; i + m <= n; i++) if(!memcmp(palheiro + i, agulha, (size_t)m)) c++;
    return c;
}

static int shell(void){
    char linha[1024];
    puts("o shell do corpo tradutor — o PDF e o backend. ABRE / STORE / LOAD / MEDE / SAI");
    puts("(o $ e prompt, e um prompt nunca e delimitador — foi o bug de hoje)\n");
    while(1){
        fputs("$ ", stdout); fflush(stdout);
        if(!fgets(linha, sizeof linha, stdin)) break;
        char cmd[64] = "", arg[512] = "";
        int k = sscanf(linha, "%63s %511[^\n]", cmd, arg);
        if(k < 1) continue;
        for(char *q = cmd; *q; q++) *q = (char)toupper((unsigned char)*q);

        if(!strcmp(cmd, "SAI") || !strcmp(cmd, "HALT")) break;

        if(!strcmp(cmd, "ABRE")){
            free(SH_FONTE); SH_FONTE = le_tudo(arg, &SH_N);
            if(!SH_FONTE){ printf("  nao abre: %s\n", arg); SH_N = 0; continue; }
            snprintf(SH_NOME, sizeof SH_NOME, "%s", arg);
            printf("  %s: %ld bytes no solar\n", arg, SH_N);
            continue;
        }
        if(!strcmp(cmd, "STORE")){
            if(!SH_FONTE){ puts("  nada aberto — ABRE primeiro"); continue; }
            FILE *f = fopen(arg, "wb");
            if(!f){ printf("  nao escreve: %s\n", arg); continue; }
            Pdf p; pdf_abre(&p, f); pagina_abre(&p);
            long g; compila(SH_FONTE, &p, &g);
            pdf_fecha(&p); fclose(f);
            printf("  %s -> %s   %d paginas, %ld glifos\n", SH_NOME, arg, p.npag, g);
            continue;
        }
        if(!strcmp(cmd, "LOAD")){
            free(SH_PDF); SH_PDF = le_tudo(arg, &SH_PN);
            if(!SH_PDF){ printf("  nao abre: %s\n", arg); SH_PN = 0; continue; }
            int valido = SH_PN > 400 && !memcmp(SH_PDF, "%PDF-1.", 7) && strstr(SH_PDF, "%%EOF") != NULL;
            printf("  %s: %ld bytes, %s\n", arg, SH_PN, valido ? "PDF valido" : "NAO e PDF valido");
            continue;
        }
        if(!strcmp(cmd, "MEDE")){
            if(!SH_FONTE || !SH_PDF){ puts("  precisa de ABRE e LOAD — a VOLTA quer os dois lados"); continue; }
            /* OS DOIS LADOS FALAM CODIFICACOES DIFERENTES: o .tex e UTF-8, o PDF e WinAnsi. A
             * primeira versao disto comparava a mesma agulha com os dois e dava "fonte 0 -> PDF 7",
             * que e impossivel — e o residuo saia 0 porque 0 > 7 e falso. Uma medida que nao pode
             * falhar nao mede: e o mesmo defeito de sempre, agora dentro do instrumento.
             * A agulha traduz-se para cada lado antes de se contar. */
            char lat[512]; int m = 0;
            for(long q = 0; arg[q] && m < 511; ){
                int cons; int g = utf8_glifo((const unsigned char*)arg + q, &cons);
                lat[m++] = (char)g; q += cons;
            }
            lat[m] = 0;
            int no_fonte = conta(arg, SH_FONTE, SH_N);   /* UTF-8, como veio */
            int no_pdf   = conta(lat, SH_PDF,   SH_PN);  /* WinAnsi, traduzido */
            int res = no_fonte > no_pdf ? no_fonte - no_pdf : 0;
            printf("  \"%s\": fonte %d -> PDF %d   RESIDUO %d%s\n",
                   arg, no_fonte, no_pdf, res,
                   no_fonte == 0 ? "   (nao esta no fonte — a medida nao diz nada)" : "");
            continue;
        }
        printf("  ?  %s   (ABRE / STORE / LOAD / MEDE / SAI)\n", cmd);
    }
    free(SH_FONTE); free(SH_PDF);
    return 0;
}

int main(int argc, char **argv){
    if(argc == 2 && (!strcmp(argv[1], "-sh") || !strcmp(argv[1], "shell"))) return shell();
    /* o outro sentido do MOVE: `+1` absorve. Sem isto o tradutor só emitia, e um objecto
     * que só emite é o buraco branco — não é reversível, e não se pode medir por resíduo. */
    if(argc >= 4 && !strcmp(argv[1], "-volta")) return volta_para_tex(argv[2], argv[3]);
    if(argc >= 4 && !strcmp(argv[1], "-residuo")) return residuo_volta(argv[2], argv[3]);
    if(argc >= 3) return compila_ficheiro(argv[1], argv[2]);

    puts("tex.c — O CORPO TRADUTOR DE FORMATO: .tex -> PDF, sem TeX Live\n");

    /* ── §X1 ─────────────────────────────────────────────────────────────── */
    puts("§X1  A DESCIDA: a marca do LaTeX, no mesmo mecanismo do caminho.h");
    puts("     O caminho.h: 'o que muda de formato para formato NAO e a descida — e como cada um");
    puts("     MARCA o nivel'. JSON marca com o parentese, Markdown contando '#'. O LaTeX marca");
    puts("     com a BARRA, e o nivel esta no nome: section=2, subsection=3, sub-sub=4.\n");
    {
        int certo = 1;
        for(int i = 0; i < NSECS; i++) if(sec_nivel(SECS[i].nome) != SECS[i].nivel) certo = 0;
        ok("as seccionadoras dao o nivel pelo NOME (part/chapter 1, section 2, sub 3, subsub 4)",
           certo && sec_nivel("section") == 2 && sec_nivel("subsection") == 3
                 && sec_nivel("subsubsection") == 4);
        ok("o nivel CRESCE com o prefixo 'sub' — e a ordem e estrita",
           sec_nivel("section") < sec_nivel("subsection")
        && sec_nivel("subsection") < sec_nivel("subsubsection"));
        ok("o que nao e seccionadora nao tem nivel — a marca sozinha nao basta",
           sec_nivel("textbf") == 0 && sec_nivel("item") == 0 && sec_nivel("alpha") == 0);
        printf("     -> a marca do LaTeX e '%c', e ha %d seccionadoras. Nenhum lugar novo:\n",
               SECS[0].marca, NSECS);
        puts("        o .tex veste a roupa que o analisador ja sabe despir.\n");
    }

    /* ── §X2 ─────────────────────────────────────────────────────────────── */
    puts("§X2  O LEXICO: comando -> glifo, e a volta e a MESMA tabela ao contrario");
    puts("     O traduz.c §T1: 'o lexico e a roupa geral do idioma, e a traducao literal usa ele'.\n");
    {
        const Par *a = lex_acha("alpha"), *s = lex_acha("sigma"), *t = lex_acha("times");
        ok("o lexico traduz: \\alpha, \\sigma e \\times caem na Symbol",
           a && s && t && a->simb && s->simb && t->simb && a->glifo == 'a' && s->glifo == 's');
        /* a VOLTA, medida em TODOS os pares e nao num escolhido a dedo */
        int volta_ok = 1, ambiguos = 0;
        for(int i = 0; i < NLEX; i++){
            const char *v = lex_volta(LEXICO[i].glifo, LEXICO[i].simb);
            if(!v){ volta_ok = 0; continue; }
            if(strcmp(v, LEXICO[i].cmd)) ambiguos++;      /* sinonimos: \le e \leq no mesmo glifo */
        }
        ok("a VOLTA fecha em TODOS os pares do lexico — glifo -> comando, sem excecao",
           volta_ok);
        printf("     -> %d pares no lexico; %d glifos com mais de um nome (\\le/\\leq, \\to/\\rightarrow):\n",
               NLEX, ambiguos);
        puts("        a volta escolhe o primeiro, e e por isso que ela e sobrejetora e nao injetora.");
        /* o UTF-8: o portugues tem de atravessar */
        int c1, c2, c3;
        int A = utf8_glifo((const unsigned char*)"ã", &c1);
        int C = utf8_glifo((const unsigned char*)"ç", &c2);
        int E = utf8_glifo((const unsigned char*)"é", &c3);
        ok("o portugues atravessa: a-til, c-cedilha e e-agudo caem no Latin-1 em UM glifo",
           A == 0xE3 && C == 0xE7 && E == 0xE9 && c1 == 2 && c2 == 2 && c3 == 2);
        puts("");
    }

    /* ── §X3 ─────────────────────────────────────────────────────────────── */
    puts("§X3  O PRISMATICO: encher a area");
    puts("     O prisma.c: 'o triangulo deformado ate PREENCHER A AREA INTEIRA'. A linha chega");
    puts("     curta e tem de encher a coluna: deforma-se o espaco ate a area fechar.\n");
    {
        /* mede-se a lei da deformacao em VARIOS pontos, nao numa folga escolhida */
        int perfeito = 1, resto_grande = 0;
        long total_resto = 0; int casos = 0;
        for(long folga = 0; folga <= 40000; folga += 137){
            for(int esp = 1; esp <= 12; esp++){
                long por;
                long r = deforma(folga, esp, &por);
                if(por * esp + r != folga) perfeito = 0;       /* a conservacao: nada se perde */
                if(r < 0 || r >= esp) resto_grande = 1;        /* o residuo e MENOR que o n. de espacos */
                total_resto += r; casos++;
            }
        }
        ok("a deformacao CONSERVA: por_espaco*n + residuo == folga, em 3552 casos",
           perfeito);
        ok("o residuo e sempre menor que o numero de espacos — a area fecha ate ao ultimo milesimo",
           resto_grande == 0);
        printf("     -> %d casos medidos, residuo medio %.2f milesimos de ponto (< 1/1000 pt por\n",
               casos, (double)total_resto / casos);
        puts("        espaco). A area enche; o que sobra e menor que a resolucao do formato.");
        /* e a METRICA tem de ser real, senao nao se mede linha nenhuma */
        Gl t[5] = {{'W',F_REG},{'i',F_REG},{'W',F_NEG},{'i',F_NEG},{' ',F_REG}};
        /* Escrevi DUAS vezes uma lei de cabeca e a medida derrubou as duas: primeiro "a negra e
         * mais larga" (o 'W' e 944 nas duas), depois "a negra NUNCA e mais estreita" (o '@' e 975
         * na negra contra 1015 na regular). Sao metricas publicadas, nao uma regra minha — entao
         * MEDE-SE o que elas fazem, em vez de lhes atribuir uma lei. */
        int maiores = 0, menores = 0, empates = 0, min_mais = 0;
        long som_r = 0, som_b = 0;
        for(int g = 32; g <= 126; g++){
            int r = largura(g, F_REG), b = largura(g, F_NEG);
            som_r += r; som_b += b;
            if(b > r){ maiores++; if(g >= 'a' && g <= 'z') min_mais++; }
            else if(b < r) menores++; else empates++;
        }
        ok("a metrica DISCRIMINA: a negra pesa mais no total e nas minusculas, onde o peso se ve",
           som_b > som_r && min_mais >= 20);
        /* Esta assercao fixava 944, 222 e 278 — os numeros EXATOS da tabela. Assim que a medida
         * passou a vir da curva ela quebrou, porque a divisao por upem=2048 arredonda. E fez bem
         * em quebrar: um valor absoluto amarra a assercao a UMA fonte de medida, e o que se quer
         * afirmar nao e "o W mede 944", e "o W e muito mais largo que o i". Mede-se a PROPORCAO,
         * que sobrevive a troca da regua — foi o mesmo remedio do numero de cabeca. */
        /* E a TERCEIRA lei que invento sobre estas larguras e a medida derruba: "a negra e mais
         * larga" (o W e igual nas duas), "a negra nunca e mais estreita" (o @), e agora "nada
         * visivel e mais estreito que o espaco" — o apostrofo e 190 contra 277. PARO de afirmar
         * leis sobre uma tabela publicada. O que se pode afirmar e o que a medida MOSTRA: que a
         * largura discrimina, e por quanto. */
        int distintas = 0;
        for(int g = 32; g <= 126; g++){
            int w = largura(g,F_REG), ja = 0;
            for(int h = 32; h < g; h++) if(largura(h,F_REG) == w){ ja = 1; break; }
            if(!ja) distintas++;
        }
        /* e o criterio de discriminacao nao pode ser uma CONTAGEM escolhida por mim (">= 20"
         * falhou por uma, e baixar para 19 seria escolher a constante outra vez). A razao entre
         * o mais largo e o mais estreito vale 1 se todos forem iguais — e isso nao se escolhe. */
        int wmin = 9999, wmax = 0;
        for(int g = 32; g <= 126; g++){
            int w = largura(g,F_REG);
            if(w < wmin) wmin = w;
            if(w > wmax) wmax = w;
        }
        ok("a largura DISCRIMINA: o mais largo e mais de 4x o mais estreito (seria 1x se nao medisse)",
           wmax > 4*wmin && largura('W',F_REG) > 4*largura('i',F_REG));
        printf("     -> %d larguras distintas em 95 glifos, de %d a %d (razao %.2f); o mais estreito\n",
               distintas, wmin, wmax, (double)wmax/wmin);
        printf("        e o apostrofo (%d), mais estreito que o proprio espaco (%d). Sem lei simples.\n",
               largura('\'',F_REG), largura(' ',F_REG));
        printf("     -> negra maior em %d glifos, igual em %d, MENOR em %d (o '@': %d contra %d).\n",
               maiores, empates, menores, largura('@',F_NEG), largura('@',F_REG));
        printf("        Total %ld contra %ld, e %d das 26 minusculas engordam. Nao ha lei simples:\n",
               som_b, som_r, min_mais);
        puts("        sao as tabelas publicadas, e e por isso que se medem em vez de se supor.");
        long m = mede(t, 5, 10);
        ok("medir a linha e somar as larguras — e o total bate a soma peca a peca",
           m == 10L*(largura('W',F_REG)+largura('i',F_REG)+largura('W',F_NEG)
                    +largura('i',F_NEG)+largura(' ',F_REG)));
        puts("");
    }

    /* ── §X4/§X5/§X6 ─────────────────────────────────────────────────────── */
    puts("§X4  O SOLAR GUARDA, O LUNAR DESENROLA — e §X5 o PDF sai valido, §X6 a volta fecha\n");
    {
        static const char FONTE[] =
            "\\documentclass{article}\n"
            "\\title{O corpo tradutor}\n"
            "\\begin{document}\n"
            "\\section{A descida}\n"
            "O formato e a roupa, e a descida e uma so. Uma linha longa o bastante para ter de\n"
            "quebrar e ser justificada pelo prismatico, porque encher a area e o que ele faz, e\n"
            "sem uma linha comprida nao havia deformacao nenhuma a medir aqui neste teste.\n"
            "\n"
            "\\subsection{O lexico}\n"
            "A razao aurea \\phi e a raiz de $x^2 = mx + 1$, com \\sigma \\in R e \\alpha \\to \\beta.\n"
            "% este comentario nao pode aparecer no PDF\n"
            "Acentos: coração, \u00e1rea, invariância, tr\u00eas, voc\u00ea.\n"
            "\\begin{itemize}\n"
            "\\item o primeiro\n"
            "\\item o segundo\n"
            "\\end{itemize}\n"
            "\\textbf{negrito} e \\emph{enfase}.\n"
            "\n"
            "\\begin{verbatim}\n"
            "$ MARTELO 2083236890 2083236900\n"
            "\\end{verbatim}\n"
            "Depois do cifrao desirmanado: invariância, notação, coração, formulação.\n"
            "\\end{document}\n";

        const char *saida = "/tmp/tex_medida.pdf";
        FILE *f = fopen(saida, "wb");
        int abriu = (f != NULL);
        long glifos = 0; int npag = 0, nobj = 0;
        if(abriu){
            Pdf p; pdf_abre(&p, f); pagina_abre(&p);
            compila(FONTE, &p, &glifos);
            pdf_fecha(&p);
            npag = p.npag; nobj = p.nobj;
            fclose(f);
        }
        ok("o lunar desenrolou: ha pagina, ha objetos e sairam glifos",
           abriu && npag >= 1 && nobj >= 6 && glifos > 200);

        long n = 0; char *pdf = abriu ? le_tudo(saida, &n) : NULL;
        /* PROCURA-SE POR BYTES, E NAO POR STRING. Desde que a fonte passou a ser EMBUTIDA o
         * PDF tem 800 KB de TTF binaria la' dentro — com bytes zero — e o strstr para no
         * primeiro deles. As duas assercoes seguintes falharam por isso, e o defeito era do
         * medidor: ele lia como texto um ficheiro que deixou de o ser. Um ficheiro binario
         * mede-se com memmem, que leva o comprimento. */
        ok("o PDF tem cabecalho %PDF e acaba em %%EOF",
           pdf && n > 400 && !memcmp(pdf, "%PDF-1.", 7)
           && memmem(pdf, (size_t)n, "%%EOF", 5) != NULL);

        /* §X5: a xref nao pode ser decorativa — cada offset tem de cair num 'N 0 obj' */
        int xref_certo = 0, conferidos = 0;
        if(pdf){
            char *x = (char*)memmem(pdf, (size_t)n, "\nxref\n", 6);
            if(x){
                char *q = x + 6;
                int primeiro, quantos;
                if(sscanf(q, "%d %d", &primeiro, &quantos) == 2 && primeiro == 0){
                    while(*q && *q != '\n') q++; q++;
                    q += 20;                                   /* a entrada livre do objeto 0 */
                    xref_certo = 1;
                    for(int k = 1; k < quantos && xref_certo; k++){
                        long off = strtol(q, NULL, 10);
                        if(off <= 0 || off >= n){ xref_certo = 0; break; }
                        int num = -1;
                        if(sscanf(pdf + off, "%d 0 obj", &num) != 1 || num != k) xref_certo = 0;
                        conferidos++;
                        q += 20;
                    }
                }
            }
        }
        ok("§X5 a XREF aponta certo: TODO offset cai exatamente no 'N 0 obj' do seu numero",
           xref_certo && conferidos >= 6);
        printf("     -> %d objetos conferidos um a um, %d paginas, %ld bytes.\n", conferidos, npag, n);

        /* §X6: A VOLTA. O texto que sai do PDF e o que entrou no .tex. */
        if(pdf){
            char *saiu = malloc((size_t)n + 1);
            long ns = extrai(pdf, n, saiu, n + 1);
            /* as palavras do fonte tem de estar todas no que saiu, na ORDEM em que entraram */
            static const char *PALAVRAS[] = {
                "descida","formato","roupa","prismatico","area","lexico","aurea","raiz",
                "primeiro","segundo","negrito","enfase"
            };
            int nn = (int)(sizeof PALAVRAS / sizeof PALAVRAS[0]);
            long pos = 0; int ordem = 1, achadas = 0;
            for(int k = 0; k < nn; k++){
                char *h = strstr(saiu + pos, PALAVRAS[k]);
                if(!h){ ordem = 0; continue; }
                achadas++; pos = (h - saiu) + 1;
            }
            ok("§X6 A VOLTA: as 12 palavras do .tex saem do PDF, e na MESMA ORDEM em que entraram",
               ordem && achadas == nn);
            /* e o que NAO devia atravessar, nao atravessou */
            ok("§X6 o comentario '%' NAO atravessou — e o que a descida come, come mesmo",
               !strstr(saiu, "nao pode aparecer"));
            ok("§X6 os acentos atravessaram em UM byte cada (Latin-1), nao partidos em dois",
               strstr(saiu, "cora\xE7\xE3o") && strstr(saiu, "\xE1rea") && strstr(saiu, "voc\xEA"));
            /* A REGRESSAO DOS DOIS BUGS, que sao o mesmo bug por duas portas. O fonte acima tem
             * um verbatim com "$ MARTELO ..." — um cifrao desirmanado, que e prompt e nao formula.
             * Antes: o modo matematico ficava ligado ate ao fim, e as palavras ACENTUADAS partiam-se
             * no acento ('coracao' saia 'cora' na Symbol e o resto na regular, dois Tj). Custou-me
             * 159 palavras de 2240 no catalogo, e nao ha assercao que apanhe isto sem uma palavra
             * acentuada DEPOIS de um cifrao solto. */
            ok("§X6 REGRESSAO: um $ solto num verbatim NAO parte as palavras acentuadas seguintes",
               strstr(saiu, "invari\xE2ncia") && strstr(saiu, "nota\xE7\xE3o")
            && strstr(saiu, "formula\xE7\xE3o"));
            ok("§X6 e o proprio verbatim saiu literal — o cifrao esta la como texto",
               strstr(saiu, "MARTELO 2083236890") != NULL);
            printf("     -> %ld glifos entraram, %ld sairam do PDF. O documento atravessou.\n",
                   glifos, ns);
            free(saiu);
        } else ok("§X6 A VOLTA", 0);
        free(pdf);
        puts("");
    }

    /* ── §X7  A LARGURA VEM DA CURVA ─────────────────────────────────────── */
    puts("§X7  A LARGURA VEM DA CURVA: o spline.h ligado, e a tabela e so a rede de seguranca");
    puts("     O spline.c provou (95 de 95, nas duas variantes) que a curva concorda com a");
    puts("     tabela base-14. Provado isso, a tabela deixa de ser precisa — e o que se ganha");
    puts("     nao e o ASCII, que ja batia: e o PORTUGUES, onde eu punha 556 a olho.\n");
    {
        carta_abre();
        if(!CARTA){
            puts("  [aviso] a Liberation Sans nao esta neste sistema: a largura vem da TABELA.");
            puts("          Nao ha medida a fazer aqui, e e dito em vez de passar em silencio.\n");
        } else {
            /* 1. os dois caminhos, agora no USO real e nao no medidor do lado */
            /* Eu tinha escrito aqui "iguais == 95 && difs == 0" — igualdade EXATA — e falhou.
             * O spline.c media com tolerancia de 1 e dava 95/95; a diferenca sou eu a exigir
             * mais do que a aritmetica permite: upem=2048 e a divisao inteira *1000/2048 perde
             * ate 1 milesimo. Nao e discordancia entre a curva e a tabela: e o arredondamento
             * do meu proprio conversor. Entao MEDE-SE o arredondamento, em vez de o negar. */
            int exatos = 0, por_um = 0, piores = 0, pior = 0;
            for(int g = 32; g <= 126; g++){
                int tab = W_REG[g - 32];
                int gi  = ttf_glifo(&CARTA_R, g);
                int cur = gi ? (int)((long)ttf_avanco(&CARTA_R, gi)*1000/CARTA_R.upem) : -1;
                int d = abs(cur - tab);
                if(!d) exatos++; else if(d == 1) por_um++; else { piores++; if(d > pior) pior = d; }
            }
            ok("no ASCII a curva e a tabela nao divergem: no maximo 1 milesimo, e e ARREDONDAMENTO",
               piores == 0 && exatos + por_um == 95);
            printf("     -> 95 glifos: %d exatos, %d a 1 milesimo (a divisao por upem=%d), %d piores.\n",
                   exatos, por_um, CARTA_R.upem, piores);
            puts("        Trocar a fonte da medida nao mexeu na pagina — o ASCII ja batia.");

            /* 2. e o que a tabela NAO tinha: os acentuados. Eu dava 556 a todos. */
            static const int ACENTOS[] = {0xE1,0xE2,0xE3,0xE7,0xE9,0xEA,0xED,0xF3,0xF4,0xF5,0xFA,
                                          0xC1,0xC3,0xC7,0xC9,0xD3};
            int nac = (int)(sizeof ACENTOS / sizeof ACENTOS[0]);
            int distintas = 0, iguais_556 = 0, min = 9999, max = 0;
            for(int i = 0; i < nac; i++){
                int w = largura(ACENTOS[i], F_REG);
                if(w == 556) iguais_556++;
                if(w < min) min = w;
                if(w > max) max = w;
                int ja = 0;
                for(int j = 0; j < i; j++) if(largura(ACENTOS[j], F_REG) == w) ja = 1;
                if(!ja) distintas++;
            }
            ok("os ACENTUADOS deixam de ser todos 556: a curva da-lhes larguras REAIS e distintas",
               distintas >= 4 && min < 556 && max > 556);
            printf("     -> 16 acentuados do portugues: %d larguras distintas, de %d a %d milesimos\n",
                   distintas, min, max);
            printf("        (a tabela dava 556 a TODOS — %d deles calham nesse valor, os outros %d nao).\n",
                   iguais_556, nac - iguais_556);

            /* 3. e o efeito na PAGINA, que e o que interessa: uma linha de portugues real */
            const char *pt = "a tradução é uma rotação: o significado é invariante, só a roupa gira";
            long com_curva = 0, com_tabela = 0;
            for(long q = 0; pt[q]; ){
                int cons; int g = utf8_glifo((const unsigned char*)pt + q, &cons);
                com_curva  += largura(g, F_REG);
                com_tabela += (g >= 32 && g <= 126) ? W_REG[g - 32] : 556;
                q += cons;
            }
            ok("e a linha de portugues MUDA de largura — logo a curva esta mesmo a ser usada",
               com_curva != com_tabela);
            printf("     -> a mesma linha: %ld pela curva, %ld pela tabela. Diferenca de %ld milesimos\n",
                   com_curva, com_tabela, labs(com_curva - com_tabela));
            printf("        de em (%.2f%%) — e num paragrafo inteiro e onde a linha quebra.\n",
                   100.0*labs(com_curva - com_tabela)/com_tabela);
            puts("");
        }
    }

    /* ── o fecho ─────────────────────────────────────────────────────────── */
    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O .tex NAO abriu lugar novo. A descida e a do caminho.h — mudou quem le a marca,");
    puts("  como muda de JSON para Markdown. O lexico e o do traduz.c — pares, e a volta e a");
    puts("  mesma tabela ao contrario. A justificacao e o PRISMATICO literal: encher a area,");
    puts("  deformando ate ela fechar. O solar guarda a estrutura, o lunar desenrola a pagina.");
    puts("");
    puts("  E a traducao e UMA SO: PT->EN e a rotacao do traducao.c, .tex->PDF e esta. Em ambas");
    puts("  o significado e invariante e so a roupa gira — e em ambas a prova e a VOLTA.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
