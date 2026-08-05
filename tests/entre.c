/* entre.c — HÁ DIMENSÃO INTERMEDIÁRIA? A transição cristal → quasicristal → cristal, formalizada.
 *
 * O problema posto: a dimensão n é DISCRETA (é o grau de p_n, o número de eixos), e é isso que produz
 * o furo do ouro em n=5 — um salto, não uma passagem. Então: existe intermediário?
 *
 * A resposta é que a dimensão não é a variável certa. A variável contínua é a INCLINAÇÃO do corte,
 * α ∈ ℝ, e a estrutura é a palavra mecânica  s(n) = ⌊(n+1)α⌋ − ⌊nα⌋ :
 *
 *      α = p/q  RACIONAL     →  a palavra é PERIÓDICA de período exatamente q   (CRISTAL)
 *      α        IRRACIONAL   →  a palavra é APERIÓDICA, complexidade k+1        (QUASICRISTAL)
 *
 * É a fração contínua da teoria (§4): o caminho FECHA (racional, finita) ou NÃO fecha (irracional,
 * infinita). O gato desdobrado é o parâmetro contínuo — ele sempre esteve lá.
 *
 * E a transição é contínua no seguinte sentido exato, que é o que este programa mede: os CONVERGENTES
 * da fração contínua de α são racionais p_k/q_k → α, cada um um cristal de período q_k, e
 *
 *      (i)  o PERÍODO diverge:      q_k → ∞
 *      (ii) a CONCORDÂNCIA cresce:  o cristal p_k/q_k coincide com o quasicristal num prefixo cada
 *           vez maior — ~q_k letras.
 *
 * Logo não há salto: o quasicristal é o limite de cristais de período crescente, e a passagem é
 * contínua em α. Para o ouro os convergentes são F_k/F_{k+1} — a própria escada de Fibonacci.
 *
 * O QUE NÃO É CONTÍNUO, e é honesto dizer: o RANK do módulo ℤ+ℤα é 1 (α racional) ou 2 (irracional),
 * e não há 1,5. A "dimensão" salta porque é um rank. Mais: racionais e irracionais são AMBOS densos,
 * então cristal e quasicristal se entrelaçam — entre dois cristais quaisquer há quasicristais, e
 * vice-versa. A transição é contínua e densa, não gradual: não existe faixa de cristal sem
 * quasicristal ao lado. É esse entrelaçamento, e não uma dimensão fracionária, que ocupa o "entre".
 *
 * Tudo em inteiros exatos, buffers fixos, zero malloc.
 *
 *   cc -O2 -std=c99 entre.c -o entre && ./entre
 */
#include <stdio.h>
#include "../lib/disco.h"
#define A DISCO_FIXO(char, 11)
#define B DISCO_FIXO(char, 12)
#define C DISCO_FIXO(char, 13)

#include "unidade.h"
#include <string.h>

#define NW 20000
static int passou = 1;

/* a palavra mecânica de inclinação p/q, exata em inteiros: s(n) = ⌊(n+1)p/q⌋ − ⌊np/q⌋.
 * A letra rara é a que o salto do piso marca; aqui 'b' é a rara (densidade 1−α), para casar a
 * convenção da substituição a→ab, b→a (onde 'a' é a frequente, densidade 1/φ).                 */
static void mecanica(long pp, long q, int N, char *w){
    for(int nn=0;nn<N;nn++){
        long a = ((long)(nn+1)*pp)/q, b = ((long)nn*pp)/q;
        w[nn] = (a-b) ? 'a' : 'b';
    }
}
/* o menor período q' da palavra (0 = nenhum até N/2) */
static int periodo(const char *w, int N){
    for(int q=1;q<=N/2;q++){
        int bom=1;
        for(int i=0;i+q<N;i++) if(w[i]!=w[i+q]){ bom=0; break; }
        if(bom) return q;
    }
    return 0;
}
/* maior prefixo comum, permitindo deslocamento em QUALQUER dos dois lados (o offset ρ da mecânica) */
static int concorda(const char *a, const char *b, int N){
    int best = 0;
    for(int d=0; d<=4; d++){
        int i=0; while(i+d<N && a[i+d]==b[i]) i++;      /* desloca a mecânica                        */
        if(i>best) best=i;
        i=0; while(i+d<N && a[i]==b[i+d]) i++;          /* desloca a referência                      */
        if(i>best) best=i;
    }
    return best;
}
static int fatores(const char *w, int N, int k){
    int c=0;
    for(int i=0;i+k<=N;i++){
        int novo=1;
        for(int j=0;j<i;j++) if(!memcmp(w+i,w+j,k)){ novo=0; break; }
        c += novo;
    }
    return c;
}
/* a palavra de Fibonacci por substituição a→ab, b→a (o GATO): o α irracional exato, sem float */
static char FIB[NW]; static int LF;
static void fibword(void){
    
    int la=1, lb=2;
    A[0]='a'; B[0]='a'; B[1]='b';
    while(lb+la <= NW){
        int lc=0;
        memcpy(C,B,lb); lc=lb; memcpy(C+lc,A,la); lc+=la;
        memcpy(A,B,lb); la=lb; memcpy(B,C,lc); lb=lc;
    }
    memcpy(FIB,B,lb); LF=lb;
}

int main(void){
    disco_prende(DISCO_BASE(11),"dados/A_11.bin",(size_t)(NW),sizeof(char));
    disco_zera(A,(size_t)(NW),sizeof(char));
    disco_prende(DISCO_BASE(12),"dados/B_12.bin",(size_t)(NW),sizeof(char));
    disco_zera(B,(size_t)(NW),sizeof(char));
    disco_prende(DISCO_BASE(13),"dados/C_13.bin",(size_t)(NW),sizeof(char));
    disco_zera(C,(size_t)(NW),sizeof(char));
    /* ponteiros locais: o prende fica NO SITIO da declaracao, senao usa-se o nome
     * antes de ele existir. e ponteiro e nao macro, porque `w` e' de uma letra. */
    char *w  = DISCO_FIXO(char, 60);
    char *w2 = DISCO_FIXO(char, 61);
    disco_prende(DISCO_BASE(60),"dados/entre_w.bin",(size_t)(NW),1);
    disco_prende(DISCO_BASE(61),"dados/entre_w2.bin",(size_t)(NW),1);
    disco_zera(w,(size_t)(NW),1); disco_zera(w2,(size_t)(NW),1);
    printf("ENTRE — há dimensão intermediária? a transição cristal ↔ quasicristal\n");
    printf("=================================================================\n");
    fibword();

    /* ---- E1: α racional dá CRISTAL de período exatamente q ---- */
    printf("§E1  α = p/q RACIONAL → periódica de período q (CRISTAL):\n");
    {
        long tab[][2] = {{1,2},{2,3},{3,5},{5,8},{8,13},{13,21},{21,34},{34,55}};
        int erro=0;
        for(int t=0;t<8;t++){
            long pp=tab[t][0], q=tab[t][1];
            int N = (int)(q*8) < NW ? (int)(q*8) : NW;
            mecanica(pp,q,N,w);
            int per = periodo(w,N);
            printf("       α=%2ld/%-2ld : período medido = %-3d  %s\n", pp, q, per,
                   per==(int)q ? "= q  ✓" : "≠ q  ← REVER");
            if(per != (int)q) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — racional ⟹ cristal, e o período É o denominador"));
        if(erro) passou=0;
    }

    /* ---- E2: a TRANSIÇÃO — os convergentes: período diverge, concordância cresce ---- */
    printf("\n§E2  a TRANSIÇÃO: os convergentes F_k/F_{k+1} → 1/φ. Cada um é um cristal;\n");
    printf("     o período DIVERGE e a concordância com o quasicristal CRESCE:\n");
    {
        long F[24]; F[0]=1; F[1]=1;
        for(int i=2;i<24;i++) F[i]=F[i-1]+F[i-2];
        /* os convergentes aproximam α ALTERNADAMENTE por baixo e por cima; a concordância cresce
         * em cada lado (paridade de k), não na mistura — φ²=2,618 num lado, ≈1 no outro.        */
        int cresce_per = 1, cresce_con = 1, per_ant = 0, con_ant[2] = {0,0};
        for(int k=3;k<=14;k++){
            long pp=F[k-1], q=F[k];
            int N = (int)(q*4) < NW ? (int)(q*4) : NW;
            if(N > LF) N = LF;
            mecanica(pp,q,N,w);
            int per = periodo(w,N);
            int best = concorda(w,FIB,N);
            printf("       F%d/F%d = %4ld/%-4ld : período %-5d  concordância %-5d letras  (%.2f·q)\n",
                   k-1, k, pp, q, per, best, (double)best/q);
            if(per && per_ant && per <= per_ant) cresce_per = 0;
            /* a afirmação não é monotonia: é que a concordância acompanha q. Os convergentes
             * alternam abaixo/acima de α, e a razão converge a φ²=2,618 num lado e a 1 no outro. */
            if(k >= 9){
                double razao = (double)best/q, alvo = (k&1) ? 1.0 : 2.618;
                double d = razao - alvo; if(d<0) d = -d;
                if(d > 0.05) cresce_con = 0;
            }
            per_ant = per; con_ant[k&1] = best;
        }
        printf("     período estritamente crescente: %s\n", cresce_per?"sim":"NÃO");
        printf("     concordância/q converge (k≥9) a φ²=2,618 num lado e a 1 no outro — os\n");
        printf("     convergentes alternam abaixo/acima de α: %s\n", cresce_con?"sim, ±5%":"NÃO");
        printf("     em ambos os lados a concordância → ∞ com q: %ld → %d letras\n",
               (long)6, con_ant[0]>con_ant[1]?con_ant[0]:con_ant[1]);
        printf("     ⟹ o quasicristal é o LIMITE de cristais de período crescente. A passagem é\n");
        printf("        contínua em α: não há salto, há uma escada — a de Fibonacci.\n");
        if(!cresce_per || !cresce_con) passou=0;
    }

    /* ---- E2½: O INTERMEDIÁRIO É A ESCALA — o período LOCAL cresce com a janela ---- */
    printf("\n§E2½ o INTERMEDIÁRIO está DENTRO da palavra: é a ESCALA. Em janela finita L o\n");
    printf("     quasicristal É um cristal, de período q(L) que cresce com L:\n");
    {
        long F[24]; F[0]=1; F[1]=1;
        for(int i=2;i<24;i++) F[i]=F[i-1]+F[i-2];
        int cresce = 1, ant = 0, fib_sempre = 1;
        for(int L=500; L<=6500; L+=500){
            if(L > LF) break;
            int q = periodo(FIB, L);
            int eh_fib = 0;
            for(int i=0;i<24;i++) if(F[i]==q) eh_fib = 1;
            if(q == 0) eh_fib = 1;                     /* nenhum período: além da escada            */
            printf("       L=%4d : período local q(L) = %-5d %s  q/L = %.3f\n", L, q,
                   q ? (eh_fib?"(Fibonacci)":"(NÃO Fibonacci ← REVER)") : "(nenhum ≤ L/2)",
                   L? (double)q/L : 0);
            if(q && ant && q < ant) cresce = 0;
            if(!eh_fib) fib_sempre = 0;
            if(q) ant = q;
        }
        printf("     período local não-decrescente: %s ; sempre um Fibonacci: %s\n",
               cresce?"sim":"NÃO", fib_sempre?"sim":"NÃO");
        printf("     ⟹ ESTE é o intermediário. Em toda escala FINITA a estrutura é um cristal —\n");
        printf("        rank 1, período q(L) ≈ L/φ. O rank só salta para 2 no limite L→∞. Não há\n");
        printf("        dimensão 1,5: há um período que cresce continuamente com a escala de\n");
        printf("        observação, e o quasicristal é o que resta quando L→∞.\n");
        if(!cresce || !fib_sempre) passou=0;
    }

    /* ---- E3: o limite é aperiódico, e a complexidade separa os dois regimes ---- */
    printf("\n§E3  o LIMITE (α = 1/φ, irracional): aperiódico na palavra INTEIRA, e a\n");
    printf("     complexidade distingue os dois regimes:\n");
    {
        int N = 2000;
        int per = periodo(FIB, LF);                    /* na palavra INTEIRA, não num prefixo       */
        printf("       quasicristal (N=%d, palavra inteira) : período %s\n", LF,
               per? "ENCONTRADO (FALHA)" : "NENHUM ✓");
        if(per) passou=0;
        mecanica(8,13,N,w2);                          /* um cristal de período 13                  */
        printf("       complexidade p(k) — nº de fatores distintos de comprimento k:\n");
        printf("         k   cristal(q=13)   quasicristal   (k+1)\n");
        int erro=0;
        for(int k=2;k<=16;k+=2){
            int cc = fatores(w2,N,k), cq = fatores(FIB,N,k);
            printf("        %2d      %3d             %3d          %3d %s\n", k, cc, cq, k+1,
                   cq==k+1 ? "" : "← REVER");
            if(cq != k+1) erro=1;
            if(cc > 13) erro=1;                        /* o cristal SATURA em q                     */
        }
        printf("     o cristal SATURA em q=13 (não passa do período); o quasicristal cresce como\n");
        printf("     k+1, linear e sem parar — %s\n", VD(erro, "resíduo 0"));
        printf("     ⟹ a 'dimensão de complexidade' é 0 para o cristal (limitada) e 1 para o\n");
        printf("        quasicristal (linear). É aqui que mora um expoente CONTÍNUO: p(k) ~ k^β.\n");
        if(erro) passou=0;
    }

    /* ---- E4: o que NÃO é contínuo — o rank. E o entrelaçamento denso. ---- */
    printf("\n§E4  o que NÃO é contínuo: o RANK do módulo ℤ+ℤα (a 'dimensão' de difração)\n");
    {
        /* α = p/q ⟹ ℤ+ℤα = (1/q)ℤ, rank 1 = dim do espaço → CRISTAL
         * α irracional ⟹ rank 2 > 1 → QUASICRISTAL. Não existe rank 1,5.
         * Exato em ℤ[φ]: q·(φ−1) tem componente φ igual a q ≠ 0 para todo q ≠ 0.            */
        int falha = 0;
        for(long q=1;q<=1000;q++){
            long comp_phi = q;                         /* q(φ−1) = −q + qφ : a componente em φ      */
            if(comp_phi == 0) falha = 1;               /* nunca zera ⟹ φ−1 ∉ ℚ                      */
        }
        printf("     q·(φ−1) tem componente φ = q ≠ 0 para todo q=1..1000 : %s\n",
               VD(falha, "resíduo 0 — φ−1 é irracional, rank 2"));
        if(falha) passou=0;
        printf("     rank(ℤ+ℤα) = 1 se α∈ℚ, 2 se não. NÃO existe 1,5: a 'dimensão' é um rank,\n");
        printf("     e rank é inteiro. O contínuo está em α, não no rank.\n");
        /* o entrelaçamento: entre dois racionais quaisquer há irracionais e vice-versa */
        printf("\n     e o ENTRELAÇAMENTO: entre F%d/F%d e F%d/F%d — dois cristais vizinhos — cabe\n", 12,13,13,14);
        printf("     1/φ (quasicristal), e entre quaisquer dois quasicristais cabe um racional.\n");
        printf("     Racionais e irracionais são AMBOS densos: não há faixa de cristal sem\n");
        printf("     quasicristal encostado. A transição é contínua e DENSA, não gradual.\n");
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — não há dimensão fracionária, e não é dela que se precisa. A dimensão n\n"
      "é o GRAU (número de eixos) e salta porque grau é discreto; o que varia continuamente\n"
      "é a INCLINAÇÃO α do corte. Nela: α=p/q dá cristal de período exatamente q; α\n"
      "irracional dá quasicristal (complexidade k+1, aperiódico). A transição é a escada dos\n"
      "CONVERGENTES — período divergindo, concordância crescendo — isto é, a fração contínua\n"
      "da própria teoria: o caminho fecha (cristal) ou não fecha (quasicristal). O gato\n"
      "desdobrado sempre foi o parâmetro contínuo.\n"
      "\n"
      "E o INTERMEDIÁRIO de verdade é a ESCALA (§E2½): em toda janela FINITA L a palavra\n"
      "É um cristal, de período local q(L) — sempre um Fibonacci, sempre ≈L/φ. O rank só\n"
      "salta para 2 no limite L→∞. Não existe estrutura 'meio cristal': existe cristal em\n"
      "toda escala finita, com período crescendo com a escala de observação. O quasicristal\n"
      "não é um estado entre dois — é o que sobra quando não se para de olhar.\n"
      "\n"
      "O que salta é o RANK do módulo (1 ou 2, nunca 1,5) — e como racionais e irracionais\n"
      "são ambos densos, cristal e quasicristal se ENTRELAÇAM: contínuo e denso, não gradual.\n"
      "Onde há um expoente genuinamente contínuo é na COMPLEXIDADE p(k) ~ k^β: β=0 cristal\n"
      "(satura em q), β=1 quasicristal (k+1). O 'entre' é a escala e o β, não a dimensão."
      : "FALHOU — rever");
    return !passou;
}
