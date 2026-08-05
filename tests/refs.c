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
 *   §R1  toda \ref tem \label, documento a documento
 *   §R2  todo \label é único DENTRO do documento (duplicado é "multiply defined")
 *   §R3  nenhuma \ref atravessa documentos — no livro.tex junta-se tudo e passaria,
 *        mas cada .tex compila SOZINHO e lá sairia "??"
 *   §R4  o CONTROLO NEGATIVO: injeta-se uma órfã e uma duplicada em memória, e o
 *        medidor TEM de as apanhar. Sem isto §R1–§R3 passariam num repositório vazio.
 *
 *   cc -O2 -std=c99 -Wall refs.c -o refs && ./refs
 */
#include <stdio.h>
#include "../lib/disco.h"
#define refs_ DISCO_FIXO(Marca, 45)
#define labels DISCO_FIXO(Marca, 46)

#include "unidade.h"
#include <stdlib.h>
#include <string.h>

#define MAXN 4096
#define MAXL 256

typedef struct { char nome[MAXL]; int doc; long linha; } Marca;

 static int nlab = 0;
  static int nref = 0;

static const char *DOCS[] = { "teoria.tex", "catalogo.tex", "enredo.tex" };
static const int NDOC = 3;

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

/* o label existe? -1 se não; senão o índice do documento onde está */
static int onde(const char *nome)
{
    for (int i = 0; i < nlab; i++)
        if (!strcmp(labels[i].nome, nome)) return labels[i].doc;
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
        nlab = nref = 0; lidos = 0;
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
    int orfas = 0;
    for (int i = 0; i < nref; i++)
        if (onde(refs_[i].nome) < 0) {
            orfas++;
            printf("      ÓRFÃ  %s:%ld  \\ref{%s}\n",
                   DOCS[refs_[i].doc], refs_[i].linha, refs_[i].nome);
        }
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
    int cruzadas = 0;
    for (int i = 0; i < nref; i++) {
        int d = onde(refs_[i].nome);
        if (d >= 0 && d != refs_[i].doc) {
            cruzadas++;
            printf("      CRUZADA  %s:%ld  \\ref{%s} → vive em %s\n",
                   DOCS[refs_[i].doc], refs_[i].linha, refs_[i].nome, DOCS[d]);
        }
    }
    ok("nenhuma referência a label de outro documento", cruzadas == 0);

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

    nlab = nlab0; nref = nref0;   /* desfaz-se a injeção: o estado volta ao medido */

    printf("\n================================================================\n");
    conclui("um \\ref órfão não falha o pdflatex: emite Warning, imprime \"??\" e o PDF sai.");
    conclui("o portão mede páginas e passa. Este medidor é o que fecha esse buraco.");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas,
           falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
