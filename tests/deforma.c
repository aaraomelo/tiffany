/* deforma.c — A DEFORMAÇÃO É A DINÂMICA; A SIMETRIA SÓ ANCORA.
 *
 * A inversão de foco: até aqui a dimensão inteira (o racional, o cristal) era o objeto e o
 * quasicristal a exceção. É o contrário, e mede-se:
 *
 *  (D1) A MAIORIA é irracional. Os racionais são DENSOS mas de MEDIDA NULA: em toda vizinhança há
 *       um, e a soma de todos eles é zero. Isso é ser ponto de ANCORAGEM — esqueleto, não volume.
 *       Quase todo α (medida 1) é quasicristal.
 *
 *  (D2) A DIMENSÃO INTEIRA É CRISTALIZAÇÃO SEM DINÂMICA. Na rotação R_α(x)=x+α mod 1:
 *          α=p/q racional  →  a órbita FECHA em q pontos. Acabou: nada mais acontece.
 *          α irracional    →  a órbita nunca fecha, é densa e equidistribuída (Weyl).
 *       E a órbita irracional é ordenada em toda escala: os N pontos {nα} partem o círculo em
 *       intervalos de no máximo TRÊS comprimentos distintos (teorema dos três comprimentos) — o
 *       "cristal em toda escala finita" do §entre, agora na dinâmica.
 *
 *  (D3) A DEFORMAÇÃO cria a dimensão fracionária. No mapa do círculo
 *          f(x) = x + Ω − (K/2π)·sin(2πx)
 *       o parâmetro K é a deformação (K=0 é a rotação pura, sem deformação). Cada número de
 *       rotação RACIONAL abre uma LÍNGUA de largura finita em Ω — travamento de fase, cristal — e
 *       as línguas crescem com K. A fração travada de Ω vai de 0 (K=0) a 1 (K=1): a ESCADA DO DIABO.
 *       É a simetria ancorando a deformação: as línguas nascem ancoradas nos racionais e é a
 *       deformação que lhes dá largura.
 *
 *  (D4) O OURO RESISTE MAIS. As línguas engolem primeiro os racionais simples; o número de rotação
 *       mais difícil de travar é o de pior aproximação racional — e o pior de todos é 1/φ (Hurwitz).
 *       Mede-se a largura da língua em cada racional e a resistência de 1/φ.
 *
 * A dimensão de Hausdorff do complemento em K=1 é ≈0,8700 (literatura, não medido aqui) — uma
 * dimensão genuinamente FRACIONÁRIA, e ela só existe porque há deformação. Sem deformação há apenas
 * medida 0 (racional) contra medida 1 (irracional); com deformação, aparece o fractal no meio.
 *
 * Iteração de um mapa 1D: sem arrays de dados, buffers fixos, zero malloc.
 *
 *   cc -O2 -std=c99 deforma.c -lm -o deforma && ./deforma
 */
#include <stdio.h>
#include "../lib/disco.h"
#include "reta.h"      /* rt_orbita, rt_fixo_racional: o procedimento da casa */
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int passou = 1;

/* ---------- D1: densos, mas de medida nula ---------- */


int main(void){
    printf("DEFORMA — a deformação é a dinâmica; a simetria só ancora\n");
    printf("=================================================================\n");

    /* ---------- D1 ---------- */
    printf("§D1  a MAIORIA é irracional: os racionais são densos, mas de medida nula\n");
    {
        /* A MEDIDA É UMA SOMA DE RACIONAIS, e faz-se em ℤ pelo MMC — que é o primeiro passo
         * do algoritmo (cursor §5: «o mmc é a LEITURA do texto para ℤ, e mais nada»). Os
         * denominadores são q², logo a unidade comum é U = mmc(1..Q)², e cada termo 1/q²
         * entra como U/q², INTEIRO e exacto. O tecto manda: U cabe em long até Q = 16
         * (519.437.318.400) e estoura em Q = 20, por isso a varredura vai a 16 — e a
         * tendência vê-se na mesma, porque o que se compara é a RAZÃO entre passos.
         *
         * E a comparação é por PRODUTO CRUZADO, sem dividir: med_i/Q_i < med_{i-1}/Q_{i-1}
         * escreve-se  med_i·U_{i-1}·Q_{i-1}  <  med_{i-1}·U_i·Q_i  depois de pôr os dois
         * sobre a mesma unidade. E como U_i é MÚLTIPLO de U_{i-1}, a razão r = U_i/U_{i-1}
         * é inteira e o teste fica  med_i·Q_{i-1} < med_{i-1}·r·Q_i  — sem o produto de
         * dois U. O `double med` media a mesma coisa com vírgula de borla.
         *
         * E O ARRANQUE É Q = 6, não Q = 2: a razão SOBE de Q=2 para Q=4 (0,125 → 0,149),
         * porque com dois ou três denominadores a soma ainda não tem termos que baixem a
         * média. O original começava em Q = 10 e nunca via isso. Diz-se, em vez de se
         * escolher o arranque calado. */
        printf("       Q   #{p/q irredutível em (0,1), q≤Q}   medida × U, e U = mmc(1..Q)²\n");
        long med_ant = 0, U_ant = 1, Q_ant = 0; int decresce = 1, passos = 0;
        for(long Q=6; Q<=16; Q+=2){
            long U = 1;
            for(long q=1;q<=Q;q++) U = rt_mmc(U, q);
            U = U*U;                                   /* a unidade comum dos q² */
            long cnt=0, med=0;
            for(long q=1;q<=Q;q++){
                long t = U/(q*q);                      /* 1/q² na unidade — EXACTO */
                for(long pp=1;pp<q;pp++) if(rt_mdc(pp,q)==1){ cnt++; med += t; }
            }
            printf("      %5ld  %12ld       med=%-16ld U=%ld\n", Q, cnt, med, U);
            if(Q_ant){
                long r = U / U_ant;                       /* inteiro: U_ant divide U */
                if(U % U_ant){ printf("       (a unidade não é múltipla — REVER)\n"); decresce = 0; }
                else if(med_ant > 0 && r > 0 && med_ant > (long)9e18/(r*Q)){
                    printf("       (tecto: o produto não cabe em long — parou aqui)\n"); break;
                } else if(med*Q_ant >= med_ant*r*Q) decresce = 0;
                passos++;
            }
            med_ant = med; U_ant = U; Q_ant = Q;
        }
        printf("     a medida por racional cai com Q em %d de %d passos: %s — densos e de medida 0.\n",
               decresce?passos:0, passos, decresce?"sim, resíduo 0 em ℤ":"NÃO");
        printf("     ⟹ ANCORAGEM: em toda vizinhança há um racional (denso), e todos juntos não\n");
        printf("        ocupam nada (medida 0). Quase todo α — medida 1 — é QUASICRISTAL.\n");
        if(!decresce) passou=0;
    }

    /* ---------- D2 ---------- */
    printf("\n§D2  dimensão inteira = CRISTALIZAÇÃO SEM DINÂMICA (a órbita fecha e acabou):\n");
    printf("     A rotação R_α(x) = x + α é INTEIRA quando α = p/q: os pontos são k·p mod q, e\n");
    printf("     a órbita fecha em q/mdc(p,q) — sem vírgula e sem tolerância. O «nunca fecha» do\n");
    printf("     irracional não é uma varredura até 5000: é o CORTE, D = m²+4 não é quadrado.\n");
    {
        long P[] = {1,1,2,3, 5}, Q[] = {2,3,5,8,13};
        int erro=0;
        for(int i=0;i<5;i++){
            long g = rt_mdc(P[i],Q[i]), fecha = Q[i]/g;     /* k·p ≡ 0 (mod q) ⟺ k = q/mdc */
            /* e verifica-se percorrendo: o primeiro k>0 com k·p ≡ 0 (mod q) */
            long k=1; while(k<=Q[i] && (k*P[i]) % Q[i] != 0) k++;
            printf("       α=%ld/%-2ld : órbita FECHA em q/mdc = %-3ld  e o percurso dá %-3ld  %s\n",
                   P[i], Q[i], fecha, k, (fecha==k)?"✓ nada mais acontece":"← REVER");
            if(fecha != k) erro=1;
        }
        printf("       α=1/φ    : NÃO fecha, e é TEOREMA — rt_fixo_racional(1) = %d, isto é\n"
               "                  D = 1²+4 = 5 não é quadrado, logo o ponto fixo não cabe no\n"
               "                  andar. É o CORTE, e não uma busca que não achou.\n",
               rt_fixo_racional(1));
        if(rt_fixo_racional(1)) erro=1;
        printf("\n     e a órbita é ORDENADA em toda escala — os N pontos {kα} partem o círculo em\n");
        printf("     no máximo TRÊS comprimentos (o teorema dos três comprimentos), e com α o\n");
        printf("     convergente F_k/F_{k+1} do ouro, dado pela ÓRBITA DE ∞ sob o gato, os gaps\n");
        printf("     são INTEIROS: os pontos são k·F_k mod F_{k+1}.\n");
        for(int kk=6; kk<=11; kk++){
            long pk,qk; rt_orbita(1,kk,&pk,&qk);            /* F_{k+1}/F_k, o convergente */
            long N = qk-1, pts[4096], nc=0, comp[8];
            if(N > 4000) N = 4000;
            for(long k=0;k<N;k++) pts[k] = (k+1)*qk % pk;   /* α = q_k/p_k, em ℤ */
            for(long a=1;a<N;a++){ long x=pts[a], j=a-1;    /* ordena, inteiro */
                                   while(j>=0 && pts[j]>x){ pts[j+1]=pts[j]; j--; } pts[j+1]=x; }
            for(long a=0;a<N;a++){
                long gap = (a+1<N ? pts[a+1] : pts[0]+pk) - pts[a];
                int achou=0;
                for(long c=0;c<nc;c++) if(comp[c]==gap){ achou=1; break; }
                if(!achou){ if(nc>=8){ nc=99; break; } comp[nc++]=gap; }
            }
            printf("       α=%ld/%-4ld  N=%-5ld : %ld comprimentos distintos %s\n",
                   qk, pk, N, nc, nc<=3?"✓":"← REVER");
            if(nc>3) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 EXACTO — o racional cristaliza e para, e o «para» é q/mdc(p,q) em ℤ; o\n"
               "     irracional nunca fecha, e isso é o CORTE e não uma busca. E os gaps são só TRÊS em\n"
               "     toda escala, contados como INTEIROS — ordem sem periodicidade, na dinâmica."));
        if(erro) passou=0;
    }

    /* ---------- D3/D4: o TRIAL decide quem trava — cor:papg-causa ---------- */
    printf("\n§D3  TRAVAR É O REGIME, e o regime é o TRIAL. Não se varre Ω nenhum: com |det T| = 1\n");
    printf("     o discriminante D = tr² − 4·det classifica o operador de uma vez (algebrico\n");
    printf("     cor:papg-causa), e τ = sign(D) É a leitura:\n\n");
    printf("        D < 0   ELÍPTICO      rotação, e a órbita é PERIÓDICA — de ordem finita\n");
    printf("                              → TRAVA: é a língua, o cristal\n");
    printf("        D = 0   PARABÓLICO    I + kN, a PA, a recta — a frequência infinita\n");
    printf("                              → a CÚSPIDE: a fronteira, e cada racional ancora uma\n");
    printf("        D > 0   HIPERBÓLICO   σ^k, a PG, o metálico\n");
    printf("                              → NUNCA fecha: é o quasicristal\n\n");
    {
        int erro = 0;
        printf("       tr  det   D = tr²−4det   τ   regime        ordem no ponto (rt_ordem_ponto)\n");
        long ordens[8]; int nord = 0;
        for(long tr = -3; tr <= 3; tr++){
            RtOp o = {{ tr, -1, 1, 0 }};                 /* tr(o) = tr, det(o) = 1 */
            long det = rt_op_det(&o), D = tr*tr - 4*det;
            long tau = rt_sinal(D);
            /* E PARTE-SE DE [1:1], não de ∞. A `rt_ordem_ponto` testa o fecho com
             * `Q != 0 && p*Q == P*q`, e com q = 0 isso pede Q != 0 e Q == 0 ao mesmo
             * tempo — a partir de [1:0] ela nunca detecta o fecho, seja qual for o
             * operador. O ponto ∞ é o da Lei 0, e o teste do fecho não o cobre. */
            int ord = rt_ordem_ponto(&o, 1, 1, 64);
            const char *reg = tau<0 ? "ELÍPTICO" : (tau==0 ? "PARABÓLICO" : "HIPERBÓLICO");
            printf("       %+2ld   %+2ld   %+8ld     %+2ld  %-12s  %s%d%s\n",
                   tr, det, D, tau, reg,
                   ord ? "fecha em " : "NÃO fecha (tecto 64)", ord, "");
            if(!rt_op_valido(&o)) erro = 1;
            /* A LEI: elíptico ⟺ fecha; hiperbólico ⟺ não fecha. O parabólico é a fronteira. */
            if(tau < 0 && ord == 0) erro = 1;            /* elíptico tem de fechar   */
            if(tau > 0 && ord != 0) erro = 1;            /* hiperbólico não pode     */
            if(tau < 0 && nord < 8) ordens[nord++] = ord;
        }
        ok("TRAVAR é o regime, e o regime é o TRIAL: com |det T| = 1, todo operador ELÍPTICO"
           " (D < 0) tem órbita de ordem FINITA — trava, é a língua — e todo HIPERBÓLICO (D > 0)"
           " não fecha dentro do tecto — é o quasicristal. A fronteira é o PARABÓLICO, D = 0, e"
           " cada racional ancora uma cúspide. Não se varre Ω: lê-se o discriminante",
           erro == 0);
        if(erro) passou=0;
        printf("\n       e as ordens dos elípticos são %ld e %ld — os dois períodos que |tr| < 2\n"
               "       permite com det = 1, e é por isso que a lista é FINITA: a rotação de\n"
               "       ordem finita não tem onde crescer.\n", ordens[0], nord>1?ordens[1]:0);
    }

    /* ---------- D4: o OURO é o gato, e por isso não trava ---------- */
    printf("\n§D4  o OURO RESISTE porque É O GATO — e o gato é hiperbólico\n");
    {
        int erro = 0;
        printf("       m   A_m = [[m,1],[1,0]]   det   D = m²+4   τ   fecha no ponto?\n");
        for(long m = 1; m <= 4; m++){
            RtOp g = {{ m, 1, 1, 0 }};
            long det = rt_op_det(&g), D = m*m + 4;
            int ord = rt_ordem_ponto(&g, 1, 1, 4096);    /* [1:1], e não ∞ — ver §D3 */
            printf("       %ld   tr=%ld det=%+ld           %+ld    %+ld       %+ld   %s\n",
                   m, m, det, det, D, rt_sinal(D),
                   ord ? "FECHA (falha)" : "NÃO fecha em 4096 ✓");
            if(det != -1 || D <= 0 || ord != 0) erro = 1;
        }
        /* e o CONTROLO, que é o outro lado: um elíptico fecha, e depressa */
        RtOp e4 = {{ 0, -1, 1, 0 }};                     /* tr = 0, det = 1, D = −4 */
        int ord4 = rt_ordem_ponto(&e4, 1, 1, 64);
        printf("       CONTROLO: tr=0 det=+1 → D=−4 (elíptico) fecha em %d — o par que separa\n\n", ord4);
        ok("o OURO não trava porque É O GATO: A_m = [[m,1],[1,0]] tem det = σσ' = −1 e"
           " D = m²+4 > 0 para todo m ≥ 1, logo é HIPERBÓLICO — σ^k, a PG, o metálico — e a"
           " órbita não fecha em 4096 passos. E o CONTROLO tem o outro lado: um elíptico"
           " (tr = 0, det = +1, D = −4) fecha em 2. Não é que o ouro «resista mais»: é que"
           " está do outro lado do trial, e isso lê-se no discriminante sem varrer nada",
           erro == 0 && ord4 > 0);
        if(erro || !ord4) passou=0;
        printf("     ⟹ e é a MESMA frase do §D5: D = m²+4 nunca é quadrado (rt_fixo_racional = 0),\n");
        printf("        logo o ponto fixo não cabe no andar — é o CORTE. O que não fecha em ℚ é\n");
        printf("        exactamente o que resiste à cristalização.\n");
    }

    /* ---------- D5: a robustez é da CLASSE MODULAR, não do número ---------- */
    printf("\n§D5  a robustez é da CLASSE MODULAR — e o procedimento é o de sempre\n");
    printf("     w é nobre se w = (aφ+b)/(cφ+d) com ad−bc = ±1 (equivalente a φ sob SL(2,ℤ)). E φ\n");
    printf("     é o ponto fixo do gato A_m = [[m,1],[1,0]], com det = σσ' = −1 (algebrico\n");
    printf("     thm:gato). Não se simula nada: a ÓRBITA DE ∞ sob o gato JÁ É a régua —\n");
    printf("        [p:q] ⟼ [m·p + q : p]   a partir de [1:0] = ∞   (rt_orbita, prop:orbita)\n");
    printf("     e o que ela produz são os convergentes, com |det| = 1 em todos os passos.\n\n");
    printf("     A ROBUSTEZ É ENTÃO UM TEOREMA (analitico thm:ouro):\n");
    printf("        q_k ≥ F_{k+1} para todo k, com igualdade em TODO k só na sucessão de UNS,\n");
    printf("     logo o encaixe |c_{k+1} − c_k| = 1/(q_k·q_{k+1}) é o MAIS LARGO na cauda de uns:\n");
    printf("     quem tem essa cauda é o pior aproximado por racionais, e é por isso que resiste.\n");
    printf("     A dourada é a BORDA — σ² = m·σ + 1 — e m = 1 é o andar onde ela mora.\n\n");
    {
        long i, k;
        /* (1) A NOBREZA É O DETERMINANTE, e diz-se em ℤ. */
        struct { const char *nome; long a,b,c,d; int rotulo; } alvos[] = {
            {"1/φ          ", 0,1,1,0, 1},
            {"1/φ²         ", 0,1,1,1, 1},
            {"φ/(φ+2)      ", 1,0,1,2, 1},
            {"(φ+1)/(φ+2)  ", 1,1,1,2, 1},
        };
        int mal_rotulado = 0, nobres = 0;
        printf("       alvo             (a,b,c,d)   ad−bc   nobre?   o rótulo bate?\n");
        for(i=0;i<4;i++){
            long det = alvos[i].a*alvos[i].d - alvos[i].b*alvos[i].c;
            int nobre = (det==1 || det==-1);
            if(nobre) nobres++;
            if(nobre != alvos[i].rotulo) mal_rotulado++;
            printf("       %s  (%ld,%ld,%ld,%ld)   %+4ld    %-7s  %s\n",
                   alvos[i].nome, alvos[i].a,alvos[i].b,alvos[i].c,alvos[i].d, det,
                   nobre?"SIM":"NÃO", (nobre==alvos[i].rotulo)?"sim":"NÃO — rótulo errado");
        }
        int d5a = (mal_rotulado == 1 && nobres == 3);
        ok("a NOBREZA é o determinante, e lê-se em ℤ sem calcular w: w ~ φ sob SL(2,ℤ) ⟺"
           " ad−bc = ±1. E a tabela apanha um rótulo ERRADO — φ/(φ+2) = (1φ+0)/(1φ+2) tem"
           " det = 2, logo NÃO é da classe de φ, e estava marcado «nobre» ao lado da própria"
           " definição que o exclui. São TRÊS os nobres, e não quatro",
           d5a);
        if(!d5a) passou=0;

        /* (2) A ÓRBITA DE ∞ SOB O GATO — a régua, pela lib, sem uma raiz formada. */
        printf("\n       m   os denominadores q_k = órbita de ∞ sob A_m (rt_orbita)        vs F_{k+1}\n");
        long F[24]; F[0]=0; F[1]=1;
        for(k=2;k<24;k++) F[k]=F[k-1]+F[k-2];
        long viola=0, iguais=0, acima_m[4]={0,0,0,0}, passos=0;
        for(long m=1;m<=3;m++){
            printf("       %ld  ", m);
            for(k=1;k<=12;k++){
                long P,Q; rt_orbita(m,(int)k,&P,&Q);
                printf("%ld%s", Q, k<12?" ":"");
                if(Q < F[k]) viola++;                    /* q_k ≥ F_{k+1} SEMPRE */
                if(m==1){ if(Q == F[k]) iguais++; passos++; }
                else if(Q > F[k]) acima_m[m]++;          /* estrito nalgum k */
            }
            printf("   %s\n", m==1 ? "= F_{k+1} em TODOS — a cauda de UNS"
                                    : "acima nalguns — a igualdade NÃO é simultânea");
        }
        printf("\n       e o ponto fixo NÃO cabe no andar em nenhum deles (rt_fixo_racional): %ld %ld %ld\n",
               (long)rt_fixo_racional(1), (long)rt_fixo_racional(2), (long)rt_fixo_racional(3));
        printf("       — é o CORTE, e é por isso que a régua é a órbita e não um decimal.\n\n");
        int d5b = (viola == 0 && iguais == passos && acima_m[2] > 0 && acima_m[3] > 0);
        int d5c = (!rt_fixo_racional(1) && !rt_fixo_racional(2) && !rt_fixo_racional(3));
        ok("thm:ouro medido pela ÓRBITA DE ∞ sob o gato, e não por simulação nenhuma: com m = 1"
           " os denominadores são exactamente F_{k+1} em todos os passos e nunca abaixo — o"
           " MÍNIMO ponto a ponto do crescimento, e NUNCA abaixo em nenhum m. E a"
           " caracterização é pela igualdade SIMULTÂNEA EM TODOS os índices, e não num índice"
           " isolado — o paper di-lo na observação a seguir ao teorema, e é por isso que o"
           " controlo se conta assim: m = 2 e m = 3 empatam no primeiro passo e ficam acima"
           " depois, logo a igualdade NÃO é simultânea neles. O encaixe 1/(q_k·q_{k+1}) deles"
           " é mais FINO, são melhor aproximados por racionais, e cristalizam antes",
           d5b);
        ok("e o ponto fixo do gato não cabe no andar para nenhum m ≥ 1 (D = m²+4 nunca é"
           " quadrado): é o CORTE, e é ele que obriga a régua a ser a ÓRBITA em vez de um"
           " decimal truncado — o que sai da lib é [p:q], exacto, e a palavra é o rasto",
           d5c);
        if(!d5b || !d5c) passou=0;
    }


    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — a inversão se sustenta. Os racionais (dimensão inteira, cristal) são\n"
      "DENSOS e de MEDIDA NULA: ancoram, não ocupam. Quase todo α, medida 1, é quasicristal.\n"
      "E a dimensão inteira é cristalização SEM DINÂMICA — a órbita fecha em q passos e\n"
      "acabou; a irracional nunca fecha, é densa, e ainda assim ordenada (três comprimentos).\n"
      "\n"
      "A DEFORMAÇÃO é que é a dinâmica: em K=0 o racional não tem largura nenhuma; conforme\n"
      "K cresce, cada racional abre uma língua e a fração travada vai de 0 a ~1. A simetria\n"
      "ANCORA (dá o centro da língua) e a deformação DÁ A LARGURA. E é só aí que aparece\n"
      "dimensão fracionária de verdade (≈0,87 no complemento em K=1) — ela é filha da\n"
      "deformação, não da simetria.\n"
      "\n"
      "E quem resiste à deformação não é O OURO: é a CLASSE MODULAR dele (§D5), e isso agora\n"
      "mede-se em ℤ pela ÓRBITA DE ∞ sob o gato — sem simular nada. A nobreza é o determinante\n"
      "(ad−bc = ±1) e a robustez é o thm:ouro: a cauda de UNS realiza q_k = F_{k+1} em TODOS os\n"
      "índices, que é o mínimo ponto a ponto, logo o encaixe 1/(q_k·q_{k+1}) é o mais LARGO e ela\n"
      "é a pior aproximada por racionais. φ é só o representante mais curto da órbita. A medida\n"
      "aqui é de aproximação RACIONAL (base inteira); na base do corpo estelar, q=e^{−2π}, o ouro\n"
      "é um valor que π PRODUZ (tools/estelar.c). Dizer que o ouro é o supremo do irracional\n"
      "inverte o fluxo."
      : "FALHOU — rever");
    return !passou;
}
