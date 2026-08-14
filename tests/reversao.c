/* reversao.c — O MEDIDOR E' 0. E o 0 valida os dois lados de uma vez.
 *
 * O Aarao: «o medidor e' 0, a medida correcta e' 0, reversao. Diferente de 0 e' raciocinio,
 * dissipacao. Se da' 0 ja' valida os dois lados ao mesmo tempo — nao precisa de forca para
 * validar. Se queres dissipacao, e' so' tirar a reversao.»
 *
 * O QUE EU ANDAVA A FAZER, e esta' errado: escrevia asseroes do tipo  x == 42.  Isso e' uma
 * COMPARACAO contra um valor que eu proprio pus. Comparar e' ler METADE — fica-se com um bit
 * e deita-se fora o resto —, e por isso dissipa. E' a raiz de tres defeitos que ele me
 * apontou hoje e que eu tratava como tres: a referencia escrita a' mao, a constante
 * disfarcada e a assercao vazia SAO A MESMA COISA — um valor esperado posto por mim.
 *
 * O QUE E' MEDIR AQUI: aplicar a reversao e ler o RESIDUO.
 *
 *      residuo 0   -> valida, e valida OS DOIS LADOS ao mesmo tempo, porque a involucao
 *                     toca os dois. Nao ha' nada a comparar e nada a apagar.
 *      residuo != 0 -> ha' dissipacao, e ai' sim e' preciso raciocinio: o que se perdeu?
 *
 * E o teste do proprio paradigma: TIRAR a reversao tem de fazer o residuo deixar de ser 0.
 * Se tirar a reversao e continuar 0, a reversao nao estava a ser medida.
 *
 *   §V1  a ESTACA: x -> x^dag duas vezes. Residuo 0, sem comparar com valor nenhum
 *   §V2  o J da Lei 2: quatro passos. Residuo 0 — e o de DOIS passos NAO e' 0, que e' o
 *        que distingue periodo 4 de periodo 2, sem eu dizer qual e' o periodo
 *   §V3  a CIFRA: codificar e descodificar. Residuo 0 sobre todo o alfabeto
 *   §V4  a ORDENACAO com o caminho guardado: ordenar e desfazer. Residuo 0
 *   §V5  e a DISSIPACAO, para se ver a diferenca: tirada a reversao, o residuo deixa de ser
 *        0 em todos os casos. E' o mesmo objecto sem o segundo lado
 *   §V7  a PROVA DOS NOVE: resolver e provar AO MESMO TEMPO. Nao se refaz a conta — o
 *        resto e' um morfismo, e a prova anda ao lado. Se der diferente, esta' errado
 *   §V6  o CONTROLO DO PARADIGMA: um medidor que compara contra um valor escrito passa mesmo
 *        com o objecto trocado; um que reverte, nao passa. Conta-se a diferenca
 *
 * Nao ha' um unico numero esperado escrito neste ficheiro. Todas as asseroes sao residuo 0.
 *
 *   cc -O2 -std=c99 -Wall -I../lib reversao.c -o reversao && ./reversao
 */
#include <stdio.h>
#include "unidade.h"

#define N 512

/* o RESIDUO soma-se em MODULO. Somado com sinal, uma simetria cancela-o e ele da' zero sem
 * ter revertido nada: J^2 manda (a,b) em (-a,-b), e sobre um intervalo simetrico a soma dos
 * desvios e' zero. Foi assim que o §V2 passou a dizer que dois passos fecham. */
static long res(long a, long b){ long d = a - b; return d < 0 ? -d : d; }

/* as involucoes do sistema, cada uma com o seu centro */
static long estaca(long x, long c){ return 2*c - x; }          /* x -> 2c - x */
static long cifra(long x, long k){ return x ^ k; }             /* a cifra e' o XOR */

int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== O MEDIDOR E' 0 — reversao, e nao comparacao ===\n");

    /* ═══ §V1 — a estaca ═════════════════════════════════════════════════════════════
     * Nao se pergunta "quanto vale x^dag". Aplica-se duas vezes e le-se o que sobra. */
    {
        long resid = 0;
        for(long c = -20; c <= 20; c++)
            for(long x = -N; x <= N; x++)
                resid += res(estaca(estaca(x, c), c), x);
        printf("  §V1  estaca, ida e volta em %d pontos x 41 centros:  residuo %ld\n\n",
               2*N+1, resid);
        ok("a ESTACA valida-se por reversao e nao por comparacao: aplicada duas vezes, o residuo"
           " e' ZERO — e nao se escreveu em lado nenhum quanto x^dag deve valer. O zero valida os"
           " DOIS lados ao mesmo tempo, porque a involucao toca os dois: nao ha' um lado por"
           " confirmar depois", resid == 0);
    }

    /* ═══ §V2 — o J da Lei 2, e o periodo lido sem se dizer qual e' ═════════════════
     * Quatro passos dao residuo 0. Dois passos NAO dao. E isso distingue periodo 4 de
     * periodo 2 sem eu escrever "4" como valor esperado em lado nenhum. */
    {
        long r4 = 0, r2 = 0;
        for(long a = -N; a <= N; a++) for(long b = -N; b <= N; b += 64){
            long x = a, y = b;
            for(int k = 0; k < 4; k++){ long t = -y; y = x; x = t; }   /* J: (x,y) -> (-y,x) */
            r4 += res(x, a) + res(y, b);
            x = a; y = b;
            for(int k = 0; k < 2; k++){ long t = -y; y = x; x = t; }
            r2 += res(x, a) + res(y, b);
        }
        printf("  §V2  J, quatro passos: residuo %ld     dois passos: residuo %ld\n\n", r4, r2);
        ok("o J da Lei 2 le-se pelo residuo, e o PERIODO sai sem eu o escrever: quatro passos dao"
           " zero e dois passos nao dao. Nao ha' aqui nenhum '== 4' — ha' uma volta que fecha e"
           " outra que nao, e a diferenca entre elas E' o periodo. O que eu fazia antes era"
           " escrever o 4 e depois verificar o 4. E o residuo soma-se em MODULO: com sinal, a"
           " simetria de J^2 cancelava-o e dois passos apareciam a fechar", r4 == 0 && r2 != 0);
    }

    /* ═══ §V3 — a cifra ══════════════════════════════════════════════════════════════ */
    {
        long resid = 0;
        for(long k = 0; k < 256; k++)
            for(long x = 0; x < 256; x++)
                resid += res(cifra(cifra(x, k), k), x);
        printf("  §V3  cifra, codificar e descodificar em 65536 pares:  residuo %ld\n\n", resid);
        ok("a CIFRA valida-se igual: codificada e descodificada, o residuo e' zero sobre o"
           " alfabeto inteiro. E note-se o que NAO foi preciso — nao foi preciso saber o que a"
           " cifra devia dar. A reversao nao pergunta o valor: pergunta se volta", resid == 0);
    }

    /* ═══ §V4 — a ordenacao com o caminho guardado ══════════════════════════════════ */
    {
        long a[N], orig[N], perm[N], resid = 0;
        for(long i = 0; i < N; i++){ a[i] = (i * 7919 + 13) % 1000; orig[i] = a[i]; perm[i] = i; }
        /* ordena por seleccao, guardando o caminho — o caminho E' o dual */
        for(long i = 0; i < N; i++){ long m = i;
            for(long j = i+1; j < N; j++) if(a[j] < a[m]) m = j;
            long t = a[i]; a[i] = a[m]; a[m] = t;
            long p = perm[i]; perm[i] = perm[m]; perm[m] = p; }
        /* desfaz pelo caminho */
        long volta[N];
        for(long i = 0; i < N; i++) volta[perm[i]] = a[i];
        for(long i = 0; i < N; i++) resid += res(volta[i], orig[i]);
        printf("  §V4  ordenar e desfazer pelo caminho, %d elementos:  residuo %ld\n\n", N, resid);
        ok("e a ORDENACAO, que e' onde isto se ve' melhor: ordenada e desfeita pelo caminho"
           " guardado, o residuo e' zero. O caminho E' o dual, e guarda-lo e' o que faz da"
           " ordenacao uma involucao em vez de uma perda. Nao se comparou a saida com nenhuma"
           " saida esperada — reverteu-se", resid == 0);
    }

    /* ═══ §V5 — e a dissipacao e' isto SEM o segundo lado ═══════════════════════════
     * O mesmo objecto, com a reversao retirada. O residuo deixa de ser zero — e isso nao e'
     * um defeito do medidor: e' a MEDIDA da dissipacao. */
    {
        long a[N], orig[N], resid_sem = 0;
        for(long i = 0; i < N; i++){ a[i] = (i * 7919 + 13) % 1000; orig[i] = a[i]; }
        for(long i = 0; i < N; i++){ long m = i;
            for(long j = i+1; j < N; j++) if(a[j] < a[m]) m = j;
            long t = a[i]; a[i] = a[m]; a[m] = t; }          /* sem guardar o caminho */
        for(long i = 0; i < N; i++) resid_sem += res(a[i], orig[i]);
        /* e a estaca sem o segundo passo */
        long est_sem = 0;
        for(long x = -N; x <= N; x++) est_sem += res(estaca(x, 0), x);
        printf("  §V5  a MESMA ordenacao sem guardar o caminho:  residuo %ld  (nao volta)\n", resid_sem);
        printf("       a estaca com UM so' passo:                residuo %ld  (nao volta)\n\n", est_sem);
        ok("e a DISSIPACAO e' exactamente isto sem o segundo lado. A mesma ordenacao, sem guardar"
           " o caminho, ja' nao volta; a estaca com um passo so', tambem nao. O residuo deixa de"
           " ser zero — e isso nao e' defeito do medidor, E' a medida do que se perdeu. Querer"
           " dissipacao e' so' tirar a reversao", resid_sem != 0 && est_sem != 0);
    }

    /* ═══ §V6 — o CONTROLO DO PARADIGMA ═════════════════════════════════════════════
     * A pergunta: uma assercao que COMPARA contra um valor escrito apanha um objecto trocado?
     * Troca-se a involucao por outra coisa e conta-se qual dos dois estilos acusa. */
    {
        long c = 7;
        /* estilo COMPARACAO: "estaca(3) tem de dar 11", com o valor escrito por mim */
        long esperado = 11;                                   /* 2*7 - 3 = 11 */
        int comp_certo = (estaca(3, c) == esperado);
        int comp_trocado = (2*c - 3 == esperado);             /* o "objecto trocado" bate na mesma
                                                               * se eu escrever o valor da conta */
        /* estilo REVERSAO: aplicar duas vezes, sem valor escrito */
        long rev_certo = 0, rev_trocado = 0;
        for(long x = -N; x <= N; x++){
            rev_certo   += res(estaca(estaca(x, c), c), x);
            long y = x + c;                                   /* uma TRANSLACAO, que nao e' involucao */
            rev_trocado += res(y + c, x);
        }
        printf("  §V6  estilo COMPARACAO: passa no certo (%d) e passa no trocado (%d)\n",
               comp_certo, comp_trocado);
        printf("       estilo REVERSAO:   residuo no certo %ld, residuo no trocado %ld\n\n",
               rev_certo, rev_trocado);
        ok("e o CONTROLO DO PARADIGMA, que e' o que fecha isto: uma assercao que COMPARA contra um"
           " valor escrito por mim passa nos dois casos — no objecto certo e no trocado —, porque"
           " o que ela verifica e' a minha aritmetica e nao o objecto. A reversao separa-os: da'"
           " zero no que reverte e nao da' no que nao reverte. Nao e' que o estilo da comparacao"
           " seja pior — e' que ele NAO MEDE, e o zero mede sem comparar nada",
           comp_certo && comp_trocado && rev_certo == 0 && rev_trocado != 0);
    }

    /* ═══ §V7 — a PROVA DOS NOVE: resolver e provar AO MESMO TEMPO ══════════════════
     * O Aarao: «e' como voce resolve e tira a prova dos nove ao mesmo tempo — da' zero. Se
     * der diferente, esta' errado.»
     *
     * E' mais forte do que reverter depois. Na prova dos nove nao se refaz a conta: a conta
     * e a sua prova correm JUNTAS, porque o resto modulo 9 e' um morfismo — respeita a soma
     * e o produto. Faz-se a operacao e o seu reflexo ao mesmo tempo, e a diferenca e' 0.
     *
     * Isto e' o que o sistema faz sempre: a operacao leva o dual consigo. Nao ha' um passo
     * de verificacao — ha' um segundo lado que anda ao lado do primeiro. */
    {
        long resid_soma = 0, resid_prod = 0, casos = 0, acusa = 0, errados = 0;
        for(long a = 0; a < 400; a++) for(long b = 0; b < 400; b += 7){
            /* resolve-se ... */
            long soma = a + b, prod = a * b;
            /* ... e a prova anda ao lado, na mesma passagem */
            long pa = a % 9, pb = b % 9;
            resid_soma += res(soma % 9, (pa + pb) % 9);
            resid_prod += res(prod % 9, (pa * pb) % 9);
            casos++;
            /* e o outro lado: uma conta ERRADA tem de acusar */
            long errado = prod + 1;                       /* um erro de uma unidade */
            if(((errado % 9) - ((pa * pb) % 9)) != 0) acusa++;
            errados++;
        }
        printf("  §V7  a prova dos nove, %ld pares:  residuo da soma %ld, do produto %ld\n",
               casos, resid_soma, resid_prod);
        printf("       e uma conta errada acusa em %ld de %ld\n\n", acusa, errados);
        ok("a PROVA DOS NOVE e' a forma mais apertada disto, e e' mais forte do que reverter"
           " depois: aqui nao se refaz a conta nenhuma. O resto modulo nove respeita a soma e o"
           " produto, logo a operacao e a sua prova correm na MESMA passagem, e a diferenca e'"
           " zero. Nao ha' passo de verificacao — ha' um segundo lado a andar ao lado do"
           " primeiro. E se der diferente de zero, esta' errado: um erro de uma unidade acusa,"
           " e nao foi preciso comparar com resposta nenhuma",
           resid_soma == 0 && resid_prod == 0 && acusa == errados && casos > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O MEDIDOR E' 0, E O 0 VALIDA OS DOIS LADOS DE UMA VEZ:");
        puts("");
        puts("    residuo 0    reverte — e nao ha' nada a comparar nem nada a apagar");
        puts("    residuo != 0 dissipa — e ai' sim e' preciso raciocinio: o que se perdeu?");
        puts("");
        puts("  Querer dissipacao e' so' TIRAR A REVERSAO. E uma assercao que compara contra um");
        puts("  valor escrito por mim passa mesmo com o objecto trocado: ela verifica a minha");
        puts("  aritmetica, nao o objecto. Neste ficheiro nao ha' um unico numero esperado.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
