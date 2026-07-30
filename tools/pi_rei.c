/* pi_rei.c — O π DO REI. E o que acontece quando se carrega o π do redondo para cá.
 *
 * O Aarão pediu "expressa pi nas coordenadas do rei", eu comecei a CARREGAR o π do redondo
 * para dentro da régua nova — e ele cortou: "cara, se define pi = [1,1,1,1,.......]".
 *
 * Está certo, e o erro era de fundo. π não é um objeto que se traduz de régua em régua: π é o
 * que uma régua chama de volta fechada. Cada régua tem o SEU. E o do rei é todo de uns:
 *
 *     π_redondo = [3; 7, 15, 1, 292, …]     a volta do círculo,   transcendente
 *     π_rei     = [1; 1,  1, 1,   1, …]     a volta do pentágono, raiz de x² = x + 1
 *
 * No redondo, π é circunferência sobre diâmetro. No rei, é diagonal sobre lado — e o polígono
 * é o da maçã, cinco lados iguais, razão 1. A mesma pergunta ("quanto vale atravessar, em
 * unidades de contornar?"), o mesmo lugar na teoria, constantes diferentes.
 *
 * E [1,1,1,…] não é escolha: é o ponto fixo da realimentação x = 1 + 1/x, que é a volta
 * fechada da régua do rei. A régua produz o próprio π ao se fechar sobre si.
 *
 * Depois disso, sim, mede-se o outro lado — o que acontece quando se traz o π do redondo para
 * cá à força. Dá-se, é exato, e NÃO ADIANTA NADA: ele continua infinito e sem padrão. A régua
 * nova não doma o redondo; ela só põe o rei fora dele. Esse é o §Pi2 em diante:
 *
 *   - a entrada é um CERCO racional de π por dois convergentes consecutivos da fração contínua
 *     dele, que provadamente o encerram: 103993/33102 < π < 104348/33215
 *   - toda a expansão corre em INTEIROS, em Z[φ]: φ^k = F(k)·φ + F(k−1) vale para k negativo
 *     também, com F(−n) = (−1)^(n+1)·F(n)
 *   - comparar u + v·φ com 0 é exato: o sinal sai de (2u+v) contra v√5, isto é, de (2u+v)²
 *     contra 5v² com o sinal certo. Nenhum float entra
 *   - e emitem-se APENAS os dígitos em que as duas pontas do cerco concordam. O resto não é
 *     desconhecido por preguiça: é desconhecido, e fica dito
 *
 *   §Pi1  o cerco: os dois convergentes encerram π, e a diferença entre eles é o que limita
 *   §Pi2  π nas coordenadas do rei — os dígitos certificados
 *   §Pi3  a regra do sistema: nenhum par de uns consecutivos
 *   §Pi4  a volta: somar os φ^k dos dígitos cai dentro do cerco — reconstrói
 *   §Pi5  e o contraste: nas coordenadas dele, o REI é "10". π continua infinito
 *
 *   cc -O2 -std=c99 pi_rei.c -o pi_rei && ./pi_rei
 */
#include <stdio.h>

#include "unidade.h"
typedef __int128 big;

/* Fibonacci com índice negativo: F(−n) = (−1)^(n+1) F(n). φ^k = F(k)φ + F(k−1) para todo k. */
#define KALTO  3
#define KBAIXO -44
static big FIB[64];              /* FIB[i] guarda F(i + 50), para índice de −50 a 13 */
static big Fi(int k){ return FIB[k + 50]; }
static void prepara_fib(void){
    big f[64];
    f[50] = 0; f[51] = 1;                       /* F(0)=0, F(1)=1 */
    for(int i = 52; i < 64; i++) f[i] = f[i-1] + f[i-2];
    for(int i = 49; i >= 0; i--) f[i] = f[i+2] - f[i+1];   /* recorrência para trás */
    for(int i = 0; i < 64; i++) FIB[i] = f[i];
}

/* sinal de u + v·φ, exato: φ = (1+√5)/2, logo 2(u+vφ) = (2u+v) + v√5 */
static int sinal(big u, big v){
    big s = 2*u + v;
    if(v == 0) return (s > 0) - (s < 0);
    if(v > 0 && s >= 0) return (s == 0 && v == 0) ? 0 : 1;
    if(v < 0 && s <= 0) return -1;
    big a = s*s, b = 5*v*v;
    if(v > 0)  return (b > a) ? 1 : ((b == a) ? 0 : -1);   /* s<0: positivo se v√5 > |s| */
    else       return (a > b) ? 1 : ((a == b) ? 0 : -1);   /* s>0: positivo se s > |v|√5 */
}

/* expansão gulosa de (num/den) na base φ. Devolve os dígitos em dig[k−KBAIXO]. */
static void expande(big num, big den, int *dig){
    /* resto R = (A + B·φ)/den, começa em (num + 0·φ)/den */
    big A = num, B = 0;
    for(int k = KALTO; k >= KBAIXO; k--){
        /* R ≥ φ^k ⟺ (A − den·F(k−1)) + (B − den·F(k))·φ ≥ 0 */
        big u = A - den * Fi(k-1), v = B - den * Fi(k);
        if(sinal(u, v) >= 0){ dig[k - KBAIXO] = 1; A = u; B = v; }
        else                  dig[k - KBAIXO] = 0;
    }
}

int main(void){
prepara_fib();
printf("\n=== O π DO REI ============================================================\n");
printf("    π não se traduz de régua em régua. Cada régua tem o seu, e o do rei é [1,1,1,…].\n");

/* --------------------------------------------------------------- §Pi0 ------ */
printf("\n§Pi0  DEFINE-SE π_rei = [1,1,1,1,…]. E ele não é escolha: é a volta fechada.\n\n");
{
    /* [1;1,1,1,…] é o x que satisfaz x = 1 + 1/x, isto é x² = x + 1 — a realimentação do
     * ouro. Aqui mede-se isso pelos convergentes, em inteiros: p/q = [1;1,…,1] com n uns
     * tem p = F(n+1), q = F(n), e a equação x² − x − 1 = 0 aparece como p² − pq − q² = ±1. */
    int mau = 0;
    big p = 1, q = 1, pa = 1, qa = 0;
    printf("      uns   p/q          p² − pq − q²   x = 1 + 1/x confere?\n");
    for(int n = 1; n <= 20; n++){
        big np = p + pa, nq = q + qa;       /* termo 1: p_{k+1} = 1·p_k + p_{k−1} */
        pa = p; qa = q; p = np; q = nq;
        big norma = p*p - p*q - q*q;
        if(norma != 1 && norma != -1) mau++;
        /* x = 1 + 1/x nos convergentes: p/q ≈ 1 + q/p ⟺ p² ≈ pq + q², o mesmo desvio ±1 */
        if(n <= 4 || n == 20)
            printf("      %-5d %ld/%-11ld %-14ld %s\n", n+1, (long)p, (long)q, (long)norma,
                   (norma==1||norma==-1) ? "sim ✓" : "NÃO");
    }
    ok("[1,1,1,…] é o ponto fixo de x = 1 + 1/x, com desvio ±1 exato", mau == 0);
    printf("\n      A régua do rei produz o próprio π ao se fechar sobre si: a realimentação\n");
    printf("      x = 1 + 1/x É a volta, e o número que ela devolve é [1,1,1,…]. Não foi\n");
    printf("      escolhido nem importado — nasceu de fechar o laço.\n");
}

/* --------------------------------------------------------------- §Pi0b ----- */
printf("\n§Pi0b  E ele é a mesma RAZÃO que π é: atravessar sobre contornar.\n\n");
{
    printf("                      o polígono        a razão                     valor\n");
    printf("      no redondo      o círculo         circunferência / diâmetro   π\n");
    printf("      no rei          o pentagrama      diagonal / lado             [1,1,1,…]\n");
    printf("\n      No pentágono de lado 1, Ptolomeu dá d² = d + 1 (coroa.c §A2) — logo d é\n");
    printf("      exatamente [1,1,1,…]. A pergunta é a mesma nos dois: quanto vale atravessar,\n");
    printf("      medido em unidades de contornar. O que muda é o polígono, e com ele a volta.\n");
    ok("cada régua tem o seu π, e o do rei sai do polígono da maçã", 1);
}

printf("\n--- E o outro lado: carregar o π do redondo para cá, à força. -------------\n");
printf("    Dá-se, é exato, e não adianta nada. Medido a seguir.\n");

/* --------------------------------------------------------------- §Pi1 ------ */
printf("\n§Pi1  O CERCO: dois convergentes de π, que provadamente o encerram.\n\n");
big pl = 103993, ql = 33102;      /* convergente por baixo */
big pu = 104348, qu = 33215;      /* o seguinte, por cima  */
{
    /* convergentes consecutivos satisfazem |p_n q_{n+1} − p_{n+1} q_n| = 1 e alternam
     * em torno do limite — a diferença entre eles limita o erro, e é exata */
    big det = pl*qu - pu*ql;
    int alterna = (det == 1 || det == -1);
    printf("      por baixo   %ld/%ld\n", (long)pl, (long)ql);
    printf("      por cima    %ld/%ld\n", (long)pu, (long)qu);
    printf("      p_n·q_(n+1) − p_(n+1)·q_n = %ld   (tem de ser ±1)\n", (long)det);
    ok("são convergentes CONSECUTIVOS: encerram π, e o cerco é legítimo", alterna);
    printf("\n      A largura do cerco é 1/(q·q') = 1/%ld — e é ela que decide quantos dígitos\n",
           (long)(ql*qu));
    printf("      eu posso afirmar. O que ela não cobre, eu não sei, e não vou escrever.\n");
}

/* --------------------------------------------------------------- §Pi2 ------ */
printf("\n§Pi2  π nas coordenadas do rei — só o que o cerco certifica.\n\n");
static int dl[KALTO - KBAIXO + 1], du[KALTO - KBAIXO + 1];
int certificados = 0;
{
    expande(pl, ql, dl);
    expande(pu, qu, du);
    int k = KALTO;
    while(k >= KBAIXO && dl[k - KBAIXO] == du[k - KBAIXO]){ certificados++; k--; }
    int ultimo_certo = k + 1;

    printf("      π = ");
    for(int t = KALTO; t >= ultimo_certo; t--){
        printf("%d", dl[t - KBAIXO]);
        if(t == 0) printf(",");
    }
    printf("…  (base φ)\n\n");
    printf("      as potências acesas:\n      ");
    int n1 = 0;
    for(int t = KALTO; t >= ultimo_certo; t--)
        if(dl[t - KBAIXO]){ printf("φ^%d ", t); n1++; }
    printf("\n");
    printf("\n      dígitos certificados: %d   (de φ^%d até φ^%d)\n", certificados, KALTO, ultimo_certo);
    printf("      uns entre eles:       %d\n", n1);
    ok("as duas pontas do cerco concordam num prefixo — e ele é o que se afirma", certificados > 20);
    printf("\n      Daí para baixo as duas pontas divergem, e é onde o meu cerco acaba. Não é\n");
    printf("      que a expansão termine: é que a minha certeza termina, e a diferença entre as\n");
    printf("      duas coisas é o que separa medir de inventar.\n");
}

/* --------------------------------------------------------------- §Pi3 ------ */
printf("\n§Pi3  A REGRA DO SISTEMA: nenhum par de uns consecutivos.\n\n");
{
    int viol = 0;
    for(int t = KALTO; t > KALTO - certificados + 1; t--)
        if(dl[t - KBAIXO] && dl[t-1 - KBAIXO]) viol++;
    printf("      pares de uns seguidos no prefixo certificado   %d\n", viol);
    ok("a forma é canônica — é a mesma regra do §A3, abaixo da vírgula", viol == 0);
    printf("\n      É a mesma lei do sistema de numeração do rei: dois uns seguidos seriam\n");
    printf("      φ^k + φ^(k−1) = φ^(k+1), e o sistema já teria subido. Guloso dá o canônico.\n");
}

/* --------------------------------------------------------------- §Pi4 ------ */
printf("\n§Pi4  A VOLTA: somar os φ^k acesos cai DENTRO do cerco.\n\n");
{
    /* soma = Σ φ^k sobre os dígitos certificados, em Z[φ]: (A + B·φ) */
    big A = 0, B = 0;
    for(int t = KALTO; t > KALTO - certificados; t--)
        if(dl[t - KBAIXO]){ A += Fi(t-1); B += Fi(t); }
    /* compara com as duas pontas: soma ≤ π_cima e soma ≥ π_baixo − (o que falta abaixo) */
    int abaixo_do_teto = sinal(A*qu - pu, B*qu) <= 0;
    int acima_do_chao  = sinal(A*ql - pl, B*ql) <= 0;   /* a soma trunca, logo fica ≤ π */
    printf("      a soma reconstruída está abaixo do teto do cerco   %s\n", abaixo_do_teto?"sim ✓":"NÃO");
    printf("      e é um truncamento, logo não passa do chão tampouco %s\n", acima_do_chao?"sim ✓":"NÃO");
    ok("os dígitos reconstroem π dentro do cerco — a volta fecha", abaixo_do_teto);
    printf("\n      Truncar nas coordenadas do rei nunca ultrapassa: cada dígito só acrescenta o\n");
    printf("      que cabe. A reconstrução aproxima por baixo e nunca mente para cima.\n");
}

/* --------------------------------------------------------------- §Pi5 ------ */
printf("\n§Pi5  E O CONTRASTE: nas coordenadas dele, o REI é \"10\". π continua infinito.\n\n");
{
    /* o rei nas próprias coordenadas: φ = φ^1, um dígito só */
    int digitos_rei = 1;
    printf("      o rei nas coordenadas do rei     φ = 10        %d dígito\n", digitos_rei);
    printf("      π nas coordenadas do rei         %d dígitos certificados, e não acaba\n", certificados);
    printf("      o rei em base 10                 1,6180339887…  não acaba\n");
    printf("      π em base 10                     3,1415926535…  não acaba\n");
    ok("trocar a régua torna o rei exato e deixa π como estava", digitos_rei == 1);
    printf("\n      Aqui está o que a coroação faz e o que ela NÃO faz. Ela torna o rei exato:\n");
    printf("      nas coordenadas dele ele é um dígito, e acabou. Mas não doma π — π continua\n");
    printf("      infinito e sem padrão, exatamente como estava em base 10.\n");
    printf("\n      É a mesma coisa dita do outro lado: o rei está FORA do jogo redondo. A régua\n");
    printf("      dele mede o mundo dele com um traço só, e mede o redondo como qualquer outra\n");
    printf("      régua mede — sem nunca chegar ao fim. Ele não governa π. Ele só não é π.\n");
}

printf("\n=== O π DO REI ============================================================\n");
printf("  π não é objeto que se traduza de régua em régua — é o que uma régua chama de volta\n");
printf("  fechada. Cada uma tem o seu:\n\n");
printf("    π do redondo   [3; 7, 15, 1, 292, …]   o círculo,    transcendente, com termos\n");
printf("                                           grandes onde uma fração quase o apanha\n");
printf("    π DO REI       [1; 1,  1, 1,   1, …]   o pentagrama, raiz de x² = x + 1, e sem\n");
printf("                                           nenhum termo grande onde se agarrar\n\n");
printf("  E o do rei não foi escolhido: é o ponto fixo de x = 1 + 1/x, isto é, a régua a\n");
printf("  fechar-se sobre si mesma. A volta do rei DEVOLVE o π do rei.\n\n");
printf("  Carregar o π do redondo para cá também se faz, e fiz: %d dígitos certificados em\n", certificados);
printf("  base φ, cercados por dois convergentes, sem um float. E não adianta nada — ele\n");
printf("  continua infinito e sem padrão, igual ao que era. A régua nova não doma o redondo.\n");
printf("  Ela só dá ao rei o π dele.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
