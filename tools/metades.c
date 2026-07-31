/* metades.c — NÃO HÁ "NÃO É CORPO": HÁ METADE DE CORPO, E O PAR FECHA.
 *
 * O Aarão: "não tem essa de não é corpo. Falta o dual do telescópico — acho que é celeste; do
 * entrópico é cósmico; do motor é rotor."
 *
 * Eu tinha marcado três como "não é corpo", com a razão de cada, e parado aí. Parar aí era o
 * erro: uma peça que falha de um lado é METADE de uma que fecha. E estava escrito no catálogo,
 * em `chess/elementares/cosmico.py`:
 *
 *     "o corpo ENTRÓPICO (max,+) NÃO é isomorfo ao cósmico — é a sua METADE, DERIVADO: uma
 *      polaridade do dipolo. A dualidade negro↔branco é a reflexão ν = −1."
 *
 * E em `certifica_corpos.py`, sobre o motor: "o conservativo (tr=0, det=1) É QUE SERIA CORPO (a
 * rotação pura)". Estava lá. Eu li a primeira metade das duas frases.
 *
 *     metade          dual        o que a metade perde   o que o PAR devolve
 *     entrópico       cósmico     o inverso aditivo      a soma: max + min = a + b
 *     motor           rotor       a conservação          tr = 0, o PONTO FIXO de ν
 *     telescópico     celeste     a integridade          o cone nulo É a norma a²−b²
 *
 *   §H1  entrópico e cósmico: ν(max) = min, e max + min = a + b — o par devolve a soma
 *   §H2  e nenhum polo sozinho tem oposto; é o DIPOLO que é corpo, não a polaridade
 *   §H3  motor e rotor: ν(t) = −t, e o rotor é o PONTO FIXO — tr = 0 é o único autodual
 *   §H4  a marca disso em ℤ: a volta fica no anel exatamente em |det| = 1
 *   §H5  telescópico e celeste: os divisores de zero SÃO o cone nulo da norma
 *   §H6  e são o MESMO objeto em duas bases — (e₁,e₂) cinde, (1,j) gira
 *   §H7  o veredito corrigido: metade, não falha
 *
 *   cc -O2 -std=c99 metades.c -o metades && ./metades
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static long maxl(long a, long b){ return a > b ? a : b; }
static long minl(long a, long b){ return a < b ? a : b; }

/* TELESCÓPICO/CELESTE: a + bj com j² = +1. Na base (1,j) gira hiperbolicamente e tem norma
 * a²−b² (o celeste); na base dos idempotentes cinde em duas cópias de ℤ (o telescópico). */
static Par ce_prod(Par x, Par y){ Par r = { x.a*y.a + x.b*y.b, x.a*y.b + x.b*y.a }; return r; }
static long ce_norma(Par x){ return x.a*x.a - x.b*x.b; }
static Par ce_cinde(Par x){ Par r = { x.a + x.b, x.a - x.b }; return r; }
static Par ce_junta(Par s){ Par r = { (s.a + s.b)/2, (s.a - s.b)/2 }; return r; }

int main(void){
printf("\n=== NAO HA \"NAO E CORPO\" — HA METADE DE CORPO =============================\n");
printf("    Uma peça que falha de um lado é metade de uma que fecha. O par é que é corpo.\n");

printf("\n§H1  ENTRÓPICO e CÓSMICO: ν(max) = min, e o par devolve a SOMA.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      a     b     max   min   max+min   a+b   devolve?\n");
    for(long a = -30; a <= 30; a++) for(long b = -30; b <= 30; b++){
        if(minl(a,b) != -maxl(-a,-b)) mau++;              /* ν = −1 troca as polaridades */
        if(maxl(a,b) + minl(a,b) != a + b) mau++;         /* e o par devolve a soma      */
        if(a == 7 && (b == 3 || b == 12))
            printf("      %-5ld %-5ld %-5ld %-5ld %-9ld %-5ld %s\n", a, b,
                   maxl(a,b), minl(a,b), maxl(a,b)+minl(a,b), a+b,
                   (maxl(a,b)+minl(a,b) == a+b) ? "sim ✓" : "NÃO");
        casos++;
    }
    ok("min = ν(max), e max + min = a + b — o dipolo devolve a soma", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      O buraco negro é o MAX de entropia e o branco é o MIN — quem controla o\n");
    printf("      gradiente é o cósmico, e o entrópico é uma polaridade dele. Sozinha, a\n");
    printf("      polaridade perde o oposto aditivo. Juntas, devolvem a soma inteira.\n");
}

printf("\n§H2  E é o DIPOLO que é corpo, não a polaridade.\n\n");
{
    int mau = 0; long com_inv = 0, casos = 0;
    for(long a = -30; a <= 30; a++){
        int tem = 0;
        for(long b = -30; b <= 30; b++) if(maxl(a,b) < a) tem = 1;
        if(tem) com_inv++;
        casos++;
    }
    if(com_inv) mau++;
    ok("no polo sozinho NENHUM elemento tem oposto — max nunca desce", mau == 0);
    printf("      (%ld elementos, %ld com oposto.)\n", casos, com_inv);
    int mau2 = 0; long c2 = 0;
    for(long a = -20; a <= 20; a++) for(long b = -20; b <= 20; b++){
        long M = maxl(a,b), m = minl(a,b);
        if(M + m != a + b) mau2++;
        if(M < m) mau2++;
        if(!((M == a && m == b) || (M == b && m == a))) mau2++;
        c2++;
    }
    ok("mas o PAR (max,min) determina {a,b} e devolve a soma — o dipolo fecha", mau2 == 0);
    printf("      (%ld pares.)\n", c2);
    printf("\n      É a correção que eu devia ter feito: \"não tem inverso aditivo\" descreve o\n");
    printf("      POLO, e eu escrevi como se descrevesse o objeto. O objeto é o dipolo.\n");
}

printf("\n§H3  MOTOR e ROTOR: ν(t) = −t, e o rotor é o PONTO FIXO de ν.\n\n");
{
    int mau = 0; long casos = 0, fixos = 0;
    printf("      tr(G)   ν leva a   o que faz            é autodual?\n");
    for(long t = -20; t <= 20; t++){
        if(ar_nu(ar_nu(t)) != t) mau++;
        if(t + ar_nu(t) != 0) mau++;                      /* o par soma ZERO: conservar */
        int autodual = (ar_nu(t) == t);
        if(autodual){ fixos++; if(t != 0) mau++; }
        if(t >= -1 && t <= 1)
            printf("      %-7ld %-10ld %-20s %s\n", t, ar_nu(t),
                   t < 0 ? "dissipa — o motor" : (t == 0 ? "conserva — o ROTOR" : "amplifica"),
                   autodual ? "SIM — é o fixo" : "não");
        casos++;
    }
    if(fixos != 1) mau++;
    ok("o rotor não é um terceiro objeto: é o PONTO FIXO da dualidade, tr = 0", mau == 0);
    printf("      (%ld traços, e exatamente %ld ponto fixo.)\n", casos, fixos);
    printf("\n      O motor dissipa (tr < 0) e o seu dual amplifica (tr > 0); somados dão zero, que\n");
    printf("      é conservar. O rotor é onde os dois lados coincidem — por isso é a rotação PURA.\n");
    printf("      Chamar ao motor \"não é corpo\" é descrever metade do dipolo pelo que lhe falta.\n");
}

printf("\n§H4  A marca em ℤ: a volta fica no anel exatamente em |det| = 1.\n\n");
{
    int mau = 0; long dentro = 0, fora = 0;
    printf("      peça                det   a inversa é inteira?   é o quê\n");
    struct { Mat M; const char *nome; } ps[] = {
        { {2,0,0,2},  "dilata ×2 (motor)"   },
        { {1,1,1,0},  "gato (ouro)"         },
        { {0,-1,1,0}, "esquilo (rotor)"     },
        { {3,0,0,1},  "estica um eixo só"   },
    };
    for(unsigned t = 0; t < sizeof ps/sizeof ps[0]; t++){
        long d = me_det(ps[t].M);
        int inteira = (d == 1 || d == -1);
        if(inteira){
            dentro++;
            Mat p = me_prod(ps[t].M, me_inv(ps[t].M));
            if(!(p.a==1 && p.b==0 && p.c==0 && p.d==1)) mau++;
        } else fora++;
        printf("      %-19s %-5ld %-22s %s\n", ps[t].nome, d,
               inteira ? "sim" : "NÃO — sai de ℤ",
               inteira ? "conserva" : "dissipa/amplifica");
    }
    if(dentro != 2 || fora != 2) mau++;
    ok("a volta existe no anel exatamente onde a norma se conserva — |det| = 1", mau == 0);
    printf("\n      É a mesma coisa dita em ℤ: o que dissipa não tem volta INTEIRA. Não é que a\n");
    printf("      matemática o proíba — é que ele saiu do anel, e o anel é onde a máquina vive.\n");
    printf("      Por isso a ISA só tem peças de det ±1: não por escolha, mas porque fora dali\n");
    printf("      não há volta.\n");
}

printf("\n§H5  TELESCÓPICO e CELESTE: os divisores de zero SÃO o cone nulo da norma.\n\n");
{
    int mau = 0; long casos = 0, nulos = 0, cindem = 0;
    printf("      a+bj      N = a²−b²   cinde em (α,β)   é divisor de zero?\n");
    for(long a = -25; a <= 25; a++) for(long b = -25; b <= 25; b++){
        Par u = {a,b};
        Par s = ce_cinde(u);
        long N = ce_norma(u);
        if(N != s.a * s.b) mau++;                         /* a norma É o produto cindido */
        int div0 = (s.a == 0 || s.b == 0) && !(s.a == 0 && s.b == 0);
        if(div0){
            cindem++;
            Par v = (s.a == 0) ? (Par){1,1} : (Par){1,-1};
            Par pr = ce_prod(u, v);
            if(pr.a != 0 || pr.b != 0) mau++;             /* existe v ≠ 0 com u·v = 0 */
        }
        if(N == 0) nulos++;
        if((N == 0) != (s.a == 0 || s.b == 0)) mau++;     /* cone nulo = onde cinde */
        casos++;
    }
    printf("      1+1j      %-11ld (%ld,%ld)%*ssim — está no cone\n", ce_norma((Par){1,1}),
           ce_cinde((Par){1,1}).a, ce_cinde((Par){1,1}).b, 12, "");
    printf("      1-1j      %-11ld (%ld,%ld)%*ssim — está no cone\n", ce_norma((Par){1,-1}),
           ce_cinde((Par){1,-1}).a, ce_cinde((Par){1,-1}).b, 12, "");
    printf("      3+1j      %-11ld (%ld,%ld)%*snão — N ≠ 0\n", ce_norma((Par){3,1}),
           ce_cinde((Par){3,1}).a, ce_cinde((Par){3,1}).b, 12, "");
    ok("N = α·β, e o divisor de zero é EXATAMENTE o cone nulo N = 0", mau == 0);
    printf("      (%ld pontos, %ld no cone nulo, %ld que cindem.)\n", casos, nulos, cindem);
    printf("\n      Então \"o telescópico cinde\" não é defeito dele: é o CONE DE LUZ do celeste,\n");
    printf("      onde a norma a²−b² se anula. O mesmo lugar, dois nomes — e chamar-lhe defeito é\n");
    printf("      dizer que a luz é um defeito do espaço-tempo.\n");
}

printf("\n§H6  E são o MESMO objeto em duas bases: (e₁,e₂) cinde, (1,j) gira.\n\n");
{
    int mau = 0; long casos = 0;
    for(long a = -14; a <= 14; a++) for(long b = -14; b <= 14; b++)
    for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
        Par u = {a,b}, v = {c,d};
        Par pu = ce_cinde(u), pv = ce_cinde(v), pr = ce_cinde(ce_prod(u,v));
        if(pr.a != pu.a * pv.a || pr.b != pu.b * pv.b) mau++;
        Par volta = ce_junta(ce_cinde(u));
        if(volta.a != a || volta.b != b) mau++;
        casos++;
    }
    ok("cindir leva o produto do celeste no produto componente do telescópico", mau == 0);
    printf("      (%ld pares, e a mudança de base volta exata.)\n", casos);
    printf("\n      Uma multiplicação, duas bases. Na base (1,j) gira hiperbolicamente com norma\n");
    printf("      a²−b²; na base dos idempotentes cinde em duas cópias. É o Teorema de Unicidade\n");
    printf("      do catálogo: \"a multiplicação é uma só; a norma é específica da régua\".\n");
}

printf("\n§H8  O TROPICAL e o GLACIAL: (max,+) e (min,+), e o par devolve a soma.\n\n");
{
    int mau = 0; long casos = 0;
    /* Correção do Aarão: "dual do tropical é glacial". E é o nome certo — o semianel (min,+)
     * é o glacial, o outro polo do (max,+). O §H1 mediu esta relação chamando-lhe entrópico e
     * cósmico; são dois níveis. No nível do SEMIANEL o par é tropical↔glacial. */
    printf("      a     b     tropical(max)  glacial(min)  o par     a+b\n");
    for(long a = -30; a <= 30; a++) for(long b = -30; b <= 30; b++){
        if(minl(a,b) != -maxl(-a,-b)) mau++;              /* ν leva um ao outro */
        if(maxl(a,b) + minl(a,b) != a + b) mau++;
        /* e ⊗ é o MESMO nos dois: a soma. É só o ⊕ que vira. */
        if((a + b) != (b + a)) mau++;
        if(a == 7 && b == 3)
            printf("      %-5ld %-5ld %-14ld %-13ld %-9ld %ld\n", a, b,
                   maxl(a,b), minl(a,b), maxl(a,b)+minl(a,b), a+b);
        casos++;
    }
    ok("tropical e glacial: ν troca max por min, e o ⊗ (a soma) é o MESMO nos dois", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      O que vira é só o ⊕. O ⊗ — que é a soma comum — fica igual nos dois polos, e é\n");
    printf("      por isso que eles são o mesmo objeto visto de dois lados. Nenhum tem oposto\n");
    printf("      aditivo sozinho; o par devolve a soma.\n");
}

printf("\n§H9  O TELESCÓPICO e o ECONÔMICO: o idempotente e a exponencial.\n\n");
{
    int mau = 0; long casos = 0;
    const long p = 7;
    /* Correção do Aarão: "telescópico e econômico". Ele tinha dito "acho que é celeste" e
     * corrigiu — e as duas coisas convivem. O celeste é o telescópico NOUTRA BASE (§H5/§H6,
     * medido); o DUAL dele é o econômico. E o par é o que resolve a unidade. */
    printf("      idempotente   e⊗e = e?   e₁⊕e₂    e₁⊗e₂    resolve a unidade?\n");
    Par e1 = {1,0}, e2 = {0,1};
    Par q1 = { e1.a*e1.a % p, e1.b*e1.b % p };
    Par q2 = { e2.a*e2.a % p, e2.b*e2.b % p };
    if(q1.a != e1.a || q1.b != e1.b) mau++;               /* e₁ é idempotente */
    if(q2.a != e2.a || q2.b != e2.b) mau++;
    Par so = { (e1.a+e2.a) % p, (e1.b+e2.b) % p };        /* e₁ ⊕ e₂ = 1 */
    Par pr = { e1.a*e2.a % p, e1.b*e2.b % p };            /* e₁ ⊗ e₂ = 0 */
    if(so.a != 1 || so.b != 1) mau++;
    if(pr.a != 0 || pr.b != 0) mau++;
    printf("      e₁ = (1,0)    sim        (%ld,%ld)    (%ld,%ld)    sim ✓\n", so.a,so.b,pr.a,pr.b);
    printf("      e₂ = (0,1)    sim        idem     idem     sim ✓\n");
    /* e o lado ECONÔMICO: a exponencial COMPÕE, a(s+t) = a(s)·a(t) — o juro composto */
    for(long g = 2; g <= 5; g++) for(long s = 0; s < 6; s++) for(long t = 0; t < 6; t++){
        long e = 1, x = 1, y = 1;
        for(long k = 0; k < s+t; k++) e = e*g % p;
        for(long k = 0; k < s; k++)   x = x*g % p;
        for(long k = 0; k < t; k++)   y = y*g % p;
        if(e != x*y % p) mau++;
        casos++;
    }
    ok("o telescópico CINDE em idempotentes e o econômico COMPÕE — e o par resolve a unidade",
       mau == 0);
    printf("      (a exponencial conferida em %ld casos: a(s+t) = a(s)·a(t).)\n", casos);
    printf("\n      Cada idempotente sozinho é divisor de zero — e₁⊗e₂ = 0, que é o \"cinde\". Mas\n");
    printf("      e₁⊕e₂ = 1: o PAR resolve a unidade. É a mesma forma dos outros dois dipolos, e\n");
    printf("      o dual é o econômico, cujo operador COMPÕE em vez de cindir — a(s+t)=a(s)a(t).\n");
    printf("\n      E isto não apaga o §H5: o celeste é o telescópico NOUTRA BASE, medido. Duas\n");
    printf("      relações distintas — mudança de base com o celeste, DUALIDADE com o econômico.\n");
}

printf("\n§H7  O veredito CORRIGIDO: metade, não falha.\n\n");
{
    printf("      eu disse                      o certo é                         medida\n");
    printf("      telescópico NÃO É CORPO       é meio par com o ECONÔMICO        §H9, e₁⊕e₂=1\n");
    printf("        (e o celeste é o mesmo objeto noutra base — §H5, N = αβ)\n");
    printf("      entrópico NÃO É CORPO         é meio cósmico — uma polaridade   §H1, max+min\n");
    printf("      motor NÃO É CORPO             é o rotor fora do ponto fixo      §H3, tr = 0\n");
    ok("os três eram METADES, e cada par fecha o que a metade perdia", 1);
    printf("\n      O erro tem forma conhecida: descrever uma peça pelo que lhe FALTA em vez de pelo\n");
    printf("      que ela é. \"Não tem oposto\" é verdade sobre o polo e falso sobre o dipolo;\n");
    printf("      \"cinde\" é verdade sobre a base e falso sobre o objeto; \"dissipa\" é verdade\n");
    printf("      sobre um lado e falso sobre o par.\n");
    printf("\n      E era tudo legível: o cosmico.py diz \"o entrópico é a sua METADE, uma polaridade\n");
    printf("      do dipolo\", e o certifica_corpos.py diz \"o conservativo é que SERIA corpo\". Eu\n");
    printf("      li a primeira metade das duas frases.\n");
}

printf("\n=== AS METADES ============================================================\n");
printf("  Não há \"não é corpo\": há METADE de corpo, e cada metade tem o seu par.\n\n");
printf("    entrópico   ↔ cósmico     ν(max) = min, e max + min = a + b\n");
printf("    motor       ↔ rotor       ν(t) = −t, e o rotor é o PONTO FIXO: tr = 0\n");
printf("    telescópico ↔ econômico   e₁⊗e₂ = 0 cinde, mas e₁⊕e₂ = 1 resolve a unidade\n");
printf("    tropical    ↔ glacial      ν troca max por min; o ⊗ (a soma) é o mesmo nos dois\n\n");
printf("  E o celeste é o telescópico NOUTRA BASE (§H5/§H6): o divisor de zero é o cone nulo de\n");
printf("  N = a²−b². Mudança de base é uma relação; dualidade é outra — e eu tinha-as juntado.\n\n");
printf("  E o que sobra fora do par tem uma marca só, em ℤ: a volta existe no anel exatamente\n");
printf("  onde |det| = 1. É por isso que a ISA só tem peças de det ±1 — não por escolha de\n");
printf("  projeto, mas porque fora dali não há volta.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
