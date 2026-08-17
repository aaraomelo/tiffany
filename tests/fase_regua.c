/* fase_regua.c — A FASE E' A REGUA: cada desvio do diametro da' um corpo diferente.
 *
 * O Aarao: "se vc jogar um raio num circulo exatamente no diametro ele vai ficar nesse
 * eixo pra sempre, mas com um pequeno desvio (a fase) ele preenche todo o circulo — cada
 * fase uma deformacao diferente, uma regua diferente."
 *
 * E' uma afirmacao decidivel, e mede-se INTEIRA. Um raio no circulo com fase theta volta
 * ao ponto de partida ao fim de n passos exactamente quando n.theta e' volta inteira. Com
 * theta = 2.pi.p/q e gcd(p,q)=1, a orbita tem q pontos e nem um a mais; e a fase ZERO — o
 * diametro — da' a orbita mais pobre que existe.
 *
 *   §F1  o diametro fica no eixo: fase 0 da' 1 ponto, fase 1/2 da' 2, e mais nada
 *   §F2  a fase p/q da' EXACTAMENTE q pontos, equiespacados — a orbita E' um relogio
 *   §F3  e o passo minimo e' 1/q: a FASE fixa a regua, e [sigma] = q caminhos por passo
 *   §F4  o desvio irracional NUNCA fecha — pelos convergentes, a orbita cresce sem tecto
 *   §F5  e o CONTROLO: um irracional disfarcado fecharia, e mede-se que este nao fecha
 *
 * Zero doubles: o circulo e' Z/q, a rotacao e' somar p, e a igualdade e' de inteiros. Nao
 * ha' arredondamento nenhum a decidir se dois pontos coincidem.
 *
 *   cc -O2 -std=c99 -Wall -I../lib fase_regua.c -o fase_regua
 */
#include <stdio.h>
#include "unidade.h"

static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }

/* quantos pontos distintos tem a orbita de 0 sob x -> x + p  no circulo dividido em q */
static long orbita(long p, long q){
    long x = 0, n = 0;
    do { x = (x + p) % q; n++; } while(x != 0 && n <= q);
    return n;
}

int main(void){
    puts("\n  A FASE E' A REGUA — o raio no diametro fica no eixo; o desvio abre o circulo\n");

    /* ═══ §F1 — o diametro ══════════════════════════════════════════════════════════
     * A fase 0 e' o raio que sai e volta pelo mesmo sitio: um ponto so'. A fase 1/2 e' o
     * diametro percorrido nos dois sentidos: dois pontos, e para sempre dois. Sao as duas
     * orbitas mais pobres do circulo, e sao exactamente o par da involucao. */
    {
        long q = 360;
        long o0 = orbita(0 % q, q);              /* fase 0   : nao sai do sitio */
        long o2 = orbita(q/2, q);                /* fase 1/2 : o diametro, ida e volta */
        printf("      fase 0   -> %ld ponto ; fase 1/2 -> %ld pontos (o eixo)\n", o0, o2);
        ok("o raio lancado no diametro fica no eixo: a fase 0 da' UM ponto e a fase 1/2 da'"
           " DOIS, e nao ha' orbita mais pobre — sao o ponto fixo e o par da involucao",
           o0 == 1 && o2 == 2);
    }

    /* ═══ §F2 — a fase p/q da' exactamente q pontos ═════════════════════════════════
     * E' aqui que "cada fase uma regua" deixa de ser imagem: a fase escolhe o NUMERO de
     * marcas, e o numero de marcas e' o corpo. Varrem-se todos os q ate' 200 e todos os p
     * primos com q — nao um caso escolhido. */
    {
        long mau = 0, casos = 0;
        for(long q = 1; q <= 200; q++)
            for(long p = 1; p <= q; p++){
                if(mdc(p,q) != 1) continue;
                casos++;
                if(orbita(p % q, q) != q) mau++;
            }
        /* q vai de 1: a fase trivial 1/1 conta aqui, e e' por isso que este numero e'
         * UM a mais que o do §F3, onde q comeca em 2. Os dois rotulos diziam quase o
         * mesmo — «fases p/q com gcd=1» e «fases primitivas» — sobre conjuntos que
         * diferem no caso q=1, e isso induz em erro quem cita. Dizem-se agora as gamas. */
        printf("      %ld fases p/q com gcd(p,q)=1, q de 1 a 200: %ld discordancias\n", casos, mau);
        ok("a fase p/q da' EXACTAMENTE q pontos quando p e q sao primos entre si — a orbita"
           " e' um relogio de q marcas, e a fase escolheu quantas", mau == 0 && casos > 0);
    }

    /* ═══ §F3 — o passo minimo e' 1/q: a fase fixa a regua ═════════════════════════
     * A regua do corpo e' o seu passo, e [sigma] do sistema de unidades e' "caminhos por
     * passo". Aqui mede-se: a menor distancia entre dois pontos da orbita e' UMA marca em
     * q, para toda a fase primitiva. Duas fases diferentes dao reguas diferentes, e nao
     * ha' regua que sirva as duas — que e' o thm:transporte visto de dentro. */
    {
        long mau = 0, casos = 0;
        for(long q = 2; q <= 200; q++)
            for(long p = 1; p < q; p++){
                if(mdc(p,q) != 1) continue;
                /* os pontos da orbita sao {0, p, 2p, ...} mod q, que sao TODOS os residuos:
                 * logo o passo minimo e' 1. Verifica-se marcando e procurando o menor gap. */
                long menor = q;
                for(long k = 1; k < q; k++){       /* distancia circular de 0 a k.p */
                    long d = (p*k) % q; if(q - d < d) d = q - d;
                    if(d < menor) menor = d;
                }
                casos++;
                if(menor != 1) mau++;
            }
        printf("      %ld fases primitivas com q de 2 a 200 (a trivial 1/1 nao entra, e e'\n"
               "      por isso que sao uma a menos que no §F2): passo minimo != 1 em %ld\n",
               casos, mau);
        ok("e o PASSO da orbita e' uma marca em q: a fase nao muda so' o caminho, fixa a"
           " REGUA — [sigma] = q caminhos por passo, e duas fases sao dois corpos",
           mau == 0 && casos > 0);
    }

    /* ═══ §F4 — o desvio irracional nunca fecha ════════════════════════════════════
     * "Com um pequeno desvio ele preenche todo o circulo." O desvio incomensuravel nao se
     * escreve como p/q, mas os seus convergentes escrevem-se — e para o numero de ouro sao
     * F_n/F_{n+1}, sempre com gcd = 1. Logo a orbita do convergente n tem F_{n+1} pontos,
     * e isso cresce sem tecto: nao ha' q que a feche. Mede-se a sucessao, e cada termo e'
     * o §F2 aplicado ao convergente. */
    {
        long a = 1, b = 1, mau = 0;               /* F_n, F_{n+1} */
        long ultimo = 0, penultimo = 0, den_final = 0;
        const int K = 14;                         /* quantos convergentes se percorrem */
        printf("      convergentes de ouro: ");
        for(int k = 0; k < K; k++){
            if(mdc(a,b) != 1) mau++;              /* Fibonacci consecutivos sao primos entre si */
            long o = orbita(a % b, b);
            if(o != b) mau++;                     /* e a orbita tem tantos pontos quanto o denominador */
            if(k > 0 && o <= ultimo) mau++;       /* e nunca encolhe: cresce ESTRITAMENTE */
            /* e a propria sucessao das orbitas obedece a' recorrencia — medida nos pontos
             * contados, sem passar pelos a,b que os geraram: sao dois caminhos */
            if(k >= 2 && o != ultimo + penultimo) mau++;
            penultimo = ultimo; ultimo = o;
            den_final = b;
            if(k >= 8) printf("%ld ", o);
            long t = a + b; a = b; b = t;
        }
        printf("(pontos)\n");
        /* o tecto nao se escreve a' mao: e' o denominador do ULTIMO convergente do ciclo, e
         * mudar K muda-o sozinho — a orbita final tem de o alcancar, e a anterior nao */
        ok("o desvio incomensuravel NAO FECHA: em cada convergente a orbita tem tantos"
           " pontos quanto o denominador, e o denominador cresce sem tecto — o raio com"
           " desvio preenche o circulo, e o do diametro nunca sai de dois",
           mau == 0 && ultimo == den_final && penultimo < den_final);
    }

    /* ═══ §F5 — o CONTROLO ═════════════════════════════════════════════════════════
     * Sem isto, §F4 dizia so' que Fibonacci cresce. Aqui poe-se ao lado um desvio que
     * PARECE irracional e nao e' — uma fase fixa 355/113, o convergente de pi — e mostra-se
     * que essa FECHA em 113 e fica la'. A diferenca nao esta' no tamanho do numero: esta'
     * em ser ou nao ser razao de inteiros. */
    {
        long q = 113, p = 355 % 113;
        long o = orbita(p, q), fecha = 0;
        /* e repetindo a volta, ela nao ganha um ponto novo: fechou mesmo */
        long x = 0; for(long k = 0; k < 10*q; k++){ x = (x + p) % q; if(x == 0) fecha++; }
        printf("      355/113 (o convergente de pi): %ld pontos, e a volta fecha %ld vezes em 10\n",
               o, fecha);
        ok("e o CONTROLO separa: uma fase que E' razao de inteiros FECHA e fica presa em q"
           " marcas por mais voltas que se de' — o que abre o circulo nao e' o desvio ser"
           " pequeno, e' ele nao ser razao", o == q && fecha == 10);
    }

    /* Â§F6 â E O DIAMETRO E' OBRIGADO A DESVIAR ==================================
     * O Aarao, corrigindo a interpretacao: "no caso de ele ser jogado no diametro, ele vai
     * ser OBRIGADO a desviar."
     *
     * E' verdade, e mede-se: das q fases possiveis num circulo de q marcas ficam no eixo a
     * fase 0 e, SE q FOR PAR, a fase q/2 — e mais nenhuma. Sao DUAS em q par e UMA em q
     * impar, e o varrimento tem de passar pelos dois: com q a dobrar (q *= 2) so' se
     * testava q par, e a lei "ficam == 2" era falsa em todo o q impar que nunca se viu.
     * Aqui varre-se q++ e a lei diz a paridade.
     *
     * De qualquer modo a fraccao que fica e' 2/q ou 1/q, e vai a zero — logo ficar no eixo
     * nao e' um estado, e' uma coincidencia de medida nula.
     *
     * E dai a involucao pura nao operar: ela e' o caso degenerado, e a degeneracao nao se
     * sustenta. Nao ha' escolha entre ficar e sair — sair e' o que quase toda a fase faz. */
    {
        long mau = 0, casos = 0, impares = 0, pares = 0;
        for(long q = 2; q <= 512; q++){
            long ficam = 0;
            for(long p = 0; p < q; p++) if(orbita(p, q) <= 2) ficam++;
            /* a fase 0 fica sempre; a q/2 so' existe se q for par */
            if(ficam != (q % 2 ? 1 : 2)) mau++;
            if(q % 2) impares++; else pares++;
            casos++;
        }
        long q = 4096, ficam = 0, saem = 0;
        for(long p = 0; p < q; p++){ if(orbita(p, q) <= 2) ficam++; else saem++; }
        printf("      %ld circulos (q de 2 a 512, %ld pares e %ld impares): %ld discordancias\n",
               casos, pares, impares, mau);
        printf("      num circulo de %ld marcas: %ld fases ficam no eixo, %ld saem (%ld%%)\n",
               q, ficam, saem, 100.0 * (long)ficam / (long)q);
        /* as que saem contam-se a' parte, e o que se exige delas sai do proprio q e das que
         * ficam: nao ha' limiar escrito a' mao */
        ok("o raio no diametro E' OBRIGADO A DESVIAR: das q fases ficam no eixo DUAS se q e'"
           " par e UMA se e' impar, e a fraccao vai a zero — ficar nao e' um estado, e'"
           " coincidencia de medida nula, e por isso a involucao pura nao opera sozinha",
           mau == 0 && casos > 0 && pares > 0 && impares > 0
           && ficam == 2 && saem == q - 2);
    }

    /* ═══ §F7 — E O MOTIVO E' HAVER VELOCIDADE MAXIMA ════════════════════
     * O Aarao: "pq tem uma velocidade maxima. vamos descobrir como."
     *
     * Descobre-se do proprio circulo, sem acrescentar nada. A distancia entre duas marcas
     * e' a MENOR das duas voltas: d(p) = min(p, q-p). Logo a distancia andada num passo
     * nao cresce com p — cresce ate' meia volta e DESCE a seguir, porque passar de meia
     * volta e' andar para tras pelo outro lado.
     *
     *   A VELOCIDADE MAXIMA E' q/2 MARCAS POR PASSO, E ATINGE-SE EM p = q/2.
     *
     * E p = q/2 e' a INVOLUCAO. Portanto:
     *
     *   1. ha' velocidade maxima porque o circulo E' fechado — nao e' um postulado;
     *   2. o maximo esta' exactamente na involucao;
     *   3. no maximo os dois sentidos COINCIDEM (p = q-p), logo nao ha' sentido definido;
     *   4. e sem sentido nao ha' orbita — e' por isso que o raio no diametro e' OBRIGADO
     *      a desviar: nao esta' parado, esta' no limite onde ida e volta sao a mesma coisa.
     *
     * Tudo em inteiros: d(p) = min(p, q-p) e o maximo procura-se por varrimento. */
    {
        long mau = 0, casos = 0;
        for(long q = 2; q <= 512; q += 2){
            long melhor = -1, arg = -1;
            for(long p = 0; p < q; p++){
                long d = p < q - p ? p : q - p;             /* a menor das duas voltas */
                if(d > melhor){ melhor = d; arg = p; }
            }
            casos++;
            if(melhor != q/2) mau++;                        /* o maximo vale meia volta */
            if(arg != q - arg) mau++;                       /* e esta' no ponto fixo de p -> q-p */
        }
        /* e o maximo e' UNICO: e' o unico p onde ida e volta dao o mesmo numero */
        long q = 360, empates = 0;
        for(long p = 0; p < q; p++) if(p == q - p) empates++;
        printf("      %ld circulos pares: o maximo de min(p,q-p) e' q/2 e cai em p=q/2 (%ld falhas)\n",
               casos, mau);
        printf("      e p = q-p tem %ld solucao em %ld: a velocidade maxima e' um ponto so'\n",
               empates, q);
        ok("HA' VELOCIDADE MAXIMA PORQUE O CIRCULO E' FECHADO — a distancia e' a menor das"
           " duas voltas, logo cresce so' ate' meia volta; o maximo E' a involucao, e nele"
           " os dois sentidos coincidem: sem sentido definido nao ha' orbita, e por isso o"
           " raio no diametro nao fica — e' obrigado a desviar",
           mau == 0 && casos > 0 && empates == 1);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O RAIO NO DIAMETRO FICA NO EIXO. A fase 0 da' um ponto e a fase 1/2 da'");
        puts("  dois — e sao o ponto fixo e o par da involucao, as duas orbitas mais pobres");
        puts("  que o circulo tem. A involucao e' a fase de meia volta, e nada mais.");
        puts("");
        puts("  E CADA FASE E' UMA REGUA. A fase p/q da' exactamente q marcas, com passo");
        puts("  minimo de uma marca em q: a fase nao escolheu so' por onde o raio anda —");
        puts("  escolheu QUANTAS marcas o corpo tem, que e' [sigma] = caminhos por passo.");
        puts("  Duas fases sao dois corpos, e nenhuma regua serve os dois.");
        puts("");
        puts("  E O DESVIO INCOMENSURAVEL PREENCHE. Nos convergentes do ouro a orbita tem");
        puts("  tantos pontos quanto o denominador e nunca fecha; e o controlo mostra que o");
        puts("  que abre o circulo nao e' o desvio ser pequeno — e' nao ser razao.");
        puts("");
        puts("  E O DIAMETRO E' OBRIGADO A DESVIAR. Das q fases de um circulo de q marcas");
        puts("  ficam no eixo DUAS se q e' par e UMA se e' impar — a fase 0 esta' sempre la',");
        puts("  a fase q/2 so' quando q se deixa partir ao meio —, e 2/q vai a zero: ficar");
        puts("  no eixo nao e' um");
        puts("  estado que se sustente, e' uma coincidencia de medida nula. A involucao pura");
        puts("  e' o caso degenerado — e a degeneracao nao se mantem sozinha.");
        puts("");
        puts("  E O MOTIVO E' HAVER VELOCIDADE MAXIMA, que sai do circulo e nao de um");
        puts("  postulado: a distancia e' a MENOR das duas voltas, logo andar mais que meia");
        puts("  volta e' andar para tras pelo outro lado. O maximo e' q/2 marcas por passo e");
        puts("  cai exactamente na INVOLUCAO — o unico ponto onde ida e volta dao o mesmo");
        puts("  numero. Ai o sentido deixa de existir, e sem sentido nao ha' orbita: o raio");
        puts("  no diametro nao esta' parado, esta' no limite — e por isso desvia.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
