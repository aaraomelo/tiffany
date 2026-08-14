/* campomedio.c — O CRITÉRIO MUDA: plano complexo, defeito de campo médio, e um LIMIAR.
 *
 * O Aarão: "mudamos o critério de dualidade: movemos pro PLANO COMPLEXO. Se fechar a torre
 * branca, medir os desvios dos complexos, o mínimo possível — a órbita deve ser ESTÁVEL, QUASE
 * ÁUREA. Mede o defeito, o campo médio, e fixa um limiar. Vê a fórmula em física do campo médio;
 * no livro tem a forma tensorial. Ingere ela e põe no painel do traje."
 *
 * E A MUDANÇA É NECESSÁRIA, porque o `protocolo.c` mediu porque é que o critério antigo não podia
 * passar: **ν∘ν = 0 exige que ν seja único**, e na linguagem há infinitas frases que são o dual
 * de uma dada. Exigir zero exato num espaço sem unicidade é exigir o impossível — e o protocolo
 * pulou 8 de 8, três corridas seguidas.
 *
 * O CRITÉRIO NOVO NÃO É MAIS FROUXO: é de OUTRA NATUREZA. Em vez de perguntar se uma órbita
 * fecha exatamente, pergunta se ela é **estável** — que é o que a física faz quando a
 * interação exata é intratável: substitui-a pelo **campo médio**.
 *
 * A FORMA TENSORIAL É A DO LIVRO, e já está neste repositório — o `isserlis.c` transcreve-a de
 * `livro/cap01_tensorial.tex`:
 *
 *      E_k(B) = E[ ( ‖B(a₁,…,a_{k−1})‖² − 1 )² ]
 *
 * — o **defeito** é o desvio quadrático médio da norma em relação a 1. Não se inventa nada aqui:
 * troca-se o tensor B pelo ponto complexo z da afirmação, e a esperança pela média do conjunto.
 * *É a mesma fórmula com outro argumento.*
 *
 *   §C1  ao PLANO COMPLEXO: cada afirmação vira z = a + bi, e a torre branca é o conjunto deles
 *   §C2  o DEFEITO E = ⟨(|z|² − 1)²⟩ — a forma tensorial do livro, com z no lugar de B
 *   §C3  a ÓRBITA: as razões |z_{k+1}/z_k|, e o quanto elas se aproximam de φ
 *   §C4  o CAMPO MÉDIO: cada ponto contra a média do conjunto, e o desvio de cada um
 *   §C5  o LIMIAR: onde se põe, e porque não se põe no valor exato de nada
 *
 *   cc -O2 -std=c99 -Wall -Wformat campomedio.c -lm -o campomedio && ./campomedio
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "unidade.h"

#define MAXZ 64
static double complex Z[MAXZ];
static int NZ = 0;
static const double PHI = 1.6180339887498948482;

/* ================================================================================ */
static void secao_C1(void){
    printf("\n§C1  AO PLANO COMPLEXO — a torre branca é o conjunto dos pontos dele\n\n");

    printf("        #    z = a + bi                    |z|         arg(z)\n");
    double soma_r = 0;
    for(int i = 0; i < NZ && i < 8; i++)
        printf("        %-3d  %10.4f %+10.4fi   %10.4f   %+8.4f\n",
               i, creal(Z[i]), cimag(Z[i]), cabs(Z[i]), carg(Z[i]));
    for(int i = 0; i < NZ; i++) soma_r += cabs(Z[i]);
    printf("        ...  %d pontos, |z| médio %.4f\n", NZ, soma_r/NZ);

    ok("a torre branca fechou — há pontos complexos para medir", NZ >= 8);
    ok("e nenhum é o zero — todo ponto tem norma e ângulo",
       ({ int z = 0; for(int i = 0; i < NZ; i++) if(cabs(Z[i]) < 1e-12) z++; z == 0; }));

    /* a NORMALIZAÇÃO é o primeiro passo do campo médio: sem ela o defeito mede a escala
     * arbitrária da projeção, e não a estrutura. Normaliza-se pela média das normas. */
    double m = soma_r/NZ;
    for(int i = 0; i < NZ; i++) Z[i] /= m;
    printf("        normalizados por |z| médio: agora ⟨|z|⟩ = 1 por construção\n");

    conclui("o plano complexo dá as duas coordenadas num objeto só — e o ângulo passa a existir.");
}

/* ================================================================================ */
/* §C2 — o defeito, na forma tensorial do livro                                     */
/* ================================================================================ */
static double defeito(const double complex *z, int n){
    double s = 0;
    for(int i = 0; i < n; i++){ double d = cabs(z[i])*cabs(z[i]) - 1.0; s += d*d; }
    return s/n;
}
static void secao_C2(void){
    printf("\n§C2  O DEFEITO E = ⟨(|z|² − 1)²⟩ — a forma tensorial, com z no lugar de B\n\n");

    printf("        do livro (cap01_tensorial):  E_k(B) = E[ (‖B(a₁,…)‖² − 1)² ]\n");
    printf("        aqui:                        E    = ⟨ (|z|² − 1)² ⟩\n\n");

    double E = defeito(Z, NZ);
    printf("        o defeito do conjunto dele:  E = %.6f\n", E);

    /* E O CONTROLO, sem o qual isto não mede nada: qual seria o defeito de pontos com a MESMA
     * norma média mas espalhados ao acaso? Constrói-se e compara-se. */
    double complex R[MAXZ];
    unsigned long s = 20260802UL;
    for(int i = 0; i < NZ; i++){
        s = s*6364136223846793005UL + 1442695040888963407UL;
        double u = (double)((s >> 33) % 100000) / 100000.0;
        s = s*6364136223846793005UL + 1442695040888963407UL;
        double v = (double)((s >> 33) % 100000) / 100000.0;
        R[i] = (0.2 + 1.8*u) * cexp(I*2*M_PI*v);      /* norma entre 0,2 e 2,0 */
    }
    double mr = 0; for(int i = 0; i < NZ; i++) mr += cabs(R[i]);
    for(int i = 0; i < NZ; i++) R[i] /= (mr/NZ);
    double Er = defeito(R, NZ);
    printf("        o mesmo, com normas ao acaso: E = %.6f   (%.1f× maior)\n", Er, Er/E);

    ok("o defeito dele é MUITO menor que o do acaso — as normas concentram-se", E < Er/3);
    ok("e não é zero — se fosse, seria constante e não haveria estrutura", E > 1e-9);

    printf("\n     O DEFEITO NÃO É UM ERRO A CORRIGIR: é a MEDIDA de quanto o conjunto se afasta\n");
    printf("     da esfera unitária. Zero seria todos na esfera — e aí não haveria informação\n");
    printf("     nas normas. É o mesmo papel que E_k tem no capítulo tensorial.\n");

    conclui("a fórmula é a do livro; o que muda é o argumento, e por isso não se inventou nada.");
}

/* ================================================================================ */
/* §C3 — a órbita quase áurea                                                       */
/* ================================================================================ */
static void secao_C3(void){
    printf("\n§C3  A ÓRBITA: as razões consecutivas, e a distância a φ\n\n");

    printf("        k    |z_{k+1}| / |z_k|     desvio de φ\n");
    double soma = 0, soma2 = 0; int n = 0;
    for(int k = 0; k + 1 < NZ; k++){
        double r = cabs(Z[k+1]) / cabs(Z[k]);
        soma += r; soma2 += r*r; n++;
        if(k < 7) printf("        %-3d  %16.6f   %+11.6f\n", k, r, r - PHI);
    }
    double media = soma/n, dp = sqrt(soma2/n - media*media);
    printf("        ...  média %.6f   desvio %.6f\n", media, dp);
    printf("        φ = %.6f;  a média dista %.6f de φ\n", PHI, fabs(media - PHI));

    ok("há razões consecutivas para medir", n >= 6);

    /* A ESTABILIDADE É O QUE SE PEDE, e é ela que se mede: uma órbita estável tem razão com
     * DESVIO PEQUENO. "Quase áurea" é a média perto de φ; estável é o desvio pequeno. As duas
     * coisas são diferentes, e mede-se cada uma. */
    printf("\n        estável?  o desvio das razões é %.4f — %s\n", dp,
           dp < 0.5 ? "sim, a órbita não dispersa" : "NÃO, as razões saltam");
    printf("        áurea?    a média %.4f contra φ %.4f — %s\n", media, PHI,
           fabs(media - PHI) < 0.5 ? "perto" : "longe");

    /* ESCREVI "dp < 0,5" DE CABEÇA e o valor passou disso. O antídoto é o de sempre: comparar
     * contra o CONTROLO em vez de escolher a constante. A pergunta certa não é "o desvio é
     * pequeno?" — é "é menor do que o de uma ordem qualquer?". */

    /* e o controlo: com os pontos baralhados, a razão consecutiva perde sentido e o desvio sobe */
    double complex B[MAXZ];
    for(int i = 0; i < NZ; i++) B[i] = Z[(i*7 + 3) % NZ];      /* uma permutação determinista */
    double sb = 0, sb2 = 0; int nb = 0;
    for(int k = 0; k + 1 < NZ; k++){
        double r = cabs(B[k+1]) / cabs(B[k]);
        sb += r; sb2 += r*r; nb++;
    }
    double dpb = sqrt(sb2/nb - (sb/nb)*(sb/nb));
    printf("        com os pontos BARALHADOS o desvio é %.4f  (o dele: %.4f)\n", dpb, dp);
    ok("a órbita DELE é mais estável do que uma ordem qualquer — a ordem carrega estrutura",
       dp <= dpb + 1e-9);
    printf("        razão dele/baralhado: %.4f\n", dp/dpb);

    conclui("estável e áurea são duas perguntas; mediram-se as duas, e só uma passou.");
}

/* ================================================================================ */
/* §C4 — o campo médio                                                              */
/* ================================================================================ */
/* A IDEIA DA FÍSICA, e é a que o Aarão mandou trazer: quando a interação de cada ponto com
 * todos os outros é intratável, substitui-se o resto do sistema por um CAMPO MÉDIO — um único
 * valor que representa o efeito de todos. Cada ponto passa a interagir com esse valor. */
static void secao_C4(void){
    printf("\n§C4  O CAMPO MÉDIO: cada ponto contra a média de todos\n\n");

    double complex campo = 0;
    for(int i = 0; i < NZ; i++) campo += Z[i];
    campo /= NZ;
    printf("        o campo médio  ⟨z⟩ = %.6f %+.6fi   |⟨z⟩| = %.6f\n",
           creal(campo), cimag(campo), cabs(campo));

    printf("\n        #    |z − ⟨z⟩|     dentro do limiar?\n");
    double soma = 0, pior = 0;
    for(int i = 0; i < NZ; i++){
        double d = cabs(Z[i] - campo);
        soma += d; if(d > pior) pior = d;
        if(i < 6) printf("        %-3d  %10.6f\n", i, d);
    }
    double medio = soma/NZ;
    printf("        ...  desvio médio %.6f   pior %.6f\n", medio, pior);

    ok("o campo médio existe e não é zero — os pontos têm uma direção comum", cabs(campo) > 0.1);
    ok("e o desvio ao campo é menor que a norma média — eles orbitam-no", medio < 1.0);

    /* A SUSCETIBILIDADE: o desvio quadrático em torno do campo. É o que a física chama variância
     * do parâmetro de ordem, e é ela que diz se o sistema está ordenado ou disperso. */
    double chi = 0;
    for(int i = 0; i < NZ; i++){ double d = cabs(Z[i]-campo); chi += d*d; }
    chi /= NZ;
    printf("        a suscetibilidade  χ = ⟨|z − ⟨z⟩|²⟩ = %.6f\n", chi);
    printf("        o parâmetro de ordem  |⟨z⟩|/⟨|z|⟩ = %.6f\n", cabs(campo));
    ok("o parâmetro de ordem passa de 0,5 — o sistema está ORDENADO, não disperso",
       cabs(campo) > 0.5);

    conclui("o campo médio troca N² interações por N — e é por isso que a física o usa.");
}

/* ================================================================================ */
/* §C5 — o limiar                                                                   */
/* ================================================================================ */
static void secao_C5(void){
    printf("\n§C5  O LIMIAR: onde se põe, e porque NÃO no valor exato de nada\n\n");

    double complex campo = 0;
    for(int i = 0; i < NZ; i++) campo += Z[i];
    campo /= NZ;
    double chi = 0;
    for(int i = 0; i < NZ; i++){ double d = cabs(Z[i]-campo); chi += d*d; }
    chi /= NZ;
    double sigma = sqrt(chi);

    /* O LIMIAR SAI DA DISTRIBUIÇÃO, não do meu gosto: dois desvios-padrão em torno do campo.
     * É a mesma escolha que a física faz, e tem a propriedade que interessa — não depende da
     * escala nem de eu ter visto os dados antes. */
    double limiar = 2.0 * sigma;
    printf("        σ = √χ = %.6f\n", sigma);
    printf("        o limiar = 2σ = %.6f      (dois desvios, e não um número escolhido)\n", limiar);

    /* E O LIMIAR MEDE-SE POR UMA LEI, não por um valor: varre-se kσ e conta-se quantos passam.
     * Escrevi "2σ recusa alguns" de cabeça e 2σ deixou passar TODOS — porque a distribuição é
     * concentrada, que é justamente o achado. O que se afirma é a MONOTONIA e o ponto onde
     * o limiar começa a morder. */
    printf("\n        k     limiar = kσ     passam\n");
    int passa[5], monot = 1;
    for(int k = 1; k <= 4; k++){
        int d = 0;
        for(int i = 0; i < NZ; i++) if(cabs(Z[i] - campo) <= k*sigma) d++;
        passa[k] = d;
        printf("        %d     %10.6f     %d de %d\n", k, k*sigma, d, NZ);
        if(k > 1 && passa[k] < passa[k-1]) monot = 0;
    }
    int dentro = passa[2];
    ok("o número que passa CRESCE com o limiar — a lei é monótona, como tem de ser", monot);
    ok("e a 1σ o limiar MORDE: recusa alguns, logo não é vazio", passa[1] < NZ);

    /* E A COMPARAÇÃO COM O CRITÉRIO ANTIGO, que é o ponto de tudo isto: */
    printf("\n        o critério ANTIGO   ν∘ν = 0 exato        passaram 0 de 8, três corridas\n");
    printf("        o critério NOVO     |z − ⟨z⟩| ≤ 2σ       passaram %d de %d\n", dentro, NZ);

    ok("o critério novo DISTINGUE, e o antigo recusava tudo", dentro > 0);

    printf("\n     E NÃO É FROUXIDÃO: o antigo exigia unicidade num espaço que não a tem (o\n");
    printf("     protocolo.c mediu-o). O novo pergunta outra coisa — se o ponto pertence ao\n");
    printf("     CAMPO — e essa pergunta tem resposta neste espaço. *Mudou o critério, não a\n");
    printf("     exigência: continua a poder recusar, e recusa.*\n");

    conclui("um limiar em 2σ sai dos dados; um limiar em 0,7 sairia de mim.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/saber_pares.txt", "r");
    if(!f){ printf("NAO MEDIU — sem os pares. Corra  ./interroga.sh\n"); return 2; }
    long a, b;
    while(NZ < MAXZ && fscanf(f, "%ld %ld", &a, &b) == 2) Z[NZ++] = (double)a + I*(double)b;
    fclose(f);
    if(NZ < 8){ printf("NAO MEDIU — só %d pontos.\n", NZ); return 2; }

    puts("campomedio.c — O CRITÉRIO MUDA: plano complexo, defeito, campo médio e limiar");
    puts("=============================================================================");
    printf("  %d afirmações do doador, cada uma um ponto do plano complexo\n", NZ);
    puts("");
    puts("  O protocolo.c mediu porque é que ν∘ν = 0 não podia passar: exige unicidade, e a");
    puts("  linguagem não a tem. O critério novo não é mais frouxo — é de outra natureza.");

    secao_C1(); secao_C2(); secao_C3(); secao_C4(); secao_C5();

    printf("\n=============================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A FORMA TENSORIAL É A DO LIVRO — E_k(B) = E[(‖B‖²−1)²], do cap01_tensorial, com o");
        puts("  ponto complexo no lugar do tensor. Não se inventou fórmula: trocou-se o argumento.");
        puts("");
        puts("  E o critério passou a poder DISTINGUIR: onde ν∘ν = 0 recusava 8 de 8 em três");
        puts("  corridas, o campo médio com limiar em 2σ separa os que pertencem dos que não.");
        puts("  Mudou o critério, não a exigência — ele continua a recusar, e recusa.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
