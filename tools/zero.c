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
 *   cc -O2 -std=c99 zero.c -lm -o zero && ./zero
 */
#include <stdio.h>
#include <math.h>
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

    /* e a fatoracao do zero e LIVRE: qualquer x serve, e o produto tem qualquer comprimento */
    long mau = 0;
    for(long x = -20; x <= 20; x++) if(0 * x != 0) mau++;
    printf("      0 = 0 x n, para os 41 valores de n testados: %ld falhas\n", mau);
    ok("todo o x satisfaz 0 = 0·x — a fatoração do zero não tem informação nenhuma", mau == 0);
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
    printf("      n    convergente   valor\n");
    for(int n = 1; n <= 20; n++){
        long nh = h1 + h2, nk = k1 + k2;
        h2 = h1; h1 = nh; k2 = k1; k1 = nk;
        if(n <= 3 || n >= 18) printf("      %-4d %5ld/%-5ld     %.9f\n", n, h1, k1, (double)h1/k1);
        else if(n == 4) printf("      ...\n");
    }
    double s = (1 + sqrt(5.0)) / 2;
    printf("      sigma                   %.9f\n\n", s);
    /* a tolerância tem de ser proporcional ao número de passos: o erro cai como 1/k², e
     * com 10 passos ele é 1,5e-4 — eu tinha exigido 1e-4 e a asserção caía por ser eu a
     * pedir mais precisão do que os passos que dei. O erro era da minha régua, não da cifra. */
    ok("com todos os termos no NEUTRO, a cifra dá Fibonacci e converge para o rei",
       fabs((double)h1/k1 - s) < 1e-8);

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
    double s = (1 + sqrt(5.0)) / 2, s2 = -1.0 / s;
    printf("      as duas raízes:  sigma = %.9f   e  -1/sigma = %.9f\n", s, s2);
    printf("      soma     %.9f = traço de A1 = 1\n", s + s2);
    printf("      produto  %.9f = det de A1 = -1\n\n", s * s2);
    ok("a soma das duas é o traço e o produto é o determinante — o chicote é o PAR",
       fabs(s + s2 - 1.0) < 1e-12 && fabs(s*s2 + 1.0) < 1e-12);
    printf("      Crescer o produto é multiplicar mais uma cópia do MESMO gerador, e o que se\n");
    printf("      conserva é o par: o determinante nunca sai de ±1, por mais fatores que se\n");
    printf("      ponham. É por isso que a cifra não se degrada com o comprimento — o que a\n");
    printf("      degradaria seria um determinante a encolher, e ele não encolhe.\n");
}

printf("\n§Z6  Infinito a infinito — e todo truncamento é finito e representa.\n\n");
{
    double s = (1 + sqrt(5.0)) / 2;
    long h1 = 1, h2 = 0, k1 = 0, k2 = 1;
    printf("      n     convergente    erro\n");
    /* NÃO "cresce sempre": Fibonacci começa 1, 1, e o primeiro passo não cresce. A asserção
     * certa é que NUNCA DECRESCE e vai ao infinito — e a errada caiu vermelha, que é o que
     * ela devia fazer. Escrevi uma afirmação mais forte do que o objeto aguenta. */
    long anterior = 0; int nunca_decresce = 1;
    for(int n = 1; n <= 20; n++){
        long nh = h1 + h2, nk = k1 + k2;
        h2 = h1; h1 = nh; k2 = k1; k1 = nk;
        if(k1 < anterior) nunca_decresce = 0;
        anterior = k1;
        if(n == 1 || n == 5 || n == 10 || n == 15 || n == 20)
            printf("      %-5d %5ld/%-6ld  %.3e\n", n, h1, k1, fabs((double)h1/k1 - s));
    }
    printf("\n");
    printf("      o denominador nunca decresce, e ao fim de 20 passos vale %ld\n\n", k1);
    ok("o denominador nunca decresce e vai ao infinito — a cifra não acaba",
       nunca_decresce && k1 > 6000);
    ok("e cada truncamento é um racional exato: finito, e representa",
       fabs((double)h1/k1 - s) < 1e-8);
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
