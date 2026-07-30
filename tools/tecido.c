/* tecido.c — A MÃE COMPLETA O FILHO? E o resultado ainda é dual?
 *
 * Aarão: "se o filho não é corpo dual não é corpo, é metade; ele precisa do tecido da mãe pra
 * completar. Então testa se a soma direta da mãe com o filho vira filho, e verifica se as
 * operações desse resultado são duais via Pontryagin."
 *
 * A pergunta tem DUAS leituras, e elas dão respostas opostas. Medir as duas separadamente é a
 * única forma de não misturar:
 *
 *   EXTERNA  R^i ⊕ R^k, pendurando uma cópia da mãe por fora do filho. Tem p^(i+k) elementos,
 *            logo não vira o filho, e traz divisor de zero de volta. §T1.
 *
 *   INTERNA  R^k = R^i ⊕ (R^i)^⊥, o tecido da mãe MAIS o que ela não enxerga, os dois já
 *            dentro do filho. As dimensões sempre fecham (i + (k−i) = k), mas o encaixe só
 *            acontece quando r = k/i é ÍMPAR. §T3.
 *
 * E o achado que eu não esperava: com r PAR a mãe está dentro do próprio anulador, e em r=2 —
 * LA HIRE — ela É o próprio anulador, exatamente. Autortogonal. §T4. Ali o tecido que completa
 * a mãe é o mesmo tecido da mãe: ela é o seu próprio dual, e não há o que pendurar.
 *
 * A resposta sobre Pontryagin, §T5: na soma direta a SOMA continua dual (a transformada leva
 * convolução em produto em todo grupo abeliano finito — não precisa de corpo), mas a
 * MULTIPLICAÇÃO deixa de ser: os divisores de zero não permutam o dual. Completar por fora
 * guarda metade da dualidade e perde a outra.
 *
 *   §T1  a soma direta externa não vira o filho — tamanho errado e divisor de zero
 *   §T2  e o filho não é metade nenhuma: todo não-nulo dele tem inverso
 *   §T3  a decomposição interna R^k = R^i ⊕ (R^i)^⊥ só fecha com r ÍMPAR
 *   §T4  com r PAR a mãe está no próprio anulador; em r=2 ela É o anulador
 *   §T5  Pontryagin na soma direta: a soma continua dual, a multiplicação não
 *
 *   cc -O2 -std=c99 tecido.c -o tecido && ./tecido
 */
#include <stdio.h>

#define KMAX 6
static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-58s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}

static unsigned RED[KMAX+1];
static unsigned mulk(unsigned a, unsigned b, int n){
    unsigned r = 0, red = RED[n];
    for(int i = 0; i < n; i++) if((b >> i) & 1u) r ^= a << i;
    for(int t = 2*n-2; t >= n; t--) if((r >> t) & 1u) r ^= red << (t - n);
    return r & ((1u << n) - 1u);
}
static int irred(unsigned poly, int n){
    unsigned m = 1u << n;
    for(unsigned a = 2; a < m; a++)
        for(unsigned b = 2; b < m; b++){
            unsigned p = 0;
            for(int i = 0; i < n; i++) if((b >> i) & 1u) p ^= a << i;
            if(p == (poly | (1u << n))) return 0;
        }
    return 1;
}
static void prepara(void){
    for(int n = 1; n <= KMAX; n++){
        RED[n] = 0;
        for(unsigned p = 0; p < (1u << n); p++)
            if(irred(p, n)){ RED[n] = p | (1u << n); break; }
    }
}
static unsigned frob(unsigned x, int t, int n){
    for(int s = 0; s < t; s++) x = mulk(x, x, n);
    return x;
}
static unsigned tra(unsigned x, int k){
    unsigned r = 0;
    for(int t = 0; t < k; t++) r ^= frob(x, t, k);
    return r;
}
static unsigned tracado(unsigned x, int i, int k){
    unsigned r = 0;
    for(int t = 0; t < k/i; t++) r ^= frob(x, i*t, k);
    return r;
}

int main(void){
prepara();
printf("\n=== A MÃE COMPLETA O FILHO? E o resultado ainda é dual? ===================\n");
printf("    Duas leituras da soma direta, e elas dão respostas opostas.\n");

/* ---------------------------------------------------------------- §T1 ------ */
printf("\n§T1  A soma direta EXTERNA não vira o filho: tamanho errado, e volta o divisor.\n\n");
{
    int mau_t = 0, mau_d = 0;
    printf("      i   k   |R^i ⊕ R^k| = 2^(i+k)   |filho| = 2^k   igual?   divisor de zero\n");
    for(int k = 2; k <= 6; k++) for(int i = 1; i < k; i++){
        if(k % i) continue;
        long soma = 1L << (i+k), filho = 1L << k;
        if(soma == filho) mau_t++;
        /* procura o divisor de zero, não o afirma: dois não-nulos cujo produto zera */
        int div = 0;
        for(long u = 1; u < soma && !div; u++) for(long v = 1; v < soma && !div; v++){
            unsigned ua = u / filho, ub = u % filho, va = v / filho, vb = v % filho;
            if(mulk(ua,va,i) == 0 && mulk(ub,vb,k) == 0) div = 1;
        }
        if(!div) mau_d++;
        if((i==1&&k==2)||(i==2&&k==4)||(i==2&&k==6)||(i==3&&k==6))
            printf("      %d   %d   %21ld   %14ld   %6s   %s\n", i, k, soma, filho,
                   soma==filho?"sim":"não", div?"achado ✓":"não achado");
    }
    ok("2^(i+k) nunca é 2^k — pendurar a mãe por fora não devolve o filho", mau_t == 0);
    ok("e o divisor de zero foi ACHADO por busca em todo par testado", mau_d == 0);
    printf("\n      Pendurar a mãe por fora não completa nada: aumenta. E o que era corpo\n");
    printf("      deixa de ser, porque a componente que zera de um lado zera o produto todo.\n");
}

/* ---------------------------------------------------------------- §T2 ------ */
printf("\n§T2  E o filho não é metade nenhuma: todo não-nulo dele tem inverso.\n\n");
{
    int mau = 0;
    printf("      k   não-nulos   com inverso   falta algum?\n");
    for(int k = 1; k <= 6; k++){
        long tot = 1L << k, com = 0;
        for(unsigned x = 1; x < tot; x++){
            int achou = 0;
            for(unsigned y = 1; y < tot && !achou; y++) if(mulk(x,y,k) == 1) achou = 1;
            if(achou) com++;
        }
        if(com != tot-1) mau++;
        printf("      %d   %11ld   %12ld   %s\n", k, tot-1, com, com==tot-1?"nenhum ✓":"SIM ✗");
    }
    ok("o filho é corpo INTEIRO — nada nele está pela metade", mau == 0);
    printf("\n      \"Metade\" ele é no EMPARELHAMENTO, não na álgebra: a mãe ocupa metade do\n");
    printf("      espaço dual e o anulador dela a outra. Mas como corpo ele está completo, e\n");
    printf("      acrescentar tecido por fora só estraga o que já fechava.\n");
}

/* ---------------------------------------------------------------- §T3 ------ */
printf("\n§T3  A decomposição INTERNA: R^k = R^i ⊕ (R^i)^⊥, e ela só fecha com r ÍMPAR.\n\n");
{
    int mau = 0;
    printf("      i   k   r=k/i   |mãe ∩ anulador|   soma direta?   r é ímpar?\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        int r = k / i;
        long tot = 1L << k, inter = 0;
        for(unsigned x = 0; x < tot; x++)
            if(frob(x, i, k) == x && tracado(x, i, k) == 0) inter++;   /* mãe ∩ ker T */
        int direta = (inter == 1);          /* só o 0 em comum */
        if(direta != (r % 2 == 1)) mau++;   /* a previsão: fecha ⟺ r ímpar */
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==3&&i==1)||(k==6&&i==1))
            printf("      %d   %d   %5d   %17ld   %13s   %s\n", i, k, r, inter,
                   direta?"sim ✓":"não", (r%2)?"sim":"não");
    }
    ok("a mãe e o seu anulador fecham em soma direta ⟺ r é ÍMPAR", mau == 0);
    printf("\n      Com r ímpar o tecido da mãe e o que ela não enxerga são disjuntos e juntos\n");
    printf("      dão o filho inteiro: aí sim, a mãe COMPLETA — mas por dentro, não por fora.\n");
    printf("      Ela já estava lá; o que faltava era ver a outra metade.\n");
}

/* ---------------------------------------------------------------- §T4 ------ */
printf("\n§T4  Com r PAR a mãe está no próprio anulador. E em r=2 ela É o anulador.\n\n");
{
    int mau_c = 0, mau_i = 0;
    printf("      i   k   r   mãe ⊆ anulador?   |mãe|   |anulador|   mãe = anulador?\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        int r = k / i;
        long tot = 1L << k, nmae = 0, nanu = 0;
        int contida = 1, igual = 1;
        for(unsigned x = 0; x < tot; x++){
            int m = (frob(x, i, k) == x), a = (tracado(x, i, k) == 0);
            if(m) nmae++;
            if(a) nanu++;
            if(m && !a) contida = 0;
            if(m != a) igual = 0;
        }
        if(contida != (r % 2 == 0)) mau_c++;          /* contida ⟺ r par */
        if(igual != (r == 2)) mau_i++;                /* igual ⟺ r = 2 */
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==3)||(k==6&&i==2)||(k==4&&i==1))
            printf("      %d   %d   %d   %15s   %5ld   %10ld   %s\n", i, k, r,
                   contida?"sim":"não", nmae, nanu, igual?"SIM — La Hire ✓":"não");
    }
    ok("mãe ⊆ próprio anulador exatamente quando r é PAR", mau_c == 0);
    ok("e mãe = anulador exatamente quando r = 2 — La Hire", mau_i == 0);
    printf("\n      Este é o caso que responde à sua intuição pelo avesso. Em r=2 a mãe é o SEU\n");
    printf("      PRÓPRIO tecido complementar: autortogonal, o seu dual é ela mesma. Não há o\n");
    printf("      que pendurar por fora porque a peça que faltaria já é ela.\n");
    printf("\n      E é a mesma razão de sempre: r=2 é a involução, a reta de La Hire, a ida que\n");
    printf("      É a volta. O espelho não devolve outra coisa — devolve a própria mãe.\n");
}

/* ---------------------------------------------------------------- §T5 ------ */
printf("\n§T5  PONTRYAGIN na soma direta: a soma continua dual, a multiplicação NÃO.\n\n");
{
    int mau_s = 0, mau_m = 0;
    printf("      i   k   convolução → produto   a≠0 que permutam   total a≠0   todos?\n");
    for(int k = 2; k <= 6; k++) for(int i = 1; i < k; i++){
        if(k % i) continue;
        long ni = 1L << i, nk = 1L << k, n = ni * nk;
        /* caractere do GRUPO R^i ⊕ R^k: χ_(a,b)(x,y) = (−1)^(tr_i(ax) + tr_k(by)) */
        int bom = 1;
        {
            long f[512], g[512], conv[512];
            for(long t = 0; t < n; t++){
                f[t] = ((t*7 + 1) % 5) - 2;
                g[t] = ((t*11 + 2) % 7) - 3;
            }
            for(long z = 0; z < n; z++){
                conv[z] = 0;
                for(long y = 0; y < n; y++){
                    /* a soma do grupo: XOR em cada componente, separadamente */
                    long zx = z / nk, zy = z % nk, yx = y / nk, yy = y % nk;
                    long d = (zx ^ yx) * nk + (zy ^ yy);
                    conv[z] += f[y] * g[d];
                }
            }
            for(long c = 0; c < n; c++){
                long a = c / nk, b = c % nk, cf = 0, cg = 0, cc = 0;
                for(long y = 0; y < n; y++){
                    long yx = y / nk, yy = y % nk;
                    unsigned e = tra(mulk((unsigned)a,(unsigned)yx,i), i)
                               ^ tra(mulk((unsigned)b,(unsigned)yy,k), k);
                    int ch = e ? -1 : 1;
                    cf += f[y]*ch; cg += g[y]*ch; cc += conv[y]*ch;
                }
                if(cc != cf*cg) bom = 0;
            }
        }
        /* e a multiplicação componente a componente: quem permuta o espaço? */
        long permutam = 0;
        for(long c = 1; c < n; c++){
            long a = c / nk, b = c % nk;
            static char vis[512];
            for(long t = 0; t < n; t++) vis[t] = 0;
            int perm = 1;
            for(long y = 0; y < n && perm; y++){
                long yx = y / nk, yy = y % nk;
                long im = (long)mulk((unsigned)a,(unsigned)yx,i) * nk + mulk((unsigned)b,(unsigned)yy,k);
                if(vis[im]) perm = 0;
                vis[im] = 1;
            }
            if(perm) permutam++;
        }
        long esperado = (ni - 1) * (nk - 1);   /* só quem é não-nulo nas DUAS componentes */
        if(!bom) mau_s++;
        if(permutam != esperado || permutam == n-1) mau_m++;
        printf("      %d   %d   %20s   %17ld   %10ld   %s\n", i, k, bom?"exato ✓":"FALHOU",
               permutam, n-1, permutam==n-1?"sim":"NÃO");
    }
    ok("a SOMA continua dual — a transformada não precisa de corpo", mau_s == 0);
    ok("mas a MULTIPLICAÇÃO não: só (2^i−1)(2^k−1) permutam, e faltam os outros", mau_m == 0);
    printf("\n      Aqui está a resposta inteira. Completar por soma direta guarda METADE da\n");
    printf("      dualidade e perde a outra: a soma continua a virar produto do outro lado do\n");
    printf("      espelho, porque para isso basta ser grupo. Mas a multiplicação deixa de\n");
    printf("      permutar o dual, e quem falha são exatamente os divisores de zero — os que\n");
    printf("      têm uma componente nula esmagam o espaço em vez de girá-lo.\n");
}

printf("\n=== A RESPOSTA ============================================================\n");
printf("  PENDURAR A MÃE POR FORA não completa: R^i ⊕ R^k tem 2^(i+k), não vira o filho,\n");
printf("  e traz o divisor de zero de volta. O filho já era corpo inteiro — todo não-nulo\n");
printf("  dele tem inverso —, então não havia metade a completar na álgebra.\n\n");
printf("  O QUE COMPLETA É POR DENTRO: R^k = R^i ⊕ (R^i)^⊥, o tecido da mãe mais o que ela\n");
printf("  não enxerga. E isso fecha exatamente quando r = k/i é ÍMPAR.\n\n");
printf("  COM r PAR a mãe está dentro do próprio anulador — e em r=2, LA HIRE, ela É o\n");
printf("  anulador. Autortogonal: o tecido que a completaria já é ela mesma, e não há o que\n");
printf("  pendurar. O espelho, ali, devolve a própria mãe.\n\n");
printf("  E PONTRYAGIN na soma direta: a SOMA continua dual (basta ser grupo), a\n");
printf("  MULTIPLICAÇÃO não (os divisores de zero não permutam o dual). Completar por fora\n");
printf("  guarda metade da dualidade e perde a outra.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
