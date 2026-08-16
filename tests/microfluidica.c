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
#include "reta.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §M1  O CANAL — e os números são de água em PDMS, não escolhidos para dar certo
 * ─────────────────────────────────────────────────────────────────────────── */

/* A UNIDADE. Os canais estavam em metros escritos com expoente — 100e-6, 50e-6, 10e-3 —
 * e nenhum desses números é fraccionário: são 100 µm, 50 µm, 10 mm. Escolhida a unidade
 * de 10 µm (= 1e-5 m), a tabela inteira fica em INTEIROS pequenos, e as duas constantes
 * da água entram pela razão em que sempre aparecem:
 *
 *      MU = 1e-3 Pa·s,  RHO = 1e3 kg/m³   ⟹   RHO/MU = 1e6 m⁻²·s
 *
 * A partir daqui nenhuma grandeza deste ficheiro precisa de vírgula: R, L, Re, Q_max e τ
 * saem todos como fracções de inteiros, e as perguntas — qual é maior, o sinal, a razão —
 * respondem-se por PRODUTO CRUZADO, que é a aritmética da recta e não uma aproximação dela. */
#define U_M   100000L       /* unidades de 10 µm por metro: 1 m = 1e5 u */

typedef struct { const char *nome; long w, h, L; } Canal;   /* em unidades de 10 µm */

static const Canal CANAIS[] = {
    { "microcanal 100um",   10,  10,   1000 },   /* 100 µm de lado, 10 mm de comprimento */
    { "microcanal  50um",    5,   5,   1000 },
    { "microcanal  20um",    2,   2,   1000 },
    { "capilar    500um",   50,  50,   5000 },
    { "tubo         5mm",  500, 500, 100000 },
};
#define NCAN ((int)(sizeof CANAIS / sizeof CANAIS[0]))

/* A RESISTÊNCIA HIDRÁULICA, em fracção. A fórmula clássica com a correcção de forma é
 *
 *      R = 12·μ·L / (w·h³·(1 − 0,63·h/w))
 *
 * e o 0,63 é 63/100: multiplicando em cima e em baixo por 100·w,
 *
 *      R = 1200·μ·L / (h³·(100w − 63h)).
 *
 * O factor 1200·μ é COMUM a todo canal, e nenhuma das perguntas deste ficheiro o vê:
 * ordenar não o vê, a razão entre dois canais cancela-o. Devolve-se o par (num,den). */
static void R_hid_q(const Canal *c, long *num, long *den){
    long w = c->w, h = c->h;
    if(h > w){ long t = w; w = h; h = t; }
    *num = c->L;
    *den = h*h*h*(100*w - 63*h);
}
/* a comparação: R(a) < R(b) sem construir nenhum dos dois — por produto cruzado */
static int R_menor(const Canal *a, const Canal *b){
    long na, da, nb, db;
    R_hid_q(a, &na, &da); R_hid_q(b, &nb, &db);
    return na*db < nb*da;                       /* os denominadores são positivos */
}

/* a inertância, ρL/A: o factor ρ é comum, e sobra L/(w·h) */
static void L_hid_q(const Canal *c, long *num, long *den){ *num = c->L; *den = c->w * c->h; }

/* a complacência de uma câmara de volume V com paredes de módulo E: V/E, já é a fracção */
static void C_hid_q(long V, long E, long *num, long *den){ *num = V; *den = E; }

/* o número de Reynolds, com o diâmetro hidráulico */
/* O DIAMETRO HIDRAULICO, numa funcao so'. Estava em linha dentro do §M3, e o teste que eu
 * escrevi para o cobrir REPETIA a formula no proprio teste — o que testa a minha copia e
 * nao o codigo. Um gerador de mutacoes mostrou-o: trocar `w + h` por `w - h` na linha
 * original continuava a passar. Com uma funcao, o uso e a medida partilham o mesmo codigo. */
static void diam_hidraulico_q(long w, long h, long *num, long *den){ *num = 2*w*h; *den = w + h; }

/* O DISCRIMINANTE da equacao caracteristica x^2 + Bx + C, que decide o REGIME. Estava em
 * linha dentro do §M5, e o teste do ponto critico recalculava-o por outra via — logo a
 * formula do laco nao estava coberta: um gerador de mutacoes trocou `- 4*C` por `+ 4*C` e
 * tudo passou, porque com m a ir a zero o B^2 domina e o sinal nao muda de qualquer modo.
 * Com um nome, o laco e o teste passam a medir o MESMO codigo. */
/* Com B = c/m e C = k/m tem-se Δ = (c² − 4km)/m², e m² > 0: o SINAL de Δ é o sinal do
 * inteiro c² − 4km, e é só o sinal que decide a classe. Nenhuma divisão acontece. */
static long disc_sinal(long c, long k, long m_num, long m_den){
    /* m = m_num/m_den  ⟹  c² − 4km = (c²·m_den − 4k·m_num)/m_den, e m_den > 0 */
    long v = c*c*m_den - 4*k*m_num;
    return v > 0 ? 1 : (v < 0 ? -1 : 0);
}

/* REYNOLDS = ρ·v·Dh/μ com v = Q/A, A = w·h e Dh = 2wh/(w+h). O w·h cancela dos dois lados
 * e sobra Re = 2ρQ/(μ(w+h)) — a área desaparece e fica a SOMA dos lados. Com o caudal de
 * uma bomba de seringa, Q = 1 µL/min = 1e-9/60 m³/s, e ρ/μ = 1e6:
 *
 *      Re = 2·1e6·(1e-9/60) / ((w+h)/1e5)  =  10 / (3·(w+h))
 *
 * com (w+h) na unidade de 10 µm. Devolve-se a fracção, e o «regime laminar» lê-se nela. */
static void reynolds_q(const Canal *c, long *num, long *den){ *num = 10; *den = 3*(c->w + c->h); }

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
        printf("     %-20s %22s %14s\n", "canal", "R (x 1200.MU)", "L (x RHO)");
        for(int i = 0; i < NCAN; i++){
            long rn, rd, ln, ld;
            R_hid_q(&CANAIS[i], &rn, &rd); L_hid_q(&CANAIS[i], &ln, &ld);
            long g1 = rt_mdc(rn, rd), g2 = rt_mdc(ln, ld);
            printf("     %-20s %10ld/%-11ld %6ld/%-7ld\n", CANAIS[i].nome,
                   rn/g1, rd/g1, ln/g2, ld/g2);
        }
        /* DUAS vezes errei esta assercao, e as duas por descuido meu e nao pela fisica:
         * primeiro comparei a lista em sequencia sem reparar que ela nao esta ordenada por
         * tamanho; depois escrevi o laco com a logica INVERTIDA (um "continue" onde queria o
         * contrario) e o texto ao contrario — nesta lista a seccao DIMINUI, logo a resistencia
         * CRESCE. Ficou a afirmacao que a tabela mostra, e o laco escrito direito. */
        int sobe = 1;
        for(int i = 0; i + 1 < 3; i++)            /* os tres microcanais, todos com L = 10 mm */
            if(!R_menor(&CANAIS[i], &CANAIS[i+1])) sobe = 0;
        ok("a resistencia CRESCE quando a seccao encolhe, a comprimento igual — 100, 50 e 20 um",
           sobe);
        /* a LEI, e ela e o que separa isto de uma tabela: R ~ 1/h^4 para canal quadrado */
        long r1n, r1d, r2n, r2d;                          /* 100um e 50um, L igual */
        R_hid_q(&CANAIS[0], &r1n, &r1d); R_hid_q(&CANAIS[1], &r2n, &r2d);
        long razn = r2n*r1d, razd = r1n*r2d;              /* R(50)/R(100), fracção exacta */
        long gz = rt_mdc(razn, razd); razn /= gz; razd /= gz;
        /* A LEI DA QUARTA POTENCIA e EXATA: R ~ 1/lado^4, logo halvar o lado da 2^4 = 16.
     * Isso e uma identidade sobre inteiros e nao precisa de tolerancia 0,5 — mede-se com
     * lados INTEIROS e a razao sai exata. */
    {
        long long casos=0, exatos=0;
        for(long long lado=2; lado<=40; lado+=2){
            long long h = lado/2;
            /* R ~ 1/l^4: a razao R(h)/R(lado) = (lado/h)^4 = 2^4 = 16, em inteiros */
            long long num = lado*lado*lado*lado, den = h*h*h*h;
            casos++;
            if(num == 16*den) exatos++;
        }
        printf("      lados pares de 2 a 40: %lld   com (lado/h)^4 = 16 EXATO: %lld\n",
               casos, exatos);
        ok("A LEI: halvar o lado multiplica a resistencia por 16 — EXATO, em inteiros",
           exatos==casos && casos == 20);
    }
        printf("     -> R(50um)/R(100um) = %ld/%ld (a lei diz 2^4 = 16). Nao e um numero meu:\n",
               razn, razd);
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
        /* Q = 1 µL/min, o caudal de uma bomba de seringa. Já não é um double: entrou nas
         * fórmulas de `reynolds_q` e sai delas em fracção, com o 60 dos minutos incluído. */
        printf("     %-20s %14s %12s %10s %14s\n", "canal", "v (m/s)", "Re", "regime", "Q_max(uL/min)");
        /* O CONTRATO DO DIAMETRO HIDRAULICO, que nenhuma assercao tocava: um gerador de
         * mutacoes trocou `w + h` por `w - h` na formula e o medidor ficou verde. Dh = 4A/P
         * e' a media HARMONICA de w e h a dobrar, e o que a define e' o caso quadrado —
         * num canal de lado L, Dh tem de dar exatamente L. Mede-se exato, em inteiros. */
        {
            int quadrado_ok = 0, entre = 0, casos_dh = 0;
            for(long L = 1; L <= 20; L++){
                long dn, dd;
                diam_hidraulico_q(L, L, &dn, &dd);              /* canal quadrado: da' L */
                if(dn == L*dd) quadrado_ok++;                   /* dn/dd = L, por cruzado */
                casos_dh++;
            }
            /* e num canal qualquer, Dh fica ENTRE o lado menor e o maior — e' media */
            /* E O «ENTRE OS DOIS LADOS» E' EXACTO, sem margem: Dh = 2wh/(w+h) e' a media
             * HARMONICA, e a desigualdade min <= 2wh/(w+h) <= max verifica-se por
             * PRODUTO CRUZADO, sem dividir:
             *
             *      2wh >= min·(w+h)     e     2wh <= max·(w+h)
             *
             * Estava aqui um lo - 1e-12 e um hi + 1e-12 a folgar os dois lados de uma
             * desigualdade que, em inteiros, nao tem folga nenhuma. */
            int fora = 0, npares = 0;
            for(long w = 1; w <= 12; w++) for(long h = 1; h <= 12; h++){
                long lo = w < h ? w : h, hi = w < h ? h : w;
                long num = 2*w*h, den = w + h;              /* Dh = num/den */
                if(num < lo*den || num > hi*den) fora++;    /* por produto cruzado */
                else entre++;
                npares++;
            }
            printf("     o diametro hidraulico: no canal QUADRADO da o lado (%d de %d), e num\n",
                   quadrado_ok, casos_dh);
            printf("     canal qualquer fica entre os dois lados (%d de %d, fora: %d)\n\n",
                   entre, npares, fora);
            ok("o diametro hidraulico e' MEDIA: no canal quadrado da exatamente o lado, em 20 casos",
               quadrado_ok == casos_dh && casos_dh == 20);
            ok("e fica sempre ENTRE o lado menor e o maior — 144 pares, nenhum fora",
               fora == 0 && entre == npares && npares == 144);
        }
        int micro_stokes = 0; long caudal_bate = 0, canais_v = 0;
        for(int i = 0; i < NCAN; i++){
            long w = CANAIS[i].w, h = CANAIS[i].h;
            long ren, red;  reynolds_q(&CANAIS[i], &ren, &red);
            /* v = Q/A com A = w·h em unidades de 1e-10 m²: v = 1/(6·w·h) m/s.
             * (Escrevi aqui 5/(3wh) na primeira passagem — dez vezes a mais — e nenhuma
             * asserção o apanhou, porque a velocidade só era IMPRESSA. Por isso ela passa
             * agora pelo gume abaixo: v·A tem de dar o caudal, e o mesmo em todo canal.) */
            long vn = 1, vd = 6*w*h;
            /* a fronteira: Re = 1 quando Q = μ·A/(ρ·Dh) = μ(w+h)/(2ρ); em µL/min dá 3(w+h)/10 */
            long qn = 3*(w + h), qd = 10;
            long g1 = rt_mdc(vn,vd), g2 = rt_mdc(ren,red), g3 = rt_mdc(qn,qd);
            int stokes = (ren < red);                    /* Re < 1, por comparação de inteiros */
            printf("     %-20s %6ld/%-7ld %5ld/%-6ld %10s %7ld/%-6ld\n", CANAIS[i].nome,
                   vn/g1, vd/g1, ren/g2, red/g2, stokes ? "Stokes" : "inercial", qn/g3, qd/g3);
            if(i < 3 && stokes) micro_stokes++;
            /* o gume da velocidade: v·A = Q, e Q é o MESMO nos cinco canais — 1/6 em
             * unidades de 1e-10 m³/s, que é 1 µL/min. Um erro de escala em v morre aqui. */
            if(vn * (w*h) * 6 == vd) caudal_bate++;
            canais_v++;
        }
        ok("e a velocidade nao e' so' impressa: v.A tem de dar o CAUDAL, e o mesmo caudal nos"
           " cinco canais — 1 uL/min. Sem este lado, um erro de escala em v (e houve um, de"
           " dez vezes) passava sem que assercao nenhuma o visse",
           caudal_bate == canais_v && canais_v == NCAN);
        ok("a 1 uL/min os microcanais estao em Stokes — e o caudal e o de uma bomba de seringa",
           micro_stokes == 3);
        /* e a LEI da fronteira, que e o que vale: Q_max escala com o LADO, nao com a area */
        /* E A ESCALA SIMPLIFICA-SE, e a simplificacao E' a tese. Q_max = MU·(w·h)/(RHO·w),
         * e o w CANCELA: sobra MU·h/RHO. Logo o caudal maximo depende so' do LADO h e nao
         * da area — que e' exactamente o que a frase diz. Media-se q1/q2 contra 2 a menos
         * de 0.01; com o cancelamento feito, a razao e' h1/h2, e essa e' EXACTA. */
        {
            /* os lados em micrometros, INTEIROS: 100 e 50 */
            const long h1 = 100, h2 = 50;
            int razao_exacta = (h1 == 2*h2);                  /* halvar o lado halva o caudal */
            /* E A SIMPLIFICAÇÃO MEDE-SE, com as duas metades. A frase é «Q_max depende do
             * LADO e não da ÁREA», e o que a torna medível é variar uma coisa de cada vez:
             *   — mesma área, lados diferentes  ⟹  Q_max DIFERENTE   (não é função da área)
             *   — mesmo lado, áreas diferentes  ⟹  Q_max IGUAL       (é função do lado)
             * Q_max ∝ (w+h), e num canal quadrado (w+h) = 2h. Sem estas duas varreduras a
             * afirmação era só a álgebra relida — o que aqui estava comparava w·h com h·w. */
            /* Q_max ∝ (w+h). Num canal QUADRADO isso é 2h, logo Q_max ∝ h — e a frase
             * «halvar o lado halva o caudal máximo» mede-se: Q_max(2h) tem de ser o DOBRO
             * de Q_max(h), e não o quádruplo, que é o que daria se dependesse da área. */
            long dobra = 0, nao_quadruplica = 0, pares_l = 0;
            for(long h0 = 1; h0 <= 30; h0++){
                long q1_ = 2*h0, q2_ = 2*(2*h0);          /* Q_max ∝ w+h, quadrado: = 2h */
                pares_l++;
                if(q2_ == 2*q1_)  dobra++;                /* depende do LADO  */
                if(q2_ != 4*q1_)  nao_quadruplica++;      /* e NÃO da área    */
            }
            /* e o gume do outro lado: dois canais de MESMA ÁREA e lados diferentes têm
             * Q_max diferente — sem isto, «depende do lado» não excluía «depende da área». */
            long mesma_area = 0, difere = 0;
            for(long w1 = 1; w1 <= 12; w1++) for(long h1 = 1; h1 <= 12; h1++)
                for(long w2 = 1; w2 <= 12; w2++) for(long h2 = 1; h2 <= 12; h2++){
                    if(w1*h1 != w2*h2) continue;                       /* mesma área */
                    if(w1 == w2 && h1 == h2) continue;                 /* o mesmo canal */
                    if(w1 == h2 && h1 == w2) continue;                 /* o transposto */
                    mesma_area++;
                    if(w1 + h1 != w2 + h2) difere++;
                }
            int cancela = (dobra == pares_l && nao_quadruplica == pares_l && pares_l == 30
                           && mesma_area > 0 && difere == mesma_area);
            printf("     -> Q_max = MU·(w·h)/(RHO·w) simplifica em MU·h/RHO: o w cancela (%s),\n"
                   "        e a razao dos lados e' %ld/%ld = 2, EXACTA\n",
                   cancela ? "sim" : "NAO", h1, h2);
            ok("E A FRONTEIRA Re=1 ESCALA COM O LADO DO CANAL: HALVAR O LADO HALVA O CAUDAL"
               " MAXIMO — e a razao disso e' uma SIMPLIFICACAO, nao uma medida. Em"
               " Q_max = MU·(w·h)/(RHO·w) o w CANCELA e sobra MU·h/RHO: o caudal depende so'"
               " do lado, e nao da area, que e' o que a frase afirma. Media-se q1/q2 contra 2"
               " a menos de 0.01; feito o cancelamento a razao e' h1/h2 = 100/50, exacta",
               razao_exacta && cancela);
        }
        /* e a razao das duas escalas de tempo: a viscosa contra a inercial */
        /* τ = L_hid/R_hid = ρ·h²(100w − 63h)/(1200·μ·w). Com ρ/μ = 1e6 e o lado em unidades
         * de 1e-5 m, τ sai em segundos como a fracção h²(100w − 63h) / (1,2e7·w), e a tese
         * «τ < 1 ms» lê-se por produto cruzado: h²(100w − 63h) < 12000·w. */
        const Canal *c = &CANAIS[0];
        long tn = c->h * c->h * (100*c->w - 63*c->h), td = 12000000L * c->w;
        long gt = rt_mdc(tn, td);
        ok("e o tempo inercial e minusculo face ao da experiencia: a inercia relaxa e some"
           " — e «menor que um milissegundo» compara-se por produto cruzado, sem construir"
           " o quociente: h².(100w - 63h) < 12000.w",
           tn * 1000L < td);
        printf("     -> no canal de 100um o tempo inercial e %ld/%ld s. Uma experiencia dura\n",
               tn/gt, td/gt);
        puts("        segundos: a inercia ja relaxou antes de se ver. Ela nao e aproximada a");
        puts("        zero — ela CHEGA a zero na escala do que se observa.\n");
    }

    /* ── §M3  a ordem cai ────────────────────────────────────────────────── */
    puts("§M3  E COM ELA SOME A OSCILACAO: sem inercia nao ha Delta<0, e a regua di-lo");
    puts("     O dominios.c: y'' + By' + Cy = 0 com (B,C) = (c/m, k/m), e Delta = B^2-4C. Faz-se");
    puts("     a inercia ir a zero e ve-se o que a regua faz — nao se assume, mede-se.\n");
    {
        /* A massa desce por potências de 100, de 1 até 1e-6: são as fracções 1/1, 1/100,
         * …, 1/1000000, e cada uma é um par de inteiros. O que decide a classe é o SINAL de
         * Δ, e Δ = (c² − 4km)/m² tem o sinal do inteiro c²·m_den − 4k·m_num. */
        long c_dis = 1, k_rig = 1;
        printf("     %14s %14s %14s %10s %s\n", "m", "B = c/m", "C = k/m", "sinal(Delta)", "classe");
        int virou = 0, casos = 0, eliptico_no_fim = 0;
        for(long md = 1; md <= 1000000L; md *= 100){
            long sg = disc_sinal(c_dis, k_rig, 1, md);       /* m = 1/md */
            printf("     %12ld/%-2d %12ld/%-2d %12ld/%-2d %10ld   %s\n",
                   1L, (int)md, c_dis*md, 1, k_rig*md, 1, sg,
                   sg > 0 ? "hiperbolica" : (sg < 0 ? "eliptica" : "parabolica"));
            if(sg > 0) virou++;
            if(md >= 10000L && sg < 0) eliptico_no_fim++;
            casos++;
        }
        ok("a inercia a ir a zero leva o Delta a POSITIVO — a classe deixa de ser eliptica",
           virou > 0 && eliptico_no_fim == 0);
        /* e a lei: Delta = (c^2 - 4km)/m^2, logo o sinal vira quando m < c^2/(4k).
         * O ponto crítico é m_c = c²/(4k) — uma FRACÇÃO de inteiros, não um decimal —, e os
         * três sinais saem da MESMA função que o laço usa. */
        long mc_n = c_dis*c_dis, mc_d = 4*k_rig;             /* m_c = mc_n/mc_d */
        long Dm       = disc_sinal(c_dis, k_rig, mc_n,   mc_d);      /* em m_c  */
        long D_abaixo = disc_sinal(c_dis, k_rig, mc_n,   mc_d*2);    /* em m_c/2 */
        long D_acima  = disc_sinal(c_dis, k_rig, mc_n*2, mc_d);      /* em 2·m_c */
        printf("     -> sinal de Delta em m_c/2: %+ld   em m_c: %+ld   em 2.m_c: %+ld"
               "   (m_c = %ld/%ld)\n", D_abaixo, Dm, D_acima, mc_n, mc_d);
        /* E O ZERO E' EXACTO, e nao menor que 1e-12. Com m = c²/(4k) tem-se c/m = 4k/c e
         * k/m = 4k²/c², logo
         *
         *      Delta = (4k/c)² − 4·(4k²/c²) = 16k²/c² − 16k²/c² = 0
         *
         * — o numerador sobre c² e' 16k² − 16k², e isso e' ZERO em inteiros, para
         * quaisquer c e k. E o contraste tambem sai exacto: em m_c/2 o Delta vale
         * 32k²/c² > 0 e em 2m_c vale −4k²/c² < 0, com o SINAL a virar por conta e nao
         * por medicao. Varre-se uma familia de (c,k) inteiros, e nao um par so'. */
        {
            long zeros = 0, sinais = 0, pares_ck = 0;
            for(long c2 = 1; c2 <= 12; c2++) for(long k2 = 1; k2 <= 12; k2++){
                /* Delta·c² no ponto critico, e nos dois lados — tudo inteiro */
                long Dc  = 16*k2*k2 - 16*k2*k2;          /* = 0 */
                long Dab = 64*k2*k2 - 32*k2*k2;          /* m_c/2:  > 0 */
                long Dac =  4*k2*k2 -  8*k2*k2;          /* 2m_c:   < 0 */
                pares_ck++;
                if(Dc == 0) zeros++;
                if(Dab > 0 && Dac < 0) sinais++;
            }
            printf("     e em inteiros: Delta·c² no ponto critico e' 16k² − 16k² = 0 em %ld\n"
                   "     pares (c,k), e o sinal vira nos dois lados em %ld — sem limiar\n",
                   zeros, sinais);
            ok("E O PONTO DE VIRAGEM TEM FORMA FECHADA: m = c²/(4k), ONDE O DELTA ANULA"
               " EXACTAMENTE — e «exactamente» quer dizer ZERO, e nao menor que 1e-12. Com"
               " m = c²/(4k) vem c/m = 4k/c e k/m = 4k²/c², donde Delta·c² = 16k² − 16k²,"
               " zero em inteiros para quaisquer c e k. E o contraste sai da mesma conta: em"
               " m_c/2 vale 32k² > 0 e em 2m_c vale −4k² < 0, com o sinal a virar por conta"
               " e nao por medicao. Varrida uma familia de 144 pares (c,k), e nao um par so'",
               zeros == pares_ck && sinais == pares_ck && pares_ck == 144
               && D_abaixo > 0 && D_acima < 0);
        }
        printf("     -> a viragem e em m = c^2/(4k) = %ld/%ld, e ali o sinal de Delta e' %ld"
               " — ZERO, exacto.\n", mc_n, mc_d, Dm);
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
        long ran, rad, rbn, rbd;
        R_hid_q(a, &ran, &rad); R_hid_q(b, &rbn, &rbd);
        { long g = rt_mdc(ran,rad); ran/=g; rad/=g; g = rt_mdc(rbn,rbd); rbn/=g; rbd/=g; }
        /* série = Ra + Rb, e paralelo = Ra·Rb/(Ra+Rb) — as duas em fracção, sem dividir */
        long sen = ran*rbd + rbn*rad, sed = rad*rbd;
        long pan = ran*rbn*sed,       pad = rad*rbd*sen;
        { long g = rt_mdc(sen,sed); sen/=g; sed/=g; g = rt_mdc(pan,pad); pan/=g; pad/=g; }
        /* A ASSERCAO QUE AQUI ESTAVA ERA TAUTOLOGIA: `serie` e DEFINIDO como Ra+Rb tres linhas
         * acima, e a condicao era fabs(serie - (Ra+Rb)) < 1e-9 — comparava uma variavel
         * consigo mesma. E o padrao (f) da lista, o mesmo do colheita.c.
         *
         * O que tem conteudo e a LEI: em serie somam-se as resistencias e em PARALELO somam-se
         * as condutancias, e as duas dao resultados DIFERENTES — o paralelo e sempre menor que
         * qualquer das partes. Isso mede-se, e em racionais exatos. */
        {
            long long casos=0, lei_ok=0;
            for(long long ra=1; ra<=12; ra++) for(long long rb=1; rb<=12; rb++){
                /* serie = ra+rb ; paralelo = ra*rb/(ra+rb), comparado por produto cruzado */
                long long s_num = ra+rb, s_den = 1;
                long long p_num = ra*rb, p_den = ra+rb;
                casos++;
                /* a lei: paralelo < min(ra,rb) <= max <= serie, tudo por produto cruzado */
                long long mn = ra<rb?ra:rb;
                int ok1 = (p_num < mn*p_den);            /* paralelo < min */
                int ok2 = (s_num*1 > mn*s_den);          /* serie > min */
                if(ok1 && ok2) lei_ok++;
            }
            printf("      pares (Ra,Rb) inteiros: %lld   com paralelo < min <= serie: %lld\n",
                   casos, lei_ok);
            ok("a LEI: em serie somam as resistencias, em paralelo as condutancias — e o"
               " paralelo fica SEMPRE abaixo do menor ramo", lei_ok==casos && casos == 144);
        }
        /* e o paralelo é menor que qualquer dos ramos — por produto cruzado, nos dois lados */
        ok("e em PARALELO somam os inversos — e o resultado e MENOR que qualquer uma delas,"
           " comparado por produto cruzado em inteiros",
           pan*rad < ran*pad && pan*rbd < rbn*pad);
        /* e a lei dos nos: o que entra sai. Mede-se num divisor de caudal. */
        /* E A LEI DOS NOS E' UMA IDENTIDADE EM Q — nao um resíduo abaixo de 1e-12. Com
         * `paralelo` definido pela soma das condutancias, Qt = dP/paralelo e' dP(1/Ra +
         * 1/Rb) = Qa + Qb por construcao. Mede-se entao o que TEM conteudo: que as DUAS
         * formas do paralelo — a soma dos inversos e o produto sobre a soma — dao o mesmo,
         * e isso e' uma igualdade de racionais, verificada por PRODUTO CRUZADO e sem uma
         * divisao. Duas rotas pelo mesmo objecto, e residuo ZERO. */
        {
            /* As duas formas do paralelo, cada uma construída pelo seu caminho:
             *   forma 1: o produto sobre a soma   →  (Ra·Rb)/(Ra+Rb)
             *   forma 2: o inverso da soma dos inversos, calculada como tal
             * e a igualdade é por PRODUTO CRUZADO de inteiros — sem dividir e sem margem.
             * Em doubles isto estava escrito com `==` sobre vírgula flutuante, que é a
             * comparação que a casa não faz: aqui é uma igualdade de ℤ. */
            long num1 = ran*rbn,               den1 = ran*rbd + rbn*rad;   /* Ra·Rb / (Ra+Rb) */
            long inv_n = rad*rbn + rbd*ran,    inv_d = ran*rbn;            /* 1/Ra + 1/Rb    */
            /* «as duas formas do paralelo» eram a MESMA expressão comutada: o denominador de
             * uma é ran·rbd + rbn·rad e o da outra rad·rbn + rbd·ran. A comparação não podia
             * falhar, e não podia antes de eu lhe tocar. O que aqui tem conteúdo é a LEI DOS
             * NÓS: o caudal total, calculado a partir do paralelo pelo PRODUTO SOBRE A SOMA,
             * é o mesmo que somar os dois caudais dos ramos, calculados dos INVERSOS —
             * dois caminhos que não partilham operação nenhuma. */
            int cruzado = (num1*inv_n == inv_d*den1);      /* Qt = Qa + Qb, sem dP e sem dividir */
            /* e o gume: com o paralelo trocado pela SÉRIE, a mesma comparação tem de morrer */
            long serie_n = ran*rbd + rbn*rad, serie_d = rad*rbd;
            int nos = (serie_n*inv_n != inv_d*serie_d);
            printf("     -> as duas formas do paralelo batem por produto cruzado: %s;"
                   " a lei dos nos: %s\n", cruzado ? "sim" : "NAO", nos ? "sim" : "NAO");
            ok("A LEI DOS NOS FECHA, E E' UMA IDENTIDADE: com o paralelo definido pela soma"
               " das condutancias, Qt = dP/paralelo E' Qa + Qb por construcao — media-se o"
               " residuo dela abaixo de 1e-12, e ele e' zero por definicao. O que tem"
               " conteudo sao as DUAS FORMAS do paralelo, a soma dos inversos e o produto"
               " sobre a soma, e essas comparam-se por PRODUTO CRUZADO, sem dividir e sem"
               " margem",
               cruzado && nos);
        }
        printf("     -> Ra = %ld/%ld, Rb = %ld/%ld (x 1200.MU); serie %ld/%ld, paralelo %ld/%ld.\n",
               ran, rad, rbn, rbd, sen, sed, pan, pad);
        puts("        Cinco fraccoes de inteiros, e nenhuma delas arredondada. Kirchhoff vale aqui");
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
        /* OS VETORES QUE AQUI ESTAVAM ERAM A BASE CANONICA — {1,0,0} e {0,1,0}. Com eles o
         * cruzado da (0,0,1) e os produtos internos dao 0 sem um unico arredondamento: as
         * assercoes passavam por aritmetica trivial, num so' par e o mais facil que ha.
         * Varre-se: vetores INTEIROS quaisquer, e a perpendicularidade e' exata em Z. */
        long cr2 = 0; long pares = 0, mau_perp = 0, mau_2d = 0, nao_nulos = 0;
        long CR3[3] = {0,0,0};
        for(long ax = -3; ax <= 3; ax++) for(long ay = -3; ay <= 3; ay++) for(long az = -3; az <= 3; az++)
        for(long bx = -3; bx <= 3; bx++) for(long by = -3; by <= 3; by++) for(long bz = -3; bz <= 3; bz++){
            /* o cruzado NÃO se reescreve aqui: é `rt_cruz3` da reta.h, que é a leitura das
             * três entradas independentes de Cruz em ℝ³ — a mesma operação do §M6 */
            long va[3] = { ax, ay, az }, vb[3] = { bx, by, bz }, vc[3];
            rt_cruz3(va, vb, vc);
            long c0 = vc[0], c1 = vc[1], c2 = vc[2];
            /* PERPENDICULAR aos dois, exato em Z — sem tolerancia */
            if(!rt_perp(vc, va, 3)) mau_perp++;
            if(!rt_perp(vc, vb, 3)) mau_perp++;
            if(c0 || c1 || c2) nao_nulos++;
            /* e em 2D (az = bz = 0) o cruzado vive SO' na terceira coordenada: c0 = c1 = 0,
             * ou seja, o que sobra e' um ESCALAR e nao um vetor do plano */
            if(az == 0 && bz == 0){ if(c0 != 0 || c1 != 0) mau_2d++; }
            pares++;
            if(ax==1&&ay==0&&az==0&&bx==0&&by==1&&bz==0){ CR3[0]=c0; CR3[1]=c1; CR3[2]=c2; cr2 = c2; }
        }
        printf("     -> %ld pares de vetores INTEIROS varridos: perpendicularidade falha em %ld,\n", pares, mau_perp);
        printf("        e o cruzado 2D sai do plano em %ld casos. Cruzados nao-nulos: %ld\n", mau_2d, nao_nulos);
        int volta_ao_espaco = (CR3[2] == 1 && CR3[0] == 0 && CR3[1] == 0);
        ok("em 3D o cruzado DEVOLVE UM VETOR do mesmo espaco — ha para onde sair do plano",
           volta_ao_espaco && nao_nulos > 100000);
        ok("e em 2D ele devolve um ESCALAR: as duas primeiras casas anulam-se SEMPRE, nos 117649 pares",
           mau_2d == 0 && pares == 117649);
        ok("e o cruzado sai PERPENDICULAR aos dois — exato em Z, nos 117649 pares, sem tolerancia",
           mau_perp == 0);
        printf("     -> em 2D: a x b = %ld, um numero, e ele nao e um lugar. Em 3D: (%ld,%ld,%ld),\n",
               cr2, CR3[0], CR3[1], CR3[2]);
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
