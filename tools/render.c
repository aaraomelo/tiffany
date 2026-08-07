/* render.c — RENDERIZAR PELO CORPO MORFICO. Sem GPU, sem shader, sem virgula flutuante.
 *
 * O Aarao: «migra tudo pro disco e usa apenas CPU pra renderizar — nao tem tempo, so' latencia»
 *          «ve corpo morfico» · «tira conceito de shader» · «tudo sai de algum corpo do catalogo»
 *
 * As tres correcoes juntas dizem a mesma coisa, e a primeira versao deste ficheiro estava
 * errada nas tres: eu tinha escrito ray-marching com virgula flutuante — um SHADER traduzido
 * para C. Isso e' trazer a maquina de fora e chamar-lhe migracao.
 *
 * O que renderiza aqui e' UM CORPO DO CATALOGO, e o corpo e' o MORFICO. A regua dele, o
 * catalogo di-la: o operador e' a ADJUNCAO
 *
 *      delta  |-  epsilon        dilatacao e erosao
 *
 * e a dilatacao COMPOE por Minkowski — dilatar por B_r e depois por B_s e' dilatar por B_{r+s}.
 * Logo o PARAMETRO da deformacao e' o elemento estruturante, e para as bolas e' o RAIO:
 *
 *      dil(dil(A, B_r), B_s) = dil(A, B_{r+s})       o raio SOMA
 *
 * E o raio e' um INTEIRO. Por isso aqui nao entra virgula flutuante nenhuma: a forma nasce de
 * pontos, cresce por dilatacao com raio inteiro, e o relogio da' o raio. Nao ha' raio a
 * marchar, nao ha' normal a estimar, nao ha' shader.
 *
 * E o TEMPO nao entra: o frame depende da FASE e nao de quando se pede. Por isso pre-renderizar
 * e' legitimo — geram-se as fases uma vez, ficam no disco, e o que se faz depois e' LER:
 * MOVE(slot,+1), e mais nada.
 *
 *   ./render [fases] [ordem]     escreve PGM por fase em /tmp/render/
 *                                 a ORDEM e' a simetria que a op do card declara
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define W 96
#define H 96

/* a DILATACAO por uma bola de raio r: cada ponto aceso acende os que estao a distancia <= r.
 * a distancia e' a do quadrado (Chebyshev), que e' a bola do reticulado — inteira. */
static void dilata(const unsigned char *a, unsigned char *out, long r)
{
    for(long j = 0; j < H; j++)
        for(long i = 0; i < W; i++){
            unsigned char m = 0;
            for(long dj = -r; dj <= r && !m; dj++)
                for(long di = -r; di <= r && !m; di++){
                    long y = j + dj, x = i + di;
                    if(y >= 0 && y < H && x >= 0 && x < W && a[y*W + x]) m = 1;
                }
            out[j*W + i] = m;
        }
}

/* a EROSAO — o dual: um ponto so' fica se TODOS os vizinhos estiverem acesos */
static void erode(const unsigned char *a, unsigned char *out, long r)
{
    for(long j = 0; j < H; j++)
        for(long i = 0; i < W; i++){
            unsigned char m = 1;
            for(long dj = -r; dj <= r && m; dj++)
                for(long di = -r; di <= r && m; di++){
                    long y = j + dj, x = i + di;
                    if(y < 0 || y >= H || x < 0 || x >= W || !a[y*W + x]) m = 0;
                }
            out[j*W + i] = m;
        }
}

int main(int argc, char **argv)
{
    long nf = (argc > 1) ? atol(argv[1]) : 16;
    mkdir("/tmp/render", 0755);

    /* O GERME SAI DA OP DO CARD, e nao e' o mesmo para todos: a op declara uma simetria, e a
     * simetria diz quantos pontos ha' e onde. Um card com «ordem 6» nasce de seis pontos; um
     * com «φ» nasce do par; um com «n=5» nasce de cinco. E' o corpo do card a dar o germe,
     * como o corpo morfico da' a lei que o faz crescer. */
    long ordem = (argc > 2) ? atol(argv[2]) : 2;      /* a simetria, lida da op */
    if(ordem < 1) ordem = 1;
    static unsigned char germe[W*H], a[W*H], b[W*H];
    memset(germe, 0, sizeof germe);
    /* os pontos, igualmente espacados na volta — e a volta e' o relogio: o angulo por ponto
     * e' a fraccao 1/ordem, e as posicoes saem por contagem inteira sobre um circulo de
     * raio fixo. Sem trigonometria: usa-se a tabela do quadrado (a bola do reticulado). */
    { long R = 18;
      for(long k = 0; k < ordem; k++){
          /* a volta partida em `ordem` — em passos inteiros pelo perimetro do quadrado */
          long per = 8*R, t = (k * per) / ordem, x, y;
          if     (t <  2*R) { x = -R + t;        y = -R; }
          else if(t <  4*R) { x =  R;            y = -R + (t - 2*R); }
          else if(t <  6*R) { x =  R - (t-4*R);  y =  R; }
          else              { x = -R;            y =  R - (t - 6*R); }
          long j = H/2 + y, i = W/2 + x;
          if(j >= 0 && j < H && i >= 0 && i < W) germe[j*W + i] = 1;
      } }

    FILE *idx = fopen("/tmp/render/indice.txt", "wb");
    long escritos = 0;

    for(long f = 0; f < nf; f++){
        /* O RELOGIO DA' O RAIO, e ele e' inteiro: a fase percorre a volta e o raio sobe e
         * desce com ela — dilata na ida, erode na volta. E' o par, e nao uma animacao. */
        long meio = nf / 2;
        long r = (f <= meio) ? f : (nf - f);          /* sobe e desce: a volta fecha */

        if(r > 0) dilata(germe, a, r); else memcpy(a, germe, sizeof a);
        /* e a segunda metade da adjuncao: erodir por 1 devolve a fronteira */
        erode(a, b, 1);
        for(long k = 0; k < W*H; k++) b[k] = (unsigned char)(a[k] && !b[k]);  /* a borda */

        static unsigned char img[W*H];
        for(long k = 0; k < W*H; k++)
            img[k] = (unsigned char)(b[k] ? 235 : (a[k] ? 110 : 18));

        char nome[256]; snprintf(nome, sizeof nome, "/tmp/render/fase_%03ld.pgm", f);
        FILE *g = fopen(nome, "wb");
        fprintf(g, "P5\n%d %d\n255\n", W, H);
        fwrite(img, 1, W*H, g);
        fclose(g);
        fprintf(idx, "fase_%03ld.pgm raio=%ld\n", f, r);
        escritos++;
    }
    fclose(idx);
    printf("renderizadas %ld fases pelo CORPO MORFICO, com germe de ordem %ld,"
           " em CPU e sem virgula flutuante\n", escritos, ordem);
    printf("o raio e' INTEIRO e vem do relogio; a dilatacao compoe (Minkowski) e a erosao e' o dual\n");
    printf("sem shader, sem GPU, sem tempo: o frame depende da FASE e nao de quando se pede\n");
    return 0;
}
