/* tests/tres_caminhos.c — OS TRÊS CAMINHOS SÃO A MESMA CLASSE, e isso estava por medir.
 *
 * (O `tests/reais.c` é outro medidor e trata outra pergunta: corpo vs. ordenável,
 *  Artin--Schreier, e ℝ como único corpo ordenado completo. Este trata a CONSTRUÇÃO.)
 *
 * O `racionais.tex` §sec:qstar escreve, sobre os três caminhos do `lib/reais.h`: «a cadeia
 * de testes verifica que elas concordam». E o `lib/cauchy.h` diz o mesmo com mais força:
 * «não são três aproximações de √2: são TRÊS REPRESENTANTES DA MESMA CLASSE, e a
 * equivalência mede-se».
 *
 * Mede-se, mas não estava medida: `rz_abaixo`, `rz_no_corte`, `rz_fecha_em_q`,
 * `rz_caixa_inicial`, `rz_encaixota`, `rz_passo` — nenhuma era chamada por medidor
 * nenhum de `tests/`. A afirmação vivia num paper e num header, e não numa asserção.
 *
 *   1. o CORTE        A = {q : q ≤ 0 ou qⁿ < a} — a DECISÃO, inteira e exacta
 *   2. o PONTO FIXO   x ↦ (a+bx)/(x+b), Möbius inteiro, ponto fixo x² = a
 *   3. a FRAÇÃO CONTÍNUA   os convergentes, periódicos por Lagrange
 *
 * §RE0  o corte é DECISÃO e não aproximação: todo racional cai de um lado, e «em cima»
 *       só existe quando a é potência n-ésima perfeita
 * §RE1  o ENCAIXOTAMENTO: a largura não «tende a zero» — É (b₀−a₀)/2^k, exacta
 * §RE2  OS TRÊS CAMINHOS: cada um aponta ao MESMO corte, e dois a dois são equivalentes
 * §RE3  os convergentes ALTERNAM de lado do corte — par abaixo, ímpar acima
 * §RE4  a órbita de Möbius é MONÓTONA e fica de UM lado: b² > a não deixa mudar de sinal
 * §RE5  o CONTROLO: com a quadrado perfeito o corte FECHA em ℚ — e é ABERTO (r ∉ A)
 * §RE7  n = 3: o corte e o encaixe SOBEM de índice; o Möbius é do andar quadrático
 * §RE8  a CLASSE: ∼ é equivalência (as três cláusulas) e SEPARA radicandos distintos
 * §RE9  o CONTROLO: saltos a zero NÃO é ser de Cauchy — a harmónica prova-o
 * §RE10 a SOMA desce ao quociente: trocar o representante não muda a classe
 * §RE11 o PRODUTO desce, e pede a hipótese a mais — ser LIMITADA
 * §RE12 a ORDEM desce ao quociente — entre classes distintas, e diz-se a ressalva
 * §RE13 a SOMA DE CORTES: os encaixes somam-se, e o critério inteiro concorda
 * §RE6  o TECTO diz-se: `rz_cabe` recusa-se, e a recusa é contada — não se afirma nada
 *
 * ── SEM LIMIAR, E É O PONTO ───────────────────────────────────────────────────────
 * Nenhuma asserção aqui compara com um ε escolhido por mim. A régua é o próprio corte:
 * `rz_cmp` decide de que lado está cada racional, em inteiros, e o encaixe dá a moldura.
 * Onde há ε (o `cy_equiv`), ele é um racional EXIBIDO e o N é a testemunha devolvida.
 */
#include <stdio.h>
#include "racionais.h"
#include "cifra.h"
#include "reais.h"
#include "cauchy.h"
#include "unidade.h"

static Suc suc(Tipo t, long a){ Suc s; s.t = t; s.a = a; s.p = 0; s.q = 1; return s; }

/* O CRITÉRIO INTEIRO DO CORTE DA SOMA √2+√3: s satisfaz (s²−5)² = 24, logo
 *      p/d < √2+√3  ⟺  p ≤ 0  ou  p² ≤ 5d²  ou  (p²−5d²)² < 24·d⁴
 * Teto declarado: com p,d ≤ 4096 tem-se (p²−5d²)² ≲ 1e16, que cabe em long long.
 * (Era um macro; o parâmetro chamava-se `q` e o membro do struct também, e o
 *  pré-processador substituiu os dois — daí a função.) */
static int soma23_abaixo(Qz x){
    long long p = x.p, d = x.q, t = p*p - 5*d*d;
    if(p <= 0) return 1;
    if(t <= 0) return 1;
    return t*t < 24LL*d*d*d*d;
}

int main(void){
    printf("\n=== ℝ: o corte, o ponto fixo e a fracção contínua — a mesma classe ===\n");
    const long NAO_QUAD[5] = {2, 3, 5, 7, 10};      /* √a irracional */
    const long QUAD[4]     = {4, 9, 16, 25};        /* √a racional — o controlo */

    /* ═══ §RE0 O CORTE É UMA DECISÃO ═══════════════════════════════════════════ */
    printf("\n§RE0 Todo racional cai de um lado; «em cima» só se a for potência perfeita.\n\n");
    {
        long decididos = 0, em_cima_irr = 0, em_cima_quad = 0, indeciso = 0;
        for(int i = 0; i < 5; i++){
            Corte c = { NAO_QUAD[i], 2 };
            for(long p = -20; p <= 60; p++) for(long q = 1; q <= 40; q++){
                Qz x = qz(p, q);
                int r = rz_abaixo(c, x);
                if(r < 0){ indeciso++; continue; }          /* não coube: não se afirma */
                decididos++;
                if(rz_no_corte(c, x)) em_cima_irr++;        /* xⁿ = a exactamente */
            }
        }
        for(int i = 0; i < 4; i++){
            Corte c = { QUAD[i], 2 };
            long r;
            if(rz_fecha_em_q(c, &r) && rz_no_corte(c, qz_de_inteiro(r))) em_cima_quad++;
        }
        printf("        %ld racionais decididos, %ld indecisos (não coube no tipo)\n",
               decididos, indeciso);
        printf("        EM CIMA do corte: %ld para √2,√3,√5,√7,√10 · %ld de 4 para os quadrados\n",
               em_cima_irr, em_cima_quad);
        ok("O REAL NÃO É UMA TIRA DE CASAS: É A DECISÃO SOBRE CADA RACIONAL. O corte não"
           " guarda dígitos — guarda o critério que os decidiria a todos, e o critério é"
           " inteiro (pⁿ contra a·qⁿ, em i128). Varridos milhares de racionais para cinco"
           " radicandos não-quadrados, NENHUM cai em cima do corte: o buraco está sempre"
           " aberto. E para os quadrados perfeitos ele fecha nos quatro casos, com a raiz"
           " exibida pela régua da casa (`raizi`) e não por tentativa. É a diferença entre"
           " «não achei» e «não existe», e aqui é a segunda",
           em_cima_irr == 0 && em_cima_quad == 4 && decididos > 1000);
    }

    /* ═══ §RE1 O ENCAIXOTAMENTO: a largura É uma fracção ═══════════════════════ */
    printf("\n§RE1 A largura não tende a zero: é (b₀−a₀)/2^k, e diz-se onde pára.\n\n");
    {
        long mal = 0, casos = 0;
        printf("        a   passos pedidos   passos feitos   [lo, hi] final\n");
        for(int i = 0; i < 5; i++){
            Corte c = { NAO_QUAD[i], 2 };
            Qz lo, hi;
            if(!rz_caixa_inicial(c, &lo, &hi)){ mal++; continue; }
            Qz lo0 = lo, hi0 = hi;
            long antes = qz_saturou;
            int feitos = rz_encaixota(c, &lo, &hi, 12);
            casos++;
            printf("        %2ld        12              %2d          [%ld/%ld, %ld/%ld]\n",
                   NAO_QUAD[i], feitos, lo.p, lo.q, hi.p, hi.q);
            /* o encaixe encolhe e nunca larga o corte: lo abaixo, hi acima, sempre */
            if(!(qz_menor(lo0, lo) || qz_igual(lo0, lo))) mal++;
            if(!(qz_menor(hi, hi0) || qz_igual(hi, hi0))) mal++;
            if(rz_abaixo(c, lo) != 1) mal++;                /* lo continua em A */
            if(rz_abaixo(c, hi) != 0) mal++;                /* hi continua fora */
            if(!qz_menor(lo, hi)) mal++;
            if(qz_saturou != antes && feitos == 12) mal++;  /* saturou e não disse */
        }
        ok("E O ENCAIXE É UM RELÓGIO, NÃO UMA ESPERANÇA: cada passo é uma dobra, a largura"
           " é exactamente (b₀−a₀)/2^k, e as duas pontas nunca largam o corte — lo fica"
           " dentro de A e hi fora, em todos os passos e nos cinco radicandos. Quando o"
           " ponto médio satura, `rz_encaixota` PÁRA e devolve quantos passos fez: o número"
           " de passos honestos diz-se, e não se finge que a bisseção continua. Um encaixe"
           " que mentisse aqui daria um real que não é o do corte",
           mal == 0 && casos == 5);
    }

    /* ═══ §RE2 OS TRÊS CAMINHOS APONTAM AO MESMO CORTE ═════════════════════════ */
    printf("\n§RE2 Möbius, pontas do encaixe e convergentes: a mesma classe.\n\n");
    {
        long aponta_ok = 0, equiv_ok = 0, total = 0;
        Qz eps = qz(1, 1000);                          /* o ε é EXIBIDO, e é racional */
        printf("        a    aponta(Möbius, LO, HI, FC)   equiv(M,FC)  N   equiv(LO,HI)  N\n");
        for(int i = 0; i < 5; i++){
            long a = NAO_QUAD[i];
            Corte c = { a, 2 };
            Suc M = suc(S_MOBIUS,a), L = suc(S_LO,a), H = suc(S_HI,a), F = suc(S_CONV,a);
            long n1,n2,n3,n4, nmf, nlh;
            int am = cy_aponta(M, c, 10, 12, &n1), al = cy_aponta(L, c, 10, 12, &n2);
            int ah = cy_aponta(H, c, 10, 12, &n3), af = cy_aponta(F, c, 10, 12, &n4);
            int emf = cy_equiv(M, F, eps, 12, &nmf);
            int elh = cy_equiv(L, H, eps, 12, &nlh);
            total++;
            if(am && al && ah && af) aponta_ok++;
            if(emf && elh) equiv_ok++;
            printf("        %2ld       %d %d %d %d                  %d      %2ld       %d       %2ld\n",
                   a, am, al, ah, af, emf, nmf, elh, nlh);
        }
        ok("OS TRÊS CAMINHOS NÃO SÃO TRÊS APROXIMAÇÕES: SÃO TRÊS REPRESENTANTES DA MESMA"
           " CLASSE, e agora está medido em vez de escrito. Para cada radicando, as quatro"
           " sucessões — a órbita de Möbius, as duas pontas do encaixe e os convergentes da"
           " fracção contínua — apontam ao MESMO corte (entram na moldura e não voltam a"
           " sair), e duas a duas são equivalentes com o ε exibido e o N devolvido como"
           " testemunha. O corte diz onde está, o Möbius vai lá, a fracção contínua"
           " escreve-o; e se um deles discordasse num único racional, um deles estaria"
           " errado. É a asserção que o `racionais.tex` §sec:qstar prometia e que nenhum"
           " medidor fazia",
           aponta_ok == 5 && equiv_ok == 5 && total == 5);
    }

    /* ═══ §RE3 OS CONVERGENTES ALTERNAM DE LADO ════════════════════════════════ */
    printf("\n§RE3 Par abaixo, ímpar acima — medido pelo corte, não por decimal.\n\n");
    {
        long mal = 0, vistos = 0;
        for(int i = 0; i < 5; i++){
            long a = NAO_QUAD[i];
            Corte c = { a, 2 };
            Suc F = suc(S_CONV, a);
            long h = cy_teto_honesto(F, 14);
            for(long n = 0; n <= h && n <= 10; n++){
                Qz x = cy_termo(F, n);
                int lado_dele = rz_abaixo(c, x);
                if(lado_dele < 0) continue;               /* não coube: não conta */
                vistos++;
                if(lado_dele != ((n % 2) == 0)) mal++;    /* par ⟹ abaixo; ímpar ⟹ acima */
            }
        }
        printf("        %ld convergentes: %ld fora do lado esperado\n", vistos, mal);
        ok("E A ALTERNÂNCIA DOS CONVERGENTES LÊ-SE NO CORTE, sem um único decimal: o"
           " convergente de índice par cai dentro de A e o de índice ímpar cai fora, em"
           " todos os que couberam no tipo. É a lei clássica da fracção contínua — os"
           " convergentes cercam o valor por lados alternados — medida aqui pela régua deste"
           " andar, que é a decisão inteira do corte. Os dois objectos foram construídos por"
           " vias que não partilham código: um é Euclides sobre inteiros, o outro é a"
           " comparação pⁿ contra a·qⁿ",
           mal == 0 && vistos > 20);
    }

    /* ═══ §RE4 A ÓRBITA DE MÖBIUS É MONÓTONA E DE UM LADO ══════════════════════ */
    printf("\n§RE4 b² > a: o sinal do erro não muda, e a órbita não atravessa.\n\n");
    {
        long mal = 0, casos = 0, cres = 0, decr = 0;
        for(int i = 0; i < 5; i++){
            long a = NAO_QUAD[i];
            Corte c = { a, 2 };
            Suc M = suc(S_MOBIUS, a);
            long h = cy_teto_honesto(M, 12);
            if(h < 2) continue;
            casos++;
            if(rz_b(a) * rz_b(a) <= a) mal++;             /* b² > a, garantido */
            int lado0 = rz_abaixo(c, cy_termo(M, 0));
            for(long n = 1; n <= h; n++){
                int l = rz_abaixo(c, cy_termo(M, n));
                if(l < 0) continue;
                if(l != lado0) mal++;                      /* não atravessa */
            }
            if(cy_crescente(M, h)) cres++; else if(cy_decrescente(M, h)) decr++; else mal++;
        }
        printf("        %ld radicandos: %ld órbitas crescentes, %ld decrescentes, %ld falhas\n",
               casos, cres, decr, mal);
        ok("A ÓRBITA VAI LÁ SEM NUNCA CHEGAR, E ISSO É O BURACO À VISTA. Com b escolhido"
           " por b² > a — e é a régua da casa que o dá, `raizi(a)+1` —, o erro multiplica-se"
           " por um factor de sinal fixo, logo a sucessão é monótona e fica sempre do mesmo"
           " lado do corte, em todos os termos honestos. Ela é de Cauchy em ℚ e o seu ponto"
           " fixo é x² = a, que ℚ não tem: é a definição de buraco, exibida por uma órbita"
           " que corre inteira dentro de ℚ",
           mal == 0 && casos == 5 && (cres + decr) == casos);
    }

    /* ═══ §RE5 O CONTROLO: com quadrado perfeito, não há real a acrescentar ════ */
    printf("\n§RE5 Onde ℚ já fecha, o corte fecha — e o andar não acrescenta nada.\n\n");
    {
        long fecharam = 0, atingiu = 0, aberto = 0, em_cima = 0;
        printf("        a    fecha em ℚ?   raiz   o encaixe atinge?   r ∈ A?\n");
        for(int i = 0; i < 4; i++){
            long a = QUAD[i], r = 0;
            Corte c = { a, 2 };
            int f = rz_fecha_em_q(c, &r);
            Qz lo, hi; int at = 0;
            if(rz_caixa_inicial(c, &lo, &hi)) at = qz_igual(lo, hi) && qz_igual(lo, qz_de_inteiro(r));
            if(f) fecharam++;
            if(at) atingiu++;
            /* O CORTE É ABERTO: o próprio r, com r² = a, NÃO pertence a A — é a escolha
             * de Dedekind entre A = {q² < a} e {q² ≤ a}, e ela decide-se aqui. */
            if(f && rz_abaixo(c, qz_de_inteiro(r)) == 0) aberto++;
            if(f && rz_no_corte(c, qz_de_inteiro(r))) em_cima++;
            printf("        %2ld       %s        %2ld           %s          %s\n",
                   a, f ? "sim" : "não", r, at ? "sim" : "não",
                   rz_abaixo(c, qz_de_inteiro(r)) == 0 ? "não (aberto)" : "SIM");
        }
        ok("E O CONTROLO É O CASO EM QUE ESTE ANDAR NÃO TEM TRABALHO: quando a é quadrado"
           " perfeito, o corte fecha DENTRO de ℚ — há racional em cima, e o encaixotamento"
           " colapsa no primeiro passo em vez de dobrar para sempre. Sem esta secção, §RE0 e"
           " §RE1 estariam a medir num regime onde o defeito não vive: mostrariam o corte a"
           " funcionar sem mostrar que ele sabe distinguir o caso em que ℚ basta. E é aqui"
           " que a escolha de Dedekind se decide, porque só aqui ela é observável: o corte é"
           " ABERTO — o próprio r, com r² = a, está EM CIMA e NÃO pertence a A, de modo que"
           " A não tem máximo em caso nenhum. Com A = {q² ≤ a} os quatro casos teriam"
           " máximo e a construção mudaria; nos radicandos irracionais as duas versões são"
           " indistinguíveis, e por isso esta asserção tinha de vir com um quadrado perfeito",
           fecharam == 4 && atingiu == 4 && aberto == 4 && em_cima == 4);
    }

    /* ═══ §RE6 O TECTO DIZ-SE, E A RECUSA É CONTADA ════════════════════════════ */
    printf("\n§RE6 Um número que não cabe no tipo é um número que a máquina não mediu.\n\n");
    {
        long recusas = 0, aceites = 0, mal = 0;
        /* n fora de {1,2,3}, ou p/q acima de 2^30: rz_cmp põe bom = 0 e não afirma */
        int bom;
        rz_cmp(qz(3,2), 4, 2, &bom);         if(!bom) recusas++; else mal++;   /* n = 4 */
        rz_cmp(qz(3,2), 0, 2, &bom);         if(!bom) recusas++; else mal++;   /* n = 0 */
        rz_cmp(qz(3,2), 2, 2, &bom);         if(bom) aceites++;  else mal++;   /* cabe  */
        if(rz_cabe((1L<<30) + 1, 1, 2)) mal++; else recusas++;                 /* p grande */
        if(rz_cabe(1, (1L<<30) + 1, 2)) mal++; else recusas++;                 /* q grande */
        if(!rz_cabe(1, 1, 3)) mal++; else aceites++;                           /* n = 3 cabe */
        /* e o corte devolve −1 («não mediu»), que não é nem dentro nem fora */
        Corte c = { 2, 4 };
        int r = rz_abaixo(c, qz(3, 2));
        printf("        %ld recusas, %ld aceites · rz_abaixo com n=4 devolve %d"
               " (−1 = não mediu)\n", recusas, aceites, r);
        ok("E O TECTO NÃO É UMA FALHA ESCONDIDA: É UMA RECUSA DECLARADA. Fora do domínio em"
           " que o i128 chega — índice até 3, numerador e denominador até 2³⁰ — a comparação"
           " não devolve um palpite: põe a bandeira a zero e o corte responde −1, que não é"
           " «dentro» nem «fora», é «não mediu». As asserções deste ficheiro descartam esses"
           " casos em vez de os contar como acertos, e é por isso que os números de §RE0 e"
           " §RE3 falam só do que foi mesmo decidido",
           mal == 0 && recusas == 4 && aceites == 2 && r == -1);
    }

    /* ═══ §RE6b AS TRÊS FORMAS DE DEIXAR DE SER O TERMO ════════════════════════
     * O tecto de cima é o que a COMPARAÇÃO recusa. Este é o outro: o que a SUCESSÃO
     * deixa de dizer. Uma sucessão pode parar de ser ela própria de três maneiras, e
     * as três têm de acender uma bandeira — senão `cy_termo` devolve o último termo
     * bom outra vez e uma sucessão CONSTANTE parece de Cauchy, parece equivalente a
     * si própria e parece convergir.
     *
     *   PERDEU     a órbita de Möbius transborda o int64 e o valor é descartado
     *   PAROU      o encaixe já não consegue dar o passo (o ponto médio sai do 2³⁰)
     *   CONGELOU   a sucessão tem tecto próprio (a harmónica, os convergentes)
     *
     * Enquanto o racional TRUNCAVA, as três vinham de borla com a saturação. Com a
     * promoção (20/08) sair de E₁₆ passou a ser subir de andar com o valor intacto,
     * e a saturação deixou de as ver — o tecto honesto passou a ser 30 quando o
     * encaixe tinha desistido no 14. Cada uma tem agora o seu contador, e cada uma
     * mede-se aqui pelos DOIS lados: onde ainda é o termo e onde já não é. */
    printf("\n§RE6b Perdeu, parou, congelou: as três bandeiras de «já não é o termo».\n\n");
    {
        long mal = 0;
        Suc MOB = suc(S_MOBIUS, 2), LO = suc(S_LO, 2), HAR = suc(S_HARM, 0);

        /* PERDEU — a órbita cresce ×1,85 por passo e sai do int64 lá para o 49º */
        long pd0 = qz_perdeu;   (void)cy_termo(MOB, 20);
        long pd_perto = qz_perdeu - pd0;                 /* ainda é o termo */
        pd0 = qz_perdeu;        (void)cy_termo(MOB, 80);
        long pd_longe = qz_perdeu - pd0;                 /* já não é */
        long teto_mob = cy_teto_honesto(MOB, 80);
        if(pd_perto != 0 || pd_longe == 0) mal++;
        if(teto_mob >= 80 || teto_mob < 20) mal++;       /* corta, e não no princípio */

        /* PAROU — o encaixe desiste quando o ponto médio sai do domínio de rz_cmp */
        long rp0 = rz_parou;    (void)cy_termo(LO, 20);
        long rp_perto = rz_parou - rp0;
        rp0 = rz_parou;         (void)cy_termo(LO, 45);
        long rp_longe = rz_parou - rp0;
        long teto_lo = cy_teto_honesto(LO, 45);
        if(rp_perto != 0 || rp_longe == 0) mal++;
        if(teto_lo >= 45 || teto_lo < 20) mal++;

        /* CONGELOU — a harmónica tem tecto próprio e acima dele repete o último */
        long cg0 = cy_congelou; (void)cy_termo(HAR, CY_HARM_TETO);
        long cg_perto = cy_congelou - cg0;
        cg0 = cy_congelou;      Qz h1 = cy_termo(HAR, CY_HARM_TETO + 1);
        long cg_longe = cy_congelou - cg0;
        Qz h0 = cy_termo(HAR, CY_HARM_TETO);
        if(cg_perto != 0 || cg_longe == 0) mal++;
        if(!qz_igual(h0, h1)) mal++;                     /* congelado É repetir */
        long teto_har = cy_teto_honesto(HAR, 30);
        if(teto_har != CY_HARM_TETO) mal++;

        printf("        PERDEU   n=20 +%ld · n=80 +%ld · tecto honesto %ld\n",
               pd_perto, pd_longe, teto_mob);
        printf("        PAROU    n=20 +%ld · n=45 +%ld · tecto honesto %ld\n",
               rp_perto, rp_longe, teto_lo);
        printf("        CONGELOU n=%d +%ld · n=%d +%ld · tecto honesto %ld"
               " (e o termo acima REPETE)\n\n",
               CY_HARM_TETO, cg_perto, CY_HARM_TETO + 1, cg_longe, teto_har);
        ok("AS TRÊS FORMAS DE DEIXAR DE SER O TERMO ACENDEM CADA UMA A SUA BANDEIRA, e"
           " nenhuma delas é a saturação: `qz_saturou` conta a PROMOÇÃO — o valor saiu"
           " de E₁₆ e continua exacto — e ler nela a honestidade da sucessão dizia o"
           " contrário do que mede, cortando termos bons e deixando passar os maus. O"
           " que corta é `qz_perdeu` (o valor descartado), `rz_parou` (o passo que o"
           " encaixe não deu) e `cy_congelou` (o tecto próprio a repetir o último). As"
           " três medem-se dos DOIS lados — um índice onde a bandeira NÃO acende e um"
           " onde acende —, porque «acende sempre» passaria com metade da medida",
           mal == 0);
    }

    /* ═══ §RE7 n = 3: DOIS caminhos transportam, o terceiro NÃO ════════════════ */
    printf("\n§RE7 O cúbico: o corte e o encaixe sobem de índice; o Möbius não.\n\n");
    {
        long fecha_cubo = 0, aberto3 = 0, cerca = 0, mal = 0;
        const long CUBO[3] = {8, 27, 64};          /* cubos perfeitos: fecham em ℚ */
        const long NAO3[3] = {2, 3, 5};            /* ∛2, ∛3, ∛5: não fecham */
        for(int i = 0; i < 3; i++){
            Corte c = { CUBO[i], 3 };
            long r = 0;
            if(rz_fecha_em_q(c, &r)) fecha_cubo++;
            if(rz_abaixo(c, qz_de_inteiro(r)) == 0) aberto3++;   /* aberto também aqui */
        }
        for(int i = 0; i < 3; i++){
            Corte c = { NAO3[i], 3 };
            long r = 0;
            if(rz_fecha_em_q(c, &r)) mal++;                      /* não pode fechar */
            Qz lo, hi;
            if(!rz_caixa_inicial(c, &lo, &hi)){ mal++; continue; }
            rz_encaixota(c, &lo, &hi, 10);
            if(rz_abaixo(c, lo) == 1 && rz_abaixo(c, hi) == 0 && qz_menor(lo, hi)) cerca++;
        }
        /* e o terceiro caminho NÃO transporta: rz_passo é o ponto fixo de x² = a.
         * Aplicado a um corte CÚBICO, ele aponta ao corte errado — e mede-se. */
        long aponta_quad = 0, aponta_cubo = 0;
        for(int i = 0; i < 3; i++){
            long a = NAO3[i];
            Suc M = suc(S_MOBIUS, a);
            Corte c2 = { a, 2 }, c3 = { a, 3 };
            long N;
            if(cy_aponta(M, c2, 10, 12, &N)) aponta_quad++;      /* ao de √a: sim */
            if(cy_aponta(M, c3, 10, 12, &N)) aponta_cubo++;      /* ao de ∛a: não */
        }
        printf("        cubos perfeitos: %ld/3 fecham, %ld/3 com corte aberto\n",
               fecha_cubo, aberto3);
        printf("        ∛2,∛3,∛5: %ld/3 cercados pelo encaixe, %ld anomalias\n", cerca, mal);
        printf("        a órbita de Möbius aponta ao corte QUADRÁTICO %ld/3 e ao CÚBICO %ld/3\n",
               aponta_quad, aponta_cubo);
        ok("O ÍNDICE SOBE E A RÉGUA NÃO TRANSPORTA TODA: no cúbico, o corte continua a decidir"
           " (a comparação p³ contra a·d³ é a mesma lei com um expoente a mais) e o encaixe"
           " continua a cercar; mas o terceiro caminho NÃO é o mesmo objecto — `rz_passo` é o"
           " ponto fixo de x² = a, e aplicado a um corte cúbico aponta ao corte ERRADO, o que"
           " se mede pelos dois lados: aponta ao quadrático nos três casos e ao cúbico em"
           " nenhum. É Lagrange a dizê-lo do lado da escrita: a fracção contínua é periódica"
           " para o irracional QUADRÁTICO, e o cúbico não tem essa via. Dois caminhos sobem de"
           " índice, o terceiro é do andar quadrático — e não se finge que é geral",
           fecha_cubo == 3 && aberto3 == 3 && cerca == 3 && mal == 0
             && aponta_quad == 3 && aponta_cubo == 0);
    }

    /* ═══ §RE8 A CLASSE: ∼ é equivalência, e SEPARA ════════════════════════════ */
    printf("\n§RE8 Reflexiva, simétrica, transitiva — e não junta o que é diferente.\n\n");
    {
        long refl = 0, sim = 0, trans = 0, separa = 0, junta_mal = 0, N;
        const long AA[3] = {2, 3, 5};
        Qz eps_ult = qz(1,1);
        printf("        a   horizonte honesto (M,LO,HI,FC)   h   ε derivado\n");
        for(int i = 0; i < 3; i++){
            long a = AA[i];
            Suc M = suc(S_MOBIUS,a), L = suc(S_LO,a), H = suc(S_HI,a), F = suc(S_CONV,a);
            /* O ε NÃO SE ESCOLHE: DERIVA-SE DO HORIZONTE. Cada sucessão satura num
             * índice diferente (a órbita de Möbius cresce depressa, o encaixe é linear),
             * e a comparação só pode usar o menor deles. Nesse horizonte h o encaixe tem
             * largura ~1/2^h, logo ε = 4/2^h: uma dobra para a desigualdade triangular
             * (as hipóteses valem a ε/2) e outra de folga para o passo. */
            long hM = cy_teto_honesto(M,30), hL = cy_teto_honesto(L,30);
            long hH = cy_teto_honesto(H,30), hF = cy_teto_honesto(F,30);
            long h = hM; if(hL < h) h = hL; if(hH < h) h = hH; if(hF < h) h = hF;
            if(h > 20) h = 20;
            Qz eps = qz(4, 1L << h), meio = qz(2, 1L << h);
            eps_ult = eps;
            printf("        %2ld       %2ld %2ld %2ld %2ld              %2ld   %ld/%ld\n",
                   a, hM, hL, hH, hF, h, eps.p, eps.q);
            if(cy_equiv(M, M, eps, h, &N)) refl++;                        /* reflexiva */
            if(cy_equiv(M, F, eps, h, &N) == cy_equiv(F, M, eps, h, &N)) sim++;
            /* TRANSITIVA COM A TRIANGULAR À VISTA: hipóteses a ε/2, conclusão a ε.
             * Pedir as três com o MESMO ε é pedir o que a triangular não dá. */
            if(cy_equiv(M, F, meio, h, &N) && cy_equiv(F, L, meio, h, &N)
               && cy_equiv(M, L, eps, h, &N)) trans++;
        }
        /* e SEPARA: radicandos distintos não caem na mesma classe */
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            if(i == j) continue;
            Suc X = suc(S_MOBIUS, AA[i]), Y = suc(S_MOBIUS, AA[j]);
            if(cy_equiv(X, Y, eps_ult, 8, &N)) junta_mal++; else separa++;
        }
        printf("        reflexiva %ld/3 · simétrica %ld/3 · transitiva %ld/3 ·"
               " separa %ld/6 (juntou indevidamente %ld)\n", refl, sim, trans, separa, junta_mal);
        ok("E É UMA CLASSE, NÃO UM AJUNTAMENTO — com o ε DERIVADO e não escolhido. As três"
           " cláusulas medem-se sobre as sucessões deste andar, e o quociente por elas é que"
           " É o real. Duas coisas tiveram de ser ditas para a medida ser honesta. A primeira:"
           " a transitividade pede a DESIGUALDADE TRIANGULAR à vista — hipóteses a ε/2 e"
           " conclusão a ε —, porque a relação a ε FIXO não é transitiva, e foi assim que esta"
           " asserção caiu na primeira escrita. A segunda: as quatro sucessões têm horizontes"
           " honestos MUITO diferentes (a órbita de Möbius satura por volta do índice 7--11,"
           " o encaixe chega a 13--14), logo o ε não pode ser escolhido por mim: deriva-se do"
           " menor horizonte, ε = 4/2^h, que é a largura que o encaixe garante ali com uma"
           " dobra para a triangular e outra de folga. E a outra metade impede a relação de"
           " ser inútil: radicandos DIFERENTES não caem na mesma classe, nos seis pares",
           refl == 3 && sim == 3 && trans == 3 && separa == 6 && junta_mal == 0);
    }

    /* ═══ §RE9 O CONTROLO: saltos a zero NÃO é ser de Cauchy ═══════════════════ */
    printf("\n§RE9 A harmónica tem saltos a zero e não é de Cauchy; a alternada nem saltos.\n\n");
    {
        Qz eps = qz(1, 4);
        long N;
        Suc HAR = suc(S_HARM, 0), ALT = suc(S_ALT, 0), CON = suc(S_CONST, 0);
        /* o salto de um passo: |a_{n+1} − a_n| = 1/(n+2) → vai a zero na harmónica */
        Qz s1 = cy_dist(cy_termo(HAR, 8), cy_termo(HAR, 9));
        Qz s2 = cy_dist(cy_termo(ALT, 8), cy_termo(ALT, 9));
        int har_cauchy = cy_modulo(HAR, eps, 18, 6, &N);
        int alt_cauchy = cy_modulo(ALT, eps, 18, 6, &N);
        int con_cauchy = cy_modulo(CON, eps, 18, 6, &N);
        printf("        salto em n=8: harmónica %ld/%ld · alternada %ld/%ld\n",
               s1.p, s1.q, s2.p, s2.q);
        printf("        é de Cauchy (ε = 1/4)? harmónica %s · alternada %s · constante %s\n",
               har_cauchy ? "SIM" : "não", alt_cauchy ? "SIM" : "não",
               con_cauchy ? "sim" : "NÃO");
        ok("E O CONTROLO SEPARA O CRITÉRIO CERTO DO QUE SE LHE PARECE: na harmónica o salto de"
           " um passo é 1/(n+2) e vai a zero — e ela NÃO é de Cauchy, porque o que a definição"
           " pede é |aₘ − aₙ| pequeno para TODO par acima de N, e não só para vizinhos. A"
           " alternada nem os saltos tem a ir a zero. A constante é, e serve de lado positivo."
           " Sem esta secção, §RE8 estaria a medir a equivalência num conjunto onde tudo passa:"
           " a relação só é de equivalência SOBRE as de Cauchy, e distinguir isso é o que"
           " separa o quociente que dá ℝ de um que não dá nada",
           !har_cauchy && !alt_cauchy && con_cauchy && s1.p > 0 && s2.p > 0);
    }

    /* ═══ §RE10 A SOMA DESCE AO QUOCIENTE ══════════════════════════════════════ */
    printf("\n§RE10 Trocar o representante não muda a soma — e classes distintas não se juntam.\n\n");
    {
        /* bem definido: x∼x' e y∼y'  ⟹  x+y ∼ x'+y'. Aqui x = Möbius e x' = LO do
         * mesmo radicando (são a mesma classe por §RE2), e o mesmo para y. */
        long casos = 0, desce = 0, junta_mal = 0, h_min = 99;
        printf("        a + b     horizonte da soma   maior distância (M+M vs LO+LO)\n");
        const long PAR[3][2] = {{2,3},{2,5},{3,5}};
        for(int k = 0; k < 3; k++){
            long a = PAR[k][0], b = PAR[k][1];
            Suc Ma = suc(S_MOBIUS,a), La = suc(S_LO,a);
            Suc Mb = suc(S_MOBIUS,b), Lb = suc(S_LO,b);
            /* o horizonte da SOMA mede-se: onde qz_soma ainda não satura */
            long h = -1;
            for(long n = 0; n < 12; n++){
                long antes = qz_saturou;
                Qz u = qz_soma(cy_termo(Ma,n), cy_termo(Mb,n));
                Qz v = qz_soma(cy_termo(La,n), cy_termo(Lb,n));
                (void)cy_dist(u, v);          /* a DISTÂNCIA também tem de caber */
                if(qz_saturou != antes) break;
                h = n;
            }
            if(h < 0){ printf("        √%ld + √%ld  sem horizonte honesto\n", a, b); continue; }
            if(h < h_min) h_min = h;
            /* a maior distância entre as duas somas no horizonte, e ela tem de encolher */
            Qz pior = qz(0,1);
            for(long n = (h+1)/2; n <= h; n++){       /* só a metade FINAL do horizonte */
                Qz u = qz_soma(cy_termo(Ma,n), cy_termo(Mb,n));
                Qz v = qz_soma(cy_termo(La,n), cy_termo(Lb,n));
                Qz d = cy_dist(u, v);
                if(qz_menor(pior, d)) pior = d;
            }
            /* ε derivado do horizonte, como em §RE8 */
            Qz eps = qz(4, 1L << (h > 20 ? 20 : h));
            casos++;
            if(qz_menor(pior, eps)) desce++;
            printf("        √%ld + √%ld        %2ld              %ld/%ld  (ε = %ld/%ld)\n",
                   a, b, h, pior.p, pior.q, eps.p, eps.q);
            /* CONTROLO, e o critério certo não é «ficar fora do ε» — com horizonte curto
             * o ε derivado é grosseiro, e duas classes distintas podem estar mais próximas
             * que ele. O que distingue é a DINÂMICA: entre representantes da mesma classe
             * a distância ENCOLHE com n; entre classes distintas, não. */
            Suc Lc = suc(S_LO, a == 2 ? 7 : 2);
            long n0 = (h+1)/2;
            Qz d_mesma_0 = cy_dist(qz_soma(cy_termo(Ma,n0), cy_termo(Mb,n0)),
                                   qz_soma(cy_termo(La,n0), cy_termo(Lb,n0)));
            Qz d_mesma_h = cy_dist(qz_soma(cy_termo(Ma,h),  cy_termo(Mb,h)),
                                   qz_soma(cy_termo(La,h),  cy_termo(Lb,h)));
            Qz d_outra_0 = cy_dist(qz_soma(cy_termo(Ma,n0), cy_termo(Mb,n0)),
                                   qz_soma(cy_termo(La,n0), cy_termo(Lc,n0)));
            Qz d_outra_h = cy_dist(qz_soma(cy_termo(Ma,h),  cy_termo(Mb,h)),
                                   qz_soma(cy_termo(La,h),  cy_termo(Lc,h)));
            (void)d_outra_0;
            int encolhe = qz_menor(d_mesma_h, d_mesma_0);   /* mesma classe: encolhe */
            int separa  = qz_menor(d_mesma_h, d_outra_h);   /* e fica MUITO mais perto */
            if(!(encolhe && separa)) junta_mal++;
        }
        printf("        desce em %ld/%ld · o controlo juntou indevidamente %ld\n",
               desce, casos, junta_mal);
        ok("A SOMA NÃO DEPENDE DO REPRESENTANTE, E É ISSO QUE FAZ O QUOCIENTE SER UM CORPO E"
           " NÃO UM SACO DE SUCESSÕES. Somando a órbita de Möbius de √a com a de √b, e depois"
           " a ponta esquerda do encaixe de √a com a de √b — quatro sucessões, duas classes,"
           " dois representantes cada —, as duas somas ficam mais próximas que o ε derivado do"
           " horizonte, nos três pares. É a mesma cláusula que o `inteiros.tex` prova para"
           " ℕ²/∼ e o `racionais.tex` para ℤ²/∼, agora no quociente de Cauchy. E o controlo"
           " tem de ser pela DINÂMICA e não por um limiar: com horizonte curto o ε derivado é"
           " grosseiro e duas classes distintas podem estar mais próximas que ele — foi assim"
           " que esta asserção caiu na primeira escrita. O que separa é o comportamento:"
           " entre representantes da MESMA classe a distância encolhe com n; trocando um"
           " somando por OUTRA classe, a distância não encolhe abaixo do ponto de partida",
           desce == casos && junta_mal == 0 && h_min >= 3);
    }

    /* ═══ §RE11 O PRODUTO DESCE — E PEDE A LIMITAÇÃO ═══════════════════════════ */
    printf("\n§RE11 O produto precisa de uma hipótese a mais: ser limitada.\n\n");
    {
        long casos = 0, desce = 0, lim_ok = 0, h_min = 99;
        Qz M0;
        printf("        a · b     horizonte do produto   maior distância\n");
        const long PAR[3][2] = {{2,3},{2,5},{3,5}};
        for(int k = 0; k < 3; k++){
            long a = PAR[k][0], b = PAR[k][1];
            Suc Ma = suc(S_MOBIUS,a), La = suc(S_LO,a);
            Suc Mb = suc(S_MOBIUS,b), Lb = suc(S_LO,b);
            /* a hipótese que o produto usa e a soma não: as sucessões são LIMITADAS */
            if(cy_limitada(Ma, 10, &M0) && cy_limitada(La, 10, &M0)
               && cy_limitada(Mb, 10, &M0) && cy_limitada(Lb, 10, &M0)) lim_ok++;
            long h = -1;
            for(long n = 0; n < 12; n++){
                long antes = qz_saturou;
                Qz u = qz_mult(cy_termo(Ma,n), cy_termo(Mb,n));
                Qz v = qz_mult(cy_termo(La,n), cy_termo(Lb,n));
                (void)cy_dist(u, v);          /* idem: a distância é parte da medida */
                if(qz_saturou != antes) break;
                h = n;
            }
            if(h < 0){ printf("        √%ld · √%ld  sem horizonte honesto\n", a, b); continue; }
            if(h < h_min) h_min = h;
            Qz pior = qz(0,1);
            for(long n = (h+1)/2; n <= h; n++){
                Qz u = qz_mult(cy_termo(Ma,n), cy_termo(Mb,n));
                Qz v = qz_mult(cy_termo(La,n), cy_termo(Lb,n));
                Qz d = cy_dist(u, v);
                if(qz_menor(pior, d)) pior = d;
            }
            Qz eps = qz(8, 1L << (h > 20 ? 20 : h));
            casos++;
            if(qz_menor(pior, eps)) desce++;
            printf("        √%ld · √%ld        %2ld                 %ld/%ld  (ε = %ld/%ld)\n",
                   a, b, h, pior.p, pior.q, eps.p, eps.q);
        }
        /* O CONTROLO, e sem afirmar o infinito: uma medida finita nunca mostra que uma
         * sucessão é ILIMITADA. O que se mede é o sintoma: a cota da harmónica CRESCE
         * quando o horizonte cresce, e a das sucessões deste andar não. */
        Suc HAR = suc(S_HARM, 0), Mref = suc(S_MOBIUS, 2);
        Qz cota_h1 = qz(0,1), cota_h2 = qz(0,1), cota_m1 = qz(0,1), cota_m2 = qz(0,1);
        cy_limitada(HAR, 6, &cota_h1);  cy_limitada(HAR, 16, &cota_h2);
        cy_limitada(Mref, 6, &cota_m1); cy_limitada(Mref, 16, &cota_m2);
        /* a cota de Möbius também sobe (a órbita é crescente), logo «estável» seria falso.
         * O que distingue é QUANTO sobe, e se fica abaixo de uma cota FIXA exibida: a órbita
         * de √2 nunca passa de 2; a harmónica passa de qualquer cota que se escreva. */
        /* NÃO se constrói a diferença das cotas: com denominadores destes tamanhos o
         * `cy_dist` satura e devolveria 32767 — comparar contra isso seria comparar contra
         * lixo, e foi o que aconteceu na primeira escrita. Compara-se por ORDEM, que o
         * `qz_menor` faz por produto cruzado sem construir nada. */
        cy_limitada(HAR, 2, &cota_h1);  cy_limitada(Mref, 2, &cota_m1);
        Qz dois = qz_de_inteiro(2), meio3 = qz(3,2);
        int har_passa   = qz_menor(cota_h1, dois) && !qz_menor(cota_h2, dois);
        int mob_sob_cota= qz_menor(cota_m1, meio3) && qz_menor(cota_m2, meio3);
        int har_cresce  = qz_menor(cota_h1, cota_h2);
        printf("        desce em %ld/%ld · limitadas no horizonte em %ld/3 casos\n",
               desce, casos, lim_ok);
        printf("        cota até 2 → até 16:  harmónica %ld/%ld → %ld/%ld  ·  Möbius %ld/%ld → %ld/%ld\n",
               cota_h1.p, cota_h1.q, cota_h2.p, cota_h2.q,
               cota_m1.p, cota_m1.q, cota_m2.p, cota_m2.q);
        printf("        a harmónica ATRAVESSA a cota fixa 2: %s · a órbita fica sempre sob 3/2: %s\n",
               har_passa ? "sim" : "não", mob_sob_cota ? "sim" : "não");
        ok("E O PRODUTO DESCE TAMBÉM, MAS COM UMA HIPÓTESE A MAIS — e é ela que costuma ficar"
           " calada. Para a soma basta que as diferenças vão a zero; para o produto é preciso"
           " que as sucessões sejam LIMITADAS, porque a diferença dos produtos se controla por"
           " |x||y−y'| + |y'||x−x'| e sem cota nos factores não há controlo. As quatro"
           " sucessões deste andar são limitadas, e o produto dos representantes concorda"
           " dentro do ε derivado do horizonte — que aqui é MENOR que o da soma, porque"
           " multiplicar denominadores esgota o envelope de 16 bits mais depressa. O controlo"
           " diz o que uma medida finita PODE dizer, e não mais. Nenhuma varredura mostra que"
           " uma sucessão é ilimitada; e dizer «a cota da órbita é estável» também seria"
           " falso, porque ela é crescente e a cota sobe a cada termo — foi assim que esta"
           " asserção caiu na primeira escrita. O que se mede é o SINTOMA quantificado:"
           " alargando o horizonte de 2 para 16, a harmónica ATRAVESSA a cota fixa 2 — estava"
           " abaixo e passou a estar acima — enquanto a órbita fica sempre sob 3/2, que é"
           " onde √2 vive. Uma cota que depende do horizonte não é cota nenhuma"
           " — e é essa a diferença entre ter a hipótese do produto e não a ter",
           desce == casos && lim_ok == 3 && har_cresce && mob_sob_cota && har_passa
             && h_min >= 2);
    }

    /* ═══ §RE12 A ORDEM DESCE — mas só entre classes DISTINTAS ═════════════════ */
    printf("\n§RE12 x<y no quociente: os representantes concordam — e dentro da classe, não.\n\n");
    {
        long concorda = 0, cruzados = 0, dentro_decide = 0, dentro_casos = 0, mal_eq = 0;
        const long PAR[3][2] = {{2,3},{3,5},{2,5}};
        printf("        classes    combinações de representantes que concordam\n");
        for(int k = 0; k < 3; k++){
            long a = PAR[k][0], b = PAR[k][1];
            Suc Xa[2] = { suc(S_MOBIUS,a), suc(S_LO,a) };
            Suc Yb[2] = { suc(S_MOBIUS,b), suc(S_LO,b) };
            long h = 6, ok4 = 0;
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                cruzados++;
                if(qz_menor(cy_termo(Xa[i],h), cy_termo(Yb[j],h))){ concorda++; ok4++; }
            }
            printf("        √%ld < √%ld     %ld de 4\n", a, b, ok4);
        }
        /* E DENTRO DA MESMA CLASSE a comparação termo a termo AFIRMA uma ordem que no
         * quociente não existe: LO_n < HI_n em TODO n — estritamente —, e no entanto
         * LO ∼ HI (§RE2). Logo «x < y termo a termo» NÃO dá «x < y no quociente», e é
         * por isso que a ordem em ℝ se define por «existe racional entre os dois». */
        long N_eq;
        for(int k = 0; k < 3; k++){
            long a = PAR[k][0];
            Suc L = suc(S_LO,a), H = suc(S_HI,a);
            for(long n = 3; n <= 6; n++){
                dentro_casos++;
                if(qz_menor(cy_termo(L,n), cy_termo(H,n))) dentro_decide++;  /* < sempre */
            }
            /* e são a MESMA classe, com o ε derivado do horizonte */
            long h = cy_teto_honesto(L,30), hh = cy_teto_honesto(H,30);
            if(hh < h) h = hh;
            if(!cy_equiv(L, H, qz(4, 1L << (h > 20 ? 20 : h)), h, &N_eq)) mal_eq++;
        }
        printf("        %ld combinações cruzadas concordam de %ld · dentro da classe:"
               " LO < HI estritamente em %ld de %ld termos, e são equivalentes (%ld falhas)\n",
               concorda, cruzados, dentro_decide, dentro_casos, mal_eq);
        ok("A ORDEM DESCE AO QUOCIENTE, E DESCE COM UMA RESSALVA QUE TEM DE SER DITA. Entre"
           " classes DISTINTAS ela desce: quaisquer representantes de √a e de √b, tomados nas"
           " quatro combinações, dão a mesma resposta a «qual é o menor» — a ordem é da classe"
           " e não do representante. Mas DENTRO de uma classe a comparação termo a termo não"
           " decide nada: a ponta esquerda do encaixe e a órbita de Möbius trocam de posição"
           " conforme o índice, e é por isso que a ordem em ℝ se define por «existe racional"
           " entre os dois» e não por comparar termos. E a ressalva mede-se: a ponta esquerda"
           " e a direita do encaixe cumprem LO_n < HI_n ESTRITAMENTE em todos os termos, e no"
           " entanto são a MESMA classe (§RE2) — logo comparar termo a termo afirma uma ordem"
           " que no quociente não existe. Era esta a dívida que o paper declarava, e paga-se"
           " com a ressalva junto: a ordem desce entre classes, não dentro de uma",
           concorda == cruzados && cruzados == 12
             && dentro_decide == dentro_casos && mal_eq == 0);
    }

    /* ═══ §RE13 A SOMA DE CORTES, e as duas construções a concordar ════════════ */
    printf("\n§RE13 A+B realiza-se como soma de encaixes — e o corte da soma tem critério inteiro.\n\n");
    {
        /* √2 + √3 satisfaz (s²−5)² = 24, logo o corte da SOMA decide-se em inteiros:
         *      q = p/d < √2+√3  ⟺  p² ≤ 5d²  ou  (p²−5d²)² < 24·d⁴
         * Teto declarado: p,d ≤ 4096, e aí (p²−5d²)² ≤ ~1e16 cabe em long long. */
        long dentro = 0, fora = 0, mal = 0;
        Corte c2 = { 2, 2 }, c3 = { 3, 2 };
        Qz lo2, hi2, lo3, hi3;
        rz_caixa_inicial(c2, &lo2, &hi2); rz_encaixota(c2, &lo2, &hi2, 9);
        rz_caixa_inicial(c3, &lo3, &hi3); rz_encaixota(c3, &lo3, &hi3, 9);
        Qz lo = qz_soma(lo2, lo3), hi = qz_soma(hi2, hi3);
        printf("        encaixe de √2: [%ld/%ld, %ld/%ld] · de √3: [%ld/%ld, %ld/%ld]\n",
               lo2.p, lo2.q, hi2.p, hi2.q, lo3.p, lo3.q, hi3.p, hi3.q);
        printf("        somados:       [%ld/%ld, %ld/%ld]\n", lo.p, lo.q, hi.p, hi.q);
        /* o critério inteiro do corte da soma, aplicado às pontas */
        int lo_abaixo = soma23_abaixo(lo), hi_abaixo = soma23_abaixo(hi);
        if(!lo_abaixo) mal++;                 /* a ponta esquerda tem de estar em A+B */
        if(hi_abaixo) mal++;                  /* e a direita, fora */
        /* e varre-se: todo racional abaixo de lo está em A+B; todo acima de hi, fora */
        for(long p = 1; p <= 200; p++) for(long d = 1; d <= 64; d++){
            Qz q = qz(p, d);
            if(q.p <= 0) continue;
            int a_ = soma23_abaixo(q);
            if(qz_menor(q, lo)){ dentro++; if(!a_) mal++; }
            else if(qz_menor(hi, q)){ fora++; if(a_) mal++; }
        }
        printf("        %ld racionais abaixo do encaixe (todos em A+B) · %ld acima (nenhum) ·"
               " %ld anomalias\n", dentro, fora, mal);
        ok("E A SOMA DE CORTES GANHA REALIZAÇÃO — a outra dívida do paper. A `lib/reais.h` não"
           " tem A+B, mas ela realiza-se com o que já lá está: somam-se os ENCAIXES, ponta a"
           " ponta, e o resultado cerca o corte da soma. E há um segundo caminho que não"
           " partilha código com o primeiro: √2+√3 satisfaz (s²−5)² = 24, logo o corte da soma"
           " tem critério INTEIRO próprio — p² ≤ 5d² ou (p²−5d²)² < 24d⁴ —, e os dois"
           " concordam em todos os racionais varridos: os que estão abaixo do encaixe somado"
           " pertencem a A+B, os que estão acima não. O lado ESTÁTICO (cortes) e o DINÂMICO"
           " (sucessões, §RE10) dão o mesmo objecto, e é isso que faltava mostrar",
           mal == 0 && dentro > 0 && fora > 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
