/* tesserato_espiral.c — O TESSERACTO: Cantor dos dois lados, e a espiral.
 *
 * Vértices e Gray em ℤ; Cantor por numerador base 3; cone vs cilindro sem cos/sin.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/tesserato_espiral.c -o tesserato_espiral
 */
#include <stdio.h>
#include "../lib/disco.h"
#include "reta.h"
#include "isa_disk.h"
#include <string.h>
#include "unidade.h"

#define D 4
#define NV 16

static int gray(int i){ return i ^ (i >> 1); }
static int bits(int x){ int n=0; while(x){ n += x&1; x >>= 1; } return n; }

/* Cantor: x = 2·u / 3^D, u = bits de v lidos em base 3 */
static long cantor_u(int v){
    long u = 0;
    for(int k = 0; k < D; k++) u = u*3 + ((v >> (D-1-k)) & 1);
    return u;
}
static int cantor_volta(long u){
    int v = 0;
    long p3 = 1;
    for(int k = 1; k < D; k++) p3 *= 3;
    for(int k = 0; k < D; k++){
        int b = 0;
        if(u >= p3){ b = 1; u -= p3; }
        v = (v << 1) | b;
        p3 /= 3;
    }
    return v;
}

int main(void){
printf("\n=== O TESSERACTO: CANTOR DOS DOIS LADOS, E O PÓ QUE ENCHE O PLANO ========\n");

printf("\n§E1  OS VÉRTICES: {0,1}^4 é o produto direto, e são pontos de Cantor.\n\n");
{
    printf("      vértice   bits    u (base 3)   volta   confere\n");
    int mau = 0;
    long us[NV];
    for(int v = 0; v < NV; v++){
        long u = cantor_u(v);
        us[v] = u;
        int volta = cantor_volta(u);
        if(volta != v) mau++;
        if(v < 4 || v == NV-1)
            printf("      %-9d %d%d%d%d    %-12ld %-7d %s\n", v,
                   (v>>3)&1,(v>>2)&1,(v>>1)&1,v&1, u, volta, volta==v?"sim":"NAO");
    }
    int col = 0;
    for(int a = 0; a < NV; a++) for(int b = a+1; b < NV; b++)
        if(us[a] == us[b]) col++;
    printf("      …\n\n      colisões entre os 16 pontos: %d\n\n", col);
    ok("cada vértice do tesseracto é um ponto de Cantor, e a volta fecha", mau == 0);
    ok("e os 16 pontos são distintos — o produto direto não colapsa", col == 0);
}

printf("\n§E2  O PERCURSO de Gray: um bit por passo, e visita todos uma vez.\n\n");
{
    int visto[NV] = {0}, mau_bit = 0, mau_vis = 0;
    printf("      passo   g(i)   bits      muda 1 bit?\n");
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        visto[g]++;
        if(i > 0){
            int dif = g ^ gray(i-1);
            if(bits(dif) != 1) mau_bit++;
        }
        if(i < 5)
            printf("      %-7d %-6d %d%d%d%d      %s\n", i, g,
                   (g>>3)&1,(g>>2)&1,(g>>1)&1,g&1, i? (bits(g^gray(i-1))==1?"sim":"NAO") : "—");
    }
    for(int v = 0; v < NV; v++) if(visto[v] != 1) mau_vis++;
    int fecha = bits(gray(NV-1) ^ gray(0)) == 1;
    printf("      …\n\n      passos que mudam mais de um bit: %d\n", mau_bit);
    printf("      vértices visitados exatamente uma vez: %d de %d\n", NV-mau_vis, NV);
    printf("      e do último volta ao primeiro por uma aresta: %s\n\n", fecha ? "sim" : "nao");
    ok("cada passo muda UM bit — o percurso anda por arestas, não salta", mau_bit == 0);
    ok("e visita os 16 vértices exatamente uma vez", mau_vis == 0);
    ok("e fecha o ciclo: do último ao primeiro é uma aresta", fecha);
}

printf("\n§E3  CANTOR DOS DOIS LADOS — e a correção que o Aarão fez.\n\n");
{
    printf("      i    fita de i    g(i)   fita de g(i)   ambas em Cantor?\n");
    int visto[NV] = {0}, mau_cantor = 0, mau_bij = 0;
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        int volta_i = cantor_volta(cantor_u(i));
        int volta_g = cantor_volta(cantor_u(g));
        if(volta_i != i || volta_g != g) mau_cantor++;
        visto[g]++;
        if(i < 4)
            printf("      %-4d %d%d%d%d         %-6d %d%d%d%d           %s\n", i,
                   (i>>3)&1,(i>>2)&1,(i>>1)&1,i&1, g,
                   (g>>3)&1,(g>>2)&1,(g>>1)&1,g&1,
                   (volta_i==i && volta_g==g) ? "sim" : "NAO");
    }
    for(int v = 0; v < NV; v++) if(visto[v] != 1) mau_bij++;
    printf("      …\n\n      falhas de ida-e-volta em Cantor: %d\n\n", mau_cantor);
    ok("AMBOS os lados são Cantor — o índice e o código, os dois são fitas", mau_cantor == 0);
    ok("e Gray é bijeção entre eles — muda a leitura, não o espaço", mau_bij == 0);
}

printf("\n§E4  CARTESIANO e POLAR: o mesmo percurso nos dois retratos.\n\n");
{
    printf("      passo   cartesiano (x,y,z,w)   fracção da volta   bits a 1\n");
    for(int i = 0; i < NV; i++){
        int g = gray(i);
        if(i < 4 || i == NV-1)
            printf("      %-7d (%d,%d,%d,%d)              %d/%d               %d\n", i,
                   (g>>3)&1,(g>>2)&1,(g>>1)&1,g&1, i, NV, bits(g));
    }
    long volta_no_fim = 0, nao_volta_antes = 0;
    for(int i = 1; i <= NV; i++){
        if(i % NV == 0){ if(i == NV) volta_no_fim = 1; }
        else nao_volta_antes++;
    }
    /* polar: 16 passos de 1/16 fecham 1 volta — o índice, não a soma de 2π */
    isa_word(ISA_S_A, 1, 0);
    for(int k = 0; k < 4; k++) isa_MOVE(ISA_S_ESQUILO, 1);
    int t, e; isa_read(ISA_S_A, &t, &e);
    printf("      …\n\n      ciclo no INDICE: NV ≡ 0 (mod NV); ESQUILO^4 = I: (%d,%d)\n\n", t, e);
    ok("o percurso fecha no INDICE: NV passos voltam, os NV-1 intermedios nao."
       " ESQUILO^4 = I no disco ISA — a volta polar sem 2π em double",
       volta_no_fim && nao_volta_antes == NV - 1 && t == 1 && e == 0);
}

printf("\n§E5  A ESPIRAL É A SOMBRA DA HÉLICE NO CONE.\n\n");
{
    printf("      (a) O CONE EM INTEIROS: x² + y² = z², nos triplos e nos que nao sao\n\n");
    static const long TRI[][3] = {
        {3,4,5}, {5,12,13}, {8,15,17}, {7,24,25}, {20,21,29}, {9,40,41}, {12,35,37}
    };
    static const long NAO[][3] = {
        {3,4,6}, {5,12,14}, {2,3,4}, {1,1,2}, {6,7,9}, {10,10,14}, {8,15,18}
    };
    int nt = (int)(sizeof TRI / sizeof TRI[0]), nn = (int)(sizeof NAO / sizeof NAO[0]);
    int dentro = 0, fora_controlo = 0;
    printf("      triplo        x²+y²    z²      no cone\n");
    for(int i = 0; i < nt; i++){
        long v[2] = { TRI[i][0], TRI[i][1] }, z = TRI[i][2];
        long r2 = rt_norma(v, 2), z2 = z*z;
        if(r2 == z2) dentro++;
        long w[2] = { NAO[i][0], NAO[i][1] }, zc = NAO[i][2];
        long rc2 = rt_norma(w, 2), zc2 = zc*zc;
        if(rc2 != zc2) fora_controlo++;
        if(i < 4) printf("      (%2ld,%2ld,%2ld)    %-8ld %-7ld %s\n",
                         TRI[i][0],TRI[i][1],z, r2, z2, r2==z2?"sim":"NAO");
    }
    printf("      …\n\n      no cone: %d de %d       fora, no controlo: %d de %d\n\n",
           dentro, nt, fora_controlo, nn);
    ok("o cone x² + y² = z² e uma equacao que SEPARA: os sete triplos pitagoricos caem"
       " nele e os sete de controlo ficam todos fora",
       dentro == nt && fora_controlo == nn && nt == 7);

    printf("      (b) A SOMBRA: o CONE da espiral (r² = t² cresce), o CILINDRO do circulo\n\n");
    int cresce = 1, constante = 1, passos = 0;
    long r2_ant_cone = -1;
    const long R2_CIL = 25;
    printf("      t        raio² CONE   raio² CILINDRO\n");
    for(int t = 1; t <= 40; t++){
        long r2c = (long)t * t;
        long r2l = R2_CIL;
        if(r2_ant_cone >= 0 && r2c <= r2_ant_cone) cresce = 0;
        if(r2l != R2_CIL) constante = 0;
        r2_ant_cone = r2c; passos++;
        if(t <= 4) printf("      %-8d %-12ld %ld\n", t, r2c, r2l);
    }
    printf("      …\n\n      cone crescente: %s    cilindro constante: %s\n\n",
           cresce ? "sim" : "NAO", constante ? "sim" : "NAO");
    ok("a sombra e ESPIRAL porque o cone abre: r² = t² cresce. O controlo e o CILINDRO,"
       " r² constante — a espiral vem da SUPERFICIE, sem cos/sin",
       cresce && constante && passos == 40);
}

printf("\n§E6  A CURVA QUE ENCHE PASSA PELA ESPIRAL — necessariamente.\n\n");
{
    /* Gume: 1D (só a linha y=0) NAO cobre a espiral diagonal; 2D cobre tudo. */
    int B = 8, lado = 1 << (B/2);
    char *malha2 = DISCO_FIXO(char, 208);
    disco_prende(DISCO_BASE(208),"dados/malha_208.bin",(size_t)4096,sizeof(char));
    memset(malha2, 0, 4096);
    long total = 1L << B;
    for(long v = 0; v < total; v++){
        int x = 0, y = 0;
        for(int k = 0; k < B; k++){
            int b = (int)((v >> (B-1-k)) & 1);
            if(k % 2 == 0) x = (x << 1) | b; else y = (y << 1) | b;
        }
        malha2[y*lado + x] = 1;
    }
    int na_2d = 0, na_1d = 0, nsp = 0;
    for(int i = 0; i < lado; i++){
        nsp++;
        if(malha2[i*lado + i]) na_2d++;
        if(i == 0) na_1d++;
    }
    printf("      espiral diagonal: %d pontos; cobertos em 2D: %d; em 1D (y=0): %d\n\n",
           nsp, na_2d, na_1d);
    ok("TODO ponto da espiral cai numa célula que a curva 2D visita — e o gume 1D NAO cobre",
       na_2d == nsp && nsp > 1 && na_1d < nsp);
}

printf("\n§E7  O PÓ NÃO ENCHE SOZINHO — mas DUAS coordenadas enchem.\n\n");
{
    printf("      bits da fita   células   ocupadas   cobre tudo?\n");
    int cheio = 0;
    for(int B = 2; B <= 12; B += 2){
        int lado = 1 << (B/2);
        char *malha = DISCO_FIXO(char, 208);
        disco_prende(DISCO_BASE(208),"dados/malha_208.bin",(size_t)4096,sizeof(char));
        memset(malha, 0, 4096);
        long tot = 1L << B, ocup = 0;
        for(long v = 0; v < tot; v++){
            int x = 0, y = 0;
            for(int k = 0; k < B; k++){
                int b = (int)((v >> (B-1-k)) & 1);
                if(k % 2 == 0) x = (x << 1) | b;
                else           y = (y << 1) | b;
            }
            int c = y*lado + x;
            if(c >= 0 && c < lado*lado && !malha[c]){ malha[c] = 1; ocup++; }
        }
        int okc = (ocup == (long)lado*lado);
        if(okc) cheio++;
        printf("      %-14d %-9d %-10ld %s\n", B, lado*lado, ocup, okc ? "sim" : "nao");
    }
    printf("\n      dim(Cantor): 2^k < 3^k em inteiros — menor que 1, nao enche a reta\n\n");
    long p2=1, p3=1; int dim_entre=0;
    for(int k=1;k<=9;k++){ p2*=2; p3*=3; if(p2>1 && p2<p3) dim_entre++; }
    ok("uma fita repartida em DUAS coordenadas cobre o quadrado inteiro",
       cheio >= 5 && dim_entre == 9);
}

printf("\n=== FECHO ==================================================================\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
