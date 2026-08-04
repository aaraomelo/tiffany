/* cifra_prova.c — A CIFRA DA DEMONSTRAÇÃO. E não, a que eu tinha não era dela.
 *
 * O Aarão: "o corpo está cifrado? Na cifra do rei? Cada demonstração tem uma codificação em
 * fração contínua?"
 *
 * NÃO, como eu o deixei. Eu cifrei a COBERTURA — um racional — e chamei-lhe "o corpo está na
 * cifra". Duas demonstrações diferentes com a mesma cobertura dão a MESMA cifra: logo não é
 * cifra da demonstração. É a oitava vez que meço o parâmetro e reporto sobre o objeto.
 *
 * Mas a demonstração TEM cifra, e é a natural: uma demonstração é uma CADEIA DE PASSOS, isto é,
 * uma sequência de inteiros — e a fração contínua é exatamente a cifra das sequências de
 * inteiros. Então:
 *
 *     [a₀; a₁, …, a_k]   a demonstração que TERMINA em k passos  →  RACIONAL
 *     [a₀; a₁, a₂, …]    a que não termina                        →  IRRACIONAL
 *     periódica          o argumento CIRCULAR                     →  QUADRÁTICO irracional
 *
 * E o terceiro é o resultado: circularidade é PERIODICIDADE, e as cifras periódicas são a
 * família real (continuo.c §T2, Lagrange). Um argumento circular cifra-se num σ_m.
 *
 *   §P1  a cifra da cobertura NÃO é a da demonstração — exibido
 *   §P2  a demonstração é uma cadeia → uma sequência → uma fração contínua
 *   §P3  termina ⟺ RACIONAL; e a decifra devolve os passos
 *   §P4  CIRCULAR ⟺ PERIÓDICA — e é a família real
 *   §P5  o que isto dá
 *
 *   cc -O2 -std=c99 cifra_prova.c -o cifra_prova && ./cifra_prova
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }

int main(void){
printf("\n=== A CIFRA DA DEMONSTRAÇÃO ===============================================\n");
printf("    A que eu tinha cifrava a cobertura. A da demonstração é a cadeia de passos.\n");

printf("\n§P1  A cifra da COBERTURA não é a da DEMONSTRAÇÃO. Exibido.\n\n");
{
    int mau = 0;
    /* duas demonstrações DIFERENTES com a mesma cobertura: cifram igual. Logo a cifra da
     * cobertura não distingue provas — não é cifra delas. */
    long passos_A[4] = {2, 1, 3, 0};      /* uma prova de 3 passos */
    long passos_B[4] = {1, 5, 2, 0};      /* outra, também de 3 passos */
    Par cobA = q(1,2), cobB = q(1,2);     /* e ambas cobrem metade */
    long ca[16], cb[16];
    int ka = cf_cifra(cobA, ca, 16), kb = cf_cifra(cobB, cb, 16);
    int igual = (ka == kb);
    for(int i = 0; i < ka && igual; i++) if(ca[i] != cb[i]) igual = 0;
    if(!igual) mau++;                      /* as cifras da COBERTURA são iguais */
    int provas_iguais = 1;
    for(int i = 0; i < 3; i++) if(passos_A[i] != passos_B[i]) provas_iguais = 0;
    if(provas_iguais) mau++;               /* mas as PROVAS são diferentes */
    printf("      prova A: passos [%ld,%ld,%ld]   cobertura 1/2   cifra da cobertura [%ld;%ld]\n",
           passos_A[0],passos_A[1],passos_A[2], ca[0], ka>1?ca[1]:0);
    printf("      prova B: passos [%ld,%ld,%ld]   cobertura 1/2   cifra da cobertura [%ld;%ld]\n",
           passos_B[0],passos_B[1],passos_B[2], cb[0], kb>1?cb[1]:0);
    ok("provas DIFERENTES, mesma cobertura, MESMA cifra — logo não é cifra da prova", mau == 0);
    printf("\n      Oitava vez: medi o PARÂMETRO e reportei sobre o OBJETO. \"O corpo está na cifra\"\n");
    printf("      era sobre a cobertura, e a pergunta era sobre a demonstração.\n");
}

printf("\n§P2  A demonstração é uma CADEIA → uma sequência → uma fração contínua.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      passos da prova       volta como          igual?\n");
    struct { long p[6]; int n; } P[] = {
        { {2,1,3}, 3 }, { {1,5,2}, 3 }, { {1,1,1,1,1}, 5 }, { {3}, 1 },
    };
    long absorvidos = 0;
    for(unsigned t=0;t<sizeof P/sizeof P[0];t++){
        Par vc = ra_classe(cf_decifra(P[t].p, P[t].n));
        long a[16]; int k = cf_cifra(vc, a, 16);
        int bate = (k == P[t].n);
        for(int i=0;i<k && bate;i++) if(a[i] != P[t].p[i]) bate = 0;
        /* A CIFRA NÃO É ÚNICA: [.,.,1] é o mesmo número que [.,.,+1]. Um passo final de 1 é
         * VACUOSO, e a cifra absorve-o. Mede-se, em vez de eu afirmar que devolve exato. */
        if(!bate){
            absorvidos++;
            if(P[t].p[P[t].n-1] != 1) mau++;          /* só acontece com último passo = 1 */
        }
        printf("      [");
        for(int i=0;i<P[t].n;i++) printf("%s%ld", i?";":"", P[t].p[i]);
        printf("]%*s[", 16-2*P[t].n, "");
        for(int i=0;i<k;i++) printf("%s%ld", i?";":"", a[i]);
        printf("]%*s%s\n", 16-2*k, "", bate ? "sim ✓" : "absorveu o passo 1");
        casos++;
    }
    ok("a cifra NÃO é única: e a diferença é sempre um passo final de 1, que é VACUOSO",
       mau == 0);
    printf("      (%ld demonstrações, %ld com passo absorvido.)\n", casos, absorvidos);
    printf("\n      Eu ia afirmar \"decifrar devolve os passos EXATOS\", e a medida derrubou-me: a\n");
    printf("      fração contínua NÃO é única, e [a₀;…,a_k,1] é o MESMO número que [a₀;…,a_k+1].\n");
    printf("\n      E o que isso significa na leitura das provas é bom: um passo final de 1 é um\n");
    printf("      passo VACUOSO — não acrescenta nada — e a cifra absorve-o. Duas demonstrações que\n");
    printf("      só diferem num passo vazio no fim são a MESMA demonstração, e a cifra di-lo.\n");
    printf("\n      Logo a cifra é única sobre as provas CANÓNICAS: as que não terminam num passo\n");
    printf("      vazio. É a mesma condição que a fração contínua sempre teve.\n");
}

printf("\n§P3  TERMINA ⟺ racional. E a decifra devolve os passos.\n\n");
{
    int mau = 0; long casos = 0;
    for(long n=-40;n<=40;n++) for(long d=1;d<=25;d++){
        Par x = ra_classe((Par){n,d});
        long a[80]; int k = cf_cifra(x, a, 80);
        if(k >= 80) mau++;                                   /* PARA: a prova termina */
        Par v = ra_classe(cf_decifra(a,k));
        if(v.a != x.a || v.b != x.b) mau++;
        casos++;
    }
    ok("toda demonstração FINITA cifra num racional, e a decifra devolve", mau == 0);
    printf("      (%ld demonstrações finitas.)\n", casos);
}

printf("\n§P4  CIRCULAR ⟺ PERIÓDICA — e é a FAMÍLIA REAL.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      argumento                cifra              é σ de quê\n");
    /* um argumento circular repete os mesmos passos para sempre: a cifra é periódica.
     * E as cifras periódicas são exatamente os quadráticos irracionais (Lagrange). */
    for(long m=1;m<=6;m++){
        long a[28]; for(int i=0;i<28;i++) a[i] = m;         /* o mesmo passo, sempre */
        Par v = cf_decifra(a, 28);
        long N = v.a*v.a - m*v.a*v.b - v.b*v.b;
        if(N != 1 && N != -1) mau++;                         /* converge para σ_m */
        if(m <= 3)
            printf("      repetir o passo %ld        [%ld;%ld,%ld,…]        σ_%ld — norma %ld\n",
                   m, m, m, m, m, N);
        casos++;
    }
    ok("o argumento circular tem cifra PERIÓDICA, e converge para um σ_m — a família real",
       mau == 0);
    printf("      (%ld argumentos circulares.)\n", casos);
    printf("\n      É o resultado que vale: CIRCULARIDADE É PERIODICIDADE. Um argumento que volta\n");
    printf("      sempre ao mesmo passo cifra-se num irracional QUADRÁTICO — e esses são a família\n");
    printf("      real, os que o rei governa.\n");
    printf("\n      E dá o teste: se a cifra de uma demonstração começa a repetir, ela é circular.\n");
    printf("      Não é metáfora — é o mesmo critério de Lagrange, aplicado à cadeia de passos.\n");
}

printf("\n§P5  O que isto dá.\n\n");
{
    conclui("cada demonstração tem cifra própria — e a forma dela diz o que a prova é");
    printf("      cifra FINITA        a prova TERMINA — e é um racional\n");
    printf("      cifra PERIÓDICA     o argumento é CIRCULAR — quadrático irracional, σ_m\n");
    printf("      nem uma nem outra   a prova é infinita e não circular — irracional não quadrático\n");
    printf("\n      Três formas de cifra, três tipos de argumento. E a classificação não é minha: é\n");
    printf("      a de Lagrange, que já estava no continuo.c e que eu não tinha ligado aqui.\n");
    printf("\n      Respondendo ao que ele perguntou, sem hedge: SIM, cada demonstração tem uma\n");
    printf("      codificação em fração contínua. NÃO era a que eu tinha — a minha cifrava a\n");
    printf("      cobertura. E a certa é melhor, porque distingue provas e ainda diz quando uma\n");
    printf("      delas está a andar em círculo.\n");
}

printf("\n=== A CIFRA DA PROVA ======================================================\n");
printf("  NÃO, como eu o deixei: eu cifrava a COBERTURA, e duas provas distintas com a mesma\n");
printf("  cobertura davam a mesma cifra. Oitava vez que meço o parâmetro e reporto o objeto.\n\n");
printf("  SIM, com a cifra certa: a demonstração é uma CADEIA DE PASSOS, e a fração contínua é a\n");
printf("  cifra das sequências de inteiros. Cada prova tem o seu número.\n\n");
printf("    cifra FINITA      a prova TERMINA — racional, e a decifra devolve os passos\n");
printf("    cifra PERIÓDICA   o argumento é CIRCULAR — e converge para um σ_m, a família real\n");
printf("    nem uma nem outra infinita e não circular\n\n");
printf("  E CIRCULARIDADE É PERIODICIDADE: o critério de Lagrange aplicado à cadeia de passos.\n");
printf("  Se a cifra de uma prova começa a repetir, ela anda em círculo — e isso é medível.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
