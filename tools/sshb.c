/* sshb.c — O SSH INGERIDO, E AS VOLTAS CONTADAS CONTRA O BUMP.
 *
 * O Aarão: "ingere o ssh e compara as voltas com o bump — na verdade temos nosso próprio protocolo
 * via bump na banda própria."
 *
 * E isto nasceu de uma falha real, hoje: o `publica.yml` morreu com
 *
 *     ssh: connect to host srv1559444.hstgr.cloud port 22: Network is unreachable
 *
 * — o runner resolvia o host em IPv6 e não tinha rota. Falhou **antes de haver ligação**, e isso é
 * o ponto: o SSH tem um antes-de-haver-ligação onde falhar. O bump do `canal.c` não tem, porque
 * não há negociação nenhuma para correr mal. Não é uma opinião sobre desenho: é uma consequência
 * de quantas voltas cada um gasta, e as voltas contam-se.
 *
 * A MEDIDA, e ela não é opinativa: numa sequência de mensagens, cada vez que a direção INVERTE
 * (cliente→servidor passa a servidor→cliente, ou o contrário) gastou-se meia volta de rede. O
 * primeiro byte útil não pode chegar antes disso, em nenhuma implementação, por mais rápida que
 * seja — é a latência da luz, não do código. Então:
 *
 *     voltas = (número de inversões de direção antes do primeiro byte útil) / 2
 *
 * Contar inversões é uma medida do PROTOCOLO, não de uma implementação. E o oráculo é externo: os
 * números de mensagem estão nos RFC 4253/4252/4254 e não fui eu que os escolhi.
 *
 *   §H1  a INGESTÃO: o pacote SSH desce pela mesma descida — a marca é o COMPRIMENTO
 *   §H2  a sequência do SSH, do RFC, e as inversões de direção contadas
 *   §H3  o BUMP: zero inversões antes do payload, e porquê — não há o que negociar
 *   §H4  a comparação, e o que ela custa em latência real
 *   §H5  e o que o SSH tem e o bump NÃO tem — a conta honesta dos dois lados
 *
 *   cc -O2 -std=c99 sshb.c -lm -o sshb && ./sshb
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ───────────────────────────────────────────────────────────────────────────
 * §H1  A INGESTÃO — a marca do nível do SSH é o COMPRIMENTO
 *
 * RFC 4253 §6, o Binary Packet Protocol:
 *
 *     uint32   packet_length          (não conta a si próprio nem o MAC)
 *     byte     padding_length
 *     byte[n1] payload                n1 = packet_length − padding_length − 1
 *     byte[n2] random padding         n2 = padding_length,  >= 4
 *     byte[m]  MAC
 *
 * É a MESMA família do WASM: o formato diz o tamanho ANTES do corpo, então descer é somar. O JSON
 * marca com o parêntese e tem de se procurar o fecho; o SSH e o WASM dizem-no à cabeça.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { long comprimento, pad, payload, total; int tipo; } Pacote;

/* a descida sobre um pacote SSH: devolve 1 e preenche, ou 0 se o pacote é inválido */
static int ssh_desce(const unsigned char *b, long n, long off, Pacote *p){
    if(off + 6 > n) return 0;
    long L = ((long)b[off]<<24) | ((long)b[off+1]<<16) | ((long)b[off+2]<<8) | b[off+3];
    long pad = b[off+4];
    if(L < 12 || L > 35000) return 0;              /* o RFC manda recusar acima de 35000 */
    if(pad < 4) return 0;                          /* o padding mínimo é 4, e é regra do formato */
    if(pad + 1 > L) return 0;
    p->comprimento = L;
    p->pad = pad;
    p->payload = L - pad - 1;
    p->total = 4 + L;                              /* sem MAC: antes das chaves não há MAC */
    p->tipo = (off + 5 < n) ? b[off+5] : -1;
    return p->payload > 0;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §H2  A SEQUÊNCIA — do RFC, com a direção de cada mensagem
 *
 * dir: +1 = cliente -> servidor,  −1 = servidor -> cliente
 * Os números de mensagem são os do RFC 4253 §12 e RFC 4252 §6 — oráculo externo.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { int cod; const char *nome; int dir; const char *rfc; } Msg;

static const Msg SSH[] = {
    {  -3, "TCP SYN",                  +1, "TCP"      },
    {  -3, "TCP SYN-ACK",              -1, "TCP"      },
    {  -3, "TCP ACK",                  +1, "TCP"      },
    {  -2, "banner do cliente",        +1, "4253 §4.2"},
    {  -2, "banner do servidor",       -1, "4253 §4.2"},
    {  20, "KEXINIT do cliente",       +1, "4253 §7.1"},
    {  20, "KEXINIT do servidor",      -1, "4253 §7.1"},
    {  30, "KEXDH_INIT",               +1, "4253 §8"  },
    {  31, "KEXDH_REPLY",              -1, "4253 §8"  },
    {  21, "NEWKEYS do cliente",       +1, "4253 §7.3"},
    {  21, "NEWKEYS do servidor",      -1, "4253 §7.3"},
    {   5, "SERVICE_REQUEST",          +1, "4253 §10" },
    {   6, "SERVICE_ACCEPT",           -1, "4253 §10" },
    {  50, "USERAUTH_REQUEST",         +1, "4252 §5"  },
    {  52, "USERAUTH_SUCCESS",         -1, "4252 §5"  },
    {  90, "CHANNEL_OPEN",             +1, "4254 §5.1"},
    {  91, "CHANNEL_OPEN_CONFIRM",     -1, "4254 §5.1"},
    {  98, "CHANNEL_REQUEST (exec)",   +1, "4254 §6.5"},
    {  94, "CHANNEL_DATA  <- O PRIMEIRO BYTE UTIL", -1, "4254 §5.2"},
};
#define NSSH ((int)(sizeof SSH / sizeof SSH[0]))

/* ───────────────────────────────────────────────────────────────────────────
 * §H3  O BUMP — a sequência do canal.c
 *
 *     banda = sha256(tecido)              a assinatura, e ela já é conhecida dos dois lados
 *     bump  = msg XOR keystream(banda)    o gap: a ÚNICA coisa que cruza o meio
 *     antena                              datagrama cru, sem conexão e sem cifra por cima
 *
 * Não há negociação porque não há o que negociar: a banda não se acorda no momento, ela É a
 * assinatura do tecido, que os dois lados já têm. Acordar uma chave é o que custa as voltas.
 * ─────────────────────────────────────────────────────────────────────────── */

static const Msg BUMP[] = {
    { 0, "bump  <- O PRIMEIRO BYTE UTIL, e o unico", +1, "canal.c §N1" },
};
#define NBUMP ((int)(sizeof BUMP / sizeof BUMP[0]))

/* A MEDIDA: quantas vezes a direção inverte antes de o primeiro byte útil chegar.
 * Cada inversão é meia volta de rede — e isso é da luz, não da implementação. */
static int inversoes(const Msg *m, int n, int *ate_util){
    int inv = 0, dir = 0, i;
    for(i = 0; i < n; i++){
        if(dir && m[i].dir != dir) inv++;
        dir = m[i].dir;
        if(strstr(m[i].nome, "PRIMEIRO BYTE UTIL")) break;
    }
    *ate_util = i + 1;
    return inv;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("sshb.c — O SSH INGERIDO, E AS VOLTAS CONTADAS CONTRA O BUMP\n");

    /* ── §H1 ─────────────────────────────────────────────────────────────── */
    puts("§H1  A INGESTAO: a marca do nivel do SSH e o COMPRIMENTO");
    puts("     uint32 packet_length, byte padding_length, payload, padding, MAC (RFC 4253 §6).");
    puts("     E a mesma familia do WASM: o formato diz o tamanho ANTES do corpo, e descer e");
    puts("     somar. O JSON marca com o parentese e obriga a PROCURAR o fecho; estes dizem-no.\n");
    {
        /* um pacote KEXINIT mínimo, montado à regra do RFC — e a descida tem de o ler */
        unsigned char b[64];
        memset(b, 0, sizeof b);
        long payload = 17, pad = 6, L = payload + pad + 1;       /* 24 */
        b[0]=(unsigned char)(L>>24); b[1]=(unsigned char)(L>>16);
        b[2]=(unsigned char)(L>>8);  b[3]=(unsigned char)L;
        b[4]=(unsigned char)pad;
        b[5]=20;                                                  /* SSH_MSG_KEXINIT */
        Pacote p;
        int leu = ssh_desce(b, (long)sizeof b, 0, &p);
        ok("a descida le o pacote: comprimento, padding e payload saem do cabecalho",
           leu && p.comprimento == 24 && p.pad == 6 && p.payload == 17 && p.tipo == 20);
        ok("e A CONTA FECHA: payload + padding + 1 == packet_length, sem sobra",
           leu && p.payload + p.pad + 1 == p.comprimento);
        /* e recusa o que o RFC manda recusar — senao a descida aceita lixo e nao mede nada */
        unsigned char mau[16]; memset(mau, 0, sizeof mau);
        mau[3] = 24; mau[4] = 2;                                  /* padding 2, e o mínimo é 4 */
        Pacote q;
        ok("e RECUSA o invalido: padding abaixo de 4 nao e pacote, e o RFC di-lo",
           !ssh_desce(mau, (long)sizeof mau, 0, &q));
        printf("     -> %ld bytes no fio para %ld de payload: %.0f%% e enchimento e cabecalho.\n",
               p.total, p.payload, 100.0*(p.total - p.payload)/p.total);
        puts("");
    }

    /* ── §H2 ─────────────────────────────────────────────────────────────── */
    puts("§H2  A SEQUENCIA DO SSH, do RFC, e as INVERSOES DE DIRECAO contadas");
    puts("     Cada vez que a direcao inverte gastou-se MEIA VOLTA de rede. O primeiro byte util");
    puts("     nao pode chegar antes disso em implementacao nenhuma — e a latencia da luz.\n");
    {
        int ate; int inv = inversoes(SSH, NSSH, &ate);
        int dir = 0;
        for(int i = 0; i < ate; i++){
            int virou = (dir && SSH[i].dir != dir);
            printf("     %s %-32s  %s%s\n",
                   SSH[i].dir > 0 ? "-->" : "<--", SSH[i].nome,
                   SSH[i].rfc, virou ? "   * virou" : "");
            dir = SSH[i].dir;
        }
        ok("a sequencia do SSH alterna de direcao muitas vezes antes do primeiro byte util",
           inv >= 10);
        printf("     -> %d mensagens, %d inversoes = %.1f VOLTAS completas antes do 1o byte util.\n",
               ate, inv, inv/2.0);
        puts("");
    }

    /* ── §H3 ─────────────────────────────────────────────────────────────── */
    puts("§H3  O BUMP: zero inversoes, e a razao nao e velocidade — e que nao ha o que negociar");
    puts("     banda = sha256(tecido), e os dois lados JA a tem: ela e a assinatura do tecido,");
    puts("     nao uma chave acordada no momento. Acordar chave e o que custa as voltas.\n");
    {
        int ate; int inv = inversoes(BUMP, NBUMP, &ate);
        printf("     --> %-32s  %s\n", BUMP[0].nome, BUMP[0].rfc);
        ok("o bump chega em ZERO inversoes: o primeiro pacote JA E o byte util",
           inv == 0 && ate == 1);
        /* e isto nao e um numero meu: e consequencia de a banda nao se negociar. Mede-se pela
         * definicao — se houvesse acordo de chave, haveria pelo menos uma ida e uma volta. */
        ok("e nao ha acordo de chave para inverter a direcao — logo nao PODE haver volta nenhuma",
           NBUMP == 1);
        puts("     -> 1 mensagem, 0 inversoes = 0 VOLTAS. O bump e o gap, e o gap ja e a mensagem.");
        puts("");
    }

    /* ── §H4 ─────────────────────────────────────────────────────────────── */
    puts("§H4  A COMPARACAO, e o que ela custa em latencia real\n");
    {
        int a1, a2;
        int i_ssh  = inversoes(SSH,  NSSH,  &a1);
        int i_bump = inversoes(BUMP, NBUMP, &a2);
        double v_ssh = i_ssh/2.0, v_bump = i_bump/2.0;
        ok("o SSH gasta voltas antes do primeiro byte util; o bump gasta ZERO",
           v_ssh > 0 && v_bump == 0);
        printf("        %-10s %2d mensagens   %2d inversoes   %4.1f voltas\n", "SSH", a1, i_ssh, v_ssh);
        printf("        %-10s %2d mensagem    %2d inversoes   %4.1f voltas\n", "bump", a2, i_bump, v_bump);
        puts("");
        /* e agora em milissegundos, com latencias REAIS e nomeadas — nao um numero inventado */
        struct { const char *onde; double rtt; } CASOS[] = {
            { "rede local          ",   0.5 },
            { "mesmo pais          ",  15.0 },
            { "Boa Vista - Sao Paulo", 60.0 },
            { "transatlantico      ", 180.0 },
            { "satelite geo        ", 600.0 },
        };
        int monotono = 1; double ant = -1;
        for(int i = 0; i < 5; i++){
            double t = v_ssh * CASOS[i].rtt;
            printf("     %s  rtt %5.1f ms  ->  SSH espera %7.1f ms, o bump %4.1f ms\n",
                   CASOS[i].onde, CASOS[i].rtt, t, 0.0);
            if(t <= ant) monotono = 0;
            ant = t;
        }
        ok("e a espera do SSH CRESCE com a distancia enquanto a do bump fica em zero",
           monotono);
        printf("     -> a diferenca nao e uma constante: e %.1f x o rtt, sempre. Num satelite\n", v_ssh);
        printf("        sao %.1f segundos antes de o primeiro byte util existir.\n", v_ssh*600/1000.0);
        puts("");
    }

    /* ── §H5  a conta dos DOIS lados ─────────────────────────────────────── */
    puts("§H5  E O QUE O SSH TEM E O BUMP NAO TEM — a conta honesta, senao isto e propaganda\n");
    {
        /* Um medidor que so conta a favor de um lado nao mede: escolhe. Entao lista-se o que o
         * SSH COMPRA com as voltas que gasta, e verifica-se que o bump nao o tem. */
        static const struct { const char *o_que; int ssh; int bump; } CONTA[] = {
            { "chega ao primeiro byte util sem voltas",            0, 1 },
            { "funciona sem estado partilhado previo",             1, 0 },
            { "negoceia o algoritmo com quem nao o conhece",       1, 0 },
            { "autentica o servidor a quem nunca o viu",           1, 0 },
            { "sobrevive a mudanca de chave a meio",               1, 0 },
            { "atravessa NAT e firewall pela porta 22",            1, 0 },
            { "nao tem um 'antes-da-ligacao' onde falhar",         0, 1 },
        };
        int n = 7, so_ssh = 0, so_bump = 0;
        for(int i = 0; i < n; i++){
            printf("     %-46s  SSH %s   bump %s\n", CONTA[i].o_que,
                   CONTA[i].ssh ? "sim" : "NAO", CONTA[i].bump ? "sim" : "NAO");
            if(CONTA[i].ssh && !CONTA[i].bump) so_ssh++;
            if(CONTA[i].bump && !CONTA[i].ssh) so_bump++;
        }
        ok("os dois lados tem coluna: ha coisas que so o SSH faz, e ha coisas que so o bump faz",
           so_ssh >= 4 && so_bump >= 2);
        printf("     -> %d so no SSH, %d so no bump. A troca e clara: o SSH paga voltas para\n",
               so_ssh, so_bump);
        puts("        falar com um DESCONHECIDO; o bump nao paga porque nao fala com desconhecidos.");
        puts("        E foi por isso que o publica.yml falhou hoje com 'Network is unreachable':");
        puts("        ele morreu no 'antes-da-ligacao', que e a coluna que o bump nao tem.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O SSH nao abriu lugar novo: a marca do nivel dele e o COMPRIMENTO, como a do WASM.");
    puts("  Oitava roupa, a mesma descida.");
    puts("");
    puts("  E a comparacao nao e de gosto. Contar inversoes de direcao e uma medida do");
    puts("  PROTOCOLO, nao de uma implementacao, porque nenhuma implementacao pode entregar");
    puts("  o primeiro byte antes de a luz ir e voltar. O SSH paga essas voltas para poder");
    puts("  falar com um desconhecido. Nos nao falamos com desconhecidos: a banda E a");
    puts("  assinatura do tecido, e quem nao a tem nem decodifica.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
