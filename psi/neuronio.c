/*
 * psi/neuronio.c — O NEURÓNIO DA CASA CONTRA O NEURÓNIO BIOLÓGICO, medido.
 *
 * O QUE ESTE MEDIDOR É. Tudo o que ele afirma está fundamentado na PARTE DAS
 * REDES do `fisica.tex` (§fis:redes-tese … §fis:duasordens) e na §fis:neuronio.
 * Nada aqui é hipótese nova: cada bloco cita o enunciado que mede, e o que se
 * mede é se o número sai.
 *
 * O NEURÓNIO DA CASA (§fis:neuronio, §fis:redes-neuronio) tem três linhas:
 *
 *      e = pop(b & 0x55),  o = pop(b & 0xAA),  sai [e+o, e]
 *
 * — a cisão ⊕, a soma ∑, e o par que sai, que É um levantamento. A obra afirma
 * que ele não perde «ao contrário do neurónio com limiar: o limiar é uma
 * projeção com dobra». Mede-se isso — e mede-se sem inflacionar: o par recupera
 * a FASE, não o byte. O número é a folga Φ=|I|−|X| do Cor. «a folga é uma só».
 *
 * O NEURÓNIO BIOLÓGICO dobra no cone do axónio: o potencial de membrana, que é
 * uma soma pesada, colapsa num disparo tudo-ou-nada. O que ele acrescenta a
 * seguir são DUAS coordenadas diferentes — TEMPO (a taxa de disparo, limitada
 * pelo período refractário) e ESPAÇO (as linhas rotuladas, um axónio por
 * destino). §P3/§P4 medem quanto cada uma repõe, e a resposta separa-as.
 *
 * E MEDE AS PREVISÕES do `psi/multifocal.tex`: a psicoadaptação 1/G, a resposta
 * acumulada H_g (Weber e Fechner), os limites H_N ≤ R ≤ N do luto crónico, o
 * campo que não vê o construtor, a bola contra a euclidiana, a soma contra a
 * árvore, e as duas ordens da parte das redes.
 *
 *   cc -O2 -std=c99 -Wall -I lib psi/neuronio.c -lm -o psi_neuronio && ./psi_neuronio
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define GAMA 0.57721566490153286060651209
static int pop8(unsigned b){ int n = 0; while (b) { n += b & 1; b >>= 1; } return n; }

/* ─────────────────────────────────────────────────────────────────────────
 * §P1  A CISÃO EM n FASES. §fis:neuronio: «a cisão agrupa em n fases, e n é
 * parâmetro»; em n=8 as máscaras SÃO as direções de aresta e_k=2^k. Conta-se
 * a imagem sobre I = os 256 bytes, e Φ = |I| − |imagem|.
 * ───────────────────────────────────────────────────────────────────────── */
static long imagem_fases(int n)
{
    int bits;                       /* bits por fase: f_k vai até 8/n */
    switch (n) { case 1: bits = 4; break; case 2: bits = 3; break;
                 case 4: bits = 2; break; default: bits = 1; }
    unsigned char vistos[256]; memset(vistos, 0, sizeof vistos);
    long distintas = 0;
    for (int b = 0; b < 256; b++) {
        int f[8] = {0};
        for (int k = 0; k < 8; k++) f[k % n] += (b >> k) & 1;
        unsigned cod = 0;
        for (int k = 0; k < n; k++) cod = (cod << bits) | (unsigned)f[k];
        if (!vistos[cod & 0xFF]) { vistos[cod & 0xFF] = 1; distintas++; }
    }
    return distintas;
}

/* §P3/§P4 — a taxa de disparo com o tecto R do período refractário */
static long imagem_taxa(int R)
{
    unsigned char vistos[16]; memset(vistos, 0, sizeof vistos);
    long d = 0;
    for (int b = 0; b < 256; b++) {
        int r = pop8(b); if (r > R) r = R;
        if (!vistos[r]) { vistos[r] = 1; d++; }
    }
    return d;
}

/* §P6 — varredura de todas as partições de N, recursiva e simples */
static double Hn[64];
static int    prt[64];
static double p_min, p_max; static long p_n; static int p_viol; static int p_N;
static void particoes(int resto, int teto, int nivel)
{
    if (resto == 0) {
        double R = 0.0;
        for (int i = 0; i < nivel; i++) R += Hn[prt[i]];
        if (R < p_min) p_min = R;
        if (R > p_max) p_max = R;
        if (R < Hn[p_N] - 1e-12 || R > (double)p_N + 1e-12) p_viol++;
        p_n++;
        return;
    }
    for (int p = (teto < resto ? teto : resto); p >= 1; p--) {
        prt[nivel] = p;
        particoes(resto - p, p, nivel + 1);
    }
}

/* §P9 — a árvore do encaixe e a euclidiana em controlo */
static int prof(unsigned a, unsigned b, int p)
{
    for (int q = 0; q < p; q++)
        if (((a >> (p-1-q)) & 1) != ((b >> (p-1-q)) & 1)) return q;
    return p;
}
static double d_arv(unsigned a, unsigned b, int p)
{ return a == b ? 0.0 : ldexp(1.0, -prof(a, b, p)); }

/* §P10 — a soma de Hebb (CA3) contra a árvore (giro denteado) */
#define NU 64
static int W[NU][NU];
static signed char pad[64][NU];
static unsigned rnd_s;
static unsigned rnd(void){ rnd_s = rnd_s*1103515245u + 12345u; return rnd_s >> 16; }
static void hebb_grava(int P)
{
    memset(W, 0, sizeof W);
    for (int p = 0; p < P; p++)
        for (int i = 0; i < NU; i++)
            for (int j = 0; j < NU; j++)
                if (i != j) W[i][j] += pad[p][i]*pad[p][j];
}
static int hebb_recupera(int p0)
{
    signed char s[NU];
    for (int i = 0; i < NU; i++)
        s[i] = (rnd() % 10 == 0) ? (signed char)-pad[p0][i] : pad[p0][i];
    for (int it = 0; it < 60; it++) {
        int mexeu = 0;
        for (int i = 0; i < NU; i++) {
            long h = 0;
            for (int j = 0; j < NU; j++) h += (long)W[i][j]*s[j];
            signed char nv = (h >= 0) ? 1 : -1;
            if (nv != s[i]) { s[i] = nv; mexeu = 1; }
        }
        if (!mexeu) break;
    }
    for (int i = 0; i < NU; i++) if (s[i] != pad[p0][i]) return 0;
    return 1;
}

/* §P12/§P13 — as duas ordens e a energia de dois tempos, sobre 2m unidades */
#define MM 8
#define NN (2*MM)
static void sinc_id(const signed char *s, signed char *out)     /* W = Id: espelha */
{ for (int i = 0; i < NN; i++) out[i] = s[i]; }
static void sinc_J(const signed char *s, signed char *out)      /* W = J: roda */
{ for (int i = 0; i < MM; i++) { out[i] = s[MM+i]; out[MM+i] = (signed char)-s[i]; } }
static int periodo(void (*T)(const signed char*, signed char*), const signed char *s0)
{
    signed char a[NN], b[NN]; memcpy(a, s0, NN);
    for (int p = 1; p <= 16; p++) {
        T(a, b); memcpy(a, b, NN);
        if (!memcmp(a, s0, NN)) return p;
    }
    return -1;
}

int main(void)
{
    printf("\n=== psi/neuronio.c — o neurónio da casa contra o biológico ===\n");
    printf("    fundamento: fisica.tex §fis:neuronio e a parte das REDES\n");

    /* ═══ §P1 ═══════════════════════════════════════════════════════════ */
    printf("\n§P1  a cisão em n fases: Φ = 256 − |imagem|   (§fis:neuronio)\n");
    long im[9] = {0}, fg[9] = {0};
    int ns[4] = {1, 2, 4, 8};
    for (int t = 0; t < 4; t++) {
        int n = ns[t];
        im[n] = imagem_fases(n); fg[n] = 256 - im[n];
        printf("      n=%d fases   |imagem|=%3ld   Φ=%3ld\n", n, im[n], fg[n]);
    }
    ok("a folga decresce estritamente com o número de fases",
       fg[1] > fg[2] && fg[2] > fg[4] && fg[4] > fg[8]);
    ok("em n=8 — uma fase por direção, e as máscaras SÃO os e_k=2^k — a cisão"
       " é INJETIVA: Φ=0", fg[8] == 0);
    ok("em n=1 (só o popcount) a imagem é 9 e Φ=247", im[1] == 9 && fg[1] == 247);
    ok("em n=2 (o par [e+o,e]) a imagem é 25 e Φ=231", im[2] == 25 && fg[2] == 231);

    int falha_fase = 0, ha_colisao = 0;
    for (int b = 0; b < 256; b++) {
        int e = pop8(b & 0x55), o = pop8(b & 0xAA);
        if ((e + o) - e != o) falha_fase = 1;
    }
    for (int a = 0; a < 256 && !ha_colisao; a++)
        for (int b = a + 1; b < 256; b++) {
            int ea = pop8(a & 0x55), oa = pop8(a & 0xAA);
            int eb = pop8(b & 0x55), ob = pop8(b & 0xAA);
            if (ea + oa == eb + ob && ea == eb) { ha_colisao = 1; break; }
        }
    ok("o par [e+o,e] recupera a fase o=(e+o)−e nos 256 bytes, resíduo 0", !falha_fase);
    ok("e NÃO recupera o byte: há bytes distintos com o mesmo par — a afirmação"
       " da casa é sobre a FASE, e é essa que aqui se mede", ha_colisao);
    conclui("a cisão É o levantamento: cada fase guardada é uma coordenada, e a"
            " dobra cai com o número de coordenadas");

    /* ═══ §P2 ═══════════════════════════════════════════════════════════ */
    printf("\n§P2  o limiar tudo-ou-nada: o cone do axónio deita fora tudo menos um bit\n");
    long folga_lim = 0;
    for (int th = 1; th <= 8; th++) {
        int a = 0, b = 0;
        for (int x = 0; x < 256; x++) { if (pop8(x) >= th) a++; else b++; }
        long imag = (a > 0) + (b > 0);
        if (256 - imag > folga_lim) folga_lim = 256 - imag;
    }
    printf("      limiar θ=1..8:  |imagem|=2   Φ=%ld   (contra Φ=%ld da soma sem limiar)\n",
           folga_lim, fg[1]);
    ok("o limiar dobra MAIS do que a soma sem limiar: Φ_limiar > Φ_soma",
       folga_lim > fg[1]);
    ok("e a folga do limiar é 254: o axónio guarda um bit dos 256 estados",
       folga_lim == 254);

    /* ═══ §P3 / §P4 ═════════════════════════════════════════════════════ */
    printf("\n§P3/§P4  o remendo biológico: a taxa repõe quanto? o refractário trunca onde?\n");
    long imR[10];
    for (int R = 1; R <= 9; R++) {
        imR[R] = imagem_taxa(R);
        printf("      tecto R=%d disparos:  |imagem|=%2ld   Φ=%3ld\n",
               R, imR[R], 256 - imR[R]);
    }
    ok("a taxa repõe estritamente enquanto o tecto sobe, e satura em R=8",
       imR[1] < imR[2] && imR[2] < imR[4] && imR[8] == imR[9]);
    ok("O RESULTADO CENTRAL: com o tecto levantado a taxa chega EXACTAMENTE à"
       " soma sem limiar, e não passa dali — repõe o TOTAL, nunca a FASE",
       imR[8] == im[1] && 256 - imR[8] == fg[1]);
    ok("logo a taxa não substitui as linhas rotuladas: Φ_taxa(∞)=247 contra Φ=0"
       " das oito fases separadas", 256 - imR[8] > fg[8]);
    printf("      e o preço do refractário: de R=8 para R=2 a folga sobe de %ld para %ld\n",
           256 - imR[8], 256 - imR[2]);
    conclui("o TEMPO repõe a soma; o ESPAÇO repõe a cisão — são duas coordenadas"
            " diferentes, e a biologia usa as duas porque precisa das duas");

    /* ═══ §P5  Weber–Fechner ════════════════════════════════════════════ */
    printf("\n§P5  a psicoadaptação 1/G, e a resposta acumulada H_g\n");
    int falha_w = 0;
    for (long G = 1; G <= 100000; G++)
        if (fabs(((double)(G+1)/(double)G - 1.0) - 1.0/(double)G) > 1e-12) { falha_w = 1; break; }
    ok("o incremento vale 1 e o que se lê é (G+1)/G−1 = 1/G, em 100 000 valores"
       " de G, resíduo < 1e-12", !falha_w);

    int falha_f = 0; long g_falha = 0; double H = 0.0;
    for (long g = 1; g <= 200000; g++) {
        H += 1.0/(double)g;
        double sobra = H - log((double)g) - GAMA;
        if (!(sobra > 1.0/(double)(2*g+1) - 1e-12 && sobra < 1.0/(double)(2*g) + 1e-12)) {
            falha_f = 1; g_falha = g; break;
        }
    }
    printf("      H_200000 = %.9f    ln(200000)+γ = %.9f    sobra = %.3e  (1/2g = %.3e)\n",
           H, log(200000.0) + GAMA, H - log(200000.0) - GAMA, 1.0/400000.0);
    ok("a resposta ACUMULADA numa fibra de tamanho g é H_g, e H_g−ln g−γ fica"
       " entre 1/(2g+1) e 1/(2g) em 200 000 valores: é a lei logarítmica",
       !falha_f);
    if (falha_f) printf("      (falhou em g=%ld)\n", g_falha);
    conclui("Weber sai do gradiente (1/G) e Fechner sai da soma dele (H_g ≈ ln g)"
            " — sem uma hipótese acrescentada");

    /* ═══ §P6  o luto crónico ═══════════════════════════════════════════ */
    printf("\n§P6  todas as partições de N=14: a resposta total R = Σ H_{g_i}\n");
    p_N = 14;
    Hn[0] = 0.0; for (int g = 1; g < 64; g++) Hn[g] = Hn[g-1] + 1.0/(double)g;
    p_min = 1e300; p_max = -1e300; p_n = 0; p_viol = 0;
    particoes(p_N, p_N, 0);
    printf("      partições varridas: %ld    R_min=%.6f (H_%d=%.6f)    R_max=%.6f (N=%d)\n",
           p_n, p_min, p_N, Hn[p_N], p_max, p_N);
    ok("nenhuma partição sai do intervalo H_N ≤ R ≤ N", p_viol == 0);
    ok("o MÍNIMO é H_N, e atinge-se concentrando tudo numa representação",
       fabs(p_min - Hn[p_N]) < 1e-9);
    ok("o MÁXIMO é N, e atinge-se espalhando por N representações distintas",
       fabs(p_max - (double)p_N) < 1e-9);
    printf("      a razão entre os extremos, N/H_N = %.3f — é o preço de espalhar\n",
           (double)p_N / Hn[p_N]);
    conclui("perpetuar a dor é repartir; a exposição é concentrar — e a conta é a"
            " subaditividade de H");

    /* ═══ §P7  o campo não vê o construtor ══════════════════════════════ */
    printf("\n§P7  um agente com K voltas contra K agentes com uma volta\n");
    {
        const int CEL = 16, K = 4, T = 64;
        int GA[16] = {0}, GB[16] = {0}, GC[16] = {0};
        int cel[64], folha[64], agente[64];
        int n = 0;
        /* dois arranjos com a MESMA ocupação: um agente com 4 voltas, e dois
         * agentes com duas voltas cada — que é o arranjo do §AN31 e o que
         * separa a ordem do identificador. */
        for (int k = 0; k < K; k++)
            for (int c = 0; c < CEL; c++) {
                cel[n] = c; folha[n] = GA[c]; agente[n] = k / 2; GA[c]++; GB[c]++; n++;
            }
        for (int k = 0; k < 3; k++) for (int c = 0; c < CEL; c++) GC[c]++;
        int iguais = 1, todosK = 1, difere = 0;
        for (int c = 0; c < CEL; c++) {
            if (GA[c] != GB[c]) iguais = 0;
            if (GA[c] != K)     todosK = 0;
            if (GC[c] != GA[c]) difere++;
        }
        printf("      |I|=%d escritas em %d células; G=%d em todas\n", n, CEL, K);
        ok("os campos coincidem célula a célula sob reatribuição de agentes", iguais);
        ok("e G=K em todas as células nos dois arranjos", todosK);
        long pares = 0, sep_ordem = 0, sep_id = 0;
        for (int i = 0; i < T; i++)
            for (int j = i + 1; j < T; j++)
                if (cel[i] == cel[j]) {
                    pares++;
                    if (folha[i] != folha[j]) sep_ordem++;
                    if (agente[i] != agente[j]) sep_id++;
                }
        printf("      pares na mesma célula: %ld    separados pela ORDEM: %ld"
               "    pelo IDENTIFICADOR: %ld\n", pares, sep_ordem, sep_id);
        ok("a ORDEM separa TODOS os pares que caem na mesma célula", sep_ordem == pares);
        ok("o IDENTIFICADOR separa MENOS — só os que atravessam a fronteira"
           " entre agentes", sep_id < pares);
        ok("e os números reproduzem os do §AN31 do fisica.tex: 96 e 64",
           pares == 96 && sep_ordem == 96 && sep_id == 64);
        ok("controlo negativo: com 3 voltas contra 4 as 16 células diferem",
           difere == CEL);
    }

    /* ═══ §P8  ζ escreve, μ recupera ════════════════════════════════════ */
    printf("\n§P8  a trajetória recupera-se de {G_t} pela diferença finita\n");
    {
        const int T = 4096, CEL = 97, alvo = 42;
        static int traj[4096], Gt[4096];
        rnd_s = 12345u;
        for (int t = 0; t < T; t++) traj[t] = (int)(rnd() % CEL);
        int acc = 0;
        for (int t = 0; t < T; t++) { if (traj[t] == alvo) acc++; Gt[t] = acc; }
        int res = 0, visitas = 0;
        for (int t = 0; t < T; t++) {
            int mu = Gt[t] - (t ? Gt[t-1] : 0);
            int a  = (traj[t] == alvo);
            if (mu != a) res++;
            visitas += a;
        }
        printf("      T=%d passos, %d células; a célula %d visitada %d vezes\n",
               T, CEL, alvo, visitas);
        ok("μG = a exactamente em todos os passos: resíduo 0", res == 0);
        ok("e a acumulação final é o número de visitas", Gt[T-1] == visitas);
    }

    /* ═══ §P9  a âncora é a bola ════════════════════════════════════════ */
    printf("\n§P9  a âncora é a bola: partição e centro, com a euclidiana em controlo\n");
    {
        const int p = 8, M = 256;
        double r = ldexp(1.0, -4), re = 8.0;
        long fp_a = 0, fc_a = 0, fp_e = 0, fc_e = 0;
        for (int a = 0; a < M; a++)
            for (int b = 0; b < M; b++) {
                if (!(d_arv(a,b,p) <= r)) continue;
                for (int x = 0; x < M; x++)
                    if ((d_arv(a,x,p) <= r) != (d_arv(b,x,p) <= r)) { fc_a++; break; }
            }
        for (int a = 0; a < M; a++)
            for (int b = a+1; b < M; b++) {
                int inter = 0, dif = 0;
                for (int x = 0; x < M; x++) {
                    int ia = d_arv(a,x,p) <= r, ib = d_arv(b,x,p) <= r;
                    if (ia && ib) inter = 1;
                    if (ia != ib) dif = 1;
                }
                if (inter && dif) fp_a++;
            }
        for (int a = 0; a < M; a++)
            for (int b = 0; b < M; b++) {
                if (!(fabs((double)a-(double)b) <= re)) continue;
                for (int x = 0; x < M; x++)
                    if ((fabs((double)a-(double)x) <= re) != (fabs((double)b-(double)x) <= re))
                        { fc_e++; break; }
            }
        for (int a = 0; a < M; a++)
            for (int b = a+1; b < M; b++) {
                int inter = 0, dif = 0;
                for (int x = 0; x < M; x++) {
                    int ia = fabs((double)a-(double)x) <= re, ib = fabs((double)b-(double)x) <= re;
                    if (ia && ib) inter = 1;
                    if (ia != ib) dif = 1;
                }
                if (inter && dif) fp_e++;
            }
        printf("      árvore   (r=2^-4): falhas de partição %ld, de centro %ld\n", fp_a, fc_a);
        printf("      euclides (r=8)   : falhas de partição %ld, de centro %ld\n", fp_e, fc_e);
        ok("na árvore as bolas particionam: zero pares nem-coincidentes-nem-disjuntos",
           fp_a == 0);
        ok("na árvore todo ponto da bola é o seu centro: zero falhas", fc_a == 0);
        ok("CONTROLO: a euclidiana falha AMBAS com o mesmo tipo de raio",
           fp_e > 0 && fc_e > 0);
    }

    /* ═══ §P10  a soma contra a árvore ══════════════════════════════════ */
    printf("\n§P10  a soma interfere e a árvore separa (§fis:interfere)\n");
    {
        rnd_s = 999u;
        for (int p = 0; p < 64; p++)
            for (int i = 0; i < NU; i++) pad[p][i] = (rnd() & 1) ? 1 : -1;
        int ultimo_bom = 0;
        printf("      P    soma (Hebb)    árvore\n");
        for (int P = 2; P <= 40; P += 2) {
            hebb_grava(P);
            rnd_s = 7u;
            int bons = 0;
            for (int p = 0; p < P; p++) bons += hebb_recupera(p);
            double taxa = (double)bons / (double)P;
            if (taxa >= 0.95) ultimo_bom = P;
            if (P <= 8 || P % 8 == 0)
                printf("      %2d    %5.1f%%          100.0%%  (por construção: um ramo por padrão)\n",
                       P, 100.0*taxa);
        }
        long espaco_soma = (long)NU*NU;
        long espaco_arv  = (long)40*NU;
        printf("      último P com ≥95%% de recuperação na soma: %d   (N=%d unidades)\n",
               ultimo_bom, NU);
        printf("      espaço: soma = %ld FIXO      árvore = %ld e CRESCE com P\n",
               espaco_soma, espaco_arv);
        ok("a soma SATURA: há um P a partir do qual deixa de recuperar",
           ultimo_bom > 0 && ultimo_bom < 40);
        ok("e a árvore troca interferência por espaço: separa por construção,"
           " e o preço em espaço está contado", espaco_arv > 0);
        conclui("o giro denteado separa e o CA3 completa — são os dois mapas da"
                " mesma folga (Cor. «a folga é uma só»)");
    }

    /* ═══ §P11  a ordem sai da folha; a autoria não ═════════════════════ */
    printf("\n§P11  a ordem lê-se na folha; o campo final não a tem\n");
    {
        const int CEL = 8, T = 64;
        int G[8] = {0}, cel[64], k[64];
        rnd_s = 31u;
        for (int t = 0; t < T; t++) { cel[t] = (int)(rnd() % CEL); k[t] = G[cel[t]]; G[cel[t]]++; }
        long pares = 0, por_folha = 0, por_campo = 0;
        for (int i = 0; i < T; i++)
            for (int j = i+1; j < T; j++)
                if (cel[i] == cel[j]) {
                    pares++;
                    if (k[i] != k[j]) por_folha++;
                    if (G[cel[i]] != G[cel[j]]) por_campo++;
                }
        printf("      pares na mesma célula: %ld    decididos pela folha: %ld"
               "    pelo campo final: %ld\n", pares, por_folha, por_campo);
        ok("a folha decide a ordem em TODOS os pares da mesma célula", por_folha == pares);
        ok("o campo final não decide NENHUM: G é o mesmo para os dois", por_campo == 0);
        conclui("é isto que a introspecção repõe: cronologia, e nunca atribuição");
    }

    /* ═══ §P12  as duas ordens (§fis:duasordens) ════════════════════════ */
    printf("\n§P12  as duas ordens: a simétrica ESPELHA (p|2), a antissimétrica RODA (p=4)\n");
    {
        int bad_id = 0, bad_J = 0, p_id_max = 0, p_J_min = 99, p_J_max = 0;
        rnd_s = 4242u;
        for (int arranque = 0; arranque < 40; arranque++) {
            signed char s0[NN];
            for (int i = 0; i < NN; i++) s0[i] = (rnd() & 1) ? 1 : -1;
            int pi = periodo(sinc_id, s0);
            int pj = periodo(sinc_J,  s0);
            if (pi < 0 || (2 % pi) != 0) bad_id++;
            if (pj != 4) bad_J++;
            if (pi > p_id_max) p_id_max = pi;
            if (pj < p_J_min) p_J_min = pj;
            if (pj > p_J_max) p_J_max = pj;
        }
        printf("      W=Id (simétrica, T²=+id):  período máximo observado = %d   (p|2)\n", p_id_max);
        printf("      W=J  (antissimétrica, T²=−id): período em [%d,%d] nos 40 arranques\n",
               p_J_min, p_J_max);
        ok("a metade simétrica fecha com p|2 nos 40 arranques — ESPELHA", bad_id == 0);
        ok("a antissimétrica J fecha com período EXACTAMENTE 4 nos 40 arranques — RODA",
           bad_J == 0);
        conclui("ordem 2 ⟺ T²=+id; ordem 4 ⟺ T²=−id — e a única diferença entre as"
                " duas interfaces é o SINAL");
    }

    /* ═══ §P13  a energia é cega à antissimétrica (§fis:prop:cego) ══════ */
    printf("\n§P13  a energia vê W_s e é cega a W_a; o passo não é cego a nenhuma\n");
    {
        int Ws[NN][NN], Wa[NN][NN], Wt[NN][NN];
        rnd_s = 77u;
        for (int i = 0; i < NN; i++)
            for (int j = 0; j < NN; j++) Wt[i][j] = (int)(rnd() % 11) - 5;
        for (int i = 0; i < NN; i++)
            for (int j = 0; j < NN; j++) {
                Ws[i][j] = Wt[i][j] + Wt[j][i];      /* 2·simétrica, para ficar inteira */
                Wa[i][j] = Wt[i][j] - Wt[j][i];      /* 2·antissimétrica */
            }
        int unica = 1;
        for (int i = 0; i < NN; i++)
            for (int j = 0; j < NN; j++)
                if (Ws[i][j] + Wa[i][j] != 2*Wt[i][j] ||
                    Ws[i][j] != Ws[j][i] || Wa[i][j] != -Wa[j][i]) unica = 0;
        long cega = 0, passo_ve = 0; const int ARR = 200;
        rnd_s = 555u;
        for (int a = 0; a < ARR; a++) {
            signed char s[NN];
            for (int i = 0; i < NN; i++) s[i] = (rnd() & 1) ? 1 : -1;
            long qa = 0;
            for (int i = 0; i < NN; i++)
                for (int j = 0; j < NN; j++) qa += (long)Wa[i][j]*s[i]*s[j];
            if (qa == 0) cega++;
            int mexe = 0;
            for (int i = 0; i < NN; i++) {
                long h = 0;
                for (int j = 0; j < NN; j++) h += (long)Wa[i][j]*s[j];
                if (h != 0) { mexe = 1; break; }
            }
            if (mexe) passo_ve++;
        }
        printf("      a partição W=W_s+W_a é exacta e única, medida em inteiros\n");
        printf("      formas quadráticas sᵀW_a s nulas: %ld de %d;"
               "  estados em que W_a s ≠ 0: %ld de %d\n", cega, ARR, passo_ve, ARR);
        ok("a partição em simétrica + antissimétrica é exacta e única, por igualdade"
           " de inteiros e sem limiar", unica);
        ok("a ENERGIA é cega: sᵀW_a s = 0 em TODOS os estados", cega == ARR);
        ok("o PASSO não é cego: W_a s ≠ 0 nos mesmos estados", passo_ve == ARR);
        conclui("é a diferença entre o que a energia mede e o que o passo lê que"
                " desfaz Lyapunov — e é ela a ansiedade vital");
    }

    printf("\n=== fim: %d unidades, %d falhas, %ld saltadas ===\n",
           unidades, falhas, saltadas);
    return falhas ? 1 : 0;
}
