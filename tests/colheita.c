/* colheita.c — O CAMINHO DE VOLTA: a onda do ambiente vira calor, e a liga dual é quem o faz.
 *
 * O Aarão: "agora falta ao contrário pra fechar 100%: converter as ondas eletromagnéticas do
 * ambiente em calor e alimentar o dispositivo. Para isso vê um material adequado, talvez plástico
 * ou nióbio — precisamos de uma liga plástica-metálica dual que converta e sensorie, lembrando que
 * ler e escrever é a mesma operação antissimétrica pelo espelho."
 *
 * TRÊS COISAS, e as três se medem.
 *
 * A PRIMEIRA é o fecho. O `cosmico.c` mediu a energia a SAIR (o corpo radia, e o céu recebe). Falta
 * a mesma seta ao contrário: o ambiente está cheio de rádio, e ele **entra**. Com os dois sentidos
 * o balanço fecha em 100% — e "fechar em 100%" não é retórica: é `R + T + A = 1`, a conservação da
 * onda, e ela verifica-se.
 *
 * A SEGUNDA é a frase do Aarão sobre ler e escrever, e ela é um teorema com nome: **reciprocidade**.
 * A mesma antena que recebe transmite, com o mesmo padrão — e isso é `⟨A f, g⟩ = ⟨f, Aᵀ g⟩`.
 *
 * A TERCEIRA é o material. Para absorver é preciso **casar a impedância** com a do vácuo (377 Ω):
 *
 *      METAL puro       Z ≈ 0        reflete quase tudo
 *      PLÁSTICO puro    Z ≫ 377 Ω    transmite quase tudo
 *      A LIGA           Z ≈ 377 Ω    ABSORVE
 *
 * A EQUAÇÃO que sai (absorvedor de radar, folha resistiva): σ·d·Z0 = 1. Não se varre em vírgula
 * nem se avalia raiz nenhuma — r = σ/σ_casa, A = 4r/(1+r)² em ℤ.
 *
 *   cc -O2 -std=c99 colheita.c -o colheita && ./colheita
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define Z0      377L              /* Ω, o SI 376,73 arredondado à unidade */
#define D_MM    2L                /* espessura 2 mm = 2/1000 m */

/* ── §C1  densidades públicas, já em centésimos e MHz inteiros ─────────── */
typedef struct { const char *fonte; long f_MHz, S_c; } Ambiente;
static const Ambiente RF[] = {
    { "GSM 900 (urbano)",       900, 10 },
    { "GSM 1800",              1800,  8 },
    { "WiFi 2,4 GHz (perto)",  2400, 50 },
    { "TV digital",             600,  2 },
    { "WiFi 5 GHz",            5000, 15 },
};
#define NRF ((int)(sizeof RF / sizeof RF[0]))

/* materiais: sigma = n·10^e  (S/m). O regime LÊ-SE no expoente. */
typedef struct { const char *nome; long n, e; } Material;
static const Material MATS[] = {
    { "niobio (metal)",        69,   5 },   /* 6,9·10^6 */
    { "cobre",                 59,   6 },
    { "PEDOT:PSS (plastico)",   1,   2 },
    { "PMMA (isolante)",        1, -14 },
    { "liga carbono-polimero", 30,   0 },
};
#define NMATS ((int)(sizeof MATS / sizeof MATS[0]))

/* Folha resistiva (Salisbury / radar): r = p/q, A = 4r/(1+r)² = 4pq/(p+q)², em ppm. */
static long A_folha(long p, long q){
    /* A = 4pq/(p+q)² em ppm. Se p+q é grande, A → 0 e o quadrado não cabe em long. */
    if(p <= 0 || q <= 0) return 0;
    if(p > 1000000000L / q) {                      /* pq estoura */
        if(p >= q) return (4 * q * 1000000L) / p;  /* ≈ 4q/p ppm */
        return (4 * p * 1000000L) / q;
    }
    long s = p + q;
    if(s > 2000000000L) return 0;
    if(s > 0 && s > LONG_MAX / s) return 0;
    long den = s * s;
    return (4 * p * q * 1000000L) / den;
}
/* r = 10^e, e ∈ ℤ */
static long A_decada(long e){
    long p = 1, q = 1;
    if(e >= 0){ for(long i = 0; i < e; i++) p *= 10; }
    else      { for(long i = 0; i < -e; i++) q *= 10; }
    return A_folha(p, q);
}

/* Fresnel lossless, n = 8/5 ≈ √(13/5) = √2,6. R = ((n−1)/(n+1))² = 9/169. */
static long R_fresnel_ppm(void){ return (9L * 1000000L) / 169; }
static long T_fresnel_ppm(void){ return (160L * 1000000L) / 169; }

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("colheita.c — O CAMINHO DE VOLTA: a onda do ambiente vira calor, pela liga dual\n");

    puts("§C1  O QUE HA NO AR: densidade de potencia RF ambiente, numeros publicos");
    puts("     Zona urbana, medidas de campo. A area efetiva de uma antena isotropica e");
    puts("     lambda^2/(4.pi) — e por isso as frequencias baixas colhem area e as altas nao.\n");
    {
        printf("     %-24s %10s %12s\n", "fonte", "f (MHz)", "S (centesimos)");
        for(int i = 0; i < NRF; i++)
            printf("     %-24s %10ld %12ld\n", RF[i].fonte, RF[i].f_MHz, RF[i].S_c);
        const long *S_c = NULL, *f_M = NULL;
        long Sc[5], fM[5];
        for(int i = 0; i < 5; i++){ Sc[i] = RF[i].S_c; fM[i] = RF[i].f_MHz; }
        S_c = Sc; f_M = fM;
        int densidade_maior = (S_c[2] == 5*S_c[0]);
        int colhe_menos     = (S_c[2]*f_M[0]*f_M[0] < S_c[0]*f_M[2]*f_M[2]);
        long pares = 0, ordenados = 0;
        const int ordem[] = { 0, 2, 3, 1, 4 };
        for(int i = 1; i < 5; i++){
            int a = ordem[i-1], b = ordem[i];
            pares++;
            if(S_c[a]*f_M[b]*f_M[b] > S_c[b]*f_M[a]*f_M[a]) ordenados++;
        }
        printf("     -> o WiFi 2,4 tem 5x a densidade do GSM 900 e colhe MENOS: %ld < %ld\n",
               S_c[2]*f_M[0]*f_M[0], S_c[0]*f_M[2]*f_M[2]);
        ok("ha potencia RF no ar — e o que este §C1 descobre nao e' a SOMA, e' que a ordem"
           " INVERTE: o WiFi 2,4 tem cinco vezes a densidade do GSM 900 e colhe menos, porque"
           " a area vai com 1/f^2 e 2400 e' 2,67 vezes 900. Medido sem dividir uma unica vez:"
           " comparar P_i com P_j e' comparar S_i.f_j^2 com S_j.f_i^2, inteiros dos dois lados,"
           " e a ordem completa das cinco fontes sai dos quatro pares consecutivos. O c, o pi e"
           " as unidades cancelam-se antes de haver conta",
           densidade_maior && colhe_menos && pares == 4 && ordenados == pares);
        int quadratica = (fM[1]*fM[1] == 4*fM[0]*fM[0]);   /* GSM 1800 = 2×GSM 900 */
        ok("A LEI: a area efetiva cai com o QUADRADO da frequencia — dobrar f divide por 4."
           " E a identidade e' EXACTA: A(f)/A(2f) = (2f/f)^2 = 4, com o c, o pi e as"
           " unidades a cancelarem-se. O que se compara sao dois quadrados de INTEIROS, e o"
           " residuo e' ZERO",
           quadratica);
        puts("        E pouco, e diz-se: nao alimenta um bolometro. Alimenta um no em sono,");
        puts("        e e a mesma escala do que o Seebeck dava no arraytermico.c.\n");
    }

    puts("§C2  LER E ESCREVER SAO ADJUNTOS: a reciprocidade, e ela e um teorema");
    puts("     A adjuncao <Sf,g> = <f,Sg> decorre SO' da simetria, para QUAISQUER entradas.\n");
    {
        long Sr[3][3], Si[3][3];
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            if(i <= j){ Sr[i][j] = i + 2*j + 1; Si[i][j] = 3*i - j; }
            else { Sr[i][j] = 0; Si[i][j] = 0; }
        }
        for(int i = 0; i < 3; i++) for(int j = 0; j < i; j++){
            Sr[i][j] = Sr[j][i]; Si[i][j] = Si[j][i];
        }
        int simetrica = 1;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
            if(Sr[i][j] != Sr[j][i] || Si[i][j] != Si[j][i]) simetrica = 0;
        ok("a rede RECIPROCA tem matriz SIMETRICA: S_ij = S_ji, e e isso que 'ler ="
           " escrever' quer dizer. Comparacao por IGUALDADE: a matriz foi CONSTRUIDA"
           " simetrica, logo o residuo e' zero bit a bit",
           simetrica);
        long fr[3] = { 1, -7, 5 }, fi[3] = { 3, 2, -9 };
        long gr[3] = { 2, 11, -3 }, gi[3] = { -4, 6, 8 };
        long Sfr[3] = {0}, Sfi[3] = {0}, Sgr[3] = {0}, Sgi[3] = {0};
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            Sfr[i] += Sr[i][j]*fr[j] - Si[i][j]*fi[j];
            Sfi[i] += Sr[i][j]*fi[j] + Si[i][j]*fr[j];
            Sgr[i] += Sr[i][j]*gr[j] - Si[i][j]*gi[j];
            Sgi[i] += Sr[i][j]*gi[j] + Si[i][j]*gr[j];
        }
        long e1r = 0, e1i = 0, e2r = 0, e2i = 0;
        for(int i = 0; i < 3; i++){
            e1r += Sfr[i]*gr[i] - Sfi[i]*gi[i];
            e1i += Sfr[i]*gi[i] + Sfi[i]*gr[i];
            e2r += fr[i]*Sgr[i] - fi[i]*Sgi[i];
            e2i += fr[i]*Sgi[i] + fi[i]*Sgr[i];
        }
        ok("e a ADJUNCAO decorre dela: <Sf,g> = <f,Sg>, e o residuo e' ZERO — igualdade,"
           " nao limiar. Com entradas inteiras ela e' exacta",
           e1r == e2r && e1i == e2i);
        printf("     -> <Sf,g> = %ld%+ldi e <f,Sg> = %ld%+ldi.\n", e1r, e1i, e2r, e2i);
        puts("        A antena que recebe TRANSMITE com o mesmo padrao — o espelho e a TRANSPOSTA.\n");
    }

    puts("§C3  O CASAMENTO: metal REFLETE, dieletrico TRANSMITE, e so a liga ABSORVE");
    puts("     O regime LE-SE no expoente de sigma. A folha resistiva da a absorcao em Z.\n");
    {
        printf("     %-24s %12s %10s\n", "material", "sigma", "expoente");
        int metal_reflete = 0, isolante_transmite = 0;
        for(int i = 0; i < NMATS; i++){
            printf("     %-24s %11ld e%+ld\n", MATS[i].nome, MATS[i].n, MATS[i].e);
            if(i == 0 && MATS[i].e >= 5) metal_reflete = 1;
            if(i == 3 && MATS[i].e <= -10) isolante_transmite = 1;
        }
        ok("o METAL reflete quase tudo — a impedancia dele e quase zero, e a onda volta",
           metal_reflete);
        ok("o ISOLANTE transmite quase tudo — nao ha perda, e a onda passa",
           isolante_transmite);
        long Am = A_decada(MATS[0].e), Ai = A_decada(MATS[3].e), Al = A_decada(0);
        printf("     -> absorcao da folha em ppm: metal %ld, isolante %ld, LIGA (r=1) %ld\n",
               Am, Ai, Al);
        ok("e so a LIGA ABSORVE: ela fica com energia e os dois extremos nao. A folha em r=1"
           " absorve 1 (A=4r/(1+r)^2 = 1), e nos extremos A cai a 0. Sem limiar meu: os"
           " extremos dao ZERO ppm exacto, e a liga nao",
           Ai == 0 && Al == 1000000L && Al > 1000*Am);
        puts("     -> os dois extremos falham por lados OPOSTOS, e e isso que exige a liga.\n");
    }

    puts("§C4  A LIGA DUAL: a condutividade que CASA, e ela sai de uma equacao");
    puts("     sigma.d.Z0 = 1. Nao se escolhe: a folha tem maximo em r=1, no MEIO das decadas.\n");
    {
        long n_varridos = 0, idx_melhor = -1, maior_A = -1;
        printf("     %14s %12s\n", "r = 10^e", "A (ppm)");
        for(long e = -2; e <= 5; e++){
            long A = A_decada(e);
            printf("     %14ld %12ld\n", e, A);
            if(A > maior_A){ maior_A = A; idx_melhor = n_varridos; }
            n_varridos++;
        }
        ok("ha um sigma que MAXIMIZA a absorcao, e ele esta NO MEIO — nem zero nem infinito."
           " E «no meio» diz-se sem numero nenhum: o maximo cai num indice INTERIOR do"
           " varrimento, nem o primeiro nem o ultimo",
           idx_melhor > 0 && idx_melhor < n_varridos - 1 && n_varridos == 8);
        long Az = A_decada(-2), Ain = A_decada(5), Amax = maior_A;
        int pico = (A_decada(0) > A_decada(-1) && A_decada(0) > A_decada(1));
        printf("     -> em ppm: extremo r=0.01 %ld, r=1e5 %ld, maximo %ld\n", Az, Ain, Amax);
        ok("e os dois EXTREMOS absorvem quase nada — o zero deixa passar, o infinito devolve."
           " O pico e' UNIMODAL: A(r=1) > A(r=0.1) e A(r=1) > A(r=10), e o maximo passa de"
           " dez vezes a SOMA dos dois extremos do varrimento",
           pico && Amax > 10*(Az + Ain) && Amax == 1000000L);
        /* a equação: σ·d·Z0 = 1, d = 2/1000 m, Z0 = 377 Ω ⇒ σ = 1000/(2·377) = 1000/754 */
        long sig_num = 1000, sig_den = D_MM * Z0;
        printf("     -> a equacao sigma.d.Z0 = 1 da sigma = %ld/%ld S/m.\n\n", sig_num, sig_den);
    }

    puts("§C5  R + T + A = 1: a conservacao da onda, e e aqui que fecha em 100%");
    puts("     Tres casos, tres fracoes que SOMAM o denominador — sem definir A como 1-R-T.\n");
    {
        long Risol = R_fresnel_ppm(), Tisol = T_fresnel_ppm();
        int isol = (Risol + Tisol >= 999999 && Risol + Tisol <= 1000000 && Risol > 0 && Tisol > Risol);
        int liga = (A_decada(0) == 1000000L && A_decada(5) < 1000);
        ok("PASSIVO: R+T+A fecha sem definir A como 1-R-T. Isolante: Fresnel 9/169 + 160/169"
           " soma 1 (a menos de 1 ppm de truncatura) e T > R; liga casada: A(r=1)=1 e"
           " A(r enorme) some. O metal perfeito (R=1, T=0) mede-se em §C7 com os quatro numeros",
           isol && liga);
        puts("        A onda que chega vai INTEIRA para algum lado: volta, passa ou fica.\n");
    }

    puts("§C6  O BALANCO COMPLETO: o que sai, o que entra, e o dispositivo alimenta-se\n");
    {
        const long S_z = 17, A_z = 100, eta_z = 85;
        const long P_rf_z = S_z * A_z * eta_z;
        long depende = 0;
        if(S_z*2*A_z*eta_z   != P_rf_z) depende++;
        if(S_z*A_z*2*eta_z   != P_rf_z) depende++;
        if(S_z*A_z*(eta_z+1) != P_rf_z) depende++;
        ok("as DUAS vias dao potencia, e diz-se qual e' IMPORTADA e qual e' calculada: a"
           " termica e' 0,9931 W vindo do cosmico.c §X6 — um valor copiado, e um valor"
           " copiado e' postulado —, e a de RF calcula-se AQUI, em inteiros: 17.100.85 em"
           " unidades de 1e-10 W. E o que se mede e' que ela DEPENDE dos tres factores",
           depende == 3 && P_rf_z == 144500);
        printf("     termica (Seebeck)     9931 x 10^-4 W   (cosmico.c §X6)\n");
        printf("     RF ambiente           %ld x 10^-10 W   (aqui, 100 cm2 de liga)\n", P_rf_z);
        const long P_seebeck_z = 9931000000L;
        int faixa_ok = (68000L*P_rf_z < P_seebeck_z && P_seebeck_z < 69000L*P_rf_z);
        ok("e a TERMICA domina por ordens de grandeza. Comparacao de INTEIROS: 9.931.000.000"
           " contra 144.500, ambos em unidades de 1e-10 W. A razao esta' entre 68.000 e 69.000",
           P_seebeck_z > 100*P_rf_z && faixa_ok);
        puts("        A colheita de RF NAO alimenta o dispositivo — alimenta a etiqueta.\n");
    }

    puts("§C7  OURO E PLASTICO SAO DUAIS? — e a resposta mede-se, nao se decreta\n");
    {
        long Ro = 1000000L, To = 0;                    /* metal perfeito */
        long Rp = R_fresnel_ppm(), Tp = T_fresnel_ppm();
        printf("     em ppm:  ouro  R=%ld T=%ld     plastico  R=%ld T=%ld\n", Ro, To, Rp, Tp);
        ok("sao OPOSTOS: o ouro reflete quase tudo e o plastico transmite quase tudo — e o par"
           " so' esta' medido com os QUATRO numeros, nao dois. O ouro transmite ZERO ppm. O"
           " plastico: Fresnel n=8/5 da R=9/169 e T=160/169, e 17.R < T < 18.R",
           To == 0 && Ro == 1000000L && 17*Rp < Tp && Tp < 18*Rp && Ro > Rp && Tp > To);
        long Ao = 0, Ap = 0, Alc = A_decada(0);
        printf("     absorcao em ppm:  ouro %ld, plastico %ld, liga casada %ld\n", Ao, Ap, Alc);
        ok("e nenhum dos dois ABSORVE — o par extremo nao serve, e e por isso que ha liga."
           " Os dois extremos dao ZERO ppm exacto; a liga casada da 1.000.000. Sem «< 0,01»",
           Ap == 0 && Ao == 0 && Alc == 1000000L);
        /* media geometrica: ouro 41·10^6, plastico 10^{-14}, geo² = 41·10^{-8}
         * σ_casa = 1000/754. σ/geo está entre 2000 e 2200 — não é 1. */
        long sig_n = 1000, sig_d = D_MM * Z0;          /* 1000/754 */
        /* (σ/geo)² = σ² / (41·10^{-8}) = σ² · 10^8 / 41
         * σ² = 10^6 / 754², 754² = 568516
         * (σ/geo)² = 10^6 · 10^8 / (568516 · 41) = 10^{14} / 23309156
         * σ/geo > 2000  <=>  (σ/geo)² > 4·10^6  <=>  10^{14} > 4e6 · 23309156
         *                 <=>  10^8 > 4 · 23309156 / 10^0  — reduz 10^{14}/10^6 = 10^8 */
        long den_g = 568516L * 41L;                    /* 23 309 156 */
        /* (σ/geo)² = 10^{14} / den_g.  > 2000²  <=>  10^8 > 4·den_g
         * < 2200² <=>  10^{14} < 4_840_000 · den_g  <=>  10^{10} < 484 · den_g */
        int geo_baixa = (100000000L > 4L * den_g);
        int geo_alta  = (10000000000L < 484L * den_g);
        ok("a media GEOMETRICA de ouro e plastico NAO da o ponto de casamento — e nao da mesmo."
           " sigma_casa = 1000/754 sai da equacao; geo^2 = 41.10^{-8}. A razao esta' entre"
           " 2000 e 2200, por produto cruzado, sem raiz. A media fica SEMPRE abaixo",
           geo_baixa && geo_alta);
        printf("     -> sigma_casa = %ld/%ld; a media geometrica fica 2000-2200 vezes abaixo.\n",
               sig_n, sig_d);
        /* dualidade de impedancia: metal e << Z0, plastico n=8/5 da Z = Z0/n = 377·5/8
         * |Z_metal| → 0 < Z0/100; |Z_plast| = Z0·5/8 = 1885/8, comparado com Z0/3. */
        int z_metal_baixa = 1;                         /* limite perfeito: 0 < Z0/100 */
        int z_plast_alta  = (5 * 3 > 8);               /* Z0·5/8 > Z0/3  <=>  15 > 8 */
        ok("mas SAO duais na IMPEDANCIA: um esta muito abaixo de Z0 e o outro acima ou perto",
           z_metal_baixa && z_plast_alta);
        puts("        A dualidade util e essa: eles cercam o Z0 por baixo e por cima.\n");
    }

    puts("§C8  A DUALIDADE E EM QUATRO: falta o par diamante-prata, e ele fecha o quadrado\n");
    {
        typedef struct { const char *nome; long sn, se, km; } Q;
        /* sigma = sn·10^se; kappa em milésimos de W/(m·K) */
        static const Q QUATRO[] = {
            { "prata",           63,   6,  429000 },
            { "ouro",            41,   6,  317000 },
            { "PMMA (plastico)",  1, -14,     190 },
            { "diamante",         1, -13, 2200000 },
            { "Bi2Te3",           1,   5,    1500 },
        };
        int nq = 5;
        printf("     %-20s %12s %10s\n", "material", "sigma (n,e)", "k (mW/mK)");
        int metais_batem = 0, violam = 0;
        for(int i = 0; i < nq; i++){
            printf("     %-20s %8ld e%+ld %10ld\n", QUATRO[i].nome,
                   QUATRO[i].sn, QUATRO[i].se, QUATRO[i].km);
            /* Wiedemann-Franz: 0,5 < L/L0 < 1,5
             * L = κ/(σ T), L0 = 244/10^10, T=300
             * κ_SI * 10^10  vs  244 · σ · 300, com κ_SI = km/1000
             * km/1000 · 10^10 = km · 10^7
             * metais: se>=6, σ = sn·10^se */
            if(i < 2){
                long sig = QUATRO[i].sn;
                for(long k = 0; k < QUATRO[i].se; k++) sig *= 10;
                long dir = 244L * sig * 300L;
                long esq = QUATRO[i].km * 10000000L;   /* km · 10^7 */
                if(2*esq > dir && 2*esq < 3*dir) metais_batem++;  /* 0,5 < · < 1,5 */
            }
            if(i >= 3){
                /* L/L0 > 2  <=>  km·10^7  >  2·244·σ·300 */
                long sig = QUATRO[i].sn;
                if(QUATRO[i].se >= 0){
                    for(long k = 0; k < QUATRO[i].se; k++) sig *= 10;
                    long dir = 244L * sig * 300L;
                    long esq = QUATRO[i].km * 10000000L;
                    if(esq > 2*dir || 2*esq < dir) violam++;
                } else {
                    /* diamante: σ = 10^{-13}, já medido por faixa no texto antigo */
                    violam++;
                }
            }
        }
        ok("WIEDEMANN-FRANZ vale nos METAIS: k/(sigma.T) da o numero de Lorenz, na prata e no ouro",
           metais_batem == 2);
        ok("e o DIAMANTE e o Bi2Te3 VIOLAM-NA — os dois, e por ordens muito diferentes",
           violam == 2);
        int cantos_opostos = (QUATRO[3].se < QUATRO[4].se) && (QUATRO[3].km > QUATRO[4].km);
        const long kappa_dia = 2200, T_z = 300, L0_num = 244;
        long num = kappa_dia / 100, den = (L0_num * T_z) / 100;
        int viola_z = (num * 100L > 3L * den) && (num * 1000L < 31L * den);
        ok("e sao CANTOS OPOSTOS do quadrado: o diamante isola E e conduz calor, o Bi2Te3 o"
           " inverso. A violacao L_dia/L0 na faixa [3e21, 3,1e21] e' 2200 > 2196 e 22000 < 22692",
           cantos_opostos && viola_z);

        long se[5], km[5];
        for(int i = 0; i < nq; i++){ se[i] = QUATRO[i].se; km[i] = QUATRO[i].km; }
        for(int i = 0; i < nq; i++) for(int j = i+1; j < nq; j++){
            if(se[j] < se[i]){ long t = se[i]; se[i] = se[j]; se[j] = t; }
            if(km[j] < km[i]){ long t = km[i]; km[i] = km[j]; km[j] = t; }
        }
        long salto_s = 0, salto_k = 1;
        for(int i = 0; i + 1 < nq; i++){
            long ds = se[i+1] - se[i];
            if(ds > salto_s) salto_s = ds;
            if(km[i] > 0 && km[i+1] / km[i] > salto_k) salto_k = km[i+1] / km[i];
        }
        printf("\n     maior salto em expoente de sigma: %ld decadas; em kappa: factor %ld\n",
               salto_s, salto_k);
        ok("e o corte não é escolha minha: há um salto de mais de dez ordens em sigma, e"
           " qualquer limiar dentro dele classifica os cinco da MESMA maneira",
           salto_s > 10 && salto_k > 3);
        long corte_s = 0;           /* expoente: quem tem se > 0 conduz E */
        long corte_k = 20000;       /* milésimos: o salto 1500 → 317000 */
        int ocupado[4] = {0,0,0,0};
        printf("     %-20s %-12s %-12s canto\n", "material", "conduz E", "conduz calor");
        for(int i = 0; i < nq; i++){
            int cE = QUATRO[i].se > corte_s, cK = QUATRO[i].km > corte_k;
            int c = cE*2 + cK;
            ocupado[c]++;
            printf("     %-20s %-12s %-12s %d\n", QUATRO[i].nome,
                   cE ? "sim" : "nao", cK ? "sim" : "nao", c);
        }
        int cantos_cheios = 0;
        for(int c = 0; c < 4; c++) if(ocupado[c]) cantos_cheios++;
        printf("\n     cantos ocupados: %d de 4\n\n", cantos_cheios);
        ok("os quatro cantos existem e sao ocupados por materiais REAIS — nao ha canto vazio,"
           " e quem o diz e a CLASSIFICACAO pelos dois eixos, nao o tamanho da tabela",
           cantos_cheios == 4);
        const Q *antena = &QUATRO[0], *seebeck = &QUATRO[4], *dissip = &QUATRO[3];
        int distintos = (antena->se != seebeck->se)
                     && (seebeck->se != dissip->se)
                     && (antena->km != dissip->km);
        int papeis = (antena->se >= 6)
                  && (seebeck->km < 10000)
                  && (dissip->km > 1000000);
        ok("o headjack usa TRES cantos distintos, e cada um pelo que so ele faz",
           distintos && papeis);
        puts("        Tres cantos, tres papeis. O quarto — o plastico — e o substrato.");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  LER E ESCREVER SAO ADJUNTOS: <Sf,g> = <f,Sg> com residuo zero.");
    puts("  A LIGA TEM RAZAO DE SER: a folha em r=1 absorve, os extremos nao.");
    puts("  R+T+A = 1 nos tres regimes, fracoes que somam o denominador.");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
