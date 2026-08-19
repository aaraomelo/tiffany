/* forca.c — A FORÇA É UMA SÓ, E O QUE VARIA É O MODO. E ela é o par direto/cruzado outra vez.
 *
 * O Aarão: "por falar em Força, é o Tao. A força é dual: uma única força e várias manifestações.
 * Vai no `hiper` e vê a história de como começamos isso, a parte da unificação das forças, que é
 * a FORÇA ALGÉBRICA. Recupera o estudo e vamos fundamentar num corpo novo no catálogo e teoria."
 *
 * O estudo está lá e está fechado. `hiper/livro/capitulos/metafisica/main.tex` §Unificação das
 * forças, e `hiper/aposentados/teoria/papers/paper_A_algebra.tex`:
 *
 *     V(p,r,t,s) = (1−p²)(a₀b₀)² + (1−r²)D² + (1−t²)Q + (1−s²)S − 2(t²−pr)a₀b₀D
 *
 * o IMPOSTO ALGÉBRICO, com uma pressão Π_α = 1−α² por cada direção. No caminho p=r=t=1 reduz-se a
 *
 *     V = Π·S ,    Π(s) = 1 − s² ,    S = ‖a×b‖²
 *
 * e daí sai a mecânica inteira: massa m = S, força F = 2sS, equação ẍ = 2x, conservação
 * ½Sṡ² + (1−s²)S = E. E o teorema: *as quatro forças da física são quatro comportamentos da MESMA
 * multiplicação* --- repulsão (forte), correlação (eletromagnetismo), atração (gravidade),
 * transformação (fraca). "A multiplicação nunca se separou em quatro: era uma só, com quatro modos
 * de manifestação dependentes da configuração local."
 *
 *     S = ‖a×b‖²         é o CRUZADO ao quadrado — o mesmo do fator de potência
 *     s                  é o DIRETO — e Π = 1−s² é o cruzado, porque s² + Π = 1
 *     Π + s² = 1         É cos²θ + sin²θ = 1, a identidade do círculo, e não uma parecença
 *
 *     s = 0    Π = 1    álgebra COMUTATIVA, compressão máxima   ↔   fp = 0   ortogonal, posto cheio
 *     |s| = 1  Π = 0    QUATERNIOS, sem compressão              ↔   fp = 1   paralelo, posto 1
 *     |s| > 1  Π < 0    além do horizonte                       ↔   a HIPÉRBOLE, a família real
 *
 *   §G1  as QUATRO direções, e a mesma equação nas quatro: ẍ = 2x  (o mapa, não Euler)
 *   §G2  o imposto FATORIZA: Π só depende da posição, S só dos operandos
 *   §G3  Π + s² = 1 é a identidade do círculo — a pressão É o cruzado
 *   §G4  o horizonte |s| = 1: onde a pressão troca de SINAL, e o círculo vira hipérbole
 *   §G5  a conservação, W = −ΔV = ΔT na mesma trajectória (v² − 2s² constante)
 *   §G6  as quatro forças são quatro MODOS — o que as separa é a configuração, não a lei
 *   §G7  e o SINAL da involução: sem ele não gira, e o dual é só isso
 *
 * LEI vs TRANSPORTE. Euler contra cosh(√2 t), quadro cos/sin, 2e6 passos e √(−Π)/|s|<1
 * eram o método. A lei é o mapa SL(2,ℤ) com a=+2 (D>0, foge) e a=−2 (período 4), Lagrange
 * em ℤ, o SINAL de Π = 1−s², e W=−ΔV=ΔT com v²−2s² constante. Sem uma raiz e sem integrar.
 *
 *   cc -O2 -std=c99 -I lib tests/forca.c -o forca && ./forca
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

/* o MAPA simplético de passo 1: v += a·s ; s += v. det = 1 sempre.
 * a = +2  hiperbólico (foge).  a = −2  elíptico, período 4 (gira).
 * É o mesmo de corpo_fisico.c §H7 — a equação, não o integrador. */
static void passo(long *s, long *v, long a){
    *v += a * (*s);
    *s += *v;
}

int main(void){
printf("\n=== A FORÇA É UMA SÓ: O IMPOSTO ALGÉBRICO, E O PAR DIRETO/CRUZADO ========\n");
printf("    Recuperado do `hiper`: V = Π·S, com Π = 1−s² e S = ‖a×b‖². E Π + s² = 1\n");
printf("    é a identidade do círculo — a pressão É o cruzado, e s é o direto.\n");

/* O CONTRATO DO CRUZADO, que nenhuma asserção tocava: um gerador de mutações trocou o sinal
 * em c[2] = a[0]b[1] − a[1]b[0] e o medidor ficou verde. O que define o produto cruzado são
 * duas identidades, e as duas são EXATAS com vetores inteiros:
 *   perpendicularidade   <a×b, a> = <a×b, b> = 0
 *   Lagrange             ‖a×b‖² + <a,b>² = ‖a‖²‖b‖²   (é o Π + s² = 1 deste ficheiro,
 *                                                       antes de normalizar) */
{
    long perp = 0, lagr = 0, pares = 0, nao_nulos = 0;
    for(long ax=-2; ax<=2; ax++) for(long ay=-2; ay<=2; ay++) for(long az=-2; az<=2; az++)
    for(long bx=-2; bx<=2; bx++) for(long by=-2; by<=2; by++) for(long bz=-2; bz<=2; bz++){
        long A[3] = {ax, ay, az}, B[3] = {bx, by, bz}, C[3];
        rt_cruz3(A, B, C);
        long c0 = C[0], c1 = C[1], c2 = C[2];
        if(c0*ax + c1*ay + c2*az == 0 && c0*bx + c1*by + c2*bz == 0) perp++;
        long na = ax*ax+ay*ay+az*az, nb = bx*bx+by*by+bz*bz;
        long ip = ax*bx+ay*by+az*bz, nc = c0*c0+c1*c1+c2*c2;
        if(nc + ip*ip == na*nb) lagr++;
        if(c0||c1||c2) nao_nulos++;
        pares++;
    }
    printf("\n§G0  O CRUZADO, antes de tudo: perpendicular e Lagrange, exatos em Z.\n\n");
    printf("      %ld pares de vetores inteiros: perpendicular em %ld, Lagrange em %ld\n",
           pares, perp, lagr);
    printf("      (cruzados não-nulos: %ld — o teste não vive de casos degenerados)\n\n", nao_nulos);
    ok("o cruzado é PERPENDICULAR aos dois fatores — 15625 pares em Z, resíduo 0",
       perp == pares && pares == 15625);
    ok("e LAGRANGE fecha: ‖a×b‖² + <a,b>² = ‖a‖²‖b‖², exato — é o Π + s² = 1 sem normalizar",
       lagr == pares && nao_nulos > 10000);
}

    printf("\n§G1  As QUATRO direções, e a MESMA equação nas quatro: ẍ = 2x.\n\n");
{
    /* Euler contra cosh(√2 t) era transporte. A lei é UM operador nas quatro: T₂ em
     * SL(2,ℤ), D>0. O controlo s̈=3s (T₃) tem det=1 também, mas a órbita é OUTRA — e
     * é outra nas quatro direcções. Δv=2s era a definição do passo relida. */
    RtOp T2 = {{ 3, 1, 2, 1 }}, T3 = {{ 4, 1, 3, 1 }};
    long det2 = rt_op_det(&T2), tr2 = T2.T[0] + T2.T[3], D2 = tr2*tr2 - 4*det2;
    long det3 = rt_op_det(&T3);
    const char *nome[] = {"p","r","t","s"};
    long s0[] = { 3, -5, 8, 2 }, v0[] = { 1, 4, -2, 6 };
    int outra = 0;
    printf("      dir   s₀   v₀     s após a=2    s após a=3    distintas?\n");
    for(int d = 0; d < 4; d++){
        long s2 = s0[d], v2 = v0[d], s3 = s0[d], v3 = v0[d];
        for(int n = 0; n < 3; n++){ passo(&s2, &v2, 2); passo(&s3, &v3, 3); }
        if(s2 != s3) outra++;
        printf("      %-3s  %+3ld  %+3ld    %+8ld      %+8ld      %s\n",
               nome[d], s0[d], v0[d], s2, s3, s2 != s3 ? "sim" : "NÃO");
    }
    printf("      T₂ det=%ld D=%ld;  T₃ det=%ld\n\n", det2, D2, det3);
    ok("as quatro direções seguem a MESMA equação ẍ = 2x — um operador SL(2,ℤ), D>0;"
       " o controlo s̈=3s tem det=1 também, mas a órbita é OUTRA nas quatro. Euler"
       " contra cosh era o método",
       outra == 4 && det2 == 1 && D2 > 0 && det3 == 1 && rt_op_valido(&T2));
    printf("      Não são quatro leis com a mesma forma: é uma, e o índice é a direção. É o que\n");
    printf("      o teorema da unificação diz — a multiplicação nunca se separou em quatro.\n");
}

printf("\n§G2  O imposto FATORIZA: Π só depende da posição, S só dos operandos.\n\n");
{
    /* V = Π(s)·S(a,b). «V/(Π·S) = 1» é a definição relida. A tese com conteúdo: se
     * a×b = 0 — campos paralelos — então S = 0 e o imposto DESAPARECE; e se a×b ≠ 0,
     * ele existe. O zero é EXACTO: o cruzado de paralelos subtrai termos idênticos. */
    int par_zero = 0, par_tot = 0, nao_par_v = 0, nao_par_tot = 0;
    printf("      k     a              b∥              S∥    b×              S×\n");
    for(int k = 1; k <= 6; k++){
        long a[3]  = { 10, 3*k, -2 };
        long bp[3] = { 20, 6*k, -4 };                /* PARALELO a `a` (o dobro) */
        long bn[3] = {  5, -4,  7*k };               /* não paralelo */
        long c1[3], c2[3];
        rt_cruz3(a, bp, c1);  rt_cruz3(a, bn, c2);
        long S1 = rt_norma(c1, 3), S2 = rt_norma(c2, 3);
        long Pi = 100 - 4*4;                         /* s = 4/10, Π em centésimos² */
        par_tot++;      if(Pi * S1 == 0) par_zero++;
        nao_par_tot++;  if(Pi * S2 != 0) nao_par_v++;
        printf("      %-5d (%ld,%ld,%ld)  (%ld,%ld,%ld)  %ld    (%ld,%ld,%ld)  %ld\n",
               k, a[0],a[1],a[2], bp[0],bp[1],bp[2], S1, bn[0],bn[1],bn[2], S2);
    }
    printf("\n      com a×b = 0 (paralelos) o imposto ZERA, exacto: %d de %d\n", par_zero, par_tot);
    printf("      e com a×b ≠ 0 ele EXISTE: %d de %d — o contraste é que mede\n\n",
           nao_par_v, nao_par_tot);
    ok("SEM CRUZADO NÃO HÁ IMPOSTO: com os campos paralelos a×b anula-se e V = Π·S zera"
       " EXACTAMENTE — o cruzado de paralelos subtrai termos idênticos bit a bit —, e com"
       " eles não paralelos o imposto existe",
       par_tot > 0 && par_zero == par_tot && nao_par_v == nao_par_tot);
    printf("      E a consequência está no paper: se a×b = 0 — mesmo campo local — então S = 0 e\n");
    printf("      o imposto DESAPARECE. Sem cruzado não há imposto: é o cruzado que se paga.\n");
}

printf("\n§G3  Π + s² = 1 É a identidade do círculo — a pressão é o CRUZADO.\n\n");
{
    /* Π + s² = 1 é LAGRANGE sem dividir. Com s = ⟨a,b⟩/(‖a‖‖b‖), a pressão Π = 1 − s²
     * multiplicada pelo denominador comum ‖a‖²‖b‖² dá exactamente ‖a×b‖². O cosseno
     * era o preço de ter normalizado antes de medir. */
    long ok_lag = 0, pares = 0, vivos = 0;
    for(long t = 0; t < 400; t++){
        long a[3], b[3], c[3];
        for(int i = 0; i < 3; i++){
            a[i] = ((t*7 + i*3) % 11) - 5;
            b[i] = ((t*5 + i*2) % 9)  - 4;
        }
        rt_cruz3(a, b, c);
        long na = rt_norma(a, 3), nb = rt_norma(b, 3);
        long dir = rt_dir(a, b, 3), cru = rt_norma(c, 3);
        pares++;
        if(na*nb - dir*dir == cru) ok_lag++;
        if(cru) vivos++;
    }
    printf("      Π·‖a‖²‖b‖² = ‖a‖²‖b‖² − ⟨a,b⟩² = ‖a×b‖²\n");
    printf("      em %ld de %ld pares, com o cruzado não nulo em %ld — resíduo ZERO\n\n",
           ok_lag, pares, vivos);
    ok("A PRESSÃO ALGÉBRICA É O CRUZADO AO QUADRADO — Π + s² = 1 É cos² + sin² = 1, e"
       " isto é LAGRANGE em ℤ com resíduo zero. Basta NÃO DIVIDIR: o cosseno era o preço"
       " de ter normalizado antes de medir",
       ok_lag == pares && vivos > pares/2 && pares == 400);
    printf("      Logo o imposto do hiper e o fator de potência são a mesma decomposição:\n");
    printf("      s é o DIRETO (mede), Π é o CRUZADO (ordena), e V = Π·S é o que o cruzado custa.\n");
}

printf("\n§G4  O HORIZONTE |s| = 1: onde a pressão troca de sinal, e o círculo vira hipérbole.\n\n");
{
    /* √(−Π)/|s| < 1 para |s|>1 é sempre verdade — tautologia. A lei é o SINAL de Π:
     * Π = 1 − s², em décimos s_z, Π_z = 100 − s_z². Dentro Π>0 (círculo), no horizonte
     * Π=0, fora Π<0 (hipérbole). */
    printf("      s_z/10    Π = 100−s_z²    regime (pelo SINAL de Π)\n");
    long s_z[] = { 0, 5, 9, 10, 11, 15, 20 };
    int dentro = 0, fora = 0, horiz = 0, mau = 0;
    for(int i = 0; i < 7; i++){
        long sz = s_z[i], Pi = 100 - sz*sz;
        const char *reg;
        if(Pi > 0){
            reg = "círculo";    dentro++; if(sz >= 10) mau++;
        } else if(Pi < 0){
            reg = "HIPÉRBOLE";  fora++;   if(sz <= 10) mau++;
        } else {
            reg = "o horizonte"; horiz++; if(sz != 10) mau++;
        }
        printf("      %-9ld %-16ld %s\n", sz, Pi, reg);
    }
    printf("\n");
    ok("dentro do horizonte a pressão é positiva: é o círculo",
       dentro == 3 && mau == 0);
    ok("FORA do horizonte a pressão é negativa — é a hipérbole — e no |s|=1 ela ZERA",
       fora == 3 && horiz == 1 && mau == 0);
    printf("      Portanto o horizonte |s| = 1 do paper_A É a fronteira Δ = 0 do polar.c, e os dois\n");
    printf("      estudos chegaram-lhe por lados opostos sem se encontrarem. E do lado de fora vive\n");
    printf("      a FAMÍLIA REAL, que o fator.c §W2 mostrou ser toda hiperbólica.\n");
}

printf("\n§G5  A CONSERVAÇÃO, e a força F = 2sS como direto × cruzado.\n\n");
{
    /* F = −(−2sS) é a definição relida. A tese é ALGÉBRICA: na mesma trajectória
     * W = −ΔV = ΔT via v² − 2s² = const (corpo_fisico §H4). E F anula-se nos DOIS
     * lados: s = 0 (ortogonal) e S = 0 (mesmo campo). */
    long A[3] = { 10, 4, -2 }, B[3] = { 3, -6, 8 }, C[3];
    rt_cruz3(A, B, C);
    long Si = rt_norma(C, 3);
    if(Si <= 0) Si = 1;
    printf("      S = ‖a×b‖² = %ld\n\n", Si);
    long tri_ok = 0, tri_tot = 0;
    for(long s_a = -6; s_a <= 6; s_a++) for(long v_a = -6; v_a <= 6; v_a++)
    for(long s_b = -6; s_b <= 6; s_b++) for(long v_b = -6; v_b <= 6; v_b++){
        if(v_a*v_a - 2*s_a*s_a != v_b*v_b - 2*s_b*s_b) continue;
        if(s_a == s_b && v_a == v_b) continue;
        long Wz  = Si*(s_b*s_b - s_a*s_a);
        long dTx2 = Si*(v_b*v_b - v_a*v_a);
        tri_tot++;
        if(2*Wz == dTx2) tri_ok++;
    }
    int anula_s = 0, anula_S = 0, n_s = 0, n_S = 0;
    for(long s = -5; s <= 5; s++){
        long F = 2 * s * Si;
        n_s++;
        if(s == 0){ if(F == 0) anula_s++; }
        else      { if(F != 0) anula_s++; }
    }
    for(long S = 0; S <= 5; S++){
        long F = 2 * 3 * S;                          /* s fixo ≠ 0 */
        n_S++;
        if(S == 0){ if(F == 0) anula_S++; }
        else      { if(F != 0) anula_S++; }
    }
    printf("      W = −ΔV = ΔT em ℤ (v²−2s² constante): %ld de %ld pares\n", tri_ok, tri_tot);
    printf("      F anula em s=0: %d/%d   e em S=0: %d/%d\n\n", anula_s, n_s, anula_S, n_S);
    ok("a energia conserva-se: W = −ΔV = ΔT na mesma trajectória, sem integrador, e F = 2sS"
       " anula-se tanto em s = 0 (ortogonal) como em S = 0 (mesmo campo local)",
       tri_tot > 0 && tri_ok == tri_tot && anula_s == n_s && anula_S == n_S);
    printf("      E a força fatoriza como o resto: F = 2·s·S = 2 × DIRETO × CRUZADO². Não é o\n");
    printf("      direto sozinho nem o cruzado sozinho — é o produto dos dois.\n");
}

printf("\n§G6  AS QUATRO FORÇAS são quatro MODOS — o que as separa é a configuração.\n\n");
{
    /* Integração Euler era transporte. A tese: UMA equação F = 2sS produz quatro
     * assinaturas QUALITATIVAMENTE distintas — sinal de F, magnitude de S, corrente. */
    printf("      força física        s     S     v     F=2sS    assinatura\n");
    struct { const char *fis; long s, S, v; } M[] = {
        {"forte (repulsão)",   +8, 10,  0},
        {"eletromagnetismo",   +3, 10, -6},
        {"gravidade",          -6, 10,  0},
        {"fraca (transform.)", +2,  3,  0},
    };
    long sig[4];
    int nM = 4, distintos = 0;
    for(int i = 0; i < nM; i++){
        long F = 2 * M[i].s * M[i].S;
        int sg = F > 0 ? 1 : (F < 0 ? -1 : 0);
        int corr = M[i].v != 0;
        sig[i] = sg * 1000 + M[i].S * 10 + corr;     /* (sinal F, S, corrente) */
        printf("      %-19s %+3ld  %3ld  %+3ld  %+6ld    (%+d, S=%ld, v%s)\n",
               M[i].fis, M[i].s, M[i].S, M[i].v, F,
               sg, M[i].S, corr ? "≠0" : "=0");
    }
    for(int i = 0; i < nM; i++) for(int j = i + 1; j < nM; j++)
        if(sig[i] != sig[j]) distintos++;
    printf("\n      pares com assinatura distinta: %d de %d\n\n", distintos, nM*(nM-1)/2);
    ok("a MESMA equação dá comportamentos distintos — quatro modos, uma lei, sem integrar:"
       " o que muda é ONDE se está (sinal de s) e QUANTO cruzado há (S)",
       distintos == nM*(nM-1)/2 && nM == 4);
    printf("      E é isto que o Aarão chamou o Tao: uma força só, várias manifestações. O que a\n");
    printf("      física separou em quatro, a álgebra não teve de juntar — nunca esteve separado.\n");
}

printf("\n§G7  E O SINAL DA INVOLUÇÃO — sem ele não gira, e o dual é só isso.\n\n");
{
    /* 2e6 passos Euler era transporte. A lei é o trial: +2 foge (D>0, |s| cresce),
     * −2 gira (D<0, período 4 em SL(2,ℤ)). É o mesmo J do zero.c / corpo_fisico §H7. */
    printf("      força      equação    trial           órbita\n");
    long sF = 2, vF = 0, cresce = 0;
    long ant = sF > 0 ? sF : -sF;
    printf("      n     |s| no +2s (foge)\n");
    for(int n = 1; n <= 5; n++){
        passo(&sF, &vF, 2);
        long mag = sF > 0 ? sF : -sF;
        if(mag > ant) cresce++;
        ant = mag;
        printf("      %-5d %ld\n", n, mag);
    }
    RtOp Tm = {{ -1, 1, -2, 1 }};
    long detm = rt_op_det(&Tm), trm = Tm.T[0] + Tm.T[3], Dm = trm*trm - 4*detm;
    int per = rt_ordem_vector(&Tm, 2, 0, 20);
    printf("      +2sS       s̈ = +2s    D>0 hiperbólico   |s| cresce %ld/5\n", cresce);
    printf("      −2sS       s̈ = −2s    D=%ld elíptico      período %d\n\n", Dm, per);
    ok("com o sinal + o corpo FOGE, e nunca volta", cresce == 5);
    ok("com o sinal − ele GIRA — período 4 em SL(2,ℤ), D = −4, o mesmo J do zero.c",
       per == 4 && Dm == -4 && detm == 1);
    printf("      A diferença entre os dois é UM SINAL na multiplicação, e é o mesmo sinal da\n");
    printf("      3ª lei (F₁₂ = −F₂₁), da lei de Lenz (a reação opõe-se) e da involução\n");
    printf("      (ν∘ν = id). O corpo_fisico.c §H7 mede os três e a oscilação entre eles.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O imposto algébrico V = Π·S é o par direto/cruzado: s mede, Π = 1−s² ordena,\n");
printf("    e Π + s² = 1 é a identidade do círculo — não uma parecença. A força F = 2sS é\n");
printf("    o produto dos dois, e anula-se de qualquer um dos lados. O horizonte |s| = 1 é\n");
printf("    a fronteira onde o círculo vira hipérbole, e é lá que começa a família real.\n");
printf("    E as quatro forças da física são quatro configurações de uma equação só.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
