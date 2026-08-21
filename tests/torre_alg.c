/* torre_alg.c — A FIRMA DA TORRE: três pipelines, quatro operações, slots no disco.
 *
 * O ordem do coordenador (19/08/2026) fixa o pós-Fase-A: a largura NÃO define a álgebra —
 * a dobra sim. Este medidor torna visível o contrato de lib/torre_alg.h:
 *
 *   INVOLUÇÃO   ν∘ν = id       estrutural (swap, dual das metades, conj)
 *   RETRAÇÃO    Σ∘Π = Id       representação (FC nos slots S_CF do .mem)
 *   SUBIDA      T_k→T_{k+1}   construção (produto exige par seguinte)
 *
 * Não confundir involução com retração: espaços e representações diferentes.
 *
 *   §T1  INVOLUÇÃO: ι, dual das metades (D32, D64, I128), hip_conj — ν∘ν = id
 *        nos CINCO andares da torre
 *   §T2  RETRAÇÃO: p/q → slots → p'/q' exacto (Σ∘Π sobre a palavra FC)
 *   §T3  SUBIDA: 16→32→64 — a largura segue a dobra, não um tipo escolhido
 *   §T4  OS TRÊS PIPELINES SÃO DISTINTOS — o que cada um preserva e o que muda
 *
 * Papers: racionais.tex def:ops thm:swap-inverso · corpo_algebrico.tex thm:torre
 *         def:cone · corpo_topologico.tex thm:rn
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/torre_alg.c -o torre_alg && ./torre_alg
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "unidade.h"
#include "torre_alg.h"
#include "i128.h"
#include "dual16.h"
#include "dual32.h"
#include "torre.h"
#include "rt_cf_slot.h"

static int d32par_igual(D32par a, D32par b){
    return d16_cmp(a.p, b.p) == 0 && d16_cmp(a.q, b.q) == 0;
}

int main(void){
printf("\n=== A FIRMA DA TORRE: três pipelines, slots, dobra =================================\n");
printf("    Contrato: lib/torre_alg.h — álgebra → operação → dual → largura → slot\n");

/* ── §T1  INVOLUÇÃO ─────────────────────────────────────────────────────────── */
printf("\n§T1  INVOLUÇÃO ν∘ν = id — swap, dual das metades, conj (estrutural).\n\n");
{
    /* E CADA UMA TEM DE MOVER ALGUMA COISA. `ν∘ν = id` sozinho passa com ν = id —
     * a identidade É uma involução —, e foi assim que esta asserção esteve escrita:
     * pôr `i128_dual` a devolver o argumento não a derrubava. Um dual que não move
     * nada não é o dual: é a ausência dele com o nome certo. Por isso cada andar
     * traz também a TESTEMUNHA ν(x) ≠ x. */
    int ok_i = 0, ok_d32 = 0, ok_d64 = 0, ok_c = 0, ok_i128 = 0;
    int move_i = 0, move_d32 = 0, move_d64 = 0, move_c = 0, move_i128 = 0;
    D32par par = { d16_mult(7, 11), d16_mult(13, 17) };
    D32par ida = d16_par_dual(par);
    D32par volta = d16_par_dual(ida);
    if(d32par_igual(volta, par)) ok_i++;
    if(!d32par_igual(ida, par)) move_i++;

    D32 prod = d16_mult(30000, 30000);
    if(d16_cmp(d32_dual(d32_dual(prod)), prod) == 0) ok_d32++;
    if(d16_cmp(d32_dual(prod), prod) != 0) move_d32++;

    D64 p64 = d64_mult(60000u, 60000u);
    if(d64_cmp(d64_dual(d64_dual(p64)), p64) == 0) ok_d64++;
    if(d64_cmp(d64_dual(p64), p64) != 0) move_d64++;

    /* O ANDAR QUE FALTAVA. A torre tem CINCO andares (E₁₆, D32, D64, I128, Hip) e
     * esta asserção media QUATRO: o I128 era o único sem dual, e por isso não
     * entrava aqui. Contar «quatro sítios» quando a torre tem cinco não falha
     * nenhuma asserção — some um andar, e o texto continua a fechar. */
    I128 p128 = i128_smul(3037000499LL, 3037000499LL);   /* produto que passa dos 64 */
    if(i128_cmp(i128_dual(i128_dual(p128)), p128) == 0) ok_i128++;
    if(i128_cmp(i128_dual(p128), p128) != 0) move_i128++;

    Hip x = hip0(4);
    x.c[0] = 2; x.c[1] = -3; x.c[2] = 5; x.c[3] = -7;
    if(hip_igual(hip_conj(hip_conj(x)), x)) ok_c++;
    if(!hip_igual(hip_conj(x), x)) move_c++;

    printf("      ι∘ι no par racional (D32par)     %s\n", ok_i ? "sim" : "NÃO");
    printf("      dual∘dual em D32 (metades)      %s\n", ok_d32 ? "sim" : "NÃO");
    printf("      dual∘dual em D64 (metades)      %s\n", ok_d64 ? "sim" : "NÃO");
    printf("      dual∘dual em I128 (metades)     %s\n", ok_i128 ? "sim" : "NÃO");
    printf("      hip_conj∘hip_conj (dim 4)       %s\n", ok_c ? "sim" : "NÃO");
    printf("      e cada uma MOVE: %d de 5 andares têm ν(x) ≠ x\n\n",
           move_i + move_d32 + move_d64 + move_i128 + move_c);
    ok("a INVOLUÇÃO fecha nos CINCO andares da torre, e eram quatro: swap ι(p,q)=(q,p)"
       " no par racional, troca das metades alto/baixo em D32, D64 e I128, e conjugação"
       " Cayley–Dickson — cada uma ν∘ν = id, e NENHUMA delas é retração Σ∘Π (ordem do"
       " coordenador §3). O I128 era o único andar SEM dual: o `torre_alg.h` fixa quatro"
       " operações estruturais por andar — soma, produto, dual, inversão/fibra — e ali"
       " havia duas. Contar «quatro sítios» com a torre a ter cinco não falhava asserção"
       " nenhuma: some um andar e o texto continua a fechar. E cada uma MOVE alguma"
       " coisa — ν(x) ≠ x exibido nos cinco: `ν∘ν = id` sozinho passa com ν = id,"
       " porque a identidade É uma involução, e um dual que não move nada é a"
       " ausência dele com o nome certo",
       ok_i && ok_d32 && ok_d64 && ok_i128 && ok_c
       && move_i && move_d32 && move_d64 && move_i128 && move_c);
}

/* ── §T2  RETRAÇÃO ──────────────────────────────────────────────────────────── */
printf("\n§T2  RETRAÇÃO Σ∘Π = Id — FC nos slots S_CF=2048 (representação).\n\n");
{
    int cf_fd = rt_cf_slot_mem_abre("dados/torre_alg_cf.mem");
    RtCfSlot cf = rt_cf_slot_word(0, cf_fd);
    struct { int sg; long p, q; } casos[] = {
        { 1,  355, 113 },   /* π convergente */
        { 1,   22,   7 },   /* φ aproximado */
        { -1,    1,   3 },  /* −1/3 */
        {  1, 103993, 33102 },
    };
    int n = (int)(sizeof casos / sizeof *casos), fecha = 0;
    printf("      p/q           n   sat?   volta exacta?\n");
    for(int k = 0; k < n; k++){
        rt_cf_slot_de(casos[k].sg, casos[k].p, casos[k].q, &cf);
        long p2 = 0, q2 = 0;
        int sat = rt_cf_slot_saturou(&cf);
        int ok_r = !sat && rt_cf_slot_para(&cf, &p2, &q2)
            && p2 * casos[k].q == casos[k].sg * casos[k].p * q2;
        if(ok_r) fecha++;
        printf("      %s%ld/%ld   %-3d  %s    %s\n",
               casos[k].sg < 0 ? "−" : "", casos[k].p < 0 ? -casos[k].p : casos[k].p, casos[k].q,
               rt_cf_slot_n(&cf), sat ? "sim" : "não", ok_r ? "sim" : "NÃO");
    }
    printf("\n");
    ok("a RETRAÇÃO fecha: p/q → palavra FC nos slots .mem → p'/q' por produto cruzado"
       " — quatro racionais, ida e volta EXACTA, sem float. É Σ∘Π sobre a representação"
       " posicional (corpo_algebrico.tex def:cone), ancorada em S_CF=2048 (slot_map.h)",
       fecha == n && n == 4);

    /* ── E O TECTO DO TERMO TEM ONDE BATER, senão é comentário e não limite ────────
     * O envelope de um a_k é Word_8² (0..65535). Acima disso a escrita RECUSA-SE e
     * acende `saturou` — não enrola. Os dois lados medem-se: um quociente ENCOSTADO
     * ao tecto tem de passar e voltar exacto, um quociente ACIMA tem de ser recusado.
     * Sem o primeiro, «recusa sempre» passava; sem o segundo, «nunca recusa» passava. */
    {
        struct { long p, q; int cabe; } tecto[] = {
            { RT_CF_TERMO_MAX,     1, 1 },   /* a_0 = 65535 — encosta e cabe */
            { RT_CF_TERMO_MAX + 1, 1, 0 },   /* a_0 = 65536 — não cabe, RECUSA */
        };
        long mau = 0;
        printf("      a_0        cabe?   saturou   volta\n");
        for(int k = 0; k < 2; k++){
            rt_cf_slot_de(1, tecto[k].p, tecto[k].q, &cf);
            int sat = rt_cf_slot_saturou(&cf);
            long p2 = 0, q2 = 0;
            int volta = !sat && rt_cf_slot_para(&cf, &p2, &q2)
                     && p2 == tecto[k].p && q2 == tecto[k].q;
            if(tecto[k].cabe ? (sat || !volta) : (!sat || volta)) mau++;
            printf("      %-10ld %-7s %-9s %s\n", tecto[k].p,
                   tecto[k].cabe ? "sim" : "não", sat ? "sim" : "não",
                   volta ? "exacta" : "—");
        }
        printf("\n");
        ok("O TECTO DO TERMO É MEDIDO NOS DOIS LADOS: o a_k vive em Word_8² e 65535"
           " ENCOSTA — escreve-se e volta exacto —, enquanto 65536 é RECUSADO com"
           " `saturou` aceso e sem valor devolvido. Um tecto que só se cita não é"
           " limite: é documentação. E medir só o lado que passa deixava «recusa"
           " sempre» indistinguível de «recusa quando deve»",
           mau == 0);
    }
    if(cf_fd >= 0) close(cf_fd);
}

/* ── §T3  SUBIDA ────────────────────────────────────────────────────────────── */
printf("\n§T3  SUBIDA T_k→T_{k+1}: a largura segue a dobra do produto.\n\n");
{
    D32 p16 = d16_mult(32767, 32767);
    D64 p32 = d64_mult(70000u, 70000u);   /* 4,9×10⁹ > 2³² — exige metade alta */
    int usa32 = (p16.alto != 0);
    int usa64 = (p32.alto != 0);
    int passo = tr_passo_conj(2, 40);
    printf("      32767×32767 precisa de D32 (alto≠0)     %s\n", usa32 ? "sim" : "NÃO");
    printf("      70000×70000 precisa de D64 (alto≠0)     %s\n", usa64 ? "sim" : "NÃO");
    printf("      tr_passo_conj(2→4): ν atravessa dobra   %s\n\n", passo ? "sim" : "NÃO");
    ok("a SUBIDA mede-se pela largura NECESSÁRIA: produto 16×16 transborda int16 e"
       " materializa D32; produto 32×32 materializa D64; e tr_passo_conj confirma que"
       " a involução atravessa a dobra Cayley–Dickson (corpo_topologico.tex thm:rn)."
       " A largura não foi escolhida — foi exigida pela operação",
       usa32 && usa64 && passo == 1);
}

/* ── §T4  TRÊS PIPELINES DISTINTOS ──────────────────────────────────────────── */
printf("\n§T4  Os três pipelines preservam coisas DIFERENTES — não são «inversão» genérica.\n\n");
{
    int16_t a = 3, b = 5;
    D32par rac = { d16_mult(a, b), d16_mult(b, b) };  /* 15/25 reduzível; uso bruto */
    D32par inv = d16_par_dual(rac);
    int inv_muda = !d32par_igual(rac, inv);

    int cf_fd = rt_cf_slot_mem_abre("dados/torre_alg_cf2.mem");
    RtCfSlot cf = rt_cf_slot_word(0, cf_fd);
    rt_cf_slot_de(1, 3, 5, &cf);
    long p2 = 0, q2 = 0;
    rt_cf_slot_para(&cf, &p2, &q2);
    int retr_preserva = (p2 * 5 == 3 * q2);

    D32 antes = d16_mult(20000, 20000);
    D32 depois = d32_dual(antes);
    int subida_muda = (d16_cmp(antes, depois) != 0);

    printf("      involução ι: troca p↔q no par        %s (classe muda)\n",
           inv_muda ? "sim" : "NÃO");
    printf("      retração FC: 3/5 volta a 3/5         %s (classe igual)\n",
           retr_preserva ? "sim" : "NÃO");
    printf("      dual metades: troca alto↔baixo       %s (valor muda)\n\n",
           subida_muda ? "sim" : "NÃO");
    ok("INVOLUÇÃO, RETRAÇÃO e SUBIDA são três coisas: ι troca o representante e muda a"
       " classe racional; Σ∘Π preserva a classe e muda a palavra; dual das metades troca"
       " a memória da divisão sem confundir com nenhum dos outros (ordem do coordenador (§3)",
       inv_muda && retr_preserva && subida_muda);
    if(cf_fd >= 0) close(cf_fd);
}

printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
return falhas ? 1 : 0;
}
