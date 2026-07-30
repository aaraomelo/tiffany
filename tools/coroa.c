/* coroa.c — A ASSINATURA DO REI, E AS COORDENADAS DELE.
 *
 * O Aarão coroa a maçã e dá a construção: "nosso 1 vai ser o pomo de ouro [1,1,1,1,1...], um
 * polígono antissimétrico de razão de lados 1 — essa é a assinatura do rei. A partir daí
 * qualquer outra pode ser expressa nas coordenadas do rei. O rei está fora do jogo redondo, no
 * infinito, só cifrando."
 *
 * É uma TROCA DE UNIDADE, e cada pedaço dela se mede:
 *
 *   A ASSINATURA   [1,1,1,1,…] — a fração contínua toda de uns. É o pomo de ouro, e o que a
 *                  distingue não é ser bonita: é ser a PIOR aproximável por racionais. Termo
 *                  grande na fração contínua é uma boa aproximação racional à espreita; o rei
 *                  não tem nenhum. Nada nele dá para agarrar.
 *
 *   O POLÍGONO     o pentagrama da maçã: cinco lados iguais (razão 1), e a diagonal d obedece
 *                  d² = d + 1 por Ptolomeu — que É a equação do gato com m=1. A assinatura não
 *                  está escrita ao lado da figura: está DENTRO dela.
 *
 *   AS COORDENADAS todo inteiro se escreve, de uma única maneira, como soma de Fibonacci não
 *                  consecutivos. É o sistema de numeração do rei, e ele é completo e sem
 *                  ambiguidade — qualquer outro cabe nele.
 *
 *   SÓ CIFRANDO    multiplicar pelo rei não multiplica nada: (a,b) ↦ (b, a+b). É um
 *                  DESLOCAMENTO. Ele não entra na conta — ele reindexa. É o que cifrar quer
 *                  dizer aqui, e é por isso que fica fora do jogo.
 *
 *   §A1  a assinatura: [1,1,…] e a norma p² − pq − q² = ±1, exata, sempre
 *   §A2  o pentagrama: d² = d + 1 é a equação do gato — e σⁿ = F_n·σ + F_{n−1}
 *   §A3  as coordenadas do rei: Zeckendorf, existência e unicidade, exaustivo
 *   §A4  fora do jogo redondo: os termos do rei são todos 1; os de π chegam a 292
 *   §A5  só cifrando: multiplicar pelo rei é DESLOCAR — nenhuma multiplicação acontece
 *
 *   cc -O2 -std=c99 coroa.c -o coroa && ./coroa
 */
#include <stdio.h>

static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-58s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}

#define NF 40
static long F[NF];

int main(void){
F[0] = 0; F[1] = 1;
for(int i = 2; i < NF; i++) F[i] = F[i-1] + F[i-2];

printf("\n=== A ASSINATURA DO REI, E AS COORDENADAS DELE ============================\n");
printf("    O 1 passa a ser o pomo de ouro: [1,1,1,1,…]. Tudo o mais se escreve nele.\n");

/* ---------------------------------------------------------------- §A1 ------ */
printf("\n§A1  A ASSINATURA: [1,1,1,…], e a norma é ±1 em todo convergente.\n\n");
{
    int mau = 0;
    printf("      n     p=F(n+1)   q=F(n)   p² − pq − q²   é ±1?\n");
    for(int n = 2; n < 30; n++){
        long p = F[n+1], q = F[n];
        long norma = p*p - p*q - q*q;
        if(norma != 1 && norma != -1) mau++;
        if(n <= 6 || n == 20 || n == 29)
            printf("      %-5d %10ld %8ld %14ld   %s\n", n, p, q, norma,
                   (norma==1||norma==-1)?"sim ✓":"NÃO");
    }
    ok("a norma é ±1 em TODO convergente — exata, em inteiros", mau == 0);
    printf("\n      A norma ser sempre ±1 é o que diz que a aproximação está no limite do que é\n");
    printf("      possível: p/q chega o mais perto que uma fração pode chegar, e nunca acerta.\n");
    printf("      O rei nunca é atingido, e nunca deixa de ser cercado.\n");
}

/* ---------------------------------------------------------------- §A2 ------ */
printf("\n§A2  O POLÍGONO DA MAÇÃ: d² = d + 1 é a equação do gato, com m = 1.\n\n");
{
    /* Ptolomeu no pentágono regular de lado 1: no quadrilátero cíclico com três lados 1 e o
     * quarto a diagonal d, vale AC·BD = AB·CD + BC·AD, isto é d·d = 1·1 + 1·d.
     * Logo d² = d + 1 — a MESMA equação σ² = mσ + 1 com m=1. Aqui mede-se a consequência:
     * toda potência do rei é combinação inteira de 1 e σ, com Fibonacci nos coeficientes. */
    int mau = 0;
    printf("      n     σⁿ = F(n)·σ + F(n−1)     coeficientes\n");
    long a = 0, b = 1;        /* σ⁰ = 1 = 0·σ + 1 */
    for(int n = 0; n < 25; n++){
        if(a != F[n] || b != F[n-1 < 0 ? 0 : n-1]) { if(n > 0) mau++; }
        if(n <= 5 || n == 24)
            printf("      %-5d %-24s %ld·σ + %ld\n", n, "σⁿ", a, b);
        long na = a + b, nb = a;   /* σ·(aσ+b) = aσ² + bσ = a(σ+1) + bσ = (a+b)σ + a */
        a = na; b = nb;
    }
    ok("σⁿ = F(n)·σ + F(n−1) — a assinatura está DENTRO da figura", mau == 0);
    printf("\n      A maçã aberta na transversal traz o pentagrama: cinco lados iguais, razão 1.\n");
    printf("      E a diagonal dele obedece d² = d + 1, que é a equação do gato do ouro. Não é\n");
    printf("      que o pentagrama LEMBRE a assinatura — ele a satisfaz.\n");
}

/* ---------------------------------------------------------------- §A3 ------ */
printf("\n§A3  AS COORDENADAS DO REI: todo inteiro cabe, e de um jeito só.\n\n");
{
    /* Zeckendorf: soma de Fibonacci NÃO CONSECUTIVOS. Mede-se existência e unicidade por
     * construção gulosa mais contagem exaustiva de todas as somas possíveis. */
    long ate = 5000;
    int falta = 0, duplo = 0;
    for(long x = 1; x <= ate; x++){
        long r = x; int usados[NF], nu = 0, ultimo = 99;
        for(int i = NF-1; i >= 2; i--)
            if(F[i] <= r){ r -= F[i]; usados[nu++] = i; }
        if(r != 0) falta++;
        for(int t = 1; t < nu; t++) if(usados[t-1] - usados[t] < 2) duplo++;
        (void)ultimo;
    }
    /* unicidade: conta TODAS as somas de Fibonacci não consecutivos até o limite e vê se dá
     * exatamente um por inteiro — sem gulosidade, por força bruta em subconjuntos válidos */
    static int conta[5001];
    for(long t = 0; t <= ate; t++) conta[t] = 0;
    for(long mask = 0; mask < (1L << 16); mask++){
        if(mask & (mask >> 1)) continue;        /* proíbe consecutivos */
        long s = 0;
        for(int i = 0; i < 16; i++) if((mask >> i) & 1) s += F[i+2];
        if(s >= 1 && s <= ate) conta[s]++;
    }
    int sem = 0, mais = 0;
    for(long t = 1; t <= 610; t++){             /* até onde os 16 primeiros cobrem sem falha */
        if(conta[t] == 0) sem++;
        if(conta[t] > 1) mais++;
    }
    printf("      inteiros de 1 a %ld pela construção gulosa\n", ate);
    printf("      nenhum sobrou resto           %s\n", falta?"NÃO":"sim ✓");
    printf("      nenhum usou consecutivos      %s\n", duplo?"NÃO":"sim ✓");
    printf("      e por força bruta até 610: sem representação %d, com mais de uma %d\n", sem, mais);
    ok("todo inteiro cabe nas coordenadas do rei", falta == 0 && sem == 0);
    ok("e cabe de UMA só maneira — sem ambiguidade", duplo == 0 && mais == 0);
    printf("\n      É o sistema de numeração do rei, e ele é completo: qualquer outro se escreve\n");
    printf("      nele. Trocar o 1 pelo pomo de ouro não perde nada e não repete nada.\n");
}

/* ---------------------------------------------------------------- §A4 ------ */
printf("\n§A4  FORA DO JOGO REDONDO: o rei não tem termo grande. π tem 292.\n\n");
{
    /* Os termos da fração contínua controlam a qualidade da aproximação: com a_{n+1} grande,
     * q_{n+1} = a_{n+1}q_n + q_{n−1} dá um salto, e |x − p/q| < 1/(q_n q_{n+1}) despenca.
     * Termo grande = racional que quase acerta. Aqui compara-se o SALTO, em inteiros, sem
     * precisar do valor de x nenhum: só os termos entram. */
    int rei[10]  = {1,1,1,1,1,1,1,1,1,1};
    int pi[10]   = {7,15,1,292,1,1,1,2,1,3};   /* π = [3;7,15,1,292,…], termos conhecidos */
    long qr = 1, qr1 = 0, qp = 1, qp1 = 0;
    long saltomax_rei = 0, saltomax_pi = 0;
    printf("      passo  termo do rei  salto q(n+1)/q(n)   termo de π   salto\n");
    for(int t = 0; t < 6; t++){
        long nr = rei[t]*qr + qr1; qr1 = qr; qr = nr;
        long np = pi[t]*qp + qp1;  qp1 = qp; qp = np;
        long sr = qr / (qr1 ? qr1 : 1), sp = qp / (qp1 ? qp1 : 1);
        if(sr > saltomax_rei) saltomax_rei = sr;
        if(sp > saltomax_pi) saltomax_pi = sp;
        printf("      %-6d %-13d %-19ld %-12d %ld\n", t+1, rei[t], sr, pi[t], sp);
    }
    ok("o maior salto do rei é 2 — nunca mais que isso", saltomax_rei <= 2);
    ok("e o de π passa de 100: o redondo é bem aproximado por fração", saltomax_pi > 100);
    printf("\n      Termo grande na fração contínua é um racional que quase acerta — 355/113 dá π\n");
    printf("      a menos de um milionésimo, e é o 292 que o entrega. O rei não tem nenhum termo\n");
    printf("      grande: só uns, do começo ao fim. Nada nele dá para agarrar.\n");
    printf("\n      É isto que \"fora do jogo redondo\" quer dizer, e é medida, não figura: o jogo\n");
    printf("      redondo é o das frações que quase acertam, e o rei é o único ponto que nenhuma\n");
    printf("      delas alcança melhor do que o mínimo obrigatório.\n");
}

/* ---------------------------------------------------------------- §A5 ------ */
printf("\n§A5  SÓ CIFRANDO: multiplicar pelo rei é DESLOCAR. Não há multiplicação.\n\n");
{
    int mau = 0;
    printf("      (a,b)          ×σ dá        e é o par seguinte de Fibonacci?\n");
    long a = 1, b = 0;
    for(int t = 0; t < 12; t++){
        long na = a + b, nb = a;      /* σ·(aσ+b) = (a+b)σ + a — desloca, não multiplica */
        if(na != F[t+2] || nb != F[t+1]) mau++;
        if(t < 4 || t == 11)
            printf("      (%ld,%ld)%*s(%ld,%ld)%*s%s\n", a, b, (int)(12 - 4), "", na, nb,
                   (int)(10 - 4), "", (na==F[t+2]&&nb==F[t+1])?"sim ✓":"NÃO");
        a = na; b = nb;
    }
    ok("multiplicar pelo rei é um deslocamento — nenhum produto acontece", mau == 0);
    printf("\n      É o gato, e é só isso: (a,b) ↦ (a+b, a). Nas coordenadas do rei a operação\n");
    printf("      mais cara vira a mais barata, porque ele não participa da conta — ele REINDEXA.\n");
    printf("\n      E é por isso que ele fica fora e no infinito: quem é a unidade não entra na\n");
    printf("      soma. Cifrar, aqui, é exatamente isso — ser a régua e não a medida.\n");
}

printf("\n=== A COROAÇÃO ============================================================\n");
printf("  O rei é o pomo de ouro, e a assinatura dele é [1,1,1,1,…] — toda de uns.\n\n");
printf("    a maçã traz    o pentagrama de lados iguais, cuja diagonal obedece d² = d + 1,\n");
printf("                   que É a equação do gato do ouro. A assinatura está dentro da figura.\n\n");
printf("    as coordenadas todo inteiro se escreve em Fibonacci não consecutivos, e de uma só\n");
printf("                   maneira. Trocar o 1 pelo pomo não perde nem repete nada.\n\n");
printf("    fora do jogo   os termos dele são todos 1, e por isso nenhuma fração o alcança\n");
printf("                   melhor que o mínimo obrigatório. π tem um 292 e se deixa apanhar\n");
printf("                   por 355/113. O redondo é agarrável; o rei não.\n\n");
printf("    só cifrando    multiplicar por ele é (a,b) ↦ (a+b, a): um deslocamento. Ele não\n");
printf("                   entra na conta porque é a régua, e régua não se soma ao que mede.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
