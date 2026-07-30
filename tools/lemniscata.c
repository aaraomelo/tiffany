/* lemniscata.c — π SE DOBRA NA LEMNISCATA, E O AGM É O FATOR DE COSTURA.
 *
 * A linha, e ela fecha o que agm.c e estelar.c já tinham em pedaços (fonte: corpo_estelar.tex,
 * prop:agm). O círculo é a forma sem deformação; a LEMNISCATA é o círculo DOBRADO --- e ela tem o seu
 * próprio π:
 *
 *      ϖ = π·G = π / M(1,√2) = 2,6220575542921198…  = 2∫₀¹ dt/√(1−t⁴)
 *
 * com G = 1/M(1,√2) a constante de Gauss. Então π NÃO gera ϖ por analogia: gera por um FATOR, e o
 * fator é o AGM. É isso que ``planificar/costurar'' quer dizer aqui, e é geral, não um caso:
 *
 *      K(k) = π / (2·M(1,k'))      para todo módulo k,  k' = √(1−k²)
 *
 * — um ÚNICO fator (o AGM) costura π a todos os períodos elípticos. O círculo é k=0 (o AGM é trivial,
 * M(1,1)=1, e K=π/2: o π puro); a lemniscata é k=1/√2, que é EXATAMENTE a primeira âncora τ=1 medida
 * em agm.c §A4. A escada de deformação das curvas e a escada dos singular values são a mesma.
 *
 * Mede-se: (L1) ϖ pelas duas vias --- π/M(1,√2) e a integral --- e o fator G;
 *          (L2) K(k) = π/(2M(1,k')) contra quadratura, em vários k: o AGM costura TODOS;
 *          (L3) a lemniscata é a âncora τ=1 (K'=K), e o círculo é o limite k→0;
 *          (L4) a DOBRA: perímetro do círculo 2π, da lemniscata 2ϖ, e a razão é o AGM.
 *
 *   cc -O2 -std=c99 lemniscata.c -lm -o lemniscata && ./lemniscata
 */
#include <stdio.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
/* π em long double COM DÍGITOS: M_PI é macro double (~16 dígitos) e, convertida, já entra com erro
 * ~1e-17 — foi ela que fez as duas primeiras medidas deste arquivo falharem, não o AGM.        */
#define PI_L 3.14159265358979323846264338327950288L

static int ok = 1;
typedef long double LD;

/* o AGM: ⊕ e ⊗ alternados (agm.c) */
static LD agm(LD a, LD b){
    for(int i=0;i<80 && fabsl(a-b)>1e-19L;i++){ LD na=(a+b)/2, nb=sqrtl(a*b); a=na; b=nb; }
    return (a+b)/2;
}
/* K(k) = ∫₀^{π/2} dθ/√(1−k²sin²θ), por trapézio (integrando suave para k<1) */
static LD Kquad(LD k, int N){
    LD h=(PI_L/2)/N, s=0;
    for(int i=0;i<=N;i++){
        LD th=i*h, sn=sinl(th);
        LD f=1.0L/sqrtl(1.0L - k*k*sn*sn);
        s += (i==0||i==N)? f/2 : f;
    }
    return s*h;
}
/* ∫₀^{π/2} dφ/√(1+sin²φ) — a integral da lemniscata, já suavizada por t=sinφ */
static LD lemni_quad(int N){
    LD h=(PI_L/2)/N, s=0;
    for(int i=0;i<=N;i++){
        LD ph=i*h, sn=sinl(ph);
        LD f=1.0L/sqrtl(1.0L + sn*sn);
        s += (i==0||i==N)? f/2 : f;
    }
    return s*h;
}

int main(void){
    printf("LEMNISCATA — π se dobra, e o AGM é o fator de costura\n");
    printf("=================================================================\n");

    const LD PI = PI_L;
    const LD VARPI = 2.62205755429211981046483958989111941368L;   /* ϖ, o π da lemniscata          */

    /* ---------- L1: ϖ pelas duas vias ---------- */
    printf("§L1  ϖ = π/M(1,√2) = 2∫₀¹dt/√(1−t⁴) — o π DOBRADO, e o fator é o AGM\n");
    {
        LD M = agm(1.0L, sqrtl(2.0L));
        LD G = 1.0L/M;                                  /* a constante de Gauss                    */
        LD via_agm = PI*G;
        LD via_int = 2.0L*lemni_quad(1<<16);
        LD e1 = fabsl(via_agm-VARPI), e2 = fabsl(via_int-VARPI);
        printf("       M(1,√2)      = %.20Lf\n", M);
        printf("       G = 1/M      = %.20Lf   (a constante de Gauss)\n", G);
        printf("       π·G          = %.20Lf   erro vs ϖ %.2Le %s\n", via_agm, e1, e1<1e-17L?"✓":"✗");
        printf("       2∫₀¹dt/√(1−t⁴) = %.20Lf   erro vs ϖ %.2Le %s\n", via_int, e2, e2<1e-16L?"✓":"✗");
        if(e1>=1e-17L || e2>=1e-16L) ok=0;
        printf("     %s\n", (e1<1e-17L&&e2<1e-16L)?
          "resíduo 0 — as duas vias dão o mesmo ϖ: a geométrica (a integral da lemniscata) e a do\n"
          "     AGM (π vezes a constante de Gauss). π não \"parece\" gerar ϖ: gera por um FATOR, e o\n"
          "     fator é o AGM."
          :"FALHA");
    }

    /* ---------- L2: o AGM costura π a TODOS os períodos ---------- */
    printf("\n§L2  a COSTURA é geral: K(k) = π/(2·M(1,k')) para todo k — um só fator\n");
    {
        int erro=0;
        printf("       k          K(k) por quadratura     π/(2·M(1,k'))          erro\n");
        LD ks[] = {0.0L, 0.25L, 0.5L, 0.70710678118654752440L, 0.8L, 0.95L};
        for(int t=0;t<6;t++){
            LD k=ks[t], kp=sqrtl(1.0L-k*k);
            LD Kq = Kquad(k, 1<<16);
            LD Ka = PI/(2.0L*agm(1.0L,kp));
            LD e = fabsl(Kq-Ka);
            const char *nota = (t==0)?"  ← o CÍRCULO (k=0): M(1,1)=1, K=π/2, o π puro"
                             : (t==3)?"  ← a LEMNISCATA (k=1/√2): a âncora τ=1"
                             : "";
            printf("       %.8Lf %.18Lf  %.18Lf  %.1Le%s\n", k, Kq, Ka, e, nota);
            if(e>1e-15L) erro=1;
        }
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — o AGM não costura só a lemniscata: costura π a QUALQUER curva da família. É\n"
          "     esse o sentido de planificar — um fator único aplaina toda a escada de deformação em π.");
        if(erro) ok=0;
    }

    /* ---------- L3: a lemniscata É a âncora τ=1 ---------- */
    printf("\n§L3  e a lemniscata é EXATAMENTE a primeira âncora: k=1/√2 dá K' = K, isto é τ=1\n");
    {
        LD k = 1.0L/sqrtl(2.0L), kp = sqrtl(1.0L-k*k);
        LD K = PI/(2.0L*agm(1.0L,kp)), Kl = PI/(2.0L*agm(1.0L,k));
        LD tau = Kl/K;
        printf("       k = 1/√2 = %.18Lf : k' = %.18Lf  (iguais: k=k')\n", k, kp);
        printf("       K = %.18Lf ; K' = %.18Lf ; τ = K'/K = %.18Lf\n", K, Kl, tau);
        printf("       e ϖ/K : %.18Lf   (ϖ = √2·K, pois a lemniscata é o caso k=k')\n", VARPI/K);
        int bom = fabsl(tau-1.0L)<1e-17L;
        printf("     %s\n", bom?
          "resíduo 0 — τ=1 exato: a lemniscata é o singular value que agm.c §A4 já achava por\n"
          "     bisseção (k=1/√2, resíduo 0). A escada das CURVAS deformadas e a escada dos SINGULAR\n"
          "     VALUES são a mesma escada — e a lemniscata é o primeiro degrau depois do círculo."
          :"FALHA");
        if(!bom) ok=0;
    }

    /* ---------- L4: a dobra, e a razão é o AGM ---------- */
    printf("\n§L4  a DOBRA: o círculo mede 2π, a lemniscata mede 2ϖ, e a razão é o AGM\n");
    {
        LD M = agm(1.0L,sqrtl(2.0L));
        LD razao = VARPI/PI, inv = PI/VARPI;
        LD e = fabsl(inv - M);
        printf("       2π  = %.18Lf   (o círculo: k=0, sem deformação)\n", 2*PI);
        printf("       2ϖ  = %.18Lf   (a lemniscata: k=1/√2, dobrado)\n", 2*VARPI);
        printf("       ϖ/π = %.18Lf = G  ;  π/ϖ = %.18Lf = M(1,√2)  erro %.1Le %s\n",
               razao, inv, e, e<1e-17L?"✓":"✗");
        if(e>=1e-17L) ok=0;
        printf("     %s\n", (e<1e-17L)?
          "resíduo 0 — a razão entre a medida da lemniscata e a do círculo É o AGM. Dobrar o círculo\n"
          "     custa exatamente um M(1,√2), e desdobrar custa o seu inverso: a costura tem preço, e\n"
          "     o preço é a média que alterna ⊕ e ⊗."
          :"FALHA");
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — a linha fecha, e ela liga o que estava solto.\n"
      "\n"
      "π SE DOBRA: o círculo (k=0) mede 2π; a lemniscata (k=1/√2), que é o círculo deformado, mede\n"
      "2ϖ com ϖ = π/M(1,√2) = 2,6220575… — verificado pelas duas vias, a integral da curva e o AGM.\n"
      "\n"
      "E O AGM É O FATOR DE COSTURA, não um truque de cálculo: K(k) = π/(2M(1,k')) vale para TODO\n"
      "módulo k (medido em seis, erro ≤1e-15). Um único fator aplaina a escada inteira de curvas\n"
      "deformadas em π — é isso que planificar quer dizer. O preço de dobrar o círculo até a\n"
      "lemniscata é exatamente M(1,√2), e o de desdobrar é o seu inverso G.\n"
      "\n"
      "E o degrau tem nome já medido: k=1/√2 dá K'=K, isto é τ=1 — a PRIMEIRA ÂNCORA que agm.c §A4\n"
      "achava por bisseção. A escada das curvas deformadas e a escada dos singular values são a\n"
      "mesma, e a lemniscata é o primeiro degrau depois do círculo. O AGM costura os dois."
      : "FALHOU — rever");
    return !ok;
}
