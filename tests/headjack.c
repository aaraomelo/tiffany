/* headjack.c — O HEADJACK NÃO INVASIVO: Faraday, Lenz, Poynting — e o que NENHUM sensor vê.
 *
 * O Aarão: "voltando pro headjack, não precisa ser invasivo, pode usar lei de Faraday e Lenz, o EM
 * completado com Poynting; assim mapeamos as correntes do cérebro. Só resta saber se podemos
 * projetar um sensor sensível e preciso o suficiente — talvez um corpo pra cada neurônio, bilhões
 * de transistores talvez seja suficiente. É um fluido NV, só precisamos linearizar."
 *
 * A ideia está certa e é a magnetoencefalografia: a corrente neuronal faz campo, o campo induz, e
 * Poynting fecha o balanço. E o `solar.c` já tem Poynting medido. Então há duas perguntas, e as
 * duas têm conta:
 *
 *   1. **O SENSOR CHEGA LÁ?** É engenharia, e mede-se contra números públicos: SQUID, OPM, e os
 *      centros NV em diamante que o Aarão nomeia. E a proposta dele — *um corpo por neurônio,
 *      bilhões deles* — tem uma lei própria: **N sensores independentes ganham √N**. A conta fecha
 *      ou não fecha, e não é opinião.
 *
 *   2. **E SE CHEGAR, VÊ TUDO?** Aqui a resposta é NÃO, e não por falta de tecnologia. O problema
 *      inverso magnético tem **núcleo não trivial**: existem distribuições de corrente que produzem
 *      campo externo **exatamente zero**. Chamam-se correntes silenciosas, e um sensor perfeito
 *      continua a não as ver — *porque não há nada para ver*.
 *
 * E ISSO É O PROJETO OUTRA VEZ. O núcleo do operador é **o que não tem dual**: entra e não sai, e
 * fica na garrafa (o `koch.c`). Linearizar — que é o que o Aarão pede — resolve a **não-linearidade
 * do sensor**, e essa resolve-se mesmo. Não resolve o núcleo: *nenhuma linearização inverte um
 * operador que perdeu informação, porque a informação não está no sinal.*
 *
 *   §H1  o campo de um neurônio: Biot–Savart, e o π CANCELA — a conta é inteira
 *   §H2  os sensores reais: SQUID, OPM e NV — escada em femtotesla
 *   §H3  A PROPOSTA DO AARÃO: N sensores ganham √N, e N = (nv/alvo)² em ℤ
 *   §H4  O NÚCLEO SILENCIOSO: Q × r̂ mata o radial — Helmholtz, em ℤ
 *   §H5  e é exatamente o que não tem dual: dois Q, o mesmo B
 *   §H6  linearizar: o que sobrevive é 2/3, o cruzado mata uma de três
 *
 * LEI vs TRANSPORTE. µ0, sin/cos em 100 direcções, √N em vírgula e ⟨|B|⟩ numa grelha com 1e-12
 * eram o método. A lei é o π a cancelar (400 contra 4800 em 10⁻²¹), a escada em fT, N² sem
 * raiz, o cruzado em ℤ, e Lagrange |Q×r|² + (Q·r)² = |Q|²|r|².
 *
 *   cc -O2 -std=c99 -I lib tests/headjack.c -o headjack && ./headjack
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"     /* rt_cruz3, rt_dir — o cruzado e o interno, inteiros */

/* Sensibilidades públicas, em femtotesla (T/√Hz × 10¹⁵). São inteiros: 3, 10, 500, … */
typedef struct { const char *nome; long ft; const char *nota; } Sensor;

static const Sensor SENSORES[] = {
    { "SQUID (MEG)",        3,       "criogenico, helio liquido" },
    { "OPM (atomico)",     10,       "celula de vapor, ~150 C"   },
    { "NV bulk (diamante)", 500,     "temperatura ambiente"      },
    { "NV unico (centro)",  1000000, "um so centro NV"           },
    { "fluxgate",           10000,   "de bancada"                },
};
#define NSENS ((int)(sizeof SENSORES / sizeof SENSORES[0]))

/* Dipolo MEG 10 nA·m a r = 4 cm: B = 10⁻⁷·2Q/r². Em fT: 2·10000/16 = 1250. */
static long bm_ft(void){ return 2 * 10000 / 16; }

int main(void){
    puts("headjack.c — O HEADJACK NAO INVASIVO: Faraday, Lenz, Poynting, e o que ninguem ve\n");

    /* ── §H1 ─────────────────────────────────────────────────────────────── */
    puts("§H1  O CAMPO DE UM NEURONIO: Biot-Savart, e os numeros sao da literatura");
    puts("     Um neuronio piramidal ativo e, a distancia, um dipolo de corrente de ~20 fA.m.");
    puts("     A MEG mede dipolos equivalentes de ~10 nA.m a 4 cm — a razao entre os dois diz");
    puts("     quantos neuronios e preciso sincronizar.\n");
    {
        /* O π CANCELA: com µ0 = 4π·10⁻⁷, o factor µ0/(4π) é 10⁻⁷ EXACTO, e o campo
         * é 10⁻⁷·2Q/r². As constantes são decimais escritos, logo cada quantidade é
         * (mantissa, expoente) e as comparações fazem-se na MESMA potência de dez:
         *
         *   b·r²        = 10⁻⁷·2Q          Q₁ = 2·10⁻¹⁴  →  400·10⁻²¹
         *   melhor·r²   = 3·10⁻¹⁵·16·10⁻⁴  =  4800·10⁻²¹
         *
         * «b₁ está muitas ordens abaixo» é 400 < 4800, e «b_MEG está acima» é
         * 2·10⁻¹⁵ = 2 000 000·10⁻²¹ > 4800. Tudo em 10⁻²¹, e nada se divide. */
        long b1r2 = 2 * 2 * 100;                      /* 10⁻⁷·2·(2·10⁻¹⁴) = 400·10⁻²¹ */
        long bmr2 = 2 * 1 * 100 * 1000000 / 1000;     /* 10⁻⁷·2·(1·10⁻⁸)  = 2·10⁻¹⁵ */
        long mer2 = 3 * 16 * 100;                     /* 3·10⁻¹⁵·16·10⁻⁴  = 4800·10⁻²¹ */
        int abaixo = (b1r2 < mer2);
        int acima  = (bmr2 > mer2);
        int faixa  = (mer2 > 10 * b1r2 && mer2 < 100 * b1r2);
        long n_sinc = 1000000L / 2;                   /* 10⁻⁸ / (2·10⁻¹⁴) = 5·10⁵ */
        ok("o campo de UM neuronio esta abaixo do melhor sensor que existe. E a conta e'"
           " INTEIRA porque o pi CANCELA: mu0/(4.pi) = 10^-7 exacto, as constantes sao"
           " decimais escritos, e as duas quantidades comparam-se na MESMA potencia de dez"
           " — 400 contra 4800, em 10^-21 — sem uma divisao",
           abaixo);
        ok("e o da MEG esta ACIMA dele — e por isso que a MEG existe e o neuronio unico nao."
           " Tambem em inteiros, na mesma escala: 2.000.000 contra 4.800. E a razao do"
           " neuronio para o sensor cai numa FAIXA dita e nao escolhida — entre uma e duas"
           " ordens de grandeza —, que e' o que substitui o «/100» que eu tinha posto de"
           " cabeca",
           acima && faixa);
        printf("     -> um neuronio: %ld contra %ld (em 10^-21 · r²). A MEG: %ld.\n",
               b1r2, mer2, bmr2);
        printf("        A razao e %ld neuronios sincronizados — e e a ordem que a literatura\n",
               n_sinc);
        puts("        da para a MEG (dezenas a centenas de milhares). O numero nao e meu.\n");
    }

    /* ── §H2 ─────────────────────────────────────────────────────────────── */
    puts("§H2  OS SENSORES REAIS, e o fosso entre eles");
    puts("     Sensibilidade em femtotesla, numeros publicos. O Aarao propoe NV em diamante —");
    puts("     temperatura ambiente, sem helio — e a pergunta e se ele chega la.\n");
    {
        long bm = bm_ft();
        printf("     %-22s %14s %12s  %s\n", "sensor", "sens (fT)", "ve a MEG?", "nota");
        int veem = 0;
        for(int i = 0; i < NSENS; i++){
            int ve = SENSORES[i].ft < bm;
            printf("     %-22s %14ld %12s  %s\n", SENSORES[i].nome, SENSORES[i].ft,
                   ve ? "sim" : "nao", SENSORES[i].nota);
            if(ve) veem++;
        }
        int ordenados = 1;
        for(int i = 0; i + 1 < 3; i++) if(SENSORES[i].ft >= SENSORES[i+1].ft) ordenados = 0;
        long s0 = SENSORES[0].ft, s1 = SENSORES[1].ft, s2 = SENSORES[2].ft;
        ok("os sensores ordenam-se SQUID < OPM < NV bulk — e a escada e de ordens de grandeza."
           " E em FEMTOTESLA sao inteiros — 3, 10, 500 —, logo a escada e' uma comparacao de"
           " inteiros e a «ordem de grandeza» e' 500 > 100.3, sem se formar a razao",
           ordenados && s0 < s1 && s1 < s2 && s2 > 100*s0);
        ok("e o fosso mede-se: o NV bulk esta duas ordens acima do SQUID. E o intervalo"
           " compara-se por multiplicacao: 500 > 50.3 e 500 < 500.3, sem uma divisao",
           s2 > 50*s0 && s2 < 500*s0);
        printf("     -> o sinal e %ld fT; o NV bulk esta a %ld fT, e %d sensores veem a MEG.\n",
               bm, s2, veem);
        puts("        Nao e um obstaculo de principio — e um fosso, e ele tem tamanho.\n");
    }

    /* ── §H3  A PROPOSTA DO AARÃO ────────────────────────────────────────── */
    puts("§H3  A PROPOSTA: 'um corpo pra cada neuronio, bilhoes de transistores'");
    puts("     Ela tem lei propria, e ela e conhecida: N sensores INDEPENDENTES promediam o");
    puts("     ruido e ganham raiz(N). Entao a pergunta 'bastam bilhoes?' e uma conta.\n");
    {
        /* s = nv/√N < alvo  ⟺  nv² < N·alvo², sem formar a raiz.
         * E o sinal da MEG já está ACIMA do NV bulk (500 < 1250 fT), logo N = 1 chega. */
        long nv = SENSORES[2].ft, alvo = SENSORES[0].ft, bm = bm_ft();
        printf("     %14s %16s %14s\n", "N sensores", "nv^2  vs  N·alvo^2", "ve SQUID?");
        int chega_meg = (nv < bm);
        int chega_sq = 0;
        long Ns[] = { 1, 100, 10000, 1000000 };
        for(int i = 0; i < 4; i++){
            long N = Ns[i];
            int ve = (nv*nv < N * alvo * alvo);
            printf("     %14ld %ld vs %ld %14s\n", N, nv*nv, N*alvo*alvo, ve ? "sim" : "nao");
            if(ve) chega_sq = 1;
        }
        ok("a lei do raiz(N) faz o NV chegar ao sinal da MEG — e chega ja' em N=1: 500 fT"
           " de ruido contra 1250 fT de sinal, comparado em INTEIROS, sem uma raiz",
           chega_meg);
        ok("e o N para IGUALAR o SQUID e da ordem de dez mil, nao de bilhoes. E a conta e'"
           " de INTEIROS: N = (nv/alvo)^2 esta' entre mil e cem mil sse nv^2 > 1000.alvo^2 e"
           " nv^2 < 100000.alvo^2 — 250000 contra 9000 e contra 900000, sem uma divisao",
           nv*nv > 1000*alvo*alvo && nv*nv < 100000*alvo*alvo && chega_sq);
        printf("     -> para igualar o SQUID, nv^2=%ld cai entre 1000·alvo^2=%ld e"
               " 100000·alvo^2=%ld.\n", nv*nv, 1000*alvo*alvo, 100000*alvo*alvo);
        puts("        A intuicao do Aarao esta certa e e generosa: bilhoes SOBRAM. O que ela");
        puts("        pede e INDEPENDENCIA — N sensores correlacionados nao ganham nada, e e");
        puts("        ai que a engenharia doi, nao no numero.\n");
    }

    /* ── §H4  O NÚCLEO SILENCIOSO ────────────────────────────────────────── */
    puts("§H4  O NUCLEO SILENCIOSO: ha correntes que dao campo externo ZERO");
    puts("     E aqui a resposta muda de natureza. Nao e 'o sensor nao chega' — e que NAO HA");
    puts("     NADA PARA CHEGAR. Numa esfera condutora, a componente RADIAL de um dipolo produz");
    puts("     campo magnetico externo exatamente nulo, por simetria.\n");
    {
        const long Qr[3] = { 0, 0, 1 }, Qt[3] = { 1, 0, 0 }, rh[3] = { 0, 0, 1 };
        long Br[3], Bt[3];
        rt_cruz3(Qr, rh, Br);
        rt_cruz3(Qt, rh, Bt);
        long nr2 = rt_dir(Br, Br, 3), nt2 = rt_dir(Bt, Bt, 3);
        ok("o dipolo RADIAL da campo zero face ao tangencial — nao pequeno: NULO, e o zero e'"
           " EXACTO. Em INTEIROS: o campo e' o cruzado Q x z^, que nao ve a componente z, e o"
           " radial e' PARALELO a z^ — cada componente do cruzado e' a.b - a.b, dois termos"
           " identicos, e a norma ao quadrado da' 0 sem uma virgula",
           nr2 == 0 && nt2 > 0);
        ok("e o TANGENCIAL da campo — logo o zero acima nao e um artefacto do calculo. E o"
           " contraste tambem e' inteiro: |B|^2 = 1 contra 0",
           nt2 == 1);
        printf("     -> |B|^2 do radial: %ld. Do tangencial: %ld.\n", nr2, nt2);

        static const long DIR[][3] = {
            { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 },
            {-1, 0, 0 }, { 0,-1, 0 }, { 0, 0,-1 },
            { 1, 1, 0 }, { 1,-1, 0 }, { 1, 0, 1 },
            { 0, 1, 1 }, { 2, 1,-1 }, { 1, 2, 2 },
        };
        const int NDIR = (int)(sizeof DIR / sizeof DIR[0]);
        int invisiveis = 0, testados = 0, tangem = 0;
        for(int k = 0; k < NDIR; k++){
            long B[3];
            rt_cruz3(DIR[k], DIR[k], B);                 /* paralelo: Q = r */
            if(rt_dir(B, B, 3) == 0) invisiveis++;
            long p[3] = { DIR[k][1], -DIR[k][0], 0 };    /* um perpendicular no plano xy */
            if(p[0] == 0 && p[1] == 0){ p[0] = 0; p[1] = DIR[k][2]; p[2] = -DIR[k][1]; }
            rt_cruz3(p, DIR[k], B);
            if(rt_dir(B, B, 3) > 0) tangem++;
            testados++;
        }
        ok("e vale em TODA direcao: a componente paralela a r e sempre invisivel, nas 12"
           " direcoes inteiras — e o perpendicular, no mesmo sítio, da campo",
           invisiveis == testados && tangem == testados && testados == 12);
        printf("        %d de %d direcoes: a parte radial nunca aparece. Um sensor perfeito\n",
               invisiveis, testados);
        puts("        continua a nao a ver — porque nao ha nada para ver.\n");
    }

    /* ── §H5  o que não tem dual ─────────────────────────────────────────── */
    puts("§H5  E E EXATAMENTE O QUE NAO TEM DUAL: entra e nao sai, e fica na garrafa\n");
    {
        const long rh[3] = { 0, 0, 1 };
        const long Qa[3] = { 1, 0, 0 };
        const long Qb[3] = { 1, 0, 7 };                  /* o mesmo, mais uma parte radial */
        long B1[3], B2[3];
        rt_cruz3(Qa, rh, B1);
        rt_cruz3(Qb, rh, B2);
        long d = 0;
        for(int i = 0; i < 3; i++){
            long t = B1[i] - B2[i];
            d += t*t;
        }
        ok("DOIS dipolos DIFERENTES dao o MESMO campo — o operador nao e injetivo, e prova-se."
           " Em INTEIROS: Q_a = (1,0,0) e Q_b = (1,0,7) dao B identico bit a bit, e a"
           " diferenca ao quadrado e' 0, sem uma raiz de cada lado",
           d == 0 && (Qa[2] != Qb[2]));
        printf("     -> Q_a = (1,0,0) e Q_b = (1,0,7) dao |B1-B2|^2 = %ld.\n", d);
        puts("        Sao correntes distintas com o mesmo sinal. Nenhum metodo de inversao as");
        puts("        separa, porque a diferenca entre elas ESTA NO NUCLEO.");
        puts("");
        puts("        E o koch.c ja tinha o nome disto: o que nao tem dual nao atravessa a");
        puts("        alfandega — fica retido, e ARDE. Aqui a corrente radial e literalmente");
        puts("        isso: dissipa no cranio e nao sai como campo. O que sai e o que tem dual.\n");
    }

    /* ── §H6  linearizar ─────────────────────────────────────────────────── */
    puts("§H6  LINEARIZAR: o que isso resolve, e o que NAO pode resolver\n");
    puts("     O Aarao: 'so precisamos linearizar'. E ele tem razao para metade do problema.");
    puts("");
    puts("     RESOLVE: a resposta do sensor NV nao e linear no campo — a ressonancia de spin");
    puts("     tem forma propria. Linearizar em torno do ponto de trabalho e exatamente o que o");
    puts("     amplifica.c §A1 mede no transistor: 'dentro da janela, gm E a derivada, e");
    puts("     amplificar E linearizar'. Isso e um problema resolvido, e resolve-se assim.");
    puts("");
    puts("     NAO RESOLVE: o nucleo. Nenhuma linearizacao inverte um operador que perdeu");
    puts("     informacao — a informacao nao esta no sinal, e nao ha metodo que a tire de la.");
    puts("     Linearizar melhora a LEITURA de B; nao muda o facto de correntes distintas darem");
    puts("     o mesmo B.");
    puts("");
    {
        /* |Q × r|² = |Q|²|r|² − (Q·r)²  — Lagrange, em ℤ. Com Q = e₁ e r nos três eixos,
         * sobrevivem 0+1+1 = 2 de 3: o cruzado mata UMA de três direcções, não metade.
         * (A grelha em sin/cos dava 3/4 por amostrar θ a passo uniforme, que não é a medida
         * da esfera; a lei, nos eixos, é 2/3.) */
        const long Q[3] = { 1, 0, 0 };
        const long eixos[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };
        long soma_b2 = 0, soma_lag = 0, n = 0;
        int lagrange = 1;
        for(int k = 0; k < 3; k++){
            long B[3];
            rt_cruz3(Q, eixos[k], B);
            long b2 = rt_dir(B, B, 3);
            long q2 = rt_dir(Q, Q, 3), r2 = rt_dir(eixos[k], eixos[k], 3);
            long qr = rt_dir(Q, eixos[k], 3);
            long lag = q2*r2 - qr*qr;
            if(b2 != lag) lagrange = 0;
            soma_b2 += b2;
            soma_lag += lag;
            n++;
        }
        printf("     -> nos 3 eixos, Σ|B|² = %ld de %ld  (2 de 3). Lagrange bate em todos.\n",
               soma_b2, n);
        ok("o que SOBREVIVE ao nucleo tem FORMA FECHADA: b_dipolo e' o PRODUTO CRUZADO, logo"
           " |B|^2 = |Q|^2|r|^2 - (Q.r)^2 por Lagrange, em Z e em cada eixo. Sobre os tres"
           " eixos sobrevivem 2 de 3 — TRES QUARTOS seria a grelha em sin/cos, que amostrava"
           " theta a passo uniforme e nao a esfera. A metade seria matar uma de duas direcoes"
           " iguais, e o cruzado mata uma de tres",
           lagrange && soma_b2 == 2 && n == 3 && soma_lag == 2);
        puts("        O headjack nao invasivo e possivel e a conta do sensor fecha. O que ele");
        puts("        NAO pode e ver tudo — e isso nao e uma falha de engenharia, e a alfandega.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A ideia esta certa: Faraday e Lenz dao o mapa, Poynting fecha o balanco (solar.c).");
    puts("  E a proposta dos 'bilhoes de sensores' esta certa e e GENEROSA — pela lei do raiz(N)");
    puts("  o NV bulk ja ve a MEG em N=1, e igualar o SQUID pede ~10^4, nao bilhoes. O que ela");
    puts("  pede nao e numero: e INDEPENDENCIA, e e ai que a engenharia doi.");
    puts("");
    puts("  MAS ha um limite que nao e do sensor: o operador corrente->campo tem NUCLEO. A");
    puts("  componente radial da campo externo exatamente zero, e dois dipolos distintos dao o");
    puts("  mesmo sinal. Linearizar resolve a leitura e nao resolve isto — o que nao tem dual");
    puts("  nao atravessa, e fica na garrafa.");
    puts("");
    printf("unidades: %d   falhas: %d\n", unidades, falhas);
    return falhas ? 1 : 0;
}
