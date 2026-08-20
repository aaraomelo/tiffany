/* portugues.h — ÁLGEBRA DO PORTUGUÊS SOBRE Word_8.
 *
 * O português é uma álgebra byte-level. Nada fora da álgebra = nada fora da
 * existência. Cadeia (arquitetura §sec:aranha):
 *
 *   Σ ⊆ Word_8  →  Σ* (monoide)  →  regras  →  semântica  →  órbita I
 *   I  --π Heighway-->  ℤ²  --|π⁻¹|-->  G
 *
 * Átomo: Word_8 = {0…255}. Multi-byte UTF-8 = palavra de bytes, não “carácter”.
 * Léxico: injecção parcial (roupa); dual ν com ν∘ν = id.
 * Gramática: transformações no monoide (concat, dual, identificação de classe).
 *
 * Medidor: tests/portugues_aranha.c §PA1–§PA8.
 */
#ifndef PORTUGUES_H
#define PORTUGUES_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PT_W8       256
#define PT_PAL_MAX  256
#define PT_I_MAX    4096
#define PT_LEX_MAX  64

/* ── Word_8: o suporte ───────────────────────────────────────────────────── */
typedef uint8_t PtByte;                       /* um átomo ∈ {0…255} */

/* ── Palavra = sequência finita em Σ* ────────────────────────────────────── */
typedef struct {
    PtByte b[PT_PAL_MAX];
    int    n;                                 /* comprimento; 0 = ε */
} PtPalavra;

static PtPalavra pt_eps(void){
    PtPalavra w; w.n = 0; return w;
}

static PtPalavra pt_de_bytes(const void *s, int n){
    PtPalavra w; w.n = 0;
    if(n < 0) n = 0;
    if(n > PT_PAL_MAX) n = PT_PAL_MAX;
    if(s && n) memcpy(w.b, s, (size_t)n);
    w.n = n;
    return w;
}

static PtPalavra pt_de_cstr(const char *s){
    if(!s) return pt_eps();
    size_t n = strlen(s);
    if(n > PT_PAL_MAX) n = PT_PAL_MAX;
    return pt_de_bytes(s, (int)n);
}

static int pt_igual(PtPalavra a, PtPalavra b){
    if(a.n != b.n) return 0;
    return a.n == 0 || memcmp(a.b, b.b, (size_t)a.n) == 0;
}

/* monoide: concatenação; ε é neutro */
static PtPalavra pt_concat(PtPalavra a, PtPalavra b){
    PtPalavra r; r.n = 0;
    int n = a.n + b.n;
    if(n > PT_PAL_MAX) n = PT_PAL_MAX;
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
static uint8_t pt_sigma[PT_W8];
static int     pt_sigma_pronta;

static void pt_sigma_marca(unsigned b){
    if(b < PT_W8) pt_sigma[b] = 1;
}

static void pt_sigma_init(void){
    if(pt_sigma_pronta) return;
    memset(pt_sigma, 0, sizeof pt_sigma);
    /* a–z A–Z 0–9 espaço */
    for(unsigned c = 'a'; c <= 'z'; c++) pt_sigma_marca(c);
    for(unsigned c = 'A'; c <= 'Z'; c++) pt_sigma_marca(c);
    for(unsigned c = '0'; c <= '9'; c++) pt_sigma_marca(c);
    pt_sigma_marca(' ');
    /* pontuação mínima do sistema */
    const char *pont = ".,;:!?-_'\"()[]/";
    for(const char *p = pont; *p; p++) pt_sigma_marca((unsigned char)*p);
    /* UTF-8: inícios C2–C3 (latin-1 suplementar) + continuações 80–BF */
    for(unsigned c = 0xC2; c <= 0xC3; c++) pt_sigma_marca(c);
    for(unsigned c = 0x80; c <= 0xBF; c++) pt_sigma_marca(c);
    pt_sigma_pronta = 1;
}

static int pt_em_sigma(PtByte b){
    pt_sigma_init();
    return pt_sigma[b] != 0;
}

/* |Σ|: quantos bytes admitidos */
static int pt_sigma_card(void){
    pt_sigma_init();
    int n = 0;
    for(int i = 0; i < PT_W8; i++) if(pt_sigma[i]) n++;
    return n;
}

/* palavra só usa bytes de Σ? */
static int pt_palavra_em_sigma(PtPalavra w){
    pt_sigma_init();
    for(int i = 0; i < w.n; i++)
        if(!pt_sigma[w.b[i]]) return 0;
    return 1;
}

/* ── Léxico: pares (fonte → glosa); dual ν com ν∘ν = id ──────────────────── */
typedef struct {
    PtPalavra de;
    PtPalavra para;
} PtPar;

static PtPar pt_lex[PT_LEX_MAX];
static int   pt_lex_n;
static int   pt_lex_pronto;

static void pt_lex_poe(const char *de, const char *para){
    if(pt_lex_n >= PT_LEX_MAX) return;
    pt_lex[pt_lex_n].de   = pt_de_cstr(de);
    pt_lex[pt_lex_n].para = pt_de_cstr(para);
    pt_lex_n++;
}

static void pt_lex_init(void){
    if(pt_lex_pronto) return;
    pt_lex_n = 0;
    /* núcleo mínimo medível (roupa do idioma; não derivado da cifra) */
    pt_lex_poe("o ", "the ");
    pt_lex_poe("a ", "the ");
    pt_lex_poe("ouro", "gold");
    pt_lex_poe("prata", "silver");
    pt_lex_poe("rei", "king");
    pt_lex_poe("casa", "house");
    pt_lex_poe("do ", "of the ");
    pt_lex_poe("é ", "is ");
    pt_lex_poe(" e ", " and ");
    pt_lex_poe("dragão", "dragon");
    pt_lex_poe("órbita", "orbit");
    pt_lex_poe("álgebra", "algebra");
    pt_lex_pronto = 1;
}

/* procura exacta; devolve índice ou −1 */
static int pt_lex_acha(PtPalavra w){
    pt_lex_init();
    for(int i = 0; i < pt_lex_n; i++)
        if(pt_igual(pt_lex[i].de, w) || pt_igual(pt_lex[i].para, w))
            return i;
    return -1;
}

/* dual ν: se w é `de`, devolve `para`; se é `para`, devolve `de`; senão w */
static PtPalavra pt_dual(PtPalavra w){
    int i = pt_lex_acha(w);
    if(i < 0) return w;
    if(pt_igual(pt_lex[i].de, w)) return pt_lex[i].para;
    return pt_lex[i].de;
}

/* ── Gramática = ops no monoide ────────────────────────────────────────────
 * Três regras fechadas (lista medível):
 *   PT_R_CONCAT     — composição (já é pt_concat)
 *   PT_R_DUAL       — involução léxica ν
 *   PT_R_IDENTIFICA — classe por último byte (Nerode mínimo: mesmo fim ⇒ mesma classe)
 */
enum { PT_R_CONCAT = 0, PT_R_DUAL = 1, PT_R_IDENTIFICA = 2, PT_R_N = 3 };

static const char *pt_regra_nome(int r){
    switch(r){
        case PT_R_CONCAT:     return "concat";
        case PT_R_DUAL:       return "dual_lex";
        case PT_R_IDENTIFICA: return "identifica";
        default:              return "?";
    }
}

/* classe de identificação: último byte (0 se ε) — órbita/relógio mínimo */
static PtByte pt_classe(PtPalavra w){
    return w.n > 0 ? w.b[w.n - 1] : 0;
}

static int pt_mesma_classe(PtPalavra a, PtPalavra b){
    return pt_classe(a) == pt_classe(b);
}

/* aplica regra; para IDENTIFICA: se mesma classe, projecta no representante a */
static PtPalavra pt_aplica(int regra, PtPalavra a, PtPalavra b){
    switch(regra){
        case PT_R_CONCAT:     return pt_concat(a, b);
        case PT_R_DUAL:       return pt_dual(a);
        case PT_R_IDENTIFICA: return pt_mesma_classe(a, b) ? a : b;
        default:              return a;
    }
}

/* ── Trajetória discreta I: cada passo algébrico = um índice ────────────────
 * Percurso: ler cada byte da frase (pertence a Σ) + aplicar dual quando o
 * prefixo fecha entrada do léxico. O comprimento |I| parametriza π no dragão. */
typedef struct {
    int passo[PT_I_MAX];                      /* código do passo (byte ou regra) */
    int n;
} PtOrbita;

static void pt_orbita_zera(PtOrbita *o){ o->n = 0; }

static void pt_orbita_poe(PtOrbita *o, int codigo){
    if(o->n >= PT_I_MAX) return;
    o->passo[o->n++] = codigo;
}

/* emite I a partir de uma frase: um índice por byte ∈ Σ + um por dual aplicado */
static int pt_emite_orbita(const char *frase, PtOrbita *o){
    pt_sigma_init();
    pt_lex_init();
    pt_orbita_zera(o);
    if(!frase) return 0;
    const unsigned char *p = (const unsigned char *)frase;
    PtPalavra acc = pt_eps();
    while(*p){
        PtByte b = *p++;
        if(!pt_em_sigma(b)) continue;         /* fora de Σ: não existe no sistema */
        /* ler byte = passo */
        pt_orbita_poe(o, (int)b);
        PtPalavra um = pt_de_bytes(&b, 1);
        acc = pt_concat(acc, um);
        /* se o acumulado (ou sufixo) fecha léxico, dual = passo gramatical */
        for(int i = 0; i < pt_lex_n; i++){
            PtPalavra de = pt_lex[i].de;
            if(de.n > 0 && acc.n >= de.n &&
               memcmp(acc.b + acc.n - de.n, de.b, (size_t)de.n) == 0){
                pt_orbita_poe(o, 256 + PT_R_DUAL);  /* código > 255 = regra */
                /* consome o sufixo léxico do acumulador (torção) */
                acc.n -= de.n;
                break;
            }
        }
    }
    return o->n;
}

/* chaves para o banco: idioma/pt/... */
static int pt_chave_alfabeto(char *out, int cap){
    pt_sigma_init();
    return snprintf(out, (size_t)cap, "idioma/pt/alfabeto|card=%d", pt_sigma_card());
}

static int pt_chave_lexico(int i, char *out, int cap){
    pt_lex_init();
    if(i < 0 || i >= pt_lex_n) return 0;
    char de[48], para[48];
    int nd = pt_lex[i].de.n, np = pt_lex[i].para.n;
    if(nd > 47) nd = 47;
    if(np > 47) np = 47;
    memcpy(de, pt_lex[i].de.b, (size_t)nd); de[nd] = 0;
    memcpy(para, pt_lex[i].para.b, (size_t)np); para[np] = 0;
    return snprintf(out, (size_t)cap, "idioma/pt/lexico/%s|%s", de, para);
}

static int pt_chave_regra(int r, char *out, int cap){
    if(r < 0 || r >= PT_R_N) return 0;
    return snprintf(out, (size_t)cap, "idioma/pt/regra/%s|op", pt_regra_nome(r));
}

#endif /* PORTUGUES_H */
