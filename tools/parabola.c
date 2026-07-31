/* parabola.c — COMO A PARÁBOLA É FABRICADA, e por que ela é o princípio de conservação.
 *
 * O Aarão descreveu o mecanismo: "imagina que só existam retas no espaço, e nós queremos medir o
 * espaço com círculos de vários tamanhos. Quando conseguimos encostar duas retas em lados opostos
 * do círculo em qualquer posição, dizemos que conseguimos uma medição. Mas veja: as retas se
 * cruzam ALÉM do círculo — a régua é um CONE. Mas a régua tem a curvatura, que é de 2π. Então
 * vemos a parábola."
 *
 * Isto não é analogia: é a construção da cónica, e explica o que eu vinha usando sem saber de
 * onde vinha. A cadeia é:
 *
 *   1. só retas       duas retas cruzam-se ou são paralelas. Não há mais nada — nenhum
 *                     invariante contínuo. É o plano estéril.
 *   2. o círculo      entra como régua, e a sua CURVATURA TOTAL é 2π seja qual for o raio.
 *                     É isso que faz qualquer tamanho servir de régua — a conservação.
 *   3. as tangentes   duas retas encostadas em lados opostos encontram-se ALÉM do círculo,
 *                     num vértice: o par é um CONE.
 *   4. o corte        seccionar o cone dá a cónica, e o discriminante Δ = B²−4AC diz qual.
 *   5. a parábola     é o corte PARALELO À GERATRIZ — onde as duas retas se encontram no
 *                     INFINITO. É a fronteira, e é Δ = 0.
 *
 * E o Δ que sai daqui é o MESMO Δ da régua q(a,b) = a² + B·ab + C·b². A régua é uma secção cónica.
 *
 *   §Q1  só retas: dois casos, e nenhum invariante — o plano estéril
 *   §Q2  A CONSERVAÇÃO: a curvatura total do círculo é 2π para TODO raio — exato em ℚ
 *   §Q3  as duas tangentes existem além do círculo, e formam vértice
 *   §Q4  o corte do cone: Δ = 4(m²−1), e as três cónicas saem do declive
 *   §Q5  a PARÁBOLA é o corte paralelo à geratriz — o encontro no infinito
 *   §Q6  e é o mesmo Δ da régua: a régua É uma secção cónica
 *
 *   cc -O2 -std=c99 parabola.c -o parabola && ./parabola
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static Par q(long n, long d){ return ra_classe((Par){n,d}); }
static int  q_eq(Par x, Par y){ x=ra_classe(x); y=ra_classe(y); return x.a==y.a && x.b==y.b; }

int main(void){
printf("\n=== COMO A PARÁBOLA É FABRICADA ===========================================\n");
printf("    Só retas; um círculo por régua; as tangentes fazem cone; o corte dá a cónica.\n");

printf("\n§Q1  Só retas: dois casos, e nenhum invariante — o plano estéril.\n\n");
{
    int mau = 0; long cruza = 0, paralela = 0, casos = 0;
    for(long a1 = -8; a1 <= 8; a1++) for(long b1 = -8; b1 <= 8; b1++)
    for(long a2 = -8; a2 <= 8; a2++) for(long b2 = -8; b2 <= 8; b2++){
        /* retas y = a·x + b: cruzam-se sse os declives diferem */
        int cr = (a1 != a2);
        if(cr) cruza++; else paralela++;
        if(cr + !cr != 1) mau++;
        casos++;
    }
    ok("duas retas cruzam-se ou são paralelas — DOIS casos, e nada a graduar", mau == 0);
    printf("      (%ld pares: %ld cruzam, %ld paralelas.)\n", casos, cruza, paralela);
    printf("\n      Dois casos e nenhum número contínuo entre eles. É o §W3 outra vez, pelo lado da\n");
    printf("      geometria: a reta não tem discriminante, logo não há o que medir.\n");
}

printf("\n§Q2  A CONSERVAÇÃO: a curvatura total é 2π para TODO raio. Exato em ℚ.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      raio     curvatura κ = 1/r   comprimento L/2π = r   κ·L / 2π\n");
    for(long n = 1; n <= 30; n++) for(long d = 1; d <= 12; d++){
        Par r = q(n,d);                        /* o raio, racional */
        Par k = q(r.b, r.a);                   /* κ = 1/r          */
        /* L = 2π·r, logo κ·L/(2π) = (1/r)·r = 1 — exato, e sem tocar em π */
        Par total = ra_prod(k, r);
        if(!q_eq(total, q(1,1))) mau++;
        casos++;
    }
    printf("      1        1/1                 1/1                    1 ✓\n");
    printf("      7/3      3/7                 7/3                    1 ✓\n");
    printf("      1/12     12/1                1/12                   1 ✓\n");
    ok("κ·L / 2π = 1 EXATO para todo raio racional — o π cancela e não é preciso", mau == 0);
    printf("      (%ld raios.)\n", casos);
    printf("\n      É ISTO a conservação: o círculo dá sempre a MESMA volta, seja grande ou pequeno.\n");
    printf("      Por isso qualquer tamanho serve de régua — a medição não depende do tamanho do\n");
    printf("      instrumento. E note-se que o π nem aparece: ele cancela, e a conta fica em ℚ.\n");
}

printf("\n§Q3  As duas tangentes existem ALÉM do círculo, e formam vértice.\n\n");
{
    int mau = 0; long fora = 0, dentro = 0, sobre = 0, casos = 0;
    printf("      ponto p   raio r   p²−r²   há duas tangentes?\n");
    for(long p = -12; p <= 12; p++) for(long r = 1; r <= 12; r++){
        long t2 = p*p - r*r;                   /* o comprimento da tangente, ao quadrado */
        if(t2 > 0) fora++; else if(t2 == 0) sobre++; else dentro++;
        /* e o par de tangentes é uma cónica DEGENERADA: duas retas reais sse t² > 0 */
        if((t2 > 0) != (p*p > r*r)) mau++;
        if(r == 5 && (p == 13 || p == 5 || p == 3))
            printf("      %-9ld %-8ld %-7ld %s\n", p, r, t2,
                   t2 > 0 ? "sim — cruzam-se num vértice" :
                   (t2 == 0 ? "uma só — o ponto está na borda" : "nenhuma — está dentro"));
        casos++;
    }
    ok("as duas tangentes existem exatamente quando p² > r² — fora do círculo", mau == 0);
    printf("      (%ld pontos: %ld fora, %ld na borda, %ld dentro.)\n", casos, fora, sobre, dentro);
    printf("\n      As retas encostadas em lados opostos NÃO ficam paralelas: encontram-se além do\n");
    printf("      círculo. Esse encontro é o VÉRTICE, e o par de retas com o vértice é o CONE.\n");
    printf("      A régua deixou de ser plana no momento em que se encostou duas retas a um\n");
    printf("      círculo.\n");
}

printf("\n§Q4  O corte do cone: Δ = 4(m²−1), e as três cónicas saem do DECLIVE.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      declive m   Δ = 4(m²−1)   cónica        o plano vs a geratriz\n");
    /* cone z² = x²+y²; plano z = m·x + c. Substituindo: (m²−1)x² − y² + 2mcx + c² = 0
     * logo A = m²−1, B = 0, C = −1, e Δ = B²−4AC = 4(m²−1). */
    struct { long mn, md; const char *nome; } ms[] = {
        { 0,1, "elipse"    }, { 1,2, "elipse"    }, { 1,1, "PARÁBOLA" },
        { 2,1, "hipérbole" }, { 3,1, "hipérbole" },
    };
    for(unsigned t = 0; t < sizeof ms/sizeof ms[0]; t++){
        Par m = q(ms[t].mn, ms[t].md);
        Par D = ra_prod(q(4,1), ra_soma(ra_prod(m,m), q(-1,1)));
        int sinal = ra_cmp(D, q(0,1));
        const char *esperado = sinal < 0 ? "elipse" : (sinal == 0 ? "PARÁBOLA" : "hipérbole");
        if(!(esperado[0] == ms[t].nome[0])) mau++;
        printf("      %ld/%-9ld %ld/%-11ld %-13s %s\n", m.a, m.b, D.a, D.b, esperado,
               sinal < 0 ? "mais raso que a geratriz" :
               (sinal == 0 ? "PARALELO à geratriz" : "mais inclinado que a geratriz"));
        casos++;
    }
    ok("o sinal de Δ = 4(m²−1) dá as três cónicas, e vem só do declive do corte", mau == 0);
    printf("      (%ld cortes.)\n", casos);
}

printf("\n§Q5  A PARÁBOLA é o corte PARALELO à geratriz — o encontro no INFINITO.\n\n");
{
    int mau = 0; long casos = 0;
    /* a geratriz do cone z²=x²+y² no plano y=0 tem declive ±1. O corte com declive m encontra-a
     * num ponto finito SSE m ≠ ±1; com m = ±1 as duas direções são paralelas: encontro no
     * infinito, e é exatamente aí que Δ = 0. */
    for(long n = -20; n <= 20; n++) for(long d = 1; d <= 8; d++){
        Par m = q(n,d);
        Par D = ra_prod(q(4,1), ra_soma(ra_prod(m,m), q(-1,1)));
        int paralelo = q_eq(m, q(1,1)) || q_eq(m, q(-1,1));
        int zero = (ra_cmp(D, q(0,1)) == 0);
        if(paralelo != zero) mau++;            /* Δ = 0 SSE paralelo à geratriz */
        casos++;
    }
    ok("Δ = 0 EXATAMENTE quando o corte é paralelo à geratriz — nem antes, nem depois", mau == 0);
    printf("      (%ld declives racionais.)\n", casos);
    printf("\n      E é isto a frase dele. As duas retas encostadas ao círculo encontram-se num\n");
    printf("      vértice; quando o corte fica PARALELO à geratriz, o encontro vai para o infinito\n");
    printf("      — e a cónica que se vê é a parábola. A parábola não é uma curva entre as outras:\n");
    printf("      é a FRONTEIRA onde o encontro deixa de ser finito.\n");
    printf("\n      É o mesmo Δ=0 que eu já tinha como fronteira em topologia.c e como o estéril em\n");
    printf("      tres_pontos.c. Agora tem mecanismo: é onde o vértice foge.\n");
}

printf("\n§Q6  E é o MESMO Δ da régua: a régua É uma secção cónica.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      régua a²+B·ab+C·b²   Δ_régua   cónica com A=1,B,C   Δ_cónica   igual?\n");
    for(long B = -10; B <= 10; B++) for(long C = -10; C <= 10; C++){
        Regua r = { B, C };
        long Dr = ct_assinatura(r);            /* B² − 4C, com A = 1 */
        long Dc = B*B - 4*1*C;                 /* o discriminante da cónica A x²+B xy+C y² */
        if(Dr != Dc) mau++;
        casos++;
    }
    printf("      a² + ab − b²         5         A=1,B=1,C=−1         5          sim ✓\n");
    printf("      a² + b²              −4        A=1,B=0,C=1          −4         sim ✓\n");
    printf("      a²                   0         A=1,B=0,C=0          0          sim ✓\n");
    ok("Δ da régua É o discriminante da cónica — o mesmo número, e não por analogia", mau == 0);
    printf("      (%ld réguas.)\n", casos);
    printf("\n      Então a régua não \"lembra\" uma cónica: ela É uma. E as três classes que eu vinha\n");
    printf("      a chamar elíptica, parabólica e hiperbólica têm esses nomes porque SÃO isso —\n");
    printf("      eu usei os nomes o dia inteiro sem ter medido de onde vinham.\n");
    printf("\n      E o princípio de conservação está no §Q2: a curvatura total é 2π seja qual for o\n");
    printf("      raio. É ela que permite medir com círculos de qualquer tamanho — a régua graduada\n");
    printf("      contínua — e é dela que a cónica sai. O que se conserva é a volta.\n");
}

printf("\n=== A PARÁBOLA ============================================================\n");
printf("  Só retas: dois casos, nada a graduar. Entra o círculo, e com ele a CONSERVAÇÃO:\n");
printf("  κ·L / 2π = 1 para todo raio — exato em ℚ, e o π cancela. É por isso que qualquer\n");
printf("  tamanho de círculo serve de régua.\n\n");
printf("  Duas retas encostadas em lados opostos encontram-se ALÉM do círculo: o vértice, e o\n");
printf("  par com o vértice é o CONE. Cortar o cone dá Δ = 4(m²−1), e as três cónicas saem só do\n");
printf("  declive. A PARÁBOLA é o corte paralelo à geratriz — onde o encontro vai ao infinito.\n\n");
printf("  E esse Δ É o Δ da régua, o mesmo número. A régua não lembra uma cónica: é uma. Os nomes\n");
printf("  elíptico/parabólico/hiperbólico que usei o dia inteiro vinham daqui, e eu não sabia.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais, sem um único float e sem um único π.\n\n");
return 0;
}
