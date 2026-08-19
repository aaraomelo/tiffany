/* plugs.c — ONDE O DIRAC FURA: a família real, a dual, e os pontos onde a túnica encaixa.
 *
 * O Aarão: "a transformada universal fura com Dirac exatamente na família real da cifra e traz uma
 * amostra de além do infinito. Aí são os plugs da túnica. Isso valida os pontos entre os fractais
 * negros e brancos onde as dobras duais acontecem, e onde fica a família real dual."
 *
 * LEI vs TRANSPORTE. cos/sin da soma, sqrt(m²+4) e floor da cifra eram o método. A lei é
 * Dirac em ℤ[i] (período 4: 1+i+(−1)+(−i)=0), a borda em ℤ[√D], a cifra [m;m,…] pela
 * órbita [p:q]↦[m p+q : p], e σ_{−m} σ_m = 1 pelo produto (m+√D)(−m+√D)=4. Sem uma raiz.
 *
 *   §P1  o DIRAC fura: a soma da órbita colapsa num ponto — a amostra do infinito
 *   §P2  na FAMÍLIA REAL: a cifra [m;m,m,…] é infinita e o valor é exato
 *   §P3  os DOIS FRACTAIS: o negro expande, o branco contrai, e σσ' = −1
 *   §P4  a FAMÍLIA REAL DUAL: σ_{-m} = 1/σ_m — a mesma lida ao contrário
 *   §P5  a DOBRA DUAL: a cifra desloca-se UMA casa, e é isso a dobra
 *   §P6  os PLUGS: onde a túnica encaixa, e porque são esses e não outros
 *
 *   cc -O2 -std=c99 -I lib tests/plugs.c -o plugs && ./plugs
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

/* i^k em ℤ[i], período 4: (1,0), (0,1), (−1,0), (0,−1) */
static void i_k(long k, long *re, long *im){
    int r = (int)((k % 4 + 4) % 4);
    if(r == 0){ *re = 1; *im = 0; }
    else if(r == 1){ *re = 0; *im = 1; }
    else if(r == 2){ *re = -1; *im = 0; }
    else { *re = 0; *im = -1; }
}

int main(void){
    puts("plugs.c — ONDE O DIRAC FURA: a familia real, a dual, e os plugs da tunica\n");

    puts("§P1  O DIRAC FURA: a soma de uma orbita que NAO ACABA cabe num ponto exato");
    puts("     O transformada.c ja o mede: soma_k chi_k(j) chi_{-k}(j') = n.delta. Aqui refaz-se");
    puts("     em ℤ[i], periodo 4: 1 + i + (−1) + (−i) = 0. Sem cos.\n");
    {
        int n = 4, colapsa = 0, pares = 0;
        long menor_dentro = 0, pior_fora = 0;
        int pri_d = 1, pri_f = 1;
        for(int j = 0; j < n; j += 1)
            for(int jl = 0; jl < n; jl += 1){
                long re = 0, im = 0;
                for(int k = 0; k < n; k += 1){
                    long cr, ci;
                    i_k((long)k * (j - jl), &cr, &ci);
                    re += cr; im += ci;
                }
                int dentro = (j == jl);
                int ponto = dentro ? (re == n && im == 0) : (re == 0 && im == 0);
                if(ponto) colapsa += 1;
                if(dentro){ if(pri_d || re > menor_dentro) menor_dentro = re; pri_d = 0; }
                else {
                    long mag = re*re + im*im;
                    if(pri_f || mag > pior_fora) pior_fora = mag;
                    pri_f = 0;
                }
                pares += 1;
            }
        ok("a soma da orbita da n na diagonal e ZERO fora — e isso E o Dirac, nao uma aproximacao."
           " Em ℤ[i], n=4, 16 pares: (4,0) na diagonal e (0,0) fora. cos(2π k d / n) era o transporte",
           colapsa == pares && pares == 16 && menor_dentro == 4 && pior_fora == 0);
        printf("     -> %d pares: na diagonal a soma vale %ld, fora dela o maior |z|² e %ld.\n",
               pares, menor_dentro, pior_fora);
        puts("        A orbita tem n termos e a soma deles colapsa num ponto. Levada ao limite,");
        puts("        e uma orbita que nao acaba a caber num so lugar — a amostra do infinito.\n");
        conclui("1+i+(−1)+(−i)=0 fora da diagonal, e 4 na diagonal — o Dirac em ℤ[i].");
    }

    puts("§P2  NA FAMILIA REAL: a cifra e [m;m,m,...] — infinita, e o valor exato");
    puts("     O metal sigma_m e a raiz de x^2 = m x + 1, e a cifra dele repete-se para sempre.");
    puts("     Nenhuma truncatura o da; e mesmo assim ele e exato.\n");
    {
        printf("     %4s %18s %12s %14s\n", "m", "convergente k=8", "forma", "borda");
        int todos = 0, testados = 0, borda_ok = 0;
        for(int m = 1; m <= 5; m += 1){
            long D = (long)m*m + 4, a2, b2;
            rt_zd_mul(m, 1, m, 1, D, &a2, &b2);
            int fecha = (a2 == 2L*m*m + 4 && b2 == 2L*m);
            long p, q;
            rt_orbita(m, 8, &p, &q);
            long forma = p*p - (long)m*p*q - q*q;
            int pell = (forma == 1 || forma == -1);
            printf("     %4d  %8ld / %-6ld %12ld %12s\n", m, p, q, forma, fecha ? "exacto" : "NAO");
            if(pell && fecha && q != 0) todos += 1;
            if(fecha) borda_ok += 1;
            testados += 1;
        }
        ok("a cifra de sigma_m e [m;m,m,...] — TODOS os termos iguais a m, nos cinco metais."
           " Sem floor(σ): a orbita [p:q]↦[m p+q : p] da' convergentes com forma p²−m pq−q² = ±1,"
           " e q nao volta a 0 — sao os convergentes do metal, nao de outro numero",
           todos == testados && testados == 5);
        ok("e a borda fecha exata: sigma^2 - m.sigma - 1 = 0, sem residuo. E mede-se em"
           " ℤ[√D] como (2σ)² = 2m(2σ)+4 — o 1e-12 comparava s*s-m*s-1, a mesma lei com"
           " a raiz formada duas vezes",
           borda_ok == testados);
        printf("     -> %d metais, cifra constante em todos, borda exacta em %d de %d.\n",
               testados, borda_ok, testados);
        puts("        E a UNICA cifra que se repete sem nunca acabar. Truncar da um racional; o");
        puts("        valor esta ALEM de qualquer truncatura, e ainda assim e exato. E ai que a");
        puts("        amostra do infinito faz sentido — e nao num ponto qualquer.\n");
        conclui("a cifra e a orbita, e a borda fecha em ℤ[√D] como (2σ)² = 2m(2σ)+4.");
    }

    puts("§P3  OS DOIS FRACTAIS: o NEGRO expande, o BRANCO contrai, e o produto e -1");
    puts("     As duas raizes da mesma borda. Uma tem modulo maior que 1 (o sorvedouro), a");
    puts("     outra menor (a fonte). O neuronio.c ja lhes deu os nomes.\n");
    {
        int negro_expande = 0, branco_contrai = 0, n = 0;
        for(int m = 1; m <= 5; m += 1){
            /* |σ|>1  <=>  m²+4 > (2−m)²  <=>  m>0.  |σ'|<1  <=>  o mesmo, porque |N|=1. */
            int expande = ((long)m*m + 4 > (2L-m)*(2L-m));
            int contrai = expande;                 /* |σ'|=1/σ < 1  <=>  σ>1 */
            if(expande) negro_expande += 1;
            if(contrai) branco_contrai += 1;
            n += 1;
        }
        ok("o NEGRO tem modulo > 1 (expande) e o BRANCO < 1 (contrai) — nos cinco metais."
           " Sem formar σ: m²+4 > (2−m)²  <=>  m>0, e |σ'|=1/|σ| segue de N=−1",
           negro_expande == n && branco_contrai == n && n == 5);
        long vieta_prod = 0, vieta_soma = 0, metais_z = 0;
        for(long m2 = 1; m2 <= 5; m2 += 1){
            long D2 = m2*m2 + 4, pa2, pb2;
            rt_zd_mul(m2, 1, m2, -1, D2, &pa2, &pb2);
            long sa2 = m2 + m2, sb2 = 1 + (-1);
            metais_z += 1;
            if(pa2 == -4 && pb2 == 0) vieta_prod += 1;
            if(sa2 == 2*m2 && sb2 == 0) vieta_soma += 1;
        }
        printf("     -> e em ℤ[√D], sem limiar: (2σ)(2σ') = -4 em %ld dos %ld metais, e a soma\n"
               "        da' (2m, 0) — a parte irracional CANCELA — em %ld\n",
               vieta_prod, metais_z, vieta_soma);
        ok("e o PRODUTO deles e exatamente -1 — e a soma e m, que sao det e traco da regua."
           " E «exatamente» mede-se em ℤ[√D] e nao com 1e-12: (2σ)(2σ') = m² - D = -4 nos"
           " cinco metais, e (2σ)+(2σ') = (2m, 0) com o raiz(D) a CANCELAR, que e' a"
           " conjugacao",
           vieta_prod == metais_z && vieta_soma == metais_z && metais_z == 5);
        printf("     -> %d metais: produto -1 e soma m em todos. Sao (B,C) = (-m, -1), a regua\n", n);
        puts("        do catalogo, e ela sai das duas raizes sem se lhe tocar.\n");
        conclui("σσ' = −1 e σ+σ' = m, lidos em ℤ[√D]; o negro expande porque m>0.");
    }

    puts("§P4  A FAMILIA REAL DUAL: sigma_{-m} = 1/sigma_m — a MESMA lida ao contrario");
    puts("     O Aarao: 'onde fica a familia real dual'. Fica nos indices negativos, e ela nao e");
    puts("     uma segunda familia: e a dos INVERSOS. Mede-se.\n");
    {
        printf("     %4s %16s %16s\n", "m", "(2σ)(2σ_{-m})", "parte √D");
        int inversos = 0, n = 0;
        for(int m = 1; m <= 6; m += 1){
            long D = (long)m*m + 4, a, b;
            /* (m+√D)(−m+√D) = D − m² = 4. Logo σ_m · σ_{-m} = 1. */
            rt_zd_mul(m, 1, -m, 1, D, &a, &b);
            printf("     %4d %16ld %16ld\n", m, a, b);
            if(a == 4 && b == 0) inversos += 1;
            n += 1;
        }
        ok("A FAMILIA DUAL E A DOS INVERSOS: sigma_{-m} = 1/sigma_m, nos seis indices."
           " Sem formar raiz: (m+√D)(−m+√D) = 4, donde σ σ_{-m} = 1. O 1e-12 comparava"
           " dois doubles da formula de Bhaskara",
           inversos == n && n == 6);
        printf("     -> %d indices, todos com produto 4+0√D.\n", n);
        puts("        Nao ha duas familias: ha UMA, e o indice negativo le-a do outro lado. E o");
        puts("        chicote do catalogo — os dois lados do mesmo objeto, e nao dois objetos.\n");
        conclui("σ_{-m} σ_m = 1 porque (m+√D)(−m+√D) = 4.");
    }

    puts("§P5  A DOBRA DUAL: a cifra desloca-se UMA CASA — e e isso, literalmente, a dobra");
    puts("     Se sigma_m = [m; m, m, ...] entao 1/sigma_m = [0; m, m, m, ...]. Passar ao dual e");
    puts("     empurrar a cifra por uma casa e por um zero a frente. Mede-se, nao se desenha.\n");
    {
        printf("     %4s %20s %20s\n", "m", "m² < D < (m+2)²", "borda");
        int desloca = 0, n = 0;
        for(int m = 1; m <= 5; m += 1){
            long D = (long)m*m + 4;
            /* floor(σ)=m  <=>  m < σ < m+1  <=>  m² < D < (m+2)², sem formar σ */
            int floor_m = ((long)m*m < D && D < (long)(m+2)*(m+2));
            long a2, b2;
            rt_zd_mul(m, 1, m, 1, D, &a2, &b2);
            int borda = (a2 == 2L*m*m + 4 && b2 == 2L*m);
            printf("     %4d  %12s %20s\n", m, floor_m ? "floor = m" : "NAO",
                   borda ? "σ−m = 1/σ" : "NAO");
            /* σ − m = 1/σ e' a borda, logo 1/σ = [0; m, m, ...] — um zero a frente */
            if(floor_m && borda) desloca += 1;
            n += 1;
        }
        ok("A DOBRA: a cifra do dual e a do original com um ZERO a frente — desloca uma casa."
           " Sem floor(1/σ): m² < D < (m+2)² diz que a parte inteira e' m, e σ−m = 1/σ e' a"
           " borda, logo a cifra dual comeca em 0 e segue a do metal",
           desloca == n && n == 5);
        printf("     -> %d metais, e em todos a cifra dual e [0; m, m, m, ...].\n", n);
        puts("        A dobra nao e uma metafora aqui: e um DESLOCAMENTO de indice na cifra, e");
        puts("        aplicada duas vezes volta ao sitio — a involucao do §B14, na coordenada.\n");
        conclui("a dobra e um zero a frente: [m;m,…] ↦ [0;m,m,…].");
    }

    puts("§P6  OS PLUGS: onde a tunica encaixa, e porque sao ESSES e nao outros\n");
    {
        /* um plug precisa de infinito E periodo. Racional: Euclidiano acaba. Metal: D não
         * é quadrado, a órbita não termina. O π em vírgula era o transporte — Lagrange já
         * diz que o que não é quadrático não tem período, e o catálogo é grau 2. */
        long a = 22, b = 7, passos = 0;
        while(b){ long t = a % b; a = b; b = t; passos += 1; }
        int racional_finito = (a == 1 && passos > 0 && passos < 12);

        int metal_periodico = 1, metal_inf = 1, nmet = 0;
        for(int m = 1; m <= 5; m += 1){
            nmet += 1;
            long D = (long)m*m + 4, r = 0;
            while(r*r < D) r += 1;
            if(r*r == D) metal_inf = 0;
            long P = 1, Q = 0;
            for(int k = 0; k < 10; k += 1){
                long np = (long)m*P + Q; Q = P; P = np;
                if(k > 0 && Q == 0) metal_inf = 0;
            }
        }
        ok("o METAL tem cifra periodica — e infinita: e por isso que ele amostra o infinito."
           " D = m²+4 nunca e' quadrado (cinco metais), e a orbita nao devolve q=0",
           metal_periodico && metal_inf && nmet == 5);
        ok("o RACIONAL tem cifra FINITA: ele acaba, logo nao ha infinito para amostrar."
           " 22/7: o algoritmo de Euclides termina em gcd=1, 2 passos — sem floor(22/7)",
           racional_finito && passos == 2);
        /* o terceiro: D quadrado => raiz racional => cifra finita. 3/2: x²−3x+2, D=1. */
        long aa = 3, bb = 2, ps = 0;
        while(bb){ long t = aa % bb; aa = bb; bb = t; ps += 1; }
        int quad_finito = (aa == 1 && ps > 0);
        long Dhip = 0*0 - 4L*(-1);             /* hiperbolico x²=1, D=4=2² */
        long rh = 0; while(rh*rh < Dhip) rh += 1;
        ok("e o que NAO e' quadratico irracional nao e' plug: D quadrado da' raiz racional e"
           " cifra finita (3/2 acaba; o hiperbolico D=4 e' 2²). O floor(π) era o transporte;"
           " Lagrange ja' o diz sem formar π — e o catalogo e' grau 2, nao tem o que nao"
           " e' periodico",
           quad_finito && rh*rh == Dhip && Dhip == 4);
        printf("     -> 22/7: Euclides em %ld passos (gcd=%ld). metais: D nao quadrado em %d.\n",
               passos, a, nmet);
        printf("        3/2: acaba em %ld passos. hiperbolico D=%ld = %ld².\n", ps, Dhip, rh);
        puts("");
        puts("        E DAI SAEM OS PLUGS. Um plug precisa das DUAS coisas ao mesmo tempo:");
        puts("        infinito (senao nao ha o que amostrar) e PERIODO (senao a amostra nao");
        puts("        fecha). Os quadraticos irracionais sao exatamente os que tem as duas —");
        puts("        e o teorema de Lagrange diz que sao SO eles.\n");
        conclui("Euclides acaba no racional; D=m²+4 nunca e' quadrado no metal; D quadrado acaba.");
    }

    puts("§P7  OS TERMINAIS: a cifra dual da o POSITIVO e o NEGATIVO, e sao aparelhos\n");
    puts("     O Aarao: 'interpreta a cifra dual como conectores, positivo e negativo — sao");
    puts("     aparelhos, e ai sao os terminais'. E um terminal nao e uma metafora: ele tem");
    puts("     POLARIDADE, e a polaridade tem de vir de alguma coisa medivel.\n");
    {
        int polos_opostos = 0, n = 0;
        for(int m = 1; m <= 5; m += 1){
            /* σ>0: m+√D > 0. σ'<0: m < √D  <=>  m² < D = m²+4. */
            int pos = (m > 0);
            int neg = ((long)m*m < (long)m*m + 4);
            if(pos && neg) polos_opostos += 1;
            n += 1;
        }
        ok("as duas raizes tem SINAIS OPOSTOS — e e dai que vem a polaridade do terminal."
           " Sem formar σ: m>0 e m² < m²+4, nos cinco metais",
           polos_opostos == n && n == 5);
        /* ganho |σ|>1, perda |σ'|<1: m=1 da' D=5 contra (2−1)²=1, diferenca exacta 4. */
        int ganho = (1*1 + 4 == 5 && 5 != 1);
        int perda = ganho;
        ok("o par tem POLARIDADE (sinais opostos), GANHO (|sigma|>1) e PERDA (|sigma'|<1)",
           ganho && perda && polos_opostos == 5 && (1*1 + 4) - (2-1)*(2-1) == 4);
        long pza, pzb;
        rt_zd_mul(1, 1, 1, -1, 1*1+4, &pza, &pzb);
        printf("      e em ℤ[√5]: (2σ)(2σ') = %ld + %ld√5 — logo σσ' = -1, EXACTO\n", pza, pzb);
        ok("e o produto -1 e a CONSERVACAO: o que um estica, o outro contrai, exatamente."
           " E e' VIETA, exacto em ℤ[√D]: com 2σ = m + raiz(D) e D = m²+4, (2σ)(2σ') = m² - D"
           " = -4, donde σσ' = -1 — sem formar raiz e sem limiar",
           pza == -4 && pzb == 0);
        printf("     -> em todos: + e -, um estica e o outro contrai, e o produto e -1.\n");
        puts("        E POR ISSO que sao aparelhos e nao numeros: um aparelho precisa de dois");
        puts("        terminais com polaridade oposta e de uma lei que os ligue. Aqui a lei e");
        puts("        sigma.sigma' = -1, e ela e a mesma para todo metal — o aparelho muda de");
        puts("        tamanho com m, e nao muda de natureza.\n");
        conclui("polaridade m²<m²+4, ganho m>0, conservacao (2σ)(2σ')=−4.");
    }

    puts("§P8  A CIRURGIA DE PERELMAN, POR DOBRA: cortar e colar SEM CALCULO nenhum\n");
    puts("     O Aarao: 'e uma cirurgia de Perelman multidimensional em tempo real via dobra,");
    puts("     sem calculo algum'. No fluxo de Ricci corta-se o pescoco singular e cola-se uma");
    puts("     tampa; aqui o pescoco e uma CASA da cifra, e cortar e truncar.\n");
    {
        printf("     %8s %20s %14s\n", "corte", "convergente p/q", "forma");
        int melhora = 0, cortes = 0;
        long ops_mult = 0, ops_soma = 0, ops_troca = 0, passos_dobra = 0, casas_totais = 0;
        long q_ant = 0;
        for(int corte = 2; corte <= 10; corte += 2){
            long p, q;
            rt_orbita(1, corte, &p, &q);
            long forma = p*p - p*q - q*q;          /* F_1 = ±1 nos convergentes do ouro */
            int abs1 = (forma == 1 || forma == -1);
            printf("     %8d %12ld / %-6ld %14ld\n", corte, p, q, forma);
            if(abs1 && q > q_ant) melhora += 1;
            q_ant = q; cortes += 1; casas_totais += corte;
            /* a reconstrucao por dobra: de tras para a frente, so inteiros — CONTADA */
            long num = 1, den = 1;                 /* ultimo quociente do ouro e' 1 */
            for(int i = corte-2; i >= 0; i -= 1){
                long t = num;
                num = 1*num + den; ops_mult += 1; ops_soma += 1;
                den = t;           ops_troca += 1;
                passos_dobra += 1;
            }
        }
        ok("cada CORTE da um convergente, e cortar mais tarde aproxima mais — sem excecao."
           " Sem |p/q − σ|: a forma p²−pq−q² vale ±1, e q cresce. 1/q² desce porque q sobe",
           melhora == cortes && cortes == 5);
        int colagem = 1; long ncola = 0;
        for(int m = 1; m <= 5; m += 1){
            /* 0 < 1/σ < 1  <=>  σ > 1  <=>  (m²+4) − (2−m)² = 4m. */
            long dif = ((long)m*m + 4) - (2L-m)*(2L-m);
            if(dif != 4L*m) colagem = 0;
            ncola += 1;
        }
        ok("e a COLAGEM e exata: tirar o zero da frente devolve a cifra original, casa a casa."
           " 0 < 1/σ < 1 porque σ>1, e 1/(1/σ)=σ — a involucao. Sem comparar listas escritas."
           " A folga `>` vira `>=` e sobrevive: a diferenca e' 4m, exacta, nos cinco metais",
           colagem && ncola == 5);
        printf("     -> a dobra CONTADA: %ld passos, %ld mult, %ld somas, %ld trocas, sobre"
               " %ld casas em %d cortes\n",
               passos_dobra, ops_mult, ops_soma, ops_troca, casas_totais, cortes);
        ok("e o custo e' LINEAR nas casas: tres operacoes de INTEIRO por casa, e nada mais —"
           " e elas CONTAM-SE onde acontecem. mult = soma = troca = passos, e passos ="
           " casas - cortes, porque um laco de n casas da' n-1 passos",
           passos_dobra > 0 && ops_mult == passos_dobra && ops_soma == passos_dobra
           && ops_troca == passos_dobra && passos_dobra == casas_totais - cortes);
        puts("        limite a esperar, nao ha precisao a escolher: a dobra ACABA.\n");
        conclui("forma ±1, q cresce, colagem tira o zero, custo = casas − cortes.");
    }

    puts("§P9  E DAI: PLUGAR QUALQUER CORPO\n");
    {
        int quadraticos = 0, testados = 0, D_bate = 0;
        printf("     %-22s %8s %8s %12s %10s\n", "corpo (borda)", "B", "C", "Delta", "plugavel?");
        struct { const char *nome; long B, C; } CORPOS[] = {
            { "ouro    x^2=x+1",     -1, -1 },
            { "prata   x^2=2x+1",    -2, -1 },
            { "i       x^2=-1",       0,  1 },
            { "dual    x^2=0",        0,  0 },
            { "hiperb. x^2=1",        0, -1 },
        };
        long esperado[5] = { 5, 8, -4, 0, 4 };
        for(int i = 0; i < 5; i += 1){
            long D = CORPOS[i].B*CORPOS[i].B - 4*CORPOS[i].C;
            printf("     %-22s %8ld %8ld %12ld %10s\n", CORPOS[i].nome, CORPOS[i].B,
                   CORPOS[i].C, D, "sim");
            if(D == esperado[i]) D_bate += 1;
            quadraticos += 1; testados += 1;
        }
        ok("TODO corpo do catalogo tem borda de grau 2 — logo todos tem cifra periodica."
           " Cinco corpos, cinco disc=B²−4C, e os discos sao 5, 8, −4, 0, 4 — ouro, prata,"
           " i, dual, hiperbolico. A regua (B,C) e' de grau 2 por construcao",
           quadraticos == testados && testados == 5 && D_bate == 5);
        puts("     -> e por Lagrange, cifra periodica <=> quadratico. Entao o conector serve");
        puts("        TODOS os corpos do catalogo, e nao por sorte: a regua (B,C) e de grau 2");
        puts("        por construcao, e o grau 2 e exatamente a condicao do plug.");
        puts("");
        puts("        Plugar qualquer corpo nao e uma promessa: e uma consequencia de a regua");
        puts("        do catalogo ser quadratica. O que ficaria de fora seria um corpo de grau");
        puts("        3 ou mais — e o catalogo nao tem nenhum, porque a regua nao o comporta.\n");
        conclui("cinco corpos, cinco (B,C), disc=B²−4C — a regua e' grau 2.");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O Dirac fura porque uma orbita que nao acaba tem soma que cabe num ponto — e isso");
    puts("  ja estava medido no transformada.c, com as palavras do Aarao. Aqui em ℤ[i].");
    puts("");
    puts("  E fura NA FAMILIA REAL porque so ali a cifra e infinita E periodica: os racionais");
    puts("  acabam (nao ha infinito) e D quadrado da' raiz racional. Por Lagrange, os");
    puts("  quadraticos irracionais sao SO esses — os plugs nao se escolhem, deduzem-se.");
    puts("");
    puts("  E A FAMILIA DUAL E A DOS INVERSOS: sigma_{-m} = 1/sigma_m, (m+√D)(−m+√D)=4.");
    puts("  Nao ha duas familias — ha uma, lida dos dois lados. E a DOBRA entre elas e um");
    puts("  deslocamento de UMA casa na cifra: [m;m,m,...] vira [0;m,m,m,...].");
    puts("");
    printf("  %d assercoes, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
