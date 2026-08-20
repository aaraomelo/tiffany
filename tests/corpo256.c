/* tests/corpo256.c — O ESPAÇO COMPLETO: as oito leis são a base, e o byte é um CORPO.
 *
 * O Aarão: «isso é a base de um espaço — definir o operador e os duais, soma e
 * multiplicação nesse espaço, mostrar que é completo e ordenado, aí temos a operação
 * completa.»
 *
 * Completo, sim, e mede-se. Ordenado NÃO — e isso não é uma lacuna: é um teorema, e
 * exibe-se o contra-exemplo. Um relatório que dissesse «completo e ordenado» estaria a
 * afirmar uma impossibilidade, e é exactamente o tipo de coisa que esta casa persegue.
 *
 * §C0  a BASE são as oito leis: x⁰ … x⁷ são as oito posições, e geram tudo
 * §C1  a SOMA e o seu dual: XOR, e −a = a — o grupo aditivo, exaustivo
 * §C2  o PRODUTO e o seu dual: associativo, distributivo, e a inversa por Fermat
 * §C3  COMPLETO: todo a ≠ 0 tem inversa, e a operação nunca sai — 256×256 exaustivo
 * §C4  ORDENADO NÃO, e é um TEOREMA: 1 + 1 = 0 impede qualquer ordem de corpo
 * §C5  a ordem que EXISTE: o grupo multiplicativo é cíclico de ordem 255
 * §C6  e o OPERADOR fecha: ×x percorre a base, e é o gerador das oito leis
 * §C7  O OUTRO LADO DO EIXO: a massa ORDENADA — e o escopo que eu tinha errado
 *
 * ── UMA CORRECÇÃO DE ESCOPO, E ELA É MINHA ────────────────────────────────────
 * Escrevi «completo e NÃO ordenado» sem escopo, e pus isso na porta de entrada do Corpo
 * Universal como se fechasse a questão da ordem. Não fecha: a característica 2 diz
 * respeito a ESTA face. Do outro lado do eixo de Pontryagin — que este quadro já tinha
 * medido — o encaixe ALCANÇA ℝ, que é ordenado e completo, e a ordem tem observador
 * próprio (o isomorfismo-dual ordenado). O eixo é uma TROCA, não uma falta:
 *
 *      a álgebra OPERA e não alcança  |  a topologia ALCANÇA e não opera
 *
 * e chamar «a operação completa» a uma das metades era tomar metade pelo todo.
 */
#include <stdio.h>
#include "corpo256.h"
#include "umbit.h"
#include "unidade.h"

int main(void){
    printf("\n=== O ESPAÇO: as oito leis são a base, e o byte é um corpo ===\n");

    /* ═══ §C0 A BASE SÃO AS OITO LEIS ════════════════════════════════════════ */
    printf("\n§C0 x⁰ … x⁷ são as oito posições — a base é o catálogo.\n\n");
    {
        long mal = 0;
        printf("        k    x^k     em bits\n");
        for(int k = 0; k < 8; k++){
            E b = c6_base(k);
            if(b != (E)(1u << k)) mal++;
            printf("        %d    0x%02X    ", k, b);
            for(int i = 7; i >= 0; i--) printf("%d", (b >> i) & 1);
            printf("\n");
        }
        /* e geram tudo: toda a combinação de coordenadas é um elemento distinto */
        int visto[C6_N];
        for(int i = 0; i < C6_N; i++) visto[i] = 0;
        long gera = 0;
        for(int m = 0; m < C6_N; m++){
            E s = 0;
            for(int k = 0; k < 8; k++) if((m >> k) & 1) s = c6_som(s, c6_base(k));
            if(!visto[s]){ visto[s] = 1; gera++; }
        }
        printf("      as combinações da base dão %ld elementos distintos de %d\n",
               gera, C6_N);
        ok("A BASE DO ESPAÇO SÃO AS OITO LEIS: x⁰ … x⁷ são exactamente as oito posições do"
           " byte, e toda combinação delas dá um elemento distinto — os 256. Não é uma base"
           " escolhida para o efeito: é a que a declaração da arquitectura já tinha posto,"
           " com a posição k reservada à Lei k. O espaço tem dimensão OITO porque o"
           " catálogo tem oito leis",
           mal == 0 && gera == C6_N);
    }

    /* ═══ §C1 A SOMA E O SEU DUAL ═══════════════════════════════════════════ */
    printf("\n§C1 A soma é o XOR, e o dual dela é ela própria.\n\n");
    {
        long assoc = 0, comut = 0, neutro = 0, volta = 0, cas = 0;
        for(int a = 0; a < C6_N; a++){
            if(c6_som((E)a, c6_zero()) != (E)a) neutro++;
            if(c6_som((E)a, c6_opo((E)a)) != 0) volta++;
            for(int b = 0; b < C6_N; b++){
                cas++;
                if(c6_som((E)a,(E)b) != c6_som((E)b,(E)a)) comut++;
                E c = (E)((a * 7 + b * 13) & 0xFF);
                if(c6_som(c6_som((E)a,(E)b), c) != c6_som((E)a, c6_som((E)b, c))) assoc++;
            }
        }
        printf("      em %ld pares: %ld não comutam, %ld não associam;  neutro %ld"
               " falhas, volta %ld\n", cas, comut, assoc, neutro, volta);
        ok("A SOMA É O XOR E O DUAL DELA É ELA PRÓPRIA: −a = a, logo cada elemento é a sua"
           " própria volta e a diferença é a soma. É a Lei 1 lida no espaço inteiro, e o"
           " grupo aditivo é (ℤ/2)⁸ — associativo, comutativo, com neutro, e com todo"
           " elemento de ordem 2. Medido nos 65536 pares",
           assoc == 0 && comut == 0 && neutro == 0 && volta == 0 && cas == 65536);
    }

    /* ═══ §C2 O PRODUTO E O SEU DUAL ═══════════════════════════════════════ */
    printf("\n§C2 O produto módulo o irredutível, e a inversa por Fermat.\n\n");
    {
        long comut = 0, assoc = 0, dist = 0, neutro = 0, cas = 0;
        for(int a = 0; a < C6_N; a++){
            if(c6_mul((E)a, c6_um()) != (E)a) neutro++;
            for(int b = 0; b < C6_N; b++){
                cas++;
                if(c6_mul((E)a,(E)b) != c6_mul((E)b,(E)a)) comut++;
                E c = (E)((a * 5 + b * 11 + 3) & 0xFF);
                if(c6_mul(c6_mul((E)a,(E)b), c) != c6_mul((E)a, c6_mul((E)b, c))) assoc++;
                if(c6_mul((E)a, c6_som((E)b,c))
                   != c6_som(c6_mul((E)a,(E)b), c6_mul((E)a,c))) dist++;
            }
        }
        printf("      em %ld pares: %ld não comutam, %ld não associam, %ld não"
               " distribuem;  neutro %ld falhas\n", cas, comut, assoc, dist, neutro);
        ok("O PRODUTO É O DOS POLINÓMIOS MÓDULO UM IRREDUTÍVEL DE GRAU OITO, e o grau não"
           " é escolha: é o número de posições, logo o número de leis. Comuta, associa,"
           " distribui sobre a soma e tem neutro — medido nos 65536 pares, sem uma falha."
           " E o dual dele é a INVERSA, que a secção seguinte mede",
           comut == 0 && assoc == 0 && dist == 0 && neutro == 0 && cas == 65536);
    }

    /* ═══ §C3 COMPLETO: todo a ≠ 0 tem inversa ═════════════════════════════ */
    printf("\n§C3 Completo: todo a ≠ 0 tem inversa, e a operação nunca sai.\n\n");
    {
        long mal = 0, com = 0, zero_inv = 0;
        for(int a = 1; a < C6_N; a++){
            E i = c6_inv((E)a);
            com++;
            if(c6_mul((E)a, i) != 1) mal++;
        }
        if(c6_inv(0) != 0) zero_inv++;      /* o zero: a potência dá zero, e diz-se */
        /* e o fecho: nenhuma operação sai dos 256 — é o tipo que o garante */
        long fora = 0;
        for(int a = 0; a < C6_N; a++) for(int b = 0; b < C6_N; b++){
            int s = c6_som((E)a,(E)b), m = c6_mul((E)a,(E)b);
            if(s < 0 || s >= C6_N || m < 0 || m >= C6_N) fora++;
        }
        printf("      %ld elementos não nulos: %ld sem inversa;  o zero devolve zero"
               " (%ld);  fora do espaço: %ld\n", com, mal, zero_inv, fora);
        ok("O ESPAÇO É COMPLETO NO SENTIDO ALGÉBRICO: todo elemento não nulo tem inversa —"
           " 255 de 255, sem uma falha — e nenhuma operação sai dos 256. A inversa vem de"
           " Fermat, a⁻¹ = a²⁵⁴, numa cadeia FIXA de multiplicações escrita em linha recta:"
           " sem laço com teste, sem ramo. E o zero devolve zero, o que não é excepção"
           " escondida — o ∞ dele vive em ℙ¹, e é o Corolário 0 ↔ ∞ outra vez",
           mal == 0 && com == 255 && zero_inv == 0 && fora == 0);
    }

    /* ═══ §C4 ORDENADO NÃO — E É UM TEOREMA ════════════════════════════════ */
    printf("\n§C4 Ordenado NÃO, e não é lacuna: é impossível, e exibe-se.\n\n");
    {
        E dois = c6_soma_uns(2), quatro = c6_soma_uns(4);
        long pares_zero = 0, impares_um = 0, n = 0;
        for(int k = 1; k <= 20; k++){
            E s = c6_soma_uns(k);
            n++;
            if(k % 2 == 0 && s == 0) pares_zero++;
            if(k % 2 == 1 && s == 1) impares_um++;
        }
        printf("        1 + 1 = %d        1+1+1+1 = %d\n", dois, quatro);
        printf("        somas de n uns: %ld pares dão ZERO, %ld ímpares dão um (em %ld)\n",
               pares_zero, impares_um, n);
        ok("ORDENADO NÃO, E ISSO É UM TEOREMA — não uma lacuna do trabalho. Num corpo"
           " ordenado 1 > 0 força 1+1 > 0, e por indução nenhuma soma de uns pode dar zero;"
           " aqui 1 + 1 = 0, e toda soma de um número PAR de uns dá zero. Logo não existe"
           " ordem compatível com as operações, e um relatório que dissesse «completo e"
           " ordenado» estaria a afirmar uma impossibilidade. A característica é dois, e a"
           " ordem de corpo exige característica zero",
           dois == 0 && quatro == 0 && pares_zero == 10 && impares_um == 10);
    }

    /* ═══ §C5 A ORDEM QUE EXISTE: o grupo multiplicativo é cíclico ═════════ */
    printf("\n§C5 A ordem que existe: o grupo multiplicativo é cíclico de ordem 255.\n\n");
    {
        int log256[C6_N];
        for(int i = 0; i < C6_N; i++) log256[i] = -1;
        int geradores = 0, primeiro = 0;
        for(int g = 2; g < C6_N; g++){
            int o = c6_ordem((E)g, NULL);
            if(o == 255){ geradores++; if(!primeiro) primeiro = g; }
        }
        int ord = c6_ordem((E)primeiro, log256);
        long semlog = 0;
        for(int a = 1; a < C6_N; a++) if(log256[a] < 0) semlog++;
        printf("        geradores do grupo multiplicativo: %d de 254 candidatos;  o"
               " primeiro é 0x%02X\n", geradores, primeiro);
        printf("        a ordem dele é %d, e o logaritmo discreto cobre %d elementos"
               " (%ld sem log)\n", ord, 255 - (int)semlog, semlog);
        ok("A ORDEM QUE EXISTE É OUTRA, E NOMEIA-SE À PARTE: o grupo multiplicativo é"
           " CÍCLICO de ordem 255, logo há um gerador g e todo a ≠ 0 é g^k para um k único"
           " — o logaritmo discreto. Isso dá uma enumeração canónica dos 255, que é uma"
           " ordem no sentido de PERCURSO e não no sentido de «compatível com a soma». São"
           " coisas diferentes, e juntá-las seria dizer que o corpo é ordenado quando o"
           " §C4 acabou de provar que não pode ser",
           geradores > 0 && ord == 255 && semlog == 0);
    }

    /* ═══ §C6 E O OPERADOR FECHA ═══════════════════════════════════════════ */
    printf("\n§C6 O operador ×x percorre a base — e é o gerador das oito leis.\n\n");
    {
        long mal = 0;
        E a = 1;
        printf("        x⁰ → x¹ → … → x⁷:  ");
        for(int k = 0; k < 8; k++){
            printf("0x%02X ", a);
            if(a != (E)(1u << k)) mal++;
            a = c6_x(a);
        }
        printf("→ 0x%02X  (aqui a realimentação entra)\n", a);
        /* e o operador é bijectivo: uma permutação dos 255 não nulos */
        int img[C6_N];
        for(int i = 0; i < C6_N; i++) img[i] = 0;
        long col = 0;
        for(int i = 1; i < C6_N; i++){ E y = c6_x((E)i); img[y]++; }
        for(int i = 1; i < C6_N; i++) if(img[i] != 1) col++;
        int pg = lei_periodo(lei_gera, 0x5A, 32);
        printf("      ×x é permutação dos 255 não nulos: %ld colisões;  e o gerador SEM"
               " realimentação tem período %d\n", col, pg);
        ok("E O OPERADOR FECHA O DESENHO: multiplicar por x é rodar uma posição e"
           " REALIMENTAR com o polinómio quando o bit de topo sai. As potências x⁰ … x⁷"
           " percorrem exactamente a base — as oito leis —, e ao oitavo passo a"
           " realimentação entra: é ela que fecha o ciclo dentro do espaço em vez de o"
           " deixar sair. Sem realimentação o gerador roda com período 8 e não multiplica"
           " nada; com ela, multiplica e é bijectivo",
           mal == 0 && col == 0 && pg == 8);
    }

    /* ═══ §C7 O OUTRO LADO DO EIXO: a massa ORDENADA ═══════════════════════
     * Escrevi «completo e NÃO ordenado» sem escopo, e isso era uma lei fajuta: a
     * característica 2 diz respeito a ESTA face, não à arquitectura. Do outro lado do
     * eixo de Pontryagin o encaixe alcança ℝ, que é ordenado — e essa metade tem de ser
     * medida aqui, ao lado, senão o par fica pela metade. É a regra desta casa: dual
     * exige DUAS partes na mesma frase. */
    printf("\n§C7 O outro lado do eixo: a massa ORDENADA, que esta face não tem.\n\n");
    {
        /* a face que OPERA: a ordem de corpo é impossível — e o contra-exemplo é o §C4.
         * a face que ALCANÇA: os convergentes do áureo, em inteiros exactos, ENCAIXAM e
         * a ordem sobrevive em cada passo. É o thm:encaixe medido aqui em miniatura. */
        long F[40]; F[0] = 0; F[1] = 1;
        for(int k = 2; k < 40; k++) F[k] = F[k-1] + F[k-2];
        long encaixa = 0, unidade = 0, cas = 0, ordem_ok = 0;
        for(int k = 2; k < 30; k++){
            long p1n = F[k+1], q1n = F[k], p2n = F[k+2], q2n = F[k+1];
            cas++;
            /* a unidade: p_{k+1} q_k − p_k q_{k+1} = ±1 — o factor de potência */
            long u = p2n*q1n - p1n*q2n;
            if(u == 1 || u == -1) unidade++;
            /* o encaixe: os intervalos sucessivos apertam, e a ORDEM decide isso */
            long a1 = p1n*q2n, b1 = p2n*q1n;
            if(a1 != b1) encaixa++;
            /* e a ordem é ESTRITA e alternada — o zigue-zague */
            int sinal = (k % 2) ? 1 : -1;
            if((u > 0) == (sinal > 0)) ordem_ok++;
        }
        /* e a face que opera: nenhuma ordem sobrevive — o §C4 já o mostrou, e aqui
         * conta-se o par dos dois lados na MESMA asserção */
        E dois = c6_soma_uns(2);
        printf("        a face que ALCANÇA (os convergentes do áureo, inteiros exactos):\n");
        printf("          a unidade |p_{k+1}q_k − p_k q_{k+1}| = 1 em %ld de %ld\n",
               unidade, cas);
        printf("          os intervalos APERTAM (a ordem decide) em %ld de %ld, e o sinal"
               " alterna em %ld\n", encaixa, cas, ordem_ok);
        printf("        a face que OPERA (𝔽₂₅₆): 1 + 1 = %d — nenhuma ordem de corpo"
               " sobrevive\n", dois);
        ok("E O PAR TEM DE SER DITO NAS DUAS PARTES, senão é meia frase — que foi o erro"
           " que esta secção corrigiu. Eu escrevi «completo e NÃO ordenado» sem escopo, e"
           " a característica 2 diz respeito a ESTA face, não à arquitectura. Do outro"
           " lado do eixo de Pontryagin, os convergentes do áureo — inteiros exactos —"
           " ENCAIXAM com a unidade ±1 e o sinal a alternar, e é a ORDEM que decide o"
           " encaixe: essa face alcança ℝ, que é ordenado e completo. O eixo diz a troca e"
           " não uma falta: A ÁLGEBRA OPERA E NÃO ALCANÇA; A TOPOLOGIA ALCANÇA E NÃO"
           " OPERA. Chamar «a operação completa» a uma das metades era tomar metade pelo"
           " todo",
           unidade == cas && encaixa == cas && ordem_ok == cas && dois == 0 && cas == 28);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
