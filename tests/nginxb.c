/* nginxb.c — O NGINX INGERIDO: o `location` casa por PREFIXO MAIS LONGO, e isso É o trie.
 *
 * O Aarão: "inclui o nginx no plugue."
 *
 * E o nginx entra pelo mesmo sítio que os outros: **a marca do nível**. A config é da SEGUNDA
 * família do `caminho.h` — a dos que fazem procurar o fecho, com o JSON e o LaTeX — porque o
 * bloco abre com `{` e só se sabe onde acaba procurando o `}` que o fecha. Nada diz o tamanho
 * à cabeça.
 *
 * MAS O QUE INTERESSA NÃO É A SINTAXE, É A REGRA DE CASAMENTO. O nginx escolhe o `location`
 * pelo **prefixo mais longo** que casa, e isso não é uma heurística dele: é a descida num
 * TRIE — a nossa. O trie é o índice, e descer nele é comparar prefixos. A tabela de precedência
 * inteira do nginx cabe em quatro linhas:
 *
 *      =         exato          ganha de tudo, e a busca para
 *      ^~        prefixo        ganha das expressões regulares
 *      ~  ~*     regex          na ORDEM em que aparecem no ficheiro
 *      (nenhum)  prefixo        o MAIS LONGO ganha, e é o omisso
 *
 * E A ARMADILHA QUE ISTO EXPLICA JÁ NOS CUSTOU UM DEPLOY. O fallback do SPA —
 * `try_files $uri $uri/ /index.html` — devolve **200 + HTML** para um caminho que não existe. O
 * git, a clonar por dumb HTTP, pede um objeto solto que não existe, recebe HTML com estado 200,
 * tenta descomprimi-lo, e cospe *"inflate: data stream error"*. O clone até recupera pelo
 * packfile, mas com dois erros pelo caminho.
 *
 * *O conserto é uma linha de prefixo mais longo:* `location /repo.git/`, que devolvia `=404`
 * enquanto o espelho existiu e devolve `return 404` desde que ele saiu (2026-08-20).
 * Um objeto que não existe passa a devolver **404**, que é o que o git espera. E é a regra do
 * trie que garante que ele ganha do `location /` — não a ordem no ficheiro.
 *
 *   §N1  a config é família 2: o nível abre com { e o fecho procura-se
 *   §N2  o casamento por PREFIXO MAIS LONGO — e é a descida no trie
 *   §N3  a armadilha do SPA: 200+HTML onde tinha de ser 404, e porque o /repo.git/ ganha
 *   §N4  o que o nginx tem que a nossa descida não tem — e o que não tem
 *
 *   cc -O2 -std=c99 -Wall -Wformat nginxb.c -o nginxb && ./nginxb [config]
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "unidade.h"

/* a config a medir: a NOSSA, versionada, e procura-se porque o cwd varia */
static const char *acha_conf(const char *pedido){
    if(pedido){ FILE *f = fopen(pedido, "r"); if(f){ fclose(f); return pedido; } }
    const char *e = getenv("NGINX_CONF");
    if(e && *e){ FILE *f = fopen(e, "r"); if(f){ fclose(f); return e; } }
    static const char *c[] = { "nginx/goldenkingdom.conf", "app/nginx/goldenkingdom.conf",
                               "../app/nginx/goldenkingdom.conf", NULL };
    for(int i = 0; c[i]; i++){ FILE *f = fopen(c[i], "r"); if(f){ fclose(f); return c[i]; } }
    return NULL;
}

/* ================================================================================ */
/* §N1 — a família 2: o nível abre com {, e o fecho procura-se                      */
/* ================================================================================ */
static void secao_N1(const char *conf){
    printf("\n§N1  FAMÍLIA 2 — o nível abre com { e o fecho PROCURA-SE\n\n");

    FILE *f = fopen(conf, "r");
    if(!f){ ok("a config abre", 0); return; }
    char linha[1024];
    int nivel = 0, maxnivel = 0, negativo = 0, nlinhas = 0, blocos = 0, locations = 0;
    while(fgets(linha, sizeof linha, f)){
        nlinhas++;
        char *c = strchr(linha, '#'); if(c) *c = 0;     /* o comentário */
        if(strstr(linha, "location")) locations++;
        for(char *p = linha; *p; p++){
            if(*p == '{'){ nivel++; blocos++; if(nivel > maxnivel) maxnivel = nivel; }
            else if(*p == '}'){ nivel--; if(nivel < 0) negativo = 1; }
        }
    }
    fclose(f);

    printf("     %s\n", conf);
    printf("        %d linhas, %d blocos, %d níveis de profundidade, %d location\n",
           nlinhas, blocos, maxnivel, locations);
    printf("        o nível no fim: %d\n", nivel);

    ok("o balanço fecha em zero — todo { tem o seu }", nivel == 0);
    ok("e nunca desce abaixo de zero — nenhum } antes do seu {", !negativo);
    ok("há blocos e locations para medir", blocos > 0 && locations > 0);

    /* E A DIFERENÇA PARA A FAMÍLIA 1, que é o ponto: em nenhum sítio deste ficheiro está
     * escrito quantos bytes tem um bloco. Para saber onde ele acaba é preciso LER até ao fecho.
     * No WASM, no PACK e no SSH o tamanho vem antes, e descer é somar. */
    printf("     em nenhuma linha está escrito o TAMANHO de um bloco: descer é procurar o fecho.\n");

    conclui("a marca do nível é a chaveta, e a chaveta não diz o tamanho — família 2, como o JSON.");
}

/* ================================================================================ */
/* §N2 — o prefixo mais longo É o trie                                              */
/* ================================================================================ */
/* A regra do nginx, implementada aqui e confrontada com casos onde a resposta é decidível
 * sem ambiguidade. Não é uma tabela minha: é uma FUNÇÃO, e os casos são entradas dela. */
typedef struct { const char *pref; int prioritario; } Loc;   /* prioritario = ^~ */

static int casa_mais_longo(const Loc *L, int n, const char *uri){
    int melhor = -1; size_t maislongo = 0;
    for(int i = 0; i < n; i++){
        size_t p = strlen(L[i].pref);
        if(strncmp(uri, L[i].pref, p) == 0 && p >= maislongo){ maislongo = p; melhor = i; }
    }
    return melhor;
}

static void secao_N2(void){
    printf("\n§N2  O PREFIXO MAIS LONGO — e é a descida no TRIE, não uma heurística\n\n");

    Loc L[] = {
        { "/",              0 },
        { "/assets/",       0 },
        { "/repo.git/",     0 },
        { "/.well-known/acme-challenge/", 0 },
    };
    struct { const char *uri; int esperado; const char *porque; } casos[] = {
        { "/index.html",                          0, "só o / casa" },
        { "/assets/index-ABC123.js",              1, "/assets/ é mais longo que /" },
        { "/repo.git/HEAD",                       2, "/repo.git/ é mais longo que /" },
        { "/repo.git/objects/ab/cdef",            2, "e continua a ser, em profundidade" },
        { "/.well-known/acme-challenge/xyz",      3, "o mais longo de todos" },
        { "/repo.gitextra",                       0, "NÃO casa /repo.git/ — falta a barra" },
        { "/assets",                              0, "sem a barra final não casa /assets/" },
    };
    printf("        URI                                    escolhe          porquê\n");
    int erros = 0;
    for(int i = 0; i < 7; i++){
        int r = casa_mais_longo(L, 4, casos[i].uri);
        if(r != casos[i].esperado) erros++;
        printf("        %-38s %-16s %s\n", casos[i].uri,
               r >= 0 ? L[r].pref : "(nenhum)", casos[i].porque);
    }
    ok("os 7 casos casam pelo prefixo mais longo — inclusive os dois que NÃO casam", erros == 0);

    /* E A ORDEM NO FICHEIRO NÃO CONTA, que é o que distingue o trie de uma lista de regras.
     * Baralha-se a tabela e a resposta tem de ser a mesma. */
    Loc B[] = { { "/repo.git/", 0 }, { "/", 0 }, { "/.well-known/acme-challenge/", 0 }, { "/assets/", 0 } };
    int discorda = 0;
    for(int i = 0; i < 7; i++){
        int a = casa_mais_longo(L, 4, casos[i].uri);
        int b = casa_mais_longo(B, 4, casos[i].uri);
        if(a < 0 || b < 0){ if(a != b) discorda++; continue; }
        if(strcmp(L[a].pref, B[b].pref) != 0) discorda++;
    }
    printf("     com a tabela BARALHADA, as 7 respostas mudam em %d casos\n", discorda);
    ok("a ordem no ficheiro não conta — é trie, não lista de regras", discorda == 0);

    conclui("descer num trie é comparar prefixos, e o nginx faz exatamente isso a cada pedido.");
}

/* ================================================================================ */
/* §N3 — a armadilha do SPA, e porque o /repo.git/ ganha                            */
/* ================================================================================ */
static void secao_N3(const char *conf){
    printf("\n§N3  A ARMADILHA DO SPA: 200+HTML onde tinha de ser 404\n\n");

    /* O que aconteceu, e está anotado na própria config: o fallback do SPA responde 200 com o
     * index.html a QUALQUER caminho que não exista. O git, a clonar por dumb HTTP, pede um
     * objeto solto, recebe HTML, tenta inflar, e cospe "inflate: data stream error". */
    printf("     o fallback:   location /        try_files $uri $uri/ /index.html   → 200 + HTML\n");
    printf("     o conserto:   location /repo.git/   return 404                     → 404\n");
    printf("     (era try_files $uri =404 enquanto o espelho existiu: o mesmo 404, servindo)\n\n");

    /* SEM a linha do repo, o pedido cai no fallback. COM ela, o prefixo mais longo ganha. */
    Loc sem[] = { { "/", 0 } };
    Loc com[] = { { "/", 0 }, { "/repo.git/", 0 } };
    const char *pedido = "/repo.git/objects/ab/cdef0123456789";

    int a = casa_mais_longo(sem, 1, pedido);
    int b = casa_mais_longo(com, 2, pedido);
    printf("        sem o location /repo.git/ →  %s   (o fallback: 200 + index.html)\n", sem[a].pref);
    printf("        com o location /repo.git/ →  %s   (404 seco, e não HTML disfarçado)\n", com[b].pref);

    ok("sem a linha, o objeto inexistente cai no fallback do SPA", a == 0 && !strcmp(sem[a].pref, "/"));
    ok("com a linha, o prefixo mais longo ganha e o pedido recebe 404",
       b == 1 && !strcmp(com[b].pref, "/repo.git/"));

    /* E A CONFIG REAL TEM DE TER AS TRÊS PEÇAS. Mudaram de sinal em 2026-08-20: o espelho
     * saiu da Patria (o repositório é privado), e o mesmo prefixo mais longo que servia o
     * clone passou a FECHÁ-LO. O que se confere é o fecho, e continua a ser o ficheiro que
     * responde — não a minha lembrança dele. */
    FILE *f = fopen(conf, "r");
    if(!f){ ok("a config real abre para conferência", 0); return; }
    char tudo[16384] = {0};
    size_t n = fread(tudo, 1, sizeof tudo - 1, f);
    tudo[n] = 0;
    fclose(f);

    int tem_repo   = strstr(tudo, "location /repo.git/") != NULL;
    int tem_return = strstr(tudo, "return 404") != NULL;
    int tem_alias  = strstr(tudo, "alias /var/www/goldenkingdom/repo.git") != NULL;
    printf("\n        na config real:  location /repo.git/  %s   return 404  %s   alias  %s\n",
           tem_repo?"sim":"NÃO", tem_return?"sim":"NÃO", tem_alias?"AINDA":"nenhum");
    ok("a config em produção tem o location /repo.git/ — a armadilha está tapada", tem_repo);
    ok("e o que ele devolve é 404 por return, não o fallback do SPA", tem_return);
    /* A terceira peça é uma AUSÊNCIA, e é o que separa fechar de servir: enquanto houvesse
     * alias, o 404 seria só dos objetos que faltam — com ele fora, não há nada a servir. */
    ok("e nenhum alias para o espelho — não é um clone partido, é um clone que não existe",
       !tem_alias);

    int tem_canal = strstr(tudo, "location = /canal") != NULL;
    int tem_proxy = strstr(tudo, "proxy_pass http://127.0.0.1:47314") != NULL;
    int tem_alias_canal = strstr(tudo, "alias /canal") != NULL;
    printf("        location = /canal  %s   proxy 47314  %s   alias /canal  %s\n",
           tem_canal?"sim":"NÃO", tem_proxy?"sim":"NÃO", tem_alias_canal?"AINDA":"nenhum");
    ok("location = /canal — o SPA não come o Upgrade WSS", tem_canal);
    ok("o canal é proxy ao loopback, não alias de ficheiro", tem_proxy && !tem_alias_canal);

    conclui("a mesma regra de casamento serviu o clone e agora fecha-o: o que mudou foi o corpo do bloco.");
}

/* ================================================================================ */
static void secao_N4(void){
    printf("\n§N4  O QUE ELE TEM QUE NÓS NÃO TEMOS — e o contrário\n\n");

    struct { const char *o; const char *nosso; int temos; } M[] = {
        { "location por prefixo mais longo",  "a descida no trie",              1 },
        { "location = (exato)",               "a folha do trie",                1 },
        { "try_files: cadeia de tentativas",  "LOAD com omisso",                1 },
        { "alias: reescrita do caminho",      "o slot é o endereço, sem alias", 0 },
        { "location ~ (regex, por ordem)",    "— não temos regex",              0 },
        { "gzip por tipo de conteúdo",        "— não temos compressão",         0 },
    };
    printf("        no nginx                            aqui                            temos\n");
    int sim = 0, nao = 0;
    for(int i = 0; i < 6; i++){
        printf("        %-35s %-31s %s\n", M[i].o, M[i].nosso, M[i].temos ? "sim" : "NÃO");
        if(M[i].temos) sim++; else nao++;
    }
    ok("a tabela tem linhas dos dois lados — se todas batessem, não era comparação",
       sim > 0 && nao > 0);

    printf("\n     E A DIFERENÇA QUE PESA: o regex quebra o trie. Com `location ~` a escolha deixa\n");
    printf("     de ser por prefixo e passa a ser POR ORDEM NO FICHEIRO — deixa de haver estrutura\n");
    printf("     e passa a haver uma lista. É por isso que a nossa config não usa nenhum, e é por\n");
    printf("     isso que a ordem dos blocos dela não importa.\n");

    conclui("enquanto for só prefixo, é trie; um regex e passa a ser uma lista lida de cima a baixo.");
}

/* ================================================================================ */
int main(int argc, char **argv){
    const char *conf = acha_conf(argc > 1 ? argv[1] : NULL);
    if(!conf){
        printf("NAO MEDIU — sem a config (app/nginx/goldenkingdom.conf ou NGINX_CONF=<caminho>).\n");
        printf("Um medidor sem o objeto a medir não passa nem falha: não mediu.\n");
        return 2;
    }

    puts("nginxb.c — O NGINX INGERIDO: o location casa por prefixo mais longo, e isso É o trie");
    puts("===================================================================================");
    printf("  a config: %s\n", conf);
    puts("");
    puts("  A sintaxe é família 2 (o fecho procura-se). Mas o que interessa é a REGRA DE");
    puts("  CASAMENTO: prefixo mais longo — que é a descida no trie, e não uma heurística.");

    secao_N1(conf); secao_N2(); secao_N3(conf); secao_N4();

    printf("\n===================================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  E O QUE ISTO GUARDA: a config em produção tem as três peças que fecham o espelho");
        puts("  sem o deixar a meio — o location /repo.git/, o return 404 e nenhum alias. Não é");
        puts("  lembrança minha: o medidor lê o ficheiro. Se alguém repuser o alias, ou tirar o");
        puts("  bloco e deixar o fallback do SPA responder 200 + HTML, a bateria fica vermelha.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
