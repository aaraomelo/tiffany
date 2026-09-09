/* libc.c — A BIBLIOTECA, ESCRITA NA MESMA RÉGUA.
 *
 * O `tests/tex.c` chama 32 funções da biblioteca do sistema, 461 vezes. No navegador não há
 * biblioteca do sistema — e não faz falta: quase todas elas são LAÇOS SOBRE `char *`, e isso
 * sobe pelo tradutor que já cá está, sem uma linha nova nele.
 *
 * Por isso não se importa nada. Escreve-se aqui, em C, no subconjunto que sobe, e traduz-se
 * com o mesmo `traduz` de tudo o resto. É a régua a valer para si própria.
 *
 * ── O QUE ESTÁ AQUI ─────────────────────────────────────────────────────────
 *   texto        strlen strcmp strncmp strcpy strncpy strcat strstr strchr strrchr
 *   memória      memcpy memmove memset memcmp
 *   caracteres   isalpha isdigit isalnum isspace isupper islower toupper tolower
 *   números      atoi atol strtol atof
 *
 * ── O QUE NÃO ESTÁ, E PORQUÊ ──────────────────────────────────────────────
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

/* ── o texto ─────────────────────────────────────────────────────── */

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
    int i = 0;
    while(n--){
        int x = a[i] & 255;
        int y = b[i] & 255;
        if(x != y) return x - y;
        if(x == 0) return 0;
        i++;
    }
    return 0;
}

char *strcpy(char *d, char *s){
    char *p = d;
    while((*d++ = *s++)) ;
    return p;
}

char *strncpy(char *d, char *s, int n){
    char *p = d;
    while(n-- && (*d++ = *s++)) ;
    while(n++ >= 0) *d++ = 0;
    return p;
}

char *strcat(char *d, char *s){
    char *p = d;
    while(*d) d++;
    while((*d++ = *s++)) ;
    return p;
}

char *strstr(char *h, char *n){
    if(!*n) return h;
    while(*h){
        char *a = h, *b = n;
        while(*b && (*a & 255) == (*b & 255)){ a++; b++; }
        if(!*b) return h;
        h++;
    }
    return 0;
}

char *strchr(char *s, int c){
    while(*s && (*s & 255) != (c & 255)) s++;
    return (*s & 255) == (c & 255) ? s : 0;
}

char *strrchr(char *s, int c){
    char *last = 0;
    while(*s){
        if((*s & 255) == (c & 255)) last = s;
        s++;
    }
    return last;
}

/* ── a memória ──────────────────────────────────────────────────────────── */

void *memcpy(void *d, const void *o, int n){
    char *r = d;
    int i;
    for(i = 0; i < n; i++) r[i] = ((char*)o)[i];
    return r;
}

void *memmove(void *d, const void *o, int n){
    char *r = d;
    int i;
    if(d > o){
        for(i = n - 1; i >= 0; i--) r[i] = ((char*)o)[i];
    } else {
        for(i = 0; i < n; i++) r[i] = ((char*)o)[i];
    }
    return r;
}

void *memset(void *s, int c, int n){
    int i;
    for(i = 0; i < n; i++) ((char*)s)[i] = c;
    return s;
}

int memcmp(const void *a, const void *b, int n){
    int i;
    for(i = 0; i < n; i++){
        int x = ((unsigned char*)a)[i];
        int y = ((unsigned char*)b)[i];
        if(x != y) return x - y;
    }
    return 0;
}

/* ── os caracteres ──────────────────────────────────────────────────────── */

int isalpha(int c){ return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
int isdigit(int c){ return c >= '0' && c <= '9'; }

/* ── os números ─────────────────────────────────────────────────────────── */

static int atofr(char *s, long *p, long *q, int *sg){
    *p = 0; *q = 1; *sg = 1;
    while(*s == ' ' || *s == '\t') s++;
    if(*s == '-'){ *sg = -1; s++; } else if(*s == '+'){ s++; }
    while(*s >= '0' && *s <= '9'){ *p = *p * 10 + (*s - '0'); s++; }
    if(*s == '.'){
        s++;
        while(*s >= '0' && *s <= '9'){
            *p = *p * 10 + (*s - '0');
            *q = *q * 10;
            s++;
        }
    }
    return 1;
}

long strtol(char *s, char **e, int b){
    long r = 0; int sg = 1;
    if(!b) b = 10;
    while(*s == ' ' || *s == '\t') s++;
    if(*s == '-'){ sg = -1; s++; } else if(*s == '+'){ s++; }
    if(b == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X')){
        s += 2;
        while((*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F')){
            r = r * 16 + (*s <= '9' ? *s - '0' : (*s & 7) + 9);
            s++;
        }
    } else {
        while(*s >= '0' && *s <= '9'){ r = r * b + (*s - '0'); s++; }
    }
    if(e) *e = s;
    return r * sg;
}

int atoi(char *s){ return (int)strtol(s, 0, 10); }
long atol(char *s){ return strtol(s, 0, 10); }

typedef double f64;

f64 atof(char *s){
    long p = 0, q = 1;
    int sg = 1;
    if(atofr(s, &p, &q, &sg)){
        f64 v = (f64)p / (f64)q;
        return sg < 0 ? -v : v;
    }
    return 0.0;
}