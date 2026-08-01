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
#define C_RAIZ  5            /* a raiz quadrada, unária e prefixa */
#define C_FAT   6            /* o fatorial, unário e PÓSFIXO — o único assim */

/* A CÉLULA GANHOU DENOMINADOR — e com ele a máquina deixou de ser Z e passou a ser Q.
 *
 * Não houve sintaxe nova: o '/' já era lido como operador, e o que mudou foi que ele deixa de
 * PARAR e passa a construir. Uma fração é um par (p,q) com q != 0, e a igualdade não é de
 * pares: 2/4 e 1/2 são o MESMO número porque 2x2 = 4x1. É classe de equivalência, e o
 * representante canónico obtém-se por EUCLIDES — o mesmo algoritmo que gera a cifra do rei.
 *
 * O inteiro é o caso den = 1, e não um tipo à parte. */
typedef struct { long tipo, val, den; } Cel;
#define CS ((long)sizeof(Cel))

#define CT_FITA  0           /* a fita começa na célula 0 */
#define CT_PILHA 4096        /* a pilha do emparelhamento, à frente */
#define CT_OUT   8192        /* a fita de saída, para a reescrita distributiva */

static Cel ct_le(int fd, long i){ Cel c = {0,0,1}; pread(fd, &c, CS, i*CS); return c; }
static void ct_poeq(int fd, long i, long t, long v, long d){
    Cel c = {t,v,d}; pwrite(fd, &c, CS, i*CS);
}
static void ct_poe(int fd, long i, long t, long v){ ct_poeq(fd, i, t, v, 1); }

/* EUCLIDES — o mesmo que gera a cifra, aqui a reduzir a fração ao representante canónico. */
static long ct_mdc(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
/* a forma reduzida, com o sinal sempre no numerador */
static void ct_reduz(long *p, long *q){
    if(*q < 0){ *p = -*p; *q = -*q; }
    long g = ct_mdc(*p, *q);
    *p /= g; *q /= g;
}
static int ct_mulcabe(long a, long b){
    if(!a || !b) return 1;
    long A = a < 0 ? -a : a, B = b < 0 ? -b : b;
    return A <= 4611686018427387903L / B;
}

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
            /* e depois de uma RAIZ também: sem C_RAIZ nesta lista, "raiz -4" lia-se como
             * "raiz menos 4" e a raiz ficava sem operando — silêncio em vez de recusa. */
            if(!n || ant.tipo == C_OP || ant.tipo == C_ABRE || ant.tipo == C_RAIZ){
                const char *q = s + 1;
                while(*q == ' ') q++;
                if(*q >= '0' && *q <= '9'){
                    long v = 0;
                    while(*q >= '0' && *q <= '9'){ v = v*10 + (*q - '0'); q++; }
                    ct_poe(fd, CT_FITA + n++, C_NUM, -v); s = q; continue;
                }
            }
        }
        if(!strncmp(s, "raiz", 4)){ ct_poe(fd, CT_FITA + n++, C_RAIZ, 0); s += 4; continue; }
        if(!strncmp(s, "mod", 3)){ ct_poe(fd, CT_FITA + n++, C_OP, 'm'); s += 3; continue; }
        if(*s == '%'){ ct_poe(fd, CT_FITA + n++, C_OP, 'm'); s++; continue; }
        if(*s == '!'){ ct_poe(fd, CT_FITA + n++, C_FAT, 0); s++; continue; }
        if(*s == '^'){ ct_poe(fd, CT_FITA + n++, C_OP, '^'); s++; continue; }
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
            /* a fração escreve-se sem espaços — "7/2" — para não se confundir com a divisão
             * por fazer, que leva espaços: "7 / 2". A vista distingue o valor da operação. */
            if(c.den != 1 && c.den != 0){
                if(c.val < 0 && op_antes) snprintf(b, sizeof b, "(%ld/%ld)", c.val, c.den);
                else                      snprintf(b, sizeof b, "%ld/%ld", c.val, c.den);
            }
            else if(c.val < 0 && op_antes) snprintf(b, sizeof b, "(%ld)", c.val);
            else                          snprintf(b, sizeof b, "%ld", c.val);
        }
        else if(c.tipo == C_RAIZ) snprintf(b, sizeof b, "raiz ");
        else if(c.tipo == C_FAT)  snprintf(b, sizeof b, "!");
        else if(c.tipo == C_OP){
            if(c.val == 'm')      snprintf(b, sizeof b, " mod ");
            else                  snprintf(b, sizeof b, " %c ", c.val == '*' ? 'x' : (char)c.val);
        }
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
    /* PASSADA 0: O FATORIAL, unário e PÓSFIXO — o único operador assim em toda a máquina.
     * Liga mais forte que tudo: 2^3! é 2^6 = 64, e não (2^3)! . E 3!! é (3!)! = 720, porque
     * o pósfixo associa à esquerda sem ter de se decidir nada. */
    for(long i = ini; i < fim; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo != C_FAT) continue;
        long e = ct_ante(fd, i);
        if(e < 0) continue;
        Cel A = ct_le(fd, CT_FITA + e);
        if(A.tipo != C_NUM) continue;
        if(A.den && A.den != 1){
            snprintf(porque, lim, "o fatorial de %ld/%ld não é desta conta: ele conta arranjos, "
                     "e de uma fração de coisas não há arranjo. A função gama estende-o, e aí "
                     "%ld/%ld! deixa de ser um produto e passa a ser um integral", A.val, A.den,
                     A.val, A.den);
            return -1;
        }
        if(A.val < 0){
            snprintf(porque, lim, "%ld! não existe: o fatorial conta arranjos, e de um número "
                     "negativo de coisas não há nenhum. A função gama estende-o aos não "
                     "inteiros, e nos inteiros negativos tem polo — não é lacuna de definição, "
                     "é infinito lá", A.val);
            return -1;
        }
        long r = 1, mau = 0;
        for(long k = 2; k <= A.val && !mau; k++){
            if(r > 4611686018427387903L / k) mau = 1; else r *= k;
        }
        if(mau){
            snprintf(porque, lim, "%ld! não cabe num inteiro da máquina — 20! ainda cabe e 21! "
                     "já não. O número existe, a caixa é que acaba", A.val);
            return -1;
        }
        if(A.val <= 1)
            snprintf(porque, lim, "%ld! = 1   (é o produto VAZIO, o neutro da multiplicação; e a "
                     "recursão obriga: n! = n x (n-1)! com n = 1 dá 1! = 1 x 0!)", A.val);
        else
            snprintf(porque, lim, "%ld! = %ld", A.val, r);
        ct_poe(fd, CT_FITA + e, C_NUM, r);
        ct_poe(fd, CT_FITA + i, C_VAZIO, 0);
        return 1;
    }
    /* PASSADA 0: A RAIZ, unária, agarra o operando imediato. Vem antes da potência para que
     * "raiz 4 ^ 2" se leia da esquerda: (raiz 4)^2. Quem quiser raiz(4^2) põe parênteses. */
    for(long i = ini; i < fim; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo != C_RAIZ) continue;
        long d = ct_prox(fd, i, n);
        if(d < 0) continue;
        Cel B = ct_le(fd, CT_FITA + d);
        if(B.tipo != C_NUM) continue;
        if(B.den && B.den != 1){
            /* raiz de fração: fecha se em cima e em baixo fecharem. raiz(4/9) = 2/3. */
            long p = B.val, q = B.den;
            if(p < 0){
                snprintf(porque, lim, "a raiz de %ld/%ld não existe em R: o numerador é negativo",
                         p, q);
                return -1;
            }
            long rp = 0, rq = 0;
            while((rp+1)*(rp+1) <= p) rp++;
            while((rq+1)*(rq+1) <= q) rq++;
            if(rp*rp != p || rq*rq != q){
                snprintf(porque, lim, "a raiz de %ld/%ld não fecha em Q: para fechar, o "
                         "numerador e o denominador da forma reduzida têm de ser AMBOS quadrados "
                         "perfeitos, e aqui %s não é", p, q, rp*rp != p ? "o de cima" : "o de baixo");
                return -1;
            }
            snprintf(porque, lim, "raiz de %ld/%ld = %ld/%ld   (fecha porque %ld e %ld são "
                     "quadrados; e -%ld/%ld também é raiz)", p, q, rp, rq, p, q, rp, rq);
            ct_poeq(fd, CT_FITA + i, C_NUM, rp, rq);
            ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
            return 1;
        }
        if(B.val < 0){
            snprintf(porque, lim, "a raiz de %ld não existe em R: nenhum real ao quadrado dá "
                     "negativo. Em C existe, e é aí que ela mora", B.val);
            return -1;
        }
        long r = 0; while((r+1)*(r+1) <= B.val) r++;
        if(r*r != B.val){
            /* E AQUI O CORPUS CIENTIFICO ENCOSTA: irracional e RELATIVO a Q. A raiz de 2 nao
             * esta em Z nem em Q, e esta em Z/7, onde 3x3 = 9 = 2. Dizer so "e irracional"
             * seria o mesmo absoluto que ja apanhei no corpus. */
            const char *onde = B.val == 2 ? " Mas em Z/7 existe: 3 x 3 = 9 = 2. Irracional é"
                                            " relativo a Q, e não uma propriedade do número."
                              : B.val == 3 ? " Em Z/11 existe: 5 x 5 = 25 = 3." : "";
            /* E AQUI ESTÁ O PONTO: a máquina JÁ ESTÁ em Q — o 7/2 fecha, o 2^-1 fecha — e a
             * raiz de 2 continua a não fechar. Não é falta de alcance da máquina: é o que
             * torna a raiz de 2 IRRACIONAL, e irracional quer dizer exatamente isto, fora
             * de Q. A prova é a de sempre: p² = 2q² faria p e q ambos pares, e a forma
             * reduzida não pode ter os dois pares. */
            snprintf(porque, lim, "a raiz de %ld não fecha em Z nem em Q: %ld ao quadrado é %ld, "
                     "%ld ao quadrado é %ld, e não há inteiro no meio; e em Q também não, porque "
                     "p² = %ldq² faria p e q ambos pares e a forma reduzida não pode. É isso que "
                     "quer dizer IRRACIONAL — fora de Q.%s", B.val, r, r*r, r+1, (r+1)*(r+1),
                     B.val, onde);
            return -1;
        }
        snprintf(porque, lim, "raiz de %ld = %ld   (e -%ld também: a raiz principal é uma "
                 "escolha, não um facto)", B.val, r, r);
        ct_poe(fd, CT_FITA + i, C_NUM, r);
        ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
        return 1;
    }
    /* PASSADA 1: A POTÊNCIA, e ela associa à DIREITA — ao contrário de tudo o resto aqui.
     * 2^3^2 é 2^(3^2) = 512, e não (2^3)^2 = 64. A varredura é a MESMA, no sentido contrário:
     * onde a subtração dobra o primeiro da esquerda, a potência dobra o último da direita. */
    for(long i = fim - 1; i >= ini; i--){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo != C_OP || c.val != '^') continue;
        long e = ct_ante(fd, i), d = ct_prox(fd, i, n);
        if(e < 0 || d < 0) continue;
        Cel A = ct_le(fd, CT_FITA + e), B = ct_le(fd, CT_FITA + d);
        if(A.tipo != C_NUM || B.tipo != C_NUM) continue;
        if(B.den && B.den != 1){
            snprintf(porque, lim, "o expoente fracionário é outra coisa: %ld elevado a %ld/%ld "
                     "é a raiz de índice %ld, e essa quase nunca fecha em Q — é por aí que se "
                     "sai de Q para os reais", A.val, B.val, B.den, B.den);
            return -1;
        }
        {   /* EXPOENTE NEGATIVO: em Z não fechava, em Q fecha — é o inverso, e o inverso é
             * exatamente o dual que Q tem e Z não tinha. */
            long pa0 = A.val, qa0 = A.den ? A.den : 1;
            if(B.val < 0){
                long pot = -B.val, rp = 1, rq = 1, mau = 0;
                for(long k = 0; k < pot && !mau; k++){
                    if(!ct_mulcabe(rp, pa0) || !ct_mulcabe(rq, qa0)) mau = 1;
                    else { rp *= pa0; rq *= qa0; }
                }
                if(pa0 == 0){
                    snprintf(porque, lim, "0 elevado a expoente negativo é 1 sobre 0, e isso não "
                             "existe em corpo nenhum");
                    return -1;
                }
                if(mau){
                    snprintf(porque, lim, "não cabe num inteiro da máquina");
                    return -1;
                }
                long np = rq, nq = rp; ct_reduz(&np, &nq);
                char e1[48], e3[48];
                if(qa0 != 1) snprintf(e1, sizeof e1, "%ld/%ld", pa0, qa0);
                else         snprintf(e1, sizeof e1, "%ld", pa0);
                if(nq != 1)  snprintf(e3, sizeof e3, "%ld/%ld", np, nq);
                else         snprintf(e3, sizeof e3, "%ld", np);
                snprintf(porque, lim, "%s elevado a %ld = %s   (em Z não fechava; o expoente "
                         "negativo pede o INVERSO, e é o inverso que Q tem e Z não tinha)",
                         e1, B.val, e3);
                ct_poeq(fd, CT_FITA + e, C_NUM, np, nq);
                ct_poe(fd, CT_FITA + i, C_VAZIO, 0);
                ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
                return 1;
            }
            if(qa0 != 1){                       /* (p/q)^n = p^n/q^n */
                long rp = 1, rq = 1, mau = 0;
                for(long k = 0; k < B.val && !mau; k++){
                    if(!ct_mulcabe(rp, pa0) || !ct_mulcabe(rq, qa0)) mau = 1;
                    else { rp *= pa0; rq *= qa0; }
                }
                if(mau){ snprintf(porque, lim, "não cabe num inteiro da máquina"); return -1; }
                ct_reduz(&rp, &rq);
                snprintf(porque, lim, "%ld/%ld elevado a %ld = %ld/%ld   (a potência entra em "
                         "cima e em baixo)", pa0, qa0, B.val, rp, rq);
                ct_poeq(fd, CT_FITA + e, C_NUM, rp, rq);
                ct_poe(fd, CT_FITA + i, C_VAZIO, 0);
                ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
                return 1;
            }
        }
        if(A.val == 0 && B.val == 0){
            snprintf(porque, lim, "0 elevado a 0 não tem resposta única: é 1 na combinatória e "
                     "nas séries, porque conta a função vazia, e indefinido na análise, porque "
                     "o limite depende do caminho. Depende do que se está a fazer");
            return -1;
        }
        long r = 1, mau = 0, ab = A.val < 0 ? -A.val : A.val;
        for(long k = 0; k < B.val && !mau; k++){
            if(ab > 1 && (r > 4611686018427387903L / ab)) mau = 1;
            else r *= A.val;
        }
        if(mau){
            snprintf(porque, lim, "%ld elevado a %ld não cabe num inteiro da máquina — o número "
                     "existe, a caixa é que acaba, e isso é da máquina e não da matemática",
                     A.val, B.val);
            return -1;
        }
        snprintf(porque, lim, "%ld elevado a %ld = %ld", A.val, B.val, r);
        ct_poe(fd, CT_FITA + e, C_NUM, r);
        ct_poe(fd, CT_FITA + i, C_VAZIO, 0);
        ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
        return 1;
    }
    for(int passada = 0; passada < 2; passada++){
        const char *conj = passada == 0 ? "*/m" : "+-";
        for(long i = ini; i < fim; i++){
            Cel c = ct_le(fd, CT_FITA + i);
            if(c.tipo != C_OP || !strchr(conj, (int)c.val)) continue;
            long e = ct_ante(fd, i), d = ct_prox(fd, i, n);
            if(e < 0 || d < 0) continue;
            Cel A = ct_le(fd, CT_FITA + e), B = ct_le(fd, CT_FITA + d);
            if(A.tipo != C_NUM || B.tipo != C_NUM) continue;
            long quero = c.val, r;
            if(quero == 'm'){
                if((A.den && A.den != 1) || (B.den && B.den != 1)){
                    snprintf(porque, lim, "o resto é de INTEIROS: Z/n parte a reta dos inteiros "
                             "em n classes, e uma fração não vive lá. Em Q não há resto porque "
                             "toda a divisão já fecha — o resto é o que sobra de não fechar");
                    return -1;
                }
                /* O MÓDULO É O Z/n DO CORPUS, e é aqui que os dois lados do sistema se
                 * encontram: onde a divisão PARAVA por não fechar em Z, o resto diz o que
                 * sobra — e juntos são a divisão euclidiana.
                 *
                 * E O SINAL É UMA CONVENÇÃO, que se declara em vez de se herdar: fica-se com
                 * o representante NÃO NEGATIVO, que é o canónico de Z/n. O C trunca para zero
                 * e daria -7 mod 3 = -1; aqui dá 2, como em Python e como na aritmética
                 * modular. As duas respostas são o MESMO elemento de Z/3 — mudam de roupa,
                 * não de classe — mas escolher e não dizer é que seria o erro. */
                if(B.val == 0){
                    snprintf(porque, lim, "%ld mod 0 não existe: Z/0 não parte a reta em classe "
                             "nenhuma, e não há resto de uma divisão que não existe", A.val);
                    return -1;
                }
                long m = B.val < 0 ? -B.val : B.val;
                r = A.val % m; if(r < 0) r += m;
                snprintf(porque, lim, "%ld mod %ld = %ld   (o representante não negativo de "
                         "Z/%ld; em C, que trunca, daria %ld)", A.val, B.val, r, m,
                         A.val % B.val);
                ct_poe(fd, CT_FITA + e, C_NUM, r);
                ct_poe(fd, CT_FITA + i, C_VAZIO, 0);
                ct_poe(fd, CT_FITA + d, C_VAZIO, 0);
                return 1;
            }
            /* AS QUATRO OPERAÇÕES EM Q. O inteiro é o caso den = 1, e não um ramo à parte:
             * escrever a regra racional é escrever também a inteira, e uma só é preciso. */
            long pa = A.val, qa = A.den ? A.den : 1, pb = B.val, qb = B.den ? B.den : 1;
            long rp, rq;
            if(quero == '/'){
                if(pb == 0){
                    snprintf(porque, lim, "%ld a dividir por 0 não existe em corpo nenhum: se "
                             "0 vezes x fosse 1, então 0 = 1 e a estrutura colapsava", pa);
                    return -1;
                }
                if(!ct_mulcabe(pa, qb) || !ct_mulcabe(qa, pb)){
                    snprintf(porque, lim, "esta divisão dá números que não cabem num inteiro da "
                             "máquina — o valor existe, a caixa é que acaba");
                    return -1;
                }
                rp = pa * qb; rq = qa * pb;
            }
            else if(quero == '*'){
                if(!ct_mulcabe(pa, pb) || !ct_mulcabe(qa, qb)){
                    snprintf(porque, lim, "este produto não cabe num inteiro da máquina");
                    return -1;
                }
                rp = pa * pb; rq = qa * qb;
            }
            else {
                /* a soma pede denominador comum, e o comum é o produto reduzido pelo mdc */
                long g = ct_mdc(qa, qb), m1 = qb / g;
                if(!ct_mulcabe(qa, m1) || !ct_mulcabe(pa, m1) || !ct_mulcabe(pb, qa/g)){
                    snprintf(porque, lim, "esta soma pede um denominador que não cabe na máquina");
                    return -1;
                }
                rq = qa * m1;
                long t1 = pa * m1, t2 = pb * (qa / g);
                rp = quero == '-' ? t1 - t2 : t1 + t2;
            }
            ct_reduz(&rp, &rq);
            {
                char e1[48], e2[48], e3[48];
                if(qa != 1) snprintf(e1, sizeof e1, "%ld/%ld", pa, qa); else snprintf(e1, sizeof e1, "%ld", pa);
                if(qb != 1) snprintf(e2, sizeof e2, "%ld/%ld", pb, qb); else snprintf(e2, sizeof e2, "%ld", pb);
                if(rq != 1) snprintf(e3, sizeof e3, "%ld/%ld", rp, rq); else snprintf(e3, sizeof e3, "%ld", rp);
                /* quando a operação SAI de Z, diz-se — porque foi o corpo que mudou, e o
                 * corpo é a única coisa que este sistema nunca deixa implícita. */
                int saiu = rq != 1 && qa == 1 && qb == 1;
                if(saiu && quero == '/')
                    snprintf(porque, lim, "%s / %s = %s   (em Z não fechava; em Q fecha, e é uma "
                             "CLASSE: %ld/%ld é o mesmo que %ld/%ld)", e1, e2, e3, rp, rq,
                             rp*2, rq*2);
                else {
                    /* voltou a Z: entrou fração e saiu inteiro. Vale a pena dizer, porque é a
                     * prova de que Z está DENTRO de Q e não ao lado — 1/2 + 1/2 é 1, e o 1 é o
                     * mesmo 1 dos dois lados. */
                    const char *nota = ((qa != 1 || qb != 1) && rq == 1)
                                       ? "   (e fechou de volta em Z: Z está DENTRO de Q)" : "";
                    snprintf(porque, lim, "%s %c %s = %s%s", e1, quero=='*' ? 'x' : (char)quero,
                             e2, e3, nota);
                }
            }
            ct_poeq(fd, CT_FITA + e, C_NUM, rp, rq);
            ct_poe(fd, CT_FITA + i, C_VAZIO, 0);
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
        /* A POTÊNCIA SOBRE O GRUPO — e aqui está o erro mais comum que há.
         *
         * (a x b)^n = a^n x b^n   SIM, porque a potência é multiplicação repetida e o produto
         *                          comuta: os fatores separam-se e cada um leva o expoente.
         * (a + b)^n = a^n + b^n   NÃO. (2+3)^2 é 25 e 2^2 + 3^2 é 13. O que falta é o termo
         *                          cruzado — o 2ab, que aqui vale 12 — e ele não desaparece
         *                          por se querer que desapareça.
         *
         * Distribui-se o primeiro caso e recusa-se o segundo, com os números à vista. */
        if(c.val == '^'){
            Cel esq0 = ct_le(fd, CT_FITA + i - 1);
            if(esq0.tipo != C_FECHA) continue;
            long g1 = ct_fator_ini(fd, i - 1), g2 = i - 1;
            if(g1 < 0) continue;
            long pp = 0, mais = 0, vezes = 0;
            for(long k = g1 + 1; k < g2; k++){
                Cel d = ct_le(fd, CT_FITA + k);
                if(d.tipo == C_ABRE) pp++;
                else if(d.tipo == C_FECHA) pp--;
                else if(d.tipo == C_OP && pp == 0){
                    if(d.val == '+' || d.val == '-') mais++;
                    else if(d.val == '*') vezes++;
                }
            }
            if(mais){
                snprintf(porque, lim, "aqui não distribuo, e este é o engano mais comum que há: "
                         "(a+b) elevado a n NÃO é a^n + b^n. (2+3)^2 é 25 e 2^2 + 3^2 é 13 — "
                         "falta o termo cruzado, o 2ab, que ali vale 12 e não desaparece por se "
                         "querer que desapareça");
                return -1;
            }
            if(!vezes) continue;
            /* (a x b)^n -> a^n x b^n : cada fator leva o expoente */
            long ei = i + 1, ef = ei;
            Cel ex = ct_le(fd, CT_FITA + ei);
            if(ex.tipo == C_ABRE) ef = ct_grupo_fim(fd, ei, n);
            else if(ex.tipo != C_NUM) continue;
            if(ef < 0) continue;
            long o = 0;
            for(long k = 0; k < g1; k++) ct_copia(fd, k, o++);
            ct_poe(fd, CT_OUT + o++, C_ABRE, '(');
            long fi2 = g1 + 1; pp = 0;
            for(long k = g1 + 1; k <= g2; k++){
                Cel d = ct_le(fd, CT_FITA + k);
                if(d.tipo == C_ABRE) pp++;
                else if(d.tipo == C_FECHA) pp--;
                if(!((k == g2) || (pp == 0 && d.tipo == C_OP && d.val == '*'))) continue;
                long ff2 = k - 1;
                if(fi2 > g1 + 1) ct_poe(fd, CT_OUT + o++, C_OP, '*');
                int mm = (ff2 > fi2);
                if(mm) ct_poe(fd, CT_OUT + o++, C_ABRE, '(');
                for(long q = fi2; q <= ff2; q++) ct_copia(fd, q, o++);
                if(mm) ct_poe(fd, CT_OUT + o++, C_FECHA, ')');
                ct_poe(fd, CT_OUT + o++, C_OP, '^');
                for(long q = ei; q <= ef; q++) ct_copia(fd, q, o++);
                fi2 = k + 1;
            }
            ct_poe(fd, CT_OUT + o++, C_FECHA, ')');
            for(long k = ef + 1; k < n; k++) ct_copia(fd, k, o++);
            for(long k = 0; k < o; k++){
                Cel d = ct_le(fd, CT_OUT + k);
                ct_poe(fd, CT_FITA + k, d.tipo, d.val);
            }
            ct_poe(fd, CT_FITA + o, C_VAZIO, 0);
            snprintf(porque, lim, "a potência entra em cada FATOR: (a x b)^n = a^n x b^n, e vale "
                     "porque o produto comuta");
            return o;
        }
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
/* o valor final. Devolve numerador E denominador — sem o segundo, "7/2" saía como "7", que
 * é a resposta errada escrita na linha que mais conta. */
static int ct_valorq(int fd, long n, long *p, long *q){
    long v = 0, d = 1, quantos = 0;
    for(long i = 0; i < n; i++){
        Cel c = ct_le(fd, CT_FITA + i);
        if(c.tipo == C_VAZIO) continue;
        if(c.tipo != C_NUM) return 0;
        v = c.val; d = c.den ? c.den : 1; quantos++;
    }
    if(quantos != 1) return 0;
    *p = v; *q = d; return 1;
}
static int ct_valor(int fd, long n, long *out){
    long p, q;
    if(!ct_valorq(fd, n, &p, &q) || q != 1) return 0;   /* só fecha em Z se o den for 1 */
    *out = p; return 1;
}
/* a resposta escrita: "7/2" ou "3" */
static void ct_escreve(long p, long q, char *out, size_t lim){
    if(q == 1) snprintf(out, lim, "%ld", p);
    else       snprintf(out, lim, "%ld/%ld", p, q);
}

#endif
