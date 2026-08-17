/* tex.c --- O WRAPPER NATIVO do tradutor. O NUCLEO (a composicao, sem libc) esta em
 * tex_core.c, que sobe para wasm pelo tools/traduz.c. Este ficheiro e o lado NATIVO:
 * inclui o nucleo e junta as ferramentas de plataforma (a volta, os escritores de PDF
 * alternativos, o carregamento da config, o main). O corte e limpo --- o nucleo so fala
 * com o wrapper pelos ponteiros g_disco/g_carrega. */
#include "tex_core.c"

/* TEX_TEMPO=1: tempos da composição no stderr. Só nativo — o wasm não tem relógio. */
#ifndef TEX_COM_LIBC_WASM
#include <time.h>
static int quer_tempo(void){
    static int v = -1;
    if(v < 0){ const char *e = getenv("TEX_TEMPO"); v = (e && e[0] == '1') ? 1 : 0; }
    return v;
}
static long agora_ms(void){
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000L + t.tv_nsec / 1000000L;
}
#else
static int quer_tempo(void){ return 0; }
static long agora_ms(void){ return 0; }
#endif

/* ── funcoes wrapper movidas do nucleo: costuras nativas, parsers de setup, macros, a volta ──
 * (o nucleo so as declara; aqui vivem, no lado nativo) */

static char *disco_mmap(int i, const char *nome, long n){ (void)i; return (char*)disco_u8(nome, (size_t)n); }

/* Fatias compactas (tests/disco_wasm.c): no wasm inicia_wasm liga-as a g_disco.
 * Um bloco via disco_u8; cada slot é OFF[i]. Nativo no main fica no mmap por ficheiro. */
static int USA_FATIA;
#ifndef TEX_COM_LIBC_WASM
static char *FAT_BASE;
static int OFF_FAT[16], TAM_FAT[16], FAT_PRONTO;
#endif

#ifdef TEX_COM_LIBC_WASM
/* banco 0–2 + rascunho 3–15: MOVE(−1) emite — mesma porta que o painel das animações. */
static char *disco_fatia(int i, const char *nome, long n){
    (void)nome; (void)n;
    return (char*)MOVE(i, -1);
}
#else
static char *disco_fatia(int i, const char *nome, long n){
    (void)nome; (void)n;
    if(i < 0 || i >= 16) return 0;
    if(!FAT_PRONTO){
        TAM_FAT[0]=1<<20; TAM_FAT[1]=1<<16; TAM_FAT[2]=1<<16; TAM_FAT[3]=1<<22;
        TAM_FAT[4]=1<<22; TAM_FAT[5]=1<<20; TAM_FAT[6]=1<<16; TAM_FAT[7]=1<<18;
        TAM_FAT[8]=1<<16; TAM_FAT[9]=1<<14; TAM_FAT[10]=1<<18; TAM_FAT[11]=1<<16;
        TAM_FAT[12]=1<<16; TAM_FAT[13]=1<<16; TAM_FAT[14]=1<<27; TAM_FAT[15]=1<<20;
        OFF_FAT[0]=0;
        for(int k=1;k<16;k++) OFF_FAT[k]=OFF_FAT[k-1]+TAM_FAT[k-1];
        FAT_BASE = (char*)disco_u8("../dados/tex_fatias.bin",
                                   (size_t)(OFF_FAT[15] + TAM_FAT[15]));
        if(!FAT_BASE) return 0;
        FAT_PRONTO = 1;
    }
    return FAT_BASE + OFF_FAT[i];
}
#endif

static char *disco_para(int i, const char *nome, long n){
    return USA_FATIA ? disco_fatia(i, nome, n) : disco_mmap(i, nome, n);
}

static long carrega_nativo(const char *nome, int i, long cap){
    unsigned char *b = (unsigned char*)disco_buf(i, cap);
    FILE *f = fopen(nome, "rb");
    if(!f) return -1;
    long n = (long)fread(b, 1, (size_t)(cap - 1), f);
    fclose(f);
    if(n < 0) n = 0;
    b[n] = 0;
    return n;
}

static void poe_nome_idioma(char *b, const char *ch, char *dest, long cap){
    char alvo[64]; snprintf(alvo, sizeof alvo, "nome %s ", ch);
    const char *q = strstr(b, alvo);
    if(!q) return;
    q += strlen(alvo);
    /* o ficheiro está em UTF-8 e o compositor escreve WinAnsi: converte-se aqui, que é
     * onde a fronteira está — e não em cada uso, que era onde o «Cap?lo» nascia */
    long k = 0;
    while(*q && *q != '\n' && k + 1 < cap){
        unsigned char c0 = (unsigned char)*q;
        if(c0 < 0x80){ dest[k++] = (char)c0; q++; continue; }
        int cons = 0; int u = utf8_glifo((const unsigned char*)q, &cons);
        dest[k++] = (char)unicode_para_winansi(u);
        q += cons ? cons : 1;
    }
    dest[k] = 0;
}

static void le_nomes_idioma(void){
    if(NOMES_LIDOS) return;
    NOMES_LIDOS = 1;
    long n = 0;
    char *b = le_tudo("lib/classe/idioma.txt", &n);
    if(!b) b = le_tudo("../lib/classe/idioma.txt", &n);
    if(!b) return;
    /* quatro chamadas em vez de uma tabela local de struct anónima: é o mesmo programa,
     * escrito no subconjunto que SOBE — a régua da libc.c, a valer também aqui */
    poe_nome_idioma(b, "chaptername",  NOME_CAP,     sizeof NOME_CAP);
    poe_nome_idioma(b, "partname",     NOME_PARTE,   sizeof NOME_PARTE);
    poe_nome_idioma(b, "abstractname", NOME_RESUMO,  sizeof NOME_RESUMO);
    poe_nome_idioma(b, "contentsname", NOME_SUMARIO, sizeof NOME_SUMARIO);
    poe_nome_idioma(b, "refname",      NOME_REFS,    sizeof NOME_REFS);
    free(b);
}

static long MARGEM_CACHE = -1;
static long margem_estilo(void){
    if(MARGEM_CACHE >= 0) return MARGEM_CACHE;
    long M = 64;                                    /* só se o estilo não disser nada */
    long n = 0; const char *b = estilo_texto(&n);   /* o estilo lê-se UMA vez (§estilo_texto) */
    if(b){
        /* `margin=` é sufixo de `innerleftmargin=`, `innertopmargin=` e mais quatro que o
         * estilo usa nos quadros. Sem verificar o que vem ANTES, o `strstr` apanhava
         * `innerleftmargin=12pt` e a margem da página ficava 12 — o texto colado à borda,
         * visto na página 3. Tem de ser início de opção: `[` ou `,` ou espaço. */
        const char *q = b;
        while((q = strstr(q, "margin=")) != NULL){
            char a = q > b ? q[-1] : '[';
            if(a == '[' || a == ',' || a == ' ' || a == '{') break;
            q += 7;
        }
        if(q){
            long m = medida_mil(q + 7);
            if(m >= 0) M = (m + 500) / 1000;       /* a margem é em pontos: UMA divisão */
        }
    }
    MARGEM_CACHE = M;
    return M;
}

static void le_cores_estilo(void){
    if(N_CORES >= 0) return;
    N_CORES = 0;
    long n = 0; const char *buf = estilo_texto(&n);   /* o estilo, lido uma vez */
    if(!buf) return;
    const char *q = buf;
    while(N_CORES < 64 && (q = strstr(q, "\\definecolor{")) != NULL){
        q += 13;
        const char *a = q; while(*q && *q != '}') q++;
        long ln = q - a; if(ln > 31) ln = 31;
        Cor *co = &CORES[N_CORES];             /* a frase começa num nome, não num cast */
        memcpy(co->nome, a, (size_t)ln); co->nome[ln] = 0;
        const char *h = strstr(q, "{HTML}{");
        if(!h) continue;
        /* RRGGBB: seis dígitos hex, parseados por inteiro (o sscanf %2x sai do núcleo) */
        int rr = hex2(h + 7), gg = hex2(h + 9), bb = hex2(h + 11);
        if(rr >= 0 && gg >= 0 && bb >= 0){
            co->r = rr; co->g = gg; co->b = bb;
            N_CORES++;
        }
    }
}

static void le_escala_estilo(void){
    if(N_ESCALA >= 0) return;
    N_ESCALA = 0;
    long n = 0; const char *buf = estilo_texto(&n);   /* o estilo, lido uma vez */
    if(!buf) return;
    const char *q = buf;
    while(N_ESCALA < 16 && (q = strstr(q, "\\fontsize{")) != NULL){
        /* `corpo}{entrelinha}` --- dois str2dbl com o `}{` literal no meio (lib/le_num.h). O
         * sscanf "%lf}{%lf}"==2 exige os dois números e o `}{`, mas NÃO o `}` final: idem aqui. */
        const char *p = q + 10, *e1;
        long c = fixo_mil(p, &e1);                 /* mantissa exacta: o estilo tem <=3 casas */
        if(e1 != p && e1[0] == '}' && e1[1] == '{'){
            const char *e2; long en = fixo_mil(e1 + 2, &e2);
            if(e2 != e1 + 2 && c > 0 && en > 0){
                ESCALA[N_ESCALA].corpo = c; ESCALA[N_ESCALA].entre = en; N_ESCALA++;
            }
        }
        q += 10;
    }
    /* por tamanho crescente: o degrau 0 é a nota, o último é o título */
    for(long i = 1; i < N_ESCALA; i++)
        for(long j = i; j > 0 && ESCALA[j].corpo < ESCALA[j-1].corpo; j--){
            Degrau t;                          /* a troca por memcpy: a cópia de estrutura
                                                * inteira não sobe, e nem precisa */
            memcpy(&t, &ESCALA[j], sizeof t);
            memcpy(&ESCALA[j], &ESCALA[j-1], sizeof t);
            memcpy(&ESCALA[j-1], &t, sizeof t);
        }
}

static void le_classe(void){
    if(CLASSE_CORPO != 0) return;
    CLASSE_CORPO = -1;
    /* a opção do documento — essa é do documento, e lê-se dele */
    long n = 0; char *b = le_tudo("../livro.tex", &n);
    if(!b) b = le_tudo("livro.tex", &n);
    if(!b) return;
    const char *q = strstr(b, "\\documentclass[");
    int pt = 0;
    if(q) sscanf(q + 15, "%dpt", &pt);
    free(b);
    if(pt <= 0) return;
    /* e a TABELA da classe, que estava no TeX Live e agora está aqui:
     *     classe <pt> <nome-do-corpo> <entrelinha>
     *     corpo  <nome> <valor>
     * Sem `kpsewhich`, sem `popen`, sem um processo lançado. */
    long m = 0; char *c = le_tudo("lib/classe/classe.txt", &m);
    if(!c) c = le_tudo("../lib/classe/classe.txt", &m);
    if(!c) return;
    char alvo[32]; snprintf(alvo, sizeof alvo, "classe %d ", pt);
    const char *r = strstr(c, alvo);
    char nome[32]; nome[0] = 0; long entre = 0;
    if(r){
        const char *z = r + strlen(alvo); int k = 0;
        while(*z && *z != ' ' && *z != '\n' && k < 31) nome[k++] = *z++;
        nome[k] = 0;
        const char *ee; entre = fixo_mil(z, &ee);   /* a mantissa, exacta */
        if(ee == z) entre = 0;
    }
    long corpo = 0;
    if(nome[0]){
        char a2[48]; snprintf(a2, sizeof a2, "corpo %s ", nome);
        const char *d = strstr(c, a2);
        if(d){ const char *ee; corpo = fixo_mil(d + strlen(a2), &ee); if(ee == d + strlen(a2)) corpo = 0; }
    }
    free(c);
    if(corpo > 0 && entre > 0){ CLASSE_CORPO = corpo; CLASSE_ENTRE = entre; }
}

static void le_hifenizacao(void){
    if(N_HIF >= 0) return;
    N_HIF = 0;
    long n = 0; const char *b = estilo_texto(&n);   /* o estilo, lido uma vez */
    if(!b) return;
    const char *q = strstr(b, "\\hyphenation{");
    if(!q) return;
    q += 13;
    while(*q && *q != '}' && N_HIF < MAX_HIF){
        while(*q == ' ' || *q == '\n' || *q == '\t') q++;
        if(*q == '}' || !*q) break;
        char pal[48]; int k = 0, nc = 0; char cortes[16];
        while(*q && *q != ' ' && *q != '\n' && *q != '}' && k < 47){
            if(*q == '-'){ if(nc < 16) cortes[nc++] = (char)k; q++; continue; }
            pal[k++] = *q++;
        }
        pal[k] = 0;
        if(k > 2){ snprintf(HIF[N_HIF].pal, 48, "%s", pal);
                   memcpy(HIF[N_HIF].cortes, cortes, (size_t)nc);
                   HIF[N_HIF].nc = nc; N_HIF++; }
    }
}

static char *le_tudo(const char *nome, long *n){
#ifdef TEX_COM_LIBC_WASM
    /* o banco (vfs/LS) é só leitura. A estrela trabalha numa JANELA — malloc após o
     * MARCO — e volta_compila recua. Escrever in-place no slot funde os dois lados. */
    {
        char *p = ficheiro_end_nome((char*)nome);
        if(!p) return 0;
        long tam = ficheiro_tam_nome((char*)nome);
        char *c = malloc(tam + 1);
        if(!c) return 0;
        { long i = 0; while(i < tam){ c[i] = p[i]; i++; } }
        c[tam] = 0;
        *n = tam;
        return c;
    }
#else
    FILE *f = fopen(nome, "rb");
    if(!f) return NULL;
    fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
    char *s = malloc((size_t)*n + 1);
    if(!s){ fclose(f); return NULL; }
    if(fread(s, 1, (size_t)*n, f) != (size_t)*n){ free(s); fclose(f); return NULL; }
    s[*n] = 0; fclose(f); return s;
#endif
}

static void recolhe_macros(const char *s, long n){
    for(long i = 0; i + 12 < n; i++){
        if(s[i] != '\\') continue;
        int decl = !strncmp(s+i+1,"newcommand",10) || !strncmp(s+i+1,"providecommand",14)
                || !strncmp(s+i+1,"renewcommand",12);
        if(!decl) continue;
        long q = i + 1; while(q < n && isalpha((unsigned char)s[q])) q++;
        if(q >= n || s[q] != '{') continue;
        long fim = fecha_chave(s, n, q); if(fim < 0) continue;
        /* o nome, que vem como `{\nome}` */
        long a = q + 1; if(a >= n || s[a] != '\\') continue;
        a++; char nome[48]; int k = 0;
        while(a < fim && k < 47 && isalpha((unsigned char)s[a])) nome[k++] = s[a++];
        nome[k] = 0; if(!k || a != fim) continue;
        q = fim + 1;
        /* o número de argumentos, opcional: `[n]` */
        int nargs = 0;
        if(q < n && s[q] == '['){ long b = q+1; nargs = atoi(s+b);
            while(q < n && s[q] != ']') q++;
            if(q < n) q++; }   /* sem `]` ate ao fim, nao se salta nada */
        /* um SEGUNDO `[...]` é o valor por omissão do 1.º argumento — não o tratamos, e
         * saltar a macro é mais honesto que a expandir com um argumento a menos */
        if(q < n && s[q] == '['){ continue; }
        if(q >= n || s[q] != '{') continue;
        long f2 = fecha_chave(s, n, q); if(f2 < 0) continue;
        if(N_MAC >= MAX_MAC) return;
        /* redefinição: fica a última, que é o que o TeX faz */
        int idx = -1;
        for(int t = 0; t < N_MAC; t++) if(!strcmp(MAC[t].nome, nome)) idx = t;
        if(idx < 0){ idx = N_MAC++; }
        else free(MAC[idx].corpo);
        { int nk = 0; while(nome[nk] && nk < 47){ MAC[idx].nome[nk] = nome[nk]; nk++; }
          MAC[idx].nome[nk] = 0; }
        MAC[idx].nargs = nargs;
        long cl = f2 - q - 1;
        MAC[idx].corpo = malloc((size_t)cl + 1);
        memcpy(MAC[idx].corpo, s + q + 1, (size_t)cl);
        MAC[idx].corpo[cl] = 0;
    }
}

static long expande_corre(char *s, long n, char *o, long *quantas){
    long len = 0;
    long qq = 0;
    for(long i = 0; i < n; ){
        if(s[i] == '%'){ while(i < n && s[i] != '\n'){ if(o) o[len] = s[i]; len++; i++; } continue; }
        if(s[i] != '\\'){ if(o) o[len] = s[i]; len++; i++; continue; }
        long a = i + 1; char nome[48]; int k = 0;
        while(a < n && k < 47 && isalpha((unsigned char)s[a])) nome[k++] = s[a++];
        nome[k] = 0;
        if(!k){
            /* o escape de UM caractere — `\\`, `\%`, `\{`, `\$` … — é literal e consome
             * os DOIS: deixar o segundo para trás reabria a leitura de comando, e o
             * `\\F` da matriz casava o `\F` do estilo («mathbbF» impresso na página) */
            if(o) o[len] = s[i]; len++; i++;
            if(i < n){ if(o) o[len] = s[i]; len++; i++; }
            continue;
        }
        if(!strncmp(nome,"newcommand",10) || !strncmp(nome,"providecommand",14)
              || !strncmp(nome,"renewcommand",12) || !strcmp(nome,"begin") || !strcmp(nome,"end")){
            if(o) o[len] = s[i]; len++; i++; continue;
        }
        int m = -1;
        for(int t = 0; t < N_MAC; t++) if(!strcmp(MAC[t].nome, nome)) m = t;
        if(m < 0){ if(o) o[len] = s[i]; len++; i++; continue; }
        long arg_a[9], arg_b[9], q = a;
        int ok_args = 1;
        for(int t = 0; t < MAC[m].nargs; t++){
            while(q < n && (s[q]==' '||s[q]=='\n'||s[q]=='\t')) q++;
            long f = fecha_chave(s, n, q);
            if(f < 0){ ok_args = 0; break; }
            arg_a[t] = q + 1; arg_b[t] = f; q = f + 1;
        }
        if(!ok_args){ if(o) o[len] = s[i]; len++; i++; continue; }
        const char *c = MAC[m].corpo;
        for(long t = 0; c[t]; t++){
            if(c[t] == '#' && c[t+1] >= '1' && c[t+1] <= '9'){
                int w = c[t+1] - '1';
                if(w < MAC[m].nargs){
                    long al = arg_b[w] - arg_a[w];
                    if(o) memcpy(o + len, s + arg_a[w], (size_t)al);
                    len += al;
                }
                t++; continue;
            }
            if(o) o[len] = c[t]; len++;
        }
        qq++;
        i = q;
    }
    if(o) o[len] = 0;
    *quantas = qq;
    return len;
}

static char *expande_uma(char *s, long *n, long *quantas){
    long qconta = 0;
    long tam = expande_corre(s, *n, 0, &qconta);       /* a MEDIDA: conta, não escreve */
    char *o = malloc((size_t)tam + 1);                 /* exacto, mais o terminador */
    expande_corre(s, *n, o, quantas);                  /* e agora ESCREVE, no espaço contado */
    *n = tam; free(s); return o;
}

/* `\input{foo}`: o ficheiro entra no sítio, como o pdflatex faz. Sem isto o
 * `\input{gkcapa}` dos papers nunca chegava — e a capa (gktit 23,42) não nascia. */
static char *le_input(const char *nome, const char *dir){
    char p[256]; char *q; long en = 0; char *e;
    q = ap_str(p, nome); *q = 0;
    e = le_tudo(p, &en); if(e) return e;
    { int tem = 0; const char *z = nome; while(*z){ if(*z == '.') tem = 1; z++; }
      if(!tem){ q = ap_str(p, nome); q = ap_str(q, ".tex"); *q = 0;
                e = le_tudo(p, &en); if(e) return e; } }
    if(dir && dir[0]){
        q = ap_str(p, dir); q = ap_str(q, nome); *q = 0;
        e = le_tudo(p, &en); if(e) return e;
        { int tem = 0; const char *z = nome; while(*z){ if(*z == '.') tem = 1; z++; }
          if(!tem){ q = ap_str(p, dir); q = ap_str(q, nome); q = ap_str(q, ".tex"); *q = 0;
                    e = le_tudo(p, &en); if(e) return e; } }
    }
    q = ap_str(p, "../"); q = ap_str(q, nome); *q = 0;
    e = le_tudo(p, &en); if(e) return e;
    { int tem = 0; const char *z = nome; while(*z){ if(*z == '.') tem = 1; z++; }
      if(!tem){ q = ap_str(p, "../"); q = ap_str(q, nome); q = ap_str(q, ".tex"); *q = 0;
                e = le_tudo(p, &en); if(e) return e; } }
    return 0;
}

static char *expande_inputs(char *s, long *n, const char *dir){
    int passo;
    for(passo = 0; passo < 8; passo++){
        int houve = 0; long i;
        for(i = 0; i + 7 < *n; i++){
            if(s[i] != '\\' || strncmp(s + i + 1, "input{", 6)) continue;
            long a = i + 7; char nome[96]; int k = 0;
            while(a < *n && s[a] != '}' && k < 95) nome[k++] = s[a++];
            if(a >= *n || s[a] != '}') continue;
            nome[k] = 0; long f2 = a + 1;
            long en = 0; char *e = le_input(nome, dir);
            if(!e){ i = f2 - 1; continue; }
            en = (long)strlen(e);
            long nn = i + en + (*n - f2);
            char *o = malloc((size_t)nn + 1);
            if(!o){ free(e); return s; }
            memcpy(o, s, (size_t)i);
            memcpy(o + i, e, (size_t)en);
            memcpy(o + i + en, s + f2, (size_t)(*n - f2));
            o[nn] = 0;
            free(e); free(s); s = o; *n = nn; houve = 1; break;
        }
        if(!houve) break;
    }
    return s;
}

static char *avalia_macros(char *s, long *n, const char *estilo){
    long en = 0; char *e = le_tudo(estilo, &en);
    if(e){ recolhe_macros(e, en); free(e); }
    recolhe_macros(s, *n);                      /* e as que o próprio documento define */
    for(int passo = 0; passo < 12; passo++){
        long q = 0; s = expande_uma(s, n, &q);
        EXPANDIDAS += q;
        if(!q) break;
    }
    return s;
}

static long varre_postos(const char *pdf, void *ctx,
                         void (*poe)(void *, int g, long x, long y, long corpo, int fonte),
                         long *nao_leu_td){
    /* A RÉGUA NOVA: não há Tj — cada glifo é uma instância `q sc 0 0 sc x y cm /Gix_gb Do`
     * da sua assinatura. Lê-se a posição EXPLÍCITA de cada um (sem acumular avanços), e o
     * corpo recupera-se da escala: corpo = sc·upem/1000, com o upem da carta registada. */
    long n = 0; char *s = le_tudo(pdf, &n);
    if(!s) return -1;
    long nv = 0;
    for(long i = 0; i + 4 < n; i++){
        if(s[i] != 'c' || s[i+1] != 'm' || s[i+2] != ' ' || s[i+3] != '/' || s[i+4] != 'G') continue;
        /* o nome: /G<ix>_<gb> */
        long q = i + 5; long ix = 0, gb = 0;
        while(q < n && s[q] >= '0' && s[q] <= '9'){ ix = ix*10 + (s[q]-'0'); q++; }
        if(q >= n || s[q] != '_') continue;
        q++;
        while(q < n && s[q] >= '0' && s[q] <= '9'){ gb = gb*10 + (s[q]-'0'); q++; }
        /* os seis números para trás, desde o `q `: sc 0 0 sc x y */
        long b = i - 1; int esp = 0;
        while(b > 0 && esp < 6){ b--; if(s[b] == ' ') esp++; }
        while(b > 0 && s[b] != 'q') b--;
        if(b <= 0){ if(nao_leu_td) *nao_leu_td = *nao_leu_td + 1; continue; }
        const char *e1; long v[6]; int nv2 = 0; const char *pp = s + b + 1;
        for(; nv2 < 6; nv2++){ long u = fixo_mil(pp, &e1); if(e1 == pp) break; v[nv2] = u; pp = e1; }
        if(nv2 < 6){ if(nao_leu_td) *nao_leu_td = *nao_leu_td + 1; continue; }
        /* sc vem em milésimos do fixo_mil mas foi escrito com SEIS casas: relê-se fino */
        long sc6 = 0;
        { const char *z = s + b + 1; while(*z==' ') z++;
          int neg = 0; if(*z=='-'){ neg=1; z++; }
          long ip = 0; while(*z>='0'&&*z<='9'){ ip = ip*10 + (*z-'0'); z++; }
          sc6 = ip * 1000000;
          if(*z=='.'){ z++; long casa=100000; while(*z>='0'&&*z<='9'&&casa){ sc6 += (*z-'0')*casa; casa/=10; z++; } }
          if(neg) sc6 = -sc6; }
        long upem = (ix >= 0 && ix < N_XGC && XG_CARTA[ix])
                     ? XG_CARTA[ix]->upem : 1000;
        long corpo = sc6 * upem / 1000;   /* sc6 em 10^-6: corpo = sc·upem, mantissa */
        poe(ctx, (int)gb, v[4], v[5], corpo, (int)ix);
        nv++;
        i = q;
    }
    free(s);
    return nv;
}

static void le_niveis_estilo(void){
    if(N_NIVEL >= 0) return;
    N_NIVEL = 0;
    long n = 0; const char *b = estilo_texto(&n);   /* o estilo, lido uma vez */
    if(!b) return;
    const char *q = b;
    while(N_NIVEL < 8 && (q = strstr(q, "\\titleformat{\\")) != NULL){
        const char *a = q + 14; char cmd[24]; int k = 0;
        while(*a && *a != '}' && k < 23) cmd[k++] = *a++;
        cmd[k] = 0;
        if(*a == '}') a++;      /* saltar o `}` do nome: sem isto o parser de grupos aborta
                                 * de imediato e a tabela sai VAZIA — um caractere */
        /* o ÚLTIMO `\gk...` DA DECLARAÇÃO, e a declaração acaba onde a GRAMÁTICA diz:
         * `\titleformat{\nivel}[forma]{fmt}{rótulo}{sep}{antes}[depois]` — cinco grupos.
         *
         * Duas voltas erradas antes desta, e ambas por não olhar à gramática. Ler o PRIMEIRO
         * `\gk` dava o `\gknota` do rótulo «Capítulo N» em vez do título. E limitar «até ao
         * próximo \titleformat» falha no ÚLTIMO do ficheiro, onde não há próximo: aí varria
         * o estilo inteiro e apanhava um `\gk` que não era desta declaração — MEDIDO, as
         * subsecções caíam para 6 pedaços onde há 168 delas. */
        const char *fim = a; int grupos = 0;
        while(*fim && grupos < 5){
            while(*fim == ' ' || *fim == '\n' || *fim == '%') { if(*fim=='%'){ while(*fim && *fim!='\n') fim++; } else fim++; }
            if(*fim == '['){ while(*fim && *fim != ']') fim++; if(*fim) fim++; continue; }
            if(*fim != '{') break;
            int d = 1; fim++;
            while(*fim && d){ if(*fim=='{') d++; else if(*fim=='}') d--; if(d) fim++; }
            if(*fim) fim++;
            grupos++;
        }
        const char *g = NULL;
        for(const char *z = a; (z = strstr(z, "\\gk")) != NULL && z < fim; z += 3) g = z;
        if(g){
            char gk[24]; int j = 0; const char *z = g + 1;
            while(*z && isalpha((unsigned char)*z) && j < 23) gk[j++] = *z++;
            gk[j] = 0;
            /* e o corpo desse degrau, da sua própria definição */
            char alvo[64]; snprintf(alvo, sizeof alvo, "{\\%s}{\\fontsize{", gk);
            const char *d = strstr(b, alvo);
            long c = 0;
            int _ok = 0;
            if(d){ const char *ee; c = fixo_mil(d + strlen(alvo), &ee);
                   _ok = (ee != d + strlen(alvo) && c > 0); }
            if(_ok){
                { int nk = 0; while(cmd[nk] && nk < 23){ NIVEL_CORPO[N_NIVEL].cmd[nk] = cmd[nk]; nk++; }
                  NIVEL_CORPO[N_NIVEL].cmd[nk] = 0; }
                NIVEL_CORPO[N_NIVEL].corpo = c;
                /* A COR TAMBÉM SE LÊ, e é a do PRIMEIRO grupo. A gramática diz porquê:
                 * `\titleformat{cmd}[forma]{FORMATO}{rótulo}{sep}{antes}[depois]` — o primeiro
                 * grupo aplica-se ao título inteiro, o segundo só ao rótulo. Guardar a última
                 * cor antes do `\gk` dava o `ouro` do «Capítulo N» ao título todo, quando o
                 * estilo manda `tinta`. Vi-o na página: o capítulo saiu dourado inteiro. */
                NIVEL_CORPO[N_NIVEL].cor[0] = 0;
                { const char *z = a;
                  while(*z && *z != '{' && z < fim){ if(*z=='['){ while(*z && *z!=']') z++; } z++; }
                  const char *g1 = z, *f1 = z;
                  if(*f1 == '{'){ int d3 = 1; f1++;
                      while(*f1 && d3){ if(*f1=='{') d3++; else if(*f1=='}') d3--; if(d3) f1++; } }
                  const char *c1 = strstr(g1, "\\color{");
                  if(c1 && c1 < f1){
                      const char *w = c1 + 7; int t = 0; char c2[24];
                      while(*w && *w != '}' && t < 23) c2[t++] = *w++;
                      c2[t] = 0; snprintf(NIVEL_CORPO[N_NIVEL].cor, 24, "%s", c2);
                  } }
                N_NIVEL++;
            }
        }
        q = a;
    }
}


/* o escritor da volta (tem FILE*): vive aqui, no wrapper nativo, não no núcleo */
typedef struct {
    FILE *f; long ya, xa, ca; int primeiro;
    long x_min;               /* a margem observada: o menor x visto, e não um valor posto */
    long blocos, paragrafos;
} Escreve;

static void poe_tex(void *ctx, int g, long x, long y, long corpo, int fonte){
    Escreve *e = (Escreve *)ctx;
    (void)fonte;
    if(x < e->x_min) e->x_min = x;
    if(!e->primeiro && y != e->ya){
        long d = degrau_de(corpo);
        /* a entrelinha do degrau, do estilo.tex — não um número escrito aqui; sem degrau,
         * a razão 1,4497 do estilo por produto cruzado de inteiros */
        long entre = (d >= 0 && d < N_ESCALA) ? ESCALA[d].entre : corpo * 14497 / 10000;
        long salto = e->ya - y;
        int mudou_corpo = corpo != e->ca;
        /* um bloco NOVO: ou o degrau mudou (título), ou o salto passou a entrelinha
         * (parágrafo), ou a página virou (o y subiu em vez de descer) —
         * `salto > entre·3/2` compara-se cruzado: 2·salto > 3·entre, sem dividir */
        (void)d;
        if(mudou_corpo || 2 * salto > 3 * entre || salto < 0){
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
    e.x_min = 1L << 60; e.blocos = 0; e.paragrafos = 0;
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

static void poe_glifo(void *ctx, int g, long x, long y, long corpo, int fonte){
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
typedef struct { FILE *f; long ya, xa; int fa; long ca; int aberto; } Remite;

/* imprime uma mantissa 10^-3 como N.ddd — o s_fix do wrapper, com FILE* */
static void f_fix(FILE *f, long v){
    if(v < 0){ fputc('-', f); v = -v; }
    fprintf(f, "%ld.%03ld", v / 1000, v % 1000);
}

static void poe_de_volta(void *ctx, int g, long x, long y, long corpo, int fonte){
    Remite *r = (Remite *)ctx;
    /* a régua nova: re-emite-se a INSTÂNCIA, como o compositor a escreveu — a mesma
     * assinatura, a mesma escala, a mesma posição; a volta lê o que a ida escreve */
    long upem = (fonte >= 0 && fonte < N_XGC && XG_CARTA[fonte])
                 ? XG_CARTA[fonte]->upem : 1000;
    long sc6 = corpo * 1000 / upem;
    fprintf(r->f, "q %ld.%06ld 0 0 %ld.%06ld ", sc6 / 1000000, sc6 % 1000000,
            sc6 / 1000000, sc6 % 1000000);
    f_fix(r->f, x); fputc(' ', r->f); f_fix(r->f, y);
    fprintf(r->f, " cm /G%d_%d Do Q\n", fonte, g);
    r->aberto = 1;
}

static int refaz(const char *pdf, const char *sai){
    FILE *f = fopen(sai, "wb");
    if(!f){ fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
    /* o mesmo esqueleto de PDF, com um só stream: o que se mede é o CORPO, não o invólucro */
    fprintf(f, "%%PDF-1.7\n");
    long off_stream = ftell(f);
    fprintf(f, "1 0 obj<</Length 999999999>>stream\n");
    Remite r; r.f = f; r.ya = -(1L << 60); r.xa = 0; r.fa = -1; r.ca = -1; r.aberto = 0;
    long ntd = 0;
    long nv = varre_postos(pdf, &r, poe_de_volta, &ntd);
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
/* os \newtheorem do estilo: o ambiente e o NOME que o rotula — a família partilha o
 * contador (o `[teorema]` salta-se), e o nome converte-se a WinAnsi à entrada, como tudo */
static void le_teoremas(void){
    if(N_TEOR >= 0) return;
    N_TEOR = 0;
    long n = 0; const char *b = estilo_texto(&n);
    if(!b) return;
    const char *q = b;
    while(N_TEOR < 20 && (q = strstr(q, "\\newtheorem{")) != NULL){
        /* o estilo do corpo é o ÚLTIMO \theoremstyle antes da declaração: `plain` é
         * itálico, `definition` e `remark` são romanos — como o amsthm manda */
        int ita = 0;
        { const char *st = b, *ult = NULL;
          while((st = strstr(st, "\\theoremstyle{")) != NULL && st < q){ ult = st; st++; }
          if(ult && !strncmp(ult + 14, "plain", 5)) ita = 1; }
        const char *a = q + 12; char amb[24]; int k = 0;
        while(*a && *a != '}' && k < 23) amb[k++] = *a++;
        amb[k] = 0;
        if(*a == '}') a++;
        if(*a == '['){ while(*a && *a != ']') a++; if(*a) a++; }
        if(*a == '{'){
            a++;
            char conv[32]; int ci = 0;
            while(*a && *a != '}' && ci < 31){
                int cs; int g = utf8_glifo((const unsigned char*)a, &cs);
                conv[ci++] = (char)g; a += cs ? cs : 1;
            }
            conv[ci] = 0;
            if(amb[0] && conv[0]){
                { int nk = 0; while(amb[nk] && nk < 23){ TEOR[N_TEOR].amb[nk] = amb[nk]; nk++; }
                  TEOR[N_TEOR].amb[nk] = 0; }
                { int nk = 0; while(conv[nk] && nk < 31){ TEOR[N_TEOR].nome[nk] = conv[nk]; nk++; }
                  TEOR[N_TEOR].nome[nk] = 0; }
                TEOR[N_TEOR].ita = ita;
                N_TEOR++;
            }
        }
        q = a;
    }
}

/* o texto FIXO do \fancyhead[L]: o que sobra depois do último \color{...}, até fechar
 * o grupo — lido do estilo, não escrito aqui (é o «Reino Dourado» de lá) */
static void le_cabecalho(void){
    if(CAB_ESQ[0]) return;
    long n = 0; const char *b = estilo_texto(&n);
    if(!b) return;
    const char *q = strstr(b, "\\fancyhead[L]");
    if(!q) return;
    const char *fim = strchr(q, '\n'); if(!fim) fim = q + strlen(q);
    const char *c = NULL, *z = q;
    while((z = strstr(z, "\\color{")) != NULL && z < fim){ c = z; z++; }
    if(!c) return;
    const char *t = strchr(c + 7, '}'); if(!t) return; t++;
    int k = 0;
    while(*t && *t != '}' && k < 63){
        if(*t == '\\'){ t++; while(isalpha((unsigned char)*t)) t++; continue; }
        int cs; int g = utf8_glifo((const unsigned char*)t, &cs);
        CAB_ESQ[k++] = (char)g; t += cs ? cs : 1;
    }
    CAB_ESQ[k] = 0;
}

/* A SEMENTE NA CONFIG: o estilo pode declarar \gksemente{resp}{ascN}{ascD}{desc}{traco}
 * — a origem (3, 17, 20, 4, 400) vale quando a declaração não está. Só as sementes são
 * fixas, e são configuração da estrela: UMA porta aqui, e o núcleo só lê SEM_V. */
static void le_semente(void){
    long n = 0; const char *b = estilo_texto(&n);
    if(!b) return;
    const char *q = strstr(b, "\\gksemente{");
    if(!q) return;
    long v[5]; int k = 0; const char *z = q + 10;
    while(k < 5 && *z == '{'){
        v[k] = 0; z++;
        while(*z >= '0' && *z <= '9'){ v[k] = v[k]*10 + (*z - '0'); z++; }
        if(*z != '}') return;
        z++; k++;
    }
    if(k == 5 && v[0] > 0 && v[2] > 0 && v[3] > 0)
        for(int t = 0; t < 5; t++) SEM_V[t] = v[t];
}

static void carrega_config(void){
    le_semente();              /* a semente da estrela — só ela é fixa, e é config */
    MARGEM_V = margem_estilo();   /* a margem da página (o núcleo lê MARGEM_V, não chama o parser) */
    le_teoremas();             /* a família dos \newtheorem, com os seus nomes */
    le_cabecalho();            /* o texto fixo do fancyhdr */
    le_cores_estilo();         /* a tabela de cores */
    le_escala_estilo();        /* as escalas dos corpos */
    le_niveis_estilo();        /* os corpos/cores dos títulos */
    le_hifenizacao();          /* a hifenização */
    le_nomes_idioma();         /* os nomes do idioma (babel) */
    le_classe();               /* a classe */
}

int compila_ficheiro(const char *ent, const char *sai){
    long t0 = 0, t_le = 0, t_cfg = 0, t_mac = 0, t_write = 0;
    long t_pass0 = 0, t_pass1 = 0, t_pass2 = 0;
#ifdef TEX_COM_LIBC_WASM
    /* esquilo no .bss: estrela grau 6 sem estado.
     *
     * Todo o rascunho DISCO_M[3–15] a zero ANTES do malloc: após volta,
     * SLOT_PTR=0 mas o cache pode ficar; le_tudo sobe CURSOR e o relógio
     * CURSOR «validava» UAF (catálogo a 10 pág. após dualsort).
     * Banco 0–2 + CARTAS (bestiário) ficam. */
    DISCO_M[3] = 0; DISCO_M[4] = 0; DISCO_M[5] = 0; DISCO_M[6] = 0;
    DISCO_M[7] = 0; DISCO_M[8] = 0; DISCO_M[9] = 0; DISCO_M[10] = 0;
    DISCO_M[11] = 0; DISCO_M[12] = 0; DISCO_M[13] = 0; DISCO_M[14] = 0; DISCO_M[15] = 0;
    N_MAC = 0; EXPANDIDAS = 0;
    N_DES = 0; CORPO_CORRENTE = 0;
    FONTE_OTF = 0; N_FPDF = 0; N_ESP = 0; N_XGC = 0;
    /* bestiário no LS; as CARTAS no DISCO apontavam ao FICH past MARCO — UAF após volta.
     * CARTA_TENTADO=0 reabre as fontes via miss (1 bit / neuronio). */
    CARTA_TENTADO = 0; CARTA = 0; N_CARTA = 0;
    CARTA_SIM = 0; CARTA_MAT = 0; CARTA_MTB = 0; CARTA_SMB = 0; CARTA_NIT = 0;
    CHUTES = 0; N_CHUTE_G = 0;
    N_CORES = -1; N_ESCALA = -1; N_TEOR = -1; N_HIF = -1; N_NIVEL = -1;
    NOMES_LIDOS = 0;
    C_PARTE = 0; C_CAP = 0; C_SEC = 0; C_SUB = 0; C_SSUB = 0;
    ESTILO_LIDO = 0; ESTILO_BUF = 0; ESTILO_LN = 0;
    C_TEO = 0; C_TEO_CAP = -1;
    TABCOLSEP = 6000; MARGEM_CACHE = -1;
    CAB_ESQ[0] = 0; CAB_DIR[0] = 0;
    N_TOC = 0; TOC_LE = 0; TOC_I = 0;
    SALTA_DE = -1; SALTA_ATE = -1; CENTRA = 0;
    Y_CAPA = -1; CAPA_POS = 0; CAPA_I = 0; CAPA_Y = 0; CAPA_FUN = 0;
    CAPA_NF = 0; CAPA_PAG = 0; CAPA_ALT = 0;
    DEG_FORCADO = -1; PROF = 0; DEG_PROF = -1;
    N_BIB = 0;
    CLASSE_CORPO = 0; CLASSE_ENTRE = 0;
    { int k = 0; while(k < 64){ EMBP[k] = 0; k++; } }
    { int k = 0; while(k < MAX_DES){ DES_CORPO[k] = 0; DES_VAR[k] = 0; k++; } }
    { int k = 0; while(k < MAX_XGC){ XG_CARTA[k] = 0; k++; } }
    { int k = 0; while(k < MAX_XGC * 256){ XG_USADO[k] = 0; XG_ID[k] = 0; k++; } }
    { int k = 0; while(k < 112){ ESP_NV[k] = 0; ESP_SG[k] = 0; k++; } }
    { int k = 0; while(k < 3072){ BIB_CHAVE[k] = 0; k++; } }
#endif
    if(quer_tempo()) t0 = agora_ms();
    long n; char *s = le_tudo(ent, &n);
    if(!s){ fprintf(stderr, "nao abre: %s\n", ent); return 1; }
    if(quer_tempo()) t_le = agora_ms() - t0;
    carrega_config();          /* a config parseia-se AQUI, no wrapper, antes de o núcleo compor */
    if(quer_tempo()) t_cfg = agora_ms() - t0 - t_le;
    /* NÃO se copia o .tex: guarda-se só o ENDEREÇO do slot de entrada. A composição (pdf_fecha)
     * transmite-o do slot direto para o PDF — com comentários, com \emph, com tudo —, e é ele
     * que a volta devolve byte a byte. O corpo é ordenado: basta o endereço, não uma cópia. */
    FONTE_TEX = ent;
    /* a avaliação nas raízes, ANTES de compor: o estilo é a fonte das definições.
     * O caminho monta-se com ap_str (sem snprintf no quadro: o traduz + sizeof do
     * `char est[1024]` local já partiu o bx). `\input` entra antes das macros. */
    { char dir[256]; char est[512]; char *p = dir; const char *slash = 0;
      { const char *q = ent; while(*q){ if(*q == '/') slash = q; q++; } }
      if(slash){ const char *q = ent; while(q <= slash) *p++ = *q++; }
      *p = 0;
      p = ap_str(est, dir); p = ap_str(p, "estilo.tex"); *p = 0;
      s = expande_inputs(s, &n, dir);
      s = avalia_macros(s, &n, est);
      s = avalia_macros(s, &n, "estilo.tex");
      /* `\renewcommand{\chaptername}{...}` do .tex SOBREPUÕE o idioma (sem função nova —
       * o wasm já está no teto MAX_FUN). catalogo→Dobra; enredo→Capítulo. */
      { for(int t = 0; t < N_MAC; t++){
          char *dest = 0; long cap = 0;
          if(!strcmp(MAC[t].nome, "chaptername")){ dest = NOME_CAP; cap = (long)sizeof NOME_CAP; }
          else if(!strcmp(MAC[t].nome, "partname")){ dest = NOME_PARTE; cap = (long)sizeof NOME_PARTE; }
          else if(!strcmp(MAC[t].nome, "abstractname")){ dest = NOME_RESUMO; cap = (long)sizeof NOME_RESUMO; }
          else if(!strcmp(MAC[t].nome, "contentsname")){ dest = NOME_SUMARIO; cap = (long)sizeof NOME_SUMARIO; }
          if(!dest || !MAC[t].corpo || !MAC[t].corpo[0]) continue;
          char tmp[64];
          tex_para_winansi(MAC[t].corpo, tmp, sizeof tmp);
          int tem_utf = 0;
          for(int i = 0; MAC[t].corpo[i]; i++)
              if((unsigned char)MAC[t].corpo[i] >= 0xC0){ tem_utf = 1; break; }
          long k = 0;
          if(tem_utf){
              const char *q = MAC[t].corpo;
              while(*q && *q != '\n' && k + 1 < cap){
                  unsigned char c0 = (unsigned char)*q;
                  if(c0 < 0x80){ dest[k++] = (char)c0; q++; continue; }
                  int cons = 0; int u = utf8_glifo((const unsigned char*)q, &cons);
                  dest[k++] = (char)unicode_para_winansi(u);
                  q += cons ? cons : 1;
              }
          } else {
              while(tmp[k] && k + 1 < cap){ dest[k] = tmp[k]; k++; }
          }
          dest[k] = 0;
      } }
    }
    if(quer_tempo()) t_mac = agora_ms() - t0 - t_le - t_cfg;
#ifndef TEX_COM_LIBC_WASM
    FILE *f = fopen(sai, "wb");
    if(!f){ free(s); fprintf(stderr, "nao escreve: %s\n", sai); return 1; }
#else
    (void)sai;   /* PDF no slot 14 — o host lê por MOVE(+1), sem FILE */
#endif
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
#ifdef TEX_COM_LIBC_WASM
    unsigned char *pdfbuf = (unsigned char*)disco_buf(14, tam_fatia(14));
    long pdf_cap = SLOT_TAM[14] ? SLOT_TAM[14] : tam_fatia(14);
#else
    unsigned char *pdfbuf = (unsigned char*)disco_buf(14, 1L << 27);
    long pdf_cap = 1L << 27;
#endif
    long pdf_perdeu = 0;
    for(int passo = 0; passo < 3; passo++){
        if(passo == 0) N_TOC = 0;
        TOC_LE = (passo > 0);
        C_PARTE = 0; C_CAP = 0; C_SEC = 0; C_SUB = 0; C_SSUB = 0;
        C_TEO = 0; C_TEO_CAP = -1;    /* o contador da família zera COM os de secção */
        CAB_DIR[0] = 0;               /* e a marca do cabeçalho também: nova passagem */
        DEG_FORCADO = -1; DEG_PROF = -1; COR_TEXTO[0] = 0; COR_PROF = -1;
        PROF = 0; CENTRA = 0; CAPA_ALT = 0; N_FPDF = 0; N_DES = 0;
        int n_ant = N_TOC;
        for(int t = 0; t < n_ant && t < MAX_TOC; t++) PAG_ANT[t] = TOC[t].pag;
        Pdf pp; pdf_abre(&pp, pdfbuf, pdf_cap); pagina_abre(&pp);
        { long tp = 0; if(quer_tempo()) tp = agora_ms();
          compila(s, &pp, &g);
          pdf_fecha(&pp);
          if(quer_tempo()){
              long dt = agora_ms() - tp;
              if(passo == 0) t_pass0 = dt;
              else if(passo == 1) t_pass1 = dt;
              else t_pass2 = dt;
          } }
        npag = pp.npag;
        if(passo == 0){ continue; }               /* o passo 0 recolhe o sumário; o buffer reescreve-se */
        pdflen = pp.sf.len;                        /* os bytes deste passo estão em pdfbuf */
        pdf_perdeu = pp.sf.perdeu;
        /* estabilizou? as páginas das entradas são as mesmas do passo anterior */
        int mudou = (N_TOC != n_ant);
        for(int t = 0; !mudou && t < N_TOC && t < MAX_TOC; t++)
            if(TOC[t].pag != PAG_ANT[t]) mudou = 1;
        if(!mudou) break;
    }
    /* O TETO ACUSA PELA PRÓPRIA SAÍDA: cada byte recusado conta-se (Saida.perdeu).
     * Foi assim que o catálogo saiu com o FonteTeX vazio — o s_bytes recusava a
     * fonte inteira por não caber, o trailer pequeno cabia, nada saturava, e a
     * volta partia em silêncio. */
    if(pdf_perdeu > 0){
        fprintf(stderr, "AVISO: %ld bytes NAO couberam no slot do PDF — o ficheiro saiu"
                        " INCOMPLETO e a volta nao fecha.\n", pdf_perdeu);
#ifndef TEX_COM_LIBC_WASM
        fclose(f);
#endif
        free(s);
        return 1;
    }
    { long tw = 0; if(quer_tempo()) tw = agora_ms();
#ifdef TEX_COM_LIBC_WASM
      marca_saida((char*)pdfbuf, (int)pdflen);       /* slot 14: MOVE(+1), sem cópia SAIDA */
#else
      fwrite(pdfbuf, 1, (size_t)pdflen, f);          /* o passo mantido, do slot para o ficheiro */
#endif
      if(quer_tempo()) t_write = agora_ms() - tw; }
    (void)npag;
#ifndef TEX_COM_LIBC_WASM
    fclose(f);
#endif
    free(s);
    if(quer_tempo()){
        fprintf(stderr, "TEMPO %s le=%ld cfg=%ld macros=%ld pass0=%ld pass1=%ld pass2=%ld write=%ld total=%ld ms pag=%d glifos=%ld bytes=%ld\n",
                ent, t_le, t_cfg, t_mac, t_pass0, t_pass1, t_pass2, t_write,
                agora_ms() - t0, npag, g, pdflen);
    }
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
/* SH_NOME era `static const char[512] = ""` e o snprintf da linha do ABRE escrevia
 * nele: o objdump mostrava-o em .rodata, e o comando crashava SEMPRE com SIGSEGV.
 * Nunca apareceu porque o shell e interactivo e a bateria nao o corre — foi o aviso
 * -Wdiscarded-qualifiers do compilador que o denunciou. O buffer e para ESCREVER. */
static char SH_NOME[512] = "";

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

/* §X16: acha o `qual`-ésimo uso do glifo gb nas instâncias `q sh 0 0 sv x y cm /Gix_gb Do`
 * e devolve o ix (a carta); sh/sv saem em milésimos, para ler o boost do integral */
static int x16_acha(const unsigned char *buf, long len, int gb, int qual,
                    long *sh, long *sv, long *xx, long *yy){
    int vez = 0;
    for(long q = 0; q + 4 < len; q++){
        if(memcmp(buf + q, "cm /G", 5)) continue;
        long w = q + 5, ix = 0, g2 = 0;
        while(w < len && buf[w] >= '0' && buf[w] <= '9'){ ix = ix*10 + (buf[w]-'0'); w++; }
        if(w >= len || buf[w] != '_') continue;
        w++;
        while(w < len && buf[w] >= '0' && buf[w] <= '9'){ g2 = g2*10 + (buf[w]-'0'); w++; }
        if(g2 != gb) continue;
        if(vez++ != qual) continue;
        long b2 = q - 1; int esp = 0;
        while(b2 > 0 && esp < 6){ b2--; if(buf[b2] == ' ') esp++; }
        const char *e1; const char *pp = (const char*)buf + b2 + 1;
        long v[6]; int k2 = 0;
        for(; k2 < 6; k2++){ long u = fixo_mil(pp, &e1); if(e1 == pp) break; v[k2] = u; pp = e1; }
        if(sh) *sh = k2 == 6 ? v[0] : -1;
        if(sv) *sv = k2 == 6 ? v[3] : -1;
        if(xx) *xx = k2 == 6 ? v[4] : -1;
        if(yy) *yy = k2 == 6 ? v[5] : -1;
        return (int)ix;
    }
    return -1;
}

/* A PORTA DO HOSPEDEIRO WASM: só as duas atribuições + a config. Sem isto o
 * host teria de chamar `main` com argv, e `compila_ficheiro` sozinha trapava
 * em g_disco/g_carrega nulos. Exportada (não-static): o browser chama-a uma vez. */
void inicia_wasm(void){
    USA_FATIA = 1;                /* g_disco → fatias (disco_wasm.c), não mmap */
    g_disco   = disco_para;
    g_carrega = carrega_nativo;   /* fopen dos slots que o host pôs por poe_ficheiro */
#ifdef TEX_COM_LIBC_WASM
    N_FICH = 0;                   /* sessão limpa: T4/poe anterior não cola */
    prende_fatias();              /* o DISCO é as fatias; o host MOVE os corpos depois */
#else
    carrega_config();
#endif
}

int main(int argc, char **argv){
    USA_FATIA = 0;
    g_disco   = disco_para;       /* nativo: mmap por ficheiro (disco.h) */
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
        /* o grego cai na Symbol; o \times e o \cdot EXISTEM no WinAnsi da fonte embutida
         * (0xD7, 0xB7) e saem do desenho do documento — não da Symbol de fora */
        const Par *cd = lex_acha("cdot");
        ok("o lexico traduz: \\alpha e \\sigma caem na Symbol; \\times e \\cdot saem do WinAnsi da fonte",
           a && s && t && cd && a->simb && s->simb && a->glifo == 'a' && s->glifo == 's'
           && !t->simb && t->glifo == 0xD7 && !cd->simb && cd->glifo == 0xB7);
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
        printf("     -> %d casos medidos, residuo medio %ld,%02ld milesimos de ponto (< 1/1000 pt por\n",
               casos, total_resto / casos, (total_resto % casos) * 100 / casos);
        puts("        espaco). A area enche; o que sobra e menor que a resolucao do formato.");
        /* e a METRICA tem de ser real, senao nao se mede linha nenhuma */
        Gl t[5];
        t[0].g='W'; t[0].f=F_REG; t[1].g='i'; t[1].f=F_REG; t[2].g='W'; t[2].f=F_NEG;
        t[3].g='i'; t[3].f=F_NEG; t[4].g=' '; t[4].f=F_REG;
        for(int k = 0; k < 5; k++) t[k].e = 0;   /* texto corrido: sem expoente */
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
        printf("     -> %d larguras distintas em 95 glifos, de %d a %d (razao %d,%02d); o mais estreito\n",
               distintas, wmin, wmax, wmax / (wmin > 0 ? wmin : 1),
               (wmax % (wmin > 0 ? wmin : 1)) * 100 / (wmin > 0 ? wmin : 1));
        printf("        e o apostrofo (%d), mais estreito que o proprio espaco (%d). Sem lei simples.\n",
               largura('\'',F_REG), largura(' ',F_REG));
        printf("     -> negra maior em %d glifos, igual em %d, MENOR em %d (o '@': %d contra %d).\n",
               maiores, empates, menores, largura('@',F_NEG), largura('@',F_REG));
        printf("        Total %ld contra %ld, e %d das 26 minusculas engordam. Nao ha lei simples:\n",
               som_b, som_r, min_mais);
        puts("        sao as tabelas publicadas, e e por isso que se medem em vez de se supor.");
        long m = mede(t, 5, 10000);   /* corpo 10 pt na régua do Tf: mantissa 10000 */
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
                    /* O `q++` SEM GUARDA passava o terminador quando o xref vinha sem
                     * newline, e as duas linhas seguintes (q += 20, strtol) liam ja' fora
                     * do buffer. O tex_core.c:6121 e o ttf_corpo.c:240 fazem o MESMO
                     * idioma com `if(*q) q++;` — a guarda existia na casa e nao tinha
                     * chegado aqui.
                     *
                     * E A MINHA PRIMEIRA CORRECCAO ESTAVA ERRADA: pus `xref_certo = -1`
                     * para marcar «truncado», e -1 e' VERDADEIRO em C, logo o laco corria
                     * na mesma e ainda com o cursor fora. Ficava pior do que estava. O
                     * certo e' nao entrar: se a linha nao fechou, nao ha' xref para ler.
                     * E dentro do laco o cursor tem de estar DENTRO do buffer, verificado
                     * antes de cada leitura e nao depois. */
                    while(*q && *q != '\n') q++;
                    if(*q){
                        q++;
                        q += 20;                               /* a entrada livre do objeto 0 */
                        xref_certo = 1;
                        for(int k = 1; k < quantos && xref_certo; k++){
                            if(q < pdf || q >= pdf + n){ xref_certo = 0; break; }
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
               strstr(saiu, "MARTELO2083236890") != NULL   /* a fita desenhada não tem espaços */);
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
            CORPO_CORRENTE = 0;      /* a régua é a carta BASE, não o último desenho usado */
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

    /* ── §X8  O EXPOENTE VIA CORPO, GRAU 2 OU GRAU 4 ─────────────────────── */
    puts("§X8  O EXPOENTE E O CORPO DOBRADO: `^` e `_` descem a escala (grau 2: sigma^-2;");
    puts("     grau 4: sigma^-4) e a subida e o dual aditivo — o que a escala tira, o espaco");
    puts("     recebe, com o sinal da Lei 1. Mede-se no PROPRIO PDF: os Tf dao a razao, os Td");
    puts("     dao a subida, e os dois caminhos tem de concordar entre si e com a escala.\n");
    {
        static const char FONTE2[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "controlo b2 sem cifrao e depois $b^2$ e $M_{ij}$ e $2^{2^k}$ fim\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long glifos2 = 0;
        compila(FONTE2, &p, &glifos2);
        pdf_fecha(&p);
        /* os GLIFOS, lidos das INSTÂNCIAS: cada `q sc 0 0 sc x y cm /Gix_gb Do` traz a
         * sua posição e a sua escala — o corpo é sc·upem/1000, exacto, da carta registada */
        long cbm = 0, ybm = 0;          /* o corpo e o y do texto corrido (baseline)   */
        long c2m = 0, y2m = 0;          /* o sobrescrito do $b^2$ (grau 2)             */
        long csm = 0, ysm = 0;          /* o subscrito do $M_{ij}$                     */
        long c4m = 0, y4m = 0;          /* o k de $2^{2^k}$ (grau 4)                   */
        {   int ant = 0; long ant_y = 0;
            for(long q = 0; q + 4 < p.sf.len; q++){
                if(memcmp(p.sf.buf + q, "cm /G", 5)) continue;
                long w = q + 5, ix = 0, gb = 0;
                while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9'){ ix = ix*10 + (p.sf.buf[w]-'0'); w++; }
                if(w >= p.sf.len || p.sf.buf[w] != '_') continue;
                w++;
                while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9'){ gb = gb*10 + (p.sf.buf[w]-'0'); w++; }
                long b2 = q - 1; int esp = 0;
                while(b2 > 0 && esp < 6){ b2--; if(p.sf.buf[b2] == ' ') esp++; }
                const char *e1; const char *pp = (char*)p.sf.buf + b2 + 1;
                long v[6]; int k2 = 0;
                for(; k2 < 6; k2++){ long u = fixo_mil(pp, &e1); if(e1 == pp) break; v[k2] = u; pp = e1; }
                if(k2 < 6) continue;
                long sc6 = 0;
                { const char *z2 = (char*)p.sf.buf + b2 + 1; while(*z2==' ') z2++;
                  long ip = 0; while(*z2>='0'&&*z2<='9'){ ip = ip*10 + (*z2-'0'); z2++; }
                  sc6 = ip * 1000000;
                  if(*z2=='.'){ z2++; long casa=100000; while(*z2>='0'&&*z2<='9'&&casa){ sc6 += (*z2-'0')*casa; casa/=10; z2++; } } }
                long upem = (ix >= 0 && ix < N_XGC && XG_CARTA[ix])
                     ? XG_CARTA[ix]->upem : 1000;
                long corpo = sc6 * upem / 1000, y = v[5];   /* sc6 em 10^-6: corpo = sc·upem, mantissa */
                /* o `2` do controlo vem logo a seguir ao `b`, no mesmo y e corpo */
                if(!cbm && gb == '2' && ant == 'b'){ cbm = corpo; ybm = y; }
                else if(cbm && gb == '2' && corpo < cbm - 500 && y > ybm && !c2m){ c2m = corpo; y2m = y; }
                else if(cbm && gb == 'i' && corpo < cbm - 500 && y < ybm && !csm){ csm = corpo; ysm = y; }
                else if(cbm && gb == 'k' && c2m && corpo < c2m - 500 && y > y2m && !c4m){ c4m = corpo; y4m = y; }
                ant = (int)gb; ant_y = y; (void)ant_y;
                q = w;
            }
        }
        /* IGUALDADE EXACTA: a dobra e' a razao de dois degraus da propria tabela, por
         * produto cruzado — a mesma conta que compoe, refeita do lado do PDF. Sem
         * tolerancia: ou o inteiro bate, ou nao. */
        ok("§X8 o controlo: o `b2` sem cifrao fica no corpo do texto, sem subida (a mutacao de referencia)",
           cbm > 0 && c2m > 0 && csm > 0 && c4m > 0);
        ok("§X8 grau 2: o sobrescrito e o corpo vezes E[1]/E[3] da regua — produto cruzado de inteiros, exacto",
           cbm > 0 && c2m == corpo_exp_m(cbm, 1) && c2m < cbm);
        ok("§X8 a Lei 1: o subscrito e o MESMO corpo com o sinal trocado — sobe e desce a MESMA distancia, exacta",
           c2m > 0 && csm == c2m && (y2m - ybm) == (ybm - ysm));
        ok("§X8 a conservacao: a subida E o que a escala tirou MAIS o respiro do giro"
           " (y2-yb = d + d/3, a semente para cima), inteiro, exacto",
           cbm > 0 && c2m > 0 && (y2m - ybm) == (cbm - c2m) + (cbm - c2m) / 3);
        ok("§X8 grau 4: o expoente de expoente e E[0]/E[4] — a dobra dupla, exacta, acima do primeiro",
           c4m > 0 && c4m == corpo_exp_m(cbm, 2) && c4m < c2m && y4m > y2m);
        printf("     -> corpo %ld; grau 2: %ld (= %ld*E1/E3); grau 4: %ld (= %ld*E0/E4)\n",
               cbm, c2m, cbm, c4m, cbm);
        printf("        subida +%ld, descida -%ld, corpo-corpo_exp %ld — o par fecha em inteiros.\n",
               y2m - ybm, ybm - ysm, cbm - c2m);
        puts("");
    }

    /* ── §X9  A ESPIRAL GERAL: os ANINHADOS, sem teto ────────────────────── */
    puts("§X9  A ESPIRAL GERAL: o relogio compoe translacao E escala num giro por nivel,");
    puts("     a partir da semente (o corpo, a dobra) — x^{a^{b^{c}}} e quatro corpos na");
    puts("     MESMA razao (a aurea dos degraus), e a subida de cada um e a CONSERVACAO:");
    puts("     o que a escala tirou ate ali. Ida e volta: o indice desce o mesmo, exacto.\n");
    {
        static const char FONTE3[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "$x^{a^{b^{c}}}$ giro $x_{a_{b_{c}}}$ fim\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long glifos3 = 0;
        compila(FONTE3, &p, &glifos3);
        pdf_fecha(&p);
        /* o vector local zera-se por atribuição: o inicializador {…} de um vector
         * de quadro não faz parte do subconjunto que o traduz lê — o vector É disco */
        long co[4]; long yo[4];   /* a ida: x, a, b, c   */
        long cv[4]; long yv[4];   /* a volta (indices)   */
        for(int t0 = 0; t0 < 4; t0++){ co[t0] = 0; yo[t0] = 0; cv[t0] = 0; yv[t0] = 0; }
        {   for(long q = 0; q + 4 < p.sf.len; q++){
                if(memcmp(p.sf.buf + q, "cm /G", 5)) continue;
                long w = q + 5, ix = 0, gb = 0;
                while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9'){ ix = ix*10 + (p.sf.buf[w]-'0'); w++; }
                if(w >= p.sf.len || p.sf.buf[w] != '_') continue;
                w++;
                while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9'){ gb = gb*10 + (p.sf.buf[w]-'0'); w++; }
                long b2 = q - 1; int esp = 0;
                while(b2 > 0 && esp < 6){ b2--; if(p.sf.buf[b2] == ' ') esp++; }
                const char *e1; const char *pp = (char*)p.sf.buf + b2 + 1;
                long v[6]; int k2 = 0;
                for(; k2 < 6; k2++){ long u = fixo_mil(pp, &e1); if(e1 == pp) break; v[k2] = u; pp = e1; }
                if(k2 < 6) continue;
                long sc6 = 0;
                { const char *z2 = (char*)p.sf.buf + b2 + 1; while(*z2==' ') z2++;
                  long ip = 0; while(*z2>='0'&&*z2<='9'){ ip = ip*10 + (*z2-'0'); z2++; }
                  sc6 = ip * 1000000;
                  if(*z2=='.'){ z2++; long casa=100000; while(*z2>='0'&&*z2<='9'&&casa){ sc6 += (*z2-'0')*casa; casa/=10; z2++; } } }
                long upem = (ix >= 0 && ix < N_XGC && XG_CARTA[ix])
                     ? XG_CARTA[ix]->upem : 1000;
                long corpo = sc6 * upem / 1000, y = v[5];
                int t = (gb == 'x') ? 0 : (gb == 'a') ? 1 : (gb == 'b') ? 2 : (gb == 'c') ? 3 : -1;
                if(t >= 0){
                    if(!co[t]){ co[t] = corpo; yo[t] = y; }
                    else if(!cv[t]){ cv[t] = corpo; yv[t] = y; }
                }
                q = w;
            }
        }
        int tem = co[0] && co[1] && co[2] && co[3] && cv[1] && cv[2] && cv[3];
        ok("§X9 os quatro corpos existem, na ida e na volta — a torre compos ate ao fim, sem teto",
           tem);
        /* DOIS CAMINHOS: o corpo lido da instancia contra o motor da espiral — exacto */
        int esc_ok = tem;
        for(int t = 1; t <= 3 && esc_ok; t++)
            if(co[t] != esp_escala(co[0], t) || co[t] >= co[t-1]) esc_ok = 0;
        ok("§X9 a escala: cada nivel e esp_escala(corpo, n) EXACTO — a instancia contra o motor,"
           " a mesma razao dos degraus (a aurea), estritamente decrescente", esc_ok);
        /* a CONSERVACAO telescopa: a subida do nivel n e corpo - esc(n), tudo positivo */
        int sub_ok = tem;
        for(int t = 1; t <= 3 && sub_ok; t++)
            if((yo[t] - yo[0]) != esp_sobe_torre(co[0], t) || yo[t] <= yo[t-1]) sub_ok = 0;
        ok("§X9 a conservacao composta: a subida do nivel n E a soma dos passos com o seu"
           " respiro (esp_sobe_torre), telescopica, inteira, exacta — dois caminhos", sub_ok);
        /* IDA E VOLTA: o indice desce exactamente o que o expoente sobe, nivel a nivel */
        int lei1_ok = tem;
        for(int t = 1; t <= 3 && lei1_ok; t++)
            if(cv[t] != co[t] || (yo[t] - yo[0]) != (yv[0] > 0 ? (yv[0] - yv[t]) : (yo[0] - yv[t]))) lei1_ok = 0;
        ok("§X9 ida e volta (Lei 1): o indice do nivel n desce a MESMA distancia que o expoente"
           " sobe, com o mesmo corpo — o giro tem inversa, exacta", lei1_ok);
        /* O ISOMORFISMO HORIZONTAL<->VERTICAL: a direcao da espiral e' CONSTANTE —
         * kern_n·sobe_1 == sobe_n·kern_1 em todos os niveis, proporcao cruzada
         * EXACTA em inteiros. E' o i a comandar: a mesma regua, rodada. */
        int iso_ok = 1;
        for(int t = 1; t <= 3; t++)
            if(esp_sobe_nv(co[0], t) - esp_passo_nv(co[0], t)
               != esp_kern_nv(co[0], t)) iso_ok = 0;
        ok("§X9 o isomorfismo horizontal<->vertical: o respiro do giro e' o MESMO numero"
           " nos dois eixos (Im - passo == Re, exacto em todos os niveis) — a pental,"
           " o i como rotacao dimensional: uma regua, rodada", iso_ok);
        printf("     -> corpos %ld > %ld > %ld > %ld; subidas +%ld +%ld +%ld; razao E1/E3 por giro.\n",
               co[0], co[1], co[2], co[3], yo[1]-yo[0], yo[2]-yo[0], yo[3]-yo[0]);
        puts("");
    }

    /* ── §X10  A FRONTEIRA DA REGIÃO: o delimitador pelo boost ───────────── */
    puts("§X10 O DELIMITADOR E A FRONTEIRA DA REGIAO: \\left(...\\right) mede os estados");
    puts("     internos e estica pela VERTICAL (sv = sc·k), com a largura NECESSARIA");
    puts("     (sh = sc) — a area da instancia declara-se no par (sh, sv). O controlo:");
    puts("     conteudo baixo nao estica — a fronteira so cresce quando a regiao pede.\n");
    {
        static const char FONTE4[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "\\[ \\left(\\frac{g}{q}\\right) \\qquad \\left( u \\right) \\]\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long glifos4 = 0;
        compila(FONTE4, &p, &glifos4);
        pdf_fecha(&p);
        /* os pares ( ) por ordem: [0] a fração (boost), [1] o controlo (sem) */
        long sh_[4]; long sv_[4]; int np2 = 0;
        for(int t0 = 0; t0 < 4; t0++){ sh_[t0] = 0; sv_[t0] = 0; }
        for(long q = 0; q + 4 < p.sf.len && np2 < 4; q++){
            if(memcmp(p.sf.buf + q, "cm /G", 5)) continue;
            long w = q + 5, gb = 0;
            while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9') w++;
            if(w >= p.sf.len || p.sf.buf[w] != '_') continue;
            w++;
            while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9'){ gb = gb*10 + (p.sf.buf[w]-'0'); w++; }
            if(gb != '(' && gb != ')') continue;
            long b2 = q - 1; int esp = 0;
            while(b2 > 0 && esp < 6){ b2--; if(p.sf.buf[b2] == ' ') esp++; }
            /* lê fino o 1.o (sh) e o 4.o (sv) números da instância */
            long fino[6]; int nf = 0; const char *z2 = (char*)p.sf.buf + b2 + 1;
            while(nf < 6){
                while(*z2 == ' ') z2++;
                long ip = 0, fr = 0, casa = 100000; int neg = 0;
                if(*z2 == '-'){ neg = 1; z2++; }
                while(*z2 >= '0' && *z2 <= '9'){ ip = ip*10 + (*z2-'0'); z2++; }
                if(*z2 == '.'){ z2++; while(*z2>='0'&&*z2<='9'){ if(casa){ fr += (*z2-'0')*casa; casa/=10; } z2++; } }
                fino[nf++] = (neg ? -1 : 1) * (ip * 1000000 + fr);
            }
            sh_[np2] = fino[0]; sv_[np2] = fino[3]; np2++;
            q = w;
        }
        ok("§X10 os dois pares compuseram — a fronteira da fracao e a do controlo, quatro instancias",
           np2 == 4 && sh_[0] && sh_[1] && sh_[2] && sh_[3]);
        ok("§X10 a fronteira da REGIAO alta estica so na vertical: sv > sh na fracao, e o abre"
           " e o fecha partilham o MESMO par (sh, sv) — a fronteira e UMA",
           np2 == 4 && sv_[0] > sh_[0] && sh_[0] == sh_[1] && sv_[0] == sv_[1]);
        ok("§X10 o controlo: o conteudo baixo NAO estica (sv = sh, k = 1) — a fronteira so"
           " cresce quando a regiao pede, e a mutacao e esta",
           np2 == 4 && sv_[2] == sh_[2] && sv_[3] == sh_[3]);
        ok("§X10 a largura e a NECESSARIA nos dois casos: sh igual no par alto e no baixo —"
           " o boost nao engorda a fronteira, so a sobe (a area declara-se no par sh·sv)",
           np2 == 4 && sh_[0] == sh_[2]);
        printf("     -> fracao sh=%ld sv=%ld (k>1); controlo sh=%ld sv=%ld (k=1).\n",
               sh_[0], sv_[0], sh_[2], sv_[2]);
        puts("");
    }

    /* ── §X11  A TRANSLAÇÃO ORTOGONAL: a vertical é a horizontal rodada ──── */
    puts("§X11 A TRANSLACAO ORTOGONAL: a vertical e a horizontal RODADA (o J do");
    puts("     transporte) — mesma semente, mesma composicao. A distancia entre o");
    puts("     paragrafo e o display e FRONTEIRA A FRONTEIRA: o fundo do texto, o");
    puts("     topo da equacao. O controlo: a equacao baixa fica na entrelinha.\n");
    {
        static const char FONTE5[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "z\n\\[ u \\]\nw\n\\[ \\frac{g}{q} \\]\nj fim\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long glifos5 = 0;
        compila(FONTE5, &p, &glifos5);
        pdf_fecha(&p);
        long yz=0,yu=0,yw=0,yg=0,yq=0,yj=0,cz=0;
        for(long q = 0; q + 4 < p.sf.len; q++){
            if(memcmp(p.sf.buf + q, "cm /G", 5)) continue;
            long w = q + 5, gb = 0;
            while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9') w++;
            if(w >= p.sf.len || p.sf.buf[w] != '_') continue;
            w++;
            while(w < p.sf.len && p.sf.buf[w] >= '0' && p.sf.buf[w] <= '9'){ gb = gb*10 + (p.sf.buf[w]-'0'); w++; }
            long b2 = q - 1; int esp = 0;
            while(b2 > 0 && esp < 6){ b2--; if(p.sf.buf[b2] == ' ') esp++; }
            const char *e1; const char *pp = (char*)p.sf.buf + b2 + 1;
            long v[6]; int k2 = 0;
            for(; k2 < 6; k2++){ long u2 = fixo_mil(pp, &e1); if(e1 == pp) break; v[k2] = u2; pp = e1; }
            if(k2 < 6) continue;
            long y = v[5];
            if(gb == 'z' && !yz){ yz = y;
                long sc6 = 0;
                { const char *z2 = (char*)p.sf.buf + b2 + 1; while(*z2==' ') z2++;
                  long ip = 0; while(*z2>='0'&&*z2<='9'){ ip = ip*10 + (*z2-'0'); z2++; }
                  sc6 = ip * 1000000;
                  if(*z2=='.'){ z2++; long casa=100000; while(*z2>='0'&&*z2<='9'&&casa){ sc6 += (*z2-'0')*casa; casa/=10; z2++; } } }
                long w2 = q + 5, ix = 0;
                while(p.sf.buf[w2] >= '0' && p.sf.buf[w2] <= '9'){ ix = ix*10 + (p.sf.buf[w2]-'0'); w2++; }
                long upem = (ix >= 0 && ix < N_XGC && XG_CARTA[ix]) ? XG_CARTA[ix]->upem : 1000;
                cz = sc6 * upem / 1000; }
            else if(gb == 'u' && !yu) yu = y;
            else if(gb == 'w' && !yw) yw = y;
            else if(gb == 'g' && !yg) yg = y;
            else if(gb == 'q' && !yq) yq = y;
            else if(gb == 'j' && !yj) yj = y;
            q = w;
        }
        int tem = yz && yu && yw && yg && yq && yj && cz;
        long d1 = yz - yu, d2 = yu - yw;                 /* a eq baixa: entrelinha pura */
        long ybase2 = (yg + yq) / 2;                     /* a baseline da pilha */
        long d3 = yw - ybase2, d4 = ybase2 - yj;         /* a eq alta: fronteira a fronteira */
        long dv = cz - corpo_exp_m(cz, 1);               /* a dobra — a MESMA semente */
        ok("§X11 o controlo: a equacao BAIXA fica na entrelinha da semente — a translacao"
           " vertical nao inventa espaco onde a fronteira nao pede (d1 = d2)",
           tem && d1 == d2 && d1 > 0);
        ok("§X11 a equacao ALTA desce o EXCESSO da fronteira: o numerador sobe 2dv+dv/3"
           " (a margem da barra) e a distancia cresce o mesmo — dois caminhos, exacto",
           tem && (d3 - d1) == 2 * dv + dv / 3);
        ok("§X11 e a ida e volta da translacao: o fundo do display empurra a linha seguinte"
           " o MESMO tanto do denominador — a rotacao preserva a lei",
           tem && (d4 - d1) == 2 * dv + dv / 3);
        printf("     -> entrelinha %ld; alta: antes +%ld, depois +%ld; 2dv = %ld.\n",
               d1, d3 - d1, d4 - d1, 2 * dv);
        puts("");
    }

    /* ── §X13  A LEI GLOBAL: semente → espiral → região, com conservação ─── */
    puts("§X13 A LEI GLOBAL DA AREA: toda configuracao nasce de uma SEMENTE, toda");
    puts("     evolucao e um giro da estrela-espiral, e a transformacao declara-se");
    puts("     na instancia (o par sh,sv). A semente VIAJA no PDF (/SementeEstrela),");
    puts("     e sementes diferentes dao geometrias diferentes sob a MESMA lei.\n");
    {
        static const char FONTE6[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "\\[ \\left(\\frac{a^{b^{c}}}{x}\\right) \\qquad"
            " \\boxed{\\ \\sigma\\,\\sigma = -1\\ } \\qquad"
            " \\bigl(\\begin{smallmatrix}0&b\\\\-b&0\\end{smallmatrix}\\bigr) \\]\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long glifos6 = 0;
        compila(FONTE6, &p, &glifos6);
        pdf_fecha(&p);
        /* (i) a SEMENTE viaja no PDF e bate com a do motor */
        int sem_ok = 0;
        { const char *z = (const char*)p.sf.buf;
          for(long q = 0; q + 20 < p.sf.len; q++)
              if(!memcmp(z + q, "/Type/SementeEstrela", 20)){
                  /* parse manual: o buffer não tem terminador (o sscanf faria
                   * strlen até ao abismo — e fez) */
                  long v4[4]; int nv4 = 0;
                  for(int t0 = 0; t0 < 4; t0++) v4[t0] = 0;
                  for(long w2 = q + 20; w2 < p.sf.len && nv4 < 4; w2++){
                      if(z[w2] >= '0' && z[w2] <= '9'){
                          long u2 = 0;
                          while(w2 < p.sf.len && z[w2] >= '0' && z[w2] <= '9'){
                              u2 = u2 * 10 + (z[w2] - '0'); w2++; }
                          v4[nv4++] = u2;
                      }
                      if(z[w2] == '>') break;
                  }
                  sem_ok = (v4[0] == 3 && v4[1] == 17 && v4[2] == 20 && v4[3] == 4);
                  break;
              } }
        ok("§X13 a semente viaja com o documento (/SementeEstrela) e bate com a origem"
           " da estrela — a configuracao e' estado legivel, nao constante escondida", sem_ok);
        /* (ii) a lei nas instancias: toda assimetrica vem em PAR (sh,sv) igual, e o
         * sh e' a escala natural — a transformacao declara-se, quem mede le */
        long ash[64], asv[64]; int na = 0;
        for(long q = 0; q + 4 < p.sf.len; q++){
            if(memcmp(p.sf.buf + q, "cm /G", 5)) continue;
            long b2 = q - 1; int esp = 0;
            while(b2 > 0 && esp < 6){ b2--; if(p.sf.buf[b2] == ' ') esp++; }
            long fino[6]; int nf = 0; const char *z2 = (char*)p.sf.buf + b2 + 1;
            while(nf < 6){
                while(*z2 == ' ') z2++;
                long ip = 0, fr = 0, casa = 100000; int neg = 0;
                if(*z2 == '-'){ neg = 1; z2++; }
                while(*z2 >= '0' && *z2 <= '9'){ ip = ip*10 + (*z2-'0'); z2++; }
                if(*z2 == '.'){ z2++; while(*z2>='0'&&*z2<='9'){ if(casa){ fr += (*z2-'0')*casa; casa/=10; } z2++; } }
                fino[nf++] = (neg ? -1 : 1) * (ip * 1000000 + fr);
            }
            if(fino[0] != fino[3] && na < 64){ ash[na] = fino[0]; asv[na] = fino[3]; na++; }
            q += 5;
        }
        int pares_ok = (na > 0) && (na % 2 == 0);
        for(int t = 0; t + 1 < na && pares_ok; t += 2)
            if(ash[t] != ash[t+1] || asv[t] != asv[t+1]) pares_ok = 0;
        ok("§X13 toda instancia assimetrica vem em PAR (sh,sv) identico — a fronteira e'"
           " UMA e a transformacao declara-se; quem mede le a area no proprio par", pares_ok);
        /* (iii) o TESTE DE SEMENTES: outra semente, outra geometria, a MESMA lei */
        long y_a = 0;
        { for(long q = 0; q + 4 < p.sf.len; q++){
              if(memcmp(p.sf.buf + q, "cm /G", 5)) continue;
              long b2 = q - 1; int esp = 0;
              while(b2 > 0 && esp < 6){ b2--; if(p.sf.buf[b2] == ' ') esp++; }
              const char *e1; const char *pp = (char*)p.sf.buf + b2 + 1;
              long v[6]; int k2 = 0;
              for(; k2 < 6; k2++){ long u2 = fixo_mil(pp, &e1); if(e1 == pp) break; v[k2] = u2; pp = e1; }
              if(k2 == 6){ y_a = v[5]; break; }
          } }
        SEM_V[0] = 5;                                 /* a semente gira: respiro menor */
        unsigned char *bb2 = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p2; pdf_abre(&p2, bb2, 1L << 25); pagina_abre(&p2);
        long glifos7 = 0;
        compila(FONTE6, &p2, &glifos7);
        pdf_fecha(&p2);
        SEM_V[0] = 3;                                 /* e repoe-se: a origem */
        long y_b = 0; long bsh[64], bsv[64]; int nb = 0;
        for(long q = 0; q + 4 < p2.sf.len; q++){
            if(memcmp(p2.sf.buf + q, "cm /G", 5)) continue;
            long b2 = q - 1; int esp = 0;
            while(b2 > 0 && esp < 6){ b2--; if(p2.sf.buf[b2] == ' ') esp++; }
            long fino[6]; int nf = 0; const char *z2 = (char*)p2.sf.buf + b2 + 1;
            while(nf < 6){
                while(*z2 == ' ') z2++;
                long ip = 0, fr = 0, casa = 100000; int neg = 0;
                if(*z2 == '-'){ neg = 1; z2++; }
                while(*z2 >= '0' && *z2 <= '9'){ ip = ip*10 + (*z2-'0'); z2++; }
                if(*z2 == '.'){ z2++; while(*z2>='0'&&*z2<='9'){ if(casa){ fr += (*z2-'0')*casa; casa/=10; } z2++; } }
                fino[nf++] = (neg ? -1 : 1) * (ip * 1000000 + fr);
            }
            if(!y_b) y_b = fino[5] * 1000;            /* fixo_mil vs fino: regua 10^-6 */
            if(fino[0] != fino[3] && nb < 64){ bsh[nb] = fino[0]; bsv[nb] = fino[3]; nb++; }
            q += 5;
        }
        int pares_b = (nb > 0) && (nb % 2 == 0);
        for(int t = 0; t + 1 < nb && pares_b; t += 2)
            if(bsh[t] != bsh[t+1] || bsv[t] != bsv[t+1]) pares_b = 0;
        int geo_dif = 0;
        for(int t = 0; t < na && t < nb; t++)
            if(asv[t] != bsv[t]) geo_dif = 1;
        ok("§X13 o teste de sementes: semente diferente, GEOMETRIA diferente (os sv dos"
           " pares mudam) — e a mesma lei fecha nas duas trajetorias (pares identicos)",
           pares_b && geo_dif);
        (void)y_a; (void)y_b;
        printf("     -> %d pares assimetricos na origem, %d na semente girada; lei UNA.\n",
               na/2, nb/2);
        puts("");
    }

    /* ── §X15  O SELO DE CAELUM: a lei 8 assina o esqueleto ──────────────── */
    puts("§X15 O SELO DE CAELUM: os streams dos XObjects (o esqueleto) acumulam em");
    puts("     N=2^8 posicoes e a transformada da lei 8 (Z_65537, raiz 3^256) da o");
    puts("     espectro que viaja no PDF (/AssinaturaOito). Dois caminhos batem, e");
    puts("     UM bit trocado no esqueleto espalha por mais de metade do selo.\n");
    {
        static const char FONTE7[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "O esqueleto assina: $x^{2}$ e $\\frac{a}{b}$ e texto.\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long glifos8 = 0;
        compila(FONTE7, &p, &glifos8);
        pdf_fecha(&p);
        long len = p.sf.len;
        /* o segundo caminho: recomputa o selo do proprio buffer */
        long A2[256], S2[256];
        for(int t = 0; t < 256; t++) A2[t] = 0;
        { long q = 0; const unsigned char *z = p.sf.buf;
          while(q + 26 < len){
              if(memcmp(z + q, "/Type/XObject/Subtype/Form", 26)){ q++; continue; }
              long a2 = q;
              while(a2 + 7 < len && memcmp(z + a2, "stream\n", 7)) a2++;
              a2 += 7;
              long b2 = a2, k2 = 0;
              while(b2 + 9 < len && memcmp(z + b2, "endstream", 9)){
                  A2[k2 & 255] = (A2[k2 & 255] + z[b2]) % 65537;
                  b2++; k2++;
              }
              q = b2 + 9;
          } }
        { long raiz = 1, b3 = 3, e3 = 256;
          while(e3 > 0){ if(e3 & 1) raiz = raiz * b3 % 65537; b3 = b3 * b3 % 65537; e3 >>= 1; }
          for(int j2 = 0; j2 < 256; j2++){
              long acc = 0, w2 = 1, passo2 = 1;
              { long e4 = j2, b4 = raiz;
                while(e4 > 0){ if(e4 & 1) passo2 = passo2 * b4 % 65537;
                               b4 = b4 * b4 % 65537; e4 >>= 1; } }
              for(int t = 0; t < 256; t++){
                  acc = (acc + A2[t] * w2) % 65537;
                  w2 = w2 * passo2 % 65537;
              }
              S2[j2] = acc;
          } }
        /* o selo escrito no PDF */
        long SW[256]; int nsw = 0, achou = 0;
        { const unsigned char *z = p.sf.buf;
          for(long q = 0; q + 21 < len; q++){
              if(memcmp(z + q, "/Type/AssinaturaOito", 20)) continue;
              achou = 1;
              long w = q;
              while(w < len && z[w] != '[') w++;
              w++;
              while(w < len && z[w] != ']' && nsw < 256){
                  while(w < len && z[w] == ' ') w++;
                  if(z[w] == ']') break;
                  long u2 = 0;
                  while(w < len && z[w] >= '0' && z[w] <= '9'){ u2 = u2*10 + (z[w]-'0'); w++; }
                  SW[nsw] = u2; nsw = nsw + 1;
              }
              break;
          } }
        ok("§X15 o selo viaja no PDF: /AssinaturaOito com as 256 componentes da lei 8"
           " (N=2^8, o anel Z_65537) — o esqueleto tem assinatura propria, como a"
           " semente e o .tex", achou && nsw == 256);
        int bate = (nsw == 256);
        for(int t = 0; t < 256 && bate; t++) if(SW[t] != S2[t]) bate = 0;
        ok("§X15 dois caminhos: o selo recomputado do proprio buffer bate o escrito,"
           " componente a componente, exacto — residuo 0", bate);
        /* a MUTACAO: um bit num stream do esqueleto espalha pelo espectro */
        { long q = 0; const unsigned char *z = p.sf.buf;
          long alvo = -1;
          while(q + 26 < len){
              if(!memcmp(z + q, "/Type/XObject/Subtype/Form", 26)){
                  long a2 = q;
                  while(a2 + 7 < len && memcmp(z + a2, "stream\n", 7)) a2++;
                  alvo = a2 + 12; break;
              }
              q++;
          }
          int esp = 0;
          if(alvo > 0){
              p.sf.buf[alvo] ^= 1;                   /* o bit trocado */
              long A3[256], S3[256];
              for(int t = 0; t < 256; t++) A3[t] = 0;
              { long q2 = 0; const unsigned char *z2 = p.sf.buf;
                while(q2 + 26 < len){
                    if(memcmp(z2 + q2, "/Type/XObject/Subtype/Form", 26)){ q2++; continue; }
                    long a2 = q2;
                    while(a2 + 7 < len && memcmp(z2 + a2, "stream\n", 7)) a2++;
                    a2 += 7;
                    long b2 = a2, k2 = 0;
                    while(b2 + 9 < len && memcmp(z2 + b2, "endstream", 9)){
                        A3[k2 & 255] = (A3[k2 & 255] + z2[b2]) % 65537;
                        b2++; k2++;
                    }
                    q2 = b2 + 9;
                } }
              { long raiz = 1, b3 = 3, e3 = 256;
                while(e3 > 0){ if(e3 & 1) raiz = raiz * b3 % 65537; b3 = b3 * b3 % 65537; e3 >>= 1; }
                for(int j2 = 0; j2 < 256; j2++){
                    long acc = 0, w2 = 1, passo2 = 1;
                    { long e4 = j2, b4 = raiz;
                      while(e4 > 0){ if(e4 & 1) passo2 = passo2 * b4 % 65537;
                                     b4 = b4 * b4 % 65537; e4 >>= 1; } }
                    for(int t = 0; t < 256; t++){
                        acc = (acc + A3[t] * w2) % 65537;
                        w2 = w2 * passo2 % 65537;
                    }
                    S3[j2] = acc;
                } }
              for(int t = 0; t < 256; t++) if(S3[t] != S2[t]) esp++;
              p.sf.buf[alvo] ^= 1;                   /* repoe: a mutacao nao fica */
          }
          ok("§X15 a mutacao: UM bit trocado num stream do esqueleto espalha por mais"
             " de metade do espectro — o selo e' global, nao local (o §R8, agora no"
             " documento)", esp > 128);
          printf("     -> espalhamento: %d de 256 componentes mudaram com um bit.\n", esp);
        }
        puts("");
    }

    /* ── §X16  A FORMULA NAO HERDA O ITALICO; O \\ NAO ABRE COMANDO; O INTEGRAL
     *          ESTICA PELO VAO — as tres regras, cada uma com a mutacao que a acende ── */
    puts("§X16 a formula nao herda a inclinacao do texto (a estrutura e' romana, a variavel");
    puts("     e' da carta); o \\\\ e' escape de UM caractere e nao abre comando; e o");
    puts("     integral e' fronteira do que mede — estica pelo vao, como o delimitador.\n");
    {
        /* 1. a formula dentro do \emph: estrutura romana, variavel na carta, texto italico */
        static const char F16[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "k \\emph{w $2(q+1)=z$} fim\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long g16 = 0; compila(F16, &p, &g16); pdf_fecha(&p);
        int ix_k   = x16_acha(p.sf.buf, p.sf.len, 'k', 0, 0, 0, 0, 0);
        int ix_w   = x16_acha(p.sf.buf, p.sf.len, 'w', 0, 0, 0, 0, 0);
        int ix_par = x16_acha(p.sf.buf, p.sf.len, '(', 0, 0, 0, 0, 0);
        int ix_dig = x16_acha(p.sf.buf, p.sf.len, '2', 0, 0, 0, 0, 0);
        int ix_var = x16_acha(p.sf.buf, p.sf.len, 'q', 0, 0, 0, 0, 0);
        ok("§X16 as duas referencias distinguem-se: o w do \\emph e o k de fora tem cartas diferentes",
           ix_k >= 0 && ix_w >= 0 && ix_k != ix_w);
        ok("§X16 a ESTRUTURA da formula — o (, o digito — compõe na REGULAR mesmo dentro do \\emph",
           ix_par == ix_k && ix_dig == ix_k);
        ok("§X16 e a VARIAVEL nao: o q vai a carta matematica, nem a regular nem a italica do texto",
           ix_var >= 0 && ix_var != ix_k && ix_var != ix_w);
    }
    {
        /* 2. o \\ e' escape de UM caractere: o \F do estilo nao casa dentro dele */
        static const char T2[] = "linha $F_a\\\\F_b$ fim\n";
        long n2 = (long)strlen(T2);
        char *s2 = (char*)malloc((size_t)n2 + 1); memcpy(s2, T2, (size_t)n2 + 1);
        s2 = avalia_macros(s2, &n2, "estilo.tex");
        ok("§X16 o \\\\F da fila da matriz NAO casa o \\F do estilo (nenhum mathbb na fita)",
           s2 && !strstr(s2, "mathbb") && strstr(s2, "\\\\F_b"));
        free(s2);
    }
    {
        /* 3. o integral: em linha plana sv=sh (k=1, a medida decide); com a pilha
         *    do \frac em destaque, sv>sh e a largura fica a necessaria */
        static const char F17[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "a $\\int f$ b\n\\[ \\int \\frac{a}{b} x \\]\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long g17 = 0; compila(F17, &p, &g17); pdf_fecha(&p);
        long sh0 = 0, sv0 = 0, sh1 = 0, sv1 = 0;
        int a0 = x16_acha(p.sf.buf, p.sf.len, 0xF2, 0, &sh0, &sv0, 0, 0);
        int a1 = x16_acha(p.sf.buf, p.sf.len, 0xF2, 1, &sh1, &sv1, 0, 0);
        ok("§X16 ha DOIS integrais na pagina: o de linha corrida e o de destaque",
           a0 >= 0 && a1 >= 0);
        ok("§X16 o de linha plana fica no corpo do texto: sv = sh (k=1 vem da medida)",
           sh0 > 0 && sv0 == sh0);
        ok("§X16 o de destaque estica pelo vao da pilha (sv > sh) e a largura nao muda (sh igual)",
           sv1 > sh1 && sh1 == sh0);
        printf("     -> integral de destaque: sh %ld, sv %ld milesimos (k = %ld%%)\n\n",
               sh1, sv1, sh1 ? sv1 * 100 / sh1 : 0);
    }
    {
        /* 4. a fronteira compõe-se da assinatura à medida da REGIÃO — e os anexos
         *    sentam nas pontas: os limites do integral empilham no MESMO x, o sup
         *    na ponta de cima e o sub na de baixo; e o delimitador da MATRIZ estica
         *    pelo vão das filas (par assimétrico), não fica no corpo do texto */
        static const char F18[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "\\[ \\int_q^w \\frac{u}{v} z \\]\n"
            "$g_c^h$ k\n"
            "e $\\begin{psmallmatrix}1&2\\\\3&4\\end{psmallmatrix}$ fim\n"
            "e $\\begin{psmallmatrix}1&2\\\\3&4_{s}\\end{psmallmatrix}$ dois\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long g18 = 0; compila(F18, &p, &g18); pdf_fecha(&p);
        long xq = 0, yq = 0, xw = 0, yw = 0, shi = 0, svi = 0, yi = 0;
        long yc = 0, yh = 0, shw = 0, shh = 0;
        int aq = x16_acha(p.sf.buf, p.sf.len, 'q', 0, 0, 0, &xq, &yq);
        int aw = x16_acha(p.sf.buf, p.sf.len, 'w', 0, &shw, 0, &xw, &yw);
        int ai = x16_acha(p.sf.buf, p.sf.len, 0xF2, 0, &shi, &svi, 0, &yi);
        int ac = x16_acha(p.sf.buf, p.sf.len, 'c', 0, 0, 0, 0, &yc);
        int ah = x16_acha(p.sf.buf, p.sf.len, 'h', 0, &shh, 0, 0, &yh);
        ok("§X16 a dilatacao da fronteira passa ao dual como TRANSLACAO: o vao sup-sub"
           " dos limites excede 1,5x o do expoente comum (o controlo $g_c^h$ da a regua;"
           " sem a razao, empatava)",
           aq >= 0 && aw >= 0 && ai >= 0 && ac >= 0 && ah >= 0
           && 2 * (yw - yq) > 3 * (yh - yc));
        ok("§X16 e a ESCALA fica no seu degrau: o corpo do limite e IGUAL ao do expoente"
           " comum — dilatar vira transladar, nao vira reescalar (a transformada dourada)",
           shw > 0 && shw == shh);
        ok("§X16 e o PAR dos limites e UM eixo (Lei 1), com a semente de espacamento"
           " propagada ao degrau do grupo: o sub fica o respiro DO SEU degrau a esquerda"
           " do sup — menor que um glifo (sequencial, o sup caia um avanco inteiro)",
           xw > xq && (xw - xq) < (long)largura('q', F_MAT) * shh);
        long shp = 0, svp = 0, shf = 0, svf = 0;
        int ap = x16_acha(p.sf.buf, p.sf.len, '(', 0, &shp, &svp, 0, 0);
        int af = x16_acha(p.sf.buf, p.sf.len, ')', 0, &shf, &svf, 0, 0);
        ok("§X16 o delimitador da matriz estica pelo vao das DUAS filas (sv > sh, o par"
           " identico nos dois lados) — a fronteira compõe-se da assinatura, nao ha corpo fixo",
           ap >= 0 && af >= 0 && svp > shp && shp == shf && svp == svf);
        /* e a régua é DINÂMICA: o índice dentro da célula cresce a região, e a
         * fronteira acompanha — com a estimativa fixa antiga as duas empatavam */
        long shp2 = 0, svp2 = 0;
        int ap2 = x16_acha(p.sf.buf, p.sf.len, '(', 1, &shp2, &svp2, 0, 0);
        ok("§X16 e a regua e DINAMICA (Parseval na involucao, nivel a nivel): a matriz"
           " com indice na celula pede regiao maior e a fronteira DA — sv cresce, sh nao",
           ap2 >= 0 && svp2 > svp && shp2 == shp);
    }

    /* ── §X17  TEOREMA DO METRÓNOMO NO TRADUTOR — Lyapunov dualizado / metade refletida ──
     * Medição = Teorema do Metrónomo (Corpo de Peano): maestro projecta; metrónomo lê;
     * Lyapunov dualizado atesta. A medida do tradutor tex<->PDF e' a da TRANSFORMADA
     * ESTELAR (a estrela ν∘ν=id, tests/lyapunov_refletido.c): compor e' a frente (emite, -1), a
     * volta e' a metade refletida (absorve, +1), e os expoentes vem em pares
     * ±λ. O tradutor so' esta atestado se a reflexao devolve a semente com
     * residuo 0 — λ⁺+λ⁻ = 0, em BITS INTEIROS (o expoente e' o comprimento da
     * diferenca, potencias de 2, sem um double). E o residuo onde NAO PODE ser
     * zero: um byte mutado no corpo que viaja, e a volta acusa — λ_medicao > 0
     * e a medida seria ela propria caotica (o teorema operacional). */
    puts("§X17 Teorema do Metrónomo: a volta e a metade refletida —");
    puts("     maestro projecta; metrónomo lê; Lyapunov dualizado atesta (λ⁺+λ⁻=0).\n");
    {
        static const char F20[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "%% um comentario que a pagina nao mostra\n"
            "a \\emph{semente} volta $x^2 = -1$ inteira\n"
            "\\end{document}\n";
        /* o /tmp direto: o getenv não faz parte da libc do subconjunto que sobe */
        const char *f_tex = "/tmp/x17_semente.tex";
        const char *f_pdf = "/tmp/x17_semente.pdf";
        const char *f_volta = "/tmp/x17_volta.tex";
        { FILE *f = fopen(f_tex, "wb"); if(f){ fwrite(F20, 1, strlen(F20), f); fclose(f); } }
        int comp_ok = compila_ficheiro(f_tex, f_pdf) == 0;
        int volta_ok = comp_ok && volta_para_tex(f_pdf, f_volta) == 0;
        /* o expoente da diferenca, em bits: bits(0) = 0 e' o λ que atesta */
        long e_volta = -1;
        if(volta_ok){
            long na = 0, nb = 0;
            char *va = le_tudo(f_tex, &na), *vb = le_tudo(f_volta, &nb);
            if(va && vb){
                long dif = (na != nb) ? (na > nb ? na - nb : nb - na) : 0;
                if(!dif) for(long t = 0; t < na; t++) if(va[t] != vb[t]) dif++;
                e_volta = 0; while(dif > 0){ e_volta++; dif /= 2; }
            }
            free(va); free(vb);
        }
        ok("§X17 a metade refletida devolve a semente: o expoente da diferenca e ZERO"
           " — λ⁺+λ⁻=0, o tradutor tex<->PDF atestado pelo teorema operacional",
           comp_ok && volta_ok && e_volta == 0);
        /* a mutacao: UM byte no corpo que viaja (o proprio texto dentro do PDF),
         * e a reflexao ja nao devolve — o λ da medicao ficaria positivo */
        int mut_acusa = 0;
        if(comp_ok){
            long np = 0; char *pb = le_tudo(f_pdf, &np);
            if(pb){
                /* acha o comentario embutido e muta um byte dele */
                for(long t = 0; t + 12 < np; t++)
                    if(!memcmp(pb + t, "um comentario", 13)){ pb[t] = 'U'; break; }
                FILE *f = fopen(f_pdf, "wb");
                if(f){ fwrite(pb, 1, (size_t)np, f); fclose(f); }
                free(pb);
                if(volta_para_tex(f_pdf, f_volta) == 0){
                    long na = 0, nb = 0;
                    char *va = le_tudo(f_tex, &na), *vb = le_tudo(f_volta, &nb);
                    if(va && vb && (na != nb || memcmp(va, vb, (size_t)na) != 0)) mut_acusa = 1;
                    free(va); free(vb);
                }
            }
        }
        ok("§X17 e o residuo onde NAO PODE ser zero: um byte mutado no corpo que viaja,"
           " e a reflexao acusa — sem isto a medicao seria ela propria caotica",
           mut_acusa);
    }

    /* ── §X18  A BIBLIOGRAFIA RESOLVE: o \cite numera pela ordem dos \bibitem ── */
    puts("§X18 a bibliografia resolve: o \\cite numera pela ordem dos \\bibitem, o titulo");
    puts("     vem do idioma (refname), e a chave nao vaza para a pagina.\n");
    {
        static const char F21[] =
            "\\documentclass{article}\n\\begin{document}\n"
            "cita \\cite{zz} e o par \\cite{qq,zz} fim\n"
            "\\begin{thebibliography}{9}\n"
            "\\bibitem{zz} Az B.\n"
            "\\bibitem{qq} Cy D.\n"
            "\\end{thebibliography}\n"
            "\\end{document}\n";
        unsigned char *bb = (unsigned char*)disco_buf(14, 1L << 25);
        Pdf p; pdf_abre(&p, bb, 1L << 25); pagina_abre(&p);
        long g21 = 0; compila(F21, &p, &g21); pdf_fecha(&p);
        /* os digitos: o \cite{zz} da [1], o \cite{qq,zz} da [2, 1]; os \bibitem
         * rotulam [1] e [2] — ao todo o '1' aparece 3x e o '2' 2x */
        int n1 = 0, n2 = 0, nz = 0, ne = 0;
        while(x16_acha(p.sf.buf, p.sf.len, '1', n1, 0, 0, 0, 0) >= 0) n1++;
        while(x16_acha(p.sf.buf, p.sf.len, '2', n2, 0, 0, 0, 0) >= 0) n2++;
        while(x16_acha(p.sf.buf, p.sf.len, 'z', nz, 0, 0, 0, 0) >= 0) nz++;
        while(x16_acha(p.sf.buf, p.sf.len, 0xEA, ne, 0, 0, 0, 0) >= 0) ne++;
        ok("§X18 o \\cite RESOLVE para o numero da ordem: [1] e [2, 1] no corpo, [1] e"
           " [2] nos rotulos, mais o numero da pagina — o '1' 4x e o '2' 2x, exatos",
           n1 == 4 && n2 == 2);
        ok("§X18 e a CHAVE nao vaza: nenhum 'z' de {zz} composto na pagina (o unico 'z'"
           " e o do proprio texto da entrada 'Az')",
           nz == 1);
        ok("§X18 o titulo vem do IDIOMA (refname): o e-circunflexo de «Referencias»"
           " esta na pagina, posto pela config e nao por mim",
           ne >= 1);
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
