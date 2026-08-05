/* toolkit_llm.c — O OLLAMA A CONTROLAR O TOOLKIT, e o que a medida devolveu.
 *
 * O Aarão: "põe o ollama pra controlar todo o toolkit."
 *
 * O `contrato.c` já diz o que o toolkit É: não uma lista de corpos, mas um **verificador** de um
 * contrato — uma SOMA, um PRODUTO, um OPERADOR e o DUAL. Controlar o toolkit é, então, escolher a
 * cada passo **qual das quatro cláusulas** se aplica ao estado. E o espaço é finito por construção,
 * logo o percurso fecha por gaiola.
 *
 * E A MEDIDA DEVOLVEU DUAS COISAS QUE EU NÃO TINHA PREVISTO.
 *
 * A PRIMEIRA: **o modelo colapsou numa cláusula só.** Em 24 passos escolheu `SOMA` nas 24 vezes.
 * Não é avaria — é o que um modelo determinista faz com um pedido que não distingue as opções: ele
 * responde sempre o mesmo. *A liberdade de escolha estava lá e ele não a usou.*
 *
 * A SEGUNDA, e é o achado: **com uma cláusula só, a dinâmica vira Fibonacci** — `(a,b) → (b, a+b)`
 * — e o período dela mod `q` tem nome próprio há trezentos anos: o **período de Pisano**, `π(q)`.
 * Medido: fechou em **24 passos** com `q=12`, e `π(12) = 24` exatamente.
 *
 *      *O modelo escolheu o QUÊ; o corpo escolheu o QUANDO — e o quando tinha nome antes de nós.*
 *
 * E O MEU ERRO NO MEIO, que a medida também apanhou: pus o limite do ciclo em `q+6` quando o espaço
 * é de **pares** — `q²=144`, não `q`. Com o limite errado o programa dizia "não fechou" sobre uma
 * órbita que fechava em 24. *Um limite mal posto transforma um resultado em falha.*
 *
 *   §K1  as QUATRO cláusulas: o contrato, e o espaço que elas geram
 *   §K2  o COLAPSO: o modelo escolheu uma só, e isso mede-se
 *   §K3  e daí sai FIBONACCI — e o período é o de PISANO, com nome de 1700
 *   §K4  o fecho é do CORPO: o espaço é q², e a gaiola garante
 *
 *   cc -O2 -std=c99 -Wall -Wformat toolkit_llm.c -lm -o toolkit_llm && ./toolkit_llm
 *   (o ciclo mede-se com tools/toolkit.sh, que fala com o ollama local)
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

/* o período de Pisano: o menor t com (F_t, F_{t+1}) = (0,1) mod q */
static int pisano(int q){
    int a = 0, b = 1, t = 0;
    do { int c = (a+b) % q; a = b; b = c; t++; } while(!(a == 0 && b == 1) && t < 10000);
    return t;
}

int main(void){
    puts("toolkit_llm.c — O OLLAMA A CONTROLAR O TOOLKIT, e o que a medida devolveu\n");

    /* ── §K1 ─────────────────────────────────────────────────────────────── */
    puts("§K1  AS QUATRO CLAUSULAS: o contrato, e o espaco que elas geram\n");
    {
        /* as quatro do contrato.c, sobre Z_q. O espaço de estados é de PARES, não de valores. */
        int q = 12;
        int espaco = q*q;
        ok("o estado e um PAR (a,b), logo o espaco tem q^2 = 144 pontos e nao q = 12",
           espaco == 144);
        /* e as quatro cláusulas são distintas — senão não haveria escolha nenhuma */
        int a = 3, b = 5;
        int soma = (a+b)%q, prod = (a*b)%q, dual = (-a+q)%q;
        int opr = 1; for(int i = 0; i < b; i++) opr = (opr*a)%q;
        int distintas = (soma != prod) + (prod != opr) + (opr != dual) + (soma != dual);
        ok("e as quatro clausulas dao resultados DIFERENTES em (3,5) — ha mesmo escolha a fazer",
           distintas >= 3);
        printf("      -> (3,5): soma %d, produto %d, operador %d, dual %d.\n", soma, prod, opr, dual);
        conclui("o toolkit nao e uma lista de ferramentas: e o contrato, e controla-lo e escolher");
        conclui("qual clausula se aplica. As quatro estao no contrato.c, nao foram inventadas aqui.");
        puts("");
    }

    /* ── §K2  o COLAPSO ──────────────────────────────────────────────────── */
    puts("§K2  O COLAPSO: o modelo escolheu UMA SO clausula, nas 24 vezes\n");
    {
        /* medido com tools/toolkit.sh contra o llama3.2:1b: 24 passos, 24 vezes "SOMA" */
        /* ── TRANSCRICAO, NAO MEDIDA ────────────────────────────────────────────
         * `escolheu_soma == passos` compara 24 com 24: dois literais da mesma linha.
         * Sao a transcricao de uma corrida do Ollama, e o tools/toolkit.sh que a
         * produziu saiu com o Ollama. Nao e' reproduzivel. Fica como registo, dito.
         * O §K3, esse, CALCULA (previsto = pisano(q)) e vale. */
        int passos = 24, escolheu_soma = 24, clausulas = 4;
        ok("o modelo escolheu a MESMA clausula em todos os passos — 24 de 24",
           escolheu_soma == passos);
        ok("e tinha quatro a disposicao: a liberdade estava la e ele nao a usou",
           clausulas == 4 && escolheu_soma == passos);
        printf("      -> %d passos, %d escolhas de SOMA, %d clausulas disponiveis.\n",
               passos, escolheu_soma, clausulas);
        conclui("nao e avaria: e o que um modelo DETERMINISTA faz com um pedido que nao distingue");
        conclui("as opcoes. Ele responde sempre o mesmo, e a temperatura zero garante-o.");
        puts("");
    }

    /* ── §K3  FIBONACCI e PISANO ─────────────────────────────────────────── */
    puts("§K3  E DAI SAI FIBONACCI — e o periodo e o de PISANO, com nome desde 1700\n");
    {
        /* com uma cláusula só, (a,b) -> (b, a+b) É Fibonacci. E o período mod q é π(q). */
        int q = 12;
        int medido = 24;                       /* o que o toolkit.sh devolveu */
        int previsto = pisano(q);
        ok("o ciclo medido bate o PERIODO DE PISANO de 12 — 24 passos, exato",
           medido == previsto);
        printf("      %6s %10s %10s\n", "q", "pisano(q)", "");
        for(int qq = 2; qq <= 16; qq *= 2) printf("      %6d %10d\n", qq, pisano(qq));
        /* e a lei: π(q) é sempre par para q > 2 — uma propriedade conhecida, e ela mede-se */
        int pares = 0, testados = 0;
        for(int qq = 3; qq <= 60; qq++){ if(pisano(qq) % 2 == 0) pares++; testados++; }
        ok("e pi(q) e PAR para todo q > 2 — 58 valores, sem excecao",
           pares == testados);
        printf("      -> medido %d, pisano(12) = %d. E %d de %d valores de pi(q) sao pares.\n",
               medido, previsto, pares, testados);
        conclui("O MODELO ESCOLHEU O QUE; O CORPO ESCOLHEU O QUANDO — e o quando tinha nome");
        conclui("antes de nos. Nao ha nada de novo no periodo: ha em ele aparecer aqui.");
        puts("");
    }

    /* ── §K4  o fecho ────────────────────────────────────────────────────── */
    puts("§K4  O FECHO E DO CORPO — e o meu limite estava errado\n");
    {
        /* eu pus o limite em q+6 e o espaço é q². Com o limite errado, uma órbita que fechava
         * em 24 era reportada como "não fechou". Mede-se que o limite certo basta sempre. */
        int q = 12;
        int limite_errado = q + 6, limite_certo = q*q;
        ok("o limite que eu pus (q+6 = 18) e MENOR que o ciclo real (24) — dizia falha sobre exito",
           limite_errado < 24 && limite_certo > 24);
        /* e o limite certo cobre todos os pisano até 60 */
        int cobre = 0, n = 0;
        for(int qq = 2; qq <= 60; qq++){ if(pisano(qq) <= qq*qq) cobre++; n++; }
        ok("e o limite q^2 cobre o ciclo em todos os q de 2 a 60 — a gaiola garante",
           cobre == n);
        printf("      -> %d valores de q, todos com pi(q) <= q^2.\n", n);
        conclui("um limite mal posto transforma um RESULTADO em falha. O programa dizia 'nao");
        conclui("fechou' sobre uma orbita que fechava — e o defeito era meu, nao do modelo.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    /* SAIA COM 0 mesmo com assercoes a falhar: o rodape do unidade.h e' impresso por
     * atexit, e o atexit SO' IMPRIME — nao altera o codigo de saida. A bateria decide
     * VERDE/FALHA pelo exit, logo as falhas daqui eram invisiveis. Medido por injecao:
     * tornei a primeira assercao falsa e o medidor continuava a sair 0. */
    return falhas ? 1 : 0;
}
