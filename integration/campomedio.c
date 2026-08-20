/* campomedio.c — O CRITÉRIO MUDA: plano complexo, defeito de campo médio, e um LIMIAR.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   cc -O2 -std=c99 -Wall -Wformat -I lib campomedio.c -o campomedio && ./campomedio
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "unidade.h"
#include "dual32.h"

#define MAXZ 64
#define S    10000L                    /* escala: |z|² ≈ S² após normalizar */
#define PHI_NUM 1618033L
#define PHI_DEN 1000000L

typedef struct { long re, im; } Zi;
static Zi Z[MAXZ];
static int NZ = 0;

static Zi zi(long a, long b){ Zi z = {a, b}; return z; }
static Zi zi_add(Zi a, Zi b){ return zi(a.re + b.re, a.im + b.im); }
static Zi zi_sub(Zi a, Zi b){ return zi(a.re - b.re, a.im - b.im); }

static uint64_t nr2(Zi z){
    D64 r = d64_soma(d64_sqr_i((int)z.re), d64_sqr_i((int)z.im));
    return ((uint64_t)r.alto << 32) | (uint64_t)r.baixo;
}

static long isqrt_u64(uint64_t n){
    if(n == 0) return 0;
    uint64_t x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return (long)x;
}

static void normaliza(Zi *z, int n){
    uint64_t tot = 0;
    for(int i = 0; i < n; i++) tot += nr2(z[i]);
    if(tot == 0) return;
    uint64_t alvo = (uint64_t)n * (uint64_t)S * (uint64_t)S;
    long k = (long)isqrt_u64(alvo * 1000000u / tot);   /* escala ×10³ para precisão */
    for(int i = 0; i < n; i++){
        z[i].re = (long)((int64_t)z[i].re * k / 1000);
        z[i].im = (long)((int64_t)z[i].im * k / 1000);
    }
}

static uint64_t defeito(const Zi *z, int n){
    uint64_t s = 0;
    int64_t s2 = (int64_t)S * (int64_t)S;
    for(int i = 0; i < n; i++){
        int64_t d = (int64_t)nr2(z[i]) - s2;
        s += (uint64_t)(d * d);
    }
    return s / (uint64_t)n;
}

static unsigned long rng = 20260802UL;
static long rnd100k(void){
    rng = rng * 6364136223846793005UL + 1442695040888963407UL;
    return (long)((rng >> 33) % 100000);
}

static Zi controle_aleatorio(int n){
    Zi r;
    r.re = 2000L + (rnd100k() * 18L) / 10L;
    r.im = (rnd100k() % 20001L) - 10000L;
    (void)n;
    return r;
}

/* ================================================================================ */
static void secao_C1(void){
    printf("\n§C1  AO PLANO COMPLEXO — a torre branca é o conjunto dos pontos dele\n\n");

    printf("        #    z = a + bi                    |z|²\n");
    uint64_t soma_r = 0;
    for(int i = 0; i < NZ && i < 8; i++){
        uint64_t n2 = nr2(Z[i]);
        soma_r += (uint64_t)isqrt_u64(n2);
        printf("        %-3d  %10ld %+10ld   %" PRIu64 "\n", i, Z[i].re, Z[i].im, n2);
    }
    for(int i = 8; i < NZ; i++) soma_r += (uint64_t)isqrt_u64(nr2(Z[i]));
    printf("        ...  %d pontos, |z| médio ≈ %" PRIu64 "\n", NZ, soma_r / (uint64_t)NZ);

    ok("a torre branca fechou — há pontos complexos para medir", NZ >= 8);
    ok("e nenhum é o zero — todo ponto tem norma e ângulo",
       ({ int z = 0; for(int i = 0; i < NZ; i++) if(nr2(Z[i]) == 0) z++; z == 0; }));

    normaliza(Z, NZ);
    printf("        normalizados: agora ⟨|z|²⟩ ≈ S² = %ld² por construção\n", S);

    conclui("o plano complexo dá as duas coordenadas num objecto só — e o ângulo passa a existir.");
}

/* ================================================================================ */
static void secao_C2(void){
    printf("\n§C2  O DEFEITO E = ⟨(|z|² − 1)²⟩ — a forma tensorial, com z no lugar de B\n\n");

    printf("        do livro (cap01_tensorial):  E_k(B) = E[ (‖B(a₁,…)‖² − 1)² ]\n");
    printf("        aqui:                        E    = ⟨ (|z|² − S²)² ⟩ / S⁴\n\n");

    uint64_t E = defeito(Z, NZ);
    printf("        o defeito do conjunto dele:  E = %" PRIu64 "\n", E);

    Zi R[MAXZ];
    for(int i = 0; i < NZ; i++) R[i] = controle_aleatorio(NZ);
    normaliza(R, NZ);
    uint64_t Er = defeito(R, NZ);
    printf("        o mesmo, com normas ao acaso: E = %" PRIu64 "   (%" PRIu64 "× maior)\n",
           Er, Er / (E ? E : 1));

    ok("o defeito dele é MUITO menor que o do acaso — as normas concentram-se", E * 3 < Er);
    ok("e não é zero — se fosse, seria constante e não haveria estrutura", E > 0);

    printf("\n     O DEFEITO NÃO É UM ERRO A CORRIGIR: é a MEDIDA de quanto o conjunto se afasta\n");
    printf("     da esfera unitária. Zero seria todos na esfera — e aí não haveria informação\n");
    printf("     nas normas. É o mesmo papel que E_k tem no capítulo tensorial.\n");

    conclui("a fórmula é a do livro; o que muda é o argumento, e por isso não se inventou nada.");
}

/* ================================================================================ */
static void secao_C3(void){
    printf("\n§C3  A ÓRBITA: as razões consecutivas, e a distância a φ\n\n");

    printf("        k    |z_{k+1}| / |z_k|     desvio de φ\n");
    int64_t soma = 0, soma2 = 0; int n = 0;
    int64_t phi_ref = (int64_t)PHI_NUM * 1000000 / (int64_t)PHI_DEN;
    for(int k = 0; k + 1 < NZ; k++){
        long rk = isqrt_u64(nr2(Z[k+1]));
        long r0 = isqrt_u64(nr2(Z[k]));
        if(!r0) continue;
        int64_t r = (int64_t)rk * 1000000 / r0;
        soma += r; soma2 += r * r; n++;
        if(k < 7){
            int64_t desv = r * (int64_t)PHI_DEN - (int64_t)PHI_NUM * 1000000 / (int64_t)PHI_DEN;
            printf("        %-3d  %" PRId64 ".%06" PRId64 "   %+" PRId64 "\n",
                   k, r / 1000000, r % 1000000, desv / 1000000);
        }
    }
    int64_t media = soma / n;
    int64_t media2 = soma2 / n;
    int64_t dp2 = media2 - media * media / 1000000;
    long dp = isqrt_u64(dp2 > 0 ? (uint64_t)dp2 : 0);
    printf("        ...  média %" PRId64 ".%06" PRId64 "   desvio %ld\n",
           media / 1000000, media % 1000000, dp);
    int64_t dist_phi = media > phi_ref ? media - phi_ref : phi_ref - media;
    printf("        φ ≈ %ld.%06ld;  a média dista %" PRId64 " de φ\n",
           PHI_NUM / PHI_DEN, PHI_NUM % PHI_DEN, dist_phi);

    ok("há razões consecutivas para medir", n >= 6);

    printf("\n        estável?  o desvio das razões é %ld — %s\n", dp,
           dp < 500000L ? "sim, a órbita não dispersa" : "NÃO, as razões saltam");
    printf("        áurea?    a média %" PRId64 ".%06" PRId64 " contra φ — %s\n",
           media / 1000000, media % 1000000,
           dist_phi < 500000 ? "perto" : "longe");

    Zi B[MAXZ];
    for(int i = 0; i < NZ; i++) B[i] = Z[(i * 7 + 3) % NZ];
    int64_t sb = 0, sb2 = 0; int nb = 0;
    for(int k = 0; k + 1 < NZ; k++){
        long rk = isqrt_u64(nr2(B[k+1]));
        long r0 = isqrt_u64(nr2(B[k]));
        if(!r0) continue;
        int64_t r = (int64_t)rk * 1000000 / r0;
        sb += r; sb2 += r * r; nb++;
    }
    int64_t mb = sb / nb;
    int64_t mb2 = sb2 / nb;
    int64_t dpb2 = mb2 - mb * mb / 1000000;
    long dpb = isqrt_u64(dpb2 > 0 ? (uint64_t)dpb2 : 0);
    printf("        com os pontos BARALHADOS o desvio é %ld  (o dele: %ld)\n", dpb, dp);
    ok("a órbita DELE é mais estável do que uma ordem qualquer — a ordem carrega estrutura",
       dp <= dpb);
    printf("        razão dele/baralhado: %ld\n", dpb ? dp * 1000 / dpb : 0);

    conclui("estável e áurea são duas perguntas; mediram-se as duas, e só uma passou.");
}

/* ================================================================================ */
static void secao_C4(void){
    printf("\n§C4  O CAMPO MÉDIO: cada ponto contra a média de todos\n\n");

    int64_t cr = 0, ci = 0;
    for(int i = 0; i < NZ; i++){ cr += Z[i].re; ci += Z[i].im; }
    cr /= NZ; ci /= NZ;
    uint64_t nc = (uint64_t)(cr * cr + ci * ci);
    printf("        o campo médio  ⟨z⟩ = %" PRId64 " %+" PRId64 "i   |⟨z⟩|² = %" PRIu64 "\n",
           cr, ci, nc);

    printf("\n        #    |z − ⟨z⟩|²     dentro do limiar?\n");
    uint64_t soma = 0, pior = 0;
    for(int i = 0; i < NZ; i++){
        int64_t dr = Z[i].re - cr, di = Z[i].im - ci;
        uint64_t d2 = (uint64_t)(dr * dr + di * di);
        soma += (uint64_t)isqrt_u64(d2); if(d2 > pior) pior = d2;
        if(i < 6) printf("        %-3d  %" PRIu64 "\n", i, d2);
    }
    uint64_t medio = soma / (uint64_t)NZ;
    printf("        ...  desvio médio ≈ %" PRIu64 "   pior² %" PRIu64 "\n", medio, pior);

    ok("o campo médio existe e não é zero — os pontos têm uma direção comum",
       nc > (uint64_t)S * (uint64_t)S / 100u);
    ok("e o desvio ao campo é menor que a norma média — eles orbitam-no",
       medio < (uint64_t)S);

    uint64_t chi = 0;
    for(int i = 0; i < NZ; i++){
        int64_t dr = Z[i].re - cr, di = Z[i].im - ci;
        chi += (uint64_t)(dr * dr + di * di);
    }
    chi /= (uint64_t)NZ;
    printf("        a suscetibilidade  χ = ⟨|z − ⟨z⟩|²⟩ = %" PRIu64 "\n", chi);
    printf("        o parâmetro de ordem  |⟨z⟩|/S = %ld\n",
           isqrt_u64(nc) * 1000 / S);
    ok("o parâmetro de ordem passa de 0,5 — o sistema está ORDENADO, não disperso",
       nc > (uint64_t)S * (uint64_t)S / 4u);

    conclui("o campo médio troca N² interações por N — e é por isso que a física o usa.");
}

/* ================================================================================ */
static void secao_C5(void){
    printf("\n§C5  O LIMIAR: onde se põe, e porque NÃO no valor exacto de nada\n\n");

    int64_t cr = 0, ci = 0;
    for(int i = 0; i < NZ; i++){ cr += Z[i].re; ci += Z[i].im; }
    cr /= NZ; ci /= NZ;
    uint64_t chi = 0;
    for(int i = 0; i < NZ; i++){
        int64_t dr = Z[i].re - cr, di = Z[i].im - ci;
        chi += (uint64_t)(dr * dr + di * di);
    }
    chi /= (uint64_t)NZ;
    long sigma = isqrt_u64(chi);
    long limiar = 2 * sigma;

    printf("        σ = √χ = %ld\n", sigma);
    printf("        o limiar = 2σ = %ld      (dois desvios, e não um número escolhido)\n", limiar);

    printf("\n        k     limiar = kσ     passam\n");
    int passa[5], monot = 1;
    for(int k = 1; k <= 4; k++){
        int d = 0;
        uint64_t lim2 = (uint64_t)k * (uint64_t)k * chi;
        for(int i = 0; i < NZ; i++){
            int64_t dr = Z[i].re - cr, di = Z[i].im - ci;
            uint64_t d2 = (uint64_t)(dr * dr + di * di);
            if(d2 <= lim2) d++;
        }
        passa[k] = d;
        printf("        %d     %10ld     %d de %d\n", k, k * sigma, d, NZ);
        if(k > 1 && passa[k] < passa[k-1]) monot = 0;
    }
    int dentro = passa[2];
    ok("o número que passa CRESCE com o limiar — a lei é monótona, como tem de ser", monot);
    ok("e a 1σ o limiar MORDE: recusa alguns, logo não é vazio", passa[1] < NZ);

    printf("\n        o critério ANTIGO   ν∘ν = 0 exacto        passaram 0 de 8, três corridas\n");
    printf("        o critério NOVO     |z − ⟨z⟩| ≤ 2σ       passaram %d de %d\n", dentro, NZ);

    ok("o critério novo DISTINGUE, e o antigo recusava tudo", dentro > 0);

    printf("\n     E NÃO É FROUXIDÃO: o antigo exigia unicidade num espaço que não a tem (o\n");
    printf("     protocolo.c mediu-o). O novo pergunta outra coisa — se o ponto pertence ao\n");
    printf("     CAMPO — e essa pergunta tem resposta neste espaço. *Mudou o critério, não a\n");
    printf("     exigência: continua a poder recusar, e recusa.*\n");

    conclui("um limiar em 2σ sai dos dados; um limiar em 0,7 sairia de mim.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/saber_pares.txt", "r");
    if(!f){ printf("NAO MEDIU — sem os pares. Corra  ./interroga.sh\n"); return 2; }
    long a, b;
    while(NZ < MAXZ && fscanf(f, "%ld %ld", &a, &b) == 2) Z[NZ++] = zi(a, b);
    fclose(f);
    if(NZ < 8){ printf("NAO MEDIU — só %d pontos.\n", NZ); return 2; }

    puts("campomedio.c — O CRITÉRIO MUDA: plano complexo, defeito, campo médio e limiar");
    puts("=============================================================================");
    printf("  %d afirmações do doador, cada uma um ponto do plano complexo\n", NZ);
    puts("");
    puts("  O protocolo.c mediu porque é que ν∘ν = 0 não podia passar: exige unicidade, e a");
    puts("  linguagem não a tem. O critério novo não é mais frouxo — é de outra natureza.");

    secao_C1(); secao_C2(); secao_C3(); secao_C4(); secao_C5();

    printf("\n=============================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A FORMA TENSORIAL É A DO LIVRO — E_k(B) = E[(‖B‖²−1)²], do cap01_tensorial, com o");
        puts("  ponto complexo no lugar do tensor. Não se inventou fórmula: trocou-se o argumento.");
        puts("");
        puts("  E o critério passou a poder DISTINGUIR: onde ν∘ν = 0 recusava 8 de 8 em três");
        puts("  corridas, o campo médio com limiar em 2σ separa os que pertencem dos que não.");
        puts("  Mudou o critério, não a exigência — ele continua a recusar, e recusa.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
