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
 *   §B5  o RELOGIO: a animacao e' o FLUXO, e o relogio sao os instantes em que ele FECHA —
 *        e os kernels entram no banco tambem: nada fica fora da estrela
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

    /* ═══ §B5 — O RELOGIO: a animacao nao e' tempo a correr, e' o fluxo a fechar ═══
     * O Aarao: «a animacao seria o tempo correndo, o relogio actuando. Concentra a estrela,
     * tudo nela, nada fora.»
     *
     * E a teoria resolve a tensao com o paper, que diz que o sistema NAO TEM TEMPO
     * (§sec:estrela: «o que se mede como tempo e' latencia»). A teoria (§sec:relogio):
     *
     *     «O fluxo NAO e' o relogio. O relogio e' o conjunto dos instantes em que o fluxo
     *      VOLTA A SI PROPRIO — e esses instantes, por serem pontos fixos de uma orbita que
     *      nao dissipa, sao isolados e igualmente espacados. Formam um reticulado.»
     *
     * Logo a animacao nao e' um relogio a andar: e' o FLUXO, continuo. O relogio sao os
     * instantes em que ele FECHA — e sao esses que se contam, nao os fotogramas. Um fluxo que
     * nunca fecha nao tem relogio nenhum, por mais depressa que corra.
     *
     * Mede-se: os kernels entram no banco como tudo o resto (nada fora da estrela), e o fluxo
     * de cada um tem periodo — volta a si proprio em instantes igualmente espacados. */
    {
        /* os kernels, no banco — nada fica fora */
        const char *ks[] = { "aura", "galaxia", "floco", "cristal", "espiral", "onda", "campo" };
        long postos = 0, lidos = 0, resid = 0;
        unsigned char out[VMAX];
        for(int i = 0; i < 7; i++){
            char chave[64]; snprintf(chave, sizeof chave, "kernel/%s", ks[i]);
            unsigned char v[64]; long n = (long)snprintf((char*)v, sizeof v, "{\"kernel\":\"%s\"}", ks[i]);
            if(gravar(&b, chave, v, n)) postos++;
            long m = ler(&b, chave, out, sizeof out);
            if(m == n && memcmp(out, v, (size_t)n) == 0) lidos++; else resid++;
        }

        /* e o RELOGIO do fluxo: os instantes em que ele volta a si proprio.
         * o fluxo e' a rotacao por J (periodo 4); conta-se em que passos fecha. */
        long fecha_em[16], nf = 0, espacamento_maus = 0;
        { long x = 1, y = 0;
          for(long k = 1; k <= 12; k++){
              long nx = -y, ny = x; x = nx; y = ny;              /* um passo do fluxo */
              if(x == 1 && y == 0 && nf < 16) fecha_em[nf++] = k; /* voltou a si proprio */
          } }
        /* os instantes tem de ser IGUALMENTE ESPACADOS — e' o que faz deles um reticulado */
        for(long i = 1; i + 1 < nf; i++)
            if(fecha_em[i+1] - fecha_em[i] != fecha_em[1] - fecha_em[0]) espacamento_maus++;

        /* e o CONTROLO: um fluxo que NAO fecha nao tem relogio, por mais que corra */
        long sem_relogio = 0;
        /* uma translacao a serio: x -> x + 1. Corre tanto como o rotor e NUNCA volta.
         * (A primeira versao usava x -> x + y com y = 0, que nao movia nada e fechava de
         *  graca — um controlo que nao se move nao controla.) */
        { long x = 1, y = 0, voltou = 0;
          for(long k = 1; k <= 12; k++){ x = x + 1; if(x == 1 && y == 0) voltou = 1; }
          if(!voltou) sem_relogio = 1; }

        printf("  §B5  kernels no banco: %ld postos, %ld lidos de volta, %ld residuo\n",
               postos, lidos, resid);
        printf("       o fluxo fecha em k = ");
        for(long i = 0; i < nf; i++) printf("%ld ", fecha_em[i]);
        printf(" — espacamento constante: %s\n", espacamento_maus ? "NAO" : "sim");
        printf("       e um fluxo que nao fecha: %s tem relogio\n\n", sem_relogio ? "NAO" : "");
        ok("O RELOGIO NAO E' A ANIMACAO A CORRER. A teoria di-lo e o paper exige-o: o sistema nao"
           " tem tempo, so' latencia — e o relogio e' «o conjunto dos instantes em que o fluxo"
           " VOLTA A SI PROPRIO», isolados e igualmente espacados. Logo a animacao e' o FLUXO,"
           " continuo, e o relogio sao os instantes em que ele FECHA: conta-se o fechar, e nao"
           " os fotogramas. Medido nos dois lados — o fluxo de J fecha em instantes igualmente"
           " espacados, e uma translacao, que corre na mesma, NAO fecha e por isso nao tem"
           " relogio nenhum. E os kernels entraram no banco como tudo o resto: nada fica fora"
           " da estrela", postos == 7 && lidos == 7 && resid == 0
           && nf >= 2 && espacamento_maus == 0 && sem_relogio == 1);
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
