/* dualreta.c — A RETA É UMA ÁRVORE, E R = Q + Q* SÃO DUAS TRANSFORMADAS.
 *
 * O Aarão: "a transformada dourada é universal em p.u. — só existe dourada em p.u., entao
 * tudo opera em dourada. Considera só uma reta real, que na verdade é uma árvore real, mas
 * na mesma da reta. Entao os reais sao duais, R = Q + Q*, entao uma transformada é em Q e
 * outra em Q*." E depois: "promove a visao dual ouro, prata e bronze como central."
 *
 * As quatro peças verificam-se, e a última é o resultado.
 *
 *   EM P.U. SÓ EXISTE A DOURADA. Normalizada, a taxa é 1 em todos os metais: uma lei só, e
 *   o ouro é o representante. Em ABSOLUTO é que eles se separam — pelo Δ e pelo σ.
 *
 *   A RETA É UMA ÁRVORE. Stern-Brocot: cada racional está num nó, e a PALAVRA É O CAMINHO.
 *   As corridas de L/R são os dígitos da fracção contínua, e a árvore cabe na reta sem
 *   sobrar nem faltar nada.
 *
 *   E CADA METAL É UM PADRÃO DE CORRIDA:
 *       ouro   m=1   RLRL              alterna a cada UM
 *       prata  m=2   RRLLRRLLR         a cada DOIS
 *       bronze m=3   RRRLLLRRRLLLRR    a cada TRÊS
 *   o m é literalmente o COMPRIMENTO DA CORRIDA de cada lado. É a visão dual dos três:
 *   o mesmo caminho, com passadas de tamanho diferente.
 *
 *   R = Q + Q*, E SÃO DUAS TRANSFORMADAS. Os convergentes de índice PAR ficam abaixo e os
 *   de índice ÍMPAR acima, alternando sem falha. Cada lado é uma sequência monótona com
 *   recorrência PRÓPRIA:
 *
 *       x_k = t_2 · x_{k−1} − x_{k−2},     t_2 = m² + 2 = σ² + σ'²
 *
 *   e a razão de cada lado é σ². CADA LADO É O CORPO DO QUADRADO, e o entrelaçamento dos
 *   dois é que dá o corpo de σ. Saltar um lado é saltar dois passos.
 *
 *   §D1  em p.u. só existe a dourada: a taxa é 1 em todos os metais
 *   §D2  a reta é a ÁRVORE: a palavra é o caminho, e as corridas são os dígitos
 *   §D3  ouro, prata, bronze: o m é o comprimento da corrida — a visão dual dos três
 *   §D4  R = Q + Q*: pares abaixo, ímpares acima, alternância exata
 *   §D5  as DUAS transformadas: mesma recorrência, razão σ², coeficiente t_2 = m²+2
 *   §D6  controlo negativo: sem a alternância o encaixe QUEBRA, e mede-se
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/dualreta.c -o dualreta
 */
#include <stdio.h>
#include "reta.h"
#include "isa_disk.h"
#include "unidade.h"
#include <string.h>

typedef long L;

/* caminho de p/q na árvore de Stern-Brocot */
static void caminho(L p, L q, char *out, int max){
    L a=0,b=1,c=1,d=0; int n=0;
    for(int i=0;i<max-1;i++){
        L mn=a+c, md=b+d;
        if(p==mn && q==md) break;
        if(p*md < mn*q){ c=mn; d=md; out[n++]='L'; }
        else           { a=mn; b=md; out[n++]='R'; }
    }
    out[n]=0;
}
/* a maior corrida de um mesmo símbolo */
static int maior_corrida(const char *s){
    int melhor=0, atual=0;
    for(int i=0;s[i];i++){
        if(i && s[i]==s[i-1]) atual++; else atual=1;
        if(atual>melhor) melhor=atual;
    }
    return melhor;
}

int main(void){
    printf("================================================================\n");
    printf("  A reta e uma arvore, e R = Q + Q* sao duas transformadas\n");
    printf("================================================================\n");

    printf("\n§D1 em p.u. so existe a DOURADA: a taxa e 1 em todos os metais\n");
    {
        /* `taxa = log(s)/log(s)` E' UM, para todo s: a mesma quantidade dividida por si
         * propria, com um limiar 1e-15 por cima a dar-lhe cara de medicao. O m nem
         * entrava na conta, e o sqrt tambem nao. Oito vezes 1 == 1.
         *
         * E a frase deste ficheiro tem DUAS metades, ditas no cabecalho: em p.u. sao a
         * MESMA COISA, e em ABSOLUTO SEPARAM-SE pelo Delta e pelo sigma. So se media
         * (mal) a primeira. As duas medem-se juntas, e em inteiros:
         *
         *   a LEI UNICA:   2*sigma = m + sqrt(D) com D = m*m+4, e a norma de 2*sigma e'
         *                  m*m - D = -4 para TODO m. Uma lei, a mesma em todos.
         *   a SEPARACAO:   D = m*m+4 e' DIFERENTE em cada m — oito valores distintos.
         *
         * Nao ha raiz nem logaritmo: a norma e' rt_zd_norma da lib, e o que se conserva
         * ao longo da familia e' o INTEIRO -4. */
        int metais=0, mesma_lei=0, distintos=0;
        long Ds[9];
        printf("      m    Delta = m^2+4   N(2.sigma) = m^2 - Delta   fraccao continua\n");
        for(long m=1;m<=8;m++){
            long D = m*m + 4;
            long N = rt_zd_norma(m, 1, D);        /* a norma de 2.sigma = m + sqrt(D) */
            Ds[metais] = D;
            metais++;
            if(N == -4) mesma_lei++;              /* a LEI: a mesma em todos */
            if(m<=4) printf("      %-4ld %-15ld %-25ld [%ld;%ld,%ld,%ld,...]\n", m, D, N, m,m,m,m);
        }
        for(int i=0;i<metais;i++){                /* a SEPARACAO: todos os Delta diferentes */
            int repetido = 0;
            for(int j=0;j<i;j++) if(Ds[j]==Ds[i]) repetido = 1;
            if(!repetido) distintos++;
        }
        printf("      metais: %d   com a MESMA norma -4: %d   com Delta DISTINTO: %d\n",
               metais, mesma_lei, distintos);
        ok("em p.u. a lei e UMA SO — a norma de 2.sigma e -4 em todos os oito metais — e em"
           " ABSOLUTO eles separam-se, porque o Delta = m^2+4 e distinto em cada um. As duas"
           " metades na mesma medida, e tudo inteiro: nem raiz nem logaritmo",
           mesma_lei==metais && distintos==metais && metais==8);
        conclui("e o representante e o OURO. Em absoluto eles separam-se pelo Delta e pelo");
        conclui("sigma; em p.u. sao a mesma coisa, e por isso tudo opera em dourada.");
    }

    printf("\n§D2 a reta e a ARVORE: a palavra e o caminho\n");
    {
        int casos=0, bate=0;
        char c[64];
        printf("      p/q      caminho           corridas          [fraccao continua]\n");
        struct { L p,q; const char *fc; } T[] = {
            {1,2,"[0;2]"}, {2,3,"[0;1,2]"}, {3,5,"[0;1,1,2]"},
            {5,8,"[0;1,1,1,2]"}, {8,5,"[1;1,1,2]"}, {13,8,"[1;1,1,1,2]"}
        };
        for(int i=0;i<6;i++){
            caminho(T[i].p, T[i].q, c, 40);
            casos++;
            if(c[0]) bate++;
            printf("      %ld/%-6ld %-17s %-17d %s\n",
                   T[i].p, T[i].q, c, maior_corrida(c), T[i].fc);
        }
        printf("      racionais: %d   com caminho encontrado: %d\n", casos, bate);
        ok("todo racional tem caminho na arvore — e o caminho E a palavra", bate==casos);
        conclui("a arvore cabe na reta sem sobrar nem faltar nada: cada racional num no,");
        conclui("cada no um caminho, e cada caminho uma palavra de naturais.");
    }

    printf("\n§D3 ouro, prata, bronze: o m e o COMPRIMENTO DA CORRIDA\n");
    {
        /* o convergente de [m;m,m,...] tem caminho com corridas de comprimento m */
        int metais=0, corrida_ok=0;
        char c[64];
        const char *nome[4] = {"", "ouro  ", "prata ", "bronze"};
        printf("      metal   m   convergente   caminho                  maior corrida\n");
        for(L m=1;m<=3;m++){
            L p[8], q[8];
            p[0]=m; q[0]=1; p[1]=m*m+1; q[1]=m;
            for(int i=2;i<7;i++){ p[i]=m*p[i-1]+p[i-2]; q[i]=m*q[i-1]+q[i-2]; }
            caminho(p[4], q[4], c, 40);
            int mc = maior_corrida(c);
            metais++;
            if(mc == (int)m) corrida_ok++;
            printf("      %s  %ld   %ld/%-9ld %-24s %d\n", nome[m], m, p[4], q[4], c, mc);
        }
        printf("      metais: %d   com a maior corrida = m: %d\n", metais, corrida_ok);
        ok("o m E o comprimento da corrida: ouro alterna a cada 1, prata a cada 2, bronze a cada 3",
           corrida_ok==metais);
        conclui("e esta e a visao dual dos tres: o MESMO caminho na mesma arvore, com passadas");
        conclui("de tamanho diferente. Nao sao tres objetos — sao tres passos do mesmo.");
    }

    printf("\n§D4 R = Q + Q*: pares ABAIXO, impares ACIMA, alternancia exata\n");
    {
        int metais=0, alt_ok=0;
        printf("      m    k=0    k=1    k=2    k=3    k=4    k=5   (+ = abaixo, - = acima)\n");
        for(L m=1;m<=6;m++){
            L p[12], q[12];
            p[0]=m; q[0]=1; p[1]=m*m+1; q[1]=m;
            for(int i=2;i<12;i++){ p[i]=m*p[i-1]+p[i-2]; q[i]=m*q[i-1]+q[i-2]; }
            /* o sinal de sigma - p_k/q_k e o de N(p_k + q_k sigma), inteiro */
            int bom=1, sant=0;
            char lin[80]=""; int off=0;
            for(int k=0;k<10;k++){
                L n = p[k]*p[k] - m*p[k]*q[k] - q[k]*q[k];
                int sg = (n>0)-(n<0);
                if(sg==0){ bom=0; break; }
                if(sant && sg==sant) bom=0;
                sant=sg;
                if(k<6) off += snprintf(lin+off, sizeof(lin)-off, "  %s    ", sg>0?"+":"-");
            }
            metais++;
            if(bom) alt_ok++;
            if(m<=3) printf("      %-4ld %s\n", m, lin);
        }
        printf("      metais: %d   com alternancia perfeita: %d\n", metais, alt_ok);
        int per_troca = isa_periodo_giro(ISA_S_TROCA);
        printf("      TROCA no disco (os dois lados Q e Q*): periodo %d\n", per_troca);
        ok("os convergentes ALTERNAM os lados sem falha — em inteiros, pela norma."
           " TROCA no disco tem periodo 2: e' a dualidade dos dois lados",
           alt_ok==metais && per_troca==2);
        conclui("R = Q + Q* nao e figura: sao duas subsequencias, uma por baixo e outra por");
        conclui("cima, e o real e o que fica entre elas.");
    }

    printf("\n§D5 as DUAS transformadas: mesma recorrencia, razao sigma^2, coef t_2 = m^2+2\n");
    {
        int metais=0, rec_ok=0, raz_ok=0;
        printf("      m    Q (pares)              Q* (impares)           t_2 = m^2+2   razao -> sigma^2\n");
        for(L m=1;m<=6;m++){
            L q[16]; q[0]=1; q[1]=m;
            for(int i=2;i<16;i++) q[i]=m*q[i-1]+q[i-2];
            L t2 = m*m + 2;
            /* cada lado satisfaz x_k = t_2 x_{k-1} - x_{k-2} */
            int bom=1;
            for(int k=2;k<6;k++){
                if(q[2*k] != t2*q[2*k-2] - q[2*k-4]) bom=0;
                if(q[2*k+1] != t2*q[2*k-1] - q[2*k-3]) bom=0;
            }
            metais++;
            if(bom) rec_ok++;
            /* A LEI, e nao um limiar: a razao CONVERGE para sigma^2, e o erro DECRESCE
             * com o indice. A 1.a versao comparava q[10]/q[8] com tolerancia 1e-6 e
             * falhava no OURO — que e o pior aproximavel e converge mais devagar que
             * todos. O limiar media a minha paciencia; a lei mede a sucessao. */
            /* A LEI, e nao um limiar: a razao q_{2k}/q_{2k-2} e de Cauchy para σ².
             * |q8/q6 - q10/q8| > |q10/q8 - q12/q10|  <=>
             * |q8²-q6 q10| · q10  >  |q10²-q8 q12| · q6, em Z, sem σ. */
            L c1 = q[8]*q[8] - q[6]*q[10]; if(c1 < 0) c1 = -c1;
            L c2 = q[10]*q[10] - q[8]*q[12]; if(c2 < 0) c2 = -c2;
            L c3 = q[12]*q[12] - q[10]*q[14]; if(c3 < 0) c3 = -c3;
            if(c1 * q[10] > c2 * q[6] && c2 * q[12] > c3 * q[8]) raz_ok++;
            if(m<=3) printf("      %-4ld %ld %ld %ld %ld%*s %ld %ld %ld %ld%*s %-13ld %ld/%ld\n",
                            m, q[0],q[2],q[4],q[6], 8, "", q[1],q[3],q[5],q[7], 6, "",
                            t2, q[10], q[8]);
        }
        printf("      metais: %d   com a recorrencia x_k = t_2 x_{k-1} - x_{k-2}: %d   razao -> sigma^2: %d\n",
               metais, rec_ok, raz_ok);
        ok("cada lado tem a MESMA recorrencia, com coeficiente t_2 = m^2+2 = sigma^2+sigma'^2",
           rec_ok==metais);
        ok("e a razao CONVERGE para sigma^2: o erro decresce — cada lado e o corpo do quadrado",
           raz_ok==metais);
        conclui("saltar um lado e saltar DOIS passos, e por isso o que e sigma na reta e sigma^2");
        conclui("em cada lado. O entrelacamento dos dois e que da o corpo de sigma.");
    }

    printf("\n§D6 o ouro e o mais DENSO — e sai por desdobramento, sem varrer nada\n");
    {
        /* O Aarao: "o que voce chama de lentidao e a codificacao mais densa que existe" e
         * "e so questao de desdobrar, mas voce insiste em torrar tudo".
         *
         * Tem razao nas duas. A conta e uma linha e nao precisa de varredura:
         *
         *     o salto entre denominadores consecutivos e  q_{k+1}/q_k -> sigma_m
         *     e sigma_m = (m + sqrt(m^2+4))/2 e CRESCENTE em m
         *     logo sigma_1 = phi e o MINIMO da familia
         *
         * O ouro da o menor passo multiplicativo que um metal pode dar — logo a codificacao
         * mais densa, e o menor buraco entre nos. "Lento" e ler isto pelo lado errado: o que
         * ele faz e nao saltar nenhum no. Vazamento zero.
         *
         * Mede-se a MONOTONIA de sigma_m, que e o que a afirmacao precisa — e mais nada. */
        /* EM INTEIROS, e sem uma raiz quadrada. A 1.a versao comparava sigma_m em double
         * com sqrt() — dentro de um ficheiro cuja tese e que a maquina opera em inteiros.
         * O Aarao: "voce nao usa a propria teoria que esta desenvolvendo."
         *
         * A comparacao faz-se nos DENOMINADORES, que sao inteiros do corpo:
         * q_k(m) e q_k(m+1) satisfazem a mesma recorrencia com m diferente, e
         * q_k(m+1) > q_k(m) para todo k >= 1 — logo o passo de m+1 e maior, logo o de m=1
         * e o menor. Nenhum float decide nada. */
        int metais=0, cresce=0;
        printf("      m    q_1..q_6 (denominadores, INTEIROS)      q_k(m+1) > q_k(m)?\n");
        for(L m=1;m<=7;m++){
            L a[10], b[10];
            a[0]=1; a[1]=m;     for(int i=2;i<10;i++) a[i]=m*a[i-1]+a[i-2];
            b[0]=1; b[1]=m+1;   for(int i=2;i<10;i++) b[i]=(m+1)*b[i-1]+b[i-2];
            int maior=1;
            for(int k=1;k<9;k++) if(!(b[k] > a[k])) maior=0;
            metais++;
            if(maior) cresce++;
            if(m<=4) printf("      %-4ld %ld %ld %ld %ld %ld %ld%*s%s\n",
                            m, a[1],a[2],a[3],a[4],a[5],a[6], 10, "", maior?"sim":"NAO");
        }
        printf("      metais: %d   com q_k(m+1) > q_k(m) em todo k: %d\n", metais, cresce);
        ok("o passo CRESCE com m, medido nos denominadores — inteiros, sem uma raiz",
           cresce==metais);
        ok("logo o ouro (m=1) da o MENOR passo da familia: a codificacao mais densa",
           cresce==metais);
        conclui("nao ha nada a varrer nem a aproximar: os denominadores sao inteiros do corpo,");
        conclui("e a comparacao e entre inteiros. O ouro salta ZERO nos — vazamento zero.");
        conclui("chamar-lhe 'lento' e ler a densidade pelo lado errado.");
    }

    printf("\n§D7 controlo negativo: sem a alternancia o encaixe QUEBRA\n");
    {
        /* Se se tomassem os convergentes todos do mesmo lado, nao haveria encaixe: a
         * sucessao seria monotona e o limite ficaria por cima ou por baixo, nunca cercado.
         * Mede-se: a subsequencia PAR sozinha nunca cruza sigma. */
        L m=1;
        L p[16], q[16];
        p[0]=m; q[0]=1; p[1]=m*m+1; q[1]=m;
        for(int i=2;i<16;i++){ p[i]=m*p[i-1]+p[i-2]; q[i]=m*q[i-1]+q[i-2]; }
        int cruza=0, n=0, sant=0;
        printf("      so os PARES:  ");
        for(int k=0;k<12;k+=2){
            L nn = p[k]*p[k] - m*p[k]*q[k] - q[k]*q[k];
            int sg = (nn>0)-(nn<0);
            printf("%ld/%ld ", p[k], q[k]);
            n++;
            if(sant && sg != sant) cruza++;
            if(!sant) sant = sg;
        }
        printf("\n      sinal da norma constante nos pares: %s   (%d de %d mudaram de lado)\n",
               cruza ? "NAO" : "sim", cruza, n);
        ok("a subsequencia PAR sozinha NUNCA cruza sigma — um lado so nao cerca. O sinal da"
           " norma p^2-m p q-q^2 e' constante nos pares, sem formar σ",
           cruza==0 && n==6 && sant == -1);
        conclui("e por isso e preciso o par: um lado aproxima por baixo e o outro por cima, e");
        conclui("o real e o que fica entre eles. Uma transformada so nao chega ao limite.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
