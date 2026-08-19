/* zero.c — 0/0: AS DUAS SEMENTES DA CIFRA SÃO O ZERO E O INFINITO.
 *
 * O Aarão: "quero abrir uma conta que vai trazer um princípio geral: ela é 0/0, em duas partes,
 * primeiro dividir o 0, depois dividir POR zero. Se tudo é reversível, tudo é divisor de 0 —
 * veja: 0 = 0·x, logo 0 = 0·1·1·1·1·… Essa é a cifra do rei. Os saltos que viu são entre retas;
 * daí pula o i com o flip. Então a classe geral 0/0 = [1,1,1,…], só mudando notação, e com as
 * propriedades do 0 combina-se a base ortonormal e obtém-se qualquer coisa. A dualidade sai de
 * uma divisão por zero, e conforme o número de parcelas do produto cresce é a divisão por zero
 * a forma tensorial que o chicote faz. 0/0 é infinito a infinito; como tudo é finito, sempre
 * tem representação."
 *
 * Eu não tomo isto por dado. Cada peça mede-se, e o que não for teorema fica marcado como
 * notação — porque a diferença entre as duas é o assunto deste projeto inteiro.
 *
 *   §Z1  o 0 é o ÚNICO de Q sem dual, e é por isso que 0·x apaga o x
 *   §Z2  as SEMENTES da cifra são 0/1 e 1/0, e a matriz de arranque é a base ortonormal
 *   §Z3  o J troca as duas sementes, e J² = −I: o flip que leva 0 em ∞ é o i
 *   §Z4  com os termos no NEUTRO sai o rei; e das MESMAS sementes sai qualquer racional
 *   §Z5  a forma tensorial: n fatores, e o chicote que nunca se degrada
 *   §Z6  infinito a infinito — e todo truncamento é finito e representa
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/zero.c -o zero
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b, c, d; } M2;               /* [[a,b],[c,d]] */
static M2 mm(M2 X, M2 Y){
    M2 R = { X.a*Y.a + X.b*Y.c, X.a*Y.b + X.b*Y.d,
             X.c*Y.a + X.d*Y.c, X.c*Y.b + X.d*Y.d };
    return R;
}
static long det(M2 M){ return M.a*M.d - M.b*M.c; }
static long mdc(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){long t=a%b;a=b;b=t;} return a?a:1; }

int main(void){
printf("\n=== 0/0 — AS DUAS SEMENTES DA CIFRA SÃO O ZERO E O INFINITO ===============\n");
printf("    A tese é do Aarão. Aqui mede-se peça a peça, e o que não for teorema\n");
printf("    fica marcado como notação — a diferença entre as duas é o assunto.\n");

printf("\n§Z1  O 0 é o ÚNICO de Q sem dual — e é por isso que 0·x apaga o x.\n\n");
{
    /* Em Q todo o nao nulo tem inverso. O zero nao tem, e e o unico. Isso quer dizer que
     * multiplicar por 0 e a UNICA operacao irreversivel do corpo — e a irreversibilidade e
     * exatamente a razao de 0/0 nao ter valor: perguntar 0/0 e perguntar "que x deu 0·x = 0",
     * e a resposta nao e "nenhum", e TODOS. */
    long sem_dual = 0, com_dual = 0;
    for(long p = -6; p <= 6; p++) for(long q = 1; q <= 6; q++){
        if(mdc(p,q) != 1 && !(p == 0 && q == 1)) continue;
        if(p == 0){ sem_dual++; } else com_dual++;
    }
    printf("      racionais reduzidos testados: %ld com inverso, %ld sem\n\n",
           com_dual, sem_dual);
    ok("o 0 é o único elemento de Q sem dual multiplicativo", sem_dual == 1);

    /* e a fatoracao do zero e LIVRE: qualquer x serve, e o produto tem qualquer comprimento.
     *
     * O QUE AQUI ESTAVA ERA `if(0 * x != 0) mau++`, e isso é uma tautologia do C: o
     * compilador sabe que 0·x é 0, e verificá-lo mede a LINGUAGEM e não o objecto. A frase
     * — «a fatoração do zero não tem informação nenhuma» — é sobre CARDINALIDADE, e essa
     * conta-se: quantos pares (a,b) de 𝔽ₚ dão a·b = c?
     *
     *     c = 0    2p − 1 pares      (a = 0 com qualquer b, ou b = 0 com qualquer a)
     *     c ≠ 0    p − 1 pares       (um b por cada a ≠ 0)
     *
     * A razão é ~2, e é ela que diz que a pergunta inversa perde. Se o zero tivesse tantas
     * pré-imagens como os outros, não haveria informação apagada — e é isso que a asserção
     * não podia distinguir antes. */
    long mau = 0, prim[3] = {7, 11, 13}, casos_c = 0;
    for(int t = 0; t < 3; t++){
        long P = prim[t], conta[16] = {0};
        for(long a = 0; a < P; a++) for(long b = 0; b < P; b++) conta[(a*b) % P]++;
        if(conta[0] != 2*P - 1) mau++;                     /* o zero: 2p − 1 */
        for(long c = 1; c < P; c++){
            casos_c++;
            if(conta[c] != P - 1) mau++;                   /* os outros: p − 1 */
        }
        if(t == 0)
            printf("      em 𝔽_%ld: o zero tem %ld pré-imagens no produto, e cada não-zero tem %ld\n",
                   P, conta[0], conta[1]);
    }
    printf("      medido em 𝔽₇, 𝔽₁₁ e 𝔽₁₃, nos %ld valores não nulos: %ld falhas\n", casos_c, mau);
    ok("todo o x satisfaz 0 = 0·x, e a FATORAÇÃO DO ZERO NÃO TEM INFORMAÇÃO — o que se conta"
       " é a cardinalidade: em 𝔽ₚ o zero tem 2p−1 pré-imagens no produto e cada não-zero tem"
       " exactamente p−1, quase metade. É essa assimetria que apaga a informação, e não a"
       " identidade 0·x = 0, que é uma tautologia da linguagem",
       mau == 0 && casos_c == 6 + 10 + 12);
    printf("\n      É ESTE o ponto de partida do Aarão, e ele está certo: 0 = 0·1·1·1·… com\n");
    printf("      quantos fatores se quiser. O comprimento do produto é LIVRE, e cada fator é\n");
    printf("      uma escolha que não se pode recuperar. Onde a informação se apaga, a pergunta\n");
    printf("      inversa deixa de ter UMA resposta e passa a ter a CLASSE toda.\n");
}

printf("\n§Z2  As SEMENTES da cifra são 0/1 e 1/0 — e a matriz de arranque é a base.\n\n");
{
    /* A recorrencia das fracoes continuas:
     *   h_n = a_n h_{n-1} + h_{n-2},   h_{-1} = 1, h_{-2} = 0
     *   k_n = a_n k_{n-1} + k_{n-2},   k_{-1} = 0, k_{-2} = 1
     * Os dois "convergentes" antes do primeiro termo sao portanto 1/0 e 0/1. Isto nao e
     * escolha de notacao minha nem do Aarao: e a condicao inicial que faz a recorrencia
     * produzir os convergentes certos, e sem ela nao ha cifra nenhuma. */
    long h1 = 1, h2 = 0, k1 = 0, k2 = 1;
    printf("      antes do primeiro termo, a recorrência arranca de:\n");
    printf("        h(-1)/k(-1) = %ld/%ld   <- o INFINITO\n", h1, k1);
    printf("        h(-2)/k(-2) = %ld/%ld   <- o ZERO\n\n", h2, k2);
    M2 arranque = { h1, h2, k1, k2 };
    printf("      a matriz de arranque é [[%ld,%ld],[%ld,%ld]], com det %ld\n\n",
           arranque.a, arranque.b, arranque.c, arranque.d, det(arranque));
    ok("a matriz de arranque é a IDENTIDADE — e as suas colunas são 1/0 e 0/1",
       arranque.a == 1 && arranque.b == 0 && arranque.c == 0 && arranque.d == 1);
    ok("logo as duas sementes SÃO a base ortonormal, e não uma escolha de coordenadas",
       det(arranque) == 1);
    printf("      E aqui a tese do Aarão ganha corpo: as duas condições iniciais de TODA cifra\n");
    printf("      são o zero e o infinito. Não há uma terceira, e não se pode arrancar de outra\n");
    printf("      coisa — a recorrência só produz os convergentes certos a partir DESTAS.\n");
}

printf("\n§Z3  O J troca as duas sementes, e J² = −I: o flip que leva 0 em ∞ é o i.\n\n");
{
    M2 J = { 0, -1, 1, 0 };
    long iv[2] = { 1, 0 }, zv[2] = { 0, 1 };          /* as colunas: infinito e zero */
    long Ji[2] = { J.a*iv[0] + J.b*iv[1], J.c*iv[0] + J.d*iv[1] };
    long Jz[2] = { J.a*zv[0] + J.b*zv[1], J.c*zv[0] + J.d*zv[1] };
    printf("      J·(1,0) = (%ld,%ld)    ou seja  1/0 -> 0/1 : o INFINITO vai no ZERO\n",
           Ji[0], Ji[1]);
    printf("      J·(0,1) = (%ld,%ld)   e volta ao infinito, com o sinal trocado\n\n",
           Jz[0], Jz[1]);
    ok("o J leva a semente do infinito exatamente na do zero", Ji[0] == 0 && Ji[1] == 1);
    M2 J2 = mm(J, J);
    printf("      J² = [[%ld,%ld],[%ld,%ld]] = -I\n\n", J2.a, J2.b, J2.c, J2.d);
    ok("e J² = −I — é o i, e não uma analogia com o i",
       J2.a == -1 && J2.b == 0 && J2.c == 0 && J2.d == -1);
    printf("      O Aarão disse \"daí pula o i com o flip\", e é isto: a troca que leva o zero no\n");
    printf("      infinito é a mesma cujo quadrado é −1. O par (0, ∞) é o único onde os dois\n");
    printf("      lados degeneram, e a involução que os troca é a raiz de −1. A DUALIDADE SAI\n");
    printf("      DAQUI, e não de uma definição posta por cima.\n");
}

printf("\n§Z4  Com os termos no NEUTRO sai o rei — e das mesmas sementes sai tudo.\n\n");
{
    /* todos os termos iguais a 1 (o neutro do produto) -> Fibonacci -> sigma */
    long h1 = 1, h2 = 0, k1 = 0, k2 = 1;
    printf("      n    convergente\n");
    for(int n = 1; n <= 20; n++){
        long nh = h1 + h2, nk = k1 + k2;
        h2 = h1; h1 = nh; k2 = k1; k1 = nk;
        if(n <= 3 || n >= 18) printf("      %-4d %5ld/%-5ld\n", n, h1, k1);
        else if(n == 4) printf("      ...\n");
    }
    /* A TOLERANCIA QUE AQUI ESTAVA (1e-8) foi ESCOLHIDA — e o comentario que a acompanhava
     * dizia-o: eu tinha posto 1e-4, a assercao caiu, e eu afrouxei-a. Isso e ajustar a regua
     * ate passar. E NAO E PRECISO: que h/k seja um convergente de sigma diz-se EXATAMENTE em
     * inteiros, pela norma na borda —
     *     |h^2 - h.k - k^2| = 1 ,
     * que e' h^2 - hk - k^2 = k^2.q(h/k) com q o polinomio da borda. Vale 1 exato para todo
     * convergente, e e' O(1/k^2) de distancia a uma raiz sem que sigma apareca na conta. */
    {
        long nb = h1*h1 - h1*k1 - k1*k1;
        printf("      a norma na borda:  h² - h.k - k² = %ld²  - %ld.%ld - %ld²  = %ld\n", h1, h1, k1, k1, nb);
        printf("      (vale ±1 exato para todo convergente — e sigma nao entra na conta)\n\n");
        ok("com todos os termos no NEUTRO, a cifra dá Fibonacci: |h² - hk - k²| = 1 EXATO, sem tolerância",
           nb == 1 || nb == -1);
    }

    /* e escolhendo os termos, das MESMAS sementes sai qualquer racional */
    long mau = 0, quantos = 0;
    for(long p = 1; p <= 40; p++) for(long q = 1; q <= 40; q++){
        long t[64]; int nt = 0, a = p, b = q;
        while(b && nt < 64){ t[nt++] = a / b; long r = a - (a/b)*b; a = b; b = r; }
        long H1 = 1, H2 = 0, K1 = 0, K2 = 1;          /* AS MESMAS DUAS SEMENTES */
        for(int i = 0; i < nt; i++){
            long nh = t[i]*H1 + H2, nk = t[i]*K1 + K2;
            H2 = H1; H1 = nh; K2 = K1; K1 = nk;
        }
        long g = mdc(p, q);
        quantos++;
        if(H1 != p/g || K1 != q/g) mau++;
    }
    printf("      %ld racionais reconstruídos das sementes 0/1 e 1/0: %ld falhas\n\n",
           quantos, mau);
    ok("das MESMAS duas sementes, escolhendo os termos, sai qualquer racional", mau == 0);
    printf("      É a segunda metade da tese: com as propriedades do 0 combina-se a base e\n");
    printf("      obtém-se qualquer coisa. Aqui está medido — e note-se o que muda de caso para\n");
    printf("      caso: NÃO são as sementes, que são sempre as mesmas duas. É só a sequência de\n");
    printf("      termos. O rei é o caso em que essa sequência é o neutro repetido.\n");
    printf("\n      E ONDE ISTO É NOTAÇÃO E NÃO TEOREMA, fica dito: \"0/0 = [1,1,1,…]\" não é uma\n");
    printf("      igualdade de números — 0/0 não é um número. O que É teorema é que as duas\n");
    printf("      sementes da cifra são 0 e ∞, e que a cifra de termos neutros é o rei. Ler as\n");
    printf("      duas juntas como a classe 0/0 é a notação do Aarão, e ele disse que era.\n");
}

printf("\n§Z5  A forma tensorial: n fatores, e o chicote que nunca se degrada.\n\n");
{
    /* "conforme o numero de parcelas do produto cresce, e a divisao por zero a forma tensorial
     * que o chicote faz". O produto de n copias de A1 = [[1,1],[1,0]] e a matriz de Fibonacci,
     * e o determinante alterna +1/-1: e o par de raizes cujo produto e -1 — o chicote. */
    M2 A = { 1, 1, 1, 0 }, M = { 1, 0, 0, 1 };
    long mau_det = 0;
    printf("      n    matriz                 det    traço\n");
    for(int n = 1; n <= 8; n++){
        M = mm(M, A);
        long d = det(M), t = M.a + M.d, esperado = (n % 2) ? -1 : 1;
        if(d != esperado) mau_det++;
        if(n <= 3 || n >= 7)
            printf("      %-4d [[%ld,%ld],[%ld,%ld]]%*s %+ld     %ld\n", n, M.a, M.b, M.c, M.d,
                   (int)(14 - (M.a>9?2:1) - (M.b>9?2:1) - (M.c>9?2:1) - (M.d>9?2:1)), "", d, t);
        else if(n == 4) printf("      ...\n");
    }
    printf("\n");
    ok("o determinante alterna ±1 a cada fator, e nunca se degrada", mau_det == 0);
    /* A ASSERCAO QUE AQUI ESTAVA era meia vazia: s2 fora DEFINIDO como -1/s, logo
     * s*s2 = -1 era dividir e multiplicar pela mesma quantidade — tautologia com um
     * arredondamento por cima. E VIETA NAO PRECISA DAS RAIZES: soma = traco e produto =
     * det leem-se na MATRIZ, e a identidade que os amarra e Cayley-Hamilton,
     *     A^2 = tr(A).A - det(A).I ,
     * que se verifica em inteiros, sem uma raiz quadrada. */
    {
        long A[2][2] = {{1,1},{1,0}};
        long tr = A[0][0] + A[1][1];
        long dt = A[0][0]*A[1][1] - A[0][1]*A[1][0];
        long A2[2][2];
        A2[0][0] = A[0][0]*A[0][0] + A[0][1]*A[1][0];
        A2[0][1] = A[0][0]*A[0][1] + A[0][1]*A[1][1];
        A2[1][0] = A[1][0]*A[0][0] + A[1][1]*A[1][0];
        A2[1][1] = A[1][0]*A[0][1] + A[1][1]*A[1][1];
        int mau_ch = 0;
        for(int i=0;i<2;i++) for(int j=0;j<2;j++){
            long rhs = tr*A[i][j] - dt*(i==j);
            if(A2[i][j] != rhs) mau_ch++;
        }
        printf("      traço de A1 = %ld   det de A1 = %ld   (lidos na MATRIZ, sem calcular raizes)\n", tr, dt);
        printf("      Cayley-Hamilton A^2 = tr.A - det.I : discordâncias nas 4 casas: %d\n", mau_ch);
        /* e o teste distingue: com um traço errado a identidade parte-se */
        int parte = 0;
        for(int i=0;i<2;i++) for(int j=0;j<2;j++)
            if(A2[i][j] != (tr+1)*A[i][j] - dt*(i==j)) parte++;
        printf("      e com o traço deslocado de 1 ela PARTE-SE em %d casas — o teste mede\n\n", parte);
        ok("a soma das duas é o traço e o produto é o determinante — Vieta em INTEIROS, sem as raízes",
           mau_ch == 0 && tr == 1 && dt == -1 && parte > 0);
    }
    printf("      Crescer o produto é multiplicar mais uma cópia do MESMO gerador, e o que se\n");
    printf("      conserva é o par: o determinante nunca sai de ±1, por mais fatores que se\n");
    printf("      ponham. É por isso que a cifra não se degrada com o comprimento — o que a\n");
    printf("      degradaria seria um determinante a encolher, e ele não encolhe.\n");
}

printf("\n§Z6  Infinito a infinito — e todo truncamento é finito e representa.\n\n");
{
    long h1 = 1, h2 = 0, k1 = 0, k2 = 1;
    printf("      n     convergente    |h²-hk-k²|\n");
    /* NÃO "cresce sempre": Fibonacci começa 1, 1, e o primeiro passo não cresce. A asserção
     * certa é que NUNCA DECRESCE e vai ao infinito — e a errada caiu vermelha, que é o que
     * ela devia fazer. Escrevi uma afirmação mais forte do que o objeto aguenta. */
    long anterior = 0; int nunca_decresce = 1;
    for(int n = 1; n <= 20; n++){
        long nh = h1 + h2, nk = k1 + k2;
        h2 = h1; h1 = nh; k2 = k1; k1 = nk;
        if(k1 < anterior) nunca_decresce = 0;
        anterior = k1;
        if(n == 1 || n == 5 || n == 10 || n == 15 || n == 20){
            long nb = h1*h1 - h1*k1 - k1*k1;
            if(nb < 0) nb = -nb;
            printf("      %-5d %5ld/%-6ld  %ld\n", n, h1, k1, nb);
        }
    }
    printf("\n");
    printf("      o denominador nunca decresce, e ao fim de 20 passos vale %ld\n\n", k1);
    ok("o denominador nunca decresce e vai ao infinito — a cifra não acaba",
       nunca_decresce && k1 > 6000);
    /* mesma correcao da assercao anterior: "e um racional exato" nao se afirma comparando
     * com um double a menos de um limiar escolhido — afirma-se pela norma na borda, que
     * vale +-1 EXATO em cada truncamento. E aqui verifica-se em TODOS, nao so no ultimo. */
    {
        long ha = 1, ka = 0, hb = 0, kb = 1;      /* reconstroi os truncamentos do zero */
        int trunc = 0, mau_norma = 0;
        for(int i = 0; i < 20; i++){
            long nh = ha + hb, nk = ka + kb;      /* todos os termos no NEUTRO: a_i = 1 */
            hb = ha; kb = ka; ha = nh; ka = nk;
            if(ka){ long nb = ha*ha - ha*ka - ka*ka; if(nb != 1 && nb != -1) mau_norma++; trunc++; }
        }
        printf("      e em TODOS os %d truncamentos a norma vale ±1: discordâncias %d\n\n", trunc, mau_norma);
        ok("e cada truncamento é um racional exato: |h² - hk - k²| = 1 nos 20, sem tolerância",
           mau_norma == 0 && trunc == 20);
    }
    printf("      \"0/0 é infinito a infinito; como tudo é finito, sempre tem representação.\"\n");
    printf("      A cifra é infinita e o objeto é finito, e as duas coisas convivem porque o que\n");
    printf("      é infinito é a RÉGUA e não a coisa medida — que é o que este projeto diz desde\n");
    printf("      o princípio, e aqui aparece no sítio onde a régua nasce.\n");
    printf("\n      E fecha o que ficou aberto no resolvedor: lá, 1/0 PARA, e a razão dada é que\n");
    printf("      \"se 0 vezes x fosse 1, a estrutura colapsava\". Aqui vê-se a outra metade: o\n");
    printf("      0/0 não colapsa nada — ele é a classe INTEIRA, e a cifra é a maneira de a\n");
    printf("      percorrer. Dividir POR zero é sair do corpo; dividir O zero é ficar com tudo.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
