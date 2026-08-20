/* tests/projetiva.c — ZERO E INFINITO SÃO INVERSOS: o corolário sem caso especial.
 *
 * Esta casa repete «0⁻¹ NÃO EXISTE — a fibra é vazia» em todo andar, como se fosse A
 * excepção. Via Möbius não é excepção nenhuma: em coordenadas homogéneas a inversão é uma
 * TROCA, [p:q] ↦ [q:p], e então 1/0 = ∞ e 1/∞ = 0.
 *
 * O que se mede aqui é isso, e sobretudo o que ele CUSTA — porque um corolário que só
 * ganha e não paga costuma estar mal contado.
 *
 * §J0  a inversão é TOTAL e involutiva em ℙ¹, e 0 ↔ ∞
 * §J1  e o código não tem um `if`: o «não dá» era da carta, não do objecto
 * §J2  ν(x) = −1/x: período DOIS em ℙ¹ e QUATRO no par — a diferença é o sinal
 * §J3  o GATO atravessa o infinito: A_m leva 0 ↦ ∞ ↦ m, e a órbita só fecha em ℙ¹
 * §J4  O PREÇO: a soma deixa de ser total — a excepção MUDA DE SÍTIO
 * §J5  o gume: com det = 0 a Möbius colapsa ℙ¹ e deixa de ser bijecção
 */
#include <stdio.h>
#include "projetiva.h"
#include "unidade.h"

/* percorre uma amostra de ℙ¹: os [p:q] reduzidos com |p|,|q| ≤ R, mais o ∞ */
#define PJ_R 40

int main(void){
    printf("\n=== ZERO E INFINITO SÃO INVERSOS ===\n");

    /* ═══ §J0 A INVERSÃO É TOTAL E INVOLUTIVA, e 0 ↔ ∞ ════════════════════════ */
    printf("\n§J0 A inversão é uma TROCA: [p:q] ↦ [q:p].\n\n");
    {
        long mal = 0, casos = 0, sem_inversa = 0;
        for(long p = -PJ_R; p <= PJ_R; p++) for(long q = -PJ_R; q <= PJ_R; q++){
            Pj x;
            if(!pj(p,q,&x)) continue;               /* só o (0,0) cai fora */
            casos++;
            Pj i = pj_inverte(x);
            Pj v = pj_inverte(i);
            if(!pj_igual(v, x)) mal++;              /* involução: 1/(1/x) = x */
            /* e nenhum ponto fica sem inversa — a fibra NUNCA é vazia */
            Pj chk;
            if(!pj(i.p, i.q, &chk)) sem_inversa++;
        }
        Pj z = pj_zero(), inf = pj_infinito();
        int zi = pj_igual(pj_inverte(z), inf);
        int iz = pj_igual(pj_inverte(inf), z);
        printf("      1/0 = ∞ ?  %s        1/∞ = 0 ?  %s\n", zi ? "sim" : "NÃO",
               iz ? "sim" : "NÃO");
        printf("      involução em %ld pontos de ℙ¹: %ld divergências;  sem inversa:"
               " %ld\n", casos, mal, sem_inversa);
        ok("ZERO E INFINITO SÃO INVERSOS, E A INVERSÃO É TOTAL EM ℙ¹. Em ℚ a casa dizia"
           " «0⁻¹ não existe, a fibra é vazia», e repetia-o em todo andar como se fosse A"
           " excepção. Não era uma verdade sobre o objecto: era sobre a carta. Na recta"
           " projectiva [p:q] a inversão é a TROCA [q:p], é involutiva, e nenhum ponto"
           " fica sem inversa — nem o zero, que passa a ter o infinito",
           mal == 0 && sem_inversa == 0 && zi && iz && casos > 3000);
    }

    /* ═══ §J1 E O CÓDIGO NÃO TEM UM `if` ═════════════════════════════════════ */
    printf("\n§J1 A prova mais barata: a função não tem onde perguntar «é zero?».\n\n");
    {
        /* a inversão em ℚ precisa do teste; em ℙ¹ não há divisão para testar.
         * Mede-se comparando as duas nos pontos onde AMBAS estão definidas. */
        long mal = 0, comuns = 0, so_projectiva = 0;
        for(long p = -PJ_R; p <= PJ_R; p++) for(long q = 1; q <= PJ_R; q++){
            Pj x;
            if(!pj(p,q,&x)) continue;
            Pj i = pj_inverte(x);
            if(p == 0){ so_projectiva++; continue; }   /* em ℚ isto não existia */
            comuns++;
            /* em ℚ: 1/(p/q) = q/p, com o sinal a subir para o numerador */
            long np = q, nq = p;
            if(nq < 0){ np = -np; nq = -nq; }
            Pj r;
            if(!pj(np,nq,&r) || !pj_igual(i,r)) mal++;
        }
        printf("      onde ℚ e ℙ¹ coincidem (%ld pontos): %ld divergências\n",
               comuns, mal);
        printf("      e os pontos que só ℙ¹ tem: %ld (o zero, cuja inversa é ∞)\n",
               so_projectiva);
        ok("E O ARGUMENTO MAIS BARATO É O CÓDIGO: `pj_inverte` é `r.p = x.q; r.q = x.p;` —"
           " não divide, logo não tem onde perguntar se o denominador é zero. Em ℚ o `if`"
           " era obrigatório porque a operação era uma divisão; em ℙ¹ a operação é uma"
           " TROCA. Onde as duas cartas se sobrepõem dão o mesmo, e a projectiva ainda"
           " responde onde a outra se calava — que é a definição de a carta ser melhor, e"
           " não de o objecto ter mudado",
           mal == 0 && comuns > 0 && so_projectiva > 0);
    }

    /* ═══ §J2 O PERÍODO É DOIS EM ℙ¹ E QUATRO NO PAR ════════════════════════ */
    printf("\n§J2 ν(x) = −1/x: dois em ℙ¹, quatro no par — e o zero não é excepção.\n\n");
    {
        long mal2 = 0, casos = 0, par_mal4 = 0, par_cedo = 0, pcasos = 0;
        for(long p = -PJ_R; p <= PJ_R; p++) for(long q = -PJ_R; q <= PJ_R; q++){
            Pj x;
            if(!pj(p,q,&x)) continue;
            casos++;
            /* em ℙ¹: ι² = id — período DOIS, porque [−p:−q] é o mesmo ponto */
            if(!pj_igual(pj_nu(pj_nu(x)), x)) mal2++;
            /* no PAR, sem projectivizar: período QUATRO, e não menos */
            if(p == 0 && q == 0) continue;
            Par y = { p, q };
            pcasos++;
            Par a = pj_nu_par(y), b = pj_nu_par(a), c = pj_nu_par(b), d = pj_nu_par(c);
            if(!pj_par_igual(d, y)) par_mal4++;
            if(pj_par_igual(b, y)) par_cedo++;      /* não pode fechar ao segundo */
        }
        Pj z = pj_zero();
        Pj n1 = pj_nu(z), n2 = pj_nu(n1), n3 = pj_nu(n2), n4 = pj_nu(n3);
        printf("      a órbita do ZERO por ν: [%d:%d] → [%d:%d] → [%d:%d] → [%d:%d] →"
               " [%d:%d]\n", z.p,z.q, n1.p,n1.q, n2.p,n2.q, n3.p,n3.q, n4.p,n4.q);
        printf("      em ℙ¹:  ι² = id em %ld pontos, %ld divergências — período DOIS\n",
               casos, mal2);
        printf("      no PAR: ν⁴ = id em %ld pares, %ld divergências, e %ld fecham cedo"
               " — período QUATRO\n", pcasos, par_mal4, par_cedo);
        ok("O PERÍODO É DOIS EM ℙ¹ E QUATRO NO PAR, e a diferença é exactamente o SINAL."
           " Escrevi quatro nos dois e a medição desmentiu-me: ι² = id em 6560 de 6560,"
           " porque [−p:−q] é o MESMO ponto projectivo que [p:q] — a projectivização mata"
           " o sinal. No par, sem projectivizar, o passo [p:q] ↦ [−q:p] fecha em quatro e"
           " não antes: é o quarto de volta, «o bit é i». É a mesma frase do Teorema do"
           " Gato noutro sítio — a medida não vê o sinal —, e aqui é ℙ¹ que não o vê. E o"
           " que importa para o corolário vale nos dois: a órbita do ZERO passa pelo"
           " infinito e volta, sem que o programa tenha um ramo para isso",
           mal2 == 0 && par_mal4 == 0 && par_cedo == 0 && casos > 3000 && pcasos > 3000);
    }

    /* ═══ §J3 O GATO ATRAVESSA O INFINITO ════════════════════════════════════ */
    printf("\n§J3 A_m como Möbius leva 0 ↦ ∞ ↦ m: a órbita só fecha em ℙ¹.\n\n");
    {
        long mal = 0, passou = 0, n = 0;
        printf("        m   0 ↦ ?      ∞ ↦ ?     e a órbita 0 → ∞ → m → …\n");
        for(long m = 1; m <= 5; m++){
            Pj z = pj_zero(), a, b, c;
            n++;
            if(!pj_gato(m, z, &a)){ mal++; continue; }
            if(!pj_gato(m, pj_infinito(), &b)){ mal++; continue; }
            if(!pj_gato(m, a, &c)){ mal++; continue; }
            if(pj_e_infinito(a)) passou++;          /* 0 vai mesmo para ∞ */
            printf("        %-3ld [%d:%d]      [%d:%d]     0 → [%d:%d] → [%d:%d]\n",
                   m, a.p,a.q, b.p,b.q, a.p,a.q, c.p,c.q);
            if(!pj_igual(c, b)) mal++;              /* a(0) = ∞, e daí vai para m */
        }
        /* e uma órbita longa, que em ℚ teria parado no infinito */
        Pj x = pj_zero();
        long passos = 0, tocou = 0;
        for(int k = 0; k < 12; k++){
            Pj y;
            if(!pj_gato(2, x, &y)) break;
            if(pj_e_infinito(y)) tocou++;
            x = y; passos++;
        }
        printf("      e a órbita de 0 pelo gato A₂, 12 passos: %ld deram, e passou pelo"
               " ∞ %ld vez(es)\n", passos, tocou);
        ok("O GATO ATRAVESSA O INFINITO, E A CASA ESCREVIA AO LADO QUE ELE NÃO EXISTIA."
           " A_m lida como Möbius é x ↦ (m·x + 1)/x: aplicada a 0 dá ∞, e aplicada a ∞ dá"
           " m. O objecto central desta casa PRECISA do ponto que a casa declarava"
           " inexistente — e a órbita a partir do zero só fecha em ℙ¹. Em ℚ ela morria ao"
           " primeiro passo, e nós chamávamos a isso a fibra vazia",
           mal == 0 && passou == n && passos == 12 && tocou == 1);
    }

    /* ═══ §J4 O PREÇO: a soma deixa de ser total ═════════════════════════════ */
    printf("\n§J4 O preço: a excepção MUDA DE SÍTIO, não desaparece.\n\n");
    {
        Pj inf = pj_infinito(), z = pj_zero(), r;
        int soma_inf = pj_soma(inf, inf, &r);       /* ∞ + ∞ → [0:0]: recusa */
        int mult_zi  = pj_mult(z, inf, &r);         /* 0 · ∞ → [0:0]: recusa */
        int soma_ok  = pj_soma(pj_um(), pj_um(), &r);
        long recusas = 0, boas = 0, casos = 0;
        for(long p = -12; p <= 12; p++) for(long q = -12; q <= 12; q++)
        for(long s = -12; s <= 12; s++) for(long t = -12; t <= 12; t++){
            Pj a, b, c;
            if(!pj(p,q,&a) || !pj(s,t,&b)) continue;
            casos++;
            if(pj_soma(a,b,&c)) boas++; else recusas++;
        }
        printf("      ∞ + ∞: %s        0 · ∞: %s        1 + 1: %s\n",
               soma_inf ? "dá (mau)" : "RECUSA", mult_zi ? "dá (mau)" : "RECUSA",
               soma_ok ? "dá" : "recusa (mau)");
        printf("      a soma em %ld pares de ℙ¹: %ld definidas, %ld recusadas\n",
               casos, boas, recusas);
        ok("E O PREÇO DIZ-SE, SENÃO O COROLÁRIO ESTAVA MAL CONTADO: ℙ¹ NÃO é um corpo. A"
           " inversão torna-se total, e a SOMA deixa de o ser — ∞ + ∞ e 0·∞ não têm valor,"
           " porque dariam [0:0], que não é ponto. A excepção não foi abolida: foi MUDADA"
           " DE SÍTIO. E o sítio novo é o certo, porque lá ela é consequência da"
           " construção — [0:0] é o único par excluído, e é excluído por definição — em"
           " vez de ser um ramo escrito à mão em cada função que divide",
           !soma_inf && !mult_zi && soma_ok && recusas > 0 && boas > 0);
    }

    /* ═══ §J5 O GUME: det = 0 colapsa ℙ¹ ═════════════════════════════════════ */
    printf("\n§J5 O gume: sem det ≠ 0 a Möbius deixa de ser bijecção.\n\n");
    {
        long recusou = 0, deg = 0, aceitou = 0, boas = 0;
        for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
        for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
            Pj x = pj_um(), r;
            int ok_ = pj_mobius(a,b,c,d,x,&r);
            if(a*d - b*c == 0){ deg++; if(!ok_) recusou++; }
            else { boas++; if(ok_) aceitou++; }
        }
        /* e com |det| = 1 — o Gato — a acção é bijecção de ℙ¹: mede-se a INJECTIVIDADE */
        long colisoes = 0, vistos = 0;
        {
            /* PRIMEIRO deduplicar a FONTE: [1:2] e [2:4] são o MESMO ponto, e contá-los
             * duas vezes fazia a «colisão» medir a minha enumeração e não a injectividade.
             * Foi o que aconteceu à primeira: 438 colisões em 200 «pontos distintos». */
            #define PJ_N 400
            Pj fonte[PJ_N], img[PJ_N]; int nf = 0;
            for(long p = -7; p <= 7 && nf < PJ_N; p++) for(long q = -7; q <= 7 && nf < PJ_N; q++){
                Pj x;
                if(!pj(p,q,&x)) continue;
                int novo = 1;
                for(int i = 0; i < nf; i++) if(pj_igual(fonte[i], x)) novo = 0;
                if(novo) fonte[nf++] = x;
            }
            int ni = 0;
            for(int k = 0; k < nf; k++){
                Pj y;
                if(!pj_gato(2, fonte[k], &y)) continue;
                for(int i = 0; i < ni; i++) if(pj_igual(img[i], y)) colisoes++;
                img[ni++] = y; vistos++;
            }
        }
        printf("      det = 0: %ld matrizes, %ld recusadas;  det ≠ 0: %ld, %ld aceites\n",
               deg, recusou, boas, aceitou);
        printf("      e o gato A₂ em %ld pontos distintos: %ld colisões — é injectiva\n",
               vistos, colisoes);
        ok("E O GUME ESTÁ ONDE TEM DE ESTAR: a acção de Möbius só é uma bijecção de ℙ¹"
           " quando det ≠ 0 — com det = 0 ela colapsa a recta inteira num ponto, e a"
           " função recusa em todas. Com |det| = 1, que é a lei do Teorema do Gato, ela é"
           " bijecção E preserva o reticulado: nenhuma colisão nos pontos distintos"
           " testados. Logo o corolário não flutua sozinho — está pendurado na mesma"
           " condição que o teorema",
           recusou == deg && aceitou == boas && colisoes == 0 && vistos > 50);
    }

    printf("\n=== %d asserções, %d falhas, %ld estouros ===\n",
           unidades, falhas, pj_estouros);
    return falhas ? 1 : 0;
}
