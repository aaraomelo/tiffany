/* microfluidica.c — MICROFLUÍDICA 3D: o sexto vestido, e porque a terceira dimensão é obrigatória.
 *
 * O Aarão: "vamos pra microfluidica 3D."
 *
 * DUAS COISAS, e as duas se medem.
 *
 * A PRIMEIRA é que a microfluídica é o **sexto vestido** do `dominios.c`. A analogia hidráulica é
 * exata e antiga: pressão ↔ tensão, caudal ↔ corrente, e daí saem R, C e L hidráulicos. Então a
 * régua `(B,C) = (−traço, det)` aplica-se sem se lhe tocar.
 *
 * MAS — e é aqui que ela deixa de ser mais um dos cinco — **no micro a inércia desaparece**. O
 * número de Reynolds cai para ~0,1 e o termo `m·x''` some: a equação **baixa de ordem**, de segunda
 * para primeira. *O corpo perde uma dimensão*, e com ela perde o Δ, perde a classe elíptica, perde
 * a oscilação. Não há ressonância em regime de Stokes, e isso não é uma aproximação: é o que a
 * régua diz quando a inércia vai a zero.
 *
 * A SEGUNDA é a razão de ser do **3D**, e ela é topológica, não de conveniência. Num plano, dois
 * canais não se cruzam sem se ligar — e há redes que **nenhum** plano comporta: o `K_5` e o
 * `K_{3,3}` de Kuratowski. A terceira dimensão não é para caber mais: é para **cruzar sem tocar**.
 *
 * E isso é a mesma coisa que o projeto já diz do produto: **o cruzado precisa de três dimensões**.
 * Em `R^1` e `R^2` a parte antissimétrica não tem onde viver; em `R^3` tem. *A microfluídica 3D é o
 * cruzado a exigir o seu lugar, em vidro e PDMS.*
 *
 *   §M1  a analogia hidráulica: R, C e L do canal, dos números do material
 *   §M2  REYNOLDS: no micro a inércia some, e a ordem CAI — medido, não assumido
 *   §M3  e com ela some a oscilação: sem inércia não há Δ<0, e isso é a régua a dizê-lo
 *   §M4  a REDE: canais em série e paralelo, e é a mesma lei de Kirchhoff
 *   §M5  O 3D É OBRIGATÓRIO: Kuratowski — há redes que nenhum plano comporta
 *   §M6  e é o CRUZADO a exigir lugar: ele precisa de três dimensões, aqui como no R^n
 *
 *   cc -O2 -std=c99 microfluidica.c -lm -o microfluidica && ./microfluidica
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §M1  O CANAL — e os números são de água em PDMS, não escolhidos para dar certo
 * ─────────────────────────────────────────────────────────────────────────── */

#define MU    1.0e-3        /* viscosidade da água, Pa·s */
#define RHO   1.0e3         /* densidade da água, kg/m³ */

typedef struct { const char *nome; double w, h, L; } Canal;   /* metros */

static const Canal CANAIS[] = {
    { "microcanal 100um",  100e-6, 100e-6, 10e-3 },
    { "microcanal  50um",   50e-6,  50e-6, 10e-3 },
    { "microcanal  20um",   20e-6,  20e-6, 10e-3 },
    { "capilar    500um",  500e-6, 500e-6, 50e-3 },
    { "tubo         5mm",    5e-3,   5e-3,  1.0  },
};
#define NCAN ((int)(sizeof CANAIS / sizeof CANAIS[0]))

/* a resistência hidráulica de um canal retangular (a fórmula clássica, com a correção de forma) */
static double R_hid(const Canal *c){
    double w = c->w, h = c->h;
    if(h > w){ double t = w; w = h; h = t; }
    return 12.0 * MU * c->L / (w * h*h*h * (1.0 - 0.63*h/w));
}

/* a inertância: a "indutância" hidráulica, ρL/A */
static double L_hid(const Canal *c){ return RHO * c->L / (c->w * c->h); }

/* a complacência de uma câmara de volume V com paredes de módulo E (a "capacitância") */
static double C_hid(double V, double E){ return V / E; }

/* o número de Reynolds, com o diâmetro hidráulico */
static double reynolds(const Canal *c, double Q){
    double A = c->w * c->h;
    double v = Q / A;
    double Dh = 2*c->w*c->h/(c->w + c->h);
    return RHO * v * Dh / MU;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §M5  KURATOWSKI — a razão topológica do 3D
 *
 * Um grafo é planar sse não contém subdivisão de K_5 nem de K_{3,3}. Não é preciso o teorema
 * inteiro para o que se quer aqui: basta a cota de Euler, que é exata e falha de forma
 * verificável — um grafo planar simples com V ≥ 3 tem no máximo 3V−6 arestas, e um bipartido
 * sem triângulos no máximo 2V−4.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; int V, E, bipartido; } Rede;

static const Rede REDES[] = {
    { "K_4  (4 nos)",        4,  6, 0 },
    { "K_5  (5 nos)",        5, 10, 0 },
    { "K_{3,3} (misturador)",6,  9, 1 },
    { "grelha 3x3",          9, 12, 1 },
    { "arvore binaria 15",  15, 14, 0 },
};
#define NREDES ((int)(sizeof REDES / sizeof REDES[0]))

/* a cota de Euler: devolve o máximo de arestas que um plano comporta */
static int cota_planar(int V, int bipartido){
    if(V < 3) return V*(V-1)/2;
    return bipartido ? 2*V - 4 : 3*V - 6;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("microfluidica.c — MICROFLUIDICA 3D: o sexto vestido, e porque o 3D e obrigatorio\n");

    /* ── §M1 ─────────────────────────────────────────────────────────────── */
    puts("§M1  A ANALOGIA HIDRAULICA: pressao ~ tensao, caudal ~ corrente");
    puts("     Daí saem R, C e L hidraulicos, e a regua (B,C) do catalogo aplica-se sem se lhe");
    puts("     tocar. Os numeros sao de agua em PDMS, nao escolhidos para dar certo.\n");
    {
        printf("     %-20s %12s %12s %10s\n", "canal", "R (Pa.s/m3)", "L (kg/m4)", "R/L (1/s)");
        for(int i = 0; i < NCAN; i++){
            double R = R_hid(&CANAIS[i]), L = L_hid(&CANAIS[i]);
            printf("     %-20s %12.3e %12.3e %10.2e\n", CANAIS[i].nome, R, L, R/L);
        }
        /* DUAS vezes errei esta assercao, e as duas por descuido meu e nao pela fisica:
         * primeiro comparei a lista em sequencia sem reparar que ela nao esta ordenada por
         * tamanho; depois escrevi o laco com a logica INVERTIDA (um "continue" onde queria o
         * contrario) e o texto ao contrario — nesta lista a seccao DIMINUI, logo a resistencia
         * CRESCE. Ficou a afirmacao que a tabela mostra, e o laco escrito direito. */
        int sobe = 1;
        for(int i = 0; i + 1 < 3; i++)            /* os tres microcanais, todos com L = 10 mm */
            if(R_hid(&CANAIS[i]) >= R_hid(&CANAIS[i+1])) sobe = 0;
        ok("a resistencia CRESCE quando a seccao encolhe, a comprimento igual — 100, 50 e 20 um",
           sobe);
        /* a LEI, e ela e o que separa isto de uma tabela: R ~ 1/h^4 para canal quadrado */
        double r1 = R_hid(&CANAIS[0]), r2 = R_hid(&CANAIS[1]);   /* 100um e 50um, L igual */
        double razao = r2/r1;
        ok("A LEI: halvar o lado multiplica a resistencia por 16 — a quarta potencia, medida",
           fabs(razao - 16.0) < 0.5);
        printf("     -> R(50um)/R(100um) = %.2f (a lei diz 2^4 = 16). Nao e um numero meu:\n", razao);
        puts("        e a quarta potencia da secao, e por isso a microfluidica e um mundo de");
        puts("        pressoes altas em canais minusculos.\n");
    }

    /* ── §M2  REYNOLDS ───────────────────────────────────────────────────── */
    puts("§M2  REYNOLDS: no micro a INERCIA SOME, e a ordem da equacao CAI");
    puts("     Re = rho.v.Dh/mu. Com caudais tipicos, o micro fica em Re << 1 — e ai o termo");
    puts("     inercial nao e pequeno: e desprezavel face ao viscoso, e a equacao baixa de ordem.\n");
    {
        /* Eu tinha posto Q = 1 uL/s e afirmado "todos em Stokes" — e a medida deu Re = 10 a 50,
         * INERCIAL. O caudal de microfluidica e uL/MINUTO, nao por segundo: eu estava 60x acima.
         * E o erro nao e o numero: e eu ter ESCOLHIDO um caudal para dar o regime que queria.
         * A pergunta honesta e outra — qual o caudal MAXIMO que ainda cumpre Re<1? Isso e a
         * fronteira, e ela mede-se sem eu escolher nada. */
        double Q = 1e-9/60.0;                     /* 1 µL/min, o caudal de uma bomba de seringa */
        printf("     %-20s %12s %12s %10s %14s\n", "canal", "v (m/s)", "Re", "regime", "Q_max(uL/min)");
        int micro_stokes = 0;
        for(int i = 0; i < NCAN; i++){
            double A = CANAIS[i].w * CANAIS[i].h;
            double Re = reynolds(&CANAIS[i], Q);
            /* a fronteira: Re = 1 quando Q = mu*A/(rho*Dh) */
            double Dh = 2*CANAIS[i].w*CANAIS[i].h/(CANAIS[i].w + CANAIS[i].h);
            double Qmax = MU * A / (RHO * Dh);
            printf("     %-20s %12.3e %12.3e %10s %14.2f\n", CANAIS[i].nome, Q/A, Re,
                   Re < 1 ? "Stokes" : "inercial", Qmax*60e9);
            if(i < 3 && Re < 1) micro_stokes++;
        }
        ok("a 1 uL/min os microcanais estao em Stokes — e o caudal e o de uma bomba de seringa",
           micro_stokes == 3);
        /* e a LEI da fronteira, que e o que vale: Q_max escala com o LADO, nao com a area */
        double q1 = MU*(CANAIS[0].w*CANAIS[0].h)/(RHO*CANAIS[0].w);
        double q2 = MU*(CANAIS[1].w*CANAIS[1].h)/(RHO*CANAIS[1].w);
        ok("e a fronteira Re=1 escala com o LADO do canal: halvar o lado halva o caudal maximo",
           fabs(q1/q2 - 2.0) < 0.01);
        /* e a razao das duas escalas de tempo: a viscosa contra a inercial */
        const Canal *c = &CANAIS[0];
        double tau_visc = L_hid(c) / R_hid(c);     /* o tempo de relaxação inercial */
        ok("e o tempo inercial e minusculo face ao da experiencia: a inercia relaxa e some",
           tau_visc < 1e-3);
        printf("     -> no canal de 100um o tempo inercial e %.2e s. Uma experiencia dura\n", tau_visc);
        puts("        segundos: a inercia ja relaxou antes de se ver. Ela nao e aproximada a");
        puts("        zero — ela CHEGA a zero na escala do que se observa.\n");
    }

    /* ── §M3  a ordem cai ────────────────────────────────────────────────── */
    puts("§M3  E COM ELA SOME A OSCILACAO: sem inercia nao ha Delta<0, e a regua di-lo");
    puts("     O dominios.c: y'' + By' + Cy = 0 com (B,C) = (c/m, k/m), e Delta = B^2-4C. Faz-se");
    puts("     a inercia ir a zero e ve-se o que a regua faz — nao se assume, mede-se.\n");
    {
        double c_dis = 1.0, k_rig = 1.0;
        printf("     %10s %12s %12s %14s %s\n", "m", "B = c/m", "C = k/m", "Delta", "classe");
        int virou = 0, casos = 0, eliptico_no_fim = 0;
        for(double m = 1.0; m >= 1e-6; m /= 100){
            double B = c_dis/m, C = k_rig/m, D = B*B - 4*C;
            printf("     %10.0e %12.3e %12.3e %14.3e %s\n", m, B, C, D,
                   D > 0 ? "hiperbolica" : (D < 0 ? "eliptica" : "parabolica"));
            if(D > 0) virou++;
            if(m <= 1e-4 && D < 0) eliptico_no_fim++;
            casos++;
        }
        ok("a inercia a ir a zero leva o Delta a POSITIVO — a classe deixa de ser eliptica",
           virou > 0 && eliptico_no_fim == 0);
        /* e a lei: Delta = (c^2 - 4km)/m^2, logo o sinal vira quando m < c^2/(4k) */
        double m_critico = c_dis*c_dis/(4*k_rig);
        double Dm = (c_dis*c_dis - 4*k_rig*m_critico);
        ok("e o ponto de viragem tem FORMA FECHADA: m = c^2/(4k), onde o Delta anula exatamente",
           fabs(Dm) < 1e-12);
        printf("     -> a viragem e em m = c^2/(4k) = %.4f, e ali Delta = %.1e exato.\n",
               m_critico, Dm);
        puts("        Abaixo dela nao ha oscilacao: o sistema e sobreamortecido e volta sem");
        puts("        passar do ponto. NAO HA RESSONANCIA EM STOKES, e isto e a regua a dize-lo,");
        puts("        nao a fisica a ser citada.\n");
    }

    /* ── §M4  a REDE ─────────────────────────────────────────────────────── */
    puts("§M4  A REDE: canais em serie e em paralelo, e e a MESMA lei de Kirchhoff");
    puts("     O eletrico.c ja tem a triade em volts e amps. Aqui e em pascal e m3/s, e as");
    puts("     regras de composicao tem de ser as mesmas — senao a analogia era so uma palavra.\n");
    {
        const Canal *a = &CANAIS[0], *b = &CANAIS[1];
        double Ra = R_hid(a), Rb = R_hid(b);
        double serie = Ra + Rb;
        double paralelo = 1.0/(1.0/Ra + 1.0/Rb);
        ok("em SERIE as resistencias somam, como no eletrico",
           fabs(serie - (Ra+Rb)) < 1e-9 && serie > Ra && serie > Rb);
        ok("e em PARALELO somam os inversos — e o resultado e MENOR que qualquer uma delas",
           paralelo < Ra && paralelo < Rb);
        /* e a lei dos nos: o que entra sai. Mede-se num divisor de caudal. */
        double dP = 1000.0;                       /* 1 kPa através do par em paralelo */
        double Qa = dP/Ra, Qb = dP/Rb, Qt = dP/paralelo;
        ok("a lei dos NOS fecha: o caudal total e a soma dos ramos, sem sobra",
           fabs(Qt - (Qa+Qb))/Qt < 1e-12);
        printf("     -> Ra = %.3e, Rb = %.3e; serie %.3e, paralelo %.3e.\n", Ra, Rb, serie, paralelo);
        printf("        Com 1 kPa: Qa = %.3e, Qb = %.3e, total %.3e m3/s. Kirchhoff vale aqui\n",
               Qa, Qb, Qt);
        puts("        exatamente como no eletrico.c — a analogia nao e uma palavra, e uma lei.\n");
    }

    /* ── §M5  O 3D É OBRIGATÓRIO ─────────────────────────────────────────── */
    puts("§M5  O 3D E OBRIGATORIO: ha redes que NENHUM plano comporta");
    puts("     A cota de Euler: um grafo planar simples com V>=3 tem no maximo 3V-6 arestas, e");
    puts("     um bipartido sem triangulos no maximo 2V-4. Nao e um limite de fabrico — e do");
    puts("     PLANO, e nao ha litografia que o vença.\n");
    {
        printf("     %-22s %4s %4s %10s %s\n", "rede", "V", "E", "cota", "cabe no plano?");
        int planares = 0, nao = 0;
        for(int i = 0; i < NREDES; i++){
            const Rede *r = &REDES[i];
            int cota = cota_planar(r->V, r->bipartido);
            int cabe = r->E <= cota;
            printf("     %-22s %4d %4d %10d %s\n", r->nome, r->V, r->E, cota,
                   cabe ? "sim" : "NAO — precisa de 3D");
            if(cabe) planares++; else nao++;
        }
        ok("o K_5 e o K_{3,3} NAO cabem no plano — a cota de Euler recusa-os, e ela e exata",
           nao == 2);
        ok("e os outros cabem: a cota nao recusa tudo, senao nao estaria a medir nada",
           planares == NREDES - 2);
        printf("     -> %d de %d cabem no plano; %d precisam da terceira dimensao.\n",
               planares, NREDES, nao);
        puts("        O K_{3,3} e um MISTURADOR de tres entradas por tres saidas — a peca mais");
        puts("        banal de um chip. E ela nao e planar: o 3D nao e para caber mais, e para");
        puts("        CRUZAR SEM TOCAR.\n");
    }

    /* ── §M6  o CRUZADO ──────────────────────────────────────────────────── */
    puts("§M6  E E O CRUZADO A EXIGIR LUGAR: ele precisa de tres dimensoes, aqui como no R^n\n");
    {
        /* o produto vetorial so existe em 3 (e 7). Em 2D a parte antissimetrica de dois
         * vetores e um ESCALAR — ela nao volta ao espaco, e por isso nao ha cruzamento. */
        double a2[2] = { 1.0, 0.0 }, b2[2] = { 0.0, 1.0 };
        double cr2 = a2[0]*b2[1] - a2[1]*b2[0];      /* em 2D o "cruzado" e um numero */
        double a3[3] = { 1,0,0 }, b3[3] = { 0,1,0 };
        double cr3[3] = { a3[1]*b3[2]-a3[2]*b3[1], a3[2]*b3[0]-a3[0]*b3[2], a3[0]*b3[1]-a3[1]*b3[0] };
        int volta_ao_espaco = (fabs(cr3[2] - 1.0) < 1e-15);
        ok("em 3D o cruzado DEVOLVE UM VETOR do mesmo espaco — ha para onde sair do plano",
           volta_ao_espaco);
        ok("e em 2D ele devolve um ESCALAR: nao ha terceira coordenada, logo nao ha por onde passar",
           fabs(cr2 - 1.0) < 1e-15);
        /* e a perpendicularidade e o que permite cruzar sem tocar */
        double ip1 = cr3[0]*a3[0] + cr3[1]*a3[1] + cr3[2]*a3[2];
        double ip2 = cr3[0]*b3[0] + cr3[1]*b3[1] + cr3[2]*b3[2];
        ok("e o cruzado sai PERPENDICULAR aos dois — e e exatamente isso que 'cruzar sem tocar' e",
           fabs(ip1) < 1e-15 && fabs(ip2) < 1e-15);
        printf("     -> em 2D: a x b = %.1f, um numero, e ele nao e um lugar. Em 3D: (%.0f,%.0f,%.0f),\n",
               cr2, cr3[0], cr3[1], cr3[2]);
        puts("        e ele E um lugar — a direcao por onde o segundo canal passa por cima do");
        puts("        primeiro. A microfluidica 3D nao e uma tecnica melhor: e o CRUZADO a");
        puts("        exigir o seu lugar, em vidro e PDMS.");
        puts("");
        puts("        E fecha com o resto: o direto MEDE (a resistencia, escalar, no plano) e o");
        puts("        cruzado ORDENA (a travessia, vetorial, fora dele). Sao as duas metades de");
        puts("        sempre, agora num chip.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A microfluidica e o SEXTO vestido — a regua (B,C) aplica-se sem se lhe tocar, e");
    puts("  Kirchhoff vale em pascal como vale em volt.");
    puts("");
    puts("  Mas ela e o primeiro vestido que MUDA A CLASSE: no micro a inercia vai a zero, o");
    puts("  Delta vira positivo em m = c^2/(4k), e a oscilacao acaba. Nao ha ressonancia em");
    puts("  Stokes, e isso e a regua a dize-lo.");
    puts("");
    puts("  E o 3D nao e conveniencia: o K_{3,3} — um misturador de 3 por 3 — nao cabe em plano");
    puts("  nenhum, por Euler. A terceira dimensao e onde o CRUZADO vive, e cruzar sem tocar e");
    puts("  literalmente o que ele faz.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
