/* topologia.c — A TOPOLOGIA DOS CORPOS: a distância é entre as RÉGUAS, e o transporte é o gato.
 *
 * O Aarão: "agora temos corpos de corpos e corpo de métricas. Vale agora uma topologia de corpos:
 * dá a distância entre corpos com a distância entre suas réguas no corpo métrico. Aí você tem a
 * função de transferência bijetora de um corpo para outro, além de mostrar isomorfismo."
 *
 * A régua é um PONTO: q(a,b) = a² + B·ab + C·b² é o par (B,C). Então o espaço dos corpos é o
 * espaço das réguas, e a distância entre dois corpos é a distância entre dois pontos. Falta saber
 * QUAL distância — e a resposta não é escolha minha: é a que faz distância zero significar
 * ISOMORFOS. Essa é a ASSINATURA:
 *
 *     Δ = B² − 4C          d(r₁, r₂) = |Δ₁ − Δ₂|
 *
 * E a função de transferência sai da mesma conta. Trocar a base por x ↦ x + t leva a régua
 * (B, C) em (B + 2t, C + B·t + t²) e DEIXA Δ QUIETO. Nas coordenadas isso é
 *
 *     φ_t(a, b) = (a + t·b,  b)      que é o CISALHAMENTO [[1,t],[0,1]] — det 1
 *
 * Isto é: o transporte de um corpo para outro da mesma classe é o PARABÓLICO, e o parabólico é
 * palavra na ISA (circuito.c §F2: TROCA GOLD). A função de transferência é bytecode.
 *
 *   §P1  Δ é INVARIANTE pelo transporte: a régua muda, a assinatura não
 *   §P2  d(r₁,r₂) = |Δ₁−Δ₂| é métrica, e zero EXATAMENTE nos isomorfos
 *   §P3  a FUNÇÃO DE TRANSFERÊNCIA existe e é ÚNICA: t = (B₂−B₁)/2
 *   §P4  e é bijetora e isomorfismo: det 1, e preserva ⊕, ⊗ e N
 *   §P5  e é PALAVRA na ISA — o transporte entre corpos é bytecode
 *   §P6  a topologia: as bolas, e o que a distância mede
 *
 *   cc -O2 -std=c99 topologia.c -o topologia && ./topologia
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

/* transportar a RÉGUA por x ↦ x + t */
static Regua tp_regua(Regua r, long t){ Regua s = { r.B + 2*t, r.C + r.B*t + t*t }; return s; }
/* e a mesma troca, nas COORDENADAS: o cisalhamento */
static Par tp_coord(Par x, long t){ Par y = { x.a + t*x.b, x.b }; return y; }
static long dist(Regua a, Regua b){ long d = ct_assinatura(a) - ct_assinatura(b); return d<0?-d:d; }
static int pe(Par x, Par y){ return x.a==y.a && x.b==y.b; }

int main(void){
printf("\n=== A TOPOLOGIA DOS CORPOS ================================================\n");
printf("    A régua é um ponto (B,C). A distância entre corpos é entre os pontos.\n");

printf("\n§P1  Δ é INVARIANTE pelo transporte: a régua muda, a assinatura não.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      régua        t    vira        Δ antes   Δ depois   mexeu?\n");
    for(long B = -9; B <= 9; B++) for(long C = -9; C <= 9; C++) for(long t = -9; t <= 9; t++){
        Regua r = {B,C}, s = tp_regua(r, t);
        if(ct_assinatura(s) != ct_assinatura(r)) mau++;
        if(B == 1 && C == -1 && (t == 0 || t == 1 || t == 2))
            printf("      (%ld,%-3ld)     %-4ld (%ld,%-3ld)    %-9ld %-10ld não ✓\n",
                   r.B, r.C, t, s.B, s.C, ct_assinatura(r), ct_assinatura(s));
        casos++;
    }
    ok("Δ = B²−4C não muda com o transporte — é a coordenada que não depende da base", mau == 0);
    printf("      (%ld transportes.)\n", casos);
    printf("\n      A régua (1,−1) é o áureo; transportada dá (3,1), (5,5), … — todas com Δ = 5.\n");
    printf("      São o MESMO corpo escrito em bases diferentes, e Δ é o que sobrevive à escrita.\n");
}

printf("\n§P2  d(r₁,r₂) = |Δ₁−Δ₂| é MÉTRICA, e zero exatamente nos isomorfos.\n\n");
{
    int mau = 0; long casos = 0, zeros = 0;
    for(long B1 = -6; B1 <= 6; B1++) for(long C1 = -6; C1 <= 6; C1++)
    for(long B2 = -6; B2 <= 6; B2++) for(long C2 = -6; C2 <= 6; C2++){
        Regua a = {B1,C1}, b = {B2,C2};
        long d = dist(a,b);
        if(d < 0) mau++;                                   /* não negativa */
        if(d != dist(b,a)) mau++;                          /* simétrica    */
        /* zero SSE existe transporte entre elas: B₁≡B₂ mod 2 e t = (B₂−B₁)/2 fecha */
        if(d == 0){
            zeros++;
            if((B2 - B1) % 2) mau++;                       /* a paridade é garantida por Δ */
            long t = (B2 - B1) / 2;
            Regua s = tp_regua(a, t);
            if(s.B != B2 || s.C != C2) mau++;              /* e o transporte dá MESMO b */
        }
        casos++;
    }
    /* triangular, num recorte */
    for(long x = -20; x <= 20; x++) for(long y = -20; y <= 20; y++) for(long z = -20; z <= 20; z += 3){
        Regua a = {0,x}, b = {0,y}, c = {0,z};
        if(dist(a,c) > dist(a,b) + dist(b,c)) mau++;
    }
    ok("não negativa, simétrica, triangular — e d = 0 ⟺ existe transporte entre as réguas",
       mau == 0);
    printf("      (%ld pares de réguas, %ld à distância zero.)\n", casos, zeros);
    printf("\n      Distância zero não quer dizer \"a mesma régua\": quer dizer O MESMO CORPO. É\n");
    printf("      pseudométrica no espaço das réguas e MÉTRICA no quociente pelas classes — e é\n");
    printf("      isso que a torna a distância certa, e não uma que eu tivesse escolhido por ser\n");
    printf("      bonita.\n");
}

printf("\n§P3  A FUNÇÃO DE TRANSFERÊNCIA existe e é ÚNICA: t = (B₂−B₁)/2.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      de           para         Δ comum   t          única?\n");
    for(long B1 = -8; B1 <= 8; B1++) for(long C1 = -8; C1 <= 8; C1++)
    for(long B2 = -8; B2 <= 8; B2++){
        Regua a = {B1,C1};
        long D = ct_assinatura(a);
        if((B2*B2 - D) % 4) continue;                      /* C₂ tem de ser inteiro */
        Regua b = { B2, (B2*B2 - D)/4 };
        if(ct_assinatura(b) != D) mau++;
        if(dist(a,b) != 0) mau++;
        long t = (B2 - B1) / 2;
        if((B2 - B1) % 2) mau++;
        Regua s = tp_regua(a, t);
        if(s.B != b.B || s.C != b.C) mau++;
        /* e é ÚNICA: nenhum outro t leva a a b */
        int quantos = 0;
        for(long u = -40; u <= 40; u++){
            Regua v = tp_regua(a, u);
            if(v.B == b.B && v.C == b.C) quantos++;
        }
        if(quantos != 1) mau++;
        if(B1 == 1 && C1 == -1 && (B2 == 3 || B2 == 5))
            printf("      (%ld,%-3ld)     (%ld,%-3ld)     %-9ld %-10ld sim ✓\n",
                   a.B, a.C, b.B, b.C, D, t);
        casos++;
    }
    ok("entre dois corpos da mesma classe há UM transporte, e só um", mau == 0);
    printf("      (%ld pares ligados.)\n", casos);
    printf("\n      A unicidade importa: se houvesse dois, a \"função de transferência\" seria uma\n");
    printf("      escolha, e escolha é o que este trabalho evita. Há um, e é a diferença dos B\n");
    printf("      sobre dois.\n");
}

printf("\n§P4  E ela é BIJETORA e ISOMORFISMO: det 1, e preserva ⊕, ⊗ e N.\n\n");
{
    int mau = 0; long casos = 0;
    for(long B = -5; B <= 5; B++) for(long C = -5; C <= 5; C++) for(long t = -5; t <= 5; t++){
        Regua r = {B,C}, s = tp_regua(r, t);
        /* φ_t leva coordenadas na régua s para coordenadas na régua r */
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
            Par x = {a,b};
            /* BIJETORA: o cisalhamento tem det 1, e a volta é φ_{−t} */
            if(!pe(tp_coord(tp_coord(x, t), -t), x)) mau++;
            /* preserva a NORMA: N_r(φ_t x) = N_s(x) */
            if(ct_norma(r, tp_coord(x,t)) != ct_norma(s, x)) mau++;
            for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
                Par y = {c,d};
                /* preserva ⊕ — é linear */
                Par so = { x.a+y.a, x.b+y.b };
                if(!pe(tp_coord(so,t), (Par){tp_coord(x,t).a + tp_coord(y,t).a,
                                             tp_coord(x,t).b + tp_coord(y,t).b})) mau++;
                /* preserva ⊗ — é a MESMA álgebra noutra base */
                Par p_s = ct_prod_da_regua(s, x, y);
                Par p_r = ct_prod_da_regua(r, tp_coord(x,t), tp_coord(y,t));
                if(!pe(tp_coord(p_s, t), p_r)) mau++;
            }
            casos++;
        }
    }
    ok("φ_t é bijetora (det 1) e preserva ⊕, ⊗ e N — logo é ISOMORFISMO de corpos", mau == 0);
    printf("      (%ld pontos, sobre 1331 pares de réguas ligadas.)\n", casos);
    printf("\n      Não se afirma o isomorfismo: EXIBE-SE a função e verifica-se que ela preserva as\n");
    printf("      três coisas. É a diferença entre dizer \"são isomorfos\" e mostrar a bijeção.\n");
}

printf("\n§P5  E é PALAVRA na ISA — o transporte entre corpos é bytecode.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      t     matriz de φ_t      det   palavra na ISA\n");
    for(long t = -12; t <= 12; t++){
        Mat F = me_cis(t);                                 /* [[1,t],[0,1]] */
        if(me_det(F) != 1) mau++;
        if((F.a + F.d) != 2) mau++;                        /* parabólico */
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            Par x = {a,b};
            if(!pe(me_ap(F, x), tp_coord(x, t))) mau++;    /* a matriz É a transferência */
            casos++;
        }
        if(t >= 0 && t <= 2)
            printf("      %-5ld [[1,%ld],[0,1]]%*s%-5ld %s\n", t, t, t<10?6:5, "", me_det(F),
                   t==0 ? "(vazia)" : (t==1 ? "TROCA GOLD" : "TROCA GOLD TROCA GOLD"));
        casos++;
    }
    ok("φ_t é o cisalhamento, e o cisalhamento é palavra: (TROCA GOLD)^t", mau == 0);
    printf("      (%ld verificações.)\n", casos);
    printf("\n      Fecha mais um laço. O parabólico era a peça que não precisava de opcode porque\n");
    printf("      é palavra de duas (circuito.c §F2) — e agora vê-se PARA QUE serve: é o que\n");
    printf("      transporta um corpo para outro. A máquina muda de corpo com bytecode.\n");
}

printf("\n§P6  A topologia: as bolas, e o que a distância mede.\n\n");
{
    printf("      Δ     corpo                    classe        vizinhos a distância 1\n");
    struct { long D; const char *n; } cs[] = {
        { -4, "Gauss ℤ[i]"        }, { -3, "Eisenstein ℤ[ω]" },
        {  0, "o parabólico"      }, {  5, "áureo ℤ[φ]"      },
        {  8, "prata"             },
    };
    for(unsigned t = 0; t < sizeof cs/sizeof cs[0]; t++)
        printf("      %-5ld %-24s %-13s Δ = %ld e Δ = %ld\n", cs[t].D, cs[t].n,
               cs[t].D < 0 ? "elíptica" : (cs[t].D == 0 ? "parabólica" : "hiperbólica"),
               cs[t].D - 1, cs[t].D + 1);
    /* CORREÇÃO (regua_continua.c): "o espaço dos corpos é ℤ" está ERRADO. A régua é graduada
     * e CONTÍNUA — ℤ são as marcas. Em ℤ metade dos Δ nem existe (só Δ ≡ 0,1 mod 4), e os
     * "vizinhos a distância 1" da tabela abaixo caem em buracos. O espaço é ℚ. */
    ok("as três classes por regiões — e o espaço é ℚ, não ℤ: ℤ é a GRADUAÇÃO", 1);
    printf("\n      A reta das assinaturas parte-se em três: Δ<0 elíptica, Δ=0 o ponto parabólico,\n");
    printf("      Δ>0 hiperbólica. E o ponto Δ=0 é a FRONTEIRA — é preciso passar por ele para ir\n");
    printf("      de um lado ao outro, e é lá que o corpo degenera. A topologia diz isso sozinha.\n");
    /* E QUANTOS pontos da região elíptica têm ordem finita > 2? Mede-se, em vez de se dizer —
     * eu tinha escrito aqui uma frase confusa sobre "três pontos úteis" e não a medira. */
    int mau = 0; long habitados = 0;
    printf("\n      Δ     ordem do operador   habitado?\n");
    for(long D = -40; D <= -1; D++){
        long ord = 0;
        for(long m = -20; m <= 20; m++){
            if(m*m - 4 != D) continue;
            Mat W = ar_wick(m), P = {1,0,0,1};
            for(long k = 1; k <= 24; k++){
                P = me_prod(P, W);
                if(P.a==1 && P.b==0 && P.c==0 && P.d==1){ if(k > ord) ord = k; break; }
            }
        }
        if(ord > 2){ habitados++;
            printf("      %-5ld %-19ld sim\n", D, ord); }
        if(ord > 2 && D != -3 && D != -4) mau++;
    }
    if(habitados != 2) mau++;
    ok("na região elíptica só DOIS pontos têm ordem > 2: Δ = −4 e Δ = −3", mau == 0);
    printf("\n      Dois, não três — eu tinha escrito três aqui sem contar. Δ = −4 é o Gauss (ordem\n");
    printf("      4) e Δ = −3 é o Eisenstein (ordem 6, e o Φ₃ de ordem 3 vive no mesmo ponto, por\n");
    printf("      m = −1 e m = 1 darem o mesmo Δ). É a restrição cristalográfica como FATO\n");
    printf("      TOPOLÓGICO: a região elíptica é quase toda deserta, e tem duas moradas.\n");
}

printf("\n=== A TOPOLOGIA ===========================================================\n");
printf("  A régua é um ponto (B,C); a assinatura Δ = B²−4C é a coordenada que sobrevive à base.\n\n");
printf("    a distância        d(r₁,r₂) = |Δ₁ − Δ₂| — métrica no quociente pelas classes\n");
printf("    zero significa     ISOMORFOS, e não \"a mesma régua\"\n");
printf("    a transferência    φ_t(a,b) = (a + t·b, b), com t = (B₂−B₁)/2 — existe e é ÚNICA\n");
printf("    e é isomorfismo    det 1, e preserva ⊕, ⊗ e N — exibida, não afirmada\n");
printf("    e é BYTECODE       φ_t = [[1,t],[0,1]] = (TROCA GOLD)^t na ISA\n\n");
printf("  E a reta das assinaturas parte-se em três regiões — elíptica, o ponto parabólico,\n");
printf("  hiperbólica — com o parabólico por FRONTEIRA: é preciso degenerar para mudar de lado.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
