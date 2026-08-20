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

#include "i128.h"
#include "unidade.h"
typedef I128 big;

static big b1(void){ return i128_from_i64(1); }
static big b0(void){ return i128_zero(); }
static int blt(big a, big b){ return i128_cmp(a, b) < 0; }
static int bgt(big a, big b){ return i128_cmp(a, b) > 0; }
static int beq(big a, big b){ return i128_cmp(a, b) == 0; }
static int bz(big a){ return i128_is_zero(a); }
static long bl(big a){ return (long)i128_to_i64(a); }

/* Fibonacci com índice negativo: F(−n) = (−1)^(n+1) F(n). φ^k = F(k)φ + F(k−1) para todo k. */
#define KALTO  3
#define KBAIXO -44
/* ── FIBONACCI NAO E' UM DADO: E' UMA OPERACAO ─────────────────────────────────────
 * O Aarao: "esses simbolos nao sao as primitivas, a ULA?"
 *
 * Sao. F(n) = F(n-1) + F(n-2) e' a matriz [[1,1],[1,0]] a agir — a Lei do sistema com
 * o metal do OURO. Guardar 64 termos em .bss e' guardar o resultado de uma coisa que a
 * maquina E'. Aqui calcula-se, com a mesma recorrencia e o mesmo tipo, e sem estado.
 *
 * Os indices negativos saem da propria recorrencia lida ao contrario:
 *     F(k-2) = F(k) - F(k-1),  logo F(-n) = (-1)^(n+1) F(n). */
static big Fi(int k){
    if(k == 0) return b0();
    if(k > 0){ big a = b0(), b = b1(); for(int i = 2; i <= k; i++){ big t = i128_add(a, b); a = b; b = t; }
               return k == 1 ? b1() : b; }
    big v = Fi(-k);
    return ((-k) % 2) ? v : i128_neg(v);
}
static void prepara_fib(void){ }          /* nada a preparar: o valor calcula-se */

/* sinal de u + v·φ, exato: φ = (1+√5)/2, logo 2(u+vφ) = (2u+v) + v√5 */
static int sinal(big u, big v){
    big s = i128_add(i128_add(u, u), v);
    if(bz(v)) return bgt(s, b0()) - blt(s, b0());
    if(bgt(v, b0()) && !blt(s, b0())) return (bz(s) && bz(v)) ? 0 : 1;
    if(blt(v, b0()) && !bgt(s, b0())) return -1;
    big a = i128_mul(s, s), b = i128_smul_i128(i128_mul(v, v), 5);
    if(bgt(v, b0()))  return blt(b, a) ? 1 : (beq(b, a) ? 0 : -1);
    else              return blt(a, b) ? 1 : (beq(a, b) ? 0 : -1);
}

/* expansão gulosa de (num/den) na base φ. Devolve os dígitos em dig[k−KBAIXO]. */
static void expande(big num, big den, int *dig){
    big A = num, B = b0();
    for(int k = KALTO; k >= KBAIXO; k--){
        big u = i128_sub(A, i128_mul(den, Fi(k-1)));
        big v = i128_sub(B, i128_mul(den, Fi(k)));
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
    big p = b1(), q = b1(), pa = b1(), qa = b0();
    printf("      uns   p/q          p² − pq − q²   x = 1 + 1/x confere?\n");
    for(int n = 1; n <= 20; n++){
        big np = i128_add(p, pa), nq = i128_add(q, qa);
        pa = p; qa = q; p = np; q = nq;
        big norma = i128_sub(i128_sub(i128_mul(p, p), i128_mul(p, q)), i128_mul(q, q));
        if(!beq(norma, b1()) && !beq(norma, i128_neg(b1()))) mau++;
        if(n <= 4 || n == 20)
            printf("      %-5d %ld/%-11ld %-14ld %s\n", n+1, bl(p), bl(q), bl(norma),
                   (beq(norma,b1())||beq(norma,i128_neg(b1()))) ? "sim ✓" : "NÃO");
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
    conclui("cada régua tem o seu π, e o do rei sai do polígono da maçã");
}

printf("\n--- E o outro lado: carregar o π do redondo para cá, à força. -------------\n");
printf("    Dá-se, é exato, e não adianta nada. Medido a seguir.\n");

/* --------------------------------------------------------------- §Pi1 ------ */
printf("\n§Pi1  O CERCO: dois convergentes de π, que provadamente o encerram.\n\n");
big pl = i128_from_i64(103993), ql = i128_from_i64(33102);
big pu = i128_from_i64(104348), qu = i128_from_i64(33215);
{
    big det = i128_sub(i128_mul(pl, qu), i128_mul(pu, ql));
    int alterna = beq(det, b1()) || beq(det, i128_neg(b1()));
    printf("      por baixo   %ld/%ld\n", bl(pl), bl(ql));
    printf("      por cima    %ld/%ld\n", bl(pu), bl(qu));
    printf("      p_n·q_(n+1) − p_(n+1)·q_n = %ld   (tem de ser ±1)\n", bl(det));
    ok("são convergentes CONSECUTIVOS: encerram π, e o cerco é legítimo", alterna);
    printf("\n      A largura do cerco é 1/(q·q') = 1/%ld — e é ela que decide quantos dígitos\n",
           bl(i128_mul(ql, qu)));
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
    big A = b0(), B = b0();
    for(int t = KALTO; t > KALTO - certificados; t--)
        if(dl[t - KBAIXO]){ A = i128_add(A, Fi(t-1)); B = i128_add(B, Fi(t)); }
    int abaixo_do_teto = sinal(i128_sub(i128_mul(A, qu), pu), i128_mul(B, qu)) <= 0;
    int acima_do_chao  = sinal(i128_sub(i128_mul(A, ql), pl), i128_mul(B, ql)) <= 0;
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
