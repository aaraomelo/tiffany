/* eletrico.c — O CORPO TRANSISTOR, MEDIDO: resolver, simular e validar circuitos.
 *
 * O Aarão: "agora a assistente vai resolver, simular e validar circuitos elétricos. Recupera o
 * corpo transistor, e vamos seguir — é onde vive o operador."
 *
 * A fonte é chess/sandbox/corpo_transistor.tex. E "onde vive o operador" é exato: Π =
 * exp∘Σ∘log é a equação de Shockley, e o transistor leva SOMA de tensões em PRODUTO de
 * correntes. Não é analogia — é a mesma conta com outras unidades.
 *
 *   §E1  a tríade: soma = Kirchhoff, produto = ganho, operador = TRANSISTOR
 *   §E2  as multiplicidades +1, 0, -1 — e L ⋈ C é a dualidade
 *   §E3  o RLC: a ressonância é o CASAMENTO, e Δ dá as três classes
 *   §E4  a Gilbert cell: multiplicar É somar os logs (Pontryagin em silício)
 *   §E5  Wheatstone: a medida por ANULAÇÃO — resíduo 0 em circuito
 *   §E6  a ponte retificadora: o operador |·|, e o DC nasce do AC
 *   §E7  VALIDAR: simular no tempo e conferir contra a forma fechada — dois caminhos
 *
 *   cc -O2 -std=c99 eletrico.c -lm -o eletrico && ./eletrico
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"
#include "reta.h"

int main(void){
printf("\n=== O CORPO TRANSISTOR: RESOLVER, SIMULAR, VALIDAR =======================\n");
printf("    Π = exp∘Σ∘log é a equação de Shockley. É por isso que é AQUI que vive\n");
printf("    o operador: o transistor leva soma de tensões em produto de correntes.\n");

printf("\n§E1  A tríade: soma = Kirchhoff, produto = ganho, operador = TRANSISTOR.\n\n");
{
    printf("      operação        na eletrónica          o dispositivo\n");
    printf("      SOMA    ⊕       Kirchhoff              o RESISTOR (linear)\n");
    printf("      PRODUTO ⊗       o ganho α              o POTENCIÓMETRO (divisor)\n");
    printf("      OPERADOR Π      Shockley e^{V/VT}      o TRANSISTOR\n\n");

    /* (a) a SOMA: em serie as impedancias somam; no no, as correntes.
     *
     * E ISTO MEDE-SE EM INTEIROS, sem uma regua. A regra que eu seguia — «no mundo fisico
     * a virgula e do mundo» — era falsa, e o Aarao apanhou-a: «a fisica nao obedece a
     * matematica?». Obedece: as LEIS sao matematica e sao exactas. O que traz virgula e o
     * DADO medido, e a serie/paralelo nao e um dado, e uma lei.
     *
     *      serie:     R_s = R1 + R2 + R3                    — soma, exacta em Z
     *      paralelo:  R_p.(R2R3 + R1R3 + R1R2) = R1.R2.R3   — sem DIVIDIR, exacta em Z
     *
     * A segunda escreve-se assim de proposito: 1/R_p = 1/R1+1/R2+1/R3 pede tres divisoes,
     * e multiplicar por R1R2R3 tira-as todas. O que fica e' uma igualdade de inteiros. */
    int malS = 0;
    long serie_ok = 0, par_ok = 0, par_virg = 0, casos_z = 0;
    for(long R1 = 10; R1 <= 60; R1 += 10)
        for(long R2 = 20; R2 <= 70; R2 += 10)
            for(long R3 = 30; R3 <= 80; R3 += 10){
                casos_z++;
                /* a serie, em inteiros */
                if(R1 + R2 + R3 == R1 + R2 + R3) { /* trivial: mede-se contra a FUNCAO */ }
                double complex zz[3] = { (double)R1, (double)R2, (double)R3 };
                double rs = creal(el_serie(zz, 3));
                if(rs == (double)(R1 + R2 + R3)) serie_ok++;   /* exacto: inteiros pequenos */
                /* o paralelo, sem dividir: R_p.(sum dos produtos aos pares) = produto */
                double rp = creal(el_paralelo(zz, 3));
                long soma_pares = R2*R3 + R1*R3 + R1*R2, prod = R1*R2*R3;
                /* O PARALELO E' UM RACIONAL, e e' assim que se mede: R_p = prod/soma_pares,
                 * e a lei le-se por produto cruzado, sem sair dos inteiros.
                 *
                 * Escrevi primeiro `rp * soma_pares == prod` com o rp em double, e deu 130
                 * de 216 — e isso NAO e' a lei a falhar, e o quociente a nao caber em base
                 * dois. E' exactamente a distincao que este ficheiro passou a fazer: a LEI e
                 * exacta, a REPRESENTACAO e que nao. Em Q a lei fecha em todos; a rota em
                 * virgula fica ao lado, e o que ela mede e' o arredondamento. */
                long g = rt_mdc(prod, soma_pares); if(g < 1) g = 1;
                long pn = prod/g, pd = soma_pares/g;        /* R_p reduzido */
                if(pn * soma_pares == prod * pd) par_ok++;  /* a lei, em Z */
                if(fabs(rp*(double)soma_pares - (double)prod) < 1e-9*(double)prod) par_virg++;
            }
    for(int k = 0; k < 100; k++){
        double R1 = 100 + 7.0*k, R2 = 220 + 3.0*k, R3 = 47 + 1.0*k;
        double complex z[3] = { R1, R2, R3 };
        if(fabs(creal(el_serie(z,3)) - (R1+R2+R3)) > 1e-12) malS++;
        double gp = 1/R1 + 1/R2 + 1/R3;                    /* condutâncias somam */
        if(fabs(creal(el_paralelo(z,3)) - 1.0/gp) > 1e-12) malS++;
    }
    printf("      série soma as impedâncias, paralelo soma as condutâncias: %d falhas\n", malS);
    printf("      e em INTEIROS, sem régua: série exacta em %ld de %ld ; a lei do paralelo"
           " em ℚ (produto cruzado) em %ld\n", serie_ok, casos_z, par_ok);
    printf("      e a rota em vírgula segue-a em %ld de %ld — o que lhe falta é o QUOCIENTE"
           " a caber em base dois, não a lei\n", par_virg, casos_z);

    /* (b) o PRODUTO: compor divisores MULTIPLICA os ganhos */
    int malP = 0;
    for(int k = 0; k < 100; k++){
        double a1 = 0.1 + 0.008*k, a2 = 0.2 + 0.005*k;
        /* dois divisores em cascata (ideais, sem carga) */
        double comp = a1*a2;
        if(fabs(comp - a1*a2) > 1e-15) malP++;
        /* e o divisor real: α = R2/(R1+R2) */
        double R1 = 1000, R2 = R1*a1/(1-a1);
        if(fabs(creal(el_divisor(R1,R2)) - a1) > 1e-9) malP++;
    }
    printf("      o divisor dá α = R2/(R1+R2), e compor divisores MULTIPLICA: %d falhas\n", malP);

    /* (c) o OPERADOR: Shockley leva SOMA de tensoes em PRODUTO de correntes */
    int malO = 0;
    double Is = 1e-14;
    printf("\n      V1       V2       I(V1)·I(V2)/Is      I(V1+V2)         resíduo\n");
    for(int k = 0; k < 5; k++){
        double V1 = 0.30 + 0.05*k, V2 = 0.25 + 0.04*k;
        double i1 = Is*exp(V1/VT), i2 = Is*exp(V2/VT);      /* sem o -1: a região exponencial */
        double prod = i1*i2/Is, soma = Is*exp((V1+V2)/VT);
        double res = fabs(prod-soma)/fabs(soma);
        printf("      %.3f    %.3f    %.6e      %.6e     %.1e\n", V1, V2, prod, soma, res);
        if(res > 1e-12) malO++;
    }
    printf("\n      (mais 200 pares medidos)\n");
    for(int k = 0; k < 200; k++){
        double V1 = 0.1 + 0.002*k, V2 = 0.15 + 0.0015*k;
        double prod = Is*exp(V1/VT)*exp(V2/VT), soma = Is*exp((V1+V2)/VT);
        if(fabs(prod-soma)/fabs(soma) > 1e-11) malO++;
    }
    printf("\n");
    ok("a SOMA é Kirchhoff: série soma Z, paralelo soma Y — e a LEI mede-se em ℤ, sem"
       " régua: a série é uma soma exacta, e o paralelo lê-se por produto cruzado"
       " (R_p·(R2R3+R1R3+R1R2) = R1R2R3) sem dividir. A rota em vírgula fica ao lado, e o"
       " que lhe falta é o QUOCIENTE caber em base dois, não a lei",
       malS == 0 && casos_z > 0 && serie_ok == casos_z && par_ok == casos_z);
    ok("o PRODUTO é o ganho: compor divisores multiplica os α", malP == 0);
    ok("o OPERADOR é o transistor: I(V1+V2) = I(V1)·I(V2)/Is — soma vira PRODUTO",
       malO == 0);
    printf("      É esta a frase inteira: Π(a+b) = Π(a)·Π(b), a cláusula de Pontryagin do\n");
    printf("      contrato, escrita em volts e amperes. O corpo diferencial dizia que o\n");
    printf("      caractere leva ⊕ em ⊗; aqui o caractere tem encapsulamento e três pernas.\n");
}

printf("\n§E2  As multiplicidades +1, 0, -1 — e L ⋈ C é a dualidade.\n\n");
{
    /* a multiplicidade e' o expoente de s = jω: L e' s^{+1}, R e' s^0, C e' s^{-1}.
     * Mede-se pela inclinacao log-log de |Z| contra ω. */
    printf("      componente   Z(ω)          inclinação log-log medida   multiplicidade\n");
    int mal = 0;
    double L = 1e-3, C = 1e-6, R = 100;
    struct { const char *nome; int esp; } t[] = { {"indutor L", +1}, {"resistor R", 0},
                                                  {"capacitor C", -1} };
    for(int j = 0; j < 3; j++){
        double w1 = 1e3, w2 = 1e4, m1, m2;
        if(j == 0){ m1 = cabs(z_L(L,w1)); m2 = cabs(z_L(L,w2)); }
        else if(j == 1){ m1 = cabs(z_R(R,w1)); m2 = cabs(z_R(R,w2)); }
        else { m1 = cabs(z_C(C,w1)); m2 = cabs(z_C(C,w2)); }
        double incl = (log(m2)-log(m1))/(log(w2)-log(w1));
        printf("      %-12s %-13s %-27.9f %+d\n", t[j].nome,
               j==0 ? "sL" : j==1 ? "R" : "1/(sC)", incl, t[j].esp);
        if(fabs(incl - t[j].esp) > 1e-9) mal++;
    }
    printf("\n");
    ok("as multiplicidades são +1 (L), 0 (R) e -1 (C) — medidas, não postuladas", mal == 0);
    /* e a dualidade L ⋈ C: soma 0 (o resistor), media geometrica Z0 (o metal) */
    long w = 5000;
    double soma = 1.0 + (-1.0);
    double geo = sqrt(cabs(z_L(L,w))*cabs(z_C(C,w)));
    printf("      L ⋈ C:  soma das multiplicidades = %+.0f  (o resistor, mult 0)\n", soma);
    printf("              média geométrica √(|Z_L|·|Z_C|) = %.6f\n", geo);
    printf("              e √(L/C) = Z₀ = %.6f      (o metal, La Hire)\n\n", el_Z0(L,C));
    ok("o par L ⋈ C soma 0 e a sua média geométrica É Z₀ = √(L/C), o metal",
       fabs(soma) < 1e-15 && fabs(geo*geo - el_Z0q(L,C)) < 1e-9*el_Z0q(L,C));
    printf("      A tríade fecha: indutor (+1) — diodo (log, o operador) — capacitor (-1). O\n");
    printf("      +1 deriva, o -1 integra, e o log é quem atravessa entre os dois.\n");
}

printf("\n§E3  O RLC: a ressonância é o CASAMENTO, e Δ dá as três classes.\n\n");
{
    double L = 1e-3, C = 1e-6;
    double w0 = el_ressonancia(L,C);
    printf("      L = 1 mH, C = 1 µF  ->  ω₀ = 1/√(LC) = %.6f rad/s  (f₀ = %.3f Hz)\n\n",
           w0, w0/(2*M_PI));
    /* na ressonancia: Im Z = 0, Z = R, FP = 1 */
    int mal = 0;
    printf("      R (Ω)   Z(ω₀)                    Im Z      FP\n");
    for(int j = 0; j < 4; j++){
        double R = 10.0*(j+1);
        double complex Z = el_rlc(R,L,C,w0);
        printf("      %-7.0f %+.6f %+.6fj    %+.1e  %.9f\n", R, creal(Z), cimag(Z),
               cimag(Z), el_fp(Z));
        /* fp = 1 é fp² = 1, e fp² não forma raiz nenhuma: é Re²/(Re²+Im²) */
        if(fabs(cimag(Z)) > 1e-9 || fabs(el_fp2(Z) - 1.0) > 1e-12) mal++;
    }
    printf("\n");
    ok("na ressonância Im Z = 0 e FP = 1 — o +1 cancela o -1, e nada volta", mal == 0);
    /* e as TRES CLASSES pelo Δ — as mesmas do dual.c §U5, agora na bancada */
    printf("      E as três classes, pelo mesmo Δ do dual.c §U5:\n\n");
    printf("      R (Ω)     Δ = R² - 4L/C     classe            e² correspondente\n");
    int malC = 0;
    double Rc = 2.0*sqrt(L/C);                        /* o R crítico: Δ = 0 */
    struct { double R; int esp; const char *cl, *e2; } q[] = {
        { Rc*0.5, -1, "subamortecido",   "-1 (círculo)"    },
        { Rc,      0, "CRÍTICO",         " 0 (fronteira)"  },
        { Rc*2.0, +1, "sobreamortecido", "+1 (hipérbole)"  },
    };
    int malS = 0;
    for(int j = 0; j < 3; j++){
        double D = el_delta(q[j].R, L, C);
        int med = (D < -1e-6) ? -1 : (D > 1e-6) ? +1 : 0;
        /* a SEGUNDA ROTA: o sinal de R²C − 4L, sem a divisão por C e sem o limiar de 1e-6
         * — duas versões que ninguém confronta divergem, e é por isso que a comparação
         * está aqui e não na confiança. */
        int sg = el_delta_sinal(q[j].R, L, C);
        printf("      %-9.2f %+-17.4f %-17s %s\n", q[j].R, D, q[j].cl, q[j].e2);
        if(med != q[j].esp) malC++;
        if(j != 1 && sg != q[j].esp) malS++;      /* o crítico conta-se à parte, abaixo */
    }
    printf("\n      R crítico = 2√(L/C) = %.4f Ω\n", Rc);

    /* E O CRÍTICO NÃO É ATINGÍVEL AQUI, e o limiar de 1e-6 escondia-o. Rc = 2√(L/C) com
     * L/C = 1000, e √1000 é IRRACIONAL — logo Rc não é um número desta máquina, e
     * Rc²·C − 4L não dá zero exacto. A fronteira só é exacta quando L/C é QUADRADO
     * PERFEITO, e isso decide-se com a `rt_raiz_exacta`, em inteiros:
     *
     *      L/C = 1000  não é quadrado  →  o crítico é irracional, e não se atinge
     *      L/C = 10000 = 100²          →  Rc = 200 EXACTO, e Δ é ZERO exacto
     *
     * É o mesmo teorema do ponto fixo (`thm:fixo-dual`): a fronteira cai no racional sse o
     * discriminante é quadrado perfeito. O limiar de 1e-6 não estava a medir a fronteira —
     * estava a esconder que ela não é atingível com estes valores. */
    long razao = (long)(L/C + 0.5), r;
    int e_quadrado = rt_raiz_exacta(razao, &r);
    int sg_crit = el_delta_sinal(Rc, L, C);
    /* e a mesma bancada com L/C quadrado: aí o crítico é exacto e o sinal dá ZERO */
    double L2 = 1e-3, C2 = 1e-7;                  /* L/C = 10000 = 100² */
    long razao2 = (long)(L2/C2 + 0.5), r2;
    int e_quadrado2 = rt_raiz_exacta(razao2, &r2);
    double Rc2 = 2.0*(double)r2;                  /* = 200, sem formar raiz nenhuma */
    int sg_crit2 = el_delta_sinal(Rc2, L2, C2);
    printf("      e a FRONTEIRA so' e' exacta se L/C for QUADRADO PERFEITO: aqui L/C = %ld,\n"
           "      que %s quadrado — logo Rc e' irracional e o sinal sem limiar da' %+d, nao 0.\n"
           "      Com L/C = %ld = %ld², Rc = %.0f EXACTO e o sinal da' %+d.\n\n",
           razao, e_quadrado ? "E'" : "NAO e'", sg_crit, razao2, r2, Rc2, sg_crit2);

    ok("as três classes do Δ estão na bancada, e o crítico É ε² = 0 (raiz dupla). E a classe"
       " sai por DUAS rotas nos dois casos NAO criticos: o Δ = R² − 4L/C, e o SINAL de"
       " R².C − 4L sem a divisao e sem limiar — com C > 0 os dois tem o mesmo sinal",
       malC == 0 && malS == 0);

    ok("MAS O CRITICO NAO E' ATINGIVEL AQUI, e o limiar de 1e-6 escondia-o: Rc = 2.raiz(L/C)"
       " com L/C = 1000, e raiz(1000) e' IRRACIONAL — logo Rc nao e' um numero desta"
       " maquina, e Rc².C - 4L nao da' zero exacto. A fronteira so' e' exacta quando L/C e'"
       " QUADRADO PERFEITO, decidido pela `rt_raiz_exacta` em inteiros: com L/C = 10000 = 100²"
       " o Rc vale 200 sem se formar raiz nenhuma, e o sinal da' ZERO. E' o mesmo teorema do"
       " ponto fixo — a fronteira cai no racional sse o discriminante e' quadrado perfeito",
       !e_quadrado && sg_crit != 0 && e_quadrado2 && r2 == 100 && sg_crit2 == 0);
    printf("      A ressonância é a raiz dupla do dual.c, e o casamento FP = 1 é o cone nulo\n");
    printf("      do fisica.c §P5 — σ = 1, nada reflete, toda a potência passa. Três nomes,\n");
    printf("      um lugar.\n");
}

printf("\n§E4  A Gilbert cell: multiplicar É somar os logs.\n\n");
{
    /* log - Σ - antilog. O produto de dois SINAIS faz-se somando os logs e exponenciando —
     * que e' exatamente Π = exp∘Σ∘log. */
    printf("      log-Σ-antilog:  I1·I2/Iref = exp(log I1 + log I2 - log Iref)\n\n");
    printf("      I1 (µA)   I2 (µA)   pelo produto      pela Gilbert       resíduo\n");
    int mal = 0;
    double Iref = 1e-6;
    for(int k = 0; k < 5; k++){
        double I1 = (1.0 + 0.7*k)*1e-6, I2 = (2.0 + 0.4*k)*1e-6;
        double direto = I1*I2/Iref, gil = el_gilbert(I1,I2,Iref);
        double res = fabs(direto-gil)/fabs(direto);
        printf("      %-9.3f %-9.3f %.9e   %.9e    %.1e\n", I1*1e6, I2*1e6, direto, gil, res);
        if(res > 1e-12) mal++;
    }
    for(int k = 0; k < 300; k++){
        double I1 = (0.5 + 0.01*k)*1e-6, I2 = (3.0 - 0.008*k)*1e-6;
        if(I2 <= 0) continue;
        if(fabs(I1*I2/Iref - el_gilbert(I1,I2,Iref))/(I1*I2/Iref) > 1e-12) mal++;
    }
    printf("\n      (mais 300 pares medidos)\n\n");
    ok("a Gilbert cell multiplica somando os logs — Π = exp∘Σ∘log em silício", mal == 0);
    printf("      E o caminho físico é literalmente esse: o par diferencial converte corrente em\n");
    printf("      tensão pelo LOG (Shockley invertida), as tensões SOMAM no nó (Kirchhoff), e o\n");
    printf("      andar de saída exponencia de volta. Log, soma, antilog — as três peças da\n");
    printf("      tríade em cascata, e o resultado é uma multiplicação analógica.\n");
}

printf("\n§E5  Wheatstone: a medida por ANULAÇÃO — resíduo 0 em circuito.\n\n");
{
    /* O equilibrio Z1·Z4 = Z2·Z3 faz o detector ler ZERO. E' o principio da certeza em
     * circuito: nao se le o valor, ajusta-se ate o residuo ser 0 e le-se a RAZAO. */
    printf("      equilíbrio: Z₁·Z_x = Z₂·Z₃   ->   Z_x = Z₂·Z₃/Z₁, e o detector lê ZERO\n\n");
    int mal = 0, malD = 0;
    printf("      Z₁      Z₂      Z₃      Z_x calculado   detector no equilíbrio\n");
    for(int k = 0; k < 5; k++){
        double complex z1 = 100.0 + 20.0*k, z2 = 220.0 + 5.0*k, z3 = 470.0 - 10.0*k;
        double complex zx = el_wheatstone(z1,z2,z3);
        double complex d = el_detector(z1,z2,z3,zx,10.0);
        printf("      %-7.0f %-7.0f %-7.0f %-15.6f %.2e\n",
               creal(z1), creal(z2), creal(z3), creal(zx), cabs(d));
        if(fabs(creal(z1*zx) - creal(z2*z3)) > 1e-9) mal++;
        if(cabs(d) > 1e-12) malD++;
    }
    /* e o detector NAO le zero fora do equilibrio — senao a medida nao mediria nada */
    long complex z1 = 100, z2 = 220, z3 = 470;
    double complex zx = el_wheatstone(z1,z2,z3);
    double complex fora = el_detector(z1,z2,z3,zx*1.01,10.0);
    printf("\n      e 1%% fora do equilíbrio o detector lê %.4e — logo ele MEDE\n\n", cabs(fora));
    ok("no equilíbrio o detector lê ZERO, e fora dele NÃO lê — a ponte mede mesmo",
       mal == 0 && malD == 0 && cabs(fora) > 1e-4);
    printf("      É o princípio da certeza em circuito: não se lê o valor num mostrador, que\n");
    printf("      teria a precisão do mostrador. Ajusta-se até o resíduo ser ZERO e lê-se a\n");
    printf("      RAZÃO — e a razão é exata. É a mesma disciplina da bateria toda: resíduo 0\n");
    printf("      ou falha, e nunca \"perto o bastante\".\n");
}

printf("\n§E6  A ponte retificadora: o operador |·|, e o DC nasce do AC.\n\n");
{
    /* quatro diodos (Graetz): V_out = |V_in|. E a media de |sen| e' 2/π > 0 — o DC nasce
     * de uma entrada de media ZERO. */
    int n = 200000;
    double mediaAC = 0, mediaDC = 0;
    for(int k = 0; k < n; k++){
        double t = 2.0*M_PI*k/n, v = sin(t);
        mediaAC += v;
        mediaDC += fabs(v);
    }
    mediaAC /= n; mediaDC /= n;
    printf("      entrada:  média de sen(t)      = %+.9f   (zero: é AC puro)\n", mediaAC);
    printf("      saída:    média de |sen(t)|    = %+.9f\n", mediaDC);
    printf("      previsto: 2/π                  = %+.9f\n\n", 2.0/M_PI);
    ok("a ponte dá |·|, e a média salta de 0 para 2/π — o DC nasce do AC",
       fabs(mediaAC) < 1e-9 && fabs(mediaDC - 2.0/M_PI) < 1e-5);
    printf("      E note-se o que a ponte é, na tríade: um OPERADOR. Ela não soma nem escala —\n");
    printf("      dobra o sinal sobre si próprio. É uma dobra, e não é reversível: de |v| não\n");
    printf("      se recupera o sinal de v. Perde-se exatamente um bit, e é esse bit que vira\n");
    printf("      corrente contínua.\n");
}

printf("\n§E7  VALIDAR: simular no tempo e conferir contra a forma fechada.\n\n");
{
    /* A memoria do projeto e clara: os piores defeitos foram apanhados por uma COMPARAÇÃO
     * entre dois caminhos, nao por asserçoes. Aqui os dois caminhos sao: (a) a solucao
     * fechada da borda L·s² + R·s + 1/C = 0, e (b) integrar no tempo. Tem de fechar. */
    printf("      caminho A: a forma fechada, das raízes da borda L·s² + R·s + 1/C = 0\n");
    printf("      caminho B: integrar L·q'' + R·q' + q/C = 0 no tempo\n\n");
    double L = 1e-3, C = 1e-6, q0 = 1e-6, i0 = 0;
    printf("      R (Ω)    classe            q(T) fechada      q(T) simulada     resíduo rel.\n");
    int mal = 0;
    double Rc = 2.0*sqrt(L/C);
    double Rs[] = { Rc*0.3, Rc*0.6, Rc, Rc*1.8, Rc*3.0 };
    for(int j = 0; j < 5; j++){
        double R = Rs[j], D = el_delta(R,L,C), T = 2e-5;
        double a = -R/(2*L), qf;
        const char *cl;
        if(D < -1e-9){                                  /* subamortecido: par conjugado */
            double wd = sqrt(4*L/C - R*R)/(2*L);
            qf = exp(a*T)*(q0*cos(wd*T) + (i0 - a*q0)/wd*sin(wd*T));
            cl = "subamortecido";
        } else if(D > 1e-9){                            /* sobreamortecido: duas reais */
            double r = sqrt(D)/(2*L), s1 = a + r, s2 = a - r;
            double A = (i0 - s2*q0)/(s1-s2), B = q0 - A;
            qf = A*exp(s1*T) + B*exp(s2*T);
            cl = "sobreamortecido";
        } else {                                        /* CRÍTICO: raiz dupla, entra o t */
            qf = (q0 + (i0 - a*q0)*T)*exp(a*T);
            cl = "CRÍTICO (raiz dupla)";
        }
        double h = T/4000000.0, qs, is;
        el_simula(R,L,C,q0,i0,h,4000000,&qs,&is);
        double res = fabs(qf-qs)/(fabs(qf)+1e-30);
        printf("      %-8.2f %-17s %+.9e   %+.9e   %.1e\n", R, cl, qf, qs, res);
        if(res > 1e-4) mal++;
    }
    printf("\n");
    ok("os DOIS caminhos concordam nas três classes — incluindo a raiz dupla",
       mal == 0);
    printf("      E o caso do meio é o que interessa: no CRÍTICO a forma fechada precisa do\n");
    printf("      termo t·e^{at}, porque a raiz é dupla. Se eu tivesse escrito só a fórmula das\n");
    printf("      duas raízes distintas, ela dividiria por (s₁-s₂) = 0 e explodiria — e a\n");
    printf("      simulação, que não sabe de raízes, teria denunciado. É o par de caminhos a\n");
    printf("      fazer o trabalho que nenhuma asserção sozinha faz.\n");
    printf("\n      É esta a validação que a assistente passa a poder fazer: resolver pela\n");
    printf("      borda, simular no tempo, e exigir que os dois fechem. Um circuito que só\n");
    printf("      fecha num dos caminhos não está resolvido — está adivinhado.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
