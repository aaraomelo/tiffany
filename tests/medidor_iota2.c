/* tests/medidor_iota2.c — ORÁCULO INDEPENDENTE para ι₂ : X₂ ↪ X₃
 *
 *    Estado: alvo implementado (lib/iota2.h). Este medidor está executado e
 *    validado contra o que CONTRATO_X3.md exige em §4, §6 e §7: cobertura
 *    exaustiva 65536/65536, três canais separados, oráculo O₂ independente, e
 *    os três controlos negativos a detectar o canal que cada um viola.
 *    NÃO está executada a bateria de dez mutações de §9 — nenhuma taxa de
 *    detecção é afirmada, e o VEREDITO impresso é o de §10 (contadores), não
 *    uma afirmação de capacidade de auditoria. §12.1 e §12.3 ficam decididas
 *    na prática por este ficheiro: representação em três bytes, e oráculo por
 *    Horner big-endian. O contrato não foi alterado para reflectir isto.
 *
 *    Este ficheiro mede iota2 sem a chamar para construir o esperado.
 *
 * Porquê existe (CONTRATO_X3.md): o contrato define a seta iota₂, o oráculo
 * O₂(b₀,b₁)=b₀+256·b₁, e três controlos negativos. Este instrumento mede
 * sem depender da implementação para construir o esperado:
 *   - cobertura positiva exaustiva, elemento a elemento de X₂;
 *   - três canais lidos SEPARADAMENTE (π₀, π₁, π₂), todos no veredicto;
 *   - oráculo O₂ calculado dos argumentos de entrada, nunca da saída;
 *   - três controlos negativos, cada um violando exactamente um canal.
 *
 * ── INDEPENDÊNCIA (CONTRATO_X3.md §6, §7) ──────────────────────────────
 *   O tipo Tri e as funções de construção (S1, C3, oracle, construir_A/B1/B2,
 *   e as projeções pi0/pi1/pi2) são PRÓPRIOS do medidor. Nenhuma delas chama
 *   iota2, nem lê um Iota2Out, nem inclui lógica do alvo. A única linha de todo
 *   o ficheiro que executa uma chamada a iota2 é a secção §4, dentro de main,
 *   e dela saem três leituras de byte independentes (out.b0, out.b1, out.b2)
 *   copiadas para um Tri local antes de qualquer verificação. A verificação —
 *   que é partilhada entre alvo e controlos — nunca vê a saída de iota2 a menos
 *   de três bytes, e nunca a vê por valor empacotado.
 *
 * Representação (CONTRATO_X3.md §3.3, indexação 0-based):
 *   π₀(y)=b₀   π₁(y)=b₁   π₂(y)=b₂
 *   C₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂   (0 ≤ C₃ ≤ 16777215)
 *
 * O oráculo e a codificação C3 são implementados por forma INDEPENDENTE do
 * alvo (CONTRATO_X3.md §12.3): C3 é acumulado em ordem big-endian por Horner,
 *   C3 = ((t2·256 + t1)·256 + t0),
 * e o alvo (lib/iota2.h) não calcula código nenhum — preenche três bytes. Não
 * há, portanto, forma partilhada a duplicar.
 *
 * Não altera: campos.tex, espaco.tex, CONTRATO_X3.md, lib/iota2.h,
 *             lib/iota1.h, CONTRATO_X2.md, papers, headers existentes.
 * ────────────────────────────────────────────────────────────────────────
 * Compilar:
 *   cc -O2 -std=c99 -Wall -Wextra -Ilib -o medidor_iota2 tests/medidor_iota2.c
 * Executar:
 *   ./medidor_iota2
 *   Sai com 0 se VEREDITO=FECHADO, 1 caso contrário.
 */
#include <stdio.h>
#include <stdint.h>
#include "iota2.h"

/* ─── tipo do MEDIDOR: independente de Iota2Out ───
 * Os controlos são construídos a partir deste tipo, sem qualquer dependência
 * do header do alvo. t0,t1,t2 correspondem a π₀,π₁,π₂ (0-based, §3.3). */
typedef struct { uint8_t t0, t1, t2; } Tri;

/* ─── projeções π₀,π₁,π₂ do MEDIDOR: leem três bytes separados ───
 * Cada função lê UM byte e nada mais — obrigação de canal, §3.3. */
static uint8_t pi0(Tri t){ return t.t0; }
static uint8_t pi1(Tri t){ return t.t1; }
static uint8_t pi2(Tri t){ return t.t2; }

/* ─── codificação C3 do MEDIDOR, forma independente (Horner big-endian) ───
 * C₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂. uint32_t cabe 2²⁴−1 sem folga de
 * overflow, §3.1. Não usa nenhuma função do alvo. */
static uint32_t C3(Tri t){
    uint32_t acc = (uint32_t)t.t2;
    acc = acc * 256u + (uint32_t)t.t1;
    acc = acc * 256u + (uint32_t)t.t0;
    return acc;
}

/* ─── sucessor S₁ do paper: S₁(b) = (b+1) mod 256 (campos.tex:433) ─── */
static uint8_t S1(uint8_t b){ return (uint8_t)(b + 1u); }

/* ─── ORÁCULO O₂, INDEPENDENTE (§6) ───
 * O₂(b₀,b₁) = C₃(b₀,b₁,0) = b₀ + 256·b₁. Calculado APENAS dos argumentos de
 * entrada b₀,b₁ e do zero da terceira posição, pela forma posicional de §3.1.
 * Não chama iota2; não lê a saída de iota2; sobrevive a qualquer alteração do
 * alvo. */
static uint32_t oracle(uint8_t b0, uint8_t b1){
    Tri e; e.t0 = b0; e.t1 = b1; e.t2 = (uint8_t)0;   /* (b₀,b₁,0) */
    return C3(e);
}

/* ─── CONSTRUÇÃO DOS CONTROLOS, INDEPENDENTE (§7) ───
 * Cada construtor devolve directamente um Tri com coordenadas explícitas.
 * Nenhum chama iota2; nenhum parte de uma saída de iota2 corrompida. */

/* A — terceira coordenada errada: (b₀,b₁,b₂=b₁), b₁≠0  (§7.1) */
static Tri construir_A(uint8_t b0, uint8_t b1){
    Tri c; c.t0 = b0; c.t1 = b1; c.t2 = b1; return c;   /* t2 := b1, errado */
}

/* B1 — primeira coordenada errada: (S₁(b₀),b₁,0), b₀≠255  (§7.2) */
static Tri construir_B1(uint8_t b0, uint8_t b1){
    Tri c; c.t0 = S1(b0); c.t1 = b1; c.t2 = (uint8_t)0; return c;
}

/* B2 — segunda coordenada errada: (b₀,S₁(b₁),0), b₁≠255  (§7.3) */
static Tri construir_B2(uint8_t b0, uint8_t b1){
    Tri c; c.t0 = b0; c.t1 = S1(b1); c.t2 = (uint8_t)0; return c;
}

/* ─── contadores, separados por componente (SECÇÃO 5 do pedido) ─── */
static int cob_pos = 0;          /* elementos de X₂ examinados            */
static int pos_ok0 = 0, pos_ok1 = 0, pos_ok2 = 0;   /* π₀,π₁,π₂ correctas */
static int pos_f0  = 0, pos_f1  = 0, pos_f2  = 0;   /* falhas por canal   */
static int orc_ok = 0, orc_f = 0;                    /* oráculo O₂        */
static int A_det = 0, A_fora = 0, B1_det = 0, B1_fora = 0;
static int B2_det = 0, B2_fora = 0;                  /* controlos         */
static int autotest_falhas = 0;                      /* instrumento       */

int main(void){
    printf("═══ medidor_iota2 — ι₂ implementada, oráculo independente ═══\n\n");

    /* ══ §0  AUTOTESTE DO INSTRUMENTO (antes de confiar em qualquer zero) ══ */
    printf("§0  autoteste do instrumento (valores fechados)\n");
    if(oracle(0,0) != 0u)                 { printf("      FALHA oracle(0,0)\n");     autotest_falhas++; }
    if(oracle(1,0) != 1u)                 { printf("      FALHA oracle(1,0)\n");     autotest_falhas++; }
    if(oracle(0,1) != 256u)               { printf("      FALHA oracle(0,1)\n");     autotest_falhas++; }
    if(oracle(5,3) != 773u)               { printf("      FALHA oracle(5,3)\n");     autotest_falhas++; }
    if(oracle(255,255) != 65535u)         { printf("      FALHA oracle(255,255)\n"); autotest_falhas++; }
    { Tri t; t.t0=1; t.t1=2; t.t2=3;
      if(C3(t) != 197121u)                { printf("      FALHA C3(1,2,3)\n");      autotest_falhas++; } }
    { Tri t; t.t0=255; t.t1=255; t.t2=255;
      if(C3(t) != 16777215u)              { printf("      FALHA C3(255,255,255)\n");autotest_falhas++; } }
    if(S1(254) != 255u || S1(255) != 0u)  { printf("      FALHA S1\n");             autotest_falhas++; }
    if(autotest_falhas == 0)
        printf("      instrumento coerente: O₂, C₃, S₁ validados.\n");
    else
        printf("      instrumento NÃO confiável — %d falha(s).\n", autotest_falhas);

    /* ══ §1  CONTROLES NEGATIVOS — independentes, ANTES do alvo ══
     * Cada um viola exactamente um canal. Detecção = o padrão (π₀,π₁,π₂)
     * sai como o esperado. Servem também de prova de que os canais
     * distinguem os três modos de falha. Nenhum bloco chama iota2. */

    /* A: espera π₀ PASSA, π₁ PASSA, π₂ FALHA. b₁=1..255, b₀=0..255 → 65280 */
    printf("\n§1  controlo A — (b₀,b₁,b₁) b₁≠0: 256·255 = 65280\n");
    for(int b1 = 1; b1 < 256; b1++)
        for(int b0 = 0; b0 < 256; b0++){
            Tri c = construir_A((uint8_t)b0,(uint8_t)b1);
            int t0 = (pi0(c) == (uint8_t)b0);   /* deve passar */
            int t1 = (pi1(c) == (uint8_t)b1);   /* deve passar */
            int t2 = (pi2(c) == (uint8_t)0);    /* deve falhar */
            if(t0 && t1 && !t2) A_det++; else A_fora++;
        }
    printf("      detecção (passa,passa,falha): %d/65280   fora: %d\n", A_det, A_fora);

    /* B1: espera π₀ FALHA, π₁ PASSA, π₂ PASSA. b₀=0..254, b₁=0..255 → 65280 */
    printf("\n§2  controlo B1 — (S₁(b₀),b₁,0) b₀≠255: 255·256 = 65280\n");
    for(int b0 = 0; b0 < 255; b0++)
        for(int b1 = 0; b1 < 256; b1++){
            Tri c = construir_B1((uint8_t)b0,(uint8_t)b1);
            int t0 = (pi0(c) == (uint8_t)b0);   /* deve falhar */
            int t1 = (pi1(c) == (uint8_t)b1);   /* deve passar */
            int t2 = (pi2(c) == (uint8_t)0);    /* deve passar */
            if(!t0 && t1 && t2) B1_det++; else B1_fora++;
        }
    printf("      detecção (falha,passa,passa): %d/65280   fora: %d\n", B1_det, B1_fora);

    /* B2: espera π₀ PASSA, π₁ FALHA, π₂ PASSA. b₀=0..255, b₁=0..254 → 65280 */
    printf("\n§3  controlo B2 — (b₀,S₁(b₁),0) b₁≠255: 256·255 = 65280\n");
    for(int b0 = 0; b0 < 256; b0++)
        for(int b1 = 0; b1 < 255; b1++){
            Tri c = construir_B2((uint8_t)b0,(uint8_t)b1);
            int t0 = (pi0(c) == (uint8_t)b0);   /* deve passar */
            int t1 = (pi1(c) == (uint8_t)b1);   /* deve falhar */
            int t2 = (pi2(c) == (uint8_t)0);    /* deve passar */
            if(t0 && !t1 && t2) B2_det++; else B2_fora++;
        }
    printf("      detecção (passa,falha,passa): %d/65280   fora: %d\n", B2_det, B2_fora);

    /* ══ §4  ALVO ι₂ — a ÚNICA secção que chama iota2 ══
     * Cobertura exaustiva 65536/65536. Para cada (b₀,b₁): y=ι₂(b₀,b₁);
     * lê-se out.b0, out.b1, out.b2 (três bytes independentes) para um Tri;
     * verifica-se π₀=b₀, π₁=b₁, π₂=0 nos três canais; e compara-se a
     * codificação independente C3(y) com o oráculo O₂(b₀,b₁). O esperado
     * vem sempre dos argumentos b₀,b₁ — nunca de iota2. */
    printf("\n§4  cobertura positiva exaustiva do alvo iota2 — 256·256 = 65536\n");
    for(int b0 = 0; b0 < 256; b0++)
        for(int b1 = 0; b1 < 256; b1++){
            Iota2Out out = iota2((uint8_t)b0,(uint8_t)b1);  /* ← única chamada */
            Tri y; y.t0 = out.b0; y.t1 = out.b1; y.t2 = out.b2;  /* 3 leituras */

            int t0 = (pi0(y) == (uint8_t)b0);
            int t1 = (pi1(y) == (uint8_t)b1);
            int t2 = (pi2(y) == (uint8_t)0);
            if(t0) pos_ok0++; else pos_f0++;
            if(t1) pos_ok1++; else pos_f1++;
            if(t2) pos_ok2++; else pos_f2++;
            cob_pos++;

            if(C3(y) == oracle((uint8_t)b0,(uint8_t)b1)) orc_ok++; else orc_f++;
        }
    printf("      cobertura examinada: %d/65536\n", cob_pos);
    printf("      π₀==b₀: %d/65536   falhas: %d\n", pos_ok0, pos_f0);
    printf("      π₁==b₁: %d/65536   falhas: %d\n", pos_ok1, pos_f1);
    printf("      π₂==0 : %d/65536   falhas: %d\n", pos_ok2, pos_f2);
    printf("      oráculo C₃(y)==O₂(b₀,b₁): %d/65536   falhas: %d\n", orc_ok, orc_f);

    /* ══ §5  RESÍDUO, FALHAS, VEREDITO ══ */
    int resíduo = 65536 - cob_pos;          /* §10: elementos sem veredicto */
    int falhas  = pos_f0 + pos_f1 + pos_f2  /* falhas por canal no alvo    */
                + orc_f                    /* oráculo                      */
                + A_fora + B1_fora + B2_fora /* controlos fora do padrão    */
                + autotest_falhas;         /* instrumento                  */

    int cobertura_ok   = (cob_pos == 65536);
    int canais_ok      = (pos_f0==0 && pos_f1==0 && pos_f2==0);
    int oraculo_ok     = (orc_ok == 65536);
    int A_ok = (A_det == 65280), B1_ok = (B1_det == 65280), B2_ok = (B2_det == 65280);
    int ctrls_ok = (A_ok && B1_ok && B2_ok);
    int veredicto = (autotest_falhas==0 && cobertura_ok && canais_ok &&
                     oraculo_ok && ctrls_ok && resíduo==0 && falhas==0);

    printf("\n══════════════════════════════════════════════════════════════\n");
    printf("  COBERTURA     : %d/65536\n", cob_pos);
    printf("  π₀            : %d/65536   (falhas %d)\n", pos_ok0, pos_f0);
    printf("  π₁            : %d/65536   (falhas %d)\n", pos_ok1, pos_f1);
    printf("  π₂            : %d/65536   (falhas %d)\n", pos_ok2, pos_f2);
    printf("  ORÁCULO       : %d/65536   (falhas %d)\n", orc_ok, orc_f);
    printf("  CONTROLE A    : %d/65280 detectam  %s\n", A_det,  A_ok  ? "OK":"FORA");
    printf("  CONTROLE B1   : %d/65280 detectam  %s\n", B1_det, B1_ok ? "OK":"FORA");
    printf("  CONTROLE B2   : %d/65280 detectam  %s\n", B2_det, B2_ok ? "OK":"FORA");
    printf("  AUTOTESTE     : %s\n", (autotest_falhas==0)?"PASS":"FALHA");
    printf("  RESÍDUO       : %d\n", resíduo);
    printf("  FALHAS        : %d\n", falhas);
    printf("  VEREDITO      : %s\n", veredicto ? "FECHADO" : "ABERTO");
    if(veredicto)
        printf("  (cobertura e controlos NUNCA somados: 65536 e 195840 são\n"
               "   grandezas distintas, §8 do contrato)\n");
    return veredicto ? 0 : 1;
}
