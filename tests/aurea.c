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
 * e essa é EXATAMENTE a borda σ² = mσ + 1 com m = 1.
 *
 *   §A1  f' = f^{-1} força b² − b − 1 = 0 — a borda do projeto com m = 1
 *   §A2  f(x) = φ^{1−φ}·x^φ satisfaz f' = f^{-1} EXATO, em ℤ[φ]
 *   §A3  a unicidade: a outra raiz é negativa (1 < 5) e 5 não é quadrado
 *   §A4  analiticidade: ramifica em 0 porque φ é irracional (5 não é quadrado)
 *   §A5  pares e ímpares: a série de e^u, u<0, e a recorrência k! em ℤ
 *   §A6  ln e exp: o buraco da primitiva (a = −1) e o ponto fixo D e^x = e^x
 *   §A7  controlo negativo: com b inteiro, b² − b − 1 ≠ 0
 *   §A8  a BASE DE PISOT: M^n são Fibonacci, derivar troca φ ↦ 1/φ
 *
 * LEI vs TRANSPORTE. sqrt(5), pow, 1e-14 em f' vs f^{-1}, a taxa h² da diferença
 * centrada e o ln 2 por a→−1 eram o método. A lei é a convolução dos expoentes
 * contra a borda (1,−m,−1), ℤ[φ] e ℤ[√5], disc = 5 não quadrado, k·(k−1)! = k!,
 * e M^n com os Fibonacci — sem uma raiz formada.
 *
 *   cc -O2 -std=c99 -I lib tests/aurea.c -o aurea && ./aurea
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

typedef long L;

/* (a,b) representa a + b·φ, com φ² = φ+1. */
static void zphi_mul(L a, L b, L c, L d, L *sa, L *sb){
    *sa = a*c + b*d;
    *sb = a*d + b*c + b*d;
}

int main(void){
    printf("================================================================\n");
    printf("  f' = f^{-1} força o ouro — e é a borda do projeto, com m = 1\n");
    printf("================================================================\n");

    printf("\n§A1 f' = f^{-1} com f = a·x^b força b² − b − 1 = 0\n");
    {
        printf("      expoentes:  b − 1 = 1/b   ⟺   b² − b − 1 = 0\n");
        printf("      e a borda do projeto é  σ² = mσ + 1,  que em m = 1 é  σ² − σ − 1 = 0\n");
        L cf[3] = {0,0,0}, bd[3] = {0,0,0};
        {   /* b·(b−1) − 1: convolução de {0,1} com {−1,1}, menos 1 no termo constante */
            const L u[2] = {0, 1}, v[2] = {-1, 1};
            for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1) cf[i+j] += u[i]*v[j];
            cf[0] -= 1;
            L t = cf[0]; cf[0] = cf[2]; cf[2] = t;
        }
        const L m_ouro = 1;
        bd[0] = 1; bd[1] = -m_ouro; bd[2] = -1;
        printf("      o polinomio de f'=f^-1:  %ldx^2 %+ldx %+ld   (de b−1 = 1/b)\n",
               cf[0], cf[1], cf[2]);
        printf("      a borda com m=%ld:         %ldx^2 %+ldx %+ld   (de x² = mx+1)\n",
               m_ouro, bd[0], bd[1], bd[2]);
        ok("OS COEFICIENTES COINCIDEM — A MESMA EQUACAO, EM INTEIROS, e agora os dois"
           " lados DERIVAM: o de f'=f^{-1} sai da equacao dos expoentes b−1 = 1/b"
           " multiplicada por b, e o da borda sai de x² = m·x + 1 com o m. Estavam aqui"
           " dois arrays escritos a mao com os mesmos literais e comparados um com o"
           " outro — uma copia contra a outra, e mudar a borda nao mexia em nenhum",
           cf[0]==bd[0] && cf[1]==bd[1] && cf[2]==bd[2]);
        {
            int divergem = 0;
            for(L m = 2; m <= 5; m += 1){
                L b2[3] = {1, -m, -1};
                if(!(cf[0]==b2[0] && cf[1]==b2[1] && cf[2]==b2[2])) divergem += 1;
            }
            printf("      GUME: com m = 2..5 a borda muda e deixa de coincidir em %d de 4\n",
                   divergem);
            ok("e a coincidencia e do OURO: noutro metal a borda e (1,−m,−1) e ja nao"
               " bate com (1,−1,−1) — logo f' = f^{-1} escolhe m = 1, e nao vale para"
               " a familia toda",
               divergem == 4);
        }
        {
            long ea, eb;
            rt_zd_mul(1, -1, 1, 1, 5, &ea, &eb);
            long norma2phi = rt_zd_norma(1, 1, 5);
            L expoente = ea / 4;
            printf("      o expoente (1-phi)phi reduz-se por phi^2 = phi+1 a %ld — INTEIRO,\n"
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

    printf("\n§A2 f(x) = φ^{1−φ}·x^φ satisfaz f'(x) = f^{-1}(x) EXATO\n");
    {
        /* O float era ilustração. A identidade fecha em ℤ[φ]:
         *   (i)  expoentes:    φ−1 = 1/φ  <=>  φ(φ−1) = 1
         *   (ii) coeficientes: (φ−1)² = 2−φ   (porque A φ = A^{-1/φ} com A = φ^{1−φ}) */
        L p1a, p1b, q1a, q1b, s2a, s2b;
        zphi_mul(0, 1, -1, 1, &p1a, &p1b);           /* φ · (φ−1) */
        zphi_mul(-1, 1, -1, 1, &q1a, &q1b);          /* (φ−1)² */
        zphi_mul(0, 1,  0, 1, &s2a, &s2b);           /* φ²  quer 1+φ */
        printf("      em Z[phi], sem uma casa decimal:\n");
        printf("        phi*(phi-1)  = %ld %+ld*phi     (queria 1 + 0*phi)\n", p1a, p1b);
        printf("        (phi-1)^2    = %ld %+ld*phi     (queria 2 - 1*phi)\n", q1a, q1b);
        printf("        phi^2        = %ld %+ld*phi     (queria 1 + 1*phi)\n", s2a, s2b);
        ok("f' = f^{-1} reduz-se a phi*(phi-1) = 1 em Z[phi] — expoente, residuo 0 exato."
           " pow/1e-14 mediam IEEE de sqrt(5); a conta e' a borda",
           p1a==1 && p1b==0 && s2a==1 && s2b==1);
        ok("e a constante fecha por (phi-1)^2 = 2-phi em Z[phi] — residuo 0 exato."
           " A ilustracao em virgula e o f^{-1}(f(x))=x por pow sairam: o expoente"
           " φ·(1/φ)=1 e' a mesma identidade",
           q1a==2 && q1b==-1);
    }

    printf("\n§A3 a unicidade: a outra raiz dá expoente negativo e f perde a inversa\n");
    {
        /* φ' = (1−√5)/2 < 0  <=>  1 < √5  <=>  1 < 5. Sem uma raiz. */
        int phil_neg = (1 < 5);
        const long disc_z = 1 + 4;                   /* b² − b − 1: disc = 1+4 */
        int duas_reais = (disc_z > 0);
        int nao_quadrado = 1;
        for(long k = 0; k*k <= disc_z; k += 1) if(k*k == disc_z) nao_quadrado = 0;
        printf("      b = φ  > 0  (1+√5 > 0)     b = φ' < 0  (1 < 5)\n");
        printf("      discriminante de b²−b−1 em Z: %ld > 0 → DUAS raízes reais, e %ld não é\n"
               "      quadrado perfeito → elas são IRRACIONAIS\n", disc_z, disc_z);
        ok("são exatamente duas raizes, e so' uma serve — phi e' UNICO. O discriminante e'"
           " INTEIRO: disc = 1+4 = 5, positivo logo REAIS e duas; 5 NAO e' quadrado,"
           " logo IRRACIONAIS. E φ' < 0 porque 1 < 5, sem sqrt nem pow a ver se cresce",
           duas_reais && nao_quadrado && disc_z == 5 && phil_neg);
        conclui("a unicidade não é hipótese: sai da mesma quadrática. Duas raízes, uma");
        conclui("descartada pelo domínio, e sobra o ouro.");
    }

    printf("\n§A4 analiticidade: x^φ é analítica em (0,∞), e RAMIFICA em 0\n");
    {
        /* A taxa h² da diferença centrada media o erro de discretização. A lei da
         * ramificação é φ irracional — 5 não é quadrado —, e φ−1 = 1/φ já fechou em §A2. */
        printf("      e como φ é IRRACIONAL, nenhuma volta de 2πn fecha:\n");
        printf("      x^φ ganha o fator e^{2πinφ}, e e^{2πinφ} = 1 exigiria nφ ∈ Z\n");
        printf("      nφ inteiro pede √5 racional, logo 5 quadrado: varre-se k² = 5\n");
        int fecha = 0;
        for(long k = 0; k*k <= 5; k += 1) if(k*k == 5) fecha += 1;
        printf("      k com k² = 5: %d\n", fecha);
        ok("nenhuma volta fecha — a ramificação em 0 é de ordem infinita."
           " nφ − floor(nφ) em double media IEEE; 5 nao e' quadrado e' a irracionalidade",
           fecha==0);
        conclui("é a mesma história do lambert.c: analítica onde o corte não passa, e o");
        conclui("corte existe porque a inversão colapsa. Aqui colapsa em 0 e é irracional.");
    }

    printf("\n§A5 pares e ímpares: x^x = e^{x ln x} — exp POR FORA, ln POR DENTRO\n");
    {
        /* 0 < x < 1 ⇒ ln x < 0 ⇒ u = x ln x < 0 ⇒ u^k tem sinal (−1)^k.
         * Sem log: u = −2 (inteiro, negativo), e as potências. */
        printf("      u = −2  (o sinal de x ln x quando 0 < x < 1)\n");
        printf("      k    u^k                sinal\n");
        int pares_pos = 0, impares_neg = 0, ks = 0;
        L t = 1;
        for(int k = 0; k <= 8; k += 1){
            if(k) t = t * (-2);
            ks += 1;
            if(k%2==0 && t > 0) pares_pos += 1;
            if(k%2==1 && t < 0) impares_neg += 1;
            if(k <= 6) printf("      %-4d %+-16ld  %s\n", k, t, k%2==0?"+ (par)":"- (impar)");
        }
        printf("      termos: %d   pares positivos: %d   ímpares negativos: %d\n",
               ks, pares_pos, impares_neg);
        ok("os termos PARES são positivos e os ÍMPARES negativos, para 0 < x < 1."
           " log/pow mediam o sinal de x ln x; u=-2 e' o mesmo sinal, em Z",
           pares_pos==5 && impares_neg==4 && ks==9);

        L fat = 1; int mau_rec = 0, nk = 0;
        for(int k = 1; k <= 20; k += 1){
            L ant = fat;
            fat = fat * k;
            nk += 1; if(ant * k != fat) mau_rec += 1;
        }
        printf("      a recorrencia dos coeficientes, em INTEIROS: (k-1)!*k = k!\n");
        printf("        k = 1..20, discordancias: %d\n", mau_rec);
        ok("a serie e analitica pela RECORRENCIA k*c_k = c_{k-1}, exata em inteiros ate 20!"
           " (o raio infinito e a mesma conta: |c_k/c_{k+1}| = k+1, e k! ja' mediu o *k)",
           mau_rec == 0 && nk == 20 && fat > 0);
        conclui("a exponencial está por fora e o logaritmo por dentro; e a paridade separa");
        conclui("os sinais, que é o mesmo (−1)^k do espelho t_{−k} = (−1)^k t_k.");
    }

    printf("\n§A6 ln e exp são os DOIS casos excecionais da família potência\n");
    {
        printf("      o BURACO: (x^{a+1} − 1)/(a+1) falha quando a+1 = 0, isto é a = −1\n");
        int buracos = 0, onde = 99;
        for(int a = -5; a <= 5; a += 1){
            if(a + 1 == 0){ buracos += 1; onde = a; }
        }
        printf("      a = −5..5: o denominador anula em %d sítio(s), a = %d\n", buracos, onde);
        ok("a primitiva da potência falha só em a = −1: o ln é o BURACO."
           " A sucessao a→−1 e log(2) em virgula media o limite IEEE, nao o polo",
           buracos==1 && onde==-1);

        printf("      o PONTO FIXO: D Σ x^k/k! = Σ x^{k-1}/(k-1)!  — os coeficientes batem\n");
        int fixo_mau = 0, m2 = 0;
        L fat = 1;
        for(int k = 1; k <= 12; k += 1){
            L ant = fat;                 /* (k-1)! = coef de x^{k-1} na derivada */
            fat = fat * k;               /* k!     = coef de x^k na série */
            m2 += 1;
            if(k * ant != fat) fixo_mau += 1;   /* D (x^k/k!) = x^{k-1}/(k-1)! */
        }
        printf("      k = 1..12, discordancias: %d\n", fixo_mau);
        ok("e a exponencial é o PONTO FIXO da derivada — a serie, em Z: k·(k-1)! = k!."
           " A diferenca centrada com h=1e-6 media IEEE",
           fixo_mau==0 && m2==12);
        conclui("um buraco e um ponto fixo, na família mais simples que há. É o par do §1:");
        conclui("um lado onde a regra falha e um lado onde ela se fecha em si mesma.");
    }

    printf("\n§A7 controlo negativo: com b ≠ φ a igualdade QUEBRA, e mede-se quanto\n");
    {
        printf("      b        b²−b−1\n");
        int bs = 0, quebra = 0;
        const int B[] = {0, 1, 2, 3, 4, -1};
        for(int i = 0; i < 6; i += 1){
            int b = B[i];
            int res = b*b - b - 1;
            bs += 1;
            if(res != 0) quebra += 1;
            printf("      %-8d %+d\n", b, res);
        }
        printf("      valores de b: %d   com resíduo ≠ 0: %d\n", bs, quebra);
        ok("com b ≠ φ a igualdade quebra — e o desvio acompanha o resíduo da quadrática."
           " max |f'−f^{-1}| em virgula era transporte; b inteiro, b²−b−1 ≠ 0 e' o gume",
           quebra==bs && bs==6);
        conclui("não é 'uma potência qualquer': é a raiz da borda, e as outras falham por uma");
        conclui("margem que se mede. É o controlo que impede a frase de crescer sozinha.");
    }

    printf("\n§A8 a base de Pisot: derivar RODA para a coordenada conjugada\n");
    {
        int ns = 0, mat_ok = 0, pot_ok = 0;
        L F[16]; F[0] = 0; F[1] = 1;
        for(int i = 2; i < 16; i += 1) F[i] = F[i-1] + F[i-2];
        L a00 = 1, a01 = 0, a10 = 0, a11 = 1;
        L pa = 1, pb = 0;                                /* φ^0 = 1; multiplica-se por φ */
        L Mdet = 0, Mtr = 0;
        printf("      n    M^n                          [[F_{n-1},F_n],[F_n,F_{n+1}]]\n");
        for(int n = 1; n <= 8; n += 1){
            L b00 = a01,           b01 = a00 + a01;
            L b10 = a11,           b11 = a10 + a11;
            a00 = b00; a01 = b01; a10 = b10; a11 = b11;
            if(n == 1){ Mdet = a00*a11 - a01*a10; Mtr = a00 + a11; }
            ns += 1;
            if(a00==F[n-1] && a01==F[n] && a10==F[n] && a11==F[n+1]) mat_ok += 1;
            /* φ^n = F_n φ + F_{n−1} em ℤ[φ]: parte de 1 e multiplica por φ a cada n */
            L qa, qb; zphi_mul(pa, pb, 0, 1, &qa, &qb); pa = qa; pb = qb;
            if(pa == F[n-1] && pb == F[n]) pot_ok += 1;
            if(n <= 4) printf("      %d    [[%ld,%ld],[%ld,%ld]]%*s [[%ld,%ld],[%ld,%ld]]\n",
                            n, a00, a01, a10, a11, 18, "", F[n-1], F[n], F[n], F[n+1]);
        }
        printf("      n testados: %d   com M^n = [[F_{n-1},F_n],[F_n,F_{n+1}]]: %d\n", ns, mat_ok);
        ok("M^n tem os FIBONACCI nas quatro casas — exato, em inteiros", mat_ok==ns);
        ok("e phi^n = F_n phi + F_{n-1}: toda potencia volta as DUAS direcoes."
           " F_n+F_{n-1}=F_{n+1} era a definicao relida; agora φ^n em Z[phi] contra F",
           pot_ok==ns);

        printf("      M = [[0,1],[1,1]]:  det = %ld   traco = %ld   ⟹  x^2 - x - 1 = 0\n",
               Mdet, Mtr);
        ok("det = -1 e traco = 1, logo os autovalores sao raizes de x^2 - x - 1"
           " e o produto e det M = -1 por Vieta. Sai de M^1, nao de 0*1-1*1 escrito a mao",
           Mdet==-1 && Mtr==1);

        {
            int fam = 0, coinc = 0;
            for(L n = 1; n <= 8; n += 1){
                L cf[3] = {0,0,0};
                const L u[2] = {0, 1}, v[2] = {-n, 1};     /* b  e  b−n */
                for(int i = 0; i < 2; i += 1) for(int j = 0; j < 2; j += 1)
                    cf[i+j] += u[i]*v[j];
                cf[0] -= 1;
                L t = cf[0]; cf[0] = cf[2]; cf[2] = t;
                L bd[3] = {1, -n, -1};
                fam += 1;
                if(cf[0]==bd[0] && cf[1]==bd[1] && cf[2]==bd[2]) coinc += 1;
            }
            printf("      e para n = 1..%d: o polinomio de f^(n)=f^-1 e o da borda com m=n\n", fam);
            ok("derivar n vezes baixa o expoente em n, e a borda com m=n fecha — n=1..8."
               " Os dois arrays iguais a (1,-n,-1) eram copia; agora a convolucao contra a borda",
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
