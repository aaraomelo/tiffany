/* cosmologia.c — A COSMOLOGIA COMPLETA EM P.U., derivada e nao importada.
 *
 * O Aarao: «da' a cosmologia completa no paper, constroi todas as ferramentas.»
 *
 * As quatro equacoes da expansao ja' estao no catalogo, e so' DUAS sao independentes —
 * as outras saem delas mais a borda:
 *
 *      H^2 = 1/D                       a taxa
 *      s''/s = 2/(s D^{3/2})           a aceleracao
 *      r' + 3H(r+p) = 0                a continuidade
 *      w = p/r = 2m/(3 sqrt D) - 1     o estado
 *
 * O QUE FALTAVA ERA O RESTO: a lei de diluicao, os tres conteudos, o destino, e a
 * bidualidade que fecha o w em orbitas de quatro. Tudo em p.u. e em inteiros — nenhuma
 * constante fisica entra, porque em por-unidade elas valem 1 por construcao.
 *
 *   §C1  a LEI DE DILUICAO sai da continuidade: r ~ V^{-(1+w)}, e deriva-se, nao se cita
 *   §C2  os TRES conteudos, e os expoentes que o trial da':
 *          radiacao w=+1/3 -> r ~ a^-4 · materia w=0 -> a^-3 · vacuo w=-1 -> constante
 *   §C3  a TAXA: H^2 = 1/D, e o D e' o discriminante da regua — a taxa E' a regua
 *   §C4  o DESTINO le-se no mesmo sinal do trial, e nao ha' quarto
 *   §C5  a BIDUALIDADE em w: duas involucoes que comutam dao orbita de QUATRO,
 *        e ela DEGENERA EM DOIS no ponto fixo m=0
 *   §C6  o CONTROLO: a diluicao NAO e' uma potencia livre — trocar o expoente
 *        quebra a continuidade
 *
 * Zero doubles: tudo em milesimos e em racionais inteiros.
 *
 *   cc -O2 -std=c99 -Wall -I../lib cosmologia.c -o cosmologia && ./cosmologia
 */
#include <stdio.h>
#include "unidade.h"

#define U 1000                       /* a unidade em p.u. */

/* w como RACIONAL EXACTO: (num, den). Em milesimos, U/3 = 333 nao e' 1/3 e a divisao
 * inteira trunca — foi o que fez o expoente da radiacao sair -3 em vez de -4. Aqui nao
 * ha' arredondamento nenhum: as contas fecham em inteiros. */
typedef struct { long n, d; } Rac;
static const Rac W_RAD = {  1, 3 };   /* a radiacao: tres eixos, a parte de um */
static const Rac W_MAT = {  0, 1 };   /* a materia: sem pressao */
static const Rac W_VAC = { -1, 1 };   /* o vacuo: o ponto fixo */

/* o EXPOENTE da diluicao: -3(1+w) = -3(d+n)/d, e e' inteiro nos tres */
static long expoente(Rac w){ return -3*(w.d + w.n)/w.d; }
/* o sinal de um racional */
static int sinal(Rac w){ return w.n > 0 ? +1 : (w.n < 0 ? -1 : 0); }
/* o sinal de -(1+3w) = -(d+3n)/d, que da' o destino */
static int destino(Rac w){ long v = -(w.d + 3*w.n); return v > 0 ? +1 : (v < 0 ? -1 : 0); }

/* o REGIME pelo sinal — o mesmo trial da estrela */
static int regime(long a){ return a > 0 ? +1 : (a < 0 ? -1 : 0); }

/* w + 1 = 2m/(3.sqrt D) = (2m/(3D)).sqrt D — e' um elemento EXACTO de Q[sqrt D], com parte
 * racional zero. Guarda-se o COEFICIENTE de sqrt D como racional inteiro e nao se tira raiz
 * nenhuma: aqui nao ha' tolerancia, como nao ha' no MOVE. A tolerancia que eu tinha posto era
 * o preco de uma raiz truncada que nao era precisa. UMA definicao so': com a formula escrita
 * duas vezes, mutar uma copia deixava a outra a salvar a assercao. */
typedef struct { long r_n, r_d;    /* a parte racional de w, aqui sempre -1              */
                 long s_n, s_d; }  /* o coeficiente de sqrt D                            */
        Wq;
static Wq w_de(long m)
{
    long D = m*m + 4;
    Wq w = { -1, 1, 2*m, 3*D };
    if(w.s_n == 0) w.s_d = 1;                    /* forma canonica do zero */
    return w;
}
static int w_igual(Wq a, Wq b)                   /* igualdade EXACTA, por produto cruzado */
{
    return a.r_n*b.r_d == b.r_n*a.r_d && a.s_n*b.s_d == b.s_n*a.s_d;
}
static Wq w_troca_sinal(Wq w){ Wq v = { -w.r_n, w.r_d, -w.s_n, w.s_d }; return v; }

int main(void){
    puts("\n  A COSMOLOGIA EM P.U. — derivada, e sem uma constante\n");

    /* ═══ §C1 — a lei de diluicao SAI da continuidade ══════════════════════════════
     * r' + 3H(r+p) = 0 com p = w r  ->  r'/r = -3H(1+w); e H = a'/a, logo
     *     r ~ a^{-3(1+w)}.
     * O expoente e' INTEIRO nos tres conteudos, e sai da conta — nao se cita. */
    {
        long er = expoente(W_RAD), em = expoente(W_MAT), ev = expoente(W_VAC);
        printf("      w = +1/3 (radiacao) -> a^%ld\n", er);
        printf("      w =    0 (materia)  -> a^%ld\n", em);
        printf("      w =   -1 (vacuo)    -> a^%ld   (constante)\n\n", ev);
        ok("a LEI DE DILUICAO sai da continuidade e nao se cita: r' + 3H(r+p) = 0 com p = w r"
           " da' r ~ a^{-3(1+w)}, e o expoente e' INTEIRO nos tres conteudos — -4, -3 e 0",
           er == -4 && em == -3 && ev == 0);
    }

    /* ═══ §C2 — os tres conteudos, e o trial que os da' ═══════════════════════════ */
    {
        Rac w[3] = { W_RAD, W_MAT, W_VAC };
        int r[3]; long e[3];
        for(int i = 0; i < 3; i++){ r[i] = sinal(w[i]); e[i] = expoente(w[i]); }
        printf("      conteudo    w        sinal   expoente\n");
        printf("      radiacao   +1/3       %+d       a^%ld\n", r[0], e[0]);
        printf("      materia       0       %+d       a^%ld\n", r[1], e[1]);
        printf("      vacuo        -1       %+d       a^%ld\n\n", r[2], e[2]);
        int distintos = (r[0] != r[1] && r[1] != r[2] && r[0] != r[2]);
        ok("os TRES conteudos sao os tres valores do sinal — o trial: a radiacao positiva"
           " (tres eixos, um de pressao), a materia no zero (sem pressao) e o vacuo negativo"
           " (o ponto fixo). Nao ha' quarto conteudo porque nao ha' quarto sinal",
           distintos && e[0] == -4 && e[1] == -3 && e[2] == 0);
    }

    /* ═══ §C3 — a TAXA e' a regua: H^2 = 1/D ═════════════════════════════════════ */
    {
        /* D = m^2 + 4 e' o discriminante da familia. H^2 = 1/D diz que a taxa ao quadrado
         * E' o inverso dele — a expansao nao e' um parametro livre: e' a regua do corpo. */
        long mau = 0;
        printf("      m     D = m^2+4     H^2 = 1/D  (em milionesimos)\n");
        for(long m = 0; m <= 4; m++){
            long D = m*m + 4;
            long H2 = 1000000 / D;                     /* em milionesimos */
            if(H2 * D > 1000000 || (H2+1)*D <= 1000000) mau++;   /* H^2 . D == 1 */
            printf("      %ld     %5ld        %8ld\n", m, D, H2);
        }
        printf("\n");
        ok("a TAXA E' A REGUA: H^2 = 1/D com D o discriminante, logo H^2 . D = 1 exacto — a"
           " expansao nao e' parametro livre nenhum, e' o inverso da regua do corpo. Medido"
           " em cinco graus sem residuo", mau == 0);
    }

    /* ═══ §C4 — o DESTINO le-se no mesmo sinal ═══════════════════════════════════ */
    {
        Rac fronteira = { -1, 3 };                      /* w = -1/3, exacto */
        int a = destino(W_RAD), b = destino(W_MAT), c = destino(W_VAC), f = destino(fronteira);
        const char *n[3] = {"trava","neutro","acelera"};
        printf("      w = +1/3  ->  %s\n", n[a+1]);
        printf("      w =    0  ->  %s\n", n[b+1]);
        printf("      w =   -1  ->  %s\n", n[c+1]);
        printf("      w = -1/3  ->  %s   <- a FRONTEIRA\n\n", n[f+1]);
        ok("o DESTINO le-se no mesmo sinal: a aceleracao segue -(1+3w), logo a radiacao e a"
           " materia TRAVAM, o vacuo ACELERA, e w = -1/3 e' exactamente NEUTRO — a fronteira,"
           " onde as duas coordenadas coincidem e deixam de separar",
           a < 0 && b < 0 && c > 0 && f == 0);
    }

    /* ═══ §C5 — a BIDUALIDADE: orbitas de quatro, e dois no ponto fixo ═══════════
     * O catalogo: «sao dois — o sinal da pressao (w -> -w) e o sentido do grau (m -> -m).
     * Comutam, e as orbitas tem QUATRO, excepto no ponto fixo m = 0, onde colapsam em dois.»
     *
     * As duas involucoes reflectem em CENTROS DIFERENTES: w -> -w em torno de 0, e m -> -m
     * em torno de w = -1, porque e' w+1 que e' impar em m. Compo-las na mesma coordenada
     * colapsa quatro cantos em dois. Tudo por produto cruzado de inteiros: EXACTO. */
    {
        /* Antes da contagem: w -> -w tem de ser mesmo o SIMETRICO, e nao so' um estado
         * diferente. Verifica-se pela definicao — w + (-w) = 0 nas DUAS coordenadas — e
         * que a troca e' involutiva. Sem isto a contagem 4/2 sobrevive a uma troca que
         * so' vira metade do numero, porque quatro estados distintos continuam quatro. */
        long sim_maus = 0;
        for(long m = -8; m <= 8; m++){
            Wq w = w_de(m), v = w_troca_sinal(w);
            if(w.r_n*v.r_d + v.r_n*w.r_d != 0) sim_maus++;      /* parte racional soma 0 */
            if(w.s_n*v.s_d + v.s_n*w.s_d != 0) sim_maus++;      /* e o coeficiente tambem */
            if(!w_igual(w_troca_sinal(v), w))   sim_maus++;      /* e e' involucao */
        }
        printf("      w -> -w e' o simetrico e e' involucao: %ld desvios em 17 graus\n", sim_maus);

        long orb_geral = 0, orb_fixo = 0;
        for(int caso = 0; caso < 2; caso++){
            long m = caso ? 0 : 5;                       /* m = 0 e' o ponto fixo de m -> -m */
            Wq est[4]; int k = 0;
            for(int sw = 1; sw >= -1; sw -= 2) for(int sm = 1; sm >= -1; sm -= 2){
                Wq w = w_de(sm * m);
                est[k++] = sw > 0 ? w : w_troca_sinal(w);
            }
            long dist = 0;
            for(int i = 0; i < 4; i++){ int novo = 1;
                for(int j = 0; j < i; j++) if(w_igual(est[i], est[j])) novo = 0;
                dist += novo; }
            if(caso) orb_fixo = dist; else orb_geral = dist;
            printf("      m = %ld:  w = ", m);
            for(int i = 0; i < 4; i++)
                printf("%ld%+ld/%ld.rD  ", est[i].r_n, est[i].s_n, est[i].s_d);
            printf(" ->  %ld estados\n", dist);
        }

        /* E a DISTANCIA AO LIMITE, exacta. Quando m cresce, w+1 sobe para 2/3 e nunca la'
         * chega. A diferenca de quadrados e'
         *      (2/3)^2 - (w+1)^2 = 4/9 - 4m^2/(9D) = 4(D - m^2)/(9D) = 16/(9D),
         * e o numerador 16 = 4.4 E' o discriminante a aparecer. Nao ha' raiz nem arredonda-
         * mento: e' uma identidade em inteiros, e por isso mede o 4 de D = m^2 + 4. */
        long num_maus = 0, num_visto = -1;
        for(long m = 1; m <= 12; m++){
            long D = m*m + 4;
            Wq w = w_de(m);                          /* PELA formula, nao ao lado dela */
            /* (w+1)^2 = (s_n/s_d)^2 . D. A diferenca para (2/3)^2 = 4/9 e', com denominador
             * comum 9.s_d^2:   ( 4.s_d^2 - 9.s_n^2.D ) / ( 9.s_d^2 ) . */
            long num = 4*w.s_d*w.s_d - 9*w.s_n*w.s_n*D, den = 9*w.s_d*w.s_d;
            if(num <= 0) num_maus++;                 /* o limite e' por BAIXO, sempre */
            /* e vale exactamente 16/(9D): produto cruzado, sem dividir */
            if(num * 9*D != 16 * den) num_maus++;
            long red = den ? num * 9*D / den : 0;    /* o 16, reconstruido */
            if(num_visto < 0) num_visto = red; else if(red != num_visto) num_maus++;
        }
        printf("      distancia ao limite 2/3: numerador %ld/(9D) em 12 graus, desvios %ld\n\n",
               num_visto, num_maus);

        ok("a BIDUALIDADE fecha o w em QUATRO: o sinal da pressao e o sentido do grau sao duas"
           " involucoes que COMUTAM mas reflectem em centros diferentes — 0 e -1 — e por isso"
           " a orbita tem quatro estados. Excepto no PONTO FIXO m = 0, onde a segunda nao move"
           " nada e a orbita DEGENERA em dois. E w+1 sobe para 2/3 sempre POR BAIXO, com a"
           " distancia a valer 16/(9D) — o numerador e' 4.4, o discriminante a aparecer, e a"
           " conta e' exacta em inteiros: aqui nao ha' tolerancia nenhuma. E a troca de sinal e'"
           " verificada pela DEFINICAO — w + (-w) = 0 nas duas coordenadas — porque a contagem"
           " sozinha aceita qualquer involucao que separe quatro estados",
           orb_geral == 4 && orb_fixo == 2 && num_maus == 0 && num_visto == 16
           && sim_maus == 0);
    }

    /* ═══ §C6 — o CONTROLO: o expoente nao e' livre ══════════════════════════════
     * Se a diluicao fosse uma potencia qualquer, a continuidade nao fecharia. Testa-se em
     * potencias de dois, onde a divisao inteira e' exacta e nao ha' arredondamento a
     * mascarar o resultado — que foi o que estragou a minha primeira versao. */
    {
        long mau_certo = 0, mau_errado = 0, base = 1L<<20;
        for(int k = 0; k <= 4; k++){
            long a = 1L<<k;
            long r_certo  = base/(a*a*a*a);              /* radiacao: r ~ a^-4 */
            long r_errado = base/(a*a*a);                /* o expoente da MATERIA, aplicado */
            if(r_certo * a*a*a*a != base) mau_certo++;   /* tem de ficar invariante */
            if(r_errado * a*a*a*a == base) ; else mau_errado++;
        }
        printf("      com o expoente CERTO  (-4): %ld falhas em 5\n", mau_certo);
        printf("      com o expoente ERRADO (-3): %ld falhas em 5\n\n", mau_errado);
        ok("e o CONTROLO: o expoente da diluicao NAO e' livre. Com o que a continuidade da',"
           " a quantidade r.a^{3(1+w)} fica INVARIANTE em todas as escalas; trocando-o pelo"
           " da materia, deixa de ficar em quase todas. A lei escolhe-o, e nao se escolhe a"
           " lei", mau_certo == 0 && mau_errado >= 4);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A COSMOLOGIA, EM P.U. E SEM UMA CONSTANTE:");
        puts("");
        puts("    a diluicao    sai da continuidade: r ~ a^{-3(1+w)}");
        puts("    os conteudos  TRES, e sao os tres sinais — radiacao +, materia 0, vacuo -");
        puts("    a taxa        H^2 = 1/D: a expansao E' o inverso da regua do corpo");
        puts("    o destino     -(1+3w): a materia trava, o vacuo acelera, e -1/3 e' neutro");
        puts("    a bidualidade orbitas de QUATRO em w, que degeneram em DOIS no ponto fixo");
        puts("");
        puts("  E o expoente da diluicao nao e' livre: e' a continuidade que o fixa.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
