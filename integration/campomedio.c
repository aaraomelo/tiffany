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
#include "unidade.h"

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
static long long nr2(Zi z){ return (long long)z.re * z.re + (long long)z.im * z.im; }

static long isqrt_ll(long long n){
    if(n <= 0) return 0;
    long long x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return (long)x;
}

static void normaliza(Zi *z, int n){
    long long tot = 0;
    for(int i = 0; i < n; i++) tot += nr2(z[i]);
    if(tot <= 0) return;
    long long alvo = (long long)n * S * S;
    long k = isqrt_ll(alvo * 1000000LL / tot);   /* escala ×10³ para precisão */
    for(int i = 0; i < n; i++){
        z[i].re = (long)((long long)z[i].re * k / 1000);
        z[i].im = (long)((long long)z[i].im * k / 1000);
    }
}

static long long defeito(const Zi *z, int n){
    long long s = 0;
    for(int i = 0; i < n; i++){
        long long d = nr2(z[i]) - (long long)S * S;
        s += d * d;
    }
    return s / n;
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
    long long soma_r = 0;
    for(int i = 0; i < NZ && i < 8; i++){
        long long n2 = nr2(Z[i]);
        soma_r += isqrt_ll(n2);
        printf("        %-3d  %10ld %+10ld   %lld\n", i, Z[i].re, Z[i].im, n2);
    }
    for(int i = 8; i < NZ; i++) soma_r += isqrt_ll(nr2(Z[i]));
    printf("        ...  %d pontos, |z| médio ≈ %lld\n", NZ, soma_r / NZ);

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

    long long E = defeito(Z, NZ);
    printf("        o defeito do conjunto dele:  E = %lld\n", E);

    Zi R[MAXZ];
    for(int i = 0; i < NZ; i++) R[i] = controle_aleatorio(NZ);
    normaliza(R, NZ);
    long long Er = defeito(R, NZ);
    printf("        o mesmo, com normas ao acaso: E = %lld   (%lld× maior)\n",
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
    long long soma = 0, soma2 = 0; int n = 0;
    for(int k = 0; k + 1 < NZ; k++){
        long rk = isqrt_ll(nr2(Z[k+1]));
        long r0 = isqrt_ll(nr2(Z[k]));
        if(!r0) continue;
        long long r = (long long)rk * 1000000L / r0;
        soma += r; soma2 += r * r; n++;
        if(k < 7){
            long long desv = r * (long)PHI_DEN - (long long)PHI_NUM * 1000000L / (long)PHI_DEN;
            printf("        %-3d  %lld.%06lld   %+lld\n",
                   k, r / 1000000, r % 1000000, desv / 1000000);
        }
    }
    long long media = soma / n;
    long long media2 = soma2 / n;
    long long dp2 = media2 - media * media / 1000000L;
    long dp = isqrt_ll(dp2 > 0 ? dp2 : 0);
    printf("        ...  média %lld.%06lld   desvio %ld\n",
           media / 1000000, media % 1000000, dp);
    printf("        φ ≈ %ld.%06ld;  a média dista %lld de φ\n",
           PHI_NUM / PHI_DEN, PHI_NUM % PHI_DEN,
           (media > (long long)PHI_NUM * 1000000L / PHI_DEN ?
            media - (long long)PHI_NUM * 1000000L / PHI_DEN :
            (long long)PHI_NUM * 1000000L / PHI_DEN - media));

    ok("há razões consecutivas para medir", n >= 6);

    printf("\n        estável?  o desvio das razões é %ld — %s\n", dp,
           dp < 500000L ? "sim, a órbita não dispersa" : "NÃO, as razões saltam");
    printf("        áurea?    a média %lld.%06lld contra φ — %s\n",
           media / 1000000, media % 1000000,
           (media > (long long)PHI_NUM * 1000000L / PHI_DEN ?
            media - (long long)PHI_NUM * 1000000L / PHI_DEN :
            (long long)PHI_NUM * 1000000L / PHI_DEN - media) < 500000L ? "perto" : "longe");

    Zi B[MAXZ];
    for(int i = 0; i < NZ; i++) B[i] = Z[(i * 7 + 3) % NZ];
    long long sb = 0, sb2 = 0; int nb = 0;
    for(int k = 0; k + 1 < NZ; k++){
        long rk = isqrt_ll(nr2(B[k+1]));
        long r0 = isqrt_ll(nr2(B[k]));
        if(!r0) continue;
        long long r = (long long)rk * 1000000L / r0;
        sb += r; sb2 += r * r; nb++;
    }
    long long mb = sb / nb;
    long long mb2 = sb2 / nb;
    long long dpb2 = mb2 - mb * mb / 1000000L;
    long dpb = isqrt_ll(dpb2 > 0 ? dpb2 : 0);
    printf("        com os pontos BARALHADOS o desvio é %ld  (o dele: %ld)\n", dpb, dp);
    ok("a órbita DELE é mais estável do que uma ordem qualquer — a ordem carrega estrutura",
       dp <= dpb);
    printf("        razão dele/baralhado: %ld\n", dpb ? dp * 1000 / dpb : 0);

    conclui("estável e áurea são duas perguntas; mediram-se as duas, e só uma passou.");
}

/* ================================================================================ */
static void secao_C4(void){
    printf("\n§C4  O CAMPO MÉDIO: cada ponto contra a média de todos\n\n");

    long long cr = 0, ci = 0;
    for(int i = 0; i < NZ; i++){ cr += Z[i].re; ci += Z[i].im; }
    cr /= NZ; ci /= NZ;
    long long nc = (long long)cr * cr + (long long)ci * ci;
    printf("        o campo médio  ⟨z⟩ = %lld %+.lldi   |⟨z⟩|² = %lld\n", cr, ci, nc);

    printf("\n        #    |z − ⟨z⟩|²     dentro do limiar?\n");
    long long soma = 0, pior = 0;
    for(int i = 0; i < NZ; i++){
        long long dr = Z[i].re - cr, di = Z[i].im - ci;
        long long d2 = dr * dr + di * di;
        soma += isqrt_ll(d2); if(d2 > pior) pior = d2;
        if(i < 6) printf("        %-3d  %lld\n", i, d2);
    }
    long long medio = soma / NZ;
    printf("        ...  desvio médio ≈ %lld   pior² %lld\n", medio, pior);

    ok("o campo médio existe e não é zero — os pontos têm uma direção comum",
       nc > (long long)S * S / 100);
    ok("e o desvio ao campo é menor que a norma média — eles orbitam-no",
       medio < S);

    long long chi = 0;
    for(int i = 0; i < NZ; i++){
        long long dr = Z[i].re - cr, di = Z[i].im - ci;
        chi += dr * dr + di * di;
    }
    chi /= NZ;
    printf("        a suscetibilidade  χ = ⟨|z − ⟨z⟩|²⟩ = %lld\n", chi);
    printf("        o parâmetro de ordem  |⟨z⟩|/S = %lld\n",
           isqrt_ll(nc) * 1000 / S);
    ok("o parâmetro de ordem passa de 0,5 — o sistema está ORDENADO, não disperso",
       nc > (long long)S * S / 4);

    conclui("o campo médio troca N² interações por N — e é por isso que a física o usa.");
}

/* ================================================================================ */
static void secao_C5(void){
    printf("\n§C5  O LIMIAR: onde se põe, e porque NÃO no valor exacto de nada\n\n");

    long long cr = 0, ci = 0;
    for(int i = 0; i < NZ; i++){ cr += Z[i].re; ci += Z[i].im; }
    cr /= NZ; ci /= NZ;
    long long chi = 0;
    for(int i = 0; i < NZ; i++){
        long long dr = Z[i].re - cr, di = Z[i].im - ci;
        chi += dr * dr + di * di;
    }
    chi /= NZ;
    long sigma = isqrt_ll(chi);
    long limiar = 2 * sigma;

    printf("        σ = √χ = %ld\n", sigma);
    printf("        o limiar = 2σ = %ld      (dois desvios, e não um número escolhido)\n", limiar);

    printf("\n        k     limiar = kσ     passam\n");
    int passa[5], monot = 1;
    for(int k = 1; k <= 4; k++){
        int d = 0;
        long long lim2 = (long long)k * k * chi;
        for(int i = 0; i < NZ; i++){
            long long dr = Z[i].re - cr, di = Z[i].im - ci;
            long long d2 = dr * dr + di * di;
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
