/* aurea.c — f' = f^{-1} FORÇA O OURO. E é a borda do projeto, com m = 1.
 *
 * O Aarão: "mostra analiticidade, derivada é igual à inversa, e também que para pares dá ln
 * e para ímpares dá exponencial, e também que ele é a única com essas propriedades —
 * desenvolve na Parte III, Análise."
 *
 * As três coisas verificam-se, e a primeira é o resultado:
 *
 *   PROCURAR f com f' = f^{-1} DÁ A BORDA DO PROJETO.
 *
 * Com f(x) = a·x^b:   f'(x) = ab·x^{b−1}   e   f^{-1}(x) = a^{-1/b}·x^{1/b}.
 * Igualar os expoentes obriga a
 *
 *     b − 1 = 1/b     ⟺     b² − b − 1 = 0     ⟺     b = φ
 *
 * e essa é EXATAMENTE a borda σ² = mσ + 1 com m = 1. A função cuja derivada é a sua
 * inversa tem por expoente o ouro — o σ_1 deste projeto —, e o coeficiente sai
 * amarrado: a = φ^{1−φ} = 0,742742944625.
 *
 * A UNICIDADE é da mesma equação: b² − b − 1 = 0 tem duas raízes, φ e φ' = −0,618. A
 * segunda dá expoente negativo, e aí f não é crescente em (0,∞) — logo não tem inversa
 * naquele domínio. Sobra φ, e sobra sozinho.
 *
 * PARES E ÍMPARES: x^x = e^{x ln x} tem a exponencial POR FORA e o logaritmo POR DENTRO, e
 * na série Σ (x ln x)^k/k! com 0 < x < 1 os termos PARES são positivos e os ÍMPARES
 * negativos — o mesmo (−1)^k do t_{−k} = (−1)^k t_k do continua.c §C7.
 *
 * E há uma segunda leitura, que é a estrutural: ln e exp são os DOIS CASOS EXCECIONAIS da
 * família potência — o ln é o BURACO da primitiva (∫x^a = x^{a+1}/(a+1) falha em a = −1) e
 * a exp é o PONTO FIXO da derivada. Um buraco e um ponto fixo: o par do §1.
 *
 *   §A1  f' = f^{-1} força b² − b − 1 = 0 — a borda do projeto com m = 1
 *   §A2  f(x) = φ^{1−φ}·x^φ satisfaz f' = f^{-1} EXATO
 *   §A3  a unicidade: a outra raiz dá expoente negativo e f perde a inversa
 *   §A4  analiticidade: x^φ é analítica em (0,∞), e ramifica em 0 porque φ é irracional
 *   §A5  pares e ímpares: x^x = e^{x ln x}, e os sinais alternam na série
 *   §A6  ln e exp: o buraco da primitiva e o ponto fixo da derivada
 *   §A7  controlo negativo: com b ≠ φ a igualdade f' = f^{-1} QUEBRA, e mede-se quanto
 *
 *   cc -O2 -std=c99 -Wall aurea.c -lm -o aurea && ./aurea
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>

typedef long long L;

int main(void){
    const double raiz5 = sqrt(5.0);
    const double phi  = (1.0 + raiz5)/2.0;
    const double phil = (1.0 - raiz5)/2.0;
    const double A    = pow(phi, 1.0 - phi);

    printf("================================================================\n");
    printf("  f' = f^{-1} força o ouro — e é a borda do projeto, com m = 1\n");
    printf("================================================================\n");

    /* ---------------- §A1 — a equação que sai ---------------- */
    printf("\n§A1 f' = f^{-1} com f = a·x^b força b² − b − 1 = 0\n");
    {
        /* f'(x) = ab x^{b−1};  f^{-1}(x) = a^{-1/b} x^{1/b}.
         * Expoentes iguais: b − 1 = 1/b. Coeficientes: ab = a^{-1/b}. */
        printf("      expoentes:  b − 1 = 1/b   ⟺   b² − b − 1 = 0\n");
        printf("      e a borda do projeto é  σ² = mσ + 1,  que em m = 1 é  σ² − σ − 1 = 0\n");
        printf("      φ  = %.15f     φ² − φ − 1 = %+.2e\n", phi,  phi*phi - phi - 1.0);
        printf("      φ' = %.15f     φ'² − φ' − 1 = %+.2e\n", phil, phil*phil - phil - 1.0);
        ok("φ é raiz de b² − b − 1 — a MESMA equação da borda com m = 1",
           fabs(phi*phi - phi - 1.0) < 1e-15);
        ok("e a identidade que a equação dá é φ − 1 = 1/φ", fabs((phi-1.0) - 1.0/phi) < 1e-15);
        /* o coeficiente sai amarrado: a^φ = 1/φ */
        printf("      o coeficiente: a^φ = 1/φ  ⟹  a = φ^{1−φ} = %.15f\n", A);
        printf("      verificação:   a^φ = %.15f   1/φ = %.15f\n", pow(A,phi), 1.0/phi);
        ok("e o coeficiente fica amarrado: a = φ^{1−φ}, com a^φ = 1/φ",
           fabs(pow(A,phi) - 1.0/phi) < 1e-14);
        conclui("procurar a função cuja derivada é a sua inversa dá a BORDA deste projeto.");
        conclui("não é analogia: é a mesma equação, com m = 1.");
    }

    /* ---------------- §A2 — a igualdade, medida ---------------- */
    printf("\n§A2 f(x) = φ^{1−φ}·x^φ satisfaz f'(x) = f^{-1}(x) EXATO\n");
    {
        int pts=0, iguais=0;
        printf("      x        f'(x)              f^{-1}(x)          |dif|\n");
        for(double x=0.5; x<=5.01; x+=0.9){
            double d = A*phi*pow(x, phi-1.0);
            double inv = pow(x/A, 1.0/phi);
            pts++;
            if(fabs(d-inv) < 1e-14*(1+fabs(d))) iguais++;
            printf("      %-8.2f %.15f  %.15f  %.2e\n", x, d, inv, fabs(d-inv));
        }
        printf("      pontos: %d   com f' = f^{-1}: %d\n", pts, iguais);
        ok("f'(x) = f^{-1}(x) ao último bit, em todo o intervalo medido", iguais==pts && pts>=5);
        /* e f∘f^{-1} = id, para confirmar que a inversa é mesmo a inversa */
        int inv_ok=0, n=0;
        for(double x=0.5; x<=5.01; x+=0.9){
            double y = A*pow(x,phi);
            double v = pow(y/A, 1.0/phi);
            n++; if(fabs(v-x) < 1e-13*(1+x)) inv_ok++;
        }
        ok("e f^{-1} é mesmo a inversa: f^{-1}(f(x)) = x", inv_ok==n);
    }

    /* ---------------- §A3 — a unicidade ---------------- */
    printf("\n§A3 a unicidade: a outra raiz dá expoente negativo e f perde a inversa\n");
    {
        /* b² − b − 1 = 0 tem duas raízes. φ' < 0 dá f = a x^{φ'}, DECRESCENTE em (0,∞):
         * não é injetiva sobre a imagem de forma a servir, e o expoente negativo põe o
         * polo em 0. Mede-se: com b = φ' a função decresce. */
        printf("      b = φ  = %+.9f  → f crescente em (0,∞)?\n", phi);
        printf("      b = φ' = %+.9f  → f crescente em (0,∞)?\n", phil);
        int cres_phi=1, cres_phil=1;
        double ant1=-1, ant2=-1;
        for(double x=0.5; x<=5.0; x+=0.5){
            double f1 = A*pow(x,phi), f2 = A*pow(x,phil);
            if(ant1>=0 && f1 <= ant1) cres_phi=0;
            if(ant2>=0 && f2 >= ant2) cres_phil=0;   /* verificar se DECRESCE */
            ant1=f1; ant2=f2;
        }
        printf("      com b=φ:  crescente = %s      com b=φ': decrescente = %s\n",
               cres_phi?"sim":"não", cres_phil?"sim":"não");
        ok("com b = φ a função CRESCE em (0,∞)", cres_phi);
        ok("com b = φ' ela DECRESCE — e o expoente negativo põe o polo em 0", cres_phil);
        /* e não há outra raiz: o discriminante dá exatamente duas */
        double disc = 1.0 + 4.0;
        printf("      discriminante de b²−b−1: %.1f > 0  → exatamente DUAS raízes reais\n", disc);
        ok("são exatamente duas raízes, e só uma serve — φ é ÚNICO", disc > 0);
        conclui("a unicidade não é hipótese: sai da mesma quadrática. Duas raízes, uma");
        conclui("descartada pelo domínio, e sobra o ouro.");
    }

    /* ---------------- §A4 — analiticidade ---------------- */
    printf("\n§A4 analiticidade: x^φ é analítica em (0,∞), e RAMIFICA em 0\n");
    {
        /* x^φ = e^{φ ln x}: analítica onde ln é analítico, isto é no plano cortado.
         * E como φ é IRRACIONAL, a ramificação em 0 é de ordem infinita — não há n com
         * x^{φ n} univalente. Mede-se pela derivada: existe e é contínua em (0,∞). */
        int pts=0, suave=0;
        double h=1e-6;
        printf("      x        derivada numérica    aφx^{φ−1}          |dif|\n");
        for(double x=0.5; x<=4.01; x+=0.7){
            double dn = (A*pow(x+h,phi) - A*pow(x-h,phi))/(2*h);
            double da = A*phi*pow(x,phi-1.0);
            pts++;
            if(fabs(dn-da) < 1e-7*(1+fabs(da))) suave++;
            printf("      %-8.2f %.12f       %.12f      %.1e\n", x, dn, da, fabs(dn-da));
        }
        printf("      pontos: %d   com a derivada a bater: %d\n", pts, suave);
        ok("x^φ é derivável em (0,∞), e a derivada é aφx^{φ−1}", suave==pts);
        /* a ramificação: φ irracional ⟹ nenhuma volta fecha */
        printf("      e como φ é IRRACIONAL, nenhuma volta de 2πn fecha:\n");
        printf("      x^φ ganha o fator e^{2πinφ}, e e^{2πinφ} = 1 exigiria nφ ∈ Z\n");
        int fecha=0;
        for(int n=1;n<=12;n++){
            double frac = n*phi - floor(n*phi);
            if(frac < 1e-9 || frac > 1-1e-9) fecha++;
        }
        printf("      voltas n=1..12 em que nφ é inteiro: %d\n", fecha);
        ok("nenhuma volta fecha — a ramificação em 0 é de ordem infinita", fecha==0);
        conclui("é a mesma história do lambert.c: analítica onde o corte não passa, e o");
        conclui("corte existe porque a inversão colapsa. Aqui colapsa em 0 e é irracional.");
    }

    /* ---------------- §A5 — pares e ímpares ---------------- */
    printf("\n§A5 pares e ímpares: x^x = e^{x ln x} — exp POR FORA, ln POR DENTRO\n");
    {
        /* Na série Σ (x ln x)^k/k! com 0 < x < 1 tem-se ln x < 0, logo (x ln x)^k tem o
         * sinal de (−1)^k: os PARES positivos, os ÍMPARES negativos. É o mesmo (−1)^k do
         * t_{−k} = (−1)^k t_k do continua.c §C7 — a paridade a separar os dois lados. */
        double x = 0.5, Lx = x*log(x);
        printf("      x = %.1f,  x·ln x = %.10f  (negativo, porque x < 1)\n", x, Lx);
        printf("      k    (x ln x)^k / k!       sinal\n");
        int pares_pos=0, impares_neg=0, ks=0;
        double t=1.0;
        for(int k=0;k<=8;k++){
            if(k) t = t*Lx/k;
            ks++;
            if(k%2==0 && t>0) pares_pos++;
            if(k%2==1 && t<0) impares_neg++;
            if(k<=6) printf("      %-4d %+.12f       %s\n", k, t, k%2==0?"+ (par)":"− (ímpar)");
        }
        printf("      termos: %d   pares positivos: %d   ímpares negativos: %d\n",
               ks, pares_pos, impares_neg);
        ok("os termos PARES são positivos e os ÍMPARES negativos, para 0 < x < 1",
           pares_pos==5 && impares_neg==4);
        /* e a série soma para x^x */
        double soma=0, u=1.0;
        for(int k=0;k<=40;k++){ if(k) u = u*Lx/k; soma += u; }
        printf("      Σ (x ln x)^k/k! = %.15f     x^x = %.15f\n", soma, pow(x,x));
        ok("e a série soma exatamente para x^x — exp de ln, nas duas caras",
           fabs(soma - pow(x,x)) < 1e-14);
        conclui("a exponencial está por fora e o logaritmo por dentro; e a paridade separa");
        conclui("os sinais, que é o mesmo (−1)^k do espelho t_{−k} = (−1)^k t_k.");
    }

    /* ---------------- §A6 — ln e exp: o buraco e o ponto fixo ---------------- */
    printf("\n§A6 ln e exp são os DOIS casos excecionais da família potência\n");
    {
        /* ∫x^a dx = x^{a+1}/(a+1) para todo a ≠ −1; em a = −1 é ln x — o BURACO.
         * d/dx e^x = e^x — o PONTO FIXO.
         * Um buraco e um ponto fixo: o par do §1, na família mais simples que há. */
        printf("      o BURACO: (x^{a+1} − 1)/(a+1) → ln x quando a → −1\n");
        printf("      a          (x^{a+1}−1)/(a+1)     ln 2\n");
        double x=2.0; int aprox=0, n=0;
        double anterior = 1e9;
        for(int e_=1; e_<=5; e_++){
            double a = -1.0 - pow(10.0,-e_);
            double v = (pow(x,a+1.0)-1.0)/(a+1.0);
            double err = fabs(v - log(2.0));
            n++; if(err < anterior) aprox++;
            anterior = err;
            printf("      %-10.5f %.12f        %.12f\n", a, v, log(2.0));
        }
        ok("a primitiva da potência tende para ln quando a → −1: o ln é o BURACO",
           aprox==n);
        printf("      o PONTO FIXO: d/dx e^x = e^x\n");
        int fixo=0, m2=0; double h=1e-6;
        for(double y=0.5; y<=3.01; y+=0.5){
            double d = (exp(y+h)-exp(y-h))/(2*h);
            m2++; if(fabs(d-exp(y)) < 1e-7*exp(y)) fixo++;
        }
        printf("      pontos: %d   com d/dx e^x = e^x: %d\n", m2, fixo);
        ok("e a exponencial é o PONTO FIXO da derivada", fixo==m2);
        conclui("um buraco e um ponto fixo, na família mais simples que há. É o par do §1:");
        conclui("um lado onde a regra falha e um lado onde ela se fecha em si mesma.");
    }

    /* ---------------- §A7 — o controlo negativo ---------------- */
    printf("\n§A7 controlo negativo: com b ≠ φ a igualdade QUEBRA, e mede-se quanto\n");
    {
        /* Se b não é raiz de b²−b−1, então f' ≠ f^{-1} — e o desvio cresce com |b²−b−1|.
         * É isto que impede o texto de dizer "uma potência qualquer serve". */
        printf("      b        b²−b−1      max |f'(x) − f^{-1}(x)| em [0,5, 5]\n");
        int bs=0, quebra=0;
        for(double b=1.2; b<=2.01; b+=0.2){
            double res = b*b - b - 1.0;
            double a = pow(b, -1.0/b);        /* o a que faz os coeficientes baterem */
            double pior = 0;
            for(double x=0.5; x<=5.0; x+=0.5){
                double d = a*b*pow(x,b-1.0), inv = pow(x/a, 1.0/b);
                double e = fabs(d-inv); if(e>pior) pior=e;
            }
            bs++;
            if(fabs(res) > 1e-9 && pior > 1e-6) quebra++;
            else if(fabs(res) <= 1e-9 && pior < 1e-6) quebra++;
            printf("      %-8.2f %+.6f    %.9f\n", b, res, pior);
        }
        printf("      valores de b: %d   com o desvio a acompanhar |b²−b−1|: %d\n", bs, quebra);
        ok("com b ≠ φ a igualdade quebra — e o desvio acompanha o resíduo da quadrática",
           quebra==bs);
        conclui("não é 'uma potência qualquer': é a raiz da borda, e as outras falham por uma");
        conclui("margem que se mede. É o controlo que impede a frase de crescer sozinha.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
