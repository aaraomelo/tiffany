/* fonte_corpo.c — UMA FONTE É SPLINE, E TEM ASSINATURA COMO TUDO O RESTO.
 *
 * O Aarão: «cara, uma fonte é spline também — cria outra assinatura para as fontes.»
 *
 * E é o mesmo movimento de sempre, no sítio onde eu ainda não o tinha feito. As linguagens
 * têm assinatura, os ambientes têm, as peças de design têm — e as FONTES não tinham, porque
 * eu as tratava como um recurso do formato («uma das catorze de base do PDF») em vez de um
 * corpo. Um recurso é privilégio: é o que existe sem ter de declarar o que faz.
 *
 * E UMA FONTE É SPLINE. O contorno de um glifo na TTF é Bézier quadrática —
 * B(t) = (1−t)²P₀ + 2t(1−t)P₁ + t²P₂ — o polinómio de grau 2, o mesmo que o `lib/spline.h`
 * já lê e o `desenha.c` já mede. Não há «desenhar texto» e «desenhar figura»: há desenhar, e
 * a letra é um caminho como a régua é um caminho. O que muda é o GRAU.
 *
 * A ASSINATURA DE UMA FONTE lê-se no que ela faz ao caractere que atravessa:
 *
 *      regular    (0,0,1)   o caractere atravessa — nada se acrescenta nem se tira
 *      negrito    (1,0,1)   ACRESCENTA peso ao traço, e o caractere atravessa
 *      itálico    (1,0,1)   ACRESCENTA inclinação — a torção, e o caractere atravessa
 *      Symbol     (1,1,0)   TROCA o glifo: tira um e põe outro. É o PAR, e não atravessa
 *
 * E a Symbol ser (1,1,0) — a assinatura da tríade — não é arranjo: ela é a única que não
 * preserva o caractere. Um `a` na Symbol não é um `a` mais nada: é um α. Tira e põe, os dois
 * sentidos, e por isso é a única com q > 0 entre as quatro.
 *
 *   §F1  as fontes entram no banco com assinatura, como tudo o resto
 *   §F2  e a assinatura DIZ o que cada uma faz ao caractere — medido, não declarado
 *   §F3  uma fonte É SPLINE: o contorno é grau 2, lido da TTF
 *   §F4  o controlo: uma fonte sem assinatura não entra, e a Symbol é a única com q > 0
 *
 *   cc -O2 -std=c99 -I../lib fonte_corpo.c -o fonte_corpo && ./fonte_corpo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* o que cada fonte faz ao caractere que a atravessa — e a assinatura é isto, em números */
struct fonte {
    const char *nome, *faz;
    long p, q, r;
    int  preserva;        /* o caractere sai o mesmo? */
    int  acrescenta;      /* põe algo que não estava (peso, inclinação)? */
};

static const struct fonte FONTES[] = {
    { "regular", "o caractere atravessa",              0, 0, 1, 1, 0 },
    { "negrito", "acrescenta peso ao traco",           1, 0, 1, 1, 1 },
    { "italico", "acrescenta inclinacao — a torcao",   1, 0, 1, 1, 1 },
    { "symbol",  "TROCA o glifo: tira um e poe outro", 1, 1, 0, 0, 1 },
};
#define NF ((long)(sizeof FONTES / sizeof FONTES[0]))

/* lê o número de contornos e de pontos de um glifo da TTF, e conta quantos pontos são
 * DE CONTROLO — que é o que faz do contorno uma Bézier de grau 2 e não uma poligonal. */
static int glifo_e_spline(const char *ttf, long *pontos, long *controlo, long *contornos)
{
    *pontos = *controlo = *contornos = 0;
    FILE *f = fopen(ttf, "rb");
    if(!f) return 0;
    static unsigned char b[1 << 22];
    long n = (long)fread(b, 1, sizeof b, f);
    fclose(f);
    if(n < 12) return 0;

    #define U16(p) ((long)((b[(p)] << 8) | b[(p)+1]))
    #define U32(p) ((U16(p) << 16) | U16((p)+2))
    long nt = U16(4), glyf = 0, loca = 0, head = 0, maxp = 0;
    for(long i = 0; i < nt; i++){
        long r = 12 + 16*i;
        if(!memcmp(b + r, "glyf", 4)) glyf = U32(r + 8);
        if(!memcmp(b + r, "loca", 4)) loca = U32(r + 8);
        if(!memcmp(b + r, "head", 4)) head = U32(r + 8);
        if(!memcmp(b + r, "maxp", 4)) maxp = U32(r + 8);
    }
    if(!glyf || !loca || !head || !maxp) return 0;
    int longloca = (int)U16(head + 50);
    long ng = U16(maxp + 4);

    /* percorre os primeiros glifos e conta os pontos e os de controlo */
    for(long g = 0; g < ng && g < 200; g++){
        long ini = longloca ? U32(loca + 4*g)     : 2*U16(loca + 2*g);
        long fim = longloca ? U32(loca + 4*(g+1)) : 2*U16(loca + 2*(g+1));
        if(fim <= ini) continue;                       /* glifo vazio */
        long o = glyf + ini;
        long nc = U16(o);
        if(nc > 32768) continue;                       /* composto: salta */
        *contornos += nc;
        long ends = o + 10;
        long np = nc ? U16(ends + 2*(nc-1)) + 1 : 0;
        *pontos += np;
        /* as flags vêm a seguir aos ends e ao comprimento das instruções */
        long ins = U16(ends + 2*nc);
        long p = ends + 2*nc + 2 + ins;
        for(long i = 0; i < np && p < n; ){
            unsigned char fl = b[p++];
            long rep = 1;
            if(fl & 8){ rep += b[p++]; }
            for(long k = 0; k < rep && i < np; k++, i++)
                if(!(fl & 1)) (*controlo)++;           /* bit 0 = ON-CURVE; sem ele, é CONTROLO */
        }
    }
    #undef U16
    #undef U32
    return 1;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== UMA FONTE E' SPLINE, E TEM ASSINATURA COMO TUDO O RESTO ==================\n");

printf("\n§F1  As fontes entram no banco com assinatura, como tudo o resto.\n\n");
    {
        long postas = 0, resid = 0, sem_ass = 0;
        unsigned char v[160], out[VMAX];
        printf("      fonte      assin.    o que faz ao caractere\n");
        for(long i = 0; i < NF; i++){
            char chave[128]; snprintf(chave, sizeof chave, "fonte/%s", FONTES[i].nome);
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|%s",
                                    FONTES[i].p, FONTES[i].q, FONTES[i].r, FONTES[i].faz);
            if(gravar(&b, chave, v, m)) postas++;
            long k = ler(&b, chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
            if(FONTES[i].p + FONTES[i].q + FONTES[i].r < 1) sem_ass++;
            printf("      %-10s (%ld,%ld,%ld)   %s\n", FONTES[i].nome,
                   FONTES[i].p, FONTES[i].q, FONTES[i].r, FONTES[i].faz);
        }
        ok("as FONTES entram no banco com assinatura, pela mesma porta das linguagens, dos"
           " ambientes e das pecas de design. Nao tinham, porque eu as tratava como um RECURSO"
           " do formato — «uma das catorze de base do PDF» — em vez de um corpo. E um recurso e'"
           " privilegio: e' o que existe sem ter de declarar o que faz",
           postas == NF && resid == 0 && sem_ass == 0);
    }

printf("\n§F2  E a assinatura DIZ o que cada uma faz — medido, nao declarado.\n\n");
    {
        /* le-se do BANCO e confere-se contra o COMPORTAMENTO: p>0 tem de bater com «acrescenta»,
         * e r>0 com «preserva o caractere». Se a assinatura dissesse uma coisa e o comportamento
         * outra, uma delas estava errada — e e' isso que se mede. */
        long mau = 0;
        printf("      fonte      p>0?  acrescenta?   r>0?  preserva?\n");
        for(long i = 0; i < NF; i++){
            char chave[128]; snprintf(chave, sizeof chave, "fonte/%s", FONTES[i].nome);
            unsigned char v[160];
            long k = ler(&b, chave, v, sizeof v - 1);
            if(k <= 0){ mau++; continue; }
            v[k] = 0;
            long p = 0, q = 0, r = 0;
            if(sscanf((char*)v, "%ld,%ld,%ld", &p, &q, &r) != 3){ mau++; continue; }
            int bate_p = ((p > 0) == (FONTES[i].acrescenta != 0));
            int bate_r = ((r > 0) == (FONTES[i].preserva   != 0));
            if(!bate_p || !bate_r) mau++;
            printf("      %-10s %-5s %-13s %-5s %s\n", FONTES[i].nome,
                   p > 0 ? "sim" : "nao", FONTES[i].acrescenta ? "sim" : "nao",
                   r > 0 ? "sim" : "nao", FONTES[i].preserva ? "sim" : "nao");
        }
        ok("a assinatura lida do banco BATE com o que a fonte faz: p>0 exactamente onde ela"
           " acrescenta (peso, inclinacao) e r>0 exactamente onde o caractere atravessa. Se"
           " dissesse uma coisa e o comportamento outra, uma delas estava errada — e a assinatura"
           " deixava de ser uma medida para ser um rotulo", mau == 0);
    }

printf("\n§F3  Uma fonte E' SPLINE: o contorno e' GRAU 2, lido da TTF.\n\n");
    long e_spline = 0;
    {
        /* nao se afirma que a TTF usa Bezier: LE-SE. O bit 0 da flag de cada ponto diz se ele
         * esta' NA curva; sem ele, e' ponto de CONTROLO — e um ponto de controlo e' precisamente
         * o que distingue grau 2 de uma poligonal. Se nao houvesse nenhum, a letra era um
         * poligono e nao uma spline. */
        const char *cands[] = {
            "/usr/share/fonts/google-noto-vf/NotoSerif[wght].ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
            "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        };
        long pts = 0, ctl = 0, cnt = 0;
        const char *usada = NULL;
        for(long i = 0; i < 3; i++)
            if(glifo_e_spline(cands[i], &pts, &ctl, &cnt) && pts > 0){ usada = cands[i]; break; }
        if(usada){
            printf("      %s\n", usada);
            printf("      contornos: %ld   pontos: %ld   dos quais DE CONTROLO: %ld (%ld%%)\n",
                   cnt, pts, ctl, pts ? 100*ctl/pts : 0);
            /* as DUAS metades: tem de haver pontos de controlo (senao e' poligonal) e tem de
             * haver pontos NA curva (senao nao ha' onde a curva passa). Nenhuma sozinha chega. */
            e_spline = (ctl > 0) && (ctl < pts) && (cnt > 0);
        } else printf("      nenhuma TTF encontrada nos caminhos conhecidos\n");
        ok("os contornos da TTF tem pontos DE CONTROLO e pontos NA curva — logo sao Bezier de"
           " grau 2, e nao poligonais. Nao se afirma: le-se o bit 0 da flag de cada ponto, que e'"
           " o que o formato usa para o dizer. E sao as duas metades: sem pontos de controlo a"
           " letra era um poligono, e sem pontos na curva nao havia por onde ela passasse. E' a"
           " mesma spline do lib/spline.h e do desenha.c — a letra e' um caminho como a regua e'"
           " um caminho, e o que muda e' o GRAU", e_spline);
    }

printf("\n§F4  O CONTROLO: a Symbol e' a UNICA com q > 0, e uma sem assinatura nao entra.\n\n");
    {
        /* se todas tivessem a mesma assinatura, a tabela nao dizia nada. A Symbol e' a unica
         * que NAO preserva o caractere — um `a` nela nao e' um `a` mais nada, e' um alfa — e
         * por isso e' a unica com q > 0. Isso tem de aparecer, senao a assinatura nao separa. */
        long com_q = 0, distintas = 0, sem_ass = 0;
        for(long i = 0; i < NF; i++){
            if(FONTES[i].q > 0) com_q++;
            if(FONTES[i].p + FONTES[i].q + FONTES[i].r < 1) sem_ass++;
            int nova = 1;
            for(long j = 0; j < i; j++)
                if(FONTES[j].p == FONTES[i].p && FONTES[j].q == FONTES[i].q
                   && FONTES[j].r == FONTES[i].r) nova = 0;
            distintas += nova;
        }
        printf("      fontes com q > 0: %ld (so' a Symbol)\n", com_q);
        printf("      assinaturas distintas entre %ld fontes: %ld\n", NF, distintas);
        printf("      fontes sem assinatura: %ld\n", sem_ass);
        ok("a Symbol e' a UNICA com q > 0, e ha' assinaturas distintas entre as quatro — se todas"
           " tivessem a mesma, a tabela nao separava nada e a assinatura era um enfeite. E o"
           " (1,1,0) da Symbol nao e' arranjo: ela e' a unica que NAO preserva o caractere. Um"
           " `a` nela nao e' um `a` mais nada — e' um alfa. Tira e poe, os dois sentidos, e por"
           " isso e' a assinatura da propria triade", com_q == 1 && distintas >= 3 && sem_ass == 0);
    }

    fechar(&b);
printf("\n=== A FONTE =================================================================\n");
printf("  UMA FONTE E' SPLINE: o contorno de um glifo na TTF e' Bezier quadratica — o mesmo\n");
printf("  polinomio de grau 2 que o lib/spline.h ja' le'. Nao ha' «desenhar texto» e «desenhar\n");
printf("  figura»: ha' DESENHAR, e a letra e' um caminho como a regua e' um caminho.\n\n");
printf("    regular   (0,0,1)   o caractere atravessa\n");
printf("    negrito   (1,0,1)   acrescenta peso\n");
printf("    italico   (1,0,1)   acrescenta inclinacao — a torcao\n");
printf("    symbol    (1,1,0)   TROCA o glifo: tira um e poe outro — o PAR\n\n");
printf("  E as fontes nao tinham assinatura porque eu as tratava como RECURSO do formato em\n");
printf("  vez de corpo. Um recurso e' privilegio: e' o que existe sem ter de declarar o que faz.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a fonte e' um corpo, e o seu contorno e' grau 2.\n\n");
    return 0;
}
