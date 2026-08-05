/* relogio.h — O RELOGIO, QUE OPERA. ("colisor" e' sinonimo; este ficheiro chamou-se
 * colisor.h ate' se ter verificado que o objecto ja' existia com o nome antigo.)
 *
 * O Aarao: "ve tambem se o relogio ja' e' o colisor, pq ele faz tudo isso. As colisoes sao
 * as contagens do relogio. Concentra tudo no relogio e diz que colisor e' sinonimo."
 *
 * E' ISSO, E FUI VERIFICAR ANTES DE O ESCREVER. O tests/relogio.c ja' tinha, com estes
 * nomes e ha' muito:
 *
 *   §R5  O CONTADOR: quantas marcas ate' t — floor(t/d)+1, e a densidade e' 1/d
 *   §R3  O PENTE E' AUTODUAL: o dual do pente de passo d e' o pente de passo N/d
 *   §R4  e os dois passos MULTIPLICAM-SE para dar N — a norma, outra vez
 *   §R2  autodual <=> Delta = 1
 *
 * UMA COLISAO E' UMA MARCA DO CONTADOR. O percurso e' a volta do pente. Os "dois lados" do
 * bidual sao o par (d, N/d) do §R3 — nao sao uma construcao nova, sao o pente e o seu
 * dual. E "tres batidas sao a volta" e' o periodo lido no mesmo contador.
 *
 * E o banco_relogio.c ja' o tinha dito do outro lado: `i = (i+1) % NSLOT` E' o pente de
 * passo 1, e cada sondagem e' uma marca.
 *
 * ── PORQUE FICA ESTE FICHEIRO, ENTAO ────────────────────────────────────────────────
 *
 * Porque o relogio.c MEDE e este OPERA: aqui estao as funcoes que escolhem o percurso, e
 * o relogio.c e' onde se prova que elas sao o que dizem ser. Nao ha' objecto novo — ha' um
 * segundo nome para o mesmo, e o nome bom e' RELOGIO. Este ficheiro chama-se colisor
 * porque foi assim que a conversa chegou aqui, e o cabecalho existe para que ninguem o
 * tome por uma coisa nova.
 *
 * (E' o meu erro mais repetido: trazer regua nova para um objecto que ja' tinha a sua.
 * Desta vez o objecto tinha ate' o mesmo teorema — o pente autodual — e eu construi-o
 * outra vez com outro nome.)
 *
 * O colisor de um corpo le-se da assinatura: se ela e' (p,q,r), o grau e' n = p+q+r, a base
 * e' {+-1}^n, e as involucoes sao as n que trocam UM eixo. Ate' aqui e' leitura. O que faz
 * dele ferramenta e' isto: ELE ESCOLHE O PERCURSO.
 *
 * ── O QUE ISTO CORRIGE ──────────────────────────────────────────────────────────────
 *
 * Uma prova de involucao escreve-se quase sempre assim:
 *
 *      x --v--> y --v--> x        e exige-se que o segundo x seja o primeiro
 *
 * e isso SO' e' valido se `v` tiver periodo 2. Quando o objecto e' bidual — quando o que
 * o move sao DUAS involucoes por lados diferentes —, aplicar a mesma duas vezes leva ao
 * ponto ANTIPODA e nao a casa. A orbita tem quatro estados e nao dois:
 *
 *      x, iA(x), iB(x), iA(iB(x))          o grupo de Klein: id, iA, iB, iA.iB
 *
 * E' o que o spinor32.c §S6 mediu: dualizar uma vez nao fecha; sao precisas duas, POR
 * LADOS DIFERENTES. Repetir o mesmo lado nao e' bidualidade — e' a mesma involucao outra
 * vez.
 *
 * ── O QUE ESTE FICHEIRO DA' ─────────────────────────────────────────────────────────
 *
 *   colisor_fecha(n)      existe circuito que usa cada colisao uma vez e volta? (n par)
 *   colisor_desdobra(n)   a composta de todas troca a paridade? (n impar)
 *   colisor_estados(n)    2^n
 *   colisor_colisoes(n)   n.2^(n-1)
 *   colisor_biduais(n)    2^n / 4, com os eixos partidos em dois blocos
 *   colisor_passos(n)     QUANTOS passos FECHAM a orbita do par: 4 se ha' dois lados
 *   colisor_volta(per)    quantas batidas INVERTEM: periodo-1 — tres, se o periodo e' 4
 *   colisor_percurso(...) QUAIS: a sequencia de eixos a percorrer, alternando os lados
 *
 * Nada disto guarda estado: sao contas sobre o grau. Zero .bss.
 */
#ifndef RELOGIO_OP_H
#define RELOGIO_OP_H

/* quantos estados tem o colisor de grau n */
static long colisor_estados(int n){ return 1L << n; }

/* quantas colisoes distintas — contadas andando nelas, nao pela formula */
static long colisor_colisoes(int n){
    long c = 0, V = 1L << n;
    for(long v = 0; v < V; v++) for(int b = 0; b < n; b++) if(!((v >> b) & 1)) c++;
    return c;
}

/* existe percurso que usa CADA colisao uma vez e regressa? cada estado tem grau n, e um
 * circuito precisa de grau par — logo isto e' a paridade do grau, e nada mais */
static int colisor_fecha(int n){ return n > 0 && (n % 2 == 0); }

/* a composta de todas as involucoes troca a paridade sse n e' impar: e' o desdobramento */
static int colisor_desdobra(int n){ return n > 0 && (n % 2 == 1); }

/* as orbitas do par de involucoes parciais tem quatro estados */
static long colisor_biduais(int n){ return n >= 2 ? (1L << n) / 4 : 0; }

/* ── O QUE FAZ DELE FERRAMENTA ──────────────────────────────────────────────────────
 *
 * QUANTOS passos fecham a orbita, dados `lados` blocos de eixos:
 *
 *   1 lado  -> a involucao e' uma so', periodo 2: dois passos fecham
 *   2 lados -> grupo de Klein, orbita de quatro: sao precisos QUATRO
 *
 * Exigir dois passos de um objecto de dois lados e' o defeito que este ficheiro existe
 * para impedir. */
static int colisor_passos(int lados){ return lados <= 1 ? 2 : 4; }

/* ── E TRES BATIDAS SAO A VOLTA ─────────────────────────────────────────────────────
 *
 * O Aarao: "isso sao 3 colisoes e volta, nao passa pelo 0 — ja' temos referencias dessas
 * 3 batidas no texto."
 *
 * Tem, em oito sitios do catalogo, e sempre com a mesma frase: TRES BATIDAS DE UM LADO SAO
 * A VOLTA. E' o periodo 4 lido do lado util: se F^4 = id, entao
 *
 *      F^3 = F^{-1}
 *
 * isto e', tres aplicacoes NUM sentido valem uma no sentido OPOSTO. Fechar leva quatro,
 * mas VOLTAR leva tres — e a diferenca importa, porque a terceira batida ja' esta' do
 * outro lado, e o percurso NAO PASSA PELO PONTO DE PARTIDA no meio.
 *
 * Dai a distincao que este ficheiro guarda: `colisor_passos` conta o que FECHA a orbita,
 * `colisor_volta` conta o que a INVERTE. Sao numeros diferentes e nao se substituem. */
static int colisor_volta(int periodo){ return periodo > 1 ? periodo - 1 : 0; }

/* e os estados que o percurso visita antes de fechar: nenhum deles e' o de partida, e
 * nenhum e' o ponto fixo — sao `periodo - 1` pontos distintos. */
static int colisor_intermedios(int periodo){ return periodo > 1 ? periodo - 1 : 0; }

/* E QUAIS: a sequencia de lados a aplicar, ALTERNANDO. Escreve `colisor_passos(lados)`
 * entradas em `saida` e devolve quantas. Com dois lados sai 0,1,0,1 — e nao 0,0, que e'
 * o mesmo lado duas vezes e nunca fecha uma orbita de quatro. */
static int colisor_percurso(int lados, int *saida){
    int k = colisor_passos(lados);
    for(int i = 0; i < k; i++) saida[i] = (lados <= 1) ? 0 : (i % lados);
    return k;
}

/* aplica o percurso a um estado do colisor: cada lado e' uma mascara de eixos, e colidir
 * e' fazer XOR com ela. Devolve o estado final — que TEM de ser o inicial se o percurso
 * fecha, e e' assim que se verifica sem acreditar em ninguem. */
static unsigned colisor_anda(unsigned x, const unsigned *mascaras, const int *percurso, int k){
    for(int i = 0; i < k; i++) x ^= mascaras[percurso[i]];
    return x;
}

#endif
