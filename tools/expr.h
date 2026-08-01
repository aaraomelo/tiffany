/* expr.h — A EXPRESSÃO NUMÉRICA RESOLVIDA POR DOBRA, NO BANCO.
 *
 * O Aarão: "vamos para as expressões numéricas, a assistente resolve passo a passo e explica;
 * começa simples, com parênteses, colchetes e chaves, soma e multiplicação."
 *
 * E não há máquina nova a inventar. Uma expressão aninhada É uma árvore, e resolver é DOBRAR de
 * dentro para fora — que é o OP_FOLD do banco, o mesmo que junta folhas de merkle aos pares até
 * sobrar uma. Aqui as folhas são os números e a dobra é a operação.
 *
 * O que isso dá de graça, e é o que o Aarão pediu:
 *
 *   - o PASSO A PASSO não se programa: cada dobra é um passo, e mostrar a fita depois de cada
 *     uma é a explicação inteira. Não há um "modo explicativo" ao lado do modo que resolve —
 *     é o mesmo caminho, visto de fora;
 *   - a PRECEDÊNCIA não é uma tabela: é a ORDEM das dobras. Dobra-se o mais fundo primeiro
 *     (o parêntese mais interno), e dentro de cada nível dobra-se o × antes do +. A regra
 *     escolar cai fora do desdobramento em vez de ser imposta por cima dele;
 *   - os três delimitadores são O MESMO. Parêntese, colchete e chave só mudam de roupa, e quem
 *     manda é a PROFUNDIDADE. É o critério do Aarão aplicado: não interessa a roupa, interessa
 *     se fecha. Mas fechar tem de fechar com o par certo — um ']' não fecha um '(' — e isso é
 *     verificado, e recusado se falhar. Fail-closed, como o resto.
 *
 * NADA EM RAM. A fita é um ficheiro, lida e escrita por pread/pwrite. A pilha do emparelhamento
 * também mora lá, num troço à frente da fita. O único inteiro que anda fora é o número que se
 * está a formar ao ler os dígitos, e esse é um long, não um acumulador que cresce.
 */
#ifndef EXPR_H
#define EXPR_H


#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* a célula da fita: {tipo, valor} */
#define C_VAZIO 0            /* célula já consumida por uma dobra */
#define C_NUM   1            /* um número */
#define C_OP    2            /* '+' ou '*' */
#define C_ABRE  3            /* '(', '[' ou '{' */
#define C_FECHA 4            /* ')', ']' ou '}' */

typedef struct { long tipo, val; } Cel;
#define CS ((long)sizeof(Cel))

#define CT_FITA  0           /* a fita começa na célula 0 */
#define CT_PILHA 4096        /* a pilha do emparelhamento, à frente */

static Cel ct_le(int fd, long i){ Cel c = {0,0}; pread(fd, &c, CS, i*CS); return c; }
static void ct_poe(int fd, long i, long t, long v){ Cel c = {t,v}; pwrite(fd, &c, CS, i*CS); }

/* o par do delimitador: qual fecho casa com qual abertura */
static long ct_par(long abre){ return abre=='(' ? ')' : abre=='[' ? ']' : abre=='{' ? '}' : 0; }

/* LER: o texto entra e vai DIRETO para a fita, símbolo a símbolo. Devolve o comprimento, ou
 * negativo se a expressão não fecha — e o negativo diz onde. */
static long ct_leia(int fd, const char *s){
    long n = 0, topo = 0;
    while(*s){
        if(*s == ' ' || *s == '\t'){ s++; continue; }
        if(*s >= '0' && *s <= '9'){
            long v = 0;
            while(*s >= '0' && *s <= '9'){ v = v*10 + (*s - '0'); s++; }
            ct_poe(fd, CT_FITA + n++, C_NUM, v);
            continue;
        }
        if(*s == '+' || *s == '*' || *s == 'x' || *s == 'X'){
            ct_poe(fd, CT_FITA + n++, C_OP, (*s=='+') ? '+' : '*'); s++; continue;
        }
        if(*s == '(' || *s == '[' || *s == '{'){
            ct_poe(fd, CT_PILHA + topo++, C_ABRE, *s);       /* a pilha vive no banco */
            ct_poe(fd, CT_FITA + n++, C_ABRE, *s); s++; continue;
        }
        if(*s == ')' || *s == ']' || *s == '}'){
            if(topo == 0) return -1;                          /* fecha sem ter aberto */
            Cel a = ct_le(fd, CT_PILHA + --topo);
            if(ct_par(a.val) != *s) return -2;                /* fecha com a roupa errada */
            ct_poe(fd, CT_FITA + n++, C_FECHA, *s); s++; continue;
        }
        return -3;                                            /* símbolo que não é desta conta */
    }
    if(topo != 0) return -4;                                  /* abriu e não fechou */
    ct_poe(fd, CT_FITA + n, C_VAZIO, 0);                      /* o fim */
    return n;
}

/* MOSTRAR: a fita como se lê, saltando o que já foi consumido. */
static void ct_mostra(int fd, long n, char *out, size_t lim){
    size_t k = 0; out[0] = 0;
    for(long i = 0; i < n; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo == C_VAZIO) continue;
        char b[32];
        if(c.tipo == C_NUM)      snprintf(b, sizeof b, "%ld", c.val);
        else if(c.tipo == C_OP)  snprintf(b, sizeof b, " %c ", c.val == '*' ? 'x' : '+');
        else                     snprintf(b, sizeof b, "%c", (char)c.val);
        size_t l = strlen(b);
        if(k + l + 1 >= lim) break;
        memcpy(out + k, b, l); k += l; out[k] = 0;
    }
}

/* a célula útil seguinte/anterior — a fita fica com buracos e é assim que ela encolhe */
static long ct_prox(int fd, long i, long n){
    for(long k = i+1; k < n; k++) if(ct_le(fd, CT_FITA + k).tipo != C_VAZIO) return k;
    return -1;
}
static long ct_ante(int fd, long i){
    for(long k = i-1; k >= 0; k--) if(ct_le(fd, CT_FITA + k).tipo != C_VAZIO) return k;
    return -1;
}

/* UM PASSO. Devolve 1 se dobrou (e escreve a explicação em porque), 0 se já não há que dobrar.
 *
 * A ordem — e ela é o mecanismo inteiro:
 *   1. acha a maior profundidade de parêntese que ainda tenha operação lá dentro;
 *   2. nesse nível dobra um '*'; se não houver nenhum, dobra um '+';
 *   3. se o nível já é um número sozinho entre delimitadores, tira os delimitadores.
 * A precedência CAI daqui. Não há tabela.
 */
static int ct_passo(int fd, long n, char *porque, size_t lim){
    long alvo = -1, ini = 0, fim = n;

    /* varre uma vez guardando o troço mais fundo que ainda tem operador ou que é redutível */
    long p = 0, melhor = -1, mi = 0, mf = 0;
    for(long i = 0; i < n; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo == C_ABRE){ p++; if(p > melhor){ melhor = p; mi = i; } }
        else if(c.tipo == C_FECHA){ if(p == melhor && mf <= mi) mf = i; p--; }
    }
    if(melhor > 0){ ini = mi + 1; fim = mf; alvo = mi; }

    /* dentro do troço: primeiro os '*', depois os '+' — e é isto a precedência */
    for(int passada = 0; passada < 2; passada++){
        long quero = passada == 0 ? '*' : '+';
        for(long i = ini; i < fim; i++){
            Cel c = ct_le(fd, CT_FITA + i);
            if(c.tipo != C_OP || c.val != quero) continue;
            long e = ct_ante(fd, i), d = ct_prox(fd, i, n);
            if(e < 0 || d < 0) continue;
            Cel A = ct_le(fd, CT_FITA + e), B = ct_le(fd, CT_FITA + d);
            if(A.tipo != C_NUM || B.tipo != C_NUM) continue;
            long r = quero == '*' ? A.val * B.val : A.val + B.val;
            snprintf(porque, lim, "%ld %s %ld = %ld", A.val, quero=='*' ? "x" : "+", B.val, r);
            ct_poe(fd, CT_FITA + e, C_NUM, r);                /* o resultado ocupa o lugar do 1º */
            ct_poe(fd, CT_FITA + i, C_VAZIO, 0);              /* o operador e o 2º saem */
            ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
            return 1;
        }
    }
    /* o troço já é um número só: tira a roupa. É a dilatação a devolver o valor ao nível de cima */
    if(alvo >= 0){
        long u = ct_prox(fd, alvo, n);
        if(u >= 0 && u < fim && ct_le(fd, CT_FITA + u).tipo == C_NUM && ct_prox(fd, u, n) == fim){
            Cel v = ct_le(fd, CT_FITA + u);
            Cel a = ct_le(fd, CT_FITA + alvo);
            snprintf(porque, lim, "%c%ld%c já é um número: tiro os %c%c",
                     (char)a.val, v.val, (char)ct_par(a.val), (char)a.val, (char)ct_par(a.val));
            ct_poe(fd, CT_FITA + alvo, C_VAZIO, 0);
            ct_poe(fd, CT_FITA + fim,  C_VAZIO, 0);
            return 1;
        }
    }
    return 0;
}

/* o valor final, quando sobra um número só; devolve 0 se não sobrou um só */
static int ct_valor(int fd, long n, long *out){
    long v = 0, quantos = 0;
    for(long i = 0; i < n; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo == C_VAZIO) continue;
        if(c.tipo != C_NUM) return 0;
        v = c.val; quantos++;
    }
    if(quantos != 1) return 0;
    *out = v; return 1;
}

#endif
