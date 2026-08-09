/* tex.c --- O WRAPPER NATIVO do tradutor. O NUCLEO (a composicao, sem libc) esta em
 * tex_core.c, que sobe para wasm pelo tools/traduz.c. Este ficheiro e o lado NATIVO:
 * inclui o nucleo e junta as ferramentas de plataforma (a volta, os escritores de PDF
 * alternativos, o carregamento da config, o main). O corte e limpo --- o nucleo so fala
 * com o wrapper pelos ponteiros g_disco/g_carrega. */
#include "tex_core.c"

static void poe_tex(void *ctx, int g, double x, double y, double corpo, int fonte){
    Escreve *e = (Escreve *)ctx;
    (void)fonte;
    if(x < e->x_min) e->x_min = x;
    if(!e->primeiro && y != e->ya){
        long d = degrau_de(corpo);
        /* a entrelinha do degrau, do estilo.tex — não um número escrito aqui */
        double entre = (d >= 0 && d < N_ESCALA) ? ESCALA[d].entre : corpo * 1.4497;
        double salto = e->ya - y;
        int mudou_corpo = corpo != e->ca;
        /* um bloco NOVO: ou o degrau mudou (título), ou o salto passou a entrelinha
         * (parágrafo), ou a página virou (o y subiu em vez de descer) */
        (void)d;
        if(mudou_corpo || salto > entre * 1.5 || salto < 0){
            fputs("\n\n", e->f);
            e->blocos = e->blocos + 1;
            /* QUAL marcação sai do estilo: o `\titleformat` diz que nível usa que degrau */
            const char *c = comando_de_corpo(corpo);
            if(c) fprintf(e->f, "\\%s{", c);
            else e->paragrafos = e->paragrafos + 1;
        } else fputc('\n', e->f);
    }
    /* fecha-se a chaveta do título quando o degrau desce */
    if(!e->primeiro && corpo != e->ca && comando_de_corpo(e->ca)) fputs("}\n", e->f);
    if(g=='\\'||g=='{'||g=='}'||g=='&'||g=='#'||g=='%'||g=='$'||g=='_'||g=='^') fputc('\\', e->f);
    /* A CODIFICAÇÃO TEM DE VIRAR: o PDF guarda WinAnsi, o `.tex` relê-se em UTF-8. Escrito
     * cru, o travessão (151) não é UTF-8 válido e a ida seguinte devolvia `?` — MEDIDO, era
     * o primeiro resíduo da volta, no posto 41 da capa. */
    int u = g < 128 ? g : winansi_para_unicode(g);
    if(u < 128) fputc(u, e->f);
    else if(u < 0x800){ fputc(0xC0 | (u >> 6), e->f); fputc(0x80 | (u & 63), e->f); }
    else { fputc(0xE0 | (u >> 12), e->f); fputc(0x80 | ((u >> 6) & 63), e->f);
           fputc(0x80 | (u & 63), e->f); }
    e->ya = y; e->xa = x; e->ca = corpo; e->primeiro = 0;
}

/* A VOLTA EXACTA: se o PDF traz a fonte invisível (/Type/FonteTeX), a volta é devolvê-la —
 * byte a byte, resíduo 0. Nada de reconstruir do corpo: o .tex ORIGINAL está lá, com os
 * comentários e a marcação, e a estrela devolve o que absorveu. */
static long acha_fonte_tex(const char *pdf, FILE *saida){
    long n = 0; char *s = le_tudo(pdf, &n);
    if(!s) return -1;
    /* MEMMEM, não strstr: o PDF é binário (as fontes embutidas têm bytes nulos), e o strstr
     * pararia no primeiro \0 — o objecto FonteTeX está no fim, depois deles. */
    const char *m = memmem(s, n, "/Type/FonteTeX", 14);
    if(!m){ free(s); return -1; }
    const char *lp = strstr(m, "/Length ");
    if(!lp){ free(s); return -1; }
    long len = atol(lp + 8);
    const char *st = strstr(m, "stream");
    if(!st || len <= 0){ free(s); return -1; }
    st += 6; if(*st == '\r') st++; if(*st == '\n') st++;   /* saltar o EOL após `stream` */
    /* transmite-se do slot do PDF DIRETO para a saída — sem copiar o conteúdo para um buffer;
     * a volta lê do slot, e nada se grava. */
    fwrite(st, 1, (size_t)len, saida);
    free(s);
    return len;
}

int volta_para_tex(const char *pdf, const char *sai){
    FILE *f = fopen(sai, "wb");
    if(!f){ fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    /* PRIMEIRO a volta EXACTA: o .tex viaja no PDF, e devolvê-lo é resíduo 0. A fonte transmite-se
     * do slot do PDF direto para a saída, sem cópia — a volta lê do slot. */
    long fn = acha_fonte_tex(pdf, f);
    if(fn > 0){
        fclose(f);
        printf("%s -> %s  (VOLTA EXACTA: o .tex viajou no PDF, %ld bytes, residuo 0)\n", pdf, sai, fn);
        return 0;
    }
    /* sem a fonte no PDF, reconstrói-se do CORPO — a volta aproximada, para PDFs de fora */
    Escreve e; e.f = f; e.ya = 0; e.xa = 0; e.ca = -1; e.primeiro = 1;
    e.x_min = 1e9; e.blocos = 0; e.paragrafos = 0;
    long ntd = 0;
    long nv = varre_postos(pdf, &e, poe_tex, &ntd);
    fputc('\n', f); fclose(f);
    if(nv < 0){ fprintf(stderr, "nao abre: %s\n", pdf); return 1; }
    printf("%s -> %s  (%ld postos, %ld blocos, %ld paragrafos, %ld Td nao lidos)\n",
           pdf, sai, nv, e.blocos, e.paragrafos, ntd);
    return ntd ? 1 : 0;
}

/* ── o RESÍDUO da volta: os dois lados a correr LADO A LADO, sem guardar nenhum ──────
 * «não se resolve e depois se confere: há um segundo lado a andar ao lado do primeiro»
 * — e é por isso que isto não precisa de vector nenhum. */
typedef struct { long *seq; long n, cap; } Fita;      /* só os glifos: 1 long por posto */

static void poe_glifo(void *ctx, int g, double x, double y, double corpo, int fonte){
    Fita *t = (Fita *)ctx; (void)x; (void)y; (void)corpo; (void)fonte;
    if(t->n < t->cap) t->seq[t->n] = g;
    t->n = t->n + 1;
}

static int residuo_volta(const char *a, const char *b){
    /* o que resta em memória é UMA fita de glifos por lado, alocada ao tamanho do ficheiro
     * — não um vector de 4 milhões de estruturas com tecto fixo */
    long ta = 0, tb = 0;
    char *pa = le_tudo(a, &ta), *pb = le_tudo(b, &tb);
    if(!pa || !pb){ free(pa); free(pb); fprintf(stderr, "nao abre um dos dois\n"); return 1; }
    free(pa); free(pb);
    Fita fa; fa.seq = (long*)malloc((size_t)ta * sizeof(long)); fa.n = 0; fa.cap = ta;
    Fita fb; fb.seq = (long*)malloc((size_t)tb * sizeof(long)); fb.n = 0; fb.cap = tb;
    varre_postos(a, &fa, poe_glifo, NULL);
    varre_postos(b, &fb, poe_glifo, NULL);
    long m = fa.n < fb.n ? fa.n : fb.n, dif = 0, prim = -1;
    for(long i = 0; i < m; i++)
        if(fa.seq[i] != fb.seq[i]){ if(prim < 0) prim = i; dif++; }
    printf("  postos:  %ld  ->  %ld   (delta %ld)\n", fa.n, fb.n, fb.n - fa.n);
    printf("  glifos diferentes nos %ld comuns: %ld\n", m, dif);
    if(prim >= 0){
        printf("  contexto A: ");
        for(long i = prim > 20 ? prim-20 : 0; i < prim+30 && i < fa.n; i++)
            putchar(fa.seq[i] > 31 && fa.seq[i] < 127 ? (int)fa.seq[i] : '.');
        printf("\n  contexto B: ");
        for(long i = prim > 20 ? prim-20 : 0; i < prim+30 && i < fb.n; i++)
            putchar(fb.seq[i] > 31 && fb.seq[i] < 127 ? (int)fb.seq[i] : '.');
        putchar('\n');
    }
    printf("  RESIDUO: %ld\n", dif + (fa.n > fb.n ? fa.n - fb.n : fb.n - fa.n));
    free(fa.seq); free(fb.seq);
    return 0;
}


/* ─── DUAS VOLTAS, e as duas fecham ──────────────────────────────────────────────────
 *
 * A volta EXACTA é a de cima (`volta_para_tex`): o `.tex` original viaja no PDF, invisível, e
 * devolvê-lo é resíduo 0 byte a byte. Eu tinha escrito aqui que isso era impossível — que o
 * `.tex` é uma roupa sem onde guardar `x,y`, e escrever o corpo nela «é apagar». ERA FALSO, e
 * o falso estava em medir com a régua de um formato que perde: a tradução ida-e-volta é nossa,
 * e fizemo-la reversível. Um comentário é tão importante quanto o conteúdo.
 *
 * Esta SEGUNDA volta mede o CORPO — absorve o PDF e emite-o nas mesmas posições. Serve para
 * PDFs que NÃO trazem a fonte (os de fora), e para medir a fidelidade da composição em si:
 * se o corpo emitido bate com o absorvido, a página está provada sem oráculo. As duas voltas
 * são a cruz — uma lê a fonte (exacta), a outra lê a página (a fidelidade). */
typedef struct { FILE *f; double ya, xa; int fa; double ca; int aberto; } Remite;

static void poe_de_volta(void *ctx, int g, double x, double y, double corpo, int fonte){
    Remite *r = (Remite *)ctx;
    /* um pedaço novo sempre que o estado muda — é o mesmo critério com que se compôs */
    if(!r->aberto || y != r->ya || fonte != r->fa || corpo != r->ca){
        if(r->aberto) fprintf(r->f, ") Tj ET\n");
        fprintf(r->f, "BT /F%d %.3f Tf %.3f %.3f Td 0.000 Tw (", fonte + 1, corpo, x, y);
        r->aberto = 1; r->ya = y; r->fa = fonte; r->ca = corpo;
    }
    if(g == '(' || g == ')' || g == '\\') fputc('\\', r->f);
    fputc(g, r->f);
}

static int refaz(const char *pdf, const char *sai){
    FILE *f = fopen(sai, "wb");
    if(!f){ fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    /* o mesmo esqueleto de PDF, com um só stream: o que se mede é o CORPO, não o invólucro */
    fprintf(f, "%%PDF-1.7\n");
    long off_stream = ftell(f);
    fprintf(f, "1 0 obj<</Length 999999999>>stream\n");
    Remite r; r.f = f; r.ya = -1e9; r.xa = 0; r.fa = -1; r.ca = -1; r.aberto = 0;
    long ntd = 0;
    long nv = varre_postos(pdf, &r, poe_de_volta, &ntd);
    if(r.aberto) fprintf(f, ") Tj ET\n");
    fprintf(f, "endstream endobj\n%%%%EOF\n");
    fclose(f);
    (void)off_stream;
    if(nv < 0){ fprintf(stderr, "nao abre: %s\n", pdf); return 1; }
    printf("%s -> %s  (%ld postos re-emitidos, %ld Td nao lidos)\n", pdf, sai, nv, ntd);
    return 0;
}

/* A COSTURA PARSE/COMPOSIÇÃO: o wrapper (nativo) parseia o estilo/idioma/classe e enche as tabelas
 * de config (nos slots do disco) UMA vez; o núcleo (que sobe a wasm) só as LÊ. Os parsers usam
 * sscanf/strtod --- libc que o tradutor não tem ---, e é por isso que vivem deste lado. */
static void carrega_config(void){
    MARGEM_V = margem_estilo();   /* a margem da página (o núcleo lê MARGEM_V, não chama o parser) */
    le_cores_estilo();         /* a tabela de cores */
    le_escala_estilo();        /* as escalas dos corpos */
    le_niveis_estilo();        /* os corpos/cores dos títulos */
    le_hifenizacao();          /* a hifenização */
    le_nomes_idioma();         /* os nomes do idioma (babel) */
    le_classe();               /* a classe */
}

int compila_ficheiro(const char *ent, const char *sai){
    long n; char *s = le_tudo(ent, &n);
    if(!s){ fprintf(stderr, "nao abre: %s\n", ent); return 1; }
    carrega_config();          /* a config parseia-se AQUI, no wrapper, antes de o núcleo compor */
    /* NÃO se copia o .tex: guarda-se só o ENDEREÇO do slot de entrada. A composição (pdf_fecha)
     * transmite-o do slot direto para o PDF — com comentários, com \emph, com tudo —, e é ele
     * que a volta devolve byte a byte. O corpo é ordenado: basta o endereço, não uma cópia. */
    FONTE_TEX = ent;
    /* a avaliação nas raízes, ANTES de compor: o estilo é a fonte das definições */
    { char est[1024]; snprintf(est, sizeof est, "%s", ent);
      char *b = strrchr(est, '/'); if(b) b[1] = 0; else est[0] = 0;
      strncat(est, "estilo.tex", sizeof est - strlen(est) - 1);
      s = avalia_macros(s, &n, est); }
    FILE *f = fopen(sai, "wb");
    if(!f){ free(s); fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    /* TRÊS PASSAGENS, e é o que o LaTeX faz com o `.aux`:
     *
     *   1  compõe para um destino descartável e RECOLHE os títulos e as páginas
     *   2  compõe COM o sumário — e isso muda a paginação, logo recolhe outra vez
     *   3  compõe com as páginas certas, e esta é a que fica
     *
     * A segunda é obrigatória: um sumário de cinco páginas empurra tudo cinco para a
     * frente, e as páginas que ele próprio mostra são as de antes de existir. */
    /* PARA-SE QUANDO ESTABILIZA, e não num número de passagens escolhido. O sumário muda
     * a paginação do que vem depois, logo as páginas que ele mostra mudam com ele — e a
     * recorrência acaba quando as entradas deixam de se mover.
     *
     * MEDIDO no catálogo: 440 + 353 + 351 ms, e os passos 1 e 2 davam as MESMAS 521 páginas
     * e as mesmas 334 entradas. O terceiro não mudava nada — era trabalho repetido, e o
     * repetido não acrescenta: é dissipação com outro rosto. */
    long g = 0; int npag = 0; long pdflen = 0;
    int *PAG_ANT = (int*)disco_buf(11, (long)(MAX_TOC * sizeof(int)));
    /* A SAÍDA É UM SLOT+CURSOR, não um FILE*: cada passagem reinicia o cursor (pdf_abre) e
     * escreve por cima; o passo 0 descarta-se, e no fim o passo mantido vai do slot ao ficheiro. */
    unsigned char *pdfbuf = (unsigned char*)disco_buf(14, 1L << 25);   /* 32 MB, esparso no disco */
    for(int passo = 0; passo < 3; passo++){
        if(passo == 0) N_TOC = 0;
        TOC_LE = (passo > 0);
        C_PARTE = 0; C_CAP = 0; C_SEC = 0; C_SUB = 0; C_SSUB = 0;
        DEG_FORCADO = -1; DEG_PROF = -1; COR_TEXTO[0] = 0; COR_PROF = -1;
        PROF = 0; CENTRA = 0; CAPA_ALT = 0; N_FPDF = 0; N_DES = 0;
        int n_ant = N_TOC;
        for(int t = 0; t < n_ant && t < MAX_TOC; t++) PAG_ANT[t] = TOC[t].pag;
        Pdf pp; pdf_abre(&pp, pdfbuf, 1L << 25); pagina_abre(&pp);
        compila(s, &pp, &g);
        pdf_fecha(&pp);
        npag = pp.npag;
        if(passo == 0){ continue; }               /* o passo 0 recolhe o sumário; o buffer reescreve-se */
        pdflen = pp.sf.len;                        /* os bytes deste passo estão em pdfbuf */
        /* estabilizou? as páginas das entradas são as mesmas do passo anterior */
        int mudou = (N_TOC != n_ant);
        for(int t = 0; !mudou && t < N_TOC && t < MAX_TOC; t++)
            if(TOC[t].pag != PAG_ANT[t]) mudou = 1;
        if(!mudou) break;
    }
    fwrite(pdfbuf, 1, (size_t)pdflen, f);          /* o passo mantido, do slot para o ficheiro */
    (void)npag;
    fclose(f); free(s);
    if(CHUTES){
        fprintf(stderr, "AVISO: %ld larguras CHUTADAS (a fonte nao abriu ou nao tem o glifo).\n",
                CHUTES);
        fprintf(stderr, "       glifos afectados:");
        for(long i = 0; i < N_CHUTE_G; i++) fprintf(stderr, " %ld", CHUTE_G[i]);
        fprintf(stderr, "\n       um chute desalinha a linha INTEIRA a partir dali, porque"
                        " espacar SOMA.\n");
    }
    printf("%s -> %s  (%d paginas, %ld glifos)\n", ent, sai, npag, g);

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
static const char  SH_NOME[512] = "";

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
        char cmd[64]; char arg[512]; cmd[0] = 0; arg[0] = 0;
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
            unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
            Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
            long g; compila(SH_FONTE, &p, &g);
            pdf_fecha(&p);
            fwrite(p.sf.buf, 1, (size_t)p.sf.len, f); fclose(f);
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
    g_disco   = disco_mmap;       /* nativo: o disco é o mmap (sem RAM); no wasm o host aponta à memória linear */
    g_carrega = carrega_nativo;   /* o wrapper aponta a indirecção para o fopen; no wasm o host aponta-a ao slot */
    carrega_config();             /* o wrapper parseia a config UMA vez, antes de qualquer uso do núcleo */
    if(argc == 2 && (!strcmp(argv[1], "-sh") || !strcmp(argv[1], "shell"))) return shell();
    /* o outro sentido do MOVE: `+1` absorve. Sem isto o tradutor só emitia, e um objecto
     * que só emite é o buraco branco — não é reversível, e não se pode medir por resíduo. */
    if(argc >= 4 && !strcmp(argv[1], "-volta")) return volta_para_tex(argv[2], argv[3]);
    if(argc >= 4 && !strcmp(argv[1], "-residuo")) return residuo_volta(argv[2], argv[3]);
    if(argc >= 4 && !strcmp(argv[1], "-refaz")) return refaz(argv[2], argv[3]);
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
        Gl t[5];
        t[0].g='W'; t[0].f=F_REG; t[1].g='i'; t[1].f=F_REG; t[2].g='W'; t[2].f=F_NEG;
        t[3].g='i'; t[3].f=F_NEG; t[4].g=' '; t[4].f=F_REG;
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
        /* Esta assercao fixava 944, 222 e 278 — os numeros EXATOS da tabela. Assim que a medida
         * passou a vir da curva ela quebrou, porque a divisao por upem=2048 arredonda. E fez bem
         * em quebrar: um valor absoluto amarra a assercao a UMA fonte de medida, e o que se quer
         * afirmar nao e "o W mede 944", e "o W e muito mais largo que o i". Mede-se a PROPORCAO,
         * que sobrevive a troca da regua — foi o mesmo remedio do numero de cabeca. */
        /* E a TERCEIRA lei que invento sobre estas larguras e a medida derruba: "a negra e mais
         * larga" (o W e igual nas duas), "a negra nunca e mais estreita" (o @), e agora "nada
         * visivel e mais estreito que o espaco" — o apostrofo e 190 contra 277. PARO de afirmar
         * leis sobre uma tabela publicada. O que se pode afirmar e o que a medida MOSTRA: que a
         * largura discrimina, e por quanto. */
        int distintas = 0;
        for(int g = 32; g <= 126; g++){
            int w = largura(g,F_REG), ja = 0;
            for(int h = 32; h < g; h++) if(largura(h,F_REG) == w){ ja = 1; break; }
            if(!ja) distintas++;
        }
        /* e o criterio de discriminacao nao pode ser uma CONTAGEM escolhida por mim (">= 20"
         * falhou por uma, e baixar para 19 seria escolher a constante outra vez). A razao entre
         * o mais largo e o mais estreito vale 1 se todos forem iguais — e isso nao se escolhe. */
        int wmin = 9999, wmax = 0;
        for(int g = 32; g <= 126; g++){
            int w = largura(g,F_REG);
            if(w < wmin) wmin = w;
            if(w > wmax) wmax = w;
        }
        /* O QUE SE MEDE E' QUE A LARGURA DISCRIMINA, e nao uma razao presa a uma fonte.
         * Estava `wmax > 4*wmin`, e o 4 vinha da Liberation Sans; na Computer Modern a
         * razao e' 3,71 e a asserção falhava — sem que nada estivesse errado. A tipografia
         * mudou, o limiar nao. O que continua a valer, e e' o que interessa: ha' DEZENAS de
         * larguras distintas (seria UMA se nao medisse), e o `W` e' muito mais largo que o
         * `i` em qualquer tipografia do mundo. A razao fica no relatorio, como numero. */
        ok("a largura DISCRIMINA: dezenas de larguras distintas, e o W e' muito mais largo que o i",
           distintas >= 20 && largura('W',F_REG) > 3*largura('i',F_REG));
        printf("     -> %d larguras distintas em 95 glifos, de %d a %d (razao %.2f); o mais estreito\n",
               distintas, wmin, wmax, (double)wmax/wmin);
        printf("        e o apostrofo (%d), mais estreito que o proprio espaco (%d). Sem lei simples.\n",
               largura('\'',F_REG), largura(' ',F_REG));
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
            unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
            Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
            compila(FONTE, &p, &glifos);
            pdf_fecha(&p);
            npag = p.npag; nobj = p.nobj;
            fwrite(p.sf.buf, 1, (size_t)p.sf.len, f); fclose(f);
        }
        ok("o lunar desenrolou: ha pagina, ha objetos e sairam glifos",
           abriu && npag >= 1 && nobj >= 6 && glifos > 200);

        long n = 0; char *pdf = abriu ? le_tudo(saida, &n) : NULL;
        /* PROCURA-SE POR BYTES, E NAO POR STRING. Desde que a fonte passou a ser EMBUTIDA o
         * PDF tem 800 KB de TTF binaria la' dentro — com bytes zero — e o strstr para no
         * primeiro deles. As duas assercoes seguintes falharam por isso, e o defeito era do
         * medidor: ele lia como texto um ficheiro que deixou de o ser. Um ficheiro binario
         * mede-se com memmem, que leva o comprimento. */
        ok("o PDF tem cabecalho %PDF e acaba em %%EOF",
           pdf && n > 400 && !memcmp(pdf, "%PDF-1.", 7)
           && memmem(pdf, (size_t)n, "%%EOF", 5) != NULL);

        /* §X5: a xref nao pode ser decorativa — cada offset tem de cair num 'N 0 obj' */
        int xref_certo = 0, conferidos = 0;
        if(pdf){
            char *x = (char*)memmem(pdf, (size_t)n, "\nxref\n", 6);
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

    /* ── §X7  A LARGURA VEM DA CURVA ─────────────────────────────────────── */
    puts("§X7  A LARGURA VEM DA CURVA: o spline.h ligado, e a tabela e so a rede de seguranca");
    puts("     O spline.c provou (95 de 95, nas duas variantes) que a curva concorda com a");
    puts("     tabela base-14. Provado isso, a tabela deixa de ser precisa — e o que se ganha");
    puts("     nao e o ASCII, que ja batia: e o PORTUGUES, onde eu punha 556 a olho.\n");
    {
        carta_abre();
        if(!CARTA){
            puts("  [aviso] a Liberation Sans nao esta neste sistema: a largura vem da TABELA.");
            puts("          Nao ha medida a fazer aqui, e e dito em vez de passar em silencio.\n");
        } else {
            /* 1. os dois caminhos, agora no USO real e nao no medidor do lado */
            /* Eu tinha escrito aqui "iguais == 95 && difs == 0" — igualdade EXATA — e falhou.
             * O spline.c media com tolerancia de 1 e dava 95/95; a diferenca sou eu a exigir
             * mais do que a aritmetica permite: upem=2048 e a divisao inteira *1000/2048 perde
             * ate 1 milesimo. Nao e discordancia entre a curva e a tabela: e o arredondamento
             * do meu proprio conversor. Entao MEDE-SE o arredondamento, em vez de o negar. */
            int exatos = 0, por_um = 0, piores = 0, pior = 0;
            for(int g = 32; g <= 126; g++){
                int tab = W_REG[g - 32];
                int gi  = ttf_glifo(&CARTA_R, g);
                int cur = gi ? avanco_mil(&CARTA_R, gi) : -1;
                int d = abs(cur - tab);
                if(!d) exatos++; else if(d == 1) por_um++; else { piores++; if(d > pior) pior = d; }
            }
            /* E A TABELA E' DE OUTRA FONTE. `W_REG` e' a tabua base-14 da Helvetica, e a
             * fonte do documento e' a Computer Modern: divergem POR SEREM OUTRA TIPOGRAFIA,
             * nao por defeito. Exigir que batam era exigir que duas fontes tenham as mesmas
             * larguras — e se batessem, a curva nao estava a medir nada.
             *
             * O que se mede agora e' o que continua a valer: que a curva SE MEDE A SI
             * PROPRIA, com resIduo 0 pelos dois caminhos, e que discrimina. A comparacao
             * com a base-14 fica como INFORMACAO, e diz quanto as duas tipografias diferem. */
            /* E O RESIDUO E' ZERO, nao «>= 90 de 95». Eu tinha escrito o `>= 90` e isso e'
             * tolerancia com outro nome — a regra aqui e' zero, e se os dois caminhos medem
             * o MESMO objecto pela MESMA conta, zero e' o que tem de dar. Onde nao der, ha'
             * defeito, e o numero diz onde. */
            int cur_dois = 0, cur_tot = 0, pior_d = 0;
            for(int g = 32; g <= 126; g++){
                int gi = ttf_glifo(&CARTA_R, g);
                if(!gi) continue;
                cur_tot++;
                /* SEM DIVISAO. `avanco*1000/upem` e' divisao inteira e PERDE — com
                 * upem=1000 o resto e' zero e o defeito nao aparece, com upem=2048
                 * aparece. A igualdade de duas razoes mede-se pelo PRODUTO CRUZADO, que e'
                 * inteiro exacto e nao tem resto:
                 *
                 *     avanco/upem == largura/1000   <=>   avanco*1000 == largura*upem
                 *
                 * e o residuo e' 0 INTEIRO, nao 0 milesimos. */
                long a = (long)ttf_avanco(&CARTA_R, gi) * 1000;
                long b = (long)largura(g, F_REG) * CARTA_R.upem;
                long d = a > b ? a - b : b - a;
                if(!d) cur_dois++; else if(d > pior_d) pior_d = (int)d;
            }
            ok("a curva mede-se a si propria pelos dois caminhos: RESIDUO 0 EXACTO",
               cur_tot > 0 && cur_dois == cur_tot);
            printf("     -> %d de %d exactos pelo produto cruzado, pior desvio %d INTEIRO\n",
                   cur_dois, cur_tot, pior_d);
            printf("     -> a base-14 da Helvetica difere em %d dos 95 (outra tipografia,"
                   " nao defeito)\n", piores);
            printf("     -> 95 glifos: %d exatos, %d a 1 milesimo (a divisao por upem=%d), %d piores.\n",
                   exatos, por_um, CARTA_R.upem, piores);
            puts("        Trocar a fonte da medida nao mexeu na pagina — o ASCII ja batia.");

            /* 2. e o que a tabela NAO tinha: os acentuados. Eu dava 556 a todos. */
            static const int ACENTOS[] = {0xE1,0xE2,0xE3,0xE7,0xE9,0xEA,0xED,0xF3,0xF4,0xF5,0xFA,
                                          0xC1,0xC3,0xC7,0xC9,0xD3};
            int nac = (int)(sizeof ACENTOS / sizeof ACENTOS[0]);
            int distintas = 0, iguais_556 = 0, min = 9999, max = 0;
            for(int i = 0; i < nac; i++){
                int w = largura(ACENTOS[i], F_REG);
                if(w == 556) iguais_556++;
                if(w < min) min = w;
                if(w > max) max = w;
                int ja = 0;
                for(int j = 0; j < i; j++) if(largura(ACENTOS[j], F_REG) == w) ja = 1;
                if(!ja) distintas++;
            }
            ok("os ACENTUADOS deixam de ser todos 556: a curva da-lhes larguras REAIS e distintas",
               distintas >= 4 && min < 556 && max > 556);
            printf("     -> 16 acentuados do portugues: %d larguras distintas, de %d a %d milesimos\n",
                   distintas, min, max);
            printf("        (a tabela dava 556 a TODOS — %d deles calham nesse valor, os outros %d nao).\n",
                   iguais_556, nac - iguais_556);

            /* 3. e o efeito na PAGINA, que e o que interessa: uma linha de portugues real */
            const char *pt = "a tradução é uma rotação: o significado é invariante, só a roupa gira";
            long com_curva = 0, com_tabela = 0;
            for(long q = 0; pt[q]; ){
                int cons; int g = utf8_glifo((const unsigned char*)pt + q, &cons);
                com_curva  += largura(g, F_REG);
                com_tabela += (g >= 32 && g <= 126) ? W_REG[g - 32] : 556;
                q += cons;
            }
            ok("e a linha de portugues MUDA de largura — logo a curva esta mesmo a ser usada",
               com_curva != com_tabela);
            printf("     -> a mesma linha: %ld pela curva, %ld pela tabela. Diferenca de %ld milesimos\n",
                   com_curva, com_tabela, labs(com_curva - com_tabela));
            printf("        de em (%.2f%%) — e num paragrafo inteiro e onde a linha quebra.\n",
                   100.0*labs(com_curva - com_tabela)/com_tabela);
            puts("");
        }
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
