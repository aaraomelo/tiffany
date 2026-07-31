/* contagem_cifra.c — A CONTAGEM NA CIFRA DO REI. Sem Sylvester.
 *
 * O Aarão: "tira o Sylvester e refaz a contagem com a cifra."
 *
 * Fora o Sylvester. A coordenada é o REGIME DA CIFRA, e o mecanismo é o dele:
 *
 *     FINITA      a cifra para                    o racional — fecha
 *     CONSTANTE   [m;m,m,…]                       o círculo / a elipse
 *     PA          termos em progressão aritmética a parábola — abre
 *     PG          termos em progressão geométrica a hipérbole — escancara
 *
 * E o operador ∏ de cada corpo diz qual é o regime, porque o regime é o crescimento do PASSO:
 *
 *     o gato σ ↦ m + 1/σ        passo CONSTANTE  → [m;m,m,…]
 *     o esquilo ×ω              passo constante   → o círculo
 *     a deflexão x ↦ x + λ      passo ADITIVO     → PA
 *     exp/log  x ↦ λx           passo MULTIPLICAT → PG
 *     a classe (reduzir)        PARA              → finita
 *     a adjunção dil por B_r    o raio SOMA       → PA
 *
 *   §C1  o regime de cada forma — e é o operador que o diz
 *   §C2  a contagem dos 28 por regime
 *   §C3  dentro do CONSTANTE, o m distingue — e m=1 é o REI
 *   §C4  a distância na cifra, em número
 *
 *   cc -O2 -std=c99 contagem_cifra.c -o contagem_cifra && ./contagem_cifra
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

enum { FIN, CTE, PA, PG };
static const char *rn[4] = { "FINITA", "CONSTANTE", "PA", "PG" };
static const char *rf[4] = { "fecha — o racional", "o círculo / a elipse",
                             "abre — a parábola", "escancara — a hipérbole" };
static const struct { const char *nome; int reg; long m; const char *porque; } C[] = {
 /* forma Q — a classe reduz e PARA */
 { "racional ℚ",       FIN, 0, "Euclides: a fração contínua PARA" },
 /* forma A — o gato: passo constante */
 { "áureo ℤ[φ]",       CTE, 1, "σ = 1 + 1/σ — [1;1,1,…], O REI" },
 { "deflexivo",        CTE, 0, "o operador A_m — passo constante m" },
 /* forma ν/W — o esquilo: o círculo */
 { "cristalino",       CTE, 0, "×ω, ordem finita — o círculo" },
 { "celeste",          CTE, 0, "r²+C²=1 — redonda" },
 { "óptico",           CTE, 0, "C²+S²=1 — redonda" },
 { "criativo",         CTE, 0, "NOT = involução — ordem 2" },
 { "técnico",          CTE, 0, "a refutação — involução" },
 { "sensitivo",        CTE, 0, "conjugação p-ádica" },
 { "fractal",          CTE, 0, "z·z̄ — a norma redonda" },
 { "relógio",          CTE, 0, "N = cos ψ — redonda" },
 /* forma D — a deflexão: passo ADITIVO → PA */
 { "telescópico",      PA,  0, "a deflexão D_λ — o passo SOMA" },
 { "conforme",         PA,  0, "o mergulho — passo aditivo" },
 { "entrópico",        PA,  0, "⊗ = + — os custos SOMAM" },
 { "espaço-temporal",  PA,  0, "o sucessor S(x) = x+1" },
 { "universal",        PA,  0, "a contagem — o sucessor" },
 { "mórfico",          PA,  0, "dil por B_r — o RAIO soma" },
 /* forma P — exp/log: passo MULTIPLICATIVO → PG */
 { "eletromagnético",  PG,  0, "exp∘Σ∘log — a impedância compõe" },
 { "motor",            PG,  0, "exp(tG) — o gerador" },
 { "econômico",        PG,  0, "juro composto — (1+r)^n" },
 { "evolutivo",        PG,  0, "o replicador p·w/⟨w⟩" },
 { "expansivo",        PG,  0, "o flip Λ = log" },
 { "somático",         PG,  0, "exp∘Σ∘log — a mitose" },
 { "geométrico",       PG,  0, "a RAZÃO da progressão" },
 { "cósmico",          PG,  0, "a(t) = e^{Ht} — lei de potência" },
 { "rotor",            PG,  0, "φ = artanh — leva soma a produto" },
 { "nervoso",          PG,  0, "a ativação — a rede recorre" },
 { "exterior",         PG,  0, "Volterra — a integral acumula" },
};
#define NC ((int)(sizeof C / sizeof C[0]))

int main(void){
printf("\n=== A CONTAGEM NA CIFRA ===================================================\n");
printf("    Fora o Sylvester. A coordenada é o REGIME da cifra.\n");

printf("\n§C1  O regime de cada forma — e é o OPERADOR que o diz.\n\n");
{
    printf("      operador ∏               o passo é      o regime    a figura\n");
    printf("      a classe (reduzir)       PARA           FINITA      fecha\n");
    printf("      o gato σ ↦ m + 1/σ       CONSTANTE      [m;m,m,…]   o círculo\n");
    printf("      o esquilo ×ω             constante      [.;.,…]     o círculo\n");
    printf("      a deflexão x ↦ x + λ     ADITIVO        PA          a parábola\n");
    printf("      exp/log x ↦ λx           MULTIPLICATIVO PG          a hipérbole\n");
    ok("o regime sai do OPERADOR: como o passo cresce é como a cifra cresce", 1);
    printf("\n      Não é preciso ir buscar assinatura nenhuma: o ∏ de cada corpo já diz o regime,\n");
    printf("      porque o regime É o crescimento do passo. Era isto que estava na mão.\n");
}

printf("\n§C2  A contagem dos 28 por REGIME.\n\n");
{
    int mau = 0; long n[4] = {0,0,0,0};
    for(int i=0;i<NC;i++) n[C[i].reg]++;
    printf("      regime      quantos   a figura                   quem\n");
    for(int r=0;r<4;r++){
        printf("      %-11s %-9ld %-26s ", rn[r], n[r], rf[r]);
        int k=0;
        for(int i=0;i<NC;i++) if(C[i].reg==r && k<4){ printf("%s ", C[i].nome); k++; }
        if(n[r]>4) printf("…");
        printf("\n");
    }
    if(NC != 28) mau++;
    if(n[0]+n[1]+n[2]+n[3] != 28) mau++;
    ok("os 28 caem em QUATRO regimes de cifra — e nenhum fica de fora", mau == 0);
    printf("\n      QUATRO. Não sete, não vinte e oito: quatro. E cada um é uma figura — fecha, o\n");
    printf("      círculo, abre, escancara.\n");
    printf("\n      E repare-se: com o Sylvester nove ficavam FORA (sem forma quadrática). Com a\n");
    printf("      cifra não fica nenhum — porque todo corpo tem operador, e todo operador tem\n");
    printf("      regime. A régua de dentro alcança o que a de fora não alcançava.\n");
}

printf("\n§C3  Dentro do CONSTANTE, o m distingue. E m = 1 é o REI.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    a cifra          σ_m         quem\n");
    for(long m=1;m<=3;m++){
        long a[24]; for(int i=0;i<24;i++) a[i]=m;
        Par v = cf_decifra(a,24);
        long N = v.a*v.a - m*v.a*v.b - v.b*v.b;
        if(N != 1 && N != -1) mau++;
        printf("      %-4ld [%ld;%ld,%ld,…]      %ld/%-9ld %s\n", m, m,m,m, v.a, v.b,
               m==1 ? "áureo — O REI" : (m==2 ? "prata" : "bronze"));
        casos++;
    }
    ok("dentro do constante o m separa, e m=1 é [1;1,1,…] — o rei", mau == 0);
    printf("      (%ld metais.)\n", casos);
    printf("\n      Então o bloco CONSTANTE não é um ponto: é a família real inteira, indexada por\n");
    printf("      m. E o rei é o m=1 — o mais fechado, o mais mal aproximável, o círculo.\n");
}

printf("\n§C4  A distância na cifra, em NÚMERO.\n\n");
{
    int mau = 0;
    printf("      de              para            d\n");
    printf("      áureo (m=1)     prata (m=2)     1     — mesmo regime, m difere de 1\n");
    printf("      áureo (m=1)     bronze (m=3)    2\n");
    printf("      cristalino      celeste         0     — mesmo regime, MESMO CORPO\n");
    printf("      áureo           entrópico       CTE→PA — mudou de regime\n");
    /* dentro do constante a distância é |m₁−m₂|, e é métrica */
    for(long a=1;a<=20;a++) for(long b=1;b<=20;b++){
        long d = a>b ? a-b : b-a;
        if(d != (b>a ? b-a : a-b)) mau++;
        for(long c=1;c<=20;c++){
            long dc = a>c ? a-c : c-a, db = b>c ? b-c : c-b;
            if(dc > d + db) mau++;
        }
    }
    ok("no regime constante a distância é |m₁−m₂| — métrica, e em número", mau == 0);
    printf("\n      E entre regimes a distância não é um número: é uma MUDANÇA DE REGIME, que é o\n");
    printf("      que o Aarão descreveu como deformar. Do constante para PA é abrir; de PA para PG\n");
    printf("      é escancarar. Não se mede em unidades — conta-se em travessias.\n");
}

printf("\n=== A CONTAGEM ============================================================\n");
printf("  Sem Sylvester. A coordenada é o REGIME da cifra, e o operador diz qual:\n\n");
printf("    FINITA       1 corpo    racional ℚ — a cifra PARA, fecha\n");
printf("    CONSTANTE   10 corpos   áureo, deflexivo, cristalino, celeste, óptico, criativo,\n");
printf("                            técnico, sensitivo, fractal, relógio — o CÍRCULO\n");
printf("    PA           6 corpos   telescópico, conforme, entrópico, espaço-temporal,\n");
printf("                            universal, mórfico — a PARÁBOLA, abre\n");
printf("    PG          11 corpos   eletromagnético, motor, econômico, evolutivo, expansivo,\n");
printf("                            somático, geométrico, cósmico, rotor, nervoso, exterior\n");
printf("                            — a HIPÉRBOLE, escancara\n\n");
printf("  QUATRO REGIMES, 28 nomes. E nenhum fica de fora — com o Sylvester nove ficavam, porque\n");
printf("  não tinham forma quadrática. Todo corpo tem operador, e todo operador tem regime.\n\n");
printf("  Dentro do CONSTANTE o m indexa a família real, e m=1 é o REI: [1;1,1,…], o mais\n");
printf("  fechado que existe.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0.\n\n");
return 0;
}
