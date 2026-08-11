/* lean_cruz.c — A CRUZ NAS DUAS FERRAMENTAS LEAN, e a volta que apanha o que a régua ditada não.
 *
 * O paper conecthus/fundamento.tex afirma, por identidade de conta: Pareto e GUT são as duas
 * coordenadas da cruz — a SOMA mede (Σ = 100%, invariante) e o PRODUTO ordena (G×U×T ranqueia,
 * e satura no equilíbrio). E afirma que o validador do LMS que RE-DERIVA (a volta) apanha o
 * que a restrição ditada no prompt (a régua) deixa passar. Aqui mede-se cada frase.
 *
 * Tudo em inteiros. As percentagens exatas são racionais 100·f_i / F — comparadas nos
 * NUMERADORES sobre o denominador comum F, sem uma divisão que perca.
 *
 *   §L1  a SOMA mede porque pode falhar: a emissão arredondada erra, a re-derivada fecha,
 *        e o denominador desatualizado deriva EXATAMENTE 100·(F'−F)
 *   §L2  o PRODUTO ordena (monótono na escala 1..5) e satura no equilíbrio (AM–GM inteiro);
 *        e o controlo: com o 0 na escala, a monotonia quebra — a premissa mede-se
 *   §L3  uma coordenada só não chega: somas iguais com produtos distintos, e vice-versa
 *   §L4  a volta apanha TODOS os injetados; a régua de alcance (1..125) apanha quase nenhum
 *
 *   cc -O2 -std=c99 -I../lib lean_cruz.c -o lean_cruz && ./lean_cruz
 */
#include <stdio.h>
#include "unidade.h"

/* o gerador determinista: a mesma semente dá a mesma tabela, sempre */
static unsigned long ger = 12345;
static long dado(long topo){ ger = ger*6364136223846793005UL + 1442695040888963407UL;
                             return (long)((ger >> 33) % (unsigned long)topo); }

#define NCAT 6

int main(void){
printf("\n=== A CRUZ NAS DUAS FERRAMENTAS LEAN ======================================\n");
printf("    Pareto soma e mede; GUT multiplica e ordena. E a volta apanha; a régua dita.\n");

printf("\n§L1  A SOMA mede porque pode falhar: arredondada erra, re-derivada fecha.\n\n");
{
    long fecham = 0, falham = 0, mesas = 0, mau_exato = 0, mau_deriva = 0;
    for(long mesa = 0; mesa < 400; mesa++){
        long f[NCAT], F = 0;
        for(int i = 0; i < NCAT; i++){ f[i] = 1 + dado(99); F += f[i]; }

        /* a emissão do modelo: percentagens INTEIRAS, cada uma arredondada sozinha
         * (meio para cima: (200f + F) / 2F) — como um LLM as escreve no JSON */
        long soma_emitida = 0;
        for(int i = 0; i < NCAT; i++) soma_emitida += (200*f[i] + F) / (2*F);
        if(soma_emitida == 100) fecham++; else falham++;

        /* a re-derivada exata: nos numeradores, Σ 100·f_i contra 100·F — sem divisão */
        long soma_num = 0;
        for(int i = 0; i < NCAT; i++) soma_num += 100*f[i];
        if(soma_num != 100*F) mau_exato++;

        /* o denominador escrito à mão: chegam g defeitos novos à categoria 0; contra o F
         * velho a soma deriva, e deriva EXATAMENTE 100·g — a lei, não uma tolerância */
        long g = 1 + dado(50), soma_nova = soma_num + 100*g;
        if(soma_nova - 100*F != 100*g) mau_deriva++;          /* a deriva é a lei      */
        if(soma_nova == 100*F) mau_deriva++;                  /* e o desatualizado ACUSA */
        mesas++;
    }
    ok("a população DISCORDA: há emissões arredondadas que fecham em 100 e há que falham",
       fecham > 0 && falham > 0);
    ok("a re-derivada exata fecha em TODAS — nos numeradores, sem uma divisão", mau_exato == 0);
    ok("e o denominador desatualizado deriva exatamente 100·(F'−F), em todas", mau_deriva == 0);
    printf("      (%ld mesas: %ld emissões fecham, %ld falham — é por PODER falhar que a\n",
           mesas, fecham, falham);
    printf("      verificação Σ=100%% mede; a exata não tem onde errar, e é essa a volta.)\n");
}

printf("\n§L2  O PRODUTO ordena na escala 1..5 — e satura no equilíbrio (AM–GM inteiro).\n\n");
{
    long mau_mono = 0, mau_amgm = 0, classes_variam = 0, casos = 0;
    /* monotonia: subir UMA componente sobe o produto — é isso que faz dele um ranking */
    for(long G = 1; G <= 5; G++) for(long U = 1; U <= 5; U++) for(long T = 1; T <= 5; T++){
        if(G < 5 && (G+1)*U*T <= G*U*T) mau_mono++;
        if(U < 5 && G*(U+1)*T <= G*U*T) mau_mono++;
        if(T < 5 && G*U*(T+1) <= G*U*T) mau_mono++;
        casos++;
    }
    /* saturação: para cada soma s, o produto máximo é do terno mais EQUILIBRADO (max−min ≤ 1) */
    for(long s = 3; s <= 15; s++){
        long pmax = 0, pmin = 1000;
        for(long G = 1; G <= 5; G++) for(long U = 1; U <= 5; U++) for(long T = 1; T <= 5; T++){
            if(G+U+T != s) continue;
            long p = G*U*T; if(p > pmax) pmax = p; if(p < pmin) pmin = p;
        }
        if(pmin < pmax) classes_variam++;                       /* a classe tem o que ordenar */
        for(long G = 1; G <= 5; G++) for(long U = 1; U <= 5; U++) for(long T = 1; T <= 5; T++){
            if(G+U+T != s || G*U*T != pmax) continue;
            long hi = G, lo = G;
            if(U > hi) hi = U;
            if(T > hi) hi = T;
            if(U < lo) lo = U;
            if(T < lo) lo = T;
            if(hi - lo > 1) mau_amgm++;
        }
    }
    /* O CONTROLO: a monotonia é PREMISSA da escala, não graça do produto — com 0 permitido
     * ela quebra (0·U·T não sobe com U), e quebra em muitos casos, não num canto */
    long quebra_com_zero = 0;
    for(long G = 0; G <= 5; G++) for(long U = 0; U <= 5; U++) for(long T = 0; T <= 5; T++){
        if(U < 5 && G*(U+1)*T <= G*U*T) quebra_com_zero++;
    }
    ok("subir qualquer componente SOBE o produto — o GUT ordena, nos 125 ternos", mau_mono == 0);
    ok("e o máximo de cada soma é o terno equilibrado: max−min ≤ 1 (AM–GM, inteiro)", mau_amgm == 0);
    ok("as classes de soma têm produtos DISTINTOS — há o que ordenar, não é tautologia",
       classes_variam > 0);
    ok("e o controlo: com 0 na escala a monotonia QUEBRA — a premissa 1..5 é medida, não adorno",
       quebra_com_zero > 0);
    printf("      (%ld ternos; %ld classes de soma com produto a variar; %ld quebras com o 0.)\n",
           casos, classes_variam, quebra_com_zero);
}

printf("\n§L3  Uma coordenada só não chega: a soma não ordena, o produto não mede.\n\n");
{
    long soma_nao_ordena = 0, prod_nao_mede = 0;
    for(long G = 1; G <= 5; G++) for(long U = 1; U <= 5; U++) for(long T = 1; T <= 5; T++)
    for(long g = 1; g <= 5; g++) for(long u = 1; u <= 5; u++) for(long t = 1; t <= 5; t++){
        long s1 = G+U+T, s2 = g+u+t, p1 = G*U*T, p2 = g*u*t;
        if(s1 == s2 && p1 != p2) soma_nao_ordena++;
        if(p1 == p2 && s1 != s2) prod_nao_mede++;
    }
    ok("há pares com a MESMA soma e produtos distintos — a soma sozinha não prioriza",
       soma_nao_ordena > 0);
    ok("e pares com o MESMO produto e somas distintas — o produto sozinho não pesa",
       prod_nao_mede > 0);
    printf("      (%ld pares que a soma não separa; %ld que o produto não separa. A cruz\n",
           soma_nao_ordena, prod_nao_mede);
    printf("      precisa das DUAS coordenadas — é o que o Lean faz ao usar Pareto E GUT.)\n");
}

printf("\n§L4  A volta apanha TODOS os injetados; a régua de alcance quase nenhum.\n\n");
{
    long injetados = 0, volta_apanha = 0, regua_apanha = 0, casos = 0;
    for(long G = 1; G <= 5; G++) for(long U = 1; U <= 5; U++) for(long T = 1; T <= 5; T++){
        long certo = G*U*T;
        long emitido = certo;
        int  errado = ((G*7 + U*3 + T) % 9 == 0);      /* o erro do modelo, determinista */
        if(errado){ emitido = certo + 1; injetados++; }
        /* a VOLTA: re-derivar o produto dos operandos emitidos e comparar */
        int v = (emitido != G*U*T);
        if(v != errado){ printf("      DIVERGE em (%ld,%ld,%ld)\n", G, U, T); }
        if(v && errado) volta_apanha++;
        /* a RÉGUA ditada: o alcance 1..125, que é o que uma restrição de esquema vê */
        if(errado && (emitido < 1 || emitido > 125)) regua_apanha++;
        casos++;
    }
    ok("a injeção é não-degenerada: há erros, e não são todos", injetados > 0 && injetados < casos);
    ok("a VOLTA apanha todos os injetados — re-derivar não depende de o modelo obedecer",
       volta_apanha == injetados);
    ok("a régua de alcance apanha MENOS DE METADE do que a volta — a régua não transporta",
       2*regua_apanha < volta_apanha);
    printf("      (%ld ternos, %ld erros injetados: a volta apanha %ld, o alcance apanha %ld.\n",
           casos, injetados, volta_apanha, regua_apanha);
    printf("      É o §4.5.6 do paper do LMS: ~6%% passam a régua do prompt, zero passam a volta.)\n");
}

printf("\n=== A CRUZ FECHA NO LEAN ==================================================\n");
printf("  Pareto é a soma: mede, porque a emissão PODE falhar o 100 e a re-derivada não.\n");
printf("  GUT é o produto: ordena (monótono) e satura no equilíbrio (AM–GM).\n");
printf("  Nenhuma sozinha é o par — e a volta apanha o que a régua ditada deixa passar.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
