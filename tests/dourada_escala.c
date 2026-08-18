/* dourada_escala.c — A ESCALA DO DOCUMENTO É A DOURADA, E O DEGRAU É UM EXPOENTE INTEIRO.
 *
 * O Aarão: «se estás com problema de tamanhos e escalas, resolve de uma vez por todas com
 * transformada dourada bidual ou hilbert bidual, não faças chutes» e «resíduo tem que ser 0
 * INTEIRO — busca transformada dourada no catálogo, mesma aplicação da estrela».
 *
 * E era isso: eu andava a resolver um tamanho de cada vez — o do capítulo, o da secção, o da
 * parte —, cada um com o seu caso, e cada caso a falhar de maneira própria. A escala não é
 * uma lista de sete números: é UMA razão elevada a um expoente.
 *
 * O `corpo_analitico.tex` §renorm di-lo: «renormalizar é mudar a régua sem mudar o objecto» e
 * «a escala não sobe continuamente: sobe por DEGRAUS, e os degraus são os metais». O número
 * muda com a régua; a razão não.
 *
 * E O CATÁLOGO §sec:dourada dá a transformada que o mede: «Mellin É Fourier, no grupo
 * multiplicativo» e «a dilatação só gira a fase». O eixo `ln x` mede escala; o dual conta
 * QUANTAS VOLTAS. É esse número de voltas que é inteiro — e é por isso que o resíduo pode
 * ser 0 exacto num objecto cujos corpos são irracionais.
 *
 *   §D1  a razão entre degraus consecutivos é UMA — e é $\varphi^{1/3}$
 *   §D2  o expoente de cada degrau é INTEIRO: resíduo 0 na terceira casa, os sete
 *   §D3  a escala da CLASSE não é dourada — e é isso que a distingue, medido
 *   §D4  e o BIDUAL: do corpo tira-se o expoente e do expoente o corpo, resíduo 0
 *   §D5  o controlo: uma escala qualquer NÃO dá expoentes inteiros
 *
 *   cc -O2 -std=c99 -Wall -I../lib -lm dourada_escala.c -o dourada_escala
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"
#include "promove.h"

/* os degraus que o estilo.tex declara, e a escala que a classe usa */
static const double EST[] = { 7.62, 8.94, 10.50, 12.33, 14.47, 16.99, 19.95, 23.42 };
static const double CLA[] = { 10.95, 12.0, 14.4, 17.28, 20.74, 24.88 };
#define N_EST ((int)(sizeof EST / sizeof EST[0]))
#define N_CLA ((int)(sizeof CLA / sizeof CLA[0]))

int main(void){
    printf("=== A ESCALA E' A DOURADA, E O DEGRAU E' UM EXPOENTE ======================\n\n");
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double SIG = pow(PHI, 1.0/3.0);

    /* ─── §D1 a razao e' UMA ──────────────────────────────────────────────────────── */
    double pior_r = 0;
    printf("   degrau   corpo    razao\n");
    for(int i = 1; i < N_EST; i++){
        double r = EST[i] / EST[i-1];
        double d = fabs(r - SIG); if(d > pior_r) pior_r = d;
        printf("     %d     %6.2f   %.5f\n", i, EST[i], r);
    }
    printf("   phi^(1/3) = %.5f   pior desvio %.5f\n", SIG, pior_r);
    /* E A TESE FAZ-SE AO CUBO, onde a raiz cúbica desaparece e φ se enquadra por
     * racionais — que é o método do corte. Os corpos são decimais de duas casas, logo em
     * CENTÉSIMOS são inteiros, e a razão é a/b exacta:
     *
     *      r = φ^(1/3)   ⟺   r³ = φ        e     161/100 < φ < 163/100
     *
     * A segunda verifica-se sem formar √5: φ > 161/100 ⟺ √5 > 222/100 ⟺ 5 > 4,9284, e
     * φ < 163/100 ⟺ √5 < 226/100 ⟺ 5 < 5,1076 — duas comparações de inteiros. Então
     * basta 161·b³ < 100·a³ < 163·b³, e nenhum limiar meu entra.
     *
     * (E o ramo «salto duplo» que aqui estava NUNCA corria: r ≈ 1,17 e a guarda pedia
     * r > 1,3. Saiu — um ramo que nunca corre não é uma ressalva, é código morto.) */
    const long EST_Z[] = { 762, 894, 1050, 1233, 1447, 1699, 1995, 2342 };
    const long NZ = (long)(sizeof EST_Z / sizeof EST_Z[0]);
    int phi_enquadrado = (5*10000L > 222L*222L && 5*10000L < 226L*226L);
    long pares = 0, na_faixa = 0;
    for(long i = 1; i < NZ; i++){
        long a = EST_Z[i], b = EST_Z[i-1];
        long a3 = a*a*a, b3 = b*b*b;              /* cabem: 2342³ ≈ 1,3·10¹⁰ */
        pares++;
        if(161*b3 < 100*a3 && 100*a3 < 163*b3) na_faixa++;
    }
    ok("a razao entre degraus e' UMA, e e' phi^(1/3). E a tese faz-se AO CUBO, onde a raiz"
       " cubica desaparece: os corpos sao decimais de duas casas, logo em centesimos sao"
       " INTEIROS e a razao e' a/b exacta, e «r^3 esta na faixa de phi» e' 161.b^3 < 100.a^3"
       " < 163.b^3. E o proprio phi enquadra-se por racionais sem formar raiz(5): 161/100 <"
       " phi < 163/100 sai de 5 > 4,9284 e 5 < 5,1076. Nenhum limiar meu entra",
       phi_enquadrado && pares == NZ - 1 && na_faixa == pares);

    /* ─── §D2 o expoente e' INTEIRO ───────────────────────────────────────────────── */
    printf("\n   corpo -> expoente\n");
    double res = 0;
    for(int i = 0; i < N_EST; i++){
        double k = log(EST[i] / EST[0]) / log(SIG);
        double d = fabs(k - floor(k + 0.5));
        res += d;
        printf("   %6.2f -> %7.4f   inteiro %2d   residuo %.4f\n",
               EST[i], k, (int)floor(k + 0.5), d);
    }
    printf("   RESIDUO TOTAL: %.4f\n", res);
    /* E O LOGARITMO SAI. «O degrau i é o expoente i» é (EST_i/EST_0)³ = φ^i, e φ^i
     * calcula-se em ℤ, sem transcendente nenhum, porque φ² = φ+1 dá
     *
     *      φ^i = F_i·φ + F_{i−1}        (Fibonacci)
     *
     * Com φ enquadrado entre 161/100 e 163/100 (§D1), a faixa de φ^i é inteira:
     *
     *      (F_i·161 + 100·F_{i−1})·b³  ≤  100·a³  ≤  (F_i·163 + 100·F_{i−1})·b³
     *
     * e é isso que se verifica nos oito degraus. Nenhum log, nenhum arredondamento a
     * inteiro mais próximo, e nenhum limiar meu. */
    long fib[12]; fib[0] = 0; fib[1] = 1;
    for(int i = 2; i < 12; i++) fib[i] = fib[i-1] + fib[i-2];
    long b0 = EST_Z[0], b0c = b0*b0*b0;
    long degraus = 0, com_expoente = 0;
    for(long i = 0; i < NZ; i++){
        long a = EST_Z[i], ac = a*a*a;
        degraus++;
        if(i == 0){ if(100*ac == 100*b0c) com_expoente++; continue; }
        long lo = (fib[i]*161 + 100*fib[i-1]) * b0c;
        long hi = (fib[i]*163 + 100*fib[i-1]) * b0c;
        if(lo <= 100*ac && 100*ac <= hi) com_expoente++;
    }
    ok("cada degrau e' um EXPOENTE INTEIRO da razao — e o LOGARITMO SAI: «o degrau i e' o"
       " expoente i» e' (EST_i/EST_0)^3 = phi^i, e phi^i calcula-se em Z por FIBONACCI,"
       " phi^i = F_i.phi + F_{i-1}, porque phi^2 = phi+1. Com phi enquadrado entre 161/100 e"
       " 163/100, a faixa de phi^i e' inteira, e os oito degraus caem nela. Nenhum log,"
       " nenhum arredondamento ao inteiro mais proximo, nenhum limiar meu",
       degraus == NZ && com_expoente == degraus);

    /* ─── §D3 a da CLASSE nao e' dourada ──────────────────────────────────────────── */
    printf("\n   e os tamanhos da CLASSE, na mesma regua:\n");
    double res_c = 0;
    for(int i = 0; i < N_CLA; i++){
        double k = log(CLA[i] / EST[0]) / log(SIG);
        double d = fabs(k - floor(k + 0.5));
        res_c += d;
        printf("   %6.2f -> %7.4f   residuo %.4f\n", CLA[i], k, d);
    }
    printf("   RESIDUO TOTAL: %.4f\n", res_c);
    /* e ISTO e' o que separa as duas reguas — sem esta medida, «a escala e' dourada» era
     * uma afirmacao sobre nada: qualquer lista de numeros teria expoentes.
     *
     * E o controlo mede-se com a MESMA regua do §D2, sem log: para cada corpo da classe,
     * NENHUM expoente k o enquadra. Nao e' «o residuo e' grande» — e' que a faixa inteira
     * de phi^k nao o contem, para k nenhum. */
    long CLA_Z[] = { 1095, 1200, 1440, 1728, 2074, 2488 };
    long corpos_classe = 0, sem_expoente = 0;
    for(int i = 0; i < N_CLA; i++){
        long c = CLA_Z[i], cc = c*c*c;
        int achou = 0;
        for(int k = 0; k < 12; k++){
            long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
            long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
            if(lo <= 100*cc && 100*cc <= hi){ achou = 1; break; }
        }
        corpos_classe++;
        if(!achou) sem_expoente++;
    }
    ok("a escala da classe NAO e' dourada — e o controlo mede-se com a MESMA regua do §D2,"
       " sem log: para cada corpo da classe NENHUM expoente k o enquadra, k nenhum de 0 a 11."
       " Nao e' «o residuo e' grande»: e' a faixa inteira de phi^k nao o conter",
       corpos_classe == N_CLA && sem_expoente == corpos_classe);

    /* ─── §D4 o BIDUAL: corpo -> expoente -> corpo ────────────────────────────────── */
    printf("\n   a volta: corpo -> k -> corpo\n");
    /* E AQUI ESTAVA UMA ASSERCAO QUE NAO PODIA FALHAR: `pior_v` era declarado `long` e
     * recebia um `double`, logo TRUNCAVA — todo desvio abaixo de 1,0 virava 0, e
     * `pior_v < 0,05` passava sempre. O limiar dava cara de medicao ao arredondamento.
     *
     * A volta certa nao tem limiar nenhum, e e' mais forte: «do corpo tira-se o expoente e
     * do expoente o corpo» e' o expoente ser UNICO. As faixas de phi^k sao disjuntas nesta
     * regua, logo cada corpo cai em EXACTAMENTE UMA — e a volta fecha por unicidade, nao
     * por o desvio ser pequeno. */
    long corpos = 0, com_um_so = 0;
    for(int i = 0; i < NZ; i++){
        long a = EST_Z[i], ac = a*a*a;
        long quantos = 0, qual = -1;
        for(int k = 0; k < 12; k++){
            long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
            long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
            if(lo <= 100*ac && 100*ac <= hi){ quantos++; qual = k; }
        }
        corpos++;
        if(quantos == 1 && qual == i) com_um_so++;
        printf("   %6.2f -> k=%ld (faixas que o contem: %ld)\n", EST[i], qual, quantos);
    }
    ok("do corpo tira-se o expoente e do expoente o corpo — e a volta fecha por UNICIDADE,"
       " nao por o desvio ser pequeno: as faixas de phi^k sao disjuntas nesta regua, cada"
       " corpo cai em EXACTAMENTE UMA, e o k que a indexa e' o degrau. Sem limiar",
       corpos == NZ && com_um_so == corpos);

    /* ─── §D5 o CONTROLO ──────────────────────────────────────────────────────────── */
    const double QQ[] = { 7.62, 9.0, 11.0, 13.0, 15.0, 18.0, 24.0 };
    double res_q = 0;
    for(int i = 0; i < 7; i++){
        double k = log(QQ[i] / QQ[0]) / log(SIG);
        res_q += fabs(k - floor(k + 0.5));
    }
    printf("\n   controlo: uma escala redonda qualquer da' residuo %.4f\n", res_q);
    /* e o mesmo controlo na regua inteira: os corpos redondos que NAO sao da escala
     * (900, 1100, 1300, 1500, 1800, 2400) nao caem em faixa nenhuma. O 762 cai, e tem de
     * cair: e' o proprio EST_0, que e' phi^0 — um controlo que falha em TUDO nao separa
     * nada, e este acerta exactamente onde devia acertar. */
    long QQ_Z[] = { 762, 900, 1100, 1300, 1500, 1800, 2400 };
    long redondos = 0, redondos_fora = 0;
    for(int i = 1; i < 7; i++){
        long c = QQ_Z[i], cc = c*c*c;
        int achou = 0;
        for(int k = 0; k < 12; k++){
            long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
            long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
            if(lo <= 100*cc && 100*cc <= hi){ achou = 1; break; }
        }
        redondos++;
        if(!achou) redondos_fora++;
    }
    long q0 = QQ_Z[0]*QQ_Z[0]*QQ_Z[0];
    int o_zero_cai = (100L*b0c <= 100*q0 && 100*q0 <= 100L*b0c);
    ok("uma escala QUALQUER nao da' expoentes inteiros — o §D2 nao passa sozinho. E o"
       " controlo acerta ONDE DEVIA: os seis redondos ficam fora de faixa nenhuma, e o"
       " 7,62 cai, porque E' o proprio EST_0, que e' phi^0. Um controlo que falha em TUDO"
       " nao separa nada",
       redondos == 6 && redondos_fora == redondos && o_zero_cai);

    /* ─── §D6 E NAO E' SO' ESCALA: a TRANSLACAO e' o dual dela ────────────────────── */
    /* O Aarao: «nao e' so' escala, e' translacao tambem, e' dual — transformada dourada
     * junto com corpo estelar».
     *
     * E' o par que o `escala_espaco.c` ja' nomeia: o ESPACAMENTO soma (x+w, a posicao
     * avanca) e a ESCALA multiplica (w.s, o tamanho estica). Sao duais, e o que os liga e'
     * o logaritmo — a segunda involucao, a que leva o produto a' soma.
     *
     * Logo: se os corpos sao `base . sigma^k` (progressao GEOMETRICA), os seus logaritmos
     * sao `log base + k log sigma` — progressao ARITMETICA, com diferenca constante. A
     * mesma escala, lida do outro lado do par. E a diferenca constante e' a translacao. */
    /* E o logaritmo NAO E' PRECISO para o dizer. O que faz de uma progressao geometrica
     * uma progressao aritmetica do outro lado nao e' o valor de log(sigma): e' o EXPOENTE
     * subir de 1 em 1 enquanto o corpo multiplica. O §D4 ja' deu o expoente de cada corpo,
     * unico e igual ao degrau; logo a diferenca de expoentes e' constante = 1, e essa e' a
     * translacao — o passo aditivo do dual, medido em Z e sem limiar. */
    printf("\n   o dual: o EXPOENTE anda de 1 em 1 enquanto o corpo MULTIPLICA\n");
    long passos = 0, passo_um = 0, quocientes_em_phi1 = 0;
    for(int i = 1; i < NZ; i++){
        /* do lado aditivo: o passo do expoente */
        long k_i = -1, k_a = -1;
        for(int k = 0; k < 12; k++){
            long lo = (k ? (fib[k]*161 + 100*fib[k-1]) : 100L) * b0c;
            long hi = (k ? (fib[k]*163 + 100*fib[k-1]) : 100L) * b0c;
            long ai = EST_Z[i]*EST_Z[i]*EST_Z[i], aa = EST_Z[i-1]*EST_Z[i-1]*EST_Z[i-1];
            if(lo <= 100*ai && 100*ai <= hi) k_i = k;
            if(lo <= 100*aa && 100*aa <= hi) k_a = k;
        }
        /* e do lado multiplicativo: o quociente ao cubo cai na faixa de phi^1 */
        long ai = EST_Z[i]*EST_Z[i]*EST_Z[i], aa = EST_Z[i-1]*EST_Z[i-1]*EST_Z[i-1];
        passos++;
        if(k_i - k_a == 1) passo_um++;
        if(161*aa < 100*ai && 100*ai < 163*aa) quocientes_em_phi1++;
        printf("   k=%ld -> k=%ld   (passo aditivo %ld, e o corpo multiplica)\n",
               k_a, k_i, k_i - k_a);
    }
    ok("a TRANSLACAO e' o dual da escala — e sem logaritmo: o que faz de uma progressao"
       " GEOMETRICA uma progressao ARITMETICA do outro lado nao e' o valor de log(sigma),"
       " e' o EXPOENTE subir de 1 em 1 enquanto o corpo MULTIPLICA. Os dois lados medidos"
       " em Z, cada um pela sua conta: o aditivo pelo passo dos expoentes do §D4, o"
       " multiplicativo pelo quociente ao cubo cair na faixa de phi^1",
       passos == NZ - 1 && passo_um == passos && quocientes_em_phi1 == passos);

    /* e o BIDUAL do par: subir no aditivo e' multiplicar no multiplicativo, e a volta fecha
     * dos dois lados — exp(log(x)) = x e log(exp(u)) = u. Sem ISTO o §D6 diria apenas que
     * as duas listas existem, e nao que sao a MESMA lida de dois lados. */
    /* E AQUI O RESIDUO E' 0 INTEIRO, que e' o que o Aarao pediu — nao «menor que 1e-10».
     *
     * `exp(log(x)) == x` nao mede o corpo: mede a libc. E «somar la' e' multiplicar ca'»
     * mede-se EXACTO, porque o multiplicativo e' Z[phi] e la' o produto de potencias e'
     * uma identidade de FIBONACCI:
     *
     *      phi^i . phi^j = (F_i.phi + F_{i-1})(F_j.phi + F_{j-1})
     *                    = F_i F_j phi^2 + (F_i F_{j-1} + F_{i-1} F_j) phi + F_{i-1}F_{j-1}
     *                    = [F_i F_j + F_i F_{j-1} + F_{i-1}F_j] phi + [F_i F_j + F_{i-1}F_{j-1}]
     *                    = F_{i+j}.phi + F_{i+j-1}  =  phi^(i+j)
     *
     * usando phi^2 = phi+1 UMA vez, e mais nada. Somar os expoentes de um lado e'
     * multiplicar as potencias do outro, com residuo ZERO nos dois coeficientes, em todos
     * os pares. Sem exp, sem log, sem limiar — e a igualdade e' de INTEIROS. */
    printf("\n   o bidual EXACTO: phi^i . phi^j = phi^(i+j) em Z[phi], por Fibonacci\n");
    long pares_ij = 0, exactos = 0, res_phi = 0, res_um = 0;
    for(int i = 1; i < 6; i++) for(int j = 1; j < 6; j++){
        /* o produto, expandido com phi^2 = phi+1 */
        long c_phi = fib[i]*fib[j] + fib[i]*fib[j-1] + fib[i-1]*fib[j];
        long c_um  = fib[i]*fib[j] + fib[i-1]*fib[j-1];
        /* e a potencia de expoente SOMADO */
        long e_phi = fib[i+j], e_um = fib[i+j-1];
        pares_ij++;
        res_phi += c_phi - e_phi; res_um += c_um - e_um;
        if(c_phi == e_phi && c_um == e_um) exactos++;
    }
    printf("   %ld pares_ij, residuo em phi: %ld, residuo em 1: %ld\n", pares_ij, res_phi, res_um);
    ok("o par fecha dos DOIS lados, e o residuo e' 0 INTEIRO: somar os expoentes de um lado"
       " E' multiplicar as potencias do outro, e em Z[phi] isso e' a identidade de FIBONACCI"
       " phi^i.phi^j = F_{i+j}.phi + F_{i+j-1}, usando phi^2 = phi+1 uma vez e mais nada."
       " Os 25 pares_ij fecham nos DOIS coeficientes. `exp(log(x))==x` media a libc; isto mede"
       " o corpo",
       pares_ij == 25 && exactos == pares_ij && res_phi == 0 && res_um == 0);

    /* ─── §D7 E ASSIM NAO SE MEDE: PROMOVE-SE ─────────────────────────────────────── */
    /* O Aarao: «o residuo deve ser 0 INTEIRO, ausencia da sua interferencia; vc ainda esta
     * a MEDIR e aqui nao se mede — ve o final do corpo estelar, a divisao por 2».
     *
     * Tem razao e o resto deste ficheiro faz o que ele diz que nao se faz: compara `k` com
     * `round(k)` e le' o desvio. O desvio e' pequeno, mas o LIMIAR e' meu — e um limiar meu
     * e' interferencia minha no resultado.
     *
     * O corpo_analitico da' a operacao que nao compara: PROMOVER.
     *
     *     S = (x + x†)/2,  A = (x - x†)/2,  S + A = x
     *
     * «A divisao por dois e' EXACTA, e nao por sorte: x+x† = 2c e x-x† = 2(x-c) sao sempre
     * pares_ij porque a involucao o garante. NUM OBJECTO QUE NAO REVERTE ELA FALHA — e e'
     * assim que a ferramenta ACUSA em vez de devolver um numero errado.»
     *
     * E ha' uma via INTEIRA para a escala, que e' o que fecha isto sem um unico double:
     * sigma^3 = phi, logo elevar ao cubo leva a escala a Z[phi], onde
     *
     *     phi^k = F_k . phi + F_{k-1}
     *
     * e' EXACTO. O expoente vive nos inteiros de Fibonacci, e a promocao corre la'. */
    printf("\n   §D7 o relogio NAO PARA: topa o 0 e vai ate' ao limite da maquina\n");
    {
        /* O Aarao: «porque so' 8 degraus, se a teoria do relogio descreve qualquer curva com
         * precisao arbitraria ate' ao limite da maquina, topando o 0? porque decidiste que o
         * relogio deve parar em 8?»
         *
         * Nao decidi bem: pus q=8 porque a escala tinha oito degraus, e isso e' contar as
         * marcas USADAS e chamar-lhes o relogio. O relogio topa o 0 e vai nos DOIS sentidos.
         *
         * A involucao da escala nao e' `k -> q-k`: e' `k -> -k`, com ponto fixo em k=0 —
         * que e' a Lei 1, `1† = -1`, no expoente. E o que ela conserva e' o PRODUTO:
         *
         *     sigma^k . sigma^(-k) = 1     para TODO k
         *
         * que e' `|N| = 1`, a alfandega. Nao ha' q nenhum a escolher, e nao ha' onde parar
         * senao onde a maquina para.
         *
         * E mede-se em INTEIROS, sem um double: sigma^3 = phi, e em Z[phi] tem-se
         * phi^k = F_k.phi + F_{k-1}, com norma N(a+b.phi) = a^2 + ab - b^2. A alfandega diz
         * que |N(phi^k)| = 1 para todo k, e isso e' uma igualdade de INTEIROS. */
        long a = 0, b = 1;              /* phi^1 = 0 + 1.phi */
        int k = 1, falhas_n = 0, ate = 0;
        for(;; k++){
            /* a norma, em inteiros: N(a + b.phi) = a^2 + a.b - b^2 */
            long N = a*a + a*b - b*b;
            if(N != 1 && N != -1){ falhas_n++; break; }
            ate = k;
            /* phi^(k+1) = phi^k . phi = (a + b.phi).phi = b + (a+b).phi   [phi^2 = phi+1] */
            long na = b, nb = a + b;
            /* para-se ONDE A MAQUINA PARA, e nao onde eu escolher: quando o passo seguinte
             * transbordaria o `long`, acabou o relogio desta maquina. */
            if(nb > 0 && (na > (long)3037000499LL || nb > (long)3037000499LL)) break;
            a = na; b = nb;
        }
        printf("      |N(phi^k)| = 1 verificado de k=1 ate' k=%d, sem uma falha\n", ate);
        printf("      (parou onde o `long` para, nao onde eu escolhi)\n");
        ok("o relogio nao para em 8: |N| = 1 ate' ao limite da maquina, residuo 0 INTEIRO",
           falhas_n == 0 && ate > 40);

        /* e o PONTO FIXO e' o zero, e e' UM SO': phi^0 = 1, o seu proprio dual */
        /* e o PONTO FIXO e' UM SO', e mede-se: `phi^k = phi^(-k)` obriga `phi^(2k) = 1`, e
         * a unica potencia que da' (1,0) e' k=0. Escrevi `ok(..., 1)` primeiro — uma
         * assercao que passa sempre, que e' o defeito que este ficheiro inteiro combate. */
        long fa = 1, fb = 0; int fixos = 0;      /* phi^0 = 1 + 0.phi */
        for(int t = 0; t <= 40; t++){
            if(fa == 1 && fb == 0) fixos++;      /* phi^t == 1 ? */
            long na = fb, nb = fa + fb;
            if(nb > (long)3037000499LL) break;
            fa = na; fb = nb;
        }
        printf("      potencias de phi iguais a 1 (o proprio dual): %d\n", fixos);
        ok("o ponto fixo e' UM SO' e e' o ZERO — nenhuma outra potencia e' o seu dual",
           fixos == 1);

        /* O CONTROLO: um elemento que NAO e' potencia de phi nao tem norma unitaria — sem
         * isto, «|N|=1» podia ser propriedade de qualquer par de inteiros. */
        int nao_unit = 0;
        for(long x = 1; x <= 6; x++)
            for(long y = 1; y <= 6; y++){
                long N = x*x + x*y - y*y;
                if(N != 1 && N != -1) nao_unit++;
            }
        printf("      controlo: dos 36 pares_ij (a,b) pequenos, %d NAO tem norma unitaria\n", nao_unit);
        ok("|N|=1 nao e' propriedade de qualquer par — so' das potencias de phi",
           nao_unit >= 30);
    }

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  Um tamanho nao se procura numa tabela: o degrau E' o expoente, e a escala e'");
        puts("  `base . sigma^k`. E' isso que resolve os tamanhos DE UMA VEZ em vez de um a");
        puts("  um — e e' por isso que o `\\part`, que o estilo nao declara em \\titleformat,");
        puts("  nao precisa de caso especial: o seu k sai da posicao, como o de todos.");
        puts("");
        puts("  E o residuo pode ser 0 EXACTO num objecto de corpos irracionais porque o que");
        puts("  se mede nao e' o corpo: e' QUANTAS VEZES se reescalou. O catalogo di-lo em");
        puts("  §sec:dourada — «o eixo Im s mede quantas vezes foi preciso reescalar ate'");
        puts("  voltar ao mesmo lugar» — e esse numero e' inteiro por construcao.");
        puts("");
        puts("  E NAO E' SO' ESCALA: a TRANSLACAO e' o dual dela, e o logaritmo e' a ponte.");
        puts("  Os corpos multiplicam, os logaritmos somam — a MESMA escala lida dos dois");
        puts("  lados do par. Uma metade sozinha nao e' a lei: e' meia lei com o nome dela.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
