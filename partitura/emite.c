/* emite.c --- Pi_tradutor -> Pi_musical (vozes paralelas, um relogio)
 *
 * Ponte de realizacao integral do tradutor. NAO sonificacao de dados.
 * NAO inventa informacao fora da assinatura do tradutor.
 *
 *   estado do sistema -> assinatura (Pi) -> vozes paralelas -> execucao sincronizada
 *
 * Cadeia: Corpo -> Pi -> emissor nativo -> .ly -> LilyPond (so grava).
 * Fundamento: relogio -> metronomo -> maestro -> batuta -> Pi -> pi_k -> orquestra.
 *
 * Vozes da partitura completa (StaffGroup, mesmo \\time / \\tempo):
 *   Metronomo   --- tick (tempo)
 *   Batuta      --- inversor trial {-1,0,+1}
 *   Maestro     --- sincronizacao da projecao
 *   Cordas(1) Madeiras(2) Metais(4) Percussao(8) --- naipes Hurwitz
 *   Lei8        --- AssinaturaOito / Z/8Z sob o palco
 *
 * Params da assinatura (SementeEstrela / defaults do motor):
 *   --dim N --alcance K --lado 0|1 --iface N --bpm N
 *
 *   cc -O2 -std=c99 -Wall emite.c -o emite
 *   ./emite tradutor [-o gabarito/tradutor-completo.ly]
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
        "  %s lei8 [-o ficheiro.ly]\n"
        "  %s naipes [-o ficheiro.ly]\n"
        "  %s pqr <p> <q> <r> [-o ficheiro.ly]\n"
        "\n"
        "tradutor = partitura completa: todas as vozes, um relogio, uma regencia.\n",
        argv0, argv0, argv0, argv0);
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

/* --- motivos estruturais (realizam classes; nao amostram series) --- */

static void voz_metronomo(FILE *o, const Assinatura *a){
    int c;
    fprintf(o, "metronomo = \\absolute {\n"
               "  \\global \\clef percussion\n"
               "  \\override Staff.StaffSymbol.line-count = #1\n");
    for(c = 0; c < a->compassos; c++){
        /* um tick por pulso --- o relogio lido (thm:metronomo) */
        fputs("  c'4 c'4 c'4 c'4", o);
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_batuta(FILE *o, const Assinatura *a){
    /* inversor I in {-1,0,+1}: grave / silencio / agudo --- trial Lei 3 */
    static const char *ciclo[] = { "c4", "r4", "e'4", "r4" }; /* - 0 + 0 */
    int c, p;
    fputs("batuta = \\absolute {\n  \\global \\clef bass\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  ", o);
        for(p = 0; p < 4; p++){
            fputs(ciclo[(c + p) % 4], o);
            if(p < 3) fputc(' ', o);
        }
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_maestro(FILE *o, const Assinatura *a){
    /* sincronizacao: nota longa por compasso --- a projecao partilhada */
    int c;
    fputs("maestro = \\absolute {\n  \\global \\clef treble\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  g'1", o);
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_cordas(FILE *o, const Assinatura *a){
    static const char *m[] = { "c'4", "d'4", "e'4", "f'4" };
    int c, p;
    fputs("cordas = \\absolute {\n  \\global \\clef treble\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  ", o);
        for(p = 0; p < 4; p++){
            fputs(m[p], o);
            if(p < 3) fputc(' ', o);
        }
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_madeiras(FILE *o, const Assinatura *a){
    /* periodo 4 (fase i) --- grau 2 */
    static const char *m[] = { "e''4", "g''4", "e''4", "c''4" };
    int c, p;
    fputs("madeiras = \\absolute {\n  \\global \\clef treble\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  ", o);
        for(p = 0; p < 4; p++){
            fputs(m[(c + p) % 4], o);
            if(p < 3) fputc(' ', o);
        }
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_metais(FILE *o, const Assinatura *a){
    static const char *m[] = { "c'4", "e'4", "af'4", "f'4" };
    int c, p;
    fputs("metais = \\absolute {\n  \\global \\clef treble\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  ", o);
        for(p = 0; p < 4; p++){
            fputs(m[p], o);
            if(p < 3) fputc(' ', o);
        }
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_percussao(FILE *o, const Assinatura *a){
    int c;
    fputs("percussao = \\absolute {\n  \\global \\clef bass\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  c4 r4 g4 r4", o);
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void voz_lei8(FILE *o, const Assinatura *a){
    /* AssinaturaOito: k |-> k+1 mod 8 --- um periodo por 2 compassos em 4/4 */
    static const char *n8[] = {
        "c'4","d'4","e'4","f'4","g'4","a'4","b'4","c''4"
    };
    int c, p, k = 0;
    fputs("leiOito = \\absolute {\n  \\global \\clef treble\n", o);
    for(c = 0; c < a->compassos; c++){
        fputs("  ", o);
        for(p = 0; p < 4; p++){
            fputs(n8[k % 8], o);
            k++;
            if(p < 3) fputc(' ', o);
        }
        if(c + 1 < a->compassos) fputs(" |\n", o);
        else fputs(" \\bar \"|.\"\n", o);
    }
    fputs("}\n", o);
}

static void emite_tradutor(FILE *o, const Assinatura *a){
    fprintf(o,
        "%% gerado por partitura/emite.c --- PARTITURA COMPLETA DO TRADUTOR\n"
        "%% Pi_tradutor |-> Pi_musical: todas as vozes, um relogio, uma regencia.\n"
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

    voz_metronomo(o, a);
    voz_batuta(o, a);
    voz_maestro(o, a);
    voz_cordas(o, a);
    voz_madeiras(o, a);
    voz_metais(o, a);
    voz_percussao(o, a);
    voz_lei8(o, a);

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

/* --- modos auxiliares (excertos; o alvo e' tradutor) --- */

static void emite_lei8(FILE *o){
    Assinatura a = A_default();
    a.compassos = 2;
    fputs("%% excerto Lei 8 --- preferir: ./emite tradutor\n", o);
    fprintf(o, "\\version \"2.24.0\"\n\\language \"english\"\n"
               "\\header { title = \"Lei 8\" tagline = ##f }\n"
               "global = { \\key c \\major \\time 4/4 \\tempo 4 = %d }\n", a.bpm);
    voz_lei8(o, &a);
    fputs("\\score { \\new Staff \\with { instrumentName = #\"Lei 8\" } { \\leiOito } \\layout { } }\n", o);
}

static void emite_naipes(FILE *o){
    Assinatura a = A_default();
    a.compassos = 4;
    fputs("%% excerto naipes --- preferir: ./emite tradutor\n", o);
    fprintf(o, "\\version \"2.24.0\"\n\\language \"english\"\n"
               "\\header { title = \"Naipes Hurwitz\" tagline = ##f }\n"
               "global = { \\key c \\major \\time 4/4 \\tempo 4 = %d }\n", a.bpm);
    voz_cordas(o, &a); voz_madeiras(o, &a); voz_metais(o, &a); voz_percussao(o, &a);
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

    if(!strcmp(argv[1], "tradutor")){
        i = 2;
        while(i < argc){
            if(!strcmp(argv[i], "-o")) break;
            if(parse_int_flag(&i, argc, argv, "--dim", &a.dim)) continue;
            if(parse_int_flag(&i, argc, argv, "--alcance", &a.alcance)) continue;
            if(parse_int_flag(&i, argc, argv, "--lado", &a.lado)) continue;
            if(parse_int_flag(&i, argc, argv, "--iface", &a.iface)) continue;
            if(parse_int_flag(&i, argc, argv, "--bpm", &a.bpm)) continue;
            if(parse_int_flag(&i, argc, argv, "--compassos", &a.compassos)) continue;
            fprintf(stderr, "flag desconhecida: %s\n", argv[i]); uso(argv[0]); return 1;
        }
        if(a.dim < 1) a.dim = 1;
        if(a.compassos < 1) a.compassos = 1;
        if(a.compassos > 64) a.compassos = 64;
        if(a.bpm < 30) a.bpm = 30;
        if(a.bpm > 240) a.bpm = 240;
        /* iface tipica hexal se nao dada: 6 * 2^(alcance/3) */
        if(a.iface <= 0){
            int e = a.alcance / 3, p2 = 1, t;
            for(t = 0; t < e && t < 8; t++) p2 *= 2;
            a.iface = 6 * p2;
        }
        o = abre_out(&i, argc, argv);
        emite_tradutor(o, &a);
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
