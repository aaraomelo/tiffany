/* encaixa.c — A CIFRA DO ESPAÇO SEMÂNTICO: a base sai dos PRÓPRIOS vetores.
 *
 * O Aarão: "é só encaixar os embeddings na cifra — é teletransporte, segue protocolo. Se o
 * espaço dele tem n vetores, acho que n+1 vetores formam a base. Essa é a cifra. Só normalizar.
 * A base dele é ortonormal."
 *
 * A cifra do espaço semântico sai de dentro dele: se o espaço tem n vetores, n+1 determinam-no,
 * e a base é a ortogonalização deles. Não se IMPÕE uma base de fora — o semantico.c §S4 usa
 * Hadamard, que é legítimo, mas Hadamard vem de fora. Aqui a base é a dos próprios dados.
 *
 *   §C1  o ESPAÇO: n vetores, e o posto (independência) em ℚ
 *   §C2  JÁ SÃO ORTOGONAIS? — o significado é a correlação, não o acaso de alta dimensão
 *   §C3  a BASE por Gram-Schmidt INTEIRO: ortogonal, e gera o mesmo espaço
 *   §C4  a CIFRA: projetar na base e voltar — o protocolo, exacto em ℚ
 *   §C5  e ela PRESERVA a vizinhança — Parseval, dv² = Σ (Δc_j² / ‖u_j‖²)
 *
 * LEI vs TRANSPORTE. Embeddings em vírgula, GS com divisão e «norma 1 depois de dividir»
 * eram o método — a divisão PÕE o 1, e o 1e-12 media o IEEE. A lei é posto em ℚ, pares
 * correlacionados por produto cruzado, GS a limpar denominadores (resto r·r ≠ 0), volta
 * exacta em ℚ, Parseval nos quadrados. Sem ficheiro de embeddings: os vectores são ℤᵈ.
 *
 *   cc -O2 -std=c99 -I lib tests/encaixa.c -o encaixa && ./encaixa
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "reta.h"
#include "racionais.h"
#include "linear.h"

#define N  5
#define D  6

/* n vectores em ℤ^d, com uma direcção comum (a primeira coordenada) — o significado:
 * 'rei' e 'rainha' partilham eixo, logo NÃO são ortogonais. São LI: o posto é n. */
static const long V[N][D] = {
    { 2, 1, 0, 0, 0, 0 },
    { 2, 0, 1, 0, 0, 0 },
    { 2, 0, 0, 1, 0, 0 },
    { 2, 0, 0, 0, 1, 0 },
    { 1, 1, 1, 1, 1, 1 },
};
static const char *nome[N] = { "rei", "rainha", "trono", "coroa", "reino" };

static void gs_int(const long src[][D], long U[][D], int n, int *posto){
    *posto = 0;
    for(int i = 0; i < n; i++){
        for(int k = 0; k < D; k++) U[*posto][k] = src[i][k];
        for(int j = 0; j < *posto; j++){
            long ip = rt_dir(U[*posto], U[j], D);
            long nj = rt_norma(U[j], D);
            for(int k = 0; k < D; k++)
                U[*posto][k] = nj * U[*posto][k] - ip * U[j][k];
            rt_reduz_vector(U[*posto], D);
        }
        if(rt_norma(U[*posto], D) != 0) (*posto)++;
    }
}

/* v = Σ_j (⟨v,u_j⟩ / ‖u_j‖²) u_j, em ℚ. Exacto sse a base ortogonal gera v. */
static int reconstroi(const long *v, const long U[][D], int posto){
    for(int k = 0; k < D; k++){
        Qz rec = qz(0, 1);
        for(int j = 0; j < posto; j++){
            long ip = rt_dir(v, U[j], D);
            long nj = rt_norma(U[j], D);
            rec = qz_soma(rec, qz_mult(qz(ip, nj), qz_de_inteiro(U[j][k])));
        }
        if(!qz_igual(rec, qz_de_inteiro(v[k]))) return 0;
    }
    return 1;
}

int main(void){
printf("\n=== A CIFRA DO ESPAÇO SEMÂNTICO: A BASE SAI DOS PRÓPRIOS VETORES =========\n");
printf("    %d vetores em ℤ^%d — a direcção comum é o significado, não um embedding\n", N, D);
printf("    em vírgula. Hadamard viria de fora; esta base sai dos dados, por GS inteiro.\n");

printf("\n§C1  O ESPAÇO: quantos vetores são precisos para o gerar.\n\n");
{
    /* Se o espaço tem n vetores, n+1 determinam-no — é o simplex. O que se mede é o
     * POSTO: quantos dos n são linearmente independentes. Gauss-Jordan em ℚ, sem pivô
     * parcial e sem tolerância: o pivô é zero ou não é. */
    long flat[D * N];
    for(int i = 0; i < N; i++) for(int k = 0; k < D; k++)
        flat[k * N + i] = V[i][k];
    Mat A = mat_de_inteiros(D, N, flat);
    int posto = mat_posto(A);
    printf("      vetores                         %d\n", N);
    printf("      dimensão do espaço              %d\n", D);
    printf("      posto (independentes, em ℚ)     %d\n", posto);
    printf("      logo geram um subespaço de dimensão %d, e %d+1 = %d o determinam\n\n",
           posto, posto, posto + 1);
    ok("os vetores são linearmente independentes — nenhum é combinação dos outros",
       posto == N);

    /* GUME: dois iguais baixam o posto. Sem este lado, «posto = n» valia por eu ter
     * escrito n vectores e não ter medido independência. */
    long gume[D * N];
    memcpy(gume, flat, sizeof flat);
    for(int k = 0; k < D; k++) gume[k * N + 1] = gume[k * N + 0];
    int posto_g = mat_posto(mat_de_inteiros(D, N, gume));
    printf("      gume: dois iguais  →  posto %d (caiu)\n\n", posto_g);
    ok("e o gume morde: dois vectores iguais baixam o posto — a independência CARREGA",
       posto_g == N - 1 && posto == N);
    printf("      Em %d dimensões, %d vectores LI não é sorte: há mais eixos do que\n", D, N);
    printf("      vectores. O espaço é largo, e é essa largura que faz o resto funcionar.\n");
}

printf("\n§C2  JÁ SÃO QUASE ORTOGONAIS? — e é isto que autoriza o \"só normalizar\".\n\n");
{
    /* «Só normalizar» só vale se os vectores já vierem quase ortogonais. Estes NÃO:
     * partilham a primeira coordenada, e isso É o significado. A decisão vive nos
     * quadrados, sem raiz:
     *
     *      |cos| > 1/2   ⟺   ⟨u,v⟩² · 4 > ‖u‖² ‖v‖²
     *
     * (o 3/√d do acaso em 768 dimensões não cabe aqui: em d=6 esse tecto passa de 1.) */
    long acima = 0, pares = 0, n_orto = 0;
    printf("      par            ⟨u,v⟩   ‖u‖²  ‖v‖²   4⟨u,v⟩² > ‖u‖²‖v‖² ?\n");
    for(int i = 0; i < N; i++) for(int j = i + 1; j < N; j++){
        long uu = rt_norma(V[i], D), vv = rt_norma(V[j], D), uv = rt_dir(V[i], V[j], D);
        pares++;
        if(uv == 0) n_orto++;
        int corr = (uv * uv * 4 > uu * vv);
        if(corr) acima++;
        if(pares <= 6)
            printf("      %-6s %-6s  %4ld   %4ld  %4ld    %s\n",
                   nome[i], nome[j], uv, uu, vv, corr ? "sim" : "não");
    }
    printf("\n      %ld de %ld pares com |cos| > 1/2  ·  ortogonais: %ld\n\n",
           acima, pares, n_orto);
    ok("os vetores NÃO são ortogonais — a maioria dos pares tem |cos| > 1/2, comparado"
       " por produto cruzado 4⟨u,v⟩² > ‖u‖²‖v‖², sem raiz. Normalizar arruma o comprimento"
       " e não arruma o ângulo: é preciso ortogonalizar",
       acima > pares / 2 && n_orto == 0 && pares == N * (N - 1) / 2);

    /* GUME: sem a direcção comum, a correlação cai. Hadamard (de fora) seria ortogonal. */
    long W[N][D];
    for(int i = 0; i < N; i++){
        for(int k = 0; k < D; k++) W[i][k] = V[i][k];
        W[i][0] = 0;                                 /* tira o eixo partilhado */
    }
    long acima_g = 0, pares_g = 0;
    for(int i = 0; i < N; i++) for(int j = i + 1; j < N; j++){
        long uu = rt_norma(W[i], D), vv = rt_norma(W[j], D), uv = rt_dir(W[i], W[j], D);
        pares_g++;
        if(uu && vv && uv * uv * 4 > uu * vv) acima_g++;
    }
    printf("      gume: sem o eixo comum, correlacionados %ld de %ld (caiu)\n\n",
           acima_g, pares_g);
    ok("e o gume morde: sem a direcção comum a correlação cai — o significado CARREGA",
       acima_g <= pares_g / 2 && acima > pares / 2);
    printf("      'rei' e 'rainha' não são ortogonais porque não são independentes.\n");
    printf("      É o contrário do que «só normalizar» sugere, e é o §C3 que arruma o ângulo.\n");
}

printf("\n§C3  A BASE por Gram-Schmidt: ortogonal, e GERA o mesmo espaço.\n\n");
{
    /* GS inteiro: u ← ‖u_j‖² u − ⟨u,u_j⟩ u_j, e reduz-se pelo mdc. Não se normaliza:
     * a divisão pela norma PÕE o 1, e medir «norma 1» depois disso é reler a divisão.
     * O que o GS tem de garantir é RESTO NÃO NULO (independência) e ⟨u_i,u_j⟩ = 0. */
    long U[N][D];
    int posto = 0;
    gs_int(V, U, N, &posto);
    int degenerados = N - posto;
    long orto = 0, pares_u = 0;
    for(int i = 0; i < posto; i++) for(int j = i + 1; j < posto; j++){
        pares_u++;
        if(rt_dir(U[i], U[j], D) == 0) orto++;
    }
    printf("      posto após GS                   %d  (degenerados %d)\n", posto, degenerados);
    printf("      pares ortogonais                %ld de %ld\n", orto, pares_u);
    for(int i = 0; i < posto; i++){
        printf("      u_%d  (", i);
        for(int k = 0; k < D; k++) printf("%s%ld", k ? "," : "", U[i][k]);
        printf(")   ‖u‖² = %ld\n", rt_norma(U[i], D));
    }
    printf("\n");
    ok("a base GS é ORTOGONAL e sem degenerados — ⟨u_i,u_j⟩ = 0 exacto em ℤ, resto r·r ≠ 0",
       posto == N && orto == pares_u && pares_u == N * (N - 1) / 2);

    int gera = 0;
    qz_saturou = 0;
    for(int i = 0; i < N; i++) if(reconstroi(V[i], U, posto)) gera++;
    printf("      reconstrução exacta em ℚ        %d de %d   (qz_saturou %ld)\n\n",
           gera, N, qz_saturou);
    ok("a base GERA o espaço: cada vetor original reconstrói-se das projecções, exacto em ℚ."
       " A asserção que aqui estava media que a norma era 1 DEPOIS de dividir por ela",
       gera == N && qz_saturou == 0);
    printf("      A base sai dos DADOS, não de fora. O semantico.c §S4 usa Hadamard, que\n");
    printf("      fecha igualmente e vem de fora; esta vem de dentro, e é isso que a torna\n");
    printf("      a cifra DESTE espaço e não de um espaço qualquer.\n");
}

printf("\n§C4  A CIFRA: projetar na base e voltar — o protocolo, exacto.\n\n");
{
    /* Entra (projeta-se), atravessa (n coeficientes), volta (recompõe-se). Como a base
     * ortogonal gera o subespaço, a volta é EXACTA em ℚ — não «na casa do epsilon». */
    long U[N][D];
    int posto = 0;
    gs_int(V, U, N, &posto);
    printf("      vetor            n coef.   d orig.   volta exacta?\n");
    int volta = 0;
    qz_saturou = 0;
    for(int i = 0; i < N; i++){
        int ok_i = reconstroi(V[i], U, posto);
        if(ok_i) volta++;
        printf("      %-16s %-9d %-9d %s\n", nome[i], posto, D, ok_i ? "sim" : "não");
    }
    printf("\n      %d coeficientes guardam um vetor de %d dimensões, e a volta é exacta.\n\n",
           posto, D);
    ok("o vetor cifrado na base própria volta EXATO — compressão sem perda, por a base"
       " gerar exactamente o subespaço, sem residual IEEE",
       volta == N && posto == N && posto < D && qz_saturou == 0);
}

printf("\n§C5  E ELA PRESERVA A VIZINHANÇA — o que o telómero não fazia.\n\n");
{
    /* Um hash espalha; uma base ORTOGONAL preserva distâncias — Parseval, e mede-se
     * nos quadrados, sem normalizar:
     *
     *      ‖v−w‖²  =  Σ_j  ⟨v−w, u_j⟩² / ‖u_j‖²
     *
     * Os dois lados em ℚ. Se isto falhasse, a cifra servia para identificar e não para
     * procurar. */
    long U[N][D];
    int posto = 0;
    gs_int(V, U, N, &posto);
    long vivos = 0, fecha = 0;
    qz_saturou = 0;
    for(int i = 0; i < N; i++) for(int j = i + 1; j < N; j++){
        long dif[D];
        for(int k = 0; k < D; k++) dif[k] = V[i][k] - V[j][k];
        long dv2 = rt_norma(dif, D);
        if(dv2 == 0) continue;
        vivos++;
        Qz esq = qz_de_inteiro(dv2);
        Qz dir = qz(0, 1);
        for(int t = 0; t < posto; t++){
            long dc = rt_dir(dif, U[t], D);
            long nj = rt_norma(U[t], D);
            dir = qz_soma(dir, qz(dc * dc, nj));
        }
        if(qz_igual(esq, dir)) fecha++;
    }
    printf("      Parseval dv² = Σ Δc_j²/‖u_j‖² em %ld de %ld pares (qz_saturou %ld)\n\n",
           fecha, vivos, qz_saturou);
    ok("a cifra preserva as distâncias — Parseval: dv² = Σ Δc²/n_j nos pares não nulos,"
       " exacto em ℚ, sem raiz e sem escala 1e-15",
       fecha == vivos && vivos > 0 && qz_saturou == 0);
    printf("      É aqui que esta cifra e o telómero se separam: o telómero ESPALHA (por\n");
    printf("      isso identifica sem colidir) e esta PRESERVA (por isso procura). São o\n");
    printf("      par dual, e a busca precisa das duas, como o dna.c §N6 já dizia.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A base sai dos próprios vetores e é ortogonal por GS inteiro; a volta é\n");
printf("    exacta em ℚ; e as distâncias sobrevivem. Mas 'só normalizar' não bastava —\n");
printf("    os vetores partilham eixo, e isso é o significado.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
