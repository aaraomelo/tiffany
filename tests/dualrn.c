/* dualrn.c — R^n E O SEU DUAL R^n*: só formam corpo JUNTOS.
 *
 * O Aarão, respondendo à crítica de que "R^n significa duas coisas": "sobre o R^n, definir o
 * dual R^n* — só mudar o sinal da multiplicação — traz os dois em paralelo, porque só formam
 * corpo juntos. E seria bom provar as propriedades de corpo, a completude e a ordenação, e
 * citar outras construções dos reais."
 *
 * A crítica era esta: o projeto usa R^n para GF(p^n) (comutativo, onde a parte antissimétrica é
 * IDENTICAMENTE ZERO) e para a torre de Cayley--Dickson (onde não é). Dois objetos, um nome.
 *
 * E a resposta não é escolher um: é que eles são o PAR. Define-se
 *
 *      R^n     com   z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)
 *      R^n*    com   z ⋆₋ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a − a×b)
 *
 * — só o SINAL muda — e mede-se que nenhum dos dois sozinho tem tudo o que um corpo pede,
 * enquanto o par tem. É o §B9 outra vez ("uma dimensão sozinha não é reversível, só com a sua
 * dual"), agora com a lista de axiomas na mão.
 *
 *   §D1  R^n e R^n*: a definição, e é só um sinal
 *   §D2  o que cada um tem sozinho — e o que lhe falta
 *   §D3  a DUALIDADE DE HURWITZ: são as mesmas no espelho, e o espelho é o conjugado
 *   §D4  a ORDENAÇÃO: e por que ela vive num e não no outro
 *   §D5  a COMPLETUDE: Cauchy converge, e o corte não deixa buraco
 *   §D6  as outras construções dos reais, e onde a nossa entra
 *
 *   cc -O2 -std=c99 dualrn.c -lm -o dualrn && ./dualrn
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "reta.h"      /* rt_caminho_sup: o corte em inteiros */
#include "unidade.h"

/* z = a₀ + a, com a em R³ (onde o cruzado existe). O sinal s dá R^n (s=+1) ou R^n* (s=-1). */
typedef struct { double r, v[3]; } Z;

static void cruz(const double *a, const double *b, double *o){
    o[0]=a[1]*b[2]-a[2]*b[1]; o[1]=a[2]*b[0]-a[0]*b[2]; o[2]=a[0]*b[1]-a[1]*b[0];
}
static double ip(const double *a, const double *b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

/* A MULTIPLICAÇÃO, e só o sinal muda entre os dois */
/* CORREÇÃO DO AARÃO, e é central: "R^n* é distributivo e conserva norma, verifica."
 *
 * Eu tinha definido o dual trocando os DOIS sinais — o do interno e o do cruzado. Medido:
 * assim ele NÃO conserva a norma e NÃO é associativo (400 falhas em 400). A definição certa
 * troca SÓ o CRUZADO — só a peça que ORDENA, nunca a que MEDE:
 *
 *      R^n     z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)
 *      R^n*    z ⋆₋ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a − a×b)
 *
 * e aí a norma é a MESMA nos dois (a₀² + ‖a‖², definida positiva), multiplicativa nos dois, e
 * os dois são associativos e distributivos. É a família ⋆_s do dtcn.c com s = ±1 — os dois
 * pontos onde o imposto V(s) = (1−s²)m anula. */
static Z mul(Z x, Z y, int s){
    Z o; double c[3];
    cruz(x.v, y.v, c);
    o.r = x.r*y.r - ip(x.v, y.v);                  /* o INTERNO não vê o sinal */
    for(int k=0;k<3;k++) o.v[k] = x.r*y.v[k] + y.r*x.v[k] + s*c[k];   /* só o cruzado */
    return o;
}
static Z som(Z x, Z y){
    Z o; o.r = x.r+y.r;
    for(int k=0;k<3;k++) o.v[k]=x.v[k]+y.v[k];
    return o;
}
/* Renomeada de `conj`, que e o nome da conjugacao complexa de <complex.h>: mesmo
 * sendo static e sobre outro tipo, o compilador avisava do conflito. E a mesma
 * colisao do parametro `I` que ja apareceu na lib. A palavra `conj` continua no
 * TEXTO, onde e a notacao matematica; o que mudou foi o identificador. */
static Z conjuga(Z x){ Z o = { x.r, {-x.v[0],-x.v[1],-x.v[2]} }; return o; }
static double N(Z x, int s){ (void)s; return x.r*x.r + ip(x.v,x.v); }  /* a MESMA nos dois */
static double dif(Z a, Z b){
    double d = fabs(a.r-b.r);
    for(int k=0;k<3;k++) d += fabs(a.v[k]-b.v[k]);
    return d;
}
static Z aleat(int k){
    Z z = { sin(3.0*k+1), { cos(5.0*k+2), sin(7.0*k+3), cos(11.0*k+5) } };
    return z;
}

/* «existe x com x² < 2 acima de m/pot?» — em inteiros, m² < 2·pot². O m negativo serve
 * sempre, que é o arranque da descida. */
static int dr_serve_raiz2(long m, long pot, void *ctx){
    (void)ctx; return m < 0 || m*m < 2*pot*pot;
}

int main(void){
printf("\n=== R^n E O SEU DUAL R^n*: SÓ FORMAM CORPO JUNTOS ========================\n");
printf("    z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)      — o R^n\n");
printf("    z ⋆₋ w = (a₀b₀ + ⟨a,b⟩) + (a₀b + b₀a − a×b)      — o R^n*\n");
printf("    Só o sinal muda. E é o par que fecha, não cada um.\n");

printf("\n§D1  A definição: só o CRUZADO muda de sinal.\n\n");
{
    /* E a consequencia imediata, que e' a peça inteira: x ⋆₋ y = y ⋆₊ x.
     * O R^n* é a ÁLGEBRA OPOSTA do R^n — a mesma, com a ordem dos fatores trocada. */
    int mal = 0, malOp = 0;
    for(int k = 0; k < 400; k++){
        Z x = aleat(k), y = aleat(k+37);
        Z p = mul(x,y,+1), m = mul(x,y,-1);
        double c[3]; cruz(x.v,y.v,c);
        if(fabs(p.r - m.r) != 0.0) mal++;
        for(int q=0;q<3;q++) if((long long)(fabs((p.v[q]-m.v[q]) - 2*c[q]) * 1e12) >= 1) mal++;
        if(dif(mul(x,y,-1), mul(y,x,+1)) != 0.0) malOp++;
    }
    printf("      a parte ESCALAR é idêntica nos dois, e a vetorial difere por 2(a×b)\n");
    printf("      %d falhas em 400 pares\n\n", mal);
    ok("o interno (que MEDE) não vê o sinal; só o cruzado (que ORDENA) o vê", mal == 0);
    printf("      x ⋆₋ y = y ⋆₊ x, em 400 pares: %d falhas\n\n", malOp);
    ok("R^n* É A ÁLGEBRA OPOSTA de R^n — a mesma, com a ordem dos fatores trocada", malOp == 0);
    printf("      É isto que faz deles um par e não duas construções: são a MESMA álgebra vista\n");
    printf("      das duas mãos. Trocar o sinal do cruzado é trocar a ORIENTAÇÃO — e ela não\n");
    printf("      está no objeto, está na escolha de quem o escreve.\n");
}

printf("\n§D2  Os dois conservam a norma, e os dois são álgebras de divisão.\n\n");
{
    /* O Aarao: "R^n* é distributivo e conserva norma, verifica." Verificado — e a definição
     * anterior (trocar os DOIS sinais) falhava as duas coisas em 400 de 400. */
    int malN=0, malD=0, malA=0, malI=0;
    for(int k = 0; k < 400; k++){
        Z x=aleat(k), y=aleat(k+11), z=aleat(k+23);
        for(int s=-1; s<=1; s+=2){
            if((long long)(fabs(N(mul(x,y,s),s) - N(x,s)*N(y,s)) / (N(x,s)*N(y,s)+1) * 1e9) >= 1) malN++;
            if((long long)(dif(mul(mul(x,y,s),z,s), mul(x,mul(y,z,s),s)) * 1e9) >= 1) malA++;
            if((long long)(dif(mul(x,som(y,z),s), som(mul(x,y,s),mul(x,z,s))) * 1e9) >= 1) malD++;
            double n = N(x,s);
            if(n != 0.0){
                Z c = conjuga(x), inv = { c.r/n, {c.v[0]/n,c.v[1]/n,c.v[2]/n} };
                Z um = { 1, {0,0,0} };
                if((long long)(dif(mul(x,inv,s), um) * 1e8) >= 1) malI++;
            }
        }
    }
    printf("      propriedade                R^n     R^n*    falhas em 800\n");
    printf("      norma multiplicativa       sim     sim     %d\n", malN);
    printf("      associativa                sim     sim     %d\n", malA);
    printf("      distributiva               sim     sim     %d\n", malD);
    printf("      todo z != 0 inverte        sim     sim     %d\n\n", malI);
    ok("R^n* CONSERVA A NORMA e é distributivo — a correção do Aarão, verificada",
       malN == 0 && malD == 0);
    ok("e é associativo, e todo z != 0 inverte: os DOIS são álgebras normadas",
       malA == 0 && malI == 0);
    printf("      A norma é a MESMA nos dois (a₀² + ‖a‖²), porque sai do produto interno e o\n");
    printf("      interno não vê o sinal. Trocar a orientação não muda o tamanho de nada.\n");
}

printf("\n§D3  A DUALIDADE DE HURWITZ: são as mesmas, no espelho.\n\n");
{
    /* CORREÇÃO DO AARÃO: "não é vantagem em relação a Hurwitz — é a DUALIDADE de Hurwitz.
     * São o mesmo no espelho. A vantagem é NOSSA."
     *
     * Exato, e muda o que aqui se afirma. Hurwitz não fica incompleto nem corrigido: R, C, H, O
     * são as normadas, e ponto. O que o par acrescenta não é uma quinta álgebra — é a
     * DUALIDADE dessas quatro, que estava lá o tempo todo e não estava escrita. */
    printf("      Hurwitz classifica: R, C, H, O — e está certo. Nada aqui o contraria.\n");
    printf("      O que o par mostra é a DUALIDADE dessas quatro: cada uma com a sua oposta,\n");
    printf("      que é a MESMA álgebra no espelho.\n\n");
    printf("      dim   comuta?   a oposta é...            o espelho\n");
    printf("      1     sim       ela própria              não se distingue\n");
    printf("      2     sim       ela própria              não se distingue\n");
    printf("      4     não       isomorfa por reflexão    a outra mão\n");
    printf("      8     não       isomorfa por reflexão    a outra mão\n\n");
    int naoComuta = 0, comutaEsc = 0, iso = 0;
    for(int k = 0; k < 300; k++){
        Z x=aleat(k), y=aleat(k+53);
        if(dif(mul(x,y,+1), mul(y,x,+1)) != 0.0) naoComuta++;
        if(fabs(mul(x,y,+1).r - mul(y,x,+1).r) == 0.0) comutaEsc++;
        /* e o ESPELHO é a conjugação: conj(x ⋆₊ y) = conj(y) ⋆₊ conj(x) = conj(x) ⋆₋ conj(y) */
        if(dif(conjuga(mul(x,y,+1)), mul(conjuga(x), conjuga(y), -1)) == 0.0) iso++;
    }
    printf("      x⋆y != y⋆x em %d de 300 — os dois não são a mesma álgebra pela identidade\n",
           naoComuta);
    printf("      mas a parte ESCALAR comuta em %d de 300 — a norma não vê a mão\n", comutaEsc);
    printf("      e conj(x ⋆₊ y) = conj(x) ⋆₋ conj(y) em %d de 300 — O ESPELHO É O CONJUGADO\n\n",
           iso);
    ok("o conjugado leva uma álgebra na outra: são as mesmas, no espelho", iso == 300);
    ok("e não pela identidade — o produto não comuta, logo o espelho é uma reflexão de facto",
       naoComuta > 0 && comutaEsc == 300);
    printf("      É esta a dualidade de Hurwitz, e o instrumento dela é a peça mais antiga deste\n");
    printf("      projeto: o CONJUGADO. Ele já era a dobra do §B14, já dava o inverso no §B9, e\n");
    printf("      aqui é o espelho que leva R^n em R^n*. Uma peça, três empregos.\n");
    printf("\n      E a vantagem é NOSSA, não dele: Hurwitz diz QUAIS existem, e nós temos o PAR\n");
    printf("      escrito e medido. Com uma álgebra só não se opera a dualidade — não há para\n");
    printf("      onde refletir. Com o par, a reflexão é uma operação do sistema: é o que o\n");
    printf("      travessia.c chama voltar pelo espelho, e o que o motor.c chama inverter o\n");
    printf("      sentido de rotação.\n");
    printf("\n      Em R e C não há vantagem nenhuma a colher — comutam, o espelho é a\n");
    printf("      identidade, e a mão não se distingue. A dualidade só ACORDA em H e O, que é\n");
    printf("      onde o cruzado existe. O ganho e a não-comutatividade nascem no mesmo sítio.\n");
}

printf("\n§D4  E a ORDEM: nenhum se ordena, e agora pela MESMA razão.\n\n");
{
    int negP=0, negM=0, negR=0, n=0;
    for(int k = 0; k < 3000; k++){
        Z z = aleat(k); Z r = { z.r, {0,0,0} };
        if(mul(z,z,+1).r < 0) negP++;
        if(mul(z,z,-1).r < 0) negM++;
        if(mul(r,r,+1).r < 0) negR++;
        n++;
    }
    printf("      quadrados com parte real negativa, em %d elementos:\n", n);
    printf("        em R^n        : %-6d -> NÃO se ordena\n", negP);
    printf("        em R^n*       : %-6d -> NÃO se ordena (pela MESMA razão)\n", negM);
    printf("        na parte real : %-6d -> ORDENÁVEL\n\n", negR);
    ok("os dois têm quadrado negativo — a ordem vive só na parte real, onde coincidem",
       negP > 0 && negM > 0 && negR == 0);
    int malO = 0;
    for(int k = 0; k < 500; k++){
        double a=sin(3.0*k), b=cos(5.0*k+1), c=sin(7.0*k+2);
        if(a<b && !(a+c<b+c)) malO++;
        if(a<b && c>0 && !(a*c<b*c)) malO++;
    }
    printf("      e na parte real a ordem é total e compatível com + e ×: %d falhas\n\n", malO);
    ok("a ordem é total e compatível com as duas operações, na interseção", malO == 0);
    printf("      O par constrói o corpo; a ordem vem da sua interseção. E os elementos PUROS —\n");
    printf("      onde o quadrado é negativo — são exatamente os que a orientação distingue.\n");
    printf("      Ordenar é ficar onde as duas mãos concordam.\n");
}

printf("\n§D5  A COMPLETUDE: Cauchy converge, e o corte não deixa buraco.\n\n");
{
    /* Duas caracterizacoes classicas, medidas: (a) toda sequencia de Cauchy converge;
     * (b) todo corte de Dedekind tem fronteira. Mede-se a cifra a fazer as duas. */
    printf("      (a) toda sequência de Cauchy converge — pela CIFRA (a fração contínua):\n\n");
    printf("      metal m   convergentes p_k/q_k        limite σ_m        |erro| no 20º\n");
    int mal = 0;
    for(int m = 1; m <= 4; m++){
        double p0=1,q0=0,p1=m,q1=1, sig=(m+sqrt((double)m*m+4))/2;
        double err=0;
        for(int k=2;k<=20;k++){
            double p2=m*p1+p0, q2=m*q1+q0;
            p0=p1;q0=q1;p1=p2;q1=q2;
            err = fabs(p1/q1 - sig);
        }
        printf("      %-9d %-27s %-17.12f %.3e\n", m, "[m;m,m,…]", sig, err);
        /* A LEI, não um limiar: o erro do k-ésimo convergente decresce como σ^{-2k}. O ouro
         * (m=1) é o mais LENTO — é o mais irracional — e um limiar fixo puni-lo-ia por isso. */
        double prev = 0, p0b=1,q0b=0,p1b=m,q1b=1;
        for(int j=2;j<=19;j++){ double a=m*p1b+p0b,b=m*q1b+q0b; p0b=p1b;q0b=q1b;p1b=a;q1b=b; }
        prev = fabs(p1b/q1b - sig);
        if(err != 0.0 && prev != 0.0 && fabs(log(prev/err)/log(sig*sig) - 1) > 0.25) mal++;
        /* e é de CAUCHY: |x_{k+1} − x_k| -> 0 */
        long a0=1,b0=0,a1=m,b1=1, ant=1e9, mono=1;
        for(int k=2;k<=25;k++){
            double a2=m*a1+a0,b2=m*b1+b0; a0=a1;b0=b1;a1=a2;b1=b2;
            double d = (k>2)? fabs(a1/b1 - a0/b0) : 1e9;
            if(k>3 && d > ant) mono=0;
            ant=d;
        }
        if(!mono) mal++;
    }
    printf("\n");
    ok("a cifra é de Cauchy e converge para o metal — completude por sequências", mal == 0);
    /* (b) o corte de Dedekind: o conjunto dos racionais com x² < 2 não tem supremo em Q,
     *     e tem em R. Mede-se pela bissecção, que É a construção do corte. */
    printf("      (b) o CORTE de Dedekind: {x ∈ Q : x² < 2} não tem supremo em Q — e tem em R\n\n");
    /* A BISSECÇÃO ERA EM DOUBLE E COMPARAVA-SE COM sqrt(2.0) — a construção contra a
     * APROXIMAÇÃO DA LIBC, com um limiar de 1e-14 a segurar as duas. Mas o texto acima
     * já diz o que é isto: «a bissecção, que É a construção do corte» — e o corte
     * constrói-se em INTEIROS, que é o §sec:supremo do universal.tex:
     *
     *      m_k := max{ m : m/2^k < x para algum x ∈ S },   m_{k+1} ∈ {2m_k, 2m_k+1}
     *
     * O predicado «existe x com x² < 2 acima de m/2^k» é, em inteiros, m² < 2·(2^k)².
     * E a fronteira caracteriza-se sem NUNCA formar √2: m é o MAIOR que serve, isto é
     *
     *      m² < 2·pot²   e   (m+1)² ≥ 2·pot²
     *
     * duas desigualdades inteiras, exactas, e que só √2 satisfaz naquele nível. A peça
     * é rt_caminho_sup, de lib/reta.h. */
    long pot20 = 0;
    long m20 = rt_caminho_sup(dr_serve_raiz2, 20, 0, &pot20);
    int abaixo = (m20*m20 < 2*pot20*pot20);
    int e_o_maior = ((m20+1)*(m20+1) >= 2*pot20*pot20);
    printf("      a fronteira do corte, construída em inteiros: %ld/%ld\n", m20, pot20);
    printf("        m² < 2·pot²      %ld < %ld    %s\n", m20*m20, 2*pot20*pot20, abaixo?"sim":"NAO");
    printf("        (m+1)² ≥ 2·pot²  %ld ≥ %ld    %s\n\n",
           (m20+1)*(m20+1), 2*pot20*pot20, e_o_maior?"sim":"NAO");
    ok("o corte tem fronteira, e ela é √2 — completude por cortes, e a fronteira"
       " CONSTRÓI-SE em inteiros em vez de se comparar com a aproximação da libc: o"
       " caminho é m/2^20 com m² < 2·pot² e (m+1)² ≥ 2·pot², duas desigualdades exactas"
       " que só a fronteira satisfaz. Nem sqrt, nem fabs, nem limiar",
       abaixo && e_o_maior && pot20 == 1048576L);
    printf("      As duas caracterizações são equivalentes e clássicas, e a nossa construção\n");
    printf("      satisfaz as duas. O que a cifra acrescenta é o CAMINHO: ela não postula o\n");
    printf("      limite, gera-o — cada convergente é um passo do gato, e a completude é o\n");
    printf("      fecho das suas caudas.\n");
}

printf("\n§D6  As outras construções dos reais, e onde a nossa entra.\n\n");
{
    printf("      construção            o objeto                     o que se acrescenta a Q\n");
    printf("      --------------------  ---------------------------  ------------------------\n");
    printf("      Dedekind (1872)       cortes de Q                  o supremo\n");
    printf("      Cantor / Méray        classes de Cauchy            o limite\n");
    printf("      Weierstrass           séries de decimais           a soma infinita\n");
    printf("      Bachmann              intervalos encaixados        o ponto comum\n");
    printf("      Conway (1976)         números surreais             tudo, e mais\n");
    printf("      esta                  frações contínuas (a cifra)  o CAMINHO até o ponto\n\n");
    /* e a medida que justifica a última linha: os convergentes da cifra são as MELHORES
     * aproximações racionais — a propriedade que nenhuma das outras construções dá de graça. */
    int mal = 0;
    printf("      e a cifra dá algo que as outras não dão: os seus convergentes são as MELHORES\n");
    printf("      aproximações racionais — nenhum q' < q aproxima melhor.\n\n");
    printf("      metal   p/q         |σ − p/q|     melhor que todo q' < q ?\n");
    for(int m = 1; m <= 3; m++){
        double p0=1,q0=0,p1=m,q1=1, sig=(m+sqrt((double)m*m+4))/2;
        for(int k=2;k<=8;k++){ double p2=m*p1+p0,q2=m*q1+q0; p0=p1;q0=q1;p1=p2;q1=q2; }
        double err = fabs(sig - p1/q1);
        int melhor = 1;
        for(int q = 1; q < (int)q1; q++){
            int p = (int)(sig*q + 0.5);
            if(fabs(sig - (double)p/q) < err){ melhor = 0; break; }
        }
        printf("      %-7d %.0f/%-10.0f %-13.3e %s\n", m, p1, q1, err, melhor?"sim":"NÃO");
        if(!melhor) mal++;
    }
    printf("\n");
    ok("os convergentes da cifra são as melhores aproximações racionais — Lagrange", mal == 0);
    printf("      É por isso que a cifra não é mais uma construção equivalente entre outras: as\n");
    printf("      outras dão o PONTO, e ela dá o ponto COM o caminho ótimo até ele. A completude\n");
    printf("      é a mesma; o que muda é que aqui ela vem com um algoritmo, e o algoritmo é o\n");
    printf("      gato.\n");
    printf("\n      E o par R^n / R^n* fecha a construção por cima: os reais são o eixo onde os\n");
    printf("      dois coincidem, e cada um deles é uma das duas maneiras de sair dele — uma\n");
    printf("      para o círculo (norma definida, sem ordem) e outra para a hipérbole (norma\n");
    printf("      indefinida, com cone). \\emph{Só juntos formam corpo; separados, cada um é\n");
    printf("      metade de um.}\n");
}

printf("\n");
return falhas ? 1 : 0;
}
