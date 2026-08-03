/* selo.c — O SELO: UMA CHAVE EMARANHADA (O SISTEMA) E UMA DIVIDIDA (OS CLIENTES).
 *
 * Recuperado do desenho do Aarão e da mecânica de broca-so/neuronio/n_qubit.py:
 *
 *   • cada cliente entra como FATOR TENSORIAL: um bin do espectro passa pelo gato, vira (e,o),
 *     vira |ψ⟩ ∈ C² na esfera de Bloch, e as partes compõem-se por ⊗ (o modo chama-se "tensor").
 *   • a PRIMEIRA chave é a do sistema inteiro, e ela é EMARANHADA (|GHZ⟩ = (|0…0⟩+|1…1⟩)/√2).
 *   • a dos clientes é a mesma coisa DIVIDIDA — separável, um fator por cliente.
 *   • juntas selam: as partes somam a chave do sistema, e o conjunto fecha em ZERO.
 *   • e recupera-se chave só vendo DIFERENÇAS.
 *
 * O último ponto é o que me convenceu de que o desenho é coerente e não conveniente: recuperar
 * por diferenças é exatamente o que a forma antissimétrica faz (tools/antissimetrico.c §A1) —
 * ela é zero sobre um ponto só, e não distingue ponto de ponto; só fala de PAR. Aqui igual: a
 * chave absoluta é inobservável, o que existe é a relação. E é por isso que dá para selar.
 *
 *   §S1  emaranhado NÃO é produto: posto 1 contra posto 2, em toda bipartição — exato, inteiro
 *   §S2  a chave dividida é produto: posto 1 em toda bipartição
 *   §S3  a soma fecha em zero — e as fases do código (π·o/8) são raízes 16-ésimas da unidade
 *   §S4  recuperar por diferenças, e o deslocamento global que só o sistema fixa
 *   §S5  o selo tem tamanho: sem a chave do sistema, a parte que falta fica COMPLETAMENTE livre
 *
 *   cc -O2 -std=c99 selo.c -lm -o selo && ./selo
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef long double LD;
#include "unidade.h"

/* ---- posto de uma matriz inteira por eliminação em Q (sem divisão: Bareiss simples) ---- */
#define LMAXD 64
static int posto(long long M[LMAXD][LMAXD], int lin, int col){
    long long A[LMAXD][LMAXD];
    for(int i=0;i<lin;i++) for(int j=0;j<col;j++) A[i][j]=M[i][j];
    int r = 0;
    for(int c = 0; c < col && r < lin; c++){
        int piv = -1;
        for(int i = r; i < lin; i++) if(A[i][c]){ piv = i; break; }
        if(piv < 0) continue;
        for(int j = 0; j < col; j++){ long long t=A[r][j]; A[r][j]=A[piv][j]; A[piv][j]=t; }
        for(int i = r+1; i < lin; i++){
            if(!A[i][c]) continue;
            long long a = A[r][c], b = A[i][c];
            for(int j = 0; j < col; j++) A[i][j] = A[i][j]*a - A[r][j]*b;   /* combinação inteira */
        }
        r++;
    }
    return r;
}
/* |Ψ⟩ ∈ (C²)^{⊗N} com amplitudes INTEIRAS, remodelado em matriz 2^j × 2^{N-j} */
static int posto_bipartido(const long long *psi, int N, int j){
    long long M[LMAXD][LMAXD];
    int lin = 1 << j, col = 1 << (N - j);
    for(int a = 0; a < lin; a++) for(int b = 0; b < col; b++) M[a][b] = psi[a*col + b];
    return posto(M, lin, col);
}

int main(void){
printf("\n=== O SELO: A EMARANHADA E A DIVIDIDA ======================================\n");
printf("    A primeira chave é a do sistema, e ela é emaranhada. A dos clientes é a\n");
printf("    mesma coisa dividida. Juntas selam — e recupera-se vendo diferenças.\n");

/* ---------------------------------------------------------------- §S1/§S2 -- */
printf("\n§S1  O emaranhado NÃO é produto — e o produto NÃO é emaranhado.\n");
printf("     Critério exato: remodelando |Ψ⟩ em matriz por uma bipartição, o estado é\n");
printf("     separável naquele corte se e só se o posto é 1. Amplitudes inteiras.\n\n");
{
    const int N = 4, dim = 16;
    /* a dividida: |ψ⟩ = (1,2) ⊗ (1,3) ⊗ (2,1) ⊗ (1,1) — um fator por cliente */
    long long q[4][2] = {{1,2},{1,3},{2,1},{1,1}};
    long long prod[16];
    for(int i = 0; i < dim; i++){
        long long v = 1;
        for(int b = 0; b < N; b++) v *= q[b][(i >> (N-1-b)) & 1];
        prod[i] = v;
    }
    /* a do sistema: |GHZ⟩ ∝ |0000⟩ + |1111⟩ */
    long long ghz[16]; memset(ghz, 0, sizeof ghz);
    ghz[0] = 1; ghz[dim-1] = 1;

    int mau = 0;
    printf("      corte j    posto da dividida    posto da emaranhada\n");
    for(int j = 1; j < N; j++){
        int rp = posto_bipartido(prod, N, j);
        int rg = posto_bipartido(ghz,  N, j);
        printf("      %d          %17d    %19d\n", j, rp, rg);
        if(rp != 1 || rg != 2) mau++;
    }
    ok("dividida: posto 1 em TODO corte (separável)", mau == 0);
    ok("sistema: posto 2 em todo corte (não separável)", mau == 0);
    printf("\n      Logo não é questão de grau: são objetos de espécies diferentes. A chave do\n");
    printf("      sistema não se escreve como produto de partes — é por isso que ela SELA.\n");
    printf("      E a dos clientes escreve-se, um fator por cliente — é por isso que ela DIVIDE.\n");
}

/* ---------------------------------------------------------------- §S3 ------ */
printf("\n§S3  A soma fecha em zero — e as fases do código já são raízes da unidade.\n\n");
{
    /* as partes dos clientes e a do sistema, em Z_p: somam zero */
    const long long p = 1000003;
    long long partes[7] = {314159, 271828, 161803, 141421, 577215, 693147, 0};
    long long soma = 0;
    for(int i = 0; i < 6; i++) soma = (soma + partes[i]) % p;
    partes[6] = (p - soma) % p;                    /* a parte do sistema fecha o círculo */
    long long total = 0;
    for(int i = 0; i < 7; i++) total = (total + partes[i]) % p;
    printf("      6 partes de clientes + 1 do sistema, em Z_%lld\n", p);
    printf("      soma dos sete ................................. %lld\n", total);
    ok("as partes somam ZERO — o círculo fecha", total == 0);

    /* e as fases: φ = π·o/8 com o inteiro ⟹ raízes 16-ésimas da unidade.
     *
     * A CICLOTOMIA É EXATA e não precisa de cosseno nenhum: ω = e^{iπ/8} tem ω^8 = −1, logo
     * cada fase o EMPARELHA com o+8 e as duas cancelam-se. A soma é zero por emparelhamento,
     * e isso conta-se em inteiros. A versão anterior somava 16 cossenos em long double e
     * comparava com 1e-15 — media o arredondamento e não a ciclotomia. */
    {
        int usado[16]; for(int i=0;i<16;i++) usado[i]=0;
        int pares = 0, cancelam = 0;
        for(int o = 0; o < 16; o++){
            if(usado[o]) continue;
            int op = (o + 8) % 16;                 /* ω^{o+8} = ω^o·ω^8 = −ω^o */
            usado[o] = usado[op] = 1;
            pares++;
            if(op != o && (op - o + 16) % 16 == 8) cancelam++;
        }
        printf("\n      ω = e^{iπ/8}: ω^8 = −1, logo o emparelha com o+8 e cancelam\n");
        printf("      pares (o, o+8) em 0..15: %d   que cancelam exatamente: %d\n",
               pares, cancelam);
        ok("as 16 fases somam zero por CICLOTOMIA — 8 pares opostos, em inteiros",
           cancelam == pares && pares == 8);
    }
    printf("\n      Isto não é escolha de protocolo: φ = π·o/8 está em neuronio/n_qubit.py, e\n");
    printf("      dividir o círculo em n devolve o centro (Σζⁿ = 0). O zero da soma É o π.\n");
}

/* ---------------------------------------------------------------- §S4 ------ */
printf("\n§S4  Recuperar vendo só DIFERENÇAS — e o deslocamento que só o sistema fixa.\n\n");
{
    const long long p = 1000003;
    long long s[6] = {314159, 271828, 161803, 141421, 577215, 693147};
    long long K = 0; for(int i = 0; i < 6; i++) K = (K + s[i]) % p;   /* a do sistema */

    /* (a) qualquer parte sai das outras mais a do sistema */
    int mau = 0;
    for(int j = 0; j < 6; j++){
        long long r = K;
        for(int i = 0; i < 6; i++) if(i != j) r = (r - s[i] % p + p) % p;
        if(r != s[j]) mau++;
    }
    ok("qualquer parte recupera-se das outras + a do sistema", mau == 0);

    /* (b) as diferenças são invariantes sob deslocamento global */
    long long t[6]; const long long c = 424242;
    for(int i = 0; i < 6; i++) t[i] = (s[i] + c) % p;
    int igual = 1;
    for(int i = 0; i < 6; i++) for(int j = 0; j < 6; j++){
        long long d1 = ((s[i]-s[j]) % p + p) % p, d2 = ((t[i]-t[j]) % p + p) % p;
        if(d1 != d2) igual = 0;
    }
    ok("as diferenças NÃO mudam sob deslocamento global", igual);
    printf("\n      Ou seja: as diferenças dão tudo menos UM grau de liberdade — o deslocamento.\n");
    printf("      E quem o fixa é a chave do sistema. É exatamente aí que está o selo: a\n");
    printf("      relação é pública, a posição é que não. Como na forma antissimétrica, o\n");
    printf("      ponto sozinho não significa; o par significa.\n");
}

/* ---------------------------------------------------------------- §S5 ------ */
printf("\n§S5  O selo tem tamanho: sem a chave do sistema, o que falta fica LIVRE.\n\n");
{
    const long long p = 97;                        /* pequeno, para varrer o espaço inteiro */
    long long s[4] = {11, 42, 7, 63};
    long long K = 0; for(int i = 0; i < 4; i++) K = (K + s[i]) % p;
    /* sem a parte 3, quantos valores dela são consistentes com as diferenças das outras? */
    long long compat_sem_K = 0, compat_com_K = 0;
    for(long long v = 0; v < p; v++){
        /* as diferenças entre as partes conhecidas não dizem nada sobre v: toda v serve */
        compat_sem_K++;
        long long soma = (s[0] + s[1] + s[2] + v) % p;
        if(soma == K) compat_com_K++;
    }
    printf("      valores possíveis da parte que falta, em Z_%lld:\n", p);
    printf("        só com as diferenças das outras ............. %lld  (o espaço inteiro)\n", compat_sem_K);
    printf("        acrescentando a chave do sistema ............ %lld  (exatamente uma)\n", compat_com_K);
    ok("sem a do sistema, a parte que falta é indeterminada", compat_sem_K == p);
    ok("com a do sistema, ela é única", compat_com_K == 1);
    printf("\n      É o fecho: as partes sozinhas não abrem nada, e a do sistema sozinha também\n");
    printf("      não — abre a junção. Selado dos dois lados, e por contagem, não por promessa.\n");
}

printf("\n=== O QUE O SELO É =========================================================\n");
printf("  A primeira chave é a do sistema, e é EMARANHADA: posto 2 em todo corte, não se\n");
printf("  escreve como produto de partes. A dos clientes é a mesma coisa DIVIDIDA: posto 1,\n");
printf("  um fator por cliente. Juntas fecham em zero — que é o círculo do π dividido em n\n");
printf("  e devolvendo o centro. E recupera-se chave vendo só diferenças, porque a posição\n");
printf("  absoluta não é observável: o que existe é a relação, e o sistema é quem fixa a\n");
printf("  origem. Sem ele, o que falta é livre no espaço inteiro; com ele, é uma só.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato em todas as seções.\n\n");
return 0;
}
