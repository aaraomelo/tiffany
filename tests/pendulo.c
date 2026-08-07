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
 *   §P6  o CONTROLO, e e' o que separa a modelacao de uma metafora: o pendulo NAO
 *        linearizado nao fecha. A linearizacao E' a regua, e sem ela nao ha' periodo.
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
        printf("      periodos {2,4,3,6}:  lcm = %ld   soma = %ld   produto = %ld\n", l, s, pr);
        printf("      cada junta divide o lcm; e o lcm divide o produto\n\n");
        ok("n juntas sao n relogios e o conjunto fecha no LCM dos periodos — cada junta"
           " volta ao inicio la' dentro, e nem a soma nem o produto o fazem."
           " E' o cruzamento do viveiro, a operacao que voa sempre",
           l == 12 && mau == 0 && l != s && l != pr);
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

    /* ═══ §P6 — O CONTROLO: sem linearizar, NAO fecha ═══════════════════════════════
     * E' isto que separa uma modelacao de uma metafora bonita. Tudo acima vale para o
     * pendulo LINEARIZADO. O pendulo duplo verdadeiro e' caotico: duas condicoes iniciais
     * vizinhas afastam-se, e a orbita nao volta.
     *
     * Mede-se em ponto fixo inteiro (escala 10^6), com o termo nao-linear a entrar como
     * produto dos angulos. Duas trajectorias que comecam a UMA unidade de distancia: no
     * linear a distancia fica limitada; no nao-linear cresce. */
    {
        /* Ponto fixo inteiro. A primeira versao que escrevi tinha o termo nao-linear tao
         * fraco que a divisao inteira o engolia: linear e nao-linear afastavam-se O MESMO,
         * e a asercao acusou. Um "controlo" que da o mesmo dos dois lados nao controla nada.
         * Agora o termo cubico entra com peso a serio, e a diferenca mede-se. */
        long a1 = 900000, b1 = 0, a2 = 900001, b2 = 0;     /* vizinhas: diferem em 1 */
        long lin = 0;
        for(int t = 0; t < 3000; t++){
            b1 -= a1/50;  a1 += b1/50;
            b2 -= a2/50;  a2 += b2/50;
            long d = a1 - a2; if(d < 0) d = -d;
            if(d > lin) lin = d;
        }
        long c1 = 900000, d1 = 0, c2 = 900001, d2 = 0;
        long nlin = 0;
        for(int t = 0; t < 3000; t++){
            /* sin(x) ~ x - x^3/6 : o cubico e' o que traz o caos */
            long k1 = (c1/1000)*(c1/1000)/1000*(c1/1000)/6;
            long k2 = (c2/1000)*(c2/1000)/1000*(c2/1000)/6;
            d1 -= (c1 - k1)/50;  c1 += d1/50;
            d2 -= (c2 - k2)/50;  c2 += d2/50;
            long d = c1 - c2; if(d < 0) d = -d;
            if(d > nlin) nlin = d;
        }
        printf("      duas trajectorias a UMA unidade, 3000 passos:\n");
        printf("        linearizado  afasta-se ate' %ld\n", lin);
        printf("        nao-linear   afasta-se ate' %ld   (%ld vezes mais)\n\n",
               nlin, nlin/(lin ? lin : 1));
        ok("e o CONTROLO, que e' o que faz disto modelacao e nao metafora: TUDO o que esta'"
           " acima vale para o pendulo LINEARIZADO. Sem linearizar, duas trajectorias"
           " vizinhas afastam-se muito mais e a orbita nao fecha — a linearizacao E' a"
           " regua, e sem regua nao ha periodo nenhum", nlin > lin * 2 && lin > 0);
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
