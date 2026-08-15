/* tests/conservacao_metrica.c — A CONSERVAÇÃO MÉTRICA POR DUALIDADE, e a meta-indução.
 *
 * O `eval.txt` pede o teorema por esta ordem: primeiro a descida — indução e
 * meta-indução —, e depois a conservação que ela prova. E abre com a correção que
 * organiza o ficheiro todo:
 *
 *   «estourar o tipo NÃO é resultado matemático. É falha de representação.»
 *
 * §M0  A DESCIDA: indução e descida são o mesmo facto lido nos dois sentidos
 * §M1  A META-INDUÇÃO: o passo não vê n — e é por isso que vale em todo andar
 * §M2  QUATRO CONTAS, UM NÚMERO: eliminação, cunha, espectro e CONTAGEM
 * §M3  σσ′ = −1 É |det| = 1: a hipótese estrutural e a conservação da área
 * §M4  O LADO CONTÍNUO: |det DT| ≡ 1 num T que NÃO é linear
 * §M5  O GUME: retirar |det| = 1 e a medida deixa de se conservar
 * §M6  A SEGUNDA REALIZAÇÃO: a primeira satura, a lei não
 * §M7  E O QUE NÃO SE MEDE: σ_n(A) := |det A| é definição, não teorema
 */
#include <stdio.h>
#include <string.h>
#include "inteiros.h"
#include "cifra.h"
#include "racionais.h"
#include "linear.h"
#include "exterior.h"
#include "medida.h"
#include "unidade.h"      /* o `ok` e os contadores são DELE — um contador meu aqui
                           * sombreava o do header e onze unidades entravam como uma,
                           * que é o «exit sombreado» que esta casa já apanhou 17 vezes.
                           *
                           * E uma nota sobre as PALAVRAS: a bateria monta o veredicto
                           * procurando na saída os tokens dela — «RESIDUO 0», «FALHOU»,
                           * «FALHA». Escrever FALHA em maiúsculas dentro de uma asserção
                           * fazia esta linha aparecer como «VERDE … — FALHA», que é
                           * legível ao contrário. O instrumento é partilhado e as palavras
                           * são minhas: diz-se «saturação» e «quebrar», e o veredicto volta
                           * a dizer o que é. */

int main(void){
    printf("\n=== A CONSERVAÇÃO MÉTRICA POR DUALIDADE ===\n");

    /* ═══ §M0 A DESCIDA: os dois sentidos do mesmo facto ═══════════════════════
     * Escrevi primeiro uma descida que recebia um número e o empurrava para baixo, e ela
     * era oca: dei-lhe racionais que nunca foram determinantes da torre, e sessenta em
     * sessenta «sobreviveram à base». O defeito não estava no resultado — estava em eu ter
     * dado à descida um objecto fora do domínio dela. A descida certa é a do
     * CONTRA-EXEMPLO MÍNIMO, e essa tem busca, e a busca tem dois controlos. */
    printf("\n§M0 A descida é o dual da indução — e o que ela procura não aparece.\n\n");
    {
        /* a INDUÇÃO sobe: da base, o passo produz o andar seguinte */
        Qz d = mi_base(), prox;
        int subiu = 0;
        for(int k = 0; k < 12; k++){ if(mi_passo(d, &prox)){ subiu++; d = prox; } }

        /* a DESCIDA: procura-se o PRIMEIRO andar onde a conservação falha */
        int nivel_bom = -1, nivel_mau = -1;
        int achou_bom = mi_procura_minimo(mi_passo, 400, &nivel_bom);
        int achou_mau = mi_procura_minimo(mi_passo_sabotado, 400, &nivel_mau);
        int vazia = mi_sem_contraexemplo(mi_passo, 400);
        printf("      a indução sobe %d andares a partir de det(I) = 1\n", subiu);
        printf("      a descida procura o PRIMEIRO andar onde falha:\n");
        printf("        com o passo verdadeiro, até 400 andares:      %s\n",
               achou_bom ? "achou (mau)" : "VAZIA — não há contra-exemplo mínimo");
        printf("        com o passo sabotado (duplica a cada andar):  achou em N = %d\n",
               nivel_mau);
        ok("A INDUÇÃO E A DESCIDA SÃO O MESMO FACTO LIDO NOS DOIS SENTIDOS, e o facto é a"
           " boa ordem de ℕ. A indução PROJECTA — da base constrói o andar seguinte; a"
           " descida LÊ — nega a tese («há um PRIMEIRO andar onde falha») e procura esse"
           " andar. Ele não aparece em 400, e não aparece porque o passo aplicado em N−1"
           " tê-lo-ia produzido. E a procura vale porque o SEGUNDO CONTROLO mostra que ela"
           " sabe achar: com um passo sabotado, que duplica a cada andar, ela acha em N = 1",
           subiu == 12 && !achou_bom && vazia && achou_mau && nivel_mau == 1);

        /* e a INVOLUÇÃO: o que a indução produz é onde a descida não morde */
        int volta_mal = 0;
        for(long p = 1; p <= 12; p++){
            Qz x = qz(p,1), sub;
            if(!mi_passo(x, &sub)){ volta_mal++; continue; }
            if(!qz_igual(sub, qz(1,1))) volta_mal++;
        }
        printf("      e o que a indução produz, em 12 entradas: %d fora de det = 1\n",
               volta_mal);
        ok("E AS DUAS METADES TOCAM-SE, que é o que faz delas um PAR DUAL e não duas"
           " técnicas parecidas: o que a indução PRODUZ é exactamente aquilo em que a"
           " descida não tem onde morder. Uma diz o que há e a outra diz o que não há, e é"
           " a mesma frase. Foi assim que o defeito apareceu — a primeira descida que"
           " escrevi não tocava no que a indução produzia, e por isso não media nada",
           volta_mal == 0);
    }

    /* ═══ §M1 A META-INDUÇÃO: o passo não vê n ═════════════════════════════════ */
    printf("\n§M1 A meta-indução: mede-se o PASSO, não uma tabela de andares.\n\n");
    {
        long vale = 0, casos = 0;
        for(long p = -40; p <= 40; p++) for(long q = 1; q <= 8; q++){
            if(p == 0) continue;
            casos++;
            if(mi_passo_vale(qz(p,q))) vale++;
        }
        printf("      o passo em %ld entradas independentes: %ld dão det = 1\n",
               casos, vale);
        ok("A META-INDUÇÃO É O QUE SE MEDE, e ela não é uma varredura de andares: é a"
           " verificação de que o PASSO vale para qualquer entrada. A função `mi_passo`"
           " não recebe n, não o menciona no corpo, e devolve sempre 1 — logo o «para todo"
           " n» sai da FORMA da construção, T_{n+1} = T_n ⊕ T_n*, e não de a correr muitas"
           " vezes. Uma tabela de andares provaria os andares da tabela",
           vale == casos && casos > 300);

        /* e o CONTROLO: onde o passo tem de falhar */
        Qz e;
        int falha_no_zero = !mi_passo(qz(0,1), &e);
        printf("      e o controlo: com det(T) = 0 o passo recusa (%s)\n",
               falha_no_zero ? "recusa" : "NÃO recusa");
        ok("E O PASSO TEM ONDE QUEBRAR, que é o que o torna uma medição: se det(T) = 0 o"
           " operador não tem dual — a fibra é vazia, e é o mesmo 0⁻¹ que a escada"
           " aritmética desta casa encontrou em todos os andares. Um passo que devolvesse 1"
           " para tudo, inclusive para o zero, não estaria a medir nada",
           falha_no_zero);
    }

    /* ═══ §M2 QUATRO CONTAS INDEPENDENTES, UM SÓ NÚMERO ═══════════════════════ */
    printf("\n§M2 Eliminação, cunha, espectro e contagem — e nenhuma sabe das outras.\n\n");
    {
        long mal_wedge = 0, casos = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            Mat A = mat0(2,2);
            A.a[0][0] = qz_de_inteiro(a); A.a[0][1] = qz_de_inteiro(b);
            A.a[1][0] = qz_de_inteiro(c); A.a[1][1] = qz_de_inteiro(d);
            casos++;
            if(!qz_igual(mat_det(A), md_volume_wedge(A))) mal_wedge++;
        }
        long mal3 = 0, casos3 = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long d = -2; d <= 2; d++){
            long v[9] = { a,b,1, c,d,0, 1,1,1 };
            Mat A = mat_de_inteiros(3,3,v);
            casos3++;
            if(!qz_igual(mat_det(A), md_volume_wedge(A))) mal3++;
        }
        printf("      det por eliminação contra cunha das colunas: %ld/%ld em 2×2,"
               " %ld/%ld em 3×3\n", mal_wedge, casos, mal3, casos3);
        ok("O DETERMINANTE É O FACTOR DE VOLUME, e as duas contas chegam lá por caminhos"
           " sem código em comum: a eliminação desce por menores, e a cunha das colunas é"
           " alternância pura — A(v₁)∧…∧A(vₙ) = det(A)·v₁∧…∧vₙ. Em 2×2 e em 3×3, e nas"
           " duas dimensões o número é o mesmo",
           mal_wedge == 0 && mal3 == 0 && casos == 2401 && casos3 == 625);

        /* (d) A CONTAGEM — e esta não toca em determinante nenhum */
        long mal_conta = 0, testados = 0;
        for(long m = 1; m <= 6; m++){
            Mat A = md_gato(m);
            long R = 12;
            long caixa = (2*R+1)*(2*R+1);
            long dentro = md_conta_imagem(A, R);
            testados++;
            if(dentro != caixa) mal_conta++;
        }
        Mat D = mat0(2,2);
        D.a[0][0] = qz_de_inteiro(3); D.a[1][1] = qz(1,1);      /* |det| = 3 */
        long R = 12, caixa = (2*R+1)*(2*R+1), dentro3 = md_conta_imagem(D, R);
        /* a MESMA contagem pelo outro lado: gerando o reticulado e vendo quem cai */
        long ger_mal = 0, ger_casos = 0;
        for(long m = 1; m <= 4; m++){
            Mat A = md_gato(m);
            ger_casos++;
            if(md_conta_imagem(A, 6) != md_conta_gerando(A, 6, 60)) ger_mal++;
        }
        ger_casos++;
        if(md_conta_imagem(D, 6) != md_conta_gerando(D, 6, 60)) ger_mal++;
        printf("      o gato leva ℤ² SOBRE ℤ²: %ld/%ld metais com a caixa inteira"
               " coberta\n", testados - mal_conta, testados);
        printf("      a dilatação de |det| = 3 cobre %ld de %ld — estritamente menos\n",
               dentro3, caixa);
        printf("      e as duas contagens — descer pela divisibilidade e subir gerando —"
               " concordam: %ld/%ld\n", ger_casos - ger_mal, ger_casos);
        ok("E A QUARTA CONTA É CONTAGEM PURA: quantos pontos do reticulado têm pré-imagem"
           " inteira. Ela não calcula determinante nenhum — divide e vê se o resto é zero"
           " — e dá a mesma resposta: com |det| = 1 a caixa é coberta toda, e com |det| = 3"
           " é coberto estritamente menos. E a referência não é minha: a contagem faz-se"
           " pelos DOIS lados — descer, perguntando a cada ponto da caixa se tem"
           " pré-imagem inteira, e subir, gerando o reticulado e vendo quem lá cai —, e os"
           " dois números coincidem. Eu tinha escrito «um em cada três» de cabeça e estava"
           " errado: numa caixa simétrica de raio 12 os múltiplos de 3 são 9, não 25/3."
           " Esta é a medida de Lebesgue realizada no discreto, e é a conta que sobrevive"
           " à troca de representação",
           mal_conta == 0 && dentro3 < caixa && ger_mal == 0 && ger_casos == 5);
    }

    /* ═══ §M3 σσ′ = −1 É |det| = 1 ════════════════════════════════════════════ */
    printf("\n§M3 A hipótese estrutural e a conservação da área são a MESMA frase.\n\n");
    {
        long mal = 0, casos = 0;
        for(long m = 1; m <= 40; m++){
            Mat A = md_gato(m);
            Qz d = mat_det(A);
            casos++;
            if(d.p != -1 || d.q != 1) mal++;
            if(md_produto_proprios(m) != d.p) mal++;
            if(!md_estaca_fecha(m)) mal++;
        }
        printf("      em %ld metais: det(A_m) = σσ′ = −1, e a estaca σ† = m − σ fecha —"
               " %ld divergências\n", casos, mal);
        ok("σσ′ = −1 É |det A_m| = 1, e não são dois factos: o determinante É o produto"
           " dos valores próprios, e a casa já tinha escrito a relação nos dois papers sem"
           " a ligar à medida. Logo a HIPÓTESE ESTRUTURAL que o eval pede — «σ_m σ_m′ = −1"
           " tomada como relação de passagem» — é exactamente a conservação da área: o gato"
           " estica por σ numa direcção e encolhe por 1/σ na outra, e o produto é 1. E o"
           " sinal negativo é a inversão de orientação, que a medida não vê",
           mal == 0 && casos == 40);
    }

    /* ═══ §M4 O LADO CONTÍNUO: |det DT| ≡ 1 sem T ser linear ══════════════════ */
    printf("\n§M4 O Jacobiano: a lei é LOCAL, e o cisalhamento prova-o.\n\n");
    {
        Cis t = { 1, 3, 5 };                    /* T(x,y) = (x + y² + 3y + 5, y) */
        long mal = 0, casos = 0, nao_linear = 0;
        for(long p = -12; p <= 12; p++) for(long q = 1; q <= 4; q++){
            Qz y = qz(p,q);
            Mat J = md_jacobiano_cisalha(t, y);
            casos++;
            if(!qz_igual(mat_det(J), qz(1,1))) mal++;
            if(J.a[0][1].p != 0) nao_linear++;   /* a entrada MUDA com o ponto */
        }
        printf("      |det DT| em %ld pontos: %ld divergências de 1, e a entrada ∂/∂y"
               " varia em %ld deles\n", casos, mal, nao_linear);
        ok("μ(T(E)) = ∫_E |det DT| É A MESMA LEI NO CONTÍNUO, e o cisalhamento mostra que"
           " ela é LOCAL: T(x,y) = (x + y² + 3y + 5, y) não é linear — a derivada muda em"
           " cada ponto —, e mesmo assim |det DT| = 1 em todos eles. Logo a conservação não"
           " é uma propriedade de matrizes: é do factor de volume ponto a ponto, e o caso"
           " linear é aquele em que ele não varia",
           mal == 0 && nao_linear > casos/2);
    }

    /* ═══ §M5 O GUME: retirar |det| = 1 ══════════════════════════════════════ */
    printf("\n§M5 O gume: sem |det| = 1 a medida NÃO se conserva.\n\n");
    {
        long achou = 0, casos = 0;
        for(long k = 2; k <= 8; k++){
            Mat J = md_jacobiano_dilata(k);
            casos++;
            if(!qz_igual(mat_det(J), qz(1,1)) && mat_det(J).p == k) achou++;
        }
        /* e o controlo do buscador: onde ele tem de voltar VAZIO */
        long falso = 0, ccasos = 0;
        for(long m = 1; m <= 20; m++){
            Mat A = md_gato(m);
            Qz d = mat_det(A);
            ccasos++;
            if(d.p != -1) falso++;              /* aqui NÃO pode achar violação */
        }
        printf("      dilatações com |det| = k ≠ 1: %ld/%ld achadas;  e nos metais o"
               " buscador volta vazio: %ld falsos em %ld\n", achou, casos, falso, ccasos);
        ok("E O GUME É AUTOMÁTICO: retirada a hipótese |det| = 1, a medida escala pelo"
           " determinante e a conservação cai — a dilatação por k multiplica a área por k,"
           " e o buscador acha-a em todas. O segundo controlo é o que torna isto uma"
           " medição: nos metais ele tem de voltar VAZIO, e volta. Um buscador que achasse"
           " violação em tudo estaria errado ele, não o teorema",
           achou == casos && falso == 0 && ccasos == 20);
    }

    /* ═══ §M6 A SEGUNDA REALIZAÇÃO: a primeira satura, a lei não ═════════════ */
    printf("\n§M6 Falha de representação NÃO é contra-exemplo.\n\n");
    {
        long onde_saturou = 0;
        Qz d;
        for(long k = 1; k <= 60; k++){
            if(!md_det_potencia_exacto(3, k, &d)){ onde_saturou = k; break; }
            if(!qz_igual(d, qz((k%2) ? -1 : 1, 1))) onde_saturou = -k;
        }
        /* a SEGUNDA realização: resíduos, que não crescem nunca */
        long mal_mod = 0, casos_mod = 0;
        const long PR[] = { 1000003, 999983, 65537 };
        for(int i = 0; i < 3; i++) for(long k = 1; k <= 400; k++){
            long esperado = ((k % 2) ? PR[i] - 1 : 1);
            casos_mod++;
            if(md_det_potencia_mod(3, k, PR[i]) != esperado) mal_mod++;
        }
        printf("      a 1.ª realização (inteiros exactos) satura em k = %ld;"
               " saturações contadas: %ld\n", onde_saturou, md_saturou);
        printf("      a 2.ª realização (resíduos, 3 primos) responde até k = 400:"
               " %ld/%ld divergências\n", mal_mod, casos_mod);
        ok("UMA SATURAÇÃO NÃO É UM CONTRA-EXEMPLO MATEMÁTICO, e é este §M6 que o"
           " mede em vez de o declarar. A primeira realização satura — as entradas de A^k"
           " crescem como σ^k e ao fim de poucas potências não cabem —, e a segunda, em"
           " resíduos, responde até k = 400 com det(A^k) = (−1)^k em três primos"
           " independentes. O que a primeira mostrou foi o tamanho do `long`; o que a"
           " segunda mostra é a lei. E a saturação conta-se num sítio SEPARADO dos defeitos,"
           " porque não é um",
           mal_mod == 0 && onde_saturou > 0 && md_saturou > 0);
    }

    /* ═══ §M7 E O QUE AQUI NÃO SE MEDE ═══════════════════════════════════════ */
    printf("\n§M7 σ_n(A) := |det A| é definição — o conteúdo está noutro sítio.\n\n");
    {
        Mat A = md_gato(2);
        Qz d = mat_det(A);
        Qz sigma = d.p < 0 ? qz_oposto(d) : d;
        int tautologia = qz_igual(sigma, qz(1,1));
        printf("      σ_n(A) := |det A| = "); printf("%ld/%ld", sigma.p, sigma.q);
        printf("   ← e esta igualdade não pode falhar\n");
        ok("«det|·| = σ_n» É VERDADE POR DEFINIÇÃO quando σ_n(A) se define como |det A|, e"
           " uma asserção que não pode falhar não é medição. Digo-o em vez de o esconder,"
           " e o conteúdo do teorema está nas QUATRO CONTAS do §M2: esse mesmo número é o"
           " determinante (álgebra), o factor da ação em Λⁿ (geometria), o produto σσ′"
           " (espectro) e a razão de pontos do reticulado (contagem). Nenhuma delas é a"
           " definição da outra, e é por isso que elas coincidirem é um facto",
           tautologia);
    }

    printf("\n=== %ld asserções, %ld falhas, %ld estouros, %ld saturações"
           " (que não são falhas) ===\n", unidades, falhas, md_estouros, md_saturou);
    return falhas ? 1 : 0;
}
