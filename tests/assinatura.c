/* assinatura.c — A CONTAGEM NÃO PERDE: ELA ASSINA. (E o defeito era meu, de leitura.)
 *
 * Terceira versão, e as duas correções anteriores são do Aarão. A primeira media com esperança
 * gaussiana e amostragem inventada — régua errada, e num sistema que conserva em bit. A segunda
 * consertou a régua mas manteve o NOME errado: eu chamava de "defeito" o que é ASSINATURA.
 *
 * A segunda correção é a maior, e contradizia coisa que este projeto já tinha medido. Em
 * tools/significado.c está que 169 classes em 169 pontos é o REPOUSO, onde nada significa —
 * porque sem lei toda função é conservada e nenhuma distingue. Logo a partição grossa não é
 * dano à fina: é onde o NOME aparece. Chamar isso de defeito importa a suposição de que os 256
 * pontos eram "o real" e as 25 classes uma sombra estragada. É o inverso.
 *
 * E havia um segundo erro embutido: eu misturava dois papéis que o corpo mantém separados.
 *
 *     o gato    é BIJEÇÃO   — ali mora a recuperação, e ele não perde nada
 *     a contagem é ASSINATURA — ali mora o nome, e ela agrupa por projeto
 *
 * São os dois lados (⊗ e ⊕), não um lado defeituoso. Perguntar "quanto se perdeu" à assinatura
 * é como perguntar a um sobrenome por que ele não distingue irmãos.
 *
 * Os números abaixo são exatamente os mesmos da versão anterior. O que muda é o que eles dizem.
 *
 *   §F1  o lado que RECUPERA: a passagem é permutação, e devolve tudo
 *   §F2  o lado que ASSINA: as fibras, e a partição que elas fazem
 *   §F3  as quatro assinaturas próprias — os puros
 *   §F4  a assinatura tem resolução, e ela é um inteiro
 *   §F5  a mesma assinatura nos dois meios, o que é força e não limitação
 *
 *   cc -O2 -std=c99 defeito.c -o defeito && ./defeito
 */
#include <stdio.h>
#include <stdint.h>

#include "unidade.h"
static int bits(unsigned x){ int n = 0; while(x){ n += (int)(x & 1); x >>= 1; } return n; }

int main(void){
printf("\n=== A ASSINATURA, CONTADA =================================================\n");
printf("    Sem gaussiana, sem float, sem amostra: 256 bytes, varridos inteiros.\n");
printf("    E sem chamar de perda o que é nome.\n");

/* ---------------------------------------------------------------- §F1 ------ */
printf("\n§F1  O lado que RECUPERA: a passagem é permutação, e devolve tudo.\n\n");
{
    /* (x,y) -> (m x + y, x) mod 2^4, com m = 1: enumeração exaustiva */
    int visto[256]; for(int i = 0; i < 256; i++) visto[i] = 0;
    int N = 16, distintas = 0;
    for(int x = 0; x < N; x++) for(int y = 0; y < N; y++){
        int nx = (x + y) % N, ny = x;
        if(!visto[nx*N + ny]){ visto[nx*N + ny] = 1; distintas++; }
    }
    printf("      pares de entrada .............................. %d\n", N*N);
    printf("      imagens distintas ............................. %d\n", distintas);
    ok("a contração não perde nada: bijeção exata", distintas == N*N);
    printf("\n      Quem atravessa o gato volta idêntico, e a volta é o esquilo. Este é o lado\n");
    printf("      da RECUPERAÇÃO, e ele está inteiro: nenhum bit some. Guardar isto importa,\n");
    printf("      porque é o que impede confundir os dois papéis mais adiante.\n");
}

/* ---------------------------------------------------------------- §F2 ------ */
printf("\n§F2  O lado que ASSINA: a contagem guarda QUANTOS, e é isso que ela promete.\n\n");
{
    /* b |-> (pop(b & 0xAA), pop(b & 0x55)) : 8 bits entram, dois números de 0..4 saem */
    int fibra[5][5]; for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++) fibra[p][i] = 0;
    for(int b = 0; b <= 255; b++) fibra[bits(b & 0xAA)][bits(b & 0x55)]++;

    int soma = 0, imagens = 0, maior = 0, menor = 999;
    for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++){
        soma += fibra[p][i];
        if(fibra[p][i]){ imagens++; if(fibra[p][i] > maior) maior = fibra[p][i];
                         if(fibra[p][i] < menor) menor = fibra[p][i]; }
    }
    printf("      tamanho das fibras (linhas = par, colunas = ímpar):\n\n          ");
    for(int i = 0; i < 5; i++) printf("%4d", i);
    printf("\n");
    for(int p = 0; p < 5; p++){
        printf("      %d   ", p);
        for(int i = 0; i < 5; i++) printf("%4d", fibra[p][i]);
        printf("\n");
    }
    printf("\n      entradas ...................................... 256\n");
    printf("      soma das fibras ............................... %d\n", soma);
    printf("      imagens distintas ............................. %d\n", imagens);
    printf("      maior fibra ................................... %d      menor: %d\n", maior, menor);
    ok("as fibras PARTICIONAM: soma = 256, nada criado nem perdido", soma == 256);
    ok("e as imagens são 25: a assinatura tem 25 valores", imagens == 25);
    printf("\n      A quebra continua sendo partição, e cada fibra é C(4,par)·C(4,ímpar) —\n");
    printf("      combinatória, não medida. E a fibra NÃO é o que se perdeu: é a CLASSE, o\n");
    printf("      conjunto dos que assinam igual. Sem alguma fibra não haveria nome nenhum:\n");
    printf("      256 pontos e 256 classes é o repouso, onde nada significa (significado.c).\n");
}

/* ---------------------------------------------------------------- §F3 ------ */
printf("\n§F3  As quatro assinaturas PRÓPRIAS: os que assinam sozinhos.\n\n");
{
    int fibra[5][5]; for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++) fibra[p][i] = 0;
    for(int b = 0; b <= 255; b++) fibra[bits(b & 0xAA)][bits(b & 0x55)]++;

    int sozinhos = 0;
    printf("      byte   par ímpar   fibra   recupera-se?\n");
    for(int b = 0; b <= 255; b++){
        int p = bits(b & 0xAA), i = bits(b & 0x55);
        if(fibra[p][i] == 1){
            sozinhos++;
            printf("      0x%02X   %3d %5d   %5d   sim ✓\n", b, p, i, fibra[p][i]);
        }
    }
    printf("\n      bytes que voltam inteiros ..................... %d de 256\n", sozinhos);
    ok("exatamente QUATRO têm assinatura própria", sozinhos == 4);
    printf("\n      E não são quatro quaisquer: o vazio (0x00), a metade par inteira (0xAA), a\n");
    printf("      metade ímpar inteira (0x55) e o cheio (0xFF) — os PUROS: nada, um lado, o\n");
    printf("      outro lado, tudo. Assinar sozinho é privilégio de quem não tem mistura; e\n");
    printf("      note que assinatura própria é justamente a que NÃO forma classe. Os quatro\n");
    printf("      puros são singulares, e por isso não nomeiam grupo nenhum.\n");
}

/* ---------------------------------------------------------------- §F4 ------ */
printf("\n§F4  A assinatura tem RESOLUÇÃO, e ela é um inteiro exato.\n\n");
{
    int fibra[5][5]; for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++) fibra[p][i] = 0;
    for(int b = 0; b <= 255; b++) fibra[bits(b & 0xAA)][bits(b & 0x55)]++;

    long colidem = 0, total_pares = 256L*255L/2L;
    for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++){
        long f = fibra[p][i];
        colidem += f*(f-1)/2;                    /* pares que assinam igual */
    }
    printf("      pares de bytes distintos ...................... %ld\n", total_pares);
    printf("      pares que assinam IGUAL ...................... %ld\n", colidem);
    printf("      pares que a assinatura separa ................ %ld\n", total_pares - colidem);
    ok("a resolução é um inteiro, não uma variância", colidem > 0);
    printf("\n      Esta é a resolução da assinatura, e é um inteiro exato: %ld pares que ela\n", colidem);
    printf("      escolhe não separar. Nenhuma tolerância, nenhuma casa decimal. E as 25\n");
    printf("      classes são mais grossas que os 256 pontos SEM os cruzar — o retículo de\n");
    printf("      refinamentos outra vez (instrumento.c): quem vê menos vê um pedaço do que\n");
    printf("      quem vê mais, nunca outra coisa. Grosso não é errado; é outra resolução.\n");
}

/* ---------------------------------------------------------------- §F5 ------ */
printf("\n§F5  A MESMA assinatura nos dois meios — e isso é força, não limitação.\n\n");
{
    /* no nó, a corrente é a soma dos bits: o nó lê QUANTOS, nunca QUAIS */
    int fibra[5][5]; for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++) fibra[p][i] = 0;
    int mau = 0;
    for(int b = 0; b <= 255; b++){
        int ip = 0, ii = 0;                       /* correntes nos dois ramos */
        for(int k = 0; k < 8; k++){
            int bit = (b >> k) & 1;
            if(!bit) continue;
            if(k % 2) ip += 1; else ii += 1;      /* ramo par / ramo ímpar */
        }
        if(ip != bits(b & 0xAA) || ii != bits(b & 0x55)) mau++;
        fibra[ip][ii]++;
    }
    int imagens = 0, soma = 0;
    for(int p = 0; p < 5; p++) for(int i = 0; i < 5; i++){ soma += fibra[p][i]; if(fibra[p][i]) imagens++; }
    printf("      correntes distintas no nó ..................... %d\n", imagens);
    printf("      soma das fibras do lado analógico ............. %d\n", soma);
    ok("o nó lê exatamente as mesmas 25 classes", imagens == 25 && soma == 256);
    ok("e ramo a ramo bate o digital, byte a byte", mau == 0);
    printf("\n      Não é o analógico sendo pior: é a MESMA aplicação. A corrente num nó é a\n");
    printf("      soma, e soma não guarda parcela — então a assinatura é a mesma nos dois\n");
    printf("      meios, e isso é FORÇA: uma assinatura que dependesse do meio não serviria\n");
    printf("      para identificar nada. Se fosse artefato de implementação, os números\n");
    printf("      seriam dois; são um.\n");
}

printf("\n=== O QUE A CONTAGEM FAZ ===================================================\n");
printf("  Ela não perde: ela ASSINA. 256 bytes entram, 25 assinaturas saem, 4 delas são\n");
printf("  próprias (os puros: nada, um lado, o outro, tudo) e 2322 pares escolhem assinar\n");
printf("  igual — que é o mesmo que dizer: formam classe. A lei do espelho está de pé (as\n");
printf("  fibras somam 256), e é por ela estar de pé que se pode falar com exatidão.\n");
printf("  E os dois papéis ficam separados, que era o erro: o GATO recupera (bijeção,\n");
printf("  nada some) e a CONTAGEM nomeia (assinatura, agrupa por projeto). Perguntar\n");
printf("  quanto a assinatura perdeu é perguntar a um sobrenome por que não distingue\n");
printf("  irmãos. O defeito era meu, de leitura — os números sempre estiveram certos.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, e exato aqui quer dizer contado.\n\n");
return 0;
}
