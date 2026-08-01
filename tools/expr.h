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
#define CT_OUT   8192        /* a fita de saída, para a reescrita distributiva */

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
        /* O SINAL UNÁRIO. Um '-' no início, ou logo depois de uma abertura ou de outro
         * operador, não é subtração: é o sinal do número que vem a seguir. Sem isto,
         * "2 x (0 - 5)" resolve mas "-5 + 2" não entra — e o negativo já existe na fita
         * desde que a subtração existe, logo recusá-lo à entrada era incoerência minha. */
        if(*s == '-'){
            Cel ant = n ? ct_le(fd, CT_FITA + n - 1) : (Cel){C_OP, '+'};
            if(!n || ant.tipo == C_OP || ant.tipo == C_ABRE){
                const char *q = s + 1;
                while(*q == ' ') q++;
                if(*q >= '0' && *q <= '9'){
                    long v = 0;
                    while(*q >= '0' && *q <= '9'){ v = v*10 + (*q - '0'); q++; }
                    ct_poe(fd, CT_FITA + n++, C_NUM, -v); s = q; continue;
                }
            }
        }
        if(*s=='+' || *s=='-' || *s=='*' || *s=='x' || *s=='X' || *s=='/' || *s==':'){
            long o = (*s=='+') ? '+' : (*s=='-') ? '-' : (*s=='/'||*s==':') ? '/' : '*';
            ct_poe(fd, CT_FITA + n++, C_OP, o); s++; continue;
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
        /* o negativo vai entre parênteses quando é operando de alguma coisa, senão sai
         * "1 + -4", que é o que a máquina tem lá dentro mas não é o que se escreve. */
        if(c.tipo == C_NUM){
            int op_antes = i > 0 && ct_le(fd, CT_FITA + i - 1).tipo == C_OP;
            if(c.val < 0 && op_antes) snprintf(b, sizeof b, "(%ld)", c.val);
            else                      snprintf(b, sizeof b, "%ld", c.val);
        }
        else if(c.tipo == C_OP)  snprintf(b, sizeof b, " %c ",
                                          c.val == '*' ? 'x' : (char)c.val);
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

    /* Dentro do troço: primeiro {x, /}, depois {+, -}. E cada passada trata os DOIS do seu
     * nível JUNTOS, na ordem em que aparecem — não pode ser um antes do outro.
     *
     * Isto foi o que quase me escapou: fazer todos os '+' e só depois os '-' daria, em
     * "10 - 2 + 3", primeiro 2+3=5 e depois 10-5=5, quando o certo é (10-2)+3 = 11. A
     * subtração NÃO é associativa, e a associatividade à esquerda tem de sair da varredura:
     * dobra-se o PRIMEIRO do conjunto que se encontra da esquerda para a direita, e cada
     * chamada faz uma dobra só. O mesmo em "8 / 2 x 2", que é 8 e não 2. */
    for(int passada = 0; passada < 2; passada++){
        const char *conj = passada == 0 ? "*/" : "+-";
        for(long i = ini; i < fim; i++){
            Cel c = ct_le(fd, CT_FITA + i);
            if(c.tipo != C_OP || !strchr(conj, (int)c.val)) continue;
            long e = ct_ante(fd, i), d = ct_prox(fd, i, n);
            if(e < 0 || d < 0) continue;
            Cel A = ct_le(fd, CT_FITA + e), B = ct_le(fd, CT_FITA + d);
            if(A.tipo != C_NUM || B.tipo != C_NUM) continue;
            long quero = c.val, r;
            if(quero == '/'){
                /* A DIVISÃO NÃO FECHA EM Z, e isso não se arredonda nem se cala. O corpus
                 * científico já o diz da subtração em N; aqui a máquina encontra-o de facto. */
                if(B.val == 0){
                    snprintf(porque, lim, "%ld a dividir por 0 não existe em corpo nenhum: se "
                             "0 vezes x fosse 1, então 0 = 1 e a estrutura colapsava", A.val);
                    return -1;
                }
                if(A.val % B.val != 0){
                    long q = A.val / B.val, resto = A.val - q*B.val;
                    snprintf(porque, lim, "%ld a dividir por %ld não fecha em Z: %ld = %ld x %ld "
                             "+ %ld, e sobra %ld. Em Q existe e vale %ld sobre %ld",
                             A.val, B.val, A.val, q, B.val, resto, resto, A.val, B.val);
                    return -1;
                }
                r = A.val / B.val;
            }
            else if(quero == '*') r = A.val * B.val;
            else if(quero == '-') r = A.val - B.val;
            else                  r = A.val + B.val;
            snprintf(porque, lim, "%ld %c %ld = %ld", A.val, quero=='*' ? 'x' : (char)quero,
                     B.val, r);
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

/* ─── A DISTRIBUTIVA, E O SEU DUAL ─────────────────────────────────────────────────────────
 *
 * a x (b + c) = a x b + a x c
 *
 * Aqui ela não é mais uma regra a acrescentar à lista: é a REESCRITA que prova que o valor não
 * depende da ordem por que se dobra. Dobrar por dentro dá 2 x 7 = 14; distribuir dá 6 + 8 = 14.
 * Os dois caminhos fecham no mesmo, e não por acaso — é isso que a lei afirma.
 *
 * E ela tem DUAL: fatorar. Distribuir abre, fatorar fecha, e são o mesmo par de sempre — como a
 * erosão e a dilatação do mórfico. Uma lei sem o seu dual seria meia lei.
 *
 * ONDE ELA NÃO VALE, E ISSO VERIFICA-SE: a distributiva é entre DUAS operações diferentes. O x
 * distribui sobre o +; o x sobre o x não distribui — 2 x (3 x 4) é 24, e distribuir ali daria
 * (2x3) x (2x4) = 48, o DOBRO, porque o fator entraria duas vezes e as parcelas multiplicam-se
 * entre si. Sobre o + isso não acontece: as parcelas somam, e o fator sai inteiro em evidência.
 * O + sobre o + também não distribui. Quem pedir para distribuir onde não vale leva recusa, com o
 * motivo dito — é a mesma disciplina do corpus: a lei vale no corpo declarado, e não em geral.
 *
 * A reescrita vai para uma segunda fita no MESMO banco (CT_OUT) e volta copiada. Nada em RAM.
 */

/* copia a célula i da fita de entrada para a posição j da saída */
static void ct_copia(int fd, long de, long para){
    Cel c = ct_le(fd, CT_FITA + de);
    ct_poe(fd, CT_OUT + para, c.tipo, c.val);
}
/* o fim do grupo que abre em i (i aponta para o C_ABRE); -1 se não fecha */
static long ct_grupo_fim(int fd, long i, long n){
    long p = 0;
    for(long k = i; k < n; k++){
        Cel c = ct_le(fd, CT_FITA + k);
        if(c.tipo == C_ABRE) p++;
        else if(c.tipo == C_FECHA){ p--; if(!p) return k; }
    }
    return -1;
}
/* o início do fator que TERMINA em i: um número é uma célula, um grupo é o grupo todo */
static long ct_fator_ini(int fd, long i){
    Cel c = ct_le(fd, CT_FITA + i);
    if(c.tipo == C_NUM) return i;
    if(c.tipo != C_FECHA) return -1;
    long p = 0;
    for(long k = i; k >= 0; k--){
        Cel d = ct_le(fd, CT_FITA + k);
        if(d.tipo == C_FECHA) p++;
        else if(d.tipo == C_ABRE){ p--; if(!p) return k; }
    }
    return -1;
}

/* DISTRIBUIR. Acha o primeiro  F x (A + B + ...)  ou  (A + B + ...) x F  e reescreve.
 * Devolve o novo comprimento, 0 se não há nada a distribuir, e -1 se o grupo achado só tem x
 * lá dentro — que é o pedido de distribuir onde a lei não vale. */
static long ct_distribui(int fd, long n, char *porque, size_t lim){
    for(long i = 0; i < n; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo != C_OP || (c.val != '*' && c.val != '/')) continue;

        /* de que lado está o grupo, e de que lado está o fator.
         *
         * E AQUI A DIVISÃO É ASSIMÉTRICA, o que a multiplicação não é: (a+b)/c distribui e
         * dá a/c + b/c, mas c/(a+b) NÃO distribui. Mede-se: (4+2)/2 = 3 e 4/2 + 2/2 = 3;
         * já 12/(2+4) = 2 enquanto 12/2 + 12/4 = 9. A divisão só distribui pela DIREITA, e é
         * o mesmo defeito de simetria que faz dela não comutativa. */
        long gi = -1, gf = -1, fi = -1, ff = -1;
        Cel dir = ct_le(fd, CT_FITA + i + 1);
        if(c.val == '/' && dir.tipo == C_ABRE){
            snprintf(porque, lim, "aqui não distribuo: a divisão só distribui pela DIREITA. "
                     "(a+b)/c é a/c + b/c, mas c/(a+b) não é c/a + c/b — 12/(2+4) é 2 e "
                     "12/2 + 12/4 seria 9");
            return -1;
        }
        if(dir.tipo == C_ABRE){
            gi = i + 1; gf = ct_grupo_fim(fd, gi, n);
            ff = i - 1; fi = ct_fator_ini(fd, ff);
        } else {
            Cel esq = ct_le(fd, CT_FITA + i - 1);
            if(esq.tipo != C_FECHA) continue;
            gf = i - 1; gi = ct_fator_ini(fd, gf);
            fi = i + 1; ff = fi;
            Cel f = ct_le(fd, CT_FITA + fi);
            if(f.tipo == C_ABRE) ff = ct_grupo_fim(fd, fi, n);
            else if(f.tipo != C_NUM) continue;
        }
        if(gi < 0 || gf < 0 || fi < 0 || ff < 0) continue;

        /* o grupo tem de ter um + ou um - no seu nível de topo — a distributiva vale sobre a
         * soma E sobre a subtração, que é a mesma operação com o dual de um lado. */
        long p = 0, somas = 0;
        for(long k = gi + 1; k < gf; k++){
            Cel d = ct_le(fd, CT_FITA + k);
            if(d.tipo == C_ABRE) p++;
            else if(d.tipo == C_FECHA) p--;
            else if(d.tipo == C_OP && (d.val == '+' || d.val == '-') && p == 0) somas++;
        }
        if(!somas){
            snprintf(porque, lim, "aqui não distribuo: dentro do grupo só há x, e o x NÃO "
                                  "distribui sobre o x. 2 x (3 x 4) é 24; distribuir ali daria "
                                  "(2x3) x (2x4) = 48, o dobro, porque o fator entraria duas "
                                  "vezes");
            return -1;
        }

        /* escreve: prefixo, ( F x t1 + F x t2 + ... ), sufixo */
        long o = 0;
        for(long k = 0; k < (fi < gi ? fi : gi); k++) ct_copia(fd, k, o++);
        ct_poe(fd, CT_OUT + o++, C_ABRE, '(');
        long ti = gi + 1, sinal = '+';                /* o sinal com que ESTE termo foi cortado */
        p = 0;
        for(long k = gi + 1; k <= gf; k++){
            Cel d = ct_le(fd, CT_FITA + k);
            if(d.tipo == C_ABRE) p++;
            else if(d.tipo == C_FECHA) p--;
            int corta = (k == gf) ||
                        (p == 0 && d.tipo == C_OP && (d.val == '+' || d.val == '-'));
            if(!corta) continue;
            long tf = k - 1;                              /* o termo é [ti .. tf] */
            if(ti > gi + 1) ct_poe(fd, CT_OUT + o++, C_OP, sinal);
            /* com a divisão o fator vai DEPOIS: (a+b)/c dá a/c + b/c, e não c/a + c/b */
            if(c.val != '/'){
                for(long q = fi; q <= ff; q++) ct_copia(fd, q, o++);
                ct_poe(fd, CT_OUT + o++, C_OP, '*');
            }
            /* termo composto leva parênteses — a não ser que JÁ seja um grupo fechado, e aí
             * pôr outro par por cima só suja o que o aluno lê: ((4 x 5)) em vez de (4 x 5). */
            int muitos = (tf > ti) &&
                         !(ct_le(fd, CT_FITA + ti).tipo == C_ABRE &&
                           ct_grupo_fim(fd, ti, n) == tf);
            if(muitos) ct_poe(fd, CT_OUT + o++, C_ABRE, '(');
            for(long q = ti; q <= tf; q++) ct_copia(fd, q, o++);
            if(muitos) ct_poe(fd, CT_OUT + o++, C_FECHA, ')');
            if(c.val == '/'){
                ct_poe(fd, CT_OUT + o++, C_OP, '/');
                for(long q = fi; q <= ff; q++) ct_copia(fd, q, o++);
            }
            if(d.tipo == C_OP) sinal = d.val;         /* o próximo termo herda este sinal */
            ti = k + 1;
        }
        ct_poe(fd, CT_OUT + o++, C_FECHA, ')');
        for(long k = (fi > gf ? ff : gf) + 1; k < n; k++) ct_copia(fd, k, o++);

        for(long k = 0; k < o; k++){                      /* a saída volta a ser a fita */
            Cel d = ct_le(fd, CT_OUT + k);
            ct_poe(fd, CT_FITA + k, d.tipo, d.val);
        }
        ct_poe(fd, CT_FITA + o, C_VAZIO, 0);
        snprintf(porque, lim, "distributiva: o fator entra em cada parcela (%ld parcelas)",
                 somas + 1);
        return o;
    }
    return 0;
}

/* FATORAR — o dual. Acha  F x A + F x B  com o MESMO fator numérico e devolve  F x (A + B). */
static long ct_fatora(int fd, long n, char *porque, size_t lim){
    for(long i = 0; i + 4 < n; i++){
        Cel f1 = ct_le(fd, CT_FITA + i), o1 = ct_le(fd, CT_FITA + i + 1);
        if(f1.tipo != C_NUM || o1.tipo != C_OP || o1.val != '*') continue;
        /* o primeiro termo depois do x */
        long ai = i + 2, af = ai;
        Cel a = ct_le(fd, CT_FITA + ai);
        if(a.tipo == C_ABRE) af = ct_grupo_fim(fd, ai, n);
        else if(a.tipo != C_NUM) continue;
        if(af < 0) continue;
        Cel mais = ct_le(fd, CT_FITA + af + 1);
        if(mais.tipo != C_OP || mais.val != '+') continue;
        Cel f2 = ct_le(fd, CT_FITA + af + 2), o2 = ct_le(fd, CT_FITA + af + 3);
        if(f2.tipo != C_NUM || f2.val != f1.val) continue;     /* o MESMO fator */
        if(o2.tipo != C_OP || o2.val != '*') continue;
        long bi = af + 4, bf = bi;
        Cel b = ct_le(fd, CT_FITA + bi);
        if(b.tipo == C_ABRE) bf = ct_grupo_fim(fd, bi, n);
        else if(b.tipo != C_NUM) continue;
        if(bf < 0) continue;

        long o = 0;
        for(long k = 0; k < i; k++) ct_copia(fd, k, o++);
        ct_poe(fd, CT_OUT + o++, C_NUM,  f1.val);
        ct_poe(fd, CT_OUT + o++, C_OP,   '*');
        ct_poe(fd, CT_OUT + o++, C_ABRE, '(');
        for(long k = ai; k <= af; k++) ct_copia(fd, k, o++);
        ct_poe(fd, CT_OUT + o++, C_OP,   '+');
        for(long k = bi; k <= bf; k++) ct_copia(fd, k, o++);
        ct_poe(fd, CT_OUT + o++, C_FECHA, ')');
        for(long k = bf + 1; k < n; k++) ct_copia(fd, k, o++);

        for(long k = 0; k < o; k++){
            Cel d = ct_le(fd, CT_OUT + k);
            ct_poe(fd, CT_FITA + k, d.tipo, d.val);
        }
        ct_poe(fd, CT_FITA + o, C_VAZIO, 0);
        snprintf(porque, lim, "fatorar: o %ld é comum às duas parcelas, e sai para fora",
                 f1.val);
        return o;
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
