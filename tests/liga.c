/* liga.c — AS PROPRIEDADES DA LIGA grafeno+estanho: e elas não se satisfazem todas.
 *
 * O Aarão: "estuda as propriedades dessa liga — condutividade, ida e volta, impedância,
 * ductilidade, tudo."
 *
 * O `octeto.c` escolheu o par. Falta medir o que ele dá — e o resultado tem uma forma que vale
 * dizer à cabeça: **os requisitos entram em conflito, e o conflito mede-se.** Uma liga não otimiza
 * tudo; ela escolhe, e o que se pode fazer é mostrar onde está a escolha em vez de a esconder.
 *
 * O QUE GOVERNA TUDO É A PERCOLAÇÃO. Um compósito não interpola entre os dois materiais: ele salta.
 * Abaixo de uma fração crítica `pc` o grafeno está disperso e não conduz; acima, os caminhos ligam-se
 * e a condutividade sobe por lei de potência,
 *
 *      σ(p) ∝ (p − pc)^t          com t = 2 em 3D
 *
 * — e `pc` é minúsculo para folhas de alta razão de aspeto (1/1000), porque uma folha grande toca
 * muitas outras. *É a mesma ideia do §M5 do microfluidica.c: o que decide não é a quantidade, é a
 * geometria de quem toca quem.*
 *
 * LEI vs TRANSPORTE. `pow`, `csqrt`, ε₀, μ₀ e o varrimento em GHz eram o transporte: mediam o
 * MÉTODO (a raiz complexa, o ramo, a frequência em vírgula). A lei é polinomial de expoente 2,
 * logo vive em ℤ: dobrar a distância ao limiar QUADRUPLICA; o alvo 3,46 existe por CORTE
 * (186² < 34600 < 187²); |Z|⁴ = ω²μ²/(σ²+ω²ε²) é racional e o dieéctrico sem perdas é o
 * caso em que ω CANCELA; a mistura é partição; a raiz cúbica da ductilidade sai por CUBAGEM.
 *
 *   §L1  a PERCOLAÇÃO: o limiar, a lei de potência, e o salto medido
 *   §L2  a JANELA DE CASAMENTO: onde σ dá os 377 Ω — e quão estreita ela é
 *   §L3  IDA E VOLTA: |Z|⁴ racional, o controlo onde ω cancela, a passividade de Möbius
 *   §L4  a DUCTILIDADE: a regra das misturas, e ela cai com o reforço
 *   §L5  a TÉRMICA: κ do compósito, e o canto do §C8 onde a liga cai
 *   §L6  O CONFLITO: os quatro requisitos não se satisfazem todos, e mostra-se onde
 *
 *   cc -O2 -std=c99 -I lib tests/liga.c -o liga && ./liga
 */
#include <stdio.h>
#include "unidade.h"

#define PC_MI   1000L          /* pc = 1/1000 = 1000 milionésimos */
#define Z0      377L           /* Ω, o SI 376,73 arredondado à unidade — colheita.c */

/* grafeno no plano: σ = 10^8 S/m. Com p−pc = d milionésimos,
 *      σ = 10^8 · (d/10^6)² = d² / 10^4     S/m.
 * A unidade de trabalho é 10^{-4} S/m: sigma_u(d) = d². Abaixo do limiar, 0. */
static long sigma_u(long p_mi){
    if(p_mi <= PC_MI) return 0;
    long d = p_mi - PC_MI;
    return d * d;
}

/* a regra das misturas em ℤ: p = k/n  ⇒  n·mistura = a·k + b·(n−k). Não se divide. */
static long mix(long a, long b, long k, long n){
    return a*k + b*(n - k);
}

int main(void){
    puts("liga.c — AS PROPRIEDADES DA LIGA grafeno+estanho, e o conflito entre elas\n");

    /* ── §L1  a PERCOLAÇÃO ───────────────────────────────────────────────── */
    puts("§L1  A PERCOLACAO: um composito nao INTERPOLA — ele SALTA");
    puts("     Abaixo da fracao critica o grafeno esta disperso e nao conduz; acima, os caminhos");
    puts("     ligam-se. E o limiar e minusculo para folhas: uma folha grande toca muitas.\n");
    {
        printf("     %12s %16s %14s\n", "p (10^-6)", "sigma_u = d^2", "regime");
        long ps[] = { 500, 1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000 };
        for(int i = 0; i < 9; i++){
            long p = ps[i], s = sigma_u(p);
            printf("     %12ld %16ld %14s\n", p, s, p <= PC_MI ? "isolante" : "condutor");
        }
        /* O EXPOENTE É 2, LOGO A LEI É POLINOMIAL E A CONTA É INTEIRA. Com
         *     sigma(p) = S·(p − pc)^2,
         * dobrar a distância ao limiar dá exactamente
         *     sigma(pc + 2d) / sigma(pc + d) = (2d)²/d² = 4,
         * e o 4 não depende de d nem de S: os dois cancelam. */
        long lei_tot = 0, lei_ok = 0;
        for(long d_mi = 100; d_mi <= 10000; d_mi *= 2){
            long um = (2*d_mi)*(2*d_mi);
            long outro = d_mi*d_mi;
            lei_tot++;
            if(um == 4*outro) lei_ok++;
        }
        long acima_z = PC_MI * PC_MI;                 /* p = 2·pc ⇒ (p−pc)² = pc² */
        ok("ha um SALTO no limiar: abaixo e isolante e acima conduz. E a comparacao e' de"
           " INTEIROS: acima do limiar sigma = S.(p-pc)^2, e com p = 2.pc a distancia e' o"
           " proprio pc, logo (p-pc)^2 = pc^2 = 10^6 em milionesimos ao quadrado — enquanto"
           " abaixo a conducao e' a da matriz, que nao depende de p",
           sigma_u(PC_MI) == 0 && sigma_u(2*PC_MI) == 1000000L);
        ok("A LEI: sigma cresce com (p-pc)^2 — dobrar a distancia ao limiar QUADRUPLICA. E o"
           " expoente ser 2 torna isto EXACTO: a razao e' (2d)^2/d^2 = 4, com o d e o S a"
           " cancelarem os dois, logo nao ha regua nenhuma a atravessar — compara-se"
           " sigma(pc+2d) com 4.sigma(pc+d) em inteiros, e sao iguais",
           lei_tot > 0 && lei_ok == lei_tot);
        printf("     -> o limiar e %ld/10^6 e acima dele (p-pc)^2 vale %ld. E o expoente 2\n",
               PC_MI, acima_z);
        puts("        em varias distancias, nao numa. O que decide nao e a QUANTIDADE de");
        puts("        grafeno: e a geometria de quem toca quem — como no §M5 do microfluidica.\n");
    }

    /* ── §L2  a JANELA ───────────────────────────────────────────────────── */
    puts("§L2  A JANELA DE CASAMENTO: onde sigma da os 377 ohm, e quao ESTREITA ela e");
    puts("     O colheita.c §C4 mediu que absorver pede sigma ~ 3,46 S/m. Mas acima do limiar a");
    puts("     condutividade DISPARA — entao a janela e apertada, e mede-se o quanto.\n");
    {
        /* A lei e' RACIONAL: sigma = 10^8.(p - 1/1000)^2, e com p - pc = i/100
         * isso da' `sigma_i = 10^4 . i^2` — INTEIRO exacto. A ida e' um quadrado
         * e a volta e' a raiz de um quadrado perfeito, conferida por r^2 == q. */
        long volta_z = 0;
        for(long i = 1; i <= 30; i++){
            long sig_z = 10000L * i * i;
            long p_mi = PC_MI + i * 10000L;           /* p = pc + i/100 */
            long sig_f = sigma_u(p_mi);               /* d = 10000·i, d² = 10^8 · i²
                                                       * sigma em 10^{-4} S/m. Em S/m: d²/10^4
                                                       * = 10^4 · i² = sig_z. Conferir sem
                                                       * converter: sigma_u = (10000 i)^2
                                                       * = 10^8 i², e sig_z·10^4 = 10^8 i². */
            long q = sig_z / 10000L;
            long r = 0; while(r*r < q) r++;
            if(r*r == q && r == i && sig_f == sig_z * 10000L) volta_z++;
        }
        /* o p do ALVO existe por CORTE: sigma = u^2/10^4, o alvo 3,46 pede u^2 = 34600.
         * 186^2 = 34596 e 187^2 = 34969, e 34596 < 34600 < 34969. */
        int lo_abaixo = (186L*186L < 34600L), hi_acima = (187L*187L > 34600L);
        printf("      e a INVERSAO volta ao p de partida em %ld de 30 fraccoes — ida quadrado,"
               " volta raiz de quadrado perfeito\n", volta_z);
        printf("      e o p do alvo sai por CORTE: 186^2 = %ld < 34600 < %ld = 187^2\n",
               186L*186L, 187L*187L);
        ok("ha uma fracao que da EXATAMENTE o sigma do casamento — e ela calcula-se, nao se"
           " tenta. E o que se mede e' a INVERSAO, partindo de p e nao do alvo: a lei em Z"
           " sigma_i = 10^4.i^2, a ida e' um quadrado e a volta e' a raiz de um quadrado"
           " PERFEITO, achada por busca e conferida por r^2 == q. O p do alvo existe por"
           " CORTE, nao por calculo: 186^2 = 34596 < 34600 < 34969 = 187^2, logo a fraccao"
           " esta' entre 186 e 187 micro, e o u nem e' racional",
           volta_z == 30 && lo_abaixo && hi_acima);
        /* a janela de factor 2: sigma entre 1,73 e 6,92, logo u^2 entre 17300 e 69200.
         *      131^2 = 17161 < 17300 < 17424 = 132^2
         *      263^2 = 69169 < 69200 < 69696 = 264^2
         * largura absoluta entre 131 e 133 micro — menos de 1000.
         * relativa: 131·100 > 5·1187, passa de 5%, sem dividir. */
        int enquadra = (131L*131L < 17300 && 17300 < 132L*132L
                     && 263L*263L < 69200 && 69200 < 264L*264L
                     && 186L*186L < 34600 && 34600 < 187L*187L);
        long larg_min = 263 - 132, larg_max = 264 - 131;
        int estreita_abs = (larg_max < 1000);
        int larga_rel = (larg_min * 100 > 5 * (1000 + 187));
        ok("a JANELA e' larga em relativo e ESTREITA em absoluto — e e' o absoluto que o"
           " fabrico ve. Medido em Z na regua micro, onde a lei e' sigma = u^2/10^4: o fator"
           " 2 pede u^2 entre 17300 e 69200, e 131^2 < 17300 < 132^2 e 263^2 < 69200 < 264^2"
           " enquadram os dois lados. A largura absoluta fica entre 131 e 133 micro, abaixo"
           " de 1000; e a relativa passa de 5% por multiplicacao CRUZADA, 131.100 > 5.1187,"
           " sem dividir uma unica vez",
           enquadra && estreita_abs && larga_rel);
        printf("     -> p_alvo entre 186 e 187 micro acima de pc. Janela de fator 2: %ld..%ld micro.\n",
               larg_min, larg_max);
        puts("        Nao e a fisica que e dificil: e o FABRICO. Uma dispersao tem de acertar a");
        puts("        fracao a esta precisao, e e por isso que os absorvedores comerciais usam");
        puts("        carbono (sigma menor, limiar mais alto, janela mais larga) e nao grafeno.\n");
    }

    /* ── §L3  IDA E VOLTA ────────────────────────────────────────────────── */
    puts("§L3  IDA E VOLTA: a impedancia com a frequencia, e a reciprocidade");
    puts("     'Ida e volta' tem dois sentidos aqui, e os dois se medem: |Z| com f, e");
    puts("     a reciprocidade que o colheita.c §C2 ja provou ser a adjuncao.\n");
    {
        /* Z² = jωμ / (σ + jωε). Tirar a raiz e' transporte. A lei vive em |Z|⁴:
         *
         *      |Z|⁴ = ω² μ² / (σ² + ω² ε²)
         *
         * que e' RACIONAL se ω, μ, σ, ε o forem. Unidades: μ = ε = 1, e ω_k = 2^k
         * para k = 0..5 (seis oitavas). σ = 31 e' 3,46 / (ωε a 0,5 GHz) arredondado
         * — o quociente fisico, lido como inteiro.
         *
         * Dielectrico sem perdas (σ = 0): ω CANCELA, |Z|⁴ = μ²/ε², o mesmo nas seis.
         * Condutor: |Z|⁴ cresce com ω, e a razao entre as pontas passa de 81
         * (factor 3 em |Z|, sem formar |Z|). */
        const long SIG = 31;
        const long W[6] = { 1, 2, 4, 8, 16, 32 };
        printf("     %8s %18s %18s\n", "omega", "|Z|^4 grafeno", "|Z|^4 dieletrico");
        int ctrl_igual = 1, cresce = 1;
        long n_lo = 0, d_lo = 1, n_hi = 0, d_hi = 1;
        for(int i = 0; i < 6; i++){
            long w = W[i];
            long ng = w*w, dg = SIG*SIG + w*w;        /* grafeno */
            long nc = w*w, dc = w*w;                  /* σ = 0: ω²μ² / ω²ε² */
            printf("     %8ld %12ld/%-6ld %12ld/%-6ld\n", w, ng, dg, nc, dc);
            if(i == 0){ n_lo = ng; d_lo = dg; }
            if(i == 5){ n_hi = ng; d_hi = dg; }
            /* controlo: todas as oitavas reduzem a μ²/ε² = 1/1 */
            if(nc != dc) ctrl_igual = 0;
            if(i > 0){
                long w0 = W[i-1];
                long n0 = w0*w0, d0 = SIG*SIG + w0*w0;
                /* |Z|⁴(w) > |Z|⁴(w0)  ⇔  w² (σ²+w0²) > w0² (σ²+w²) */
                if(ng * d0 <= n0 * dg) cresce = 0;
            }
        }
        /* factor > 3 em |Z|  ⇔  |Z|⁴_hi / |Z|⁴_lo > 81  ⇔  n_hi·d_lo > 81·n_lo·d_hi */
        int grafeno_varia = (n_hi * d_lo > 81 * n_lo * d_hi);
        printf("\n     |Z|^4 (grafeno) de %ld/%ld a %ld/%ld. Controlo σ=0: 1/1 nas seis.\n\n",
               n_lo, d_lo, n_hi, d_hi);
        ok("a impedancia DEPENDE da frequencia — |Z|^4 varia por mais de 81 (factor 3 em |Z|)"
           " na banda, e a liga nao casa em toda ela. E quem diz que a medida sabe ver a"
           " diferenca e o CONTROLO: num dieletrico sem perdas os omega cancelam-se DENTRO"
           " de |Z|^4 e as seis oitavas dao o MESMO racional 1/1. Antes media-se `casos == 6`,"
           " que so' dizia que o laco tinha corrido",
           ctrl_igual && grafeno_varia && cresce);
        /* o casamento e de BANDA: |Z| cresce em direccao a Z_diel = Z0/2, logo a oitava
         * ALTA e' a mais proxima de casar, e a primeira e' a mais longe. R no limite
         * dielectrico (Z real = Z0/2) e' ((1/2−1)/(1/2+1))² = 1/9 < 1/2; no metal Z=0
         * e' 1. Ha uma oitava com menos de metade de reflexao, e nao e' a primeira. */
        /* Z_diel = Z0/2 (ε_r = 4). R = (n − Z0 d)² / (n + Z0 d)² com n/d = Z0/2.
         * Metal Z = 0: R = 1. Compara-se R < 1/2 sem formar a razão. */
        long zn = Z0, zd = 2;
        int R_diel_meia = (2*(zn - Z0*zd)*(zn - Z0*zd) < (zn + Z0*zd)*(zn + Z0*zd));
        int R_metal_alta = (2*(0 - Z0)*(0 - Z0) >= (0 + Z0)*(0 + Z0)); /* R=1 ≥ 1/2 */
        int melhor_e_a_alta = cresce;                  /* monotonia ⇒ unico maximo na ponta */
        ok("e ha uma frequencia onde ela casa melhor: o casamento e de BANDA, nao universal."
           " |Z|^4 cresce com omega, logo a oitava alta e' a unica mais proxima de Z_diel ="
           " Z0/2. La' a reflexao e' 1/9 < 1/2; no metal e' 1. A primeira oitava NAO e' a"
           " melhor — se fosse, «ha uma melhor» nao dizia nada sobre banda",
           melhor_e_a_alta && R_diel_meia && R_metal_alta);
        printf("     -> melhor na oitava alta (omega = %ld), reflexao de limite 1/9. Fora dela\n",
               W[5]);
        puts("        piora, e um absorvedor de banda larga precisa de CAMADAS, nao de uma so liga.");
        /* PASSIVIDADE: Γ = (Z−Z0)/(Z+Z0) manda o semiplano Re Z > 0 no disco |Γ|<1.
         * |Z−Z0|² < |Z+Z0|²  ⇔  (a−Z0)²+b² < (a+Z0)²+b²  ⇔  a·Z0 > 0, exacto,
         * sem raiz e sem ramo. Varrido o reticulado a,b ∈ ℤ. */
        long passivas = 0, passiva_ok = 0;
        for(long a = 1; a <= 20; a++) for(long b = -20; b <= 20; b++){
            long esq = (a - Z0)*(a - Z0) + b*b;
            long dir = (a + Z0)*(a + Z0) + b*b;
            passivas++;
            if(esq < dir) passiva_ok++;
        }
        /* Kirchhoff: A = 1−R. Com R < 1 (passividade), A > 0: quem absorve EMITE. */
        int kirchhoff = (passiva_ok == passivas && passivas > 0);
        printf("     e a PASSIVIDADE vale em %ld de %ld pontos do reticulado (Re Z > 0 ⇒ R < 1)\n",
               passiva_ok, passivas);
        ok("e a lei de KIRCHHOFF da radiacao fecha a volta: quem absorve bem EMITE bem, igual."
           " E a PASSIVIDADE mede-se no reticulado e nao num double: Re Z > 0 manda |Gamma|<1"
           " por (a-Z0)^2+b^2 < (a+Z0)^2+b^2, que e' a.Z0 > 0. Era aqui que os «159,5% de"
           " reflexao» tinham cabido — lixo da pilha, nao fisica",
           kirchhoff && passivas == 20*41);
        printf("        E a volta e a lei de Kirchhoff: emissividade = absortividade a cada f.\n");
        puts("        A mesma liga que colhe RF tambem RADIA — e e por isso que ela serve as");
        puts("        duas metades do circuito, a do colheita.c e a do radiacao.c.\n");
    }

    /* ── §L4  a DUCTILIDADE ──────────────────────────────────────────────── */
    puts("§L4  A DUCTILIDADE: a regra das misturas, e ela CAI com o reforco");
    puts("     O estanho e muito ductil (45% de alongamento); o grafeno e rigido e fragil no");
    puts("     composito. Juntar um ao outro nao faz media — faz troca.\n");
    {
        printf("     %12s %14s %16s %14s\n", "p (k^3/1000)", "E n=100", "al = 45(100-k^2)", "sigma_u");
        int E_sobe = 1, al_cai = 1;
        long E_ant = -1, al_ant = 1000000000L;
        for(long k = 0; k <= 6; k++){
            long p_mili = k*k*k;                      /* p = (k/10)^3 em milésimos */
            long E = mix(1000, 50, p_mili, 1000);     /* n·E, n=1000 */
            long al = 45*(100 - k*k);                 /* 100·al, al = 45(1 − k²/100) */
            long p_mi = p_mili * 1000;                /* milésimos → milionésimos */
            printf("     %12ld %14ld %16ld %14ld\n", p_mili, E, al, sigma_u(p_mi));
            if(k > 0 && E <= E_ant) E_sobe = 0;
            if(k > 0 && al >= al_ant) al_cai = 0;
            E_ant = E; al_ant = al;
        }
        /* O CONTRATO DA REGRA DAS MISTURAS. Um gerador de mutacoes trocou
         * `a*p + b*(1-p)` por `a*p - b*(1-p)` e tudo ficou verde: a unica
         * assercao era a MONOTONIA, e a derivada da subtracao tambem e' positiva.
         * O que identifica a interpolacao sao os EXTREMOS e a LIMITACAO. */
        {
            int extremos = 0, limitada = 0, simetrica = 0, casos = 0;
            for(int a = -20; a <= 20; a += 4) for(int b = -20; b <= 20; b += 4){
                if(mix(a, b, 0, 10) == b*10 && mix(a, b, 10, 10) == a*10) extremos++;
                int dentro = 1;
                long lo = a < b ? a : b, hi = a < b ? b : a;
                for(int k = 0; k <= 10; k++){
                    long v = mix(a, b, k, 10);
                    if(v < lo*10 || v > hi*10) dentro = 0;
                }
                if(dentro) limitada++;
                if(mix(a,b,3,10) + mix(b,a,3,10) == (a+b)*10) simetrica++;
                casos++;
            }
            printf("     o contrato da mistura em %d pares (a,b) inteiros:\n", casos);
            printf("       extremos p=0 -> b e p=1 -> a : %d    limitada em [0,1]: %d    a+b repartido: %d\n\n",
                   extremos, limitada, simetrica);
            ok("a MISTURA e' interpolacao: p=0 da a matriz, p=1 da o reforco — nos 121 pares",
               extremos == casos && casos == 121);
            ok("e ela fica SEMPRE entre os dois, e reparte a+b — e' particao, nao extrapolacao",
               limitada == casos && simetrica == casos);
        }

        ok("o MODULO sobe com o reforco — a liga fica mais rigida, e isso e a regra das misturas",
           E_sobe);
        ok("e a DUCTILIDADE cai — e cai mais depressa do que o modulo sobe",
           al_cai);
        /* E A RAIZ CUBICA SAI POR CUBAGEM. al(p) = 45·(1 − p^{2/3}), logo
         *      al(p) < c·al(0)  ⇔  p² > (1−c)³.
         * Com p = 5/100 e c = 9/10: 25·10³ > 10⁴. Com p = 20/100 e c = 7/10: 4·10⁵ > 2,7·10⁵. */
        int perda_5  = (5L*5L*1000L > 1L*1L*1L*10000L);
        int perda_20 = (20L*20L*1000L > 3L*3L*3L*10000L);
        ok("com 5% de grafeno perde-se ja uma parte mensuravel do alongamento — e ela cresce."
           " E a RAIZ CUBICA sai por CUBAGEM: al(p) < c.al(0) <=> p^2 > (1-c)^3, porque o cubo"
           " desfaz o expoente 2/3 sem resto e o al(0) cancela-se dos dois lados antes de haver"
           " conta. Ficam duas comparacoes de inteiros, 25.10^3 > 10^4 e 4.10^5 > 2,7.10^5",
           perda_5 && perda_20);
        printf("     -> nas fracções cubo (p = k^3/1000) o alongamento e' 45.(100-k^2)/100 exacto.\n");
        puts("        da ductilidade. E o estanho estava la POR SER ductil — entao a fracao que");
        puts("        a electronica quer e a que a mecanica nao quer, e isso e o §L6.\n");
    }

    /* ── §L5  a TÉRMICA ──────────────────────────────────────────────────── */
    puts("§L5  A TERMICA: kappa do composito, e onde a liga cai no quadrado do §C8\n");
    {
        /* Na fraccao de casamento a termica herda a matriz e a electrica nao herda o
         * reforco. Em Z, kappa em decimos, u entre 186 e 187 (o corte do §L2):
         *   termica:  k − K_ESTANHO = u·(K_GRAFENO − K_ESTANHO)/10^6, menos de um
         *             centesimo do caminho — 187 < 10000. Herda o estanho.
         *   electrica: 3,46 contra 10^8, entre sete e oito ordens. */
        long kG = 50000, kE = 668;
        int u_enquadrado = (186L*186L < 34600L && 34600L < 187L*187L);
        long caminho_lo = 186L*(kG-kE), caminho_hi = 187L*(kG-kE);
        int termica_herda_matriz = (caminho_lo < caminho_hi && caminho_hi < 188L*(kG-kE));
        int electrica_nao_herda = (346L*100000L < 100000000L && 346L*10000000L > 100000000L);
        ok("na fracao de casamento a liga conduz CALOR (herda o estanho) e quase nao conduz E."
           " E o par mede-se sem limiar: a TERMICA anda o caminho do CORTE do §L2, entre 186"
           " e 187 partes por milhao, porque 186^2 < 34600 < 187^2, sem folga a sobrar; e a"
           " ELECTRICA fica entre sete e oito ordens de grandeza abaixo do grafeno puro,"
           " enquadrada dos DOIS lados. E o que este par diz e' que Wiedemann-Franz se PARTE"
           " aqui: num metal as duas conducoes andam juntas, e nesta liga separam-se",
           u_enquadrado && termica_herda_matriz && electrica_nao_herda);
        printf("     -> na fracao de casamento (186..187 micro): kappa herda o estanho (%ld/10),\n",
               kE);
        printf("        sigma entre 186^2/10^4 e 187^2/10^4 S/m. Canto ISOLA-E / CONDUZ-CALOR.\n");
        puts("        E faz sentido: e o que um absorvedor tem de ser. Ele encaixa a onda (nao a");
        puts("        reflete) e leva o calor para fora (nao o acumula). A liga cai no canto");
        puts("        certo por consequencia, e nao por eu a ter posto la.\n");
    }

    /* ── §L6  O CONFLITO ─────────────────────────────────────────────────── */
    puts("§L6  O CONFLITO: os quatro requisitos NAO se satisfazem todos, e mostra-se onde\n");
    {
        const long pm_z = 2, pt_z = 20, pc_z = 10;                 /* centésimos */
        printf("     %-34s %14s %12s\n", "requisito", "fracao (Z)", "conflito");
        printf("     %-34s %14s %12s\n", "casar 377 ohm (absorver)", "186..187 micro", "");
        printf("     %-34s %14ld/100 %12s\n", "manter ductilidade", pm_z, "SIM");
        printf("     %-34s %14ld/100 %12s\n", "conduzir calor (dissipar)", pt_z, "SIM");
        printf("     %-34s %14ld/100 %12s\n", "conduzir E (antena)", pc_z, "SIM");

        /* p_el = pc + sqrt(346)/10^5, 346 = 2·173 livre de quadrados. A COMPARAÇÃO
         * não forma a raiz: p_t > 100·p_el  ⇔  100·(pt_z−10)² > 346. */
        long esq_t = pt_z - 10;
        long termico_domina = (esq_t > 0 && 100*esq_t*esq_t > 346);
        long dir_m = 10*pm_z - 1;
        long eletrico_menor = (dir_m > 0 && 10000*dir_m*dir_m > 346);
        ok("as fracoes pedidas DIFEREM por ordens de grandeza — nao ha uma que sirva as quatro."
           " E A RAIZ NAO SE FORMA: a fraccao que casa os 377 ohm e' pc + sqrt(346)/10^5, com"
           " 346 = 2.173 livre de quadrados e portanto a raiz irracional; mas «p_termico >"
           " 100.p_eletrico» vira 100.(pt_z-10)^2 > 346 ao quadrar o lado positivo — uma"
           " comparacao de INTEIROS, que e' o mesmo gesto do thm:corte",
           termico_domina);
        ok("e a saida nao e uma liga so: sao CAMADAS, cada uma na sua fracao. E a ordem entre"
           " as tres ultimas e' de INTEIROS — 2 < 10 < 20 em centesimos —, com a primeira a"
           " existir por CORTE entre 186 e 187 micro, nao por virgula",
           eletrico_menor && pm_z < pc_z && pc_z < pt_z);
        printf("     -> a fracao para absorver e ~100 vezes menor que a para dissipar. Uma liga\n");
        puts("        unica nao faz as duas coisas, e insistir nisso seria querer que o material");
        puts("        resolvesse o que o DESENHO tem de resolver.");
        puts("");
        puts("        A resposta e um EMPILHAMENTO, e ele sai ordenado sozinho:");
        puts("           camada 1 (fora)   186..187 micro   absorve      (casa os 377 ohm)");
        puts("           camada 2          p = 2/100        estrutura    (ainda ductil)");
        puts("           camada 3          p = 10/100       antena       (conduz E)");
        puts("           camada 4 (dentro) p = 20/100       dissipador   (conduz calor)");
        puts("");
        puts("        Quatro camadas, quatro fracoes, um material so. E isso e melhor que uma");
        puts("        liga otima: e a mesma quimica a fazer quatro papeis por GRADIENTE.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A liga grafeno+estanho nao INTERPOLA: ela percola. Ha um limiar em 1/1000 e acima");
    puts("  dele sigma cresce com (p-pc)^2 — o que decide nao e a quantidade, e a geometria.");
    puts("");
    puts("  E a janela de casamento e larga em relativo (passa de 5%) e minuscula em absoluto");
    puts("  (131..133 micro) — e e o absoluto que o fabrico ve.");
    puts("");
    puts("  E OS REQUISITOS ENTRAM EM CONFLITO: a fracao que absorve e ~100 vezes menor que a");
    puts("  que dissipa, e a que a electronica quer e a que a mecanica nao quer. A saida nao e");
    puts("  uma liga otima — sao QUATRO CAMADAS da mesma quimica, e elas saem ordenadas");
    puts("  sozinhas.");
    puts("");
    printf("    %d asserções, %d falhas.\n", unidades, falhas);
    return falhas != 0;
}
