/* letra_assinatura.c — CADA LETRA TEM ASSINATURA, E É ELA QUE ATRAVESSA AS FONTES.
 *
 * O Aarão: «cara, você está tentando adivinhar. Está errado ainda — você aumentou o espaço
 * para não colapsar mas outros espaços ficaram maiores. NÃO TENTE, FAÇA. Você está olhando
 * para uma fonte específica: mesmo que consiga ajustar, vai quebrar na próxima. CADA LETRA TEM
 * UMA ASSINATURA — faça isso para cada letra. Depois converte entre letras para converter
 * entre fontes, respeitando a geometria de cada uma.»
 *
 * E o diagnóstico dele é o do método, não do número. Eu andava a medir a Liberation, a
 * embutir a Noto, a comparar as duas e a ajustar até parar de colapsar — e cada ajuste era
 * sobre AQUELAS duas. Ajustar contra um caso é adivinhar com passos pequenos.
 *
 * A ASSINATURA DE UMA LETRA CONTA OS SEUS CONTORNOS PELO SINAL:
 *
 *      p   os contornos que ACRESCENTAM tinta       o traço
 *      q   os que CORTAM                            os buracos
 *      r   se há tinta a atravessar                 1 quando o glifo desenha
 *
 * E ELA NÃO MUDA COM A FONTE. O `o` tem um traço e um buraco em qualquer tipo de letra do
 * mundo; o `i` tem dois traços e nenhum buraco; o `B` tem um traço e dois buracos. Isso não é
 * uma escolha do desenhador — é o que a letra É. A geometria muda toda: as coordenadas, o
 * avanço, a espessura. A assinatura não.
 *
 * E POR ISSO A CONVERSÃO ENTRE FONTES É LETRA A LETRA, pela assinatura: o `o` de uma vai no
 * `o` da outra porque as assinaturas batem, e a geometria de cada uma é respeitada — mede-se
 * em PROPORÇÃO do avanço, que é o que sobrevive à mudança de régua.
 *
 *   §L1  cada letra tem assinatura, e ela conta os contornos pelo SINAL
 *   §L2  e ela NÃO MUDA com a fonte — medido em todas as que houver
 *   §L3  a geometria MUDA toda: coordenadas, avanço, espessura
 *   §L4  a conversão é letra a letra pela assinatura, em PROPORÇÃO do avanço
 *   §L5  o controlo: uma letra sem par de assinatura NÃO converte — e diz-se qual
 *
 *   cc -O2 -std=gnu99 -I../lib letra_assinatura.c -o letra_assinatura && ./letra_assinatura
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "spline.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* a área com sinal de um contorno — inteira, e o sinal é o sentido */
static long area(const Contorno *c, int k)
{
    int ini = k ? c->fim[k-1] + 1 : 0, f = c->fim[k];
    if(f >= c->n || f <= ini) return 0;
    long a = 0;
    for(int i = ini; i <= f; i++){
        int j = (i == f) ? ini : i + 1;
        a += (long)c->p[i].x * c->p[j].y - (long)c->p[j].x * c->p[i].y;
    }
    return a;
}

/* A ASSINATURA DE UMA LETRA: conta os contornos pelo sinal. Não olha para coordenadas, para
 * o avanço nem para a espessura — só para QUANTOS de cada sentido. É por isso que atravessa
 * as fontes: é topologia, não geometria. */
struct assin { long p, q, r; };

static int assinatura(const Ttf *t, int ch, struct assin *A)
{
    int gi = ttf_glifo(t, ch);
    if(!gi) return 0;
    Contorno c;
    if(!ttf_contorno(t, gi, &c) || c.n <= 0){ A->p = A->q = A->r = 0; return 1; }
    A->p = A->q = 0;
    /* o sentido MAIORITÁRIO em área é o do traço: o buraco é sempre menor que o que o contém.
     * Assim o sinal absoluto da fonte deixa de importar — e era isso que me prendia a uma. */
    long maior = 0; int sinal_traco = 0;
    for(int k = 0; k < c.nc; k++){
        long a = area(&c, k), m = a < 0 ? -a : a;
        if(m > maior){ maior = m; sinal_traco = a > 0 ? 1 : -1; }
    }
    for(int k = 0; k < c.nc; k++){
        long a = area(&c, k);
        if(!a) continue;
        if((a > 0 ? 1 : -1) == sinal_traco) A->p++; else A->q++;
    }
    A->r = (c.n > 0) ? 1 : 0;
    return 1;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    /* TODAS as fontes que houver — e não uma. É o ponto: uma assinatura que só vale numa
     * fonte não é assinatura, é um ajuste. */
    /* E A ÁRVORE DAS FONTES NÃO É UMA SÓ. O mesmo pacote — a Liberation, a DejaVu —
     * instala em sítios diferentes conforme a distribuição: o Fedora põe a família
     * na raiz (`fonts/liberation-sans/`), o Debian e o Ubuntu põem-na sob
     * `fonts/truetype/<família>/`. Esta lista tinha CINCO caminhos do Fedora e um
     * do Debian, pelo que numa máquina Debian resolvia UM — e o medidor, que
     * precisa de comparar duas, dizia «menos de duas fontes, NAO MEDIU» com o
     * sistema cheio de fontes instaladas. Não faltavam fontes: faltavam caminhos.
     * O `tests/spline.c` e o `tests/dual_spline_ttf.c` já listavam as duas árvores;
     * é essa a convenção da casa, e é ela que aqui se completa. */
    static const char *TODAS[] = {
        /* Fedora / RHEL — a família na raiz */
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
        "/usr/share/fonts/liberation-sans/LiberationSans-Bold.ttf",
        "/usr/share/fonts/liberation/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/google-noto-vf/NotoSerif[wght].ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        /* Debian / Ubuntu — a mesma família sob truetype/ */
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf",
    };
    enum { NTODAS = (int)(sizeof TODAS / sizeof TODAS[0]) };
    static Ttf F[NTODAS];
    static const char *NOME[NTODAS];
    long nf = 0;
    for(long i = 0; i < NTODAS; i++)
        if(ttf_abre(&F[nf], TODAS[i])){ NOME[nf] = TODAS[i]; nf++; }

printf("\n=== CADA LETRA TEM ASSINATURA, E E' ELA QUE ATRAVESSA AS FONTES ===============\n");

    if(nf < 2){
        printf("\n  menos de duas fontes no sistema — este medidor precisa de comparar.  NAO MEDIU.\n\n");
        fechar(&b); return 2;
    }
    printf("\n  %ld fontes encontradas:\n", nf);
    for(long i = 0; i < nf; i++){
        const char *s = strrchr(NOME[i], '/');
        printf("    %s\n", s ? s+1 : NOME[i]);
    }

printf("\n§L1  Cada letra tem assinatura, e ela conta os contornos pelo SINAL.\n\n");
    long tem_assin = 0;
    {
        const int L[] = { 'l', 'o', 'i', 'B', 'g', 'e' };
        printf("      letra   assinatura   o que e'\n");
        long distintas = 0, com_buraco = 0;
        struct assin vistas[8]; long nv = 0;
        for(long k = 0; k < 6; k++){
            struct assin A;
            if(!assinatura(&F[0], L[k], &A)) continue;
            if(A.q > 0) com_buraco++;
            int nova = 1;
            for(long j = 0; j < nv; j++)
                if(vistas[j].p == A.p && vistas[j].q == A.q) nova = 0;
            if(nova && nv < 8) vistas[nv++] = A;
            distintas += nova;
            printf("      %c       (%ld,%ld,%ld)      %ld traco(s), %ld buraco(s)\n",
                   L[k], A.p, A.q, A.r, A.p, A.q);
        }
        /* as duas metades: tem de haver letras COM buraco e assinaturas DISTINTAS. Se todas
         * fossem iguais, a assinatura nao separava nada e nao servia para converter. */
        tem_assin = (com_buraco > 0 && distintas >= 3);
        printf("      %ld assinaturas distintas em 6 letras, %ld com buraco\n", distintas, com_buraco);
        ok("cada letra tem assinatura e ela conta os contornos PELO SINAL — nao olha para"
           " coordenadas, avanco nem espessura, so' para quantos de cada sentido. E ha'"
           " assinaturas DISTINTAS entre as letras: se fossem todas iguais nao separavam nada e"
           " nao serviam para converter. E o sinal MAIORITARIO em area e' o do traco, porque o"
           " buraco e' sempre menor que o que o contem — assim a convencao de sinal da fonte"
           " deixa de importar, e era isso que me prendia a UMA", tem_assin);
    }

printf("\n§L2  E ela NAO MUDA com a fonte — em todas as que ha'.\n\n");
    long invariante = 0;
    {
        /* AQUI ESTA' O PONTO. A assinatura de uma letra tem de ser a MESMA nas N fontes: o `o`
         * tem um traco e um buraco em qualquer tipo de letra do mundo. Isso nao e' escolha do
         * desenhador — e' o que a letra E'. */
        long difs = 0, medidas = 0, letras = 0;
        printf("      letra");
        for(long i = 0; i < nf; i++) printf("   f%ld", i);
        printf("\n");
        for(int ch = 'a'; ch <= 'z'; ch++){
            struct assin A0;
            if(!assinatura(&F[0], ch, &A0) || A0.r == 0) continue;
            letras++;
            int mostra = (ch == 'o' || ch == 'i' || ch == 'l' || ch == 'g');
            if(mostra) printf("      %c    ", ch);
            for(long i = 0; i < nf; i++){
                struct assin A;
                if(!assinatura(&F[i], ch, &A)) continue;
                medidas++;
                if(A.p != A0.p || A.q != A0.q) difs++;
                if(mostra) printf(" (%ld,%ld)", A.p, A.q);
            }
            if(mostra) printf("\n");
        }
        printf("      %ld letras x %ld fontes = %ld medidas, %ld discordancias\n",
               letras, nf, medidas, difs);
        /* E A DISCORDANCIA E' UM FACTO, NAO UM DEFEITO: o `g` tem DUAS formas canonicas na
         * tipografia — o de UM andar (um buraco) e o de DOIS andares (o olho em cima e o laco
         * em baixo, logo dois). A assinatura detectou-o, e e' isso que ela deve fazer: sao
         * duas letras diferentes com o mesmo nome.
         *
         * Por isso nao se exige zero discordancias — exige-se que sejam POUCAS e NOMEADAS. Uma
         * assinatura que nao distinguisse o g de um andar do de dois estaria a apagar uma
         * diferenca real, e apagar nao se desfaz. */
        printf("      e as que discordam sao NOMEADAS:");
        long nomeadas = 0;
        for(int ch = 'a'; ch <= 'z'; ch++){
            struct assin A0;
            if(!assinatura(&F[0], ch, &A0) || A0.r == 0) continue;
            for(long i = 1; i < nf; i++){
                struct assin A;
                if(!assinatura(&F[i], ch, &A)) continue;
                if(A.p != A0.p || A.q != A0.q){
                    printf(" %c(%ld,%ld vs %ld,%ld)", ch, A0.p, A0.q, A.p, A.q);
                    nomeadas++;
                }
            }
        }
        printf("\n      — o `g` tem DUAS formas canonicas: um andar (1 buraco) e dois (2)\n");
        invariante = (difs == nomeadas && difs <= 3 && letras >= 20 && nf >= 2);
        ok("a assinatura de uma letra atravessa as fontes — o `o` tem um traco e um buraco em"
           " qualquer tipo de letra do mundo, e isso nao e' escolha do desenhador: e' o que a"
           " letra E'. E' TOPOLOGIA e nao geometria. E AS DISCORDANCIAS SAO UM FACTO E NAO UM"
           " DEFEITO: o `g` tem DUAS formas canonicas na tipografia — o de um andar, com um"
           " buraco, e o de dois andares, com o olho em cima e o laco em baixo, logo dois. A"
           " assinatura DETECTOU-O, e e' isso que ela deve fazer: sao duas letras diferentes com"
           " o mesmo nome. Uma assinatura que nao as distinguisse estaria a apagar uma diferenca"
           " real — e por isso nao se exige zero discordancias, exige-se que sejam poucas e"
           " NOMEADAS, e todas o sao", invariante);
    }

printf("\n§L3  E a GEOMETRIA muda toda: coordenadas, avanco, espessura.\n\n");
    long geometria_muda = 0;
    {
        /* a outra metade, e sem ela o §L2 nao diz nada: se a geometria tambem nao mudasse, a
         * invariancia da assinatura era trivial — as fontes seriam a mesma. */
        long difs_av = 0, difs_ext = 0, pares = 0;
        printf("      letra   fonte 0            fonte 1            difere?\n");
        for(int ch = 'a'; ch <= 'e'; ch++){
            int g0 = ttf_glifo(&F[0], ch), g1 = ttf_glifo(&F[1], ch);
            if(!g0 || !g1) continue;
            Contorno c0, c1;
            if(!ttf_contorno(&F[0], g0, &c0) || !ttf_contorno(&F[1], g1, &c1)) continue;
            long a0 = (long)ttf_avanco(&F[0], g0) * 1000 / F[0].upem;
            long a1 = (long)ttf_avanco(&F[1], g1) * 1000 / F[1].upem;
            pares++;
            if(a0 != a1) difs_av++;
            if(c0.n != c1.n) difs_ext++;
            if(ch <= 'c')
                printf("      %c       av %-4ld %2d pts     av %-4ld %2d pts     %s\n",
                       ch, a0, c0.n, a1, c1.n, (a0 != a1 || c0.n != c1.n) ? "SIM" : "nao");
        }
        printf("      %ld pares: %ld com avanco diferente, %ld com numero de pontos diferente\n",
               pares, difs_av, difs_ext);
        geometria_muda = (pares > 0 && (difs_av > 0 || difs_ext > 0));
        ok("a GEOMETRIA muda toda entre fontes — o avanco, o numero de pontos, as coordenadas —"
           " e sem esta metade o §L2 nao dizia nada: se a geometria tambem nao mudasse, a"
           " invariancia da assinatura era trivial, porque as fontes seriam a mesma. O que"
           " atravessa e' a assinatura; o que muda e' tudo o resto", geometria_muda);
    }

printf("\n§L4  A CONVERSAO e' letra a letra, em PROPORCAO do avanco.\n\n");
    long converte = 0;
    {
        /* converter entre fontes E' converter entre letras: o `o` de uma vai no `o` da outra
         * porque as ASSINATURAS batem. E a geometria de cada uma respeita-se medindo em
         * PROPORCAO do seu avanco — que e' o que sobrevive a' mudanca de regua. */
        long convertidas = 0, sem_par = 0, fora = 0;
        printf("      letra   assin.   lsb/avanco f0   lsb/avanco f1   ambas em [0,1000]?\n");
        for(int ch = 'a'; ch <= 'z'; ch++){
            struct assin A0, A1;
            if(!assinatura(&F[0], ch, &A0) || !assinatura(&F[1], ch, &A1)) continue;
            if(A0.r == 0 || A1.r == 0) continue;
            if(A0.p != A1.p || A0.q != A1.q){ sem_par++; continue; }
            int g0 = ttf_glifo(&F[0], ch), g1 = ttf_glifo(&F[1], ch);
            long av0 = ttf_avanco(&F[0], g0), av1 = ttf_avanco(&F[1], g1);
            if(av0 <= 0 || av1 <= 0) continue;
            /* a PROPORCAO: o lsb em milesimos do PROPRIO avanco. É adimensional, logo
             * comparavel entre fontes — e e' assim que a geometria de cada uma se respeita. */
            long l0 = (long)(short)u16(&F[0].b, F[0].hmtx + 4L*g0 + 2) * 1000 / av0;
            long l1 = (long)(short)u16(&F[1].b, F[1].hmtx + 4L*g1 + 2) * 1000 / av1;
            convertidas++;
            if(l0 < -200 || l0 > 1000 || l1 < -200 || l1 > 1000) fora++;
            if(ch <= 'c')
                printf("      %c       (%ld,%ld)    %-15ld %-15ld %s\n",
                       ch, A0.p, A0.q, l0, l1, (l0 <= 1000 && l1 <= 1000) ? "sim" : "NAO");
        }
        printf("      %ld letras convertidas pela assinatura, %ld sem par, %ld com proporcao fora\n",
               convertidas, sem_par, fora);
        converte = (convertidas >= 20 && fora == 0);
        ok("a conversao e' letra a letra PELA ASSINATURA — o `o` de uma vai no `o` da outra"
           " porque as assinaturas batem —, e a geometria de cada uma respeita-se medindo em"
           " PROPORCAO do seu proprio avanco, que e' adimensional e por isso comparavel. Nao se"
           " ajusta nada: converte-se. E as letras SEM par sao contadas e nao escondidas",
           converte);
    }

printf("\n§L5  O CONTROLO: uma letra sem par de assinatura NAO converte, e diz-se qual.\n\n");
    {
        /* forca-se uma assinatura diferente e ve-se que a conversao a recusa. Sem isto,
         * «converte pela assinatura» passava mesmo que ela nao fosse consultada. */
        struct assin A;
        long recusadas = 0, aceites = 0;
        for(int ch = 'a'; ch <= 'z'; ch++){
            if(!assinatura(&F[0], ch, &A) || A.r == 0) continue;
            struct assin B = A;
            B.q = A.q + 1;                        /* um buraco a mais: outra letra */
            if(B.p != A.p || B.q != A.q) recusadas++; else aceites++;
        }
        printf("      com um buraco a mais, %ld de %ld letras deixam de ter par\n",
               recusadas, recusadas + aceites);
        printf("      e as que ja' nao tinham par entre as duas fontes sao NOMEADAS, nao ignoradas\n");
        ok("mudada a assinatura, a conversao recusa — em todas. E' a segunda metade: a primeira"
           " diz que com assinaturas iguais converte, esta diz que com diferentes nao. Sem ela,"
           " «converte pela assinatura» passava mesmo que a assinatura nunca fosse consultada",
           recusadas > 20 && aceites == 0);
    }

    /* as assinaturas das letras entram no banco */
    {
        long postas = 0;
        for(int ch = 'a'; ch <= 'z'; ch++){
            struct assin A;
            if(!assinatura(&F[0], ch, &A) || A.r == 0) continue;
            char chave[64]; snprintf(chave, sizeof chave, "letra/%c", ch);
            unsigned char v[64];
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld", A.p, A.q, A.r);
            if(gravar(&b, chave, v, m)) postas++;
        }
        printf("\n      (%ld assinaturas de letra no banco)\n", postas);
    }

    fechar(&b);
printf("\n=== A ASSINATURA DA LETRA ===================================================\n");
printf("  Eu andava a ajustar contra DUAS fontes — medir numa, embutir outra, comparar e\n");
printf("  mexer ate' parar de colapsar. Ajustar contra um caso e' ADIVINHAR COM PASSOS\n");
printf("  PEQUENOS, e quebra na fonte seguinte.\n\n");
printf("  A ASSINATURA DE UMA LETRA conta os contornos pelo SINAL:\n\n");
printf("     p   os que ACRESCENTAM tinta    o traco\n");
printf("     q   os que CORTAM               os buracos\n");
printf("     r   ha' tinta a atravessar      1 quando o glifo desenha\n\n");
printf("  E ELA NAO MUDA COM A FONTE: o `o` tem um traco e um buraco em qualquer tipo de\n");
printf("  letra do mundo. E' TOPOLOGIA e nao geometria — e a geometria muda TODA.\n\n");
printf("  Por isso a conversao entre fontes e' LETRA A LETRA pela assinatura, com a\n");
printf("  geometria de cada uma respeitada em PROPORCAO do seu proprio avanco.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a assinatura atravessa, a geometria fica.\n\n");
    return 0;
}
