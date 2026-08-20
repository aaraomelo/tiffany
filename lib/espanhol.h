/* espanhol.h — ÁLGEBRA DO ESPANHOL SOBRE Word_8.
 *
 * O espanhol é uma álgebra byte-level. Nada fora da álgebra = nada fora da
 * existência. Cadeia (arquitetura §sec:aranha):
 *
 *   Σ ⊆ Word_8  →  Σ* (monoide)  →  regras  →  semântica  →  órbita I
 *   I  --π Heighway-->  ℤ²  --|π⁻¹|-->  G
 *
 * Átomo: Word_8 = {0…255}. Multi-byte UTF-8 = palavra de bytes, não “carácter”.
 * Léxico: injecção parcial (roupa); dual ν com ν∘ν = id.
 * Gramática: transformações no monoide (concat, dual, identificação de classe).
 *
 * Mesma tubagem que portugues/ingles. Léxico ES→PT.
 * Medidor: tests/espanhol_aranha.c §EA1–§EA8.
 */
#ifndef ESPANHOL_H
#define ESPANHOL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ES_W8       256
#define ES_PAL_MAX  256
#define ES_I_MAX    4096
#define ES_LEX_MAX  64

/* ── Word_8: o suporte ───────────────────────────────────────────────────── */
typedef uint8_t EsByte;                       /* um átomo ∈ {0…255} */

/* ── Palavra = sequência finita em Σ* ────────────────────────────────────── */
typedef struct {
    EsByte b[ES_PAL_MAX];
    int    n;                                 /* comprimento; 0 = ε */
} EsPalavra;

static EsPalavra es_eps(void){
    EsPalavra w; w.n = 0; return w;
}

static EsPalavra es_de_bytes(const void *s, int n){
    EsPalavra w; w.n = 0;
    if(n < 0) n = 0;
    if(n > ES_PAL_MAX) n = ES_PAL_MAX;
    if(s && n) memcpy(w.b, s, (size_t)n);
    w.n = n;
    return w;
}

static EsPalavra es_de_cstr(const char *s){
    if(!s) return es_eps();
    size_t n = strlen(s);
    if(n > ES_PAL_MAX) n = ES_PAL_MAX;
    return es_de_bytes(s, (int)n);
}

static int es_igual(EsPalavra a, EsPalavra b){
    if(a.n != b.n) return 0;
    return a.n == 0 || memcmp(a.b, b.b, (size_t)a.n) == 0;
}

/* monoide: concatenação; ε é neutro */
static EsPalavra es_concat(EsPalavra a, EsPalavra b){
    EsPalavra r; r.n = 0;
    int n = a.n + b.n;
    if(n > ES_PAL_MAX) n = ES_PAL_MAX;
    int na = a.n; if(na > n) na = n;
    if(na) memcpy(r.b, a.b, (size_t)na);
    int nb = n - na;
    if(nb > b.n) nb = b.n;
    if(nb) memcpy(r.b + na, b.b, (size_t)nb);
    r.n = na + nb;
    return r;
}

/* ── Alfabeto Σ ⊆ Word_8 ───────────────────────────────────────────────────
 * Máscara: bit 1 = byte admitido. Inclui ASCII imprimível usado em PT,
 * bytes de continuação UTF-8 (0x80–0xBF) e inícios C2–C3 (acentos latinos).
 * Espaço e pontuação básica entram: são regras do monoide, não “fora”. */
static uint8_t es_sigma[ES_W8];
static int     es_sigma_pronta;

static void es_sigma_marca(unsigned b){
    if(b < ES_W8) es_sigma[b] = 1;
}

static void es_sigma_init(void){
    if(es_sigma_pronta) return;
    memset(es_sigma, 0, sizeof es_sigma);
    /* a–z A–Z 0–9 espaço */
    for(unsigned c = 'a'; c <= 'z'; c++) es_sigma_marca(c);
    for(unsigned c = 'A'; c <= 'Z'; c++) es_sigma_marca(c);
    for(unsigned c = '0'; c <= '9'; c++) es_sigma_marca(c);
    es_sigma_marca(' ');
    /* pontuação mínima do sistema */
    const char *pont = ".,;:!?-_'\"()[]/";
    for(const char *p = pont; *p; p++) es_sigma_marca((unsigned char)*p);
    /* UTF-8: inícios C2–C3 (latin-1 suplementar) + continuações 80–BF */
    for(unsigned c = 0xC2; c <= 0xC3; c++) es_sigma_marca(c);
    for(unsigned c = 0x80; c <= 0xBF; c++) es_sigma_marca(c);
    es_sigma_pronta = 1;
}

static int es_em_sigma(EsByte b){
    es_sigma_init();
    return es_sigma[b] != 0;
}

/* |Σ|: quantos bytes admitidos */
static int es_sigma_card(void){
    es_sigma_init();
    int n = 0;
    for(int i = 0; i < ES_W8; i++) if(es_sigma[i]) n++;
    return n;
}

/* palavra só usa bytes de Σ? */
static int es_palavra_em_sigma(EsPalavra w){
    es_sigma_init();
    for(int i = 0; i < w.n; i++)
        if(!es_sigma[w.b[i]]) return 0;
    return 1;
}

/* ── Léxico: pares (fonte → glosa); dual ν com ν∘ν = id ──────────────────── */
typedef struct {
    EsPalavra de;
    EsPalavra para;
} EsPar;

static EsPar es_lex[ES_LEX_MAX];
static int   es_lex_n;
static int   es_lex_pronto;

static void es_lex_poe(const char *de, const char *para){
    if(es_lex_n >= ES_LEX_MAX) return;
    es_lex[es_lex_n].de   = es_de_cstr(de);
    es_lex[es_lex_n].para = es_de_cstr(para);
    es_lex_n++;
}

static void es_lex_init(void){
    if(es_lex_pronto) return;
    es_lex_n = 0;
    /* núcleo mínimo medível (roupa do idioma; não derivado da cifra) */
    es_lex_poe("el ", "o ");
    es_lex_poe("la ", "a ");
    es_lex_poe("oro", "ouro");
    es_lex_poe("plata", "prata");
    es_lex_poe("rey", "rei");
    es_lex_poe("casa", "casa");
    es_lex_poe("del ", "do ");
    es_lex_poe("es ", "é ");
    es_lex_poe(" y ", " e ");
    es_lex_poe("dragón", "dragão");
    es_lex_poe("órbita", "órbita");
    es_lex_poe("álgebra", "álgebra");
    es_lex_poe("ñ", "nh");
    es_lex_pronto = 1;
}

/* procura exacta; devolve índice ou −1 */
static int es_lex_acha(EsPalavra w){
    es_lex_init();
    for(int i = 0; i < es_lex_n; i++)
        if(es_igual(es_lex[i].de, w) || es_igual(es_lex[i].para, w))
            return i;
    return -1;
}

/* dual ν: se w é `de`, devolve `para`; se é `para`, devolve `de`; senão w */
static EsPalavra es_dual(EsPalavra w){
    int i = es_lex_acha(w);
    if(i < 0) return w;
    if(es_igual(es_lex[i].de, w)) return es_lex[i].para;
    return es_lex[i].de;
}

/* ── Gramática = ops no monoide ────────────────────────────────────────────
 * Três regras fechadas (lista medível):
 *   ES_R_CONCAT     — composição (já é es_concat)
 *   ES_R_DUAL       — involução léxica ν
 *   ES_R_IDENTIFICA — classe por último byte (Nerode mínimo: mesmo fim ⇒ mesma classe)
 */
enum { ES_R_CONCAT = 0, ES_R_DUAL = 1, ES_R_IDENTIFICA = 2, ES_R_N = 3 };

static const char *es_regra_nome(int r){
    switch(r){
        case ES_R_CONCAT:     return "concat";
        case ES_R_DUAL:       return "dual_lex";
        case ES_R_IDENTIFICA: return "identifica";
        default:              return "?";
    }
}

/* classe de identificação: último byte (0 se ε) — órbita/relógio mínimo */
static EsByte es_classe(EsPalavra w){
    return w.n > 0 ? w.b[w.n - 1] : 0;
}

static int es_mesma_classe(EsPalavra a, EsPalavra b){
    return es_classe(a) == es_classe(b);
}

/* aplica regra; para IDENTIFICA: se mesma classe, projecta no representante a */
static EsPalavra es_aplica(int regra, EsPalavra a, EsPalavra b){
    switch(regra){
        case ES_R_CONCAT:     return es_concat(a, b);
        case ES_R_DUAL:       return es_dual(a);
        case ES_R_IDENTIFICA: return es_mesma_classe(a, b) ? a : b;
        default:              return a;
    }
}

/* ── Trajetória discreta I: cada passo algébrico = um índice ────────────────
 * Percurso: ler cada byte da frase (pertence a Σ) + aplicar dual quando o
 * prefixo fecha entrada do léxico. O comprimento |I| parametriza π no dragão. */
typedef struct {
    int passo[ES_I_MAX];                      /* código do passo (byte ou regra) */
    int n;
} EsOrbita;

static void es_orbita_zera(EsOrbita *o){ o->n = 0; }

static void es_orbita_poe(EsOrbita *o, int codigo){
    if(o->n >= ES_I_MAX) return;
    o->passo[o->n++] = codigo;
}

/* emite I a partir de uma frase: um índice por byte ∈ Σ + um por dual aplicado */
static int es_emite_orbita(const char *frase, EsOrbita *o){
    es_sigma_init();
    es_lex_init();
    es_orbita_zera(o);
    if(!frase) return 0;
    const unsigned char *p = (const unsigned char *)frase;
    EsPalavra acc = es_eps();
    while(*p){
        EsByte b = *p++;
        if(!es_em_sigma(b)) continue;         /* fora de Σ: não existe no sistema */
        /* ler byte = passo */
        es_orbita_poe(o, (int)b);
        EsPalavra um = es_de_bytes(&b, 1);
        acc = es_concat(acc, um);
        /* se o acumulado (ou sufixo) fecha léxico, dual = passo gramatical */
        for(int i = 0; i < es_lex_n; i++){
            EsPalavra de = es_lex[i].de;
            if(de.n > 0 && acc.n >= de.n &&
               memcmp(acc.b + acc.n - de.n, de.b, (size_t)de.n) == 0){
                es_orbita_poe(o, 256 + ES_R_DUAL);  /* código > 255 = regra */
                /* consome o sufixo léxico do acumulador (torção) */
                acc.n -= de.n;
                break;
            }
        }
    }
    return o->n;
}

/* chaves para o banco: idioma/es/... */
static int es_chave_alfabeto(char *out, int cap){
    es_sigma_init();
    return snprintf(out, (size_t)cap, "idioma/es/alfabeto|card=%d", es_sigma_card());
}

static int es_chave_lexico(int i, char *out, int cap){
    es_lex_init();
    if(i < 0 || i >= es_lex_n) return 0;
    char de[48], para[48];
    int nd = es_lex[i].de.n, np = es_lex[i].para.n;
    if(nd > 47) nd = 47;
    if(np > 47) np = 47;
    memcpy(de, es_lex[i].de.b, (size_t)nd); de[nd] = 0;
    memcpy(para, es_lex[i].para.b, (size_t)np); para[np] = 0;
    return snprintf(out, (size_t)cap, "idioma/es/lexico/%s|%s", de, para);
}

static int es_chave_regra(int r, char *out, int cap){
    if(r < 0 || r >= ES_R_N) return 0;
    return snprintf(out, (size_t)cap, "idioma/es/regra/%s|op", es_regra_nome(r));
}

#endif /* ESPANHOL_H */
