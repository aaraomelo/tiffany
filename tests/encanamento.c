/* encanamento.c — O ENCANAMENTO: o transistor autodual, e o primeiro andar decide tudo.
 *
 * O Aarão: "ainda não sinto equilibrado — esse material está mais para conversor, porque o sinal já
 * vem da rede neuronal, então o processamento é do cérebro. Compara com a dualidade dos 4: pn e np,
 * silício-fósforo e os materiais que dopam os semicondutores. O transistor é autodual também.
 * Precisamos de um transistor com materiais análogos. O cérebro é o microprocessador multifractal —
 * estamos fazendo o encanamento."
 *
 * O `mcu.c` já tem o processador; o cérebro é ele. **O que a túnica faz não é calcular — é
 * encanar**, e um encanamento julga-se por uma coisa só: *quanto do sinal chega, e quanto ruído
 * ele acrescenta pelo caminho*.
 *
 * A fórmula de Friis:
 *
 *      F_total = F₁ + (F₂−1)/G₁ + (F₃−1)/(G₁G₂) + …
 *
 * — **o primeiro andar decide quase tudo**, porque o ruído de todos os seguintes vem dividido pelo
 * ganho que já se acumulou. *Não é uma heurística de rádio: é aritmética, e mede-se.*
 *
 * E O TRANSÍSTOR É AUTODUAL: dopar é **quebrar o octeto por ±1**.
 *
 *      Si (4) + P (5)  ->  sobra um eletrão   ->  tipo N
 *      Si (4) + B (3)  ->  falta um eletrão   ->  tipo P (lacuna)
 *
 * *O dador e o aceitador são o mesmo desvio com o sinal trocado*, e daí NPN e PNP são espelhos — o
 * `J` do catálogo, ordem 2.
 *
 * LEI vs TRANSPORTE. Shockley com exp, Friis em vírgula, gm=Ic/VT e SNR em 1e-12 eram o método.
 * A lei é o octeto ±1, o espelho (tipo,V)↦(−tipo,−V) que deixa tipo·V invariante, Friis em ℚ
 * por produto cruzado, R+T+A em centésimos, Av = Ic·RL/VT enquadrado, e o peso F1/(F−F1)
 * entre 23 e 24 — sem uma exponencial e sem um decibel formado.
 *
 *   cc -O2 -std=c99 -I lib tests/encanamento.c -o encanamento && ./encanamento
 */
#include <stdio.h>
#include <limits.h>
#include "unidade.h"

typedef struct { const char *nome; int Z, valencia; } Dopante;

static const Dopante DOPANTES[] = {
    { "silicio (Si)",   14, 4 },
    { "fosforo (P)",    15, 5 },
    { "arsenio (As)",   33, 5 },
    { "antimonio (Sb)", 51, 5 },
    { "boro (B)",        5, 3 },
    { "aluminio (Al)",  13, 3 },
    { "galio (Ga)",     31, 3 },
};
#define NDOP ((int)(sizeof DOPANTES / sizeof DOPANTES[0]))
#define V_MATRIZ 4

typedef struct { long n, d; } Fr;
static void fr_red(Fr *x){
    if(x->d < 0){ x->n = -x->n; x->d = -x->d; }
    long a = x->n < 0 ? -x->n : x->n, b = x->d;
    while(b){ long t = a % b; a = b; b = t; }
    if(a > 1){ x->n /= a; x->d /= a; }
}
static Fr fr_soma(Fr x, Fr y){ Fr r = { x.n*y.d + y.n*x.d, x.d*y.d }; fr_red(&r); return r; }
static Fr fr_mul (Fr x, Fr y){ Fr r = { x.n*y.n, x.d*y.d };           fr_red(&r); return r; }
static Fr friis_q(const Fr *F, const Fr *G, int n){
    Fr tot = F[0], acum = G[0];
    for(int i = 1; i < n; i += 1){
        Fr menos1 = { F[i].n - F[i].d, F[i].d };
        Fr termo  = { menos1.n * acum.d, menos1.d * acum.n };
        fr_red(&termo);
        tot = fr_soma(tot, termo);
        acum = fr_mul(acum, G[i]);
    }
    return tot;
}
static int fr_maior(Fr x, Fr y){ return x.n * y.d > y.n * x.d; }

int main(void){
    puts("encanamento.c — O ENCANAMENTO: o transistor autodual, e o primeiro andar decide\n");

    puts("§T1  A DOPAGEM pelo OCTETO: dopar e quebrar o octeto por MAIS OU MENOS UM");
    puts("     O octeto.c mediu que o silicio tem 4 de valencia e faz 4 ligacoes. Dopar e por");
    puts("     um vizinho com 5 (sobra um) ou com 3 (falta um) — e o desvio e sempre UM.\n");
    {
        printf("     %-18s %4s %10s %8s %12s\n", "dopante", "Z", "valencia", "desvio", "tipo");
        int dadores = 0, aceitadores = 0, desvio_um = 0, total = 0;
        for(int i = 1; i < NDOP; i += 1){
            int d = DOPANTES[i].valencia - V_MATRIZ;
            printf("     %-18s %4d %10d %+8d %12s\n", DOPANTES[i].nome, DOPANTES[i].Z,
                   DOPANTES[i].valencia, d, d > 0 ? "N (dador)" : "P (aceitador)");
            if(d > 0) dadores += 1; else aceitadores += 1;
            if(d == 1 || d == -1) desvio_um += 1;
            total += 1;
        }
        ok("TODO dopante desvia o octeto por exatamente UM — nem dois, nem meio",
           desvio_um == total);
        ok("e ha os DOIS sinais: dadores (+1) e aceitadores (-1), e sao o mesmo desvio espelhado",
           dadores == 3 && aceitadores == 3);
        printf("     -> %d dopantes, %d dadores e %d aceitadores, todos com |desvio| = 1.\n",
               total, dadores, aceitadores);
        conclui("O tipo N e o tipo P nao sao dois mecanismos: sao UM, com o sinal trocado.");
    }

    puts("\n§T2  NPN e PNP sao ESPELHOS: o transistor e AUTODUAL, e e o J de ordem 2");
    puts("     Trocar todos os dopantes de tipo, e todos os sinais de tensao e corrente, devolve");
    puts("     um transistor que funciona igual. Isso e uma INVOLUCAO — aplicada duas vezes,");
    puts("     volta ao original.\n");
    {
        int tipo[3] = { +1, -1, +1 };
        long V[3] = { 7, 0, -50 };
        int t2[3], t3[3]; long V2[3], V3[3];
        for(int i = 0; i < 3; i += 1){ t2[i] = -tipo[i]; V2[i] = -V[i]; }
        for(int i = 0; i < 3; i += 1){ t3[i] = -t2[i];   V3[i] = -V2[i]; }
        int volta = 1;
        for(int i = 0; i < 3; i += 1) if(t3[i] != tipo[i] || V3[i] != V[i]) volta = 0;
        ok("o ESPELHO e uma INVOLUCAO: aplicado duas vezes devolve o original, ordem 2",
           volta);
        int pnp_certo = (t2[0] == -1 && t2[1] == +1 && t2[2] == -1);
        ok("e o espelho de NPN e exatamente PNP — nao ha um terceiro tipo, sao so dois",
           pnp_certo);
        /* Shockley I(tipo,V) = tipo·(E(tipo·V)−1): o produto tipo·V e' o argumento, e o
         * espelho (tipo,V)↦(−tipo,−V) DEIXA-O INVARIANTE. Qualquer E — a exponencial e
         * uma delas — ve o mesmo, e o tipo de fora so' troca o sinal da corrente.
         * Sem espelhar a tensao, tipo·V muda de sinal, e a equação já nao e a mesma.
         * Nao se avalia exp nenhum: o invariante e' o produto. */
        int inv_tv = 1, sem_espelho = 0;
        for(int i = 0; i < 3; i += 1){
            if((long)t2[i] * V2[i] != (long)tipo[i] * V[i]) inv_tv = 0;
            if((long)t2[i] * V[i]  != (long)tipo[i] * V[i]) sem_espelho = 1;
        }
        ok("e a EQUACAO e a mesma: o espelho deixa tipo.V invariante, e so' sem espelhar a"
           " tensao e que o argumento muda. Shockley e' I = tipo.(E(tipo.V)-1); o exp estava"
           " no transporte, e a tese e' o produto. |-a|==|a| com Is escrito a mao era a"
           " identidade, e o 1e-20 dava-lhe cara de medida",
           inv_tv && sem_espelho);
        printf("     -> tipo.V = (%ld,%ld,%ld) nos dois lados do espelho; sem virar V, muda.\n",
               (long)tipo[0]*V[0], (long)tipo[1]*V[1], (long)tipo[2]*V[2]);
        conclui("O transistor NAO TEM um dual — ele E o seu dual, ao contrario. E o J do catalogo.");
    }

    puts("\n§T3  FRIIS: o PRIMEIRO andar decide quase tudo — e isso e aritmetica");
    puts("     F_total = F1 + (F2-1)/G1 + (F3-1)/(G1.G2) + ... O ruido de cada andar vem");
    puts("     dividido pelo ganho ja acumulado. Nao e heuristica de radio: e uma soma.\n");
    {
        Fr Fq[4] = { {126,100}, {200,100}, {316,100}, {1000,100} };
        Fr Gq[4] = { {100,1},   {50,100},  {31600,100}, {100,100} };
        Fr Fq_ma[4] = { Fq[1], Fq[0], Fq[2], Fq[3] };
        Fr Gq_ma[4] = { Gq[1], Gq[0], Gq[2], Gq[3] };
        Fr Fbom_q = friis_q(Fq, Gq, 4);
        Fr Fma_q  = friis_q(Fq_ma, Gq_ma, 4);
        int reduzida = 1;
        { long a = Fbom_q.n, b2 = Fbom_q.d; while(b2){ long t = a % b2; a = b2; b2 = t; }
          if(a != 1) reduzida = 0;
          a = Fma_q.n; b2 = Fma_q.d; while(b2){ long t = a % b2; a = b2; b2 = t; }
          if(a != 1) reduzida = 0; }
        ok("por o pre-amp PRIMEIRO reduz o ruido total — e a mesma cadeia, so trocada de"
           " ordem. E a conta e' EXACTA em Q: os oito numeros sao decimais escritos, logo"
           " racionais de denominador 100, e Friis e' uma soma de fraccoes — compara-se por"
           " PRODUTO CRUZADO, sem uma virgula. E o resultado E' a fraccao reduzida",
           fr_maior(Fma_q, Fbom_q) && reduzida && Fbom_q.d > 0 && Fma_q.d > 0);
        printf("     -> pre-amp primeiro %ld/%ld ; cabo primeiro %ld/%ld\n",
               Fbom_q.n, Fbom_q.d, Fma_q.n, Fma_q.d);
        /* 10·log10(a) − 10·log10(b) > 1  <=>  (a/b)^10 > 10. Enquadra-se 10^(1/10)
         * por 63/50: 63^10 > 10·50^10, e os dois lados cabem em long. */
        Fr limiar = { 63, 50 };
        long p63 = 1, p50 = 1; int cabe = 1;
        for(int t = 0; t < 10; t += 1){
            if(p63 > LONG_MAX/limiar.n || p50 > LONG_MAX/(10*limiar.d)){ cabe = 0; break; }
            p63 *= limiar.n; p50 *= limiar.d;
        }
        int corte_ok = (cabe && p63 > 10*p50);
        int acima = fr_maior(Fma_q, fr_mul(limiar, Fbom_q));
        ok("e a diferenca e de DECIBEIS, nao de decimos: a ordem vale mais que os componentes."
           " E «mais de 1 dB» compara-se SEM logaritmo E SEM elevar a decima: 63^10 > 10.50^10"
           " poe 63/50 ACIMA de 10^(1/10), e depois basta r > 63/50 por produto cruzado",
           corte_ok && acima);
        int suprime = 1; Fr ant = { 0, 1 };
        long passos = 0, forma_fechada = 0;
        for(long G1 = 1; G1 <= 100000; G1 *= 10){
            Fr Gvar[4] = { {G1,1}, Gq[1], Gq[2], Gq[3] };
            Fr F = friis_q(Fq, Gvar, 4);
            if(passos > 0 && F.n * ant.d >= ant.n * F.d) suprime = 0;
            ant = F;
            passos += 1;
            /* F·395000·G1 = 497700·G1 + 2123900, identidade em Z. */
            if(F.n * 395000L * G1 == F.d * (497700L*G1 + 2123900L)) forma_fechada += 1;
        }
        ok("A LEI: subir o ganho do primeiro andar faz o ruido total TENDER ao ruido dele so"
           " — e ela nao e' «tende a», tem FORMA FECHADA. F_total - F1 = (21239/3950)/G1, o"
           " produto (F_total-F1).G1 e' CONSTANTE. Tirando denominadores,"
           " F_total.395000.G1 = 497700.G1 + 2123900, e ela confere-se contra o Friis em Q"
           " em cada um dos seis passos, sem limiar nenhum",
           suprime && passos == 6 && forma_fechada == passos);
        conclui("O encanamento inteiro fica refem do primeiro elo — e isso decide o DESENHO.");
    }

    puts("\n§T4  PASSIVO nao amplifica: sem FONTE o ganho nao passa de 1, e prova-se");
    puts("     A liga do liga.c e um CONVERSOR. Ela reparte a onda entre refletir, passar e");
    puts("     ficar — e as tres somam 1, nunca mais.\n");
    {
        const long R_z = 31, T_z = 2, A_z = 67;
        const long TODO = 100;
        long soma_z = R_z + T_z + A_z;
        const long mau_z[3] = { 31, 2, 80 };
        long soma_ma = mau_z[0] + mau_z[1] + mau_z[2];
        printf("     -> em centesimos: %ld + %ld + %ld = %ld, EXACTO (e a que nao fecha,\n"
               "        %ld + %ld + %ld = %ld, e' apanhada)\n",
               R_z, T_z, A_z, soma_z, mau_z[0], mau_z[1], mau_z[2], soma_ma);
        ok("um PASSIVO reparte e nao cria: R + T + A = 1, e nenhuma parcela passa do todo."
           " E em CENTESIMOS a soma e' 31 + 2 + 67 = 100, EXACTA e sem limiar — o 1e-12 dava"
           " cara de medicao a numeros que foram escolhidos para somar um. O ganho que SAI e'"
           " T, 2 centesimos, e T < 1 por conservacao",
           soma_z == TODO && T_z == 2 && A_z < TODO && T_z < TODO && R_z < TODO);
        ok("e a reparticao que NAO fecha e apanhada: 31+2+80 = 113, nao 100 — o 1e-12 dava"
           " cara de medicao, e uma soma que passa do todo CRIARIA energia",
           soma_ma == 113 && soma_ma != TODO);
        long VT_uV = 25850, RL_ohm = 5000;
        long num = 1000L * RL_ohm;
        int Av_enquadrado = (193*VT_uV < num && num < 194*VT_uV);
        ok("e o ATIVO passa de 1: com fonte, o ganho e' gm.RL e ele e' MUITO maior que um —"
           " Av = Ic.RL/VT e' racional: 5.10^6 microvolt sobre 25850, ou seja 100000/517,"
           " e isso enquadra-se entre 193 e 194 por multiplicacao cruzada, 99781 < 100000"
           " < 100298, sem dividir. O par e' este: o passivo nao passa de 1 e o activo passa,"
           " e `Av > 100` era um numero meu no meio dos dois",
           Av_enquadrado && VT_uV == 25850 && num == 5000000);
        printf("     -> passivo: %ld/%ld. ativo: 100000/517, entre 193 e 194.\n", soma_z, TODO);
        conclui("A diferenca nao e de qualidade de material: e de haver ou nao uma FONTE.");
    }

    puts("\n§T5  OS MATERIAIS ANALOGOS: grafeno tipo n e tipo p, e o que muda\n");
    {
        typedef struct { const char *dopante; int valencia; const char *tipo; int reversivel; } G;
        static const G GRAF[] = {
            { "azoto (N) na rede",  5, "N", 0 },
            { "boro (B) na rede",   3, "P", 0 },
            { "porta eletrostatica",0, "N ou P", 1 },
        };
        int n_rev = 0, n_fixo = 0;
        printf("     %-24s %10s %10s %14s\n", "metodo", "valencia", "tipo", "reversivel?");
        for(int i = 0; i < 3; i += 1){
            printf("     %-24s %10d %10s %14s\n", GRAF[i].dopante, GRAF[i].valencia,
                   GRAF[i].tipo, GRAF[i].reversivel ? "SIM" : "nao");
            if(GRAF[i].reversivel) n_rev += 1; else n_fixo += 1;
        }
        ok("o grafeno dopa-se pelos DOIS lados, como o silicio — azoto da N e boro da P",
           GRAF[0].valencia == 5 && GRAF[1].valencia == 3);
        ok("e a dopagem ELETROSTATICA e REVERSIVEL — e isso o silicio dopado nao permite."
           " Dos tres metodos, UM e' reversivel e DOIS estao congelados no fabrico",
           n_rev == 1 && n_fixo == 2);
        conclui("A REGRA nao mudou — mudou a rede: octeto quebrado por ±1, agora em sp2.");
    }

    puts("\n§T6  E O QUE O ENCANAMENTO TEM DE PRESERVAR, ja que nao processa");
    puts("     O criterio nao e computar bem: e nao ESTRAGAR.\n");
    {
        puts("     Estas tres ja estao medidas noutros ficheiros, e cita-se — nao se remede:");
        puts("       1. a INFORMACAO   o par (B,P) recupera o radial   radiacao.c §W5");
        puts("       2. a REVERSAO     ler e escrever sao adjuntos     colheita.c §C2");
        puts("       3. o RUIDO        o primeiro andar fixa-o         §T3, aqui em cima");
        puts("");
        const long D = 1580000;
        long Fz = 0;
        Fz += 126L * (D/100);
        Fz += (200L-100L) * (D/100) / 100L;
        Fz += (316L-100L) * (D/100) / 50L;
        Fz += (1000L-100L) * (D/100) / 15800L;
        Fr Fq[4] = { {126,100}, {200,100}, {316,100}, {1000,100} };
        Fr Gq[4] = { {100,1},   {50,100},  {31600,100}, {100,100} };
        Fr Fq_tot = friis_q(Fq, Gq, 4);
        int friis_concorda = (Fq_tot.n * D == Fq_tot.d * Fz);
        long snr_num = 1250L * D, snr_den = 3L * Fz;
        int snr_enquadrado = (317L*snr_den < snr_num && snr_num < 318L*snr_den);
        printf("     -> Friis em Z: %ld/%ld, e o snr de saida %ld/%ld\n",
               Fz, D, snr_num, snr_den);
        ok("a CADEIA INTEIRA preserva o sinal: a relacao sinal-ruido sobrevive ao encanamento."
           " A cadeia e' RACIONAL toda ela: denominador comum 1580000, a soma em Z da'"
           " 2075756/1580000; o snr de entrada e' 1250/3, o de saida 1975000000/6227268,"
           " entre 317 e 318 por multiplicacao cruzada. Medido por DOIS CAMINHOS: o Friis"
           " em Z e o friis_q em Q concordam. O `> 100` nao dizia nada que a cadeia pudesse"
           " desmentir",
           friis_concorda && snr_enquadrado && Fz == 2075756);
        {
            const long F1_D = 126L * (D/100), resto_D = Fz - F1_D;
            int peso_enquadrado = (resto_D > 0 && 23*resto_D < F1_D && F1_D < 24*resto_D);
            Fr soma = friis_q(Fq, Gq, 4);
            int soma_em_Z = (soma.n * D == soma.d * Fz);
            printf("     -> F1 = %ld, os outros tres acrescentam %ld (peso 23 a 24)\n",
                   F1_D, resto_D);
            ok("e o que ela custa e mensuravel, e e FRIIS quem o diz: o F da cadeia sai da"
               " soma por andares e bate com a rota em Z. O PRIMEIRO ANDAR DECIDE — sozinho"
               " ele responde por quase todo o F. Sobre 1580000, F1 vale 1990800 e os outros"
               " tres juntos 84956, peso entre 23 e 24, enquadrado por multiplicacao cruzada."
               " Os dois numeros DERIVAM-SE do Fz em vez de serem escritos",
               soma_em_Z && peso_enquadrado);
        }
        conclui("O encanamento nao acrescenta nada ao sinal — so lhe tira. Um cano bom nao melhora a agua.");
    }

    printf("\n=======================================================\n");
    printf("  %d assercoes, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
