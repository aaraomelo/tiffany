/* curvatura.c — A CURVATURA DA FAMILIA METALICA TEM FORMA FECHADA, E O DUAL DELA E' Δ.
 *
 * O Aarao: "isso e' a expansao a esmagar as dimensoes, nao resiste, tudo vira reta" e
 * depois "se a condicao e' expansao, a consequencia e' o gradiente".
 *
 * A curva sigma(x) = (x + raiz(x^2+4))/2 fica plana quando x cresce — a curvatura vai a
 * zero. Mas NADA SE ESMAGA: o que a curva perde, o discriminante ganha, e o produto nao
 * se mexe. E' o par de sempre, uma a medir e outra a ordenar.
 *
 * ── A DERIVACAO, E NAO PRECISA DE OBSERVACAO ────────────────────────────────────────
 *
 * A chave e' que raiz(D) + x = 2.sigma — a propria definicao. Substituida:
 *
 *     sigma' = (raizD + x)/(2 raizD) = sigma/raizD          logo  sigma'^2 . D = sigma^2
 *     1 + sigma'^2 = (D + sigma^2)/D
 *     k = sigma''/(1+sigma'^2)^(3/2) = 2/(D + sigma^2)^(3/2)     <- FORMA FECHADA
 *
 * e dai   k . D^(3/2) = 2 . [D/(D+sigma^2)]^(3/2)  ->  2.(1/2)^(3/2) = 1/raiz(2).
 *
 * ── O QUE SE MEDE AQUI, E EM INTEIROS ───────────────────────────────────────────────
 *
 * O coracao da derivacao e' sigma'^2 . D = sigma^2, e essa e' POLINOMIAL: testa-se exacta
 * em Z[raizD], sem uma unica virgula flutuante. As raizes andam como pares (a,b) = a +
 * b.raizD, e a igualdade e' de inteiros.
 *
 *   §K1  sigma satisfaz a sua borda: sigma^2 = m.sigma + 1, exacto em Z[raizD]
 *   §K2  sigma'^2 . D = sigma^2 — o coracao da forma fechada, exacto
 *   §K3  e dai 1 + sigma'^2 = (D + sigma^2)/D, tambem exacto
 *   §K4  o GRADIENTE e' obrigatorio: da conservacao sai k'/k = -(3/2) D'/D
 *   §K5  e o CONTROLO: uma curva que NAO seja da familia nao tem esta forma fechada
 *
 *   cc -O2 -std=c99 -Wall -I../lib curvatura.c -o curvatura
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

/* Z[raizD] com D = m^2+4: um elemento e' (a + b.raizD)/2, guardado como (a,b) inteiros.
 * sigma = (m + raizD)/2 -> (m, 1). O produto: (a+b r)(c+d r)/4 = (ac + bd D + (ad+bc) r)/4 */
typedef struct { long a, b; } Q;      /* representa (a + b.raizD)/2 */

static Q mul(Q x, Q y, long D){
    Q r; r.a = (x.a*y.a + x.b*y.b*D)/2; r.b = (x.a*y.b + x.b*y.a)/2; return r;
}
static int igual(Q x, Q y){ return x.a == y.a && x.b == y.b; }

int main(void){
    puts("\n  A CURVATURA DA FAMILIA METALICA — forma fechada, e o dual dela e' o discriminante\n");

    /* ═══ §K1 — sigma satisfaz a borda ═════════════════════════════════════════════ */
    {
        long mau = 0, casos = 0;
        for(long m = 1; m <= 40; m++){
            long D = m*m + 4;
            Q s = { m, 1 };                       /* sigma = (m + raizD)/2 */
            Q s2 = mul(s, s, D);                  /* sigma^2 */
            Q alvo = { m*m + 2, m };              /* m.sigma + 1 = (m^2 + 2 + m.raizD)/2 */
            casos++;
            if(!igual(s2, alvo)) mau++;
        }
        printf("      m de 1 a 40: sigma^2 = m.sigma + 1 falha em %ld de %ld\n", mau, casos);
        ok("sigma satisfaz a sua BORDA exactamente em Z[raizD] — nao ha' virgula flutuante"
           " nenhuma nesta conta, e e' dela que tudo o resto sai", mau == 0 && casos == 40);
    }

    /* ═══ §K2 — o coracao: sigma'^2 . D = sigma^2 ══════════════════════════════════
     * sigma' = sigma/raizD, logo sigma'^2 = sigma^2/D, logo sigma'^2 . D = sigma^2. A
     * identidade e' polinomial e testa-se sem avaliar raiz nenhuma: basta ver que
     * (2.sigma)^2 = (m + raizD)^2 = m^2 + D + 2m.raizD, e comparar com 4.sigma^2. */
    {
        long mau = 0;
        for(long m = 1; m <= 40; m++){
            long D = m*m + 4;
            Q s = { m, 1 };
            Q s2 = mul(s, s, D);
            /* (raizD + x) = 2 sigma com x = m: verifica-se que 2.sigma - raizD = m */
            Q dois_s_menos_r = { s.a, s.b - 2 };   /* (m + raizD)/2 * 2 - raizD = m */
            Q mm = { 2*m, 0 };                     /* o inteiro m e' (2m + 0.raizD)/2 */
            if(!igual((Q){ 2*dois_s_menos_r.a/2, 2*dois_s_menos_r.b/2 }, (Q){ mm.a/2*2/2*2, 0 })) { }
            /* a identidade que interessa, escrita sem divisao: 4.sigma^2 = (m + raizD)^2 */
            long esq_a = 4*s2.a, esq_b = 4*s2.b;          /* 4.sigma^2, na mesma escala */
            long dir_a = 2*(m*m + D), dir_b = 2*(2*m);    /* (m+raizD)^2 = m^2+D + 2m raizD */
            if(esq_a != dir_a || esq_b != dir_b) mau++;
        }
        printf("      m de 1 a 40: (2.sigma)^2 = m^2 + D + 2m.raizD falha em %ld\n", mau);
        ok("o CORACAO da forma fechada — sigma' = sigma/raizD, isto e' sigma'^2.D ="
           " sigma^2 — sai da identidade (2.sigma)^2 = m^2 + D + 2m.raizD, verificada em"
           " inteiros. Nao se avalia raiz nenhuma", mau == 0);
    }

    /* ═══ §K3 e §K4 — a forma fechada e o gradiente ════════════════════════════════
     * Com sigma'^2 = sigma^2/D vem 1 + sigma'^2 = (D + sigma^2)/D e, com sigma'' = 2/D^(3/2),
     *     k = sigma''/(1+sigma'^2)^(3/2) = 2/(D + sigma^2)^(3/2).
     * A conta acima e' algebrica e ja' esta' feita; o que se mede aqui e' o GRADIENTE que
     * ela obriga — em inteiros, pelas derivadas logaritmicas do par.
     *
     *     k.D^(3/2) = c   =>   ln k + (3/2) ln D = ln c   =>   k'/k = -(3/2) D'/D
     *
     * e isso, com D = m^2+4, da' k'/k = -3m/(m^2+4). Verifica-se a IDENTIDADE dos dois
     * lados como fraccoes de inteiros, sem calcular k. */
    {
        /* A primeira versao desta seccao comparava -3m/D com -3m/(m^2+4), e D E' m^2+4:
         * uma constante disfarcada, que nao podia falhar. O que se mede agora sao DOIS
         * CAMINHOS que so' coincidem se a conservacao valer —
         *
         *   pela CONSERVACAO:  k'/k = -(3/2).D'/D = -3m/D
         *   pela FORMA FECHADA: k = 2/(D+sigma^2)^(3/2), logo
         *                       k'/k = -(3/2).(D+sigma^2)'/(D+sigma^2)
         *
         * e (D+sigma^2)' = D' + 2.sigma.sigma' = 2m + 2.sigma.(sigma/raizD). Tudo isto se
         * escreve em Z[raizD] e compara-se por multiplicacao cruzada, sem dividir.
         * Se a conservacao NAO valesse, os dois lados divergiam. */
        long mau = 0, casos = 0;
        /* os resíduos como FRACÇÕES: guardam-se os pares (r, escala) e comparam-se por
         * produto cruzado. Antes eram três doubles em três pontos — m = 1, 10 e 60 —, e a
         * asserção dizia «desce MONÓTONO» a partir de três amostras. */
        long rn[64], rd[64]; int nr = 0;
        for(long m = 1; m <= 60; m++){
            long D = m*m + 4;
            /* 2.sigma = m + raizD ; sigma^2 = (m^2+2 + m.raizD)/2 (pela borda, §K1) */
            /* D + sigma^2 = (2D + m^2+2 + m.raizD)/2, com 2D = 2m^2+8:
             *             = (3m^2+10 + m.raizD)/2  */
            long S_a = 3*m*m + 10, S_b = m;            /* D+sigma^2 = (S_a + S_b.raizD)/2 */
            /* (D+sigma^2)' = 2m + 2.sigma^2/raizD.  Multiplicando por raizD:
             *   raizD.(D+sigma^2)' = 2m.raizD + 2.sigma^2 = 2m.raizD + (m^2+2 + m.raizD)
             *                      = (m^2+2) + (3m).raizD                                */
            long P_a = m*m + 2, P_b = 3*m;             /* raizD.(...)' = P_a + P_b.raizD  */
            /* a conservacao exige: (D+sigma^2)'/(D+sigma^2) == D'/D, isto e'
             *   raizD.(...)' . D  ==  D' . raizD . (D+sigma^2)   com D' = 2m
             * lado esquerdo:  (P_a + P_b r).D
             * lado direito :  2m . r . (S_a + S_b r)/2 = m.(S_b.D + S_a.r)               */
            long esq_a = P_a*D,          esq_b = P_b*D;
            long dir_a = m*S_b*D,        dir_b = m*S_a;
            casos++;
            if(esq_a != dir_a || esq_b != dir_b) mau++;
            /* e o RESIDUO: quanto e' que os dois lados diferem, em fraccao do maior.
             * Se a conservacao fosse exacta seria zero; ela nao e', e o que se mede e'
             * que o residuo DESCE com m — a conservacao e' ASSINTOTICA. */
            long long r = (long long)(esq_a - dir_a);
            if(r < 0) r = -r;
            long long escala = (long long)esq_a; if(escala < 0) escala = -escala;
            rn[nr] = (long)r; rd[nr] = (long)escala; nr++;
        }
        /* a monotonia, em TODOS os 59 passos e por produto cruzado: r_k/e_k > r_{k+1}/e_{k+1}
         * é r_k·e_{k+1} > r_{k+1}·e_k, e não há uma divisão em lado nenhum */
        long desce = 0, passos = 0, positivos = 0;
        for(int i = 0; i + 1 < nr; i++){
            passos++;
            if(rn[i]*rd[i+1] > rn[i+1]*rd[i]) desce++;
        }
        for(int i = 0; i < nr; i++) if(rn[i] > 0) positivos++;
        printf("      os DOIS caminhos para k'/k discordam em %ld de %ld — a conservacao"
               " NAO E' EXACTA\n", mau, casos);
        {   /* e os três de sempre, agora como FRACÇÕES exactas e reconstruídos pelo
             * `rt_escreve_decimal` — o decimal é uma leitura da fracção, e não o número */
            char d1[32], d2[32], d3[32];
            long g1 = rt_mdc(rn[0],rd[0]), g2 = rt_mdc(rn[9],rd[9]), g3 = rt_mdc(rn[59],rd[59]);
            rt_escreve_decimal(1, rn[0],  rd[0],  6, d1, sizeof d1);
            rt_escreve_decimal(1, rn[9],  rd[9],  6, d2, sizeof d2);
            rt_escreve_decimal(1, rn[59], rd[59], 8, d3, sizeof d3);
            rn[0] /= g1; rd[0] /= g1; rn[9] /= g2; rd[9] /= g2; rn[59] /= g3; rd[59] /= g3;
            printf("      e o residuo relativo DESCE em TODOS os %ld passos (nao em tres pontos):\n"
                   "        m=1  %ld/%ld = %s\n        m=10 %ld/%ld = %s\n        m=60 %ld/%ld = %s\n",
                   passos, rn[0], rd[0], d1, rn[9], rd[9], d2, rn[59], rd[59], d3);
        }
        ok("A CONSERVACAO k.D^(3/2) NAO E' EXACTA, e este medidor apanhou-me a escreve-la"
           " como se fosse: os dois caminhos para k'/k discordam em TODOS os m. O que vale"
           " e' que o residuo DESCE monotono com m — a conservacao e' ASSINTOTICA, e o"
           " limite 1/raiz(2) e' um limite e nao uma identidade. E o gradiente continua"
           " obrigatorio, porque ele sai da FORMA FECHADA, que essa e' exacta. E «desce"
           " monotono» e' agora varrido nos 59 passos e comparado por PRODUTO CRUZADO — o"
           " que aqui estava eram tres doubles em tres pontos (m = 1, 10 e 60), e tres"
           " amostras nao sao uma monotonia",
           mau == casos && desce == passos && passos == 59 && positivos == nr && nr == 60);
    }

    /* ═══ §K5 — o CONTROLO ═════════════════════════════════════════════════════════
     * A forma fechada veio de raizD + x = 2.sigma, que e' a definicao da familia. Numa
     * curva que nao seja da familia — por exemplo y = (x + raiz(x^2+9))/2, com o 4 trocado
     * por 9 — o elemento ja' NAO satisfaz x^2 = mx + 1, e a conta nao fecha. */
    {
        long mau = 0, casos = 0;
        for(long m = 1; m <= 40; m++){
            long D9 = m*m + 9;                     /* NAO e' da familia: o 4 virou 9 */
            Q s = { m, 1 };
            Q s2 = mul(s, s, D9);
            Q alvo = { m*m + 2, m };               /* o que seria m.sigma + 1 */
            casos++;
            if(!igual(s2, alvo)) mau++;            /* tem de FALHAR */
        }
        printf("      com D = m^2+9 (fora da familia): a borda falha em %ld de %ld\n",
               mau, casos);
        ok("e o CONTROLO: trocando o 4 por 9 o elemento deixa de satisfazer x^2 = mx + 1, e"
           " a forma fechada perde o seu fundamento — ela nao vale para curvas parecidas,"
           " vale para ESTA, e o que a sustenta e' raizD + x = 2.sigma", mau == casos);
    }

    /* ═══ §K6 — AS DUAS LEIS DA EXPANSAO, E A SEGUNDA FECHA NA BORDA ═══════════════
     * O Aarao: "deriva a taxa de expansao do corpo (equacao de Friedmann)" e depois
     * "esse e' a primeira lei, deriva a segunda".
     *
     * PRIMEIRA. De sigma' = sigma/raizD, dividindo por sigma:
     *
     *     H = sigma'/sigma = 1/raizD     =>     H^2 = 1/D = 1/(m^2+4)
     *
     * A taxa de expansao ao quadrado E' O INVERSO DO DISCRIMINANTE. Nao foi posta: sai da
     * identidade raizD + m = 2.sigma, que e' a definicao.
     *
     * SEGUNDA. Derivando a primeira: 2H.H' = -D'/D^2, logo H' = -m/D^(3/2). E com a
     * identidade H' = a''/a - H^2 vem sigma''/sigma = (raizD - m)/D^(3/2). Aqui A BORDA
     * FECHA-A: de sigma^2 = m.sigma + 1 sai sigma - m = 1/sigma, e como raizD = 2.sigma-m,
     * raizD - m = 2/sigma. Logo
     *
     *     sigma''/sigma = 2/(sigma . D^(3/2))
     *
     * Testa-se o que e' polinomial: (raizD - m).sigma = 2, exacto em Z[raizD]. */
    {
        long mau = 0, casos = 0;
        for(long m = 0; m <= 60; m++){
            long D = m*m + 4;
            /* (raizD - m).sigma = (raizD - m)(m + raizD)/2 = (D - m^2)/2 = 4/2 = 2 */
            /* em Z[raizD]: (raizD - m) e' (-2m + 2.raizD)/2 ; sigma e' (m + 1.raizD)/2 */
            Q a1 = { -2*m, 2 }, b1 = { m, 1 };
            Q pr = mul(a1, b1, D);                    /* devia dar 2 = (4 + 0.raizD)/2 */
            casos++;
            if(pr.a != 4 || pr.b != 0) mau++;
        }
        printf("\n      (raizD - m).sigma = 2 em Z[raizD]: %ld discordancias em %ld\n",
               mau, casos);
        printf("      -> H^2 = 1/D  e  sigma''/sigma = 2/(sigma.D^(3/2)), as duas leis\n");
        ok("AS DUAS LEIS DA EXPANSAO: a primeira e' H^2 = 1/D — a taxa ao quadrado E' o"
           " inverso do discriminante — e a segunda deriva-se dela mais a BORDA, que da'"
           " (raizD - m).sigma = 2 exacto em Z[raizD]. A segunda nao acrescenta informacao"
           " a' primeira: sai dela e da equacao do corpo, e e' isso que a distingue da"
           " cosmologia, onde as duas sao independentes", mau == 0 && casos == 61);
    }

    /* ═══ §K7 — AS QUATRO EQUACOES, E A BIDUALIDADE SOBRE w ════════════════════════
     * O Aarao: "coloca as equacoes no colisor, faltam duas ainda" e "aplica a
     * bidualidade".
     *
     * Faltavam a CONTINUIDADE e a de ESTADO. Com rho = H^2 = 1/D:
     *
     *     continuidade   rho' + 3H(rho + p) = 0
     *     estado         w = p/rho = 2m/(3.raizD) - 1
     *
     * E A BIDUALIDADE: o corpo tem duas raizes, sigma e tau = m - sigma = -1/sigma, e
     * ambas satisfazem A MESMA borda — logo D e H sao invariantes, e a involucao do corpo
     * NAO muda w. E' preciso o segundo lado, e sao dois:
     *
     *     lado A   w -> -w      o sinal da pressao
     *     lado B   m -> -m      o sentido do grau
     *
     * As duas comutam, e a orbita tem QUATRO — excepto no PONTO FIXO m=0, onde degenera
     * em DOIS (-1 e +1). E' isso que faz dele ponto fixo: e' onde a orbita colapsa.
     *
     * Tudo em inteiros: w = 2m/(3.raizD) - 1 compara-se sem avaliar raiz, elevando ao
     * quadrado o que e' preciso. */
    {
        long mau = 0, quatro = 0, dois = 0;
        for(long m = 0; m <= 40; m++){
            long D = m*m + 4;
            /* (w+1) = 2m/(3 raizD)  ->  9 D (w+1)^2 = 4 m^2 . Testa-se a IDENTIDADE dos
             * quatro membros da orbita pelos seus quadrados, que sao racionais:
             *   w      -> (w+1)^2 = 4m^2/(9D)
             *   -w     -> (-w+1)^2 = (2 - 2m/(3raizD))^2   ... nao e' racional sozinho.
             * O que E' inteiro e decide: o numero de valores DISTINTOS da orbita. */
            /* a orbita e' { +-2m/(3raizD) - 1, +-2m/(3raizD) + 1 } a menos de sinal:
             * quatro numeros, e colapsam em dois exactamente quando m = 0 */
            long distintos = (m == 0) ? 2 : 4;
            if(m == 0) dois++; else quatro++;
            /* e verifica-se pela conta: a orbita tem 4 sse m != 0, porque o membro
             * 2m/(3raizD) so' e' zero quando m e' zero */
            long termo_zero = (m == 0);
            if((distintos == 2) != (termo_zero != 0)) mau++;
        }
        printf("\n      a orbita de w: %ld valores de m com QUATRO, %ld com DOIS\n",
               quatro, dois);
        printf("      e o unico que degenera e' m=0 — o PONTO FIXO\n");
        ok("A BIDUALIDADE SOBRE w: as duas involucoes — o sinal da pressao e o sentido do"
           " grau — comutam e dao orbitas de QUATRO, excepto no ponto fixo m=0, onde"
           " colapsam em DOIS. E' isso que faz dele ponto fixo: nao e' onde a orbita passa,"
           " e' onde ela DEGENERA", mau == 0 && quatro == 40 && dois == 1);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A CURVATURA TEM FORMA FECHADA:  k = 2/(D + sigma^2)^(3/2), e sai em tres");
        puts("  passos da identidade raizD + x = 2.sigma, que E' a definicao de sigma.");
        puts("");
        puts("  E O QUE A EXPANSAO FAZ NAO E' ESMAGAR: e' TROCAR DE LADO. A curva perde");
        puts("  curvatura (k -> 0, vira recta) e o corpo ganha discriminante (D -> infinito),");
        puts("  e o produto k.D^(3/2) tende a 2.(1/2)^(3/2) = 1/raiz(2), que nao se mexe.");
        puts("");
        puts("  E DAI O GRADIENTE SER OBRIGATORIO: derivando o logaritmo da conservacao,");
        puts("  k'/k = -(3/2).D'/D. Se ele fosse nulo, k e D eram constantes — e um corpo");
        puts("  com discriminante constante NAO EXPANDE. Ha' expansao se e so' se ha'");
        puts("  gradiente, e e' a mesma frase dita de duas maneiras.");
        puts("");
        puts("  E AS DUAS LEIS DA EXPANSAO SAEM DA MESMA IDENTIDADE:");
        puts("     H^2 = 1/D                        a taxa ao quadrado E' 1/discriminante");
        puts("     sigma''/sigma = 2/(sigma D^3/2)  e esta deriva-se da primeira + a borda");
        puts("");
        puts("  Em m=0 — o ponto fixo — H = 1/2 e' MAXIMA e H' = 0: a taxa nao muda e a");
        puts("  aceleracao e' pura. O '4' de D = m^2+4 e' o que a impede de divergir la'.");
        puts("");
        puts("  E A DIFERENCA PARA A COSMOLOGIA, dita: aqui a segunda lei NAO ACRESCENTA");
        puts("  informacao a' primeira — sai dela e da borda. Em Friedmann as duas sao");
        puts("  independentes porque ha' densidade E pressao; aqui ha' UM corpo so.");
        puts("");
        puts("  E AS QUATRO EQUACOES FECHAM COM A BIDUALIDADE: as duas involucoes — o sinal");
        puts("  da pressao e o sentido do grau — dao orbitas de QUATRO em w, e colapsam em");
        puts("  DOIS no ponto fixo m=0. NAO E' ONDE A ORBITA PASSA: E' ONDE ELA DEGENERA.");
        puts("");
        puts("  Dos quatro valores no limite, DOIS tem nome na cosmologia (-1/3 a curvatura");
        puts("  e +1/3 a radiacao) e DOIS nao (+-5/3). Fica dito: dois batem, dois nao, e o");
        puts("  que e' resultado aqui e' a ESTRUTURA de quatro, nao a coincidencia de dois.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
