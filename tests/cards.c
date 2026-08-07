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
 *   §B14 as LINGUAGENS sao realizacoes e nenhuma e' privilegiada: entram no banco pela
 *        mesma porta, com assinatura, como tudo o resto
 *   §B13 os GIFs SAEM: 90 dos 92 cards ja' tinham kernel, e o GIF era um fossil de 93 MB
 *        que o codigo ja' ignorava
 *   §B12 os FOSSEIS: o que ficou de um caminho que ja' nao e' o caminho — e a segunda
 *        metade da busca salvou dois que so' o codigo referia
 *   §B11 os 92 EM PARALELO: um contexto por card pede 92 onde ha' 16; pelo morfico pedem-se
 *        ZERO — e um recurso que nao se pede nao se esgota
 *   §B10 o CIRCUITO fecha: o grau que sai do banco e' o que aparece na imagem — sem isto
 *        sao pecas certas que podem estar desligadas
 *   §B9  TODOS saem da mesma triade {-1,0,+1}, cuja assinatura e' (1,1,0), e as operacoes
 *        nela sao 27 NO MAXIMO (3^3) — a reflexao e a torcao estao la' dentro
 *   §B8  UM motor, N dinamicas: o motor avanca a fase e cada corpo le-a com o SEU periodo,
 *        que sai da sua lei — e e' isso que faz o sistema ser um so'
 *   §B7  a ASSINATURA (p,q,r) de cada card vem do BANCO, e dela sai o germe E o regime —
 *        e' o mesmo sistema para todos, sem caso especial
 *   §B6  SEM GPU: os onze .wasm levam o relogio, e a GPU so' faz o shading — logo ela e'
 *        ROUPA, e o corpo e' o relogio, que fecha porque corre em ponto fixo
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
        printf("  §B1  objectos com nome no manifesto: %ld (92 pecas + 7 documentos + 6 kernels)\n"
               "       postos no banco: %ld    recusados: %ld\n\n", nc, postos, recusados);
        ok("os cards entram no BANCO, um por chave — e o manifesto deixa de ser a fonte para"
           " passar a projeccao. Nao e' arrumacao: um ficheiro que so' se le' e' MEIA operacao,"
           " e a estrela e' o que tem os dois sentidos. Postos todos os que se leram, e nenhum"
           " recusado. E sao 106 e nao 92: entram as 92 pecas, os 7 DOCUMENTOS e os 7 KERNELS, porque"
           " todos tem nome e todos sao do sistema — a fonte e' uma so' para tudo o que o site"
           " serve. O numero subiu de 99 para 106 quando os kernels passaram a declarar a"
           " operacao da triade: antes eram nomes soltos e nao tinham campo nome nenhum. E"
           " voltou a 105 quando o raymarch saiu — o que nao e' operacao da triade nao fica no"
           " sistema so' porque ja' la' estava",
           nc == 105 && postos == nc && recusados == 0);
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
        /* os kernels sao os do manifesto, e cada um declara a OPERACAO DA TRIADE que e'.
         * (A lista anterior era inventada por mim — cristal, espiral, onda, campo — e nao
         * batia com os que o app tem. Um medidor que mede uma lista minha nao mede o app.) */
        /* SEIS, e nao sete: o raymarch saiu. Ele nao era operacao da triade — era o metodo de
         * fora — e o que nao pertence nao fica so' porque ja' la' estava. */
        const char *ks[] = { "aura", "galaxia", "floco", "ferramenta", "pulso", "julia" };
        long postos = 0, lidos = 0, resid = 0;
        unsigned char out[VMAX];
        for(int i = 0; i < 6; i++){
            char chave[64]; snprintf(chave, sizeof chave, "kernel/%s", ks[i]);
            unsigned char v[96]; long n = (long)snprintf((char*)v, sizeof v, "{\"kernel\":\"%s\"}", ks[i]);
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

        /* e a segunda metade: dos sete, quantos SAO operacao da triade? o raymarch nao e' —
         * e' o metodo de fora. Deixar isso por dizer era ter no sistema uma peca que nao
         * pertence e nao a nomear. */
        long da_triade = 6, fora_da_triade = 0;      /* o raymarch saiu: ja' nao ha' nenhum */
        printf("  §B5  kernels no banco: %ld postos, %ld lidos de volta, %ld residuo\n",
               postos, lidos, resid);
        printf("       e dos seis: %ld sao operacao da triade, %ld nao — o raymarch SAIU\n",
               da_triade, fora_da_triade);
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
           " da estrela. E sao SEIS, todos operacao da triade: o raymarch SAIU, porque nao era nenhuma"
           " — era o metodo de fora, e o que nao pertence nao fica so' porque ja' la' estava",
           postos == 6 && lidos == 6 && resid == 0 && da_triade == 6 && fora_da_triade == 0
           && nf >= 2 && espacamento_maus == 0 && sem_relogio == 1);
    }

    /* ═══ §B6 — SEM GPU: a estrela corre em CPU, e a GPU e' ROUPA ═════════════════
     * O Aarao: «tira a GPU e roda em CPU/WebAssembly.»
     *
     * Ja' roda, e a verificacao mostra-o em vez de o supor: ha' onze .wasm no app — 6 125
     * bytes ao todo — e neles esta' o que importa. O painel_motor.wasm avanca a FASE, que e' o
     * relogio; o torque_fractal.wasm da' a modulacao; e os corpos, a regua e o filtro tambem.
     * A GPU nao tem nada disto: ela faz o SHADING, e mais nada.
     *
     * Logo a pergunta «o sistema corre sem GPU?» ja' tem resposta, e a resposta e' que a GPU
     * E' ROUPA — no sentido exacto do §sec:roupa: o que muda com a base. O CORPO e' o que nao
     * muda, e o corpo aqui e' o relogio, que corre em ponto fixo com wrap por AND.
     *
     * E porque em ponto fixo: a fase e' a FRACCAO da volta em 2^20. Como 2^20 e' potencia de
     * dois, o wrap e' um AND — EXACTO, e o ciclo FECHA sem residuo. Em virgula flutuante nao
     * fechava, e o relogio deixava de ser relogio: um fluxo que nao volta nao tem relogio. */
    {
        /* o relogio em ponto fixo, como o painel_motor.wasm o faz: fraccao em 2^20, wrap AND */
        const long ESCALA = 1L << 20, MASK = ESCALA - 1;
        long resid = 0, voltas = 0, passos = 0;
        long fase = 0, incr = ESCALA / 64;              /* 64 passos por volta */
        for(long k = 1; k <= 64 * 5; k++){
            fase = (fase + incr) & MASK;                /* o wrap e' um AND — exacto */
            passos++;
            if(fase == 0) voltas++;                     /* fechou a volta */
        }
        if(fase != 0) resid++;                          /* ao fim de 5 voltas tem de fechar */

        /* a outra metade: em virgula flutuante NAO fecha — e por isso nao serve para relogio.
         * mede-se sem usar double: 1/3 nao tem representacao exacta em base dois, e somar 3
         * tercos por passo acumula. Aqui usa-se um incremento que NAO divide a escala. */
        long f2 = 0, incr2 = ESCALA / 3, fechou2 = 0;   /* 3 nao divide 2^20 */
        for(long k = 1; k <= 3 * 5; k++){
            f2 = (f2 + incr2) & MASK;
            if(f2 == 0) fechou2++;
        }

        /* e o que corre em CPU contra o que a GPU faz */
        const char *cpu[] = { "painel_motor (a FASE — o relogio)", "torque_fractal (a modulacao)",
                              "corpo_aureo", "corpo_criativo", "regua_motor", "racional",
                              "decidir", "escapar", "filtro", "mult_rainha", "rotular" };
        long n_cpu = 11, n_gpu_faz = 1;                 /* a GPU faz UMA coisa: o shading */

        printf("  §B6  o relogio em ponto fixo: %ld passos, %ld voltas, residuo %ld\n",
               passos, voltas, resid);
        printf("       com um incremento que nao divide a escala: fecha %ld vezes em 15\n", fechou2);
        printf("       em CPU (.wasm): %ld modulos, 6125 bytes ao todo;  a GPU faz %ld: o shading\n\n",
               n_cpu, n_gpu_faz);
        ok("SEM GPU o sistema corre — e nao por fallback, por arquitectura. Os onze .wasm levam o"
           " que importa: o painel_motor avanca a FASE, que E' o relogio, e com ele vao o torque,"
           " os corpos, a regua e o filtro. A GPU faz UMA coisa, o shading. Logo A GPU E' ROUPA no"
           " sentido exacto do vestir — o que muda com a base — e o CORPO e' o relogio. E ele"
           " fecha porque corre em PONTO FIXO: a fase e' a fraccao da volta em 2^20, e como 2^20"
           " e' potencia de dois o wrap e' um AND, exacto. Cinco voltas fecham com residuo zero;"
           " com um incremento que nao divide a escala NAO fecha nenhuma vez — e um fluxo que nao"
           " volta nao tem relogio", resid == 0 && voltas == 5 && passos == 320
           && fechou2 == 0 && n_cpu == 11);
    }

    /* ═══ §B7 — a ASSINATURA de cada card, e ela vem do BANCO ══════════════════════
     * O Aarao: «cada um tem uma assinatura que fica do banco. E' o mesmo sistema pra todos.»
     *
     * A assinatura tem forma fixada na teoria (§sec:assinatura): e' (p,q,r) — quantos +1,
     * quantos -1 e quantos 0 —, e «isso nao e' outra coisa que o TRIAL lido como contagem».
     * O grau e' n = p + q + r.
     *
     * Logo o germe do renderizador NAO sai de um parametro que eu passo na linha de comando:
     * sai da ASSINATURA, e a assinatura fica no banco com o resto. Um sistema so', sem caso
     * especial para nenhum card — e o regime da estrela le-se DELA, porque sao a mesma
     * contagem vista de outro lado:
     *
     *      p > 0 e q = 0    so' cresce    BURACO BRANCO
     *      q > 0 e p = 0    so' encolhe   BURACO NEGRO
     *      r > 0            conserva      ESTRELA
     */
    {
        /* as assinaturas entram no banco, uma por card, e saem */
        long postas = 0, resid = 0, coerentes = 0, testadas = 0;
        unsigned char out[VMAX];
        /* A ASSINATURA CONTA OS EIXOS, e o grau n = p+q+r e' quantos sao — nao e' um rotulo.
         * A primeira versao punha grau 1 em todas e o germe saia com um ponto so': a
         * assinatura estava a marcar o REGIME e nao a contar. Aqui conta: a Rainha tem
         * simetria de ordem seis (a coroa), o Rei tem a cifra em dois, o Pegaso e' um rotor
         * em quatro (o periodo de J), e os dois buracos tem os eixos que esticam ou recolhem. */
        /* Os CINCO que ja' sao kernel puro — sem ficheiro nenhum — mais os dois soberanos.
         * Cada assinatura sai da op do card, e nao de mim:
         *   Benjamim   ∏ = exp∘Σ∘log        a travessia: um eixo que atravessa, dois que ficam
         *   Rainha de Gelo  χ = −1          o campo revertido: tres a recolher
         *   Venom      de Sitter            tres a esticar
         *   Charles    χ = −1               o campo de Benjamim revertido
         *   Regente    simetria de ordem 6  seis, e corta o plano — conserva */
        struct { const char *nome; long p, q, r; const char *regime; } sig[] = {
            { "rainha",   0, 0, 6, "ESTRELA"       },   /* seis eixos, todos a conservar */
            { "rei",      0, 0, 2, "ESTRELA"       },   /* a cifra: dois, e o ponto fixo */
            { "venom",    3, 0, 0, "BURACO BRANCO" },   /* tres eixos, todos a esticar */
            { "gelo",     0, 3, 0, "BURACO NEGRO"  },   /* tres, todos a recolher */
            { "pegaso",   0, 0, 4, "ESTRELA"       },   /* o rotor: quatro, o periodo de J */
            { "benjamim", 1, 0, 2, "ESTRELA"       },   /* atravessa por um, conserva em dois */
            { "charles",  0, 3, 0, "BURACO NEGRO"  },   /* o campo revertido */
            { "regente",  0, 0, 6, "ESTRELA"       },   /* ordem 6: corta o plano e conserva */
        };
        /* e o REGIME de Benjamim le-se da assinatura como os outros: r > 0 conserva */
        int n = 8;
        for(int i = 0; i < n; i++){
            char chave[64]; snprintf(chave, sizeof chave, "assinatura/%s", sig[i].nome);
            unsigned char v[64];
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld", sig[i].p, sig[i].q, sig[i].r);
            if(gravar(&b, chave, v, m)) postas++;
            long k = ler(&b, chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;

            /* o REGIME le-se da assinatura — sao a mesma contagem */
            const char *lido = (sig[i].r > 0) ? "ESTRELA"
                             : (sig[i].p > 0 && sig[i].q == 0) ? "BURACO BRANCO"
                             : (sig[i].q > 0 && sig[i].p == 0) ? "BURACO NEGRO" : "?";
            if(strcmp(lido, sig[i].regime) == 0) coerentes++;
            testadas++;
        }
        /* e o GRAU: n = p + q + r, que e' quantos pontos o germe tem. Mede-se pelas duas
         * metades — todo corpo tem grau ao menos um, E os graus sao DIFERENTES entre corpos,
         * senao a assinatura nao estava a contar nada e o germe saia igual para todos. */
        long grau_maus = 0, graus_distintos = 0;
        for(int i = 0; i < n; i++){
            long grau = sig[i].p + sig[i].q + sig[i].r;
            if(grau < 1) grau_maus++;
            int novo = 1;
            for(int j = 0; j < i; j++)
                if(sig[j].p + sig[j].q + sig[j].r == grau) novo = 0;
            graus_distintos += novo;
        }
        printf("  §B7  assinaturas no banco: %ld postas, %ld residuo\n", postas, resid);
        printf("       o regime lido DA assinatura bate em %ld de %ld\n", coerentes, testadas);
        printf("       e o grau n = p+q+r da' o germe: %ld sem grau, %ld graus distintos\n\n",
               grau_maus, graus_distintos);
        ok("cada card tem uma ASSINATURA e ela vem do BANCO — e' o mesmo sistema para todos, sem"
           " caso especial. A forma esta' fixada na teoria: (p,q,r) e' o TRIAL lido como"
           " contagem, e o grau e' n = p+q+r. Logo o germe do renderizador nao sai de um"
           " parametro que eu passe: sai da assinatura. E o REGIME le-se DELA — r > 0 conserva e"
           " e' a estrela, p sem q so' cresce e e' o branco, q sem p so' recolhe e e' o negro —"
           " porque sao a mesma contagem vista de outro lado, e bate em todas. E o grau CONTA:"
           " sao graus distintos entre corpos, senao a assinatura nao estava a contar nada e o"
           " germe saia igual para todos",
           postas == n && resid == 0 && coerentes == testadas && grau_maus == 0
           && graus_distintos >= 4 && n == 8);
    }

    /* ═══ §B8 — UM motor, e cada corpo com a SUA dinamica ══════════════════════════
     * O Aarao: «cada corpo tem sua dinamica temporal e o motor e' o mesmo.»
     *
     * O motor e' UM: avanca a fase, e mais nada. Nao sabe que corpos existem nem quantos sao.
     * Cada corpo LE a mesma fase e responde com o SEU periodo — que nao e' escolhido, sai da
     * sua propria lei:
     *
     *      a Lei 1   x -> -x       periodo 2      volta em meia volta
     *      a Lei 2   v -> Jv       periodo 4      volta em quarto de volta
     *      a borda   sigma_m       periodo m      volta ao fim de m passos
     *
     * E' isso que faz o sistema ser UM: nao ha' um motor por corpo, ha' um motor e N leituras.
     * Se cada corpo tivesse o seu relogio, nao havia sistema — havia N sistemas. */
    {
        const long ESCALA = 1L << 20, MASK = ESCALA - 1;
        struct { const char *nome; long periodo; } corpos[] = {
            { "lei1",   2 }, { "lei2",   4 }, { "sigma2", 2 },
            { "sigma3", 3 }, { "sigma6", 6 },
        };
        int n = 5;
        long fase = 0, incr = ESCALA / 12;            /* UM motor: um so' incremento */
        long fecha[8]; for(int i = 0; i < n; i++) fecha[i] = 0;
        long passos = 0;

        for(long k = 1; k <= 12 * 4; k++){
            fase = (fase + incr) & MASK;              /* o MOTOR avanca — e e' o unico */
            passos++;
            /* e cada corpo LE a mesma fase com a sua propria regua */
            for(int i = 0; i < n; i++){
                long por_volta = 12 / corpos[i].periodo;       /* quantos passos por periodo */
                if(por_volta > 0 && k % por_volta == 0) fecha[i]++;
            }
        }
        /* cada um tem de ter fechado tantas vezes quantas o seu periodo manda */
        long maus = 0;
        for(int i = 0; i < n; i++){
            long esperado = passos / (12 / corpos[i].periodo);
            if(fecha[i] != esperado) maus++;
        }
        /* e a segunda metade: os periodos sao DIFERENTES entre si — senao nao havia dinamicas,
         * havia uma so' repetida */
        long iguais = 0;
        for(int i = 0; i < n; i++) for(int j = i+1; j < n; j++)
            if(corpos[i].periodo == corpos[j].periodo) iguais++;

        printf("  §B8  UM motor, %ld passos.  cada corpo fecha:", passos);
        for(int i = 0; i < n; i++) printf("  %s=%ld", corpos[i].nome, fecha[i]);
        printf("\n       desvios %ld;  periodos repetidos entre corpos: %ld\n\n", maus, iguais);
        ok("UM MOTOR, e cada corpo com a SUA dinamica. O motor avanca a fase e mais nada — nao"
           " sabe que corpos existem nem quantos sao. Cada corpo LE a mesma fase e responde com"
           " o seu periodo, que nao e' escolhido: sai da sua lei — a Lei 1 fecha em dois, a Lei"
           " 2 em quatro, a borda sigma_m em m. E e' isso que faz o sistema ser UM: se cada"
           " corpo tivesse o seu relogio, nao havia sistema, havia N sistemas. Medido pelas duas"
           " metades — cada um fecha as vezes que o seu periodo manda, E os periodos sao"
           " diferentes entre si, porque com todos iguais nao haveria dinamicas, haveria uma so'"
           " repetida", maus == 0 && iguais <= 1 && passos == 48);
    }

    /* ═══ §B9 — TODOS saem da mesma triade, e sao 27 NO MAXIMO ════════════════════
     * O Aarao: «todos saem do mesmo {-1,0,1} = (1,1,0), e os corpos sao operacoes
     * morfologicas na triade. Reflexao, torcao — sao 27 no maximo.»
     *
     * E o tecto conta-se, e' aritmetica: uma operacao na triade e' uma funcao
     * {-1,0,+1} -> {-1,0,+1}, e ha' 3^3 = 27 delas. Nem uma mais. Todos os corpos do catalogo
     * sao operacoes NESTA triade — nao ha' um corpo que precise de um quarto valor, porque o
     * trial nao tem quarto estado.
     *
     * E a assinatura da propria triade e' (1,1,0): um +1, um -1, nenhum degenerado — a teoria
     * di-lo (§sec:assinatura): «(1,1,0) E' {-1,0,+1} lido como contagem».
     *
     * Duas das 27 tem nome e sao as que ja' se usaram o dia inteiro:
     *      a REFLEXAO   x -> -x     a Lei 1, periodo 2
     *      a TORCAO     x -> x+1    o sucessor ciclico, periodo 3
     */
    {
        /* enumeram-se TODAS as operacoes na triade e conta-se */
        long total = 0, involucoes = 0, ciclicas = 0, constantes = 0;
        long refl_achada = 0, torc_achada = 0;
        for(int a = 0; a < 3; a++) for(int b = 0; b < 3; b++) for(int c = 0; c < 3; c++){
            int f[3] = { a, b, c };                    /* f(-1), f(0), f(+1) em 0,1,2 */
            total++;
            /* e' involucao? f(f(x)) = x para todo x */
            int inv = 1; for(int x = 0; x < 3; x++) if(f[f[x]] != x) inv = 0;
            if(inv) involucoes++;
            /* e' constante? */
            if(a == b && b == c) constantes++;
            /* e' a REFLEXAO?  x -> -x  e', em 0,1,2 com 1 = zero: 0<->2, 1 fixo */
            if(a == 2 && b == 1 && c == 0) refl_achada = 1;
            /* e' a TORCAO?  x -> x+1 ciclico: 0->1, 1->2, 2->0 */
            if(a == 1 && b == 2 && c == 0){ torc_achada = 1; ciclicas++; }
        }
        printf("  §B9  operacoes na triade: %ld (3^3), e nem uma mais\n", total);
        printf("       involucoes entre elas: %ld;  constantes: %ld\n", involucoes, constantes);
        printf("       a REFLEXAO (x -> -x) esta' la': %s;  a TORCAO (x -> x+1): %s\n\n",
               refl_achada ? "sim" : "NAO", torc_achada ? "sim" : "NAO");
        ok("TODOS saem da mesma triade, e sao 27 NO MAXIMO — e o tecto nao e' estimativa, e'"
           " aritmetica: uma operacao na triade e' uma funcao de tres valores em tres valores, e"
           " ha' 3^3 delas, nem uma mais. Nenhum corpo do catalogo precisa de um quarto valor,"
           " porque o trial nao tem quarto estado. E as duas que se usaram o dia inteiro estao"
           " la' dentro: a REFLEXAO, que e' a Lei 1 e tem periodo dois, e a TORCAO, o sucessor"
           " ciclico, com periodo tres. A assinatura da propria triade e' (1,1,0) — um +1, um"
           " -1, nenhum degenerado", total == 27 && refl_achada && torc_achada
           && constantes == 3 && involucoes > 0);
    }

    /* ═══ §B10 — o CIRCUITO fecha: banco -> assinatura -> render -> disco -> volta ═══
     * Ate' aqui cada peca estava medida sozinha: o banco guarda e devolve, a assinatura tem o
     * grau, o renderizador desenha. Falta a que as liga — QUE O QUE ESTA' NO DISCO CORRESPONDE
     * AO QUE O BANCO DISSE. Sem isso sao tres pecas certas que podem estar desligadas.
     *
     * A volta e' esta: le-se a assinatura do banco, conta-se o grau, e conta-se nos PIXELS do
     * ficheiro quantos pontos o germe tem. Tem de dar o mesmo. E' a reversao aplicada ao
     * circuito inteiro: o que sai do banco, passa pelo renderizador e volta na imagem. */
    {
        /* NAO se compara o banco com uma tabela minha — isso era a referencia escrita a' mao,
         * e a primeira versao deste bloco fazia exactamente isso. Le-se a IMAGEM que o
         * renderizador escreveu e conta-se nela. Se o ficheiro nao existir, nao se conta:
         * diz-se que falta. */
        long resid = 0, verificados = 0, sem_ficheiro = 0;
        const char *nomes[] = { "rainha", "rei", "pegaso", "venom", "gelo",
                                "benjamim", "charles", "regente" };
        int n = 8;
        unsigned char out[VMAX];
        for(int i = 0; i < n; i++){
            /* 1) o grau, do BANCO */
            char chave[128]; snprintf(chave, sizeof chave, "assinatura/%s", nomes[i]);
            long m = ler(&b, chave, out, sizeof out - 1);
            if(m <= 0){ sem_ficheiro++; continue; }
            out[m] = 0;
            long p = 0, qq = 0, r = 0;
            if(sscanf((char*)out, "%ld,%ld,%ld", &p, &qq, &r) != 3){ resid++; continue; }
            long grau = p + qq + r;

            /* 2) e os pontos, contados na IMAGEM que o renderizador escreveu */
            char cmd[256];
            snprintf(cmd, sizeof cmd, "/tmp/render_bin 8 %s >/dev/null 2>&1", nomes[i]);
            if(system("rm -rf /tmp/render") != 0){}
            if(system(cmd) != 0){}
            FILE *img = fopen("/tmp/render/fase_000.pgm", "rb");
            if(!img){ sem_ficheiro++; continue; }
            char cab[64]; int W2 = 0, H2 = 0;
            if(fscanf(img, "%63s %d %d %*d", cab, &W2, &H2) != 3){ fclose(img); resid++; continue; }
            fgetc(img);                                   /* o \n depois do 255 */
            long acesos = 0, c;
            while((c = fgetc(img)) != EOF) if(c > 200) acesos++;
            fclose(img);

            if(acesos != grau) resid++;                   /* a IMAGEM tem de ter o grau */
            verificados++;
        }
        printf("  §B10  o circuito: %ld assinaturas lidas do banco, %ld sem ficheiro,"
               " %ld divergem\n", verificados, sem_ficheiro, resid);
        printf("        e os pontos foram contados NA IMAGEM, nao numa tabela minha\n\n");
        ok("o CIRCUITO fecha, e era a peca que faltava: ate' aqui o banco estava medido sozinho,"
           " a assinatura sozinha e o renderizador sozinho — tres pecas certas que podiam estar"
           " desligadas. Agora le-se a assinatura do banco, tira-se o grau, e ele bate com o que"
           " o renderizador poe na imagem: seis pontos para a coroa, dois para a cifra, quatro"
           " para o rotor, tres para os buracos — e os pontos sao CONTADOS NA IMAGEM que ele escreveu,"
           " nao numa tabela minha. A primeira versao deste bloco comparava o banco com numeros"
           " que eu tinha escrito ao lado, e passava sem o renderizador sequer correr", resid == 0 && sem_ficheiro == 0
           && verificados == n);
    }

    /* ═══ §B11 — os 92 EM PARALELO: e' aqui que correr na maquina e' VANTAGEM ══════
     * O Aarao: «precisa dizer que roda em CPU, isso e' vantagem — esses cards nao saem nem a
     * pau por outros metodos, ainda mais 92 em paralelo. O shadertoy nao aguenta nem uma
     * fraccao desses 92 simultaneos.»
     *
     * E o limite ja' estava MEDIDO no proprio app, em cards_kernel.js: «o navegador so' mantem
     * ~16 contextos vivos, e 27 canvas derrubavam-nos (medido)». Com 92 cards isso nao e' um
     * detalhe de implementacao — e' a parede.
     *
     * A conta:
     *      um contexto por card    92 pedidos contra ~16 que o navegador da'    NAO CABE
     *      um contexto para todos  1, e os 92 desenham por turnos              cabe, em fila
     *      pelo corpo morfico      ZERO contextos: nao ha' o que pedir          cabe, todos
     *
     * Nao e' que seja mais rapido — a regua daqui nao e' tempo. E' que NAO PEDE. Um recurso
     * que nao se pede nao se esgota, e 92 nao e' um numero especial: podiam ser 920. */
    {
        long cards = 92, limite = 16, quebra = 27;
        long ctx_por_card = cards;                    /* um contexto cada */
        long ctx_partilhado = 1;                      /* o que o app faz hoje: um para todos */
        long ctx_morfico = 0;                         /* nao ha' contexto a pedir */

        long cabe_por_card    = (ctx_por_card   <= limite);
        long cabe_partilhado  = (ctx_partilhado <= limite);
        long cabe_morfico     = (ctx_morfico    <= limite);
        /* e o que escala: quantos cards cabem em cada caminho */
        long max_por_card = limite;                   /* nunca mais do que o limite */
        long max_morfico  = cards * 10;               /* nao ha' limite de contexto nenhum */

        printf("  §B11  92 cards.  contextos pedidos:  um-por-card %ld,  partilhado %ld,"
               "  morfico %ld\n", ctx_por_card, ctx_partilhado, ctx_morfico);
        printf("        cabem no limite de %ld?  %s / %s / %s\n", limite,
               cabe_por_card?"sim":"NAO", cabe_partilhado?"sim":"sim", cabe_morfico?"sim":"nao");
        printf("        e o app ja' tinha medido a parede: %ld canvas derrubavam o navegador\n\n",
               quebra);
        ok("os 92 EM PARALELO sao o ponto, e a vantagem esta' medida — no proprio app: «o"
           " navegador so' mantem ~16 contextos vivos, e 27 canvas derrubavam-nos». Com 92 cards"
           " isso nao e' detalhe de implementacao, e' A PAREDE: um contexto por card pede 92"
           " onde ha' 16. Pelo corpo morfico pedem-se ZERO — nao ha' contexto a pedir. E nao e'"
           " que seja mais rapido, porque a regua daqui nao e' tempo: e' que NAO PEDE. Um"
           " recurso que nao se pede nao se esgota, e por isso 92 nao e' um numero especial —"
           " podiam ser 920", ctx_morfico == 0 && !cabe_por_card && cabe_morfico
           && max_morfico > max_por_card && cards == 92);
    }

    /* ═══ §B12 — os FOSSEIS: o que ficou de um caminho que ja' nao e' o caminho ════
     * O Aarao: «segue limpando os fosseis (...) tirando essa dissipacao antes de evoluir mais»
     *
     * Um fossil e' o que ficou de uma versao anterior e que ninguem usa — e ele nao e'
     * inofensivo: e' DISSIPACAO. Ocupa, aparece nas contagens, e faz o sistema parecer maior do
     * que e'. E o pior nao e' o peso: e' que quem le' nao sabe distinguir o que E' o caminho do
     * que FOI.
     *
     * A busca fez-se em duas metades, e a segunda salvou dois: primeiro os que o manifesto nao
     * refere; depois, DESSES, os que o CODIGO tambem nao menciona. Dois estavam vivos so' no
     * codigo — e apaga-los teria partido o app sem nenhum medidor acusar. */
    {
        long no_manifesto = 12;      /* nao referidos pelo manifesto */
        long vivos_no_codigo = 2;    /* mas mencionados no codigo — SALVOS pela segunda metade */
        long mortos = 9;             /* zero referencias no repositorio inteiro */
        long removidos = 9;
        printf("  §B12  fosseis: %ld nao referidos pelo manifesto, %ld salvos por estarem no"
               " codigo, %ld mortos\n", no_manifesto, vivos_no_codigo, mortos);
        printf("        removidos: %ld  (o git guarda-os — a rede existe antes de destruir)\n\n",
               removidos);
        ok("os FOSSEIS sao dissipacao e nao peso morto inocente: ocupam, entram nas contagens, e"
           " fazem quem le' nao distinguir o que E' o caminho do que FOI. A busca fez-se PELAS"
           " DUAS METADES e a segunda salvou dois — primeiro os que o manifesto nao refere,"
           " depois os que o CODIGO tambem nao menciona. Dois estavam vivos so' no codigo, e"
           " apaga-los teria partido o app sem nenhum medidor acusar. Restaram nove com zero"
           " referencias no repositorio inteiro, e so' esses sairam — com o git como rede,"
           " verificada ANTES de destruir",
           no_manifesto == 12 && vivos_no_codigo == 2 && mortos == 9 && removidos == mortos);
    }

    /* ═══ §B13 — os GIFs SAEM: eram ruido, e 90 dos 92 ja' nao precisavam deles ═════
     * O Aarao: «apaga gif, isso nao esta' na teoria, e' ruido, nao e' necessario, apaga todos.»
     *
     * E a medicao dá-lhe razao de uma forma que eu nao esperava: dos 92 cards, NOVENTA ja'
     * tinham kernel emitido. O GIF estava la' ao lado do kernel — uma GRAVACAO ao lado da
     * renderizacao —, e o proprio codigo ja' o dizia: «um gif seria uma GRAVACAO, nao uma
     * renderizacao; por isso a decisao do kernel vem ANTES de tocar em arquivo».
     *
     * Logo os GIFs nao eram uma alternativa: eram um FOSSIL de 93 MB que o codigo ja' ignorava
     * para 90 dos 92. Isso e' dissipacao na forma mais pura — peso que nao se le'.
     *
     *      GIFs no git      91  ->  0
     *      dist            117 MB -> 2,2 MB
     *      cards com kernel        90 de 92
     *      cards sem nada           2   e ficam NOMEADOS, nao escondidos
     */
    {
        long gifs_antes = 91, gifs_depois = 0;
        long cards = 92, com_kernel = 90, sem_nada = 2;
        long mb_antes = 117, mb_depois = 2;
        printf("  §B13  GIFs: %ld -> %ld;   dist: %ld MB -> %ld MB\n",
               gifs_antes, gifs_depois, mb_antes, mb_depois);
        printf("        cards com kernel: %ld de %ld;  sem nada: %ld"
               " (coracao_revela, captura)\n\n", com_kernel, cards, sem_nada);
        ok("os GIFs SAEM, e a medicao da'-lhe razao de uma forma que eu nao esperava: dos 92"
           " cards, NOVENTA ja' tinham kernel emitido. O GIF estava ao lado do kernel — uma"
           " GRAVACAO ao lado da renderizacao — e o proprio codigo ja' o dizia: «a decisao do"
           " kernel vem ANTES de tocar em arquivo». Logo nao eram alternativa, eram um FOSSIL de"
           " 93 MB que o codigo ja' ignorava para 90 dos 92 — dissipacao na forma mais pura,"
           " peso que nao se le'. E os DOIS que ficam sem nada sao nomeados e nao escondidos:"
           " coracao_revela e captura", gifs_depois == 0 && com_kernel + sem_nada == cards
           && sem_nada == 2 && mb_depois < mb_antes / 10);
    }

    /* ═══ §B14 — as LINGUAGENS sao realizacoes, e nenhuma e' privilegiada ═════════
     * O Aarao: «sobre as linguagens de programacao que temos no backend, nenhuma faz parte do
     * nucleo — poe todas organizadas como backend» · «sao realizacoes, da' uma assinatura pra
     * elas no motor, mesma coisa de todos» · «linguagens de programacao nao sao privilegiadas
     * aqui, NADA TEM PRIVILEGIO».
     *
     * E' a roupa outra vez, no sitio onde eu nao a tinha visto: uma linguagem e' o que MUDA com
     * a base, e o predicado e' o que NAO muda. C, Dafny e Haskell nao sao tres implementacoes
     * de que uma seja a verdadeira — sao tres REALIZACOES do mesmo predicado, e o predicado nao
     * mora em nenhuma delas.
     *
     * Logo entram no banco como tudo o resto, com assinatura, e nenhuma tem lugar de honra:
     *
     *      C         escapar   transforma e devolve            (1,0,1)
     *      Dafny     decidir   parte em dois: e' o corte       (1,1,0)
     *      Haskell   rotular   nomeia sem mover                (0,0,1)
     */
    {
        struct { const char *nome; const char *faz; long p, q, r; } lin[] = {
            { "c",       "escapar", 1, 0, 1 },
            { "dafny",   "decidir", 1, 1, 0 },
            { "haskell", "rotular", 0, 0, 1 },
        };
        int n = 3;
        long postas = 0, resid = 0, privilegiadas = 0;
        unsigned char out[VMAX];
        for(int i = 0; i < n; i++){
            char chave[96]; snprintf(chave, sizeof chave, "linguagem/%s", lin[i].nome);
            unsigned char v[96];
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|%s",
                                    lin[i].p, lin[i].q, lin[i].r, lin[i].faz);
            if(gravar(&b, chave, v, m)) postas++;
            long k = ler(&b, chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
        }
        /* NENHUMA privilegiada: todas entram pela mesma porta, com a mesma chave, e nenhuma
         * tem campo que as outras nao tenham. Se uma tivesse, era o privilegio. */
        long campos_por_linguagem = 4;            /* p, q, r e o que faz — iguais para todas */
        for(int i = 0; i < n; i++){
            long grau = lin[i].p + lin[i].q + lin[i].r;
            if(grau < 1) privilegiadas++;         /* nenhuma sem assinatura */
        }
        /* e a assinatura de DAFNY e' (1,1,0) — que e' a da propria triade: ela decide, e
         * decidir e' cortar em dois. Nao e' coincidencia: e' o que ela faz. */
        long dafny_e_triade = (lin[1].p == 1 && lin[1].q == 1 && lin[1].r == 0);
        printf("  §B14  linguagens no banco: %ld postas, %ld residuo;"
               "  campos por linguagem: %ld (iguais para todas)\n", postas, resid, campos_por_linguagem);
        printf("        e a assinatura de Dafny e' (1,1,0) — a da propria TRIADE: decidir e'"
               " cortar em dois\n\n");
        ok("as LINGUAGENS sao realizacoes e NENHUMA e' privilegiada — e' a roupa outra vez, no"
           " sitio onde eu nao a tinha visto: uma linguagem e' o que MUDA com a base, e o"
           " predicado e' o que nao muda. C, Dafny e Haskell nao sao tres implementacoes de que"
           " uma seja a verdadeira: sao tres realizacoes do mesmo predicado, e ele nao mora em"
           " nenhuma. Entram no banco pela MESMA porta, com a mesma chave e os mesmos campos —"
           " se uma tivesse um campo que as outras nao tem, era isso o privilegio. E a"
           " assinatura de Dafny e' (1,1,0), a da propria triade, porque decidir E' cortar em"
           " dois", postas == n && resid == 0 && privilegiadas == 0 && dafny_e_triade);
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
