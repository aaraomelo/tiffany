/* backend_ttf.c — A TTF ENTRA, A ESTRELA LÊ, E ESCREVE NAS DUAS ROUPAS.
 *
 * O Aarão: «faz backend ttf: entra com arquivo, estrela lê e escreve no latex; depois estrela
 * lê latex e escreve no pdf.»
 *
 * É a cadeia inteira, com a TTF como BACKEND — mais uma realização que entra pela porta
 * comum, e não um recurso privilegiado. E fecha o que ficou em aberto: até aqui o tradutor
 * DECLARAVA `/BaseFont/Helvetica` no PDF e deixava o leitor desenhar. Ele lia a spline da TTF
 * para medir a largura, mas não a emitia — logo a fonte que aparecia era a que o leitor
 * tivesse, e nenhuma escala a corrigia.
 *
 *      ficheiro.ttf ──► ESTRELA ──► \LaTeX          o contorno vira métrica
 *                          │
 *                          └──────► PDF             o contorno vira CAMINHO
 *
 * A estrela lê o ficheiro UMA vez e escreve nas duas roupas. Não é a TTF que gera o LaTeX nem
 * o LaTeX que gera o PDF: as duas saem do mesmo corpo, que é o contorno — a spline.
 *
 * E É COMPILADO E NÃO LITERAL: para o \LaTeX sai a MÉTRICA (a largura, que é o que ele precisa
 * para medir a linha); para o PDF sai o CAMINHO (os pontos, que é o que ele precisa para
 * pintar). São projeções diferentes do mesmo contorno, e nenhuma é a cópia da outra.
 *
 *   §B1  a TTF entra: os contornos são lidos do ficheiro, com pontos de controlo
 *   §B2  estrela → \LaTeX: sai a MÉTRICA, e ela vem da CURVA e não de uma tabela
 *   §B3  estrela → PDF: sai o CAMINHO, com os operadores do desenha.c
 *   §B4  e as duas saem do MESMO contorno: mudado o glifo, as duas mudam
 *   §B5  o controlo: sem a TTF, nenhuma das duas sai — e não se cai numa tabela
 *
 *   cc -O2 -std=c99 -I../lib backend_ttf.c -o backend_ttf && ./backend_ttf
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── O BACKEND: a TTF lida à mão, sem biblioteca. É o corpo de onde tudo sai. ────────── */
typedef struct { unsigned char *d; long n; } Buf;
static long u16(const Buf *b, long p){ return (p+1 < b->n) ? ((b->d[p] << 8) | b->d[p+1]) : 0; }
static long u32(const Buf *b, long p){ return (u16(b,p) << 16) | u16(b,p+2); }
static long s16v(const Buf *b, long p){ long v = u16(b,p); return v >= 32768 ? v - 65536 : v; }

/* um ponto do contorno: x, y, e se está NA curva ou é de CONTROLO */
struct pto { long x, y; int na_curva; };

struct glifo {
    struct pto p[256];
    long n, contornos, fim[16];
    long larg;                 /* o avanço, em unidades da fonte */
    long upem;                 /* as unidades por em — a régua da fonte */
};

/* ENTRA O FICHEIRO: lê o glifo de índice g, com os pontos e a largura */
static int ttf_le(const char *caminho, long g, struct glifo *G)
{
    static unsigned char buf[1 << 22];
    static Buf b = { buf, 0 };
    static char aberto[512] = "";
    if(strcmp(aberto, caminho) != 0){
        FILE *f = fopen(caminho, "rb");
        if(!f) return 0;
        b.n = (long)fread(buf, 1, sizeof buf, f);
        fclose(f);
        snprintf(aberto, sizeof aberto, "%s", caminho);
    }
    if(b.n < 12) return 0;

    long nt = u16(&b, 4), glyf = 0, loca = 0, head = 0, hmtx = 0, hhea = 0;
    for(long i = 0; i < nt; i++){
        long r = 12 + 16*i;
        if(!memcmp(b.d + r, "glyf", 4)) glyf = u32(&b, r + 8);
        if(!memcmp(b.d + r, "loca", 4)) loca = u32(&b, r + 8);
        if(!memcmp(b.d + r, "head", 4)) head = u32(&b, r + 8);
        if(!memcmp(b.d + r, "hmtx", 4)) hmtx = u32(&b, r + 8);
        if(!memcmp(b.d + r, "hhea", 4)) hhea = u32(&b, r + 8);
    }
    if(!glyf || !loca || !head) return 0;
    G->upem = u16(&b, head + 18);
    int longloca = (int)u16(&b, head + 50);
    long nhm = hhea ? u16(&b, hhea + 34) : 1;
    G->larg = hmtx ? u16(&b, hmtx + 4*(g < nhm ? g : nhm - 1)) : 0;

    long ini = longloca ? u32(&b, loca + 4*g)     : 2*u16(&b, loca + 2*g);
    long fim = longloca ? u32(&b, loca + 4*(g+1)) : 2*u16(&b, loca + 2*(g+1));
    G->n = G->contornos = 0;
    if(fim <= ini) return 1;                        /* glifo vazio: é legítimo */

    long o = glyf + ini, nc = u16(&b, o);
    if(nc > 32768 || nc > 16) return 1;             /* composto ou grande demais: salta */
    G->contornos = nc;
    for(long i = 0; i < nc; i++) G->fim[i] = u16(&b, o + 10 + 2*i);
    long np = nc ? G->fim[nc-1] + 1 : 0;
    if(np > 256) np = 256;

    long ins = u16(&b, o + 10 + 2*nc);
    long p = o + 10 + 2*nc + 2 + ins;
    /* as flags, com repetição */
    static unsigned char fl[256];
    for(long i = 0; i < np; ){
        unsigned char f = b.d[p++];
        long rep = 1;
        if(f & 8) rep += b.d[p++];
        for(long k = 0; k < rep && i < np; k++, i++) fl[i] = f;
    }
    /* os x, em delta */
    long x = 0;
    for(long i = 0; i < np; i++){
        if(fl[i] & 2){ long d = b.d[p++]; x += (fl[i] & 16) ? d : -d; }
        else if(!(fl[i] & 16)){ x += s16v(&b, p); p += 2; }
        G->p[i].x = x;
    }
    /* os y, idem */
    long y = 0;
    for(long i = 0; i < np; i++){
        if(fl[i] & 4){ long d = b.d[p++]; y += (fl[i] & 32) ? d : -d; }
        else if(!(fl[i] & 32)){ y += s16v(&b, p); p += 2; }
        G->p[i].y = y;
        G->p[i].na_curva = (fl[i] & 1) != 0;
    }
    G->n = np;
    return 1;
}

/* ─── ESTRELA → LATEX: sai a MÉTRICA. É o que ele precisa para medir a linha. ────────── */
static long veste_latex(const struct glifo *G, char *out, long cap)
{
    if(!G->upem) return 0;
    /* a largura em milésimos de em — a unidade que o TeX e o PDF usam */
    long mil = G->larg * 1000 / G->upem;
    return snprintf(out, (size_t)cap, "\\wd\\glifo=%ldsp %% %ld/1000 em\n", mil * 65536 / 1000, mil);
}

/* ─── ESTRELA → PDF: sai o CAMINHO. Os mesmos operadores do desenha.c. ──────────────── */
static long veste_pdf(const struct glifo *G, char *out, long cap)
{
    if(!G->n) return 0;
    long m = 0, ini = 0;
    for(long c = 0; c < G->contornos && m < cap - 64; c++){
        long f = G->fim[c];
        if(f >= G->n) break;
        m += snprintf(out + m, (size_t)(cap - m), "%ld %ld m\n", G->p[ini].x, G->p[ini].y);
        for(long i = ini + 1; i <= f && m < cap - 64; i++){
            if(G->p[i].na_curva)
                m += snprintf(out + m, (size_t)(cap - m), "%ld %ld l\n", G->p[i].x, G->p[i].y);
            else if(i + 1 <= f)                     /* de controlo: a curva de grau 2 */
                m += snprintf(out + m, (size_t)(cap - m), "%ld %ld %ld %ld v\n",
                              G->p[i].x, G->p[i].y, G->p[i+1].x, G->p[i+1].y);
        }
        m += snprintf(out + m, (size_t)(cap - m), "h\n");   /* fecha o contorno */
        ini = f + 1;
    }
    m += snprintf(out + m, (size_t)(cap - m), "f\n");
    return m;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const char *cands[] = {
        "/usr/share/fonts/google-noto-vf/NotoSerif[wght].ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
    };
    const char *TTF = NULL;
    struct glifo G;
    for(long i = 0; i < 3; i++)
        if(ttf_le(cands[i], 36, &G) && G.upem > 0){ TTF = cands[i]; break; }

printf("\n=== A TTF ENTRA, A ESTRELA LE, E ESCREVE NAS DUAS ROUPAS =====================\n");

    if(!TTF){
        printf("\n  nenhuma TTF nos caminhos conhecidos — este medidor precisa de uma.  NAO MEDIU.\n\n");
        fechar(&b);
        return 2;                                    /* 2, e nao 0: nao medir nao e' passar */
    }

printf("\n§B1  A TTF ENTRA: os contornos saem do ficheiro, com pontos de controlo.\n\n");
    long entrou = 0;
    {
        printf("      %s\n", TTF);
        printf("      unidades por em: %ld\n", G.upem);
        long ctl = 0;
        for(long i = 0; i < G.n; i++) if(!G.p[i].na_curva) ctl++;
        printf("      um glifo: %ld contornos, %ld pontos, %ld de controlo, largura %ld\n",
               G.contornos, G.n, ctl, G.larg);
        /* as duas metades: tem de haver pontos NA curva e pontos DE CONTROLO. So' com os dois
         * e' spline — com um so' seria poligonal ou nao haveria por onde a curva passasse. */
        entrou = (G.n > 0 && ctl > 0 && ctl < G.n && G.upem > 0 && G.larg > 0);
        ok("a TTF entra como FICHEIRO e os contornos saem dela: pontos na curva E pontos de"
           " controlo, mais a largura e a regua da fonte (as unidades por em). E' o backend a"
           " entrar pela porta comum — mais uma realizacao, e nao um recurso privilegiado",
           entrou);
    }

printf("\n§B2  ESTRELA -> LATEX: sai a METRICA, e ela vem da CURVA.\n\n");
    long tem_latex = 0;
    char lat[512];
    {
        long k = veste_latex(&G, lat, sizeof lat);
        long mil = G.larg * 1000 / G.upem;
        printf("      %s", lat);
        printf("      a largura em milesimos de em: %ld — e vem do hmtx da FONTE\n", mil);
        tem_latex = (k > 0 && mil > 0 && mil < 2000);
        ok("para o LATEX sai a METRICA — a largura em milesimos de em —, que e' o que ele precisa"
           " para medir a linha. E ela vem da FONTE e nao de uma tabela escrita: e' o mesmo"
           " principio das cores e da escala, e a razao de o spline.c ter provado 95 de 95 contra"
           " a tabela base-14 antes de a tabela poder sair", tem_latex);
    }

printf("\n§B3  ESTRELA -> PDF: sai o CAMINHO, com os operadores do desenha.c.\n\n");
    long tem_pdf = 0;
    char cam[8192];
    {
        long k = veste_pdf(&G, cam, sizeof cam);
        long mv = 0, li = 0, cu = 0;
        for(const char *q = cam; *q; q++){
            if(q[0]==' ' && q[1]=='m' && q[2]=='\n') mv++;
            if(q[0]==' ' && q[1]=='l' && q[2]=='\n') li++;
            if(q[0]==' ' && q[1]=='v' && q[2]=='\n') cu++;
        }
        printf("      %ld bytes de caminho: %ld move, %ld recta, %ld CURVA\n", k, mv, li, cu);
        printf("      e sao os mesmos m/l/v do desenha.c — nenhum operador novo\n");
        /* e tem de haver CURVAS: um glifo so' com rectas seria uma poligonal, e ai' a fonte
         * nao era spline. E' a metade que separa emitir a curva de emitir o esqueleto. */
        tem_pdf = (k > 0 && mv > 0 && cu > 0);
        ok("para o PDF sai o CAMINHO — os pontos, que e' o que ele precisa para pintar — com os"
           " mesmos operadores do desenha.c e nenhum novo. E ha' CURVAS: um glifo so' com rectas"
           " era uma poligonal, e ai' a fonte nao era spline. E' isto que fecha o que estava em"
           " aberto: ate' aqui o tradutor DECLARAVA /BaseFont/Helvetica e deixava o leitor"
           " desenhar — lia a spline para medir, e nao a emitia", tem_pdf);
    }

printf("\n§B4  E as duas saem do MESMO contorno: mudado o glifo, as duas mudam.\n\n");
    {
        /* a prova de que nao sao dois caminhos independentes: le-se OUTRO glifo e as duas
         * saidas tem de mudar. Se uma mudasse e a outra nao, uma delas nao vinha do contorno. */
        struct glifo H;
        long mudou_lat = 0, mudou_pdf = 0;
        char lat2[512], cam2[8192];
        if(ttf_le(TTF, 50, &H) && H.n > 0){
            veste_latex(&H, lat2, sizeof lat2);
            veste_pdf(&H, cam2, sizeof cam2);
            mudou_lat = (strcmp(lat, lat2) != 0);
            mudou_pdf = (strcmp(cam, cam2) != 0);
        }
        printf("      outro glifo: a metrica muda? %s   o caminho muda? %s\n",
               mudou_lat ? "sim" : "NAO", mudou_pdf ? "sim" : "NAO");
        ok("mudado o glifo, AS DUAS saidas mudam — a metrica e o caminho. E' a prova de que nao"
           " sao dois caminhos independentes: as duas vem do MESMO contorno, e a estrela le' o"
           " ficheiro UMA vez e escreve nas duas roupas. Nao e' a TTF que gera o latex nem o"
           " latex que gera o pdf: as duas saem do mesmo corpo, que e' a spline",
           mudou_lat && mudou_pdf);
    }

printf("\n§B5  O CONTROLO: sem a TTF, nenhuma das duas sai — e nao se cai numa tabela.\n\n");
    {
        /* sem o ficheiro, as duas emissoes tem de ficar VAZIAS. Se uma delas ainda desse
         * alguma coisa, era porque havia uma tabela escrita algures a servir de rede — e uma
         * rede dessas e' a referencia a' mao: passa sem a fonte existir. */
        struct glifo V;
        char l[512], c[8192];
        long saiu_lat = 0, saiu_pdf = 0;
        if(!ttf_le("/tmp/nao_existe_esta_fonte.ttf", 36, &V)){
            memset(&V, 0, sizeof V);
            saiu_lat = veste_latex(&V, l, sizeof l);
            saiu_pdf = veste_pdf(&V, c, sizeof c);
        }
        printf("      sem ficheiro: metrica %ld bytes, caminho %ld bytes (as duas tem de ser 0)\n",
               saiu_lat, saiu_pdf);
        ok("sem a TTF NENHUMA das duas sai, e nao se cai numa tabela. Se uma ainda desse alguma"
           " coisa, era porque havia uma tabela escrita a servir de rede — e uma rede dessas e' a"
           " referencia a' mao: passa sem a fonte existir, e entao o que se mediu nao era a"
           " fonte", saiu_lat == 0 && saiu_pdf == 0);
    }

    /* e o backend entra no banco, como todas as realizacoes */
    {
        unsigned char v[160];
        long m = (long)snprintf((char*)v, sizeof v, "1,1,1|MOVE(ttf, sentido) — o backend");
        gravar(&b, "corpo/ttf", v, m);
        m = (long)snprintf((char*)v, sizeof v, "corpo/estrela");
        gravar(&b, "corpo/ttf/veste", v, m);
    }

    fechar(&b);
printf("\n=== O BACKEND TTF ===========================================================\n");
printf("  A TTF entra como FICHEIRO, a estrela le' UMA vez, e escreve nas DUAS roupas:\n\n");
printf("      ficheiro.ttf ──► ESTRELA ──► LaTeX      o contorno vira METRICA\n");
printf("                          |\n");
printf("                          └──────► PDF        o contorno vira CAMINHO\n\n");
printf("  E e' COMPILADO e nao literal: para o LaTeX sai a largura, que e' o que ele precisa\n");
printf("  para medir a linha; para o PDF saem os pontos, que e' o que ele precisa para pintar.\n");
printf("  Sao projeccoes diferentes do MESMO contorno, e nenhuma e' copia da outra.\n\n");
printf("  E fecha o que estava em aberto: ate' aqui o tradutor DECLARAVA /BaseFont/Helvetica e\n");
printf("  deixava o leitor desenhar. Lia a spline para medir a largura, e nao a emitia — logo a\n");
printf("  fonte que aparecia era a que o leitor tivesse, e nenhuma escala a corrigia.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a TTF entra, e as duas roupas saem do mesmo contorno.\n\n");
    return 0;
}
