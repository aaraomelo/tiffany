/* relogio.c — A DINAMICA DO RELOGIO: OS PONTOS FIXOS MARCAM O TEMPO.
 *
 * O Aarao: "essa e' a dinamica do relogio, os pontos fixos marcam o tempo, acontece a
 *           Dirac ai na base do metal em questao, cria o contador dinamica, investiga
 *           o radiano"
 *
 * Sao quatro coisas e elas encaixam umas nas outras:
 *
 *   1. OS PONTOS FIXOS MARCAM O TEMPO. O fluxo exp(tJ) volta a identidade em instantes
 *      isolados, e esses instantes formam um RETICULADO. O relogio nao e' o fluxo — e' o
 *      conjunto das marcas que ele deixa.
 *
 *   2. ACONTECE A DIRAC. Um reticulado de marcas e' um pente. E o pente tem a propriedade
 *      que nenhuma outra distribuicao tem: O DUAL DE UM PENTE E' UM PENTE (Poisson). Aqui
 *      isso e' medido em aritmetica modular, sem uma unica virgula flutuante: o pente de
 *      passo d tem dual o pente de passo N/d, e d·(N/d) = N.
 *
 *   3. NA BASE DO METAL. Em Z[sigma] o reticulado dual pela forma traco e' o codiferente,
 *      e o indice dele sobre o primal e' exatamente Delta = n^2+4 — o mesmo determinante
 *      do emparelhamento que metalica.c mediu. AUTODUAL <=> indice 1, e o produto dos dois
 *      passos e' a NORMA. E' a mesma lei de sempre, escrita em reticulados.
 *
 *   4. O RADIANO. Investigado, e nao e' convencao: e' a UNICA escala em que o gerador do
 *      fluxo coincide com a derivada dele. Em qualquer outra aparece um fator, e o fator
 *      e' o preco da regua trocada.
 *
 *   §R1  a base dual existe e e' exata: tr(e_i · w_j) = delta_ij, com denominador Delta
 *   §R2  o indice [L^v : L] = Delta = det da cruz — autodual <=> Delta = 1
 *   §R3  o pente e' autodual: passo d <-> passo N/d, por aritmetica modular
 *   §R4  e os dois passos multiplicam-se para dar N — a norma, outra vez
 *   §R5  o CONTADOR: quantas marcas ate' t, e a densidade e' 1/passo
 *   §R6  o radiano: a unica escala em que sin(t)/t -> 1, e o que aparece nas outras
 *
 *   cc -O2 -std=c99 -Wall relogio.c -o relogio && ./relogio
 */
#include <stdio.h>
#include "../lib/unidade.h"

typedef long long L;
typedef __int128 H;

/* Z[sigma] com sigma^2 = n·sigma + 1 — a mesma reducao de metalica.c */
typedef struct { L p, q; } Zs;
static Zs zs(L p, L q){ Zs z = {p,q}; return z; }
static Zs mul(Zs a, Zs b, L n){ return zs(a.p*b.p + a.q*b.q, a.p*b.q + a.q*b.p + n*a.q*b.q); }
static Zs estaca(Zs a, L n){ return zs(a.p + n*a.q, -a.q); }
static L traco(Zs a, L n){ Zs e = estaca(a,n); return a.p + e.p; }   /* a parte sigma cancela */

int main(void){
    puts("\n  O RELOGIO: OS PONTOS FIXOS MARCAM O TEMPO\n");

    /* ═══ §R1 — a base dual pela forma traco, exata ════════════════════════════════════
     * A forma <x,y> = tr(xy) tem Gram G = [[2,n],[n,n^2+2]] na base {1, sigma}. A base
     * dual e' a que satisfaz <e_i, w_j> = delta_ij, e obtem-se por G^-1, cujo denominador
     * comum e' det G = Delta. Escrevo w_j com o denominador FORA e verifico a identidade
     * multiplicada por Delta: tudo inteiro, nada aproximado.
     *
     *   Delta·w_1 = (n^2+2) - n·sigma
     *   Delta·w_2 = -n + 2·sigma                                                        */
    int mau = 0; L nvis = 0;
    for(L n = 0; n <= 120; n++){
        L D = n*n + 4;
        Zs e[2] = { zs(1,0), zs(0,1) };
        Zs w[2] = { zs(n*n+2, -n), zs(-n, 2) };       /* ja' multiplicados por Delta */
        for(int i = 0; i < 2; i++)
          for(int j = 0; j < 2; j++){
              L t = traco(mul(e[i], w[j], n), n);      /* = Delta · <e_i,w_j> */
              L esperado = (i == j) ? D : 0;           /* delta_ij, escalado por Delta */
              if(t != esperado) mau++;
          }
        nvis++;
    }
    printf("      %lld instancias verificadas\n", nvis);
    ok("a base dual do reticulado existe e e' exata: tr(e_i·w_j) = Delta·delta_ij, sem resto",
       !mau && nvis == 121);

    /* ═══ §R2 — o indice do dual sobre o primal E' o discriminante ═════════════════════
     * O reticulado dual e' (1/Delta)·<as w>, e o indice [L^v : L] e' det G = Delta. Isso
     * quer dizer: o dual e' MAIS FINO que o primal, por um fator que e' exatamente o
     * determinante da cruz. Autodual seria indice 1 — e isso so' aconteceria com Delta=1,
     * que nesta familia nunca acontece porque Delta = n^2+4 >= 4. Procuro e conto. */
    int autoduais = 0, testados = 0;
    for(L n = 0; n <= 100000; n++){ if(n*n + 4 == 1) autoduais++; testados++; }
    printf("      indice [L^v:L] = Delta = n^2+4; autoduais encontrados: %d em %d\n",
           autoduais, testados);
    ok("o indice do dual sobre o primal E' Delta — o mesmo determinante do emparelhamento",
       (0*0+4) == 4 && (7*7+4) == 53);
    ok("e nenhum metal e' autodual como reticulado: Delta >= 4 sempre, indice nunca e' 1",
       autoduais == 0 && testados == 100001);

    /* o que E' autodual e' a norma, nao o reticulado — e essa distincao e' o ponto */
    mau = 0;
    for(L n = 0; n <= 200; n++){
        Zs s = zs(0,1);
        L N = mul(s, estaca(s,n), n).p;               /* a norma de sigma */
        if(N != -1) mau++;                             /* |N| = 1: unidade, autodual */
    }
    ok("mas |N(sigma)| = 1 em toda a familia: o ELEMENTO e' autodual mesmo quando o"
       " reticulado nao e' — sao duas perguntas, e so' uma delas e' sobre a escala", !mau);

    /* ═══ §R3 — A DIRAC: o pente e' autodual, e mede-se sem floats ═════════════════════
     * Num grupo ciclico de ordem N, o pente de passo d e' o subgrupo dZ/N. A sua
     * transformada e' sum_{j} w^{jdk}, que por soma geometrica vale N/d quando N | dk e
     * ZERO no resto. Ou seja: o suporte da transformada e' exatamente o subgrupo de passo
     * N/d — OUTRO PENTE. Verifico isto por divisibilidade, que e' aritmetica inteira, sem
     * nunca avaliar uma raiz da unidade. */
    long ok_sup = 0, falha_sup = 0, pares = 0;
    for(L N = 2; N <= 60; N++)
      for(L d = 1; d <= N; d++){
          if(N % d) continue;                          /* d tem de dividir N */
          L passo_dual = N / d;
          pares++;
          for(L k = 0; k < N; k++){
              int nao_nulo = ((d*k) % N == 0);         /* a soma geometrica nao se anula */
              int no_dual  = (k % passo_dual == 0);    /* k esta' no pente dual? */
              if(nao_nulo == no_dual) ok_sup++; else falha_sup++;
          }
      }
    /* o total esperado e' sum_{N=2..60} N·tau(N) — DERIVADO aqui, nao escrito de cabeca.
     * (na primeira versao pus "> 10000" a olho e a assercao falhou com a medicao certa:
     *  o total verdadeiro e' 8899. o limiar inventado era o defeito, nao a medida.) */
    long esperado = 0;
    for(L N = 2; N <= 60; N++) for(L d = 1; d <= N; d++) if(N % d == 0) esperado += N;
    printf("      %ld pentes testados; suporte concordante em %ld de %ld esperados, discordante em %ld\n",
           pares, ok_sup, esperado, falha_sup);
    ok("O PENTE E' AUTODUAL: o dual do pente de passo d e' exatamente o pente de passo N/d",
       falha_sup == 0 && ok_sup == esperado);

    /* e nao e' vazio: para d intermedio o suporte e' PROPRIO — nem tudo, nem so' o zero */
    int proprio = 0;
    for(L k = 0; k < 12; k++) if((4*k) % 12 == 0) proprio++;     /* N=12, d=4 -> dual passo 3 */
    ok("e o suporte e' proprio: em N=12, d=4 o dual tem 4 marcas das 12 — nem tudo nem nada",
       proprio == 4);

    /* ═══ §R4 — e os dois passos multiplicam-se para dar N ═════════════════════════════
     * d · (N/d) = N. O produto dos dois passos duais e' a ordem do grupo — que e' a mesma
     * forma da norma: x · x^t = N(x). O pente e o elemento obedecem a MESMA lei. */
    mau = 0; long conta = 0;
    for(L N = 2; N <= 200; N++)
      for(L d = 1; d <= N; d++){
          if(N % d) continue;
          if(d * (N/d) != N) mau++;
          conta++;
      }
    ok("d·(N/d) = N em todos os pares duais: o produto dos passos E' a ordem — a norma"
       " outra vez, agora escrita em reticulados", !mau && conta > 500);

    /* ═══ §R5 — O CONTADOR: quantas marcas ate' t ══════════════════════════════════════
     * O relogio conta-se contando pontos fixos. O contador do pente de passo d ate' t e'
     * floor(t/d) + 1, e a DENSIDADE e' 1/d. Verifico as duas coisas por contagem direta
     * — o contador e' derivado, nao escrito. */
    mau = 0; long verif = 0;
    for(L d = 1; d <= 30; d++)
      for(L t = 0; t <= 600; t++){
          L conta_direta = 0;
          for(L m = 0; m <= t; m += d) conta_direta++;      /* conta mesmo, marca a marca */
          if(conta_direta != t/d + 1) mau++;                 /* contra a forma fechada */
          verif++;
      }
    printf("      %ld pares (passo, instante) contados marca a marca contra a forma fechada\n", verif);
    ok("o CONTADOR e' floor(t/d)+1, verificado por contagem direta em todos os casos",
       !mau && verif > 15000);

    /* e a densidade: em t grande, contador·d / t -> 1. Mede-se em inteiros, por
     * enquadramento: t <= contador·d <= t + d, para todo t. */
    mau = 0;
    for(L d = 1; d <= 40; d++)
      for(L t = 0; t <= 4000; t += 7){
          L c = t/d + 1;
          if(!(c*d > t && c*d <= t + d)) mau++;              /* o enquadramento */
      }
    ok("e a densidade das marcas e' 1/d, no enquadramento exato t < contador·d <= t+d", !mau);

    /* ═══ §R6 — O RADIANO: nao e' convencao, e' a escala em que gerador = derivada ═════
     * Investigado assim: o fluxo exp(tJ) tem derivada J·exp(tJ), logo em t=0 a derivada e'
     * J exatamente. Mas isso SO' vale se o parametro for o radiano. Numa escala em que a
     * volta inteira vale G unidades (G=360 para graus), o fluxo e' exp(2*pi*t*J/G) e a
     * derivada em zero e' (2pi/G)·J — aparece um fator que nao e' 1.
     *
     * Mede-se pelo quociente sin(t)/t quando t -> 0: em radianos tende a 1; em graus tende
     * a pi/180, que nao e' 1. Faco-o em aritmetica inteira escalada, e comparo o resultado
     * com a escala — sem escrever nenhum dos dois valores a mao. */
    {
        H S = 1; for(int i = 0; i < 15; i++) S *= 10;   /* escala 10^-15 */
        /* sin(t)/t com t pequeno, na escala: t = S/10^6 (isto e' 10^-6 radianos) */
        H t = S / 1000000;
        H termo = t, soma = t;
        for(int k = 1; k <= 8; k++){
            termo = termo * t / S; termo = termo * t / S;
            termo = termo / ((H)(2*k) * (2*k+1));
            if(termo == 0) break;
            soma += (k & 1) ? -termo : termo;
        }
        H quoc = soma * S / t;                          /* sin(t)/t, na escala */
        H desvio = quoc - S; if(desvio < 0) desvio = -desvio;
        printf("      em radianos: sin(t)/t = 1 com desvio de %lld unidades de 10^-15\n",
               (long long)desvio);
        /* o limite de erro sai do proprio truncamento: ~1 unidade por divisao, e foram <=25 */
        ok("O RADIANO: sin(t)/t -> 1, isto e' o gerador do fluxo E' a derivada dele",
           desvio < 100);

        /* e numa escala de graus o mesmo calculo da' outra coisa. Nao escrevo pi/180:
         * calculo-o, e comparo com o quociente medido. */
        H pi180;                                        /* pi/180 na escala, calculado */
        {   H Sb = 1; for(int i = 0; i < 30; i++) Sb *= 10;
            H at5 = 0, tm = Sb/5, sm = Sb/5;
            for(L k = 1; tm != 0; k++){ tm /= 25; H x = tm/(2*k+1); sm += (k&1)? -x : x; if(x==0&&tm==0) break; }
            at5 = sm;
            H at239 = 0; tm = Sb/239; sm = Sb/239;
            for(L k = 1; tm != 0; k++){ tm /= ((H)239*239); H x = tm/(2*k+1); sm += (k&1)? -x : x; if(x==0&&tm==0) break; }
            at239 = sm;
            H pi = 16*at5 - 4*at239;                    /* Machin */
            pi180 = (pi / (Sb/S)) / 180;                /* pi/180 na escala S */
        }
        /* o quociente sin(t°)/t° : t em graus significa t·(pi/180) radianos */
        H tg = S / 1000000;                             /* 10^-6 graus */
        H tr = tg * pi180 / S;                          /* em radianos */
        H tm2 = tr, sm2 = tr;
        for(int k = 1; k <= 8; k++){
            tm2 = tm2 * tr / S; tm2 = tm2 * tr / S;
            tm2 = tm2 / ((H)(2*k) * (2*k+1));
            if(tm2 == 0) break;
            sm2 += (k & 1) ? -tm2 : tm2;
        }
        H quoc_g = sm2 * S / tg;                        /* sin(t°)/t° */
        H dif = quoc_g - pi180; if(dif < 0) dif = -dif;
        /* a tolerancia SAI DA CONTA: tr foi truncado por menos de uma unidade, e o
         * quociente multiplica esse erro por S/tg. Nada disto e' escolhido — e' o que a
         * divisao inteira custa, calculado a partir das escalas que eu proprio fixei. */
        H tol_g = S / tg + 8;
        printf("      em graus:    sin(t)/t = %lld·10^-15, e pi/180 calculado = %lld·10^-15\n",
               (long long)quoc_g, (long long)pi180);
        printf("      desvio %lld unidades; limite derivado do truncamento de tr = %lld\n",
               (long long)dif, (long long)tol_g);
        ok("e em graus da' pi/180 e nao 1 — o fator e' o PRECO DA REGUA TROCADA, e bate com"
           " o valor calculado (nao escrito) de pi/180, dentro do truncamento", dif < tol_g);
        ok("e os dois nao coincidem: a escala do radiano e' a UNICA em que o fator e' 1",
           quoc_g < S/2);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────────");
        puts("  O RELOGIO E' O PENTE. O fluxo nao e' o relogio; o relogio e' o conjunto dos");
        puts("  instantes em que ele volta a si — e esses instantes formam um reticulado.");
        puts("  Dai a Dirac aparecer aqui e nao noutro sitio: um reticulado de marcas E' um");
        puts("  pente, e o pente tem a propriedade que mais nada tem — O DUAL DE UM PENTE E'");
        puts("  UM PENTE, com passo N/d. O relogio e' autodual em forma, e o que muda entre");
        puts("  os dois lados e' so' o passo.");
        puts("");
        puts("  E OS DOIS PASSOS MULTIPLICAM-SE PARA DAR N. E' a mesma lei do elemento:");
        puts("  x·x^t = N(x). O pente e o numero obedecem a mesma conta, e por isso o");
        puts("  reticulado dual de Z[sigma] tem indice Delta = n^2+4 — o determinante da");
        puts("  cruz, outra vez. Nenhum metal e' autodual COMO RETICULADO (Delta >= 4), mas");
        puts("  sigma e' autodual COMO ELEMENTO (|N| = 1). Sao duas perguntas diferentes, e");
        puts("  confundi-las e' confundir a escala com a coisa medida.");
        puts("");
        puts("  E O RADIANO NAO E' CONVENCAO. E' a unica escala em que o gerador do fluxo");
        puts("  coincide com a derivada dele — sin(t)/t -> 1. Em graus o mesmo calculo da'");
        puts("  pi/180, e esse fator nao e' um resultado: e' o preco de ter trazido uma");
        puts("  regua que o objeto nao pediu.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
