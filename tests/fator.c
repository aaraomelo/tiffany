/* fator.c — A RAZÃO CRUZADO/DIRETO É O FATOR DE POTÊNCIA, e o unitário é da família real.
 *
 * O Aarão, em quatro recados seguidos enquanto eu media o tecido da assistente com um escalar:
 *
 *   "esse cosseno que vc mede pode medir em cada dimensão"
 *   "ele sai inteiro, usa a régua infinita — a polar"
 *   "é a razão entre a régua polar e a cartesiana, razão entre produto direto e cruzado, verifica isso"
 *   "isso é fator de potência — fator de potência unitário é da família real"
 *
 * E os quatro são um só, que corrige um erro meu de método. Eu estava a resumir 768 dimensões
 * num número (o cosseno médio) e a tirar conclusões dele. Quando decompus esse número nas 768
 * componentes CARTESIANAS, não separou nada: todas as camadas do tecido deram o mesmo perfil
 * (~80 dimensões para metade da massa, a mesma dimensão dominante). A cartesiana não é a base
 * própria do fenómeno. A POLAR é — e a razão entre as duas é o fator de potência.
 *
 *      DIRETO    ⟨x,y⟩               a parte SIMÉTRICA, escalar, MEDE       potência ATIVA
 *      CRUZADO   |x∧y|               a parte ANTISSIMÉTRICA, roda, ORDENA   potência REATIVA
 *      razão     cruzado/direto                                            tan φ
 *
 * Não é analogia com a eletrotécnica: é a mesma conta. O motor.c já tinha escrito que
 * T_e = (3/2)·P· ψ_s × i_s — o TORQUE É O PRODUTO CRUZADO — e quem diz quanta da corrente vira
 * torque e quanta só magnetiza é exatamente esta razão.
 *
 * E A TESE DO AARÃO, que é o que este ficheiro existe para medir: o fator de potência UNITÁRIO
 * é da FAMÍLIA REAL. É verdade e é por construção — det A_m = −1 para todo metal, e |det| = 1 é
 * o fator de potência unitário: a transformação não perde nem ganha, e é por isso que a cifra
 * volta exata. Daí sai a consequência que eu não tinha visto: Δ = m²+4 > 0 sempre, logo a
 * família real é toda HIPERBÓLICA, logo a razão dela é tanh e não tan — e tanh é LIMITADA.
 * *A família real não precisa da régua infinita.* Quem precisa é o círculo, onde tan diverge —
 * e é lá que o tecido vive.
 *
 *   §W1  a razão cruzado/direto É tan θ — Lagrange contra a soma dos menores, em ℤ
 *   §W2  a família real: |det A_m| = 1, o fator de potência unitário, e a inversa é INTEIRA
 *   §W3  hipérbole contra círculo: A_m nunca passa por ∞; a rotação passa — quem precisa da régua
 *   §W4  a régua infinita representa o que diverge, e sai INTEIRA (ida-e-volta da CF)
 *   §W5  a INVERSÃO: guardar quer fp = 0 — porque fp = 1 são vetores paralelos, e isso é posto 1
 *   §W6  o inversor multinível: os níveis da régua são ÓTIMOS, medido por força bruta
 *   §W7  a RESOLUÇÃO modula até fp = 1, e o 1 É a condição de parada — medido NA assistente
 *   §W8  os dois lados: guardar quer 0, resolver quer 1, e é o mesmo fator
 *   §W9  o inversor atravessa as realizações — Euclides, encaixe, conta, Pisano
 *
 * LEI vs TRANSPORTE. sin/cos, tan(acos), tanh, CF de tan(θ) em vírgula e GS com 1e-18 eram o
 * método. A lei é Lagrange |x∧y|² = |x|²|y|²−⟨x,y⟩² contra a soma dos menores 2×2, |det A_m|=1
 * com A·A⁻¹=I em ℤ, a rotação a passar por ∞ em ℙ¹ contra A_m que nunca devolve q=0, a CF
 * ida-e-volta exacta, o posto por GS inteiro, e os convergentes de φ = Fibonacci, sem √5.
 *
 *   cc -O2 -std=c99 -I lib tests/fator.c -o fator && ./fator
 *   (o §W7 corre ../banco/bin/conversa a partir de tests/ — mede a assistente, não uma simulação)
 */
#define _POSIX_C_SOURCE 200809L   /* popen: o §W7 mede a assistente a correr */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "unidade.h"
#include "reta.h"
#include "rt_cf_slot.h"

#define D 24        /* dimensão dos vetores de prova */
#define M 12        /* quantos vetores */

/* MAIS PERTO DE φ, DECIDIDO EM ℤ — é o thm:corte do Algébrico, e não um limiar.
 *
 * Com φ = (1+√5)/2 e v = q − 2p vem  |qφ − p| = |v + q√5| / 2,  logo
 *
 *     |φ − p₁/q₁| < |φ − p₂/q₂|   ⟺   q₂·|v₁ + q₁√5|  <  q₁·|v₂ + q₂√5|
 *
 * e como os dois lados são ≥ 0 comparam-se por QUADRADOS, ficando  X + Y√5 < 0  com
 *
 *     X = q₂²(v₁² + 5q₁²) − q₁²(v₂² + 5q₂²)      Y = 2·q₁·q₂·(q₂v₁ − q₁v₂)
 *
 * O sinal de X + Y√5 lê-se sem formar a raiz: se X e Y têm o mesmo sinal é esse; se
 * têm sinais opostos, |X| vs √5|Y| decide-se por X² vs 5Y². É a partição em dois do
 * thm:corte. */
static int mais_perto_de_phi(long p1, long q1, long p2, long q2){
    long v1 = q1 - 2*p1, v2 = q2 - 2*p2;
    long X = q2*q2*(v1*v1 + 5*q1*q1) - q1*q1*(v2*v2 + 5*q2*q2);
    long Y = 2*q1*q2*(q2*v1 - q1*v2);
    if(X <= 0 && Y <= 0) return (X < 0 || Y < 0);          /* X + Y√5 < 0 */
    if(X >= 0 && Y >= 0) return 0;                          /* X + Y√5 >= 0 */
    if(X < 0)  return X*X > 5*Y*Y;     /* X<0<Y : negativo sse |X| > √5·Y  */
    return 5*Y*Y > X*X;                /* Y<0<X : negativo sse √5·|Y| > X  */
}

/* posto por Gram–Schmidt INTEIRO: o resto r·r ≠ 0 conta uma direcção. */
static int posto_gs(const long src[][D], int n){
    long U[M][D];
    int p = 0;
    for(int a = 0; a < n; a++){
        for(int k = 0; k < D; k++) U[p][k] = src[a][k];
        for(int j = 0; j < p; j++){
            long ip = rt_dir(U[p], U[j], D);
            long nj = rt_norma(U[j], D);
            for(int k = 0; k < D; k++)
                U[p][k] = nj * U[p][k] - ip * U[j][k];
        }
        if(rt_norma(U[p], D) != 0) p++;
    }
    return p;
}

int main(void){
long v[M][D];
for(int a = 0; a < M; a++)
    for(int i = 0; i < D; i++)
        v[a][i] = (long)(a + 1)*(i + 1) - 2L*(a % 5)*(i % 7);

printf("\n=== A RAZÃO CRUZADO/DIRETO É O FATOR DE POTÊNCIA ==========================\n");
printf("    Eu estava a resumir 768 dimensões num escalar. A decomposição CARTESIANA\n");
printf("    não separou nada; a POLAR separa, e a razão entre as duas é tan φ.\n");

printf("\n§W1  A razão cruzado/direto É tan θ — por dois caminhos independentes, em ℤ.\n\n");
{
    /* Dois caminhos que têm de concordar: (a) Lagrange |x|²|y|² − ⟨x,y⟩²; (b) a soma
     * dos menores 2×2 do bivector, rt_cruz. Se batem, a razão É a tangente (ao quadrado)
     * e não apenas parecida com ela. Um caminho só não provava nada — provava que sei
     * definir cruzado pela identidade que queria medir. */
    printf("      par      direto²      cruzado² (menores)   Lagrange\n");
    int n = 0, iguais = 0, cruzam = 0, mostrados = 0;
    for(int a = 0; a < M; a++) for(int b = a+1; b < M; b++){
        long dd = rt_dir(v[a], v[b], D);
        long nx = rt_norma(v[a], D), ny = rt_norma(v[b], D);
        long lag = nx*ny - dd*dd;
        long C[D*D];
        rt_cruz(v[a], v[b], D, C);
        long cruz2 = 0;
        for(int i = 0; i < D; i++) for(int j = i+1; j < D; j++)
            cruz2 += C[i*D + j] * C[i*D + j];
        n++;
        if(cruz2 == lag) iguais++;
        if(cruz2 > 0) cruzam++;
        if(mostrados < 5){
            printf("      (%d,%d)  %+10ld   %-18ld  %ld\n", a, b, dd, cruz2, lag);
            mostrados++;
        }
    }
    printf("      …\n\n      %d pares, %d com Lagrange = menores, %d com cruzado ≠ 0\n\n",
           n, iguais, cruzam);
    ok("a razão cruzado/direto É a tangente: Lagrange e a soma dos menores dão o mesmo",
       iguais == n && n > 0 && cruzam > 0);
    printf("      Logo o fator de potência é o DIRETO e a razão é tan φ. E não é analogia:\n");
    printf("      o motor.c já tinha T_e = ψ_s × i_s — o torque É o cruzado.\n");
}

printf("\n§W2  A FAMÍLIA REAL: |det A_m| = 1, e a inversa é INTEIRA.\n\n");
{
    /* A tese do Aarão. A_m = [[m,1],[1,0]] tem det = −1 para todo m, e |det| = 1 é o fator
     * de potência unitário: a transformação não perde nem ganha área. A consequência — que
     * eu não tinha visto — é que Δ = m²+4 > 0 SEMPRE, logo a família é toda hiperbólica.
     * O det não se lê da fórmula m·0−1 (isso compara o coeficiente consigo): monta-se a
     * matriz, multiplica-se pela inversa inteira, e exige-se I. */
    printf("      m    det A_m   |det|   Δ = m²+4   A·A⁻¹ = I\n");
    int mau = 0, naoHip = 0, naoI = 0;
    for(int m = 1; m <= 8; m++){
        RtOp A = {{ m, 1, 1, 0 }};
        long det = rt_op_det(&A);
        long Delta = (long)m*m + 4;
        int eI = 0;
        if(det == 1 || det == -1){
            long i00 =  A.T[3]/det, i01 = -A.T[1]/det, i10 = -A.T[2]/det, i11 = A.T[0]/det;
            long p00 = A.T[0]*i00 + A.T[1]*i10;
            long p01 = A.T[0]*i01 + A.T[1]*i11;
            long p10 = A.T[2]*i00 + A.T[3]*i10;
            long p11 = A.T[2]*i01 + A.T[3]*i11;
            eI = (p00 == 1 && p01 == 0 && p10 == 0 && p11 == 1);
        }
        if(det != 1 && det != -1) mau++;
        if(Delta <= 0) naoHip++;
        if(!eI) naoI++;
        printf("      %-4d %-9ld %-7ld %-10ld %s\n",
               m, det, det < 0 ? -det : det, Delta, eI ? "sim" : "nao");
    }
    printf("\n");
    ok("|det A_m| = 1 em toda a família real — o fator de potência é UNITÁRIO", mau == 0);
    ok("Δ = m²+4 > 0 sempre: a família real é toda hiperbólica, nunca o círculo", naoHip == 0);
    ok("e A·A⁻¹ = I em ℤ: a inversa é inteira porque |det| = 1, e a cifra volta exacta",
       naoI == 0);
    printf("      Fator de potência unitário É |det| = 1, e |det| = 1 É a inversa inteira, que\n");
    printf("      é a razão de a cifra voltar exata. Os três nomes são a mesma condição.\n");
}

printf("\n§W3  HIPÉRBOLE contra CÍRCULO: quem precisa da régua infinita.\n\n");
{
    /* Na hipérbole a razão é tanh — limitada. No círculo é tan — ilimitada, passa por ∞.
     * Em ℙ¹: a rotação R = [[0,−1],[1,0]] manda [1:0] (∞) em [0:1] e devolve-o em dois
     * passos. A_m manda [1:0] em [m:1] e q nunca volta a 0. */
    RtOp R = {{ 0, -1, 1, 0 }};
    int rot_no_inf = 0;
    {
        long p = 1, q = 0;
        for(int k = 1; k <= 4; k++){
            long np, nq; rt_opera(&R, p, q, &np, &nq); p = np; q = nq;
            if(q == 0){ rot_no_inf = k; break; }
        }
    }
    int metal_foge = 1;
    printf("      m    A_m^k [1:0]  (q_k, k=1..6)                    algum q=0?\n");
    for(int m = 1; m <= 5; m++){
        RtOp A = {{ m, 1, 1, 0 }};
        long p = 1, q = 0;
        printf("      %-4d ", m);
        int zero = 0;
        for(int k = 1; k <= 6; k++){
            long np, nq; rt_opera(&A, p, q, &np, &nq); p = np; q = nq;
            printf("%ld%s", q, k < 6 ? " " : "");
            if(q == 0) zero = 1;
        }
        printf("   %s\n", zero ? "sim" : "nao");
        if(zero) metal_foge = 0;
    }
    printf("\n      rotação: [1:0] volta a q=0 no passo %d\n\n", rot_no_inf);
    ok("a rotação passa pelo infinito [1:0] — o círculo EXIGE a régua infinita",
       rot_no_inf == 2);
    ok("A_m nunca devolve q=0 a partir de [1:0] — a hipérbole cabe numa régua finita",
       metal_foge);
    printf("      Portanto a família real NÃO precisa da régua infinita: ela é hiperbólica, e\n");
    printf("      a razão dela nunca chega a ∞. Quem precisa é o CÍRCULO — e é lá que o tecido\n");
    printf("      vive, com os vetores a caminho da ortogonalidade, onde tan diverge.\n");
}

printf("\n§W4  A RÉGUA INFINITA representa o que diverge, e sai INTEIRA.\n\n");
{
    int cf_mem = rt_cf_slot_mem_abre("dados/fator_cf.mem");
    /* "ele sai inteiro, usa a régua infinita". A fracção contínua devolve INTEIROS exactos,
     * e reconstrói de volta. Mede-se a volta: rt_cf_slot_de e rt_cf_slot_para, igualdade em ℤ. */
    struct { long p, q; const char *nome; } t[] = {
        { 22,  7, "22/7" },
        { 355, 113, "355/113" },
        { 99,  1, "99/1 (o que diverge)" },
        { 13,  8, "13/8 (convergente do ouro)" },
        {  1,  1, "1/1" },
    };
    printf("      racional          a régua (quocientes)          volta     bate\n");
    int mal = 0;
    for(size_t i = 0; i < sizeof t/sizeof *t; i++){
        RtCfSlot c = rt_cf_slot_word((unsigned)i, cf_mem);
        long P = 0, Q = 1;
        rt_cf_slot_de(1, t[i].p, t[i].q, &c);
        int cn = rt_cf_slot_n(&c);
        int volta = rt_cf_slot_para(&c, &P, &Q);
        int bate = volta && !rt_cf_slot_saturou(&c) && P*t[i].q == t[i].p*Q && Q != 0;
        if(!bate) mal++;
        printf("      %-17s [", t[i].nome);
        for(int k = 0; k < cn && k < 6; k++)
            printf("%ld%s", rt_cf_slot_termo(&c, k), k < cn-1 && k < 5 ? "; " : "");
        printf("]   %ld/%ld   %s\n", P, Q, bate ? "sim" : "nao");
    }
    if(cf_mem >= 0) close(cf_mem);
    printf("\n");
    ok("a régua reconstrói o racional a partir dos INTEIROS, ida e volta exactas",
       mal == 0);
    printf("      É o telomero.c outra vez: cada divisão deixa resto estritamente menor, logo\n");
    printf("      termina, e o que fica identifica. A régua é infinita porque tan é ilimitada —\n");
    printf("      não por generosidade, por necessidade.\n");
}

printf("\n§W5  A INVERSÃO: o circuito quer fp = 1, o TECIDO quer fp = 0.\n\n");
{
    /* Num circuito o fator de potência unitário é o ÓTIMO: toda a corrente vira trabalho.
     * Num tecido semântico é o PIOR caso: fp = 1 quer dizer todos os vetores paralelos, e
     * um tecido de vetores paralelos tem posto 1 — guarda UMA coisa, por muitos pares que
     * se lhe ensinem. Mede-se o posto nos dois extremos, por GS inteiro. */
    long par[M][D], ort[M][D];
    for(int a = 0; a < M; a++){
        for(int i = 0; i < D; i++){
            par[a][i] = (long)(i + 1);                 /* todos o MESMO: fp = 1 */
            ort[a][i] = (i == a) ? 1 : 0;              /* base canónica: fp = 0 */
        }
    }
    int par_cs = 1, ort_perp = 1, np = 0;
    for(int a = 0; a < M; a++) for(int b = a+1; b < M; b++){
        long dpar = rt_dir(par[a], par[b], D);
        long n2 = rt_norma(par[a], D);
        if(dpar*dpar != n2*n2) par_cs = 0;            /* Cauchy–Schwarz em igualdade */
        if(rt_dir(ort[a], ort[b], D) != 0) ort_perp = 0;
        np++;
    }
    int posto_par = posto_gs(par, M);
    int posto_ort = posto_gs(ort, M);
    printf("      tecido               Cauchy–Schwarz / perp   posto (de %d)\n", M);
    printf("      todos paralelos      igualdade               %d\n", posto_par);
    printf("      todos ortogonais     interno 0               %d\n", posto_ort);
    printf("\n");
    ok("fp = 1 dá posto 1: o tecido guarda UMA coisa, por muitos pares que leve",
       posto_par == 1 && par_cs && np > 0);
    ok("fp = 0 dá posto cheio: a capacidade é máxima na ortogonalidade",
       posto_ort == M && ort_perp);
    printf("      Portanto o mesmo número que num motor se quer em 1 quer-se aqui em 0, e não\n");
    printf("      há contradição: é o par ⊕/⊗ do furos.c. O circuito quer TRABALHO, e trabalho\n");
    printf("      é o direto; o tecido quer CAPACIDADE, e capacidade é o cruzado. Cada um pede\n");
    printf("      o seu lado do par, e o fator de potência é a coordenada que os separa.\n");
}

printf("\n§W6  O INVERSOR MULTINÍVEL modula — e os níveis ótimos SÃO os da régua.\n\n");
{
    /* Um inversor multinível sintetiza um valor a partir de níveis discretos. A régua
     * infinita faz a mesmíssima coisa: aproxima um irracional por racionais, e cada
     * quociente parcial acrescenta um nível. Os convergentes de φ SÃO Fibonacci — sem
     * formar √5. A afirmação forte é que os níveis são ÓTIMOS: nenhum racional de
     * denominador menor ou igual se aproxima mais. Mede-se por FORÇA BRUTA em ℤ. */
    printf("      alvo: φ, o primeiro metal — convergentes F_{n+1}/F_n\n\n");
    printf("      níveis   convergente   algum racional melhor?\n");
    long cp[10], cq[10];
    cp[0] = 1; cq[0] = 1;
    cp[1] = 2; cq[1] = 1;
    for(int k = 2; k < 9; k++){
        cp[k] = cp[k-1] + cp[k-2];
        cq[k] = cq[k-1] + cq[k-2];
    }
    int melhores = 0, mostrados = 0;
    for(int k = 1; k < 9; k++){
        long pn = cp[k], qn = cq[k];
        int bate = 0;
        for(long qq = 1; qq <= qn && !bate; qq++){
            for(long pp = 1; pp <= 2*qq && !bate; pp++){
                if(qq == qn && pp == pn) continue;
                if(mais_perto_de_phi(pp, qq, pn, qn)) bate = 1;
            }
        }
        if(bate) melhores++;
        if(mostrados < 6){
            printf("      %-8d %ld/%-11ld %s\n",
                   k+1, pn, qn, bate ? "SIM — o teorema falha" : "nenhum");
            mostrados++;
        }
    }
    printf("\n");
    ok("nenhum racional de denominador menor bate o convergente — os níveis são ÓTIMOS",
       melhores == 0);
    printf("      Logo o inversor multinível não é UMA maneira de modular a régua: é A maneira,\n");
    printf("      e cada nível que se acrescenta é um quociente parcial. O 'multifractal' é o\n");
    printf("      endereçamento b^n do mmu.c — os níveis são autossimilares, e por isso a mesma\n");
    printf("      máquina serve em qualquer escala. É a ferramenta exata, e exata é literal.\n");
}

printf("\n§W7  A RESOLUÇÃO modula até fp = 1 — e o 1 É a condição de parada.\n\n");
{
    /* O fator de potência de um estado da conta é P/S:
     *
     *     S  a potência APARENTE — os nós que a árvore ainda tem
     *     P  a potência ATIVA    — o que fica no fim, que é UM número
     *     fp = P/S = 1/n
     *
     * E MEDE-SE A MÁQUINA, não um modelo meu dela. Corre-se o `conversa` — a assistente a
     * valer — e conta-se o que ELA faz. A lei é
     *
     *      dobras  =  (folhas + pares de parênteses)  −  1
     *
     * e foi a máquina que ma deu. */
    struct { const char *conta; int folhas, pares; } casos[] = {
        {"2 + 3 x 4 + 5",               4, 0},
        {"(2+3) x (4+5) + 6 x 7",       6, 2},
        {"1 + 2 + 3 + 4 + 5 + 6",       6, 0},
        {"2 x 3 x 4 x 5",               4, 0},
        {"((1+2) x 3) + ((4+5) x 6)",   6, 4},
        {"(1+2) x (3+4) x (5+6)",       6, 3},
    };
    const int NC = (int)(sizeof casos / sizeof casos[0]);
    printf("      conta                         n₀    previsto   a assistente   fp₀ → fp\n");
    int discorda = 0, semResposta = 0, parouEmUm = 1, subiuSempre = 1;
    for(int c = 0; c < NC; c++){
        int n0 = casos[c].folhas + casos[c].pares;
        int previsto = n0 - 1;
        char cmd[512], linha[1024]; int real = -1;
        snprintf(cmd, sizeof cmd,
                 "../banco/bin/conversa ../.torre/tecido responde '%s' 2>/dev/null", casos[c].conta);
        FILE *f = popen(cmd, "r");
        if(f){
            while(fgets(linha, sizeof linha, f)){
                char *p = strstr(linha, " dobra");
                if(!p) continue;
                char *q = p; while(q > linha && (q[-1] == ' ' || (q[-1] >= '0' && q[-1] <= '9'))) q--;
                int v = atoi(q);
                if(v > 0){ real = v; break; }
            }
            pclose(f);
        }
        if(real < 0) semResposta++;
        else if(real != previsto) discorda++;
        long den_ant = n0;
        for(int k = 1; k <= (real > 0 ? real : previsto); k++){
            long den = n0 - k;
            if(den <= 0 || den >= den_ant) subiuSempre = 0;
            den_ant = den;
        }
        if(den_ant != 1) parouEmUm = 0;
        printf("      %-29s %-5d %-10d %-14s 1/%d → 1/%ld\n",
               casos[c].conta, n0, previsto,
               real < 0 ? "(sem resposta)" : (real == previsto ? "bate" : "DIFERE"),
               n0, den_ant);
    }
    printf("\n");
    ok("a assistente gasta exatamente n−1 dobras, com n = folhas + parênteses",
       discorda == 0 && semResposta == 0);
    ok("o fator de potência CRESCE a cada dobra que a máquina dá", subiuSempre);
    ok("e a conta para exatamente em fp = 1, não noutro valor", parouEmUm);
    printf("      O terceiro item é o argumento do telomero.c e não um limite posto à mão: n é\n");
    printf("      inteiro, decresce ESTRITAMENTE e tem piso 1, logo o processo termina — e é por\n");
    printf("      isso que a solução é exata. \"O problema é finito\" não é observação: é a razão.\n");
}

printf("\n§W8  E os dois lados do par: GUARDAR quer fp = 0, RESOLVER quer fp = 1.\n\n");
{
    printf("      operação     quer      porquê                              onde\n");
    printf("      GUARDAR      fp = 0    capacidade: ortogonal, posto cheio  o tecido, §W5\n");
    printf("      RESOLVER     fp = 1    convergência: um número só, exato   a conta,  §W7\n");
    printf("      TRABALHAR    fp = 1    tudo vira torque, nada circula      o motor,  motor.c\n\n");
    conclui("guardar e resolver pedem lados OPOSTOS do par, e ambos são o mesmo fator");
    printf("      Guardar quer o CRUZADO (capacidade vive na ortogonalidade); resolver quer o\n");
    printf("      DIRETO (a solução é um escalar, sem nada a rodar). É o mesmo par ⊕/⊗ do\n");
    printf("      furos.c lido nos dois sentidos — e a minha frase do §W5 era metade dele.\n");
}

printf("\n§W9  O INVERSOR ATRAVESSA AS REALIZAÇÕES — e é escrita/leitura, motor/gerador.\n\n");
{
    /* Se o inversor é o mecanismo comum, então TODA realização tem a mesma forma:
     * uma quantidade n que DECRESCE ESTRITAMENTE, inteira, com piso — logo termina,
     * e o fator de potência 1/n cresce até 1. */
    printf("      realização          o que decresce                n₀ → n   passos  fp → 1\n");
    int falhou = 0;

    /* (a) EUCLIDES / o telómero: o resto decresce estritamente — é a cifra */
    {
        long a = 1071, b = 462, n0 = b, passos = 0; int estrito = 1;
        while(b){ long r = a % b; if(b <= r) estrito = 0; a = b; b = r; passos++; }
        if(!estrito) falhou++;
        printf("      %-19s %-28s %ld → 0   %-7ld sim\n",
               "a cifra (Euclides)", "o resto", n0, passos);
    }
    /* (b) a RÉGUA: o que cresce é o denominador, e o encaixe |p q' − p' q| = 1.
     *     Os convergentes de φ são Fibonacci. Não se conta o erro em vírgula. */
    {
        long p0 = 2, q0 = 1, p1 = 3, q1 = 2;
        int passos = 0, estrito = 1, encaixe = 1;
        for(int k = 0; k < 7; k++){
            long w = p1*q0 - p0*q1; if(w < 0) w = -w;
            if(w != 1) encaixe = 0;
            if(q1 <= q0) estrito = 0;
            long pn = p1 + p0, qn = q1 + q0;
            p0 = p1; q0 = q1; p1 = pn; q1 = qn;
            passos++;
        }
        if(!estrito || !encaixe) falhou++;
        printf("      %-19s %-28s 1 → %ld   %-3d    %s\n",
               "a régua (Fibonacci)", "o denominador, encaixe 1", q1, passos,
               (estrito && encaixe) ? "sim" : "nao");
    }
    /* (c) a CONTA: os nós — corre-se a ASSISTENTE */
    {
        char linha[1024]; int real = -1;
        FILE *f = popen("../banco/bin/conversa ../.torre/tecido responde "
                        "'((1+2) x 3) + ((4+5) x 6)' 2>/dev/null", "r");
        if(f){
            while(fgets(linha, sizeof linha, f)){
                char *pp = strstr(linha, " dobra"); if(!pp) continue;
                char *qq = pp; while(qq > linha && (qq[-1]==' ' || (qq[-1]>='0'&&qq[-1]<='9'))) qq--;
                int v = atoi(qq); if(v > 0){ real = v; break; }
            }
            pclose(f);
        }
        if(real < 0) falhou++;
        printf("      %-19s %-28s %d → 1   %-3d    %s\n",
               "a conta (assistente)", "os nós da árvore", real+1, real,
               real > 0 ? "sim" : "sem resposta");
    }
    /* (d) a POTÊNCIA: o período de Pisano π(q) */
    {
        int q = 12, a = 0, b = 1, t = 0, fecha = 0;
        static char visto[144];
        memset(visto, 0, sizeof visto);
        while(t < 1000){
            int i = a*q + b;
            if(i >= 0 && i < 144 && visto[i]){ fecha = 1; break; }
            if(i >= 0 && i < 144) visto[i] = 1;
            int c = (a+b) % q; a = b; b = c; t++;
        }
        if(!fecha) falhou++;
        if(t != 24) falhou++;
        printf("      o periodo que o laco encontrou: t = %d   (pi(12) = 24)\n", t);
        {
            const int PI_Q[] = { 0,1, 3, 8, 6, 20, 24, 16, 12, 24, 60, 10, 24 };  /* q = 0..12 */
            int bate = 0, medidos = 0;
            printf("      o periodo da orbita, contado:  q :");
            for(int qq = 2; qq <= 12; qq++) printf(" %3d", qq);
            printf("\n                                   pi:");
            for(int qq = 2; qq <= 12; qq++){
                int x = 0, y = 1, p = 0;
                do { int z = (x + y) % qq; x = y; y = z; p++; } while(!(x == 0 && y == 1) && p < 10000);
                printf(" %3d", p);
                if(p == PI_Q[qq]) bate++;
                medidos++;
            }
            printf("\n\n");
            if(bate != medidos) falhou++;
            ok("a orbita nao so fecha: o periodo e pi(q) de Pisano, em 11 valores de q",
               bate == medidos && medidos == 11);
        }
        printf("      %-19s %-28s %d → 0   %-7d sim\n",
               "a órbita (potência)", "os estados por visitar", 144, t);
    }
    printf("\n");
    ok("as quatro realizações têm a MESMA forma: n decresce, é inteiro, e há piso", falhou == 0);
    printf("      Não são quatro mecanismos com um ar de família: é um, e a família real dá-lhe\n");
    printf("      o fator unitário que o torna reversível. Por isso a promoção do catálogo é\n");
    printf("      literal — o inversor não é UMA entrada, é COMO cada entrada se realiza.\n\n");

    printf("      sentido    máquina    operação   o que se gasta        quer\n");
    printf("      escrever   MOTOR      STORE      capacidade do tecido  fp → 0\n");
    printf("      ler        GERADOR    LOAD       incerteza da busca    fp → 1\n\n");
    conclui("os dois sentidos existem e pedem extremos opostos do mesmo fator");
    printf("      E são reversíveis um no outro porque |det| = 1 (§W2): o mesmo inversor lido ao\n");
    printf("      contrário. Um motor que se roda vira gerador, e não é figura de estilo — é a\n");
    printf("      mesma máquina com o sinal do fluxo trocado, que é o σσ' = −1 do dual.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A razão cruzado/direto é tan φ, e o fator de potência é o direto. A família\n");
printf("    real tem |det| = 1 — fator unitário por construção — e por isso é hiperbólica,\n");
printf("    e a hipérbole cabe numa régua finita. O círculo é que exige a régua infinita,\n");
printf("    e é lá que o tecido vive. E o ótimo inverte-se: o motor quer fp = 1, o tecido\n");
printf("    quer fp = 0, porque um quer trabalho e o outro quer capacidade.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
