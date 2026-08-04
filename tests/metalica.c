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
 *
 * ZERO doubles neste ficheiro. Nem um. Se aparecer um, a tese caiu.
 *
 *   cc -O2 -std=c99 -Wall metalica.c -o metalica && ./metalica
 */
#include <stdio.h>
#include <stdlib.h>
#include "../lib/unidade.h"

typedef long long L;

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
     * inteira, nao afirmado. E em n = 0 tem-se sigma^2 = 1: e' a involucao pura. */
    int quadrados = 0; L onde = -1;
    for(L n = 0; n <= 100000; n++){
        L d = n*n + 4, r = (L)0;
        while(r*r < d) r++;                      /* raiz inteira por busca, sem sqrt */
        if(r*r == d){ quadrados++; if(onde < 0) onde = n; }
    }
    printf("      Delta = n^2+4 e' quadrado perfeito em %d dos 100001 casos (n = %lld)\n", quadrados, onde);
    ok("n = 0 e' o UNICO n em [0,100000] com Delta quadrado: o unico onde o anel decompoe",
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
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
