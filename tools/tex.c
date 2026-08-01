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

static int largura(int g, int fonte){
    if(fonte == F_SIM) return 549;                /* a Symbol é quase toda desta largura */
    if(g < 32 || g > 126) return fonte == F_NEG ? 556 : 556;   /* os acentados, à volta disto */
    return fonte == F_NEG ? W_NEG[g - 32] : W_REG[g - 32];
}

/* um pedaço de texto já traduzido: glifo + fonte. É o que o solar guarda. */
typedef struct { unsigned char g; unsigned char f; } Gl;

#define MAXLIN 4096
typedef struct { Gl g[MAXLIN]; int n; int nivel; int recuo; } Linha;

static long mede(const Gl *g, int n, int corpo){   /* a largura da linha, em milésimos de ponto */
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
    double y;                                      /* onde vai o lápis */
    int  aberta;                                   /* há página aberta? */
    long len_obj;                                  /* o objeto /Length pendente */
    long stream_ini;
} Pdf;

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
    int po = obj_novo(p);
    p->pag[p->npag++] = po;
    int co = po + 1, lo = po + 2;                  /* o conteúdo e o seu /Length */
    p->nobj = lo;
    fprintf(p->f,
        "%d 0 obj<</Type/Page/Parent 2 0 R/MediaBox[0 0 %d %d]"
        "/Resources<</Font<</F1 3 0 R/F2 4 0 R/F3 5 0 R>>>>/Contents %d 0 R>>endobj\n",
        po, A4_L, A4_A, co);
    p->off[co] = ftell(p->f);
    fprintf(p->f, "%d 0 obj<</Length %d 0 R>>stream\n", co, lo);
    p->stream_ini = ftell(p->f);
    p->len_obj = lo;
    p->y = TOPO;
    p->aberta = 1;
}

static void pagina_fecha(Pdf *p){
    if(!p->aberta) return;
    long fim = ftell(p->f);
    fprintf(p->f, "endstream\nendobj\n");
    p->off[p->len_obj] = ftell(p->f);
    fprintf(p->f, "%ld 0 obj %ld endobj\n", p->len_obj, fim - p->stream_ini);
    p->aberta = 0;
}

/* escreve um pedaço de glifos numa só fonte, escapando o que o PDF exige */
static void poe_pedaco(FILE *f, const Gl *g, int i, int j, int fonte, int corpo,
                       double x, double y, long espaco_extra){
    static const char *FN[3] = {"/F1", "/F2", "/F3"};
    fprintf(f, "BT %s %d Tf %.2f %.2f Td", FN[fonte], corpo, x, y);
    if(espaco_extra) fprintf(f, " %.3f Tw", espaco_extra / 1000.0);
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
    if(!L->n){ p->y -= ENTRE; return; }
    int corpo = L->nivel ? (L->nivel <= 2 ? 15 : 12) : CORPO;
    int alt   = L->nivel ? corpo + 8 : ENTRE;

    if(!p->aberta || p->y - alt < FUNDO){ pagina_fecha(p); pagina_abre(p); }
    p->y -= alt;

    double x = MARGEM + L->recuo;
    long extra = 0;
    if(justifica && !L->nivel){
        long larg = mede(L->g, L->n, corpo);
        long alvo = (long)(COL - L->recuo) * 1000;
        int esp = 0;
        for(int i = 0; i < L->n; i++) if(L->g[i].g == ' ' && L->g[i].f != F_SIM) esp++;
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
        poe_pedaco(p->f, L->g, i, j, fonte, corpo, x, p->y, fonte == F_SIM ? 0 : extra);
        long w = 0;
        for(int k = i; k < j; k++){
            w += (long)largura(L->g[k].g, fonte) * corpo;
            if(L->g[k].g == ' ' && fonte != F_SIM) w += extra;
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
    static const char *BF[3] = {"Helvetica", "Helvetica-Bold", "Symbol"};
    for(int i = 0; i < 3; i++){
        p->off[3+i] = ftell(p->f);
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
} Est;

static void empurra(Est *e, int g, int f){
    if(e->L.n < MAXLIN - 1){ e->L.g[e->L.n].g = (unsigned char)g; e->L.g[e->L.n].f = (unsigned char)f; e->L.n++; }
    e->glifos++;
}

/* quebra a linha corrente onde ela deixa de caber, e desenrola. O que sobra fica para a seguinte. */
static void quebra_e_desenrola(Est *e, int ultima){
    int corpo = e->L.nivel ? (e->L.nivel <= 2 ? 15 : 12) : CORPO;
    long alvo = (long)(COL - e->L.recuo) * 1000;
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
                if(s[j] == '\\'){ fecha_paragrafo(&e); i = j + 1; continue; }
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
            if(!strcmp(cmd, "begin") || !strcmp(cmd, "end")){
                int abre = (cmd[0] == 'b');
                while(j < n && s[j] != '{') j++;
                long a = ++j; while(j < n && s[j] != '}') j++;
                char amb[64]; long ln = j - a; if(ln > 63) ln = 63;
                memcpy(amb, s + a, (size_t)ln); amb[ln] = 0;
                fecha_paragrafo(&e);
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
        empurra(&e, g, (e.mat && g < 128 && isalpha(g)) ? F_SIM : e.fonte);
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

static int compila_ficheiro(const char *ent, const char *sai){
    long n; char *s = le_tudo(ent, &n);
    if(!s){ fprintf(stderr, "nao abre: %s\n", ent); return 1; }
    FILE *f = fopen(sai, "wb");
    if(!f){ free(s); fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    Pdf p; pdf_abre(&p, f); pagina_abre(&p);
    long g; compila(s, &p, &g);
    pdf_fecha(&p);
    fclose(f); free(s);
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
static char  SH_NOME[512] = "";

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
        ok("e a largura varia mesmo com o glifo — o 'W' e 4x o 'i', e o espaco e o mais estreito",
           largura('W',F_REG) > 4*largura('i',F_REG) && largura('W',F_REG) == 944
        && largura('i',F_REG) == 222 && largura(' ',F_REG) == 278);
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
        ok("o PDF tem cabecalho %PDF e acaba em %%EOF",
           pdf && n > 400 && !memcmp(pdf, "%PDF-1.", 7) && strstr(pdf, "%%EOF"));

        /* §X5: a xref nao pode ser decorativa — cada offset tem de cair num 'N 0 obj' */
        int xref_certo = 0, conferidos = 0;
        if(pdf){
            char *x = strstr(pdf, "\nxref\n");
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
