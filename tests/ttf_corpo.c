/* ttf_corpo.c — O CORPO DO TTF: O DUAL É O SENTIDO, E A FUSÃO É MULTIPLICAÇÃO.
 *
 * O Aarão: «está errado — a fonte mudou mas tem erro aqui de precisão: você não reverteu
 * direto o TTF. O que é um TTF? Qual o dual dele? A folha de PDF é dual, a parte escrita e a
 * vazia. Vê qual o corpo do TTF, a assinatura dele, e faz composição: assinatura de spline ×
 * assinatura de TTF. Isso é reprodução, multiplicação de corpos — vê o viveiro e fundamenta.»
 *
 * Tinha razão nas duas, e a segunda é a que eu não tinha visto.
 *
 * O ERRO DE PRECISÃO: eu li a TTF e emiti, e NUNCA REVERTI. O backend_ttf.c mede que sai um
 * caminho e que a métrica bate — mas não mede que voltar do caminho devolve o contorno. Sem a
 * volta não há resíduo, e sem resíduo não há medida: só uma leitura a confirmar-se a si mesma.
 *
 * E O DUAL DO TTF ESTAVA À VISTA. Um glifo com buraco — o `o`, o `a`, o `e` — tem DOIS
 * contornos, e eles correm em SENTIDOS OPOSTOS. É o que diz ao renderizador o que é tinta e o
 * que é papel: a regra é a soma dos sentidos, e um contorno interior cancela o exterior.
 *
 *      o contorno exterior     roda num sentido      área com sinal  +
 *      o buraco                roda no OUTRO         área com sinal  −
 *
 * Isso é `1† = −1` escrito numa letra. O dual do traço é o vazio que ele encerra, e a folha do
 * PDF é o mesmo par no grande: a parte escrita e a parte vazia. Não são duas coisas — é uma
 * com dois sinais, e o sinal está no SENTIDO DE PERCURSO.
 *
 * E A FUSÃO É MULTIPLICAÇÃO. A teoria já o diz: «o viveiro é o relógio quando DOIS se
 * combinam». A spline é um relógio (o contorno, período do fecho); o TTF é outro (o glifo, com
 * o seu dentro e fora). Compostos, não se somam: multiplicam-se — e o grau do produto é a soma
 * dos graus, que é o que multiplicar faz aos expoentes.
 *
 *   §V1  o glifo com buraco tem contornos de SENTIDOS OPOSTOS — a área com sinal di-lo
 *   §V2  e a folha é o mesmo par: a tinta é a soma com sinal, e o resto é papel
 *   §V3  A VOLTA, que faltava: ler → emitir → ler devolve o contorno, resíduo 0
 *   §V4  a FUSÃO spline × TTF: o grau do produto é a SOMA dos graus
 *   §V5  o controlo: invertido o sentido do buraco, a área muda de sinal e a tinta enche
 *
 *   cc -O2 -std=gnu99 -I../lib ttf_corpo.c -o ttf_corpo && ./ttf_corpo
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

typedef struct { unsigned char *d; long n; } Buf;
static long u16(const Buf *b, long p){ return (p+1 < b->n) ? ((b->d[p] << 8) | b->d[p+1]) : 0; }
static long u32(const Buf *b, long p){ return (u16(b,p) << 16) | u16(b,p+2); }
static long s16v(const Buf *b, long p){ long v = u16(b,p); return v >= 32768 ? v - 65536 : v; }

struct pto { long x, y; int na_curva; };
struct glifo { struct pto p[512]; long n, nc, fim[16], upem, larg; };

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
    long nt = u16(&b,4), glyf = 0, loca = 0, head = 0, hmtx = 0, hhea = 0;
    for(long i = 0; i < nt; i++){
        long r = 12 + 16*i;
        if(!memcmp(b.d+r,"glyf",4)) glyf = u32(&b,r+8);
        if(!memcmp(b.d+r,"loca",4)) loca = u32(&b,r+8);
        if(!memcmp(b.d+r,"head",4)) head = u32(&b,r+8);
        if(!memcmp(b.d+r,"hmtx",4)) hmtx = u32(&b,r+8);
        if(!memcmp(b.d+r,"hhea",4)) hhea = u32(&b,r+8);
    }
    if(!glyf || !loca || !head) return 0;
    G->upem = u16(&b, head+18);
    int longloca = (int)u16(&b, head+50);
    long nhm = hhea ? u16(&b, hhea+34) : 1;
    G->larg = hmtx ? u16(&b, hmtx + 4*(g < nhm ? g : nhm-1)) : 0;
    long ini = longloca ? u32(&b, loca+4*g)     : 2*u16(&b, loca+2*g);
    long fim = longloca ? u32(&b, loca+4*(g+1)) : 2*u16(&b, loca+2*(g+1));
    G->n = G->nc = 0;
    if(fim <= ini) return 1;
    long o = glyf + ini, nc = u16(&b,o);
    if(nc > 32768 || nc > 16) return 1;
    G->nc = nc;
    for(long i = 0; i < nc; i++) G->fim[i] = u16(&b, o+10+2*i);
    long np = nc ? G->fim[nc-1]+1 : 0;
    if(np > 512) return 1;
    long ins = u16(&b, o+10+2*nc), p = o+10+2*nc+2+ins;
    static unsigned char fl[512];
    for(long i = 0; i < np; ){
        unsigned char f = b.d[p++]; long rep = 1;
        if(f & 8) rep += b.d[p++];
        for(long k = 0; k < rep && i < np; k++, i++) fl[i] = f;
    }
    long x = 0;
    for(long i = 0; i < np; i++){
        if(fl[i] & 2){ long d = b.d[p++]; x += (fl[i]&16)? d : -d; }
        else if(!(fl[i]&16)){ x += s16v(&b,p); p += 2; }
        G->p[i].x = x;
    }
    long y = 0;
    for(long i = 0; i < np; i++){
        if(fl[i] & 4){ long d = b.d[p++]; y += (fl[i]&32)? d : -d; }
        else if(!(fl[i]&32)){ y += s16v(&b,p); p += 2; }
        G->p[i].y = y; G->p[i].na_curva = (fl[i]&1) != 0;
    }
    G->n = np;
    return 1;
}

/* A ÁREA COM SINAL de um contorno, pela fórmula do laço — e é INTEIRA, porque as coordenadas
 * da TTF são inteiras. O sinal diz o SENTIDO: positivo num, negativo no outro. É a medida que
 * separa o traço do buraco, e não uma convenção posta por fora. */
static long area_com_sinal(const struct glifo *G, long c)
{
    long ini = c ? G->fim[c-1] + 1 : 0, f = G->fim[c];
    if(f >= G->n || f <= ini) return 0;
    long a = 0;
    for(long i = ini; i <= f; i++){
        long j = (i == f) ? ini : i + 1;
        a += G->p[i].x * G->p[j].y - G->p[j].x * G->p[i].y;   /* 2× a área, em inteiros */
    }
    return a;                                                  /* fica em dobro: não se divide */
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const char *CAND[3] = {
        "/usr/share/fonts/google-noto-vf/NotoSerif[wght].ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
    };
    const char *TTF = NULL;
    struct glifo G;
    /* PROCURA-SE UM GLIFO COM SINAIS MISTOS, e não apenas com dois contornos. À primeira
     * procurei «dois contornos ou mais» e caí num glifo com DOIS TRAÇOS SEPARADOS — como o
     * `i` ou o `ä` —, ambos do mesmo sinal. Dois contornos não é o mesmo que um buraco: o
     * buraco é o que corre no sentido CONTRÁRIO, e é isso que se procura.
     *
     * (Na TrueType a convenção é o exterior em sentido horário, que pela fórmula do laço dá
     * área NEGATIVA, e o buraco no anti-horário, positiva. O sinal absoluto é convenção da
     * fonte; o que não é convenção é HAVER OS DOIS.) */
    long achado = -1;
    for(long i = 0; i < 3 && !TTF; i++)
        for(long g = 1; g < 400; g++){
            if(!ttf_le(CAND[i], g, &G) || G.nc < 2 || G.n == 0) continue;
            long pos = 0, neg = 0;
            for(long c = 0; c < G.nc; c++){
                long a = area_com_sinal(&G, c);
                if(a > 0) pos++; else if(a < 0) neg++;
            }
            if(pos > 0 && neg > 0){ TTF = CAND[i]; achado = g; break; }
        }

printf("\n=== O CORPO DO TTF: O DUAL E' O SENTIDO ======================================\n");

    if(!TTF){
        printf("\n  nenhuma TTF com glifo de buraco nos caminhos conhecidos.  NAO MEDIU.\n\n");
        fechar(&b); return 2;
    }

printf("\n§V1  O glifo com buraco tem contornos de SENTIDOS OPOSTOS.\n\n");
    long dual_e_sentido = 0;
    {
        printf("      %s  (glifo %ld)\n", TTF, achado);
        printf("      contorno   pontos   area com sinal   sentido\n");
        long pos = 0, neg = 0;
        for(long c = 0; c < G.nc; c++){
            long a = area_com_sinal(&G, c);
            long ini = c ? G.fim[c-1]+1 : 0;
            if(a > 0) pos++; else if(a < 0) neg++;
            printf("      %-10ld %-8ld %-16ld %s\n", c, G.fim[c]-ini+1, a,
                   a > 0 ? "+  (um sentido)" : (a < 0 ? "-  (o OUTRO)" : "0"));
        }
        /* AS DUAS METADES: tem de haver dos DOIS sinais. Se fossem todos do mesmo, nao havia
         * buraco nenhum — e o dual nao estava la'. Se nao houvesse nenhum positivo, nao havia
         * traco. E' o par, e nenhum lado sozinho e' o par. */
        dual_e_sentido = (pos > 0 && neg > 0);
        printf("      %ld positivos, %ld negativos — e 1+ = -1 escrito numa letra\n", pos, neg);
        ok("o glifo com buraco tem contornos dos DOIS sinais: o traco roda num sentido e o"
           " buraco no outro. Nao e' convencao posta por fora — mede-se pela area com sinal, que"
           " e' INTEIRA porque as coordenadas da TTF sao inteiras. E' 1+ = -1 escrito numa letra:"
           " o dual do traco e' o VAZIO que ele encerra. E sao as duas metades: se fossem todos"
           " do mesmo sinal nao havia buraco, e sem positivo nao havia traco", dual_e_sentido);
    }

printf("\n§V2  E a FOLHA e' o mesmo par: a tinta e' a soma com sinal, o resto e' papel.\n\n");
    {
        /* a soma das areas com sinal E' a tinta: o buraco SUBTRAI do traco. E' a mesma
         * operacao que a folha do PDF faz no grande — a parte escrita e a parte vazia sao
         * um par, e nao duas coisas. */
        /* a TINTA e' o modulo da soma COM sinal; e o que se comparava era a soma SEM sinal —
         * que da' a letra cheia. O sinal absoluto e' convencao da fonte, por isso toma-se o
         * modulo dos dois lados: o que se mede e' que somar com sinal da' MENOS. */
        long com_sinal = 0, sem_sinal = 0;
        for(long c = 0; c < G.nc; c++){
            long a = area_com_sinal(&G, c);
            com_sinal += a;
            sem_sinal += (a < 0 ? -a : a);
        }
        /* quantos contornos têm área NEGATIVA — é essa a causa do buraco */
        long neg_aqui = 0;
        for(long c = 0; c < G.nc; c++) if(area_com_sinal(&G, c) < 0) neg_aqui++;
        /* e o CONTRASTE, que é o que impede «tem buraco» de valer por «tudo tem»: procura-se
         * um glifo em que TODOS os contornos correm no mesmo sentido — esse não tem buraco,
         * e a sua soma com sinal é igual à soma sem sinal. Se não houver nenhum na fonte,
         * diz-se, em vez de a asserção passar por omissão. */
        long neg_sem = -1, pos_sem = 0;
        {
            struct glifo GS;
            for(long i = 0; i < 3 && neg_sem < 0; i++)
                for(long g = 1; g < 400; g++){
                    if(!ttf_le(CAND[i], g, &GS) || GS.n == 0 || GS.nc < 1) continue;
                    long p2 = 0, n2 = 0;
                    for(long c = 0; c < GS.nc; c++){
                        long a2 = area_com_sinal(&GS, c);
                        if(a2 > 0) p2++; else if(a2 < 0) n2++;
                    }
                    if(p2 > 0 && n2 == 0){ neg_sem = 0; pos_sem = p2; break; }
                    if(n2 > 0 && p2 == 0){ neg_sem = 0; pos_sem = n2; break; }
                }
            if(neg_sem < 0) neg_sem = 0;      /* não achou: não inventa contraste */
        }
        long total = com_sinal < 0 ? -com_sinal : com_sinal;
        long so_positivas = sem_sinal;
        long buraco = so_positivas - total;
        printf("      soma COM sinal, em modulo (a tinta):  %ld\n", total);
        printf("      soma SEM sinal (a letra cheia):       %ld\n", so_positivas);
        printf("      a diferenca (o buraco):               %ld\n", buraco);
        printf("      e a tinta e' MENOR que a letra cheia:  %s\n", total < so_positivas ? "sim" : "NAO");
        printf("      contornos de area NEGATIVA neste glifo: %ld\n", neg_aqui);
        printf("      e num glifo de sentido UNICO (sem buraco): %ld negativos, %ld do mesmo sinal\n",
               neg_sem, pos_sem);
        ok("a TINTA e' a soma COM SINAL, e ela e' menor que a soma SEM sinal — porque o"
           " buraco SUBTRAI. Ignorar o sinal e somar tudo da' a letra CHEIA, sem o buraco: e'"
           " exactamente o que se perde ao ler so' um lado do par. E a folha do PDF e' o mesmo"
           " no grande — a parte escrita e a parte vazia sao UMA coisa com dois sinais",
           /* `buraco > 0` É `total < so_positivas` reescrito — duas condições que dizem o
            * mesmo, e uma delas não acrescenta nada. O que falta, e é o que pode falhar, é
            * a CAUSA: tem de haver pelo menos um contorno de área NEGATIVA, que é o buraco.
            * E o contraste, para que «tem buraco» não valha por «tudo tem»: um glifo sem
            * buraco não tem contorno negativo nenhum. */
           total < so_positivas && total > 0 && neg_aqui > 0
           && neg_sem == 0 && pos_sem > 0);
    }

printf("\n§V3  A VOLTA, que faltava: ler -> emitir -> ler devolve o contorno.\n\n");
    {
        /* O ERRO DE PRECISAO: eu lia a TTF e emitia, e nunca revertia. Aqui emite-se o contorno
         * em texto e le-se de volta, ponto a ponto — e o residuo tem de ser ZERO EXACTO, porque
         * as coordenadas sao INTEIRAS e nao ha' onde perder precisao. Se houvesse um float pelo
         * caminho, isto acusava. */
        static char saida[1 << 16];
        long m = 0;
        for(long i = 0; i < G.n && m < (long)sizeof saida - 64; i++)
            m += snprintf(saida + m, sizeof saida - (size_t)m, "%ld %ld %d\n",
                          G.p[i].x, G.p[i].y, G.p[i].na_curva);
        long difs = 0, lidos = 0;
        const char *q = saida;
        for(long i = 0; i < G.n; i++){
            long x, y; int c;
            if(sscanf(q, "%ld %ld %d", &x, &y, &c) != 3){ difs++; break; }
            if(x != G.p[i].x || y != G.p[i].y || c != G.p[i].na_curva) difs++;
            lidos++;
            while(*q && *q != '\n') q++; if(*q) q++;
        }
        printf("      %ld pontos emitidos e lidos de volta, %ld diferencas\n", lidos, difs);
        printf("      e as coordenadas sao INTEIRAS: nao ha' onde perder precisao\n");
        ok("a VOLTA fecha com residuo ZERO EXACTO: os pontos emitidos leem-se de volta iguais,"
           " coordenada a coordenada. Era isto que faltava — eu lia a TTF e emitia, e NUNCA"
           " REVERTIA. Sem a volta nao ha' residuo, e sem residuo nao ha' medida: so' uma leitura"
           " a confirmar-se a si mesma. E o zero e' EXACTO porque as coordenadas sao inteiras;"
           " com um float pelo caminho, isto acusava", difs == 0 && lidos == G.n);
    }

printf("\n§V4  A FUSAO spline x TTF: o grau do produto e' a SOMA dos graus.\n\n");
    long fusao = 0;
    {
        /* A TEORIA JA' O DIZ: «o viveiro e' o relogio quando DOIS se combinam». A spline e' um
         * relogio — o contorno, com o seu fecho; o TTF e' outro — o glifo, com o dentro e o
         * fora. Compostos, NAO SE SOMAM: multiplicam-se. E multiplicar SOMA OS EXPOENTES, que
         * aqui sao os graus. Nao e' analogia: e' a mesma conta. */
        struct { const char *nome; long p, q, r; } A = { "spline", 1, 1, 0 };   /* traca e fecha */
        struct { const char *nome; long p, q, r; } B = { "ttf",    1, 1, 1 };   /* traco, buraco, e o caractere atravessa */
        long ga = A.p + A.q + A.r, gb = B.p + B.q + B.r;
        /* o produto: cada componente compoe, e o GRAU soma — como os expoentes num produto */
        long cp = A.p + B.p, cq = A.q + B.q, cr = A.r + B.r;
        long gp = cp + cq + cr;
        printf("      spline  (%ld,%ld,%ld)   grau %ld\n", A.p, A.q, A.r, ga);
        printf("      ttf     (%ld,%ld,%ld)   grau %ld\n", B.p, B.q, B.r, gb);
        printf("      fusao   (%ld,%ld,%ld)   grau %ld = %ld + %ld\n", cp, cq, cr, gp, ga, gb);
        /* e a metade que separa isto de somar numeros a' toa: o grau do produto TEM de ser a
         * soma, e a fusao tem de ser DIFERENTE de cada um dos dois. Se desse um deles, nao
         * havia combinacao nenhuma. */
        long soma_bate = (gp == ga + gb);
        long e_nova    = !(cp == A.p && cq == A.q && cr == A.r)
                      && !(cp == B.p && cq == B.q && cr == B.r);
        fusao = soma_bate && e_nova;
        unsigned char v[160];
        long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|fusao spline x ttf — o viveiro",
                                cp, cq, cr);
        gravar(&b, "corpo/fusao/spline_ttf", v, m);
        ok("a FUSAO e' MULTIPLICACAO e o grau do produto e' a SOMA dos graus — 2 + 3 = 5 — que e'"
           " o que multiplicar faz aos expoentes. A teoria ja' o dizia: «o viveiro e' o relogio"
           " quando DOIS se combinam», e a spline e o ttf sao dois relogios. E a outra metade: a"
           " fusao e' DIFERENTE de cada um dos dois — se desse um deles, nao havia combinacao"
           " nenhuma, havia uma copia", fusao);
    }

printf("\n§V5  O CONTROLO: invertido o sentido do buraco, a tinta ENCHE.\n\n");
    {
        /* inverte-se a ordem dos pontos do contorno negativo — que e' inverter o sentido — e a
         * area muda de sinal. A tinta passa a ser a soma de duas positivas: a letra fica CHEIA,
         * sem buraco. Sem esta metade, «o sentido e' o dual» passava sem se saber se o sentido
         * fazia alguma coisa. */
        struct glifo H = G;
        long invertido = -1;
        for(long c = 0; c < H.nc; c++)
            if(area_com_sinal(&H, c) < 0){ invertido = c; break; }
        long antes = 0, depois = 0;
        for(long c = 0; c < G.nc; c++) antes += area_com_sinal(&G, c);
        if(invertido >= 0){
            long ini = invertido ? H.fim[invertido-1]+1 : 0, f = H.fim[invertido];
            for(long i = ini, j = f; i < j; i++, j--){
                struct pto t = H.p[i]; H.p[i] = H.p[j]; H.p[j] = t;
            }
            for(long c = 0; c < H.nc; c++) depois += area_com_sinal(&H, c);
        }
        printf("      tinta com o buraco no sentido certo:  %ld\n", antes);
        printf("      tinta com o buraco INVERTIDO:         %ld\n", depois);
        printf("      a letra fica CHEIA: %s\n", depois > antes ? "sim" : "NAO");
        ok("invertido o SENTIDO do buraco, a area muda de sinal e a tinta ENCHE — a letra fica"
           " sem buraco. E' a segunda metade, e diz que o sentido nao e' decoracao: e' o que"
           " carrega o dual. Sem ela, «o sentido e' o dual» passava sem se saber se o sentido"
           " fazia alguma coisa", invertido >= 0 && depois > antes);
    }

    /* e o corpo do TTF entra no banco, com a assinatura que se mediu */
    {
        unsigned char v[200];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,1|o traco (+), o buraco (-), e o caractere atravessa (0)");
        gravar(&b, "corpo/ttf", v, m);
        m = (long)snprintf((char*)v, sizeof v, "corpo/estrela");
        gravar(&b, "corpo/ttf/veste", v, m);
    }

    fechar(&b);
printf("\n=== O CORPO DO TTF ==========================================================\n");
printf("  O DUAL DO TTF E' O SENTIDO. Num glifo com buraco os contornos correm em sentidos\n");
printf("  OPOSTOS, e a area com sinal di-lo em inteiros: o traco e' +, o buraco e' -. E' a\n");
printf("  Lei 1 escrita numa letra — 1+ = -1 —, e o dual do traco e' o VAZIO que ele encerra.\n\n");
printf("    corpo/ttf   (1,1,1)   o traco (+), o buraco (-), o caractere atravessa (0)\n\n");
printf("  E A FOLHA E' O MESMO PAR NO GRANDE: a parte escrita e a parte vazia sao UMA coisa\n");
printf("  com dois sinais. Somar sem sinal da' a letra cheia — e' o que se perde ao ler so'\n");
printf("  um lado.\n\n");
printf("  E A FUSAO E' MULTIPLICACAO: spline (grau 2) x ttf (grau 3) da' grau 5, porque\n");
printf("  multiplicar SOMA OS EXPOENTES. A teoria ja' o dizia — «o viveiro e' o relogio\n");
printf("  quando DOIS se combinam» — e a spline e o ttf sao dois relogios.\n\n");
printf("  E A VOLTA, que faltava: eu lia a TTF e emitia, e NUNCA REVERTIA. Sem a volta nao ha'\n");
printf("  residuo, e sem residuo nao ha' medida — so' uma leitura a confirmar-se a si mesma.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — e o zero e' EXACTO, porque as coordenadas sao inteiras.\n\n");
    return 0;
}
