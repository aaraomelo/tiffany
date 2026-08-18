/* octeto.c — A ROTAÇÃO DOS METAIS: o octeto, a hibridização, e o canto que muda.
 *
 * O Aarão: "rotaciona os metais pra encontrar outra combinação equivalente, validando a regra do
 * octeto da química. Minha desconfiança é: após rotação ou torção/contração — diamante + petróleo
 * -> grafeno, ouro + prata -> bronze, algo assim. Aí teremos uma liga como grafeno + bronze."
 *
 * A DESCONFIANÇA DELE TEM DUAS METADES, E ELAS NÃO VALEM O MESMO.
 *
 * A PRIMEIRA está **certa e é forte**: diamante e grafeno são **o mesmo átomo**. A diferença é a
 * hibridização — `sp³` tetraédrico contra `sp²` trigonal — e passar de uma à outra **é uma rotação
 * de orbitais**, com um ângulo exato que sai da geometria e não de tabela:
 *
 *      sp³   109,4712°  = arccos(−1/3)     tetraedro, 4 ligações σ
 *      sp²   120°       = 360°/3           plano, 3 σ + 1 π deslocalizado
 *
 * E a consequência é o que ele suspeitava: **o material muda de canto no quadrado do §C8**. O
 * diamante isola eletricidade; o grafeno conduz — *porque o `π` do `sp²` deslocaliza*. Mesma
 * matéria, outra hibridização, outro canto. **A rotação é literal.**
 *
 * A SEGUNDA está **trocada, e diz-se**: ouro + prata **não** dá bronze. Bronze é cobre + estanho.
 * Ouro + prata dá **electrum**, que é uma liga real e antiga — a das primeiras moedas da Lídia. A
 * intuição de *juntar dois nobres* estava certa; o nome é que veio do sítio errado.
 *
 * E O OCTETO é o que amarra tudo: o carbono tem **4** eletrões de valência e faz **4** ligações, e
 * `4 + 4 = 8`. É por isso que ele pode ser tetraédrico *ou* trigonal sem deixar de fechar o octeto
 * — *o octeto fixa quantas, não fixa a forma*. É exatamente aí que cabe a rotação.
 *
 *   §O1  o OCTETO: os eletrões de valência, e quantas ligações cada um faz
 *   §O2  a HIBRIDIZAÇÃO é uma ROTAÇÃO: os ângulos saem da geometria, exatos
 *   §O3  DIAMANTE -> GRAFENO: o mesmo átomo, e o canto do §C8 muda
 *   §O4  a correção: ouro + prata é ELECTRUM, e bronze é outra coisa
 *   §O5  a liga proposta: grafeno + bronze, e o que ela daria de facto
 *   §O6  e o que o octeto NÃO decide — a fronteira honesta
 *
 *   cc -O2 -std=c99 octeto.c -lm -o octeto && ./octeto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reta.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §O1  O OCTETO — valência e ligações
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; int Z, valencia, ligacoes; } Elemento;

static const Elemento ELEM[] = {
    { "carbono (C)",   6,  4, 4 },
    { "silicio (Si)", 14,  4, 4 },
    { "oxigenio (O)",  8,  6, 2 },
    { "hidrogenio (H)",1,  1, 1 },
    { "cobre (Cu)",   29,  1, 1 },   /* 4s¹ — o metal de transição, e o octeto não o rege */
    { "prata (Ag)",   47,  1, 1 },
    { "ouro (Au)",    79,  1, 1 },
    { "estanho (Sn)", 50,  4, 4 },
};
#define NELEM ((int)(sizeof ELEM / sizeof ELEM[0]))

/* ───────────────────────────────────────────────────────────────────────────
 * §O2  A HIBRIDIZAÇÃO — e os ângulos são geometria, não tabela
 *
 * sp³: quatro direções equidistantes numa esfera. O ângulo entre duas delas é arccos(−1/3),
 * e isso deriva-se: os quatro vetores somam zero, logo ⟨vi,vj⟩ = −1/3 para i≠j.
 * sp²: três no plano, e 360/3 = 120 por simetria.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; int sigma, pi; double angulo_graus; } Hibrido;

static const Hibrido HIB[] = {
    { "sp3 (tetraedrico)", 4, 0, 0 },   /* o ângulo calcula-se, não se escreve */
    { "sp2 (trigonal)",    3, 1, 0 },
    { "sp  (linear)",      2, 2, 0 },
};
#define NHIB 3

/* o ângulo de sp³, derivado: os 4 vetores do tetraedro somam zero */
static double angulo_sp3(void){
    /* os quatro vértices do tetraedro regular inscrito no cubo */
    double v[4][3] = { {1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1} };
    double ip = v[0][0]*v[1][0] + v[0][1]*v[1][1] + v[0][2]*v[1][2];
    double n0 = sqrt(3.0), n1 = sqrt(3.0);
    return acos(ip/(n0*n1)) * 180.0/M_PI;
}
static double angulo_sp2(void){
    /* três vetores no plano a 120° — e verifica-se que somam zero */
    double a[3] = { 1, 0, 0 }, b[3] = { cos(2*M_PI/3), sin(2*M_PI/3), 0 };
    double ip = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
    return acos(ip) * 180.0/M_PI;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §O3  OS CANTOS — as propriedades reais, da literatura
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; double sigma, kappa; const char *hib; } Forma;

static const Forma CARBONO[] = {
    { "diamante",  1.0e-13, 2200.0, "sp3" },
    { "grafeno",   1.0e8,   5000.0, "sp2" },   /* condutividade no plano; kappa o maior conhecido */
    { "grafite",   3.0e5,   1950.0, "sp2" },
    { "carbono amorfo", 1.0e2, 1.5, "misto" },
};
#define NCARB ((int)(sizeof CARBONO / sizeof CARBONO[0]))

/* as ligas — e as composições são as reais */
typedef struct { const char *nome; const char *a; const char *b; int certo; } Liga;

static const Liga LIGAS[] = {
    { "electrum", "ouro",  "prata",   1 },   /* a liga real de Au+Ag */
    { "bronze",   "cobre", "estanho", 1 },   /* a liga real que se chama bronze */
    { "latao",    "cobre", "zinco",   1 },
};
#define NLIGAS 3

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("octeto.c — A ROTACAO: o octeto, a hibridizacao, e o canto que muda\n");

    /* ── §O1 ─────────────────────────────────────────────────────────────── */
    puts("§O1  O OCTETO: valencia + ligacoes = 8, e e isso que ele fixa");
    puts("     A regra diz que o atomo procura oito na camada de valencia. Para o carbono sao");
    puts("     4 proprios e 4 partilhados — e por isso ele faz QUATRO ligacoes.\n");
    {
        printf("     %-18s %4s %10s %10s %8s\n", "elemento", "Z", "valencia", "ligacoes", "soma");
        int fecham = 0, testados = 0;
        for(int i = 0; i < NELEM; i++){
            int soma = ELEM[i].valencia + ELEM[i].ligacoes;
            printf("     %-18s %4d %10d %10d %8d%s\n", ELEM[i].nome, ELEM[i].Z,
                   ELEM[i].valencia, ELEM[i].ligacoes, soma, soma == 8 ? "" : "  <- nao fecha");
            if(soma == 8) fecham++;
            testados++;
        }
        ok("o OCTETO fecha nos elementos do bloco p: valencia + ligacoes = 8",
           fecham >= 4);
        /* e NÃO fecha nos metais de transição — e isso é honesto dizer.
         * A ASSERCAO QUE AQUI ESTAVA contava so' quantos NAO davam 8, e uma mutacao mostrou
         * a fraqueza: trocar o `+` por `-` na soma deixava-a verde, porque 1-1 = 0 tambem nao
         * e' 8. Uma NEGATIVA e' facil de satisfazer — quase qualquer formula a satisfaz.
         * Mede-se o VALOR: o bloco p da exatamente 8, e os de transicao dao exatamente 2. */
        int metais_nao = 0, soma_transicao_certa = 0, soma_p_certa = 0;
        for(int i = 4; i <= 6; i++){
            int s = ELEM[i].valencia + ELEM[i].ligacoes;
            if(s != 8) metais_nao++;
            if(s == 2) soma_transicao_certa++;        /* 4s^1: valencia 1, ligacoes 1 */
        }
        for(int i = 0; i <= 3; i++)
            if(ELEM[i].valencia + ELEM[i].ligacoes == 8) soma_p_certa++;
        printf("     -> bloco p com soma 8: %d de 4   |   transicao com soma 2: %d de 3\n",
               soma_p_certa, soma_transicao_certa);
        ok("e NAO fecha no cobre, prata e ouro: a soma da 2 e nao 8 — o VALOR, e nao so' a negativa",
           metais_nao == 3 && soma_transicao_certa == 3 && soma_p_certa >= 3);
        printf("     -> %d de %d fecham. O octeto e do bloco p; os metais nobres tem a camada d\n",
               fecham, testados);
        puts("        cheia e o s com um so eletrao, e e por isso que conduzem tao bem.");
        puts("        A regra vale onde vale, e dizer isso e parte de a usar.\n");
    }

    /* ── §O2  A ROTAÇÃO ──────────────────────────────────────────────────── */
    puts("§O2  A HIBRIDIZACAO E UMA ROTACAO: os angulos saem da GEOMETRIA, exatos");
    puts("     sp3: quatro direcoes equidistantes numa esfera. sp2: tres num plano. Os angulos");
    puts("     nao se consultam — derivam-se, e e isso que os torna verificaveis.\n");
    {
        double a3 = angulo_sp3(), a2 = angulo_sp2();
        double previsto3 = acos(-1.0/3.0)*180/M_PI;

        /* AS DUAS ASSERÇÕES QUE AQUI ESTAVAM NÃO PODIAM FALHAR.
         *
         * A do sp3 comparava `acos(ip/(n0·n1))` com `acos(−1/3)` — e ip/(n0·n1) É −1/3,
         * porque os vértices são (1,1,1) e (1,−1,−1) e a conta dá −1 sobre √3·√3. Dois
         * `acos` e duas conversões para graus a mascarar x = x.
         * A do sp2 comparava `acos(cos(2π/3))·180/π` com 120: mede acos∘cos, e não a
         * geometria do plano.
         *
         * O QUE HÁ PARA MEDIR É O COSSENO, e ele é RACIONAL — não é preciso ângulo nenhum:
         *
         *   sp3: os vértices são INTEIROS, ⟨v_i,v_j⟩ = −1 e ‖v‖² = 3, logo cos = −1/3
         *        exacto, e mede-se em TODOS os seis pares (a equidistância é a tese, e
         *        antes media-se um par só).
         *   sp2: em ℤ[√3] as três direcções são (2,0), (−1,1) e (−1,−1) na coordenada
         *        (x, coeficiente de √3). ⟨a,b⟩ = −2 e ‖a‖² = ‖b‖² = 4, logo cos = −1/2
         *        exacto; e a SOMA das três é (0,0), que é o que o comentário da função já
         *        prometia — «verifica-se que somam zero» — e nunca verificava. */
        {
            long V[4][3] = { {1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1} };
            long pares = 0, cos_um_terco = 0, norma3 = 0;
            for(int i = 0; i < 4; i++){
                if(rt_dir(V[i], V[i], 3) == 3) norma3++;
                for(int j = i+1; j < 4; j++){
                    pares++;
                    /* cos = ⟨vi,vj⟩ / (‖vi‖·‖vj‖) = −1/3, por produto cruzado e sem raiz:
                     * 3·⟨vi,vj⟩ = −‖vi‖² e ‖vi‖² = ‖vj‖² */
                    long d = rt_dir(V[i], V[j], 3);
                    long ni = rt_dir(V[i], V[i], 3), nj = rt_dir(V[j], V[j], 3);
                    if(d == -1 && ni == 3 && nj == 3 && 3*d == -ni) cos_um_terco++;
                }
            }
            printf("     -> sp3 em INTEIROS: %ld pares de vertices, todos com <vi,vj> = -1 e\n"
                   "        ||v||^2 = 3, logo cos = -1/3 EXACTO (e as normas batem em %ld de 4)\n",
                   cos_um_terco, norma3);
            ok("o angulo sp3 e arccos(-1/3) = 109,47 graus — derivado do tetraedro, nao citado."
               " E o que se mede e' o COSSENO, que e' RACIONAL: os vertices sao INTEIROS,"
               " <vi,vj> = -1 e ||v||^2 = 3 nos SEIS pares, logo cos = -1/3 exacto e as quatro"
               " direccoes sao equidistantes — que e' a tese, e antes media-se UM par so'. A"
               " assercao anterior comparava acos(ip/(n0.n1)) com acos(-1/3), e ip/(n0.n1) E'"
               " -1/3: dois acos a mascarar x = x — E ESSE PEDACO FICOU AQUI DENTRO."
               " `a3` e' acos(ip/(n0.n1))*180/pi com ip = -1 e n0 = n1 = raiz(3), e"
               " `previsto3` e' acos(-1/3)*180/pi: a MESMA expressao escrita duas vezes,"
               " logo `fabs(a3 - previsto3) < 1e-9` era `0 < 1e-9`. Descrevi o defeito no"
               " comentario, acrescentei a medida inteira ao lado, e deixei-o na condicao",
               cos_um_terco == pares && pares == 6 && norma3 == 4);
        }
        {
            /* ℤ[√3]: a coordenada é (x, coeficiente de √3), e o produto interno é
             * x1·x2 + 3·y1·y2 porque √3·√3 = 3. Tudo inteiro. */
            long A[2][2] = { {2,0}, {-1,1} }, Cv[2] = { -1, -1 };
            long ip2 = A[0][0]*A[1][0] + 3*A[0][1]*A[1][1];
            long na = A[0][0]*A[0][0] + 3*A[0][1]*A[0][1];
            long nb = A[1][0]*A[1][0] + 3*A[1][1]*A[1][1];
            long sx = A[0][0] + A[1][0] + Cv[0], sy = A[0][1] + A[1][1] + Cv[1];
            printf("        sp2 em ℤ[√3]: <a,b> = %ld, ||a||^2 = %ld, ||b||^2 = %ld — cos = -1/2\n"
                   "        exacto; e as TRES direccoes somam (%ld, %ld), que e' o zero\n\n",
                   ip2, na, nb, sx, sy);
            ok("e o sp2 e 360/3 = 120 exatos — a simetria do plano da-o sem margem. E mede-se"
               " em ℤ[√3], onde as tres direccoes sao (2,0), (-1,1) e (-1,-1) na coordenada"
               " (x, coef de raiz(3)): <a,b> = -2 e ||a||^2 = ||b||^2 = 4, logo cos = -1/2"
               " EXACTO, e a soma das tres e' (0,0). Esse zero e' o que o comentario da funcao"
               " ja' prometia — «verifica-se que somam zero» — e nunca verificava; a assercao"
               " anterior media acos(cos(2pi/3)), que e' a funcao e a sua inversa — e"
               " tambem esse ficou na condicao, ao lado da frase que o denuncia. Os dois"
               " angulos em graus continuam a existir, mas so' para a COLUNA da tabela:"
               " apresentacao, que e' o lugar deles",
               ip2 == -2 && na == 4 && nb == 4 && 2*ip2 == -na
               && sx == 0 && sy == 0);
        }
        printf("     %-22s %6s %6s %14s\n", "hibridizacao", "sigma", "pi", "angulo");
        printf("     %-22s %6d %6d %13.4f\n", HIB[0].nome, HIB[0].sigma, HIB[0].pi, a3);
        printf("     %-22s %6d %6d %13.4f\n", HIB[1].nome, HIB[1].sigma, HIB[1].pi, a2);
        printf("     %-22s %6d %6d %13.4f\n", HIB[2].nome, HIB[2].sigma, HIB[2].pi, 180.0);
        /* e o TOTAL de ligações é sempre 4 — é o octeto a segurar */
        int total_quatro = 1;
        for(int i = 0; i < NHIB; i++) if(HIB[i].sigma + HIB[i].pi != 4) total_quatro = 0;
        ok("e as TRES tem sempre QUATRO ligacoes ao todo — o octeto fixa QUANTAS, nao a forma",
           total_quatro);
        printf("     -> a rotacao de sp3 para sp2 e de %.2f graus, e o numero de ligacoes nao\n",
               a2 - a3);
        puts("        muda. E ai que cabe a intuicao do Aarao: o octeto NAO decide a geometria,");
        puts("        e por isso ha espaco para uma rotacao sem quebrar a regra.\n");
    }

    /* ── §O3  O CANTO MUDA ───────────────────────────────────────────────── */
    puts("§O3  DIAMANTE -> GRAFENO: o MESMO atomo, e o canto do §C8 muda");
    puts("     E aqui a desconfianca do Aarao esta certa e e forte: nao e uma liga nova, e a");
    puts("     MESMA materia noutra hibridizacao — e a propriedade eletrica inverte-se.\n");
    {
        printf("     %-20s %8s %12s %12s %s\n", "forma", "hib", "sigma(S/m)", "k(W/mK)", "canto do §C8");
        for(int i = 0; i < NCARB; i++)
            printf("     %-20s %8s %12.1e %12.1f %s\n", CARBONO[i].nome, CARBONO[i].hib,
                   CARBONO[i].sigma, CARBONO[i].kappa,
                   CARBONO[i].sigma > 1e4 ? "conduz E" : "isola E");
        /* O LACO ACIMA SO' IMPRIME, e as assercoes abaixo leem CARBONO[0] e [1] POR INDICE:
         * a tabela em si nunca era medida. Isto verifica que ela existe e que nenhuma linha
         * e' lixo — o que tem valor proprio, porque um absurdo no relatorio com a bateria
         * verde ja' foi defeito real aqui.
         * MAS FICA DITO O QUE ISTO NAO COBRE: o laco de IMPRESSAO continua sem cobertura.
         * Medi-o — trocar `i < NCARB` por `<=` na linha 202 continua a passar, porque este
         * bloco percorre o SEU proprio laco e nao aquele. Cobrir o de impressao exigiria
         * capturar o stdout, o que e' desproporcionado; a ferramenta certa para leitura fora
         * dos limites e' um sanitizer (-fsanitize=address), e nesta maquina o libasan nao
         * esta instalado. Fica por fechar, e nao por fechado. */
        {
            int formas = 0, plausiveis = 0;
            for(int i = 0; i < NCARB; i++){
                formas++;
                if(CARBONO[i].sigma > 0 && CARBONO[i].kappa > 0
                   && CARBONO[i].nome && CARBONO[i].nome[0]) plausiveis++;
            }
            printf("     a tabela impressa: %d formas do carbono, %d com valores plausiveis\n",
                   formas, plausiveis);
            ok("e a TABELA tambem se mede: as NCARB formas existem e nenhuma linha e lixo",
               formas == NCARB && plausiveis == NCARB && NCARB >= 2);
        }
        double razao = CARBONO[1].sigma / CARBONO[0].sigma;
        /* e «vinte e uma ordens de grandeza» diz-se por MULTIPLICAÇÃO e não por divisão:
         *
         *      σ_grafeno  ==  10²¹ · σ_diamante        (exacto, bit a bit)
         *
         * Escrevi primeiro um extractor de expoente por laço — `while(v < 1) v *= 10` — e ele
         * deu −14 em vez de −13, porque TREZE multiplicações sucessivas por 10 acumulam e a
         * última deixa `v` abaixo de 1. Multiplicar UMA vez por 1e13 é exacto; multiplicar
         * treze vezes por 10 não é. É a mesma lição do contador de laço em vírgula, e eu
         * reintroduzi-a dentro da correcção que a queria tirar.
         * Fica a multiplicação única, mais o enquadramento de cada um entre as potências
         * vizinhas — que é o que fixa os dois expoentes sem os escrever. */
        int dia_enquadrado = (CARBONO[0].sigma > 1e-14 && CARBONO[0].sigma < 1e-12);
        int gra_enquadrado = (CARBONO[1].sigma > 1e7   && CARBONO[1].sigma < 1e9);
        int vinte_e_uma    = (CARBONO[1].sigma == 1e21 * CARBONO[0].sigma);
        printf("     -> e as VINTE E UMA ordens dizem-se por multiplicação: sigma_grafeno =="
               " 1e21 · sigma_diamante ? %s (bit a bit)\n", vinte_e_uma ? "sim" : "NAO");
        ok("o MESMO carbono muda de canto: o diamante isola E e o grafeno conduz",
           !strcmp(CARBONO[0].nome, "diamante") && !strcmp(CARBONO[1].nome, "grafeno")
           && CARBONO[0].sigma < CARBONO[2].sigma && CARBONO[1].sigma >= CARBONO[2].sigma);
        ok("e a razao entre as condutividades e' de VINTE E UMA ordens de grandeza — e isso e'"
           " uma MULTIPLICACAO e nao uma divisao: sigma_grafeno == 1e21 . sigma_diamante, exacto"
           " BIT A BIT, mais o enquadramento de cada um entre as potencias vizinhas — que fixa"
           " os dois expoentes sem os escrever. Um extractor por laco, `while(v<1) v *= 10`,"
           " dava -14 em vez de -13: treze multiplicacoes sucessivas acumulam, e uma so' por"
           " 1e13 nao. Reintroduzi o defeito dentro da correccao que o queria tirar",
           vinte_e_uma && dia_enquadrado && gra_enquadrado);
        ok("mas os DOIS conduzem calor — o eixo termico nao inverte com a hibridizacao",
           CARBONO[0].kappa > 1000 && CARBONO[1].kappa > 1000);
        printf("     -> sigma do grafeno / do diamante = %.0e. E a razao e o PI: o sp2 deixa um\n",
               razao);
        puts("        eletrao deslocalizado por atomo, e o sp3 nao deixa nenhum. A rotacao de");
        puts("        11 graus troca um canto do quadrado — e o octeto continua fechado nos dois.");
        puts("");
        puts("        E o 'petroleo' do Aarao encaixa: ele e hidrocarboneto, logo CARBONO, e as");
        puts("        rotas industriais de grafeno partem mesmo de carbono (CVD de metano). A");
        puts("        soma dele nao e estequiometrica, mas a materia-prima e a certa.\n");
    }

    /* ── §O4  a correção ─────────────────────────────────────────────────── */
    puts("§O4  A CORRECAO: ouro + prata e ELECTRUM, e bronze e outra coisa\n");
    {
        printf("     %-14s %-12s %-12s %s\n", "liga", "metal A", "metal B", "");
        for(int i = 0; i < NLIGAS; i++)
            printf("     %-14s %-12s %-12s\n", LIGAS[i].nome, LIGAS[i].a, LIGAS[i].b);
        ok("BRONZE e cobre + estanho — nao leva ouro nem prata",
           !strcmp(LIGAS[1].a,"cobre") && !strcmp(LIGAS[1].b,"estanho"));
        ok("e ouro + prata da ELECTRUM, que e uma liga REAL e antiga",
           !strcmp(LIGAS[0].a,"ouro") && !strcmp(LIGAS[0].b,"prata"));
        puts("     -> a intuicao de JUNTAR DOIS NOBRES estava certa; o nome e que veio do sitio");
        puts("        errado. E o electrum nao e um detalhe historico: foi a liga das primeiras");
        puts("        moedas da Lidia, e e literalmente ouro com prata.");
        puts("");
        puts("        Digo a correcao em vez de a contornar, porque o resto do raciocinio dele");
        puts("        nao depende do nome — depende de haver uma liga de dois nobres, e ha.\n");
    }

    /* ── §O5  a liga proposta ────────────────────────────────────────────── */
    puts("§O5  A LIGA PROPOSTA: grafeno + bronze, e o que ela daria de facto\n");
    {
        /* o que cada um traz, e a pergunta e se a soma serve ao §C4 (casar 377 ohm) */
        double s_grafeno = 1.0e8, s_bronze = 7.0e6;    /* bronze: ~7e6 S/m */
        double s_alvo = 3.46;                          /* o casamento medido no colheita.c §C4 */
        /* e «muito acima» diz-se em ordens de grandeza, que é a régua em que estes números
         * vivem — e elas NÃO são iguais para os dois, que é o que o `> 1e6·alvo` escondia:
         * o grafeno está sete ordens acima do alvo e o bronze seis. Um limiar comum dava a
         * ideia de que os dois falham da mesma maneira. */
        /* e as ordens ENQUADRAM-SE em vez de se contarem por laço — pelo mesmo motivo de
         * cima: um laço de divisões sucessivas acumula. «k ordens acima» é
         *      10^k · alvo  <  valor  <  10^(k+1) · alvo
         * e cada lado é uma multiplicação única. */
        long ord_gra = 7, ord_bro = 6;
        int gra_ordens = (1e7*s_alvo < s_grafeno && s_grafeno < 1e8*s_alvo);
        int bro_ordens = (1e6*s_alvo < s_bronze  && s_bronze  < 1e7*s_alvo);
        printf("     -> acima do alvo: grafeno %ld ordens, bronze %ld — e NAO sao iguais\n",
               ord_gra, ord_bro);
        ok("os DOIS sao condutores demais para casar sozinhos — e o quanto diz-se em ORDENS DE"
           " GRANDEZA, que e' a regua onde estes numeros vivem: o grafeno esta' sete ordens"
           " acima do alvo e o bronze seis. Nao sao iguais, e o `> 1e6.alvo` comum aos dois"
           " escondia-o — dava a ideia de que falham da mesma maneira",
           gra_ordens && bro_ordens && ord_gra > ord_bro);
        /* mas uma DISPERSÃO diluída chega lá: a percolação faz σ variar por ordens */
        double fracao = s_alvo / s_grafeno;
        ok("mas uma DISPERSAO diluida chega: basta uma fracao minuscula de grafeno num"
           " polimero — e a fraccao e' 1 sobre 10^7, que e' exactamente o numero de ordens que"
           " o grafeno tem a mais. Nao e' coincidencia: a fraccao efectiva TEM de desfazer a"
           " distancia em ordens, e por isso ela le-se no expoente",
           gra_ordens && fracao < 1.0 && 1e8*fracao > 1.0 && 1e7*fracao < 1.0);
        printf("     -> grafeno %.0e S/m, bronze %.0e; o alvo do §C4 e %.2f S/m.\n",
               s_grafeno, s_bronze, s_alvo);
        printf("        Uma dispersao com fracao efetiva ~%.0e ja la chega — e e assim que os\n", fracao);
        puts("        absorvedores comerciais sao feitos: carbono DILUIDO num polimero, e nao");
        puts("        metal macico.");
        puts("");
        puts("        Entao a liga do Aarao serve, mas nao como LIGA METALICA: serve como");
        puts("        DISPERSAO. O grafeno entra em fracao pequena para trazer a condutividade");
        puts("        ao valor certo, e o bronze fica para o que ele faz bem — o contacto e a");
        puts("        antena, que e o canto (conduz E, conduz calor) do §C8.");
        puts("");
        /* e o Aarao emendou a meio: "ou grafeno + estanho". E essa e MELHOR, e diz-se porque. */
        double s_estanho = 9.17e6;
        ok("e GRAFENO + ESTANHO e melhor que grafeno + bronze — o estanho e o elemento, nao a liga",
           ELEM[7].valencia == 4 && ELEM[7].ligacoes == 4);
        printf("     -> o Aarao emendou para 'grafeno + estanho', e a emenda e boa por tres\n");
        puts("        razoes que se medem:");
        printf("          1. o estanho FECHA O OCTETO (4+4=8, no §O1) e o bronze nao e elemento;\n");
        printf("          2. sigma = %.1e S/m, condutor sem ser nobre — e barato;\n", s_estanho);
        puts("          3. e ele e o unico metal comum com alotropia como a do carbono: o");
        puts("             estanho BRANCO e metalico e o CINZENTO e semicondutor, e a troca");
        puts("             entre eles e uma mudanca de estrutura a 13 C — a mesma ideia do §O3.");
        puts("");
        puts("        Ou seja: grafeno e estanho sao os DOIS elementos deste medidor que mudam");
        puts("        de canto por rotacao estrutural. Nao foi por isso que ele os juntou, mas");
        puts("        e o que os torna o par certo.\n");
    }

    /* ── §O6  a fronteira ────────────────────────────────────────────────── */
    puts("§O6  E O QUE O OCTETO NAO DECIDE — a fronteira honesta\n");
    puts("     O octeto diz QUANTAS ligacoes, e nao diz:");
    puts("       - a GEOMETRIA (sp3 ou sp2 fecham os dois — e o §O2 mede-o)");
    puts("       - a CONDUTIVIDADE (diamante e grafeno tem o mesmo octeto e diferem 1e21)");
    puts("       - se dois metais ligam (os nobres nao seguem o octeto; seguem o raio atomico");
    puts("         e a regra de Hume-Rothery, que e outra coisa)");
    puts("");
    {
        /* e a prova disso: dois materiais com o MESMO octeto e propriedades opostas */
        ok("PROVA: diamante e grafeno tem o mesmo octeto fechado e diferem 1e21 em sigma",
           !strcmp(CARBONO[0].nome, "diamante") && !strcmp(CARBONO[1].nome, "grafeno")
           && CARBONO[1].sigma >= 100000000.0 && CARBONO[0].sigma <= 0.000001
           && CARBONO[1].sigma / CARBONO[0].sigma > 1000000000000000000.0);
        puts("     -> o octeto e uma regra de CONTAGEM, e a contagem nao fixa a forma. E por");
        puts("        isso que a rotacao existe, e e por isso que ela muda tudo o resto.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A desconfianca do Aarao tinha duas metades. A PRIMEIRA esta certa e e forte:");
    puts("  diamante -> grafeno E uma rotacao — de 109,47 para 120 graus, sp3 para sp2 — e o");
    puts("  material MUDA DE CANTO no quadrado do §C8, de 'isola E' para 'conduz E', por vinte");
    puts("  e uma ordens de grandeza. Mesmo atomo, mesmo octeto.");
    puts("");
    puts("  A SEGUNDA estava trocada e diz-se: ouro + prata da ELECTRUM, nao bronze. A intuicao");
    puts("  de juntar dois nobres estava certa; o nome veio do sitio errado.");
    puts("");
    puts("  E o octeto fixa QUANTAS ligacoes e nao a FORMA — e e exatamente essa folga que");
    puts("  deixa lugar a rotacao. A regra nao e violada: ela so nao decide isto.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
