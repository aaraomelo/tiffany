/* booleana.h — A LÓGICA É O CORPO GF(2), E NÃO UMA MÁQUINA À PARTE.
 *
 * O `conversa.c` já o dizia no ramo do transístor: «fora da janela, o contínuo colapsa
 * em GF(2) — AND É a MULTIPLICAÇÃO de GF(2), XOR É a SOMA, e NOT é uma dobra de ordem
 * 2, como o conj e o J; em GF(2) vale −x = x, logo somar É subtrair, e é por isso que
 * o XOR é reversível de graça». Aqui isso deixa de ser uma frase num ramo e passa a ser
 * o corpo onde a lógica se resolve — com as MESMAS cinco operações:
 *
 *   ⊕ SOMA           o XOR (e a subtração é ela própria: −x = x)
 *   ⊗ MULTIPLICAÇÃO  o AND (e x⊗x = x: o idempotente é a lei booleana)
 *   ÷ DIVISÃO        a FIBRA: dada a saída, quais as entradas — é resolver a equação
 *   † DUAL           a ANF (Zhegalkin) pela transformada de Möbius, que é INVOLUÇÃO
 *   ↺ INVERSÃO       a volta: substituir a solução e obter o valor pedido, resíduo 0
 *
 * E a EQUAÇÃO reduz-se como a polinomial: `A = B` é `A ⊕ B = 0`, porque em GF(2) a
 * igualdade é o XOR a zerar. Uma lei, e não dois casos.
 *
 * Tudo exaustivo e exato: com n variáveis há 2ⁿ atribuições, e percorrem-se TODAS —
 * não há amostragem nem heurística, e o corpo é finito por construção. */
#ifndef BOOLEANA_H
#define BOOLEANA_H
#include <string.h>

#define BL_VAR 10                     /* até 10 variáveis: 1024 linhas, e é dito */
#define BL_MAX (1 << BL_VAR)

typedef struct {
    char nome[BL_VAR];                /* as variáveis, por ordem de aparição */
    int  nv;
    unsigned char t[BL_MAX];          /* a tabela: t[x] = valor na atribuição x */
    int  bom;
} Bool;

/* ─── o leitor: descida recursiva sobre a fala ───────────────────────────────────
 * A precedência é a do corpo: NOT (a dobra) primeiro, depois ⊗ (AND), depois ⊕ (XOR)
 * e o OU — que não é primitivo: a∨b = a ⊕ b ⊕ ab, e é assim que ele entra. */
typedef struct { const char *s; Bool *b; unsigned x; int erro; } BlCtx;

static int bl_indice(Bool *b, char c){
    for(int k = 0; k < b->nv; k++) if(b->nome[k] == c) return k;
    if(b->nv >= BL_VAR) return -1;
    b->nome[b->nv] = c;
    return b->nv++;
}
static void bl_esp(BlCtx *c){ while(*c->s == ' ') c->s++; }
static int bl_xor(BlCtx *c);
static int bl_implica(BlCtx *c);

static int bl_atomo(BlCtx *c){
    bl_esp(c);
    if(!strncmp(c->s, "nao ", 4)){ c->s += 4; return !bl_atomo(c); }
    if(!strncmp(c->s, "não ", 5)){ c->s += 5; return !bl_atomo(c); }
    if(*c->s == '!' || *c->s == '~'){ c->s++; return !bl_atomo(c); }
    if(*c->s == '('){
        c->s++;
        int v = bl_implica(c);
        bl_esp(c);
        if(*c->s == ')') c->s++; else c->erro = 1;
        return v;
    }
    int v;
    if(*c->s == '0'){ c->s++; v = 0; }
    else if(*c->s == '1'){ c->s++; v = 1; }
    else if(*c->s >= 'a' && *c->s <= 'z'){
        char nome = *c->s++;
        int i = bl_indice(c->b, nome);
        if(i < 0){ c->erro = 1; return 0; }
        v = (c->x >> i) & 1;
    }
    else { c->erro = 1; return 0; }
    /* O COMPLEMENTO POSFIXO: `y'` é ¬y. É a notação do ficheiro — «F = AB + AB'» —, e
     * sem ela a casa escrevia uma coisa e lia outra. */
    while(*c->s == '\''){ c->s++; v = !v; }
    return v;
}
static int bl_e(BlCtx *c){                          /* ⊗ — o AND */
    int v = bl_atomo(c);
    for(;;){
        const char *antes = c->s;
        bl_esp(c);
        if(*c->s == '*' || *c->s == '.' || *c->s == '&'){ c->s++; v = bl_atomo(c) && v; }
        else if(!strncmp(c->s, "e ", 2)){ c->s += 2; v = bl_atomo(c) && v; }
        /* A JUSTAPOSIÇÃO É O PRODUTO — «AB», «AB'», «x(y+z)» —, e SÓ SEM ESPAÇO. É o
         * espaço que decide: com ele, o `o` de «a ou b» seria lido como variável e o
         * OU virava produto. Sem espaço não há ambiguidade nenhuma; com espaço, o
         * operador tem de vir escrito. */
        else if(c->s == antes && (*c->s == '(' || *c->s == '!' || *c->s == '~'
                                  || (*c->s >= 'a' && *c->s <= 'z')
                                  || *c->s == '0' || *c->s == '1'))
            v = bl_atomo(c) && v;
        else { c->s = antes; break; }
    }
    return v;
}
/* A NOTAÇÃO É A DO `eval.txt`, E DIZ-SE: ali o `+` é o OU («A + AB = A», «F = AB +
 * AB'»), que é a convenção da álgebra booleana e dos circuitos. O XOR tem sinal
 * próprio: `^` ou a palavra `xor`. Ler o `+` como XOR dava «a + ab = a·b̄» — certo em
 * GF(2) e ERRADO no ficheiro que manda aqui; e um `+` ambíguo é pior que os dois.
 * Por dentro o corpo continua GF(2): a∨b = a ⊕ b ⊕ ab, e é assim que o OU entra. */
static int bl_ou(BlCtx *c){
    int v = bl_e(c);
    for(;;){
        bl_esp(c);
        if(*c->s == '+'){ c->s++; int w = bl_e(c); v = v | w; }        /* + é o OU */
        else if(*c->s == '|'){ c->s++; int w = bl_e(c); v = v | w; }
        else if(!strncmp(c->s, "ou ", 3)){ c->s += 3; int w = bl_e(c); v = v | w; }
        else if(*c->s == '^'){ c->s++; v ^= bl_e(c); }                 /* ^ é o XOR */
        else if(!strncmp(c->s, "xor ", 4)){ c->s += 4; v ^= bl_e(c); }
        else break;
    }
    return v;
}
/* A IMPLICAÇÃO E O BICONDICIONAL — e a implicação NÃO é primitiva: é o que o
 * ficheiro diz logo no problema 1, «P → Q ≡ ¬P ∨ Q». Entra por essa identidade, e
 * não por uma tabela escrita à mão: a definição é a lei. O bicondicional é a
 * conjunção das duas implicações — e é por isso que ele é a IGUALDADE do corpo. */
static int bl_implica(BlCtx *c){
    int v = bl_ou(c);
    for(;;){
        bl_esp(c);
        if(!strncmp(c->s, "<->", 3)){ c->s += 3; int w = bl_ou(c); v = (v == w); }
        else if(!strncmp(c->s, "->", 2)){ c->s += 2; int w = bl_ou(c); v = (!v) | w; }
        else if(!strncmp(c->s, "implica ", 8)){ c->s += 8; int w = bl_ou(c); v = (!v) | w; }
        else if(!strncmp(c->s, "sse ", 4)){ c->s += 4; int w = bl_ou(c); v = (v == w); }
        else break;
    }
    return v;
}
static int bl_xor(BlCtx *c){ return bl_implica(c); }  /* a porta de entrada, um nome só */
/* lê a expressão e enche a tabela — exaustivo nas 2ⁿ atribuições */
static int bl_le(const char *s, Bool *b){
    memset(b, 0, sizeof *b);
    b->bom = 0;
    { BlCtx c = { s, b, 0, 0 };                     /* uma passagem só para os nomes */
      bl_xor(&c);
      bl_esp(&c);
      if(c.erro || *c.s) return 0; }
    if(b->nv == 0) b->nv = 0;
    unsigned n = 1u << b->nv;
    for(unsigned x = 0; x < n; x++){
        BlCtx c = { s, b, x, 0 };
        int v = bl_xor(&c);
        if(c.erro) return 0;
        b->t[x] = (unsigned char)(v & 1);
    }
    b->bom = 1;
    return 1;
}
/* ─── O DUAL: a ANF (Zhegalkin) pela transformada de Möbius, que é INVOLUÇÃO ──────
 * A mesma transformada leva a tabela na ANF e a ANF na tabela — ν∘ν = id, sem
 * inversa separada. É a Lei 1 no corpo de característica 2, e é de graça porque
 * somar É subtrair. */
static void bl_mobius(unsigned char *t, int nv){
    unsigned n = 1u << nv;
    for(int i = 0; i < nv; i++)
        for(unsigned x = 0; x < n; x++)
            if(x & (1u << i)) t[x] ^= t[x ^ (1u << i)];
}
/* avalia a ANF numa atribuição: soma dos monómios cujo suporte cabe em x */
static int bl_val_anf(const unsigned char *anf, int nv, unsigned x){
    unsigned n = 1u << nv;
    int v = 0;
    for(unsigned m = 0; m < n; m++)
        if(anf[m] && (m & x) == m) v ^= 1;
    return v;
}
/* ─── OS CONJUNTOS: a pertença é a variável, e a prova é a tabela ────────────────
 *
 * O `eval.txt` dá a ponte inteira e diz para que serve:
 *
 *     conjuntos ↔ Booleano ↔ árvore ↔ prova
 *
 * «o conjunto é uma representação semântica; a árvore é a discretização; o rastro é a
 * demonstração; a volta é a prova de que a transformação preservou o objeto.»
 *
 * A tradução é a definição, não um truque: `x ∈ A` É a variável booleana `a`, e daí
 *     A ∪ B ↦ a + b        (a união é o OU da pertença)
 *     A ∩ B ↦ a * b        (a interseção é o E)
 *     A \ B ↦ a * ¬b       (a diferença é o E com a negação — a definição, à letra)
 *     Aᶜ    ↦ ¬a           (o complemento é a negação)
 *     A △ B ↦ a ^ b        (a diferença simétrica É o XOR: o próprio ficheiro o diz)
 *     A ⊆ B ↦ a → b        (a inclusão é a implicação da pertença, ∀x)
 *     ∅ ↦ 0    U ↦ 1
 *
 * E então provar `A = B` é provar que as duas pertenças têm a MESMA tabela — o que o
 * ficheiro chama «provar igualdade das folhas de pertencimento». As 2ⁿ atribuições são
 * os átomos do diagrama de Venn: percorrem-se TODAS, e onde diferem há CONTRAEXEMPLO
 * concreto, que se exibe em vez de se dizer «não é igual». */
static void conj_traduz(const char *s, char *out, size_t lim){
    size_t o = 0;
    while(*s && o + 8 < lim){
        if(!strncmp(s, "uniao", 5) || !strncmp(s, "união", 6)){
            o += (size_t)snprintf(out+o, lim-o, " + ");
            s += (*(s+2) == 'i') ? 5 : 6; continue;
        }
        if(!strncmp(s, "interseccao", 11) || !strncmp(s, "intersecao", 10) ||
           !strncmp(s, "interseção", 11) || !strncmp(s, "inter", 5)){
            o += (size_t)snprintf(out+o, lim-o, " * ");
            s += !strncmp(s, "interseccao", 11) ? 11 :
                 !strncmp(s, "intersecao", 10) ? 10 :
                 !strncmp(s, "interseção", 11) ? 11 : 5;
            continue;
        }
        if(!strncmp(s, "menos", 5)){ o += (size_t)snprintf(out+o, lim-o, " * nao "); s += 5; continue; }
        if(!strncmp(s, "delta", 5)){ o += (size_t)snprintf(out+o, lim-o, " ^ ");     s += 5; continue; }
        if(!strncmp(s, "contido", 7)){ o += (size_t)snprintf(out+o, lim-o, " -> ");  s += 7; continue; }
        if(!strncmp(s, "comp ", 5)){ o += (size_t)snprintf(out+o, lim-o, " nao ");   s += 5; continue; }
        if(!strncmp(s, "vazio", 5)){ o += (size_t)snprintf(out+o, lim-o, "0");       s += 5; continue; }
        if(!strncmp(s, "universo", 8)){ o += (size_t)snprintf(out+o, lim-o, "1");    s += 8; continue; }
        if(*s >= 'A' && *s <= 'Z'){                    /* o conjunto é a sua pertença */
            char v = (char)(*s - 'A' + 'a');
            if(s[1] == '\''){ o += (size_t)snprintf(out+o, lim-o, "nao %c", v); s += 2; continue; }
            out[o++] = v; s++; continue;
        }
        out[o++] = *s++;
    }
    out[o] = 0;
}

/* ─── A SIMPLIFICAÇÃO, E A LEI EM CADA TRANSIÇÃO ─────────────────────────────────
 *
 * O `eval.txt` põe a caixa de ferramentas inteira — idempotência, dominação, absorção,
 * involução, De Morgan, dualidade, DNF/CNF, Shannon — e acaba com a regra desta casa:
 * «fazendo cada um com VOLTA OBRIGATÓRIA: transformação → resultado → reconstrução».
 *
 * Então a simplificação não é um resultado que aparece: é uma DESCIDA em que cada
 * passo nomeia a lei que o autoriza. E a lei que faz o trabalho é sempre a mesma:
 *
 *     x·y + x·y' = x·(y + y') = x·1 = x
 *      distributividade    complemento    identidade
 *
 * Duas linhas que diferem num só bit juntam-se, e o bit sai. É a ADJACÊNCIA, e é o
 * corte da árvore: o que não distingue, não fica.
 *
 * O implicante escreve-se (mask, val): `mask` diz quais variáveis aparecem, `val` diz
 * com que valor. Sem mask não há variável — é o 1 (a dominação). */
typedef struct { unsigned mask, val; int vivo; } Imp;

static int bl_cobre(Imp im, unsigned x){ return (x & im.mask) == (im.val & im.mask); }

/* os PRIMOS: junta-se enquanto houver adjacência, e o que já não junta é primo */
static int bl_primos(const Bool *b, Imp *pr, int max, int *rondas){
    unsigned n = 1u << b->nv;
    Imp cur[BL_MAX], nxt[BL_MAX];
    int nc = 0, np = 0;
    *rondas = 0;
    for(unsigned x = 0; x < n; x++)
        if(b->t[x]){ cur[nc].mask = n - 1; cur[nc].val = x; cur[nc].vivo = 1; nc++; }
    while(nc > 0){
        int nn = 0;
        for(int i = 0; i < nc; i++) cur[i].vivo = 0;
        for(int i = 0; i < nc; i++)
            for(int j = i+1; j < nc; j++){
                if(cur[i].mask != cur[j].mask) continue;
                unsigned d = (cur[i].val ^ cur[j].val) & cur[i].mask;
                if(!d || (d & (d-1))) continue;          /* difere em UM bit só */
                cur[i].vivo = cur[j].vivo = 1;           /* juntaram-se: não são primos */
                Imp novo2 = { cur[i].mask & ~d, cur[i].val & ~d, 0 };
                int rep = 0;
                for(int k = 0; k < nn; k++)
                    if(nxt[k].mask == novo2.mask && (nxt[k].val & novo2.mask) == (novo2.val & novo2.mask)) rep = 1;
                if(!rep && nn < BL_MAX) nxt[nn++] = novo2;
            }
        for(int i = 0; i < nc; i++)
            if(!cur[i].vivo && np < max){                /* não juntou: é PRIMO */
                int rep = 0;
                for(int k = 0; k < np; k++)
                    if(pr[k].mask == cur[i].mask && (pr[k].val & pr[k].mask) == (cur[i].val & cur[i].mask)) rep = 1;
                if(!rep) pr[np++] = cur[i];
            }
        if(!nn) break;
        memcpy(cur, nxt, (size_t)nn * sizeof *nxt);
        nc = nn;
        (*rondas)++;
    }
    return np;
}
/* a COBERTURA: os essenciais primeiro (quem cobre um mintermo sozinho), depois o
 * guloso — e a VOLTA confere no fim, que é o que torna a escolha segura */
static int bl_cobertura(const Bool *b, Imp *pr, int np, int *sel){
    unsigned n = 1u << b->nv;
    unsigned char coberto[BL_MAX];
    memset(coberto, 0, n);
    int ns = 0;
    for(int k = 0; k < np; k++) sel[k] = 0;
    for(unsigned x = 0; x < n; x++){                     /* os ESSENCIAIS */
        if(!b->t[x]) continue;
        int quem = -1, quantos = 0;
        for(int k = 0; k < np; k++) if(bl_cobre(pr[k], x)){ quem = k; quantos++; }
        if(quantos == 1 && !sel[quem]){
            sel[quem] = 1; ns++;
            for(unsigned y = 0; y < n; y++) if(b->t[y] && bl_cobre(pr[quem], y)) coberto[y] = 1;
        }
    }
    for(;;){                                             /* o guloso para o resto */
        int falta = 0;
        for(unsigned x = 0; x < n; x++) if(b->t[x] && !coberto[x]) falta = 1;
        if(!falta) break;
        int melhor = -1, cob = 0;
        for(int k = 0; k < np; k++){
            if(sel[k]) continue;
            int c = 0;
            for(unsigned x = 0; x < n; x++) if(b->t[x] && !coberto[x] && bl_cobre(pr[k], x)) c++;
            if(c > cob){ cob = c; melhor = k; }
        }
        if(melhor < 0) break;
        sel[melhor] = 1; ns++;
        for(unsigned x = 0; x < n; x++) if(b->t[x] && bl_cobre(pr[melhor], x)) coberto[x] = 1;
    }
    return ns;
}
/* a VOLTA: a soma dos implicantes escolhidos dá a MESMA tabela? resíduo, e é ele
 * que decide se a simplificação se afirma */
static int bl_residuo(const Bool *b, Imp *pr, int np, const int *sel){
    unsigned n = 1u << b->nv;
    int r = 0;
    for(unsigned x = 0; x < n; x++){
        int v = 0;
        for(int k = 0; k < np; k++) if(sel[k] && bl_cobre(pr[k], x)) v = 1;
        if(v != b->t[x]) r++;
    }
    return r;
}
#endif
