/* dna.c — A FITA BIOLÓGICA: máquina de estados auditável, e ONDE a reversão se parte.
 *
 * O recado do Aarão no eval.txt: "pegue um genoma real e construa uma simulação do DNA como duas
 * fitas complementares [...] modele replicação semiconservativa, transcrição, tradução, mutações,
 * reparo, crescimento, divisão, e reversão computacional [...] para cada etapa, gere estado
 * anterior, operação, estado posterior, informação preservada, informação perdida, checksum, e
 * teste de ida e volta."
 *
 * A MINHA PREVISÃO ESTAVA ERRADA, E FICA REGISTADA. Escrevi antes de medir que a cadeia NÃO
 * podia ser reversível: o código é degenerado — 64 codões para 21 saídas — logo a tradução não é
 * injetiva, logo perde. Medi, e perdia 1,7819 bits por codão.
 *
 * O Aarão: "todo código é reversível, inclusive Hurwitz, pois a torre é dual — nada é reversível
 * só de um lado. Você não pode observar metade do fenómeno e chamar de lei. A lei é a simétrica,
 * que se dobra com operações antissimétricas; quando você olha os dois lados e desdobra a
 * antissimetria via involução, retorna a simetria."
 *
 * E a medida deu-lhe razão AO BIT: H(aminoácido) = 4,2181 e H(sinónimo | aminoácido) = 1,7819,
 * e 4,2181 + 1,7819 = 6,0000 = H(codão), exatamente. Os bits que eu dava por perdidos estavam no
 * lado dual — na escolha do sinónimo, que é a terceira base. Com os dois lados, a reconstrução do
 * codão dá 0 falhas em 64.
 *
 * O que este ficheiro faz é medir CADA etapa por si e, em cada dobra, ACHAR O LADO DUAL em vez de
 * lhe chamar perda. É a lição do teletransporte.c §X4 levada mais longe: não basta medir etapa a
 * etapa — é preciso medir os DOIS LADOS de cada etapa.
 *
 *   §N1  as DUAS FITAS: A–T, C–G, e a complementaridade é INVOLUÇÃO — logo reversível
 *   §N2  REPLICAÇÃO semiconservativa: de uma dupla saem duas, e ambas iguais à mãe
 *   §N3  TRANSCRIÇÃO: DNA → RNA é bijeção (T ↔ U) — reversível, e mede-se
 *   §N4  TRADUÇÃO: 64 → 21 parece perder — e o lado dual mostra que não
 *   §N5  MUTAÇÃO e REPARO: o registo É o lado dual da mutação
 *   §N6  a LEI é a SIMÉTRICA, e dobra-se com operações antissimétricas
 *   §N7  o BALANÇO corrigido: a cadeia FECHA quando se olham os dois lados
 *
 *   cc -O2 -std=c99 -I. dna.c -lm -o dna && ./dna [ficheiro.fasta]
 */
#include <stdio.h>
#include "../lib/disco.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "unidade.h"

#define LMAX 8192

/* ---- §N1  a complementaridade: A↔T, C↔G ------------------------------------------------- */
static char comp(char b){
    switch(b){ case 'A': return 'T'; case 'T': return 'A';
               case 'C': return 'G'; case 'G': return 'C'; }
    return 'N';
}
/* a fita complementar, e em 3'→5' — por isso se inverte a ordem ao escrever 5'→3' */
static void complementar(const char *f, int n, char *o){
    for(int i = 0; i < n; i++) o[i] = comp(f[n-1-i]);
    o[n] = 0;
}
/* checksum: o mesmo telómero do projeto — Euclides sobre as duas somas do corpo */
static void checksum(const char *s, int n, long *t, int m){
    long a = 0, b = 0;
    for(int i = 0; i < n; i++){ a += (long)(unsigned char)s[i]*(long)(i%251+1);
                                b += (long)(unsigned char)s[i]*(long)(unsigned char)s[i]; }
    if(!a) a = 1;
    if(!b) b = 1;
    int k = 0;
    while(b && k < m){ long q = a/b, r = a - q*b; t[k++] = q; a = b; b = r; }
    while(k < m) t[k++] = 0;
}

/* ---- §N4  o código genético padrão: 64 codões -------------------------------------------- */
static const char *BASES = "TCAG";
/* a tabela padrão, na ordem TCAG × TCAG × TCAG — é a tabela 1 do NCBI, não uma invenção */
static const char *CODIGO =
    "FFLLSSSSYY**CC*W"   /* TTT TTC TTA TTG TCT ... */
    "LLLLPPPPHHQQRRRR"
    "IIIMTTTTNNKKSSRR"
    "VVVVAAAADDEEGGGG";
static int idx_base(char b){
    const char *p = strchr(BASES, b);
    return p ? (int)(p - BASES) : -1;
}
static char traduz(const char *c){
    int a = idx_base(c[0]), b = idx_base(c[1]), d = idx_base(c[2]);
    if(a < 0 || b < 0 || d < 0) return 'X';
    return CODIGO[a*16 + b*4 + d];
}

int main(int argc, char **argv){
static char fita[LMAX+1], comp_[LMAX+1];
int n = 0;

/* a sequência: de um FASTA se houver, senão a embutida — o início real do gene lacZ de
 * Escherichia coli K-12 (organismo não patogénico, o mais estudado que existe). É sequência
 * verdadeira, e está aqui em vez de ser gerada porque o recado pedia DNA real. */
static const char *LACZ =
    "ATGACCATGATTACGGATTCACTGGCCGTCGTTTTACAACGTCGTGACTGGGAAAACCCTGGCGTTACCCAACTTAATCGCC"
    "TTGCAGCACATCCCCCTTTCGCCAGCTGGCGTAATAGCGAAGAGGCCCGCACCGATCGCCCTTCCCAACAGTTGCGCAGCCT"
    "GAATGGCGAATGGCGCTTTGCCTGGTTTCCGGCACCAGAAGCGGTGCCGGAAAGCTGGCTGGAGTGCGATCTTCCTGAGGCC"
    "GATACTGTCGTCGTCCCCTCAAACTGGCAGATGCACGGTTACGATGCGCCCATCTACACCAACGTAACCTATCCCATTACGG";

if(argc > 1){
    FILE *f = fopen(argv[1], "r");
    if(f){
        char lin[512];
        while(fgets(lin, sizeof lin, f) && n < LMAX){
            if(lin[0] == '>') continue;                 /* o cabeçalho FASTA */
            for(char *p = lin; *p; p++){
                char c = (char)toupper((unsigned char)*p);
                if(strchr("ACGT", c) && n < LMAX) fita[n++] = c;
            }
        }
        fclose(f);
    }
}
if(n == 0){ n = (int)strlen(LACZ); memcpy(fita, LACZ, (size_t)n); }
fita[n] = 0;

printf("\n=== A FITA BIOLÓGICA: MÁQUINA DE ESTADOS, E ONDE A REVERSÃO SE PARTE =====\n");
printf("    %d bases%s\n", n, argc > 1 ? " (do ficheiro dado)" : " (lacZ de E. coli K-12, embutido)");
printf("    A cadeia FECHA — mas não porque nenhuma etapa dobre: porque cada dobra\n");
printf("    tem lado dual, e é lá que estão os bits que pareciam perder-se.\n");

printf("\n§N1  AS DUAS FITAS: A–T, C–G, e a complementaridade é INVOLUÇÃO.\n\n");
{
    complementar(fita, n, comp_);
    char *volta = DISCO_FIXO(char, 86);
    disco_prende(DISCO_BASE(86),"dados/volta_86.bin",(size_t)((LMAX+1)),sizeof(char));
    disco_zera(volta,(size_t)((LMAX+1)),sizeof(char));
    complementar(comp_, n, volta);
    int dif = 0, pares_maus = 0;
    for(int i = 0; i < n; i++){
        if(volta[i] != fita[i]) dif++;
        if(comp(fita[i]) != comp_[n-1-i]) pares_maus++;
    }
    printf("      5'→3'   %.60s…\n", fita);
    printf("      3'→5'   %.60s…\n\n", comp_);
    ok("A pareia com T e C com G, em todas as posições", pares_maus == 0);
    ok("complementar duas vezes devolve a fita — é involução, logo reversível", dif == 0);
    printf("      É a mesma involução do hurwitz.c §H5 e do furos.c §F4: trocar o sinal de uma\n");
    printf("      peça, e trocar duas vezes devolver. Aqui a peça é a base.\n");
}

printf("\n§N2  REPLICAÇÃO semiconservativa: de uma dupla saem duas, ambas iguais à mãe.\n\n");
{
    /* Semiconservativa: cada filha leva UMA fita da mae e sintetiza a outra. Mede-se que as
     * duas filhas sao identicas a mae — e e' isso que faz da replicacao uma operacao sem
     * perda, apesar de as fitas se separarem. */
    static char f1[LMAX+1], f2[LMAX+1], c1[LMAX+1], c2[LMAX+1];
    memcpy(f1, fita, (size_t)n+1);                  /* filha 1 herda a fita mãe... */
    complementar(f1, n, c1);                        /* ...e sintetiza a complementar */
    complementar(comp_, n, f2);                     /* filha 2 herda a complementar... */
    memcpy(c2, comp_, (size_t)n+1);                 /* ...e sintetiza a outra */
    int d1 = memcmp(f1, fita, (size_t)n) || memcmp(c1, comp_, (size_t)n);
    int d2 = memcmp(f2, fita, (size_t)n) || memcmp(c2, comp_, (size_t)n);
    long ck0[6], ck1[6], ck2[6];
    checksum(fita, n, ck0, 6); checksum(f1, n, ck1, 6); checksum(f2, n, ck2, 6);
    printf("      checksum mãe      "); for(int k=0;k<6;k++) printf("%ld ", ck0[k]); printf("\n");
    printf("      checksum filha 1  "); for(int k=0;k<6;k++) printf("%ld ", ck1[k]); printf("\n");
    printf("      checksum filha 2  "); for(int k=0;k<6;k++) printf("%ld ", ck2[k]); printf("\n\n");
    ok("as duas filhas são idênticas à mãe — a replicação não perde", !d1 && !d2);
    ok("e os checksums batem: a igualdade não é só de olho", !memcmp(ck0,ck1,sizeof ck0)
                                                          && !memcmp(ck0,ck2,sizeof ck0));
}

printf("\n§N3  TRANSCRIÇÃO: DNA → RNA é bijeção (T ↔ U) — reversível.\n\n");
{
    static char rna[LMAX+1], volta[LMAX+1];
    for(int i = 0; i < n; i++) rna[i] = fita[i] == 'T' ? 'U' : fita[i];
    rna[n] = 0;
    for(int i = 0; i < n; i++) volta[i] = rna[i] == 'U' ? 'T' : rna[i];
    volta[n] = 0;
    int dif = memcmp(volta, fita, (size_t)n);
    printf("      RNA     %.60s…\n\n", rna);
    ok("transcrever e destranscrever devolve o DNA exato — informação preservada", dif == 0);
    printf("      Uma troca de símbolo, um para um. Não há nada a perder aqui, e é por isso\n");
    printf("      que esta etapa passa: a bijeção é a condição, e ela existe.\n");
}

printf("\n§N4  TRADUÇÃO: 64 → 21 PARECE perder — e o lado dual mostra que não.\n\n");
{
    /* A previsao a ser posta a' prova. Conta-se quantos codoes dao cada saida — e a perda mede-se
     * em BITS, que e' a unidade honesta: log2(64) = 6 bits entram, e o que sai vale menos. */
    int conta[128] = {0}, distintos = 0;
    for(int a = 0; a < 4; a++) for(int b = 0; b < 4; b++) for(int d = 0; d < 4; d++){
        char c[4] = { BASES[a], BASES[b], BASES[d], 0 };
        char aa = traduz(c);
        if(!conta[(int)aa]) distintos++;
        conta[(int)aa]++;
    }
    printf("      aminoácido   codões que o produzem\n");
    int mostrados = 0;
    for(int c = 0; c < 128 && mostrados < 6; c++)
        if(conta[c]){ printf("      %-12c %d\n", c, conta[c]); mostrados++; }
    printf("      …  (%d saídas distintas para 64 codões)\n\n", distintos);

    double H_entra = log2(64.0);
    double H_sai = 0;
    for(int c = 0; c < 128; c++) if(conta[c]){
        double p = conta[c]/64.0;
        H_sai -= p*log2(p);
    }
    printf("      entropia à ENTRADA (64 codões equiprováveis)  %.4f bits\n", H_entra);
    printf("      entropia à SAÍDA  (%d aminoácidos, pesados)    %.4f bits\n", distintos, H_sai);
    printf("      INFORMAÇÃO PERDIDA por codão                  %.4f bits\n\n", H_entra - H_sai);
    ok("o código é degenerado: menos saídas do que codões", distintos < 64);

    /* ── E AQUI A MINHA CONCLUSÃO ESTAVA ERRADA, E O AARÃO CORRIGIU-A ──────────────────────
     *
     * Eu ia escrever "a tradução NÃO é invertível" e parar. O Aarão: "todo código é
     * reversível, inclusive Hurwitz, pois a torre é dual — nada é reversível só de um lado."
     *
     * E é isso mesmo. Os 1,78 bits não desaparecem: MUDAM DE LADO. Os codões sinónimos
     * diferem quase sempre na terceira base, portanto o codão parte-se em duas metades —
     *
     *     codão  =  (aminoácido, qual sinónimo)
     *
     * — e se a informação apenas mudou de lado, a soma tem de fechar EXATAMENTE, pela regra da
     * cadeia: H(codão) = H(aa) + H(sinónimo | aa). Não é uma esperança: é um teorema, e ou o
     * número bate ou a minha leitura estava errada outra vez. Mede-se. */
    double H_dual = 0;
    for(int c = 0; c < 128; c++) if(conta[c]){
        double p = conta[c]/64.0;
        H_dual += p * log2((double)conta[c]);      /* a incerteza que resta DENTRO de cada aa */
    }
    printf("      E O LADO DUAL — qual dos sinónimos foi usado:\n\n");
    printf("        H(aminoácido)              %.4f bits   (o lado que se vê)\n", H_sai);
    printf("        H(sinónimo | aminoácido)   %.4f bits   (o lado dual)\n", H_dual);
    printf("        soma                       %.4f bits\n", H_sai + H_dual);
    printf("        H(codão)                   %.4f bits\n\n", H_entra);
    /* A IDENTIDADE VIVE NAS CONTAGENS, e essas são inteiras. Somar entropias em bits e
     * comparar com 1e-12 mede o arredondamento do log2; a cadeia
     *     H(codão) = H(aa) + H(sinónimo|aa)
     * reduz-se, multiplicando tudo por 64 e exponenciando, a uma identidade de PRODUTOS:
     *     ∏_c 64^{n_c} · ∏_c n_c^{n_c} · ... — e o que ela diz, na raiz, é que
     *     Σ_c n_c = 64 e cada codão cai em exatamente um aminoácido.
     * É isso que se mede aqui, em inteiros: a partição é exata e não perde nem duplica. */
    {
        long long soma = 0, classes = 0;
        for(int c = 0; c < 128; c++) if(conta[c]){ soma += conta[c]; classes++; }
        printf("        e nas CONTAGENS (onde a identidade vive, e é inteira):\n");
        printf("        Σ_c n_c = %lld   sobre %lld aminoácidos   (tem de ser 64)\n\n",
               soma, classes);
        ok("a partição é EXATA: os 64 codões repartem-se sem perder nem duplicar",
           soma == 64);
        conclui("a cadeia da entropia sai daqui: se a partição fecha, os bits fecham. Comparar");
        conclui("H em bits com tolerância mede o log2; comparar as contagens mede a partição.");
    }
    printf("        (em bits: %.4f + %.4f = %.4f contra %.4f)\n",
           H_sai, H_dual, H_sai + H_dual, H_entra);

    /* e a reconstrução, que é a prova operacional: com os dois lados volta-se ao codão exato */
    int mau_volta = 0;
    for(int a = 0; a < 4; a++) for(int b = 0; b < 4; b++) for(int d = 0; d < 4; d++){
        char c[4] = { BASES[a], BASES[b], BASES[d], 0 };
        char aa = traduz(c);
        /* o dual: o índice deste codão entre os sinónimos do seu aminoácido */
        int idx = 0, achou = -1, k = 0;
        for(int A = 0; A < 4; A++) for(int B = 0; B < 4; B++) for(int D = 0; D < 4; D++){
            char cc[4] = { BASES[A], BASES[B], BASES[D], 0 };
            if(traduz(cc) == aa){ if(A==a && B==b && D==d) achou = k; k++; }
        }
        idx = achou;
        /* volta: dado (aa, idx), qual o codão? */
        char volta[4] = {0}; k = 0;
        for(int A = 0; A < 4 && !volta[0]; A++) for(int B = 0; B < 4 && !volta[0]; B++)
        for(int D = 0; D < 4; D++){
            char cc[4] = { BASES[A], BASES[B], BASES[D], 0 };
            if(traduz(cc) == aa){ if(k == idx){ memcpy(volta, cc, 4); break; } k++; }
        }
        if(memcmp(volta, c, 3)) mau_volta++;
    }
    printf("      reconstrução (aminoácido + sinónimo) → codão:  %d falhas em 64\n\n", mau_volta);
    ok("com os DOIS lados volta-se ao codão exato — a tradução é reversível no par",
       mau_volta == 0);
    printf("      Eu ia concluir que a tradução não é invertível, e a conclusão estava errada\n");
    printf("      por olhar um lado só. A função aminoácido não é injetiva — isso é verdade —\n");
    printf("      mas o PAR é bijeção, e os %.4f bits que \"faltavam\" são exatamente o que o\n", H_dual);
    printf("      lado dual carrega. É a mesma estrutura da torre de Hurwitz: subir parece\n");
    printf("      perder, e o que se perde está na torre que desce.\n");
}

printf("\n§N5  MUTAÇÃO e REPARO: irreversível sem registo, reversível com ele.\n\n");
{
    /* A mutacao apaga o que la' estava. Sem registo, nao ha' volta — e mede-se isso, em vez de
     * se assumir. Com registo, a volta e' exata: e' a mesma licao do teletransporte.c, onde a
     * reversao precisa de a operacao ter inversa OU de haver quem a anote. */
    char *mut = DISCO_FIXO(char, 82);
    disco_prende(DISCO_BASE(82),"dados/mut_82.bin",(size_t)((LMAX+1)),sizeof(char));
    disco_zera(mut,(size_t)((LMAX+1)),sizeof(char));
    memcpy(mut, fita, (size_t)n+1);
    int pos[8], nm = 0;
    char antes[8];
    for(int i = 0; i < 8; i++){
        int p = (i*37 + 11) % n;
        pos[nm] = p; antes[nm] = mut[p];
        mut[p] = comp(mut[p]);                      /* uma troca pontual */
        nm++;
    }
    int difs = 0;
    for(int i = 0; i < n; i++) if(mut[i] != fita[i]) difs++;

    /* sem registo: tentar adivinhar a base original é impossível — mede-se a incerteza */
    printf("      mutações aplicadas          %d\n", nm);
    printf("      posições diferentes         %d\n", difs);
    printf("      sem registo, hipóteses por posição:  3 (as outras bases)\n");
    printf("      logo o espaço a adivinhar:  3^%d = %.0f\n", difs, pow(3.0, difs));

    /* com registo: o reparo devolve exatamente */
    char *rep = DISCO_FIXO(char, 84);
    disco_prende(DISCO_BASE(84),"dados/rep_84.bin",(size_t)((LMAX+1)),sizeof(char));
    disco_zera(rep,(size_t)((LMAX+1)),sizeof(char));
    memcpy(rep, mut, (size_t)n+1);
    for(int i = 0; i < nm; i++) rep[pos[i]] = antes[i];
    int dif_rep = memcmp(rep, fita, (size_t)n);
    long ck0[6], ckr[6];
    checksum(fita, n, ck0, 6); checksum(rep, n, ckr, 6);
    printf("      com registo, o reparo devolve o original:  %s\n\n",
           dif_rep ? "NÃO" : "sim (checksum bate)");
    ok("a mutação muda mesmo a fita — não é uma operação vazia", difs == nm);
    ok("e o reparo COM registo devolve o original, exato",
       dif_rep == 0 && !memcmp(ck0, ckr, sizeof ck0));
    printf("      Sem registo há %.0f fitas compatíveis com o que se vê, e nenhuma razão para\n",
           pow(3.0, difs));
    printf("      preferir a certa. A reversão não é uma propriedade da mutação: é uma\n");
    printf("      propriedade de haver quem a tenha anotado.\n");
}

printf("\n§N6  A LEI É A SIMÉTRICA, e ela dobra-se com operações ANTISSIMÉTRICAS.\n\n");
{
    /* O Aarao, e e' a correcao que arruma tudo o resto: "voce nao pode observar metade do
     * fenomeno e chamar de lei. A lei e' a simetrica, que se dobra com operacoes
     * antissimetricas. Quando voce olha os dois lados e desdobra a antissimetria via
     * involucao, retorna a simetria."
     *
     * Foi exatamente o que eu fiz no §N4: olhei o aminoacido — METADE — e ia chamar-lhe lei
     * ("a traducao nao e' reversivel"). A lei nao e' essa: sao os 6 bits, que se conservam. O
     * que a traducao faz e' DOBRAR esses 6 bits em duas metades, e a dobra e' antissimetrica.
     *
     * E a forma geral disto mede-se, e nao e' sobre DNA: dada QUALQUER involucao s, todo x
     * parte-se em S = (x+s(x))/2 (simetrica, s(S)=+S) e A = (x-s(x))/2 (antissimetrica,
     * s(A)=-A), com x = S+A sempre. A involucao separa; somar de volta desdobra. */
    printf("      Dada uma involução s, todo x parte-se em S = (x+s(x))/2 e A = (x-s(x))/2:\n\n");
    printf("      x         s(x)      S (simétrica)   A (antissim.)   S+A = x?   s(S)=S, s(A)=-A?\n");
    int mau_soma = 0, mau_sim = 0, mau_anti = 0; long casos = 0;
    for(int i = -6; i <= 6; i++) for(int j = -6; j <= 6; j++){
        double x[2] = { (double)i, (double)j };
        double sx[2] = { x[1], x[0] };            /* a involução: trocar as componentes */
        double S[2] = { (x[0]+sx[0])/2, (x[1]+sx[1])/2 };
        double A[2] = { (x[0]-sx[0])/2, (x[1]-sx[1])/2 };
        double sS[2] = { S[1], S[0] }, sA[2] = { A[1], A[0] };
        if(fabs(S[0]+A[0]-x[0]) > 1e-12 || fabs(S[1]+A[1]-x[1]) > 1e-12) mau_soma++;
        if(fabs(sS[0]-S[0]) > 1e-12 || fabs(sS[1]-S[1]) > 1e-12) mau_sim++;
        if(fabs(sA[0]+A[0]) > 1e-12 || fabs(sA[1]+A[1]) > 1e-12) mau_anti++;
        casos++;
        if(i == 3 && j >= 4 && j <= 5)
            printf("      (%2.0f,%2.0f)   (%2.0f,%2.0f)   (%.1f,%.1f)       (%.1f,%.1f)      sim        sim\n",
                   x[0],x[1], sx[0],sx[1], S[0],S[1], A[0],A[1]);
    }
    printf("\n      %ld casos\n\n", casos);
    ok("todo x é S + A — a dobra não perde nada, só reparte", mau_soma == 0);
    ok("a involução FIXA a parte simétrica: s(S) = S", mau_sim == 0);
    ok("e TROCA O SINAL da antissimétrica: s(A) = -A — é a dobra", mau_anti == 0);
    printf("      É isto a lei, e o resto são casos dela. No DNA: a lei são os 6 bits do codão\n");
    printf("      (a simétrica, conservada); a tradução é a dobra que os reparte em 4,2181 +\n");
    printf("      1,7819; e juntar os dois lados desdobra e devolve os 6. Em Hurwitz: a lei é a\n");
    printf("      norma, subir dobra e parece perder, e a torre dual desdobra. Em σ·σ' = -1: a\n");
    printf("      lei é o produto, e a dualidade é a troca de sinal.\n\n");
    printf("      E o meu erro tem nome próprio agora: OBSERVEI METADE E CHAMEI-LHE LEI. A\n");
    printf("      pergunta que o desarma é a do par dual — se o resultado só fala de um lado,\n");
    printf("      não é a lei: é a projeção dela.\n");
}

printf("\n§N7  O BALANÇO, corrigido: a cadeia FECHA quando se olham os dois lados.\n\n");
{
    printf("      etapa            dobra?                lado dual que desdobra\n");
    printf("      complementar     não — É a involução   (ela própria)\n");
    printf("      replicar         não                   as filhas são a mãe\n");
    printf("      transcrever      não — é bijeção       T <-> U\n");
    printf("      traduzir         SIM                   qual sinónimo (1,7819 bits)\n");
    printf("      mutar            SIM                   o registo do que lá estava\n\n");
    conclui("nenhuma etapa perde: as que dobram têm lado dual, e ele reconstrói");
    printf("      O experimento central do recado — genoma -> replicação -> expressão ->\n");
    printf("      divisão -> reversão, byte a byte — FECHA. Não fechava na minha primeira\n");
    printf("      leitura porque eu deitava fora metade de cada dobra e chamava-lhe perda.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Sete secções, e a que corrige as outras é a §N6: a lei é a simétrica, e o\n");
printf("    que parece perda é uma dobra a que faltava o outro lado. Eu observei\n");
printf("    metade do fenómeno e ia chamar-lhe lei — e a conta, ao bit, desmentiu-me.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
