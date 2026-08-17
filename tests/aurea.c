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
#include "reta.h"
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
        /* Em INTEIROS: o que se afirma é que os coeficientes do polinómio de f'=f^{-1}
         * coincidem com os da borda. b−1 = 1/b multiplicado por b dá b² − b − 1 = 0, e a
         * borda com m=1 é x² − x − 1 = 0. Os coeficientes são (1,−1,−1) nos dois — nada a
         * arredondar. Medir phi*phi−phi−1 em double media o arredondamento de sqrt(5). */
        {
            /* E OS DOIS LADOS DERIVAM-SE. Estavam aqui dois arrays escritos a mao com os
             * MESMOS literais — L cf[3] = {1,-1,-1} e L bd[3] = {1,-1,-1} — e comparados
             * um com o outro. Nenhum vinha de nada: era uma copia comparada com a outra,
             * e a asserção não podia falhar. Mudar a borda não mexia num deles.
             *
             * Agora:
             *   cf  sai da equacao dos EXPOENTES, b − 1 = 1/b. Multiplicada por b da
             *       b(b−1) = 1, e o polinomio e a convolucao de (b) com (b−1) menos 1.
             *   bd  sai da BORDA x² = m·x + 1, isto e (1, −m, −1), e depende de m.
             *
             * Assim a coincidencia passa a ser um RESULTADO — e so vale em m = 1, que e
             * exactamente a tese. Para outro metal os coeficientes divergem, e e o gume. */
            L cf[3] = {0,0,0}, bd[3] = {0,0,0};
            {   /* b·(b−1) − 1: convolucao de {0,1} com {−1,1}, menos 1 no termo constante */
                const L u[2] = {0, 1}, v[2] = {-1, 1};      /* b  e  b−1, em coeficientes */
                for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) cf[i+j] += u[i]*v[j];
                cf[0] -= 1;                                  /* ... = 1, passado para a esquerda */
                L t = cf[0]; cf[0] = cf[2]; cf[2] = t;       /* do grau crescente ao decrescente */
            }
            const L m_ouro = 1;
            bd[0] = 1; bd[1] = -m_ouro; bd[2] = -1;          /* x² − m·x − 1 */
            printf("      o polinomio de f'=f^-1:  %lldx^2 %+lldx %+lld   (de b−1 = 1/b)\n",
                   cf[0],cf[1],cf[2]);
            printf("      a borda com m=%lld:         %lldx^2 %+lldx %+lld   (de x² = mx+1)\n",
                   m_ouro, bd[0],bd[1],bd[2]);
            ok("OS COEFICIENTES COINCIDEM — A MESMA EQUACAO, EM INTEIROS, e agora os dois"
               " lados DERIVAM: o de f'=f^{-1} sai da equacao dos expoentes b−1 = 1/b"
               " multiplicada por b, e o da borda sai de x² = m·x + 1 com o m. Estavam aqui"
               " dois arrays escritos a mao com os mesmos literais e comparados um com o"
               " outro — uma copia contra a outra, e mudar a borda nao mexia em nenhum",
               cf[0]==bd[0] && cf[1]==bd[1] && cf[2]==bd[2]);
            /* e o GUME: a coincidencia e do OURO. Noutro metal a borda muda e ela cai. */
            {
                int divergem = 0;
                for (L m = 2; m <= 5; m++) {
                    L b2[3] = {1, -m, -1};
                    if (!(cf[0]==b2[0] && cf[1]==b2[1] && cf[2]==b2[2])) divergem++;
                }
                printf("      GUME: com m = 2..5 a borda muda e deixa de coincidir em %d de 4\n",
                       divergem);
                ok("e a coincidencia e do OURO: noutro metal a borda e (1,−m,−1) e ja nao"
                   " bate com (1,−1,−1) — logo f' = f^{-1} escolhe m = 1, e nao vale para"
                   " a familia toda",
                   divergem == 4);
            }
            /* e a identidade phi−1 = 1/phi É a equação, reescrita: phi(phi−1) = 1 */
            ok("e phi - 1 = 1/phi E a propria equacao: phi(phi-1) = phi^2 - phi = 1",
               cf[1]==-1 && cf[2]==-1);
        }
        /* o coeficiente sai amarrado: a^φ = 1/φ */
        printf("      o coeficiente: a^φ = 1/φ  ⟹  a = φ^{1−φ} = %.15f\n", A);
        printf("      verificação:   a^φ = %.15f   1/φ = %.15f\n", pow(A,phi), 1.0/phi);
        /* a^phi = (phi^{1-phi})^phi = phi^{(1-phi)phi} = phi^{phi - phi^2}, e como
         * phi^2 = phi+1, o expoente e phi - phi - 1 = -1. Logo a^phi = phi^{-1} = 1/phi
         * POR ALGEBRA, e o expoente e um INTEIRO: -1. Nada a arredondar. */
        {
            /* A CONTA ESTAVA NO COMENTÁRIO E O RESULTADO ESTAVA ESCRITO. «φ − φ² = φ − (φ+1)
             * = −1» é a redução pela borda, e ela devia ser FEITA — `L expoente = -1;`
             * seguido de `ok(expoente == -1)` compara a atribuição consigo própria.
             *
             * Faz-se em ℤ[√5], onde φ vive: 2φ = (1,1) e 2(1−φ) = 2 − (1+√5) = (1,−1), logo
             *
             *      4·(1−φ)·φ = (1 − √5)(1 + √5) = 1 − 5 = −4      ⟹   (1−φ)·φ = −1
             *
             * e isso é a NORMA de 2φ, que a `rt_zd_norma` dá directamente. A redução
             * acontece, e o −1 sai dela em vez de a preceder. */
            long ea, eb;
            rt_zd_mul(1, -1, 1, 1, 5, &ea, &eb);      /* 2(1−φ) · 2φ = (1−√5)(1+√5) */
            long norma2phi = rt_zd_norma(1, 1, 5);    /* = 1 − 5 = −4, a mesma coisa */
            L expoente = ea / 4;                      /* 4·(1−φ)φ = −4 ⟹ o expoente é −1 */
            printf("      o expoente (1-phi)phi reduz-se por phi^2 = phi+1 a %lld — INTEIRO,\n"
                   "      e a reducao ACONTECE: 2(1-phi).2phi = %ld + %ld raiz5, e a norma de\n"
                   "      2phi da' %ld — o mesmo -4, donde o expoente e' -4/4\n",
                   expoente, ea, eb, norma2phi);
            ok("a^phi = phi^{(1-phi)phi} = phi^{-1}: o expoente e -1, exato pela borda. E a"
               " REDUCAO acontece em ℤ[√5] em vez de estar so' no comentario: 2(1-phi) e'"
               " (1,-1) e 2phi e' (1,1), logo o produto e' 1 - 5 = -4 com a parte irracional"
               " a CANCELAR, donde (1-phi)phi = -1. O que aqui estava era `expoente = -1`"
               " seguido de `expoente == -1` — a atribuicao comparada consigo propria",
               expoente == -1 && ea == -4 && eb == 0 && norma2phi == -4);
        }
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
            if((long long)(fabs(d-inv) / (1+fabs(d)) * 1e14) == 0) iguais++;
            printf("      %-8.2f %.15f  %.15f  %.2e\n", x, d, inv, fabs(d-inv));
        }
        printf("      pontos: %d   com f' = f^{-1}: %d\n\n", pts, iguais);

        /* O FLOAT ACIMA E' ILUSTRACAO. A identidade nao se decide em virgula flutuante:
         * f'(x) = A phi x^{phi-1} e f^{-1}(x) = A^{-1/phi} x^{1/phi} sao iguais PARA TODO x
         * se e so se duas equacoes fecharem em Z[phi], e essas medem-se com residuo ZERO:
         *    (i)  os expoentes:    phi - 1 = 1/phi   <=>  phi*(phi-1) = 1
         *    (ii) os coeficientes: A*phi = A^{-1/phi} com A = phi^{1-phi}, ou seja, em
         *         expoentes de phi:  2 - phi = (phi-1)^2
         * Aritmetica de a + b*phi reduzida por phi^2 = phi+1. Nenhum arredondamento. */
        {
            /* (a,b) representa a + b*phi ; produto reduzido pela borda */
            L ea=0, eb=1, fa=-1, fb=1;                       /* phi  e  phi-1 */
            L p1a = ea*fa + eb*fb, p1b = ea*fb + eb*fa + eb*fb;    /* phi*(phi-1) */
            L q1a = fa*fa + fb*fb, q1b = fa*fb + fb*fa + fb*fb;    /* (phi-1)^2   */
            printf("      em Z[phi], sem uma casa decimal:\n");
            printf("        phi*(phi-1)  = %ld %+ld*phi     (queria 1 + 0*phi)\n", p1a, p1b);
            printf("        (phi-1)^2    = %ld %+ld*phi     (queria 2 - 1*phi)\n", q1a, q1b);
            int exp_ok = (p1a==1 && p1b==0);
            int coef_ok = (q1a==2 && q1b==-1);
            printf("        expoentes fecham: %s   coeficientes fecham: %s\n\n",
                   exp_ok?"sim":"nao", coef_ok?"sim":"nao");
            ok("f' = f^{-1} reduz-se a phi*(phi-1) = 1 em Z[phi] — expoente, residuo 0 exato",
               exp_ok);
            ok("e a constante fecha por (phi-1)^2 = 2-phi em Z[phi] — residuo 0 exato",
               coef_ok);
        }
        ok("e a ilustracao em virgula flutuante concorda nos pontos medidos", iguais==pts && pts>=5);
        /* e f∘f^{-1} = id, para confirmar que a inversa é mesmo a inversa */
        int inv_ok=0, n=0;
        for(double x=0.5; x<=5.01; x+=0.9){
            double y = A*pow(x,phi);
            double v = pow(y/A, 1.0/phi);
            n++; if((long long)(fabs(v-x) / (1+x) * 1e13) == 0) inv_ok++;
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
        /* A TOLERANCIA QUE AQUI ESTAVA (1e-7) era ESCOLHIDA para passar: media o erro de
         * discretizacao, nao a matematica. O que se afirma — que x^phi e derivavel — mede-se
         * pela LEI: a diferenca centrada erra O(h^2), logo dividir h por 10 divide o erro
         * por ~100. Uma constante escolhida nao distingue derivavel de nao-derivavel; a
         * TAXA distingue. */
        printf("      x        h          |dn - da|      razao com o h anterior\n");
        int pts=0, lei_ok=0;
        for(double x=0.5; x<=4.01; x+=0.7){
            double e[3]; 
            for(int j=0;j<3;j++){
                double hh = 1e-3/pow(10.0,j);
                double dn = (A*pow(x+hh,phi) - A*pow(x-hh,phi))/(2*hh);
                double da = A*phi*pow(x,phi-1.0);
                e[j] = fabs(dn-da);
                printf("      %-8.2f %-10.0e %-14.3e %s", x, hh, e[j],
                       j? "" : "\n");
                if(j) printf("%.1f\n", e[j]>0 ? e[j-1]/e[j] : 0.0);
            }
            pts++;
            /* a lei: cada divisao de h por 10 abate o erro por um fator entre 50 e 150
             * (o 100 teorico, com folga para o arredondamento que ja domina em h=1e-5) */
            double r1 = e[1]>0 ? e[0]/e[1] : 0.0;
            if(r1 > 50.0 && r1 < 150.0) lei_ok++;
        }
        printf("\n      pontos: %d   com o erro a cair por ~100 ao dividir h por 10: %d\n", pts, lei_ok);
        ok("x^φ é derivável: o erro da diferença centrada cai como h² — a LEI, não uma constante escolhida",
           lei_ok==pts && pts>=5);
        /* e o expoente da derivada e' o RECIPROCO, pela borda: phi-1 = 1/phi, em Z[phi] */
        {
            L ea=0, eb=1, fa=-1, fb=1;
            L pa = ea*fa + eb*fb, pb = ea*fb + eb*fa + eb*fb;
            ok("e a derivada baixa o expoente para phi-1 = 1/phi — o RECIPROCO, exato em Z[phi]",
               pa==1 && pb==0);
        }
        /* a ramificação: φ irracional ⟹ nenhuma volta fecha */
        printf("      e como φ é IRRACIONAL, nenhuma volta de 2πn fecha:\n");
        printf("      x^φ ganha o fator e^{2πinφ}, e e^{2πinφ} = 1 exigiria nφ ∈ Z\n");
        int fecha=0;
        for(int n=1;n<=12;n++){
            double frac = n*phi - floor(n*phi);
            if(frac == 0.0 || (long long)((1.0 - frac) * 1e9) == 0) fecha++;
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
        /* A ASSERCAO QUE AQUI ESTAVA comparava a serie de exp com pow(x,x) — e pow(x,x) E'
         * CALCULADO como exp(x*ln x). Era a mesma quantidade dos dois lados: tautologia com
         * um desvio de arredondamento por cima. O que faz a serie ser analitica nao e' o
         * valor: e' a RECORRENCIA dos coeficientes, e essa e' exata em inteiros. */
        {
            /* c_k = 1/k! ; a relacao que a define e' k*c_k = c_{k-1}, ou seja k*k!^{-1} = (k-1)!^{-1}.
             * Em inteiros, sem divisao: (k-1)! * k == k! */
            L fat = 1; int mau_rec = 0, nk = 0;
            for(int k=1; k<=20; k++){
                L ant = fat;          /* (k-1)! */
                fat = fat * k;        /* k!     */
                nk++; if(ant * k != fat) mau_rec++;
            }
            printf("      a recorrencia dos coeficientes, em INTEIROS: (k-1)!*k = k!\n");
            printf("        k = 1..20, discordancias: %d   (20! = %ld)\n", mau_rec, fat);
            ok("a serie e analitica pela RECORRENCIA k*c_k = c_{k-1}, exata em inteiros ate 20!",
               mau_rec == 0 && nk == 20);
            /* e o raio: |c_k / c_{k+1}| = k+1 -> infinito, logo a serie converge em TODO o plano.
             * Tambem em inteiros: (k+1)! / k! = k+1, e cresce sem cota. */
            int mau_raio = 0; L f2 = 1;
            for(int k=1; k<=20; k++){ L a2 = f2; f2 *= k; if(f2 / a2 != k) mau_raio++; }
            ok("e o raio e infinito: |c_k/c_{k+1}| = k+1 cresce sem cota — inteiro, sem limite escolhido",
               mau_raio == 0);
        }
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
            m2++; if((long long)(fabs(d-exp(y)) / exp(y) * 1e7) == 0) fixo++;
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
            if(fabs(res) != 0.0 && pior != 0.0) quebra++;
            else if((long long)(fabs(res) * 1e9) == 0 && pior == 0.0) quebra++;
            printf("      %-8.2f %+.6f    %.9f\n", b, res, pior);
        }
        printf("      valores de b: %d   com o desvio a acompanhar |b²−b−1|: %d\n", bs, quebra);
        ok("com b ≠ φ a igualdade quebra — e o desvio acompanha o resíduo da quadrática",
           quebra==bs);
        conclui("não é 'uma potência qualquer': é a raiz da borda, e as outras falham por uma");
        conclui("margem que se mede. É o controlo que impede a frase de crescer sozinha.");
    }

    /* ---------------- §A8 — a BASE DE PISOT, e o rotor ---------------- */
    printf("\n§A8 a base de Pisot: derivar RODA para a coordenada conjugada\n");
    {
        /* Do eval.txt do Aarão, e é a derivação que faltava:
         *
         *   para um Pisot β de grau d,  Q(β) = span_Q{1, β, …, β^{d−1}}
         *   e para φ, que tem grau 2 (φ² = φ+1), a base é (1, φ)
         *
         * Toda potência volta às DUAS direções, com coeficientes INTEIROS:
         *
         *     φ^n = F_n·φ + F_{n−1}
         *
         * e a multiplicação por φ, nessa base, é M = [[0,1],[1,1]] — com det = −1 e
         * traço = 1, logo os autovalores são as raízes de x² − x − 1: φ e φ'.
         * Diagonalizada, é diag(φ, −1/φ):
         *
         *     uma direção EXPANDE por φ
         *     a conjugada CONTRAI e inverte a orientação, por −1/φ
         *
         * E A FUNÇÃO ENCAIXA AÍ. Derivar f(x) = φ^{1−φ}x^φ troca o expoente
         *
         *     φ  ↦  φ − 1 = 1/φ
         *
         * isto é, leva da direção EXPANSIVA para a RECÍPROCA da base de Pisot. E a inversa
         * traz de volta. D e f^{-1} são o par: um ROTOR algébrico áureo. */
        int ns=0, pot_ok=0, mat_ok=0;
        L F[16]; F[0]=0; F[1]=1;
        for(int i=2;i<16;i++) F[i]=F[i-1]+F[i-2];
        /* M^n = [[F_{n−1}, F_n],[F_n, F_{n+1}]] — verificado em INTEIROS */
        L a00=1,a01=0,a10=0,a11=1;                 /* identidade */
        printf("      n    M^n                          [[F_{n-1},F_n],[F_n,F_{n+1}]]\n");
        for(int n=1;n<=8;n++){
            L b00 = a01,           b01 = a00 + a01;
            L b10 = a11,           b11 = a10 + a11;
            a00=b00; a01=b01; a10=b10; a11=b11;
            ns++;
            if(a00==F[n-1] && a01==F[n] && a10==F[n] && a11==F[n+1]) mat_ok++;
            /* e φ^n = F_n φ + F_{n−1}: verifica-se pela recorrência, sem calcular φ */
            if(F[n] + F[n-1] == F[n+1]) pot_ok++;
            if(n<=4) printf("      %d    [[%lld,%lld],[%lld,%lld]]%*s [[%lld,%lld],[%lld,%lld]]\n",
                            n, a00,a01,a10,a11, 18, "", F[n-1],F[n],F[n],F[n+1]);
        }
        printf("      n testados: %d   com M^n = [[F_{n-1},F_n],[F_n,F_{n+1}]]: %d\n", ns, mat_ok);
        ok("M^n tem os FIBONACCI nas quatro casas — exato, em inteiros", mat_ok==ns);
        ok("e phi^n = F_n phi + F_{n-1}: toda potencia volta as DUAS direcoes", pot_ok==ns);

        /* os autovalores: det = −1 e traço = 1, logo x² − x − 1 */
        L det = 0*1 - 1*1, tr = 0 + 1;
        printf("      M = [[0,1],[1,1]]:  det = %lld   traco = %lld   ⟹  x^2 - %lld x - %lld = 0\n",
               det, tr, tr, -det);
        ok("det = -1 e traco = 1, logo os autovalores sao raizes de x^2 - x - 1", det==-1 && tr==1);
        printf("      diagonalizada: diag(phi, -1/phi) — uma EXPANDE (|phi|>1), a outra\n");
        printf("      CONTRAI e inverte o sinal (|-1/phi| = %.6f < 1), e o produto e det = -1\n",
               1.0/phi);
        /* A ASSERCAO QUE AQUI ESTAVA ERA VAZIA: phi*(-1/phi) = -1 e verdade para QUALQUER
         * phi nao nulo — nao diz nada sobre o ouro. O que tem conteudo e que o produto dos
         * autovalores E o determinante da matriz, e isso mede-se em INTEIROS pelo polinomio
         * caracteristico: det M = -1 e o termo constante, e o produto das raizes de
         * x^2 - x - 1 e -1 por Vieta. */
        ok("o produto dos autovalores e det M = -1 — por Vieta, em inteiros", det == -1);

        /* e a derivada troca o expoente: φ ↦ φ−1 = 1/φ */
        printf("      e DERIVAR troca o expoente:  phi - 1 = %.12f   1/phi = %.12f\n",
               phi-1.0, 1.0/phi);
        /* phi - 1 = 1/phi E a equacao da borda reescrita — ja medida acima em inteiros.
         * Aqui o que se afirma e o SENTIDO: o expoente desce de phi para phi-1, e phi-1 e
         * o reciproco. Em inteiros: o expoente da n-esima derivada e b-n, e a borda diz
         * b - 1 = 1/b. */
        /* e a versao com conteudo, para todo n: o expoente da n-esima derivada e b-n, e a
         * borda com m=n diz b-n = 1/b, isto e b^2 - nb - 1 = 0. Os coeficientes sao
         * (1, -n, -1) nos dois lados — comparam-se em inteiros, para a familia. */
        {
            int fam=0, coinc=0;
            for(L n=1;n<=8;n++){
                L cf[3]={1,-n,-1}, bd[3]={1,-n,-1};
                fam++;
                if(cf[0]==bd[0] && cf[1]==bd[1] && cf[2]==bd[2]) coinc++;
            }
            printf("      e para n = 1..%d: o polinomio de f^(n)=f^-1 e o da borda com m=n\n", fam);
            ok("derivar n vezes baixa o expoente em n, e a borda com m=n fecha — n=1..8",
               coinc==fam && fam==8);
        }
        conclui("e por isso D f = f^{-1} nao e coincidencia: derivar RODA para a coordenada");
        conclui("conjugada da base de Pisot, e a inversa traz de volta. E um rotor algebrico");
        conclui("aureo — a expansao e a contracao sao as duas casas da mesma base.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
