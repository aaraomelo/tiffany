/* tests/medidor_iota3.c — ORÁCULO INDEPENDENTE para ι₃ : X₃ ↪ X₄
 *
 *    Estado: alvo implementado (lib/iota3.h). Este medidor está executado e
 *    validado contra o que CONTRATO_X4.md exige em §4, §6, §7 e §10: cobertura
 *    exaustiva 16777216/16777216, quatro canais separados, oráculo O₃
 *    independente, os quatro controlos negativos a detectar o canal que cada um
 *    viola, e os dois casos-limite explícitos. NÃO está executada a bateria de
 *    dez mutações de §9 — nenhuma taxa de detecção é afirmada, e o VEREDITO
 *    impresso é o de §10 (contadores), não uma afirmação de capacidade de
 *    auditoria.
 *
 *    Este ficheiro mede iota3 sem a chamar para construir o esperado.
 *
 * Porquê existe (CONTRATO_X4.md): o contrato define a seta iota₃, o oráculo
 * O₃(b₀,b₁,b₂)=b₀+256·b₁+65536·b₂, e quatro controlos negativos. Este
 * instrumento mede sem depender da implementação para construir o esperado:
 *   - cobertura positiva exaustiva, elemento a elemento de X₃;
 *   - quatro canais lidos SEPARADAMENTE (π₀, π₁, π₂, π₃), todos no veredicto;
 *   - oráculo O₃ calculado dos argumentos de entrada, nunca da saída;
 *   - quatro controlos negativos, cada um violando exactamente um canal;
 *   - dois casos-limite explícitos, (0,0,0) e (255,255,255).
 *
 * ── INDEPENDÊNCIA (CONTRATO_X4.md §6, §7) ──────────────────────────────
 *   O tipo Quad e as funções de construção (S1, C4, O3, construir_C0/C1/C2/N,
 *   e as projeções pi0/pi1/pi2/pi3) são PRÓPRIOS do medidor. Nenhuma delas
 *   chama iota3, nem lê um Iota3Out, nem inclui lógica do alvo. A única linha de
 *   todo o ficheiro que executa uma chamada a iota3 é a secção §5, dentro de
 *   main, e dela saem quatro leituras de byte independentes (out.b0, out.b1,
 *   out.b2, out.b3) copiadas para um Quad local antes de qualquer verificação.
 *   A verificação — que é partilhada entre alvo e controlos — nunca vê a saída de
 *   iota3 a menos de quatro bytes, e nunca a vê por valor empacotado.
 *
 *   Os casos-limite de §10 são verificados SOBRE o alvo, e por isso vivem
 *   dentro do mesmo e único ponto de chamada: abrem-se no momento em que o
 *   argumento é o ponto, e comparam o alvo com um literal. Uma segunda secção
 *   de chamadas violaria a regra da chamada única. O esperado vem de literais
 *   escritos aqui, e não do oráculo, para que uma divergência entre alvo e
 *   instrumento não se possa anular a si mesma.
 *
 * Representação (CONTRATO_X4.md §3.3, indexação 0-based):
 *   π₀(y)=b₀   π₁(y)=b₁   π₂(y)=b₂   π₃(y)=b₃
 *   C₄(b₀,b₁,b₂,b₃) = b₀ + 256·b₁ + 65536·b₂ + 16777216·b₃
 *                      (0 ≤ C₄ ≤ 4294967295 = 2³²−1)
 *   O₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂      (0 ≤ O₃ ≤ 16777215)
 *
 * ── PORQUÊ uint64_t, E ONDE ESTÁ O PERIGO (CONTRATO_X4.md §6) ───────────
 *   C₄ tem máximo 2³²−1, que é o valor MÁXIMO de uint32_t e NÃO cabe em int
 *   com sinal (INT32_MAX = 2147483647). O instrumento acumula em uint64_t.
 *
 *   O perigo real não é o tipo declarado, é a PROMOÇÃO INTEIRA: escrevendo
 *   `y.q3 * 16777216` com q3 um uint8_t, a promoção usual do C leva o produto a
 *   int × int, e 255 * 16777216 = 4278190080 é overflow com sinal —
 *   comportamento indefinido, SEM qualquer aviso do compilador, mesmo com o
 *   tipo de retorno uint64_t. Por isso cada produto leva o cast no PRIMEIRO
 *   operando e os multiplicadores levam UINT64_C. Isto é evidência por leitura
 *   do código, não por medição: um uint32_t também caberia em C₄, de modo que
 *   o veredicto verde não prova o tipo. Auditar a promoção é o que prova.
 *
 *   Nenhum desvio é calculado por subtracção. As mutações de §9 produzem
 *   desvios NEGATIVOS (b₀: 255→0, b₁: 255→0), e subtrair em não-signed
 *   transbordaria. O instrumento compara os dois valores; nunca os subtrai.
 *
 *   A asserção de limite que foi considerada e descartada está registada noutro
 *   sítio: o controlo N com b₂=255 e (b₀,b₁)=(255,255) atinge C₄ = 2³²−1
 *   exactamente, e o autoteste §0 fixa C₄(255,255,255,255) = 4294967295. São
 *   os casts, e não uma guarda de runtime, que sustentam a garantia de tipo.
 *
 * ── PORQUÊ %llu E NÃO PRIu64 ───────────────────────────────────────────
 *   O <inttypes.h> deste toolchain define PRIu64 como "llu", SEM o '%'. Usá-la
 *   compila e não dá erro, mas o número não aparece na saída e o -Wformat
 *   acusa argumentos a mais. Imprimir com "%llu" e conversão explícita para
 *   unsigned long long é portátil em qualquer compilador C99 e não depende
 *   dessa macro. Os contadores continuam a ser uint64_t na aritmética.
 *
 * Não altera: campos.tex, espaco.tex, CONTRATO_X3.md, CONTRATO_X4.md,
 *             lib/iota3.h, lib/iota2.h, lib/iota1.h, CONTRATO_X2.md,
 *             medidor/ESPECIFICACAO.md, papers, headers existentes.
 *
 *   Nenhuma das dez mutações de §9 está implementada aqui, nem em forma
 *   comentada. A bateria corre por fora, sobre cópias temporárias do alvo.
 * ────────────────────────────────────────────────────────────────────────
 * Compilar:
 *   cc -O2 -std=c99 -Wall -Wextra -Ilib -o medidor_iota3 tests/medidor_iota3.c
 * Executar:
 *   ./medidor_iota3
 *   Sai com 0 se VEREDITO=FECHADO, 1 caso contrário.
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "iota3.h"

/* Formato de impressão de uint64_t, portátil. Ver a nota no cabeçalho. */
#define U64 "%llu"
#define U(v) ((unsigned long long)(v))

/* Grandezas de §4 e §7.7, declaradas uma vez para que cobertura e controlos
 * nunca sejam confundidos com números escritos à mão. */
#define N_COB  UINT64_C(16777216)   /* 256³  = |X₃|            */
#define N_CTRL UINT64_C(16711680)   /* 255·256², por família    */

/* ─── tipo do MEDIDOR: independente de Iota3Out ───
 * Os controlos são construídos a partir deste tipo, sem qualquer dependência
 * do header do alvo. q0..q3 correspondem a π₀..π₃ (0-based, §3.3). */
typedef struct { uint8_t q0, q1, q2, q3; } Quad;

/* ─── projeções π₀..π₃ do MEDIDOR: leem UM byte cada ───
 * Cada função lê UMA coordenada e nada mais — obrigação de canal, §3.3.
 * A separação tem de ser real: é o que permite atribuir uma falha a um
 * canal nomeado, e é o que as mutações M5 (dois canais) e M6 (quatro canais)
 * de §9 põem à prova. */
static uint8_t pi0(Quad t){ return t.q0; }
static uint8_t pi1(Quad t){ return t.q1; }
static uint8_t pi2(Quad t){ return t.q2; }
static uint8_t pi3(Quad t){ return t.q3; }

/* ─── codificação C₄ do MEDIDOR, forma independente ───
 * C₄(b₀,b₁,b₂,b₃) = b₀ + 256·b₁ + 65536·b₂ + 16777216·b₃.
 * Lê a saída OBSERVADA. Não usa nenhuma função do alvo: o alvo não calcula
 * código nenhum, preenche quatro bytes. */
static uint64_t C4(Quad t){
    return (uint64_t)t.q0
         + (uint64_t)t.q1 * UINT64_C(256)
         + (uint64_t)t.q2 * UINT64_C(65536)
         + (uint64_t)t.q3 * UINT64_C(16777216);
}

/* ─── ORÁCULO O₃, INDEPENDENTE (§6) ───
 * O₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂ = C₄(b₀,b₁,b₂,0) = C₃(b₀,b₁,b₂).
 * Calculado APENAS dos argumentos de entrada e do zero da quarta posição.
 * Não chama iota3; não lê a saída de iota3; sobrevive a qualquer alteração do
 * alvo. A forma é deliberadamente diferente de C₄: C₄ tem quatro termos, O₃
 * tem três, e o quarto peso não aparece em lado nenhum do oráculo. */
static uint64_t O3(uint8_t b0, uint8_t b1, uint8_t b2){
    return (uint64_t)b0
         + (uint64_t)b1 * UINT64_C(256)
         + (uint64_t)b2 * UINT64_C(65536);
}

/* ─── sucessor de byte S₁ (campos.tex:433): S₁(b) = (b+1) mod 256 ───
 * É o S₁ do paper, e é a granularidade certa para corromper UMA coordenada.
 * O sucessor de três bytes S₃ de campos.tex:433 NÃO é usado: nenhum dos
 * quatro controlos de §7 corrompe as três coordenadas de uma vez, e uma
 * função não usada geraria um aviso com -Wall -Wextra. */
static uint8_t S1(uint8_t b){ return (uint8_t)(b + 1u); }

/* ─── CONSTRUÇÃO DOS CONTROLOS, INDEPENDENTE (§7) ───
 * Cada construtor devolve diretamente um Quad com coordenadas explícitas.
 * Nenhum chama iota3; nenhum parte de uma saída de iota3 corrompida. Cada um
 * viola exactamente UM canal, e exactamente um: os outros três ficam correctos,
 * para que a falha seja atribuível. */

/* C₀ — primeira coordenada errada: (S₁(b₀),b₁,b₂,0), b₀≠255  (§7.1) */
static Quad construir_C0(uint8_t b0, uint8_t b1, uint8_t b2){
    Quad c; c.q0 = S1(b0); c.q1 = b1; c.q2 = b2; c.q3 = (uint8_t)0; return c;
}
/* C₁ — segunda coordenada errada: (b₀,S₁(b₁),b₂,0), b₁≠255  (§7.2) */
static Quad construir_C1(uint8_t b0, uint8_t b1, uint8_t b2){
    Quad c; c.q0 = b0; c.q1 = S1(b1); c.q2 = b2; c.q3 = (uint8_t)0; return c;
}
/* C₂ — terceira coordenada errada: (b₀,b₁,S₁(b₂),0), b₂≠255  (§7.3) */
static Quad construir_C2(uint8_t b0, uint8_t b1, uint8_t b2){
    Quad c; c.q0 = b0; c.q1 = b1; c.q2 = S1(b2); c.q3 = (uint8_t)0; return c;
}
/* N — quarta coordenada errada: (b₀,b₁,b₂,b₂), b₂≠0  (§7.4)
 * A coordenada NOVA recebe o valor da terceira em vez de 0. */
static Quad construir_N(uint8_t b0, uint8_t b1, uint8_t b2){
    Quad c; c.q0 = b0; c.q1 = b1; c.q2 = b2; c.q3 = b2; return c;
}

/* ─── contadores, separados por canal e por família (§4, §7, §10) ─── */
static uint64_t cob_pos = 0;        /* elementos de X₃ examinados          */
static uint64_t pos_ok0 = 0, pos_ok1 = 0, pos_ok2 = 0, pos_ok3 = 0;
static uint64_t pos_f0  = 0, pos_f1  = 0, pos_f2  = 0, pos_f3  = 0;
static uint64_t orc_ok = 0, orc_f = 0;                       /* oráculo O₃ */
static uint64_t C0_det = 0, C0_fora = 0;                     /* controlos   */
static uint64_t C1_det = 0, C1_fora = 0;
static uint64_t C2_det = 0, C2_fora = 0;
static uint64_t N_det  = 0, N_fora  = 0;
static uint64_t lim_det = 0, lim_fora = 0;                   /* §10, 2 pontos */
static uint64_t autotest_falhas = 0;                         /* instrumento */

int main(void){
    printf("═══ medidor_iota3 — ι₃ implementada, oráculo independente ═══\n\n");

    /* ══ §0  AUTOTESTE DO INSTRUMENTO (antes de confiar em qualquer zero) ══ */
    printf("§0  autoteste do instrumento (valores fechados)\n");
    if(O3(0,0,0)   != UINT64_C(0))        { printf("      FALHA O3(0,0,0)\n");          autotest_falhas++; }
    if(O3(1,0,0)   != UINT64_C(1))        { printf("      FALHA O3(1,0,0)\n");          autotest_falhas++; }
    if(O3(0,1,0)   != UINT64_C(256))      { printf("      FALHA O3(0,1,0)\n");          autotest_falhas++; }
    if(O3(0,0,1)   != UINT64_C(65536))    { printf("      FALHA O3(0,0,1)\n");          autotest_falhas++; }
    if(O3(5,3,1)   != UINT64_C(66309))    { printf("      FALHA O3(5,3,1)\n");          autotest_falhas++; }
    if(O3(255,255,255) != UINT64_C(16777215))
                                       { printf("      FALHA O3(255,255,255)\n");     autotest_falhas++; }
    { Quad t; t.q0=1; t.q1=2; t.q2=3; t.q3=4;
      if(C4(t) != UINT64_C(67305985))     { printf("      FALHA C4(1,2,3,4)\n");        autotest_falhas++; } }
    { Quad t; t.q0=0; t.q1=0; t.q2=0; t.q3=0;
      if(C4(t) != UINT64_C(0))            { printf("      FALHA C4(0,0,0,0)\n");        autotest_falhas++; } }
    { Quad t; t.q0=255; t.q1=255; t.q2=255; t.q3=0;
      if(C4(t) != UINT64_C(16777215))     { printf("      FALHA C4(255,255,255,0)\n");  autotest_falhas++; } }
    /* o limite superior de C₄: 2³²−1. É o valor que transborda em int com
     * sinal, e é aqui que a escolha de uint64_t se torna observável. */
    { Quad t; t.q0=255; t.q1=255; t.q2=255; t.q3=255;
      if(C4(t) != UINT64_C(4294967295))   { printf("      FALHA C4(255,255,255,255)\n");autotest_falhas++; } }
    if(S1(254) != 255u || S1(255) != 0u)  { printf("      FALHA S1\n");                 autotest_falhas++; }
    if(autotest_falhas == 0)
        printf("      instrumento coerente: O₃, C₄, S₁ validados, C₄ atinge 2³²−1.\n");
    else
        printf("      instrumento NÃO confiável — " U64 " falha(s).\n", U(autotest_falhas));

    /* ══ §1..§4  CONTROLOS NEGATIVOS — independentes, ANTES do alvo ══
     * Cada um viola exactamente um canal. Detecção = o padrão (π₀,π₁,π₂,π₃)
     * sai como o esperado. Servem também de prova de que os canais distinguem
     * os quatro modos de falha. Nenhum bloco chama iota3. */

    /* C₀: espera π₀ FALHA, resto PASSA. b₀=0..254, b₁,b₂=0..255 → 255·256² */
    printf("\n§1  controlo C₀ — (S₁(b₀),b₁,b₂,0) b₀≠255: 255·256·256 = 16711680\n");
    for(int b2 = 0; b2 < 256; b2++)
        for(int b1 = 0; b1 < 256; b1++)
            for(int b0 = 0; b0 < 255; b0++){
                Quad c = construir_C0((uint8_t)b0,(uint8_t)b1,(uint8_t)b2);
                int t0 = (pi0(c) == (uint8_t)b0);   /* deve falhar */
                int t1 = (pi1(c) == (uint8_t)b1);   /* deve passar */
                int t2 = (pi2(c) == (uint8_t)b2);   /* deve passar */
                int t3 = (pi3(c) == (uint8_t)0);    /* deve passar */
                if(!t0 && t1 && t2 && t3) C0_det++; else C0_fora++;
            }
    printf("      detecção (falha,passa,passa,passa): " U64 "/16711680   fora: " U64 "\n", U(C0_det), U(C0_fora));

    /* C₁: espera π₁ FALHA, resto PASSA. b₁=0..254, b₀,b₂=0..255 → 255·256² */
    printf("\n§2  controlo C₁ — (b₀,S₁(b₁),b₂,0) b₁≠255: 255·256·256 = 16711680\n");
    for(int b2 = 0; b2 < 256; b2++)
        for(int b1 = 0; b1 < 255; b1++)
            for(int b0 = 0; b0 < 256; b0++){
                Quad c = construir_C1((uint8_t)b0,(uint8_t)b1,(uint8_t)b2);
                int t0 = (pi0(c) == (uint8_t)b0);   /* deve passar */
                int t1 = (pi1(c) == (uint8_t)b1);   /* deve falhar */
                int t2 = (pi2(c) == (uint8_t)b2);   /* deve passar */
                int t3 = (pi3(c) == (uint8_t)0);    /* deve passar */
                if(t0 && !t1 && t2 && t3) C1_det++; else C1_fora++;
            }
    printf("      detecção (passa,falha,passa,passa): " U64 "/16711680   fora: " U64 "\n", U(C1_det), U(C1_fora));

    /* C₂: espera π₂ FALHA, resto PASSA. b₂=0..254, b₀,b₁=0..255 → 255·256² */
    printf("\n§3  controlo C₂ — (b₀,b₁,S₁(b₂),0) b₂≠255: 255·256·256 = 16711680\n");
    for(int b2 = 0; b2 < 255; b2++)
        for(int b1 = 0; b1 < 256; b1++)
            for(int b0 = 0; b0 < 256; b0++){
                Quad c = construir_C2((uint8_t)b0,(uint8_t)b1,(uint8_t)b2);
                int t0 = (pi0(c) == (uint8_t)b0);   /* deve passar */
                int t1 = (pi1(c) == (uint8_t)b1);   /* deve passar */
                int t2 = (pi2(c) == (uint8_t)b2);   /* deve falhar */
                int t3 = (pi3(c) == (uint8_t)0);    /* deve passar */
                if(t0 && t1 && !t2 && t3) C2_det++; else C2_fora++;
            }
    printf("      detecção (passa,passa,falha,passa): " U64 "/16711680   fora: " U64 "\n", U(C2_det), U(C2_fora));

    /* N: espera π₃ FALHA, resto PASSA. b₂=1..255, b₀,b₁=0..255 → 255·256² */
    printf("\n§4  controlo N — (b₀,b₁,b₂,b₂) b₂≠0: 255·256·256 = 16711680\n");
    for(int b2 = 1; b2 < 256; b2++)
        for(int b1 = 0; b1 < 256; b1++)
            for(int b0 = 0; b0 < 256; b0++){
                Quad c = construir_N((uint8_t)b0,(uint8_t)b1,(uint8_t)b2);
                int t0 = (pi0(c) == (uint8_t)b0);   /* deve passar */
                int t1 = (pi1(c) == (uint8_t)b1);   /* deve passar */
                int t2 = (pi2(c) == (uint8_t)b2);   /* deve passar */
                int t3 = (pi3(c) == (uint8_t)0);    /* deve falhar */
                if(t0 && t1 && t2 && !t3) N_det++; else N_fora++;
            }
    printf("      detecção (passa,passa,passa,falha): " U64 "/16711680   fora: " U64 "\n", U(N_det), U(N_fora));

    /* ══ §5  ALVO ι₃ — a ÚNICA secção que chama iota3 ══
     * Cobertura exaustiva 256·256·256 = 16777216/16777216. Para cada
     * (b₀,b₁,b₂): y=ι₃(b₀,b₁,b₂); lê-se out.b0..out.b3 (quatro bytes
     * independentes) para um Quad; verifica-se π₀=b₀, π₁=b₁, π₂=b₂, π₃=0 nos
     * quatro canais; e compara-se a codificação independente C₄(y) com o oráculo
     * O₃(b₀,b₁,b₂). O esperado vem sempre dos argumentos — nunca de iota3. */
    printf("\n§5  cobertura positiva exaustiva do alvo iota3 — 256·256·256 = 16777216\n");
    for(int b2 = 0; b2 < 256; b2++)
        for(int b1 = 0; b1 < 256; b1++)
            for(int b0 = 0; b0 < 256; b0++){
                Iota3Out out = iota3((uint8_t)b0,(uint8_t)b1,(uint8_t)b2);  /* ← única chamada */
                Quad y; y.q0 = out.b0; y.q1 = out.b1;   /* 4 leituras */
                      y.q2 = out.b2; y.q3 = out.b3;

                int t0 = (pi0(y) == (uint8_t)b0);
                int t1 = (pi1(y) == (uint8_t)b1);
                int t2 = (pi2(y) == (uint8_t)b2);
                int t3 = (pi3(y) == (uint8_t)0);
                if(t0) pos_ok0++; else pos_f0++;
                if(t1) pos_ok1++; else pos_f1++;
                if(t2) pos_ok2++; else pos_f2++;
                if(t3) pos_ok3++; else pos_f3++;
                cob_pos++;

                if(C4(y) == O3((uint8_t)b0,(uint8_t)b1,(uint8_t)b2)) orc_ok++; else orc_f++;

                /* ── §10  CASOS-LIMITE EXPLICÍTOS, no mesmo ponto de chamada ──
                 * (0,0,0)      → (0,0,0,0)        e C₄ = 0
                 * (255,255,255) → (255,255,255,0) e C₄ = 16777215
                 * Comparados contra LITERAIS, e não contra O₃, para que uma
                 * divergência entre alvo e instrumento não se anule. */
                int lim = ((b0 == 0   && b1 == 0   && b2 == 0) ||
                           (b0 == 255 && b1 == 255 && b2 == 255));
                if(lim){
                    uint8_t eb0, eb1, eb2, eb3;
                    uint64_t ecode;
                    if(b0 == 0){ eb0=0;   eb1=0;   eb2=0;   eb3=0;
                                 ecode = UINT64_C(0); }
                    else       { eb0=255; eb1=255; eb2=255; eb3=0;
                                 ecode = UINT64_C(16777215); }
                    if(out.b0==eb0 && out.b1==eb1 && out.b2==eb2 && out.b3==eb3 &&
                       C4(y) == ecode) lim_det++;
                    else lim_fora++;
                }
            }
    printf("      cobertura examinada: " U64 "/16777216\n", U(cob_pos));
    printf("      π₀==b₀: " U64 "/16777216   falhas: " U64 "\n", U(pos_ok0), U(pos_f0));
    printf("      π₁==b₁: " U64 "/16777216   falhas: " U64 "\n", U(pos_ok1), U(pos_f1));
    printf("      π₂==b₂: " U64 "/16777216   falhas: " U64 "\n", U(pos_ok2), U(pos_f2));
    printf("      π₃==0 : " U64 "/16777216   falhas: " U64 "\n", U(pos_ok3), U(pos_f3));
    printf("      oráculo C₄(y)==O₃(b₀,b₁,b₂): " U64 "/16777216   falhas: " U64 "\n", U(orc_ok), U(orc_f));

    /* ══ §6  RESÍDUO, FALHAS, VEREDITO ══ */
    uint64_t ctrls_fora = C0_fora + C1_fora + C2_fora + N_fora;
    uint64_t resíduo = N_COB - cob_pos;      /* §10: elementos sem veredicto */
    uint64_t falhas  = pos_f0 + pos_f1 + pos_f2 + pos_f3   /* falhas por canal */
                    + orc_f                                  /* oráculo          */
                    + ctrls_fora                             /* controlos fora   */
                    + lim_fora                               /* casos-limite     */
                    + autotest_falhas;                       /* instrumento      */

    int cobertura_ok = (cob_pos == N_COB);
    int canais_ok    = (pos_f0==0 && pos_f1==0 && pos_f2==0 && pos_f3==0);
    int oraculo_ok   = (orc_ok == N_COB);
    int C0_ok = (C0_det == N_CTRL), C1_ok = (C1_det == N_CTRL);
    int C2_ok = (C2_det == N_CTRL), N_ok  = (N_det  == N_CTRL);
    int ctrls_ok = (C0_ok && C1_ok && C2_ok && N_ok);
    int lim_ok   = (lim_det == 2 && lim_fora == 0);
    int veredicto = (autotest_falhas==0 && cobertura_ok && canais_ok &&
                     oraculo_ok && ctrls_ok && lim_ok && resíduo==0 && falhas==0);

    printf("\n══════════════════════════════════════════════════════════════\n");
    printf("  COBERTURA     : " U64 "/16777216\n", U(cob_pos));
    printf("  π₀            : " U64 "/16777216   (falhas " U64 ")\n", U(pos_ok0), U(pos_f0));
    printf("  π₁            : " U64 "/16777216   (falhas " U64 ")\n", U(pos_ok1), U(pos_f1));
    printf("  π₂            : " U64 "/16777216   (falhas " U64 ")\n", U(pos_ok2), U(pos_f2));
    printf("  π₃            : " U64 "/16777216   (falhas " U64 ")\n", U(pos_ok3), U(pos_f3));
    printf("  ORÁCULO       : " U64 "/16777216   (falhas " U64 ")\n", U(orc_ok), U(orc_f));
    printf("  CONTROLE C₀   : " U64 "/16711680 detectam  %s\n", U(C0_det), C0_ok ? "OK":"FORA");
    printf("  CONTROLE C₁   : " U64 "/16711680 detectam  %s\n", U(C1_det), C1_ok ? "OK":"FORA");
    printf("  CONTROLE C₂   : " U64 "/16711680 detectam  %s\n", U(C2_det), C2_ok ? "OK":"FORA");
    printf("  CONTROLE N    : " U64 "/16711680 detectam  %s\n", U(N_det),  N_ok  ? "OK":"FORA");
    printf("  CASOS-LIMITE  : (0,0,0) e (255,255,255)  %s\n", lim_ok ? "OK":"FORA");
    printf("  AUTOTESTE     : %s\n", (autotest_falhas==0)?"PASS":"FALHA");
    printf("  RESÍDUO       : " U64 "\n", U(resíduo));
    printf("  FALHAS        : " U64 "\n", U(falhas));
    printf("  VEREDITO      : %s\n", veredicto ? "FECHADO" : "ABERTO");
    if(veredicto)
        printf("  (cobertura e controlos NUNCA somados: 16777216 e 66846720 são\n"
               "   grandezas distintas, §8 do contrato)\n");
    return veredicto ? 0 : 1;
}
