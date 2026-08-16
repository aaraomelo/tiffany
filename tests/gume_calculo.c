/* gume_calculo.c — O GUME AUTOMÁTICO A CORRER, E ELE ESTAVA ESCRITO SEM NUNCA TER CORRIDO.
 *
 * `lib/calculo.h` traz `fn_gume`, e o comentário dela diz exactamente o que é preciso:
 *
 *     «retirar a hipótese e PROCURAR o contra-exemplo. Precisa de DOIS controlos: um
 *      regime onde tem de achar e outro onde tem de voltar vazio — senão "não achou"
 *      não distingue a lei do buscador partido.»
 *
 * A função estava lá, com o comentário certo, e nenhum medidor a chamava. Um buscador de
 * contra-exemplos que nunca buscou nada é a forma mais silenciosa de asserção vazia: ele
 * não falha, não aparece, e a metodologia que ele serve fica por cumprir.
 *
 * ── O QUE SE MEDE, E POR QUE SÃO SEMPRE DOIS ────────────────────────────────────
 * A tese de um teorema com hipótese tem duas metades, e uma sem a outra não vale:
 *
 *      COM a hipótese      o gume volta VAZIO      — a lei é verdadeira
 *      SEM a hipótese      o gume ACHA             — a hipótese trabalha
 *
 * Se só se medisse a primeira, um buscador partido — que nunca acha nada — daria o mesmo
 * resultado. Se só se medisse a segunda, não se saberia se a lei vale. É o par.
 *
 * ── ROLLE, QUE É O CASO MAIS CURTO ──────────────────────────────────────────────
 * Para f = ax² + bx + c em [0,1]:
 *
 *      hip:  f(0) = f(1)  ⟺  c = a+b+c  ⟺  b = −a
 *      então f′ = 2ax + b = a(2x − 1), com raiz em x = 1/2, que está em (0,1)   SEMPRE
 *
 * e sem a hipótese o contra-exemplo é imediato: f = x² tem f′ = 2x, cuja única raiz é 0,
 * que está FORA do intervalo aberto. A hipótese não é decoração — é o que põe a raiz lá
 * dentro.
 *
 *   §G1  ROLLE: com a hipótese o gume volta vazio; sem ela, ACHA — e exibe-se
 *   §G2  O VALOR MÉDIO: a mesma forma, com a inclinação da corda no lugar do zero
 *   §G3  E O BUSCADOR ESTÁ VIVO: uma tese FALSA de propósito, que ele tem de apanhar
 *
 * Tudo em ℚ exacto (Qz). Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib gume_calculo.c -o gume_calculo && ./gume_calculo
 */
#include <stdio.h>
#include "racionais.h"
#include "calculo.h"
#include "unidade.h"

/* ── as hipóteses e as teses, na assinatura que fn_gume pede ───────────────────── */

/* a raiz de um polinómio de grau ≤ 1 está em (0,1)? — exacto, sem procurar */
static int raiz_no_aberto(Cf g){
    if(g.n == 0) return g.c[0].p == 0;          /* g ≡ 0: todo ponto é raiz */
    if(g.n != 1) return 0;
    Qz r;
    if(!qz_divide(qz_oposto(g.c[0]), g.c[1], &r)) return 0;
    return qz_menor(qz(0,1), r) && qz_menor(r, qz(1,1));
}
static int hip_rolle(Cf f){                      /* f(0) = f(1) */
    return qz_igual(fn_av(f, qz(0,1)), fn_av(f, qz(1,1)));
}
static int hip_livre(Cf f){ (void)f; return 1; } /* a hipótese RETIRADA */
static int tese_rolle(Cf f){                     /* f′ anula-se em (0,1) */
    return raiz_no_aberto(fn_deriva(f));
}
/* o Valor Médio: f′(c) = f(1) − f(0), que é a inclinação da corda em [0,1] */
static int tese_vm(Cf f){
    Qz s = qz_soma(fn_av(f, qz(1,1)), qz_oposto(fn_av(f, qz(0,1))));
    Cf g = fn_deriva(f);
    g.c[0] = qz_soma(g.c[0], qz_oposto(s));      /* f′ − s, e procura-se a raiz */
    fn_ajusta(&g);
    return raiz_no_aberto(g);
}
static int tese_falsa(Cf f){ (void)f; return 0; } /* nunca vale: o buscador TEM de achar */

int main(void){
    printf("\n=== O GUME AUTOMÁTICO A CORRER — e ele nunca tinha corrido ===\n");

    /* ═══ §G1  ROLLE, COM OS DOIS CONTROLOS ═════════════════════════════════ */
    printf("\n§G1 Rolle: com a hipótese o gume volta vazio; sem ela, acha.\n\n");
    {
        Cf c1, c2;
        long com = fn_gume(4, hip_rolle, tese_rolle, &c1);
        long sem = fn_gume(4, hip_livre, tese_rolle, &c2);
        printf("      hipótese f(0) = f(1)   → contra-exemplos: %ld   (a lei vale)\n", com);
        printf("      hipótese RETIRADA      → contra-exemplos: %ld   (ela trabalha)\n", sem);
        if(sem) printf("      e o primeiro exibe-se: f = %d/%d·x² %+d/%d·x %+d/%d\n",
                       c2.c[2].p, c2.c[2].q, c2.c[1].p, c2.c[1].q, c2.c[0].p, c2.c[0].q);
        /* E O NÚMERO PREVÊ-SE, que é o que torna isto uma medida e não um «achou alguma
         * coisa». A tese não depende de c, logo tudo se conta em (a,b) e multiplica por 9:
         *
         *      a = 0 e b ≠ 0    f′ = b constante não nula: sem raiz      8·9  =  72
         *      a ≠ 0            −b/2a fora de (0,1)                     48·9  = 432
         *                                                              ─────────────
         *                                                                       504
         *
         * Escrever `sem > 0` deixaria passar um gume partido que achasse dois; o número
         * exacto não deixa. */
        long prev_a0 = 0, prev_an = 0;
        for(long a2 = -4; a2 <= 4; a2++) for(long b2 = -4; b2 <= 4; b2++){
            if(a2 == 0){ if(b2 != 0) prev_a0++; continue; }
            /* −b/2a ∈ (0,1) ? em inteiros: 0 < −b/(2a) < 1 */
            long num = -b2, den = 2*a2;
            long dentro = (den > 0) ? (num > 0 && num < den) : (num < 0 && num > den);
            if(!dentro) prev_an++;
        }
        long previsto = (prev_a0 + prev_an) * 9;
        printf("      e o número PREVÊ-SE: %ld (a=0, b≠0) + %ld (a≠0, raiz fora) vezes 9"
               " = %ld\n", prev_a0, prev_an, previsto);
        ok("ROLLE PASSA PELO GUME NOS DOIS SENTIDOS, E É PRECISO OS DOIS: com a hipótese"
           " f(0) = f(1) o buscador volta VAZIO — logo a lei é verdadeira em toda a"
           " varredura —, e retirando-a ele ACHA, com o contra-exemplo exibido. Uma medida"
           " sem a outra não valia: se só se medisse a primeira, um buscador partido daria"
           " o mesmo resultado; se só a segunda, não se saberia se a lei vale. E a razão é"
           " curta — com b = −a a derivada é a(2x−1), cuja raiz é 1/2 e está dentro; sem a"
           " hipótese, f = x² tem a raiz da derivada em 0, que está FORA do aberto. E o"
           " número que ele acha PREVÊ-SE: a tese não depende de c, logo conta-se em (a,b)"
           " e multiplica-se por 9 — 72 com a = 0 e b ≠ 0, mais 432 com a ≠ 0 e a raiz fora"
           " do aberto, que dá 504. Escrever «achou mais que zero» deixaria passar um gume"
           " partido que achasse dois",
           com == 0 && sem == previsto && previsto == 504
           && cl_estouros == 0 && qz_saturou == 0);
    }

    /* ═══ §G2  O VALOR MÉDIO ════════════════════════════════════════════════ */
    printf("\n§G2 O Valor Médio: a mesma forma, com a corda no lugar do zero.\n\n");
    {
        /* Rolle é o Valor Médio com corda nula. Aqui a tese é f′(c) = f(1) − f(0), e a
         * hipótese de Rolle deixa de ser precisa — o teorema vale para TODO polinómio de
         * grau 2. Logo o gume tem de voltar vazio mesmo com a hipótese RETIRADA, e é isso
         * que distingue os dois teoremas: um precisa da hipótese, o outro não. */
        Cf c1, c2;
        long com = fn_gume(4, hip_rolle, tese_vm, &c1);
        long sem = fn_gume(4, hip_livre, tese_vm, &c2);
        printf("      hipótese f(0) = f(1)   → contra-exemplos: %ld\n", com);
        printf("      hipótese RETIRADA      → contra-exemplos: %ld   (não precisa dela)\n", sem);
        ok("E O VALOR MÉDIO NÃO PRECISA DA HIPÓTESE DE ROLLE, o que o gume mostra pela"
           " diferença: com a corda no lugar do zero, o buscador volta vazio nos DOIS"
           " regimes — com a hipótese e sem ela. Rolle é o caso de corda nula, e é aí que"
           " f(0) = f(1) faz falta; no enunciado geral a corda absorve-a. Que o mesmo"
           " buscador dê vazio aqui e ACHE no §G1 é o que separa um teorema que precisa da"
           " hipótese de um que não precisa",
           com == 0 && sem == 0 && cl_estouros == 0);
    }

    /* ═══ §G3  E O BUSCADOR ESTÁ VIVO ═══════════════════════════════════════ */
    printf("\n§G3 O controlo do próprio buscador: uma tese FALSA, que ele tem de apanhar.\n\n");
    {
        /* «Não achou» só vale se «achar» for possível. Dá-se-lhe uma tese que é falsa em
         * todo o lado, e conta-se: ele tem de achar TODOS os polinómios varridos. Sem
         * este controlo, os zeros do §G1 e do §G2 não distinguiriam a lei de um buscador
         * que nunca busca. */
        Cf c;
        long todos = fn_gume(4, hip_livre, tese_falsa, &c);
        long esperado = 9L*9*9;                   /* lim = 4: (2·4+1)³ combinações */
        printf("      tese sempre falsa      → contra-exemplos: %ld   (esperado %ld)\n",
               todos, esperado);
        long filtrado = fn_gume(4, hip_rolle, tese_falsa, &c);
        printf("      e com a hipótese de Rolle a filtrar: %ld   (esperado %ld)\n",
               filtrado, 9L*9);
        printf("      — e o 81 não é um número solto: f(0) = f(1) é b = −a, que tira"
               " exactamente UM grau\n        de liberdade aos três coeficientes, logo"
               " 9³ passa a 9²\n");
        ok("E O BUSCADOR ESTÁ VIVO, QUE É O CONTROLO SEM O QUAL OS ZEROS NÃO VALEM: dada"
           " uma tese falsa em todo o lado, ele acha TODOS os polinómios varridos — as"
           " (2·4+1)³ combinações de coeficientes. É isto que faz dos zeros do §G1 e do §G2"
           " uma afirmação sobre a LEI e não sobre o buscador. «Não achou» só significa"
           " alguma coisa depois de se mostrar que «achar» é possível. E o número que a"
           " hipótese deixa passar também se prevê: f(0) = f(1) é b = −a, que tira UM grau"
           " de liberdade aos três coeficientes, logo 9³ = 729 passa a 9² = 81 — medido, e"
           " não uma contagem de cabeça",
           todos == esperado && filtrado == 81);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  O gume estava escrito, com o comentário certo, e nunca tinha corrido.\n");
        printf("  Um buscador de contra-exemplos que nunca buscou é a asserção vazia\n");
        printf("  mais silenciosa: não falha, não aparece, e a metodologia que ele\n");
        printf("  serve fica por cumprir. Agora corre — com os DOIS controlos que o\n");
        printf("  próprio comentário exigia, e com o terceiro que prova que ele busca.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
