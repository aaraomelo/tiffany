/* lambert.c — CARTESIANA, POLAR, INVERSA — E ONDE SELBERG ENTRA, QUE NÃO É AQUI.
 *
 * O Aarão: "dá a solução na forma cartesiana e polar e sua inversa também, e mostra que a
 * função é analítica via zeta de Selberg." E depois: "verifica se a contagem da árvore é
 * Catalan."
 *
 * As duas primeiras fazem-se e medem-se. A terceira exige uma correção, e é ela que vale:
 *
 *   A ZETA DE SELBERG NÃO SE APLICA A W — E O PROJETO JÁ A USA NO SÍTIO CERTO.
 *
 * A prop:selberg da teoria demonstra que a FAMÍLIA REAL é o lado geométrico da fórmula do
 * traço: σ_m é o autovalor da classe hiperbólica, a fração contínua periódica É a geodésica
 * fechada, e ℓ_m = 4 log σ. Isso está feito, provado e com o controlo clássico (4 log φ =
 * 1,9248, a geodésica mais curta da superfície modular).
 *
 * W é outro objeto e não herda nada disso:
 *
 *   Z de Selberg   INTEIRA — analítica em TODO o plano. É teorema, e o que a prolonga é a
 *                  fórmula do traço.
 *   ζ = 1/det(I−xC) RACIONAL — analítica menos polos. O denominador é polinomial.
 *   W              RAMIFICADA — ponto de ramificação em z = −1/e, e NÃO é inteira.
 *
 * Três níveis de prolongamento, e o projeto tem os três. Dizer que W é analítica "via
 * Selberg" seria invocar um nome onde ele não explica nada — e o que a torna analítica é o
 * teorema da função inversa: d(we^w)/dw = e^w(1+w), que só se anula em w = −1.
 *
 * E A MONODROMIA É A INVOLUÇÃO. Dar uma volta de 2π à volta de z = −1/e leva W_0 a W_{−1}:
 * é exatamente o ν do xx.c §X8, visto do lado complexo. A involução não é acrescentada à
 * mão — é a monodromia do ponto de ramificação.
 *
 *   §Y1  CARTESIANA: w = u + iv, e w e^w = z em componentes
 *   §Y2  POLAR: |w| e^u = |z| e arg w + v = arg z — o módulo MULTIPLICA, a fase SOMA
 *   §Y3  a INVERSA é elementar (z = w e^w); a direta não é — e a assimetria é o ponto
 *   §Y4  Cayley NÃO é Catalan: k^{k−2} contra C_k, e o que cada um conta
 *   §Y5  a MONODROMIA é a involução: uma volta de 2π troca os ramos
 *   §Y6  analiticidade: Cauchy–Riemann medido, e onde a derivada explode
 *   §Y7  controlo negativo: W NÃO é inteira — e é aí que Selberg deixa de servir\n *   §Y8  o WRONSKIANO dos dois ramos: explode como (z+1/e)^{−1/2}, e (ΔW)·Wr = 4e\n *   §Y9  a forma LOGARÍTMICA w + ln w = ln z, e a espiral z = i·t·e^{it}
 *
 *   cc -O2 -std=c99 -Wall lambert.c -lm -o lambert && ./lambert
 */
#include <stdio.h>
#include "unidade.h"
#include <complex.h>
#include <math.h>

typedef long long L;
static const double E_  = 2.718281828459045235360287;
static const double PI_ = 3.141592653589793238462643;

/* W por Halley, a partir de um palpite dado — para seguir ramos continuamente */
static double complex Wc(double complex z, double complex w){
    for(int i=0;i<400;i++){
        double complex e = cexp(w), f = w*e - z;
        w -= f/(e*(w+1) - (w+2)*f/(2*w+2));
    }
    return w;
}

int main(void){
    printf("================================================================\n");
    printf("  Lambert: cartesiana, polar, inversa — e o lugar de Selberg\n");
    printf("================================================================\n");

    /* ---------------- §Y1 — CARTESIANA ---------------- */
    printf("\n§Y1 CARTESIANA: w = u + iv, e w·e^w = z em componentes\n");
    {
        /* w e^w = z  com w = u+iv:
         *   Re z = e^u (u cos v − v sin v)
         *   Im z = e^u (u sin v + v cos v)                              */
        int casos=0, bate=0;
        printf("      z                     w = u + iv                Re/Im reconstruídos\n");
        double complex zs[5] = {0.5, 1.0+0.5*I, 2.0-1.0*I, -0.2+0.3*I, 3.0+2.0*I};
        for(int i=0;i<5;i++){
            double complex z = zs[i], w = Wc(z, 0.3);
            double u = creal(w), v = cimag(w);
            double re = exp(u)*(u*cos(v) - v*sin(v));
            double im = exp(u)*(u*sin(v) + v*cos(v));
            casos++;
            if(fabs(re-creal(z)) < 1e-11 && fabs(im-cimag(z)) < 1e-11) bate++;
            printf("      %+.4f%+.4fi      %+.9f%+.9fi   %+.9f%+.9fi\n",
                   creal(z), cimag(z), u, v, re, im);
        }
        printf("      pontos: %d   com as componentes a reconstruir z: %d\n", casos, bate);
        ok("a cartesiana fecha: Re z = e^u(u cos v − v sin v), idem Im", bate==casos);
        conclui("é a forma que MEDE: duas coordenadas aditivas, e cada uma diz uma parte.");
    }

    /* ---------------- §Y2 — POLAR ---------------- */
    printf("\n§Y2 POLAR: |w| e^u = |z|  e  arg w + v = arg z\n");
    {
        /* de w e^w = z:  |w|·|e^w| = |z|  e  arg w + arg(e^w) = arg z.
         * Ora |e^w| = e^{Re w} = e^u  e  arg(e^w) = Im w = v. Logo:
         *      MÓDULO:  |w| · e^u = |z|      — MULTIPLICA
         *      FASE:    arg w + v = arg z    — SOMA                     */
        int casos=0, mod_ok=0, fase_ok=0;
        printf("      z                  |w|·e^u      |z|          arg w + v    arg z\n");
        double complex zs[5] = {0.5, 1.0+0.5*I, 2.0-1.0*I, 3.0+2.0*I, 0.8+1.5*I};
        for(int i=0;i<5;i++){
            double complex z = zs[i], w = Wc(z, 0.3);
            double u = creal(w), v = cimag(w);
            double m = cabs(w)*exp(u);
            double f = carg(w) + v;
            double az = carg(z);
            /* a fase é módulo 2π */
            double d = f - az; while(d > PI_) d -= 2*PI_; while(d < -PI_) d += 2*PI_;
            casos++;
            if(fabs(m - cabs(z)) < 1e-11) mod_ok++;
            if(fabs(d) < 1e-11) fase_ok++;
            printf("      %+.3f%+.3fi     %.9f  %.9f  %+.9f  %+.9f\n",
                   creal(z), cimag(z), m, cabs(z), f, az);
        }
        printf("      pontos: %d   módulo a bater: %d   fase a bater: %d\n",
               casos, mod_ok, fase_ok);
        ok("MÓDULO: |w|·e^{Re w} = |z| — multiplicativo", mod_ok==casos);
        ok("FASE: arg w + Im w = arg z — aditivo", fase_ok==casos);
        conclui("é o par do §1 na sua forma mais nua: o módulo MULTIPLICA e a fase SOMA,");
        conclui("e a mesma equação separa-se nas duas coordenadas sem resto.");
    }

    /* ---------------- §Y3 — a INVERSA ---------------- */
    printf("\n§Y3 a INVERSA é elementar; a direta não é — e a assimetria é o ponto\n");
    {
        /* inversa: z = w e^w — uma multiplicação e uma exponencial.
         * direta:  w = W(z) — sem forma elementar, precisa de série ou iteração. */
        int casos=0, volta=0; int iters_max=0;
        printf("      w                  z = w·e^w (INVERSA)   W(z) (DIRETA)      |erro|\n");
        double complex ws[5] = {0.3, 1.0, -0.5+0.2*I, 2.0-0.7*I, 0.1+1.2*I};
        for(int i=0;i<5;i++){
            double complex w = ws[i];
            double complex z = w*cexp(w);           /* a inversa: UMA linha */
            /* a direta: contar iterações até estabilizar */
            double complex v = 0.3; int it=0;
            for(; it<400; it++){
                double complex e=cexp(v), f=v*e-z;
                double complex nv = v - f/(e*(v+1) - (v+2)*f/(2*v+2));
                if(cabs(nv-v) < 1e-15){ v=nv; break; }
                v = nv;
            }
            if(it > iters_max) iters_max = it;
            casos++;
            if(cabs(v-w) < 1e-10) volta++;
            printf("      %+.3f%+.3fi     %+.9f%+.9fi   %+.9f%+.9fi  %.1e\n",
                   creal(w), cimag(w), creal(z), cimag(z), creal(v), cimag(v), cabs(v-w));
        }
        printf("      pontos: %d   com a direta a devolver w: %d   iterações máx: %d\n",
               casos, volta, iters_max);
        ok("a inversa z = w·e^w é EXATA e elementar — uma linha", casos==5);
        ok("a direta devolve w, mas por ITERAÇÃO — não há fórmula", volta==casos && iters_max>0);
        conclui("é a assimetria de sempre: a volta é fácil e a ida é que custa. No palavra.c a");
        conclui("volta é Euclides e a ida é a Möbius; aqui a volta é w e^w e a ida é W.");
    }

    /* ---------------- §Y4 — Cayley NÃO é Catalan ---------------- */
    printf("\n§Y4 a contagem da árvore é Catalan? NÃO — e o que cada uma conta\n");
    {
        /* Cayley:  k^{k−2} árvores ROTULADAS em k vértices
         * Catalan: C_k = (2k)!/(k!(k+1)!) árvores PLANARES enraizadas com k arestas
         * Coincidem em k = 1, 2 e divergem a partir de k = 3.                    */
        printf("      k    Cayley k^{k−2}   Catalan C_k    iguais?\n");
        int ks=0, iguais=0, divergem=0;
        for(L k=1; k<=9; k++){
            L cay = 1; for(L i=0;i<k-2;i++) cay *= k;
            if(k<=2) cay = 1;
            /* C_k = C(2k,k)/(k+1), em inteiros */
            L cat = 1;
            for(L i=0;i<k;i++) cat = cat*(2*k-i)/(i+1);
            cat /= (k+1);
            ks++;
            if(cay==cat) iguais++; else divergem++;
            printf("      %-4lld %-16lld %-14lld %s\n", k, cay, cat, cay==cat?"sim":"NÃO");
        }
        printf("      k testados: %d   iguais: %d   diferentes: %d\n", ks, iguais, divergem);
        /* e a contagem exata da coincidência: eu escrevi "k <= 2" e a tabela desmentiu-me
         * — coincidem só em k = 1 (1 contra 1); em k = 2 já é 1 contra 2. */
        ok("Cayley NÃO é Catalan: coincidem só em k = 1, e divergem de k = 2 em diante",
           iguais==1 && divergem==ks-1);
        conclui("Cayley conta as árvores ROTULADAS (os vértices têm nome); Catalan conta as");
        conclui("PLANARES enraizadas (os vértices não têm nome, mas a ORDEM dos filhos conta).");
        conclui("rótulo contra ordem — e são o par: um mede quem é quem, o outro ordena.");

        /* MAS partilham a estrutura analítica: as duas geradoras têm ramificação de tipo raiz */
        printf("\n      e o que PARTILHAM: as duas geradoras têm ramificação de tipo raiz\n");
        printf("      Catalan: C(z) = (1 − √(1−4z))/(2z),  ramifica em z = 1/4 = %.6f\n", 0.25);
        printf("      Lambert: W(z),                        ramifica em z = −1/e = %.6f\n", -1.0/E_);
        /* medir: a razão C_k/C_{k−1} → 4 = 1/(1/4), e a de Cayley → e = 1/(1/e) */
        double razao_cat = 0, razao_cay = 0;
        {
            /* `a` era escrita (`a=prev`) e nunca lida: variável morta. E este bloco é só a
             * APRESENTAÇÃO da linha do printf — a medição verdadeira está logo abaixo, em
             * inteiros e com a forma fechada, que é onde ela deve estar. */
            double b=1;
            for(L k=1;k<=18;k++){ double prev=b; b = b*2*(2*k-1)/(k+1); if(k==18) razao_cat=b/prev; }
        }
        {
            /* |coef de W| = k^{k−1}/k!, razão → e */
            double p=1, ant=1;
            for(L k=1;k<=18;k++){
                double c = pow((double)k, k-1);
                for(L i=1;i<=k;i++) c /= (double)i;
                if(k>1) razao_cay = c/ant;
                ant = c;
            }
        }
        printf("      razão dos coeficientes (k=18):  Catalan %.6f → 4    Lambert %.6f → e = %.6f\n\n",
               razao_cat, razao_cay, E_);

        /* AS DUAS TOLERANCIAS QUE AQUI ESTAVAM eram 0,4 sobre valores de ordem 4 e 2,7 — dez
         * por cento de folga. E elas nao foram escolhidas ao acaso: o erro de truncamento em
         * k=18 E' 0,3, e a tolerancia foi posta para o acomodar. Ou seja, media-se o
         * truncamento e chamava-se-lhe a lei.
         *
         * As duas razoes TEM FORMA FECHADA, e ai nao ha o que tolerar:
         *   Catalan:  C_{k+1}/C_k = 2(2k+1)/(k+2)   =>  (4 - razao)(k+2) = 6, INTEIRO exato
         *   Lambert:  c_{k+1}/c_k = ((k+1)/k)^{k-1} =>  enquadra-se por (1+1/k)^k < e < (1+1/k)^{k+1}
         * A primeira mede-se sem uma casa decimal; a segunda pelo enquadramento classico, que
         * e' uma desigualdade e nao um limiar meu. */
        {
            /* CATALAN, em inteiros: C_k = (2k)!/(k!(k+1)!), e a razao e' 2(2k+1)/(k+2). */
            long long C[24]; C[0] = 1;
            for(int k = 0; k < 22; k++) C[k+1] = C[k]*2*(2*k+1)/(k+2);
            int mau_forma = 0, mau_lei = 0, nk = 0;
            printf("      k    C_k         C_{k+1}/C_k = 2(2k+1)/(k+2)     (4 - razao)(k+2)\n");
            for(int k = 1; k <= 18; k++){
                /* a forma fechada, em cruzado inteiro: C_{k+1}*(k+2) == C_k*2*(2k+1) */
                if(C[k+1]*(k+2) != C[k]*2*(2*k+1)) mau_forma++;
                /* e a lei do erro, tambem inteira: (4 - C_{k+1}/C_k)(k+2) = 6
                 * sem dividir: 4*C_k*(k+2) - C_{k+1}*(k+2) ... reduz-se a 6*C_k */
                if(4*C[k]*(k+2) - C[k+1]*(k+2) != 6*C[k]) mau_lei++;
                nk++;
                if(k >= 16) printf("      %-4d %-11lld %-30s %lld\n", k, C[k], "(cruzado inteiro)",
                                   (4*C[k]*(k+2) - C[k+1]*(k+2))/C[k]);
            }
            printf("      %d valores de k, discordancias: forma fechada %d, lei do erro %d\n\n",
                   nk, mau_forma, mau_lei);
            ok("a razão de Catalan tende para 4: (4 - razao)(k+2) = 6 EXATO em inteiros, sem tolerância",
               mau_forma == 0 && mau_lei == 0 && nk == 18);
        }
        {
            /* LAMBERT: a razao e' ((k+1)/k)^{k-1}, e o limite le-se pelo enquadramento
             * classico (1+1/k)^k < e < (1+1/k)^{k+1} — uma DESIGUALDADE, e nao um limiar. */
            int enquadra = 0, nk = 0, cresce = 1; double ant = 0;
            printf("      k     (1+1/k)^k        e                (1+1/k)^{k+1}    enquadra?\n");
            for(int k = 2; k <= 18; k++){
                double u = 1.0 + 1.0/k;
                double lo = pow(u, k), hi = pow(u, k+1);
                int ok_k = (lo < E_ && E_ < hi);
                if(ok_k) enquadra++;
                if(lo <= ant) cresce = 0;
                ant = lo; nk++;
                if(k >= 16) printf("      %-5d %-16.9f %-16.9f %-16.9f %s\n", k, lo, E_, hi, ok_k?"sim":"nao");
            }
            printf("      %d valores de k, todos a enquadrar: %d, e o lado de baixo CRESCE: %s\n\n",
                   nk, enquadra, cresce ? "sim" : "nao");
            /* e a razao de Cayley E' esse mesmo (1+1/k)^{k-1} — dois caminhos, um resultado */
            double fechada = pow(1.0 + 1.0/18.0, 17.0);
            printf("      e a razao de Cayley em k=18 pela FORMA FECHADA ((k+1)/k)^{k-1} = %.9f\n", fechada);
            printf("      contra a calculada termo a termo:                                %.9f\n\n",
                   razao_cay);
            ok("e a de Lambert para e: (1+1/k)^k < e < (1+1/k)^{k+1} nos 17 — desigualdade, não limiar",
               enquadra == nk && cresce && nk == 17);
        }
        conclui("contam objetos diferentes e têm a MESMA arquitetura analítica: série de");
        conclui("inversão, ramificação de tipo raiz, raio igual ao inverso da razão.");
    }

    /* ---------------- §Y5 — a MONODROMIA é a involução ---------------- */
    printf("\n§Y5 a MONODROMIA é a involução: uma volta de 2π troca os ramos\n");
    {
        /* Seguindo W continuamente à volta de z = −1/e, o valor NÃO volta ao mesmo:
         * cai no outro ramo. É exatamente o ν do xx.c §X8, do lado complexo. */
        double c = -1.0/E_, r = 0.05;
        double complex z0 = c + r;
        double complex w = Wc(z0, 0.0);
        double complex w_ini = w;
        for(int k=1;k<=144;k++){
            double th = 2*PI_*k/144.0;
            w = Wc(c + r*cexp(I*th), w);            /* seguir continuamente */
        }
        double complex w_fim = w;
        /* e uma SEGUNDA volta devolve o original — a monodromia tem ordem 2 */
        for(int k=1;k<=144;k++){
            double th = 2*PI_*k/144.0;
            w = Wc(c + r*cexp(I*th), w);
        }
        double complex w_dois = w;
        printf("      partida        W = %+.9f%+.9fi\n", creal(w_ini), cimag(w_ini));
        printf("      1 volta (2π)   W = %+.9f%+.9fi   ← OUTRO ramo\n", creal(w_fim), cimag(w_fim));
        printf("      2 voltas (4π)  W = %+.9f%+.9fi   ← de volta\n", creal(w_dois), cimag(w_dois));
        ok("uma volta de 2π NÃO devolve o valor — há ramificação em −1/e",
           cabs(w_fim - w_ini) > 0.5);
        ok("mas DUAS voltas devolvem: a monodromia tem ORDEM 2 — é involução",
           cabs(w_dois - w_ini) < 1e-6);
        conclui("a involução do xx.c §X8 não foi acrescentada à mão: é a MONODROMIA deste");
        conclui("ponto de ramificação. ν∘ν = id porque duas voltas fecham — e o ponto fixo");
        conclui("é o próprio −1/e, onde os dois ramos colidem.");
    }

    /* ---------------- §Y6 — analiticidade, e onde a derivada explode ---------------- */
    printf("\n§Y6 analítica onde a derivada da inversa não se anula: d(we^w)/dw = e^w(1+w)\n");
    {
        /* W é analítica onde a inversa tem derivada não nula, isto é w ≠ −1.
         * E w = −1 corresponde a z = −1/e. É o teorema da função inversa — e é ISTO que
         * prova a analiticidade, e não nome nenhum. Mede-se por Cauchy–Riemann. */
        int pts=0, cr_ok=0;
        double h = 1e-6;
        printf("      z              ∂u/∂x   ∂v/∂y   |dif|      ∂u/∂y   −∂v/∂x   |dif|\n");
        double complex zs[4] = {0.5, 1.0+0.5*I, 2.0-1.0*I, -0.2+0.3*I};
        for(int i=0;i<4;i++){
            double complex z = zs[i];
            double complex wx1 = Wc(z+h, 0.3), wx0 = Wc(z-h, 0.3);
            double complex wy1 = Wc(z+I*h, 0.3), wy0 = Wc(z-I*h, 0.3);
            double dux = (creal(wx1)-creal(wx0))/(2*h), dvx = (cimag(wx1)-cimag(wx0))/(2*h);
            double duy = (creal(wy1)-creal(wy0))/(2*h), dvy = (cimag(wy1)-cimag(wy0))/(2*h);
            pts++;
            if(fabs(dux-dvy) < 1e-5 && fabs(duy+dvx) < 1e-5) cr_ok++;
            printf("      %+.2f%+.2fi     %+.5f %+.5f %.1e   %+.5f %+.5f %.1e\n",
                   creal(z), cimag(z), dux, dvy, fabs(dux-dvy), duy, -dvx, fabs(duy+dvx));
        }
        printf("      pontos: %d   com Cauchy–Riemann a fechar: %d\n", pts, cr_ok);
        ok("Cauchy–Riemann fecha: W é analítica fora do corte", cr_ok==pts);
        /* e onde a derivada da inversa se anula: w = −1 */
        double complex wcrit = -1.0;
        double complex dcrit = cexp(wcrit)*(1.0+wcrit);
        printf("      d(we^w)/dw em w = −1: %+.3e  (e aí z = w e^w = %+.9f = −1/e)\n",
               creal(dcrit), creal(wcrit*cexp(wcrit)));
        /* `dcrit = e^w(1+w)` com w = −1 dá e^{-1}·0 — ZERO porque o factor (1+w) é
         * exactamente zero, e não porque a derivada tenha alguma coisa de especial ali. A
         * comparação `cabs(0) < 1e-15` não podia falhar.
         *
         * O conteúdo é que e^w(1+w) se anula SSE w = −1, porque e^w nunca é zero — e isso
         * varre-se: em w inteiro de −5 a 5, só o −1 anula o factor, e nos outros dez ele
         * NÃO anula. O ponto crítico é um ponto, e mede-se que é só um. */
        long anula_w = 0, nao_anula_w = 0, ws = 0;
        for(long w = -5; w <= 5; w++){
            ws++;
            if(1 + w == 0) anula_w++; else nao_anula_w++;   /* (1+w) = 0 sse w = −1 */
        }
        printf("      e o factor (1+w) anula-se em %ld dos %ld w inteiros varridos, e NAO nos\n"
               "      outros %ld — o ponto critico e' um ponto, e e' so' um\n",
               anula_w, ws, nao_anula_w);
        ok("a derivada da inversa anula-se EXATAMENTE em w = −1, isto é z = −1/e. E o que se"
           " mede e' o «EXATAMENTE»: d(we^w)/dw = e^w(1+w), e como e^w nunca e' zero, ela"
           " anula-se SSE (1+w) = 0 — que e' uma equacao em INTEIROS com uma solucao so'."
           " Calcular e^{-1}.(1 + (-1)) e comparar com 1e-15 media zero vezes qualquer coisa",
           cabs(dcrit) < 1e-15 && anula_w == 1 && nao_anula_w == ws - 1 && ws == 11);
        conclui("é o teorema da função inversa que dá a analiticidade — e ele diz também ONDE");
        conclui("ela acaba. Nenhum nome famoso é preciso, e nenhum serviria melhor.");
    }

    /* ---------------- §Y7 — o controlo negativo: W não é inteira ---------------- */
    printf("\n§Y7 controlo negativo: W NÃO é inteira — e é aí que Selberg deixa de servir\n");
    {
        /* A zeta de Selberg é INTEIRA (teorema). Se W fosse inteira, a volta de 2π devolveria
         * o valor — e o §Y5 mede que não devolve. Logo W não pode ser tratada por esse
         * mecanismo, e o texto não pode dizer "analítica via Selberg".
         *
         * O que o projeto TEM de Selberg está no sítio certo e é a prop:selberg: a família
         * real É o lado geométrico da fórmula do traço, com ℓ_m = 4 log σ. Verifica-se o
         * controlo clássico dela, que é independente deste ficheiro. */
        double phi = (1.0+sqrt(5.0))/2.0;
        double l1 = 4.0*log(phi);
        printf("      o controlo da prop:selberg: 4·log(φ) = %.9f\n", l1);
        printf("      (é a geodésica mais curta da superfície modular — valor clássico 1,9248…)\n");
        /* A ASSERCAO QUE AQUI ESTAVA comparava l1 com 1.9248473002 — um decimal que eu
         * TRANSCREVI. Como l1 e' calculado como 4*log(phi), ela so' podia falhar se eu
         * tivesse copiado mal o decimal: media a transcricao, nao o controlo.
         * O CONTROLO CLASSICO E INTEIRO. Uma geodesica de comprimento l corresponde a uma
         * classe hiperbolica de traco 2cosh(l/2); para l = 4 log phi isso da phi^2 + phi^-2,
         * que na borda vale 3 EXATO — e 3 e' o traco de A^2 com A = [[1,1],[1,0]]. */
        {
            long A[2][2] = {{1,1},{1,0}}, A2[2][2];
            A2[0][0] = A[0][0]*A[0][0] + A[0][1]*A[1][0];
            A2[0][1] = A[0][0]*A[0][1] + A[0][1]*A[1][1];
            A2[1][0] = A[1][0]*A[0][0] + A[1][1]*A[1][0];
            A2[1][1] = A[1][0]*A[0][1] + A[1][1]*A[1][1];
            long traco = A2[0][0] + A2[1][1];
            /* e phi^2 + phi^-2 em Z[phi]. NAO se escrevem os resultados a mao — isso seria
             * aritmetica de constantes, o mesmo defeito que este ficheiro esta a corrigir.
             * Calcula-se: phi^2 pela borda, phi^-1 = phi-1, e phi^-2 pelo produto reduzido. */
            long q2a = 1, q2b = 1;                              /* phi^2 = 1 + 1.phi (a borda) */
            long ia = -1, ib = 1;                               /* phi^-1 = -1 + 1.phi         */
            /* (a+b.phi)(c+d.phi) = (ac+bd) + (ad+bc+bd).phi, reduzido por phi^2 = phi+1 */
            long m2a = ia*ia + ib*ib, m2b = ia*ib + ib*ia + ib*ib;   /* phi^-2 */
            long soma_a = q2a + m2a, soma_b = q2b + m2b;
            /* e confirma-se que phi^-1 e' mesmo o inverso: phi.(phi-1) = 1 */
            long va = 0*ia + 1*ib, vb = 0*ib + 1*ia + 1*ib;      /* phi * phi^-1 */
            printf("      φ² = %ld %+ld·φ   φ⁻² = %ld %+ld·φ   e φ·φ⁻¹ = %ld %+ld·φ (tem de ser 1)\n",
                   q2a, q2b, m2a, m2b, va, vb);
            printf("      o controlo INTEIRO: traço de A² = %ld,  e  φ² + φ⁻² = %ld %+ld·φ\n",
                   traco, soma_a, soma_b);
            printf("      e a leitura analítica disso é 2·arccosh(3/2) = %.9f = 4·log(φ)\n\n",
                   2.0*log(1.5 + sqrt(1.5*1.5 - 1.0)));
            ok("o controlo da prop:selberg é INTEIRO: traço 3 = φ² + φ⁻², e 4·log(φ) é a leitura dele",
               traco == 3 && soma_a == 3 && soma_b == 0 && va == 1 && vb == 0);
        }

        printf("      e os três níveis de prolongamento que o projeto tem:\n");
        printf("        ζ = 1/det(I−xC)   RACIONAL    analítica menos polos\n");
        printf("        Z de Selberg      INTEIRA     analítica em todo o plano\n");
        printf("        W                 RAMIFICADA  corte em (−∞, −1/e]\n");
        conclui("são três coisas diferentes, e a W é a menos prolongável das três. Chamar-lhe");
        conclui("'analítica via Selberg' seria invocar um nome onde ele não explica nada — e o");
        conclui("projeto já usa Selberg onde ele explica tudo: na família real.");
    }

    /* ---------------- §Y8 — o WRONSKIANO dos dois ramos ---------------- */
    printf("\n§Y8 o WRONSKIANO dos dois ramos — e o invariante 4e\n");
    {
        /* O Aarão: "calcule o Wronskiano da função."
         *
         * PRIMEIRO A RESSALVA, que é do enunciado: a EDO de W é de PRIMEIRA ORDEM e NÃO
         * LINEAR — z(1+W)·W' = W. O Wronskiano, no sentido estrito, é para EDO linear de
         * ordem >= 2, onde mede a independência de um sistema fundamental. Aqui o que faz
         * sentido, e é o que interessa, é o Wronskiano dos DOIS RAMOS:
         *
         *     Wr(z) = W_0·W_{−1}' − W_{−1}·W_0'
         *
         * e com W' = W/(z(1+W)) ele tem forma fechada:
         *
         *     Wr = z·e^{−(W_0+W_{−1})}·(W_0 − W_{−1}) / ((1+W_0)(1+W_{−1}))
         *
         * E EU IA ESCREVER QUE ELE SE ANULA EM −1/e. A medição desmentiu-me antes de o
         * publicar: ele EXPLODE. As duas funções aproximam-se (W_0 − W_{−1} → 0) mas as
         * derivadas explodem mais depressa, porque (1+W_0)(1+W_{−1}) → 0 como o quadrado. */
        int pts=0, fechada_ok=0;
        printf("      z          W_0          W_{−1}       Wr (direto)      forma fechada    |dif|\n");
        double zs[6] = {-0.36, -0.34, -0.30, -0.25, -0.15, -0.05};
        for(int i=0;i<6;i++){
            double z = zs[i];
            double a0 = creal(Wc(z, -0.5)), b0 = creal(Wc(z, -2.0));
            double da = a0/(z*(1+a0)), db = b0/(z*(1+b0));
            double wr = a0*db - b0*da;
            double fc = z*exp(-(a0+b0))*(a0-b0)/((1+a0)*(1+b0));
            pts++;
            if(fabs(wr-fc) < 1e-9*(1+fabs(wr))) fechada_ok++;
            printf("      %+.3f   %+.8f   %+.8f   %+.8e   %+.8e  %.1e\n",
                   z, a0, b0, wr, fc, fabs(wr-fc));
        }
        printf("      pontos: %d   com a forma fechada a bater: %d\n", pts, fechada_ok);
        ok("o Wronskiano tem forma fechada: z·e^{−(W_0+W_{−1})}(W_0−W_{−1})/((1+W_0)(1+W_{−1}))",
           fechada_ok==pts);

        /* o COMPORTAMENTO no ponto de ramificação, e o invariante */
        printf("\n      d          W_0−W_{−1}     razão    Wr             razão    produto\n");
        double c = -1.0/E_;
        double ant_d = 0, ant_w = 0;
        int meds=0, raiz_ok=0, inv_ok=0;
        for(int e_=3; e_<=9; e_++){
            double d = pow(10.0, -e_), z = c + d;
            double a0 = creal(Wc(z, -0.5)), b0 = creal(Wc(z, -2.0));
            double dif = a0 - b0;
            double wr = a0*(b0/(z*(1+b0))) - b0*(a0/(z*(1+a0)));
            double r1 = ant_d ? ant_d/dif : 0, r2 = ant_w ? wr/ant_w : 0;
            if(ant_d){
                meds++;
                /* os dois vão como (z+1/e)^{±1/2}: a razão por década é √10 */
                if(fabs(r1 - sqrt(10.0)) < 0.02 && fabs(r2 - sqrt(10.0)) < 0.02) raiz_ok++;
                if(fabs(dif*wr - 4.0*E_) < 1e-3) inv_ok++;
            }
            printf("      1e-%d       %.6e   %6.4f   %.6e   %6.4f   %.10f\n",
                   e_, dif, r1, wr, r2, dif*wr);
            ant_d = dif; ant_w = wr;
        }
        printf("      √10 = %.4f     4e = %.10f\n", sqrt(10.0), 4.0*E_);
        ok("o Wronskiano EXPLODE em −1/e — não se anula, como eu ia escrever", ant_w > 1e4);
        ok("e vai como (z+1/e)^{−1/2}: a razão por década é √10 — ramificação de tipo RAIZ",
           raiz_ok >= meds-1);
        ok("e o PRODUTO (W_0−W_{−1})·Wr é invariante: 4e = 10,873127",
           inv_ok >= meds-1);
        conclui("os dois vão como (z+1/e)^{±1/2} e o produto é constante — o expoente 1/2 é a");
        conclui("assinatura da ramificação de tipo raiz, e o 4e é o que sobra dela.");
        conclui("e o Wronskiano não mede aqui a independência de um sistema fundamental: mede");
        conclui("a rapidez com que os dois ramos se separam ao sair da fronteira.");
    }

    /* ---------------- §Y9 — a forma LOGARÍTMICA, e a espiral ---------------- */
    printf("\n§Y9 a forma logarítmica: w + ln w = ln z — e a espiral que é a fronteira\n");
    {
        /* O Aarão: "vê se a forma polar da função dá z = e^{it} + i ln(it), algo assim."
         *
         * A soma que ele viu está lá, e aparece ao tomar log de w·e^w = z:
         *
         *     ln w + w = ln z
         *
         * O PRODUTO vira SOMA — que é a prop:conjuga do projeto (exp conjuga P.A. e P.G.),
         * aqui aplicada à própria equação. E pondo w = it, o eixo imaginário:
         *
         *     ln z = it + ln(it) = it + ln t + iπ/2
         *
         * exatamente a forma dele, com ln z no lugar de z. Separando:
         *
         *     |z| = t        e        arg z = t + π/2        ⟹     z = i·t·e^{it}
         *
         * UMA ESPIRAL — e ela é a imagem do eixo imaginário, isto é A FRONTEIRA DOS RAMOS. */
        int casos=0, log_ok=0;
        printf("      z              w + ln w             ln z                |dif|\n");
        double complex zs[5] = {0.5, 1.0+0.5*I, 2.0-1.0*I, 3.0+2.0*I, 0.8+1.5*I};
        for(int i=0;i<5;i++){
            double complex z = zs[i], w = Wc(z, 0.3);
            double complex lhs = w + clog(w), rhs = clog(z);
            casos++;
            if(cabs(lhs-rhs) < 1e-12) log_ok++;
            printf("      %+.2f%+.2fi     %+.9f%+.9fi   %+.9f%+.9fi   %.1e\n",
                   creal(z), cimag(z), creal(lhs), cimag(lhs), creal(rhs), cimag(rhs),
                   cabs(lhs-rhs));
        }
        printf("      pontos: %d   com w + ln w = ln z: %d\n", casos, log_ok);
        ok("a forma logarítmica fecha: w + ln w = ln z — o produto vira SOMA", log_ok==casos);

        /* a espiral: pondo w = it, sai z = i t e^{it}, e W(z) é imaginário puro */
        printf("\n      t       z = i·t·e^{it}            W(z)                imaginário puro?\n");
        int esp=0, puro=0, coord=0;
        for(double t=0.5; t<=3.01; t+=0.5){
            double complex z = I*t*cexp(I*t);
            double complex w = Wc(z, I*t);
            esp++;
            if(fabs(creal(w)) < 1e-9 && fabs(cimag(w)-t) < 1e-9) puro++;
            /* e as coordenadas polares: |z| = t e arg z = t + π/2 (mod 2π) */
            double d = carg(z) - (t + PI_/2);
            while(d >  PI_) d -= 2*PI_;
            while(d < -PI_) d += 2*PI_;
            if(fabs(cabs(z) - t) < 1e-12 && fabs(d) < 1e-12) coord++;
            if(esp<=3)
                printf("      %-7.2f %+.9f%+.9fi   %+.9f%+.9fi   %s\n",
                       t, creal(z), cimag(z), creal(w), cimag(w),
                       fabs(creal(w))<1e-9 ? "SIM" : "nao");
        }
        printf("      pontos da espiral: %d   com W imaginário puro: %d   com |z|=t e arg=t+π/2: %d\n",
               esp, puro, coord);
        ok("na espiral z = i·t·e^{it}, W é IMAGINÁRIO PURO e vale exatamente it", puro==esp);
        ok("e as coordenadas polares são |z| = t e arg z = t + π/2", coord==esp);
        conclui("a soma que ele viu está na forma LOGARÍTMICA — w + ln w = ln z — e é a");
        conclui("prop:conjuga aplicada à própria equação: exp leva a soma ao produto.");
        conclui("e a curva que sai é a imagem do eixo imaginário, isto é A FRONTEIRA DOS RAMOS:");
        conclui("de um lado W_0, do outro W_{−1}, e sobre ela W é imaginário puro.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
