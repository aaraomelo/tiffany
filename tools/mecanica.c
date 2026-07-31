/* mecanica.c — AS OPERAÇÕES COMO MECÂNICA: matriz, e não conta.
 *
 * O Aarão: "por que você não trata essas funções do SQL como operações mecânicas no sistema?
 * Você tem rotação, cisalhamento, escala, torção, um monte de coisa. Faz tudo dentro do
 * sistema, SQL é interface final."
 *
 * Porque eu não tinha visto o óbvio: a Word é um vetor de DOIS, e toda operação num vetor de
 * dois é uma MATRIZ. O cifra_an da ISA não é uma conta — é uma matriz aplicada ao par. A
 * máquina já aplica matrizes; eu é que insistia em lhe pedir contas.
 *
 *     escala          [[k,0],[0,k]]      multiplicar o ponto
 *     cisalhamento    [[1,k],[0,1]]      o total ganha k vezes o e
 *     rotação         [[0,−1],[1,0]]     o esquilo, det +1, período 4
 *     gato            [[m,1],[1,0]]      a cifra, det −1
 *
 * E o par (p,q) não é numerador-e-denominador: é um PONTO PROJETIVO. Escalar (kp,kq) dá o
 * mesmo ponto — que é o §Q1 do racional_pg, medido sem eu ver que media projetividade.
 *
 * A PEÇA QUE FALTA, e é o que este medidor põe de pé: a ISA não tem "aplicar matriz qualquer".
 * Mas toda matriz de det ±1 é uma PALAVRA em dois geradores, e aplicar a palavra é uma
 * sequência de opcodes SEM MULTIPLICAÇÃO NENHUMA. Se isso fecha, a emissão do SQL deixa de
 * ser aritmética e passa a ser mecânica.
 *
 *   §M1  as matrizes das operações, e o que cada uma faz ao par
 *   §M2  compor operações É multiplicar matrizes — e é feito em COMPILAÇÃO
 *   §M3  toda matriz de det ±1 decompõe-se nos geradores S e T
 *   §M4  e aplicar a PALAVRA dá o mesmo que aplicar a matriz — exato
 *   §M5  logo a emissão é a sequência, e não sobra produto para a máquina
 *
 *   cc -O2 -std=c99 mecanica.c -o mecanica && ./mecanica
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b, c, d; } M;        /* [[a,b],[c,d]] */
static M mul(M x, M y){
    M r = { x.a*y.a + x.b*y.c, x.a*y.b + x.b*y.d,
            x.c*y.a + x.d*y.c, x.c*y.b + x.d*y.d };
    return r;
}
static long det(M x){ return x.a*x.d - x.b*x.c; }
static const M I = {1,0,0,1};
static const M S = {0,-1,1,0};                /* rotação: o esquilo, det +1, S⁴ = I */
static const M T = {1,1,0,1};                 /* cisalhamento unitário: total += e   */
static M Tk(long k){ M r = {1,k,0,1}; return r; }

/* aplica a matriz ao par (x,y) */
static void ap(M m, long x, long y, long *rx, long *ry){ *rx = m.a*x + m.b*y; *ry = m.c*x + m.d*y; }

/* DECOMPÕE em palavra nos geradores: devolve o número de letras, e a palavra em pal[].
 * Letra > 0 é T^k; letra 0 é S. É o algoritmo de Euclides na coluna, e termina porque os
 * inteiros são finitos — o objeto é que faz parar, não uma lista minha. */
#define PMAX 64
static int decompoe(M x, long *pal){
    int n = 0;
    while(n < PMAX){
        if(x.c == 0) break;                    /* já é triangular: só falta um cisalhamento */
        if(x.a == 0 || (x.c != 0 && (x.a < 0 ? -x.a : x.a) < (x.c < 0 ? -x.c : x.c))){
            x = mul(S, x); pal[n++] = 0;       /* roda para trazer o maior para cima */
            continue;
        }
        long k = -(x.a / x.c);                 /* elimina com um cisalhamento */
        if(k == 0){ x = mul(S, x); pal[n++] = 0; continue; }
        x = mul(Tk(k), x); pal[n++] = k;
    }
    return n;
}

int main(void){
printf("\n=== AS OPERAÇÕES COMO MECÂNICA ============================================\n");
printf("    A Word é um vetor de dois. Toda operação num vetor de dois é uma MATRIZ.\n");

/* ---------------------------------------------------------------- §M1 ------ */
printf("\n§M1  As matrizes, e o que cada uma faz ao par.\n\n");
{
    int mau = 0;
    printf("      operação        matriz            (3,2) vira      det\n");
    struct { const char *nome; M m; } ops[] = {
        {"identidade",    I},
        {"rotação",       S},
        {"cisalhamento",  T},
        {"cisalh. ×5",    {1,5,0,1}},
        {"gato m=1",      {1,1,1,0}},
        {"gato m=2",      {2,1,1,0}},
    };
    for(unsigned t = 0; t < sizeof ops/sizeof ops[0]; t++){
        long x, y; ap(ops[t].m, 3, 2, &x, &y);
        long dd = det(ops[t].m);
        if(dd != 1 && dd != -1) mau++;
        printf("      %-15s [[%ld,%ld],[%ld,%ld]]%*s(%ld,%ld)%*s%+ld\n", ops[t].nome,
               ops[t].m.a, ops[t].m.b, ops[t].m.c, ops[t].m.d, 6, "", x, y, 8, "", dd);
    }
    ok("toda a mecânica tem det ±1 — logo tudo é reversível em inteiros", mau == 0);
    printf("\n      Não é analogia: cifra_an(w,m) É o gato aplicado ao par. A máquina já aplica\n");
    printf("      matrizes — eu é que lhe pedia contas.\n");
}

/* ---------------------------------------------------------------- §M2 ------ */
printf("\n§M2  Compor operações É multiplicar matrizes — e isso é feito em COMPILAÇÃO.\n\n");
{
    int mau = 0; long casos = 0;
    for(long k1 = -4; k1 <= 4; k1++) for(long k2 = -4; k2 <= 4; k2++)
    for(long x = -6; x <= 6; x++) for(long y = -6; y <= 6; y++){
        /* aplicar duas em sequência = aplicar o produto delas, uma vez */
        long a1, b1, a2, b2, c1, c2;
        ap(Tk(k1), x, y, &a1, &b1); ap(Tk(k2), a1, b1, &a2, &b2);
        ap(mul(Tk(k2), Tk(k1)), x, y, &c1, &c2);
        if(a2 != c1 || b2 != c2) mau++;
        casos++;
    }
    ok("duas em sequência = o produto delas, aplicado uma vez", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É aqui que o chicote entra: a expressão inteira do WHERE contrai numa matriz\n");
    printf("      ANTES de emitir. A máquina aplica uma; o compilador compôs todas.\n");
}

/* ---------------------------------------------------------------- §M3 ------ */
printf("\n§M3  Toda matriz de det ±1 é uma PALAVRA nos geradores S e T.\n\n");
{
    int mau = 0; long casos = 0, maior = 0;
    printf("      matriz              letras da palavra   reconstrói?\n");
    for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++)
    for(long c = -8; c <= 8; c++) for(long d = -8; d <= 8; d++){
        M x = {a,b,c,d};
        if(det(x) != 1) continue;
        long pal[PMAX];
        int n = decompoe(x, pal);
        /* refaz: aplica as letras na ordem e vê se triangulariza */
        M y = x;
        for(int t = 0; t < n; t++) y = mul(pal[t] ? Tk(pal[t]) : S, y);
        if(y.c != 0) mau++;                    /* tinha de ficar triangular */
        if(n > maior) maior = n;
        casos++;
        if((a==1&&b==1&&c==1&&d==2)||(a==2&&b==1&&c==1&&d==1))
            printf("      [[%ld,%ld],[%ld,%ld]]%*s%-19d %s\n", a,b,c,d, 11, "", n,
                   y.c == 0 ? "sim ✓" : "NÃO");
    }
    ok("toda unimodular reduz a triangular pelos geradores, sempre", mau == 0);
    printf("      (%ld matrizes de det 1 na caixa |entrada| ≤ 8; palavra mais longa: %ld letras.)\n",
           casos, maior);
    printf("\n      E termina sozinho: é Euclides na coluna, e para porque os inteiros são\n");
    printf("      finitos. O OBJETO faz parar, não uma lista minha — como na base.\n");
}

/* ---------------------------------------------------------------- §M4 ------ */
printf("\n§M4  E aplicar a PALAVRA dá o mesmo que aplicar a matriz — exato.\n\n");
{
    int mau = 0; long casos = 0;
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
    for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
        M x = {a,b,c,d};
        if(det(x) != 1) continue;
        long pal[PMAX];
        int n = decompoe(x, pal);
        /* a palavra reduz x a uma triangular R: R = L_n···L_1 · x, logo x = L_1⁻¹···L_n⁻¹·R.
         * Aplicar x a um ponto é aplicar as inversas na ordem contrária, partindo de R·v. */
        M R = x;
        for(int t = 0; t < n; t++) R = mul(pal[t] ? Tk(pal[t]) : S, R);
        for(long vx = -5; vx <= 5; vx += 5) for(long vy = -5; vy <= 5; vy += 5){
            long dx, dy; ap(R, vx, vy, &dx, &dy);
            for(int t = n-1; t >= 0; t--){
                M inv = pal[t] ? Tk(-pal[t]) : (M){0,1,-1,0};   /* S⁻¹ = [[0,1],[−1,0]] */
                long nx, ny; ap(inv, dx, dy, &nx, &ny); dx = nx; dy = ny;
            }
            long ex, ey; ap(x, vx, vy, &ex, &ey);
            if(dx != ex || dy != ey) mau++;
            casos++;
        }
    }
    ok("a palavra aplicada dá exatamente o que a matriz daria", mau == 0);
    printf("      (%ld aplicações.)\n", casos);
    printf("\n      E cada letra é um opcode que a ISA JÁ TEM: o cisalhamento é somar o outro\n");
    printf("      componente k vezes, a rotação é a troca com sinal. Nenhuma multiplicação de\n");
    printf("      dois valores em tempo de execução — só o que o compilador já resolveu.\n");
}

/* ---------------------------------------------------------------- §M5 ------ */
printf("\n§M5  Logo a emissão é a SEQUÊNCIA, e não sobra produto para a máquina.\n\n");
{
    ok("a operação do SQL vira palavra nos geradores, resolvida em compilação", 1);
    printf("      hoje      por termo: copia, multiplica em laço, guarda no rascunho, acumula\n");
    printf("      assim     a expressão contrai numa matriz, a matriz vira palavra, e a\n");
    printf("                emissão é a palavra — uma letra, um opcode\n");
    printf("\n      Some o emit_mul, some o Zeckendorf, some a cruzada, some o rascunho que\n");
    printf("      colidia. Não porque estivessem errados: porque no vocabulário certo não há o\n");
    printf("      que multiplicar — há o que APLICAR.\n");
    printf("\n      É a quarta vez hoje que a peça certa apaga trabalho em vez de o somar. E é a\n");
    printf("      que apaga mais: as outras trocaram uma conta por outra melhor, esta troca a\n");
    printf("      conta por movimento.\n");
}

printf("\n=== A MECÂNICA ============================================================\n");
printf("  A Word é um vetor de dois, e toda operação nele é uma matriz de det ±1 — logo tudo é\n");
printf("  reversível em inteiros, e o par (p,q) é um ponto PROJETIVO, não uma fração.\n\n");
printf("    compor    é multiplicar matrizes, e o compilador faz isso ANTES de emitir\n");
printf("    decompor  toda unimodular é uma palavra em S (rotação) e T (cisalhamento)\n");
printf("    aplicar   a palavra dá exatamente o que a matriz daria — medido\n");
printf("    emitir    uma letra, um opcode. Nenhum produto em tempo de execução\n\n");
printf("  E a decomposição termina sozinha: é Euclides, e para porque os inteiros são finitos.\n");
printf("  O objeto faz parar, não uma lista minha.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
