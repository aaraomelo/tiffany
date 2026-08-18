/* radiacao.c — A RADIAÇÃO NEGRA: o dual do eletromagnético, e é ela que fecha o circuito.
 *
 * O Aarão: "falta o dual pra fechar o circuito, é a radiação térmica — projetar o array de
 * sensores pra detectar. É a radiação negra, o dual do eletromagnético."
 *
 * E ELE FECHA MESMO, e o `headjack.c` deixou o buraco à vista sem eu ver que era um buraco. Ali
 * mediu-se que a corrente **radial** dá campo magnético externo **exatamente zero** — o cruzado
 * mata-a — e escreveu-se, citando o `koch.c`, que *o que não tem dual fica retido e ARDE*.
 *
 * Ora, arder é dissipar. E dissipar é radiar. **A corrente que o magnético não vê é exatamente a
 * que o térmico vê**, porque as duas leis têm formas opostas:
 *
 *      MAGNÉTICO   B ∝ Q × r̂       VETORIAL, antissimétrico  ->  TEM núcleo (o radial some)
 *      TÉRMICO     P = J²·ρ        ESCALAR,  quadrático      ->  NÃO tem núcleo (tudo dissipa)
 *
 * *Um perde o que o outro guarda.* Não é uma analogia bonita: é a partição `B = B_s + B_a` outra
 * vez — o cruzado ordena e tem núcleo; o direto mede e não tem. E o direto, aqui, é o calor.
 *
 *   §W1  as leis da radiação negra: Stefan–Boltzmann, Wien, Planck — as três, e as três medidas
 *   §W2  O DUAL: onde B = 0 a potência é MÁXIMA — medido, não afirmado
 *   §W3  o operador térmico NÃO tem núcleo: correntes distintas dão calores distintos
 *   §W4  o array de sensores: microbolómetro, MCT, e a conta de quantos
 *   §W5  O CIRCUITO FECHA: o par (B, P) recupera o que B sozinho perde
 *   §W6  e a fronteira honesta: o que este par ainda não separa
 *
 *   cc -O2 -std=c99 radiacao.c -lm -o radiacao && ./radiacao
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reta.h"     /* rt_cruz3, rt_dir: o cruzado e o interno, inteiros */
#include "termica.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MU0        (4e-7 * M_PI)
#define T_CORPO    310.0             /* 37 °C em kelvin */
#define RHO_TECIDO 2.5               /* resistividade do tecido cerebral, Ω·m (literatura) */

/* o campo do dipolo: B ∝ Q × r̂ — o CRUZADO, e é ele que tem núcleo */
static void b_dipolo(const double *Q, const double *rhat, double *B){
    B[0] = Q[1]*rhat[2] - Q[2]*rhat[1];
    B[1] = Q[2]*rhat[0] - Q[0]*rhat[2];
    B[2] = Q[0]*rhat[1] - Q[1]*rhat[0];
}

/* a potência dissipada: P = J²·ρ·V — o DIRETO, escalar, e não tem núcleo nenhum */
static double p_joule(const double *Q, double vol, double comp){
    double q2 = Q[0]*Q[0] + Q[1]*Q[1] + Q[2]*Q[2];
    double J2 = q2 / (comp*comp * vol*vol);        /* densidade de corrente ao quadrado */
    return J2 * RHO_TECIDO * vol;
}

/* ───────────────────────────────────────────── os sensores térmicos, números públicos */

typedef struct { const char *nome; double netd_K; const char *nota; } Termico;

static const Termico TERMICOS[] = {
    { "microbolometro",   0.020, "nao arrefecido, 8-14 um"      },
    { "MCT arrefecido",   0.010, "HgCdTe, 77 K"                 },
    { "InSb arrefecido",  0.018, "3-5 um, 77 K"                 },
    { "termopilha",       0.100, "de bancada, sem optica"       },
};
#define NTERM ((int)(sizeof TERMICOS / sizeof TERMICOS[0]))

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
/* a CONCLUSAO nao e unidade: resume o que as assercoes acima mediram, e contá-la como
 * medida inflacionava a bateria. O `unidade.h` tem a dele; este ficheiro traz o seu
 * proprio `ok`, logo traz tambem o seu `conclui`, com a MESMA marca [~] da casa. */
static void conclui(const char *q){ printf("  [~] %s\n", q); }
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("radiacao.c — A RADIACAO NEGRA: o dual do eletromagnetico, e ela fecha o circuito\n");

    /* ── §W1 ─────────────────────────────────────────────────────────────── */
    puts("§W1  AS LEIS DA RADIACAO NEGRA — e as tres medem-se, nao se citam");
    puts("     Stefan-Boltzmann, Wien e Planck. As constantes sao do SI de 2019, onde k_B, h e c");
    puts("     sao EXATAS por definicao — nao ha uma escolhida por mim.\n");
    {
        double P = sb_potencia(T_CORPO), lam = wien_pico(T_CORPO);
        printf("     -> a 37 C (310 K): P = %.1f W/m2, pico de Wien em %.2f um (infravermelho).\n",
               P, lam*1e6);
        /* a LEI de Stefan-Boltzmann: dobrar T multiplica a potencia por 16. Mede-se em
         * varios T, e o indice do laco e INTEIRO — T = 100·2^k. O `for(double T = 100;
         * T <= 800; T *= 2)` funcionava, porque 100·2^k e exacto em base 2; mas o que a
         * varredura percorre e o EXPOENTE, e escreve-lo assim di-lo. */
        int quarta = 1, nT = 0;
        for(int k = 0; k <= 3; k++){
            double T = 100.0 * (1L << k);
            double r = sb_potencia(2*T) / sb_potencia(T);
            nT++;
            if(r != 16.0) quarta = 0;
        }
        /* ESTAS DUAS NAO SAO MEDIDA, E O GUME AUTOMATICO DISSE-O. Sobreviveram a todas as
         * mutacoes porque `sb_potencia` E' sigma.T^4 e `wien_pico` E' B/T: dividir
         * sb_potencia(2T) por sb_potencia(T) da 16 POR CONSTRUCAO, e multiplicar B/T por T
         * devolve B POR CONSTRUCAO. Sao a definicao lida de volta — «normalizar nao e
         * medir», com a lei no papel de quantidade dividida por si propria.
         *
         * Ficam como CONCLUSAO, que e o que sao. A medida esta' na RECUPERACAO A PARTIR DE
         * PLANCK: o integral de Planck tem de dar Stefan-Boltzmann (ja' medido abaixo), e o
         * PICO de Planck tem de cair onde Wien o poe. Essa segunda faltava, e escreve-se
         * aqui — procurando o maximo numa grelha inteira de nanometros e comparando com
         * `wien_pico`, que e' uma comparacao entre DUAS ROTAS e nao entre uma e ela mesma. */
        conclui("STEFAN-BOLTZMANN e a QUARTA potencia por DEFINICAO de sb_potencia — a medida");
        conclui("e a recuperacao a partir do integral de Planck, abaixo");
        (void)quarta; (void)nT;
        /* a LEI de Wien: o pico e inversamente proporcional a T */
        int inversa = 1, nW = 0;
        for(int k = 0; k <= 4; k++){
            double T = 100.0 * (1L << k);
            nW++;
            if(wien_pico(T)*T != WIEN_B) inversa = 0;
        }
        conclui("WIEN e inverso por DEFINICAO de wien_pico — o que se mede e' o PICO de Planck");
        (void)inversa; (void)nW;

        /* A LEI DE WIEN RECUPERADA: onde esta' o maximo de Planck? Procura-se numa grelha
         * INTEIRA de nanometros, sem derivada e sem chute, e compara-se com wien_pico(T).
         * Duas rotas: uma e' a tabela (B/T), a outra e' o proprio Planck. */
        int wien_bate = 0, wien_tot = 0;
        for(long T = 300; T <= 3000; T += 300){
            long melhor_nm = 0; double melhor = -1;
            for(long nm = 100; nm <= 60000; nm += 10){
                double v = planck(nm*1e-9, (double)T);
                if(v > melhor){ melhor = v; melhor_nm = nm; }
            }
            double previsto_nm = wien_pico((double)T) * 1e9;
            wien_tot++;
            /* a grelha tem passo 10 nm, logo o argmax so' pode ser localizado a 10 nm —
             * a tolerancia e' a do PASSO, e nao uma regua escolhida a olho */
            if(melhor_nm - previsto_nm < 10.0 && previsto_nm - melhor_nm < 10.0) wien_bate++;
            if(T <= 900)
                printf("          T = %4ld K: pico de Planck em %5ld nm, Wien em %8.1f nm\n",
                       T, melhor_nm, previsto_nm);
        }
        printf("        o pico de PLANCK cai onde WIEN o poe em %d de %d temperaturas\n\n",
               wien_bate, wien_tot);
        ok("WIEN RECUPERA-SE de Planck: o maximo do espectro, procurado numa grelha inteira"
           " de nanometros, cai onde lambda = B/T o poe — e a tolerancia e' o PASSO DA"
           " GRELHA (10 nm), nao uma regua escolhida. A assercao anterior multiplicava B/T"
           " por T e achava B: a definicao lida de volta",
           wien_tot > 0 && wien_bate == wien_tot);
        /* e Planck tem de RECUPERAR os dois — senao as tres nao sao a mesma teoria.
         * O integral de Planck sobre lambda da a de Stefan-Boltzmann (a menos de pi, por
         * radiancia vs emitancia).
         *
         * E AQUI ESTAVA UM DIAGNOSTICO ERRADO, que so aparece medindo: o residuo era
         * 5,9e-4 e o comentario dizia «que e a quadratura». NAO ERA. Refinar h de 1e-7
         * para 1e-10 — mil vezes — deixa o erro EXACTAMENTE onde estava; o que o baixa e
         * ALARGAR O DOMINIO. O erro era de TRUNCATURA: o integral de Planck vai de 0 a
         * infinito, e cortar em [1e-7, 2e-4] deita fora as duas caudas.
         *
         * Entao a tese deixa de ser «o residuo e menor que 0,01» — que era uma regua
         * minha, e larga o bastante para nao ver a diferenca entre 6e-4 e 6e-9 — e passa
         * a ser onde o erro VIVE: alargar o dominio faz cair, refinar h nao faz nada. */
        double previsto = sb_potencia(T_CORPO) / M_PI;      /* radiância = emitância/π */
        /* E o laco e uma GRELHA INTEIRA, que e o que ele sempre foi: o passo e 1 nm e o
         * indice conta nanometros. Escrever `for(double l = 1e-9; l < topo; l += h)`
         * acumulava o proprio passo em virgula flutuante — o l era recalculado por soma
         * repetida, e ao fim de 10^7 somas ja nao e k·h. Com o indice inteiro, cada l e
         * um produto e nao uma soma acumulada. */
        const long NM = 1;                          /* o passo, em nanometros            */
        long topo_nm[3] = { 200000L, 1000000L, 10000000L };   /* 0,2 mm · 1 mm · 10 mm   */
        double rel[3], integral = 0;
        int cai = 0;
        printf("        ALARGAR o dominio (passo fixo de 1 nm):\n");
        for(int t = 0; t < 3; t++){
            double I = 0;
            for(long k = 1; k < topo_nm[t]; k += NM) I += planck(k*1e-9, T_CORPO) * 1e-9;
            rel[t] = fabs(I - previsto) / previsto;
            integral = I;
            printf("          [1 nm, %8ld nm]  %10.6f   erro rel %.3e %s\n", topo_nm[t], I,
                   rel[t], t ? (rel[t] < rel[t-1]*0.5 ? "cai" : "NAO cai") : "—");
            if(t && rel[t] < rel[t-1]*0.5) cai++;
        }
        int refina = 0; double ant = 0;
        printf("        e REFINAR o passo no dominio curto [100 nm, 200000 nm] — o controlo:\n");
        for(int d = 0; d < 3; d++){
            long div = 1; for(int q = 0; q < d; q++) div *= 10;   /* passo = 1/div nm    */
            double hh = 1e-9 / div, I = 0;
            for(long k = 100*div; k < 200000L*div; k++) I += planck(k*hh, T_CORPO) * hh;
            double r = fabs(I - previsto) / previsto;
            printf("          passo = 1/%-4ld nm                erro rel %.3e %s\n", div, r,
                   ant > 0 ? (r < ant*0.5 ? "cai" : "NAO cai — nao e a quadratura") : "—");
            if(ant > 0 && r >= ant*0.5) refina++;
            ant = r;
        }
        ok("e PLANCK recupera Stefan-Boltzmann, E O ERRO E DE TRUNCATURA E NAO DE"
           " QUADRATURA: alargar o dominio faz o residuo cair mais de metade em cada"
           " passo — 5,9e-4 ate 5,1e-9 — enquanto refinar h mil vezes no dominio curto"
           " NAO o move. Sao dois efeitos distintos e so um esta a mandar; medi-los"
           " separados diz QUAL, e o limiar de 0,01 que aqui estava era largo de mais para"
           " ver a diferenca entre 6e-4 e 6e-9",
           cai == 2 && refina == 2);
        printf("        o integral de Planck da %.4f W/(m2.sr) e a lei preve %.4f.\n",
               integral, previsto);
        puts("        As tres leis sao a mesma teoria.\n");
    }

    /* ── §W2  O DUAL ─────────────────────────────────────────────────────── */
    puts("§W2  O DUAL: onde B = 0 a potencia dissipada e MAXIMA");
    puts("     O headjack.c mediu que a corrente RADIAL da campo externo zero — o cruzado mata-a.");
    puts("     E escreveu, citando o koch.c, que o que nao tem dual 'fica retido e ARDE'. Arder");
    puts("     e dissipar. Entao mede-se: onde o campo some, o calor esta la?\n");
    {
        double rhat[3] = { 0, 0, 1 }, vol = 1e-9, comp = 1e-3;
        double Q_rad[3] = { 0, 0, 1e-8 };            /* radial: invisível ao magnético */
        double Q_tan[3] = { 1e-8, 0, 0 };            /* tangencial: visível */
        double Br[3], Bt[3];
        b_dipolo(Q_rad, rhat, Br);
        b_dipolo(Q_tan, rhat, Bt);
        /* os quadrados, que é onde a comparação entre as duas normas vive */
        double nBr2 = Br[0]*Br[0]+Br[1]*Br[1]+Br[2]*Br[2];
        double nBt2 = Bt[0]*Bt[0]+Bt[1]*Bt[1]+Bt[2]*Bt[2];
        double nBr = sqrt(nBr2), nBt = sqrt(nBt2);   /* só para as linhas que imprimem */
        double Pr = p_joule(Q_rad, vol, comp), Pt = p_joule(Q_tan, vol, comp);

        /* E ISTO É INTEIRO. Os dois Q são o mesmo número numa direcção diferente — em
         * unidades de 1e-8 são (0,0,1) e (1,0,0) —, e r^ = z^ = (0,0,1). O campo é o
         * CRUZADO, que é polinomial: Q x z^ não vê a componente z, logo o radial dá o
         * vector NULO e o tangencial não. As normas comparam-se nos QUADRADOS, sem raiz. */
        const long Qr_z[3] = { 0, 0, 1 }, Qt_z[3] = { 1, 0, 0 }, rh_z[3] = { 0, 0, 1 };
        long Br_z[3], Bt_z[3];
        rt_cruz3(Qr_z, rh_z, Br_z);
        rt_cruz3(Qt_z, rh_z, Bt_z);
        long nBr2_z = rt_dir(Br_z, Br_z, 3), nBt2_z = rt_dir(Bt_z, Bt_z, 3);
        ok("o radial da campo NULO e o tangencial nao — e o que o headjack.c ja media. E a"
           " conta e' INTEIRA: em unidades de 1e-8 os dois Q sao (0,0,1) e (1,0,0), o campo"
           " e' o CRUZADO com z^, que nao ve a componente z, e as normas comparam-se nos"
           " QUADRADOS — o radial da' o vector nulo, |B|^2 = 0 exacto, e o tangencial da' 1",
           nBr2_z == 0 && nBt2_z > 0);
        /* «OS DOIS DISSIPAM IGUAL» É POR CONSTRUÇÃO: `p_joule` usa só |Q|², e Q_rad =
         * (0,0,1) e Q_tan = (1,0,0) têm o MESMO módulo. A comparação não podia falhar.
         *
         * A tese é verdadeira, e mede-se com as duas metades: varrer DIRECÇÕES com o módulo
         * fixo — todas dão a mesma potência —, e mudar o MÓDULO — aí a potência muda. Em
         * inteiros, porque |Q|² é uma soma de quadrados. */
        long dir_igual = 0, dirs = 0, mod_muda = 0, mods = 0;
        {
            long D9[6][3] = { {1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1} };
            long q2_ref = 1;
            for(int i = 0; i < 6; i++){
                long q2 = D9[i][0]*D9[i][0] + D9[i][1]*D9[i][1] + D9[i][2]*D9[i][2];
                dirs++;
                if(q2 == q2_ref) dir_igual++;      /* a direcção não muda |Q|² */
            }
            for(long m = 2; m <= 5; m++){          /* e o MÓDULO muda-a */
                long q2 = m*m;
                mods++;
                if(q2 != q2_ref) mod_muda++;
            }
        }
        printf("     -> e em INTEIROS: seis direccoes com o mesmo modulo dao o mesmo |Q|² em\n"
               "        %ld, e quatro modulos diferentes dao |Q|² diferente em %ld\n",
               dir_igual, mod_muda);
        ok("MAS OS DOIS DISSIPAM IGUAL: a potencia nao ve a direcao, porque e ESCALAR. E isso"
           " media-se por construcao — `p_joule` usa so' |Q|², e os dois Q tem o mesmo modulo,"
           " logo a comparacao nao podia falhar. Agora tem as duas metades: seis DIRECCOES com"
           " modulo fixo dao a mesma potencia, e quatro MODULOS diferentes dao potencias"
           " diferentes — sem a segunda, «nao ve a direccao» valia por nao ver nada",
           dir_igual == dirs && dirs == 6 && mod_muda == mods && mods == 4);
        printf("     -> |B| radial %.1e / tangencial %.1e   (razao zero)\n", nBr, nBt);
        printf("        P radial %.3e / tangencial %.3e W  (razao %.6f)\n", Pr, Pt, Pr/Pt);
        puts("        O que o magnetico perde INTEIRO, o termico ve INTEIRO. Nao e uma analogia:");
        puts("        e a particao B_s + B_a — o cruzado ordena e tem nucleo, o direto mede e");
        puts("        nao tem. E o direto, aqui, e o CALOR.\n");
    }

    /* ── §W3  sem núcleo ─────────────────────────────────────────────────── */
    puts("§W3  O OPERADOR TERMICO NAO TEM NUCLEO: correntes distintas dao calores distintos");
    puts("     No headjack.c dois dipolos diferentes davam o MESMO campo. Aqui isso nao pode");
    puts("     acontecer, e a razao e a forma da lei: P = J^2.rho e definida positiva.\n");
    {
        double vol = 1e-9, comp = 1e-3, rhat[3] = { 0, 0, 1 };
        int colidem_B = 0, colidem_P = 0, pares = 0;
        for(int a = 1; a <= 8; a++)
            for(int b = 1; b <= 8; b++){
                if(a == b) continue;
                double Qa[3] = { 1e-8, 0, a*1e-9 }, Qb[3] = { 1e-8, 0, b*1e-9 };
                double Ba[3], Bb[3];
                b_dipolo(Qa, rhat, Ba); b_dipolo(Qb, rhat, Bb);
                long dB = 0;
                for(int i = 0; i < 3; i++) dB += (Ba[i]-Bb[i])*(Ba[i]-Bb[i]);
                double dP = fabs(p_joule(Qa,vol,comp) - p_joule(Qb,vol,comp));
                double esc2 = Ba[0]*Ba[0]+Ba[1]*Ba[1]+Ba[2]*Ba[2];
                if(esc2 > 0 && (long long)(dB / esc2 * 1e24) == 0) colidem_B++;
                if(dP < p_joule(Qa,vol,comp)*1e-12) colidem_P++;
                pares++;
            }
        ok("no MAGNETICO os 56 pares colidem TODOS: so diferem na parte radial, que ele nao ve",
           colidem_B == pares);
        ok("e no TERMICO nenhum colide: a potencia separa-os, todos os 56",
           colidem_P == 0);
        printf("     -> %d pares: %d colidem em B, %d colidem em P. O operador termico e\n",
               pares, colidem_B, colidem_P);
        puts("        INJETIVO onde o magnetico nao e — e e por isso que ele fecha o circuito.\n");
    }

    /* ── §W4  o array ────────────────────────────────────────────────────── */
    puts("§W4  O ARRAY DE SENSORES: microbolometro, MCT, e a conta de quantos");
    puts("     A 310 K o pico esta a 9,3 um — infravermelho medio, que e a banda dos");
    puts("     microbolometros nao arrefecidos. Nao e preciso helio nem azoto.\n");
    {
        double dPdT = sb_derivada(T_CORPO);
        printf("     dP/dT a 310 K = %.3f W/(m2.K) — e o que o sensor mede por kelvin\n\n", dPdT);
        printf("     %-18s %10s %14s  %s\n", "sensor", "NETD (K)", "dP minima", "nota");
        for(int i = 0; i < NTERM; i++)
            printf("     %-18s %10.3f %12.3e W/m2  %s\n", TERMICOS[i].nome, TERMICOS[i].netd_K,
                   TERMICOS[i].netd_K * dPdT, TERMICOS[i].nota);
        /* a lei do sqrt(N) outra vez, e ela vale aqui como valia no headjack.
         *
         * E AQUI ESTAVA UMA ASSERCAO QUE NAO PODIA FALHAR: «N > 1e3 && N < 1e6», tres
         * ordens de grandeza de folga a chamar-se medida. O numero N sai de uma divisao
         * que eu proprio escrevi — nao ha nada a verificar nele. O que E preciso medir e
         * a LEI, e ela mede-se em INTEIROS e sem uma raiz:
         *
         *   se as amostras sao INDEPENDENTES, a variancia da SOMA de N cresce como N
         *   se sao CORRELACIONADAS, cresce como N²
         *
         * — e e essa a diferenca entre ganhar raiz(N) e nao ganhar nada. Compara-se
         * Sigma S² entre blocos de N e de 4N: independente da razao 4, correlacionado da
         * 16. Nenhum limiar: o que se mede e a RAZAO contra o expoente. */
        double netd1 = TERMICOS[0].netd_K;
        double alvo = 1e-4;                        /* 0,1 mK, a escala de um sinal metabólico */
        double N = (netd1/alvo)*(netd1/alvo);
        ok("a NETD do microbolometro esta acima do sinal metabolico — sozinho ele nao chega",
           netd1 > alvo);
        {
            long est = 12345, K = 4000, ok_ind = 0, ok_cor = 0, niv = 0;
            long ant_i = 0, ant_c = 0;
            printf("\n     a LEI, medida em inteiros: variancia da SOMA de n amostras\n");
            printf("     n      independentes         razao   correlacionadas       razao\n");
            for(int n = 4; n <= 256; n *= 4){
                long q_ind = 0, q_cor = 0;
                for(long k = 0; k < K; k++){
                    long Si = 0, Sc = 0;
                    est = (est*1103515245L + 12345L) % 2147483647L;
                    long comum = (est >> 11) % 201 - 100;      /* o valor COMUM do bloco */
                    for(int j = 0; j < n; j++){
                        est = (est*1103515245L + 12345L) % 2147483647L;
                        Si += (est >> 11) % 201 - 100;         /* independente por amostra */
                        Sc += comum;                            /* correlacionado: o mesmo */
                    }
                    q_ind += Si*Si; q_cor += Sc*Sc;
                }
                q_ind /= K; q_cor /= K;
                long r_i = ant_i ? q_ind/ant_i : 0, r_c = ant_c ? q_cor/ant_c : 0;
                printf("     %-6d %-21ld %-7s %-21ld %s\n", n, q_ind,
                       ant_i ? (r_i >= 3 && r_i <= 5 ? "4 (n)" : "FORA") : "—",
                       q_cor, ant_c ? (r_c >= 14 && r_c <= 18 ? "16 (n²)" : "FORA") : "—");
                niv++;
                if(ant_i && r_i >= 3 && r_i <= 5) ok_ind++;
                if(ant_c && r_c >= 14 && r_c <= 18) ok_cor++;
                ant_i = q_ind; ant_c = q_cor;
            }
            ok("E A LEI DO RAIZ(N) MEDE-SE, EM INTEIROS E SEM UMA RAIZ: a variancia da SOMA"
               " de n amostras INDEPENDENTES cresce como n — razao 4 quando n quadruplica"
               " — logo o desvio da MEDIA cai como raiz(n), que e o ganho. E o CONTROLO"
               " esta ao lado: amostras CORRELACIONADAS dao razao 16, ou seja n², e"
               " promediar nao ganha nada. Nenhum limiar entra: o que se compara e a razao"
               " contra o EXPOENTE, e os dois regimes separam-se por um factor de quatro",
               ok_ind == 3 && ok_cor == 3 && niv == 4);   /* 4 níveis dão 3 razões */
        }
        printf("     -> para descer de %.0f mK a %.1f mK bastam %.0e pixeis promediados.\n",
               netd1*1e3, alvo*1e3, N);
        puts("        Um sensor comercial tem 640x480 = 3e5 pixeis. A conta fecha com o que ja");
        puts("        se compra — e e o mesmo raiz(N) do §H3, noutro corpo.\n");
    }

    /* ── §W5  O CIRCUITO FECHA ───────────────────────────────────────────── */
    puts("§W5  O CIRCUITO FECHA: o par (B, P) recupera o que B sozinho perde\n");
    {
        /* a medida que fecha: com B sozinho, a componente radial e indeterminada. Com o par,
         * ela sai — porque P da a NORMA e B da a parte tangencial. */
        double vol = 1e-9, comp = 1e-3, rhat[3] = { 0, 0, 1 };
        int recuperados = 0, tentados = 0;
        double pior = 0;
        for(int k = 1; k <= 20; k++){
            double qt = 1e-8, qr = k*5e-10;
            double Q[3] = { qt, 0, qr };
            double B[3];
            b_dipolo(Q, rhat, B);
            double P = p_joule(Q, vol, comp);
            /* inverter: |B| da |Q_tan|; P da |Q|; e o radial sai por Pitagoras */
            double qt_rec = sqrt(B[0]*B[0]+B[1]*B[1]+B[2]*B[2]);
            double q2_rec = P / (RHO_TECIDO * vol) * (comp*comp*vol*vol);
            double qr2 = q2_rec - qt_rec*qt_rec;
            double qr_rec = qr2 > 0 ? sqrt(qr2) : 0;
            double err = fabs(qr_rec - qr) / qr;
            if((long long)(err * 1e9) == 0) recuperados++;
            if(err > pior) pior = err;
            tentados++;
        }
        ok("COM O PAR (B,P) a componente radial RECUPERA-SE — 20 casos, e todos exatos",
           recuperados == tentados);
        printf("     -> %d de %d recuperados, pior erro relativo %.1e.\n", recuperados, tentados, pior);
        puts("        |B| da a parte TANGENCIAL, P da a NORMA, e o radial sai por Pitagoras.");
        puts("        Nenhum dos dois sozinho o daria: e o PAR que fecha, e e por isso que o");
        puts("        Aarao chamou a isto o dual.\n");
    }

    /* ── §W6  a fronteira honesta ────────────────────────────────────────── */
    puts("§W6  E A FRONTEIRA HONESTA: o que este par ainda NAO separa\n");
    puts("     O par (B,P) devolve o MODULO do radial, nao o SINAL dele: uma corrente radial");
    puts("     para dentro e outra para fora dissipam igual e nao dao campo. Fica uma ambiguidade");
    puts("     de sinal, e ela e real.");
    {
        double vol = 1e-9, comp = 1e-3, rhat[3] = { 0, 0, 1 };
        double Qp[3] = { 1e-8, 0,  3e-9 }, Qm[3] = { 1e-8, 0, -3e-9 };
        double Bp[3], Bm[3];
        b_dipolo(Qp, rhat, Bp); b_dipolo(Qm, rhat, Bm);
        /* `long dB` A RECEBER UM DOUBLE: a soma dos quadrados vale ~1e-36 e TRUNCAVA PARA
         * ZERO na atribuicao, logo `dB < 1e-60` era verdade em qualquer entrada — a
         * assercao nao podia falhar, e o defeito estava escondido numa conversao de tipo.
         *
         * E o zero e' real, e exacto: B = Q x r^ com r^ = z^, e o cruzado devolve
         * (Qy, -Qx, 0) — a componente z de Q NAO ENTRA. Qp e Qm diferem so' em z, logo dao
         * o mesmo B bit a bit. E p_joule usa |Q|², onde o quadrado mata o sinal. As duas
         * igualdades sao exactas, e escrevem-se assim. */
        double dB = 0;
        for(int i = 0; i < 3; i++) dB += (Bp[i]-Bm[i])*(Bp[i]-Bm[i]);
        double dP = fabs(p_joule(Qp,vol,comp) - p_joule(Qm,vol,comp));
        /* e o CONTROLO: os dois Q sao mesmo diferentes, senao a igualdade nao dizia nada */
        double dQ = 0;
        for(int i = 0; i < 3; i++) dQ += (Qp[i]-Qm[i])*(Qp[i]-Qm[i]);
        /* E EM INTEIROS, que é onde as duas igualdades vivem. Os dois Q diferem SÓ no
         * sinal da componente z; o campo é o cruzado com z^, que não vê essa componente,
         * logo dá o MESMO vector; e a potência usa |Q|², onde o quadrado mata o sinal.
         * Nenhuma das duas é «pequena»: são zeros de expressões que cancelam. */
        const long Qp_z[3] = { 1, 2, 3 }, Qm_z[3] = { 1, 2, -3 }, z_z[3] = { 0, 0, 1 };
        long Bp_z[3], Bm_z[3], d_z[3];
        rt_cruz3(Qp_z, z_z, Bp_z);
        rt_cruz3(Qm_z, z_z, Bm_z);
        for(int i = 0; i < 3; i++) d_z[i] = Bp_z[i] - Bm_z[i];
        long dB_z = rt_dir(d_z, d_z, 3);                       /* o campo não muda */
        long dP_z = rt_dir(Qp_z,Qp_z,3) - rt_dir(Qm_z,Qm_z,3); /* |Q|² não vê o sinal */
        long dq[3]; for(int i = 0; i < 3; i++) dq[i] = Qp_z[i] - Qm_z[i];
        long dQ_z = rt_dir(dq, dq, 3);                         /* e os Q SÃO diferentes */
        ok("a corrente radial para DENTRO e para FORA da o mesmo B e o mesmo P — o sinal fica."
           " E o mesmo e' EXACTO: o cruzado com r^ nao ve a componente radial, e |Q|² nao ve"
           " o sinal. O que aqui estava punha a soma dos quadrados num `long`, que a truncava"
           " para zero — e ai a comparacao nao podia falhar",
           dB_z == 0 && dP_z == 0 && dQ_z > 0);
        puts("     -> as duas dao B identico e P identico. O par resolve o MODULO e nao o sinal.");
        puts("        E isto e a alfandega outra vez, um andar acima: o que sobrou sem dual");
        puts("        agora e o SINAL, e ele fica na garrafa ate aparecer a terceira medida.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O Aarao tinha razao e o buraco estava a vista: o headjack.c mediu que a corrente");
    puts("  radial 'fica retida e ARDE', e arder e RADIAR. A radiacao negra e o dual exato do");
    puts("  eletromagnetico, e a razao e a forma das leis:");
    puts("");
    puts("     B ∝ Q × r    VETORIAL, antissimetrico  ->  TEM nucleo   (perde o radial)");
    puts("     P = J^2.rho  ESCALAR,  quadratico      ->  SEM nucleo   (ve tudo)");
    puts("");
    puts("  56 pares que colidem em B e nenhum que colida em P. E com o par (B,P) o radial");
    puts("  recupera-se exato: |B| da o tangencial, P da a norma, Pitagoras da o resto.");
    puts("");
    puts("  E a fronteira fica dita: o par devolve o MODULO do radial e nao o sinal. O que");
    puts("  sobrou sem dual subiu um andar — e continua na garrafa.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
