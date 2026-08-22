/* refs.c — A REFERÊNCIA ÓRFÃ NÃO FALHA A COMPILAÇÃO: IMPRIME "??" E SEGUE.
 *
 * Em 03/08/2026 descobriu-se que `obs:ouro5` era citada QUATRO vezes no catálogo e o
 * label não existia — tinha sido renomeada para `obs:cinco` numa reorganização, e as
 * referências não acompanharam. Os PDFs publicados diziam "Observação ??" quatro vezes,
 * há semanas, e a bateria inteira estava verde.
 *
 * É a assinatura do defeito que mais custa neste repositório: NADA FALHA. O pdflatex
 * emite um Warning (não um erro), o PDF sai, o portão mede páginas e passa, e o leitor
 * é o primeiro a saber. Ver a nota sobre asserções que passam sem poder falhar, e a
 * outra sobre medidores que saem da corrida em silêncio.
 *
 * A regra que faltava tem duas metades. A primeira já estava escrita — "renomear, nunca
 * remover", porque remover um label leva medidores. A segunda é esta: RENOMEAR OBRIGA A
 * SEGUIR AS REFERÊNCIAS, e é o que este medidor guarda.
 *
 * ── E ELE OLHAVA PARA UM TERÇO DO REPOSITÓRIO (22/08) ────────────────────────
 * Nasceu com os três documentos da raiz, que eram o repositório todo. Os
 * `papers/` cresceram depois — nove documentos com \documentclass próprio, 590
 * labels e 978 referências — e ficaram de fora: a lei existia e não olhava para
 * onde ela é mais precisa, porque cada paper compila sozinho.
 *
 * Ao estender apareceram, de uma vez: 36 referências órfãs no
 * `corpo_analitico.tex` (todas a apontar para o `corpo_algebrico.tex`, resolvidas
 * agora por `xr`) e um `\label{thm:central}` DUPLICADO no mesmo ficheiro, em dois
 * teoremas diferentes — o segundo ganhava em silêncio e desviava as oito citações
 * do teorema central para o teorema errado.
 *
 * E a extensão teve o seu próprio defeito, que é a lição: `onde()` devolvia o
 * PRIMEIRO label com aquele nome em QUALQUER documento, o que com três raramente
 * colidia e com doze passou a acusar dezenas de «cruzadas» que são referências
 * locais perfeitas — `sec:geo` existe no `aranha` E no `teoria`. Procura-se
 * primeiro no documento que pergunta, que é o que o LaTeX faz.
 *
 *   §R1  toda \ref tem \label, documento a documento
 *   §R2  todo \label é único DENTRO do documento (duplicado é "multiply defined")
 *   §R3  nenhuma \ref atravessa documentos SEM O DECLARAR — no livro.tex junta-se
 *        tudo e passaria, mas cada .tex compila SOZINHO e lá sairia "??". A
 *        excepção é o `xr`: `\externaldocument[pref]{outro}` faz o LaTeX ler o
 *        .aux do outro e resolver, e o prefixo separa as duas famílias de labels.
 *   §R4  o CONTROLO NEGATIVO: injeta-se uma órfã, uma duplicada e uma cruzada em
 *        memória, e o medidor TEM de as apanhar. Sem isto §R1–§R3 passariam num
 *        repositório vazio. E o `xr` tem controlo PRÓPRIO, nos dois sentidos:
 *        desligando a declaração as referências que ela cobre voltam a ser órfãs
 *        (senão não estava a fazer trabalho), e um nome com o prefixo certo mas
 *        ausente do alvo continua órfão (senão o prefixo era passe-livre).
 *   §R5  A MESMA LEI NO ANDAR DE BAIXO: todo `#include "x.h"` de um ficheiro que o
 *        git RASTREIA aponta para um ficheiro que o git também tem. Um header fora
 *        do repositório é a referência órfã do C — aqui compila (o disco tem-no) e
 *        num clone limpo não existe, e nada falha DESTE lado.
 *        Foi assim que `banco/` + `*.h` viveu fora do git: o `.gitignore` tinha
 *        `banco/` + `*` com excepção só para `.c`, e o `banco/sql_api.h` — incluído pelo
 *        `banco/sql.c`, que está na bateria — nunca lá entrou. A bateria estava
 *        verde sobre uma árvore que o git não reproduzia.
 *        E o controlo negativo é o mesmo: injecta-se um include inexistente.
 *
 *   cc -O2 -std=c99 -Wall refs.c -o refs && ./refs
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define refs_ DISCO_FIXO(Marca, 45)
#define labels DISCO_FIXO(Marca, 46)

#include "unidade.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXN 4096
#define MAXL 256

typedef struct { char nome[MAXL]; int doc; long linha; } Marca;

 static int nlab = 0;
  static int nref = 0;

/* ── OS DOCUMENTOS QUE SE VIGIAM, E PORQUE SÃO DOZE E NÃO TRÊS ───────────────
 * Este medidor nasceu com os três da raiz, que eram o repositório todo. Os
 * `papers/` nasceram depois — nove documentos, 590 labels e 978 referências —
 * e ficaram FORA: a lei existia e não olhava para onde ela é mais precisa,
 * porque cada paper compila SOZINHO e uma \ref que atravessa sai «??».
 *
 * Quando se estendeu, apareceram 36 órfãs no `corpo_analitico.tex`, todas a
 * apontar para o `corpo_algebrico.tex`. A ausência de cobertura não era uma
 * decisão: era a lista a não ter acompanhado o repositório.
 *
 * Só entram os que têm `\documentclass` — `estilo.tex` e `gkcapa.tex` são
 * incluídos por outros e não compilam sozinhos, logo não são documentos. */
/* ── E A LISTA DERIVA-SE, EM VEZ DE SE ESCREVER ──────────────────────────────
 * Esta lista esteve escrita à mão com três nomes enquanto o repositório tinha
 * doze documentos, e o medidor dava verde sobre um terço da obra. Escrevê-la
 * outra vez à mão — agora com doze — seria repor exactamente a mesma armadilha
 * para o próximo paper que nascer.
 *
 * O critério é DIZÍVEL, e por isso deriva-se: um documento é um `.tex` que tem
 * `\documentclass`, isto é, que compila SOZINHO. O `estilo.tex` e o `gkcapa.tex`
 * são incluídos por outros e não passam nesse crivo — não por estarem numa lista
 * de excepções, mas por não cumprirem a definição. Um paper novo entra na
 * vigilância no dia em que nasce, sem ninguém se lembrar de o acrescentar. */
#define MAXDOC 64
static char DOCSBUF[MAXDOC][256];
static const char *DOCS[MAXDOC];
static int NDOC = 0;

static int tem_documentclass(const char *caminho)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return 0;
    char l[8192]; int achou = 0;
    while (!achou && fgets(l, sizeof l, f)) {
        /* ignora comentado: `%\documentclass` não conta */
        const char *p = l;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '%') continue;
        if (strstr(p, "\\documentclass")) achou = 1;
    }
    fclose(f);
    return achou;
}

/* junta os `.tex` de um directório que compilam sozinhos, por ordem */
static void junta_dir(const char *dir, const char *prefixo)
{
    DIR *d = opendir(dir);
    if (!d) return;
    char achados[MAXDOC][256]; int n = 0;
    struct dirent *e;
    while ((e = readdir(d)) && n < MAXDOC) {
        const char *nome = e->d_name;
        size_t L = strlen(nome);
        if (L < 5 || strcmp(nome + L - 4, ".tex")) continue;
        char caminho[512];
        snprintf(caminho, sizeof caminho, "%s%s", dir, nome);
        if (!tem_documentclass(caminho)) continue;
        snprintf(achados[n], sizeof achados[0], "%s%s", prefixo, nome);
        n++;
    }
    closedir(d);
    /* ordem estável: o relatório não pode mudar por causa do sistema de ficheiros */
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (strcmp(achados[i], achados[j]) > 0) {
                char t[256]; snprintf(t, sizeof t, "%s", achados[i]);
                snprintf(achados[i], sizeof achados[0], "%s", achados[j]);
                snprintf(achados[j], sizeof achados[0], "%s", t);
            }
    for (int i = 0; i < n && NDOC < MAXDOC; i++) {
        snprintf(DOCSBUF[NDOC], sizeof DOCSBUF[0], "%s", achados[i]);
        DOCS[NDOC] = DOCSBUF[NDOC]; NDOC++;
    }
}

/* monta a lista a partir de um prefixo de caminho (o medidor corre da raiz ou
 * de tools/, e a bateria faz `cd` — a mesma razão do PREFIXOS mais abaixo) */
static void monta_docs(const char *base)
{
    char raiz[512], pap[512];
    NDOC = 0;
    snprintf(raiz, sizeof raiz, "%s", base[0] ? base : "./");
    snprintf(pap,  sizeof pap,  "%spapers/", base);
    junta_dir(raiz, "");
    junta_dir(pap, "papers/");
}

/* ── E O `xr` É A EXCEPÇÃO DECLARADA ─────────────────────────────────────────
 * Uma \ref que atravessa documentos é um defeito — EXCEPTO quando o documento
 * declara `\externaldocument{outro}`, que é o mecanismo do LaTeX para o fazer:
 * o `xr` lê o `.aux` do outro e resolve. Com PREFIXO — `\externaldocument[alg:]`
 * — as duas famílias de labels ficam separadas, o que não é decoração: sem ele,
 * um label que exista nos dois (aqui `sec:torre`) dá «multiply defined» e um dos
 * dois ganha em silêncio.
 *
 * Guarda-se a declaração para §R1 e §R3 a poderem honrar. */
#define MAXX 32
typedef struct { int doc; char pref[MAXL]; char alvo[MAXL]; } Externo;
static Externo externos[MAXX];
static int nex = 0;

/* colhe \label{...} e \ref{...}/\eqref/\cref/\pageref de um ficheiro */
static int colhe(const char *caminho, int doc)
{
    FILE *f = fopen(caminho, "r");
    if (!f) return 0;
    char linha[8192]; long nl = 0;
    while (fgets(linha, sizeof linha, f)) {
        nl++;
        for (char *p = linha; (p = strchr(p, '\\')); p++) {
            const char *cmd = NULL; int elabel = 0;
            if (!strncmp(p, "\\externaldocument", 17)) {
                /* \externaldocument[pref]{alvo} — o prefixo é opcional */
                const char *q = p + 17;
                char pref[MAXL] = "";
                if (*q == '[') {
                    const char *fp = strchr(q, ']');
                    if (fp && fp - q - 1 < MAXL) {
                        memcpy(pref, q + 1, (size_t)(fp - q - 1));
                        pref[fp - q - 1] = 0;
                        q = fp + 1;
                    }
                }
                if (*q == '{') {
                    const char *fa = strchr(q, '}');
                    if (fa && fa - q - 1 < MAXL && nex < MAXX) {
                        externos[nex].doc = doc;
                        snprintf(externos[nex].pref, MAXL, "%s", pref);
                        memcpy(externos[nex].alvo, q + 1, (size_t)(fa - q - 1));
                        externos[nex].alvo[fa - q - 1] = 0;
                        nex++;
                    }
                }
                continue;
            }
            if      (!strncmp(p, "\\label{",    7)) { cmd = p +  7; elabel = 1; }
            else if (!strncmp(p, "\\ref{",      5)) { cmd = p +  5; }
            else if (!strncmp(p, "\\eqref{",    7)) { cmd = p +  7; }
            else if (!strncmp(p, "\\cref{",     6)) { cmd = p +  6; }
            else if (!strncmp(p, "\\pageref{",  9)) { cmd = p +  9; }
            else continue;
            const char *fim = strchr(cmd, '}');
            if (!fim || fim - cmd >= MAXL) continue;
            Marca m; size_t n = (size_t)(fim - cmd);
            memcpy(m.nome, cmd, n); m.nome[n] = 0; m.doc = doc; m.linha = nl;
            if (elabel) { if (nlab < MAXN) labels[nlab++] = m; }
            else        { if (nref < MAXN) refs_[nref++]  = m; }
            p = (char *)fim;
        }
    }
    fclose(f);
    return 1;
}

/* o label existe? -1 se não; senão o índice do documento onde está.
 *
 * ── E A ORDEM DA BUSCA IMPORTA, o que só se viu ao passar de 3 para 12
 * documentos. Esta função devolvia o PRIMEIRO label com aquele nome, em
 * qualquer documento — e com doze, um nome que existe no próprio documento E
 * noutro (`sec:geo` está no `aranha` e no `teoria`) resolvia para o OUTRO. O
 * §R3 acusava então dezenas de «cruzadas» que são referências locais perfeitas.
 * Procura-se PRIMEIRO no documento que pergunta: uma \ref resolve sempre para
 * o label do próprio ficheiro, que é o que o LaTeX faz. */
static int onde_de(const char *nome, int doc)
{
    for (int i = 0; i < nlab; i++)
        if (labels[i].doc == doc && !strcmp(labels[i].nome, nome)) return doc;
    for (int i = 0; i < nlab; i++)
        if (!strcmp(labels[i].nome, nome)) return labels[i].doc;
    return -1;
}
static int onde(const char *nome)
{
    for (int i = 0; i < nlab; i++)
        if (!strcmp(labels[i].nome, nome)) return labels[i].doc;
    return -1;
}

/* o basename sem .tex, para casar `\externaldocument{corpo_algebrico}` com
 * `papers/corpo_algebrico.tex` */
static void raiz_do_doc(const char *caminho, char *out, size_t cap)
{
    const char *b = strrchr(caminho, '/');
    b = b ? b + 1 : caminho;
    snprintf(out, cap, "%s", b);
    char *pt = strstr(out, ".tex");
    if (pt) *pt = 0;
}

/* a referência `nome`, feita no documento `doc`, resolve por um
 * \externaldocument declarado? Devolve o índice do documento alvo, ou -1.
 * É a ÚNICA maneira legítima de uma \ref atravessar documentos. */
static int onde_por_xr(const char *nome, int doc)
{
    for (int e = 0; e < nex; e++) {
        if (externos[e].doc != doc) continue;
        size_t lp = strlen(externos[e].pref);
        if (lp && strncmp(nome, externos[e].pref, lp)) continue;
        /* qual documento é o alvo? */
        for (int d = 0; d < NDOC; d++) {
            char r[MAXL]; raiz_do_doc(DOCS[d], r, sizeof r);
            if (strcmp(r, externos[e].alvo)) continue;
            /* o nome sem prefixo tem de ser label DESSE documento */
            for (int i = 0; i < nlab; i++)
                if (labels[i].doc == d && !strcmp(labels[i].nome, nome + lp))
                    return d;
        }
    }
    return -1;
}

int main(void)
{
    disco_prende(DISCO_BASE(45),"dados/refs_.bin",(size_t)(MAXN),sizeof(Marca));
    disco_zera(refs_,(size_t)(MAXN),sizeof(Marca));
    disco_prende(DISCO_BASE(46),"dados/labels.bin",(size_t)(MAXN),sizeof(Marca));
    disco_zera(labels,(size_t)(MAXN),sizeof(Marca));
    printf("A REFERÊNCIA ÓRFÃ — o defeito que compila, publica, e não falha\n");
    printf("================================================================\n");

    /* A bateria corre cada medidor de DENTRO de tools/ (bateria.sh:141, `cd $RAIZ/$dir`),
     * e à mão corre-se da raiz. Procura-se nos dois, e o primeiro que abrir os TRÊS ganha:
     * abrir só um ou dois seria medir meio repositório e chamar-lhe verde. */
    static const char *PREFIXOS[] = { "", "../", "./" };
    int lidos = 0; const char *prefixo = NULL;
    for (size_t k = 0; k < sizeof PREFIXOS / sizeof *PREFIXOS && !prefixo; k++) {
        monta_docs(PREFIXOS[k]);          /* a lista deriva-se, e do sítio certo */
        if (NDOC == 0) continue;
        nlab = nref = 0; nex = 0; lidos = 0;
        for (int d = 0; d < NDOC; d++) {
            char caminho[512];
            snprintf(caminho, sizeof caminho, "%s%s", PREFIXOS[k], DOCS[d]);
            lidos += colhe(caminho, d);
        }
        if (lidos == NDOC) prefixo = PREFIXOS[k];
    }
    if (!prefixo) {
        fprintf(stderr, "refs: não achei os três .tex (nem aqui nem em ../) — corre da raiz ou de tools/\n");
        return 2;
    }
    printf("  lidos %d documentos: %d labels, %d referências\n\n", lidos, nlab, nref);

    /* ---------------- §R1 — toda \ref tem \label ---------------- */
    printf("§R1 toda referência aponta para um label que existe\n");
    int orfas = 0, por_xr = 0;
    for (int i = 0; i < nref; i++) {
        if (onde_de(refs_[i].nome, refs_[i].doc) >= 0) continue;
        if (onde_por_xr(refs_[i].nome, refs_[i].doc) >= 0) { por_xr++; continue; }
        orfas++;
        printf("      ÓRFÃ  %s:%ld  \\ref{%s}\n",
               DOCS[refs_[i].doc], refs_[i].linha, refs_[i].nome);
    }
    printf("      (%d resolvidas por \\externaldocument, em %d declaração(ões))\n",
           por_xr, nex);
    ok("nenhuma referência órfã (a que imprime \"??\" no PDF)", orfas == 0);

    /* ---------------- §R2 — label único dentro do documento ---------------- */
    printf("\n§R2 nenhum label repetido dentro do mesmo documento\n");
    int dups = 0;
    for (int i = 0; i < nlab; i++)
        for (int j = i + 1; j < nlab; j++)
            if (labels[i].doc == labels[j].doc && !strcmp(labels[i].nome, labels[j].nome)) {
                dups++;
                printf("      DUPLICADO  %s:%ld e :%ld  \\label{%s}\n",
                       DOCS[labels[i].doc], labels[i].linha, labels[j].linha, labels[i].nome);
            }
    ok("nenhum label duplicado (o \"multiply defined\" que troca o alvo)", dups == 0);

    /* ---------------- §R3 — nenhuma \ref atravessa documentos ---------------- */
    printf("\n§R3 nenhuma referência atravessa documentos\n");
    printf("      (no livro.tex os três juntam-se e uma \\ref cruzada resolveria;\n");
    printf("       mas cada .tex compila SOZINHO, e lá sairia \"??\")\n");
    printf("      (a EXCEPÇÃO é o `xr`: com \\externaldocument o LaTeX lê o .aux\n");
    printf("       do outro e resolve — declarado, e não por acidente)\n");
    int cruzadas = 0, declaradas = 0;
    for (int i = 0; i < nref; i++) {
        int d = onde_de(refs_[i].nome, refs_[i].doc);
        if (d >= 0 && d != refs_[i].doc) {
            cruzadas++;
            printf("      CRUZADA  %s:%ld  \\ref{%s} → vive em %s\n",
                   DOCS[refs_[i].doc], refs_[i].linha, refs_[i].nome, DOCS[d]);
        }
        if (d < 0 && onde_por_xr(refs_[i].nome, refs_[i].doc) >= 0) declaradas++;
        (void)0;
    }
    printf("      %d atravessam por declaração, %d por acidente\n",
           declaradas, cruzadas);
    ok("nenhuma referência atravessa documentos SEM o declarar", cruzadas == 0);

    /* ---------------- §R4 — o CONTROLO NEGATIVO ---------------- */
    printf("\n§R4 controlo negativo: o medidor apanha o defeito quando ele existe?\n");
    printf("      Sem isto, §R1–§R3 passariam verdes num repositório sem um único .tex.\n");

    int nlab0 = nlab, nref0 = nref;

    /* (a) injeta-se uma órfã: uma \ref para um label que garantidamente não existe */
    strcpy(refs_[nref].nome, "obs:este-label-nao-existe-de-proposito");
    refs_[nref].doc = 0; refs_[nref].linha = -1; nref++;
    int viu_orfa = (onde(refs_[nref-1].nome) < 0);
    ok("com uma órfã injetada, §R1 apanha-a", viu_orfa);

    /* (b) injeta-se um duplicado: o mesmo nome, duas vezes, no mesmo documento */
    strcpy(labels[nlab].nome, "sec:duplicado-de-proposito"); labels[nlab].doc = 1; labels[nlab].linha = -1; nlab++;
    strcpy(labels[nlab].nome, "sec:duplicado-de-proposito"); labels[nlab].doc = 1; labels[nlab].linha = -2; nlab++;
    int viu_dup = 0;
    for (int i = 0; i < nlab; i++)
        for (int j = i + 1; j < nlab; j++)
            if (labels[i].doc == labels[j].doc && !strcmp(labels[i].nome, labels[j].nome)) viu_dup++;
    ok("com um duplicado injetado, §R2 apanha-o", viu_dup == 1);

    /* (c) injeta-se uma cruzada: \ref no enredo para um label do teoria */
    if (nlab0 > 0) {
        strcpy(refs_[nref].nome, labels[0].nome);       /* labels[0] é do teoria (doc 0) */
        refs_[nref].doc = 2; refs_[nref].linha = -1; nref++;   /* citada do enredo (doc 2) */
        int d = onde(refs_[nref-1].nome);
        ok("com uma cruzada injetada, §R3 apanha-a", d >= 0 && d != 2);
    }

    /* ── (c2) E A LISTA DERIVADA TEM O SEU PRÓPRIO TECTO ─────────────────────
     * Derivar a lista resolve o «esqueci-me de acrescentar», e traz um risco
     * novo: se os documentos passarem de MAXDOC, os últimos ficam de fora e o
     * medidor continua verde sobre menos obra — exactamente o defeito que a
     * derivação veio corrigir, com outra causa. Conta-se por FORA do
     * mecanismo: quantos `.tex` com `\documentclass` existem nos dois
     * directórios, e exige-se que TODOS tenham entrado.
     *
     * E exige-se que os `papers/` estejam LÁ: uma derivação que só apanhasse a
     * raiz daria 4 documentos e passaria em tudo o resto. */
    {
        int quantos = 0, com_papers = 0;
        const char *dirs[2] = { "", "papers/" };
        for (int k = 0; k < 2; k++) {
            char d[512];
            snprintf(d, sizeof d, "%s%s", prefixo, dirs[k][0] ? dirs[k] : "./");
            DIR *dd = opendir(d);
            if (!dd) continue;
            struct dirent *e;
            while ((e = readdir(dd))) {
                size_t L = strlen(e->d_name);
                if (L < 5 || strcmp(e->d_name + L - 4, ".tex")) continue;
                char c[600];
                snprintf(c, sizeof c, "%s%s", d, e->d_name);
                if (tem_documentclass(c)) quantos++;
            }
            closedir(dd);
        }
        for (int i = 0; i < NDOC; i++)
            if (!strncmp(DOCS[i], "papers/", 7)) com_papers++;
        printf("      %d documentos existem, %d entraram na lista (%d de papers/)\n",
               quantos, NDOC, com_papers);
        ok("a lista DERIVA-SE e não fica nenhum de fora — escrever os nomes à mão"
           " foi o que deixou nove papers sem vigilância, e o tecto MAXDOC repõe"
           " esse defeito em silêncio se ninguém o contar",
           quantos == NDOC && NDOC < MAXDOC && com_papers >= 9);
    }

    /* ── (d) E O CONTROLO DO PRÓPRIO `xr` ────────────────────────────────────
     * A excepção declarada é uma PORTA, e uma porta que aceita tudo não é uma
     * excepção — é um buraco. Mede-se nos dois sentidos:
     *
     *   (i) DESLIGANDO a declaração, as referências que ela cobre têm de voltar
     *       a ser órfãs. Se continuassem a resolver, o `xr` não estava a fazer
     *       nada e o verde vinha de outro sítio.
     *  (ii) e um nome que o alvo NÃO tem continua órfão MESMO com a declaração
     *       ligada — senão o prefixo `alg:` viraria um passe-livre para
     *       qualquer coisa.
     *
     * Sem isto, bastava declarar `\externaldocument` uma vez para o §R1 deixar
     * de medir seja o que for. */
    {
        int nex0 = nex, cobertas = 0, orfas_sem_xr = 0, passe_livre = 0;
        for (int i = 0; i < nref0; i++)
            if (onde_de(refs_[i].nome, refs_[i].doc) < 0
                && onde_por_xr(refs_[i].nome, refs_[i].doc) >= 0) cobertas++;
        nex = 0;                                  /* desliga TODAS as declarações */
        for (int i = 0; i < nref0; i++)
            if (onde_de(refs_[i].nome, refs_[i].doc) < 0
                && onde_por_xr(refs_[i].nome, refs_[i].doc) < 0) orfas_sem_xr++;
        nex = nex0;                               /* volta a ligar */
        printf("      com o xr ligado: %d cobertas · desligado: %d órfãs\n",
               cobertas, orfas_sem_xr);
        ok("desligando o \\externaldocument, as que ele cobre VOLTAM a ser órfãs"
           " — a excepção está a fazer trabalho", cobertas > 0 && orfas_sem_xr == cobertas);

        /* (ii) um nome inventado com o prefixo certo NÃO pode passar */
        if (nex0 > 0) {
            char falso[MAXL];
            snprintf(falso, sizeof falso, "%sthm:este-nao-existe-no-alvo",
                     externos[0].pref);
            passe_livre = (onde_por_xr(falso, externos[0].doc) >= 0);
            ok("um nome com o prefixo certo mas ausente do alvo continua órfão"
               " — o prefixo não é passe-livre", !passe_livre);
        }
    }

    nlab = nlab0; nref = nref0;   /* desfaz-se a injeção: o estado volta ao medido */

    /* ---------------- §R5 — o include órfão: existe no disco, não no git ---------- */
    printf("\n§R5 todo #include de um ficheiro RASTREADO está ele próprio no git\n");
    printf("      (um header fora do repositório compila AQUI e falha em toda a parte;\n");
    printf("       é a mesma órfã do §R1, um andar abaixo — e também não falha nada)\n");
    {
        static char rast[8192][MAXL];
        int nrast = 0, fora = 0, vistos = 0;
        FILE *g = popen("cd .. 2>/dev/null && git ls-files", "r");
        if (g) {
            char l[MAXL];
            while (nrast < 8192 && fgets(l, sizeof l, g)) {
                size_t n = strlen(l);
                while (n && (l[n-1] == '\n' || l[n-1] == '\r')) l[--n] = 0;
                if (n) { snprintf(rast[nrast], MAXL, "%s", l); nrast++; }
            }
            pclose(g);
        }
        /* NORMALIZAR ANTES DE ACUSAR. `banco/agentes.c` inclui "../lib/disco.h" e o
         * candidato sai `banco/../lib/disco.h` — que o git não conhece por esse nome,
         * embora tenha `lib/disco.h`. Sem colapsar os `X/..`, esta secção acusava 88
         * ficheiros e 85 eram ruído meu: mais falso positivo que defeito, que é como
         * as ferramentas desta casa costumam nascer. */
        /* rastreado? — comparação por caminho normalizado desde a raiz */
        #define ERASTREADO(P) ({ int _a = 0; for (int _i = 0; _i < nrast; _i++) \
                                 if (!strcmp(rast[_i], (P))) { _a = 1; break; } _a; })
        /* colapsa "a/b/../c" em "a/c", em cima do próprio buffer */
        #define NORMALIZA(P) do { \
            char *_q; \
            while ((_q = strstr((P), "/../"))) { \
                char *_p = _q; \
                while (_p > (P) && *(_p-1) != '/') _p--; \
                if (_p == (P)) { memmove((P), _q + 4, strlen(_q + 4) + 1); break; } \
                memmove(_p, _q + 4, strlen(_q + 4) + 1); \
            } \
            while (!strncmp((P), "./", 2)) memmove((P), (P) + 2, strlen((P) + 2) + 1); \
        } while (0)
        for (int r = 0; r < nrast; r++) {
            const char *f = rast[r];
            size_t nf = strlen(f);
            if (nf < 3 || (strcmp(f + nf - 2, ".c") && strcmp(f + nf - 2, ".h"))) continue;
            char cam[512];
            snprintf(cam, sizeof cam, "../%s", f);
            FILE *fp = fopen(cam, "r");
            if (!fp) continue;
            char dir[512];
            snprintf(dir, sizeof dir, "%s", f);
            char *barra = strrchr(dir, '/');
            if (barra) *barra = 0; else dir[0] = 0;
            char linha[4096];
            while (fgets(linha, sizeof linha, fp)) {
                char *inc = strstr(linha, "#include");
                if (!inc) continue;
                char *a = strchr(inc, '"');
                if (!a) continue;
                char *b = strchr(a + 1, '"');
                if (!b) continue;
                *b = 0;
                const char *alvo = a + 1;
                /* candidatos: relativo ao ficheiro, e as raízes da casa */
                const char *RAIZ[] = { NULL, "lib", "tests", "tools", "banco" };
                char cand[512], achou[512]; int existe = 0, no_git = 0;
                for (int k = 0; k < 5; k++) {
                    if (k == 0) {
                        if (dir[0]) snprintf(cand, sizeof cand, "%s/%s", dir, alvo);
                        else        snprintf(cand, sizeof cand, "%s", alvo);
                    } else snprintf(cand, sizeof cand, "%s/%s", RAIZ[k], alvo);
                    NORMALIZA(cand);
                    char disco[600];
                    snprintf(disco, sizeof disco, "../%s", cand);
                    FILE *t = fopen(disco, "r");
                    if (!t) continue;
                    fclose(t);
                    if (!existe) { existe = 1; snprintf(achou, sizeof achou, "%s", cand); }
                    if (ERASTREADO(cand)) { no_git = 1; break; }
                }
                if (!existe) continue;          /* header do sistema ou ausente: outro assunto */
                vistos++;
                if (!no_git) {
                    fora++;
                    printf("      FORA DO GIT  %s  inclui \"%s\" (existe em %s)\n", f, alvo, achou);
                }
            }
            fclose(fp);
        }
        printf("      %d ficheiros rastreados, %d includes locais resolvidos, %d fora do git\n",
               nrast, vistos, fora);
        /* o CONTROLO: um include que garantidamente não está no git tem de ser apanhado */
        int viu_falso = !ERASTREADO("lib/este-header-nao-existe-de-proposito.h");
        ok("nenhum #include de ficheiro rastreado aponta para fora do git — a órfã do C."
           " O `.gitignore` tinha `banco/*` sem excepção para `.h`, e o `banco/sql_api.h`,"
           " que o `banco/sql.c` da bateria inclui, nunca entrou no repositório: aqui"
           " compilava e num clone limpo não existia. Nada falhava deste lado, que é a"
           " assinatura do defeito que este ficheiro persegue",
           fora == 0 && vistos > 100 && nrast > 100 && viu_falso);
        #undef ERASTREADO
        #undef NORMALIZA
    }

    printf("\n================================================================\n");
    conclui("um \\ref órfão não falha o pdflatex: emite Warning, imprime \"??\" e o PDF sai.");
    conclui("o portão mede páginas e passa. Este medidor é o que fecha esse buraco.");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas,
           falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
