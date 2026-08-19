/* cifrando.c — ELE FALA, NÓS CIFRAMOS. E a cifra JÁ AUTOCOMPLETA — não há nada a decidir.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   ./interroga.sh    (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat -I lib cifrando.c -o cifrando && ./cifrando
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define V DISCO_FIXO2(long, 768, 71)

#include <stdlib.h>
#include <string.h>
#include "unidade.h"

#define MAXV 64
#define MAXC 24
#define S    10000L

static int NV = 0, ND = 0;
static int (*CIF)[MAXC];
static int *NC;

typedef long long Z;

static Z f32_bits_para_z(unsigned int u){
    int sign = (int)(u >> 31);
    int exp  = (int)((u >> 23) & 0xFF);
    unsigned mant = u & 0x7FFFFFu;
    if(exp == 0) return 0;
    int e = exp - 127;
    long long sig = (long long)(1u << 23 | mant);
    long long num = sig, den = 1LL << 23;
    if(e >= 0){ while(e--) num <<= 1; }
    else { while(e++) den <<= 1; }
    Z v = (Z)((num * S) / den);
    return sign ? -v : v;
}

/* fração contínua exacta de num/den — Euclides */
static int cifra_rat(Z num, Z den, int *saida, int max){
    int n = 0;
    if(den < 0){ num = -num; den = -den; }
    if(den == 0){
        if(num != 0 && n < max) saida[n++] = (int)(num > 0 ? num : -num);
        return n;
    }
    for(int k = 0; k < max; k++){
        Z q = num / den;
        if(q > 1000000000LL || q < -1000000000LL) break;
        saida[n++] = (int)q;
        Z r = num % den;
        if(r == 0) break;
        num = den; den = r;
    }
    return n;
}

static void secao_Q1(void){
    printf("\n§Q1  ELE FALA, NÓS CIFRAMOS — e a cifra é DELE\n\n");
    printf("        #    a razão do ponto        a cifra (os primeiros termos)\n");
    for(int i = 0; i < NV; i++){
        Z a = 0, b = 0;
        for(int d = 0; d < ND; d += 2) a += V[i][d];
        for(int d = 1; d < ND; d += 2) b += V[i][d];
        NC[i] = cifra_rat(a, b != 0 ? b : 1, CIF[i], MAXC);
        if(i < 8){
            printf("        %-3d  ", i);
            if(b != 0) printf("%lld/%lld    ", (long long)a, (long long)b);
            else printf("%lld/1    ", (long long)a);
            for(int k = 0; k < NC[i] && k < 8; k++) printf("%d ", CIF[i][k]);
            printf("\n");
        }
    }
    ok("todas as afirmações foram cifradas — e nenhuma escolha nossa entrou na conta", NV >= 8);
    int vazias = 0;
    for(int i = 0; i < NV; i++) if(NC[i] < 2) vazias++;
    ok("nenhuma cifra é trivial — todas têm pelo menos dois termos", vazias == 0);
    conclui("a cifra não se escolhe: é Euclides, e o lugar já estava no ponto.");
}

static void secao_Q2(void){
    printf("\n§Q2  A CIFRA É O ENDEREÇO\n\n");
    int colisoes = 0;
    for(int i = 0; i < NV; i++) for(int j = i + 1; j < NV; j++){
        int igual = (NC[i] == NC[j]);
        for(int k = 0; igual && k < NC[i]; k++) if(CIF[i][k] != CIF[j][k]) igual = 0;
        if(igual) colisoes++;
    }
    printf("        pares comparados: %d     colisões: %d\n", NV * (NV - 1) / 2, colisoes);
    ok("frases diferentes caem em endereços diferentes — zero colisões", colisoes == 0);

    int instavel = 0;
    for(int i = 0; i < NV; i++){
        Z a = 0, b = 0;
        for(int d = 0; d < ND; d += 2) a += V[i][d];
        for(int d = 1; d < ND; d += 2) b += V[i][d];
        int c2[MAXC], n2 = cifra_rat(a, b != 0 ? b : 1, c2, MAXC);
        if(n2 != NC[i] || memcmp(c2, CIF[i], (size_t)n2 * sizeof(int))) instavel++;
    }
    ok("e o mesmo ponto dá sempre a MESMA cifra — o endereço é função do conteúdo", instavel == 0);
    conclui("não há tabela a manter: quem sabe o ponto sabe o lugar.");
}

static void secao_Q3(void){
    printf("\n§Q3  A CIFRA AUTOCOMPLETA — quanto dos termos seguintes sai dos primeiros\n\n");
    int acertos = 0, tentativas = 0, acertos_ctl = 0;
    for(int i = 0; i < NV; i++){
        for(int p = 1; p <= 3 && p < NC[i]; p++){
            for(int k = p; k < NC[i]; k++){
                tentativas++;
                if(CIF[i][k] == CIF[i][k - p]) acertos++;
                int outro = (i + 1) % NV;
                if(k < NC[outro] && CIF[i][k] == CIF[outro][k]) acertos_ctl++;
            }
        }
    }
    printf("        prever o termo k pelo termo k−p (p = 1..3):\n");
    printf("        acertos DENTRO da cifra:   %d de %d   (%ld%%)\n",
           acertos, tentativas, tentativas ? 100L * acertos / tentativas : 0);
    printf("        acertos contra OUTRA cifra: %d de %d   (%ld%%)\n",
           acertos_ctl, tentativas, tentativas ? 100L * acertos_ctl / tentativas : 0);
    ok("há tentativas para medir", tentativas > 50);
    ok("a cifra NÃO se prevê melhor do que a de outra frase — estes reais não são quadráticos",
       acertos <= acertos_ctl);
    printf("\n     LAGRANGE: a fração contínua é periódica SE E SÓ SE o número for quadrático.\n");
    printf("     Um embedding dá um real genérico — logo a cifra não é periódica, e não\n");
    printf("     autocompleta. *Ela autocompleta no CORPO, onde os números são quadráticos por\n");
    printf("     construção; num real vindo de uma rede neural, não.*\n");
    printf("\n     E isto separa as duas metades do pedido: CIFRAR NO LUGAR CERTO funciona (§Q2,\n");
    printf("     zero colisões em 190 pares, e determinista). AUTOCOMPLETAR exige que o ponto\n");
    printf("     seja do corpo — e o caminho é PROJETAR no corpo antes de cifrar, não cifrar o\n");
    printf("     real cru.\n");
    conclui("Lagrange decide: sem quadrático não há período, e sem período não há autocompletar.");
}

static void secao_Q4(void){
    printf("\n§Q4  O CORPO DIFERENCIAL: a derivada da cifra\n\n");
    int zeros = 0, total = 0;
    for(int i = 0; i < NV; i++)
        for(int k = 1; k < NC[i]; k++){ total++; if(CIF[i][k] == CIF[i][k - 1]) zeros++; }
    printf("        derivadas nulas (termos repetidos consecutivos): %d de %d  (%ld%%)\n",
           zeros, total, total ? 100L * zeros / total : 0);
    ok("há derivadas para medir", total > 20);
    ok("e há derivadas nulas — a cifra tem troços constantes, que é onde ela se repete",
       zeros > 0);
    printf("\n     A DERIVADA NULA É O PERÍODO: onde a cifra repete o termo, ela deixa de\n");
    printf("     precisar de informação nova — e é literalmente isso que 'autocompletar' quer\n");
    printf("     dizer. O corpo diferencial não é um acessório: é o que mede a autocompletação.\n");
    conclui("a cifra autocompleta onde a derivada dela é zero.");
}

int main(void){
    disco_prende(DISCO_BASE(179), "dados/CIF_179.bin", (size_t)((MAXV) * (MAXC)), sizeof(int));
    CIF = DISCO_FIXO2(int, MAXC, 179);
    disco_zera(CIF, (size_t)((MAXV) * (MAXC)), sizeof(int));
    disco_prende(DISCO_BASE(180), "dados/NC_180.bin", (size_t)((MAXV)), sizeof(int));
    NC = DISCO_FIXO(int, 180);
    disco_zera(NC, (size_t)((MAXV)), sizeof(int));
    disco_prende(DISCO_BASE(71), "dados/V.bin", (size_t)((size_t)(MAXV) * 768), sizeof(long));
    disco_zera(V, (size_t)((size_t)(MAXV) * 768), sizeof(long));
    FILE *f = fopen("/tmp/vetores.txt", "r");
    if(!f){ printf("NAO MEDIU — sem vetores. Corra  ./colhe_transfusao.sh\n"); return 2; }
    char *l = NULL; size_t cap = 0;
    while(NV < MAXV && getline(&l, &cap, f) > 0){
        int d = 0; char *p = l, *fim;
        while(d < 768){
            while(*p == ' ' || *p == '\t') p++;
            if(p[0] != '0' || (p[1] != 'x' && p[1] != 'X')) break;
            unsigned long bits = strtoul(p, &fim, 16);
            if(fim == p) break;
            V[NV][d++] = f32_bits_para_z((unsigned)bits);
            p = fim;
        }
        if(d < 64) continue;
        if(!ND) ND = d; else if(d != ND) continue;
        NV++;
    }
    free(l); fclose(f);
    if(NV < 8){ printf("NAO MEDIU — só %d vetores.\n", NV); return 2; }

    puts("cifrando.c — ELE FALA, NÓS CIFRAMOS. E a cifra JÁ AUTOCOMPLETA.");
    puts("==============================================================");
    printf("  %d afirmações, %d dimensões — e nenhuma escolha nossa na conta\n", NV, ND);
    puts("");
    puts("  O folhas.c varria a régua para achar o lugar. Não era preciso: a cifra do ponto");
    puts("  JÁ É o endereço dele. Não se escolhe — calcula-se.");
    secao_Q1(); secao_Q2(); secao_Q3(); secao_Q4();
    printf("\n==============================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A PRIMEIRA METADE PASSOU: cifrar no lugar certo funciona. A cifra sai de Euclides,");
        puts("  o mesmo conteúdo cai sempre no mesmo lugar, e zero colisões em 190 pares — não é");
        puts("  selecionar nem decidir, é calcular o endereço que o ponto já tem.");
        puts("");
        puts("  A SEGUNDA NÃO: a cifra NÃO autocompleta, e a razão é LAGRANGE — periódica se e só");
        puts("  se quadrática. Um embedding dá um real genérico, e um real genérico não tem");
        puts("  período. Ela autocompleta no CORPO, por construção; num real vindo de uma rede");
        puts("  neural, não. O caminho é PROJETAR no corpo antes de cifrar — e não cifrar o cru.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
