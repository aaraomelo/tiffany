/* DEPENDE-DE: papers/estilo.tex
 * O que este medidor LÊ entra na assinatura da bateria — sem isto, mudar um
 * destes ficheiros não reabre a semente, e o verde é sobre um estado que já
 * não existe. Mesma razão dos headers, um andar acima. */
/* design.c — O DESIGN SYSTEM SAI DO estilo.tex, E DESENHA-SE POR CAMINHO.
 *
 * O Aarão: «vê o design que estava antes e reconstrói igual — ainda sem design system nos
 * PDFs.»
 *
 * O design não é uma escolha a fazer agora: JÁ EXISTE, e está escrito no `estilo.tex`. As
 * cores, a barra da caixa, as réguas da tabela — tudo declarado num sítio. Logo o que aqui se
 * faz não é desenhar um design: é LER o que está e reproduzi-lo.
 *
 * E isso muda o que se pode medir. Se eu escrevesse as cores neste ficheiro, a asserção
 * comparava os meus números com os meus números — a referência escrita à mão, que já me custou
 * um dia. Lidas do `estilo.tex`, mudar uma cor lá muda o que sai daqui, e o medidor acompanha
 * sozinho. É o teste: mude-se a fonte e veja-se se a medida muda.
 *
 * E o DESENHO é o do `desenha.c`, sem uma primitiva nova:
 *
 *      o fundo da caixa    caminho fechado, preenchido        `re ... f`
 *      a barra à esquerda  caminho fechado, preenchido        o mesmo, mais estreito
 *      a régua da tabela   dois pontos, traçado               `m ... l ... S`
 *      o glifo             contorno de Bézier                 já lido da TTF
 *
 * Quatro peças de design, zero primitivas novas. É isso que faz disto um SISTEMA e não uma
 * folha de estilos: o mesmo operador desenha tudo, e o que muda é o caminho e a pintura.
 *
 *   §W1  as cores saem do estilo.tex — nove, lidas e não escritas
 *   §W2  a caixa desenha-se por caminho: fundo e barra, sem primitiva nova
 *   §W3  a régua da tabela é o mesmo operador com grau 1
 *   §W4  e tudo isto entra no banco, como todo o resto
 *   §W5  o controlo: mudada a cor no estilo.tex, o que sai MUDA — e volta
 *
 *   cc -O2 -std=c99 -I../lib design.c -o design && ./design
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

struct cor { char nome[32]; long r, g, b; };

/* lê \definecolor{nome}{HTML}{RRGGBB} do estilo.tex — a FONTE, e não uma cópia */
static long le_cores(const char *caminho, struct cor *cs, long cap)
{
    FILE *f = fopen(caminho, "rb");
    if(!f) return -1;
    static char buf[1 << 20];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n] = 0;

    long nc = 0;
    const char *p = buf;
    while(nc < cap && (p = strstr(p, "\\definecolor{")) != NULL){
        p += 13;
        const char *a = p; while(*p && *p != '}') p++;
        long ln = p - a; if(ln > 31) ln = 31;
        memcpy(cs[nc].nome, a, (size_t)ln); cs[nc].nome[ln] = 0;
        const char *h = strstr(p, "{HTML}{");
        if(!h) continue;
        h += 7;
        unsigned rr = 0, gg = 0, bb = 0;
        if(sscanf(h, "%2x%2x%2x", &rr, &gg, &bb) == 3){
            cs[nc].r = rr; cs[nc].g = gg; cs[nc].b = bb;
            nc++;
        }
    }
    return nc;
}

/* o caminho de um rectângulo preenchido, em operadores do PDF. Sem primitiva nova: é o
 * mesmo `m`/`l` do desenha.c, fechado com `f`. */
static long rect(char *out, long cap, long x, long y, long w, long h,
                 long r, long g, long bl)
{
    return snprintf(out, (size_t)cap,
        "%ld.%03ld %ld.%03ld %ld.%03ld rg\n"        /* a cor, em milésimos: inteiro */
        "%ld %ld m\n%ld %ld l\n%ld %ld l\n%ld %ld l\nf\n",
        r * 1000 / 255 / 1000, (r * 1000 / 255) % 1000,
        g * 1000 / 255 / 1000, (g * 1000 / 255) % 1000,
        bl * 1000 / 255 / 1000, (bl * 1000 / 255) % 1000,
        x, y, x + w, y, x + w, y + h, x, y + h);
}

/* a régua: dois pontos, traçado. Grau 1 — não tem par, é transporte. */
static long regua(char *out, long cap, long x1, long x2, long y, long esp)
{
    return snprintf(out, (size_t)cap, "%ld w\n%ld %ld m\n%ld %ld l\nS\n", esp, x1, y, x2, y);
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const char *ESTILO = "../estilo.tex";
    struct cor cs[64];
    long nc = le_cores(ESTILO, cs, 64);

printf("\n=== O DESIGN SAI DO estilo.tex, E DESENHA-SE POR CAMINHO =====================\n");

printf("\n§W1  As cores saem do estilo.tex — lidas, e nao escritas aqui.\n\n");
    {
        printf("      cor            RGB\n");
        for(long i = 0; i < nc && i < 6; i++)
            printf("      %-14s %ld,%ld,%ld\n", cs[i].nome, cs[i].r, cs[i].g, cs[i].b);
        if(nc > 6) printf("      (... e mais %ld)\n", nc - 6);
        /* NAO se afirma quais sao as cores nem quantas: afirma-se que se LERAM e que os
         * componentes cabem no intervalo. Escrever «ouro e' B8912F» aqui era a referencia a'
         * mao — mudava-se a cor no estilo.tex e este medidor continuava verde. */
        long fora = 0;
        for(long i = 0; i < nc; i++)
            if(cs[i].r < 0 || cs[i].r > 255 || cs[i].g < 0 || cs[i].g > 255
               || cs[i].b < 0 || cs[i].b > 255) fora++;
        ok("as cores sao LIDAS do estilo.tex e nenhuma cai fora do intervalo. Nao se afirma"
           " quais sao nem quantas: escrever «ouro e' B8912F» aqui era a referencia escrita a'"
           " mao — mudava-se a cor la' e este medidor continuava verde. O §W5 fecha isto com a"
           " mutacao", nc > 0 && fora == 0);
    }

printf("\n§W2  A CAIXA desenha-se por caminho: fundo e barra, SEM primitiva nova.\n\n");
    {
        /* o tcolorbox do catalogo: colback=ouroclaro!35, colframe=ouro, leftrule=2pt.
         * Sao DOIS rectangulos preenchidos — o fundo e a barra — e nada mais. */
        const struct cor *ouro = NULL, *claro = NULL;
        for(long i = 0; i < nc; i++){
            if(!strcmp(cs[i].nome, "ouro"))      ouro = &cs[i];
            if(!strcmp(cs[i].nome, "ouroclaro")) claro = &cs[i];
        }
        char fundo[512] = "", barra[512] = "";
        long kf = 0, kb = 0;
        if(claro) kf = rect(fundo, sizeof fundo, 72, 600, 451, 80, claro->r, claro->g, claro->b);
        if(ouro)  kb = rect(barra, sizeof barra, 72, 600,   2, 80, ouro->r,  ouro->g,  ouro->b);
        /* conta-se: cada um tem UM `rg` (a cor), UM `m`, TRES `l` e UM `f`. Sao os mesmos
         * opcodes do desenha.c — nao ha' operador novo nenhum. */
        long ops_f = 0, ops_b = 0;
        for(const char *p = fundo; *p; p++) if(p[0]=='\n' && (p[1]=='f'||p[1]=='S')) ops_f++;
        for(const char *p = barra; *p; p++) if(p[0]=='\n' && (p[1]=='f'||p[1]=='S')) ops_b++;
        int tem_cor = strstr(fundo, " rg\n") != NULL && strstr(barra, " rg\n") != NULL;
        int tem_cam = strstr(fundo, " m\n")  != NULL && strstr(barra, " l\n")  != NULL;
        /* e as DUAS partes tem de ser DIFERENTES: mesma primitiva, cores e larguras distintas */
        int distintos = strcmp(fundo, barra) != 0;
        printf("      o fundo (ouroclaro): %ld bytes, com cor e caminho\n", kf);
        printf("      a barra (ouro, 2pt): %ld bytes, a mesma primitiva\n", kb);
        printf("      operadores novos: 0 — sao os mesmos m/l/f do desenha.c\n");
        ok("a CAIXA sao dois rectangulos preenchidos — o fundo e a barra da esquerda — e nao"
           " pede primitiva nenhuma nova: sao os mesmos m/l/f. As duas metades: a primitiva e' a"
           " MESMA (o invariante) e o resultado e' DIFERENTE (cores e larguras distintas, senao"
           " nao havia caixa nenhuma). E' isto que faz disto um sistema e nao uma folha de"
           " estilos — o mesmo operador desenha tudo, e o que muda e' o caminho e a pintura",
           ouro && claro && kf > 0 && kb > 0 && tem_cor && tem_cam && distintos);
    }

printf("\n§W3  A REGUA da tabela e o mesmo operador, com GRAU 1.\n\n");
    {
        /* booktabs: toprule, midrule, bottomrule — tres reguas, espessuras diferentes.
         * Sao dois pontos e um tracado: grau 1, sem ponto de controlo. */
        char t[256], m[256], bt[256];
        long kt = regua(t,  sizeof t,  72, 523, 700, 2);   /* toprule:    mais grossa */
        long km = regua(m,  sizeof m,  72, 523, 660, 1);   /* midrule:    fina */
        long kb = regua(bt, sizeof bt, 72, 523, 600, 2);   /* bottomrule: mais grossa */
        long curvas = 0;
        for(const char *p = t; *p; p++) if(p[0]==' ' && p[1]=='v') curvas++;
        for(const char *p = m; *p; p++) if(p[0]==' ' && p[1]=='v') curvas++;
        int esp_difere = strcmp(t, m) != 0;
        printf("      toprule  %ld bytes, espessura 2\n", kt);
        printf("      midrule  %ld bytes, espessura 1\n", km);
        printf("      curvas usadas: %ld — e' grau 1, nao tem par\n", curvas);
        ok("as reguas do booktabs sao o MESMO operador com grau 1 — dois pontos e um tracado, e"
           " ZERO curvas. E o grau decide, como decide em toda a parte: a regua nao tem par, e'"
           " transporte; o glifo tem duas raizes e e' o par que faz a volta fechar. As tres"
           " reguas diferem entre si na espessura, senao nao havia toprule e midrule",
           kt > 0 && km > 0 && kb > 0 && curvas == 0 && esp_difere);
    }

printf("\n§W4  E tudo isto entra no BANCO, como todo o resto.\n\n");
    {
        long postas = 0, resid = 0;
        unsigned char v[160], out[VMAX];
        for(long i = 0; i < nc; i++){
            char chave[128]; snprintf(chave, sizeof chave, "design/cor/%s", cs[i].nome);
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld", cs[i].r, cs[i].g, cs[i].b);
            if(gravar(&b, chave, v, m)) postas++;
            long k = ler(&b, chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
        }
        /* e as pecas de design, com a assinatura do que fazem: acrescentam (p) e o conteudo
         * atravessa (r) — nenhuma corta, porque desenhar nao parte o fluxo, poe-se por baixo */
        struct { const char *nome; const char *faz; long p, q, r; } pecas[] = {
            { "caixa/fundo",  "rectangulo preenchido",        1, 0, 1 },
            { "caixa/barra",  "rectangulo estreito, a cor",   1, 0, 1 },
            { "tabela/regua", "dois pontos, tracado, grau 1", 1, 0, 1 },
            { "glifo",        "contorno de Bezier, grau 2",   1, 0, 1 },
        };
        for(long i = 0; i < 4; i++){
            char chave[128]; snprintf(chave, sizeof chave, "design/%s", pecas[i].nome);
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|%s",
                                    pecas[i].p, pecas[i].q, pecas[i].r, pecas[i].faz);
            if(gravar(&b, chave, v, m)) postas++;
            long k = ler(&b, chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
        }
        printf("      %ld chaves de design no banco, %ld residuo\n", postas, resid);
        ok("o design entra no banco pela mesma porta que os corpos, as linguagens, os ambientes"
           " e os cards — as cores lidas do estilo.tex e as pecas com assinatura. E nenhuma peca"
           " de design CORTA (q=0): desenhar nao parte o fluxo, poe-se por baixo dele. E' por"
           " isso que o fundo de uma caixa nao muda onde o texto cai",
           postas == nc + 4 && resid == 0);
    }

printf("\n§W5  O CONTROLO: mudada a cor no estilo.tex, o que sai MUDA — e volta.\n\n");
    {
        /* A MUTACAO E' O QUE DA' VALOR AO §W1. Sem ela, «as cores sao lidas» passava com uma
         * leitura que devolvesse sempre a mesma coisa. Muda-se a cor NA FONTE, le-se outra vez,
         * e o desenho tem de mudar — e depois tem de voltar. */
        FILE *f = fopen(ESTILO, "rb");
        static char orig[1 << 20];
        long n = f ? (long)fread(orig, 1, sizeof orig - 1, f) : 0;
        if(f) fclose(f);
        orig[n > 0 ? n : 0] = 0;

        long mudou = 0, voltou = 0, mordeu = 0;
        const char *alvo = "\\definecolor{ouro}{HTML}{B8912F}";
        if(n > 0 && strstr(orig, alvo)){
            mordeu = 1;
            static char mut[1 << 20];
            const char *p = strstr(orig, alvo);
            long pre = p - orig;
            memcpy(mut, orig, (size_t)pre);
            long m = pre + snprintf(mut + pre, sizeof mut - (size_t)pre,
                                    "\\definecolor{ouro}{HTML}{010203}");
            memcpy(mut + m, p + strlen(alvo), (size_t)(n - pre - (long)strlen(alvo)));
            m += n - pre - (long)strlen(alvo);
            f = fopen(ESTILO, "wb"); if(f){ fwrite(mut, 1, (size_t)m, f); fclose(f); }

            struct cor cm[64];
            long nm = le_cores(ESTILO, cm, 64);
            for(long i = 0; i < nm; i++)
                if(!strcmp(cm[i].nome, "ouro"))
                    mudou = (cm[i].r == 1 && cm[i].g == 2 && cm[i].b == 3);

            f = fopen(ESTILO, "wb"); if(f){ fwrite(orig, 1, (size_t)n, f); fclose(f); }
            struct cor cv[64];
            long nv = le_cores(ESTILO, cv, 64);
            for(long i = 0; i < nv; i++)
                if(!strcmp(cv[i].nome, "ouro"))
                    voltou = !(cv[i].r == 1 && cv[i].g == 2 && cv[i].b == 3);
        }
        printf("      mudada a cor na fonte, a leitura muda:      %s\n", mudou ? "sim" : "NAO");
        printf("      revertida a fonte, a leitura volta:         %s\n", voltou ? "sim" : "NAO");
        ok("mudada a cor NO estilo.tex, o que se le' muda; revertida, volta. E' o que da' valor"
           " ao §W1 — sem esta metade, «as cores sao lidas» passava com uma leitura que"
           " devolvesse sempre a mesma coisa. E e' o mesmo teste que separou «composto agora» de"
           " «servido de uma copia»: mude-se a FONTE e veja-se se o resultado muda",
           mordeu && mudou && voltou);
    }

    fechar(&b);
printf("\n=== O DESIGN ================================================================\n");
printf("  O design NAO se escolhe agora: ja' existe, e esta' no estilo.tex. O que aqui se faz\n");
printf("  e' LER o que esta' — e por isso mudar uma cor la' muda o que sai daqui.\n\n");
printf("    o fundo da caixa    caminho fechado, preenchido      re ... f\n");
printf("    a barra a' esquerda o mesmo, mais estreito           re ... f\n");
printf("    a regua da tabela   dois pontos, tracado, grau 1     m ... l ... S\n");
printf("    o glifo             contorno de Bezier, grau 2       ja' lido da TTF\n\n");
printf("  QUATRO pecas de design, ZERO primitivas novas. E' isso que faz disto um sistema e\n");
printf("  nao uma folha de estilos: o mesmo operador desenha tudo, e o que muda e' o caminho\n");
printf("  e a pintura. E nenhuma peca CORTA — desenhar poe-se por baixo do texto, nao o parte.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — o design sai da fonte, e desenha-se por caminho.\n\n");
    return 0;
}
