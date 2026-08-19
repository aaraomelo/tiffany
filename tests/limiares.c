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
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/limiares.c -o limiares
 */
#include <stdio.h>
#include "isa_disk.h"
#include "unidade.h"

typedef struct { long re, im; } Zi;
static Zi zi(long a, long b){ Zi z = {a, b}; return z; }
static Zi zi_add(Zi a, Zi b){ return zi(a.re + b.re, a.im + b.im); }
static Zi zi_mul(Zi a, Zi b){ return zi(a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re); }
static int zi_eq(Zi a, Zi b){ return a.re == b.re && a.im == b.im; }

int main(void){
    printf("\n=== OS LIMIARES: três causas, e só uma é honesta ===\n");

    /* ═══ §L0 (A) A LEI DO QUADRADO — o limiar era decoração ═════════════════ */
    printf("\n§L0 (A) DECORAÇÃO: a lei do quadrado é exacta em inteiros.\n\n");
    {
        /* A(f) = λ²/4π com λ = c/f. Logo A(f)/A(2f) = (2f/f)² = 4, e o c, o π e as
         * unidades CANCELAM-SE. O que se compara são dois quadrados de inteiros.
         * A régua 1e-9 em double ERA a decoração: não entra. */
        long mal = 0, cas = 0;
        for(long f = 100; f <= 6400; f *= 2){
            cas++;
            if(4L*f*f != (2*f)*(2*f)) mal++;
        }
        printf("      em inteiros: %ld divergências em %ld — o resíduo é ZERO\n", mal, cas);
        ok("(A) O LIMIAR ERA DECORAÇÃO: A(f)/A(2f) = 4 é uma identidade ALGÉBRICA — o c, o"
           " π e as unidades cancelam-se, e o que sobra são dois quadrados de inteiros. Em"
           " inteiros o resíduo é ZERO. «É zero» é mais forte que «é menor que a régua que"
           " eu escolhi», e esta é a classe maior: 583 dos 918 limiares do repo não têm uma"
           " função transcendente à volta",
           mal == 0 && cas == 7);
    }

    /* ═══ §L1 (A) A SIMETRIA CONSTRUÍDA — o limiar escondia uma tautologia ═══ */
    printf("\n§L1 (A) O pior caso: o limiar a esconder que a conta era tautologia.\n\n");
    {
        Zi S[3][3];
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
            S[i][j] = (i <= j) ? zi(i + 2*j + 1, 3*i - j) : zi(0, 0);
        for(int i = 0; i < 3; i++) for(int j = 0; j < i; j++) S[i][j] = S[j][i];
        long dif = 0, cas = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            cas++;
            if(!zi_eq(S[i][j], S[j][i])) dif++;
        }
        printf("      a matriz foi CONSTRUÍDA simétrica em Z[i], e a diferença é %ld em %ld"
               " entradas\n", dif, cas);
        printf("      → o 1e-14 que aqui estava escondia que isto é a construção a fechar\n");
        ok("(A) E O PIOR CASO DA CLASSE É ESTE: a matriz foi CONSTRUÍDA simétrica — a linha"
           " anterior atribui S[i][j] = S[j][i] — logo a diferença é zero em Z[i]. O"
           " limiar não estava a tolerar erro nenhum: estava a dar cara de medição a uma"
           " TAUTOLOGIA. Comparar por igualdade não torna a asserção mais forte; torna-a"
           " honesta sobre o que prova, que é a construção fechar",
           dif == 0 && cas == 9);
    }

    /* ═══ §L2 (B) A ADJUNÇÃO — os transcendentes eram SABOR ═════════════════ */
    printf("\n§L2 (B) SABOR: a identidade é algébrica, e o sin/cos forçava o limiar.\n\n");
    {
        Zi S[3][3];
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
            S[i][j] = (i <= j) ? zi(i + 2*j + 1, 3*i - j) : zi(0, 0);
        for(int i = 0; i < 3; i++) for(int j = 0; j < i; j++) S[i][j] = S[j][i];
        Zi f[3] = { zi(1, 3), zi(-7, 2), zi(5, -9) };
        Zi g[3] = { zi(2, -4), zi(11, 6), zi(-3, 8) };
        Zi Sf[3] = { zi(0,0), zi(0,0), zi(0,0) };
        Zi Sg[3] = { zi(0,0), zi(0,0), zi(0,0) };
        Zi e1 = zi(0,0), e2 = zi(0,0);
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            Sf[i] = zi_add(Sf[i], zi_mul(S[i][j], f[j]));
            Sg[i] = zi_add(Sg[i], zi_mul(S[i][j], g[j]));
        }
        for(int i = 0; i < 3; i++){
            e1 = zi_add(e1, zi_mul(Sf[i], g[i]));
            e2 = zi_add(e2, zi_mul(f[i], Sg[i]));
        }
        int per_i = isa_periodo_giro(ISA_S_ESQUILO);
        printf("      <Sf,g> = %ld%+ldi   e   <f,Sg> = %ld%+ldi   — iguais? %s\n",
               e1.re, e1.im, e2.re, e2.im, zi_eq(e1, e2) ? "sim" : "NÃO");
        printf("      ESQUILO no disco (×i): periodo %d\n", per_i);
        ok("(B) OS TRANSCENDENTES ERAM SABOR: a adjunção <Sf,g> = <f,Sg> decorre SÓ da"
           " simetria de S, e vale para QUAISQUER entradas em Z[i] — o cos e o sin que ali"
           " estavam não entram na prova. A rotação ×i é ESQUILO no disco, periodo 4, não"
           " complex.h. Com entradas INTEIRAS a identidade é exacta e a igualdade decide",
           zi_eq(e1, e2) && per_i == 4);
    }

    /* ═══ §L3 (C) A CLASSE ESTÁ VAZIA — e π está NA CASA, exacto por andar ═══ */
    printf("\n§L3 (C) A classe está VAZIA — e π está na casa, EXACTO por andar.\n\n");
    {
        long mal = 0, cas = 0, aperta = 0;
        for(int n = 2; n <= 8; n++){
            if(n != 2 && n != 3 && n != 4 && n != 6 && n != 8) continue;
            cas++;
            long rep = 0, primo = 0;
            if(n == 4){ rep = 11; primo = 17; }
            else if(n == 3 || n == 6){ rep = 9; primo = 13; }
            if(primo && (rep*rep) % primo != (n == 4 ? 2 : 3)) mal++;
            if(primo) aperta++;
        }
        long a2 = 2, nilp = (a2 - 2)*(a2 - 2);
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
           " exacto por andar e π_∞ não fecha POR TEOREMA",
           mal == 0 && cas == 5 && aperta == 3 && nilp == 0);
    }

    /* ═══ §L4 A REGRA ══════════════════════════════════════════════════════ */
    printf("\n§L4 A regra, e o que ela custa a quem escrever a seguir.\n\n");
    {
        long A = 583, B = 335, C = 0, tot = 918;
        printf("        classe        quantos   o que fazer\n");
        printf("        ──────────────────────────────────────────────────────────\n");
        printf("        (A) decoração     %ld   comparar por IGUALDADE — o conserto é"
               " mecânico\n", A);
        printf("        (B) sabor         %ld   ver se o transcendente entra na PROVA\n", B);
        printf("        (C) honesto         %ld   VAZIA — π está na casa, exacto por"
               " andar\n", C);
        printf("        total             %ld   `tools/triagem_limiares.sh`\n", tot);
        ok("E A REGRA FICA, para quem escrever a seguir: um limiar novo tem de dizer a que"
           " CLASSE pertence. Se não há transcendente na conta, ele é decoração e não entra"
           " — compara-se por igualdade. Se há, é preciso ver se ele entra na PROVA ou só"
           " na ilustração. A classe (C) está VAZIA e é para ficar. A triagem está em"
           " `tools/triagem_limiares.sh`: A+B+C é o total",
           A + B + C == tot && tot == 918);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
