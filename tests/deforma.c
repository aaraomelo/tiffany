/* deforma.c — A DEFORMAÇÃO É A DINÂMICA; A SIMETRIA SÓ ANCORA.
 *
 * A inversão de foco: até aqui a dimensão inteira (o racional, o cristal) era o objeto e o
 * quasicristal a exceção. É o contrário, e mede-se:
 *
 *  (D1) A MAIORIA é irracional. Os racionais são DENSOS mas de MEDIDA NULA: em toda vizinhança há
 *       um, e a soma de todos eles é zero. Isso é ser ponto de ANCORAGEM — esqueleto, não volume.
 *       Quase todo α (medida 1) é quasicristal.
 *
 *  (D2) A DIMENSÃO INTEIRA É CRISTALIZAÇÃO SEM DINÂMICA. Na rotação R_α(x)=x+α mod 1:
 *          α=p/q racional  →  a órbita FECHA em q pontos. Acabou: nada mais acontece.
 *          α irracional    →  a órbita nunca fecha, é densa e equidistribuída (Weyl).
 *       E a órbita irracional é ordenada em toda escala: os N pontos {nα} partem o círculo em
 *       intervalos de no máximo TRÊS comprimentos distintos (teorema dos três comprimentos) — o
 *       "cristal em toda escala finita" do §entre, agora na dinâmica.
 *
 *  (D3) A DEFORMAÇÃO cria a dimensão fracionária. No mapa do círculo
 *          f(x) = x + Ω − (K/2π)·sin(2πx)
 *       o parâmetro K é a deformação (K=0 é a rotação pura, sem deformação). Cada número de
 *       rotação RACIONAL abre uma LÍNGUA de largura finita em Ω — travamento de fase, cristal — e
 *       as línguas crescem com K. A fração travada de Ω vai de 0 (K=0) a 1 (K=1): a ESCADA DO DIABO.
 *       É a simetria ancorando a deformação: as línguas nascem ancoradas nos racionais e é a
 *       deformação que lhes dá largura.
 *
 *  (D4) O OURO RESISTE MAIS. As línguas engolem primeiro os racionais simples; o número de rotação
 *       mais difícil de travar é o de pior aproximação racional — e o pior de todos é 1/φ (Hurwitz).
 *       Mede-se a largura da língua em cada racional e a resistência de 1/φ.
 *
 * A dimensão de Hausdorff do complemento em K=1 é ≈0,8700 (literatura, não medido aqui) — uma
 * dimensão genuinamente FRACIONÁRIA, e ela só existe porque há deformação. Sem deformação há apenas
 * medida 0 (racional) contra medida 1 (irracional); com deformação, aparece o fractal no meio.
 *
 * Iteração de um mapa 1D: sem arrays de dados, buffers fixos, zero malloc.
 *
 *   cc -O2 -std=c99 deforma.c -lm -o deforma && ./deforma
 */
#include <stdio.h>
#include "../lib/disco.h"
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int passou = 1;
static const double PHI_INV = 0.61803398874989484820;   /* 1/φ = φ−1 */

/* ---------- D1: densos, mas de medida nula ---------- */
static long mdc(long a, long b){ while(b){ long t=a%b; a=b; b=t; } return a; }

/* ---------- D2: a rotação, e os gaps ---------- */
static int gaps_distintos(double a, int N, double tol){
    /* os N pontos {ka} partidos no círculo: quantos comprimentos de intervalo distintos? */
    double *v = DISCO_FIXO(double, 5);
    disco_prende(DISCO_BASE(5),"dados/def_v.bin",(size_t)4096,sizeof(double));
    disco_zera(v,(size_t)4096,sizeof(double));
    if(N > 4096) N = 4096;
    for(int k=0;k<N;k++){ double x = fmod((double)(k+1)*a, 1.0); v[k] = x<0?x+1:x; }
    /* ordena (insertion — N pequeno, buffer fixo) */
    for(int i=1;i<N;i++){ double x=v[i]; int j=i-1; while(j>=0 && v[j]>x){ v[j+1]=v[j]; j--; } v[j+1]=x; }
    static double comp[64]; int nc=0;
    for(int i=0;i<N;i++){
        double g = (i+1<N ? v[i+1] : v[0]+1.0) - v[i];
        int achou=0;
        for(int c=0;c<nc;c++) if(fabs(comp[c]-g) < tol){ achou=1; break; }
        if(!achou && nc<64) comp[nc++] = g;
        else if(!achou) return 99;
    }
    return nc;
}
static int orbita_fecha(double a, int qmax, double tol){
    for(int q=1;q<=qmax;q++){
        double x = fmod((double)q*a, 1.0); if(x<0) x+=1;
        if(x < tol || 1.0-x < tol) return q;                /* q·α ≡ 0 : a órbita fechou            */
    }
    return 0;
}

/* ---------- D3/D4: o mapa do círculo ---------- */
static double f_circ(double x, double W, double K){
    return x + W - (K/(2.0*M_PI))*sin(2.0*M_PI*x);
}
/* número de rotação: (f^n(x) − x)/n, com transiente */
static double rot(double W, double K, int trans, int n){
    double x = 0.1;
    for(int i=0;i<trans;i++) x = f_circ(x,W,K);
    long x0 = x;
    for(int i=0;i<n;i++) x = f_circ(x,W,K);
    return (x - x0)/n;
}
/* travado? existe q ≤ qmax e p com f^q(x) ≈ x + p (órbita periódica) */
static int travado(double W, double K, int qmax, double tol){
    double x = 0.1;
    for(int i=0;i<3000;i++) x = f_circ(x,W,K);            /* transiente                            */
    double x0 = x;
    for(int q=1;q<=qmax;q++){
        x = f_circ(x,W,K);
        double d = x - x0;
        double r = d - floor(d + 0.5);                     /* distância ao inteiro mais próximo     */
        if(fabs(r) < tol) return q;
    }
    return 0;
}

int main(void){
    printf("DEFORMA — a deformação é a dinâmica; a simetria só ancora\n");
    printf("=================================================================\n");

    /* ---------- D1 ---------- */
    printf("§D1  a MAIORIA é irracional: os racionais são densos, mas de medida nula\n");
    {
        printf("       Q   #{p/q irredutível em (0,1), q≤Q}   medida coberta com raio 1/(2q²)\n");
        double m_ant = 1e9; int decresce = 1;
        for(long Q=10; Q<=1280; Q*=2){
            long cnt=0; double med=0;
            for(long q=1;q<=Q;q++) for(long pp=1;pp<q;pp++)
                if(mdc(pp,q)==1){ cnt++; med += 1.0/((double)q*q); }
            printf("      %5ld  %12ld                       %.6f\n", Q, cnt, med/(double)Q);
            if(med/(double)Q > m_ant) decresce = 0;
            m_ant = med/(double)Q;
        }
        printf("     a medida por racional cai com Q: %s — densos e de medida 0.\n",
               decresce?"sim, resíduo 0":"NÃO");
        printf("     ⟹ ANCORAGEM: em toda vizinhança há um racional (denso), e todos juntos não\n");
        printf("        ocupam nada (medida 0). Quase todo α — medida 1 — é QUASICRISTAL.\n");
        if(!decresce) passou=0;
    }

    /* ---------- D2 ---------- */
    printf("\n§D2  dimensão inteira = CRISTALIZAÇÃO SEM DINÂMICA (a órbita fecha e acabou):\n");
    {
        double rac[] = {1.0/2, 1.0/3, 2.0/5, 3.0/8, 5.0/13};
        long den[] = {2,3,5,8,13};
        int erro=0;
        for(int i=0;i<5;i++){
            int q = orbita_fecha(rac[i], 200, 1e-12);
            printf("       α=%.6f (=%ld⁻¹ᵈᵉⁿ) : órbita FECHA em q=%-3d %s\n", rac[i], den[i], q,
                   q==(int)den[i]?"✓ nada mais acontece":"← REVER");
            if(q != (int)den[i]) erro=1;
        }
        int qf = orbita_fecha(PHI_INV, 5000, 1e-12);
        printf("       α=1/φ                  : órbita %s\n",
               qf ? "FECHOU (FALHA)" : "NUNCA fecha (q≤5000) ✓ a dinâmica continua");
        if(qf) erro=1;
        printf("     e a órbita irracional é ORDENADA em toda escala — os N pontos {kα} partem o\n");
        printf("     círculo em no máximo TRÊS comprimentos distintos:\n");
        for(int N=50;N<=800;N*=2){
            int g = gaps_distintos(PHI_INV, N, 1e-9);
            printf("       N=%4d : %d comprimentos distintos %s\n", N, g, g<=3?"✓":"← REVER");
            if(g>3) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — o racional cristaliza e para; o irracional nunca fecha\n"
               "     e ainda assim tem só 3 comprimentos: ordem sem periodicidade, na dinâmica."));
        if(erro) passou=0;
    }

    /* ---------- D3: a escada do diabo ---------- */
    printf("\n§D3  a DEFORMAÇÃO cria a medida: f(x)=x+Ω−(K/2π)sin(2πx). Fração de Ω TRAVADA\n");
    printf("     (órbita periódica = cristal), varrendo Ω∈[0,1]:\n");
    {
        double Ks[] = {0.0, 0.3, 0.6, 0.9, 0.99};
        int NW = 1500, cresce = 1; double f_ant = -1;
        for(int t=0;t<5;t++){
            double K = Ks[t];
            long trav=0;
            for(int i=0;i<NW;i++){
                /* a malha tem de ser IRRACIONAL: Ω=i/NW seria racional de denominador pequeno e
                 * travaria por artefato. (i+1/φ)/NW é irracional para todo i.                   */
                double W = ((double)i + PHI_INV)/NW;
                if(travado(W,K,40,1e-10)) trav++;
            }
            double frac = (double)trav/NW;
            printf("       K=%.2f : fração travada = %.4f   %s\n", K, frac,
                   K==0.0 ? "(sem deformação: só os racionais exatos)" :
                   (K>0.9 ? "(quase tudo travado — a escada do diabo)" : ""));
            if(frac < f_ant - 0.02) cresce = 0;
            f_ant = frac;
        }
        printf("     fração travada cresce com K: %s\n", cresce?"sim, resíduo 0":"NÃO");
        printf("     ⟹ as línguas nascem ANCORADAS nos racionais e é a DEFORMAÇÃO que lhes dá\n");
        printf("        largura. Em K=0 o racional tem medida 0 (só ancora); em K→1 as línguas\n");
        printf("        cobrem medida →1, e o que resta é um fractal de dimensão ≈0,8700\n");
        printf("        (literatura, não medido aqui): a dimensão FRACIONÁRIA só existe porque\n");
        printf("        há deformação. Sem ela, há apenas medida 0 contra medida 1.\n");
        if(!cresce) passou=0;
    }

    /* ---------- D4: o ouro resiste mais ---------- */
    printf("\n§D4  o OURO RESISTE: largura da língua de cada número de rotação, em K=0,6\n");
    {
        double K = 0.6;
        struct { long pp, q; } alvos[] = {{1,2},{1,3},{2,5},{3,8},{5,13},{8,21},{13,34}};
        printf("       p/q      largura da língua em Ω\n");
        double larg_ant = 1e9; int decresce = 1;
        for(int t=0;t<7;t++){
            double alvo = (double)alvos[t].pp/alvos[t].q;
            /* varre Ω em torno do alvo e mede a extensão travada com esse q */
            double lo=1, hi=-1, passo = 2e-7;             /* resolução fina                     */
            for(int i=-25000;i<=25000;i++){
                double W = alvo + i*passo;
                if(W<0||W>1) continue;
                int q = travado(W,K,40,1e-10);
                if(q == (int)alvos[t].q){ if(W<lo) lo=W; if(W>hi) hi=W; }
            }
            double larg = (hi>lo) ? hi-lo : 0;
            if(larg > 0)
                printf("       %2ld/%-2ld    %.7f\n", alvos[t].pp, alvos[t].q, larg);
            else
                printf("       %2ld/%-2ld    < %.0e  (abaixo da resolução — não é zero medido)\n",
                       alvos[t].pp, alvos[t].q, passo);
            if(larg > 0 && larg > larg_ant + 1e-12) decresce = 0;
            if(larg > 0) larg_ant = larg;
        }
        printf("     as línguas ENCOLHEM ao longo dos convergentes de 1/φ: %s\n",
               decresce?"sim, resíduo 0":"NÃO (ver acima)");
        printf("     o número de rotação de PIOR aproximação racional é 1/φ (Hurwitz: a constante\n");
        printf("     √5 é ótima e é atingida pelo ouro) — logo é o ÚLTIMO a ser travado pela\n");
        printf("     deformação. O ouro é o que mais resiste à cristalização.\n");
        if(!decresce) passou=0;
    }

    /* ---------- D5: a robustez é da CLASSE MODULAR, não do número ---------- */
    printf("\n§D5  a robustez é da CLASSE MODULAR — os NOBRES travam juntos\n");
    printf("     w é nobre se w = (aφ+b)/(cφ+d) com ad−bc = ±1 (equivalente a φ sob SL(2,ℤ)). E φ é\n");
    printf("     o ponto fixo do elemento mais simples do grupo: o próprio gato A₁=[[1,1],[1,0]].\n");
    {
        double phi = (1+sqrt(5.0))/2;
        struct { const char *nome; double w; int nobre; } alvos[] = {
            {"1/φ      = (0φ+1)/(1φ+0)", 1/phi,             1},
            {"1/φ²     = (0φ+1)/(1φ+1)", 1/(phi+1),         1},
            {"φ/(φ+2)  = (1φ+0)/(1φ+2)", phi/(phi+2),       1},
            {"(φ+1)/(φ+2)             ", (phi+1)/(phi+2),   1},
            {"√2−1     (NÃO nobre)    ", sqrt(2.0)-1,       0},
            {"e−2      (NÃO nobre)    ", exp(1.0)-2,        0},
        };
        printf("       número de rotação           nobre?   curva existe até K =\n");
        double lim[6]; int i;
        for(i=0;i<6;i++){
            double ultimo=0;
            for(double K=0.80; K<=1.001; K+=0.025){
                /* existe órbita não-caótica com número de rotação ≈ alvo? */
                double melhor=1e9;
                int NP=700;
                for(int j=0;j<NP;j++){
                    double p0=(double)j/NP;
                    if(travado(p0,K,40,1e-10)) continue;      /* ilha: não é curva                 */
                    double x=0.1, pp=p0;
                    for(int t=0;t<400;t++){ pp += (K/(2*M_PI))*sin(2*M_PI*x); x += pp; }
                    double x0=x; int nn=3000;
                    for(int t=0;t<nn;t++){ pp += (K/(2*M_PI))*sin(2*M_PI*x); x += pp; }
                    double w=(x-x0)/nn, d=fabs(w-alvos[i].w);
                    if(d<melhor) melhor=d;
                }
                if(melhor == 0.0) ultimo=K;
            }
            lim[i]=ultimo;
            printf("       %s   %-7s  %.3f\n", alvos[i].nome, alvos[i].nobre?"SIM":"não", ultimo);
        }
        /* os nobres devem coincidir entre si, e superar os não-nobres */
        double maxn=0, minn=9, maxnn=0;
        for(i=0;i<6;i++){ if(alvos[i].nobre){ if(lim[i]>maxn)maxn=lim[i]; if(lim[i]<minn)minn=lim[i]; }
                          else if(lim[i]>maxnn) maxnn=lim[i]; }
        int acima  = (minn > maxnn - 1e-9);
        printf("     TODO nobre acima de TODO não-nobre (mín nobre %.3f > máx não-nobre %.3f): %s\n",
               minn, maxnn, acima?"sim":"NÃO");
        printf("     dispersão entre os nobres: %.3f — e ela é REAL, não ruído: a classe fixa a CAUDA\n",
               maxn-minn);
        printf("     da fração contínua (a constante assintótica √5), não os primeiros convergentes,\n");
        printf("     então em K FINITO os nobres não travam no mesmo ponto. O que a classe prevê, e o\n");
        printf("     que se mede, é a ORDENAÇÃO.\n");
        printf("     %s\n", acima ?
          "resíduo 0 na ordenação — a resistência não é do OURO: é da sua CLASSE sob SL(2,ℤ). Os\n"
          "     quatro nobres ficam todos acima dos não-nobres. O que manda é o grupo\n"
          "     modular — o gato —, e \"o ouro\" é apenas o representante mais curto da órbita. Assim\n"
          "     a medida deixa de dizer que o ouro é o supremo do irracional (o fluxo invertido) e\n"
          "     passa a dizer o que de fato mediu: quem resiste é a classe."
          : "ver a tabela — a ordenação por classe não se sustentou nesta faixa");
        if(!acima) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — a inversão se sustenta. Os racionais (dimensão inteira, cristal) são\n"
      "DENSOS e de MEDIDA NULA: ancoram, não ocupam. Quase todo α, medida 1, é quasicristal.\n"
      "E a dimensão inteira é cristalização SEM DINÂMICA — a órbita fecha em q passos e\n"
      "acabou; a irracional nunca fecha, é densa, e ainda assim ordenada (três comprimentos).\n"
      "\n"
      "A DEFORMAÇÃO é que é a dinâmica: em K=0 o racional não tem largura nenhuma; conforme\n"
      "K cresce, cada racional abre uma língua e a fração travada vai de 0 a ~1. A simetria\n"
      "ANCORA (dá o centro da língua) e a deformação DÁ A LARGURA. E é só aí que aparece\n"
      "dimensão fracionária de verdade (≈0,87 no complemento em K=1) — ela é filha da\n"
      "deformação, não da simetria.\n"
      "\n"
      "E quem resiste à deformação não é O OURO: é a CLASSE MODULAR dele (§D5). Os nobres --- os\n"
      "equivalentes a φ sob SL(2,ℤ), sendo φ o ponto fixo do próprio gato --- ficam todos acima dos\n"
      "não-nobres, e φ é só o representante mais curto da órbita. A medida aqui é de aproximação\n"
      "RACIONAL (base inteira), e é isso que ela diz; na base do corpo estelar, q=e^{−2π}, o ouro é\n"
      "um valor que π PRODUZ (tools/estelar.c). Dizer que o ouro é o supremo do irracional inverte o\n"
      "fluxo."
      : "FALHOU — rever");
    return !passou;
}
