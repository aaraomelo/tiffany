/* meio.c — o MEIO nos Teoremas 3 e 5 do `papers/aranha.tex`.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/meio tests/meio.c && /tmp/meio
 *
 * O Teor. 3 força as tabelas de B pela DISTRIBUTIVIDADE — «as duas composições
 * a formarem uma só estrutura». A pergunta é se um terceiro símbolo pelo meio
 * cabe no mesmo requisito, e a resposta mede-se varrendo: não cabe como MEIO,
 * cabe como símbolo. O meio é uma leitura, e vive um andar acima, onde o 2 se
 * inverte — que é o Teor. 5.
 *
 * NADA AQUI TEM VÍRGULA. O meio representa-se em QUARTOS: {0,½,1} escreve-se
 * {0,2,4}, e então m = (a+b)/2 e δ = (a−b)/2 são inteiros exactos, sem uma
 * divisão que não feche. A régua é a mesma dos dois lados.
 *
 *   §M1  a distributividade admite um terceiro SÍMBOLO e não um MEIO
 *   §M2  associatividade e distributividade são DUAS leis, e não recortam o mesmo
 *   §M3  a decomposição a = m+δ, b = m−δ: resíduo 0 — e o módulo perde METADE
 *   §M4  a conservação da norma SAI daqui, e não foi pedida
 */
#include "unidade.h"
#include <stdio.h>

/* os três símbolos: 0, o meio, 1 */
enum { Z = 0, M = 1, U = 2, N = 3 };
static const char *nm[N] = { "0", "1/2", "1" };

/* uma tabela é 3x3; as entradas de B estão fixas pelo Teor. 3 e as três casas
 * que o meio acrescenta são as livres (a tabela é comutativa) */
typedef int Tab[N][N];

static void monta(Tab t, const int b[2][2], int m0, int mm, int m1){
    t[Z][Z] = b[0][0]; t[Z][U] = b[0][1];
    t[U][Z] = b[1][0]; t[U][U] = b[1][1];
    t[M][Z] = t[Z][M] = m0;
    t[M][M] = mm;
    t[M][U] = t[U][M] = m1;
}

static int associativa(const Tab t){
    for(int a = 0; a < N; a++) for(int b = 0; b < N; b++) for(int c = 0; c < N; c++)
        if(t[t[a][b]][c] != t[a][t[b][c]]) return 0;
    return 1;
}

/* P distribui sobre S:  a·(b⊕c) = (a·b)⊕(a·c) */
static int distribui(const Tab P, const Tab S){
    for(int a = 0; a < N; a++) for(int b = 0; b < N; b++) for(int c = 0; c < N; c++)
        if(P[a][S[b][c]] != S[P[a][b]][P[a][c]]) return 0;
    return 1;
}

int main(void){
    /* as tabelas de B, tal como o Teor. 3 as força */
    const int somaB[2][2] = { {Z, U}, {U, Z} };     /* 0+0=0, 0+1=1, 1+1=0 */
    const int prodB[2][2] = { {Z, Z}, {Z, U} };     /* 0·x=0, 1·1=1       */

    printf("O MEIO nos Teoremas 3 e 5 do aranha.tex\n\n");

    /* ── §M1 ─────────────────────────────────────────────────────────────────
     * O requisito é o do próprio Teor. 3: distributividade, com 0 neutro de ⊕,
     * 1 neutro de · e 0 absorvente. Varrem-se as 729 maneiras de preencher as
     * seis casas livres. O que se procura é se ALGUMA delas põe o meio a ser o
     * CENTRO — e o centro somado consigo dá o todo, ½⊕½ = 1. */
    {
        long pares = 0, passam = 0, com_centro = 0, mm_zero = 0;
        int achou_s[3][3], achou_p[3][3];
        for(int s0 = 0; s0 < N; s0++) for(int sm = 0; sm < N; sm++) for(int s1 = 0; s1 < N; s1++)
        for(int p0 = 0; p0 < N; p0++) for(int pm = 0; pm < N; pm++) for(int p1 = 0; p1 < N; p1++){
            Tab S, P;
            monta(S, somaB, s0, sm, s1);
            monta(P, prodB, p0, pm, p1);
            pares++;
            int ok_n = 1;
            for(int x = 0; x < N; x++){
                if(S[x][Z] != x) ok_n = 0;          /* 0 neutro de ⊕      */
                if(P[x][U] != x) ok_n = 0;          /* 1 neutro de ·      */
                if(P[x][Z] != Z) ok_n = 0;          /* 0 absorvente de ·  */
            }
            if(!ok_n) continue;
            if(!distribui(P, S)) continue;
            if(passam < 3){ achou_s[passam][0] = s0; achou_s[passam][1] = sm;
                            achou_s[passam][2] = s1;
                            achou_p[passam][0] = p0; achou_p[passam][1] = pm;
                            achou_p[passam][2] = p1; }
            passam++;
            if(sm == Z) mm_zero++;
            if(sm == U) com_centro++;              /* ½⊕½ = 1: o meio É o centro */
        }
        printf("§M1  varridos %ld pares de tabelas (3^6)\n", pares);
        printf("     passam a distributividade + neutros: %ld\n", passam);
        for(long k = 0; k < passam && k < 3; k++)
            printf("        ⊕: 1/2+0=%-4s 1/2+1/2=%-4s 1/2+1=%-4s"
                   "   ·: 1/2·0=%-4s 1/2·1/2=%-4s 1/2·1=%s\n",
                   nm[achou_s[k][0]], nm[achou_s[k][1]], nm[achou_s[k][2]],
                   nm[achou_p[k][0]], nm[achou_p[k][1]], nm[achou_p[k][2]]);
        printf("     com 1/2+1/2 = 0 (o meio é o seu próprio oposto): %ld\n", mm_zero);
        printf("     com 1/2+1/2 = 1 (o meio é o CENTRO):             %ld\n\n", com_centro);
        ok("A DISTRIBUTIVIDADE ADMITE UM TERCEIRO SÍMBOLO, E NÃO ADMITE UM MEIO. O requisito"
           " não é trazido de fora: é o que o Teor. 3 já usa para forçar as tabelas de B — as"
           " duas composições a formarem uma só estrutura. Pergunta-se o mesmo com um símbolo"
           " a mais no meio, varrendo as 729 maneiras de preencher as seis casas que ele"
           " acrescenta, com 0 neutro da soma, 1 neutro do produto e 0 absorvente. Passam"
           " TRÊS, e o que decide não é quantas são: é que as três têm 1/2+1/2 = 0. O centro"
           " exigiria 1/2+1/2 = 1 — o meio somado consigo dá o todo —, e NENHUMA o dá; em"
           " todas o meio é o seu próprio oposto, exactamente como os outros dois, que é o"
           " que faz dele mais um símbolo e não um meio. E as duas contagens são medidas"
           " SEPARADAMENTE, porque uma só delas não decidia nada: se ambas fossem zero a"
           " estrutura não admitiria o símbolo de todo, e é preciso ver que admite para poder"
           " dizer o que ela recusa. O meio não é um símbolo desta estrutura — é uma leitura"
           " dela, e vive onde o 2 se inverte, que é o §M3.",
           passam == 3 && mm_zero == 3 && com_centro == 0);
    }

    /* ── §M2 ─────────────────────────────────────────────────────────────────
     * «associatividade de um lado e distributividade do outro, como operações
     * duais»: são duas leis e não a mesma. A associatividade é a lei de UMA
     * composição; a distributividade é a lei ENTRE as duas. Mede-se que
     * recortam conjuntos diferentes — e onde a segunda vale nos DOIS sentidos. */
    {
        long as_soma = 0, as_prod = 0;
        for(int a = 0; a < N; a++) for(int b = 0; b < N; b++) for(int c = 0; c < N; c++){
            Tab S, P;
            monta(S, somaB, a, b, c);
            monta(P, prodB, a, b, c);
            if(associativa(S)) as_soma++;
            if(associativa(P)) as_prod++;
        }
        printf("§M2  das 27 extensões: ⊕ associativa em %ld, · associativa em %ld\n",
               as_soma, as_prod);

        /* o par (⊕,·) não é simétrico na distributividade — e já não é em B */
        int d_po = 1, d_op = 1, ce[3] = {0,0,0};
        for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++) for(int c = 0; c < 2; c++){
            if((a & (b ^ c)) != ((a & b) ^ (a & c))) d_po = 0;
            if((a ^ (b & c)) != ((a ^ b) & (a ^ c))){
                if(d_op){ ce[0] = a; ce[1] = b; ce[2] = c; }
                d_op = 0;
            }
        }
        printf("     EM B: · sobre ⊕ = %s   ·   ⊕ sobre · = %s  (falha em a=%d,b=%d,c=%d)\n",
               d_po ? "vale" : "NAO", d_op ? "vale" : "NAO", ce[0], ce[1], ce[2]);

        /* e com ∨/∧ — o par dual — vale nos DOIS sentidos */
        int r_ao = 1, r_oa = 1;
        for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++) for(int c = 0; c < 2; c++){
            int mx_bc = b > c ? b : c, mn_bc = b < c ? b : c;
            int mn_ab = a < b ? a : b, mn_ac = a < c ? a : c;
            int mx_ab = a > b ? a : b, mx_ac = a > c ? a : c;
            if((a < mx_bc ? a : mx_bc) != (mn_ab > mn_ac ? mn_ab : mn_ac)) r_ao = 0;
            if((a > mn_bc ? a : mn_bc) != (mx_ab < mx_ac ? mx_ab : mx_ac)) r_oa = 0;
        }
        printf("     EM B: ∧ sobre ∨ = %s   ·   ∨ sobre ∧ = %s  (o par DUAL)\n\n",
               r_ao ? "vale" : "NAO", r_oa ? "vale" : "NAO");

        ok("SÃO DUAS LEIS, E NÃO RECORTAM O MESMO. A associatividade é a lei de UMA composição;"
           " a distributividade é a lei ENTRE as duas — uma olha para dentro de cada face, a"
           " outra para o que as liga. Mede-se que são distintas em vez de se supor: das 27"
           " extensões de ⊕ ao trio, a associatividade deixa passar QUATRO, e a"
           " distributividade das duas juntas deixa TRÊS (§M1) — conjuntos diferentes, com"
           " tamanhos diferentes, apurados na mesma varredura. E O PAR (⊕,·) NÃO É SIMÉTRICO"
           " NESSA LEI: o produto distribui sobre a soma, e a soma NÃO distribui sobre o"
           " produto — o que se afirma com o escopo à frente, porque isto já falha em B, sem"
           " meio nenhum, em a=1,b=0,c=1. Atribuir essa falha ao meio seria lê-la onde ela não"
           " nasce. Onde a distributividade VALE NOS DOIS SENTIDOS é no par (∨,∧), e mede-se"
           " também, porque é esse o par que o Teor. 5 usa para ler a operação a partir do"
           " meio: as duas operações duais são o máximo e o mínimo, não a soma e o produto.",
           as_soma == 4 && d_po && !d_op && r_ao && r_oa);
    }

    /* ── §M3 ─────────────────────────────────────────────────────────────────
     * O Teor. 5 a partir do meio. Em QUARTOS, {0,½,1} = {0,2,4}: então
     * m = (a+b)/2 e δ = (a−b)/2 são inteiros, e a volta é a+b e a−b outra vez.
     * O módulo NÃO entra aqui — entra só quando se pergunta qual é o maior. */
    {
        const int T[3] = { 0, 2, 4 };              /* 0, 1/2, 1 — em quartos */
        long mau_volta = 0, pares_ord = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            int a = T[i], b = T[j];
            int m = (a + b) / 2, d = (a - b) / 2;   /* exactos: a,b são pares */
            if((a + b) % 2 || (a - b) % 2) mau_volta++;   /* nunca acontece   */
            if(m + d != a || m - d != b) mau_volta++;
            pares_ord++;
        }
        /* e o que o MÓDULO perde: quantas imagens distintas dá (a∨b, a∧b)? */
        int vis[9][2]; long nimg = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            int a = T[i], b = T[j];
            int mx = a > b ? a : b, mn = a < b ? a : b;
            int novo = 1;
            for(long k = 0; k < nimg; k++) if(vis[k][0] == mx && vis[k][1] == mn) novo = 0;
            if(novo){ vis[nimg][0] = mx; vis[nimg][1] = mn; nimg++; }
        }
        printf("§M3  a = m+δ, b = m−δ nos %ld pares ORDENADOS: resíduo %ld\n",
               pares_ord, mau_volta);
        printf("     (a∨b, a∧b) dá %ld imagens para %ld pares — perde %ld\n\n",
               nimg, pares_ord, pares_ord - nimg);
        ok("A DECOMPOSIÇÃO PELO MEIO É EXACTA, E O MÓDULO É QUE PERDE METADE. Com a média"
           " m = (a+b)/2 e o desvio δ = (a−b)/2 — COM SINAL —, vale a = m+δ e b = m−δ, e isto"
           " é uma igualdade e não uma aproximação: o par (m,δ) e o par (a,b) são a mesma"
           " coisa escrita duas vezes, e a volta devolve o par ORDENADO com resíduo 0 nos nove"
           " casos. Não aparece módulo nenhum, e é esse o ponto: o módulo não é o objecto, é a"
           " INTENSIDADE, e a intensidade é metade — o que ela deixa cair é o sinal de δ, isto"
           " é, qual dos dois é qual. Isso não se afirma, MEDE-SE: os nove pares ordenados caem"
           " em SEIS imagens por (a∨b, a∧b), e as três que colidem são exactamente os pares"
           " trocados. Nove em nove de um lado, seis em nove do outro, e a diferença é o sinal."
           " É por isso que os extremos entram DEPOIS na leitura e não antes: ∨ e ∧ são"
           " m ± |δ|, e pôr o módulo à cabeça seria começar por deitar fora a metade que ainda"
           " se ia usar. E a aritmética é inteira: em quartos, {0,1/2,1} escreve-se {0,2,4} e"
           " as duas divisões por 2 fecham exactas — não há vírgula em lado nenhum.",
           mau_volta == 0 && nimg == 6);
    }

    /* ── §M4 ─────────────────────────────────────────────────────────────────
     * A conservação NÃO é hipótese: sai da decomposição. ∂ troca a com b, isto
     * é, troca o sinal de δ e fixa m; e como δ entra ao QUADRADO, o que ∂ move
     * não sobrevive. O controlo é a primeira potência, que não é invariante. */
    {
        const int T[3] = { 0, 2, 4 };
        long mau = 0, ctl_varia = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            int a = T[i], b = T[j];
            int m = (a + b) / 2, d = (a - b) / 2;
            if(m * m - d * d != a * b) mau++;                 /* ab = m² − δ²      */
            if((m * m - d * d) != (m * m - (-d) * (-d))) mau++;/* invariante por ∂  */
            /* CONTROLO: à primeira potência, δ ↦ −δ MOVE o valor — excepto onde
             * δ = 0, que é a diagonal. Sem isto a invariância de cima passaria
             * por a troca não fazer nada. */
            if(d != 0 && (m + d) == (m - d)) mau++;
            if(d != 0) ctl_varia++;
        }
        printf("§M4  ab = m² − δ² e invariante por δ ↦ −δ: %ld falhas\n", mau);
        printf("     CONTROLO — pares onde a troca MOVE de facto (δ ≠ 0): %ld de 9\n\n",
               ctl_varia);
        ok("A CONSERVAÇÃO DA NORMA SAI COMO TEOREMA, E NÃO FOI PEDIDA. Em nenhum sítio se pôs"
           " como requisito que ∂ conservasse coisa alguma além do que a Def. 1 diz: o que se"
           " impôs foram as leis — distributividade no §M1, e a decomposição no §M3. Daí o"
           " produto do par escreve-se, expandindo, ab = (m+δ)(m−δ) = m² − δ², e ∂ — que troca"
           " a com b — é exactamente δ ↦ −δ com m fixo. Como δ entra AO QUADRADO, o que ∂ move"
           " não sobrevive à expansão, e o produto é invariante: a forma quadrática do par"
           " aparece sozinha, sem se ter ido buscar métrica nenhuma. É esta a ordem, e ela"
           " importa: a norma DEPOIS das leis, porque posta antes seria uma hipótese a mais a"
           " fazer o trabalho, e o que se quer mostrar é que ela não é precisa. E O CONTROLO"
           " impede que a invariância passe por a troca não fazer nada: mede-se que δ ≠ 0 em"
           " SEIS dos nove pares, isto é, que na maioria deles ∂ move mesmo o desvio — se a"
           " troca fosse a identidade, qualquer expressão seria invariante e o teorema não"
           " diria nada.",
           mau == 0 && ctl_varia == 6);
    }

    return falhas ? 1 : 0;
}
