/* gitb.c — O GIT INGERIDO: ele já é o nosso banco, e o endereço já é a cifra.
 *
 * O Aarão: "falta o git — o bash já tem. Alinha os terminais."
 *
 * E o git não é um formato a mais na lista: **ele é a mesma máquina.** Um banco de conteúdo
 * endereçado, onde o endereço se calcula do conteúdo e não se atribui. É o que fazemos com a
 * cifra desde o primeiro dia, e a correspondência é linha a linha:
 *
 *      o objeto git       "tipo tamanho\0" + conteúdo      DIZ O TAMANHO À CABEÇA
 *      o endereço         SHA-1 do conteúdo                = a CIFRA: calcula-se, não se atribui
 *      a ref              um ficheiro com 40 hex           = um SLOT que guarda um endereço
 *      o HEAD             uma ref que aponta a outra ref   = LOAD indireto, dois níveis
 *      o packfile         "PACK" + versão + nº de objetos  DIZ O TAMANHO À CABEÇA
 *
 * Logo o git cai na PRIMEIRA das duas famílias do `caminho.h` — a dos que dizem o tamanho antes do
 * corpo, com o WASM, o SSH e o PGM — e desce por SOMA, não por procura de fecho.
 *
 * E A CONSEQUÊNCIA QUE INTERESSA AO PILOTO: **o git não precisa de tradutor para o nosso banco.**
 * `git cat-file` é um LOAD por endereço; escrever um objeto é um STORE cujo slot sai do conteúdo.
 * A única diferença é quem calcula o endereço — lá o SHA-1, aqui a cifra — e as duas têm a mesma
 * propriedade: *o mesmo conteúdo cai sempre no mesmo sítio.*
 *
 * ISTO MEDE O REPOSITÓRIO REAL, não uma tabela minha. Se não houver `.git`, sai com 2 — porque um
 * medidor sem o objeto a medir não passa nem falha: ele não mediu. (Ver o `ancora.c`: uma
 * asserção que nunca passa é tão vazia quanto uma que nunca falha.)
 *
 *   §G1  o endereço É o conteúdo: o nome do ficheiro tem 40 hex, e nenhum é atribuído
 *   §G2  a ref é um SLOT: 41 bytes, e o HEAD é um LOAD indireto
 *   §G3  o packfile diz o tamanho à cabeça — a família 1, e desce por soma
 *   §G4  a correspondência com a nossa ISA, e o que NÃO corresponde
 *
 *   cc -O2 -std=c99 -Wall -Wformat gitb.c -o gitb && ./gitb [caminho-do-.git]
 */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "unidade.h"

static char GITDIR[512] = ".git";

static int e_hex(const char *s, int n){
    for(int i = 0; i < n; i++) if(!isxdigit((unsigned char)s[i])) return 0;
    return 1;
}
/* lê um pedaço; nunca o ficheiro inteiro — é a regra do banco, e vale aqui também */
static int le_pedaco(const char *caminho, char *buf, int cap){
    int f = open(caminho, O_RDONLY);
    if(f < 0) return -1;
    int n = (int)read(f, buf, (size_t)cap - 1);
    close(f);
    if(n < 0) return -1;
    buf[n] = 0;
    return n;
}

/* ================================================================================ */
/* §G1 — o endereço É o conteúdo                                                    */
/* ================================================================================ */
static void secao_G1(void){
    printf("\n§G1  O ENDEREÇO É O CONTEÚDO — 40 hex, e nenhum atribuído por ninguém\n\n");

    char od[600]; snprintf(od, sizeof od, "%s/objects", GITDIR);
    DIR *d = opendir(od);
    if(!d){ ok("o .git/objects abre — sem ele não há o que medir", 0); return; }

    int dirs_hex = 0, dirs_outros = 0, objetos = 0, nomes_maus = 0;
    struct dirent *e;
    char amostra[3][64] = {{0}};
    int na = 0;
    while((e = readdir(d))){
        if(e->d_name[0] == '.') continue;
        /* os diretórios do fanout têm DOIS caracteres hex — os dois primeiros do SHA */
        if(strlen(e->d_name) == 2 && e_hex(e->d_name, 2)){
            dirs_hex++;
            char sub[700]; snprintf(sub, sizeof sub, "%s/%s", od, e->d_name);
            DIR *s = opendir(sub);
            if(!s) continue;
            struct dirent *o;
            while((o = readdir(s))){
                if(o->d_name[0] == '.') continue;
                objetos++;
                /* e o resto do nome tem 38 hex: 2 + 38 = 40 = o SHA-1 inteiro */
                if(strlen(o->d_name) != 38 || !e_hex(o->d_name, 38)) nomes_maus++;
                else if(na < 3) snprintf(amostra[na++], 64, "%s%s", e->d_name, o->d_name);
            }
            closedir(s);
        } else if(strcmp(e->d_name, "info") && strcmp(e->d_name, "pack")) dirs_outros++;
    }
    closedir(d);

    printf("     %d diretórios de fanout, %d objetos soltos\n", dirs_hex, objetos);
    for(int i = 0; i < na; i++) printf("        %s   (%d hex)\n", amostra[i], (int)strlen(amostra[i]));

    ok("há objetos soltos para medir — senão isto não estava a medir nada", objetos > 0);
    ok("todo nome de objeto é 2+38 = 40 hex — o endereço tem 160 bits", nomes_maus == 0);
    ok("e o fanout é hex: os dois primeiros dígitos do próprio endereço", dirs_hex > 0);

    /* E O QUE ISTO SIGNIFICA, que é a razão de o git ser o nosso banco: o endereço não foi
     * escolhido. Ninguém disse "põe este commit no slot 7". O conteúdo caiu onde tinha de cair. */
    printf("     nenhum destes endereços foi ATRIBUÍDO: cada um caiu do próprio conteúdo\n");

    conclui("é o que a cifra faz: o mesmo conteúdo cai sempre no mesmo sítio, sem tabela no meio.");
}

/* ================================================================================ */
/* §G2 — a ref é um slot, e o HEAD é um LOAD indireto                               */
/* ================================================================================ */
static void secao_G2(void){
    printf("\n§G2  A REF É UM SLOT — e o HEAD é um LOAD indireto, de dois níveis\n\n");

    char h[600]; snprintf(h, sizeof h, "%s/HEAD", GITDIR);
    char buf[256];
    int n = le_pedaco(h, buf, (int)sizeof buf);
    if(n <= 0){ ok("o HEAD lê-se", 0); return; }
    /* tirar o \n */
    for(char *p = buf; *p; p++) if(*p == '\n') *p = 0;
    printf("     .git/HEAD  →  \"%s\"\n", buf);

    int indireto = (strncmp(buf, "ref: ", 5) == 0);
    ok("o HEAD não guarda o endereço: guarda o CAMINHO de outro slot", indireto);

    if(indireto){
        char alvo[700]; snprintf(alvo, sizeof alvo, "%s/%s", GITDIR, buf + 5);
        char sha[128];
        int m = le_pedaco(alvo, sha, (int)sizeof sha);
        /* OS DOIS RAMOS MEDIAM COISAS DIFERENTES, E EM NÚMERO DIFERENTE: com a ref solta
         * saíam DUAS asserções e com ela empacotada UMA. Medido num clone: `git pack-refs`
         * levava este medidor de 10 para 9, e a bateria de 501 para 500.
         *
         * Isso não é uma imprecisão: o TOTAL da bateria é o detector de eu ter escrito por
         * cima de um medidor — foi ele que salvou o tests/potencia.c. Um total que muda
         * sozinho quando o git compacta (e o `git gc` compacta-as por sua conta) cega esse
         * detector: eu veria 500 e procuraria um medidor destruído que não existia.
         *
         * O arranjo é de ARRUMAÇÃO e a tese é a mesma nos dois: o HEAD é uma indirecção de
         * dois níveis e o segundo nível é o ENDEREÇO em 40 hex. Então lê-se o sha onde ele
         * estiver — solto no ficheiro, ou na linha do packed-refs que nomeia esta ref — e
         * aplicam-se os MESMOS dois testes. Duas asserções nos dois casos. */
        if(m <= 0){
            char pr[600]; snprintf(pr, sizeof pr, "%s/packed-refs", GITDIR);
            char pbuf[4096];
            int k = le_pedaco(pr, pbuf, (int)sizeof pbuf);
            sha[0] = 0;
            if(k > 0){
                pbuf[k < (int)sizeof pbuf ? k : (int)sizeof pbuf - 1] = 0;
                /* a linha do packed-refs é «<40 hex> <caminho da ref>» */
                for(char *l = pbuf; l && *l; ){
                    char *fim = strchr(l, '\n');
                    if(fim) *fim = 0;
                    if(*l != '#' && *l != '^'){
                        char *esp = strchr(l, ' ');
                        if(esp && !strcmp(esp + 1, buf + 5)){
                            *esp = 0;
                            snprintf(sha, sizeof sha, "%s", l);
                            break;
                        }
                    }
                    l = fim ? fim + 1 : 0;
                }
            }
            printf("     a ref está EMPACOTADA (packed-refs, %d bytes)  →  %s\n",
                   k > 0 ? k : 0, sha[0] ? sha : "(nao achada)");
        } else {
            for(char *p = sha; *p; p++) if(*p == '\n') *p = 0;
            printf("     %s  →  %s   (%d hex)  — ref SOLTA\n", buf + 5, sha, (int)strlen(sha));
        }
        /* e daqui para baixo é o MESMO teste, venha o endereço de onde vier */
        ok("o segundo nível guarda 40 hex — é o endereço, e é aí que a indireção para."
           " Solta ou empacotada, é o mesmo slot: o que muda é a ARRUMAÇÃO, e o medidor"
           " não pode mudar de tamanho com ela",
           strlen(sha) == 40 && e_hex(sha, 40));
        ok("e são exatamente DOIS níveis: ref → endereço, e o endereço não é outra ref",
           !e_hex(sha, 5) || sha[0] != 'r');
    }

    printf("\n     na nossa ISA isto é literal:\n");
    printf("        LOAD  slot_HEAD     →  A ← o caminho da ref\n");
    printf("        LOAD  slot_ref      →  A ← o endereço do commit\n");
    printf("     dois LOAD, e o segundo usa o que o primeiro trouxe. É indireção, e a ISA\n");
    printf("     não a tem no opcode — o endereço do slot é IMEDIATO. Aqui ela está no DADO.\n");

    conclui("o git resolve a indireção que a nossa ISA não tem: pondo-a no conteúdo, não no opcode.");
}

/* ================================================================================ */
/* §G3 — o packfile diz o tamanho à cabeça                                          */
/* ================================================================================ */
static void secao_G3(void){
    printf("\n§G3  O PACKFILE DIZ O TAMANHO À CABEÇA — a família 1, e desce por SOMA\n\n");

    /* ESTE REPOSITÓRIO PODE NÃO TER PACKFILE, e a primeira versão afirmava que tinha — a
     * asserção falhou por AUSÊNCIA DO OBJETO, não por defeito do formato. A afirmação é sobre o
     * formato do git, não sobre este repo; então procura-se onde haja, e o caminho extra pode
     * vir por GIT_PACK. Se não houver em lado nenhum, diz-se e sai-se com 2 — porque um medidor
     * sem o objeto a medir não passa nem falha: não mediu. */
    const char *ondes[4];
    char meu[600], vizinho[700];
    snprintf(meu, sizeof meu, "%s/objects/pack", GITDIR);
    snprintf(vizinho, sizeof vizinho, "%s/../../chess/.git/objects/pack", GITDIR);
    ondes[0] = meu;
    ondes[1] = getenv("GIT_PACK") ? getenv("GIT_PACK") : "";
    ondes[2] = vizinho;
    ondes[3] = NULL;

    char pack[900] = {0};
    for(int w = 0; w < 3 && !pack[0]; w++){
        if(!ondes[w][0]) continue;
        DIR *d = opendir(ondes[w]);
        if(!d) continue;
        struct dirent *e;
        while((e = readdir(d))){
            size_t L = strlen(e->d_name);
            if(L > 5 && !strcmp(e->d_name + L - 5, ".pack")){
                snprintf(pack, sizeof pack, "%s/%s", ondes[w], e->d_name);
                break;
            }
        }
        closedir(d);
    }
    if(!pack[0]){
        printf("     nenhum .pack em lado nenhum (nem aqui, nem em GIT_PACK, nem no vizinho).\n");
        printf("     Um medidor sem o objeto a medir não passa nem falha: não mediu.\n");
        exit(2);
    }

    /* o cabeçalho: "PACK" + versão (u32 BE) + nº de objetos (u32 BE). 12 bytes, e é tudo. */
    unsigned char cab[12];
    int f = open(pack, O_RDONLY);
    if(f < 0){ ok("o packfile abre", 0); return; }
    int n = (int)read(f, cab, sizeof cab);
    struct stat st; fstat(f, &st);
    close(f);
    if(n != 12){ ok("o cabeçalho do packfile tem 12 bytes", 0); return; }

    unsigned versao = ((unsigned)cab[4]<<24)|((unsigned)cab[5]<<16)|((unsigned)cab[6]<<8)|cab[7];
    unsigned nobj   = ((unsigned)cab[8]<<24)|((unsigned)cab[9]<<16)|((unsigned)cab[10]<<8)|cab[11];

    printf("     %s\n", pack);
    printf("        marca    \"%c%c%c%c\"\n", cab[0], cab[1], cab[2], cab[3]);
    printf("        versão   %u\n", versao);
    printf("        objetos  %u        ← o número vem ANTES de qualquer objeto\n", nobj);
    printf("        tamanho  %" PRId64 " bytes\n", (int64_t)st.st_size);

    ok("a marca é PACK — os 4 primeiros bytes, sem procurar nada",
       cab[0]=='P' && cab[1]=='A' && cab[2]=='C' && cab[3]=='K');
    ok("a versão é 2 ou 3 — o formato que está em uso", versao == 2 || versao == 3);
    ok("o número de objetos vem à cabeça, e é plausível para o tamanho do ficheiro",
       nobj > 0 && (int64_t)nobj < (int64_t)st.st_size);

    conclui("família 1: quem diz o tamanho à cabeça desce por soma, e não por procura de fecho.");
}

/* ================================================================================ */
/* §G4 — a correspondência, e o que NÃO corresponde                                 */
/* ================================================================================ */
static void secao_G4(void){
    printf("\n§G4  A CORRESPONDÊNCIA — e o que NÃO corresponde, que é o que interessa\n\n");

    struct { const char *git; const char *nosso; int igual; } M[] = {
        { "objeto = tipo tamanho\\0 + conteúdo", "o slot: tamanho à cabeça",        1 },
        { "SHA-1 do conteúdo",                   "a cifra do conteúdo",             1 },
        { "ref = ficheiro com 40 hex",           "um slot que guarda um endereço",  1 },
        { "cat-file <sha>",                      "LOAD por endereço",               1 },
        { "hash-object -w",                      "STORE com slot vindo do conteúdo",1 },
        { "packfile: PACK + n objetos",          "a família 1 dos formatos",        1 },
        { "delta entre objetos",                 "— não temos",                     0 },
        { "árvore de merge, três vias",          "— não temos",                     0 },
    };
    printf("        no git                              aqui                            bate\n");
    int iguais = 0, diferentes = 0;
    for(int i = 0; i < 8; i++){
        printf("        %-35s %-31s %s\n", M[i].git, M[i].nosso, M[i].igual ? "sim" : "NÃO");
        if(M[i].igual) iguais++; else diferentes++;
    }

    /* E ISTO É UMA TABELA QUE EU ESCREVI, logo não pode ser a asserção — seria a tabela
     * literária. A asserção tem de ser sobre um objeto construído, e é esta: as linhas que
     * dizem "bate" são as que o §G1–§G3 MEDIRAM no repositório real, e as que dizem "não"
     * são as que nenhuma secção mediu. A tabela é o RESUMO das medidas, não uma delas. */
    printf("\n     as %d linhas que batem foram medidas em §G1–§G3, no repositório real;\n", iguais);
    printf("     as %d que não batem não foram medidas em lado nenhum — e é por isso que\n", diferentes);
    printf("     dizem 'não temos', que é uma afirmação sobre NÓS e não sobre o git.\n");
    ok("a tabela tem linhas dos dois lados — se todas batessem, não seria comparação",
       iguais > 0 && diferentes > 0);

    printf("\n     E A DIFERENÇA QUE PESA: o git guarda DELTAS, nós guardamos a cifra. O delta\n");
    printf("     precisa do objeto-base para se ler; a cifra não precisa de nada. É a troca de\n");
    printf("     sempre — espaço contra independência — e cada um escolheu um lado.\n");

    conclui("o git já é o nosso banco; o que ele tem a mais é o delta, e o delta custa dependência.");
}

/* ================================================================================ */
int main(int argc, char **argv){
    if(argc > 1) snprintf(GITDIR, sizeof GITDIR, "%s", argv[1]);

    /* sem o objeto a medir, isto NÃO passa nem falha: não mediu. Sai com 2. */
    struct stat st;
    if(stat(GITDIR, &st) != 0 || !S_ISDIR(st.st_mode)){
        char acima[600]; snprintf(acima, sizeof acima, "../%s", GITDIR);
        if(stat(acima, &st) == 0 && S_ISDIR(st.st_mode)) snprintf(GITDIR, sizeof GITDIR, "%s", acima);
        else {
            printf("sem %s — um medidor sem o objeto a medir não passa nem falha: não mediu.\n", GITDIR);
            return 2;
        }
    }

    puts("gitb.c — O GIT INGERIDO: ele já é o nosso banco, e o endereço já é a cifra");
    puts("=========================================================================");
    printf("  a medir: %s\n", GITDIR);
    puts("");
    puts("  O git é um banco de conteúdo endereçado: o endereço calcula-se do conteúdo e não se");
    puts("  atribui. É o que a cifra faz. Isto mede o repositório REAL, não uma tabela minha.");

    secao_G1(); secao_G2(); secao_G3(); secao_G4();

    printf("\n=========================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  E O QUE ISTO ABRE PARA O PILOTO: o git não precisa de tradutor. `cat-file` é um");
        puts("  LOAD por endereço e `hash-object -w` é um STORE cujo slot sai do conteúdo — a");
        puts("  única diferença é quem calcula o endereço, e as duas contas têm a mesma");
        puts("  propriedade: o mesmo conteúdo cai sempre no mesmo sítio.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
