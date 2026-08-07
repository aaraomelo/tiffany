/* cards.c — OS CARDS PASSAM A VIVER NO BANCO. O manifesto deixa de ser a fonte.
 *
 * O Aarao: «agora ele tem que funcionar no sistema. Nao quero eles fora do sistema, e' um
 * sistema so'. Poe o banco (estrela) a gerenciar tudo, aqui local primeiro.»
 *
 * A virada e' de arquitectura e nao de codigo: o manifesto.json era a FONTE e o app lia-o
 * directamente — um ficheiro estatico ao lado do sistema. Agora a fonte e' o BANCO, e o
 * manifesto passa a ser uma PROJECCAO dele.
 *
 * E isso nao e' uma escolha de gosto: e' a regra da estrela aplicada aos dados. A estrela tem
 * os dois sentidos e por isso reverte (papers/corpo-estelar.tex §sec:estrela):
 *
 *      MOVE(slot, +1)  le'      — absorve, o lado negro
 *      MOVE(slot, -1)  escreve  — emite, o lado branco
 *
 * Um ficheiro que so' se le' e' meia operacao. Posto no banco, o card entra por um sentido e
 * sai pelo outro, e o par fecha:
 *
 *      DESCER   banco -> manifesto     a projeccao que o app consome
 *      SUBIR    manifesto -> banco     e tem de devolver o mesmo, com RESIDUO 0
 *
 * E' o promover/descer de lib/promove.h aplicado ao conteudo em vez de ao numero: quem escreve
 * o par ja' leu, e o que nao reverte nao entra.
 *
 *   §B1  os 92 cards entram no banco, um por chave
 *   §B2  e saem: cada um lido de volta byte a byte, RESIDUO 0
 *   §B3  a projeccao: do banco sai o manifesto, e do manifesto volta-se ao banco — a volta
 *        fecha, e e' isso que faz do ficheiro uma projeccao e nao uma segunda fonte
 *   §B4  o CONTROLO: um card corrompido NAO passa — o banco tem crc, e sem ele a volta
 *        fecharia na mesma e nao se saberia
 *
 *   cc -O2 -std=c99 -Wall -I../lib cards.c -o cards && ./cards
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"
#define NC   92

/* um card, como o manifesto o tem: a chave e' seccao/nome */
struct card { char chave[128]; unsigned char val[VMAX]; long n; };

/* le' o manifesto e parte-o em cards — sem parser de JSON: procura os limites por chavetas,
 * que e' o que ha' de mais simples e nao traz dependencia nenhuma. */
static long carrega(const char *caminho, struct card *cs, long cap)
{
    FILE *f = fopen(caminho, "rb");
    if(!f) return 0;
    static char buf[4*1024*1024];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n] = 0;

    long nc = 0; const char *p = buf;
    while(nc < cap && (p = strstr(p, "\"nome\":")) != NULL){
        /* recua ate' a chaveta que abre este objecto */
        const char *ini = p;
        while(ini > buf && *ini != '{') ini--;
        /* avanca ate' a chaveta que o fecha, contando */
        const char *fim = ini; int d = 0;
        do { if(*fim == '{') d++; else if(*fim == '}') d--; fim++; } while(d > 0 && *fim);
        long len = fim - ini;
        if(len > 0 && len < VMAX){
            /* a chave: o valor de "nome" mais o indice, para nao colidir */
            const char *q = strchr(p, ':'); q = strchr(q, '"'); q++;
            const char *r = strchr(q, '"');
            long kn = r - q; if(kn > 90) kn = 90;
            snprintf(cs[nc].chave, sizeof cs[nc].chave, "%.*s#%ld", (int)kn, q, nc);
            memcpy(cs[nc].val, ini, (size_t)len);
            cs[nc].n = len;
            nc++;
        }
        p = fim;
    }
    return nc;
}

int main(void)
{
    long falhas = 0;
    puts("\n=== OS CARDS NO BANCO — a estrela a gerir, e nao um ficheiro ao lado ===\n");

    static struct card cs[256];
    long nc = carrega("app/src/manifesto.json", cs, 256);
    if(nc == 0) nc = carrega("../app/src/manifesto.json", cs, 256);
    if(nc == 0) nc = carrega("app/src/manifesto.json", cs, 256);
    if(nc == 0) nc = carrega("../app/src/manifesto.json", cs, 256);

    /* ═══ §B1 — os cards entram no banco ═══════════════════════════════════════════ */
    struct base b;
    {
        char pd[512], pi[512];
        snprintf(pd, sizeof pd, "%s.dat", BASE); snprintf(pi, sizeof pi, "%s.idx", BASE);
        remove(pd); remove(pi);
        if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }
        long postos = 0, recusados = 0;
        for(long i = 0; i < nc; i++){
            if(gravar(&b, cs[i].chave, cs[i].val, cs[i].n)) postos++;
            else recusados++;
        }
        printf("  §B1  objectos com nome no manifesto: %ld (as 92 pecas + os 7 documentos)\n"
               "       postos no banco: %ld    recusados: %ld\n\n", nc, postos, recusados);
        ok("os cards entram no BANCO, um por chave — e o manifesto deixa de ser a fonte para"
           " passar a projeccao. Nao e' arrumacao: um ficheiro que so' se le' e' MEIA operacao,"
           " e a estrela e' o que tem os dois sentidos. Postos todos os que se leram, e nenhum"
           " recusado — e sao 99 e nao 92, porque os sete DOCUMENTOS tambem tem nome e tambem entram:"
           " a fonte e' uma so' para tudo o que o site serve", nc == 99 && postos == nc
           && recusados == 0);
    }

    /* ═══ §B2 — e saem: byte a byte, residuo 0 ═════════════════════════════════════ */
    {
        long resid = 0, lidos = 0;
        unsigned char out[VMAX];
        for(long i = 0; i < nc; i++){
            long n = ler(&b, cs[i].chave, out, sizeof out);
            if(n != cs[i].n){ resid++; continue; }
            if(memcmp(out, cs[i].val, (size_t)n) != 0) resid++;
            lidos++;
        }
        printf("  §B2  lidos de volta: %ld    residuo byte a byte: %ld\n\n", lidos, resid);
        ok("e SAEM pelo outro sentido, byte a byte, com residuo ZERO. E' o par a fechar — entra"
           " por MOVE(-1) e sai por MOVE(+1) —, e e' isso que faz do banco uma estrela e nao um"
           " ficheiro: um ficheiro guarda, uma estrela guarda E devolve", resid == 0 && lidos == nc);
    }

    /* ═══ §B3 — a projeccao: banco -> manifesto -> banco ═══════════════════════════
     * O manifesto passa a sair do banco. E para ele ser PROJECCAO e nao segunda fonte, a
     * volta tem de fechar: reconstruido a partir do que se projectou, o banco devolve o
     * mesmo. Se nao fechasse, seriam duas fontes a divergir. */
    {
        long resid = 0;
        unsigned char out[VMAX];
        /* desce: escreve-se a projeccao */
        FILE *f = fopen("/tmp/cards_projeccao.json", "wb");
        fputs("{\"cards\":[", f);
        for(long i = 0; i < nc; i++){
            long n = ler(&b, cs[i].chave, out, sizeof out);
            if(n <= 0){ resid++; continue; }
            fwrite(out, 1, (size_t)n, f);
            if(i + 1 < nc) fputc(',', f);
        }
        fputs("]}", f); fclose(f);
        /* sobe: le-se a projeccao e compara-se com o banco */
        static struct card vv[256];
        long nv = carrega("/tmp/cards_projeccao.json", vv, 256);
        long divergem = 0;
        for(long i = 0; i < nv && i < nc; i++){
            long n = ler(&b, cs[i].chave, out, sizeof out);
            if(n != vv[i].n || memcmp(out, vv[i].val, (size_t)n) != 0) divergem++;
        }
        printf("  §B3  projectados: %ld    relidos da projeccao: %ld    divergem: %ld\n\n",
               nc, nv, divergem);
        ok("e a PROJECCAO fecha: do banco sai o manifesto e do manifesto volta-se ao banco sem"
           " divergencia. E' isso que faz do ficheiro uma projeccao e nao uma segunda fonte —"
           " duas fontes divergem, uma fonte e a sua projeccao nao podem", resid == 0
           && nv == nc && divergem == 0);
    }

    /* ═══ §B4 — o CONTROLO: um card corrompido nao passa ═══════════════════════════ */
    {
        unsigned char out[VMAX];
        long acusou = 0, tentativas = 0;
        /* corrompe-se o .dat num byte de conteudo e ve-se se a leitura acusa */
        char pd[512]; snprintf(pd, sizeof pd, "%s.dat", BASE);
        FILE *f = fopen(pd, "r+b");
        if(f){
            fseek(f, 40, SEEK_SET);
            int c = fgetc(f); fseek(f, 40, SEEK_SET); fputc(c ^ 0xFF, f);
            fclose(f);
            struct base b2;
            if(abrir(&b2, BASE, 0)){
                for(long i = 0; i < nc; i++){
                    long n = ler(&b2, cs[i].chave, out, sizeof out);
                    tentativas++;
                    if(n <= 0 || memcmp(out, cs[i].val, (size_t)(n > 0 ? n : 0)) != 0) acusou++;
                }
                fechar(&b2);
            }
        }
        printf("  §B4  com um byte corrompido: %ld de %ld leituras acusam\n\n", acusou, tentativas);
        ok("e o CONTROLO: um card corrompido NAO passa. Trocado um byte no ficheiro, a leitura"
           " acusa — o banco leva crc, e sem ele a volta fecharia na mesma e ninguem saberia."
           " E' a segunda metade da medida: a primeira diz que o que esta' certo passa, esta diz"
           " que o que esta' errado nao. Acusa UM — o que foi corrompido — e nao todos: se"
           " acusasse todos, o que estaria a falhar era a leitura e nao o crc",
           acusou == 1 && tentativas == nc);
    }

    fechar(&b);
    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A FONTE PASSOU A SER O BANCO, E O MANIFESTO E' UMA PROJECCAO:");
        puts("");
        puts("    SUBIR   manifesto -> banco    MOVE(-1): emite, o lado branco");
        puts("    DESCER  banco -> manifesto    MOVE(+1): absorve, o lado negro");
        puts("    e a volta fecha com residuo 0 — por isso e' UMA fonte e nao duas");
        puts("");
        puts("  Um ficheiro que so' se le' e' meia operacao. A estrela e' o que tem os dois");
        puts("  sentidos, e agora os cards vivem dentro dela em vez de ao lado.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
