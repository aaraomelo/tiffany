/* dualidade.c — A SOMA E A MULTIPLICAÇÃO DO FILHO SÃO DUAIS? Pontryagin responde.
 *
 * Pergunta do Aarão. A resposta tem duas metades, e a primeira é um NÃO que precisa ser dito
 * antes do sim, senão o sim vira exagero:
 *
 *   NÃO como grupos. (R^k,+) tem 2^k elementos e (R^k*,×) tem 2^k−1. Ordens diferentes, logo
 *   um não é o grupo dual do outro — cada um é, separadamente, autodual. Medido em §P0.
 *
 *   SIM pelo emparelhamento, e de um jeito muito mais forte do que "dual como grupos" seria:
 *   a transformada de Pontryagin do corpo TROCA UMA OPERAÇÃO PELA OUTRA. A convolução — que é
 *   feita com a SOMA — vira o PRODUTO ponto a ponto. É a dualidade soma↔produto, e é exata.
 *
 * E o achado que fecha com o hipociclo: o mapa da dualidade É O TRAÇADO. O emparelhamento é
 * ⟨x,y⟩ = tr(xy), o traço; a inclusão do pai no filho e o traçado do filho no pai são ADJUNTOS
 * um do outro; e o anulador do pai é exatamente o núcleo do traçado, de tamanho p^(k−i) — o
 * MESMO número que hipociclo.c §H4 mediu como fibra da curva. Um número, duas leituras.
 *
 *   §P0  o que NÃO é verdade: as ordens são 2^k e 2^k−1, não são duais como grupos
 *   §P1  o emparelhamento tr(xy) é não-degenerado, e dá os 2^k caracteres, todos distintos
 *   §P2  A DUALIDADE: a transformada leva CONVOLUÇÃO (soma) em PRODUTO ponto a ponto
 *   §P3  e a multiplicação age no dual: multiplicar por a de um lado é multiplicar do outro
 *   §P4  o traçado é adjunto da inclusão — Tr_i(T(x)u) = Tr_k(xu), e Tr_k = Tr_i ∘ T
 *   §P5  o anulador do pai é o núcleo do traçado, p^(k−i); e o dual do dual volta
 *
 * Tudo em inteiros: em característica 2 os caracteres valem ±1, então não entra um único float.
 *
 *   cc -O2 -std=c99 dualidade.c -o dualidade && ./dualidade
 */
#include <stdio.h>

#define KMAX 6
#include "unidade.h"

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
/* o traço absoluto: soma das k posições da roda. Cai em {0,1}. */
static unsigned tra(unsigned x, int k){
    unsigned r = 0;
    for(int t = 0; t < k; t++) r ^= frob(x, t, k);
    return r;
}
/* o traço relativo — o TRAÇADO do hipociclo.c: r = k/i pontas, cai em R^i */
static unsigned tracado(unsigned x, int i, int k){
    unsigned r = 0;
    for(int t = 0; t < k/i; t++) r ^= frob(x, i*t, k);
    return r;
}
/* o traço absoluto da MÃE, mas calculado dentro do filho: a mãe não tem
 * representação própria aqui — os seus elementos estão espalhados em R^k. */
static unsigned tra_mae(unsigned z, int i, int k){
    unsigned r = 0;
    for(int t = 0; t < i; t++) r ^= frob(z, t, k);
    return r;
}
/* o caractere: χ_x(y) = (−1)^tr(xy). Vale ±1 — inteiro, sem float nenhum. */
static int chi(unsigned x, unsigned y, int k){ return tra(mulk(x, y, k), k) ? -1 : 1; }

int main(void){
prepara();
printf("\n=== SOMA E MULTIPLICAÇÃO SÃO DUAIS? ======================================\n");
printf("    Pontryagin responde, e a resposta tem um NÃO antes do sim.\n");

/* ---------------------------------------------------------------- §P0 ------ */
printf("\n§P0  O que NÃO é verdade: não são duais COMO GRUPOS.\n\n");
{
    int mau = 0;
    printf("      k   |(R^k,+)| = 2^k   |(R^k*,×)| = 2^k−1   iguais?\n");
    for(int k = 1; k <= 6; k++){
        long adit = 1L << k, mult = adit - 1;
        if(adit == mult) mau++;
        printf("      %d   %15ld   %19ld   %s\n", k, adit, mult, adit==mult?"sim":"não");
    }
    ok("as ordens NUNCA coincidem — um não é o dual do outro", mau == 0);
    printf("\n      Cada um é autodual por si (grupo abeliano finito sempre é), mas não são\n");
    printf("      duais um do outro: têm tamanhos diferentes e nem podiam ser. Dizer o\n");
    printf("      contrário seria inventar. A dualidade que EXISTE é outra, e é mais forte.\n");
}

/* ---------------------------------------------------------------- §P1 ------ */
printf("\n§P1  O emparelhamento ⟨x,y⟩ = tr(xy) é não-degenerado.\n\n");
{
    int mau_d = 0, mau_c = 0, mau_v = 0;
    printf("      k   tr cai em {0,1}   todo x≠0 tem par   caracteres distintos   2^k\n");
    for(int k = 1; k <= 6; k++){
        long tot = 1L << k;
        int val = 1, ndeg = 1;
        for(unsigned x = 0; x < tot; x++) if(tra(x,k) > 1) val = 0;
        for(unsigned x = 1; x < tot; x++){
            int achou = 0;
            for(unsigned y = 0; y < tot && !achou; y++) if(tra(mulk(x,y,k),k)) achou = 1;
            if(!achou) ndeg = 0;      /* x seria ortogonal a tudo — colapso do dual */
        }
        /* os 2^k caracteres χ_x têm de ser todos diferentes entre si */
        long distintos = 0;
        for(unsigned x = 0; x < tot; x++){
            int novo = 1;
            for(unsigned z = 0; z < x && novo; z++){
                int igual = 1;
                for(unsigned y = 0; y < tot && igual; y++) if(chi(x,y,k) != chi(z,y,k)) igual = 0;
                if(igual) novo = 0;
            }
            if(novo) distintos++;
        }
        if(!val) mau_v++;
        if(!ndeg) mau_d++;
        if(distintos != tot) mau_c++;
        printf("      %d   %15s   %16s   %20ld   %3ld\n", k, val?"sim ✓":"NÃO",
               ndeg?"sim ✓":"NÃO", distintos, tot);
    }
    ok("o traço cai em {0,1} — é o caractere, não um número qualquer", mau_v == 0);
    ok("não-degenerado: ninguém é ortogonal a todo o mundo", mau_d == 0);
    ok("e os 2^k caracteres são TODOS distintos — o dual é o próprio corpo", mau_c == 0);
    printf("\n      É aqui que o corpo se olha ao espelho: o dual de (R^k,+) é indexado pelo\n");
    printf("      PRÓPRIO R^k, e quem faz a indexação é o traço — o traçado do hipociclo.\n");
}

/* ---------------------------------------------------------------- §P2 ------ */
printf("\n§P2  A DUALIDADE: a transformada leva CONVOLUÇÃO em PRODUTO ponto a ponto.\n\n");
{
    /* convolução usa a SOMA do corpo; o resultado, do outro lado do espelho, é o
     * PRODUTO. É literalmente "soma e multiplicação são duais" — e em inteiros. */
    int mau = 0;
    printf("      k   funções testadas   (f*g)^ = f̂·ĝ em todo ponto\n");
    for(int k = 1; k <= 5; k++){
        long tot = 1L << k;
        int casos = 0, bom = 1;
        for(int s = 0; s < 4; s++){
            long f[32], g[32], conv[32];
            /* funções deterministas, sem sorteio: o teste tem de repetir igual sempre */
            for(long y = 0; y < tot; y++){
                f[y] = ((y*7 + 3*s + 1) % 5) - 2;
                g[y] = ((y*11 + 5*s + 2) % 7) - 3;
            }
            for(long z = 0; z < tot; z++){
                conv[z] = 0;
                for(long y = 0; y < tot; y++) conv[z] += f[y] * g[z ^ y];   /* usa a SOMA */
            }
            for(long x = 0; x < tot; x++){
                long cf = 0, cg = 0, cc = 0;
                for(long y = 0; y < tot; y++){
                    int c = chi((unsigned)x, (unsigned)y, k);
                    cf += f[y]*c; cg += g[y]*c; cc += conv[y]*c;
                }
                if(cc != cf*cg) bom = 0;      /* o PRODUTO, do outro lado */
                casos++;
            }
        }
        if(!bom) mau++;
        printf("      %d   %16d   %s\n", k, casos, bom?"exato ✓":"FALHOU ✗");
    }
    ok("convolução de um lado É produto ponto a ponto do outro", mau == 0);
    printf("\n      Esta é a resposta. A soma e a multiplicação são duais NÃO como dois grupos\n");
    printf("      que se correspondem, mas como as DUAS FACES DA MESMA TRANSFORMADA: o que\n");
    printf("      de um lado se junta somando, do outro se junta multiplicando. Atravessar o\n");
    printf("      espelho troca uma operação pela outra, e nada mais muda.\n");
    printf("\n      E em característica 2 os caracteres valem ±1: a conta inteira é INTEIRA,\n");
    printf("      não entra um float. O espelho é exato porque não há o que arredondar.\n");
}

/* ---------------------------------------------------------------- §P3 ------ */
printf("\n§P3  E a multiplicação age no dual: multiplicar de um lado é multiplicar do outro.\n\n");
{
    int mau_a = 0, mau_p = 0;
    printf("      k   χ_(ax)(y) = χ_x(ay)   a≠0 permuta os caracteres\n");
    for(int k = 1; k <= 5; k++){
        long tot = 1L << k;
        int adj = 1, perm = 1;
        for(unsigned a = 0; a < tot; a++)
            for(unsigned x = 0; x < tot; x++)
                for(unsigned y = 0; y < tot; y++)
                    if(chi(mulk(a,x,k), y, k) != chi(x, mulk(a,y,k), k)) adj = 0;
        for(unsigned a = 1; a < tot; a++){
            static char vis[1 << 6];
            for(long t = 0; t < tot; t++) vis[t] = 0;
            for(unsigned x = 0; x < tot; x++){
                unsigned im = mulk(a, x, k);
                if(vis[im]) perm = 0;
                vis[im] = 1;
            }
        }
        if(!adj) mau_a++;
        if(!perm) mau_p++;
        printf("      %d   %19s   %25s\n", k, adj?"sim ✓":"NÃO", perm?"sim ✓":"NÃO");
    }
    ok("a multiplicação é autoadjunta pelo emparelhamento", mau_a == 0);
    ok("e todo a≠0 permuta o dual — a roda gira o espelho junto", mau_p == 0);
    printf("\n      A multiplicação não é uma estranha do outro lado: ela atravessa e continua\n");
    printf("      a ser ela. Por isso é o mesmo mecanismo dos dois lados do espelho.\n");
}

/* ---------------------------------------------------------------- §P4 ------ */
printf("\n§P4  O TRAÇADO é o adjunto da INCLUSÃO — é ele o mapa da dualidade.\n\n");
{
    int mau_a = 0, mau_t = 0;
    printf("      i   k   r   Tr_i(T(x)u) = Tr_k(xu)   Tr_k = Tr_i ∘ T\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        long tot = 1L << k;
        int adj = 1, trans = 1;
        for(unsigned x = 0; x < tot; x++){
            /* transitividade: o traço todo é o traço do traçado — o rolamento compõe */
            if(tra(x, k) != tra_mae(tracado(x, i, k), i, k)) trans = 0;
            for(unsigned u = 0; u < tot; u++){
                if(frob(u, i, k) != u) continue;   /* u percorre a MÃE, espalhada em R^k */
                /* adjunção: o traço da mãe do traçado, contra o traço do filho */
                if(tra_mae(mulk(tracado(x,i,k), u, k), i, k) != tra(mulk(x,u,k), k)) adj = 0;
            }
        }
        if(!adj) mau_a++;
        if(!trans) mau_t++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==4&&i==1))
            printf("      %d   %d   %d   %22s   %15s\n", i, k, k/i,
                   adj?"sim ✓":"NÃO", trans?"sim ✓":"NÃO");
    }
    ok("o traçado é ADJUNTO da inclusão pelo emparelhamento", mau_a == 0);
    ok("e o traço compõe: Tr_k = Tr_i ∘ T — o rolamento encaixa", mau_t == 0);
    printf("\n      É o fecho com o hipociclo. O pai ENTRA no filho pela inclusão e VOLTA pelo\n");
    printf("      traçado, e as duas passagens são uma o dual da outra. A curva que o ponto\n");
    printf("      desenha ao rolar não é ilustração: é o próprio mapa da dualidade.\n");
}

/* ---------------------------------------------------------------- §P5 ------ */
printf("\n§P5  O anulador do pai É o núcleo do traçado — e o dual do dual volta.\n\n");
{
    int mau_n = 0, mau_b = 0;
    printf("      i   k   |anulador de R^i|   |núcleo do traçado|   2^(k−i)   dual²=id\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        long tot = 1L << k, toti = 1L << i;
        long anul = 0, nucleo = 0;   /* u percorre a MÃE dentro de R^k, não os inteiros < 2^i */
        static char ea[1 << 6], en[1 << 6];
        for(long t = 0; t < tot; t++){ ea[t] = 0; en[t] = 0; }
        for(unsigned x = 0; x < tot; x++){
            int orto = 1;
            for(unsigned u = 0; u < tot; u++){
                if(frob(u, i, k) != u) continue;
                if(tra(mulk(x, u, k), k)) orto = 0;
            }
            if(orto){ ea[x] = 1; anul++; }
            if(tracado(x, i, k) == 0){ en[x] = 1; nucleo++; }
        }
        int mesmo = 1;
        for(unsigned x = 0; x < tot; x++) if(ea[x] != en[x]) mesmo = 0;
        /* dual do dual: o anulador do anulador é o pai de volta */
        long volta = 0; int igual_pai = 1;
        for(unsigned x = 0; x < tot; x++){
            int orto = 1;
            for(unsigned y = 0; y < tot; y++) if(ea[y] && tra(mulk(x,y,k),k)) orto = 0;
            if(orto){ volta++; if(frob(x, i, k) != x) igual_pai = 0; }
        }
        if(!mesmo || anul != (1L << (k-i))) mau_n++;
        if(!igual_pai || volta != toti) mau_b++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==6&&i==1))
            printf("      %d   %d   %18ld   %20ld   %8ld   %s\n", i, k, anul, nucleo,
                   1L << (k-i), (igual_pai&&volta==toti)?"sim ✓":"NÃO");
    }
    ok("o anulador do pai É o núcleo do traçado, e tem p^(k−i)", mau_n == 0);
    ok("e o dual do dual devolve o pai, exatamente", mau_b == 0);
    printf("\n      O p^(k−i) é o MESMO número que hipociclo.c §H4 mediu como fibra da curva:\n");
    printf("      quantos pontos do filho o traçado leva em cada ponto do pai. Um número, duas\n");
    printf("      leituras — de um lado é o borrão do desenho, do outro é o que o pai não vê.\n");
}

/* ---------------------------------------------------------------- §F1 ------ */
printf("\n§F1  E O FILHO É DUAL DA MÃE? Como grupo aditivo, NÃO.\n\n");
{
    int mau = 0;
    printf("      i   k   |dual da mãe| = 2^i   |filho| = 2^k   coincidem?\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        /* o dual de (R^i,+) tem exatamente tantos caracteres quantos elementos —
         * já medido em §P1. Logo tem 2^i, e o filho tem 2^k. */
        if(i != k && (1L<<i) == (1L<<k)) mau++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==6&&i==6))
            printf("      %d   %d   %19ld   %14ld   %s\n", i, k, 1L<<i, 1L<<k,
                   i==k ? "só se i=k" : "não");
    }
    ok("fora do caso trivial i=k, os tamanhos nem podem coincidir", mau == 0);
    printf("\n      A mãe é menor que o filho e o dual tem o tamanho do original. Então o\n");
    printf("      filho não é o dual da mãe — é maior demais para caber nesse papel.\n");
}

/* ---------------------------------------------------------------- §F2 ------ */
printf("\n§F2  Mas a mãe e o dual dela PREENCHEM o filho, exatamente.\n\n");
{
    int mau = 0;
    printf("      i   k   |mãe| = 2^i   |anulador| = 2^(k−i)   produto   |filho| = 2^k\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        long mae = 1L << i, anul = 1L << (k-i);
        if(mae * anul != (1L << k)) mau++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==6&&i==1))
            printf("      %d   %d   %12ld   %21ld   %8ld   %13ld\n", i, k, mae, anul,
                   mae*anul, 1L<<k);
    }
    ok("mãe × anulador = filho, sem sobra e sem falta", mau == 0);
    printf("\n      Esta é a relação verdadeira: o filho não é o dual da mãe, o filho é a MÃE\n");
    printf("      VEZES O DUAL DELA. A mãe entra inteira, e o que falta para completar o\n");
    printf("      filho é exatamente aquilo que a mãe não enxerga — o núcleo do traçado.\n");
    printf("\n      Por isso o filho voa e a mãe não some nele: ela é uma das duas metades, e\n");
    printf("      a outra metade é o seu próprio reflexo.\n");
}

/* ---------------------------------------------------------------- §F3 ------ */
printf("\n§F3  No retículo da torre, mãe e filho são duais SÓ quando i·k = N.\n\n");
{
    /* pontryagin.c mediu a dualidade do retículo: numa torre de grau N,
     * dual(R^a) = R^(N/a). Então perguntar se o filho é o dual da mãe é
     * perguntar se lcm(i,j)·i = N — e isso acontece às vezes, não sempre. */
    const int N = 12;
    int sim = 0, nao = 0;
    printf("      torre de grau N=%d      i   j   filho=lcm   dual da mãe=N/i   é dual?\n", N);
    for(int i = 1; i <= N; i++) for(int j = 1; j <= N; j++){
        if(N % i || N % j) continue;
        long a = i, b = j; while(b){ long t = a % b; a = b; b = t; }
        long lcm = (long)i / a * j;
        if(lcm > N) continue;
        int e = (lcm == N / i);
        if(e) sim++; else nao++;
        if((i==2&&j==6)||(i==3&&j==4)||(i==4&&j==3)||(i==2&&j==3)||(i==6&&j==2))
            printf("      %22s%d  %2d   %9ld   %15d   %s\n", "", i, j, lcm, N/i,
                   e ? "SIM ✓" : "não");
    }
    ok("a dualidade mãe↔filho acontece, mas é condição, não regra", sim > 0 && nao > 0);
    printf("      (%d pares em que o filho É o dual da mãe; %d em que não é.)\n", sim, nao);
    printf("\n      Ou seja: não há lei dizendo que o filho é o dual da mãe. Há uma CONDIÇÃO —\n");
    printf("      i·lcm(i,j) = N —, e quando ela vale, mãe e filho são um o espelho do outro\n");
    printf("      dentro da torre. Fora dela, são só mãe e filho.\n");
}

/* ---------------------------------------------------------------- §F4 ------ */
printf("\n§F4  O que É autodual: o grupo do filho sobre a mãe. E ele são as PONTAS.\n\n");
{
    int mau_o = 0, mau_f = 0;
    printf("      i   k   r=k/i   ordem de φ^i em R^k   fixos de φ^i   2^i   pontas\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        int r = k / i;
        long tot = 1L << k;
        int ordem = 0;
        for(int t = 1; t <= r && !ordem; t++){
            int todos = 1;
            for(unsigned x = 0; x < tot; x++) if(frob(x, i*t, k) != x){ todos = 0; break; }
            if(todos) ordem = t;
        }
        long fixos = 0;
        for(unsigned x = 0; x < tot; x++) if(frob(x, i, k) == x) fixos++;
        if(ordem != r) mau_o++;
        if(fixos != (1L << i)) mau_f++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==6&&i==1))
            printf("      %d   %d   %5d   %19d   %13ld   %3ld   %6d\n", i, k, r, ordem,
                   fixos, 1L<<i, r);
    }
    ok("o grupo do filho sobre a mãe é cíclico de ordem r = k/i", mau_o == 0);
    ok("e o que ele fixa é exatamente a mãe — nada mais", mau_f == 0);
    printf("\n      Um grupo cíclico de ordem r tem exatamente r caracteres: é AUTODUAL. E r é\n");
    printf("      o número de pontas do hipociclo do §H4. Então o que se espelha em si mesmo\n");
    printf("      não é a mãe nem o filho — é a PASSAGEM de uma para o outro.\n");
    printf("\n      E aí La Hire outra vez: r=2 é o grupo de duas pontas, a involução, a reta.\n");
    printf("      O único caso em que a passagem é o seu próprio inverso.\n");
}

printf("\n=== A RESPOSTA ============================================================\n");
printf("  NÃO como grupos: (R^k,+) tem 2^k e (R^k*,×) tem 2^k−1. Ordens diferentes,\n");
printf("  logo um não é o dual do outro, e afirmá-lo seria inventar.\n\n");
printf("  SIM pela transformada, que é mais forte: a convolução — feita com a SOMA —\n");
printf("  vira o PRODUTO ponto a ponto do outro lado do espelho. As duas operações são\n");
printf("  as duas faces do mesmo mapa: atravessar troca uma pela outra, e nada mais muda.\n\n");
printf("  E o mapa É O TRAÇADO. O emparelhamento é o traço; a inclusão do pai no filho e\n");
printf("  o traçado do filho no pai são adjuntos; o anulador do pai é o núcleo do traçado,\n");
printf("  com a mesma fibra p^(k−i) que o hipociclo já tinha medido. A curva que o ponto\n");
printf("  desenha ao rolar não é figura de linguagem: é o mapa de Pontryagin do corpo.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
