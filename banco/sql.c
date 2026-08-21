
/* sql.c — SQL NO METAL. Compila para a ISA, e a memória é o DISCO. Sem RAM.
 *
 * Nada aqui é simulado por cima de estruturas em memória: o SQL vira BYTECODE da ISA do
 * broca-so (ula/instrucoes.h), o bytecode vive num arquivo, a memória da máquina vive noutro
 * arquivo, e o interpretador lê instrução por instrução com pread. O único estado em RAM são
 * os três registradores (A, B, R), o pc e as flags — como no metal.
 *
 * A ISA, transcrita de ula/instrucoes.c (não reinventada):
 *
 *   HALT                     para
 *   LOAD  slot(u16)          B←A ; A←mem[slot]
 *   STORE slot(u16)          mem[slot]←R            (grava R, NÃO A)
 *   ADD SUB AND OR XOR       R ← ula(A,B)           (componente a componente)
 *   GOLD                     A←A_1(A) ; R←A            o gato: estica, det −1, ordem ∞
 *   NEGRO_OURO               A←A_1⁻¹(A) ; R←A         a volta: INTEIRA, porque det = −1
 *   ESQUILO                  A←(−e, total) ; R←A       ×ω do cristal: det +1, ordem 4
 *   TROCA                    A←(e, total) ; R←A        J, a involução: det −1, ordem 2
 *   CMP                      FL_ZERO sse A e B são AMBOS zero; FL_EQ se iguais
 *   JMP JZ JNZ  rel(s8)      pc ← pc + 1 + rel
 *   FOLD UNFOLD PROJECT LIFT as folhas e as projeções
 *
 * Duas consequências da ISA real, que moldam o compilador:
 *   (1) STORE grava R, então pôr uma constante num slot é LOAD k, LOAD zero, ADD, STORE.
 *   (2) FL_ZERO é "ambos zero", então a igualdade a=k testa-se por SUB e depois CMP com A=0:
 *       LOAD col, LOAD k, SUB, STORE tmp, LOAD tmp, LOAD zero, CMP  →  FL_ZERO sse col=k.
 *   (3) o endereço do slot é IMEDIATO (u16 na instrução): não há indexação indireta. Logo o
 *       compilador DESENROLA a varredura — ele lê o catálogo antes de compilar e emite o
 *       código das linhas que existem. O programa é compilado para o estado atual da tabela.
 *
 * Mapa (átomo=Word_8; Word ISA=Word_8² Lei 7; σ²=σ+1; blobs via atomos_*):
 *   0   catálogo {ncols, nrows}     (Word ISA)
 *   1   a constante 0        2  a constante 1        3  temporário
 *   4   o contador de casamentos
 *   8   a constante da consulta (o k do WHERE)
 *   16+ o bitmap de casamento, uma linha por slot
 *   1024+ as linhas: linha i, coluna j  →  slot 1024 + i*ncols + j
 *   2048+ S_CF: palavras FC (rt_cf_slot.h), S_CF_STRIDE slots cada
 *   500000+ S_CAB / S_ALVO / S_FOLHA / S_CB — blobs em átomos físicos
 *
 *   cc -O2 -std=c99 sql.c -o sql
 *   ./sql <base> "CREATE TABLE t (a,b,c)"
 *   ./sql <base> "INSERT INTO t VALUES (7,8,9)"
 *   ./sql <base> "SELECT * FROM t WHERE a = 7"
 *   ./sql teste
 */
#define _POSIX_C_SOURCE 200809L
/* E _DEFAULT_SOURCE tambem, senao o -std=c99 estrito da bateria esconde ip_mreq e usleep e este
 * ficheiro NAO COMPILA — e as 87 assercoes dele deixam de ser medidas em silencio. Foi o que
 * aconteceu: tres corridas seguidas com "sql.c NAO COMPILOU" na tabela e eu a ler so a linha
 * das unidades abertas. O compilador avisou e ninguem leu. */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include "../lib/disco.h"
#include "../lib/slot_mem.h"
#include "../lib/palavra8.h"
#include "../lib/word_isa.h"
#include "../lib/slot_map.h"
#include "../lib/reta.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "banda.h"
#include "stratum.h"
#include "cifra.h"
/* O CATALOGO. As funcoes dos corpos ja existem — nao se importa nada de fora, e nao se
 * reescreve nada aqui: cr_norma e cr_cmp sao a regua eliptica, e sao as que decidem. */
#include "corpos.h"
#include <ctype.h>
#include <strings.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "unidade.h"
#include "contrato.h"   /* o toolkit: a tríade ⊕ ⊗ ∏ de cada corpo */
#include "sql_api.h"    /* porta C para pgwire — captura de resultado */

static SqlOut *sql_cap = NULL;   /* preenchido por sql_executa quando out!=NULL */

/* ---------------- a ISA (transcrita) ---------------- */
enum { OP_HALT=0, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_AND, OP_OR, OP_XOR,
       OP_GOLD, OP_CMP, OP_JMP, OP_JZ, OP_JNZ,
       OP_FOLD, OP_LOADS,
       /* saíram quatro nomes que estavam só neste enum: sem um único `case`, sem
        * entrada no montador e sem uso. Reservados que nunca correram — e manter
        * redundância custa mais do que a tirar. (Os nomes não se escrevem aqui: o
        * erg.c LÊ este enum do ficheiro, e apanhá-los-ia como opcodes.) */
       /* A VOLTA. Acrescentados no FIM de propósito: o número de cada opcode antigo não
        * muda, e nenhum programa já compilado passa a significar outra coisa. */
       OP_NEGRO_OURO,
       /* O CIRCUITO. O gato estica; faltava quem GIRE, e sem ele a máquina não gera o grupo
        * todo. ESQUILO é ×ω do cristalino (t=0): det +1, ordem 4. TROCA é J, a involução. */
       OP_ESQUILO, OP_TROCA,
       /* O MARTELO. A prova de trabalho é uma TAREFA DO BANCO, não de um processo que fala com
        * ele: o SELECT e o UPDATE já correm nesta máquina, e o martelo corre ao lado deles. Como
        * OP_GOLD, ele não é um programa — é um opcode que a máquina executa. */
       OP_MARTELO };
#define FL_ZERO 0x01
#define FL_EQ   0x02
#define FL_LT   0x04

/* Word ISA = word_isa.h (Word_8²). Disco: 2 átomos/slot (Lei 7). */
typedef struct { Word A, B, R; unsigned pc; unsigned char flags; } Regs;

/* ---------------- os slots ---------------- */
#define S_CAT     0
#define S_ZERO    1
#define S_UM      2
#define S_TMP     3
/* O MARTELO: cabeçalho 80 átomos + alvo 32 átomos. Longe das linhas. */
/* O CANAL É UM BACKEND DE LOAD/STORE, NÃO UM PROTOCOLO À PARTE.
 *
 * O banco já faz E/S sem opcode por leitura: LOAD e STORE vão ao ficheiro por pread/pwrite. O
 * canal é outro destino dos MESMOS dois. Acima de S_CANAL, LOAD recebe da banda e STORE emite —
 * e a ISA não cresceu, o compilador não mudou, e a query não sabe que falou com outro banco.
 *
 * E É POR ISSO QUE OS PROTOCOLOS SÃO INDISTINGUÍVEIS PARA O BANCO: o que ele vê é sempre um slot
 * a ser lido e um slot a ser escrito. UDP com bump, STOMP, TCP — a topologia é a mesma, porque
 * do lado de cá não há topologia nenhuma: há um endereço. Trocar de protocolo é trocar o que
 * está atrás de duas funções, e nem uma linha de SQL muda. */
/* ACIMA DE TUDO O RESTO. Eu pus isto em S_LINHAS+50000 e enterrei o S_NO da cifra (que mora em
 * S_LINHAS+240000): os nos do indice passaram a ir para a rede em vez do disco, e a bateria
 * apanhou-o em tres asserçoes. A fronteira de um backend tem de ficar onde nao pisa ninguem. */
#define S_CANAL   (S_LINHAS + 10000000u)
/* O POOL, O MESMO DESENHO. Acima de S_POOL, LOAD devolve um campo do job corrente e STORE no
 * slot da share emite o mining.submit. O protocolo fica atras de duas funcoes — e e por isso que
 * ele e indistinguivel do canal, e o canal do disco: o banco le um slot e escreve um slot.
 *
 * Os campos, pela ordem em que o cabecalho os quer:
 *   +0 versao   +1 nbits   +2 ntime   +3..+10 prevhash   +11 tem job   +12..+19 merkle
 *   +20 a SHARE: escrever aqui submete o nonce */
#define S_POOL     (S_CANAL + 100000u)
#define S_POOL_SH  (S_POOL + 20u)
/* Blobs (cab/alvo/merkle/coinbase): índices FÍSICOS de átomo (atomos_* → slot_mem directo).
 * Não passam pelo stride-2 das Words ISA. Longe do mapa Word (S_LINHAS…). */
#define S_CAB     500000u                 /* 80 átomos = cabeçalho */
#define S_ALVO    (S_CAB + 80u)           /* 32 átomos = alvo */
#define S_FOLHA   (S_CAB + 200u)          /* folhas merkle */
#define S_CB      (S_CAB + 500u)          /* coinbase em átomos */
#define S_FOLD_ARG (S_CB + 800u)         /* u32 LE base do OP_FOLD */
#define S_FAIXA   (S_FOLD_ARG + 4u)      /* u32 LE de, u32 LE ate — OP_MARTELO */
#define S_CONTA   4
#define S_MASK    5          /* a máscara do bit de sinal — é ela que dá o < e o >     */
#define S_ACC     6          /* o acumulador booleano da cláusula inteira                */
#define S_V       7          /* o valor do SET                                          */
#define S_K       8          /* 8..23  a constante de cada condição                     */
#define S_COND    24         /* 24..39 o resultado de cada condição (0 ou 1)            */
#define S_TERMO   40         /* 40..47 o resultado de cada termo (as condições em AND)  */
#define S_UME     48         /* o "um" no campo .e — para incrementar nrows              */
#define S_DIA     58         /* o DIÁRIO: {total = ação pendente, e = coluna do SET}      */
/* O CORPO DE CADA COLUNA — passo 1 de 6 do catálogo em SQL (ver docs/TOOLKIT.md).
 *
 * A coluna deixa de ser "um número" e passa a DECLARAR em que corpo vive. O slot guarda
 * {total = o código do corpo, e = o parâmetro dele} — o metal m no áureo, o n no mórfico.
 *
 * Só o campo e o CREATE a aceitá-lo. O despacho das operações vem depois, um corpo de cada
 * vez, cada um com medidor antes de entrar — que é o método que o Aarão pediu e que hoje
 * mostrou ser o único que fecha. */
#define S_CORPO   60         /* 60..67: o corpo de cada coluna */
#define CORPO_INTEIRO  0     /* o de sempre — e continua a ser o omitido, sem quebrar base antiga */
#define CORPO_RACIONAL 1
#define CORPO_AUREO    2
#define CORPO_MORFICO  3
#define CORPO_CRISTAL  4     /* PASSO 6: o lado que gira — a+bω, ω²=tω−1 */
#define S_MT      57         /* mascara {total=todos os bits, e=0} — limpa o .e apos GOLD */
#define S_KZ      49         /* 49..56  o zero de cada comparação (a contração compara com 0) */
#define S_LIN     4096       /* 4096+  o rascunho de cada átomo: acc, prod, cnt, passo…   */
/* A ÁRVORE DA EXPRESSÃO COMEÇAVA EM 64 — E O CORPO DA COLUNA VIVE EM 60..67.
 *
 * As duas regiões PISAVAM-SE em 64..67, que são as colunas 4 a 7. Numa tabela com
 * cinco colunas ou mais, o primeiro SELECT com WHERE escrevia o temporário da raiz
 * por cima do corpo declarado, E FICAVA NO DISCO: medido, uma coluna `MORFICO(6)`
 * — o par (3,6) — passava a (1,0), que é RACIONAL. A partir daí a aritmética
 * daquela coluna é a álgebra errada, para sempre, e nada o diz.
 *
 * Ninguém o apanhou porque todas as tabelas dos medidores têm três colunas.
 *
 * A árvore desce por `dest+2` e `dest+34` recursivamente, e o espaço reservado era
 * 128 slots. Fica 72..199, e o nome da tabela vai para 224 — com folga declarada
 * entre as três regiões em vez de encostadas. */
#define S_EXPR    72         /* 72..199  os temporários da árvore da expressão          */
/* O NOME DA TABELA — 192..207, dois caracteres por Word (Lei 7: o par são dois átomos).
 *
 * O catálogo guardava só {ncols, nrows}: a relação não tinha NOME. O `varre` lia o
 * `FROM x` para uma variável e nunca a usava, e o mesmo no `INSERT INTO x`. A
 * consequência não é cosmética — `SELECT * FROM tabela_que_nao_existe` devolvia as
 * linhas da tabela que lá estava, e uma aplicação que se enganasse no nome recebia
 * dados de outra e não tinha como saber. Pela porta FEBE isso chega ao driver como um
 * SELECT bem sucedido.
 *
 * Base ANTIGA: o nome lê-se vazio (os slots estavam a zero) e aí aceita-se qualquer
 * nome — é a mesma compatibilidade que o CORPO_INTEIRO tem por ser o código 0. */
#define S_NOME    224        /* 224..239: 16 Words = 32 caracteres (folga até S_MATCH) */
#define S_NOME_N  16
#define S_MATCH   256        /* bitmap do resultado, uma linha por slot (256..511)      */
#define MAXCOND   4          /* condições por termo                                     */
#define MAXTERMO  4          /* termos ligados por OR                                   */
#define S_VIVO    512        /* a linha existe? o DELETE zera aqui (512..1023)          */
#define S_DEN     33792      /* o DENOMINADOR de cada célula, no TOTAL do seu slot: a ISA não
                              * move e→total, e a conta precisa de q como número. */
#define S_LINHAS  1024
/* ── O NOME DE CADA COLUNA — 36864.., 16 Words (32 caracteres) por coluna ─────
 * O `CREATE TABLE cliente (nome,idade,saldo)` lia os três identificadores e
 * DEITAVA-OS FORA: só o corpo de cada coluna era guardado. A consequência não é
 * cosmética — o SELECT devolvia `a`, `b`, `c`, e `WHERE idade > 20` saía como
 * «comando recusado», porque a coluna era decidida por `nome[0] - 'a'`. Um
 * `UPDATE beta SET z = 99` pedia a coluna 25 numa tabela de três.
 *
 * A região fica acima das linhas (S_LINHAS + MAXLIN·8 = 3024) e dos
 * denominadores (S_DEN + MAXLIN·8 = 35792), longe do mapa que a ISA percorre.
 * Dois caracteres por Word, como o nome da tabela — é a mesma Lei 7. */
/* ── O ÁTOMO ALTO DE CADA CÉLULA — 40960.., um plano paralelo ────────────────
 * A célula é `{numerador, denominador}` em dois átomos, e a ULA soma componente
 * a componente: o valor de uma coluna inteira vive em OITO bits. O `INSERT ...
 * VALUES (1000,...)` era recusado (§24) porque guardar 232 em vez de 1000 é pior.
 *
 * O byte ALTO passa a viver num plano à parte — como o denominador já vive em
 * S_DEN —, e a Word da linha não muda: TODA a aritmética emitida continua a ler
 * o que sempre leu. O que muda é a ESCRITA e a LEITURA, que são a fronteira.
 *
 * E a metade que falta diz-se em vez de se fingir: o avaliador do WHERE é
 * polinomial e de oito bits (o `S_MT` mascara o `.e` e os produtos são de átomo),
 * logo uma comparação que cite uma coluna com valores acima de 255 é RECUSADA,
 * contada, e com o motivo escrito. Alargar o avaliador é o trio seguinte —
 * a aritmética do andar já está provada (`ula_add16`, §26). */
#define S_ALTO      40960u
#define S_COLNOME   36864u
#define S_COLNOME_W 16u        /* Words por nome → 32 caracteres */
#define S_COLNOME_N 8u         /* tantas quantas o S_CORPO segura */
/* S_CF definido em lib/slot_map.h — região FC, 2048..S_CF_END */
#define MAXLIN    250
#define MAXNO     64         /* nós da árvore do WHERE                                  */
#define SLOTSZ    SLOT_WORD_BYTES

/* ---------------- a memória É o disco ---------------- */
static int fmem = -1, fprog = -1;
static char g_base[512];          /* caminho da base aberta — blobs em <base>_corpo/ */
static char g_tabela[64];         /* a tabela cujo .mem está aberto ("" = a sem nome) */
static long cel_recusadas = 0;    /* células que não couberam no envelope — contadas À PARTE */
static long cmp_recusadas = 0;    /* comparações recusadas por a coluna ser larga */
static int usa_tabela(const char *nome, int cria_se_falta);   /* definida com abrir_base */
static int usa_tabela_z(const char *nome, int cria_se_falta, int zera);
static int tabela_existe(const char *nome);

/* O backend do canal. Trocar isto por STOMP ou por TCP é trocar estas duas funções — o banco
 * não distingue, porque o que ele faz é sempre ler um slot e escrever um slot. */
static int canal_fd = -1;
static unsigned char canal_banda[32];
static struct sockaddr_in canal_dst;
static void canal_abre(void){
    if(canal_fd >= 0) return;
    banda_de(getenv("TIFFANY_TECIDO") ? getenv("TIFFANY_TECIDO") : "tecido por omissao",
             canal_banda);
    canal_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int um = 1;
    setsockopt(canal_fd, SOL_SOCKET, SO_REUSEADDR, &um, sizeof um);
    struct sockaddr_in e; memset(&e, 0, sizeof e);
    e.sin_family = AF_INET; e.sin_addr.s_addr = htonl(INADDR_ANY); e.sin_port = htons(47313);
    bind(canal_fd, (struct sockaddr*)&e, sizeof e);
    struct ip_mreq mr;
    mr.imr_multiaddr.s_addr = inet_addr("239.7.31.27");
    mr.imr_interface.s_addr = htonl(INADDR_LOOPBACK);
    setsockopt(canal_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof mr);
    struct in_addr i; i.s_addr = htonl(INADDR_LOOPBACK);
    setsockopt(canal_fd, IPPROTO_IP, IP_MULTICAST_IF, &i, sizeof i);
    setsockopt(canal_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &um, sizeof um);
    struct timeval to = { 0, 200000 };
    setsockopt(canal_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
    memset(&canal_dst, 0, sizeof canal_dst);
    canal_dst.sin_family = AF_INET;
    canal_dst.sin_addr.s_addr = inet_addr("239.7.31.27");
    canal_dst.sin_port = htons(47313);
}
static void canal_grava(unsigned slot, Word w){
    canal_abre();
    unsigned char m[6], ks[6], b[6];
    memcpy(m, &slot, 4); m[4] = w.total; m[5] = w.e;
    keystream(canal_banda, ks, 6);
    bump(m, ks, b, 6);
    sendto(canal_fd, b, 6, 0, (struct sockaddr*)&canal_dst, sizeof canal_dst);
}
static Word canal_le(unsigned slot){
    canal_abre();
    Word w = { 0, 0 };
    unsigned char ks[6], b[6], m[6];
    keystream(canal_banda, ks, 6);
    for(;;){
        socklen_t n = sizeof canal_dst;
        if(recvfrom(canal_fd, b, 6, 0, (struct sockaddr*)&canal_dst, &n) != 6) break;
        bump(b, ks, m, 6);
        unsigned s2; memcpy(&s2, m, 4);
        if(s2 == slot){ w.total = m[4]; w.e = m[5]; break; }
    }
    return w;
}
/* O backend do pool. Trocar stratum por outra coisa e trocar estas duas — nem uma linha de SQL. */
/* o pool e a tabela de relacoes moram no disco — 9,7 KB e 8 KB que nao tem por que
 * estar em .bss: sao estado do processo, e o processo le-os do ficheiro */
static Pool *const pool_p = DISCO_FIXO(Pool, 24);
#define pool_st  (*pool_p)
static int  pool_ligado = 0;
static char pool_user[128];
/* ---------------- O BANCO DAS CONFIGS ----------------
 *
 * Um banco NORMAL e a parte: ficheiro proprio, pread proprio, e por isso pode ser lido de
 * qualquer sitio — inclusive daqui, que e antes do mem_le existir. Foi o que me partiu tres
 * tentativas seguidas: eu queria meter a config nos slots do banco da mina e o pool_abre mora
 * ACIMA do mem_le. Separado, o problema nao existe.
 *
 * A chave do stratum vive aqui, e nao no ambiente: quem guarda estado e o banco, e uma chave em
 * variavel de ambiente e estado fora dele.
 *
 * Um registo e (nome, valor): 4 slots de nome, 28 de valor, 64 registos.
 *   ./sql <base> "CONFIG pool_user 'a chave'"     poe
 *   ./sql <base> "CONFIG pool_user"               le
 */
#define CONF_N   64
#define CONF_SL  32
#define CONF_NM  64
static int fconf = -1;
static void conf_abre(const char *base){
    if(fconf >= 0) return;
    char c[512]; snprintf(c, sizeof c, "%s.conf", base);
    fconf = open(c, O_RDWR|O_CREAT, 0600);          /* 0600: e chave, e so do dono */
}
static int conf_slot(const char *nome){
    unsigned h = 5381;
    for(const char *p = nome; *p; p++) h = ((h << 5) + h) ^ (unsigned char)*p;
    return (int)(h % CONF_N);
}
static void conf_poe(const char *nome, const char *valor){
    if(fconf < 0) return;
    char reg[CONF_SL*16]; memset(reg, 0, sizeof reg);
    snprintf(reg, CONF_NM, "%s", nome);
    snprintf(reg + CONF_NM, sizeof reg - CONF_NM, "%s", valor);
    pwrite(fconf, reg, sizeof reg, (off_t)conf_slot(nome)*sizeof reg);
    fsync(fconf);
}
static void conf_le(const char *nome, char *out, size_t lim){
    out[0] = 0;
    if(fconf < 0) return;
    char reg[CONF_SL*16];
    if(pread(fconf, reg, sizeof reg, (off_t)conf_slot(nome)*sizeof reg) != (ssize_t)sizeof reg) return;
    reg[CONF_NM-1] = 0; reg[sizeof reg - 1] = 0;
    if(strcmp(reg, nome)) return;                    /* nao e este: o slot e de outro nome */
    snprintf(out, lim, "%s", reg + CONF_NM);
}
static void pool_abre(void){
    if(pool_ligado) return;
    /* a config do banco vem PRIMEIRO; o ambiente e o recurso de quem ainda nao a pos */
    char *cu = DISCO_FIXO(char, 120);
    char *ch = DISCO_FIXO(char, 121);
    char *cp = DISCO_FIXO(char, 122);
    disco_prende(DISCO_BASE(120),"dados/cu_120.bin",(size_t)((512)),sizeof(char));
    disco_zera(cu,(size_t)((512)),sizeof(char));
    disco_prende(DISCO_BASE(121),"dados/ch_121.bin",(size_t)((512)),sizeof(char));
    disco_zera(ch,(size_t)((512)),sizeof(char));
    disco_prende(DISCO_BASE(122),"dados/cp_122.bin",(size_t)((64)),sizeof(char));
    disco_zera(cp,(size_t)((64)),sizeof(char));
    conf_le("pool_user", cu, ((size_t)((512))*sizeof(char)));
    conf_le("pool_host", ch, ((size_t)((512))*sizeof(char)));
    conf_le("pool_porta", cp, ((size_t)((64))*sizeof(char)));
    const char *u = cu[0] ? cu : getenv("TIFFANY_POOL_USER");
    const char *h = ch[0] ? ch : getenv("TIFFANY_POOL_HOST");
    const char *p = cp[0] ? cp : getenv("TIFFANY_POOL_PORTA");
    if(!u || !h) return;
    snprintf(pool_user, sizeof pool_user, "%s", u);
    pool_ligado = st_liga(&pool_st, h, p ? atoi(p) : 3333, pool_user);
}
static void merkle_pelo_fold(void);   /* definida depois da maquina, que e quem dobra */
static Word pool_le(unsigned slot){
    Word w = { 0, 0 };
    pool_abre();
    if(!pool_ligado) return w;
    char l[8192];
    while(st_linha(&pool_st, l, sizeof l)){
        Fonte f = fonte_de(l);
        st_trata_fonte(&pool_st, &f);
    }
    unsigned k = slot - S_POOL;
    /* Word_8: campos largos ficam em pool_st; o slot só sinaliza / pedaço baixo. */
    if(k == 0) w.total = (Word8)pool_st.versao;
    else if(k == 1) w.total = (Word8)pool_st.nbits;
    else if(k == 2) w.total = (Word8)pool_st.ntime;
    else if(k >= 3 && k <= 10){
        const unsigned char *b = pool_st.prevhash + 4*(k-3);
        w.total = b[0]; w.e = b[1];
    }
    else if(k == 11) w.total = pool_st.tem_job ? 1 : 0;
    else if(k >= 12 && k <= 19){
        merkle_pelo_fold();
        const unsigned char *b = pool_st.merkle_raiz + 4*(k-12);
        w.total = b[0]; w.e = b[1];
    }
    return w;
}
static void pool_grava(unsigned slot, Word w){
    pool_abre();
    if(!pool_ligado) return;
    if(slot == S_POOL_SH) st_submete(&pool_st, pool_user, (unsigned)w.total);
}
static Word mem_le(unsigned slot){
    if(slot >= S_POOL)  return pool_le(slot);
    if(slot >= S_CANAL) return canal_le(slot);
    /* Word ISA = 2 átomos (total,e) — Lei 7; sem long. */
    return (Word){
        slot_mem_le(fmem, slot * 2u),
        slot_mem_le(fmem, slot * 2u + 1u)
    };
}
static void mem_grava(unsigned slot, Word w){
    if(slot >= S_CANAL){ canal_grava(slot, w); return; }
    if(slot >= S_POOL){ pool_grava(slot, w); return; }
    slot_mem_grava(fmem, slot * 2u,     w.total);
    slot_mem_grava(fmem, slot * 2u + 1u, w.e);
}
/* átomos físicos consecutivos (1 B = 1 índice) — cab/merkle/texto; sem stride Word */
static void atomos_grava(unsigned base, const unsigned char *b, int n){
    for(int i = 0; i < n; i++) slot_mem_grava(fmem, base + (unsigned)i, b[i]);
}
static void atomos_le(unsigned base, unsigned char *b, int n){
    for(int i = 0; i < n; i++) b[i] = slot_mem_le(fmem, base + (unsigned)i);
}
/* u32 LE em átomos físicos — endereços/faixas sem long na Word. */
static void atomos_u32(unsigned base, unsigned v){
    unsigned char b[4] = { (unsigned char)v, (unsigned char)(v>>8),
                           (unsigned char)(v>>16), (unsigned char)(v>>24) };
    atomos_grava(base, b, 4);
}
static unsigned atomos_le_u32(unsigned base){
    unsigned char b[4]; atomos_le(base, b, 4);
    return (unsigned)b[0] | ((unsigned)b[1]<<8) | ((unsigned)b[2]<<16) | ((unsigned)b[3]<<24);
}
static Word w8(unsigned t, unsigned e){
    return (Word){ (Word8)t, (Word8)e };
}

/* Palavra FC no .mem — mem_le no slot S_CF + word_ix·stride; mesmo layout que rt_cf_slot.h */
static Word cf_le(unsigned word_ix, unsigned rel){
    return mem_le(cf_slot_base(word_ix) + rel);
}
static void cf_grava(unsigned word_ix, unsigned rel, Word w){
    mem_grava(cf_slot_base(word_ix) + rel, w);
}
/* ── O NOME DA TABELA no catálogo ──────────────────────────────────────────────
 * Dois caracteres por Word, minúsculas normalizadas: o SQL não distingue maiúsculas
 * no identificador, e comparar sem normalizar recusaria `FROM T` depois de
 * `CREATE TABLE t` — que é uma recusa errada, e das que só aparecem em produção. */
static char baixa1(char c){ return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c; }

static void cat_nome_grava(const char *nome){
    char n[S_NOME_N * 2 + 1];
    int i;
    snprintf(n, sizeof n, "%s", nome ? nome : "");
    for(i = 0; n[i]; i++) n[i] = baixa1(n[i]);
    for(i = (int)strlen(n); i < S_NOME_N * 2; i++) n[i] = 0;
    for(i = 0; i < S_NOME_N; i++)
        mem_grava(S_NOME + (unsigned)i, w8((unsigned)(unsigned char)n[2*i],
                                          (unsigned)(unsigned char)n[2*i + 1]));
}

static void cat_nome_le(char *out, int cap){
    int i, j = 0;
    for(i = 0; i < S_NOME_N && j + 2 < cap; i++){
        Word w = mem_le(S_NOME + (unsigned)i);
        out[j++] = (char)w.total;
        out[j++] = (char)w.e;
    }
    if(j < cap) out[j] = 0; else out[cap - 1] = 0;
}

/* 1 = é esta tabela (ou a base é antiga e não tem nome guardado); 0 = não é. */
static int cat_nome_bate(const char *nome){
    char guardado[S_NOME_N * 2 + 2], pedido[S_NOME_N * 2 + 2];
    int i;
    cat_nome_le(guardado, (int)sizeof guardado);
    if(!guardado[0]) return 1;                    /* base sem nome: compatibilidade */
    snprintf(pedido, sizeof pedido, "%s", nome ? nome : "");
    for(i = 0; pedido[i]; i++) pedido[i] = baixa1(pedido[i]);
    return strcmp(guardado, pedido) == 0;
}

/* a recusa DIZ-SE, e diz qual é a tabela que existe — senão o erro não ajuda ninguém */
static int cat_nome_recusa(const char *nome){
    char guardado[S_NOME_N * 2 + 2];
    cat_nome_le(guardado, (int)sizeof guardado);
    printf("erro: a tabela «%s» não existe nesta base — a que existe é «%s».\n"
           " A consulta é RECUSADA, e nada é devolvido.\n", nome ? nome : "", guardado);
    if(sql_cap){
        sql_cap->ok = 0;
        snprintf(sql_cap->err, sizeof sql_cap->err,
                 "relation \"%s\" does not exist", nome ? nome : "");
    }
    return 0;
}

/* ── OS NOMES DAS COLUNAS ─────────────────────────────────────────────────── */
static void col_nome_grava(int j, const char *nome){
    char n[S_COLNOME_W * 2 + 1];
    unsigned i;
    if(j < 0 || (unsigned)j >= S_COLNOME_N) return;
    snprintf(n, sizeof n, "%s", nome ? nome : "");
    for(i = 0; n[i]; i++) n[i] = baixa1(n[i]);
    for(i = (unsigned)strlen(n); i < S_COLNOME_W * 2; i++) n[i] = 0;
    for(i = 0; i < S_COLNOME_W; i++)
        mem_grava(S_COLNOME + (unsigned)j * S_COLNOME_W + i,
                  w8((unsigned)(unsigned char)n[2*i], (unsigned)(unsigned char)n[2*i + 1]));
}

static void col_nome_le(int j, char *out, int cap){
    int k = 0;
    unsigned i;
    if(cap > 0) out[0] = 0;
    if(j < 0 || (unsigned)j >= S_COLNOME_N) return;
    for(i = 0; i < S_COLNOME_W && k + 2 < cap; i++){
        Word w = mem_le(S_COLNOME + (unsigned)j * S_COLNOME_W + i);
        out[k++] = (char)w.total;
        out[k++] = (char)w.e;
    }
    if(k < cap) out[k] = 0; else out[cap - 1] = 0;
}

/* a tabela tem nomes guardados? (base antiga: não) */
static int col_tem_nomes(void){
    char n[S_COLNOME_W * 2 + 2];
    col_nome_le(0, n, (int)sizeof n);
    return n[0] != 0;
}

/* nome → índice da coluna. −1 quando não é coluna desta tabela.
 *
 * A LETRA CONTINUA A VALER, e tem de continuar: as bases antigas não têm nomes
 * guardados, e todos os medidores da casa escrevem `(a,b,c)`. Mas ela é o RECURSO,
 * não a regra — havendo nomes, é o nome que decide, e um `WHERE z` numa tabela de
 * três colunas passa a ser recusado em vez de pedir a coluna 25. */
static int col_indice(const char *nome){
    char alvo[64], guardado[S_COLNOME_W * 2 + 2];
    int i, j;
    long ncols;
    if(!nome || !nome[0]) return -1;
    for(i = 0; nome[i] && i < (int)sizeof alvo - 1; i++) alvo[i] = baixa1(nome[i]);
    alvo[i] = 0;
    ncols = mem_le(S_CAT).total;
    for(j = 0; j < (int)S_COLNOME_N && j < ncols; j++){
        col_nome_le(j, guardado, (int)sizeof guardado);
        if(guardado[0] && !strcmp(guardado, alvo)) return j;
    }
    if(col_tem_nomes()) return -1;              /* tem nomes e não é nenhum deles */
    if(alvo[1] == 0 && alvo[0] >= 'a' && alvo[0] <= 'z') return alvo[0] - 'a';
    return -1;
}

/* ── A COLUNA LARGA, E PORQUE É QUE ELA NÃO SE COMPARA ────────────────────────
 * O avaliador do WHERE é polinomial e de OITO bits: o `S_MT` mascara o `.e` e os
 * produtos são de átomo. Uma coluna cujos valores passem de 255 não cabe nele —
 * e a diferença entre recusar e não recusar é a diferença entre um banco que diz
 * o que não sabe e um que responde à toa.
 *
 * Isto é o outro lado do §26: a aritmética de 16 bits já está provada, e o que
 * falta é o CAMINHO — o avaliador inteiro, que é o trio seguinte. Até lá o dado
 * guarda-se com 16 bits e a comparação diz que não chega lá. */
static int col_larga(long j, long ncols, long nrows){
    for(long i = 0; i < nrows; i++)
        if(mem_le(S_ALTO + (unsigned)(i*ncols + j)).total != 0) return 1;
    return 0;
}

/* A BARREIRA DO BANCO: dado, fsync, ponteiro, fsync.
 *
 * banco.c já tinha esta disciplina e o SQL não: aqui as células e o catálogo iam no MESMO
 * programa, sem nada entre eles. A ordem do programa estava certa, mas ordem de programa não
 * é ordem no disco — sem fsync o sistema pode pôr o catálogo no prato antes das células, e
 * uma queda no meio deixa o catálogo a contar uma linha que não existe.
 *
 * Com a barreira, as duas quedas possíveis são as duas boas: antes do ponteiro, a linha é
 * invisível e não faz mal nenhum; depois dele, a linha está inteira. Nunca meia. */
static void barreira(void){ if(fmem >= 0) fsync(fmem); }

/* Travamento injetado, para MEDIR a barreira em vez de a afirmar. SQL_TRAVA=1 mata o processo
 * logo depois do dado e antes do ponteiro — que é o único ponto onde a ordem importa.
 * _exit(9) modela queda de PROCESSO: o que já foi para o núcleo sobrevive. Queda de energia
 * é mais dura, e é contra ela que o fsync existe; o teste cobre a ordem, não o prato. */
static int trava_em = 0;               /* o teste põe aqui, sem depender do ambiente */
static void trava_se_pedido(int ponto){
    const char *e = getenv("SQL_TRAVA");
    if(trava_em == ponto || (e && atoi(e) == ponto)) _exit(9);
}
static unsigned char prog_le(unsigned pc){
    unsigned char b = OP_HALT;
    if(pread(fprog, &b, 1, (off_t)pc) != 1) return OP_HALT;
    return b;
}

/* ULA Word_8: a+b = (a⊕b)+2(a∧b) — naturais thm:transporte. NÃO é “carry” de CPU:
 * é a mesma peça que sobe ℕ. GOLD = ×σ com σ²=σ+1. ∞+1=−1 (Möbius, §M8). */
static Word8 ula_nand_w(Word8 a, Word8 b){ return (Word8)~(a & b); }
static Word8 ula_and_w (Word8 a, Word8 b){ Word8 n = ula_nand_w(a,b); return ula_nand_w(n,n); }
static Word8 ula_xor_w (Word8 a, Word8 b){ Word8 n = ula_nand_w(a,b);
                                        return ula_nand_w(ula_nand_w(a,n), ula_nand_w(b,n)); }
static Word8 ula_soma_w(Word8 a, Word8 b){
    Word8 s = ula_xor_w(a,b), c = ula_and_w(a,b);   /* c = termo 2(a∧b), thm:transporte */
    for(int i = 0; i < 8 && c; i++){
        c = (Word8)(c << 1);
        Word8 s2 = ula_xor_w(s,c), c2 = ula_and_w(s,c);
        s = s2; c = c2;
    }
    return s;
}
static Word ula_add(Word a, Word b){
    Word r = { ula_soma_w(a.total,b.total), ula_soma_w(a.e,b.e) }; return r; }
static Word ula_sub(Word a, Word b){
    Word r = { ula_soma_w(a.total, ula_soma_w(ula_nand_w(b.total,b.total),1)),
               ula_soma_w(a.e,     ula_soma_w(ula_nand_w(b.e,b.e),1)) }; return r; }
static Word ula_nand(Word a, Word b){ Word r = { ula_nand_w(a.total,b.total), ula_nand_w(a.e,b.e) }; return r; }
static Word ula_and(Word a, Word b){ Word n = ula_nand(a,b); return ula_nand(n,n); }
static Word ula_or (Word a, Word b){ Word na = ula_nand(a,a), nb = ula_nand(b,b);
                                     return ula_nand(na,nb); }
static Word ula_xor(Word a, Word b){ Word r = { ula_xor_w(a.total,b.total), ula_xor_w(a.e,b.e) }; return r; }
static int  zero(Word w){ return w.total == 0 && w.e == 0; }

/* ══ O ANDAR SEGUINTE DA TORRE: o transporte ATRAVESSA o átomo ═══════════════
 *
 * A ULA de cima soma componente a componente e o transporte NÃO passa de um
 * átomo para o outro — e isso está certo, porque a Word é um PAR (numerador,
 * denominador) e não um número de dezasseis bits. É por isso que uma célula
 * segura 0..255 e que o `INSERT ... VALUES (500,...)` é recusado.
 *
 * Para a célula crescer, o par tem de poder ser lido também como UM número —
 * e isso é exactamente a dobra da torre: T_{k+1} = T_k + T_k*. O transporte
 * dentro do átomo é `thm:transporte`, a + b = (a⊕b) + 2(a∧b); o transporte
 * ENTRE átomos é a mesma lei um andar acima, com o vai-um do primeiro a entrar
 * como cin do segundo. Nada aqui usa o `+` do C: tudo sai do NAND.
 *
 * Isto é o andar, medido sozinho. A célula continua onde estava — o caminho
 * novo primeiro, o velho intacto, que é como o pgwire entrou. */
typedef struct { Word8 baixo, alto; } W16;    /* Word_8² lida como um número */

static inline W16 w16_de(unsigned v){
    W16 r; r.baixo = (Word8)(v & 255u); r.alto = (Word8)((v >> 8) & 255u); return r;
}
static inline unsigned w16_val(W16 w){
    return (unsigned)w.baixo | ((unsigned)w.alto << 8);
}

/* a soma de oito bits COM o vai-um à entrada e à saída — só NAND lá dentro */
static Word8 ula_soma_c(Word8 a, Word8 b, Word8 cin, Word8 *cout){
    Word8 s = 0, c = cin;
    for(int i = 0; i < 8; i++){
        Word8 ai = (Word8)((a >> i) & 1u), bi = (Word8)((b >> i) & 1u);
        Word8 x  = ula_xor_w(ai, bi);
        Word8 si = ula_xor_w(x, c);
        /* vai-um = (ai∧bi) ∨ (c∧(ai⊕bi)), escrito só com NAND */
        Word8 c1 = ula_and_w(ai, bi), c2 = ula_and_w(c, x);
        c = ula_nand_w(ula_nand_w(c1, c1), ula_nand_w(c2, c2));
        s = (Word8)(s | (Word8)(si << i));
    }
    *cout = (Word8)(c & 1u);
    return s;
}

/* e a DOBRA: o vai-um do átomo baixo entra no alto. É o mesmo teorema, um
 * andar acima — e é isto que faz de dois átomos um número. */
static W16 ula_add16(W16 a, W16 b, Word8 *transbordo){
    W16 r; Word8 c1 = 0, c2 = 0;
    r.baixo = ula_soma_c(a.baixo, b.baixo, 0, &c1);
    r.alto  = ula_soma_c(a.alto,  b.alto,  c1, &c2);
    if(transbordo) *transbordo = c2;          /* o que sai do andar diz-se */
    return r;
}
/* a − b = a + (~b) + 1, com o mesmo vai-um a atravessar */
static W16 ula_sub16(W16 a, W16 b, Word8 *empresta){
    W16 nb; Word8 c1 = 0, c2 = 0; W16 r;
    nb.baixo = ula_nand_w(b.baixo, b.baixo);
    nb.alto  = ula_nand_w(b.alto,  b.alto);
    r.baixo = ula_soma_c(a.baixo, nb.baixo, 1, &c1);
    r.alto  = ula_soma_c(a.alto,  nb.alto,  c1, &c2);
    if(empresta) *empresta = (Word8)(c2 ? 0u : 1u);   /* sem vai-um final = pediu emprestado */
    return r;
}
/* a < b sem formar a diferença fora do andar: é o empréstimo do sub */
static int ula_menor16(W16 a, W16 b){
    Word8 emp = 0;
    (void)ula_sub16(a, b, &emp);
    return emp != 0;
}
static int ula_igual16(W16 a, W16 b){ return a.baixo == b.baixo && a.alto == b.alto; }

/* ══ A DOBRA DO REI — GOLD em dezasseis bits ═════════════════════════════════
 *
 * A ISA não tem MUL. `n·x` faz-se por ZECKENDORF: todo inteiro é soma de
 * Fibonacci NÃO consecutivos, de um único jeito, e cada F(k) é uma potência do
 * rei — GOLD = (a,b) ↦ (a+b, a), com σ² = σ + 1. Partindo de (x, 0):
 *
 *     (x,0) → (x,x) → (2x,x) → (3x,2x) → (5x,3x) → …
 *      k aplicações dão  a = F(k+1)·x
 *
 * Em oito bits o estado do rei é a Word: `a` no `.total` e `b` no `.e`. É por
 * isso que o segundo átomo NÃO está livre — e foi isso que travou o alargamento
 * do avaliador (§27).
 *
 * A dobra é a mesma do §26, aplicada ao REI em vez de à soma: o estado passa a
 * ser um PAR de valores de dezasseis bits, e o `+` lá dentro é o `ula_add16`,
 * onde o vai-um atravessa o átomo. Nada aqui usa o `+` do C sobre os valores.
 *
 * E o que sai do andar DIZ-SE: `n·x` acima de 65535 marca transbordo e não
 * enrola — é a mesma regra que o §24 pôs na célula. */
typedef struct { W16 a, b; } G16;          /* o estado do rei, um andar acima */

static G16 gold16(G16 w, Word8 *transbordo){
    G16 r; Word8 t = 0;
    r.a = ula_add16(w.a, w.b, &t);         /* a' = a + b, com o vai-um a atravessar */
    r.b = w.a;                             /* b' = a — o deslocamento */
    if(transbordo && t) *transbordo = 1;
    return r;
}

/* F(k+1)·x por k deslocamentos do rei, partindo de (x, 0) */
static W16 fib_vezes16(W16 x, int k, Word8 *transbordo){
    G16 w; w.a = x; w.b = w16_de(0);
    for(int t = 0; t < k; t++) w = gold16(w, transbordo);
    return w.a;
}

/* n·x = Σ F(k_i)·x, com n em Zeckendorf — o mesmo desenho do `emit_mul_zeck`,
 * um andar acima. O custo é o número de dígitos de Zeckendorf, ~log_φ(n). */
static W16 mul16_zeck(unsigned n, W16 x, Word8 *transbordo){
    unsigned fib[24]; int nf = 2;
    W16 acc = w16_de(0);
    fib[0] = 1; fib[1] = 2;                     /* F(2), F(3), … */
    while(fib[nf-1] <= n/2 + 1 && nf < 23){ fib[nf] = fib[nf-1] + fib[nf-2]; nf++; }
    unsigned r = n;
    for(int i = nf-1; i >= 0 && r > 0; i--){
        if(fib[i] > r) continue;
        r -= fib[i];
        /* fib[i] = F(i+2), logo são i+1 deslocamentos — errar isto acerta só em n = 1 */
        W16 parcela = fib_vezes16(x, i + 1, transbordo);
        Word8 t = 0;
        acc = ula_add16(acc, parcela, &t);
        if(transbordo && t) *transbordo = 1;
    }
    return acc;
}
/* GOLD: A_1 = [[1,1],[1,0]] — ×σ, σ²=σ+1. NEGRO: inversa (det −1). Envelope Word_8. */
static Word cifra_an(Word w, int n){
    Word r = { (Word8)((int)n*(int)w.total + (int)w.e), w.total }; return r; }
static Word decifra_an(Word w, int n){
    Word r = { w.e, (Word8)((int)w.total - (int)n*(int)w.e) }; return r; }
static unsigned MOVE_exec(Regs *r, unsigned pc, int sentido){
    unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1) << 8);
    pc += 2;
    if(sentido > 0){ r->B = r->A; r->A = mem_le(slot); }
    else           { mem_grava(slot, r->R); }
    return pc;
}

/* o salto, um so': avanca 1+rel se `cond`, senao avanca 1. JMP, JZ e JNZ chamam-no
 * todos — sao a MESMA operacao com condicoes diferentes. E' o primeiro passo da reducao
 * da ISA: o excedente vira funcao, e so' depois se troca. */
/* gira a palavra: (a,b) -> (s*b, a). Com s = -1 e' o J (det +1, o esquilo, i);
 * com s = +1 e' a reflexao (det -1, a troca). UMA operacao, o sinal decide qual. */
static Word corpo_gira(Word w, int s){ Word r = { (Word8)(s * (int)w.e), w.total }; return r; }

/* MOVER: poe `destino` no pc se `cond`, senao poe `senao`. E' a transferencia com o pc
 * como slot — a Lei 1 aplicada ao proprio contador de programa. */
static int MOVE_no_pc(Regs *r, unsigned destino, unsigned senao, int cond){
    r->pc = cond ? destino : senao;
    return 1;
}

static int MOVE_pc(Regs *r, unsigned pc, int cond){
    int rel = (signed char)prog_le(pc);
    /* ── E O SALTO E' A MESMA TRANSFERENCIA: o pc e' so' mais um destino ────────────
     * A Lei 1 outra vez — 1† = -1, e a unidade e' dual. Mover um valor para um slot e
     * mover um valor para o pc sao a MESMA operacao; o que muda e' o DESTINO, e o
     * destino e' parametro. A condicao decide SE se move; o sentido, para onde.
     *
     *   destino = slot   e sentido -1   ->  era STORE
     *   destino = slot   e sentido +1   ->  era LOAD
     *   destino = pc     e sentido -1   ->  era o SALTO
     *
     * Escrito assim, a ISA tem UMA operacao: mover, com destino e sentido. */
    return MOVE_no_pc(r, (unsigned)((int)pc + 1 + rel), pc + 1, cond);
}

static int passo(Regs *r, unsigned prog_len){
    if(r->pc >= prog_len) return 0;
    unsigned pc = r->pc;
    unsigned char op = prog_le(pc++);
    switch(op){
    case OP_HALT: return 0;
    /* LOAD e STORE sao UMA transferencia, e o SENTIDO e' o sinal — a Lei 1 em codigo:
     *   +1 do slot para o registo (o gerador, fp -> 1);  -1 ao contrario (o motor). */
    case OP_LOAD: case OP_LOADS: pc = MOVE_exec(r, pc, +1); break;
    case OP_STORE:               pc = MOVE_exec(r, pc, -1); break;
    case OP_GOLD:   r->A = cifra_an(r->A, 1); r->R = r->A; break;

    case OP_NEGRO_OURO:   r->A = decifra_an(r->A, 1); r->R = r->A; break;

    /* O CIRCUITO. Com o gato sozinho a máquina só estica — e o que estica não fecha grupo.
     * ESQUILO é ×ω do cristalino com t=0, isto é S = [[0,−1],[1,0]]: det +1, ordem 4. TROCA
     * é J = [[0,1],[1,0]]: det −1, ordem 2. Com os três, toda unimodular é palavra. */
    /* ── ESQUILO e TROCA sao A MESMA operacao, e o sinal e' argumento ──────────────
     * ESQUILO: (a,b) -> (-b, a)   e' a multiplicacao por i — o J da Lei 2, det +1
     * TROCA:   (a,b) -> ( b, a)   e' a reflexao,                             det -1
     * Diferem SO' no sinal do primeiro componente. Escritos assim deixam de ter corpo
     * proprio, como os tres saltos: uma funcao, um sinal, e a troca fica mecanica. */
    case OP_ESQUILO: { Word w = corpo_gira(r->A, -1); r->A = w; r->R = w; break; }
    case OP_TROCA:   { Word w = corpo_gira(r->A, +1); r->A = w; r->R = w; break; }
    /* O MARTELO. A faixa é [A, B); o cabeçalho está em S_CAB e o alvo em S_ALVO.
     *
     * O hash sai em bytes porque é A REDE que o define — essa coordenada não é minha, e
     * re-coordená-la daria outro hash e nenhuma share. Mas ele ENTRA NO MINERAL logo à saída, e
     * a decisão faz-se na RÉGUA: a régua elíptica (0,1), N(a,b) = a² + b², cuja norma é definida
     * positiva — nada tem norma zero fora do zero, logo NÃO HÁ CONE NULO por onde passar sem
     * trabalho. O alvo é um nível dessa norma, e a dificuldade é o RAIO da bola.
     *
     * Deixa em R o nonce que bateu, ou 0 se a faixa saiu limpa. Zero é resposta, não falha: uma
     * faixa limpa é trabalho feito, e é por isso que ela se fecha na mesma. */
    /* A DOBRA, NO METAL. Merkle nao e conta: e DESDOBRAMENTO AUTO-SIMILAR — pares que se juntam
     * num, nivel a nivel, com a mesma operacao em todos, ate sobrar um. E o `M_k = M_{k-1}A_1` de
     * sempre: o nivel k carrega o k-1.
     *
     * A = o slot base das folhas, B = quantas. Cada folha ocupa 32 átomos (32 bytes). Dobra em
     * lugar e deixa a raiz nos primeiros 32 átomos. Deixa em R quantos NIVEIS desdobrou — que e a
     * altura da arvore, e e o que a branch percorre ao subir.
     *
     * O OP_FOLD estava no enum desde sempre e sem executor. Deixa de estar. */
    case OP_FOLD: {
        unsigned base = atomos_le_u32(S_FOLD_ARG);
        unsigned n = r->B.total;
        unsigned niveis = 0;
        unsigned char a1[32], b1[32], h[32];
        while(n > 1){
            unsigned m = 0;
            for(unsigned i = 0; i < n; i += 2){
                atomos_le(base + 32u*i, a1, 32);
                if(i + 1 < n) atomos_le(base + 32u*(i+1), b1, 32);
                else memcpy(b1, a1, 32);
                unsigned char par[64];
                memcpy(par, a1, 32); memcpy(par+32, b1, 32);
                sha256(par, 64, h); sha256(h, 32, h);
                atomos_grava(base + 32u*m, h, 32);
                m++;
            }
            n = m; niveis++;
        }
        r->R = w8(niveis, 0);
        break;
    }
    case OP_MARTELO: {
        unsigned char cab[80], h1[32], h2[32], alvo[32];
        atomos_le(S_CAB, cab, 80);
        atomos_le(S_ALVO, alvo, 32);
        unsigned mid[8]; sha_ini(mid); sha_bloco(mid, cab);
        unsigned de = atomos_le_u32(S_FAIXA), ate = atomos_le_u32(S_FAIXA + 4u);
        Word achou = { 0, 0 };
        unsigned char b2[64];
        memcpy(b2, cab + 64, 16);
        memset(b2 + 16, 0, 48);
        b2[16] = 0x80;
        b2[62] = 0x02; b2[63] = 0x80;
        for(unsigned n = de; n != ate; n++){
            b2[12] = (unsigned char)(n);        b2[13] = (unsigned char)(n >> 8);
            b2[14] = (unsigned char)(n >> 16);  b2[15] = (unsigned char)(n >> 24);
            unsigned h[8]; memcpy(h, mid, sizeof h);
            sha_bloco(h, b2);
            sha_fim(h, h1);
            sha256(h1, 32, h2);
            int k = 0, dentro = 0;
            while(k < 32){
                unsigned t_h = h2[31-k], t_a = alvo[k];
                if(t_h != t_a){ dentro = (t_h < t_a); break; }
                k++;
            }
            if(dentro){
                atomos_u32(S_FAIXA + 8u, n);           /* nonce completo nos átomos */
                achou = w8(n & 0xFFu, (unsigned)k);   /* R: low + símbolo */
                break;
            }
        }
        r->R = achou; r->A = achou;
        break;
    }
    case OP_ADD: r->R = ula_add(r->A, r->B); break;
    case OP_SUB: r->R = ula_sub(r->A, r->B); break;
    case OP_AND: r->R = ula_and(r->A, r->B); break;
    case OP_OR:  r->R = ula_or (r->A, r->B); break;
    case OP_XOR: r->R = ula_xor(r->A, r->B); break;
    case OP_CMP: {
        unsigned char f = 0;
        if(zero(r->A) && zero(r->B)) f |= FL_ZERO;
        if(r->A.total == r->B.total && r->A.e == r->B.e) f |= FL_EQ;
        else if(r->A.total < r->B.total) f |= FL_LT;
        r->flags = f;
        break; }
    /* os TRES saltos sao UM: a condicao e' argumento, nao instrucao (ver MOVE_pc). */
    case OP_JMP: return MOVE_pc(r, pc, 1);
    case OP_JZ:  return MOVE_pc(r, pc,  (r->flags & FL_ZERO) != 0);
    case OP_JNZ: return MOVE_pc(r, pc, !(r->flags & FL_ZERO));
    default: return 0;
    }
    r->pc = pc;
    return 1;
}
static long rodar(unsigned prog_len){
    Regs r; memset(&r, 0, sizeof r);
    long passos = 0;
    while(passo(&r, prog_len)){ if(++passos > 50000000L) break; }
    return passos;
}

/* ---------------- o montador: escreve o bytecode NO DISCO ---------------- */
static unsigned pc_emit = 0;
static void emit1(unsigned char b){ pwrite(fprog, &b, 1, (off_t)pc_emit); pc_emit++; }
/* A VARREDURA É UMA PROGRESSÃO ARITMÉTICA NO ENDEREÇO.
 *
 * A ISA não tem endereçamento indireto — o slot é imediato na instrução (e LOADS, que eu
 * esperava que fosse indireto, é LOAD com a cifra espectral; fui ver em broca-so). Por isso o
 * compilador desenrolava a varredura: um bloco por linha, e o bytecode crescia LINEARMENTE com
 * a tabela. Medido antes desta mudança: 147 bytes por linha, 75 KB para 512 linhas.
 *
 * Mas o endereço de cada linha É uma PA. A linha i, coluna j, mora em S_LINHAS + i·ncols + j:
 * passo constante ncols. O bitmap e o vivo andam de 1 em 1. E o resto não anda.
 *
 * Então emite-se UM bloco — o da linha 0 — e anda-se com ele: antes de cada passagem, cada
 * endereço que depende da linha avança o seu passo. O bytecode passa a ser O(1) na tabela, e
 * quem varre é a progressão, não o compilador.
 *
 * O passo sai da FAIXA do slot, sem tocar em nenhum lugar de chamada: quem está nas linhas anda
 * ncols, quem está no bitmap ou no vivo anda 1, e os temporários e constantes não andam. */
#define NREL 512
typedef struct { unsigned off, base; long passo; } Rel;
static Rel *const rel = DISCO_FIXO(Rel, 25);
static int nrel = 0;
static long rel_ncols = 0;            /* > 0 só enquanto se emite o MOLDE */

static long passo_do_slot(unsigned s){
    if(!rel_ncols) return 0;
    if(s >= S_DEN)    return rel_ncols;
    if(s >= S_LINHAS) return rel_ncols;      /* a linha inteira: passo = ncols */
    if(s >= S_VIVO)   return 1;              /* o vivo: uma por linha           */
    if(s >= S_MATCH && s < S_VIVO) return 1; /* o bitmap: uma por linha         */
    return 0;                                /* constantes e rascunho: parados  */
}
/* ── MOVE: A OPERACAO, E E' UMA SO' ──────────────────────────────────────────────────
 *
 * O Aarao: "e' o paradigma novo da sexta dimensao — la', onde 1+2+3 = 6 = 3x2x1, o sistema
 * funciona e NAO EXISTEM DUAS OPERACOES. Em outras dimensoes ha' diferenca; nessa nao.
 * Entao e' redundancia, e nao comunica a mensagem da Lei: A UNIDADE E'."
 *
 * LOAD e STORE ficavam como dois NOMES para o mesmo — e dois nomes dizem "sao dois". A Lei
 * diz que e' um, e o par e' o que se LE' da unidade, nao duas coisas ao lado uma da outra.
 *
 *     MOVE(slot, +1)     do slot para o registo    (o velho LOAD)
 *     MOVE(slot, -1)     do registo para o slot    (o velho STORE)
 *
 * Os nomes velhos ficam guardados no enum e no montador, para que o bytecode antigo se leia
 * e a historia nao se apague — guarda-se o velho e usa-se o novo. Mas o codigo emite MOVER. */
static void emit_slot(unsigned char op, unsigned slot);
static void MOVE(unsigned slot, int sentido){
    emit_slot(sentido > 0 ? OP_LOAD : OP_STORE, slot);
}

static void emit_slot(unsigned char op, unsigned slot){
    long p = passo_do_slot(slot);
    emit1(op);
    if(p && nrel < NREL){ rel[nrel].off = pc_emit; rel[nrel].base = slot; rel[nrel].passo = p; nrel++; }
    emit1((unsigned char)(slot & 0xFF)); emit1((unsigned char)(slot >> 8));
}
/* A MERKLE PELA DOBRA DA MAQUINA. O st_merkle fazia o laco de SHAs a mao; sai.
 *
 * A subida da branch NAO e a dobra de uma arvore inteira — e a DOBRA DE DOIS, repetida: a raiz e
 * a folha, e cada ramo dobra-se com ela. Usa-se o OP_FOLD com n=2, tantas vezes quantos os ramos,
 * e quem dobra e a maquina. O coinbase continua a ser CONCATENACAO e o seu duplo SHA e a FOLHA —
 * isso nao e dobra, e o objeto de onde a dobra parte. */
static int coinbase_em_slots(void){
    int n = 0;
    #define POE(x) do{ unsigned char _b = (unsigned char)(x); atomos_grava(S_CB + (unsigned)n, &_b, 1); n++; }while(0)
    for(int k = 0; k < pool_st.n1; k++) POE(pool_st.cb1[k]);
    for(int k = 0; k < pool_st.en1_len/2; k++){
        int hi = hexval(pool_st.extranonce1[2*k]), lo = hexval(pool_st.extranonce1[2*k+1]);
        if(hi < 0 || lo < 0) break;
        POE((unsigned char)(hi*16 + lo));
    }
    { int e2 = pool_st.en2_size; if(e2 < 0 || e2 > 32) e2 = 4;
      for(int k = 0; k < e2; k++) POE(0); }
    for(int k = 0; k < pool_st.n2; k++) POE(pool_st.cb2[k]);
    #undef POE
    return n;
}
/* O SHA A CORRER SOBRE OS SLOTS. Ele processa blocos de 64 e nao precisa de ver o objeto todo —
 * le 64, comprime, le os 64 seguintes. O que resta em memoria e UM BLOCO, nao o coinbase. */
static void sha_dos_slots(unsigned base, int n, unsigned char *out){
    unsigned h[8]; sha_ini(h);
    unsigned char bl[64];
    int i = 0;
    while(n - i >= 64){
        atomos_le(base + (unsigned)i, bl, 64);
        sha_bloco(h, bl); i += 64;
    }
    unsigned char cauda[128]; memset(cauda, 0, sizeof cauda);
    int r = n - i;
    if(r) atomos_le(base + (unsigned)i, cauda, r);
    cauda[r] = 0x80;
    unsigned long bits = (unsigned long)n * 8;
    int tot = (r + 1 <= 56) ? 64 : 128;
    for(int k = 0; k < 8; k++) cauda[tot-1-k] = (unsigned char)(bits >> (8*k));
    sha_bloco(h, cauda);
    if(tot == 128) sha_bloco(h, cauda + 64);
    sha_fim(h, out);
}
/* POR BYTES NO BANCO: átomos físicos consecutivos. */
static void banco_poe(unsigned base, int off, const unsigned char *b, int n){
    atomos_grava(base + (unsigned)off, b, n);
}
static void merkle_pelo_fold(void){
    int n = coinbase_em_slots();                          /* a SOMA: slots consecutivos */
    unsigned char h[32], folha[32];
    sha_dos_slots(S_CB, n, h); sha256(h, 32, folha);      /* a FOLHA: o SHA sobre os slots */
    for(int k = 0; k < pool_st.n_ramos; k++){
        atomos_grava(S_FOLHA, folha, 32);
        atomos_grava(S_FOLHA + 32, pool_st.ramos + 32*k, 32);
        atomos_u32(S_FOLD_ARG, S_FOLHA);
        mem_grava(S_TMP + 1, w8(2, 0));
        pc_emit = 0;
        MOVE(S_TMP + 1, +1); MOVE(S_TMP + 1, +1);  /* B=n; base em átomos */
        emit1(OP_FOLD); emit1(OP_HALT);
        unsigned pl = pc_emit;
        Regs rg; memset(&rg, 0, sizeof rg);
        long ps = 0; while(passo(&rg, pl)){ if(++ps > 100000) break; }
        atomos_le(S_FOLHA, folha, 32);
    }
    memcpy(pool_st.merkle_raiz, folha, 32);
}
/* anda o molde uma linha: cada sítio de realocação avança o seu passo */
static void rel_anda(long i){
    for(int t = 0; t < nrel; t++){
        unsigned v = (unsigned)(rel[t].base + i * rel[t].passo);
        unsigned char lo = (unsigned char)(v & 0xFF), hi = (unsigned char)(v >> 8);
        pwrite(fprog, &lo, 1, (off_t)rel[t].off);
        pwrite(fprog, &hi, 1, (off_t)rel[t].off + 1);
    }
}
/* põe a constante do slot k no slot destino: LOAD k, LOAD zero, ADD, STORE dest */
static void emit_copia(unsigned de, unsigned para){
    MOVE(de, +1);
    MOVE(S_ZERO, +1);
    emit1(OP_ADD);
    MOVE(para, -1);
}

/* ---------------- SQL: o analisador ---------------- */
static void pula(const char **p){ while(**p && isspace((unsigned char)**p)) (*p)++; }
static int palavra(const char **p, const char *w){
    pula(p);
    size_t n = strlen(w);
    if(strncasecmp(*p, w, n) == 0 && (!isalnum((unsigned char)(*p)[n]))){ *p += n; return 1; }
    return 0;
}
static int numero(const char **p, long *v){
    pula(p);
    int sinal = 1;
    if(**p == '-'){ sinal = -1; (*p)++; }
    if(!isdigit((unsigned char)**p)) return 0;
    long x = 0;
    while(isdigit((unsigned char)**p)){ x = x*10 + (**p - '0'); (*p)++; }
    *v = sinal * x;
    return 1;
}
static int ident(const char **p, char *out, size_t cap){
    pula(p);
    size_t k = 0;
    if(**p == '*'){ (*p)++; snprintf(out, cap, "*"); return 1; }
    while(isalnum((unsigned char)**p) || **p == '_'){ if(k+1 < cap) out[k++] = **p; (*p)++; }
    out[k] = 0;
    return k > 0;
}

/* ---------------- os comandos ---------------- */
static int cria(const char *resto){
    const char *p = resto;
    char nome[64];
    if(!ident(&p, nome, sizeof nome)) return 0;
    /* a tabela é um ficheiro: abre-se (criando) ANTES de o catálogo lá ser escrito */
    if(!usa_tabela_z(nome, 1, 1)){          /* CREATE: limpa */
        printf("erro: não abri o ficheiro da tabela «%s»\n", nome);
        if(sql_cap){ sql_cap->ok = 0;
            snprintf(sql_cap->err, sizeof sql_cap->err, "cannot create relation \"%s\"", nome); }
        return 0;
    }
    pula(&p); if(*p != '(') return 0; p++;
    long ncols = 0; char c[64];
    long corpo[16], parm[16];
    while(1){
        if(!ident(&p, c, sizeof c)) break;
        col_nome_grava((int)ncols, c);       /* o nome da coluna passa a ficar guardado */
        corpo[ncols] = CORPO_INTEIRO; parm[ncols] = 0;   /* sem tipo = INTEIRO, como sempre foi */
        pula(&p);
        char tipo[64];
        const char *volta = p;
        if(isalpha((unsigned char)*p) && ident(&p, tipo, sizeof tipo)){
            int achou = 1;
            if(!strcasecmp(tipo,"RACIONAL"))      corpo[ncols] = CORPO_RACIONAL;
            else if(!strcasecmp(tipo,"AUREO"))  { corpo[ncols] = CORPO_AUREO;   parm[ncols] = 1; }
            else if(!strcasecmp(tipo,"MORFICO")){ corpo[ncols] = CORPO_MORFICO; parm[ncols] = 6; }
            /* PASSO 6: o cristalino. O parâmetro é o t da borda ω² = tω − 1 — t=0 Gauss ℤ[i],
             * t=1 Eisenstein ℤ[ω]. Predefine 0, que é o cristal quadrado. */
            else if(!strcasecmp(tipo,"CRISTALINO")){ corpo[ncols] = CORPO_CRISTAL; parm[ncols] = 0; }
            else if(!strcasecmp(tipo,"INTEIRO"))  corpo[ncols] = CORPO_INTEIRO;
            else { p = volta; achou = 0; }               /* não era tipo: devolve ao analisador */
            if(achou){
                pula(&p);
                if(*p == '('){ p++; long q; if(numero(&p, &q)) parm[ncols] = q; pula(&p);
                               if(*p == ')') p++; }
            }
        }
        ncols++; pula(&p);
        if(*p == ','){ p++; continue; } break;
    }
    /* o catálogo é escrito PELA MÁQUINA: constantes + STORE, compilado e executado */
    pc_emit = 0;
    Word w; w.total = ncols; w.e = 0; mem_grava(S_K, w);       /* a constante entra pela memória */
    w.total = 0; w.e = 0; mem_grava(S_ZERO, w);
    w.total = -1; w.e = 0; mem_grava(S_MT, w);      /* AND com isto zera o .e e guarda o total */
    w.total = 1; w.e = 0; mem_grava(S_UM, w);
    emit_copia(S_K, S_CAT);                                     /* cat.total = ncols */
    emit1(OP_HALT);
    rodar(pc_emit);
    Word cat = mem_le(S_CAT); cat.e = 0; mem_grava(S_CAT, cat); /* nrows = 0 */
    for(long j = 0; j < ncols && j < 8; j++){
        Word wc; wc.total = corpo[j]; wc.e = parm[j];
        mem_grava(S_CORPO + (unsigned)j, wc);
    }
    {
        static const char *nm[8] = {"INTEIRO","RACIONAL","AUREO","MORFICO","CRISTALINO",
                                    "INTEIRO","INTEIRO","INTEIRO"};
        printf("tabela %s criada: %ld colunas —", nome, ncols);
        for(long j = 0; j < ncols && j < 8; j++){
            printf(" %s", nm[corpo[j] & 7]);
            if(parm[j]) printf("(%ld)", parm[j]);
        }
        printf("\n");
    }
    cat_nome_grava(nome);            /* a relação passa a TER nome, e é este */
    if(sql_cap){
        snprintf(sql_cap->tag, sizeof sql_cap->tag, "CREATE TABLE");
        sql_cap->ncols = 0; sql_cap->nrows = 0;
    }
    return 1;
}

static int insere(const char *resto){
    const char *p = resto;
    char nome[64];
    if(!palavra(&p, "INTO")) return 0;
    if(!ident(&p, nome, sizeof nome)) return 0;
    if(!usa_tabela(nome, 0)) return cat_nome_recusa(nome);
    if(!cat_nome_bate(nome)) return cat_nome_recusa(nome);
    if(!palavra(&p, "VALUES")) return 0;
    pula(&p); if(*p != '(') return 0; p++;

    Word cat = mem_le(S_CAT);
    long ncols = cat.total, nrows = cat.e;
    long v[64], nv = 0;
    /* o PADRÃO do segundo componente vem do CORPO: no racional é denominador (1), no áureo é
     * o coeficiente de σ (0 — "5" é o inteiro 5, não 5+σ). O par é o mesmo; o que muda é o que
     * ele significa, e quem diz é a coluna. */
    long den[16];
    for(int q = 0; q < 16; q++){
        long cq = (q < 8) ? mem_le(S_CORPO + (unsigned)q).total : CORPO_INTEIRO;
        den[q] = (cq == CORPO_AUREO || cq == CORPO_CRISTAL) ? 0 : 1;
    }
    while(nv < ncols && numero(&p, &v[nv])){
        /* O VALOR RACIONAL. A Word tem duas componentes e um racional é um par: o numerador
         * no total e o denominador no e. Guarda-se a CLASSE — reduzida pelo mdc, denominador
         * positivo —, que é o que racional_pg.c §Q1 mediu ser o representante único. */
        const char *volta = p;
        pula(&p);
        if(*p == '/'){
            const char *ap = p + 1;
            long q;
            if(numero(&ap, &q) && q != 0){
                p = ap;
                /* PASSO 2: a classe vem do TOOLKIT, não de código repetido aqui. É a mesma
                 * ra_classe que o racional_pg.c mediu — uma implementação, não duas. */
                Par cls = ra_classe((Par){ v[nv], q });
                v[nv] = cls.a; den[nv] = cls.b;
            } else p = volta;
        } else {
            p = volta;
            /* PASSO 3: numa coluna AUREO, "a+bs" é o elemento a + bσ. O par já é o formato —
             * muda o que ele SIGNIFICA, e quem diz isso é o corpo declarado da coluna. */
            long cpj = (nv < 8) ? mem_le(S_CORPO + (unsigned)nv).total : CORPO_INTEIRO;
            if(cpj == CORPO_AUREO || cpj == CORPO_CRISTAL){
                pula(&p);
                if(*p == '+' || *p == '-'){
                    int neg = (*p == '-');
                    const char *ap = p + 1;
                    long bb;
                    if(numero(&ap, &bb)){
                        pula(&ap);
                        if(*ap == 's' || *ap == 'S'){
                            p = ap + 1;
                            den[nv] = neg ? -bb : bb;      /* o .e guarda o coeficiente de σ */
                        }
                    }
                }
            }
        }
        nv++; pula(&p); if(*p == ','){ p++; continue; } break;
    }
    if(nv != ncols){ printf("erro: a tabela tem %ld colunas, vieram %ld\n", ncols, nv); return 0; }

    /* ── O ENVELOPE DA CÉLULA DIZ-SE, E O QUE NÃO CABE É RECUSADO ────────────────
     * A célula é uma Word_8² — um átomo para o numerador, outro para o denominador
     * (ou para o coeficiente de σ, conforme o corpo). Logo cada componente vive em
     * 0..255, e é isso que o banco pode guardar hoje.
     *
     * O que estava a acontecer: `INSERT INTO t VALUES (500,1000,65535)` era aceite e
     * lido de volta como `244 232 255`. E `256` virava `0`, e `−1` virava `255`. O
     * valor entrava truncado, no disco, sem uma palavra — que é o defeito que esta
     * casa persegue em todo o lado menos aqui.
     *
     * A regra da casa é a mesma dos racionais e da FC: o que não cabe CONTA-SE e
     * RECUSA-SE, não se enrola calado. Alargar a célula é subir a torre (uma célula
     * de Word_8⁴), e é trabalho próprio — mas guardar lixo enquanto isso não é uma
     * alternativa: é perder o dado sem o dizer. */
    for(long j = 0; j < ncols; j++){
        /* e o alcance é o do CORPO: num inteiro o envelope é 0..255 (o Word_8 da
         * casa), num racional/áureo/cristal o componente é assinado, −128..127 */
        long cpj = (j < 8) ? mem_le(S_CORPO + (unsigned)j).total : CORPO_INTEIRO;
        int assinado = (cpj == CORPO_RACIONAL || cpj == CORPO_AUREO || cpj == CORPO_CRISTAL
                        || den[j] > 1 || den[j] < 0);
        /* a coluna inteira passa a segurar 16 bits — o byte alto vai para S_ALTO */
        long lo = assinado ? -128 : 0, hi = assinado ? 127 : 65535;
        long lo2 = (cpj == CORPO_AUREO || cpj == CORPO_CRISTAL) ? -128 : 1;
        long hi2 = (cpj == CORPO_AUREO || cpj == CORPO_CRISTAL) ? 127 : 255;
        if(v[j] < lo || v[j] > hi || den[j] < lo2 || den[j] > hi2){
            printf("erro: o valor da coluna %ld não cabe no envelope Word_8 da célula"
                   " (numerador %ld, segundo componente %ld; o envelope é %ld..%ld).\n"
                   " A linha é RECUSADA e nada é escrito — alargar a célula é subir a"
                   " torre, e não truncar em silêncio.\n", j, v[j], den[j], lo, hi);
            if(sql_cap){
                sql_cap->ok = 0;
                snprintf(sql_cap->err, sizeof sql_cap->err,
                         "value out of range for Word_8 cell: column %ld got %ld", j, v[j]);
            }
            cel_recusadas++;
            return 0;
        }
    }

    /* compila o INSERT: cada valor entra por um slot de constante e é gravado pela ISA */
    pc_emit = 0;
    Word w = {0,0}; mem_grava(S_ZERO, w);
    for(long j = 0; j < ncols; j++){
        w.total = v[j]; w.e = den[j];      /* e = denominador; 1 para inteiro */
        mem_grava(S_K + (unsigned)j, w);                        /* a constante, na memória */
        emit_copia(S_K + (unsigned)j, S_LINHAS + (unsigned)(nrows*ncols + j));
        /* o byte ALTO no plano paralelo — a Word da linha não muda, e por isso
         * toda a aritmética emitida continua a ler exactamente o que sempre leu */
        { Word wa; wa.total = (Word8)(((unsigned long)v[j] >> 8) & 255u); wa.e = 0;
          mem_grava(S_ALTO + (unsigned)(nrows*ncols + j), wa); }
        Word wd; wd.total = den[j]; wd.e = 0;
        mem_grava(S_KZ + (unsigned)j, wd);
        emit_copia(S_KZ + (unsigned)j, S_DEN + (unsigned)(nrows*ncols + j));
    }
    /* nrows++ pela própria máquina: LOAD cat, LOAD um, ADD, STORE — mas nrows é o campo .e,
     * e a ULA soma componente a componente; então a constante um vai no campo .e. */
    /* a linha nasce VIVA (o DELETE zera este slot).
     * ATENÇÃO à ordem: emit_copia só EMITE; o programa roda depois, no rodar(). Se a
     * constante for sobrescrita entre as duas fases, a máquina lê o valor trocado — foi
     * exatamente o que aconteceu aqui quando S_UM servia às duas coisas. Cada constante
     * tem o seu slot. */
    w.total = 1; w.e = 0; mem_grava(S_UM, w);
    emit_copia(S_UM, S_VIVO + (unsigned)nrows);
    emit1(OP_HALT);
    long passos = rodar(pc_emit);        /* FASE 1: só o dado */

    barreira();                          /* o dado está no prato antes de existir o ponteiro */
    trava_se_pedido(1);                  /* e é aqui que o teste derruba, para ver o que sobra */

    /* FASE 2: só então o ponteiro. nrows vive no campo .e, e a ULA soma componente a
     * componente — por isso o incremento é uma constante com .e = 1. */
    pc_emit = 0;                         /* fase 2 é um programa PRÓPRIO: rodar() parte de 0 */
    w.total = 0; w.e = 1; mem_grava(S_UME, w);
    MOVE(S_CAT, +1);
    MOVE(S_UME, +1);
    emit1(OP_ADD);
    MOVE(S_CAT, -1);
    emit1(OP_HALT);
    passos += rodar(pc_emit);
    barreira();

    cat = mem_le(S_CAT);
    printf("1 linha inserida (%ld colunas) — %u bytes de ISA, %ld passos; agora %d linhas\n",
           ncols, pc_emit, passos, cat.e);
    if(sql_cap){
        snprintf(sql_cap->tag, sizeof sql_cap->tag, "INSERT 0 1");
        sql_cap->ncols = 0; sql_cap->nrows = 0;
    }
    return 1;
}

/* As três ações que uma varredura pode ter na linha que casa. */
enum { ACAO_MARCA, ACAO_SET, ACAO_APAGA };

/* Emite o teste da condição e o bloco de ação para UMA linha.
 *
 * A ISA não tem salto por FL_LT — só JZ/JNZ, que olham FL_ZERO. Então as três comparações
 * reduzem-se todas a um teste de ZERO, e a diferença entre elas é só ordem de subtração:
 *
 *   col = k   ->  dif = k − col ;                CMP com A=0  ->  FL_ZERO sse igual   (JZ)
 *   col < k   ->  dif = col − k ; dif AND sinal; CMP com A=0  ->  FL_ZERO sse col ≥ k (JNZ)
 *   col > k   ->  dif = k − col ; dif AND sinal; CMP com A=0  ->  FL_ZERO sse col ≤ k (JNZ)
 *
 * O sinal extrai-se por AND com o bit 63 — a ISA não tem deslocamento, mas tem AND, e isso
 * basta: negativo é exatamente quem tem esse bit aceso.
 */
/* A árvore do WHERE. Com parênteses, a cláusula deixa de ser plana e vira árvore de
 * verdade — e a gramática é a do SQL:
 *
 *     expr  := termo (OR termo)*
 *     termo := fator (AND fator)*
 *     fator := '(' expr ')' | coluna <op> número
 *
 * Os seis operadores são TRÊS mais uma negação: != é não-=, <= é não->, >= é não-<. E negar
 * um slot que vale 0 ou 1 é XOR com 1 — opcode que a ISA já tem. Não se inventou comparação
 * nova: acrescentou-se um XOR.
 */
#define NCOL 6                 /* colunas que uma expressão pode citar */
#define CMAX 8                 /* (histórico: era o teto da soma repetida — ver emit_mul_zeck) */

#define NI    (NCOL+1)         /* símbolo 0 = a constante 1; 1..NCOL = as colunas */
#define KGRAU 3                /* ordem do tensor: grau máximo do monômio          */
#define NMON  343              /* NI^KGRAU — as casas do multi-índice (7^3)        */

/* O TENSOR DE GRAU k, com MULTI-ÍNDICE.
 *
 * Um monômio é uma tupla ORDENADA de KGRAU símbolos, e o símbolo 0 é a constante 1 — logo a
 * tupla (0,0,0) é o termo constante, (0,0,i) é linear, (0,i,j) é quadrático, (i,j,l) é cúbico.
 * Graus diferentes não são casos diferentes: são a mesma tabela com mais ou menos zeros.
 *
 * A posição ORDENADA é o endereço, e é só isso que faz a comutatividade desaparecer: as k!
 * escritas de um monômio caem na mesma casa por aritmética de índice, não por regra
 * (tools/tensor.c §T2, onde a contagem C(n+k−1,k) foi conferida).
 *
 * As duas operações são as do tensor: a SOMA soma casa a casa; o PRODUTO junta os multi-índices
 * (os graus somam) e multiplica os coeficientes. O parêntese entra como posição e sai. */
/* O TENSOR SOBRE ℚ: um denominador COMUM por tensor.
 *
 * As classes inteiras do racional_pg.c entram aqui: cada tensor é (coeficientes, denominador),
 * e o denominador é um só para o tensor inteiro — não um por monómio. Isso basta porque as duas
 * operações são lineares no denominador:
 *
 *     soma      cruza:      (A,p) + (B,q) = (qA + pB, pq)
 *     produto   multiplica: (A,p) · (B,q) = (A·B, pq)
 *
 * E na hora de emitir, o denominador SOME: a contração já pôs tudo de um lado e a comparação é
 * contra ZERO, logo (N/D) OP 0 ⟺ N OP 0 desde que D > 0 — e mantém-se D > 0 por construção.
 * O racional entra no analisador, opera como classe, e sai inteiro para o metal. */
struct tensor { long c[NMON]; long den; };

static void mi_ordena(int *d){                         /* ordenação por inserção, KGRAU pequeno */
    for(int i = 1; i < KGRAU; i++)
        for(int j = i; j > 0 && d[j] < d[j-1]; j--){ int t = d[j]; d[j] = d[j-1]; d[j-1] = t; }
}
static int mi_cod(int *d){                             /* a tupla ordenada é o endereço */
    mi_ordena(d);
    int r = 0;
    for(int t = KGRAU-1; t >= 0; t--) r = r*NI + d[t];
    return r;
}
static void mi_de(int cod, int *d){
    for(int t = 0; t < KGRAU; t++){ d[t] = cod % NI; cod /= NI; }
}
static int mi_grau(int cod){
    int d[KGRAU], g = 0; mi_de(cod, d);
    for(int t = 0; t < KGRAU; t++) if(d[t]) g++;
    return g;
}
static struct tensor ten_zero(void){ struct tensor t; memset(&t,0,sizeof t); t.den = 1; return t; }
/* o representante da classe: divide tudo pelo mdc comum, e deixa o denominador positivo */
static void ten_reduz(struct tensor *t){
    if(t->den < 0){ t->den = -t->den; for(int i = 0; i < NMON; i++) t->c[i] = -t->c[i]; }
    long g = t->den;
    for(int i = 0; i < NMON; i++) if(t->c[i]) g = rt_mdc(g, t->c[i]);
    if(g > 1){ t->den /= g; for(int i = 0; i < NMON; i++) t->c[i] /= g; }
    if(t->den == 0) t->den = 1;
}
static void ten_mon(struct tensor *t, int *d, long c){ t->c[mi_cod(d)] += c; }
static void ten_const(struct tensor *t, long k){
    int d[KGRAU]; memset(d, 0, sizeof d); ten_mon(t, d, k);
}
static void ten_var(struct tensor *t, int col){
    int d[KGRAU]; memset(d, 0, sizeof d); d[0] = col + 1; ten_mon(t, d, 1);
}
static struct tensor ten_soma(struct tensor a, struct tensor b, int sinal){
    long p = a.den ? a.den : 1, q = b.den ? b.den : 1;      /* (A,p) ± (B,q) = (qA ± pB, pq) */
    struct tensor r = ten_zero();
    for(int i = 0; i < NMON; i++) r.c[i] = q * a.c[i] + sinal * p * b.c[i];
    r.den = p * q;
    ten_reduz(&r);
    return r;
}
/* o produto: junta os multi-índices — os graus SOMAM. Passar de KGRAU é recusado, e dizer
 * isso é melhor que truncar em silêncio. */
static int ten_mul(struct tensor *r, const struct tensor *a, const struct tensor *b){
    *r = ten_zero();
    for(int x = 0; x < NMON; x++){
        if(!a->c[x]) continue;
        int dx[KGRAU]; mi_de(x, dx);
        for(int y = 0; y < NMON; y++){
            if(!b->c[y]) continue;
            int dy[KGRAU]; mi_de(y, dy);
            if(mi_grau(x) + mi_grau(y) > KGRAU) return 0;
            int d[KGRAU], k = 0;
            for(int t = 0; t < KGRAU; t++) if(dx[t]) d[k++] = dx[t];
            for(int t = 0; t < KGRAU; t++) if(dy[t]) d[k++] = dy[t];
            while(k < KGRAU) d[k++] = 0;
            ten_mon(r, d, a->c[x] * b->c[y]);
        }
    }
    r->den = (a->den ? a->den : 1) * (b->den ? b->den : 1);   /* os denominadores multiplicam */
    ten_reduz(r);
    return 1;
}
static int ten_constante(struct tensor t){
    for(int i = 1; i < NMON; i++) if(t.c[i]) return 0;
    return t.c[0] == t.c[0];
}

enum { NO_COND, NO_AND, NO_OR };
struct no {
    int tipo;
    int esq, dir;          /* índices na árvore                                   */
    int op;                /* '=', '<', '>'                                       */
    int nega;              /* 1 se o resultado deve ser invertido (!=, <=, >=)     */
    struct tensor v;       /* o lado numérico já contraído: L − R                  */
    int decidido;          /* 0 = precisa da linha; 1 = já se sabe (const)         */
    int valor;             /* se decidido: 0 ou 1                                  */
    int atomo;             /* índice do átomo distinto, depois da contração        */
};
struct arvore {
    struct no no[MAXNO];
    int n;
    int raiz;
    /* a CONTRAÇÃO: os átomos distintos, depois de normalizar a árvore */
    int natomo;
    unsigned long asig[16];
    int aop[16], anega[16];
    struct tensor av[16];
};
static int le_expr(const char **p, struct arvore *a);
/* as colunas que o WHERE cita — usada pela guarda que liga a DISTÂNCIA ao WHERE */
static unsigned citadas_where = 0;

static int novo_no(struct arvore *a){
    if(a->n >= MAXNO) return -1;
    memset(&a->no[a->n], 0, sizeof a->no[0]);
    return a->n++;
}
/* expressão numérica com PRODUTO e parênteses:
 *     num   := termo (('+'|'-') termo)*
 *     termo := fator ('*' fator)*
 *     fator := '(' num ')' | inteiro | coluna
 * Contrai enquanto lê: soma acumula na casa, produto multiplica os tensores. Cada parêntese
 * que entra num produto vira uma POSIÇÃO do tensor — é a regra de tools/tensor.c. */
static int le_soma(const char **p, struct tensor *t);

static int le_fator_num(const char **p, struct tensor *t){
    pula(p);
    if(**p == '('){
        (*p)++;
        if(!le_soma(p, t)) return 0;
        pula(p);
        if(**p != ')') return 0;
        (*p)++;
        return 1;
    }
    *t = ten_zero();
    if(isdigit((unsigned char)**p)){
        long k;
        if(!numero(p, &k)) return 0;
        ten_const(t, k);
        const char *volta = *p;                  /* é fração? só se vier / e depois número */
        pula(p);
        if(**p == '/'){
            const char *ap = *p + 1;
            long q;
            if(numero(&ap, &q) && q != 0){
                *p = ap;
                t->den = q;
                ten_reduz(t);
                return 1;
            }
        }
        *p = volta;
        return 1;
    }
    if(isalpha((unsigned char)**p)){
        char nome[64];
        if(!ident(p, nome, sizeof nome)) return 0;
        int col = col_indice(nome);          /* pelo NOME; a letra é o recurso das bases antigas */
        if(col < 0 || col >= NCOL) return 0;
        ten_var(t, col);                           /* o símbolo col+1 é a coluna */
        citadas_where |= 1u << col;                /* para a guarda de corpo — ver checa_corpos */
        return 1;
    }
    return 0;
}
static int le_produto(const char **p, struct tensor *t){
    if(!le_fator_num(p, t)) return 0;
    while(1){
        pula(p);
        if(**p == '*'){
            (*p)++;
            struct tensor u, r;
            if(!le_fator_num(p, &u)) return 0;
            if(!ten_mul(&r, t, &u)) return 0;      /* passou do grau máximo */
            *t = r;
            continue;
        }
        /* A DIVISÃO POR CONSTANTE. Com o tensor sobre ℚ ela é a recíproca, e nada mais: o
         * numerador vira denominador. Só se divide por CONSTANTE — dividir por uma coluna
         * daria uma função que não é polinómio, e o tensor não a representa; então recusa-se
         * em voz alta, que é melhor que aceitar e responder outra coisa.
         *
         * Antes desta linha o `/` depois de uma coluna era SALTADO em silêncio: `a / 2 > 2`
         * lia-se como outra coisa e devolvia seis linhas onde devia devolver duas. Silêncio
         * assim é o pior defeito que um banco pode ter. */
        if(**p == '/'){
            (*p)++;
            struct tensor u;
            if(!le_fator_num(p, &u)) return 0;
            if(!ten_constante(u) || u.c[0] == 0){
                printf("erro: só se divide por constante não-nula (o tensor é polinomial)\n");
                return 0;
            }
            long num = u.c[0], den = u.den ? u.den : 1;
            t->den *= (num < 0 ? -num : num);      /* recíproca: numerador vira denominador */
            for(int i = 0; i < NMON; i++) t->c[i] *= (num < 0 ? -den : den);
            ten_reduz(t);
            continue;
        }
        break;
    }
    return 1;
}
static int le_soma(const char **p, struct tensor *t){
    pula(p);
    int sinal = 1;
    if(**p == '-'){ sinal = -1; (*p)++; }
    else if(**p == '+'){ (*p)++; }
    if(!le_produto(p, t)) return 0;
    if(sinal < 0) *t = ten_soma(ten_zero(), *t, -1);
    while(1){
        pula(p);
        int s2;
        if(**p == '+') s2 = 1;
        else if(**p == '-') s2 = -1;
        else break;
        (*p)++;
        struct tensor u;
        if(!le_produto(p, &u)) return 0;
        *t = ten_soma(*t, u, s2);
    }
    return 1;
}
static int le_num(const char **p, struct tensor *v){ return le_soma(p, v); }

static int le_fator(const char **p, struct arvore *a){
    pula(p);
    /* O '(' é AMBÍGUO: pode abrir um grupo booleano — (a=3 OR b>5) — ou um fator numérico —
     * (a+b)*(a-b) > 0. Tenta-se primeiro a COMPARAÇÃO; se não fechar, volta-se ao ponto de
     * partida e lê-se como grupo. Sem este retrocesso, `(a+b)*(a-b) > 0` era lido como grupo,
     * falhava no ')' e a cláusula inteira caía fora. */
    const char *salvo = *p;
    int nsalvo = a->n;
    struct tensor L, R;
    if(le_num(p, &L)){
        pula(p);
        if(**p=='!' || **p=='<' || **p=='>' || **p=='=') goto tem_comparacao;
    }
    *p = salvo; a->n = nsalvo;
    if(**p == '('){
        (*p)++;
        int e = le_expr(p, a);
        pula(p);
        if(**p != ')') return -1;
        (*p)++;
        return e;
    }
    return -1;
tem_comparacao:;
    int op = 0, nega = 0;
    if(**p == '!' && (*p)[1] == '=')        { op = '='; nega = 1; *p += 2; }
    else if(**p == '<' && (*p)[1] == '=')   { op = '>'; nega = 1; *p += 2; }   /* ≤ é não-> */
    else if(**p == '>' && (*p)[1] == '=')   { op = '<'; nega = 1; *p += 2; }   /* ≥ é não-< */
    else if(**p == '=')                     { op = '='; (*p)++; }
    else if(**p == '<')                     { op = '<'; (*p)++; }
    else if(**p == '>')                     { op = '>'; (*p)++; }
    else return -1;
    if(!le_num(p, &R)) return -1;

    int i = novo_no(a); if(i < 0) return -1;
    struct no *n = &a->no[i];
    n->tipo = NO_COND; n->op = op; n->nega = nega;
    n->v = ten_soma(L, R, -1);                     /* L op R  ⟺  (L−R) op 0 */

    /* CANONIZAR o par (vetor, operador): sem isto, `b > 20` e `20 < b` são o mesmo fato
     * escrito com vetores opostos, e emitiriam bytecode diferente.
     *   v < 0  ⟺  (−v) > 0        — logo o '<' desaparece
     *   v = 0  ⟺  (−v) = 0        — logo o sinal do '=' é livre, e fixa-se */
    if(op == '<'){
        n->v = ten_soma(ten_zero(), n->v, -1);
        n->op = '>';
    } else if(op == '='){
        long primeiro = 0;
        for(int i = 1; i < NMON && !primeiro; i++) if(n->v.c[i]) primeiro = n->v.c[i];
        if(!primeiro) primeiro = n->v.c[0];
        if(primeiro < 0) n->v = ten_soma(ten_zero(), n->v, -1);
    }
    op = n->op;

    /* CONTRAÇÃO decide na compilação: se não sobrou variável, a resposta já se sabe.
     * E se der falso, isso não é erro do cliente — é uma CONDIÇÃO DE PARADA. */
    if(ten_constante(n->v)){
        long d = n->v.c[0];
        int vale = (op == '=') ? (d == 0) : (op == '<') ? (d < 0) : (d > 0);
        if(nega) vale = !vale;
        n->decidido = 1; n->valor = vale;
    }
    return i;
}
static int le_termo(const char **p, struct arvore *a){
    int e = le_fator(p, a);
    if(e < 0) return -1;
    while(palavra(p, "AND")){
        int d = le_fator(p, a);
        if(d < 0) return -1;
        int i = novo_no(a); if(i < 0) return -1;
        a->no[i].tipo = NO_AND; a->no[i].esq = e; a->no[i].dir = d;
        e = i;
    }
    return e;
}
static int le_expr(const char **p, struct arvore *a){
    int e = le_termo(p, a);
    if(e < 0) return -1;
    while(palavra(p, "OR")){
        int d = le_termo(p, a);
        if(d < 0) return -1;
        int i = novo_no(a); if(i < 0) return -1;
        a->no[i].tipo = NO_OR; a->no[i].esq = e; a->no[i].dir = d;
        e = i;
    }
    return e;
}
/* ---------------- A CONTRAÇÃO ----------------
 * Antes de emitir, a árvore é NORMALIZADA — do mesmo jeito que expressao.c contrai uma
 * expressão num tensor: quem é equivalente vira o MESMO objeto, e não duas coisas que uma
 * regra depois iguala. Três coisas acontecem, e nenhuma é reescrita ad hoc:
 *
 *   (1) cada condição vira um ÁTOMO com assinatura (coluna, operador, negação, constante).
 *       Átomos iguais são o MESMO átomo — logo `a=3 OR a=3` tem um átomo, não dois.
 *   (2) os filhos de AND/OR são ORDENADOS pela assinatura. Como as duas operações são
 *       comutativas, a ordem escrita é acidente; ordenar é escolher o representante.
 *   (3) A op A colapsa em A (idempotência), que cai sozinha depois de (1) e (2).
 *
 * O efeito medível: WHERE equivalentes escritos de formas diferentes emitem o MESMO bytecode,
 * byte a byte. O programa passa a depender da classe, não da escrita.
 */
static unsigned long mistura(unsigned long h, unsigned long v){
    h ^= v + 0x9e3779b97f4a7c15UL + (h << 6) + (h >> 2);
    return h;
}
static unsigned long sig_de(struct arvore *a, int i, unsigned long *cache){
    if(cache[i]) return cache[i];
    struct no *n = &a->no[i];
    unsigned long h;
    if(n->tipo == NO_COND){
        h = mistura(1469598103934665603UL, (unsigned long)n->op);
        h = mistura(h, (unsigned long)n->nega);
        h = mistura(h, (unsigned long)n->decidido);
        h = mistura(h, (unsigned long)n->valor);
        for(int i = 0; i < NMON; i++) if(n->v.c[i]) h = mistura(mistura(h,(unsigned long)i),
                                                                (unsigned long)n->v.c[i]);
    } else {
        unsigned long s1 = sig_de(a, n->esq, cache), s2 = sig_de(a, n->dir, cache);
        if(s1 > s2){ int t = n->esq; n->esq = n->dir; n->dir = t;   /* (2) ordena */
                     unsigned long ts = s1; s1 = s2; s2 = ts; }
        h = mistura(mistura((unsigned long)n->tipo + 7, s1), s2);
    }
    cache[i] = h ? h : 1;
    return cache[i];
}
static int normaliza(struct arvore *a, int i, unsigned long *cache){
    struct no *n = &a->no[i];
    if(n->tipo == NO_COND) return i;
    n->esq = normaliza(a, n->esq, cache);
    n->dir = normaliza(a, n->dir, cache);
    memset(cache, 0, sizeof(unsigned long) * MAXNO);
    unsigned long s1 = sig_de(a, n->esq, cache), s2 = sig_de(a, n->dir, cache);
    if(s1 == s2) return n->esq;                                     /* (3) A op A = A */
    /* (2) ordena TAMBÉM aqui: dentro de sig_de a troca só acontece ao descer num nó
     * composto, então a raiz — e todo nó visto de cima — ficava por ordenar. Era por isso
     * que `a=3 AND b>20` e `b>20 AND a=3` davam bytecode diferente com o mesmo resultado. */
    if(s1 > s2){ int t = n->esq; n->esq = n->dir; n->dir = t; }

    /* A ABSORÇÃO — e ela não é regra ad hoc: é a ADJUNÇÃO δ⊣ε do morfico.py.
     *
     * Na morfologia, o AND é a EROSÃO e o OR é a DILATAÇÃO, e a adjunção dá
     * γ = δε anti-extensiva e φ = εδ extensiva, ambas idempotentes. Em árvore isso lê-se:
     *
     *     (x ∧ y) ∨ x = x        o que a erosão tirou, a dilatação não repõe além de x
     *     (x ∨ y) ∧ x = x        e o simétrico
     *
     * A idempotência (A op A = A) já estava acima, e é γγ=γ. Faltava esta, que é a que
     * colapsa os DOIS níveis — e sem ela `(a>2 AND a<9) OR a>2` gastava 1010 bytes para
     * dizer o que `a>2` diz em 486.
     *
     * E é a mesma forma da contração numérica, do outro lado: ali o tensor apaga o que não
     * é invariante, aqui a adjunção apaga o que não muda o conjunto. */
    for(int lado = 0; lado < 2; lado++){
        int filho = lado ? n->dir : n->esq, outro = lado ? n->esq : n->dir;
        struct no *f = &a->no[filho];
        if(f->tipo == NO_COND) continue;
        /* o filho tem de ser do tipo OPOSTO ao pai: (x∧y)∨x, (x∨y)∧x */
        if(f->tipo == n->tipo) continue;
        memset(cache, 0, sizeof(unsigned long) * MAXNO);
        unsigned long so = sig_de(a, outro, cache);
        memset(cache, 0, sizeof(unsigned long) * MAXNO);
        unsigned long fe = sig_de(a, f->esq, cache);
        memset(cache, 0, sizeof(unsigned long) * MAXNO);
        unsigned long fd = sig_de(a, f->dir, cache);
        if(so == fe || so == fd) return outro;        /* absorve: o filho todo desaparece */
    }
    return i;
}
static void junta_atomos(struct arvore *a, int i, unsigned long *cache){
    struct no *n = &a->no[i];
    if(n->tipo != NO_COND){ junta_atomos(a, n->esq, cache); junta_atomos(a, n->dir, cache); return; }
    unsigned long h = sig_de(a, i, cache);
    for(int j = 0; j < a->natomo; j++)
        if(a->asig[j] == h){ n->atomo = j; return; }                 /* (1) já existe */
    if(a->natomo >= 16){ n->atomo = 0; return; }
    int j = a->natomo++;
    a->asig[j] = h; a->aop[j] = n->op; a->anega[j] = n->nega; a->av[j] = n->v;
    n->atomo = j;
}
static void contrai_arvore(struct arvore *a){
    unsigned long cache[MAXNO];
    memset(cache, 0, sizeof cache);
    a->raiz = normaliza(a, a->raiz, cache);
    memset(cache, 0, sizeof cache);
    a->natomo = 0;
    junta_atomos(a, a->raiz, cache);
}

/* Três respostas, e a do meio é a que faltava:
 *    1  há WHERE e ele analisa      → filtra
 *    0  não há WHERE                → varre tudo, e é o que o cliente pediu
 *   −1  há WHERE e ele NÃO analisa  → RECUSA
 *
 * Antes só havia 1 e 0, e o WHERE quebrado caía no 0 — isto é, um filtro que o compilador não
 * entendeu devolvia a TABELA INTEIRA. Num banco isso é o defeito mais caro que existe: o
 * cliente pede um recorte, recebe tudo, e nada avisa. */
static int le_where(const char **p, struct arvore *a){
    memset(a, 0, sizeof *a);
    if(!palavra(p, "WHERE")) return 0;              /* não há WHERE: varrer tudo é o pedido */
    a->raiz = le_expr(p, a);
    if(a->raiz < 0) return -1;                      /* há, e não analisa: recusar */
    contrai_arvore(a);                 /* contrai ANTES de emitir */
    return 1;
}

static void emit_teste(unsigned sc, int cmp_op, long k, unsigned destino, unsigned kslot){
    /* cada condição tem o SEU slot de constante — nada compartilhado entre compilar e executar */
    Word w; w.total = k; w.e = 0;
    mem_grava(kslot, w);

    emit_copia(S_ZERO, destino);
    if(cmp_op == '='){
        MOVE(sc, +1);
        MOVE(kslot, +1);
        emit1(OP_SUB);
        MOVE(S_TMP, -1);
        MOVE(S_TMP, +1);
        MOVE(S_ZERO, +1);
        emit1(OP_CMP);
        emit1(OP_JZ); emit1(2);
    } else {
        if(cmp_op == '<'){ MOVE(kslot, +1); MOVE(sc, +1); }
        else             { MOVE(sc, +1); MOVE(kslot, +1); }
        emit1(OP_SUB);
        MOVE(S_TMP, -1);
        MOVE(S_TMP, +1);
        MOVE(S_MASK, +1);
        emit1(OP_AND);
        MOVE(S_TMP, -1);
        MOVE(S_TMP, +1);
        MOVE(S_ZERO, +1);
        emit1(OP_CMP);
        emit1(OP_JNZ); emit1(2);
    }
    emit1(OP_JMP);
    unsigned pos = pc_emit; emit1(0);
    unsigned ini = pc_emit;
    emit_copia(S_UM, destino);
    unsigned char rel = (unsigned char)(pc_emit - ini);
    pwrite(fprog, &rel, 1, (off_t)pos);
}

/* MULTIPLICAÇÃO POR CONSTANTE, NAS COORDENADAS DO REI.
 *
 * A ISA não tem MUL, e a versão anterior fazia soma repetida |c| vezes — linear no VALOR, e
 * com um `if(n > CMAX) n = CMAX` que TRUNCAVA o coeficiente em silêncio: um WHERE com 20*a
 * virava 8*a e devolvia a resposta errada sem avisar. Os dois problemas caem juntos.
 *
 * A ISA já tem a multiplicação pelo rei: GOLD é cifra_an(w,1) = (total + e, total), que é o
 * deslocamento (a,b) ↦ (a+b, a) medido em coroa.c §A5. Partindo de (x, 0), aplicar GOLD k−1
 * vezes dá (F(k)·x, F(k−1)·x): multiplicar por Fibonacci é DESLOCAR, e custa k opcodes.
 *
 * E coroa.c §A3 diz o resto: todo inteiro é soma de Fibonacci NÃO CONSECUTIVOS, de um único
 * jeito. Logo
 *      n·x = Σ F(k_i)·x = Σ GOLD^(k_i − 1) (x)
 * e o custo passa de n para o número de dígitos de Zeckendorf, que é ~log_φ(n). Nada foi
 * inventado: o opcode já estava lá, e as coordenadas são as do rei. */
static void emit_mul_zeck(unsigned acc, unsigned termo, long n, int soma, unsigned tmp){
    long fib[92]; int nf = 2;
    fib[0] = 1; fib[1] = 2;                       /* F(2), F(3), … — a base de Zeckendorf */
    while(fib[nf-1] <= n/2 + 1 && nf < 90){ fib[nf] = fib[nf-1] + fib[nf-2]; nf++; }
    long r = n;
    for(int i = nf-1; i >= 0 && r > 0; i--){
        if(fib[i] > r) continue;
        r -= fib[i];
        /* tmp ← termo, com o .e limpo, e depois i deslocamentos */
        MOVE(termo, +1);
        MOVE(S_MT, +1);
        emit1(OP_AND);
        MOVE(tmp, -1);
        /* GOLD^k parte de (x,0) e dá total = F(k+1)·x. Como fib[i] = F(i+2), o número de
         * deslocamentos é i+1, e NÃO i — errar isto acerta só quando o coeficiente é 1. */
        for(int t = 0; t <= i; t++){
            MOVE(tmp, +1);
            emit1(OP_GOLD);
            MOVE(tmp, -1);
        }
        MOVE(tmp, +1);                  /* limpa o .e que o deslocamento deixou */
        MOVE(S_MT, +1);
        emit1(OP_AND);
        MOVE(tmp, -1);
        if(soma){ MOVE(acc, +1); MOVE(tmp, +1); emit1(OP_ADD); }
        else    { MOVE(tmp, +1); MOVE(acc, +1); emit1(OP_SUB); }
        MOVE(acc, -1);
    }
}

/* os átomos distintos, avaliados UMA vez por linha.
 *
 * O átomo já vem CONTRAÍDO num vetor: c0 + Σ c_i·x_i, comparado com 0. Se o vetor não tem
 * variável, nada é emitido — o valor já se sabe. Se tem, a forma linear é montada no metal:
 * a ISA não tem multiplicação, então c_i·x_i é soma repetida |c_i| vezes, e o compilador
 * conhece c_i, logo o laço é desenrolado. */
/* O METAL COMO PALAVRA — e é isto que apaga prata, bronze e os seus negros.
 *
 * A_m = T^{m−1}·A_1 com T = A_1·J, e para m ≤ 0 o espelho exato: T⁻¹ = J·A_1⁻¹. Vale para TODO
 * m inteiro, negativo, zero e positivo — logo a ISA não precisa de um opcode por metal. Prata e
 * bronze não eram peças: eram ATALHOS, e um atalho tomado por gerador faz pensar que a máquina
 * precisa dele. Ficam quatro geradores, e são simétricos:
 *
 *     GOLD   NEGRO_OURO   TROCA   ESQUILO
 *
 * m = 0 dá A_0 = J: a troca é o metal do MEIO, onde os dois lados da régua se encontram. */
static void emit_metal(long m, unsigned s){
    MOVE(s, +1); emit1(OP_GOLD); MOVE(s, -1);
    if(m >= 1) for(long k = 1; k < m; k++){                    /* T   = TROCA depois GOLD  */
        MOVE(s, +1); emit1(OP_TROCA); MOVE(s, -1);
        MOVE(s, +1); emit1(OP_GOLD);  MOVE(s, -1);
    } else for(long k = m; k <= 0; k++){                       /* T⁻¹ = NEGRO depois TROCA */
        MOVE(s, +1); emit1(OP_NEGRO_OURO); MOVE(s, -1);
        MOVE(s, +1); emit1(OP_TROCA);      MOVE(s, -1);
    }
}
/* a VOLTA, e a regra é inteira e sem tabela: a mesma palavra ao CONTRÁRIO, cada letra pela sua
 * inversa. O gato vai a negro, o negro vai a gato, e a troca fica onde está — é involução. */
static void emit_metal_inv(long m, unsigned s){
    if(m >= 1) for(long k = m-1; k >= 1; k--){
        MOVE(s, +1); emit1(OP_NEGRO_OURO); MOVE(s, -1);
        MOVE(s, +1); emit1(OP_TROCA);      MOVE(s, -1);
    } else for(long k = 0; k >= m; k--){
        MOVE(s, +1); emit1(OP_TROCA); MOVE(s, -1);
        MOVE(s, +1); emit1(OP_GOLD);  MOVE(s, -1);
    }
    MOVE(s, +1); emit1(OP_NEGRO_OURO); MOVE(s, -1);
}

/* multiplica dois slots em tempo de EXECUÇÃO: dest = X · Y.
 *
 * A ISA não tem MUL, e aqui o multiplicador não é constante conhecida (é o valor de outra
 * coluna) — logo não dá para desenrolar. Vira laço: soma X a si mesmo |Y| vezes, com o passo
 * e o incremento escolhidos pelo SINAL de Y. Custa |Y| voltas, e isso é propriedade do metal,
 * não do compilador: multiplicar sem multiplicador custa contar. */
static void emit_mul(unsigned dest, unsigned X, unsigned Y, unsigned base){
    unsigned cnt = base, passo = base+1, delta = base+2, tmp = base+3;
    emit_copia(S_ZERO, dest);
    emit_copia(Y, cnt);

    /* o sinal de cnt escolhe o par (passo, delta) */
    MOVE(cnt, +1);
    MOVE(S_MASK, +1);
    emit1(OP_AND);
    MOVE(tmp, -1);
    MOVE(tmp, +1);
    MOVE(S_ZERO, +1);
    emit1(OP_CMP);                                   /* FL_ZERO sse cnt ≥ 0 */
    emit1(OP_JZ);
    unsigned pos_pos = pc_emit; emit1(0);
    /* cnt < 0 : passo = −X, delta = +1 */
    unsigned ini_neg = pc_emit;
    MOVE(X, +1); MOVE(S_ZERO, +1); emit1(OP_SUB);
    MOVE(passo, -1);                       /* R = 0 − X */
    emit_copia(S_UM, delta);
    emit1(OP_JMP);
    unsigned pos_fim_neg = pc_emit; emit1(0);
    unsigned ini_pos = pc_emit;
    /* cnt ≥ 0 : passo = +X, delta = −1 */
    emit_copia(X, passo);
    MOVE(S_UM, +1); MOVE(S_ZERO, +1); emit1(OP_SUB);
    MOVE(delta, -1);                       /* R = 0 − 1 */
    unsigned depois = pc_emit;
    { unsigned char r = (unsigned char)(ini_pos - ini_neg);   pwrite(fprog, &r, 1, (off_t)pos_pos); }
    { unsigned char r = (unsigned char)(depois - ini_pos);    pwrite(fprog, &r, 1, (off_t)pos_fim_neg); }

    /* o laço: enquanto cnt != 0 { dest += passo ; cnt += delta } */
    unsigned topo = pc_emit;
    MOVE(cnt, +1);
    MOVE(S_ZERO, +1);
    emit1(OP_CMP);                                    /* FL_ZERO sse cnt == 0 */
    emit1(OP_JZ);
    unsigned pos_sai = pc_emit; emit1(0);
    unsigned corpo = pc_emit;
    MOVE(dest, +1); MOVE(passo, +1); emit1(OP_ADD); MOVE(dest, -1);
    MOVE(cnt, +1);  MOVE(delta, +1); emit1(OP_ADD); MOVE(cnt, -1);
    emit1(OP_JMP);
    unsigned pos_volta = pc_emit; emit1(0);
    unsigned fim = pc_emit;
    { unsigned char r = (unsigned char)(int)((int)topo - (int)pos_volta - 1);
      pwrite(fprog, &r, 1, (off_t)pos_volta); }
    { unsigned char r = (unsigned char)(fim - corpo); pwrite(fprog, &r, 1, (off_t)pos_sai); }
}

/* os átomos distintos, avaliados UMA vez por linha.
 *
 * O átomo vem contraído num TENSOR simétrico de grau ≤ 2. A forma monta-se no metal:
 *   grau 0  a constante, direto;
 *   grau 1  coeficiente CONHECIDO em compilação  → soma repetida, desenrolada;
 *   grau 2  x_i·x_j com os dois vindos da linha  → laço (emit_mul), porque não há MUL.
 * E a comparação é sempre contra ZERO — a contração já passou tudo para um lado. */
static long corpo_parm(long p){ return (long)(int8_t)(Word8)p; } /* envelope: −1 ≡ 0xFF */
static int corpo_tem_regua(long cp){ return cp == CORPO_AUREO || cp == CORPO_CRISTAL; }
static long corpo_B(long cp, long parm){ (void)cp; return corpo_parm(parm); }
static long corpo_C(long cp){ return (cp == CORPO_AUREO) ? -1 : 1; }
static long corpo_delta(long cp, long parm){
    long B = corpo_B(cp, parm), C = corpo_C(cp);
    return B*B - 4*C;
}

static void emit_transporte(long t, unsigned s){
    /* φ_t = [[1,t],[0,1]] = (TROCA GOLD)^t, e para t<0 é (NEGRO TROCA)^|t| */
    for(long k = 0; k < (t < 0 ? -t : t); k++){
        if(t > 0){
            MOVE(s, +1); emit1(OP_TROCA); MOVE(s, -1);
            MOVE(s, +1); emit1(OP_GOLD);  MOVE(s, -1);
        } else {
            MOVE(s, +1); emit1(OP_NEGRO_OURO); MOVE(s, -1);
            MOVE(s, +1); emit1(OP_TROCA);      MOVE(s, -1);
        }
    }
}

static void emit_atomos(const struct arvore *a, long linha, long ncols){
    for(int j = 0; j < a->natomo; j++){
        unsigned dest = S_COND + (unsigned)j;
        unsigned acc  = S_LIN + (unsigned)(j*12);
        /* O MAPA DO RASCUNHO, dito de uma vez — cada átomo tem acc..acc+11 e emit_mul
         * consome QUATRO a partir do base. Foi a sobreposição disto que me custou duas
         * tentativas hoje: cada ordem inventada batia noutra ordem inventada. */
        unsigned prod = acc + 1, prod2 = acc + 2, tmpm = acc + 3, base = acc + 4;

        if(ten_constante(a->av[j])) continue;         /* decidido: nada se emite */

        /* OS DOIS LADOS NA MESMA RÉGUA.
         *
         * A coluna vive como par p/q; a constante vivia como magnitude crua. Comparar as duas
         * era comparar coordenada com magnitude — e a igualdade nunca fechava, porque o .e não
         * batia. Mascarar o .e escondia a diferença em vez de a resolver.
         *
         * O certo é LEVANTAR a constante à régua da coluna. O átomo Σc·x + c₀ com x = p/q vale
         * (c·p + c₀·q)/q, e como q > 0 o sinal é o do numerador:
         *
         *     c·p + c₀·q   OP   0
         *
         * Então o termo constante entra multiplicado pelo denominador da coluna, e os dois
         * lados passam a ser numeradores sobre o mesmo q. Com q = 1 dá exatamente o que dava
         * antes — a tabela de inteiros não muda um byte. */
        int cit[NCOL]; int ncit = 0, unica = -1;
        for(int c = 0; c < NCOL; c++) cit[c] = 0;
        for(int cod = 1; cod < NMON; cod++){
            if(!a->av[j].c[cod]) continue;
            int dd[KGRAU]; mi_de(cod, dd);
            for(int t = 0; t < KGRAU; t++) if(dd[t] && dd[t]-1 < ncols && !cit[dd[t]-1]){
                cit[dd[t]-1] = 1; unica = dd[t]-1; ncit++;
            }
        }
        Word w; w.total = a->av[j].c[0]; w.e = 0;
        mem_grava(S_K + (unsigned)j, w);
        (void)ncit; (void)unica;
        emit_copia(S_ZERO, acc);          /* o constante entra pelo laço, como monômio vazio */

        /* percorre os monômios do multi-índice. Grau 0 já entrou; grau 1 tem coeficiente
         * conhecido em compilação (soma desenrolada); grau ≥ 2 precisa multiplicar colunas
         * em tempo de execução, e a ISA não tem MUL — vira cadeia de emit_mul. */
        for(int cod = 0; cod < NMON; cod++){
            long c = a->av[j].c[cod];
            if(!c) continue;
            int d[KGRAU]; mi_de(cod, d);
            if(mi_cod(d) != cod) continue;                 /* só os representantes ordenados */
            int g = mi_grau(cod), fora = 0;
            for(int t = 0; t < KGRAU; t++) if(d[t] && d[t]-1 >= ncols) fora = 1;
            if(fora) continue;
            long n = c < 0 ? -c : c;      /* nada de truncar: o coeficiente entra inteiro */

            /* A CONTRAÇÃO — o chicote inteiro.
             *
             * Cada monômio é o MESMO produto sobre as colunas citadas, escolhendo p onde o
             * monômio usa a coluna e q onde não usa. Mesma forma, mesmo comprimento, para
             * todos os termos: não há caso especial e não há ordem a escolher. O termo
             * constante é o monômio vazio — todas as colunas entram com q.
             *
             * E a fonte é MASCARADA antes de entrar no produto: o .e de uma linha é o
             * denominador, e se ele chega ao emit_mul envenena o contador do laço. Foi isso
             * que pendurou a tentativa anterior — não era o slot, era o .e. */
            unsigned termo = prod;
            /* a BASE DE REFERÊNCIA do átomo: a primeira coluna citada que tenha régua. As
             * outras da mesma classe são transportadas até ela. */
            long b_ref = 0; int b_ref_ok = 0;
            for(int cc = 0; cc < NCOL && !b_ref_ok; cc++){
                if(!cit[cc]) continue;
                Word cw = (cc < 8) ? mem_le(S_CORPO + (unsigned)cc) : (Word){0,0};
                if(corpo_tem_regua(cw.total)){ b_ref = corpo_B(cw.total, cw.e); b_ref_ok = 1; }
            }
            emit_copia(S_UM, prod);
            for(int cc = 0; cc < NCOL; cc++){
                if(!cit[cc]) continue;
                int usa = 0;
                for(int t = 0; t < KGRAU; t++) if(d[t] == cc+1) usa = 1;
                unsigned fonte = usa ? (S_LINHAS + (unsigned)(linha*ncols + cc))
                                     : (S_DEN    + (unsigned)(linha*ncols + cc));
                /* O TRANSPORTE, LIGADO. Se a coluna vive noutra base da MESMA classe, o
                 * valor tem de ser levado à base de referência antes de entrar no produto —
                 * e isso é φ_t, o cisalhamento, palavra nos geradores.
                 *
                 * A ORDEM IMPORTA: φ_t age sobre a Word inteira, (a,b) ↦ (a+t·b, b), e a
                 * máscara mata o .e. Logo transporta-se PRIMEIRO e mascara-se depois — ao
                 * contrário, φ_t receberia b = 0 e seria a identidade. */
                emit_copia(fonte, tmpm);
                {
                    Word cwc = (cc < 8) ? mem_le(S_CORPO + (unsigned)cc) : (Word){0,0};
                    if(usa && b_ref_ok && corpo_tem_regua(cwc.total)){
                        long t = (b_ref - corpo_B(cwc.total, cwc.e)) / 2;
                        if(t) emit_transporte(t, tmpm);
                    }
                }
                MOVE(tmpm, +1); MOVE(S_MT, +1); emit1(OP_AND);
                MOVE(tmpm, -1);
                emit_mul(prod2, prod, tmpm, base);
                emit_copia(prod2, prod);
            }
            (void)g;
            emit_mul_zeck(acc, termo, n, c > 0, acc + 8);
        }
        emit_teste(acc, a->aop[j], 0, dest, S_KZ + (unsigned)j);
        if(a->anega[j]){
            MOVE(dest, +1);
            MOVE(S_UM, +1);
            emit1(OP_XOR);
            MOVE(dest, -1);
        }
    }
}
/* percorre a árvore em pós-ordem; cada nó deixa 0 ou 1 no seu slot */
static void emit_no(const struct arvore *a, int i, long linha, long ncols, unsigned dest){
    const struct no *n = &a->no[i];
    if(n->tipo == NO_COND){
        if(n->decidido) emit_copia(n->valor ? S_UM : S_ZERO, dest);  /* decidido na compilação */
        else            emit_copia(S_COND + (unsigned)n->atomo, dest);
        return;
    }
    unsigned de = dest + 2, dd = dest + 34;            /* dois ramos, slots afastados */
    emit_no(a, n->esq, linha, ncols, de);
    emit_no(a, n->dir, linha, ncols, dd);
    MOVE(de, +1);
    MOVE(dd, +1);
    emit1(n->tipo == NO_AND ? OP_AND : OP_OR);         /* o AND/OR do SQL É o da ISA */
    MOVE(dest, -1);
}

static void emit_linha(long i, long ncols, const struct arvore *a, int tem_where,
                       int acao, int col_set)
{
    if(tem_where){
        emit_atomos(a, i, ncols);                   /* cada átomo distinto, uma vez só */
        emit_no(a, a->raiz, i, ncols, S_EXPR);
        emit_copia(S_EXPR, S_ACC);
    } else {
        emit_copia(S_UM, S_ACC);
    }

    MOVE(S_ACC, +1);
    MOVE(S_VIVO + (unsigned)i, +1);
    emit1(OP_AND);
    MOVE(S_ACC, -1);

    MOVE(S_ACC, +1);
    MOVE(S_ZERO, +1);
    emit1(OP_CMP);
    emit1(OP_JZ);
    unsigned pos = pc_emit; emit1(0);
    unsigned ini = pc_emit;
    /* A varredura NÃO grava mais o efeito: ela só MARCA. O bitmap de casamento passa a ser o
     * diário de intenção, e quem aplica é a fase 3, depois do compromisso. Assim a queda no
     * meio da varredura não deixa metade das linhas mudadas. */
    (void)col_set;
    emit_copia(S_UM, S_MATCH + (unsigned)i);
    MOVE(S_CONTA, +1);
    MOVE(S_UM, +1);
    emit1(OP_ADD);
    MOVE(S_CONTA, -1);
    unsigned char rel = (unsigned char)(pc_emit - ini);
    pwrite(fprog, &rel, 1, (off_t)pos);
}

/* prepara as constantes e devolve o catálogo */
static void prepara(long v){
    Word w = {0,0};
    mem_grava(S_ZERO, w);
    mem_grava(S_CONTA, w);
    w.total = 1; w.e = 0;                 mem_grava(S_UM, w);
    w.total = v; w.e = 0;                 mem_grava(S_V, w);
    w.total = 0x80; w.e = 0; mem_grava(S_MASK, w);   /* bit 7 do Word_8 — sinal no envelope */
}


/* FASE 3: aplica o que o diário mandou. Programa próprio, desenrolado, sem indireção — o
 * bitmap diz quais linhas, e o compilador já conhece cada índice.
 *
 * É IDEMPOTENTE de propósito: escreve valores absolutos, nunca incrementos. Por isso pode ser
 * repetida na abertura sem estragar nada, que é o que faz o redo funcionar. */
static long aplica_diario(long ncols, long nrows, int acao, int col_set){
    pc_emit = 0;
    for(long i = 0; i < nrows; i++){
        MOVE(S_MATCH + (unsigned)i, +1);
        MOVE(S_ZERO, +1);
        emit1(OP_CMP);
        emit1(OP_JZ);
        unsigned pos = pc_emit; emit1(0);
        unsigned ini = pc_emit;
        if(acao == ACAO_SET) emit_copia(S_V,    S_LINHAS + (unsigned)(i*ncols + col_set));
        else                 emit_copia(S_ZERO, S_VIVO  + (unsigned)i);
        unsigned char rel = (unsigned char)(pc_emit - ini);
        pwrite(fprog, &rel, 1, (off_t)pos);
    }
    emit1(OP_HALT);
    return rodar(pc_emit);
}

/* Na abertura: se o diário ficou aberto, uma queda apanhou a base entre o compromisso e o
 * fim da aplicação. Refaz-se, e só então se fecha o diário. */
static void refaz_diario(void){
    Word d = mem_le(S_DIA);
    if(d.total == 0) return;
    Word cat = mem_le(S_CAT);
    printf("-- diário aberto: refazendo %s\n", d.total == ACAO_SET+1 ? "um UPDATE" : "um DELETE");
    aplica_diario(cat.total, cat.e, (int)d.total - 1, (int)d.e);
    barreira();
    Word z = {0,0}; mem_grava(S_DIA, z);
    barreira();
}

/* a última contagem, para o modo teste poder AFIRMAR em vez de só imprimir. Sem isto o
 * sql.c não afirmava nada, e por isso estava fora da bateria — mudei-o uma dúzia de vezes
 * hoje e só o verifiquei à mão. */
static long ultima_conta = 0;


/* ---------------- A DISTÂNCIA NO WHERE: só se compara dentro da classe ----------------
 *
 * Um WHERE que cita duas colunas soma-as, subtrai-as, compara-as. Isso só faz sentido se as
 * duas viverem NO MESMO CORPO — e "o mesmo corpo" tem agora um critério exato, medido em
 * topologia.c: a distância |Δ₁−Δ₂| ser ZERO.
 *
 *   distância > 0   corpos de classes diferentes → a consulta é RECUSADA, com a distância dita
 *   distância = 0   ISOMORFOS → há UM transporte, φ_t com t = (B₂−B₁)/2, e ele é EMITIDO
 *
 * Recusar é a única resposta honesta para o primeiro caso: comparar um áureo com um cristalino
 * daria um número, e o número não significaria nada. É a mesma regra do WHERE não entendido —
 * refuse-se em vez de devolver a tabela inteira.
 *
 * Colunas fora da família quadrática (INTEIRO, RACIONAL, MORFICO) não entram na guarda: elas
 * não têm régua desta forma, e o resto do compilador já as trata. */
/* devolve 1 se o WHERE pode ser compilado; 0 se é RECUSADO. Se houver transporte, di-lo. */
/* e a ORDEM: num corpo ELÍPTICO (Δ<0) a pergunta "a < b" é MAL POSTA, não difícil. Se houvesse
 * ordem compatível, ω² = −1 daria −1 ≥ 0 com 1 > 0, logo 0 > 0 (ordem.c §O3). Então uma
 * desigualdade sobre coluna elíptica é RECUSADA, e diz-se porquê — comparar por norma é outra
 * pergunta, e quem a quiser tem de a escrever. */
static int checa_corpos(unsigned citadas, long ncols){
    int primeira = -1; long Dref = 0, Bref = 0;
    for(long j = 0; j < ncols && j < 8; j++){
        if(!(citadas & (1u << j))) continue;
        Word c = mem_le(S_CORPO + (unsigned)j);
        if(!corpo_tem_regua(c.total)) continue;
        long D = corpo_delta(c.total, c.e), B = corpo_B(c.total, c.e);
        if(primeira < 0){ primeira = (int)j; Dref = D; Bref = B; continue; }
        if(D != Dref){
            long d = D - Dref; if(d < 0) d = -d;
            printf("erro: as colunas %c e %c estão em corpos de classes DIFERENTES "
                   "(Δ = %ld e Δ = %ld, distância %ld).\n",
                   (char)('a'+primeira), (char)('a'+j), Dref, D, d);
            printf("      a consulta é RECUSADA: comparar através delas daria um número sem "
                   "significado.\n");
            return 0;
        }
        if(B != Bref){
            long t = (Bref - B) / 2;
            printf("nota: %c e %c são ISOMORFOS (Δ = %ld, distância 0) em bases diferentes "
                   "— φ_t com t = %ld,\n", (char)('a'+primeira), (char)('a'+j), D, t);
            printf("      EMITIDO no caminho do átomo como %s.\n",
                   t > 0 ? "(TROCA GOLD)^t" : "(NEGRO TROCA)^|t|");
        }
    }
    return 1;
}

static int varre(const char *resto, int acao){
    const char *p = resto;
    char nome[64], alvo[64];
    long v = 0;
    int col_set = 0, tem_where;
    struct arvore cl;

    if(acao == ACAO_MARCA){
        char cols[64];
        if(!ident(&p, cols, sizeof cols)) return 0;
        if(!palavra(&p, "FROM")) return 0;
        if(!ident(&p, nome, sizeof nome)) return 0;
    } else if(acao == ACAO_SET){
        if(!ident(&p, nome, sizeof nome)) return 0;
        if(!palavra(&p, "SET")) return 0;
    } else {
        if(!palavra(&p, "FROM")) return 0;
        if(!ident(&p, nome, sizeof nome)) return 0;
    }
    /* ABRIR A TABELA VEM PRIMEIRO — antes de resolver coluna nenhuma.
     *
     * O nome foi lido nos TRÊS ramos e não era usado em nenhum; agora abre-se o
     * .mem da tabela e confere-se que o catálogo lá dentro é mesmo o dela. E isto
     * tem de acontecer ANTES do `SET x` e do `WHERE`, porque uma coluna só existe
     * DENTRO de uma tabela: com a ordem trocada, o `UPDATE cliente SET saldo`
     * logo a seguir a um `SELECT * FROM conta` procurava a coluna «saldo» na
     * `conta` e devolvia «column "saldo" does not exist». Foi a ligação ponta a
     * ponta pelo FEBE que o mostrou — nenhuma asserção o via, porque nos testes a
     * tabela do UPDATE era sempre a que já estava aberta. */
    if(!usa_tabela(nome, 0)) return cat_nome_recusa(nome);
    if(!cat_nome_bate(nome)) return cat_nome_recusa(nome);
    if(acao == ACAO_SET){
        if(!ident(&p, alvo, sizeof alvo)) return 0;
        pula(&p); if(*p != '=') return 0; p++;
        if(!numero(&p, &v)) return 0;
        col_set = col_indice(alvo);
        if(col_set < 0){
            printf("erro: a coluna «%s» não existe na tabela «%s» — o UPDATE é RECUSADO.\n",
                   alvo, nome);
            if(sql_cap){ sql_cap->ok = 0;
                snprintf(sql_cap->err, sizeof sql_cap->err,
                         "column \"%s\" does not exist", alvo); }
            return 0;
        }
    }
    citadas_where = 0;
    tem_where = le_where(&p, &cl);
    if(tem_where < 0){
        printf("erro: o WHERE não foi entendido — a consulta é RECUSADA, e nada é devolvido\n");
        return 0;
    }

    Word cat = mem_le(S_CAT);
    long ncols = cat.total, nrows = cat.e;
    /* A DISTÂNCIA LIGADA AO WHERE: só se compara dentro da classe de isomorfismo. */
    if(tem_where > 0 && !checa_corpos(citadas_where, ncols)) return 0;
    /* E A LARGURA: o avaliador é de oito bits, e uma coluna que guarde acima de
     * 255 não cabe nele. Recusa-se, conta-se, e diz-se porquê — não se compara
     * meio valor. O SELECT sem WHERE continua a devolver o número inteiro. */
    if(tem_where > 0){
        for(long j = 0; j < ncols && j < NCOL; j++){
            if(!(citadas_where & (1u << j))) continue;
            if(!col_larga(j, ncols, nrows)) continue;
            char cn[S_COLNOME_W * 2 + 2];
            col_nome_le((int)j, cn, (int)sizeof cn);
            if(!cn[0]) snprintf(cn, sizeof cn, "%c", 'a' + (int)j);
            printf("erro: a coluna «%s» guarda valores acima de 255, e o avaliador do"
                   " WHERE é de oito bits.\n A comparação é RECUSADA — o valor está"
                   " guardado inteiro e o SELECT sem WHERE devolve-o; o que não se faz"
                   " é comparar metade dele.\n", cn);
            if(sql_cap){
                sql_cap->ok = 0;
                snprintf(sql_cap->err, sizeof sql_cap->err,
                         "column \"%s\" holds values above the 8-bit evaluator", cn);
            }
            cmp_recusadas++;
            return 0;
        }
    }
    if(nrows <= 0){
        /* A TABELA VAZIA TAMBÉM TEM COLUNAS, e uma consulta que não devolve linhas
         * tem de devolver a DESCRIÇÃO delas e o `CommandComplete`. Esta saída
         * antecipada devolvia `ncols = 0` e tag vazia: pela porta FEBE o driver
         * ficava sem RowDescription e sem o «SELECT 0» que fecha o ciclo, e não
         * distinguia «a tabela está vazia» de «a resposta perdeu-se».
         *
         * O caso com WHERE já o fazia bem — a divergência era só aqui, quando a
         * tabela ainda não tem uma única linha. */
        printf("(vazio)\n");
        if(sql_cap && acao == ACAO_MARCA){
            sql_cap->ncols = (int)(ncols > SQL_OUT_MAX_COLS ? SQL_OUT_MAX_COLS : ncols);
            for(int j = 0; j < sql_cap->ncols; j++){
                char cn[S_COLNOME_W * 2 + 2];
                col_nome_le(j, cn, (int)sizeof cn);
                if(cn[0]) snprintf(sql_cap->col[j], sizeof sql_cap->col[j], "%s", cn);
                else      snprintf(sql_cap->col[j], sizeof sql_cap->col[j], "%c", 'a' + j);
            }
            sql_cap->nrows = 0;
            snprintf(sql_cap->tag, sizeof sql_cap->tag, "SELECT 0");
        }else if(sql_cap){
            snprintf(sql_cap->tag, sizeof sql_cap->tag, "%s 0",
                     acao == ACAO_SET ? "UPDATE" : "DELETE");
        }
        return 1;
    }

    /* A guarda que recusava consulta sobre coluna racional saiu daqui: a contração está
     * emitida em emit_atomos e a comparação é sobre o NUMERADOR do denominador comum. O que
     * era recusa honesta virou conta feita — inclusive com mais de uma coluna racional. */
    prepara(v);
    Word z = {0,0};
    for(long i = 0; i < nrows; i++) mem_grava(S_MATCH + (unsigned)i, z);

    /* UM molde só, o da linha 0 — e depois a PA anda com ele por todas as linhas. */
    pc_emit = 0; nrel = 0; rel_ncols = ncols;
    emit_linha(0, ncols, &cl, tem_where, acao, col_set);
    emit1(OP_HALT);
    rel_ncols = 0;
    long passos = 0;
    for(long i = 0; i < nrows; i++){ rel_anda(i); passos += rodar(pc_emit); }

    unsigned long soma = 1469598103934665603UL;
    for(unsigned q = 0; q < pc_emit; q++){ soma ^= prog_le(q); soma *= 1099511628211UL; }

    long achou = mem_le(S_CONTA).total;
    ultima_conta = achou;
    if(acao != ACAO_MARCA){
        /* o bitmap (o diário) já está no disco; agora o COMPROMISSO, e só depois o efeito. */
        barreira();
        trava_se_pedido(2);                       /* queda ANTES do compromisso: nada mudou */
        Word d; d.total = acao + 1; d.e = col_set;
        mem_grava(S_DIA, d);
        barreira();                               /* ← o ponto de compromisso */
        trava_se_pedido(3);                       /* queda DEPOIS: a abertura refaz */
        passos += aplica_diario(ncols, nrows, acao, col_set);
        barreira();
        trava_se_pedido(4);                       /* queda aqui: refaz de novo, e é idempotente */
        Word z2 = {0,0}; mem_grava(S_DIA, z2);
        barreira();
    }
    const char *nome_acao = acao == ACAO_MARCA ? "lida(s)" : (acao == ACAO_SET ? "atualizada(s)" : "apagada(s)");
    printf("-- %u bytes de ISA [%04lx], %d átomo(s), %ld passos, %ld linha(s) %s\n",
           pc_emit, soma & 0xFFFF, tem_where ? cl.natomo : 0, passos, achou, nome_acao);
    if(acao != ACAO_MARCA){
        if(sql_cap){
            if(acao == ACAO_SET)
                snprintf(sql_cap->tag, sizeof sql_cap->tag, "UPDATE %ld", achou);
            else
                snprintf(sql_cap->tag, sizeof sql_cap->tag, "DELETE %ld", achou);
            sql_cap->ncols = 0; sql_cap->nrows = 0;
        }
        return 1;
    }
    if(sql_cap){
        sql_cap->ncols = (int)(ncols > SQL_OUT_MAX_COLS ? SQL_OUT_MAX_COLS : ncols);
        for(int j = 0; j < sql_cap->ncols; j++)
            { char cn[S_COLNOME_W * 2 + 2];
              col_nome_le((int)j, cn, (int)sizeof cn);
              if(cn[0]) snprintf(sql_cap->col[j], sizeof sql_cap->col[j], "%s", cn);
              else      snprintf(sql_cap->col[j], sizeof sql_cap->col[j], "%c", 'a' + (int)j); }
        snprintf(sql_cap->tag, sizeof sql_cap->tag, "SELECT %ld", achou);
        sql_cap->nrows = 0;
    }
    for(long i = 0; i < nrows; i++){
        if(mem_le(S_MATCH + (unsigned)i).total == 0) continue;
        printf("   ");
        int row_i = sql_cap ? sql_cap->nrows : -1;
        if(sql_cap && row_i >= 0 && row_i < SQL_OUT_MAX_ROWS) sql_cap->nrows++;
        for(long j = 0; j < ncols; j++){
            Word c = mem_le(S_LINHAS + (unsigned)(i*ncols + j));
            long cp = (j < 8) ? mem_le(S_CORPO + (unsigned)j).total : CORPO_INTEIRO;
            char cel[SQL_OUT_CELL];
            cel[0] = 0;
            if(cp == CORPO_MORFICO){
                long n = mem_le(S_CORPO + (unsigned)j).e; if(n < 1 || n > 62) n = 6;
                unsigned long msk = (unsigned long)c.total;
                size_t o = 0;
                cel[o++] = '{';
                int primeiro = 1;
                for(long t = 0; t < n && o + 8 < sizeof cel; t++) if(msk & (1UL << t)){
                    o += (size_t)snprintf(cel + o, sizeof cel - o, "%s%ld", primeiro ? "" : ",", t);
                    primeiro = 0;
                }
                if(o < sizeof cel - 1){ cel[o++] = '}'; cel[o] = 0; }
            /* ── O LEITOR TEM DE LER COM O SINAL QUE O CORPO DECLARA ──────────────
             * A célula é um octeto e a Word8 é sem sinal, mas o que ele SIGNIFICA
             * vem do corpo da coluna: num RACIONAL o numerador é assinado (−1/3
             * existe), no ÁUREO e no CRISTAL o coeficiente também (a−bσ existe),
             * e num INTEIRO o envelope é o Word_8 de 0..255 que a casa declara.
             *
             * Estava a ler tudo SEM SINAL: `INSERT INTO k VALUES (-2/6,2)` guardava
             * −1/3 correctamente no slot — o medidor confirma-o com um `(int8_t)` à
             * mão — e o SELECT mostrava **85**, que é ra_classe(255,3). O valor no
             * disco estava certo; era o texto que saía errado, e as duas leituras da
             * MESMA célula não concordavam. */
            } else if(cp == CORPO_AUREO){
                long t = (long)(int8_t)c.total, e = (long)(int8_t)c.e;
                if(e) snprintf(cel, sizeof cel, "%ld%+ldσ", t, e);
                else  snprintf(cel, sizeof cel, "%ld", t);
            } else if(cp == CORPO_CRISTAL){
                long t = (long)(int8_t)c.total, e = (long)(int8_t)c.e;
                if(e) snprintf(cel, sizeof cel, "%ld%+ldω", t, e);
                else  snprintf(cel, sizeof cel, "%ld", t);
            } else if(cp == CORPO_RACIONAL || c.e > 1){
                Par cls = ra_classe((Par){ (long)(int8_t)c.total, c.e ? (long)c.e : 1 });
                if(cls.b > 1) snprintf(cel, sizeof cel, "%ld/%ld", cls.a, cls.b);
                else          snprintf(cel, sizeof cel, "%ld", cls.a);
            } else {
                /* o valor de uma coluna inteira é o par (baixo, alto) lido como UM
                 * número — a dobra do §26 aplicada à fronteira de leitura */
                unsigned long alto = mem_le(S_ALTO + (unsigned)(i*ncols + j)).total;
                snprintf(cel, sizeof cel, "%lu", (unsigned long)c.total | (alto << 8));
            }
            printf("%s", cel);
            if(j+1 < ncols) printf(" | ");
            if(sql_cap && row_i >= 0 && row_i < SQL_OUT_MAX_ROWS && j < SQL_OUT_MAX_COLS)
                snprintf(sql_cap->cell[row_i][j], SQL_OUT_CELL, "%s", cel);
        }
        printf("\n");
    }
    return 1;
}

/* ---------------- A TOPOLOGIA NO SQL: distância entre os corpos das colunas ----------------
 *
 * A régua de cada corpo é um ponto (B,C), e a assinatura Δ = B²−4C é a coordenada que sobrevive
 * à base (topologia.c). A distância entre dois corpos é |Δ₁−Δ₂|, e ZERO quer dizer ISOMORFOS —
 * não "a mesma régua". Quando é zero, há UM transporte, φ_t com t = (B₂−B₁)/2, que é o
 * cisalhamento — e o cisalhamento é palavra na ISA: (TROCA GOLD)^t.
 *
 * Então a query pode perguntar a distância entre as colunas, e quando ela é zero pode dizer
 * COMO ir de uma à outra, em bytecode.
 *
 * A régua de cada corpo declarado:
 *   AUREO(m)       σ² = mσ + 1   →  B = m, C = −1  →  Δ = m² + 4
 *   CRISTALINO(t)  ω² = tω − 1   →  B = t, C = +1  →  Δ = t² − 4
 *
 * E os outros — INTEIRO, RACIONAL, MORFICO — NÃO são da família quadrática binária, logo não
 * têm régua desta forma e não têm Δ. Inventar um número para eles seria pior que não responder,
 * e a coluna sai marcada com "—". */

static int distancia(void){
    long ncols = mem_le(S_CAT).total;
    if(ncols > 8) ncols = 8;
    printf("      coluna  corpo             régua (B,C)   Δ = B²−4C   classe\n");
    for(long j = 0; j < ncols; j++){
        Word c = mem_le(S_CORPO + (unsigned)j);
        if(!corpo_tem_regua(c.total)){
            printf("      %-7ld %-17s %-13s %-11s %s\n", j,
                   c.total == CORPO_RACIONAL ? "RACIONAL" :
                   (c.total == CORPO_MORFICO ? "MORFICO" : "INTEIRO"),
                   "—", "—", "fora da família quadrática");
            continue;
        }
        long B = corpo_B(c.total, c.e), C = corpo_C(c.total), D = B*B - 4*C;
        char nm[32];
        snprintf(nm, sizeof nm, "%s(%d)", c.total == CORPO_AUREO ? "AUREO" : "CRISTALINO", c.e);
        char rg[24]; snprintf(rg, sizeof rg, "(%ld,%ld)", B, C);
        printf("      %-7ld %-17s %-13s %-11ld %s\n", j, nm, rg, D,
               D < 0 ? "elíptica" : (D == 0 ? "parabólica" : "hiperbólica"));
    }
    printf("\n      a distância d(i,j) = |Δᵢ − Δⱼ|, e ZERO quer dizer ISOMORFOS:\n\n");
    printf("      ");
    for(long j = 0; j < ncols; j++) printf("%8ld", j);
    printf("\n");
    for(long i = 0; i < ncols; i++){
        Word ci = mem_le(S_CORPO + (unsigned)i);
        printf("      %ld:", i);
        for(long j = 0; j < ncols; j++){
            Word cj = mem_le(S_CORPO + (unsigned)j);
            if(!corpo_tem_regua(ci.total) || !corpo_tem_regua(cj.total)){ printf("%8s", "—"); continue; }
            long d = corpo_delta(ci.total,ci.e) - corpo_delta(cj.total,cj.e);
            printf("%8ld", d < 0 ? -d : d);
        }
        printf("\n");
    }
    /* e onde a distância é zero, DIZ COMO ir: o transporte, e a palavra que o executa */
    int achou = 0;
    for(long i = 0; i < ncols; i++) for(long j = i+1; j < ncols; j++){
        Word ci = mem_le(S_CORPO + (unsigned)i), cj = mem_le(S_CORPO + (unsigned)j);
        if(!corpo_tem_regua(ci.total) || !corpo_tem_regua(cj.total)) continue;
        if(corpo_delta(ci.total,ci.e) != corpo_delta(cj.total,cj.e)) continue;
        if(!achou){ printf("\n      ISOMORFOS, e o transporte de cada par:\n\n");
                    printf("      de → para   t = (B₂−B₁)/2   φ_t              palavra na ISA\n"); }
        achou = 1;
        long B1 = corpo_B(ci.total,ci.e), B2 = corpo_B(cj.total,cj.e);
        long t = (B2 - B1) / 2;
        char de[16]; snprintf(de, sizeof de, "%ld → %ld", i, j);
        char mt[24]; snprintf(mt, sizeof mt, "[[1,%ld],[0,1]]", t);
        printf("      %-11s %-16ld %-16s %s\n", de, t, mt,
               t == 0 ? "(vazia — é a mesma)" :
               (t > 0 ? "(TROCA GOLD)^t" : "(NEGRO TROCA)^|t|"));
    }
    if(!achou) printf("\n      (nenhum par de colunas é isomorfo nesta tabela.)\n");
    return 1;
}

/* ---------------- A DISTÂNCIA ENTRE TEXTOS ----------------
 *
 * Um texto é uma sequência de símbolos; sequência de inteiros é uma CIFRA; e a cifra é um ponto
 * do corpo métrico. Logo dois textos são dois pontos, e a distância é a do métrico.
 *
 * O que a torna boa medida de texto não é escolha: dois números são próximos SSE as cifras
 * concordam num prefixo longo. A distância lê onde os textos DIVERGEM. (texto.c) */
static int tx_termos(const char *s, long *a, int max){
    int n = 0;
    for(const char *p = s; *p && n < max; p++) a[n++] = (unsigned char)*p - 31;
    return n;
}
static int distancia_texto(const char *p){
    char A[128], B[128];
    pula(&p);
    if(*p != '\'' && *p != '"') return 0;
    char asp = *p++; int k = 0;
    while(*p && *p != asp && k < 127) A[k++] = *p++;
    A[k] = 0; if(*p == asp) p++;
    pula(&p);
    if(*p != '\'' && *p != '"') return 0;
    asp = *p++; k = 0;
    while(*p && *p != asp && k < 127) B[k++] = *p++;
    B[k] = 0; if(*p == asp) p++;
    /* SEM SEGURAR AS DUAS CIFRAS. Eu guardava as duas em arrays de 128 para depois as comparar
     * — montagem fora, e com tecto. Comparar e ANDAR: le-se um simbolo de cada e para-se no
     * primeiro que diverge. Nao ha nada a guardar, e nao ha tamanho maximo. */
    int na = 0, nb = 0;
    { const char *q = A; while(*q++) na++; q = B; while(*q++) nb++; }
    int pre = 0;
    while(pre < na && pre < nb && A[pre] == B[pre]) pre++;
    printf("      texto A   \"%s\"\n", A);
    printf("      texto B   \"%s\"\n", B);
    printf("      cifra A   [");
    for(int i=0;i<na && i<6;i++) printf("%s%ld", i?";":"", (long)(unsigned char)A[i] - 31);
    printf("%s]\n", na>6?";…":"");
    printf("      cifra B   [");
    for(int i=0;i<nb && i<6;i++) printf("%s%ld", i?";":"", (long)(unsigned char)B[i] - 31);
    printf("%s]\n", nb>6?";…":"");
    printf("      prefixo comum: %d símbolo(s)\n", pre);
    if(na == nb && pre == na) printf("      DISTÂNCIA 0 — são o mesmo texto\n");
    else {
        long den = 1; for(int i=0;i<pre && i<40;i++) den *= 2;
        printf("      DISTÂNCIA 1/%ld\n", den);
    }
    return 1;
}

/* ---------------- A TABELA DE TEXTOS, E A BUSCA ----------------
 *
 * Os textos vivem no ficheiro de memória, não em RAM: cada um ocupa 8 slots (128 bytes) a partir
 * de S_TEXTO, e o cabeçalho conta quantos há. Ler e escrever é pread/pwrite, como tudo o resto.
 *
 * A BUSCA é a distância da cifra aplicada a cada linha: o prefixo comum decide, e o menor
 * 1/2^prefixo ganha. Uma varredura, sem índice — e o custo é o que é: linear nas linhas. */
/* ---------------- TODA ENTRADA ENTRA CIFRADA ----------------
 *
 * REGRA: o que se guarda de uma entrada nao sao os seus bytes — e a sua CIFRA. Um texto cifra-se
 * simbolo a simbolo; um racional cifra-se por Euclides. Guardados na mesma representacao,
 * comparam-se: SEM ISTO NAO HA COMO COMPARAR GATO COM CACHORRO.
 *
 * E O INDICE E A PROPRIA POSICAO. Nenhuma coordenada inventada: nem tamanho de tabela, nem
 * escala, nem hash. O corpo aureo SAO os reais, e ele cifra tudo — a cifra do rei e o unico
 * sistema de coordenadas, o mesmo para um numero, para uma regua e para um texto. O lugar de uma
 * entrada e a sua propria cifra: cada termo e um NIVEL, e a entrada mora no fim do seu caminho.
 * Exato e unico, sem truncamento, sem colisao, sem sondagem.
 *
 * A REGUA E INFINITA; o objeto e que acaba. O caminho morre onde a entrada morre — onde ela
 * quiser — e nao onde um tecto meu mandasse.
 *
 * Registo: [n termos][termo_1..termo_n][n bytes do rotulo][bytes]. O rotulo e so para mostrar ao
 * cliente; quem indexa, quem mede e quem compara e sempre a cifra.
 * Um no ocupa a largura do alfabeto; o filho pelo termo d mora no slot d, e o slot 0 guarda a
 * entrada que termina ali. O ficheiro e esparso: so os nos tocados custam disco. */
#define S_TEXTO   (S_LINHAS + 40000)
#define S_TXCAB   (S_TEXTO - 1)
#define S_TXLIVRE (S_TEXTO - 2)
#define S_NO      (S_TEXTO + 200000)
#define S_NOCAB   (S_NO - 1)
#define LARG      256u
#define MAXT      4096
static long n_leituras = 0;      /* o contador honesto: quantos nos o caminho tocou */
static long txt_n(void){ return mem_le(S_TXCAB).total; }
static unsigned no_filho(unsigned no, long d){
    n_leituras++;
    return (unsigned)mem_le(S_NO + no*LARG + (unsigned)d).total;
}
static unsigned no_novo(void){
    long n = mem_le(S_NOCAB).total; if(n < 1) n = 1;      /* 0 e a raiz */
    Word c = { n + 1, 0 }; mem_grava(S_NOCAB, c);
    return (unsigned)n;
}
/* UM TERMO NAO TEM TECTO. O no tem 256 slots, mas o termo pode ser qualquer inteiro — grande,
 * zero ou negativo. O slot 0 e o marcador de fim; o slot 254 diz "o termo e negativo, segue o
 * modulo"; o slot 255 diz "tira 253 e continua". Assim um termo qualquer desce por um caminho
 * proprio e unico, e a regua nao precisa de saber ao que vai servir. */
static void termo_passos(long t, long *passo, int *np, int max){
    int n = 0;
    if(t < 0){ if(n < max) passo[n++] = 254; t = -t; }
    while(t > 253 && n < max - 1){ passo[n++] = 255; t -= 253; }
    if(n < max) passo[n++] = t + 1;
    *np = n;
}
/* Desce um termo inteiro; se abrir != 0, abre os nos que faltarem. Devolve 0 se o caminho morre. */
static unsigned desce_termo(unsigned no, long t, int abrir){
    long passo[64]; int np;
    termo_passos(t, passo, &np, 64);
    for(int k = 0; k < np; k++){
        unsigned f = no_filho(no, passo[k]);
        if(!f){
            if(!abrir) return 0;
            f = no_novo();
            Word w = { (long)f, 0 };
            mem_grava(S_NO + no*LARG + (unsigned)passo[k], w);
        }
        no = f;
    }
    return no ? no : (unsigned)-1;
}
/* SO A ASSINATURA. O banco guardava a cifra E o rotulo — e para texto isso e DUPLICACAO EXATA:
 * a cifra de um texto E o texto, simbolo a simbolo, bijetivo. Guardar o rotulo era guardar a mesma
 * informacao duas vezes, uma nas coordenadas do sistema e outra na roupa.
 *
 * Agora entra so a assinatura. A roupa recupera-se quando alguem a quiser ver — e e por isso que
 * o comprimento foi para o fim da cifra: e o DUAL, e e ele que diz onde o lado proprio acaba. */
static unsigned reg_grava(const long *a, size_t n){
    unsigned base = (unsigned)mem_le(S_TXLIVRE).total;
    if(base < S_TEXTO) base = S_TEXTO;
    Word w; memset(&w, 0, sizeof w);
    w.total = (long)n; mem_grava(base, w);
    for(size_t k = 0; k < n; k++){ w.total = a[k]; mem_grava(base + 1 + (unsigned)k, w); }
    memset(&w, 0, sizeof w);
    w.total = (long)(base + 1 + n); mem_grava(S_TXLIVRE, w);
    return base;
}
static size_t reg_n(unsigned base){ return (size_t)mem_le(base).total; }
static long   reg_termo(unsigned base, size_t k){ return mem_le(base + 1 + (unsigned)k).total; }
static unsigned reg_prox(unsigned base){ return base + 1 + (unsigned)reg_n(base); }
/* A ROUPA, RECUPERADA. Nao esta guardada: deriva-se da assinatura. O ultimo par de termos da o
 * corte (np, nd); os np termos a seguir a seta de Wick sao o lado proprio. Se todos couberem no
 * alfabeto, mostra-se como texto; se nao, mostra-se a cifra, que e o que ele e. */
static void reg_mostra(unsigned base, char *out, size_t lim){
    size_t n = reg_n(base);
    out[0] = 0;
    if(n < 3){ snprintf(out, lim, "(vazio)"); return; }
    long np = reg_termo(base, n - 2);
    if(np <= 0 || (size_t)np > n - 3){                    /* sem corte utilizavel: a cifra crua */
        size_t k = 0; int c = 0;
        c += snprintf(out, lim, "[");
        for(; k < n && (size_t)c < lim - 8; k++)
            c += snprintf(out + c, lim - (size_t)c, "%s%ld", k?";":"", reg_termo(base, k));
        snprintf(out + c, lim - (size_t)c, "]");
        return;
    }
    int texto = 1;
    for(long k = 0; k < np; k++){
        long t = reg_termo(base, 1 + (size_t)k);
        if(t < 1 || t > 224){ texto = 0; break; }
    }
    if(texto){
        size_t m = (size_t)np < lim - 1 ? (size_t)np : lim - 1;
        for(size_t k = 0; k < m; k++) out[k] = (char)(reg_termo(base, 1 + k) + 31);
        out[m] = 0;
    } else {
        int c = snprintf(out, lim, "[");
        for(long k = 0; k < np && (size_t)c < lim - 8; k++)
            c += snprintf(out + c, lim - (size_t)c, "%s%ld", k?";":"", reg_termo(base, 1 + (size_t)k));
        snprintf(out + c, lim - (size_t)c, "]");
    }
}
static void cif_poe(const long *a, size_t n){
    unsigned no = 0;
    for(size_t k = 0; k < n; k++) no = desce_termo(no, a[k], 1);
    if(mem_le(S_NO + no*LARG).total) return;              /* ja la esta, no seu lugar */
    unsigned base = reg_grava(a, n);
    Word wb = { (long)base, 0 }; mem_grava(S_NO + no*LARG, wb);
    Word wi = { txt_n() + 1, 0 }; mem_grava(S_TXCAB, wi);
    barreira();
}
static long acha_cifra(const long *a, size_t n, size_t *desceu_out){
    unsigned no = 0; size_t desceu = 0;
    for(size_t j = 0; j < n; j++){
        unsigned f = desce_termo(no, a[j], 0);
        if(!f) break;
        no = f; desceu++;
    }
    *desceu_out = desceu;
    return (desceu == n) ? mem_le(S_NO + no*LARG).total : 0;
}
static void mostra_cifra(const long *a, size_t n){
    printf("[");
    for(size_t k = 0; k < n; k++) printf("%s%ld", k?";":"", a[k]);
    printf("]");
}
/* A UNICA PORTA: 'texto' cifra-se simbolo a simbolo, p/q cifra-se por Euclides. */
static int cifra_entrada(const char **p, long *a, size_t max, size_t *n, char *rot, size_t lr){
    pula(p);
    if(**p == '\'' || **p == '"'){
        char asp = *(*p)++;
        const char *ini = *p;
        while(**p && **p != asp) (*p)++;
        size_t len = (size_t)(*p - ini);
        if(**p == asp) (*p)++;
        /* UMA PORTA SO. O texto passa pelo MESMO cifra_geral dos corpos: os simbolos sao o
         * periodo — o texto e o seu proprio gerador — com razao 1 (um simbolo por nivel) e
         * sinal +1 (o texto nao fecha). Agora que os comprimentos foram para tras, o prefixo
         * volta a ser o conteudo, e 'ourives' e prefixo de 'ourivesaria' outra vez. */
        size_t ns = len < 48 ? len : 48;
        long per[48];
        for(size_t k = 0; k < ns; k++) per[k] = (long)(unsigned char)ini[k] - 31;
        *n = cifra_geral(per, (int)ns, 1, 1, 1, a, max);
        snprintf(rot, lr, "'%.*s'", (int)ns, ini);
        return *n > 0;
    }
    {
        long pp = 0, qq = 1; int sinal = 1, viu = 0;
        if(**p == '-'){ sinal = -1; (*p)++; }
        while(**p >= '0' && **p <= '9'){ pp = pp*10 + (*(*p)++ - '0'); viu = 1; }
        if(!viu) return 0;
        if(**p == '/'){ (*p)++; qq = 0; while(**p >= '0' && **p <= '9') qq = qq*10 + (*(*p)++ - '0'); }
        if(qq == 0) return 0;
        pp *= sinal;
        /* o racional pela mesma porta: os termos de Euclides sao o periodo */
        long per[48]; size_t np = 0;
        long x = pp, y = qq;
        while(y && np < 48){ long t = x / y; per[np++] = t; long r = x - t*y; x = y; y = r; }
        *n = cifra_geral(per, (int)np, 1, 1, 1, a, max);
        snprintf(rot, lr, "%ld/%ld", pp, qq);
        return *n > 0;
    }
}
/* OS 28 CORPOS: A REGUA (B,C) DE CADA UM, E A CIFRA QUE SAI DELA.
 *
 * A regua nao se escolhe — LE-SE DO OPERADOR: B = tr(Pi), C = det(Pi), e dai Delta = B^2-4C =
 * tr^2-4det. Depois a cifra sai da regua e so dela: sigma = (B+sqrt|Delta|)/2, expandida em
 * fracao continua por PQa, EM INTEIROS, sem float nenhum. Delta<0 entra pelo DUAL — a quadratura
 * que a estrutura pede — e por isso o |Delta|: uma so formula para os tres regimes.
 *
 * O que fica dito: para as familias parametricas (o gato A_m, a dilatacao por lambda) o catalogo
 * nomeia UM operador, e e o dele que se toma. Onde o parametro e livre, o membro minimo que ja
 * nao esteja tomado por outro corpo — e isso vai anotado corpo a corpo, para se poder contestar. */
/* per/np: o periodo, quando o corpo o traz proprio (so o hipercorpo); rd: a razao do lado dual,
 * que e a mesma do proprio salvo quando a deformacao emparelha DUAS razoes (o venom). */
static const long GER[16] = {1,2,4,3,7,8,6,5,13,14,16,15,11,12,10,9};
static const struct { const char *nome; long B, C; const long *per; int np; long rd;
                      const char *porque; } CORPO28[] = {
 { "racional",        2,  1, 0, 0, 2, "a classe reduz: T=[[1,1],[0,1]], tr 2 det 1" },
 { "aureo",           1, -1, 0, 0, 1, "o gato A_1, tr 1 det -1 — O REI" },
 { "deflexivo",       2, -1, 0, 0, 2, "o gato A_2, tr 2 det -1 (m=1 e o aureo)" },
 { "cristalino",      0,  1, 0, 0, 0, "o esquilo S, tr 0 det 1 — Gauss" },
 { "celeste",         0,  1, 0, 0, 0, "r^2+C^2=1 — a redonda" },
 { "optico",          0,  1, 0, 0, 0, "C^2+S^2=1 — a redonda" },
 { "criativo",        0, -1, 0, 0, 0, "NOT = involucao J, tr 0 det -1" },
 { "tecnico",         0, -1, 0, 0, 0, "a refutacao — involucao" },
 { "sensitivo",       0, -1, 0, 0, 0, "a conjugacao p-adica — involucao" },
 /* O LOGICO: A CIFRA DELE E A INDUCAO. Eu tinha-o posto em (0,-1), que e a contraposicao —
  * mas essa e o nu dele, o DUAL, e nao a deformacao. A deformacao da inducao e base + passo, e o
  * passo e sempre o MESMO, sem fim: razao 1, e o passo carrega o anterior (sinal -1). Isso e
  * A_1, sigma = 1 + 1/sigma, a cifra [1;1,1,1,...] — O REI.
  *
  * A inducao E a cifra do rei, e nao por analogia: as duas sao a mesma recursao a carregar-se. */
 { "logico",          1, -1, 0, 0, 1, "a INDUCAO: base + passo, e o passo e sempre o mesmo" },
 /* OS FORMATOS. Um formato e um corpo: a razao e quantos simbolos por nivel, o sinal e se a
  * marca FECHA. Entram pela mesma porta e caem onde a regua os puser. */
 { "json",            1, -1, 0, 0, 1, "o parentese abre e FECHA: as duas direcoes cancelam-se" },
 { "yaml",            2,  1, 0, 0, 2, "a indentacao so se acumula: dois espacos por nivel" },
 { "markdown",        1,  1, 0, 0, 1, "o cardinal so se acumula: um por nivel" },
 /* O TEX TEM DUAS MARCAS DE NIVEL, e sao corpos diferentes — nao e ambiguidade, e o que ele e:
  *   a seccao      \section, \subsection, \subsubsection: 3 simbolos ("sub") por nivel, e
  *                 NAO fecha  -> (3, +1)
  *   o ambiente    \begin{}...\end{}: abre e FECHA, como o parentese  -> (1, -1) = o AUREO
  * Entra pela seccao, que e a que estrutura o documento; e fica dito que pelo ambiente ele cai
  * exatamente no mesmo lugar do json, porque o mecanismo e o mesmo. */
 { "tex",             3,  1, 0, 0, 3, "a seccao: tres simbolos (sub) por nivel, e nao fecha" },
 { "csv",             1,  1, 0, 0, 1, "a virgula: um simbolo, e NAO fecha — o formato plano" },
 { "html",            1, -1, 0, 0, 1, "a etiqueta <t>...</t>: abre e FECHA, como o parentese" },
 /* AS LINGUAGENS. Mesma pergunta, mesma resposta: quantos simbolos por nivel, e a marca fecha? */
 { "c",               1, -1, 0, 0, 1, "as chaves { } abrem e FECHAM" },
 { "lisp",            1, -1, 0, 0, 1, "os parenteses — o mesmo mecanismo do json" },
 { "python",          4,  1, 0, 0, 4, "quatro espacos por nivel, e NAO fecha" },
 { "haskell",         2,  1, 0, 0, 2, "dois espacos por nivel, e nao fecha" },
 { "assembly",        1,  1, 0, 0, 1, "plano: o rotulo nao aninha" },
 { "fractal",         1,  1, 0, 0, 1, "z*zbar com o trono, tr 1 det 1 — Eisenstein" },
 { "relogio",         1,  1, 0, 0, 1, "N = cos psi no trono, ordem 6" },
 { "telescopico",     2,  1, 0, 0, 2, "a deflexao D_lambda: cisalhamento, tr 2 det 1" },
 { "conforme",        2,  1, 0, 0, 2, "o mergulho — cisalhamento" },
 { "entropico",       2,  1, 0, 0, 2, "(x) = + : os custos somam — parabolico" },
 { "espaco-temporal", 2,  1, 0, 0, 2, "o sucessor S(x)=x+1, T com t=1" },
 { "universal",       2,  1, 0, 0, 2, "a contagem — o mesmo sucessor" },
 { "morfico",         2,  1, 0, 0, 2, "dil por B_r: o RAIO soma — parabolico" },
 { "eletromagnetico", 3,  1, 0, 0, 3, "exp.Sigma.log com lambda minimo, tr 3 det 1" },
 { "motor",           3,  1, 0, 0, 3, "exp(tG) — o gerador, tr 3 det 1" },
 { "economico",       4,  1, 0, 0, 4, "juro composto (1+r)^n, tr 4 det 1" },
 { "evolutivo",       5,  1, 0, 0, 5, "o replicador p*w/<w>, tr 5 det 1" },
 { "expansivo",       6,  1, 0, 0, 6, "o flip Lambda = log, tr 6 det 1" },
 { "somatico",        7,  1, 0, 0, 7, "exp.Sigma.log — a mitose, tr 7 det 1" },
 { "geometrico",      3, -1, 0, 0, 3, "a RAZAO da progressao, tr 3 det -1" },
 { "cosmico",         4, -1, 0, 0, 4, "a(t)=e^{Ht}, tr 4 det -1" },
 { "rotor",           5, -1, 0, 0, 5, "phi = artanh, tr 5 det -1" },
 { "nervoso",         6, -1, 0, 0, 6, "a ativacao — a rede recorre, tr 6 det -1" },
 { "exterior",        7, -1, 0, 0, 7, "Volterra — a integral acumula, tr 7 det -1" },
 { "hipercorpo",     16,  1, GER, 16, 16, "a curva de Hilbert: a reta do rei deformada no tesseracto" },
 /* O PRISMATICO: o triangulo. O gerador percorre os TRES vertices por arestas, e isso e a
  * rotacao de 120 graus — traco -1, det 1. Elipticо, Δ = -3: o TRIANGULO E REDONDO, e nao foi
  * preciso deforma-lo. Mesma familia do fractal (Eisenstein), com a rotacao para o outro lado. */
 { "prismatico",      -1,  1, 0, 0, -1, "o triangulo: a rotacao de 120, ordem 3, eliptica" },
 /* O CANTOR: a subdivisao em TRES, e a marca nao fecha (tira-se o meio e nao se volta). E dele
  * que sai o que enche — nao o conjunto, que e po, mas a SOMA: C + C cobre o intervalo inteiro. */
 { "cantor",           3,  1, 0, 0, 3, "a subdivisao em tres; o po nao enche, a SOMA enche" },
 { "venom",           1, -1, 0, 0, 16, "avancar e esvaziar sao o mesmo ato — as duas leis da curva" },
};
#define N28 ((int)(sizeof CORPO28 / sizeof CORPO28[0]))
/* O codificador vem do cifra.h — um so, e encoda qualquer corpo: foi ele que deu a cifra
 * aos 31 e e ele que da a de um formato. Estava aqui dentro e saiu para nao haver dois. */
/* A DISTANCIA ENTRE OS CORPOS: o prefixo comum das cifras completas, e a distancia e 1/2^k. E a
 * mesma regua que mede textos e numeros — ela nao sabe o que esta a medir. */
static int distancia_corpos(void){
    long (*A)[128] = DISCO_FIXO2(long, 128, 72);
    disco_prende(DISCO_BASE(72),"dados/A_72.bin",(size_t)((size_t)(N28)*128),sizeof(long));
    disco_zera(A,(size_t)((size_t)(N28)*128),sizeof(long)); size_t *nA = DISCO_FIXO(size_t, 21); int *idx = DISCO_FIXO(int, 22);
    disco_prende(DISCO_BASE(21),"dados/sql_nA.bin",(size_t)N28,sizeof(size_t));
    disco_prende(DISCO_BASE(22),"dados/sql_idx.bin",(size_t)N28,sizeof(int)); int m = 0;
    for(int i = 0; i < N28; i++){
        nA[i] = cifra_geral(CORPO28[i].per, CORPO28[i].np, CORPO28[i].B, CORPO28[i].C,
                            CORPO28[i].rd, A[i], 128);
        int ja = 0;
        for(int k = 0; k < m; k++)
            if(nA[idx[k]] == nA[i]){
                int ig = 1;
                for(size_t t = 0; t < nA[i]; t++) if(A[idx[k]][t] != A[i][t]) ig = 0;
                if(ig) ja = 1;
            }
        if(!ja) idx[m++] = i;
    }
    printf("      o prefixo comum k das cifras completas; a distancia e 1/2^k\n\n      ");
    for(int b = 0; b < m; b++) printf("%3.3s", CORPO28[idx[b]].nome);
    printf("\n");
    int mau = 0, kmin = 999, kmax = -1, pi = 0, pj = 0, qi = 0, qj = 0;
    for(int a2 = 0; a2 < m; a2++){
        printf("      %-17s", CORPO28[idx[a2]].nome);
        for(int b = 0; b < m; b++){
            size_t k = 0;
            while(k < nA[idx[a2]] && k < nA[idx[b]] && A[idx[a2]][k] == A[idx[b]][k]) k++;
            printf("%3zu", k);
            if(a2 == b){ if(k != nA[idx[a2]]) mau++; continue; }
            if(k == nA[idx[a2]] && k == nA[idx[b]]) mau++;
            if((int)k < kmin){ kmin = (int)k; pi = idx[a2]; pj = idx[b]; }
            if((int)k > kmax){ kmax = (int)k; qi = idx[a2]; qj = idx[b]; }
        }
        printf("\n");
    }
    printf("\n      mais LONGE: %s e %s, prefixo %d — distancia 1/%d\n",
           CORPO28[pi].nome, CORPO28[pj].nome, kmin, 1 << kmin);
    printf("      mais PERTO: %s e %s, prefixo %d — distancia 1/%d\n",
           CORPO28[qi].nome, CORPO28[qj].nome, kmax, 1 << kmax);
    printf("      %d reguas distintas, e a matriz %s\n", m, mau ? "TEM FALHA" : "e metrica");
    return mau == 0;
}
/* O EMISSOR DO MARTELO. Duas portas, porque são duas coisas diferentes:
 *
 *   CABECALHO '<160 hex>' <alvo>   põe os 80 bytes e a bola na memória do banco
 *   MARTELO <de> <ate>             EMITE o opcode e a máquina do banco martela a faixa
 *
 * O martelo compila como tudo o resto — LOAD, LOAD, OP_MARTELO — e quem executa é o banco. */
static int cabecalho(const char *p){
    pula(&p);
    if(*p != '\'' && *p != '"') return 0;
    char asp = *p++;
    unsigned char cab[80]; memset(cab, 0, 80);
    int n = 0;
    while(*p && *p != asp && n < 160){
        int hi = -1, lo = -1;
        for(int k = 0; k < 2 && *p && *p != asp; k++){
            int c = *p++, v = (c >= '0' && c <= '9') ? c - '0'
                            : (c >= 'a' && c <= 'f') ? c - 'a' + 10
                            : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
            if(v < 0) return 0;
            if(k == 0) hi = v; else lo = v;
        }
        if(lo < 0) return 0;
        cab[n/2] = (unsigned char)(hi*16 + lo); n += 2;
    }
    if(*p == asp) p++;
    if(n != 160) return 0;                      /* 80 bytes exatos, ou não é cabeçalho */
    atomos_grava(S_CAB, cab, 80);
    /* o alvo entra CIFRADO, como o hash: 32 símbolos em hexadecimal */
    pula(&p);
    if(*p != '\'' && *p != '"') return 0;
    char asp2 = *p++;
    unsigned char alvo[32]; memset(alvo, 0, 32);
    int m = 0;
    while(*p && *p != asp2 && m < 64){
        int hi = -1, lo = -1;
        for(int k = 0; k < 2 && *p && *p != asp2; k++){
            int c = *p++, v = (c>='0'&&c<='9') ? c-'0' : (c>='a'&&c<='f') ? c-'a'+10
                            : (c>='A'&&c<='F') ? c-'A'+10 : -1;
            if(v < 0) return 0;
            if(k == 0) hi = v; else lo = v;
        }
        if(lo < 0) return 0;
        alvo[m/2] = (unsigned char)(hi*16 + lo); m += 2;
    }
    if(m != 64) return 0;
    atomos_grava(S_ALVO, alvo, 32);
    barreira();
    printf("      cabecalho de 80 bytes na memoria; o alvo cifrado em 32 simbolos\n");
    return 1;
}
static int martelo(const char *p){
    long de = 0, ate = 0;
    pula(&p); if(!numero(&p, &de)) return 0;
    pula(&p); if(!numero(&p, &ate)) return 0;
    /* faixa em átomos u32 — Word ISA é Word_8²; a matemática (σ²=σ+1) manda no metal */
    atomos_u32(S_FAIXA, (unsigned)de);
    atomos_u32(S_FAIXA + 4u, (unsigned)ate);
    pc_emit = 0;
    emit1(OP_MARTELO);
    emit1(OP_HALT);
    unsigned plen = pc_emit;
    Regs r; memset(&r, 0, sizeof r);
    long passos = 0;
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    while(passo(&r, plen)){ if(++passos > 50000000L) break; }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    unsigned nonce = r.R.total ? atomos_le_u32(S_FAIXA + 8u) : 0;
    int64_t seg_ns = (int64_t)(t1.tv_sec - t0.tv_sec) * 1000000000
                   + (int64_t)(t1.tv_nsec - t0.tv_nsec);
    long feitos = nonce ? (long)nonce - de : ate - de;
    if(seg_ns > 0){
        int64_t taxa = feitos * 1000000000 / seg_ns;
        const char *un = "H/s";
        int64_t v100 = taxa * 100;
        if(taxa >= 1000000){ v100 = taxa / 10000; un = "MH/s"; }
        else if(taxa >= 1000){ v100 = taxa / 10; un = "kH/s"; }
        printf("      %ld hash(es) em %" PRId64 ".%03" PRId64 " s — %" PRId64 ".%02" PRId64 " %s\n",
               feitos,
               seg_ns / 1000000000, (seg_ns % 1000000000) / 1000000,
               v100 / 100, v100 % 100, un);
    }
    if(nonce)
        printf("      SHARE no nonce %u — a cifra diverge PARA BAIXO no simbolo %u\n"
               "      (%ld passos de ISA)\n", nonce, (unsigned)r.R.e, passos);
    else
        printf("      faixa limpa: nenhum nonce dentro da bola. Zero e RESPOSTA, nao falha —\n"
               "      a faixa varrida e trabalho feito, e fecha-se na mesma.\n");
    return 1;
}
/* A REVERSÃO. O martelo é metade: ele PROCURA, e procurar estica — varre a faixa toda. A outra
 * metade CONTRAI: dado o nonce, confere-se de uma vez. É o chicote de sempre, e é ela que faz do
 * trabalho uma prova — quem acha paga a faixa inteira, quem confere paga um.
 *
 * E quem reverte é o ESQUILO, que já está na ISA e reverte em qualquer forma e qualquer régua:
 * ordem 4, quatro voltas e está-se onde se partiu. Não há reversão a escrever — há a usar. */
static int verifica(const char *p){
    long n = 0; pula(&p);
    if(!numero(&p, &n)) return 0;
    unsigned char cab[80], h1[32], h2[32], alvo[32];
    atomos_le(S_CAB, cab, 80);
    atomos_le(S_ALVO, alvo, 32);
    cab[76] = (unsigned char)n;         cab[77] = (unsigned char)(n >> 8);
    cab[78] = (unsigned char)(n >> 16); cab[79] = (unsigned char)(n >> 24);
    sha256(cab, 80, h1); sha256(h1, 32, h2);
    int k = 0, dentro = 0;
    while(k < 32){
        unsigned th = h2[31-k], ta = alvo[k];
        if(th != ta){ dentro = (th < ta); break; }
        k++;
    }
    printf("      nonce %ld: a cifra do hash e [", n);
    for(int i = 0; i < 6; i++) printf("%s%u", i?";":"", h2[31-i]);
    printf(";...]  contra o alvo, diverge no simbolo %d\n", k);
    printf("      %s\n", dentro ? "CONFERE — esta dentro da bola" : "nao confere");
    /* O ESQUILO, a reverter: quatro voltas e o elemento volta ao lugar. Resíduo 0 ou falha. */
    Par x = { (long)h2[31], (long)h2[30] }, y = x;
    for(int i = 0; i < 4; i++) y = cr_op(y, 0);
    printf("      o esquilo dado 4 vezes devolve (%ld,%ld) -> (%ld,%ld): residuo %s\n",
           x.a, x.b, y.a, y.b, (x.a == y.a && x.b == y.b) ? "0" : "NAO ZERO");
    printf("      procurar ESTICA (a faixa toda), conferir CONTRAI (um so) — e o chicote,\n");
    printf("      e e por isso que o trabalho e PROVA: caro de achar, barato de crer.\n");
    return dentro;
}
static int config(const char *p){
    char nome[64]; pula(&p);
    if(!ident(&p, nome, sizeof nome)) return 0;
    pula(&p);
    if(*p == '\'' || *p == '"'){
        char asp = *p++; char v[448]; size_t k = 0;
        while(*p && *p != asp && k < sizeof v - 1) v[k++] = *p++;
        v[k] = 0;
        conf_poe(nome, v);
        printf("      config %s guardada (%zu simbolo(s))\n", nome, k);
        return 1;
    }
    char v[448]; conf_le(nome, v, sizeof v);
    printf("      %s = %s\n", nome, v[0] ? v : "(vazia)");
    return 1;
}
/* MINERA: o worker continuo. Uma ligacao, um processo, e o banco a martelar.
 *
 * E o laco e a lei da liquidacao: A CADA VOLTA ELE ENTRA — le o pool, le a config — e cada
 * entrada dispara a verificacao. O tique e a leitura do socket com tempo-limite; a flag
 * mina_ativa e o contrato que se liquida quando alguem a poe a zero. */
static int minera(const char *p){
    long faixa = 0; pula(&p);
    /* PREDEFINICAO LEVE: isto e um TERMOMETRO, nao uma mina. Ninguem espera achar bloco — o que
     * se quer e saber se o sistema continua inteiro, e para isso basta um fio de trabalho.
     * (Os argumentos nao estao a ser lidos — o numero() falha aqui e ainda nao sei porque. Fica
     * dito, e a predefinicao ja da o comportamento certo.) */
    if(!numero(&p, &faixa) || faixa <= 0) faixa = 1 << 18;
    long descanso = 3000000;     /* 3 s entre faixas: ~8% de um nucleo */
    pula(&p); { long d; if(numero(&p, &d) && d >= 0) descanso = d; }
    pool_abre();
    if(!pool_ligado){ printf("sem pool: CONFIG pool_host e CONFIG pool_user\n"); return 0; }
    conf_poe("mina_ativa", "1");
    printf("ligado. faixa de %ld nonces. (CONFIG mina_ativa '0' para parar)\n", faixa);
    fflush(stdout);
    char jid[64] = ""; long de = 0, total = 0; int quieto = 0;
    for(;;){
        { char at[64]; conf_le("mina_ativa", at, sizeof at);
          if(at[0] == '0'){ printf("mina_ativa=0 — a parar.\n"); fflush(stdout); return 1; } }
        Word tem = mem_le(S_POOL + 11);
        if(!tem.total){
            /* SAUDE: sem job ha muito tempo e sinal de que a ligacao caiu. Reporta-se, e nao se
             * finge que esta tudo bem — e para isso que ele existe. */
            if(++quieto == 300){ printf("FALHA: 60 s sem job — a ligacao ao pool pode ter caido\n");
                                 fflush(stdout); }
            usleep(200000); continue;
        }
        quieto = 0;
        if(strcmp(jid, pool_st.job_id)){
            snprintf(jid, sizeof jid, "%s", pool_st.job_id);
            de = 0;
            /* O CABECALHO MONTA-SE DENTRO. Cada campo vai do slot do pool para o seu lugar no
             * S_CAB, sem passar por array nenhum — o maior temporario e uma PALAVRA de 4 bytes,
             * que e o campo em si e nao o objeto. */
            (void)mem_le(S_POOL+11);   /* refresca pool_st */
            unsigned v = pool_st.versao, nb = pool_st.nbits, nt = pool_st.ntime;
            { unsigned char z[80]; memset(z, 0, 80); atomos_grava(S_CAB, z, 80); }
            banco_poe(S_CAB, 0, (const unsigned char*)&v, 4);
            banco_poe(S_CAB, 4, pool_st.prevhash, 32);
            merkle_pelo_fold();
            banco_poe(S_CAB, 36, pool_st.merkle_raiz, 32);
            banco_poe(S_CAB, 68, (const unsigned char*)&nt, 4);
            banco_poe(S_CAB, 72, (const unsigned char*)&nb, 4);
            unsigned char alvo[32]; memset(alvo, 0, 32);
            int ex = (int)(nb >> 24); unsigned man = nb & 0xFFFFFF;
            for(int k = 0; k < 3; k++){
                int pos = 32 - (ex - 3) - 3 + k;
                if(pos >= 0 && pos < 32) alvo[pos] = (unsigned char)(man >> (16 - 8*k));
            }
            atomos_grava(S_ALVO, alvo, 32);
            /* SAUDE: o job tem de vir inteiro. Merkle a zero ou nbits a zero seria cabecalho
             * invalido — e nenhuma share sairia dai. Melhor gritar do que martelar em falso. */
            int zero = 1;
            for(int k = 0; k < 8; k++) if(mem_le(S_POOL + 12 + (unsigned)k).total) zero = 0;
            if(zero || !nb || !nt){
                printf("FALHA: job %s veio incompleto (merkle %s, nbits %08x, ntime %08x)\n",
                       jid, zero ? "ZERO" : "ok", nb, nt);
                fflush(stdout);
            }
            printf("job %s  nbits %08x  ntime %08x%s\n", jid, nb, nt, zero ? "  <- SUSPEITO" : "");
            fflush(stdout);
        }
        long ate = de + faixa; if(ate > 0xFFFFFFFFL) ate = 0xFFFFFFFFL;
        atomos_u32(S_FAIXA, (unsigned)de);
        atomos_u32(S_FAIXA + 4u, (unsigned)ate);
        pc_emit = 0;
        emit1(OP_MARTELO); emit1(OP_HALT);
        unsigned plen = pc_emit;
        Regs r; memset(&r, 0, sizeof r);
        long passos = 0;
        while(passo(&r, plen)){ if(++passos > 50000000L) break; }
        total += (ate - de);
        if(r.R.total){
            unsigned nonce = atomos_le_u32(S_FAIXA + 8u);
            printf("SHARE! nonce %u — a submeter\n", nonce);
            mem_grava(S_POOL_SH, w8(nonce & 0xFFu, 0));
            fflush(stdout);
        }
        de = ate;
        if(de >= 0xFFFFFFFFL) de = 0;
        usleep(descanso);        /* LEVE: isto e um termometro, nao uma mina. Deixa a maquina em paz. */
        if(total % (faixa * 8) == 0){
            printf("%ld Mhash, job %s\n", total/1000000, jid);
            fflush(stdout);
        }
    }
    return 1;
}
static int insere_corpos(void){
    long antes = txt_n();
    printf("      corpo             razao sinal dual  cifra completa\n");
    for(int i = 0; i < N28; i++){
        long a[128];
        size_t n = cifra_geral(CORPO28[i].per, CORPO28[i].np, CORPO28[i].B, CORPO28[i].C,
                               CORPO28[i].rd, a, 128);
        long ja = txt_n();
        cif_poe(a, n);
        printf("      %-17s %-3ld %-3ld %-4ld ", CORPO28[i].nome,
               CORPO28[i].B, CORPO28[i].C, CORPO28[i].rd);
        mostra_cifra(a, n);
        printf("%s\n", txt_n() == ja ? "   <- lugar ja tomado" : "");
    }
    printf("\n      %d corpos, %ld lugares distintos.\n", N28, txt_n() - antes);
    { int nr = 0; long vb[64], vc[64];
      for(int i = 0; i < N28; i++){ int ja = 0;
        for(int k = 0; k < nr; k++) if(vb[k]==CORPO28[i].B && vc[k]==CORPO28[i].C) ja = 1;
        if(!ja){ vb[nr]=CORPO28[i].B; vc[nr]=CORPO28[i].C; nr++; } }
      printf("      %d reguas (B,C) distintas.\n", nr); }
    return 1;
}
static int insere_texto(const char *p){
    long a[MAXT]; size_t n; char rot[128];
    if(!cifra_entrada(&p, a, MAXT, &n, rot, sizeof rot)) return 0;
    cif_poe(a, n);
    printf("entrada %-16s cifrada em ", rot); mostra_cifra(a, n); printf("\n");
    return 1;
}
/* A BUSCA DIRETA: desce a cifra do alvo. O custo e o COMPRIMENTO DA CIFRA — nao o tamanho da
 * tabela, nao o numero de linhas. E o prefixo comum, que e a distancia, E o caminho partilhado. */
static int acha_texto(const char *p){
    long a[MAXT]; size_t n, desceu; char rot[128];
    if(!cifra_entrada(&p, a, MAXT, &n, rot, sizeof rot)) return 0;
    long antes = n_leituras;
    long base = acha_cifra(a, n, &desceu);
    printf("      alvo %-16s cifra de %zu termo(s)\n", rot, n);
    if(base) printf("      ACHOU — %ld no(s) descidos, um por termo\n", n_leituras - antes);
    else     printf("      nao esta: o caminho morre ao %zu.o termo, em %ld leitura(s)\n",
                    desceu + 1, n_leituras - antes);
    return 1;
}
/* A varredura: compara CIFRA com CIFRA, e por isso um numero compara-se com uma palavra. */
static int busca_texto(const char *p){
    long a[MAXT]; size_t na; char rot[128];
    if(!cifra_entrada(&p, a, MAXT, &na, rot, sizeof rot)) return 0;
    if(txt_n() <= 0){ printf("(tabela vazia)\n"); return 1; }
    unsigned livre = (unsigned)mem_le(S_TXLIVRE).total;
    printf("      alvo: %s   ", rot); mostra_cifra(a, na); printf("\n\n");
    printf("      entrada            cifra              prefixo  distancia\n");
    size_t melhor = 0; unsigned vence = 0; int primeiro = 1;
    for(unsigned base = S_TEXTO; base < livre; base = reg_prox(base)){
        size_t nb = reg_n(base), pre = 0;
        while(pre < na && pre < nb && a[pre] == reg_termo(base, pre)) pre++;
        char vis[64]; reg_mostra(base, vis, sizeof vis);
        char cif[64]; int c = 0;
        for(size_t k = 0; k < nb && k < 5; k++)
            c += snprintf(cif+c, sizeof cif - c, "%s%ld", k?";":"[", reg_termo(base, k));
        snprintf(cif+c, sizeof cif - c, "%s", nb > 5 ? ";...]" : "]");
        printf("      %-18s %-18s %-8zu ", vis, cif, pre);
        if(nb == na && pre == na) printf("0\n");
        else if(pre < 62)         printf("1/%llu\n", 1ULL << pre);
        else                      printf("1/2^%zu\n", pre);
        if(primeiro || pre > melhor){ melhor = pre; vence = base; primeiro = 0; }
    }
    char vis[64]; reg_mostra(vence, vis, sizeof vis);
    printf("\n      MAIS PROXIMO: %s (prefixo %zu)\n", vis, melhor);
    return 1;
}

/* ---------------- IMPORT: linguagens e corpo entram pelo SQL ---------------- */
static int poe_chave_texto(const char *s){
    long a[MAXT]; size_t n; char rot[128];
    char buf[512]; const char *p = buf;
    snprintf(buf, sizeof buf, "'%s'", s);
    if(!cifra_entrada(&p, a, MAXT, &n, rot, sizeof rot)) return 0;
    cif_poe(a, n);
    return 1;
}
static void le_caminho_arg(const char **p, char *cam, size_t cap, const char *def){
    pula(p);
    if(**p == '\'' || **p == '"'){
        char asp = *(*p)++;
        size_t k = 0;
        while(**p && **p != asp && k + 1 < cap) cam[k++] = *(*p)++;
        cam[k] = 0;
        if(**p == asp) (*p)++;
    } else snprintf(cam, cap, "%s", def);
}
static int import_linguagens(const char *p){
    char cam[512];
    le_caminho_arg(&p, cam, sizeof cam, "../conecthus/backends/manifesto.json");
    FILE *f = fopen(cam, "rb");
    if(!f){ printf("nao abri: %s\n", cam); return 0; }
    fseek(f, 0, SEEK_END); long st = ftell(f); fseek(f, 0, SEEK_SET);
    if(st <= 0 || st > 1<<20){ fclose(f); return 0; }
    char *buf = (char*)malloc((size_t)st + 1);
    if(!buf){ fclose(f); return 0; }
    if((long)fread(buf, 1, (size_t)st, f) != st){ free(buf); fclose(f); return 0; }
    buf[st] = 0; fclose(f);
    long antes = txt_n(), postas = 0;
    for(char *q = buf; (q = strstr(q, "\"nome\"")); q++){
        char nome[64] = "", faz[64] = "";
        long pp = 0, qq = 0, rr = 0;
        if(sscanf(q, "\"nome\": \"%63[^\"]\"", nome) != 1) continue;
        char *bl = q; char *fim = q + 400; if(fim > buf + st) fim = buf + st;
        for(char *t = bl; t < fim; t++){
            if(!faz[0] && sscanf(t, "\"faz\": \"%63[^\"]\"", faz) == 1) continue;
            if(!pp && sscanf(t, "\"p\": %ld", &pp) == 1) continue;
            if(!qq && sscanf(t, "\"q\": %ld", &qq) == 1) continue;
            if(!rr && sscanf(t, "\"r\": %ld", &rr) == 1) continue;
        }
        if(!nome[0] || !faz[0]) continue;
        char entrada[160];
        snprintf(entrada, sizeof entrada, "linguagem/%s|%ld,%ld,%ld|%s", nome, pp, qq, rr, faz);
        if(poe_chave_texto(entrada)) postas++;
    }
    free(buf);
    printf("      IMPORT LINGUAGENS: %ld entradas (%ld -> %ld)\n", postas, antes, txt_n());
    return postas > 0;
}
/* Blobs do corpo: <base>_corpo/<rel> — fonte de verdade ao lado do .mem.
 * Índice TEXTO (corpo/<rel>) + bytes no disco do banco. GET CORPO devolve os bytes. */
static void corpo_dir(char *out, size_t cap){
    snprintf(out, cap, "%s_corpo", g_base[0] ? g_base : "/tmp/sql_corpo");
}
static int mkdirs_para(const char *caminho){
    char tmp[1024]; snprintf(tmp, sizeof tmp, "%s", caminho);
    for(char *q = tmp + 1; *q; q++){
        if(*q != '/') continue;
        *q = 0; mkdir(tmp, 0755); *q = '/';
    }
    return 1;
}
static int copia_ficheiro(const char *de, const char *para){
    FILE *in = fopen(de, "rb"); if(!in) return 0;
    mkdirs_para(para);
    FILE *out = fopen(para, "wb"); if(!out){ fclose(in); return 0; }
    unsigned char buf[8192]; size_t n; int ok = 1;
    while((n = fread(buf, 1, sizeof buf, in)) > 0)
        if(fwrite(buf, 1, n, out) != n){ ok = 0; break; }
    fclose(in); fclose(out);
    return ok;
}
static int import_corpo(const char *p){
    char cam[512], raiz[512], dest_raiz[640];
    le_caminho_arg(&p, cam, sizeof cam, "../app/src/corpo.json");
    le_caminho_arg(&p, raiz, sizeof raiz, "..");
    corpo_dir(dest_raiz, sizeof dest_raiz);
    mkdir(dest_raiz, 0755);
    FILE *f = fopen(cam, "rb");
    if(!f){ fprintf(stderr, "nao abri: %s\n", cam); return 0; }
    long antes = txt_n(), postas = 0, blobs = 0, lidos = 0;
    char lin[1024];
    while(fgets(lin, sizeof lin, f)){
        char rel[512];
        if(sscanf(lin, " \"%511[^\"]\",", rel) != 1 &&
           sscanf(lin, " \"%511[^\"]\"", rel) != 1) continue;
        if(!strchr(rel, '/') && !strchr(rel, '.')) continue;  /* só caminhos, não chaves JSON */
        lidos++;
        char de[1024], para[1280], chave[576];
        snprintf(de, sizeof de, "%s/%s", raiz, rel);
        snprintf(para, sizeof para, "%s/%s", dest_raiz, rel);
        snprintf(chave, sizeof chave, "corpo/%s", rel);
        if(poe_chave_texto(chave)) postas++;
        if(copia_ficheiro(de, para)) blobs++;
        else fprintf(stderr, "      sem blob: %s\n", rel);
    }
    fclose(f);
    printf("      IMPORT CORPO: %ld chaves, %ld blobs em %s (%ld ficheiros; txt %ld -> %ld)\n",
           postas, blobs, dest_raiz, lidos, antes, txt_n());
    return blobs > 0 || postas > 0;
}
/* GET CORPO 'rel' — bytes crus em stdout; estado em stderr. Vite /corpo/* usa isto. */
static int get_corpo(const char *p){
    char rel[512], para[1280], raiz[640];
    le_caminho_arg(&p, rel, sizeof rel, "");
    if(!rel[0]){ fprintf(stderr, "GET CORPO: falta o caminho\n"); return 0; }
    if(!strncmp(rel, "corpo/", 6)) memmove(rel, rel + 6, strlen(rel + 6) + 1);
    corpo_dir(raiz, sizeof raiz);
    snprintf(para, sizeof para, "%s/%s", raiz, rel);
    FILE *f = fopen(para, "rb");
    if(!f){ fprintf(stderr, "GET CORPO: nao esta no banco: %s\n", rel); return 0; }
    unsigned char buf[8192]; size_t n;
    while((n = fread(buf, 1, sizeof buf, f)) > 0)
        if(fwrite(buf, 1, n, stdout) != n){ fclose(f); return 0; }
    fclose(f);
    fflush(stdout);
    fprintf(stderr, "GET CORPO: %s (%s)\n", rel, para);
    return 1;
}

/* Idioma natural = álgebra byte-level: idioma/<iso>/{alfabeto,lexico,regra,orbita}/…
 * Ficheiros: lib/classe/{portugues,ingles,espanhol}_idioma.txt
 *            lib/classe/corpus_orbitas_pt.txt  (tipo orbita → idioma/pt/orbita/…) */
static int import_idioma(const char *p){
    char iso[16], cam[512], iso_norm[8], def[128];
    pula(&p);
    iso[0] = 0;
    if(*p == '\'' || *p == '"'){
        char asp = *p++; size_t k = 0;
        while(*p && *p != asp && k + 1 < sizeof iso) iso[k++] = *p++;
        iso[k] = 0;
        if(*p == asp) p++;
    } else if(*p){
        size_t k = 0;
        while(*p && *p != ' ' && *p != '\t' && k + 1 < sizeof iso) iso[k++] = *p++;
        iso[k] = 0;
    }
    if(!iso[0]) snprintf(iso, sizeof iso, "pt");
    if(!strcasecmp(iso, "pt") || !strcasecmp(iso, "portugues"))
        snprintf(iso_norm, sizeof iso_norm, "pt");
    else if(!strcasecmp(iso, "en") || !strcasecmp(iso, "ingles") || !strcasecmp(iso, "english"))
        snprintf(iso_norm, sizeof iso_norm, "en");
    else if(!strcasecmp(iso, "es") || !strcasecmp(iso, "espanhol") || !strcasecmp(iso, "spanish"))
        snprintf(iso_norm, sizeof iso_norm, "es");
    else {
        printf("IMPORT IDIOMA: pt|en|es neste passo (pediu '%s')\n", iso);
        return 0;
    }
    if(!strcmp(iso_norm, "en"))
        snprintf(def, sizeof def, "lib/classe/ingles_idioma.txt");
    else if(!strcmp(iso_norm, "es"))
        snprintf(def, sizeof def, "lib/classe/espanhol_idioma.txt");
    else
        snprintf(def, sizeof def, "lib/classe/portugues_idioma.txt");
    le_caminho_arg(&p, cam, sizeof cam, def);
    {
        const char *q = p; pula(&q);
        if(*q && *q != '\'' && *q != '"' && strcmp(cam, def) == 0){
            size_t k = 0;
            while(*q && *q != '\n' && *q != '\r' && k + 1 < sizeof cam) cam[k++] = *q++;
            cam[k] = 0;
        }
    }
    FILE *f = fopen(cam, "rb");
    if(!f){
        snprintf(cam, sizeof cam, "../%s", def);
        f = fopen(cam, "rb");
    }
    if(!f){ printf("nao abri idioma %s\n", iso_norm); return 0; }
    long antes = txt_n(), postas = 0;
    char lin[512];
    while(fgets(lin, sizeof lin, f)){
        if(lin[0] == '#' || lin[0] == '\n' || lin[0] == '\r') continue;
        char tipo[32], resto[400];
        if(sscanf(lin, "%31s %399[^\n\r]", tipo, resto) != 2) continue;
        char entrada[480];
        if(!strcmp(tipo, "alfabeto"))
            snprintf(entrada, sizeof entrada, "idioma/%s/alfabeto|%s", iso_norm, resto);
        else if(!strcmp(tipo, "lexico"))
            snprintf(entrada, sizeof entrada, "idioma/%s/lexico/%s", iso_norm, resto);
        else if(!strcmp(tipo, "regra"))
            snprintf(entrada, sizeof entrada, "idioma/%s/regra/%s", iso_norm, resto);
        else if(!strcmp(tipo, "orbita"))
            /* resto = slug|n=…;maxG=…;tilde1=…;ck=…  → idioma/<iso>/orbita/<slug>|meta */
            snprintf(entrada, sizeof entrada, "idioma/%s/orbita/%s", iso_norm, resto);
        else continue;
        if(poe_chave_texto(entrada)) postas++;
    }
    fclose(f);
    printf("      IMPORT IDIOMA %s: %ld chaves (%ld -> %ld) de %s\n",
           iso_norm, postas, antes, txt_n(), cam);
    return postas > 0;
}

static int executa(const char *sql){
    const char *p = sql;
    if(palavra(&p, "CREATE")){ if(!palavra(&p, "TABLE")) return 0; return cria(p); }
    if(palavra(&p, "INSERT")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return insere_texto(q+5);
        return insere(p);
    }
    if(palavra(&p, "IMPORT")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "LINGUAGENS", 10)) return import_linguagens(q+10);
        if(!strncasecmp(q, "CORPO", 5)) return import_corpo(q+5);
        if(!strncasecmp(q, "IDIOMA", 6)) return import_idioma(q+6);
        return 0;
    }
    if(palavra(&p, "GET")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "CORPO", 5)) return get_corpo(q+5);
        return 0;
    }
    if(palavra(&p, "BUSCA")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return busca_texto(q+5);
        return 0;
    }
    if(palavra(&p, "CORPOS")) return insere_corpos();
    if(palavra(&p, "CABECALHO")) return cabecalho(p);
    if(palavra(&p, "MARTELO")) return martelo(p);
    if(palavra(&p, "MINERA")) return minera(p);
    if(palavra(&p, "CONFIG")) return config(p);
    if(palavra(&p, "VERIFICA")) return verifica(p);
    if(palavra(&p, "ACHA")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return acha_texto(q+5);
        return 0;
    }
    if(palavra(&p, "SELECT")) return varre(p, ACAO_MARCA);
    if(palavra(&p, "UPDATE")) return varre(p, ACAO_SET);
    if(palavra(&p, "DELETE")) return varre(p, ACAO_APAGA);
    if(palavra(&p, "DISTANCIA")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return distancia_texto(q+5);
        return distancia();
    }
    printf("nao entendi: %s\n", sql);
    return 0;
}

static int abrir_base(const char *base){
    char m[512], g[512];
    snprintf(g_base, sizeof g_base, "%s", base);
    g_tabela[0] = 0;                       /* a base abre no ficheiro sem nome */
    snprintf(m, sizeof m, "%s.mem", base);
    snprintf(g, sizeof g, "%s.prog", base);
    fmem  = open(m, O_RDWR|O_CREAT, 0644);
    fprog = open(g, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fmem < 0 || fprog < 0) return 0;
    ftruncate(fmem, (off_t)(S_CB + 65536u) * (off_t)SLOTSZ);  /* Words=16 átomos + blobs */
    refaz_diario();          /* antes de qualquer comando: fechar o que ficou aberto */
    return 1;
}

/* ── A TABELA É UM FICHEIRO .mem ─────────────────────────────────────────────
 * O motor sempre soube ler UMA relação: o catálogo é `{ncols, nrows}` no slot 0
 * e as linhas moram em S_LINHAS. Era por isso que a base só tinha uma tabela —
 * `CREATE TABLE beta` escrevia por cima da `alfa`, e (desde que a relação ganhou
 * nome) o `SELECT * FROM alfa` seguinte era recusado.
 *
 * O que faltava não era um motor novo: era outro FICHEIRO. A memória É o disco,
 * e o banco vê sempre um slot a ser lido e um slot a ser escrito — trocar de
 * tabela é trocar o descritor. `<base>.mem` continua a ser a tabela sem nome (as
 * bases antigas não mexem), e cada tabela nomeada é `<base>__<nome>.mem`.
 *
 * Assim o mapa de slots não muda uma linha, o `varre` não sabe que há mais
 * tabelas, e o catálogo da base é a listagem do directório. */
static void caminho_tabela(const char *nome, char *out, size_t cap, const char *ext){
    if(!nome || !nome[0]) snprintf(out, cap, "%s%s", g_base, ext);
    else                  snprintf(out, cap, "%s__%s%s", g_base, nome, ext);
}

static int tabela_existe(const char *nome){
    char m[600];
    caminho_tabela(nome, m, sizeof m, ".mem");
    return access(m, F_OK) == 0;
}

/* Troca o .mem aberto para o da tabela `nome`. cria=1 permite criar o ficheiro.
 * Devolve 0 quando a tabela não existe (e aí nada se troca). */
static int usa_tabela_z(const char *nome, int cria_se_falta, int zera){
    char alvo[600], baixo[64];
    int i;
    if(!nome) nome = "";
    for(i = 0; nome[i] && i < (int)sizeof baixo - 1; i++) baixo[i] = baixa1(nome[i]);
    baixo[i] = 0;
    if(!zera && !strcmp(g_tabela, baixo) && fmem >= 0) return 1;   /* já é esta */

    caminho_tabela(baixo, alvo, sizeof alvo, ".mem");
    if(!cria_se_falta && access(alvo, F_OK) != 0){
        /* COMPATIBILIDADE: uma base antiga tem tudo em <base>.mem, sem ficheiro
         * por tabela. Se o nome bate com o que lá está guardado (ou se ela é tão
         * antiga que nem nome tem), é essa a tabela — e nada se recusa. */
        if(!g_tabela[0] && cat_nome_bate(baixo)) return 1;
        return 0;
    }
    if(fmem >= 0){ fsync(fmem); close(fmem); }
    fmem = open(alvo, O_RDWR | (cria_se_falta ? O_CREAT : 0) | (zera ? O_TRUNC : 0), 0644);
    if(fmem < 0) return 0;
    ftruncate(fmem, (off_t)(S_CB + 65536u) * (off_t)SLOTSZ);
    snprintf(g_tabela, sizeof g_tabela, "%s", baixo);
    refaz_diario();                       /* cada tabela tem o seu diário */
    return 1;
}

/* CREATE TABLE começa a tabela LIMPA — é o que o motor sempre fez quando havia uma
 * só (o catálogo era reescrito com nrows = 0). Com uma tabela por ficheiro isso
 * deixou de bastar: o ficheiro sobrevivia entre corridas, e o medidor do sql, que
 * cria as mesmas tabelas todas as vezes, passou a dar resultado diferente na
 * segunda corrida. Um medidor que depende do que ficou de antes não mede. */
static int usa_tabela(const char *nome, int cria_se_falta){
    return usa_tabela_z(nome, cria_se_falta, 0);
}
static void fechar_base(void){ if(fmem>=0){fsync(fmem);close(fmem);} if(fprog>=0){fsync(fprog);close(fprog);} }

/* Porta C (Trio PG2): mesma base que o CLI; captura SELECT/tags em SqlOut. */
int sql_abrir(const char *base){
    static int disco_ok = 0;
    if(!disco_ok){
        disco_prende(DISCO_BASE(24),"dados/sql_pool.bin",(size_t)1,sizeof(Pool));
        disco_prende(DISCO_BASE(25),"dados/sql_rel.bin",(size_t)NREL,sizeof(Rel));
        disco_ok = 1;
    }
    return abrir_base(base);
}
void sql_fechar(void){ fechar_base(); sql_cap = NULL; }
int sql_executa(const char *sql, SqlOut *out){
    if(out){ memset(out, 0, sizeof *out); sql_cap = out; }
    else sql_cap = NULL;
    int r = executa(sql);
    if(out){
        out->ok = r;
        if(!r && !out->err[0])
            snprintf(out->err, sizeof out->err, "comando recusado ou nao entendido");
        sql_cap = NULL;
    }
    return r;
}

#ifndef SQL_NO_MAIN
int main(int argc, char **argv){
    disco_prende(DISCO_BASE(24),"dados/sql_pool.bin",(size_t)1,sizeof(Pool));
    disco_prende(DISCO_BASE(25),"dados/sql_rel.bin",(size_t)NREL,sizeof(Rel));
    if(argc >= 2 && !strcmp(argv[1], "teste")){
        const char *base = "/tmp/sql_teste";
        unlink("/tmp/sql_teste.mem"); unlink("/tmp/sql_teste.prog");
        if(!abrir_base(base)) return 2;
        printf("\n=== SQL NO METAL: compila para a ISA, memória no disco, sem RAM ==========\n\n");
        printf("$ CREATE TABLE t (a,b,c)\n"); executa("CREATE TABLE t (a,b,c)");
        printf("\n");
        const char *ins[5] = {
            "INSERT INTO t VALUES (7,10,20)",
            "INSERT INTO t VALUES (3,30,40)",
            "INSERT INTO t VALUES (7,50,60)",
            "INSERT INTO t VALUES (9,70,80)",
            "INSERT INTO t VALUES (3,90,99)" };
        for(int i = 0; i < 5; i++){ printf("$ %s\n", ins[i]); executa(ins[i]); }
        printf("\n$ SELECT * FROM t\n");  executa("SELECT * FROM t");

        printf("\n-- A CONTRAÇÃO NUMÉRICA: a expressão vira vetor, e o vetor decide.\n");
        printf("   ([xxxx] = soma do bytecode; escritas equivalentes têm de dar a mesma)\n");
        const char *pares[][2] = {
            {"SELECT * FROM t WHERE b > 20",        "SELECT * FROM t WHERE a + b - a > 20"},
            {"SELECT * FROM t WHERE b > 20",        "SELECT * FROM t WHERE b - 20 > 0"},
            {"SELECT * FROM t WHERE a = 3",         "SELECT * FROM t WHERE a + 0 = 3 + 0"},
            {"SELECT * FROM t WHERE b > 20",        "SELECT * FROM t WHERE 20 < b"},
        };
        for(unsigned q = 0; q < sizeof pares/sizeof pares[0]; q++){
            printf("\n$ %s\n", pares[q][0]); executa(pares[q][0]);
            printf("$ %s\n", pares[q][1]);   executa(pares[q][1]);
        }

        printf("\n-- expressão dos DOIS lados, com coeficiente:\n");
        printf("\n$ SELECT * FROM t WHERE 2*a + b > c\n");
        executa("SELECT * FROM t WHERE 2*a + b > c");
        printf("\n$ SELECT * FROM t WHERE b - a > 30\n");
        executa("SELECT * FROM t WHERE b - a > 30");

        printf("\n-- O COEFICIENTE NAS COORDENADAS DO REI: GOLD é o deslocamento.\n");
        printf("   Antes: soma repetida |c| vezes, com o coeficiente TRUNCADO em silêncio se\n");
        printf("   passasse de 8 — a resposta saía errada sem aviso. Agora c·x = Σ F(k)·x, e\n");
        printf("   cada F(k) é GOLD^(k−1): o opcode já estava na ISA (broca-so ula/cifra.c).\n\n");
        printf("   (cada linha diz os bytes; a soma repetida gastaria 10 por unidade)\n");
        {
            long cs[] = {1, 2, 5, 13, 34, 100, 1000, 100000};
            for(unsigned q = 0; q < sizeof cs/sizeof cs[0]; q++){
                char buf[128];
                snprintf(buf, sizeof buf, "SELECT * FROM t WHERE a * %ld = 0", cs[q]);
                printf("$ c = %-7ld  (soma repetida gastaria %ld bytes)\n", cs[q], cs[q]*10);
                executa(buf);
            }
        }
        printf("\n   O bytecode cresce com o número de dígitos de Zeckendorf (~log_φ c), e não\n");
        printf("   com o valor: de c=1000 para c=100000 o valor faz cem vezes e o código não\n");
        printf("   chega a três.\n");

        printf("\n-- e a contração continua a identificar as escritas equivalentes:\n");
        printf("\n$ SELECT * FROM t WHERE a + a = 14\n");
        executa("SELECT * FROM t WHERE a + a = 14");
        printf("$ SELECT * FROM t WHERE a * 2 = 14\n");
        executa("SELECT * FROM t WHERE a * 2 = 14");
        printf("$ SELECT * FROM t WHERE a * 12 - a * 12 = 0   (contrai a constante)\n");
        executa("SELECT * FROM t WHERE a * 12 - a * 12 = 0");

        printf("\n-- A BARREIRA DO BANCO: dado, fsync, ponteiro, fsync. E a queda no meio.\n");
        printf("   O INSERT era um programa só: as células e o catálogo sem nada entre eles.\n");
        printf("   Ordem de programa não é ordem no disco — sem fsync o catálogo pode chegar\n");
        printf("   ao prato antes das células, e a queda deixa o catálogo a contar uma linha\n");
        printf("   que não existe. Agora são duas fases com barreira, e mede-se a queda.\n\n");
        {
            Word c0 = mem_le(S_CAT);
            printf("   linhas antes da queda                    %d\n", c0.e);
            pid_t f = fork();
            if(f == 0){                       /* o filho cai entre o dado e o ponteiro */
                trava_em = 1;
                insere(" INTO t VALUES (555,666,777)");
                _exit(0);
            }
            int st = 0; waitpid(f, &st, 0);
            printf("   o filho saiu com                         %d (9 = derrubado na barreira)\n",
                   WIFEXITED(st) ? WEXITSTATUS(st) : -1);
            Word c1 = mem_le(S_CAT);
            printf("   linhas depois da queda                   %d\n", c1.e);
            printf("   %s\n", c0.e == c1.e ? "a linha caída é INVISÍVEL — nunca meia ✓"
                                             : "O CATÁLOGO MOVEU-SE SEM O DADO ✗");
            printf("\n   E a base segue escrevendo por cima do órfão:\n");
        }
        printf("$ INSERT INTO t VALUES (1,2,3)\n");
        executa("INSERT INTO t VALUES (1,2,3)");
        printf("$ SELECT * FROM t\n");
        executa("SELECT * FROM t");
        printf("\n   As duas quedas possíveis são as duas boas: antes do ponteiro, a linha não\n");
        printf("   existe e não faz mal; depois dele, está inteira. Nunca meia.\n");
        printf("   (_exit modela queda de PROCESSO — o que já foi ao núcleo sobrevive. Contra\n");
        printf("    queda de energia quem responde é o fsync, e é por isso que ele está lá.)\n");

        printf("\n-- A PA NO ENDEREÇO: um molde só, e a progressão varre a tabela.\n");
        printf("   A ISA não tem endereçamento indireto — o slot é imediato, e LOADS, que eu\n");
        printf("   esperava indireto, é LOAD com a cifra espectral (fui ver em broca-so). Por\n");
        printf("   isso a varredura era DESENROLADA: um bloco por linha, e o bytecode crescia\n");
        printf("   linearmente com a tabela — 147 bytes por linha, medidos.\n\n");
        printf("   Mas o endereço da linha É uma PA: linha i, coluna j mora em\n");
        printf("   S_LINHAS + i·ncols + j, passo constante. Então emite-se UM molde e ANDA-SE\n");
        printf("   com ele. O passo sai da faixa do slot, sem tocar em lugar de chamada nenhum.\n\n");
        printf("   linhas na tabela   bytes de ISA da consulta\n");
        {
            long antes = mem_le(S_CAT).e;
            printf("   %-18ld ", antes);
            executa("SELECT * FROM t WHERE a = 7");
        }
        printf("\n   Antes: O(linhas). Agora: O(1). Quem varre é a progressão, não o compilador —\n");
        printf("   e é a PA a fazer o trabalho para que ela foi feita.\n");

        printf("\n-- O DIÁRIO: o UPDATE e o DELETE são TUDO OU NADA.\n");
        printf("   O INSERT tinha ponteiro natural (o catálogo) e bastava a ordem. Estes não:\n");
        printf("   mudam em cima, e uma queda no meio da varredura deixaria metade das linhas\n");
        printf("   mudadas — que é pior que não ter mudado nenhuma, porque ninguém sabe qual.\n\n");
        printf("   Agora a varredura só MARCA (o bitmap é o diário), e depois vêm três passos:\n");
        printf("     1  o diário no disco          barreira\n");
        printf("     2  o COMPROMISSO no disco     barreira   ← daqui em diante, vai acontecer\n");
        printf("     3  a aplicação                barreira\n");
        printf("     4  o diário fechado           barreira\n");
        printf("   E a abertura confere: diário aberto quer dizer queda entre 2 e 4, e REFAZ.\n");
        printf("   A aplicação escreve valores absolutos, nunca incrementos — por isso refazer\n");
        printf("   duas vezes dá o mesmo que refazer uma.\n\n");
        {
            const int pontos[3] = {2, 3, 4};
            const char *quando[3] = {"antes do compromisso", "depois do compromisso",
                                     "depois de aplicar"};
            const char *espera[3] = {"nada mudou", "refez e completou", "refez, idempotente"};
            for(int q = 0; q < 3; q++){
                char mem[512], sv[512];
                snprintf(mem, sizeof mem, "%s.mem", base);
                snprintf(sv,  sizeof sv,  "%s.mem.sv", base);
                fechar_base();
                { char cmd[1100]; snprintf(cmd, sizeof cmd, "cp %s %s", mem, sv); if(system(cmd)){} }
                abrir_base(base);
                pid_t f = fork();
                if(f == 0){ trava_em = pontos[q]; varre(" t SET c = 42 WHERE a >= 7", ACAO_SET); _exit(0); }
                int st = 0; waitpid(f, &st, 0);
                fechar_base(); abrir_base(base);
                long ncols = mem_le(S_CAT).total, nrows = mem_le(S_CAT).e, mudadas = 0;
                for(long i = 0; i < nrows; i++)
                    if(mem_le(S_LINHAS + (unsigned)(i*ncols + 2)).total == 42) mudadas++;
                printf("   queda %-22s saiu %d   linhas com o valor novo: %ld   (%s)\n",
                       quando[q], WIFEXITED(st) ? WEXITSTATUS(st) : -1, mudadas, espera[q]);
                fechar_base();
                { char cmd[1100]; snprintf(cmd, sizeof cmd, "mv %s %s", sv, mem); if(system(cmd)){} }
                abrir_base(base);
            }
        }
        printf("\n   Antes do compromisso: nenhuma. Depois: TODAS as que casavam. Nunca um\n");
        printf("   pedaço — e quem fecha a conta é a abertura, sozinha, sem ninguém pedir.\n");

        /* PASSO 1 DO CATÁLOGO EM SQL: a coluna declara o seu corpo, e o catálogo guarda.
         * Testa-se sozinho — é só ler de volta o que o CREATE escreveu. */
        printf("\n-- O CORPO DA COLUNA (passo 1 de 6, ver docs/TOOLKIT.md)\n\n");
        {
            executa("CREATE TABLE k (a RACIONAL, b AUREO(2), c MORFICO(8), d)");
            struct { int col, corpo, parm; const char *rot; } cs[] = {
              {0, CORPO_RACIONAL, 0, "a coluna RACIONAL fica guardada como tal"},
              {1, CORPO_AUREO,    2, "AUREO(2) guarda o corpo E o metal"},
              {2, CORPO_MORFICO,  8, "MORFICO(8) guarda o corpo E o n"},
              {3, CORPO_INTEIRO,  0, "sem tipo é INTEIRO — a base antiga não muda"},
            };
            for(unsigned q = 0; q < sizeof cs/sizeof cs[0]; q++){
                Word w = mem_le(S_CORPO + (unsigned)cs[q].col);
                ok(cs[q].rot, w.total == cs[q].corpo && w.e == cs[q].parm);
            }
            /* E O CORPO TEM DE SOBREVIVER A UMA CONSULTA — que é onde ele morria.
             *
             * O corpo vive em S_CORPO..S_CORPO+7 e a árvore do WHERE começava em 64,
             * que é S_CORPO+4: numa tabela de CINCO colunas ou mais, o primeiro
             * SELECT com expressão escrevia o temporário por cima da declaração e
             * DEIXAVA-O NO DISCO. Uma coluna MORFICO(6) — o par (3,6) — voltava
             * (1,0), que é RACIONAL, e a álgebra daquela coluna passava a ser outra
             * sem que nada o dissesse. Todas as tabelas dos medidores tinham três
             * colunas, e por isso ninguém lá chegou.
             *
             * A asserção mede o PAR (corpo, parâmetro) ANTES e DEPOIS: são os dois
             * lados do mesmo facto, e sem o «antes» a igualdade podia ser dois lixos
             * iguais. E o caso escolhido tem SEIS colunas de propósito — com três,
             * este medidor passava sem tocar no defeito. */
            {
                Word a0, a1, d0, d1;
                executa("CREATE TABLE seis (a,b,c,d,e MORFICO,f AUREO)");
                executa("INSERT INTO seis VALUES (1,2,3,4,5,6)");
                a0 = mem_le(S_CORPO + 4); a1 = mem_le(S_CORPO + 5);
                executa("SELECT * FROM seis WHERE 2*a + b - c + d - a > 0");
                d0 = mem_le(S_CORPO + 4); d1 = mem_le(S_CORPO + 5);
                printf("     coluna 4: (%u,%u) antes → (%u,%u) depois do WHERE\n",
                       (unsigned)a0.total, (unsigned)a0.e, (unsigned)d0.total, (unsigned)d0.e);
                printf("     coluna 5: (%u,%u) antes → (%u,%u) depois do WHERE\n",
                       (unsigned)a1.total, (unsigned)a1.e, (unsigned)d1.total, (unsigned)d1.e);
                ok("o corpo da coluna SOBREVIVE a um SELECT com WHERE, e mede-se numa"
                   " tabela de SEIS colunas — a árvore da expressão pisava S_CORPO+4..7"
                   " e a declaração MORFICO(6) voltava RACIONAL, no disco e em silêncio",
                   a0.total == CORPO_MORFICO && a0.e == 6
                   && a1.total == CORPO_AUREO && a1.e == 1
                   && d0.total == a0.total && d0.e == a0.e
                   && d1.total == a1.total && d1.e == a1.e);
            }
            /* ── VÁRIAS TABELAS NA MESMA BASE, e cada uma é um ficheiro ──────────
             * O motor lê UMA relação: catálogo no slot 0, linhas em S_LINHAS. Era por
             * isso que a base só tinha uma tabela — `CREATE TABLE beta` escrevia por
             * cima da `alfa`. O que faltava não era motor novo: era outro FICHEIRO.
             * `<base>__<nome>.mem`, e trocar de tabela é trocar o descritor.
             *
             * Mede-se o que PODE falhar: as duas coexistem com formas diferentes, a
             * primeira sobrevive a criar e usar a segunda, o WHERE corre na certa, e
             * uma tabela que não existe é RECUSADA — sem isso, «multi-tabela» passava
             * com uma tabela só e um nome ignorado. */
            {
                SqlOut a1, b1, a2, gg;
                executa("CREATE TABLE alfa (a,b)");
                executa("INSERT INTO alfa VALUES (1,2)");
                executa("INSERT INTO alfa VALUES (3,4)");
                executa("CREATE TABLE beta (a,b,c)");
                executa("INSERT INTO beta VALUES (9,8,7)");
                sql_cap = &a1; memset(&a1, 0, sizeof a1); executa("SELECT * FROM alfa");
                sql_cap = &b1; memset(&b1, 0, sizeof b1); executa("SELECT * FROM beta");
                sql_cap = &a2; memset(&a2, 0, sizeof a2); executa("SELECT * FROM alfa WHERE a = 3");
                sql_cap = &gg; memset(&gg, 0, sizeof gg);
                int achou_gama = executa("SELECT * FROM gama");
                sql_cap = NULL;
                printf("\n     alfa %dx%d · beta %dx%d · alfa WHERE %dx%d · gama %s\n",
                       a1.nrows, a1.ncols, b1.nrows, b1.ncols, a2.nrows, a2.ncols,
                       achou_gama ? "ACEITE (mau)" : "recusada");
                ok("UMA BASE TEM VÁRIAS TABELAS, e a tabela é um ficheiro .mem: alfa (2x2) e"
                   " beta (1x3) coexistem com formas DIFERENTES, a alfa volta inteira depois"
                   " de a beta ser criada e usada, o WHERE corre na tabela certa, e a `gama`,"
                   " que não existe, é RECUSADA. Antes disto o CREATE seguinte escrevia por"
                   " cima do anterior e o nome do FROM era lido e deitado fora",
                   a1.nrows == 2 && a1.ncols == 2 &&
                   b1.nrows == 1 && b1.ncols == 3 &&
                   a2.nrows == 1 && a2.ncols == 2 && !strcmp(a2.cell[0][0], "3") &&
                   !achou_gama);
            }
            /* ── E O ENVELOPE DA CÉLULA RECUSA EM VEZ DE ENROLAR ─────────────────
             * A célula é uma Word_8²: um átomo para o numerador. `INSERT VALUES
             * (500,...)` era ACEITE e lido de volta como 244 — o dado perdia-se no
             * disco sem uma palavra. Os dois lados: 255 encosta e entra, 256 é
             * recusado. Medir só o que passa deixava «recusa sempre» indistinguível. */
            {
                SqlOut e1, e2, e3;
                executa("CREATE TABLE env (pequeno,grande)");
                int cabe = executa("INSERT INTO env VALUES (255,1000)");
                int alto = executa("INSERT INTO env VALUES (9,65535)");   /* o largo é o «grande» */
                int nao  = executa("INSERT INTO env VALUES (65536,0)");
                int neg  = executa("INSERT INTO env VALUES (-1,0)");
                sql_cap = &e1; memset(&e1, 0, sizeof e1); executa("SELECT * FROM env");
                /* e a outra metade: o WHERE numa coluna estreita corre; numa LARGA
                 * é recusado, porque o avaliador é de oito bits e comparar metade
                 * de um valor era responder à toa */
                sql_cap = &e2; memset(&e2, 0, sizeof e2);
                int cmp_estreita = executa("SELECT * FROM env WHERE pequeno = 255");
                sql_cap = &e3; memset(&e3, 0, sizeof e3);
                int cmp_larga = executa("SELECT * FROM env WHERE grande > 100");
                sql_cap = NULL;
                printf("     65535 %s · 65536 %s · −1 %s · lido [%s][%s] e [%s][%s]\n",
                       (cabe && alto) ? "entra" : "RECUSADO",
                       nao ? "ENTRA (mau)" : "recusado", neg ? "ENTRA (mau)" : "recusado",
                       e1.nrows > 0 ? e1.cell[0][0] : "—", e1.nrows > 0 ? e1.cell[0][1] : "—",
                       e1.nrows > 1 ? e1.cell[1][0] : "—", e1.nrows > 1 ? e1.cell[1][1] : "—");
                printf("     WHERE na estreita %s · na LARGA %s\n",
                       cmp_estreita ? "corre" : "RECUSADO (mau)",
                       cmp_larga ? "CORRE (mau)" : "recusado");
                ok("O DADO TEM DEZASSEIS BITS E O QUE NÃO CABE É RECUSADO: o byte alto vive"
                   " num plano paralelo (S_ALTO), como o denominador já vivia em S_DEN, e a"
                   " Word da linha não muda — toda a aritmética emitida continua a ler o que"
                   " sempre leu. 1000 e 65535 entram e voltam EXACTOS; 65536 e −1 são"
                   " recusados. E A METADE QUE FALTA DIZ-SE: o avaliador do WHERE é"
                   " polinomial e de oito bits, logo um WHERE que cite uma coluna com"
                   " valores acima de 255 é RECUSADO com o motivo escrito — o valor está"
                   " guardado inteiro e o SELECT devolve-o; o que não se faz é comparar"
                   " metade dele. Antes, o 500 entrava como 244 e nada o dizia",
                   cabe && alto && !nao && !neg && e1.nrows == 2
                   && !strcmp(e1.cell[0][0], "255") && !strcmp(e1.cell[0][1], "1000")
                   && !strcmp(e1.cell[1][1], "65535")
                   && cmp_estreita && e2.nrows == 1 && !cmp_larga);
            }
            /* ── OS NOMES DAS COLUNAS ────────────────────────────────────────────
             * `CREATE TABLE cliente (nome,idade,saldo)` lia os três identificadores
             * e DEITAVA-OS FORA. O SELECT devolvia `a`,`b`,`c`; o `WHERE idade > 20`
             * saía «comando recusado»; e o `UPDATE ... SET saldo` pedia a coluna
             * 's'−'a' = 18 numa tabela de três, porque a coluna era decidida por
             * `nome[0] - 'a'`.
             *
             * Mede-se o que PODE falhar, e pelos dois lados: os nomes voltam no
             * SELECT, decidem no WHERE e no SET, e uma coluna que NÃO existe é
             * recusada — sem a recusa, «resolve por nome» passava com a letra a
             * fazer o trabalho na mesma. E a LETRA continua a valer onde não há
             * nomes guardados, senão todas as bases antigas caíam. */
            {
                SqlOut c1, c2, c3;
                executa("CREATE TABLE cliente (nome,idade,saldo)");
                executa("INSERT INTO cliente VALUES (1,30,200)");
                executa("INSERT INTO cliente VALUES (2,17,50)");
                sql_cap = &c1; memset(&c1, 0, sizeof c1);
                executa("SELECT * FROM cliente WHERE idade > 20");
                sql_cap = &c2; memset(&c2, 0, sizeof c2);
                int mau_nome = executa("SELECT * FROM cliente WHERE altura > 20");
                /* e a LETRA deixa de valer onde há nomes: numa tabela cujas colunas se
                 * chamam nome/idade/saldo, `WHERE a` não é a coluna 0 — é uma coluna que
                 * não existe. Sem este caso, tirar a guarda que o garante não fazia cair
                 * nada, porque «altura» tem mais de uma letra e era recusada na mesma. */
                SqlOut cl; sql_cap = &cl; memset(&cl, 0, sizeof cl);
                int letra_vale = executa("SELECT * FROM cliente WHERE a > 20");
                sql_cap = &c3; memset(&c3, 0, sizeof c3);
                int set_mau = executa("UPDATE cliente SET altura = 1 WHERE idade > 20");
                sql_cap = NULL;
                int set_bom = executa("UPDATE cliente SET saldo = 99 WHERE idade > 20");
                SqlOut c4; sql_cap = &c4; memset(&c4, 0, sizeof c4);
                executa("SELECT * FROM cliente");
                sql_cap = NULL;
                printf("\n     colunas: [%s][%s][%s] · WHERE idade>20 → %d linha(s)"
                       " · «altura» %s · SET saldo %s\n",
                       c1.col[0], c1.col[1], c1.col[2], c1.nrows,
                       (!mau_nome && !set_mau) ? "recusada nos dois" : "ACEITE (mau)",
                       set_bom ? "aplicado" : "RECUSADO (mau)");
                printf("     e a letra «a» numa tabela com nomes: %s\n",
                       letra_vale ? "ACEITE (mau)" : "recusada");
                ok("A COLUNA TEM NOME, e ele decide: `CREATE TABLE cliente (nome,idade,saldo)`"
                   " guarda os três, o SELECT devolve-os, o `WHERE idade > 20` corre e o"
                   " `UPDATE ... SET saldo` acerta na coluna certa. Antes a coluna era"
                   " `nome[0] - 'a'`: o SELECT dizia `a`,`b`,`c`, o WHERE por nome era"
                   " recusado e o SET pedia a coluna 18. E o outro lado: uma coluna que não"
                   " existe («altura») é RECUSADA no WHERE e no SET. E a LETRA deixa de"
                   " valer onde há nomes: `WHERE a` nesta tabela é recusado, senão a letra"
                   " fazia o trabalho na mesma e tirar a guarda não derrubava nada. Onde"
                   " NÃO há nomes guardados — as bases antigas, e todos os medidores que"
                   " escrevem (a,b,c) — a letra continua a ser a régua",
                   !strcmp(c1.col[0], "nome") && !strcmp(c1.col[1], "idade")
                   && !strcmp(c1.col[2], "saldo") && c1.nrows == 1
                   && !mau_nome && !set_mau && set_bom && !letra_vale
                   && !strcmp(c4.cell[0][2], "99") && !strcmp(c4.cell[1][2], "50"));
            }
            /* ── O ANDAR SEGUINTE DA TORRE: o transporte ATRAVESSA o átomo ───────
             * A célula segura 0..255 porque a ULA soma componente a componente e o
             * vai-um NÃO passa de um átomo para o outro. Para ela crescer, o par tem
             * de poder ser lido como UM número — e isso é a dobra: o vai-um do átomo
             * baixo entra no alto, que é `thm:transporte` um andar acima.
             *
             * Mede-se o andar SOZINHO, antes de mexer na célula — o caminho novo
             * primeiro, o velho intacto. E o gume é o sítio onde a ULA de baixo pára:
             * 255+1 dá 0 nela e tem de dar 256 aqui. Sem esse caso, uma soma de 16
             * bits que ignorasse o vai-um passava em tudo o que não atravessa. */
            {
                long mau = 0, atravessa = 0, casos = 0;
                /* varre-se o andar todo nos sítios onde o transporte decide */
                const unsigned A[] = { 0, 1, 200, 255, 256, 300, 1000, 32767, 65280, 65535 };
                const unsigned B[] = { 0, 1, 55, 255, 256, 999, 65535, 1, 255, 1 };
                for(unsigned i = 0; i < sizeof A/sizeof *A; i++)
                for(unsigned j = 0; j < sizeof B/sizeof *B; j++){
                    Word8 tr = 0;
                    W16 r = ula_add16(w16_de(A[i]), w16_de(B[j]), &tr);
                    unsigned esperado = (A[i] + B[j]) & 0xFFFFu;
                    casos++;
                    if(w16_val(r) != esperado) mau++;
                    if(tr != ((A[i] + B[j]) > 0xFFFFu)) mau++;
                    /* o transporte atravessou? (o átomo baixo transbordou) */
                    if(((A[i] & 255u) + (B[j] & 255u)) > 255u) atravessa++;
                    /* e a subtracção e a ordem, no mesmo par */
                    Word8 emp = 0;
                    W16 d = ula_sub16(w16_de(A[i]), w16_de(B[j]), &emp);
                    if(w16_val(d) != ((A[i] - B[j]) & 0xFFFFu)) mau++;
                    if((emp != 0) != (A[i] < B[j])) mau++;
                    if(ula_menor16(w16_de(A[i]), w16_de(B[j])) != (A[i] < B[j])) mau++;
                    if(ula_igual16(w16_de(A[i]), w16_de(B[j])) != (A[i] == B[j])) mau++;
                }
                /* O GUME: onde a ULA de BAIXO pára. 255+1 dá 0 nela, 256 aqui. */
                Word8 t8 = ula_soma_w(255, 1);
                Word8 tr = 0;
                W16 t16 = ula_add16(w16_de(255), w16_de(1), &tr);
                printf("\n     16 bits: %ld pares, %ld falhas · o transporte atravessa em %ld"
                       " deles\n     e onde a ULA de 8 pára: 255+1 = %u nela, %u no andar de"
                       " cima\n", casos, mau, atravessa, (unsigned)t8, w16_val(t16));
                ok("O TRANSPORTE ATRAVESSA O ÁTOMO, e é isso que faz de dois átomos UM"
                   " número: `thm:transporte` um andar acima, com o vai-um do baixo a entrar"
                   " como cin do alto — e tudo construído do NAND, sem um `+` do C. A soma, a"
                   " subtracção e a ORDEM batem nos pares varridos, o transbordo do andar"
                   " diz-se em vez de se perder, e o gume é o sítio onde a ULA de baixo pára:"
                   " 255+1 dá 0 nela e 256 aqui. Sem esse caso, uma soma que ignorasse o"
                   " vai-um passava em tudo o que não atravessa",
                   mau == 0 && casos == 100 && atravessa > 10
                   && t8 == 0 && w16_val(t16) == 256 && tr == 0);
            }
            /* ── A DOBRA DO REI: GOLD em dezasseis bits ──────────────────────────
             * A ISA não tem MUL: `n·x` é ZECKENDORF, e cada F(k) é uma potência do
             * rei, GOLD = (a,b) ↦ (a+b, a). O estado do rei É a Word — `a` no
             * `.total`, `b` no `.e` —, e é por isso que o segundo átomo não está
             * livre e que o avaliador ficou em oito bits (§27).
             *
             * A dobra é a do §26 aplicada ao REI em vez de à soma: o estado passa a
             * ser um par de valores de dezasseis bits e o `+` lá dentro é o
             * `ula_add16`. Mede-se SOZINHA, com o caminho velho intacto.
             *
             * Três coisas, e a terceira é a que interessa:
             *   (a) GOLD^k (x,0) dá mesmo F(k+1)·x — o rei é Fibonacci
             *   (b) n·x por Zeckendorf bate com n·x, varrido
             *   (c) E O GUME: onde o rei de OITO bits parte. 3·100 = 300 enrola
             *       para 44 lá, e dá 300 aqui. Sem esse caso, um GOLD de 16 que
             *       ignorasse o transporte passava em tudo o que não atravessa. */
            {
                long mau = 0, casos = 0, largos = 0, acima = 0;
                /* (a) o rei É Fibonacci */
                const unsigned F[] = { 1,1,2,3,5,8,13,21,34,55,89,144,233,377,610,987 };
                for(int k = 1; k <= 15; k++){
                    Word8 tr = 0;
                    W16 r = fib_vezes16(w16_de(7), k, &tr);
                    unsigned esperado = F[k] * 7u;
                    if(esperado <= 0xFFFFu && (w16_val(r) != esperado || tr)) mau++;
                }
                /* (b) n·x por Zeckendorf, varrido onde o transporte decide */
                /* o regime TEM de conter o defeito: com n ≤ 40 e x ≤ 300 o produto
                 * nunca passa de 12000, logo o ramo do transbordo NUNCA CORRIA e a
                 * bandeira do andar ficava por medir — tirá-la não derrubava nada.
                 * Agora vai até 300·300 = 90000, bem acima do tecto de 65535. */
                for(unsigned n = 0; n <= 300; n += 7)
                for(unsigned x = 0; x <= 300; x += 37){
                    Word8 tr = 0;
                    W16 r = mul16_zeck(n, w16_de(x), &tr);
                    unsigned esperado = n * x;
                    casos++;
                    if(esperado > 255u) largos++;
                    if(esperado <= 0xFFFFu){ if(w16_val(r) != esperado || tr) mau++; }
                    else { acima++; if(!tr) mau++; }    /* acima do andar, TEM de dizer */
                }
                /* (c) o gume: onde o rei de oito bits parte */
                Word w8est = { 100, 0 };                 /* (x, 0) em oito bits */
                for(int t = 0; t < 3; t++) w8est = cifra_an(w8est, 1);   /* GOLD³ → 3·x */
                Word8 tr16 = 0;
                W16 r16 = mul16_zeck(3, w16_de(100), &tr16);
                printf("\n     GOLD16: %ld produtos (%ld acima de 255, %ld acima de 65535),"
                       " %ld falhas\n"
                       "     e onde o rei de 8 parte: 3·100 = %u nele, %u no andar de cima\n",
                       casos, largos, acima, mau, (unsigned)w8est.total, w16_val(r16));
                ok("A DOBRA DO REI: GOLD em dezasseis bits. A ISA não tem MUL — `n·x` é"
                   " ZECKENDORF, todo inteiro é soma de Fibonacci não consecutivos e cada"
                   " F(k) é uma potência do rei, GOLD = (a,b) ↦ (a+b, a). O estado do rei É"
                   " a Word, e é por isso que o segundo átomo não está livre e o avaliador"
                   " ficou em oito bits. Aqui o estado passa a um PAR de valores de 16 e o"
                   " `+` lá dentro é o `ula_add16`, onde o vai-um atravessa: GOLD^k (x,0) dá"
                   " F(k+1)·x nos quinze primeiros, e n·x bate no varrimento — com o que"
                   " passa do andar DITO em vez de enrolado — e o varrimento SOBE ao"
                   " tecto de propósito, até 300·300 = 90000, senão o ramo do transbordo"
                   " nunca corria e a bandeira ficava por medir. E o gume é o sítio onde o"
                   " rei de oito parte: 3·100 dá 44 nele e 300 aqui",
                   /* `acima > 0` e não um número escolhido: o que se afirma é que o
                    * ramo do transbordo É VISITADO e que TODOS os que lá caem o dizem
                    * (o `mau` conta os que não dissessem). Pôr aqui o 17 que hoje sai
                    * era escrever o resultado da medição dentro dela. */
                   mau == 0 && casos == 387 && largos > 100 && acima > 0
                   && w8est.total == 44 && w16_val(r16) == 300 && tr16 == 0);
            }
            /* ── E A BASE ANTIGA CONTINUA A ANDAR ────────────────────────────────
             * A letra é o RECURSO, e um recurso que nunca corre não é compatibilidade:
             * é uma linha que ninguém mediu. Todas as tabelas deste ficheiro nascem
             * com nomes guardados, logo o ramo da letra nunca era visitado — tirá-lo
             * não derrubava nada.
             *
             * Uma base escrita antes desta mudança tem os slots do nome A ZERO, e é
             * isso que se reproduz aqui: zeram-se, e o `WHERE a > 20` — que numa
             * tabela com nomes é recusado — tem de voltar a valer. */
            {
                SqlOut o1, o2;
                executa("CREATE TABLE antiga (nome,idade)");
                executa("INSERT INTO antiga VALUES (1,30)");
                executa("INSERT INTO antiga VALUES (2,17)");
                sql_cap = &o1; memset(&o1, 0, sizeof o1);
                int com_letra = executa("SELECT * FROM antiga WHERE a > 0");
                sql_cap = NULL;
                { Word z = {0,0}; unsigned i;      /* apaga os nomes: fica como uma base antiga */
                  for(i = 0; i < S_COLNOME_N * S_COLNOME_W; i++) mem_grava(S_COLNOME + i, z); }
                sql_cap = &o2; memset(&o2, 0, sizeof o2);
                int sem_nomes = executa("SELECT * FROM antiga WHERE b > 20");
                sql_cap = NULL;
                printf("     com nomes: «a» %s · nomes apagados (base antiga): «b» %s,"
                       " %d linha(s), colunas [%s][%s]\n",
                       com_letra ? "ACEITE (mau)" : "recusada",
                       sem_nomes ? "vale" : "RECUSADA (mau)", o2.nrows, o2.col[0], o2.col[1]);
                ok("A BASE ANTIGA CONTINUA A ANDAR, e isso mede-se em vez de se prometer:"
                   " apagados os slots do nome — que é exactamente o que uma base escrita"
                   " antes desta mudança tem —, a LETRA volta a ser a régua, o `WHERE b > 20`"
                   " corre, e o SELECT devolve `a`,`b`. Com os nomes lá, a mesma letra é"
                   " recusada. São os dois lados do mesmo recurso, e sem o segundo o ramo"
                   " da compatibilidade nunca era visitado por medidor nenhum",
                   !com_letra && sem_nomes && o2.nrows == 1
                   && !strcmp(o2.col[0], "a") && !strcmp(o2.col[1], "b"));
            }
            /* ── E A TABELA VAZIA TEM COLUNAS ─────────────────────────────────── */
            {
                SqlOut v1, v2;
                executa("CREATE TABLE vazia (alfa,beta)");
                sql_cap = &v1; memset(&v1, 0, sizeof v1);
                executa("SELECT * FROM vazia");
                sql_cap = NULL;                       /* senão o INSERT escreve por cima de v1 */
                executa("INSERT INTO vazia VALUES (1,2)");
                sql_cap = &v2; memset(&v2, 0, sizeof v2);
                executa("SELECT * FROM vazia WHERE alfa = 9");
                sql_cap = NULL;
                printf("     vazia: sem linhas %dx%d tag=[%s] · com WHERE sem casar %dx%d"
                       " tag=[%s]\n", v1.nrows, v1.ncols, v1.tag, v2.nrows, v2.ncols, v2.tag);
                ok("UMA TABELA VAZIA TAMBÉM TEM COLUNAS: um SELECT que não devolve linhas"
                   " devolve na mesma a DESCRIÇÃO delas e o «SELECT 0» que fecha o ciclo."
                   " A saída antecipada do caso «ainda não há uma linha» devolvia ncols = 0"
                   " e tag vazia — pela porta FEBE o driver ficava sem RowDescription e sem"
                   " CommandComplete, e não distinguia «está vazia» de «a resposta"
                   " perdeu-se». Os DOIS caminhos até zero linhas (tabela vazia e WHERE que"
                   " não casa) têm de dizer o mesmo, e agora dizem",
                   v1.ncols == 2 && v1.nrows == 0 && !strcmp(v1.tag, "SELECT 0")
                   && !strcmp(v1.col[0], "alfa") && !strcmp(v1.col[1], "beta")
                   && v2.ncols == v1.ncols && v2.nrows == 0
                   && !strcmp(v2.tag, v1.tag) && !strcmp(v2.col[0], v1.col[0]));
            }
            executa("CREATE TABLE t (a,b,c)");     /* repõe a tabela do resto do teste */
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 2: a classe vem do TOOLKIT, e a saída despacha pelo corpo declarado. */
        printf("\n-- O RACIONAL PELO TOOLKIT (passo 2 de 6)\n\n");
        {
            executa("CREATE TABLE k (a RACIONAL, b)");
            executa("INSERT INTO k VALUES (6/8,1)");
            executa("INSERT INTO k VALUES (-2/6,2)");
            executa("INSERT INTO k VALUES (5,3)");
            Word c0 = mem_le(S_LINHAS + 0), c1 = mem_le(S_LINHAS + 2), c2 = mem_le(S_LINHAS + 4);
            ok("6/8 entra reduzido a 3/4 — ra_classe do corpos.h",  c0.total == 3 && c0.e == 4);
            ok("-2/6 vira -1/3, com o sinal no numerador",          (int8_t)c1.total == -1 && c1.e == 3);
            ok("e o inteiro fica inteiro, denominador 1",           c2.total == 5 && c2.e == 1);
            Word cp = mem_le(S_CORPO + 0);
            ok("a saída despacha pelo corpo declarado da coluna",   cp.total == CORPO_RACIONAL);
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 3: o áureo. O par é o mesmo; o que muda é o que ele SIGNIFICA. */
        printf("\n-- O ÁUREO ℤ[φ] (passo 3 de 6)\n\n");
        {
            executa("CREATE TABLE k (a AUREO(1), b AUREO(2))");
            executa("INSERT INTO k VALUES (3+2s,1+1s)");
            executa("INSERT INTO k VALUES (5,0-1s)");
            Word x = mem_le(S_LINHAS + 0), y = mem_le(S_LINHAS + 2);
            ok("3+2s guarda o par (3,2) — a + bσ",  x.total == 3 && x.e == 2);
            ok("e 5 sozinho é 5, não 5+σ: o padrão vem do CORPO", y.total == 5 && y.e == 0);
            Word cm = mem_le(S_CORPO + 1);
            ok("AUREO(2) leva o metal na coluna — a borda é dele", cm.e == 2);
            /* o invariante do corpo, medido sobre o que está GUARDADO: a norma é
             * multiplicativa, e é ela que o áureo conserva (familia_real.c §F1). */
            Par p = { x.total, x.e }, q = { mem_le(S_LINHAS+1).total, mem_le(S_LINHAS+1).e };
            long m = 1;
            ok("e a NORMA é multiplicativa no que foi guardado",
               au_norma(au_prod(p, q, m), m) == au_norma(p, m) * au_norma(q, m));
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 4: o mórfico, por DESCOBERTA — ele já operava no WHERE. */
        printf("\n-- O MÓRFICO (passo 4 de 6, por descoberta)\n\n");
        {
            executa("CREATE TABLE k (a MORFICO(6), b MORFICO(4))");
            executa("INSERT INTO k VALUES (13,3)");
            executa("INSERT INTO k VALUES (63,0)");
            Word x = mem_le(S_LINHAS + 0), y = mem_le(S_LINHAS + 2);
            ok("13 guarda a máscara — e {0,2,3} é o mesmo objeto", x.total == 13);
            ok("o topo e o vazio também: 63 e 0",                  y.total == 63);
            Word cn = mem_le(S_CORPO + 0);
            ok("MORFICO(6) leva o n na coluna — o universo é dele", cn.e == 6);
            /* O INVARIANTE que distingue este corpo de todos os outros: TODO elemento é
             * IDEMPOTENTE, A ∧ A = A. É por isso que ele só é corpo quando n = 1 — com n > 1
             * há divisor de zero e elemento sem inverso (morfico.py, teo:socorpon1). */
            unsigned A = (unsigned)x.total, B = (unsigned)y.total;
            int idem = 1;
            for(unsigned t = 0; t < 64; t++) if(mo_prod(t,t) != t) idem = 0;
            /* `t & t == t` é identidade booleana para todo unsigned, e continua verdade com o produto
             * trocado por OU, por min, ou por qualquer operação idempotente. NÃO é a marca deste
             * corpo — é a marca de uma classe inteira de operações. Um revisor apanhou-o. */
            conclui("todo elemento é idempotente: A ∧ A = A — mas isso vale para toda operação");
            conclui("idempotente, e por isso não distingue este corpo de nenhum outro.");
            (void)idem;
            /* `A & B & ~A == 0` é identicamente zero em álgebra de Boole: os valores de A e B eram
             * irrelevantes. O que a erosão tem de próprio é ENCOLHER de facto — existir A,B com
             * A∧B estritamente contido em A —, e isso pode falhar. */
            {
                int encolhe = 0, testados = 0;
                for(unsigned u=1; u<64; u++) for(unsigned v=1; v<64; v++){
                    testados++;
                    if(mo_prod(u,v) != u) encolhe++;      /* estritamente menor */
                }
                printf("      pares (A,B) em que A ∧ B é ESTRITAMENTE menor que A: %d de %d\n",
                       encolhe, testados);
                ok("a erosão ENCOLHE de facto — e não é a identidade disfarçada", encolhe > testados/2);
            }
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 6: o CRISTALINO — o lado que gira entra no catálogo. */
        printf("\n-- O CRISTALINO (passo 6 de 6): o lado que gira\n\n");
        {
            executa("CREATE TABLE k (a CRISTALINO(0), b CRISTALINO(1), c AUREO(1))");
            executa("INSERT INTO k VALUES (3+2s, 1+1s, 3+2s)");
            Word x = mem_le(S_LINHAS + 0), y = mem_le(S_LINHAS + 1);
            ok("3+2ω entra no cristal como par — o MESMO par do áureo",
               x.total == 3 && x.e == 2);
            Word cg = mem_le(S_CORPO + 0), ce = mem_le(S_CORPO + 1);
            ok("CRISTALINO(0) é Gauss ℤ[i] — o t da borda viaja na coluna",
               cg.total == CORPO_CRISTAL && cg.e == 0);
            ok("CRISTALINO(1) é Eisenstein ℤ[ω], o Φ₆ do trono",
               ce.total == CORPO_CRISTAL && ce.e == 1);
            /* O par é o mesmo; o que muda é A BORDA, e dela sai tudo. Afirma-se o INVARIANTE,
             * não só o armazenamento: no cristal a norma é multiplicativa E positiva, e o
             * operador tem ordem FINITA — ao contrário do áureo guardado ao lado. */
            Par par_u = { x.total, x.e }, par_v = { y.total, y.e };
            ok("a norma do cristal é multiplicativa no que foi guardado",
               cr_norma(cr_prod(par_u,par_v,0),0) == cr_norma(par_u,0) * cr_norma(par_v,0));
            ok("e é POSITIVA — o áureo ao lado alterna de sinal",
               cr_norma(par_u,0) > 0 && cr_norma(par_v,1) > 0);
            /* a ordem finita, contada no metal: ×ω volta ao ponto de partida */
            Par g = par_u; int ordem = 0;
            for(int t = 1; t <= 12; t++){ g = cr_op(g,0); if(g.a==par_u.a && g.b==par_u.b){ ordem = t; break; } }
            ok("×ω em Gauss tem ordem 4 — gira e VOLTA, o que o gato nunca faz", ordem == 4);
            Par e6 = par_v; int o6 = 0;
            for(int t = 1; t <= 12; t++){ e6 = cr_op(e6,1); if(e6.a==par_v.a && e6.b==par_v.b){ o6 = t; break; } }
            ok("×ω em Eisenstein tem ordem 6 — o Φ₆, sentado no trono", o6 == 6);
            /* e a volta do gato, agora no toolkit: a antípoda conjugada pela involução */
            Mat A = me_gato(2), Ai = me_antigato(2), J = me_troca();
            Mat id = me_prod(A, Ai);
            ok("a volta do gato é INTEIRA: A·A⁻¹ = I sem sair de ℤ",
               id.a==1 && id.b==0 && id.c==0 && id.d==1);
            Mat conj = me_prod(J, me_prod(me_gato(-2), J));
            ok("e A⁻¹ É J·A_{−m}·J — a mesma peça virada, não uma segunda máquina",
               conj.a==Ai.a && conj.b==Ai.b && conj.c==Ai.c && conj.d==Ai.d);
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 5, a PRIMEIRA PEDRA: a máquina a aplicar uma MATRIZ como opcodes.
         *
         * mecanica.c mediu que toda matriz de det ±1 é palavra nos geradores. Aqui verifica-se
         * no METAL: emite-se a palavra, a máquina corre, e compara-se com o que a matriz daria.
         *
         * E o gerador da ISA não é o cisalhamento — é o GATO. cifra_an(w,m) = (m·total + e,
         * total) É A_m aplicado ao par, e é um opcode: GOLD, SILVER, BRONZE. Aplicar A_m^k é
         * repetir o opcode k vezes, sem multiplicação nenhuma.
         *
         * Isto NÃO troca ainda a emissão do WHERE — é a pedra, não a parede. Trocar a parede
         * é mexer no emit_atomos, e hoje já mostrei três vezes o que acontece quando faço isso
         * com pressa. */
        printf("\n-- A MATRIZ COMO OPCODES (passo 5, primeira pedra)\n\n");
        {
            int mau = 0;
            printf("      m   k   par de entrada   pela máquina   pela matriz   iguais?\n");
            for(long m = 1; m <= 3; m++) for(int k = 1; k <= 6; k++){
                Word v; v.total = 3; v.e = 2;
                mem_grava(S_TMP, v);
                pc_emit = 0;
                for(int t = 0; t < k; t++)           /* a PALAVRA: k metais, cada um palavra */
                    emit_metal(m, S_TMP);
                emit1(OP_HALT);
                rodar(pc_emit);
                Word saiu = mem_le(S_TMP);
                /* e a matriz, pelo toolkit: A_m^k aplicado ao mesmo par */
                Mat A = me_gato(m), P = {1,0,0,1};
                for(int t = 0; t < k; t++) P = me_prod(A, P);
                Par esperado = me_ap(P, (Par){3,2});
                if(saiu.total != (Word8)esperado.a || saiu.e != (Word8)esperado.b) mau++;
                if((m==1&&k<=2)||(m==3&&k==6))
                    printf("      %ld   %d   (3,2)%*s(%d,%d)%*s(%ld,%ld)%*s%s\n", m, k,
                           12, "", saiu.total, saiu.e, 8, "", esperado.a, esperado.b, 6, "",
                           (saiu.total==(Word8)esperado.a && saiu.e==(Word8)esperado.b) ? "sim ✓" : "NÃO");
            }
            ok("a máquina aplicando a PALAVRA dá o que a matriz daria", mau == 0);
            ok("e cada letra é UM opcode: GOLD/SILVER/BRONZE, sem multiplicação", mau == 0);
        }

        /* PASSO 5, segunda pedra: A CADEIA DE MINERAIS, e a volta pelo negro.
         *
         * Correção do Aarão, e ela desmonta o mecanica.c num ponto: eu decompus em T, o
         * cisalhamento — e T NÃO É OPCODE. Os opcodes são a CADEIA DE MINERAIS: GOLD, SILVER,
         * BRONZE são A_1, A_2, A_3, e é neles que a palavra tem de ser escrita.
         *
         * E a volta é PELO NEGRO: det(A_m) = −1, logo a inversa é INTEIRA, e desfazer a ida é
         * aplicar as inversas na ordem contrária. Ir e voltar fecha na identidade, exatamente
         * — é isso que faz o percurso reversível, e é o esquilo (det +1) a fechá-lo.
         *
         * Aqui mede-se no METAL: uma cadeia qualquer de minerais, a ida pela máquina, a volta
         * pela máquina, e o par tem de voltar ao que era. */
        printf("\n-- A CADEIA DE MINERAIS, E A VOLTA PELO NEGRO (passo 5, segunda pedra)\n\n");
        {
            int mau = 0; long casos = 0;
            printf("      cadeia          ida            volta        fecha?\n");
            int cadeias[6][4] = {{1,0,0,0},{1,2,0,0},{1,2,3,0},{3,1,2,0},{2,2,2,2},{1,1,1,1}};
            int comps[6] = {1,2,3,3,4,4};
            for(int t = 0; t < 6; t++){
                Word v; v.total = 5; v.e = 3;
                mem_grava(S_TMP, v);
                /* A IDA: a cadeia de minerais, um opcode por elo */
                pc_emit = 0;
                for(int e = 0; e < comps[t]; e++) emit_metal(cadeias[t][e], S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word meio = mem_le(S_TMP);
                /* A VOLTA PELO NEGRO, NO METAL. A_m⁻¹ = [[0,1],[1,−m]] é inteira porque
                 * det = −1, e a palavra dela é a da ida ao contrário com as letras invertidas.
                 * A cadeia desfaz-se elo a elo, sem uma chamada C no caminho. */
                pc_emit = 0;
                for(int e = comps[t]-1; e >= 0; e--) emit_metal_inv(cadeias[t][e], S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word volta = mem_le(S_TMP);
                if(volta.total != 5 || volta.e != 3) mau++;
                /* e o toolkit CONFERE, não executa: a conta do metal tem de dar a mesma coisa */
                Par p = { meio.total, meio.e };
                for(int e = comps[t]-1; e >= 0; e--){
                    long m = cadeias[t][e];
                    Mat inv = me_antigato(m);
                    p = me_ap(inv, p);
                }
                if((Word8)p.a != volta.total || (Word8)p.b != volta.e) mau++;
                casos++;
                if(t == 0 || t == 4)
                    printf("      %-15s (%d,%d)%*s(%d,%d)%*s%s\n",
                           t==0?"ouro":"prata⁴", meio.total, meio.e, 8, "", volta.total, volta.e, 6, "",
                           (volta.total==5&&volta.e==3) ? "sim ✓" : "NÃO");
            }
            ok("a cadeia vai e VOLTA PELO NEGRO — e a volta é OPCODE, não toolkit", mau == 0);
            ok("e fecha porque det = −1: a inversa é INTEIRA, não é reconstrução", mau == 0);
            ok("o toolkit confere a máquina e concorda — mas quem executa é o metal", mau == 0);
            printf("      (%ld cadeias, até quatro elos, misturando ouro, prata e bronze.)\n", casos);
            printf("\n      O percurso é agora INTEIRO no metal: ida e volta são opcodes, e o toolkit\n");
            printf("      passou de executor a testemunha. O que destravou isto foi saber o que a\n");
            printf("      inversa É — a antípoda (m ↦ −m) conjugada pela involução J — em vez de a\n");
            printf("      tratar como uma segunda máquina que a ISA teria de aprender do zero.\n");
        }

        /* O OPCODE DA INVERSA, medido sozinho: sem cadeia, sem tabela, só a peça. */
        printf("\n-- O OPCODE DA INVERSA (passo 5, terceira pedra): a volta no metal\n\n");
        {
            int mau = 0; long casos = 0;
            printf("      metal    opcode          par     ida        volta      desfaz?\n");
            const char *nm[3] = {"ouro","prata","bronze"};
            for(int k = 0; k < 3; k++)
            for(long a = -7; a <= 7; a++) for(long b = -7; b <= 7; b++){
                Word v; v.total = a; v.e = b;
                mem_grava(S_TMP, v);
                pc_emit = 0; emit_metal(k+1, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word ida = mem_le(S_TMP);
                pc_emit = 0; emit_metal_inv(k+1, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word vt = mem_le(S_TMP);
                if(vt.total != (Word8)a || vt.e != (Word8)b) mau++;          /* desfaz no envelope */
                /* e a ORDEM não importa: aplicar a inversa primeiro também fecha */
                mem_grava(S_TMP, v);
                pc_emit = 0; emit_metal_inv(k+1, S_TMP); emit_metal(k+1, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word ot = mem_le(S_TMP);
                if(ot.total != (Word8)a || ot.e != (Word8)b) mau++;
                if(a == 5 && b == 3)
                    printf("      %-8s %-15s (5,3)   (%d,%d)%*s(%d,%d)%*s%s\n",
                           nm[k], k==0?"NEGRO":(k==1?"NEGRO TROCA NEGRO":"NEGRO TROCA NEGRO×2"),
                           ida.total, ida.e, 5, "", vt.total, vt.e, 5, "",
                           (vt.total==a&&vt.e==b) ? "sim ✓" : "NÃO");
                casos++;
            }
            ok("o opcode negro desfaz o metal EXATO, nos dois sentidos e sem divisão", mau == 0);
            printf("      (%ld pares, três metais.)\n", casos);
            /* e o que ele É: a antípoda conjugada pela involução, conferido contra o toolkit */
            int idm = 0;
            for(long m = 1; m <= 3; m++){
                Mat J = me_troca();
                Mat conj = me_prod(J, me_prod(me_gato(-m), J));
                Mat ai = me_antigato(m);
                if(conj.a!=ai.a||conj.b!=ai.b||conj.c!=ai.c||conj.d!=ai.d) idm++;
            }
            ok("e o que o opcode É: J·A_{−m}·J — a mesma peça virada", idm == 0);
            printf("\n      Um opcode que precisasse de divisão não caberia nesta máquina. Este não\n");
            printf("      precisa: (a,b) ↦ (b, a − m·b), tudo em inteiros, porque det A_m = −1. A\n");
            printf("      reversibilidade não foi acrescentada à ISA — ela já estava no determinante,\n");
            printf("      e só faltava escrevê-la.\n");
        }

        /* A TOPOLOGIA NO SQL: a distância entre os corpos das colunas. */
        printf("\n-- A TOPOLOGIA NA QUERY: distância entre os corpos das colunas\n\n");
        {
            executa("CREATE TABLE k (a AUREO(1), b AUREO(3), c CRISTALINO(0), d RACIONAL)");
            printf("$ DISTANCIA\n\n");
            executa("DISTANCIA");
            /* AUREO(m) tem Δ = m²+4; CRISTALINO(t) tem Δ = t²−4 */
            ok("AUREO(1) tem Δ = 5 — a régua (1,−1), hiperbólica",
               corpo_delta(CORPO_AUREO, 1) == 5);
            ok("AUREO(3) tem Δ = 13, e a distância a AUREO(1) é 8",
               corpo_delta(CORPO_AUREO, 3) == 13 &&
               corpo_delta(CORPO_AUREO,3) - corpo_delta(CORPO_AUREO,1) == 8);
            ok("CRISTALINO(0) tem Δ = −4 — Gauss, elíptica",
               corpo_delta(CORPO_CRISTAL, 0) == -4);
            ok("o RACIONAL não tem régua quadrática, e sai marcado — não se inventa Δ",
               !corpo_tem_regua(CORPO_RACIONAL));
            /* a métrica: simétrica e triangular, nos corpos que a tabela tem */
            long D1 = corpo_delta(CORPO_AUREO,1), D2 = corpo_delta(CORPO_AUREO,3),
                 D3 = corpo_delta(CORPO_CRISTAL,0);
            long d12 = D1>D2?D1-D2:D2-D1, d23 = D2>D3?D2-D3:D3-D2, d13 = D1>D3?D1-D3:D3-D1;
            ok("a distância é simétrica e triangular nas colunas desta tabela",
               d13 <= d12 + d23);
        }

        /* E o caso que interessa: DUAS COLUNAS ISOMORFAS, e o transporte entre elas. */
        printf("\n-- DUAS COLUNAS ISOMORFAS: distância ZERO, e o transporte em bytecode\n\n");
        {
            /* AUREO(1) e AUREO(−1) têm o MESMO Δ = 5: são o mesmo corpo, escrito diferente */
            executa("CREATE TABLE k (a AUREO(1), b AUREO(-1))");
            printf("$ DISTANCIA\n\n");
            executa("DISTANCIA");
            ok("AUREO(1) e AUREO(−1) têm o mesmo Δ = 5 — distância ZERO",
               corpo_delta(CORPO_AUREO,1) == corpo_delta(CORPO_AUREO,-1));
            /* e o transporte é φ_t com t = (B₂−B₁)/2 = −1, que é o cisalhamento */
            long t = (corpo_B(CORPO_AUREO,-1) - corpo_B(CORPO_AUREO,1)) / 2;
            ok("o transporte entre elas é t = −1, e é único", t == -1);
            /* CONFERIDO NO METAL: o cisalhamento roda como palavra e faz o transporte */
            Word v; v.total = 5; v.e = 3;
            mem_grava(S_TMP, v);
            pc_emit = 0;
            MOVE(S_TMP, +1); emit1(OP_NEGRO_OURO); MOVE(S_TMP, -1);
            MOVE(S_TMP, +1); emit1(OP_TROCA);      MOVE(S_TMP, -1);
            emit1(OP_HALT); rodar(pc_emit);
            Word w = mem_le(S_TMP);
            /* T⁻¹ = [[1,−1],[0,1]] em (5,3) dá (5−3, 3) = (2,3) */
            ok("e a máquina executa-o: (5,3) vai em (2,3) por NEGRO TROCA — é φ_{−1}",
               w.total == 2 && w.e == 3);
            printf("      A query descobre que duas colunas são o MESMO corpo e diz COMO ir de uma\n");
            printf("      à outra — em opcodes, não em fórmula. O parabólico, que não precisava de\n");
            printf("      opcode por ser palavra de duas, é exatamente quem faz o transporte.\n");
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* A DISTÂNCIA NO WHERE: filtrar por corpo isomorfo, e recusar fora da classe. */
        printf("\n-- A DISTÂNCIA NO WHERE: só se compara dentro da classe de isomorfismo\n\n");
        {
            /* duas colunas em CLASSES DIFERENTES: Δ = 5 e Δ = −4, distância 9 */
            executa("CREATE TABLE k (a AUREO(1), b CRISTALINO(0))");
            executa("INSERT INTO k VALUES (3+2s, 1+1s)");
            printf("$ SELECT * FROM k WHERE a - b > 0\n");
            int r = executa("SELECT * FROM k WHERE a - b > 0");
            ok("a consulta que atravessa classes diferentes é RECUSADA, e nada é devolvido",
               r == 0);
            printf("\n");
            /* a mesma coluna, sozinha: nada a atravessar, e passa */
            printf("$ SELECT * FROM k WHERE a > 0\n");
            int r2 = executa("SELECT * FROM k WHERE a > 0");
            ok("mas a consulta dentro de UMA coluna passa — não há nada a atravessar", r2 == 1);
        }
        printf("\n-- E QUANDO SÃO ISOMORFOS: passa, e o transporte é dito\n\n");
        {
            /* Δ = 5 nas duas, bases diferentes (B = 1 e B = −1): isomorfos, com transporte */
            executa("CREATE TABLE k (a AUREO(1), b AUREO(-1))");
            executa("INSERT INTO k VALUES (7+0s, 3+0s)");
            executa("INSERT INTO k VALUES (2+0s, 9+0s)");
            printf("$ SELECT * FROM k WHERE a - b > 0\n");
            int r = executa("SELECT * FROM k WHERE a - b > 0");
            ok("isomorfas em bases diferentes: PASSA, e o transporte é emitido", r == 1);
            /* e o transporte tem de estar MESMO no bytecode: sem ele o valor de b entraria
             * cru. Confere-se aplicando φ_t à mão e comparando com o que a máquina faria. */
            {
                Word v; v.total = 3; v.e = 0;
                mem_grava(S_TMP, v);
                long t = (corpo_B(CORPO_AUREO,1) - corpo_B(CORPO_AUREO,-1)) / 2;
                pc_emit = 0; emit_transporte(t, S_TMP); emit1(OP_HALT); rodar(pc_emit);
                Word w = mem_le(S_TMP);
                Par esperado = me_ap(me_cis(t), (Par){3,0});
                ok("φ_t emitido concorda com a matriz [[1,t],[0,1]] — o transporte é o certo",
                   w.total == esperado.a && w.e == esperado.b);
            }
            ok("e o transporte é o único que existe: t = (B₁−B₂)/2 = 1",
               (corpo_B(CORPO_AUREO,1) - corpo_B(CORPO_AUREO,-1)) / 2 == 1);
            /* e a MESMA base passa: nada a transportar */
            executa("CREATE TABLE k (a AUREO(1), b AUREO(1))");
            executa("INSERT INTO k VALUES (7+0s, 3+0s)");
            executa("INSERT INTO k VALUES (2+0s, 9+0s)");
            printf("\n$ SELECT * FROM k WHERE a - b > 0\n");
            int r3 = executa("SELECT * FROM k WHERE a - b > 0");
            /* PASSA A GUARDA — e é só isso que se afirma. A comparação em si, dentro de uma
             * coluna áurea, ainda NÃO é a certa: o caminho do átomo trata o .e como DENOMINADOR
             * (é o racional), e no áureo o .e é a parte σ. Por isso 7−3 > 0 não casa aqui. Isso
             * é outra camada, e fica dito em vez de escondido atrás de um rótulo. */
            ok("mesma classe E mesma base: a guarda DEIXA PASSAR (é só isso que se afirma)",
               r3 == 1);
            /* CONFERIDO NO METAL: emit_transporte executa φ_t */
            Word v; v.total = 5; v.e = 3;
            mem_grava(S_TMP, v);
            pc_emit = 0; emit_transporte(1, S_TMP); emit1(OP_HALT); rodar(pc_emit);
            Word w = mem_le(S_TMP);
            ok("emit_transporte(1) faz (5,3) ↦ (8,3) — é φ_1, o cisalhamento",
               w.total == 8 && w.e == 3);
            mem_grava(S_TMP, v);
            pc_emit = 0; emit_transporte(-1, S_TMP); emit1(OP_HALT); rodar(pc_emit);
            Word u = mem_le(S_TMP);
            ok("e emit_transporte(−1) faz (5,3) ↦ (2,3) — φ_{−1}, e desfaz o outro",
               u.total == 2 && u.e == 3);
            printf("\n      A regra é a mesma do WHERE não entendido: RECUSAR em vez de devolver um\n");
            printf("      número sem significado. Agora a distância decide, e ela é medida.\n");
            printf("\n      E as três respostas, agora que o transporte está LIGADO:\n");
            printf("        distância > 0            classes diferentes → RECUSA\n");
            printf("        distância 0, base ≠      isomorfos → PASSA, com φ_t emitido no átomo\n");
            printf("        distância 0, base =      nada a transportar → passa\n");
            printf("\n      O que destravou foi a ORDEM: φ_t age sobre a Word inteira, e a máscara\n");
            printf("      mata o .e. Transportar DEPOIS de mascarar seria aplicar φ_t a b = 0 —\n");
            printf("      a identidade, e eu não veria diferença nenhuma. Transporta-se primeiro.\n");
            printf("\n      E O QUE CONTINUA ABERTO, dito: a consulta acima devolve 0 linhas onde\n");
            printf("      7−3 > 0 devia casar. O transporte está LIGADO e confere com a matriz —\n");
            printf("      o bytecode cresceu de 743 para 797 e φ_t bate. O que ainda não está é a\n");
            printf("      COMPARAÇÃO dentro de um corpo quadrático: o caminho do átomo trata o .e\n");
            printf("      como DENOMINADOR (foi escrito para o racional), e comparar a+bσ com c+dσ\n");
            printf("      precisa da NORMA. São dois itens distintos, e só um fechou hoje.\n");
            printf("\n      Eu tinha escrito \"transporte emitido\" no caso do meio, e a consulta\n");
            printf("      devolveu 0 linhas onde 7−3 > 0 devia casar. A nota era falsa: emit_transporte\n");
            printf("      existe e roda (medido acima), mas não está no caminho do átomo. Recusar é a\n");
            printf("      resposta honesta enquanto não estiver.\n");
            printf("\n      E o terceiro caso apanhou-me DE NOVO, no mesmo dia e na mesma feature: eu\n");
            printf("      rotulei \"passa, e devolve a linha que casa\", e ele passa mas NÃO devolve.\n");
            printf("      O motivo é outra camada: o caminho do átomo trata o .e como DENOMINADOR,\n");
            printf("      porque foi escrito para o racional — e no áureo o .e é a parte σ. Comparar\n");
            printf("      dentro de um corpo quadrático precisa da NORMA, e a norma não está no\n");
            printf("      emit_atomos.\n");
            printf("\n      Então o que esta guarda faz, dito com precisão: ela decide se a comparação\n");
            printf("      é PERMITIDA. Não a torna correta. São duas coisas, e eu ia entregá-las\n");
            printf("      como uma.\n");
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* A DISTÂNCIA ENTRE TEXTOS: a query simples. */
        printf("\n-- A QUERY SIMPLES: distância entre dois textos\n\n");
        {
            printf("$ DISTANCIA TEXTO 'ouro' 'ouro'\n");
            int r1 = executa("DISTANCIA TEXTO 'ouro' 'ouro'");
            printf("\n$ DISTANCIA TEXTO 'ouro' 'ourz'\n");
            int r2 = executa("DISTANCIA TEXTO 'ouro' 'ourz'");
            printf("\n$ DISTANCIA TEXTO 'ouro' 'prata'\n");
            int r3 = executa("DISTANCIA TEXTO 'ouro' 'prata'");
            ok("a query corre nos três casos — iguais, próximos, distantes", r1 && r2 && r3);
            printf("\n      O texto entra pela mesma porta dos números: vira cifra, e a cifra é um\n");
            printf("      ponto do corpo métrico. Não foi preciso régua nova.\n");
        }

        /* TODA ENTRADA CIFRADA, E O ÍNDICE QUE É A PRÓPRIA POSIÇÃO. */
        printf("\n-- TODA ENTRADA ENTRA CIFRADA: senao nao ha como comparar gato com cachorro\n\n");
        {
            Word z = {0,0}; mem_grava(S_TXCAB, z); mem_grava(S_NOCAB, z); mem_grava(S_TXLIVRE, z);
            executa("INSERT TEXTO 'ouro'");
            executa("INSERT TEXTO 'ourives'");
            executa("INSERT TEXTO 'prata'");
            executa("INSERT TEXTO 'ourico'");
            executa("INSERT TEXTO 7/3");
            executa("INSERT TEXTO 22/7");
            printf("\n      Texto e numero entraram pela MESMA porta e sairam na MESMA\n");
            printf("      representacao. Nao ha duas tabelas, nao ha dois indices, nao ha duas\n");
            printf("      reguas: ha uma cifra.\n");
            ok("seis entradas, texto e racional, no mesmo espaco", txt_n() == 6);

            printf("\n$ BUSCA TEXTO 'ourivesaria'\n\n");
            int r = executa("BUSCA TEXTO 'ourivesaria'");
            ok("a varredura compara cifra com cifra", r == 1);
            printf("\n$ BUSCA TEXTO 7/2      -- um numero medido contra palavras\n\n");
            executa("BUSCA TEXTO 7/2");
            printf("\n      7/2 = [3;2] e 7/3 = [2;3]: divergem no primeiro termo, distancia 1.\n");
            printf("      22/7 = [3;7] partilha o 3 com 7/2: distancia 1/2. O gato e o cachorro\n");
            printf("      ficaram comparados, e quem os comparou foi a regua, nao eu.\n");

            printf("\n-- O INDICE E A PROPRIA POSICAO: a cifra do rei poe cada um no seu lugar\n\n");
            n_leituras = 0;
            executa("ACHA TEXTO 'ourives'");
            ok("acha descendo a cifra, um no por termo", n_leituras == 12);
            printf("\n");
            executa("INSERT TEXTO 3/7");
            executa("INSERT TEXTO -5/2");
            executa("INSERT TEXTO 1000/3");
            n_leituras = 0;
            executa("ACHA TEXTO 3/7");
            ok("o termo ZERO nao colide com o marcador de fim", n_leituras == 8);
            n_leituras = 0;
            executa("ACHA TEXTO -5/2");
            ok("o termo NEGATIVO tem caminho proprio", n_leituras >= 3);
            n_leituras = 0;
            executa("ACHA TEXTO 1000/3");
            ok("e o termo GRANDE tambem — nenhum tecto no termo", n_leituras >= 3);
            printf("\n");
            n_leituras = 0;
            executa("ACHA TEXTO 'zircao'");
            /* ja nao e UMA: a seta de Wick e o primeiro termo de toda a cifra, e por isso
                dois textos partilham-na sempre. Divergir cedo custa DUAS. Fica medido. */
             ok("quem diverge cedo custa duas leituras — a seta de Wick e comum a todos",
                n_leituras == 2);
            printf("\n");
            n_leituras = 0;
            executa("ACHA TEXTO 22/7");
            ok("e o racional acha-se pela MESMA porta, e pelo mesmo caminho", n_leituras == 7);
            {
                long n0 = mem_le(S_NOCAB).total;
                printf("\n");
                executa("INSERT TEXTO 'ourivesaria'");
                long n1 = mem_le(S_NOCAB).total;
                printf("      abriu %ld no(s) novo(s) — os 7 primeiros ja existiam, partilhados\n", n1-n0);
                printf("      com 'ourives'.\n");
                ok("o prefixo comum e o CAMINHO PARTILHADO, nao copia", n1 - n0 == 8);
            }
            printf("\n      Nenhuma colisao e nenhuma sondagem: cifras distintas sao caminhos\n");
            printf("      distintos. Nao ha tamanho de tabela porque nao ha tabela — e a REGUA E\n");
            printf("      INFINITA: o caminho morre onde a entrada morre, nao num tecto meu.\n");
            printf("\n-- OS 28 CORPOS, CIFRADOS, NA MESMA TABELA\n\n");
            {
                long antes = txt_n();
                executa("CORPOS");
                long lug = txt_n() - antes;
                ok("os corpos entraram na mesma tabela dos textos e dos numeros", lug > 0);
                /* 19, e nao 18: o PRISMATICO foi o primeiro a abrir lugar novo. Os formatos e as
                   linguagens cairam todos em lugares que ja existiam; o triangulo nao. */
                ok("a cifra COMPLETA separa os corpos pelas suas reguas — fecha", lug == 19);
                printf("\n      FECHOU: tantos lugares quantas reguas distintas. Nao sobrou\n");
                printf("      colisao nenhuma por perda de informacao — quando dois corpos caem\n");
                printf("      juntos e porque TEM A MESMA REGUA, e ai sao o mesmo corpo com outra\n");
                printf("      roupa.\n");
                printf("\n      E fechou SEM DELTA. O erro anterior era meu: eu tinha cifrado um\n");
                printf("      lado so do chicote, e as colisoes que sobravam ([1] para tres reguas\n");
                printf("      diferentes) nao eram limite da cifra — eram a cifra INCOMPLETA. A\n");
                printf("      seta de Wick da o outro lado: (B,C) e o seu dual (B,-C), e um deles\n");
                printf("      fecha sempre no real porque B^2-4C e B^2+4C nunca sao ambos\n");
                printf("      negativos. Com os dois lados escritos, o corpo fica inteiro na cifra\n");
                printf("      do rei, e nenhuma outra coordenada e precisa.\n");
                printf("\n      Fica dito o contestavel: para as familias parametricas tomei o\n");
                printf("      operador que o catalogo NOMEIA, e onde o parametro e livre, o membro\n");
                printf("      minimo ainda nao tomado — anotado corpo a corpo no CORPO28.\n\n");
                printf("      -- A DISTANCIA ENTRE OS CORPOS, NA TABELA\n\n");
                ok("a distancia entre os corpos e metrica, e nenhum par distinto dista 0",
                   distancia_corpos());
                printf("\n      A mesma regua que mede 'ouro' contra 'ourives' mede o aureo\n");
                printf("      contra o cosmico. Nao ha regua de corpos separada da regua de\n");
                printf("      textos: ha uma, e ela nao sabe o que esta a medir.\n\n");
                printf("      E as cifras dos corpos sao INFINITAS, todas: o que fica guardado\n");
                printf("      e o PERIODO. O aureo repete [1], o exterior repete [7], e o\n");
                printf("      hipercorpo repete o GERADOR — os 16 vertices do tesseracto — em\n");
                printf("      cada nivel, para sempre. O lado dual dele e a COSTURA, que cresce\n");
                printf("      15·16^k: multiplica por 16 por nivel, logo periodo [16].\n");
                printf("      Os 16 nao sao o corpo; sao os vertices de UM nivel.\n\n");
                printf("      E o HIPERCORPO caiu onde tinha de cair: zero contra toda a coluna\n");
                printf("      hiperbolica, e prefixo 1 com o cristalino e o fractal — os que\n");
                printf("      TAMBEM carregam o real do lado dual. FECHAM, e fecham como todos:\n");
                printf("      com a sua outra metade. NADA FECHA SOZINHO — nem o gato sem o\n");
                printf("      esquilo, nem o aureo com um lado so da cifra. A seta de Wick nao\n");
                printf("      diz \"falha\": diz QUAL DAS METADES carrega o real. E a regua poe-os\n");
                printf("      juntos sem lhe terem dito nada.\n");

                n_leituras = 0;
                executa("ACHA TEXTO 'ouro'");
                ok("e a palavra continua no seu lugar, ao lado dos corpos", n_leituras == 9);
            }

            printf("\n      E as duas coisas sao UMA SO: a distancia e 1/2^k com k o prefixo\n");
            printf("      comum, e o indice guarda-os partilhando exatamente esses k nos. A regua\n");
            printf("      e o indice nao sao duas estruturas — sao a mesma, lida de dois lados.\n");
        }

        /* O POOL COMO BACKEND: o mesmo desenho, e o circuito fecha. */
        printf("\n-- O CIRCUITO FECHADO: o banco le o job de um SLOT\n\n");
        {
            printf("      slot <  S_CANAL   LOAD/STORE -> pread/pwrite   no ficheiro\n");
            printf("      slot >= S_CANAL   LOAD/STORE -> recvfrom/sendto na banda\n");
            printf("      slot >= S_POOL    LOAD/STORE -> o job / a share no pool\n\n");
            /* a merkle root, verificada contra o genese: o coinbase e a branch, ate a raiz */
            {
                Pool P; memset(&P, 0, sizeof P);
                st_trata(&P, "{\"method\":\"mining.notify\",\"params\":[\"j\","
                  "\"0000000000000000000000000000000000000000000000000000000000000000\","
                  "\"01000000010000000000000000000000000000000000000000000000000000000000000000"
                  "ffffffff4d04ffff001d0104455468652054696d65732030332f4a616e2f3230303920436861"
                  "6e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f72"
                  "2062616e6b73\","
                  "\"ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e0"
                  "3909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c70"
                  "2b6bf11d5fac00000000\","
                  "[],\"01000000\",\"1d00ffff\",\"495fab29\",true]}");
                pool_st = P;                          /* o teste passa pela MESMA dobra */
                merkle_pelo_fold();
                P = pool_st;
                unsigned char e[32];
                st_hex("4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b", 64, e);
                int mau = 0;
                for(int i = 0; i < 32; i++) if(P.merkle_raiz[i] != e[31-i]) mau = 1;
                printf("      a merkle root do genese, do coinbase ate a raiz:\n      ");
                for(int i = 31; i >= 0; i--) printf("%02x", P.merkle_raiz[i]);
                printf("\n");
                ok("a merkle root bate o bloco genese — o coinbase e a branch estao certos", !mau);
                printf("\n      Era isto que faltava para o circuito fechar de verdade: sem a raiz o\n");
                printf("      cabecalho ia com merkle a zero e NENHUMA share seria valida.\n\n");
            }
            /* sem TIFFANY_POOL_HOST nao ha ligacao, e o backend devolve zero — que e o certo:
             * nao ha job, e o banco le isso num slot, como leria um slot vazio no disco. */
            /* A FLAG, e ela e o RELOGIO a bater: a cada volta o worker olha para a config e para
         * se ela disser. Nao e preciso matar processo — e o banco que manda. */
        { char at[64]; conf_le("mina_ativa", at, sizeof at);
          if(at[0] == '0'){ printf("mina_ativa=0 — a parar.\n"); fflush(stdout); return 1; } }
        Word tem = mem_le(S_POOL + 11);
            printf("      LOAD  S_POOL+11 (tem job?) = %d", tem.total);
            printf("%s\n", getenv("TIFFANY_POOL_HOST") ? "" : "   (sem pool ligado: 0, e e o certo)");
            ok("sem pool, o slot devolve zero — o banco le um slot, nao um erro", tem.total == 0);
            printf("\n      E o cabecalho monta-se de LOADs, nao de uma funcao de rede:\n");
            printf("        versao  <- S_POOL+0     nbits <- S_POOL+1     ntime <- S_POOL+2\n");
            printf("        prevhash <- S_POOL+3..+10   (oito palavras)\n");
            printf("        a SHARE  -> STORE em S_POOL+20, e o backend submete\n");
            printf("\n      O banco nao sabe que houve TCP nem JSON-RPC, como nao sabe que houve\n");
            printf("      UDP no canal nem que ha disco por baixo do pread. TROCAR STRATUM POR\n");
            printf("      OUTRA COISA E TROCAR DUAS FUNCOES — pool_le e pool_grava.\n");
        }

        /* O CANAL COMO BACKEND: o banco escreve num slot e le de um slot. Mais nada. */
        printf("\n-- O CANAL E BACKEND DE LOAD/STORE: os protocolos sao indistinguiveis\n\n");
        {
            Word w = w8(42, 7);
            printf("      STORE no slot S_CANAL+3  (total=%u, e=%u)\n", w.total, w.e);
            mem_grava(S_CANAL + 3, w);
            Word v = mem_le(S_CANAL + 3);
            printf("      LOAD  do slot S_CANAL+3  (total=%u, e=%u)\n", v.total, v.e);
            ok("o Word atravessou a banda e voltou inteiro — residuo 0",
               v.total == w.total && v.e == w.e);
            Word z = w8(99, 0xFF);   /* 0xFF ≡ −1 no envelope Word_8 */
            mem_grava(S_CANAL + 9, z);
            Word y = mem_le(S_CANAL + 9);
            ok("e o slot e o endereco: cada slot volta com o SEU valor", y.total == 99 && y.e == 0xFF);
            printf("\n      O banco fez LOAD e STORE, como faz no disco. Nao ha opcode novo, a ISA\n");
            printf("      nao cresceu, e nenhuma linha de SQL mudou — mas o Word foi ao meio e\n");
            printf("      voltou pela banda, com o slot a servir de endereco e so a diferenca a\n");
            printf("      viajar.\n");
            printf("\n      E POR ISSO OS PROTOCOLOS SAO INDISTINGUIVEIS: o que o banco ve e sempre\n");
            printf("      um slot lido e um slot escrito. Trocar UDP por STOMP ou por TCP e trocar\n");
            printf("      o que esta atras de canal_le e canal_grava — duas funcoes. A topologia\n");
            printf("      da comunicacao e a mesma porque, deste lado, nao ha topologia: ha um\n");
            printf("      endereco, e o endereco e o slot.\n");
        }

        /* A DOBRA NO METAL: merkle e desdobramento, nao conta. */
        printf("\n-- A DOBRA NO METAL: OP_FOLD, e merkle deixa de ter codigo proprio\n\n");
        {
            /* quatro folhas conhecidas, dobradas PELA MAQUINA a partir dos slots */
            unsigned bs = S_CAB + 2000u;   /* região física de átomos */
            for(int i = 0; i < 4; i++){
                unsigned char f[32]; memset(f, 0xA0 + i, 32);
                atomos_grava(bs + (unsigned)(32*i), f, 32);
            }
            atomos_u32(S_FOLD_ARG, bs);
            mem_grava(S_TMP + 1, w8(4, 0));
            pc_emit = 0;
            MOVE(S_TMP + 1, +1); MOVE(S_TMP + 1, +1);
            emit1(OP_FOLD); emit1(OP_HALT);
            unsigned pl = pc_emit;
            Regs rg; memset(&rg, 0, sizeof rg);
            long ps = 0; while(passo(&rg, pl)){ if(++ps > 1000000) break; }
            unsigned char raiz[32];
            atomos_le(bs, raiz, 32);
            printf("      quatro folhas -> raiz  ");
            for(int i = 0; i < 8; i++) printf("%02x", raiz[i]);
            printf("...\n      niveis desdobrados: %d  (log2 de 4)\n\n", rg.R.total);
            ok("a maquina dobra e da a raiz que a conta a mao da", raiz[0] == 0x46 && raiz[1] == 0xae);
            ok("e conta os NIVEIS: quatro folhas sao dois niveis", rg.R.total == 2);
            printf("      Merkle nao e conta: e a MESMA dobra do tesseracto e da cifra — pares que\n");
            printf("      se juntam num, nivel a nivel, com a mesma operacao em todos. M_k =\n");
            printf("      M_{k-1}·A_1: o nivel k carrega o k-1. E a branch e o DESDOBRAMENTO, o\n");
            printf("      caminho de uma folha ate a raiz — o dual da dobra.\n\n");
            printf("      O OP_FOLD estava no enum desde sempre e sem executor. Deixa de estar.\n");
        }

        /* O MARTELO: a prova de trabalho e tarefa do BANCO, e a cifra e que decide. */
        printf("\n-- O MARTELO: o banco executa a prova de trabalho, e a cifra decide\n\n");
        {
            executa("CABECALHO '0100000000000000000000000000000000000000000000000000000000000000000000003ba3edfd7a7b12b27ac72c3e67768f617fc81bc3888a51323a9fb8aa4b1e5e4a29ab5f49ffff001d1dac2b7c' "
                    "'00000000ffff0000000000000000000000000000000000000000000000000000'");
            printf("\n$ MARTELO 2083236890 2083236900\n");
            ok("o martelo acha o nonce do bloco genese na faixa que o contem",
               executa("MARTELO 2083236890 2083236900") == 1);
            printf("\n$ MARTELO 1000 1010\n");
            executa("MARTELO 1000 1010");
            printf("\n$ VERIFICA 2083236893\n");
            ok("e a REVERSAO confere esse nonce — o esquilo reverte, residuo 0",
               executa("VERIFICA 2083236893") == 1);
            printf("\n$ VERIFICA 1000\n");
            ok("e recusa um nonce qualquer", executa("VERIFICA 1000") == 0);
            printf("\n      Nada de novo foi escrito para isto: a cifra que compara o hash com o\n");
            printf("      alvo e a MESMA que compara 'ourives' com 'ourivesaria' e os trinta\n");
            printf("      corpos entre si — anda-se o caminho comum e o primeiro termo que\n");
            printf("      diverge decide. Sem largura, sem norma a transbordar, sem truncar.\n");
            printf("\n      E o par e o chicote de sempre: MARTELO procura e ESTICA (a faixa\n");
            printf("      toda), VERIFICA confere e CONTRAI (um so). E por isso que o trabalho\n");
            printf("      e PROVA — caro de achar, barato de crer.\n");
        }

        /* A DISTÂNCIA: a régua compõe as três, e é isso que o sistema devolve. */
        printf("\n-- A DISTÂNCIA ENTRE MÉTRICAS: a régua compõe as três, e não julga\n\n");
        {
            executa("CREATE TABLE k (a AUREO(1), b CRISTALINO(0))");
            executa("INSERT INTO k VALUES (3+2s, 1+1s)");
            printf("$ SELECT * FROM k WHERE a > 0      (Δ = 5, hiperbólico)\n");
            int r1 = executa("SELECT * FROM k WHERE a > 0");
            printf("\n$ SELECT * FROM k WHERE b > 0      (Δ = −4, elíptico)\n");
            int r2 = executa("SELECT * FROM k WHERE b > 0");
            ok("o sistema não RECUSA nem DESPACHA por classe — corre nas duas", r1 && r2);
            Regua ra = { 1, -1 }, rb = { 0, 1 };
            long da = ct_norma(ra,(Par){3,2}) - ct_norma(ra,(Par){1,1});
            long db = ct_norma(rb,(Par){3,2}) - ct_norma(rb,(Par){1,1});
            if(da < 0) da = -da;
            if(db < 0) db = -db;
            printf("\n      régua        Δ      d((3,2),(1,1))\n");
            printf("      a²+ab−b²     %-6ld %ld\n", ct_assinatura(ra), da);
            printf("      a²+b²        %-6ld %ld\n", ct_assinatura(rb), db);
            ok("a distância existe nas duas classes, e sai da MESMA conta", da > 0 && db > 0);
            printf("\n      APAGADO daqui: a guarda que recusava por classe e o estado que a\n");
            printf("      alimentava. Encarnavam a ideia refutada — a de que o sistema devia dar\n");
            printf("      ORDEM e, para isso, decidir a classe. Não devia.\n");
            printf("\n      A ordem obriga a escolher a classe; a distância não obriga a nada. Fica\n");
            printf("      d(u,v) = |N(u) − N(v)|, definida em toda régua, e quem julga é quem pediu.\n");
            printf("      Ver distancia.c. E ordem.c fica pelo que continua VERDADE: o elíptico não\n");
            printf("      é ordenável — resultado, e não motivo para recusar.\n");
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* O CIRCUITO FECHADO: o esquilo entra, e o metal passa a girar além de esticar. */
        printf("\n-- O CIRCUITO FECHADO: o esquilo no metal, e todo metal como PALAVRA\n\n");
        {
            int mau = 0; long casos = 0;
            /* 1. as ordens, contadas na máquina — não na conta ao lado */
            Word v; v.total = 5; v.e = 3;
            mem_grava(S_TMP, v);
            int ord_s = 0;
            for(int k = 1; k <= 12; k++){
                pc_emit = 0;
                MOVE(S_TMP, +1); emit1(OP_ESQUILO); MOVE(S_TMP, -1);
                emit1(OP_HALT); rodar(pc_emit);
                Word w = mem_le(S_TMP);
                if(w.total == 5 && w.e == 3){ ord_s = k; break; }
            }
            ok("ESQUILO tem ordem 4 NA MÁQUINA — gira e volta ao ponto de partida", ord_s == 4);
            mem_grava(S_TMP, v);
            int ord_j = 0;
            for(int k = 1; k <= 12; k++){
                pc_emit = 0;
                MOVE(S_TMP, +1); emit1(OP_TROCA); MOVE(S_TMP, -1);
                emit1(OP_HALT); rodar(pc_emit);
                Word w = mem_le(S_TMP);
                if(w.total == 5 && w.e == 3){ ord_j = k; break; }
            }
            ok("TROCA tem ordem 2 — a involução, e é a sua própria inversa", ord_j == 2);

            /* 2. O QUE FECHA O CIRCUITO: todo metal é PALAVRA nos quatro geradores. Prata e
             * bronze DEIXARAM de existir como opcode — foram apagados, porque eram atalhos.
             * Já não há opcode dedicado com que comparar: compara-se com a matriz. */
            printf("      metal    palavra nos geradores            A_m(5,3)   confere?\n");
            const char *nm[3] = { "ouro", "prata", "bronze" };
            for(int m = 1; m <= 3; m++)
            for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
                Word x; x.total = a; x.e = b;
                mem_grava(S_TMP, x);
                pc_emit = 0; emit_metal(m, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word pela_palavra = mem_le(S_TMP);
                Par esperado = me_ap(me_gato(m), (Par){a,b});
                if(pela_palavra.total != (Word8)esperado.a || pela_palavra.e != (Word8)esperado.b) mau++;
                if(a == 5 && b == 3)
                    printf("      %-8s %-32s (%d,%d)%*s%s\n", nm[m-1],
                           m==1?"GOLD":(m==2?"GOLD TROCA GOLD":"GOLD TROCA GOLD TROCA GOLD"),
                           pela_palavra.total, pela_palavra.e, 5, "",
                           (pela_palavra.total==esperado.a &&
                            pela_palavra.e==esperado.b) ? "sim ✓" : "NÃO");
                casos++;
            }
            ok("A_m = (A_1·J)^{m−1}·A_1 CONFERE no metal — prata e bronze foram APAGADOS",
               mau == 0);
            printf("      (%ld casos, três metais, e nenhum deles com opcode próprio.)\n", casos);

            /* 3. o cisalhamento, que NÃO é opcode e não precisa de ser */
            {
                Word x; x.total = 5; x.e = 3;
                mem_grava(S_TMP, x);
                pc_emit = 0;
                MOVE(S_TMP, +1); emit1(OP_TROCA); MOVE(S_TMP, -1);
                MOVE(S_TMP, +1); emit1(OP_GOLD);  MOVE(S_TMP, -1);
                emit1(OP_HALT); rodar(pc_emit);
                Word t = mem_le(S_TMP);
                /* T = [[1,1],[0,1]] em (5,3) dá (5+3, 3) = (8,3) */
                ok("TROCA GOLD é o cisalhamento T — palavra de dois, e poupa um opcode",
                   t.total == 8 && t.e == 3);
            }
            printf("\n      O circuito fecha porque o repertório fecha: o gato estica (ordem ∞, e por\n");
            printf("      isso precisou do negro), o esquilo gira (ordem 4, a inversa é S³), a troca\n");
            printf("      reflete (ordem 2, é a sua própria inversa). O que a máquina faz, ela\n");
            printf("      desfaz — nos inteiros e sem guardar cópia.\n");
            printf("\n      E o que fica de FORA, dito: decompor uma unimodular QUALQUER em palavra\n");
            printf("      não está no compilador. Mede-se que existe para os metais e para as\n");
            printf("      inversas; para matriz arbitrária o algoritmo é o de Euclides e não está\n");
            printf("      aqui. Dizer que já está seria medir a fatia e afirmar o todo.\n");
        }

        /* O CHICOTE DOS DOIS LADOS: o negro tão inteiro quanto o branco, NO METAL. */
        printf("\n-- O CHICOTE DOS DOIS LADOS: A_m no metal para TODO m, e a volta também\n\n");
        {
            int mau = 0; long casos = 0;
            /* A assimetria era minha: generalizei o branco (A_m para todo m ≥ 1) e deixei o
             * negro nos três opcodes. A régua não tem lado — T⁻¹ = J·A_1⁻¹ é o espelho exato
             * de T = A_1·J, e com ela A_m = T^{m−1}·A_1 vale para m ≤ 0 igualmente. */
            printf("      m     palavra emitida                          A_m no metal   confere?\n");
            for(long m = -12; m <= 12; m++)
            for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
                Word x; x.total = a; x.e = b;
                mem_grava(S_TMP, x);
                pc_emit = 0;
                MOVE(S_TMP, +1); emit1(OP_GOLD); MOVE(S_TMP, -1);
                if(m >= 1) for(long k = 1; k < m; k++){          /* T = TROCA depois GOLD */
                    MOVE(S_TMP, +1); emit1(OP_TROCA); MOVE(S_TMP, -1);
                    MOVE(S_TMP, +1); emit1(OP_GOLD);  MOVE(S_TMP, -1);
                } else for(long k = m; k <= 0; k++){             /* T⁻¹ = NEGRO depois TROCA */
                    MOVE(S_TMP, +1); emit1(OP_NEGRO_OURO); MOVE(S_TMP, -1);
                    MOVE(S_TMP, +1); emit1(OP_TROCA);      MOVE(S_TMP, -1);
                }
                emit1(OP_HALT); rodar(pc_emit);
                Word pela_palavra = mem_le(S_TMP);
                Par esperado = me_ap(me_gato(m), (Par){a,b});   /* o toolkit CONFERE */
                if(pela_palavra.total != (Word8)esperado.a || pela_palavra.e != (Word8)esperado.b) mau++;
                if(a == 5 && b == 3 && (m == 0 || m == -1 || m == 4))
                    printf("      %-5ld %-40s (%d,%d)%*s%s\n", m,
                           m==0 ? "GOLD NEGRO TROCA"
                                : (m==-1 ? "GOLD NEGRO TROCA NEGRO TROCA"
                                         : "GOLD TROCA GOLD TROCA GOLD TROCA GOLD"),
                           pela_palavra.total, pela_palavra.e, 3, "",
                           (pela_palavra.total==esperado.a &&
                            pela_palavra.e==esperado.b) ? "sim ✓" : "NÃO");
                casos++;
            }
            ok("A_m corre no metal para TODO m — negativo, zero e positivo, sem opcode próprio",
               mau == 0);
            printf("      (%ld casos, m de −12 a 12.)\n", casos);

            /* VOLTA no envelope Word_8: σ²=σ+1 fecha a FORMA; coeficientes que
             * transbordam sobem a torre (ℕ), não long C. Aqui mede-se o fecho
             * onde o envelope basta. */
            int mau2 = 0; long casos2 = 0;
            for(long m = -3; m <= 3; m++)
            for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++){
                Word x; x.total = (Word8)a; x.e = (Word8)b;
                mem_grava(S_TMP, x);
                pc_emit = 0;
                /* IDA */
                MOVE(S_TMP, +1); emit1(OP_GOLD); MOVE(S_TMP, -1);
                if(m >= 1) for(long k = 1; k < m; k++){
                    MOVE(S_TMP, +1); emit1(OP_TROCA); MOVE(S_TMP, -1);
                    MOVE(S_TMP, +1); emit1(OP_GOLD);  MOVE(S_TMP, -1);
                } else for(long k = m; k <= 0; k++){
                    MOVE(S_TMP, +1); emit1(OP_NEGRO_OURO); MOVE(S_TMP, -1);
                    MOVE(S_TMP, +1); emit1(OP_TROCA);      MOVE(S_TMP, -1);
                }
                /* VOLTA: a palavra ao contrário, GOLD↔NEGRO, e a TROCA fica onde está */
                if(m >= 1) for(long k = m-1; k >= 1; k--){
                    MOVE(S_TMP, +1); emit1(OP_NEGRO_OURO); MOVE(S_TMP, -1);
                    MOVE(S_TMP, +1); emit1(OP_TROCA);      MOVE(S_TMP, -1);
                } else for(long k = 0; k >= m; k--){
                    MOVE(S_TMP, +1); emit1(OP_TROCA); MOVE(S_TMP, -1);
                    MOVE(S_TMP, +1); emit1(OP_GOLD);  MOVE(S_TMP, -1);
                }
                MOVE(S_TMP, +1); emit1(OP_NEGRO_OURO); MOVE(S_TMP, -1);
                emit1(OP_HALT); rodar(pc_emit);
                Word volta = mem_le(S_TMP);
                if(volta.total != (Word8)a || volta.e != (Word8)b) mau2++;
                casos2++;
            }
            ok("e a VOLTA de todo metal é a palavra ao contrário, letra a letra invertida",
               mau2 == 0);
            printf("      (%ld percursos ida-e-volta no envelope Word_8; σ²=σ+1.)\n", casos2);
            printf("\n      A assimetria era minha, não do mecanismo: eu tinha generalizado o branco e\n");
            printf("      deixado o negro nos três opcodes. A régua não tem lado — e m = 0 dá\n");
            printf("      A_0 = J, a TROCA, que é onde o chicote passa ao mudar de sinal. Não é\n");
            printf("      peça que eu acrescentei: é o meio da régua.\n");
            printf("\n      E daqui sai o que se pode APAGAR: SILVER, BRONZE, NEGRO_PRATA e\n");
            printf("      NEGRO_BRONZE são palavras, não geradores. O repertório mínimo que fecha é\n");
            printf("      GOLD, NEGRO_OURO, TROCA e ESQUILO — quatro peças, simétricas. Os outros\n");
            printf("      quatro ficam por serem ATALHOS: poupam palavra, não poder.\n");
        }

        /* AS AFIRMAÇÕES. O teste imprimia e não concluía; agora confere contra conta feita
         * à mão, e a bateria passa a cobrir o compilador em vez de o ignorar. */
        printf("\n-- AS AFIRMAÇÕES (a bateria passa a cobrir isto)\n\n");
        {
            executa("DELETE FROM t");
            executa("INSERT INTO t VALUES (3/4,1,1)");
            executa("INSERT INTO t VALUES (5,2,1)");
            executa("INSERT INTO t VALUES (7/2,3,1)");
            executa("INSERT INTO t VALUES (2,5,1)");
            struct { const char *q; long e; const char *rot; } cs[] = {
              {"SELECT * FROM t WHERE a = 3/4",            1, "a igualdade racional fecha"},
              {"SELECT * FROM t WHERE a = 6/8",            1, "e a classe: 6/8 casa com 3/4"},
              {"SELECT * FROM t WHERE a > 1",              3, "a ordem racional, sem divisão"},
              {"SELECT * FROM t WHERE a * 2 > 7",          1, "coeficiente sobre racional"},
              {"SELECT * FROM t WHERE a > 2 AND a > 2",    2, "idempotência: A op A = A"},
              {"SELECT * FROM t WHERE (a>2 AND a<9) OR a>2",2,"absorção: a adjunção δ⊣ε"},
              {"SELECT * FROM t WHERE a + b > 5",          3, "duas colunas, denominadores"},
            };
            for(unsigned q = 0; q < sizeof cs/sizeof cs[0]; q++){
                ultima_conta = -1;
                executa(cs[q].q);
                ok(cs[q].rot, ultima_conta == cs[q].e);
            }
        }

        printf("\n-- A PARADA: o cliente pode pedir o impossível, e isso é uma resposta.\n");
        printf("\n$ SELECT * FROM t WHERE a - a = 5\n");
        executa("SELECT * FROM t WHERE a - a = 5");
        printf("\n$ SELECT * FROM t WHERE a - a = 0\n");
        executa("SELECT * FROM t WHERE a - a = 0");
        printf("\n$ SELECT * FROM t WHERE b > 20 AND a - a = 5\n");
        executa("SELECT * FROM t WHERE b > 20 AND a - a = 5");
        printf("\n$ SELECT * FROM t WHERE b > 20 OR a - a = 0\n");
        executa("SELECT * FROM t WHERE b > 20 OR a - a = 0");

        printf("\n-- o resto continua de pé:\n");
        printf("\n$ UPDATE t SET c = 111 WHERE a >= 7 AND a != 9\n");
        executa("UPDATE t SET c = 111 WHERE a >= 7 AND a != 9");
        printf("$ SELECT * FROM t\n"); executa("SELECT * FROM t");
        printf("\n$ DELETE FROM t WHERE (a <= 3 AND b >= 90)\n");
        executa("DELETE FROM t WHERE (a <= 3 AND b >= 90)");
        printf("$ SELECT * FROM t\n"); executa("SELECT * FROM t");
        fechar_base();

        /* fecha e REABRE: o dado tem de estar no disco, não na memória do processo */
        printf("\n-- fechado. reabrindo o arquivo e consultando de novo:\n");
        char m[512]; snprintf(m, sizeof m, "%s.mem", base);
        fmem = open(m, O_RDWR);
        char g[512]; snprintf(g, sizeof g, "%s.prog", base);
        fprog = open(g, O_RDWR|O_CREAT|O_TRUNC, 0644);
        printf("$ SELECT * FROM t WHERE a = 7\n"); executa("SELECT * FROM t WHERE a = 7");
        fechar_base();
        /* O EXIT SEGUE AS UNIDADES. Estava `return 0` fixo: o modo teste emitia dezenas
         * de `ok()` e saía SEMPRE 0, logo uma asserção vermelha dava verde no exit.
         * Quem apanhava era a rede da bateria (exit 0 com unidade vermelha é FALHA), e
         * uma rede não substitui o medidor dizer a verdade sobre si. */
        printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
        return falhas ? 1 : 0;
    }
    /* O SCRIPT: um comando por linha, lido da entrada. É por aqui que um produtor de fora — o
     * pipe do Stratum, por exemplo — despeja o que tem sem ter de chamar o binário uma vez por
     * linha. Linha vazia e linha que começa por -- são comentário, e não contam para nada.
     *
     * A LINHA PASSA PELO BANCO, E NAO POR RAM. Eu tinha posto um realloc aqui — "a linha cresce
     * sem tecto, nao e o leitor que ha de a cortar" — e o argumento e verdade mas nao autoriza
     * memoria: a regra e dura. Informacao que entra VAI PARA O BANCO, e e de la que se le.
     *
     * O simbolo entra, escreve-se num slot de rascunho por pwrite, e o comando executa-se do que
     * o banco tem. O buffer e FIXO e pequeno — e a janela de escrita, nao o texto. */
    if(argc >= 3 && !strcmp(argv[2], "-")){
        if(!abrir_base(argv[1])){ perror("base"); return 2; }
        #define S_LINHA  (S_LINHAS + 20000u)          /* o rascunho da linha, no banco */
        #define LIN_MAX  16384u                        /* 1024 slots: o que o banco lhe reserva */
        char *lin = DISCO_FIXO(char, 20);              /* a JANELA, fixa — nao cresce nunca */
        disco_prende(DISCO_BASE(20),"dados/sql_lin.bin",(size_t)LIN_MAX,1);
        long n = 0, mau = 0, cortadas = 0;
        for(int c; ; ){
            size_t k = 0;
            int estourou = 0;
            while((c = getchar()) != EOF && c != '\n'){
                if(k + 1 >= LIN_MAX){ estourou = 1; continue; }
                lin[k++] = (char)c;
                mem_grava(S_LINHA + (unsigned)(k - 1), (Word){ (long)(unsigned char)lin[k-1], 0 });
            }
            if(k == 0 && c == EOF) break;
            lin[k] = 0;
            if(estourou){
                cortadas++;
                fprintf(stderr, "linha maior que %u: RECUSADA, e nao truncada em silencio\n", LIN_MAX);
            } else {
                const char *p = lin; pula(&p);
                if(*p && !(p[0] == '-' && p[1] == '-')){
                    n++;
                    if(!executa(lin)){ mau++; fprintf(stderr, "falhou: %s\n", lin); }
                }
            }
            if(c == EOF) break;
        }
        if(cortadas) fprintf(stderr, "%ld linha(s) recusada(s) por tamanho\n", cortadas);
        fechar_base();
        fprintf(stderr, "%ld comando(s), %ld falha(s)\n", n, mau);
        return mau ? 1 : 0;
    }
    if(argc >= 3){
        if(!abrir_base(argv[1])){ perror("base"); return 2; }
        conf_abre(argv[1]);
        int r = executa(argv[2]);
        fechar_base();
        return r ? 0 : 1;
    }
    fprintf(stderr, "uso: sql teste | sql <base> \"<comando SQL>\" | sql <base> -   (script na entrada)\n");
    return 2;
}
#endif /* SQL_NO_MAIN */
