/* libc.c — A BIBLIOTECA, ESCRITA NA MESMA RÉGUA.
 *
 * O `tests/tex.c` chama 32 funções da biblioteca do sistema, 461 vezes. No navegador não há
 * biblioteca do sistema — e não faz falta: quase todas elas são LAÇOS SOBRE `char *`, e isso
 * sobe pelo tradutor que já cá está, sem uma linha nova nele.
 *
 * Por isso não se importa nada. Escreve-se aqui, em C, no subconjunto que sobe, e traduz-se
 * com o mesmo `traduz` de tudo o resto. É a régua a valer para si própria.
 *
 * ── O QUE ESTÁ AQUI ─────────────────────────────────────────────────────────────────
 *   texto        strlen strcmp strncmp strcpy strncpy strcat strstr strchr strrchr
 *   memória      memcpy memmove memset memcmp
 *   caracteres   isalpha isdigit isalnum isspace isupper islower toupper tolower
 *   números      atoi atol strtol atof
 *
 * ── O QUE NÃO ESTÁ, E PORQUÊ ────────────────────────────────────────────────────────
 *   malloc/free/realloc   pedir memória em execução é o que este sistema não faz. Onde o
 *                         `tex.c` os usa, o que ele quer é um slot — e isso é uma decisão
 *                         do desenho, não minha para tomar calado.
 *   fopen/fread/…         o ficheiro é um backend do MOVE, como o canal e o pool do sql.c:
 *                         o host põe os bytes nos slots e o programa lê slots.
 *   printf/sscanf         o formatador é o próximo pedaço, e é grande: mede-se à parte.
 *
 * O `char` do wasm lê-se COM SINAL, e o C manda comparar texto sem ele — daí o `& 255` em
 * cada comparação. Não é enfeite: sem ele um byte acima de 127 comparava negativo, e a
 * ordenação de qualquer texto acentuado saía ao contrário.
 *
 *   ../tools/bin/traduz libc.c -o libc.wasm
 */

/* ── o texto ─────────────────────────────────────────────────────────────────────── */

int strlen(char *s){
    int n = 0;
    while(s[n]) n++;
    return n;
}

int strcmp(char *a, char *b){
    int i = 0;
    for(;;){
        int x = a[i] & 255;
        int y = b[i] & 255;
        if(x != y) return x - y;
        if(x == 0) return 0;
        i++;
    }
}

int strncmp(char *a, char *b, int n){
    for(int i = 0; i < n; i++){
        int x = a[i] & 255;
        int y = b[i] & 255;
        if(x != y) return x - y;
        if(x == 0) return 0;
    }
    return 0;
}

char *strcpy(char *d, char *o){
    int i = 0;
    while(o[i]){ d[i] = o[i]; i++; }
    d[i] = 0;
    return d;
}

/* e o `strncpy` do C NÃO termina se não couber — é assim que ele é, e copiá-lo diferente
 * daria um texto que o resto do programa lê até onde não deve */
char *strncpy(char *d, char *o, int n){
    int i = 0;
    while(i < n && o[i]){ d[i] = o[i]; i++; }
    while(i < n){ d[i] = 0; i++; }
    return d;
}

char *strcat(char *d, char *o){
    int i = 0;
    while(d[i]) i++;
    int j = 0;
    while(o[j]){ d[i] = o[j]; i++; j++; }
    d[i] = 0;
    return d;
}

char *strchr(char *s, int c){
    int i = 0;
    for(;;){
        if((s[i] & 255) == (c & 255)) return s + i;
        if(s[i] == 0) return 0;
        i++;
    }
}

char *strrchr(char *s, int c){
    char *achou = 0;
    int i = 0;
    for(;;){
        if((s[i] & 255) == (c & 255)) achou = s + i;
        if(s[i] == 0) return achou;
        i++;
    }
}

char *strstr(char *h, char *a){
    if(a[0] == 0) return h;
    int i = 0;
    while(h[i]){
        int j = 0;
        while(a[j] && h[i+j] == a[j]) j++;
        if(a[j] == 0) return h + i;
        i++;
    }
    return 0;
}

/* ── a memória ───────────────────────────────────────────────────────────────────── */

char *memcpy(char *d, char *o, int n){
    for(int i = 0; i < n; i++) d[i] = o[i];
    return d;
}

/* e o `memmove` sabe quando os dois se sobrepõem: copia ao contrário, senão o que se lê
 * já foi escrito por cima */
char *memmove(char *d, char *o, int n){
    if(d == o || n <= 0) return d;
    if(d < o){ for(int i = 0; i < n; i++) d[i] = o[i]; return d; }
    for(int i = n - 1; i >= 0; i--) d[i] = o[i];
    return d;
}

char *memset(char *d, int c, int n){
    for(int i = 0; i < n; i++) d[i] = c;
    return d;
}

int memcmp(char *a, char *b, int n){
    for(int i = 0; i < n; i++){
        int x = a[i] & 255;
        int y = b[i] & 255;
        if(x != y) return x - y;
    }
    return 0;
}

/* ── os caracteres ───────────────────────────────────────────────────────────────── */

int isdigit(int c){ return c >= 48 && c <= 57; }
int isupper(int c){ return c >= 65 && c <= 90; }
int islower(int c){ return c >= 97 && c <= 122; }
int isalpha(int c){ return isupper(c) || islower(c); }
int isalnum(int c){ return isalpha(c) || isdigit(c); }
int isspace(int c){ return c == 32 || (c >= 9 && c <= 13); }
int toupper(int c){ return islower(c) ? c - 32 : c; }
int tolower(int c){ return isupper(c) ? c + 32 : c; }

/* ── os números ──────────────────────────────────────────────────────────────────── */

/* o `strtol` do C pára onde deixa de perceber e diz ONDE parou — e é isso que o torna útil:
 * quem chama continua a ler dali. Por isso o `fim` é um ponteiro para ponteiro. */
long strtol(char *s, char **fim, int base){
    int i = 0;
    while(isspace(s[i] & 255)) i++;
    int sinal = 1;
    if(s[i] == 45){ sinal = -1; i++; }
    else if(s[i] == 43) i++;
    if(base == 0){
        if(s[i] == 48 && (s[i+1] == 120 || s[i+1] == 88)){ base = 16; i += 2; }
        else if(s[i] == 48){ base = 8; i++; }
        else base = 10;
    } else if(base == 16 && s[i] == 48 && (s[i+1] == 120 || s[i+1] == 88)) i += 2;
    long v = 0;
    int algum = 0;
    for(;;){
        int c = s[i] & 255;
        int d = -1;
        if(c >= 48 && c <= 57) d = c - 48;
        else if(c >= 97 && c <= 122) d = c - 97 + 10;
        else if(c >= 65 && c <= 90) d = c - 65 + 10;
        if(d < 0 || d >= base) break;
        v = v * base + d;
        algum = 1;
        i++;
    }
    if(fim) *fim = algum ? s + i : s;
    return sinal * v;
}

int atoi(char *s){ return (int)strtol(s, 0, 10); }
long atol(char *s){ return strtol(s, 0, 10); }

/* o `atof` lê o sinal, a parte inteira, a fracção e o expoente. A fracção divide-se por dez
 * uma vez por casa em vez de se multiplicar por 10^-k: são as mesmas contas na mesma ordem
 * que o formatador do sistema faz, e é assim que o último bit bate. */
double atof(char *s){
    int i = 0;
    while(isspace(s[i] & 255)) i++;
    double sinal = 1.0;
    if(s[i] == 45){ sinal = -1.0; i++; }
    else if(s[i] == 43) i++;
    double v = 0.0;
    while(isdigit(s[i] & 255)){ v = v * 10.0 + (double)((s[i] & 255) - 48); i++; }
    if(s[i] == 46){
        i++;
        double p = 1.0;
        while(isdigit(s[i] & 255)){
            p = p / 10.0;
            v = v + p * (double)((s[i] & 255) - 48);
            i++;
        }
    }
    if(s[i] == 101 || s[i] == 69){
        i++;
        int es = 1;
        if(s[i] == 45){ es = -1; i++; }
        else if(s[i] == 43) i++;
        int e = 0;
        while(isdigit(s[i] & 255)){ e = e * 10 + ((s[i] & 255) - 48); i++; }
        for(int k = 0; k < e; k++){
            if(es > 0) v = v * 10.0;
            else v = v / 10.0;
        }
    }
    return sinal * v;
}

/* ── O FORMATADOR ────────────────────────────────────────────────────────────────────
 *
 * Os formatos NÃO são os que a norma tem: são os que o `tex.c` usa, medidos nele —
 * `%d %ld %s %c %x %f` com largura, precisão, zero à esquerda e o `%%`. Escrever os outros
 * seria escrever código que ninguém chama, e código que ninguém chama não se mede.
 *
 * O `...` chega aqui como uma FITA de slots de oito bytes: quem chama escreveu-os no seu
 * quadro e passou o endereço. `va_arg` lê um slot e anda um.
 *
 * E o `snprintf` do C conta o que ESCREVERIA, não o que coube — é por isso que ele serve para
 * medir antes de escrever, e copiá-lo de outra maneira daria um tamanho errado a quem confia.
 */

/* O formatador escreve por UM sítio só: `poe1`. Com buffer (`snprintf`, que mede) grava em
 * `d`; SEM buffer (`fprintf`/`printf`, que emitem) `d` é nulo e cada byte sai EM ORDEM pela
 * porta — a estrela irradia, não enfileira. Nenhum `char t[4096]` a reter o que já se sabe. */
int fputc(int c, int h);
int SINK_H;                                     /* a porta por onde o formatador sem buffer emite */
/* neuronio.c: 1 bit desenha; o dual é a ausência. O banco (FICH_NOME) não cola na
 * célula da estrela — um store vizinho (SINK_H) apagava FICH[0] e o catálogo sumia. */
long GUARDA_SINK;
static int poe1(char *d, int n, int *k, int c){
    if(d){ if(*k < n - 1) d[*k] = c; }          /* com buffer: grava, contado */
    else fputc(c, SINK_H);                       /* sem buffer: emite em ordem, e nada se grava */
    *k = *k + 1;
    return 0;
}

/* o inteiro em texto: as casas saem ao contrário, e por isso viram-se no fim */
static int poe_num(char *d, int n, int *k, long v, int base, int larg, int zero, int sinal_mais){
    char t[24];
    int m = 0;
    int neg = 0;
    if(sinal_mais && v < 0){ neg = 1; v = -v; }
    if(v == 0){ t[m] = 48; m++; }
    while(v != 0){
        long r = v % base;
        if(r < 0) r = -r;
        t[m] = r < 10 ? (char)(48 + r) : (char)(97 + r - 10);
        m++;
        v = v / base;
    }
    int corpo = m + neg;
    if(zero && larg > corpo){
        if(neg) poe1(d, n, k, 45);
        for(int i = 0; i < larg - corpo; i++) poe1(d, n, k, 48);
        neg = 0;
    } else {
        for(int i = 0; i < larg - corpo; i++) poe1(d, n, k, 32);
    }
    if(neg) poe1(d, n, k, 45);
    for(int i = m - 1; i >= 0; i--) poe1(d, n, k, t[i]);
    return 0;
}

/* O REAL COM CASAS FIXAS, E A CONTA É EM INTEIROS.
 *
 * Somar meio e cortar dá 3 onde a biblioteca do sistema dá 2 — ela leva o meio para o PAR. Mas
 * corrigir só a regra não chega: `v × 10^p` JÁ ARREDONDA antes de haver o que decidir, e aí o
 * empate que se está a julgar não é o do número, é o do produto. Foi o que o medidor mostrou
 * em `-0.0005`.
 *
 * Um `double` é `m / 2^s` EXACTO, e o `s` acha-se dobrando até não haver fracção — dobrar é
 * exacto. Daí em diante é tudo inteiro: `m·10^p` dividido por `2^s`, com o resto a decidir, e
 * o empate a ir para o par. Sem uma única divisão de reais.
 *
 * Quando os inteiros não chegam (expoentes grandes), diz-se pelo caminho antigo em vez de
 * fingir exactidão — e é o único sítio onde isso acontece. */
static int poe_real(char *d, int n, int *k, double v, int prec){
    if(prec < 0) prec = 6;
    int neg = 0;
    if(v < 0.0){ neg = 1; v = -v; }
    long escala_i = 1;
    for(int i = 0; i < prec; i++) escala_i = escala_i * 10;

    /* v = m / 2^s, exacto */
    double t = v;
    long s = 0;
    while(t != (double)(long)t && s < 62){ t = t * 2.0; s = s + 1; }
    long m = (long)t;

    long r = 0;
    int exacto = 0;
    if(t == (double)m && s < 62 && m <= 9223372036854775807 / escala_i){
        long num = m * escala_i;
        if(s == 0) r = num;
        else {
            long q = num >> s;
            long resto = num - (q << s);
            long meio = 1L << (s - 1);
            if(resto > meio) q = q + 1;
            else if(resto == meio){ if(q % 2 != 0) q = q + 1; }
            r = q;
        }
        exacto = 1;
    }
    if(!exacto){
        double escala = 1.0;
        for(int i = 0; i < prec; i++) escala = escala * 10.0;
        double x = v * escala;
        r = (long)x;
        double frac = x - (double)r;
        if(frac > 0.5) r = r + 1;
        else if(frac == 0.5){ if(r % 2 != 0) r = r + 1; }
    }

    if(neg) poe1(d, n, k, 45);
    poe_num(d, n, k, r / escala_i, 10, 0, 0, 0);
    if(prec > 0){
        poe1(d, n, k, 46);
        long f = r % escala_i;
        long p = escala_i / 10;
        for(int i = 0; i < prec; i++){
            long dig = (f / p) % 10;
            poe1(d, n, k, 48 + (int)dig);
            p = p / 10;
        }
    }
    return 0;
}

int vsnprintf(char *d, int n, char *f, va_list ap){
    int k = 0;
    int i = 0;
    while(f[i]){
        if((f[i] & 255) != 37){ poe1(d, n, &k, f[i] & 255); i++; continue; }
        i++;
        if((f[i] & 255) == 37){ poe1(d, n, &k, 37); i++; continue; }
        int zero = 0;
        int esq = 0;
        while((f[i] & 255) == 48 || (f[i] & 255) == 45){
            if((f[i] & 255) == 48) zero = 1; else esq = 1;
            i++;
        }
        int larg = 0;
        while(isdigit(f[i] & 255)){ larg = larg * 10 + ((f[i] & 255) - 48); i++; }
        int prec = -1;
        if((f[i] & 255) == 46){
            i++;
            prec = 0;
            while(isdigit(f[i] & 255)){ prec = prec * 10 + ((f[i] & 255) - 48); i++; }
        }
        int lon = 0;
        while((f[i] & 255) == 108){ lon = 1; i++; }
        int c = f[i] & 255;
        i++;
        if(c == 100 || c == 105){                       /* d, i */
            long v;
            if(lon) v = va_arg(ap, long);
            else v = (long)va_arg(ap, int);
            poe_num(d, n, &k, v, 10, larg, zero, 1);
            continue;
        }
        if(c == 120){                                   /* x */
            long v;
            if(lon) v = va_arg(ap, long);
            else v = (long)va_arg(ap, int);
            poe_num(d, n, &k, v, 16, larg, zero, 0);
            continue;
        }
        if(c == 117){                                   /* u — sem sinal, e aqui cabe em long */
            long v;
            if(lon) v = va_arg(ap, long);
            else v = (long)va_arg(ap, int);
            if(v < 0) v = -v;
            poe_num(d, n, &k, v, 10, larg, zero, 0);
            continue;
        }
        if(c == 99){                                    /* c */
            int v = va_arg(ap, int);
            poe1(d, n, &k, v & 255);
            continue;
        }
        if(c == 115){                                   /* s */
            char *t = va_arg(ap, char *);
            if(t == 0) t = "(null)";
            int m = 0;
            while(t[m] && (prec < 0 || m < prec)) m++;
            if(!esq) for(int j = 0; j < larg - m; j++) poe1(d, n, &k, 32);
            for(int j = 0; j < m; j++) poe1(d, n, &k, t[j] & 255);
            if(esq) for(int j = 0; j < larg - m; j++) poe1(d, n, &k, 32);
            continue;
        }
        if(c == 102 || c == 70){                        /* f */
            double v = va_arg(ap, double);
            poe_real(d, n, &k, v, prec);
            continue;
        }
        poe1(d, n, &k, 37);                             /* o que não conheço, digo-o inteiro */
        poe1(d, n, &k, c);
    }
    if(d && n > 0){                             /* o terminador só existe se houver buffer */
        if(k < n) d[k] = 0;
        else d[n-1] = 0;
    }
    return k;
}

int snprintf(char *d, int n, char *f, ...){
    va_list ap;
    va_start(ap, f);
    int r = vsnprintf(d, n, f, ap);
    va_end(ap);
    return r;
}

int sprintf(char *d, char *f, ...){
    va_list ap;
    va_start(ap, f);
    int r = vsnprintf(d, 1000000, f, ap);
    va_end(ap);
    return r;
}

/* ── A LEITURA ───────────────────────────────────────────────────────────────────────
 *
 * O `sscanf` é o dual do formatador, e os formatos são os mesmos que o `tex.c` usa: `%lf`,
 * `%d`, `%s` com largura, `%c`, e o conjunto `%2[a-z]` — que é o que lê as unidades («pt»,
 * «cm») logo a seguir ao número.
 *
 * E devolve QUANTOS atribuiu, parando no primeiro que não bate: é isso que torna útil um
 * formato como «%lf}{%lf» — quem chama vê pelo número quanto é que o texto tinha.
 */

static int le_inteiro(char *s, int *pos, long *v, int larg){
    int i = *pos;
    int lidos = 0;
    while(isspace(s[i] & 255)) i++;
    int sinal = 1;
    if(s[i] == 45){ sinal = -1; i++; lidos++; }
    else if(s[i] == 43){ i++; lidos++; }
    long r = 0;
    int alg = 0;
    while(isdigit(s[i] & 255)){
        if(larg > 0 && lidos >= larg) break;
        r = r * 10 + ((s[i] & 255) - 48);
        alg = 1; i++; lidos++;
    }
    if(!alg) return 0;
    *v = sinal * r;
    *pos = i;
    return 1;
}

static int le_real(char *s, int *pos, double *v){
    int i = *pos;
    while(isspace(s[i] & 255)) i++;
    int ini = i;
    if(s[i] == 45 || s[i] == 43) i++;
    int alg = 0;
    while(isdigit(s[i] & 255)){ i++; alg = 1; }
    if(s[i] == 46){ i++; while(isdigit(s[i] & 255)){ i++; alg = 1; } }
    if(!alg) return 0;
    if(s[i] == 101 || s[i] == 69){
        int g = i + 1;
        if(s[g] == 45 || s[g] == 43) g++;
        if(isdigit(s[g] & 255)){ i = g; while(isdigit(s[i] & 255)) i++; }
    }
    /* o texto já está delimitado: quem o converte é o `atof`, e é um só a saber a conta */
    char t[64];
    int m = 0;
    for(int k = ini; k < i; k++){ if(m < 63){ t[m] = s[k]; m++; } }
    t[m] = 0;
    *v = atof(t);
    *pos = i;
    return 1;
}

/* o conjunto `[a-z]`, `[^ ]`, `[abc]` — devolve 1 se o byte pertence */
static int no_conjunto(char *f, int ini, int fim, int c, int nega){
    int k = ini;
    int achou = 0;
    while(k < fim){
        if(k + 2 < fim && (f[k+1] & 255) == 45){
            if((c >= (f[k] & 255)) && (c <= (f[k+2] & 255))) achou = 1;
            k = k + 3;
            continue;
        }
        if((f[k] & 255) == c) achou = 1;
        k++;
    }
    if(nega) return !achou;
    return achou;
}

int vsscanf(char *s, char *f, va_list ap){
    int i = 0;
    int j = 0;
    int n = 0;
    while(f[j]){
        int fc = f[j] & 255;
        if(isspace(fc)){ while(isspace(s[i] & 255)) i++; j++; continue; }
        if(fc != 37){
            if((s[i] & 255) != fc) return n;
            i++; j++;
            continue;
        }
        j++;
        if((f[j] & 255) == 37){
            if((s[i] & 255) != 37) return n;
            i++; j++;
            continue;
        }
        int larg = 0;
        while(isdigit(f[j] & 255)){ larg = larg * 10 + ((f[j] & 255) - 48); j++; }
        int lon = 0;
        while((f[j] & 255) == 108){ lon = 1; j++; }
        int c = f[j] & 255;
        j++;
        if(c == 100 || c == 105){
            long v = 0;
            if(!le_inteiro(s, &i, &v, larg)) return n;
            if(lon){ long *p = va_arg(ap, long *); *p = v; }
            else { int *p = va_arg(ap, int *); *p = (int)v; }
            n++;
            continue;
        }
        if(c == 102 || c == 101 || c == 103){
            double v = 0.0;
            if(!le_real(s, &i, &v)) return n;
            if(lon){ double *p = va_arg(ap, double *); *p = v; }
            else { double *p = va_arg(ap, double *); *p = v; }
            n++;
            continue;
        }
        if(c == 115){
            while(isspace(s[i] & 255)) i++;
            if(s[i] == 0) return n;
            char *p = va_arg(ap, char *);
            int m = 0;
            while(s[i] && !isspace(s[i] & 255)){
                if(larg > 0 && m >= larg) break;
                p[m] = s[i];
                m++; i++;
            }
            p[m] = 0;
            n++;
            continue;
        }
        if(c == 99){
            char *p = va_arg(ap, char *);
            int quantos = larg > 0 ? larg : 1;
            for(int k = 0; k < quantos; k++){
                if(s[i] == 0) return n;
                p[k] = s[i];
                i++;
            }
            n++;
            continue;
        }
        if(c == 91){
            int nega = 0;
            if((f[j] & 255) == 94){ nega = 1; j++; }
            int ini = j;
            if((f[j] & 255) == 93) j++;
            while(f[j] && (f[j] & 255) != 93) j++;
            int fim = j;
            if((f[j] & 255) == 93) j++;
            char *p = va_arg(ap, char *);
            int m = 0;
            while(s[i] && no_conjunto(f, ini, fim, s[i] & 255, nega)){
                if(larg > 0 && m >= larg) break;
                p[m] = s[i];
                m++; i++;
            }
            p[m] = 0;
            if(m == 0) return n;
            n++;
            continue;
        }
        return n;
    }
    return n;
}

int sscanf(char *s, char *f, ...){
    va_list ap;
    va_start(ap, f);
    int r = vsscanf(s, f, ap);
    va_end(ap);
    return r;
}

/* ── OS FICHEIROS SÃO SLOTS ──────────────────────────────────────────────────────────
 *
 * No navegador não há sistema de ficheiros, e não faz falta: o `sql.c` já mostrou que
 * `LOAD`/`STORE` mudam de backend pelo NÚMERO DO SLOT — abaixo de `S_CANAL` vão ao ficheiro,
 * acima vão à banda, e «a ISA não cresceu, o compilador não mudou». Aqui é o mesmo: quem
 * hospeda põe os bytes em slots e diz o nome; o programa abre pelo nome e lê slots.
 *
 * E os nomes não são inventados: são os que o `tools/corpo.sh` MEDIU que o tradutor abre —
 * o `fopen` interceptado disse-os, ficheiro a ficheiro.
 *
 * A agulha é a única coisa que se guarda por ficheiro aberto: onde ela vai. Ler é
 * `MOVE(+1)` a partir dela, escrever é `MOVE(-1)`, e `fseek` é pô-la noutro sítio — a mesma
 * operação com o destino mudado, que é o que o salto sempre foi.
 */

#define MAX_FICH   64
#define MAX_AGULHA 16
#define NOME_MAX   160

/* o nome mora no banco (vfs/LS), não num array .bss — copiar para DRAM é fundir
 * estrela e banco, e a composição escrevia por cima (catalogo.tex sumia). */
char *FICH_NOME[MAX_FICH];
int   FICH_END[MAX_FICH];
int   FICH_TAM[MAX_FICH];
int   N_FICH;

int  AG_FICH[MAX_AGULHA];      /* que ficheiro; 0 = agulha livre */
int  AG_POS[MAX_AGULHA];       /* onde ela vai */
int  AG_ESC[MAX_AGULHA];       /* 1 se foi aberta para escrever */

/* a área onde o que se escreve fica, até quem hospeda a vir buscar.
 * NÃO é um array estático: o catálogo do tradutor passa dos 64 MB, e um
 * `char SAIDA[128M]` no .bss do wasm é o MONTE que a teoria da medida
 * proíbe. Cresce por malloc (memory.grow), contado. tmpfile ainda passa
 * aqui; o PDF da composição NÃO — vive no slot 14, lido por MOVE(+1). */
char *SAIDA;
int   SAIDA_CAP;
int   SAIDA_N;

/* ── o disco do tradutor: banco 0–2 + rascunho 3–15 (dual do mmap) ──
 * O hospedeiro escreve na vista; a ISA só MOVE(slot, ±1). Um disco, duas
 * plataformas. PDF = slot 14, malloc após MARCO — cresce no compose, recua. */
int FAT_TAM[16];
int FAT_OFF[16];
int FAT_BASE;
int FAT_OK;
int PDF_N;
int CURSOR;                /* o cursor do disco linear — uma declaração só */
int DISCO_FIM;
int MARCO;                 /* depois do banco (fatias 0–2 + vfs); rascunho past isto */
int SLOT_PTR[16];          /* rascunho 3–15: malloc após MARCO; recua com o 1 bit */
int SLOT_TAM[16];          /* bytes nascidos (PDF pode ser < 128 MiB, o resto do tecto) */

void fat_layout(void){
    if(FAT_OK) return;
    FAT_TAM[0]=1<<20; FAT_TAM[1]=1<<16; FAT_TAM[2]=1<<16; FAT_TAM[3]=1<<22;
    FAT_TAM[4]=1<<22; FAT_TAM[5]=1<<20; FAT_TAM[6]=1<<16; FAT_TAM[7]=1<<18;
    FAT_TAM[8]=1<<16; FAT_TAM[9]=1<<14; FAT_TAM[10]=1<<18; FAT_TAM[11]=1<<16;
    FAT_TAM[12]=1<<16; FAT_TAM[13]=1<<16; FAT_TAM[14]=1<<27; FAT_TAM[15]=1<<20;
    FAT_OFF[0]=0;
    { int k = 1; while(k < 16){ FAT_OFF[k] = FAT_OFF[k-1] + FAT_TAM[k-1]; k = k + 1; } }
    FAT_OK = 1;
}

/* só o banco no inicia: estilo/classe/idioma. Rascunho (3–15) não se reserva. */
int tam_fatias(void){
    fat_layout();
    return FAT_OFF[3];
}

char *prende_fatias(void){
    fat_layout();
    if(FAT_BASE) return (char*)FAT_BASE;
    if(CURSOR == 0){
        CURSOR = __disco_paginas() * 65536;
        DISCO_FIM = CURSOR;
    }
    {
        /* +64 KB entre banco 0–2 e vfs: soma directa. O rascunho (fonte/fundo/PDF)
         * nasce depois do MARCO — já não pode comer o nome catalogo.tex. */
        int n = tam_fatias() + 65536;
        FAT_BASE = CURSOR;
        while(FAT_BASE + n > DISCO_FIM){
            int faltam = (FAT_BASE + n - DISCO_FIM + 65535) / 65536;
            int antes = __disco_cresce(faltam);
            if(antes < 0){ FAT_BASE = 0; return 0; }
            DISCO_FIM = DISCO_FIM + faltam * 65536;
        }
        CURSOR = FAT_BASE + n;
        MARCO = CURSOR;            /* banco = fatias 0–2; o host MOVE o corpo depois e marca_vfs */
    }
    return (char*)FAT_BASE;
}

int end_fatia(int i){
    if(i < 0 || i >= 16) return 0;
    if(i > 2){
        if(SLOT_PTR[i]) return SLOT_PTR[i];
        return 0;                  /* ainda não nasceu — não prende 128 MiB vazios */
    }
    if(!FAT_BASE){ if(!prende_fatias()) return 0; }
    return FAT_BASE + FAT_OFF[i];
}

int tam_fatia(int i){
    fat_layout();
    if(i < 0 || i >= 16) return 0;
    return FAT_TAM[i];
}

/* MOVE(slot, sentido): Lei 1 + trial. −1 emite (garante endereço), +1 absorve
 * (só o nascido), 0 atravessa. Mesmo contrato do painel (escreve→chama→lê):
 * o host escreve na vista; o módulo só aponta o slot. disco_fatia = MOVE(−1). */
int MOVE(int slot, int sentido){
    int n;
    char *p;
    if(slot < 0 || slot >= 16) return 0;
    if(sentido >= 0) return end_fatia(slot);   /* +1 absorve / 0 atravessa: não nasce */
    /* −1 emite */
    if(slot <= 2) return end_fatia(slot);
    if(SLOT_PTR[slot]) return SLOT_PTR[slot];
    fat_layout();
    n = FAT_TAM[slot];
    if(slot == 14){
        int tecto = 4096 * 65536;
        int folga = 12 * 1048576;
        int base = MARCO ? MARCO : CURSOR;
        int resto = tecto - base - folga;
        if(resto > 0 && resto < n) n = resto;
        if(n < 1048576) n = 1048576;
    }
    p = malloc(n);
    if(!p) return 0;
    if(slot != 14){
        int j = 0;
        while(j < n){ p[j] = 0; j = j + 1; }
    }
    SLOT_PTR[slot] = (int)p;
    SLOT_TAM[slot] = n;
    return (int)p;
}

/* reserva no disco linear (depois das fatias, ou antes se ainda não presas).
 * É o malloc do vfs: o host escreve aí, poe_ficheiro só regista o nome. */
int vfs_reserva(int n){
    if(n < 1) n = 1;
    n = ((n + 7) / 8) * 8;
    if(CURSOR == 0){
        CURSOR = __disco_paginas() * 65536;
        DISCO_FIM = CURSOR;
    }
    while(CURSOR + n > DISCO_FIM){
        int faltam = (CURSOR + n - DISCO_FIM + 65535) / 65536;
        int antes = __disco_cresce(faltam);
        if(antes < 0) return 0;
        DISCO_FIM = DISCO_FIM + faltam * 65536;
    }
    {
        int p = CURSOR;
        CURSOR = CURSOR + n;
        return p;
    }
}

void marca_saida(char *p, int n){
    if(p) SLOT_PTR[14] = (int)p;
    PDF_N = n;
}

/* o banco (LS) já está no vfs: daqui para a frente é só a composição.
 * 1 bit: CURSOR volta ao MARCO — MOVE(−1) no monte, sem apagar o corpo.
 * SLOT_PTR recua: o rascunho (fonte/PDF) não alimenta o próximo doc. */
void marca_vfs(void){ MARCO = CURSOR; }
int volta_compila(void){
    if(MARCO) CURSOR = MARCO;
    PDF_N = 0;
    SAIDA_N = 0;
    { int i = 3; while(i < 16){ SLOT_PTR[i] = 0; SLOT_TAM[i] = 0; i++; } }
    { int h = 1; while(h < MAX_AGULHA){ AG_FICH[h] = 0; AG_ESC[h] = 0; AG_POS[h] = 0; h++; } }
    /* banco = LS/Map no host; o FICH da composição recua com o 1 bit.
     * (poe antes de marca_vfs também sai — o cliente volta a pôr via miss.) */
    N_FICH = 0;
    return MARCO;
}

/* ── a porta do hospedeiro ───────────────────────────────────────────────────────── */

int poe_ficheiro(char *nome, char *dados, int n){
    if(N_FICH >= MAX_FICH) return 0;
    FICH_NOME[N_FICH] = nome;          /* ponteiro no disco; sem strcpy para .bss */
    FICH_END[N_FICH] = (int)dados;
    FICH_TAM[N_FICH] = n;
    N_FICH = N_FICH + 1;
    return N_FICH;
}
int end_saida(void){
    if(SLOT_PTR[14]) return SLOT_PTR[14];
    return (int)(long)SAIDA;
}
/* bytes do slot já posto pelo host — a carta lê sem segunda cópia (malloc). */
char *ficheiro_bytes(int h){
    if(h <= 0 || h >= MAX_AGULHA) return 0;
    int f = AG_FICH[h] - 1;
    if(f < 0) return 0;
    return (char*)(FICH_END[f] + AG_POS[h]);
}
int ficheiro_tam(int h){
    if(h <= 0 || h >= MAX_AGULHA) return 0;
    int f = AG_FICH[h] - 1;
    if(f < 0) return 0;
    return FICH_TAM[f] - AG_POS[h];
}

int tam_saida(void){ return PDF_N; }
void limpa_saida(void){ PDF_N = 0; SAIDA_N = 0; }

/* o nome pode vir com `../` à frente ou sem `.tex`: quem abre não sabe de onde
 * está a olhar. Compara-se pelo fim e pela base (após `/`, sem `.tex`).
 * (sem função auxiliar — MAX_FUN=256.) */
static int acha_ficheiro(char *nome){
    int ln = strlen(nome);
    if(ln <= 0) return -1;
    for(int i = 0; i < N_FICH; i++){
        char *fn = FICH_NOME[i];
        if(!fn) continue;
        int li = strlen(fn);
        if(li == 0) continue;
        if(li == ln && strcmp(fn, nome) == 0) return i;
        if(li < ln && strcmp(fn, nome + (ln - li)) == 0) return i;
        if(ln < li && strcmp(fn + (li - ln), nome) == 0) return i;
        {
            char *aa = fn;
            char *bb = nome;
            { char *p = fn; while(*p){ if(*p == 47) aa = p + 1; p++; } }
            { char *p = nome; while(*p){ if(*p == 47) bb = p + 1; p++; } }
            int la = 0; while(aa[la]) la++;
            int lb = 0; while(bb[lb]) lb++;
            if(la > 4 && aa[la-4] == 46 && aa[la-3] == 116 && aa[la-2] == 101 && aa[la-1] == 120) la = la - 4;
            if(lb > 4 && bb[lb-4] == 46 && bb[lb-3] == 116 && bb[lb-2] == 101 && bb[lb-1] == 120) lb = lb - 4;
            if(la == lb && la > 0){
                int k = 0;
                while(k < la && aa[k] == bb[k]) k++;
                if(k == la) return i;
            }
        }
    }
    return -1;
}

/* a carta lê pelo NOME: o host já pôs os bytes, não há agulha a guardar.
 * fopen/fclose é streaming; um blob que já está no slot só se aponta.
 * Duas funções, sem ponteiro de saída — o traduz não grava `int*` de local.
 * wasm: miss → __fich_miss (import) — sem função auxiliar (MAX_FUN=256). */
char *ficheiro_end_nome(char *nome){
    int f = acha_ficheiro(nome);
#ifdef TEX_COM_LIBC_WASM
    if(f < 0 && __fich_miss(nome)) f = acha_ficheiro(nome);
#endif
    if(f < 0) return 0;
    return (char*)FICH_END[f];
}
int ficheiro_tam_nome(char *nome){
    int f = acha_ficheiro(nome);
#ifdef TEX_COM_LIBC_WASM
    if(f < 0 && __fich_miss(nome)) f = acha_ficheiro(nome);
#endif
    if(f < 0) return 0;
    return FICH_TAM[f];
}

int fopen(char *nome, char *modo){
    int esc = (modo[0] == 119 || modo[0] == 97);          /* w, a */
    int f = esc ? -1 : acha_ficheiro(nome);
#ifdef TEX_COM_LIBC_WASM
    if(!esc && f < 0 && __fich_miss(nome)) f = acha_ficheiro(nome);
#endif
    if(!esc && f < 0) return 0;
    for(int h = 1; h < MAX_AGULHA; h++){
        if(AG_FICH[h] == 0 && AG_ESC[h] == 0){
            AG_FICH[h] = esc ? -1 : f + 1;
            AG_POS[h] = 0;
            AG_ESC[h] = esc;
            return h;
        }
    }
    return 0;
}

int fclose(int h){ if(h > 0 && h < MAX_AGULHA){ AG_FICH[h] = 0; AG_ESC[h] = 0; } return 0; }

/* o `tmpfile` é uma agulha de escrita sem nome — e é isso que ele sempre foi */
int tmpfile(void){ return fopen("", "w"); }

int fread(char *d, int tam, int quantos, int h){
    if(h <= 0 || h >= MAX_AGULHA) return 0;
    int f = AG_FICH[h] - 1;
    if(f < 0) return 0;
    int n = tam * quantos;
    int resta = FICH_TAM[f] - AG_POS[h];
    if(n > resta) n = resta;
    if(n <= 0) return 0;
    char *o = (char*)(FICH_END[f] + AG_POS[h]);
    for(int i = 0; i < n; i++) d[i] = o[i];
    AG_POS[h] = AG_POS[h] + n;
    return tam > 0 ? n / tam : 0;
}

int fgetc(int h){
    if(h <= 0 || h >= MAX_AGULHA) return -1;
    int f = AG_FICH[h] - 1;
    if(f < 0 || AG_POS[h] >= FICH_TAM[f]) return -1;
    char *o = (char*)(FICH_END[f] + AG_POS[h]);
    AG_POS[h] = AG_POS[h] + 1;
    return o[0] & 255;
}

char *fgets(char *d, int n, int h){
    int i = 0;
    while(i < n - 1){
        int c = fgetc(h);
        if(c < 0) break;
        d[i] = c; i = i + 1;
        if(c == 10) break;
    }
    d[i] = 0;
    return i > 0 ? d : 0;
}

/* garante que a saída cabe em `precisa` bytes — cresce contado, sem dobrar à toa */
static int saida_cabe(int precisa){
    if(precisa <= SAIDA_CAP && SAIDA) return 1;
    int novo = SAIDA_CAP > 0 ? SAIDA_CAP : (1 << 20);   /* começa em 1 MB */
    while(novo < precisa){
        if(novo >= (1 << 27)) return 0;                 /* tecto 128 MB */
        novo = novo * 2;
    }
    char *p = realloc(SAIDA, novo);
    if(!p){ p = malloc(novo); if(!p) return 0;
            if(SAIDA && SAIDA_N > 0){ for(int i = 0; i < SAIDA_N; i++) p[i] = SAIDA[i]; } }
    SAIDA = p; SAIDA_CAP = novo;
    return 1;
}

int fwrite(char *d, int tam, int quantos, int h){
    int n = tam * quantos;
    if(h <= 0 || h >= MAX_AGULHA || n <= 0) return 0;
    if(!AG_ESC[h]) return tam > 0 ? quantos : 0;   /* stdout sem slot: não toca na SAIDA */
    if(!saida_cabe(AG_POS[h] + n)) n = SAIDA_CAP - AG_POS[h];
    if(n <= 0) return 0;
    for(int i = 0; i < n; i++) SAIDA[AG_POS[h] + i] = d[i];
    AG_POS[h] = AG_POS[h] + n;
    if(AG_POS[h] > SAIDA_N) SAIDA_N = AG_POS[h];
    return tam > 0 ? n / tam : 0;
}

int fputc(int c, int h){
    if(h <= 0 || h >= MAX_AGULHA) return -1;
    if(!AG_ESC[h]) return c & 255;                 /* stdout sem slot: descarta */
    if(!saida_cabe(AG_POS[h] + 1)) return -1;
    SAIDA[AG_POS[h]] = c;
    AG_POS[h] = AG_POS[h] + 1;
    if(AG_POS[h] > SAIDA_N) SAIDA_N = AG_POS[h];
    return c & 255;
}

/* `fseek` é a agulha noutro sítio, e mais nada: 0 do princípio, 1 de onde está, 2 do fim */
int fseek(int h, long onde, int donde){
    if(h <= 0 || h >= MAX_AGULHA) return -1;
    int f = AG_FICH[h] - 1;
    int fim = AG_ESC[h] ? SAIDA_N : (f >= 0 ? FICH_TAM[f] : 0);
    long p = onde;
    if(donde == 1) p = AG_POS[h] + onde;
    else if(donde == 2) p = fim + onde;
    if(p < 0) p = 0;
    AG_POS[h] = (int)p;
    return 0;
}
long ftell(int h){ return (h > 0 && h < MAX_AGULHA) ? (long)AG_POS[h] : -1; }
void rewind(int h){ if(h > 0 && h < MAX_AGULHA) AG_POS[h] = 0; }

/* SEM buffer: `d = 0` faz o formatador emitir cada byte pela porta `SINK_H`, em ordem. Não há
 * `char t[4096]` na pilha, nem uma cópia do que já se sabe: escreve-se uma vez, direto. */
int fprintf(int h, char *f, ...){
    va_list ap;
    va_start(ap, f);
    SINK_H = h;
    int n = vsnprintf(0, 0, f, ap);
    va_end(ap);
    return n;
}

int printf(char *f, ...){
    va_list ap;
    va_start(ap, f);
    SINK_H = 1;                                  /* stdout */
    int n = vsnprintf(0, 0, f, ap);
    va_end(ap);
    return n;
}

/* ── O DISCO CRESCE CONTADO, NÃO RESERVADO ───────────────────────────────────────────
 *
 * O Aarão, e é a teoria da medida: «vc ta calculando o infinito de novo». Eu tinha posto
 * um `char MONTE[12M]` — reservar o pior caso, o infinito — e um `realloc` que DOBRAVA,
 * que é aproximar por potências. As duas coisas são o que o papers/medida.tex proíbe:
 * não se reserva o infinito nem se aproxima; CONTA-SE.
 *
 * Aqui o espaço é o DISCO do módulo — a memória linear, o mmap do disco.h — e ele começa
 * onde os dados estáticos acabam e ESTENDE-SE pelo que se escreve, uma página de cada vez.
 * `__disco_cresce` é `memory.grow`: o motor pagina o que se toca e larga o resto, logo o
 * tecto declarado não custa RAM. Um disco que nunca se escreve não pesa — é o vector grande
 * nunca escrito do corpo-estelar. O `free` pontual é vazio; a involução da composição é
 * `volta_compila`: 1 bit, CURSOR ← MARCO. Sem isso o malloc só emite (buraco branco) e
 * o segundo PDF grande parte — Lyapunov λ>0, estado_caos. */
/* os endereços do wasm são de 32 bits: o cursor e a fronteira são `int`, não `long` — um
 * ponteiro é i32, e misturar i64 aqui era pedir uma conversão a cada passo. */

char *malloc(long n){
    int m = (int)n;
    if(m < 1) m = 1;
    m = ((m + 7) / 8) * 8;                      /* alinhado a 8, por divisão inteira */
    if(CURSOR == 0){                            /* a primeira vez: começa após os estáticos */
        CURSOR = __disco_paginas() * 65536;
        DISCO_FIM = CURSOR;
    }
    while(CURSOR + m > DISCO_FIM){              /* falta disco: estende-o, CONTADO */
        int faltam = (CURSOR + m - DISCO_FIM + 65535) / 65536;
        int antes = __disco_cresce(faltam);
        if(antes < 0) return 0;                 /* o motor recusou: acusa, não finge */
        DISCO_FIM = DISCO_FIM + faltam * 65536;
    }
    char *p = (char*)CURSOR;
    CURSOR = CURSOR + m;
    return p;
}
void free(char *p){ }
char *disco_u8(char *nome, long n){ (void)nome; return malloc(n); }

/* os três que faltavam ao tex.c — todos sobre o que já há */
int fputs(char *s, int h){ return fwrite(s, 1, strlen(s), h); }
int putchar(int c){ return fputc(c, 1); }
int puts(char *s){ fputs(s, 1); return fputc(10, 1); }

/* realloc: o tamanho novo é DADO — contado por quem chama, não adivinhado por dobrar. O
 * velho fica imóvel (não há free), o novo é a frente do disco, e copia-se o que o novo
 * comporta; quem chama só cresce, logo o conteúdo antigo cabe todo. Sem dobrar, sem
 * aproximar: a medida entra pronta. */
char *realloc(char *p, long n){
    char *q = malloc(n);
    if(!q) return 0;
    if(p){ int i = 0; while(i < (int)n && (int)(p + i) < CURSOR){ q[i] = p[i]; i = i + 1; } }
    return q;
}

/* strncat: junta no máximo n bytes e termina — o strcat com tecto */
char *strncat(char *d, char *o, int n){
    int i = 0; while(d[i]) i++;
    int j = 0; while(j < n && o[j]){ d[i] = o[j]; i++; j++; }
    d[i] = 0;
    return d;
}
/* memmem: acha o bloco `a` (na bytes) dentro de `h` (nh bytes) — o strstr sem o zero */
char *memmem(char *h, int nh, char *a, int na){
    if(na == 0) return h;
    for(int i = 0; i + na <= nh; i++){
        int j = 0; while(j < na && h[i+j] == a[j]) j++;
        if(j == na) return h + i;
    }
    return 0;
}

/* fflush: no disco não há buffer a esvaziar — o que se escreveu já lá está (MAP_SHARED do
 * disco.h: «o que se escreve JÁ ESTÁ no ficheiro»). Devolve 0, como o sistema. */
int fflush(int h){ return 0; }

/* abs/labs: o valor sem sinal, por um `if` — a mesma conta que a estaca não move */
int abs(int x){ return x < 0 ? -x : x; }
long labs(long x){ return x < 0 ? -x : x; }
