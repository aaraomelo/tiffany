/* ingles.h — ÁLGEBRA DO INGLÊS SOBRE Word_8 (mesma tubagem que portugues.h).
 *
 *   Σ ⊆ Word_8  →  Σ*  →  regras  →  órbita I  →  π Heighway  →  G
 *
 * Σ do inglês: ASCII imprimível (sem exigir C2–C3). Léxico EN→PT = dual do PT.
 * Regras idênticas: concat, dual_lex, identifica.
 *
 * Medidor: tests/ingles_aranha.c §IA1–§IA8.
 */
#ifndef INGLES_H
#define INGLES_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define EN_W8       256
#define EN_PAL_MAX  256
#define EN_I_MAX    4096
#define EN_LEX_MAX  64

typedef uint8_t EnByte;

typedef struct {
    EnByte b[EN_PAL_MAX];
    int    n;
} EnPalavra;

static EnPalavra en_eps(void){
    EnPalavra w; w.n = 0; return w;
}

static EnPalavra en_de_bytes(const void *s, int n){
    EnPalavra w; w.n = 0;
    if(n < 0) n = 0;
    if(n > EN_PAL_MAX) n = EN_PAL_MAX;
    if(s && n) memcpy(w.b, s, (size_t)n);
    w.n = n;
    return w;
}

static EnPalavra en_de_cstr(const char *s){
    if(!s) return en_eps();
    size_t n = strlen(s);
    if(n > EN_PAL_MAX) n = EN_PAL_MAX;
    return en_de_bytes(s, (int)n);
}

static int en_igual(EnPalavra a, EnPalavra b){
    if(a.n != b.n) return 0;
    return a.n == 0 || memcmp(a.b, b.b, (size_t)a.n) == 0;
}

static EnPalavra en_concat(EnPalavra a, EnPalavra b){
    EnPalavra r; r.n = 0;
    int n = a.n + b.n;
    if(n > EN_PAL_MAX) n = EN_PAL_MAX;
    int na = a.n; if(na > n) na = n;
    if(na) memcpy(r.b, a.b, (size_t)na);
    int nb = n - na;
    if(nb > b.n) nb = b.n;
    if(nb) memcpy(r.b + na, b.b, (size_t)nb);
    r.n = na + nb;
    return r;
}

/* Σ inglês: ASCII letras/dígitos/espaço/pontuação — Word_8 sem C2–C3 obrigatório */
static uint8_t en_sigma[EN_W8];
static int     en_sigma_pronta;

static void en_sigma_marca(unsigned b){
    if(b < EN_W8) en_sigma[b] = 1;
}

static void en_sigma_init(void){
    if(en_sigma_pronta) return;
    memset(en_sigma, 0, sizeof en_sigma);
    for(unsigned c = 'a'; c <= 'z'; c++) en_sigma_marca(c);
    for(unsigned c = 'A'; c <= 'Z'; c++) en_sigma_marca(c);
    for(unsigned c = '0'; c <= '9'; c++) en_sigma_marca(c);
    en_sigma_marca(' ');
    const char *pont = ".,;:!?-_'\"()[]/";
    for(const char *p = pont; *p; p++) en_sigma_marca((unsigned char)*p);
    en_sigma_pronta = 1;
}

static int en_em_sigma(EnByte b){
    en_sigma_init();
    return en_sigma[b] != 0;
}

static int en_sigma_card(void){
    en_sigma_init();
    int n = 0;
    for(int i = 0; i < EN_W8; i++) if(en_sigma[i]) n++;
    return n;
}

static int en_palavra_em_sigma(EnPalavra w){
    en_sigma_init();
    for(int i = 0; i < w.n; i++)
        if(!en_sigma[w.b[i]]) return 0;
    return 1;
}

typedef struct {
    EnPalavra de;
    EnPalavra para;
} EnPar;

static EnPar en_lex[EN_LEX_MAX];
static int   en_lex_n;
static int   en_lex_pronto;

static void en_lex_poe(const char *de, const char *para){
    if(en_lex_n >= EN_LEX_MAX) return;
    en_lex[en_lex_n].de   = en_de_cstr(de);
    en_lex[en_lex_n].para = en_de_cstr(para);
    en_lex_n++;
}

static void en_lex_init(void){
    if(en_lex_pronto) return;
    en_lex_n = 0;
    /* dual do léxico PT: EN → PT */
    en_lex_poe("the ", "o ");
    en_lex_poe("gold", "ouro");
    en_lex_poe("silver", "prata");
    en_lex_poe("king", "rei");
    en_lex_poe("house", "casa");
    en_lex_poe("of the ", "do ");
    en_lex_poe("is ", "é ");
    en_lex_poe(" and ", " e ");
    en_lex_poe("dragon", "dragão");
    en_lex_poe("orbit", "órbita");
    en_lex_poe("algebra", "álgebra");
    en_lex_pronto = 1;
}

static int en_lex_acha(EnPalavra w){
    en_lex_init();
    for(int i = 0; i < en_lex_n; i++)
        if(en_igual(en_lex[i].de, w) || en_igual(en_lex[i].para, w))
            return i;
    return -1;
}

static EnPalavra en_dual(EnPalavra w){
    int i = en_lex_acha(w);
    if(i < 0) return w;
    if(en_igual(en_lex[i].de, w)) return en_lex[i].para;
    return en_lex[i].de;
}

enum { EN_R_CONCAT = 0, EN_R_DUAL = 1, EN_R_IDENTIFICA = 2, EN_R_N = 3 };

static const char *en_regra_nome(int r){
    switch(r){
        case EN_R_CONCAT:     return "concat";
        case EN_R_DUAL:       return "dual_lex";
        case EN_R_IDENTIFICA: return "identifica";
        default:              return "?";
    }
}

static EnByte en_classe(EnPalavra w){
    return w.n > 0 ? w.b[w.n - 1] : 0;
}

static int en_mesma_classe(EnPalavra a, EnPalavra b){
    return en_classe(a) == en_classe(b);
}

static EnPalavra en_aplica(int regra, EnPalavra a, EnPalavra b){
    switch(regra){
        case EN_R_CONCAT:     return en_concat(a, b);
        case EN_R_DUAL:       return en_dual(a);
        case EN_R_IDENTIFICA: return en_mesma_classe(a, b) ? a : b;
        default:              return a;
    }
}

typedef struct {
    int passo[EN_I_MAX];
    int n;
} EnOrbita;

static void en_orbita_zera(EnOrbita *o){ o->n = 0; }

static void en_orbita_poe(EnOrbita *o, int codigo){
    if(o->n >= EN_I_MAX) return;
    o->passo[o->n++] = codigo;
}

static int en_emite_orbita(const char *frase, EnOrbita *o){
    en_sigma_init();
    en_lex_init();
    en_orbita_zera(o);
    if(!frase) return 0;
    const unsigned char *p = (const unsigned char *)frase;
    EnPalavra acc = en_eps();
    while(*p){
        EnByte b = *p++;
        if(!en_em_sigma(b)) continue;
        en_orbita_poe(o, (int)b);
        EnPalavra um = en_de_bytes(&b, 1);
        acc = en_concat(acc, um);
        for(int i = 0; i < en_lex_n; i++){
            EnPalavra de = en_lex[i].de;
            if(de.n > 0 && acc.n >= de.n &&
               memcmp(acc.b + acc.n - de.n, de.b, (size_t)de.n) == 0){
                en_orbita_poe(o, 256 + EN_R_DUAL);
                acc.n -= de.n;
                break;
            }
        }
    }
    return o->n;
}

static int en_chave_alfabeto(char *out, int cap){
    en_sigma_init();
    return snprintf(out, (size_t)cap, "idioma/en/alfabeto|card=%d", en_sigma_card());
}

static int en_chave_lexico(int i, char *out, int cap){
    en_lex_init();
    if(i < 0 || i >= en_lex_n) return 0;
    char de[48], para[48];
    int nd = en_lex[i].de.n, np = en_lex[i].para.n;
    if(nd > 47) nd = 47;
    if(np > 47) np = 47;
    memcpy(de, en_lex[i].de.b, (size_t)nd); de[nd] = 0;
    memcpy(para, en_lex[i].para.b, (size_t)np); para[np] = 0;
    return snprintf(out, (size_t)cap, "idioma/en/lexico/%s|%s", de, para);
}

static int en_chave_regra(int r, char *out, int cap){
    if(r < 0 || r >= EN_R_N) return 0;
    return snprintf(out, (size_t)cap, "idioma/en/regra/%s|op", en_regra_nome(r));
}

#endif /* INGLES_H */
