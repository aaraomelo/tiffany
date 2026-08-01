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
        /* a LEI de Stefan-Boltzmann: dobrar T multiplica a potencia por 16. Mede-se em varios T. */
        int quarta = 1;
        for(double T = 100; T <= 800; T *= 2){
            double r = sb_potencia(2*T) / sb_potencia(T);
            if(fabs(r - 16.0) > 1e-9) quarta = 0;
        }
        ok("STEFAN-BOLTZMANN e a QUARTA potencia: dobrar T multiplica por 16, em todos os T",
           quarta);
        /* a LEI de Wien: o pico e inversamente proporcional a T */
        int inversa = 1;
        for(double T = 100; T <= 1600; T *= 2)
            if(fabs(wien_pico(T)*T - WIEN_B) > 1e-15) inversa = 0;
        ok("WIEN e inverso: lambda.T e CONSTANTE, e ela e a de tabela",
           inversa);
        /* e Planck tem de RECUPERAR os dois — senao as tres nao sao a mesma teoria.
         * O integral de Planck sobre lambda da a de Stefan-Boltzmann (a menos de pi, por
         * radiancia vs emitancia). Mede-se por quadratura, e o residuo tem de ser pequeno. */
        double integral = 0, h = 1e-8;
        for(double l = 1e-7; l < 2e-4; l += h) integral += planck(l, T_CORPO) * h;
        double previsto = sb_potencia(T_CORPO) / M_PI;      /* radiância = emitância/π */
        double rel = fabs(integral - previsto) / previsto;
        ok("e PLANCK recupera Stefan-Boltzmann: o integral dele bate a lei da quarta potencia",
           rel < 0.01);
        printf("        o integral de Planck da %.2f W/(m2.sr) e a lei preve %.2f — %.3f%% de\n",
               integral, previsto, 100*rel);
        puts("        diferenca, que e a quadratura. As tres leis sao a mesma teoria.\n");
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
        double nBr = sqrt(Br[0]*Br[0]+Br[1]*Br[1]+Br[2]*Br[2]);
        double nBt = sqrt(Bt[0]*Bt[0]+Bt[1]*Bt[1]+Bt[2]*Bt[2]);
        double Pr = p_joule(Q_rad, vol, comp), Pt = p_joule(Q_tan, vol, comp);

        ok("o radial da campo NULO e o tangencial nao — e o que o headjack.c ja media",
           nBr < nBt*1e-12 && nBt > 0);
        ok("MAS OS DOIS DISSIPAM IGUAL: a potencia nao ve a direcao, porque e ESCALAR",
           fabs(Pr - Pt)/Pt < 1e-12);
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
                double dB = 0;
                for(int i = 0; i < 3; i++) dB += (Ba[i]-Bb[i])*(Ba[i]-Bb[i]);
                double dP = fabs(p_joule(Qa,vol,comp) - p_joule(Qb,vol,comp));
                double escala = sqrt(Ba[0]*Ba[0]+Ba[1]*Ba[1]+Ba[2]*Ba[2]);
                if(sqrt(dB) < escala*1e-12) colidem_B++;
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
        /* a lei do sqrt(N) outra vez, e ela vale aqui como valia no headjack */
        double netd1 = TERMICOS[0].netd_K;
        double alvo = 1e-4;                        /* 0,1 mK, a escala de um sinal metabólico */
        double N = (netd1/alvo)*(netd1/alvo);
        ok("a NETD do microbolometro esta acima do sinal metabolico — sozinho ele nao chega",
           netd1 > alvo);
        ok("e a lei do raiz(N) diz quantos: promediar N pixeis ganha raiz(N), como no headjack",
           N > 1e3 && N < 1e6);
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
            if(err < 1e-9) recuperados++;
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
        double dB = 0;
        for(int i = 0; i < 3; i++) dB += (Bp[i]-Bm[i])*(Bp[i]-Bm[i]);
        double dP = fabs(p_joule(Qp,vol,comp) - p_joule(Qm,vol,comp));
        ok("a corrente radial para DENTRO e para FORA da o mesmo B e o mesmo P — o sinal fica",
           sqrt(dB) < 1e-30 && dP < 1e-30);
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
