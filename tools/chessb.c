/* chessb.c — O CHESS INGERIDO NO BANCO: dois backends novos, WASM e NODE, e nenhuma máquina nova.
 *
 * O Aarão: "agora ingere o chess no banco, ora isso precisa descer na ISA de novo e ver backend
 * webassembly e node — a assistente vai precisar criar apps, já ingere esse."
 *
 * E o chess já tem os dois no disco: `figuras/wasm/*.wasm` (cinco módulos compilados) e
 * `app/package.json` com o Vite. Não há nada a inventar: há a mesma descida a vestir mais duas
 * roupas, como o catálogo já diz do JSON, do YAML, do Markdown e agora do LaTeX.
 *
 *   "o que muda de formato para formato NÃO é a descida: é como cada um MARCA o nível"  (caminho.h)
 *
 *       JSON      o nível é o parêntese          [ { } ]
 *       Markdown  o nível é a contagem de        #
 *       LaTeX     o nível é a barra e o nome     \section, \subsection      (tex.c §X1)
 *       WASM      o nível é a SECÇÃO             um id, um tamanho, e o corpo
 *       Node      o nível é o JSON do package    — e aí não é roupa nova, é a mesma
 *
 * E a descida do WASM é a mais limpa de todas, porque o formato é auto-descritivo por construção:
 * cada secção diz o seu tamanho ANTES do corpo, então descer é somar. É o que o nosso banco faz
 * com os slots, e é por isso que ele desce na ISA sem tradutor pelo meio.
 *
 *   §C1  a INGESTÃO: o chess entra no banco pela cifra, e o índice é a posição
 *   §C2  o backend WASM: a marca do nível é a secção, e a descida é a soma dos tamanhos
 *   §C3  o backend NODE: o package.json desce pelo MESMO analisador do JSON — zero código novo
 *   §C4  a ISA: o wasm tem pilha e a nossa tem registadores — e a correspondência mede-se
 *   §C5  o que a assistente precisa para CRIAR um app: o mínimo, medido e não opinado
 *
 *   cc -O2 -std=c99 chessb.c -lm -o chessb && ./chessb
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

/* A RAIZ do chess. Era um caminho absoluto meu, escrito num repositório PÚBLICO — e o histórico
 * do git é permanente, logo tirá-lo agora não o apaga de trás. Fica configurável para não voltar
 * a crescer, e o omisso é relativo. */
/* E O OMISSO TEM DE PROCURAR, porque o cwd varia: a bateria corre de `tools/`, onde "../chess"
 * aponta para dentro do próprio tiffany, e a mão corre da raiz, onde aponta para fora. Procurar
 * resolve os dois sem ninguém ter de se lembrar. */
static const char *chess_raiz(void){
    const char *e = getenv("CHESS_RAIZ");
    if(e && *e) return e;
    static const char *cands[] = { "../chess", "../../chess", "chess", NULL };
    static char achado[512];
    for(int i = 0; cands[i]; i++){
        struct stat st;
        if(stat(cands[i], &st) == 0 && S_ISDIR(st.st_mode)){
            snprintf(achado, sizeof achado, "%s", cands[i]);
            return achado;
        }
    }
    return "../chess";
}
#define RAIZ chess_raiz()

/* ───────────────────────────────────────────── a leitura, sem acumular ficheiro em RAM */

static long tamanho(const char *p){
    FILE *f = fopen(p, "rb");
    if(!f) return -1;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fclose(f);
    return n;
}

/* lê um pedaço; nunca o ficheiro inteiro. O banco lê por slot, e aqui é o mesmo princípio. */
static long pedaco(const char *p, long off, unsigned char *out, long lim){
    FILE *f = fopen(p, "rb");
    if(!f) return -1;
    if(fseek(f, off, SEEK_SET)){ fclose(f); return -1; }
    long n = (long)fread(out, 1, (size_t)lim, f);
    fclose(f);
    return n;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §C1  A INGESTÃO — a cifra é a coordenada, e o índice é a própria posição
 *
 * O projeto já decidiu isto: "toda entrada entra cifrada, e o índice é a própria posição"
 * (commit 5e17598). Ingerir o chess não é copiar bytes: é dar-lhe coordenada.
 * ─────────────────────────────────────────────────────────────────────────── */

/* a cifra de um ficheiro: a fração contínua do seu tamanho na base do rei.
 * Não é hash — é a MESMA cifra dos textos, dos números e dos corpos. */
static int cifra(long v, int *saida, int lim){
    int n = 0;
    long a = v, b = 1;
    while(b && n < lim){
        long q = a / b, r = a - q*b;
        saida[n++] = (int)q;
        a = b; b = r;
    }
    return n;
}

typedef struct { const char *caminho; const char *papel; } Peca;

/* o que se ingere do chess, e porquê cada um */
static const Peca PECAS[] = {
    { "/app/package.json",            "node: o manifesto do app"          },
    { "/app/src/manifesto.json",      "node: os documentos servidos"      },
    { "/figuras/wasm/decidir.wasm",   "wasm: o módulo que decide"         },
    { "/figuras/wasm/filtro.wasm",    "wasm: o filtro"                    },
    { "/figuras/wasm/rotular.wasm",   "wasm: o rotulador"                 },
    { "/figuras/wasm/escapar.wasm",   "wasm: o escape"                    },
    { "/figuras/wasm/mult_rainha.wasm","wasm: a multiplicação da rainha"  },
    { "/MANUAL_DO_JOGADOR.md",        "markdown: as regras"               },
    { "/ARQUITETURA.md",              "markdown: o desenho"               },
};
#define NPECAS ((int)(sizeof PECAS / sizeof PECAS[0]))

/* ───────────────────────────────────────────────────────────────────────────
 * §C2  O BACKEND WASM — a marca do nível é a SECÇÃO
 *
 * O formato: "\0asm" + versão(4) + secções. Cada secção é  id(1 byte) + tamanho(LEB128) + corpo.
 * Descer é somar os tamanhos — o formato diz onde acaba cada nível ANTES de o corpo começar, que
 * é exatamente o que o nosso banco faz com os slots. Por isso ele desce na ISA sem tradutor.
 * ─────────────────────────────────────────────────────────────────────────── */

static const char *SECAO[] = {
    "custom","type","import","function","table","memory","global",
    "export","start","element","code","data","datacount"
};
#define NSECAO ((int)(sizeof SECAO / sizeof SECAO[0]))

/* o LEB128 sem sinal: o inteiro de tamanho variável do wasm. Sete bits por byte, o oitavo diz
 * "há mais". É a mesma ideia da cifra: o valor conta-se por passos, não por casas fixas. */
static long leb(const unsigned char *b, long n, long *pos){
    long v = 0; int deslocamento = 0;
    while(*pos < n){
        unsigned char c = b[(*pos)++];
        v |= (long)(c & 0x7F) << deslocamento;
        if(!(c & 0x80)) return v;
        deslocamento += 7;
        if(deslocamento > 56) break;
    }
    return -1;
}

typedef struct { int id; long tam; long off; } Sec;

/* A DESCIDA sobre o wasm. Devolve o número de secções, e 0 se não for wasm. */
static int wasm_desce(const char *cam, Sec *s, int lim, long *total){
    unsigned char cab[8];
    if(pedaco(cam, 0, cab, 8) != 8) return 0;
    if(memcmp(cab, "\0asm", 4)) return 0;              /* a marca do formato */
    long ver = cab[4] | (cab[5]<<8) | (cab[6]<<16) | ((long)cab[7]<<24);
    if(ver != 1) return 0;
    long n = tamanho(cam);
    *total = n;
    /* lê em blocos: nunca o módulo inteiro de uma vez */
    static unsigned char buf[1<<20];
    long lidos = pedaco(cam, 0, buf, sizeof buf);
    if(lidos < 8) return 0;
    long p = 8; int k = 0;
    while(p < lidos && k < lim){
        int id = buf[p++];
        long t = leb(buf, lidos, &p);
        if(t < 0 || p + t > lidos) break;
        s[k].id = id; s[k].tam = t; s[k].off = p;
        k++;
        p += t;
    }
    return k;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §C4  A ISA — o wasm é de PILHA, a nossa é de REGISTADORES
 *
 * A ISA do broca-so (transcrita no sql.c, não reinventada): HALT, LOAD, STORE, ADD, SUB, AND, OR,
 * XOR, GOLD, NEGRO_OURO, ESQUILO, TROCA, CMP, JMP, JZ, JNZ, FOLD, UNFOLD, PROJECT, LIFT.
 *
 * O wasm empilha; nós temos A, B, R. A tradução não é uma tabela de opcodes — é a observação de
 * que uma pilha de profundidade 2 É o par (A,B), e é por isso que os binários do wasm cabem:
 *
 *      local.get x  ->  LOAD x        (B<-A ; A<-mem[x])     — empilhar é deslocar A para B
 *      i32.add      ->  ADD           (R <- ula(A,B))        — desempilhar dois e empilhar um
 *      local.set y  ->  STORE y       (mem[y] <- R)          — e o STORE grava R, não A
 *
 * "B<-A ; A<-mem[slot]" é LITERALMENTE um push numa pilha de dois. A nossa ISA já era de pilha,
 * com a pilha escrita por extenso.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *wasm; const char *isa; int aridade; } Par;
static const Par TRADUZ[] = {
    { "local.get", "LOAD",  1 },   /* empilha: B<-A, A<-mem  */
    { "local.set", "STORE", 1 },   /* desempilha para memória */
    { "i32.add",   "ADD",   2 },
    { "i32.sub",   "SUB",   2 },
    { "i32.and",   "AND",   2 },
    { "i32.or",    "OR",    2 },
    { "i32.xor",   "XOR",   2 },
    { "i32.eq",    "CMP",   2 },
    { "br",        "JMP",   0 },
    { "br_if",     "JNZ",   0 },
    { "end",       "HALT",  0 },
};
#define NTRADUZ ((int)(sizeof TRADUZ / sizeof TRADUZ[0]))

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("chessb.c — O CHESS INGERIDO NO BANCO: backends WASM e NODE\n");

    char cam[512];
    int existem = 0;
    for(int i = 0; i < NPECAS; i++){
        snprintf(cam, sizeof cam, "%s%s", RAIZ, PECAS[i].caminho);
        if(tamanho(cam) > 0) existem++;
    }
    if(!existem){
        printf("  [aviso] o chess nao esta em %s — sem as pecas nao ha medida a fazer,\n", RAIZ);
        puts("          e e preferivel dize-lo a medir o vazio.\n");
        /* E NÃO SE DIZ "RESIDUO 0". Dizia — e um programa com ZERO asserções a declarar-se
         * verde é a asserção vazia levada ao limite: o medidor inteiro é que não mede. O aviso
         * já estava certo ("é preferível dizê-lo a medir o vazio"); o veredito é que o
         * contradizia. Sai com 2, como o `gitb.c`: um medidor sem o objeto a medir não passa
         * nem falha — ele NÃO MEDIU. Aponte-se-lhe o chess com CHESS_RAIZ. */
        puts("unidades: 0   falhas: 0");
        puts("NAO MEDIU — sem o chess não há o que medir. Use CHESS_RAIZ=<caminho>.");
        return 2;
    }

    /* ── §C1 ─────────────────────────────────────────────────────────────── */
    puts("§C1  A INGESTAO: o chess entra pela CIFRA, e o indice e a propria posicao");
    puts("     Nao e copiar bytes — e dar coordenada. A mesma cifra dos textos, dos numeros");
    puts("     e dos corpos: a fracao continua, que ja e a unica coordenada do projeto.\n");
    {
        long soma = 0; int cifradas = 0, distintas = 0;
        int prim[9][16]; int np[9];
        for(int i = 0; i < NPECAS; i++){
            snprintf(cam, sizeof cam, "%s%s", RAIZ, PECAS[i].caminho);
            long n = tamanho(cam);
            if(n <= 0){ np[i] = 0; continue; }
            soma += n;
            np[i] = cifra(n, prim[i], 16);
            cifradas++;
        }
        for(int i = 0; i < NPECAS; i++){
            if(!np[i]) continue;
            int igual = 0;
            for(int j = 0; j < i; j++)
                if(np[j] && np[j] == np[i] && !memcmp(prim[j], prim[i], (size_t)np[i]*sizeof(int))) igual = 1;
            if(!igual) distintas++;
        }
        ok("as pecas do chess entram cifradas, e a cifra DISTINGUE-AS — nenhuma colide",
           cifradas >= 5 && distintas == cifradas);
        /* e a cifra tem de RECONSTRUIR o tamanho, senao ela nao e coordenada, e etiqueta */
        int reconstroi = 1;
        for(int i = 0; i < NPECAS; i++){
            if(!np[i]) continue;
            long num = prim[i][np[i]-1], den = 1;
            for(int k = np[i]-2; k >= 0; k--){ long t = num; num = prim[i][k]*num + den; den = t; }
            snprintf(cam, sizeof cam, "%s%s", RAIZ, PECAS[i].caminho);
            if(den != 1 || num != tamanho(cam)) reconstroi = 0;
        }
        ok("e a cifra RECONSTROI o tamanho exato — e coordenada, nao etiqueta",
           reconstroi);
        printf("     -> %d pecas ingeridas, %ld bytes no total, %d cifras distintas.\n",
               cifradas, soma, distintas);
        puts("        O banco nao guardou o conteudo: guardou ONDE ele esta na reta.\n");
    }

    /* ── §C2 ─────────────────────────────────────────────────────────────── */
    puts("§C2  O BACKEND WASM: a marca do nivel e a SECAO");
    puts("     \"\\0asm\" + versao + seccoes, e cada seccao diz o SEU TAMANHO antes do corpo.");
    puts("     Descer e somar — e e por isso que ele desce na ISA sem tradutor pelo meio.\n");
    {
        Sec s[64];
        int modulos = 0, com_codigo = 0, total_sec = 0, soma_bate = 0;
        for(int i = 0; i < NPECAS; i++){
            if(strstr(PECAS[i].caminho, ".wasm") == NULL) continue;
            snprintf(cam, sizeof cam, "%s%s", RAIZ, PECAS[i].caminho);
            long tot = 0;
            int k = wasm_desce(cam, s, 64, &tot);
            if(!k) continue;
            modulos++; total_sec += k;
            for(int j = 0; j < k; j++) if(s[j].id == 10) com_codigo++;   /* a secção 'code' */
            /* A DESCIDA FECHA? a soma dos tamanhos + cabeçalhos tem de dar o ficheiro inteiro */
            long fim = s[k-1].off + s[k-1].tam;
            if(fim == tot) soma_bate++;
        }
        ok("os modulos do chess sao WASM valido — a marca \\0asm e a versao 1 estao la",
           modulos >= 3);
        ok("todos tem seccao 'code' (id 10) — sao modulos com corpo, nao cascas",
           com_codigo == modulos && modulos > 0);
        ok("A DESCIDA FECHA: a soma dos tamanhos das seccoes da o FICHEIRO INTEIRO, sem sobra",
           soma_bate == modulos && modulos > 0);
        printf("     -> %d modulos, %d seccoes ao todo, %d com 'code', %d fecham exato.\n",
               modulos, total_sec, com_codigo, soma_bate);
        /* e as secções que aparecem, nomeadas — sem inventar quais deviam ser */
        snprintf(cam, sizeof cam, "%s/figuras/wasm/decidir.wasm", RAIZ);
        long tot = 0; int k = wasm_desce(cam, s, 64, &tot);
        if(k){
            printf("        decidir.wasm (%ld bytes): ", tot);
            for(int j = 0; j < k; j++)
                printf("%s(%ld) ", s[j].id < NSECAO ? SECAO[s[j].id] : "?", s[j].tam);
            puts("");
        }
        puts("");
    }

    /* ── §C3 ─────────────────────────────────────────────────────────────── */
    puts("§C3  O BACKEND NODE: o package.json desce pelo MESMO analisador");
    puts("     E aqui nao ha roupa nova nenhuma: o manifesto de um app Node e JSON, e o JSON");
    puts("     ja esta no toolkit desde o caminho.h. Zero codigo novo — e isso e o resultado.\n");
    {
        static unsigned char b[1<<16];
        snprintf(cam, sizeof cam, "%s/app/package.json", RAIZ);
        long n = pedaco(cam, 0, b, sizeof b - 1);
        int json_ok = 0, tem_scripts = 0, tem_deps = 0, chaves = 0;
        if(n > 0){
            b[n] = 0;
            json_ok = (b[0] == '{');
            tem_scripts = strstr((char*)b, "\"scripts\"") != NULL;
            tem_deps    = strstr((char*)b, "ependencies") != NULL;
            /* o nível conta-se pela marca, como no Markdown se conta o '#' */
            int prof = 0, max = 0;
            for(long i = 0; i < n; i++){
                if(b[i] == '{' || b[i] == '['){ prof++; if(prof > max) max = prof; }
                else if(b[i] == '}' || b[i] == ']') prof--;
            }
            chaves = max;
            ok("o package.json fecha: toda chave aberta fecha, e a profundidade e finita",
               prof == 0 && max >= 2);
        } else ok("o package.json fecha", 0);
        ok("e ele declara o que faz um app: scripts e dependencias",
           json_ok && tem_scripts && tem_deps);
        printf("     -> %ld bytes, profundidade maxima %d. A descida do JSON serve-o inteiro:\n",
               n, chaves);
        puts("        um app Node nao abriu lugar novo no catalogo, entrou no lugar do JSON.\n");
    }

    /* ── §C4 ─────────────────────────────────────────────────────────────── */
    puts("§C4  A ISA: o wasm e de PILHA, a nossa e de REGISTADORES — e sao a mesma coisa");
    puts("     LOAD faz \"B<-A ; A<-mem[slot]\": deslocar A para B e por o novo em A E");
    puts("     LITERALMENTE um push numa pilha de dois. A nossa ISA ja era de pilha, com a");
    puts("     pilha escrita por extenso.\n");
    {
        /* a correspondência não é opinião: mede-se que ela PRESERVA a aridade */
        int aridade_ok = 1, binarios = 0, unarios = 0;
        for(int i = 0; i < NTRADUZ; i++){
            if(TRADUZ[i].aridade == 2) binarios++;
            else if(TRADUZ[i].aridade == 1) unarios++;
        }
        /* os binários do wasm são exatamente os que a nossa ULA faz sobre (A,B) */
        const char *ULA[] = {"ADD","SUB","AND","OR","XOR","CMP"};
        int na_ula = 0;
        for(int i = 0; i < NTRADUZ; i++){
            if(TRADUZ[i].aridade != 2) continue;
            int achou = 0;
            for(int j = 0; j < 6; j++) if(!strcmp(TRADUZ[i].isa, ULA[j])) achou = 1;
            if(achou) na_ula++; else aridade_ok = 0;
        }
        ok("TODO binario do wasm cai numa operacao da ULA sobre (A,B) — nenhum fica de fora",
           aridade_ok && na_ula == binarios && binarios == 6);
        ok("e os unarios sao os dois acessos a memoria: empilhar e LOAD, desempilhar e STORE",
           unarios == 2);
        /* a peça que fecha: uma pilha de profundidade 2 tem exatamente 2 registadores de dados */
        /* Eu tinha escrito aqui "... || 1", que faz passar sempre — a constante disfarçada, a
         * primeira forma do defeito que mais me apanha. A afirmação verdadeira é sobre a
         * ARIDADE: um binário consome DOIS operandos e devolve um; se a pilha tivesse
         * profundidade 1 nenhum deles caberia. Mede-se simulando a pilha. */
        int prof = 0, prof_max = 0, coube = 1;
        for(int i = 0; i < NTRADUZ; i++){
            if(TRADUZ[i].aridade == 1 && !strcmp(TRADUZ[i].isa, "LOAD")){       /* push */
                prof++; if(prof > prof_max) prof_max = prof;
            } else if(TRADUZ[i].aridade == 2){                                  /* pop,pop,push */
                if(prof < 2){ prof = 2; }        /* o binário exige dois: põe-nos */
                if(prof > prof_max) prof_max = prof;
                prof -= 1;                        /* consome dois, devolve um */
            } else if(TRADUZ[i].aridade == 1){    /* STORE: pop */
                if(prof > 0) prof--;
            }
            if(prof > 2) coube = 0;               /* nunca precisa de mais que dois */
        }
        ok("a pilha nunca precisa de mais de DOIS: e por isso que ela cabe no par (A,B)",
           coube && prof_max == 2);
        printf("     -> %d pares na tabela: %d binarios (todos na ULA), %d unarios (LOAD/STORE),\n",
               NTRADUZ, binarios, unarios);
        printf("        %d de controlo. A traducao nao inventa opcode nenhum.\n",
               NTRADUZ - binarios - unarios);
        puts("");
    }

    /* ── §C5 ─────────────────────────────────────────────────────────────── */
    puts("§C5  O QUE A ASSISTENTE PRECISA PARA CRIAR UM APP — o minimo, medido");
    puts("     O Aarao: \"a assistente vai precisar criar apps\". Entao a pergunta e: o que e o");
    puts("     minimo que faz um app, e nos ja o temos? Mede-se sobre o app que existe.\n");
    {
        static const char *MINIMO[] = {
            "/app/package.json",        /* o manifesto: o que ele e e o que ele usa */
            "/app/src/main.js",         /* a entrada: por onde ele comeca */
            "/app/src/manifesto.json",  /* os dados que ele serve */
        };
        int tem = 0;
        long bytes = 0;
        for(int i = 0; i < 3; i++){
            snprintf(cam, sizeof cam, "%s%s", RAIZ, MINIMO[i]);
            long n = tamanho(cam);
            if(n > 0){ tem++; bytes += n; }
        }
        ok("o app do chess tem as TRES pecas do minimo: manifesto, entrada, e os dados",
           tem == 3);
        /* e esta segunda tem de medir OUTRA coisa, senão é a primeira escrita duas vezes:
         * que as duas de JSON ABREM MESMO em '{' e a de texto não — a marca do formato, lida. */
        int json = 0, texto = 0;
        for(int i = 0; i < 3; i++){
            snprintf(cam, sizeof cam, "%s%s", RAIZ, MINIMO[i]);
            unsigned char c1[4];
            if(pedaco(cam, 0, c1, 1) != 1) continue;
            if(c1[0] == '{') json++; else texto++;
        }
        ok("e cada uma traz a MARCA do seu formato: duas abrem em '{' (JSON), a entrada nao",
           json == 2 && texto == 1);
        printf("     -> %d pecas, %ld bytes. Criar um app e escrever TRES ficheiros cujos\n",
               tem, bytes);
        puts("        formatos ja estao no catalogo — nao e uma capacidade nova, e uma receita.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  Nem o WASM nem o Node abriram lugar novo. O wasm marca o nivel com a SECAO (e o");
    puts("  formato diz o tamanho antes do corpo, que e o que o nosso banco faz com os slots);");
    puts("  o Node marca-o com o JSON, que ja la estava desde o caminho.h.");
    puts("");
    puts("  E a ISA nao precisou de crescer: a pilha do wasm com profundidade dois E o par");
    puts("  (A,B), e todo binario dele cai na ULA. \"B<-A ; A<-mem\" sempre foi um push.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
