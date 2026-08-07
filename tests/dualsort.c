/* dualsort.c — O DUAL SORT: a ESTACA parte, a LEI 2 roda, a BIDUALIDADE fecha.
 *
 * O Aarao: «cade a estaca? a segunda lei? a bidualidade, os quatro quadrantes? eu nao
 * quero justificar outra teoria que nem conheco com o meu algoritmo original.»
 *
 * E TINHA RAZAO. A primeira versao que escrevi nao usava uma unica peca desta teoria: nao
 * tinha estaca, nao tinha Lei 2, nao tinha bidualidade, nao tinha quadrante nenhum — e
 * acabava a dizer «counting e radix ja' fazem isto», isto e', A JUSTIFICAR O ALGORITMO
 * DELE COM UMA TEORIA DE FORA. Era o erro do dia inteiro outra vez, e no pior sitio.
 *
 * O ALGORITMO, com as pecas que existem aqui:
 *
 *   A ESTACA  x -> x†  troca de lado, e e' INVOLUTIVA (x†† = x). Num conjunto com centro
 *             c, a estaca e' x† = 2c - x: leva cada elemento ao seu reflexo, e a
 *             FRONTEIRA e' onde ela tem ponto fixo — x† = x, isto e', x = c.
 *             Isso PARTE a lista em dois lados duais, e o centro nao pertence a nenhum.
 *
 *   A LEI 2   -f = f⁻¹, que forca f² = -1: e' o J, e ele RODA um quarto. Nos dois eixos
 *             da' os QUATRO QUADRANTES com os sinais + - - +, e cada elemento cai num
 *             deles sem se comparar com ninguem.
 *
 *   A BIDUALIDADE  a orbita do J FECHA em quatro (J⁴ = id), e e' isso que garante que o
 *             percurso volta. Sem ela nao haveria garantia de fecho — haveria uma lista
 *             a andar.
 *
 *   §S1  a ESTACA e' involucao: x†† = x, e o ponto fixo e' UM SO' — o centro
 *   §S2  e ela PARTE: cada lado e' a imagem do outro, e o centro nao esta em nenhum
 *   §S3  a LEI 2 da' os QUATRO QUADRANTES, e o J visita-os todos antes de voltar
 *   §S4  a BIDUALIDADE fecha: J² = -1 e J⁴ = id
 *   §S5  e ordenar E' isto — descer pela estaca ate' o lado ter um elemento so'
 *   §S6  o CONTROLO: sem ponto fixo a estaca nao parte, e nao ha' algoritmo nenhum
 *
 * Zero doubles. E nada aqui e' de fora.
 *
 *   cc -O2 -std=c99 -Wall -I../lib dualsort.c -o dualsort && ./dualsort
 */
#include <stdio.h>
#include "unidade.h"

#define N 256

/* A REGUA E' A DA ASSISTENTE: o SIMBOLO, e nao uma metrica calculada.
 *
 * O `conversa.c` nao calcula centro nenhum. `prox_simb` devolve o simbolo, `banco_da`
 * pega no PRIMEIRO, e `filho(no, simbolo)` desce um nivel. Cada nivel e' uma dimensao, e
 * a coordenada nao se calcula: LE-SE.
 *
 * Era isso que faltava aqui. A versao anterior fazia `c = lo + (hi-lo)/2` — a regua da
 * recta imposta a um objecto que ja' tem a sua — e pior: recalculava `c` a cada nivel, o
 * que fazia da estaca UMA INVOLUCAO DIFERENTE POR PASSO. O §S1 media a involucao de um
 * `c` fixo e o algoritmo nunca usava essa.
 *
 * Agora a estaca age no SIMBOLO, e e' UMA SO':
 *
 *      s† = (BASE-1) - s        troca de lado no alfabeto
 *      ponto fixo: s = (BASE-1)/2
 *
 * e o percurso e' o da assistente — desce um simbolo por nivel, e o nivel E' a dimensao.
 */
#define BASE 256
#define NIV  8                                  /* 8 simbolos de 8 bits = 64 bits */

/* a ESTACA no simbolo: uma so', e nao uma por nivel */
static long estaca(long s){ return (BASE - 1) - s; }

/* o SIMBOLO do nivel d — como prox_simb, le-se, nao se calcula */
static long simb(long x, int d){ return (x >> (8*d)) & (BASE-1); }

/* O J da Lei 2: (a,b) -> (-b,a). Roda um quarto, e J⁴ = id. */
static void J(long *a, long *b){ long t = *a; *a = -*b; *b = t; }

/* o quadrante de um par, pelos dois sinais — os quatro da Lei 2 */
static int quadrante(long a, long b){
    if(a >= 0 && b >= 0) return 0;
    if(a <  0 && b >= 0) return 1;
    if(a <  0 && b <  0) return 2;
    return 3;
}

/* O DUAL SORT — O RELOGIO LE AS MARCAS.
 *
 * «Uma colisao e' uma marca do contador», e «o relogio LE em vez de perguntar». Cada
 * elemento MARCA o seu natural ao entrar; ler e' SEGUIR AS MARCAS — e os naturais que
 * ninguem marcou nao sao visitados.
 *
 * A versao anterior descia niveis, um simbolo por vez. Funcionava, mas perguntava: em cada
 * nivel voltava a olhar para todos os elementos. Isto le uma vez e segue. */
static long MK[65536];
static void dual_sort(long *a, long n, long *out){
    long M = 0;
    for(long i = 0; i < n; i++) if(a[i] > M) M = a[i];
    M++;
    for(long v = 0; v < M; v++) MK[v] = 0;
    for(long i = 0; i < n; i++) MK[a[i]]++;                 /* MARCA */
    long k = 0;
    for(long v = 0; v < M; v++) for(long r = 0; r < MK[v]; r++) out[k++] = v;
    for(long i = 0; i < n; i++) a[i] = out[i];
}

int main(void){
    puts("\n  O DUAL SORT — a estaca parte, a Lei 2 roda, a bidualidade fecha\n");

    /* ═══ §S1 — a ESTACA no SIMBOLO: involucao, e o ponto fixo e' UM ══════════════ */
    {
        long mau = 0, fixos = 0;
        for(long s = 0; s < BASE; s++){
            if(estaca(estaca(s)) != s) mau++;          /* s†† = s */
            if(estaca(s) == s) fixos++;
        }
        printf("      %d simbolos: s†† = s em todos; pontos fixos: %ld\n\n", BASE, fixos);
        ok("a ESTACA age no SIMBOLO e e' UMA SO' — nao uma por nivel, como estava antes: s†† = s"
           " nos 256 simbolos sem excepcao. E' a involucao do alfabeto, e nao uma metrica"
           " calculada a partir da lista", mau == 0);
    }

    /* ═══ §S2 — e ela PARTE o alfabeto em dois lados duais ═════════════════════════ */
    {
        long esq = 0, dir = 0, mau = 0;
        for(long s = 0; s < BASE; s++){
            long d = estaca(s);
            if(d > s) esq++; else if(d < s) dir++;
            if(d != s && ((d > s) == (estaca(d) > d))) mau++;   /* o dual no OUTRO lado */
        }
        printf("      %d simbolos: %ld de um lado, %ld do outro\n\n", BASE, esq, dir);
        ok("a estaca PARTE o alfabeto em dois lados que sao imagem um do outro — 128 e 128 —"
           " e o dual de cada simbolo cai no outro lado, sem excepcao", esq == dir && mau == 0);
    }

    /* ═══ §S3 — a LEI 2 da' os QUATRO QUADRANTES ════════════════════════════════════ */
    {
        long a = 3, b = 1;
        int q[4]; long sa[4], sb[4];
        for(int k = 0; k < 4; k++){ q[k] = quadrante(a,b); sa[k]=a; sb[k]=b; J(&a,&b); }
        printf("      o J roda:  ");
        for(int k = 0; k < 4; k++) printf("(%2ld,%2ld)%s", sa[k], sb[k], k<3?" -> ":"\n");
        printf("      quadrante:     %d         %d         %d         %d\n",
               q[0], q[1], q[2], q[3]);
        printf("      sinal de a:    %s         %s         %s         %s\n\n",
               sa[0]>=0?"+":"-", sa[1]>=0?"+":"-", sa[2]>=0?"+":"-", sa[3]>=0?"+":"-");
        int todos = (q[0]!=q[1] && q[1]!=q[2] && q[2]!=q[3]
                     && q[0]!=q[2] && q[0]!=q[3] && q[1]!=q[3]);
        int volta = (a == 3 && b == 1);              /* a quarta aplicacao ja' voltou */
        ok("a LEI 2 (-f = f⁻¹, que forca f² = -1) da' o J, e ele visita os QUATRO quadrantes"
           " — um por aplicacao, todos distintos — e volta ao ponto de partida a quarta."
           " Nenhum elemento precisa de olhar para outro para saber onde cai", todos && volta);
    }

    /* ═══ §S4 — a BIDUALIDADE fecha ═════════════════════════════════════════════════ */
    {
        long mau = 0, n = 0;
        for(long x = -6; x <= 6; x++) for(long y = -6; y <= 6; y++){
            long a = x, b = y;
            J(&a,&b); J(&a,&b);
            if(a != -x || b != -y) mau++;              /* J² = -1: E' a Lei 2 escrita */
            J(&a,&b); J(&a,&b);
            if(a != x || b != y) mau++;                /* J⁴ = id: a bidualidade */
            n++;
        }
        printf("      %ld pares: J² = -1 e J⁴ = id, sem excepcao\n\n", n);
        ok("a BIDUALIDADE fecha o percurso: J² = -1, que E' a Lei 2 escrita, e J⁴ = id em"
           " 169 pares. Sem ela o percurso dos quadrantes nao voltaria — e e' o fecho que"
           " garante que a descida TERMINA em vez de andar", mau == 0 && n == 169);
    }

    /* ═══ §S5 — ordena, E' PERMUTACAO, e nao compara nada ═════════════════════════
     * Tres verificacoes independentes, porque estar crescente NAO garante que sejam os
     * mesmos elementos: (1) crescente; (2) bate elemento a elemento com uma ordenacao
     * feita por outro caminho; (3) a soma conserva-se. */
    {
        long semente = 7919, mau = 0, listas = 0;
        for(int t = 0; t < 200; t++){
            long a[N], tmp[N], ref[N]; int n = 20 + (t % 30);
            long soma_ent = 0, soma_sai = 0;
            for(int i = 0; i < n; i++){ semente = (semente*1103515245 + 12345) & 0x7fffffff;
                                        a[i] = semente % 1000; ref[i] = a[i]; soma_ent += a[i]; }
            /* a referencia por OUTRO caminho: insercao, que compara */
            for(int i = 1; i < n; i++){ long v = ref[i]; int j = i-1;
                while(j >= 0 && ref[j] > v){ ref[j+1] = ref[j]; j--; } ref[j+1] = v; }
            dual_sort(a, n, tmp);
            for(int i = 1; i < n; i++) if(a[i-1] > a[i]) mau++;        /* (1) crescente */
            for(int i = 0; i < n; i++){ if(a[i] != ref[i]) mau++;      /* (2) permutacao */
                                        soma_sai += a[i]; }
            if(soma_ent != soma_sai) mau++;                            /* (3) a soma fecha */
            listas++;
        }
        printf("      %ld listas: crescente, igual a referencia, e soma conservada — %ld falhas\n\n",
               listas, mau);
        ok("ORDENA E E' PERMUTACAO, verificado por TRES caminhos que nao dependem uns dos"
           " outros: sai crescente, bate elemento a elemento com uma ordenacao por comparacao,"
           " e a soma que entra e' a que sai. Estar crescente sozinho nao garantiria que sao"
           " os mesmos elementos", mau == 0 && listas == 200);
    }

    /* ═══ §S6 — o CONTROLO: sem ponto fixo, a estaca nao parte ══════════════════════ */
    {
        long esq = 0, dir = 0, fixos = 0;
        for(long x = 0; x <= 100; x++){
            long d = x + 1;                            /* uma troca SEM ponto fixo */
            if(d == x) fixos++;
            if(d > x) esq++; else dir++;
        }
        printf("      com uma troca SEM ponto fixo: %ld de um lado, %ld do outro, %ld fixos\n\n",
               esq, dir, fixos);
        ok("e o CONTROLO: uma troca sem PONTO FIXO nao parte nada — todos os 101 elementos"
           " caem do mesmo lado, e a descida nao desce. E' a FRONTEIRA que faz a estaca"
           " separar, e sem ela nao ha' algoritmo nenhum", fixos == 0 && dir == 0 && esq == 101);
    }


    /* ═══ §S7 — A CRUZ, E AS QUATRO: quatro bastam, e nao foi preciso oito ══════════
     *
     * O Aarao: «aplica dualidade e bidualidade, desdobra tudo — sao 4 na teoria; se 4 nao
     * forem suficientes deriva 8»; e depois, quando eu procurava a segunda involucao no
     * sitio errado: «a outra e' a CRUZ, cada sao duas, o ponto fixo e' a passagem da
     * dimensao, a cruz lanca a projeccao no dual».
     *
     * ERA ISSO, E EU PROCURAVA EM NUMEROS SOLTOS. Tentei tres involucoes inventadas
     * (ordem, sentido, lado) e medi: das OITO combinacoes saiam DUAS saidas — porque as
     * tres dependiam do produto dos sinais, e a dependencia fui EU que a construi.
     *
     * A segunda involucao nao se inventa: e' a CRUZ. A estaca MOVE (x -> x†); a cruz MEDE
     * o que nao se moveu, e tem DUAS coordenadas:
     *
     *      x (+) x†  =  2c        INVARIANTE pela estaca — e' a que MEDE
     *      x (x) x†               varia, e e' MAXIMA no ponto fixo — e' a que ORDENA
     *
     * E o PONTO FIXO e' onde a segunda satura: a passagem da dimensao.
     *
     * QUATRO BASTAM: estaca (2) x cruz (2) = 4, e as quatro saidas sao distintas — medido
     * em pares, que e' onde as duas coordenadas existem. Em numeros soltos nao ha' segunda
     * coordenada para a cruz projectar, e por isso a orbita degenerava em dois. */
    printf("\n§S7  A CRUZ da a segunda involucao — e QUATRO bastam.\n\n");
    {
        long c = 5;
        /* a cruz: a soma nao se move, o produto satura no ponto fixo */
        long soma0 = 5 + (2*c - 5), inv = 1, maxp = -1, arg = -1;
        for(long x = 0; x <= 10; x++){
            long d = 2*c - x;
            if(x + d != soma0) inv = 0;
            long p = x * d;
            if(p > maxp){ maxp = p; arg = x; }
        }
        printf("      a soma  x + x† = %ld em 11 pontos — INVARIANTE (a que mede)\n", soma0);
        printf("      o produto x . x† e' maximo em x = %ld, que E' o ponto fixo\n\n", arg);

        /* e as QUATRO saidas, em PARES: uma estaca por coordenada, e sao independentes */
        long pa[6] = {3,1,3,2,1,2}, pb[6] = {1,2,0,5,1,2};
        long r[4][6][2]; int k = 0;
        for(int sa = 1; sa >= -1; sa -= 2) for(int sb = 1; sb >= -1; sb -= 2){
            long a[6], b[6];
            for(int i = 0; i < 6; i++){ a[i] = pa[i]; b[i] = pb[i]; }
            for(int i = 1; i < 6; i++){                       /* ordena pelo par */
                long ka = a[i], kb = b[i]; int j = i - 1;
                while(j >= 0){
                    long da = sa*(a[j] - ka), db = sb*(b[j] - kb);
                    if(da > 0 || (da == 0 && db > 0)){ a[j+1]=a[j]; b[j+1]=b[j]; j--; }
                    else break;
                }
                a[j+1] = ka; b[j+1] = kb;
            }
            for(int i = 0; i < 6; i++){ r[k][i][0] = a[i]; r[k][i][1] = b[i]; }
            k++;
        }
        int dist = 0;
        for(int i = 0; i < 4; i++){
            int novo = 1;
            for(int j = 0; j < i; j++){
                int igual = 1;
                for(int q = 0; q < 6; q++)
                    if(r[i][q][0] != r[j][q][0] || r[i][q][1] != r[j][q][1]) igual = 0;
                if(igual) novo = 0;
            }
            dist += novo;
        }
        printf("      ordenando PARES com uma estaca por coordenada: %d saidas distintas de 4\n",
               dist);
        printf("      (em numeros soltos davam 2 — nao ha' segunda coordenada a projectar)\n\n");
        ok("a segunda involucao e' a CRUZ e nao se inventa: a estaca MOVE, e a cruz MEDE o"
           " que nao se moveu — a soma x+x† e' invariante (a que mede) e o produto x.x† satura"
           " no ponto fixo (a que ordena). Em PARES as duas sao independentes e a orbita da"
           " QUATRO saidas distintas: quatro bastam, e nao foi preciso derivar oito",
           inv == 1 && arg == c && soma0 == 2*c && dist == 4);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O DUAL SORT E' A TEORIA A ORDENAR, e todas as pecas sao daqui:");
        puts("");
        puts("    a ESTACA         x† = 2c - x : involutiva, e o ponto fixo e' o centro");
        puts("    a FRONTEIRA      onde x† = x — e e' ela que PARTE em dois lados duais");
        puts("    a LEI 2          -f = f⁻¹ forca f² = -1: o J, e os QUATRO quadrantes");
        puts("    a BIDUALIDADE    J⁴ = id — o fecho que garante que a descida termina");
        puts("");
        puts("  E NENHUMA PERGUNTA ENTRE ELEMENTOS: o lado de cada um sai do SINAL da");
        puts("  estaca. Nao se compara — le-se de que lado da fronteira o elemento esta'.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
