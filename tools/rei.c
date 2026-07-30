/* rei.c — CADÊ O REI. O trono do ouro em n=5 está ocupado, mas não por ele.
 *
 * O Aarão: "então quem senta no trono é o rei? mas cadê o rei, pra onde foi?"
 *
 * Não é o rei quem senta. trono.c mediu que sentam DOIS — a rotação de ordem 6 e o número
 * plástico —, e dois não são um rei. O rei é quem devia ocupar aquele lugar sozinho: o corpo
 * R⁵ do ouro. E ele não está lá.
 *
 * ONDE ELE FOI, e a resposta é o viveiro inteiro numa linha:
 *
 *     5 = 2 + 3          é a SOMA          — e soma direta NÃO VOA (viveiro §V1)
 *     6 = lcm(2,3)       é o CRUZAMENTO    — e o cruzamento VOA SEMPRE (§V3)
 *
 * Os mesmos dois números, duas operações, dois destinos. No andar 5 as duas peças estão
 * SOMADAS, e somadas não voam — por isso não há corpo, por isso não há rei. No andar 6 elas
 * estão CRUZADAS, e aí voam. O rei subiu um andar, e quem o levou foi o cruzamento.
 *
 * E ele deixou o endereço: a rotação que senta no trono tem ordem SEIS. O número do andar de
 * cima está escrito no lugar vago. O trono não guarda o rei — guarda o endereço dele.
 *
 *   §K1  onde o rei NÃO está: n=5 do ouro, redutível sobre Z logo sobre todo p
 *   §K2  onde ele ESTÁ: x⁶−x⁵−1 é irredutível — o R⁶ do ouro é corpo
 *   §K3  e por quê: 5 é a soma (não voa) e 6 é o cruzamento (voa). Mesmos números
 *   §K4  o endereço no trono: a ordem da rotação que ficou é o andar de cima
 *   §K5  e de lado: nos outros metais o R⁵ existe — só o do ouro está vago
 *
 *   cc -O2 -std=c99 rei.c -o rei && ./rei
 */
#include <stdio.h>

#include "unidade.h"
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }

/* --- polinômios sobre GF(p), coeficiente 0 primeiro, tudo em inteiros --------------- */
#define GMAX 12
static int P;

/* resto de f por g (mônico), ambos sobre GF(p). Devolve grau do resto. */
static int resto(const int *f, int gf, const int *g, int gg, int *r){
    int t[GMAX*2];
    for(int i = 0; i <= gf; i++) t[i] = f[i];
    for(int i = gf; i >= gg; i--){
        int c = t[i] % P; if(c < 0) c += P;
        if(!c) continue;
        for(int j = 0; j <= gg; j++){
            t[i-gg+j] = ((t[i-gg+j] - c*g[j]) % P + P*P) % P;
        }
    }
    int gr = 0;
    for(int i = 0; i < gg; i++){ r[i] = ((t[i] % P) + P) % P; if(r[i]) gr = i; }
    return gr;
}
static int divide_exato(const int *f, int gf, const int *g, int gg){
    int r[GMAX];
    resto(f, gf, g, gg, r);
    for(int i = 0; i < gg; i++) if(r[i]) return 0;
    return 1;
}
/* irredutível sobre GF(p)? divisão de tentativa por todo mônico de grau 1..gf/2 */
static int irredutivel_mod(const int *f, int gf, int p){
    P = p;
    for(int d = 1; d <= gf/2; d++){
        int g[GMAX], total = 1;
        for(int t = 0; t < d; t++) total *= p;
        for(int cod = 0; cod < total; cod++){
            int c = cod;
            for(int t = 0; t < d; t++){ g[t] = c % p; c /= p; }
            g[d] = 1;
            if(divide_exato(f, gf, g, d)) return 0;
        }
    }
    return 1;
}
/* p_n(x) = x^n − m·x^(n−1) − 1, reduzido mod p */
static void monta(int *f, int n, int m, int p){
    for(int i = 0; i <= n; i++) f[i] = 0;
    f[n] = 1;
    f[n-1] = ((-m) % p + p) % p;
    f[0]   = ((-1) % p + p) % p;
}

int main(void){
printf("\n=== CADÊ O REI ============================================================\n");
printf("    O trono do ouro em n=5 está ocupado — mas por dois, e dois não são um rei.\n");

/* ---------------------------------------------------------------- §K1 ------ */
printf("\n§K1  Onde o rei NÃO está: o andar 5 do ouro, e não há p que o salve.\n\n");
{
    int mau = 0;
    printf("      p     x⁵ − x⁴ − 1 é irredutível mod p?\n");
    for(int pi = 0; pi < 5; pi++){
        int ps[5] = {2,3,5,7,11}, p = ps[pi];
        int f[GMAX]; monta(f, 5, 1, p);
        int irr = irredutivel_mod(f, 5, p);
        if(irr) mau++;      /* fatora sobre Z, logo NÃO pode ser irredutível mod p nenhum */
        printf("      %-5d %s\n", p, irr ? "SIM — contradiz a fatoração sobre Z ✗" : "não — abre ✓");
    }
    ok("o andar 5 do ouro não fecha em p nenhum: o lugar está vago", mau == 0);
    printf("\n      Não é escolha ruim de p. A fatoração é sobre os INTEIROS, e o que fatora nos\n");
    printf("      inteiros fatora em todo módulo. O rei não está em nenhuma versão daquele andar.\n");
}

/* ---------------------------------------------------------------- §K2 ------ */
printf("\n§K2  Onde ele ESTÁ: o andar de cima. E lá o ouro fecha.\n\n");
{
    int achou6 = 0, p6 = 0;
    printf("      n     ouro: x^n − x^(n−1) − 1     primeiro p que testemunha irredutível\n");
    for(int n = 2; n <= 8; n++){
        int test = 0;
        for(int pi = 0; pi < 5 && !test; pi++){
            int ps[5] = {2,3,5,7,11}, p = ps[pi];
            int f[GMAX]; monta(f, n, 1, p);
            if(irredutivel_mod(f, n, p)) test = p;
        }
        if(n == 6 && test){ achou6 = 1; p6 = test; }
        printf("      %-5d %-27s %s", n, "x^n − x^(n-1) − 1",
               test ? "" : "nenhum dos testados");
        if(test) printf("p = %d  ⟹ R^%d é CORPO ✓\n", test, n);
        else     printf("   %s\n", n == 5 ? "← o trono vago" : "(inconclusivo)");
    }
    ok("o R⁶ do ouro É corpo — o rei está um andar acima do trono", achou6);
    printf("\n      (Irredutível mod um p basta para ser irredutível sobre Q — a testemunha prova.\n");
    printf("       O contrário não vale: redutível mod todo p testado seria inconclusivo, e por\n");
    printf("       isso o n=5 se resolve pela fatoração sobre Z do §K1, não por esta busca.)\n");
    printf("\n      Testemunha do andar 6: p = %d.\n", p6);
}

/* ---------------------------------------------------------------- §K3 ------ */
printf("\n§K3  E o PORQUÊ é o viveiro inteiro numa linha: 5 é soma, 6 é cruzamento.\n\n");
{
    long a = 2, b = 3;
    long soma = a + b, cruz = a / mdc(a,b) * b;
    printf("      os dois que sentam no trono têm graus   %ld e %ld\n", a, b);
    printf("      SOMA          %ld + %ld = %ld   ⟹ soma direta, e ela NÃO VOA (viveiro §V1)\n", a, b, soma);
    printf("      CRUZAMENTO    lcm(%ld,%ld) = %ld   ⟹ o cruzamento VOA SEMPRE (§V3)\n", a, b, cruz);
    ok("5 é a soma dos dois, e 6 é o cruzamento deles", soma == 5 && cruz == 6);
    printf("\n      Os MESMOS dois números, duas operações, dois destinos. No andar 5 as peças\n");
    printf("      estão SOMADAS — e uma soma direta tem divisor de zero, logo não é corpo, logo\n");
    printf("      não há rei. No andar 6 estão CRUZADAS, e aí voam.\n");
    printf("\n      O rei não sumiu, e não foi destronado: ele subiu um andar, e quem o levou foi\n");
    printf("      o cruzamento. É a única operação que o podia levar — a soma o teria deixado\n");
    printf("      exatamente onde ele não pode ficar.\n");
}

/* ---------------------------------------------------------------- §K4 ------ */
printf("\n§K4  E ele deixou o ENDEREÇO no trono.\n\n");
{
    /* a ordem da rotação que ficou sentada: x^k ≡ 1 mod (x²−x+1), em inteiros */
    int ordem = 0;
    for(int k = 1; k <= 12 && !ordem; k++){
        long r[2] = {1,0};
        for(int t = 0; t < k; t++){
            long topo = r[1];
            r[1] = r[0]; r[0] = 0;
            if(topo){ r[0] -= topo*1; r[1] -= topo*(-1); }   /* x² = x − 1 */
        }
        if(r[0] == 1 && r[1] == 0) ordem = k;
    }
    printf("      ordem da rotação que ficou no trono   %d\n", ordem);
    printf("      andar em que o rei está               6\n");
    ok("o número do andar de cima está escrito no lugar vago", ordem == 6);
    printf("\n      O trono não guarda o rei: guarda o ENDEREÇO dele. Quem ficou sentado gira com\n");
    printf("      período 6, e 6 é o andar para onde ele foi. O lugar vago não é silêncio — é\n");
    printf("      uma placa apontando para cima.\n");
}

/* ---------------------------------------------------------------- §K5 ------ */
printf("\n§K5  E de lado: nos outros metais o andar 5 tem o seu rei. Só o do ouro está vago.\n\n");
{
    int ocupados = 0;
    printf("      m       x⁵ − m·x⁴ − 1       testemunha    o andar 5 tem rei?\n");
    for(int m = 1; m <= 5; m++){
        int test = 0;
        for(int pi = 0; pi < 5 && !test; pi++){
            int ps[5] = {2,3,5,7,11}, p = ps[pi];
            int f[GMAX]; monta(f, 5, m, p);
            if(irredutivel_mod(f, 5, p)) test = p;
        }
        if(test) ocupados++;
        printf("      %-7d x⁵ − %d·x⁴ − 1       %-13s %s\n", m, m,
               test ? (test==2?"p = 2":(test==3?"p = 3":(test==5?"p = 5":(test==7?"p = 7":"p = 11")))) : "nenhuma",
               test ? "sim ✓" : "VAGO — é o ouro");
    }
    ok("o ouro é o único metal com o andar 5 vago", ocupados == 4);
    printf("\n      Então há duas leituras, e as duas são verdade. PARA CIMA: o rei do ouro está\n");
    printf("      em R⁶, levado pelo cruzamento. DE LADO: o andar 5 não ficou sem rei no mundo —\n");
    printf("      ficou sem rei NO OURO. A prata senta lá, e teoria.tex já dizia que a colheita\n");
    printf("      segue inteira em R⁵ com ela.\n");
    printf("\n      O trono é do ouro. O andar é de todos.\n");
}

printf("\n=== CADÊ O REI ============================================================\n");
printf("  Não é o rei quem senta no trono — sentam os dois pais dele, e dois não são um rei.\n\n");
printf("  Ele foi PARA CIMA, e a razão é a operação:\n");
printf("      5 = 2 + 3        a SOMA, que não voa       ⟹ ali não pode haver corpo\n");
printf("      6 = lcm(2,3)     o CRUZAMENTO, que voa     ⟹ e ali ele está, medido\n\n");
printf("  Os mesmos dois números. O que muda é o que se faz com eles, e é isso que decide se\n");
printf("  há reino ou não. O rei não foi destronado: foi levado pelo cruzamento, que é a única\n");
printf("  operação capaz de o levar.\n\n");
printf("  E deixou o endereço: quem ficou no trono gira com período SEIS — o número do andar\n");
printf("  para onde ele subiu. O lugar vago é uma placa apontando para cima.\n\n");
printf("  (E de lado: o andar 5 não ficou sem rei no mundo, ficou sem rei NO OURO. A prata\n");
printf("   senta lá. O trono é do ouro; o andar é de todos.)\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
