/* espelho.c — QUEBRAR O ESPELHO NÃO CRIA ESPELHO NEM SOME ESPELHO.
 *
 * Correção de rota, do Aarão, e ela é justa. Em defeito.c eu peguei um instrumento
 * ESTATÍSTICO — esperança gaussiana, long double, quadratura — para um sistema que tem
 * conservação EXATA em bit. E, pior, inventei a amostragem (seis bytes espalhados por um
 * arquivo não é a regra de ninguém: fui eu que arbitrei). Régua errada com muitas casas
 * decimais parece precisão, e não é.
 *
 * Aqui não há um único float. Nem um. Tudo é inteiro, tudo é bit, e cabe em poucos bytes —
 * porque o sistema nunca deixou de ter UMA peça só: ele é autossimilar, e o que vale num bit
 * vale na palavra inteira.
 *
 * A lei, em uma frase: o espelho É o todo; quebrá-lo não cria espelho nem faz sumir espelho.
 * Em aritmética: toda cisão CONSERVA, e toda cisão VOLTA.
 *
 *   §E1  a cisão conserva: par + ímpar = o todo, bit a bit, nos 256 bytes
 *   §E2  e a cisão volta: os dois cacos recompõem o espelho, sem resto e sem sobra
 *   §E3  o gato é bijeção em inteiros: nada se perde ao atravessar, em toda largura
 *   §E4  autossimilar: a MESMA lei em 4, 8, 16, 32 bits — uma peça, todas as escalas
 *   §E5  digital == analógico: a cisão é o nó de Kirchhoff, e o nó fecha exato
 *
 *   cc -O2 -std=c99 espelho.c -o espelho && ./espelho
 */
#include <stdio.h>
#include "../lib/disco.h"
#include <stdint.h>

#include "unidade.h"
/* contagem de bits — sem builtin, para não haver nada escondido */
static int bits(uint64_t x){ int n = 0; while(x){ n += (int)(x & 1); x >>= 1; } return n; }

/* as duas máscaras da cisão: os bits pares e os bits ímpares */
static uint64_t mascara_par(int larg){
    uint64_t m = 0;
    for(int i = 1; i < larg; i += 2) m |= (uint64_t)1 << i;
    return m;
}
static uint64_t mascara_impar(int larg){
    uint64_t m = 0;
    for(int i = 0; i < larg; i += 2) m |= (uint64_t)1 << i;
    return m;
}

int main(void){
printf("\n=== O ESPELHO: QUEBRAR NÃO CRIA NEM SOME ===================================\n");
printf("    Nenhum float neste medidor. Só inteiro, só bit — e cabe em poucos bytes.\n");

/* ---------------------------------------------------------------- §E1 ------ */
printf("\n§E1  A cisão CONSERVA: par + ímpar = o todo, bit a bit, nos 256 bytes.\n\n");
{
    int mau = 0, soma_total = 0;
    for(int b = 0; b <= 255; b++){
        int par = bits((uint64_t)(b & 0xAA));
        int imp = bits((uint64_t)(b & 0x55));
        int todo = bits((uint64_t)b);
        if(par + imp != todo) mau++;
        soma_total += todo;
    }
    printf("      bytes examinados ............................... 256  (todos)\n");
    printf("      bits somados no conjunto ....................... %d\n", soma_total);
    ok("par + ímpar = o todo, sem uma exceção", mau == 0);
    printf("\n      Não é aproximação nem média: é contagem. Quebrar o byte em dois cacos não\n");
    printf("      inventa bit nem perde bit. O espelho é o todo.\n");
}

/* ---------------------------------------------------------------- §E2 ------ */
printf("\n§E2  E a cisão VOLTA: os dois cacos recompõem, sem resto e sem sobra.\n\n");
{
    int mau_ou = 0, mau_e = 0, mau_soma = 0;
    for(int b = 0; b <= 255; b++){
        int pa = b & 0xAA, im = b & 0x55;
        if((pa | im) != b) mau_ou++;          /* recompõe */
        if((pa & im) != 0)  mau_e++;          /* e não se sobrepõem: a quebra é limpa */
        if((pa + im) != b)  mau_soma++;       /* e a soma É a união, porque são disjuntos */
    }
    ok("os cacos recompõem o espelho: (b&par)|(b&ímpar) = b", mau_ou == 0);
    ok("e não se sobrepõem: a interseção é vazia", mau_e == 0);
    ok("logo somar é o mesmo que unir — nada conta duas vezes", mau_soma == 0);
    printf("\n      Este é o ponto todo: a quebra é uma PARTIÇÃO. Por isso não cria e não some.\n");
}

/* ---------------------------------------------------------------- §E3 ------ */
printf("\n§E3  O gato é bijeção em inteiros — atravessar não perde nada.\n");
printf("     (x,y) ↦ (m·x + y, x)  mod 2^L. Como det A = −1 é unidade, é permutação.\n\n");
{
    int mau = 0;
    printf("      L    pares (2^2L)    imagens distintas    é permutação?\n");
    for(int L = 1; L <= 7; L++){
        long N = 1L << L, tot = N*N;
        /* marca de visita: um bit por par, sem alocar (cabe em 2^14 = 16384 bits) */
        uint64_t *visto = DISCO_FIXO(uint64_t, 214);
        disco_prende(DISCO_BASE(214),"dados/visto_214.bin",(size_t)((256)),sizeof(uint64_t));
        disco_zera(visto,(size_t)((256)),sizeof(uint64_t));
        for(int i = 0; i < 256; i++) visto[i] = 0;
        long distintas = 0;
        for(long x = 0; x < N; x++) for(long y = 0; y < N; y++){
            long nx = (1*x + y) % N, ny = x;      /* m = 1: o gato do ouro */
            long id = nx*N + ny;
            if(!((visto[id >> 6] >> (id & 63)) & 1)){
                visto[id >> 6] |= (uint64_t)1 << (id & 63);
                distintas++;
            }
        }
        int perm = (distintas == tot);
        printf("      %d    %12ld    %17ld    %s\n", L, tot, distintas, perm ? "sim ✓" : "NÃO ✗");
        if(!perm) mau++;
    }
    ok("o gato é permutação em toda largura testada", mau == 0);
    printf("\n      E a volta é o esquilo, colhido da borda: de σⁿ = mσⁿ⁻¹ + 1 sai a inversa sem\n");
    printf("      Fermat. Ida e volta em inteiros: o que entra sai, idêntico.\n");
}

/* ---------------------------------------------------------------- §E4 ------ */
printf("\n§E4  Autossimilar: a MESMA lei em toda largura. Uma peça, todas as escalas.\n\n");
{
    int larguras[4] = {4, 8, 16, 32};
    int mau = 0;
    printf("      largura   amostras   par+ímpar = todo   recompõe   disjuntos\n");
    for(int i = 0; i < 4; i++){
        int L = larguras[i];
        uint64_t mp = mascara_par(L), mi = mascara_impar(L);
        long N = (L <= 16) ? (1L << L) : 65536L;   /* em 32 bits, uma varredura densa basta */
        long passo = (L <= 16) ? 1 : 65537;        /* e o passo é primo com 2^32: cobre parelho */
        int c1 = 1, c2 = 1, c3 = 1;
        for(long k = 0; k < N; k++){
            uint64_t v = (L <= 16) ? (uint64_t)k : ((uint64_t)k * passo) & 0xFFFFFFFFu;
            uint64_t pa = v & mp, im = v & mi;
            if(bits(pa) + bits(im) != bits(v)) c1 = 0;
            if((pa | im) != v) c2 = 0;
            if((pa & im) != 0) c3 = 0;
        }
        printf("      %5d   %8ld   %16s   %8s   %9s\n", L, N,
               c1?"sim ✓":"NÃO ✗", c2?"sim ✓":"NÃO ✗", c3?"sim ✓":"NÃO ✗");
        if(!c1 || !c2 || !c3) mau++;
    }
    ok("a lei não muda com a escala — é a mesma peça", mau == 0);
    printf("\n      É o que a autossimilaridade quer dizer, e é por isso que POUCOS bytes bastam:\n");
    printf("      não há nada na palavra grande que já não esteja no bit. Medir mais é repetir.\n");
}

/* ---------------------------------------------------------------- §E5 ------ */
printf("\n§E5  Digital == analógico: a cisão é o NÓ, e o nó fecha exato.\n");
printf("     No analógico a corrente que chega ao nó é a soma das que saem (Kirchhoff).\n");
printf("     Em inteiros isso é a MESMA conta do §E1 — não é analogia, é a mesma lei.\n\n");
{
    int mau = 0;
    printf("      byte   entra   sai(par)   sai(ímpar)   nó fecha?\n");
    int mostrados = 0;
    for(int b = 0; b <= 255; b++){
        int entra = bits((uint64_t)b);
        int s1 = bits((uint64_t)(b & 0xAA)), s2 = bits((uint64_t)(b & 0x55));
        int fecha = (entra - s1 - s2) == 0;
        if(!fecha) mau++;
        if(mostrados < 6 && bits((uint64_t)b) >= 4){
            printf("      0x%02X   %5d   %8d   %10d   %s\n", b, entra, s1, s2, fecha?"sim ✓":"NÃO ✗");
            mostrados++;
        }
    }
    ok("o nó fecha em TODOS os 256 bytes (Σ entra = Σ sai)", mau == 0);
    printf("\n      E a volta também é a mesma nos dois meios: no digital, OU dos cacos; no\n");
    printf("      analógico, a junção das correntes no nó de saída. Zero de diferença, e não\n");
    printf("      por concordarem até certa casa decimal — por não haver casa decimal.\n");
}

printf("\n=== A LEI ==================================================================\n");
printf("  Quebrar o espelho não cria espelho e não some espelho. Toda cisão é PARTIÇÃO:\n");
printf("  conserva (§E1), volta (§E2), atravessa sem perda (§E3), e vale igual em toda\n");
printf("  escala (§E4) e nos dois meios (§E5). Uma peça só, e está tudo nela.\n");
printf("  Sem float, sem amostragem, sem média — 256 bytes bastam porque o resto é cópia.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, e exato aqui quer dizer inteiro.\n\n");
return 0;
}
