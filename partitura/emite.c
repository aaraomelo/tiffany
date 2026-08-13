/* emite.c --- Pi_tradutor -> Pi_musical (vozes paralelas, um relogio)
 *
 * Ponte de realizacao integral do tradutor. NAO sonificacao de dados.
 * NAO inventa informacao fora da assinatura do tradutor.
 *
 *   estado do sistema -> assinatura (Pi) -> vozes paralelas -> execucao sincronizada
 *
 * Cadeia: Corpo -> pi_k -> Pi -> Maestro -> vozes -> partitura.
 * Fundamento: relogio -> metronomo -> maestro -> batuta -> Pi -> pi_k -> orquestra.
 *
 * Condicoes do Corpo de Peano / Corpo estelar (realizacao, nao teorema):
 *   Lei~0  --- barra / corte de compasso (\\Bb)
 *   Lei~1  --- involucao nu: figura \\N <-> silencio \\Rr; clave sol <-> fa
 *   Lei~3  --- trial I in {-1,0,+1} na Batuta (grave / 0 / agudo)
 *   Lei~5  --- pentagrama: 5 linhas; 8 alturas = linhas+espacos (grau)
 *   Lei~6  --- metronomo: tick comum (iface hexal na assinatura)
 *   Lei~8  --- AssinaturaOito / Z/8Z sob o palco
 *   Hurwitz --- naipes Cordas(1) Madeiras(2) Metais(4) Percussao(8)
 *   pi_k   --- Maestro: projecao sustentada (semibreve)
 *
 *   cc -O2 -std=c99 -Wall emite.c -o emite
 *   ./emite tradutor [-o gabarito/tradutor-completo.ly]
 *   ./emite tex     [-o ../papers/partitura_vozes.tex]
 *   ./emite lei8|naipes|pqr p q r [-o ficheiro.ly]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int dim;       /* SementeEstrela /Dim --- 2,4,8,16,... */
    int alcance;   /* /Alcance --- andares da espiral */
    int lado;      /* 0 Hurwitz, 1 Gentil */
    int iface;     /* /Interface --- hexal 6*2^(floor(alcance/3)) */
    int bpm;       /* pulso do metronomo (leitura, nao inventa estrutura) */
    int compassos; /* quantos compassos gravar (janela da peca) */
} Assinatura;

/* Um pulso: altura 0..7 (grau LilyPond) ou silencio (nu). */
typedef struct { int pitch; int rest; } Ev;

enum {
    V_BAT = 0, V_MAE, V_COR, V_MAD, V_MET, V_PER, V_L8
};

static Assinatura A_default(void){
    Assinatura a;
    a.dim = 8; a.alcance = 3; a.lado = 0; a.iface = 6; a.bpm = 72; a.compassos = 8;
    return a;
}

static void uso(const char *argv0){
    fprintf(stderr,
        "uso:\n"
        "  %s tradutor [--dim N] [--alcance K] [--lado 0|1] [--iface N]\n"
        "              [--bpm N] [--compassos N] [-o ficheiro.ly]\n"
        "  %s tex      [mesmas flags] [-o papers/partitura_vozes.tex]\n"
        "  %s lei8 [-o ficheiro.ly]\n"
        "  %s naipes [-o ficheiro.ly]\n"
        "  %s pqr <p> <q> <r> [-o ficheiro.ly]\n"
        "\n"
        "tradutor = LilyPond StaffGroup; tex = vozes desenhadas (Peano).\n",
        argv0, argv0, argv0, argv0, argv0);
}

static FILE *abre_out(int *argi, int argc, char **argv){
    if(*argi + 1 < argc && !strcmp(argv[*argi], "-o")){
        FILE *f = fopen(argv[*argi + 1], "w");
        if(!f){ perror(argv[*argi + 1]); exit(1); }
        *argi += 2;
        return f;
    }
    return stdout;
}

static int parse_int_flag(int *argi, int argc, char **argv, const char *flag, int *dst){
    if(*argi + 1 < argc && !strcmp(argv[*argi], flag)){
        *dst = atoi(argv[*argi + 1]);
        *argi += 2;
        return 1;
    }
    return 0;
}

static int parse_assinatura(int *i, int argc, char **argv, Assinatura *a){
    while(*i < argc){
        if(!strcmp(argv[*i], "-o")) break;
        if(parse_int_flag(i, argc, argv, "--dim", &a->dim)) continue;
        if(parse_int_flag(i, argc, argv, "--alcance", &a->alcance)) continue;
        if(parse_int_flag(i, argc, argv, "--lado", &a->lado)) continue;
        if(parse_int_flag(i, argc, argv, "--iface", &a->iface)) continue;
        if(parse_int_flag(i, argc, argv, "--bpm", &a->bpm)) continue;
        if(parse_int_flag(i, argc, argv, "--compassos", &a->compassos)) continue;
        fprintf(stderr, "flag desconhecida: %s\n", argv[*i]);
        return 0;
    }
    if(a->dim < 1) a->dim = 1;
    if(a->compassos < 1) a->compassos = 1;
    if(a->compassos > 64) a->compassos = 64;
    if(a->bpm < 30) a->bpm = 30;
    if(a->bpm > 240) a->bpm = 240;
    if(a->iface <= 0){
        int e = a->alcance / 3, p2 = 1, t;
        for(t = 0; t < e && t < 8; t++) p2 *= 2;
        a->iface = 6 * p2;
    }
    return 1;
}

/* --- motivos Peano (mesma estrutura em .ly e .tex) --- */

static Ev ev_de(int voz, int c, int p, int *k8){
    Ev e;
    e.pitch = 0;
    e.rest = 0;
    switch(voz){
    case V_BAT: {
        /* Lei~3 trial: I in {-1,0,+1}; ciclo (-,0,+,0) rodado por compasso */
        static const int I[4] = { -1, 0, +1, 0 };
        int v = I[(c + p) % 4];
        if(v == 0){ e.rest = 1; }
        else if(v < 0){ e.pitch = 0; }   /* grave --- linha de baixo */
        else { e.pitch = 4; }            /* agudo --- linha de topo */
        break;
    }
    case V_MAE:
        /* pi_k: ataque no 1; sustenta por silencios (nu no alfabeto ritmico) */
        e.pitch = 2; /* linha do meio */
        e.rest = (p != 0);
        break;
    case V_COR: {
        /* Hurwitz 1 = R: ordem total c d e f --- linhas 0..3 */
        static const int m[4] = { 0, 1, 2, 3 };
        e.pitch = m[p];
        break;
    }
    case V_MAD: {
        /* Hurwitz 2 = C: fase i, periodo 4 --- fig. Peano: e g e c */
        static const int m[4] = { 2, 4, 2, 0 };
        e.pitch = m[(c + p) % 4];
        break;
    }
    case V_MET: {
        /* Hurwitz 4 = H: fig. Peano c e as f */
        static const int m[4] = { 0, 2, 4, 3 };
        e.pitch = m[p];
        break;
    }
    case V_PER:
        /* Hurwitz 8 = O: golpe discreto C . G . */
        if(p == 1 || p == 3) e.rest = 1;
        else e.pitch = (p == 0) ? 0 : 3;
        break;
    case V_L8:
        /* Lei~8 / Z/8Z: k |-> k+1 mod 8 (Lily cromatico; TeX projecta nas 5 linhas) */
        e.pitch = (*k8) % 8;
        (*k8)++;
        break;
    default:
        e.rest = 1;
        break;
    }
    return e;
}

/* LilyPond pitch names for heights 0..7 (realizacao C major) */
static const char *ly_pitch_hi[8] = {
    "c'", "d'", "e'", "f'", "g'", "a'", "b'", "c''"
};
static const char *ly_pitch_mad[8] = {
    "c''", "d''", "e''", "f''", "g''", "a''", "b''", "c'''"
};

static void ly_ev(FILE *o, int voz, Ev e){
    if(e.rest){ fputs("r4", o); return; }
    if(voz == V_BAT){
        if(e.pitch <= 2) fputs("c4", o);
        else fputs("e'4", o);
        return;
    }
    if(voz == V_MAE){ fputs("g'1", o); return; } /* semibreve: so no p==0 */
    if(voz == V_PER){
        fputs(e.pitch <= 2 ? "c4" : "g4", o);
        return;
    }
    if(voz == V_MAD){
        fputs(ly_pitch_mad[e.pitch], o); fputs("4", o);
        return;
    }
    if(voz == V_MET && e.pitch == 4){
        fputs("af'4", o); /* Delta metalica */
        return;
    }
    fputs(ly_pitch_hi[e.pitch], o); fputs("4", o);
}

static void voz_ly(FILE *o, const char *id, const char *clef, int voz,
                   const Assinatura *a, int one_line){
    int c, p, k8 = 0;
    fprintf(o, "%s = \\absolute {\n  \\global \\clef %s\n", id, clef);
    if(one_line)
        fputs("  \\override Staff.StaffSymbol.line-count = #1\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  ", o);
        if(voz == V_MAE){
            /* um evento por compasso */
            ly_ev(o, voz, ev_de(voz, c, 0, &k8));
        } else {
            for(p = 0; p < 4; p++){
                ly_ev(o, voz, ev_de(voz, c, p, &k8));
                if(p < 3) fputc(' ', o);
            }
        }
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void emite_tradutor(FILE *o, const Assinatura *a){
    fprintf(o,
        "%% gerado por partitura/emite.c --- PARTITURA COMPLETA DO TRADUTOR\n"
        "%% Pi_tradutor |-> Pi_musical: Peano/estelar (nu, trial, pi_k, Lei 5/6/8, Hurwitz).\n"
        "%% NAO inventa informacao fora da assinatura.\n"
        "%% SementeEstrela: Dim=%d Alcance=%d Lado=%d Interface=%d bpm=%d compassos=%d\n"
        "\\version \"2.24.0\"\n"
        "\\language \"english\"\n"
        "\\header {\n"
        "  title = \"Tradutor --- partitura completa\"\n"
        "  subtitle = \"Pi: Dim=%d Alcance=%d Lado=%d Iface=%d\"\n"
        "  composer = \"tiffany / emite.c\"\n"
        "  tagline = ##f\n"
        "}\n"
        "global = {\n"
        "  \\key c \\major\n"
        "  \\time 4/4\n"
        "  \\tempo \"Metronomo\" 4 = %d\n"
        "}\n\n",
        a->dim, a->alcance, a->lado, a->iface, a->bpm, a->compassos,
        a->dim, a->alcance, a->lado, a->iface, a->bpm);

    /* Metronomo (Lei~6): um tick por pulso --- relogio lido */
    {
        int c;
        fputs("metronomo = \\absolute {\n  \\global \\clef percussion\n"
              "  \\override Staff.StaffSymbol.line-count = #1\n", o);
        for(c = 0; c < a->compassos; c++){
            fputs("  c'4 c'4 c'4 c'4", o);
            if(c + 1 < a->compassos) fputs(" |\n", o);
            else fputs(" \\bar \"|.\"\n", o);
        }
        fputs("}\n", o);
    }
    voz_ly(o, "batuta", "bass", V_BAT, a, 0);
    voz_ly(o, "maestro", "treble", V_MAE, a, 0);
    voz_ly(o, "cordas", "treble", V_COR, a, 0);
    voz_ly(o, "madeiras", "treble", V_MAD, a, 0);
    voz_ly(o, "metais", "treble", V_MET, a, 0);
    voz_ly(o, "percussao", "bass", V_PER, a, 0);
    voz_ly(o, "leiOito", "treble", V_L8, a, 0);

    fputs(
        "\n\\score {\n"
        "  \\new StaffGroup <<\n"
        "    \\new Staff \\with { instrumentName = #\"Metronomo\" shortInstrumentName = #\"Metr.\" }\n"
        "      { \\metronomo }\n"
        "    \\new Staff \\with { instrumentName = #\"Batuta\" shortInstrumentName = #\"Bat.\" }\n"
        "      { \\batuta }\n"
        "    \\new Staff \\with { instrumentName = #\"Maestro\" shortInstrumentName = #\"Mae.\" }\n"
        "      { \\maestro }\n"
        "    \\new Staff \\with { instrumentName = #\"Cordas (1)\" shortInstrumentName = #\"Cor.\" }\n"
        "      { \\cordas }\n"
        "    \\new Staff \\with { instrumentName = #\"Madeiras (2)\" shortInstrumentName = #\"Mad.\" }\n"
        "      { \\madeiras }\n"
        "    \\new Staff \\with { instrumentName = #\"Metais (4)\" shortInstrumentName = #\"Met.\" }\n"
        "      { \\metais }\n"
        "    \\new Staff \\with { instrumentName = #\"Percussao (8)\" shortInstrumentName = #\"Per.\" }\n"
        "      { \\percussao }\n"
        "    \\new Staff \\with { instrumentName = #\"Lei 8\" shortInstrumentName = #\"L8\" }\n"
        "      { \\leiOito }\n"
        "  >>\n"
        "  \\layout {\n"
        "    \\context { \\StaffGroup \\consists \"Span_bar_engraver\" }\n"
        "  }\n"
        "  \\midi { }\n"
        "}\n", o);
}

/* --- TeX do tradutor: 8 alturas no pentagrama (Lei~5) --- */

/*
 * TeX: UMA linha no cone + espiral de expoentes (sobe_exp / esp_gira).
 * Multi-lane com \\par nao cabe no pentagrama (entrelinha >> 1,55mm).
 * Pitch 0..4 = _{_note} .. ^{^{note}} --- translacao Y real, eixo do Metrónomo intacto.
 */
static int linha_cone(int pitch){
    static const int esp[8] = { 0, 1, 2, 3, 4, 3, 2, 1 };
    if(pitch < 0) return 0;
    if(pitch > 7) pitch = 7;
    return esp[pitch];
}

static void tex_nota_y(FILE *o, Ev e){
    static const char *forms[5] = {
        "$_{_{\\note}}$",
        "$_{\\note}$",
        "$\\note$",
        "$^{\\note}$",
        "$^{^{\\note}}$"
    };
    int L;
    if(e.rest){ fputs("\\Rr", o); return; }
    L = linha_cone(e.pitch);
    if(L < 0) L = 0;
    if(L > 4) L = 4;
    fputs(forms[L], o);
}

static void tex_voz_penta(FILE *o, const char *nome, const char *papel,
                          int voz, const Assinatura *a){
    int c, p, k8 = 0;
    fprintf(o, "\\VozAbre{%s}{%s}{\\Penta}{-9.5mm}%%\n\\noindent", nome, papel);
    for(c = 0; c < a->compassos; c++){
        for(p = 0; p < 4; p++){
            Ev e = ev_de(voz, c, p, &k8);
            tex_nota_y(o, e);
            fputs("\\,", o);
        }
        if(c + 1 < a->compassos) fputs("\\Bb", o);
    }
    fputs("%\n\\par\n\n", o);
}

static void tex_voz_metro(FILE *o, const Assinatura *a){
    int c, p;
    /* Tick = \\bullet na régua dourada (bulbo centrado; Y contínuo, sem teto). */
    fputs("\\VozMetro{Lei~6 --- eixo horizontal / tick}{-5.45mm}%\n", o);
    for(c = 0; c < a->compassos; c++){
        for(p = 0; p < 4; p++) fputs("\\Tick\\,", o);
        if(c + 1 < a->compassos) fputs("\\Bb", o);
    }
    fputs("%\n\\par\n\n", o);
}

static void emite_tradutor_tex(FILE *o, const Assinatura *a){
    fprintf(o,
        "%% gerado por partitura/emite.c tex --- NAO editar a mao\n"
        "%% Condicoes Peano/estelar alinhando notas no pentagrama.\n"
        "%% Dim=%d Alcance=%d Lado=%d Iface=%d bpm=%d compassos=%d\n"
        "%% \\N figura  \\Rr silencio (nu)  \\E vazio em Y  \\Bb barra (Lei~0)\n"
        "%% Translacao Y = linha_cone(pitch) nas 5 geratrizes (nao linear).\n\n"
        "\\noindent{\\small Dim$=$%d · Alcance$=$%d · Lado$=$%d · Iface$=$%d ·\n"
        "Metronomo ($q{=}%d$) · %d compassos · $4/4$ · armadura C}\\par\n\n",
        a->dim, a->alcance, a->lado, a->iface, a->bpm, a->compassos,
        a->dim, a->alcance, a->lado, a->iface, a->bpm, a->compassos);

    tex_voz_metro(o, a);
    tex_voz_penta(o, "Batuta",
        "vareta / Lei~3 trial $I\\in\\{-1,0,+1\\}$",
        V_BAT, a);
    tex_voz_penta(o, "Maestro",
        "$\\pi_k$ --- fonte da espiral (clave G)",
        V_MAE, a);
    tex_voz_penta(o, "Cordas (1)",
        "orquestra --- Hurwitz $\\mathbb{R}$",
        V_COR, a);
    tex_voz_penta(o, "Madeiras (2)",
        "orquestra --- Hurwitz $\\mathbb{C}$ (fase $i$)",
        V_MAD, a);
    tex_voz_penta(o, "Metais (4)",
        "orquestra --- Hurwitz $\\mathbb{H}$ ($\\Delta$)",
        V_MET, a);
    tex_voz_penta(o, "Percussao (8)",
        "orquestra --- Hurwitz $\\mathbb{O}$",
        V_PER, a);
    tex_voz_penta(o, "Lei 8",
        "espiral $\\mathbb{Z}/8\\mathbb{Z}$ no cone",
        V_L8, a);
}

/* --- modos auxiliares (excertos) --- */

static void emite_lei8(FILE *o){
    Assinatura a = A_default();
    a.compassos = 2;
    fputs("%% excerto Lei 8 --- preferir: ./emite tradutor\n", o);
    fprintf(o, "\\version \"2.24.0\"\n\\language \"english\"\n"
               "\\header { title = \"Lei 8\" tagline = ##f }\n"
               "global = { \\key c \\major \\time 4/4 \\tempo 4 = %d }\n", a.bpm);
    voz_ly(o, "leiOito", "treble", V_L8, &a, 0);
    fputs("\\score { \\new Staff \\with { instrumentName = #\"Lei 8\" } { \\leiOito } \\layout { } }\n", o);
}

static void emite_naipes(FILE *o){
    Assinatura a = A_default();
    a.compassos = 4;
    fputs("%% excerto naipes --- preferir: ./emite tradutor\n", o);
    fprintf(o, "\\version \"2.24.0\"\n\\language \"english\"\n"
               "\\header { title = \"Naipes Hurwitz\" tagline = ##f }\n"
               "global = { \\key c \\major \\time 4/4 \\tempo 4 = %d }\n", a.bpm);
    voz_ly(o, "cordas", "treble", V_COR, &a, 0);
    voz_ly(o, "madeiras", "treble", V_MAD, &a, 0);
    voz_ly(o, "metais", "treble", V_MET, &a, 0);
    voz_ly(o, "percussao", "bass", V_PER, &a, 0);
    fputs("\\score { \\new StaffGroup <<\n"
          "  \\new Staff \\with { instrumentName = #\"Cordas (1)\" } { \\cordas }\n"
          "  \\new Staff \\with { instrumentName = #\"Madeiras (2)\" } { \\madeiras }\n"
          "  \\new Staff \\with { instrumentName = #\"Metais (4)\" } { \\metais }\n"
          "  \\new Staff \\with { instrumentName = #\"Percussao (8)\" } { \\percussao }\n"
          ">> \\layout { } }\n", o);
}

static void motivo_p(FILE *o, int n){
    static const char *notas[] = {
        "c'4","d'4","e'4","f'4","g'4","a'4","b'4","c''4",
        "d''4","e''4","f''4","g''4","a''4","b''4","c'''4","d'''4"
    };
    int i, lim;
    if(n <= 0){ fputs("  r1 | r1 | r1 | r1 \\bar \"|.\"\n", o); return; }
    lim = n * 4; if(lim > 16) lim = 16;
    fputs("  ", o);
    for(i = 0; i < lim; i++){
        fputs(notas[i], o);
        if(i + 1 < lim){ if((i + 1) % 4 == 0) fputs(" |\n  ", o); else fputc(' ', o); }
    }
    fputs(" \\bar \"|.\"\n", o);
}

static void motivo_q(FILE *o, int n){
    static const char *notas[] = {
        "c''4","b'4","a'4","g'4","f'4","e'4","d'4","c'4",
        "b4","a4","g4","f4","e4","d4","c4","b,4"
    };
    int i, lim;
    if(n <= 0){ fputs("  r1 | r1 | r1 | r1 \\bar \"|.\"\n", o); return; }
    lim = n * 4; if(lim > 16) lim = 16;
    fputs("  ", o);
    for(i = 0; i < lim; i++){
        fputs(notas[i], o);
        if(i + 1 < lim){ if((i + 1) % 4 == 0) fputs(" |\n  ", o); else fputc(' ', o); }
    }
    fputs(" \\bar \"|.\"\n", o);
}

static void emite_pqr(FILE *o, int p, int q, int r){
    fprintf(o,
        "%% trial (p,q,r)=(%d,%d,%d) --- excerto; preferir ./emite tradutor\n"
        "\\version \"2.24.0\"\n\\language \"english\"\n"
        "\\header { title = \"(p,q,r)=(%d,%d,%d)\" tagline = ##f }\n"
        "global = { \\key c \\major \\time 4/4 \\tempo 4 = 66 }\n",
        p, q, r, p, q, r);
    fputs("vozP = \\absolute { \\global \\clef treble\n", o); motivo_p(o, p); fputs("}\n", o);
    fputs("vozQ = \\absolute { \\global \\clef treble\n", o); motivo_q(o, q); fputs("}\n", o);
    fputs("vozR = \\absolute { \\global \\clef treble\n", o);
    if(r > 0){
        int t, lim = r < 4 ? r : 4;
        fputs("  ", o);
        for(t = 0; t < lim; t++){ fputs("r1", o); if(t + 1 < lim) fputs(" | ", o); }
        fputs(" \\bar \"|.\"\n", o);
    } else fputs("  r1 | r1 | r1 | r1 \\bar \"|.\"\n", o);
    fputs("}\n\\score { \\new StaffGroup <<\n"
          "  \\new Staff \\with { instrumentName = #\"p (+)\" } { \\vozP }\n"
          "  \\new Staff \\with { instrumentName = #\"q (-)\" } { \\vozQ }\n"
          "  \\new Staff \\with { instrumentName = #\"r (0)\" } { \\vozR }\n"
          ">> \\layout { } }\n", o);
}

int main(int argc, char **argv){
    int i;
    FILE *o;
    Assinatura a = A_default();
    if(argc < 2){ uso(argv[0]); return 1; }

    if(!strcmp(argv[1], "tradutor") || !strcmp(argv[1], "tex")){
        int eh_tex = !strcmp(argv[1], "tex");
        i = 2;
        if(!parse_assinatura(&i, argc, argv, &a)){ uso(argv[0]); return 1; }
        o = abre_out(&i, argc, argv);
        if(eh_tex) emite_tradutor_tex(o, &a);
        else emite_tradutor(o, &a);
        if(o != stdout) fclose(o);
        return 0;
    }
    if(!strcmp(argv[1], "lei8")){
        i = 2; o = abre_out(&i, argc, argv); emite_lei8(o);
        if(o != stdout) fclose(o);
        return 0;
    }
    if(!strcmp(argv[1], "naipes")){
        i = 2; o = abre_out(&i, argc, argv); emite_naipes(o);
        if(o != stdout) fclose(o);
        return 0;
    }
    if(!strcmp(argv[1], "pqr")){
        int p, q, r;
        if(argc < 5){ uso(argv[0]); return 1; }
        p = atoi(argv[2]); q = atoi(argv[3]); r = atoi(argv[4]);
        if(p < 0 || q < 0 || r < 0){ fprintf(stderr, "p,q,r >= 0\n"); return 1; }
        i = 5; o = abre_out(&i, argc, argv); emite_pqr(o, p, q, r);
        if(o != stdout) fclose(o);
        return 0;
    }
    uso(argv[0]);
    return 1;
}
