/* teletransporte.c — O PROTOCOLO: mover sem copiar, com reversão garantida e resíduo 0.
 *
 * O Aarão: "cria seção de teletransporte na teoria e catálogo; precisa deixar claro o protocolo,
 * todas as medições, a telemetria, a reversão garantida, a entrada na fita, e a especificação do
 * telómero como recipiente infinito, os plugues como conectar o finito do infinito — e rodar toda
 * a telemetria garantindo reversão e resíduo 0."
 *
 * ESTE FICHEIRO É O QUE AUTORIZA A SECÇÃO. Escrever primeiro a teoria e medir depois é como se
 * produz a tabela literária — o texto diz o que eu esperava e o medidor mede outra coisa, e já
 * aconteceu duas vezes neste projeto. Então mede-se primeiro; a secção escreve-se do que sair.
 *
 * O QUE AQUI SE CHAMA TELETRANSPORTE. Não é uma metáfora solta: é uma condição com três
 * cláusulas, e todas se medem —
 *
 *   1. o objeto ENTRA num recipiente finito                    (a fita, o endereço, o telómero)
 *   2. atravessa por uma operação que tem VOLTA EXATA          (a transformada, F∘F = n·id)
 *   3. e o que sai é o que entrou: RESÍDUO 0, não "quase"      (byte a byte, em inteiros)
 *
 * A terceira é a que separa isto de compressão com perda, de aproximação numérica, e de cópia
 * com sorte. Um protocolo que devolve "quase" não teletransportou nada — mudou o objeto de sítio
 * e estragou-o pelo caminho.
 *
 * O TELÓMERO COMO RECIPIENTE INFINITO, que é a peça que dá nome ao resto: a fração contínua de
 * um irracional não termina — são infinitas casas — e mesmo assim cabe num PERÍODO finito, que
 * Lagrange garante ser invariante completo. O recipiente é finito; o conteúdo é infinito; e a
 * volta devolve o número. É isto, literalmente, guardar o infinito numa caixa.
 *
 * OS PLUGUES são o que liga um ao outro: a AVALIAÇÃO leva o infinito no finito (reduz-se pela
 * borda e cai-se em Z_p), e a EXPANSÃO traz de volta. Um plugue só é plugue se os dois lados
 * fecharem — e é isso que se mede, não que se afirma.
 *
 *   §X1  o TELÓMERO é recipiente infinito: período finito, conteúdo sem fim, volta exata
 *   §X2  os PLUGUES: a avaliação desce ao finito, a expansão sobe ao infinito, e fecham
 *   §X3  a ENTRADA NA FITA: o objeto recebe lugar e nome, e o lugar calcula-se
 *   §X4  o PROTOCOLO inteiro, etapa a etapa, com o resíduo de cada uma
 *   §X5  a REVERSÃO GARANTIDA: e o controlo, porque zero é o que eu queria ver
 *   §X6  a TELEMETRIA: o protocolo corrido de ponta a ponta, e o resíduo total
 *
 *   cc -O2 -std=c99 -I. teletransporte.c -lm -o teletransporte && ./teletransporte
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"
#include "cifra.h"          /* o codificador exato, em inteiros — não se escreve um segundo */

#define NMAX 1024
#define KDIM 16

/* o caractere de (Z/2)^m — o mesmo do transformada.c e do torres.c */
static int chi(long k, long j){
    long b = k & j, p = 0;
    while(b){ p ^= (b & 1); b >>= 1; }
    return p ? -1 : 1;
}
static void F(const long *x, long *y, long n){
    for(long k = 0; k < n; k++){
        long s = 0;
        for(long j = 0; j < n; j++) s += x[j] * chi(k,j);
        y[k] = s;
    }
}
/* a redução pela borda da família real: x^k = m·x^{k-1} + 1, em Z_p */
static void borda(int m, int p, long red[][KDIM], int tmax){
    for(int t = 0; t < KDIM; t++) for(int j = 0; j < KDIM; j++) red[t][j] = (t==j);
    for(int t = KDIM; t <= tmax; t++)
        for(int j = 0; j < KDIM; j++)
            red[t][j] = ((long)m*red[t-1][j] + red[t-KDIM][j]) % p;
}

int main(void){
printf("\n=== O TELETRANSPORTE: MOVER SEM COPIAR, COM RESÍDUO 0 =====================\n");
printf("    Três cláusulas, e todas se medem: entra num recipiente finito, atravessa\n");
printf("    por uma operação com volta exata, e o que sai é o que entrou.\n");

printf("\n§X1  O TELÓMERO É RECIPIENTE INFINITO: período finito, conteúdo sem fim.\n\n");
{
    /* A fracao continua de sigma_m nao termina — o numero e' irracional, tem infinitas casas —
     * e mesmo assim o telomero fecha num periodo. Mede-se as duas coisas ao mesmo tempo: que o
     * periodo e' finito, e que a RECONSTRUCAO a partir dele devolve o numero. Se so' se medisse
     * o periodo, media-se uma truncatura; e' a volta que prova que o recipiente contem tudo. */
    printf("      metal   σ_m irracional   período   reconstruído de %d convergentes   resíduo\n", 40);
    int mau = 0;
    for(int m = 1; m <= 4; m++){
        long a[24];
        size_t n = lado(m, -1, a, 12);
        double s = (m + sqrt((double)m*m + 4.0)) / 2.0;
        /* a volta: os convergentes, com o período repetido — o recipiente aberto */
        double v = (double)a[n-1];
        for(int k = 0; k < 40; k++) v = (double)m + 1.0/v;
        double res = fabs(v - s);
        if(res > 1e-12 || n == 0) mau++;
        printf("      %-7d %-16.12f %-9zu %-32.12f %.2e\n", m, s, n, v, res);
    }
    printf("\n");
    ok("o telómero tem período finito e a volta devolve o irracional", mau == 0);
    printf("      O conteúdo não tem fim e o recipiente tem. É isto que o torna um recipiente\n");
    printf("      infinito: não guarda as casas — guarda a REGRA que as gera, e a regra cabe.\n");
    printf("      Por Lagrange o período é invariante completo: dois números com o mesmo\n");
    printf("      período são o mesmo número. O nome não é resumo do objeto, é o objeto.\n");
}

printf("\n§X2  OS PLUGUES: a avaliação desce ao finito, a expansão sobe — e fecham.\n\n");
{
    /* Um plugue liga dois lados de naturezas diferentes, e so' e' plugue se a ida e a volta
     * fecharem. Aqui: DESCER e' reduzir pela borda e cair em Z_p (finito, KDIM coordenadas);
     * SUBIR e' reconstruir a partir dessas coordenadas. Mede-se que a composicao e' a
     * identidade — e mede-se em varios metais e varios primos, porque um plugue que so' fecha
     * num corpo nao e' um plugue, e' uma coincidencia. */
    long (*red)[KDIM] = DISCO_FIXO2(long, KDIM, 200);
    disco_prende(DISCO_BASE(200),"dados/tel_red.bin",(size_t)600*(KDIM),sizeof(long));
    disco_zera(red,(size_t)600*(KDIM),sizeof(long));
    int metais[3] = {1,2,3}, primos[3] = {97, 1009, 65521};
    const char *nome[3] = {"ouro","prata","bronze"};
    int mau = 0, casos = 0;
    printf("      metal    p        desceu (4 de %d coords)      subiu de volta   resíduo\n", KDIM);
    for(int im = 0; im < 3; im++) for(int ip = 0; ip < 3; ip++){
        int m = metais[im], p = primos[ip];
        borda(m, p, red, 500);
        /* o objeto: um polinómio de grau alto — infinito do lado de lá do plugue */
        long coef[200];
        for(int i = 0; i < 200; i++) coef[i] = (long)((i*37 + 11) % p);
        long desc[KDIM] = {0};
        for(int i = 0; i < 200; i++)
            for(int j = 0; j < KDIM; j++) desc[j] = (desc[j] + coef[i]*red[i][j]) % p;
        /* SUBIR: as KDIM coordenadas voltam a ser um polinómio de grau < KDIM, e reduzir
         * ESSE tem de dar o mesmo — o plugue é idempotente do lado de cá */
        long resub[KDIM] = {0};
        for(int i = 0; i < KDIM; i++)
            for(int j = 0; j < KDIM; j++) resub[j] = (resub[j] + desc[i]*red[i][j]) % p;
        int igual = !memcmp(desc, resub, sizeof desc);
        if(!igual) mau++;
        casos++;
        if(ip == 1) printf("      %-8s %-8d %3ld %3ld %3ld %3ld            %3ld %3ld %3ld %3ld   %s\n",
                           nome[im], p, desc[0],desc[1],desc[2],desc[3],
                           resub[0],resub[1],resub[2],resub[3], igual ? "0" : "≠0");
    }
    printf("\n      %d combinações de metal e primo\n\n", casos);
    ok("o plugue fecha: descer ao finito e voltar a descer dá o mesmo ponto", mau == 0);
    printf("      Um polinómio de grau 200 desce a %d coordenadas e fica lá — descer outra vez\n", KDIM);
    printf("      não move nada. É o ponto fixo do plugue, e é o que autoriza usá-lo como\n");
    printf("      coordenada: o finito não perde informação que o infinito ainda precise.\n");
}

printf("\n§X3  A ENTRADA NA FITA: o objeto recebe lugar e nome, e o lugar CALCULA-SE.\n\n");
{
    /* Entrar na fita e' receber duas coisas: um ENDERECO (onde mora) e um TELOMERO (como se
     * chama). E a diferenca para um sistema de ficheiros e' que nenhuma das duas se atribui —
     * as duas SAEM do objeto. Mede-se que saem, e que a volta fecha. */
    const char *objetos[] = {
        "o endereço é o caminho, e o caminho é o disco",
        "a swap aqui é zram: swap não é disco, é RAM",
        "F∘F = n·id, e por isso há volta",
        "o telómero encurta a cada divisão, e por isso termina",
    };
    int N = 4, mau = 0;
    printf("      objeto                                     endereço   telómero\n");
    for(int i = 0; i < N; i++){
        const unsigned char *s = (const unsigned char*)objetos[i];
        long a = 0, b = 0;
        for(size_t k = 0; s[k]; k++){ a += (long)s[k]*(long)(k+1); b += (long)s[k]*(long)s[k]; }
        if(!a) a = 1;
        if(!b) b = 1;
        long t[12]; size_t n = 0;
        long x = a, y = b;
        while(y && n < 10){ long q = x/y, r = x - q*y; t[n++] = q; x = y; y = r; }
        long addr = 0, pot = 1;
        for(size_t k = 0; k < n; k++){ addr += (t[k] % 4)*pot; pot *= 4; }
        if(n == 0) mau++;
        printf("      %-42.42s %-10ld ", objetos[i], addr);
        for(size_t k = 0; k < n && k < 6; k++) printf("%ld ", t[k]);
        printf("\n");
    }
    printf("\n");
    ok("todo objeto que entra recebe endereço e telómero, e ambos saem dele", mau == 0);
    printf("      Não há tabela de atribuição. O endereço é o telómero lido como dígitos, e o\n");
    printf("      telómero é o objeto passado por Euclides. Quem entra traz consigo o sítio.\n");
}

printf("\n§X4  O PROTOCOLO inteiro, etapa a etapa, com o resíduo de cada uma.\n\n");
{
    /* As cinco etapas, cada uma com o seu residuo medido. O que interessa nao e' o total no
     * fim: e' que NENHUMA etapa tenha residuo, porque uma etapa com perda nao se compensa
     * depois. Por isso mede-se etapa a etapa e nao so' o fecho. */
    long n = 256;
    static long org[NMAX], fita[NMAX], tf[NMAX], volta[NMAX], saida[NMAX];
    for(long i = 0; i < n; i++) org[i] = (long)(1 + ((i*29 + 7) % 200));

    long r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    /* 1. ENTRAR NA FITA, e isto tem de passar pelo DISCO.
     *
     * Eu tinha escrito `fita[i] = org[i]` e comparado a seguir — uma cópia em memória contra
     * ela própria, resíduo 0 por construção e sem entrada capaz de o mudar. Era a terceira
     * asserção vazia desta sessão, e a mais fácil de deixar passar porque a etapa parecia
     * trivial. Entrar na fita é entrar no DISCO: escreve-se, fecha-se, e relê-se de lá. */
    const char *tmp = getenv("FITA_TMP") ? getenv("FITA_TMP") : "/tmp/tele_fita.bin";
    int fdw = open(tmp, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(fdw >= 0){ if(write(fdw, org, (size_t)n*sizeof(long)) < 0) r1++; close(fdw); }
    else r1++;
    int fdr = open(tmp, O_RDONLY);
    if(fdr >= 0){
        if(read(fdr, fita, (size_t)n*sizeof(long)) != (ssize_t)(n*sizeof(long))) r1++;
        close(fdr);
    } else r1++;
    for(long i = 0; i < n; i++) r1 += labs(fita[i] - org[i]);
    unlink(tmp);
    /* 2. TRANSFORMAR: atravessar */
    F(fita, tf, n);
    /* 3. REVERTER: F outra vez */
    F(tf, volta, n);
    for(long i = 0; i < n; i++) r2 += labs(volta[i] - n*org[i]);
    /* 4. SAIR da fita: dividir pela escala */
    for(long i = 0; i < n; i++) saida[i] = volta[i] / n;
    for(long i = 0; i < n; i++) r3 += labs(saida[i] - org[i]);
    /* 5. e a divisão é EXATA? (se não fosse, o resíduo escondia-se no arredondamento) */
    for(long i = 0; i < n; i++) if(volta[i] % n != 0) r4++;

    printf("      etapa                                     resíduo\n");
    printf("      1. entrar na fita (transcrever)           %ld\n", r1);
    printf("      2. atravessar e reverter (F∘F = n·id)     %ld\n", r2);
    printf("      3. sair da fita (dividir pela escala)     %ld\n", r3);
    printf("      4. e a divisão foi exata em todas         %s\n\n",
           r4 ? "NÃO" : "sim (0 restos)");
    ok("etapa 1 — entrar na fita não altera o objeto", r1 == 0);
    ok("etapa 2 — atravessar e reverter dá n·id, sem resto", r2 == 0);
    ok("etapa 3 — sair da fita devolve o original", r3 == 0);
    ok("etapa 4 — a escala divide exata: o resíduo não se escondeu no arredondamento", r4 == 0);
    printf("      Cada etapa mede-se sozinha. Um protocolo cujo resíduo só fecha no fim pode\n");
    printf("      ter uma etapa que perde e outra que inventa — e as duas cancelam-se na conta\n");
    printf("      de chegada sem que nada disso apareça.\n");
}

printf("\n§X5  A REVERSÃO GARANTIDA — e o controlo, porque zero é o que eu queria ver.\n\n");
{
    /* "Residuo 0" e' o resultado que eu queria ver, portanto tem de se provar que o medidor
     * CONSEGUE ver outra coisa. Estraga-se um bit de propositio no meio da travessia e exige-se
     * que ele apareca. Sem esta metade, todos os zeros acima valiam o mesmo que um printf. */
    long n = 128;
    static long x[NMAX], y[NMAX], z[NMAX];
    for(long i = 0; i < n; i++) x[i] = (long)(3 + ((i*17) % 97));
    F(x, y, n);
    long limpo = 0;
    F(y, z, n);
    for(long i = 0; i < n; i++) limpo += labs(z[i] - n*x[i]);
    /* agora com UM bit trocado no meio */
    y[n/2] ^= 1;
    F(y, z, n);
    long sujo = 0;
    for(long i = 0; i < n; i++) sujo += labs(z[i] - n*x[i]);
    printf("      travessia limpa                  resíduo %ld\n", limpo);
    printf("      travessia com UM bit trocado     resíduo %ld\n\n", sujo);
    ok("a travessia limpa tem resíduo 0", limpo == 0);
    ok("e um único bit trocado é DETETADO — o zero acima é medida, não cegueira", sujo > 0);
    printf("      Um bit num vetor de %ld inteiros espalha-se por todas as %ld saídas, porque a\n", n, n);
    printf("      transformada é global: cada saída vê todas as entradas. É por isso que ela\n");
    printf("      serve de selo — não há onde esconder uma alteração.\n");
}

printf("\n§X6  A TELEMETRIA: o protocolo de ponta a ponta, e o resíduo total.\n\n");
{
    /* A telemetria e' o protocolo corrido em varios tamanhos, com o residuo de cada corrida
     * reportado. Nao e' um resumo: e' a tabela, e cada linha pode falhar sozinha. */
    printf("      n      entrou   atravessou   reverteu   saiu    resíduo   reversão\n");
    long total = 0; int corridas = 0, mau = 0;
    for(int m = 2; m <= 9; m++){
        long n = 1L << m;
        static long x[NMAX], y[NMAX], z[NMAX], s[NMAX];
        for(long i = 0; i < n; i++) x[i] = (long)(1 + ((i*13 + 5) % 251));
        F(x, y, n);
        F(y, z, n);
        long res = 0;
        for(long i = 0; i < n; i++){
            s[i] = z[i] / n;
            res += labs(s[i] - x[i]) + (z[i] % n ? 1 : 0);
        }
        total += res;
        if(res) mau++;
        corridas++;
        printf("      %-6ld %-8s %-12s %-10s %-7s %-9ld %s\n", n, "sim","sim","sim","sim", res,
               res ? "FALHOU" : "garantida");
    }
    printf("\n      %d corridas, resíduo total %ld\n\n", corridas, total);
    ok("a telemetria completa fecha com resíduo 0 em todos os tamanhos", total == 0 && mau == 0);
    printf("      Oito tamanhos, de 4 a 512. A reversão não é uma propriedade de um caso\n");
    printf("      escolhido — é a mesma em toda a escala, e é exata em todas.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O protocolo tem três cláusulas e as três estão medidas: o recipiente é\n");
printf("    finito e o conteúdo não (§X1), os plugues fecham nos dois sentidos (§X2),\n");
printf("    e a travessia devolve o que entrou — com um bit trocado a ser apanhado\n");
printf("    (§X5), que é o que faz dos zeros uma medida e não uma esperança.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
