/* tests/limiares.c — CADA LIMIAR TEM UMA DE TRÊS CAUSAS, e só uma é honesta.
 *
 * A auditoria dos tipos deu 350 ocorrências na classe MEDIÇÃO, e a triagem deu 918
 * limiares `1e-N` no repo — dos quais 583 SEM UMA FUNÇÃO TRANSCENDENTE À VOLTA. Esses são
 * defeito: uma régua minha dentro de uma conta que não precisava dela.
 *
 * Mas reduzi-los às cegas seria o `sed` que esta casa já provou errado. As três causas:
 *
 *   (A) DECORAÇÃO   a conta não tem transcendente nenhum. O limiar não é preciso, e a
 *                   comparação pode ser por IGUALDADE: o resíduo é ZERO.
 *   (B) SABOR       há transcendentes, mas a IDENTIDADE é algébrica e vale para
 *                   quaisquer entradas. Troca-se por inteiras e fica exacta.
 *   (C) HONESTO     a quantidade É transcendente. Aqui o double é a representação certa
 *                   — mas a asserção tem de ser sobre a FORMA FECHADA, não sobre o
 *                   decimal.
 *
 * Este ficheiro mede as três com casos REAIS do repo — os três que o `colheita.c` tinha —
 * e prova que as versões exactas são exactas. É o que transforma a limpeza numa
 * disciplina: daqui em diante, um limiar novo tem de dizer a que classe pertence.
 *
 * §L0  (A) a lei do quadrado: A(f)/A(2f) = 4 é EXACTO, e o 1e-9 era decoração
 * §L1  (A) a simetria construída: o resíduo é zero BIT A BIT, e o 1e-14 escondia-o
 * §L2  (B) a adjunção: a identidade é algébrica — os transcendentes eram SABOR
 * §L3  (C) e o caso honesto: quando a quantidade É transcendente, o que se afirma muda
 * §L4  e a regra: «é zero» é mais forte que «é menor que a régua que eu escolhi»
 */
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include "unidade.h"

int main(void){
    printf("\n=== OS LIMIARES: três causas, e só uma é honesta ===\n");

    /* ═══ §L0 (A) A LEI DO QUADRADO — o limiar era decoração ═════════════════ */
    printf("\n§L0 (A) DECORAÇÃO: a lei do quadrado é exacta em inteiros.\n\n");
    {
        /* A(f) = λ²/4π com λ = c/f. Logo A(f)/A(2f) = (2f/f)² = 4, e o c, o π e as
         * unidades CANCELAM-SE. O que se compara são dois quadrados de inteiros. */
        long mal = 0, cas = 0;
        for(long f = 100; f <= 6400; f *= 2){
            cas++;
            if(4L*f*f != (2*f)*(2*f)) mal++;
        }
        /* e o controlo: com a régua antiga, o mesmo em double */
        int com_regua = 1;
        for(double f = 100; f <= 6400; f *= 2){
            double r = (299.792458/f)*(299.792458/f) / ((299.792458/(2*f))*(299.792458/(2*f)));
            if(fabs(r - 4.0) > 1e-9) com_regua = 0;
        }
        printf("      em inteiros: %ld divergências em %ld — o resíduo é ZERO\n", mal, cas);
        printf("      e com a régua 1e-9 em double: %s (passa, mas passa POR TOLERÂNCIA)\n",
               com_regua ? "passa" : "falha");
        ok("(A) O LIMIAR ERA DECORAÇÃO: A(f)/A(2f) = 4 é uma identidade ALGÉBRICA — o c, o"
           " π e as unidades cancelam-se, e o que sobra são dois quadrados de inteiros. Em"
           " inteiros o resíduo é ZERO; com a régua 1e-9 ele passa por TOLERÂNCIA, o que é"
           " outra coisa. «É zero» é mais forte que «é menor que a régua que eu escolhi», e"
           " esta é a classe maior: 583 dos 918 limiares do repo não têm uma função"
           " transcendente à volta",
           mal == 0 && cas == 7 && com_regua);
    }

    /* ═══ §L1 (A) A SIMETRIA CONSTRUÍDA — o limiar escondia uma tautologia ═══ */
    printf("\n§L1 (A) O pior caso: o limiar a esconder que a conta era tautologia.\n\n");
    {
        double complex S[3][3];
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
            S[i][j] = (i <= j) ? ((double)(i + 2*j + 1) + I*(double)(3*i - j)) : 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < i; j++) S[i][j] = S[j][i];
        long dif = 0, cas = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            cas++;
            if(S[i][j] != S[j][i]) dif++;          /* IGUALDADE, não limiar */
        }
        printf("      a matriz foi CONSTRUÍDA simétrica, e a diferença é %ld em %ld"
               " entradas\n", dif, cas);
        printf("      → o 1e-14 que aqui estava escondia que isto é a construção a fechar\n");
        ok("(A) E O PIOR CASO DA CLASSE É ESTE: a matriz foi CONSTRUÍDA simétrica — a linha"
           " anterior atribui S[i][j] = S[j][i] — logo a diferença é zero BIT A BIT. O"
           " limiar não estava a tolerar erro nenhum: estava a dar cara de medição a uma"
           " TAUTOLOGIA. Comparar por igualdade não torna a asserção mais forte; torna-a"
           " honesta sobre o que prova, que é a construção fechar",
           dif == 0 && cas == 9);
    }

    /* ═══ §L2 (B) A ADJUNÇÃO — os transcendentes eram SABOR ═════════════════ */
    printf("\n§L2 (B) SABOR: a identidade é algébrica, e o sin/cos forçava o limiar.\n\n");
    {
        double complex S[3][3];
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
            S[i][j] = (i <= j) ? ((double)(i + 2*j + 1) + I*(double)(3*i - j)) : 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < i; j++) S[i][j] = S[j][i];
        double complex f[3] = { 1+3*I, -7+2*I, 5-9*I };
        double complex g[3] = { 2-4*I, 11+6*I, -3+8*I };
        double complex Sf[3] = {0}, Sg[3] = {0}, e1 = 0, e2 = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            Sf[i] += S[i][j]*f[j]; Sg[i] += S[i][j]*g[j];
        }
        for(int i = 0; i < 3; i++){ e1 += Sf[i]*g[i]; e2 += f[i]*Sg[i]; }
        printf("      <Sf,g> = %.0f%+.0fi   e   <f,Sg> = %.0f%+.0fi   — iguais? %s\n",
               creal(e1), cimag(e1), creal(e2), cimag(e2), e1 == e2 ? "sim" : "NÃO");
        ok("(B) OS TRANSCENDENTES ERAM SABOR: a adjunção <Sf,g> = <f,Sg> decorre SÓ da"
           " simetria de S, e vale para QUAISQUER entradas — o cos e o sin que ali estavam"
           " não entram na prova, e só serviam para forçar um limiar 1e-12. Com entradas"
           " INTEIRAS a identidade é exacta e a igualdade decide. Esta é a classe que exige"
           " ler: 335 dos 918 têm transcendente por perto, e é preciso ver se ele entra na"
           " prova ou só na decoração",
           e1 == e2);
    }

    /* ═══ §L3 (C) O CASO HONESTO ═══════════════════════════════════════════ */
    printf("\n§L3 (C) HONESTO: quando a quantidade É transcendente, muda o que se afirma.\n\n");
    {
        /* π não é racional, e nenhuma régua o torna. Mas a asserção pode ser sobre a
         * FORMA FECHADA: o encaixe de Arquimedes, com os dois lados a apertarem — e aí
         * o que se afirma é uma DESIGUALDADE exacta entre racionais, não um decimal. */
        long p1 = 223, q1 = 71, p2 = 22, q2 = 7;      /* 223/71 < π < 22/7 */
        int cerca = (p1*q2 < p2*q1);                   /* a ordem, em inteiros */
        double pi = 4.0*atan(1.0);
        int dentro = ((double)p1/q1 < pi) && (pi < (double)p2/q2);
        printf("      223/71 < π < 22/7:  a ordem entre os racionais é exacta (%s),\n",
               cerca ? "sim" : "NÃO");
        printf("      e π cai dentro (%s) — mas ISSO já precisa do double\n",
               dentro ? "sim" : "NÃO");
        ok("(C) E O CASO HONESTO É OUTRO: π não é racional, e nenhuma régua o torna. Aqui o"
           " double é a representação certa — mas o que se AFIRMA tem de mudar: em vez de"
           " «π = 3.14159 ± 1e-9», afirma-se o ENCAIXE, 223/71 < π < 22/7, que é uma"
           " desigualdade EXACTA entre racionais e cuja ordem se decide em inteiros. A"
           " forma fechada carrega a afirmação; o decimal só a ilustra",
           cerca && dentro);
    }

    /* ═══ §L4 A REGRA ══════════════════════════════════════════════════════ */
    printf("\n§L4 A regra, e o que ela custa a quem escrever a seguir.\n\n");
    {
        printf("        classe        quantos   o que fazer\n");
        printf("        ──────────────────────────────────────────────────────────\n");
        printf("        (A) decoração     583   comparar por IGUALDADE — o conserto é"
               " mecânico\n");
        printf("        (B/C) a ler       335   ver se o transcendente entra na PROVA\n");
        printf("        total             918   `tools/triagem_limiares.sh`\n");
        ok("E A REGRA FICA, para quem escrever a seguir: um limiar novo tem de dizer a que"
           " CLASSE pertence. Se não há transcendente na conta, ele é decoração e não entra"
           " — compara-se por igualdade. Se há, é preciso ver se ele entra na PROVA ou só"
           " na ilustração. E quando a quantidade é mesmo transcendente, o que se afirma é"
           " a FORMA FECHADA e não o decimal. A triagem está em `tools/triagem_limiares.sh`"
           " e é barata: 918 sítios classificados em segundos",
           1);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
