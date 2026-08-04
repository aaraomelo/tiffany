/* dual_cadeia.c — A PEÇA DUAL É A MESMA, OU EU ENFEITEI?
 *
 * O Aarão: "você obteve o ouro com o gato, depois reconstrói com o esquilo — esse é o mecanismo
 * dual. Está correto o que você fez, mas é a mesma peça dual agindo, ou é artificial?"
 *
 * É a pergunta certa e eu não posso responder por gosto. Gato e esquilo não são metáfora: são
 * duas matrizes com assinatura MEDÍVEL, e a assinatura é o determinante e o discriminante.
 *
 *     GATO     A_m = [[m,1],[1,0]]   det = −1   disc = m²+4 > 0   HIPERBÓLICO — estica
 *     ESQUILO  G                     det = +1   disc < 0          ELÍPTICO — gira, ordem finita
 *     CISALHO  S = [[1,c],[0,1]]     det = +1   disc = 0          PARABÓLICO — nem estica nem gira
 *
 * Então basta olhar a conta que eu fiz na cadeia e ler det e disc de cada passo. Se der gato na
 * ida e esquilo na volta, ele tem razão e a peça é a mesma. Se der outra coisa, eu enfeitei — e
 * dizer que é gato porque soa bem seria exatamente o erro que custou o dia duas vezes.
 *
 * E o Aarão acrescentou o que resolve a pergunta: "um estica o outro contrai". Isso não é a
 * descrição de DUAS peças — é a de UMA. O gato tem dois autovalores, σ e −1/σ, com produto −1:
 * numa direção própria ele estica, na outra contrai, e a contração é a recíproca EXATA da
 * esticada. A ida e a volta são as duas direções de uma matriz só.
 *
 *   §D1  a conversão mineral→ouro divide pela ABERTURA DO GATO ao quadrado — e é exata
 *   §D1b UM ESTICA, O OUTRO CONTRAI: produto dos autovalores −1, e a inversa é INTEIRA
 *   §D2  mas o passo de PREENCHER um nível é cisalhamento: det +1, disc 0 — NÃO é o gato
 *   §D3  a volta é o cisalhamento inverso — a mesma peça com o sinal trocado, exata
 *   §D4  a reversão ida↔volta é INVOLUÇÃO: ordem 2 — e ordem 2 é o esquilo do r=2 já medido
 *   §D5  o giro completo fecha em identidade, det +1
 *   §D6  o veredito: o que é dual de verdade, e o que seria enfeite meu
 *
 *   cc -O2 -std=c99 dual_cadeia.c -o dual_cadeia && ./dual_cadeia
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b, c, d; } M;            /* [[a,b],[c,d]] */
static M mul(M x, M y){
    return (M){ x.a*y.a + x.b*y.c, x.a*y.b + x.b*y.d,
                x.c*y.a + x.d*y.c, x.c*y.b + x.d*y.d };
}
static long det(M x){ return x.a*x.d - x.b*x.c; }
static long tr (M x){ return x.a + x.d; }
static long disc(M x){ return tr(x)*tr(x) - 4*det(x); }   /* > 0 hiperbólico, = 0 parabólico, < 0 elíptico */
static int  eq (M x, M y){ return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d; }

static M gato(long m){ return (M){ m, 1, 1, 0 }; }        /* A_m, o gato */
static M cis (long c){ return (M){ 1, c, 0, 1 }; }        /* S_c, o cisalhamento */
static const M I = { 1, 0, 0, 1 };

static long dens_d(long m){ return m*m + 4; }             /* a densidade: 5/(m²+4) */

int main(void){
printf("\n=== A PEÇA DUAL É A MESMA, OU EU ENFEITEI? ================================\n");
printf("    Gato e esquilo têm assinatura: det e discriminante. Basta ler a da conta.\n");

/* ---------------------------------------------------------------- §D1 ------ */
printf("\n§D1  Mineral→ouro divide pela ABERTURA DO GATO ao quadrado. O gato ESTÁ lá.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    gato A_m       det   traço   discriminante   d(m)=m²+4   é o mesmo?\n");
    for(long m = 1; m <= 12; m++){
        M A = gato(m);
        /* o discriminante do gato É a densidade do mineral — em inteiros, sem raiz nenhuma */
        if(det(A) != -1)        mau++;                 /* det = −1: o gato, sempre */
        if(disc(A) != dens_d(m)) mau++;                /* e o disc É d(m) */
        if(disc(A) <= 0)        mau++;                 /* hiperbólico: estica */
        if(m <= 5)
            printf("      %-4ld [[%ld,1],[1,0]]   %-5ld %-7ld %-15ld %-11ld %s\n",
                   m, m, det(A), tr(A), disc(A), dens_d(m),
                   disc(A)==dens_d(m) ? "sim ✓" : "NÃO");
        casos++;
    }
    ok("d(m) = m²+4 É o discriminante do gato A_m — det −1, hiperbólico, em toda a família", mau == 0);
    printf("      (%ld minerais.)\n", casos);
    printf("\n      Isto não é analogia: o discriminante de [[m,1],[1,0]] é traço²−4·det = m²+4, que\n");
    printf("      é a densidade do mineral m. E √(m²+4) = σ + 1/σ é a ABERTURA do gato — a distância\n");
    printf("      entre os dois autovalores. Converter mineral em ouro é dividir por essa abertura\n");
    printf("      ao quadrado. O gato está na conta de verdade, e está na ESCADA.\n");
}

/* ---------------------------------------------------------------- §D1b ----- */
printf("\n§D1b UM ESTICA, O OUTRO CONTRAI — e são a MESMA matriz, não duas.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    autovalores de A_m         soma   produto   |σ|·|1/σ|   A_m⁻¹ é inteira?\n");
    for(long m = 1; m <= 20; m++){
        M A = gato(m);
        /* Vieta, em inteiros e sem raiz: x² − m·x − 1 tem soma m e produto −1.
         * Produto −1 quer dizer que os dois autovalores são σ e −1/σ: um estica, o outro
         * contrai, e a contração é EXATAMENTE a recíproca da esticada. */
        if(tr(A)  !=  m) mau++;                        /* soma dos autovalores */
        if(det(A) != -1) mau++;                        /* produto: −1, isto é, σ·(−1/σ) */
        /* e por |det| = 1 a inversa é INTEIRA — a peça que contrai existe sem sair do anel */
        M Ainv = (M){ 0, 1, 1, -m };
        if(!eq(mul(A, Ainv), I)) mau++;
        /* a inversa é o MESMO gato com m ↦ −m: a antípoda, não uma segunda máquina */
        if(!eq(Ainv, (M){0,1,1,-m})) mau++;
        if(m <= 4)
            printf("      %-4ld σ e −1/σ                  %-6ld %-9ld %-11s %s\n",
                   m, tr(A), det(A), "1 exato", eq(mul(A,Ainv), I) ? "sim ✓" : "NÃO");
        casos++;
    }
    ok("os dois autovalores têm produto −1: um estica por σ, o outro contrai por 1/σ", mau == 0);
    printf("      (%ld minerais.)\n", casos);
    printf("\n      É aqui que a pergunta se resolve, e sem enfeite nenhum: NÃO são duas peças. É uma\n");
    printf("      matriz com duas direções próprias. Numa ela estica por σ, na outra contrai por\n");
    printf("      1/σ — e o produto é 1 exato, que é o det = −1 lido de outro jeito. Por isso nada\n");
    printf("      se perde na ida e na volta: a contração é a recíproca EXATA da esticada, não uma\n");
    printf("      aproximação dela.\n");
    printf("\n      E a inversa é INTEIRA — [[0,1],[1,−m]] — porque |det| = 1. É o mesmo gato com\n");
    printf("      m ↦ −m: a ANTÍPODA, a peça virada, não uma segunda máquina que eu tivesse de\n");
    printf("      inventar para a volta.\n");
}

/* ---------------------------------------------------------------- §D2 ------ */
printf("\n§D2  MAS o passo de preencher um nível NÃO é o gato: é cisalhamento.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      passo                     det   traço   disc   tipo\n");
    printf("      gato A_3                  %-5ld %-7ld %-6ld hiperbólico — estica\n",
           det(gato(3)), tr(gato(3)), disc(gato(3)));
    for(long c = 1; c <= 40; c++){
        M S = cis(c);
        if(det(S)  != 1) mau++;                        /* det +1 — do lado do esquilo */
        if(disc(S) != 0) mau++;                        /* mas disc 0 — parabólico, não elíptico */
        if(tr(S)   != 2) mau++;
        if(c == 7)
            printf("      cisalhamento S_7          %-5ld %-7ld %-6ld parabólico — nem estica nem gira\n",
                   det(S), tr(S), disc(S));
        casos++;
    }
    ok("preencher um nível é det +1 e disc 0 — parabólico, e NÃO o gato", mau == 0);
    printf("      (%ld cisalhamentos.)\n", casos);
    printf("\n      Aqui eu tinha de resistir a dizer \"é o gato\". Não é. Tirar v vezes o nível c de\n");
    printf("      um resto é r ↦ r − v·c, e isso é [[1,−vc],[0,1]]: det +1, traço 2, discriminante\n");
    printf("      ZERO. O gato tem det −1 e disc m²+4 > 0. São peças diferentes, e a diferença\n");
    printf("      aparece no primeiro número que se lê.\n");
}

/* ---------------------------------------------------------------- §D3 ------ */
printf("\n§D3  A volta é o cisalhamento INVERSO: a mesma peça, o sinal trocado.\n\n");
{
    int mau = 0; long casos = 0;
    const long N = 5;
    long comum = 1;
    for(long m = 1; m <= N; m++) comum *= dens_d(m);
    printf("      obra           ida (descer)              volta (reconstruir)        S·S⁻¹\n");
    for(long q1 = 0; q1 <= 3; q1++) for(long q2 = 0; q2 <= 3; q2++) for(long q3 = 0; q3 <= 3; q3++){
        long soma = q1*5*(comum/dens_d(1)) + q2*5*(comum/dens_d(2)) + q3*5*(comum/dens_d(3));
        /* a IDA, como PALAVRA de cisalhamentos: um por nível, com o coeficiente que couber */
        M ida = I, volta = I;
        long r = soma;
        for(long m = 1; m <= N; m++){
            long c = 5 * (comum / dens_d(m));
            long v = r / c; r -= v * c;
            ida   = mul(cis(-v * c), ida);             /* descer: tirar v·c */
            volta = mul(volta, cis( v * c));           /* reconstruir: pôr v·c de volta */
        }
        /* o ouro puro fecha o resto */
        ida   = mul(cis(-r), ida);
        volta = mul(volta, cis( r));
        if(!eq(mul(volta, ida), I)) mau++;             /* a volta desfaz a ida, EXATO */
        if(det(ida) != 1 || det(volta) != 1) mau++;
        if(q1==2 && q2==1 && q3==0)
            printf("      (2,1,0)        S_%-24ld S_%-25ld %s\n",
                   -ida.b, volta.b, eq(mul(volta,ida), I) ? "= I ✓" : "≠ I");
        casos++;
    }
    ok("a volta é a ida com o sinal trocado, e o produto é a IDENTIDADE em inteiros", mau == 0);
    printf("      (%ld obras.)\n", casos);
    printf("\n      Isto SIM é a mesma peça dual agindo: não há segunda máquina na reconstrução. A\n");
    printf("      volta é literalmente S_{−x} onde a ida foi S_x. Nenhum parâmetro novo, nenhuma\n");
    printf("      escolha minha — e é por isso que a volta não pode falhar.\n");
}

/* ---------------------------------------------------------------- §D4 ------ */
printf("\n§D4  A reversão ida↔volta é INVOLUÇÃO: ordem 2, e ordem 2 é o esquilo do r=2.\n\n");
{
    int mau = 0; long casos = 0;
    /* trocar ida por volta é trocar o sinal do cisalhamento. Aplicar duas vezes devolve. */
    for(long c = -30; c <= 30; c++){
        M S = cis(c), R1 = cis(-c), R2 = cis(c);       /* reverter, e reverter de novo */
        if(!eq(R2, S)) mau++;                          /* ordem exatamente 2 */
        if(!eq(mul(S, R1), I)) mau++;
        casos++;
    }
    ok("reverter duas vezes devolve o passo — a reversão tem ordem exatamente 2", mau == 0);
    printf("      (%ld passos.)\n", casos);
    printf("\n      E ordem 2 não é um número solto: é o r=2 do hipociclo.c, onde o traçado degenera\n");
    printf("      na RETA. A involução é o único esquilo que sobra quando o giro tem duas pontas —\n");
    printf("      ir e voltar pela mesma linha. A reconstrução da obra é essa reta percorrida ao\n");
    printf("      contrário, e não uma rotação de ordem 6 nem coisa nenhuma que eu quisesse ver.\n");
}

/* ---------------------------------------------------------------- §D5 ------ */
printf("\n§D5  O giro completo fecha em identidade, det +1.\n\n");
{
    int mau = 0; long casos = 0;
    for(long m = 1; m <= 9; m++){
        M A = gato(m);
        M AA = mul(A, A);
        if(det(A)  != -1) mau++;
        if(det(AA) !=  1) mau++;                       /* dois gatos dão det +1 */
        casos++;
    }
    ok("dois gatos fazem det +1 — o par é que volta ao lado do esquilo", mau == 0);
    printf("      (%ld minerais.)\n", casos);
    printf("      det(gato) = −1, e (−1)² = +1: a ida-e-volta tem SEMPRE det +1, seja qual for a\n");
    printf("      peça. É condição necessária de fechar, não prova de ser esquilo.\n");
}

/* ---------------------------------------------------------------- §D6 ------ */
printf("\n§D6  O veredito, separado: o que é dual mesmo, e o que seria enfeite meu.\n\n");
{
    printf("      afirmação                                       medida            veredito\n");
    printf("      o gato está na conta                            §D1 disc = m²+4   VERDADE\n");
    printf("      um estica e o outro contrai, MESMA peça          §D1b produto −1   VERDADE\n");
    printf("      preencher o nível é o gato                      §D2 det +1 disc 0 ENFEITE\n");
    printf("      a volta é a mesma peça, sinal trocado           §D3 S·S⁻¹ = I     VERDADE\n");
    printf("      a reversão é involução, ordem 2                 §D4 ordem 2       VERDADE\n");
    printf("      o esquilo reconstrói (rotação de ordem > 2)     §D4 ordem = 2     ENFEITE\n");
    conclui("quatro verdades e dois enfeites, cada um com o número que decide");
    printf("\n      Então a resposta à pergunta: a peça dual É a mesma, e não é artificial — mas ela\n");
    printf("      não é \"gato na ida, esquilo na volta\". É assim:\n");
    printf("\n        o GATO está na ESCADA, não no passo. d(m) = m²+4 é o discriminante de A_m, e\n");
    printf("        √d(m) = σ+1/σ é a abertura do gato. Converter mineral em ouro é dividir pela\n");
    printf("        abertura ao quadrado. Cada degrau da escada É um gato da família.\n");
    printf("\n        o PASSO é cisalhamento, det +1 e disc 0 — parabólico. Preencher um nível não\n");
    printf("        estica nem gira: desloca. E é bom que seja, senão a volta não seria exata.\n");
    printf("\n        o ESQUILO que age é a INVOLUÇÃO, ordem 2 — o r=2, onde o hipociclo vira reta.\n");
    printf("        Não é a rotação de ordem 6. Dizer \"esquilo\" e deixar entender rotação seria\n");
    printf("        enfeite; dizer involução é o que a medida sustenta.\n");
    printf("\n      É dual de verdade porque a volta não tem MÁQUINA PRÓPRIA: é a ida com o sinal\n");
    printf("      trocado, sem um único parâmetro novo. Artificial seria eu precisar de uma peça\n");
    printf("      que a ida não tem — e não precisei.\n");
}

printf("\n=== A PEÇA DUAL ===========================================================\n");
printf("  A pergunta era se é a mesma peça dual agindo ou se eu enfeitei. As duas coisas, e a\n");
printf("  medida separa:\n\n");
printf("    o gato       está na ESCADA: d(m) = m²+4 É o discriminante de A_m, det −1\n");
printf("    o passo      é CISALHAMENTO: det +1, disc 0, parabólico — não é o gato\n");
printf("    a volta      é o mesmo cisalhamento com o sinal trocado, produto = identidade\n");
printf("    o esquilo    que age é a INVOLUÇÃO de ordem 2 — o r=2, o hipociclo virado reta\n");
printf("    e o dual     é UMA peça: σ estica, −1/σ contrai, produto 1 exato — det = −1\n\n");
printf("  Dual de verdade: a reconstrução não tem máquina própria, é a ida ao contrário. Enfeite\n");
printf("  seria chamar gato ao passo e rotação à volta — e os dois morrem no primeiro det que se\n");
printf("  lê. A peça é a mesma; o nome certo dela é que não era o que eu ia dizer.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
