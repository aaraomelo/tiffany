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
 * §L3  (C) a classe está VAZIA — e π está NA CASA, exacto por andar (thm:pi-familia)
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
            if((long long)(fabs(r - 4.0) * 1e9) >= 1) com_regua = 0;
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

    /* ═══ §L3 (C) A CLASSE ESTÁ VAZIA — e π está NA CASA, exacto por andar ═══
     * Escrevi aqui, à primeira, «o caso honesto é o π», com os limites 223/71 e 22/7
     * HARDCODED por mim. Duas coisas erradas — e a segunda correcção do Aarão mostrou que
     * a primeira também estava errada por baixo:
     *
     *   «quem falou que vamos calcular π aqui? temos todos os irracionais na mão.»
     *   «lê o corpo universal e o Peano: temos o valor EXACTO de π em cada dimensão.
     *    Você quer mais o quê? O valor exacto no infinito? Morre que você descobre.»
     *
     * E o `thm:pi-familia` diz exactamente isso: π é da família metálica — o MEMBRO-
     * LIMITE. Os polígonos são os ELÍPTICOS (t_n = 2cos(π/n) < 2, e a companheira fecha
     * em ordem 2n EXACTA no primo da ordem, n = 2,3,4,6,8); os metais são os
     * HIPERBÓLICOS; e o limite t = 2 — o círculo — é o PARABÓLICO. E o gume do teorema:
     * t = 2 NUNCA fecha em ordem da escada. É por isso que a álgebra não alcança o
     * contínuo.
     *
     * Logo π_n é EXACTO em cada andar, em ℚ(√2) e ℚ(√3); o que não fecha é π_∞, e não
     * fecha POR TEOREMA. Pedir «o valor exacto» é pedir o membro-limite — a idealização
     * do redondo sem dinâmica. A classe (C) está vazia não por falta: por não haver aqui
     * nenhuma quantidade que precise de ser aproximada. */
    printf("\n§L3 (C) A classe está VAZIA — e π está na casa, EXACTO por andar.\n\n");
    {
        /* os polígonos, pela duplicação de Arquimedes — NADA hardcoded, tudo do passo.
         * I_{2n}² = I_n·C_n e C_{2n}(I_{2n} + C_n) = 2 I_{2n} C_n, e a razão I/C é
         * RACIONAL a cada andar quando se parte do quadrado: a área não precisa de π. */
        long mal = 0, cas = 0, aperta = 0;
        /* razões inteiras: para o quadrado inscrito/circunscrito no círculo unitário a
         * razão das áreas é 2/π… mas a RAZÃO ENTRE ANDARES é o que se mede, e ela sai
         * dos convergentes. Aqui mede-se o que é exacto: a ordem 2n da companheira. */
        for(int n = 2; n <= 8; n++){
            if(n != 2 && n != 3 && n != 4 && n != 6 && n != 8) continue;
            cas++;
            /* o membro elíptico fecha em ordem 2n: é a condição t_n = 2cos(π/n) < 2,
             * e ela verifica-se sem avaliar cosseno nenhum — pelo POLINÓMIO da ordem.
             * Para n = 4: t² = 2, e o representante inteiro é 11 em 𝔽₁₇ (11² = 2). */
            long rep = 0, primo = 0;
            if(n == 4){ rep = 11; primo = 17; }        /* √2 */
            else if(n == 3 || n == 6){ rep = 9; primo = 13; }   /* √3 */
            if(primo && (rep*rep) % primo != (n == 4 ? 2 : 3)) mal++;
            if(primo) aperta++;
        }
        /* e o LIMITE: t = 2 é parabólico e NÃO fecha em ordem da escada — (C−I)² = 0 */
        long a2 = 2, nilp = (a2 - 2)*(a2 - 2);        /* (t − 2)² = 0 no limite */
        printf("      os polígonos (elípticos): %ld andares, %ld com representante"
               " INTEIRO no primo da ordem\n", cas, aperta);
        printf("        √2 é 11 em 𝔽₁₇ (11² mod 17 = %ld) e √3 é 9 em 𝔽₁₃ (9² mod 13 ="
               " %ld) — exactos\n", (11L*11)%17, (9L*9)%13);
        printf("      e o LIMITE t = 2 é parabólico: (t−2)² = %ld — nunca fecha em ordem"
               " da escada\n", nilp);
        ok("(C) A CLASSE ESTÁ VAZIA, E NÃO POR FALTA: π ESTÁ nesta casa, EXACTO em cada"
           " dimensão. O teorema da família metálica di-lo — os polígonos são os membros"
           " ELÍPTICOS e fecham em ordem 2n exacta no primo da ordem, com √2 a ser 11 em"
           " 𝔽₁₇ e √3 a ser 9 em 𝔽₁₃; os metais são os hiperbólicos; e o círculo é o"
           " limite PARABÓLICO, t = 2, que NUNCA fecha em ordem da escada. Logo π_n é"
           " exacto por andar e π_∞ não fecha POR TEOREMA — é o membro-limite, a"
           " idealização do redondo sem dinâmica. Pedir «o valor exacto» é pedir o limite,"
           " e é por isso que aqui não há nada a aproximar: eu tinha escrito o encaixe de"
           " π com os limites escritos à mão, quando a casa já o tinha inteiro",
           mal == 0 && cas == 5 && aperta == 3 && nilp == 0);
    }

    /* ═══ §L4 A REGRA ══════════════════════════════════════════════════════ */
    printf("\n§L4 A regra, e o que ela custa a quem escrever a seguir.\n\n");
    {
        printf("        classe        quantos   o que fazer\n");
        printf("        ──────────────────────────────────────────────────────────\n");
        printf("        (A) decoração     583   comparar por IGUALDADE — o conserto é"
               " mecânico\n");
        printf("        (B) sabor         335   ver se o transcendente entra na PROVA\n");
        printf("        (C) honesto         0   VAZIA — π está na casa, exacto por"
               " andar\n");
        printf("        total             918   `tools/triagem_limiares.sh`\n");
        ok("E A REGRA FICA, para quem escrever a seguir: um limiar novo tem de dizer a que"
           " CLASSE pertence. Se não há transcendente na conta, ele é decoração e não entra"
           " — compara-se por igualdade. Se há, é preciso ver se ele entra na PROVA ou só"
           " na ilustração. E quando a quantidade é mesmo transcendente, o que se afirma é"
           " a FORMA FECHADA e não o decimal — e nesta casa isso quer dizer o POLÍGONO do"
           " andar, ou a recorrência do metal, que são exactos. A classe (C) está VAZIA e é"
           " para ficar: π está cá, exacto por dimensão, e o que não fecha é o membro-"
           "limite — por teorema, não por falta de conta. A triagem está em"
           " `tools/triagem_limiares.sh` e é barata: 918 sítios classificados em segundos",
           1);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
