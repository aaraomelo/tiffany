/* emite.c --- Assinatura(corpo) -> vozes TeX para o tradutor WASM
 *
 * Unico produto: papers/partitura_vozes.tex incluso por papers/partitura.tex.
 * NAO sonificacao. NAO LilyPond. NAO inventa fora da assinatura.
 *
 * Cadeia: corpo -> Pi -> pi_k -> Maestro -> vozes -> partitura (tex.wasm).
 *
 *   cc -O2 -std=c99 -Wall emite.c -o emite
 *   ./emite [-o ../papers/partitura_vozes.tex]
 *   ./emite peano [--dim N] ... [-o ../papers/partitura_vozes.tex]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int dim;
    int alcance;
    int lado;
    int iface;
    int bpm;
    int compassos;
} Assinatura;

typedef struct { int pitch; int rest; } Ev;

enum {
    V_BAT = 0, V_MAE, V_COR, V_MAD, V_MET, V_PER, V_L8
};

static Assinatura A_corpo_topologico(void){
    Assinatura a;
    a.dim = 8;
    a.alcance = 3;
    a.lado = 0;
    a.iface = 6;
    a.bpm = 72;
    a.compassos = 8;
    return a;
}

static void uso(const char *argv0){
    fprintf(stderr,
        "uso: %s [peano] [--dim N] [--alcance K] [--lado 0|1] [--iface N]\n"
        "         [--bpm N] [--compassos N] [-o papers/partitura_vozes.tex]\n"
        "\n"
        "Emite vozes TeX de Assinatura(corpo_topologico) para o tradutor WASM.\n",
        argv0);
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

static Ev ev_de(int voz, int c, int p, int *k8){
    Ev e;
    e.pitch = 0;
    e.rest = 0;
    switch(voz){
    case V_BAT: {
        static const int I[4] = { -1, 0, +1, 0 };
        int v = I[(c + p) % 4];
        if(v == 0) e.rest = 1;
        else if(v < 0) e.pitch = 0;
        else e.pitch = 4;
        break;
    }
    case V_MAE:
        e.pitch = 2;
        e.rest = (p != 0);
        break;
    case V_COR: {
        static const int m[4] = { 0, 1, 2, 3 };
        e.pitch = m[p];
        break;
    }
    case V_MAD: {
        static const int m[4] = { 2, 4, 2, 0 };
        e.pitch = m[(c + p) % 4];
        break;
    }
    case V_MET: {
        static const int m[4] = { 0, 2, 4, 3 };
        e.pitch = m[p];
        break;
    }
    case V_PER:
        if(p == 1 || p == 3) e.rest = 1;
        else e.pitch = (p == 0) ? 0 : 3;
        break;
    case V_L8:
        e.pitch = (*k8) % 8;
        (*k8)++;
        break;
    default:
        e.rest = 1;
        break;
    }
    return e;
}

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

static void tex_cabeca(FILE *o, char clef){
    if(clef == 'F') fputs("\\CabecaF{4}{4}", o);
    else if(clef == 'P') fputs("\\CabecaP{4}{4}", o);
    else fputs("\\CabecaG{4}{4}", o);
}

/* Página = andar do relógio do Maestro (Def. π_página).
 * Janela = meia volta (Dim/2): avança o relógio ⇒ P_n→P_{n+1}.
 * Todas as vozes em paralelo nessa janela — sem sistemas duplicados. */
static int andar_compassos(const Assinatura *a){
    int w = a->dim > 1 ? a->dim / 2 : 4;
    if(w < 4) w = 4;
    if(w > a->compassos) w = a->compassos;
    if(w < 1) w = 1;
    return w;
}

static void tex_voz_penta_faixa(FILE *o, const char *nome, const char *papel,
                               int voz, const Assinatura *a, char clef,
                               int c0, int c1){
    int c, p, k8 = c0 * 4;

    fprintf(o, "\\VozAbre{%s}{%s}{\\Penta}{-9.5mm}%%\n\\noindent",
            nome, papel);
    tex_cabeca(o, clef);

    for(c = c0; c < c1; c++){
        for(p = 0; p < 4; p++){
            Ev e = ev_de(voz, c, p, &k8);
            if(voz == V_BAT && !e.rest){
                static const int I[4] = { -1, 0, +1, 0 };
                if(I[(c + p) % 4] == +1) fputs("\\Acento", o);
            }
            tex_nota_y(o, e);
            fputs("\\,", o);
        }
        if(c + 1 < a->compassos) fputs("\\Bb", o);
        else fputs("\\BbFinal", o);
    }
    fputs("%\n\\par\n\n", o);
}

static void tex_voz_metro_faixa(FILE *o, const Assinatura *a, int c0, int c1){
    int c, p;

    fputs("\\VozMetro{Lei~6 --- eixo horizontal / tick}{-5.45mm}%\n", o);
    fputs("\\CabecaP{4}{4}", o);
    for(c = c0; c < c1; c++){
        for(p = 0; p < 4; p++) fputs("\\Tick\\,", o);
        if(c + 1 < a->compassos) fputs("\\Bb", o);
        else fputs("\\BbFinal", o);
    }
    fputs("%\n\\par\n\n", o);
}

static void emite_andar(FILE *o, const Assinatura *a, int c0, int c1){
    tex_voz_metro_faixa(o, a, c0, c1);
    tex_voz_penta_faixa(o, "Batuta",
        "vareta / Lei~3 trial $I\\in\\{-1,0,+1\\}$ (acento $=$ ataque)",
        V_BAT, a, 'F', c0, c1);
    tex_voz_penta_faixa(o, "Maestro",
        "$\\pi_k$ --- fonte da espiral (clave G)",
        V_MAE, a, 'G', c0, c1);
    tex_voz_penta_faixa(o, "Cordas (1)",
        "orquestra --- Hurwitz $\\mathbb{R}$",
        V_COR, a, 'G', c0, c1);
    tex_voz_penta_faixa(o, "Madeiras (2)",
        "orquestra --- Hurwitz $\\mathbb{C}$ (fase $i$)",
        V_MAD, a, 'G', c0, c1);
    tex_voz_penta_faixa(o, "Metais (4)",
        "orquestra --- Hurwitz $\\mathbb{H}$ ($\\Delta$)",
        V_MET, a, 'G', c0, c1);
    tex_voz_penta_faixa(o, "Percussao (8)",
        "orquestra --- Hurwitz $\\mathbb{O}$",
        V_PER, a, 'F', c0, c1);
    tex_voz_penta_faixa(o, "Lei 8",
        "espiral $\\mathbb{Z}/8\\mathbb{Z}$ no cone --- $\\mathrm{Ind}^{8}$",
        V_L8, a, 'G', c0, c1);
}

static void emite_peano_tex(FILE *o, const Assinatura *a){
    int andar = andar_compassos(a);
    int npag, pg, c0, c1;

    npag = (a->compassos + andar - 1) / andar;
    if(npag < 1) npag = 1;

    fprintf(o,
        "%% gerado por partitura/emite.c --- NAO editar a mao\n"
        "%% Assinatura(corpo_topologico) = Pi^{Peano}\n"
        "%% Dim=%d Alcance=%d Lado=%d Iface=%d bpm=%d compassos=%d\n"
        "%% Pagina = andar do relogio (janela=%d compassos = meia volta); vozes em paralelo.\n"
        "%% Incluir em papers/partitura.tex; desenhar com tests/tex (WASM).\n\n",
        a->dim, a->alcance, a->lado, a->iface, a->bpm, a->compassos, andar);

    for(pg = 0; pg < npag; pg++){
        c0 = pg * andar;
        c1 = c0 + andar;
        if(c1 > a->compassos) c1 = a->compassos;

        /* Página = dimensão do relógio: andar completo num suporte. */
        if(pg > 0)
            fputs("\\AndarPagina\n\n", o);
        else
            fputs("\\AndarAbre\n\\PartituraSecao\n", o);

        if(pg == 0){
            fprintf(o,
                "\\noindent{\\small\\textbf{Assinatura(corpo\\_peano)} --- Dim$=$%d · Alcance$=$%d ·\n"
                "Lado$=$%d · Iface$=$%d · Metronomo ($q{=}%d$) · %d compassos · $4/4$ · armadura C}\\par\n\n",
                a->dim, a->alcance, a->lado, a->iface, a->bpm, a->compassos);
        }

        fprintf(o,
            "\\noindent{\\small\\color{regua}Andar $P_{%d}$ --- compassos %d--%d"
            " (rel\\'ogio do Maestro)}\\par\n\n",
            pg, c0 + 1, c1);

        emite_andar(o, a, c0, c1);
    }
}

int main(int argc, char **argv){
    int i = 1;
    FILE *o;
    Assinatura a = A_corpo_topologico();

    if(i < argc && (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))){
        uso(argv[0]); return 0;
    }
    /* alias opcional: peano | tex */
    if(i < argc && (!strcmp(argv[i], "peano") || !strcmp(argv[i], "tex")))
        i++;

    if(!parse_assinatura(&i, argc, argv, &a)){ uso(argv[0]); return 1; }
    o = abre_out(&i, argc, argv);
    emite_peano_tex(o, &a);
    if(o != stdout) fclose(o);
    return 0;
}
