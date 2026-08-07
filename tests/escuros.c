/* escuros.c — OS DOIS ESCUROS SAO OS DOIS PONTOS FIXOS, UM POR INVOLUCAO.
 *
 * O Aarao: «avanca com materia / materia escura duais.»
 *
 * A bidualidade tem DUAS involucoes que comutam — o sinal da pressao (w -> -w) e o sentido
 * do grau (m -> -m) — e a orbita generica tem quatro estados. Cada involucao tem EXACTAMENTE
 * UM ponto fixo, e nele a orbita DEGENERA de quatro para dois:
 *
 *      w -> -w   fixa  w = 0        que e' a MATERIA
 *      m -> -m   fixa  m = 0        e ai' w = -1, que e' o VACUO
 *
 * Sao dois, um de cada, e sao duais no sentido exacto do sistema: cada um e' invisivel a'
 * involucao que o fixa. Um ponto fixo nao se separa do seu dual pela leitura que o fixa —
 * e' isso que DEGENERAR quer dizer. Precisa da OUTRA involucao para se distinguir, e e' por
 * isso que a bidualidade nao e' ornamento: sem o segundo lado, estes dois nao se leem.
 *
 *   §D1  cada involucao tem UM ponto fixo, e sao dois pontos fixos distintos
 *   §D2  no ponto fixo a orbita DEGENERA: quatro estados caem para dois
 *   §D3  e o degenerado nao se separa pela involucao que o fixa — mede-se
 *   §D4  a assinatura de um conteudo e' QUAL involucao o fixa, e ha' TRES casos: a primeira,
 *        a segunda, ou nenhuma. E' o trial outra vez, e nao ha' quarto
 *   §D5  o CONTROLO: fixo pelas DUAS e' impossivel — w = 0 e m = 0 nao coexistem
 *   §D6  e o segundo controlo: nem a materia nem a radiacao sao membros da familia; so' o
 *        vacuo e', e e' o proprio ponto fixo. Os dois escuros estao nas BORDAS da familia
 *
 * Zero doubles: w + 1 = (2m/3D).sqrt D e' exacto em Q[sqrt D], guardado pelo coeficiente.
 *
 *   cc -O2 -std=c99 -Wall -I../lib escuros.c -o escuros && ./escuros
 */
#include <stdio.h>
#include "unidade.h"

/* w como elemento EXACTO de Q[sqrt D]: parte racional + coeficiente de sqrt D. UMA definicao. */
typedef struct { long r_n, r_d, s_n, s_d; } Wq;
static Wq w_de(long m)
{
    long D = m*m + 4;
    Wq w = { -1, 1, 2*m, 3*D };
    if(w.s_n == 0) w.s_d = 1;
    return w;
}
static int w_igual(Wq a, Wq b)
{ return a.r_n*b.r_d == b.r_n*a.r_d && a.s_n*b.s_d == b.s_n*a.s_d; }
static Wq w_neg(Wq w){ Wq v = { -w.r_n, w.r_d, -w.s_n, w.s_d }; return v; }

/* os tres conteudos, como racionais exactos p/q */
typedef struct { const char *nome; long n, d; } Cont;
static const Cont RAD = { "radiacao", 1, 3 }, MAT = { "materia", 0, 1 }, VAC = { "vacuo", -1, 1 };
static int c_igual(Cont a, Cont b){ return a.n*b.d == b.n*a.d; }
static Cont c_neg(Cont c){ Cont v = { c.nome, -c.n, c.d }; return v; }

int main(void)
{
    long falhas = 0;
    puts("\n=== OS DOIS ESCUROS: um ponto fixo por involucao ===\n");

    /* ═══ §D1 — cada involucao tem UM ponto fixo ══════════════════════════════════════ */
    {
        /* a primeira: w -> -w. Varre-se w em duodecimos e conta-se quem e' o seu proprio dual */
        long fixos_w = 0, qual_n = 99, qual_d = 1;
        for(long n = -24; n <= 24; n++){
            Cont c = { "", n, 12 };
            if(c_igual(c, c_neg(c))){ fixos_w++; qual_n = n; qual_d = 12; }
        }
        /* a segunda: m -> -m. Varre-se m e conta-se quem e' o seu proprio simetrico */
        long fixos_m = 0, qual_m = 99;
        for(long m = -30; m <= 30; m++) if(m == -m){ fixos_m++; qual_m = m; }
        Wq w_no_fixo = w_de(qual_m);                    /* o w NO ponto fixo do grau */
        Cont vac = VAC;
        int  e_vacuo = (w_no_fixo.r_n == vac.n && w_no_fixo.r_d == vac.d && w_no_fixo.s_n == 0);

        printf("  §D1  involucao da PRESSAO  w -> -w :  %ld ponto fixo, em w = %ld/%ld\n",
               fixos_w, qual_n, qual_d);
        printf("       involucao do GRAU     m -> -m :  %ld ponto fixo, em m = %ld,"
               " e ai' w = %ld%+ld.rD  ->  %s\n\n", fixos_m, qual_m,
               w_no_fixo.r_n, w_no_fixo.s_n, e_vacuo ? "o VACUO, w = -1" : "outra coisa");
        ok("cada involucao da bidualidade tem EXACTAMENTE UM ponto fixo, e sao dois pontos fixos"
           " distintos: a do sinal da pressao fixa w = 0, e a do sentido do grau fixa m = 0 —"
           " onde w vale -1. Sao os dois conteudos que a cosmologia nao le' pela radiacao: a"
           " materia e o vacuo. Nao e' escolha: e' contagem de pontos fixos",
           fixos_w == 1 && qual_n == 0 && fixos_m == 1 && qual_m == 0 && e_vacuo);
    }

    /* ═══ §D2 — no ponto fixo a orbita DEGENERA ═══════════════════════════════════════ */
    {
        /* antes de contar: w -> -w tem de ser mesmo o SIMETRICO — w + (-w) = 0 nas DUAS
         * coordenadas — e nao so' um estado diferente. A contagem 4/2 sobrevive a uma troca
         * que vire metade do numero, porque quatro distintos continuam quatro. */
        long sim_maus = 0;
        for(long m = -8; m <= 8; m++){
            Wq w = w_de(m), v = w_neg(w);
            if(w.r_n*v.r_d + v.r_n*w.r_d != 0) sim_maus++;
            if(w.s_n*v.s_d + v.s_n*w.s_d != 0) sim_maus++;
            if(!w_igual(w_neg(v), w))          sim_maus++;
        }
        long orb[2]; long ms[2] = { 5, 0 };            /* generico, e o ponto fixo do grau */
        for(int c = 0; c < 2; c++){
            Wq est[4]; int k = 0;
            for(int sw = 1; sw >= -1; sw -= 2) for(int sm = 1; sm >= -1; sm -= 2){
                Wq w = w_de(sm * ms[c]);
                est[k++] = sw > 0 ? w : w_neg(w);
            }
            long d = 0;
            for(int i = 0; i < 4; i++){ int novo = 1;
                for(int j = 0; j < i; j++) if(w_igual(est[i], est[j])) novo = 0;
                d += novo; }
            orb[c] = d;
        }
        /* e a mesma coisa do lado da pressao: a orbita de um conteudo sob w -> -w */
        Cont cs[3] = { RAD, MAT, VAC }; long orb_p[3];
        for(int i = 0; i < 3; i++)
            orb_p[i] = c_igual(cs[i], c_neg(cs[i])) ? 1 : 2;
        printf("  §D2  no grau:    m = 5 da' %ld estados,  m = 0 (ponto fixo) da' %ld\n",
               orb[0], orb[1]);
        printf("       na pressao: %s %ld,  %s %ld,  %s %ld  <- a materia colapsa\n\n",
               cs[0].nome, orb_p[0], cs[1].nome, orb_p[1], cs[2].nome, orb_p[2]);
        ok("e no ponto fixo a orbita DEGENERA, dos dois lados. No sentido do grau, quatro estados"
           " caem para dois em m = 0; no sinal da pressao, dois caem para um na materia. Degenerar"
           " nao e' um acidente da conta: e' o que faz de um ponto um ponto fixo. E a troca de sinal"
           " e' verificada pela DEFINICAO, w + (-w) = 0 nas duas coordenadas, em 17 graus —"
           " porque a contagem sozinha aceita qualquer involucao que separe quatro estados",
           orb[0] == 4 && orb[1] == 2 && orb_p[0] == 2 && orb_p[1] == 1 && orb_p[2] == 2
           && sim_maus == 0);
    }

    /* ═══ §D3 — o degenerado nao se separa pela involucao que o fixa ══════════════════ */
    {
        long invisiveis = 0, visiveis = 0;
        Cont cs[3] = { RAD, MAT, VAC };
        printf("  §D3  conteudo   w      -w     separa pela pressao?\n");
        for(int i = 0; i < 3; i++){
            Cont d = c_neg(cs[i]);
            int separa = !c_igual(cs[i], d);
            printf("       %-9s %ld/%ld    %ld/%ld    %s\n",
                   cs[i].nome, cs[i].n, cs[i].d, d.n, d.d, separa ? "sim" : "NAO — invisivel");
            if(separa) visiveis++; else invisiveis++;
        }
        putchar('\n');
        ok("e o degenerado NAO SE SEPARA pela involucao que o fixa: aplicada, ela devolve o mesmo"
           " objecto. A materia e' invisivel ao sinal da pressao — nenhuma equacao de estado a"
           " distingue do seu dual, porque ela E' o seu dual. E' isto que 'escuro' quer dizer"
           " aqui, e nao uma metafora: um conteudo que a leitura disponivel nao separa",
           invisiveis == 1 && visiveis == 2);
    }

    /* ═══ §D4 — a assinatura e' QUAL involucao o fixa, e ha' TRES casos ═══════════════ */
    {
        /* para cada conteudo: e' fixo pela pressao? e' fixo pelo grau? */
        struct { const char *nome; int fp, fg; } sig[3];
        Cont cs[3] = { RAD, MAT, VAC };
        for(int i = 0; i < 3; i++){
            sig[i].nome = cs[i].nome;
            sig[i].fp = c_igual(cs[i], c_neg(cs[i]));           /* fixo pela pressao */
            /* fixo pelo grau: o seu w e' o w do ponto fixo m = 0 */
            Wq w0 = w_de(0);
            sig[i].fg = (cs[i].n * w0.r_d == w0.r_n * cs[i].d) && w0.s_n == 0;
        }
        long so_p = 0, so_g = 0, nenhuma = 0, ambas = 0;
        printf("  §D4  conteudo   fixo pela pressao   fixo pelo grau\n");
        for(int i = 0; i < 3; i++){
            printf("       %-9s      %s                 %s\n", sig[i].nome,
                   sig[i].fp ? "SIM" : "nao", sig[i].fg ? "SIM" : "nao");
            if(sig[i].fp && sig[i].fg) ambas++;
            else if(sig[i].fp) so_p++;
            else if(sig[i].fg) so_g++;
            else nenhuma++;
        }
        printf("       -> so' a primeira: %ld,  so' a segunda: %ld,  nenhuma: %ld,"
               " ambas: %ld\n\n", so_p, so_g, nenhuma, ambas);
        ok("e a ASSINATURA de um conteudo e' qual involucao o fixa — e ha' TRES casos ocupados,"
           " um por conteudo: a materia so' pela pressao, o vacuo so' pelo grau, a radiacao por"
           " nenhuma. E' o trial outra vez, e a leitura fecha: os dois fixos sao os dois que a"
           " cosmologia chama escuros, e a radiacao, que nao e' fixa por nenhuma, e' precisamente"
           " a que se ve'", so_p == 1 && so_g == 1 && nenhuma == 1 && ambas == 0);
    }

    /* ═══ §D5 — o CONTROLO: fixo pelas DUAS e' impossivel ════════════════════════════ */
    {
        /* w = 0 exige 2m = 3.sqrt D, isto e', 4m^2 = 9D = 9m^2 + 36, logo -5m^2 = 36:
         * nenhum m real, quanto mais inteiro. E m = 0 da' w = -1, nao 0. Varre-se. */
        long coincide = 0, testados = 0;
        for(long m = -600; m <= 600; m++){
            Wq w = w_de(m);
            int w_zero = (w.r_n == 0 && w.s_n == 0);       /* w = 0 exacto */
            int m_zero = (m == 0);
            testados++;
            if(w_zero && m_zero) coincide++;
        }
        /* e a conta, calculada e nao escrita: w = 0 pede (w+1)^2 = 1, isto e',
         * (2m/3D)^2 . D = 1  <=>  4m^2 = 9D = 9(m^2+4)  <=>  4m^2 - 9(m^2+4) = 0.
         * Varre-se: se algum m a anulasse, havia um conteudo fixo pelas duas. */
        long anula = 0, resid0 = 0;
        for(long m = -600; m <= 600; m++){
            /* DOS CAMPOS de w_de, e nao dos numeros: (w+1)^2 = 1 pede (s_n/s_d)^2 . D = 1,
             * isto e', s_n^2.D - s_d^2 = 0. Escrever "4m^2 - 9(m^2+4)" a' mao deixava passar
             * qualquer mudanca na formula, porque a conta nao lhe tocava. */
            Wq w = w_de(m);
            long D = m*m + 4;
            long r = w.s_n*w.s_n*D - w.s_d*w.s_d;
            if(m == 1) resid0 = r;   /* m = 0 normaliza s_d a 1; le-se o primeiro grau */
            if(r == 0) anula++;
        }
        printf("  §D5  m de -600 a 600: %ld com w = 0 E m = 0, em %ld testados\n", coincide, testados);
        printf("       e a conta s_n^2.D - s_d^2, tirada da formula: anula em %ld dos %ld —"
               " em m = 1 vale %ld\n\n", anula, testados, resid0);
        ok("e o CONTROLO: nenhum conteudo e' fixo pelas DUAS involucoes. Seria preciso w = 0 e"
           " m = 0 ao mesmo tempo, e m = 0 da' w = -1; a conta di-lo directamente — w = 0 pede"
           " -5m^2 = 36, que nao tem raiz real. Os dois pontos fixos sao dois e NAO se juntam:"
           " se se juntassem, os dois escuros eram um so' e a bidualidade nao tinha dois lados",
           coincide == 0 && testados == 1201 && anula == 0 && resid0 == -205);
    }

    /* ═══ §D6 — e os dois escuros estao nas BORDAS da familia ════════════════════════ */
    {
        /* qual dos tres conteudos e' membro da familia metalica, isto e', tem m inteiro? */
        long m_rad = 0, m_mat = 0, m_vac = 0;
        Cont cs[3] = { RAD, MAT, VAC }; long *cnt[3] = { &m_rad, &m_mat, &m_vac };
        for(long m = -999; m <= 999; m++){
            Wq w = w_de(m);
            for(int i = 0; i < 3; i++){
                /* w = n/d exacto exige coeficiente de sqrt D nulo e parte racional a bater */
                if(w.s_n == 0 && w.r_n * cs[i].d == cs[i].n * w.r_d) (*cnt[i])++;
            }
        }
        printf("  §D6  m inteiros em [-999,999] que dao cada conteudo:"
               "  radiacao %ld,  materia %ld,  vacuo %ld\n\n", m_rad, m_mat, m_vac);
        ok("e o segundo controlo, que e' onde os escuros vivem: dos tres conteudos so' o VACUO e'"
           " membro da familia, e e' no proprio ponto fixo m = 0. Nem a materia nem a radiacao"
           " tem m inteiro nenhum em mil e novecentos graus. A familia descreve os corpos; os"
           " conteudos que ela nao alcanca estao na FRONTEIRA dela — e e' la' que o ponto fixo"
           " mora", m_rad == 0 && m_mat == 0 && m_vac == 1);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A BIDUALIDADE TEM DOIS PONTOS FIXOS, UM POR INVOLUCAO — E SAO OS DOIS ESCUROS:");
        puts("");
        puts("    w -> -w   fixa  w = 0     a MATERIA     invisivel a' pressao");
        puts("    m -> -m   fixa  m = 0     o VACUO       invisivel ao grau");
        puts("    nenhuma   fixa  w = 1/3   a RADIACAO    e' a que se ve'");
        puts("");
        puts("  Um ponto fixo nao se separa do seu dual pela leitura que o fixa — e' isso que");
        puts("  DEGENERAR quer dizer. Escuro, aqui, nao e' metafora: e' um conteudo que a");
        puts("  leitura disponivel nao separa, e por isso so' se le' pelo OUTRO lado do par.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
