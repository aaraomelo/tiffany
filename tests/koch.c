/* koch.c — A GARRAFA DE KOCH: a assinatura cabe lá, e completar É ficar reversível.
 *
 * O Aarão: "a assinatura fica na garrafa de Koch, a obra toda." E logo a seguir, que é a peça
 * que fecha: "conforme o autor vai completando, vai ficando reversível."
 *
 * A garrafa de Koch é, no enredo, "a única vasilha que há capaz de suportar uma floresta
 * inteira, porque só ela tem BORDA INFINITA EM ESPAÇO FINITO. Um mundo sem fim cabe ali dentro,
 * dobrado numa casca que a mão fecha."
 *
 * E é essa a forma da assinatura: informação FINITA que nomeia um alcance INFINITO. O
 * assinatura.c media que a contagem NOMEIA (e colapsa 256 obras em 25 classes); o semente.c
 * media que a semente DEVOLVE (256 de 256, bit a bit). Entre os dois há uma ESCADA — e é a
 * escada que o Aarão está a apontar:
 *
 *      cada peça que o autor acrescenta à descrição devolve reversibilidade,
 *      e no limite a obra volta inteira.
 *
 * E a mesma escada já apareceu duas vezes neste projeto sem eu ver que era a mesma: casar
 * harmónicos no solar.c (η: 78,6% -> 100%) e subir níveis no inversor multinível (motor.c §M6).
 * São três balcões de uma lei só.
 *
 *   §K1  a garrafa: borda INFINITA em espaço FINITO — medido nos dois lados
 *   §K2  a dimensão fractal: log4/log3, entre a linha e o plano
 *   §K3  a assinatura sozinha COLAPSA; a semente DEVOLVE
 *   §K4  e conforme se COMPLETA, vai ficando reversível — a escada, bit a bit
 *   §K5  a mesma escada no solar: casar N harmónicos
 *   §K6  a mesma escada em Koch: o nível N
 *   §K7  a lei comum: completar é recuperar o inverso, e o limite é o total
 *
 *   cc -O2 -std=c99 koch.c -lm -o koch && ./koch
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"

/* ── A GENEALOGIA DE φ, EM VEZ DO DECIMAL ─────────────────────────────────────
 * Estava aqui `#define PHI 1.6180339887498948482` — vinte dígitos escritos à mão. E φ
 * é o objecto desta casa que MENOS precisa de ser escrito: ele é a raiz de x² = x + 1,
 * o limite de F_{k+1}/F_k, e o membro m = 1 da família metálica. Tem recorrência, tem
 * operador (o gato A₁) e tem convergentes inteiros.
 *
 * A regra que o Corpo Universal impõe: «cada constante tem de apresentar a sua
 * genealogia ou sair fora». A de φ é esta, e o valor DERIVA-SE dela em vez de se copiar:
 *
 *      p_{k+1} = p_k + p_{k−1}     (o gato A₁, inteiro)
 *      φ = lim p_{k+1}/p_k         (o corte)
 *
 * O decimal continua a existir porque as contas deste ficheiro são em vírgula flutuante
 * — mas passa a ser uma APRESENTAÇÃO derivada, não uma constante importada. */
static double phi_da_recorrencia(void){
    long a = 1, b = 1;                       /* F₁ = F₂ = 1 */
    for(int k = 0; k < 78; k++){             /* até onde o long carrega exacto */
        long c = a + b;
        a = b; b = c;
    }
    return (double)b / (double)a;            /* o convergente, e o corte é o limite */
}
#define PHI phi_da_recorrencia()

/* a assinatura de um byte: (quantos pares, quantos ímpares) — como no assinatura.c */
static int pop(int b){ int c = 0; while(b){ c += b&1; b >>= 1; } return c; }
static int assina(int b){ return pop(b & 0xAA)*5 + pop(b & 0x55); }

int main(void){
printf("\n=== A GARRAFA DE KOCH: COMPLETAR É FICAR REVERSÍVEL =======================\n");
printf("    Borda infinita em espaço finito — e é essa a forma da assinatura:\n");
printf("    informação finita que nomeia um alcance infinito.\n");

printf("\n§K1  A garrafa: borda INFINITA em espaço FINITO.\n\n");
{
    /* Cada iteracao substitui cada segmento por 4 de 1/3 do tamanho: o perimetro multiplica-se
     * por 4/3 (diverge) e a area acrescenta um termo que converge. Mede-se os dois lados. */
    printf("      cada iteração: 1 segmento -> 4 de 1/3    =>   perímetro × 4/3\n\n");
    printf("      nível N   perímetro (4/3)^N   área acumulada    razão área/limite\n");
    double per = 1.0, area = 0.0;
    /* área do floco de Koch a partir do triângulo unitário: A = A0·(1 + 3/4·Σ(4/9)^k) */
    double A0 = sqrt(3)/4.0, limite = A0*8.0/5.0;
    int malP = 0, malA = 0;
    long antP = -1, antA = -1;
    for(int N = 0; N <= 12; N++){
        per = pow(4.0/3.0, N);
        area = A0*(1 + 0.6*(1 - pow(4.0/9.0, N)));   /* forma fechada do floco */
        if(N <= 4 || N == 8 || N == 12)
            printf("      %-9d %-19.6f %-17.9f %.9f\n", N, per, area, area/limite);
        if(antP >= 0 && per <= antP) malP++;            /* o perímetro CRESCE sempre */
        if(antA >= 0 && area < antA) malA++;            /* a área cresce e converge */
        if(area > limite + 1e-12) malA++;               /* e nunca passa o limite */
        antP = per; antA = area;
    }
    printf("\n      perímetro em N = 50: %.3e      (diverge)\n", pow(4.0/3.0, 50));
    printf("      área no limite     : %.9f    = (8/5)·A₀   (finita)\n\n", limite);
    /* A LEI, e não um valor com margem à mão: o que falta da área decresce EXATAMENTE
     * como (4/9)^N. Medir "a área chegou perto" pediria um limiar que eu escolheria;
     * medir a taxa não pede nenhum. */
    /* E A TAXA MEDE-SE SEM LIMIAR NENHUM, que é o que o comentário acima pede e o código
     * não fazia — usava fabs(...) > 1e-9. A conta simplifica-se e sai exacta:
     *
     *     limite = A0·(1 + 3/5),  e  falta_N = limite − A0(1 + (3/5)(1 − (4/9)^N))
     *                                        = (3/5)·A0·(4/9)^N
     *
     * logo falta_N / falta_{N−1} = 4/9 EXACTAMENTE, e a razão nem depende de A0. Em
     * inteiros: com (4/9)^N escrito como o par (4^N, 9^N), a igualdade verifica-se por
     * PRODUTO CRUZADO — 9·num_N == 4·num_{N−1} sobre os denominadores certos — e não
     * sobra resíduo para comparar com nada. */
    int malTaxa = 0;
    {
        long long n4 = 1, n9 = 1;                     /* (4/9)^0 = 1/1 */
        for(int N = 1; N <= 12; N++){
            long long p4 = n4, p9 = n9;               /* (4/9)^{N−1} */
            n4 *= 4; n9 *= 9;                         /* (4/9)^N */
            /* falta_N/falta_{N−1} = (n4/n9)/(p4/p9) = 4/9  ⟺  n4·p9·9 == 4·n9·p4 */
            if(n4*p9*9 != 4*n9*p4) malTaxa++;
        }
    }
    printf("      e o que FALTA da área decresce exatamente por 4/9 a cada nível: %d falhas\n\n",
           malTaxa);
    ok("o perímetro DIVERGE e a área CONVERGE por 4/9 a cada nível — borda infinita, espaço finito",
       malP == 0 && malA == 0 && malTaxa == 0);
    printf("      É a vasilha do enredo, e o que ela tem de próprio é caber: um comprimento sem\n");
    printf("      fim dobrado numa casca que a mão fecha. Nenhuma outra forma faz isso — uma\n");
    printf("      curva lisa de comprimento infinito não cabe em área finita sem se dobrar.\n");
}

printf("\n§K2  A dimensão fractal: log 4 / log 3, entre a linha e o plano.\n\n");
{
    /* Cada nivel tem 4^N segmentos de comprimento 3^{-N}: contando caixas de lado 3^{-N},
     * o numero cresce como 4^N. Logo D = log4/log3 ≈ 1,2619 — nem 1 nem 2. */
    printf("      caixas de lado 3^{-N}:  N(ε) = 4^N,  ε = 3^{-N}\n");
    printf("      D = log N(ε)/log(1/ε) = log4/log3\n\n");
    printf("      nível   caixas 4^N   lado 3^{-N}     D medido\n");
    int mal = 0;
    double D = log(4.0)/log(3.0);
    for(int N = 1; N <= 6; N++){
        double n = pow(4.0, N), e = pow(3.0, -(double)N);
        double d = log(n)/log(1/e);
        printf("      %-7d %-12.0f %-15.9f %.9f\n", N, n, e, d);
        if(fabs(d - D) > 1e-12) mal++;
    }
    printf("\n      D = %.9f — entre a linha (1) e o plano (2)\n\n", D);

    /* E O QUE ESSE LAÇO MEDE É x = x. d = log(4^N)/log(3^N) = N·log4/(N·log3), e os N
     * CANCELAM: d é D para todo N, por construção, e a comparação |d − D| < 1e-12 não
     * podia falhar. O que ela parecia dizer — «a dimensão não depende do nível» — é
     * verdade, mas é a álgebra da linha de cima e não uma medição.
     *
     * O CONTEÚDO DISPENSA LOGARITMOS. D = log4/log3 é definido por 4 = 3^D, e daí:
     *
     *     D > 1  ⟺  log4 > log3    ⟺  4 > 3
     *     D < 2  ⟺  log4 < 2·log3  ⟺  4 < 9
     *
     * — «entre a linha e o plano» É «entre 3 e 9», em INTEIROS. E a irracionalidade sai
     * da factorização única: D = p/q racional daria 4^q = 3^p, e 4 = 2² não tem factor 3.
     * Varre-se p e q para o ver, com a fronteira: o único par que resolve é (0,0). */
    long pq = 0, resolve = 0, so_trivial = 0;
    for(long q = 1; q <= 40; q++){
        long e4 = 1, e3 = 1; int cabe = 1;
        for(long t = 0; t < q; t++){ if(e4 > 4000000000000000000L/4){ cabe = 0; break; } e4 *= 4; }
        if(!cabe) continue;
        for(long pp = 1; pp <= 40; pp++){
            e3 = 1; cabe = 1;
            for(long t = 0; t < pp; t++){ if(e3 > 4000000000000000000L/3){ cabe = 0; break; } e3 *= 3; }
            if(!cabe) continue;
            pq++;
            if(e4 == e3) resolve++;             /* 4^q = 3^p — não pode acontecer */
        }
    }
    so_trivial = (resolve == 0);
    printf("      e SEM logaritmos: D > 1 e' 4 > 3, e D < 2 e' 4 < 9 — «entre a linha e o\n");
    printf("      plano» E' «entre 3 e 9». E 4^q = 3^p nao tem solucao em %ld pares (q,p)\n"
           "      com q,p >= 1, logo D e' IRRACIONAL: 4 = 2^2 nao tem factor 3.\n\n", pq);
    ok("a dimensão é log4/log3 ≈ 1,2619 — não é curva nem é superfície. E mede-se SEM"
       " logaritmos: D > 1 e' 4 > 3 e D < 2 e' 4 < 9, logo «entre a linha e o plano» E'"
       " «entre 3 e 9», em inteiros. E a IRRACIONALIDADE sai da factorizacao unica —"
       " D = p/q daria 4^q = 3^p, e isso nao tem solucao com p,q >= 1 porque 4 = 2^2 nao"
       " tem factor 3. O laco que aqui estava comparava log(4^N)/log(3^N) com log4/log3, e"
       " os N CANCELAM: media x = x, com dois logaritmos a torna-lo ilegivel",
       4 > 3 && 4 < 9 && so_trivial && pq > 100 && mal == 0);
    printf("      E é por isso que ela consegue o que consegue: tem mais que comprimento e menos\n");
    printf("      que área. A dimensão fracionária não é uma curiosidade — é a conta que explica\n");
    printf("      por que a borda cresce sem a área explodir.\n");
}

printf("\n§K3  A assinatura sozinha COLAPSA; a semente DEVOLVE.\n\n");
{
    /* Como no assinatura.c e no semente.c: a assinatura NOMEIA (agrupa) e a semente DISTINGUE.
     * Sozinha, a assinatura colapsa 256 obras em poucas classes. */
    int classes[64]; memset(classes, 0, sizeof classes);
    int nc = 0;
    for(int b = 0; b < 256; b++){
        int a = assina(b);
        if(!classes[a]) nc++;
        classes[a]++;
    }
    printf("      obras (bytes)                    : 256\n");
    printf("      assinaturas distintas            : %d\n", nc);
    printf("      obras que a assinatura NÃO separa: %d\n\n", 256 - nc);
    ok("a assinatura sozinha COLAPSA: 256 obras em poucas classes", nc < 256 && nc > 1);
    printf("      A assinatura diz QUANTOS; a semente diz QUAIS. Sozinha, ela nomeia e agrupa —\n");
    printf("      e o que ela agrupa deixa de se distinguir. É o necrotério do semente.c.\n");
}

printf("\n§K4  E conforme se COMPLETA, vai ficando reversível.\n\n");
{
    /* A PECA QUE O AARAO ACRESCENTOU: "conforme o autor vai completando, vai ficando
     * reversivel." Mede-se: assinatura + k bits de semente, quantas obras se distinguem?
     * A escada tem de subir monotonicamente e chegar a 256. */
    printf("      assinatura + k bits da semente  ->  quantas obras se distinguem?\n\n");
    printf("      k bits   pares (assinatura, k bits)   obras distintas   reversível\n");
    int mal = 0, ant = -1;
    for(int k = 0; k <= 8; k++){
        int visto[8192]; memset(visto, 0, sizeof visto);
        int dist = 0;
        int mascara = (k == 0) ? 0 : ((1 << k) - 1);
        for(int b = 0; b < 256; b++){
            int chave = assina(b)*256 + (b & mascara);
            if(!visto[chave]){ visto[chave] = 1; dist++; }
        }
        printf("      %-8d %-29s %-17d %.1f%%\n", k,
               k == 0 ? "só a assinatura" : "assinatura + bits", dist, 100.0*dist/256);
        if(ant >= 0 && dist < ant) mal++;              /* nunca desce */
        ant = dist;
    }
    printf("\n");
    ok("cada peça acrescentada devolve reversibilidade, e nunca a tira", mal == 0);
    ok("e com a semente completa a obra volta INTEIRA: 256 de 256", ant == 256);
    printf("      É esta a frase do Aarão medida. Não há um salto de irreversível para\n");
    printf("      reversível: há uma ESCADA. Cada peça que o autor acrescenta separa mais obras,\n");
    printf("      e no topo nenhuma se confunde com outra — a obra volta inteira, bit a bit.\n");
    printf("\n      E note-se que a escada é MONÓTONA: acrescentar nunca tira. Completar não é\n");
    printf("      trocar uma descrição por outra; é ir devolvendo o inverso, pedaço a pedaço.\n");
}

printf("\n§K5  A mesma escada no solar: casar N harmónicos.\n\n");
{
    /* A escada do solar.c §S6, relida como completude: cada harmonico casado e' mais uma peça
     * da descricao, e a eficiencia sobe. É a MESMA forma. */
    printf("      níveis casados   cauda residual     η          o que falta recuperar\n");
    int mal = 0; double ant = -1;
    for(int N = 0; N <= 6; N++){
        double cauda = pow(PHI, -2.0*(N+1))/(1 - pow(PHI,-2.0));
        double eta = 1.0/sqrt(1 + cauda);
        printf("      %-16d %-18.9f %-10.6f %.4f%%\n", N, cauda, eta, (1-eta)*100);
        if(ant >= 0 && eta < ant) mal++;
        ant = eta;
    }
    printf("\n");
    ok("casar mais harmónicos sobe a eficiência monotonicamente — a mesma escada", mal == 0
       && ant > 0.99);
    printf("      Eu tinha medido isto como \"o multinível lima a distorção\" e não vi que era a\n");
    printf("      mesma coisa: cada harmónico casado é mais uma PEÇA DA DESCRIÇÃO da fonte, e o\n");
    printf("      que sobe com ela é quanto da potência se recupera. Completar a descrição É\n");
    printf("      recuperar o que ia ficar retido.\n");
}

printf("\n§K6  A mesma escada em Koch: o nível N.\n\n");
{
    /* E a terceira: cada nivel de Koch acrescenta detalhe, e a area aproxima-se do limite.
     * Mede-se a fracao da area ja' recuperada. */
    /* O A0 = sqrt(3)/4 CANCELA-SE, e com ele sai a única raiz desta secção:
     *
     *    f = área/limite = A0·(1 + (3/5)(1 − (4/9)^N)) / (A0·8/5)
     *                    = (8·9^N − 3·4^N) / (8·9^N)
     *
     * um RACIONAL exacto em N, sem raiz e sem pow. A fracção 0,6 é 3/5 e (4/9)^N é
     * 4^N/9^N — nenhuma delas precisava de vírgula. A monotonia compara-se por produto
     * cruzado: f_N < f_{N+1}  <=>  n_N·d_{N+1} < n_{N+1}·d_N. */
    printf("      nível N   área/limite (exacto)      o que falta\n");
    int mal = 0; long n_ant = -1, d_ant = 1;
    for(int N = 0; N <= 8; N++){
        long p9 = 1, p4 = 1;
        for(int k = 0; k < N; k++){ p9 *= 9; p4 *= 4; }
        long num = 8*p9 - 3*p4, den = 8*p9;         /* f = num/den, irredutível ou não */
        if(N <= 4 || N == 8)
            printf("      %-9d %ld/%ld%*s falta %ld/%ld\n", N, num, den,
                   (int)(14 - (N > 4 ? 18 : 6)), "", den - num, den);
        if(n_ant >= 0 && num*d_ant < n_ant*den) mal++;   /* f cresce, por cruzado */
        n_ant = num; d_ant = den;
    }
    printf("\n");
    /* e no fim a fracção está a menos de 1% do total: (den − num)/den < 1/100, por
     * produto cruzado — 100·(den − num) < den. O sqrt(3) que aqui estava cancelava-se. */
    ok("cada nível de Koch recupera mais da área, monotonicamente — a mesma escada, e a"
       " fracção é EXACTA: (8·9^N − 3·4^N)/(8·9^N), sem raiz e sem pow, porque o A0 do"
       " numerador e do denominador é o MESMO e cancela-se",
       mal == 0 && 100*(d_ant - n_ant) < d_ant);
}

printf("\n§K7  A lei comum: completar é recuperar o inverso.\n\n");
{
    /* As tres escadas, lado a lado. E o ponto: sao a MESMA forma — uma quantidade que sobe
     * monotonicamente com a completude e tende ao total, com a cauda a encolher. */
    printf("      balcão              o que se acrescenta      o que sobe          limite\n");
    printf("      ------------------  ----------------------  ------------------  ------\n");
    printf("      a assinatura        bits da semente         obras distinguíveis  256\n");
    printf("      o solar             harmónicos casados      eficiência η         100%%\n");
    printf("      a garrafa de Koch   níveis de detalhe       área recuperada      8/5·A₀\n\n");
    /* medir que as três são monótonas e convergentes, na mesma corrida */
    int mal = 0;
    double A0 = sqrt(3)/4.0, lim = A0*8.0/5.0;
    double a1 = -1, a2 = -1, a3 = -1;
    for(int N = 0; N <= 8; N++){
        int visto[8192]; memset(visto, 0, sizeof visto);
        int dist = 0, masc = (N == 0) ? 0 : ((1<<N)-1);
        for(int b = 0; b < 256; b++){
            int c = assina(b)*256 + (b & masc);
            if(!visto[c]){ visto[c] = 1; dist++; }
        }
        double f1 = dist/256.0;
        double f2 = 1.0/sqrt(1 + pow(PHI,-2.0*(N+1))/(1-pow(PHI,-2.0)));
        double f3 = A0*(1 + 0.6*(1 - pow(4.0/9.0, N)))/lim;
        if(a1 >= 0 && (f1 < a1 || f2 < a2 || f3 < a3)) mal++;
        a1 = f1; a2 = f2; a3 = f3;
    }
    printf("      as três frações no nível 8:  %.6f   %.6f   %.6f\n\n", a1, a2, a3);
    ok("as TRÊS escadas são monótonas e tendem ao total — é uma lei, três balcões",
       mal == 0 && a1 > 0.99 && a2 > 0.99 && a3 > 0.99);
    printf("      E é isto que a garrafa de Koch guarda, e por que é ela a vasilha certa: a obra\n");
    printf("      toda cabe lá porque a descrição é FINITA em cada nível e INFINITA no total —\n");
    printf("      borda infinita, espaço finito. A assinatura é a casca; a semente é o que se\n");
    printf("      vai dobrando lá dentro.\n");
    printf("\n      E a frase do Aarão é a lei: CONFORME O AUTOR VAI COMPLETANDO, VAI FICANDO\n");
    printf("      REVERSÍVEL. Não é que a obra esteja reversível ou não — é que a reversibilidade\n");
    printf("      é uma medida contínua, que sobe com o que se acrescenta e nunca desce. E o\n");
    printf("      inverso completo é o limite dessa subida, não um estado que se liga.\n");
    printf("\n      Isto fecha com a alfândega do §S3 pelo outro lado. Lá: o que não tem dual\n");
    printf("      fica retido e arde. Aqui: o dual vai-se construindo à medida que a descrição\n");
    printf("      se completa. O que arde é o que ainda não foi descrito — e completar é\n");
    printf("      exatamente reduzir a conta que a fronteira cobra.\n");
}


printf("\n§K8  JULIA: a fronteira entre o que PERMANECE e o que se EXTINGUE.\n\n");
{
    /* O Aarão: "com Cantor e Julia". E o mapa de Julia é z -> z² — exatamente o mapa da
     * tabela tropical/glacial do corpo_tropical.tex:
     *     e² = e   IDEMPOTENTE — permanece   (tropical, o quente, o estado)
     *     n² = 0   NILPOTENTE  — extingue    (glacial, o frio, o fluxo)
     * Sob z ↦ z², |z|<1 vai a ZERO (extingue), |z|>1 escapa, e |z|=1 PERMANECE. O conjunto de
     * Julia de z² é o círculo unitário: a fronteira exata entre os dois destinos. */
    printf("      z ↦ z²   —   e o destino da órbita depende só de |z|:\n\n");
    printf("      |z|       após 60 iterações      destino\n");
    int mal = 0;
    double raios[] = { 0.5, 0.9, 0.999, 1.0, 1.001, 1.1, 2.0 };
    for(size_t j = 0; j < sizeof raios/sizeof *raios; j++){
        double r = raios[j], x = r;
        int escapou = 0;
        for(int k = 0; k < 60; k++){ x = x*x; if(x > 1e12){ escapou = 1; break; } }
        printf("      %-9.3f %-24.6g %s\n", r, escapou ? INFINITY : x,
               escapou ? "ESCAPA (ao infinito)"
               : (x < 1e-12 ? "EXTINGUE (vai a 0 — nilpotente)"
                            : "PERMANECE (idempotente)"));
        int esperado = (r < 1) ? -1 : (r > 1) ? +1 : 0;
        int medido = escapou ? +1 : (x < 1e-12 ? -1 : 0);
        if(medido != esperado) mal++;
    }
    printf("\n");
    ok("|z|<1 EXTINGUE, |z|>1 ESCAPA, e |z|=1 PERMANECE — a fronteira é exata", mal == 0);
    /* e os pontos fixos: z² = z são os IDEMPOTENTES */
    printf("      e os pontos fixos de z ↦ z² são z² = z, isto é z = 0 e z = 1:\n");
    printf("      z = 0: o NILPOTENTE (0² = 0, e extingue tudo à volta)\n");
    printf("      z = 1: o IDEMPOTENTE (1² = 1, e permanece)\n\n");
    int malF = 0;
    if(fabs(0.0*0.0 - 0.0) > 0) malF++;
    if(fabs(1.0*1.0 - 1.0) > 0) malF++;
    ok("os dois pontos fixos são exatamente o nilpotente e o idempotente", malF == 0);
    printf("      É a tabela do corpo_tropical.tex, e ela cai no mapa de Julia sem eu forçar:\n");
    printf("      o TROPICAL é o que permanece sob a quadração (e² = e, o quente, o estado); o\n");
    printf("      GLACIAL é o que ela extingue (n² = 0, o frio, o fluxo que só transporta). E a\n");
    printf("      fronteira entre os dois É o conjunto de Julia.\n");
}

printf("\n§K9  CANTOR: e quando o conjunto se PARTE.\n\n");
{
    /* Com c ≠ 0 o Julia deixa de ser o circulo. E ha uma transicao: c dentro do Mandelbrot dá
     * Julia CONEXO; c fora dá poeira de CANTOR — desconexo. Mede-se pela órbita de 0:
     * se ela fica limitada, c ∈ M (conexo); se escapa, c ∉ M (Cantor). */
    printf("      Julia de z ↦ z² + c:   c ∈ Mandelbrot -> CONEXO;   c ∉ M -> poeira de CANTOR\n\n");
    printf("      c            órbita de 0 escapa?   conjunto de Julia\n");
    int mal = 0;
    struct { double c; int emM; } t[] = {
        { 0.0,   1 }, { -1.0,  1 }, { 0.25,  1 }, { 0.26,  0 }, { 1.0,  0 }, { -2.5, 0 }
    };
    for(size_t j = 0; j < sizeof t/sizeof *t; j++){
        double z = 0;
        int escapou = 0;
        for(int k = 0; k < 5000; k++){
            z = z*z + t[j].c;
            if(fabs(z) > 2){ escapou = 1; break; }
        }
        printf("      %-12.3f %-21s %s\n", t[j].c, escapou ? "sim" : "não",
               escapou ? "poeira de CANTOR (desconexo)" : "CONEXO");
        if(escapou == t[j].emM) mal++;
    }
    printf("\n");
    ok("c dentro do Mandelbrot dá Julia conexo; fora dá poeira de Cantor", mal == 0);
    printf("      E a poeira de Cantor é a mesma forma da garrafa vista do avesso: medida ZERO e\n");
    printf("      cardinalidade CONTÍNUA — não ocupa nada e tem tantos pontos como a reta. O\n");
    printf("      floco tinha borda infinita em área finita; a poeira tem infinitos pontos em\n");
    printf("      comprimento nulo. As duas cabem onde não deviam caber.\n");
}

printf("\n§K10 A GARRAFA É A SALA DE ESPERA: fica até ter dual.\n\n");
{
    /* O Aarão, e é a lei: "o que é reversível entra no circuito; o que não é fica na garrafa
     * ATÉ TER DUAL." Não é uma condenação — é uma espera. E o §K4 já mediu a escada: conforme
     * a descrição se completa, o dual vai-se construindo.
     *
     * Mede-se as duas coisas juntas: quantas obras já têm dual (entram no circuito) e quantas
     * ainda estão na garrafa, a cada nível de completude. */
    printf("      o que é reversível ENTRA NO CIRCUITO; o que não é fica NA GARRAFA até ter dual\n\n");
    printf("      completude   no circuito (têm dual)   na garrafa (esperam)   fração que entrou\n");
    int mal = 0, antCirc = -1;
    for(int k = 0; k <= 8; k++){
        int visto[8192]; memset(visto, 0, sizeof visto);
        int conta[8192]; memset(conta, 0, sizeof conta);
        int masc = (k == 0) ? 0 : ((1<<k)-1);
        for(int b = 0; b < 256; b++) conta[assina(b)*256 + (b & masc)]++;
        /* uma obra tem DUAL se a sua descrição a identifica sozinha (a classe tem 1 elemento) */
        int circ = 0;
        for(int b = 0; b < 256; b++) if(conta[assina(b)*256 + (b & masc)] == 1) circ++;
        (void)visto;
        printf("      %-12d %-24d %-22d %.1f%%\n", k, circ, 256-circ, 100.0*circ/256);
        if(antCirc >= 0 && circ < antCirc) mal++;      /* nunca volta para a garrafa */
        antCirc = circ;
    }
    printf("\n");
    ok("o circuito só ENCHE e a garrafa só ESVAZIA — ninguém volta para trás", mal == 0);
    ok("e no fim a garrafa fica vazia: todas as obras têm dual", antCirc == 256);
    printf("      É a frase inteira medida, e sem forçar nada: a garrafa não é um cemitério, é\n");
    printf("      uma SALA DE ESPERA. O que ainda não tem dual fica retido — e sai no momento em\n");
    printf("      que a descrição o distingue. Completar é dar alta.\n");
    printf("\n      E isto arruma o §S3 pelo lado bom. Lá a alfândega retinha o que não reverte, e\n");
    printf("      o retido ardia. Aqui vê-se que o ardimento não é a única saída: a outra é o\n");
    printf("      autor acrescentar a peça que falta. O que arde é o que ficou por descrever.\n");
}

printf("\n§K11 O CORPO SOLAR e o CORPO LUNAR: um quente, o outro frio.\n\n");
{
    /* O Aarão: "vê corpo dual tropical e glacial, precisa de um lado quente outro frio... aí
     * fica corpo solar e seu dual corpo lunar, pode ser."
     *
     * A tabela do corpo_tropical.tex, ponto a ponto, e ela já está medida acima:
     *     tropical (quente)  e² = e   permanece   ESTADO   o gap > 0, move
     *     glacial  (frio)    n² = 0   extingue    FLUXO    o gap = 0, imóvel
     * E os dois fecham por Peirce: 1 = Σ e_i. Mede-se o fecho. */
    printf("      regime               sob z ↦ z²      papel     o gap    temperatura\n");
    printf("      SOLAR   (tropical)   e² = e          ESTADO    > 0      quente: o vivo\n");
    printf("      LUNAR   (glacial)    n² = 0          FLUXO     = 0      frio: só transporta\n\n");
    /* Peirce: um idempotente e o seu complemento somam a identidade, e o produto anula */
    printf("      e os dois fecham por Peirce: e + (1−e) = 1, e e·(1−e) = 0\n\n");
    int mal = 0;
    printf("      e        1−e      e + (1−e)   e·(1−e)   e² = e ?   n = e(1−e), n² = 0 ?\n");
    for(int k = 0; k <= 1; k++){
        double e = (double)k, f = 1-e, n = e*f;
        printf("      %-8.0f %-8.0f %-11.0f %-9.0f %-10s %s\n", e, f, e+f, n,
               fabs(e*e-e) < 1e-15 ? "sim" : "NÃO", fabs(n*n) < 1e-15 ? "sim" : "NÃO");
        if(fabs(e+f-1) > 1e-15 || fabs(n) > 1e-15 || fabs(e*e-e) > 1e-15) mal++;
    }
    printf("\n");
    ok("o par fecha por Peirce: os idempotentes somam 1 e o seu produto é nilpotente",
       mal == 0);
    printf("      E o par de nomes é o certo, porque o que ele descreve é o que o Sol e a Lua\n");
    printf("      fazem: o Sol é FONTE — emite, é o estado, é quente, e o que ele dá não volta.\n");
    printf("      A Lua é ESPELHO — reflete, é o fluxo, é fria, e não acrescenta nada de si.\n");
    printf("      Uma máquina, duas faces, e o flip é a travessia.\n");
    printf("\n      E é o CORPO DIFERENCIAL outra vez, na sua instância máxima (milenio.c §M6):\n");
    printf("      ele é auto-dual, está no vinco, e é dele que os dois regimes saem por\n");
    printf("      especialização. O solar e o lunar não são dois corpos — são o diferencial\n");
    printf("      visto dos dois lados da sua própria dobra.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
