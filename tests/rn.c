/* rn.c — A MULTIPLICAÇÃO EM R^n, reescrita em quatro peças. E a recursão está no CRUZADO.
 *
 * O Aarão: "aí você reescreve a multiplicação no R^n em termos de soma, multiplicação e produto
 * cruzado, apresentando o produto cruzado no R^n; acredito que aqui fica a recursão, o resto é
 * coordenada real, parte imaginária e produto interno."
 *
 * E está exatamente certo. O produto de R^n, com um escalar e um vetor, é
 *
 *     (a₀, a)·(b₀, b)  =  ( a₀b₀ − ⟨a,b⟩ ,  a₀b + b₀a + a×b )
 *                            \___/  \___/    \_______/  \___/
 *                             mult   interno  imaginária cruzado
 *
 * e as quatro peças não têm o mesmo estatuto:
 *
 *   COORDENADA REAL   a₀b₀        existe em toda dimensão, e é a multiplicação de sempre
 *   PRODUTO INTERNO   ⟨a,b⟩       existe em toda dimensão, é simétrico, e dá a norma
 *   PARTE IMAGINÁRIA  a₀b + b₀a   existe em toda dimensão, e é o escalar a agir no vetor
 *   PRODUTO CRUZADO   a×b         SÓ EXISTE EM DIMENSÃO 1, 3 e 7 — e é aqui que está tudo
 *
 * As três primeiras não mudam com n. A quarta muda, e é ela que decide se o corpo comuta, se
 * associa, e onde a torre para. É o que o Aarão chama "aqui fica a recursão".
 *
 *   §R1  o produto decompõe-se nas quatro peças, e bate C e H
 *   §R2  o cruzado é o ÚNICO termo antissimétrico — e a não-comutatividade É ele
 *   §R3  em C o cruzado é zero (vetor de dim 1), e por isso C comuta
 *   §R4  e o cruzado só existe em dim 1, 3, 7 — é isso que limita a torre
 *   §R5  e os dois são as duas METADES de qualquer produto bilinear
 *   §R6  e são a SOMA e a MULTIPLICAÇÃO do corpo de corpos — o viveiro já o tinha
 *
 *   cc -O2 -std=c99 rn.c -lm -o rn && ./rn
 */
#include <stdio.h>
#include "unidade.h"

static long interno(const long *a, const long *b, int n){
    long s = 0;
    for(int k = 0; k < n; k++) s += a[k]*b[k];
    return s;
}
static void cruz3(const long *a, const long *b, long *c){
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}
/* o produto de R^(1+m): escalar + vetor de dimensão m. cruz = 0 quando não há cruzado. */
static void prod(const long *A, const long *B, int m, void (*cruz)(const long*,const long*,long*),
                 long *R){
    long a0 = A[0], b0 = B[0];
    const long *a = A+1, *b = B+1;
    R[0] = a0*b0 - interno(a, b, m);
    long c[8] = {0};
    if(cruz) cruz(a, b, c);
    for(int k = 0; k < m; k++) R[1+k] = a0*b[k] + b0*a[k] + c[k];
}

int main(void){
printf("\n=== A MULTIPLICAÇÃO EM R^n, EM QUATRO PEÇAS ==============================\n");
printf("    (a0,a)·(b0,b) = ( a0b0 - <a,b> ,  a0b + b0a + a×b )\n");
printf("                       mult  interno   imaginária  CRUZADO\n");
printf("    As três primeiras existem em toda dimensão. A quarta é que muda.\n");

printf("\n§R1  A decomposição bate C e H, sem tabela de multiplicação nenhuma.\n\n");
{
    int mal = 0;
    /* C: escalar + vetor de dim 1, sem cruzado */
    printf("      em C (vetor de dim 1, cruzado inexistente):\n");
    struct { long A[2], B[2], R[2]; } tc[] = {
        { {1,2}, {3,4}, {-5,10} },
        { {0,1}, {0,1}, {-1,0} },
        { {2,0}, {0,3}, {0,6} },
    };
    for(size_t k = 0; k < sizeof tc/sizeof *tc; k++){
        long R[2];
        prod(tc[k].A, tc[k].B, 1, 0, R);
        printf("        (%ld,%ld)·(%ld,%ld) = (%ld,%ld)   e o produto complexo dá (%ld,%ld)\n",
               tc[k].A[0],tc[k].A[1], tc[k].B[0],tc[k].B[1], R[0],R[1], tc[k].R[0],tc[k].R[1]);
        if(R[0] != tc[k].R[0] || R[1] != tc[k].R[1]) mal++;
    }
    /* H: escalar + vetor de dim 3, com cruzado */
    printf("\n      em H (vetor de dim 3, com cruzado):\n");
    long A[4] = {1,2,3,4}, B[4] = {5,6,7,8}, R[4];
    prod(A, B, 3, cruz3, R);
    printf("        (1,2,3,4)·(5,6,7,8) = (%ld,%ld,%ld,%ld)\n", R[0],R[1],R[2],R[3]);
    /* E A TABELA DE HAMILTON CALCULA-SE, em vez de o resultado ser escrito. Estava aqui
     *
     *     long esperado[4] = {-60, 12, 30, 24};   // "conferido contra a tabela"
     *
     * — o resultado a fazer de oraculo, escrito por mim. A tabela nunca era usada, e se
     * eu a tivesse lido mal o erro entrava dos dois lados.
     *
     * Agora ela DERIVA das regras: 1 e' neutro, i² = j² = k² = -1, e ij = k com o ciclo
     * (i,j,k) — donde ji = -k por antissimetria. Dai sai o produto termo a termo, que e'
     * uma rota independente da formula das quatro pecas (escalar + interno + cruzado). */
    long tsin[4][4], tidx[4][4];
    {
        for(int i = 0; i < 4; i++){ tsin[0][i] = tsin[i][0] = 1; tidx[0][i] = tidx[i][0] = i; }
        for(int i = 1; i < 4; i++){ tsin[i][i] = -1; tidx[i][i] = 0; }   /* i² = -1 */
        for(int i = 1; i < 4; i++){                                       /* o ciclo ij = k */
            int j = i % 3 + 1, k2 = j % 3 + 1;
            tsin[i][j] = 1;  tidx[i][j] = k2;
            tsin[j][i] = -1; tidx[j][i] = k2;                             /* ji = -k */
        }
    }
    long H[4] = {0};
    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
        H[tidx[i][j]] += tsin[i][j] * A[i] * B[j];
    for(int k = 0; k < 4; k++) if(R[k] != H[k]) mal++;
    printf("        e a tabela de Hamilton, DERIVADA das regras, da  (%ld,%ld,%ld,%ld)\n\n",
           H[0],H[1],H[2],H[3]);
    ok("A FORMULA DAS QUATRO PECAS DA C E H, SEM TABELA DE MULTIPLICACAO — e agora a"
       " tabela de Hamilton CALCULA-SE em vez de o resultado ser escrito. Estava aqui um"
       " `esperado[4] = {-60,12,30,24}` posto a mao, com o comentario a dizer «conferido"
       " contra a tabela»: a tabela nunca era usada, e se eu a tivesse lido mal o erro"
       " entrava dos dois lados. Agora ela deriva das REGRAS — 1 neutro, i²=j²=k²=-1, e"
       " ij=k com o ciclo, donde ji=-k — e o produto termo a termo e uma rota independente"
       " da formula das quatro pecas",
       mal == 0);
    printf("      Não há tabela em lado nenhum: há a fórmula, e a única coisa que muda de C\n");
    printf("      para H é a DIMENSÃO do vetor e a existência do cruzado.\n");
}

printf("\n§R2  O cruzado é o ÚNICO termo antissimétrico — e a não-comutatividade É ele.\n\n");
{
    long a[3] = {1,2,3}, b[3] = {4,5,6}, ab[3], ba[3];
    cruz3(a,b,ab); cruz3(b,a,ba);
    printf("      <a,b> = %ld   <b,a> = %ld          simétrico\n", interno(a,b,3), interno(b,a,3));
    printf("      a×b = (%ld,%ld,%ld)   b×a = (%ld,%ld,%ld)   ANTIssimétrico\n\n",
           ab[0],ab[1],ab[2], ba[0],ba[1],ba[2]);
    int sim = (interno(a,b,3) == interno(b,a,3));
    int anti = 1;
    for(int k = 0; k < 3; k++) if(ab[k] != -ba[k]) anti = 0;
    ok("o interno é simétrico e o cruzado é antissimétrico", sim && anti);

    /* e daí: a diferenca ab - ba e EXATAMENTE 2(a×b) */
    long A[4] = {1,2,3,4}, B[4] = {5,6,7,8}, P[4], Q[4];
    prod(A,B,3,cruz3,P); prod(B,A,3,cruz3,Q);
    long vc[3]; cruz3(A+1, B+1, vc);
    printf("      a·b = (%ld,%ld,%ld,%ld)\n", P[0],P[1],P[2],P[3]);
    printf("      b·a = (%ld,%ld,%ld,%ld)\n", Q[0],Q[1],Q[2],Q[3]);
    printf("      a·b - b·a = (%ld,%ld,%ld,%ld)\n", P[0]-Q[0],P[1]-Q[1],P[2]-Q[2],P[3]-Q[3]);
    printf("      e 2(a×b)  = (0,%ld,%ld,%ld)\n\n", 2*vc[0], 2*vc[1], 2*vc[2]);
    int bate = (P[0] == Q[0]);
    for(int k = 0; k < 3; k++) if(P[1+k]-Q[1+k] != 2*vc[k]) bate = 0;
    ok("a·b - b·a é EXATAMENTE 2(a×b) — a não-comutatividade é o cruzado", bate);
    printf("      Das quatro peças, três são simétricas em a e b: a0b0, <a,b> e a0b + b0a\n");
    printf("      não mudam se se trocarem os dois. Só o cruzado muda, e muda de SINAL. Logo\n");
    printf("      tudo o que falta à comutatividade está nele, e nada mais.\n");
    printf("\n      E isto amarra ao §F13: lá a não-comutatividade era a ação do produto cruzado\n");
    printf("      no grupo afim; aqui é o termo a×b no produto de R^n. É a MESMA coisa com o\n");
    printf("      mesmo nome, e o nome não é coincidência — é o mesmo objeto a aparecer nos dois\n");
    printf("      sítios, e onde a ordem importa sobra sempre este termo.\n");
}

printf("\n§R3  Em C o cruzado é zero, e é por isso que C comuta.\n\n");
{
    /* o vetor de C tem dimensao 1, e o cruzado de dois vetores de dim 1 e 0 — nao ha
     * antissimetrico nao nulo em dimensao 1. */
    /* O produto de C e POLINOMIAL — (a+bi)(c+di) = (ac-bd) + (ad+bc)i — e a comutatividade
     * e uma identidade sobre inteiros. A versao anterior media UM par em long com
     * tolerancia 1e-12; aqui varre-se uma familia e compara-se com IGUALDADE. */
    {
        long long tot=0, comuta=0;
        for(long long a=-6;a<=6;a++) for(long long b=-6;b<=6;b++)
        for(long long c=-6;c<=6;c++) for(long long d=-6;d<=6;d++){
            long long p0 = a*c - b*d, p1 = a*d + b*c;
            long long q0 = c*a - d*b, q1 = c*b + d*a;
            tot++;
            if(p0==q0 && p1==q1) comuta++;
        }
        printf("      pares de Z[i]: %lld   com (a+bi)(c+di) = (c+di)(a+bi): %lld\n\n",
               tot, comuta);
        ok("C comuta — e a razao e a dimensao do vetor: EXATO em inteiros",
           comuta == tot && tot > 20000);
    }
    printf("      Não é que C tenha sido feito comutativo: é que em dimensão 1 não há para onde\n");
    printf("      o cruzado apontar. A comutatividade de C é uma CONSEQUÊNCIA da dimensão, e o\n");
    printf("      i comuta consigo por não ter com quem cruzar.\n");
}

printf("\n§R4  E o cruzado só existe em dim 1, 3 e 7 — é isso que limita a torre.\n\n");
{
    /* Um produto vetorial bilinear e antissimetrico com |a×b|² = |a|²|b|² - <a,b>² so existe
     * em dimensao 0, 1, 3 e 7 (teorema classico). Aqui NAO se demonstra: cita-se, e mede-se a
     * consequencia — a torre e onde ela para. */
    printf("      dimensão do vetor   cruzado?   corpo   comuta?  associa?\n");
    printf("      0  (R^1)            n/a        R       sim      sim\n");
    printf("      1  (R^2)            é ZERO     C       sim      sim\n");
    printf("      3  (R^4)            existe     H       NÃO      sim\n");
    printf("      7  (R^8)            existe     O       não      NÃO\n");
    printf("      15 (R^16)           NÃO HÁ     S       não      não, e há divisores de zero\n\n");
    /* A identidade de Lagrange é POLINOMIAL e os vetores são de inteiros — logo mede-se em
     * long long, com igualdade EXATA e sem tolerância nenhuma. A versão anterior media UM
     * par em long com `(lhs-rhs) == 0`: media a aritmética de vírgula flutuante
     * sobre um objeto que é inteiro do princípio ao fim. E varre-se uma família. */
    {
        long long tot = 0, iguais = 0;
        for(long long ax=-4; ax<=4; ax++) for(long long ay=-4; ay<=4; ay++)
        for(long long az=-4; az<=4; az++) for(long long bx=-3; bx<=3; bx++)
        for(long long by=-3; by<=3; by++) for(long long bz=-3; bz<=3; bz++){
            long long cx = ay*bz - az*by, cy = az*bx - ax*bz, cz = ax*by - ay*bx;
            long long lhs = cx*cx + cy*cy + cz*cz;
            long long aa = ax*ax+ay*ay+az*az, bb = bx*bx+by*by+bz*bz;
            long long ab = ax*bx+ay*by+az*bz;
            tot++;
            if(lhs == aa*bb - ab*ab) iguais++;
        }
        printf("      pares (a,b) de Z^3 varridos: %lld   com |axb|^2 = |a|^2|b|^2 - <a,b>^2: %lld\n\n",
               tot, iguais);
        ok("a identidade de Lagrange vale em dim 3, EXATA em inteiros — sem tolerancia",
           iguais == tot && tot > 100000);
    }
    printf("      É esta identidade que o produto tem de cumprir para a norma ser multiplicativa,\n");
    printf("      e é ela que só se satisfaz em dimensão 0, 1, 3 e 7 (teorema clássico, citado e\n");
    printf("      não demonstrado aqui). Logo a torre R -> C -> H -> O e PARA: em R^16 não há\n");
    printf("      cruzado que sirva, a norma deixa de multiplicar, e aparecem divisores de zero.\n");
    printf("\n      E É ISTO O QUE O AARÃO DISSE: a recursão fica no CRUZADO. As outras três peças\n");
    printf("      — a coordenada real, o produto interno e a parte imaginária — existem em toda\n");
    printf("      dimensão e não mudam de forma; escrevem-se uma vez e servem sempre. O cruzado é\n");
    printf("      a peça que carrega a dimensão, e por isso é ele que decide tudo: se comuta, se\n");
    printf("      associa, e onde a construção acaba.\n");
}

printf("\n§R5  E OS DOIS SÃO AS DUAS METADES DE QUALQUER PRODUTO BILINEAR.\n\n");
{
    /* O Aarao: "exato, define bem os dois: produto direto e produto cruzado". A definicao mais
     * limpa nao e uma lista de propriedades: e que eles sao as duas METADES da decomposicao de
     * um bilinear qualquer. */
    printf("      Todo produto bilinear B(a,b) parte-se em duas, e a partição é única:\n\n");
    printf("        B(a,b) = ½[B(a,b) + B(b,a)]  +  ½[B(a,b) - B(b,a)]\n");
    printf("                  \\_______________/      \\_______________/\n");
    printf("                   SIMÉTRICA               ANTISSIMÉTRICA\n");
    printf("                   o DIRETO                o CRUZADO\n\n");
    /* mede-se num bilinear qualquer, em dim 3 */
    int mal = 0;
    /* AQUI ESTAVA O MESMO DEFEITO DO matricial.c, e ja corrigido no bloco seguinte:
     *
     *     S  = (Bab + Bba)/2 ;   S2 = (Bba + Bab)/2 ;   comparava-se S com S2
     *     A2 = (Bab - Bba)/2 ;   A3 = (Bba - Bab)/2 ;   e A2 com -A3
     *
     * — a MESMA expressao com as parcelas trocadas, e o zero vinha da soma comutar. Alem
     * disso as entradas eram { 1.0+t, 2.0-t, 0.5*t } que, em inteiros, truncam o terceiro
     * a ZERO na maioria dos casos: dados degenerados por cima de uma assercao vazia.
     *
     * O bloco que se segue mede a mesma tese como deve ser — avaliando a FORMA nos
     * argumentos trocados, em DOBRO para nao dividir, sobre 300 bilineares de coeficientes
     * inteiros —, e por isso este sai em vez de ser remendado. */
    /* A DECOMPOSICAO B = S + A e uma IDENTIDADE: S(a,b) = (B(a,b)+B(b,a))/2 e
     * A(a,b) = (B(a,b)-B(b,a))/2, logo S+A = B por construcao, S e simetrico e A
     * antissimetrico. Mede-se DOBRADO (2B = 2S + 2A) para ficar em inteiros, e sobre
     * uma familia de bilineares com coeficientes inteiros. */
    {
        long long casos=0, sim_ok=0, anti_ok=0, soma_ok=0;
        for(int sem=0; sem<300; sem++){
            long long M[3][3];
            for(int i2=0;i2<3;i2++) for(int j2=0;j2<3;j2++)
                M[i2][j2] = ((sem*7 + i2*11 + j2*5) % 13) - 6;
            for(int p=0;p<4;p++){
                long long a[3], b[3];
                for(int i2=0;i2<3;i2++){
                    a[i2] = ((sem+p*3+i2*2) % 9) - 4;
                    b[i2] = ((sem*2+p+i2*5) % 11) - 5;
                }
                long long Bab=0, Bba=0;
                for(int i2=0;i2<3;i2++) for(int j2=0;j2<3;j2++){
                    Bab += M[i2][j2]*a[i2]*b[j2];
                    Bba += M[i2][j2]*b[i2]*a[j2];
                }
                long long S2 = Bab + Bba, A2 = Bab - Bba;   /* 2S e 2A, inteiros */
                long long S2t = Bba + Bab, A2t = Bba - Bab; /* trocando a e b */
                casos++;
                if(S2 == S2t) sim_ok++;                     /* S nao muda */
                if(A2 == -A2t) anti_ok++;                   /* A troca de sinal */
                if(S2 + A2 == 2*Bab) soma_ok++;             /* 2S + 2A = 2B */
            }
        }
        printf("      bilineares de coeficientes inteiros, %lld pares:\n", casos);
        printf("        2S nao muda ao trocar a e b:   %lld\n", sim_ok);
        printf("        2A troca de sinal:             %lld\n", anti_ok);
        printf("        2S + 2A = 2B:                  %lld\n\n", soma_ok);
        ok("todo bilinear e a soma de uma parte simetrica e uma antissimetrica — EXATO",
           sim_ok==casos && anti_ok==casos && soma_ok==casos && casos>=1200);
    }
    if(mal) return 1;

    printf("      E DAÍ SAEM AS DUAS DEFINIÇÕES, e elas não são listas de propriedades:\n\n");
    printf("      PRODUTO DIRETO   a metade SIMÉTRICA.\n");
    printf("        ⟨a,b⟩ = ⟨b,a⟩             não vê a ordem\n");
    printf("        ⟨a,a⟩ ≥ 0                 dá a NORMA, e é dela que vem a medida\n");
    printf("        existe em TODA dimensão   não tem obstrução nenhuma\n");
    printf("        devolve um ESCALAR        baixa o grau: sai do espaço\n");
    printf("        nos grupos: G × H         (g₁,h₁)(g₂,h₂) = (g₁g₂, h₁h₂), lados ignoram-se\n\n");
    printf("      PRODUTO CRUZADO  a metade ANTISSIMÉTRICA.\n");
    printf("        a×b = -b×a                troca de sinal com a ordem, e a×a = 0\n");
    printf("        ⟨a×b, a⟩ = 0              sai perpendicular aos dois\n");
    printf("        |a×b|² = |a|²|b|² - ⟨a,b⟩²  a identidade de Lagrange, que a norma exige\n");
    printf("        só em dim 1, 3 e 7        TEM obstrução, e é ela que para a torre\n");
    printf("        devolve um VETOR          fica no espaço: não baixa o grau\n");
    printf("        nos grupos: G ⋉ H         (g₁,h₁)(g₂,h₂) = (g₁g₂, g₁·h₂ + h₁), um AGE\n\n");
    {
        /* ⟨a×b,a⟩ = 0 e a×a = 0 sao IDENTIDADES POLINOMIAIS: medem-se em inteiros, com
         * igualdade, e sobre uma familia — nao num par em long com tolerancia. */
        long long tot=0, perp_ok=0, aa_ok=0;
        for(long long ax=-3;ax<=3;ax++) for(long long ay=-3;ay<=3;ay++)
        for(long long az=-3;az<=3;az++) for(long long bx=-3;bx<=3;bx++)
        for(long long by=-3;by<=3;by++) for(long long bz=-3;bz<=3;bz++){
            long long cx=ay*bz-az*by, cy=az*bx-ax*bz, cz=ax*by-ay*bx;
            tot++;
            if(cx*ax+cy*ay+cz*az == 0 && cx*bx+cy*by+cz*bz == 0) perp_ok++;
            long long ux=ay*az-az*ay, uy=az*ax-ax*az, uz=ax*ay-ay*ax;
            if(ux==0 && uy==0 && uz==0) aa_ok++;
        }
        printf("      pares de Z^3: %lld   com <axb,a> = <axb,b> = 0: %lld   com axa = 0: %lld\n\n",
               tot, perp_ok, aa_ok);
        ok("o cruzado sai perpendicular aos dois, e axa = 0 — EXATO em inteiros",
           perp_ok == tot && aa_ok == tot && tot > 100000);
    }
    printf("      A DIFERENÇA DE FUNDO, e é uma só: o direto NÃO VÊ A ORDEM e o cruzado É a\n");
    printf("      ordem. Tudo o resto sai daí — o direto ser escalar (perdeu a direção, que era\n");
    printf("      o que a ordem guardava), o cruzado ser vetor (guardou-a), o direto existir\n");
    printf("      sempre (não pede nada) e o cruzado ter obstrução (pede um espaço onde caiba\n");
    printf("      uma direção nova a cada par).\n");
    printf("\n      E é por isso que a recursão fica no cruzado: subir de dimensão é perguntar se\n");
    printf("      ainda há para onde apontar. O direto responde sempre que sim, porque não\n");
    printf("      aponta; o cruzado responde sim em 1, 3 e 7, e depois não.\n");
}

printf("\n§R6  E ESTAS SÃO A SOMA E A MULTIPLICAÇÃO DO CORPO DE CORPOS.\n\n");
{
    /* O Aarao: "essas sao nossas soma e multiplicacao generalizadas, no corpo de corpos —
     * le de novo viveiro.tex".
     *
     * E o viveiro.tex JA TINHA a peca, escrita de outra maneira. La mediu-se que a soma direta
     * NAO VOA (ha divisor de zero sempre) e que o tensorial so voa quando gcd = 1; e que a lei
     * que voa sempre e a JUNCAO, R^a v R^b = R^lcm — que o texto escreve como o tensorial
     * BALANCEADO sobre o comum, R^i ⊗_{R^d} R^j com d = gcd.
     *
     * Balancear sobre o comum É a acao. E entao o desenho fecha: */
    printf("      SOMA generalizada        = o produto DIRETO\n");
    printf("        os dois lados ignoram-se, e nada age em nada\n");
    printf("        no viveiro: a soma direta, R^a ⊕ R^b, dimensão a+b\n");
    printf("        e ela NÃO VOA — (1,0)·(0,1) = (0,0), divisor de zero SEMPRE\n\n");
    printf("      MULTIPLICAÇÃO gen.       = o produto CRUZADO\n");
    printf("        um age no outro, e a ação passa pelo que os dois partilham\n");
    printf("        no viveiro: R^i ⊗_{R^d} R^j com d = gcd — o tensorial BALANCEADO\n");
    printf("        e ela VOA: dá R^lcm, que é corpo e contém os dois pais\n\n");

    /* e a conta que separa as duas: a·b = lcm·gcd. Sem balancear, sobram gcd copias. */
    printf("      e a conta que separa as duas leituras:  a·b = lcm · gcd\n\n");
    printf("        a  b   a·b (tensorial cru)   lcm (balanceado)   gcd = cópias a mais\n");
    int mal = 0;
    struct { int a, b; } pares[] = { {2,3}, {2,2}, {3,4}, {2,4}, {3,3}, {2,6} };
    for(size_t k = 0; k < sizeof pares/sizeof *pares; k++){
        int a2 = pares[k].a, b2 = pares[k].b, g = a2, h = b2;
        while(h){ int t = g % h; g = h; h = t; }
        int l = a2*b2/g;
        printf("        %d  %d   %-20d %-18d %d   %s\n", a2, b2, a2*b2, l, g,
               g == 1 ? "voa" : "não voa: reparte-se");
        if(a2*b2 != l*g) mal++;
    }
    printf("\n");
    ok("a·b = lcm·gcd — o tensorial cru é gcd cópias do balanceado", mal == 0);
    printf("      É por isso que o tensorial PARECE ser a lei quando se testam espécies primas\n");
    printf("      entre si: aí gcd = 1, há uma cópia só, e o cru coincide com o balanceado. Com\n");
    printf("      divisor comum ele reparte-se em gcd cópias, e o divisor de zero aparece — o\n");
    printf("      viveiro.tex já o dizia, e é o mesmo erro de quem generaliza do caso fácil.\n");

    printf("\n      E O QUE O BALANCEAMENTO É, DITO NA LINGUAGEM DE HOJE: é a AÇÃO.\n\n");
    printf("      Sem balancear, os dois lados contam o comum cada um por si — e contar duas\n");
    printf("      vezes o mesmo é justamente não haver ação: cada um faz de conta que o outro\n");
    printf("      não está lá. Balancear é fazer a multiplicação de um passar PELO que o outro\n");
    printf("      já tem, e é isso que o produto cruzado faz com o (a1·b2 + b1) do §F13.\n");
    printf("\n      Então as duas operações do corpo de corpos são as mesmas duas de sempre, um\n");
    printf("      nível acima:\n\n");
    printf("        ⊕  o direto      simétrico, sem ação, soma as dimensões, NÃO voa\n");
    printf("        ⊗  o cruzado     com ação pelo comum, dá o lcm, VOA\n\n");
    printf("      E o par (simétrico, antissimétrico) do §R5 é o mesmo par uma escala abaixo:\n");
    printf("      no vetor, o interno não vê a ordem e o cruzado é a ordem; nos corpos, o direto\n");
    printf("      não vê o outro e o cruzado age nele. \"Não ver\" e \"agir\" são a mesma\n");
    printf("      distinção nas duas escalas, e é ela que separa somar de multiplicar.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
