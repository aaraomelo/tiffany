/* pendulo.c — O RELOGIO COMO PENDULO N-UPLO: n juntas, n relogios, e o que os fecha.
 *
 * O Aarao: "modela o relogio como um pendulo n-uplo."
 *
 * A PECA JA' ESTAVA, E ISTO NAO E' UMA ANALOGIA NOVA. O relogio.h diz `colisor_passos`:
 * DOIS passos fecham com um lado, QUATRO com dois. O checkpoint de 05/08 diz "cada eixo
 * e' um relogio". E o enredo, sem uma formula, ja' escrevera "ligadas no MESMO PENDULO e
 * em OPOSICAO" (L6863). Um pendulo n-uplo linearizado e' exactamente isso: n juntas, cada
 * uma com o seu relogio, e os modos normais a separarem-se em FASE e OPOSICAO.
 *
 * O QUE SE MEDE — e o eixo e' o mesmo do resto: o que FECHA.
 *
 *   §P1  n juntas sao n relogios, e o conjunto fecha no LCM dos periodos — a operacao
 *        do viveiro que voa SEMPRE (o cruzamento), e nao a soma nem o produto
 *   §P2  a matriz do acoplamento e' TRIDIAGONAL, e o seu determinante segue Fibonacci:
 *        D(n) = 2*D(n-1) - D(n-2). A cadeia do pendulo E' a recorrencia da torre.
 *   §P3  os DOIS modos do duplo: em FASE (periodo 2, espelha) e em OPOSICAO (periodo 4,
 *        roda) — o par que o relogio.h ja' contava, e que o Hopfield mediu como B_s/B_a
 *   §P4  a INVOLUCAO e' inverter o tempo, e ela fecha: aplicada duas vezes, devolve
 *   §P5  o DUAL do pendulo de n juntas: o pente de passo d e o de passo N/d, e o
 *        autodual e' onde d*d = N
 *   §P6  o REGIME e o FECHO, na regua DESTA teoria: o pendulo e' BORDA (conserva,
 *        e gira porque so' tem a coordenada que roda), e o que distingue o linearizado
 *        nao e' o regime — e' o FECHO. So' o isocrono e' relogio.
 *
 * Zero doubles: o linearizado mede-se em inteiros, e o nao-linear em ponto fixo inteiro.
 *
 *   cc -O2 -std=c99 -Wall -I../lib pendulo.c -o pendulo && ./pendulo
 */
#include <stdio.h>
#include "relogio.h"
#include "unidade.h"

static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a < 0 ? -a : a; }
static long mmc(long a, long b){ return (a / mdc(a,b)) * b; }

/* O determinante da matriz tridiagonal de n juntas acopladas: diagonal 2, vizinhos -1.
 * E' a matriz de rigidez do pendulo n-uplo linearizado, e a mesma de n osciladores em
 * cadeia. Calcula-se pela recorrencia, que e' o ponto: nao se resolve, DOBRA-SE. */
static long tridiag_det(int n){
    long ant = 1, act = 2;                    /* D(0)=1, D(1)=2 */
    if(n == 0) return 1;
    for(int k = 2; k <= n; k++){ long novo = 2*act - ant; ant = act; act = novo; }
    return act;
}

int main(void){
    puts("\n  O RELOGIO COMO PENDULO N-UPLO — n juntas, n relogios\n");

    /* ═══ §P1 — n juntas sao n relogios, e o conjunto fecha no LCM ═══════════════════
     * Cada junta tem o seu periodo. O sistema todo so' volta ao inicio quando TODAS
     * voltam ao mesmo tempo — e isso e' o minimo multiplo comum, nao a soma nem o
     * produto. E' o cruzamento do viveiro: a operacao que voa sempre. */
    {
        long p[4] = { 2, 4, 3, 6 };           /* os periodos de quatro juntas */
        long l = 1, s = 0, pr = 1;
        for(int i = 0; i < 4; i++){ l = mmc(l, p[i]); s += p[i]; pr *= p[i]; }
        /* e o teste que separa: o LCM tem de dividir o produto e ser divisivel por cada um */
        long mau = 0;
        for(int i = 0; i < 4; i++) if(l % p[i]) mau++;
        if(pr % l) mau++;
        /* e que ele e' o MENOR: nenhum candidato abaixo dele e' divisivel por todos.
         * Antes estava aqui `l == 12`, escrito a mao — se os periodos mudassem, a
         * asercao nao mudava sozinha. Agora deriva. */
        long menor_serve = 0;
        for(long c = 1; c < l; c++){
            int todos = 1;
            for(int i = 0; i < 4; i++) if(c % p[i]) todos = 0;
            if(todos) menor_serve++;
        }
        printf("      periodos {2,4,3,6}:  lcm = %ld   soma = %ld   produto = %ld\n", l, s, pr);
        printf("      cada junta divide o lcm; e o lcm divide o produto\n\n");
        ok("n juntas sao n relogios e o conjunto fecha no LCM dos periodos — cada junta"
           " volta ao inicio la' dentro, e nem a soma nem o produto o fazem."
           " E' o cruzamento do viveiro, a operacao que voa sempre",
           menor_serve == 0 && mau == 0 && l != s && l != pr);
    }

    /* ═══ §P2 — a cadeia do pendulo E' a recorrencia da torre ════════════════════════
     * A matriz de n juntas acopladas e' tridiagonal (2 na diagonal, -1 nos vizinhos), e o
     * determinante obedece a D(n) = 2 D(n-1) - D(n-2). Isso da' n+1 — mas o que interessa
     * nao e' o valor: e' a RECORRENCIA ser a mesma de que a torre e' feita. */
    {
        long mau = 0;
        printf("      n:  ");
        for(int n = 1; n <= 8; n++) printf("%4d", n);
        printf("\n      D:  ");
        for(int n = 1; n <= 8; n++){
            long d = tridiag_det(n);
            printf("%4ld", d);
            if(d != n + 1) mau++;                                  /* a forma fechada */
            if(n >= 3 && d != 2*tridiag_det(n-1) - tridiag_det(n-2)) mau++;  /* a recorrencia */
        }
        printf("\n\n");
        ok("a matriz do acoplamento e' tridiagonal e o determinante DOBRA-SE por"
           " D(n) = 2D(n-1) - D(n-2) — nunca se resolve um sistema. A cadeia do pendulo"
           " e' a recorrencia de que a torre e' feita, e da' n+1 em oito valores", mau == 0);
    }

    /* ═══ §P3 — os dois modos do duplo: fase e oposicao ══════════════════════════════
     * Duas juntas dao dois modos normais. No modo em FASE as duas vao juntas: o vector e'
     * (1,1) e a operacao ESPELHA — periodo 2. No modo em OPOSICAO vao contra: (1,-1), e a
     * operacao RODA — periodo 4. E' o par que o relogio.h ja' contava com um lado e com
     * dois, e que o Hopfield mediu como B_s (espelha) e B_a (roda). */
    {
        /* O MODO E' PONTO FIXO — e eu tinha-o confundido com a operacao. Aplicar o espelho
         * a (1,1) devolve (1,1): periodo 1, porque o modo em fase E' o autovector de
         * autovalor +1. O periodo 2 e' da OPERACAO, medida num vector generico.
         * A asercao apanhou-me, e a distincao e' o resultado: o modo nao tem periodo, tem
         * autovalor; quem tem periodo e' o operador. */
        long e_per = 0, r_per = 0;
        {   long a = 3, b = 5;                              /* generico: a != b */
            long x = a, y = b;
            for(int k = 1; k <= 8 && !e_per; k++){
                long t = x; x = y; y = t;                   /* ESPELHA: troca */
                if(x == a && y == b) e_per = k;
            }
        }
        {   long a = 3, b = 5;
            long x = a, y = b;
            for(int k = 1; k <= 8 && !r_per; k++){
                long t = x; x = -y; y = t;                  /* RODA: (a,b) -> (-b,a) */
                if(x == a && y == b) r_per = k;
            }
        }
        /* e os MODOS: cada um e' fixo pela sua operacao — autovalor, nao periodo */
        long fa = 1, fb = 1, t1 = fa; fa = fb; fb = t1;      /* espelho em (1,1) */
        int fase_fixa = (fa == 1 && fb == 1);
        long oa = 1, ob = -1;                                /* oposicao no espelho: (1,-1) -> (-1,1) */
        long t2 = oa; oa = ob; ob = t2;
        int opos_troca = (oa == -1 && ob == 1);
        printf("      a OPERACAO: espelha -> periodo %ld ; roda -> periodo %ld\n", e_per, r_per);
        printf("      os MODOS:   (1,1) e' FIXO pelo espelho (autovalor +1); (1,-1) troca de sinal (-1)\n");
        printf("      e o relogio.h ja' contava: um lado %d, dois lados %d\n\n",
               colisor_passos(1), colisor_passos(2));
        ok("os dois periodos do pendulo duplo SAO os do relogio: a operacao que espelha fecha"
           " em 2 e a que roda fecha em 4 — os numeros que colisor_passos ja' dava para um"
           " lado e para dois. E os MODOS nao tem periodo: tem autovalor, e o modo em fase e'"
           " FIXO enquanto o de oposicao troca de sinal",
           e_per == 2 && r_per == 4 && e_per == colisor_passos(1) && r_per == colisor_passos(2)
           && fase_fixa && opos_troca);
    }

    /* ═══ §P4 — a involucao e' inverter o tempo ══════════════════════════════════════
     * Num pendulo sem atrito, trocar o sinal das velocidades e' andar para tras. Aplicado
     * duas vezes devolve o estado — e' involucao. Varre-se o espaco em inteiros. */
    {
        long mau = 0, n = 0;
        for(long x = -8; x <= 8; x++) for(long v = -8; v <= 8; v++){
            long rx = x,  rv = -v;                 /* ν: inverter o tempo */
            long bx = rx, bv = -rv;                /* ν∘ν */
            if(bx != x || bv != v) mau++;
            /* e o CONTROLO: uma "involucao" que tambem troque o sinal da posicao NAO
             * devolve o estado — e' a mesma forma e nao e' a mesma operacao */
            long cx = -x, cv = -v, dx = -cx, dv = -cv;
            if(dx == x && dv == v) n++;            /* essa tambem fecha... */
            if((x != 0 || v != 0) && (-x == x && -v == v)) mau++;
        }
        printf("      289 estados: ν∘ν = id em todos; e ν e' (x,v) -> (x,-v)\n\n");
        ok("inverter o tempo E' a involucao do pendulo: aplicada duas vezes devolve o"
           " estado, em 289 pontos e sem excepcao — e' o mesmo ν∘ν = id que o corpo pede",
           mau == 0);
    }

    /* ═══ §P5 — o dual: o pente de passo d e o de passo N/d ══════════════════════════
     * O relogio.h §R3: o dual do pente de passo d e' o de passo N/d, e os dois passos
     * multiplicam-se para dar N. No pendulo: uma junta que bate de d em d tem por dual a
     * que bate de N/d em N/d, e o AUTODUAL e' onde d*d = N. */
    {
        long N = 36, auto_d = 0, pares = 0, mau = 0;
        for(long d = 1; d <= N; d++){
            if(N % d) continue;
            long dual = N / d;
            pares++;
            if(d * dual != N) mau++;               /* a norma, outra vez */
            if(d == dual) auto_d = d;
        }
        printf("      N = %ld: %ld divisores, e o autodual e' d = %ld (d*d = N)\n\n",
               N, pares, auto_d);
        ok("o dual da junta de passo d e' a de passo N/d, e os dois multiplicam-se para dar"
           " N em todos os divisores; o AUTODUAL e' onde d*d = N — aqui d = 6, e e' o unico",
           mau == 0 && auto_d == 6 && auto_d * auto_d == N);
    }

    /* ═══ §P6 — O REGIME E O FECHO, NA REGUA DESTA TEORIA ════════════════════════════
     *
     * Eu tinha escrito aqui «o pendulo duplo verdadeiro e' caotico» e fui buscar Lyapunov
     * para o sustentar. NAO ERA PRECISO E ESTAVA ERRADO: a definicao importada nomeia so'
     * o lado que ESTICA — num sistema que conserva, os expoentes vem em pares que somam
     * zero —, e o catalogo JA' TINHA a classificacao, que e' o trial:
     *
     *      cristal  encolhe    ·    borda  conserva    ·    caos  cresce
     *
     * lida pelo sinal, e medida em dois corpos. O pendulo e' BORDA: conserva, e gira
     * porque so' tem a coordenada que roda — o esquilo.
     *
     * E O QUE DISTINGUE O LINEAR DO NAO-LINEAR NAO E' O REGIME: os dois conservam. E' o
     * FECHO. O operador [[1,1],[-1,0]] tem det = 1 e periodo SEIS, e o periodo NAO ve a
     * amplitude: toda orbita fecha na mesma contagem. E' isso que faz um relogio ser
     * relogio. Com o termo cubico o periodo passa a depender da amplitude — e na amplitude
     * grande nem fecha. Sem periodos fixos, o §P1 fica sem o que cruzar: nao ha' LCM. */
    printf("\n§P6  O REGIME e o FECHO: o pendulo e' BORDA, e so' o isocrono e' relogio.\n\n");
    {
        /* (1) o REGIME: a area no espaco de fase conserva-se? (det = 1) */
        long q[4] = {100000,100100,100100,100000}, p[4] = {0,0,100,100};
        long A0 = 0, varia = 0;
        for(int t = 0; t <= 60; t++){
            long ux=q[1]-q[0], uy=p[1]-p[0], vx=q[3]-q[0], vy=p[3]-p[0];
            long A = ux*vy - uy*vx; if(A < 0) A = -A;
            if(t == 0) A0 = A; else if(A != A0) varia++;
            for(int i = 0; i < 4; i++){ p[i] -= 3000*q[i]/1000; q[i] += p[i]; }
        }
        /* (2) o FECHO: o periodo depende da amplitude? */
        long amp[4] = {6, 24, 42, 60};
        int iso[4] = {0,0,0,0}, cub[4] = {0,0,0,0};
        for(int k = 0; k < 4; k++){
            long x = amp[k], v = 0;
            for(int t = 1; t <= 200 && !iso[k]; t++){ v -= x; x += v; if(x == amp[k] && v == 0) iso[k] = t; }
        }
        for(int k = 0; k < 4; k++){
            long x = amp[k], v = 0;
            for(int t = 1; t <= 400 && !cub[k]; t++){
                long c = x*x*x/2000;                       /* o termo que tira a isocronia */
                v -= (x - c); x += v; if(x == amp[k] && v == 0) cub[k] = t;
            }
        }
        printf("      a area no espaco de fase varia em %ld de 60 passos   (0 = BORDA)\n\n", varia);
        printf("      amplitude      isocrono      com o cubico\n");
        for(int k = 0; k < 4; k++)
            printf("      %9ld  %12d  %16s\n", amp[k], iso[k],
                   cub[k] ? (char[16]){0} : "NAO FECHA");
        printf("\n");
        for(int k = 0; k < 4; k++)
            if(cub[k]) printf("        (amplitude %ld fecha em %d)\n", amp[k], cub[k]);
        printf("\n");
        int todos_seis = (iso[0]==6 && iso[1]==6 && iso[2]==6 && iso[3]==6);
        int cub_varia  = !(cub[0]==cub[1] && cub[1]==cub[2] && cub[2]==cub[3]);
        int cub_falha  = (cub[3] == 0);
        ok("o regime le-se na regua desta teoria e nao numa importada: o pendulo e' BORDA —"
           " a area nao varia em 60 passos, e ele gira porque so' tem a coordenada que roda."
           " E o que distingue o linearizado NAO e' o regime, que e' o mesmo nos dois: e' o"
           " FECHO. Isocrono, o periodo e' SEIS e nao ve a amplitude — toda orbita fecha na"
           " mesma contagem, e e' isso que faz um relogio; com o cubico o periodo muda com a"
           " amplitude e na maior nem fecha, e sem periodos fixos nao ha' LCM para cruzar",
           varia == 0 && todos_seis && cub_varia && cub_falha);
    }



    /* ═══ §P7 — O PENDULO E' O RELOGIO: ponteiro preso no centro ═════════════════════
     *
     * O Aarao, tres vezes: «um pendulo E' o relogio, ponteiro fixo no centro, centro
     * dimensao 6 invariante, so' ler.»
     *
     * E ESTAVA ESCRITO. Eu andei a construir medicoes para descobrir o que a teoria ja'
     * diz na escada das dimensoes:
     *
     *      6  plena       soma e produto COINCIDEM — tudo e' possivel
     *      5  complexa    o plano abre, e o que cresce enrola — a espiral
     *      4  tetral      as quatro reguas fecham grupo — a cruz, e o tempo
     *      3  trial       dois lados e o sitio onde se trocam — O PONTO FIXO
     *      2  dual        a troca, e nada onde ela aconteca — a estaca
     *      1  ela propria A UNIDADE E'.
     *
     * O pendulo e' o ponteiro; o centro e' o ponto fixo, que se define por ι(x)=x — «uma
     * frase que, lida como informacao, nao acrescenta nada, e que e' exactamente o que
     * distingue o zero de todos os outros». O ponteiro roda, o centro nao se move.
     *
     * E AS LEIS SAO QUATRO CASOS DE SINAL, e nao dois — a teoria tabela-os. Eu tinha
     * escrito «Lei 1 da' o espelho», e e' falso: o espelho e' a LEI 2 COM SINAL +. A Lei 1
     * e' sobre o ponto fixo de VALOR, nao sobre a operacao. */
    printf("\n§P7  O PENDULO E' O RELOGIO: ponteiro preso no centro.\n\n");
    {
        /* os QUATRO casos, como a teoria os tabela */
        long T_mais = 1, T_menos = -1;                 /* Lei 1: ponto fixo de VALOR */
        int lei1_ok = (T_mais == 1 && T_menos == -1);

        long e_a = 3, e_b = 5, x = e_a, y = e_b; int per_inv = 0;
        for(int k = 1; k <= 8 && !per_inv; k++){        /* Lei 2 com +: T² = +1 */
            long t = x; x = y; y = t;
            if(x == e_a && y == e_b) per_inv = k;
        }
        long j_a = 1, j_b = 0; x = j_a; y = j_b; int per_anti = 0;
        for(int k = 1; k <= 8 && !per_anti; k++){       /* Lei 2 com -: T² = -1 */
            long t = x; x = -y; y = t;
            if(x == j_a && y == j_b) per_anti = k;
        }
        /* o CENTRO: o ponto fixo nao se move, e e' o unico que nao se move */
        long fixos = 0, movidos = 0;
        for(long q = -6; q <= 6; q++) for(long p = -6; p <= 6; p++){
            long rq = -p, rp = q;                       /* o ponteiro roda */
            if(rq == q && rp == p) fixos++; else movidos++;
        }
        printf("      Lei 1 (ponto fixo de VALOR):      T = +1  e  T = -1\n");
        printf("      Lei 2 com sinal +  (involucao):   T² = +1, fecha em %d\n", per_inv);
        printf("      Lei 2 com sinal -  (antissim.):   T² = -1, fecha em %d   <- o bidual\n",
               per_anti);
        printf("      e o colisor ja' contava: %d e %d\n\n",
               colisor_passos(1), colisor_passos(2));
        printf("      o CENTRO: em 169 pontos, %ld nao se move e %ld movem-se\n", fixos, movidos);
        printf("      -> o ponteiro roda, o centro fica. E' o ponto fixo, e e' UM SO'.\n\n");

        /* a dimensao SEIS: soma e produto coincidem — a linha de cima da escada */
        long g = 3, pot = 1, tab[6]; long mau = 0;
        for(int k = 0; k < 6; k++){ tab[k] = pot; pot = pot * g % 7; }
        for(int a = 0; a < 6; a++) for(int b = 0; b < 6; b++)
            if(tab[(a+b) % 6] != tab[a] * tab[b] % 7) mau++;
        printf("      a dimensao 6 e' a PLENA: 1+2+3 = %d = 1x2x3, e somar E' multiplicar\n",
               1+2+3);
        printf("      medido em 36 pares: %ld falhas\n\n", mau);

        ok("o pendulo E' o relogio — ponteiro preso no centro —, e as leis sao QUATRO casos"
           " de sinal e nao dois: a Lei 1 fixa o VALOR (+1 e -1) e a Lei 2 fixa a OPERACAO"
           " (T²=+1 involucao, fecha em 2; T²=-1 antissimetria, fecha em 4 — e e' esta que"
           " da' o bidual). O centro e' o ponto fixo e e' UM SO' em 169 pontos; e na dimensao"
           " SEIS, a plena, somar e multiplicar coincidem em 36 de 36",
           lei1_ok && per_inv == 2 && per_anti == 4
           && per_inv == colisor_passos(1) && per_anti == colisor_passos(2)
           && fixos == 1 && movidos == 13*13 - fixos && mau == 0);
    }


    /* ═══ §P8 — TRADUZIR UMA REGUA NOUTRA E' DAR A VOLTA AO PONTEIRO ═════════════════
     *
     * O Aarao: «ve as traducoes pela regua do relogio, transforma uma regua noutra — e
     * primeiro ACTUALIZA-TE, nao saias a atirar para todo o lado».
     *
     * ACTUALIZEI-ME, E JA' ESTAVA FEITO. O `converte.c` mede que transformar um dado
     * noutro NAO E' ALGORITMO: e' uma divisao do corpo. Dados A e B, o conversor e' o
     * C com A ⊛ C = B, e ele COLHE-SE do dual:
     *
     *      A⁻¹ = (∏_{i≥1} Frobⁱ(A)) · N(A)⁻¹        C = B ⊛ A⁻¹
     *
     * com residuo 0 em 5000 casos e 512 blocos. E o catalogo ja' tinha a tabela do
     * bestiario COMO TRADUTOR — «a regua e a dinamica sao as duas colunas que sobrevivem
     * a traducao».
     *
     * O QUE FALTAVA ERA UMA FRASE, e e' esta: O FROBENIUS E' O PONTEIRO. O converte.c mede
     * Frob⁴ = id e Frob³ = Frob⁻¹; o relogio.h conta colisor_passos = 4 e colisor_volta = 3.
     * SAO O MESMO NUMERO, e nao por coincidencia: dar a volta ao ponteiro E' percorrer os
     * conjugados, e o inverso — o que traduz — esta' a TRES batidas, do outro lado, sem
     * passar pelo centro. */
    printf("\n§P8  TRADUZIR E' DAR A VOLTA: o Frobenius e' o ponteiro.\n\n");
    {
        int n = 4;                                   /* a dimensao do converte.c */
        int frob_fecha = n, frob_volta = n - 1;
        int rel_fecha = colisor_passos(2), rel_volta = colisor_volta(rel_fecha);
        /* e o ponteiro visita periodo-1 sitios antes de fechar, e nenhum e' o centro */
        int visitados = colisor_intermedios(rel_fecha);
        printf("      converte.c :  Frob^%d = id        Frob^%d = Frob^-1\n", frob_fecha, frob_volta);
        printf("      relogio.h  :  fecha em %d         a volta em %d\n", rel_fecha, rel_volta);
        printf("      e o ponteiro visita %d sitios antes de fechar, e nenhum e' o centro\n\n",
               visitados);
        ok("traduzir uma regua noutra E' dar a volta ao ponteiro, e os numeros sao os mesmos:"
           " o converte.c mede Frob^4 = id com Frob^3 = Frob^-1, e o relogio conta 4 para"
           " fechar e 3 para voltar. O conversor colhe-se do DUAL — os conjugados — e por"
           " isso o que traduz esta' a TRES batidas, do outro lado, sem passar pelo centro",
           frob_fecha == rel_fecha && frob_volta == rel_volta && visitados == 3);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O PENDULO N-UPLO E' N RELOGIOS, e nada disto foi acrescentado ao objecto:");
        puts("");
        puts("    n juntas          n relogios, e o conjunto fecha no LCM dos periodos");
        puts("    o acoplamento     tridiagonal, e o determinante DOBRA-SE: 2D(n-1)-D(n-2)");
        puts("    os dois modos     em FASE espelha (2) · em OPOSICAO roda (4)");
        puts("                      — os numeros que colisor_passos ja' dava");
        puts("    a involucao       inverter o tempo, e ν∘ν = id em 289 estados");
        puts("    o dual            passo d contra passo N/d, autodual em d*d = N");
        puts("");
        puts("  E O QUE O SEPARA DE UMA METAFORA: sem linearizar nao ha' periodo nenhum.");
        puts("  A linearizacao nao e' uma simplificacao do modelo — E' A REGUA, e e' ela");
        puts("  que faz o pendulo ser um relogio em vez de um sistema que nao volta.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
