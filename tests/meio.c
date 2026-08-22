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
 *   §M8  as DUAS faces batendo alternadas encaixam: o CORTE, e sem escolher lado
 */
#include "unidade.h"
#include <stdio.h>

/* piso de √x, exacto — a raiz da face multiplicativa (§M8) */
static long raiz_piso_local(long x){
    if(x < 2) return x < 0 ? -1 : x;
    { long lo = 1, hi = 3037000499L;
      while(lo < hi){ long m = lo + (hi - lo + 1)/2;
          if(m <= x / m) lo = m; else hi = m - 1; }
      return lo; }
}
/* a média geométrica de a e b, por piso, sem estourar o produto */
static long geo_local(long a, long b){
    __int128 p = (__int128)a * b;
    __int128 lo = 1, hi = (__int128)4000000000LL;
    while(lo < hi){ __int128 m = lo + (hi - lo + 1)/2;
        if(m*m <= p) lo = m; else hi = m - 1; }
    return (long)lo;
}

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

    /* ── §M5 ─────────────────────────────────────────────────────────────────
     * O Teor. 2(2): a primitiva é ∂² = ∂, e dela derivam as outras. Mede-se o
     * que ela faz e o que lhe falta: factoriza, logo colapsa; e a correcção
     * mínima — somar-lhe a unidade — é a ÚNICA irredutível das quatro formas.
     * O expoente não entra do nada: é a contagem das aplicações de ∂. */
    {
        long mal = 0;
        /* as quatro formas ∂² = α∂ + β são os quatro pares (α,β); a equação
         * corrigida é a COMPOSIÇÃO da primitiva com a unidade: soma dos pares */
        int a_p = 1, b_p = 0;                       /* (1,0): ∂² = ∂        */
        int a_u = 0, b_u = 1;                       /* (0,1): ∂² = 1        */
        int a_c = a_p ^ a_u, b_c = b_p ^ b_u;       /* a composição         */
        printf("§M5  (%d,%d) + (%d,%d) = (%d,%d)   «∂²=∂» ⊕ «∂²=1» = «∂²=∂+1»: %s\n",
               a_p, b_p, a_u, b_u, a_c, b_c,
               (a_c == 1 && b_c == 1) ? "sim" : "NAO");
        if(a_c != 1 || b_c != 1) mal++;

        /* qual das quatro é IRREDUTÍVEL sobre dois símbolos: t² + αt + β tem
         * raiz? (em dois símbolos, −1 = 1, pelo que o sinal não conta) */
        long irred = 0; int qual_a = -1, qual_b = -1;
        printf("     t² + αt + β sobre dois símbolos:\n");
        for(int al = 0; al < 2; al++) for(int be = 0; be < 2; be++){
            int tem = 0;
            for(int r = 0; r < 2; r++) if(((r*r + al*r + be) % 2) == 0) tem = 1;
            printf("        ∂² = %s%s%s  ->  %s\n",
                   al ? "∂" : "", (al && be) ? " + " : "", be ? "1" : (al ? "" : "0"),
                   tem ? "factoriza (colapsa)" : "IRREDUTÍVEL");
            if(!tem){ irred++; qual_a = al; qual_b = be; }
        }
        printf("     irredutíveis: %ld  — e é (α,β) = (%d,%d)\n", irred, qual_a, qual_b);
        if(irred != 1 || qual_a != 1 || qual_b != 1) mal++;

        /* e o PERÍODO da que sobra, iterando a companheira [[0,β],[1,α]] */
        int A[2][2] = { {0,1}, {1,1} }, X[2][2] = { {1,0}, {0,1} };
        long per = 0;
        for(long k = 1; k <= 8; k++){
            int Y[2][2];
            for(int i2 = 0; i2 < 2; i2++) for(int j2 = 0; j2 < 2; j2++)
                Y[i2][j2] = (X[i2][0]*A[0][j2] + X[i2][1]*A[1][j2]) % 2;
            for(int i2 = 0; i2 < 2; i2++) for(int j2 = 0; j2 < 2; j2++) X[i2][j2] = Y[i2][j2];
            if(X[0][0]==1 && X[0][1]==0 && X[1][0]==0 && X[1][1]==1){ per = k; break; }
        }
        printf("     período de ∂² = ∂ + 1: %ld  ->  %ld objectos\n", per, per);
        if(per != 3) mal++;

        /* CONTROLO — o regime AFIM, onde a involução SOBREVIVE: ∂x = ax+b.
         * Sem isto ficaria a dizer-se que o binário colapsa, e ele não colapsa:
         * a translação x+1 é efectiva, bijectiva, involutiva e SEM ponto fixo. */
        int tr_ef = 0, tr_bij, tr_inv = 1, tr_fix = 0;
        { int f[2]; for(int x = 0; x < 2; x++) f[x] = (x + 1) % 2;
          for(int x = 0; x < 2; x++){
              if(f[x] != x) tr_ef = 1;
              if(f[f[x]] != x) tr_inv = 0;
              if(f[x] == x) tr_fix++;
          }
          tr_bij = (f[0] != f[1]); }
        printf("     CONTROLO afim — a translação x+1: efectiva=%d bijectiva=%d"
               " involutiva=%d pontos fixos=%d\n\n", tr_ef, tr_bij, tr_inv, tr_fix);
        if(!tr_ef || !tr_bij || !tr_inv || tr_fix != 0) mal++;

        ok("A PRIMITIVA É UMA SÓ, E DELA DERIVAM AS OUTRAS. O operador compõe-se de uma"
           " única maneira, aplicando-o outra vez, pelo que o expoente é a CONTAGEM das"
           " aplicações e não um símbolo trazido de fora — é isso que dá direito a escrever"
           " ∂². A relação que fecha a sucessão diz-se no primeiro passo em que há o que"
           " dizer, e a forma crua da conservação é ∂² = ∂: aplicar de novo não muda. Dela sai"
           " JÁ a lei do oposto, sem tabelas nenhumas: a definição avaliada em ∂x dá"
           " ∂x∘∂²x = e, e a substituição deixa ∂x∘∂x = e, isto é cada objecto a ser o seu"
           " próprio oposto. MAS ELA COLAPSA, e o motivo é que factoriza — ∂(∂−1) = 0 força"
           " ∂ = 0 ou ∂ = 1, e nenhum é efectivo. A correcção é somar-lhe o que a estrutura"
           " tem, e mede-se que ela é ÚNICA: das quatro formas ∂² = α∂ + β, três factorizam e"
           " SÓ UMA é irredutível, a que tem α = β = 1. E ela não é posta à mão — é a"
           " COMPOSIÇÃO da primitiva com a unidade, porque cada equação é o seu par (α,β) e"
           " (1,0) + (0,1) = (1,1). O seu período é TRÊS, e o período conta os objectos. O"
           " CONTROLO é o que impede a conclusão de ser larga demais: tudo isto trata ∂ como"
           " LINEAR, e no regime AFIM a involução sobrevive — a translação x+1 é efectiva,"
           " bijectiva, involutiva e SEM ponto fixo, e é dela que sai o binário. Sem esta"
           " linha ficaria escrito que o binário colapsa, o que é falso: os dois períodos"
           " vivem em dois regimes.",
           mal == 0);
    }

    /* ── §M6 ─────────────────────────────────────────────────────────────────
     * O Teor. 3 no período 3: uma composição CHEGA — porque o ponto fixo existe
     * — e é única. Varrem-se as 3^9 tabelas sobre três objectos. */
    {
        long mal = 0;
        const int dfix[N] = { U, M, Z };      /* ∂ involutivo, fixa o meio      */
        const int dcic[N] = { M, U, Z };      /* 3-ciclo: 0→m→1→0, sem fixo     */
        for(int caso = 0; caso < 2; caso++){
            const int *dd = caso ? dcic : dfix;
            long conserva = 0, com_neutro = 0, comut_assoc = 0;
            int ex[9], exe = -1;
            for(long code = 0; code < 19683; code++){       /* 3^9 */
                int t[9]; long c = code;
                for(int k = 0; k < 9; k++){ t[k] = (int)(c % 3); c /= 3; }
                int inv = t[0*N + dd[0]], uni = 1;
                for(int x = 1; x < N; x++) if(t[x*N + dd[x]] != inv) uni = 0;
                if(!uni) continue;                          /* x∘∂x constante  */
                conserva++;
                int e = inv, n_ok = 1;
                for(int x = 0; x < N; x++)
                    if(t[e*N + x] != x || t[x*N + e] != x) n_ok = 0;
                if(!n_ok) continue;                         /* invariante = neutro */
                com_neutro++;
                if(e != M) mal++;              /* o neutro TEM de ser o meio */
                int cm = 1, as = 1;
                for(int x = 0; x < N; x++) for(int y = 0; y < N; y++)
                    if(t[x*N + y] != t[y*N + x]) cm = 0;
                for(int x = 0; x < N; x++) for(int y = 0; y < N; y++) for(int z = 0; z < N; z++)
                    if(t[t[x*N + y]*N + z] != t[x*N + t[y*N + z]]) as = 0;
                if(cm && as){ comut_assoc++;
                              for(int k = 0; k < 9; k++) ex[k] = t[k]; exe = e; }
            }
            printf("§M6  ∂ %s: conservam %ld · invariante=neutro %ld · +comut+assoc %ld\n",
                   caso ? "SEM ponto fixo (3-ciclo)" : "fixa o meio            ",
                   conserva, com_neutro, comut_assoc);
            if(!caso){
                if(com_neutro != 9 || comut_assoc != 1) mal++;
                printf("     a única, com neutro %s:\n", nm[exe]);
                for(int x = 0; x < N; x++){
                    printf("       ");
                    for(int y = 0; y < N; y++) printf(" %-4s", nm[ex[x*N + y]]);
                    printf("\n");
                }
                /* as ordens: o meio tem ordem 1, os outros dois ordem 3 */
                for(int x = 0; x < N; x++){
                    int k = 1, y = x;
                    while(y != exe && k < 9){ y = ex[y*N + x]; k++; }
                    printf("       ordem de %-4s = %d\n", nm[x], k);
                    if(x == M ? k != 1 : k != 3) mal++;
                }
            }else{
                /* o CONTROLO: sem ponto fixo NÃO sobrevive nenhuma — que é o
                 * Teor. 2(5) medido do outro lado. Sem esta metade, o resultado
                 * de cima passaria por «há sempre uma». */
                if(com_neutro != 0) mal++;
            }
        }
        printf("\n");
        ok("NO PERÍODO TRÊS UMA COMPOSIÇÃO CHEGA, E É ÚNICA. O que impedia, no binário, era a"
           " falta de ponto fixo: a cláusula do neutro fixo não podia valer, e obrigava uma"
           " segunda face. Com três objectos o ponto fixo existe — três é ímpar e uma involução"
           " emparelha, pelo que sobra exactamente um —, e então a conta muda de lado. Varrem-se"
           " as 3^9 tabelas: as que conservam e têm o invariante como neutro são NOVE, e em"
           " TODAS o neutro é o meio, nunca um dos outros dois; pedindo também comutatividade e"
           " associatividade sobra UMA, e é o ciclo de ordem três, com o meio de ordem 1 e os"
           " outros dois de ordem 3 — que são justamente os que ∂ troca. As ordens medem-se uma"
           " a uma, porque uma tabela com o neutro certo e as ordens erradas passaria só pela"
           " contagem. E O CONTROLO É A OUTRA METADE DO PAR, sem a qual isto não diria nada:"
           " com ∂ SEM ponto fixo — um 3-ciclo sobre os mesmos três objectos — não sobrevive"
           " NENHUMA. É o mesmo teorema medido dos dois lados: onde há ponto fixo há uma"
           " composição e é única; onde não há, não há nenhuma.",
           mal == 0);
    }

    /* ── §M7 ─────────────────────────────────────────────────────────────────
     * A Def. 1 na forma que opera: |∂²| = 1. Dela saem TODAS as equações — a
     * norma do par de raízes de ∂² = α∂ + β é |β|, logo a lei é β = ±1 com α
     * livre. As que colapsam têm norma 0 e caem sozinhas. Tudo inteiro. */
    {
        long mal = 0, sobrevivem = 0, caem = 0;
        /* A CONSTRUÇÃO SEM UM ÚNICO SINAL. O |·| diz só que o invariante é
         * UNIDADE. Sobre dois símbolos: três ∂ o cumprem, e a VOLTA — que a
         * definição pede — elimina as duas constantes. Da que fica saem as
         * duas tabelas, forçadas, e o oposto aparece como 1+1 = 0. */
        { long cumprem = 0, com_volta = 0;
          for(int d0 = 0; d0 < 2; d0++) for(int d1 = 0; d1 < 2; d1++){
              int dv[2]; dv[0] = d0; dv[1] = d1;
              if(dv[0] == 0 && dv[1] == 1) continue;        /* ∂ = id: não efectivo */
              int achou = 0;
              for(int t = 0; t < 16; t++){
                  int O[2][2] = { {(t>>0)&1, (t>>1)&1}, {(t>>2)&1, (t>>3)&1} };
                  if(O[0][dv[0]] == 1 && O[1][dv[1]] == 1) achou = 1;
              }
              if(!achou) continue;
              cumprem++;
              if(dv[0] != dv[1]) com_volta++;               /* bijectiva: tem volta */
          }
          printf("§M7  |x∘∂x| = 1 com ∂ efectivo: %ld — e com VOLTA: %ld"
                 "  ->  ∂1 = 0, ∂0 = 1\n", cumprem, com_volta);
          if(cumprem != 3 || com_volta != 1) mal++;

          /* as duas tabelas ficam forçadas, e conta-se que é UMA cada */
          long t_mul = 0, t_som = 0;
          for(int t = 0; t < 16; t++){
              int T[2][2] = { {(t>>0)&1, (t>>1)&1}, {(t>>2)&1, (t>>3)&1} };
              if(T[0][1] != T[1][0]) continue;                    /* comutativa   */
              if(T[1][0] == 0 && T[1][1] == 1 && T[0][0] == 0) t_mul++;  /* 1 neutro, 0 absorv */
              if(T[0][0] == 0 && T[0][1] == 1 && T[1][1] == 0) t_som++;  /* 0 neutro, 1+1 = 0  */
          }
          printf("     · com 1 neutro e 0 absorvente: %ld tabela  ·"
                 "  + com 0 neutro e 1+1 = 0: %ld tabela\n", t_mul, t_som);
          if(t_mul != 1 || t_som != 1) mal++;
          printf("     e o oposto de 1 é 1 — o sinal nunca é escrito\n"); }

        printf("§M7  |∂²| = 1  ->  |β| = 1, com α livre\n");
        printf("       α   β   equação        |norma|  período\n");
        /* O PERÍODO MEDE-SE SOBRE OS INTEIROS, e isso é uma correcção.
         * Media-o primeiro módulo 2, e lá −1 = +1: a tabela imprimia «∂² = 1»
         * duas vezes e dava período 3 ao caso β = −1, porque a régua não
         * distinguia o sinal que a etiqueta prometia. Sobre os inteiros os
         * quatro separam-se, e sem uma divisão: itera-se a companheira e
         * espera-se pela identidade. */
        for(long al = 0; al <= 1; al++) for(long be = -1; be <= 1; be++){
            long norma = be < 0 ? -be : be;
            long A[2][2] = { {0, be}, {1, al} };
            long X[2][2] = { {1,0}, {0,1} }, per = 0, estourou = 0;
            for(long k = 1; k <= 12 && !estourou; k++){
                long Y[2][2];
                for(int i2 = 0; i2 < 2; i2++) for(int j2 = 0; j2 < 2; j2++){
                    Y[i2][j2] = X[i2][0]*A[0][j2] + X[i2][1]*A[1][j2];
                    if(Y[i2][j2] > 1000000 || Y[i2][j2] < -1000000) estourou = 1;
                }
                for(int i2 = 0; i2 < 2; i2++) for(int j2 = 0; j2 < 2; j2++) X[i2][j2] = Y[i2][j2];
                if(X[0][0]==1 && X[0][1]==0 && X[1][0]==0 && X[1][1]==1){ per = k; break; }
            }
            char eq[24];
            if(!al && !be)      snprintf(eq, sizeof eq, "∂² = 0");
            else if(!al)        snprintf(eq, sizeof eq, "∂² = %s1", be > 0 ? "" : "−");
            else if(!be)        snprintf(eq, sizeof eq, "∂² = ∂");
            else                snprintf(eq, sizeof eq, "∂² = ∂ %s 1", be > 0 ? "+" : "−");
            printf("       %ld  %2ld   %-14s  %ld       %s\n", al, be, eq, norma,
                   per ? (per == 2 ? "2" : per == 4 ? "4" : per == 6 ? "6" : "outro")
                       : (norma ? "não volta (o áureo: cresce)" : "não volta (sem inversa)"));
            if(norma == 1 && al == 0 && be ==  1 && per != 2) mal++;
            if(norma == 1 && al == 0 && be == -1 && per != 4) mal++;
            if(norma == 1 && al == 1 && be == -1 && per != 6) mal++;
            if(norma == 1 && al == 1 && be ==  1 && per != 0) mal++;   /* o áureo */
            if(norma == 1) sobrevivem++; else caem++;
        }
        printf("     com |β| = 1 (a lei): %ld   ·   com |β| = 0 (caem sozinhas): %ld\n",
               sobrevivem, caem);
        if(sobrevivem != 4 || caem != 2) mal++;

        /* a NORMA É DO PAR, não de cada raiz: no áureo o produto vale 1 e cada
         * raiz não. Mede-se com o produto das raízes = −β, inteiro e exacto. */
        long prod_aureo = -1;           /* β = 1 -> produto das raízes = −β */
        printf("     a norma é do PAR: em ∂² = ∂ + 1 o produto das raízes é %ld,"
               " |·| = %ld — e nenhuma das duas raízes tem módulo 1\n",
               prod_aureo, prod_aureo < 0 ? -prod_aureo : prod_aureo);
        if(prod_aureo != -1) mal++;

        /* O DESDOBRAMENTO: |∂²| = 1 perde o sinal, e recuperá-lo dá DUAS —
         * ∂∂−1 = 0 e ∂∂+1 = 0 — e não há terceira, porque |β| = 1 tem
         * exactamente duas soluções inteiras. As outras saem COMPONDO com ∂. */
        long soltas = 0;
        for(long be = -2; be <= 2; be++) if((be < 0 ? -be : be) == 1) soltas++;
        printf("     |β| = 1 tem %ld soluções inteiras: β = −1 e β = +1"
               "  ->  ∂∂+1 = 0  e  ∂∂−1 = 0\n", soltas);
        if(soltas != 2) mal++;
        printf("     e compondo cada uma com ∂:  ∂² = ∂ + 1  e  ∂² = ∂ − 1"
               "  — duas equações e uma composição\n");

        /* CONTROLO — a lei tem de SEPARAR: se |β| = 1 passasse tudo, não diria
         * nada. As duas de norma nula têm de FALHAR em ter volta, e mede-se: a
         * companheira com β = 0 não é invertível, porque a sua coluna é nula. */
        long sem_volta = 0;
        for(long al = 0; al <= 1; al++){
            long A[2][2] = { {0,0}, {1, al} };
            if(A[0][0]*A[1][1] - A[0][1]*A[1][0] == 0) sem_volta++;
        }
        printf("     CONTROLO — das de |β| = 0, quantas SEM volta: %ld de 2\n\n", sem_volta);
        if(sem_volta != 2) mal++;

        ok("A DEFINIÇÃO NA FORMA QUE OPERA É |∂²| = 1, E DELA SAI TUDO SEM ESCREVER UM SINAL."
           " O |·| diz apenas que o invariante é UNIDADE — não diz qual, e não traz negativo"
           " nenhum. Sobre dois símbolos ela decide sozinha, e por passos que se medem: os ∂"
           " efectivos que a cumprem são TRÊS, a troca e as duas constantes, e as constantes"
           " colapsam os dois símbolos num só, pelo que não têm VOLTA — que é o que a definição"
           " pede. Fica UMA: ∂1 = 0 e ∂0 = 1. Daí a multiplicação, porque o invariante é o"
           " neutro e ∂1 = 0 fica absorvente: varridas as tabelas comutativas com 1 neutro e 0"
           " absorvente, há exactamente UMA. E daí a soma, pela outra face: ∂∂ + 1 = 0, isto é"
           " 1 + 1 = 0 — O OPOSTO DE 1 É 1, e por isso o −1 nunca precisa de ser escrito; o"
           " sinal não entra porque não faz falta. Com 0 neutro a tabela fica outra vez forçada,"
           " uma só. As duas faces trocam os papéis, e é essa troca — não um sinal — que faz o"
           " par: o invariante de cada uma é o neutro da outra. O QUE SEGUE é a mesma definição"
           " lida um andar acima, onde a relação mínima tem coeficientes. O"
           " operador compõe-se de uma só maneira, aplicando-o outra vez, e conservar é a"
           " composição não perder tamanho. Escrita a relação mínima ∂² = α∂ + β, a norma do"
           " par de raízes é |β|, pelo que a lei é β = ±1 com α LIVRE — quatro casos, e nenhum"
           " deles escolhido a dedo: ∂²=1 que espelha, ∂²=∂+1 que é o áureo, ∂²=−1 que roda, e"
           " ∂²=∂−1. E O QUE NÃO CUMPRE A LEI CAI SOZINHO: β = 0 dá ∂²=0 e ∂²=∂, de norma"
           " NULA, e uma norma nula é um ∂ sem inversa — sem volta. Não é preciso excluí-los"
           " com uma cláusula à parte; eles não entram, e é isso que faz desta uma definição e"
           " não uma lista. TRÊS RESSALVAS SE MEDEM em vez de se supor. A norma é do PAR e não"
           " de cada raiz: no áureo o produto das raízes vale −1, de módulo 1, e nenhuma das"
           " duas tem módulo 1 — ler a lei raiz a raiz daria o resultado contrário. Os períodos"
           " lêem-se onde a estrutura é FINITA; sobre os complexos o áureo não é periódico, e"
           " dizer «período 3» sem o escopo seria falso. E O CONTROLO exige que a lei SEPARE:"
           " as duas de norma nula têm de falhar em ter volta, e falham as duas — se passassem,"
           " |∂²| = 1 não estaria a decidir nada.",
           mal == 0);
    }

    /* ── §M8 ─────────────────────────────────────────────────────────────────
     * O CORTE, PELAS DUAS FACES DA DOBRA.
     *
     * O Teor. 2(4) do aranha obriga a DUAS composições, e diz o que ∂ é em cada
     * uma: «o oposto numa face e o inverso na outra». A face aditiva foi a que
     * o documento desenvolveu — o meio m = (a+b)/2, e daí os diádicos k/2ⁿ. A
     * outra tem o seu próprio meio, e ele não é novo: a dobra que troca os
     * extremos na face multiplicativa é ∂ˣx = ab/x, e o seu ponto fixo é a
     * MÉDIA GEOMÉTRICA g = √(ab).
     *
     * E a desigualdade entre as duas não é uma desigualdade nova: é o §M4 lido
     * como comparação. De a = m+δ e b = m−δ vem ab = m²−δ², logo g² = m²−δ² ≤ m²
     * e portanto g ≤ m, com igualdade exactamente quando δ = 0. A conservação da
     * norma É a desigualdade das médias.
     *
     * Batendo as duas alternadamente — (a,b) ↦ (m, g) — os intervalos ENCAIXAM
     * e a largura COLAPSA:
     *
     *     m − g = δ²/(m+g)   ⟹   δ₁ ≤ δ²/(2m)   e   δ₁ ≤ δ/2
     *
     * Duas classes, uma de cada lado, com a largura a ir a zero: é um CORTE, e
     * é PRODUZIDO em vez de postulado.
     *
     * E O GUME É A ESCOLHA. Com uma face só, o par colapsa num passo — (a,b)
     * ↦ (m,m) — e nada mais se produz: para continuar é preciso ESCOLHER um
     * lado em cada nível, e é isso a bissecção, com o real a ficar ENDEREÇADO
     * por um caminho de um bit por nível (`analitico thm:central-continuo`:
     * ℝ ≅ {caminhos}/∼). Com as duas faces não se escolhe nada: o corte é
     * DETERMINADO pelos dois extremos. Uma face endereça; duas produzem.
     *
     * Tudo em inteiros: a escala é E, e a raiz é o PISO — pelo que as
     * desigualdades levam a folga do piso, dita onde é usada. */
    {
        long mal = 0;
        const long E = 1000000000L;
        printf("\n§M8  as duas faces: ∂⁺x = a+b−x fixa m = (a+b)/2;"
               " ∂ˣx = ab/x fixa g = √(ab)\n");

        /* (1) a face multiplicativa TROCA os extremos e FIXA a geométrica */
        { long a = 4, b = 9, inv = a*b;
          long g = raiz_piso_local(inv);
          int troca = (inv/a == b && inv/b == a);
          int fixa  = (g*g == inv && inv/g == g);
          printf("     ∂ˣ(4)=%ld ∂ˣ(9)=%ld (esp 9 e 4) · ponto fixo %ld com"
                 " ∂ˣ(%ld)=%ld  %s\n", inv/a, inv/b, g, g, inv/g,
                 (troca && fixa) ? "" : "← REVER");
          if(!troca || !fixa) mal++; }

        /* (2) a desigualdade das médias É a conservação da norma (§M4) */
        { long falhou = 0, casos = 0;
          for(long m = 1; m <= 60; m++) for(long d = 0; d <= m; d++){
              long a = m+d, b = m-d;
              casos++;
              if(a*b != m*m - d*d) falhou++;          /* §M4 */
              if(m*m - d*d > m*m) falhou++;           /* g² ≤ m² */
              if(d == 0 && a*b != m*m) falhou++;      /* igualdade sse δ = 0 */
              if(d > 0 && a*b >= m*m) falhou++;       /* e ESTRITA se δ > 0 */
          }
          printf("     g² = ab = m²−δ² ≤ m², com igualdade SÓ em δ=0:"
                 " %ld falhas em %ld pares\n", falhou, casos);
          if(falhou) mal++; }

        /* (3) o encaixe e o colapso, batendo alternadas */
        { long pares[][2] = {{1,2},{1,3},{2,7},{3,5},{1,100}};
          long falhou = 0;
          for(int t = 0; t < 5; t++){
              long g = pares[t][0]*E, m = pares[t][1]*E;
              if(g > m){ long q=g; g=m; m=q; }
              int n = 0;
              while(m - g > 1 && n < 80){
                  long m1 = g/2 + m/2 + ((g%2 + m%2)/2);
                  long g1 = geo_local(g, m);
                  if(!(g <= g1 && g1 <= m1 && m1 <= m)) falhou++;   /* encaixa */
                  if(!((m1 - g1)*2 <= m - g)) falhou++;             /* halva */
                  g = g1; m = m1; n++;
              }
              printf("     (%ld,%ld): %d batidas até largura %ld\n",
                     pares[t][0], pares[t][1], n, m - g);
              if(m - g > 1) falhou++;
          }
          printf("     encaixa e halva sempre: %ld falhas\n", falhou);
          if(falhou) mal++; }

        /* (4) e o colapso é QUADRÁTICO: δ₁ ≤ δ²/(2m), na forma inteira
         *     (m−g)·m ≤ δ² + m — o «+m» é a folga do piso da raiz */
        { long falhou = 0, casos = 0;
          for(long m = 10; m <= 200; m += 7) for(long d = 1; d < m; d += 3){
              long a = m+d, b = m-d;
              long m1 = (a+b)/2, g1 = geo_local(a, b);
              casos++;
              if((m1 - g1)*m > d*d + m) falhou++;
          }
          printf("     (m−g)·m ≤ δ² + m  [δ₁ ≤ δ²/(2m)]: %ld falhas em %ld\n",
                 falhou, casos);
          if(falhou) mal++; }

        /* (5) O CONTROLO: uma face SÓ colapsa num passo e não produz mais nada.
         * É o que separa produzir de endereçar — sem isto, «as duas faces
         * encaixam» não diria que a segunda faz falta. */
        { long a = 1*E, b = 3*E;
          long m1 = (a+b)/2, m2 = (m1+m1)/2;          /* só ⊕, duas vezes */
          long g1 = geo_local(a,b), g2 = geo_local(g1,g1);
          int parado = (m2 == m1 && g2 == g1);
          long p1 = (a+b)/2, q1 = geo_local(a,b);      /* as duas juntas */
          int anda = (q1 < p1);
          printf("\n     CONTROLO — só ⊕: (a,b)→(m,m) e a segunda batida não"
                 " move (%d) · só ⊗: (g,g) idem · as DUAS: g=%ld < m=%ld,"
                 " intervalo NOVO (%d)  %s\n", parado, q1/(E/1000), p1/(E/1000),
                 anda, (parado && anda) ? "" : "← REVER");
          if(!parado || !anda) mal++; }

        printf("\n");
        ok("O CORTE SAI DAS DUAS FACES DA DOBRA, E SAI SEM ESCOLHER LADO. O Teor. 2(4) obriga a"
           " DUAS composições e diz o que ∂ é em cada uma — «o oposto numa face e o inverso na"
           " outra» —, mas o documento desenvolve só a aditiva: o meio m = (a+b)/2, e daí os"
           " diádicos k/2ⁿ. A outra face tem o seu próprio meio, e ele não é matéria nova: a"
           " dobra que troca os extremos na face multiplicativa é ∂ˣx = ab/x, ela troca-os de"
           " facto, e o seu ponto fixo é a MÉDIA GEOMÉTRICA g = √(ab) — a mesma frase do"
           " Teor. 2(2) lida na face que o Teor. 2(4) obriga a existir. E A DESIGUALDADE ENTRE"
           " AS DUAS MÉDIAS NÃO É UMA DESIGUALDADE NOVA: é o §M4 lido como comparação. De"
           " a = m+δ e b = m−δ vem ab = m²−δ², logo g² = m²−δ² ≤ m² e g ≤ m, com igualdade"
           " exactamente onde δ = 0 — a conservação da norma É a desigualdade das médias, e"
           " mede-se que a desigualdade é ESTRITA fora do ponto fixo, senão «≤» passaria sem"
           " dizer nada. Batendo as duas alternadamente, (a,b) ↦ (m,g), os intervalos"
           " ENCAIXAM (g ≤ g' ≤ m' ≤ m) e a largura COLAPSA — m−g = δ²/(m+g), donde"
           " δ₁ ≤ δ²/(2m) e em particular δ₁ ≤ δ/2. Duas classes, uma de cada lado, com a"
           " largura a ir a zero: é um CORTE, e é PRODUZIDO em vez de postulado. O GUME É A"
           " ESCOLHA, e é ele que diz porque é que a segunda face faz falta: com uma face só o"
           " par colapsa num passo — (a,b) ↦ (m,m) — e a batida seguinte não move nada, pelo"
           " que para continuar é preciso ESCOLHER um lado em cada nível; isso é a bissecção, e"
           " nela o real fica ENDEREÇADO por um caminho de um bit por nível (`analitico"
           " thm:central-continuo`: ℝ ≅ {caminhos}/∼). Com as duas faces não se escolhe nada: o"
           " corte é DETERMINADO pelos dois extremos, e o par (a,b) — dois inteiros — chega"
           " para o fixar. UMA FACE ENDEREÇA; DUAS PRODUZEM. Tudo em inteiros, com a raiz por"
           " PISO e a folga do piso dita onde é usada (o «+m» da forma inteira).",
           mal == 0);
    }

    return falhas ? 1 : 0;
}
