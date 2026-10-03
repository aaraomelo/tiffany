/* metalica.c — A FAMILIA METALICA SEM OS REAIS.
 *
 * O Aarao: "sai do corpo primal dual e involucao, tira R da teoria e desce mais,
 *           apresenta familia metalica algebrica"
 *
 * O que estava escrito ate' agora apresentava sigma_n como um NUMERO REAL,
 * (n + sqrt(n^2+4))/2, e depois provava coisas sobre ele. Isso e' realizacao: precisa
 * do contínuo para ser dito, e por isso desceu para o Catalogo com Mobius e as fraccoes
 * continuas. Mas a familia metalica NAO PRECISA DE R PARA EXISTIR — ela e' o anel
 *
 *     A_n = Z[x]/(x^2 - n·x - 1),   sigma = classe de x,
 *
 * e tudo o que a teoria diz dela le-se lá dentro, em INTEIROS. Nenhuma raiz quadrada,
 * nenhum limite, nenhuma ordem do contínuo. E' isso que este medidor mostra.
 *
 * O ponto central, e e' a Lei 2 escrita sem disfarce:
 *
 *     §M3   sigma^-1 = -sigma'      <=>     N(x) = -1
 *
 * "a inversa e' menos a conjugada". Do lado esquerdo esta' a Lei 2 (f^-1 = -f, a
 * dualidade e' dual); do lado direito esta' a norma, que e' um INTEIRO. A lei nao e'
 * uma propriedade de um numero real particular: e' a equacao que define quais elementos
 * do anel sao unidades de norma -1 — e essa pergunta e' algebrica.
 *
 *   §M1  N(sigma) = -1 para todo n, e sai da reducao, nao de uma raiz
 *   §M2  sigma^-1 = sigma - n e' INTEIRA: sigma e' unidade de A_n, para todo n
 *   §M3  A LEI 2: x^-1 = -x' vale exatamente nos x de norma -1 (varridos, contados)
 *   §M4  a conjugacao e' involucao E homomorfismo — e' o J do corpo dual
 *   §M5  a familia de potencia e' inteira: sigma^k = F_{k-1} + F_k·sigma
 *   §M6  o traco fecha em Z e obedece a recorrencia de Lucas
 *   §M7  N(sigma^k) = (-1)^k — a alternancia preto-e-branco da torre, em Z
 *   §M8  a forma traco E' o emparelhamento dual, e o seu determinante e' Delta = n^2+4
 *   §M9  n = 0 e' a involucao, e e' o unico n em que o anel DECOMPOE
 *   §M10 o que R acrescenta, e o que ele NAO acrescenta — a fronteira, medida
 *   §M13 o CRITERIO: f_n tem raiz em K  <=>  Delta_n e' quadrado em K (sem R, sem Z)
 *   §M14 a TRICOTOMIA: corpo / K x K / nao reduzido, decidida por Delta — com
 *        idempotente e nilpotente EXIBIDOS, nao afirmados
 *   §M15 a EXCEPCAO de caracteristica 2, registada a parte: la o Delta nao decide nada
 *
 * ZERO doubles neste ficheiro. Nem um. Se aparecer um, a tese caiu.
 *
 *   cc -O2 -std=c99 -Wall metalica.c -o metalica && ./metalica
 */
#include <stdio.h>
#include <stdlib.h>
#include "../lib/unidade.h"

typedef long L;

/* ─── o anel A_n = Z[sigma], sigma^2 = n·sigma + 1 ────────────────────────────────────
 * Um elemento e' p + q·sigma com p,q em Z. A reducao usa a borda e mais nada; nao ha'
 * onde um real entrar. NOTA: sigma' satisfaz a MESMA borda, logo a mesma reducao serve
 * para os dois lados do par — e e' por isso que o anel ja' contem o seu dual. */
typedef struct { L p, q; } Zs;

static Zs zs(L p, L q){ Zs z = {p, q}; return z; }
static int eq(Zs a, Zs b){ return a.p == b.p && a.q == b.q; }

static Zs mul(Zs a, Zs b, L n){
    /* (p1 + q1 s)(p2 + q2 s) = p1p2 + (p1q2 + q1p2) s + q1q2 s^2,  s^2 = n s + 1 */
    return zs(a.p*b.p + a.q*b.q,
              a.p*b.q + a.q*b.p + n*a.q*b.q);
}
static Zs add(Zs a, Zs b){ return zs(a.p + b.p, a.q + b.q); }
static Zs neg(Zs a){ return zs(-a.p, -a.q); }

/* a conjugacao: sigma -> n - sigma. E' o unico automorfismo nao trivial de A_n,
 * e e' ele o J do corpo dual — nao um J escolhido por mim. */
static Zs estaca(Zs a, L n){ return zs(a.p + n*a.q, -a.q); }

/* a norma e o traco: os dois invariantes do par, ambos em Z */
static L norma(Zs a, L n){ Zs r = mul(a, estaca(a, n), n); return r.q == 0 ? r.p : (L)0x7fffffff; }
static L traco(Zs a, L n){ Zs r = add(a, estaca(a, n)); return r.q == 0 ? r.p : (L)0x7fffffff; }

/* ─── §M13–§M15: a aritmética mod p, para decidir o discriminante sem reais ────────────
 * A irredutibilidade de f_n = x^2 - n x - 1 decide-se por BUSCA: conta-se quantos r em
 * F_p satisfazem f_n(r) = 0. O "Delta_n e' quadrado?" decide-se do mesmo modo, por busca
 * em F_p. Nenhum real, nenhuma raiz quadrada de ponto flutuante — o espirito de §M9.
 *
 * Os elementos de F_p[x]/(x^2 - n x - 1) sao pares (c0, c1) = c0 + c1·s, e a multiplicacao
 * e' a de §M1–§M8 com s^2 = n s + 1 — a MESMA borda, so' que agora reduzida modulo p.
 * E' por isso que o idempotente e o nilpotente de §M14 se medem: sao elementos do anel. */
typedef struct { int c0, c1; } Fp;

static int imod(long long a, int p){ a %= p; if(a < 0) a += p; return (int)a; }

static Fp fp_mul(Fp a, Fp b, int n, int p){
    long long t0 = (long long)a.c0*b.c0 + (long long)a.c1*b.c1;
    long long t1 = (long long)a.c0*b.c1 + (long long)a.c1*b.c0
                 + (long long)n*a.c1*b.c1;
    return (Fp){ imod(t0, p), imod(t1, p) };
}
static Fp fp_add(Fp a, Fp b, int p){
    return (Fp){ imod((long long)a.c0 + b.c0, p), imod((long long)a.c1 + b.c1, p) };
}
static Fp fp_sub(Fp a, Fp b, int p){
    return (Fp){ imod((long long)a.c0 - b.c0, p), imod((long long)a.c1 - b.c1, p) };
}
static Fp fp_const(int c, int p){ return (Fp){ imod(c, p), 0 }; }
static int  fp_zero(Fp a){ return a.c0 == 0 && a.c1 == 0; }
static int  fp_eq(Fp a, Fp b){ return a.c0 == b.c0 && a.c1 == b.c1; }

/* avaliar c0 + c1·s numa raiz r de F_p: e' um polinomio de grau 1, logo so' uma mult. */
static int fp_em(Fp a, int r, int p){ return imod((long long)a.c0 + (long long)a.c1*r, p); }

/* quantas raizes f_n tem em F_p, e onde */
static int fp_raizes(int n, int p, int *onde){
    int k = 0;
    for(int r = 0; r < p; r++)
        if(imod((long long)r*r - (long long)n*r - 1, p) == 0){ if(onde) onde[k] = r; k++; }
    return k;
}

/* Delta_n = n^2 + 4 em F_p: 0 = quadrado NULO, 1 = quadrado nao nulo, 2 = nao quadrado */
static int fp_classe(int n, int p, int *s){
    int d = imod((long long)n*n + 4, p);
    if(d == 0) return 0;
    for(int u = 1; u < p; u++)
        if((long long)u*u % p == d){ if(s) *s = u; return 1; }
    return 2;
}

/* inverso modulo p, por busca (p e' sempre pequeno aqui) */
static int fp_inv(int a, int p){
    for(int u = 1; u < p; u++) if((long long)a*u % p == 1) return u;
    return 0;
}

/* F_4 = F_2[t]/(t^2 + t + 1): o elemento i = c0 + c1·t e' guardado como c0 | (c1 << 1),
 * e t^2 = t + 1. Caracteristica 2, logo a soma e' XOR. Existe para §M15: e' o unico
 * corpo de caracteristica 2 com mais de dois elementos que a excepcao exige. */
static int f4_mul(int a, int b){
    int a0 = a & 1, a1 = (a >> 1) & 1, b0 = b & 1, b1 = (b >> 1) & 1;
    int c0 = (a0*b0 + a1*b1) & 1;                      /* o termo t^2 vira t + 1 */
    int c1 = (a0*b1 + a1*b0 + a1*b1) & 1;
    return c0 | (c1 << 1);
}
static int f4_add(int a, int b){ return a ^ b; }
static int f4_inv(int a){
    if(a == 0) return 0;
    for(int u = 1; u < 4; u++) if(f4_mul(a, u) == 1) return u;
    return 0;
}

int main(void){
    puts("\n  A FAMILIA METALICA, ALGEBRICA — sem um unico real\n");

    /* ═══ §M1 — a norma de sigma e' -1, e sai da REDUCAO ═══════════════════════════════
     * Nao se calcula (n+sqrt(n^2+4))/2 e depois se multiplica. Multiplica-se sigma pela
     * sua conjugada dentro do anel, e o que sai e' o inteiro -1. */
    int mau = 0, vistos = 0;
    for(L n = 0; n <= 200; n++){
        if(norma(zs(0,1), n) != -1) mau++;
        vistos++;
    }
    ok("N(sigma) = -1 para todo n em [0,200], obtido pela reducao e nao por uma raiz", !mau && vistos == 201);

    /* e o traco e' n: o parametro da familia E' o traco, lido no anel */
    mau = 0;
    for(L n = 0; n <= 200; n++) if(traco(zs(0,1), n) != n) mau++;
    ok("tr(sigma) = n: o indice da familia metalica E' o traco, e le-se dentro do anel", !mau);

    /* ═══ §M2 — sigma e' unidade, e a inversa e' INTEIRA ═══════════════════════════════ */
    mau = 0;
    for(L n = 0; n <= 200; n++){
        Zs inv = zs(-n, 1);                       /* sigma - n, candidata */
        if(!eq(mul(zs(0,1), inv, n), zs(1,0))) mau++;   /* verificado por MULTIPLICACAO */
    }
    ok("sigma·(sigma - n) = 1 em Z[sigma] para todo n: a inversa e' INTEIRA, sigma e' unidade", !mau);

    /* ═══ §M3 — A LEI 2, e ela caracteriza exatamente a norma -1 ═══════════════════════
     * x^-1 = -x' quer dizer x·(-x') = 1, isto e' -N(x) = 1, isto e' N(x) = -1.
     * Varro todo o anel numa janela e conto: a lei vale EXATAMENTE nos de norma -1.
     * Se houvesse um x com N(x) != -1 a cumprir a lei, ou um de norma -1 a falha-la,
     * esta assercao caia. */
    long lei = 0, nm1 = 0, discord = 0, total = 0;
    for(L n = 0; n <= 12; n++)
      for(L p = -14; p <= 14; p++)
        for(L q = -14; q <= 14; q++){
            Zs x = zs(p,q);
            if(p == 0 && q == 0) continue;
            total++;
            int cumpre = eq(mul(x, neg(estaca(x,n)), n), zs(1,0));   /* x·(-x') = 1 ? */
            int norm   = (norma(x,n) == -1);
            if(cumpre) lei++;
            if(norm)   nm1++;
            if(cumpre != norm) discord++;
        }
    printf("      varridos %ld elementos: %ld cumprem a Lei 2, %ld tem norma -1\n", total, lei, nm1);
    ok("a Lei 2 (x^-1 = -x') vale EXATAMENTE nos elementos de norma -1 — zero discordancias",
       discord == 0 && lei > 0 && lei == nm1);

    /* e nem todo elemento a cumpre: se cumprissem todos, a lei nao diria nada */
    ok("e ela SEPARA: a esmagadora maioria dos elementos nao a cumpre, logo a lei tem conteudo",
       lei < total / 10);

    /* ═══ §M4 — a conjugacao e' o J: involucao e homomorfismo ══════════════════════════ */
    int nao_inv = 0, nao_hom_add = 0, nao_hom_mul = 0;
    for(L n = 0; n <= 10; n++)
      for(L p = -8; p <= 8; p++)
        for(L q = -8; q <= 8; q++){
            Zs x = zs(p,q);
            if(!eq(estaca(estaca(x,n),n), x)) nao_inv++;
            for(L r = -4; r <= 4; r++) for(L s = -4; s <= 4; s++){
                Zs y = zs(r,s);
                if(!eq(estaca(add(x,y),n), add(estaca(x,n),estaca(y,n)))) nao_hom_add++;
                if(!eq(estaca(mul(x,y,n),n), mul(estaca(x,n),estaca(y,n),n))) nao_hom_mul++;
            }
        }
    ok("a conjugacao e' involucao (J·J = id) em toda a janela — resíduo exatamente zero", !nao_inv);
    ok("e e' homomorfismo nas DUAS operacoes: preserva a escrita (+) e a leitura (·)",
       !nao_hom_add && !nao_hom_mul);

    /* ═══ §M5 — a familia de potencia, inteira ═════════════════════════════════════════
     * sigma^k = F_{k-1} + F_k·sigma com F o Fibonacci de ordem n. Os coeficientes sao
     * gerados pela recorrencia e comparados com a potencia calculada no anel: dois
     * caminhos independentes que tem de concordar. */
    mau = 0; int casos = 0;
    for(L n = 1; n <= 6; n++){
        Zs pot = zs(1,0);
        L Fm = 0, F = 1;                         /* F_0 = 0, F_1 = 1 */
        for(int k = 1; k <= 22; k++){
            pot = mul(pot, zs(0,1), n);          /* caminho A: potencia no anel */
            if(!eq(pot, zs(Fm, F))) mau++;       /* caminho B: a recorrencia */
            casos++;
            L nf = n*F + Fm; Fm = F; F = nf;
        }
    }
    printf("      %d potencias verificadas por dois caminhos independentes\n", casos);
    ok("sigma^k = F_{k-1} + F_k·sigma com F o Fibonacci de ordem n — os dois caminhos concordam",
       !mau && casos == 132);

    /* ═══ §M6 — o traco fecha em Z e e' Lucas ══════════════════════════════════════════ */
    mau = 0; casos = 0;
    for(L n = 1; n <= 8; n++){
        Zs pot = zs(1,0);
        L t[26];
        for(int k = 0; k <= 24; k++){
            t[k] = traco(pot, n);
            if(t[k] == 0x7fffffff) mau++;        /* saiu de Z: falharia aqui */
            pot = mul(pot, zs(0,1), n);
        }
        for(int k = 1; k <= 23; k++){
            if(t[k+1] != n*t[k] + t[k-1]) mau++; /* a recorrencia de Lucas */
            casos++;
        }
    }
    ok("o traco de sigma^k fica em Z e obedece a t_{k+1} = n·t_k + t_{k-1} — Lucas, sem R",
       !mau && casos == 184);

    /* ═══ §M7 — a alternancia da torre, em Z ═══════════════════════════════════════════ */
    mau = 0;
    for(L n = 0; n <= 30; n++){
        Zs pot = zs(1,0);
        for(int k = 0; k <= 14; k++){
            L esperado = (k % 2 == 0) ? 1 : -1;  /* (-1)^k, derivado da paridade */
            if(norma(pot, n) != esperado) mau++;
            pot = mul(pot, zs(0,1), n);
        }
    }
    ok("N(sigma^k) = (-1)^k: a alternancia preto-e-branco da torre e' multiplicatividade da norma", !mau);

    /* ═══ §M8 — a forma traco E' o emparelhamento dual ═════════════════════════════════
     * <x,y> = tr(x·y) e' uma forma bilinear A x A -> Z. Na base {1, sigma} a sua matriz
     * de Gram e' [[2, n],[n, n^2+2]], e o determinante e' n^2 + 4 = Delta. Ou seja: o
     * DISCRIMINANTE E' O DETERMINANTE DO EMPARELHAMENTO. E' isto que faz de A_n um corpo
     * dual sobre Z — a leitura existe, e' nao-degenerada, e nao pediu R a ninguem. */
    mau = 0;
    for(L n = 0; n <= 100; n++){
        L g00 = traco(mul(zs(1,0), zs(1,0), n), n);   /* <1,1>       */
        L g01 = traco(mul(zs(1,0), zs(0,1), n), n);   /* <1,sigma>   */
        L g11 = traco(mul(zs(0,1), zs(0,1), n), n);   /* <s,s>       */
        L det = g00*g11 - g01*g01;
        if(g00 != 2 || g01 != n || g11 != n*n + 2) mau++;
        if(det != n*n + 4) mau++;                     /* det = Delta */
        if(det == 0) mau++;                           /* nunca degenera */
    }
    ok("a forma traco tem Gram [[2,n],[n,n^2+2]] e det = n^2+4 = Delta: o DISCRIMINANTE",
       !mau);
    ok("e ela nunca degenera (Delta >= 4 > 0): a leitura existe em Z, sem pedir R", !mau);

    /* ═══ §M9 — n = 0 e' a involucao, e e' o unico n onde o anel DECOMPOE ══════════════
     * Delta = n^2+4 e' quadrado perfeito so' em n = 0 (Delta = 4). Procurado por busca
     * inteira, nao afirmado. E em n = 0 tem-se sigma^2 = 1: e' a involucao pura.
     *
     * NOTA DE ARITMETICA (transbordo corrigido). Esta busca usava `long`, que no Windows
     * tem 32 bits — o modelo LLP64 diz int=32, long=32, long long=64, e `long` NAO e' o
     * inteiro longo de que se fala. Acima de n = 46340 o produto n*n transborda e a
     * busca passa a ver numeros que nao sao n^2+4: o teste acusava 6 casos, sendo 5
     * falsos — o primeiro em n = 65536, onde n^2 + 4 ≡ 4 (mod 2^32) e o falso quadrado
     * perfeito e' exatamente o 4 do n = 0, e mais quatro em n = 65664, 67616, 71464,
     * 72344. A MATEMATICA estava certa (a unica solucao de (k-n)(k+n) = 4 e' n = 0); o
     * instrumento e' que estava errado. Num transbordo silencioso isto e' pior que a
     * asercao que denuncia, porque e' o unico lugar do ficheiro que diz ONDE o anel
     * decompoe. Por isso `long long` explicito, e por isso a janela vai nos dois
     * sentidos: Delta so' depende de |n|, e |n| = 0 e' o unico. */
    long long quadrados = 0; long long onde = -1;
    for(long long m = 0; m <= 100000; m++){
        long long d = m*m + 4, r = 0;
        while(r*r < d) r++;                      /* raiz inteira por busca, sem sqrt */
        if(r*r == d){ quadrados++; if(onde < 0) onde = m; }
    }
    printf("      Delta = n^2+4 e' quadrado perfeito em %lld dos 100001 casos (|n| = %lld)\n",
           quadrados, onde);
    ok("|n| = 0 e' o UNICO |n| em [0,100000] com Delta quadrado: o unico onde o anel decompoe",
       quadrados == 1 && onde == 0);

    ok("e nesse n a borda da' sigma^2 = 1: o nivel 0 da escada E' a involucao pura",
       eq(mul(zs(0,1), zs(0,1), 0), zs(1,0)));

    /* e a Lei 1 la' dentro: em n = 0 a conjugacao e' x -> -x sobre a parte sigma,
     * e sigma' = -sigma. "a unidade e' dual", literal. */
    ok("e em n = 0 vale sigma' = -sigma: a Lei 1 (1 ~ -1) le-se no proprio anel",
       eq(estaca(zs(0,1), 0), neg(zs(0,1))));

    /* mas em n != 0 NAO vale — se valesse sempre, a Lei 1 nao distinguiria nada */
    int falha_fora = 0;
    for(L n = 1; n <= 50; n++) if(eq(estaca(zs(0,1), n), neg(zs(0,1)))) falha_fora++;
    ok("e so' em n = 0: para n em [1,50] sigma' != -sigma, logo a Lei 1 e' o degrau zero e nao um adorno",
       falha_fora == 0);

    /* ═══ §M10 — a fronteira: o que R acrescenta e o que nao ═══════════════════════════
     * Esta e' a razao de existir deste ficheiro. A pergunta "sigma e' unidade?" e' de
     * norma, logo algebrica, logo fica na Teoria. A pergunta "o conjugado e' pequeno?"
     * (Pisot) precisa de comparar tamanhos, logo e' metrica, logo desce ao Catalogo.
     * Os dois criterios nao coincidem — e e' isso que justifica a separacao. */
    long unidades = 0, pisot_alg = 0, so_pisot = 0, so_unid = 0;
    for(L A = 1; A <= 40; A++)
      for(L B = -40; B <= 40; B++){
          /* x^2 - A x + B: unidade <=> |B| = 1 (criterio algebrico, so' o termo constante)
           * Pisot em grau 2 <=> -A-1 < B < A-1 (criterio de inteiros ja' medido em escada.c) */
          int u = (B == 1 || B == -1);
          int p = (-A-1 < B && B < A-1) && (A*A - 4*B > 0);
          if(u) unidades++;
          if(p) pisot_alg++;
          if(u && p) continue;
          if(p && !u) so_pisot++;
          if(u && !p) so_unid++;
      }
    printf("      grau 2, A em [1,40], B em [-40,40]: %ld unidades, %ld Pisot;"
           " %ld Pisot nao-unidade, %ld unidade nao-Pisot\n",
           unidades, pisot_alg, so_pisot, so_unid);
    ok("unidade e Pisot NAO sao o mesmo criterio: ha' Pisot que nao e' unidade",
       so_pisot > 0);
    ok("e ha' unidade que nao e' Pisot — logo nenhum dos dois implica o outro em geral",
       so_unid > 0);

    /* e a familia metalica esta' na interseccao: e' o que justifica o teorema */
    int fora = 0;
    for(L n = 1; n <= 40; n++){
        /* sigma_n: x^2 - n x - 1, isto e' A = n, B = -1 */
        int u = 1;                                    /* B = -1, unidade sempre */
        int p = (-n-1 < -1 && -1 < n-1) && (n*n + 4 > 0);
        if(!(u && p)) fora++;
    }
    ok("toda a familia metalica esta' na interseccao dos dois criterios, sem excecao em [1,40]",
       !fora);

    /* ═══ §M11 — AS DUAS NOTACOES: a estaca e a cruz ═══════════════════════════════════
     * O Aarao: "cruz e estaca definem os dois, e com isso primal e dual sai representacao
     * para tudo". Sao estas:
     *
     *     x^t        ESTACA  — o dual do elemento (aqui: a conjugacao)
     *     x^x        CRUZ    — o par (x + x^t, x · x^t) = (traco, norma)
     *
     * A estaca troca os lados; a cruz projeta-os no que fica FIXO. E as potencias das
     * duas dizem coisas diferentes: a estaca tem periodo 2 (bidualidade), a cruz e'
     * idempotente (ja' esta' no fixo, cruzar outra vez nao move). Nenhuma das duas
     * precisou de R para ser definida, e as duas juntas dao os invariantes todos. */
    int per2 = 0, per1 = 0;
    for(L n = 0; n <= 20; n++)
      for(L p = -10; p <= 10; p++)
        for(L q = -10; q <= 10; q++){
            Zs x = zs(p,q);
            if(!eq(estaca(estaca(x,n),n), x)) per2++;             /* t^2 = id */
            if(q != 0 && eq(estaca(x,n), x))  per1++;             /* t = id ? so' no fixo */
        }
    ok("a ESTACA tem periodo exatamente 2: x^tt = x sempre, e x^t != x fora do subanel fixo",
       per2 == 0 && per1 == 0);

    /* a cruz cai no subanel fixo pela estaca — e por isso cruzar duas vezes nao move */
    int fora_fixo = 0, nao_idem = 0;
    for(L n = 0; n <= 20; n++)
      for(L p = -10; p <= 10; p++)
        for(L q = -10; q <= 10; q++){
            Zs x = zs(p,q);
            Zs t = add(x, estaca(x,n));           /* a soma da cruz  */
            Zs v = mul(x, estaca(x,n), n);        /* o produto da cruz */
            if(t.q != 0 || v.q != 0) fora_fixo++;              /* ambos no fixo */
            if(!eq(estaca(t,n), t) || !eq(estaca(v,n), v)) nao_idem++;  /* fixos pela estaca */
        }
    ok("a CRUZ (x+x^t, x·x^t) cai sempre no subanel fixo pela estaca — as duas coordenadas",
       fora_fixo == 0);
    ok("e o fixo E' fixo: cruzar de novo nao move nada, a cruz e' idempotente", nao_idem == 0);

    /* e a cruz SEPARA: elementos distintos podem ter a mesma cruz — e' o preco da projecao,
     * e e' exatamente por isso que o PAR e' preciso e uma coordenada so' nao chega */
    long mesmo_traco = 0, mesma_cruz = 0, pares = 0;
    for(L p = -6; p <= 6; p++) for(L q = -6; q <= 6; q++)
      for(L r = -6; r <= 6; r++) for(L s = -6; s <= 6; s++){
          Zs x = zs(p,q), y = zs(r,s);
          if(eq(x,y)) continue;
          pares++;
          if(traco(x,3) == traco(y,3)) mesmo_traco++;
          if(traco(x,3) == traco(y,3) && norma(x,3) == norma(y,3)) mesma_cruz++;
      }
    printf("      n=3: %ld pares distintos; %ld colidem no traco, %ld colidem na CRUZ inteira\n",
           pares, mesmo_traco, mesma_cruz);
    ok("o traco sozinho colide muito mais que a cruz: uma coordenada nao chega, o PAR chega mais",
       mesma_cruz < mesmo_traco / 4);

    /* ═══ §M12 — a ordem sai dos duais, nao de R ═══════════════════════════════════════
     * O Aarao: "a teoria nao precisa de R, ja' tem limite completude ordenacao, sai dos
     * duais". Aqui esta' a ordem, construida so' com a cruz: dois elementos comparam-se
     * pelo par (traco, norma) lexicograficamente, e isso e' uma ordem TOTAL em Z x Z.
     * Nenhum corte de Dedekind, nenhuma sucessao de Cauchy. */
    int nao_total = 0, nao_trans = 0;
    Zs am[9]; int na = 0;
    for(L p = -1; p <= 1; p++) for(L q = -1; q <= 1; q++) am[na++] = zs(p,q);
    for(int i = 0; i < na; i++) for(int j = 0; j < na; j++){
        L ti = traco(am[i],3), tj = traco(am[j],3);
        L ni = norma(am[i],3), nj = norma(am[j],3);
        int menor = (ti < tj) || (ti == tj && ni < nj);
        int maior = (tj < ti) || (tj == ti && nj < ni);
        int igual = (ti == tj && ni == nj);
        if(menor + maior + igual != 1) nao_total++;      /* tricotomia */
        for(int k = 0; k < na; k++){
            L tk = traco(am[k],3), nk = norma(am[k],3);
            int mij = (ti<tj)||(ti==tj&&ni<nj), mjk = (tj<tk)||(tj==tk&&nj<nk);
            int mik = (ti<tk)||(ti==tk&&ni<nk);
            if(mij && mjk && !mik) nao_trans++;          /* transitividade */
        }
    }
    ok("a cruz ordena: (traco, norma) lexicografico e' total e transitiva — a ordem sai do DUAL",
       nao_total == 0 && nao_trans == 0);

    /* ═══ §M13 — O CRITERIO: a irredutibilidade decide-se pelo discriminante, sem R ══════
     *
     * §M9 mediu QUE n = 0 e' o unico onde o anel decompoe, por busca em Z. O que faltava
     * era o CRITERIO, e o que o torna honesto e' que ele nao esta escrito em N nem em Z:
     *
     *     f_n = x^2 - n x - 1 tem raiz em K   <=>   Delta_n = n^2 + 4 e' quadrado em K
     *
     * A ponte e' (2 r - n)^2 = n^2 + 4 quando r e' raiz de f_n, e r = (n ± s)/2 quando
     * s^2 = n^2 + 4. O 2 e' o unico ponto onde a caracteristica entra — e e' por isso que
     * §M15 existe e nao e' uma cautela de estilo.
     *
     * Aqui a equivalencia e' MEDIDA em corpos finitos, onde "quadrado" se decide por busca:
     * os dois lados sao calculados por metodos INDEPENDENTES (contar raizes de f_n, por um
     * lado; classificar Delta, pelo outro) e comparados. Um so' mediria o que ja sabe. */
    const int primos[] = {3,5,7,11,13,17,19,23,29,31,37,41,101,1009};
    int nprimos = (int)(sizeof(primos)/sizeof(primos[0]));
    int medidos = 0, disc_c2 = 0, quantos_corpo = 0;
    for(int i = 0; i < nprimos; i++){
        int p = primos[i];
        for(int n = 0; n < p; n++){
            int tem_raiz     = (fp_raizes(n, p, NULL) != 0);
            int eh_quadrado  = (fp_classe(n, p, NULL) != 2);
            if(tem_raiz != eh_quadrado) disc_c2++;
            if(!tem_raiz) quantos_corpo++;
            medidos++;
        }
    }
    printf("      %d pares (p, n) em %d primos: %d com f_n irredutivel, %d discordancias\n",
           medidos, nprimos, quantos_corpo, disc_c2);
    ok("em F_p, nos 14 primos medidos: f_n tem raiz <=> Delta_n e' quadrado — dois caminhos, zero discordancias",
       disc_c2 == 0 && medidos == 1346 && quantos_corpo > 0);

    /* e a identidade que DA a ponte, verificada no anel e nao na memoria:
     *     (2·s - n)^2 = n^2 + 4 = Delta_n.
     * E' ela que separa a caracteristica 2 das outras, e e' o que torna a u de §M14 uma
     * involucao. Primeiro em Z, depois em F_p. */
    int id_zeta = 0;
    for(L n = -40; n <= 40; n++){
        Zs u = zs(-n, 2);                            /* 2·sigma - n */
        Zs q = mul(u, u, n);
        if(!eq(q, zs(n*n + 4, 0))) id_zeta++;         /* n^2 + 4, lido em Z */
    }
    ok("a identidade (2·sigma - n)^2 = Delta_n = n^2 + 4 vale em Z[sigma] para todo n em [-40,40]",
       id_zeta == 0);

    int id_fp = 0;
    for(int i = 0; i < nprimos; i++){
        int p = primos[i];
        for(int n = 0; n < p; n++){
            Fp u = (Fp){ imod(-n, p), imod(2, p) };
            if(!fp_eq(fp_mul(u, u, n, p), (Fp){ imod((long long)n*n + 4, p), 0 })) id_fp++;
        }
    }
    ok("e a mesma identidade vale em F_p[x]/(x^2 - n x - 1) em todos os (p, n) medidos",
       id_fp == 0);

    /* ═══ §M14 — A TRICOTOMIA: o mesmo Delta decide tres objectos de especies diferentes ═
     *
     * §M13 disse QUANDO f_n tem raiz. O que o quociente E' ainda nao foi dito, e e' a
     * consequencia que interessa: o Delta nao e' so' o determinante da leitura (§M8), e' o
     * QUE DECIDE o anel — e decide objectos de especies diferentes,
     *
     *     Delta_n NAO quadrado  ->  corpo         (extensao quadratica de K)
     *     Delta_n = s^2 != 0    ->  K x K         (reduzido, com divisores de zero)
     *     Delta_n = 0           ->  nao reduzido  (com o nilpotente sigma - n/2)
     *
     * Primeiro contam-se os tres regimes e compara-se com as formulas fechadas — a
     * contagem por pares (n, s) com s^2 = n^2 + 4, que em F_p e' p - 1. Depois EXIBEM-SE
     * as testemunhas: o par de idempotentes complementares no regime K x K, e o nilpotente
     * no regime nao reduzido.Afirmar que existem e' barato; exibi-los e' o que fecha. */
    int conta_certa = 0, soma_corpo = 0, soma_prod = 0, soma_nred = 0;
    for(int i = 0; i < nprimos; i++){
        int p = primos[i], c = 0, pr = 0, nr = 0;
        for(int n = 0; n < p; n++){
            int cl = fp_classe(n, p, NULL);
            if(cl == 2) c++;
            else if(cl == 1) pr++;
            else nr++;
        }
        /* formulas fechadas: (p+1)/2 corpos se p = 3 (mod 4), (p-1)/2 se p = 1 (mod 4);
         * Delta = 0 em 2 valores se p = 1 (mod 4) (as raizes de -1, multiplicadas por 2). */
        int nr_esperado  = (p % 4 == 1) ? 2 : 0;
        int c_esperado   = (p % 4 == 1) ? (p - 1)/2 : (p + 1)/2;
        if(c == c_esperado && nr == nr_esperado && pr == p - c - nr) conta_certa++;
        soma_corpo += c; soma_prod += pr; soma_nred += nr;
    }
    printf("      %d primos: %d n com f_n irredutivel (corpo), %d com f_n a partir em\n"
             "      factores distintos (K x K), %d com Delta = 0 (nao reduzido)\n",
           nprimos, soma_corpo, soma_prod, soma_nred);
    ok("os tres regimes particionam F_p e as contagens fecham: (p+1)/2 corpos se p = 3 (mod 4),\n"
       "      (p-1)/2 se p = 1 (mod 4), e Delta = 0 em 2 valores so' quando p = 1 (mod 4)",
       conta_certa == nprimos && soma_corpo + soma_prod + soma_nred == 1346);

    /* as testemunhas. Regime K x K: u = (2·s - n)/s tem u^2 = 1, e e± = (1 ± u)/2 são
     * complementares — um em cada factor. Regime nao reduzido: eps = s - n/2 tem eps^2 = 0
     * e eps != 0. As duas expressões sao as do texto, escritas no anel e reduzidas mod p. */
    int idem_falhou = 0, quantos_idem = 0, nil_falhou = 0, quantos_nil = 0;
    for(int i = 0; i < nprimos; i++){
        int p = primos[i], meio = fp_inv(2, p);
        for(int n = 0; n < p; n++){
            int s = 0, cl = fp_classe(n, p, &s);

            if(cl == 1){
                Fp w  = fp_mul((Fp){ imod(-n, p), imod(2, p) }, fp_const(fp_inv(s, p), p), n, p);
                Fp ep = fp_mul(fp_add(fp_const(1, p), w, p), fp_const(meio, p), n, p);
                Fp em = fp_mul(fp_sub(fp_const(1, p), w, p), fp_const(meio, p), n, p);
                int rp = imod((long long)n + s, p) * meio % p;    /* (n + s)/2 */
                int rm = imod((long long)n - s, p) * meio % p;    /* (n - s)/2 */
                /* as duas raizes sao mesmo raizes, e sao distintas (s != 0 neste regime) */
                if(fp_raizes(n, p, NULL) != 2 || rp == rm) idem_falhou++;
                if(!fp_eq(fp_add(ep, em, p), fp_const(1, p))) idem_falhou++;   /* e+ + e- = 1 */
                if(!fp_zero(fp_mul(ep, em, n, p)))            idem_falhou++;   /* e+ · e- = 0 */
                if(!fp_eq(ep, fp_mul(ep, ep, n, p)))           idem_falhou++;   /* e+ ^2 = e+ */
                if(fp_em(ep, rp, p) != 1)                     idem_falhou++;   /* 1 na sua */
                if(fp_em(ep, rm, p) != 0)                     idem_falhou++;   /* 0 na outra */
                quantos_idem++;
            }
            else if(cl == 0){
                Fp eps = (Fp){ imod(-(long long)n * meio, p), 1 };
                if(fp_zero(eps) || !fp_zero(fp_mul(eps, eps, n, p))) nil_falhou++;
                quantos_nil++;
            }
        }
    }
    printf("      testemunhas: %d pares de idempotentes complementares (e+ + e- = 1, e+ · e- = 0,\n"
             "      e+ = 1 na raiz (n+s)/2 e 0 na outra), %d nilpotentes eps com eps^2 = 0 != eps\n",
           quantos_idem, quantos_nil);
    ok("o regime K x K EXIBE idempotentes complementares: e± = (1 ± (2·s - n)/s)/2, com e+ em cada raiz",
       quantos_idem > 0 && idem_falhou == 0);
    ok("e o regime nao reduzido EXIBE o nilpotente eps = s - n/2: eps^2 = 0 e eps != 0",
       quantos_nil > 0 && nil_falhou == 0);

    /* e a ponte com o §M9 fecha pelo outro lado: em Z o anel Z[s] so' DEPOE em n = 0, e
     * e' la que aparecem divisores de zero — (s - 1)(s + 1) = s^2 - 1 = 0 com os dois
     * factores NAO nulos. Para n != 0 o anel e' dominio de integridade, e nao ha' tal par
     * (Z[s] esta' dentro de Q(sqrt(sf(Delta))), que e' dominio). Logo o criterio de §M13 e'
     * o mesmo, visto por dentro: o que o Delta proibe e' exactamente o que §M9 mediu. */
    int zd_n0 = (eq(mul(zs(-1,1), zs(1,1), 0), zs(0,0))
                 && !eq(zs(-1,1), zs(0,0)) && !eq(zs(1,1), zs(0,0)));
    int zd_fora = 0;
    for(L n = 1; n <= 8; n++)
      for(L p = -6; p <= 6; p++) for(L q = -6; q <= 6; q++)
        for(L r = -6; r <= 6; r++) for(L s = -6; s <= 6; s++){
            Zs x = zs(p,q), y = zs(r,s);
            if(eq(x, zs(0,0)) || eq(y, zs(0,0))) continue;
            if(eq(mul(x, y, n), zs(0,0))) zd_fora++;
        }
    ok("e em Z concorda com §M9, visto por dentro: so' n = 0 tem divisores de zero\n"
       "      ((sigma - 1)(sigma + 1) = 0 com ambos nao nulos), e nenhum para n != 0",
       zd_n0 && zd_fora == 0);

    /* ═══ §M15 — A EXCEPCAO DE CARACTERISTICA 2, registada a parte ══════════════════════
     *
     * §M13 usa (2 r - n)^2 = Delta_n E a volta r = (n ± s)/2 — e e' a segunda perna que
     * quebra: em caracteristica 2 nao ha' por onde dividir. O resultado e' que o criterio do
     * discriminante fica VAZIO: la Delta_n = n^2 + 4 = n^2 = (n)^2 e' quadrado para todo
     * n, logo o criterio prometeria "sempre redutivel" — e isso e' FALSO. O criterio certo,
     * sem o 2, escreve-se por r:
     *
     *     f_n tem raiz em K (char 2)  <=>  n = r + r^-1 para alguma r em K^×
     *
     * porque r^2 + n r + 1 = 0  <=>  n = (r^2 + 1)/r = r + r^-1. E medido em F_2 e F_4. */
    ok("em F_2, n = 0: f_n = x^2 + 1 = (x + 1)^2 tem raiz — o regime nao reduzido",
       fp_raizes(0, 2, NULL) == 1);
    ok("em F_2, n = 1: f_n = x^2 + x + 1 nao tem raiz nenhuma — logo e' CORPO, o campo F_4",
       fp_raizes(1, 2, NULL) == 0);

    /* o criterio do §M13 promete "redutivel" em TODO o lado, e so' um cumpre: a excecao
     * e' NECESSARIA, e a contra-exemplo e' um corpo de 4 elementos. */
    int promete = 0, cumpre = 0;
    for(int n = 0; n < 2; n++){
        if(fp_classe(n, 2, NULL) != 2) promete++;       /* Delta quadrado => "redutivel" */
        if(fp_raizes(n, 2, NULL) != 0) cumpre++;
    }
    ok("o criterio de §M13 promete 'redutivel' nos 2 valores de n de F_2 e so' 1 cumpre:\n"
       "      logo a excecao de caracteristica 2 e' NECESSARIA, nao uma cautela de estilo",
       promete == 2 && cumpre == 1);

    /* F_2 pelo criterio certo: S = {r + r^-1 : r nao nulo}. O unico r nao nulo e' 1, e
     * 1 + 1/1 = 0, logo S = {0} — e o criterio "redutivel <=> n em S" tem de bater com a
     * busca de §M13, medido pelos dois lados. */
    int tam_S2 = 0, S2[2];
    for(int i = 0; i < 2; i++) S2[i] = -1;
    for(int r = 1; r < 2; r++){
        int v = imod(r + r, 2), novo = 1;          /* com r = 1 tem-se r^-1 = r */
        for(int i = 0; i < 2; i++) if(S2[i] == v) novo = 0;
        if(novo) S2[tam_S2++] = v;
    }
    int inequiv2 = 0, com_raiz2 = 0;
    for(int n = 0; n < 2; n++){
        int em_S = 0;
        for(int i = 0; i < tam_S2; i++) if(S2[i] == n) em_S = 1;
        int achou = (fp_raizes(n, 2, NULL) != 0);
        if(em_S != achou) inequiv2++;
        if(achou) com_raiz2++;
    }
    ok("em F_2 o criterio certo da S = {r + r^-1} = {0}, e bate certo com a busca: so' n = 0",
       inequiv2 == 0 && tam_S2 == 1 && com_raiz2 == 1);

    /* F_4: S = {0, 1} e' o corpo TODO, logo nenhum n da um corpo */
    int tam_S = 0, S[4];
    for(int i = 0; i < 4; i++) S[i] = -1;
    for(int r = 1; r < 4; r++){
        int v = f4_add(r, f4_inv(r)), novo = 1;
        for(int i = 0; i < 4; i++) if(S[i] == v) novo = 0;
        if(novo) S[tam_S++] = v;
    }
    int inequiv = 0, sem_raiz = 0, quantos_raiz4 = 0, quebra4 = 0;
    for(int n = 0; n < 4; n++){
        int em_S = 0;
        for(int i = 0; i < tam_S; i++) if(S[i] == n) em_S = 1;
        int achou = 0;
        for(int r = 0; r < 4; r++)
            if(f4_add(f4_add(f4_mul(r, r), f4_mul(n, r)), 1) == 0) achou = 1;
        if(em_S != achou) inequiv++;
        if(achou) quantos_raiz4++;
        if(!achou){
            sem_raiz++;
            /* o teste do §M13 promete "redutivel" (Delta = n^2 e' quadrado, sempre) e aqui
             * f_n NAO tem raiz nenhuma: o corpo e' um campo de 16 elementos */
            if(fp_classe(n, 4, NULL) != 2) quebra4++;
        }
    }
    printf("      char 2: em F_2, S = {r + r^-1} = {0} e 1 dos 2 n da' corpo;\n"
             "              em F_4, S = %d elementos e %d dos 4 n dao' corpo (os %d irredutiveis)\n",
           tam_S, quantos_raiz4, sem_raiz);
    ok("char 2: f_n tem raiz <=> n = r + r^-1 — o criterio certo, verificado em F_2 e em F_4",
       inequiv == 0 && tam_S == 2 && sem_raiz == 2 && quantos_raiz4 == 2);
    ok("e o teste do discriminante ERRA em caracteristica 2, e no mesmo sentido: promete\n"
       "      'redutivel' onde f_n e' irredutivel — 1 caso em F_2 e 2 em F_4, todos campos", quebra4 == 2);

    /* ═══ e a contagem de reais neste ficheiro ═════════════════════════════════════════ */
    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────────");
        puts("  O QUE ISTO SEPARA. A familia metalica nao precisa dos reais. Ela e' o anel");
        puts("  Z[x]/(x^2 - nx - 1), o traco e' o indice n, a norma e' -1, e a Lei 2 le-se");
        puts("  la' dentro na forma mais curta que ela tem: A INVERSA E' MENOS A CONJUGADA.");
        puts("  Essa equacao vale exatamente nos elementos de norma -1 — e norma e' inteiro.");
        puts("");
        puts("  O corpo dual tambem esta' la': a conjugacao e' o J (involucao e homomorfismo),");
        puts("  e a forma traco e' a leitura, com determinante n^2+4 = Delta. O discriminante");
        puts("  NAO e' uma quantidade acessoria: e' o determinante do emparelhamento dual.");
        puts("");
        puts("  E a fronteira ficou medida. Ser unidade e' algebrico e fica na Teoria; ser");
        puts("  Pisot precisa de comparar tamanhos e desce ao Catalogo com Mobius e as");
        puts("  fraccoes continuas. Os dois criterios nao coincidem — ha' Pisot que nao e'");
        puts("  unidade e unidade que nao e' Pisot. A familia metalica esta' na interseccao,");
        puts("  e e' por isso que o teorema vale: nao por sorte, por estar nos dois lados.");
        puts("");
        puts("  E O DELTA DECIDE O ANEL. Nao e' so' o determinante da leitura (§M8): e' o que");
        puts("  diz QUE OBJETO o quociente e'. f_n tem raiz em K <=> Delta_n e' quadrado em K,");
        puts("  e entao o quociente e' corpo; se Delta_n = s^2 != 0, e' K x K, com idempotentes");
        puts("  EXIBIDOS — u = (2·sigma - n)/s tem u^2 = 1, e e± = (1 ± u)/2 sao complementares,");
        puts("  um em cada factor; e se Delta_n = 0, o anel nao e' reduzido e o nilpotente");
        puts("  eps = sigma - n/2 se ve: eps^2 = 0 com eps != 0.");
        puts("");
puts("  Sobre R a familia nunca da' um corpo: da' sempre R x R. E a excecao de");
        puts("  caracteristica 2 ficou registada A PARTE, porque la o discriminante nao decide");
        puts("  nada — em F_2 o n = 1 da' o campo F_4, e em F_4 sao 2 dos 4 n que dao' campo,");
        puts("  sempre contra o que o teste do Delta promete. Um instrumento que esconde a");
        puts("  excecao mede o instrumento, nao a familia.");

    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
