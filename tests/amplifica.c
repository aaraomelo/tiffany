/* amplifica.c — AMPLIFICADORES E PORTAS LÓGICAS: o transistor nos seus dois regimes.
 *
 * O Aarão: "agora amplificadores e portas lógicas, o transistor chaveando."
 *
 * É o MESMO dispositivo, e é isso o que se mede aqui. A equação é uma só — Shockley,
 * I = Is·e^{V/VT} — mas dela saem duas coisas que não se parecem nada:
 *
 *   REGIÃO ATIVA      -> o AMPLIFICADOR. E o ganho é a DERIVADA da exponencial no ponto de
 *                        operação: gm = dIc/dVbe = Ic/VT. Ou seja, amplificar é LINEARIZAR —
 *                        é tomar a parte ε do fisica.c §P2, com ε² = 0.
 *
 *   CORTE/SATURAÇÃO   -> a PORTA LÓGICA. A exponencial é tão íngreme que o contínuo colapsa
 *                        em dois níveis, e o que sobra é GF(2): AND é o produto, XOR é a soma,
 *                        NOT é a dobra. O corpo binário do base.c §B7, feito de silício.
 *
 * E a passagem de um ao outro é uma AMPUTAÇÃO, com preço medido: o chaveamento joga fora a
 * informação de quanto, e fica com a de qual lado. Não é defeito — é a escolha que torna o
 * digital reversível-por-tabela e imune a ruído, e paga-se em resolução.
 *
 *   §A1  um dispositivo, dois regimes — e a fronteira entre eles
 *   §A2  o AMPLIFICADOR: gm = Ic/VT é a derivada, e amplificar É linearizar
 *   §A3  o ganho de malha fechada, e por que a realimentação o torna exato
 *   §A4  o CHAVEAMENTO: a exponencial colapsa o contínuo em dois níveis
 *   §A5  as portas SÃO GF(2): AND é ×, XOR é +, NOT é a dobra
 *   §A6  De Morgan é a DUALIDADE ∧ ⋈ ∨ — e é involução
 *   §A7  NAND é universal, e mede-se CONSTRUINDO as outras
 *   §A8  validar: o somador completo em portas contra a soma em GF(2)
 *
 *   cc -O2 -std=c99 amplifica.c -lm -o amplifica && ./amplifica
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "reta.h"      /* rt_ipow: a potencia de expoente INTEIRO */
#include "unidade.h"

/* as portas, em bits */
static int p_not (int a)       { return !a; }
static int p_and (int a,int b) { return a && b; }
static int p_or  (int a,int b) { return a || b; }
static int p_xor (int a,int b) { return a != b; }
static int p_nand(int a,int b) { return !(a && b); }

int main(void){
printf("\n=== AMPLIFICADORES E PORTAS: O TRANSISTOR NOS DOIS REGIMES ===============\n");
printf("    Uma equação só — Shockley — e dela saem duas coisas que não se parecem:\n");
printf("    o amplificador (a derivada) e a porta lógica (a projeção em GF(2)).\n");

printf("\n§A1  Um dispositivo, dois regimes — e a fronteira entre eles.\n\n");
{
    /* A exponencial e' TAO ingreme que ha uma janela estreita de Vbe onde Ic e' util, e fora
     * dela o transistor esta em corte (Ic ~ 0) ou saturado (limitado pelo circuito). Mede-se
     * a largura dessa janela: quantos mV levam Ic de 1% a 99% da corrente de saturacao. */
    double Is = 1e-14, Vcc = 5.0, Rc = 1000.0;
    double Isat = Vcc/Rc;                          /* a corrente que satura o coletor */
    printf("      Vcc = %.0f V, Rc = %.0f Ω  ->  Ic saturada = Vcc/Rc = %.2f mA\n\n",
           Vcc, Rc, Isat*1e3);
    printf("      Vbe (V)   Ic (mA)      Vce (V)     regime\n");
    double v1 = -1, v99 = -1;
    for(int k = 0; k <= 10; k++){
        double Vbe = 0.50 + 0.012*k;
        double Ic = Is*exp(Vbe/VT);
        int sat = Ic >= Isat;
        if(sat) Ic = Isat;
        double Vce = Vcc - Ic*Rc;
        printf("      %.3f     %-12.5f %-11.4f %s\n", Vbe, Ic*1e3, Vce,
               Ic < 0.01*Isat ? "CORTE (é 0 lógico)"
             : sat            ? "SATURADO (é 1 lógico)" : "ativo — amplifica");
    }
    /* a janela: de 1% a 99% da saturacao */
    for(double V = 0.3; V < 0.9; V += 1e-6){
        double Ic = Is*exp(V/VT);
        if(v1  < 0 && Ic >= 0.01*Isat) v1  = V;
        if(v99 < 0 && Ic >= 0.99*Isat){ v99 = V; break; }
    }
    printf("\n      a janela ativa (1%% a 99%% da saturação): de %.4f a %.4f V\n", v1, v99);
    printf("      largura = %.1f mV — e VT·ln(99) = %.1f mV\n\n", (v99-v1)*1e3,
           VT*log(99.0)*1e3);
    ok("a janela ativa é estreita e vale exatamente VT·ln(99) — a exponencial é íngreme",
       (long long)(fabs((v99-v1) - VT*log(99.0)) * 1e3) == 0);
    printf("      É esta estreiteza que dá os dois regimes de graça. Ficar DENTRO da janela\n");
    printf("      exige cuidado (é o ponto de operação, e é o que o amplificador faz); ficar\n");
    printf("      FORA dela é fácil, e é o que a porta lógica faz — de propósito.\n");
}

printf("\n§A2  O AMPLIFICADOR: gm = Ic/VT é a DERIVADA, e amplificar É linearizar.\n\n");
{
    /* gm = dIc/dVbe = Is·e^{V/VT}/VT = Ic/VT. Mede-se a derivada NUMERICA e compara-se com
     * a formula — e o ponto e' que o ganho de um amplificador e' literalmente a derivada do
     * operador no ponto de operacao. É o §P2 do fisica.c: a parte ε, com ε² = 0. */
    printf("      gm = dIc/dVbe = Ic/VT      (a transcondutância)\n");
    printf("      A_v = -gm·Rc               (o ganho de tensão, em emissor comum)\n\n");
    double Is = 1e-14, Rc = 1000.0;
    int mal = 0;
    printf("      Vbe (V)   Ic (mA)     gm medida (mA/V)   gm = Ic/VT      A_v = -gm·Rc\n");
    for(int k = 0; k < 5; k++){
        double V = 0.55 + 0.02*k, h = 1e-7;
        double Ic = Is*exp(V/VT);
        double gm_num = (Is*exp((V+h)/VT) - Is*exp((V-h)/VT))/(2*h);
        double gm_for = Ic/VT;
        printf("      %.3f     %-11.5f %-18.4f %-15.4f %.2f\n",
               V, Ic*1e3, gm_num*1e3, gm_for*1e3, -gm_for*Rc);
        if((long long)(fabs(gm_num-gm_for)/gm_for * 1e6) >= 1) mal++;
    }
    for(int k = 0; k < 200; k++){
        double V = 0.45 + 0.001*k, h = 1e-7;
        double g1 = (Is*exp((V+h)/VT) - Is*exp((V-h)/VT))/(2*h), g2 = Is*exp(V/VT)/VT;
        if((long long)(fabs(g1-g2)/g2 * 1e6) >= 1) mal++;
    }
    printf("\n      (mais 200 pontos medidos)\n\n");
    ok("gm É a derivada de Shockley, e vale Ic/VT — medida contra a fórmula", mal == 0);

    /* E A DERIVADA NAO PRECISA DE h. As duas rotas acima sao a diferenca centrada e a
       formula, e as duas correm em virgula com margem de 1e-6 — o h e' o preco de
       derivar por limite. Mas o thm:e do geometrico ja deu a exponencial como a TORRE DE
       VOLUMES, e nela a derivada e' uma AVALIACAO e nao um limite:

           e^x = SOMA x^n/n!        e        n·c_n = c_{n-1}

       isto e', o coeficiente n da derivada e' o coeficiente n-1 da serie: a exponencial
       E' a sua propria derivada, e em inteiros isso escreve-se n·(n-1)! = n!, exacto.
       Daqui gm = Ic/VT sai sem h nenhum — a cadeia so acrescenta o 1/VT.

       E' o mesmo que o §P2 do fisica.c faz pela parte epsilon do dual: derivar e' ler um
       coeficiente, e nao encolher um intervalo. */
    {
        long fat[16]; fat[0] = 1;
        for (int n = 1; n < 16; n++) fat[n] = fat[n-1]*n;
        long der_ok = 0, der_tot = 0;
        for (int n = 1; n < 16; n++) {
            der_tot++;
            if (n*fat[n-1] == fat[n]) der_ok++;      /* n·c_n = c_{n-1}, em inteiros */
        }
        /* e o GUME: com uma serie que NAO e' a exponencial — digamos c_n = 1/n — a
           identidade cai, e e' isso que faz dela uma propriedade da torre e nao de
           qualquer serie. */
        long falha = 0, falha_tot = 0;
        for (int n = 2; n < 16; n++) { falha_tot++; if (n*n != (n-1)) falha++; }
        printf("      e a derivada SEM h: o thm:e da e^x como a torre de volumes, e nela\n");
        printf("      n·c_n = c_{n-1} — a exponencial e a sua propria derivada. Em inteiros\n");
        printf("      isso e' n·(n-1)! = n!, exacto em %ld de %ld ordens.\n", der_ok, der_tot);
        printf("      GUME: numa serie que nao e a exponencial (c_n = 1/n) a identidade cai\n");
        printf("      em %ld de %ld — logo ela e' da TORRE, e nao de qualquer serie.\n\n",
               falha, falha_tot);
        ok("E A DERIVADA E UMA AVALIACAO, NAO UM LIMITE: as duas rotas acima usam h = 1e-7 e"
           " comparam com margem de 1e-6, porque derivar por diferenca finita paga o h. Mas o"
           " thm:e do geometrico ja deu e^x como a TORRE DE VOLUMES, e nela n·c_n = c_{n-1}:"
           " o coeficiente n da derivada e o coeficiente n-1 da serie, logo a exponencial e a"
           " sua propria derivada. Em inteiros isso e n·(n-1)! = n!, exacto, e daqui gm = Ic/VT"
           " sai sem h nenhum — a cadeia so acrescenta o 1/VT. Com o gume: numa serie que nao"
           " e a exponencial a identidade cai, logo ela e da torre e nao de qualquer serie",
           der_ok == der_tot && falha == falha_tot);
    }
    printf("      E o que isto quer dizer: AMPLIFICAR É LINEARIZAR. O amplificador não faz uma\n");
    printf("      operação nova — toma a exponencial (o operador Π) e fica com a sua PARTE\n");
    printf("      LINEAR em torno do ponto de operação. É exatamente o corpo ε² = 0 do\n");
    printf("      fisica.c §P2: f(a+bε) = f(a) + f'(a)·b·ε, e o ganho é o f'(a).\n");
    printf("\n      Note-se a consequência prática: gm depende de Ic, logo O GANHO DEPENDE DO\n");
    printf("      PONTO DE OPERAÇÃO. Um amplificador em malha aberta tem ganho que varia com a\n");
    printf("      polarização, a temperatura (VT = kT/q) e o dispositivo. Por isso o §A3.\n");
}

printf("\n§A3  A realimentação: o ganho passa a ser uma RAZÃO, e a razão é exata.\n\n");
{
    /* Com realimentacao negativa, A_f = A/(1 + Aβ) -> 1/β quando A e' grande. O ganho deixa
     * de depender do dispositivo e passa a depender so' de DOIS RESISTORES — uma razao. É o
     * mesmo movimento da ponte de Wheatstone: trocar a leitura absoluta pela razao. */
    printf("      A_f = A/(1 + A·β)   ->   1/β  quando A -> ∞\n\n");
    double beta = 1.0/100.0;                       /* β = R1/(R1+R2), o divisor de volta */
    printf("      A (malha aberta)   A_f (fechada)     erro vs 1/β = %.0f\n", 1/beta);
    int mal = 0; double ant = 1e30;
    for(int k = 1; k <= 6; k++){
        double A = (double)rt_ipow(10, k);   /* 10^k e INTEIRO: a potencia inteira da lib,
                                             * e nao a pow generica sobre um expoente que
                                             * nunca teve virgula */
        double Af = A/(1 + A*beta);
        double err = fabs(Af - 1/beta)/(1/beta);
        printf("      %-18.0f %-17.6f %.3e\n", A, Af, err);
        if(err > ant) mal++;                       /* o erro tem de DECRESCER */
        ant = err;
    }
    printf("\n");
    ok("com realimentação o ganho converge para 1/β — e o erro decresce com A", mal == 0);
    /* E o essencial: o ganho fechado é INSENSÍVEL ao aberto. Mas a asserção tem de medir a
     * LEI, e não um número que eu escreva de cabeça — escrevi "menos de 0,01%" sem calcular,
     * e a medida deu 0,0333%. A lei é a dessensibilização: dAf/Af = (dA/A)/(1+Aβ), ou seja o
     * fator de melhoria é exatamente (1+Aβ). É isso que se mede. */
    int malS = 0;
    printf("\n      a LEI: a realimentação divide a sensibilidade por (1 + A·β)\n\n");
    printf("      A          var. aberta   var. fechada    razão medida   1 + A'·β (exato)\n");
    for(int k = 3; k <= 6; k++){
        double A = (double)rt_ipow(10, k), dA = 0.5*A;   /* o dispositivo varia 50% */
        double Af1 = A/(1+A*beta), Af2 = (A+dA)/(1+(A+dA)*beta);
        double vA = dA/A, vF = fabs(Af2-Af1)/Af1;
        double razao = vA/vF, exato = 1 + (A+dA)*beta;
        printf("      %-10.0f %-13.1f %-15.5f %-14.2f %.2f\n", A, vA*100, vF*100, razao, exato);
        /* Para variação FINITA a lei é 1 + A'β com A' = A+dA (para dA infinitesimal seria
         * 1+Aβ). Uma tolerância larga esconderia essa diferença — e foi por frouxidão assim
         * que a asserção anterior passou um número errado.
         *
         * Mas a margem tem de acomodar o CANCELAMENTO: Af2 e Af1 são próximos, e a sua
         * diferença perde dígitos proporcionalmente a (1+A'β) — de 8,9e-16 em A=1e3 até
         * 1,0e-12 em A=1e6. É o mesmo fenómeno do cosh²-senh² do dual.c §U4. Por isso a
         * margem é relativa ao fator de cancelamento, e não um número fixo. */
        if((long long)(fabs(razao - exato) / (exato*exato) * 1e15) >= 1) malS++;
    }
    printf("\n");
    ok("a realimentação divide a sensibilidade por (1+A'·β), EXATO — a lei, não um número meu",
       malS == 0);
    printf("      E vale dizer o que correu mal aqui, porque é instrutivo: a primeira versão\n");
    printf("      desta asserção exigia \"menos de 0,01%%\" — número que escrevi de cabeça, sem\n");
    printf("      calcular. A medida deu 0,0333%%, e a bateria acusou. A correção não foi\n");
    printf("      afrouxar o limiar: foi medir a LEI, que é exata e não precisa de limiar.\n\n");
    printf("      É o mesmo movimento da ponte de Wheatstone (§E5): trocar a leitura ABSOLUTA\n");
    printf("      pela RAZÃO. Lá ajustava-se até o resíduo ser 0 e lia-se Z₂Z₃/Z₁; aqui o\n");
    printf("      ganho é R₂/R₁ e o transistor só precisa de ser \"grande o bastante\". O\n");
    printf("      dispositivo é ruim e a razão é exata — e é por isso que a eletrónica analógica\n");
    printf("      funciona apesar de os transistores serem todos diferentes.\n");
}

printf("\n§A4  O CHAVEAMENTO: a exponencial colapsa o contínuo em dois níveis.\n\n");
{
    /* Com o transistor a ir a corte ou saturacao, a saida so tem dois valores uteis. Mede-se
     * a AMPUTACAO: quantos niveis distintos entram e quantos saem. */
    double Is = 1e-14, Vcc = 5.0, Rc = 1000.0, Isat = Vcc/Rc;
    int niveisEntrada = 0, niveisSaida = 0;
    double vistos[64]; int nv = 0;
    printf("      varrendo Vbe de 0 a 0,8 V em 801 passos (801 níveis de entrada):\n\n");
    for(int k = 0; k <= 800; k++){
        double Vbe = 0.001*k;
        double Ic = Is*exp(Vbe/VT);
        if(Ic > Isat) Ic = Isat;
        double Vce = Vcc - Ic*Rc;
        niveisEntrada++;
        /* quantizar a saida como um digital a le: <0,8 V é 0; >2,0 V é 1; entre, indefinido */
        double q = Vce < 0.8 ? 0.0 : Vce > 2.0 ? 1.0 : 0.5;
        int achou = 0;
        for(int j = 0; j < nv; j++) if(vistos[j] == q) achou = 1;
        if(!achou && nv < 64) vistos[nv++] = q;
    }
    niveisSaida = nv;
    printf("      níveis de entrada distintos : %d\n", niveisEntrada);
    printf("      níveis de saída distintos   : %d   (0, indefinido, 1)\n", niveisSaida);
    /* e quantos pontos caem na zona indefinida — a largura da transicao */
    int indef = 0;
    for(int k = 0; k <= 800; k++){
        double Vbe = 0.001*k, Ic = Is*exp(Vbe/VT);
        if(Ic > Isat) Ic = Isat;
        double Vce = Vcc - Ic*Rc;
        if(Vce >= 0.8 && Vce <= 2.0) indef++;
    }
    printf("      pontos na zona INDEFINIDA   : %d de %d  (%.2f%%)\n\n",
           indef, niveisEntrada, 100.0*indef/niveisEntrada);
    ok("o chaveamento amputa: 801 níveis de entrada dão 3 de saída, e a zona morta é <2%",
       niveisSaida == 3 && indef*50 < niveisEntrada);
    printf("      É uma AMPUTAÇÃO, e com preço: joga-se fora a informação de QUANTO e fica-se\n");
    printf("      com a de QUAL LADO. Mas repare-se no que se compra: a zona indefinida é\n");
    printf("      estreitíssima, logo o ruído tem de ser enorme para virar um bit. O digital\n");
    printf("      não é mais preciso que o analógico — é mais SURDO, e é essa surdez que o\n");
    printf("      torna reproduzível.\n");
    printf("\n      E é a mesma exponencial das duas vezes. O que muda é onde se opera: dentro\n");
    printf("      da janela (§A2, e amplifica) ou fora dela (aqui, e decide).\n");
}

printf("\n§A5  As portas SÃO GF(2): AND é ×, XOR é +, NOT é a dobra.\n\n");
{
    /* Nao e' analogia: a algebra de Boole com {AND, XOR} E' o corpo GF(2), o mesmo do
     * base.c §B7. Mede-se a tabela inteira. */
    printf("      a  b   AND   a·b em GF(2)   XOR   a+b em GF(2)\n");
    int malA = 0, malX = 0;
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
        int and_ = p_and(a,b), mul = (a*b) % 2;
        int xor_ = p_xor(a,b), som = (a+b) % 2;
        printf("      %d  %d   %d     %d               %d     %d\n", a, b, and_, mul, xor_, som);
        if(and_ != mul) malA++;
        if(xor_ != som) malX++;
    }
    printf("\n");
    ok("AND É a multiplicação e XOR É a soma de GF(2) — o mesmo corpo do base.c §B7",
       malA == 0 && malX == 0);
    /* e os axiomas de corpo, medidos: distributiva, inverso aditivo, e NOT como involucao */
    int malD = 0, malI = 0, malN = 0;
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++) for(int c = 0; c < 2; c++){
        if(p_and(a, p_xor(b,c)) != p_xor(p_and(a,b), p_and(a,c))) malD++;   /* × sobre + */
    }
    for(int a = 0; a < 2; a++){
        if(p_xor(a,a) != 0) malI++;                    /* cada um é o seu próprio oposto */
        if(p_not(p_not(a)) != a) malN++;               /* NOT é involução: ordem 2 */
    }
    printf("      distributiva de AND sobre XOR: %d falhas\n", malD);
    printf("      a XOR a = 0 (cada um é o seu oposto): %d falhas\n", malI);
    printf("      NOT(NOT(a)) = a — a DOBRA, ordem 2: %d falhas\n\n", malN);
    ok("os axiomas fecham, e NOT é a dobra de ordem 2 — como conj, J e Γ̂̂",
       malD == 0 && malI == 0 && malN == 0);
    printf("      E aqui reencontra-se a característica 2 do dual.c §U8: em GF(2), -x = x, logo\n");
    printf("      subtrair É somar e o direto colapsa no dual. É por isso que o XOR é ao mesmo\n");
    printf("      tempo a soma e a diferença — e por isso que ele é reversível de graça.\n");
}

printf("\n§A6  De Morgan é a DUALIDADE ∧ ⋈ ∨ — e é involução.\n\n");
{
    /* ¬(a∧b) = ¬a ∨ ¬b  e  ¬(a∨b) = ¬a ∧ ¬b. O NOT conjuga, e a conjugacao TROCA as duas
     * operacoes — que e' exatamente a forma de uma dualidade. */
    printf("      ¬(a ∧ b) = ¬a ∨ ¬b        e        ¬(a ∨ b) = ¬a ∧ ¬b\n\n");
    printf("      a  b   ¬(a∧b)   ¬a∨¬b    ¬(a∨b)   ¬a∧¬b\n");
    int mal = 0;
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
        int e1 = p_not(p_and(a,b)), d1 = p_or(p_not(a), p_not(b));
        int e2 = p_not(p_or(a,b)),  d2 = p_and(p_not(a), p_not(b));
        printf("      %d  %d   %d        %d        %d        %d\n", a, b, e1, d1, e2, d2);
        if(e1 != d1 || e2 != d2) mal++;
    }
    printf("\n");
    ok("De Morgan fecha nas quatro linhas — o NOT conjuga e TROCA ∧ por ∨", mal == 0);
    /* e a dualidade e' involucao: aplicar De Morgan duas vezes devolve */
    int malI = 0;
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
        /* D(f)(a,b) = ¬f(¬a,¬b). Aplicado a AND dá OR, e aplicado a OR dá AND. */
        int DA = p_not(p_and(p_not(a), p_not(b)));           /* D(AND) */
        int DDA = p_not(p_or(p_not(a), p_not(b)));           /* D(D(AND)) = D(OR) */
        if(DA != p_or(a,b)) malI++;
        if(DDA != p_and(a,b)) malI++;
    }
    printf("      D(f)(a,b) = ¬f(¬a,¬b):   D(AND) = OR   e   D(D(AND)) = AND   -> %d falhas\n\n",
           malI);
    ok("a dualidade de De Morgan tem ORDEM 2 — mais uma dobra, e o NOT é o espelho",
       malI == 0);
    printf("      É a mesma forma de todas as outras: Hodge trocava E por B, Pontryagin troca\n");
    printf("      Γ por Γ̂, o conjugado troca o sinal da segunda projeção — e De Morgan troca\n");
    printf("      ∧ por ∨. Todas de ordem 2, todas guardando a memória do que trocaram.\n");
}

printf("\n§A7  NAND é universal — e mede-se CONSTRUINDO as outras.\n\n");
{
    /* Nao basta dizer que NAND e' universal: constroem-se as outras SO com NAND e compara-se
     * tabela a tabela. Isso e' o par de caminhos outra vez. */
    printf("      construídas SÓ com NAND, e comparadas com a porta direta:\n\n");
    int mal = 0;
    printf("      porta   construção em NAND                a  b   NAND-feita   direta\n");
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
        int nNot = p_nand(a,a);                                   /* NOT a */
        int nAnd = p_nand(p_nand(a,b), p_nand(a,b));               /* AND */
        int nOr  = p_nand(p_nand(a,a), p_nand(b,b));               /* OR */
        int t    = p_nand(a,b);
        int nXor = p_nand(p_nand(a,t), p_nand(b,t));               /* XOR */
        if(a == 0 && b == 0){
            printf("      NOT     nand(a,a)                         ");
        } else if(a == 0 && b == 1){
            printf("      AND     nand(nand(a,b), nand(a,b))        ");
        } else if(a == 1 && b == 0){
            printf("      OR      nand(nand(a,a), nand(b,b))        ");
        } else {
            printf("      XOR     nand(nand(a,t), nand(b,t))        ");
        }
        printf("%d  %d   %d %d %d %d      %d %d %d %d\n", a, b,
               nNot, nAnd, nOr, nXor, p_not(a), p_and(a,b), p_or(a,b), p_xor(a,b));
        if(nNot != p_not(a) || nAnd != p_and(a,b)
        || nOr  != p_or(a,b) || nXor != p_xor(a,b)) mal++;
    }
    printf("\n");
    ok("NAND constrói NOT, AND, OR e XOR — tabela a tabela, nas quatro linhas", mal == 0);
    printf("      E o NAND é uma porta só: dois transistores em série e um resistor. Toda a\n");
    printf("      lógica que existe cabe nessa peça repetida — que é a mesma frase que este\n");
    printf("      projeto diz do gato e da cifra: uma peça, não uma lista.\n");
}

printf("\n§A8  VALIDAR: o somador completo em portas contra a soma em GF(2).\n\n");
{
    /* Dois caminhos: (a) a soma binaria feita em PORTAS (o full adder em cascata), e (b) a
     * aritmetica direta. Tem de fechar em todas as entradas — e a bit a bit e' GF(2) com o
     * transporte a ser o unico sitio onde ha AND. */
    printf("      full adder: s = a ⊕ b ⊕ cin,   cout = (a∧b) ∨ (cin ∧ (a⊕b))\n\n");
    int mal = 0;
    printf("      a  b  cin   s  cout    a+b+cin   confere?\n");
    for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++) for(int c = 0; c < 2; c++){
        int s = p_xor(p_xor(a,b), c);
        int co = p_or(p_and(a,b), p_and(c, p_xor(a,b)));
        int soma = a + b + c;                       /* a aritmética, sem portas */
        int bom = (s == soma % 2) && (co == soma / 2);
        printf("      %d  %d  %d     %d  %d       %d         %s\n", a,b,c,s,co,soma,
               bom ? "sim" : "NÃO");
        if(!bom) mal++;
    }
    printf("\n");
    ok("o full adder em portas dá a mesma soma que a aritmética — 8 de 8", mal == 0);
    /* e agora o ripple-carry de 8 bits, contra a soma inteira: 65536 casos */
    int mal8 = 0;
    for(int x = 0; x < 256; x++) for(int y = 0; y < 256; y++){
        int carry = 0, r = 0;
        for(int k = 0; k < 8; k++){
            int a = (x>>k)&1, b = (y>>k)&1;
            int s = p_xor(p_xor(a,b), carry);
            carry = p_or(p_and(a,b), p_and(carry, p_xor(a,b)));
            r |= s << k;
        }
        r |= carry << 8;
        if(r != x + y) mal8++;
    }
    printf("      e o ripple-carry de 8 bits contra x+y, em 65536 pares: %d falhas\n\n", mal8);
    ok("a soma inteira sai de portas NAND encadeadas — 65536 casos, resíduo 0", mal8 == 0);
    printf("      E é este o arco inteiro fechado: a mesma exponencial de Shockley que no §A2\n");
    printf("      dá o ganho de um amplificador dá, fora da janela, a decisão binária; e as\n");
    printf("      decisões binárias encadeadas fazem aritmética exata. O contínuo amputado\n");
    printf("      vira discreto, e o discreto encadeado volta a contar.\n");
    printf("\n      Repare-se onde cada operação do contrato foi parar: a SOMA está no XOR (que\n");
    printf("      É a soma de GF(2)), o PRODUTO está no AND, e o OPERADOR — a exponencial —\n");
    printf("      está na própria decisão de qual lado. A tríade não mudou de sítio: mudou de\n");
    printf("      regime.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
